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





#include "../stdInclude.h"
#include "../common.h"
#include "Matrix.h"

#include "newmat.h"
#include "newmatap.h"


using namespace std;
#pragma warning(3:4244)

Matrix::Matrix(int r,int c) {
	this->r=r;
	this->c=c;

	v=new double *[r];
	for(int i=0;i<r;i++) v[i]=new double[c];
}


Matrix::~Matrix() {
	for(int i=0;i<r;i++) delete [](v[i]);
	delete []v;
}

double *Matrix::GetColumn(int j,double *c) {
	for(int i=0;i<r;i++) c[i]=v[i][j];
	return c;
}

double *Matrix::GetRow(int i,double *r) {
	for(int j=0;j<c;j++) r[j]=v[i][j];
	return r;
}

void Matrix::SetColumn(int j,const double *c) {
	for(int i=0;i<r;i++) v[i][j]=c[i];
}

void Matrix::SetRow(int i,const double *r) {
	for(int j=0;j<c;j++) v[i][j]=r[j];
}

void Matrix::GetCM(double *B) {
	for(int j=0;j<c;j++) 
		for(int i=0;i<r;i++) *(B++)=v[i][j];
}

void Matrix::GetCM(float *B) {
	for(int j=0;j<c;j++) 
		for(int i=0;i<r;i++) *(B++)=(float)(v[i][j]);
}

void Matrix::Get(float *B) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) *(B++)=(float)(v[i][j]);
}

void Matrix::Get(double *B) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) *(B++)=v[i][j];
}

void Matrix::Set(const double *B) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) v[i][j]=*(B++);
}

void Matrix::Set(const float *B) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) v[i][j]=*(B++);
}

void Matrix::Set(double val00,...) {
	va_list marker;
	va_start(marker,val00);
	v[0][0]=val00;

	int i,j=1;
	for(i=0;i<r;i++) {
		for(;j<c;j++) v[i][j]=va_arg(marker,double);
		j=0;
	}

	va_end(marker);
}


double Matrix::Determinante() {
	if (r!=c) ErrorExit("The determinant is not defined for this matrix.");

	NEWMAT::Matrix In_(r,c);
	for(int i=0;i<r;i++)
		for(int j=0;j<c;j++) In_[i][j]=v[i][j];
	return In_.determinant();	

}

Matrix *Matrix::GetMinor(int i_t,int j_t,Matrix *Minor) {
	int sr=0,sc;

	for(int i=0;i<r;i++) {
		if (i!=i_t) {
			sc=0;
			for(int j=0;j<c;j++) {
				if (j!=j_t) {
					Minor->v[sr][sc]=v[i][j];
					sc++;
				}
			}
			sr++;
		}
	}
	return Minor;
}

void Matrix::SetMinor(int i_t,int j_t,Matrix *Minor) {
	int sr=0,sc;

	for(int i=0;i<r;i++) {
		if (i!=i_t) {
			sc=0;
			for(int j=0;j<c;j++) {
				if (j!=j_t) {
					v[i][j]=Minor->v[sr][sc];
					sc++;
				}
			}
			sr++;
		}
	}
}

void Matrix::operator=(Matrix &m) {
	Set(&m);
}

void Matrix::Set(Matrix *B) {
	if (r!=B->r) ErrorExit("Operatore = su matrici di dimensione diversa");
	if (c!=B->c) ErrorExit("Operatore = su matrici di dimensione diversa");

	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) 
			v[i][j]=B->v[i][j];
		
}

void Matrix::Add(Matrix *B) {
	if (r!=B->r) ErrorExit("Operatore + su matrici di dimensione diversa");
	if (c!=B->c) ErrorExit("Operatore + su matrici di dimensione diversa");

	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) 
			v[i][j]+=B->v[i][j];
}
	
void Matrix::SetZero() {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) v[i][j]=0;
}

void Matrix::SetIdentity() {
	int i,j,min;
	
	for(i=0;i<r;i++) {
		for(j=0;j<c;j++) v[i][j]=0;
	}

	// min is the min{r,c}
	min=r;
	if (c<min) min=c;
	
	for(i=0;i<min;i++) v[i][i]=1;
}

void Matrix::SetConstant(double val) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) v[i][j]=val;
}

void Matrix::Multiply(double a) {
	for(int i=0;i<r;i++) 
		for(int j=0;j<c;j++) 
			v[i][j]*=a;
}

void Matrix::Traspose() {
	if (r!=c) ErrorExit("The transpose is not defined for this matrix.");

	for(int i=0;i<r;i++) 
		for(int j=i+1;j<c;j++) {
			::swap<double>((v[i][j]),(v[j][i]));
		}
}
void Matrix::Traspose(Matrix *B) {
	if (r!=B->c) ErrorExit("Matrix size does not match.");
	if (c!=B->r) ErrorExit("Matrix size does not match.");

	for(int i=0;i<r;i++)
		for(int j=0;j<c;j++) B->v[j][i]=v[i][j];
}

