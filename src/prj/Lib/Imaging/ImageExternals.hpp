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





void GetAlphaChannel(Bitmap<ColorRGBA> *src, Bitmap<GreyLevel> *dst) {
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	ColorRGBA  *src_p=src->getBuffer();
	GreyLevel  *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=ALPHA_C(*src_p);
}
void GetRedChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst) {
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	ColorRGB  *src_p=src->getBuffer();
	GreyLevel *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=src_p->r;
}
void GetGreenChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst){
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	ColorRGB  *src_p=src->getBuffer();
	GreyLevel *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=src_p->g;
}
void GetBlueChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst){
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	ColorRGB  *src_p=src->getBuffer();
	GreyLevel *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=src_p->b;
}
void SetRedChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst) {
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	GreyLevel *src_p=src->getBuffer();
	ColorRGB  *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) dst_p->r=(*src_p);
}
void SetGreenChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst) {
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	GreyLevel *src_p=src->getBuffer();
	ColorRGB  *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) dst_p->g=(*src_p);
}
void SetBlueChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst) {
	if ((dst->width!=src->width) || (dst->height!=src->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=src->width*src->height;

	GreyLevel *src_p=src->getBuffer();
	ColorRGB  *dst_p=dst->getBuffer();

	for(int i=0;i<dim;i++,src_p++,dst_p++) dst_p->b=(*src_p);
}
void SetAlphaChannel(Bitmap<GreyLevel> *alpha,Bitmap<ColorRGBA> *dst) {
	if ((dst->width!=alpha->width) || (dst->height!=alpha->height)) ErrorExit("Bitmap: Images size don't match.");
	int dim=alpha->width*alpha->height;

	GreyLevel *src_p=alpha->getBuffer();
	BYTE      *dst_p=(BYTE *)dst->getBuffer();

	dst_p+=3;
	for(int i=0;i<dim;i++,src_p++,dst_p+=4) (*dst_p)=(*src_p);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CONVERSION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// 
//  Converte Float -> RGB
//		Scala src da (Min_,Max_) a (0 - 255)
//			Tutti i dati fuori (Min_,Max_) vengono colorati di rosso o verde
//
//      Se (Min_,Max_) non vengono specificati -> li detecta automaticamente
//			considerando invalidi i dati fuori da (CLAMP_MIN,CLAMP_MAX) di default = (-FLT_MAX,+FLT_MAX)
//

ColorRGB *FloattoRGB_Clamped(float *Img,int width,int height,float CLAMP_MIN=-FLT_MAX,float CLAMP_MAX=FLT_MAX,float Min_=FLT_MAX,float Max_=-FLT_MAX) {
	ColorRGB *nBuf=new ColorRGB[width*height];

	float Max=-FLT_MAX;
	float Min=FLT_MAX;
	
	if (Min_>Max_) {
		for(int j=0;j<width*height;j++) {
			if ((Img[j]<=CLAMP_MIN) || (Img[j]>=CLAMP_MAX)) continue;
			if (Img[j]>Max) Max=Img[j];
			if (Img[j]<Min) Min=Img[j];
		}
	} else {
		Max=Max_;
		Min=Min_;
	}
	
	// reduce numerical errors
	double d,of;
	if (Min==Max) {
		d=0.0;
		of=0.0;
	} else {
		d=255.0/(1.0*Max-1.0*Min);
		of=-255.0*(1.0/((Max/Min)-1.0));
	}

	for(int j=0;j<width*height;j++) {
		if (Img[j]<Min) {
			nBuf[j].g=255;
			nBuf[j].r=nBuf[j].b=0;
			continue;
		}
		if (Img[j]>Max) {
			nBuf[j].r=255;
			nBuf[j].g=nBuf[j].b=0;
			continue;
		} 
		float tmp=(float)((Img[j]*d)+of);
		nBuf[j].r=nBuf[j].g=nBuf[j].b=(BYTE)tmp;
	}

	return nBuf;
}

ColorRGB *FloattoRGB(float *Img,int width,int height) {
	ColorRGB *nBuf=new ColorRGB[width*height];

	float Max=-FLT_MAX;
	float Min=FLT_MAX;
	
	for(int j=0;j<width*height;j++) {
		if (Img[j]>Max) Max=Img[j];
		if (Img[j]<Min) Min=Img[j];
	}


	for(int j=0;j<width*height;j++) 
		nBuf[j].r=nBuf[j].g=nBuf[j].b=(BYTE)(((Img[j]-Min)/(Max-Min))*255);

	return nBuf;
}

float *GreytoFloat(GreyLevel *Img,int width,int height) {
	float *nBuf=new float[width*height];

	for(int j=0;j<width*height;j++) 
		nBuf[j]=(float)(Img[j]/255.0);

	return nBuf;
}


DWORD *RGBtoRGBA(BYTE *Image,UINT w,UINT h) {
	
	DWORD *nBuf=new DWORD[w*h];

	UINT k=0;
	for(UINT j=0;j<h;j++) {
		for(UINT i=0;i<w;i++) {
			UINT R=Image[(i+j*w)*3];
			UINT G=Image[(i+j*w)*3+1];
			UINT B=Image[(i+j*w)*3+2];
			nBuf[k++]=(R+(G<<8)+(B<<16));
		}
	}

	return nBuf;
}


BYTE *RGBAtoRGB(DWORD *Image,UINT w,UINT h) {
	
	BYTE *nBuf=new BYTE[w*h*3];

	UINT k=0;
	for(UINT j=0;j<h;j++) {
		for(UINT i=0;i<w;i++) {
			BYTE R=GetRValue(Image[(i+j*w)]);
			BYTE G=GetGValue(Image[(i+j*w)]);
			BYTE B=GetBValue(Image[(i+j*w)]);
			nBuf[k++]=R;
			nBuf[k++]=G;
			nBuf[k++]=B;
		}
	}

	return nBuf;
}

BYTE *Convert2GreyScale(BYTE *Img24bit,UINT width,UINT height) {

	BYTE *nBuf=new BYTE[width*height];

	UINT k=0;
	for(UINT j=0;j<height;j++) {
		for(UINT i=0;i<width;i++) {
			BYTE R=Img24bit[(i+j*width)*3];
			BYTE G=Img24bit[(i+j*width)*3+1];
			BYTE B=Img24bit[(i+j*width)*3+2];
			
			double gray=0.299*R+0.587*G+0.114*B;
			
			BYTE g=(BYTE)gray;
			if (gray>255) g=255;
			if (gray<0) g=0;

			nBuf[k++]=g;
		}
	}

	return nBuf;
}

ColorRGB *GraytoRGB(BYTE *Img,UINT width,UINT height) {
	ColorRGB *nBuf=new ColorRGB[width*height];

	for(UINT j=0;j<width*height;j++) 
		nBuf[j].r=nBuf[j].g=nBuf[j].b=Img[j];

	return nBuf;
}

ColorRGBA RGBtoRGBA(ColorRGB c) {
	DWORD a;
	a=(((DWORD)c.r)<<16)+(((DWORD)c.g)<<8)+(DWORD)c.b;
	return a;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////






















template <class color_type1,class color_type2>
void BitmapCopy(Bitmap<color_type1> *dst,Bitmap<color_type2> *src) {
	if (dst->width!=src->width) ErrorExit("Can not copy two bitmaps with different sizes.");
	if (dst->height!=src->height) ErrorExit("Can not copy two bitmaps with different sizes.");

	for(int i=0;i<dst->width;i++) {
		for(int j=0;j<dst->height;j++) {
			dst->SetPoint(i,j,(color_type1)src->Point(i,j));
		}
	}
}

template void BitmapBlend(Bitmap<GreyLevel> *dst,Bitmap<ColorFloat> *src,float blend_factor);

template <class color_type1,class color_type2>
void BitmapBlend(Bitmap<color_type1> *dst,Bitmap<color_type2> *src,float blend_factor) {
	if (dst->width!=src->width) ErrorExit("Can not copy two bitmaps with different sizes.");
	if (dst->height!=src->height) ErrorExit("Can not copy two bitmaps with different sizes.");

	color_type1 c_dst;
	color_type2 c_src;
	for(int i=0;i<dst->width;i++) {
		for(int j=0;j<dst->height;j++) {
			c_src=(color_type2)(src->Point(i,j)*(1-blend_factor));
			c_dst=(color_type1)(dst->Point(i,j)*blend_factor);
			
			dst->SetPoint(i,j,c_dst+((color_type1)c_src));
		}
	}
}


void Invert(Bitmap<GreyLevel> *img) {
	for(int i=0;i<img->width;i++) {
		for(int j=0;j<img->height;j++) {
			img->SetPoint(i,j,255-img->Point(i,j));
		}
	}
}

void MakeRGBA(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *mask,Bitmap<ColorRGBA> *dst) {
	int size_p=src->width*src->height;
	
	BYTE *d_p=(BYTE *)dst->getBuffer();
	ColorRGB *s_p=src->getBuffer();
	GreyLevel *m_p=mask->getBuffer();
	for(int i=0;i<size_p;i++) {
		*(d_p++)=s_p->r;
		*(d_p++)=s_p->g;
		*(d_p++)=s_p->b;
		s_p++;
		*(d_p++)=*(m_p++);
	}
}

void Differences(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	ColorRGB *p_A=A->getBuffer();
	ColorRGB *p_B=B->getBuffer();

	for(int i=0;i<size_p;i++) {
		if ((*p_A).r>(*p_B).r) (*p_A).r=(*p_A).r-(*p_B).r;
		else (*p_A).r=(*p_B).r-(*p_A).r;
		if ((*p_A).g>(*p_B).g) (*p_A).g=(*p_A).g-(*p_B).g;
		else (*p_A).g=(*p_B).g-(*p_A).g;
		if ((*p_A).b>(*p_B).b) (*p_A).b=(*p_A).b-(*p_B).b;
		else (*p_A).b=(*p_B).b-(*p_A).b;
		
		p_A++;
		p_B++;
	}
}

void Differences(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	ColorFloat *p_A=A->getBuffer();
	ColorFloat *p_B=B->getBuffer();

	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if ((*p_A)>(*p_B)) (*p_A)=(*p_A)-(*p_B);
		else (*p_A)=(*p_B)-(*p_A);
	}
}

void Differences(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,Bitmap<GreyLevel> *Out_Image) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	ColorRGB *p_A=A->getBuffer();
	ColorRGB *p_B=B->getBuffer();
	GreyLevel *Out=Out_Image->getBuffer();

	for(int i=0;i<size_p;i++,p_A++,p_B++,Out++) {
		if ((*p_A).r>(*p_B).r) (*Out)=((*p_A).r-(*p_B).r)/3;
		else (*Out)=((*p_B).r-(*p_A).r)/3;
		if ((*p_A).g>(*p_B).g) (*Out)+=((*p_A).g-(*p_B).g)/3;
		else (*Out)+=((*p_B).g-(*p_A).g)/3;
		if ((*p_A).b>(*p_B).b) (*Out)+=((*p_A).b-(*p_B).b)/3;
		else (*Out)+=((*p_B).b-(*p_A).b)/3;
	}
}

double Differences_d(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,int safe_x,int safe_y) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");


	int line_size=A->width*3;
	int safe_lines=line_size*safe_y;
	BYTE *p_A=(BYTE*)A->getBuffer()+safe_lines;
	BYTE *p_B=(BYTE*)B->getBuffer()+safe_lines;
	int n_lines=A->height-(2*safe_y);
	int safe_cols=safe_x*3;
	int safe_line_size=line_size-(2*safe_cols);

	double sum=0;
	for(int j=0;j<n_lines;j++) {
		BYTE *p_At=p_A+safe_cols;
		BYTE *p_Bt=p_B+safe_cols;
		
		for(int i=0;i<safe_line_size;i++,p_At++,p_Bt++) {
			if ((*p_At)>(*p_Bt)) sum+=(*p_At)-(*p_Bt);
			else sum+=(*p_Bt)-(*p_At);
		}

		p_A+=line_size;
		p_B+=line_size;
	}

	sum/=(safe_line_size*n_lines);
	return sum;
}
double Differences_d(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B,int safe_x,int safe_y) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");


	int line_size=A->width;
	int safe_lines=line_size*safe_y;
	BYTE *p_A=(BYTE*)A->getBuffer()+safe_lines;
	BYTE *p_B=(BYTE*)B->getBuffer()+safe_lines;
	int n_lines=A->height-(2*safe_y);
	int safe_cols=safe_x;
	int safe_line_size=line_size-(2*safe_cols);

	double sum=0;
	for(int j=0;j<n_lines;j++) {
		BYTE *p_At=p_A+safe_cols;
		BYTE *p_Bt=p_B+safe_cols;
		
		for(int i=0;i<safe_line_size;i++,p_At++,p_Bt++) {
			if ((*p_At)>(*p_Bt)) sum+=(*p_At)-(*p_Bt);
			else sum+=(*p_Bt)-(*p_At);
		}

		p_A+=line_size;
		p_B+=line_size;
	}

	sum/=(safe_line_size*n_lines);
	return sum;
}

