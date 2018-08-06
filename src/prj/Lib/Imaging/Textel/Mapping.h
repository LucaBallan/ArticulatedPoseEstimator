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





// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// -------------------------- Triangle ---------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

template <class data_type>
class TPoint2D {
public:
	data_type x,y;
};


template <class point_type>
class Triangle {
public:
	point_type Point[3];
};






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///	    Precise and Fast Renderer
///
///				- thread-safe (tested)
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextelRenderer {
protected:
	int n_input;
	int width,height;
	int *edge_l;
	int *edge_r;
	float **edge_lvc;
	float **edge_rvc;
	float *vc;
	
	inline void ScanEdge(int x1,int y1,int x2,int y2,float *vc1,float *vc2);
	void getVC(float d,float *vc1,float *vc2);
	void setVC(float d,float *vc1,float *vc2,float *vc_);
	virtual void PixelShader(int x,int y,float *vc) = 0;

public:

	TextelRenderer(int width,int height,int n_input);
	virtual ~TextelRenderer();

	
	void DrawTextel(Triangle<typename TPoint2D<int>> *D,Triangle<float *> *Vect);
				// Limiti Output > Controllati

};





class RGBTextelRenderer : public TextelRenderer {
private:
	Triangle<float *> Vect;

protected:
	virtual void PixelShader(int x,int y,float *vc);
	void FlatShader(int x,int y,float *vc);
	void BilinearFiltering(int x,int y,float *vc);

public:
	Bitmap<ColorRGB> *Out;
	Bitmap<ColorRGB> *Src;
	Bitmap<float>    *ZBuffer;
	
	bool WriteOut;
	bool UseOutputZBuffer;
	bool WriteZBuffer;
	bool TestZBuffer;
	bool UseSrc;
	ColorRGB Flat_color;
	float Lasco_InputZBuffer;
	void (*AddShader)(int x,int y,float *vc);


	RGBTextelRenderer(int width,int height);
	virtual ~RGBTextelRenderer();



	void DrawTextel(Triangle<typename TPoint2D<int>> *D,Triangle<float> *z,Triangle<typename TPoint2D<float>> *UV);
};




















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////  FAST RENDERER //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ------------------ OpenGL_FastTextelRenderer ------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

/*
class Triangle2D {
public:
	Vector<2> Point[3];
	float Area();
};


void init_opengl();

class OpenGL_FastTextelRenderer {
	PBuffer pbuffer;

public:
	int width,height,src_w,src_h;

	OpenGL_FastTextelRenderer(int width,int height,int src_w,int src_h);
	~OpenGL_FastTextelRenderer();

	UINT LoadTexture(Bitmap<ColorRGB> *tex);
	UINT LoadTexture(Bitmap<ColorRGBA> *tex);
	void DeleteTexture(UINT index);

	void StartDraw();
 	void DrawTextel(Triangle<TPoint2D<int>> *D,Triangle<TPoint2D<int>> *S,UINT Src);
	void DrawFace(Triangle<TPoint2D<int>> *D,int color);
	void EndDraw();

	void GetBuffer(BYTE *mem);
	void GetBuffer(int x,int y,int lx,int ly,BYTE *mem);

	bool IsValid();
};
*/


