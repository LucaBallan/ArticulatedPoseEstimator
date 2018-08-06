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


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////   UTILITY         ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


bool CheckSegment(Vector<2> o,Vector<2> dir,int Num_Max_Check,bool(*F)(CK_SEG_INFO_STRUCT *,void *),void *UserInfo) {
	CK_SEG_INFO_STRUCT ck_seg_info;
	ck_seg_info.origin=o;
	ck_seg_info.direction=dir;
	ck_seg_info.num_max_iterations=Num_Max_Check;
	ck_seg_info.t=0;
	double sign;

	if (abs(dir[0])>abs(dir[1])) {
		// Disegno lungo x
		if (dir[0]<0) sign=-1;
		else sign=1;
		double dd=sign*dir[1]/dir[0];
		for(int i=0;i<Num_Max_Check;i++) {
			ck_seg_info.P_before=ck_seg_info.P;
			ck_seg_info.P=o.toInteger();
			if (F(&ck_seg_info,UserInfo)) return true;
			ck_seg_info.t++;
			o[0]+=sign;
			o[1]+=dd;
		}
		return false;
	
	} else {
		if (dir[1]==0) return false;
		// Disegno lungo y
		if (dir[1]<0) sign=-1;
		else sign=1;
		double dd=sign*dir[0]/dir[1];
		for(int i=0;i<Num_Max_Check;i++) {
			ck_seg_info.P_before=ck_seg_info.P;
			ck_seg_info.P=o.toInteger();
			if (F(&ck_seg_info,UserInfo)) return true;
			ck_seg_info.t++;
			o[0]+=dd;
			o[1]+=sign;
		}
		return false;

	}

	return false;
}








template <class T>
struct ui_DrawLine_struct {
	T color;
	Bitmap<T> *image;
	float alpha;
};

template <class T>
struct ui_DrawLine_gradient_struct {
	T color1;
	T color2;
	int num;
	Bitmap<T> *image;
	float alpha;
};

bool DrawLine_rt(CK_SEG_INFO_STRUCT *Info,ui_DrawLine_struct<GreyLevel> *ui_Info) {
	GreyLevel c=ui_Info->image->Point((int)(Info->P)[0],(int)(Info->P)[1]);
	c=(GreyLevel)(((1.0-ui_Info->alpha)*c)+(ui_Info->alpha*ui_Info->color));
	ui_Info->image->SetPoint((int)(Info->P)[0],(int)(Info->P)[1],c);
	return false;
}

bool DrawLine_rt_RGB(CK_SEG_INFO_STRUCT *Info,ui_DrawLine_struct<ColorRGB> *ui_Info) {
	ColorRGB c=ui_Info->image->Point((int)(Info->P)[0],(int)(Info->P)[1]);
	c=(c*(1.0-ui_Info->alpha))+(ui_Info->color*ui_Info->alpha);
	ui_Info->image->SetPoint((int)(Info->P)[0],(int)(Info->P)[1],c);
	return false;
}

bool DrawLine_rt_RGB_gradient(CK_SEG_INFO_STRUCT *Info,ui_DrawLine_gradient_struct<ColorRGB> *ui_Info) {
	ColorRGB c=ui_Info->image->Point((int)(Info->P)[0],(int)(Info->P)[1]);
	float grad=Info->t/(1.0f*(ui_Info->num-1));
	ColorRGB x=(ui_Info->color1*(1.0-grad))+(ui_Info->color2*grad);
	c=(c*(1.0-ui_Info->alpha))+(x*ui_Info->alpha);
	ui_Info->image->SetPoint((int)(Info->P)[0],(int)(Info->P)[1],c);
	return false;
}

void DrawLine(Vector<2> p1,Vector<2> p2,GreyLevel color,Bitmap<GreyLevel> *im,float alpha) {
	int num;
	Vector<2> d=p2-p1;
	
	if (abs(d[0])>abs(d[1])) num=Approx(abs(d[0])+1);
	else num=Approx(abs(d[1])+1);
	
	ui_DrawLine_struct<GreyLevel> st_info;
	st_info.color=color;
	st_info.image=im;
	st_info.alpha=alpha;
	CheckSegment(p1,d,num,(CK_SEG_CALLBACK)DrawLine_rt,&st_info);
}

