//
// GammaLib: Computer Vision library
//
//    Copyright (C) 1998-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
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
using namespace std;


Skinned_Mesh::Skinned_Mesh() : HyperMesh<3>() {
	Pose_Points=NULL;
	BoneRoot=NULL;
	BoneRefTable=NULL;
	Weigth=NULL;
	num_bones=0;
}
Skinned_Mesh::~Skinned_Mesh() {
	SDELETE(Pose_Points);
	SDELETE(BoneRoot);
	SDELETEA(BoneRefTable);
	SDELETEA_RECA(Weigth,num_p);
}
Skinned_Mesh::Skinned_Mesh(Skinned_Mesh &original)  {
	ErrorExit("Operatore di copia tra Skinned_Mesh implementato solo nella forma a puntatore");
}
Skinned_Mesh::Skinned_Mesh(Skinned_Mesh *original) : HyperMesh<3>(original) {
	Pose_Points=NULL;
	BoneRoot=NULL;
	BoneRefTable=NULL;
	Weigth=NULL;
	num_bones=original->num_bones;
	
	//BoneRoot
	BoneRoot=Bone::CloneBoneTree(original->BoneRoot,NULL);

	//BoneRefTable
	BoneRefTable=new Bone*[num_bones];
	for(int j=0;j<num_bones;j++) 
		BoneRefTable[j]=BoneRoot->SearchBone(original->BoneRefTable[j]->GetName());
	
	//Weigth
	Weigth=new double*[num_p];
	for(int i=0;i<num_p;i++) {
		Weigth[i]=new double[num_bones];
		for(int j=0;j<num_bones;j++) Weigth[i][j]=original->Weigth[i][j];
	}
	
	//Pose_Points
	Pose_Points=new Array<PointType>(num_p);
	for(int i=0;i<num_p;i++) Pose_Points->append((*(original->Pose_Points))[i]);
}





















void Skinned_Mesh::AddMesh(Skinned_Mesh *input,int force_n_frames) {
	int original_num_p=num_p;
	HyperMesh<3>::AddMesh((HyperMesh<3>*)input);


	//Pose_Points
	SNEW(Pose_Points,Array<PointType>(num_p));
	if (!(input->Pose_Points)) ErrorExit("Cannot add an empty mesh");

	int         first_pose_point   =Pose_Points->numElements();
	int         numInputPosePoint  =input->Pose_Points->numElements();
	Vector<3>   *InputPosePoint    =input->Pose_Points->getMem();
	
	if (original_num_p!=first_pose_point) ErrorExit("Inconsistend skinned mesh");

	for(int i=0;i<numInputPosePoint;i++,InputPosePoint++) Pose_Points->append(*InputPosePoint);
	

	//BoneRoot
	int tot_num_of_frames=0;
	Bone *origial_boneroot=NULL;
	if (force_n_frames==0) {
		if (BoneRoot) {
			origial_boneroot=BoneRoot;
			tot_num_of_frames=BoneRoot->getNumberOfFrames();
		
			int tmp=input->BoneRoot->getNumberOfFrames();
			if (tmp>tot_num_of_frames) {
				tot_num_of_frames=tmp;
				origial_boneroot=Bone::CloneBoneTree(BoneRoot,NULL,tot_num_of_frames);
				SDELETE(BoneRoot);
			}
		} else {
			tot_num_of_frames=input->BoneRoot->getNumberOfFrames();
		}
	} else {
		tot_num_of_frames=force_n_frames;
		if (BoneRoot) {
			origial_boneroot=BoneRoot;
		
			if (tot_num_of_frames!=BoneRoot->getNumberOfFrames()) {
				origial_boneroot=Bone::CloneBoneTree(BoneRoot,NULL,tot_num_of_frames);
				SDELETE(BoneRoot);
			}
		}
	}
	
	Bone *input_boneroot=Bone::CloneBoneTree(input->BoneRoot,NULL,tot_num_of_frames);
	
	if (origial_boneroot) {
		bool result=false;
		
		if (str_begin(origial_boneroot->GetName(),"Root_NULL_Bone_")) 
			result=origial_boneroot->AddChild(input_boneroot);

		if (result) {
			BoneRoot=origial_boneroot;
		} else {
			int root_null_bone_index=0;
			if (str_begin(origial_boneroot->GetName(),"Root_NULL_Bone_")) {
				sscanf(origial_boneroot->GetName()+strlen("Root_NULL_Bone_"),"%d",&root_null_bone_index);
				root_null_bone_index++;
			}
			char TMP_BONE_NAME[BONE_MAX_NAME_LEN+1];
			sprintf(TMP_BONE_NAME,"Root_NULL_Bone_%i",root_null_bone_index);
			Bone *new_BoneRoot=NULL;
			SNEW(new_BoneRoot,Bone(TMP_BONE_NAME,NULL,1.0,tot_num_of_frames));
			new_BoneRoot->LockX=true;
			new_BoneRoot->LockY=true;
			new_BoneRoot->LockZ=true;
			new_BoneRoot->LockRX=true;
			new_BoneRoot->LockRY=true;
			new_BoneRoot->LockRZ=true;
			new_BoneRoot->MinR[0]=new_BoneRoot->MinR[1]=new_BoneRoot->MinR[2]=0.0;
			new_BoneRoot->MaxR[0]=new_BoneRoot->MaxR[1]=new_BoneRoot->MaxR[2]=0.0;
			
			new_BoneRoot->AddChild(origial_boneroot);
			new_BoneRoot->AddChild(input_boneroot);
			BoneRoot=new_BoneRoot;
		}
	} else {
		BoneRoot=input_boneroot;
	}
	BoneRoot->UpdateTreeLevels();

	//num_bones
	int original_num_bones=num_bones;
	num_bones=num_bones+input->num_bones;


	//BoneRefTable
	Bone **new_BoneRefTable=NULL;
	SNEWA(new_BoneRefTable,Bone*,num_bones);
	int j=0;
	for(;j<original_num_bones;j++)
		new_BoneRefTable[j]=BoneRoot->SearchBone(BoneRefTable[j]->GetName());
	for(;j<num_bones;j++)
		new_BoneRefTable[j]=BoneRoot->SearchBone(input->BoneRefTable[j-original_num_bones]->GetName());
	
	SDELETEA(BoneRefTable);
	BoneRefTable=new_BoneRefTable;

	
	//Weigth
	double **new_Weigth=NULL;
	SNEWA_P(new_Weigth,double*,num_p);
	for(int i=0;i<num_p;i++) {
		SNEWA((new_Weigth[i]),double,num_bones);
		if (i<original_num_p) {
			for(j=0;j<original_num_bones;j++) new_Weigth[i][j]=Weigth[i][j];
			for(;j<num_bones;j++)             new_Weigth[i][j]=0.0;
		} else {
			for(j=0;j<original_num_bones;j++) new_Weigth[i][j]=0.0;
			for(;j<num_bones;j++)             new_Weigth[i][j]=input->Weigth[i-original_num_p][j-original_num_bones];
		}
	}
	SDELETEA_RECA(Weigth,original_num_p);
	Weigth=new_Weigth;
}
































