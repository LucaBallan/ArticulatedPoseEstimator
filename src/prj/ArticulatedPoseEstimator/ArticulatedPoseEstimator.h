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






#define LIMITS_SPINE {-5,5,-5,+5,-10,+60}
#define LIMITS_TORSIONE_BUSTO {-20,20,-10,+10,0,+20}	// il tilt e la torsione si dovrebbero dividere con spine
#define LIMITS_HEAD {-70,+70,-50,+50,-60,+30}
#define LIMITS_CLAVICLE {-10,+10,-10,+10,0,0}			// certe volte puo' ruotare in Z. 
#define LIMITS_L_UPPERARM {-90,40,-40,+85,-90,30}
#define LIMITS_R_UPPERARM {-40,90,-85,+40,-90,30}
#define LIMITS_FOREARM {0,0,0,0,-140,0}
#define LIMITS_CALF {0,0,0,0,-140,0}
#define LIMITS_L_HAND {-90,30,-80,80,0,0}				// Le mani possono ruotare di poco -10,10 anche in Z
#define LIMITS_R_HAND {-30,90,-80,80,0,0}
#define LIMITS_L_THIGH {-90,90,-40,90,-160,90}
#define LIMITS_R_THIGH {-90,90,-90,40,-160,90}
#define LIMITS_FOOT {-90,90,0,0,-90,50}					// i piedi possono ruotare di poco -10,10 anche in Y
#define LIMITS_TOE0 {0,0,0,0,-30,30}					// TODO: limite errato.... (penso nn possa essere <0)
#define LIMITS_FINGER0 {0,0,0,0,0,90}


#define MIDDLE_SPINE "Spine1"
#define TILT_SPINE1 false
#define SUGIU_SPALLE false
#define AVANTIDIETRO_SPALLE false
#define ROTAZIONE_SPALLE false
#define MOVE_HAND false
#define TILT_HAND false

Degrees_of_Freedom_Table_Entry Biped_Degrees_of_Freedom_Simply_Table[]={\
{"Pelvis",true,true,true,true,true,true,LIMITS_DEF_T,LIMITS_DEF_FREE},\
{"Spine",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_SPINE},\
{"Spine1",false,false,false,false,false,true,LIMITS_DEF_T,LIMITS_SPINE},\
{"Spine2",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Spine3",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Spine4",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Spine5",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Neck",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Head",false,false,false,true,true,true,LIMITS_DEF_T,LIMITS_HEAD},\
{"Clavicle",false,false,false,AVANTIDIETRO_SPALLE,SUGIU_SPALLE,ROTAZIONE_SPALLE,LIMITS_DEF_T,LIMITS_CLAVICLE},\
{"L UpperArm",false,false,false,true,true,true,LIMITS_DEF_T,LIMITS_L_UPPERARM},\
{"R UpperArm",false,false,false,true,true,true,LIMITS_DEF_T,LIMITS_R_UPPERARM},\
{"Forearm",false,false,false,false,false,true,LIMITS_DEF_T,LIMITS_FOREARM},\
{"L Hand",false,false,false,TILT_HAND,MOVE_HAND,false,LIMITS_DEF_T,LIMITS_L_HAND},\
{"R Hand",false,false,false,TILT_HAND,MOVE_HAND,false,LIMITS_DEF_T,LIMITS_R_HAND},\
{"Finger0",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_FINGER0},\
{"Finger01",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger02",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},
{"Finger1",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger11",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger12",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger2",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger21",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger22",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger3",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger31",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger32",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger4",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger41",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Finger42",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"L Thigh",false,false,false,true,true,true,LIMITS_DEF_T,LIMITS_L_THIGH},\
{"R Thigh",false,false,false,true,true,true,LIMITS_DEF_T,LIMITS_R_THIGH},\
{"Calf",false,false,false,false,false,true,LIMITS_DEF_T,LIMITS_CALF},\
{"Foot",false,false,false,true,false,true,LIMITS_DEF_T,LIMITS_FOOT},\
{"Toe0",false,false,false,false,false,true,LIMITS_DEF_T,LIMITS_TOE0},\
{"Toe01",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Toe02",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Toe03",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Toe04",false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF},\
{"Bone01",true,true,true,true,true,true,LIMITS_DEF_T,LIMITS_DEF_FREE},\
{NULL,false,false,false,false,false,false,LIMITS_DEF_T,LIMITS_DEF}};

