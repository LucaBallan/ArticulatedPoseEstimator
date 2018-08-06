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





//***************************************************************************************
//***************************** Vector **************************************************
//***************************************************************************************

typedef double Element_Type;
#define Element_Type_Max DBL_MAX



template <int dim> 
class Vector {
protected:
	Element_Type value[dim];


public:
	Vector();
	Vector(const Element_Type *c);
	~Vector() {};


	// equality properties
	bool operator== (const Vector<dim> &v) const;
	bool operator!= (const Vector<dim> &v) const; 
	bool isEqual    (const Vector<dim> &x,double tollerance=DBL_EPSILON) const;
	bool isZero     ();
	bool isFinite   ();


	// components
	      Element_Type &operator[](const int i) {return value[i];}
	const Element_Type &operator[](const int i) const {return value[i];}


	// global property
	      Element_Type  Norm2     () const;


	// operatori binari
	const Vector<dim>  &operator= (const Vector<dim> &v);
	const Vector<dim>  &operator= (const Element_Type *c);
	const Vector<dim>   operator+ (const Vector<dim> &v) const;
	const Vector<dim>  &operator+=(const Vector<dim> &v);
	const Vector<dim>   operator- (const Vector<dim> &v) const;
	const Vector<dim>  &operator-=(const Vector<dim> &v);
	const Vector<dim>   operator* (const Element_Type &a) const;
	const Vector<dim>  &operator*=(const Element_Type &a);
	const Vector<dim>   operator^ (const Vector<dim> &v) const;
	      Element_Type  operator* (const Vector<dim> &v) const;

	

	// components change
	      Vector<dim-1> Projection         () const;
	const Vector<dim>   Projection         (const Vector<dim> &versor) const;
	const Vector<dim>   PositiveProjection (const Vector<dim> &versor) const;
	      Vector<dim>   ToCyl              () const;                             // [r,theta,....]  theta e' tra[-pi,+pi]
	      Vector<dim>   FromCylToR3        () const;	
	      Vector<dim>   abs                () const;
	      Vector<dim>   toInteger          () const;
	      Vector<dim>   Versore            () const;



	// operation on the vector
	void  SetZero();
	void  SetVal(Element_Type x);
	void  Normalize();
	void  Random();							// Random vector whose components are between 0.0 - 1.0
	void  Min(Vector<dim> *x);				// this = min(this,x)
	void  Max(Vector<dim> *x);				// this = max(this,x)


	// low level i/o
	inline Element_Type *GetList();
	void SetList(const float *c);
	void SetList(const double *c);
	void GetList(float *c);
	void GetList(double *c);
	void SerializeFloat(float *buffer);

};


// Costruttori inlinea
Vector<2> VECTOR(Element_Type x,Element_Type y);
Vector<3> VECTOR(Element_Type x,Element_Type y,Element_Type z);




template <int dim>
const Vector<dim> operator*(const Element_Type &a, const Vector<dim> &p) ;

template <class stream_type,int dim>
stream_type &operator<<(stream_type &os, const Vector<dim> &p);

template <class stream_type,int dim>
stream_type &operator>>(stream_type &os, Vector<dim> &p);

template <int dim>
void Multiply(Matrix *A,Vector<dim> b,Element_Type *c);				// c deve avere len = A->r

void Multiply(Matrix *A,Vector<3> *b,Vector<3> *c);					// faster



template <int dim>
Vector<dim> Max(Vector<dim> a,Vector<dim> b);

template <int dim>
Vector<dim> Min(Vector<dim> a,Vector<dim> b);


Vector<2> GetOrthogonalVersor(Vector<2> versor);					// find a versor orthogonal to a (a must be a versor)


/////////////////////////////////////////////////////////////////////////////////////////////
//                      Extrinsic, Intrinsic and Projection Matrices                      ///
/////////////////////////////////////////////////////////////////////////////////////////////

//
//  E   = [R,T];
//  P   = [E;0,0,0,1];
//  Ext = E con un asse invertito...
//


// 3 point (P1,P2,P3) -> axes [R,P1]
bool PstoRT(Vector<3> P1,Vector<3> P2,Vector<3> P3,Matrix *axes);		// return false if it cannot extract an axes matrix


// [R,T] <-> P
void RTtoP(Matrix *P,Matrix *R,Vector<3> *T);		// Convert [R,T] to P (3x4 or 4x4)
void PtoRT(Matrix *P,Matrix *R,Vector<3> *T);		// Convert P (3x4 or 4x4) in [R,T]

// E <-> P
void RtoP(Matrix *R,Matrix *P);						// Convert R (3x3) to P (4x4)
void EtoP(Matrix *E,Matrix *P);						// Convert E (3x4 or 4x4) to P (4x4)
void PtoE(Matrix *P,Matrix *E);						// Convert P (3x4 or 4x4) to E (3x4)
void GtoG(Matrix *src,Matrix *dst);					// Convert src (3x3,3x4,4x4) to dst (3x4,4x4) (automatically detecting the format)

// P <-> GL <- RT
void PtoGL(Matrix *P,double *m);					// Convert P (3x4 or 4x4) in GLMatrix (double m[16])
void GLtoP(double *m,Matrix *P);					// Convert GLMatrix (double m[16]) to P (3x3, 3x4 or 4x4)
void RTtoGL(Matrix *R,Vector<3> *T,double *m);

