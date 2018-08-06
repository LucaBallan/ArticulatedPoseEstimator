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

SoftZBuffer::SoftZBuffer(int width,int height,View *Vista) : Renderer(width,height) {
	BufferZ=new Bitmap<ColorFloat>(width,height);
	Renderer.WriteOut=false;
	Renderer.UseOutputZBuffer=true;
	Renderer.WriteZBuffer=true;
	Renderer.TestZBuffer=true;
	Renderer.UseSrc=false;
	Renderer.ZBuffer=BufferZ;

	this->Vista=Vista;
	ZMin=+FLT_MAX;
	ZMax=-FLT_MAX;
}

SoftZBuffer::~SoftZBuffer() {
	SDELETE(BufferZ);
}


void SoftZBuffer::DrawTextel(List<typename Vector<3>> *Points,HyperFace<3> *Face) {
	Vector<3> D_;
	Triangle<typename TPoint2D<int>> D;
	Triangle<float> z;
	Triangle<typename TPoint2D<float>> UV;

	for(int j=0;j<3;j++) {
		D_=Vista->ProjectionZ((*Points)[Face->Point[j]]);

		D.Point[j].x=Approx(D_[0]);
		D.Point[j].y=Approx(D_[1]);
		z.Point[j]=(float)D_[2];
		
		if (ZMin>(float)D_[2]) ZMin=(float)D_[2];
		if (ZMax<(float)D_[2]) ZMax=(float)D_[2];
	}

	Renderer.DrawTextel(&D,&z,&UV);
}

void SoftZBuffer::DrawObj(HyperMesh<3> *o) {
	BufferZ->Clear(+FLT_MAX);
	for(int i=0;i<o->num_f;i++) {
		DrawTextel(&(o->Points),&(o->Faces[i]));
	}
	ZLasco=(ZMax-ZMin)*SOFTZBUFFER_TOLLERANCE;
}

bool SoftZBuffer::isVisible(int i_vertex,HyperMesh<3> *mesh,Vector<2> *Pr_Point) {
		
	Vector<3> D=Vista->ProjectionZ(mesh->Points[i_vertex]);
	if (Pr_Point!=NULL) {
		(*Pr_Point)[0]=D[0];
		(*Pr_Point)[1]=D[1];
	}
	float z=BufferZ->Point(Approx(D[0]),Approx(D[1]));

	if (D[2]<=z+ZLasco) return true;
	return false;
}



































/*



	RENDERIZZATORE totale -> FUNZIONANTE!!!








Bitmap<ColorRGB> TextTemp("luca.bmp");
	Bitmap<ColorRGB> OOO(SDimX,SDimY);
	ColorRGB x;
	x.r=255;
	x.g=x.b=0;
	OOO.Clear(x);
	Bitmap<float> BufferZ(SDimX,SDimY);
	BufferZ.Clear(FLT_MAX);


	RGBTextelRenderer *Renderer=new RGBTextelRenderer(SDimX,SDimY);  // Occhio Dim
	Renderer->WriteOut=true;
	Renderer->WriteZBuffer=true;
	Renderer->TestZBuffer=true;
	Renderer->UseSrc=true;
	Renderer->ZBuffer=&BufferZ;
	Renderer->Src=&TextTemp;
	Renderer->Out=&OOO;

	for(int i=0;i<Obj->num_f;i++) {
		Vector<3> D_;
		Triangle<typename TPoint2D<int>> D;
		Triangle<float> z;
		Triangle<typename TPoint2D<float>> UV;

		for(int j=0;j<3;j++) {
			D_=Vista[0].ProjectionZ(Obj->Points[Obj->Faces[i].Point[j]]);

			D.Point[j].x=Approx(D_[0]);
			D.Point[j].y=Approx(D_[1]);
			z.Point[j]=D_[2];
			
			UV.Point[j].x=FaceMap[(3*i)+j][0]*(TextTemp.width-1);
			UV.Point[j].y=FaceMap[(3*i)+j][1]*(TextTemp.height-1);
		}

		Renderer->DrawTextel(&D,&z,&UV);

	}


	OOO.Save("OOO.bmp");
*/
