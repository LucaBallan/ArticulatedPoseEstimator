//
// Articulated Pose Estimator on videos
//
//    Copyright (C) 2005-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
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
#include <iostream>
#include <io.h>
#include <conio.h>
#include <time.h>
#include <math.h> 
#include <stdlib.h> 
#include <assert.h>
#include <string.h> 
#include "resource.h"
#include "IKSolver.h"

using namespace std;
#pragma warning(3:4244)


#define FreeDegRX 0
#define FreeDegRY 1
#define FreeDegRZ 2
#define FreeDegX  3
#define FreeDegY  4
#define FreeDegZ  5


Matrix **IKSolver::Derivates_=NULL;

IKSolver::IKSolver(Skinned_Mesh *mesh,Vector<3> VolumeCenter,double VolumeRadius,bool show_debug_information,bool show_minimal_debug_information) {
	this->mesh=mesh;
	this->VolumeCenter=VolumeCenter;
	this->VolumeScaleFactor=VolumeRadius/(M_PI);
	this->show_debug_information=show_debug_information;
	this->show_minimal_debug_information=show_minimal_debug_information;

	// Freedom_Degrees
	num_Freedom_Degrees=0;
	Freedom_Degrees=NULL;
	
	// Cache per il calcolo di D_AM
	D_AM=NULL;
	ROW=NULL;

	// LM solver
	times_upd=0;
	Zero_Anomalies=0;
	Tot_Calculations=0;
	temp_frame_index=0;
	Temp_TargetList_2D=NULL;
	IKState=NULL;
	Calcolated_p=NULL;
	Solution=NULL;
	upperBound=NULL;
	lowerBound=NULL;
	LM_Mem_Storage=NULL;
	
	// Default parameters
	last_weighted_features=-1;
	added_sqrt_weight=1.0;

	// Debug Information
	show_debug_information=true;
	show_minimal_debug_information=true;

	// dynamic_observations
	dynamic_observations=0;

	// collision detector
	collision_mask=NULL;
	collision_detector=NULL;
	collision_intersecting_faces_buffer=NULL;
	collision_constraint_list=NULL;
	collision_ttl=0;
	collision_randomize_ttl_percentage=0.0;
	collision_sqrt_weight=0.0;
	collision_distance_sigma=0.0;
	dynamic_collision_update=false;
	outlier_function.initialize(3.0,5.0); // TODOAAA

	// LastComputedResidual
	LastComputedResidual=DBL_MAX;

	// I gradi di libertà sono tutti dell'ordine di -M_PI a +M_PI (anche quelli di traslazione)
		
	InitRigidDerivates();
	Init_Freedom_Degrees();
	Init_LM_Solver();
}

IKSolver::~IKSolver() {
	SDELETEA(Freedom_Degrees);
	SDELETEA_REC(D_AM,mesh->num_bones*num_Freedom_Degrees);
	SDELETEA(ROW);
	
	SDELETEA(IKState);
	SDELETEA(Solution);
	SDELETEA(Calcolated_p);
	SDELETEA(upperBound);
	SDELETEA(lowerBound);

	SDELETE(this->collision_constraint_list);
}

void IKSolver::GetFreedomDegressMap(int &num_Freedom_Degrees_,Freedom_Degrees_type *&Freedom_Degrees_) {
	num_Freedom_Degrees_=num_Freedom_Degrees;
	
	SDELETEA(Freedom_Degrees_);
	SNEWA(Freedom_Degrees_,Freedom_Degrees_type,num_Freedom_Degrees);

	for(int i=0;i<num_Freedom_Degrees;i++) Freedom_Degrees_[i]=Freedom_Degrees[i];
}

double IKSolver::GetVolumeScaleFactorAndVolumeCenter(Vector<3> &VolumeCenter) {
	VolumeCenter=this->VolumeCenter;
	return VolumeScaleFactor;
}
double *IKSolver::GetUpperBounds() {
	return upperBound;
}
double *IKSolver::GetLowerBounds() {
	return lowerBound;
}
double IKSolver::GetCollisionWeight() {
	return (collision_sqrt_weight*collision_sqrt_weight);
}
int IKSolver::GetCollisionTTL() {
	return collision_ttl;
}