Degrees_of_Freedom_Table_Entry *Contraint_Table;
Degrees_of_Freedom_Table_Entry *Contraint_Table_L1;

void SetOnlyPelvis(Degrees_of_Freedom_Table_Entry *x) {
	Degrees_of_Freedom_Table_Entry *pelvis=Degrees_of_Freedom_Table_Entry::SearchDoFT("Pelvis",x);
	while(x->Name!=NULL) {
		x->Translable_X=false;
		x->Translable_Y=false;
		x->Translable_Z=false;
		x->Rotable_X=false;
		x->Rotable_Y=false;
		x->Rotable_Z=false;
		x++;
	}
	pelvis->Translable_X=true;
	pelvis->Translable_Y=true;
	pelvis->Translable_Z=true;
	pelvis->Rotable_X=true;
	pelvis->Rotable_Y=true;
	pelvis->Rotable_Z=true;
}


void BlockALLExceptTranslable(Degrees_of_Freedom_Table_Entry *Table) {
	int i=0;
	while (Table[i].Name!=NULL) {
		if (!((Table[i].Translable_X==true) && (Table[i].Translable_Y==true) && (Table[i].Translable_Z==true))) {
			Table[i].Rotable_X=false;
			Table[i].Rotable_Y=false;
			Table[i].Rotable_Z=false;
		}
		i++;		
	}
}

void Set_Block(const char *name,Degrees_of_Freedom_Table_Entry *table,bool reset=false,bool inform=true,bool subtree=false) {
	if (_stricmp(name,"all")==0) {
		BlockALLExceptTranslable(table);
		if (inform) cout<<" -> All deactivated except translable ones.\n";
		return;
	}
	Degrees_of_Freedom_Table_Entry *e=Degrees_of_Freedom_Table_Entry::SearchDoFT(name,table);
	if (e==NULL) {
		cout<<" -> "<<name<<" not found.\n";
		//Beep(750,300);
		//exit(1);
	} else {
		e->set_block(reset);
		if (inform) cout<<" -> "<<name<<" deactivated.\n";
	
		if (subtree) {
			Bone *root_bone=Model->BoneRoot->SearchBone(name);
			if (root_bone==NULL) {
				cout<<" -> "<<name<<" not found. Can't propagate.\n";
				exit(1);
			}
			for(int i=0;i<Model->num_bones;i++) {
				Bone *Curr=Model->BoneRefTable[i];
				if (Curr->IsChildOf(root_bone))
					Set_Block(Curr->GetName(),table,reset,inform,false);
			}
		}
	}
}
void Set_No_Propagation(char *name,Bone *boneroot) {
	Bone *b=boneroot->SearchBone(name);
	if (b==NULL) {
		cout<<" -> "<<name<<" not found.\n";
		//Beep(750,300);
	} else {
		b->AllowPropagation_=false;
		cout<<" -> "<<name<<" does not propagate the IK information.\n";
	}
}


/*
void SetMiddle_Spine(char *Name) {
	Degrees_of_Freedom_Table_Limits_Entry Limits=LIMITS_TORSIONE_BUSTO;
	Degrees_of_Freedom_Table_Entry *x=Degrees_of_Freedom_Table_Entry::SearchDoFT(Name,Biped_Degrees_of_Freedom_Table);
	x->Rotable_X=true;
	x->Rotable_Y=TILT_SPINE1;
	x->Rotable_Z=true;
	x->Limits=Limits;
	x=Degrees_of_Freedom_Table_Entry::SearchDoFT(Name,Contraint_Table);
	x->Rotable_X=true;
	x->Rotable_Y=TILT_SPINE1;
	x->Rotable_Z=true;
	x->Limits=Limits;
}
*/


#define FILE_NOT_FOUND(x)  cout<<" -> File not found: "<<x<<"\n";


