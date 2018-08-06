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

void Expand2(Bitmap<ColorRGB> *TextureSimple,Bitmap<ColorFloat> *TextureMask) {

	// Orizontal
	for(int j=0;j<TextureMask->height;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureMask->width;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(i,j)!=0.0) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint((i-1),j,(ColorFloat)0.00001);
						TextureSimple->SetPoint((i-1),j,TextureSimple->Point(i,j));
					}
				}
			} else {
				if (TextureMask->Point(i,j)==0.0) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(i,j,(ColorFloat)0.00001);
						TextureSimple->SetPoint(i,j,TextureSimple->Point((i-1),j));
					}
				}
			
			}
		}
	}


	// Vertical
	for(int j=0;j<TextureMask->width;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureMask->height;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(j,i)!=0.0) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint(j,(i-1),(ColorFloat)0.00001);
						TextureSimple->SetPoint(j,i-1,TextureSimple->Point(j,i));
					}
				}
			} else {
				if (TextureMask->Point(j,i)==0.0) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(j,i,(ColorFloat)0.00001);
						TextureSimple->SetPoint(j,i,TextureSimple->Point(j,i-1));
					}
				}
			
			}
		}
	}

}

/// NOT USED!!!!!
Bitmap<ColorFloat> *ResizeMask(Bitmap<ColorFloat> *Mask) {
	int valid;
	float media;
	float In[4];
	Bitmap<ColorFloat> *RMask=new Bitmap<ColorFloat>(Mask->width/2,Mask->height/2);


	for(int i=0;i<Mask->width/2;i++) {
		for(int j=0;j<Mask->height/2;j++) {
			// [j,i]
			In[0]=Mask->Point(2*i,2*j);
			In[1]=Mask->Point(2*i+1,2*j);
			In[2]=Mask->Point(2*i,2*j+1);
			In[3]=Mask->Point(2*i+1,2*j+1);
			

			// Media Pesata
			valid=0;
			media=0;
			for(int k=0;k<4;k++) {
				if (In[k]>=0) {			// TODO** era > ho messo >=
					valid++;
					media+=In[k];
				}
			}

			if (valid<=2) {
				RMask->SetPoint(i,j,-1);
			} else {
				media/=4.0;								// NB: in questo modo, pesa meno se non copre tutto il quadrato (Assunzione)
				RMask->SetPoint(i,j,media);
			}
		}
	}

	return RMask;
}


/// THIS IS THE NEW ONE!!!
void ResizeMasks(Bitmap<ColorFloat> **Mask,int num_src,Bitmap<ColorFloat> **RMask,bool regolarize_masks) {
	int valid,mask_width,mask_height;
	float media;
	float In[4];
	ColorFloat *buff,*c_buff;

	for(int index=0;index<num_src;index++) {
		mask_width=Mask[index]->width;
		mask_height=Mask[index]->height;
		buff=Mask[index]->getBuffer();

		RMask[index]=new Bitmap<ColorFloat>((mask_width>>1),(mask_height>>1));

		for(int i=0;i<(mask_width>>1);i++) {
			for(int j=0;j<(mask_height>>1);j++) {
				// [j,i]
				c_buff=buff+((i<<1)+((j<<1)*mask_width));
				In[0]=*(c_buff);							// Mask[index]->Point(2*i,2*j);
				In[1]=*(c_buff+1);							// Mask[index]->Point(2*i+1,2*j);
				In[2]=*(c_buff+mask_width);					// Mask[index]->Point(2*i,2*j+1);
				In[3]=*(c_buff+1+mask_width);				// Mask[index]->Point(2*i+1,2*j+1);

				// Media Pesata
				valid=0;
				media=0;
				for(int k=0;k<4;k++) {
					if (In[k]>=0.0) {
						valid++;
						media+=In[k];
					}
				}
				
				//
				// - X		<- Conservativo -> Se vi e' un solo punto invalido
				// X X		   Invalida tutto!!
				//
				if (valid!=4) {
					RMask[index]->SetPoint(i,j,-1);
				} else {
					media/=4.0;							// NB: in questo modo, pesa meno se non copre tutto il quadrato (Assunzione)
					RMask[index]->SetPoint(i,j,media);
				}
			}
		}

		if (regolarize_masks) RMask[index]->Gaussian();		// TODO**   Regolarizzo il vincitore -> Pietro, NON DEVE TOCCARE GLI INVALIDI!!!

	}

}



