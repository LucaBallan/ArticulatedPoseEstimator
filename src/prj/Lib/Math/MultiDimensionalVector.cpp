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


template class Tensor<double>;
template class Tensor<float>;
template class Tensor<WI_Type>;
template void Multiply(Tensor<double> *A,Tensor<double> *B,Tensor<double> *C);
template void Multiply(Tensor<double> *A,Tensor<double> *B,Tensor<double> *C,int q,int shift);
template void PointMultiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C,int shift);
template void TensorMultiply(Tensor<double> *A,Tensor<double> *B,Tensor<double> *C);


#define CREATEPOS(pos,n)       int *(pos)=new int[(n)];
#define DELETEPOS(pos)         delete [](pos);
#define READPOS(pos,first_par) {pos[0]=first_par;va_list marker;va_start(marker,first_par);for(int i=1;i<n;i++) pos[i]=va_arg(marker,const int);va_end(marker);}





TensorIndex::TensorIndex(int order) {
	if (order<1) ErrorExit("Cannot create a tensor index with order less than 1.");
	n=order;
	i=new int [n+1];
	i[0]=-1;
}
TensorIndex::~TensorIndex() {
	delete[] i;
}
void TensorIndex::clear() {
	i[0]=-1;
}
bool TensorIndex::isValid(int order,int *sizes) {
	if (order!=n) return false;
	for(int s=0;s<n;s++) {
		if ((i[s]<0) || (i[s]>=sizes[s])) return false;
	}
	return true;
}
void TensorIndex::set(int *pos) {
	for(int s=0;s<n;s++) i[s]=pos[s];
}
bool TensorIndex::compare(TensorIndex *b) {
	if (b->n!=n) return false;
	for(int s=0;s<n;s++) {
		if (i[s]!=b->i[s]) return false;
	}
	return true;
}
bool TensorIndex::is_symmetric() {
	if (n%2!=0) return false;
	for(int s=0;s<(n/2);s++) {
		if (i[s]!=i[n-1-s]) return false;
	}
	return true;
}
void TensorIndex::cat(int h) {
	int s;
	for(s=0;s<n;s++) 
		if (i[s]==-1) break;
	if (s==n) ErrorExit("Tensor index overflow.");
	i[s]=h;
	i[s+1]=-1;
}
void TensorIndex::cat(TensorIndex *b,int start) {
	if (start<0) ErrorExit("Tensor index overflow.");
	if (b->n-1<start) return;

	int s;
	for(s=0;s<n;s++) 
		if (i[s]==-1) break;
	if (s==n) ErrorExit("Tensor index overflow.");
	int o=0;
	for(;s<n;s++,o++) {
		if (o+start>=b->n) break;
		i[s]=b->i[o+start];
	}
	if (o+start<b->n) ErrorExit("Tensor index overflow.");
	i[s]=-1;
}
void TensorIndex::cat(TensorIndex *b,int start,int end) {
	if (start<0)   ErrorExit("Tensor index overflow.");
	if (end>=b->n) ErrorExit("Tensor index overflow.");
	if (end<start) return;

	int s;
	for(s=0;s<n;s++) 
		if (i[s]==-1) break;
	if (s==n) ErrorExit("Tensor index overflow.");
	int o=0;
	for(;s<n;s++,o++) {
		if (o+start>end) break;
		if (o+start>=b->n) ErrorExit("Tensor index overflow.");
		i[s]=b->i[o+start];
	}
	if (o+start<=end) ErrorExit("Tensor index overflow.");
	i[s]=-1;
}
void TensorIndex::print() {
	cout<<"(";
	for(int s=0;s<n-1;s++) cout<<i[s]<<",";
	cout<<i[n-1]<<")";
}



///////////////////////////////////////////////////////
//  Constructurs
///////////////////////////////////////////////////////
template <class Element_Type>
Tensor<Element_Type>::Tensor(int n,const int size0,...) {
	if (n<=0) ErrorExit("Cannot create a zero-dimensional array.");  // MULTI_TODO
	
	this->n=n;
	this->size=1;
	this->dim=NULL;
	this->value=NULL;
	
	SNEWA(this->dim,int,this->n);
	this->dim[0]=size0;
	if (this->dim[0]<=0) ErrorExit("Cannot create an array with a size less than one.");
	this->size=this->dim[0];

	va_list marker;
	va_start(marker,size0);
	for(int i=1;i<(this->n);i++) {
		this->dim[i]=va_arg(marker,const int);
		if (this->dim[i]<=0) ErrorExit("Cannot create an array with a size less than one.");
		this->size*=this->dim[i];
	}
	va_end(marker);

	SNEWA(this->value,Element_Type,this->size);
}

template <class Element_Type>
Tensor<Element_Type>::Tensor(int n,const int *size) {
	if (n<=0) ErrorExit("Cannot create a zero-dimensional array.");  // MULTI_TODO
	
	this->n=n;
	this->size=1;
	this->dim=NULL;
	this->value=NULL;

	
	SNEWA(this->dim,int,this->n);
	for(int i=0;i<(this->n);i++) {
		this->dim[i]=size[i];
		if (this->dim[i]<=0) ErrorExit("Cannot create an array with a size less than one.");
		this->size*=this->dim[i];
	}

	SNEWA(this->value,Element_Type,this->size);
}

