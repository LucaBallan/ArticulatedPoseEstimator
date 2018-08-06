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
#include "IKSolver.h"

using namespace std;

int FILTER_EXT=1;


int num_viste;
View *Vista=NULL;
double model_radius;

Array<UINT> *SalientVertices;
bool filter_data,use_texture,save_filter_data;
char CAMERA_MATRIX_FILE[500];
char TEXTURE[500];
char INTERSECTION_MASK_FILE[500];
char IMAGE_FILE[500];
char ModelName[500];
char MODEL_TMP_DIR[500];
char GroundTruth_ModelName[500];
Skinned_Mesh *Model=NULL;
Skinned_Mesh *GroundTruth_Model=NULL;
AdvancedRenderWindow *Mondo;
bool showdata_bones=false;
bool showdata_joints=false;
bool MakeLimits=false;
char BoneToShowName[100];
bool show_distance_data=false;
int vertex_index=-1;
char file_track[500];
int limit_views;
char FILTER_FILE_NAME[300];
int Limit_Frame=-1;
char MODEL_DIR[500];
char MODEL_NAME[500];
#include "CompareMove.h"



void LoadAllTexture(HyperMesh<3> *model,RenderWindow<3> *mondo) {
	cout<<"Loading texture...";
	
	for(int i=0;i<model->num_textures;i++) {
		char tex_file_name[500];
		if (i!=0) sprintf(tex_file_name,"%s%i.bmp",TEXTURE,i);
		else sprintf(tex_file_name,"%s.bmp",TEXTURE);
		cout<<"\n"<<tex_file_name<<"\n";
		model->Texture_GLMap[i]=mondo->LoadTexture(tex_file_name);
		if (model->Texture_GLMap[i]>0) cout<<(i+1)<<"..";
	}
	cout<<"Done.\n";
}

void CorrectBiped(Bone *Root) {
	Root->CalcAbsoluteFromRelative(0);
	Bone *pelvis=Root->SearchBone("Bone_Bip01 Pelvis");
	if (pelvis==NULL) cout<<"   CorrectBiped: Biped01 non trovato.\n";
	else {
		Bone *spine=Root->SearchBone("Bone_Bip01 Spine");
		Bone *Rth=spine->SearchBone("Bone_Bip01 R Thigh");
		Bone *Lth=spine->SearchBone("Bone_Bip01 L Thigh");
		if (Rth!=NULL) {
			cout<<"   WARNING: Bip01 Triangle Pelvis activated. I try to correct it.\n";
			spine->DetachChild(Rth);
			spine->DetachChild(Lth);
			pelvis->AddChild(Rth);
			pelvis->AddChild(Lth);
			Root->CalcRelativeFromAbsolute(0);
		}
	}
	pelvis=Root->SearchBone("Bone_Bip02 Pelvis");
	if (pelvis==NULL) cout<<"   CorrectBiped: Biped02 non trovato.\n";
	else {
		Bone *spine=Root->SearchBone("Bone_Bip02 Spine");
		Bone *Rth=spine->SearchBone("Bone_Bip02 R Thigh");
		Bone *Lth=spine->SearchBone("Bone_Bip02 L Thigh");
		if (Rth!=NULL) {
			cout<<"   WARNING: Bip02 Triangle Pelvis activated. I try to correct it.\n";
			spine->DetachChild(Rth);
			spine->DetachChild(Lth);
			pelvis->AddChild(Rth);
			pelvis->AddChild(Lth);
			Root->CalcRelativeFromAbsolute(0);
		}
	}
}