/*void ResizeMasks(Bitmap<ColorFloat> **Mask,int num_src,Bitmap<ColorFloat> **RMask) {
	
	Wavelet<ColorFloat> X(Mask[0]->width,Mask[0]->height);
	X.Transform(Mask[0]);


	RMask[0]=new Bitmap<ColorFloat>(Mask[0]->width/2,Mask[0]->height/2);
	RMask[1]=new Bitmap<ColorFloat>(Mask[1]->width/2,Mask[1]->height/2);

	
	ColorFloat *obuf0=RMask[0]->getBuffer();
	ColorFloat *obuf1=RMask[1]->getBuffer();
	WI_Type *Ibuf=X.LL->getBuffer();
	
	for(int i=0;i<RMask[0]->width*RMask[0]->height;i++) {
		*obuf0=(((ColorFloat)(*(Ibuf++)))/WAVELET_FLOAT_PRECISION)+0.5f;
		*obuf1=1.0f-(*obuf0);
		obuf0++;
		obuf1++;
	}


	static int counter=0;
	char Text[500];
	sprintf(Text,"G:\\res\\t%0i.jpg",counter);
//	RMask[0]->Save(Text);
	counter++;

}
*/



//void ResizeMasks(Bitmap<ColorFloat> **Mask,int num_src,Bitmap<ColorFloat> **RMask) {
//	float In[4];
//	
//	RMask[0]=new Bitmap<ColorFloat>(Mask[0]->width/2,Mask[0]->height/2);
//	RMask[1]=new Bitmap<ColorFloat>(Mask[1]->width/2,Mask[1]->height/2);
//
//	
//	for(int i=0;i<Mask[0]->width/2;i++) {
//		for(int j=0;j<Mask[0]->height/2;j++) {
//			// [j,i]
//			In[0]=Mask[0]->Point(2*i,2*j);
//			In[1]=Mask[0]->Point(2*i+1,2*j);
//			In[2]=Mask[0]->Point(2*i,2*j+1);
//			In[3]=Mask[0]->Point(2*i+1,2*j+1);
//			
//			RMask[0]->SetPoint(i,j,0.0);
//			RMask[1]->SetPoint(i,j,1.0);
//			
//			float media=0.0;
//			for(int k=0;k<4;k++) {
//				media+=(In[k]/4.0);
//				/*if (In[k]>0) {
//					RMask[0]->SetPoint(i,j,1.0);
//					RMask[1]->SetPoint(i,j,0.0);
//					break;
//				}*/
//			}
//			RMask[0]->SetPoint(i,j,media);
//			RMask[1]->SetPoint(i,j,1.0-media);
//		}
//	}
//
//	static int counter=0;
//	char Text[500];
//	sprintf(Text,"G:\\res\\t%0i.jpg",counter);
//	//RMask[0]->Save(Text);
//	counter++;
//
//
//
//	/*	for(int index=0;index<num_src;index++) {
//		RMask[index]=new Bitmap<ColorFloat>(Mask[index]->width/2,Mask[index]->height/2);
//
//		for(int i=0;i<Mask->width/2;i++) {
//			for(int j=0;j<Mask->height/2;j++) {
//				// [j,i]
//				In[0]=Mask->Point(2*i,2*j);
//				In[1]=Mask->Point(2*i+1,2*j);
//				In[2]=Mask->Point(2*i,2*j+1);
//				In[3]=Mask->Point(2*i+1,2*j+1);
//			
//
//				// Media Pesata
//				valid=0;
//				media=0;
//				for(int k=0;k<4;k++) {
//					if (In[k]>=0) {
//						valid++;
//						media+=In[k];
//					}
//				}
//
//				// 0 1 2 3 4
//				// X X X o o 
//				if (valid<=2) {
//					RMask->SetPoint(i,j,-1);
//				} else {
//					media/=4.0;								// NB: in questo modo, pesa meno se non copre tutto il quadrato (Assunzione)
//					RMask->SetPoint(i,j,media);
//				}
//			}
//		}
//	}
//	*/
//}

