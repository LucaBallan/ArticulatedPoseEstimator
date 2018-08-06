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





#ifdef USE_OPENGL_LIBRARY


#define KLT_MAX_POINT_NUM 400


class KLT {
	void *points,*old_points;
	int max_num_points;
	int num_points;
	
	char *status;
	float *track_error;

	void Detect(void *img,int win_size,double quality,double min_dist=0.0,HyperBox<2> *mask=NULL);
	void Track(void *img1,void *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2=NULL);
	int FindCorrespondences(void *img1,void *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1=NULL,HyperBox<2> *msk2=NULL);
public:
	KLT(int max_num_points=KLT_MAX_POINT_NUM);
	~KLT();
	
	// Create point list
	void SetPointList(Array<Vector<2>> *points);
	void Detect(Bitmap<ColorRGB> *img,int win_size,double quality,double min_dist=0.0,HyperBox<2> *mask=NULL);
	void Detect(Bitmap<GreyLevel> *img,int win_size,double quality,double min_dist=0.0,HyperBox<2> *mask=NULL);

	// Retrieve updated point list 
	int GetNumPoints();
	void GetPointList(Array<Vector<2>> *points);
	
	// Alter point list
	void Track(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2=NULL);
	void Track(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,int win_size,int level,int iterations,double max_error,HyperBox<2> *msk2=NULL);



	// All in one
	int FindCorrespondences(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1=NULL,HyperBox<2> *msk2=NULL);
	int FindCorrespondences(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2,int win_size,int level,int iterations,double max_error,double quality,double min_dist,HyperBox<2> *msk1=NULL,HyperBox<2> *msk2=NULL);


	void ShowCorrespondences(Bitmap<GreyLevel> *img1,Bitmap<GreyLevel> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2);
	void ShowCorrespondences(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Array<Vector<2>> *points1,Array<Vector<2>> *points2);
};


#endif