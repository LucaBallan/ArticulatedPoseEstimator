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


EdgeDetector::EdgeDetector(AdvancedRenderWindow *gpu_obj,int width,int height) {
	// Init
	fx   = NULL;
	fy   = NULL;
	norm = NULL;
	sx   = NULL;
	sy   = NULL;
	weak_set1 = NULL;
	weak_set2 = NULL;
	for(int i=0;i<7;i++) Float_Texture_Array[i]=INVALID_TEXTURE;


	this->gpu_obj=gpu_obj;
	IR=new ImageProcessingRenderer(gpu_obj);
	ffp=gpu_obj->CreateFrameBuffer(width,height,Float_Texture_Array,7,TEXTURE_CH_32,0,false);


	SNEW(fx   ,Bitmap<ColorFloat>(width,height));
	SNEW(fy   ,Bitmap<ColorFloat>(width,height));
	SNEW(norm ,Bitmap<ColorFloat>(width,height));
	weak_set1 = cvCreateImage(cvSize(width,height),8,1);
	weak_set2 = cvCreateImage(cvSize(width,height),8,1);

	int estimated_array_size=(int)(width*height*0.03);
	SNEW(sx,Array<int>(estimated_array_size));
	SNEW(sy,Array<int>(estimated_array_size));
}

EdgeDetector::~EdgeDetector() {
	gpu_obj->DeleteFrameBuffer(&ffp);
	SDELETE(IR);

	for(int i=0;i<7;i++) gpu_obj->DeleteTexture(Float_Texture_Array[i]);
	SDELETE(fx);
	SDELETE(fy);
	SDELETE(norm);
	SDELETE(sx);
	SDELETE(sy);
	cvReleaseImage(&weak_set1);
	cvReleaseImage(&weak_set2);
}


#define UPDATE_WEAK_STRONG_EDGES																		\
				if (((*in)>=gradmag1) && ((*in)>=gradmag2)) {											\
					if ((*in)>low_threshold) {															\
						ws[j+i*weak_set1->widthStep]=0xFF;												\
						if ((accurate_method) && ((*in)>high_threshold)) {								\
							sx->append(j);																\
							sy->append(i);																\
						}																				\
					}																					\
				}


void EdgeDetector::Canny_Get(UINT image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold) {
	int filter_width=gaussian_min_width_from_sigma(sigma,0.004);
	bool accurate_method=(low_threshold!=high_threshold)?true:false;

	// Smooth
	int filter_semi_width=(filter_width-1)/2;
	float *filter_gau=new float[filter_width];
	for(int j=0;j<filter_width;j++) filter_gau[j]=(float)(exp(-((j-filter_semi_width)*(j-filter_semi_width))/(2*sigma*sigma))/(2*M_PI*sigma*sigma));
	IR->FilterImage(&ffp,image,filter_gau,filter_width,0,NULL);
	delete[]filter_gau;

	
	// Compute derivatives
	Matrix filter_d(filter_width,filter_width);
	for(int i=0;i<filter_width;i++)
		for(int j=0;j<filter_width;j++) 
			filter_d[j][i]=-(i-filter_semi_width)*exp(-((i-filter_semi_width)*(i-filter_semi_width)+(j-filter_semi_width)*(j-filter_semi_width))/(2*sigma*sigma))/(M_PI*sigma*sigma);
	IR->NormFxFyFilterImage(&ffp,Float_Texture_Array[0],&filter_d,1,2,3,fx,fy,norm);
	// Status:
	//		Float_Texture_Array[0]       Filtered image
	//		Float_Texture_Array[1,2,3]   fx, fy, norm (not normalized)


	// Normalize norm
	float *buff=norm->getBuffer();
	int    size=norm->width*norm->height;
	float  max=0.0;
	for(int i=0;i<size;i++,buff++) max=MAX((*buff),max);
	buff=norm->getBuffer();
	if (max!=0.0) for(int i=0;i<size;i++,buff++) (*buff)/=max;

	
	// non-maximum suppression
	cvSetZero(weak_set1);
	sx->clear();
	sy->clear();

	unsigned char *ws=(unsigned char *)(weak_set1->imageData);
	float *ix=fx->getBuffer()+norm->width;
	float *iy=fy->getBuffer()+norm->width;
	float *in=norm->getBuffer()+norm->width;

	for(int i=1;i<norm->height-1;i++) {
		ix++;iy++;in++;
		
		for(int j=1;j<norm->width-1;j++,ix++,iy++,in++) {
			
			if ((((*iy)<=0.0) && ((*ix)>-(*iy))) || (((*iy)>=0.0) && ((*ix)<-(*iy)))) {
				float d = abs((*iy)/(*ix));
				float gradmag1 = (*(in+1))*(1-d)+(*(in+1-norm->width))*d;
				float gradmag2 = (*(in-1))*(1-d)+(*(in-1+norm->width))*d;
				UPDATE_WEAK_STRONG_EDGES;
			}
			
			if ((((*ix)>0.0) && (-(*iy)>=(*ix))) || (((*ix)<0.0) && (-(*iy)<=(*ix)))) {
				float d = abs((*ix)/(*iy));
				float gradmag1 = (*(in-norm->width))*(1-d)+(*(in+1-norm->width))*d;
				float gradmag2 = (*(in+norm->width))*(1-d)+(*(in-1+norm->width))*d;
				UPDATE_WEAK_STRONG_EDGES;
			}
			
			if ((((*ix)<=0.0) && ((*ix)>(*iy))) || (((*ix)>=0.0) && ((*ix)<(*iy)))) {
				float d = abs((*ix)/(*iy));
				float gradmag1 = (*(in-norm->width))*(1-d)+(*(in-1-norm->width))*d;
				float gradmag2 = (*(in+norm->width))*(1-d)+(*(in+1+norm->width))*d;
				UPDATE_WEAK_STRONG_EDGES;				
			}
			
			if ((((*iy)<0.0) && ((*ix)<=(*iy))) || (((*iy)>0.0) && ((*ix)>=(*iy)))) {
				float d = abs((*iy)/(*ix));
				float gradmag1 = (*(in-1))*(1-d)+(*(in-1-norm->width))*d;
				float gradmag2 = (*(in+1))*(1-d)+(*(in+1+norm->width))*d;
				UPDATE_WEAK_STRONG_EDGES;
			}
		}
		ix++;iy++;in++;
	}



	if (accurate_method) {
		cvCopy(weak_set1,weak_set2);
	
		int num_seeds=sx->numElements();
		int *isx=sx->getMem();
		int *isy=sy->getMem();
		for(int i=0;i<num_seeds;i++,isx++,isy++) {
			CvPoint p=cvPoint((*isx),(*isy));
			cvFloodFill(weak_set2,p,cvScalar(0),cvScalar(0),cvScalar(0),NULL,8);
		}


		BYTE *dst_buffer=dst->getBuffer();
		unsigned char *ws1_row=(unsigned char *)(weak_set1->imageData);
		unsigned char *ws2_row=(unsigned char *)(weak_set2->imageData);
		for(int i=0;i<dst->height;i++) {
			unsigned char *ws1=ws1_row;
			unsigned char *ws2=ws2_row;
			
			for(int j=0;j<dst->width;j++,dst_buffer++,ws1++,ws2++) {
				(*dst_buffer)|=(*ws1)&(~(*ws2));
			}
			
			ws1_row+=weak_set1->widthStep;
			ws2_row+=weak_set2->widthStep;
		}

	} else {
		BYTE *dst_buffer=dst->getBuffer();
		unsigned char *ws1_row=(unsigned char *)(weak_set1->imageData);
		for(int i=0;i<dst->height;i++) {
			unsigned char *ws1=ws1_row;
			
			for(int j=0;j<dst->width;j++,dst_buffer++,ws1++) {
				(*dst_buffer)|=(*ws1);
			}
			
			ws1_row+=weak_set1->widthStep;
		}
	}
}