int main(int argc,char* argv[]) {
	InitGammaLib("Viewer", argc, argv);
	srand((unsigned)time(NULL));

	if (ParseInputData(argc,argv,t_start,t_end,filter_data,save_filter_data,use_texture)<0) return 0;
	t=t_start;

	if ((!showdata_bones) && (!showdata_joints)) cout<<"Reading model.\n";
	Model=new Skinned_Mesh();
	Model->Read(ModelName);
	cerr<<Model->GetRadius(Model->GetCenter());
	cout<<"\n";
	if (Model->num_p==0) ErrorExit("Model file not found.");
	

	if (t_end==-1) t_end=Model->getNumberOfFrames()-1;
	if (t_end>=Model->getNumberOfFrames()) t_end=Model->getNumberOfFrames()-1;
	if (t_start>=Model->getNumberOfFrames()) t_start=1;
	if (t_start>t_end) {
		cout<<"Nothing to visualize.\n";
		return 0;
	}
	if (strlen(GroundTruth_ModelName)) {
		GroundTruth_Model=new Skinned_Mesh();
		GroundTruth_Model->Read(GroundTruth_ModelName);
	}
	if (strlen(CAMERA_MATRIX_FILE)) LoadCamerasMatrices(CAMERA_MATRIX_FILE);
	if (strlen(IMAGE_FILE)) GetCameraResolution(IMAGE_FILE,Vista,num_viste);



	// Get Informations
	Model->BoneRoot->CalcRelativeFromAbsolute(0);
	if (GroundTruth_Model) GroundTruth_Model->BoneRoot->CalcRelativeFromAbsolute(0);

	// Corrections
	CorrectBiped(Model->BoneRoot);
	if (GroundTruth_Model) CorrectBiped(GroundTruth_Model->BoneRoot);

	///////////////////////////////////
	{
		char SalientVerticesFile[500];
		sprintf(SalientVerticesFile,"%s\\..\\salient3d.txt",MODEL_DIR);
		IFileBuffer in(SalientVerticesFile);

		UINT p_index;
		SalientVertices=new Array<UINT>(30);
		while(!in.IsFinished()) {
			in>>p_index;
			SalientVertices->append(p_index);
		}
		if (SalientVertices->numElements()) cout<<"Read "<<SalientVertices->numElements()<<" salient points."<<endl;
	}
	///////////////////////////////////

	if (filter_data) {
		cout<<"Adjusting relative matrices...";
		
		for(int i=t_start-FILTER_EXT;i<=t_end+FILTER_EXT;i++) {
			if (i<1) continue;
			if (i>=Model->getNumberOfFrames()) continue;
			Model->BoneRoot->CalcRelativeFromAbsolute(i);
			Model->BoneRoot->Orthonormalize(i);
			Model->BoneRoot->CalcAbsoluteFromRelative(i);
			
			if (GroundTruth_Model) GroundTruth_Model->BoneRoot->CalcRelativeFromAbsolute(i);
		}
		cout<<" Done.\n";

		cout<<"Filtering...";
		for(int i=t_start;i<=t_end;i++) {
			Model->BoneRoot->Filter(i,FILTER_EXT);
		}
		for(int i=t_start;i<=t_end;i++) {
			Model->BoneRoot->CalcAbsoluteFromRelative(i);
		}
		cout<<" Done.\n";

		if (save_filter_data) {
			cout<<"Saving..."<<FILTER_FILE_NAME<<" ";
			OFileBuffer Out(FILTER_FILE_NAME);
			Bone::SaveSkullMotion(&Out,Model->BoneRoot);
			Out.Close();
			cout<<" Done.\n";
		}
	}

	if (show_distance_data==true) {
		IFileBuffer X(file_track);
		Vector<3> pt;
		double next_frame;
		double num_views;
		X>>pt;
		X>>next_frame;
		X>>num_views;
		mean_std_data dist_info;
		for(int i=t_start;i<=t_end;i++) {
			if ((int)next_frame==i) {
				Model->Apply_Skin(i);
				if (num_views>limit_views) {
					if (((Model->Points[vertex_index]-pt).Norm2())<100) {
					mean_median_std_i((Model->Points[vertex_index]-pt).Norm2(),&dist_info);
					cout<<(Model->Points[vertex_index]-pt).Norm2()<<"\n";
					}
				}
				if (X.IsFinished()) break;
				X>>pt;
				if (X.IsFinished()) break;
				X>>next_frame;
				if (X.IsFinished()) break;
				X>>num_views;
				
			}
		}
		double mean,std,median;UINT n;
		mean_median_std_r(&dist_info,mean,median,std,n);
		cout<<fixed<<median<<" & "<<fixed<<mean<<" & "<<fixed<<std<<" & "<<fixed<<dist_info.max<<" & "<<fixed<<n<<"  \\\\ \n";
		return 0;
	}

	if ((showdata_bones) && (GroundTruth_Model)) {
		for(int i=t_start;i<=t_end;i++) {
			Model->BoneRoot->CalcRelativeFromAbsolute(i);
			GroundTruth_Model->BoneRoot->CalcRelativeFromAbsolute(i);
		}
		mean_std_data over_all;
		for(int bone_id=0;bone_id<Model->num_bones;bone_id++) {
			char filename[500];
			sprintf(filename,"results_positions_%02i.txt",bone_id);
			OFileBuffer out_positions(filename);
			Bone *GT_bone=GroundTruth_Model->BoneRoot->SearchBone(Model->BoneRefTable[bone_id]->GetName());
			Bone *ES_bone=Model->BoneRefTable[bone_id];
			
			//cout<<Model->BoneRefTable[bone_id]->GetName()<<"\n";
			out_positions<<Model->BoneRefTable[bone_id]->GetName()<<"\r\n";
			Vector<3> GT_e;
			Vector<3> ES_e;
			mean_std_data bone_info;
			
			Vector<3> Rot1,T1,Rot2,T2;
			for(int i=t_start;i<=t_end;i++) {
				GT_bone->GetEndPoint(i,&GT_e);
				ES_bone->GetEndPoint(i,&ES_e);
				//cout<<i<<" "<<(ES_e-GT_e).Norm2()<<"\n";
				out_positions<<(ES_e-GT_e).Norm2()<<"\r\n";
				mean_std_i((ES_e-GT_e).Norm2(),&bone_info);
				mean_std_i((ES_e-GT_e).Norm2(),&over_all);
			}
			double mean,std;
			mean_std_r(&bone_info,mean,std);
			cout.precision(4);
			//cout<<" avg = "<<fixed<<mean<<" [mm]    std = "<<fixed<<std<<" [mm]    "<<Model->BoneRefTable[bone_id]->GetName()<<"\n";
			out_positions.Close();
		}
		double mean,std;
		mean_std_r(&over_all,mean,std);
		//cout<<" avg = "<<fixed<<mean<<" [mm]    std = "<<fixed<<std<<" [mm]    ---------------------------------\n";
		//cout<<" max = "<<fixed<<over_all.max<<" [mm]    min = "<<fixed<<over_all.min<<" [min]   ---------------------------------\n";
		cout<<fixed<<mean<<" & "<<fixed<<std<<" & "<<fixed<<over_all.max<<" \\\\ \n";
		over_all.clear();
		for(int bone_id=0;bone_id<Model->num_bones;bone_id++) {
			char filename[500];
			sprintf(filename,"results_angles_%02i.txt",bone_id);
			OFileBuffer out_angles(filename);

			Bone *GT_bone=GroundTruth_Model->BoneRoot->SearchBone(Model->BoneRefTable[bone_id]->GetName());
			Bone *ES_bone=Model->BoneRefTable[bone_id];
			
			//cout<<Model->BoneRefTable[bone_id]->GetName()<<"\n";
			out_angles<<Model->BoneRefTable[bone_id]->GetName()<<"\r\n";
			Vector<3> GT_e;
			Vector<3> ES_e;
			mean_std_data bone_info_rotation;
			
			Vector<3> Rot1,T1,Rot2,T2;
			for(int i=t_start;i<=t_end;i++) {
				GT_bone->RelativeMotion(i,0,&T1,&Rot1);
				ES_bone->RelativeMotion(i,0,&T2,&Rot2);
				for(int m=0;m<3;m++) {
					if ((Rot1[m]!=0.0) || (Rot2[m]!=0.0)) {
						// TODO assumo che questo sia fisso dentro i constraints

						// TODO controllare
						double d1,d2;
						if (Rot1[m]>Rot2[m]) d2=fabs(2*M_PI-Rot1[m]+Rot2[m]);
						else d2=fabs(2*M_PI-Rot2[m]+Rot1[m]);
						d1=fabs(Rot1[m]-Rot2[m]);
						d1=min(d1,d2);
						
						out_angles<<(360.0*d1/(2*M_PI))<<"\r\n";
						mean_std_i(d1,&bone_info_rotation);
						mean_std_i(d1,&over_all);
					}
				}
			}
			double mean,std;
			UINT num_ele;
			mean_std_r(&bone_info_rotation,mean,std,num_ele);
			if (num_ele<3) mean=std=0.0;
			cout.precision(4);
			//cout<<" avg = "<<fixed<<(360.0*mean/(2*M_PI))<<" [deg]   std = "<<fixed<<(360.0*std/(2*M_PI))<<" [deg]   "<<Model->BoneRefTable[bone_id]->GetName()<<"\n";
			out_angles.Close();
		}
		mean_std_r(&over_all,mean,std);
		//cout<<" avg = "<<fixed<<(360.0*mean/(2*M_PI))<<" [deg]   std = "<<fixed<<(360.0*std/(2*M_PI))<<" [deg]   ---------------------------------\n";
		//cout<<" max = "<<fixed<<(360.0*over_all.max/(2*M_PI))<<" [deg]   min = "<<fixed<<(360.0*over_all.min/(2*M_PI))<<" [deg]   ---------------------------------\n";
		cout<<fixed<<(360.0*mean/(2*M_PI))<<" & "<<fixed<<(360.0*std/(2*M_PI))<<" & "<<fixed<<(360.0*over_all.max/(2*M_PI))<<" \\\\ \n";
		return 0;
	}

	if (showdata_bones) {
		Bone *BoneToShow=Model->BoneRoot->SearchBone(BoneToShowName);
		if (BoneToShow==NULL) ErrorExit("Error: Bone not found.");
		for(int i=t_start;i<=t_end;i++) {
			Model->BoneRoot->CalcRelativeFromAbsolute(i);
			Vector<3> Rot,T;
			BoneToShow->RelativeMotion(i,0,&T,&Rot);
			double a,b,c;
			a=Rot[0];b=Rot[1];c=Rot[2];
			a=a*180/M_PI;
			b=b*180/M_PI;
			c=c*180/M_PI;
			cout<<i<<" "<<a<<" "<<b<<" "<<c<<"\n";
		}
		return 0;
	}

	if (showdata_joints) {
		Bone *BoneToShow=Model->BoneRoot->SearchBone(BoneToShowName);
		if (BoneToShow==NULL) ErrorExit("Error: Bone not found.");
		for(int i=t_start;i<=t_end;i++) {
			Vector<3> T=BoneToShow->GetJointPosition(i);
			cout<<T[0]<<" "<<T[1]<<" "<<T[2]<<"\n";
		}
		return 0;
	}
	if (MakeLimits) {
		for(int i=0;i<Model->num_bones;i++) {
			Bone *BoneToShow=Model->BoneRefTable[i];
			cout<<BoneToShow->GetName()<<"\n";

			Vector<3> R_MAX,R_MIN;
			Vector<3> T_MAX,T_MIN;
			if (t_start==0) t_start=1;
			if (Limit_Frame!=-1) {
				t_start=t_end=Limit_Frame;
			}
			for(int t=t_start;t<=t_end;t++) {
				Model->BoneRoot->CalcRelativeFromAbsolute(t);
				Vector<3> Rot,T;
				BoneToShow->RelativeMotion(t,0,&T,&Rot);
				Rot=(180.0/M_PI)*Rot;
				R_MAX=Max(Rot,R_MAX);
				R_MIN=Min(Rot,R_MIN);
				T_MAX=Max(T,T_MAX);
				T_MIN=Min(T,T_MIN);
			}

			if ((T_MAX-T_MIN).Norm2()<10000*FLT_EPSILON) cout<<"0 ";
			else cout<<"1 ";
			
			for(int k=0;k<3;k++) {
				if (fabs((R_MAX[k]-R_MIN[k]))<10000*FLT_EPSILON) cout<<"0 ";
				else cout<<"1 ";
			}
			cout<<"\n";
			cout.precision(1);
			cout<<fixed<<R_MIN[0]<<" "<<fixed<<R_MAX[0]<<"\n";
			cout<<fixed<<R_MIN[1]<<" "<<fixed<<R_MAX[1]<<"\n";
			cout<<fixed<<R_MIN[2]<<" "<<fixed<<R_MAX[2]<<"\n";
		}
		return 0;
	}
	Model->Apply_Skin(1);Model->ComputaNormals();
	if (GroundTruth_Model) {GroundTruth_Model->Apply_Skin(1);GroundTruth_Model->ComputaNormals();}
	Vector<3> C=Model->GetCenter();
	model_radius=Model->GetRadius(C);
	
	Mondo=new AdvancedRenderWindow("Compare By Ballan Luca 2008",display,coma,NULL,0xFFFFFF);
	Mondo->DefineLocalCoords(C,model_radius,false,10.0);
	Mondo->DefineNearFarPlane(model_radius/10,20*model_radius);
	if (use_texture) LoadAllTexture(Model,Mondo);
	Mondo->EnableDraw();
	

	cout<<"Commands:\n";
	cout<<"    q/a - increase/decrease time\n";
	cout<<"    1   - span iterations if available\n";
	cout<<"    2/3 - span particles\n";
	cout<<"    w   - show skeleton/skin\n";
	cout<<"    5   - display cameras\n";
	//cout<<"    p   - print angles information\n";
	cout<<"    p   - select bone\n";
	cout<<"    o   - select axis\n";
	cout<<"    l   - print extrinsic\n";
	cout<<"    c   - change camera\n";
	cout<<"    [/] - rotate selected bone\n";
	cout<<"    *   - save a video\n";
	cout<<"    \\ - exit\n\n";
	cout<<"    SHIFT - select a vertex \n";
	cout<<"    CTRL  - select a bone \n";
	MSG msg;
  	clock_t init=clock(),end;
	while (1) {
		end=clock();
		if (GO) {
			if (end-init>0.0001*CLK_TCK) {
				init=clock();
				t++;
				if (t>t_end) t=t_start;
				Model->Apply_Skin(t);Model->ComputaNormals();
				if (GroundTruth_Model) {GroundTruth_Model->Apply_Skin(t);GroundTruth_Model->ComputaNormals();}
				Mondo->Redraw();
			}
		}
		if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
			if (WM_QUIT==msg.message) break;
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}


	cout<<"Closing...";
	delete Mondo;
	delete Model;
	if (GroundTruth_Model) delete GroundTruth_Model;
	cout<<" Done.\n";
	return 0;
}












