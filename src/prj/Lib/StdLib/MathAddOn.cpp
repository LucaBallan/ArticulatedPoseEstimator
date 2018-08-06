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
#include <time.h>
#include <limits>
using namespace std;


//
// PSO_Solver
//
PSO_Solver::PSO_Solver(int n_dof,int n_particles) {
	this->n_dof=n_dof;
	this->n_particles=n_particles;
	this->particles=NULL;
	this->particles_speed=NULL;
	this->local_optimal=NULL;
	this->local_optimal_cost=NULL;
	this->global_optimal=NULL;
	this->max_x=NULL;
	this->min_x=NULL;

	SNEWA_P(particles,double *,n_particles);
	for(int i=0;i<n_particles;i++) SNEWA(particles[i],double,n_dof);
	SNEWA_P(particles_speed,double *,n_particles);
	for(int i=0;i<n_particles;i++) SNEWA(particles_speed[i],double,n_dof);
	SNEWA_P(local_optimal,double *,n_particles);
	for(int i=0;i<n_particles;i++) SNEWA(local_optimal[i],double,n_dof);
	SNEWA(local_optimal_cost,double,n_particles);
	SNEWA(global_optimal,double,n_dof);
	SNEWA_I(max_x,double,n_dof,(DBL_MAX));
	SNEWA_I(min_x,double,n_dof,(-DBL_MAX));
	global_optimal_cost=DBL_MAX;
}

PSO_Solver::~PSO_Solver() {
	SDELETEA_RECA(particles,n_particles);
	SDELETEA_RECA(particles_speed,n_particles);
	SDELETEA_RECA(local_optimal,n_particles);
	SDELETEA(local_optimal_cost);
	SDELETEA(global_optimal);
	SDELETEA(max_x);
	SDELETEA(min_x);
}

void PSO_Solver::Initialize(double *particle,int p_index) {
	for(int j=0;j<n_dof;j++) particle[j]=Uniform_double_rand(min_x[j],max_x[j]);
}

double PSO_Solver::Solve(double *initial_x,int n_iterations,bool verbose) {
	
	// Init
	for(int j=0;j<n_dof;j++) particles[0][j]=initial_x[j];
	for(int j=0;j<n_dof;j++) particles_speed[0][j]=0.0;
	local_optimal_cost[0]=DBL_MAX;
	for(int i=1;i<n_particles;i++) {
		Initialize(particles[i],i);
		for(int j=0;j<n_dof;j++) particles_speed[i][j]=0.0;
		local_optimal_cost[i]=DBL_MAX;
	}
	global_optimal_cost=DBL_MAX;
	
	// Minimization
	for(int i=0;i<n_iterations;i++) {
		// Evaluate
		for(int i=0;i<n_particles;i++) {
			double v=Evaluate(particles[i],false);
			if (v<local_optimal_cost[i]) {
				local_optimal_cost[i]=v;
				for(int j=0;j<n_dof;j++) local_optimal[i][j]=particles[i][j];
			}
			if (v<global_optimal_cost) {
				global_optimal_cost=v;
				for(int j=0;j<n_dof;j++) global_optimal[j]=particles[i][j];
			}
		}

		// Compute speed
		double c1=2.8;
		double c2=1.3;
		double K=2.0/fabs(2.0-(c1+c2)-sqrt(((c1+c2)*(c1+c2))-4*(c1+c2)));
		for(int i=0;i<n_particles;i++) {
			for(int j=0;j<n_dof;j++) {
				double r1=Uniform_double_rand(0.0,1.0);
				double r2=Uniform_double_rand(0.0,1.0);
				particles_speed[i][j]=K*( particles_speed[i][j] + c1*r1*( local_optimal[i][j] - particles[i][j] ) + c2*r2*( global_optimal[j] - particles[i][j] ) );
			
				// TODO: if particles==global_optimal==local_optimal -> allora questo nn si muove -> continuo a testarlo
				// basterebbe controllare questa condizione e aggiungere velocita' a caso o copiarla da qualcuno a caso
			}
		}

		// Update
		for(int i=0;i<n_particles;i++) {
			for(int j=0;j<n_dof;j++) {
				double Max=max_x[j];
				double Min=min_x[j];

				double new_state=(particles[i][j]+particles_speed[i][j]);
				if (new_state<=Min) {
					new_state=Min;
					particles_speed[i][j]=0.0;
				}
				if (new_state>=Max) {
					new_state=Max;
					particles_speed[i][j]=0.0;
				}
				particles[i][j]=new_state;
			}
		}

		if (verbose) cout<<"Cost: "<<global_optimal_cost<<"\n";
	}

	return global_optimal_cost;
}



