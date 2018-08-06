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





class Matrix {
public:
	double **v;
	int r,c;

	Matrix(int r,int c);
	~Matrix();

	double *operator[](const int i) {return v[i];}
	void operator=(Matrix &m);

	double *GetRow(int i,double *r);
	double *GetColumn(int j,double *c);
	double GetColumnNorm(int j);
	Matrix *GetMinor(int i_t,int j_t,Matrix *Minor);
	void SetRow(int i,const double *r);
	void SetColumn(int j,const double *c);
	void SetMinor(int i_t,int j_t,Matrix *Minor);

	void Set(Matrix *B);
	// Get and Set Row Major
	void Get(float *B);
	void Get(double *B);
	void Set(const float  *B);
	void Set(const double *B);
	void Set(double val00,...);
	// Get and Set Coloum Major
	void GetCM(double *B);
	void GetCM(float *B);

	// Square Matrices
	double Determinante();
	void Inversion();
	void Traspose();

	
	// 3x3
	double IsOrthonormal(bool check_rigid=false);			// check_rigid = true -> controlla det>0 se <0 ritorna DBL_MAX

	// all matrices
	void Traspose(Matrix *B);	

	void Multiply(double a);
	void Add(Matrix *B);
	void Print(char *MatrixName=NULL);

	void SetZero();
	void SetIdentity();
	void SetConstant(double val);
	void SetGaussian(double sigma);
};


void Multiply(Matrix *A,Matrix *B,Matrix *C);			// C=A*B 
void Multiply(Matrix *M,Matrix *T);				        // M=M*T
void PreMultiply(Matrix *T,Matrix *M);				    // M=T*M
void Filter(Matrix *A,Matrix *B,Matrix *C);				// C=A(*)B


void RelativeRotation(Matrix *FinalMatrix,Matrix *InitialMatrix,Matrix *Rotation);

void GaussianFilter(int size,double sigma,Matrix *&G);



template <class stream_type>
stream_type &operator>>(stream_type &os, Matrix &p) {
	for(int i=0;i<p.r;i++) 
		for(int j=0;j<p.c;j++) os>>p[i][j];
	
	return os;
}

template <class stream_type>
stream_type &operator<<(stream_type &os, Matrix &p) {
	for(int i=0;i<p.r;i++) {
		int j=0;
		for(;j<(p.c-1);j++) os<<(p[i][j])<<" ";
		if (p.c!=0) os<<(p[i][j])<<"\r\n";
	}
	return os;
}

