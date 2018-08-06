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



ObjectContainer_class::ObjectContainer_class(void *(*Create)(UINT index,void *userdata),
											 void *(*Replace)(void *old_obj,UINT index,void *userdata),
											 void  (*Delete)(void *obj,UINT index,void *userdata),
											 void *userdata,
											 int policy,
											 int max_elements) {
	
	CriticalSection=CreateMutex(NULL,FALSE,NULL);

	mem=new Array<ObjectIndex_couple>(max_elements);
	this->Create=Create;
	this->Replace=Replace;
	this->Delete=Delete;
	this->userdata=userdata;
	this->policy=policy;
	this->max_elements=max_elements;

	Current_Creation_Index=0;
}

ObjectContainer_class::~ObjectContainer_class() {
	ENTER_C;
	int num_elements=mem->numElements();
	for(int i=0;i<num_elements;i++) Delete((*mem)[i].object,(*mem)[i].index,userdata);
	delete mem;
	mem=NULL;
	EXIT_C;

	CloseHandle(CriticalSection);
}

/*
void *ObjectContainer_class::Get(UINT seach_index) {

}
*/

void *ObjectContainer_class::Get(UINT seach_index) {
	ENTER_C;

	int num_elements;
	ObjectIndex_couple *element;


	// Look if the object exists
	num_elements=mem->numElements();
	element=mem->getMem();
	for(int i=0;i<num_elements;i++,element++) {
		if (element->index==seach_index) {
			element->n_accesses++;
			void *finded_obj=element->object;
			EXIT_C;
			return finded_obj;
		}
	}

	// Object does not exist
	// Insert a new one at the end of the list of whereever there is space (list_index_to_insert)
	int list_index_to_insert=-1;


	// Free some space if needed
	if (num_elements>=max_elements) {
		int list_index_to_delete;
		switch (policy) {
			case POLICY_FIFO:
				{
					// Find the first inserted (FIFO)
					int min_creation_i=INT_MAX;
					element=mem->getMem();
					for(int i=0;i<num_elements;i++,element++) 
						MIN_I(element->i_creation,min_creation_i,i,list_index_to_delete);
				}
				break;
			case POLICY_FILO:
				{
					// Find the last inserted (FILO)
					int max_creation_i=INT_MIN;
					element=mem->getMem();
					for(int i=0;i<num_elements;i++,element++) 
						MAX_I(element->i_creation,max_creation_i,i,list_index_to_delete);
				}
				break;
			case POLICY_BEST:
				{
					// Find the least accessed one
					int min_n_accesses=INT_MAX;
					element=mem->getMem();
					for(int i=0;i<num_elements;i++,element++) 
						MIN_I(element->n_accesses,min_n_accesses,i,list_index_to_delete);
				}
				break;
		}

		if (!Replace) Delete((((*mem)[list_index_to_delete]).object),(((*mem)[list_index_to_delete]).index),userdata);
		list_index_to_insert=list_index_to_delete;
	} 


	// the new object
	ObjectIndex_couple newobj;
	newobj.index=seach_index;
	newobj.n_accesses=1;				// TODO: mettere initial_weight invece di 1 se no gli ultimi escono subito!!
	newobj.i_creation=Current_Creation_Index;
	Current_Creation_Index++;


	// Insert the object into the list
	if (list_index_to_insert==-1) {
		newobj.object=Create(seach_index,userdata);
		mem->append(newobj);
	} else {
		if (!Replace) newobj.object=Create(seach_index,userdata);
		else newobj.object=Replace(((*mem)[list_index_to_insert]).object,seach_index,userdata);
		
		((*mem)[list_index_to_insert])=newobj;
	}

	EXIT_C;
	return newobj.object;
}











GenericObj::GenericObj(void *obj,size_t size) {
	this->size=size;
	this->obj=malloc(size);
	if (!(this->obj)) ErrorExit("Not enough memory");
	if (obj) memcpy(this->obj,obj,size);
}
GenericObj::~GenericObj() {
	if (obj) free(obj);
}
void GenericObj::copyFrom(void *obj) {
	if (obj) memcpy(this->obj,obj,size);
}
void GenericObj::copyFrom(GenericObj *gobj) {
	if (size!=gobj->size) {
		if (obj) free(obj);
		size=gobj->size;
		obj=malloc(size);
		if (!obj) ErrorExit("Not enough memory");
	}
	memcpy(obj,gobj->obj,size);
}
void GenericObj::copyTo(void *obj) {
	memcpy(obj,this->obj,size);
}



LowestScoreChart::LowestScoreChart(int num_elements,size_t obj_size) {
	this->num_elements=num_elements;
	mem=new GenericObj*[num_elements];
	for(int i=0;i<num_elements;i++) mem[i]=new GenericObj(NULL,obj_size);
	score=new double[num_elements];
	index=new int[num_elements];
	clear();
}
LowestScoreChart::~LowestScoreChart() {
	for(int i=0;i<num_elements;i++) delete mem[i];
	delete []mem;
	delete []score;
	delete []index;
}
void LowestScoreChart::insert(int obj_index,void *obj,double obj_score) {
	for(int i=0;i<num_elements;i++) {
		if (obj_score<=score[i]) {
			for(int j=num_elements-1;j>i;j--) {
				index[j]=index[j-1];
				score[j]=score[j-1];
				mem[j]->copyFrom(mem[j-1]);
			}
			index[i]=obj_index;
			score[i]=obj_score;
			mem[i]->copyFrom(obj);
			break;
		}
	}
}
void LowestScoreChart::print() {
	for(int i=0;i<num_elements;i++) {
		cout<<index[i]<<": "<<score[i]<<" ";
		for(size_t j=0;j<mem[i]->size;j++) cout<<((static_cast<char *>(mem[i]->obj))[j]);
		cout<<"\n";
	}
}
void LowestScoreChart::clear() {
	for(int i=0;i<num_elements;i++) score[i]=DBL_MAX;
}
