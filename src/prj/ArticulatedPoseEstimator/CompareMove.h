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






int t, t_start, t_end;
bool saving_video=false;
bool GO=false;
int  what=2;
bool Show_camera=true;
int selected_vertex=-1;
OpenGL_State ogl_state;

int GetMotionFile(char *model_path,int time,int iteration,char *Out) {
	sprintf(Out,"%s\\motion\\L1_%04i_%04i.motion",model_path,time,iteration);
	if (!fileExists(Out)) {
		sprintf(Out,"%s\\motion\\LF_%04i_%04i.motion",model_path,time,iteration);
		if (!fileExists(Out)) {
			iteration=0;
			sprintf(Out,"%s\\motion\\L1_%04i_%04i.motion",model_path,time,iteration);
			if (!fileExists(Out)) return -1;
		}
	}
	return iteration;
}

void MakeOutput(Skinned_Mesh *Model) {
	if (MODEL_NAME[0]!=0) {
		char MotionFileName[300];
		strcpy(MotionFileName,MODEL_DIR);
		strcat(MotionFileName,"\\");
		strcat(MotionFileName,MODEL_NAME);
		strcat(MotionFileName,".motion");

		OFileBuffer Out(MotionFileName);
		Bone::SaveSkullMotion(&Out,Model->BoneRoot);
		Out.Close();
	} else {
		char MotionFileName[300];
		strcpy(MotionFileName,ModelName);
		strcat(MotionFileName,".motion");

		OFileBuffer Out(MotionFileName);
		Bone::SaveSkullMotion(&Out,Model->BoneRoot);
		Out.Close();
	}
}

void SavePictures() {
	static AdvancedRenderWindow *GPU_OBJ     =NULL;
	static ModelRenderer        *ModelRender =NULL;
	static FrameBuffer_struct    Texture_FBO;
	static Bitmap<ColorRGB>     *TextureImage=NULL;

	if (GPU_OBJ==NULL)     GPU_OBJ=new AdvancedRenderWindow(NULL,NULL,NULL,NULL,0x000000,NULL,WS_OVERLAPPEDWINDOW,RENDERWINDOW_DO_NOT_SHOW_INFO);
	if (ModelRender==NULL) {
		ModelRender=new ModelRenderer(GPU_OBJ);
		Texture_FBO=ModelRender->BuildRenderFrameBufferTex(1920,1080);
		TextureImage=new Bitmap<ColorRGB>(1920,1080);
	}
	Model->Apply_Skin(t);
	Model->ComputaNormals();
	
	if (Vista!=NULL) {
		for(int i=0;i<num_viste;i++) {
			char Text[200];
			sprintf(Text,"C:\\Users\\lballan\\Desktop\\FrameCollector\\image%04i_%02i.bmp",t,i+1);
			if (!fileExists(Text)) continue;
			UINT texture=GPU_OBJ->LoadTexture(Text);
			
			double min_z,max_z;
			GetNearFarPlane(Model,&(Vista[i].Ext),min_z,max_z);
			max_z=max_z+max_z*0.001;
			min_z=min_z-min_z*0.001;
			if (min_z<0.0) {
				Warning("One of the cameras intersect the object. Clipping plane set to 0.1.");
				min_z=0.1;
			}
			
			GPU_OBJ->DefineNearFarPlane(min_z,max_z);
			GPU_OBJ->SetFrameBuffer(&Texture_FBO);									
			GPU_OBJ->SetDrawBuffer(0);
			GPU_OBJ->SetReadBuffer(0);
			glDepthMask(1);
			SetStandardOGLEnvironment(true,false,true);		
			GPU_OBJ->SetIE(&(Vista[i].Int),Vista[i].width,Vista[i].height,&(Vista[i].ExtOGL));
			glViewport(0,0,Texture_FBO.w,Texture_FBO.h);
				glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
				glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
				glEnable(GL_MULTISAMPLE);
				glEnable(GL_MULTISAMPLE_ARB);
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glEnable(GL_POINT_SMOOTH);
				glEnable(GL_POLYGON_SMOOTH);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					GPU_OBJ->DrawTexture(texture,1.0,false);

					glEnable(GL_LIGHTING);
					GPU_OBJ->SetCameraCoords();
						glEnable(GL_LIGHT0);
						GLfloat ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
						GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
						GLfloat light  [] = {2,0,0,1.0};
						glLightfv(GL_LIGHT0,GL_POSITION,light);
						glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
						glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
						glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0f);
						glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0f);
						glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0f);
					GPU_OBJ->ClearRT();

					glDisable(GL_CULL_FACE);
					Vector<4> c1;
					//c1[0]=81;c1[1]=131;c1[2]=206;c1[3]=1.0;
					c1[0]=34;c1[1]=108;c1[2]=218;c1[3]=1.0;
					GPU_OBJ->SetMaterial(c1,0.5,2.0,200.0);
					GPU_OBJ->DrawHyperMesh(Model);
			GPU_OBJ->ClearImageCoords();								
			
			GPU_OBJ->RetrieveFrameBuffer(&Texture_FBO,TextureImage);
			GPU_OBJ->ClearFrameBuffer(&Texture_FBO);
			TextureImage->Flip_Vertical();
			
			sprintf(Text,"C:\\Users\\lballan\\Desktop\\FrameCollector\\image%04i_%02iC.bmp",t,i+1);
			TextureImage->Save(Text);
			
			GPU_OBJ->DeleteTexture(texture);
		}
	}
}

int                        selected_collision_detector_level=0;
bool                      *collision_mask=NULL;
HyperMeshPartitionTree    *collision_detector=NULL;
Array<ordered_pair>       *collision_intersecting_faces_buffer=NULL;
bool                       collision_update=false;

