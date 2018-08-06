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
#include "Stocastic_Articulated_ICP.h"

using namespace std;
#pragma warning(3:4244)

#define MAX_NOISE_THRESHOLD 1e+10   // Do not use DBL_MAX -> it has to be inputed from a text file
									// it will never read DBL_MAX
#define USE_GENETIC_APPROACH

Stocastic_Articulated_ICP::Stocastic_Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
													 Features **FeaturesList,View *Views,int num_views,
													 Degrees_of_Freedom_Table_Entry *Constraint_level0,Degrees_of_Freedom_Table_Entry *Constraint_level1,
													 int debug_level,int particle_debug_level,
													 int num_processors,int num_particles_per_iteration,int resample_elements) : PoseEstimator(Views,num_views) {
	this->main_mesh=mesh;
	this->mesh_buffer=NULL;
	this->mesh_buffer_cd=NULL;
	this->AICP=NULL;
	this->Task=NULL;
	this->TaskFinished=NULL;
	this->IKState=NULL;
	this->ABSIKState=NULL;
	this->tmp_frame_index=0;
	this->tmp_debug_index=0;
	
	// Correct this->num_processors & num_processors
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int actual_num_processors=(int)sysinfo.dwNumberOfProcessors;
	if ((num_processors<1) || (num_processors>actual_num_processors)) num_processors=actual_num_processors;
	this->num_processors=num_processors;

	// Correct this->num_particles & num_particles_per_iteration
	this->num_particles=Approx(((double)num_particles_per_iteration)/num_processors)*num_processors;
	if (this->num_particles==0) {
		this->num_processors=num_processors=1;
		this->num_particles=max(num_particles_per_iteration,1);
	}
	num_particles_per_iteration=this->num_particles;
	
	// Correct resample_elements
	if (resample_elements<=0) resample_elements=1;
	if (resample_elements>this->num_particles) resample_elements=this->num_particles;


	this->num_Freedom_Degrees=0;
	this->Freedom_Degrees=NULL;
	this->IK_Solver_Ref=NULL;
	this->VolumeCenter.SetZero();
	this->VolumeScaleFactor=0.0;

	this->ParticleCounter=0;
	this->Particles=NULL;
	this->ParticleChart=NULL;
	this->dof_noise_ratio=NULL;

	this->show_debug_information=true;
	this->show_minimal_debug_information=true;
	if (debug_level<2) this->show_debug_information=false;
	if (debug_level<1) this->show_minimal_debug_information=false;
	if (particle_debug_level>debug_level) particle_debug_level=debug_level;
	
	if (this->show_minimal_debug_information) {
		cout<<"Pose Estimator initizializing:\n";
		cout<<"    "<<this->num_processors<<" cpu cores\n";
		cout<<"    "<<this->num_particles<<" particles per iteration\n";
	}

	
	// Init Main_mesh
	main_mesh->BoneRoot->CalcRelativeFromAbsolute(0);
	main_mesh->SetConstraints(Constraint_level0);
	main_mesh->BoneRoot->UpdateTreeLevels();


	if (show_minimal_debug_information) cout<<"Setting up local minimizers...\n";

	// Meshes
	SNEWA_P(mesh_buffer,Skinned_Mesh*,num_processors);
	for(int i=0;i<num_processors;i++) {
		SNEW(mesh_buffer[i],Skinned_Mesh(main_mesh));
		mesh_buffer[i]->BoneRoot->UpdateTreeLevels();
	}
	
	// IKState
	SNEWA_P(IKState,double*,num_processors);
	for(int i=0;i<num_processors;i++) SNEWA(IKState[i],double,(6*main_mesh->num_bones));
	SNEWA_P(mesh_buffer_cd,HyperMeshPartitionTree*,num_processors);
	SNEWA(ABSIKState,double,(6*main_mesh->num_bones));

	// AICPs
	Features **tmp_featlist=FeaturesList;
	SNEWA_P(AICP,Articulated_ICP*,num_processors);
	for(int i=0;i<num_processors;i++) {
		AICP[i]=new Articulated_ICP(mesh_buffer[i],RoomCenter,RoomRadius,tmp_featlist,Views,num_views,Constraint_level0,Constraint_level1,particle_debug_level);
		tmp_featlist=NULL;
	}
	
	// Tasks
	Task=new MultiProcessing(num_processors,MultiProcessing::Interval_Split,(TASK_FUNCTION)TaskMinimize,this);
	TaskFinished=CreateEvent(NULL,TRUE,TRUE,NULL);


	// IKSolver Object (ReadOnly)
	IK_Solver_Ref=AICP[0]->GetIKSolverL0();
	IK_Solver_Ref->GetFreedomDegressMap(num_Freedom_Degrees,Freedom_Degrees);
	VolumeScaleFactor=IK_Solver_Ref->GetVolumeScaleFactorAndVolumeCenter(VolumeCenter);


	// Particles
	SNEWA(Particles,ParticleElement,this->num_particles);
	for(int i=0;i<this->num_particles;i++) {
		Particles[i].State=NULL;SNEWA(Particles[i].State,double,num_Freedom_Degrees);
		Particles[i].Speed=NULL;SNEWA(Particles[i].Speed,double,num_Freedom_Degrees);
		Particles[i].Local_Optimum=NULL;SNEWA(Particles[i].Local_Optimum,double,num_Freedom_Degrees);
	}
	ParticleChart=new LowestScoreChart(resample_elements,sizeof(double)*num_Freedom_Degrees);


	// Noise
	SNEWA(dof_noise_ratio,double,num_Freedom_Degrees);
	

	// Default Parameters
	this->initial_iteration_l0=0;
	this->initial_iteration_l1=0;
	this->resample_k=0.05f;
	this->resample_policy=SAICP_POLICY_WTA;
	this->noise_ratio=0.04;
	this->noise_decrease_ratio=0.20;
	this->particle_internal_iteration_ratio=0.3;
	this->apply_noise_on_root=false;
}
Stocastic_Articulated_ICP::~Stocastic_Articulated_ICP() {
	if (Task) {
		// Before delete the tasks and then the GPU contexes (AICPs)
		WAIT_EVENT(TaskFinished);
		SDELETE(Task);
	} 
	CloseHandle(TaskFinished);
	SDELETEA_REC(AICP,num_processors);

	SDELETEA(dof_noise_ratio);

	for(int i=0;i<this->num_particles;i++) {
		SDELETEA(Particles[i].State);
		SDELETEA(Particles[i].Speed);
		SDELETEA(Particles[i].Local_Optimum);
	}
	SDELETEA(Particles);
	SDELETE(ParticleChart);
	SDELETEA(Freedom_Degrees);

	SDELETEA_REC(mesh_buffer_cd,num_processors);
	SDELETEA_REC(mesh_buffer,num_processors);
	SDELETEA_REC(IKState,num_processors);
	SDELETEA(ABSIKState);
}








