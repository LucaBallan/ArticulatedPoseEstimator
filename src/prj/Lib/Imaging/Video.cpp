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
using namespace std;


//#define OUTPUT_VIDEO_DEBUG_INFORMATION				// Scrive una * ad ogni lettura di un file da disco 
														// Indicativamente fornisce gli accessi al disco (PS: potrebbe essere cachato da funzioni OPENCV o API di windows)


#ifdef OUTPUT_VIDEO_DEBUG_INFORMATION
	#define LOADING_OUTPUT		{cout<<"*\n";}
#else
	#define LOADING_OUTPUT		
#endif






template <class color_type>
Video<color_type>::Video() {
	width=0;
	height=0;
	num_frames=0;
	fps=1;
	bpp=3;
}


























//////////////////////////////////////////////////////////////////
//	CachedBufferedVideo
//////////////////////////////////////////////////////////////////
template <class color_type>
CachedBufferedVideo<color_type>::CachedBufferedVideo(BufferedVideo<color_type> *in,int policy,int max_cached_elements) {
	CriticalSection=CreateMutex(NULL,FALSE,NULL);
	this->in=in;
	
	width=in->width;
	height=in->height;
	num_frames=in->num_frames;
	fps=in->fps;
	bpp=in->bpp;

	// cache gestisce la sincronizzazione dell'oggetto
	cache=new ObjectContainer_class((void *(*)(UINT,void *))CachedBufferedVideo<color_type>::internal_LoadFrame,(void *(*)(void *,UINT,void *))CachedBufferedVideo<color_type>::internal_ReplaceFrame,(void (*)(void *,UINT,void *))CachedBufferedVideo<color_type>::internal_DeleteFrame,this,policy,max_cached_elements);
}

template <class color_type>
CachedBufferedVideo<color_type>::~CachedBufferedVideo() {
	delete cache;
	delete in;
	CloseHandle(CriticalSection);
}

template <class color_type>
void *CachedBufferedVideo<color_type>::internal_LoadFrame(UINT index,CachedBufferedVideo<color_type> *src) {
	Bitmap<color_type> *obj=new Bitmap<color_type>(src->width,src->height);
	if (!(src->in->GetFrame(index,obj))) {
		delete obj;
		return NULL;
	}
	
	return obj;
}

template <class color_type>
void *CachedBufferedVideo<color_type>::internal_ReplaceFrame(Bitmap<color_type> *old_obj,UINT index,CachedBufferedVideo<color_type> *src) {
	Bitmap<color_type> *obj=old_obj;
	if (obj==NULL) obj=new Bitmap<color_type>(src->width,src->height);

	if (!(src->in->GetFrame(index,obj))) {
		delete obj;
		return NULL;
	}
	
	return obj;
}

template <class color_type>
void CachedBufferedVideo<color_type>::internal_DeleteFrame(Bitmap<color_type> *obj,UINT index,CachedBufferedVideo<color_type> *src) {
	if (obj!=NULL) delete obj;
}


template <class color_type>
BYTE *CachedBufferedVideo<color_type>::GetFrameReadOnly_(LONG n) {
	Bitmap<color_type> *obj=(Bitmap<color_type> *)cache->Get(n);
	if (obj==NULL) return NULL;

	return ((BYTE*)obj->getBuffer());
}

template <class color_type>
Bitmap<color_type> *CachedBufferedVideo<color_type>::GetFrameReadOnly(LONG n) {
	Bitmap<color_type> *obj=(Bitmap<color_type> *)cache->Get(n);
	return obj;
}

template <class color_type>
bool CachedBufferedVideo<color_type>::GetFrame(LONG n,Bitmap<color_type> *img) {
	ENTER_C;
	Bitmap<color_type> *obj=(Bitmap<color_type> *)cache->Get(n);
	if (obj==NULL) {
		EXIT_C;
		return false;
	}
	
	img->CopyFrom(obj);
	EXIT_C;
	return true;
}

template <class color_type>
UINT CachedBufferedVideo<color_type>::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	ENTER_C;
	Bitmap<color_type> *obj=(Bitmap<color_type> *)cache->Get(n);
	if (obj==NULL) {
		EXIT_C;
		return INVALID_TEXTURE;
	}

	UINT texture=w->LoadTexture(obj,options);
	EXIT_C;
	return texture;
}





