template <class Element_Type>
Tensor<Element_Type>::Tensor(Bitmap<Element_Type> *img) {
	
	this->n=2;
	this->size=1;
	this->dim=NULL;
	this->value=NULL;

	
	SNEWA(this->dim,int,this->n);
	this->dim[0]=img->width;
	this->dim[1]=img->height;
	this->size=this->dim[0]*this->dim[1];

	SNEWA(this->value,Element_Type,this->size);
	for(int i=0;i<(this->size);i++) value[i]=img->Point((int)(i%img->width),(int)(i/img->width));
}

template <class Element_Type>
Tensor<Element_Type>::Tensor(const Tensor<Element_Type> &b) {
	this->n=b.n;
	this->size=1;
	this->dim=NULL;
	this->value=NULL;

	SNEWA(this->dim,int,this->n);
	for(int i=0;i<(this->n);i++) {
		this->dim[i]=b.dim[i];
		this->size*=this->dim[i];
	}
	
	SNEWA(this->value,Element_Type,this->size);
	for(int i=0;i<(this->size);i++) value[i]=b.value[i];
}

template <class Element_Type>
Tensor<Element_Type>::~Tensor() {
	SDELETEA(dim);
	SDELETEA(value);
}












///////////////////////////////////////////////////////
//  Coordinates
///////////////////////////////////////////////////////
template <class Element_Type>
void Tensor<Element_Type>::index2coords(const int index,int *pos) const {
	int step=size;
	int v_index=index;
	if ((index<0) || (index>=size)) ErrorExit("Index out of bounds.");

	for (int i=0;i<n;i++) {
		step/=dim[n-1-i];
		pos[n-1-i]=v_index/step;
		v_index-=pos[n-1-i]*step;
	}
}
void get_coords(int n,int *dim,int size,const int index,int *pos) {
	int step=size;
	int v_index=index;
	if ((index<0) || (index>=size)) ErrorExit("Index out of bounds.");

	for (int i=0;i<n;i++) {
		step/=dim[n-1-i];
		pos[n-1-i]=v_index/step;
		v_index-=pos[n-1-i]*step;
	}
}
template <class Element_Type>
int Tensor<Element_Type>::coords2index(const int *pos) const {
	// x(0) + x(1)*dim[0] + x(2)*dim[0]*dim[1] + x(3)*dim[0]*dim[1]*dim[2] + ...
	
	for (int i=0;i<n;i++) 
		if ((pos[i]<0) || (pos[i]>=dim[i])) ErrorExit("Index out of bounds.");

	int index=0;
	for (int i=n-1;i>=1;i--) {
		index+=pos[i];
		index*=dim[i-1];
	}
	index+=pos[0];

	return index;
}

template <class Element_Type>
int Tensor<Element_Type>::getstep(int dim_index) const {
	int step=1;
	for(int i=0;i<dim_index;i++) step*=dim[i];
	return step;
}








///////////////////////////////////////////////////////
//  Components access
///////////////////////////////////////////////////////
template <class Element_Type>
Element_Type &Tensor<Element_Type>::vl(const int i) {
	return value[i];
}
template <class Element_Type>
const Element_Type &Tensor<Element_Type>::vl(const int i) const {
	return value[i];
}

template <class Element_Type>
Element_Type &Tensor<Element_Type>::vv(const int *pos) {
	return value[coords2index(pos)];
}
template <class Element_Type>
const Element_Type &Tensor<Element_Type>::vv(const int *pos) const {
	return value[coords2index(pos)];
}
//
//  n = num_dimensions
//  Dim[i]  i=0,...,n-1
//  
//	Coords(x(0),x(1),...,x(n-1)) = x(0) + x(1)*dim[0] + x(2)*dim[0]*dim[1] + x(3)*dim[0]*dim[1]*dim[2] + ...
// 
template <class Element_Type>
Element_Type &Tensor<Element_Type>::v(const int i_0,...) {
	int step=1;
	int location=step*i_0;
	if ((i_0<0) || (i_0>=dim[0])) ErrorExit("Index out of bounds.");

	va_list marker;
	va_start(marker,i_0);
	for(int j=1;j<n;j++) {
		step*=dim[j-1];

		int i_j=va_arg(marker,const int);
		if ((i_j<0) || (i_j>=dim[j])) ErrorExit("Index out of bounds.");
		location+=(i_j*step);
	}
	va_end(marker);
	
	return value[location];
}
template <class Element_Type>
const Element_Type &Tensor<Element_Type>::v(const int i_0,...) const {
	int step=1;
	int location=step*i_0;
	if ((i_0<0) || (i_0>=dim[0])) ErrorExit("Index out of bounds.");

	va_list marker;
	va_start(marker,i_0);
	for(int j=1;j<n;j++) {
		step*=dim[j-1];

		int i_j=va_arg(marker,const int);
		if ((i_j<0) || (i_j>=dim[j])) ErrorExit("Index out of bounds.");
		location+=(i_j*step);
	}
	va_end(marker);
	
	return value[location];
}
















///////////////////////////////////////////////////////
//  Info
///////////////////////////////////////////////////////
template <class Element_Type>
void Tensor<Element_Type>::PrintLin(int st_order,int *ref_pos,char *format) {
	char tmp[30];
	int *pos=new int[n];
	cout<<"(";
	for(int j=0;j<n;j++) pos[j]=ref_pos[j];
	for(int i=0;i<dim[st_order];i++) {
		pos[st_order]=i;
		if (st_order-1>=0) PrintLin(st_order-1,pos,format);
		else {
			sprintf(tmp,format,vv(pos));
			cout<<tmp;
		}
		if (i!=dim[st_order]-1) cout<<", ";
	}
	cout<<")";
	delete[]pos;
}
template <class Element_Type>
void Tensor<Element_Type>::PrintLin(char *tensor_name,char *format) {
	int *pos=new int[n];
	for(int j=0;j<n;j++) pos[j]=0;
	if (tensor_name) cout<<tensor_name<<" = ";	
	PrintLin(n-1,pos,format);
	delete[]pos;
}

