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
using namespace std;

#define T0T1_ORDER(a,b) 	if (t0<=t1) {					\
								if (t<=t0) return a;		\
								if (t>=t1) return b;		\
							} else {						\
								if (t>=t0) return a;		\
								if (t<=t1) return b;		\
							}								\
							t=(t-t0)/(t1-t0);				

#define T0T1_ORDERM 		if (t0<=t1) {				\
								if (t<=t0) {			\
									c->Set(a);			\
									return;				\
								}						\
								if (t>=t1) {			\
									c->Set(b);			\
									return;				\
								}						\
							} else {					\
								if (t>=t0) {			\
									c->Set(a);			\
									return;				\
								}						\
								if (t<=t1) {			\
									c->Set(b);			\
									return;				\
								}						\
							}							\
							t=(t-t0)/(t1-t0);			




double Cyclic_Lin_Interp(double a,double b,double t,double module,double t0,double t1) {
	T0T1_ORDER(a,b);

	a=Module(a,module);
	b=Module(b,module);

	double d1=fabs(b-a),d2;
	if (b<a) {
		d2=b+(module-a);
	} else {
		d2=a+(module-b);
	}
	
	if (d1<d2) return R_Lin_Interp(a,b,t,t0,t1);
	
	double c;
	if (b<a) c=R_Lin_Interp(-(module-a),b,t,t0,t1);
	c=R_Lin_Interp(a,-(module-b),t,t0,t1);

	return Module(c,module);
}

double R_Lin_Interp(double a,double b,double t,double t0,double t1) {
	T0T1_ORDER(a,b);

	return ((b-a)*t+a);
}

double R_Cos_Interp(double a,double b,double t,double t0,double t1) {
	T0T1_ORDER(a,b);

	return (((b-a)*0.5*(1-cos(t*M_PI)))+a);
}

double R_Ripple_Interp(double V00,double V10,double t,double V05,double t0,double t1) {
	return R_Poly_Interp(V00,V10,t,V05,0.0,0.0,0.0,t0,t1);
}

double R_Poly_Interp(double V00,double V10,double t,double V05,double D00,double D05,double D10,double t0,double t1) {
	T0T1_ORDER(V00,V10);

	double w= 4*D10 + 16*D05 +  4*D00 - 24*V10          + 24*V00;
	double z=-8*D10 - 40*D05 - 12*D00 + 52*V10 + 16*V05 - 68*V00;
	double a= 5*D10 + 32*D05 + 13*D00 - 34*V10 - 32*V05 + 66*V00;
	double b=-1*D10 -  8*D05 -  6*D00 +  7*V10 + 16*V05 - 23*V00;
	double c=D00;
	double d=V00;

	return (d+t*(c+t*(b+t*(a+t*(z+t*w)))));
}

Vector<3> R3_Lin_Interp(Vector<3> *a,Vector<3> *b,double t,double t0,double t1) {
	T0T1_ORDER((*a),(*b));

	return ((t*((*b)-(*a)))+(*a));
}

Vector<2> R2_Lin_Interp(Vector<2> *a,Vector<2> *b,double t,double t0,double t1) {
	T0T1_ORDER((*a),(*b));

	return ((t*((*b)-(*a)))+(*a));
}



Vector<3> R3_Cyl_Interp(Vector<3> *a,Vector<3> *b,double t,Vector<3> *axis,Vector<3> *o,double t0,double t1) {
	T0T1_ORDER((*a),(*b));
	// t: 0.0 - 1.0
	
	Vector<3> ta=(*a-*o);
	Vector<3> tb=(*b-*o);
	

	Matrix Rot(3,3);
	BuildRotationMatrix(&Rot,*axis,2,&ta,0,false);
	Rot.Traspose();

	Vector<3> cl_a=Transform(&Rot,ZERO_3,ta).ToCyl();
	Vector<3> cl_b=Transform(&Rot,ZERO_3,tb).ToCyl();
	Vector<3> cl_c;

	cl_c=R3_Lin_Interp(&cl_a,&cl_b,t,0.0,1.0);
	
	Rot.Traspose();
	Vector<3> c=Transform(&Rot,ZERO_3,cl_c.FromCylToR3())+(*o);

	return c;
}



					


