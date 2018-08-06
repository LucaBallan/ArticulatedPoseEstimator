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






/************************************************************************************/
/************************ INPUT PARSERS *********************************************/
/************************************************************************************/

void PrintHelp() {
	char software_name[]="ArticulatedPoseEstimator";
	cout<<software_name<<" data.txt -A first_frame last_frame [motion_file] [OPTIONS]\n";
	cout<<"              Estimates all the frames from first_frame+1 to last_frame\n\n\n";
	cout<<"Debug:\n";
	cout<<"              -O x -> Output type   [0] - full information and debug images\n";
	cout<<"                                     1  - minimal information and debug images\n";
	cout<<"                                     2  - minimal information only\n";
	cout<<"                                     3  - nothing\n";
	cout<<"              -C x -> Debug type    [0] - Pixel discrepancy error\n";
	cout<<"                                     1  - Estimated silhouettes\n";
	cout<<"                                     2  - Colour silhouette overlapping\n";
	cout<<"                                     3  - Show correspondences\n";
	cout<<"                                     4  - Do not save images\n";
	cout<<"                                     5  - Show only salient points\n";
	cout<<"                                     6  - Wireframe rendering \n";
	cout<<"              -X   -> Save debug iterations images\n";
	cout<<"              -R   -> Save only the debug images of the last iteration\n";
	cout<<"              -D   -> Do not delete old debug images\n";
	cout<<"\nInput:\n";
	cout<<"              -K [limit_file] -> Load limit_file (default limits.txt)\n";
	cout<<"              -L   -> Create default limits file limits.txt\n";
	cout<<"              -U   -> Use the motion file in output dir\n";
	cout<<"\n";
	cout<<"              -V x -> ViewMask (def=11...1)  0  disable view\n";
	cout<<"                                            [1] enable view\n";
	cout<<"                                             2  enable only optical flow\n";
	cout<<"                                             3  enable only silhouette\n";
	cout<<"\n";
	cout<<"              -n \"Name\" x  -> Block bone \"Name\" during the estimation\n";
	cout<<"                                \"Name\"=\"all\" -> block all except the translable ones.\n";
	cout<<"                                n is a number between 1 and 9\n";
	cout<<"                                reset = 0 Block the bone\n";
	cout<<"                                        1 Reset the bone dofs\n";
	cout<<"                                        2 block the entire subtree\n";
	cout<<"                                        3 block and reset the entire subtree\n";
	cout<<"\n";
	cout<<"              -. w_e w_o \"Name1\" [\"Name2\"]\n";
	cout<<"                       w_e    extra weight for edges\n";
	cout<<"                       w_o    extra weight for optical flow\n";
	cout<<"                       Add more weight to correspondences related to that those\n";
	cout<<"                       bones.\n";
	cout<<"\n\nLocal Optimization:\n";
	cout<<"              -P L0 L1 p_f\n";
	cout<<"                       L0    iterations L0                           (def="<<L0_ITERATIONS<<")\n";
	cout<<"                       L1    iterations L1                           (def="<<L1_ITERATIONS<<")\n";
	cout<<"                       p_f   proximity_factor                        (def="<<PROXIMITY_F<<")\n";
	cout<<"\n";
	cout<<"              -I n_i n_c\n";
	cout<<"                   -> Use the prediction as initial guess\n";
	cout<<"                       n_i   number of iterations                    (def="<<PREDICTION_ITERATIONS<<")\n";
	cout<<"                       n_c   number of vertices to predict           (def="<<MAX_PREDICTION_C<<")\n";
	cout<<"\n";
	cout<<"              -G   -> Use the current estimate as initial guess\n";
	cout<<"                       E.g.: -G -A 75 75              Refining\n";
	cout<<"                       E.g.: -G -A 10 30 -P 0 0 0     Reproject all and\n";
	cout<<"                                                      set constraints\n";
	cout<<"                       E.g.: -G -A 10 30 -P 0 0 0 -Q  Reproject all\n";
	cout<<"                       E.g.: -G -A 10 30 -P 0 0 0 -0  Set Constraints\n";
	cout.precision(1);
	cout<<"\n";
	cout<<"              -M x -> Outliers smoothness threshold                  (def="<<fixed<<SOGLIA_SMOOTH<<")\n";
	cout.precision(1);
	cout<<"\n";
	cout<<"              -Z s_s s_d [e_s eit emt emd m_d]\n";
	cout<<"                   -> Silhouette/Edges information\n";
	cout<<"                       s_s   shape descriptor threshold              (def="<<fixed<<SOGLIA_TOLLERANZA_GRADIENTE<<")\n";
	cout<<"                       s_d   max search distance                     (def="<<LEN_MAX<<"px)\n";
	cout<<"                       e_s   edge/silhouette preferences             (def="<<EDGE_SIL_CONF<<")\n";
	cout<<"                               bit 0 = 0 force to use only the\n"; 
	cout<<"                                         external silhouettes\n";
	cout<<"                                     = 1 use edge map when available\n";
	cout<<"                               bit 1 = 0 use depthmap edges\n";
	cout<<"                                     = 1 use textured model edges\n";
	cout<<"                       eit   edge threshold for input edge map       (def="<<INPUT_EDGES_IMAGES_EDGE_THRESHOLD<<")\n";
	cout<<"                       emt   edge threshold for the textured model   (def="<<TEXTURED_MODEL_IMAGES_EDGE_THRESHOLD<<")\n";
	cout<<"                       emd   edge threshold for the depth model      (def="<<DEPTH_MODEL_IMAGES_EDGE_THRESHOLD<<")\n";
	cout<<"                       m_d   max distance between an external edge   (def="<<TIPICAL_EROSION_FOR_OBJECT_MASK<<")\n";
	cout<<"                             and the mask\n";
	cout.precision(1);
	cout<<"\n";
	cout<<"              -N o_d o_i o_w\n";
	cout<<"                   -> OpticalFlow information\n";
	cout<<"                       o_d   max allowed displacement                (def="<<MAX_OF_INFO_DISTANCE<<"% screen size)\n";
	cout.precision(0);
	cout<<"                       o_i   percentange of iterations               (def="<<fixed<<OPTICALFLOW_ITERATIONS<<"%)\n";
	cout.precision(1);
	cout<<"                       o_w   optical flow weight                     (def="<<fixed<<OPTICALFLOW_LOCAL_WEIGHT<<")\n";
	cout<<"\n";
	cout<<"              -F L0_w L1_w\n";
	cout<<"                   -> Depth information\n";
	cout<<"                       L0_w  weight L0                               (def="<<fixed<<DEPTH_LOCAL_WEIGHT_L0<<")\n";
	cout<<"                       L1_w  weight L1                               (def="<<fixed<<DEPTH_LOCAL_WEIGHT_L1<<")\n";
	cout<<"\n";
	cout<<"              -T t_w\n";
	cout<<"                   -> Texture information\n";
	cout<<"                       t_w   texture weight                          (def="<<fixed<<TEXTURE_LOCAL_WEIGHT<<")\n";
	cout<<"\n";
	cout<<"              -B s_w m_d [s_t d_t]\n";
	cout<<"                   -> Salient point information\n";
	cout<<"                       s_w   salient point weight                    (def="<<fixed<<SALIENT_POINT_LOCAL_WEIGHT<<")\n";
	cout<<"                       m_d   max search distance                     (def="<<fixed<<SALIENT_POINT_MAX_DISTANCE<<")\n";
	cout<<"                       s_t   input points confidence threshold       (def="<<fixed<<SALIENT_THRESHOLD<<")\n";
	cout<<"                       d_t   occlusion depth tollerance              (def="<<fixed<<SALIENT_DEPTH_THRESHOLD<<")\n";
	cout<<"\n";
	cout<<"              -E L0_w L1_w c_s n_o [ttl] [c_w cttl]\n";
	cout<<"                   -> Collisions avoidance information\n";
	//cout<<"                      (LINEAR INTERPOLATION HAS TO BE IMPLEMENTED & SIGMA TO SET AUTO)\n";
	cout.precision(1);
	cout<<"                       L0_w  weight L0                               (def="<<fixed<<COLLISION_WEIGHTS_L0<<") (suggested 30.0)\n";
	cout<<"                       L1_w  weight L1                               (def="<<fixed<<COLLISION_WEIGHTS_L1<<")\n";
	cout<<"                       c_s   collision sigma                         (def="<<fixed<<collision_sigma<<")\n";
	cout<<"                       n_o   collision observations                  (def="<<ADDITIONAL_OBSERVATIONS<<")\n";
	cout<<"                               not suff. -> a random set is selected\n";
#ifdef FORCE_ONE_COLLISION_PER_VERTEX
	cout<<"                                   suff. -> stocastic result (FORCE_ONE_COLLISION_PER_VERTEX)\n";
#else
	cout<<"                                   suff. -> deterministic result (!FORCE_ONE_COLLISION_PER_VERTEX)\n";
#endif
	cout<<"                       ttl  collision time to live                 (def="<<COLLISION_TTL<<")\n";
	cout<<"                       c_w  contact points weight                  (def="<<CONTACT_POINTS_WEIGHT<<")\n";
	cout<<"                       cttl contact points time to live            (def="<<CONTACT_POINTS_TTL<<")\n";
	cout<<"\n\nGlobal Optimization:\n";
	cout<<"              -J n_p n_r ns ni\n";
	cout<<"                   -> Enable simulated annealing\n";
	cout<<"                       n_p     particles per iterations/evolutions   (def="<<Annealing_num_particles_per_iteration<<")\n";
	cout<<"                       n_r     particles to save at each iteration   (def="<<Annealing_resample_elements<<")\n";
	cout.precision(1);
	cout<<"                       ns      noise ratio  (% of motion extension)  (def="<<fixed<<noise_ratio<<")\n";
	cout<<"                               ns*h_level is the actual ratio\n";
	cout.precision(0);
	cout<<"                       ni      number of itarations (=resamples)     (def="<<fixed<<PARTICLE_INTERNAL_ITERATION<<")\n";
	cout<<"                               (L0+L1)/ni local opt. per particle\n";
	cout<<"                               (L0+L1)=0  -> no local optimization\n";
	cout<<"\n";
	cout.precision(1);
	cout<<"              -W o_w d_w c_w t_w e_w p_w y_w\n";
	cout<<"                   -> Information weight (~= inverse of the standard deviation)\n";
	cout<<"                       o_w   optical flow weight                     (def="<<fixed<<OPTICALFLOW_WEIGHT<<")\n";
	cout<<"                       d_w   depth weight                            (def="<<fixed<<DEPTH_WEIGHT<<")\n";
	cout<<"                       c_w   collision weight                        (def="<<fixed<<COLLISION_WEIGHT<<")\n";
	cout<<"                       t_w   texture weight                          (def="<<fixed<<TEXTURE_WEIGHT<<")\n";
	cout<<"                       e_w   edges weight                            (def="<<fixed<<EDGES_WEIGHT<<")\n";
	cout<<"                       p_w   salient points weight                   (def="<<fixed<<SALIENT_POINTS_WEIGHT<<")\n";
	cout<<"                       y_w   local optimization weight               (def="<<fixed<<OPTIMIZATION_WEIGHT<<")\n";
	cout<<"\n";
	cout<<"              -H i_L0 i_L1 n_p rnd ar\n";
	cout<<"                   -> Additional parameters\n";
	cout<<"                       i_L0    initial iterations L0                 (def="<<initial_iteration_l0<<")\n";
	cout<<"                       i_L1    initial iterations L1                 (def="<<initial_iteration_l1<<")\n";
	cout<<"                       n_p     number of processor                   (def=0 autoselect)\n";
	cout<<"                       rnd     random seed                           (def=0 time-randomized)\n";
	cout<<"                       ar      apply noise on root                   (def=0 false)\n";
	cout << "\n";
	cout<<"              -help";
	cout<<"\n";
	cout<<"\n";
	cout<<software_name<<" data.txt -S first_frame target_frame [motion_file] [OPTIONS]\n";
	cout<<"            Perform a single estimation of target_frame from first_frame\n";
	cout<<"              [motion_file] the first frame is loaded from this file\n";
	cout<<"\n";
	cout<<software_name<<" data.txt -Y first_frame target_frame [start_motion_file] [OPTIONS]\n";
	cout<<"            Interactive estimation (use key z to estimate)\n";
	cout<<"\n";
	cout<<"To print the error function use the following syntax:\n";
	cout<<software_name<<" tmp_data.txt -A first_frame last_frame -G -P 0 0 0 -Q -U -W 0 -D -C 0\n";
	cout<<"and remember to set the correct limits.txt otherwise it will be constrained and saved.\n";
	cout<<"NOTE: If edge map is used, the related nask should always be eroded enought to do\n";
	cout<<"      do not contain the external edges.\n\n\n";
}

