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





///
/// MEGLIO SU TEXTURE!!!!!!!!! COPIA!!
///

#define WAVELET_FLOAT_PRECISION 1000000			// 0.000001


template <class input_color_type>
class Wavelet {
	int width,height;
	int HalfWidth,HalfHeight;

public:
	Bitmap<WI_Type> *HH,*HL;
	Bitmap<WI_Type> *LH,*LL;
	

	Wavelet(int width,int height);
	~Wavelet();

	void Transform(Bitmap<input_color_type> *In);
	void AntiTransform(Bitmap<input_color_type> *Out,bool DeleteOld=true);					// DeleteOld delete the bands -> Frees memory immediately 
};



















//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************


template <class dest_type>
class Wavelet_ori {
	int width,height;

public:
	Bitmap<dest_type> *In;
	Bitmap<dest_type> *High,*Low;
	

	Wavelet_ori(int width,int height);
	~Wavelet_ori();

	void Process();
	void Process(int start_x,int end_x);
	void Process(Bitmap<ColorRGB> *In);
	void Synthetize(Bitmap<ColorRGB> *Out);

	void GetBitmapLow(Bitmap<ColorRGB> *out);
	void GetBitmapHigh(Bitmap<ColorRGB> *out);
	void GetBitmapLow(Bitmap<ColorRGBA> *out);
	void GetBitmapHigh(Bitmap<ColorRGBA> *out);

	void SetBitmapLow(Bitmap<ColorRGB> *in);
	void SetBitmapHigh(Bitmap<ColorRGB> *in);
	void SetBitmapLow(Bitmap<ColorRGBA> *in);
	void SetBitmapHigh(Bitmap<ColorRGBA> *in);

	bool IsValid();
};




template <class src_type,class dest_type>
void analyze_lpf(src_type inSig[], int inOff, int inLen, int inStep, 
				 dest_type lowSig[], int lowOff, int lowStep,
				 dest_type highSig[], int highOff, int highStep);

template <class src_type,class dest_type>
void synthetize_lpf(dest_type lowSig[], int lowOff, int lowLen, int lowStep,
				    dest_type highSig[], int highOff, int highLen, int highStep, 
				    src_type outSig[], int outOff, int outStep);


template <class src_type,class dest_type>
void analyze_lpf_fast(src_type inSig[], int inOff, int inLen, int inStep, 
				      dest_type lowSig[], int lowOff, int lowStep,
				      dest_type highSig[], int highOff, int highStep);

template <class src_type,class dest_type>
void synthetize_lpf_fast(dest_type lowSig[], int lowOff, int lowLen, int lowStep,
				    dest_type highSig[], int highOff, int highLen, int highStep, 
				    src_type outSig[], int outOff, int outStep);