void LoadCamerasMatrices(char *FileName,char *Distortions) {
	IFileBuffer In(FileName);
	if (In.IsFinished()) {
		FILE_NOT_FOUND(FileName);
		exit(-1);
	}
	In>>num_viste;
	Vista=new View[num_viste];

	for(int i=0;i<num_viste;i++) {
		In>>Vista[i].Int;
		In>>Vista[i].Ext;
		CorrectExtrinsic(&(Vista[i].Ext));
		Vista[i].CreateProjectionMatrix();
		Vista[i].CreateUnProjectionMatrix();
	}
	
	// Distortion
	IFileBuffer InDist(Distortions);
	if (InDist.IsFinished()) {
		FILE_NOT_FOUND(Distortions);
		return;
	} 
	int num_tmp;
	In>>num_tmp;
	if (num_tmp!=num_viste) {
		cout<<"Error: Distortion file corrupted.\n";
		exit(-1);
	}

	for(int i=0;i<num_viste;i++) {
		In>>Vista[i].kc[0];In>>Vista[i].kc[1];In>>Vista[i].kc[2];In>>Vista[i].kc[3];In>>Vista[i].kc[4];
	}
}











ViewStreamer **VStream=NULL;
int           *Sync_data=NULL;

#define BMP_TEST_SEQ(path,post_fix,ext)		sprintf(path,"%s%s%02i_%04i.%s",Image_File,post_fix,i+1,0,ext);		\
											if (!fileExists(path)) {											\
												sprintf(path,"%s%s%02i.avi",Image_File,post_fix,i+1);			\
												if (!fileExists(path)) sprintf(path,"");						\
											} else {															\
												sprintf(path,"%s%s%02i_%%04i.%s",Image_File,post_fix,i+1,ext);	\
											}


void InitLoadFrames(char *Image_File,View *Views,int num_views) {
	VStream=new ViewStreamer*[num_views];

	char path_rgb[FILE_PATH_SIZE];
	char path_mask[FILE_PATH_SIZE];
	char path_depth[FILE_PATH_SIZE];
	char path_edges[FILE_PATH_SIZE];
	char path_invalid_area[FILE_PATH_SIZE];
	char *paths[]={path_rgb,path_mask,path_depth,path_edges,path_invalid_area,NULL};

	for(int i=0;i<num_views;i++) {
		Views[i].SetChannelTypes("CGFGG");
		BMP_TEST_SEQ(path_rgb,"C",BITMAP_EXTENSION);
		BMP_TEST_SEQ(path_mask,"",BITMAP_EXTENSION);
		BMP_TEST_SEQ(path_depth,"D","float");
		BMP_TEST_SEQ(path_edges,"E",BITMAP_EXTENSION);
		BMP_TEST_SEQ(path_invalid_area,"M",BITMAP_EXTENSION);
		VStream[i]=new ViewStreamer(&(Views[i]),paths);
	}

	Sync_data=new int [num_views];
	for(int i=0;i<num_views;i++) Sync_data[i]=0;
	sprintf(path_mask,"%s_sync.txt",Image_File);
	if (fileExists(path_mask)) {
		IFileBuffer In(path_mask);
		for(int i=0;i<num_views;i++) In>>Sync_data[i];
	} else {
		FILE_NOT_FOUND(path_mask);
	}
}
void DestroyLoadFrames(int num_views) {
	for(int i=0;i<num_views;i++) SDELETE(VStream[i]);
	SDELETE(VStream);
	SDELETEA(Sync_data);
}
inline bool AllZero(Bitmap<GreyLevel> *Mask,int x,int y) {
	GreyLevel *c_p=Mask->getBuffer()+(x+(y*Mask->width));

	if (*(c_p+1)!=0) return false;
	if (*(c_p-1)!=0) return false;

	c_p=c_p-Mask->width;

	if (*c_p!=0) return false;
	if (*(c_p+1)!=0) return false;
	if (*(c_p-1)!=0) return false;

	c_p=c_p+(Mask->width<<1);

	if (*c_p!=0) return false;
	if (*(c_p+1)!=0) return false;
	if (*(c_p-1)!=0) return false;

	return true;
}
void LoadFrames(int frame_index,View *Views,int num_views) {
	for(int i=0;i<num_views;i++) {
		if (!(VStream[i]->LoadChannels(frame_index+Sync_data[i]))) ErrorExit("Unable to load all the channels");
		if (Views[i].Mask) {
			Views[i].Mask->ThresholdInvert(128);
			

			// TODOCHECKS check if it is all 255 -> in qual caso l'immagine e' invalida
			GreyLevel *tmp=Views[i].Mask->getBuffer();
			int num_pixels=Views[i].Mask->width*Views[i].Mask->height;
			int q=0;
			for(;q<num_pixels;q++,tmp++) {
				if (*tmp!=255) break;
			}
			if (q==num_pixels) {
				SDELETE(Views[i].Mask);
				SDELETE(Views[i].Edges);
			}
		}
		if (Views[i].Edges) {
			// fill with all the edges in MASK
			if (Views[i].Mask) {
				for(int Px=0;Px<Views[i].Mask->width;Px++) {
					for(int Py=0;Py<Views[i].Mask->height;Py++) {
						if (Views[i].Mask->Point_NCB(Px,Py)!=0) continue;
						if (Views[i].Mask->isOutside(Px,Py,3)) continue;
						
						if (AllZero(Views[i].Mask,Px,Py)) continue;
						Views[i].Edges->SetPoint(Px,Py,0xFF);
					}
				}
			}
		}
	}
}



