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





////////////////////
// Check Gradient //
////////////////////
struct CK_SEG_INFO_STRUCT {
	// Current info
	Vector<2> P;							// =P[t]
	Vector<2> P_before;						// =P[t-1] valid only if t!=0
	int t;									// t goes from 0 to num_max_iterations-1

	// CheckSegment Call information
	Vector<2> origin;
	Vector<2> direction;
	int num_max_iterations;
};
typedef bool(*CK_SEG_CALLBACK)(CK_SEG_INFO_STRUCT *,void *);

bool CheckSegment(Vector<2> o,Vector<2> dir,int Num_Max_Check,bool(*F)(CK_SEG_INFO_STRUCT *,void *),void *UserInfo);





///////////////////
// Raster a line //
///////////////////
void DrawLine(Vector<2> p1,Vector<2> p2,GreyLevel color,Bitmap<GreyLevel> *im,float aplha=1.0f);
void DrawLine(Vector<2> p1,Vector<2> p2,ColorRGB  color,Bitmap<ColorRGB>  *im,float aplha=1.0f);
void DrawLine(Vector<2> p1,Vector<2> p2,ColorRGB  color1,ColorRGB color2,Bitmap<ColorRGB> *im,float alpha=1.0f);  // Draw a gradient line




///////////////////
// Find Gradient //
///////////////////
struct ui_FindGradient_struct {
	Vector<2> Gradient_To_Search;
	double soglia_tolleranza_gradiente;
	int Gradient_Window_Size;						// must be odd

	Bitmap<GreyLevel> *image;
	Vector<2> finded_point;
};
bool FindGradient(CK_SEG_INFO_STRUCT *Info,ui_FindGradient_struct *ui_Info);

template <class color_type>
Vector<2> ImageGradient(int x,int y,Bitmap<color_type> *im,color_type color,int dim_mask); // dim_mask must be odd




///////////////////
//   Entities	 //
///////////////////
/* OBSOLETE
template <class color_type>
void DrawPoint(Vector<2> p,color_type color,Bitmap<color_type> *im);
*/




///////////////////
//   Gaussians   //
///////////////////
void MakeGaussian(Bitmap<GreyLevel> *I,double fall_off=0.6,double tollerance=(1.0/15.0));		// Padded  Gaussian:   0.0 Gaussian  1.0 White Circle
void MakeSunDisc(Bitmap<GreyLevel> *I,double disc_ray=0.6,double tollerance=(1.0/15.0));		// Clamped Gaussian:   0.0 Gaussian  1.0 White Circle
void MakeGaussian(Bitmap<GreyLevel> *I,double cx,double cy,double r_x,double r_y,double fall_off=0.6,double tollerance=(1.0/15.0),bool invert=false);
void MakeGaussian(Bitmap<GreyLevel> *I,HyperBox<2> *box,double fall_off=0.6,double tollerance=(1.0/15.0),bool invert=false);





///////////////////
//   Similarity  //
///////////////////
																										// Good   Bad
template <class color_type>
float NCD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size);	//  0.0 - 2.0

template <class color_type>
float DPD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size);	//  0.0 - 1.0

template <class color_type>
float SAD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size);	//  0.0 - Inf

template <class color_type>
float SAD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size,float (*ColorMetric)(color_type A,color_type B));
