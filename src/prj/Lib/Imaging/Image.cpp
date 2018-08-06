//
// GammaLib: Computer Vision library
//
//    Copyright (C) 1998-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
//
//    Third party copyrights are property of their respective owners.
//
//
//    The MIT License(MIT)
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
//
//
//





#include "lib.h"

#include "ImageBMP.hpp"							// Load/Save BMP
#include "ImageExternals.hpp"					// Conversion and others
#include "colors.hpp"


inline BYTE WI_Type2BYTE(WI_Type x) {
	x=x+((WI_Type)128);

	if (x<0) x=0;
	if (x>255) x=255;

	return (BYTE)x;
}

inline WI_Type BYTE2WI_Type(BYTE x) {
	return (((WI_Type)x)-((WI_Type)128));
}

bool operator==(const ColorRGB &a,const ColorRGB &b) {
    return ((a.r==b.r)&&(a.g==b.g)&&(a.b==b.b));
}
bool operator!=(const ColorRGB &a,const ColorRGB &b) {
    return ((a.r!=b.r)||(a.g!=b.g)||(a.b!=b.b));
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////  Bitmap   ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


template <class color_type>
Bitmap<color_type>::Bitmap(char *filename) {
	this->width=0;
	this->height=0;
	this->virtual_=false;
	this->Img=NULL;
	SAVE_FLOAT_MIN_VAL=+FLT_MAX;
	SAVE_FLOAT_MAX_VAL=-FLT_MAX;

	Load(filename);
}

template <class color_type>
Bitmap<color_type>::Bitmap(int width,int height) {
	this->width=width;
	this->height=height;
	this->virtual_=false;
	this->Img=NULL;
	SAVE_FLOAT_MIN_VAL=+FLT_MAX;
	SAVE_FLOAT_MAX_VAL=-FLT_MAX;


	if ((this->height<=0) || (this->width<=0)) {
		this->height=0;
		this->width=0;
		Img=NULL;
		virtual_=true;
	} else {
		Img=new color_type[width*height];
		if (Img==NULL) ErrorExit("Can't allocate bitmap memory.");
		virtual_=false;
	}
}


template <class color_type>
Bitmap<color_type>::~Bitmap() {
	if ((!virtual_) && (Img!=NULL)) delete []Img;
}









template<>
bool Bitmap<ColorRGB>::isgrayscale() {
	return false;
}
template<>
bool Bitmap<GreyLevel>::isgrayscale() {
	return true;
}
template<>
bool Bitmap<ColorRGBA>::isgrayscale() {
	return false;
}
template<>
bool Bitmap<ColorFloat>::isgrayscale() {
	return true;
}
template<>
bool Bitmap<ColorRGB_int>::isgrayscale() {
	return false;
}
template<>
bool Bitmap<WI_Type>::isgrayscale() {
	return true;
}
//template<>
//bool Bitmap<ColorInt>::isgrayscale() {
//	return true;
//}






template <>
void Bitmap<ColorRGBA>::ExtractChannel(int channel,Bitmap<ColorFloat> *dst) {
	if ((width!=dst->width) || (height!=dst->height)) ErrorExit("Bitmap: Images size don't match.");
	if ((channel<0) || (channel>3)) ErrorExit("Bitmap: Channel number does not exist.");

	BYTE       *srb=(BYTE *)Img;
	ColorFloat *dsb=dst->getBuffer();
	int dim=width*height;

	srb+=channel;
	for(int i=0;i<dim;i++,srb+=4,dsb++) 
		(*dsb)=(ColorFloat)(*srb);
}

template <>
void Bitmap<ColorRGB>::ExtractChannel(int channel,Bitmap<ColorFloat> *dst) {
	if ((width!=dst->width) || (height!=dst->height)) ErrorExit("Bitmap: Images size don't match.");
	if ((channel<0) || (channel>2)) ErrorExit("Bitmap: Channel number does not exist.");

	BYTE       *srb=(BYTE *)Img;
	ColorFloat *dsb=dst->getBuffer();
	int dim=width*height;

	srb+=channel;
	for(int i=0;i<dim;i++,srb+=3,dsb++) 
		(*dsb)=(ColorFloat)(*srb);
}

template <class color_type>
void Bitmap<color_type>::ExtractChannel(int channel,Bitmap<ColorFloat> *dst) {
	NOT_IMPLEMENTED;
}


template<>
ColorRGB Bitmap<ColorRGB>::GetColor(float r,float g,float b,float a) {
	ColorRGB X;
	X.r=(BYTE)(r*255);
	X.g=(BYTE)(g*255);
	X.b=(BYTE)(b*255);
	return X;
}

template<>
ColorRGBA Bitmap<ColorRGBA>::GetColor(float r,float g,float b,float a) {
	ColorRGBA X;
	X=(((BYTE)(a*255))<<24)+(((BYTE)(r*255))<<16)+(((BYTE)(g*255))<<8)+((BYTE)(b*255));
	return X;
}

template<>
ColorFloat Bitmap<ColorFloat>::GetColor(float r,float g,float b,float a) {
	ColorFloat X;
	X=(ColorFloat)(0.299*r+0.587*g+0.114*b);
	return X;
}

template<>
GreyLevel Bitmap<GreyLevel>::GetColor(float r,float g,float b,float a) {
	GreyLevel X;
	X=(BYTE)((0.299*r+0.587*g+0.114*b)*255);
	return X;
}

template<class color_type>
color_type Bitmap<color_type>::GetColor(float r,float g,float b,float a) {
	NOT_IMPLEMENTED;
}



template<class color_type>
int Bitmap<color_type>::bpp() {
	return sizeof(color_type);
}










////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Save
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////// Common
void SaveRAW_Data(char *filename,BYTE *img,int width,int height,int pixel_size) {
	OFileBuffer O(filename);
	O.WriteLine((char*)&width,sizeof(int));
	O.WriteLine((char*)&height,sizeof(int));
	O.WriteLine((char*)&pixel_size,sizeof(int));

	int dim=height;
	int p_s=width*((int)pixel_size);
	for(int i=0;i<dim;i++) {
		O.WriteLine((char *)(img+(i*p_s)),p_s);
	}
	O.Close();
}

////////////

bool isSupportedByOpenCV(char *Ext) {
	bool supported=false;
	
	if (!_stricmp(Ext,"DIB")) supported=true;
	if (!_stricmp(Ext,"PNG")) supported=true;
	if (!_stricmp(Ext,"JPEG")) supported=true;
	if (!_stricmp(Ext,"JPG")) supported=true;
	if (!_stricmp(Ext,"JPE")) supported=true;
	if (!_stricmp(Ext,"JP2")) supported=true;
	if (!_stricmp(Ext,"PPM")) supported=true;
	if (!_stricmp(Ext,"PGM")) supported=true;
	if (!_stricmp(Ext,"PBM")) supported=true;
	if (!_stricmp(Ext,"TIFF")) supported=true;
	if (!_stricmp(Ext,"TIF")) supported=true;
	
	return supported;
}

template <>
void Bitmap<ColorFloat>::Save(char *filename) {
	if ((width==0) || (height==0)) ErrorExit("Bitmap: Image size is not specified.");

	char *Ext=Get_File_Extension(filename);
	if (Ext==NULL) ErrorExit("Bitmap: Not a valid file extension.");

	if (!_stricmp(Ext,"BMP")) return SaveBMP(filename);
	if (!_stricmp(Ext,"RAW")) return SaveRAW(filename);
	if (!_stricmp(Ext,"FLOAT")) return SaveFLOAT(filename);

	if (isSupportedByOpenCV(Ext)) {
		BYTE *tbuf=(BYTE *)FloattoRGB_Clamped(Img,width,height,-FLT_MAX,FLT_MAX,SAVE_FLOAT_MIN_VAL,SAVE_FLOAT_MAX_VAL);
		
		Bitmap<ColorRGB> tmp(0,0);
		tmp.setBuffer((ColorRGB*)tbuf);
		tmp.width=width;
		tmp.height=height;

		SaveImageOCV(filename,&tmp,tmp.isgrayscale());
		delete []tbuf;
		return;
	}
}

template <class color_type>
void Bitmap<color_type>::Save(char *filename) {
	if ((width==0) || (height==0)) ErrorExit("Bitmap: Image size is not specified.");

	char *Ext=Get_File_Extension(filename);
	if (Ext==NULL) ErrorExit("Bitmap: Not a valid file extension.");

	if (!_stricmp(Ext,"BMP")) return SaveBMP(filename);
	if (!_stricmp(Ext,"RAW")) return SaveRAW(filename);
	if (!_stricmp(Ext,"FLOAT")) return SaveFLOAT(filename);

	if (isSupportedByOpenCV(Ext)) return SaveImageOCV(filename,this,isgrayscale());
}



template <>
void Bitmap<GreyLevel>::SaveBMP(char *filename) {
	BYTE *tbuf=(BYTE *)GraytoRGB((BYTE*)Img,width,height);
	::SaveBMP(filename,tbuf,width,height);
	delete []tbuf;
}

template <>
void Bitmap<ColorRGBA>::SaveBMP(char *filename) {
	BYTE *tbuf=(BYTE *)RGBAtoRGB((DWORD*)Img,width,height);
	::SaveBMP(filename,tbuf,width,height);
	delete []tbuf;
}    	

template <>
void Bitmap<ColorFloat>::SaveBMP(char *filename) {
	BYTE *tbuf=(BYTE *)FloattoRGB_Clamped(Img,width,height,-FLT_MAX,FLT_MAX,SAVE_FLOAT_MIN_VAL,SAVE_FLOAT_MAX_VAL);
	::SaveBMP(filename,tbuf,width,height);
	delete []tbuf;
}

template <>
void Bitmap<ColorRGB>::SaveBMP(char *filename) {
	::SaveBMP(filename,(BYTE *)Img,width,height);
}


template <class color_type>
void Bitmap<color_type>::SaveRAW(char *filename) {
	SaveRAW_Data(filename,(BYTE *)Img,width,height,(int)sizeof(color_type));
}

template <>
void Bitmap<ColorFloat>::SaveFLOAT(char *filename) {
	SaveRAW_Data(filename,(BYTE *)Img,width,height,(int)sizeof(ColorFloat));
}

template <>
void Bitmap<GreyLevel>::SaveFLOAT(char *filename) {
	float *tbuf=GreytoFloat(Img,width,height);
	SaveRAW_Data(filename,(BYTE *)tbuf,width,height,(int)sizeof(float));
	delete []tbuf;
}

template <>
void Bitmap<ColorRGB>::SaveFLOAT(char *filename) {
	BYTE *GreyImg=Convert2GreyScale((BYTE *)Img,width,height);
	float *FloatImg=GreytoFloat(GreyImg,width,height);
	SaveRAW_Data(filename,(BYTE *)FloatImg,width,height,(int)sizeof(float));
	delete []FloatImg;
	delete []GreyImg;
}

template <>
void Bitmap<ColorRGBA>::SaveFLOAT(char *filename) {
	BYTE *RGBImg=RGBAtoRGB((DWORD*)Img,width,height);
	BYTE *GreyImg=Convert2GreyScale(RGBImg,width,height);
	float *FloatImg=GreytoFloat(GreyImg,width,height);
	SaveRAW_Data(filename,(BYTE *)FloatImg,width,height,(int)sizeof(float));
	delete []FloatImg;
	delete []GreyImg;
	delete []RGBImg;
}    	



template <class color_type>
void Bitmap<color_type>::SaveFLOAT(char *filename) {
	ErrorExit("SaveFLOAT: Not implemented.");
}

template <class color_type>
void Bitmap<color_type>::SaveBMP(char *filename) {
	ErrorExit("SaveBMP: Not implemented.");
}

template <>
float ToGrey(GreyLevel c) {
	return (c/255.0f);
}

template <>
float ToGrey(ColorRGB c) {
	return ((0.299f*c.r+0.587f*c.g+0.114f*c.b)/255.0f);
}

template <>
float ToGrey(ColorFloat c) {
	return c;
}












////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Load
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <class color_type>
void Bitmap<color_type>::Load(char *filename) {
	// Delete Previous Image and Image Information
	if ((!virtual_) && (Img!=NULL)) delete []Img;
	this->width=0;
	this->height=0;
	this->virtual_=false;
	this->Img=NULL;

	char *Ext=Get_File_Extension(filename);
	if (Ext==NULL) ErrorExit("Bitmap: Not a valid file extension.");

	if (!_stricmp(Ext,"BMP"))   LoadBMP(filename);
	if (!_stricmp(Ext,"DIB"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"PNG"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"JPEG"))  LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"JPG"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"JPE"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"JP2"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"PPM"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"PGM"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"PBM"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"TIFF"))  LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"TIF"))   LoadImageOCV(filename,this,isgrayscale());
	if (!_stricmp(Ext,"RAW"))   LoadRAW(filename);
	if (!_stricmp(Ext,"FLOAT")) LoadFLOAT(filename);
}