int  edge_sigma;
int  edge_high_threshold;
int  edge_low_threshold;
bool edge_equal_th;
Bitmap<GreyLevel> *edge_output;
IplImage          *edge_outputIpl;
UINT			   edge_input_image;
EdgeDetector      *edge_cur;

void CannyGreyInteractiveUpdate(int pos) {
	if (edge_equal_th) edge_low_threshold=edge_high_threshold;
	edge_cur->CannyGrey(edge_input_image,edge_output,edge_sigma/10.0f,edge_high_threshold/100.0f,edge_low_threshold/100.0f,0);
	Bitmap2Ipl(edge_output,edge_outputIpl);
	cvShowImage("Edge Detector",edge_outputIpl);
}

void EdgeDetector::CannyGreyInteractive(UINT input_image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold) {
	edge_sigma=Approx(sigma*10.0);
	edge_high_threshold=Approx(high_threshold*100.0);
	edge_low_threshold=Approx(low_threshold*100.0);
	edge_equal_th=true;
	if (edge_equal_th) edge_low_threshold=edge_high_threshold;

	IplImage *outputIpl;
	outputIpl=cvCreateImage(cvSize(dst->width,dst->height),8,1);
	edge_output=dst;
	edge_outputIpl=outputIpl;
	edge_input_image=input_image;
	edge_cur=this;

	cvNamedWindow("Edge Detector",CV_WINDOW_AUTOSIZE);
	cvCreateTrackbar("sigma*10","Edge Detector",&edge_sigma,14,CannyGreyInteractiveUpdate);
	cvCreateTrackbar("th*100","Edge Detector",&edge_high_threshold,255,CannyGreyInteractiveUpdate);
	CannyGreyInteractiveUpdate(0);
	cvWaitKey();

	sigma=edge_sigma/10.0f;
	high_threshold=edge_high_threshold/100.0f;
	low_threshold=edge_low_threshold/100.0f;
	CannyGrey(input_image,dst,sigma,high_threshold,low_threshold,0);

	cvDestroyWindow("Edge Detector");
	cvReleaseImage(&outputIpl);
}

