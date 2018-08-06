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





#ifdef LIST_MEMCHECK
#define MEMCHECK(mem_p) if (mem_p==NULL) ErrorExit("List: Not enought memory.");
#else
#define MEMCHECK(mem_p)
#endif



template <class data_type>
DoubleList<data_type>::DoubleList(int r,int c) : mem(r) {
	created_list_num=0;
	def_c=c;

	// elimina i NULL
	create_sub_list();
}


template <class data_type>
DoubleList<data_type>::~DoubleList() {
	for(int i=0;i<created_list_num;i++) {
		delete ((List<data_type> *)(mem[i].pointer));
	}
}


// Elimina i NULL esistenti in mem[x].pointer 
// Se chiamata ad ogni modifica di dimensione di mem -> si e' sicuri che nn vi sia alcun NULL.
template <class data_type>
void DoubleList<data_type>::create_sub_list() {
	
	for(int j=created_list_num;j<mem.actual_buffer_dim();j++) {
		mem[j].pointer=(void *)new List<data_type>(def_c);
	}
	
	created_list_num=mem.actual_buffer_dim();
}








template <class data_type>
List<data_type>::List(const List<data_type> &o) {
	num=o.num;
	mem=new data_type[num];
	MEMCHECK(mem);
	memcpy(mem,o.mem,sizeof(data_type)*num);
}	

template <class data_type>
List<data_type>::List(int num) {
	if (num<=0) ErrorExit("List: Initial dimension must be >0");
	this->num=num;
	mem=new data_type[num];
	MEMCHECK(mem);
}

template <class data_type>
List<data_type>::~List() {
	if (mem!=NULL) delete []mem;
}

template <class data_type>
void List<data_type>::CopyFrom(List<data_type> *src) {
	if (src->num>num) {
		if (mem!=NULL) delete []mem;
		mem=new data_type[src->num];
		MEMCHECK(mem);
		num=src->num;
	}
	
	memcpy(mem,src->mem,sizeof(data_type)*src->num);
}

template <class data_type>
void List<data_type>::makeDouble() {
	data_type *tmp=new data_type[2*num];
	MEMCHECK(tmp);
	memcpy(tmp,mem,sizeof(data_type)*num);
	delete []mem;
	mem=tmp;
	num*=2;
}

template <class data_type>
void List<data_type>::resize(int newdim) {
	if (newdim==num) return;
	data_type *tmp=new data_type[newdim];
	MEMCHECK(tmp);
	memcpy(tmp,mem,sizeof(data_type)*min(num,newdim));
	delete []mem;
	mem=tmp;
	num=newdim;
}

template <class data_type>
void List<data_type>::enlarge_to_at_least(int dim) {
	if (num<dim) resize(dim);
}

template <class data_type>
int List<data_type>::actual_buffer_dim() {
	return num;
}


template <class data_type>
void Array<data_type>::CopyFrom(Array<data_type> *src) {
	if (src->num_elements>num) {
		if (mem!=NULL) delete []mem;
		mem=new data_type[src->num_elements];
		MEMCHECK(mem);
		num=src->num_elements;
	}
	
	num_elements=src->num_elements;
	memcpy(mem,src->mem,sizeof(data_type)*num_elements);
}

template <class data_type>
int Array<data_type>::append(data_type v) {
	(*this)[num_elements]=v;
	return num_elements++;
}

template <class data_type>
void Array<data_type>::append(Array<data_type> *a) {
	int add_num=a->numElements();
	
	for(int i=0;i<add_num;i++) append((*a)[i]);
}

template <class data_type>
bool Array<data_type>::del(int i) {
	if (i>=num_elements) return false;

	for(int j=i+1;j<num_elements;j++) mem[j-1]=mem[j];
	num_elements--;

	return true;
}

template <class data_type>
bool Array<data_type>::insertBefore(int i,data_type v) {
	if (i>=num_elements) return false;
	
	// Make the list one element longer
	(*this)[num_elements]=v;

	// Inverse copy
	for(int j=num_elements-1;j>=i;j--) mem[j+1]=mem[j];
	mem[i]=v;
	num_elements++;

	return true;
}

template <class data_type>
void Array<data_type>::clear() {
	num_elements=0;
}
	
template <class data_type>
void Array<data_type>::reverse() {
	data_type tmp;
	int half=num_elements/2;
	
	for(int j=0;j<half;j++) {
		tmp=mem[j];
		mem[j]=mem[num_elements-1-j];
		mem[num_elements-1-j]=tmp;
	}
}

template <class data_type>
int Array<data_type>::search(data_type v) {
	data_type *tmp_mem=mem;
	for(int i=0;i<num_elements;i++,tmp_mem++)
		if ((*tmp_mem)==v) return i;

	return -1;
}

template <class data_type>
int Array<data_type>::search(bool (*compare_fnc)(data_type *a,data_type *b),data_type *to_src) {
	data_type *tmp_mem=mem;
	for(int i=0;i<num_elements;i++,tmp_mem++)
		if (compare_fnc(tmp_mem,to_src)) return i;

	return -1;
}