//
// Array operations
//
int find_max(const double *data,int n,int step) {
	if (n<=0) return -1;
	double max=data[0];
	int index=0;
	for(int i=1;i<n;i++) {
		if (max<data[i*step]) {
			max=data[i*step];
			index=i;
		}
	}
	return index;
}

int find_min(const double *data,int n,int step) {
	if (n<=0) return -1;
	double min=data[0];
	int index=0;
	for(int i=1;i<n;i++) {
		if (min>data[i*step]) {
			min=data[i*step];
			index=i;
		}
	}
	return index;
}

int find_max(const float *data,int n,int step) {
	if (n<=0) return -1;
	float max=data[0];
	int index=0;
	for(int i=1;i<n;i++) {
		if (max<data[i*step]) {
			max=data[i*step];
			index=i;
		}
	}
	return index;
}

int find_min(const float *data,int n,int step) {
	if (n<=0) return -1;
	float min=data[0];
	int index=0;
	for(int i=1;i<n;i++) {
		if (min>data[i*step]) {
			min=data[i*step];
			index=i;
		}
	}
	return index;
}

void mean_std(const double *data,int n,double &mean,double &std,int step) {
	double avg=0.0,pwd=0.0;	
	
	for(int q=0;q<n;q++,data+=step) {
		avg+=(*data);
		pwd+=(*data)*(*data);
	}

	avg=avg/n;
	pwd=pwd/n;
	
	mean=avg;
	std=sqrt(pwd-(avg*avg));
}


mean_std_data::mean_std_data() {
	array_data=NULL;
	clear();
}

void mean_std_data::clear() {
	sum=0;
	sum_of_square=0;
	num_ele=0;
	max=-DBL_MAX;
	min=DBL_MAX;
	SDELETE(array_data);
}

void mean_std_i(double data,mean_std_data *info) {
	info->sum+=data;
	info->sum_of_square+=data*data;
	info->num_ele++;
	if (info->max<data) info->max=data;
	if (info->min>data) info->min=data;
}

void mean_median_std_i(double data,mean_std_data *info) {
	mean_std_i(data,info);
	SNEW(info->array_data,Array<double>(50));
	static_cast<Array<double>*>(info->array_data)->append(data);
}

void mean_std_r(mean_std_data *info,double &mean,double &std) {
	if (info->num_ele==0) {
		mean=0.0;
		std=0.0;
		return;
	}

	mean=info->sum/info->num_ele;
	std=info->sum_of_square/info->num_ele;
	std=sqrt(std-(mean*mean));
}

void mean_std_r(mean_std_data *info,double &mean,double &std,UINT &num_ele) {
	mean_std_r(info,mean,std);
	num_ele=info->num_ele;
}

int compare_double(const double *a,const double *b) {
	if ((*a)<(*b)) return 1;
	if ((*a)==(*b)) return 0;
	return -1;
}
void mean_median_std_r(mean_std_data *info,double &mean,double &median,double &std,UINT &num_ele) {
	mean_std_r(info,mean,std,num_ele);
	if (num_ele==0) {
		median=0.0;
	} else {
		Array<double>*data=static_cast<Array<double>*>(info->array_data);
		data->sort((int (*)(const void *a,const void *b))compare_double);
		if (num_ele%2) {
			median=(*(data))[num_ele/2];
		} else {
			median=((*(data))[num_ele/2]+(*(data))[(num_ele/2)+1])/2;
		}
	}
}



int *find_i_max(double *Data,int NumData,int N) {

	int *NMax=new int[N];
	if (NMax==NULL) {printf ("Error: Out of memory.\n");exit(1);}

	int last=0;
	NMax[0]=0;

	for(int i=1;i<NumData;i++) {
		if (last<N-1) {
			//OrderInsert(NMax,last,Data,N);
			int j;
			for(j=last;j>=0;j--) {
				if (Data[i]<Data[NMax[j]]) break;
			}
			j++;
			for(int h=last;h>=j;h--) {
				NMax[h+1]=NMax[h];
			}
			NMax[j]=i;
			last++;
			continue;
		}
		if (Data[i]>Data[NMax[last]]) {
			//OrderInsert&Delete(NMax,last,Data,N);
			int j;
			for(j=last;j>=0;j--) {
				if (Data[i]<Data[NMax[j]]) break;
			}
			j++;
			for(int h=last-1;h>=j;h--) {
				NMax[h+1]=NMax[h];
			}
			NMax[j]=i;
		} 
	}
	
	return NMax;
}

