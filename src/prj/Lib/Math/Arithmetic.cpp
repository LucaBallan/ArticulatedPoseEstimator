/////////////////////////////////////////////////////////////////////////////////////////////////

// Polynomial Root Finder (recursive bisection/binary-search algorithm)
// Copyright (c) 2003, by Per Vognsen.
// http://www.flipcode.com/archives/Polynomial_Root-Finder.shtml

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "lib.h"
using namespace std;





template<typename T>
inline int sign(T x) {
	if (x>0)        return 1;
	else if (x<0)   return -1;
	else            return 0;
}



inline void differentiate(double *coeff,unsigned int coeff_size,double *deriv) {			// deriv -> coeff_size-1
	for(unsigned int i=0;i<coeff_size-1;i++) 
		deriv[i]=(i+1)*coeff[i+1];
}



inline double evaluate(double *coeff,unsigned int coeff_size,double x) {
	double lc=coeff[coeff_size-1];
	
	if (x==DBL_MAX) return (lc>0? (DBL_MAX) : (-DBL_MAX));
	if (x==-DBL_MAX) {
		if (coeff_size%2==0) return (lc>0? (-DBL_MAX) : (DBL_MAX));
		else                 return (lc>0? (DBL_MAX)  : (-DBL_MAX));
	}

	double value=0.0,y=1.0;
	
	for (unsigned int i=0;i<coeff_size;i++) {
		value+=coeff[i]*y;
		y*=x;
	}

	return value;
}




inline double finite_bisect(double *coeff,unsigned int coeff_size,double low,double high,
							double ROOT_FINDER_TOLLERANCE_Y,
							double ROOT_FINDER_TOLLERANCE_X,
							unsigned int ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS) {

	if (high<low) std::swap(low,high);

	
	double   e_low=evaluate(coeff,coeff_size,low);
	int    s_e_low=sign(e_low);

	for (unsigned int i=0;i<ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS;i++) {
		if (IS_ZERO(e_low,ROOT_FINDER_TOLLERANCE_Y)) return low;
	
		double mid=0.5*low+0.5*high;
		double e_mid=evaluate(coeff,coeff_size,mid);
		int    s_e_mid=sign(e_mid);
		
		if (s_e_low != s_e_mid) {
			high=mid;
		} else {
			low=mid;
			e_low=e_mid;
			s_e_low=s_e_mid;
		}

		if ((high-low)<ROOT_FINDER_TOLLERANCE_X) return ((low+high)/2.0);
	}

	// ROOT_FINDER_TOLLERANCE_Y not satisfied
	return ((low+high)/2.0);
}




inline double bisect(double *coeff,unsigned int coeff_size,double low,double high,	  					 
					 double ROOT_FINDER_TOLLERANCE_Y,
					 double ROOT_FINDER_TOLLERANCE_X,
					 unsigned int ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS) {

	if (high<low) std::swap(low,high);
	
	double e=1.0;
	if (low==(-DBL_MAX)) {
		low=high;
		int s_high=sign(evaluate(coeff,coeff_size,high));
		do {
			low-=e;
			e*=2.0;
		} while (sign(evaluate(coeff,coeff_size,low)) == s_high);
	} else if (high == (DBL_MAX)) {
		high=low;
		int s_low=sign(evaluate(coeff,coeff_size,low));
		do {
			high+=e;
			e*=2.0;
		} while (sign(evaluate(coeff,coeff_size,high)) == s_low);
	}

	return finite_bisect(coeff,coeff_size,low,high,ROOT_FINDER_TOLLERANCE_Y,ROOT_FINDER_TOLLERANCE_X,ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS);
}