void Skinned_Mesh::Read(char *BaseFileName) {
	char MeshFileName[300];
	char BoneStructureFileName[300];
	char WeigthFileName[300];
	char MotionFileName[300];
	char PoseFileName[300];
	char AdjustFileName[300];
	
	strcpy(MeshFileName,BaseFileName);
	strcpy(BoneStructureFileName,BaseFileName);
	strcpy(WeigthFileName,BaseFileName);
	strcpy(MotionFileName,BaseFileName);
	strcpy(PoseFileName,BaseFileName);
	strcpy(AdjustFileName,BaseFileName);
	strcat(MeshFileName,".objbin");
	strcat(BoneStructureFileName,".skel");
	strcat(WeigthFileName,".skin");
	strcat(MotionFileName,".motion");
	strcat(PoseFileName,".pose");
	strcat(AdjustFileName,".adj");

	// Adjust (If it exists)
	bool valid_adj;
	double adj_scale_factor;
	int adj_num_of_frames,local_num_of_frames;
	IFileBuffer AdjFile(AdjustFileName);
	if (!AdjFile.IsFinished()) {
		valid_adj=true;
		AdjFile>>adj_scale_factor;
		AdjFile>>adj_num_of_frames;
	} else {
		valid_adj=false;
		adj_num_of_frames=0;
		adj_scale_factor=1.0;
	}


	// Mesh
	int first_point=num_p;
	HyperMesh<3>::Read(MeshFileName);
	if (num_p-first_point==0) return;
	if (!Pose_Points) Pose_Points=new Array<PointType>(num_p);
	for(int i=first_point;i<num_p;i++) Pose_Points->append(Points[i]);
		
	// number of frames
	local_num_of_frames=adj_num_of_frames;
	if (BoneRoot) adj_num_of_frames=BoneRoot->getNumberOfFrames();

	// Skeletron
	IFileBuffer BoneFile(BoneStructureFileName);
	Bone *loaded_skull;
	if (local_num_of_frames==0) loaded_skull=Bone::LoadSkull(&BoneFile,adj_num_of_frames,&local_num_of_frames);
	else loaded_skull=Bone::LoadSkull(&BoneFile,adj_num_of_frames);
	bool bones_table_ready=false;
	if (!BoneRoot) {
		BoneRoot=loaded_skull;
	} else {
		Array<Bone*> *BonesLoaded=loaded_skull->Enumerate();
		bool all_bones_existed=true;
		bool all_bones_did_not_exist=true;
		for(int i=0;i<BonesLoaded->numElements();i++) {
			if (BoneRoot->SearchBone((*BonesLoaded)[i]->GetName())==NULL) all_bones_existed=false;
			else all_bones_did_not_exist=false;
		}
		delete BonesLoaded;

		if ((!all_bones_did_not_exist) && (!all_bones_existed)) {
			cout<<"Rig names does not overlap completely: can't handle this situation.";
			exit(1);
		}

		if (all_bones_did_not_exist) {
			char TMP_BONE_NAME[BONE_MAX_NAME_LEN+1];
			sprintf(TMP_BONE_NAME,"Root_%i",first_point);
			Bone *new_BoneRoot=new Bone(TMP_BONE_NAME,NULL,1.0,BoneRoot->getNumberOfFrames());
			new_BoneRoot->LockX=true;
			new_BoneRoot->LockY=true;
			new_BoneRoot->LockZ=true;
			new_BoneRoot->LockRX=true;
			new_BoneRoot->LockRY=true;
			new_BoneRoot->LockRZ=true;
			new_BoneRoot->MinR[0]=new_BoneRoot->MinR[1]=new_BoneRoot->MinR[2]=0.0;
			new_BoneRoot->MaxR[0]=new_BoneRoot->MaxR[1]=new_BoneRoot->MaxR[2]=0.0;
			
			new_BoneRoot->AddChild(BoneRoot);
			new_BoneRoot->AddChild(loaded_skull);
			BoneRoot=new_BoneRoot;
			BoneRoot->UpdateTreeLevels();
			bones_table_ready=false;
		} else {
			bones_table_ready=true;
		}
	}

	
	// Skin
	int len;
	float TmpFloat;
	char TmpName[200];
	int num_old_bones,num_new_bones;
	int *RemapBone=NULL;

	IFileBuffer WeigthFile(WeigthFileName);
	len=4;WeigthFile.ReadLine((char*)&num_new_bones,len);
	num_old_bones=num_bones;
	if (!bones_table_ready) num_bones+=num_new_bones;
	else RemapBone=new int[num_new_bones];

	// Creo BoneRefTable
	if (BoneRefTable) {
		if (!bones_table_ready) {
			Bone **new_BoneRefTable=new Bone*[num_bones];
			for(int j=0;j<num_old_bones;j++) new_BoneRefTable[j]=BoneRefTable[j];
			delete []BoneRefTable;
			BoneRefTable=new_BoneRefTable;
		}
	} else BoneRefTable=new Bone*[num_bones];
	
	if (!bones_table_ready) {
		for(int j=num_old_bones;j<num_bones;j++) {
			len=199;WeigthFile.ReadPharse(TmpName,len);
			BoneRefTable[j]=BoneRoot->SearchBone(TmpName);
		}
	} else {
		for(int j=0;j<num_new_bones;j++) {
			len=199;WeigthFile.ReadPharse(TmpName,len);
			Bone *tmp=BoneRoot->SearchBone(TmpName);
			int q=0;
			for(;q<num_bones;q++) 
				if (BoneRefTable[q]==tmp) break;
			if (q==num_bones) {
				cout<<"Irreversible error in adding a mesh"<<endl;
				exit(1);
			}
			RemapBone[j]=q;
		}
	}


	// Creo Weigth
	if (Weigth) {
		if (!bones_table_ready) {
			double **new_Weigth=new double*[num_p];
			for(int i=0;i<first_point;i++) {
				new_Weigth[i]=new double[num_bones];
				for(int j=0;j<num_old_bones;j++) new_Weigth[i][j]=Weigth[i][j];
				for(int j=num_old_bones;j<num_bones;j++) new_Weigth[i][j]=0.0;
				delete [](Weigth[i]);
			}
			delete []Weigth;
			Weigth=new_Weigth;
			for(int i=first_point;i<num_p;i++) {
				Weigth[i]=new double[num_bones];
				for(int j=0;j<num_old_bones;j++) Weigth[i][j]=0.0;
				for(int j=num_old_bones;j<num_bones;j++) {
					len=4;WeigthFile.ReadLine((char*)&TmpFloat,len);
					Weigth[i][j]=TmpFloat;
				}
			}
		} else {
			double **new_Weigth=new double*[num_p];
			for(int i=0;i<first_point;i++) {
				new_Weigth[i]=new double[num_bones];
				for(int j=0;j<num_bones;j++) new_Weigth[i][j]=Weigth[i][j];
				delete [](Weigth[i]);
			}
			delete []Weigth;
			Weigth=new_Weigth;
			for(int i=first_point;i<num_p;i++) {
				Weigth[i]=new double[num_bones];
				for(int j=0;j<num_bones;j++) Weigth[i][j]=0.0;
				for(int j=0;j<num_new_bones;j++) {
					len=4;WeigthFile.ReadLine((char*)&TmpFloat,len);
					Weigth[i][RemapBone[j]]=TmpFloat;
				}
			}
		}
	} else {
		Weigth=new double*[num_p];
		for(int i=0;i<num_p;i++) {
			Weigth[i]=new double[num_bones];
			for(int j=0;j<num_bones;j++) {
				len=4;WeigthFile.ReadLine((char*)&TmpFloat,len);
				Weigth[i][j]=TmpFloat;
			}
		}
	}
	SDELETEA(RemapBone);
	// TODO: fin qui mi fermo con le modifiche... modificare da qui in poi....


	// Motion
	IFileBuffer MotionFile(MotionFileName);
	Bone::LoadSkullMotion(&MotionFile,BoneRoot,local_num_of_frames);

	// Pose (If it exists)
	IFileBuffer PoseFile(PoseFileName);
	if (!PoseFile.IsFinished()) Bone::LoadSkullPose(&PoseFile,BoneRoot);

	// Set Up Frame the Pose Frame
	BoneRoot->CalcRelativeFromAbsolute(0);

	// Adjust 
	if (valid_adj) ScalePose(adj_scale_factor);
}









