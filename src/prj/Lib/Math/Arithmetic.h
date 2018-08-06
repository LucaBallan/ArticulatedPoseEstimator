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





// Polynomial Root Finder (recursive bisection/binary-search algorithm)
//
// 0==y=f(x)=a(n)x^n+a(n-1)x^(n-1)+...+a(1)x+a(0)
//
// Trova tutte le soluzioni del polinomio:
//     Per ogni soluzione trovata x vale almeno una delle seguenti
//          1) |f(x)|<ROOT_FINDER_TOLLERANCE_Y
//          2) |x-x*|<ROOT_FINDER_TOLLERANCE_X
//                  Dove x* e' la vera soluzione al problema
//          3) x e' vicino a x* ma non ho tempo per trovarla
//
// Le soluzioni sono ordinate dalla piu' piccola alla piu' grande e non ci sono doppie
// ci possone essere al piu' dim_vector-1 soluzioni
//
//
// In:
//    [a(0),a(1),...,a(n-1)] -> polinomio con n elementi, di ordine n-1, con max n-1 soluzioni
//
// Out:
//    Dichiarare un Array<double> solutions(n-1)
//         solutions[0]==DBL_MAX  -> polynomial impossible or indeterminate
//         solutions[..]          -> soluzioni del polinomio
//
//
// Testato molto accuratamente e molto ottimizzato in velocita'
//
void find_poly_roots(double *coeff,unsigned int coeff_size,Array<double> *solutions,
					 double ROOT_FINDER_TOLLERANCE_Y=0.0000001,
					 double ROOT_FINDER_TOLLERANCE_X=0.0000000001,
					 unsigned int ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS=100000);


double *polynomial(double a,double b);
double *polynomial(double a,double b,double c);
double *polynomial(double a,double b,double c,double d);





//
// Extract numbers form a bag 
//
class RandomSelector {
	Array<int> *list_ele;
public:
	RandomSelector(int num_ele);
	~RandomSelector();
	int select();						// -1 if there is no more numbers
};







//
// PolynomialFitting (TESTED ACCURATIVELY)
//
// values[0]            = f(0)
// values[1]            = f(-1)
// ....
// values[num_values-1] = f(-num_values+1)
//
//
// coefficients[0]            = a0
// coefficients[1]            = a1
// ....
// coefficients[num_values-1] = a(-num_values+1)
//
//				order         = num_values - 1
//

void   PolynomialFitting  (int num_values,double *values,double *coefficients);			// return the polinomial
double PolynomialPredictor(int num_values,double *values);								// return f(1) prediction








// 
// FiniteDifferencesTemplate (TESTED ACCURATIVELY)
// 
// d = derivative order: 1 first derivate, 2 second derivate, ...
// p = returned value
// 
// 
// double *Template=double [i_max-i_min+1]
// 
// 
//   F(d)(x) =   sum_i{i_min,i_max} ( Template[i-i_min] * F(x+i*h) )      +   O(h^p)
// 
// 

int   FiniteDifferencesTemplate(int i_min,int i_max,int d,int h,double *&Template);					// Template=NULL -> auto Creation, to delete with delete[]
void  FiniteDifferencesTemplate(int i_min,int i_max,int d_x,int d_y,int h_x,int h_y,				// Template=NULL -> auto Creation, to delete with delete
								Matrix *&Template,          
								int &p_x,int &p_y,double smoothing_sigma);			





