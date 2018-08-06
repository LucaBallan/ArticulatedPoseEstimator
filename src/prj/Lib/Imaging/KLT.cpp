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



#ifdef USE_OPENGL_LIBRARY


IplImage *Bitmap2Ipl(Bitmap<GreyLevel> *img) {
	int w=img->width;
	int h=img->height;
	IplImage *image=cvCreateImage(cvSize(w,h),8,1);

	for(int j=0;j<h;j++) {
		for(int i=0;i<w;i++) {
			image->imageData[image->widthStep*j+i]=img->Point(i,j);
		}
	}
	
	return image;
}

IplImage *Bitmap2Ipl(Bitmap<ColorRGB> *img) {
	int w=img->width;
	int h=img->height;
	IplImage *image=cvCreateImage(cvSize(w,h),8,3);

	for(int j=0;j<h;j++) {
		for(int i=0;i<w;i++) {
			image->imageData[image->widthStep*j+i*3]=img->Point(i,j).b;
			image->imageData[image->widthStep*j+i*3+1]=img->Point(i,j).g;
			image->imageData[image->widthStep*j+i*3+2]=img->Point(i,j).r;
		}
	}
	
	return image;
}

KLT::KLT(int max_num_points) {
	this->points=new CvPoint2D32f[max_num_points];
	this->old_points=new CvPoint2D32f[max_num_points];
	this->status=new char[max_num_points];
	this->track_error=new float[max_num_points];
	this->max_num_points=max_num_points;
	this->num_points=0;
	
	
}
KLT::~KLT() {
	delete []points;
	delete []old_points;
	delete []status;
	delete []track_error;
}

void KLT::Detect(Bitmap<GreyLevel> *img,int win_size,double quality,double min_dist,HyperBox<2> *mask) {
	IplImage *GreyImage=Bitmap2Ipl(img);

	Detect((void *)GreyImage,win_size,quality,min_dist,mask);

	cvReleaseImage(&GreyImage);
}

void KLT::Detect(Bitmap<ColorRGB> *img,int win_size,double quality,double min_dist,HyperBox<2> *mask) {
	IplImage *ColorImage=Bitmap2Ipl(img);
	IplImage *GreyImage=cvCreateImage(cvGetSize(ColorImage),8,1);
	cvCvtColor(ColorImage,GreyImage,CV_BGR2GRAY);

	Detect((void *)GreyImage,win_size,quality,min_dist,mask);

	cvReleaseImage(&GreyImage);
	cvReleaseImage(&ColorImage);
}