//////////////////////////////////////////////////////////////////
//	TexCachedBufferedVideo
//////////////////////////////////////////////////////////////////
template <class color_type>
TexCachedBufferedVideo<color_type>::TexCachedBufferedVideo(BufferedVideo<color_type> *in,int policy,int max_cached_elements,RenderWindow<3> *w,int options) {
	this->in=in;
	this->w=w;
	this->options=options;

	width=in->width;
	height=in->height;
	num_frames=in->num_frames;
	fps=in->fps;
	bpp=in->bpp;

	// cache gestisce la sincronizzazione dell'oggetto
	cache=new ObjectContainer_class((void *(*)(UINT,void *))TexCachedBufferedVideo<color_type>::internal_LoadFrame,NULL,(void (*)(void *,UINT,void *))TexCachedBufferedVideo<color_type>::internal_DeleteFrame,this,policy,max_cached_elements);
}

template <class color_type>
TexCachedBufferedVideo<color_type>::~TexCachedBufferedVideo() {
	delete cache;
	delete in;
}

template <class color_type>
void *TexCachedBufferedVideo<color_type>::internal_LoadFrame(UINT index,TexCachedBufferedVideo<color_type> *src) {
	UINT obj=src->in->LoadFrameToTexture(index,src->w,src->options);
	return ((void*)obj);
}

template <class color_type>
void TexCachedBufferedVideo<color_type>::internal_DeleteFrame(void *obj,UINT index,TexCachedBufferedVideo<color_type> *src) {
	UINT texture=(UINT)obj;
	
	if (texture!=INVALID_TEXTURE) {
		src->w->DeleteTexture(texture);
	}
}


template <class color_type>
BYTE *TexCachedBufferedVideo<color_type>::GetFrameReadOnly_(LONG n) {
	return NULL;
}

template <class color_type>
Bitmap<color_type> *TexCachedBufferedVideo<color_type>::GetFrameReadOnly(LONG n) {
	return NULL;
}

template <class color_type>
bool TexCachedBufferedVideo<color_type>::GetFrame(LONG n,Bitmap<color_type> *img) {
	return false;
}

template <class color_type>
UINT TexCachedBufferedVideo<color_type>::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	UINT obj=(UINT)cache->Get(n);
	return obj;
}

template CachedBufferedVideo<ColorRGB>;
template CachedBufferedVideo<ColorRGBA>;
template CachedBufferedVideo<GreyLevel>;
template TexCachedBufferedVideo<ColorRGB>;
template TexCachedBufferedVideo<ColorRGBA>;
template TexCachedBufferedVideo<GreyLevel>;












#ifdef USE_OPENGL_LIBRARY


template <class color_type>
AviVideoIn_OCV<color_type>::AviVideoIn_OCV(char *filename,bool use_null_frames) {
	tmp_buffer=NULL;
	NULLBYTE_FRAME=NULL;
	NULLBITMAP_FRAME=NULL;
	
	cv_cap_filter=cvCreateFileCapture(filename);
	if (cv_cap_filter!=NULL) {
		width=(int)cvGetCaptureProperty((CvCapture *)cv_cap_filter,CV_CAP_PROP_FRAME_WIDTH);
		height=(int)cvGetCaptureProperty((CvCapture *)cv_cap_filter,CV_CAP_PROP_FRAME_HEIGHT);
		fps=cvGetCaptureProperty((CvCapture *)cv_cap_filter,CV_CAP_PROP_FPS);
		num_frames=(int)cvGetCaptureProperty((CvCapture *)cv_cap_filter,CV_CAP_PROP_FRAME_COUNT);
		tmp_buffer=new Bitmap<color_type>(width,height);
		bpp=Bitmap<color_type>::bpp();
		if (use_null_frames) {
			NULLBITMAP_FRAME=new Bitmap<color_type>(width,height);
			NULLBITMAP_FRAME->Clear(NULLBITMAP_FRAME->GetColor(1.0,0,0,0));
			NULLBYTE_FRAME=(BYTE*)NULLBITMAP_FRAME->getBuffer();
		}
	} else num_frames=-1;
}

template <class color_type>
AviVideoIn_OCV<color_type>::~AviVideoIn_OCV() {
	CvCapture *cv_cap_filter_ocv=(CvCapture *)cv_cap_filter;
	if (cv_cap_filter_ocv!=NULL) {
		cvReleaseCapture(&cv_cap_filter_ocv);
		cv_cap_filter=NULL;
	}
	if (tmp_buffer!=NULL) delete tmp_buffer;
	if (NULLBITMAP_FRAME!=NULL) delete NULLBITMAP_FRAME;
	NULLBITMAP_FRAME=NULL;
	NULLBYTE_FRAME=NULL;
}




template <class color_type>
void *AviVideoIn_OCV<color_type>::_GetFrame(LONG n) {
	cvSetCaptureProperty((CvCapture *)cv_cap_filter,CV_CAP_PROP_POS_FRAMES,(double)n);
	IplImage *frame=cvQueryFrame((CvCapture *)cv_cap_filter);
	LOADING_OUTPUT;
	return frame;
}


