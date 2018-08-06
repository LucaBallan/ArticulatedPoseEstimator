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





class Pointer {
public:
	void *pointer;

	Pointer() : pointer(NULL) {}
	Pointer(void *p) : pointer(p) {}
	bool operator== (const Pointer &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,Pointer &v);
IFileBuffer &operator>>(IFileBuffer &os,Pointer &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Pointer &v);




class ObjectIndex_couple {
public:	
	UINT index;
	void *object;
	int n_accesses;
	int i_creation;

	ObjectIndex_couple() {};
	bool operator== (const ObjectIndex_couple &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,ObjectIndex_couple &v);
IFileBuffer &operator>>(IFileBuffer &os,ObjectIndex_couple &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,ObjectIndex_couple &v);




class Task {
public:
	void (*F)(void *data,void *global_data);
	void *data;
	HANDLE done;

	Task() {};
	bool operator== (const Task &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,Task &v);
IFileBuffer &operator>>(IFileBuffer &os,Task &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Task &v);




class RepetitiveTask {
public:
	bool (*F)(void *data,void *global_data);
	void *data;
	int task_id;
	int task_class;
		// Redundant
		int task_class_array_index;
		int task_priority;
	

	RepetitiveTask() {};
	bool operator== (const RepetitiveTask &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,RepetitiveTask &v);
IFileBuffer &operator>>(IFileBuffer &os,RepetitiveTask &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,RepetitiveTask &v);




class RepetitiveTaskClassInfo {
public:
	// Info
	int class_id;
	int priority;
	bool exclusive_execution;
	
	// internal variables
	bool avoid_execution;


	RepetitiveTaskClassInfo() {};
	bool operator== (const RepetitiveTaskClassInfo &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,RepetitiveTaskClassInfo &v);
IFileBuffer &operator>>(IFileBuffer &os,RepetitiveTaskClassInfo &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,RepetitiveTaskClassInfo &v);





class ordered_pair {
public:	
	int i;
	int j;
	
	ordered_pair() {};
	bool operator== (const ordered_pair &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,ordered_pair &v);
IFileBuffer &operator>>(IFileBuffer &os,ordered_pair &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,ordered_pair &v);





class weighted_index {
public:	
	int i;
	double w_1,w_2,w_3;
	
	weighted_index() {};
	weighted_index(int i,double w_1,double w_2,double w_3);
	bool operator== (const weighted_index &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,weighted_index &v);
IFileBuffer &operator>>(IFileBuffer &os,weighted_index &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,weighted_index &v);





// Coppia ordinata di oggetti
//
//		object_type    =  deve essere un oggetto solido  (ovvero, oggetti i cui costruttori NON effettuano allocazioni o new)
//						  deve avere gli operatori =, ==, << (OFileBuffer and STANDARD_OUTPUT), >> (IFileBuffer)
//
//
template <class object_type>
class object_pair {
public:	
	object_type i;
	object_type j;
	
	object_pair() {};
	bool operator== (const object_pair<object_type> &v) const;
};
template <class object_type>
OFileBuffer &operator<<(OFileBuffer &os,object_pair<object_type> &v);
template <class object_type>
IFileBuffer &operator>>(IFileBuffer &os,object_pair<object_type> &v);
template <class object_type>
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,object_pair<object_type> &v);








template <class T>
int sort_increasing_compare(const void *a,const void *b);




