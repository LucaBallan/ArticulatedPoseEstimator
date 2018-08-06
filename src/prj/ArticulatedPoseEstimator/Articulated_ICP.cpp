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
#include "SalientPoints.h"
#include "IKSolver.h"
#include "PoseEstimator.h"
#include "Articulated_ICP.h"

using namespace std;
#pragma warning(3:4244)


#define GRADIENT_WINDOW_SIZE             7                      // Dimensione della finestra per la ricerca del gradiente
#define MEAN_MIN_PIXELS                  3                      // Movimento medio minimo dei punti
#define NUMERO_MIN_PER_MEDIA_VALIDA     20                      // Numero minimo di target per avere una media valida


#include "Articulated_ICP_Utils.h"
#include "Articulated_ICP_Correspondences.h"
#include "Articulated_ICP_Debug.h"



Articulated_ICP::Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
								 Features **FeaturesList,View *Views,int num_views,
								 Degrees_of_Freedom_Table_Entry *Constraint_level0,Degrees_of_Freedom_Table_Entry *Constraint_level1,
								 int debug_level) : PoseEstimator(Views,num_views) {
	
	this->GPU_OBJ=NULL;
	this->mesh=mesh;
	this->FeaturesList=FeaturesList;
	this->Constraint_level0=Constraint_level0;
	this->Constraint_level1=Constraint_level1;

	this->ModelRender=NULL;
	this->DepthRender=NULL;
	this->EdgesRenderer=NULL;
	Silhouette_FBO.valid=false;
	Texture_FBO.valid=false;
	Depth_FBO.valid=false;
	Edges_FBO.valid=false;
	this->SilhouetteImage=NULL;
	this->TextureImage=NULL;
	this->DepthImage=NULL;
	this->EdgesImage=NULL;
	ResetBufferStatus();

	this->IK_Solver=NULL;
	this->IK_Solver_Pyramid_Level_1=NULL;

	this->TargetList_1D=NULL;
	this->TargetList_2D=NULL;
	this->TargetList_3D=NULL;
	this->TargetList_3D_ContactPoints=NULL;
	this->Feat_TargetList=NULL;


	this->CorrVertexList=NULL;
	this->DebugImage=NULL;
	this->LastUsed_Feature_StartFrame=0;
	this->LastUsed_Feature_EndFrame=0;
	this->show_debug_information=true;
	this->show_minimal_debug_information=true;
	if (debug_level<2) this->show_debug_information=false;
	if (debug_level<1) this->show_minimal_debug_information=false;

	this->mean_move=NULL;
	this->mean_move_num=NULL;

	this->collision_mask=NULL;
	this->collision_detector=NULL;
	this->collision_intersecting_faces=NULL;

	this->DistanceTransform_Buffers=NULL;

	// GPU_OBJ
	GPU_OBJ=new AdvancedRenderWindow(NULL,NULL,NULL,NULL,0x000000,NULL,WS_OVERLAPPEDWINDOW,RENDERWINDOW_DO_NOT_SHOW_INFO);

	// Init
	SNEWA(CorrVertexList,int,mesh->num_p);
	mesh->ComputaNormals();
	mesh->BoneRoot->CalcRelativeFromAbsolute(0);

	if (Constraint_level1!=NULL) {
		mesh->SetConstraints(Constraint_level1);
		IK_Solver_Pyramid_Level_1=new IKSolver(mesh,RoomCenter,RoomRadius,show_debug_information,show_minimal_debug_information);
	} 

	if (Constraint_level0!=NULL) mesh->SetConstraints(Constraint_level0);
	IK_Solver=new IKSolver(mesh,RoomCenter,RoomRadius,show_debug_information,show_minimal_debug_information);
	

	// Liste
	SNEW(TargetList_1D,Array<TargetList_1D_Item_type>(100));
	SNEW(TargetList_2D,Array<TargetList_2D_Item_type>(100));
	SNEW(TargetList_3D,Array<TargetList_3D_Item_type>(100));
	SNEW(TargetList_3D_ContactPoints,Array<TargetList_3D_Item_type>(100));
	SNEW(Feat_TargetList,Array<TargetList_2D_Item_type>(100));
	

	// Contour.h
	SNEWA(mean_move,double,mesh->num_bones);
	SNEWA(mean_move_num,unsigned int,mesh->num_bones);


	// Renderers
	SNEW(ModelRender,ModelRenderer(GPU_OBJ));
	SNEW(DepthRender,DepthMapRenderer(GPU_OBJ));
	DepthRender->set_empty_pixel_to_infinity=true;
	SNEW(EdgesRenderer,DepthEdgesDetectorRenderer(GPU_OBJ));


	if (INVALID_INTERNAL_EDGES) {
		cout<<"***************************************************************************************"<<endl;
		cout<<"***************************************************************************************"<<endl;
		Warning("INVALID_INTERNAL_EDGES settato to true");
		cout<<"***************************************************************************************"<<endl;
		cout<<"***************************************************************************************"<<endl;
	}
}