void KLT::Detect(void *img,int win_size,double quality,double min_dist,HyperBox<2> *mask) {
	// TODO: to check and make it faster
	IplImage *GreyImage=(IplImage *)img;
	IplImage *eig=cvCreateImage(cvGetSize(GreyImage),32,1);
	IplImage *temp=cvCreateImage(cvGetSize(GreyImage),32,1);
	
	num_points=max_num_points;
	cvGoodFeaturesToTrack( GreyImage, eig, temp, (CvPoint2D32f*)points, &num_points,
						   quality, min_dist, NULL, win_size, 0, 10);
	
	int win_size_sub = (win_size/2)+1;
	cvFindCornerSubPix( GreyImage, (CvPoint2D32f*)points, num_points,
		cvSize(win_size_sub,win_size_sub), cvSize(-1,-1),
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
	

	if (mask) {
		CvPoint2D32f* p=(CvPoint2D32f*)points;
		int out_num=0;
		for(int i=0;i<num_points;i++) {
			bool del=false;
			if (p[i].x>mask->P1[0]) del=true;
			if (p[i].x<mask->P0[0]) del=true;
			if (p[i].y>mask->P1[1]) del=true;
			if (p[i].y<mask->P0[1]) del=true;
			
			p[out_num]=p[i];
			if (!del) out_num++;
		}
		num_points=out_num;
	}


	cvReleaseImage(&eig);
	cvReleaseImage(&temp);
}

void KLT::SetPointList(Array<Vector<2>> *points) {
	if (points->numElements()>max_num_points) ErrorExit("Not enought points were allocated.");
	this->num_points=points->numElements();

	CvPoint2D32f* p=(CvPoint2D32f*)this->points;
	for(int i=0;i<num_points;i++) {
		p[i].x=(float)((*points)[i][0]);
		p[i].y=(float)((*points)[i][1]);
	}
}

int KLT::GetNumPoints() {
	return num_points;
}

void KLT::GetPointList(Array<Vector<2>> *points) {
	Vector<2> tmp;
	CvPoint2D32f* p=(CvPoint2D32f*)this->points;

	points->clear();
	for(int i=0;i<num_points;i++) {
		tmp[0]=p[i].x;
		tmp[1]=p[i].y;
		points->append(tmp);
	}
}

void KLT::Track(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2) {
	IplImage *Cimage1=Bitmap2Ipl(img1);
	IplImage *Cimage2=Bitmap2Ipl(img2);
	IplImage *image1=cvCreateImage(cvGetSize(Cimage1),8,1);
	IplImage *image2=cvCreateImage(cvGetSize(Cimage2),8,1);
	cvCvtColor(Cimage1,image1,CV_BGR2GRAY);
	cvCvtColor(Cimage2,image2,CV_BGR2GRAY);

	Track((void *)image1,(void *)image2,win_size,level,iterations,max_error,msk2);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseImage(&Cimage1);
	cvReleaseImage(&Cimage2);
}

void KLT::Track(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2) {
	IplImage *image1=Bitmap2Ipl(img1);
	IplImage *image2=Bitmap2Ipl(img2);

	Track((void *)image1,(void *)image2,win_size,level,iterations,max_error,msk2);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
}

void KLT::Track(void *img1,void *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2) {
    IplImage *image1=(IplImage *)img1;
	IplImage *image2=(IplImage *)img2;
	IplImage *pyramid1=cvCreateImage(cvGetSize(image1),8,1);
    IplImage *pyramid2=cvCreateImage(cvGetSize(image2),8,1);
	CvPoint2D32f* p=(CvPoint2D32f*)points;
	CvPoint2D32f* op=(CvPoint2D32f*)old_points;

	if (num_points==0) return;
	for(int i=0;i<num_points;i++) op[i]=p[i];
	cvCalcOpticalFlowPyrLK(image1,image2,pyramid1,pyramid2,
						   op,p,num_points,cvSize(win_size,win_size),level,status,track_error,
						   cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,iterations,0.03),0);

	
	
	float th=(float)(win_size*win_size*max_error);
	for(int i=0;i<num_points;i++) {
		if (status[i]==0) continue;
		if (track_error[i]>th) status[i]=0;
	
		if (msk2) {
			if (p[i].x>msk2->P1[0]) status[i]=0;
			if (p[i].x<msk2->P0[0]) status[i]=0;
			if (p[i].y>msk2->P1[1]) status[i]=0;
			if (p[i].y<msk2->P0[1]) status[i]=0;
		}
	}


	cvReleaseImage(&pyramid1);
	cvReleaseImage(&pyramid2);
}


int KLT::FindCorrespondences(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1,HyperBox<2> *msk2) {
	IplImage *image1=Bitmap2Ipl(img1);
	IplImage *image2=Bitmap2Ipl(img2);

	int ret=FindCorrespondences((void*)image1,(void*)image2,points1,points2,win_size,level,iterations,max_error,quality,min_dist,msk1,msk2);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	
	return ret;
}

int KLT::FindCorrespondences(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1,HyperBox<2> *msk2) {
	IplImage *Cimage1=Bitmap2Ipl(img1);
	IplImage *Cimage2=Bitmap2Ipl(img2);
	IplImage *image1=cvCreateImage(cvGetSize(Cimage1),8,1);
	IplImage *image2=cvCreateImage(cvGetSize(Cimage2),8,1);
	cvCvtColor(Cimage1,image1,CV_BGR2GRAY);
	cvCvtColor(Cimage2,image2,CV_BGR2GRAY);

	int ret=FindCorrespondences((void*)image1,(void*)image2,points1,points2,win_size,level,iterations,max_error,quality,min_dist,msk1,msk2);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseImage(&Cimage1);
	cvReleaseImage(&Cimage2);
	
	return ret;
}