template <class color_type>
BYTE *AviVideoIn_OCV<color_type>::GetFrameReadOnly_(LONG n) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return NULLBYTE_FRAME;
	
	return (BYTE *)(frame->imageDataOrigin);
}

template <class color_type>
IplImage *AviVideoIn_OCV<color_type>::GetFrameReadOnlyCV(LONG n) {
	return (IplImage *)_GetFrame(n);
}


template <class color_type>
Bitmap<color_type> *AviVideoIn_OCV<color_type>::GetFrameReadOnly(LONG n) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return NULLBITMAP_FRAME;

	Ipl2Bitmap(frame,tmp_buffer);
	return tmp_buffer;
}

template <class color_type>
bool AviVideoIn_OCV<color_type>::GetFrame(LONG n,Bitmap<color_type> *img) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return false;

	Ipl2Bitmap(frame,img);
	return true;
}

template <class color_type>
UINT AviVideoIn_OCV<color_type>::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return INVALID_TEXTURE;
	
	return Ipl2Texture(frame,w,options);
}

template AviVideoIn_OCV<ColorRGB>;
template AviVideoIn_OCV<ColorRGBA>;
template AviVideoIn_OCV<GreyLevel>;
template AviVideoIn_OCV<ColorFloat>;


#endif





MonochromeVideoSequence::MonochromeVideoSequence(char *filename) {
	UINT32 tmp;int len;

	fps=1.0;
	bpp=1;
	width=0;
	height=0;
	num_frames=-1;
	offset=0;
	buffer=NULL;
	source=NULL;
	tmp_buffer=NULL;

	source=new IFileBuffer(filename);
	if (source->IsFinished()) return;

	len=4;source->ReadLine((char *)(&tmp),len);num_frames=tmp;
	len=4;source->ReadLine((char *)(&tmp),len);width=tmp;
	len=4;source->ReadLine((char *)(&tmp),len);height=tmp;
	offset=(int)(ceil((width*height)/32.0));
	buffer=new UINT32[offset];
	tmp_buffer=new Bitmap<GreyLevel>(width,height);
}

MonochromeVideoSequence::~MonochromeVideoSequence() {
	if (buffer) delete []buffer;
	if (tmp_buffer!=NULL) delete tmp_buffer;
	if (source) delete source;
}
	

BYTE *MonochromeVideoSequence::GetFrameReadOnly_(LONG n) {
	if (n>=num_frames) return NULL;
	source->SetPosition(12+(n*offset*4));
	
	// Leggi
	int size=width*height;
	source->ReadBitLine((char*)tmp_buffer->getBuffer(),size,buffer);
	LOADING_OUTPUT;
	return ((BYTE*)tmp_buffer->getBuffer());
}


Bitmap<GreyLevel> *MonochromeVideoSequence::GetFrameReadOnly(LONG n) {
	if (GetFrameReadOnly_(n)==NULL) return NULL;
	return tmp_buffer;
}

bool MonochromeVideoSequence::GetFrame(LONG n,Bitmap<GreyLevel> *img) {
	if (n>=num_frames) return false;
	source->SetPosition(12+(n*offset*4));

	// Leggi
	int size=width*height;
	source->ReadBitLine((char*)img->getBuffer(),size,buffer);
	LOADING_OUTPUT;	
	return true;
}


UINT MonochromeVideoSequence::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	if (GetFrameReadOnly_(n)==NULL) return INVALID_TEXTURE;
	return (w->LoadTexture(tmp_buffer,options));
}



GrayScaleVideoSequence::GrayScaleVideoSequence(char *filename) {
	UINT32 tmp;int len;

	fps=1.0;
	bpp=1;
	width=0;
	height=0;
	num_frames=-1;
	offset=0;
	source=NULL;
	tmp_buffer=NULL;

	source=new IFileBuffer(filename);
	if (source->IsFinished()) return;

	len=4;source->ReadLine((char *)(&tmp),len);num_frames=tmp;
	len=4;source->ReadLine((char *)(&tmp),len);width=tmp;
	len=4;source->ReadLine((char *)(&tmp),len);height=tmp;
	offset=(int)width*height;
	tmp_buffer=new Bitmap<GreyLevel>(width,height);
}

GrayScaleVideoSequence::~GrayScaleVideoSequence() {
	if (tmp_buffer!=NULL) delete tmp_buffer;
	if (source) delete source;
}
	

BYTE *GrayScaleVideoSequence::GetFrameReadOnly_(LONG n) {
	if (n>=num_frames) return NULL;
	source->SetPosition(12+(n*offset));
	
	// Leggi
	int size=width*height;
	source->ReadLine((char*)tmp_buffer->getBuffer(),size);
	LOADING_OUTPUT;
	return ((BYTE*)tmp_buffer->getBuffer());
}


