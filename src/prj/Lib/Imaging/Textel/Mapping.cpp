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
#include "TextelRenderer.hpp"




RGBTextelRenderer::RGBTextelRenderer(int width,int height) : TextelRenderer(width,height,3) {
	Out=NULL;
	Src=NULL;
	ZBuffer=NULL;
	
	UseOutputZBuffer=true;
	WriteZBuffer=true;
	TestZBuffer=true;
	WriteOut=true;
	UseSrc=true;
	Flat_color=(BYTE)0;
	Lasco_InputZBuffer=0.0f;
	AddShader=NULL;

	Vect.Point[0]=new float[3];
	Vect.Point[1]=new float[3];
	Vect.Point[2]=new float[3];
}

RGBTextelRenderer::~RGBTextelRenderer() {
	delete []Vect.Point[0];
	delete []Vect.Point[1];
	delete []Vect.Point[2];
}

void RGBTextelRenderer::PixelShader(int x,int y,float *vc) {
	// vc[0]=x
	// vc[1]=y
	// vc[2]=z

	if (UseOutputZBuffer) {
		// OutputZBuffer
		if ((!TestZBuffer) || (vc[2]<=ZBuffer->Point(x,y))) {
			if (WriteOut) {
				if (UseSrc) BilinearFiltering(x,y,vc);
				else FlatShader(x,y,vc);
			}	
			if (WriteZBuffer) ZBuffer->SetPoint(x,y,vc[2]);
		}
	} else {
		// InputZBuffer
		int ru=Approx(vc[0]);
		int rv=Approx(vc[1]);

		if ((!TestZBuffer) || (vc[2]<=Lasco_InputZBuffer+ZBuffer->Point(ru,rv))) {
			if (WriteOut) {
				if (UseSrc) BilinearFiltering(x,y,vc);
				else FlatShader(x,y,vc);
			}	
			if (WriteZBuffer) ZBuffer->SetPoint(ru,rv,vc[2]);
		}
	}
}







void RGBTextelRenderer::DrawTextel(Triangle<typename TPoint2D<int>> *D,Triangle<float> *z,Triangle<typename TPoint2D<float>> *UV) {
	Vect.Point[0][0]=UV->Point[0].x;
	Vect.Point[0][1]=UV->Point[0].y;
	Vect.Point[0][2]=z->Point[0];
	Vect.Point[1][0]=UV->Point[1].x;
	Vect.Point[1][1]=UV->Point[1].y;
	Vect.Point[1][2]=z->Point[1];
	Vect.Point[2][0]=UV->Point[2].x;
	Vect.Point[2][1]=UV->Point[2].y;
	Vect.Point[2][2]=z->Point[2];

	TextelRenderer::DrawTextel(D,&Vect);

}
inline void RGBTextelRenderer::BilinearFiltering(int x,int y,float *vc) {
	// vc[0]=u
	// vc[1]=v

	ColorRGB zij=Src->Point((int)vc[0],(int)vc[1]);
	ColorRGB zIj=Src->Point((int)vc[0]+1,(int)vc[1]);
	ColorRGB ziJ=Src->Point((int)vc[0],(int)vc[1]+1);
	ColorRGB zIJ=Src->Point((int)vc[0]+1,(int)vc[1]+1);

	// Bilinear Filtering
	float ku=vc[0]-((int)vc[0]);
	float kv=vc[1]-((int)vc[1]);
	float Ku=1-ku;
	float Kv=1-kv;

	float c0=Ku*Kv;
	float c1=ku*Kv;
	float c2=Ku*kv;
	float c3=ku*kv;

	ColorRGB c;
	c.r=(BYTE)((zij.r*c0)+(zIj.r*c1)+(ziJ.r*c2)+(zIJ.r*c3));
	c.g=(BYTE)((zij.g*c0)+(zIj.g*c1)+(ziJ.g*c2)+(zIJ.g*c3));
	c.b=(BYTE)((zij.b*c0)+(zIj.b*c1)+(ziJ.b*c2)+(zIJ.b*c3));

	Out->SetPoint(x,y,c);

	if (AddShader) AddShader(x,y,vc);
}
inline void RGBTextelRenderer::FlatShader(int x,int y,float *vc) {
	// vc[0]=x
	// vc[1]=y

	Out->SetPoint(x,y,Flat_color);
}








