void Skinned_Mesh::Save(char *BaseFileName) {
	if ((!BoneRoot) || (!BoneRefTable) || (!Weigth)) ErrorExit("Cannot save an empty skinned mesh.");

	char MeshFileName[300];
	char BoneStructureFileName[300];
	char WeigthFileName[300];
	char MotionFileName[300];
	
	strcpy(MeshFileName,BaseFileName);
	strcpy(BoneStructureFileName,BaseFileName);
	strcpy(WeigthFileName,BaseFileName);
	strcpy(MotionFileName,BaseFileName);
	strcat(MeshFileName,".objbin");
	strcat(BoneStructureFileName,".skel");
	strcat(WeigthFileName,".skin");
	strcat(MotionFileName,".motion");


	// Mesh (reset to base pose)
	Vector<3> *pose_points=Pose_Points->getMem();
	Vector<3> *points=Points.getMem();
	for(int i=0;i<num_p;i++,points++,pose_points++) {
		(*points)=(*pose_points);
	}
	HyperMesh<3>::Save(MeshFileName);


	// Skeletron
	OFileBuffer BoneFile(BoneStructureFileName);
	Bone::SaveSkull(&BoneFile,BoneRoot);
	BoneFile.Close();

	// Skin
	OFileBuffer WeigthFile(WeigthFileName);
	WeigthFile.WriteLine((char*)&num_bones,4);
	for(int j=0;j<num_bones;j++) {
		const char *bone_name=BoneRefTable[j]->GetName();
		WeigthFile.WriteLine(bone_name,(int)strlen(bone_name));
		WeigthFile.WriteLine("\n",1);
	}
	float tmp_float;
	for(int i=0;i<num_p;i++) {
		for(int j=0;j<num_bones;j++) {
			tmp_float=(float)(Weigth[i][j]);
			WeigthFile.WriteLine((char*)&tmp_float,4);
		}
	}
	WeigthFile.Close();


	// Motion
	OFileBuffer MotionFile(MotionFileName);
	Bone::SaveSkullMotion(&MotionFile,BoneRoot);
	MotionFile.Close();

}





