Bitmap<GreyLevel> *GrayScaleVideoSequence::GetFrameReadOnly(LONG n) {
	if (GetFrameReadOnly_(n)==NULL) return NULL;
	return tmp_buffer;
}

bool GrayScaleVideoSequence::GetFrame(LONG n,Bitmap<GreyLevel> *img) {
	if (n>=num_frames) return false;
	source->SetPosition(12+(n*offset));

	// Leggi
	int size=width*height;
	source->ReadLine((char*)img->getBuffer(),size);
	LOADING_OUTPUT;	
	return true;
}


UINT GrayScaleVideoSequence::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	if (GetFrameReadOnly_(n)==NULL) return INVALID_TEXTURE;
	return (w->LoadTexture(tmp_buffer,options));
}

























template <class color_type>
ImageSequence<color_type>::ImageSequence() {
	tmp_buffer=NULL;
	this->filename=NULL;
	this->start_index=0;

	width=0;
	height=0;
	fps=1.0;
	num_frames=-1;
	bpp=Bitmap<color_type>::bpp();
}



template <class color_type>
ImageSequence<color_type>::ImageSequence(char *filename,int start_index) {
	tmp_buffer=NULL;
	this->filename=strcln(filename);
	this->start_index=start_index;

	fps=1.0;
	num_frames=0;
	bpp=Bitmap<color_type>::bpp();

	
	char path[500];
	convert_printf_wildcard_int(filename,path);
	int min_index;
	int num_files=count_all_files_by_template(path,&min_index);
	if (num_files==0) {
		num_frames=-1;
		return;
	} 

	sprintf(path,filename,min_index);
	tmp_buffer=new Bitmap<color_type>(path);
	if (tmp_buffer->getBuffer()==NULL) num_frames=-1;
	width=tmp_buffer->width;
	height=tmp_buffer->height;
}

template <class color_type>
ImageSequence<color_type>::~ImageSequence() {
	if (tmp_buffer!=NULL) delete tmp_buffer;
	if (filename!=NULL) delete []filename;
}



template <class color_type>
void ImageSequence<color_type>::LoadBitmap(LONG n) {
	char path[500];
	sprintf(path,filename,((int)n+start_index));
	tmp_buffer->Load(path);
	LOADING_OUTPUT;	
}

template <class color_type>
BYTE *ImageSequence<color_type>::GetFrameReadOnly_(LONG n) {
	LoadBitmap(n);

	return ((BYTE*)tmp_buffer->getBuffer());
}


template <class color_type>
Bitmap<color_type> *ImageSequence<color_type>::GetFrameReadOnly(LONG n) {
	LoadBitmap(n);

	if (tmp_buffer->getBuffer()==NULL) return NULL;
	return tmp_buffer;
}

template <class color_type>
bool ImageSequence<color_type>::GetFrame(LONG n,Bitmap<color_type> *img) {
	LoadBitmap(n);
	if (tmp_buffer->getBuffer()==NULL) return false;

	//PerformanceWarning("This is not faster than other.");	// TODOSS
	img->CopyFrom(tmp_buffer);
	return true;
}

template <class color_type>
UINT ImageSequence<color_type>::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	LoadBitmap(n);
	return (w->LoadTexture(tmp_buffer,options));
}





template <class color_type>
CachedImageSequence<color_type>::CachedImageSequence(char *filename,int start_index) {
	cache=NULL;
	tmp_buffer=NULL;
	this->filename=strcln(filename);
	this->start_index=start_index;

	fps=1.0;
	num_frames=0;
	bpp=Bitmap<color_type>::bpp();


	cache=new ObjectContainer_class((void *(*)(UINT,void *))CachedImageSequence<color_type>::internal_LoadFrame,NULL,(void (*)(void *,UINT,void *))CachedImageSequence<color_type>::internal_DeleteFrame,this,POLICY_BEST,300);

	LoadBitmap(0);
	if (tmp_buffer->getBuffer()==NULL) num_frames=-1;
	width=tmp_buffer->width;
	height=tmp_buffer->height;
}

template <class color_type>
CachedImageSequence<color_type>::~CachedImageSequence() {
	if (cache!=NULL) delete cache;
	cache=NULL;
	tmp_buffer=NULL;
}

template <class color_type>
void *CachedImageSequence<color_type>::internal_LoadFrame(UINT index,CachedImageSequence<color_type> *src) {
	char path[500];
	sprintf(path,src->filename,index+src->start_index);
	return (new Bitmap<color_type>(path));
}

template <class color_type>
void CachedImageSequence<color_type>::internal_DeleteFrame(Bitmap<color_type> *obj,UINT index,CachedImageSequence<color_type> *src) {
	delete obj;
}