void normalize(double *Vect,int Size) {
	
	double Tmp=0;
	
	for(int i=0;i<Size;i++) Tmp+=Vect[i]*Vect[i];

	if (Tmp==0) return;
	Tmp=sqrt(Tmp);

	for(int i=0;i<Size;i++) Vect[i]/=Tmp;
}













//
// Math functions
//

int Approx(double x) {
	if (x<0.0) {
		int val=(int)x;
		if ((x+(double)(-val))<-0.5) return val-1;
		return val;
	}

	int val=(int)x;

	if ((x-(double)val)>0.5) return val+1;
	return val;
}

int Approx(float x) {
	if (x<0.0f) {
		int val=(int)x;
		if ((x+(float)(-val))<-0.5f) return val-1;
		return val;
	}

	int val=(int)x;

	if ((x-(float)val)>0.5f) return val+1;
	return val;
}

double realAcos(double x) {
	if (x>1.0) x=1.0;
	if (x<-1.0) x=-1.0;
	return acos(x);
}

int sign(double x) {
	if (x>=0) return +1;
	return -1;
}

double Module(double a,double module) {
	if (a<0) {
		double tmp=-a;
		double n=floor(tmp/module);
		a=-(tmp-(n*module));
		a=module+a;
	}
	double n=floor(a/module);
	return (a-(n*module));
}

int Module(int a,int module) {
	if (a<0) {
		int tmp=-a;
		int n=tmp/module;
		a=-(tmp-(n*module));
		a=module+a;
	}
	int n=a/module;
	return (a-(n*module));
}

int	ModuleDistance(int a,int b,int module) {
	a=Module(a,module);
	b=Module(b,module);

	int d1=abs(a-b)-1;
	int d2=(((module-1)-a)+b);
	int d3=(((module-1)-b)+a);

	return (min(d1,min(d2,d3)));
}

double ModuleDistance(double a,double b,double module) {
	a=Module(a,module);
	b=Module(b,module);

	double d1=fabs(a-b);
	double d2=((module-a)+b);
	double d3=((module-b)+a);

	return (min(d1,min(d2,d3)));
}

int ModuleSign(double a,double b,double module) {
	a=Module(a,module);
	b=Module(b,module);

	double d1=fabs(a-b);
	int    s1=sign(a-b);
	double d2=((module-a)+b);
	int    s2=-sign((module-a)+b);
	double d3=((module-b)+a);
	int    s3=sign((module-b)+a);

	double min=(min(d1,min(d2,d3)));
	if (min==0.0) return 0;
	if (d1==min)  return s1;
	if (d2==min)  return s2;
	return s3;
}
double PercentageRatio(double x,double total) {
	if (total==0) return numeric_limits<double>::infinity();
	return ((100.0*x)/total);
}

int factorial(int n) {
	int val=1;
	for(int i=n;i>1;--i) val*=i;
	return val;
}
















//
// Random variables
//

double Uniform_double_rand(double min,double max) {
	
	double ext_ratio=(max-min)/(double)RAND_MAX;
	
	return ((rand()*ext_ratio)+min);
}

int Uniform_int_rand(int min,int max) {
	int var;

	do {
		var=Approx(Uniform_double_rand(min,max));
	} while ((var<min) && (var>max));

	return var;
}
double Normal_double_rand(double mean,double sigma) {
	static double n2=0.0;
	static bool cached=false;

	if (!cached) {
		double x,y,r;
		do {
			x=(rand()*2.0/(double)RAND_MAX)-1.0;
			y=(rand()*2.0/(double)RAND_MAX)-1.0;
			r=x*x+y*y;
		} while ((r<=(10.0*FLT_EPSILON))||(r>=1.0));

		double d=sqrt(-2.0*log(r)/r);
		double n1=x*d;
		n2 = y*d;
		cached=true;
		return (n1*sigma)+mean;
	}

	cached=false;
	return (n2*sigma)+mean;
}



















