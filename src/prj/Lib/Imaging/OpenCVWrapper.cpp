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


HCY_color RGB2HCY(cv::Vec3b colorBGR) {
	// set color are between [0,1]
	HCY_color out;
	float chroma;
	float r = colorBGR[2] / 255.0f, g = colorBGR[1] / 255.0f, b = colorBGR[0] / 255.0f;

	float M = max(max(r, g), b);
	float m = min(min(r, g), b);

	chroma = M - m;																	// chroma belongs to [0,+1]
	out.y = (0.2125f*r) + (0.7154f*g) + (0.0721f*b);								// y belongs to [0,+1]

	float crx = (r - ((g + b) / 2.0f));												// hue crx,cry projected
	float cry = (sqrt(3.0f)*(b - g) / 2.0f);
	float cr = (sqrt((crx*crx) + (cry*cry)));

	if (cr == 0.0f) {
		out.cs = 0.0f;
		out.cc = 0.0f;
	}
	else {
		out.cs = chroma*(crx / cr);				//   (cs,cc) norm belongs to [0,+1]
		out.cc = -chroma*(cry / cr);			//   -> cs,cc belongs to [-1,+1]
	}

	return out;
}
cv::Vec3b RGB2HCY_ub(cv::Vec3b colorBGR) {
	HCY_color c = RGB2HCY(colorBGR);

	c.y = round(c.y*255.0f);
	c.cs = round(((c.cs + 1.0f) / 2.0f)*255.0f);
	c.cc = round(((c.cc + 1.0f) / 2.0f)*255.0f);
	c.y = min(max(c.y, 0.0f), 255.0f);
	c.cs = min(max(c.cs, 0.0f), 255.0f);
	c.cc = min(max(c.cc, 0.0f), 255.0f);

	colorBGR[0] = (unsigned char)c.y;
	colorBGR[1] = (unsigned char)c.cc;
	colorBGR[2] = (unsigned char)c.cs;
	return colorBGR;
}
cv::Vec3w RGB2HCY_us(cv::Vec3b colorBGR) {
	HCY_color c = RGB2HCY(colorBGR);

	c.y = round(c.y*255.0f);
	c.cs = round(((c.cs + 1.0f) / 2.0f)*512.0f);
	c.cc = round(((c.cc + 1.0f) / 2.0f)*512.0f);
	c.y = min(max(c.y, 0.0f), 256.0f);
	c.cs = min(max(c.cs, 0.0f), 512.0f);
	c.cc = min(max(c.cc, 0.0f), 512.0f);

	cv::Vec3w out;
	out[0] = (unsigned short)c.y;
	out[1] = (unsigned short)c.cc;
	out[2] = (unsigned short)c.cs;
	return out;
}


void linecopy(BYTE *src,BYTE *dst,int n_lines,int linesize,int effective_linesize) {
	if (linesize==effective_linesize) memcpy(dst,src,(size_t)(n_lines*linesize));
	else {
		for(int j=0;j<n_lines;j++) {
			memcpy(dst,src,(size_t)(linesize));
			src+=effective_linesize;
			dst+=linesize;
		}
	}
}

void linecopyBTOI(BYTE *src,BYTE *dst,int n_lines,int linesize,int effective_linesize) {
	if (linesize==effective_linesize) memcpy(dst,src,(size_t)(n_lines*linesize));
	else {
		for(int j=0;j<n_lines;j++) {
			memcpy(dst,src,(size_t)(linesize));
			src+=linesize;
			dst+=effective_linesize;
		}
	}
}

template <class color_type>
void Ipl2Bitmap_slow(IplImage *src,Bitmap<color_type> *dst) {
	int w=src->width;
	int h=src->height;
	int dst_ch=Bitmap<color_type>::bpp();
	int src_ch=src->nChannels;
	IplImage *tmp=NULL;
	
	if (!((src_ch==1) && (dst_ch==1))) tmp=cvCreateImage(cvSize(w,h),8,dst_ch);
	switch(src_ch) {
		case 4:
			{
				switch(dst_ch) {
					case 1: 
						cvCvtColor(src,tmp,CV_BGRA2GRAY);
						break;
					case 2: 
						ErrorExit("Two channels images are not supported.");
						break;
					case 3: 
						cvCvtColor(src,tmp,CV_BGRA2RGB);
						break;
					case 4: 
						cvCvtColor(src,tmp,CV_BGRA2RGBA);
						break;
					default:
						ErrorExit("Wrong channel number.");
				};
				break;
			}
		case 3:
			{
				switch(dst_ch) {
					case 1: 
						cvCvtColor(src,tmp,CV_BGR2GRAY);
						break;
					case 2: 
						ErrorExit("Two channels images are not supported.");
						break;
					case 3: 
						cvCvtColor(src,tmp,CV_BGR2RGB);
						break;
					case 4: 
						cvCvtColor(src,tmp,CV_BGR2RGBA);
						break;
					default:
						ErrorExit("Wrong channel number.");
				};
				break;
			}
		case 2:
			{
				ErrorExit("Two channels images are not supported.");
				break;
			}
		case 1:
			{
				switch(dst_ch) {
					case 1: 
						linecopy((BYTE*)src->imageData,(BYTE*)dst->getBuffer(),h,w*dst_ch,src->widthStep);
						return;
						break;
					case 2: 
						ErrorExit("Two channels images are not supported.");
						break;
					case 3: 
						cvCvtColor(src,tmp,CV_GRAY2RGB);
						break;
					case 4: 
						cvCvtColor(src,tmp,CV_GRAY2RGBA);
						break;
					default:
						ErrorExit("Wrong channel number.");
				};
				break;
			}
		default:
			{
				ErrorExit("Wrong channel number.");
				break;
			}
	}

	linecopy((BYTE*)tmp->imageData,(BYTE*)dst->getBuffer(),h,w*dst_ch,tmp->widthStep);
	cvReleaseImage(&tmp);
}