template <class color_type>
void CachedImageSequence<color_type>::LoadBitmap(LONG n) {
	tmp_buffer=(Bitmap<color_type>*)cache->Get((UINT)n);
}


template ImageSequence<ColorRGB>;
template ImageSequence<ColorRGBA>;
template ImageSequence<GreyLevel>;
template ImageSequence<ColorFloat>;
template CachedImageSequence<ColorRGB>;
template CachedImageSequence<ColorRGBA>;
template CachedImageSequence<GreyLevel>;







VideoWriter::VideoWriter(int w,int h,double fps,int num_frames,bool color,char *path,int type) {
	this->w=w;this->h=h;
	this->color=color;
	this->fps=fps;
	this->type=type;
	this->path=new char[strlen(path)+1];
	strcpy(this->path,path);

	// Init
	this->counter=0;
	this->video=NULL;
	this->tmpC=NULL;
	this->tmpG=NULL;
	this->tmpM=NULL;
	this->iplD=NULL;

	switch (type) {
		case VIDEOTYPE_XVID_AVI:
			//CV_FOURCC('P','I','M','1') //HFYU
			video=cvCreateVideoWriter(path,CV_FOURCC('X','V','I','D'),fps,cvSize(w,h),(color==true?1:0));
			break;
		case VIDEOTYPE_MJPG_AVI:
			video=cvCreateVideoWriter(path,CV_FOURCC('M','J','P','G'),fps,cvSize(w,h),(color==true?1:0));
			break;
		case VIDEOTYPE_HFYU_AVI:
			video=cvCreateVideoWriter(path,CV_FOURCC('H','F','Y','U'),fps,cvSize(w,h),(color==true?1:0));
			break;
		case VIDEOTYPE_LAGS_AVI:
			video=cvCreateVideoWriter(path,CV_FOURCC('L','A','G','S'),fps,cvSize(w,h),(color==true?1:0));
			break;
		case VIDEOTYPE_PROMPT_AVI:
			video=cvCreateVideoWriter(path,CV_FOURCC_PROMPT,fps,cvSize(w,h),(color==true?1:0));
			break;
		case VIDEOTYPE_MONOC_AVI:
			video=new OFileBuffer(path);
			{
				UINT32 tmp;
				tmp=num_frames;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
				tmp=w;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
				tmp=h;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
			}
			break;
		case VIDEOTYPE_BW_AVI:
			video=new OFileBuffer(path);
			{
				UINT32 tmp;
				tmp=num_frames;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
				tmp=w;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
				tmp=h;((OFileBuffer *)video)->WriteLine((char *)(&tmp),4);
			}
			break;
		case VIDEOTYPE_IMAGE_SEQ:
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}
}

VideoWriter::~VideoWriter() {
	Close();
}

bool VideoWriter::isValid() {
	return (video!=NULL);
}

void VideoWriter::Close() {
	switch (type) {
		case VIDEOTYPE_IMAGE_SEQ:			
			break;
		case VIDEOTYPE_MONOC_AVI:
			if (video) {
				((OFileBuffer *)video)->Flush();
				((OFileBuffer *)video)->Close();
				delete video;
				video=NULL;
			}
			break;
		case VIDEOTYPE_BW_AVI:
			if (video) {
				((OFileBuffer *)video)->Flush();
				((OFileBuffer *)video)->Close();
				delete video;
				video=NULL;
			}
			break;
		default:
			if (video) {
				cvReleaseVideoWriter((CvVideoWriter**)(&video));
				video=NULL;
			}
			break;
	}
	if (tmpM) {delete []tmpM;tmpM=NULL;}
	if (tmpC) {delete tmpC;tmpC=NULL;}
	if (tmpG) {delete tmpG;tmpG=NULL;}
	if (iplD) {cvReleaseImage((IplImage **)(&iplD));iplD=NULL;}
}

void VideoWriter::Save(Bitmap<ColorRGB> *I) {
	if (type<VIDEOTYPE_MONOC_AVI) {
		if (video) {
			if (!iplD) iplD=cvCreateImage(cvSize(w,h),8,(color==true?3:1));
			Bitmap2Ipl(I,(IplImage *)iplD);
			cvWriteFrame((CvVideoWriter*)video,(IplImage *)iplD);
		}
		return;
	}

	switch (type) {
		case VIDEOTYPE_MONOC_AVI:
			if (!tmpG) tmpG=new Bitmap<GreyLevel>(w,h);
			tmpG->CopyFrom(I);
			if (!tmpM) tmpM=new UINT32[(int)(ceil((w*h)/32.0))];
			((OFileBuffer *)video)->WriteBitLine((char*)tmpG->getBuffer(),w*h,tmpM);
			break;
		case VIDEOTYPE_BW_AVI:
			if (!tmpG) tmpG=new Bitmap<GreyLevel>(w,h);
			tmpG->CopyFrom(I);
			((OFileBuffer *)video)->WriteLine((char*)tmpG->getBuffer(),w*h);
			break;
		case VIDEOTYPE_IMAGE_SEQ:
			char Text[500];
			sprintf(Text,path,counter);
			if (color) {
				I->Save(Text);
			} else {
				if (!tmpG) tmpG=new Bitmap<GreyLevel>(w,h);
				tmpG->CopyFrom(I);
				tmpG->Save(Text);
			}
			counter++;
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}
}

