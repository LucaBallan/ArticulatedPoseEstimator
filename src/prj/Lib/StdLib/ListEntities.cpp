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




bool Pointer::operator== (const Pointer &v) const {
	if (v.pointer==pointer) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,Pointer &v) {
	os<<"0x"<<v.pointer;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Pointer &v) {
	os<<"0x"<<v.pointer;
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,Pointer &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}




bool ObjectIndex_couple::operator== (const ObjectIndex_couple &v) const {
	if (v.index==index) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,ObjectIndex_couple &v) {
	os<<"("<<v.index<<",0x"<<v.object<<")";
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,ObjectIndex_couple &v) {
	os<<"("<<v.index<<",0x"<<v.object<<")";
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,ObjectIndex_couple &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}




bool Task::operator== (const Task &v) const {
	if ((v.F==F) && (v.data==data)) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,Task &v) {
	os<<"Task (0x"<<v.F<<",0x"<<v.data<<")";
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Task &v) {
	os<<"Task (0x"<<v.F<<",0x"<<v.data<<")";
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,Task &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}





bool RepetitiveTask::operator== (const RepetitiveTask &v) const {
	if (v.task_id==task_id) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,RepetitiveTask &v) {
	os<<"RepetitiveTask ("<<v.task_id<<") class: "<<v.task_class<<", task: (0x"<<v.F<<",0x"<<v.data<<")";
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,RepetitiveTask &v) {
	os<<"RepetitiveTask ("<<v.task_id<<") class: "<<v.task_class<<", task: (0x"<<v.F<<",0x"<<v.data<<")";
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,RepetitiveTask &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}






bool RepetitiveTaskClassInfo::operator== (const RepetitiveTaskClassInfo &v) const {
	if (v.class_id==class_id) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,RepetitiveTaskClassInfo &v) {
	os<<"class ("<<v.class_id<<") priority: "<<v.priority<<(v.exclusive_execution?", exclusive":"");
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,RepetitiveTaskClassInfo &v) {
	os<<"class ("<<v.class_id<<") priority: "<<v.priority<<(v.exclusive_execution?", exclusive":"");
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,RepetitiveTaskClassInfo &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}






bool ordered_pair::operator== (const ordered_pair &v) const {
	if ((v.i==i) && (v.j==j)) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,ordered_pair &v) {
	os<<v.i<<" "<<v.j<<" ";
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,ordered_pair &v) {
	os<<"("<<v.i<<","<<v.j<<")";
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,ordered_pair &v) {
	os>>v.i;
	os>>v.j;
	return os;
}





weighted_index::weighted_index(int i,double w_1,double w_2,double w_3) {
	this->i=i;
	this->w_1=w_1;
	this->w_2=w_2;
	this->w_3=w_3;
}
bool weighted_index::operator== (const weighted_index &v) const {
	if (v.i==i) return true;
	return false;
}
OFileBuffer &operator<<(OFileBuffer &os,weighted_index &v) {
	os<<v.i<<" ("<<v.w_1<<","<<v.w_2<<","<<v.w_3<<") ";
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,weighted_index &v) {
	os<<v.i<<" ("<<v.w_1<<","<<v.w_2<<","<<v.w_3<<") ";
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,weighted_index &/*v*/) {
	NOT_IMPLEMENTED;
	return os;
}






template <class object_type>
bool object_pair<object_type>::operator== (const object_pair<object_type> &v) const {
	if ((v.i==i) && (v.j==j)) return true;
	return false;
}
template <class object_type>
OFileBuffer &operator<<(OFileBuffer &os,object_pair<object_type> &v) {
	os<<v.i<<" "<<v.j<<" ";
	return os;
}
template <class object_type>
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,object_pair<object_type> &v) {
	os<<"("<<v.i<<","<<v.j<<")";
	return os;
}
template <class object_type>
IFileBuffer &operator>>(IFileBuffer &os,object_pair<object_type> &v) {
	os>>v.i;
	os>>v.j;
	return os;
}







template <>
int sort_increasing_compare<double>(const void *a,const void *b) {
	double a_=*((double*)a);
	double b_=*((double*)b);
	
	// a_   b_
	if (a_==DBL_MAX) {
		if (b_==DBL_MAX) return 0;
		return +1;
	}
	if (b_==DBL_MAX) return -1;
	if (a_==-DBL_MAX) {
		if (b_==-DBL_MAX) return 0;
		return -1;
	}
	if (b_==-DBL_MAX) return +1;

	if (a_==b_) return 0;
	return ((a_<b_)?-1:+1);
}

template <>
int sort_increasing_compare<float>(const void *a,const void *b) {
	float a_=*((float*)a);
	float b_=*((float*)b);
	
	// a_   b_
	if (a_==FLT_MAX) {
		if (b_==FLT_MAX) return 0;
		return +1;
	}
	if (b_==FLT_MAX) return -1;
	if (a_==-FLT_MAX) {
		if (b_==-FLT_MAX) return 0;
		return -1;
	}
	if (b_==-FLT_MAX) return +1;

	if (a_==b_) return 0;
	return ((a_<b_)?-1:+1);
}

template <>
int sort_increasing_compare<int>(const void *a,const void *b) {
	return (int)(((*(int*)a)-(*(int*)b)));								// attenzione!!! funziona solo con int
}











