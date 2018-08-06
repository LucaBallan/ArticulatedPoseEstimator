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






#define SAICP_POLICY_WTA		0

struct ParticleElement {
	int      Index;						// indice quasi univoco (non distingue le particle a differenti iterazioni)
	double  *State;						// identificatore univoco
	double   Residual;					// DBL_MAX = Unknown
	int      Type;						// 0 previous best particle
										// 1 tracked particle
										// 2 previous best particle noised
										// 3 tracked particle noised
	double  *Speed;						// 
	double  *Local_Optimum;				// 
	double   Local_Optimum_Residual;    // 

	// Info
	double   Residual_S;				// Silhouette   Residual
	double   Residual_D;				// Depth        Residual
	double   Residual_O;				// OpticalFlow  Residual
	double   Residual_C;				// Collision    Residual
	double   Residual_T;				// Texture      Residual
	double   Residual_P;				// SalientPts   Residual
	double   Residual_E;				// Edges        Residual
	double   n_observations_S;
	double   n_observations_D;
	double   n_observations_O;
	double   n_observations_T;
	double   n_observations_P;
	double   n_observations_E;
};

class Stocastic_Articulated_ICP : public PoseEstimator {
	Skinned_Mesh                  *main_mesh;
	Skinned_Mesh                 **mesh_buffer;
	HyperMeshPartitionTree       **mesh_buffer_cd;
	Articulated_ICP              **AICP;
	MultiProcessing               *Task;
	HANDLE                         TaskFinished;
	double                       **IKState;
	double                        *ABSIKState;
	int num_processors;
	int num_particles;
	
	
	// IKSolver Object (ReadOnly)
	int num_Freedom_Degrees;
	Freedom_Degrees_type *Freedom_Degrees;
	IKSolver *IK_Solver_Ref;
	Vector<3> VolumeCenter;
	double VolumeScaleFactor;
	static void GetState(Skinned_Mesh *mesh,int frame_index,double *IKState);				// Relative must be computed
	static void SetState(Skinned_Mesh *mesh,int frame_index,double *IKState);
	void GetDOFState(Skinned_Mesh *mesh,int frame_index,double *IKState,double *DOF);		// Relative must be computed
	void SetDOFState(Skinned_Mesh *mesh,int frame_index,double *IKState,double *DOF);
	static void TaskMinimize(void *data,Stocastic_Articulated_ICP *saicp);

	// Shared between the tasks
	int tmp_frame_index;
	int tmp_debug_index;

	// Particles
	int                ParticleCounter;
	ParticleElement   *Particles;
	LowestScoreChart  *ParticleChart;

	// Noise
	double *dof_noise_ratio;

	// Internals
	void Minimize(int particle_j,int proc,int frame);
	void AddNoise(int particle);
	void ResampleParticle();


	// Debug
	bool show_debug_information;
	bool show_minimal_debug_information;
	void ShowParticleInfo(ParticleElement *CParticle,double p_residual,bool account_for_local_residuals=true);


	// PSO
	double EstimateFramePSO(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction);
	static void TaskMinimizePSO(void *data,Stocastic_Articulated_ICP *saicp);
	void MinimizePSO(int particle_j,int proc,int frame);
	void AddNoisePSO(int particle);

public:
	Stocastic_Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
							  Features **FeaturesList,View *Views,int num_views,
							  Degrees_of_Freedom_Table_Entry *Constraint_level0,Degrees_of_Freedom_Table_Entry *Constraint_level1,
							  int debug_level,int particle_debug_level,
							  int num_processors,int num_particles_per_iteration,int resample_elements);
	virtual ~Stocastic_Articulated_ICP();
	//
	//			num_processors   (0 -> autodetect)
	//

	
	// collision_detector only used to get the level
	virtual void SetCollisionDetector(HyperMeshPartitionTree *collision_detector,int collision_detector_level,bool *collision_mask,
									  double collision_local_weight_l0,double collision_local_weight_l1,double collision_sigma,int max_collision_observations,int ttl);


	// Parameters
	int    initial_iteration_l0;
	int    initial_iteration_l1;
	float  resample_k;
	int    resample_policy;						// TODO not used
	double noise_ratio;							// sigma = extension*ns
	double noise_decrease_ratio;
	double particle_internal_iteration_ratio;
	bool   apply_noise_on_root;


	// Note:
	//			DebugImages =  PEDEBUG_NEVER       Never
	//						   PEDEBUG_ITERATIONS  Save each particle
	//						   PEDEBUG_FRAMES      Save only the final result
	//


	virtual void   ResetContactPoints();		// TODOLUCA ContactPoints not fully implemented in stocastic ICP


	//			Mesh 
	//					Abs matrices ready
	//			View
	//					I,E,P,UP updated
	//					Image channels Loaded
	//					DebugImages ready with correct size
	virtual double EstimateFrame(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction);
	//			-) the resulting mesh has
	//					up to date Abs and Relative matrices
	//					up to date Skin 
	//					ut to date Normals
	//

};