void IKSolver::Init_Freedom_Degrees() {
	int num=0;
	for(int i=0;i<mesh->num_bones;i++) {
		if (!mesh->BoneRefTable[i]->LockX) num++;
		if (!mesh->BoneRefTable[i]->LockY) num++;
		if (!mesh->BoneRefTable[i]->LockZ) num++;
		if (!mesh->BoneRefTable[i]->LockRX) num++;
		if (!mesh->BoneRefTable[i]->LockRY) num++;
		if (!mesh->BoneRefTable[i]->LockRZ) num++;
	}
	num_Freedom_Degrees=num;
	ROW=new Vector<3>[num_Freedom_Degrees];
	Freedom_Degrees=new Freedom_Degrees_type[num];
	cout<<"IKSolver initialized with "<<num_Freedom_Degrees<<" Freedom Degrees\n";

	num=0;
	for(int i=0;i<mesh->num_bones;i++) {
		if (!mesh->BoneRefTable[i]->LockX) {
			Freedom_Degrees[num].type=FreeDegX;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
		if (!mesh->BoneRefTable[i]->LockY) {
			Freedom_Degrees[num].type=FreeDegY;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
		if (!mesh->BoneRefTable[i]->LockZ) {
			Freedom_Degrees[num].type=FreeDegZ;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
		if (!mesh->BoneRefTable[i]->LockRX) {
			Freedom_Degrees[num].type=FreeDegRX;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
		if (!mesh->BoneRefTable[i]->LockRY) {
			Freedom_Degrees[num].type=FreeDegRY;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
		if (!mesh->BoneRefTable[i]->LockRZ) {
			Freedom_Degrees[num].type=FreeDegRZ;
			Freedom_Degrees[num].Ref=mesh->BoneRefTable[i];
			Freedom_Degrees[num].BoneIndex=i;
			num++;
		}
	}

	num=mesh->num_bones*num_Freedom_Degrees;
	D_AM=new Matrix*[num];
	for(int i=0;i<num;i++) D_AM[i]=new Matrix(4,4);

}

void IKSolver::InitRigidDerivates() {
	if (Derivates_!=NULL) return;

	Derivates_=new Matrix*[3];
	Derivates_[0]=new Matrix(3,3);
	Derivates_[0]->SetZero();
	(*Derivates_[0])[1][2]=-1;
	(*Derivates_[0])[2][1]=1;
	Derivates_[1]=new Matrix(3,3);
	Derivates_[1]->SetZero();
	(*Derivates_[1])[0][2]=1;
	(*Derivates_[1])[2][0]=-1;
	Derivates_[2]=new Matrix(3,3);
	Derivates_[2]->SetZero();
	(*Derivates_[2])[0][1]=-1;
	(*Derivates_[2])[1][0]=1;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void IKSolver::Init_LM_Solver() {
	SNEWA(Solution,double,num_Freedom_Degrees);
	SNEWA(IKState,double,6*mesh->num_bones);
	SNEWA(Calcolated_p,double,num_Freedom_Degrees);
	
	// Set Limits
	if ((upperBound) || (lowerBound)) return;
	SNEWA(upperBound,double,num_Freedom_Degrees);
	SNEWA(lowerBound,double,num_Freedom_Degrees);
	for(int i=0;i<num_Freedom_Degrees;i++) {
		if (Freedom_Degrees[i].type<3) {
			upperBound[i]=(M_PI/180.0)*Freedom_Degrees[i].Ref->MaxR[Freedom_Degrees[i].type];
			lowerBound[i]=(M_PI/180.0)*Freedom_Degrees[i].Ref->MinR[Freedom_Degrees[i].type];
		} else {
			double scale=1.0/VolumeScaleFactor;
			upperBound[i]=scale*(Freedom_Degrees[i].Ref->MaxT[Freedom_Degrees[i].type-3]-VolumeCenter[Freedom_Degrees[i].type-3]);
			lowerBound[i]=scale*(Freedom_Degrees[i].Ref->MinT[Freedom_Degrees[i].type-3]-VolumeCenter[Freedom_Degrees[i].type-3]);
		}
	}
}

void IKSolver::getRotationDerivates(Matrix *MK,int type,Matrix *Out) {
	Matrix tmp1(3,3),tmp2(3,3);
	MK->GetMinor(3,3,&tmp1);
	Multiply(Derivates_[type],&tmp1,&tmp2);
	Out->SetZero();
	Out->SetMinor(3,3,&tmp2);
	(*Out)[3][3]=1.0;									// TODO***: TODO This probably should be 0.0, so the previous twist do not add any translation to the speed vector
}

void IKSolver::Computa_D_AM_S(int frame_index,int bone,int degree_index) {
	Matrix *Out=D_AM[(bone*num_Freedom_Degrees)+degree_index];
	int type=Freedom_Degrees[degree_index].type;
	Bone *ToDerive_Bone=Freedom_Degrees[degree_index].Ref;
	

	Array<Bone*> IKChain(40);
	Matrix DMK(4,4);
	Matrix Previous(4,4);
	Matrix Subsequent(4,4);	


	Bone *Current_Bone=mesh->BoneRefTable[bone];
	Current_Bone->GetIKChain(&IKChain);
	int inner_k=IKChain.search(ToDerive_Bone);
	if (inner_k==-1) {
		Out->SetZero();
		return;
	}


	// Computa Previous Rotation
	if (inner_k-1<0) {
		Previous.SetIdentity();
	} else {
		IKChain[inner_k-1]->GetABSTrasformMatrix(frame_index,&Previous);
		Previous[0][3]=0;
		Previous[1][3]=0;
		Previous[2][3]=0;
	}


	if (type>=3) {
		// Traslation
		Out->SetIdentity();
		(*Out)[0][3]=Previous[0][type-3];
		(*Out)[1][3]=Previous[1][type-3];
		(*Out)[2][3]=Previous[2][type-3];
	} else {
		// Rotation

		// Computa Subsequent Trasformation
		{
			IKChain[inner_k]->GetABSTrasformMatrix(frame_index,&Subsequent);
			Invert4x4RotoTraslationMatrix(&Subsequent);							// Subsequent.Inversion();
		}

		// Derive Mk
		{
			Matrix MK(4,4);
			IKChain[inner_k]->GetRelTrasformMatrix(frame_index,&MK);
			getRotationDerivates(&MK,type,&DMK);
		}

		// Computa_D_AM_S
		{
			Matrix tmp(4,4);
			Multiply(&Previous,&DMK,&tmp);
			Multiply(&tmp,&Subsequent,Out);
		}
	}

}

void IKSolver::Computa_D_AM(int frame_index) {
	for(int i=0;i<mesh->num_bones;i++) {
		for(int j=0;j<num_Freedom_Degrees;j++) {
			Computa_D_AM_S(frame_index,i,j);
		}
	}
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






void IKSolver::SetDynamic_Observations(int dynamic_observations) {
	this->dynamic_observations=max(0,dynamic_observations);
}

void IKSolver::SetCollisionDetector(HyperMeshPartitionTree *collision_detector,bool *collision_mask,Array<ordered_pair> *collision_intersecting_faces_buffer) {
	// external
	this->collision_detector=collision_detector;
	this->collision_mask=collision_mask;
	this->collision_intersecting_faces_buffer=collision_intersecting_faces_buffer;
	// internal
	SNEW(this->collision_constraint_list,Array<collision_constraint>(INITIAL_IKSOLVER_COLLISION_BUFFERSIZE));
	Clear_Collision_Constraints();
	// parameters
	SetCollisionParameters(0,0.0,50.0,0.5);		// Default values
}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool compare_TargetList_3D_Item_type(TargetList_3D_Item_type *a,TargetList_3D_Item_type *b) {
	if ((a->face_i==b->face_i) && (a->face_j==b->face_j)) return true;
	if ((a->face_j==b->face_i) && (a->face_i==b->face_j)) return true;
	return false;
}

void IKSolver::Add_Contact_Points(Array<TargetList_3D_Item_type> *TargetList_3D_ContactPoints,double weight,int ttl) {
	if (!collision_detector) return;
	if (weight==0.0) return;
	
	int saved_num=TargetList_3D_ContactPoints->numElements();

	float sqrt_weight=(float)sqrt(weight);

	collision_detector->UpdateBoundingBoxes();
	collision_intersecting_faces_buffer->clear();
	collision_detector->SelfIntersection_BinaryTree_Skinned_Mesh(collision_intersecting_faces_buffer,collision_mask);

	int                             num_face_pairs=collision_intersecting_faces_buffer->numElements();
	ordered_pair                        *face_pair=collision_intersecting_faces_buffer->getMem();
	Vector<3>                                   *P=mesh->Points.getMem();
	HyperFace<3>                                *F=mesh->Faces.getMem();
	double                                      *Area=mesh->AreaPoint.getMem();

	for(int i=0;i<num_face_pairs;i++) {
		HyperFace<3> *a=&(F[face_pair[i].i]);
		HyperFace<3> *b=&(F[face_pair[i].j]);
		
		int min_a,min_b;
		double cur_min=DBL_MAX;
		for(int ai=0;ai<3;ai++) {
			for(int bi=0;bi<3;bi++) {
				double dist=(P[a->Point[ai]]-P[b->Point[bi]]).Norm2();
				if (dist<cur_min) {
					min_a=ai;
					min_b=bi;
					cur_min=dist;
				}
			}
		}

		//if (cur_min>***) { // TODOLUCA
		//}

		TargetList_3D_Item_type x;
		x.sqrt_weight=sqrt_weight*((float)Area[a->Point[min_a]]);
		x.VertexIndex=a->Point[min_a];
		x.VertexIndex_Target=b->Point[min_b];
		x.face_i=face_pair[i].i;
		x.face_j=face_pair[i].j;
		x.Target=P[x.VertexIndex]-P[x.VertexIndex_Target];
		if (collision_randomize_ttl_percentage!=0.0) {
			int delta_ttl=(int)(max(collision_randomize_ttl_percentage*ttl,1));
			x.ttl=Uniform_int_rand(max(ttl-delta_ttl,0),ttl+delta_ttl);
		} else x.ttl=ttl;

		// Check duplicates TODOAAA (forse e' meglio tenerli?) TODOAAA not tested yet
		int duplicate_index=TargetList_3D_ContactPoints->search(compare_TargetList_3D_Item_type,&x);
		if (duplicate_index==-1) {
			TargetList_3D_ContactPoints->append(x);
		} else {
			// aggiorna il ttl 
			(*TargetList_3D_ContactPoints)[duplicate_index].ttl=max((*TargetList_3D_ContactPoints)[duplicate_index].ttl,x.ttl);
		}
	}

	if (show_debug_information) {
		if (saved_num==0) {
			cout<<"     Contacts:       "<<TargetList_3D_ContactPoints->numElements()<<"\n";
		} else {
			cout<<"     Contacts:       "<<TargetList_3D_ContactPoints->numElements()<<" ("<<(TargetList_3D_ContactPoints->numElements()-saved_num)<<" added)\n";
		}
	}
}

void IKSolver::Contact_Points_Increase_Time(Array<TargetList_3D_Item_type> *TargetList_3D_ContactPoints,double weight,int ttl) {
	if (!collision_detector) return;
	if (weight==0.0) return;

	int saved_num=TargetList_3D_ContactPoints->numElements();

	for(int i=0;i<TargetList_3D_ContactPoints->numElements();i++) {
		if ((--((*(TargetList_3D_ContactPoints))[i].ttl))==0) {
			TargetList_3D_ContactPoints->del(i);
			i--;
		}
	}
	if (show_debug_information) {
		cout<<"     Contacts:       "<<saved_num-TargetList_3D_ContactPoints->numElements()<<" deleted"<<endl;
			   
	}
}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IKSolver::SetCollisionParameters(int ttl,double randomize_ttl_percentage,double weight,double distance_sigma) {
	collision_ttl=ttl;
	collision_randomize_ttl_percentage=randomize_ttl_percentage;
	collision_sqrt_weight=sqrt(weight);
	collision_distance_sigma=distance_sigma;
}
void IKSolver::UpdateCollisionWeights(double weight) {
	collision_sqrt_weight=sqrt(weight);
}

inline bool collision_constraint_uniqueness(collision_constraint *v,collision_constraint *to_src) {
	#ifdef FORCE_ONE_COLLISION_PER_VERTEX
		if (v->vertex_index==to_src->vertex_index) return true;
	#else
		if ((v->vertex_index==to_src->vertex_index) && (v->i_face==to_src->i_face)) return true;
	#endif
	return false;
}

void IKSolver::Add_Collision_Constraints() {
	if (!collision_constraint_list) return;
	if (collision_sqrt_weight==0.0) return;

	int free_observations_slots=dynamic_observations-(collision_constraint_list->numElements());
	if (free_observations_slots<=0) return;
	
	int initial_free_observations_slots=free_observations_slots;

	collision_detector->UpdateBoundingBoxes();
	collision_intersecting_faces_buffer->clear();
	collision_detector->SelfIntersection_BinaryTree_Skinned_Mesh(collision_intersecting_faces_buffer,collision_mask);

	HyperFace<3>                                *F=mesh->Faces.getMem();
	Vector<3>                                   *P=mesh->Points.getMem();
	int                             num_face_pairs=collision_intersecting_faces_buffer->numElements();
	ordered_pair                        *face_pair=collision_intersecting_faces_buffer->getMem();
	int                            face_pair_index=-1;
	ordered_pair               *selected_face_pair;
	collision_constraint                      tmp;
	if (num_face_pairs==0) {
		if (show_debug_information) {
			cout<<"     Collisions:     0 ("<<collision_constraint_list->numElements()<<" total observations)\n";	
		}
		return;
	}

	// Add randomly the pairs
	// NOTE: evem if there is enough space, the result is always randomized because who get first get in (the seconds are eliminated) the reordering is always performed
	RandomSelector RSel(num_face_pairs);

	do {
		face_pair_index=RSel.select();
		if (face_pair_index==-1) break;

		// Randomize ttl
		if (collision_randomize_ttl_percentage!=0.0) {
			int delta_ttl=(int)(max(collision_randomize_ttl_percentage*collision_ttl,1));
			tmp.ttl=Uniform_int_rand(max(collision_ttl-delta_ttl,0),collision_ttl+delta_ttl);
		} else tmp.ttl=collision_ttl;


		selected_face_pair=face_pair+face_pair_index;
		int             f1=selected_face_pair->i;
		int             f2=selected_face_pair->j;


		tmp.i_face=f2;
		tmp.dir=F[f2].Normal(&(mesh->Points));
		#ifndef USE_CONIC_DISTANCE_FUNCTION
			tmp.o=P[F[f2].Point[0]];
		#else
			tmp.o=F[f2].Baricentro(&(mesh->Points));
			tmp.r=0.0;
			for(int q=0;q<3;q++) tmp.r=max(tmp.r,(P[F[f2].Point[q]]-tmp.o).Norm2());
		#endif
			
		for(int q=0;q<3;q++) {
			tmp.vertex_index=F[f1].Point[q];
			// Force that a face-point pair is not double
			// If(FORCE_ONE_COLLISION_PER_VERTEX) -> Force a vertex to have only one collision
			if (collision_constraint_list->search(collision_constraint_uniqueness,&tmp)==-1) {		
				// Esclude quelli che sono gia' fuori dalla collisione (OK)
				if ((P[tmp.vertex_index]-tmp.o)*tmp.dir<0.0) {
					if (free_observations_slots>0) {
						collision_constraint_list->append(tmp);
						free_observations_slots--;
					} 
				}
			}
		}

		tmp.i_face=f1;
		tmp.dir=F[f1].Normal(&(mesh->Points));
		#ifndef USE_CONIC_DISTANCE_FUNCTION
			tmp.o=P[F[f1].Point[0]];
		#else
			tmp.o=F[f1].Baricentro(&(mesh->Points));
			tmp.r=0.0;
			for(int q=0;q<3;q++) tmp.r=max(tmp.r,(P[F[f1].Point[q]]-tmp.o).Norm2());
		#endif
		

		for(int q=0;q<3;q++) {
			tmp.vertex_index=F[f2].Point[q];
			if (collision_constraint_list->search(collision_constraint_uniqueness,&tmp)==-1) {		
				if ((P[tmp.vertex_index]-tmp.o)*tmp.dir<0.0) {
					if (free_observations_slots>0) {
						collision_constraint_list->append(tmp);
						free_observations_slots--;
					}
				}
			}
		}
		
	} while (free_observations_slots>0);

	if (show_debug_information) {
		if (initial_free_observations_slots-free_observations_slots!=collision_constraint_list->numElements()) {
			cout<<"     Collisions:     "<<collision_constraint_list->numElements()<<" ("<<(initial_free_observations_slots-free_observations_slots)<<" added) ("<<collision_intersecting_faces_buffer->numElements()<<" actual colliding faces)\n";
		} else {
			cout<<"     Collisions:     "<<collision_constraint_list->numElements()<<" ("<<collision_intersecting_faces_buffer->numElements()<<" actual colliding faces)\n";
		}
	}
}

double IKSolver::Collision_Residual(int *n_observations) {
	(*n_observations)=0;
	if (!collision_constraint_list) return 0.0;
	if (collision_sqrt_weight==0.0) return 0.0;

	int num_soft_constraints       =collision_constraint_list->numElements();
	collision_constraint *c_list   =collision_constraint_list->getMem();
	Vector<3> *Points=mesh->Points.getMem();
	double *AreaPoint=mesh->AreaPoint.getMem();

	double residual=0.0;

	for(int i=0;i<num_soft_constraints;i++,c_list++) {
		#ifndef USE_CONIC_DISTANCE_FUNCTION
			double dist=(Points[c_list->vertex_index]-(c_list->o))*(c_list->dir);
			double tmp=AreaPoint[c_list->vertex_index]*Raised_Cos(dist,collision_distance_sigma);
		#else
			distance_function.Set(collision_distance_sigma);		// TODO*** speed it up or change with the surface area!!
			double tmp=AreaPoint[c_list->vertex_index]*distance_function.F(Points[c_list->vertex_index],c_list->o,c_list->dir,c_list->r);
		#endif
		residual+=tmp*tmp;
	}
	
	(*n_observations)=num_soft_constraints;
	return residual;
}

void IKSolver::Collision_Increase_Time() {
	if (!collision_constraint_list) return;

	int old_num=collision_constraint_list->numElements();

	// TODO*** eliminare solo se soddisfatti?
	for(int i=0;i<collision_constraint_list->numElements();i++) {
		if ((--((*(collision_constraint_list))[i].ttl))==0) {
			collision_constraint_list->del(i);
			i--;
		}
	}
	if (show_debug_information) {
		cout<<"     Collisions:     "<<old_num-collision_constraint_list->numElements()<<" deleted"<<endl;
	}
}

void IKSolver::Clear_Collision_Constraints() {
	if (show_debug_information) {
		cout<<"     Collisions:     cleared\n";
	}
	if (collision_constraint_list) collision_constraint_list->clear();
}
void IKSolver::Copy_Collision_Constraints(IKSolver *IK) {
	if ((collision_constraint_list) && (IK->collision_constraint_list)) {
		if (show_debug_information) {
			cout<<"     Collisions:     "<<IK->collision_constraint_list->numElements()<<" copied\n";
		}
		collision_constraint_list->CopyFrom(IK->collision_constraint_list);
	}
}

void IKSolver::Dynamic_Collision_Update(bool activate) {
	dynamic_collision_update=activate;
}





















void IKSolver::LM_Update(double *p,IKSolver *IK_OBJ,bool Final=false) {
	int i;
	int num_Freedom_Degrees=IK_OBJ->num_Freedom_Degrees;
	double *Calcolated_p=IK_OBJ->Calcolated_p;
	int temp_frame_index=IK_OBJ->temp_frame_index;
	int num_bones=IK_OBJ->mesh->num_bones;


	for(i=0;i<num_Freedom_Degrees;i++) 
		if (Calcolated_p[i]!=p[i]) break;
	if (i==num_Freedom_Degrees) return;
	for(;i<num_Freedom_Degrees;i++) Calcolated_p[i]=p[i];
	IK_OBJ->mesh->BoneRoot->Copy(0,temp_frame_index);
	if (!Final) IK_OBJ->times_upd++;

	// Aggiorna IKState (lo stato globale)
	Freedom_Degrees_type *DOF_tmp=IK_OBJ->Freedom_Degrees;
	double *IK_State_tmp=IK_OBJ->IKState;
	double scale=IK_OBJ->VolumeScaleFactor;
	for(int k=0;k<num_Freedom_Degrees;k++) {
		if (DOF_tmp->type>=3) IK_State_tmp[(DOF_tmp->BoneIndex*6)+DOF_tmp->type]=(scale*p[k])+IK_OBJ->VolumeCenter[DOF_tmp->type-3];
		else IK_State_tmp[(DOF_tmp->BoneIndex*6)+DOF_tmp->type]=p[k];
		
		DOF_tmp++;
	}


	// Calcola Rotazione Bones
	Vector<3> s;
	for(int k=0;k<num_bones;k++) {
		Bone *Curr=IK_OBJ->mesh->BoneRefTable[k];
		s[0]=*(IK_State_tmp++);
		s[1]=*(IK_State_tmp++);
		s[2]=*(IK_State_tmp++);
		Curr->Rotate(temp_frame_index,s);
		s[0]=*(IK_State_tmp++);
		s[1]=*(IK_State_tmp++);
		s[2]=*(IK_State_tmp++);
		Curr->RelativeTrasformationT[temp_frame_index]+=s;
	}


	// Calcolare assolute
	IK_OBJ->mesh->BoneRoot->CalcAbsoluteFromRelative(temp_frame_index);	


	if (Final) return;
	IK_OBJ->mesh->Apply_Skin(temp_frame_index);
	
	

	if (IK_OBJ->dynamic_collision_update) {
		IK_OBJ->Collision_Increase_Time();
		IK_OBJ->Add_Collision_Constraints();
	}
}

void IKSolver::LM_Function(double *p,double *x,int m,int n,void *data) {
	IKSolver *IK_OBJ=static_cast<IKSolver *>(data);
	int num_elements_1D=IK_OBJ->Temp_TargetList_1D->numElements();
	int num_elements_2D=IK_OBJ->Temp_TargetList_2D->numElements();
	int num_elements_3D=IK_OBJ->Temp_TargetList_3D->numElements();

	// Fast locals
	TargetList_1D_Item_type   *TargetList_1D_  =IK_OBJ->Temp_TargetList_1D->getMem();
	TargetList_2D_Item_type   *TargetList_2D_  =IK_OBJ->Temp_TargetList_2D->getMem();
	TargetList_3D_Item_type   *TargetList_3D_  =IK_OBJ->Temp_TargetList_3D->getMem();
	Vector<3>                 *Points          =IK_OBJ->mesh->Points.getMem();
	double                    *AreaPoint       =IK_OBJ->mesh->AreaPoint.getMem();


	// Update BoneStructure/SelectedPointsCoordinates
	IKSolver::LM_Update(p,IK_OBJ);
	#ifdef IKS_SHOW_MINIMIZATION_CALLS
	if (IK_OBJ->show_debug_information) cout<<"F";
	#endif


	// Computa x
	Vector<3> tmp_p;
	Vector<2> PP2;
	double *inc_x=x;

	// Compute 2D observations
	for(int i=0;i<num_elements_2D;i++,TargetList_2D_++) {
		int point_index=TargetList_2D_->VertexIndex;
		tmp_p=Points[point_index];
		float tmp_w=TargetList_2D_->sqrt_weight;	
		PP2=TargetList_2D_->View->Projection(tmp_p); 
		if (i<=IK_OBJ->last_weighted_features) tmp_w*=(float)IK_OBJ->added_sqrt_weight;	// TODO eliminare

		*(inc_x++)=tmp_w*PP2[0];
		*(inc_x++)=tmp_w*PP2[1];
	}
	



	// Compute 1D observations
	for(int i=0;i<num_elements_1D;i++,TargetList_1D_++) {
		tmp_p=Points[TargetList_1D_->VertexIndex];
		tmp_p=TargetList_1D_->View->ProjectionZ(tmp_p); 
		
		// TODOCHECKS controllo che nn sia out of the 2D projections of the point it is related to
		*(inc_x++)=TargetList_1D_->sqrt_weight*tmp_p[2];
	}




	// Compute 3D observations
	for(int i=0;i<num_elements_3D;i++,TargetList_3D_++) {
		tmp_p=Points[TargetList_3D_->VertexIndex];
		if (TargetList_3D_->VertexIndex_Target!=-1) 
			tmp_p=tmp_p-Points[TargetList_3D_->VertexIndex_Target];
		float tmp_w=TargetList_3D_->sqrt_weight;
		
		if (TargetList_3D_->VertexIndex_Target!=-1)  { // TODOAAA 
			double pp_dist=(tmp_p-TargetList_3D_->Target).Norm2();
			*(inc_x++)=tmp_w*IK_OBJ->outlier_function.SqrtF(pp_dist);
			*(inc_x++)=0.0;
			*(inc_x++)=0.0;
		} else {
			*(inc_x++)=tmp_w*tmp_p[0];
			*(inc_x++)=tmp_w*tmp_p[1];
			*(inc_x++)=tmp_w*tmp_p[2];
		}
	}




	// Add dynamic observations
	if (IK_OBJ->dynamic_observations) {
		int i=0;

		// Compute soft contraints values
		if (IK_OBJ->collision_constraint_list) {
			int num_soft_constraints       =IK_OBJ->collision_constraint_list->numElements();
			collision_constraint *c_list   =IK_OBJ->collision_constraint_list->getMem();
		
			for(;i<num_soft_constraints;i++,inc_x++,c_list++) {
				#ifndef USE_CONIC_DISTANCE_FUNCTION
					double dist=(Points[c_list->vertex_index]-(c_list->o))*(c_list->dir);
					*inc_x=(IK_OBJ->collision_sqrt_weight)*AreaPoint[c_list->vertex_index]*Raised_Cos(dist,IK_OBJ->collision_distance_sigma);
				#else
					IK_OBJ->distance_function.Set(IK_OBJ->collision_distance_sigma);		// TODO*** speed it up or change with the surface area!!
					*inc_x=(IK_OBJ->collision_sqrt_weight)*AreaPoint[c_list->vertex_index]*IK_OBJ->distance_function.F(Points[c_list->vertex_index],c_list->o,c_list->dir,c_list->r);
				#endif

			}
		}

		// Fill the rest
		for(;i<IK_OBJ->dynamic_observations;i++,inc_x++) *inc_x=0.0;
	}
}






void IKSolver::LM_Jacobian(double *p,double *jac,int m,int n,void *data) {
	Vector<3> PP;
	Vector<3> tmp_p;
	Vector<4> tmp_row_m;
	Vector<3> dPPt;
	Vector<4> e,point;

	IKSolver *IK_OBJ=static_cast<IKSolver *>(data);
	Skinned_Mesh *mesh=IK_OBJ->mesh;
	Array<TargetList_1D_Item_type> *TargetList_1D_=IK_OBJ->Temp_TargetList_1D;
	Array<TargetList_2D_Item_type> *TargetList_2D_=IK_OBJ->Temp_TargetList_2D;
	Array<TargetList_3D_Item_type> *TargetList_3D_=IK_OBJ->Temp_TargetList_3D;
	int temp_frame_index=IK_OBJ->temp_frame_index;
	int num_Freedom_Degrees=m;
	int num_elements_1D=IK_OBJ->Temp_TargetList_1D->numElements();
	int num_elements_2D=IK_OBJ->Temp_TargetList_2D->numElements();
	int num_elements_3D=IK_OBJ->Temp_TargetList_3D->numElements();
	int num_bones=mesh->num_bones;
	Vector<3> *ROW=IK_OBJ->ROW;
	Vector<3> *ROWp;


	// Update BoneStructure/SelectedPointsCoordinates
	IKSolver::LM_Update(p,IK_OBJ);
	#ifdef IKS_SHOW_MINIMIZATION_CALLS
	if (IK_OBJ->show_debug_information) cout<<"J";
	#endif

	// Computate Jacobian
	IK_OBJ->Computa_D_AM(temp_frame_index);



	// Compute Jac 2D observations
	for(int i=0;i<num_elements_2D;i++) {																// TODOSS use pointers inside the loop on all this function
		// Get Point
		int point_index=(*TargetList_2D_)[i].VertexIndex;
		tmp_p=mesh->Points[point_index];
		double Target_Weight=(*TargetList_2D_)[i].sqrt_weight;
		if (i<=IK_OBJ->last_weighted_features) Target_Weight*=IK_OBJ->added_sqrt_weight;
		point[0]=tmp_p[0];
		point[1]=tmp_p[1];
		point[2]=tmp_p[2];
		point[3]=1.0;
		
		PP=(*TargetList_2D_)[i].View->Trasform(tmp_p);

		// Resetto le colonne ROW[:]
		for(int j=0;j<num_Freedom_Degrees;j++) ROW[j].SetZero();
		
		// Calcolo variazione dP 3D di point
		for(int bone_=0;bone_<num_bones;bone_++) {
			double w_tmp=mesh->Weigth[point_index][bone_];
			if (w_tmp!=0.0) {
				
				for(int j=0;j<num_Freedom_Degrees;j++) {
					Matrix *D_AM_tmp=IK_OBJ->D_AM[(bone_*num_Freedom_Degrees)+j];

					if (IK_OBJ->Freedom_Degrees[j].type<3) {
						Multiply(D_AM_tmp,point,e.GetList());
					} else {
						e[0]=(*D_AM_tmp)[0][3];
						e[1]=(*D_AM_tmp)[1][3];
						e[2]=(*D_AM_tmp)[2][3];
					}

					ROWp=&(ROW[j]);
					(*ROWp)[0]+=w_tmp*e[0];
					(*ROWp)[1]+=w_tmp*e[1];
					(*ROWp)[2]+=w_tmp*e[2];
				}

			}
		}

		for(int j=0;j<num_Freedom_Degrees;j++) {
			// Compute R*dP -> dPPt
			ROWp=&(ROW[j]);
			tmp_row_m[0]=(*ROWp)[0];
			tmp_row_m[1]=(*ROWp)[1];
			tmp_row_m[2]=(*ROWp)[2];
			tmp_row_m[3]=0.0;
			
			if (!IS_ZERO(tmp_row_m.Norm2(),DBL_EPSILON)) {
				Multiply(&((*TargetList_2D_)[i].View->P),tmp_row_m,dPPt.GetList());
				double factor=Target_Weight/(PP[2]*PP[2]);
				if (dPPt[2]==0) IK_OBJ->Zero_Anomalies++;
				IK_OBJ->Tot_Calculations++;
				*jac=factor*(PP[2]*dPPt[0]-PP[0]*dPPt[2]);							//jac[i*2*num_Freedom_Degrees+j]=factor*(PP[2]*dPPt[0]-PP[0]*dPPt[2]);
				*(jac+num_Freedom_Degrees)=factor*(PP[2]*dPPt[1]-PP[1]*dPPt[2]);	//jac[(i*2+1)*num_Freedom_Degrees+j]=factor*(PP[2]*dPPt[1]-PP[1]*dPPt[2]);
				jac++;
			} else {
				*jac=0.0;															//jac[i*2*num_Freedom_Degrees+j]=0.0;
				*(jac+num_Freedom_Degrees)=0.0;										//jac[(i*2+1)*num_Freedom_Degrees+j]=0.0;
				jac++;
			}
		}
		// Passa alla riga di jac successiva 
		jac+=num_Freedom_Degrees;
	}



























	// controllo righe
	for(int i=0;i<num_elements_1D;i++) {
		// Get Point
		int      point_index   = ( *TargetList_1D_)[i].VertexIndex;
		double   Target_Weight = ( *TargetList_1D_)[i].sqrt_weight;
		Matrix  *PrjMatrix     =&((*TargetList_1D_)[i].View->P);
		tmp_p=mesh->Points[point_index];
		point[0]=tmp_p[0];
		point[1]=tmp_p[1];
		point[2]=tmp_p[2];
		point[3]=1.0;
		
		// Resetto le colonne ROW[:]
		for(int j=0;j<num_Freedom_Degrees;j++) ROW[j].SetZero();
		
		// Calcolo variazione dP 3D di point
		for(int bone_=0;bone_<num_bones;bone_++) {
			double w_tmp=mesh->Weigth[point_index][bone_];
			if (w_tmp!=0.0) {
				
				ROWp=ROW;
				for(int j=0;j<num_Freedom_Degrees;j++,ROWp++) {
					Matrix *D_AM_tmp=IK_OBJ->D_AM[(bone_*num_Freedom_Degrees)+j];

					if (IK_OBJ->Freedom_Degrees[j].type<3) {
						Multiply(D_AM_tmp,point,e.GetList());
					} else {
						e[0]=(*D_AM_tmp)[0][3];
						e[1]=(*D_AM_tmp)[1][3];
						e[2]=(*D_AM_tmp)[2][3];
					}

					(*ROWp)[0]+=w_tmp*e[0];
					(*ROWp)[1]+=w_tmp*e[1];
					(*ROWp)[2]+=w_tmp*e[2];
				}

			}
		}
		


		ROWp=ROW;
		for(int j=0;j<num_Freedom_Degrees;j++,jac++,ROWp++) {
			// Compute R*dP -> dPPt
			tmp_row_m[0]=(*ROWp)[0];
			tmp_row_m[1]=(*ROWp)[1];
			tmp_row_m[2]=(*ROWp)[2];
			tmp_row_m[3]=0.0;
			
			Multiply(PrjMatrix,tmp_row_m,dPPt.GetList());
			*jac=Target_Weight*dPPt[2];
		}
		// todo** deve passare alla linea successiva
	}












	// controllo righe
	for(int i=0;i<num_elements_3D;i++) {
		// Get Point
		int     point_index   =( *TargetList_3D_)[i].VertexIndex;
		double  Target_Weight =( *TargetList_3D_)[i].sqrt_weight;
		tmp_p=mesh->Points[point_index];
		point[0]=tmp_p[0];
		point[1]=tmp_p[1];
		point[2]=tmp_p[2];
		point[3]=1.0;
		
		// Resetto le colonne ROW[:]
		for(int j=0;j<num_Freedom_Degrees;j++) ROW[j].SetZero();
		
		// Calcolo variazione dP 3D di point
		for(int bone_=0;bone_<num_bones;bone_++) {
			double w_tmp=mesh->Weigth[point_index][bone_];
			if (w_tmp!=0.0) {
				

				ROWp=ROW;
				for(int j=0;j<num_Freedom_Degrees;j++,ROWp++) {
					Matrix *D_AM_tmp=IK_OBJ->D_AM[(bone_*num_Freedom_Degrees)+j];

					if (IK_OBJ->Freedom_Degrees[j].type<3) {
						Multiply(D_AM_tmp,point,e.GetList());
					} else {
						e[0]=(*D_AM_tmp)[0][3];
						e[1]=(*D_AM_tmp)[1][3];
						e[2]=(*D_AM_tmp)[2][3];
					}

					(*ROWp)[0]+=w_tmp*e[0];
					(*ROWp)[1]+=w_tmp*e[1];
					(*ROWp)[2]+=w_tmp*e[2];
				}

			}
		}
		

		if ((*TargetList_3D_)[i].VertexIndex_Target!=-1) {
			point_index=( *TargetList_3D_)[i].VertexIndex_Target;
			tmp_p=mesh->Points[point_index];
			point[0]=tmp_p[0];
			point[1]=tmp_p[1];
			point[2]=tmp_p[2];
			point[3]=1.0;
			
			// Calcolo variazione dP 3D di point
			for(int bone_=0;bone_<num_bones;bone_++) {
				double w_tmp=mesh->Weigth[point_index][bone_];
				if (w_tmp!=0.0) {
					

					ROWp=ROW;
					for(int j=0;j<num_Freedom_Degrees;j++,ROWp++) {
						Matrix *D_AM_tmp=IK_OBJ->D_AM[(bone_*num_Freedom_Degrees)+j];

						if (IK_OBJ->Freedom_Degrees[j].type<3) {
							Multiply(D_AM_tmp,point,e.GetList());
						} else {
							e[0]=(*D_AM_tmp)[0][3];
							e[1]=(*D_AM_tmp)[1][3];
							e[2]=(*D_AM_tmp)[2][3];
						}

						(*ROWp)[0]-=w_tmp*e[0];
						(*ROWp)[1]-=w_tmp*e[1];
						(*ROWp)[2]-=w_tmp*e[2];
					}

				}
			}
		}

		if ((*TargetList_3D_)[i].VertexIndex_Target!=-1) {	// TODOAAA
			ROWp=ROW;
			
			tmp_p=mesh->Points[(*TargetList_3D_)[i].VertexIndex];
			tmp_p=tmp_p-mesh->Points[(*TargetList_3D_)[i].VertexIndex_Target];
			tmp_p=tmp_p-(*TargetList_3D_)[i].Target;
			double pp_dist=tmp_p.Norm2();
			double pp_n=1.0;
			if (pp_dist<10.0*DBL_EPSILON) {
				tmp_p.SetZero();
			} else {
				pp_n=IK_OBJ->outlier_function.DSqrtF(pp_dist)/pp_dist;
			}
			tmp_p=pp_n*tmp_p;

			for(int j=0;j<num_Freedom_Degrees;j++,jac++,ROWp++) {
				*jac                            = Target_Weight*(tmp_p*(*ROWp));
				*(jac+num_Freedom_Degrees)      = 0.0;
				*(jac+(num_Freedom_Degrees<<1)) = 0.0;
			}
		} else {
			ROWp=ROW;
			for(int j=0;j<num_Freedom_Degrees;j++,jac++,ROWp++) {
				*jac                            = Target_Weight*((*ROWp)[0]);
				*(jac+num_Freedom_Degrees)      = Target_Weight*((*ROWp)[1]);
				*(jac+(num_Freedom_Degrees<<1)) = Target_Weight*((*ROWp)[2]);
			}
		}
		
		// Passa alla riga di jac successiva 
		jac+=(num_Freedom_Degrees<<1);
	}





	
	
	
	
	// Compute Jac Dynamic Observations
	if (IK_OBJ->dynamic_observations) {
		int i=0;


		// Compute soft contraints jac values
		if (IK_OBJ->collision_constraint_list) {
			int num_soft_constraints     =IK_OBJ->collision_constraint_list->numElements();
			collision_constraint *c_list =IK_OBJ->collision_constraint_list->getMem();

			for(;i<num_soft_constraints;i++,c_list++) {
				// Get Point
				int point_index=c_list->vertex_index;
				tmp_p=mesh->Points[point_index];
				point[0]=tmp_p[0];
				point[1]=tmp_p[1];
				point[2]=tmp_p[2];
				point[3]=1.0;

				// Part 1 (independent by theta)
				#ifndef USE_CONIC_DISTANCE_FUNCTION
					double dist=(tmp_p-(c_list->o))*(c_list->dir);
					double d_part_1=(IK_OBJ->collision_sqrt_weight)*mesh->AreaPoint[point_index]*D_Raised_Cos(dist,IK_OBJ->collision_distance_sigma);
				#else
					IK_OBJ->distance_function.Set(IK_OBJ->collision_distance_sigma);		// TODO*** speed it up or change with the surface area!!
					Vector<3> d_part_1=((IK_OBJ->collision_sqrt_weight)*mesh->AreaPoint[point_index])*IK_OBJ->distance_function.DF(tmp_p,c_list->o,c_list->dir,c_list->r);
				#endif
				

				// Part 2
				// Resetto le colonne ROW[:]
				for(int j=0;j<num_Freedom_Degrees;j++) ROW[j].SetZero();
				
				// Calcolo variazione dP 3D di point  -> ROW[j] = dP(i)/d(theta(j)) belongs to R3
				for(int bone_=0;bone_<num_bones;bone_++) {
					double w_tmp=mesh->Weigth[point_index][bone_];
					if (w_tmp!=0.0) {
						for(int j=0;j<num_Freedom_Degrees;j++) {
							Matrix *D_AM_tmp=IK_OBJ->D_AM[(bone_*num_Freedom_Degrees)+j];

							if (IK_OBJ->Freedom_Degrees[j].type<3) {
								Multiply(D_AM_tmp,point,e.GetList());
							} else {
								e[0]=(*D_AM_tmp)[0][3];
								e[1]=(*D_AM_tmp)[1][3];
								e[2]=(*D_AM_tmp)[2][3];
							}

							ROWp=&(ROW[j]);
							(*ROWp)[0]+=w_tmp*e[0];					// actually it should have been called COLUMN not ROW
							(*ROWp)[1]+=w_tmp*e[1];
							(*ROWp)[2]+=w_tmp*e[2];
						}
					}
				}

				#ifndef USE_CONIC_DISTANCE_FUNCTION
					for(int j=0;j<num_Freedom_Degrees;j++,jac++) *jac=d_part_1*(ROW[j]*c_list->dir);
				#else
					for(int j=0;j<num_Freedom_Degrees;j++,jac++) *jac=d_part_1*(ROW[j]);
				#endif
			}
		}

		
		// Fill the rest
		for(;i<IK_OBJ->dynamic_observations;i++) 
			for(int j=0;j<num_Freedom_Degrees;j++,jac++) *jac=0.0;
	}

}






#include "levmar.h"


char *LM_ERRORS[7] ={"stopped by small gradient J^T e",									    \
					 "stopped by small Dp",												    \
					 "stopped by itmax",												    \
					 "singular matrix. Restart from current p with increased mu",		    \
					 "no further error reduction is possible. Restart with increased mu",   \
			         "stopped by small ||e||^2",											\
			         "stopped by invalid (i.e. NaN or Inf) \"func\" values."};


bool IKSolver::Perspective_Solve_LM(int frame_index,
									Array<TargetList_1D_Item_type> *TargetList_1D,
									Array<TargetList_2D_Item_type> *TargetList_2D,
									Array<TargetList_3D_Item_type> *TargetList_3D,
									int Num_of_Iteration,
									double Solution_Proximity,
									double Stop_Error_Th) {

	int num_1D_observations=TargetList_1D->numElements();
	int num_2D_observations=TargetList_2D->numElements();
	int num_3D_observations=TargetList_3D->numElements();
	int num_observations=(3*num_3D_observations)+(2*num_2D_observations)+num_1D_observations+dynamic_observations;
	
	if (show_debug_information) {
		cout<<"-> LM ("<<num_3D_observations+num_2D_observations+num_1D_observations<<" elements)\n";
		//cout<<"     "<<last_weighted_features+1<<" of them are weighted "<<added_sqrt_weight<<".\n";
		//cout<<"     "<<dynamic_observations<<" are reserved for dynamic observations.\n";
	}

	// Init
	times_upd=0;
	double *B=new double[num_observations];
	Temp_TargetList_1D=TargetList_1D;
	Temp_TargetList_2D=TargetList_2D;
	Temp_TargetList_3D=TargetList_3D;
	Zero_Anomalies=0;
	Tot_Calculations=0;
	double *Old_Solution=new double[num_Freedom_Degrees];
	

	// Set up starting point
	GetState_and_CurrentDegreesOfFreedom_Values(frame_index,IKState,Solution);
	for(int i=0;i<num_Freedom_Degrees;i++) {
		Calcolated_p[i]=Solution[i];
		Old_Solution[i]=Solution[i];
	}


	// Work on temp_frame
	temp_frame_index=frame_index;
	


	// Computa Measurament Vector (B)
	double *tmp_B=B;
	for(int i=0;i<num_2D_observations;i++) {
		// Get Point
		int    point_index   = (*TargetList_2D)[i].VertexIndex;
		double Target_Weight = (*TargetList_2D)[i].sqrt_weight;
		if (i<=last_weighted_features) Target_Weight*=added_sqrt_weight;

		// Set Obbiettivo
		(*(tmp_B++))=Target_Weight*(*TargetList_2D)[i].Target[0];
		(*(tmp_B++))=Target_Weight*(*TargetList_2D)[i].Target[1];
	}
	for(int i=0;i<num_1D_observations;i++)  {
		(*(tmp_B++))=((*TargetList_1D)[i].sqrt_weight)*((*TargetList_1D)[i].Target);
	}
	for(int i=0;i<num_3D_observations;i++)  {
		double Target_Weight = ((*TargetList_3D)[i].sqrt_weight);
		if ((*TargetList_3D)[i].VertexIndex_Target!=-1) { // TODOAAA
			(*(tmp_B++))=0.0;
			(*(tmp_B++))=0.0;
			(*(tmp_B++))=0.0;
		} else {
			(*(tmp_B++))=Target_Weight*((*TargetList_3D)[i].Target[0]);
			(*(tmp_B++))=Target_Weight*((*TargetList_3D)[i].Target[1]);
			(*(tmp_B++))=Target_Weight*((*TargetList_3D)[i].Target[2]);
		}
	}
	for(int i=0;i<dynamic_observations;i++) (*(tmp_B++))=0.0;

	
	
	// Non-Linear Least Square
	double opts[LM_OPTS_SZ+1];
	double info[LM_INFO_SZ+1];
	if (show_debug_information) cout<<"     Solution_Proximity "<<Solution_Proximity<<"\n";
	opts[0]=Solution_Proximity;
	opts[1]=1e-20;
	opts[2]=1e-20;
	opts[3]=Stop_Error_Th;


	LM_Mem_Storage=new double[2*LM_BC_DER_WORKSZ(num_Freedom_Degrees,num_observations)];
	// L'allocazione di memoria si puo ottimizzare ma LM tiene talmente tanto tempo che una
	// ottimizzazione dell'allocazione non cambia la velocita' effettiva di tutta la chiamata
	// con drawback nella memoria utilizzata
	// Nota: e' moltiplicata per due per sicurezza

	int ret=dlevmar_bc_der(IKSolver::LM_Function,IKSolver::LM_Jacobian,Solution,B,num_Freedom_Degrees,num_observations,lowerBound,upperBound,Num_of_Iteration,opts,info,LM_Mem_Storage,NULL,this);
	
	if (show_debug_information) {
		if (Tot_Calculations) cout<<"     Zero_Anomalies "<<Zero_Anomalies<<" on "<<Tot_Calculations<<" ("<<((100.0*Zero_Anomalies)/Tot_Calculations)<<"%)\n";
		cout<<"     ret "<<ret<<"\n";
		cout<<"     initial residual "<<info[0]<<"\n";
		cout<<"     final residual   "<<info[1]<<"\n";
		cout<<"     JT "<<info[2]<<"\n";
		cout<<"     DP "<<info[3]<<"\n";
		cout<<"     mu "<<info[4]<<"\n";
		cout<<"     iterations "<<info[5]<<"\n";
		cout<<"     J computed "<<info[8]<<" times\n";
		cout<<"     F computed "<<info[7]<<" times\n";
		cout<<"     updated    "<<times_upd<<" times\n";
		cout<<"     terminated because "<<LM_ERRORS[((int)info[6])-1]<<".\n";
	} else {
		if (show_minimal_debug_information) cout<<"   LM  "<<info[0]<<"  ->  "<<info[1]<<"\n";
	}

	// Controlla la validità del risultato
	bool valid_result=true;
	//if (info[6]==2) valid_result=false;

	double norm_diff_sol=0.0;
	for(int i=0;i<num_Freedom_Degrees;i++) norm_diff_sol+=fabs(Solution[i]-Old_Solution[i]);
	norm_diff_sol/=num_Freedom_Degrees;
	if (norm_diff_sol>M_PI*30/180) valid_result=false;		// TODO: sopra i 30° salta!!!
	if (show_debug_information) cout<<"     pose space movement "<<norm_diff_sol<<"\n";
		
	// Salva risultato
	if (valid_result) {
		LastComputedResidual=info[1];
		IKSolver::LM_Update(Solution,this,true);
	} else {
		LastComputedResidual=info[0];
		IKSolver::LM_Update(Old_Solution,this,true);
		if (show_debug_information) cout<<"Operation cancelled.\n";
		Beep(750,300);
	}
	mesh->Apply_Skin(frame_index);
	mesh->ComputaNormals();	
	

	delete []LM_Mem_Storage;
	delete []Old_Solution;
	delete []B;

	return true;
}



void IKSolver::GetCurrentDegreesOfFreedom_Values(int frame_index,double *Values_DOF) {
	Matrix Actual_Rotation(3,3);

	for(int k=0;k<num_Freedom_Degrees;k++) {
		Bone *Curr=Freedom_Degrees[k].Ref;
		int type=Freedom_Degrees[k].type;
		
		if (type<3) {
			RelativeRotation(Curr->RelativeTrasformationMatrix[frame_index],Curr->RelativeTrasformationMatrix[0],&Actual_Rotation);
			Vector<3> Rot=RotationMatrix2AngleAxis(&Actual_Rotation);
			
			Values_DOF[k]=Rot[type];
			while ((k+1<num_Freedom_Degrees) && (Freedom_Degrees[k+1].Ref==Curr) && (Freedom_Degrees[k+1].type<3)) {
				k++;
				Values_DOF[k]=Rot[Freedom_Degrees[k].type];
			}
		} else {
			Vector<3> Pos=Curr->RelativeTrasformationT[frame_index]-Curr->RelativeTrasformationT[0];
			
			Values_DOF[k]=Pos[type-3];
			while ((k+1<num_Freedom_Degrees) && (Freedom_Degrees[k+1].Ref==Curr) && (Freedom_Degrees[k+1].type>=3)) {
				k++;
				Values_DOF[k]=Pos[Freedom_Degrees[k].type-3];
			}
		}
	}
}


void IKSolver::GetState_and_CurrentDegreesOfFreedom_Values(int frame_index,double *IKState,double *Values_DOF) {
	mesh->GetCurrentBoneState(frame_index,IKState);

	double scale=1.0/VolumeScaleFactor;
	for(int k=0;k<num_Freedom_Degrees;k++) {
		if (Freedom_Degrees[k].type>=3) Values_DOF[k]=scale*(IKState[(Freedom_Degrees[k].BoneIndex*6)+Freedom_Degrees[k].type]-VolumeCenter[Freedom_Degrees[k].type-3]);
		else Values_DOF[k]=IKState[(Freedom_Degrees[k].BoneIndex*6)+Freedom_Degrees[k].type];
	}
}























inline void conic_f::Set(double sigma) {
	this->sigma=sigma;

	coeff_1=-((1.0-(2.0*sigma))/(4*sigma*sigma));
	coeff_2=-(1.0/(2.0*sigma));
	coeff_3=((3.0-2.0*sigma)/4.0);
}
inline double conic_f::g(double x) {
	if (x<-sigma) return (-x+1.0-sigma);
	if (x>sigma) return 0.0;

	return (coeff_1*x*x)+(coeff_2*x)+coeff_3;
}
inline double conic_f::dg(double x) {
	if (x<-sigma) return -1.0;
	if (x>sigma) return 0.0;

	return (2.0*coeff_1*x)+coeff_2;
}
inline double conic_f::F(Vector<3> x,Vector<3> o,Vector<3> d,double r) {
	Vector<3>     x_o=(x-o);
	double       dist=x_o*d;
	double          s=(x_o-(dist*d)).Norm2();
	double     l_dist=r-(r/sigma)*dist;

	double q;
	if (l_dist<=0.0) {
		q=0.0;
	} else { 
		q=(s/l_dist);
		if (q>=1.0) return 0.0;
	}

	if (dist<-sigma) return (1.0-q)*(-dist+1.0-sigma);
	if (dist>sigma) return 0.0;

	return (1.0-q)*((coeff_1*dist*dist)+(coeff_2*dist)+coeff_3);
}
Vector<3> conic_f::DF(Vector<3> x,Vector<3> o,Vector<3> d,double r) {
	A.Set(1.0-d[0]*d[0],-d[0]*d[1],-d[0]*d[2],
          -d[0]*d[1],1.0-d[1]*d[1],-d[1]*d[2],
		  -d[0]*d[2],-d[1]*d[2],1.0-d[2]*d[2]);

	Vector<3>     x_o=(x-o);
	double       dist=x_o*d;
	Vector<3> lat_prj=(x_o-(dist*d));
	double          s=lat_prj.Norm2();
	double     l_dist=r-(r/sigma)*dist;

	double q;
	if (l_dist<=0.0) {
		q=0.0;
	} else { 
		q=(s/l_dist);
		if (q>=1.0) return ZERO_3;
	}

	Vector<3> DS_riga;
	if (s==0) {
		DS_riga=ZERO_3;
	} else {
		Multiply(&A,&lat_prj,&DS_riga);
		DS_riga=(1.0/s)*DS_riga;
	}

	Vector<3> DQ_riga;
	if (l_dist<=0.0) {
		DQ_riga=ZERO_3;
	} else {
		double i_l_dist=1.0/l_dist;
		DQ_riga=i_l_dist*DS_riga;
		DQ_riga=DQ_riga+((i_l_dist*i_l_dist*s*r/sigma)*d);
	}

	Vector<3> df=(-g(dist))*DQ_riga+((1.0-q)*dg(dist))*d;
	
	return df;
}








































/*
inline double Raised_Cos(double x,double sigma) {
	if (x<-sigma) return  (-x+(1-sigma));
	if (x>sigma) return   0.0;

	return ((-0.5*(1-cos(((x+sigma)/(2*sigma))*M_PI)))+1.0);
}

inline double D_Raised_Cos(double x,double sigma) {
	if (x<-sigma) return -1.0;
	if (x>sigma) return   0.0;
	return (-(M_PI/(4.0*sigma))*sin((x+sigma)*M_PI/(2.0*sigma)));
}



double *Solution_w=NULL;
double *Calcolated_w=NULL;
bool *Valid_w=NULL;


bool IKSolver::Perspective_Solve_ALPHA_WEIGHTS(int frame_index,Array<TargetList_2D_Item_type_Frame_Info> *TargetList_2D,int Num_of_Iteration,double Solution_Proximity,double Stop_Error_Th) {
	int num_2D_observations=TargetList_2D->numElements();
	cout<<"-> Starting AW with "<<num_2D_observations<<" elements --\n";

	if (!Calcolated_w) {
		Calcolated_w=new double[mesh->num_p*mesh->num_bones];
		Valid_w=new bool[mesh->num_p*mesh->num_bones];
		Solution_w=new double[mesh->num_p*mesh->num_bones];
	}
	double *Old_Solution=new double[mesh->num_p*mesh->num_bones];

	// Init
	times_upd=0;
	double *B=new double[2*num_2D_observations];
	Temp_TargetList_FI=TargetList_2D;
	Zero_Anomalies=0;
	Tot_Calculations=0;
	

	// Set up starting point
	read_weights(mesh,Solution_w);
	for(int i=0;i<(mesh->num_p*mesh->num_bones);i++) {
		Calcolated_w[i]=Solution_w[i];
		Old_Solution[i]=Solution_w[i];
	}


	// Computa Measurament Vector (B)
	int current_frame=-1;
	for(int i=0;i<num_2D_observations;i++) {
		if ((*TargetList_2D)[i].frame!=current_frame) {
			current_frame=(*TargetList_2D)[i].frame;
			mesh->Apply_Skin(current_frame);
		}

		// Get Point
		int point_index=(*TargetList_2D)[i].VertexIndex;
		double Target_Weight=mesh->AreaPoint[point_index];
		if ((*TargetList_2D)[i].Weighted) Target_Weight*=added_sqrt_weight;

		// Set Obbiettivo
		B[i*2]=Target_Weight*(*TargetList_2D)[i].Target[0];
		B[i*2+1]=Target_Weight*(*TargetList_2D)[i].Target[1];
	}

	


	// Non-Linear Least Square
	double opts[4];
	double info[9];
	cout<<"   Solution_Proximity "<<Solution_Proximity<<"\n";
	opts[0]=Solution_Proximity; 
	opts[1]=1e-20; 
	opts[2]=1e-20; 
	opts[3]=Stop_Error_Th;
	
	#define LM_DER_WORKSZ(npar, nmeas) (2*(nmeas) + 4*(npar) + (nmeas)*(npar) + (npar)*(npar))
	LM_Mem_Storage=new double[2*LM_DER_WORKSZ(mesh->num_p*mesh->num_bones,2*num_2D_observations)];

	
	//int ret=dlevmar_der(IKSolver::LM_Function,IKSolver::LM_Jacobian,Solution,B,num_Freedom_Degrees,2*num_2D_observations,Num_of_Iteration,opts,info,LM_Mem_Storage,NULL,this); 
	int ret=dlevmar_bc_der(LM_Function_WEIGHTS,IKSolver::LM_Jacobian,Solution_w,B,num_Freedom_Degrees,2*num_2D_observations,lowerBound,upperBound,Num_of_Iteration,opts,info,LM_Mem_Storage,NULL,this); 
	

	cout<<"   Zero_Anomalies "<<Zero_Anomalies<<" on "<<Tot_Calculations<<" ("<<((100.0*Zero_Anomalies)/Tot_Calculations)<<"%)\n";
	cout<<"   ret "<<ret<<"\n";
	cout<<"   Initial e2 "<<info[0]<<"\n";
	cout<<"   e2 "<<info[1]<<"\n";
	cout<<"   JT "<<info[2]<<"\n";
	cout<<"   DP "<<info[3]<<"\n";
	cout<<"   mu "<<info[4]<<"\n";
	cout<<"   Done "<<info[5]<<" iterations\n";
	cout<<"   Call J "<<info[8]<<" times\n";
	cout<<"   Call F "<<info[7]<<" times\n";
	cout<<"   Updated "<<times_upd<<" times\n";
	cout<<"   Terminated because "<<LM_ERRORS[((int)info[6])-1]<<".\n";


////////////////////////////////////////////////////////////////////////////////
	// Controlla la validità del risultato
	bool valid_result=true;


		
	// Salva risultato
	if (valid_result) set_weights(mesh,Solution_w);
	else {set_weights(mesh,Old_Solution);cout<<"Operation cancelled.\n";Beep(750,300);}
	mesh->Apply_Skin(frame_index);
	mesh->ComputaNormals();
	

	delete LM_Mem_Storage;
	delete Old_Solution;
	delete B;
	return true;
}


void generate_assignement(Skinned_Mesh *m,double *p) {
	Array<ordered_pair> *o=new Array<ordered_pair>();
	
	for(int j=0;j<m->num_p;j++) {
		
	}

}


void read_weights(Skinned_Mesh *m,double *p) {
	for(int i=0;i<m->num_bones;i++) {
		for(int j=0;j<m->num_p;j++) {
			p[(i*(m->num_p))+j]=m->Weigth[j][i];
		}
	}

	
}

void set_weights(Skinned_Mesh *m,double *p) {
	for(int i=0;i<m->num_bones;i++) {
		for(int j=0;j<m->num_p;j++) {
			m->Weigth[j][i]=p[(i*(m->num_p))+j];
		}
	}
}


void IKSolver::LM_Function_WEIGHTS(double *p,double *x,int m,int n,void *data) {
	IKSolver *IK_OBJ=static_cast<IKSolver *>(data);
	int num_elements=IK_OBJ->Temp_TargetList_2D->numElements();
	Array<TargetList_2D_Item_type_Frame_Info> *TargetList_=IK_OBJ->Temp_TargetList_2D;
	List<HyperMesh<3>::PointType> *Points=&(IK_OBJ->mesh->Points);
	List<double> *AreaPoint=&(IK_OBJ->mesh->AreaPoint);

	// Update WEIGHTS
	set_weights(IK_OBJ->mesh,p);


	// Computa x
	Vector<3> tmp_p;
	Vector<2> PP2;

	int current_frame=-1;
	for(int i=0;i<numElements;i++) {
		if ((*TargetList_)[i].frame!=current_frame) {
			current_frame=(*TargetList_)[i].frame;
			IK_OBJ->mesh->Apply_Skin(current_frame);
		}
		
		int point_index=(*TargetList_)[i].VertexIndex;
		tmp_p=(*Points)[point_index];
		double Target_Weight=(*AreaPoint)[point_index];
		PP2=(*TargetList_)[i].View->Projection(tmp_p); 
		if ((*TargetList_)[i].Weighted) Target_Weight*=IK_OBJ->added_sqrt_weight;

		*(x++)=Target_Weight*PP2[0];
		*(x++)=Target_Weight*PP2[1];
	}

}


*/

