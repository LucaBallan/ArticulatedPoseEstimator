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





#include "../lib.h"



//// Segment V normalize
//// len = lunghezza segmento da 0
//bool SegmentMeshIntersect(HyperMesh<3> *M,Vector<3> V,Vector<3> O,double len) {
//	Vector<3> IP;
//	for(int i=0;i<M->num_f;i++) {
//		if (SegmentFaceIntersect(M->Faces[i],&M->Points,V,O,len,IP)) return true;
//	}
//	return false;
//}
//
//bool SegmentFaceIntersect(HyperFace<3> F,List<typename Space<3>::PointType> *Points,Vector<3> V,Vector<3> O,double len,Vector<3> &IntersectPoint) {
//	
//	double k,s,t;
//	Vector<3> PlaneNormal;
//
//	PlaneNormal=F.Normal(Points);
//	k=(*Points)[F.Point[0]]*PlaneNormal;
//
//	s=V*PlaneNormal;
//	
//	if (s==0) {
//		// V e il piano sono paralleli o PlaneNormal=0
//		if (O*PlaneNormal==k) {
//			// tutti i punti del segmento appartengono al piano
//			// alla faccia?
//		} else return false;
//	} else t=(k-O*PlaneNormal)/s;
//
//	if ((t>len) || (t<0)) return false;
//
//	IntersectPoint=V*t+O;
//
//	Vector<3> u=(*Points)[F.Point[1]]-(*Points)[F.Point[0]];
//	Vector<3> v=(*Points)[F.Point[2]]-(*Points)[F.Point[0]];
//
//	{
//	double uu, uv, vv, wu, wv, D, s , t;
//    uu = u*u;
//    uv = u*v;
//    vv = v*v;
//    Vector<3> w = IntersectPoint-(*Points)[F.Point[0]];
//    wu = w*u;
//    wv = w*v;
//    D = uv*uv-uu*vv;
//
//	s = (uv * wv - vv * wu) / D;
//    if ((s < 0.0) || (s > 1.0)) return false;
//    t = (uv * wu - uu * wv) / D;
//    if ((t < 0.0) || ((s + t) > 1.0)) return false;
//
//	}
//
//    return true;            
//
//	/*if (3==2) {
//		Vector<3> fo=(*Points)[[F.Point[0]]];
//		Vector<3> fv=(*Points)[[F.Point[1]]]-(*Points)[[F.Point[0]]];
//		double len_edge=fv.Norm2();
//		fv=fv.Versore();
//
//		if (len_edge==0) {
//			// un edge e' nullo
//			return false;		// per convenzione ma si dovrebbero fare i casi
//		}
//		
//		double f_t=(IntersectPoint-fo)*fv;
//
//		if ((f_t>len_edge) || (f_t<0)) return false;
//
//		return true;
//	}
//
//	if (dim==3) {
//		for(int i=0;i<3;i++) {
//			Vector<dim> fo=(*Points)[[F.Point[0]]];
//			Vector<dim> fv=(*Points)[[F.Point[1]]]-(*Points)[[F.Point[0]]];
//			double len_edge=fv.Norm2();
//			fv=fv.Versore();
//
//			if (len_edge==0) {
//				// un edge e' nullo
//				return false;		// per convenzione ma si dovrebbero fare i casi
//			}
//			
//			double f_t=(IntersectPoint-fo)*fv;
//
//			if ((f_t>len_edge) || (f_t<0)) return false;
//
//			return true;
//		}
//	}
//	
//	// per le altre dimensioni?????
//	return false;*/
//}
//
//
//
//// E' la piu' piccola HyperBox<2> che contiene tutti i pixel != da background
//// i punti ad esso appartenenti si determinano:
////			P0<=x<=P1
////
//
//bool GetBoundingBox(Image *img,BYTE background,HyperBox<2> &Box) {
//
//	HyperBox<2> Rect;
//
//	Rect.P0[0]=img->width;
//	Rect.P0[1]=img->height;
//	Rect.P1[0]=0;
//	Rect.P1[1]=0;
//
//	bool isempty=true;
//	bool inside_y=false;
//	for(UINT i=0;i<img->height;i++) {
//		bool inside=false;
//		bool finded=false;
//		for(UINT j=0;j<img->width;j++) {
//			if (img->Point(j,i)!=background) {           // ____XXXX__XXXX____
//				inside=true;
//				finded=true;
//				isempty=false;
//				if (Rect.P0[0]>j) Rect.P0[0]=j;
//			} else {
//				if (inside) {
//					inside=false;
//					if (Rect.P1[0]<j-1) Rect.P1[0]=j-1;
//				}
//			}
//		}
//		if (finded) {
//			inside_y=true;
//			if (i<Rect.P0[1]) Rect.P0[1]=i;
//		} else {
//			if (inside_y) {
//				inside_y=false;
//				if (Rect.P1[1]<i-1) Rect.P1[1]=i-1;
//			} 
//		}
//	}
//			
//	if (isempty) return false;
//	Box=Rect;
//	return true;
//}
//
//
//
//kdTree_Node<2> *DistanceMap(Image *img,BYTE background) {
//
//	Vector<2> point;
//	Array<typename Space<2>::PointType> P(100);
//		
//
//	for(int i=0;i<(int)img->height;i++) {
//		for(int j=0;j<(int)img->width;j++) {
//			if (img->Point(j,i)!=background) {
//				point[0]=j;point[1]=i;
//				P.append(point);
//			}
//		}
//	}
//
//	kdTree_Node<2> *root;
//	root=Build<2>(&P);
//	
//	return root;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//#define PICCO 0.6
//#define LEVEL 7
//#define NumPoints 60
//
//HyperMesh<2> *FindContour(Image *img) {
//	
//	Vector<2> d,point,d_min,neighbours;
//	HyperBox<2> Box;
//	OcTree_Node<2> *root;
//	HyperMesh<2> *Mesh;
//
//	d[0]=img->width;
//	d[1]=img->height;
//
//	root=new OcTree_Node<2>(d);
//
//	GetBoundingBox(img,0,Box);
//	
//	for(int j=(int)Box.P0[1];j<=(int)Box.P1[1];j++) {
//		for(int i=(int)Box.P0[0];i<=(int)Box.P1[0];i++) {
//			
//			if (img->Point(i,j)!=0) {
//				point[0]=i;point[1]=j;
//
//				OcTree_Node<2> *leaf=root->CreateNodeFromPoint(point,LEVEL);
//				leaf->f=PICCO;
//				
//				d_min=leaf->d;   // solo una volta basta
//			
//				//Crea i neighbours
//				for(int d_i=0;d_i<2;d_i++) {
//					neighbours=point;
//
//					neighbours[d_i]+=leaf->d[d_i];
//					root->CreateNodeFromPoint(neighbours,LEVEL);
//
//					neighbours[d_i]-=2*leaf->d[d_i];
//					root->CreateNodeFromPoint(neighbours,LEVEL);
//
//					neighbours[d_i]+=2*leaf->d[d_i];
//					root->CreateNodeFromPoint(neighbours,LEVEL);
//
//					neighbours[d_i]-=4*leaf->d[d_i];
//					root->CreateNodeFromPoint(neighbours,LEVEL);
//
//				}
//			
//
//			}
//
//		}
//	}
//
//	Mesh=Elipse(Box.Center(),Box.d()[0],Box.d()[1],3.1415/NumPoints);
//
//	root->Aggiorna_f_fromLeaf();
//	root->Create_Grad_Field();
//
//
//	// Se e' un img.. le derivate dipendono solo da PICCO
//	// la propagazione del gradiente dipende solo dalla sua entita' -> solo da PICCO
//	for (int i=0;i<20;i++) root->Itera_Propagazione_F(d_min);
//	for (int i=0;i<100;i++) root->OneStep_Mesh(Mesh);
//	
//	delete root;
//	return Mesh;
//}
//
//
//
//
//
//
//
//
//// CompositeSpace
//template <int dim>
//bool CS_less(Vector<dim> A,Vector<dim> B,int d) {
//	
//	if (A[d]<B[d]) return true;
//
//	if (A[d]==B[d]) {
//		for(int i=0;i<dim;i++) {
//			if ((d!=i) && (A[i]>=B[i])) return false;
//		}
//		return true;
//	}
//
//	return false;
//}
//
//// CompositeSpace
//template <int dim>
//bool CS_lessOrEquals(Vector<dim> A,Vector<dim> B,int d) {
//	
//	if (A[d]<=B[d]) return true;
//
//	if (A[d]==B[d]) {
//		for(int i=0;i<dim;i++) {
//			if ((d!=i) && (A[i]>=B[i])) return false;
//		}
//		return true;
//	}
//
//	return false;
//}
//
//
//
