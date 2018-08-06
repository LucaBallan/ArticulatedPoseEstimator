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
Tree *kdTree_Node<dim>::CreateNode(Tree *Father,int type) {
	return new kdTree_Node<dim>((kdTree_Node<dim> *)Father);
}


// ritorna kdTree_Node leaf associato al punto piu' vicino a P di distanza < dbest
// ritorna dbest, la distanza dal piu' vicino
// ritorna NULL se nn c'e' nulla < dbest, dbest rimane invariata

template <int dim>
kdTree_Node<dim>* kdTree_Node<dim>::RecNearestNeighbour(Vector<dim> P,double &dbest) {


	if (IsLeaf()) {
		double dist=(Point-P).Norm2();
		if (dist<dbest) {
			dbest=dist;
			return this;
		}
		return NULL;
	}

	kdTree_Node<dim> *nearer,*further,*Possible1,*Possible2;

	if (P[LineDim]<=Line) {
		nearer=(kdTree_Node*)Child[0];
		further=(kdTree_Node*)Child[1];
	} else {
		nearer=(kdTree_Node*)Child[1];
		further=(kdTree_Node*)Child[0];
	}
	
	Possible1=NULL;
	if (nearer!=NULL) Possible1=nearer->RecNearestNeighbour(P,dbest);

	Possible2=NULL;
	if (further!=NULL) {
		if (P[LineDim]<=Line) {
			if (P[LineDim]+dbest>Line) Possible2=further->RecNearestNeighbour(P,dbest);
		} else {
			if (P[LineDim]-dbest<=Line) Possible2=further->RecNearestNeighbour(P,dbest);
		}
	}

	if (Possible2!=NULL) return Possible2;
	if (Possible1!=NULL) return Possible1;

	return NULL;

}

template <int dim>
Vector<dim> kdTree_Node<dim>::NearestNeighbour(Vector<dim> P,double &d) {

	double dist=DBL_MAX;
	kdTree_Node<dim>* k=RecNearestNeighbour(P,dist);
	
	ASSERT(k!=NULL);

	d=dist;
	return k->Point;
}




// Ordina dal piu' da -infinito a +infinito la dimensione corrente
int order_dimension;

template <int dim>
int compare(const void*a,const void*b) {
	Vector<dim> *A=(Vector<dim> *)a;
	Vector<dim> *B=(Vector<dim> *)b;

	if ((*A)[order_dimension]==(*B)[order_dimension]) return 0;
	if ((*A)[order_dimension]<(*B)[order_dimension]) return -1;
	return 1;
}


template <int dim>
kdTree_Node<dim> *Build(Array<Vector<dim>> *P) {

	Array<Vector<dim>> *OrderedPoints[dim];
	for(int i=0;i<dim;i++) {
		OrderedPoints[i]=new Array<Vector<dim>>(*P);
		order_dimension=i;
		OrderedPoints[i]->sort(compare<dim>);
	}

	kdTree_Node<dim> *root=RecBuild<dim>(OrderedPoints,0);

	for(int i=0;i<dim;i++) delete OrderedPoints[i];
	
	return root;
}

#ifdef LIB_DEBUG
int max_dept=0;
#endif

// P[dim] Array di ugual dimensione 
template <int dim>
kdTree_Node<dim> *RecBuild(Array<Vector<dim>> *P[dim],int depth) {

	#ifdef LIB_DEBUG
		if (depth>max_dept) max_dept=depth;
	#endif
	
	int num=P[0]->numElements();
	ASSERT(num>0);

	int curr_dim=depth%dim;


	if (num==0) return NULL;

	if (num==1) {
		kdTree_Node<dim> *leaf=new kdTree_Node<dim>(NULL);
		leaf->Point=(*P[0])[0];
		return leaf;
	}

	// Selection in linear time
	Array<double> tmp(num);
	double prec=(*(P[curr_dim]))[0][curr_dim];
	tmp.append(prec);
	for(int i=1;i<num;i++) {
		if ((*(P[curr_dim]))[i][curr_dim]!=prec) {
			prec=(*(P[curr_dim]))[i][curr_dim];
			tmp.append(prec);
		}
	}
	double median=tmp[(tmp.numElements()-1)/2];

	// Split in linear time  -> Mantiene l'ordinamento -> avranno al max dimensione (num/2)+1
	
	Array<Vector<dim>> *OrderedPointsL[dim];
	Array<Vector<dim>> *OrderedPointsR[dim];
	for(int i=0;i<dim;i++) {
		OrderedPointsL[i]=new Array<Vector<dim>>((num/2)+1);
		OrderedPointsR[i]=new Array<Vector<dim>>((num/2)+1);
	}

	for(int i=0;i<num;i++) {
		for(int j=0;j<dim;j++) {
			if ((*P[j])[i][curr_dim]<=median) {
				OrderedPointsL[j]->append((*P[j])[i]);
			} else {
				OrderedPointsR[j]->append((*P[j])[i]);
			}
		}
	}

	// OrderedPointsL[i],OrderedPointsR[i] gia' ordinati

	kdTree_Node<dim> *left=RecBuild<dim>(OrderedPointsL,depth+1);
	kdTree_Node<dim> *right=RecBuild<dim>(OrderedPointsR,depth+1);

	for(int i=0;i<dim;i++) {
		delete OrderedPointsL[i];
		delete OrderedPointsR[i];
	}
	


	kdTree_Node<dim> *node=new kdTree_Node<dim>(NULL);
	node->Level=depth;

	node->AppendChild(left,0);
	node->AppendChild(right,1);

	node->LineDim=curr_dim;
	node->Line=median;

	return node;

}


/*
template <int dim>
void kdTree_Node<dim>::Draw(World<dim> *w) {
	if (IsLeaf()) {
		w->DrawPoint(Point,0xffffff);
	} else {
		Vector<dim> A,B;
		for(int i=0;i<dim;i++) A[i]=0;
		for(int i=0;i<dim;i++) B[i]=1000;
		A[LineDim]=Line;
		B[LineDim]=Line;
		w->DrawLine(A,B,BoxPen);
		
		if (Child[0]!=NULL) ((kdTree_Node*)Child[0])->Draw(w);
		if (Child[1]!=NULL) ((kdTree_Node*)Child[1])->Draw(w);
	}
}
*/