/*
bool BMP_Sequence=true;
AviVideoIn_OCV<GreyLevel> **VideoSrc;

void InitLoadFrames(char *Image_File,View *Views,int num_views) {
	char path[300];
	sprintf(path,"%s%02i_%04i.bmp",Image_File,1,1);
	if (!fileExists(path)) {
		BMP_Sequence=false;
		VideoSrc=new AviVideoIn_OCV<GreyLevel> *[num_views];
		for(int i=0;i<num_views;i++) {
			sprintf(path,"%s%02i.avi",Image_File,i+1);
			VideoSrc[i]=new AviVideoIn_OCV<GreyLevel>(path);
			if **VideoSrc[i]** ErrorExit("Cannot open the video files.");
		}
	} else BMP_Sequence=true;

	Sync_data=new int [num_views];
	for(int i=0;i<num_views;i++) Sync_data[i]=0;
	sprintf(path,"%s_sync.txt",Image_File);
	if (fileExists(path)) {
		IFileBuffer In(path);
		for(int i=0;i<num_views;i++) In>>Sync_data[i];
	}
}
void LoadFrames(int frame_index,char *Image_File,View *Views,int num_views) {
	if (BMP_Sequence) {
		char path[300];
		for(int i=0;i<num_views;i++) {
			sprintf(path,"%s%02i_%04i.bmp",Image_File,i+1+Sync_data[i],frame_index);

			SDELETE(Views[i].Mask);
			Views[i].Mask=new Image(path);
			Views[i].Mask->ThresholdInvert(128);
			Views[i].width=Views[i].Mask->width;
			Views[i].height=Views[i].Mask->height;
		}
	} else {
		for(int i=0;i<num_views;i++) {
			SNEW(Views[i].Mask,Bitmap<GreyLevel>(VideoSrc[i]->width,VideoSrc[i]->height));
			if (!(VideoSrc[i]->GetFrame(frame_index+Sync_data[i],Views[i].Mask))) ErrorExit("Frame does not exist.");
			
			Views[i].Mask->ThresholdInvert(128);
			Views[i].width=Views[i].Mask->width;
			Views[i].height=Views[i].Mask->height;
		}
	}
}
void DestroyLoadFrames(View *Views,int num_views) {
	for(int i=0;i<num_views;i++) {	
		SDELETE(Views[i].Mask);
		if (!BMP_Sequence) SDELETE(VideoSrc[i]);
	}
}
*/
Features** LoadFeatures(char *Feature_File,View *Views,int num_views) {
	Features **FeaturesList=NULL;
	SNEWA_P(FeaturesList,Features*,num_views);
	for(int i=0;i<num_views;i++) {
		char filename[400];filename[0]=0;
		if (strlen(Feature_File)) {
			sprintf(filename,"%s%02i.feat",Feature_File,i+1);
			if (!fileExists(filename)) sprintf(filename,"%s%02i.txt",Feature_File,i+1);
		}
		SNEW(FeaturesList[i],Features(filename,Sync_data[i]));
	}
	return FeaturesList;
}