void print_par(int row,int st,int ed,bool open) {
	if (open) {
		if (row==st) cout<<(char)(218);
		else if (row==ed) cout<<(char)(192);
		else if ((row>st) && (row<ed)) cout<<(char)(179);
		else cout<<" ";
	} else {
		if (row==st) cout<<(char)(191);
		else if (row==ed) cout<<(char)(217);
		else if ((row>st) && (row<ed)) cout<<(char)(179);
		else cout<<" ";
	}
}
template <class Element_Type>
void Tensor<Element_Type>::Print2D_Mat(char *format,int st_order,int *pos,int row,int offset,int screen_size) {
	char tmp[30];

	print_par(row,n-st_order,screen_size-(n-st_order)-1,true);
	if (n>1) {
		for(int c=0;c<dim[1];c++) {
			pos[1]=c;
			
			// row allignment
			size_t max_len=0;
			for(int r=0;r<dim[0];r++) {
				pos[0]=r;
				sprintf(tmp,format,vv(pos));
				max_len=max(strlen(tmp),max_len);
			}
			pos[0]=row-(n-1);
			if ((pos[0]>=0) && (pos[0]<dim[0])) {
				sprintf(tmp,format,vv(pos));
				for(size_t p=0;p<(max_len-strlen(tmp));p++) cout<<" ";
				cout<<tmp;
			} else {
				for(size_t p=0;p<max_len;p++) cout<<" ";
			}

			if (c!=dim[1]-1) cout<<" ";
		}
	} else {
		// row allignment
		size_t max_len=0;
		for(int r=0;r<dim[0];r++) {
			pos[0]=r;
			sprintf(tmp,format,vv(pos));
			max_len=max(strlen(tmp),max_len);
		}
		pos[0]=row-n;
		if ((pos[0]>=0) && (pos[0]<dim[0])) {
			sprintf(tmp,format,vv(pos));
			for(size_t p=0;p<(max_len-strlen(tmp));p++) cout<<" ";
			cout<<tmp;
		} else {
			for(size_t p=0;p<max_len;p++) cout<<" ";
		}
	}
	print_par(row,n-st_order,screen_size-(n-st_order)-1,false);
}

template <class Element_Type>
void Tensor<Element_Type>::Print2D(char *format,int st_order,int *ref_pos,int row,int offset,int screen_size) {
	int *pos=new int[n];
	print_par(row,n-st_order-1,screen_size-(n-st_order),true);
	for(int j=0;j<n;j++) pos[j]=ref_pos[j];
	
	for(int i=0;i<dim[st_order];i++) {
		pos[st_order]=i;

		if (st_order>2) {
			Print2D(format,st_order-1,pos,row,offset,screen_size);
		} else {
			Print2D_Mat(format,st_order,pos,row,offset,screen_size);
		}
		
		if (i!=dim[st_order]-1) {
			if (row==offset) cout<<",";
			else cout<<" ";
		}
	}

	print_par(row,n-st_order-1,screen_size-(n-st_order),false);
	delete[]pos;
}
template <class Element_Type>
void Tensor<Element_Type>::Print(char *tensor_name,char *format,bool matrix_form) {
	if (!matrix_form) {
		PrintLin(tensor_name,format);
		return;
	}
	system("chcp 437 >NULL");
	int *pos=new int[n];
	for(int j=0;j<n;j++) pos[j]=0;
	int screen_size=dim[0]+max(2*(n-1),2);
	int offset=(int)ceil((screen_size/2.0)-0.5);
	for(int row=0;row<screen_size;row++) {
		if (tensor_name) {
			if (row==offset) cout<<tensor_name<<" = ";
			else for(size_t c=0;c<strlen(tensor_name)+3;c++) cout<<" ";
		}
		if (n>2) Print2D(format,n-1,pos,row,offset,screen_size);
		else Print2D_Mat(format,n,pos,row,offset,screen_size);

		cout<<"\n";
	}
	delete[]pos;
}

template <class Element_Type>
bool Tensor<Element_Type>::isSquare() {
	for(int i=1;i<n;i++) 
		if (dim[i-1]!=dim[i]) return false;
	
	return true;
}

template <class Element_Type>
int Tensor<Element_Type>::dimension() {
	return size;
}

template <class Element_Type>
int Tensor<Element_Type>::order() {
	return n;
}

template <class Element_Type>
int Tensor<Element_Type>::sizes(int dimension) {
	return dim[dimension];
}

template <class Element_Type>
int *Tensor<Element_Type>::sizes() {
	return dim;
}





















