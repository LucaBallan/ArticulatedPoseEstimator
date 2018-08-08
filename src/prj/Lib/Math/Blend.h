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







#define DEFAULT_INVALID_WI_VALUE 0


void WTA_FUS(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int _defaultSrc=0);		// Fusion WTA  
void MEAN_FUS(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int _defaultSrc=0);		// Fusion MEAN 
void BlendRecoursive(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int level,			// Fusion RECURSIVE
					 int _defaultSrc=0,bool MEAN_LAST=false,bool regolarize_masks=false);			//			level = 1 ->  !MEAN_LAST  -> Fusion WTA
																									//			               MEAN_LAST  -> Fusion MEAN



void Blend_RGB_Bitmaps(Bitmap<ColorRGB> **Src,Bitmap<ColorFloat> **Mask,int num_src,int level,		// Fusion RECURSIVE of RGB images
					   int _defaultSrc=0,bool MEAN_LAST=false,bool regolarize_masks=false);			//			level = 1 ->  !MEAN_LAST  -> Fusion WTA
																									//			               MEAN_LAST  -> Fusion MEAN



void Blend_RGBA_Bitmaps(Bitmap<ColorRGBA> **Images,int num,int level,int _defaultSrc,				// ALPHA CHANNEL =   0       Invalid
						bool MEAN_LAST,bool regolarize_masks,int _default_src_no_info,				//                   1-255   Valid     1 -> 0.0
						Bitmap<ColorRGB> *out);														//									 255 -> 1.0
																									// _default_src_no_info = -1 Nothing			
																									//						   X set alpha of X to 1 (i.e., mask to 0.0)




void Blend2_RGB_Bitmaps(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,Bitmap<ColorFloat> *Mask,			// Result in A
						ColorFloat MAX_MASK_VAL,int level,int _defaultSrc=0,
						bool MEAN_LAST=false,bool regolarize_masks=false);