//
// Gaussian Filters Design
//

int gaussian_min_width_from_sigma(double sigma,double tollerance) {
	double semi_width=(sigma*sqrt(-2.0*log(tollerance)));
	int semi_width_i=max(((int)ceil(semi_width)),1);
	return (2*semi_width_i)+1;
}

double gaussian_max_sigma_from_width(int width,double tollerance) {
	int semi_width=(width-1)/2;
	return sqrt(-(semi_width*semi_width)/(2*log(tollerance)));
}








//
// Truncated_Quadratic
//

Truncated_Quadratic::Truncated_Quadratic() {
	a=0.0;
	b=0.0;
	c=0.0;
	k=1.0;
	k_sqrt=1.0;
	k_ratio=1.0;
	low_th=0.0;
	high_th=0.0;
}
void Truncated_Quadratic::initialize(double low_th,double high_th) {
	this->low_th=low_th;
	this->high_th=high_th;
	
	double a_b=low_th-high_th;
	k=1.0/(low_th*high_th);
	a=k*low_th/a_b;
	b=-k*(2*low_th*high_th)/a_b;
	c=k*(low_th*low_th*high_th)/a_b;
	k_sqrt=sqrt(k);
	k_ratio=k/k_sqrt;
}
void Truncated_Quadratic::Graph(double a,double b,int num) {
	for(int i=0;i<num;i++) {
		cout<<((((b-a)/(num-1)))*i+a)<<" "<<F((((b-a)/(num-1)))*i+a)<<" "<<DF((((b-a)/(num-1)))*i+a)<<endl;
	}
}























//
// Gaussian Filters TODO***
//

template<class T>
T *GaussianFilter(T sigma,T tollerance,bool full_filter,bool normalize,int &size,int &n_elements) {


	int    win_size=(int)ceil(sqrt(-((T)2.0)*sigma*sigma*log(tollerance)));
	T	   ksigma  =-((T)(1.0)/((T)(2.0)*sigma*sigma));
		   size    =(2*win_size+1);

	T      *Filter,*p;
	int	   stard_index;
	T      factor=(T)(0.0);
	
	if (win_size<1) {
		win_size=0;
		size=1;
		Filter=new T[1];
		n_elements=1;
		Filter[0]=(T)1.0;
		return Filter;
	}

	if (full_filter) {
		Filter=new T[size];
		stard_index=-win_size;
		n_elements=size;
	} else {
		Filter=new T[win_size+1];
		stard_index=0;
		n_elements=win_size+1;
	}

	p=Filter;
	for(int i=stard_index;i<=win_size;i++,p++) {
		(*p)=(T)exp(ksigma*i*i);
		factor+=(*p);
	}
	if (normalize) {
		if (!full_filter) factor=((T)2.0)*(factor-Filter[0])+Filter[0];
		factor=(T)(1.0)/factor;
		p=Filter;
		for(int i=stard_index;i<=win_size;i++,p++) (*p)*=factor;
	}
	
	return Filter;
}


template float *GaussianFilter(float sigma,float tollerance,bool full_filter,bool normalize,int &size,int &n_elements);
template double *GaussianFilter(double sigma,double tollerance,bool full_filter,bool normalize,int &size,int &n_elements);





SpeedTester::SpeedTester() {
	first=true;
	LastSpeed=SPEED_UNKNOW;
}

double SpeedTester::CheckPoint(UINT Pos) {
	time_t now;

	if (first) {
		LastPosRefresh=Pos;
		time(&LastRefresh);
		first=false;
		return SPEED_UNKNOW;
	}

	time(&now);
	if (Pos==LastPosRefresh) return LastSpeed;
	if (LastRefresh==now) return LastSpeed;
	
	LastSpeed=(Pos-LastPosRefresh)/difftime(now,LastRefresh);

	LastRefresh=now;
	LastPosRefresh=Pos;

	return LastSpeed;
}

int SpeedTester::RemainTime(UINT EndPos) {
	if (LastSpeed==SPEED_UNKNOW) return -1;
	if (LastSpeed==0) return -1;

	return (int)((EndPos-LastPosRefresh)/LastSpeed);
}

void SpeedTester::RemainTime(char *Out,UINT EndPos) {
	int sec=RemainTime(EndPos);

	if (sec==-1) {
		sprintf(Out,"??");
		return;
	}
	
	PrintTime(Out,sec);
}