void DrawLine(Vector<2> p1,Vector<2> p2,ColorRGB color,Bitmap<ColorRGB> *im,float alpha) {
	int num;
	Vector<2> d=p2-p1;
	
	if (abs(d[0])>abs(d[1])) num=Approx(abs(d[0])+1);
	else num=Approx(abs(d[1])+1);
	
	ui_DrawLine_struct<ColorRGB> st_info;
	st_info.color=color;
	st_info.image=im;
	st_info.alpha=alpha;
	CheckSegment(p1,d,num,(CK_SEG_CALLBACK)DrawLine_rt_RGB,&st_info);
}

void DrawLine(Vector<2> p1,Vector<2> p2,ColorRGB color1,ColorRGB color2,Bitmap<ColorRGB> *im,float alpha) {
	int num;
	Vector<2> d=p2-p1;
	
	if (abs(d[0])>abs(d[1])) num=Approx(abs(d[0])+1);
	else num=Approx(abs(d[1])+1);
	
	ui_DrawLine_gradient_struct<ColorRGB> st_info;
	st_info.color1=color1;
	st_info.color2=color2;
	st_info.num=num;
	st_info.image=im;
	st_info.alpha=alpha;
	CheckSegment(p1,d,num,(CK_SEG_CALLBACK)DrawLine_rt_RGB_gradient,&st_info);
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class color_type>
Vector<2> ImageGradient(int x,int y,Bitmap<color_type> *im,color_type color,int dim_mask) { 
	int d=(dim_mask-1)/2;
	Vector<2> dir;

	for(int i=-d+x;i<=x+d;i++) {
		for(int j=-d+y;j<=y+d;j++) {
			if (im->Point(i,j)==color) {
				dir[0]+=i-x;
				dir[1]+=j-y;
			}
		}
	}

	return dir;
}
template Vector<2> ImageGradient(int x,int y,Bitmap<ColorFloat> *im,ColorFloat color,int dim_mask);
template Vector<2> ImageGradient(int x,int y,Bitmap<GreyLevel>  *im,GreyLevel  color,int dim_mask);



bool FindGradient(CK_SEG_INFO_STRUCT *Info,ui_FindGradient_struct *ui_Info) {
	Bitmap<GreyLevel> *im=ui_Info->image;

	if (Info->t==0) return false;				// Scarta sempre il primo (nel caso sia quello giusto uso P_before)
	int Px=(int)((Info->P)[0]);
	int Py=(int)((Info->P)[1]);

	if (im->isOutside(Px,Py,((ui_Info->Gradient_Window_Size-1)/2))) return false;

	
	// Condizione necessaria è quella di trovarmi su un punto edge nella sponda color_to_be
	GreyLevel color_to_be,color_before;
	if (ui_Info->Gradient_To_Search*(Info->direction)<0) color_to_be=0x00;
	else color_to_be=0xFF;
	if (im->Point(Px,Py)!=color_to_be) return false;	// Non sono nella sponda color_to_be
	color_before=im->Point((int)((Info->P_before)[0]),(int)((Info->P_before)[1]));	// Sono sicuramente inside perche sono vicino a P che è interno ad una sfera di raggio Gradient_Window_Size
	if (color_to_be==color_before) return false;						// Non sono in un edge

	// Ritorna true se il gradiente attuale è simile a quello cercato
	Vector<2> v=ImageGradient(Px,Py,im,(GreyLevel)0xFF,ui_Info->Gradient_Window_Size);
	if ((v.Versore())*ui_Info->Gradient_To_Search>ui_Info->soglia_tolleranza_gradiente) {
		if (color_to_be==0xFF) ui_Info->finded_point=Info->P_before;
		else ui_Info->finded_point=Info->P;
		return true;
	} 
	return false;
}











////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* OBSOLETE
template <class color_type>
void DrawPoint(Vector<2> p,color_type color,Bitmap<color_type> *im) {
	im->SetPoint(p[0],p[1],color);
}


template void DrawPoint(Vector<2> p,GreyLevel color,Bitmap<GreyLevel> *im);
template void DrawPoint(Vector<2> p,ColorRGB color,Bitmap<ColorRGB> *im);
template void DrawPoint(Vector<2> p,ColorFloat color,Bitmap<ColorFloat> *im);
*/






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MakeGaussian(Bitmap<GreyLevel> *I,double cx,double cy,double rx,double ry,double fall_off,double tollerance,bool invert) {

	double sigmaq=-(1.0-fall_off)*(1.0-fall_off)/log(tollerance);

	for(int i=0;i<I->width;i++) {
		for(int j=0;j<I->height;j++) {
			double lx=(i-cx)/rx;
			double ly=(j-cy)/ry;
			double r=sqrt((lx*lx)+(ly*ly));
			
			double val;
			if (r<fall_off) val=1.0;
			else val=exp(-((r-fall_off)*(r-fall_off)/sigmaq));
			if (invert) val=1.0-val;

			GreyLevel c=(GreyLevel)Approx(val*255.0);
			I->SetPoint(i,j,c);
		}
	}
}


void MakeGaussian(Bitmap<GreyLevel> *I,double fall_off,double tollerance) {
	double cx=(I->width-1.0)/2.0;
	double cy=(I->height-1.0)/2.0;
	double raggio_max=max((I->width-1.0)-cx,(I->height-1.0)-cy);

	double sigmaq=-(1.0-fall_off)*(1.0-fall_off)/log(tollerance);

	for(int i=0;i<I->width;i++) {
		for(int j=0;j<I->height;j++) {
			double r=(sqrt((i-cx)*(i-cx)+(j-cy)*(j-cy)))/raggio_max;         // 0 to 1.0 o piu negli angoli
			
			double val;
			if (r<fall_off) val=1.0;
			else val=exp(-((r-fall_off)*(r-fall_off)/sigmaq));

			GreyLevel c=(GreyLevel)Approx(val*255.0);
			I->SetPoint(i,j,c);
		}
	}

}

void MakeSunDisc(Bitmap<GreyLevel> *I,double disc_ray,double tollerance) {
	double cx=(I->width-1.0)/2.0;
	double cy=(I->height-1.0)/2.0;
	double raggio_max=max((I->width-1.0)-cx,(I->height-1.0)-cy);

	double sigmaq=-1.0/log(tollerance);
	double k=1.0/exp(-(disc_ray*disc_ray/sigmaq));

	for(int i=0;i<I->width;i++) {
		for(int j=0;j<I->height;j++) {
			double r=(sqrt((i-cx)*(i-cx)+(j-cy)*(j-cy)))/raggio_max;         // 0 to 1.0 o piu negli angoli
			
			double val;
			val=k*exp(-(r*r/sigmaq));
			if (val>1.0) val=1.0;

			GreyLevel c=(GreyLevel)Approx(val*255.0);
			I->SetPoint(i,j,c);
		}
	}

}

void MakeGaussian(Bitmap<GreyLevel> *I,HyperBox<2> *box,double fall_off,double tollerance,bool invert) {
	double cx=(box->P1[0]+box->P0[0])/2.0;
	double cy=(box->P1[1]+box->P0[1])/2.0;
	double rx=fabs((box->P1[0]-box->P0[0])/2.0);
	double ry=fabs((box->P1[1]-box->P0[1])/2.0);
	
	MakeGaussian(I,cx,cy,rx,ry,fall_off,tollerance,invert);
}












////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
template <class color_type>
float NCD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size) {
	int half_win=win_size/2;
	float ni=1.0f/(win_size*win_size);

	float cross=0,sum1=0,sum2=0;
	float norm1=0,norm2=0;
	int p1_0=Approx(p1[0]),p1_1=Approx(p1[1]),p2_0=Approx(p2[0]),p2_1=Approx(p2[1]);
	for(int i=-half_win;i<=half_win;i++) {
		for(int j=-half_win;j<=half_win;j++) {
			float d1=ToGrey(img1->Point(p1_0+i,p1_1+j));
			float d2=ToGrey(img2->Point(p2_0+i,p2_1+j));

			cross+=d1*d2;
			sum1+=d1;
			sum2+=d2;
			norm1+=d1*d1;
			norm2+=d2*d2;
		}
	}
	cross=(cross-(sum1*sum2*ni));
	norm1=(norm1-(sum1*sum1*ni));
	norm2=(norm2-(sum2*sum2*ni));
	//cout<<cross<<"  "<<norm1<<"  "<<norm2<<"\n";

	if ((IS_ZERO(norm1,1e-8)) && (IS_ZERO(norm2,1e-8))) return 0.0;
	if (IS_ZERO(norm1,1e-8)) return sqrt(norm2);						// TODO: mmm Data dependent value!!! se i dati sono compresi tra 0 e 1 ok!!
	if (IS_ZERO(norm2,1e-8)) return sqrt(norm1);						// TODO: mmm

	float den=sqrt(norm1*norm2);
	if (IS_ZERO(den,1e-8)) return 0.0;
	cross/=den;
	
	
	if (cross!=cross) {
		cout<<cross<<"  "<<norm1<<"  "<<norm2<<"\n";	
	}
	if (cross<-1.0) return -1.0;
	if (cross>1.0) return 1.0;

	return (1.0f-cross);
}