void WTA_FUS(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int _defaultSrc) {
	int width=Src[0]->width;
	int height=Src[0]->height;	

	// Controllo coerenza dimensioni
	for(int i=0;i<num_src;i++) {
		if ((width!=Src[i]->width) || (height!=Src[i]->height) || (width!=Mask[i]->width) || (height!=Mask[i]->height)) 
			ErrorExit("Blender: Source and destionation sizes do not match.");
	}

	for(int i=0;i<width;i++) {
		for(int j=0;j<height;j++) {
			
			int best_choice=_defaultSrc;
			ColorFloat best_value=Mask[best_choice]->Point(i,j);
			
			for(int s=0;s<num_src;s++) {
				if (Mask[s]->Point(i,j)>best_value) {
					best_value=Mask[s]->Point(i,j);
					best_choice=s;
				}
			}
			
			if (best_value<0) {
				// Nessuno dato valido
				Src[0]->SetPoint(i,j,DEFAULT_INVALID_WI_VALUE);
				//Mask[0]->SetPoint(i,j,best_value);		// TODO: mettere tutto come una funzione a parte, altrimenti c'e' conflitto con i vari stage e calcolo la stessa cosa + volte anche se nn mi serve
			} else {
				Src[0]->SetPoint(i,j,Src[best_choice]->Point(i,j));
				//Mask[0]->SetPoint(i,j,best_value);
			}
		}
	}

}

void MEAN_FUS(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int _defaultSrc) {
	int width=Src[0]->width;
	int height=Src[0]->height;	

	// Controllo coerenza dimensioni
	for(int i=0;i<num_src;i++) {
		if ((width!=Src[i]->width) || (height!=Src[i]->height) || (width!=Mask[i]->width) || (height!=Mask[i]->height)) 
			ErrorExit("Blender: Source and destionation sizes do not match.");
	}

	for(int i=0;i<width;i++) {
		for(int j=0;j<height;j++) {
			
			float media=0;
			float factor=0;
			int valid_src=0;
			for(int s=0;s<num_src;s++) {
				ColorFloat peso=Mask[s]->Point(i,j);
				if (peso>=0) {
					media+=((ColorFloat)Src[s]->Point(i,j))*peso;
					factor+=peso;
					valid_src++;
				}
			}
			if (valid_src==0) {
				// Nessuno dato valido
				Src[0]->SetPoint(i,j,DEFAULT_INVALID_WI_VALUE);
				//Mask[0]->SetPoint(i,j,-1);
			} else {
				if (factor==0.0) {
					// tutti i pesi sono uguali e nulli
					// medio tutti con media normale
					media=0;
					for(int s=0;s<num_src;s++) {
						if (Mask[s]->Point(i,j)>=0) 
							media+=((ColorFloat)Src[s]->Point(i,j))/((ColorFloat)1.0*valid_src);
					}
					Src[0]->SetPoint(i,j,(WI_Type)media);
				} else {
					media/=factor;
					Src[0]->SetPoint(i,j,(WI_Type)media);
				}
				//Mask[0]->SetPoint(i,j,(factor/valid_src));
			}
		}
	}

}