int KLT::FindCorrespondences(void *img1,void *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1,HyperBox<2> *msk2) {
	Vector<2> tmp;
	Array<Vector<2>> tmp_array(max_num_points);
	points1->clear();
	points2->clear();


	Detect(img1,win_size,quality,min_dist,msk1);
	if (num_points==0) return 0;
	GetPointList(&tmp_array);
	Track(img1,img2,win_size,level,iterations,max_error,msk2);


	int num=0;
	CvPoint2D32f* p=(CvPoint2D32f*)this->points;
	for(int i=0;i<num_points;i++) {
		if (status[i]!=0) {
			points1->append(tmp_array[i]);
			tmp[0]=p[i].x;
			tmp[1]=p[i].y;
			points2->append(tmp);
			num++;
		}
	}

	return num;
}

void KLT::ShowCorrespondences(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2) {
	IplImage *image1=Bitmap2Ipl(img1);
	IplImage *image2=Bitmap2Ipl(img2);

	CvSize size;
	size.width=cvGetSize(image1).width+cvGetSize(image2).width;
	size.height=max(cvGetSize(image1).height,cvGetSize(image2).height);
	IplImage *show_image=cvCreateImage(size,8,1);

	for(int i=0;i<cvGetSize(image1).width;i++) {
		for(int j=0;j<cvGetSize(image1).height;j++) {
			show_image->imageData[show_image->widthStep*j+i]=image1->imageData[image1->widthStep*j+i];
		}
	}
	for(int i=0;i<cvGetSize(image2).width;i++) {
		for(int j=0;j<cvGetSize(image1).height;j++) {
			show_image->imageData[show_image->widthStep*j+i+cvGetSize(image1).width]=image2->imageData[image1->widthStep*j+i];
		}
	}

	cvNamedWindow("Image1-2",0);
	cvShowImage("Image1-2",show_image);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseImage(&show_image);
}

void KLT::ShowCorrespondences(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2) {
	IplImage *image1=Bitmap2Ipl(img1);
	IplImage *image2=Bitmap2Ipl(img2);

	CvSize size;
	size.width=cvGetSize(image1).width+cvGetSize(image2).width;
	size.height=max(cvGetSize(image1).height,cvGetSize(image2).height);
	IplImage *show_image=cvCreateImage(size,8,3);

	for(int i=0;i<cvGetSize(image1).width;i++) {
		for(int j=0;j<cvGetSize(image1).height;j++) {
			show_image->imageData[show_image->widthStep*j+i*3]=image1->imageData[image1->widthStep*j+i*3];
			show_image->imageData[show_image->widthStep*j+i*3+1]=image1->imageData[image1->widthStep*j+i*3+1];
			show_image->imageData[show_image->widthStep*j+i*3+2]=image1->imageData[image1->widthStep*j+i*3+2];
		}
	}
	for(int i=0;i<cvGetSize(image2).width;i++) {
		for(int j=0;j<cvGetSize(image1).height;j++) {
			show_image->imageData[show_image->widthStep*j+i*3+cvGetSize(image1).width*3]=image2->imageData[image1->widthStep*j+i*3];
			show_image->imageData[show_image->widthStep*j+i*3+1+cvGetSize(image1).width*3]=image2->imageData[image1->widthStep*j+i*3+1];
			show_image->imageData[show_image->widthStep*j+i*3+2+cvGetSize(image1).width*3]=image2->imageData[image1->widthStep*j+i*3+2];
		}
	}

	
	for(int i=0;i<points1->numElements();i++) {
		CvPoint p1,p2;
		p1.x=Approx((*points1)[i][0]);
		p1.y=Approx((*points1)[i][1]);
		p2.x=Approx((*points2)[i][0]+cvGetSize(image1).width);
		p2.y=Approx((*points2)[i][1]);

		cvLine(show_image,p1,p2,cvScalar(0,255,0,0));
	}

	cvNamedWindow("Image1-2",0);
	cvShowImage("Image1-2",show_image);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseImage(&show_image);
}


#endif