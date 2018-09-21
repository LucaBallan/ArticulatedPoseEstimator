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






//
// TODO				- Minor problem
// TODOSS			- to speed up
// TODOLUCA			- to do important
// TODOLUCA**			- to do very important
// TODOAPPROX			- approximation, not a general solution
// TODOCHECKS			- special cases
// TODODEBUG	
// TODOEDGE
// TODOSTOCASTIC
// TODOTEXTURE
// DEPRECATED
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
#include "Time_Articulated_ICP.h"
#include "Stocastic_Articulated_ICP.h"

using namespace std;
#pragma warning(3:4244)




//////////// DEFINES ///////////////

int LEN_MAX=100;				
double SOGLIA_TOLLERANZA_GRADIENTE=0.0;		// 0   only ortogonal
						// 0.8 cut > 45°
						// 1.0 only parallel
double SOGLIA_SMOOTH=2.0;
#define DEFALUT_L0_ITERATIONS  20
#define DEFALUT_L1_ITERATIONS  10
#define N_ITER_FIND_OUTLIERS   0	 	// 2 or 5
#define OUTLIERS_THRESH        7
#define DEFALUT_PROXIMITY_F    0.1
#define ROOM_MULTIPLIER        2

char   CAMERA_MATRIX_FILE[500];
char   INTERSECTION_MASK_FILE[500];
char   DISTORTION_FILE[500];
char   MODEL_FILE[500];
char   MODEL_DIR[500];
char   MODEL_NAME[100];
char   IMAGE_FILE[500];
char   FEAUTURE_FILE[500];
char   SALIENT_3D_FILENAME[500];
char   SALIENT_2D_FILENAME[500];
char   OUT_FILE_DIR[500];
char   LIMITS_FILE_NAME[500];
char   OUT_DEBUG_FILE_DIR[500];
char   VALIDLIST_FILENAME[500];
char   START_MotionFileName[500];
char   VIEW_VALID_MASK[50];
bool   DebugLast,UseLast,AlwaysDebug,NeverDebug;

int    L0_ITERATIONS=DEFALUT_L0_ITERATIONS;
int    L1_ITERATIONS=DEFALUT_L1_ITERATIONS;
double PROXIMITY_F=DEFALUT_PROXIMITY_F;

double TEXTURE_LOCAL_WEIGHT=1.0;
double DEPTH_LOCAL_WEIGHT_L0=1.0;
double DEPTH_LOCAL_WEIGHT_L1=1.0;
double SALIENT_POINT_LOCAL_WEIGHT=1.0;
double OPTICALFLOW_LOCAL_WEIGHT=1.0;
double COLLISION_WEIGHTS_L0=0.0;
double COLLISION_WEIGHTS_L1=0.0;

double OPTICALFLOW_WEIGHT=1.0;
double COLLISION_WEIGHT=0.0;
double DEPTH_WEIGHT=1.0;
double TEXTURE_WEIGHT=1.0;
double EDGES_WEIGHT=1.0;
double SALIENT_POINTS_WEIGHT=1.0;
double OPTIMIZATION_WEIGHT=0.0;

double MAX_OF_INFO_DISTANCE=0.2;
double OPTICALFLOW_ITERATIONS=25.0;
double collision_sigma=0.5;
int    ADDITIONAL_OBSERVATIONS=600;
bool   REFINING=false;
bool   PREDICTING=false;
int    PREDICTION_ITERATIONS=10;
int    MAX_PREDICTION_C=10000;
bool   delete_all_debug_files=true;
bool   load_limits=false;
int    time_window_size=4;
bool   apply_constrains=true;
bool   avoid_collisions=false;
int    optimization_method=1;
bool   APPLY_NOISE_ON_ROOT=false;
int    initial_iteration_l0=0;
int    initial_iteration_l1=0;
int    Annealing_num_particles_per_iteration=9;
int    Annealing_resample_elements=6;
double noise_ratio=0.5;
double PARTICLE_INTERNAL_ITERATION=4;
double SALIENT_THRESHOLD=1.8;
double SALIENT_DEPTH_THRESHOLD=1.0;
int    EDGE_SIL_CONF=1;
int    INPUT_EDGES_IMAGES_EDGE_THRESHOLD=30;
int    TEXTURED_MODEL_IMAGES_EDGE_THRESHOLD=30;
double DEPTH_MODEL_IMAGES_EDGE_THRESHOLD=1.0;
int    TIPICAL_EROSION_FOR_OBJECT_MASK=4;
int    SALIENT_POINT_MAX_DISTANCE=30;
int    COLLISION_TTL=0;
double CONTACT_POINTS_WEIGHT=0.0;
int    CONTACT_POINTS_TTL=0;

