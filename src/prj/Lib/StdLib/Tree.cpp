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





#include "../stdInclude.h"
#include "../common.h"
#include "../StdLib/IObuffer.h"
using namespace std;
#include "ListEntities.h"
#include "List.h"
#include "Tree.h"
#pragma warning(3:4244)

Tree::Tree(int num_child,Tree *Father) {
	// Init Tree
	this->Father=Father;
	this->num_child=num_child;

	if (Father==NULL) Level=0;
	else Level=Father->Level+1;

	Child=new Tree*[num_child];
	for(int i=0;i<num_child;i++) Child[i]=NULL;
}
Tree::~Tree() {
	for(int i=0;i<num_child;i++) {
		if (Child[i]!=NULL) delete Child[i];
	}
	delete []Child;
}


Tree *Tree::CreateNode(Tree *Father,int type) {
	// Do what u want...
	return new Tree(num_child,Father);
}	


bool Tree::IsLeaf() {
	return (Child[0]==NULL);
}

int Tree::GetNumChild() {
	return num_child;
}

int Tree::GetType() {
	if (Father==NULL) return TREE_INVALID_NODE;
	for(int i=0;i<num_child;i++) {
		if (Father->Child[i]==this) return i;
	}
	return TREE_INVALID_NODE;
}

void Tree::ExpanNode() {

	if (!IsLeaf()) return;

	for(int i=0;i<num_child;i++) 
		Child[i]=CreateNode(this,i);
}

bool Tree::AddChild(Tree *child) {
	for(int i=0;i<num_child;i++) 
		if (Child[i]==child) return false;

	for(int i=0;i<num_child;i++) 
		if (Child[i]==NULL) return AppendChild(child,i);

	return false;
}

bool Tree::DetachChild(Tree *child) {
	int i;
	for(i=0;i<num_child;i++) 
		if (Child[i]==child) break;
	
	if (i==num_child) return false;

	Child[i]=NULL;
	for(int j=i+1;j<num_child;j++) Child[j-1]=Child[j];
	Child[num_child-1]=NULL;

	return true;
}

bool Tree::AppendChild(Tree *child,int type) {
	ASSERT(type<num_child);
	
	if 	(Child[type]!=NULL) return false;
	
	if (child==NULL) return true;

	child->Father=this;
	child->Level=Level+1;
	Child[type]=child;
	return true;
}

bool Tree::IsChildOf(Tree *hypothetical_father) {
	if (Father==hypothetical_father) return true;
	if (Father==NULL) return false;
	return Father->IsChildOf(hypothetical_father);
}

bool Tree::Traversal(bool(*F)(Tree*,void*,void*),void *In,void *Out) {
	if (F(this,In,Out)) return true;
	if (IsLeaf()) return false;
	
	for(int i=0;i<num_child;i++) {
		if (Child[i]!=NULL) {
			if (Child[i]->Traversal(F,In,Out)) return true;
		}
	}

	return false;
}

bool Tree::Reverse_Traversal(bool(*F)(Tree*,void*,void*),void *In,void *Out) {
	if (IsLeaf()) {
		if (F(this,In,Out)) return true;
		return false;
	}
	
	for(int i=0;i<num_child;i++) {
		if (Child[i]!=NULL) {
			if (Child[i]->Reverse_Traversal(F,In,Out)) return true;
		}
	}

	if (F(this,In,Out)) return true;
	return false;
}

bool Enumerate_Fnc(Tree *x,void *c_,void*) {
	Array<Tree*> *c=static_cast<Array<Tree*> *>(c_);
	
	c->append(x);
	return false;
}

Array<Tree*> *Tree::Enumerate() {
	Array<Tree*> *collection=new Array<Tree*>(10);

	Traversal(Enumerate_Fnc,collection,NULL);

	return collection;
}


OFileBuffer &operator<<(OFileBuffer &os,Tree *v) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Tree *v) {
	NOT_IMPLEMENTED;
	return os;
}
IFileBuffer &operator>>(IFileBuffer &os,Tree *v) {
	NOT_IMPLEMENTED;
	return os;
}


void Tree::UpdateTreeLevels(int initial_level) {
	Level=initial_level;

	initial_level++;
	for(int i=0;i<num_child;i++) {
		if (Child[i]!=NULL) Child[i]->UpdateTreeLevels(initial_level);
	}
}
