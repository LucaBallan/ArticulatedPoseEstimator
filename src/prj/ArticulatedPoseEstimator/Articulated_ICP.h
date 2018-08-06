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






#define INVALID_INTERNAL_EDGES   true						// TODOLUCA : tutto il blocco di codice funziona solo per rope!!


#define AICP_ZBUFFER_TOLLERANCE     0.02					// 2% tollerance on the visibility detection (with respect to the entire rendering volume)
//#define AICP_USE_L2_INSTEAD								// Use L2 instead of L1 for the global error
#define RANDOMIZE_TTL               0.1


class Articulated_ICP : public PoseEstimator {
	
	struct EdgeSearch_struct {
		Vector<2> target;
		int       target_type;				// 0 External, 1 Internal, -1 not found
		double    gradient_threshold;

		Vector<2> gradient;
		int       gradient_type;			// 0 External, 1 Internal
		int       win_d;					
		int       win_d_for_input_mask;		// win_d_for_input_mask must be > than win_d
		BYTE      edge_threshold;

		Bitmap<GreyLevel>    *mask;
		Bitmap<GreyLevel>    *edge_map;

		bool      search_only_external_edges;
	};

	// Renders
	AdvancedRenderWindow *GPU_OBJ;

	// Parameters
	Skinned_Mesh *mesh;
	Features **FeaturesList;
	Degrees_of_Freedom_Table_Entry *Constraint_level0;
	Degrees_of_Freedom_Table_Entry *Constraint_level1;

	
	// Renderers
	ModelRenderer               *ModelRender;
	DepthMapRenderer            *DepthRender;
	DepthEdgesDetectorRenderer  *EdgesRenderer;
	FrameBuffer_struct	         Silhouette_FBO;
	FrameBuffer_struct	         Texture_FBO;
	FrameBuffer_struct	         Depth_FBO;
	FrameBuffer_struct	         Edges_FBO;
	Bitmap<GreyLevel>           *SilhouetteImage;
	Bitmap<ColorRGBA>           *TextureImage;
	Bitmap<ColorFloat>          *DepthImage;
	Bitmap<ColorFloat>          *EdgesImage;

	// Locals
	IKSolver *IK_Solver;
	IKSolver *IK_Solver_Pyramid_Level_1;

	// --- Targets
	// 1D Target
	Array<TargetList_1D_Item_type> *TargetList_1D;
	// 2D Target
	Array<TargetList_2D_Item_type> *TargetList_2D;
	Array<TargetList_2D_Item_type> *Feat_TargetList;
	// 3D Target
	Array<TargetList_3D_Item_type> *TargetList_3D;
	Array<TargetList_3D_Item_type> *TargetList_3D_ContactPoints;
	// ----------

	// CorrectionVectors
	int *CorrVertexList;

	// Debug 
	Bitmap<ColorRGB> *DebugImage;
	int LastUsed_Feature_StartFrame,LastUsed_Feature_EndFrame;
	bool show_debug_information;
	bool show_minimal_debug_information;

	// Debug: chiamare dopo aver calcolato le rispettive informazioni con GetCorrespondences
	void ShowCorrespondencesInfo(int Level);
			void   SilhouetteEdgesSalientPointsOpticalFlow_CorrespondencesResidual(double &residual_S,double &residual_E,double &residual_P,double &residual_O,int *n_observations_S,int *n_observations_E,int *n_observations_P,int *n_observations_O);
			double Depth_CorrespondencesResidual(int *n_observations);
			double Texture_CorrespondencesResidual(int *n_observations);
			double Feat_3D_CorrespondencesResidual(int *n_observations);


	// Adaptation
	void AdaptInternalDebugBuffers(int width,int height);
	void Adapt_Silhouette_FBO_size(int width,int height);
	void Adapt_Texture_FBO_size(int width,int height);
	void Adapt_Depth_FBO_size(int width,int height);
	void Adapt_Edges_FBO_size(int width,int height);
	void SetNearFarPlane(int v_i);
	struct {
		bool    SilhouetteReady;
		bool    DepthReady;
		bool    EdgesReady;
		bool    TextureReady;
		bool    NearFarPlaneReady;
		double  min_z_plane,max_z_plane;
		float   z_tollerance;
		float   edge_threshold_r;
	} buffer_status;
	void  ResetBufferStatus();
		void  RenderSilhouetteImage(int v_i);			// Note: all these function need 
		void  RenderDepthImage(int v_i);				//       ResetBufferStatus to be 
		void  RenderEdgesImage(int v_i);				//       called beforehand.
		void  RenderTextureImage(int v_i);				// RenderEdgesImage return the correct edge threshold for the edges computed on the rendered image/depth maps
	


