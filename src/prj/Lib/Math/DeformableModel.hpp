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





#include "DeformableModel.h"



//***************************************************************************************

//***************************************************************************************
//*************************** Deformable Models *****************************************
//***************************************************************************************

//***************************************************************************************



template <int dim>
DeformableModel<dim>::DeformableModel(HyperMesh<dim> *Mesh,OcTree_Node<dim> *ForceField,kdTree_Node<dim> *DistanceMap) : F_ext(INITIAL_POINT_NUMBER),F_int(INITIAL_POINT_NUMBER) {
	this->Mesh=Mesh;
	this->ForceField=ForceField;
	this->DistanceMap=DistanceMap;
	Rigidity=0.9;
	Elasticity=0;
	InternalMagnitude=1;
	Integrate=false;
}

template <int dim>
DeformableModel<dim>::~DeformableModel() {
}


#define EPOCH_TIME 100
template <int dim>
void DeformableModel<dim>::UntilStability() {
	
	double er;
	Integrate=true;

	do {
		for (int i=0;i<EPOCH_TIME;i++) CalculateFlow();
		er=ConvergenceError();
	} while (er>=6);

}


// Distanza Media tra i punti della mesh e i punti in convergenza

// Problemi:		- Dipende dall'unita' di misura
//					- Vi possono esser buchi nella definizione di distance map (manca info -> interpola)
//					- La mesh puo' avere punti di cui nn si vuole tener conto (spuri)

template <int dim>
double DeformableModel<dim>::ConvergenceError() {
	
	double error=0,d;
	for(int i=0;i<Mesh->num_p;i++) {
		DistanceMap->NearestNeighbour(Mesh->Points[i],d);
		error+=d;
	}
	error/=Mesh->num_p;

	return error;
}




template <int dim>
void DeformableModel<dim>::CalculateFlow() {
	Vector<dim> Laplacian;
	Vector<dim> Biharmonic;

	// Aggiorna Stato Mesh
	Mesh->ComputaNormals();
	Mesh->ComputaLaplacians();
	Mesh->ComputaBiharmonics();
	
	double average_stress=Mesh->AverageStress();		// NN e' meglio il minimo stress????
														// E' MEGLIO IN BASE AL MASSIMO... VEDI SFERA cosi' siam sicuri della convergenza
	double Max_curvature=Mesh->MaxCurvature();
	double Max_curvature_variation=Mesh->MaxCurvatureVariation();
	if (Max_curvature==0) Max_curvature=1;
	if (Max_curvature_variation==0) Max_curvature_variation=1;


	for(int i=0;i<Mesh->num_p;i++) {
	
		
		//F_ext=F_Interpolate(Mesh->Points[i]);							// Nota: solo per piccoli??
		OcTree_Node<dim> *L=ForceField->NodeFromPoint(Mesh->Points[i]);

		F_ext[i]=L->F;
		F_ext[i]=F_ext[i].Versore();

		F_ext[i]=(F_ext[i]*Mesh->Normals[i])*Mesh->Normals[i];
		
		// F_ext ha norma <=1             (il fatto che sia <1 nn ha molto senso)
		

		Laplacian=Mesh->Laplacians[i];
		Biharmonic=Mesh->Biharmonics[i];
	
		F_int[i]=((Rigidity*(1/Max_curvature)*Laplacian)-(Elasticity*(1/Max_curvature_variation)*Biharmonic));
				
		// Sicuramente < di ( bisonga mettere il max)


		// Fissati F_int e F_ext -> e resi paragonabili 
		
		// -> devo muovermi di uno step proporzionale all'area? o alla distanza media tra gli edge...
		// in modo da assicurare la convergenza
		
		if (Integrate) {
			Vector<dim> D=(average_stress/20)*(F_ext[i]+InternalMagnitude*F_int[i]);
			Mesh->Points[i]=Mesh->Points[i]+D;
		} 
	}
}





















//***************************************************************************************

//***************************************************************************************
//******************** Deformable Models Using Silhuette ********************************
//***************************************************************************************

//***************************************************************************************



// ----------------------------- 
// ------ Virtual Renderer
// ----------------------------- 

World_offscreen<3> *VirtualRenderer;
World_DepthRender *ZRenderer;
HyperMesh<3> *Current_Mesh;

void disp_virtual_renderer() {
	VirtualRenderer->DrawFillHyperMesh(Current_Mesh,0x808080);
}




// ----------------------------- 
// -------- View Code
// ----------------------------- 