bool Check_Size(IplImage *&src,int w,int h) {
	if ((src->width!=w) || (src->height!=h)) {
		PerformanceWarning("Not explicitly called bitmap resizing.");
		IplImage *tmp=cvCreateImage(cvSize(w,h),src->depth,src->nChannels);
		cvResize(src,tmp,CV_INTER_LINEAR);
		src=tmp;
		return true;
	}
	return false;
}

bool Check_Size(cv::Mat *&src, int w, int h) {
	if ((src->cols != w) || (src->rows != h)) {
		ErrorExit("Not explicitly called bitmap resizing.");
		return true;
	}
	return false;
}

template <>
void Ipl2Bitmap(IplImage *src,Bitmap<GreyLevel> *dst) {
	bool delete_after=Check_Size(src,dst->width,dst->height);

	if (src->nChannels==1) linecopy((BYTE*)src->imageData,(BYTE*)dst->getBuffer(),src->height,src->width*1,src->widthStep);
	else Ipl2Bitmap_slow(src,dst);

	if (delete_after) cvReleaseImage(&src);
}

template <>
void Ipl2Bitmap(IplImage *src,Bitmap<ColorFloat> *dst) {
	PerformanceWarning("Loading a ColorFloat image.");

	Bitmap<GreyLevel> *g_src=new Bitmap<GreyLevel>(src->width,src->height);
	Ipl2Bitmap(src,g_src);
	
	dst->CopyFrom(g_src);
	delete g_src;
}
template <>
void Ipl2Bitmap(IplImage *src,Bitmap<ColorRGB_int> *dst) {
	ErrorExit("Not implemented.");
}
template <>
void Ipl2Bitmap(IplImage *src,Bitmap<WI_Type> *dst) {
	PerformanceWarning("Loading a WI_Type image.");

	Bitmap<GreyLevel> *g_src=new Bitmap<GreyLevel>(src->width,src->height);
	Ipl2Bitmap(src,g_src);
	
	dst->CopyFrom(g_src);
	delete g_src;
}

template <>
void Ipl2Bitmap(IplImage *src,Bitmap<ColorRGB> *dst) {
	bool delete_after=Check_Size(src,dst->width,dst->height);
	int src_ch=src->nChannels;
	
	switch(src_ch) {
		case 3:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->imageData;
				BYTE *d=(BYTE*)dst->getBuffer();
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_s=s;
					for(int i=0;i<w;i++) {
						(*(d++))=(*(tmp_s+2));
						(*(d++))=(*(tmp_s+1));
						(*(d++))=(*(tmp_s));
						tmp_s+=3;
					}
					s+=src->widthStep;
				}
			}
			break;
		case 4:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->imageData;
				BYTE *d=(BYTE*)dst->getBuffer();
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_s=s;
					for(int i=0;i<w;i++) {
						(*(d++))=(*(tmp_s+2));
						(*(d++))=(*(tmp_s+1));
						(*(d++))=(*(tmp_s));
						tmp_s+=4;
					}
					s+=src->widthStep;
				}
			}
			break;
		default:
			Ipl2Bitmap_slow(src,dst);
			break;
	}
	if (delete_after) cvReleaseImage(&src);
}