void Matrix::Inversion() {
	if (r!=c) ErrorExit("The inverse is not defined for this matrix.");

	NEWMAT::Matrix In_(r,c);
	NEWMAT::Matrix Out_(r,c);
	for(int i=0;i<r;i++)
		for(int j=0;j<c;j++) In_[i][j]=v[i][j];
	try {
		Out_=In_.i();
	} catch(NEWMAT::BaseException) {
		ErrorExit("The inverse is not defined for this matrix.");
	}
	for(int i=0;i<r;i++)
		for(int j=0;j<c;j++) v[i][j]=Out_[i][j];
}


void Matrix::Print(char *MatrixName) {
	char pre[200];pre[0]=0;
	
	if (MatrixName!=NULL) {
		int i,len;
		len=((int)strlen(MatrixName))+1;
		for(i=0;i<len;i++) pre[i]=' ';
		pre[i]=0;
	}
	
	for(int i=0;i<r;i++) {
		if ((MatrixName!=NULL) && (i==(r/2))) printf("%s=[  ",MatrixName);
		else printf("%s[  ",pre);
		for(int j=0;j<c;j++) {				
			printf("%4f  ",(float)v[i][j]);
		}	
		printf("  ]\n");
	}
}

void Multiply(Matrix *A,Matrix *B,Matrix *C) {
	if (A->c!=B->r) ErrorExit("These matrices are not compatible for multiplication.");

	for(int i=0;i<A->r;i++) {
		for(int j=0;j<B->c;j++) {
			C->v[i][j]=0;
			for(int k=0;k<A->c;k++) {
				C->v[i][j]+=A->v[i][k]*B->v[k][j];
			}
		}
	}
}

void Multiply(Matrix *M,Matrix *T) {
	if (T->r!=T->c) ErrorExit("These matrices are not compatible for multiplication.");
	if (T->r!=M->c) ErrorExit("These matrices are not compatible for multiplication.");
	
	Matrix original(M->r,M->c);
	original.Set(M);

	Multiply(&original,T,M);
}

void PreMultiply(Matrix *T,Matrix *M) {
	// M=T*M
	if (T->c!=M->r) ErrorExit("These matrices are not compatible for multiplication.");
	if (T->r!=M->r) ErrorExit("These matrices are not compatible for multiplication.");

	Matrix original(M->r,M->c);
	original.Set(M);

	Multiply(T,&original,M);
}

double Matrix::IsOrthonormal(bool check_rigid) {
	Matrix B(3,3),C(3,3);
	B.Set(this);
	B.Traspose();
	::Multiply(this,&B,&C);
	
	double sum=0;
	for(int i=0;i<3;i++) {
		for(int j=0;j<3;j++) {
			if (i!=j) sum+=fabs(C[i][j]);
			else sum+=fabs(C[i][j]-1.0);
		}
	}
	sum/=9.0;
	
	if (sum>10e-8) return sum;
	if ((check_rigid) && (Determinante()<0)) return DBL_MAX;
	return 0.0;
}



void RelativeRotation(Matrix *FinalMatrix,Matrix *InitialMatrix,Matrix *Rotation) {
	// FinalR=Rotation*InitialR
	
	Matrix tmp(3,3);
	tmp.Set(InitialMatrix);
	tmp.Inversion();
	Multiply(FinalMatrix,&tmp,Rotation);
}


void Filter(Matrix *A,Matrix *B,Matrix *C) {
	if (B->r%2==0) ErrorExit("Matrix rows and columns must be odd.");
	if (B->c%2==0) ErrorExit("Matrix rows and columns must be odd.");

	int wer=(B->r-1)>>1;
	int wec=(B->c-1)>>1;
	for(int i=0;i<C->r;i++) {
		for(int j=0;j<C->c;j++) {
			(*C)[i][j]=0.0;
			for(int s=-wer;s<=wer;s++) {
				for(int t=-wec;t<=wec;t++) {
					if ((i+s>=0) && (i+s<A->r) && (j+t>=0) && (j+t<A->c)) {
						(*C)[i][j]+=((*A)[i+s][j+t])*((*B)[s+wer][t+wec]);
					}
				}
			}
		}
	}
}


void Matrix::SetGaussian(double sigma) {
	if ((c!=r) || (r%2==0)) ErrorExit("The matrix should be square and odd.");

	int we=(r-1)>>1;
	double d=1.0/(2.0*sigma*sigma);

	if (sigma==0.0) {
		SetZero();
		v[we][we]=1.0;
	} else {
		for(int i=-we;i<=we;i++) {
			for(int j=-we;j<=we;j++) {
				v[i+we][j+we]=exp(-((i*i)+(j*j))*d);
			}
		}
		double sum=0.0;
		for(int i=0;i<r;i++) 
			for(int j=0;j<c;j++) sum+=v[i][j];
		for(int i=0;i<r;i++) 
			for(int j=0;j<c;j++) v[i][j]/=sum;
	}
}