Positional_Number::Positional_Number(unsigned int Len,unsigned int Base) {
	this->Len=Len;
	this->Base=Base;
	RoundUp=false;
	Val=new unsigned int[Len];
	SetZero();
}

Positional_Number::~Positional_Number() {
	delete []Val;
}

Positional_Number *Positional_Number::operator++() {
	bool Carry;
	unsigned int i=0;

	do {
		Carry=false;
		if (i==Len) {
			RoundUp=true;
			break;
		}

		Val[i]++;
		if (Val[i]>=Base) {
			Val[i]=0;
			Carry=true;
		}
		i++;
	
	} while(Carry);
	
	return this;
}

void Positional_Number::SetZero() {
	RoundUp=false;
	for(unsigned int i=0;i<Len;i++) Val[i]=0;
}

bool Positional_Number::HasAllDifferentElements() {
	for(unsigned int i=0;i<Len;i++) {
		for(unsigned int j=i+1;j<Len;j++) {
			if (Val[i]==Val[j]) return false;
		}
	}
	return true;
}
bool Positional_Number::IsInside(int x) {
	for(unsigned int i=0;i<Len;i++) {
		if (Val[i]==x) return true;
	}
	return false;
}

Profiler::Profiler(char *Name) {
	strcpy(this->Name,Name);
	counting=true;
	start=clock();
}
Profiler::Profiler() {
	this->Name[0]=0;
	counting=true;
	start=clock();
}
Profiler::~Profiler() {
	if (counting) Stop();
}
double Profiler::Stop() {
	clock_t end=clock();
	counting=false;
	double sec=((end-start)*1.0/CLOCKS_PER_SEC);
	if (strlen(Name)!=0) cout<<"Fnc "<<Name<<"     "<<sec<<" sec.\n";
	return sec;
}



void WindowStat(int num_elements,abstract_data_interface *data,int WINDOW_SIZE) {
	double start_sum=0;
	double end_sum=0;
	double start_s_sum=0;
	double end_s_sum=0;

	for(int i=0;i<WINDOW_SIZE;i++) {
		double x=data->get_data(i);
		if (i<num_elements) {
			end_sum+=x;
			end_s_sum+=x*x;
		}
	}

	for(int i=0;i<num_elements;i++) {
		// i-WINDOW_SIZE, ..., i, ..., i+WINDOW_SIZE
		//start_s_sum				// sum(...,i-WINDOW_SIZE-1)
		//end_s_sum					// sum(...,i+WINDOW_SIZE)
		
		if (i-WINDOW_SIZE-1>=0) {
			double x=data->get_data(i-WINDOW_SIZE-1);
			start_sum+=x;
			start_s_sum+=x*x;
		}
		if (i+WINDOW_SIZE<num_elements) {
			double x=data->get_data(i+WINDOW_SIZE);
			end_sum+=x;
			end_s_sum+=x*x;
		} 

		int num_ele=min(i,WINDOW_SIZE)+min(num_elements-i-1,WINDOW_SIZE)+1;


		double power=(end_s_sum-start_s_sum)/num_ele;
		double mean=(end_sum-start_sum)/num_ele;
		double var=((end_s_sum-start_s_sum)-(num_ele*mean*mean))/(num_ele-1);
		double std=sqrt(var);
		
		data->inform_statistics(i,mean,power,var,std);
	}
}