void find_k_nearest_points(Vector<3> *p,Vector<3> *array_points,int num_p,int k,int *lp) {
	Array<int>       list_point(k+1);
	Array<double>    list_point_distance(k+1);
	
	for(int j=0;j<k;j++) {
		double d=(array_points[j]-(*p)).Norm2();
		int inserted=list_point_distance.append_sorted(d,(int (*)(double *a,double *b))compare_double);
		if (!list_point.insertBefore(inserted,j)) list_point.append(j);
	}

	for(int j=0;j<num_p;j++) {
		double d=(array_points[j]-(*p)).Norm2();
		int inserted=list_point_distance.append_sorted(d,(int (*)(double *a,double *b))compare_double);
		
		if (inserted>=k) {
			list_point_distance.del(inserted);
		} else {
			list_point.insertBefore(inserted,j);
			list_point.del(k);
		}
	}

	for(int j=0;j<k;j++) lp[j]=list_point[j];
}


double **Skinned_Mesh::transfer_skin(HyperMesh<3> *model) {
	
	double **W=new double*[model->num_p];
	for(int i=0;i<model->num_p;i++) W[i]=new double[num_bones];

	int kNN=3;
	int *lp=new int[kNN];

	for(int i=0;i<model->num_p;i++) {
		Vector<3> p=model->Points[i];

		find_k_nearest_points(&p,model->Points.getMem(),num_p,kNN,lp);
	
		for(int q=0;q<num_bones;q++) {
			W[i][q]=0;
			for(int j=0;j<kNN;j++) {
				W[i][q]+=Weigth[lp[j]][q];
			}
			W[i][q]/=kNN;
		}

	}
	delete []lp;
	return W;
}




void Skinned_Mesh::Apply_Skin(int FrameIndex) {
	
	{
		Vector<3> *points=Points.getMem();
		for(int i=0;i<num_p;i++,points++) {
			(*points)[0]=0.0;
			(*points)[1]=0.0;
			(*points)[2]=0.0;
		}
	}
	
	Matrix P1(4,4);
	Matrix P2(4,4);
	Matrix P3(4,4);
	Vector<4> PP,OP;
	
	// NOTA: la rotazione delle normali nn e' possibile con una semplice moltiplicazione.
	// Devon essere moltiplicate per M^(-T) che e' diverso da M, xche' M nn e' ortogonale.

	for(int j=0;j<num_bones;j++) {
		RTtoP(&P1,BoneRefTable[j]->TrasformationMatrix[0],&(BoneRefTable[j]->TrasformationT[0]));
		RTtoP(&P2,BoneRefTable[j]->TrasformationMatrix[FrameIndex],&(BoneRefTable[j]->TrasformationT[FrameIndex]));
		P1.Inversion();
		Multiply(&P2,&P1,&P3);

		Vector<3> *pose_points=Pose_Points->getMem();
		Vector<3> *points=Points.getMem();

		for(int i=0;i<num_p;i++,points++,pose_points++) {
			double tmpW=Weigth[i][j];
			if (tmpW==0.0) continue;
			
			PP[0]=(*pose_points)[0];
			PP[1]=(*pose_points)[1];
			PP[2]=(*pose_points)[2];
			PP[3]=1.0;
			Multiply(&P3,PP,OP.GetList());
			
			(*points)[0]+=tmpW*OP[0];
			(*points)[1]+=tmpW*OP[1];
			(*points)[2]+=tmpW*OP[2];
		}
	}
}




























void Skinned_Mesh::MovePose(Vector<3> T) {
	BoneRoot->TrasformationT[0]+=T;
	BoneRoot->CalcRelativeFromAbsolute(0);
	for(int i=0;i<num_p;i++) (*Pose_Points)[i]+=T;
}
void Skinned_Mesh::ScalePose(double factor) {
	if (factor==1.0) return;
	factor=1.0/factor;

	Vector<3> Baricentro=BoneRoot->TrasformationT[0];
	BoneRoot->TrasformationT[0].SetZero();
	BoneRoot->RelativeTrasformationT[0].SetZero();
	BoneRoot->CalcAbsoluteFromRelative(0);

	for(int j=0;j<num_bones;j++) {
		if (BoneRefTable[j]!=BoneRoot) BoneRefTable[j]->TrasformationT[0]*=factor;
		BoneRefTable[j]->length*=factor;
	}
	BoneRoot->CalcRelativeFromAbsolute(0);
	BoneRoot->TrasformationT[0]=Baricentro;
	BoneRoot->RelativeTrasformationT[0]=Baricentro;
	BoneRoot->CalcAbsoluteFromRelative(0);

	for(int i=0;i<num_p;i++) (*Pose_Points)[i]=(((*Pose_Points)[i]-Baricentro)*factor)+Baricentro;
}