void VideoWriter::Save(Bitmap<GreyLevel> *I) {
	if (type<VIDEOTYPE_MONOC_AVI) {
		if (video) {
			if (!iplD) iplD=cvCreateImage(cvSize(w,h),8,(color==true?3:1));
			Bitmap2Ipl(I,(IplImage *)iplD);
			cvWriteFrame((CvVideoWriter*)video,(IplImage *)iplD);
		}
		return;
	}

	switch (type) {
		case VIDEOTYPE_MONOC_AVI:
			if (!tmpM) tmpM=new UINT32[(int)(ceil((w*h)/32.0))];
			((OFileBuffer *)video)->WriteBitLine((char*)I->getBuffer(),w*h,tmpM);
			break;
		case VIDEOTYPE_BW_AVI:
			((OFileBuffer *)video)->WriteLine((char*)I->getBuffer(),w*h);
			break;
		case VIDEOTYPE_IMAGE_SEQ:
			char Text[500];
			sprintf(Text,path,counter);
			if (!color) {
				I->Save(Text);
			} else {
				if (!tmpC) tmpC=new Bitmap<ColorRGB>(w,h);
				tmpC->CopyFrom(I);
				tmpC->Save(Text);
			}
			counter++;
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}
}

















































//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OPENGL_LIBRARY
#ifdef USE_FFMPEG_LIBRARY

#ifdef WIN32
	#define __STDC_CONSTANT_MACROS
#else
	#include<stdint.h>
	#define INT8_C(value) ((int8_t) value)
	#define UINT8_C(value) ((uint8_t) __CONCAT(value, U))
	#define INT16_C(value) value
	#define UINT16_C(value) __CONCAT(value, U)
	#define INT32_C(value) __CONCAT(value, L)
	#define UINT32_C(value) __CONCAT(value, UL)
	#define INT64_C(value) __CONCAT(value, LL)
	#define UINT64_C(value) __CONCAT(value, ULL)
	#define INTMAX_C(value) __CONCAT(value, LL)
	#define UINTMAX_C(value) __CONCAT(value, ULL)
#endif

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

class VideoSourceFFMPEG {
public:
	AVFormatContext  *pFormatCtx;
	AVCodecContext   *pCodecCtx;
	AVCodec          *pCodec;
	AVFrame          *pFrame;
	AVFrame          *pFrameBGR;
	SwsContext       *pScaler;
	IplImage         *bgr_image;
	int64_t           picture_pts;
	int               videoStream;



	VideoSourceFFMPEG();
	~VideoSourceFFMPEG();
	
	bool open(char *video_filename);
	void close();

	IplImage *getFrame();                       // (read-only)

	bool      seek(int64_t frame_n);
	bool      seek2(int64_t timestamp);
	double    timestamp2frame(int64_t timestamp);
	double    num_frames();
	int64_t   getTimeStamp() {return picture_pts;};
};


bool FFMPEG_INITIALIZED=false;