template <>
void Bitmap<GreyLevel>::LoadBMP(char *filename) {
	UINT W_,H_;
	BYTE *buf=::LoadBMP(filename,&W_,&H_);
	width=(int)W_;
	height=(int)H_;
	virtual_=false;

	Img=(GreyLevel *)Convert2GreyScale(buf,W_,H_);
	delete []buf;
}

template <>
void Bitmap<ColorRGBA>::LoadBMP(char *filename) {
	UINT W_,H_;
	BYTE *buf=::LoadBMP(filename,&W_,&H_);
	width=(int)W_;
	height=(int)H_;
	virtual_=false;

	Img=(ColorRGBA *)RGBtoRGBA((BYTE *)buf,W_,H_);
	delete []buf;
}

template <>
void Bitmap<ColorRGB>::LoadBMP(char *filename) {
	UINT W_,H_;
	BYTE *buf=::LoadBMP(filename,&W_,&H_);
	width=(int)W_;
	height=(int)H_;
	virtual_=false;

	Img=(ColorRGB *)buf;
}

template <>
void Bitmap<ColorFloat>::LoadBMP(char *filename) {
	UINT W_,H_;
	BYTE *buf=::LoadBMP(filename,&W_,&H_);
	width=(int)W_;
	height=(int)H_;
	virtual_=false;
	
	GreyLevel *GreyImg=(GreyLevel *)Convert2GreyScale(buf,W_,H_);
	delete []buf;
	Img=(ColorFloat *)GreytoFloat(GreyImg,W_,H_);
	delete []GreyImg;
}


