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





template <int dim>
class kdTree_Node: public Tree {
private:
	virtual Tree *CreateNode(Tree *Father,int type);
	kdTree_Node<dim> *RecNearestNeighbour(Vector<dim> P,double &dbest);


public:

	Vector<dim> Point;
	double Line;
	int LineDim;
		

	kdTree_Node(kdTree_Node<dim> *Father) : Tree(2,Father) {};
	virtual ~kdTree_Node() {};
	
	// Ritorna il punto piu' vicino a P e la distanza da esso
	Vector<dim> NearestNeighbour(Vector<dim> P,double &d);
	
};


// Costruisce un Kd-Tree da un insieme di punti
template <int dim>
kdTree_Node<dim> *Build(Array<Vector<dim>> *P);