double Differences_d(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height*3;

	BYTE *p_A=(BYTE*)A->getBuffer();
	BYTE *p_B=(BYTE*)B->getBuffer();
	
	double sum=0;
	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if ((*p_A)>(*p_B)) sum+=(*p_A)-(*p_B);
		else sum+=(*p_B)-(*p_A);
	}

	sum/=size_p;
	return sum;
}




void Min(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	GreyLevel *p_A=A->getBuffer();
	GreyLevel *p_B=B->getBuffer();

	for(int i=0;i<size_p;i++) {
		if ((*p_A)>(*p_B)) (*p_A)=(*p_B);
		
		p_A++;
		p_B++;
	}
}

void Max(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	GreyLevel *p_A=A->getBuffer();
	GreyLevel *p_B=B->getBuffer();

	for(int i=0;i<size_p;i++) {
		if ((*p_A)<(*p_B)) (*p_A)=(*p_B);
		
		p_A++;
		p_B++;
	}
}

void ImgMult(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=A->width*A->height;

	ColorFloat *p_A=A->getBuffer();
	ColorFloat *p_B=B->getBuffer();

	for(int i=0;i<size_p;i++) {
		(*p_A)=(*p_A)*(*p_B);
		
		p_A++;
		p_B++;
	}
}

void Bool_OR(Bitmap<ColorRGB> *I,Bitmap<GreyLevel> *O) {
	if ((I->width!=O->width) || (I->height!=O->height)) ErrorExit("Bitmap sizes mismatch.");
	int size_p=I->width*I->height;

	ColorRGB *p_A=I->getBuffer();
	GreyLevel *p_B=O->getBuffer();

	for(int i=0;i<size_p;i++) {
		if ((*p_A).r || (*p_A).g || (*p_A).b) (*p_B)=255;
		else (*p_B)=0;
		
		p_A++;
		p_B++;
	}
}





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