///////////////////////////////////////////////////////
//  Set
///////////////////////////////////////////////////////
template <class Element_Type>
void Tensor<Element_Type>::Set(Element_Type x) {
	for(int i=0;i<size;i++) value[i]=x;
}
template <class Element_Type>
void Tensor<Element_Type>::Set_to_Indices() {
	int *pos=new int[n];
	for(int i=0;i<size;i++) {
		index2coords(i,pos);
		double m=0.0;
		for(int q=0;q<n;q++) m=(m*10.0)+(pos[q]+1);
		value[i]=(Element_Type)m;
	}
	delete[]pos;
}
template <class Element_Type>
void Tensor<Element_Type>::SetIdentity() {
	if (n%2!=0) ErrorExit("Cannot create an identity array of odd order.");
	int *pos=new int[n];
	for(int i=0;i<size;i++) {
		index2coords(i,pos);
		bool valid=true;
		for(int q=0;q<(n/2);q++) valid=valid&(pos[q]==pos[n-1-q]);
		if (valid) value[i]=(Element_Type)1.0;
		else value[i]=(Element_Type)0.0;
	}
	delete[]pos;
}
template <class Element_Type>
void Tensor<Element_Type>::SetRandom() {
	for(int i=0;i<size;i++) value[i]=(((Element_Type)rand())/RAND_MAX);
}




template <class Element_Type>
void Multiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C) {
	TensorIndex  a(A->order());
	TensorIndex  b(B->order());
	TensorIndex  c(C->order());
	TensorIndex c_(C->order());

	int h_index=a.n-1;
	int h_size=(A->sizes())[h_index];
	if (h_size!=(B->sizes())[0]) ErrorExit("Cannot multiply two not compatible tensors.");
	if (c.n!=a.n+b.n-2) ErrorExit("Output tensors not compatible.");

	
	a.set(A->sizes());
	b.set(B->sizes());
	c.set(C->sizes());
	c_.cat(&a,0,a.n-2);
	c_.cat(&b,1,b.n-1);
	if (!c_.compare(&c)) ErrorExit("Output tensor not compatible.");

	int c_dim=C->dimension();
	for(int i=0;i<c_dim;i++) {
		C->index2coords(i,c.i);
		a.clear();
		b.clear();
		a.cat(&c,0,a.n-2);
		b.cat(0);
		b.cat(&c,a.n-1);
		
		Element_Type sum=0.0;
		for(int h=0;h<h_size;h++) {
			a.i[a.n-1]=h;
			b.i[0]=h;
			sum+=A->vv(a.i)*B->vv(b.i);
		}
		C->vl(i)=sum;
	}

}

template <class Element_Type>
void PointMultiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C,int shift) {
	Multiply(A,B,C,B->order(),shift);
}

template <class Element_Type>
void Multiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C,int q,int shift) {
	TensorIndex  a(A->order());
	TensorIndex  b(B->order());
	TensorIndex  c(C->order());
	TensorIndex c_(C->order());

	if (shift<0)   shift=a.n;
	if (shift>a.n) ErrorExit("q-order multiplication not possible.");
	int offset=a.n-shift;
	if (q<1) ErrorExit("q must be greater than one.");
	if (q>min(a.n-offset,b.n)) ErrorExit("q-order multiplication not possible.");
	if (c.n!=a.n+b.n-2*q) ErrorExit("Output tensors not compatible.");
	for(int i=0;i<q;i++) 
		if ((A->sizes())[a.n-1-i-offset]!=(B->sizes())[i]) ErrorExit("Cannot multiply two not compatible tensors.");
	
	
	a.set(A->sizes());
	b.set(B->sizes());
	c.set(C->sizes());
	c_.cat(&a,0,a.n-offset-1-q);
	c_.cat(&a,a.n-offset,a.n-1);
	c_.cat(&b,q,b.n-1);
	if (!c_.compare(&c)) ErrorExit("Output tensor not compatible.");

	
	TensorIndex q_size(q);
	q_size.cat(&b,0,q-1);
	Tensor<Element_Type> tmp(q,q_size.i);
	int tmp_dim=tmp.dimension();

	int c_dim=C->dimension();
	for(int i=0;i<c_dim;i++) {
		C->index2coords(i,c.i);
		a.clear();
		b.clear();
		a.cat(&c,0,a.n-offset-1-q);
		for(int k=0;k<q;k++) a.cat(0);
		a.cat(&c,a.n-offset-q,a.n-q-1);
		for(int k=0;k<q;k++) b.cat(0);
		b.cat(&c,a.n-q);
		
		Element_Type sum=0.0;
		for(int k=0;k<tmp_dim;k++) {
			tmp.index2coords(k,q_size.i);
			for(int m=0;m<q;m++) {
				a.i[a.n-1-m-offset]=q_size.i[m];
				b.i[m]=q_size.i[m];
			}
			sum+=A->vv(a.i)*B->vv(b.i);
		}
		C->vl(i)=sum;
	}

}



template <class Element_Type>
void TensorMultiply(Tensor<Element_Type> *A,Tensor<Element_Type> *B,Tensor<Element_Type> *C) {
	TensorIndex  a(A->order());
	TensorIndex  b(B->order());
	TensorIndex  c(C->order());
	TensorIndex c_(C->order());

	if (c.n!=a.n+b.n) ErrorExit("Output tensors not compatible.");
	
	a.set(A->sizes());
	b.set(B->sizes());
	c.set(C->sizes());
	c_.cat(&a);
	c_.cat(&b);
	if (!c_.compare(&c)) ErrorExit("Output tensor not compatible.");

	int c_dim=C->dimension();
	for(int i=0;i<c_dim;i++) {
		C->index2coords(i,c.i);
		a.clear();
		b.clear();
		a.cat(&c,0,a.n-1);
		b.cat(&c,a.n);
		
		C->vl(i)=A->vv(a.i)*B->vv(b.i);
	}

}