void EdgeDetector::CannyGrey(UINT input_image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold,int interactive) {
	if (low_threshold==-1) low_threshold=high_threshold*0.4F;
	if (interactive) return CannyGreyInteractive(input_image,dst,sigma,high_threshold,low_threshold);

	dst->Clear();
	Canny_Get(input_image,dst,sigma,high_threshold,low_threshold);
}
void EdgeDetector::Canny(Bitmap<GreyLevel> *src,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold) {
	if ((src->width!=dst->width) || (src->height!=dst->height)) ErrorExit("Image sizes do not match.");
	if (low_threshold==-1) low_threshold=high_threshold*0.4F;

	UINT input_image=gpu_obj->LoadTexture(src);
	dst->Clear();
	Canny_Get(input_image,dst,sigma,high_threshold,low_threshold);
	gpu_obj->DeleteTexture(input_image);
}

void EdgeDetector::Canny(Bitmap<ColorRGB>  *src,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold) {
	if ((src->width!=dst->width) || (src->height!=dst->height)) ErrorExit("Image sizes do not match.");
	if (low_threshold==-1) low_threshold=high_threshold*0.4F;

	UINT input_image=gpu_obj->LoadTexture(src);
	IR->RGBtoYCbCr(&ffp,input_image,4,5,6,NULL,NULL,NULL);
	gpu_obj->DeleteTexture(input_image);


	dst->Clear();
	Canny_Get(Float_Texture_Array[4],dst,sigma,high_threshold,low_threshold);
	Canny_Get(Float_Texture_Array[5],dst,sigma,high_threshold,low_threshold);
	Canny_Get(Float_Texture_Array[6],dst,sigma,high_threshold,low_threshold);
}



















ImageGradientEstimator::ImageGradientEstimator(AdvancedRenderWindow *gpu_obj,int width,int height) {
	this->gpu_obj=gpu_obj;
	IR=NULL;
	ffp.valid=false;
	for(int i=0;i<4;i++) Float_Texture_Array[i]=INVALID_TEXTURE;
	

	IR=new ImageProcessingRenderer(gpu_obj);
	ffp=gpu_obj->CreateFrameBuffer(width,height,Float_Texture_Array,4,TEXTURE_CH_32,0,false);
}
ImageGradientEstimator::~ImageGradientEstimator() {
	gpu_obj->DeleteFrameBuffer(&ffp);
	SDELETE(IR);
	
	for(int i=0;i<4;i++) gpu_obj->DeleteTexture(Float_Texture_Array[i]);
}
void ImageGradientEstimator::Gradient(Bitmap<GreyLevel> *src,Bitmap<ColorFloat> *fx,Bitmap<ColorFloat> *fy) {
	if ((src->width!=fx->width) || (src->height!=fx->height)) ErrorExit("Image sizes do not match.");
	if ((src->width!=fy->width) || (src->height!=fy->height)) ErrorExit("Image sizes do not match.");

	UINT image=gpu_obj->LoadTexture(src);
	Gradient(image,fx,fy);
	gpu_obj->DeleteTexture(image);
}
void ImageGradientEstimator::Gradient(UINT image,Bitmap<ColorFloat> *fx,Bitmap<ColorFloat> *fy) {
//	filter_width=gaussian_min_width_from_sigma(sigma,0.004); // TODOCOLOR
	
	
	/*
	float *filter_gau=new float[filter_width];
	for(int j=0;j<filter_width;j++) filter_gau[j]=(float)(exp(-((j-filter_semi_width)*(j-filter_semi_width))/(2*sigma*sigma))/(2*M_PI*sigma*sigma));
	IR->FilterImage(&ffp,image,filter_gau,filter_width,0,NULL);
	delete[]filter_gau;
	*/
	
	// TODOCOLOR all this function
	// Compute derivatives
/*
	int filter_width=13;
	int filter_semi_width=(filter_width-1)/2;
	Matrix *filter_d=new Matrix(filter_width,filter_width);
	int p_x,p_y;
	FiniteDifferencesTemplate(-filter_semi_width,filter_semi_width,1,0,1,0,filter_d,p_x,p_y,0.0);
	filter_d->Print();
*/	
	
	
	int filter_width=13;
	double sigma=3;
	filter_width=gaussian_min_width_from_sigma(sigma,0.4); // TODO
	int filter_semi_width=(filter_width-1)/2;

	Matrix *filter_d=new Matrix(filter_width,filter_width);
	
	double sum=0.0;
	for(int i=0;i<filter_width;i++)
		for(int j=0;j<filter_width;j++) {
			(*filter_d)[j][i]=-(i-filter_semi_width)*exp(-((i-filter_semi_width)*(i-filter_semi_width)+(j-filter_semi_width)*(j-filter_semi_width))/(2*sigma*sigma))/(M_PI*sigma*sigma);
			sum+=(*filter_d)[j][i];
		}
	filter_d->Print();
	std::cout<<sum<<std::endl;

	
	IR->NormFxFyFilterImage(&ffp,image,filter_d,0,1,2,fx,fy,NULL);
	SDELETE(filter_d);
	// Status:
	//		Float_Texture_Array[0,1,2]   fx, fy, norm (not normalized)
}


