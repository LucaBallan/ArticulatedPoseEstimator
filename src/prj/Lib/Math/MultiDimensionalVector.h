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





// Tutte ok e testate tranne le TODO

class TensorIndex {
public:
	int *i;
	int n;	

	TensorIndex(int order);
	~TensorIndex();

	void clear();
	void set(int *pos);
	void cat(int h);
	void cat(TensorIndex *b,int start=0);
	void cat(TensorIndex *b,int start,int end);

	bool isValid(int order,int *sizes);
	bool compare(TensorIndex *b);
	bool is_symmetric();
	void print();


};


template <class Element_Type>
class Tensor {
	int n;
	Element_Type *value;
	int *dim;	
	int size;

	// private functions
	void PrintLin(char *tensor_name,char *format);
	void PrintLin(int st_order,int *ref_pos,char *format);
	void Print2D(char *format,int st_order,int *ref_pos,int row,int offset,int screen_size);
	void Print2D_Mat(char *format,int st_order,int *pos,int row,int offset,int screen_size);
	

public:
	 Tensor(int n,const int size0,...);						// n, size(0), size(1), ..., size(n-1)
	 Tensor(int n,const int *size);
	 //Tensor(Matrix *m);
	 Tensor(Bitmap<Element_Type> *img);
	 Tensor(const Tensor<Element_Type> &b);
	~Tensor();


	
	// Coordinates
		int  coords2index(const int *pos) const;
		void index2coords(const int index,int *pos) const;


	// Component access
		// Linear access
		Element_Type       &vl(const int i);
		const Element_Type &vl(const int i) const;

		// Vector access
		Element_Type       &vv(const int *pos);
		const Element_Type &vv(const int *pos) const;

		// Element list
		Element_Type       &v(const int i_0,...);
		const Element_Type &v(const int i_0,...) const;
		

	// Info
		void Print(char *tensor_name=NULL,char *format="%.2f",bool matrix_form=true);
		
		bool isSquare();
		int  order();				// tensor order
		int  sizes(int dimension);	// tensor size[dimension]
		int *sizes();				// tensor size[dimension]
		int  dimension();			// tensor space dimension

	
	// Set Vector
		void Set(Element_Type x);
		void Set_to_Indices();
		void SetIdentity();
		void SetRandom();			// Random tensor whose components are between 0.0 - 1.0

		


	// TODO
		int  getstep(int dim_index) const;	// MULTI_TODO		// dato una coordinata x(0),..,0,..,x(n-1)  dove lo 0 e' piazzato sulla dimensione direction
											// gli indici della linea passante per x e parallela a direction
											// sono coords2index(x)+getstep(direction)*k   k=0,...,dim[direction]-1
		void subspace_explore(int *s_pos,int missing_dim_index,int &start_index,int &step,int &len) const; // ritorna i parametri per trovare tutti i punti della linea definita da pos e missing_dim_index
		void setBitmap(void *img);				// (Bitmap<Element_Type> *) TODO
		void *retrieveBitmap();			// (Bitmap<Element_Type> *) TODO
	// TODO




	

	// SubSpaces
	static const int variabile(const int b_index);
		// Vector access
		void insert(const Tensor<Element_Type> *b,const int *indexes);
		void extract(Tensor<Element_Type> *b,const int *indexes) const;

		// Element list
		void insert(const Tensor<Element_Type> *b,const int i0,...);		// b,v(0),v(1),x,v(3),...,v(n-1)
		void extract(Tensor<Element_Type> *b,const int i0,...) const;		// x=variable(index of b) 
																							// Es: A.insert(&B,3,Tensor<double>::variabile(1),Tensor<double>::variabile(0));
																							// TODO .. extract e extract_v non l'ho testato ma dovrebbe funzionare

		// Direct Output
		Tensor<Element_Type> *extract(const int i0,...) const;// TODO
		Tensor<Element_Type> *resize() const;// TODO





	// Similar to a reshape
	const Tensor<Element_Type> &operator=(const Tensor &b);	// TODO

	
	// Operations
	const Tensor<Element_Type> &operator+=(const Tensor<Element_Type> &b);// TODO
	const Tensor<Element_Type> &operator-=(const Tensor<Element_Type> &b);// TODO
	const Tensor<Element_Type> operator+(const Tensor<Element_Type> &b) const;// TODO
	const Tensor<Element_Type> operator-(const Tensor<Element_Type> &b) const;// TODO
	const Tensor<Element_Type> &operator*=(const Element_Type &b);// TODO
	const Tensor<Element_Type> operator*(const Element_Type &b) const;// TODO


	void Diagonal(Element_Type empty,Element_Type x0,...);					// 0, x0, x1, .., x(dim[0]-1)


	void DWT(int dimension_index,Tensor<Element_Type> *&l,Tensor<Element_Type> *&h) const;	// TODO
	void iDWT(int dimension_index,Tensor<Element_Type> *l,Tensor<Element_Type> *h);

	Tensor<Element_Type> **DWT();
	void iDWT(Tensor<Element_Type> **a);
};



template <class Element_Type>
void Multiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C);

template <class Element_Type>
void Multiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C,int q,int shift=-1);

template <class Element_Type>
void PointMultiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C,int shift=-1);

template <class Element_Type>
void TensorMultiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C);




void WAVELET_FUS(Tensor<WI_Type> *A,Tensor<WI_Type> *B,Tensor<ColorFloat> *mA,Tensor<ColorFloat> *mB,Tensor<WI_Type> *C,int level);








////////////////////////////////////////////////////
// Theorem 1:
////////////////////////////////////////////////////
/***************************************************
	srand(time(NULL));
	Tensor<double> X(2,2,3);
	X.SetRandom();
	X.Print("X");
	Tensor<double> Y(2,4,5);
	Y.SetRandom();
	Y.Print("Y");
	Tensor<double> Z(4,2,3,4,5);
	TensorMultiply(&X,&Y,&Z);
	Z.Print("X*Y","%.5f");
	Tensor<double> M(6,4,2,5,4,3,2);
	M.SetRandom();
	M.Print("M");
	Tensor<double> Q(2,4,2);
	PointMultiply(&M,&Z,&Q);
	Q.Print("M.*(X*Y)");
	Q.SetRandom();
	Tensor<double> S(4,4,2,5,4);
	PointMultiply(&M,&X,&S);
	PointMultiply(&S,&Y,&Q);
	Q.Print("(M.*X).*Y");
	Q.SetRandom();
	Tensor<double> D(4,4,2,3,2);
	PointMultiply(&M,&Y,&D,M.order()-Y.order());
	PointMultiply(&D,&X,&Q);
	Q.Print("(M(.*2)Y).*X");
****************************************************/
	