void LoadSalientPoints(char *SalientVerticesFile,char *SalientPointsFilePattern,Array<UINT> *&SalientVertices,SalientPoints **&Salient2DPoints,int num_views,int num_frames) {
	SalientVertices=NULL;
	Salient2DPoints=NULL;
	SNEWA_P(Salient2DPoints,SalientPoints*,num_views);

	if (fileExists(SalientVerticesFile)) {
		IFileBuffer in(SalientVerticesFile);

		UINT p_index;
		SalientVertices=new Array<UINT>(30);
		while(!in.IsFinished()) {
			in>>p_index;
			SalientVertices->append(p_index);
		}

		char filename[FILE_PATH_SIZE];
		for(int i=0;i<num_views;i++) {
			sprintf(filename,SalientPointsFilePattern,i+1);
			if (fileExists(filename)) Salient2DPoints[i]=new SalientPoints(filename,num_frames,SALIENT_THRESHOLD);
			else FILE_NOT_FOUND(filename);
		}
		cout<<" -> loaded.\n";
	} else {
		FILE_NOT_FOUND(SalientVerticesFile);
	}
}






























///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////




bool FIRST_TIME_ON_THIS_FRAME=true;
int curren_time;
int What=0;
int Interactive_Iterations=1;
bool enableDraw=true;
AdvancedRenderWindow *Mondo;

bool coma(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *) {
	switch(uMsg) {
		case WM_CHAR:
			wParam=tolower(wParam);
			switch (wParam) {
				case '\\':
					{
					enableDraw=false;
					Solver->DebugImages=PEDEBUG_ITERATIONS;
					Solver->iteration_l0=Interactive_Iterations;
					Solver->iteration_l1=0;
					if (FIRST_TIME_ON_THIS_FRAME) Solver->EstimateFrame(curren_time,curren_time-1,true,false);
					else (dynamic_cast<Articulated_ICP*>(Solver))->Process(curren_time);				// Use the last OpticalFlow data
					FIRST_TIME_ON_THIS_FRAME=false;
					Model->Apply_Skin(curren_time);Model->ComputaNormals();
					enableDraw=true;
					}
					return true;
				case '1': {					
					What=!What;
					return true;
					}
				case 'a': {
					Interactive_Iterations--;
					if (Interactive_Iterations<=0) Interactive_Iterations=1;
					return true;
					}
				case 's': {
					Interactive_Iterations++;
					return true;
					}
				case 'q': {
					curren_time--;
					if (curren_time<=0) curren_time=1;
					Model->Apply_Skin(curren_time);Model->ComputaNormals();
					FIRST_TIME_ON_THIS_FRAME=true;
					return true;
					}
				case 'w': {
					curren_time++;
					Model->Apply_Skin(curren_time);Model->ComputaNormals();
					FIRST_TIME_ON_THIS_FRAME=true;
					return true;
					}
				case '_': {
					Model->SetConstraints(Contraint_Table);
					Model->ApplyConstraints(curren_time);
					Model->Apply_Skin(curren_time);
					Model->ComputaNormals();
					return true;
					}

			}
			return false;
	};
	return false;
}
















void display(void*) {
	if (enableDraw) {
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LESS);
		glShadeModel(GL_SMOOTH);
		
		glEnable(GL_NORMALIZE);
		Mondo->SetLocalCoords();

		Vector<4> c1;
		c1[0]=81;c1[1]=131;c1[2]=206;c1[3]=1.0;
		Mondo->SetMaterial(c1);
		glEnable(GL_LIGHT0);
		float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		Mondo->SetCameraCoords();
		GLfloat light[4]={2,0,0,1.0};
		glLightfv(GL_LIGHT0,GL_POSITION,light);
		Mondo->ClearRT();
		glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION, 0.0f);
		glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION, 0.0f);


		
		if (What) Model->BoneRoot->DrawAll(curren_time,Mondo);
		else Mondo->DrawHyperMesh(Model);

		for(int i=0;i<num_viste;i++) Mondo->DrawCamera(&Vista[i].Ext,i+1,1200);
		
		Mondo->SetImageCoords();
		Vector<3> P;P[0]=-1.0;P[1]=-0.98;
		Mondo->Print(P,"Number of Iterations [a/s]: %i",Interactive_Iterations);
		P[1]=-0.93;
		Mondo->Print(P,"Frame to estimate [q/w]: %i",curren_time);
		Mondo->ClearImageCoords();
	}
}