void Stocastic_Articulated_ICP::SetCollisionDetector(HyperMeshPartitionTree *collision_detector,int collision_detector_level,bool *collision_mask,
													 double collision_local_weight_l0,double collision_local_weight_l1,double collision_sigma,int max_collision_observations,int ttl) {
	
	for(int i=0;i<num_processors;i++) {
		SDELETE(mesh_buffer_cd[i]);
		mesh_buffer_cd[i]=HyperMeshPartitionTree::Generate(mesh_buffer[i],collision_detector_level);
		AICP[i]->SetCollisionDetector(mesh_buffer_cd[i],collision_detector_level,collision_mask,collision_local_weight_l0,collision_local_weight_l1,collision_sigma,max_collision_observations,ttl);
	}
}







void Stocastic_Articulated_ICP::GetState(Skinned_Mesh *mesh,int frame_index,double *IKState) {
	mesh->GetCurrentBoneState(frame_index,IKState);
}

void Stocastic_Articulated_ICP::SetState(Skinned_Mesh *mesh,int frame_index,double *IKState) {
	mesh->BoneRoot->Copy(0,frame_index);

	Vector<3> s;
	for(int k=0;k<mesh->num_bones;k++) {
		Bone *Curr=mesh->BoneRefTable[k];
		s[0]=*(IKState++);
		s[1]=*(IKState++);
		s[2]=*(IKState++);
		Curr->Rotate(frame_index,s);
		s[0]=*(IKState++);
		s[1]=*(IKState++);
		s[2]=*(IKState++);
		Curr->Translate(frame_index,s);
	}

	// Update ABS
	mesh->BoneRoot->CalcAbsoluteFromRelative(frame_index);	
}

void Stocastic_Articulated_ICP::GetDOFState(Skinned_Mesh *mesh,int frame_index,double *IKState,double *DOF) {
	GetState(mesh,frame_index,IKState);

	double scale=1.0/VolumeScaleFactor;
	for(int k=0;k<num_Freedom_Degrees;k++) {
		if (Freedom_Degrees[k].type>=3) DOF[k]=scale*(IKState[(Freedom_Degrees[k].BoneIndex*6)+Freedom_Degrees[k].type]-VolumeCenter[Freedom_Degrees[k].type-3]);
		else DOF[k]=IKState[(Freedom_Degrees[k].BoneIndex*6)+Freedom_Degrees[k].type];
	}
}

void Stocastic_Articulated_ICP::SetDOFState(Skinned_Mesh *mesh,int frame_index,double *IKState,double *DOF) {
	Freedom_Degrees_type *DOF_tmp=Freedom_Degrees;
	double *IK_State_tmp=IKState;
	double scale=VolumeScaleFactor;

	for(int k=0;k<num_Freedom_Degrees;k++) {
		if (DOF_tmp->type>=3) IK_State_tmp[(DOF_tmp->BoneIndex*6)+DOF_tmp->type]=(scale*DOF[k])+VolumeCenter[DOF_tmp->type-3];
		else IK_State_tmp[(DOF_tmp->BoneIndex*6)+DOF_tmp->type]=DOF[k];
		
		DOF_tmp++;
	}

	SetState(mesh,frame_index,IKState);
}




void Stocastic_Articulated_ICP::TaskMinimize(void *data,Stocastic_Articulated_ICP *saicp) {
	int first,last;
	bscanf((BYTE*)data,"ii",&first,&last);

	int proc=first/(saicp->num_particles/saicp->num_processors);
	for(int i=first;i<=last;i++) {
		saicp->AICP[proc]->DebugIndex=saicp->tmp_debug_index;
		saicp->Minimize(i,proc,saicp->tmp_frame_index);
	}
}

void Stocastic_Articulated_ICP::Minimize(int particle_j,int proc,int frame) {
	Articulated_ICP *CAICP=AICP[proc];
	Skinned_Mesh    *CMesh=mesh_buffer[proc];
	double          *CIKState=IKState[proc];
	ParticleElement *CParticle=&(Particles[particle_j]);
	double residual;


	if (CAICP->DebugImages!=0) {
		char out_dir_path[500];
		if (CAICP->DebugImages==PEDEBUG_FRAMES) strcpy(out_dir_path,OutDebugDir);
		else sprintf(out_dir_path,"%s\\Frame_%06i_P_%02i",OutDebugDir,frame,particle_j);
		CAICP->SetOutDebugDir(out_dir_path,false);
	}

	// Local optimization
	for(int i=0;i<(6*main_mesh->num_bones);i++) CIKState[i]=ABSIKState[i];
	SetDOFState(CMesh,frame,CIKState,CParticle->State);
	CAICP->PrepareMesh(frame);
	residual=CAICP->Process(frame);
	GetDOFState(CMesh,frame,CIKState,CParticle->State);


	// Error Evaluation Request
	if ((CAICP->iteration_l0==0) && (CAICP->iteration_l1==0)) residual=0.0;
	else residual=local_optimization_weight*residual;


	// Global errors
	// CMesh->Apply_Skin(frame);	// This is not necessary.
	// CMesh->ComputaArea();        // It is better to use the old one. More coherent with the residual.
	CParticle->Residual_S=0.0;
	CParticle->Residual_D=0.0;
	CParticle->Residual_O=0.0;
	CParticle->Residual_C=0.0;
	CParticle->Residual_T=0.0;
	CParticle->Residual_E=0.0;
	CParticle->Residual_P=0.0;
	CParticle->n_observations_S=0.0;
	CParticle->n_observations_D=0.0;
	CParticle->n_observations_O=0.0;
	CParticle->n_observations_T=0.0;
	CParticle->n_observations_E=0.0;
	CParticle->n_observations_P=0.0;

	CParticle->Residual_S=CAICP->ComputaSilhouetteResidual(&(CParticle->n_observations_S));
	if (!_finite(CParticle->Residual_S)) {
		Warning("Object completely out of one view. Don't know what to do."); // TODO funzionera' ancora dopo il cambiamanto? o devo cercare un'altro?
	}
	if (depth_weight!=0.0)          CParticle->Residual_D=depth_weight*CAICP->ComputaDepthResidual(&(CParticle->n_observations_D));
	if (opticalflow_weight!=0.0)    CParticle->Residual_O=opticalflow_weight*CAICP->ComputaOpticalFlowResidual(&(CParticle->n_observations_O));
	if (collision_weight!=0.0)      CParticle->Residual_C=collision_weight*CAICP->ComputaCollisionsResidual();
	if (texture_weight!=0.0)        CParticle->Residual_T=texture_weight*CAICP->ComputaTextureResidual(&(CParticle->n_observations_T));
	if (salient_points_weight!=0.0) CParticle->Residual_P=salient_points_weight*CAICP->ComputaSalientPointResidual(&(CParticle->n_observations_P),frame);
	if (edges_weight!=0.0)          CParticle->Residual_E=edges_weight*CAICP->ComputaEdgeResidual(&(CParticle->n_observations_E));

	residual+=CParticle->Residual_S+CParticle->Residual_D+CParticle->Residual_O+CParticle->Residual_C+CParticle->Residual_T+CParticle->Residual_E+CParticle->Residual_P;

	// Debug
	if ((show_debug_information) && (CParticle->Residual<residual) 
		&& ((CAICP->iteration_l0) || (CAICP->iteration_l1))) cout<<"        Particle #"<<CParticle->Index<<" went up the hill during local minimization\n";


	CParticle->Residual=residual;
}