template <class color_type>
void Bitmap<color_type>::LoadRAW(char *filename) {
	IFileBuffer I(filename);
	int len;
	int pixel_size;
	len=sizeof(int);I.ReadLine((char*)&width,len);
	len=sizeof(int);I.ReadLine((char*)&height,len);
	len=sizeof(int);I.ReadLine((char*)&pixel_size,len);
	if (pixel_size!=bpp()) ErrorExit("Cannot open a raw file with different bit per pixels");

	len=width*height*pixel_size;
	Img=(color_type*)new char[len];
	I.ReadLine((char*)Img,len);
}

template <>
void Bitmap<ColorFloat>::LoadFLOAT(char *filename) {
	LoadRAW(filename);
}

template <class color_type>
void Bitmap<color_type>::LoadFLOAT(char *filename) {
	ErrorExit("LoadFLOAT: Not implemented.");
}

template <class color_type>
void Bitmap<color_type>::LoadBMP(char *filename) {
	ErrorExit("LoadBMP: Not implemented.");
}















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Convert Copy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////// From GreyLevel
template <class color_type>
void Bitmap<color_type>::CopyFrom(Bitmap<GreyLevel> *src) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFrom(Bitmap<GreyLevel> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	memcpy(Img,src->Img,sizeof(GreyLevel)*width*height);
}
template <>
void Bitmap<ColorRGB>::CopyFrom(Bitmap<GreyLevel> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	GreyLevel *srb=src->Img;
	ColorRGB *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		dsb->r=(*srb);
		dsb->g=(*srb);
		dsb->b=(*srb);
		
		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorRGBA>::CopyFrom(Bitmap<GreyLevel> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	GreyLevel *srb=src->Img;
	ColorRGBA *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		(*dsb) =((UINT)(*srb))<<16;
		(*dsb)+=((UINT)(*srb))<<8;
		(*dsb)+=((UINT)(*srb));

		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorFloat>::CopyFrom(Bitmap<GreyLevel> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	GreyLevel *srb=src->Img;
	ColorFloat *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		(*dsb)=(ColorFloat)((*srb)/255.0);
		srb++;dsb++;
	}
}
template <>
void Bitmap<WI_Type>::CopyFrom(Bitmap<GreyLevel> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	GreyLevel *srb=src->Img;
	WI_Type *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		(*dsb)=BYTE2WI_Type(*srb);
		srb++;dsb++;
	}
}

