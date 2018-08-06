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





//
// Oggetto List:
//
//			Lista virtuale di oggetti solidi (ad accesso veloce)
//				oggetti solidi =  oggetti i cui costruttori NON effettuano allocazioni o new
//								  sono caratterizzati dalla sola dimensione in memoria, specificata da sizeof()
//								  List li cancellera' con un delete
//				
//				accesso veloce =  l'accesso veloce e' ottenuto tramite l'operatore []
//								  esso ritorna SEMPRE un riferimento all'oggetto evitando quindi copie
//
//				restrizioni    =  NON devono essere eseguite espressioni i cui l-value che r-value
//								  contengano riferimenti alla stessa lista:
//									Es:
//											List[i]=List[j]+List[k];		// ERRORE: risultato imprevisto!!!
//									
//											tmp=List[j]+List[k];			// CORRETTO
//											List[i]=tmp;
//
//
//								  Usare i riferimenti ad oggetti nella lista anziche' gli oggetti veri 
//								  e' possibile fintanto che non si acceda a locazioni List superiori a quelle 
//								  usate precedentemente:
//									Es:
//											List[10]=0;
//											tmp=&(List[1]);	
//											List[1]=tmp;					// CORRETTO
//											List[15]=tmp;					// ERRORE: risultato imprevisto!!
//
//
//				
//				Multi-tasking  =  L'accesso alla lista deve essere esclusivo
//								  L'accesso alla lista deve essere esclusivo anche finche' ci si riferisce ad un suo oggetto
//									Es:
//											x=&(List[j]);
//											x-> ...							// Finche' accedo a x
//																			// devo mantenere l'accesso esclusivo a List
//
//


template <class data_type>
class List {
protected:
	int num;
	data_type *mem;

	void makeDouble();
public:
	List(const List<data_type> &o);
	List(int num);
	~List();

	data_type &operator[](const int i) {
		while (i>=num) makeDouble();
		return mem[i];
	}
	
	void resize(int newdim);
	void enlarge_to_at_least(int dim);

	int actual_buffer_dim();
	
	data_type *getMem() {return mem;}

	void CopyFrom(List<data_type> *src);
};	









//
// Oggetto Array:
//
//			Array virtuale di oggetti solidi (ad accesso veloce)
//
//				restrizioni    =  stesse considerazioni di List e in aggiunta:
//
//								  I riferimenti ad oggetti nell'array si possono usare 
//								  fintanto che non si esegua operazioni di modifica della lista
//										del
//										append
//										insertBefore
//										reverse, copy, sort, clear
//
//				
//				Multi-tasking  =  stesse considerazioni di List 
//
//


template <class data_type>
class Array: public List<data_type> {
	template <class data_type> friend OFileBuffer &operator<<(OFileBuffer &os,Array<data_type> &v);
	template <class data_type> friend IFileBuffer &operator>>(IFileBuffer &os,Array<data_type> &v);
	template <class data_type> friend STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Array<data_type> &v);
	int binary_search_imin(int (*compare_fnc)(data_type *a,data_type *b),data_type *to_src);

	int num_elements;
public:
	Array(const Array<data_type> &o) : List<data_type>(o),num_elements(o.num_elements) {};
	Array(const List<data_type> &l,int num) : List<data_type>(l),num_elements(num) {};
	Array(int num) : List<data_type>(num),num_elements(0) {};
	~Array() {};

	int  numElements() const {return num_elements;}

	// Metodi di modifica della lista
	bool insertBefore(int i,data_type v);
	int  append(data_type v);
	void append(Array<data_type> *a);
	int  append_sorted(data_type v,int (*compare_fnc)(data_type *a,data_type *b));

	bool del(int i);
	void clear();

	void sort(int(__cdecl *compare)(const void*,const void*));
	void reverse();
	void CopyFrom(Array<data_type> *src);




	// Medodi di lettura
	int  search(data_type v);																		// usa operatore == per la comparazione	
																									// ritorna l'indice o -1 se nn lo trova
	int  search(bool (*compare_fnc)(data_type *a,data_type *b),data_type *to_src);					// overload dell'operatore ==
	int  binary_search(int (*compare_fnc)(data_type *a,data_type *b),data_type *to_src);			


	// compare_fnc(a,b)         -1 if a < b
	//                           0 if a = b
	//						    +1 if a > b



	data_type Moda();



};
template <class data_type> OFileBuffer &operator<<(OFileBuffer &os,Array<data_type> &v);
template <class data_type> IFileBuffer &operator>>(IFileBuffer &os,Array<data_type> &v);
template <class data_type> STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Array<data_type> &v);

// Comparison Functions
int compare_byte   (const void *a,const void *b);
int compare_int    (const void *a,const void *b);
int compare_double (const void *a,const void *b);










// Doppia lista.
// Non e' possibile farlo con list<list> perche' list nn e' un true object

template <class data_type>
class DoubleList {
	List<Pointer> mem;

	int created_list_num;
	int def_c;

	void create_sub_list();
public:
	DoubleList(int r,int c);
	~DoubleList();

	List<data_type> &operator[](const int i) {
		if (i>=created_list_num) {
			mem[i].pointer=NULL;             // ingrandisce mem
			create_sub_list();               // elimina i NULL
		}
		return (*((List<data_type> *)(mem[i].pointer)));
	}
	
};	











template <class data_type>
class CircularArray {
protected:
	int i,o;
	int num;
	data_type *mem;

public:
	CircularArray(int size);
	~CircularArray();

	void       push(data_type x);
	data_type  pop();
	bool       isEmpty();
	bool       isFull();
};



