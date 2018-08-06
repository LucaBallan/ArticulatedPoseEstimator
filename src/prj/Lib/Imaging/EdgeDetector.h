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





class EdgeDetector {
	AdvancedRenderWindow *gpu_obj;
	ImageProcessingRenderer *IR;
	FrameBuffer_struct ffp;
	UINT Float_Texture_Array[7];

	Bitmap<ColorFloat> *fx,*fy,*norm;
	IplImage *weak_set1,*weak_set2;
	Array<int> *sx,*sy;

	void Canny_Get(UINT image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold);
	void CannyGreyInteractive(UINT input_image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold);

public:
	EdgeDetector(AdvancedRenderWindow *gpu_obj,int width,int height);
	~EdgeDetector();

	//
	// low_threshold=-1                -> low_threshold = 40% of high_threshold
	// low_threshold=high_threshold    -> use simple mode   -> for similar results set both to the 80% of the previous high_threshold value
	//
	void Canny(Bitmap<ColorRGB>  *src,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold=-1);
	void Canny(Bitmap<GreyLevel> *src,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold=-1);
	
	void CannyGrey(UINT input_image,Bitmap<GreyLevel> *dst,float sigma,float high_threshold,float low_threshold=-1,int interactive=0);
};



class ImageGradientEstimator {
	AdvancedRenderWindow    *gpu_obj;
	ImageProcessingRenderer *IR;
	FrameBuffer_struct       ffp;
	UINT                     Float_Texture_Array[4];

	void Gradient(UINT image,Bitmap<ColorFloat> *fx,Bitmap<ColorFloat> *fy);
public:
	ImageGradientEstimator(AdvancedRenderWindow *gpu_obj,int width,int height);
	~ImageGradientEstimator();

	void Gradient(Bitmap<GreyLevel> *src,Bitmap<ColorFloat> *fx,Bitmap<ColorFloat> *fy);
};