///////// From ColorFloat
template <class color_type>
void Bitmap<color_type>::CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,color_type VAL_MIN,color_type VAL_MAX) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,GreyLevel VAL_MIN,GreyLevel VAL_MAX) {
	ColorFloat *srb;
	GreyLevel  *dsb;
	int dim=width*height;

	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	srb=src->Img;
	for(int j=0;j<dim;j++,srb++) {
		if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
		if ((*srb)>Max) Max=(*srb);
		if ((*srb)<Min) Min=(*srb);
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	dsb=Img;
	srb=src->Img;
	for(int j=0;j<dim;j++,srb++,dsb++) {
		if ((*srb)<Min) {(*dsb)=VAL_MIN;continue;}
		if ((*srb)>Max) {(*dsb)=VAL_MAX;continue;}
		float tmp=(float)(((*srb)*d)+of);

		(*dsb)=(BYTE)tmp;
	}
}


template <>
void Bitmap<ColorRGB>::CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,ColorRGB VAL_MIN,ColorRGB VAL_MAX) {
	ColorFloat *srb;
	ColorRGB   *dsb;
	int dim=width*height;

	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	srb=src->Img;
	for(int j=0;j<dim;j++,srb++) {
		if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
		if ((*srb)>Max) Max=(*srb);
		if ((*srb)<Min) Min=(*srb);
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	dsb=Img;
	srb=src->Img;
	for(int j=0;j<dim;j++,srb++,dsb++) {
		if ((*srb)<Min) {(*dsb)=VAL_MIN;continue;}
		if ((*srb)>Max) {(*dsb)=VAL_MAX;continue;}
		float tmp=(float)(((*srb)*d)+of);

		dsb->r=dsb->g=dsb->b=(BYTE)tmp;
	}
}

template <>
void Bitmap<ColorRGBA>::CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,ColorRGBA VAL_MIN,ColorRGBA VAL_MAX) {
	ColorFloat *srb;
	ColorRGBA  *dsb;
	int dim=width*height;

	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	srb=src->Img;
	for(int j=0;j<dim;j++,srb++) {
		if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
		if ((*srb)>Max) Max=(*srb);
		if ((*srb)<Min) Min=(*srb);
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	dsb=Img;
	srb=src->Img;
	for(int j=0;j<dim;j++,srb++,dsb++) {
		if ((*srb)<Min) {(*dsb)=VAL_MIN;continue;}
		if ((*srb)>Max) {(*dsb)=VAL_MAX;continue;}
		UINT tmp=(UINT)(((*srb)*d)+of);

		(*dsb) =(tmp)<<16;
		(*dsb)+=(tmp)<<8;
		(*dsb)+=tmp;
	}
}

template <>
void Bitmap<ColorFloat>::CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,ColorFloat VAL_MIN,ColorFloat VAL_MAX) {
	ColorFloat *srb;
	ColorFloat *dsb;
	int dim=width*height;

	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	srb=src->Img;
	for(int j=0;j<dim;j++,srb++) {
		if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
		if ((*srb)>Max) Max=(*srb);
		if ((*srb)<Min) Min=(*srb);
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	dsb=Img;
	srb=src->Img;
	for(int j=0;j<dim;j++,srb++,dsb++) {
		if ((*srb)<Min) {(*dsb)=VAL_MIN;continue;}
		if ((*srb)>Max) {(*dsb)=VAL_MAX;continue;}
		float tmp=(float)(((*srb)*d)+of);

		(*dsb)=tmp;
	}
}

///////// From ColorFloat
template <class color_type>
void Bitmap<color_type>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	ColorFloat *srb;
	GreyLevel  *dsb;
	int dim=width*height;
	float Max,Min;
	

	if (adjust) {
		Max=-FLT_MAX;
		Min=FLT_MAX;
		srb=src->Img;
		for(int j=0;j<dim;j++,srb++) {
			if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
			if ((*srb)>Max) Max=(*srb);
			if ((*srb)<Min) Min=(*srb);
		}
	} else {
		Min=0.0f;
		Max=255.0f;
	}

	dsb=Img;
	srb=src->Img;
	float multiplier=255.0f/(Max-Min);
	for(int j=0;j<dim;j++) {
		if ((*srb)<Min) {
			(*dsb)=0;
			srb++;dsb++;
			continue;
		}
		if ((*srb)>Max) {
			(*dsb)=255;
			srb++;dsb++;
			continue;
		} 
		(*dsb)=(BYTE)(((*srb)-Min)*multiplier);
		srb++;dsb++;
	}

}
template <>
void Bitmap<ColorRGB>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	ColorFloat *srb;
	ColorRGB   *dsb;
	int dim=width*height;
	float Max,Min;
	
	if (adjust) {
		Max=-FLT_MAX;
		Min=FLT_MAX;
		srb=src->Img;
		for(int j=0;j<dim;j++,srb++) {
			if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
			if ((*srb)>Max) Max=(*srb);
			if ((*srb)<Min) Min=(*srb);
		}
	} else {
		Min=0.0f;
		Max=255.0f;
	}

	dsb=Img;
	srb=src->Img;
	float multiplier=255.0f/(Max-Min);
	for(int j=0;j<dim;j++) {
		if ((*srb)<Min) {
			dsb->g=255;
			dsb->b=dsb->r=0;
			srb++;dsb++;
			continue;
		}
		if ((*srb)>Max) {
			dsb->r=255;
			dsb->b=dsb->g=0;
			srb++;dsb++;
			continue;
		} 
		dsb->r=dsb->g=dsb->b=(BYTE)(((*srb)-Min)*multiplier);
		srb++;dsb++;
	}

}
template <>
void Bitmap<ColorRGBA>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	ColorFloat *srb;
	ColorRGBA  *dsb;
	int dim=width*height;
	float Max,Min;
	
	if (adjust) {
		Max=-FLT_MAX;
		Min=FLT_MAX;
		srb=src->Img;
		for(int j=0;j<dim;j++,srb++) {
			if (((*srb)<=CLAMP_MIN) || ((*srb)>=CLAMP_MAX)) continue;
			if ((*srb)>Max) Max=(*srb);
			if ((*srb)<Min) Min=(*srb);
		}
	} else {
		Min=0.0f;
		Max=255.0f;
	}

	dsb=Img;
	srb=src->Img;
	UINT tmp;
	float multiplier=255.0f/(Max-Min);
	for(int j=0;j<dim;j++) {
		if ((*srb)<Min) {
			(*dsb)=0x00FF00;
			srb++;dsb++;
			continue;
		}
		if ((*srb)>Max) {
			(*dsb)=0xFF0000;
			srb++;dsb++;
			continue;
		} 
		tmp=(UINT)(((*srb)-Min)*multiplier);
		(*dsb) =(tmp)<<16;
		(*dsb)+=(tmp)<<8;
		(*dsb)+=tmp;

		srb++;dsb++;
	}

}
template <>
void Bitmap<ColorFloat>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	memcpy(Img,src->Img,sizeof(ColorFloat)*width*height);
}
template <>
void Bitmap<WI_Type>::CopyFrom(Bitmap<ColorFloat> *src,bool adjust,float CLAMP_MIN,float CLAMP_MAX) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	ColorFloat *srb=src->Img;
	WI_Type  *dsb=Img;
	int dim=width*height;

	if (adjust) {
		for(int j=0;j<dim;j++) {
			(*dsb)=(WI_Type)(*srb);
			if ((*srb)<-128) (*dsb)=(WI_Type)(-128);
			if ((*srb)>127) (*dsb)=(WI_Type)(127);
			srb++;dsb++;
		}
	} else {
		for(int j=0;j<dim;j++) {
			(*dsb)=(WI_Type)(*srb);
			srb++;dsb++;
		}
	}

}