	// Observations
	double *mean_move;
	unsigned int *mean_move_num;
	void RetrieveObservations(int Level,int frame_index);
			void GetSilhouettesCorrespondences(int view,int Level);
			void GetEdgesCorrespondences(int view,int Level);
			void GetDepthCorrespondences(int view,int Level);
			void GetTextureCorrespondences(int view,int Level);
			void GetSalientPointCorrespondences(int view,int Level,int frame_index);
				void SearchForEdgeCorrespondence(int x,int y,int vertex_index,View *c_v,EdgeSearch_struct *ui_EdgeStruct);
				static bool FindCoherentEdge(CK_SEG_INFO_STRUCT *Info,EdgeSearch_struct *ui_Info);
	void WriteDebugImages(int Level,int frame_index);
			void DebugSilhouette(int view,int Level);
			void DebugDepth(int view,int Level);
			// Internal (call after RenderDepthImage and RenderEdgesImage)
			void DebugEdgeDetectorOnImages(EdgeSearch_struct *ui_Info,Bitmap<ColorRGB> *output);
			void DebugEdgeDetectorOnRenderedImages(int view,float edge_threshold,int win_d,Bitmap<ColorRGB> *output);

	
	// Collision detector (shared by this, ik_solver, ik_solver_pyr_1) -> no multitasking between those
	bool *collision_mask;
	HyperMeshPartitionTree *collision_detector;
	Array<ordered_pair> *collision_intersecting_faces;

	// Distance Transform
	SilhouetteDistance_Buffer **DistanceTransform_Buffers;
	
	// Outliers
	void Clean_TargetList_2D_FromOutliers(int first_element_to_check);

public: 
	//
	// Constructor:
	//			The first frame (0) of mesh must have Absolute coordinates setted.
	//			debug_level = 0 (none) 
	//						  1 (minimal) 
	//						  2 (full)
	// Thread-safe:
	//			mesh    -> write and read                (not sharable)
	//			Views   -> read only                     (sharable)
	//			GPU_OBJ -> read only                     (sharable)
	//			Constraint_level0 -> read only           (sharable)
	//			Constraint_level1 -> read only           (sharable)
	Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
		            Features **FeaturesList,View *Views,int num_views,
					Degrees_of_Freedom_Table_Entry *Constraint_level0=NULL,Degrees_of_Freedom_Table_Entry *Constraint_level1=NULL,
					int debug_level=2);
	//
	//  Note: NVIDIA goes in deadlock here if a task that was using this class still exists.
	//        Before deleting the Articulated_ICP class, please delete the tasks.
	virtual ~Articulated_ICP();




	virtual void SetCollisionDetector(HyperMeshPartitionTree *collision_detector,int collision_detector_level,bool *collision_mask,
									  double collision_local_weight_l0,double collision_local_weight_l1,double collision_sigma,int max_collision_observations,int ttl);


	// Info
	IKSolver *GetIKSolverL0();
	Array<TargetList_2D_Item_type> *GetFeat_TargetList();







// Simple Mode:
	// Should be ready: (in frame_index)
	//			Mesh 
	//					Abs matrices ready
	//			View
	//					I,E,P,UP updated
	//					Image channels Loaded
	virtual double EstimateFrame(int frame_index,int base_frame_index,bool Refinement,bool UsePrediction);
	//			-) the resulting mesh has
	//					up to date Abs and Relative matrices
	//					up to date Skin 
	//					ut to date Normals
	//
	



	virtual void   ResetContactPoints();



