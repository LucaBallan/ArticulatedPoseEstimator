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
#include "SalientPoints.h"
#include "PoseEstimator.h"



PoseEstimator::PoseEstimator(View *Views,int num_views) {
	this->num_views=num_views;
	this->Views=Views;


	// Default Parameters
	iteration_l0=1;
	iteration_l1=0;
	proximity_factor=0.01;
	Stop_Error_Th=1e-20;
	internal_iteration=10;
	local_optimization_weight=1.0;
	// Optical Flow
		max_of_info_distance=0.2;
		OpticalFlowIterations=30.0;
		opticalflow_local_weight=1.0;
		opticalflow_weight=1.0;
	// Depth
		depth_local_weight_l0=1.0;
		depth_local_weight_l1=1.0;
		depth_weight=1.0;
	// Silhouettes
		EdgesSource=0;
		max_length_search=80;
		gradient_tollerance=0.0;
		caotic_smooth_tollerance=2.0;
		textured_model_edge_threshold=30;
		depth_model_edge_threshold=1.0;
		input_edges_images_edge_threshold=30;
		edge_mask_max_distance=4;
		edges_weight=1.0;
	// Collisions
		collision_weight=1.0;
	// Contact
		contact_weight=0.0;
		contact_ttl=0;
	// SalientPoints
		salient_points_weight=1.0;
		salient_points_local_weight=1.0;
		salient_points_th=30.0;
		salient_depth_tollerance=1.0;
		salient_points_association_method=0;
		salient_points_vertices=NULL;
		salient_points_2D=NULL;
	// Texture
		texture_local_weight=1.0;
		texture_weight=1.0;
	// General Outlier
		n_iter_find_outliers=0;
		outliers_thresh=7.0;
	// Contraints (DuplicaFrame)
		Apply_Constraints=true;
	// Prediction
		prediction_iterations=10;
		max_prediction_correspondences=10000;
	// Extra weighted bones 
		ext_bone_num=0;
		ext_bone_names=NULL;
		ext_weight_edges=1.0;
		ext_weight_optical_flow=1.0;
	// Debug
		OutDebugDir=NULL;
		DebugIndex=0;
		Debug_Type=0;
		DebugImages=PEDEBUG_NEVER;

}
PoseEstimator::~PoseEstimator() {
	SDELETEA(OutDebugDir);
}
void del_frame_directories(char *dirname,char *subdirname,char *file_template) {
	_strlwr(subdirname);
	if (str_begin(subdirname,"frame_")) {
		char *fulldir=new char[strlen(dirname)+strlen(subdirname)+10];
		strcpy(fulldir,dirname);
		strcat(fulldir,"\\");
		strcat(fulldir,subdirname);
		rmdir(fulldir);
		delete []fulldir;
	}
}
void PoseEstimator::SetOutDebugDir(char *dir,bool delete_all) {
	str_replace(&OutDebugDir,dir);
	if (OutDebugDir) {
		mkdir(OutDebugDir);

		if (delete_all) directory_list(OutDebugDir,del_frame_directories,NULL);

		char *tmp_dir=new char[strlen(OutDebugDir)+20];
		for(int i=0;i<num_views;i++) {
			sprintf(tmp_dir,"%s\\%i",OutDebugDir,i);
			if (Debug_Type!=4) mkdir(tmp_dir);
			if (delete_all) del_all_files_in_dir(tmp_dir);
		}
		sprintf(tmp_dir,"%s\\motion",OutDebugDir);
		mkdir(tmp_dir);

		delete []tmp_dir;
	}
}
void PoseEstimator::CopyParametersTo(PoseEstimator *dst,bool CopyOutDebugDir) {
		dst->iteration_l0=iteration_l0;
		dst->iteration_l1=iteration_l1;
		dst->proximity_factor=proximity_factor;
		dst->Stop_Error_Th=Stop_Error_Th;
		dst->internal_iteration=internal_iteration;
		dst->local_optimization_weight=local_optimization_weight;
	// Optical Flow
		dst->max_of_info_distance=max_of_info_distance;
		dst->OpticalFlowIterations=OpticalFlowIterations;
		dst->opticalflow_local_weight=opticalflow_local_weight;
		dst->opticalflow_weight=opticalflow_weight;
	// Depth
		dst->depth_local_weight_l0=depth_local_weight_l0;
		dst->depth_local_weight_l1=depth_local_weight_l1;
		dst->depth_weight=depth_weight;
	// Silhouettes
		dst->EdgesSource=EdgesSource;
		dst->max_length_search=max_length_search;
		dst->gradient_tollerance=gradient_tollerance;
		dst->caotic_smooth_tollerance=caotic_smooth_tollerance;
		dst->textured_model_edge_threshold=textured_model_edge_threshold;
		dst->depth_model_edge_threshold=depth_model_edge_threshold;
		dst->input_edges_images_edge_threshold=input_edges_images_edge_threshold;
		dst->edge_mask_max_distance=edge_mask_max_distance;
		dst->edges_weight=edges_weight;
	// Collisions
		dst->collision_weight=collision_weight;
	// Contact
		dst->contact_weight=contact_weight;
		dst->contact_ttl=contact_ttl;
	// SalientPoints
		dst->salient_points_weight=salient_points_weight;
		dst->salient_points_local_weight=salient_points_local_weight;
		dst->salient_points_th=salient_points_th;
		dst->salient_depth_tollerance=salient_depth_tollerance;
		dst->salient_points_association_method=salient_points_association_method;
		dst->salient_points_vertices=salient_points_vertices;
		dst->salient_points_2D=salient_points_2D;
	// Texture
		dst->texture_local_weight=texture_local_weight;
		dst->texture_weight=texture_weight;
	// General Outlier
		dst->n_iter_find_outliers=n_iter_find_outliers;
		dst->outliers_thresh=outliers_thresh;
	// Contraints (DuplicaFrame)
		dst->Apply_Constraints=Apply_Constraints;
	// Prediction
		dst->prediction_iterations=prediction_iterations;
		dst->max_prediction_correspondences=max_prediction_correspondences;
	// Extra weighted bones 
		dst->ext_bone_num=ext_bone_num;
		dst->ext_bone_names=ext_bone_names;
		dst->ext_weight_edges=ext_weight_edges;
		dst->ext_weight_optical_flow=ext_weight_optical_flow;
	// Debug
		if (CopyOutDebugDir) dst->SetOutDebugDir(OutDebugDir);
		dst->DebugIndex=DebugIndex;
		dst->Debug_Type=Debug_Type;
		dst->DebugImages=DebugImages;
}