template <class Element_Type>
void Tensor<Element_Type>::Diagonal(Element_Type empty,Element_Type x0,...) {
	if (!isSquare()) ErrorExit("Cannot set the diagonal of a non square multi-dimensional vector.");
	Set(empty);
	CREATEPOS(pos,n);

	va_list marker;
	va_start(marker,x0);
	for(int i=0;i<dim[0];i++) {
		for(int j=0;j<n;j++) pos[j]=i;
		
		if (i==0) value[coords2index(pos)]=x0;
		else value[coords2index(pos)]=va_arg(marker,Element_Type);
	}
	va_end(marker);
	
	DELETEPOS(pos);
}











template <>
void *Tensor<double>::retrieveBitmap() {
	ErrorExit("retrieveBitmap not implemented for double vectors.");
	return NULL;
}

template <class Element_Type>
void *Tensor<Element_Type>::retrieveBitmap() {
	if (n!=2) ErrorExit("retrieveBitmap is only allowed on bidimensional vectors.");

	Bitmap<Element_Type> *img=new Bitmap<Element_Type>(dim[0],dim[1]);
	for(int i=0;i<size;i++) img->SetPoint((int)(i%img->width),(int)(i/img->width),value[i]);
	return img;
}


template <>
void Tensor<GreyLevel>::setBitmap(void *img) {
	if (n!=2) ErrorExit("setBitmap is only allowed on bidimensional vectors.");
	
	Bitmap<GreyLevel> *img_=(Bitmap<GreyLevel> *)img;
	if ((img_->width!=dim[0]) || (img_->height!=dim[1])) ErrorExit("size do not corresponds.");

	for(int i=0;i<size;i++) value[i]=img_->Point((int)(i%img_->width),(int)(i/img_->width));
}

template <>
void Tensor<WI_Type>::setBitmap(void *img) {
	if (n!=2) ErrorExit("setBitmap is only allowed on bidimensional vectors.");
	
	Bitmap<WI_Type> *img_=(Bitmap<WI_Type> *)img;
	if ((img_->width!=dim[0]) || (img_->height!=dim[1])) ErrorExit("size do not corresponds.");

	for(int i=0;i<size;i++) value[i]=img_->Point((int)(i%img_->width),(int)(i/img_->width));
}

template <class Element_Type>
void Tensor<Element_Type>::setBitmap(void *img) {
	ErrorExit("setBitmap not implemented for double vectors.");
}


///////////////////////////////////////////////////////
//  
///////////////////////////////////////////////////////
template <class Element_Type>
const Tensor<Element_Type> &Tensor<Element_Type>::operator=(const Tensor<Element_Type> &b) {
	if (b.size!=size) ErrorExit("Size does not match.");

	for(int i=0;i<size;i++) value[i]=b.value[i];
	return (*this);
}












///////////////////////////////////////////////////////
//  Operations
///////////////////////////////////////////////////////
template <class Element_Type>
const Tensor<Element_Type> &Tensor<Element_Type>::operator+=(const Tensor<Element_Type> &b) {
	if (b.size!=size) ErrorExit("Size doesn't match.");
	
	for(int i=0;i<size;i++) value[i]+=b.value[i];
	return *this; 
}
template <class Element_Type>
const Tensor<Element_Type> &Tensor<Element_Type>::operator-=(const Tensor<Element_Type> &b) {
	if (b.size!=size) ErrorExit("Size doesn't match.");

	for(int i=0;i<size;i++) value[i]-=b.value[i];
	return *this; 
}
template <class Element_Type>
const Tensor<Element_Type> Tensor<Element_Type>::operator+(const Tensor<Element_Type> &b) const {
	if (b.size!=size) ErrorExit("Size doesn't match.");

	Tensor<Element_Type> q(*this);
	q+=b;
	return q;
}
template <class Element_Type>
const Tensor<Element_Type> Tensor<Element_Type>::operator-(const Tensor<Element_Type> &b) const {
	if (b.size!=size) ErrorExit("Size doesn't match.");

	Tensor<Element_Type> q(*this);
	q-=b;
	return q; 
}
template <class Element_Type>
const Tensor<Element_Type> &Tensor<Element_Type>::operator*=(const Element_Type &b) {
	for(int i=0;i<size;i++) value[i]=b*value[i];
	return (*this);
}
template <class Element_Type>
const Tensor<Element_Type> Tensor<Element_Type>::operator*(const Element_Type &b) const{
	Tensor<Element_Type> q(*this);
	q*=b;
	return q; 
}



///////////////////////////////////////////////////////
//    SubSpaces
///////////////////////////////////////////////////////
template <class Element_Type>
const int Tensor<Element_Type>::variabile(const int b_index) {
	return (-1-b_index);
}

template <class Element_Type>
void Tensor<Element_Type>::insert(const Tensor<Element_Type> *b,const int *indexes) {
	
	// Controlla tutti tranne i negativi (>=0 && >=dim e' lo stesso di >=dim)
	for(int i=0;i<n;i++) if (indexes[i]>=dim[i]) ErrorExit("Index out of bounds.");
	
	for(int i=0;i<n;i++) {
		if (indexes[i]<0) {
			if ((-indexes[i]-1)>=b->n) ErrorExit("Variable index out of bounds.");
			if (b->dim[(-indexes[i]-1)]!=dim[i]) ErrorExit("Size doesn't match.");
		}
	}

	// copia
	CREATEPOS(SI,b->n);
	CREATEPOS(TI,n);
	for(int i=0;i<b->size;i++) {
		b->index2coords(i,SI);
		
		for(int j=0;j<n;j++) {
			if (indexes[j]>=0) TI[j]=indexes[j];
			else TI[j]=SI[-indexes[j]-1];
		}
		
		vv(TI)=b->vl(i);
	}
	DELETEPOS(TI);
	DELETEPOS(SI);
}