Articulated_ICP::~Articulated_ICP() {
	SDELETEA_REC(DistanceTransform_Buffers,num_views);

	SDELETE(IK_Solver);
	SDELETE(IK_Solver_Pyramid_Level_1);

	SDELETEA(mean_move_num);
	SDELETEA(mean_move);

	SDELETE(collision_intersecting_faces);

	SDELETE(TargetList_1D);
	SDELETE(TargetList_2D);
	SDELETE(TargetList_3D);
	SDELETE(TargetList_3D_ContactPoints);
	SDELETE(Feat_TargetList);

	SDELETEA(CorrVertexList);
	SDELETE(DebugImage);


	SDELETE(ModelRender);
	SDELETE(DepthRender);
	SDELETE(EdgesRenderer);
	SDELETE(SilhouetteImage);
	SDELETE(TextureImage);
	SDELETE(DepthImage);
	SDELETE(EdgesImage);
	

//  Note: NVIDIA goes in deadlock here if a task that was using GPU_OBJ
//  still exists. Before deleting Articulated_ICP, please delete the tasks.
	GPU_OBJ->DeleteFrameBuffer(&Edges_FBO);
	GPU_OBJ->DeleteFrameBuffer(&Depth_FBO);
	GPU_OBJ->DeleteFrameBuffer(&Texture_FBO);
	GPU_OBJ->DeleteFrameBuffer(&Silhouette_FBO);
	SDELETE(GPU_OBJ);
}




























////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// INIT //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IKSolver *Articulated_ICP::GetIKSolverL0() {
	return IK_Solver;
}

Array<TargetList_2D_Item_type> *Articulated_ICP::GetFeat_TargetList() {
	return Feat_TargetList;
}

void Articulated_ICP::SetCollisionDetector(HyperMeshPartitionTree *collision_detector,int collision_detector_level,bool *collision_mask,double collision_local_weight_l0,double collision_local_weight_l1,double collision_sigma,int max_collision_observations,int ttl) {
	this->collision_detector=collision_detector;
	this->collision_mask=collision_mask;
	SNEW(collision_intersecting_faces,Array<ordered_pair>(mesh->num_f));

	if (IK_Solver) {
		IK_Solver->SetDynamic_Observations(max_collision_observations);
		IK_Solver->SetCollisionDetector(collision_detector,collision_mask,collision_intersecting_faces);
		IK_Solver->SetCollisionParameters(ttl,RANDOMIZE_TTL,collision_local_weight_l0,collision_sigma);
		IK_Solver->Dynamic_Collision_Update(false);
	}
	if (IK_Solver_Pyramid_Level_1) {
		IK_Solver_Pyramid_Level_1->SetDynamic_Observations(max_collision_observations);
		IK_Solver_Pyramid_Level_1->SetCollisionDetector(collision_detector,collision_mask,collision_intersecting_faces);
		IK_Solver_Pyramid_Level_1->SetCollisionParameters(ttl,RANDOMIZE_TTL,collision_local_weight_l1,collision_sigma);
		IK_Solver_Pyramid_Level_1->Dynamic_Collision_Update(false);
	}
}







































