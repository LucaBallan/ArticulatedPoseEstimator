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






#define INITIAL_IKSOLVER_COLLISION_BUFFERSIZE              80
#define USE_CONIC_DISTANCE_FUNCTION
//#define FORCE_ONE_COLLISION_PER_VERTEX						// If set it is always not deterministic even with enough observations


struct Freedom_Degrees_type {
	int type;
	Bone *Ref;				
	int BoneIndex;			// Redundant for fast memory indexing
};


class conic_f {
	double sigma;
	double coeff_1,coeff_2,coeff_3;
	Matrix A;

	double g(double x);
	double dg(double x);

public:
	conic_f() : A(3,3) {};

	// Init
	void     Set(double sigma);
	double     F(Vector<3> x,Vector<3> o,Vector<3> d,double r);
	Vector<3> DF(Vector<3> x,Vector<3> o,Vector<3> d,double r);
};



//
// IKSolver minimizza:
//
//		(area^2)*L2(sil)											+
//		(added_sqrt_weight^2)*(area^2)*L2(Feat)						+ 
//		(collision_sqrt_weight^2)*(area^2)*(conic^2)(collisions)	+
//		(depth_local_weight)*(area^2)*L2(depth)
//
//
//        TODO   should not be area^2
//        TODO** se aggiungo altre informazioni ric il peso e' sqrt
//

class IKSolver {
	// Room
	Vector<3> VolumeCenter;
	double VolumeScaleFactor;

	// Mesh
	Skinned_Mesh *mesh;
	
	// Freedom_Degrees
	int num_Freedom_Degrees;
	Freedom_Degrees_type *Freedom_Degrees;
	void Init_Freedom_Degrees();

	// Derivates 
	static Matrix **Derivates_;
	static void InitRigidDerivates();
	static void getRotationDerivates(Matrix *MK,int type,Matrix *Out);

	// Cache per il calcolo di D_AM
	Matrix **D_AM;
	Vector<3> *ROW;
	void Computa_D_AM(int frame_index);
	void Computa_D_AM_S(int frame_index,int bone,int degree_index);

	// LM solver
	int times_upd,Zero_Anomalies,Tot_Calculations;
	int temp_frame_index;
	Array<TargetList_1D_Item_type> *Temp_TargetList_1D;
	Array<TargetList_2D_Item_type> *Temp_TargetList_2D;
	Array<TargetList_3D_Item_type> *Temp_TargetList_3D;
	double *IKState;
	double *Calcolated_p;
	double *Solution;
	double *upperBound;
	double *lowerBound;
	double *LM_Mem_Storage;
	static void LM_Jacobian(double *p,double *jac,int m, int n,void *data);
	static void LM_Function(double *p,double *x,int m,int n,void *data);
	static void LM_Update  (double *p,IKSolver *THIS_,bool Skin);
	void Init_LM_Solver();


	// Rel Matrices must be Ready
	void GetCurrentDegreesOfFreedom_Values(int frame_index,double *Values_DOF);
	void GetState_and_CurrentDegreesOfFreedom_Values(int frame_index,double *IKState,double *Values_DOF);


	// Debug Information
	bool show_debug_information;
	bool show_minimal_debug_information;


	// Dynamic Observation
	int dynamic_observations;


	// Collision detector
		// External
		HyperMeshPartitionTree      *collision_detector;
		bool                        *collision_mask;
		Array<ordered_pair>         *collision_intersecting_faces_buffer;
		// 	Internal
		Array<collision_constraint> *collision_constraint_list;
		bool                         dynamic_collision_update;
		conic_f						 distance_function;
		Truncated_Quadratic          outlier_function; // TODOAAA
		// Parameters	
			int    collision_ttl;
			double collision_distance_sigma;
			double collision_sqrt_weight;
			double collision_randomize_ttl_percentage;
			

public:


