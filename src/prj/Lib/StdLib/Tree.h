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





#define TREE_INVALID_NODE 0xFFFFFF

class Tree {
protected:
	int num_child;
	virtual Tree *CreateNode(Tree *Father,int type);

public:
	Tree *Father;
	Tree **Child;
	int Level;																	// Not recursively updated by append_child
	

	Tree(int num_child,Tree *Father);
	virtual ~Tree();
	
	bool IsLeaf();
	int  GetType();
	int  GetNumChild();
	void ExpanNode();
	bool IsChildOf(Tree *hypothetical_father);
	
	bool AddChild(Tree *child);
	bool DetachChild(Tree *child);
	bool AppendChild(Tree *child,int type);
	
	bool Traversal(bool(*F)(Tree*,void*,void*),void *In,void *Out);				// root to leaf traversal (Depth-first)
																				// F return true to terminate else it return false
																				// Traversal return true if F has saied true else it return false
	bool Reverse_Traversal(bool(*F)(Tree*,void*,void*),void *In,void *Out);		// leaf to root traversal (reverse Depth-first) (to test!!)

	Array<Tree*> *Enumerate();													// distruggere l'array ritornato -> x=Enumerate(); ...; delete x;

	void UpdateTreeLevels(int initial_level=0);									// correct all the levels value in the tree (da lanciare dal root, initial_level sara' il livello di root)
};
OFileBuffer &operator<<(OFileBuffer &os,Tree *v);
IFileBuffer &operator>>(IFileBuffer &os,Tree *v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Tree *v);