VideoSourceFFMPEG::VideoSourceFFMPEG() {
	if (!FFMPEG_INITIALIZED) {
		av_register_all();
		FFMPEG_INITIALIZED=true;
	}

	pFormatCtx=NULL;
	pCodecCtx=NULL;
	pCodec=NULL;
	pFrame=NULL;
	pFrameBGR=NULL;
	pScaler=NULL;
	bgr_image=NULL;
	videoStream=-1;
	picture_pts=0;
}
VideoSourceFFMPEG::~VideoSourceFFMPEG() {
	close();
}
void VideoSourceFFMPEG::close() {
	if (pScaler)    {sws_freeContext(pScaler);pScaler=NULL;}
	if (pFrameBGR)  {av_free(pFrameBGR);pFrameBGR=NULL;}
	if (pFrame)     {av_free(pFrame);pFrame=NULL;}
	if (bgr_image)  {if (bgr_image->imageData) delete[](bgr_image->imageData);cvReleaseImageHeader(&bgr_image);bgr_image=NULL;}
	if (pCodecCtx)  {avcodec_close(pCodecCtx);pCodecCtx=NULL;}
	if (pFormatCtx) {av_close_input_file(pFormatCtx);pFormatCtx=NULL;}
	pCodec=NULL;
	videoStream=-1;
	picture_pts=0;
}
bool VideoSourceFFMPEG::open(char *video_filename) {
	close();

	if (avformat_open_input(&pFormatCtx,video_filename,NULL,NULL)!=0) return false;
	if (av_find_stream_info(pFormatCtx)<0) {close();return false;}
	av_dump_format(pFormatCtx,0,video_filename,0);

	videoStream=-1;
	for(unsigned int i=0;i<pFormatCtx->nb_streams;i++) {
		if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	}
	if(videoStream==-1) {close();return false;}

	pCodecCtx=pFormatCtx->streams[videoStream]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec==NULL) {close();return false;}
	if (avcodec_open(pCodecCtx,pCodec)<0) {close();return false;}

	pFrame=avcodec_alloc_frame();
	if (pFrame==NULL) {close();return false;}


	pFrameBGR=avcodec_alloc_frame();
	if (pFrameBGR==NULL) {close();return false;}
	int numBytes=avpicture_get_size(PIX_FMT_BGR24,pCodecCtx->width,pCodecCtx->height);
	
	// pFrame
	uint8_t *tmp_buff=new uint8_t[numBytes];
	avpicture_fill((AVPicture *)pFrameBGR,tmp_buff,PIX_FMT_BGR24,pCodecCtx->width,pCodecCtx->height);
	
	// Opencv image
	bgr_image=cvCreateImageHeader(cvSize(pCodecCtx->width,pCodecCtx->height),8,3);
	cvSetImageData(bgr_image,tmp_buff,pFrameBGR->linesize[0]);

	pScaler=sws_getCachedContext(NULL,pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height,PIX_FMT_BGR24,SWS_BILINEAR,NULL,NULL,NULL);
	if (pScaler==NULL) {close();return false;}

	return true;
}
IplImage *VideoSourceFFMPEG::getFrame() {
	AVPacket  packet;
	int       frameFinished;
	bool      first_packet=true;

	if (!pFormatCtx) return NULL;


	while (av_read_frame(pFormatCtx,&packet)>=0) {
		if (packet.stream_index==videoStream) {
			if (first_packet) {
				picture_pts=packet.pts;
				if (picture_pts!=0)
					first_packet=false;
			}
			avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);

			if (frameFinished) {
				if ((picture_pts==AV_NOPTS_VALUE) || (picture_pts==0)) picture_pts=pFrame->best_effort_timestamp;
				if ((picture_pts==AV_NOPTS_VALUE) || (picture_pts==0)) picture_pts=pFrame->pts;
				
				sws_scale(pScaler,pFrame->data,pFrame->linesize,0,pFrame->height,pFrameBGR->data,pFrameBGR->linesize);

				av_free_packet(&packet);
				return bgr_image;
			}
		}
		av_free_packet(&packet);
	}
	return NULL;
}
double VideoSourceFFMPEG::num_frames() {
	double frameScale=av_q2d(pFormatCtx->streams[videoStream]->time_base)*av_q2d(pFormatCtx->streams[videoStream]->r_frame_rate);
	return floor(pFormatCtx->streams[videoStream]->duration*frameScale);
}
double VideoSourceFFMPEG::timestamp2frame(int64_t timestamp) {
	AVRational time_base=pFormatCtx->streams[videoStream]->time_base;
    AVRational frame_base=pFormatCtx->streams[videoStream]->r_frame_rate;
	double  timeScale=(time_base.den/(double)time_base.num)/(frame_base.num/frame_base.den);

	if (pFormatCtx->start_time!=AV_NOPTS_VALUE) timestamp-=pFormatCtx->start_time;
	timestamp-=pFormatCtx->streams[videoStream]->first_dts;
	
	return (timestamp*1.0/timeScale);
}
bool VideoSourceFFMPEG::seek2(int64_t timestamp) {
	int flags=AVSEEK_FLAG_FRAME;
	if (timestamp<pFormatCtx->streams[videoStream]->cur_dts) flags|=AVSEEK_FLAG_BACKWARD;
	if (avformat_seek_file(pFormatCtx,videoStream,0,timestamp,timestamp,flags)<0) return false;
	avcodec_flush_buffers(pCodecCtx);
	return true;
}
bool VideoSourceFFMPEG::seek(int64_t frame_n) {
	int64_t timestamp=pFormatCtx->streams[videoStream]->first_dts;
	AVRational time_base=pFormatCtx->streams[videoStream]->time_base;
    AVRational frame_base=pFormatCtx->streams[videoStream]->r_frame_rate;
	double  timeScale=(time_base.den/(double)time_base.num)/(frame_base.num/frame_base.den);
	timestamp+=(int64_t)(frame_n*timeScale);
	if (pFormatCtx->start_time!=AV_NOPTS_VALUE) timestamp+=pFormatCtx->start_time;


	int flags=AVSEEK_FLAG_FRAME;
	if (timestamp<pFormatCtx->streams[videoStream]->cur_dts) flags|=AVSEEK_FLAG_BACKWARD;
	if (avformat_seek_file(pFormatCtx,videoStream,0,timestamp,timestamp,flags)<0) return false;
	avcodec_flush_buffers(pCodecCtx);

	return true;
}