	// (not parallelizable)
	IKSolver(Skinned_Mesh *mesh,Vector<3> VolumeCenter,double VolumeRadius,			  // I vincoli su mesh dei bones devono essere settati before call this
			 bool show_debug_information,bool show_minimal_debug_information);		  // The capture volume is a ball (VolumeCenter,VolumeRadius)

	~IKSolver();



	// Ser parameters (do not do anything) (they must be called only once at the beginning)
	void SetDynamic_Observations (int dynamic_observations);
	void SetCollisionDetector    (HyperMeshPartitionTree *collision_detector,bool *collision_mask,Array<ordered_pair> *collision_intersecting_faces_buffer);


	// Info
	void GetFreedomDegressMap(int &num_Freedom_Degrees_,Freedom_Degrees_type *&Freedom_Degrees_);			// Create Freedom_Degrees_ (to be deleted by the caller)
	double GetVolumeScaleFactorAndVolumeCenter(Vector<3> &VolumeCenter);
	double *GetUpperBounds();
	double *GetLowerBounds();
	double GetCollisionWeight();
	int    GetCollisionTTL();



	// Parameters                     (can be changed at any time before Perspective_Solve_LM)
		// 2D Modify Weights
		int    last_weighted_features;				// Asserisce che il blocco di features da 0 a last_weighted_features deve essere pesato con peso added_sqrt_weight
		double added_sqrt_weight;


	// Add Soft Contraints            (can be called at any time before Perspective_Solve_LM)
		// collisions (SetCollisionDetector & Dynamic Observation must be setted otherwise nothing happend)
		void SetCollisionParameters(int ttl,double randomize_ttl_percentage,double weight,double distance_sigma);	// randomize_ttl_percentage == 0.0 -> all with the same ttl
		void UpdateCollisionWeights(double weight);
		// Manual Collision Update
		void Add_Collision_Constraints();							// Add soft contraints on the base of the current collisions (mesh/skin should be updated)
		void Clear_Collision_Constraints();							// PS: add only if there is space left in the observations
		void Copy_Collision_Constraints(IKSolver *IK);
		void Collision_Increase_Time();								// tick a clock and eliminate dead collisions
		// Automatic Update
		void Dynamic_Collision_Update(bool activate);				// Update collisions ad every update of the mesh (either F or J call)
																	// It does not work since F and JF changes too frequently for the LM which is not able no more 
																	// to deal with the problem
		double Collision_Residual(int *n_observations);				// Chiamare dopo Add_Collision_Constraints

		// Contact Points
		void Add_Contact_Points(Array<TargetList_3D_Item_type> *TargetList_3D_ContactPoints,double weight,int ttl);			// (mesh/skin should be updated) (non resetta TargetList_3D, addiziona solo)
		void Contact_Points_Increase_Time(Array<TargetList_3D_Item_type> *TargetList_3D_ContactPoints,double weight,int ttl);

	// Perspective_Solve_LM:
		// before call
		//  - Skin deve essere must be Ready
		//  - Abs e Rel Matrices must be Ready
		bool Perspective_Solve_LM(int frame_index,
								  Array<TargetList_1D_Item_type> *TargetList_1D,
								  Array<TargetList_2D_Item_type> *TargetList_2D,
								  Array<TargetList_3D_Item_type> *TargetList_3D,
								  int Num_of_Iteration=10,
								  double Solution_Proximity=0.01,
								  double Stop_Error_Th=1e-20);
		//			-) the resulting mesh has
		//					up to date Abs and Relative matrices
		//					up to date Skin 
		//					ut to date Normals
		double LastComputedResidual;
		



	






	////////////////////////// To test
	//bool Perspective_Solve_ALPHA_WEIGHTS(int frame_index,Array<TargetList_2D_Item_type_Frame_Info> *TargetList_2D,int Num_of_Iteration,double Solution_Proximity,double Stop_Error_Th);
	//Array<TargetList_2D_Item_type_Frame_Info> *Temp_TargetList_FI;
	//void LM_Function_WEIGHTS(double *p,double *x,int m,int n,void *data);
};