template <>
void Ipl2Bitmap(IplImage *src,Bitmap<ColorRGBA> *dst) {
	bool delete_after=Check_Size(src,dst->width,dst->height);
	int src_ch=src->nChannels;
	
	switch(src_ch) {
		case 3:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->imageData;
				BYTE *d=(BYTE*)dst->getBuffer();
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_s=s;
					for(int i=0;i<w;i++) {
						(*(d++))=(*(tmp_s+2));
						(*(d++))=(*(tmp_s+1));
						(*(d++))=(*(tmp_s));
						tmp_s+=3;d++;
					}
					s+=src->widthStep;
				}
			}
			break;
		case 4:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->imageData;
				BYTE *d=(BYTE*)dst->getBuffer();
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_s=s;
					for(int i=0;i<w;i++) {
						(*(d++))=(*(tmp_s+2));
						(*(d++))=(*(tmp_s+1));
						(*(d++))=(*(tmp_s));
						(*(d++))=(*(tmp_s+3));
						tmp_s+=4;
					}
					s+=src->widthStep;
				}
			}
			break;
		default:
			Ipl2Bitmap_slow(src,dst);
			break;
	}
	if (delete_after) cvReleaseImage(&src);
}


UINT Ipl2Texture(IplImage *src,RenderWindow<3> *rw,int options) {
   	UINT texture;

	int w=src->width;
	int h=src->height;

	if ((src->nChannels!=3) || (w*3!=src->widthStep)) {
		PerformanceWarning("Loading a texture with number of channels different than 3 or padded by 0.");
		Bitmap<ColorRGB> *tmp=new Bitmap<ColorRGB>(w,h);
		Ipl2Bitmap(src,tmp);
		texture=rw->LoadTexture(tmp,options);
		delete tmp;
		return texture;
	} 

	texture=rw->LoadTexture((BYTE*)src->imageData,w,h,options);
	return texture;
}

template <>
void Bitmap2Ipl(Bitmap<GreyLevel> *src,IplImage *dst) {
	int dst_ch=dst->nChannels;
	
	switch(dst_ch) {
		case 1:
			linecopyBTOI((BYTE*)src->getBuffer(),(BYTE*)dst->imageData,dst->height,dst->width,dst->widthStep);
			break;
		case 3:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->getBuffer();
				BYTE *d=(BYTE*)dst->imageData;
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_d=d;
					for(int i=0;i<w;i++) {
						(*(tmp_d++))=(*s);
						(*(tmp_d++))=(*s);
						(*(tmp_d++))=(*s);
						s++;
					}
					d+=dst->widthStep;
				}
			}
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}

}

template <>
void Bitmap2Ipl(Bitmap<ColorRGB> *src,IplImage *dst) {
	int dst_ch=dst->nChannels;
	
	switch(dst_ch) {
		case 1:
			{
				int w=src->width;
				int h=src->height;
				ColorRGB *s=src->getBuffer();
				BYTE *d=(BYTE*)dst->imageData;
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_d=d;
					for(int i=0;i<w;i++,s++,tmp_d++) {
						int tmp_i=(int)((0.299*s->r)+(0.587*s->g)+(0.114*s->b));
						*tmp_d=(BYTE)tmp_i;
						if (tmp_i>255) *tmp_d=255;
						if (tmp_i<0) *tmp_d=0;
					}
					d+=dst->widthStep;
				}
			}
			break;
		case 3:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->getBuffer();
				BYTE *d=(BYTE*)dst->imageData;
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_d=d;
					for(int i=0;i<w;i++) {
						(*(tmp_d++))=(*(s+2));
						(*(tmp_d++))=(*(s+1));
						(*(tmp_d++))=(*(s));
						s+=3;
					}
					d+=dst->widthStep;
				}
			}
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}

}

template <>
void Bitmap2Ipl(Bitmap<ColorRGBA> *src,IplImage *dst) {
	int dst_ch=dst->nChannels;
	
	switch(dst_ch) {
		case 1:
			ErrorExit("Not implemented.");
			break;
		case 3:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->getBuffer();
				BYTE *d=(BYTE*)dst->imageData;
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_d=d;
					for(int i=0;i<w;i++) {
						(*(tmp_d++))=(*(s+2));
						(*(tmp_d++))=(*(s+1));
						(*(tmp_d++))=(*(s));
						s+=4;
					}
					d+=dst->widthStep;
				}
			}
			break;
		case 4:
			{
				int w=src->width;
				int h=src->height;
				BYTE *s=(BYTE*)src->getBuffer();
				BYTE *d=(BYTE*)dst->imageData;
				
				for(int j=0;j<h;j++) {
					BYTE *tmp_d=d;
					for(int i=0;i<w;i++) {
						(*(tmp_d++))=(*(s+2));
						(*(tmp_d++))=(*(s+1));
						(*(tmp_d++))=(*(s));
						(*(tmp_d++))=(*(s+3));
						s+=4;
					}
					d+=dst->widthStep;
				}
			}
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}

}

template <>
void Bitmap2Ipl(Bitmap<WI_Type> *src,IplImage *dst) {
	PerformanceWarning("Saving a WI_Type image.");
	Bitmap<GreyLevel> *g_src=new Bitmap<GreyLevel>(src->width,src->height);
	g_src->CopyFrom(src);
	Bitmap2Ipl(g_src,dst);
	delete g_src;
}

