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
// HCY color space is shaped as a cylinder having Y as main axis.
// H (hue) and C (Chroma) are converted in Cartesian coordinates
// 
// It is very similar to the HCL color space as it maintains the same properties. 
//
// RGB to HCY is an easy operation, while the inverse needs a lookup table.
//
// The RGB color space is completely contained inside the HCY color space but
// it does not cover it all.
// RGB gamut in HCY is very complicated and can be visualized with the code below.
//
// It is visible how red and blue dominates when the luminance is low while green
// dominate at high luminance.
// This color space is supposed to be perceptually coherent.
//

struct HCY_color {
	float y, cs, cc;
};

HCY_color RGB2HCY(ColorRGB color);
ColorRGB RGB2HCY_ub(ColorRGB color);
//
// HCY_color RGB2HCY(cv::Vec3b colorBGR);             --> see opencvwrapper.h
// cv::Vec3b RGB2HCY_ub(cv::Vec3b colorBGR);
// cv::Vec3w RGB2HCY_us(cv::Vec3b colorBGR);
//


//
// IMPORTANT:
// 
// There is an implementation on the shaders RGBA2HCYA in ImageOperationsRenderer  (it saves in YHHA format)
//









//
// Other color spaces
//


struct ihls_color {
    float y,s,h;
};

struct sh_color {
	float y,s,h_x,h_y;		// h_x=cos(h),h_y=sin(h)
	float C,S;				// Derived C=s*h_x,S=s*h_y
};



void RGB2IHLS(ColorRGB rgb,ihls_color &ihls);
float DH(sh_color c,float m_C,float m_S);
void IHLS2SH(ihls_color ihls,sh_color &sh);

ColorRGB colormap_hot(double x);
ColorRGB colormap_gray(double x);
ColorRGB colormap_rgb(double x1,double x2,double x3);

void ComputaIHLSRange();
double Log_Scale(double x,double factor);



#define MIN_Y 0.0
#define MAX_Y 1.0
#define MIN_s 0.0
#define MAX_s 1.0
#define MIN_h 0.0
#define MAX_h (2*M_PI)
#define MIN_h_x -1.0
#define MAX_h_x +1.0
#define MIN_h_y -1.0
#define MAX_h_y +1.0
#define MIN_C -1.0
#define MAX_C +1.0
#define MIN_S -1.0
#define MAX_S +1.0




