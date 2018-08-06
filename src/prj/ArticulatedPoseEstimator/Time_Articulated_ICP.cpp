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
#include "IKSolver.h"
#include "PoseEstimator.h"
#include "Articulated_ICP.h"
#include "Time_Articulated_ICP.h"



MultiFrameSkinnedMesh::MultiFrameSkinnedMesh(Skinned_Mesh *mesh,int window_size) {
	this->mesh=mesh;
	this->window_size=window_size;
	this->MultiFrameMesh=new Skinned_Mesh();
	this->obj_face_index=new int[window_size];

	Skinned_Mesh *tmp_mesh=new Skinned_Mesh();
	tmp_mesh->AddMesh(mesh,2);

	for(int i=0;i<window_size;i++) {
		char prefix[50];
		sprintf(prefix,"Frame%04i_",i);
		if (i==0)  tmp_mesh->BoneRoot->AddPrefix(prefix);
		else       tmp_mesh->BoneRoot->SubStPrefix(prefix);
		obj_face_index[i]=MultiFrameMesh->num_f;
		MultiFrameMesh->AddMesh(tmp_mesh);
	}
	SDELETE(tmp_mesh);
}

MultiFrameSkinnedMesh::~MultiFrameSkinnedMesh() {
	SDELETE(MultiFrameMesh);
	SDELETEA(obj_face_index);
}

Skinned_Mesh *MultiFrameSkinnedMesh::GetMesh() {
	return MultiFrameMesh;
}




void MultiFrameSkinnedMesh::SetFrames(int start_frame) {
	// todo** mesh absolute should be ready 
	int frame_index;

	for(int j=0;j<MultiFrameMesh->num_bones;j++) {
			
		// Get the original bone from original mesh
		const char *ext_bone_name=MultiFrameMesh->BoneRefTable[j]->GetName();
		sscanf(ext_bone_name+strlen("Frame"),"%d",frame_index);
		const char *original_bone_name=ext_bone_name+strlen("Frame0000_");
		Bone *original_bone=mesh->BoneRoot->SearchBone(original_bone_name);

		// Copy original bone (frame start_frame+frame_index) in MultiFrameMesh->BoneRefTable[j] (frame 1)
		((MultiFrameMesh->BoneRefTable[j]->GetABSRotationArray())[1])->Set((original_bone->GetABSRotationArray())[start_frame+frame_index]);
		((MultiFrameMesh->BoneRefTable[j]->GetABSTranslationArray())[1])=((original_bone->GetABSTranslationArray())[start_frame+frame_index]);
	}

	MultiFrameMesh->BoneRoot->CalcRelativeFromAbsolute(1);
	// MultiFrameMesh->Apply_Skin(1); todo*** need it?
	// MultiFrameMesh->...
}


// todo*** update limits accordingly to mesh?? does it works?

void MultiFrameSkinnedMesh::RetrieveFrames(int start_frame) {
	// todo** MultiFrameMesh absolute should be ready 
	int frame_index;

	for(int j=0;j<MultiFrameMesh->num_bones;j++) {
		// Get the original bone from original mesh
		const char *ext_bone_name=MultiFrameMesh->BoneRefTable[j]->GetName();
		sscanf(ext_bone_name+strlen("Frame"),"%d",frame_index);
		const char *original_bone_name=ext_bone_name+strlen("Frame0000_");
		Bone *original_bone=mesh->BoneRoot->SearchBone(original_bone_name);

		// Copy MultiFrameMesh->BoneRefTable[j] (frame 1) in original bone (frame start_frame+frame_index)
		((original_bone->GetABSRotationArray())[start_frame+frame_index])->Set((MultiFrameMesh->BoneRefTable[j]->GetABSRotationArray())[1]);
		((original_bone->GetABSTranslationArray())[start_frame+frame_index])=(MultiFrameMesh->BoneRefTable[j]->GetABSTranslationArray())[1];
	}

	for(int i=0;i<window_size;i++) {
		mesh->BoneRoot->CalcRelativeFromAbsolute(start_frame+i);	
	}
	// mesh->Apply_Skin(1); todo*** need it? ... etc ... 
}


















Time_Articulated_ICP::Time_Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
                                           Features **FeaturesList,View *Views,int num_views,
                                           int window_size,
                                           Degrees_of_Freedom_Table_Entry *Constraint_level0,
                                           Degrees_of_Freedom_Table_Entry *Constraint_level1,
                                           int debug_level) 
                                           : Articulated_ICP(InitConstructor(mesh,window_size),
                                                             RoomCenter,RoomRadius,
                                                             FeaturesList,Views,num_views,
                                                             Constraint_level0,Constraint_level1,
                                                             debug_level)
{
	this->time_window_size=window_size;
}

Time_Articulated_ICP::~Time_Articulated_ICP() {
	SDELETE(MF_Mesh);
}

Skinned_Mesh *Time_Articulated_ICP::InitConstructor(Skinned_Mesh *mesh,int window_size) {
	MF_Mesh=new MultiFrameSkinnedMesh(mesh,window_size);
	return MF_Mesh->GetMesh();
}


void Time_Articulated_ICP::RetrieveObservations(int Level,int frame_index) {
//	// Delete list
//	TargetList_1D->clear();
//	TargetList_2D->clear();
//	TargetList_3D->clear();
//	
//
//	// Get Observations
//	if (show_debug_information) cout<<"-> Correspondences\n";
//	//TargetList_2D->append(Feat_TargetList); // TODO optical flow
//
//	/*
//	
//	for(int i=0;i<num_views;i++) {
//		ResetBufferStatus();
//		
//		GetSilhouettesCorrespondences(i,Level);
//		GetEdgesCorrespondences(i,Level);
//		GetDepthCorrespondences(i,Level);
//		GetSalientPointCorrespondences(i,Level,frame_index);
//		//GetTextureCorrespondences(i,Level);
//	}
//
//	// Computa collisioni (skin is ready from the previous calls) (do not do anything if they are not enabled)
//	// NOTE: IK_Solver->SetCollisionParameters -> can be called somewhere before this next calls to set the new weights or other parameters
//	//       or UpdateCollisionWeights
//	if (Level==0) {
//		IK_Solver->Clear_Collision_Constraints();
//		IK_Solver->Add_Collision_Constraints();
//	} else {
//		IK_Solver_Pyramid_Level_1->Clear_Collision_Constraints();
//		IK_Solver_Pyramid_Level_1->Add_Collision_Constraints();
//	}
//
//
//	if (show_minimal_debug_information) ShowCorrespondencesInfo(Level);
//
//*/
//
//
//	// TODO** here do it for all the meshes
//	
//	// Get current frame
//	int current_frame=View[]...
//
//	for each time  {
//		for each view {
//			load current cameras
//		}
//	
//		old::RetrieveObservations (do not delete old)
//	
//	}
//
//	// add the motion contrains observations scalar per vertex?
//
//
//
	

}

