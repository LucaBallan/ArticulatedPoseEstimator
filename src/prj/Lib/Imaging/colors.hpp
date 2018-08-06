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





#include <ctype.h>
#include <limits>




HCY_color RGB2HCY(ColorRGB color) {
	// set color are between [0,1]
	HCY_color out;
	float chroma;
	float r = color.r / 255.0f, g = color.g / 255.0f, b = color.b / 255.0f;

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

ColorRGB RGB2HCY_ub(ColorRGB color) {
	HCY_color c = RGB2HCY(color);

	c.y = round(c.y*255.0f);
	c.cs = round(((c.cs + 1.0f) / 2.0f)*255.0f);
	c.cc = round(((c.cc + 1.0f) / 2.0f)*255.0f);
	c.y = min(max(c.y, 0.0f), 255.0f);
	c.cs = min(max(c.cs, 0.0f), 255.0f);
	c.cc = min(max(c.cc, 0.0f), 255.0f);

	color.r = (unsigned char)c.y;
	color.g = (unsigned char)c.cc;
	color.b = (unsigned char)c.cs;
	return color;
}













void RGB2IHLS(ColorRGB rgb,ihls_color &ihls) {
	float r=(float)(rgb.r/255.0);
	float g=(float)(rgb.g/255.0);
	float b=(float)(rgb.b/255.0);

	float M=max(max(r,g),b);
	float m=min(min(r,g),b);

	ihls.s=M-m;
	ihls.y=(float)((0.2125*r)+(0.7154*g)+(0.0721*b));
	
	float crx=(float)(r-((g+b)/2.0));
	float cry=(float)(sqrt(3.0)*(b-g)/2.0);
	float cr=(float)(sqrt((crx*crx)+(cry*cry)));

	if (cr==0) ihls.h=std::numeric_limits<float>::quiet_NaN();
	else {
		if (cry<=0) {
			ihls.h=acos(crx/cr);
		} else {
			ihls.h=(float)(2*M_PI-acos(crx/cr));
		}
	}
}

void IHLS2SH(ihls_color ihls,sh_color &sh) {
	if (_isnan(ihls.h)) {
		sh.C=sh.S=0.0;
		sh.h_x=sh.h_y=0.0;
	} else {
		sh.h_x=cos(ihls.h);
		sh.h_y=sin(ihls.h);
		sh.C=ihls.s*sh.h_x;
		sh.S=ihls.s*sh.h_y;
	}
	sh.y=ihls.y;
	sh.s=ihls.s;
}



float DH(sh_color c,float m_C,float m_S) {
	float d_C=m_C-c.C;
	float d_S=m_S-c.S;
	return (sqrt((d_C*d_C)+(d_S*d_S)));
}


ColorRGB colormap_gray(double x) {
	ColorRGB c;
	if (x>1.0) x=1.0;
	if (x<0.0) x=0.0;
	
	c.r=(BYTE)(x*255);
	c.g=(BYTE)(x*255);
	c.b=(BYTE)(x*255);

	return c;
}
ColorRGB colormap_rgb(double x1,double x2,double x3) {
	ColorRGB c;
	if (x1>1.0) x1=1.0;
	if (x1<0.0) x1=0.0;
	if (x2>1.0) x2=1.0;
	if (x2<0.0) x2=0.0;
	if (x3>1.0) x3=1.0;
	if (x3<0.0) x3=0.0;
	
	c.r=(BYTE)(x1*255);
	c.g=(BYTE)(x2*255);
	c.b=(BYTE)(x3*255);

	return c;
}

// x = 0.0 - 1.0 
// factor = 0.001 - 1.0  (1.0 lineare 0.0 logaritmico)
double Log_Scale(double x,double factor) {
	double v=((log(x+factor)-log(factor))/(log(1+factor)-log(factor)));
	if (_isnan(v)) std::cout<<"Errore\n";
	if (v<0.0) v=0.0;
	if (v>1.0) v=1.0;
	return v;
}


ColorRGB colormap_hot(double x) {
	double n=3.0/8.0;

	ColorRGB c;
	if (x>1.0) x=1.0;
	if (x<0.0) x=0.0;

	if (x<n) {
		c.r=(BYTE)Approx(255*(x/n));
		c.g=0;
		c.b=0;
	} else {
		c.r=255;
		if (x<2*n) {
			c.g=(BYTE)Approx(255*((x-n)/n));
			c.b=0;
		} else {
			c.g=255;
			c.b=(BYTE)Approx(255*( (x-2*n)/(1.0-2*n) ));
		}
	}
	return c;
}


void ComputaIHLSRange() {
	ColorRGB c;
	ihls_color cc;
	sh_color sh_cc;
	
	ihls_color max,min;
	sh_color s_max,s_min;
	max.h=-FLT_MAX;min.h=FLT_MAX;
	max.s=-FLT_MAX;min.s=FLT_MAX;
	max.y=-FLT_MAX;min.y=FLT_MAX;
	s_max.h_x=-FLT_MAX;s_min.h_x=FLT_MAX;
	s_max.h_y=-FLT_MAX;s_min.h_y=FLT_MAX;
	s_max.C=-FLT_MAX;s_min.C=FLT_MAX;
	s_max.S=-FLT_MAX;s_min.S=FLT_MAX;

	for(int r=0;r<=255;r++) {
		for(int g=0;g<=255;g++) {
			for(int b=0;b<=255;b++) {
				c.r=(BYTE)r;c.g=(BYTE)g;c.b=(BYTE)b;
				RGB2IHLS(c,cc);
				IHLS2SH(cc,sh_cc);
				if (!_isnan(cc.h)) {
					max.h=MAX(cc.h,max.h);
					min.h=MIN(cc.h,min.h);
				}
				max.s=MAX(cc.s,max.s);
				max.y=MAX(cc.y,max.y);
				min.s=MIN(cc.s,min.s);
				min.y=MIN(cc.y,min.y);

				s_max.h_x=MAX(sh_cc.h_x,s_max.h_x);
				s_max.h_y=MAX(sh_cc.h_y,s_max.h_y);
				s_min.h_x=MIN(sh_cc.h_x,s_min.h_x);
				s_min.h_y=MIN(sh_cc.h_y,s_min.h_y);
				s_max.C=MAX(sh_cc.C,s_max.C);
				s_max.S=MAX(sh_cc.S,s_max.S);
				s_min.C=MIN(sh_cc.C,s_min.C);
				s_min.S=MIN(sh_cc.S,s_min.S);

			}
		}
	}
	
	std::cout<<"y : [ "<<min.y<<" - "<<max.y<<" ]\n";
	std::cout<<"s : [ "<<min.s<<" - "<<max.s<<" ]\n";
	std::cout<<"h : [ "<<min.h<<" - "<<max.h<<" ]\n";
	std::cout<<"h_x : [ "<<s_min.h_x<<" - "<<s_max.h_x<<" ]\n";
	std::cout<<"h_y : [ "<<s_min.h_y<<" - "<<s_max.h_y<<" ]\n";
	std::cout<<"C : [ "<<s_min.C<<" - "<<s_max.C<<" ]\n";
	std::cout<<"S : [ "<<s_min.S<<" - "<<s_max.S<<" ]\n";
}