void BlendRecoursive(Bitmap<WI_Type> **Src,Bitmap<ColorFloat> **Mask,int num_src,int level,int _defaultSrc,bool MEAN_LAST,bool regolarize_masks) {
	int width=Src[0]->width;
	int height=Src[0]->height;	

	// Controllo coerenza dimensioni
	for(int i=0;i<num_src;i++) {
		if ((width!=Src[i]->width) || (height!=Src[i]->height) || (width!=Mask[i]->width) || (height!=Mask[i]->height)) 
			ErrorExit("Blender: Source and destionation sizes do not match.");
	}


	// Livello > 1
	if (level>1) {
		if (width%2) ErrorExit("Blender: image width is not pair.");
		if (height%2) ErrorExit("Blender: image height is not pair.");

		Wavelet<WI_Type> **X=NULL;
		Bitmap<ColorFloat> **RMask=NULL;
		
		SNEWA_P(X,Wavelet<WI_Type>*,num_src);
		SNEWA_P(RMask,Bitmap<ColorFloat>*,num_src);

		for(int i=0;i<num_src;i++) {
			X[i]=NULL;
			SNEW(X[i],Wavelet<WI_Type>(width,height));
			
			X[i]->Transform(Src[i]);
		}
		ResizeMasks(Mask,num_src,RMask,regolarize_masks);	
		
		// Fusione
		Bitmap<WI_Type> **LL=new Bitmap<WI_Type>*[num_src];
		Bitmap<WI_Type> **LH=new Bitmap<WI_Type>*[num_src];
		Bitmap<WI_Type> **HL=new Bitmap<WI_Type>*[num_src];
		Bitmap<WI_Type> **HH=new Bitmap<WI_Type>*[num_src];
		for(int k=0;k<num_src;k++) {
			LL[k]=X[k]->LL;
			LH[k]=X[k]->LH;
			HL[k]=X[k]->HL;
			HH[k]=X[k]->HH;
		}

/*		for(int k=0;k<num_src;k++) {	//TODO**
			#define SAVEI(Image,name,a,b) {char filename[300];sprintf(filename,name,a,b);Image->Save(filename);}
			Bitmap<GreyLevel> A(width/2,height/2);
			A.CopyFrom(LH[k]);
			SAVEI((&A),"G:\\res\\%i_%i_LH.jpg",k,level);
			A.CopyFrom(HL[k]);
			SAVEI((&A),"G:\\res\\%i_%i_HL.jpg",k,level);
			A.CopyFrom(HH[k]);
			SAVEI((&A),"G:\\res\\%i_%i_HH.jpg",k,level);
			A.CopyFrom(LL[k]);
			SAVEI((&A),"G:\\res\\%i_%i_LL.jpg",k,level);
		}*/
		
		WTA_FUS(LH,RMask,num_src,_defaultSrc);
		WTA_FUS(HL,RMask,num_src,_defaultSrc);
		WTA_FUS(HH,RMask,num_src,_defaultSrc);
		BlendRecoursive(LL,RMask,num_src,level-1,_defaultSrc,MEAN_LAST,regolarize_masks);

		// X[0] Contiene il risultato
		X[0]->AntiTransform(Src[0]);

		for(int i=0;i<num_src;i++) {
			SDELETE(RMask[i]);
			SDELETE(X[i]);
		}
		SDELETEA(RMask);
		SDELETEA(X);
	} else {
		if (MEAN_LAST) MEAN_FUS(Src,Mask,num_src,_defaultSrc);
		else WTA_FUS(Src,Mask,num_src,_defaultSrc);
	}
}



