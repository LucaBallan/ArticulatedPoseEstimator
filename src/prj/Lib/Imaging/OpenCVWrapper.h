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





//
// IPL SPECIFIC FUNCTION
//
#ifdef USE_OPENGL_LIBRARY
	#include "opencv/cv.h"
	#include "opencv/highgui.h"

	template <class color_type>
	void Ipl2Bitmap(IplImage *src,Bitmap<color_type> *dst);

	template <class color_type>
	void Ipl2Bitmap(cv::Mat *src, Bitmap<color_type> *dst);

	template <class color_type>
	void Bitmap2Ipl(Bitmap<color_type> *src,IplImage *dst);

	UINT Ipl2Texture(IplImage *src,RenderWindow<3> *w,int options=0);

	#define IPL_IMAGE_OGL IplImage

	CvMat  *CreateMatFromMatrix(Matrix *m);					// ??? CHECK never TODO**
	Matrix *CreateMatrixFromMat(CvMat *m);					// ??? CHECK never TODO**
	void    GetMatFromMatrix(Matrix *m,CvMat *out);			// ??? CHECK never TODO**
	void    GetMatrixFromMat(CvMat *m,Matrix *out);			// ??? CHECK never TODO**
	CvMat  *CreateMatFromArray(float *v,int num);			// ??? CHECK never TODO**


	HCY_color RGB2HCY(cv::Vec3b colorBGR);
	cv::Vec3b RGB2HCY_ub(cv::Vec3b colorBGR);
	cv::Vec3w RGB2HCY_us(cv::Vec3b colorBGR);
#else
	#define IPL_IMAGE_OGL void
#endif




//
// Load & Save Function
//
template <class color_type>
void LoadImageOCV(char *fileName,Bitmap<color_type> *c,bool force_load_greyscale);

template <class color_type>
void SaveImageOCV(char *fileName,Bitmap<color_type> *c,bool save_grayscale);



//
// Distance Transform
//
class SilhouetteDistance_Buffer {
public:	
	IPL_IMAGE_OGL *ia,*ib;
	IPL_IMAGE_OGL *iia,*iib;
	SilhouetteDistance_Buffer(int w,int h);
	~SilhouetteDistance_Buffer();

	void PrepareImageA(Bitmap<GreyLevel> *a,bool invert_image=false,int BinaryThreshold=-1);		// BinaryThreshold=-1 -> do not apply threshold
	void PrepareImageB(Bitmap<GreyLevel> *b,bool invert_image=false,int BinaryThreshold=-1);
};

//
//
// SilhouetteDistance:
//         * Average distance of a pixel in one image with respect to the pixels in the other image 
//         * ia and ib should be white with the object in black.
//         * Indicates the average distance between the black pixels of one image 
//           with respect to the black ones of the other. Average distance -> so it is not 
//           affected by the number of pixels in the images.
//         * Use PrepareImageA and PrepareImageB to load ia and ib in the correct way.
//         * ST means single thread, MT means multi threads.
//         * NOTE: the image iia is always an external blur, never internal to the object.
//         * Multi-tasking controllato -> funziona perfettamente.
//         * return DBL_INF se una delle due immagini non ha pixel neri
//         * n_observations can be NULL
//         * n_observations!=NULL -> number of observations used to compute the distance transform
//         * the result is = L1/n_observations
//
double SilhouetteDistanceMT(SilhouetteDistance_Buffer *st,int *n_observations);
double SilhouetteDistanceST(SilhouetteDistance_Buffer *st,int *n_observations);
double SilhouetteDistanceST(Bitmap<GreyLevel> *a,Bitmap<GreyLevel> *b,int BinaryThreshold,SilhouetteDistance_Buffer *st,int *n_observations); // a and b are assumed to be black with a white object -> default 128 threshold and inversion
//
//         * the result is = L2/n_observations
double SilhouetteDistanceMT_mse(SilhouetteDistance_Buffer *st,int *n_observations);





//
// Resize
//
#define  INTERPOLATION_NN        0
#define  INTERPOLATION_LINEAR    1
#define  INTERPOLATION_CUBIC     2
#define  INTERPOLATION_AREA      3
template <class color_type>
Bitmap<color_type> *Resize(Bitmap<color_type> *img,int w,int h,int interpolation_type);