// coeff_size -> polinomio di (coeff_size-1) grado -> (coeff_size-1) soluzioni
void find_roots_internal(double *coeff,unsigned int coeff_size,Array<double> *roots,
						 double ROOT_FINDER_TOLLERANCE_Y,
						 double ROOT_FINDER_TOLLERANCE_X,
						 unsigned int ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS) {

	// polinomio di (coeff_size-1) grado

	if (coeff_size==2) {
		// polinomio di 1' grado
		roots->append(-coeff[0]/coeff[1]);
		return;
	}

	double *deriv=new double[coeff_size-1];				// deriv polinomio di (coeff_size-2) grado
	differentiate(coeff,coeff_size,deriv);
	Array<double> deriv_roots(coeff_size+(-2+2));		// afterwards we add 2 solutions
	find_roots_internal(deriv,coeff_size-1,&deriv_roots,ROOT_FINDER_TOLLERANCE_Y,ROOT_FINDER_TOLLERANCE_X,ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS);
	delete []deriv;


	if (deriv_roots.numElements()==0) deriv_roots.append(0.0);
	deriv_roots.append(-DBL_MAX);
	deriv_roots.append(+DBL_MAX);
	
	// sort and delete duplicates
	deriv_roots.sort(sort_increasing_compare<double>);
	for(int i=1;i<deriv_roots.numElements();i++) {
		if (deriv_roots[i]-deriv_roots[i-1]<ROOT_FINDER_TOLLERANCE_X) {
			deriv_roots.del(i);
			i--;
		}
	}
	
	int deriv_roots_size_1=deriv_roots.numElements()-1;
	for(int i=0;i<deriv_roots_size_1;i++) {
		double first_x=deriv_roots[i],
			   second_x=deriv_roots[i+1];
		double first_value = evaluate(coeff,coeff_size,first_x),
			   second_value = evaluate(coeff,coeff_size,second_x);
		
		if (IS_ZERO(first_value,ROOT_FINDER_TOLLERANCE_Y) || sign(first_value) != sign(second_value)) {
			roots->append(bisect(coeff,coeff_size,first_x,second_x,ROOT_FINDER_TOLLERANCE_Y,ROOT_FINDER_TOLLERANCE_X,ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS));
		}
	}

	// sort and delete duplicates
	roots->sort(sort_increasing_compare<double>);
	for(int i=1;i<roots->numElements();i++) {
		if ((*roots)[i]-(*roots)[i-1]<ROOT_FINDER_TOLLERANCE_X) {
			roots->del(i);
			i--;
		}
	}

}



void find_poly_roots(double *coeff,unsigned int coeff_size,Array<double> *solutions,
 					 double ROOT_FINDER_TOLLERANCE_Y,
					 double ROOT_FINDER_TOLLERANCE_X,
					 unsigned int ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS) {

	solutions->clear();

	// Simplify polinomial
	while((coeff_size>0) && IS_ZERO(coeff[coeff_size-1],ROOT_FINDER_TOLLERANCE_X)) coeff_size--;
	if (coeff_size<=1) {
		solutions->append(DBL_MAX);
		return;
	}

	// Find roots
	find_roots_internal(coeff,coeff_size,solutions,ROOT_FINDER_TOLLERANCE_Y,ROOT_FINDER_TOLLERANCE_X,ROOT_FINDER_TOLLERANCE_MAX_ITERATIONS);
}






double *polynomial(double a, double b) {
	double *coeff=new double[3];
	coeff[0]=a*b;
	coeff[1]=-(a+b);
	coeff[2]=1.0;
	return coeff;
}
double *polynomial(double a,double b,double c) {
	double *coeff=new double[4];
	coeff[0]=(-(a*b*c));
	coeff[1]=(a*b+a*c+b*c);
	coeff[2]=(-(a+b+c));
	coeff[3]=(1.0);
	return coeff;
}
double *polynomial(double a,double b,double c,double d) {
	double *coeff=new double[5];
	coeff[0]=(a*b*c*d);
	coeff[1]=(-(a*b*d+a*c*d+b*c*d+a*b*c));
	coeff[2]=(a*b+a*c+b*c+a*d+b*d+c*d);
	coeff[3]=(-(a+b+c+d));
	coeff[4]=(1.0);
	return coeff;
}














RandomSelector::RandomSelector(int num_ele) {
	list_ele=new Array<int>(num_ele);
	for(int i=0;i<num_ele;i++) list_ele->append(i);
}
RandomSelector::~RandomSelector() {
	delete list_ele;
}
int RandomSelector::select() {
	int c_num=list_ele->numElements();
	if (c_num==0) return -1;
	int v_index=Uniform_int_rand(0,c_num-1);
	int ele_index=(*list_ele)[v_index];
	list_ele->del(v_index);
	return ele_index;
}






























/*
// TEST Poly functionality

void test_polynomial(int num,double low, double high) {
	Array<double> v_roots(4);
	v_roots.append(random_double(low, high));
	v_roots.append(random_double(low, high));
	if (num>=3) v_roots.append(random_double(low, high));
	if (num==4) v_roots.append(random_double(low, high));
	v_roots.sort(sort_increasing_compare<double>);

	double *x;
	if (num==2) x=polynomial(v_roots[0],v_roots[1]);
	if (num==3) x=polynomial(v_roots[0],v_roots[1],v_roots[2]);
	if (num==4) x=polynomial(v_roots[0],v_roots[1],v_roots[2],v_roots[3]);
	double *q=new double [10];
	for(int i=0;i<10;i++) q[i]=0.0;
	for(int i=0;i<num+1;i++) q[i]=x[i];

	Array<double> roots(num);
	find_poly_roots(q,10,&roots);

	if (roots.numElements()!=v_roots.numElements()) {
		cout<<"diverse soluzioni\n";
	}
	for (unsigned int i = 0; i < v_roots.numElements(); i++)
		std::cout << v_roots[i] << " ";
	std::cout << std::endl;

	for (unsigned int i = 0; i < roots.numElements(); i++)
		std::cout << roots[i] << " ";
	std::cout << std::endl;
	std::cout << std::endl;
	delete []x;
}
*/





