void Skinned_Mesh::RotatePose(Vector<3> Axis) {
	BoneRoot->CalcRelativeFromAbsolute(0);
	BoneRoot->Rotate(0,Axis);
	BoneRoot->CalcAbsoluteFromRelative(0);

	Matrix R(3,3);
	double angle=Axis.Norm2();
	if (IS_ZERO(angle,DBL_EPSILON)) return;
	
	AngleAxis2RotationMatrix(&R,Axis);
	Vector<3> x;
	for(int i=0;i<num_p;i++) {
		Multiply(&R,(*Pose_Points)[i],x.GetList());
		(*Pose_Points)[i]=x;
	}
}

void Skinned_Mesh::Move(Vector<3> T,int frame_index) {
	if (frame_index==0) MovePose(T);
	
	BoneRoot->RelativeTrasformationT[frame_index]+=T;
	BoneRoot->TrasformationT[frame_index]+=T;
	BoneRoot->CalcAbsoluteFromRelative(frame_index);
}

double *Skinned_Mesh::CreateConstantEnlargeFactors(double value) {
	double *Enlarge_Factors=new double[num_bones];
	for(int index_bone=0;index_bone<num_bones;index_bone++) Enlarge_Factors[index_bone]=value;

	return Enlarge_Factors;
}

void Skinned_Mesh::CustomEnlargeFactors(double *EnlargeFactors,char *BoneName,double factor) {
	int Bone_ID=GetBoneID(BoneName);
	if (Bone_ID<0) return;
	EnlargeFactors[Bone_ID]=factor;
}
void Skinned_Mesh::Enlarge(double *EnlargeFactors) {
	Array<Vector<3>> New_Points(num_p);
	for(int index_v=0;index_v<num_p;index_v++) New_Points[index_v].SetZero();
	
	for(int index_bone=0;index_bone<num_bones;index_bone++) {
		Matrix curr_T_Matrix(4,4),curr_T_Matrix_inv(4,4);
		Bone *curr_bone=BoneRefTable[index_bone];
		curr_bone->GetABSTrasformMatrix(0,&curr_T_Matrix);
		curr_bone->GetABSTrasformMatrix(0,&curr_T_Matrix_inv);
		curr_T_Matrix_inv.Inversion();

		for(int index_v=0;index_v<num_p;index_v++) {
			double curr_w=Weigth[index_v][index_bone];
			if (curr_w==0) continue;

			Vector<3> curr_point=(*Pose_Points)[index_v];

			Vector<4> tmp,trasf_point;
			tmp[0]=curr_point[0];
			tmp[1]=curr_point[1];
			tmp[2]=curr_point[2];
			tmp[3]=1.0;
			Multiply(&curr_T_Matrix_inv,tmp,trasf_point.GetList());
			Vector<2> Polar_coors;
			Polar_coors[0]=trasf_point[1];
			Polar_coors[1]=trasf_point[2];
			
			Polar_coors=EnlargeFactors[index_bone]*Polar_coors;

			trasf_point[1]=Polar_coors[0];
			trasf_point[2]=Polar_coors[1];
			Multiply(&curr_T_Matrix,trasf_point,tmp.GetList());

			New_Points[index_v][0]+=curr_w*tmp[0];
			New_Points[index_v][1]+=curr_w*tmp[1];
			New_Points[index_v][2]+=curr_w*tmp[2];
		}
	}
	for(int index_v=0;index_v<num_p;index_v++) (*Pose_Points)[index_v]=New_Points[index_v];
}

void Skinned_Mesh::Rotate(Vector<3> angle_axis,int frame_index) {
	if (frame_index==0) RotatePose(angle_axis);
	
	BoneRoot->CalcRelativeFromAbsolute(frame_index);
	BoneRoot->Rotate(frame_index,angle_axis);
	BoneRoot->CalcAbsoluteFromRelative(frame_index);
}

void Skinned_Mesh::Scale(double factor,int frame_index) {
	ScalePose(factor);
	BoneRoot->Copy(0,frame_index);
}


Vector<3> Skinned_Mesh::GetPointRelativeCoordinates(int point_index,int bone_index) {
	Matrix P(4,4);
	BoneRefTable[bone_index]->GetABSTrasformMatrix(0,&P);
	P.Inversion();

	Vector<4> PP,OP;
	PP[0]=(*Pose_Points)[point_index][0];
	PP[1]=(*Pose_Points)[point_index][1];
	PP[2]=(*Pose_Points)[point_index][2];
	PP[3]=1.0;
	Multiply(&P,PP,OP.GetList());

	Vector<3> Ret;
	Ret[0]=OP[0];
	Ret[1]=OP[1];
	Ret[2]=OP[2];
	return Ret;
}

int Skinned_Mesh::GetBoneID(Bone *bone) {
	for(int m=0;m<num_bones;m++) {
		if (BoneRefTable[m]==bone) return m;
	}
	return -1;
}
int Skinned_Mesh::GetBoneID(char *Name) {
	int len=(int)strlen(Name);
	
	for(int index_bone=0;index_bone<num_bones;index_bone++) {
		const char *curr_name=BoneRefTable[index_bone]->GetName();
		int lenName=(int)strlen(curr_name);
		if (lenName>=len) {
			curr_name+=lenName-len;

			if (_stricmp(curr_name,Name)==0) return index_bone;
		}
	}
	return -1;
}

int Skinned_Mesh::getNumberOfFrames() {
	return BoneRoot->getNumberOfFrames();
}