///////// From ColorRGB
template <class color_type>
void Bitmap<color_type>::CopyFrom(Bitmap<ColorRGB> *src) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFrom(Bitmap<ColorRGB> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	double gray;
	ColorRGB  *srb=src->Img;
	GreyLevel *dsb=Img;
	int dim=width*height;

	for(int j=0;j<dim;j++) {
		gray=(0.299*srb->r)+(0.587*srb->g)+(0.114*srb->b);
			
		(*dsb)=(BYTE)gray;
		if (gray>255) (*dsb)=255;
		if (gray<0) (*dsb)=0;

		dsb++;srb++;
	}
}
template <>
void Bitmap<ColorRGB>::CopyFrom(Bitmap<ColorRGB> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	memcpy(Img,src->Img,width*height*sizeof(ColorRGB));
}
template <>
void Bitmap<ColorRGBA>::CopyFrom(Bitmap<ColorRGB> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	ColorRGB *srb=src->Img;
	ColorRGBA *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		(*dsb) =((UINT)(srb->r))<<16;
		(*dsb)+=((UINT)(srb->g))<<8;
		(*dsb)+=((UINT)(srb->b));

		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorFloat>::CopyFrom(Bitmap<ColorRGB> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	double gray;
	ColorRGB *srb=src->Img;
	ColorFloat *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		gray=((0.299*srb->r)+(0.587*srb->g)+(0.114*srb->b))/255.0;
			
		(*dsb)=(ColorFloat)gray;
		if (gray>1.0) (*dsb)=1.0;
		if (gray<0.0) (*dsb)=0.0;

		srb++;dsb++;
	}
}
template <>
void Bitmap<WI_Type>::CopyFrom(Bitmap<ColorRGB> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	double gray;
	BYTE gray_byte;
	ColorRGB *srb=src->Img;
	WI_Type *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		gray=(0.299*srb->r)+(0.587*srb->g)+(0.114*srb->b);
		gray_byte=(BYTE)gray;
		if (gray>255) gray_byte=255;
		if (gray<0) gray_byte=0;

		(*dsb)=BYTE2WI_Type(gray_byte);
		srb++;dsb++;
	}
}

///////// From ColorRGBA
template <class color_type>
void Bitmap<color_type>::CopyFrom(Bitmap<ColorRGBA> *src) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFrom(Bitmap<ColorRGBA> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	double gray;
	ColorRGBA *srb=src->Img;
	GreyLevel *dsb=Img;
	int dim=width*height;

	for(int j=0;j<dim;j++) {
		gray=(0.299*(RED_C(*srb)))+(0.587*(GREEN_C(*srb)))+(0.114*(BLUE_C(*srb)));
			
		(*dsb)=(BYTE)gray;
		if (gray>255) (*dsb)=255;
		if (gray<0) (*dsb)=0;

		dsb++;srb++;
	}
}
template <>
void Bitmap<ColorRGB>::CopyFrom(Bitmap<ColorRGBA> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	ColorRGBA *srb=src->Img;
	ColorRGB *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		dsb->r=RED_C(*srb);
		dsb->g=GREEN_C(*srb);
		dsb->b=BLUE_C(*srb);

		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorRGBA>::CopyFrom(Bitmap<ColorRGBA> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	memcpy(Img,src->Img,width*height*sizeof(ColorRGBA));

}
template <>
void Bitmap<ColorFloat>::CopyFrom(Bitmap<ColorRGBA> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	double gray;
	ColorRGBA *srb=src->Img;
	ColorFloat *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		gray=((0.299*(RED_C(*srb)))+(0.587*(GREEN_C(*srb)))+(0.114*(BLUE_C(*srb))))/255.0;
			
		(*dsb)=(ColorFloat)gray;
		if (gray>1.0) (*dsb)=1.0;
		if (gray<0.0) (*dsb)=0.0;

		srb++;dsb++;
	}
}
template <>
void Bitmap<WI_Type>::CopyFrom(Bitmap<ColorRGBA> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	double gray;
	BYTE gray_byte;
	ColorRGBA *srb=src->Img;
	WI_Type *dsb=Img;
	int dim=width*height;

	for(int i=0;i<dim;i++) {
		gray=(0.299*(RED_C(*srb)))+(0.587*(GREEN_C(*srb)))+(0.114*(BLUE_C(*srb)));
		gray_byte=(BYTE)gray;
		if (gray>255) gray_byte=255;
		if (gray<0) gray_byte=0;

		(*dsb)=BYTE2WI_Type(gray_byte);
		srb++;dsb++;
	}
}