void SO3_Lin_Interp(Matrix *a,Matrix *b,double t,Matrix *c,double t0,double t1) {
	T0T1_ORDERM;
	// t: 0.0 - 1.0
	
	Vector<3> ra,rb,rc;
	
	// SO(3) interpolation
	ra=RotationMatrix2AngleAxis(a);
	rb=RotationMatrix2AngleAxis(b);
	Quaternion qra(ra),qrb(rb),m;
	
	{	// which is the shortest?
		Vector<4> vqra=qra.qvect().Versore();
		Vector<4> vqrb=qrb.qvect().Versore();
		double angle=acos(vqra*vqrb);
		if (angle>(M_PI/2.0)) m=Quaternion::Slerp(qra,(-1.0)*qrb,t);
		else m=Quaternion::Slerp(qra,qrb,t);
	}
	
	rc=m.Angle_Axis();
	AngleAxis2RotationMatrix(c,rc);
}

void SE3_Lin_Interp(Matrix *a,Matrix *b,double t,Matrix *c,double t0,double t1) {
	T0T1_ORDERM;
	// t: 0.0 - 1.0

	Matrix Ra(3,3),Rb(3,3),Rc(3,3);
	Vector<3> Ta,Tb,Tc;
	Vector<3> ra,rb,rc;

	PtoRT(a,&Ra,&Ta);
	PtoRT(b,&Rb,&Tb);
	
	Ra.Traspose();
	InvertHandConvention(&Ra);
	Rb.Traspose();
	InvertHandConvention(&Rb);

	

	// SO(3) interpolation
	ra=RotationMatrix2AngleAxis(&Ra);
	rb=RotationMatrix2AngleAxis(&Rb);
	Quaternion qra(ra),qrb(rb),m;
	
	{	// which is the shortest?
		Vector<4> vqra=qra.qvect().Versore();
		Vector<4> vqrb=qrb.qvect().Versore();
		double angle=acos(vqra*vqrb);
		if (angle>(M_PI/2.0)) m=Quaternion::Slerp(qra,(-1.0)*qrb,t);
		else m=Quaternion::Slerp(qra,qrb,t);
	}
	
	rc=m.Angle_Axis();
	AngleAxis2RotationMatrix(&Rc,rc);

	InvertHandConvention(&Rc);
	Rc.Traspose();


	// R3 Interpolation
	Tc=R3_Lin_Interp(&Ta,&Tb,t);


	// result in [Rc,Tc]
	RTtoP(c,&Rc,&Tc);
}


void getRT(Matrix *M,Matrix *R,Vector<3> *T) {
	//Matrix EM(3,4);
	//ExttoE(M,&EM);
	//Invert3x4Matrix(&EM);
	//PtoRT(&EM,R,T);

	PtoRT(M,R,T);
}

void serRT(Matrix *M,Matrix *R,Vector<3> *T) {
	//Matrix EM(3,4);
	//RTtoP(&EM,R,T);
	//Invert3x4Matrix(&EM);
	//ExttoE(&EM,M);

	RTtoP(M,R,T);
}

inline double merge_factors(double p1,double p2,double aux_factor) {
	double comb;

	if (p1==0.0) comb=1.0;	// evita Peso[k] -> rafforza il fatto che 0.0 e' un peso che indica totale outlier
	if (p2==0.0) comb=0.0;
	if ((p2==0.0) && (p1==0.0)) comb=0.5;
	if ((p1!=0.0) && (p2!=0.0)) comb=aux_factor*(p2/(p1+p2));

	return comb;
}