template <class Element_Type>
void Tensor<Element_Type>::extract(Tensor<Element_Type> *b,const int *indexes) const {
	
	// Controlla tutti tranne i negativi (>=0 && >=dim e' lo stesso di >=dim)
	for(int i=0;i<n;i++) if (indexes[i]>=dim[i]) ErrorExit("Index out of bounds.");
	
	for(int i=0;i<n;i++) {
		if (indexes[i]<0) {
			if ((-indexes[i]-1)>=b->n) ErrorExit("Variable index out of bounds.");
			if (b->dim[(-indexes[i]-1)]!=dim[i]) ErrorExit("Size doesn't match.");
		}
	}

	// copia
	CREATEPOS(SI,b->n);
	CREATEPOS(TI,n);
	for(int i=0;i<b->size;i++) {
		b->index2coords(i,SI);
		
		for(int j=0;j<n;j++) {
			if (indexes[j]>=0) TI[j]=indexes[j];
			else TI[j]=SI[-indexes[j]-1];
		}
		
		b->vl(i)=vv(TI);
	}
	DELETEPOS(TI);
	DELETEPOS(SI);
}

template <class Element_Type>
void Tensor<Element_Type>::insert(const Tensor<Element_Type> *b,const int i0,...) {
	// b,v(0),v(1),x,v(3),...,v(n-1)
	CREATEPOS(pos,n);
	READPOS(pos,i0);

	insert(b,pos);

	DELETEPOS(pos);
}

template <class Element_Type>
void Tensor<Element_Type>::extract(Tensor<Element_Type> *b,const int i0,...) const {
	CREATEPOS(pos,n);
	READPOS(pos,i0);
	
	extract(b,pos);

	DELETEPOS(pos);
}

template <class Element_Type>
Tensor<Element_Type> *Tensor<Element_Type>::extract(const int i0,...) const {
	CREATEPOS(pos,n);
	READPOS(pos,i0);
	
	int new_n=0;
	for(int i=0;i<n;i++) if (pos[i]<0) new_n++;
	
	CREATEPOS(new_dim,new_n);
	for(int i=0;i<new_n;i++) new_dim[i]=-1;

	for(int i=0;i<n;i++) {
		if (pos[i]<0) {
			if ((-pos[i]-1)>=new_n) ErrorExit("Variable index out of bounds.");
			new_dim[(-pos[i]-1)]=dim[i];
		}
	}
	for(int i=0;i<new_n;i++) if (new_dim[i]<0) ErrorExit("Not all the dimension of the new vector can be determined.");


	Tensor<Element_Type> *b=new Tensor<Element_Type>(new_n,new_dim);

	extract(b,pos);

	DELETEPOS(new_dim);
	DELETEPOS(pos);
	return b;
}

template <class Element_Type>
Tensor<Element_Type> *Tensor<Element_Type>::resize() const {
	// Create output
	CREATEPOS(new_dim,n);
	for(int i=0;i<n;i++) {
		if (dim[i]%2!=0) ErrorExit("Cannot resize. Dimensions are not all even.");
		new_dim[i]=dim[i]/2;
	}
	Tensor<Element_Type> *b=new Tensor<Element_Type>(n,new_dim);
	DELETEPOS(new_dim);

	
	// Create box filter
	CREATEPOS(box_dim,n);
	for(int i=0;i<n;i++) box_dim[i]=2;
	int box_size=1<<n;
	

	// Apply
	CREATEPOS(pos,n);
	CREATEPOS(pos2,n);
	for(int i=0;i<b->size;i++) {
		b->index2coords(i,pos);
		for(int j=0;j<n;j++) pos[j]*=2;
		// b->vl(i)= average cubo con left upper corner vv(pos)
		
		// Explore the cube 
		double average=0.0;
		for(int j=0;j<box_size;j++) {
			get_coords(n,box_dim,box_size,j,pos2);
			for(int q=0;q<n;q++) pos2[q]=pos[q]+pos2[q];
			average+=vv(pos2);
		}
		average/=box_size;
		
		b->vl(i)=(Element_Type)average;
	}
	DELETEPOS(pos2);
	DELETEPOS(pos);

	DELETEPOS(box_dim);
	return b;
}


template <class Element_Type>
void Tensor<Element_Type>::subspace_explore(int *s_pos,int missing_dim_index,int &start_index,int &step,int &len) const {
	CREATEPOS(pos,n);

	// Prima coordinata della linea
	for (int i=0,o=0;i<n;i++) if (i!=missing_dim_index) pos[i]=s_pos[o++];
	pos[missing_dim_index]=0;
	start_index=coords2index(pos);

	// incremento e lunghezza linea
	step=getstep(missing_dim_index);
	len=dim[missing_dim_index];

	DELETEPOS(pos);
}

template <>
void Tensor<double>::DWT(int dimension_index,Tensor<double> *&l,Tensor<double> *&h) const {
	ErrorExit("Not implemented.");
}
template <>
void Tensor<float>::DWT(int dimension_index,Tensor<float> *&l,Tensor<float> *&h) const {
	ErrorExit("Not implemented.");
}