void Blend_RGB_Bitmaps(Bitmap<ColorRGB> **Src,Bitmap<ColorFloat> **Mask,int num_src,int level,int _defaultSrc,bool MEAN_LAST,bool regolarize_masks) {
	Bitmap<WI_Type> **CSrc=NULL;
	SNEWA_P(CSrc,Bitmap<WI_Type>*,num_src);

	// Red
	for(int i=0;i<num_src;i++) {
		CSrc[i]=new Bitmap<WI_Type>(Src[i]->width,Src[i]->height);
		
		ColorRGB  *src_p=Src[i]->getBuffer();
		WI_Type   *dst_p=CSrc[i]->getBuffer();
		int        dim=(Src[i]->width)*(Src[i]->height);

		for(int j=0;j<dim;j++,src_p++,dst_p++) (*dst_p)=((((WI_Type)src_p->r)-((WI_Type)128)))<<16;		//TODO**
	
	}
	BlendRecoursive(CSrc,Mask,num_src,level,_defaultSrc,MEAN_LAST,regolarize_masks);
	{
		WI_Type   *src_p=CSrc[0]->getBuffer();
		ColorRGB  *dst_p=Src[0]->getBuffer();
		int        dim=(Src[0]->width)*(Src[0]->height);
		for(int i=0;i<dim;i++,src_p++,dst_p++) {
			WI_Type x=((*src_p)>>16)+((WI_Type)128);			//TODO** e tutto sotto!!! modificare!!!!
			if (x<0) x=0;
			if (x>255) x=255;
			dst_p->r=(BYTE)x;
		}
	}
	

	// Green
	for(int i=0;i<num_src;i++) {
		ColorRGB  *src_p=Src[i]->getBuffer();
		WI_Type   *dst_p=CSrc[i]->getBuffer();
		int        dim=(Src[i]->width)*(Src[i]->height);

		for(int j=0;j<dim;j++,src_p++,dst_p++) (*dst_p)=((((WI_Type)src_p->g)-((WI_Type)128)))<<16;

	}
	BlendRecoursive(CSrc,Mask,num_src,level,_defaultSrc,MEAN_LAST,regolarize_masks);
	{
		WI_Type   *src_p=CSrc[0]->getBuffer();
		ColorRGB  *dst_p=Src[0]->getBuffer();
		int        dim=(Src[0]->width)*(Src[0]->height);
		for(int i=0;i<dim;i++,src_p++,dst_p++) {
			WI_Type x=((*src_p)>>16)+((WI_Type)128);
			if (x<0) x=0;
			if (x>255) x=255;
			dst_p->g=(BYTE)x;
		}
	}



	// Blue
	for(int i=0;i<num_src;i++) {
		ColorRGB  *src_p=Src[i]->getBuffer();
		WI_Type   *dst_p=CSrc[i]->getBuffer();
		int        dim=(Src[i]->width)*(Src[i]->height);

		for(int j=0;j<dim;j++,src_p++,dst_p++) (*dst_p)=((((WI_Type)src_p->b)-((WI_Type)128)))<<16;

	}
	BlendRecoursive(CSrc,Mask,num_src,level,_defaultSrc,MEAN_LAST,regolarize_masks);
	{
		WI_Type   *src_p=CSrc[0]->getBuffer();
		ColorRGB  *dst_p=Src[0]->getBuffer();
		int        dim=(Src[0]->width)*(Src[0]->height);
		for(int i=0;i<dim;i++,src_p++,dst_p++) {
			WI_Type x=((*src_p)>>16)+((WI_Type)128);
			if (x<0) x=0;
			if (x>255) x=255;
			dst_p->b=(BYTE)x;
		}
	}

	for(int i=0;i<num_src;i++) SDELETE(CSrc[i]);
	SDELETEA(CSrc);
}


void Blend2_RGB_Bitmaps(Bitmap<ColorRGB> *A,Bitmap<ColorRGB> *B,Bitmap<ColorFloat> *Mask,ColorFloat MAX_MASK_VAL,int level,int _defaultSrc,bool MEAN_LAST,bool regolarize_masks) {
	Bitmap<ColorRGB> **Src=NULL;
	Bitmap<ColorFloat> **Masks=NULL;
	SNEWA_P(Src,Bitmap<ColorRGB>*,2);
	SNEWA_P(Masks,Bitmap<ColorFloat>*,2);
	
	// Src
	Src[0]=A;Src[1]=B;
	
	// Masks
	Masks[0]=Mask;
	Masks[1]=NULL;

	////
/*	Bitmap<ColorFloat> *Ciao=NULL;	//TODO**
	SNEW(Ciao,Bitmap<ColorFloat>(Masks[0]->width,Masks[0]->height));	// TODO**	
	Ciao->CopyFrom(Masks[0],false);
	Ciao->Threshold(0.05);						//TODO***
	ColorFloat *src_p=Ciao->getBuffer();
	ColorRGB *dst_p=Src[0]->getBuffer();
	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=MAX_MASK_VAL-(*src_p);
	//for(int times=0;times<90;times++) Expand2(Src[1],Ciao);		// TODO**
	SDELETE(Ciao);*/
	////

	SNEW(Masks[1],Bitmap<ColorFloat>(Masks[0]->width,Masks[0]->height));
	int dim=(Masks[0]->width)*(Masks[0]->height);
	ColorFloat *src_p=Masks[0]->getBuffer();
	ColorFloat *dst_p=Masks[1]->getBuffer();
	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=MAX_MASK_VAL-(*src_p);
	

	Src[0]->Save("G:\\TT0.jpg");
	Src[1]->Save("G:\\TT1.jpg");
	Blend_RGB_Bitmaps(Src,Masks,2,level,_defaultSrc,MEAN_LAST,regolarize_masks);
	Src[0]->Save("G:\\TT2.jpg");

	SDELETEA(Src);
	SDELETE(Masks[1]);
	SDELETEA(Masks);
}