template <class color_type>
float DPD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size) {
	int half_win=win_size/2;

	float error=0,norm1=0,norm2=0;
	int p1_0=Approx(p1[0]),p1_1=Approx(p1[1]),p2_0=Approx(p2[0]),p2_1=Approx(p2[1]);
	for(int i=-half_win;i<=half_win;i++) {
		for(int j=-half_win;j<=half_win;j++) {
			float d1=ToGrey(img1->Point(p1_0+i,p1_1+j));
			float d2=ToGrey(img2->Point(p2_0+i,p2_1+j));
			
			error+=d1*d2;
			norm1+=d1*d1;
			norm2+=d2*d2;
		}
	}
	if ((norm1==0) && (norm2==0)) return 0.0;
	if (norm1==0) return (sqrt(norm2)/(win_size*win_size));		// TODO: mmm
	if (norm2==0) return (sqrt(norm1)/(win_size*win_size));		// TODO: mmm

	error/=(sqrt(norm1*norm2));

	return (1.0f-error);
}

template <class color_type>
float SAD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size) {
	int half_win=win_size/2;

	float error=0;
	int p1_0=Approx(p1[0]),p1_1=Approx(p1[1]),p2_0=Approx(p2[0]),p2_1=Approx(p2[1]);
	for(int i=-half_win;i<=half_win;i++) {
		for(int j=-half_win;j<=half_win;j++) {
			float d1=ToGrey(img1->Point(p1_0+i,p1_1+j));
			float d2=ToGrey(img2->Point(p2_0+i,p2_1+j));
			
			error+=fabs(d1-d2);
		}
	}
	error/=(win_size*win_size);

	return error;
}

