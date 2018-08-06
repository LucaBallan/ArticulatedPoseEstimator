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


Features::Features(char *filename,int sync_frame0) : feat_list(100) {
	this->sync_frame0=sync_frame0;
	IFileBuffer In(filename);

	feat_list.clear();
	if (In.IsFinished()) return;

	char *ext=Get_File_Extension(filename);
	if (ext==NULL) ErrorExit("Cannot understand the feature file.");
	if (!_stricmp(ext,"FEAT")) {
		int     len;
		__int32 tmp;
		float   tmp_f;
		while (!In.IsFinished()) {
			feat_element_type *x=new feat_element_type();
			len=4;In.ReadLine((char*)&tmp,len);x->start=tmp;
			len=4;In.ReadLine((char*)&tmp,len);x->end=tmp;
			x->x=new double[x->end-x->start+1];
			x->y=new double[x->end-x->start+1];
			
			for(int i=0;i<x->end-x->start+1;i++) {
				len=4;In.ReadLine((char*)&tmp_f,len);x->x[i]=tmp_f;
				len=4;In.ReadLine((char*)&tmp_f,len);x->y[i]=tmp_f;
			}
			feat_list.append(x);
		}
	} else {
		while (!In.IsFinished()) {
			feat_element_type *x=new feat_element_type();	// TODO brutto come ottimizzazione di memoria, delete manca pure
			In>>x->start;
			In>>x->end;
			x->x=new double[x->end-x->start+1];
			x->y=new double[x->end-x->start+1];
			
			for(int i=0;i<x->end-x->start+1;i++) {
				In>>x->x[i];
				In>>x->y[i];
			}
			feat_list.append(x);
		}
	}
}

void Features::GetCommonFeatures(int a,int b,Array<typename Vector<2>> *out_a,Array<typename Vector<2>> *out_b) {
	a+=sync_frame0;
	b+=sync_frame0;
	int ra,rb;

	if (a>b) {
		ra=b;
		rb=a;
	} else {
		ra=a;
		rb=b;
	}
	//cout<<"Selecting Common Features from "<<ra<<" to "<<rb<<"\n";

	out_a->clear();
	out_b->clear();

	for(int i=0;i<feat_list.numElements();i++) {
		if ((feat_list[i]->start<=ra) && (feat_list[i]->end>=rb)) {
			Vector<2> x;
			x[0]=feat_list[i]->x[a-feat_list[i]->start];
			x[1]=feat_list[i]->y[a-feat_list[i]->start];
			out_a->append(x);
			x[0]=feat_list[i]->x[b-feat_list[i]->start];
			x[1]=feat_list[i]->y[b-feat_list[i]->start];
			out_b->append(x);
		}
	}
}



OFileBuffer &operator<<(OFileBuffer &os,feat_element_type *v) {
	NOT_IMPLEMENTED;
}
IFileBuffer &operator>>(IFileBuffer &os,feat_element_type *v) {
	NOT_IMPLEMENTED;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,feat_element_type *v) {
	NOT_IMPLEMENTED;
}