template <>
void Bitmap2Ipl(Bitmap<ColorRGB_int> *src,IplImage *dst) {
	ErrorExit("Not implemented.");
}

template <>
void Bitmap2Ipl(Bitmap<ColorFloat> *src,IplImage *dst) {
	switch(dst->depth) {
		case IPL_DEPTH_8U:
		case IPL_DEPTH_8S:
			{
				Warning("PROBLEM: Converting a ColorFloat image into an IPL image. It adjust automatically the colors!!!");
				Bitmap<GreyLevel> *g_src=new Bitmap<GreyLevel>(src->width,src->height);
				g_src->CopyFrom(src);
				Bitmap2Ipl(g_src,dst);
				delete g_src;
			}
			break;
		case IPL_DEPTH_32F:
			// TODO: Controllare
			linecopyBTOI((BYTE*)src->getBuffer(),(BYTE*)dst->imageData,src->height,src->width*sizeof(float),dst->widthStep);
			break;
		default:
			ErrorExit("Not implemented.");
			break;
	}
}



















template <class color_type>
void LoadImageOCV(char *fileName,Bitmap<color_type> *c,bool force_load_greyscale) {
#ifdef USE_OPENGL_LIBRARY
	IplImage *img;

	if (force_load_greyscale) img=cvLoadImage(fileName,CV_LOAD_IMAGE_GRAYSCALE);
	else img=cvLoadImage(fileName,CV_LOAD_IMAGE_COLOR);
	
	if (img==NULL) return;

	c->width=img->width;
	c->height=img->height;
	c->setVirtual(false);
	c->setBuffer(new color_type[(c->width)*(c->height)]);

	Ipl2Bitmap(img,c);
	cvReleaseImage(&img);


#else
	ErrorExit("Not implemented (missing libraries).");
#endif
}

template <>
void SaveImageOCV(char *fileName,Bitmap<ColorRGBA> *c,bool save_grayscale) {
#ifdef USE_OPENGL_LIBRARY
	IplImage *img;

	if (save_grayscale) img=cvCreateImage(cvSize(c->width,c->height),8,1);
	else img=cvCreateImage(cvSize(c->width,c->height),8,4);

	Bitmap2Ipl(c,img);
	cvSaveImage(fileName,img);
	cvReleaseImage(&img);


#else
	ErrorExit("Not implemented (missing libraries).");
#endif
}

template <class color_type>
void SaveImageOCV(char *fileName,Bitmap<color_type> *c,bool save_grayscale) {
#ifdef USE_OPENGL_LIBRARY
	IplImage *img;

	if (save_grayscale) img=cvCreateImage(cvSize(c->width,c->height),8,1);
	else img=cvCreateImage(cvSize(c->width,c->height),8,3);

	Bitmap2Ipl(c,img);
	cvSaveImage(fileName,img);
	cvReleaseImage(&img);


#else
	ErrorExit("Not implemented (missing libraries).");
#endif
}

template <class color_type>
Bitmap<color_type> *Resize(Bitmap<color_type> *img,int w,int h,int interpolation_type) {
#ifdef USE_OPENGL_LIBRARY
	IplImage *iimg,*iimg2;
	Bitmap<color_type> *img2;

	if (img->isgrayscale()) {
		iimg=cvCreateImage(cvSize(img->width,img->height),8,1);
		iimg2=cvCreateImage(cvSize(w,h),8,1);
	} else {
		iimg=cvCreateImage(cvSize(img->width,img->height),8,3);
		iimg2=cvCreateImage(cvSize(w,h),8,3);
	}

	int in_type=CV_INTER_NN;
	switch(interpolation_type) {
		case INTERPOLATION_NN: in_type=CV_INTER_NN;break;
		case INTERPOLATION_LINEAR: in_type=CV_INTER_LINEAR;break;
		case INTERPOLATION_CUBIC: in_type=CV_INTER_CUBIC;break;
		case INTERPOLATION_AREA: in_type=CV_INTER_AREA;break;
	}

	Bitmap2Ipl(img,iimg);
	cvResize(iimg,iimg2,in_type);
	cvReleaseImage(&iimg);
	img2=new Bitmap<color_type>(w,h);
	Ipl2Bitmap(iimg2,img2);
	cvReleaseImage(&iimg2);
	return img2;
#else
	ErrorExit("Not implemented (missing libraries).");
	return NULL;
#endif
}