////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// ADAPTATION ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::Adapt_Silhouette_FBO_size(int width,int height) {
	if ((width!=Silhouette_FBO.w) || (height!=Silhouette_FBO.h) || (!Silhouette_FBO.valid)) {
		GPU_OBJ->DeleteFrameBuffer(&Silhouette_FBO);
		Silhouette_FBO=ModelRender->BuildRenderFrameBufferFootPrint(width,height);
		SDELETE(SilhouetteImage);
		SilhouetteImage=new Bitmap<GreyLevel>(width,height);
	}
}

void Articulated_ICP::Adapt_Texture_FBO_size(int width,int height) {
	if ((width!=Texture_FBO.w) || (height!=Texture_FBO.h) || (!Texture_FBO.valid)) {
		GPU_OBJ->DeleteFrameBuffer(&Texture_FBO);
		Texture_FBO=ModelRender->BuildRenderFrameBufferTex(width,height);
		SDELETE(TextureImage);
		TextureImage=new Bitmap<ColorRGBA>(width,height);
	}
}

void Articulated_ICP::Adapt_Depth_FBO_size(int width,int height) {
	if ((width!=Depth_FBO.w) || (height!=Depth_FBO.h) || (!Depth_FBO.valid)) {
		GPU_OBJ->DeleteFrameBuffer(&Depth_FBO);
		Depth_FBO=DepthRender->BuildDepthFrameBuffer(width,height);
		SDELETE(DepthImage);
		DepthImage=new Bitmap<ColorFloat>(width,height);
	}
}

void Articulated_ICP::Adapt_Edges_FBO_size(int width,int height) {
	if ((width!=Edges_FBO.w) || (height!=Edges_FBO.h) || (!Edges_FBO.valid)) {
		GPU_OBJ->DeleteFrameBuffer(&Edges_FBO);
		Edges_FBO=EdgesRenderer->BuildFloatFrameBuffer(width,height);
		SDELETE(EdgesImage);
		EdgesImage=new Bitmap<ColorFloat>(width,height);
	}
}

































////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// ESTIMATE //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::PrepareMesh(int frame_index) {
	mesh->SetConstraints(Constraint_level0);							// Setta i vincoli del livello 0
	if (Apply_Constraints) mesh->ApplyConstraints(frame_index);			// Applica i vincoli del livello 0
	mesh->Apply_Skin(frame_index);
	mesh->ComputaNormals();
	mesh->ComputaArea();
}


double Articulated_ICP::EstimateFrame(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction) {
	if (Refinement) {
		Warning("TODO: I've never tested this function CHECK!!!.");
		if ((frame_index==base_frame_index) && (frame_index>1)) base_frame_index=frame_index-1;
		if (base_frame_index<1) base_frame_index=1; // Errors!!

		PrepareMesh(base_frame_index);
		GetOpticalFlowCorrespondences(frame_index,base_frame_index);
		PrepareMesh(frame_index);
	} else {
		if (frame_index==base_frame_index) Warning("Can't use optical flow here, use the function RefineFrame instead.");
		
		if ((UsePrediction) && (base_frame_index>4)) {
			PredictFrame(frame_index,base_frame_index);
			PrepareMesh(base_frame_index);
			GetOpticalFlowCorrespondences(frame_index,base_frame_index);
			PrepareMesh(frame_index);
		} else {
			mesh->BoneRoot->Copy(base_frame_index,frame_index);
			PrepareMesh(frame_index);
			GetOpticalFlowCorrespondences(frame_index,base_frame_index);
		}
	}

	// Get Contact Points
	if (contact_weight!=0.0) {
		IK_Solver->Contact_Points_Increase_Time(TargetList_3D_ContactPoints,contact_weight,contact_ttl);
		IK_Solver->Add_Contact_Points(TargetList_3D_ContactPoints,contact_weight,contact_ttl);
	}

	return Process(frame_index);
}

void Articulated_ICP::ResetContactPoints() {
	TargetList_3D_ContactPoints->clear();
}

