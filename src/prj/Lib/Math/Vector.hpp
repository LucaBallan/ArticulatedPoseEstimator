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





template <int dim>
Vector<dim>::Vector() {
	for(int i=0;i<dim;i++) value[i]=0;
}

template <int dim>
Vector<dim>::Vector(const Element_Type *c) {
	for(int i=0;i<dim;i++) value[i]=c[i];
}





//
// equality properties
//

template <int dim>
bool Vector<dim>::operator== (const Vector<dim> &v) const {
	for(int i=0;i<dim;i++) 
		if (v.value[i]!=value[i]) return false;
	
	return true;
}

template <int dim>
bool Vector<dim>::operator!= (const Vector<dim> &v) const {
	return (!((*this)==v));
}

template <int dim>
bool Vector<dim>::isEqual(const Vector<dim> &x,double tollerance) const {
	if (IS_ZERO( ((*this)-x).Norm2() , tollerance ) ) return true;
	return false;
}

template <int dim>
bool Vector<dim>::isZero() {
	for(int i=0;i<dim;i++) if (value[i]!=0) return false;
	return true;
}

template <int dim>
bool Vector<dim>::isFinite() {
	for(int i=0;i<dim;i++) if (!_finite(value[i])) return false;
	return true;
}












//
// global property
//

template <int dim>
Element_Type Vector<dim>::Norm2() const {
	return sqrt((*this)*(*this));
}
















//
// components change
//

template <int dim>
Vector<dim-1> Vector<dim>::Projection() const {
	Vector<dim-1> o;

	double divisor=value[dim-1];
	if (IS_ZERO(divisor,DBL_EPSILON)) ErrorExit("Cannot project a point which is a point at the infinity.");
	for(int i=0;i<(dim-1);i++) o[i]=value[i]/divisor;
	
	return o;
}

template <int dim>
const Vector<dim> Vector<dim>::Projection(const Vector<dim> &versor) const {
	return ((*this)*versor)*versor;
}

template <int dim>
const Vector<dim> Vector<dim>::PositiveProjection(const Vector<dim> &versor) const {
	Element_Type a=(*this)*versor;
	if (a<0) a=0;

	return a*versor;
}

template <int dim>
Vector<dim> Vector<dim>::ToCyl() const {
	Vector<dim> o;
	o=*this;
	o[0]=sqrt(value[0]*value[0]+value[1]*value[1]);
	o[1]=atan2(value[1],value[0]);
	
	return o;
}

template <int dim>
Vector<dim> Vector<dim>::FromCylToR3() const {
	Vector<dim> o;
	o=*this;
	o[0]=value[0]*cos(value[1]);
	o[1]=value[0]*sin(value[1]);
	
	return o;
}

template <int dim>
Vector<dim> Vector<dim>::abs() const {
	Vector<dim> O;
	for(int i=0;i<dim;i++) O[i]=fabs(value[i]);
	return O;
}

template <int dim>
Vector<dim> Vector<dim>::toInteger() const {
	Vector<dim> p;
	for(int i=0;i<dim;i++) p[i]=Approx(value[i]);
	return p;
}

template <int dim>
Vector<dim> Vector<dim>::Versore() const {
	Element_Type m=((*this)*(*this));
	if (m==0) return *this;
	return (*this)*(1/sqrt(m));
}


















//
// operation on the vector
//

template <int dim>
void Vector<dim>::SetZero() {
	for(int i=0;i<dim;i++) value[i]=0;
}

template <int dim>
void Vector<dim>::SetVal(Element_Type x) {
	for(int i=0;i<dim;i++) value[i]=x;
}

template <int dim>
void Vector<dim>::Normalize() {
	Element_Type m=0;
	for(int i=0;i<dim;i++) m+=(value[i]*value[i]);
	if (m!=0) {
		m=1/sqrt(m);
		for(int i=0;i<dim;i++) value[i]*=m;
	}
}

template <int dim>
void Vector<dim>::Random() {
	for(int i=0;i<dim;i++) value[i]=(((Element_Type)rand())/RAND_MAX);
}

template <int dim>
void Vector<dim>::Min(Vector<dim> *x) {
	for(int i=0;i<dim;i++) {
		if (value[i]>x->value[i]) value[i]=x->value[i];
	}
}

template <int dim>
void Vector<dim>::Max(Vector<dim> *x) {
	for(int i=0;i<dim;i++) {
		if (value[i]<x->value[i]) value[i]=x->value[i];
	}
}















//
// operatori binari
//

template <int dim>
const Vector<dim> &Vector<dim>::operator=(const Element_Type *c) {
	for(int i=0;i<dim;i++) value[i]=c[i];
	return *this;
}

template <int dim>
const Vector<dim> &Vector<dim>::operator*=(const Element_Type &a) {
	for(int i=0;i<dim;i++) value[i]=a*value[i];
	return *this; 
}