template void LoadImageOCV<ColorRGB>(char *fileName,Bitmap<ColorRGB> *c,bool force_load_greyscale);
template void LoadImageOCV<ColorRGBA>(char *fileName,Bitmap<ColorRGBA> *c,bool force_load_greyscale);
template void LoadImageOCV<GreyLevel>(char *fileName,Bitmap<GreyLevel> *c,bool force_load_greyscale);
template void LoadImageOCV<ColorFloat>(char *fileName,Bitmap<ColorFloat> *c,bool force_load_greyscale);
template void LoadImageOCV<ColorRGB_int>(char *fileName,Bitmap<ColorRGB_int> *c,bool force_load_greyscale);
template void LoadImageOCV<WI_Type>(char *fileName,Bitmap<WI_Type> *c,bool force_load_greyscale);
template void LoadImageOCV<ColorInt>(char *fileName,Bitmap<ColorInt> *c,bool force_load_greyscale);

template void SaveImageOCV<ColorRGB>(char *fileName,Bitmap<ColorRGB> *c,bool save_grayscale);
//template void SaveImageOCV<ColorRGBA>(char *fileName,Bitmap<ColorRGBA> *c,bool save_grayscale);
template void SaveImageOCV<GreyLevel>(char *fileName,Bitmap<GreyLevel> *c,bool save_grayscale);
template void SaveImageOCV<ColorFloat>(char *fileName,Bitmap<ColorFloat> *c,bool save_grayscale);
template void SaveImageOCV<ColorRGB_int>(char *fileName,Bitmap<ColorRGB_int> *c,bool save_grayscale);
template void SaveImageOCV<WI_Type>(char *fileName,Bitmap<WI_Type> *c,bool save_grayscale);
template void SaveImageOCV<ColorInt>(char *fileName,Bitmap<ColorInt> *c,bool save_grayscale);

template Bitmap<ColorRGB> *Resize(Bitmap<ColorRGB> *img,int w,int h,int interpolation_type);
template Bitmap<ColorRGBA> *Resize(Bitmap<ColorRGBA> *img,int w,int h,int interpolation_type);
template Bitmap<GreyLevel> *Resize(Bitmap<GreyLevel> *img,int w,int h,int interpolation_type);
template Bitmap<ColorFloat> *Resize(Bitmap<ColorFloat> *img,int w,int h,int interpolation_type);



























//
// Matrices
//
CvMat *CreateMatFromMatrix(Matrix *m) {
	CvMat *r=cvCreateMat(m->r,m->c,CV_64F);
	for(int i=0;i<m->r;i++) {
		for(int j=0;j<m->c;j++) cvmSet(r,i,j,(m->v[i][j]));
	}
	return r;
}
Matrix *CreateMatrixFromMat(CvMat *m) {
	Matrix *r=new Matrix(m->rows,m->cols);
	for(int i=0;i<r->r;i++) {
		for(int j=0;j<r->c;j++) r->v[i][j]=cvmGet(m,i,j);
	}
	return r;
}
void GetMatrixFromMat(CvMat *m,Matrix *out) {
	if ((out->r!=m->rows) || (out->c!=m->cols)) ErrorExit("Incompatible matrices.");
	for(int i=0;i<out->r;i++) {
		for(int j=0;j<out->c;j++) out->v[i][j]=cvmGet(m,i,j);
	}
}
void GetMatFromMatrix(Matrix *m,CvMat *out) {
	if ((m->r!=out->rows) || (m->c!=out->cols)) ErrorExit("Incompatible matrices.");
	for(int i=0;i<m->r;i++) {
		for(int j=0;j<m->c;j++) cvmSet(out,i,j,(m->v[i][j]));
	}
}
CvMat *CreateMatFromArray(float *v,int num) {
	CvMat *r=cvCreateMat(1,num,CV_64F);
	for(int j=0;j<num;j++) cvmSet(r,0,j,(v[j]));
	return r;
}































//
// Distance Transform
//
SilhouetteDistance_Buffer::SilhouetteDistance_Buffer(int w,int h) {
	ib=ia=NULL;
	iib=iia=NULL;

	ia=cvCreateImage(cvSize(w,h),8,1);
	ib=cvCreateImage(cvSize(w,h),8,1);
	iia=cvCreateImage(cvSize(w,h),32,1);
	iib=cvCreateImage(cvSize(w,h),32,1);
}
SilhouetteDistance_Buffer::~SilhouetteDistance_Buffer() {
	if (!ia) cvReleaseImage(&ia);
	if (!ib) cvReleaseImage(&ib);
	if (!iia) cvReleaseImage(&iia);
	if (!iib) cvReleaseImage(&iib);
}
void SilhouetteDistance_Buffer::PrepareImageA(Bitmap<GreyLevel> *a,bool invert_image,int BinaryThreshold) {
	Bitmap2Ipl(a,ia);
	if (BinaryThreshold!=-1) {
		if (invert_image) cvThreshold(ia,ia,BinaryThreshold,255,CV_THRESH_BINARY_INV);
		else cvThreshold(ia,ia,BinaryThreshold,255,CV_THRESH_BINARY);
	} else {
		if (invert_image) cvThreshold(ia,ia,128,255,CV_THRESH_BINARY_INV);
	}
	cvDistTransform(ia,iia,CV_DIST_L2,3,NULL,NULL);
}
void SilhouetteDistance_Buffer::PrepareImageB(Bitmap<GreyLevel> *b,bool invert_image,int BinaryThreshold) {
	Bitmap2Ipl(b,ib);
	if (BinaryThreshold!=-1) {
		if (invert_image) cvThreshold(ib,ib,BinaryThreshold,255,CV_THRESH_BINARY_INV);
		else cvThreshold(ib,ib,BinaryThreshold,255,CV_THRESH_BINARY);
	} else {
		if (invert_image) cvThreshold(ib,ib,128,255,CV_THRESH_BINARY_INV);
	}
	cvDistTransform(ib,iib,CV_DIST_L2,3,NULL,NULL);
}