void Blend_RGBA_Bitmaps(Bitmap<ColorRGBA> **Images,int num,int level,int _defaultSrc,bool MEAN_LAST,bool regolarize_masks,int _default_src_no_info,Bitmap<ColorRGB> *out) {
	

	Bitmap<ColorRGB> **Src=NULL;
	Bitmap<ColorFloat> **Masks=NULL;
	SNEWA_P(Src,Bitmap<ColorRGB>*,num);
	SNEWA_P(Masks,Bitmap<ColorFloat>*,num);
	
	// Src
	for(int i=0;i<num;i++) {
		if (i==0) Src[i]=out;
		else SNEW(Src[i],Bitmap<ColorRGB>(out->width,out->height));
		Src[i]->CopyFrom(Images[i]);
	}
	for(int i=0;i<num;i++) {
		SNEW(Masks[i],Bitmap<ColorFloat>(out->width,out->height));
		Images[i]->ExtractChannel(3,Masks[i]);
	}
	

	for(int i=0;i<num;i++) {
		int n_ele=Masks[i]->width*Masks[i]->height;
		ColorFloat *buff=Masks[i]->getBuffer();

		if (_default_src_no_info==i) {
			for(int j=0;j<n_ele;j++,buff++) (*buff)=0.0;
		} else {
			for(int j=0;j<n_ele;j++,buff++) {
				if ((*buff)==0.0) (*buff)=-1.0;
				else (*buff)=((*buff)-(ColorFloat)1.0)/(ColorFloat)254.0;
			}
		}
	}


	// TODO**
	static int index=0;		// TODO***
	for(int i=0;i<num;i++) {
		IMAGE_SAVE_2I(Src[i],"g:\\I%i_%i.bmp",i,index);
		
		Masks[i]->SAVE_FLOAT_MIN_VAL=0.0;
		Masks[i]->SAVE_FLOAT_MAX_VAL=1.0;
		IMAGE_SAVE_2I(Masks[i],"g:\\M%i_%i.bmp",i,index);
	}
	index++;



	///
	/*SNEW(Masks[1],Bitmap<ColorFloat>(Masks[0]->width,Masks[0]->height));
	int dim=(Masks[0]->width)*(Masks[0]->height);
	ColorFloat *src_p=Masks[0]->getBuffer();
	ColorFloat *dst_p=Masks[1]->getBuffer();
	for(int i=0;i<dim;i++,src_p++,dst_p++) (*dst_p)=MAX_MASK_VAL-(*src_p);*/
	

	Blend_RGB_Bitmaps(Src,Masks,num,level,_defaultSrc,MEAN_LAST,regolarize_masks);
	

	Src[0]=NULL;		// do not delete out
	SDELETEA_REC(Src,num);
	SDELETEA_REC(Masks,num);
}





















///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// TODO
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









// TODO
#define DEFAULT_COLOR_R 0
#define DEFAULT_COLOR_G 0
#define DEFAULT_COLOR_B 0



void _3ChannelBlenderBest(Bitmap<ColorRGB> **Src,Bitmap<float> **Mask,int num_src,Bitmap<ColorRGB> *Out,Bitmap<float> *OutMask,int _defaultSrc=0) {

	int width=Out->width;
	int height=Out->height;	

	for(int i=0;i<height;i++) {
		for(int j=0;j<width;j++) {
			float BestCost=-FLT_MAX;
			int Best=_defaultSrc;
			
			for(int k=0;k<num_src;k++) {
				if (Mask[k]->Point(j,i)>BestCost) {
					BestCost=Mask[k]->Point(j,i);
					Best=k;
				}
			}
			if (BestCost<0) {
				Best=_defaultSrc;
				BestCost=Mask[_defaultSrc]->Point(j,i);
			}

			Out->SetPoint(j,i,(Src[Best]->Point(j,i)));
			OutMask->SetPoint(j,i,BestCost);
		
			if (BestCost<0) {
				ColorRGB x;x.r=255;x.g=x.b=0;
				Out->SetPoint(j,i,x);
			}
		
		}

	}

}