void FilterSE3(Matrix **M,int num_elements,int filter_dim,double *weights,double oulier_detection_threshold) {
	Matrix Rcur1(3,3),Rcur2(3,3),R(3,3),R_f(3,3);
	Vector<3> Tcur1,Tcur2,T,T_f;

	// Oulier Detection on the translation
	if (oulier_detection_threshold!=0.0) {
		// WindowStat(num_elements,get_data_filter_ss3,set_data_filter_ss3,void *userdata,20);       ///TODO e' il modo migliore!!
		double SigmaTranslation=0;
		int	count=0;
		for(int i=0;i<num_elements-1;i++) {
			getRT(M[i],&Rcur1,&Tcur1);
			getRT(M[i+1],&Rcur2,&Tcur2);
			double val=(Tcur2-Tcur1).Norm2();
			SigmaTranslation+=val*val;
			count++;
		}
		SigmaTranslation=sqrt(SigmaTranslation/count);

		//Statistics X(num_elements,"CameraMovement%i.txt");
		// assume che il primo sia esatto..
		for(int i=1;i<num_elements;i++) {
			getRT(M[i-1],&Rcur1,&Tcur1);
			getRT(M[i],&Rcur2,&Tcur2);
			double val=(Tcur2-Tcur1).Norm2();
			//X.AddData(val);
			if (val>oulier_detection_threshold*SigmaTranslation) {
				cout<<"*";
				serRT(M[i],&Rcur1,&Tcur1);
			}
		}
		cout<<"\n";
	}

	
	// Filter Translation and Rotation
	if (filter_dim<=1) return;
	if (filter_dim%2==0) Warning("The filter dimension must be odd.");
	int filter_ext=(filter_dim-1)/2;


	double *Peso=new double[filter_ext+1];
	for(int i=0;i<=filter_ext;i++) Peso[i]=exp(-2.0*(i/(double)filter_ext)*(i/(double)filter_ext));
	
	
	double p1,p2,p_last_level;
	double comb;

	for(int t=filter_ext;t<num_elements-filter_ext;t++) {
		
		getRT(M[t],&R,&T);
		R_f.Set(&R);
		T_f=T;
		if (weights!=NULL) p_last_level=weights[t];
		else p_last_level=1.0;

		//       0          Peso[0]=1     
		//    -1    1       Peso[1]       
		// -2          2    Peso[2]
		// ....

		int k=1;
		for(int i=t-1;i>=t-filter_ext;i--,k++) {
			// [i] -> [2*t-i]
			getRT(M[i],&Rcur1,&Tcur1);
			getRT(M[2*t-i],&Rcur2,&Tcur2);
			
			if (weights!=NULL) {
				p1=weights[i];
				p2=weights[2*t-i];
			} else p1=p2=1.0;

			comb=merge_factors(p1,p2,1.0);
			SO3_Lin_Interp(&Rcur1,&Rcur2,comb,&R,0.0,1.0);
			T=R3_Lin_Interp(&Tcur1,&Tcur2,comb,0.0,1.0);
			
			p2=max(p1,p2);
			p1=p_last_level;
			comb=merge_factors(p1,p2,Peso[k]*2.0);

			SO3_Lin_Interp(&R_f,&R,comb,&Rcur1,0.0,1.0);R_f.Set(&Rcur1);
			T_f=R3_Lin_Interp(&T_f,&T,comb,0.0,1.0);

			p_last_level=max(p1,p2);
		}

		// Setta
		serRT(M[t],&R_f,&T_f);
	}
	
}


void FilterSO2(double *angles,int num_elements,int filter_dim,double *weights) {
	Matrix **In=NULL;
	Matrix **Out=NULL;
	SNEWA_P(In,Matrix*,num_elements);
	SNEWA_P(Out,Matrix*,num_elements);
	
	Vector<3> dir;
	dir[2]=0.0;
	for(int t=0;t<num_elements;t++) {
		SNEW(In[t],Matrix(3,3));
		SNEW(Out[t],Matrix(3,3));
		dir[0]=sin(angles[t]);
		dir[1]=cos(angles[t]);
		AngleAxis2RotationMatrix(In[t],dir);
	}
	FilterSO3(In,Out,num_elements,filter_dim,weights);

	for(int t=0;t<num_elements;t++) {
		dir=RotationMatrix2AngleAxis(Out[t]);
		angles[t]=-atan2(dir[1],dir[0])+M_PI/2.0;
		SDELETE(In[t]);
		SDELETE(Out[t]);
	}
	SDELETEA(In);
	SDELETEA(Out);
}