void CollisionDetection() {
	if (selected_collision_detector_level==0) {
		cout<<"Intersection Mask:\n";
		collision_mask=LoadIntersectionMask(Model,INTERSECTION_MASK_FILE);
		cout<<"\n";
		float c_fps;collision_detector=HyperMeshPartitionTree::Generate(Model,12,15,collision_mask,c_fps,selected_collision_detector_level);
		cout<<" -> fps: "<<c_fps<<" (Level: "<<selected_collision_detector_level<<")\n\n";
		collision_intersecting_faces_buffer=new Array<ordered_pair>(5000);
	}
	collision_detector->UpdateBoundingBoxes();
	collision_intersecting_faces_buffer->clear();
	collision_detector->SelfIntersection_BinaryTree_Skinned_Mesh(collision_intersecting_faces_buffer,collision_mask);
	collision_update=true;
}


bool coma(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *) {
	static int bone_index=-1;
	static int axis_index=-1;
	static int camera_index=-1;
	static int particle_visualized=0;
	static int iteration_visualized=0;


	switch(uMsg) {
		case WM_LBUTTONDOWN:
			if ((GetKeyState(VK_SHIFT) & 0x8000)) {
				int mx = LOWORD(lParam);
				int my = HIWORD(lParam);
				if (mx & 1 << 15) mx -= (1 << 16);
				if (my & 1 << 15) my -= (1 << 16);
				Vector<2> o,p_;
				o[0]=mx;
				o[1]=my;
				Vector<3> p;
				Vector<3> *point_list=Model->Points.getMem();
				double mind=DBL_MAX;
				selected_vertex=-1;
				for(int i=0;i<Model->num_p;i++,point_list++) {
					p=Project3(&ogl_state,*point_list,true);
					p_[0]=p[0]*ogl_state.ViewPort[2];
					p_[1]=p[1]*ogl_state.ViewPort[3];
					double d=(p_-o).Norm2();
					if (d<30) {
						if (p[2]<mind) {
							mind=p[2];
							selected_vertex=i;
						}
					}
				}
				if (mind>100) selected_vertex=-1;
				cout<<"Selected vertex: "<<selected_vertex<<"      -> "<<Model->Points[selected_vertex]<<"\n";
				
				return true;
			}
			if ((GetKeyState(VK_CONTROL) & 0x8000)) {
				int mx = LOWORD(lParam);
				int my = HIWORD(lParam);
				if (mx & 1 << 15) mx -= (1 << 16);
				if (my & 1 << 15) my -= (1 << 16);
				Vector<2> o,p_;
				o[0]=mx;
				o[1]=my;
				Vector<3> p;
				Vector<3> *point_list=Model->Points.getMem();
				double mind=DBL_MAX;
				int tmp_vtx=-1;
				for(int i=0;i<Model->num_p;i++,point_list++) {
					p=Project3(&ogl_state,*point_list,true);
					p_[0]=p[0]*ogl_state.ViewPort[2];
					p_[1]=p[1]*ogl_state.ViewPort[3];
					double d=(p_-o).Norm2();
					if (d<30) {
						if (p[2]<mind) {
							mind=p[2];
							tmp_vtx=i;
						}
					}
				}
				if (mind>100) tmp_vtx=-1;
				int b_a=Model->GetBoneIDAttachedtoVertex(tmp_vtx,0.0);
				if (b_a!=-1) bone_index=b_a;
				cout<<"Selected -> "<<Model->BoneRefTable[bone_index]->GetName()<<"\n";
				return true;
			}
			break;
		case WM_CHAR:
			wParam=tolower(wParam);
			switch (wParam) {
				case '*': 
					saving_video=!saving_video;
					return true;
				case 'q': 
					{
						t++;
						if (t>t_end) t=t_start;
						Model->Apply_Skin(t);
						Model->ComputaNormals();
						if (GroundTruth_Model) {
							GroundTruth_Model->Apply_Skin(t);
							GroundTruth_Model->ComputaNormals();
						}
						Mondo->Redraw();
						if (saving_video) {
							Bitmap<ColorRGB> X(Mondo->width(),Mondo->height());
							Mondo->RetrieveImage(&X);
							char Text[500];
							sprintf(Text,"img_%04i.png",t);
							X.Save(Text);
						}
						if (collision_update) CollisionDetection();
						cout<<"Displaying frame "<<t<<"\n";
					}
					return true;
				case 'a': 
					{
						t--;
						if (t<t_start) t=t_end;
						Model->Apply_Skin(t);
						Model->ComputaNormals();
						if (GroundTruth_Model) {
							GroundTruth_Model->Apply_Skin(t);
							GroundTruth_Model->ComputaNormals();
						}
						Mondo->Redraw();
						if (saving_video) {
							Bitmap<ColorRGB> X(Mondo->width(),Mondo->height());
							Mondo->RetrieveImage(&X);
							char Text[500];
							sprintf(Text,"img_%04i.png",t);
							X.Save(Text);
						}
						if (collision_update) CollisionDetection();
						cout<<"Displaying frame "<<t<<"\n";
					}
					return true;
				case 'c': 
					if (Vista!=NULL) {
						camera_index++;
						camera_index=camera_index%num_viste;
						Mondo->LockProjectionMatrix(&(Vista[camera_index].Int),1920,1080);
						Mondo->LockInMatrix(&(Vista[camera_index].ExtOGL));
					} else {
						if (fileExists("SavedViewPoint.txt")) {
							IFileBuffer X("SavedViewPoint.txt");
							Matrix ExtOGL(3,4);
							X>>ExtOGL;
							Mondo->LockInMatrix(&ExtOGL);
						}
					}
					return true;
				case 'v': 
					{
						Matrix ExtOGL(3,4);
						Mondo->ReadInitialExtrinsic(&ExtOGL);
						OFileBuffer X("SavedViewPoint.txt");
						X<<ExtOGL;
						X.Close();
					}
					return true;
				case '3': 
					particle_visualized-=2;
					iteration_visualized=0;
				case '2': 
					particle_visualized++;
					iteration_visualized=0;
				case '1': 
					{
						char Model_tmp_dir[500];
						char MotionFileName[500];
						int ret;

						// Try particles
						sprintf(Model_tmp_dir,"%s\\Frame_%06i_P_%02i",MODEL_TMP_DIR,t,particle_visualized);
						ret=GetMotionFile(Model_tmp_dir,t,iteration_visualized,MotionFileName);
						if (ret==-1) {
							if (particle_visualized!=0) {
								sprintf(Model_tmp_dir,"%s\\Frame_%06i_P_%02i",MODEL_TMP_DIR,t,0);
								ret=GetMotionFile(Model_tmp_dir,t,iteration_visualized,MotionFileName);
								if (ret!=-1) {
									cout<<"Particle "<<particle_visualized<<" does not exist. Visualizing particle 0.\n";
									particle_visualized=0;
									iteration_visualized=ret;
								}
							}
							if (ret==-1) {
								int ret=GetMotionFile(MODEL_TMP_DIR,t,iteration_visualized,MotionFileName);
								if (ret==-1) {
									cout<<"no saved iterations for this frame\n";
									iteration_visualized=0;
									return true;
								} else {
									iteration_visualized=ret;
									cout<<"Displaing iteration "<<(iteration_visualized)<<" "<<MotionFileName<<"\n";
								}
							}
						} else {
							iteration_visualized=ret;
							cout<<"Displaing particle "<<particle_visualized<<" at iteration "<<iteration_visualized<<" ("<<MotionFileName<<")\n";
							int ret=GetMotionFile(MODEL_TMP_DIR,t,iteration_visualized,Model_tmp_dir);
							if (ret!=-1) {
								Warning("Non particle iterations are also available for this frame.");
							}
						}

						IFileBuffer MotionFile(MotionFileName);
						Bone::LoadSkullMotion(&MotionFile,Model->BoneRoot);
						Model->Apply_Skin(t);
						Model->ComputaNormals();
						if (GroundTruth_Model) {
							GroundTruth_Model->Apply_Skin(t);
							GroundTruth_Model->ComputaNormals();
						}
						Mondo->Redraw();
						iteration_visualized++;
					}
					return true;
				case 'p':
					{
						bone_index++;
						bone_index=bone_index%(Model->num_bones);
						cout<<"Selected -> "<<Model->BoneRefTable[bone_index]->GetName()<<"\n";
					}
					return true;
				case 'o':
					{
						axis_index++;
						axis_index=axis_index%6;
						cout<<"Selected axis -> "<<(axis_index+1)<<"\n";
					}
					return true;
				case '[':
					{
						if ((bone_index==-1) || (axis_index==-1)) return true;
						Model->BoneRoot->CalcRelativeFromAbsolute(t);
						Bone *x=Model->BoneRefTable[bone_index];
						if (axis_index<3) {
							Vector<3> a;
							a[axis_index]=(M_PI/18)/5.0;
							x->Rotate(t,a);
							Model->BoneRoot->CalcAbsoluteFromRelative(t);
							Model->Apply_Skin(t);
							Model->ComputaNormals();

							Vector<3> Rot,T;
							x->RelativeMotion(t,0,&T,&Rot);
							double ea,eb,ec;
							ea=Rot[0];eb=Rot[1];ec=Rot[2];
							ea=ea*180/M_PI;
							eb=eb*180/M_PI;
							ec=ec*180/M_PI;
							cout<<" "<<ea<<" "<<eb<<" "<<ec<<"\n";
						} else {
							Vector<3> a;
							a[axis_index-3]+=2.0;
							x->Translate(t,a);

							Model->BoneRoot->CalcAbsoluteFromRelative(t);
							Model->Apply_Skin(t);
							Model->ComputaNormals();
						}
						if (collision_update) CollisionDetection();
					}
					return true;
				case '9':
					{
						if (bone_index==-1)return true;
						Bone *x=Model->BoneRefTable[bone_index];
						//Model->BoneRoot->CalcRelativeFromAbsolute(t);
						//Matrix A(4,4);
						//Vector<3> *tmp=x->GetRelTrasformMatrix(t,&A);
						//cout<<tmp[t]<<endl;
						//Model->BoneRoot->CalcAbsoluteFromRelative(t);
						Vector<3> *tmp=x->GetABSTranslationArray();
						tmp[t]=tmp[t-1];
						Matrix **tmp2=x->GetABSRotationArray();
						tmp2[t]->Set(tmp2[t-1]);
						Model->Apply_Skin(t);
						Model->ComputaNormals();
					}
					return true;
				case ']':
					{
						if ((bone_index==-1) || (axis_index==-1)) return true;
						Model->BoneRoot->CalcRelativeFromAbsolute(t);
						Bone *x=Model->BoneRefTable[bone_index];
						if (axis_index<3) {
							Vector<3> a;
							a[axis_index]=-(M_PI/18)/5.0;
							x->Rotate(t,a);
							Model->BoneRoot->CalcAbsoluteFromRelative(t);
							Model->Apply_Skin(t);
							Model->ComputaNormals();
							
							Vector<3> Rot,T;
							x->RelativeMotion(t,0,&T,&Rot);
							double ea,eb,ec;
							ea=Rot[0];eb=Rot[1];ec=Rot[2];
							ea=ea*180/M_PI;
							eb=eb*180/M_PI;
							ec=ec*180/M_PI;
							cout<<" "<<ea<<" "<<eb<<" "<<ec<<"\n";
						} else {
							Vector<3> a;
							a[axis_index-3]-=2.0;
							x->Translate(t,a);
	
							Model->BoneRoot->CalcAbsoluteFromRelative(t);
							Model->Apply_Skin(t);
							Model->ComputaNormals();
						}
						if (collision_update) CollisionDetection();
					}
					return true;
				case '/':
					{
						if ((bone_index==-1) && (selected_vertex==-1)) return true;
						Bone *x=Model->BoneRefTable[bone_index];
						Vector<3> *Translations=x->GetABSTranslationArray();
						Translations[t]=Model->Points[selected_vertex];
						Model->BoneRoot->CalcRelativeFromAbsolute(t);
						Model->Apply_Skin(t);
						Model->ComputaNormals();
					}
					return true;
				case 'è':
					{
						Model->BoneRoot->CalcRelativeFromAbsolute(t);
						cout<<"Model Information:\n";
						Model->BoneRoot->PrintRelativeInformation(t);
						cout<<"-----------------------------\n";
						if (GroundTruth_Model) {
							cout<<"GroundTruth_Model Information:\n";
							GroundTruth_Model->BoneRoot->CalcRelativeFromAbsolute(t);
							GroundTruth_Model->BoneRoot->PrintRelativeInformation(t);
							cout<<"-----------------------------\n";
						}
					}
					return true;
				case '0':
					MakeOutput(Model);
					cout<<"Motion saved.\n";
					return true;
				case 'd':
					{
						if (Vista!=NULL) {
							Model->Apply_Skin(t);
							Model->ComputaNormals();
							DepthMapRenderer DR(Mondo);
							DR.set_empty_pixel_to_infinity=true;

							for(int vista_index=0;vista_index<num_viste;vista_index++) {
								if (Vista[vista_index].width!=0) {
									Bitmap<ColorFloat> depth_bmp(Vista[vista_index].width,Vista[vista_index].height);
									FrameBuffer_struct x=DR.BuildDepthFrameBuffer(Vista[vista_index].width,Vista[vista_index].height);
										
									double min_z,max_z;
									GetNearFarPlane(Model,&(Vista[vista_index].Ext),min_z,max_z);
									max_z=max_z+max_z*0.001;
									min_z=min_z-min_z*0.001;
									if (min_z<0.0) {
										Warning("One of the cameras intersect the object. Result may be inaccurate.");
										min_z=0.1;
									}
									double cm=Mondo->clip_min();
									double cM=Mondo->clip_max();
									Mondo->DefineNearFarPlane(min_z,max_z);
									
									DR.RenderDepthMap(&x,Vista[vista_index].width,Vista[vista_index].height,&(Vista[vista_index].Int),&(Vista[vista_index].ExtOGL),&(Vista[vista_index].Ext),Model,&depth_bmp);
									Mondo->DeleteFrameBuffer(&x);
									char filename[500];
									sprintf(filename,"CameraD%02i_%04i.float",vista_index+1,t);
									depth_bmp.Flip_Vertical();
									depth_bmp.Save(filename);
									Mondo->DefineNearFarPlane(cm,cM);
								}
							}
						}
					}
					return false;
				case 's':
					if (Vista!=NULL) {
						SavePictures();
					} else {
						Bitmap<ColorRGB> tmp_image(Mondo->width(),Mondo->height());
						Mondo->RetrieveImage(&tmp_image);
						char name[500];
						static int image_index=1;
						do {
							sprintf(name,"C:\\Users\\lballan\\Desktop\\FrameCollector\\image_%02i.png",image_index);
							image_index++;
						} while (fileExists(name)==true);
						tmp_image.Save(name);
						cout<<"saved "<<name<<endl;
					}
					return false;
				case 'l':
					{
						Matrix Ext(3,4);
						Matrix E(3,4);
						Vector<3> tmp;
						Mondo->ReadInitialExtrinsic(&Ext);
						E.Set(&Ext);
						Invert4x4RotoTraslationMatrix(&E);
						E.GetColumn(0,tmp.GetList());
						tmp=-1.0*tmp;
						E.SetColumn(0,tmp.GetList());
						
						DepthMapRenderer DR(Mondo);
						int width=Mondo->width();
						int height=Mondo->height();
						FrameBuffer_struct fb=DR.BuildDepthFrameBuffer(width,height);
						Bitmap<ColorFloat> depth(width,height);
						Matrix Int(3,3);
						Int.SetIdentity();
						double f=1/(2*tan((45.0*M_PI/180)/2));
						Int[0][0]=f*height;
						Int[1][1]=f*height;
						Int[0][2]=width/2.0;
						Int[1][2]=height/2.0;
						DR.RenderDepthMap(&fb,width,height,&Int,&Ext,&E,Model,&depth);
						depth.Flip_Vertical();
						Mondo->DeleteFrameBuffer(&fb);
						Int.Print("I");
						E.Print("E");
						IMAGE_SAVE_I((&depth),"Depth_%04i.float",t);
						IMAGE_SAVE_I((&depth),"Depth_%04i.png",t);
					}
					return true;
				case 'w':
					{
						what=(what+1)%3;
					}
					return true;
				case '-':
					{
						if (collision_update) {
							collision_update=false;
						} else {
							CollisionDetection();
							collision_update=true;
						}
					}
					return true;
				case '\\':
					{
						PostQuitMessage(0);
					}
					return true;
				case '5':
					{
						Show_camera=!Show_camera;
					}
					return true;
			
			}
			return false;
	};
	return false;
}




