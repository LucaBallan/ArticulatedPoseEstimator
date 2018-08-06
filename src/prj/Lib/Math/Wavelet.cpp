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


template Wavelet<ColorRGB>;
template Wavelet<WI_Type>;
template Wavelet<ColorFloat>;
template Wavelet_ori<int>;




//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************


template <class input_color_type>
Wavelet<input_color_type>::Wavelet(int width,int height) {
	this->width=width;
	this->height=height;
	
	HalfWidth=width/2;
	if (width&1) HalfWidth++;
	HalfHeight=height/2;
	if (height&1) HalfHeight++;

	LL=HL=LH=HH=NULL;
}

template <class input_color_type>
Wavelet<input_color_type>::~Wavelet() {
	SDELETE(LL);
	SDELETE(HL);
	SDELETE(LH);
	SDELETE(HH);
}

template <class dest_type>
void ColorBitmapToTypeBuffer(Bitmap<ColorRGB> *Src,dest_type *buff) {
	
	BYTE *src=(BYTE *)Src->getBuffer();
	int dim=3*Src->width*Src->height;

	for(int i=0;i<dim;i++)
		buff[i]=((DWORD)src[i])-128;
	
}

template <class dest_type>
void ColorBitmapToTypeBuffer(Bitmap<ColorFloat> *Src,dest_type *buff) {
	
	ColorFloat *src=Src->getBuffer();
	int dim=Src->width*Src->height;

	for(int i=0;i<dim;i++)
		buff[i]=(DWORD)(WAVELET_FLOAT_PRECISION*(src[i]-0.5));
	
}

//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************


template <>
void Wavelet<ColorRGB>::Transform(Bitmap<ColorRGB> *InBmp) {
	Bitmap<WI_Type> *H=NULL,*L=NULL,*Temp=NULL;
	WI_Type *desthigh,*destlow,*src;
	

	SNEW(Temp,Bitmap<WI_Type>(3*width,height));
	ColorBitmapToTypeBuffer(InBmp,Temp->getBuffer());

	//
	// Process Temp -> H,L
	//
	SNEW(H,Bitmap<WI_Type>(3*HalfWidth,height));
	SNEW(L,Bitmap<WI_Type>(3*HalfWidth,height));
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=Temp->getBuffer();

	for(int i=0;i<height;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,width,3,destlow,0,3,desthigh,0,3);
		analyze_lpf<WI_Type,WI_Type>(src,1,width,3,destlow,1,3,desthigh,1,3);
		analyze_lpf<WI_Type,WI_Type>(src,2,width,3,destlow,2,3,desthigh,2,3);

		src+=3*width;
		destlow+=3*HalfWidth;
		desthigh+=3*HalfWidth;
	}
	SDELETE(Temp);


	//
	// Process H -> HH,HL
	//
	SNEW(HH,Bitmap<WI_Type>(3*HalfWidth,HalfHeight));
	SNEW(HL,Bitmap<WI_Type>(3*HalfWidth,HalfHeight));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,3*HalfWidth,destlow,0,3*HalfWidth,desthigh,0,3*HalfWidth);
		analyze_lpf<WI_Type,WI_Type>(src,1,height,3*HalfWidth,destlow,1,3*HalfWidth,desthigh,1,3*HalfWidth);
		analyze_lpf<WI_Type,WI_Type>(src,2,height,3*HalfWidth,destlow,2,3*HalfWidth,desthigh,2,3*HalfWidth);
		
		src+=3;
		destlow+=3;
		desthigh+=3;
	}
	SDELETE(H);


	//
	// Process L -> LH,LL
	//
	SNEW(LH,Bitmap<WI_Type>(3*HalfWidth,HalfHeight));
	SNEW(LL,Bitmap<WI_Type>(3*HalfWidth,HalfHeight));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,3*HalfWidth,destlow,0,3*HalfWidth,desthigh,0,3*HalfWidth);
		analyze_lpf<WI_Type,WI_Type>(src,1,height,3*HalfWidth,destlow,1,3*HalfWidth,desthigh,1,3*HalfWidth);
		analyze_lpf<WI_Type,WI_Type>(src,2,height,3*HalfWidth,destlow,2,3*HalfWidth,desthigh,2,3*HalfWidth);
		
		src+=3;
		destlow+=3;
		desthigh+=3;
	}
	SDELETE(L);
	
}

template <class dest_type>
inline BYTE Clamp(dest_type x) {
	x=x+128;

	if (x<0) x=0;
	if (x>255) x=255;

	return (BYTE)x;
}