void FilterSO3(Matrix **In,Matrix **Out,int num_elements,int filter_dim,double *weights) {
	Matrix Rcur1(3,3),Rcur2(3,3),R_f(3,3),R(3,3);
	Vector<3> T;
	

	// Filter Rotation
	if (filter_dim<=1) return;
	if (filter_dim%2==0) Warning("The filter dimension must be odd.");
	int filter_ext=(filter_dim-1)/2;


	double *Peso=new double[filter_ext+1];
	for(int i=0;i<=filter_ext;i++) Peso[i]=exp(-2.0*(i/(double)filter_ext)*(i/(double)filter_ext));
	
	
	double p1,p2,p_last_level;
	double comb;

	for(int t=filter_ext;t<num_elements-filter_ext;t++) {
		R_f.Set(In[t]);
		if (weights!=NULL) p_last_level=weights[t];
		else p_last_level=1.0;

		//       0          Peso[0]=1     
		//    -1    1       Peso[1]       
		// -2          2    Peso[2]
		// ....

		int k=1;
		for(int i=t-1;i>=t-filter_ext;i--,k++) {
			// [i] -> [2*t-i]
			Rcur1.Set(In[i]);
			Rcur2.Set(In[2*t-i]);
			
			if (weights!=NULL) {
				p1=weights[i];
				p2=weights[2*t-i];
			} else p1=p2=1.0;

			comb=merge_factors(p1,p2,1.0);
			SO3_Lin_Interp(&Rcur1,&Rcur2,comb,&R,0.0,1.0);

			p2=max(p1,p2);
			p1=p_last_level;
			comb=merge_factors(p1,p2,Peso[k]*2.0);

			SO3_Lin_Interp(&R_f,&R,comb,&Rcur1,0.0,1.0);R_f.Set(&Rcur1);

			p_last_level=max(p1,p2);
		}

		// Setta
		Out[t]->Set(&R_f);
	}
	
}


void SE3_Cyl_Interp(Matrix *a,Matrix *b,double t,Matrix *c,Vector<3> *axis,Vector<3> *o,double t0,double t1) {	
	T0T1_ORDERM;
	// t: 0.0 - 1.0

	Matrix Ra(3,3),Rb(3,3),Rc(3,3);
	Vector<3> Ta,Tb,Tc;
	Vector<3> ra,rb,rc;

	PtoRT(a,&Ra,&Ta);
	PtoRT(b,&Rb,&Tb);
	
	Ra.Traspose();
	InvertHandConvention(&Ra);
	Rb.Traspose();
	InvertHandConvention(&Rb);

	

	// SO(3) interpolation
	ra=RotationMatrix2AngleAxis(&Ra);
	rb=RotationMatrix2AngleAxis(&Rb);
	Quaternion qra(ra),qrb(rb),m;
	
	{	// which is the shortest?
		Vector<4> vqra=qra.qvect().Versore();
		Vector<4> vqrb=qrb.qvect().Versore();
		double angle=acos(vqra*vqrb);
		if (angle>(M_PI/2.0)) m=Quaternion::Slerp(qra,(-1.0)*qrb,t);
		else m=Quaternion::Slerp(qra,qrb,t);
	}
	
	rc=m.Angle_Axis();
	AngleAxis2RotationMatrix(&Rc,rc);

	InvertHandConvention(&Rc);
	Rc.Traspose();


	// R3 Interpolation
	Tc=R3_Cyl_Interp(&Ta,&Tb,t,axis,o);

	// result in [Rc,Tc]
	RTtoP(c,&Rc,&Tc);
	
}