void View::InitRender(int w,int h) {
	VirtualRenderer=new World_offscreen<3>(disp_virtual_renderer,w,h);
	ZRenderer=new World_DepthRender(w,h);
}

View::~View() {
	if (Immagine!=NULL) delete Immagine;
	if (Mask!=NULL) delete Mask;
	if (Rendered!=NULL) delete Rendered;
	if (DistanceMap_FromRealSilhouette!=NULL) delete DistanceMap_FromRealSilhouette;
}

void View::CreateUnProjectionMatrix() {
	InvM[0][0]=P[0][0];
	InvM[0][1]=P[0][1];
	InvM[0][2]=P[0][2];
	InvM[1][0]=P[1][0];
	InvM[1][1]=P[1][1];
	InvM[1][2]=P[1][2];
	InvM[2][0]=P[2][0];
	InvM[2][1]=P[2][1];
	InvM[2][2]=P[2][2];
	O[0]=P[0][3];
	O[1]=P[1][3];
	O[2]=P[2][3];
	
	InvM.Inversion();

	Vector<3> g0,g1;
	InvM.GetColumn(0,g0.GetList());
	InvM.GetColumn(1,g1.GetList());

	PrjPlaneNormal=(g0^g1).Versore();
}

Vector<3> View::UnProjection(Vector<2> x) {
	Vector<3> tmp;
	Vector<3> Unprj;

	tmp[0]=x[0];
	tmp[1]=x[1];
	tmp[2]=1;

	Multiply(&InvM,tmp-O,Unprj.GetList());

	return Unprj;
}


Vector<3> View::UnTrasform(Vector<3> x) {
	Vector<3> UnTr;

	Multiply(&InvM,x-O,UnTr.GetList());

	return UnTr;
}

Vector<2> View::Projection(Vector<3> x) {
	Vector<2> point;

	Vector<3> tmp=Trasform(x);

	point[0]=tmp[0]/tmp[2];
	point[1]=tmp[1]/tmp[2];
	return point;
}

Vector<3> View::Trasform(Vector<3> x) {
	Vector<3> tmp;
	Vector<4> omo;
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1;

	Multiply(&P,omo,tmp.GetList());
	
	return tmp;
}

double View::DistanceFromRealSilhouette(Vector<2> P,Vector<2> &Target) {
	double dist;
	
	Target=DistanceMap_FromRealSilhouette->NearestNeighbour(P,dist);
	
	if (Mask->Point(P[0],P[1])==0) return -dist;
	return dist;
}

bool Nearest_Bkg_Pixel(Image *Rendered,int x,int y,int r) {
	
	for(int i=-r;i<=r;i++) 
		if (Rendered->Point(x+i,y-r)==0) return true;

	for(int i=-r;i<=r;i++) 
		if (Rendered->Point(x+i,y+r)==0) return true;
	
	for(int i=-r+1;i<r;i++) 
		if (Rendered->Point(x+r,y+i)==0) return true;
		
	for(int i=-r+1;i<r;i++) 
		if (Rendered->Point(x-r,y+i)==0) return true;

	return false;
}


#define MAX_R 10
// 0 contorno
// > 0 interno  al massimo e' MAX_R
double View::DistanceFromSnakeEdge(Vector<2> P) {
	
	for(int r=1;r<MAX_R;r++) {
		if (Nearest_Bkg_Pixel(Rendered,P[0],P[1],r)) return r-1;
	}	
	
	return MAX_R;
}




// ------------------------------- 
// -------- Deformable Model
// ------------------------------- 


DeformableModel_US::DeformableModel_US(HyperMesh<3> *Mesh,OcTree_Node<3> *ForceField,kdTree_Node<3> *DistanceMap,View *Views,int num_views,Shadow *Shadows,int num_shadows) : DeformableModel<3>(Mesh,ForceField,DistanceMap), F_sil(INITIAL_POINT_NUMBER), F_shadow(INITIAL_POINT_NUMBER) {
	this->num_views=num_views;
	this->Views=Views;
	this->Shadows=Shadows;
	this->num_shadows=num_shadows;
	SilhuetteMagnitude=1;
	ShadowMagnitude=1;
	MeshRadius=Mesh->GetRadius(Mesh->GetCenter());
}

DeformableModel_US::~DeformableModel_US() {
}