/************************************************************************************/
/************************ INPUT PARSERS *********************************************/
/************************************************************************************/

void PrintHelp() {
	cout<<"-- Articulated Deformable Model Viewer  --\n";
	cout<<"--          Ballan Luca 2007            --\n\n";

	cout<<"Compare -I Model [start_index] [end_index] [GroundTruth_Model]\n";
	cout<<"Compare -D data.txt [start_index] [end_index] [GroundTruth_Model]\n\n";
	cout<<"Options:    -T                   Use textures\n";
	cout<<"            -L Frame_index       Show Maximum Euler-angle of all bones (-1 max and min for all the frames)\n";
	cout<<"            -F dd [name.motion]  Filter data for dd extension\n";
	cout<<"                                 Save the filtered motion in [motion].\n";
	cout<<"            -J BoneName          Show Euler-angle of the joint associated to \"BoneName\"\n";
	cout<<"            -B BoneName          Show Euler-angle of \"BoneName\"\n\n";
	cout<<"            -Q v min_views file.txt        Show average distance with vertex\n";
}


int ParseInputData(int argc,char* argv[],int &t_start,int &t_end,bool &filter_data,bool &save_filter_data,bool &use_texture) {
	int fnc=0;
	if (argc<1) {
		PrintHelp();
		return -1;
	}
	fnc=-1;
	t_start=1;
	t_end=-1;
	ModelName[0]=0;
	MODEL_NAME[0]=0;
	GroundTruth_ModelName[0]=0;
	CAMERA_MATRIX_FILE[0]=0;

	CommandParser Parser(argc,argv);
	if (Parser.GetParameterE('I',"s[i][i][s]",ModelName,&t_start,&t_end,GroundTruth_ModelName)) fnc=1;
	if (Parser.GetParameterE('D',"s[i][i][s]",ModelName,&t_start,&t_end,GroundTruth_ModelName)) fnc=2;


	use_texture=Parser.GetFlag('T');
	filter_data=false;
	save_filter_data=false;
	MakeLimits=false;
	if (Parser.GetParameter('F',"i",&FILTER_EXT)) filter_data=true;
	if (Parser.GetParameter('F',"is",&FILTER_EXT,FILTER_FILE_NAME)) {filter_data=true;save_filter_data=true;}
	if (Parser.GetParameter('J',"s",BoneToShowName)) showdata_joints=true;
	if (Parser.GetParameter('B',"s",BoneToShowName)) showdata_bones=true;
	if (Parser.GetParameter('L',"i",&Limit_Frame)) MakeLimits=true;
	if (Parser.GetParameter('Q',"iis",&vertex_index,&limit_views,file_track)) show_distance_data=true;

	Parser.CheckInvalid();

	if (fnc==2) {
		IFileBuffer In(ModelName);
		if (In.IsFinished()) {
			cout<<"File \"Data.txt\" not found.\n";
			cout<<"File format is:\n";
			cout<<"CAMERA_MATRIX_FILE\nMODEL_DIR\nMODEL_NAME\nIMAGE_FILES\nFEAUTURE_FILES\nOUT_FILE_DIR\nOUT_DEBUG_FILE_DIR\n\n";
			return -1;
		}
		int len;
		len=499;In.ReadPharse(CAMERA_MATRIX_FILE,len);
		len=499;In.ReadPharse(MODEL_DIR,len);
		len=99;In.ReadPharse(MODEL_NAME,len);
		len=499;In.ReadPharse(IMAGE_FILE,len);
		len=499;In.ReadPharse(MODEL_DIR,len);
		len=499;In.ReadPharse(MODEL_DIR,len);
		len=499;In.ReadPharse(MODEL_TMP_DIR,len);

		strcpy(ModelName,MODEL_DIR);
		strcat(ModelName,"\\");
		strcat(ModelName,MODEL_NAME);
		strcpy(TEXTURE,MODEL_DIR);
		strcat(TEXTURE,"\\Texture");
		
		strcpy(INTERSECTION_MASK_FILE,MODEL_DIR);
		strcat(INTERSECTION_MASK_FILE,"\\..\\IntersectionMask.txt");
	}
	if (fnc==1) {
		int i;
		for(i=(int)strlen(ModelName)-1;i>=0;i--) if (ModelName[i]=='\\') break;
		if (i!=-1) {
			for(int j=0;j<=i;j++) TEXTURE[j]=ModelName[j];
			TEXTURE[i+1]=0;
			strcpy(MODEL_DIR,TEXTURE);
			strcat(TEXTURE,"Texture");
			strcpy(INTERSECTION_MASK_FILE,MODEL_DIR);
			strcat(INTERSECTION_MASK_FILE,"IntersectionMask.txt");
		}
	}

	if (t_start<1) t_start=1;
	if (fnc<0) PrintHelp();
	return fnc;
}