void ParseInputData_Constraints(int argc,char* argv[]) {
	int reset;
	char Name[100];
	CommandParser Parser(argc,argv);

	if (Parser.GetParameterE('1',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('2',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('3',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('4',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('5',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('6',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('7',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('8',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
	if (Parser.GetParameterE('9',"si",Name,&reset)) Set_Block(Name,Contraint_Table,(reset&0x1?true:false),true,(reset&0x2?true:false));
}
int ParseInputData(int argc,char* argv[],int &st,int &ed) {
	
	// Set Default
	int fnc=0;
	int output_type=0;
	int apply_noise_on_root=0;
	START_MotionFileName[0]=0;
	strcpy(LIMITS_FILE_NAME,"");
	VIEW_VALID_MASK[0]=0;
	EXTRA_W_NAME_1[0]=0;
	EXTRA_W_NAME_2[0]=0;
	load_limits=false;
	UseLast=false;
	DebugLast=false;
	AlwaysDebug=false;
	NeverDebug=false;
	// Switches
	apply_constrains=true;
	avoid_collisions=false;
	optimization_method=1;


	CommandParser Parser(argc,argv);
	
	// main f
	if (Parser.GetParameterE('S',"ii[s]",&st,&ed,START_MotionFileName)) fnc=1;
	if (Parser.GetParameterE('A',"ii[s]",&st,&ed,START_MotionFileName)) fnc=2;
	if (Parser.GetParameterE('Y',"ii[s]",&st,&ed,START_MotionFileName)) fnc=3;
	
	// local
	Parser.GetParameterE('P',"iid",&L0_ITERATIONS,&L1_ITERATIONS,&PROXIMITY_F);
	Parser.GetParameterE('M',"d",&SOGLIA_SMOOTH);
	Parser.GetParameterE('Z',"di[iiidi]",&SOGLIA_TOLLERANZA_GRADIENTE,&LEN_MAX,&EDGE_SIL_CONF,&INPUT_EDGES_IMAGES_EDGE_THRESHOLD,&TEXTURED_MODEL_IMAGES_EDGE_THRESHOLD,&DEPTH_MODEL_IMAGES_EDGE_THRESHOLD,&TIPICAL_EROSION_FOR_OBJECT_MASK);
	Parser.GetParameterE('N',"ddd",&MAX_OF_INFO_DISTANCE,&OPTICALFLOW_ITERATIONS,&OPTICALFLOW_LOCAL_WEIGHT);
	Parser.GetParameterE('B',"di[dd]",&SALIENT_POINT_LOCAL_WEIGHT,&SALIENT_POINT_MAX_DISTANCE,&SALIENT_THRESHOLD,&SALIENT_DEPTH_THRESHOLD);
	Parser.GetParameterE('F',"dd",&DEPTH_LOCAL_WEIGHT_L0,&DEPTH_LOCAL_WEIGHT_L1);
	Parser.GetParameterE('T',"d",&TEXTURE_LOCAL_WEIGHT);
	Parser.GetParameterE('E',"dddi[i][di]",&COLLISION_WEIGHTS_L0,&COLLISION_WEIGHTS_L1,&collision_sigma,&ADDITIONAL_OBSERVATIONS,&COLLISION_TTL,&CONTACT_POINTS_WEIGHT,&CONTACT_POINTS_TTL);
	Parser.GetParameterE('.',"dds[s]",&EXTRA_W_E,&EXTRA_W_OF,EXTRA_W_NAME_1,EXTRA_W_NAME_2);
	if ((COLLISION_WEIGHTS_L0!=0.0) || (COLLISION_WEIGHTS_L1!=0.0) || (CONTACT_POINTS_WEIGHT!=0.0)) avoid_collisions=true;

	// global
	if (Parser.GetParameterE('J',"iidd",&Annealing_num_particles_per_iteration,&Annealing_resample_elements,&noise_ratio,&PARTICLE_INTERNAL_ITERATION)) optimization_method=3;
	Parser.GetParameterE('W',"ddddddd",&OPTICALFLOW_WEIGHT,&DEPTH_WEIGHT,&COLLISION_WEIGHT,&TEXTURE_WEIGHT,&EDGES_WEIGHT,&SALIENT_POINTS_WEIGHT,&OPTIMIZATION_WEIGHT);
	Parser.GetParameterE('H',"iiiii",&initial_iteration_l0,&initial_iteration_l1,&number_processor,&random_seed,&apply_noise_on_root);
	APPLY_NOISE_ON_ROOT=(apply_noise_on_root==1);
	if (Parser.GetParameterE('I',"ii",&PREDICTION_ITERATIONS,&MAX_PREDICTION_C)) PREDICTING=true;
	



	Parser.GetParameterE('V',"s",VIEW_VALID_MASK);
	if (Parser.GetParameterE('K',"[s]",LIMITS_FILE_NAME)) load_limits=true;

	Parser.GetParameterE('O',"i",&output_type);
	switch (output_type) {
		case 0:	debug_info_type=2;
				NeverDebug=false;
				break;
		case 1:	debug_info_type=1;
				NeverDebug=false;
				break;
		case 2:	debug_info_type=1;
				NeverDebug=true;
				break;
		default:
		case 3:	debug_info_type=0;
				NeverDebug=true;
				break;
	};
	if ((Parser.GetFlag('X')) && (!NeverDebug)) AlwaysDebug=true;
	if ((Parser.GetFlag('R')) && (!NeverDebug)) DebugLast=true;


	if (Parser.GetFlag('U')) UseLast=true;
	if (Parser.GetFlag('G')) REFINING=true;
	if (Parser.GetFlag('D')) delete_all_debug_files=false;
	if (Parser.GetFlag('L')) {
		Degrees_of_Freedom_Table_Entry::Save("limits.txt",Biped_Degrees_of_Freedom_Simply_Table);
		cout<<"Limits.txt created.\n";
		exit(0);
	}
	if (Parser.GetFlag('Q')) apply_constrains=false;
	Parser.GetParameterE('C',"i",&Debug_Type);
	if ((Debug_Type<0) || (Debug_Type>7)) Debug_Type=0;


	// Flush Additional parameters
	int reset;
	char Name[100];
	Parser.GetParameterE('1',"si",Name,&reset);
	Parser.GetParameterE('2',"si",Name,&reset);
	Parser.GetParameterE('3',"si",Name,&reset);
	Parser.GetParameterE('4',"si",Name,&reset);
	Parser.GetParameterE('5',"si",Name,&reset);
	Parser.GetParameterE('6',"si",Name,&reset);
	Parser.GetParameterE('7',"si",Name,&reset);
	Parser.GetParameterE('8',"si",Name,&reset);
	Parser.GetParameterE('9',"si",Name,&reset);


	//
	// Controllo errori
	//
	if (!(Parser.CheckInvalid())) {
		PrintHelp();
		return -1;
	}
	if (fnc==0) {
		PrintHelp();
		return -1;
	}
	if (fnc!=2) {
		DebugLast=false;
		AlwaysDebug=false;
	}
	if (optimization_method==3) {
		if ((OPTICALFLOW_LOCAL_WEIGHT==0.0) && (OPTICALFLOW_WEIGHT!=0.0)) {
			cout<<"Optical flow: the local weight can't be zero if the global weight is different from zero.\n";
			return -1;
		}
		if ((!avoid_collisions) && (COLLISION_WEIGHT!=0)) {
			cout<<"Collision: the local weight can't be zero if the global weight is different from zero.\n";
			return -1;
		}
	}
	
	


	
	

	IFileBuffer In(argv[1]);
	if (In.IsFinished()) {
		cout<<"ERROR: File \""<<argv[1]<<"\" not found.\n";
		cout<<"File format is:\n";
		cout<<"CAMERA_MATRIX_FILE\nMODEL_DIR\nMODEL_NAME\nIMAGE_FILES\nFEAUTURE_FILES\nOUT_FILE_DIR\nOUT_DEBUG_FILE_DIR\n\n";
		return -1;
	}
	int len;
	len=499;In.ReadPharse(CAMERA_MATRIX_FILE,len);
	len=499;In.ReadPharse(MODEL_DIR,len);
	len=99;In.ReadPharse(MODEL_NAME,len);
	strcpy(MODEL_FILE,MODEL_DIR);
	strcat(MODEL_FILE,"\\");
	strcat(MODEL_FILE,MODEL_NAME);
	strcpy(DISTORTION_FILE,MODEL_DIR);
	strcat(DISTORTION_FILE,"\\Distortion.txt");
	strcpy(VALIDLIST_FILENAME,MODEL_DIR);
	strcat(VALIDLIST_FILENAME,"\\valid_list.txt");
	strcpy(SALIENT_3D_FILENAME,MODEL_DIR);
	strcat(SALIENT_3D_FILENAME,"\\salient3d.txt");
	strcpy(SALIENT_2D_FILENAME,MODEL_DIR);
	strcat(SALIENT_2D_FILENAME,"\\salient2d_%02i.txt");
	len=499;In.ReadPharse(IMAGE_FILE,len);
	len=499;In.ReadPharse(FEAUTURE_FILE,len);
	len=499;In.ReadPharse(OUT_FILE_DIR,len);
	len=499;In.ReadPharse(OUT_DEBUG_FILE_DIR,len);
	
	if (!load_limits) {
		load_limits=true;
		strcpy(LIMITS_FILE_NAME,MODEL_DIR);
		strcat(LIMITS_FILE_NAME,"\\limits.txt");
	}
	strcpy(INTERSECTION_MASK_FILE,MODEL_DIR);
	strcat(INTERSECTION_MASK_FILE,"\\IntersectionMask.txt");

	return fnc;
}

void MakeOutput(Skinned_Mesh *Model) {
	char MeshFileName[300];
	char BoneStructureFileName[300];
	char WeigthFileName[300];
	char PoseFileName[300];
	char AdjFileName[300];
	
	char MotionFileName[300];
	char TextureFileName[300];
	

	
	strcpy(MeshFileName,MODEL_FILE);
	strcpy(BoneStructureFileName,MODEL_FILE);
	strcpy(WeigthFileName,MODEL_FILE);
	strcpy(PoseFileName,MODEL_FILE);
	strcpy(AdjFileName,MODEL_FILE);
	strcat(MeshFileName,".objbin");
	strcat(BoneStructureFileName,".skel");
	strcat(WeigthFileName,".skin");
	strcat(PoseFileName,".pose");
	strcat(AdjFileName,".adj");

	strcpy(TextureFileName,MODEL_DIR);
	strcat(TextureFileName,"\\Texture.bmp");

	
	mkdir(OUT_FILE_DIR);
	Copy(MeshFileName,OUT_FILE_DIR);
	Copy(BoneStructureFileName,OUT_FILE_DIR);
	Copy(WeigthFileName,OUT_FILE_DIR);
	Copy(PoseFileName,OUT_FILE_DIR);
	Copy(AdjFileName,OUT_FILE_DIR);
	Copy(TextureFileName,OUT_FILE_DIR);
	

	strcpy(MotionFileName,OUT_FILE_DIR);
	strcat(MotionFileName,"\\");
	strcat(MotionFileName,MODEL_NAME);
	strcat(MotionFileName,".motion");

	OFileBuffer Out(MotionFileName);
	Bone::SaveSkullMotion(&Out,Model->BoneRoot);
	Out.Close();
}

void LoadInitialMotion() {
	if (strlen(START_MotionFileName)) {
		IFileBuffer MotionFile(START_MotionFileName);
		if (MotionFile.IsFinished()) {
			cout<<"Could not load "<<START_MotionFileName<<".\n";
			exit(-1);
		}
		Bone::LoadSkullMotion(&MotionFile,Model->BoneRoot);
		cout<<"Motion File:\n -> loaded.\n";
	}
	if (UseLast) {
		char Text[500];
		sprintf(Text,"%s\\%s.motion",OUT_FILE_DIR,MODEL_NAME);
		IFileBuffer MotionFile(Text);
		if (MotionFile.IsFinished()) {
			cout<<"Last Motion File:\n -> not found.\n";
			exit(-1);
		}
		Bone::LoadSkullMotion(&MotionFile,Model->BoneRoot);
		cout<<"Last Motion File:\n -> loaded.\n";
	} else {
		char Text[500];
		sprintf(Text,"%s\\%s.motion",OUT_FILE_DIR,MODEL_NAME);
		if (fileExists(Text)) {
			cout<<"Last Motion File:\n -> found but the -U option is not set.\n -> This process will overwrite the last motion file.\n";
		}
	}
	
}

void SetViewValidMask() {
	int i;
	cout<<endl;

	for(i=0;(i<(int)strlen(VIEW_VALID_MASK))&&(i<num_viste);i++) {
		if (VIEW_VALID_MASK[i]=='0') Vista[i].Enable=0;
		if (VIEW_VALID_MASK[i]=='1') Vista[i].Enable=(VIEW_ENABLE_OPTICAL_FLOW|VIEW_ENABLE_SILHOUETTE);
		if (VIEW_VALID_MASK[i]=='2') Vista[i].Enable=VIEW_ENABLE_OPTICAL_FLOW;
		if (VIEW_VALID_MASK[i]=='3') Vista[i].Enable=VIEW_ENABLE_SILHOUETTE;
	}
	for(i=0;i<num_viste;i++) {
		cout<<"View["<<(i+1)<<"] ";
		if (Vista[i].Enable==0) cout<<"disabled.\n";
		if (Vista[i].Enable==(VIEW_ENABLE_OPTICAL_FLOW|VIEW_ENABLE_SILHOUETTE)) cout<<"enabled.\n";
		if (Vista[i].Enable==VIEW_ENABLE_OPTICAL_FLOW) cout<<"enabled only optical flow data.\n";
		if (VIEW_VALID_MASK[i]==VIEW_ENABLE_SILHOUETTE) cout<<"enabled only silhouette data.\n";
	}
	cout<<endl<<endl;
}