int Skinned_Mesh::GetBoneIDAttachedtoVertex(int point_index,double att_th) {
	int bone_=0;
	double max=Weigth[point_index][0];
	
	for(int k=1;k<num_bones;k++) {
		if (Weigth[point_index][k]>max) {
			max=Weigth[point_index][k];
			bone_=k;
		}
	}

	if (max<=att_th) return -1;
	return bone_;
}
































void parse_escape_char(OFileBuffer *file,float min,float max,bool &return_done) {
	if ((min==-FLT_MAX) && (max==FLT_MAX)) {
		(*file)<<"*";
		return_done=false;
	} else {
		if (!return_done) (*file)<<"\r\n";
		(*file)<<min<<" "<<max<<"\r\n";
		return_done=true;
	}
}
void Degrees_of_Freedom_Table_Entry::Save(char *FileName,Degrees_of_Freedom_Table_Entry *Table) {
	OFileBuffer file(FileName);
	file<<"#v2.0\r\n";

	int i=0;
	while (true) {
		if (Table[i].Name==NULL) break;
		file<<Table[i].Name<<"\r\n";
		file<<((int)Table[i].Translable_X)<<" ";
		file<<((int)Table[i].Translable_Y)<<" ";
		file<<((int)Table[i].Translable_Z)<<" ";
		file<<((int)Table[i].Rotable_X)<<" ";
		file<<((int)Table[i].Rotable_Y)<<" ";
		file<<((int)Table[i].Rotable_Z)<<"\r\n";

		bool return_done=true;
		parse_escape_char(&file,Table[i].LimitsT.MinX,Table[i].LimitsT.MaxX,return_done);
		parse_escape_char(&file,Table[i].LimitsT.MinY,Table[i].LimitsT.MaxY,return_done);
		parse_escape_char(&file,Table[i].LimitsT.MinZ,Table[i].LimitsT.MaxZ,return_done);

		parse_escape_char(&file,Table[i].LimitsR.MinX,Table[i].LimitsR.MaxX,return_done);
		parse_escape_char(&file,Table[i].LimitsR.MinY,Table[i].LimitsR.MaxY,return_done);
		parse_escape_char(&file,Table[i].LimitsR.MinZ,Table[i].LimitsR.MaxZ,return_done);
		if (!return_done) file<<"\r\n";
		i++;		
	}
	file<<"NULL\r\n";
	file.Close();
}

Degrees_of_Freedom_Table_Entry *Degrees_of_Freedom_Table_Entry::Copy(Degrees_of_Freedom_Table_Entry *Table) {
	Degrees_of_Freedom_Table_Entry *NewTable=new Degrees_of_Freedom_Table_Entry[100];

	int i=0;
	while (true) {
		if (Table[i].Name==NULL) break;
		NewTable[i]=Table[i];
		NewTable[i].Name=strcln(Table[i].Name);
		i++;
	}

	NewTable[i].Name=NULL;
	return NewTable;
}

void parse_escape_char(IFileBuffer *file,float &min,float &max,bool relative,Skinned_Mesh *Model,int frame__ref,char *table_name,Degrees_of_Freedom_Table_Entry *Table,int dof_index) {
	char tmp[200];
	file->set_wordwidth(198);
	file->FlushToTheBeginningOfNextWord();
	(*file)>>tmp;

	if (tmp[0]=='*') {
		min=-FLT_MAX;
		max=FLT_MAX;
		file->back();
		file->SetPosition(1+file->GetPosition());
	} else {
		file->back();
		(*file)>>min;
		(*file)>>max;
	}
	if (relative) {
		Model->BoneRoot->CalcRelativeFromAbsolute(frame__ref);
		for(int i=0;i<Model->num_bones;i++) {
			Bone *Curr=Model->BoneRefTable[i];
			const char *Name=Curr->GetName();
			Degrees_of_Freedom_Table_Entry *F=Degrees_of_Freedom_Table_Entry::SearchDoFT(Name,Table);
			if ((F!=NULL) && (!strcmp(table_name,F->Name))) {
				Vector<3> R,T;
				Curr->GetRelativeAngleAndTranslation(frame__ref,&R,&T);
				if (dof_index<3) {
					max+=(float)T[dof_index];
					min+=(float)T[dof_index];
				} else {
					max+=(float)R[dof_index-3];
					min+=(float)R[dof_index-3];
				}
				break;
			}
		}
	}
}

bool parse_escape_char(IFileBuffer *file,bool &var) {
	char tmp[200];
	file->set_wordwidth(198);
	(*file)>>tmp;

	if ((tmp[0]=='R') || (tmp[0]=='r')) {
		var=true;
		return true;
	} else {
		file->back();
		(*file)>>var;
		return false;
	}
}