#define   POLYNOMIAL_MAX_NUM_VALUES  7
Matrix   *polynomial_inverse  [POLYNOMIAL_MAX_NUM_VALUES]   = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};
double   *polynomial_predictor[POLYNOMIAL_MAX_NUM_VALUES]   = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};

void CreatePolynomialInverse(int num_values) {
	if ((num_values>=POLYNOMIAL_MAX_NUM_VALUES) || (num_values<=0)) ErrorExit("Degree number not supported.");
		
	polynomial_inverse[num_values]=new Matrix(num_values,num_values);

	(*(polynomial_inverse[num_values]))[0][0]=1.0;
	for(int j=2;j<=num_values;j++) (*(polynomial_inverse[num_values]))[0][j-1]=0.0;
	for(int i=2;i<=num_values;i++) {
		(*(polynomial_inverse[num_values]))[i-1][0]=1.0;
		for(int j=2;j<=num_values;j++) (*(polynomial_inverse[num_values]))[i-1][j-1]=pow(-(i-1.0),j-1);
	}
	polynomial_inverse[num_values]->Inversion();
}

void PolynomialFitting(int num_values,double *values,double *coefficients) {
	
	if (polynomial_inverse[num_values]==NULL) CreatePolynomialInverse(num_values);
	
	for(int i=0;i<num_values;i++) {
		coefficients[i]=0.0;
		for(int j=0;j<num_values;j++) {
			coefficients[i]+=((*(polynomial_inverse[num_values]))[i][j]) * (values[j]);
		}
	}

}

double PolynomialPredictor(int num_values,double *values) {
	if (polynomial_predictor[num_values]==NULL) {
		if (polynomial_inverse[num_values]==NULL) CreatePolynomialInverse(num_values);
		polynomial_predictor[num_values]=new double[num_values];
		
		for(int i=0;i<num_values;i++) {
			(polynomial_predictor[num_values])[i]=0.0;
			for(int j=0;j<num_values;j++) {
				(polynomial_predictor[num_values])[i]+=((*(polynomial_inverse[num_values]))[j][i]);
			}
		}
	}
	
	double prediction=0.0;
	for(int j=0;j<num_values;j++) {
		prediction+=((polynomial_predictor[num_values])[j]) * values[j];
	}

	return prediction;
}










// 
// FiniteDifferencesTemplate
// 

int FiniteDifferencesTemplate(int i_min,int i_max,int d,int h,double *&Template) {
	int n=i_max-i_min+1;
	int p=i_max-i_min+1-d;
	if ((p<1) || (i_min>i_max)) ErrorExit("Can't generate this finite difference template.");

	Matrix M(n,n);
	for(int j=0;j<n;j++) {
		for(int i=i_min;i<=i_max;i++) {
			M[j][i-i_min]=pow(1.0*i,j);
		}
	}
	M.Inversion();
	if (!Template) Template=new double[n];
	M.GetColumn(d,Template);

	double ratio=factorial(d)/pow(1.0*h,d);
	for(int j=0;j<n;j++) Template[j]=ratio*Template[j];

	return p;
}

void FiniteDifferencesTemplate(int i_min,int i_max,int d_x,int d_y,int h_x,int h_y,Matrix *&Template,int &p_x,int &p_y,double smoothing_sigma) {
	double *x=NULL;
	double *y=NULL;
	int     n=i_max-i_min+1;

	p_x=FiniteDifferencesTemplate(i_min,i_max,d_x,h_x,x);
	p_y=FiniteDifferencesTemplate(i_min,i_max,d_y,h_y,y);
	
	if (!Template) Template=new Matrix(n,n);
	for(int j=0;j<n;j++) {
		for(int i=0;i<n;i++) {
			(*Template)[j][i]=x[i]*y[j];
		}
	}
	
	if (smoothing_sigma!=0.0) {
		Matrix G(n,n);
		G.SetGaussian(smoothing_sigma);
		Matrix F(n,n);
		Filter(Template,&G,&F);
		Template->Set(&F);
	}

	delete[]x;
	delete[]y;
}