// Advanced:
	// Pre-processing, it needs:
	//			View						// TODO insert these thinks inside prepare mesh!!!
	//					I,E,P,UP updated
	//					Mask Loaded
	//					ZMask
	void PrepareMesh(int frame_index);												// Set the frame
	void GetOpticalFlowCorrespondences(int to_estimate_frame,int start_frame);		// Genera Feat_TargetList
																					//  -) Needs:
																					//      PrepareMesh(base_frame_index); and Views updated for base_frame_index


	// Should be ready: (in frame_index)
	//			Mesh 
	//					BoneRoot with correct initial pose
	//					Constraints set and applied
	//					Skin applied
	//					Normals computed
	//					Area    computed
	//			View
	//					I,E,P,UP updated
	//					Image channels Loaded
	//			AdaptInternalBuffer   (adatta i buffer alla risoluzione delle camere)
	//			Feat_TargetList ready (GetOpticalFlowCorrespondences)
	//			Needs:          
	//					PrepareMesh(frame_index);
	double Process(int frame_index);
	double GetResidual(int frame_index,bool write_debug_images);
	//			-) the resulting mesh has
	//					up to date Abs and Relative matrices
	//					up to date Skin 
	//					ut to date Normals
	//
	
	double PredictFrame(int frame_index,int base_frame_index);
	void   GetOnlyPredictionCorrespondences(Skinned_Mesh *ref_mesh,int base_frame_index);
	double PredictFrameMinimization(int frame_index);

	
	// Global Errors:
		// Compute Global Residual
		//   0 sil/edges                 average distance of a pixel in one image with respect to the pixels in the other image       [px] (L1/n_observations)
		//   1 depth                     average depth distance of a pixel          [m] (L1/n_observations)
		//   2 texture                   average distance in RGB color? or texture pixels space   [px] (L???)
		//   3 optical flow              average distance of the correspondences     [px] (L1/n_observations)
		//   4 collisions                number of colliding faces (indicative number of intersecting elements) (L0)
		//void ComputeGlobalResiduals(double *residuals,double *n_observations);


		//
		// Silhouette distance (norm 1)
		//
		//      -) average distance of a pixel in one image with respect to the pixels in the other image       [px] (L1/n_observations)
		//      -) infinite if the object is completely out of the screen in either the actual or the estimated one
		//		-) skin must be ready before the call
		//
		void   PrepareFrameForSilhouetteResidual();			// run it before every frame, once the new images are loaded into View::Mask, before calling ComputaSilhouetteDistance
		double ComputaSilhouetteResidual(double *n_observations);
		double ComputaEdgesResidual(double *n_observations);


		//
		// Depth Residual
		//
		//      -) average depth distance of a pixel          [m] (L1/n_observations)
		//      -) does not consider non overlapping points 
		//                        -> it has a minimum also if the images do not overlap at all (silhouette should recover)
		//		-) skin must be ready before the call
		//
		double ComputaDepthResidual(double *n_observations);


		//
		// Edge distance (norm 1)
		//
		//      -) average distance of the correspondences     [px] (L1/n_observations)
		//      -) skin and area must be ready before the call 
		//
		double ComputaEdgeResidual(double *n_observations);


		//
		// Salient points distance (norm 1)
		//
		//      -) average distance of the correspondences     [px] (L1/n_observations)
		//      -) skin and area must be ready before the call 
		//
		double ComputaSalientPointResidual(double *n_observations,int frame_index);


		//
		// Optical Flow Residual
		//      
		//      -) average distance of the correspondences     [px] (L1/n_observations)
		//      -) skin and area must be ready before the call 
		// 
		double ComputaOpticalFlowResidual(double *n_observations);

		
		//
		// Collisions Residual
		//      
		//      -) number of colliding faces (indicative number of intersecting elements) (L0)
		//      -) skin must be ready before the call
		double ComputaCollisionsResidual();



		//
		// Texture Residual
		//      
		//      -) average distance in RGB color? or texture pixels space   [px] (L???)
		//      -) skin and area must be ready before the call  TODO
		// Texture Distance of the current iteration (usable also inside iteration) 
		double ComputaTextureResidual(double *n_observations);			// TODO



		//
		// Note:
		//             norm 1   ->    average distance        least sensitive
		//             norm 2   ->    variance                ....
		//             ....
		//             norm inf ->    maximum distance        most sensitive
		//




	/*
	void Compute_W(int start,int end,int outliers_thresh,double texture_weight);
	void Compute_Correspondences_Frame(int frame_index,int starting_frame_index,int n_iter_find_outliers,int outliers_thresh,double texture_weight,Array<TargetList_2D_Item_type_Frame_Info> *New_TargetList);
	*/
};


////////////////////////////////////
//
//  Norm Minimization:
//
//        argmin_x   | e(x) |_p
//
//
//     - the norm power p denotates its sensitivity to outliers, precisely to the entity of the
//       error with respect to the number of errors.
//
//
//     - low p:  prefer solutions with a small number of error
//
//
////////////////////////////////////



