double Articulated_ICP::Process(int frame_index) {

	int iteration_no=0;
	int tot_iterations=iteration_l1+iteration_l0;
	int last_of_interation=Approx((OpticalFlowIterations/100.0)*tot_iterations);
	bool IK_result=true;
	double LastComputedResidual=DBL_MAX;



	// Clear Collisions
	IK_Solver->Clear_Collision_Constraints();
	if (IK_Solver_Pyramid_Level_1) IK_Solver_Pyramid_Level_1->Clear_Collision_Constraints();



	// Livello 1 della piramide (semplificato)
	if (IK_Solver_Pyramid_Level_1) {
		IK_Solver_Pyramid_Level_1->added_sqrt_weight=1.0;
		IK_Solver_Pyramid_Level_1->last_weighted_features=(Feat_TargetList->numElements()-1);
		mesh->SetConstraints(Constraint_level1);
		
		for(int i=0;((i<iteration_l1)&&(IK_result));i++) {
			iteration_no++;
			if (show_minimal_debug_information) {
				cout<<"\n-> Iteration "<<iteration_no<<" (L1)\n";
				cout<<"------------------\n";
			}
			if (iteration_no>last_of_interation) IK_Solver_Pyramid_Level_1->added_sqrt_weight=0.0;


			// Calcola errore
			RetrieveObservations(1,frame_index);
			// if (incoerenza<..) break;	


			// Solve 
			IK_result=IK_Solver_Pyramid_Level_1->Perspective_Solve_LM(frame_index,TargetList_1D,TargetList_2D,TargetList_3D,internal_iteration,proximity_factor,Stop_Error_Th);
			LastComputedResidual=IK_Solver_Pyramid_Level_1->LastComputedResidual;


			// Debug
			if ((DebugImages&PEDEBUG_ITERATIONS)||((DebugImages&PEDEBUG_FRAMES)&&(iteration_l0==0)&&(i==iteration_l1-1))) {
				WriteDebugImages(1,frame_index);
				
				if (DebugImages&PEDEBUG_ITERATIONS) {
					char Text[100];
					sprintf(Text,"%s\\motion\\L1_%04i_%04i.motion",OutDebugDir,frame_index,DebugIndex);
					OFileBuffer Out(Text);
					Bone::SaveSkullMotion(&Out,mesh->BoneRoot);
					Out.Close();
					DebugIndex++;
				}
			}
		}
	}

	

	// Livello 0 della piramide
	IK_result=true;
	IK_Solver->added_sqrt_weight=1.0;
	IK_Solver->last_weighted_features=(Feat_TargetList->numElements()-1);
	mesh->SetConstraints(Constraint_level0);
	if ((iteration_l0==0) && (iteration_l1==0) && (DebugImages!=0)) WriteDebugImages(0,frame_index);


	// Copy Collisions
	if (IK_Solver_Pyramid_Level_1) {
		IK_Solver->Copy_Collision_Constraints(IK_Solver_Pyramid_Level_1);
	}


	for(int i=0;((i<iteration_l0)&&(IK_result));i++) {
		iteration_no++;
		if (show_minimal_debug_information) {
			cout<<"\n-> Iteration "<<iteration_no<<"\n";
			cout<<"------------------\n";
		}
		if (iteration_no>last_of_interation) IK_Solver->added_sqrt_weight=0.0;
		

		// Calcola errore
		RetrieveObservations(0,frame_index);
		// if (incoerenza<..) break;


		// Solve
		if (i>=iteration_l0-n_iter_find_outliers) {
			if (show_minimal_debug_information) cout<<"-> Outliers detection\n";
			Array<TargetList_2D_Item_type> *New_TargetList=new Array<TargetList_2D_Item_type>(TargetList_2D->numElements());
			
			for(int h=0;h<TargetList_2D->numElements();h++) {
				Vector<2> a=(*TargetList_2D)[h].View->Projection(mesh->Points[(*TargetList_2D)[h].VertexIndex]);
				Vector<2> b=(*TargetList_2D)[h].Target;
				if ((b-a).Norm2()<outliers_thresh) New_TargetList->append((*TargetList_2D)[h]);
			}
			if (show_minimal_debug_information) cout<<"    Inliers: "<<New_TargetList->numElements()<<" ("<<TargetList_2D->numElements()-New_TargetList->numElements()<<" rejected)\n";

			IK_result=IK_Solver->Perspective_Solve_LM(frame_index,TargetList_1D,New_TargetList,TargetList_3D,internal_iteration,proximity_factor,Stop_Error_Th);
			delete New_TargetList;
		} else {
			IK_result=IK_Solver->Perspective_Solve_LM(frame_index,TargetList_1D,TargetList_2D,TargetList_3D,internal_iteration,proximity_factor,Stop_Error_Th);
		}
		LastComputedResidual=IK_Solver->LastComputedResidual;




		// Debug
		if ((DebugImages&PEDEBUG_ITERATIONS)||((DebugImages&PEDEBUG_FRAMES)&&(i==iteration_l0-1))) {
			WriteDebugImages(0,frame_index);

			if (DebugImages&PEDEBUG_ITERATIONS) {
				char Text[100];
				sprintf(Text,"%s\\motion\\LF_%04i_%04i.motion",OutDebugDir,frame_index,DebugIndex);
				OFileBuffer Out(Text);
				Bone::SaveSkullMotion(&Out,mesh->BoneRoot);
				Out.Close();
				DebugIndex++;
			}
		}
	}

	if (DebugImages&PEDEBUG_FRAMES) {
		DebugIndex++;
		char Text[100];
		sprintf(Text,"%s\\motion\\LF_%04i.motion",OutDebugDir,frame_index);
		OFileBuffer Out(Text);
		Bone::SaveSkullMotion(&Out,mesh->BoneRoot);
		Out.Close();
	}

	return LastComputedResidual;
}