void display(void *) {
	SetStandardOGLEnvironment(true,false,false);
	Mondo->SetLocalCoords();

	glEnable(GL_LIGHTING);
	Mondo->SetCameraCoords();
		glEnable(GL_LIGHT0);
	    GLfloat ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
		GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		GLfloat light  [] = {2,0,0,1.0};
		glLightfv(GL_LIGHT0,GL_POSITION,light);
	    glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
		glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0f);
		glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0f);
	    glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0f);
	Mondo->ClearRT();
	
	glDisable(GL_CULL_FACE);

	if ((Vista!=NULL) && (Show_camera))
		for(int i=0;i<num_viste;i++) Mondo->DrawCameraObject(&Vista[i].Ext,i+1,model_radius);


	if ((what==0) || (what==2)) {
		Vector<4> c1;
		c1[0]=81;c1[1]=131;c1[2]=206;c1[3]=1.0;
		bool flat=false;
		Mondo->SetMaterial(c1);
		{
			HyperFace<3> *f=Model->Faces.getMem();
			Vector<3> *P=Model->Points.getMem();
			Vector<3> *N=Model->Normals.getMem();
			
			glBegin(GL_TRIANGLES);
			for (int i=0;i<Model->num_f;i++,f++) {
				// TODOLUCA
				int b_a=Model->GetBoneIDAttachedtoVertex(f->Point[0],0.0);
				if (b_a!=-1) {
					Bone *tmpBone=Model->BoneRefTable[b_a];
					if ((strncmp(tmpBone->GetName(),"Bone",4)==0) || (strncmp(tmpBone->GetName(),"Paper",5)==0)) {
						glEnd();
						c1[0]=206;c1[1]=131;c1[2]=81;c1[3]=1.0;
						Mondo->SetMaterial(c1);
						flat=true;
						if (strncmp(tmpBone->GetName(),"BoneBall",8)==0) {
							c1[0]=0;c1[1]=131;c1[2]=81;c1[3]=1.0;
							Mondo->SetMaterial(c1);
							flat=false;
						}
						if (strncmp(tmpBone->GetName(),"Bone0",5)==0) {
							c1[0]=0;c1[1]=131;c1[2]=81;c1[3]=1.0;
							Mondo->SetMaterial(c1);
							flat=false;
						}
						if (strncmp(tmpBone->GetName(),"Paper",5)==0) {
							c1[0]=206;c1[1]=131;c1[2]=81;c1[3]=1.0;
							Mondo->SetMaterial(c1);
							flat=true;
						}
						glBegin(GL_TRIANGLES);
					} else {
						if (flat) flat=false;
					}
				}
				// TODOLUCA
				

				if (!flat) glNormal3dv(N[f->Point[0]].GetList());
				else glNormal3dv(f->Normal(&Model->Points).GetList());
				Mondo->Vertex(P[f->Point[0]]);
				if (!flat) glNormal3dv(N[f->Point[1]].GetList());
				Mondo->Vertex(P[f->Point[1]]);
				if (!flat) glNormal3dv(N[f->Point[2]].GetList());
				Mondo->Vertex(P[f->Point[2]]);
			}
			glEnd();
		}


		c1[0]=0;c1[1]=255;c1[2]=0;c1[3]=1.0;
		Mondo->SetMaterial(c1);
		if (GroundTruth_Model) Mondo->DrawHyperMesh(GroundTruth_Model);

		// select vertex
		ReadOpenGLState(&ogl_state);
		if (selected_vertex!=-1) {
			glEnable(GL_COLOR_MATERIAL);
			glColor3f(1.0,0.0,0.0);
			glDisable(GL_LIGHTING);
			Vector<3> *point_list=Model->Points.getMem();
			for(int i=0;i<Model->num_p;i++,point_list++) {
				int b_a=Model->GetBoneIDAttachedtoVertex(i,0.0);
				if (b_a!=-1) {
					Bone *tmpBone=Model->BoneRefTable[b_a];
					//if (tmpBone->Level<=2) {
					//	Mondo->DrawPoint(Model->Points[i],0x00FF00,10.0);
					//} else 
					Mondo->DrawPoint(Model->Points[i],0x000000,3.0);
				} else Mondo->DrawPoint(Model->Points[i],0x000000,3.0);
			}
			Mondo->DrawPoint(Model->Points[selected_vertex],0xFF00FF,6.0);
			glEnable(GL_LIGHTING);
		}

		if (SalientVertices) {
			for(int i=0;i<SalientVertices->numElements();i++) {
				Mondo->DrawPoint(Model->Points[(*SalientVertices)[i]],0xFF00FF,6.0);
			}
		}
	}
	if (what>=1) {
		glEnable(GL_COLOR_MATERIAL);
		glColor3f(1.0,0.0,0.0);
		Model->BoneRoot->DrawAll(t,Mondo);
	}

	if (collision_update) {
		int num_collisions=collision_intersecting_faces_buffer->numElements();
		HyperFace<3> *f=Model->Faces.getMem();
		Vector<3> *P=Model->Points.getMem();
		
		Vector<4> c1;
		c1[0]=0;c1[1]=131;c1[2]=81;c1[3]=1.0;
		Mondo->SetMaterial(c1);

		Vector<3> a,b,c,d;
		c[0]=c[1]=c[2]=1.0;
		for(int i=0;i<num_collisions;i++) {
			a=P[f[(*collision_intersecting_faces_buffer)[i].i].Point[0]];
			b=P[f[(*collision_intersecting_faces_buffer)[i].j].Point[0]];
			double r=(a-b).Norm2();
			d=r*c;
			Mondo->DrawObj(0,&a,NULL,&d,10,10,1.0);
		}
	}

	//Vector<3> P;
	//P[0]=41.930349; 
	//P[1]=-8.241522; 
	//P[2]=214.456442;
	//Mondo->DrawPoint(P,0xFF00FF,6.0);
}