template <class Element_Type>
void Tensor<Element_Type>::DWT(int dimension_index,Tensor<Element_Type> *&l,Tensor<Element_Type> *&h) const {
	// Create output
	CREATEPOS(new_dim,n);
	for(int i=0;i<n;i++) new_dim[i]=dim[i];
	if (dim[dimension_index]%2!=0) ErrorExit("Dimension is not even.");
	new_dim[dimension_index]=dim[dimension_index]/2;
	l=new Tensor<Element_Type>(n,new_dim);
	h=new Tensor<Element_Type>(n,new_dim);
	DELETEPOS(new_dim);


	// Create subspace
	CREATEPOS(subspace_dim,n-1);
	for (int i=0,o=0;i<n;i++) if (i!=dimension_index) subspace_dim[o++]=dim[i];
	int subspace_size=(size/dim[dimension_index]);
	
	
	CREATEPOS(s_pos,n-1);
	CREATEPOS(pos,n);
	for(int i=0;i<subspace_size;i++) {
		// Esploro il sottospazio di linee
		get_coords(n-1,subspace_dim,subspace_size,i,s_pos);

		int st_in,step_in,len_in;
		int st_out_l,step_out_l,len_out_l;
		int st_out_h,step_out_h,len_out_h;

		subspace_explore(s_pos,dimension_index,st_in,step_in,len_in);
		l->subspace_explore(s_pos,dimension_index,st_out_l,step_out_l,len_out_l);
		h->subspace_explore(s_pos,dimension_index,st_out_h,step_out_h,len_out_h);
		
		
		analyze_lpf<Element_Type,Element_Type>(value+st_in,0,len_in,step_in,
			                                   l->value+st_out_l,0,step_out_l,
											   h->value+st_out_h,0,step_out_h);
	}
	DELETEPOS(pos);
	DELETEPOS(s_pos);


	DELETEPOS(subspace_dim);
}



template <>
void Tensor<WI_Type>::iDWT(int dimension_index,Tensor<WI_Type> *l,Tensor<WI_Type> *h) {
	// check output/this size
	// TODO


	// Create subspace
	CREATEPOS(subspace_dim,n-1);
	for (int i=0,o=0;i<n;i++) if (i!=dimension_index) subspace_dim[o++]=dim[i];
	int subspace_size=(size/dim[dimension_index]);
	
	
	CREATEPOS(s_pos,n-1);
	CREATEPOS(pos,n);
	for(int i=0;i<subspace_size;i++) {
		// Esploro il sottospazio di linee
		get_coords(n-1,subspace_dim,subspace_size,i,s_pos);

		int st_in,step_in,len_in;
		int st_out_l,step_out_l,len_out_l;
		int st_out_h,step_out_h,len_out_h;

		subspace_explore(s_pos,dimension_index,st_in,step_in,len_in);
		l->subspace_explore(s_pos,dimension_index,st_out_l,step_out_l,len_out_l);
		h->subspace_explore(s_pos,dimension_index,st_out_h,step_out_h,len_out_h);
		
		// *** TODO
		
		synthetize_lpf<WI_Type,WI_Type>(l->value+st_out_l,0,len_out_l,step_out_l,
									    h->value+st_out_h,0,len_out_h,step_out_h,
									    value+st_in,0,step_in);
	}
	DELETEPOS(pos);
	DELETEPOS(s_pos);


	DELETEPOS(subspace_dim);
}

template <class Element_Type>
void Tensor<Element_Type>::iDWT(int dimension_index,Tensor<Element_Type> *l,Tensor<Element_Type> *h) {
	ErrorExit("Not implemented.");
}


template <class Element_Type>
Tensor<Element_Type> **Tensor<Element_Type>::DWT() {

	int num=(1<<n);
	Tensor<Element_Type> ** T=new Tensor<Element_Type> *[num];
	Tensor<Element_Type> **NT=new Tensor<Element_Type> *[num];
	for(int i=0;i<num;i++)  T[i]=NULL;
	

	T[0]=this;
	for(int i=n-1;i>=0;i--) {
		
		for(int j=0;j<num;j++) NT[j]=NULL;

		for(int j=0;j<num;j+=(1<<(i+1))) {
			
			T[j]->DWT(i,(NT[j]),(NT[j+(1<<i)]));
			if (T[j]!=this) delete T[j];

		}

		for(int j=0;j<num;j++) T[j]=NT[j];
		
	}
	delete []NT;
	
	// TODO controllo delete

	return T;	



	//
	// LLL HLL
	// LHL HHL
	//
	// LLH HLH
	// LHH HHH
	//
	//
	// COORDS(i,pos) -> pos[j] = 0  L
	// 		    pos[j] = 1  H
	//
	// Name:     0       1      2      3 
	//       pos[0] pos[1] pos[2] pos[3]
	//            L      L       H     H 
	//

}



//
//::DWT(int direction,classe **A,classe **B) const;
//::iDWT(int direction,classe *A,classe *B);
//
//
//