Degrees_of_Freedom_Table_Entry *Degrees_of_Freedom_Table_Entry::Load(char *FileName,Skinned_Mesh *Model,int frame__ref) {
	IFileBuffer file(FileName);
	if (file.IsFinished()) ErrorExit_str("Cannot read %s",FileName);
	Degrees_of_Freedom_Table_Entry *Table=new Degrees_of_Freedom_Table_Entry[100];
	
	int i=0;
	int len;
	char tmp[200];
	bool once_relative=false;
	
	long t_pos=file.GetPosition();
	len=199;file.ReadPharse(tmp,len);
	if (!strcmp(tmp,"#v2.0")) {
		while(!file.IsFinished()) {
			file.set_wordwidth(198);
			file.set_word_separators("\t\r\n");
			file.FlushToTheBeginningOfNextWord();
			file>>tmp;
			file.set_word_separators();
			if (!strcmp(tmp,"NULL")) break;
			Table[i].Name=strcln(tmp);
			Table[i+1].Name=NULL;
			bool RX=false,RY=false,RZ=false;
			bool TX=false,TY=false,TZ=false;

			TX=parse_escape_char(&file,Table[i].Translable_X);
			TY=parse_escape_char(&file,Table[i].Translable_Y);
			TZ=parse_escape_char(&file,Table[i].Translable_Z);
			RX=parse_escape_char(&file,Table[i].Rotable_X);
			RY=parse_escape_char(&file,Table[i].Rotable_Y);
			RZ=parse_escape_char(&file,Table[i].Rotable_Z);

			parse_escape_char(&file,Table[i].LimitsT.MinX,Table[i].LimitsT.MaxX,TX,Model,frame__ref,Table[i].Name,Table,0);
			parse_escape_char(&file,Table[i].LimitsT.MinY,Table[i].LimitsT.MaxY,TY,Model,frame__ref,Table[i].Name,Table,1);
			parse_escape_char(&file,Table[i].LimitsT.MinZ,Table[i].LimitsT.MaxZ,TZ,Model,frame__ref,Table[i].Name,Table,2);

			parse_escape_char(&file,Table[i].LimitsR.MinX,Table[i].LimitsR.MaxX,RX,Model,frame__ref,Table[i].Name,Table,3);
			parse_escape_char(&file,Table[i].LimitsR.MinY,Table[i].LimitsR.MaxY,RY,Model,frame__ref,Table[i].Name,Table,4);
			parse_escape_char(&file,Table[i].LimitsR.MinZ,Table[i].LimitsR.MaxZ,RZ,Model,frame__ref,Table[i].Name,Table,5);

			if (TX || TY || TZ || RX || RY || RZ) once_relative=true;
			i++;
		}
	} else {
		file.SetPosition(t_pos);
		while(!file.IsFinished()) {
			len=199;file.ReadPharse(tmp,len);
			if (!strcmp(tmp,"NULL")) break;
			Table[i].Name=strcln(tmp);
			file>>Table[i].Translable_X;
			Table[i].Translable_Z=Table[i].Translable_Y=Table[i].Translable_X;
			file>>Table[i].Rotable_X;
			file>>Table[i].Rotable_Y;
			file>>Table[i].Rotable_Z;
			file>>Table[i].LimitsR.MinX;
			file>>Table[i].LimitsR.MaxX;
			file>>Table[i].LimitsR.MinY;
			file>>Table[i].LimitsR.MaxY;
			file>>Table[i].LimitsR.MinZ;
			file>>Table[i].LimitsR.MaxZ;


			Table[i].LimitsT.MinX=-FLT_MAX;
			Table[i].LimitsT.MinY=-FLT_MAX;
			Table[i].LimitsT.MinZ=-FLT_MAX;
			Table[i].LimitsT.MaxX=FLT_MAX;
			Table[i].LimitsT.MaxY=FLT_MAX;
			Table[i].LimitsT.MaxZ=FLT_MAX;

			i++;
		}
	}

	Table[i].Name=NULL;
	if (once_relative) Degrees_of_Freedom_Table_Entry::Save("actual_limits.txt",Table);

	return Table;
}

Degrees_of_Freedom_Table_Entry *Degrees_of_Freedom_Table_Entry::SearchDoFT(const char *Name,Degrees_of_Freedom_Table_Entry *Table) {
	const char *tmp;
	int i;
	int len,lenName;
	lenName=(int)strlen(Name);
	
	i=0;
	while (true) {
		if (Table[i].Name==NULL) return NULL;
		len=(int)strlen(Table[i].Name);
		
		if (lenName>=len) {
			tmp=Name;
			tmp+=lenName-len;

			if (_stricmp(tmp,Table[i].Name)==0) return (Table+i);
		}
		i++;		
	}
}

void Degrees_of_Freedom_Table_Entry::set_block(bool set_zero_all_limits) {
	Translable_X=false;
	Translable_Y=false;
	Translable_Z=false;
	Rotable_X=false;
	Rotable_Y=false;
	Rotable_Z=false;
	if (set_zero_all_limits) {
		LimitsR.MaxX=LimitsR.MinX=0.0;
		LimitsR.MaxY=LimitsR.MinY=0.0;
		LimitsR.MaxZ=LimitsR.MinZ=0.0;
		LimitsT.MaxX=LimitsT.MinX=0.0;
		LimitsT.MaxY=LimitsT.MinY=0.0;
		LimitsT.MaxZ=LimitsT.MinZ=0.0;
	}
}

void Skinned_Mesh::SetConstraints(Degrees_of_Freedom_Table_Entry *Table) {
	for(int i=0;i<num_bones;i++) {
		Bone *Curr=BoneRefTable[i];
		const char *Name=Curr->GetName();
		Degrees_of_Freedom_Table_Entry *F=Degrees_of_Freedom_Table_Entry::SearchDoFT(Name,Table);
		if (F!=NULL) {
			Curr->LockX=!(F->Translable_X);
			Curr->LockY=!(F->Translable_Y);
			Curr->LockZ=!(F->Translable_Z);

			Curr->MinT[0]=F->LimitsT.MinX;
			Curr->MaxT[0]=F->LimitsT.MaxX;
			Curr->MinT[1]=F->LimitsT.MinY;
			Curr->MaxT[1]=F->LimitsT.MaxY;
			Curr->MaxT[2]=F->LimitsT.MaxZ;
			Curr->MinT[2]=F->LimitsT.MinZ;

			Curr->LockRX=!(F->Rotable_X);
			Curr->LockRY=!(F->Rotable_Y);
			Curr->LockRZ=!(F->Rotable_Z);

			Curr->MinR[0]=F->LimitsR.MinX;
			Curr->MaxR[0]=F->LimitsR.MaxX;
			Curr->MinR[1]=F->LimitsR.MinY;
			Curr->MaxR[1]=F->LimitsR.MaxY;
			Curr->MaxR[2]=F->LimitsR.MaxZ;
			Curr->MinR[2]=F->LimitsR.MinZ;
		} else {
			cout<<"Bone \""<<Name<<"\" has no default Degrees of Freedom.\n";
		}
	}
}

