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





/*

	Software ZBuffer creator
		Render objects without taking into account for problems like
			- far/near plane clipping          -> very usefull
			- depth accuracy					  (follows the float accuracy)
			- face culling disabled
			- thread-safe						  (tested)

*/

#define SOFTZBUFFER_TOLLERANCE	0.02f					// 2% tollerance on the visibility detection (with respect to the entire rendering volume)

class SoftZBuffer {
	RGBTextelRenderer Renderer;
	View *Vista;

	float ZMin,ZMax;
	float ZLasco;

public:
	SoftZBuffer(int width,int height,View *Vista);
	~SoftZBuffer();

	void DrawTextel(List<typename Vector<3>> *Points,HyperFace<3> *Face);
	void DrawObj(HyperMesh<3> *o);
	
	bool isVisible(int i_vertex,HyperMesh<3> *mesh,Vector<2> *Pr_Point=NULL);			// Dice se il vertice i della mesh appena disegnata è visibile o meno

	// read-only
	Bitmap<ColorFloat> *BufferZ;
};