template <class Element_Type>
void Tensor<Element_Type>::iDWT(Tensor<Element_Type> **a) {

	int num=(1<<n);
	Tensor<Element_Type> ** T=new Tensor<Element_Type> *[num];
	Tensor<Element_Type> **NT=new Tensor<Element_Type> *[num];
	Tensor<Element_Type> **Buffer=new Tensor<Element_Type> *[num];


	for(int i=0;i<num;i++) T[i]=a[i];

	CREATEPOS(new_dim,n);
	for(int q=0;q<n;q++) new_dim[q]=dim[q]/2;

	for(int i=0;i<n;i++) {
		
		for(int j=0;j<num;j++) NT[j]=NULL;

		for(int j=0;j<num;j+=(1<<(i+1))) {
			
			if (i==n-1) {
				NT[j]=this;
			} else {
				new_dim[i]=dim[i];
				NT[j]=new Tensor<Element_Type>(n,new_dim);
			}

			NT[j]->iDWT(i,T[j],T[j+(1<<i)]);
			
			if (T[j]!=a[i]) delete T[j];
			if (T[j+(1<<i)]!=a[j+(1<<i)]) delete T[j+(1<<i)];

		}

		for(int j=0;j<num;j++) T[j]=NT[j];
		
	}

	DELETEPOS(new_dim);
	

	// TODO controllo delete
	
	delete []NT;
	delete []T;
} 


//
//// Rimane da cancellare tutti gli a[i] ...
//
//
//
//
//
//
void WTA_FUS(Tensor<WI_Type> *A,Tensor<WI_Type> *B,Tensor<ColorFloat> *mA,Tensor<ColorFloat> *mB,Tensor<WI_Type> *C) {
	int num=A->dimension();

	for(int i=0;i<num;i++) {
		if (mA->vl(i)>mB->vl(i)) C->vl(i)=A->vl(i);
		else C->vl(i)=B->vl(i);
	}

}

void MEAN_FUS(Tensor<WI_Type> *A,Tensor<WI_Type> *B,Tensor<ColorFloat> *mA,Tensor<ColorFloat> *mB,Tensor<WI_Type> *C) {
	int num=A->dimension();

	for(int i=0;i<num;i++) C->vl(i)=(WI_Type)((mA->vl(i)*A->vl(i))+(mB->vl(i)*B->vl(i)));

}
//
//
//// A,B,mA,mB const
//
void WAVELET_FUS(Tensor<WI_Type> *A,Tensor<WI_Type> *B,Tensor<ColorFloat> *mA,Tensor<ColorFloat> *mB,Tensor<WI_Type> *C,int level) {


	if (level>1) {
		int n=A->order();
		int num=(1<<n);

		Tensor<WI_Type> **wA=A->DWT();
		Tensor<WI_Type> **wB=B->DWT();
		
		Tensor<ColorFloat> *RmA=mA->resize();
		Tensor<ColorFloat> *RmB=mB->resize();


		Tensor<WI_Type> **T=new Tensor<WI_Type> *[num];
				CREATEPOS(dim2,n);
				for(int q=0;q<n;q++) dim2[q]=A->sizes(q)/2;
		for(int j=0;j<num;j++) T[j]=new Tensor<WI_Type>(n,dim2);	// dim2=dim/2 
				DELETEPOS(dim2);

		
		for(int j=1;j<num;j++) WTA_FUS(wA[j],wB[j],RmA,RmB,T[j]);
		WAVELET_FUS(wA[0],wB[0],RmA,RmB,T[0],level-1);


		for(int j=0;j<num;j++) delete wA[j],wB[j];
		delete []wA;
		delete []wB;
		delete RmA,RmB;


		C->iDWT(T);

		//for(int j=0;j<num;j++) delete T[j]; TODO
		//delete []T;


	} else {
		WTA_FUS(A,B,mA,mB,C);
	}

}












































/*
template <class Element_Type>
Tensor<Element_Type> **Tensor<Element_Type>::DWT(int dimension_index) {
	// Creating subspace_dim
	int *subspace_dim=NULL;
	SNEWA(subspace_dim,int,n-1);
	for (int i=0,o=0;i<n;i++) if (i!=dimension_index) subspace_dim[o++]=dim[i];

	// Creating new_dim
	int *new_dim=NULL;
	SNEWA(new_dim,int,n);
	for (int i=0;i<n;i++) new_dim[i]=dim[i];
	
	// Split dimension dimension_index
	if (new_dim[dimension_index]%2!=0) ErrorExit("Not a even dimension.");
	new_dim[dimension_index]=new_dim[dimension_index]/2;
	

	// Creating Out
	Tensor<Element_Type> **Out=NULL;
	SNEWA_P(Out,Tensor<Element_Type> *,2);
	SNEW(Out[0],Tensor<Element_Type>(n,new_dim));
	SNEW(Out[1],Tensor<Element_Type>(n,new_dim));


	WI_Type *desthigh,*destlow,*src;
	src=value;
	destlow=Out[0]->value;
	desthigh=Out[1]->value;
	

	int *pos=NULL;
	SNEWA(pos,int,n-1);	// TODO DELETE
	int *n_pos=NULL;
	SNEWA(n_pos,int,n);	// TODO DELETE

	for(int i=0;i<(size/dim[dimension_index]);i++) {
		// i -> to all the coordinates nella fashion (n-1)
		// [all coords, 0] -> in coord in (n) fashion
		// questo e' il punto di partenza
		// lo step e' dato da quello attuale 
		get_coords(n-1,subspace_dim,i,pos);
		
		_coords(n-1,subspace_dim,i,pos);
	}

	for(int i=0;i<n;i++) {
		for(int i=0;i<n;i++) {
			analyze_lpf<WI_Type,WI_Type>(src,0,width,1,destlow,0,1,desthigh,0,1);

			src+=width;
			destlow+=HalfWidth;
			desthigh+=HalfWidth;
		}
	}



	
	// Deleting new_dim
	SDELETEA(new_dim);
	SDELETEA(subspace_dim);
	return Out;

	//for (int i=0;i<n_bands;i++) SDELETE(Out[i]);
	//SDELETEA(Out);
}
*/