template <int dim>
const Vector<dim> Vector<dim>::operator*(const Element_Type &a) const{
	Vector<dim> q;
	for(int i=0;i<dim;i++) q.value[i]=a*value[i];
	return q; 
}

template <int dim>
const Vector<dim> &Vector<dim>::operator=(const Vector<dim> &v) {
	for(int i=0;i<dim;i++) value[i]=v.value[i];
	return *this;
}

template <int dim>
const Vector<dim> &Vector<dim>::operator+=(const Vector<dim> &v) {
	for(int i=0;i<dim;i++) value[i]+=v.value[i];
	return *this; 
}

template <int dim>
const Vector<dim> &Vector<dim>::operator-=(const Vector<dim> &v) {
	for(int i=0;i<dim;i++) value[i]-=v.value[i];
	return *this; 
}

template <int dim>
const Vector<dim> Vector<dim>::operator+(const Vector<dim> &v) const {
	Vector<dim> q;
	for(int i=0;i<dim;i++) q.value[i]=v.value[i]+value[i];
	return q; 
}

template <int dim>
const Vector<dim> Vector<dim>::operator-(const Vector<dim> &v) const {
	Vector<dim> q;
	for(int i=0;i<dim;i++) q.value[i]=value[i]-v.value[i];
	return q; 
}

template <int dim>
Element_Type Vector<dim>::operator*(const Vector<dim> &v) const {
	Element_Type val=0;
	for(int i=0;i<dim;i++) val+=v.value[i]*value[i];
	return val; 
}

template <>
const Vector<3> Vector<3>::operator^(const Vector<3> &v) const {
	Vector<3> q;
	q.value[0]=value[1]*v.value[2]-value[2]*v.value[1];
	q.value[1]=value[2]*v.value[0]-value[0]*v.value[2];
	q.value[2]=value[0]*v.value[1]-value[1]*v.value[0];	
	return q; 
}















//
// operatori binari
//

template <int dim>
Element_Type *Vector<dim>::GetList() {
	return value;
}

template <int dim>
void Vector<dim>::SetList(const float *c) {
	for(int i=0;i<dim;i++) value[i]=c[i];
}

template <int dim>
void Vector<dim>::SetList(const double *c) {
	for(int i=0;i<dim;i++) value[i]=c[i];
}

template <int dim>
void Vector<dim>::GetList(float *c) {
	for(int i=0;i<dim;i++) c[i]=(float)value[i];
}

template <int dim>
void Vector<dim>::GetList(double *c) {
	for(int i=0;i<dim;i++) c[i]=value[i];
}

template <int dim>
void Vector<dim>::SerializeFloat(float *buffer) {
	for(int i=0;i<dim;i++) buffer[i]=(float)value[i];
}













//
// esterne
//

template <int dim>
const Vector<dim> operator*(const Element_Type &a, const Vector<dim> &p) { 
	return p*a; 
}

template <class stream_type,int dim>
stream_type &operator<<(stream_type &os, const Vector<dim> &p) {
	for(int i=0;i<dim;i++) os<<p[i]<<" ";
	return os;
}

template <class stream_type,int dim>
stream_type &operator>>(stream_type &os, Vector<dim> &p) {
	for(int i=0;i<dim;i++) os>>p[i];
	return os;
}

template <int dim>
void Multiply(Matrix *A,Vector<dim> b,Element_Type *c) {
	if (A->c!=dim) ErrorExit("Matrici non compatibili per la moltiplicazione");

	Element_Type **tmp=A->v;
	for(int i=0;i<A->r;i++) {
		(*c)=0;
		Element_Type *tmp2=*tmp;
		for(int k=0;k<A->c;k++) {
			(*c)+=(*tmp2)*b[k];
			tmp2++;
		}
		c++;tmp++;
	}
}

void Multiply(Matrix *A,Vector<3> *b,Vector<3> *c) {
	if ((A->r!=3) || (A->c!=3)) ErrorExit("Matrici non compatibili per la moltiplicazione");

	Element_Type **tmp=A->v,*row;
	Element_Type *in=b->GetList();

	row=*tmp;
	(*c)[0]=row[0]*in[0] + row[1]*in[1] + row[2]*in[2];
	tmp++;row=*tmp;
	(*c)[1]=row[0]*in[0] + row[1]*in[1] + row[2]*in[2];
	tmp++;row=*tmp;
	(*c)[2]=row[0]*in[0] + row[1]*in[1] + row[2]*in[2];
}

void Invert3x4Matrix(Matrix *M) {
	::swap<Element_Type>((*M)[0][1],(*M)[1][0]);
	::swap<Element_Type>((*M)[0][2],(*M)[2][0]);
	::swap<Element_Type>((*M)[1][2],(*M)[2][1]);

	Element_Type tmp1,tmp2,tmp3;
	tmp1=-((*M)[0][0]*(*M)[0][3] + (*M)[0][1]*(*M)[1][3] + (*M)[0][2]*(*M)[2][3]);
	tmp2=-((*M)[1][0]*(*M)[0][3] + (*M)[1][1]*(*M)[1][3] + (*M)[1][2]*(*M)[2][3]);
	tmp3=-((*M)[2][0]*(*M)[0][3] + (*M)[2][1]*(*M)[1][3] + (*M)[2][2]*(*M)[2][3]);
	
	(*M)[0][3]=tmp1;
	(*M)[1][3]=tmp2;
	(*M)[2][3]=tmp3;
}