void smooth_filter(int num_elements,SMOOTH_FILTER_FUNCTION_GET_TYPE get_data,SMOOTH_FILTER_FUNCTION_SET_TYPE set_data,void *userdata,bool Test,double sigma,double tollerance) {
	
	int    win_size=(int)ceil(sqrt(-2.0*sigma*log(tollerance)));
	double ksigma  =-(1.0/(2.0*sigma*sigma));

	if (Test) {
		printf("-> Filtering:\n");
		printf("     Window Size: %i\n",((2*win_size)+1));
		printf("     Sigma: %f\n",sigma);
	}

	// Build the Filter
	double *Filter=new double[(2*win_size+1)];
	double factor=0;
	for(int i=-win_size;i<=win_size;i++) {
		Filter[i+win_size]=exp(ksigma*i*i);
		factor+=Filter[i+win_size];
	}
	factor=1.0/factor;
	for(int i=-win_size;i<=win_size;i++) Filter[i+win_size]=factor*Filter[i+win_size];

	double *data_buffer=new double[num_elements];
	bool *data_available=new bool[num_elements];
	for(int i=0;i<num_elements;i++) data_available[i]=false;
	
	for(int i=win_size;i<num_elements-win_size;i++) {
		double val=0.0;
		for(int j=-win_size;j<=win_size;j++) {
			if (!data_available[i+j]) {
				data_buffer[i+j]=get_data(i+j,userdata);
				data_available[i+j]=true;
			}
			val+=(Filter[j+win_size]*data_buffer[i+j]);
		}
		set_data(i,val,userdata);		// Lo leggo sempre prima di settarlo!!! (non ci sono problemi di mediazione della media)
		if (Test) printf("%f, ",val);
	}

	if ((2*win_size+1)>num_elements) ErrorExit("Too few elements to smooth");

	for(int i=0;i<win_size;i++) {
		double factor=0;
		double val=0.0;
		for(int j=-i;j<=win_size;j++) {
			if (!data_available[i+j]) {
				data_buffer[i+j]=get_data(i+j,userdata);
				data_available[i+j]=true;
			}
			val+=(Filter[j+win_size]*data_buffer[i+j]);
			factor+=Filter[j+win_size];
		}
		val=val/factor;
		set_data(i,val,userdata);
	}

	for(int i=num_elements-win_size;i<num_elements;i++) {
		double factor=0;
		double val=0.0;
		for(int j=-win_size;j<=(num_elements-1)-i;j++) {
			if (!data_available[i+j]) {
				data_buffer[i+j]=get_data(i+j,userdata);
				data_available[i+j]=true;
			}
			val+=(Filter[j+win_size]*data_buffer[i+j]);
			factor+=Filter[j+win_size];
		}
		val=val/factor;
		set_data(i,val,userdata);
	}

	delete []Filter;
	delete []data_buffer;
	delete []data_available;
}



double get_timer_frequency() {
	LARGE_INTEGER freq;
	if (!QueryPerformanceFrequency(&freq)) ErrorExit("Can't find an high-resolution timer.");
	return (static_cast<double>(freq.QuadPart));
}

void CreateGaussianProbability(float_type *g_p,float_type *ng_p,double first_sigma,double last_sigma,int nof,int nos) {
	
	double *Sigma=new double[nos];
	float_type *g_pp,*ng_pp;


	double sigma_step=(last_sigma-first_sigma)/(nos-1.0);
	for(int i=0;i<nos;i++) Sigma[i]=first_sigma+sigma_step*i;


	for(int i=0;i<nos;i++) {
		if (Sigma[i]==0.0) ErrorExit("Sigma cannot be zero.");

		double k=(-1.0/(2.0*Sigma[i]*Sigma[i]));
		double norm_g=0.0;
		double norm_ng=0.0;

		g_pp=g_p;
		ng_pp=ng_p;
		for(int j=0;j<nof;j++,g_pp++,ng_pp++) {
			(*g_pp)=(float_type)exp(k*j*j);
			(*ng_pp)=((float_type)1.0)-(*g_pp);
			norm_g+=(*g_pp);
			norm_ng+=(*ng_pp);
		}

		g_pp=g_p;
		ng_pp=ng_p;
		for(int j=(nof-1);j>=0;j--,g_pp++,ng_pp++) {
			(*g_pp)=(*g_pp)/((float_type)norm_g);
			(*ng_pp)=(*ng_pp)/((float_type)norm_ng);
		}

		g_p=g_pp;
		ng_p=ng_pp;
	}

	delete []Sigma;
}












//
// CasualFilter
//
//
template <class data_type>
CasualFilter<data_type>::CasualFilter(int size) : CircularArray<data_type>(size+1) {
}

template <class data_type>
CasualFilter<data_type>::~CasualFilter() {
}

template <class data_type>
data_type CasualFilter<data_type>::filter(data_type x,data_type *filter) {
	if (isFull()) {
		pop();
		push(x);
	} else {
		push(x);
		if (!isFull()) return x;
	}
	// Buffer full -> Filter
	
	data_type s=(data_type)0.0;
	for(int k=0;k<(num-1);k++) {
		s+=mem[(o+k)%num]*filter[num-2-k];
	}

	return s;
}


template CasualFilter<float>;
template CasualFilter<double>;

