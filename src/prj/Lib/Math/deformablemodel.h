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





//***************************************************************************************
//*************************** Deformable Models *****************************************
//***************************************************************************************

template <int dim>
class DeformableModel {
protected:	
	HyperMesh<dim> *Mesh;
	OcTree_Node<dim> *ForceField;
	kdTree_Node<dim> *DistanceMap;

public:
	// Flow
	List<typename Space<dim>::PointType> F_ext;
	List<typename Space<dim>::PointType> F_int;

	double Rigidity;
	double Elasticity;
	double InternalMagnitude;

	bool Integrate;
	
	DeformableModel(HyperMesh<dim> *Mesh,OcTree_Node<dim> *ForceField,kdTree_Node<dim> *DistanceMap);
	~DeformableModel();

	virtual void CalculateFlow();

	void UntilStability();
	double ConvergenceError();
};








//***************************************************************************************

//***************************************************************************************
//******************** Deformable Models Using Silhuette ********************************
//***************************************************************************************

//***************************************************************************************



// = 0 e' il background
// !=0 e' un'immagine

class View {
public:	
	Matrix P;
	Bitmap<ColorRGB> *Immagine;
	Image *Mask;
	kdTree_Node<2> *DistanceMap_FromRealSilhouette;
	Image *Rendered;

	// Unprojection
	Matrix InvM;
	Vector<3> O;
	Vector<3> PrjPlaneNormal;
	
	Matrix depthM;
	Vector<3> depthOri;
	bool zrendered;				// false non renderizzato
	ZBuffer *Depht;				// NULL non è stato creato

	// 
	bool enable;

	static void InitRender(int w,int h);

	View() : enable(true), P(3,4), InvM(3,3),depthM(3,3), Immagine(NULL), Mask(NULL), Rendered(NULL), DistanceMap_FromRealSilhouette(NULL),Depht(NULL),zrendered(false) {};
	~View();

	void CreateUnProjectionMatrix();
	Vector<2> Projection(Vector<3> x);
	Vector<3> UnProjection(Vector<2> x);
	Vector<3> Trasform(Vector<3> x);
	Vector<3> UnTrasform(Vector<3> x);




	double DistanceFromRealSilhouette(Vector<2> P,Vector<2> &Target);
															// negativo sono fuori dalla Silhouette
															// positivo sono dentro la Silhouette

	double DistanceFromSnakeEdge(Vector<2> P);				// sempre >=0 

};

class Shadow {
public:
	int View;
	Image *ShadowMap;
	Matrix M;
	Vector<3> O;
	
	ZBuffer *zbuffer;

	Shadow() : M(3,3) {};
};



class DeformableModel_US: public DeformableModel<3> {
	View *Views;
	int num_views;
	Shadow *Shadows;
	int num_shadows;
	void Setup_All_Views();

	double MeshRadius;
	
	double DistanceFromSilhuette(Vector<3> x,int &min_view,Vector<2> &NearTarget);
//	double Computa_Alpha(Vector<3> x);
	Vector<3> DeformableModel_US::Computa_F_Sil(int i);


public:
	double SilhuetteMagnitude;
	double ShadowMagnitude;

	// Flow
	List<Space<3>::PointType> F_sil;
	List<Space<3>::PointType> F_shadow;
	

	DeformableModel_US(HyperMesh<3> *Mesh,OcTree_Node<3> *ForceField,kdTree_Node<3> *DistanceMap,View *Views,int num_views,Shadow *Shadows=NULL,int num_shadows=0);
	~DeformableModel_US();

	virtual bool CalculateFlow(bool ComputaSilhuette=true,bool Mean_Curvature=false);
};

































/*
class ImageSnake:public DeformableModel<2> {
	
	ImageSnake(Image *edgeImg);

}
ImageSnake::ImageSnake(Image *edgeImg) : DeformableModel(new {
}
*/





// f(x) = v             se c'e' il dato
//      = Infinite      se il dato nn c'e'



template <int dim>
class FunctionSmoother {
	HyperMesh<dim+1> *Mesh;
	double (*f) (Vector<dim> x);

public:
	FunctionSmoother(HyperMesh<dim+1> *Mesh,double(*f)(Vector<dim> x));
	~FunctionSmoother() {};

	void OneStep();
/*	void Flow();

	void UntilStability();
	double ConvergenceError();*/
};






