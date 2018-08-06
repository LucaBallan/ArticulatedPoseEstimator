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





BYTE * LoadBMP(char *fileName,
			   UINT *width, 
			   UINT *height);

void SaveBMP(char *fileName,	
			 BYTE * buf,		
			 UINT width,
			 UINT height);

BYTE *Convert2GreyScale(BYTE *Img24bit,UINT width,UINT height);




typedef BYTE   GreyLevel;
typedef DWORD  ColorRGBA;
typedef float  ColorFloat;
typedef int    ColorInt;
typedef int    WI_Type;

#define ALPHA_C(x) ((BYTE)(((x)>>24)&0xFF))
#define RED_C(x) ((BYTE)((x)&0xFF))
#define GREEN_C(x) ((BYTE)(((x)>>8)&0xFF))
#define BLUE_C(x) ((BYTE)(((x)>>16)&0xFF))
#define RGBA2ColorRGBA(r,g,b,a) (((UINT)( BYTECLAMP(r) ))+(((UINT)(BYTECLAMP(g)))<<8)+(((UINT)(BYTECLAMP(b)))<<16)+(((UINT)(BYTECLAMP(a)))<<24))


class ColorRGB {
public:
	BYTE r,g,b;
	
	ColorRGB() {};
	ColorRGB(BYTE r_,BYTE g_,BYTE b_) : r(r_), g(g_), b(b_) {};
	ColorRGB(BYTE gray) : r(gray), g(gray), b(gray) {};
	ColorRGB(ColorRGBA x) { 
		r=RED_C(x);
		g=GREEN_C(x);
		b=BLUE_C(x);
	}

	const ColorRGB operator*(const float &a) const {
		ColorRGB q;
		q.r=(BYTE)(r*a);
		q.g=(BYTE)(g*a);
		q.b=(BYTE)(b*a);

		return q; 
	}
	const ColorRGB operator*(const double &a) const {
		ColorRGB q;
		q.r=(BYTE)(r*a);
		q.g=(BYTE)(g*a);
		q.b=(BYTE)(b*a);

		return q; 
	}
	const ColorRGB operator+(const ColorRGB &v) const {
		ColorRGB q;
		q.r=r+v.r;
		q.g=g+v.g;
		q.b=b+v.b;

		return q; 
	}

	int toDWORD() {
		UINT color;
		color =((UINT)(r))<<16;
		color+=((UINT)(g))<<8;
		color+=((UINT)(b));
		color+=0xFF000000;
		return ((int)color);
	}
};

bool operator==(const ColorRGB &a,const ColorRGB &b);
bool operator!=(const ColorRGB &a,const ColorRGB &b);


#include "colors.h"

class ColorRGB_int {
public:
	int r,g,b;

	ColorRGB_int() {};
	ColorRGB_int(int r_,int g_,int b_) : r(r_), g(g_), b(b_) {};
	ColorRGB_int(int gray) : r(gray), g(gray), b(gray) {};

#ifdef FIXED_POINT
	const ColorRGB_int operator*(const Fixed &a) const {
		ColorRGB_int q;
		
		q.r=fxtoi(r*a);
		q.g=fxtoi(g*a);
		q.b=fxtoi(b*a);

		return q; 
	}
#endif

	const ColorRGB_int operator*(const float &a) const {
		ColorRGB_int q;
		q.r=(int)(r*a);
		q.g=(int)(g*a);
		q.b=(int)(b*a);

		return q; 
	}
	const ColorRGB_int operator+(const ColorRGB_int &v) const {
		ColorRGB_int q;
		q.r=r+v.r;
		q.g=g+v.g;
		q.b=b+v.b;

		return q; 
	}

	bool operator!=(const int z) const {
		return ((r!=z) || (g!=z) || (b!=z));
	}
};

/*
// Conversion
operator GreyLevel(const ColorRGB &c) {
	return (GreyLevel)(((int)c.r+(int)c.g+(int)c.b)/3);
}
*/

template <class color_type>
float ToGrey(color_type c);



template <class color_type>
class Bitmap {
	friend Bitmap<GreyLevel>;
	friend Bitmap<ColorRGB>;
	friend Bitmap<ColorRGBA>;
	friend Bitmap<WI_Type>;
	friend Bitmap<ColorFloat>;

public:
	static void Init();


private:
	color_type *Img;
	bool virtual_;


	// Metodi privati
	void SaveBMP(char *filename);
	void SaveRAW(char *filename);
	void SaveFLOAT(char *filename);
	void LoadBMP(char *filename);
	void LoadRAW(char *filename);
	void LoadFLOAT(char *filename);

public:
	
	
	// Properties
	int         width,height;
	bool        IsValid();
	static bool isgrayscale();
	static int  bpp();


	// Costructors and Distructors
	Bitmap(int width,int height);							// Create an image (width,height)  -> (0,0) create a virutal bitmap that is a bitmap which buffer is managed by outer entities
	Bitmap(char *filename);
	~Bitmap();


	// Load and Save
	void Load(char *filename);								// Load an image (BMP,RAW,FLOAT)
	void Save(char *filename);								// Save the image (BMP,RAW,FLOAT)
	float SAVE_FLOAT_MIN_VAL,SAVE_FLOAT_MAX_VAL;			// validi solo se si salvano dei float 



	// Copy
	void CopyFrom(Bitmap<GreyLevel> *src);
	void CopyFrom(Bitmap<ColorRGB> *src);
	void CopyFrom(Bitmap<ColorRGBA> *src);					
	void CopyFrom(Bitmap<WI_Type> *src);
	void CopyFrom(Bitmap<ColorFloat> *src,bool adjust=true,float CLAMP_MIN=-FLT_MAX,float CLAMP_MAX=FLT_MAX);
	void CopyFromRescale(Bitmap<ColorFloat> *src,float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,color_type VAL_MIN,color_type VAL_MAX);
	void ExtractChannel(int channel,Bitmap<ColorFloat> *dst);		// TODO *** Funziona solo con RGBA!!!!!!!!!!!!