void Invert2x3Matrix(Matrix *M) {
	Matrix Tmp(3,3);
	Tmp.SetMinor(2,3,M);
	Tmp[2][0]=0;Tmp[2][1]=0;Tmp[2][2]=1;
	Tmp.Inversion();
	M->SetMinor(2,3,&Tmp);
}

void Invert4x4RotoTraslationMatrix(Matrix *M) {
	::swap<Element_Type>((*M)[0][1],(*M)[1][0]);
	::swap<Element_Type>((*M)[0][2],(*M)[2][0]);
	::swap<Element_Type>((*M)[1][2],(*M)[2][1]);

	Element_Type tmp1,tmp2,tmp3;
	tmp1=-((*M)[0][3]*(*M)[0][0]+(*M)[1][3]*(*M)[0][1]+(*M)[2][3]*(*M)[0][2]);
	tmp2=-((*M)[0][3]*(*M)[1][0]+(*M)[1][3]*(*M)[1][1]+(*M)[2][3]*(*M)[1][2]);
	tmp3=-((*M)[0][3]*(*M)[2][0]+(*M)[1][3]*(*M)[2][1]+(*M)[2][3]*(*M)[2][2]);

	(*M)[0][3]=tmp1;
	(*M)[1][3]=tmp2;
	(*M)[2][3]=tmp3;
}