bool DeformableModel_US::CalculateFlow(bool ComputaSilhuette,bool Mean_Curvature) {
	Vector<3> Laplacian;
	Vector<3> Biharmonic;

	// Aggiorna Stato Mesh
	Mesh->ComputaNormals();
	Mesh->ComputaLaplacians();
	Mesh->ComputaBiharmonics();
	if (Mean_Curvature) {
		Mesh->ComputaArea();
		Mesh->ComputaCurvatureNormal();
	}
	
	double average_stress;
	if (Mean_Curvature) {
		average_stress=Mesh->MinStress();
	} else {
		// data l'azione dei laplaciani <- si puo' usare il medio -> piu' veloce
		average_stress=Mesh->AverageStress();		// NN e' meglio il minimo stress????
													// E' MEGLIO IN BASE AL MASSIMO... VEDI SFERA cosi' siam sicuri della convergenza
	}	
	double Max_curvature=Mesh->MaxCurvature();
	double Max_curvature_variation=Mesh->MaxCurvatureVariation();
	double Max_mean_curvature=Mesh->MaxMeanCurvature();
	if (Max_curvature==0) Max_curvature=1;
	if (Max_curvature_variation==0) Max_curvature_variation=1;
	if (Max_mean_curvature==0) Max_mean_curvature=1;

	// Set Render
	if (ComputaSilhuette) Setup_All_Views();


	for(int i=0;i<Mesh->num_p;i++) {
	
		
		F_ext[i]=ForceField->F_Interpolate(Mesh->Points[i]);							// Nota: solo per piccoli??
		OcTree_Node<3> *L=ForceField->NodeFromPoint(Mesh->Points[i]);

		F_ext[i]=F_ext[i].Versore();

		F_ext[i]=(F_ext[i]*Mesh->Normals[i])*Mesh->Normals[i];
		
		// F_ext ha norma <=1             (il fatto che sia <1 nn ha molto senso)
		

		Laplacian=Mesh->Laplacians[i];
		Biharmonic=Mesh->Biharmonics[i];
	
		if (Mean_Curvature) {
			F_int[i]=Mesh->CurvatureNormals[i]*(1/Max_mean_curvature);
			// Normalize the direction
			F_int[i]=(F_int[i]*Mesh->Normals[i])*Mesh->Normals[i];
		} else {
			F_int[i]=((Rigidity*(1/Max_curvature)*Laplacian)-(Elasticity*(1/Max_curvature_variation)*Biharmonic));
		}

		// Sicuramente < di ( bisonga mettere il max)

		// Fissati F_int e F_ext -> e resi paragonabili 
		

		// Ora calcolo F_Silhuette
		if (ComputaSilhuette) {
			F_sil[i]=Computa_F_Sil(i);
		}

		// Computa F_shadow
		if (ComputaSilhuette) {
			F_shadow[i]=ZERO_3;
			for(int s=0;s<num_shadows;s++) {
				Vector<2> R=Views[Shadows[s].View].Projection(Mesh->Points[i]);
				
				if (Shadows[s].ShadowMap->Point(R[0],R[1])!=0) continue;
				if (!Views[Shadows[s].View].Depht->isVisible(Mesh->Points[i])) continue;

				if (!Shadows[s].zbuffer->isVisible(Mesh->Points[i])) continue;

				F_shadow[i]+=(-1)*Mesh->Normals[i];
			}
			F_shadow[i]=F_shadow[i].Versore();
		}

		// PARAGONABILE??????



		// -> devo muovermi di uno step proporzionale all'area? o alla distanza media tra gli edge...
		// in modo da assicurare la convergenza
		
		if (Integrate) {// 2 e 1.5
			Vector<3> D=(average_stress/20)*(1.5*F_ext[i]+1*Mesh->Normals[i]+InternalMagnitude*F_int[i]+SilhuetteMagnitude*F_sil[i]+ShadowMagnitude*F_shadow[i]);

			if (!D.isFinite()) {
				printf("Problem: Point %i wants to diverge to infinity!\n",i);
				return false;
			}

			Mesh->Points[i]=Mesh->Points[i]+D;

			if (!Mesh->Points[i].isFinite()) {
				printf("Problem: Point %i diverge to infinity!\n",i);
				return false;
			}
		} 
	}
	
	return true;
}