/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// SilhouetteDistanceST
/////////////    SINGLE PROCESSING
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double SilhouetteDistanceST(Bitmap<GreyLevel> *a,Bitmap<GreyLevel> *b,int BinaryThreshold,SilhouetteDistance_Buffer *st,int *n_observations) {
#ifdef USE_OPENGL_LIBRARY
	// Convert input data
	int w=a->width;
	int h=a->height;
	if ((w!=b->width) || (h!=b->height)) ErrorExit("Image size don't match.");
	char *index_p,*index_row,*data_p;
	float *data_row;


	Bitmap2Ipl(a,st->ia);
	Bitmap2Ipl(b,st->ib);
	cvThreshold(st->ia,st->ia,BinaryThreshold,255,CV_THRESH_BINARY_INV);
	cvThreshold(st->ib,st->ib,BinaryThreshold,255,CV_THRESH_BINARY_INV);
	cvDistTransform(st->ia,st->iia,CV_DIST_L2,3,NULL,NULL);
	cvDistTransform(st->ib,st->iib,CV_DIST_L2,3,NULL,NULL);

	//
	// Distanza	di A da B    ->   0 is the foreground, 255 is the background
	//
	double sumA2B=0;
	int number_of_foregroundA2B=0;
	
	index_p=st->ia->imageData;
	data_p =st->iib->imageData;
	for(int j=0;j<h;j++,index_p+=st->ia->widthStep,data_p+=st->iib->widthStep) {
		
		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=0;i<w;i++,index_row++,data_row++) {

			if (*index_row==0) {
				sumA2B+=*data_row;
				number_of_foregroundA2B++;
			}
		}
	}
	sumA2B/=number_of_foregroundA2B;

	//
	// Distanza	di B da A
	//
	double sumB2A=0;
	int number_of_foregroundB2A=0;

	index_p=st->ib->imageData;
	data_p =st->iia->imageData;
	for(int j=0;j<h;j++,index_p+=st->ib->widthStep,data_p+=st->iia->widthStep) {

		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=0;i<w;i++,index_row++,data_row++) {

			if (*index_row==0) {
				sumB2A+=*data_row;
				number_of_foregroundB2A++;
			}
		}
	}
	sumB2A/=number_of_foregroundB2A;

	if (n_observations) (*n_observations)=number_of_foregroundB2A+number_of_foregroundA2B;
	return ((sumB2A+sumA2B)/2.0);

#else
	ErrorExit("Not implemented (missing libraries).");
	return 0.0;
#endif
}


double SilhouetteDistanceST(SilhouetteDistance_Buffer *st,int *n_observations) {
#ifdef USE_OPENGL_LIBRARY
	// Convert input data
	int w=st->ia->width;
	int h=st->ia->height;
	char *index_p,*index_row,*data_p;
	float *data_row;



	//
	// Distanza	di A da B    ->   0 is the foreground, 255 is the background
	//
	double sumA2B=0;
	int number_of_foregroundA2B=0;
	
	index_p=st->ia->imageData;
	data_p =st->iib->imageData;
	for(int j=0;j<h;j++,index_p+=st->ia->widthStep,data_p+=st->iib->widthStep) {
		
		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=0;i<w;i++,index_row++,data_row++) {

			if (*index_row==0) {
				sumA2B+=*data_row;
				number_of_foregroundA2B++;
			}
		}
	}
	sumA2B/=number_of_foregroundA2B;



	//
	// Distanza	di B da A
	//
	double sumB2A=0;
	int number_of_foregroundB2A=0;

	index_p=st->ib->imageData;
	data_p =st->iia->imageData;
	for(int j=0;j<h;j++,index_p+=st->ib->widthStep,data_p+=st->iia->widthStep) {

		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=0;i<w;i++,index_row++,data_row++) {

			if (*index_row==0) {
				sumB2A+=*data_row;
				number_of_foregroundB2A++;
			}
		}
	}
	sumB2A/=number_of_foregroundB2A;

	if (n_observations) (*n_observations)=number_of_foregroundB2A+number_of_foregroundA2B;
	return ((sumB2A+sumA2B)/2.0);