#define SEARCH_PARTICLE_J														\
int particle_j=0;																\
for(;particle_j<num_particles;particle_j++) {									\
	if ((Particles[particle_j].Index==ParticleChart->index[0]) &&				\
		(Particles[particle_j].Residual==ParticleChart->score[0])) break;		\
}


void Stocastic_Articulated_ICP::ResetContactPoints() {
	for(int i=0;i<num_processors;i++) AICP[i]->ResetContactPoints();
}







// Note:
//  IKState				    represents all the dof of the skeleton (everything) same as CopyABSPoseFrom
//  Particle				represents only the movable dof	


double Stocastic_Articulated_ICP::EstimateFrame(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction) {
	Profiler *IterationTime=NULL;
	bool NoLocalOptimization=false;

	// Decide parameters
	int L0_inc,L1_inc;
	int L0_tot=0,L1_tot=0;
	if ((_finite(particle_internal_iteration_ratio)) && (iteration_l0+iteration_l1==0)) {
		return EstimateFramePSO(frame_index,base_frame_index,Refinement,UsePrediction); // TODOLUCA
		iteration_l0=Approx(1.0/particle_internal_iteration_ratio);
		NoLocalOptimization=true;
		L0_inc=1;
		L1_inc=0;
	} else {
		L0_inc=Approx(iteration_l0*particle_internal_iteration_ratio);
		L1_inc=Approx(iteration_l1*particle_internal_iteration_ratio);
		if ((iteration_l0!=0) && (particle_internal_iteration_ratio!=0.0)) L0_inc=max(1,L0_inc);
		if ((iteration_l1!=0) && (particle_internal_iteration_ratio!=0.0)) L1_inc=max(1,L1_inc);
	}

	for(int i=0;i<num_processors;i++) {
		CopyParametersTo(AICP[i],false);
		if (DebugImages==PEDEBUG_ITERATIONS) AICP[i]->DebugImages=PEDEBUG_ITERATIONS;
		else AICP[i]->DebugImages=PEDEBUG_NEVER;

		AICP[i]->PrepareFrameForSilhouetteResidual();
	}
	

	// Debug Information
	if (show_debug_information) {
		if ((iteration_l0!=0) || (iteration_l1!=0)) {
			cout<<"\n--------------------------\n";
			cout<<"Computing Frame "<<frame_index<<"\n";
			cout<<"--------------------------\n\n";
			if (NoLocalOptimization) {
				cout<<"    Resampling "<<iteration_l0<<" times (no local optimization)\n\n";
			} else {
				cout<<"    Running "<<iteration_l1<<" iterations at level 1  (resample every "<<L1_inc<<")\n";
				cout<<"            "<<iteration_l0<<" iterations at level 0  (resample every "<<L0_inc<<")\n\n";
			}
		} else {
			cout<<"\n--------------------------\n";
			cout<<"Frame "<<frame_index<<"\n";
			cout<<"--------------------------\n\n";
		}
	} else {
		if (show_minimal_debug_information) {
			cout<<"Frame "<<frame_index<<":\n";
		}
	}
	if (show_minimal_debug_information) IterationTime=new Profiler;





	//
	// Init Particle[0]
	//
	ParticleCounter=1;
	Particles[0].Index=0;
	Particles[0].Residual=DBL_MAX;
	Particles[0].Type=0;
	main_mesh->BoneRoot->CalcRelativeFromAbsolute(base_frame_index);
	GetDOFState(main_mesh,base_frame_index,ABSIKState,Particles[0].State);

	// Get Optical Flow
	SetState(mesh_buffer[0],base_frame_index,ABSIKState);
	AICP[0]->PrepareMesh(base_frame_index);
	AICP[0]->GetOpticalFlowCorrespondences(frame_index,base_frame_index);


	// Copy Optical Flow to all processors
	Array<TargetList_2D_Item_type> *OpticalFlow=AICP[0]->GetFeat_TargetList();
	int num_of_feat=OpticalFlow->numElements();
	for(int j=1;j<num_processors;j++) {
		Array<TargetList_2D_Item_type> *fl=AICP[j]->GetFeat_TargetList();
		fl->clear();
		for(int i=0;i<num_of_feat;i++) fl->append((*OpticalFlow)[i]);
	}









	//
	// Error Evaluation Mode
	//
	if ((iteration_l0==0) && (iteration_l1==0)) {
		// Get current state
		main_mesh->BoneRoot->CalcRelativeFromAbsolute(frame_index);
		GetDOFState(main_mesh,frame_index,ABSIKState,Particles[0].State);
		// Error evaluation 
		AICP[0]->iteration_l0=0;
		AICP[0]->iteration_l1=0;
		if (DebugImages!=0) {
			AICP[0]->DebugImages=PEDEBUG_FRAMES;
			AICP[0]->DebugIndex=frame_index;
		} else AICP[0]->DebugImages=PEDEBUG_NEVER;
		Minimize(0,0,frame_index);
		
		// Note: the residual here do not account for the differentiable errors (like OF and silhouette correspondences)
		if (show_minimal_debug_information) ShowParticleInfo(&(Particles[0]),Particles[0].Residual,false);
		return Particles[0].Residual;
	}







	//
	// Minimization Mode
	//
	bool using_prediction=false;
	if (Refinement) {
		main_mesh->BoneRoot->CalcRelativeFromAbsolute(frame_index);
		GetDOFState(main_mesh,frame_index,ABSIKState,Particles[0].State);
	} else {
		//	
		// Predict state 
		// Use    Particle[0].State
		//		  ABSIKState
		//
		if (UsePrediction) {
			if (base_frame_index>4) {
				// Save original particle
				using_prediction=true;
				ParticleCounter++;
				Particles[1].Index=1;
				Particles[1].Residual=DBL_MAX;
				Particles[1].Type=0;
				for(int j=0;j<num_Freedom_Degrees;j++) Particles[1].State[j]=Particles[0].State[j];


				// Predict particle 0
				Particles[0].Type=1;
				if (show_minimal_debug_information) cout<<" -> Prediction:\n";
				AICP[0]->iteration_l0=prediction_iterations;
				AICP[0]->iteration_l1=0;

				int tmp_DebugImages=AICP[0]->DebugImages;
				AICP[0]->DebugImages=PEDEBUG_NEVER;
				
				Articulated_ICP *CAICP=AICP[0];
				Skinned_Mesh    *CMesh=mesh_buffer[0];
				double          *CIKState=IKState[0];
				ParticleElement *CParticle=&(Particles[0]);
				double residual;

				// Local optimization
				for(int i=0;i<(6*main_mesh->num_bones);i++) CIKState[i]=ABSIKState[i];
				SetDOFState(CMesh,frame_index,CIKState,CParticle->State);
				
					CAICP->GetOnlyPredictionCorrespondences(main_mesh,base_frame_index);
					CAICP->PrepareMesh(frame_index);
					residual=CAICP->PredictFrameMinimization(frame_index);

				GetDOFState(CMesh,frame_index,CIKState,CParticle->State);
				AICP[0]->DebugImages=tmp_DebugImages;

				if (show_minimal_debug_information) cout<<"    Residual "<<residual<<"\n";
			}
		}
	}








	//
	// Initial Minimization
	//
	if ((initial_iteration_l1) || (initial_iteration_l0)) {
		if (show_debug_information) cout<<" -> Initial optimization:\n";
		AICP[0]->iteration_l0=initial_iteration_l0;
		AICP[0]->iteration_l1=initial_iteration_l1;

		int tmp_DebugImages=AICP[0]->DebugImages;
		AICP[0]->DebugImages=PEDEBUG_NEVER;
		Minimize(0,0,frame_index);
		AICP[0]->DebugImages=tmp_DebugImages;
		
		if (show_debug_information)	ShowParticleInfo(&(Particles[0]),Particles[0].Residual);
	}

	// Set noise (gall step optional TODO)
	for(int j=0;j<num_Freedom_Degrees;j++) dof_noise_ratio[j]=noise_ratio;
	if (show_debug_information)	cout<<"    Noise ratio = "<<dof_noise_ratio[0]<<"\n"; // TODOLUCA only for one





	//
	// Init all the other particles equal to Particle[0]   (or to Particle[1] if using_prediction=true)
	//
	// 	 // TODO The Limits for Set limits level 0!! for IK_Solver_Ref=AICP[0]->GetIKSolverL0(); -> gia settati al momento di chiamare il costruttore di IKSolverL0 -> quindi upperbound e loverbounds are correct
	//
	for(int i=ParticleCounter;i<num_particles;i++) {
		ParticleCounter++;
		Particles[i].Index=i;
		Particles[i].Residual=DBL_MAX;
		int source_particle_index=0;
		if (using_prediction) source_particle_index=Uniform_int_rand(0,1);			// mix the prediction (0) with the original (1) state
		Particles[i].Type=2+Particles[using_prediction].Type;
		for(int j=0;j<num_Freedom_Degrees;j++) Particles[i].State[j]=Particles[source_particle_index].State[j];
		AddNoise(i);
	}


	//
	// Statistics
	//
	double total_gain=0.0;
	ParticleChart->clear();





	//
	// Iteration Loop
	//
	int iter=0;
	while (true) {
		if (show_debug_information)	cout<<" -> Optimization No."<<(iter+1)<<"\n";


		// Set iterations #
		int debug_index;
		if (L1_tot<iteration_l1) {
			for(int j=0;j<num_processors;j++) {
				AICP[j]->iteration_l0=0;
				AICP[j]->iteration_l1=L1_inc;
				debug_index=L1_tot+L0_tot;
			}
			L1_tot+=L1_inc;
		} else {
			for(int j=0;j<num_processors;j++) {
				if (NoLocalOptimization) AICP[j]->iteration_l0=0;
				else AICP[j]->iteration_l0=L0_inc;
				AICP[j]->iteration_l1=0;
				debug_index=L1_tot+L0_tot;
			}
			L0_tot+=L0_inc;
		}


		// Compute Particles[..].Residual
		tmp_frame_index=frame_index;
		tmp_debug_index=debug_index;
		Task->Add(&TaskFinished,bsprintf("ii",0,num_particles-1),NULL);
		WAIT_EVENT(TaskFinished);


		// Generate Chart
		//    note: particlechart contains the best ever, so also the one on previous iterations (which can have the same index)
		for(int j=0;j<num_particles;j++) ParticleChart->insert(Particles[j].Index,Particles[j].State,Particles[j].Residual);


		if (show_debug_information)	{
			cout<<"        Residual "<<ParticleChart->score[0]<<"\n";

			SEARCH_PARTICLE_J
			if (particle_j==num_particles) {
				cout<<"        Best #"<<ParticleChart->index[0]<<"(previous optimization)\n";
			} else {
				cout<<"        Best #"<<ParticleChart->index[0]<<"("<<particle_j<<") evlolution of ";

				double gain=Particles[0].Residual-ParticleChart->score[0];
				if (iter==0) {
					switch (Particles[particle_j].Type) {
						case 0: cout<<"previous frame estimation\n"; 
								break;
						case 1: if ((initial_iteration_l1) || (initial_iteration_l0)) cout<<"prediction + initial minimization\n"; 
								else cout<<"prediction\n"; 
								cout<<"        Gained "<<gain<<" from prediction\n"; 
								break;
						case 2: cout<<"previous frame estimation + noise\n"; 
								cout<<"        Gained "<<gain<<" from noise\n"; 
								break;
						case 3: if ((initial_iteration_l1) || (initial_iteration_l0)) cout<<"prediction + initial minimization + noise\n"; 
								else cout<<"prediction + noise\n"; 
								cout<<"        Gained "<<gain<<" from prediction and noise\n"; 
								break;
					}
				} else {
					switch (Particles[particle_j].Type) {
						case 0: cout<<"previous global minimum\n"; 
								break;
						case 1: cout<<"previous local minimum\n"; 
								cout<<"        Gained "<<gain<<" from tracking\n"; 
								break;
						case 2: cout<<"previous global minimum + noise\n"; 
								cout<<"        Gained "<<gain<<" from noise\n"; 
								break;
						case 3: cout<<"previous local minimum + noise\n"; 
								cout<<"        Gained "<<gain<<" from tracking and noise\n"; 
								break;
					}
				}
				total_gain+=gain;
			}
		}
		if (DebugImages&PEDEBUG_ITERATIONS) {
			SEARCH_PARTICLE_J
			if (particle_j!=num_particles) {
				// copy to main dir -> l'immagine potrebbe essere assente due to LO che si e' fermato prima
				char in_path[500],out_path[500];
				int approx_image_index=AICP[0]->DebugIndex-1;
				for(int i=0;i<num_views;i++) {
					if (AICP[0]->iteration_l0>0) {
						sprintf(in_path,"%s\\Frame_%06i_P_%02i\\%i\\LF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,frame_index,particle_j,i,approx_image_index);
						sprintf(out_path,"%s\\%i\\%06i_LF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,i,frame_index,iter);
					} else {
						sprintf(in_path,"%s\\Frame_%06i_P_%02i\\%i\\LF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,frame_index,particle_j,i,approx_image_index);
						sprintf(out_path,"%s\\%i\\%06i_L1_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,i,frame_index,iter);
					}
					Copy(in_path,out_path);
				}
			}
		}

		if ((L0_tot>=iteration_l0) && (L1_tot>=iteration_l1)) break;


		// Set noise (gall step optional todo)
		// TODOLUCA (noise adattativo)
		double *dof_statistics=new double [num_particles];
		for(int q_dof=0;q_dof<num_Freedom_Degrees;q_dof++) {
			for(int j=0;j<ParticleChart->num_elements;j++) {
				dof_statistics[j]=((double*)(ParticleChart->mem[j]->obj))[q_dof];
			}
			double mean,std;
			mean_std(dof_statistics,ParticleChart->num_elements,mean,std,1);
		
			double Max=IK_Solver_Ref->GetUpperBounds()[q_dof];
			double Min=IK_Solver_Ref->GetLowerBounds()[q_dof];
		
			if ((Max<=Min) || (Max>=MAX_NOISE_THRESHOLD) || (Min<=-MAX_NOISE_THRESHOLD)) std/=(M_PI/3.0);
			else std/=(Max-Min);

			bool pyramidal_noise=true; // TODOSTOCASTIC Piramidale???
			if (pyramidal_noise) std/=Freedom_Degrees[q_dof].Ref->Level;

			dof_noise_ratio[q_dof]=std*3.0; // TODOLUCA
		}
		delete []dof_statistics;
		// TODOLUCA
		//for(int j=0;j<num_Freedom_Degrees;j++) dof_noise_ratio[j]*=(1.0-noise_decrease_ratio);
		//if (show_debug_information)	cout<<"    Noise ratio = "<<dof_noise_ratio[0]<<"\n"; // TODOLUCA only for one 
		if (show_debug_information)	{
			for(int q_dof=0;q_dof<num_Freedom_Degrees;q_dof++) {
				cout<<"    Noise ratio = "<<dof_noise_ratio[q_dof]<<"\n";
			}
		}

		ResampleParticle();
		iter++;
	}


	





	//
	// Get the WTA
	//
	ParticleChart->mem[0]->copyTo(Particles[0].State);
	SetDOFState(main_mesh,frame_index,ABSIKState,Particles[0].State);
	main_mesh->Apply_Skin(frame_index);
	main_mesh->ComputaNormals();	



	//
	// Get residual and debug images if missed and needed
	//
	if (show_minimal_debug_information) cout<<"-------------------------------------------------------------------------\n";
	if ((show_debug_information) || (DebugImages==PEDEBUG_FRAMES)) {
		AICP[0]->iteration_l0=0;
		AICP[0]->iteration_l1=0;
		AICP[0]->DebugIndex=frame_index;
		if (DebugImages==PEDEBUG_FRAMES) AICP[0]->DebugImages=PEDEBUG_FRAMES;
		else AICP[0]->DebugImages=PEDEBUG_NEVER;
		Minimize(0,0,frame_index);
	}
	
	//
	// Print Statistics
	//
	if (show_minimal_debug_information) {
		cout<<"    Total gained from the stocastic "<<total_gain<<"\n";
		cout<<"    Time: "<<IterationTime->Stop()<<" sec\n";
		if (show_debug_information) ShowParticleInfo(&(Particles[0]),ParticleChart->score[0]);
		else cout<<"    Residual "<<ParticleChart->score[0]<<"\n";
		delete IterationTime;
	}


	return ParticleChart->score[0];
}



void Stocastic_Articulated_ICP::ResampleParticle() {
	if (show_debug_information)	cout<<"    Resampling\n";

	float  k=resample_k*num_particles;
	float *places=new float[ParticleChart->num_elements];


	// Generates place list
	{		
		for(int j=0;j<ParticleChart->num_elements;j++) places[j]=k*(ParticleChart->num_elements-j);
		float sum=0;
		for(int j=0;j<ParticleChart->num_elements;j++) sum+=places[j];
		for(int j=0;j<ParticleChart->num_elements;j++) places[j]=ceil(places[j]*num_particles/sum);
		sum=0;
		for(int j=0;j<ParticleChart->num_elements;j++) sum+=places[j];
		int d_i=ParticleChart->num_elements-1;
		while(sum>num_particles) {
			if (places[d_i]>0) {
				places[d_i]--;
				sum--;
			} else {
				d_i--;
				if (d_i<0) ErrorExit("Error in the algorithm..");
			}
		}
		if (sum<num_particles) ErrorExit("Error in the algorithm..");
	}


	// Generates new particles
	int p_index=0;
	for(int j=0;j<ParticleChart->num_elements;j++) {
		if (!(places[j])) continue;
		
		// Note: questa frase puo' essere ripetuta per lo stesso indice di particle 
		//       (ma a iterazioni differenti e quindi stati differenti)
		if (show_debug_information)	cout<<"        Particle #"<<ParticleChart->index[j]<<" gets "<<places[j]<<" particles ";

		bool first=true;
		while(places[j]>0) {
			if (p_index>num_particles) ErrorExit("Error in the algorithm..");
			
			ParticleChart->mem[j]->copyTo(Particles[p_index].State);
			if (!first) {
				AddNoise(p_index);
				Particles[p_index].Residual=DBL_MAX;
				Particles[p_index].Index=ParticleCounter++;
				if (j==0) Particles[p_index].Type=2;
				else Particles[p_index].Type=3;
				if (show_debug_information)	cout<<"#"<<Particles[p_index].Index<<" ";
			} else {
				first=false;
				Particles[p_index].Residual=ParticleChart->score[j];
				Particles[p_index].Index=ParticleChart->index[j];
				if (j==0) Particles[p_index].Type=0;
				else Particles[p_index].Type=1;
			}
			
			places[j]--;
			p_index++;
		}
		if (show_debug_information)	cout<<"\n";
	}

	delete []places;
}


void Stocastic_Articulated_ICP::AddNoise(int particle) {
	Freedom_Degrees_type *DOF_tmp=Freedom_Degrees;
	
	for(int j=0;j<num_Freedom_Degrees;j++,DOF_tmp++) {
		double Max=IK_Solver_Ref->GetUpperBounds()[j];
		double Min=IK_Solver_Ref->GetLowerBounds()[j];
		
		double sigma=((Max-Min)*dof_noise_ratio[j]);
		if ((sigma<0) || (Max>=MAX_NOISE_THRESHOLD) || (Min<=-MAX_NOISE_THRESHOLD)) sigma=(M_PI/3.0)*dof_noise_ratio[j];	// TODO***
		
		
		if ((DOF_tmp->Ref->Father==NULL) && (!apply_noise_on_root)) continue;
		if (str_begin((static_cast<Bone*>(DOF_tmp->Ref->Father))->GetName(),"Root_NULL_Bone")) continue; 
		// TODOAPPROX se e' root o e' root di un oggetto disconnesso -> no noise!!

		bool pyramidal_noise=true; // TODOSTOCASTIC Piramidale???
		if ((DOF_tmp->Ref->Father!=NULL) && (pyramidal_noise)) sigma=DOF_tmp->Ref->Level*sigma;


		// TODOAPPROX Non mouve in traslazione -> solo rotazione
		if (DOF_tmp->type<3) {
			Particles[particle].State[j]+=Normal_double_rand(0.0,sigma); 
			Particles[particle].State[j]=min(Particles[particle].State[j],Max);
			Particles[particle].State[j]=max(Particles[particle].State[j],Min);
		}
	}

}

























void Stocastic_Articulated_ICP::ShowParticleInfo(ParticleElement *CParticle,double p_residual,bool account_for_local_residuals) {
	double s_ratio=100.0*CParticle->Residual_S/p_residual;
	double d_ratio=100.0*CParticle->Residual_D/p_residual;
	double o_ratio=100.0*CParticle->Residual_O/p_residual;
	double c_ratio=100.0*CParticle->Residual_C/p_residual;
	double t_ratio=100.0*CParticle->Residual_T/p_residual;
	double p_ratio=100.0*CParticle->Residual_P/p_residual;
	double e_ratio=100.0*CParticle->Residual_E/p_residual;
	double local_residual=max(0.0,p_residual-(CParticle->Residual_S+CParticle->Residual_O+CParticle->Residual_D+CParticle->Residual_C+CParticle->Residual_T+CParticle->Residual_P+CParticle->Residual_E));
	double l_ratio=100.0*local_residual/p_residual;
	double n_observations=CParticle->n_observations_S+CParticle->n_observations_O+CParticle->n_observations_D+CParticle->n_observations_T+CParticle->n_observations_P+CParticle->n_observations_E;
	double s_t_ratio=100.0*CParticle->n_observations_S/n_observations;
	double d_t_ratio=100.0*CParticle->n_observations_D/n_observations;
	double o_t_ratio=100.0*CParticle->n_observations_O/n_observations;
	double t_t_ratio=100.0*CParticle->n_observations_T/n_observations;
	double p_t_ratio=100.0*CParticle->n_observations_P/n_observations;
	double e_t_ratio=100.0*CParticle->n_observations_E/n_observations;
		printf("\n");
		printf("                Residual                 Observations\n");
		printf("         S = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_S,s_ratio,s_t_ratio);
	if ((edges_weight!=0.0) && (CParticle->n_observations_E!=0.0)) {
		printf("         E = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_E,o_ratio,e_t_ratio);
	}
	if ((salient_points_weight!=0.0) && (CParticle->n_observations_P!=0.0)) {
		printf("         P = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_P,o_ratio,p_t_ratio);
	}
	if ((opticalflow_weight!=0.0) && (CParticle->n_observations_O!=0.0)) {
		printf("         O = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_O,o_ratio,o_t_ratio);
	}
	if ((depth_weight!=0.0) && (CParticle->n_observations_D!=0.0)) {
		printf("         D = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_D,d_ratio,d_t_ratio);
	}
	if ((texture_weight!=0.0) && (CParticle->n_observations_T!=0.0)) {
		printf("         T = %-10.4g  (%3.0f%%)            (%3.0f%%)\n",CParticle->Residual_T,t_ratio,t_t_ratio);
	}
	if (collision_weight!=0.0) {
		printf("         C = %-10.4g  (%3.0f%%)\n",CParticle->Residual_C,c_ratio);
	}
	if (!account_for_local_residuals) {
		printf("         L = %-10.4g  (%3.0f%%)            (not accounting for local residuals)\n",0.0,0.0);
	} else {
		if (local_optimization_weight!=0.0) {
			printf("         L = %-10.4g  (%3.0f%%)\n",local_residual,l_ratio);
		}
	}
	printf("             -----------\n");
	printf("    Total    %-10.4g\n\n",p_residual);
}































void Stocastic_Articulated_ICP::MinimizePSO(int particle_j,int proc,int frame) {
	Articulated_ICP *CAICP=AICP[proc];
	Skinned_Mesh    *CMesh=mesh_buffer[proc];
	double          *CIKState=IKState[proc];
	ParticleElement *CParticle=&(Particles[particle_j]);
	double residual;


	if (CAICP->DebugImages!=0) {
		char out_dir_path[500];
		if (CAICP->DebugImages==PEDEBUG_FRAMES) {
			strcpy(out_dir_path,OutDebugDir);
		} else {
			sprintf(out_dir_path,"%s\\Frame_%06i",OutDebugDir,frame);
			CAICP->DebugIndex=particle_j;
		}
		CAICP->SetOutDebugDir(out_dir_path,false);
	}

	// Local optimization
	for(int i=0;i<(6*main_mesh->num_bones);i++) CIKState[i]=ABSIKState[i];
	SetDOFState(CMesh,frame,CIKState,CParticle->State);
	residual=CAICP->GetResidual(frame,(CAICP->DebugImages!=0));
	GetDOFState(CMesh,frame,CIKState,CParticle->State);

	// Global errors
	CParticle->Residual_S=0.0;
	CParticle->Residual_E=0.0;
	CParticle->n_observations_S=0.0;
	CParticle->n_observations_E=0.0;

	
	CAICP->PrepareFrameForSilhouetteResidual();
	CParticle->Residual_S=CAICP->ComputaSilhouetteResidual(&(CParticle->n_observations_S));
	if (!_finite(CParticle->Residual_S)) {
		Warning("Object completely out of one view. Don't know what to do."); // TODO funzionera' ancora dopo il cambiamanto? o devo cercare un'altro?
	}
	CParticle->Residual_E=edges_weight*CAICP->ComputaEdgesResidual(&(CParticle->n_observations_E));

	residual=local_optimization_weight*residual;
	cout<<"local res "<<residual<<"\n";
	cout<<"sil res   "<<CParticle->Residual_S<<"\n";
	cout<<"edg res   "<<CParticle->Residual_E<<"\n";
	residual+=CParticle->Residual_S+CParticle->Residual_E;
	cout<<"residual  "<<residual<<"\n";

	CParticle->Residual=residual;
}

void Stocastic_Articulated_ICP::TaskMinimizePSO(void *data,Stocastic_Articulated_ICP *saicp) {
	int first,last;
	bscanf((BYTE*)data,"ii",&first,&last);

	int proc=first/(saicp->num_particles/saicp->num_processors);
	for(int i=first;i<=last;i++) {
		saicp->AICP[proc]->DebugIndex=saicp->tmp_debug_index;
		saicp->MinimizePSO(i,proc,saicp->tmp_frame_index);
	}
}

void Stocastic_Articulated_ICP::AddNoisePSO(int particle) {
	Freedom_Degrees_type *DOF_tmp=Freedom_Degrees;
	
	for(int j=0;j<num_Freedom_Degrees;j++,DOF_tmp++) {
		double Max=IK_Solver_Ref->GetUpperBounds()[j];
		double Min=IK_Solver_Ref->GetLowerBounds()[j];
		double sigma=((Max-Min)*dof_noise_ratio[j]);
		if ((sigma<0) || (Max>=MAX_NOISE_THRESHOLD) || (Min<=-MAX_NOISE_THRESHOLD)) sigma=(M_PI/3.0)*dof_noise_ratio[j];
		
		if (DOF_tmp->type<3) {
			sigma*=max(DOF_tmp->Ref->Level,1);

			Particles[particle].State[j]+=Normal_double_rand(0.0,sigma); 
			Particles[particle].State[j]=min(Particles[particle].State[j],Max);
			Particles[particle].State[j]=max(Particles[particle].State[j],Min);
		} else {
			if (DOF_tmp->Ref->IsLeaf()) sigma*=7;            // multiply for the max number of levels

			Particles[particle].State[j]+=Normal_double_rand(0.0,sigma);  // TODOLUCA 1 cm
			Particles[particle].State[j]=min(Particles[particle].State[j],Max);
			Particles[particle].State[j]=max(Particles[particle].State[j],Min);
		}
	}

	// NOTE: the warning about the violated constraints are due to the fact that linear interpolation on 
	// so(3) space does not correspond to concatenation of rotations directly, therefore the fact that a linear 
	// interpolation on so(3) does not violate any constraint, does not imply that the resulting rotation 
	// is inside the valid set. TODOLUCA

	// TODO: maybe increase the constraints range on each bones to avoid warnings and have better particles
}

double Stocastic_Articulated_ICP::EstimateFramePSO(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction) {
	Profiler *IterationTime=NULL;

	// Decide parameters
	int Evolutions=Approx(1.0/particle_internal_iteration_ratio);

		
	// Init processors
	for(int i=0;i<num_processors;i++) {
		CopyParametersTo(AICP[i],false);
		if (DebugImages==PEDEBUG_ITERATIONS) AICP[i]->DebugImages=PEDEBUG_ITERATIONS;
		else AICP[i]->DebugImages=PEDEBUG_NEVER;

		AICP[i]->PrepareFrameForSilhouetteResidual();
	}
	

	// Debug Information
	if (show_debug_information) {
		cout<<"\n--------------------------\n";
		cout<<"Computing Frame "<<frame_index<<"\n";
		cout<<"--------------------------\n\n";
		cout<<"    Evolving "<<Evolutions<<" times\n";
	} else {
		if (show_minimal_debug_information) {
			cout<<"Frame "<<frame_index<<":\n";
		}
	}
	if (show_minimal_debug_information) IterationTime=new Profiler;





	//
	// Init Particle[0]
	//
	ParticleCounter=1;
	Particles[0].Index=0;
	Particles[0].Residual=DBL_MAX;
	Particles[0].Type=0;
	main_mesh->BoneRoot->CalcRelativeFromAbsolute(base_frame_index);
	GetDOFState(main_mesh,base_frame_index,ABSIKState,Particles[0].State);

	// Get Optical Flow
	SetState(mesh_buffer[0],base_frame_index,ABSIKState);
	AICP[0]->PrepareMesh(base_frame_index);
	AICP[0]->GetOpticalFlowCorrespondences(frame_index,base_frame_index);


	// Copy Optical Flow to all processors
	Array<TargetList_2D_Item_type> *OpticalFlow=AICP[0]->GetFeat_TargetList();
	int num_of_feat=OpticalFlow->numElements();
	for(int j=1;j<num_processors;j++) {
		Array<TargetList_2D_Item_type> *fl=AICP[j]->GetFeat_TargetList();
		fl->clear();
		for(int i=0;i<num_of_feat;i++) fl->append((*OpticalFlow)[i]);
	}










	//
	// Minimization
	//
	bool using_prediction=false;
	if (Refinement) {
		main_mesh->BoneRoot->CalcRelativeFromAbsolute(frame_index);
		GetDOFState(main_mesh,frame_index,ABSIKState,Particles[0].State);
	} else {
		//	
		// Predict state 
		// Use    Particle[0].State
		//		  ABSIKState
		//
		if (UsePrediction) {
			if (base_frame_index>4) {
				// Save original particle
				using_prediction=true;
				ParticleCounter++;
				Particles[1].Index=1;
				Particles[1].Residual=DBL_MAX;
				Particles[1].Type=0;
				for(int j=0;j<num_Freedom_Degrees;j++) Particles[1].State[j]=Particles[0].State[j];


				// Predict particle 0
				Particles[0].Type=1;
				if (show_minimal_debug_information) cout<<" -> Prediction:\n";
				AICP[0]->iteration_l0=prediction_iterations;
				AICP[0]->iteration_l1=0;

				int tmp_DebugImages=AICP[0]->DebugImages;
				AICP[0]->DebugImages=PEDEBUG_NEVER;
				
				Articulated_ICP *CAICP=AICP[0];
				Skinned_Mesh    *CMesh=mesh_buffer[0];
				double          *CIKState=IKState[0];
				ParticleElement *CParticle=&(Particles[0]);
				double residual;

				// Local optimization
				for(int i=0;i<(6*main_mesh->num_bones);i++) CIKState[i]=ABSIKState[i];
				SetDOFState(CMesh,frame_index,CIKState,CParticle->State);
				
					CAICP->GetOnlyPredictionCorrespondences(main_mesh,base_frame_index);
					CAICP->PrepareMesh(frame_index);
					residual=CAICP->PredictFrameMinimization(frame_index);

				GetDOFState(CMesh,frame_index,CIKState,CParticle->State);
				AICP[0]->DebugImages=tmp_DebugImages;

				if (show_minimal_debug_information) cout<<"    Residual "<<residual<<"\n";
			}
		}
	}








	//
	// Initial Minimization
	//
	if ((initial_iteration_l1) || (initial_iteration_l0)) {
		if (show_debug_information) cout<<" -> Initial optimization:\n";
		AICP[0]->iteration_l0=initial_iteration_l0;
		AICP[0]->iteration_l1=initial_iteration_l1;

		int tmp_DebugImages=AICP[0]->DebugImages;
		AICP[0]->DebugImages=PEDEBUG_NEVER;
		MinimizePSO(0,0,frame_index);
		AICP[0]->DebugImages=tmp_DebugImages;
		
		if (show_debug_information)	ShowParticleInfo(&(Particles[0]),Particles[0].Residual);
	}

	
	
	// Set noise 
	for(int j=0;j<num_Freedom_Degrees;j++) dof_noise_ratio[j]=noise_ratio;
	if (show_debug_information)	cout<<"    Noise ratio = "<<dof_noise_ratio[0]<<"\n"; // TODOLUCA only for one





	//
	// Init all the other particles equal to Particle[0]   (or to Particle[1] if using_prediction=true)
	//
	// 	 // TODO The Limits for Set limits level 0!! for IK_Solver_Ref=AICP[0]->GetIKSolverL0(); -> gia settati al momento di chiamare il costruttore di IKSolverL0 -> quindi upperbound e loverbounds are correct
	//
	for(int i=ParticleCounter;i<num_particles;i++) {
		ParticleCounter++;
		Particles[i].Index=i;
		Particles[i].Residual=DBL_MAX;
		int source_particle_index=0;
		if (using_prediction) source_particle_index=Uniform_int_rand(0,1);			// mix the prediction (0) with the original (1) state
		Particles[i].Type=2+Particles[using_prediction].Type;
		for(int j=0;j<num_Freedom_Degrees;j++) Particles[i].State[j]=Particles[source_particle_index].State[j];
		AddNoisePSO(i);
	}


	//
	// Init Speed and local optimum
	//
	for(int i=0;i<num_particles;i++) {
		Particles[i].Local_Optimum_Residual=DBL_MAX;
		for(int j=0;j<num_Freedom_Degrees;j++) Particles[i].Local_Optimum[j]=Particles[i].State[j];
		for(int j=0;j<num_Freedom_Degrees;j++) Particles[i].Speed[j]=0.0;
	}






	//
	// Iteration Loop
	//
	double  *Global_Optimum=NULL;
	double   Global_Optimum_Residual=DBL_MAX;
	SNEWA(Global_Optimum,double,num_Freedom_Degrees);

	for(int iter=0;iter<Evolutions;iter++) {
		if (show_debug_information)	cout<<" -> Evolution No."<<(iter+1)<<"\n";

		// Set iterations #
		for(int j=0;j<num_processors;j++) {
			AICP[j]->iteration_l0=0;
			AICP[j]->iteration_l1=0;
		}

		// Compute Particles[..].Residual
		tmp_frame_index=frame_index;
		tmp_debug_index=Evolutions;
		Task->Add(&TaskFinished,bsprintf("ii",0,num_particles-1),(TASK_FUNCTION)TaskMinimizePSO);
		WAIT_EVENT(TaskFinished);



		//
		// Compute Local/Global Optimum
		//
		for(int i=0;i<num_particles;i++) {
			if (Particles[i].Residual<Particles[i].Local_Optimum_Residual) {
				for(int j=0;j<num_Freedom_Degrees;j++) Particles[i].Local_Optimum[j]=Particles[i].State[j];
				Particles[i].Local_Optimum_Residual=Particles[i].Residual;
			}
			if (Particles[i].Residual<=Global_Optimum_Residual) {
				for(int j=0;j<num_Freedom_Degrees;j++) Global_Optimum[j]=Particles[i].State[j];
				Global_Optimum_Residual=Particles[i].Residual;
			}
		}


		//
		// Compute Speed
		//
		double c1=2.8;
		double c2=1.3;
		double K=2.0/fabs(2.0-(c1+c2)-sqrt(((c1+c2)*(c1+c2))-4*(c1+c2)));
		for(int i=0;i<num_particles;i++) {
			#ifndef USE_GENETIC_APPROACH
				double r1=Uniform_double_rand(0.0,1.0);
				double r2=Uniform_double_rand(0.0,1.0);
			#endif
			for(int j=0;j<num_Freedom_Degrees;j++) {
				#ifdef USE_GENETIC_APPROACH
					double r1=Uniform_double_rand(0.0,1.0);
					double r2=Uniform_double_rand(0.0,1.0);
				#endif
				Particles[i].Speed[j]=K*( Particles[i].Speed[j] + c1*r1*( Particles[i].Local_Optimum[j] - Particles[i].State[j] ) + c2*r2*( Global_Optimum[j] - Particles[i].State[j] ) );
			}
		}


		//
		// Update model 
		//
		for(int i=0;i<num_particles;i++) {
			for(int j=0;j<num_Freedom_Degrees;j++) {
				double Max=IK_Solver_Ref->GetUpperBounds()[j];
				double Min=IK_Solver_Ref->GetLowerBounds()[j];

				double new_state=(Particles[i].State[j]+Particles[i].Speed[j]);
				if (new_state<=Min) {
					new_state=Min;
					Particles[i].Speed[j]=0;
				}
				if (new_state>=Max) {
					new_state=Max;
					Particles[i].Speed[j]=0;
				}

				Particles[i].State[j]=new_state;
			}
		}

		if (show_debug_information)	cout<<"        Residual "<<Global_Optimum_Residual<<"\n";
	}


	

	//
	// Get the WTA
	//
	for(int j=0;j<num_Freedom_Degrees;j++) Particles[0].State[j]=Global_Optimum[j];
	SetDOFState(main_mesh,frame_index,ABSIKState,Particles[0].State);
	main_mesh->Apply_Skin(frame_index);
	main_mesh->ComputaNormals();	

	
	//
	// Save motion
	//
	char Text[500];
	sprintf(Text,"%s\\motion\\LF_%04i.motion",OutDebugDir,frame_index);
	OFileBuffer Out(Text);
	Bone::SaveSkullMotion(&Out,main_mesh->BoneRoot);
	Out.Close();



	//
	// Get residual and debug images if missed and needed
	//
	if (show_minimal_debug_information) cout<<"-------------------------------------------------------------------------\n";
	if ((show_debug_information) || (DebugImages==PEDEBUG_FRAMES)) {
		AICP[0]->DebugIndex=frame_index;
		AICP[0]->DebugImages=PEDEBUG_FRAMES;
		MinimizePSO(0,0,frame_index);
	}
	
	//
	// Print Statistics
	//
	if (show_minimal_debug_information) {
		cout<<"    Time: "<<IterationTime->Stop()<<" sec\n";
		if (show_debug_information) ShowParticleInfo(&(Particles[0]),Global_Optimum_Residual);
		else cout<<"    Residual "<<Global_Optimum_Residual<<"\n";
		delete IterationTime;
	}

	SDELETEA(Global_Optimum);
	return Global_Optimum_Residual;
}