int ParseInputData(int argc,char* argv[],int &t_start,int &t_end,bool &filter_data,bool &save_filter_data,bool &use_texture);

#define BMP_TEST_SEQ(path,post_fix)			sprintf(path,"%s%s%02i_%04i.bmp",Image_File,post_fix,i+1,0);	\
											if (!fileExists(path)) {										\
												sprintf(path,"%s%s%02i.avi",Image_File,post_fix,i+1);		\
												if (!fileExists(path)) sprintf(path,"");					\
											}

void GetCameraResolution(char *Image_File,View *Views,int num_views) {
	ViewStreamer **VStream=new ViewStreamer*[num_views];

	char path_mask[FILE_PATH_SIZE];
	char path_depth[FILE_PATH_SIZE];
	char *paths[]={"",path_mask,path_depth,NULL};

	for(int i=0;i<num_views;i++) {
		Views[i].SetChannelTypes("CGF");
		BMP_TEST_SEQ(path_mask,"");
		BMP_TEST_SEQ(path_depth,"D");
		VStream[i]=new ViewStreamer(&(Views[i]),paths);
	}

	for(int i=0;i<num_views;i++) SDELETE(VStream[i]);
	SDELETE(VStream);
}
void LoadCamerasMatrices(char *FileName) {
	IFileBuffer In(FileName);
	if (In.IsFinished()) {
		cout<<FileName<<" not found.\n";
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
}






























//bool *intersection_mask=NULL;
//HyperMeshPartitionTree *collision_tree=NULL;
//Array<ordered_pair> *intersecting_faces;
//void color_face(RenderWindow<3> *w,HyperMesh<3> *mesh,int face_index,void *data) {
//	bool intersect=false;
//	for(int i=0;i<mesh->num_f;i++) {
//		/*if (i==face_index) continue;
//		
//		bool is_neigh=false;
//		for(int j=0;j<mesh->NumNeighbourFaces[face_index];j++) {
//			if (mesh->NeighbourFaces[face_index][j]==i) {
//				is_neigh=true;
//				break;
//			}
//		}
//		if (is_neigh) continue;
//
//		for(int j=0;j<3;j++) {
//			for(int q=0;q<3;q++) {
//				if (mesh->Faces[i].Point[j]==mesh->Faces[face_index].Point[q]) {
//					is_neigh=true;
//					break;
//				}
//			}
//		}
//		if (is_neigh) continue;
//
//		// Test with two triangles
//		intersect=Intersection_TriangleTriangle_v1(mesh,face_index,mesh,i);
//		if (intersect) break;*/
//
//		intersect=Intersection_Mesh(mesh,face_index,i);
//		if (intersect) break;
//	}
//	if (intersect) w->SetColor(0x00FF00,1.0);
//	else w->SetColor(0x0000FF,1.0);
//}
//void display(void *) {
//	SetStandardOGLEnvironment(true,false,false);
//	Mondo->SetLocalCoords();
//
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//    float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
//    float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
//    glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
//    glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
//	Mondo->SetCameraCoords();
//	GLfloat light[4]={2,0,0,1.0};
//	glLightfv(GL_LIGHT0,GL_POSITION,light);
//	Mondo->ClearRT();
//
//	glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0f);
//    glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0f);
//    glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0f);
//	for(int i=0;i<num_viste;i++) Mondo->DrawCameraObject(&Vista[i].Ext,i+1,model_radius);
//
//
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//	Vector<4> c1;
//	c1[0]=81;c1[1]=131;c1[2]=206;c1[3]=1.0;
//	Mondo->SetMaterial(c1);
//
//
//	
//	//Vector<3> light_pos;light_pos[0]=2;
//	//Mondo->SetLight(0,light_pos,true,1.0f,0.0f);
//
//	if (Lateral) {
//		if (GroundTruth_Model) {
//			if (!OnlyRoot) Mondo->DrawHyperMesh(GroundTruth_Model);
//			else GroundTruth_Model->BoneRoot->DrawAll(t,Mondo);
//		}
//		Vector<4> c2;
//		c2[0]=255;c2[1]=0;c2[2]=0;
//		c2[3]=1.0;
//		Mondo->SetMaterial(c2);
//		Vector<3> T;
//		T[0]=20;
//		Mondo->SetRT(NULL,&T);
//		if (!OnlyRoot) {
//			if ((use_texture) && (disp_texture)) {
//				glDisable(GL_COLOR_MATERIAL);
//				glDisable(GL_LIGHTING);
//				glEnable(GL_LIGHTING);
//				Mondo->DrawHyperMesh_Textured(Model);
//				glEnable(GL_LIGHTING);
//				glDisable(GL_TEXTURE_2D);
//			} else Mondo->DrawHyperMesh(Model);
//		} else {
//			Model->BoneRoot->DrawAll(t,Mondo);
//		}
//		Mondo->ClearRT();
//		return;
//	}
//
//
//	// Draw GroundTruth_Model
//	if (GroundTruth_Model) {
//		if ((what==0) || (what==1)) {
//			if (!OnlyRoot) Mondo->DrawHyperMesh(GroundTruth_Model);
//			else GroundTruth_Model->BoneRoot->DrawAll(t,Mondo);
//		}
//	}
//
//	if ((Pavimento) && (disp_floor)) {
//		Vector<4> c2;
//		c2[0]=0;c2[1]=0;c2[2]=255;c2[3]=1.0;
//		Mondo->SetMaterial(c2);
//		glEnable(GL_COLOR_MATERIAL);
//		glDisable(GL_LIGHTING);
//		glColor3f(0.0,0.0,1.0);
//		if (floor_tex) {
//			glColor3f(1.0,1.0,1.0);
//			glDisable(GL_COLOR_MATERIAL);
//			glDisable(GL_LIGHTING);
//			Mondo->DrawHyperMesh_Textured_Flat(Pavimento);
//			glEnable(GL_LIGHTING);
//			glDisable(GL_TEXTURE_2D);
//		} else Mondo->DrawHyperMesh(Pavimento);
//
//		/*glDisable(GL_COLOR_MATERIAL);
//		glColor3f(1.0,1.0,1.0);
//		glEnable(GL_LIGHTING);
//
//		Mondo->InitCastShadow();
//		
//		GLfloat LightPosition[4]={-870.520447,-4420.012695,974.711914};//-1210.724609,-1872.480591,2421.396729};
//		/*Vector<3> light_pos;
//		light_pos[0]=-2534.876465;
//		light_pos[1]=-2971.726563;
//		light_pos[2]=3940.583252;
//		Mondo->CastShadow(Model,1,light_pos,6000);
//
//		light_pos[0]=303.867676;
//		light_pos[1]=-3042.960938;
//		light_pos[2]=3903.585693;
//		Mondo->CastShadow(Model,1,light_pos,6000);
//
//		light_pos[0]=244.795441;
//		light_pos[1]=-711.951416;
//		light_pos[2]=814.850342;
//		Mondo->CastShadow(Model,1,light_pos,6000);
//
//		light_pos[0]=-2449.736816;
//		light_pos[1]=-635.115601;
//		light_pos[2]=837.740295;
//		Mondo->CastShadow(Model,1,light_pos,6000);
//		*//*
//		Mondo->CastShadow(Model,1,LightPosition,2000);
//
//		Mondo->Pass1_DeclareShadowOn(Pavimento);
//		//Mondo->Pass1_DeclareShadowOn(Model);
//		Mondo->Pass2_RenderShadow();
//
//				glColor4f(.2,.2,.2,0.5);
//				glEnable(GL_COLOR_MATERIAL);
//				glDisable(GL_LIGHTING);
//				glDisable(GL_TEXTURE_2D);
//				Mondo->DrawHyperMesh_Flat(Pavimento);
//				//Mondo->DrawHyperMesh(Model);
//				glEnable(GL_LIGHTING);
//
//		Mondo->Pass3_Close();
//		glColor4f(1.0,1.0,1.0,1.0);*/
//	}
//
//
//	
//	glColor3f(1.0,0.0,0.0);
//	glDisable(GL_COLOR_MATERIAL);
//	Vector<4> c2;
//	c2[0]=255;c2[1]=0;c2[2]=0;
//	if (what!=0) c2[3]=1.0;
//	else c2[3]=.5;
//	Mondo->SetMaterial(c2);
//	glEnable(GL_LIGHTING);
//
//
//	if ((what==0) || (what==2)) {
//		glEnable(GL_DEPTH_TEST);
//		glDepthFunc(GL_ALWAYS);
//		glDepthMask(1);
//		glDepthRange(1,1);
//		glColorMask(0,0,0,0);
//		if (GroundTruth_Model) {
//			if (!OnlyRoot) Mondo->DrawHyperMesh(GroundTruth_Model);
//			else GroundTruth_Model->BoneRoot->DrawAll(t,Mondo);
//		}
//		glColorMask(1,1,1,1);
//		glEnable(GL_DEPTH_TEST);
//		glDepthFunc(GL_LEQUAL);
//		glDepthMask(1);
//		glDepthRange(0,1);
//		if (!OnlyRoot) {
//			if ((use_texture) && (disp_texture)) {
//				glDisable(GL_COLOR_MATERIAL);
//				glEnable(GL_LIGHTING);
//				
//				Vector<4> c2;
//				c2[0]=255;c2[1]=255;c2[2]=255;
//				c2[3]=1.0;
//				Mondo->SetMaterial(c2);
//
//				//Vector<4> c2;
//				c2[0]=255;c2[1]=255;c2[2]=255;
//				c2[3]=1.0;
//				Mondo->SetMaterial(c2);
//				
//				Vector<3> light_pos;
//				light_pos[0]=-1210.724609;
//				light_pos[1]=-1872.480591;
//				light_pos[2]=2421.396729;
//				Mondo->SetLight(0,light_pos,false,1.0f,0.0f);
//
//				light_pos[0]=-2534.876465;
//				light_pos[1]=-2971.726563;
//				light_pos[2]=3940.583252;
//				Mondo->SetLight(1,light_pos,false,1.0f,0.0f);
//
//				light_pos[0]=303.867676;
//				light_pos[1]=-3042.960938;
//				light_pos[2]=3903.585693;
//				Mondo->SetLight(2,light_pos,false,1.0f,0.0f);
//
//				light_pos[0]=244.795441;
//				light_pos[1]=-711.951416;
//				light_pos[2]=814.850342;
//				Mondo->SetLight(3,light_pos,false,1.0f,0.0f);
//
//				light_pos[0]=-2449.736816;
//				light_pos[1]=-635.115601;
//				light_pos[2]=837.740295;
//				Mondo->SetLight(4,light_pos,false,1.0f,0.0f);
//
//				glDisable(GL_COLOR_MATERIAL);
//				glColor3f(1.0,1.0,1.0);
//				if (!use_light) glDisable(GL_LIGHTING);
//				
//				
//				Mondo->DrawHyperMesh_Textured(Model);
//				//Mondo->DrawHyperMesh(Model);
//				//glEnable(GL_LIGHTING);
//				//glDisable(GL_TEXTURE_2D);
//			} else {
//				Mondo->SetLightOff(1);
//				Mondo->SetLightOff(2);
//				Mondo->SetLightOff(3);
//				Mondo->SetLightOff(4);
//
//				c1[0]=81;c1[1]=131;c1[2]=206;c1[3]=1.0;
//				Mondo->SetMaterial(c1);
//				Mondo->DrawHyperMesh(Model);
//
//				// TODO_TMP
//				/*
//				SetStandardOGLEnvironment(true,false,false);
//				static HyperMesh<3> *TMP_MODEL=NULL;
//				if (!TMP_MODEL) {
//					TMP_MODEL=new HyperMesh<3>();
//					//TMP_MODEL->Read("Balls.objbin");
//
//					intersection_mask=LoadIntersectionMask(Model,"IntersectionMask.txt");
//					for(int i=0;i<Model->num_bones;i++) {
//						cout<<Model->BoneRefTable[i]->GetName()<<" ";
//					}
//					cout<<"\n";
//					for(int i=0;i<Model->num_bones;i++) {
//						cout<<Model->BoneRefTable[i]->GetName()<<" ";
//						for(int j=0;j<Model->num_bones;j++) {
//							cout<<intersection_mask[i+(j*Model->num_bones)]<<" ";
//						}
//						cout<<"\n";
//					}
//					float c_fps;
//					collision_tree=HyperMeshPartitionTree::Generate(Model,7,15,intersection_mask,c_fps);
//					cout<<c_fps<<"\n";
//					intersecting_faces=new Array<ordered_pair>(Model->num_f);
//				}
//
//					collision_tree->UpdateBoundingBoxes();
//					intersecting_faces->clear();
//					collision_tree->SelfIntersection_BinaryTree_Skinned_Mesh(intersecting_faces,intersection_mask);
//
//					Mondo->SetColor(0x0000FF,1.0);
//					//Mondo->DrawHyperMesh(Model);
//					Mondo->SetColor(0x00FF00,1.0);
//					glDepthFunc(GL_LEQUAL);
//					
//					Vector<3> *P=Model->Points.getMem();
//					Vector<3> *N=Model->Normals.getMem();
//					glBegin(GL_TRIANGLES);
//					for(int i=0;i<intersecting_faces->numElements();i++) {
//						HyperFace<3> *f=&(Model->Faces[(*intersecting_faces)[i].j]);
//						
//						glNormal3dv(N[f->Point[0]].GetList());
//						Mondo->Vertex(P[f->Point[0]]);
//						glNormal3dv(N[f->Point[1]].GetList());
//						Mondo->Vertex(P[f->Point[1]]);
//						glNormal3dv(N[f->Point[2]].GetList());
//						Mondo->Vertex(P[f->Point[2]]);
//						
//						f=&(Model->Faces[(*intersecting_faces)[i].i]);
//						glNormal3dv(N[f->Point[0]].GetList());
//						Mondo->Vertex(P[f->Point[0]]);
//						glNormal3dv(N[f->Point[1]].GetList());
//						Mondo->Vertex(P[f->Point[1]]);
//						glNormal3dv(N[f->Point[2]].GetList());
//						Mondo->Vertex(P[f->Point[2]]);
//					}
//					glEnd();
//
//				{
//					conic_f distance_function;
//					collision_constraint                      tmp;
//					HyperFace<3>                                *F=Model->Faces.getMem();
//					Vector<3>                                   *P=Model->Points.getMem();
//
//					for(int j=0;j<intersecting_faces->numElements();j++) {
//						int f1=(*intersecting_faces)[j].i;
//						int f2=(*intersecting_faces)[j].j;
//						for(int d=0;d<intersecting_faces->numElements();d++) {
//							if (d==j) continue;
//							if ((*intersecting_faces)[j]==(*intersecting_faces)[d]) {
//								cout<<"doppia\n";
//							}
//							if ((*intersecting_faces)[j].i==(*intersecting_faces)[d].j) {
//								if ((*intersecting_faces)[j].j==(*intersecting_faces)[d].i) {
//									cout<<"invert\n";
//								}
//							}
//						}
//
//						tmp.i_face=f2;
//						tmp.dir=F[f2].Normal(&(Model->Points));
//						tmp.o=F[f2].Baricentro(&(Model->Points));
//						tmp.r=0.0;
//						for(int q=0;q<3;q++) tmp.r=max(tmp.r,(P[F[f2].Point[q]]-tmp.o).Norm2());
//						for(int q=0;q<3;q++) {
//							tmp.vertex_index=F[f1].Point[q];
//								if ((P[tmp.vertex_index]-tmp.o)*tmp.dir<0.0) {
//									distance_function.Set(0.5);
//									Vector<3> d_part_1=distance_function.DF(P[tmp.vertex_index],tmp.o,tmp.dir,tmp.r);
//									Mondo->DrawArrow(P[tmp.vertex_index],P[tmp.vertex_index]-d_part_1,ZERO_3,0xFF0000);
//								}
//						}
//						tmp.i_face=f1;
//						tmp.dir=F[f1].Normal(&(Model->Points));
//						tmp.o=F[f1].Baricentro(&(Model->Points));
//						tmp.r=0.0;
//						for(int q=0;q<3;q++) tmp.r=max(tmp.r,(P[F[f1].Point[q]]-tmp.o).Norm2());
//						for(int q=0;q<3;q++) {
//							tmp.vertex_index=F[f2].Point[q];
//								if ((P[tmp.vertex_index]-tmp.o)*tmp.dir<0.0) {
//									distance_function.Set(0.5);
//									Vector<3> d_part_1=distance_function.DF(P[tmp.vertex_index],tmp.o,tmp.dir,tmp.r);
//									Mondo->DrawArrow(P[tmp.vertex_index],P[tmp.vertex_index]-d_part_1,ZERO_3,0xFF0000);
//								}
//						}
//					}
//				}
//				*/
//				//Mondo->DrawHyperMesh(Model,color_face,NULL);
//				
//				/*Mondo->SetColor(0xFFFFFF,1.0);
//				glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
//				Mondo->DrawHyperMesh(Model);*/
//				//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
//				//
//			}
//		} else {
//				Mondo->SetLightOff(1);
//				Mondo->SetLightOff(2);
//				Mondo->SetLightOff(3);
//				Mondo->SetLightOff(4);
//			glEnable(GL_COLOR_MATERIAL);
//			glColor3f(1.0,0.0,0.0);
//			Model->BoneRoot->DrawAll(t,Mondo);
//		}
//	}
//
//	
//	
//	
//	if (Vista!=NULL) 
//		for(int i=0;i<num_viste;i++) Mondo->DrawCameraObject(&Vista[i].Ext,i+1,model_radius);
//	
//	
//	
//	// Floor
//
//}