template <class color_type>
float SAD(Bitmap<color_type> *img1,Bitmap<color_type> *img2,Vector<2> p1,Vector<2> p2,int win_size,float (*ColorMetric)(color_type A,color_type B)) {
	int half_win=win_size/2;

	float error=0;
	int p1_0=Approx(p1[0]),p1_1=Approx(p1[1]),p2_0=Approx(p2[0]),p2_1=Approx(p2[1]);
	for(int i=-half_win;i<=half_win;i++) {
		for(int j=-half_win;j<=half_win;j++) {
			error+=ColorMetric(img1->Point(p1_0+i,p1_1+j),img2->Point(p2_0+i,p2_1+j));	
		}
	}
	error/=(win_size*win_size);

	return error;
}
			


template float NCD(Bitmap<ColorFloat> *img1,Bitmap<ColorFloat> *img2,Vector<2> p1,Vector<2> p2,int win_size);
template float DPD(Bitmap<ColorFloat> *img1,Bitmap<ColorFloat> *img2,Vector<2> p1,Vector<2> p2,int win_size);
template float SAD(Bitmap<ColorFloat> *img1,Bitmap<ColorFloat> *img2,Vector<2> p1,Vector<2> p2,int win_size);

template float NCD(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Vector<2> p1,Vector<2> p2,int win_size);
template float DPD(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Vector<2> p1,Vector<2> p2,int win_size);
template float SAD(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Vector<2> p1,Vector<2> p2,int win_size);

template float SAD(Bitmap<ColorRGB> *img1,Bitmap<ColorRGB> *img2,Vector<2> p1,Vector<2> p2,int win_size,float (*ColorMetric)(ColorRGB A,ColorRGB B));