template <>
void Wavelet<ColorRGB>::AntiTransform(Bitmap<ColorRGB> *out,bool DeleteOld) {
	Bitmap<WI_Type> *H=NULL,*L=NULL,*Temp=NULL;
	WI_Type *desthigh,*destlow,*src;

	//
	// Process LH,LL -> L
	//
	SNEW(L,Bitmap<WI_Type>(3*HalfWidth,height));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,3*HalfWidth,desthigh,0,HalfHeight,3*HalfWidth,src,0,3*HalfWidth);
		synthetize_lpf<WI_Type,WI_Type>(destlow,1,HalfHeight,3*HalfWidth,desthigh,1,HalfHeight,3*HalfWidth,src,1,3*HalfWidth);
		synthetize_lpf<WI_Type,WI_Type>(destlow,2,HalfHeight,3*HalfWidth,desthigh,2,HalfHeight,3*HalfWidth,src,2,3*HalfWidth);
		
		src+=3;
		destlow+=3;
		desthigh+=3;
	}
	if (DeleteOld) {
		SDELETE(LL);
		SDELETE(LH);
	}



	//
	// Process HH,HL -> H
	//
	SNEW(H,Bitmap<WI_Type>(3*HalfWidth,height));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,3*HalfWidth,desthigh,0,HalfHeight,3*HalfWidth,src,0,3*HalfWidth);
		synthetize_lpf<WI_Type,WI_Type>(destlow,1,HalfHeight,3*HalfWidth,desthigh,1,HalfHeight,3*HalfWidth,src,1,3*HalfWidth);
		synthetize_lpf<WI_Type,WI_Type>(destlow,2,HalfHeight,3*HalfWidth,desthigh,2,HalfHeight,3*HalfWidth,src,2,3*HalfWidth);
		
		src+=3;
		destlow+=3;
		desthigh+=3;
	}
	if (DeleteOld) {
		SDELETE(HH);
		SDELETE(HL);
	}



	//
	// H,L -> Temp
	//
	SNEW(Temp,Bitmap<WI_Type>(3*width,height));	
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=Temp->getBuffer();

	for(int i=0;i<height;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfWidth,3,desthigh,0,HalfWidth,3,src,0,3);
		synthetize_lpf<WI_Type,WI_Type>(destlow,1,HalfWidth,3,desthigh,1,HalfWidth,3,src,1,3);
		synthetize_lpf<WI_Type,WI_Type>(destlow,2,HalfWidth,3,desthigh,2,HalfWidth,3,src,2,3);

		src+=3*width;
		destlow+=3*HalfWidth;
		desthigh+=3*HalfWidth;
	}
	SDELETE(H);
	SDELETE(L);


	ColorRGB *obuf=out->getBuffer();
	WI_Type *Ibuf=Temp->getBuffer();
	
	for(int i=0;i<width*height;i++) {
		obuf->r=Clamp(*(Ibuf++));
		obuf->g=Clamp(*(Ibuf++));
		obuf->b=Clamp(*(Ibuf++));
		obuf++;
	}
	
	SDELETE(Temp);
}