void Skinned_Mesh::ApplyConstraints(int frame_index) {
	BoneRoot->CalcRelativeFromAbsolute(frame_index);
	for(int i=0;i<num_bones;i++) {
		BoneRefTable[i]->ApplyConstraints(frame_index);
	}
	BoneRoot->CalcAbsoluteFromRelative(frame_index);
}


void Skinned_Mesh::GetCurrentBoneState(int frame_index,double *values) {
	Matrix Actual_Rotation(3,3);

	for(int k=0;k<num_bones;k++) {
		Bone *Curr=BoneRefTable[k];
		
		RelativeRotation(Curr->RelativeTrasformationMatrix[frame_index],Curr->RelativeTrasformationMatrix[0],&Actual_Rotation);
		Vector<3> Rot=RotationMatrix2AngleAxis(&Actual_Rotation);
		Vector<3> Pos=Curr->RelativeTrasformationT[frame_index]-Curr->RelativeTrasformationT[0];

		*(values++)=Rot[0];		// type 0
		*(values++)=Rot[1];		// type 1
		*(values++)=Rot[2];		// type 2
		*(values++)=Pos[0];		// type 3
		*(values++)=Pos[1];		// type 4
		*(values++)=Pos[2];		// type 5
	}
}







bool Intersection_Mesh(Skinned_Mesh *Mesh,int face1,int face2,bool *intersection_mask) {
	if (face1==face2) return false;

	int *f1_p=Mesh->Faces[face1].Point;
	int *f2_p=Mesh->Faces[face2].Point;



	// Check bone intersection mask 
	//    avoid intersections between faces belonging to neighboring bones and intersection mask 
	int bone_1=0;
	double max=-DBL_MAX;
	for(int j=0;j<3;j++) {
		int tmp_index=f1_p[j];
		for(int k=0;k<Mesh->num_bones;k++) 
			MAX_I(Mesh->Weigth[tmp_index][k],max,k,bone_1);
	}
	int bone_2=0;
	max=-DBL_MAX;
	for(int j=0;j<3;j++) {
		int tmp_index=f2_p[j];
		for(int k=0;k<Mesh->num_bones;k++) 
			MAX_I(Mesh->Weigth[tmp_index][k],max,k,bone_2);
	}
	if (!(intersection_mask[(bone_1*Mesh->num_bones)+bone_2])) return false;




	// check face adiacency
	//    avoid intersections between faces sharing one or more vertices
	bool is_neigh=false;
	for(int j=0;j<3;j++) {
		int tmp_index=f1_p[j];
		for(int q=0;q<3;q++) {
			if (tmp_index==f2_p[q]) {
				is_neigh=true;
				break;
			}
		}
	}
	if (is_neigh) return false;

	return Intersection_TriangleTriangle_v2(Mesh,face1,Mesh,face2);
}

bool *LoadIntersectionMask(Skinned_Mesh *Mesh,char *IntersectionMask_filename) {
	bool *intersection_mask=new bool[Mesh->num_bones*Mesh->num_bones];
	
	for(int i=0;i<Mesh->num_bones*Mesh->num_bones;i++) intersection_mask[i]=true;
	
	// do not compare bones with themselves
	for(int i=0;i<Mesh->num_bones;i++) intersection_mask[i+(i*Mesh->num_bones)]=false;

	// do not compare bones connected by a joint
	for(int i=0;i<Mesh->num_bones;i++) {
		for(int j=0;j<Mesh->num_bones;j++) {
			if (Mesh->BoneRefTable[i]->Father==Mesh->BoneRefTable[j]) {
				intersection_mask[i+(j*Mesh->num_bones)]=false;
				intersection_mask[j+(i*Mesh->num_bones)]=false;
			}
			if (Mesh->BoneRefTable[j]->Father==Mesh->BoneRefTable[i]) {
				intersection_mask[i+(j*Mesh->num_bones)]=false;
				intersection_mask[j+(i*Mesh->num_bones)]=false;
			}
		}
	}

	if (!IntersectionMask_filename) return intersection_mask;

	
	int maxlen;
	char TmpName[BONE_MAX_NAME_LEN+1];

	IFileBuffer In(IntersectionMask_filename);
	if (In.IsFinished()) ErrorExit_str("Cannot read %s",IntersectionMask_filename);
	while (!In.IsFinished()) {
		maxlen=BONE_MAX_NAME_LEN;In.ReadPharse(TmpName,maxlen);
		
		int i=Mesh->GetBoneID(TmpName);
		if (i==-1) {
			cout<<" -> bone "<<TmpName<<" not found.\n";
		} else {
			maxlen=BONE_MAX_NAME_LEN;In.ReadPharse(TmpName,maxlen);
			int j=Mesh->GetBoneID(TmpName);
			if (j==-1) {
				cout<<" -> bone "<<TmpName<<" not found.\n";
			} else {
				intersection_mask[i+(j*Mesh->num_bones)]=false;
				intersection_mask[j+(i*Mesh->num_bones)]=false;
			}
		}
	}

	return intersection_mask;
}