ColorRGB GetMedian(Bitmap<ColorRGB> *src) {
	int size_p = src->width*src->height;

	ColorRGB *p = src->getBuffer();

	Array<BYTE> R(size_p);
	Array<BYTE> G(size_p);
	Array<BYTE> B(size_p);

	for (int i = 0; i<size_p; i++) {
		R.append((*p).r);
		G.append((*p).g);
		B.append((*p).b);
		p++;
	}
	R.sort(compare_byte);
	G.sort(compare_byte);
	B.sort(compare_byte);
	
	ColorRGB m;
	m.r = (((int)R[(size_p >> 1) - 1]) + ((int)R[(size_p >> 1)])) >> 1;
	m.g = (((int)G[(size_p >> 1) - 1]) + ((int)G[(size_p >> 1)])) >> 1;
	m.b = (((int)B[(size_p >> 1) - 1]) + ((int)B[(size_p >> 1)])) >> 1;

	return m;
}


ColorRGB GetMedian(Bitmap<ColorRGB> *src, Bitmap<GreyLevel> *mask) {
	int size_p = src->width*src->height;

	ColorRGB  *p =  src->getBuffer();
	GreyLevel *pm = mask->getBuffer();
	
	Array<BYTE> R(size_p);
	Array<BYTE> G(size_p);
	Array<BYTE> B(size_p);

	for (int i = 0; i<size_p; i++, p++, pm++) {
		if ((*pm) != 255) continue;
		R.append((*p).r);
		G.append((*p).g);
		B.append((*p).b);
	}
	R.sort(compare_byte);
	G.sort(compare_byte);
	B.sort(compare_byte);

	size_p = R.numElements();

	ColorRGB m;
	m.r = 0;
	m.g = 0;
	m.b = 0;
	if (size_p >= 2) {
		m.r = (((int)R[(size_p >> 1) - 1]) + ((int)R[(size_p >> 1)])) >> 1;
		m.g = (((int)G[(size_p >> 1) - 1]) + ((int)G[(size_p >> 1)])) >> 1;
		m.b = (((int)B[(size_p >> 1) - 1]) + ((int)B[(size_p >> 1)])) >> 1;
	}

	return m;
}