template <class data_type>
int Array<data_type>::append_sorted(data_type v,int (*compare_fnc)(data_type *a,data_type *b)) {
	if (num_elements<=0) return append(v);

	// k is the searched element or an element greater v 
	int k=binary_search_imin(compare_fnc,&v);
	int s=k;
	while((k>=0) && (compare_fnc(mem+k,&v)>0)) k--;
	if (s!=k) {
		k++;
		insertBefore(k,v);
		return k;
	}
	while((k<=num_elements-1) && (compare_fnc(mem+k,&v)<0)) k++;
	if (k>num_elements-1) return append(v);
	insertBefore(k,v);
	return k;
}


template <class data_type>
int Array<data_type>::binary_search_imin(int (*compare_fnc)(data_type *a,data_type *b),data_type *to_src) {
	int imin=0;
	int imax=num_elements-1;
	
	while (imax>=imin) {
		int imid=(imin+imax)/2;
		int compare_result=compare_fnc(mem+imid,to_src);
		if (compare_result<0) {
			imin=imid+1;
		} else if (compare_result>0) {
			imax=imid-1;
		} else {
			return imid;
		}
	}

	return imin;
}

template <class data_type>
int Array<data_type>::binary_search(int (*compare_fnc)(data_type *a,data_type *b),data_type *to_src) {
	if (num_elements<=0) return -1;
	int imin=0;
	int imax=num_elements-1;
	
	while (imax>=imin) {
		int imid=(imin+imax)/2;
		int compare_result=compare_fnc(mem+imid,to_src);
		if (compare_result<0) {
			imin=imid+1;
		} else if (compare_result>0) {
			imax=imid-1;
		} else {
			return imid;
		}
	}

	return -1;
}


template <class data_type>
void Array<data_type>::sort(int(__cdecl *compare)(const void*,const void*)) {
	qsort((void*)mem,(size_t)num_elements,sizeof(data_type),compare);
}

template <class data_type>
data_type Array<data_type>::Moda() {
	if (num_elements==0) ErrorExit("Cannot compute the moda of an empty array.");

	Array<data_type> values(num_elements);
	Array<int> count(num_elements);

	for(int i=0;i<num_elements;i++) {
		int index=values.search(mem[i]);
		if (index==-1) {
			values.append(mem[i]);
			count.append(0);
		} else {
			count[index]=count[index]+1;
		}
	}
	
	data_type max_v;
	int max_c=0;
	for(int i=0;i<values.numElements();i++) {
		MAX_I((count[i]),max_c,(values[i]),max_v);
	}
	return max_v;
}





template <class data_type>
OFileBuffer &operator<<(OFileBuffer &os,Array<data_type> &v) {
	for(int i=0;i<v.num_elements;i++) {
		os<<v.mem[i];
		if (i!=v.num_elements-1) os<<",";
		os<<"\r\n";
	}
	return os;
}
template <class data_type>
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Array<data_type> &v) {
	os<<"[";
	for(int i=0;i<v.num_elements;i++) {
		os<<v.mem[i];
		if (i!=v.num_elements-1) os<<",";
	}
	os<<"]";
	return os;
}

#pragma warning(disable:4700)
template <class data_type>
IFileBuffer &operator>>(IFileBuffer &os,Array<data_type> &v) {
	char text[51];
	data_type r;
	os.set_wordwidth(50);

	while(true) {
		os>>text;
		if (!strcmp(text,"end")) break;
		os.back();
		
		
		os>>r;
		v.append(r);
	}

	return os;
}
#pragma warning(default:4700)

int compare_byte(const void *a,const void *b) {
	BYTE *a_=(BYTE *)a;
	BYTE *b_=(BYTE *)b;
	
	if ((*a_)>(*b_)) return 1;
	if ((*a_)==(*b_)) return 0;
	return -1;
}

int compare_int(const void *a,const void *b) {
	int *a_=(int *)a;
	int *b_=(int *)b;
	
	if ((*a_)>(*b_)) return 1;
	if ((*a_)==(*b_)) return 0;
	return -1;
}

int compare_double(const void *a,const void *b) {
	double *a_=(double *)a;
	double *b_=(double *)b;
	
	if ((*a_)>(*b_)) return 1;
	if ((*a_)==(*b_)) return 0;
	return -1;
}












template <class data_type>
CircularArray<data_type>::CircularArray(int size) {
	if (size<=0) ErrorExit("CircularArray: size must be >0");
	num=size;
	mem=NULL;
	i=0;
	o=0;

	SNEWA(mem,data_type,num);
}

template <class data_type>
CircularArray<data_type>::~CircularArray() {
	SDELETEA(mem);
}

template <class data_type>
void CircularArray<data_type>::push(data_type x) {
	if (isFull()) ErrorExit("CircularArray: push on a full array.");

	mem[i]=x;
	i=(i+1)%num;
}

template <class data_type>
data_type CircularArray<data_type>::pop() {
	if (isEmpty()) ErrorExit("CircularArray: pop on an empty array.");
	
	int s_o=o;
	o=(o+1)%num;
	return mem[s_o];
}

template <class data_type>
bool CircularArray<data_type>::isEmpty() {
	return (o==i);
}

template <class data_type>
bool CircularArray<data_type>::isFull() {
	int f_i=(i+1)%num;
	return (o==f_i);
}


template CircularArray<float>;
template CircularArray<double>;