void _3ChannelBlenderAverage(Bitmap<ColorRGB> **Src,Bitmap<float> **Mask,int num_src,Bitmap<ColorRGB> *Out,Bitmap<float> *OutMask,int _defaultSrc=0) {

	int width=Out->width;
	int height=Out->height;	

	for(int i=0;i<height;i++) {
		for(int j=0;j<width;j++) {
			float Cost=0;
			int num_valid=0;
			float r=0,g=0,b=0;
		
			for(int k=0;k<num_src;k++) {
				float peso=Mask[k]->Point(j,i);
				if (peso>=0) {
					r+=peso*Src[k]->Point(j,i).r;
					g+=peso*Src[k]->Point(j,i).g;
					b+=peso*Src[k]->Point(j,i).b;
					Cost+=peso;
					num_valid++;
				}
			}

			if (!num_valid) {
				Out->SetPoint(j,i,(Src[_defaultSrc]->Point(j,i)));
				OutMask->SetPoint(j,i,(Mask[_defaultSrc]->Point(j,i)));
			} else {
				r/=Cost;
				g/=Cost;
				b/=Cost;
				Cost/=num_valid;
				ColorRGB x((BYTE)Approx(r),(BYTE)Approx(g),(BYTE)Approx(b));													// TODO: OCCHIO STRASBORDI
				Out->SetPoint(j,i,x);
				OutMask->SetPoint(j,i,Cost);
			}

		
		}

	}

}





//
//
// width, height devono essere le stesse per ogni immagine/maschera e Output compreso
// width, height devono essere pari
// 
//
#include "Expand.h"

void Media(WI_Type **Vals,float *Pesi,int num_src,WI_Type &R,WI_Type &G,WI_Type &B,float &peso);
void Max(WI_Type **Vals,float *Pesi,int num_src,WI_Type &R,WI_Type &G,WI_Type &B,float &peso);

