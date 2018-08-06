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





class SoftZBuffer {
	ZTestAffineTextelRenderer Renderer;
	View *Vista;
	float ZMin,ZMax;

public:
	Bitmap<ColorRGB> *Image;

	SoftZBuffer(int width,int height,View *Vista);
	~SoftZBuffer();

	void DrawTextel(List<typename Vector<3>> *Points,HyperFace<3> *Face,int index);
	void DrawObj(HyperMesh<3> *o);
};




SoftZBuffer::SoftZBuffer(int width,int height,View *Vista) : Renderer(width,height,1) {
	BufferZ=new Bitmap<ColorFloat>(width,height);
	BufferIndex=new Bitmap<ColorInt>(width,height);
	this->Vista=Vista;
	ZMin=+FLT_MAX;
	ZMax=-FLT_MAX;
}

SoftZBuffer::~SoftZBuffer() {
	delete BufferZ;
	delete BufferIndex;
}



void SoftZBuffer::DrawTextel(List<typename Vector<3>> *Points,HyperFace<3> *Face,int index) {

	Vector<3> D_;
	Triangle<float,int> z;
	FastTriangle D;
	
	for(int j=0;j<3;j++) {
		D_=Vista->ProjectionZ((*Points)[Face->Point[j]]);

		D.Point[j].x=dtofx(D_[0]);
		D.Point[j].y=dtofx(D_[1]);
		z.Point[j].x=D_[2];
		z.Point[j].y=index;
		
		if (ZMin>D_[2]) ZMin=D_[2];
		if (ZMax<D_[2]) ZMax=D_[2];
	}

	Renderer.DrawTextel(&D,&z,BufferZ->getBuffer(),BufferIndex->getBuffer());

}

void SoftZBuffer::DrawObj(HyperMesh<3> *o) {
	
	for(int i=0;i<o->num_f;i++) {
		DrawTextel(&(o->Points),&(o->Faces[i]),i);
	}
}