// Extrinsic information
void GetCPfromExt(Matrix *E,Vector<3> *cp);		    // Retrieve the center of projection of Ext (3x4)

// Inversiom Extrinsic
void Invert3x4Matrix(Matrix *M);					// Invert 3x4 matrix in Euclidean fashion
void Invert2x3Matrix(Matrix *M);					// Invert 2x3 matrix in Euclidean fashion
void Invert4x4RotoTraslationMatrix(Matrix *M);		// Invert 4x4 matrix in Euclidean fashion (Faster than computing the inverse of M)

void InvertHandConvention(Matrix *x);				// Invert Hand Convention of x (3x3 or 3x4)
													// x has to represents on each coloum the representing system (otherwise use its transpose)

// Scale
Vector<3> get_map_scale(Matrix *P);         		// Retrieve the linear map scale from P (3x3, 3x4 or 4x4)


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////
// Intersezioni
/////////////////////////////////////////////////////////////////////////////////////////////
#define INTERSECTION_NUMERICAL_TOLLERANCE			(1000*DBL_EPSILON)

template <int dim>
Element_Type Intersection_LinePlane(Vector<dim> o,Vector<dim> d,Vector<dim> n,Vector<dim> plane_point,Vector<dim> *p);  // Intersezione tra 
																														//       line   (o,d)           (passante per o e di direzione d)
																														//       piano  (n,plane_point) (normale n e passante per plane_point)
																														//              (double,p)      (founded intersection point p and signed distance of p from o, Element_Type_Max if no intersection)

/////////////////////////////////////////////////////////////////////////////////////////////


void CrossMatrix(Vector<2> p,Matrix *out);
void CrossMatrix(Vector<3> p,Matrix *out);





// BuildRotationMatrix 
// -> Crea una matrice di Rotazione 3x3 (Orthonormale)
//    con 
//			asse_riferimento  (solo normalizzato) in indice_asse_riferimento
//          asse_ausiliario   (normalizzato e aggiustato) in indice_asse_ausiliario
//
//   asse_ausiliario   facoltativo -> random
//   make_rigid		   Rende la rotazione rigida (det=+1)
//
void BuildRotationMatrix(Matrix *Rot,Vector<3> asse_riferimento,int indice_asse_riferimento,Vector<3> *asse_ausiliario=NULL,int indice_asse_ausiliario=-1,bool make_rigid=false);





void CorrectExtrinsic(Matrix *M);									// Ensure Normal vectors 


// Ext (Extrinsic) <-> E (opengl)									// OBSOLETE TODO
void ExttoE(Matrix *Ext,Matrix *E);									// Convert Ext (3x4) to E (3x4)
void CorrectExtrinsic_For_OpenGL3D_Visualization(Matrix *M);		// Correct Extrinsic matrix M in order to use it with RenderWindow or Opengl3D MODELVIEW matrix
																	// OpenGl want a swapped X-axis of the camera
void InvertCorrectExtrinsic_For_OpenGL3D_Visualization(Matrix *M);  // do the opposite
																	




// AngleAxis - Rotation Matrix   (3x3 or 3x4 or 4x4 matrices)
void RotationMatrix(Matrix *out,double angle,double x,double y,double z);
void AngleAxis2RotationMatrix(Matrix *out,Vector<3> AngleAxis);
Vector<3> RotationMatrix2AngleAxis(Matrix *In);

void RotationMatrixToEuler(Matrix *C,double &a,double &b,double &c);
void EulerToRotationMatrix(Matrix *C,double a,double b,double c);


void Orthogonalize(Matrix *M);										// Ortogonalizza
void Orthonormalize(Matrix *x,bool make_rigid=false);				// OrtoNormalizza (se make_rigid=true -> impone det>0)



class Complex: public Vector<2> {
public:

	Complex();
	Complex(double Re,double Im);
	//Complex(const Element_Type *c);
	~Complex() {};
	
	double re();
	double im();

	static Complex sqrt(double x);

	const Complex &operator=(const Complex &v);
	const Complex operator+(const Complex &v) const;
	const Complex operator*(const Complex &v) const;
	const Complex operator*(const double &a) const;
	Complex inv() const;

};


class Quaternion: public Vector<4> {
public:

	Quaternion();
	Quaternion(Vector<4> quaternion);
	Quaternion(Vector<3> AngleAxis);
	~Quaternion() {};
	
	// Output
	double scalar() const;
	Vector<3> vect() const;
	Vector<4> qvect() const;
	
	// Input
	void set_scalar(double val);
	void set_vect(Vector<3> x);
	

	// Angle-Axis Function
	void Set_Angle_Axis(Vector<3> x);
	Vector<3> Angle_Axis();						// TODO: da controllare (costanti strane)

	
	// Operations
	const Quaternion &operator=(const Quaternion &v);
	const Quaternion operator+(const Quaternion &v) const;
	const Quaternion operator-(const Quaternion &v) const;
	const Quaternion operator*(const Quaternion &v) const;
	const Quaternion operator*(const double &a) const;
	
	
	// Specific functions
	inline Element_Type *GetList();
	Quaternion conjugate() const;  
	Quaternion inv() const;
	Quaternion exp() const;
	Quaternion power(const double t) const;
	Quaternion Log() const;
	


	// Interpolations
	static Quaternion Slerp(const Quaternion q0,const Quaternion q1,const double t);
	static Quaternion Interpolation(const Quaternion qA,const Quaternion qB,const double t,const Quaternion Target);
};