void InvertCorrectExtrinsic_For_OpenGL3D_Visualization(Matrix *M) {
	Vector<3> tmp;

	// Normalize
	M->GetColumn(0,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(0,tmp.GetList());
	M->GetColumn(1,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(1,tmp.GetList());
	M->GetColumn(2,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(2,tmp.GetList());

	// Inverse
	Invert3x4Matrix(M);

	// Invert X
	M->GetColumn(0,tmp.GetList());
	tmp=-1.0*tmp;
	M->SetColumn(0,tmp.GetList());
}
void CorrectExtrinsic_For_OpenGL3D_Visualization(Matrix *M) {
	Vector<3> tmp;

	// TODO: Chelle' sta cosa... -> Correggere Render Window va!!!

	// Normalize
	M->GetColumn(0,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(0,tmp.GetList());
	M->GetColumn(1,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(1,tmp.GetList());
	M->GetColumn(2,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(2,tmp.GetList());

	// Invert X
	M->GetColumn(0,tmp.GetList());
	tmp=-1.0*tmp;
	M->SetColumn(0,tmp.GetList());

	// Inverse
	Invert3x4Matrix(M);
}

void CorrectExtrinsic(Matrix *M) {
	Vector<3> tmp;

	// Normalize
	M->GetColumn(0,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(0,tmp.GetList());
	M->GetColumn(1,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(1,tmp.GetList());
	M->GetColumn(2,tmp.GetList());
	tmp=tmp.Versore();
	M->SetColumn(2,tmp.GetList());
}

// 3 point (P1,P2,P3) -> axes [R,P1]
bool PstoRT(Vector<3> P1,Vector<3> P2,Vector<3> P3,Matrix *axes) {
	P2=(P2-P1).Versore();
	P3=(P3-P1).Versore();
	P3=(P3-((P3*P2)*P2)).Versore();		// orto-normalizzo
	if (P3.Norm2()<1.0) return false;
	Vector<3> N=(P2^P3).Versore();
	
	axes->SetColumn(0,P2.GetList());
	axes->SetColumn(1,P3.GetList());
	axes->SetColumn(2,N.GetList());

	return true;
}


void RTtoP(Matrix *P,Matrix *R,Vector<3> *T) {
	P->SetMinor(3,3,R);
	(*P)[0][3]=(*T)[0];
	(*P)[1][3]=(*T)[1];
	(*P)[2][3]=(*T)[2];
	if (P->r>3) {
		(*P)[3][0]=0;
		(*P)[3][1]=0;
		(*P)[3][2]=0;
		(*P)[3][3]=1;
	}
}

void RTtoGL(Matrix *R,Vector<3> *T,double *m) {
	int i,j;

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m[i*4+j]=(*R)[j][i];
	
	for(j=0;j<3;j++) 
		m[3*4+j]=(*T)[j];

	m[0*4+3]=0;
	m[1*4+3]=0;
	m[2*4+3]=0;
	m[3*4+3]=1.0;
}

void PtoRT(Matrix *P,Matrix *R,Vector<3> *T) {
	P->GetMinor(3,3,R);
	(*T)[0]=(*P)[0][3];
	(*T)[1]=(*P)[1][3];
	(*T)[2]=(*P)[2][3];
}

void PtoGL(Matrix *P,double *m) {
	int i,j;

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m[i*4+j]=(*P)[j][i];
	
	for(j=0;j<3;j++) 
		m[3*4+j]=(*P)[j][3];

	if (P->r==3) {
		for(i=0;i<3;i++) m[i*4+3]=0;
		m[3*4+3]=1.0;
	} else {
		for(i=0;i<4;i++) m[i*4+3]=(*P)[3][i];
	}
}

void GLtoP(double *m,Matrix *P) {
	int i,j;

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			(*P)[j][i]=m[i*4+j];

	if (P->r>3) {
		j=3;
		for(i=0;i<4;i++) (*P)[j][i]=m[i*4+j];
	}
	if (P->c>3) {
		i=3;
		for(j=0;j<4;j++) (*P)[j][i]=m[i*4+j];
	}
}

void EtoP(Matrix *E,Matrix *P) {
	if (E->c!=4) ErrorExit("Not an 3x4 or a 4x4 matrix");
	if ((E->r!=3) && (E->r!=4)) ErrorExit("Not an 3x4 or a 4x4 matrix");

	if (E->r==3) {
		P->SetMinor(3,4,E);
		(*P)[3][0]=0;
		(*P)[3][1]=0;
		(*P)[3][2]=0;
		(*P)[3][3]=1;
	} else {
		P->Set(E);
	}
}

void PtoE(Matrix *P,Matrix *E) {
	if (P->c!=4) ErrorExit("Not an 3x4 or a 4x4 matrix");
	if ((P->r!=3) && (P->r!=4)) ErrorExit("Not an 3x4 or a 4x4 matrix");
	if ((E->r!=3) || (E->c!=4)) ErrorExit("Not an 3x4 matrix");

	P->GetMinor(3,4,E);
}

void GtoG(Matrix *src,Matrix *dst) {
	Matrix tmp(4,4);
	if ((src->c==3) && (src->r==3)) {
		RtoP(src,&tmp);
		src=&tmp;
	}
	if (src->c!=4) ErrorExit("Not a 3x3, 3x4 or a 4x4 matrix");
	if ((src->r!=3) && (src->r!=4)) ErrorExit("Not a 3x3, 3x4 or a 4x4 matrix");
	if (dst->c!=4) ErrorExit("Not a 3x3, 3x4 or a 4x4 matrix");
	if ((dst->r!=3) && (dst->r!=4)) ErrorExit("Not a 3x3, 3x4 or a 4x4 matrix");

	if (dst->r==4) {
		// dst is a 4x4
		EtoP(src,dst);
	} else {
		// dst is a 3x4	
		PtoE(src,dst);
	}
}

void InvertHandConvention(Matrix *x) {
	Vector<3> tmp;
	x->GetColumn(2,tmp.GetList());
	tmp=(-1.0)*tmp;
	x->SetColumn(2,tmp.GetList());
}

void ExttoE(Matrix *Ext,Matrix *E) {
	Vector<3> Direction;

	E->Set(Ext);
	Invert3x4Matrix(E);	
	E->GetColumn(0,Direction.GetList());
	Direction=-1.0*Direction;
	E->SetColumn(0,Direction.GetList());
	Invert3x4Matrix(E);
}

void GetCPfromExt(Matrix *Ext,Vector<3> *cp) {
	Matrix iExt(3,4);
	iExt.Set(Ext);
	Invert3x4Matrix(&iExt);
	iExt.GetColumn(3,cp->GetList());
}
 

void RtoP(Matrix *R,Matrix *P) {
	if ((R->r!=3) || (R->c!=3)) ErrorExit("Not an 3x3 matrix");

	P->SetMinor(3,3,R);
	(*P)[0][3]=0;
	(*P)[1][3]=0;
	(*P)[2][3]=0;
	(*P)[3][0]=0;
	(*P)[3][1]=0;
	(*P)[3][2]=0;
	(*P)[3][3]=1;
}

Vector<3> get_map_scale(Matrix *P) {
	Vector<3> S;

	if (P->r==3) {
		Vector<3> tmp;

		P->GetColumn(0,tmp.GetList());
		S[0]=tmp.Norm2();
		P->GetColumn(1,tmp.GetList());
		S[1]=tmp.Norm2();
		P->GetColumn(2,tmp.GetList());
		S[2]=tmp.Norm2();
	} else {
		Vector<4> tmp;

		P->GetColumn(0,tmp.GetList());
		S[0]=tmp.Norm2();
		P->GetColumn(1,tmp.GetList());
		S[1]=tmp.Norm2();
		P->GetColumn(2,tmp.GetList());
		S[2]=tmp.Norm2();
	}
	
	return S;
}

void Orthogonalize(Matrix *M) {
	// Ortogonal
	Vector<3> p1;
	Vector<3> p2;
	Vector<3> p3;
	M->GetColumn(0,p1.GetList());
	M->GetColumn(1,p2.GetList());
	M->GetColumn(2,p3.GetList());

	p1=p1.Versore();
	p2=p2-((p2*p1)*p1);
	p2=p2.Versore();
	p3=p3-((p3*p1)*p1)-((p3*p2)*p2);
	p3=p3.Versore();

	M->SetColumn(0,p1.GetList());
	M->SetColumn(1,p2.GetList());
	M->SetColumn(2,p3.GetList());
}

void RotationMatrix(Matrix *out,double angle,double x,double y,double z) {
	double n=sqrt(x*x+y*y+z*z);
	x/=n;y/=n;z/=n;
	if ((n==0.0) || (angle==0.0)) {
		out->SetIdentity();
		return;
	}
	double c=cos(angle);
	double s=sin(angle);
	double c1=1-c;
	double xy=x*y*c1,xz=x*z*c1,zy=z*y*c1;
	double zs=z*s,xs=x*s,ys=y*s;

	(*out)[0][0]=(x*x*c1)+c;
	(*out)[0][1]=xy-zs;
	(*out)[0][2]=xz+ys;
	(*out)[1][0]=xy+zs;
	(*out)[1][1]=(y*y*c1)+c;
	(*out)[1][2]=zy-xs;
	(*out)[2][0]=xz-ys;
	(*out)[2][1]=zy+xs;
	(*out)[2][2]=(z*z*c1)+c;

	if (out->c==4) {
		(*out)[0][3]=0;
		(*out)[1][3]=0;
		(*out)[2][3]=0;
		
		if (out->r==4) {
			(*out)[3][0]=0;
			(*out)[3][1]=0;
			(*out)[3][2]=0;
			(*out)[3][3]=1;
		}
	}
}

void AngleAxis2RotationMatrix(Matrix *out,Vector<3> AngleAxis) {
	double angle=AngleAxis.Norm2();
	AngleAxis=AngleAxis.Versore();
	RotationMatrix(out,angle,AngleAxis[0],AngleAxis[1],AngleAxis[2]);
}

Vector<3> RotationMatrix2AngleAxis(Matrix *In) {
	Vector<3> AngleAxis;
	
	double tr=((*In)[0][0]+(*In)[1][1]+(*In)[2][2]-1)/2.0;
	
	double theta=realAcos(tr);
	double sin_theta=sin(theta);
	
	if (sin_theta>=1e-4) {
		AngleAxis[0]=(*In)[2][1]-(*In)[1][2];
		AngleAxis[1]=(*In)[0][2]-(*In)[2][0];
		AngleAxis[2]=(*In)[1][0]-(*In)[0][1];

		AngleAxis=(theta/(2*sin_theta))*AngleAxis;
	} else {
		if (tr>0) {
			AngleAxis.SetZero();
		} else {
			Vector<3> tmp;
			tmp[0]=1;
			if ((*In)[0][1]>=0) tmp[1]=1;
			else tmp[1]=-1;
			if ((*In)[0][2]>=0) tmp[2]=1;
			else tmp[2]=-1;
			
			Vector<3> diag;
			diag[0]=(sqrt(((*In)[0][0]+1)/2))*tmp[0];
			diag[1]=(sqrt(((*In)[1][1]+1)/2))*tmp[1];
			diag[2]=(sqrt(((*In)[2][2]+1)/2))*tmp[2];

			AngleAxis=theta*diag;
		}
	}
	
	return AngleAxis;
}    

void RotationMatrixToEuler(Matrix *M,double &a,double &b,double &c) {
	// http://prt.fernuni-hagen.de/lehre/KURSE/PRT001/course_main_sh3d/node10.html
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
	// http://gypsy.rose.utoronto.ca/people/david_dir/GEMS/GEMS.html

	
	double m[9];
	M->Get(m);
	double sb=sqrt(1-m[8]*m[8]);
	
	if (sb>16*FLT_EPSILON) {
		a=atan2(m[5],m[2]);
		b=atan2(sb,m[8]);
		c=atan2(m[7],-m[6]);
		// ambiguità -pi +pi b- ect..
	} else {
		if (m[8]>=0) b=0;
		else b=M_PI;
		std::cout<<"Gimbal\n";
		double ac=atan2((-m[1]+m[3])/2,(m[0]+m[4])/2);
		a=ac;
		c=0;		// lo fisso io cosi': la somma deve essere costante
	}
}

void EulerToRotationMatrix(Matrix *C,double a,double b,double c) {
	Matrix tmp1(4,4),tmp2(4,4),tmp3(4,4);

	RotationMatrix(&tmp1,c,0,0,1);
	RotationMatrix(&tmp2,b,0,1,0);
	Multiply(&tmp2,&tmp1,&tmp3);
	RotationMatrix(&tmp1,a,0,0,1);
	Multiply(&tmp1,&tmp3,&tmp2);
	tmp2.GetMinor(3,3,C);
}

void CrossMatrix(Vector<2> p,Matrix *out) {
	(*out)[0][0]=0;
	(*out)[0][1]=-1;
	(*out)[0][2]=p[1];

	(*out)[1][0]=1;
	(*out)[1][1]=0;
	(*out)[1][2]=-p[0];

	(*out)[2][0]=-p[1];
	(*out)[2][1]=p[0];
	(*out)[2][2]=0;
}

void CrossMatrix(Vector<3> p,Matrix *out) {
	(*out)[0][0]=0;
	(*out)[0][1]=-p[2];
	(*out)[0][2]=p[1];

	(*out)[1][0]=p[2];
	(*out)[1][1]=0;
	(*out)[1][2]=-p[0];

	(*out)[2][0]=-p[1];
	(*out)[2][1]=p[0];
	(*out)[2][2]=0;
}

template <int dim>
Element_Type Intersection_LinePlane(Vector<dim> o,Vector<dim> d,Vector<dim> n,Vector<dim> plane_point,Vector<dim> *p) {
	Element_Type c=n*plane_point;
	// piano:   n*p=c

	Element_Type parallelismo=(n*d);
	
	if (!IS_ZERO(parallelismo,INTERSECTION_NUMERICAL_TOLLERANCE)) {
		Element_Type t=(c-n*o)/(n*d);
		(*p)=t*d+o;
		return t;
	}

	// piano parallelo alla linea
	if (IS_ZERO((n*o-c),INTERSECTION_NUMERICAL_TOLLERANCE)) {
		// line belongs to the plane
		(*p)=o;
		return 0.0;
	}

	// no intersections
	(*p)=o;
	return Element_Type_Max;
}

template Element_Type Intersection_LinePlane(Vector<3> o,Vector<3> d,Vector<3> n,Vector<3> plane_point,Vector<3> *p);
template Element_Type Intersection_LinePlane(Vector<2> o,Vector<2> d,Vector<2> n,Vector<2> plane_point,Vector<2> *p);



void BuildRotationMatrix(Matrix *Rot,Vector<3> asse_riferimento,int indice_asse_riferimento,Vector<3> *asse_ausiliario,int indice_asse_ausiliario,bool make_rigid) {

	// Setta asse 1
	asse_riferimento=asse_riferimento.Versore();
	Rot->SetColumn(indice_asse_riferimento,asse_riferimento.GetList());

	Vector<3> asse_ausiliario_;
	if (asse_ausiliario==NULL) {
		do {
			asse_ausiliario_.Random();
			asse_ausiliario_=asse_ausiliario_-(asse_ausiliario_*asse_riferimento)*asse_riferimento;
		} while (asse_ausiliario_.Norm2()<0.01);

		asse_ausiliario_=asse_ausiliario_.Versore();
		indice_asse_ausiliario=(indice_asse_riferimento+1)%3;
	} else {
		asse_ausiliario_=*asse_ausiliario;
		asse_ausiliario_=(asse_ausiliario_-(asse_ausiliario_*asse_riferimento)*asse_riferimento).Versore();
	}

		// Setta asse 2
		Rot->SetColumn(indice_asse_ausiliario,asse_ausiliario_.GetList());

		// computa l'indice mancante
		bool index[3];
		int indice_mancante;
		index[0]=index[1]=index[2]=false;
		index[indice_asse_riferimento]=true;
		index[indice_asse_ausiliario]=true;
		for(indice_mancante=0;indice_mancante<3;indice_mancante++) if (index[indice_mancante]==false) break;
		
		// computa asse mancante
		Vector<3> asse_mancante=(asse_riferimento^asse_ausiliario_).Versore();
		Rot->SetColumn(indice_mancante,asse_mancante.GetList());

		if (make_rigid) {
			if (Rot->Determinante()<0) {
				asse_mancante=-1.0*asse_mancante;
				Rot->SetColumn(indice_mancante,asse_mancante.GetList());
			}
		}
}





//***********************************************************************************************************************
//
//							Complex Numbers
//
//***********************************************************************************************************************

Complex::Complex() : Vector<2>() {
}

Complex::Complex(double Re,double Im) {
	value[0]=Re;
	value[1]=Im;
}

const Complex Complex::operator*(const double &a) const {
	return Complex(a*value[0],a*value[1]);
}

const Complex &Complex::operator=(const Complex &v) {
	value[0]=v.value[0];
	value[1]=v.value[1];
	return *this;
}

const Complex Complex::operator+(const Complex &v) const {
	return Complex((value[0]+v.value[0]),
		           (value[1]+v.value[1]));
}

const Complex Complex::operator*(const Complex &v) const {
	return Complex((value[0]*v.value[0]-value[1]*v.value[1]),
		           (value[1]*v.value[0]+value[0]*v.value[1]));
}

Complex Complex::inv() const {
	double norm2=(value[0]*value[0])+(value[1]*value[1]);
	if (norm2==0) ErrorExit("Cannot invert a zero complex number.");
	return Complex(value[0]/norm2,-value[1]/norm2);
}

double Complex::re() {
	return value[0];
}

double Complex::im() {
	return value[1];
}

Complex Complex::sqrt(double x) {
	double Re=0.0,Im=0.0;
	if (x>=0) Re=::sqrt(x);
	else Im=::sqrt(-x);
	return Complex(Re,Im);
}

void Orthonormalize(Matrix *x,bool make_rigid) {
	if (x->IsOrthonormal(make_rigid)==0.0) return;
	
	Vector<3> row0,row1,row2;
	x->GetRow(0,row0.GetList());
	x->GetRow(1,row1.GetList());
	x->GetRow(2,row2.GetList());
	
	row0=(1/row0.Norm2())*row0;
	row1=row1-((row1*row0)*row0);
	row1=(1/row1.Norm2())*row1;
	row2=row2-((row2*row0)*row0)-((row2*row1)*row1);
	row2=(1/row2.Norm2())*row2;

	x->SetRow(0,row0.GetList());
	x->SetRow(1,row1.GetList());
	x->SetRow(2,row2.GetList());

	if (make_rigid) {
		if (x->Determinante()<0) {
			x->GetRow(2,row2.GetList());
			row2=-1.0*row2;
			x->SetRow(2,row2.GetList());
		}
	}
}









//***********************************************************************************************************************
//
//							Quaternion
//
//***********************************************************************************************************************

Quaternion::Quaternion() {
}

Quaternion::Quaternion(Vector<4> quaternion) {
	double *l=quaternion.GetList();
	SetList(l);
}

Quaternion::Quaternion(Vector<3> AngleAxis) {
	Set_Angle_Axis(AngleAxis);
}






Vector<4> Quaternion::qvect() const {
	return ((Vector<4>)(*this));
}
Vector<3> Quaternion::vect() const {
	Vector<3> v;
	v[0]=value[1];
	v[1]=value[2];
	v[2]=value[3];
	return v;
}
double Quaternion::scalar() const {
	return value[0];
}
void Quaternion::set_vect(Vector<3> x) {
	value[1]=x[0];
	value[2]=x[1];
	value[3]=x[2];
}
void Quaternion::set_scalar(double val) {
	value[0]=val;
}









void Quaternion::Set_Angle_Axis(Vector<3> AngleAxis) {
	double angle=AngleAxis.Norm2();
	AngleAxis=AngleAxis.Versore();
	double s=sin(angle/2.0);
	value[1]=s*AngleAxis[0];
	value[2]=s*AngleAxis[1];
	value[3]=s*AngleAxis[2];
	value[0]=cos(angle/2.0);
}
Vector<3> Quaternion::Angle_Axis() {
	Vector<3> AngleAxis;
	double angle=2.0*acos(value[0]);
	double s=sqrt(1-(value[0]*value[0]));
	if (s<0.001) {
		AngleAxis[0]=value[1];
		AngleAxis[1]=value[2];
		AngleAxis[2]=value[3];
	} else {
		AngleAxis[0]=value[1]/s;
		AngleAxis[1]=value[2]/s;
		AngleAxis[2]=value[3]/s;
	}

	AngleAxis=AngleAxis.Versore();	// Sicurezza
	AngleAxis=angle*AngleAxis;
	return AngleAxis;
}










const Quaternion &Quaternion::operator=(const Quaternion &v) {
	SetList(v.value);
	return *this;
}
const Quaternion Quaternion::operator+(const Quaternion &v) const {
	return Quaternion(this->qvect()+v.qvect());
}
const Quaternion Quaternion::operator-(const Quaternion &v) const {
	return Quaternion(this->qvect()-v.qvect());
}
const Quaternion Quaternion::operator*(const Quaternion &v) const {
	Quaternion result;

	result[0]=(scalar()*v.scalar())-(vect()*v.vect());
	Vector<3> v_=(scalar()*v.vect())+(v.scalar()*vect())+(vect()^v.vect());
	result.set_vect(v_);

	return result;
}
const Quaternion Quaternion::operator*(const double &a) const {
	Quaternion x;
	x.value[0]=a*value[0];
	x.value[1]=a*value[1];
	x.value[2]=a*value[2];
	x.value[3]=a*value[3];
	return x;
}







	
Element_Type *Quaternion::GetList() {
	return value;
}
Quaternion Quaternion::conjugate() const {
	Quaternion q;
	q.value[0]=value[0];
	q.set_vect(-1.0*vect());

	return q;
}
Quaternion Quaternion::inv() const {
	double norm=Norm2();
	if (fabs(norm)<DBL_EPSILON) ErrorExit("Quaternion: it does not exist the inverse.")
	return ((1/norm)*conjugate());
}
Quaternion Quaternion::exp() const {
	Quaternion q;

	double theta = vect().Norm2();
	double sin_theta = sin(theta);
	q.value[0]=cos(theta);

	if (fabs(sin_theta)>DBL_EPSILON) q.set_vect((sin_theta/theta)*vect());
	else q.set_vect(vect());
	
	return q;
}
Quaternion Quaternion::power(const double t) const {
	Quaternion q=Log();
	q=t*q;
	q=q.exp();
	
	return q;
}
Quaternion Quaternion::Log() const {
   Quaternion q;
   double theta = acos(scalar());
   double sin_theta = sin(theta);

   if (fabs(sin_theta)>DBL_EPSILON) q.set_vect((1.0/(sin_theta*theta))*vect());
   else q.set_vect(vect());
   q.value[0] = 0;

   return q;
}


















Quaternion Quaternion::Interpolation(const Quaternion qA,const Quaternion qB,const double t,const Quaternion Target) {
	Quaternion a[8];
	
	// Testa tutte le possibili soluzioni 
	a[0]=Slerp(qA,qB,t);
	a[1]=Slerp(qA,(-1.0)*qB,t);
	a[2]=Slerp((-1.0)*qA,qB,t);
	a[3]=Slerp((-1.0)*qA,(-1.0)*qB,t);
	a[4]=(-1.0)*a[0];
	a[5]=(-1.0)*a[1];
	a[6]=(-1.0)*a[2];
	a[7]=(-1.0)*a[3];

	if (a[0].isEqual((-1.0)*a[1],0.0001)) std::cout<<"Warning: assunzione slerp errata. Potrebbe essere fermo.\n";
	if (a[0].isEqual(a[1],0.0001))        std::cout<<"Warning: assunzione slerp errata. Potrebbe essere fermo.\n";

	double min=DBL_MAX;
	Quaternion Winner;
	for(int i=0;i<8;i++) {
		double d=(a[i]-Target).Norm2();
		if (d<min) {
			min=d;
			Winner=a[i];
		}
	}
	
	return Winner;
}
Quaternion Quaternion::Slerp(const Quaternion qA,const Quaternion qB,const double t) {
	if ((t < 0) || (t > 1)) ErrorExit("Slerp: t must be between 0 and 1.")

	Quaternion q0=(1.0/qA.Norm2())*qA;
	Quaternion q1=(1.0/qB.Norm2())*qB;

	Vector<4> a,b;
	a=q0.qvect();
	b=q1.qvect();
	

	double cos_theta = a*b;  
    double theta = acos(cos_theta);      
    double invSin = 1.0 / sin(theta);

	Quaternion val;
    double scaleQ, scaleR;
    if ((1.0 + cos_theta)>DBL_EPSILON) {
            // If angle is not small use SLERP.
            if ( (1.0 - cos_theta) > DBL_EPSILON )
            {
                    scaleQ = sin( (1.0 -t)*theta ) * invSin ;
                    scaleR = sin(t*theta) * invSin;
            }
            else    // For small angles use LERP
            {       
                    scaleQ = 1.0 - t;
                    scaleR = t;
            }
            val = q0 * scaleQ  + q1 * scaleR ;
    }
    else // This is a long way
    {
            // Clear the concept later...
			val.value[0]=q1.value[3];
			val.value[1]=-q1.value[2];
			val.value[2]=q1.value[1];
			val.value[3]=-q1.value[0];    
            
			#define PID2 1.570796326794896619231322
			scaleQ = sin( (1.0 - t)*PID2 );
            scaleR = sin( t * PID2 );
            val=scaleR*val+scaleQ*q0;
    }
	val=(1.0/val.Norm2())*val;
	return val;


	/*	double omega=a*b;
	Quaternion q=((sin((1-t)*omega)/sin(omega))*q0)+(((sin(t*omega))/sin(omega))*q1);
	return q;
	
	/*	if (a*b>=0) 
		return (q0*((q0.inv()*q1).power(t)));
	
	return  (q0*((q0.inv()*(-1.0*q1)).power(t)));*/
}












   

template <int dim>
Vector<dim> Max(Vector<dim> a,Vector<dim> b) {
	Vector<dim> c;
	for(int i=0;i<dim;i++) {
		if (a[i]>=b[i]) c[i]=a[i];
		else c[i]=b[i];
	}
	return c;
}

template <int dim>
Vector<dim> Min(Vector<dim> a,Vector<dim> b) {
	Vector<dim> c;
	for(int i=0;i<dim;i++) {
		if (a[i]<=b[i]) c[i]=a[i];
		else c[i]=b[i];
	}
	return c;
}


Vector<2> GetOrthogonalVersor(Vector<2> versor) {
	Vector<2> rnd;
	Element_Type n;
	if (versor.Norm2()==0) return versor;
	do {
		rnd.Random();
		rnd.Normalize();
		rnd=rnd-((rnd*versor)*versor);
		n=rnd.Norm2();
	} while (n==0.0);
	rnd=(1/n)*rnd;
	return rnd;
}


Vector<2> VECTOR(Element_Type x,Element_Type y) {
	Vector<2> o;
	o[0]=x;
	o[1]=y;
	return o;
}
Vector<3> VECTOR(Element_Type x,Element_Type y,Element_Type z) {
	Vector<3> o;
	o[0]=x;
	o[1]=y;
	o[2]=z;
	return o;
}
