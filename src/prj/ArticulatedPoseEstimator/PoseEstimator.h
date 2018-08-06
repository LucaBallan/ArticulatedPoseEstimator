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






#define POSE_ESTIMATOR_DEBUG_FILE_EXTENSION "png"


#define PEDEBUG_NEVER	     0
#define PEDEBUG_FRAMES       2
#define PEDEBUG_ITERATIONS   1
//#define PEDEBUG_MOTIONFILE   4





class PoseEstimator {
protected:
	// Views
	View *Views;
	int num_views;
	
	// Debug Output dir
	char *OutDebugDir;

	void CopyParametersTo(PoseEstimator *dst,bool CopyOutDebugDir=true);

public: 
	PoseEstimator(View *Views,int num_views);																													// debug_level = 0 (none), 1 (minimal), 2 (full)
	virtual ~PoseEstimator();


	virtual void SetCollisionDetector(HyperMeshPartitionTree *collision_detector,int collision_detector_level,bool *collision_mask,
		                      double collision_weight_l0,double collision_weight_l1,double collision_sigma,int max_collision_observations,int ttl) = 0;


	//
	// IMPORTANT: Every parameter one adds, modify the CopyParametersTo and the Constructor function
	//


	// Parameters for Process (can be changed at any time)
	int iteration_l0;
	int iteration_l1;								// l0=0 & l1=0 -> compute the error only
	double proximity_factor;
	double Stop_Error_Th;
	int internal_iteration;
	double local_optimization_weight;
	// Optical Flow
		double max_of_info_distance;				// [screen    % ]   0.2  -> 0.2%
		double OpticalFlowIterations;				// [iteration % ]   30   ->  30%
		double opticalflow_local_weight;
		double opticalflow_weight;
	// Depth
		double depth_local_weight_l0;
		double depth_local_weight_l1;
		double depth_weight;
	// Silhouettes/Edges
		int EdgesSource;							// 0xX0 use silhouette only
													// 0xX1 use edges if available
													// 0x01 use edges from depth
													// 0x11 use edges from textured image
		int max_length_search;						// Se ho una distanza di 4 frame è ok 40
		double gradient_tollerance;					// 0   passano anche i perpendicolari 
													// 0.8 taglia quelli > 45°
													// 1.0 passano solo i paralleli
		double caotic_smooth_tollerance;			// Smooth of the founded correspondences
		BYTE   textured_model_edge_threshold;
		double depth_model_edge_threshold;
		BYTE   input_edges_images_edge_threshold;
		int    edge_mask_max_distance;
		double edges_weight;
	// Collisions
		double collision_weight;
	// Contact
		double contact_weight;
		int    contact_ttl;
	// SalientPoints
		double salient_points_weight;
		double salient_points_local_weight;
		double salient_points_th;
		double salient_depth_tollerance;
		UINT   salient_points_association_method;
		Array<UINT>    *salient_points_vertices;
		SalientPoints **salient_points_2D;
	// Texture
		double texture_local_weight;
		double texture_weight;

	// General Outliers Detector
		int n_iter_find_outliers;					// indica il numero di iterazioni finali in cui dovranno essere calcolati gli outliers. 0 indica mai
		double outliers_thresh;						// [pixels] soglia per identificare se è un outliers
	// Contranits (for DuplicaFrameOnly)
		bool Apply_Constraints;
	// Prediction
		int prediction_iterations;
		int max_prediction_correspondences;
		
	// Extra weighted bones  (TODOAAA TODOLUCA Implemented only for edges and optical flow)
		int       ext_bone_num;
		char    **ext_bone_names;
		float     ext_weight_edges;
		float     ext_weight_optical_flow;


	// Debug
	int DebugIndex;					// File index
	int Debug_Type;					// 0 Pixel discrepancy error
									// 1 Estimated silhouettes
									// 2 Colour silhouette overlapping
									// 3 Show correspondences
									// 4 No debug images
									// 5 Show only salient points
									// 6 Wireframe rendering
	int DebugImages;				// PEDEBUG_NEVER         Never
									// PEDEBUG_ITERATIONS    Save each algorithm iterations
									// PEDEBUG_FRAMES        Save only the result
	
	void SetOutDebugDir(char *dir,  // To be called after Debug_Type is set 
		                bool delete_all=false);		// NULL -> does not create anything






	virtual void   ResetContactPoints() = 0;
	virtual double EstimateFrame(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction) = 0;
	// View Mask must be Loaded
	// Stima frame_index (immagini) partendo da starting_frame_index (di mesh), salva il risultato in frame_index (di mesh).
};