void DeformableModel_US::Setup_All_Views() {
	Current_Mesh=Mesh;
	printf("-> Start Renderer\n");
	for(int i=0;i<num_views;i++) {
		VirtualRenderer->LockInMatrix(&Views[i].P);
		VirtualRenderer->ReadBuffer(Views[i].Rendered->getBuffer());
		Views[i].zrendered=false;
	}	
	for(int i=0;i<num_shadows;i++) {
		if (!Views[Shadows[i].View].zrendered) {
			
			ZRenderer->SetView(&Views[Shadows[i].View].depthM,Views[Shadows[i].View].depthOri,Current_Mesh);
			ZRenderer->ReadBuffer(NULL,Views[Shadows[i].View].Depht);
			Views[Shadows[i].View].zrendered=true;
		}

		ZRenderer->SetView(&Shadows[i].M,Shadows[i].O,Current_Mesh);
		ZRenderer->ReadBuffer(NULL,Shadows[i].zbuffer);
	}	
	printf("-> End Render\n");
}


#define SILHUETTE_MULTIPLIER 10
#define SMOOTH_FORCE 3

Vector<3> DeformableModel_US::Computa_F_Sil(int i) {
	
	Vector<3> x=Mesh->Points[i];

	int sel_view;
	Vector<2> NearTarget;
	double Distance_From_Visual_Hull=DistanceFromSilhuette(x,sel_view,NearTarget);
	if (sel_view==-1) {
		Vector<3> ZERO;
		return ZERO;
	}

	double k=Views[sel_view].DistanceFromSnakeEdge(Views[sel_view].Projection(x));
	if (k>=SMOOTH_FORCE) {
		Vector<3> ZERO;
		return ZERO;
	}

	Vector<3> P_tr=Views[sel_view].Trasform(x);
	
	Vector<3> T_NearTarget;
	T_NearTarget[0]=NearTarget[0]*P_tr[2];
	T_NearTarget[1]=NearTarget[1]*P_tr[2];
	T_NearTarget[2]=P_tr[2];

	Vector<3> UT_NearTarget=Views[sel_view].UnTrasform(T_NearTarget);


	Vector<3> F=(UT_NearTarget-x);
	double real_distance=F.Norm2();
	
	F=F.Projection(Mesh->Normals[i]);
	F=F.Versore();
	
	// real_distance/MeshRadius = % rispetto al raggio da 0 a 1
	double distance2radius_ratio=real_distance/MeshRadius;
	
	// Limita a 1 il vettore
	if (distance2radius_ratio>1.0/SILHUETTE_MULTIPLIER) distance2radius_ratio=1.0/SILHUETTE_MULTIPLIER;

	F*=SILHUETTE_MULTIPLIER*distance2radius_ratio;			// 1/SILHUETTE_MULTIPLIER sara 1
	
	F*=(1-k/SMOOTH_FORCE);										
	
	return F;
}


// Definita come la minima distanza tra il punto della mesh e le silhuette delle varie viste
// se ve ne e' una dove sono fuori dalla silhuette allora la dist sara' negativa
// se son tutti dentro -> la dist sara' la distanza della silhuette piu' vicina

double DeformableModel_US::DistanceFromSilhuette(Vector<3> x,int &min_view,Vector<2> &NearTarget) {
	Vector<2> TmpTarget;
	double min_dist,tmp;

	int j;
	for(j=0;j<num_views;j++) {
		if (Views[j].enable) {
			min_view=j;
			min_dist=Views[j].DistanceFromRealSilhouette(Views[j].Projection(x),NearTarget);
			break;
		}
	}
	if (j==num_views) {
		min_view=-1;			// Nessuna vista attiva
		return 0;
	}
	
	j++;
	for(;j<num_views;j++) {
		if (!Views[j].enable) continue;
		tmp=Views[j].DistanceFromRealSilhouette(Views[j].Projection(x),TmpTarget);
		if (tmp<min_dist) {
			NearTarget=TmpTarget;
			min_dist=tmp;
			min_view=j;
		}
	}
	
	return min_dist;
}