void SE3_2DStabilize_Interp(Matrix *a,Matrix *b,double t,Matrix *c,Vector<3> *axis,Vector<3> *o,double t0,double t1) {
	T0T1_ORDERM;
	// t: 0.0 - 1.0


	// Convert the matrices from OPENGL_FORMAT
	Matrix Ma(3,4),Mb(3,4),Mc(3,4);
	ExttoE(a,&Ma);
	ExttoE(b,&Mb);
	Invert3x4Matrix(&Ma);
	Invert3x4Matrix(&Mb);


	// Read input
	Matrix Ra(3,3),Rb(3,3),Rc(3,3);
	Matrix iRa(3,3),iRb(3,3),iRc(3,3);
	Vector<3> Ta,Tb,Tc;
	Vector<3> lin_Ta,lin_Tb,lin_Tc;
	PtoRT(&Ma,&Ra,&Ta);			
	PtoRT(&Mb,&Rb,&Tb);
	
	Ra.Traspose(&iRa);
	Rb.Traspose(&iRb);
	Multiply(&iRa,&Ta,&lin_Ta);lin_Ta=(-1.0)*lin_Ta;		// lin_Ta = -iRa*Ta
	Multiply(&iRb,&Tb,&lin_Tb);lin_Tb=(-1.0)*lin_Tb;		// lin_Tb = -iRb*Tb

	
	//lin_Tc = Cyl(lin_Ta,lin_Tb)
	lin_Tc=R3_Cyl_Interp(&lin_Ta,&lin_Tb,t,axis,o);
	



	// 3D Point to stabilize
	Vector<3> P1=(*o);

	// R2 Interpolation of the fixed points -> cP1_,cP2_
	Vector<3> aP1=Transform(&Ra,Ta,P1);
	Vector<3> bP1=Transform(&Rb,Tb,P1);
	Vector<2> aP1_=aP1.Projection();
	Vector<2> bP1_=bP1.Projection();
	Vector<2> cP1_=R2_Lin_Interp(&aP1_,&bP1_,t,0.0,1.0);


	Vector<3> X,Y;
	X[0]=cP1_[0];
	X[1]=cP1_[1];
	X[2]=1;
	Y=P1-lin_Tc;

	double s=Y.Norm2()/X.Norm2();
	X=s*X;


	Matrix Xm(3,1),Ym(3,1),R(3,3),iR(3,3);
	Xm.SetColumn(0,X.GetList());
	Ym.SetColumn(0,Y.GetList());
	SolveRBA(&Xm,&Ym,&R);
	R.Traspose(&iR);

	// Test1
	Matrix Test(3,1);
	Multiply(&R,&Ym,&Test);
	//Ym.Print();
	//Xm.Print();
	//Test.Print();


////// Mediazione con SLERP
	Vector<3> ra=RotationMatrix2AngleAxis(&Ra);
	Vector<3> rb=RotationMatrix2AngleAxis(&Rb);
	Quaternion qra(ra),qrb(rb),m;
	
	{	// which is the shortest?
		Vector<4> vqra=qra.qvect().Versore();
		Vector<4> vqrb=qrb.qvect().Versore();
		double angle=acos(vqra*vqrb);
		if (angle>(M_PI/2.0)) m=Quaternion::Slerp(qra,(-1.0)*qrb,t);
		else m=Quaternion::Slerp(qra,qrb,t);
	}
	
	Vector<3> rc=m.Angle_Axis();
	Matrix Q(3,3);
	AngleAxis2RotationMatrix(&Q,rc);

	Matrix ToLog(3,3);
	Multiply(&Q,&iR,&ToLog);
	Vector<3> axisTL=RotationMatrix2AngleAxis(&ToLog);
	double d=axisTL*X.Versore();

	Matrix G(3,3);
	AngleAxis2RotationMatrix(&G,(d*X.Versore()));
	Multiply(&G,&R,&Rc);
/////////////


	Multiply(&Rc,&lin_Tc,&Tc);Tc=(-1.0)*Tc;		// Tc = -Rc*lin_Tc


	// Test 2
	Vector<3> cP1=Transform(&Rc,Tc,P1);
	Vector<2> cP1_Test=cP1.Projection();
	//cout<<(cP1_Test-cP1_).Norm2()<<"\n";


	RTtoP(&Mc,&Rc,&Tc);

	Invert3x4Matrix(&Mc);
	ExttoE(&Mc,c);
}