double EXTRA_W_OF=1.0;
double EXTRA_W_E=1.0;
char   EXTRA_W_NAME_1[100];
char   EXTRA_W_NAME_2[100];
char  *EXTRA_W_NAME_TMP[10];

int    Debug_Type=0;
int    debug_info_type=2;
int    number_processor=0;
int    random_seed=0;

View                      *Vista=NULL;
int                        num_viste;
Features                 **FeaturesList=NULL;
Array<UINT>               *SalientVertices=NULL;
SalientPoints            **Salient2DPoints=NULL;
Skinned_Mesh              *Model=NULL;
PoseEstimator             *Solver=NULL;
bool                      *collision_mask=NULL;
HyperMeshPartitionTree    *collision_detector=NULL;



#define BITMAP_EXTENSION		"png"
#include "ArticulatedPoseEstimator.h"




void CorrectBiped(Bone *Root) {
	Root->CalcAbsoluteFromRelative(0);
	Bone *pelvis=Root->SearchBone("Bone_Bip01 Pelvis");
	if (pelvis==NULL) cout<<" -> Biped01 not found.\n";
	else {
		Bone *spine=Root->SearchBone("Bone_Bip01 Spine");
		Bone *Rth=spine->SearchBone("Bone_Bip01 R Thigh");
		Bone *Lth=spine->SearchBone("Bone_Bip01 L Thigh");
		if (Rth!=NULL) {
			cout<<" -> Biped01 triangle pelvis deactivated.\n";
			spine->DetachChild(Rth);
			spine->DetachChild(Lth);
			pelvis->AddChild(Rth);
			pelvis->AddChild(Lth);
			Root->CalcRelativeFromAbsolute(0);
			Root->UpdateTreeLevels();
		}
	}
	pelvis=Root->SearchBone("Bone_Bip02 Pelvis");
	if (pelvis==NULL) cout<<" -> Biped01 not found.\n";
	else {
		Bone *spine=Root->SearchBone("Bone_Bip02 Spine");
		Bone *Rth=spine->SearchBone("Bone_Bip02 R Thigh");
		Bone *Lth=spine->SearchBone("Bone_Bip02 L Thigh");
		if (Rth!=NULL) {
			cout<<" -> Biped02 triangle pelvis deactivated.\n";
			spine->DetachChild(Rth);
			spine->DetachChild(Lth);
			pelvis->AddChild(Rth);
			pelvis->AddChild(Lth);
			Root->CalcRelativeFromAbsolute(0);
			Root->UpdateTreeLevels();
		}
	}
}

void PrintHelp();
int ParseInputData(int argc,char* argv[],int &sc,int &ed);
void ParseInputData_Constraints(int argc,char* argv[]);
void MakeOutput(Skinned_Mesh *Model);
void LoadInitialMotion();
void SetViewValidMask();


char *crypt_strX_readable(const char *content, int count, char *var_name);