///////// From WI_Type
template <class color_type>
void Bitmap<color_type>::CopyFrom(Bitmap<WI_Type> *src) {
	ErrorExit("CopyFrom: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::CopyFrom(Bitmap<WI_Type> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	WI_Type *srb=src->Img;
	GreyLevel *dsb=Img;
	int dim=width*height;
	
	for(int i=0;i<dim;i++) {
		(*dsb)=WI_Type2BYTE(*srb);
		
		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorRGB>::CopyFrom(Bitmap<WI_Type> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	WI_Type *srb=src->Img;
	ColorRGB *dsb=Img;
	int dim=width*height;
	
	for(int i=0;i<dim;i++) {
		dsb->b=dsb->g=dsb->r=WI_Type2BYTE(*srb);
		
		srb++;dsb++;
	}
}
template <>
void Bitmap<ColorRGBA>::CopyFrom(Bitmap<WI_Type> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	WI_Type *srb=src->Img;
	ColorRGBA *dsb=Img;
	int dim=width*height;
	
	for(int i=0;i<dim;i++) {
		UINT val=WI_Type2BYTE(*srb);
		(*dsb) =(val)<<16;
		(*dsb)+=(val)<<8;
		(*dsb)+=(val);

		srb++;dsb++;
	}

}
template <>
void Bitmap<ColorFloat>::CopyFrom(Bitmap<WI_Type> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	
	WI_Type *srb=src->Img;
	ColorFloat *dsb=Img;
	int dim=width*height;
	
	for(int i=0;i<dim;i++) {
		(*dsb)=(ColorFloat)(*srb);
		
		srb++;dsb++;
	}
}
template <>
void Bitmap<WI_Type>::CopyFrom(Bitmap<WI_Type> *src) {
	if ((width!=src->width) || (height!=src->height)) ErrorExit("Bitmap: Images size don't match.");

	memcpy(Img,src->Img,width*height*sizeof(WI_Type));
}

















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Metodi
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef SYSTEM_WINDOWS
BITMAPINFO *Gray_Scale;

template <class color_type>
void Bitmap<color_type>::Draw(HDC hdc,int x,int y,int operation) {
	ErrorExit("Draw: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::Draw(HDC hdc,int x,int y,int operation) {
	Gray_Scale->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Gray_Scale->bmiHeader.biWidth = width;
	Gray_Scale->bmiHeader.biHeight = height;
	Gray_Scale->bmiHeader.biPlanes = 1;
	Gray_Scale->bmiHeader.biBitCount = 8;
	Gray_Scale->bmiHeader.biCompression = BI_RGB;
	Gray_Scale->bmiHeader.biSizeImage = 0;
	Gray_Scale->bmiHeader.biXPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biYPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biClrUsed = 0;
	Gray_Scale->bmiHeader.biClrImportant = 0;
	
	VertFlipBuf(Img,width*1,height);
	StretchDIBits(hdc,x,y,width,height,0,0,width,height,Img,Gray_Scale,DIB_RGB_COLORS,operation);
	VertFlipBuf(Img,width*1,height);
}

template <>
void Bitmap<ColorRGB>::Draw(HDC hdc,int x,int y,int operation) {
	Gray_Scale->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Gray_Scale->bmiHeader.biWidth = width;
	Gray_Scale->bmiHeader.biHeight = height;
	Gray_Scale->bmiHeader.biPlanes = 1;
	Gray_Scale->bmiHeader.biBitCount = 24;
	Gray_Scale->bmiHeader.biCompression = BI_RGB;
	Gray_Scale->bmiHeader.biSizeImage = 0;
	Gray_Scale->bmiHeader.biXPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biYPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biClrUsed = 0;
	Gray_Scale->bmiHeader.biClrImportant = 0;
	
	BGRFromRGB((BYTE*)Img,width,height);
	VertFlipBuf((BYTE*)Img,width*3,height);
	StretchDIBits(hdc,x,y,width,height,0,0,width,height,Img,Gray_Scale,DIB_RGB_COLORS,operation);
	VertFlipBuf((BYTE*)Img,width*3,height);
	BGRFromRGB((BYTE*)Img,width,height);
}


template <>
void Bitmap<ColorRGBA>::Draw(HDC hdc,int x,int y,int operation) {
	Gray_Scale->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Gray_Scale->bmiHeader.biWidth = width;
	Gray_Scale->bmiHeader.biHeight = height;
	Gray_Scale->bmiHeader.biPlanes = 1;
	Gray_Scale->bmiHeader.biBitCount = 32;
	Gray_Scale->bmiHeader.biCompression = BI_RGB;
	Gray_Scale->bmiHeader.biSizeImage = 0;
	Gray_Scale->bmiHeader.biXPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biYPelsPerMeter = 0;
	Gray_Scale->bmiHeader.biClrUsed = 0;
	Gray_Scale->bmiHeader.biClrImportant = 0;
	
	BGRAFromRGBA((BYTE*)Img,width,height);
	VertFlipBuf((BYTE*)Img,width*4,height);
	StretchDIBits(hdc,x,y,width,height,0,0,width,height,Img,Gray_Scale,DIB_RGB_COLORS,operation);
	VertFlipBuf((BYTE*)Img,width*4,height);
	BGRAFromRGBA((BYTE*)Img,width,height);
}

#endif 


template <class color_type>
bool Bitmap<color_type>::isOutside(int x,int y,int lasco) {
	if ((x<0+lasco) || (x>=width-lasco)) {
		return true;
	}
	if ((y<0+lasco) || (y>=height-lasco)) {
		return true;
	}
	return false;
}

template <class color_type>
float Bitmap<color_type>::BorderDistance(int x,int y) {
	float dx_=(1.0f*min(x,width-1-x))/((int)(width/2));    // /floor(width/2)
	float dy_=(1.0f*min(y,height-1-y))/((int)(height/2));

	return min(dx_,dy_);
}


template <class color_type>
bool Bitmap<color_type>::IsValid() {
	return (Img!=NULL);
}


template <class color_type>
void Bitmap<color_type>::Init() {
	#ifdef SYSTEM_WINDOWS
	Gray_Scale=(BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER)+(256*sizeof(RGBQUAD)));

	for (int i=0;i<=255;i++) {
		Gray_Scale->bmiColors[i].rgbBlue=(BYTE)i;
		Gray_Scale->bmiColors[i].rgbRed=(BYTE)i;
		Gray_Scale->bmiColors[i].rgbGreen=(BYTE)i;
	}
	#endif 
}




template <class color_type>
void Bitmap<color_type>::Clear() {
	memset(Img,0,width*height*sizeof(color_type));
}

template <class color_type>
void Bitmap<color_type>::Clear(color_type x) {
	int dim=width*height;
	for(int i=0;i<dim;i++) Img[i]=x;
}

template <>
void Bitmap<GreyLevel>::Randomize() {
	int dim=width*height;
	for(int i=0;i<dim;i++) Img[i]=(BYTE)Uniform_int_rand(0,255);
}

template <>
void Bitmap<ColorRGBA>::Randomize() {
	int dim=width*height;
	for(int i=0;i<dim;i++) Img[i]=Uniform_int_rand(0,255*255*255);
}

template <>
void Bitmap<ColorRGB>::Randomize() {
	int dim=width*height;
	for(int i=0;i<dim;i++) {
		Img[i].r = (BYTE)Uniform_int_rand(0,255);
		Img[i].g = (BYTE)Uniform_int_rand(0, 255);
		Img[i].b = (BYTE)Uniform_int_rand(0, 255);
	}
}

template <>
void Bitmap<ColorFloat>::Randomize() {
	int dim=width*height;
	for(int i=0;i<dim;i++) Img[i]=(ColorFloat)Uniform_double_rand(0.0,1.0);
}




////////////////////////// MULTIPLY ///////////////////////////////////////////////////////////////

template <class color_type>
void Bitmap<color_type>::Multiply(float x) {
	for(int i=0;i<width;i++) {
		for(int j=0;j<height;j++) {
	
			SetPoint(i,j,(color_type)(Point(i,j)*x));   //TODO: Clamp it
		}
	}
}

template <>
void Bitmap<GreyLevel>::Rescale(float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,float VAL_MIN,float VAL_MAX) {
	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	for(int j=0;j<width*height;j++) {
		if ((Img[j]<=CLAMP_MIN) || (Img[j]>=CLAMP_MAX)) continue;
		if (Img[j]>Max) Max=Img[j];
		if (Img[j]<Min) Min=Img[j];
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	for(int j=0;j<width*height;j++) {
		if (Img[j]<Min) {Img[j]=(BYTE)VAL_MIN;continue;}
		if (Img[j]>Max) {Img[j]=(BYTE)VAL_MAX;continue;}
		float tmp=(float)((Img[j]*d)+of);
		
		Img[j]=(BYTE)tmp;
	}
}

template <>
void Bitmap<ColorFloat>::Rescale(float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,float VAL_MIN,float VAL_MAX) {
	float Max=-FLT_MAX;
	float Min=FLT_MAX;

	for(int j=0;j<width*height;j++) {
		if ((Img[j]<=CLAMP_MIN) || (Img[j]>=CLAMP_MAX)) continue;
		if (Img[j]>Max) Max=Img[j];
		if (Img[j]<Min) Min=Img[j];
	}

	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=(MAX-MIN)/(1.0*Max-1.0*Min);
		of=(-(MAX-MIN)*(1.0/((Max/Min)-1.0)))+MIN;
	}

	for(int j=0;j<width*height;j++) {
		if (Img[j]<Min) {Img[j]=VAL_MIN;continue;}
		if (Img[j]>Max) {Img[j]=VAL_MAX;continue;}
		Img[j]=(float)((Img[j]*d)+of);
	}
}

template <>
void Bitmap<ColorRGBA>::Multiply(float x) {
	for(int i=0;i<width;i++) {
		for(int j=0;j<height;j++) {
			ColorRGB a(Point(i,j));
			ColorRGB c=a*x;
			
			SetPoint(i,j,RGBtoRGBA(c));
		}
	}
}




////////////////////////// GAUSSIAN ///////////////////////////////////////////////////////////////

template <class color_type>
void Bitmap<color_type>::Gaussian() {
	ErrorExit("Gaussian: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::Gaussian() {
	int Fw,Fh;
	Fw=Fh=6;				// TODO
	double sigma=5;			// TODO

	double *Filter;
	Filter=new double[(2*Fw+1)*(2*Fh+1)];
	double somma=0;
	for(int a=-Fw;a<=Fw;a++) {
		for(int b=-Fh;b<=Fh;b++) {
			Filter[(b+Fh)*(2*Fw+1)+(a+Fw)]=exp(-(1.0*a*a+1.0*b*b)/(2.0*sigma*sigma));
			somma+=Filter[(b+Fh)*(2*Fw+1)+(a+Fw)];
		}
	}
	somma=1.0/somma;

	for(int i=Fw;i<(width-Fw);i++) {
		for(int j=Fh;j<(height-Fh);j++) {
			double X=0;
			for(int a=-Fw;a<=Fw;a++) {
				for(int b=-Fh;b<=Fh;b++) {
					X=X+Point(i+a,j+b)*(somma*Filter[(b+Fh)*(2*Fw+1)+(a+Fw)]);
				}
			}
			if (X>255) X=255;
			if (X<0) X=0;
			SetPoint(i,j,(BYTE)X);
		}
	}

}

template <>
void Bitmap<ColorFloat>::Gaussian() {
	int Fw,Fh;
	Fw=Fh=6;				// TODO
	double sigma=5;			// TODO

	double *Filter;
	Filter=new double[(2*Fw+1)*(2*Fh+1)];
	double somma=0;
	for(int a=-Fw;a<=Fw;a++) {
		for(int b=-Fh;b<=Fh;b++) {
			Filter[(b+Fh)*(2*Fw+1)+(a+Fw)]=exp(-(1.0*a*a+1.0*b*b)/(2.0*sigma*sigma));
			somma+=Filter[(b+Fh)*(2*Fw+1)+(a+Fw)];
		}
	}
	somma=1.0/somma;

	for(int i=Fw;i<(width-Fw);i++) {
		for(int j=Fh;j<(height-Fh);j++) {
			double X=0;
			for(int a=-Fw;a<=Fw;a++) {
				for(int b=-Fh;b<=Fh;b++) {
					X=X+Point(i+a,j+b)*(somma*Filter[(b+Fh)*(2*Fw+1)+(a+Fw)]);
				}
			}
			if (X>1.0) X=1.0;
			if (X<0) X=0;
			SetPoint(i,j,(float)X);
		}
	}

}

template <>
void Bitmap<ColorRGBA>::Gaussian() {
	printf("Warning: bad gaussian filter.\n");	

	for(int i=0;i<width;i++) {
		for(int j=0;j<height;j++) {
			ColorRGB a(Point(i,j));
			ColorRGB b(Point(i+1,j));
			ColorRGB c=a*0.5+b*0.5;
			
			SetPoint(i,j,RGBtoRGBA(c));
		}
	}

}



////////////////////////// FLIP VERTICAL ///////////////////////////////////////////////////////////////

template <class color_type>
void Bitmap<color_type>::Flip_Vertical() {
	VertFlipBuf((BYTE*)Img,width*sizeof(color_type),height);
}





////////////////////////// THRESHOLD ///////////////////////////////////////////////////////////////

template <class color_type>
void Bitmap<color_type>::ThresholdInvert(color_type level) {
	ErrorExit("ThresholdInvert: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::ThresholdInvert(GreyLevel level) {
	for(int j=0;j<height;j++) {
		for(int i=0;i<width;i++) {
			if (Img[(i+(j*width))]>level) Img[(i+(j*width))]=0;
			else Img[(i+(j*width))]=255;
		}
	}
}

template <class color_type>
void Bitmap<color_type>::Threshold(color_type level) {
	ErrorExit("Threshold: Not implemented.");
}

template <>
void Bitmap<GreyLevel>::Threshold(GreyLevel level) {
	for(int j=0;j<height;j++) {
		for(int i=0;i<width;i++) {
			if (Img[(i+(j*width))]>level) Img[(i+(j*width))]=255;
			else Img[(i+(j*width))]=0;
		}
	}
}

template <>
void Bitmap<ColorRGB>::Threshold(ColorRGB level) {
	for(int j=0;j<height;j++) {
		for(int i=0;i<width;i++) {
			if (Img[(i+(j*width))].r>level.r) Img[(i+(j*width))].r=255;
			else Img[(i+(j*width))].r=0;
			if (Img[(i+(j*width))].g>level.g) Img[(i+(j*width))].g=255;
			else Img[(i+(j*width))].g=0;
			if (Img[(i+(j*width))].b>level.b) Img[(i+(j*width))].b=255;
			else Img[(i+(j*width))].b=0;
		}
	}
}

template <>
void Bitmap<ColorFloat>::Threshold(ColorFloat level) {
	for(int j=0;j<height;j++) {
		for(int i=0;i<width;i++) {
			if (Img[(i+(j*width))]>level) Img[(i+(j*width))]=1.0f;
			else Img[(i+(j*width))]=0.0f;
		}
	}
}



////////////////////////// ADAPTSIZE ///////////////////////////////////////////////////////////////

template <class color_type>
Bitmap<color_type> *Bitmap<color_type>::AdaptSize(int w,int h) {
	Bitmap<color_type> *dst=new Bitmap<color_type>(w,h);
	dst->Clear();
	
	int w_=min(width,w);
	int h_=min(height,h);

	for(int i=0;i<w_;i++) {
		for(int j=0;j<h_;j++) {
			dst->SetPoint(i,j,Point(i,j));
		}
	}

	return dst;
}



////////////////////////// INCLUDE ///////////////////////////////////////////////////////////////

template <class color_type>
void Bitmap<color_type>::Include(Bitmap<color_type> *img,int x,int y) {
	color_type *pixel_in=img->Img;
	
	if ((x+img->width-1)  >=width ) ErrorExit("Size does not match.");
	if ((y+img->height-1) >=height) ErrorExit("Size does not match.");

	for(int j=0;j<img->height;j++) {
		color_type *out=Img+(x+((y+j)*width));
		
		for(int i=0;i<img->width;i++,pixel_in++,out++) (*out)=(*pixel_in);
	}

}














////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////   ISTANZIAZIONE   ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////



template class Bitmap<GreyLevel>;
template class Bitmap<ColorFloat>;
template class Bitmap<ColorRGB>;
template class Bitmap<ColorRGBA>;
template class Bitmap<ColorRGB_int>;
template class Bitmap<int>;


template void BitmapCopy(Bitmap<GreyLevel> *dst,Bitmap<GreyLevel> *src);
template void BitmapCopy(Bitmap<ColorRGB> *dst,Bitmap<ColorRGB> *src);
template void BitmapCopy(Bitmap<ColorRGB> *dst,Bitmap<GreyLevel> *src);
template void BitmapCopy(Bitmap<ColorRGB_int> *dst,Bitmap<GreyLevel> *src);
//template void BitmapCopy(Bitmap<GreyLevel> *dst,Bitmap<ColorRGB> *src);
template void BitmapBlend(Bitmap<ColorRGB> *dst,Bitmap<GreyLevel> *src,float blend_factor);
template void BitmapBlend(Bitmap<GreyLevel> *dst,Bitmap<GreyLevel> *src,float blend_factor);

/*
void Image::Clear(BYTE clear) {
	for(UINT j=0;j<height;j++) 
		for(UINT i=0;i<width;i++) Img[i+j*width]=clear;
}


Image_Neighbour Image::Neighbour(int x,int y,UINT d) {        
	Image_Neighbour o(this,x,y,d);
	return o;
}

void Image::Invert() {
	for(UINT i=0;i<width*height;i++) Img[i]=255-Img[i];
}

void Image::AND(Image *b) {
	for(UINT i=0;i<width*height;i++) Img[i]=Img[i]&b->Img[i];
}

double Derivata3punti(double pre,double o,double post,double d_pre,double d_post) {
	double a,b,r;

	if (d_pre==0) {
		r=(post-o)*(1/d_post);
		return r;
	}
	if (d_post==0) {
		r=(o-pre)*(1/d_pre);
		return r;
	}

	a=(post-o)*(1/(d_post/d_pre));
	b=(pre-o)*(1/(d_pre/d_post));
	r=(1/(d_pre+d_post))*(a-b);
	return r;
}

#define SquareNormGrandienteMinima 10     // il quadrato della soglia minima

Image *Image::GetEdgeImage() {
	Image *edge=new Image(width,height);

	for(UINT j=0;j<height;j++) {
		for(UINT i=0;i<width;i++) {
			double dx=Derivata3punti(Point(i-1,j),Point(i,j),Point(i+1,j),1,1);
			double dy=Derivata3punti(Point(i,j-1),Point(i,j),Point(i,j+1),1,1);
			double SquareNormGrandiente=dx*dx+dy*dy;
			
			if (SquareNormGrandiente>SquareNormGrandienteMinima) edge->Img[(i+j*width)]=255;
			else edge->Img[(i+j*width)]=0;
		}
	}

	return edge;
}

void Image::Threshold(int level) {
	for(UINT j=0;j<height;j++) {
		for(UINT i=0;i<width;i++) {
			if (Img[(i+j*width)]>level) Img[(i+j*width)]=255;
			else Img[(i+j*width)]=0;
		}
	}
}


Image_Neighbour::Image_Neighbour(Image *source,int px,int py,UINT d) {
	this->source=source;
	this->px=px;
	this->py=py;
	this->d=d;
}

void Image_Neighbour::stat(double &media,double &varianza) const {

	media=0;

	for(int j=-d;j<=+d;j++) {
		for(int i=-d;i<=+d;i++) {
			media+=Point(i,j);
		}
	}
	media/=(((2*d)+1)*((2*d)+1));

	varianza=0;
	for(int j=-d;j<=+d;j++) {
		for(int i=-d;i<=+d;i++) {
			double tmp=Point(i,j)-media;
			varianza+=tmp*tmp;
		}
	}

	varianza=sqrt(varianza);
}

double Image_Neighbour::operator*(const Image_Neighbour &b) const {
	double dot;
	double a_m,a_v,b_m,b_v;
	stat(a_m,a_v);
	b.stat(b_m,b_v);

	dot=0;
	for(int j=-d;j<=+d;j++)	{
		for(int i=-d;i<=+d;i++) {
			dot+=(Point(i,j)-a_m)*(b.Point(i,j)-b_m);
		}
	}
	dot/=(a_v*b_v);

	return dot;
}

*/