double Articulated_ICP::GetResidual(int frame_index,bool write_debug_images) {
	PrepareMesh(frame_index);
	mesh->SetConstraints(Constraint_level0);

	// Delete list
	TargetList_1D->clear();
	TargetList_2D->clear();
	TargetList_3D->clear();
	

	// Get Observations
	int remaining_correspondences=0;
	TargetList_2D->append(Feat_TargetList);
	for(int i=0;i<num_views;i++) {
		ResetBufferStatus();
		int num_correspondences=TargetList_2D->numElements();
		GetSalientPointCorrespondences(i,0,frame_index);
		num_correspondences=TargetList_2D->numElements()-num_correspondences;
		if ((Views[i].Enable) && (salient_points_2D!=NULL) && (salient_points_2D[i]!=NULL)) {
			Array<Vector<2>> *points_2D=salient_points_2D[i]->get_tracks_read_only(frame_index);
			remaining_correspondences+=(points_2D->numElements()-num_correspondences);
		}
	}

	IK_Solver->Clear_Collision_Constraints();
	IK_Solver->Add_Collision_Constraints();


	double current_opticalflow_weight=opticalflow_local_weight;
	double current_collision_weight=IK_Solver->GetCollisionWeight();
	double current_depth_weight=depth_local_weight_l0;

	double Residual_S=0.0,
		   Residual_E=0.0,
		   Residual_P=0.0,
		   Residual_O=0.0,
		   Residual_C=0.0,
		   Residual_D=0.0,
		   Residual_T=0.0;
	int    n_observations_S=0,
		   n_observations_E=0,
		   n_observations_P=0,
		   n_observations_O=0,
		   n_observations_C=0,
		   n_observations_D=0,
		   n_observations_T=0;

	SilhouetteEdgesSalientPointsOpticalFlow_CorrespondencesResidual(Residual_S,Residual_E,Residual_P,Residual_O,&n_observations_S,&n_observations_E,&n_observations_P,&n_observations_O);
	Residual_P*=salient_points_local_weight;
	Residual_O*=current_opticalflow_weight;
	Residual_C=current_collision_weight*IK_Solver->Collision_Residual(&n_observations_C);

	Residual_S=Residual_E=0.0;
	double Residual=Residual_S+Residual_E+Residual_P+Residual_O+Residual_C+remaining_correspondences;

	if (write_debug_images) WriteDebugImages(0,frame_index);

	return Residual;
}