#else
	ErrorExit("Not implemented (missing libraries).");
	return 0.0;
#endif
}










/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// SilhouetteDistanceMT
/////////////    MULTI PROCESSING
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct silhouette_parallel_data {
	Bitmap<GreyLevel> *a;
	SilhouetteDistance_Buffer *st;
	int BinaryThreshold;
	int w,h;
	double sumB2A;
	int number_of_foregroundB2A;
};

DWORD silhouette_parallel_task_s(silhouette_parallel_data *data) {
	//
	// Distanza	di B da A
	//
	char *index_p,*index_row,*data_p;
	float *data_row;

	double sumB2A=0.0;
	int number_of_foregroundB2A=0;

	index_p=data->st->ib->imageData;
	data_p =data->st->iia->imageData;
	int i_ws=data->st->ib->widthStep;
	int d_ws=data->st->iia->widthStep;
	for(int j=data->h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {

		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=data->w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumB2A+=*data_row;
				number_of_foregroundB2A++;
			}
		}
	}
	data->sumB2A=sumB2A/number_of_foregroundB2A;
	data->number_of_foregroundB2A=number_of_foregroundB2A;

	return 0;
}


double SilhouetteDistanceMT(SilhouetteDistance_Buffer *st,int *n_observations) {
#ifdef USE_OPENGL_LIBRARY
	// Convert input data
	int w=st->ia->width;
	int h=st->ia->height;
	char *index_p,*index_row,*data_p;
	float *data_row;


	silhouette_parallel_data data;
	data.a=NULL;
	data.st=st;
	data.BinaryThreshold=-1;
	data.w=w;
	data.h=h;
	DWORD dwThreadId;HANDLE hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)silhouette_parallel_task_s,&data,0,&dwThreadId);


	//
	// Distanza	di A da B    ->   0 is the foreground, 255 is the background
	//
	double sumA2B=0;
	int number_of_foregroundA2B=0;
	
	index_p=st->ia->imageData;
	data_p =st->iib->imageData;
	int i_ws=st->ia->widthStep;
	int d_ws=st->iib->widthStep;
	for(int j=h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {
			
		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumA2B+=*data_row;
				number_of_foregroundA2B++;
			}

		}

	}
	sumA2B/=number_of_foregroundA2B;

	WAIT_EVENT(hThread);
	CloseHandle(hThread);

	if (n_observations) (*n_observations)=data.number_of_foregroundB2A+number_of_foregroundA2B;
	return ((data.sumB2A+sumA2B)/2.0);

#else
	ErrorExit("Not implemented (missing libraries).");
	return 0.0;
#endif
}


























/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////// SilhouetteDistanceMT_MSE
/////////////    MULTI PROCESSING
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DWORD silhouette_parallel_task_s_mse(silhouette_parallel_data *data) {
	//
	// Distanza	di B da A
	//
	char *index_p,*index_row,*data_p;
	float *data_row;

	double sumB2A=0.0;
	int number_of_foregroundB2A=0;

	index_p=data->st->ib->imageData;
	data_p =data->st->iia->imageData;
	int i_ws=data->st->ib->widthStep;
	int d_ws=data->st->iia->widthStep;
	for(int j=data->h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {

		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=data->w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumB2A+=(*data_row)*(*data_row);
				number_of_foregroundB2A++;
			}
		}
	}
	data->sumB2A=sumB2A/number_of_foregroundB2A;
	data->number_of_foregroundB2A=number_of_foregroundB2A;

	return 0;
}


double SilhouetteDistanceMT_mse(SilhouetteDistance_Buffer *st,int *n_observations) {
#ifdef USE_OPENGL_LIBRARY
	// Convert input data
	int w=st->ia->width;
	int h=st->ia->height;
	char *index_p,*index_row,*data_p;
	float *data_row;


	silhouette_parallel_data data;
	data.a=NULL;
	data.st=st;
	data.BinaryThreshold=-1;
	data.w=w;
	data.h=h;
	DWORD dwThreadId;HANDLE hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)silhouette_parallel_task_s_mse,&data,0,&dwThreadId);


	//
	// Distanza	di A da B    ->   0 is the foreground, 255 is the background
	//
	double sumA2B=0;
	int number_of_foregroundA2B=0;
	
	index_p=st->ia->imageData;
	data_p =st->iib->imageData;
	int i_ws=st->ia->widthStep;
	int d_ws=st->iib->widthStep;
	for(int j=h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {
			
		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumA2B+=(*data_row)*(*data_row);
				number_of_foregroundA2B++;
			}

		}

	}
	sumA2B/=number_of_foregroundA2B;

	WAIT_EVENT(hThread);
	CloseHandle(hThread);

	if (n_observations) (*n_observations)=data.number_of_foregroundB2A+number_of_foregroundA2B;
	return ((data.sumB2A+sumA2B)/2.0);

