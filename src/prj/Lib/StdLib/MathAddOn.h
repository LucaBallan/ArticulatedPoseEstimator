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





#include <time.h>


class PSO_Solver {
protected:
	int      n_dof;
	int      n_particles;
	double **particles;
	double **particles_speed;
	double **local_optimal;
	double  *local_optimal_cost;
	double   global_optimal_cost;

	virtual void   Initialize(double *particle,int p_index);

public:
	double  *max_x;
	double  *min_x;
	double  *global_optimal;


	PSO_Solver(int n_dof,int n_particles);
	virtual ~PSO_Solver();
	
	virtual double Evaluate(double *particle,bool generate_output) = 0;

	double Solve(double *initial_x,int n_iterations,bool verbose);
};


//
// Array operations
//
int  find_max    (const double *data,int n,int step=1);
int  find_min    (const double *data,int n,int step=1);
int  find_max    (const float  *data,int n,int step=1);
int  find_min    (const float  *data,int n,int step=1);
int *find_i_max  (double *data,int n,int i);
void normalize   (double *data,int n);
void mean_std    (const double *data,int n,double &mean,double &std,int step);

//
// Incremental array functions
//
class mean_std_data {
public:
	double sum;
	double sum_of_square;
	UINT   num_ele;
	double max;
	double min;
	void  *array_data;

	mean_std_data();
	void clear();
};

void mean_std_i        (double data,mean_std_data *info);
void mean_median_std_i (double data,mean_std_data *info);
void mean_std_r        (mean_std_data *info,double &mean,double &std);
void mean_std_r        (mean_std_data *info,double &mean,double &std,UINT &num_ele);
void mean_median_std_r (mean_std_data *info,double &mean,double &median,double &std,UINT &num_ele);



//
// Math functions
//
int     Approx(double x);
int     Approx(float x);
double  realAcos(double x);
int     sign(double x);
int     Module(int a,int module);					      // Integer Cyclic Group [0,(module-1)]											 (Perfect)
int	    ModuleDistance(int a,int b,int module);		      // Integer distance between elements in an Integer Cyclic Group [0,(module-1)]   (Perfect)
double  Module(double a,double module);				      // Real Cyclic Group [0,module)  (i.e., module is excluded)					     (Perfect)
double  ModuleDistance(double a,double b,double module);  // Real distance between elements in a Real Cyclic Group [0,module)               (seems ok)
double  PercentageRatio(double x,double total);
int     factorial(int n);

int ModuleSign(double a,double b,double module); // not tested


//
// Random variables
//
double Uniform_double_rand(double min,double max);       // Distribuzione uniforme continua tra min e max
int    Uniform_int_rand(int min,int max);                // Distribuzione uniforme discreta tra min e max COMPRESI
double Normal_double_rand(double mean,double sigma);     // Distrubuzione Gaussiana (Marsaglia polar method)




//
// Gaussian Filters Design
//      tollerance = ratio between the peak and the lower value
//
int    gaussian_min_width_from_sigma(double sigma,double tollerance);
double gaussian_max_sigma_from_width(int width,double tollerance);


//
// Filters
// 
//
template <class data_type>
class CasualFilter: private CircularArray<data_type> {
public:
	CasualFilter(int size);
	~CasualFilter();

	data_type filter(data_type x,data_type *filter);
};










//
// Roboust Estimation: Truncated Quadratic
//
class Truncated_Quadratic {
	double a,b,c,k,k_sqrt,k_ratio;
	double low_th,high_th;

public:
	Truncated_Quadratic();
	
	void initialize(double low_th,double high_th);
	
	inline double F(double x) {
		if (x<0) x=-x;
		if (x<=low_th) return (k*x*x);
		if (x<high_th) return (((a*x+b)*x)+c);
		return 1.0;
	}
	inline double DF(double x) {
		if (x<0) x=-x;
		if (x<=low_th) return (2*k*x);
		if (x<high_th) return ((2*a*x)+b);
		return 0.0;
	}
	inline double SqrtF(double x) {
		if (x<0) x=-x;
		if (x<=low_th) return (k_sqrt*x);
		if (x<high_th) return sqrt(((a*x+b)*x)+c);
		return 1.0;
	}
	inline double DSqrtF(double x) {
		if (x<0) x=-x;
		if (x<=low_th) return k_ratio;
		if (x<high_th) return ((2*a*x)+b)/(2*sqrt(((a*x+b)*x)+c));
		return 0.0;
	}

	void Graph(double a,double b,int num);
};






// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- Fixed point -------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef FIXED_POINT
	#define DECIMAL_BITS   10
	#define DECIMAL_MASK   0x3FF
	typedef int Fixed;

	// TODO: itofx <- perde negativita'!!!
	// TODO: fxtoi <- perde negativita'!!!
	#define itofx(x)   ((Fixed)((x) << DECIMAL_BITS))			// Integer to fixed point
	#define ftofx(x)   ((Fixed)((x) * (1<<DECIMAL_BITS)))		// Float to fixed point
	#define dtofx(x)   ((Fixed)((x) * (1<<DECIMAL_BITS)))		// Double to fixed point
	#define fxtoi(x)   ((x) >> DECIMAL_BITS)					// Fixed point to integer
	#define fxtof(x)   ((float) (x) / (1<<DECIMAL_BITS))	    // Fixed point to float
	#define fxtod(x)   ((double)(x) / (1<<DECIMAL_BITS))        // Fixed point to double
	#define Mulfx(x,y) (((y) * (x)) >> DECIMAL_BITS)			// Multiply a fixed by a fixed
	#define Divfx(x,y) (((y) << DECIMAL_BITS) / (x))    		// Divide a fixed by a fixed
	#define decimal(x) ((x) & DECIMAL_MASK)					    // Get decimal part  (TODO only for positive numbers)
	#define fxFloor(x)     (fxtoi(x))
	#define fxCeiling(x)   (fxtoi((x)+DECIMAL_MASK))
	#define fxApproxToi(x) (fxtoi((x)+(1<<(DECIMAL_BITS-1))))
	#define Printfx(x) printf("%i + %f", itofx(x), fxtod(decimal(x)));

#else 
	typedef float Fixed;

	#define itofx(x)        ((float)(x))
	#define ftofx(x)        ((float)(x))
	#define dtofx(x)        ((float)(x))
	#define fxtoi(x)        ((int)(x))
	#define fxtof(x)        ((float)(x))
	#define fxtod(x)        ((double)(x))
	#define Mulfx(x,y)      ((x)*(y))
	#define Divfx(x,y)      ((y)/(x))
	#define decimal(x)      ((x)-itofx(x))
	#define fxFloor(x)      (fxtoi(x))
	#define fxCeiling(x)    (fxtoi(ceil(x)))
	#define fxApproxToi(x)  (fxtoi(Approx((x))))
	#define Printfx(x)       printf("%f", (x));
#endif

template <class T,int prec>
class Fixed_Point {
	T val;
public:
	Fixed_Point() {};
	Fixed_Point(int x) {val=itofx(x);};
	Fixed_Point(float x) {val=ftofx(x);};
	Fixed_Point(double x) {val=dtofx(x);};

	static Fixed_Point<T,prec> SetFixed(T x) {
		Fixed_Point<T,prec> n;
		n.val=x;
		return n;
	}
	
	// TODO Passare da BYTE a int???
	bool operator== (const Fixed_Point<T,prec> &v) const {return (v.val==val);};
	bool operator!= (const Fixed_Point<T,prec> &v) const {return (v.val!=val);};

	const Fixed_Point<T,prec> operator=(const Fixed_Point<T,prec> &v) {return SetFixed(val=v.val);};
	const Fixed_Point<T,prec> operator=(const int v) {Fixed_Point<T,prec> x(v);val=x.val;return x;};
	const Fixed_Point<T,prec> operator=(const float v) {Fixed_Point<T,prec> x(v);val=x.val;return x;};
	const Fixed_Point<T,prec> operator=(const double v) {Fixed_Point<T,prec> x(v);val=x.val;return x;};

	const Fixed_Point<T,prec> operator+(const Fixed_Point<T,prec> &v) const {return SetFixed(val+v.val);};
	const Fixed_Point<T,prec> operator-(const Fixed_Point<T,prec> &v) const {return SetFixed(val-v.val);};
	const Fixed_Point<T,prec> operator*(const Fixed_Point<T,prec> &v) const {return SetFixed(Mulfx(val,v.val));};
	const Fixed_Point<T,prec> operator/(const Fixed_Point<T,prec> &v) const {return SetFixed(Divfx(val,v.val));};	// TODO: Ordine??!??

	const Fixed_Point<T,prec> operator+=(const Fixed_Point<T,prec> &v) {return SetFixed(val=(val+v.val));};
	const Fixed_Point<T,prec> operator-=(const Fixed_Point<T,prec> &v) {return SetFixed(val=(val-v.val));};
	const Fixed_Point<T,prec> operator*=(const Fixed_Point<T,prec> &v) {return SetFixed(val=Mulfx(val,v.val));};
	const Fixed_Point<T,prec> operator/=(const Fixed_Point<T,prec> &v) {return SetFixed(val=Divfx(val,v.val));};	// TODO: Ordine??!??

	operator int() const {return (fxtoi(val));};
	operator float() const {return (fxtof(val));};
	operator double() const {return (fxtod(val));};
	
	int floor() {return ((int)*this);}
	int ceiling() {return (fxCeiling(val));}
	int approx() {return (fxApproxToi(val));}
};











// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- Positional_Number -------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class Positional_Number {
private:
	unsigned int Len;
	unsigned int Base;
    unsigned int *Val;
	
public:
	bool RoundUp;													// Fast


	Positional_Number(unsigned int Len,unsigned int Base);
	~Positional_Number();

	const unsigned int &operator[](const int i) const {
		return Val[i];
	};

	Positional_Number *operator ++();											// Fast
	void SetZero();

	bool HasAllDifferentElements();
	bool IsInside(int x);											// one of the digit is equal to x 
};











// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- Timing ------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Precise Timer (substitute of time())
double   get_timer_frequency();
#define  TIMER_INIT                double timer_freq=get_timer_frequency();LARGE_INTEGER timer_start,timer_end;QueryPerformanceCounter(&timer_start);
#define  TIMER_QUERY               QueryPerformanceCounter(&timer_end);double timer_seconds=static_cast<double>(timer_end.QuadPart-timer_start.QuadPart);timer_seconds=timer_seconds/timer_freq;
#define  TIMER_UPDATE              timer_start.QuadPart=timer_end.QuadPart;


// Profiler
class Profiler {
	char Name[300];
	clock_t start;
	bool counting;
public:
	Profiler();
	Profiler(char *Name);
	~Profiler();
	
	double Stop();
};

// SpeedTester
#define SPEED_UNKNOW       DBL_MAX
class SpeedTester {
	bool first;
	UINT LastPosRefresh;
	time_t LastRefresh;
	double LastSpeed;

public:	
	SpeedTester();
	~SpeedTester() {};

	double CheckPoint(UINT Pos);			// Calcola la velocità di avanzamento della variabile Pos
											// Ritorna: double velocità
											//          SPEED_UNKNOW se nn è possibile calcolarla
	
	int RemainTime(UINT EndPos);			// Stima il numero di secondi perchè Pos arrivi a EndPos
											// Ritorna: numero secondi
											//          -1 se nn disponibile

	void RemainTime(char *Out,UINT EndPos);
};










// Probability maps
//
// G is a nosxnof Matrix   (row = nos, coloums = nof)
//
// G[i*nof+o]		<- Probability distribution for a random variable with parameter of index i  ( 0 ... nos )
//							i = 0			parameter = first_parameter_val
//			  				i = nos-1		parameter = last_parameter_val
//
// G[i*nof+j]		<- Probability of the distribution of being = j  ( 0 ... nof )
//
// G[i*nof+j] o G->Point(j,i) if it is a Bitmap<float>(nof,nos)
//
// Trucated gaussian zero centered with parameter sigma
void   CreateGaussianProbability(float_type *G,float_type *NG,double first_sigma,double last_sigma,int nof=256,int nos=256);






// Filters
//
// Filtra una sequenza di dati che e' non banale da leggere e scrivere
// 
typedef double (*SMOOTH_FILTER_FUNCTION_GET_TYPE)(int,void *);				// get(index,user_data)
typedef void (*SMOOTH_FILTER_FUNCTION_SET_TYPE)(int,double,void *);			// set(index,val,user_data)
void smooth_filter(int num_elements,SMOOTH_FILTER_FUNCTION_GET_TYPE get_data,SMOOTH_FILTER_FUNCTION_SET_TYPE set_data,void *userdata,bool Test,double sigma,double tollerance=1.0/15.0);

//		Crea un filtro di lunghezza size e tipo T
//				extension = (size-1)/2
//				[-extension,...,0,...,extension]
//		
//		- Ritorna un vettore di n_elements rappresentanti either:
//				full_filter ->  [-extension,...,0,...,extension]      n_elements=size
//				half_filter ->  [0,...,extension]					  n_elements=extension+1=(size-1)/2+1
//
//		  Questo vettore deve essere distrutto dal chiamante con delete[]
//
//
//		- La tolleranza si riferisce a quella non normalizzata (o in percentuale a 1)
//
template<class T>
T *GaussianFilter(T sigma,T tollerance,bool full_filter,bool normalize,int &size,int &n_elements);				// Filtro gaussiano [normalizzato]








// abstract_data_interface
class abstract_data_interface {
public:
	abstract_data_interface() {};
	virtual ~abstract_data_interface() {};

	virtual double get_data(int index) = 0;
	virtual void set_data(int index,double val) = 0;
	virtual void inform_statistics(int index,double mean,double power,double var,double std) = 0;
};

//
// Computa le statistiche (mean,power,var,std) usando una sliding window di dimensioni WINDOW_SIZE
// 
//    Complessita'   (2*num_elements)  get_data
//					   (num_elements)  inform_statistics
//
void WindowStat(int num_elements,abstract_data_interface *data,int WINDOW_SIZE);