template <class color_type>
AviVideoIn_FFMPEG<color_type>::AviVideoIn_FFMPEG(char *filename,bool use_null_frames) {
	ffmpeg_info=new VideoSourceFFMPEG;

	if (((VideoSourceFFMPEG*)ffmpeg_info)->open(filename)) {
		num_frames=(LONG)((VideoSourceFFMPEG*)ffmpeg_info)->num_frames();
		width=((VideoSourceFFMPEG*)ffmpeg_info)->pCodecCtx->width;
		height=((VideoSourceFFMPEG*)ffmpeg_info)->pCodecCtx->height;
		fps=av_q2d(((VideoSourceFFMPEG*)ffmpeg_info)->pFormatCtx->streams[((VideoSourceFFMPEG*)ffmpeg_info)->videoStream]->r_frame_rate);
		bpp=Bitmap<color_type>::bpp();
		
		// create local image if bpp different than 3
		if (bpp!=3) {
			// TODO**
		}
	} else num_frames=-1;
}

template <class color_type>
AviVideoIn_FFMPEG<color_type>::~AviVideoIn_FFMPEG() {
	((VideoSourceFFMPEG*)ffmpeg_info)->close();
	delete (((VideoSourceFFMPEG*)ffmpeg_info));
	ffmpeg_info=NULL;
}

template <class color_type>
inline void *AviVideoIn_FFMPEG<color_type>::_GetFrame(LONG n) {

	//double frameScale=av_q2d(((VideoSourceFFMPEG*)ffmpeg_info)->pFormatCtx->streams[((VideoSourceFFMPEG*)ffmpeg_info)->videoStream]->time_base);
	

	if (((VideoSourceFFMPEG*)ffmpeg_info)->getTimeStamp()!=n-1) {
		cout<<"ffmpeg: seeking."<<endl;
		((VideoSourceFFMPEG*)ffmpeg_info)->seek2(n);
	}
	int max_skip=1000; // TODO
	IplImage *frame;
	do {
		if (max_skip==0) {
			cout<<"ffmpeg: Frame position inaccurate"<<endl;    // TODO
			break;
		}
		frame=((VideoSourceFFMPEG*)ffmpeg_info)->getFrame();
		max_skip--;
	} while (((VideoSourceFFMPEG*)ffmpeg_info)->getTimeStamp()<n);
	if (((VideoSourceFFMPEG*)ffmpeg_info)->getTimeStamp()>n) {
		cout<<"ffmpeg: Frame position inaccurate"<<endl;        // TODO
	}

	return frame;
}

template <class color_type>
BYTE *AviVideoIn_FFMPEG<color_type>::GetFrameReadOnly_(LONG n) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	//if (frame==NULL) return NULLBITMAP_FRAME;

	return (BYTE *)(frame->imageDataOrigin);
}


template <class color_type>
Bitmap<color_type> *AviVideoIn_FFMPEG<color_type>::GetFrameReadOnly(LONG n) {
	IplImage *frame=(IplImage *)_GetFrame(n);
	//if (frame==NULL) return NULLBITMAP_FRAME;

	//Ipl2Bitmap(frame,tmp_buffer);
	//return tmp_buffer;
	return NULL;
}

template <class color_type>
bool AviVideoIn_FFMPEG<color_type>::GetFrame(LONG n,Bitmap<color_type> *img) {
	/*IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return false;

	Ipl2Bitmap(frame,img);
	return true;*/
	return true;
}

template <class color_type>
UINT AviVideoIn_FFMPEG<color_type>::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	/*IplImage *frame=(IplImage *)_GetFrame(n);
	if (frame==NULL) return INVALID_TEXTURE;
	
	return Ipl2Texture(frame,w,options);*/
	return 0;
}


template AviVideoIn_FFMPEG<ColorRGB>;
template AviVideoIn_FFMPEG<ColorRGBA>;
template AviVideoIn_FFMPEG<GreyLevel>;
template AviVideoIn_FFMPEG<ColorFloat>;

#endif
#endif