#else
	ErrorExit("Not implemented (missing libraries).");
	return 0.0;
#endif
}








template <>
void Ipl2Bitmap(cv::Mat *src, Bitmap<ColorRGB> *dst) {
	Check_Size(src, dst->width, dst->height);
	int src_ch = src->channels();

	switch (src_ch) {
	case 3:
	{
		int w = src->cols;
		int h = src->rows;
		BYTE *d = (BYTE*)dst->getBuffer();

		for (int j = 0; j<h; j++) {
			cv::Vec3b *Ip = src->ptr<cv::Vec3b>(j);
			for (int i = 0; i<w; i++) {
				(*(d++)) = (*Ip)[2];
				(*(d++)) = (*Ip)[1];
				(*(d++)) = (*Ip)[0];
				Ip++;
			}
		}
	}
	break;
	default:
		ErrorExit("Not implemented");
		break;
	}

}

template <>
void Ipl2Bitmap(cv::Mat *src, Bitmap<GreyLevel> *dst) {
	Check_Size(src, dst->width, dst->height);
	int src_ch = src->channels();

	switch (src_ch) {
	case 1:
	{
		int w = src->cols;
		int h = src->rows;
		BYTE *d = (BYTE*)dst->getBuffer();

		for (int j = 0; j<h; j++) {
			BYTE *Ip = src->ptr<BYTE>(j);
			for (int i = 0; i<w; i++) {
				(*(d++)) = (*(Ip++));
			}
		}
	}
		break;
	default:
		ErrorExit("Not implemented");
		break;
	}

}





















/*
DWORD silhouette_parallel_task(silhouette_parallel_data *data) {
	cvDistTransform(data->st->ia,data->st->iia,CV_DIST_L2,3,NULL,NULL);

	char *index_p,*index_row,*data_p;
	float *data_row;

	double sumB2A=0.0;
	int number_of_foregroundB2A=0;

	index_p=data->st->ib->imageData;
	data_p =data->st->iia->imageData;
	int i_ws=data->st->ib->widthStep;
	int d_ws=data->st->iia->widthStep;
	for(int j=data->h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {

		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=data->w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumB2A+=*data_row;
				number_of_foregroundB2A++;
			}
		}
	}
	data->sumB2A=sumB2A/number_of_foregroundB2A;
	data->number_of_foregroundB2A=number_of_foregroundB2A;

	return 0;
}

double SilhouetteDistance(Bitmap<GreyLevel> *a,Bitmap<GreyLevel> *b,int BinaryThreshold,SilhouetteDistance_Buffer *st) {
#ifdef USE_OPENGL_LIBRARY	
	// Convert input data
	int w=a->width;
	int h=a->height;
	if ((w!=b->width) || (h!=b->height)) ErrorExit("Image size don't match.");
	char *index_p,*index_row,*data_p;
	float *data_row;


	Bitmap2Ipl(a,st->ia);
	Bitmap2Ipl(b,st->ib);
	cvThreshold(st->ia,st->ia,BinaryThreshold,255,CV_THRESH_BINARY_INV);
	cvThreshold(st->ib,st->ib,BinaryThreshold,255,CV_THRESH_BINARY_INV);


	silhouette_parallel_data data;
	data.a=a;
	data.st=st;
	data.BinaryThreshold=BinaryThreshold;
	data.w=w;
	data.h=h;
	DWORD dwThreadId;HANDLE hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)silhouette_parallel_task,&data,0,&dwThreadId);

	
	cvDistTransform(st->ib,st->iib,CV_DIST_L2,3,NULL,NULL);

	double sumA2B=0;
	int number_of_foregroundA2B=0;
	
	index_p=st->ia->imageData;
	data_p =st->iib->imageData;
	int i_ws=st->ia->widthStep;
	int d_ws=st->iib->widthStep;
	for(int j=h;j>0;j--,index_p+=i_ws,data_p+=d_ws) {
			
		index_row=index_p;
		data_row=(float *)data_p;
		for(int i=w;i>0;i--,index_row++,data_row++) {

			if (*index_row==0) {
				sumA2B+=*data_row;
				number_of_foregroundA2B++;
			}

		}

	}
	sumA2B/=number_of_foregroundA2B;

	WAIT_EVENT(hThread);
	CloseHandle(hThread);

	return ((data.sumB2A+sumA2B)/2.0);

#else
	ErrorExit("Not implemented (missing libraries).");
	return 0.0;
#endif
}
*/