/*
double DeformableModel_US::DistanceFromSilhuette(Vector<3> x,int &min_view,Vector<2> &NearTarget) {
	Vector<2> TmpTarget;
	double min_dist,tmp;

	min_view=0;
	min_dist=Views[0].DistanceFromRealSilhouette(Views[0].Projection(x),NearTarget);
	
	for(int j=1;j<num_views;j++) {
		tmp=Views[j].DistanceFromRealSilhouette(Views[j].Projection(x),TmpTarget);
		if (tmp<min_dist) {
			NearTarget=TmpTarget;
			min_dist=tmp;
			min_view=j;
		}
	}
	
	return min_dist;
}


int c_view;
	double Distance_From_Visual_Hull=DistanceFromSilhuette(x,c_view);

	if (Distance_From_Visual_Hull<=0) return Distance_From_Visual_Hull;

	double k=(1+Views[c_view].DistanceFromSnakeEdge(Views[c_view].Projection(x)));
	for(int i=1;i<P_Parameter;i++) k*=k;
	
	k=Distance_From_Visual_Hull/k;

	return k;
}

#define SILHUETTE_MULTIPLIER 100
#define SMOOTH_FORCE 3

Vector<3> DeformableModel_US::Computa_F_Sil(int i,int j) {
	
	Vector<3> x=Mesh->Points[i];
	Vector<2> p=Views[j].Projection(x);


	Vector<2> NearTarget;
	double Distance_From_Visual_Hull=Views[j].DistanceFromRealSilhouette(p,NearTarget);
	
	

	double k=Views[j].DistanceFromSnakeEdge(p);
	if (k>=SMOOTH_FORCE) {
		Vector<3> ZERO;
		return ZERO;
	}

	Vector<3> P_tr=Views[j].Trasform(x);
	
	Vector<3> T_NearTarget;
	T_NearTarget[0]=NearTarget[0]*P_tr[2];
	T_NearTarget[1]=NearTarget[1]*P_tr[2];
	T_NearTarget[2]=P_tr[2];

	Vector<3> UT_NearTarget=Views[j].UnTrasform(T_NearTarget);


	Vector<3> F=(UT_NearTarget-x);
	double real_distance=F.Norm2();
	//F=F.Projection(Mesh->Normals[i]);
	//F=F.Versore();
	
	//if real_distance>
	F*=(1.0*SILHUETTE_MULTIPLIER)/MeshRadius;					// real_distance/MeshRadius = % rispetto al raggio da 0 a 1
																// 1/SILHUETTE_MULTIPLIER sara 1
	
	F*=(1-k/SMOOTH_FORCE);										
	return F;
}

*/





/*
#define P_Parameter 2

double DeformableModel_US::Computa_Alpha(Vector<3> x) {

	Vector<2> p=Views[0].Projection(x);


	double Distance_From_Visual_Hull=Views[0].DistanceFromRealSilhouette(p);
	if (Distance_From_Visual_Hull<=0) {
		if (Distance_From_Visual_Hull>=-0.5) return 0;
		return -Distance_From_Visual_Hull*2;
	}

	double k=Views[0].DistanceFromSnakeEdge(p);
	if (k>=3) return 0;

	k=Distance_From_Visual_Hull;

	return k;
}
*/




























template <int dim>
FunctionSmoother<dim>::FunctionSmoother(HyperMesh<dim+1> *Mesh,double(*f)(Vector<dim> x)) {
	this->Mesh=Mesh;
	this->f=f;
}

template <int dim>
void FunctionSmoother<dim>::OneStep() {
	Vector<dim+1> F_ext;

	for(int i=0;i<Mesh->num_p;i++) {
	
		Vector<dim> x(Mesh->Points[i].GetList());
		double v=f(x);
		
		if (_finite(v)) F_ext[dim]=v-Mesh->Points[i][dim];			// Lineare??

		double Step=0.2;
		Mesh->Points[i]=Mesh->Points[i]+Step*F_ext;

		/*		
		Vector<dim> Laplacian=Mesh->Laplacians[i];
		Vector<dim> Biharmonic=Mesh->Biharmonics[i];

		Vector<dim> F_int=((Rigidity*(1/average_curvature)*Laplacian)-(Elasticity*(1/average_curvature_variation)*Biharmonic));
		// Sicuramente < di ( bisonga mettere il max)

	
		// Fissati F_int e F_ext -> e resi paragonabili -> devo muovermi di uno step proporzionale all'area?
		// o alla distanza media tra gli edge...
		// 

		// Integrazione
		//Mesh->Points[i]=Mesh->Points[i]+0.005*(F_ext+F_int);
		Vector<dim> P=0.005*(F_ext+F_int);
		Vector<dim> D=(average_stress/20)*(F_ext+F_int);
		Mesh->Points[i]=Mesh->Points[i]+D;
*/
	}


}




				/*Vector<3> V=Shadows[s].Light-Mesh->Points[i];
				int len=V.Norm2();
				V=V.Versore();
				if (!SegmentMeshIntersect(Mesh,V,Mesh->Points[i],len)) {
				*/
				/*
				Vector<3> rp=Shadows[s].Trasform(Mesh->Points[i]);
				Vector<2> pp=Shadows[s].Projection(Mesh->Points[i]);
				double z=Shadows[s].zbuffer->point(pp[0],pp[1]);
				if (z>=rp[2]) {
					// non interseca
					F_shadow[i]+=(-1)*Mesh->Normals[i];
				}
				*/