int main(int argc,char* argv[]) {
	// -----------------------
	// - Init
	// -----------------------
	InitGammaLib("Articulated Pose Estimator", argc, argv); cout << "\n";
	int st,ed;
	int fnc=ParseInputData(argc,argv,st,ed);
	if (fnc<=0) return -1;
	if (!random_seed) srand((unsigned int)time(NULL));
	else srand((unsigned int)random_seed);



	// -----------------------
	// - Load scene model
	// -----------------------
	cout<<"\nLoad Cameras Matrices:\n";
	LoadCamerasMatrices(CAMERA_MATRIX_FILE,DISTORTION_FILE);
	cout<<" -> loaded.\n\n";
	cout<<"Object Model:\n";
	Model=new Skinned_Mesh();
	Model->Read(MODEL_FILE);
	cout<<" -> loaded.\n\n";
	LoadInitialMotion();
	cout<<"\n";
	if (OPTICALFLOW_LOCAL_WEIGHT==0.0) FEAUTURE_FILE[0]=0;


	// -----------------------
	// - Load Limits
	// -----------------------
	Contraint_Table=Biped_Degrees_of_Freedom_Simply_Table;
	if (load_limits) {cout<<"Constraints:\n -> File: "<<LIMITS_FILE_NAME<<"\n";Contraint_Table=Degrees_of_Freedom_Table_Entry::Load(LIMITS_FILE_NAME,Model,st);}
	ParseInputData_Constraints(argc,argv);
	Contraint_Table_L1=Degrees_of_Freedom_Table_Entry::Copy(Contraint_Table);
	//
	Set_Block("L Forearm",Contraint_Table_L1,false,false);
	Set_Block("R Forearm",Contraint_Table_L1,false,false);
	Set_Block("L Hand",Contraint_Table_L1,false,false);
	Set_Block("R Hand",Contraint_Table_L1,false,false);
	Set_Block("Head",Contraint_Table_L1,false,false);
	Set_Block("L Foot",Contraint_Table_L1,false,false);
	Set_Block("R Foot",Contraint_Table_L1,false,false);
	Set_Block("R Toe0",Contraint_Table_L1,false,false);
	Set_Block("L Toe0",Contraint_Table_L1,false,false);
	//
	Set_Block("LFing.0_2",Contraint_Table_L1,false,false);
	Set_Block("LFing.1_2",Contraint_Table_L1,false,false);
	Set_Block("LFing.2_2",Contraint_Table_L1,false,false);
	Set_Block("LFing.3_2",Contraint_Table_L1,false,false);
	Set_Block("LFing.4_1",Contraint_Table_L1,false,false);
	Set_Block("LFing.0_1",Contraint_Table_L1,false,false);
	Set_Block("LFing.1_1",Contraint_Table_L1,false,false);
	Set_Block("LFing.2_1",Contraint_Table_L1,false,false);
	Set_Block("LFing.3_1",Contraint_Table_L1,false,false);
	Set_Block("RFing.0_2",Contraint_Table_L1,false,false);
	Set_Block("RFing.1_2",Contraint_Table_L1,false,false);
	Set_Block("RFing.2_2",Contraint_Table_L1,false,false);
	Set_Block("RFing.3_2",Contraint_Table_L1,false,false);
	Set_Block("RFing.4_1",Contraint_Table_L1,false,false);
	Set_Block("RFing.0_1",Contraint_Table_L1,false,false);
	Set_Block("RFing.1_1",Contraint_Table_L1,false,false);
	Set_Block("RFing.2_1",Contraint_Table_L1,false,false);
	Set_Block("RFing.3_1",Contraint_Table_L1,false,false);
	


	// -----------------------
	// - Load input videos
	// -----------------------
	cout<<"\nVideo Files:\n";
	InitLoadFrames(IMAGE_FILE,Vista,num_viste);
	cout<<" -> initialized.\n\n";

	// -----------------------
	// - Load Features
	// -----------------------
	cout<<"Features Tracks:\n";
	FeaturesList=LoadFeatures(FEAUTURE_FILE,Vista,num_viste);
	cout<<" -> loaded.\n\n";

	// -----------------------
	// - Load Salient Points
	// -----------------------
	cout<<"Salient Points:\n";
	LoadSalientPoints(SALIENT_3D_FILENAME,SALIENT_2D_FILENAME,SalientVertices,Salient2DPoints,num_viste,Model->getNumberOfFrames());
	cout<<"\n";



	// -----------------------
	// - Corrections
	// -----------------------
	cout<<"Correct Biped:\n";
	CorrectBiped(Model->BoneRoot);
	cout<<"\n";



	// -----------------------
	// - Load Intersections
	// -----------------------
	int selected_collision_detector_level=0;
	if (avoid_collisions) {
		// Test at skin(0)
		cout<<"Intersection Mask:\n";
		collision_mask=LoadIntersectionMask(Model,INTERSECTION_MASK_FILE);
		cout<<"\n";
		float c_fps;collision_detector=HyperMeshPartitionTree::Generate(Model,12,15,collision_mask,c_fps,selected_collision_detector_level);
		cout<<" -> fps: "<<c_fps<<" (Level: "<<selected_collision_detector_level<<")\n\n";
	}


	cout<<"Validity List:\n";
	Array<int> ValidList(Model->getNumberOfFrames());
	ValidList.append(-1);
	{// Read the list of valid frames
		IFileBuffer Valid(VALIDLIST_FILENAME);
		if (!Valid.IsFinished()) {
			do {
				int tmp_i;
				Valid>>tmp_i;
				ValidList.append(tmp_i);
			} while(!Valid.IsFinished());
		} else {
			cout<<" -> Valid_file does not exist. All the frames are considered valid.\n";
			for(int i=1;i<Model->getNumberOfFrames();i++) ValidList.append(i);
		}	
		//  0 [1] [2] 3 4 5 
		// -1 [1] [3] 4 5 6 
	}	



	// -----------------------
	// - Setup Room
	// -----------------------
	Model->Apply_Skin(1);
	Vector<3> RoomCenter=Model->GetCenter();
	double RoomRadius=ROOM_MULTIPLIER*Model->GetRadius(RoomCenter);
	cout<<"\nRoom/Model information:\n";
	cout<<" -> Center ( "<<RoomCenter<<")\n";
	cout<<" -> Radius ( "<<RoomRadius<<" )\n";
	{
		mean_std_data stat;
		for(int i=0;i<Model->num_f;i++) {
			Vector<3> b=Model->Faces[i].Baricentro(&(Model->Points));
			double radius=-DBL_MAX;
			for(int q=0;q<3;q++) radius=max(radius,(Model->Points[Model->Faces[i].Point[q]]-b).Norm2());
			mean_std_i(radius,&stat);
		}
		double mean,std;
		mean_std_r(&stat,mean,std);
		cout<<" -> edges stress:\n";
		cout.precision(4);
		cout<<"       avg = "<<fixed<<mean<<"    std = "<<fixed<<std<<"\n";
		cout<<"       max = "<<fixed<<stat.max<<"    min = "<<fixed<<stat.min<<"\n\n";
	}







	// -----------------------
	// - Set up Solver
	// -----------------------
	switch (optimization_method) {
		case 1:
			Solver=new Articulated_ICP(Model,RoomCenter,RoomRadius,FeaturesList,Vista,num_viste,Contraint_Table,Contraint_Table_L1,debug_info_type);
			break;
		case 2:
			Solver=new Time_Articulated_ICP(Model,RoomCenter,RoomRadius,FeaturesList,Vista,num_viste,time_window_size,Contraint_Table,Contraint_Table_L1,debug_info_type);
			break;
		case 3:
			Solver=new Stocastic_Articulated_ICP(Model,RoomCenter,RoomRadius,FeaturesList,Vista,num_viste,Contraint_Table,Contraint_Table_L1,debug_info_type,0,number_processor,Annealing_num_particles_per_iteration,Annealing_resample_elements); 
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->initial_iteration_l0=initial_iteration_l0;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->initial_iteration_l1=initial_iteration_l1;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->resample_k=0.05f;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->resample_policy=SAICP_POLICY_WTA;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->noise_ratio=noise_ratio;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->noise_decrease_ratio=0.20;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->particle_internal_iteration_ratio=1.0/PARTICLE_INTERNAL_ITERATION;
			(dynamic_cast<Stocastic_Articulated_ICP*>(Solver))->apply_noise_on_root=APPLY_NOISE_ON_ROOT;
			break;
	}
	// Debug
		Solver->DebugIndex=0;
		Solver->Debug_Type=Debug_Type;
		Solver->DebugImages=PEDEBUG_NEVER;
		Solver->SetOutDebugDir(OUT_DEBUG_FILE_DIR,delete_all_debug_files);
	// General
		Solver->iteration_l0=L0_ITERATIONS;
		Solver->iteration_l1=L1_ITERATIONS;
		Solver->proximity_factor=PROXIMITY_F;
		//Solver->Stop_Error_Th			Use default
		//Solver->internal_iteration	Use default
		Solver->local_optimization_weight=OPTIMIZATION_WEIGHT;
	// Information 
		Solver->max_of_info_distance=MAX_OF_INFO_DISTANCE;
		Solver->OpticalFlowIterations=OPTICALFLOW_ITERATIONS;
		Solver->opticalflow_local_weight=OPTICALFLOW_LOCAL_WEIGHT;
		Solver->opticalflow_weight=OPTICALFLOW_WEIGHT;
		Solver->depth_local_weight_l0=DEPTH_LOCAL_WEIGHT_L0;
		Solver->depth_local_weight_l1=DEPTH_LOCAL_WEIGHT_L1;
		Solver->depth_weight=DEPTH_WEIGHT;
		Solver->max_length_search=LEN_MAX;
		Solver->gradient_tollerance=SOGLIA_TOLLERANZA_GRADIENTE;
		Solver->caotic_smooth_tollerance=SOGLIA_SMOOTH;
		Solver->collision_weight=COLLISION_WEIGHT;
		Solver->texture_local_weight=TEXTURE_LOCAL_WEIGHT;
		Solver->texture_weight=TEXTURE_WEIGHT;
		Solver->EdgesSource=EDGE_SIL_CONF;
		Solver->input_edges_images_edge_threshold=(BYTE)INPUT_EDGES_IMAGES_EDGE_THRESHOLD;
		Solver->textured_model_edge_threshold = (BYTE)TEXTURED_MODEL_IMAGES_EDGE_THRESHOLD;
		Solver->depth_model_edge_threshold=DEPTH_MODEL_IMAGES_EDGE_THRESHOLD;
		Solver->edge_mask_max_distance=TIPICAL_EROSION_FOR_OBJECT_MASK;
		Solver->edges_weight=EDGES_WEIGHT;
		Solver->salient_points_weight=SALIENT_POINTS_WEIGHT;
		Solver->salient_points_local_weight=SALIENT_POINT_LOCAL_WEIGHT;
		Solver->salient_points_th=SALIENT_POINT_MAX_DISTANCE;
		Solver->salient_depth_tollerance=SALIENT_DEPTH_THRESHOLD;
		Solver->salient_points_association_method=0;				// TODOLUCA
		Solver->salient_points_vertices=SalientVertices;
		Solver->salient_points_2D=Salient2DPoints;
	// Outliers
		Solver->n_iter_find_outliers=N_ITER_FIND_OUTLIERS;
		Solver->outliers_thresh=OUTLIERS_THRESH;
	// Contraints (DuplicaFrame)
		Solver->Apply_Constraints=apply_constrains;
	// Prediction
		Solver->prediction_iterations=PREDICTION_ITERATIONS;
		Solver->max_prediction_correspondences=MAX_PREDICTION_C;
	// Collisions
		if (avoid_collisions) Solver->SetCollisionDetector(collision_detector,selected_collision_detector_level,collision_mask,COLLISION_WEIGHTS_L0,COLLISION_WEIGHTS_L1,collision_sigma,ADDITIONAL_OBSERVATIONS,COLLISION_TTL);
	// Contact
		if (avoid_collisions) {
			Solver->contact_weight=CONTACT_POINTS_WEIGHT;
			Solver->contact_ttl=CONTACT_POINTS_TTL;
		}
		Solver->ResetContactPoints();
	// Extra Weights
		if (EXTRA_W_NAME_1[0]) {
			Solver->ext_bone_num=1;
			if (EXTRA_W_NAME_2[0]) Solver->ext_bone_num=2;
			EXTRA_W_NAME_TMP[0]=EXTRA_W_NAME_1;
			EXTRA_W_NAME_TMP[1]=EXTRA_W_NAME_2;
			Solver->ext_bone_names=EXTRA_W_NAME_TMP;
			Solver->ext_weight_edges=(float)EXTRA_W_E;
			Solver->ext_weight_optical_flow=(float)EXTRA_W_OF;
		}
	


	// -----------------------
	// - Work
	// -----------------------
	SetViewValidMask();





	if (fnc==1) {
		// Debug Images
		if (NeverDebug) Solver->DebugImages=PEDEBUG_NEVER;
		else Solver->DebugImages=PEDEBUG_ITERATIONS;
		
		// Process
		LoadFrames(ValidList[ed],Vista,num_viste);
		Solver->EstimateFrame(ValidList[ed],ValidList[st],REFINING,PREDICTING);
		MakeOutput(Model);
	}
	if (fnc==2) {
		// Debug Images
		if (NeverDebug) {
			Solver->DebugImages=PEDEBUG_NEVER;
		} else {
			if (AlwaysDebug) Solver->DebugImages=PEDEBUG_ITERATIONS;
			else Solver->DebugImages=PEDEBUG_FRAMES;
		}

		// Process
		int frame_step=1;
		if (st>ed) frame_step=-1;
		int last=st;
		if ((REFINING) && (last==ed)) last-=frame_step;

		char tmp_text[200];
		clock_t start_process=clock();
		clock_t start,end;
		
		for(int i=last+frame_step;i!=ed+frame_step;i+=frame_step) {
			if (!AlwaysDebug) Solver->DebugIndex=last+frame_step;
			if ((!NeverDebug) && (DebugLast) && (i==ed)) Solver->DebugImages=PEDEBUG_ITERATIONS;
			
			start=clock();
			LoadFrames(ValidList[i],Vista,num_viste);
			Solver->EstimateFrame(ValidList[i],ValidList[last],REFINING,PREDICTING);
			end=clock();
			
			cout<<"\n-------------------------------------------------------------------------\n";
			PrintTime(tmp_text,(UINT)((end-start)*1.0/CLOCKS_PER_SEC));
			cout<<"Frame "<<ValidList[i]<<" done. Time employed: "<<tmp_text<<"\n";
			cout<<"-------------------------------------------------------------------------\n\n\n";
			cout.flush();
			last=i;
		}
		
		MakeOutput(Model);
		_tzset();
		_strtime(tmp_text);
		cout<<"\n\n\nEstimation finished. Local Time is "<<tmp_text<<".\n";
		end=clock();
		PrintTime(tmp_text,(UINT)((end-start_process)*1.0/CLOCKS_PER_SEC));
		cout<<"Time employed: "<<tmp_text<<"\n\n";
	}
	if (fnc==3) {
		// -----------------------
		Mondo=new AdvancedRenderWindow("VirtualWindows",display,coma,NULL,0xFFFFFF);
		Mondo->EnableDraw();ShowWindow(Mondo->GethWnd(),SW_SHOW);
		// -----------------------
		curren_time=st+1;
		Model->Apply_Skin(curren_time);Model->ComputaNormals();
		Vector<3> C=Model->GetCenter();
		double radius=Model->GetRadius(C);
		Mondo->DefineLocalCoords(C,radius);
		ShowWindow(Mondo->GethWnd(),SW_SHOW);
		MSG msg;
		while (true) {
			if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
				if (WM_QUIT==msg.message) break;
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
		delete Mondo;
	}
	

	SDELETE(Solver);
	delete collision_detector;
	delete []collision_mask;
	SDELETE(Model);
	SDELETEA_REC(FeaturesList,num_viste);
	DestroyLoadFrames(num_viste);
	SDELETEA(Vista);
	return 0;
}






#include "input_parser.h"