	// Low Level Operation
	inline color_type *getBuffer() {return Img;}
	void setBuffer(color_type *Img) {this->Img=Img;}
	void setVirtual(bool val) {this->virtual_=val;}



	// Pixel Access
	inline color_type Point(int x,int y) const {
		#ifdef IMAGE_CHECK_BOUNDARIES
		if ((x<0) || (x>=(int)width)) {
			#ifdef IMAGE_CHECK_BOUNDARIES_WARNING
				printf("Warning: Image boundaries violated in reading.\n");
			#endif
			return (BYTE)0;
		}
		if ((y<0) || (y>=(int)height)) {
			#ifdef IMAGE_CHECK_BOUNDARIES_WARNING
				printf("Warning: Image boundaries violated in reading.\n");
			#endif
			return (BYTE)0;
		}
		#endif
		return Img[(x+(y*width))];
	}
	inline void SetPoint(int x,int y,color_type c) {
		#ifdef IMAGE_CHECK_BOUNDARIES
		if ((x<0) || (x>=(int)width)) {
			#ifdef IMAGE_CHECK_BOUNDARIES_WARNING
				printf("Warning: Image boundaries violated in writing.\n");
			#endif
			return;
		}
		if ((y<0) || (y>=(int)height)) {
			#ifdef IMAGE_CHECK_BOUNDARIES_WARNING
				printf("Warning: Image boundaries violated in writing.\n");
			#endif
			return;
		}
		#endif
		Img[(x+(y*width))]=c;
	}



	// Fast Pixel Access
	inline color_type Point_NCB(int x,int y) const {return Img[(x+(y*width))];}


	// Boundaries
	bool isOutside(int x,int y,int lasco=0);				// lasco è usato per le maschere.. Es: 3x3 -> lasco = 1
	float BorderDistance(int x,int y);						// signed distance from the border (<0 outside). belongs to [0.0,1.0] -> 0.0 means on the border, 1.0 means in the center of the image (max value)


	// Operations
	void Multiply(float x);
	void Rescale(float MIN,float MAX,float CLAMP_MIN,float CLAMP_MAX,float VAL_MIN,float VAL_MAX);
	void Gaussian();
	void Clear();
	void Clear(color_type x);
	void Randomize();
	void Flip_Vertical();
	void Include(Bitmap<color_type> *img,int x,int y);
	void Threshold(color_type level);
	void ThresholdInvert(color_type level);
	Bitmap<color_type> *AdaptSize(int w,int h);				// Genera una nuova immagine di dimensione w,h 
															// estendendo o tagliando l'immagine originale




#ifdef SYSTEM_WINDOWS
	void Draw(HDC hdc,int x,int y,int operation=SRCCOPY);
#endif 

	// Static Functions
	static color_type GetColor(float r,float g,float b,float a);


};















ColorRGB GetMedian(Bitmap<ColorRGB> *src);
ColorRGB GetMedian(Bitmap<ColorRGB> *src, Bitmap<GreyLevel> *mask);

void GetRedChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst);
void GetGreenChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst);
void GetBlueChannel(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *dst);
void GetAlphaChannel(Bitmap<ColorRGBA> *src,Bitmap<GreyLevel> *dst);

void SetRedChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst);
void SetGreenChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst);
void SetBlueChannel(Bitmap<GreyLevel> *src,Bitmap<ColorRGB> *dst);
void SetAlphaChannel(Bitmap<GreyLevel> *alpha,Bitmap<ColorRGBA> *dst);


template <class color_type1,class color_type2>
void BitmapCopy(Bitmap<color_type1> *dst,Bitmap<color_type2> *src);

template <class color_type1,class color_type2>
void BitmapBlend(Bitmap<color_type1> *dst,Bitmap<color_type2> *src,float blend_factor);

void Invert(Bitmap<GreyLevel> *img);

void MakeRGBA(Bitmap<ColorRGB> *src,Bitmap<GreyLevel> *mask,Bitmap<ColorRGBA> *dst);



void Differences(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B);		// A = |A-B|
void Differences(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B);		// A = |A-B|
void Differences(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,Bitmap<GreyLevel> *Out_Image);
double Differences_d(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B);
double Differences_d(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,int safe_x,int safe_y);
double Differences_d(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B,int safe_x,int safe_y);
void Min(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B);			// A = min(A,B)
void Max(Bitmap<GreyLevel> *A,Bitmap<GreyLevel> *B);			// A = min(A,B)
void ImgMult(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B);		// A = A.*B

void Bool_OR(Bitmap<ColorRGB> *I,Bitmap<GreyLevel> *O);		// O <=> I.r or I.g or I.b




typedef Bitmap<GreyLevel> Image;

























/*	void Clear(BYTE clear);

	Image_Neighbour Neighbour(int x,int y,UINT d);     // d e' il raggio.. sara' 2d
	
	void Invert();
	void AND(Image *b);


	Image *GetEdgeImage();
	void Threshold(int level);
*/

/*

#define PointWeight(x,y) 1			// o gaussiana

class Image_Neighbour {
	Image *source;
	int px,py,d;
public:
	Image_Neighbour(Image *source,int px,int py,UINT d);

	// solo per 8 bit
	double operator*(const Image_Neighbour &b) const;  // normalized cross-correlation 
	void stat(double &media,double &varianza) const;
	
	BYTE Point(int x,int y) const {
		if ((x<-d) || (x>d)) return 0;
		if ((y<-d) || (y>d)) return 0;
		return source->Point(px+x,py+y);
	}
};

*/