void _3ChannelBlenderPiramidal(Bitmap<ColorRGB> **Src,Bitmap<float> **Mask,int num_src,Bitmap<ColorRGB> *Out,Bitmap<float> *OutMask,int _defaultSrc=0) {

	int width=Out->width;
	int height=Out->height;	
	if (width%2) ErrorExit("width is not pair");
	if (height%2) ErrorExit("height is not pair");

	for(int i=0;i<num_src;i++) {
		if (width!=Src[i]->width) return;
		if (height!=Src[i]->height) return;
	}


	Wavelet<ColorRGB> **X=new Wavelet<ColorRGB>*[num_src];
	Bitmap<float> **RMask=new Bitmap<float> *[num_src];

	for(int i=0;i<num_src;i++) {
		X[i]=new Wavelet<ColorRGB>(width,height);
		
		X[i]->Transform(Src[i]);
		RMask[i]=ResizeMask(Mask[i]);
	}


	
	// Fusione
	WI_Type **LL=new WI_Type*[num_src];
	WI_Type **LH=new WI_Type*[num_src];
	WI_Type **HL=new WI_Type*[num_src];
	WI_Type **HH=new WI_Type*[num_src];
	for(int k=0;k<num_src;k++) {
		LL[k]=(*(X[k]->LL)).getBuffer();
		LH[k]=(*(X[k]->LH)).getBuffer();
		HL[k]=(*(X[k]->HL)).getBuffer();
		HH[k]=(*(X[k]->HH)).getBuffer();
	}

	WI_Type **CLL=new WI_Type*[num_src];
	WI_Type **CLH=new WI_Type*[num_src];
	WI_Type **CHL=new WI_Type*[num_src];
	WI_Type **CHH=new WI_Type*[num_src];
	float *Pesi=new float[num_src];
	
	WI_Type R,G,B;
	float RPeso;

	for(int i=0;i<width/2;i++) {
		for(int j=0;j<height/2;j++) {
			// (i,j)
			for(int k=0;k<num_src;k++) {
				CLL[k]=&(LL[k][(3*i)+(j*3*width/2)]);
				CHL[k]=&(HL[k][(3*i)+(j*3*width/2)]);
				CLH[k]=&(LH[k][(3*i)+(j*3*width/2)]);
				CHH[k]=&(HH[k][(3*i)+(j*3*width/2)]);
				Pesi[k]=RMask[k]->Point(i,j);
			}
			
			// CLL[k][0],CLL[k][1],CLL[k][3]   Le componenti RGB di (k,LL)
			Media(CLL,Pesi,num_src,R,G,B,RPeso);
			(*X[0]->LL).SetPoint(3*i,j,R);
			(*X[0]->LL).SetPoint(3*i+1,j,G);
			(*X[0]->LL).SetPoint(3*i+2,j,B);

			(*OutMask).SetPoint(2*i,2*j,RPeso);				// TODO: Quale peso? Metto quello della banda base
			(*OutMask).SetPoint(2*i+1,2*j,RPeso);
			(*OutMask).SetPoint(2*i,2*j+1,RPeso);
			(*OutMask).SetPoint(2*i+1,2*j+1,RPeso);

			Max(CLH,Pesi,num_src,R,G,B,RPeso);
			(*X[0]->LH).SetPoint(3*i,j,R);
			(*X[0]->LH).SetPoint(3*i+1,j,G);
			(*X[0]->LH).SetPoint(3*i+2,j,B);
			Max(CHL,Pesi,num_src,R,G,B,RPeso);
			(*X[0]->HL).SetPoint(3*i,j,R);
			(*X[0]->HL).SetPoint(3*i+1,j,G);
			(*X[0]->HL).SetPoint(3*i+2,j,B);
			Max(CHH,Pesi,num_src,R,G,B,RPeso);
			(*X[0]->HH).SetPoint(3*i,j,R);
			(*X[0]->HH).SetPoint(3*i+1,j,G);
			(*X[0]->HH).SetPoint(3*i+2,j,B);
		
		}
	}


	// Espando prima della sintesi (solo X[0]->LL)
	// uso RMask[0]
	for(int i=0;i<width/2;i++) {
		for(int j=0;j<height/2;j++) {
			RMask[0]->SetPoint(i,j,OutMask->Point(2*i,2*j));
		}
	}
	Expand(X[0]->LL,RMask[0]);


	// Antitrasformo
	X[0]->AntiTransform(Out);
	

	for(int i=0;i<num_src;i++) {
		delete RMask[i];
		delete X[i];
	}
	delete[]RMask;
	delete[]X;
	delete[]LL;
	delete[]LH;
	delete[]HL;
	delete[]HH;
	delete[]CLL;
	delete[]CLH;
	delete[]CHL;
	delete[]CHH;				// TODO cancella tutto o qlc in piu? e con delete []??
	// Pesi??
}


void Media(WI_Type **Vals,float *Pesi,int num_src,WI_Type &R,WI_Type &G,WI_Type &B,float &peso) {
	
	peso=0;
	double mediaR=0;
	double mediaG=0;
	double mediaB=0;

	for(int k=0;k<num_src;k++) {
		if (Pesi[k]<0) continue;
		
		peso+=Pesi[k];
		mediaR+=(double)Vals[k][0]*Pesi[k];
		mediaG+=(double)Vals[k][1]*Pesi[k];
		mediaB+=(double)Vals[k][2]*Pesi[k];
	}
	
	if (peso==0) {
		R=DEFAULT_COLOR_R;
		G=DEFAULT_COLOR_G;
		B=DEFAULT_COLOR_B;
		peso=-1;
		return;
	}

	R=Approx(mediaR/peso);
	G=Approx(mediaG/peso);
	B=Approx(mediaB/peso);
}

void Max(WI_Type **Vals,float *Pesi,int num_src,WI_Type &R,WI_Type &G,WI_Type &B,float &peso) {

	int maxK=-1;
	float maxPeso=-1;

	for(int k=0;k<num_src;k++) {
		if (maxPeso<=Pesi[k]) {
			maxPeso=Pesi[k];
			maxK=k;
		}
	}
	
	if (maxPeso<0) {
		R=DEFAULT_COLOR_R;
		G=DEFAULT_COLOR_G;
		B=DEFAULT_COLOR_B;
		peso=-1;
		return;
	}

	R=Vals[maxK][0];
	G=Vals[maxK][1];
	B=Vals[maxK][2];
	peso=Pesi[maxK];
}