template <>
void Wavelet<WI_Type>::Transform(Bitmap<WI_Type> *InBmp) {
	Bitmap<WI_Type> *H=NULL,*L=NULL;
	WI_Type *desthigh,*destlow,*src;
	

	//
	// Process Temp -> H,L
	//
	SNEW(H,Bitmap<WI_Type>(HalfWidth,height));
	SNEW(L,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=InBmp->getBuffer();

	for(int i=0;i<height;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,width,1,destlow,0,1,desthigh,0,1);

		src+=width;
		destlow+=HalfWidth;
		desthigh+=HalfWidth;
	}


	//
	// Process H -> HH,HL
	//
	SNEW(HH,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	SNEW(HL,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,HalfWidth,destlow,0,HalfWidth,desthigh,0,HalfWidth);

		src++;
		destlow++;
		desthigh++;
	}
	SDELETE(H);


	//
	// Process L -> LH,LL
	//
	SNEW(LH,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	SNEW(LL,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,HalfWidth,destlow,0,HalfWidth,desthigh,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	SDELETE(L);
	
}

template <>
void Wavelet<WI_Type>::AntiTransform(Bitmap<WI_Type> *out,bool DeleteOld) {
	Bitmap<WI_Type> *H=NULL,*L=NULL;
	WI_Type *desthigh,*destlow,*src;

	//
	// Process LH,LL -> L
	//
	SNEW(L,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,HalfWidth,desthigh,0,HalfHeight,HalfWidth,src,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	if (DeleteOld) {
		SDELETE(LL);
		SDELETE(LH);
	}



	//
	// Process HH,HL -> H
	//
	SNEW(H,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,HalfWidth,desthigh,0,HalfHeight,HalfWidth,src,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	if (DeleteOld) {
		SDELETE(HH);
		SDELETE(HL);
	}



	//
	// H,L -> Temp
	//
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=out->getBuffer();

	for(int i=0;i<height;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfWidth,1,desthigh,0,HalfWidth,1,src,0,1);

		src+=width;
		destlow+=HalfWidth;
		desthigh+=HalfWidth;
	}
	SDELETE(H);
	SDELETE(L);

}



















template <>
void Wavelet<ColorFloat>::Transform(Bitmap<ColorFloat> *InBmp) {
	Bitmap<WI_Type> *H=NULL,*L=NULL,*Temp=NULL;
	WI_Type *desthigh,*destlow,*src;
	
	SNEW(Temp,Bitmap<WI_Type>(width,height));
	ColorBitmapToTypeBuffer(InBmp,Temp->getBuffer());

	//
	// Process Temp -> H,L
	//
	SNEW(H,Bitmap<WI_Type>(HalfWidth,height));
	SNEW(L,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=Temp->getBuffer();

	for(int i=0;i<height;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,width,1,destlow,0,1,desthigh,0,1);

		src+=width;
		destlow+=HalfWidth;
		desthigh+=HalfWidth;
	}
	SDELETE(Temp);


	//
	// Process H -> HH,HL
	//
	SNEW(HH,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	SNEW(HL,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,HalfWidth,destlow,0,HalfWidth,desthigh,0,HalfWidth);

		src++;
		destlow++;
		desthigh++;
	}
	SDELETE(H);


	//
	// Process L -> LH,LL
	//
	SNEW(LH,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	SNEW(LL,Bitmap<WI_Type>(HalfWidth,HalfHeight));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		analyze_lpf<WI_Type,WI_Type>(src,0,height,HalfWidth,destlow,0,HalfWidth,desthigh,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	SDELETE(L);
	
}

template <>
void Wavelet<ColorFloat>::AntiTransform(Bitmap<ColorFloat> *out,bool DeleteOld) {
	Bitmap<WI_Type> *H=NULL,*L=NULL,*Temp=NULL;
	WI_Type *desthigh,*destlow,*src;

	//
	// Process LH,LL -> L
	//
	SNEW(L,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=LH->getBuffer();
	destlow=LL->getBuffer();
	src=L->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,HalfWidth,desthigh,0,HalfHeight,HalfWidth,src,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	if (DeleteOld) {
		SDELETE(LL);
		SDELETE(LH);
	}



	//
	// Process HH,HL -> H
	//
	SNEW(H,Bitmap<WI_Type>(HalfWidth,height));
	desthigh=HH->getBuffer();
	destlow=HL->getBuffer();
	src=H->getBuffer();

	for(int i=0;i<HalfWidth;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfHeight,HalfWidth,desthigh,0,HalfHeight,HalfWidth,src,0,HalfWidth);
		
		src++;
		destlow++;
		desthigh++;
	}
	if (DeleteOld) {
		SDELETE(HH);
		SDELETE(HL);
	}



	//
	// H,L -> Temp
	//
	SNEW(Temp,Bitmap<WI_Type>(width,height));	
	desthigh=H->getBuffer();
	destlow=L->getBuffer();
	src=Temp->getBuffer();

	for(int i=0;i<height;i++) {
		synthetize_lpf<WI_Type,WI_Type>(destlow,0,HalfWidth,1,desthigh,0,HalfWidth,1,src,0,1);

		src+=width;
		destlow+=HalfWidth;
		desthigh+=HalfWidth;
	}
	SDELETE(H);
	SDELETE(L);


	ColorFloat *obuf=out->getBuffer();
	WI_Type *Ibuf=Temp->getBuffer();
	
	for(int i=0;i<width*height;i++) {
		(*obuf)=(((ColorFloat)(*(Ibuf++)))/WAVELET_FLOAT_PRECISION)+0.5f;
		obuf++;
	}
	
	SDELETE(Temp);
}






























//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************

// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------- WAVELET -------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// sia con inLen pari che dispari




template <class src_type,class dest_type>
void synthetize_lpf(dest_type lowSig[], int lowOff, int lowLen, int lowStep,
				    dest_type highSig[], int highOff, int highLen, int highStep, 
				    src_type outSig[], int outOff, int outStep) {

	int i;
	int outLen = lowLen + highLen; //Length of the output signal
	int iStep = 2*outStep; //Upsampling in outSig
	int ik; //Indexing outSig
	int lk; //Indexing lowSig
	int hk; //Indexing highSig  

	/*
	*Generate even samples (inverse low-pass filter)
	*/

	//Initialize counters
	lk = lowOff;
	hk = highOff;
	ik = outOff;

	//Handle tail boundary effect. Use symmetric extension.
	if(outLen>1) {
		outSig[ik] = (src_type)(lowSig[lk] - ((highSig[hk]+(dest_type)1)>>1));
	} else {
		outSig[ik] = (src_type)(lowSig[lk]);
	}

	lk += lowStep;  
	hk += highStep;
	ik += iStep;

	//Apply lifting step to each "inner" sample.
	for(i = 2; i < outLen-1; i += 2) {
		outSig[ik] = (src_type)(lowSig[lk] - ((highSig[hk-highStep] + highSig[hk] + (dest_type)2)>>2));

		lk += lowStep;  
		hk += highStep;
		ik += iStep;
	}

	//Handle head boundary effect if input signal has odd length.
	if((outLen % 2 == 1)&&(outLen>2)) {
		outSig[ik] = (src_type)(lowSig[lk] - ((2*highSig[hk-highStep]+(dest_type)2)>>2));
	}

	/*
	*Generate odd samples (inverse high pass-filter)
	*/

	//Initialize counters
	hk = highOff;
	ik = outOff + outStep;

	//Apply first lifting step to each "inner" sample.
	for(i = 1; i < outLen-1; i += 2) {           
		// Since signs are inversed (add instead of substract)
		// the +1 rounding dissapears.
		outSig[ik] = (src_type)(highSig[hk] + (((dest_type)outSig[ik-outStep] + (dest_type)outSig[ik+outStep]) >> 1));

		hk += highStep;
		ik += iStep;   
	}

	//Handle head boundary effect if input signal has even length.
	if( outLen%2==0 && outLen>1) {
		outSig[ik] = (src_type)(highSig[hk] + (dest_type)outSig[ik-outStep]);
	} 
}





template <class src_type,class dest_type>
void analyze_lpf(src_type inSig[], int inOff, int inLen, int inStep, 
				 dest_type lowSig[], int lowOff, int lowStep,
				 dest_type highSig[], int highOff, int highStep) {
	int i;
	int iStep = 2 * inStep; //Subsampling in inSig
	int ik; //Indexing inSig
	int lk; //Indexing lowSig
	int hk; //Indexing highSig


	/*
	*Generate high frequency subband
	*/

	//Initialize counters
	ik = inOff + inStep;
	hk = highOff;

	//Apply first lifting step to each "inner" sample.
	for(i = 1; i < inLen-1; i += 2) {           
		highSig[hk] = (dest_type)inSig[ik] - (((dest_type)inSig[ik-inStep] + (dest_type)inSig[ik+inStep])>>1);
		
		ik += iStep;   
		hk += highStep;
	}

	//Handle head boundary effect if input signal has even length.
	if( inLen % 2 == 0 ) {
		highSig[hk] = (dest_type)inSig[ik] - ((2*(dest_type)inSig[ik-inStep])>>1);
	}

	/*
	*Generate low frequency subband
	*/

	//Initialize counters
	ik = inOff;
	lk = lowOff;
	hk = highOff;

	if(inLen>1) {
		lowSig[lk] = (dest_type)inSig[ik] + ((highSig[hk] + (dest_type)1)>>1);
	}
	else {
		lowSig[lk] = (dest_type)inSig[ik];
	}

	ik += iStep;
	lk += lowStep;  
	hk += highStep;

	//Apply lifting step to each "inner" sample.
	for(i = 2; i < inLen-1; i += 2) {
		lowSig[lk] = inSig[ik] + 
			((highSig[hk-highStep] + highSig[hk] + (dest_type)2)>> 2);
		
		ik += iStep;
		lk += lowStep;  
		hk += highStep;
	}

	//Handle head boundary effect if input signal has odd length.
	if(inLen % 2 == 1) {
		if(inLen>2) {
			lowSig[lk] = (dest_type)inSig[ik] + ((2*highSig[hk-highStep]+(dest_type)2)>>2);
		}
	}    
}



// -----------------------------------------------------------------------------------------------------------------------
// ------------------------------------------ FAST WAVELET ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
// Solo per inLen pari



template <class src_type,class dest_type>
void analyze_lpf_fast(src_type inSig[], int inOff, int inLen, int inStep, 
				      dest_type lowSig[], int lowOff, int lowStep,
				      dest_type highSig[], int highOff, int highStep) {
	

	int i;
	int iStep=inStep<<1;					//Subsampling in inSig
	int num;
	src_type *in,*in_pre,*in_post;
	dest_type *out_h,*out_l,*out_h_pre;


	/*
	*Generate high frequency subband
	*/

	//Initialize counters
	in_pre=inSig+inOff;
	in=in_pre+inStep;
	in_post=in+inStep;
	out_h=highSig+highOff;

	
	//Apply first lifting step to each "inner" sample.
	num=(inLen>>1)-1;
	for(i=num;i>0;i--) {           
		(*out_h) = (dest_type)(*in) - (((dest_type)(*in_pre) + (dest_type)(*in_post))>>1);
		
		in+=iStep;
		in_pre+=iStep;
		in_post+=iStep;
		out_h+=highStep;
	}


	//Handle head boundary effect if input signal has even length.
	(*out_h) = ((dest_type)(*in)) - ((dest_type)(*in_pre));

	/*
	*Generate low frequency subband
	*/

	//Initialize counters
	in=inSig+inOff;
	out_l=lowSig+lowOff;
	out_h=highSig+highOff;

	if(inLen>1) {
		(*out_l) = (dest_type)(*in) + (((*out_h) + (dest_type)1)>>1);
	} else {
		(*out_l) = (dest_type)(*in);
	}

	
	in+=iStep;
	out_l+=lowStep;
	out_h_pre=out_h;
	out_h+=highStep;
	

	num=((inLen-3)>>1)+1;
	for(i=num;i>0;i--) {           
		(*out_l) = ((dest_type)(*in)) + (((*out_h_pre) + (*out_h) + (dest_type)2)>>2);
		
		in+=iStep;
		out_l+=lowStep;
		out_h_pre=out_h;
		out_h+=highStep;
	}

}

template <class src_type,class dest_type>
void synthetize_lpf_fast(dest_type lowSig[], int lowOff, int lowLen, int lowStep,
				    dest_type highSig[], int highOff, int highLen, int highStep, 
				    src_type outSig[], int outOff, int outStep) {

	int i;
	int num;
	int outLen = lowLen + highLen; //Length of the output signal
	int iStep = outStep<<1;		   //Upsampling in outSig

	src_type *out,*out_pre,*out_post;
	dest_type *low,*high,*high_pre;
	

	/*
	*Generate even samples (inverse low-pass filter)
	*/

	//Initialize counters
	out=outSig+outOff;
	low=lowSig+lowOff;
	high=highSig+highOff;


	//Handle tail boundary effect. Use symmetric extension.
	if(outLen>1) {
		(*out) = (src_type)((*low) - (((*high)+(dest_type)1)>>1));
	} else {
		(*out) = (src_type)(*low);
	}

	low+=lowStep;
	high_pre=high;
	high+=highStep;
	out+=iStep;

	//Apply lifting step to each "inner" sample.
	num=((outLen-3)>>1)+1;
	for(i=num;i>0;i--) {           
		(*out) = (src_type)((*low) - (((*high_pre) + (*high) + (dest_type)2)>>2));

		low+=lowStep;
		high_pre=high;
		high+=highStep;
		out+=iStep;
	}


	/*
	*Generate odd samples (inverse high pass-filter)
	*/

	//Initialize counters
	high=highSig+highOff;
	out_pre=outSig+outOff;
	out=out_pre+outStep;
	out_post=out+outStep;


	//Apply first lifting step to each "inner" sample.
	num=(outLen>>1)-1;
	for(i=num;i>0;i--) {           
		// Since signs are inversed (add instead of substract)
		// the +1 rounding dissapears.
		(*out) = (src_type)((*high) + (((dest_type)(*out_pre) + (dest_type)(*out_post)) >> 1));

		high+=highStep;
		out+=iStep;
		out_pre+=iStep;
		out_post+=iStep;
	}

	//Handle head boundary effect if input signal has even length.
	if (outLen>1) {
		(*out) = (src_type)((*high) + (dest_type)(*out_pre));
	} 
}


template <class src_type,class dest_type>
void synthetize_lpf_fast_3(dest_type lowSig[], int lowLen,
				    dest_type highSig[],int highLen,
				    src_type outSig[] ) {

	int i;
	int num;
	int outLen = lowLen + highLen; //Length of the output signal

	src_type *out,*out_pre,*out_post;
	dest_type *low,*high,*high_pre;
	

	/*
	*Generate even samples (inverse low-pass filter)
	*/

	//Initialize counters
	out=outSig;
	low=lowSig;
	high=highSig;
	high_pre=high;


	//Handle tail boundary effect. Use symmetric extension.
	if(outLen>1) {
		(*out) = (src_type)((*low) - (((*high)+(dest_type)1)>>1));
		low++;high++;out++;
		(*out) = (src_type)((*low) - (((*high)+(dest_type)1)>>1));
		low++;high++;out++;
		(*out) = (src_type)((*low) - (((*high)+(dest_type)1)>>1));
		low++;high++;out++;
	} else {
		(*out) = (src_type)(*low);
		low++;high++;out++;
		(*out) = (src_type)(*low);
		low++;high++;out++;
		(*out) = (src_type)(*low);
		low++;high++;out++;
	}

	out+=3;

	//Apply lifting step to each "inner" sample.
	num=((outLen-3)>>1)+1;
	for(i=num;i>0;i--) {           
		/*__asm {
			MOV EAX,low
			MOV EBX,high_pre
			MOV EDX,high

			MOVQ mm0,[EAX]
			MOVQ mm1,[EAX+8]

		}*/
		(*out) = (src_type)((*low) - (((*high_pre) + (*high) + (dest_type)2)>>2));
		low++;high_pre++;high++;out++;
		(*out) = (src_type)((*low) - (((*high_pre) + (*high) + (dest_type)2)>>2));
		low++;high_pre++;high++;out++;
		(*out) = (src_type)((*low) - (((*high_pre) + (*high) + (dest_type)2)>>2));
		low++;high_pre++;high++;out++;

		out+=3;
	}


	/*
	*Generate odd samples (inverse high pass-filter)
	*/

	//Initialize counters
	high=highSig;
	out_pre=outSig;
	out=out_pre+3;
	out_post=out+3;


	//Apply first lifting step to each "inner" sample.
	num=(outLen>>1)-1;
	for(i=num;i>0;i--) {           
		// Since signs are inversed (add instead of substract)
		// the +1 rounding dissapears.
		(*out) = (src_type)((*high) + (((dest_type)(*out_pre) + (dest_type)(*out_post)) >> 1));
		high++;out++;out_pre++;out_post++;
		(*out) = (src_type)((*high) + (((dest_type)(*out_pre) + (dest_type)(*out_post)) >> 1));
		high++;out++;out_pre++;out_post++;
		(*out) = (src_type)((*high) + (((dest_type)(*out_pre) + (dest_type)(*out_post)) >> 1));
		high++;out++;out_pre++;out_post++;

		out+=3;
		out_pre+=3;
		out_post+=3;
	}

	//Handle head boundary effect if input signal has even length.
	if (outLen>1) {
		(*out) = (src_type)((*high) + (dest_type)(*out_pre));
		high++;out++;out_pre++;
		(*out) = (src_type)((*high) + (dest_type)(*out_pre));
		high++;out++;out_pre++;
		(*out) = (src_type)((*high) + (dest_type)(*out_pre));
		high++;out++;out_pre++;
	} 
}








template <class dest_type>
void ColorBitmapToTypeBuffer(Bitmap<ColorRGBA> *Src,dest_type *buff) {
	
	BYTE *src=(BYTE *)Src->getBuffer();
	int dim=Src->width*Src->height;

	for(int i=0;i<dim;i++) {
		*(buff++)=((DWORD)*(src++))-128;
		*(buff++)=((DWORD)*(src++))-128;
		*(buff++)=((DWORD)*(src++))-128;
		src++;
	}
	
}



//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************
//******************************************************************************************************

template <class dest_type>
Wavelet_ori<dest_type>::Wavelet_ori(int width,int height) {
	High=new Bitmap<dest_type>(3*width/2,height);
	Low=new Bitmap<dest_type>(3*width/2,height);
	In=new Bitmap<dest_type>(3*width,height);
	this->width=width;
	this->height=height;
}

template <class dest_type>
Wavelet_ori<dest_type>::~Wavelet_ori() {
	delete High;
	delete Low;
	delete In;
}

template <class dest_type>
bool Wavelet_ori<dest_type>::IsValid() {
	return ((In->IsValid()) && (Low->IsValid()) && (High->IsValid()));
}


template <class dest_type>
void Wavelet_ori<dest_type>::Process() {

	int i;
	int linestep=3*width;
	int linestephalf=linestep>>1;
	dest_type *destlow=Low->getBuffer();
	dest_type *desthigh=High->getBuffer();
	dest_type *src=In->getBuffer();

	for(i=height;i>0;i--) {
		analyze_lpf_fast(src,0,width,3,destlow,0,3,desthigh,0,3);
		analyze_lpf_fast(src,1,width,3,destlow,1,3,desthigh,1,3);
		analyze_lpf_fast(src,2,width,3,destlow,2,3,desthigh,2,3);
		
		src+=linestep;
		destlow+=linestephalf;
		desthigh+=linestephalf;
	}

}

template <class dest_type>
void Wavelet_ori<dest_type>::Process(int start_x,int end_x) {

	int i;
	int linestep=3*width;
	int linestephalf=linestep>>1;
	int real_width=(end_x-start_x+1);
	int tmp=(start_x>>1)*3;
	dest_type *destlow=Low->getBuffer()+tmp;
	dest_type *desthigh=High->getBuffer()+tmp;
	dest_type *src=In->getBuffer()+(start_x*3);


	for(i=height;i>0;i--) {
		analyze_lpf_fast(src,0,real_width,3,destlow,0,3,desthigh,0,3);
		analyze_lpf_fast(src,1,real_width,3,destlow,1,3,desthigh,1,3);
		analyze_lpf_fast(src,2,real_width,3,destlow,2,3,desthigh,2,3);

		src+=linestep;
		destlow+=linestephalf;
		desthigh+=linestephalf;
	}

}

template <class dest_type>
void Wavelet_ori<dest_type>::Process(Bitmap<ColorRGB> *InBmp) {

	ColorBitmapToTypeBuffer(InBmp,In->getBuffer());

	int linestep=3*width;
	int linestephalf=linestep>>1;
	dest_type *destlow=Low->getBuffer();
	dest_type *desthigh=High->getBuffer();
	dest_type *src=In->getBuffer();

	for(int i=0;i<height;i++) {
		analyze_lpf_fast(src,0,width,3,destlow,0,3,desthigh,0,3);
		analyze_lpf_fast(src,1,width,3,destlow,1,3,desthigh,1,3);
		analyze_lpf_fast(src,2,width,3,destlow,2,3,desthigh,2,3);
		
		src+=linestep;
		destlow+=linestephalf;
		desthigh+=linestephalf;
	}

}

template <class dest_type>
void Wavelet_ori<dest_type>::SetBitmapLow(Bitmap<ColorRGB> *in) {
	ColorBitmapToTypeBuffer(in,Low->getBuffer());
}

template <class dest_type>
void Wavelet_ori<dest_type>::SetBitmapHigh(Bitmap<ColorRGB> *in) {
	ColorBitmapToTypeBuffer(in,High->getBuffer());
}

template <class dest_type>
void Wavelet_ori<dest_type>::SetBitmapLow(Bitmap<ColorRGBA> *in) {
	ColorBitmapToTypeBuffer(in,Low->getBuffer());
}

template <class dest_type>
void Wavelet_ori<dest_type>::SetBitmapHigh(Bitmap<ColorRGBA> *in) {
	ColorBitmapToTypeBuffer(in,High->getBuffer());
}



template <class dest_type>
void Wavelet_ori<dest_type>::GetBitmapLow(Bitmap<ColorRGB> *out) {
		
	ColorRGB *obuf=out->getBuffer();
	dest_type *Ibuf=Low->getBuffer();
	
	for(int i=0;i<(width*height>>1);i++) {
		obuf->r=Clamp(*(Ibuf++));
		obuf->g=Clamp(*(Ibuf++));
		obuf->b=Clamp(*(Ibuf++));
		obuf++;
	}
	
}

template <class dest_type>
void Wavelet_ori<dest_type>::GetBitmapHigh(Bitmap<ColorRGB> *out) {

	ColorRGB *obuf=out->getBuffer();
	dest_type *Ibuf=High->getBuffer();
	
	for(int i=0;i<(width*height>>1);i++) {
		obuf->r=Clamp(*(Ibuf++));
		obuf->g=Clamp(*(Ibuf++));
		obuf->b=Clamp(*(Ibuf++));
		obuf++;
	}

}

template <class dest_type>
void Wavelet_ori<dest_type>::GetBitmapLow(Bitmap<ColorRGBA> *out) {
		
	BYTE *obuf=(BYTE *)out->getBuffer();
	dest_type *Ibuf=Low->getBuffer();
	
	for(int i=0;i<(width*height>>1);i++) {
		*(obuf++)=Clamp(*(Ibuf++));
		*(obuf++)=Clamp(*(Ibuf++));
		*(obuf++)=Clamp(*(Ibuf++));
		obuf++;
	}
	
}

template <class dest_type>
void Wavelet_ori<dest_type>::GetBitmapHigh(Bitmap<ColorRGBA> *out) {

	BYTE *obuf=(BYTE *)out->getBuffer();
	dest_type *Ibuf=High->getBuffer();
	
	for(int i=0;i<(width*height>>1);i++) {
		*(obuf++)=Clamp(*(Ibuf++));
		*(obuf++)=Clamp(*(Ibuf++));
		*(obuf++)=Clamp(*(Ibuf++));
		obuf++;
	}

}


#ifdef _WIN64
#include <intrin.h>
inline unsigned int TimeStamp() {
	unsigned int t = (unsigned int)__rdtsc();
	return t;
}
#else
inline unsigned int TimeStamp() {
	unsigned int t;
	__asm {
		PUSH EAX
		PUSH EDX
		_emit 0x0f _asm _emit 0x31 // RDTSC
		MOV t, EAX
		POP EDX
		POP EAX
	}
	return t;
}
#define SSE
#endif

template <class dest_type>
void Wavelet_ori<dest_type>::Synthetize(Bitmap<ColorRGB> *out) {

	int i;
	int linestep=3*width;
	int linestephalf=linestep>>1;
	int halfwidth=width>>1;
	dest_type *destlow=Low->getBuffer();
	dest_type *desthigh=High->getBuffer();
	dest_type *src=In->getBuffer();

	for(i=height;i>0;i--) {
		//synthetize_lpf_fast_3(destlow,halfwidth,desthigh,halfwidth,src);
		synthetize_lpf_fast(destlow,0,halfwidth,3,desthigh,0,halfwidth,3,src,0,3);
		synthetize_lpf_fast(destlow,1,halfwidth,3,desthigh,1,halfwidth,3,src,1,3);
		synthetize_lpf_fast(destlow,2,halfwidth,3,desthigh,2,halfwidth,3,src,2,3);

		src+=linestep;
		destlow+=linestephalf;
		desthigh+=linestephalf;
	}
	
	// In -> Out
	ColorRGB *obuf=out->getBuffer();
	dest_type *Ibuf=In->getBuffer();
	


	// Only if dest = __int32
	// width*height must be divisible by 8

	BYTE *out_=(BYTE *)obuf;


#ifdef SSE
	short less[]={+128,+128,+128,+128};
	i=(width*height*3)>>3;
	
	__asm {
			MOV EDX,8
			MOV EBX,8*4
			MOV ECX,i
			MOV EAX,Ibuf
			MOV EDI,out_
			SUB EDI,EDX
			MOVQ mm4,less

		back_re:
			MOVQ mm0,[EAX]
			MOVQ mm1,[EAX+8]
			ADD EDI,EDX
			MOVQ mm2,[EAX+16]
			MOVQ mm3,[EAX+24]

			PACKSSDW mm0,mm1
			PACKSSDW mm2,mm3

			PADDW mm0,mm4
			PADDW mm2,mm4
			
			ADD EAX,EBX

			PACKUSWB mm0,mm2
			MOVQ [EDI],mm0
			
			LOOP back_re
			
			EMMS
	}
#else 
#ifdef SSE2
	short less[]={+128,+128,+128,+128,+128,+128,+128,+128};
	i=(width*height*3)>>4;

	__asm {
			MOV EDX,16
			MOV EBX,16*4
			MOV ECX,i
			MOV EAX,Ibuf
			MOV EDI,out_
			MOVDQU xmm4,less						// o allineati?

		back_re:
			MOVDQU xmm0,[EAX]						// o allineati?
			MOVDQU xmm1,[EAX+16]
			MOVDQU xmm2,[EAX+32]
			MOVDQU xmm3,[EAX+48]

			PACKSSDW xmm0,xmm1
			PACKSSDW xmm2,xmm3

			PADDW xmm0,xmm4
			PADDW xmm2,xmm4
			
			ADD EAX,EBX

			PACKUSWB xmm0,xmm2
			MOVDQU [EDI],xmm0
			
			ADD EDI,EDX

			LOOP back_re
			
			EMMS
	}
#else 
	for(i=width*height;i>0;i--) {
		obuf->r=Clamp(*(Ibuf++));
		obuf->g=Clamp(*(Ibuf++));
		obuf->b=Clamp(*(Ibuf++));
		obuf++;
	}
#endif
#endif

}







template void analyze_lpf(WI_Type inSig[], int inOff, int inLen, int inStep, 
				 WI_Type lowSig[], int lowOff, int lowStep,
				 WI_Type highSig[], int highOff, int highStep);

