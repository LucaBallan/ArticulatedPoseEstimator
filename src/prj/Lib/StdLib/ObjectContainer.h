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





#define POLICY_FIFO 0
#define POLICY_FILO 1
#define POLICY_BEST 2

// Multi-Task Ok

class ObjectContainer_class {
	HANDLE CriticalSection;

	Array<ObjectIndex_couple> *mem;
	void *(*Create)(UINT,void *);
	void *(*Replace)(void *,UINT,void *);
	void  (*Delete)(void *,UINT,void *);
	void *userdata;
	int policy,max_elements;
	
	
	int Current_Creation_Index;

public:
	ObjectContainer_class(void *(*Create)(UINT index,void *userdata),
						  void *(*Replace)(void *old_obj,UINT index,void *userdata),				// Destroy the old obj and create an new one, it can be NULL and it can only modify the data not the content
		                  void  (*Delete)(void *obj,UINT index,void *userdata),
						  void *userdata,
						  int policy,
						  int max_elements);
	~ObjectContainer_class();


	void *Get(UINT seach_index);				// Retrive the object
};








//
// Generic Object Class 
// (NOTE: the object should not contain pointers, the memory pointed by these pointers will not be stored)
//

class GenericObj {
public:
	// read-only
	void *obj;
	size_t size;

	// 
	GenericObj(void *obj,size_t size);					// Make a copy of obj
	~GenericObj();
	
	void copyTo(void *obj);
	void copyFrom(void *obj);
	void copyFrom(GenericObj *gobj);
};




//
// Chart Generator
// The first element of the list is the one with lowest score
//

class LowestScoreChart {
public:
	// read-only
	GenericObj **mem;
	double *score;
	int *index;
	int num_elements;


	LowestScoreChart(int num_elements,size_t obj_size);
	~LowestScoreChart();

	void insert(int obj_index,void *obj,double obj_score);
	void clear();
	void print();
};


