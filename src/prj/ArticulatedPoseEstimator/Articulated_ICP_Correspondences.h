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






#define BORDER_LASCO 5


void Articulated_ICP::RetrieveObservations(int Level,int frame_index) {

	// Delete list
	TargetList_1D->clear();
	TargetList_2D->clear();
	TargetList_3D->clear();
	

	// Get Observations
	if (show_debug_information) cout<<"-> Correspondences\n";
	TargetList_2D->append(Feat_TargetList);
	TargetList_3D->append(TargetList_3D_ContactPoints);
	for(int i=0;i<num_views;i++) {
		ResetBufferStatus();
		
		GetSilhouettesCorrespondences(i,Level);
		GetEdgesCorrespondences(i,Level);
		GetDepthCorrespondences(i,Level);
		GetSalientPointCorrespondences(i,Level,frame_index);
		//GetTextureCorrespondences(i,Level);
	}

	// Computa collisioni (skin is ready from the previous calls) (do not do anything if they are not enabled)
	// NOTE: IK_Solver->SetCollisionParameters -> can be called somewhere before this next calls to set the new weights or other parameters
	//       or UpdateCollisionWeights
	if (Level==0) {
		if (IK_Solver->GetCollisionTTL()==0) {
			IK_Solver->Clear_Collision_Constraints();
			IK_Solver->Add_Collision_Constraints();
		} else {
			IK_Solver->Collision_Increase_Time();
			IK_Solver->Add_Collision_Constraints();
		}
	} else {
		if (IK_Solver_Pyramid_Level_1->GetCollisionTTL()==0) {
			IK_Solver_Pyramid_Level_1->Clear_Collision_Constraints();
			IK_Solver_Pyramid_Level_1->Add_Collision_Constraints();
		} else {
			IK_Solver_Pyramid_Level_1->Collision_Increase_Time();
			IK_Solver_Pyramid_Level_1->Add_Collision_Constraints();
		}
	}


	if (show_minimal_debug_information) ShowCorrespondencesInfo(Level);
}
























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Silhouettes ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::GetSilhouettesCorrespondences(int view,int Level) {
	View *View = &(Views[view]);
	int p1_i[2];
	Vector<2> p1,p2,p3,dir;


	if (View->Mask==NULL) return;
	if ((EdgesSource&0x1) && (Views[view].Edges!=NULL)) return;
	if (!(View->Enable&VIEW_ENABLE_SILHOUETTE)) {
		if (show_debug_information) cout<<"     Silhouette:     0 (not active)\n";
		return;
	}
	


	RenderSilhouetteImage(view);


	// Set Find gradient
	ui_FindGradient_struct ui_FindGradient;
	ui_FindGradient.image=View->Mask;
	ui_FindGradient.soglia_tolleranza_gradiente=gradient_tollerance;
	ui_FindGradient.Gradient_Window_Size=GRADIENT_WINDOW_SIZE;

	// 
	int saveIndex=TargetList_2D->numElements();

	//
	// Get Contour
	int num=GetMeshContourVertex(View,mesh,CorrVertexList,SilhouetteImage,buffer_status.min_z_plane);
	

	
	// Definisco:
	// ContourMask = Current Estimate of the Model viewed by this
	// Mask        = Immagine target
	// 

	for(int i=0;i<num;i++) {
		// p1 is the current point in the image viewed by this
		View->ProjectionToImage(mesh->Points[CorrVertexList[i]],p1_i);
			p1[0]=p1_i[0];
			p1[1]=p1_i[1];

		// dir is the direction of search definited by the current model estimate
		dir=ImageGradient(p1_i[0],p1_i[1],SilhouetteImage,(GreyLevel)0x00,GRADIENT_WINDOW_SIZE);
		dir=dir.Versore();
		
		// Affermo che il gradiente da ricercare nella seconda immagine (immagine target) è quello calcolato in p1 della prima immagine (current model estimate)
		ui_FindGradient.Gradient_To_Search=dir;				// NB: after dir will be swapped
		
		// Cerchero' lungo da p1 lungo +dir e -dir un edge nella seconda immagine il cui gradiente sia simile
		// a quello definito qui sopra.
		float df=-1,db=-1;
		if (CheckSegment(p1,dir,max_length_search,(CK_SEG_CALLBACK)FindGradient,&ui_FindGradient)) {
			// Trovato lungo +dir in p2 a distanza df
			p2=ui_FindGradient.finded_point;
			df=(float)((p2-p1).Norm2());
			// ASSURDO: Scarto se nelle vicinanza di p2 vi è presente parte della stima corrente del modello
			// In tal caso infatti nn si potrebbe mai tornare indietro <- Meglio asserire se vi è un gradiente della stima corrente!!!!!
			// ovvero che gradiente != 0
			// if (IsLocallyForeground(p2[0],p2[1],ContourMask,0x00,3)) df=-1;
		}
		if (CheckSegment(p1,-1.0*dir,max_length_search,(CK_SEG_CALLBACK)FindGradient,&ui_FindGradient)) {
			// Trovato lungo -dir in p3 a distanza db
			p3=ui_FindGradient.finded_point;
			db=(float)((p3-p1).Norm2());
			// ASSURDO: Scarto se nelle vicinanza di p2 vi è presente parte della stima corrente del modello
			// if (IsLocallyForeground(p3[0],p3[1],ContourMask,0x00,3)) db=-1;
		}
		if (db==-1) {
			if (df==-1) {
				continue;
			}
		} else {
			if (df==-1) {
				df=db;
				p2=p3;
			} else {
				if (db<df) {
					df=db;
					p2=p3;
				}
			}
		}
		// Now, the movement is:
		// p1 --> p2 (df is the distance)

		// Scarto se è troppo corto (<2 pixel) TODO ??? Sicuro??? TODO ??? 
		// if (df<2) df=-1;

		TargetList_2D_Item_type tmp;
		tmp.VertexIndex=CorrVertexList[i];
		tmp.View=View;
		tmp.Source=p1;
		tmp.Target=p2;
		tmp.sqrt_weight=1.0f*((float)(mesh->AreaPoint[tmp.VertexIndex]));
		tmp.type=0;

		TargetList_2D->append(tmp);
	}


	// Outliers and Output
	int old_corresp_num=TargetList_2D->numElements();
	Clean_TargetList_2D_FromOutliers(saveIndex);
	if (show_debug_information) cout<<"     Silhouette:     "<<(TargetList_2D->numElements()-saveIndex)<<" (outliers "<<(old_corresp_num-TargetList_2D->numElements())<<")\n";
}





















































































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Edges ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define AICP_INT_EDGE_CHECK(CONDITION)	\
	for(int i=-d+x;i<=x+d;i++) {		\
		for(int j=-d+y;j<=y+d;j++) {	\
			if (CONDITION) {			\
				if (i-x<0) {			\
					dir[0]+=x-i;		\
					dir[1]+=y-j;		\
				} else {				\
					dir[0]+=i-x;		\
					dir[1]+=j-y;		\
				}						\
			}							\
		}								\
	}

#define AICP_EXT_EDGE_CHECK(CONDITION)	\
	for(int i=-d+x;i<=x+d;i++) {		\
		for(int j=-d+y;j<=y+d;j++) {	\
			if (CONDITION) {			\
				dir[0]+=i-x;			\
				dir[1]+=j-y;			\
			}							\
		}								\
	}

Vector<2> GetInternalEdgeDirection(int x,int y,Bitmap<ColorFloat> *im,float edge_threshold_r,int d) {
	Vector<2> dir;
	
	if (im->isOutside(x,y,d)) {
		AICP_INT_EDGE_CHECK(im->Point(i,j)>edge_threshold_r);
	} else {
		AICP_INT_EDGE_CHECK(im->Point_NCB(i,j)>edge_threshold_r);
	}

	return dir.Versore();
}

Vector<2> GetExternalEdgeDirection(int x,int y,Bitmap<ColorFloat> *im,int d) {
	Vector<2> dir;

	if (im->isOutside(x,y,d)) {
		AICP_EXT_EDGE_CHECK(im->Point(i,j)==FLT_MAX);
	} else {
		AICP_EXT_EDGE_CHECK(im->Point_NCB(i,j)==FLT_MAX);
	}
	
	return dir.Versore();
}

Vector<2> GetExternalEdgeDirection_NCB(int x,int y,Bitmap<GreyLevel> *im,GreyLevel bg_value,int d) {
	Vector<2> dir;

	AICP_EXT_EDGE_CHECK(im->Point_NCB(i,j)==bg_value);
	
	return dir.Versore();
}

Vector<2> GetInternalEdgeDirection_NCB(int x,int y,Bitmap<GreyLevel> *im,GreyLevel edge_threshold_r,int d) {
	Vector<2> dir;
	
	AICP_INT_EDGE_CHECK(im->Point_NCB(i,j)>edge_threshold_r);

	return dir.Versore();
}


bool Articulated_ICP::FindCoherentEdge(CK_SEG_INFO_STRUCT *Info,EdgeSearch_struct *ui_Info) {
	
	if (Info->t==0) return false;				// TODOEDGE Scarta sempre il primo (nel caso sia quello giusto uso P_before)
	int Px=(int)((Info->P)[0]);					// gia' approximato ad intero (basta il cast ora)
	int Py=(int)((Info->P)[1]);
	
	Bitmap<GreyLevel> *mask=ui_Info->mask;
	Bitmap<GreyLevel> *edge_map=ui_Info->edge_map;


	if (mask->isOutside(Px,Py,ui_Info->win_d_for_input_mask)) {
		if (((Px>=mask->width) && (Info->direction[0]>0)) ||				// TODOEDGE Funziona???
		    ((Px<0) && (Info->direction[0]<0)) ||
		    ((Py<0) && (Info->direction[1]<0)) ||
			((Py>=mask->height) && (Info->direction[1]>0))) {
			ui_Info->target_type=-1;	
			return true;
		}
		return false;
	}
	

	// Controllo se e' un edge o bordo esterno
	//    -> se assumo che tutti i bordi esterni vengano detectati come edges) 
	if (edge_map->Point_NCB(Px,Py)>ui_Info->edge_threshold) {
		
		// Check if the edge_map is valid in Px,Py
		//      - move to the point Rx,Ry inside the silhouette of the object closest to the edge (Px,Py) 
		//      - if I can't then it is an edge ouside the mask
		//
		int Rx,Ry;
		if (GetCloserPointZero_NCB(Px,Py,mask,Rx,Ry,ui_Info->win_d_for_input_mask)) {
			if (mask->isOutside(Rx,Ry,ui_Info->win_d)) return false;
			
			// Check if external edge
			Vector<2> dir=GetExternalEdgeDirection_NCB(Rx,Ry,mask,(GreyLevel)0xFF,ui_Info->win_d);
			if ((dir[0]==0) && (dir[1]==0)) {
				// Internal
				if (ui_Info->search_only_external_edges) return false;

				dir=GetInternalEdgeDirection_NCB(Px,Py,edge_map,(GreyLevel)ui_Info->edge_threshold,ui_Info->win_d);

				double dot=dir*ui_Info->gradient;
				if (fabs(dot)>ui_Info->gradient_threshold) {
					ui_Info->target=Info->P;
					ui_Info->target_type=1;
					return true;
				}
			} else {
				// External 
				double dot=dir*ui_Info->gradient;
				if (ui_Info->gradient_type==1) {
					if (fabs(dot)>ui_Info->gradient_threshold) {
						ui_Info->target=Info->P;
						ui_Info->target_type=0;
						return true;
					}
				} else {
					if (dot>ui_Info->gradient_threshold) {
						ui_Info->target=Info->P;
						ui_Info->target_type=0;
						return true;
					}
				}
			}
		}
	}
	return false;

}

#define ADJUST_WEIGHT(edge_type)													\
	if (ui_EdgeStruct->gradient_type!=edge_type) tmp.sqrt_weight*=0.3;		        \
	tmp.sqrt_weight*=((float)mesh->AreaPoint[vertex_index]);					

#define CHECK_BORDER_OVERLAP																			\
	if (ui_EdgeStruct->gradient_type==0) {																\
				if (ui_EdgeStruct->mask->isOutside((int)(pt[0]),(int)(pt[1]),0)) {						\
					Vector<2> border_point;																\
					ScreenBorderIntersection(p1,pt,BORDER_LASCO,BORDER_LASCO,ui_EdgeStruct->mask->width-1-BORDER_LASCO,ui_EdgeStruct->mask->height-1-BORDER_LASCO,border_point);		\
					if (ui_EdgeStruct->mask->Point_NCB(Approx(border_point[0]),Approx(border_point[1]))==0) {			\
						return;																							\
					}																					\
				}																						\
			}


void Articulated_ICP::SearchForEdgeCorrespondence(int x,int y,int vertex_index,View *c_v,EdgeSearch_struct *ui_EdgeStruct) {
	Vector<2> p1,p2,p3,pt;
	p1[0]=x;
	p1[1]=y;


	// Affermo che il gradiente da ricercare nella seconda immagine (immagine target) 
	// è quello calcolato in p1 della prima immagine (current model estimate)
	// Cerchero' lungo da p1 lungo +dir e -dir un edge
	float df=-1,db=-1;
	int tf,tb;
	ui_EdgeStruct->target_type=-1;
	if (CheckSegment(p1,ui_EdgeStruct->gradient,max_length_search,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) {
		if (ui_EdgeStruct->target_type!=-1) {
			// Trovato lungo +dir in p2 a distanza df
			p2=ui_EdgeStruct->target;
			df=(float)((p2-p1).Norm2());
			tf=ui_EdgeStruct->target_type;
			// ASSURDO: Scarto se nelle vicinanza di p2 vi è presente parte della stima corrente del modello
			// In tal caso infatti nn si potrebbe mai tornare indietro <- Meglio asserire se vi è un gradiente 
			// della stima corrente!!!!!  ... ovvero che gradiente != 0 ...
			// if (IsLocallyForeground(p2[0],p2[1],ContourMask,0x00,3)) df=-1;
		}
	}

	ui_EdgeStruct->target_type=-1;
	if (CheckSegment(p1,-1.0*ui_EdgeStruct->gradient,max_length_search,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) {
		if (ui_EdgeStruct->target_type!=-1) {
			// Trovato lungo -dir in p3 a distanza db
			p3=ui_EdgeStruct->target;
			db=(float)((p3-p1).Norm2());
			tb=ui_EdgeStruct->target_type;
			// ASSURDO: Scarto se nelle vicinanza di p2 vi è presente parte della stima corrente del modello
			// if (IsLocallyForeground(p3[0],p3[1],ContourMask,0x00,3)) db=-1;
		}
	}

	if (db==-1) {
		if (df==-1) {
			// no backward, no forward
			return;
		} else {
			// only forward
			// p1 -> p2
			pt=p1+(p1-p2);
			CHECK_BORDER_OVERLAP;		// TODOLUCA
		}
	} else {
		if (df==-1) {
			// only backward
			// p1 -> p3
			pt=p1+(p1-p3);
			CHECK_BORDER_OVERLAP;		// TODOLUCA

			df=db;
			tf=tb;
			p2=p3;
		} else {
			// both backward and forward
			if (db<df) {
				df=db;
				tf=tb;
				p2=p3;
			}
		}
	}
	// Now, the movement is:
	// p1 --> p2 (df is the distance)

	TargetList_2D_Item_type tmp;
	tmp.VertexIndex=vertex_index;
	tmp.View=c_v;
	tmp.Source=p1;
	tmp.Target=p2;
	tmp.type=1;
	tmp.sqrt_weight=1.0;
	//ADJUST_WEIGHT(tf);
	if (ui_EdgeStruct->gradient_type!=tf) tmp.sqrt_weight*=0.3;  // TODOLUCA
	tmp.sqrt_weight*=((float)mesh->AreaPoint[vertex_index]);


	//
	// Extra weight
	//
	int b_a=mesh->GetBoneIDAttachedtoVertex(tmp.VertexIndex,0.0);
	if (b_a!=-1) {
		Bone *tmpBone=mesh->BoneRefTable[b_a];
		bool in_list=false;
		for(int i=0;i<ext_bone_num;i++) {
			if (strncmp(tmpBone->GetName(),ext_bone_names[i],strlen(ext_bone_names[i]))==0) {
				in_list=true;
				break;
			}
		}
		if (in_list) {
			tmp.sqrt_weight*=sqrt(ext_weight_edges);
		} else {
			#define ADDED_WEIGHT_LL3 25.0 // TODOLUCA
			if (tmpBone->Level<=3) tmp.sqrt_weight*=(float)(sqrt(ADDED_WEIGHT_LL3)); // TODOLUCA
		}
	}
	

	TargetList_2D->append(tmp);
}

inline ColorFloat Nearest_Float_Not_Bkg_Pixel(Bitmap<ColorFloat> *depth,int x,int y) {
	ColorFloat *c_p=depth->getBuffer()+(x+(y*depth->width));

	if (*c_p    !=FLT_MAX) return *c_p;

	if (depth->isOutside(x,y,1)) return FLT_MAX;

	if (*(c_p+1)!=FLT_MAX) return *(c_p+1);
	if (*(c_p-1)!=FLT_MAX) return *(c_p-1);

	c_p=c_p-depth->width;

	if (*c_p    !=FLT_MAX) return *c_p;
	if (*(c_p+1)!=FLT_MAX) return *(c_p+1);
	if (*(c_p-1)!=FLT_MAX) return *(c_p-1);

	c_p=c_p+(depth->width<<1);

	if (*c_p    !=FLT_MAX) return *c_p;
	if (*(c_p+1)!=FLT_MAX) return *(c_p+1);

	return *(c_p-1);
}

void Articulated_ICP::GetEdgesCorrespondences(int view,int Level) {	
	Vector<2>   dir;
	Vector<3>   pr_pointz;
	Vector<3>  *MeshPoints=mesh->Points.getMem();
	int         saveIndex=TargetList_2D->numElements();
	View       *c_v=&(Views[view]);


	if (!(EdgesSource&0x1)) return;
	if (c_v->Mask==NULL) return;														// Mask is needed: per riconoscere chi e' chi interno o esterno
	if (c_v->Edges==NULL) return;
	if (!(c_v->Enable&VIEW_ENABLE_SILHOUETTE)) {
		if (show_debug_information) cout<<"     Edges:          0 (not active)\n";
		return;
	}


	// Ensure Silhouette, Depth and Edges are ready
	RenderSilhouetteImage(view);
	RenderDepthImage(view);
	RenderEdgesImage(view);
	
	
	float edge_threshold_r=buffer_status.edge_threshold_r;
	int   win_d=(GRADIENT_WINDOW_SIZE-1)>>1;


	//
	// The different window size:
	//
	// win_d                    (rendered)
	// win_d_for_input_mask     (input)
	//      - I assume that the mask is always smaller than the actual image
	//      - win_d_for_input_mask indicates the maximum distance between the actual border and the mask (i.e., how much I eroded it)
	//
	// edge_mask_max_distance				    // Assuming 4 pixel as erosion 9x9 windows (only slow down, do not bias internal to external)
	//											// Mask should always be smaller -> otherwise they become all internal edges

	EdgeSearch_struct ui_EdgeStruct;
	ui_EdgeStruct.mask=c_v->Mask;
	ui_EdgeStruct.edge_map=c_v->Edges;
	ui_EdgeStruct.edge_threshold=input_edges_images_edge_threshold;
	ui_EdgeStruct.win_d=win_d;
	ui_EdgeStruct.win_d_for_input_mask=max(ui_EdgeStruct.win_d,edge_mask_max_distance);
	ui_EdgeStruct.gradient_threshold=gradient_tollerance;
	ui_EdgeStruct.search_only_external_edges=false;

	/*
	TODODEBUG
	Bitmap<ColorRGB> output(c_v->width,c_v->height);
	DebugEdgeDetectorOnImages(&ui_EdgeStruct,&output);
	output.Save("C:\\Users\\lballan\\Desktop\\Edges_img.bmp");
	DebugEdgeDetectorOnRenderedImages(view,edge_threshold,win_d,&output);
	output.Save("C:\\Users\\lballan\\Desktop\\Edges_rendered.bmp");
	*/


	for(int j=0;j<mesh->num_p;j++,MeshPoints++) {
		pr_pointz=c_v->ProjectionZ(*MeshPoints);
		int     x=Approx(pr_pointz[0]);
		int     y=Approx(pr_pointz[1]);

		if (pr_pointz[2]<=buffer_status.min_z_plane) continue;					// behind the clipping plane
		if (EdgesImage->isOutside(x,y,0)) continue;

		float gradient=EdgesImage->Point_NCB(x,y);

		//
		// if the vertex is at the edge of the 3D model
		//
		if (gradient>edge_threshold_r) {

			float visible_z=Nearest_Float_Not_Bkg_Pixel(DepthImage,x,y);
			if (visible_z==FLT_MAX) continue;									// questo non accade mai ma per sicurezza

			if (pr_pointz[2]<=visible_z+buffer_status.z_tollerance) {		
				// j visibile
				// x,y,pr_pointz[2] or pr_pointz[1,2,3]
			
				if (gradient==FLT_MAX) {
					//
					// The vertex is at the EXTERNAL edge of the model
					//
					dir=GetExternalEdgeDirection(x,y,DepthImage,win_d);

					ui_EdgeStruct.gradient_type=0;
					ui_EdgeStruct.gradient=dir;

					
					if (INVALID_INTERNAL_EDGES==true) {
						int b_a_tmp=mesh->GetBoneIDAttachedtoVertex(j,0.0);
						if (b_a_tmp!=-1) {
							Bone *tmpBone=mesh->BoneRefTable[b_a_tmp];
							if (strncmp(tmpBone->GetName(),"Bone",4)==0) {
								int index=atoi(tmpBone->GetName()+4);
								if ((index!=1) && (index!=12)) {
									ui_EdgeStruct.search_only_external_edges=true;
								} else {
									ui_EdgeStruct.search_only_external_edges=true;
									SearchForEdgeCorrespondence(x,y,j,c_v,&ui_EdgeStruct);
									ui_EdgeStruct.search_only_external_edges=false;
								}
							}
						}
					}

					SearchForEdgeCorrespondence(x,y,j,c_v,&ui_EdgeStruct);
					ui_EdgeStruct.search_only_external_edges=false;
				} else {
					//
					// The vertex is at the INTERNAL edge of the model
					//
					dir=GetInternalEdgeDirection(x,y,EdgesImage,edge_threshold_r,win_d);

					ui_EdgeStruct.gradient_type=1;
					ui_EdgeStruct.gradient=GetOrthogonalVersor(dir);
					SearchForEdgeCorrespondence(x,y,j,c_v,&ui_EdgeStruct);
				}
			}
		}
	}


	//
	// Outliers and Output
	//
	int old_corresp_num=TargetList_2D->numElements();
	Clean_TargetList_2D_FromOutliers(saveIndex);
	if (show_debug_information) cout<<"     Edges:          "<<(TargetList_2D->numElements()-saveIndex)<<" (outliers "<<(old_corresp_num-TargetList_2D->numElements())<<")\n";
}



























































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Depths ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::GetDepthCorrespondences(int view,int Level) {
	Vector<3> pr_pointz;
	Vector<3> *MeshPoints;
	float  sqrt_d_weight;

	if (Views[view].Depth==NULL) return;
	if (Level==0) sqrt_d_weight=(float)sqrt(depth_local_weight_l0);
	else          sqrt_d_weight=(float)sqrt(depth_local_weight_l1);
	if (sqrt_d_weight==0.0) return;


	RenderDepthImage(view);
	float z_lasco=buffer_status.z_tollerance;
	MeshPoints=mesh->Points.getMem();

	for(int j=0;j<mesh->num_p;j++,MeshPoints++) {
		pr_pointz=Views[view].ProjectionZ(*MeshPoints);
		if (pr_pointz[2]<=buffer_status.min_z_plane) continue;

		int   x=Approx(pr_pointz[0]);
		int   y=Approx(pr_pointz[1]);
		float visible_z=DepthImage->Point(x,y);
		if (visible_z==FLT_MAX) continue;

		if (pr_pointz[2]<=visible_z+z_lasco) {
			// j visibile
			// x,y,pr_pointz[2]
			float actual_z=Views[view].Depth->Point(x,y);
			if (actual_z==FLT_MAX) continue;

			int b_a=mesh->GetBoneIDAttachedtoVertex(j,0.0);
			if (b_a==-1) continue;
			Bone *tmpBone=mesh->BoneRefTable[b_a];
			if (!tmpBone->CanRotateOrTranslate()) {
				do {
					tmpBone=static_cast<Bone *>(tmpBone->Father);
				} while((tmpBone!=NULL) && (!tmpBone->CanRotateOrTranslate()));
				if (tmpBone==NULL) continue;
			}

			TargetList_1D_Item_type tmp;
			tmp.VertexIndex=j;
			tmp.sqrt_weight=sqrt_d_weight*((float)mesh->AreaPoint[j]);
			tmp.View=&Views[view];
			tmp.Target=actual_z;
			tmp.type=0;

			TargetList_1D->append(tmp);
		}
	}

	if (show_debug_information) cout<<"     Depth:          "<<TargetList_1D->numElements()<<"\n";
}



































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Salient Points ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "lp_lib.h"

struct graph_element {
	int m;
	int q;
	float v;
};

REAL SolveAssociationProblem(int n_row,int n_col,graph_element *graph,graph_element *&solutions,int &num_solutions) {
	int   *colno=new int[n_col*n_row];
	REAL  *row=new REAL[n_col*n_row];
	lprec *lp=make_lp(0,n_col*n_row);
	solutions=NULL;
	num_solutions=0;

	set_add_rowmode(lp,TRUE);
	if (n_row>=n_col) {
		for(int q=0;q<n_col;q++) {
			for(int m=0;m<n_row;m++) {
				colno[m]=1+q+m*n_col;
				row[m]=1;
			}
			add_constraintex(lp,n_row,row,colno,EQ,1);
		}
	}
	if (n_row<=n_col) {
		for(int m=0;m<n_row;m++) {
			for(int q=0;q<n_col;q++) {
				colno[q]=1+q+m*n_col;
				row[q]=1;
			}
			add_constraintex(lp,n_col,row,colno,EQ,1);
		}
	}
	set_add_rowmode(lp, FALSE);

	for(int q=0;q<n_col*n_row;q++) {
		colno[q]=q+1;
		row[q]=graph[q].v;
	}
	set_obj_fnex(lp,n_col*n_row,row,colno);
	set_minim(lp);
	//write_LP(lp,stdout);
	set_verbose(lp,IMPORTANT);
	int ret=solve(lp);
	// TODO use ret
	//printf("Objective value: %f\n", get_objective(lp));
	REAL obj_val=get_objective(lp);
	get_variables(lp,row);

	delete_lp(lp);
	delete []colno;

	
	num_solutions=0;
	for(int j=0;j<n_col*n_row;j++) 
		if (Approx(row[j])>=1) num_solutions++;
	solutions=new graph_element[num_solutions];

	int index=0;
	for(int j=0;j<n_col*n_row;j++) {
		//printf("%s: %f\n", get_col_name(lp, j + 1), row[j]);
		if (Approx(row[j])>=1) {
			solutions[index].m=graph[j].m;
			solutions[index].q=graph[j].q;
			solutions[index].v=graph[j].v;
			index++;
		}
	}

	delete []row;
	return obj_val;
}

REAL SolveAssociationProblem_with_missing_data(int n_row,int n_col,graph_element *graph,float invalid_th,graph_element *&solutions,int &num_solutions,int &problem_relaxed) {
	REAL obj_val=SolveAssociationProblem(n_row,n_col,graph,solutions,num_solutions);

	if (n_row==n_col) {
		for(int j=0;j<num_solutions;j++) {
			if (solutions[j].v>=invalid_th) {
				//write_LP(lp,stdout);
				
				int m_,q_;
				bool break_flag=false;
				for(m_=0;m_<n_row;m_++) {
					for(q_=0;q_<n_col;q_++) {
						if ((graph[m_*n_col+q_].m==solutions[j].m) && (graph[m_*n_col+q_].q==solutions[j].q)) {
							break_flag=true;
							break;
						}
					}
					if (break_flag) break;
				}
				if (solutions) delete []solutions;
				solutions=NULL;
				num_solutions=0;
				problem_relaxed++;

				graph_element *graph1=new graph_element[n_row*n_col];
				graph_element *graph2=new graph_element[n_row*n_col];
				
				// Get rid of row m_
				for(int m1=0,m=0;m<n_row;m++) {
					if (m!=m_) {
						for(int q1=0,q=0;q<n_col;q++) {
							graph1[m1*n_col+q1]=graph[m*n_col+q];
							q1++;
						}
						m1++;
					}
				}
				// Get rid of col q_
				for(int m1=0,m=0;m<n_row;m++) {
					for(int q1=0,q=0;q<n_col;q++) {
						if (q!=q_) {
							graph2[m1*(n_col-1)+q1]=graph[m*n_col+q];
							q1++;
						}
					}
					m1++;
				}
				
				int num_solutions1=0;
				int num_solutions2=0;
				int problem_relaxed1=0;
				int problem_relaxed2=0;
				graph_element *solutions1=NULL;
				graph_element *solutions2=NULL;
				REAL obj_val1=SolveAssociationProblem_with_missing_data(n_row-1,n_col,graph1,invalid_th,solutions1,num_solutions1,problem_relaxed1);
				REAL obj_val2=SolveAssociationProblem_with_missing_data(n_row,n_col-1,graph2,invalid_th,solutions2,num_solutions2,problem_relaxed2);

				delete []graph1;
				delete []graph2;

				if (obj_val1<obj_val2) {
					solutions=solutions1;
					num_solutions=num_solutions1;
					if (solutions2) delete []solutions2;
					problem_relaxed+=problem_relaxed1;
					return obj_val1;
				} else {
					solutions=solutions2;
					num_solutions=num_solutions2;
					if (solutions1) delete []solutions1;
					problem_relaxed+=problem_relaxed2;
					return obj_val2;
				}
			}
		}
	}

	return obj_val;
}

graph_element *square_graph(int n_row,int n_col,graph_element *graph,int &n_elements,float invalid_th) {
	if (n_row==n_col) {
		n_elements=n_col;
		return graph;
	}

	n_elements=max(n_row,n_col);
	graph_element *graph_new=new graph_element[n_elements*n_elements];

	for(int q=0;q<n_elements;q++) {
		for(int m=0;m<n_elements;m++) {
			if (m>=n_row) {
				graph_new[q+m*n_elements].m=-1;
				graph_new[q+m*n_elements].q=-1;
				graph_new[q+m*n_elements].v=3*invalid_th;
				continue;
			}
			if (q>=n_col) {
				graph_new[q+m*n_elements].m=-1;
				graph_new[q+m*n_elements].q=-1;
				graph_new[q+m*n_elements].v=3*invalid_th;
				continue;
			}
			graph_new[q+m*n_elements]=graph[q+m*n_col];
		}
	}
	delete []graph;
	return graph_new;
}


REAL SolveAssociationProblem_with_missing_data_correct(int n_elements,graph_element *graph,float invalid_th,graph_element *&solutions,int &num_solutions,int &problem_relaxed) {
	problem_relaxed=0;
	
	int n_var=(n_elements*n_elements)+(2*n_elements);
	int first_aux_row_index=1+(n_elements*n_elements);
	int first_aux_col_index=1+(n_elements*n_elements)+n_elements;
	float lambda=invalid_th;


	int   *ele_index=new int[n_var];
	REAL  *weight=new REAL[n_var];
	lprec *lp=make_lp(0,n_var);
	solutions=NULL;
	num_solutions=0;

	// m is the row index
	// q is the col index


	//
	// Constraints
	//
	set_add_rowmode(lp,TRUE);
	for(int q=0;q<n_elements;q++) {
		for(int m=0;m<n_elements;m++) {
			ele_index[m]=1+(q+m*n_elements);
			weight[m]=1;
		}
		ele_index[n_elements]=first_aux_col_index+q;
		weight[n_elements]=1;
		add_constraintex(lp,n_elements+1,weight,ele_index,EQ,1);
	}

	for(int m=0;m<n_elements;m++) {
		for(int q=0;q<n_elements;q++) {
			ele_index[q]=1+(q+m*n_elements);
			weight[q]=1;
		}
		ele_index[n_elements]=first_aux_row_index+m;
		weight[n_elements]=1;
		add_constraintex(lp,n_elements+1,weight,ele_index,EQ,1);
	}
	set_add_rowmode(lp, FALSE);



	//
	// Objective functional
	//
	for(int q=0;q<n_elements*n_elements;q++) {
		ele_index[q]=1+q;
		weight[q]=graph[q].v;
	}
	int last_idx=(n_elements*n_elements);
	for(int q=0;q<n_elements;q++) {
		ele_index[last_idx+q]=first_aux_row_index+q;
		weight[last_idx+q]=lambda;
	}
	last_idx=(n_elements*n_elements)+n_elements;
	for(int q=0;q<n_elements;q++) {
		ele_index[last_idx+q]=first_aux_col_index+q;
		weight[last_idx+q]=lambda;
	}

	set_obj_fnex(lp,n_var,weight,ele_index);
	set_minim(lp);
	//write_LP(lp,stdout);
	set_verbose(lp,IMPORTANT);
	int ret=solve(lp);
	// TODO use ret
	// printf("Objective value: %f\n", get_objective(lp));
	REAL obj_val=get_objective(lp);
	get_variables(lp,weight);

	delete_lp(lp);
	delete []ele_index;

	num_solutions=0;
	for(int q=0;q<n_elements*n_elements;q++) 
		if (Approx(weight[q])>=1) num_solutions++;
	
	//cout<<"num_solutions = "<<num_solutions<<endl;

	solutions=new graph_element[num_solutions];

	int index=0;
	for(int j=0;j<n_elements*n_elements;j++) {
		//printf("%s: %f\n", get_col_name(lp, j + 1), weight[j]);
		if (Approx(weight[j])>=1) {
			if ((graph[j].q<0) || (graph[j].m<0)) {
				cout<<"warning weight < -1"<<endl;
			} else {
				solutions[index].m=graph[j].m;
				solutions[index].q=graph[j].q;
				solutions[index].v=graph[j].v;
				index++;
			}
		}
	}

	delete []weight;
	return obj_val;
}

REAL SolveAssociationProblem_with_missing_data_correct(int n_row,int n_col,graph_element *&graph,float invalid_th,graph_element *&solutions,int &num_solutions,int &problem_relaxed) {
	int n_elements;
	graph=square_graph(n_row,n_col,graph,n_elements,invalid_th);
	return SolveAssociationProblem_with_missing_data_correct(n_elements,graph,invalid_th,solutions,num_solutions,problem_relaxed);
}


void Articulated_ICP::GetSalientPointCorrespondences(int view,int Level,int frame_index) {
	Vector<2>   pr_point;
	Vector<3>   pr_pointz;
	Vector<3>  *MeshPoints=mesh->Points.getMem();
	int         saveIndex=TargetList_2D->numElements();
	View       *c_v=&(Views[view]);
	float       sqrt_s_weight=(float)sqrt(salient_points_local_weight);

	
	if (!(c_v->Enable)) {
		if (show_debug_information) cout<<"     Salient Points:    0 (not active)\n";
		return;
	}
	if (salient_points_vertices==NULL) return;
	if (salient_points_2D==NULL) return;
	if (salient_points_2D[view]==NULL) return;
	if (sqrt_s_weight==0.0) return;


	// Ensure Depth are ready
	RenderDepthImage(view);

	Array<Vector<2>> *points_2D=salient_points_2D[view]->get_tracks_read_only(frame_index);
	if (points_2D==NULL)  return;
	int				      n_row=points_2D->numElements();
	int                   n_col=salient_points_vertices->numElements();
	if (n_row*n_col<1)    return;
	graph_element        *graph=new graph_element[n_row*n_col];



	//
	// Compute the graph
	//
	for(int m=0;m<n_row;m++) {
		for(int q=0;q<n_col;q++) {
			graph[m*n_col+q].m=m;
			graph[m*n_col+q].q=q;
			graph[m*n_col+q].v=FLT_MAX;

			int          j=(*salient_points_vertices)[q];
			pr_pointz     =c_v->ProjectionZ(MeshPoints[j]);
			pr_point[0]   =pr_pointz[0];
			pr_point[1]   =pr_pointz[1];
			int          x=Approx(pr_pointz[0]);
			int          y=Approx(pr_pointz[1]);

			if (DepthImage->isOutside(x,y,0)) continue;
			float visible_z=Nearest_Float_Not_Bkg_Pixel(DepthImage,x,y);
			if (visible_z==FLT_MAX) continue;						       // questo non accade mai ma per sicurezza
			if (pr_pointz[2]<=buffer_status.min_z_plane) continue;
			if (pr_pointz[2]<=visible_z+salient_depth_tollerance*buffer_status.z_tollerance)
				graph[m*n_col+q].v=(float)((pr_point-((*points_2D)[m])).Norm2());
		}
	}



	//
	// Delete rows and coloums which do not any possible valid solution
	//
	for(int m=0;m<n_row;m++) {
		bool ok=false;
		for(int q=0;q<n_col;q++) {
			if (graph[m*n_col+q].v<salient_points_th) {
				ok=true;
				break;
			}
		}
		if (!ok) {
			// copy up
			for(int m_=m;m_<n_row-1;m_++) {
				// copy m_+1 in m_
				for(int q=0;q<n_col;q++) graph[m_*n_col+q]=graph[(m_+1)*n_col+q];
			}
			n_row=n_row-1;
			m--;
		}
	}
	for(int q=0;q<n_col;q++) {
		bool ok=false;
		for(int m=0;m<n_row;m++) {
			if (graph[m*n_col+q].v<salient_points_th) {
				ok=true;
				break;
			}
		}
		if (!ok) {
			// copy left
			for(int q_=q;q_<n_col-1;q_++) {
				// copy q_+1 in q_
				for(int m=0;m<n_row;m++) graph[m*n_col+q_]=graph[m*n_col+(q_+1)];
			}
			// delete last element of each row
			for(int m=n_row-1;m>0;m--) {
				// copy s+1 in s 
				for(int s=(m*n_col)-1;s<(n_col*n_row)-1;s++) graph[s]=graph[s+1];
			}
			n_col=n_col-1;
			q--;
		}
	}



	//
	//  Solve the association problem
	//
	int problem_relaxed=0;
	if (n_col*n_row!=0) {
		int num_solutions=0;
		graph_element *solutions=NULL;
		
		//SolveAssociationProblem_with_missing_data(n_row,n_col,graph,(float)salient_points_th,solutions,num_solutions,problem_relaxed);
		SolveAssociationProblem_with_missing_data_correct(n_row,n_col,graph,(float)salient_points_th,solutions,num_solutions,problem_relaxed);

		for(int j=0;j<num_solutions;j++) {
			TargetList_2D_Item_type tmp;
			tmp.VertexIndex=(*salient_points_vertices)[solutions[j].q];
			tmp.View=c_v;
			tmp.Source=c_v->Projection(MeshPoints[tmp.VertexIndex]);
			tmp.Target=(*points_2D)[solutions[j].m];
			if (solutions[j].v<salient_points_th) {
				tmp.sqrt_weight=sqrt_s_weight*((float)mesh->AreaPoint[tmp.VertexIndex]);
				tmp.type=3;
				TargetList_2D->append(tmp);
			}
		}

		if (solutions) delete []solutions;
	}


	delete []graph;
	if (show_debug_information) {
		if (problem_relaxed==0) cout<<"     Salient Points:    "<<(TargetList_2D->numElements()-saveIndex)<<"\n";
		else cout<<"     Salient Points:    "<<(TargetList_2D->numElements()-saveIndex)<<" (relaxed "<<problem_relaxed<<" times)\n";
	} 
}























////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Textures ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::GetTextureCorrespondences(int view,int Level) {
	// TODO
	// TODO if local_weight==0 do not compute anything.... 
	//   ok per Sil,OF,Collision,depth 
	//	 ma non per texture

	if (!(Views[view].Enable)) {
		if (show_debug_information) cout<<"     Texture:        0 (not active)\n";
		return;
	}
	if (Views[view].Img==NULL) return;
	if (texture_local_weight==0.0) return;

	RenderTextureImage(view);

	//if (show_debug_information) cout<<"     Texture:        "<<" \n";
}




















////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////  Optical Flow ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Note:	
//			View should be set at time (t-1) 
//			Mesh        is set at time (t-1)   -> PrepareMesh(base_frame_index) does this
//
void Articulated_ICP::GetOpticalFlowCorrespondences(int to_estimate_frame,int start_frame) {
	LastUsed_Feature_StartFrame=start_frame;
	LastUsed_Feature_EndFrame=to_estimate_frame;

	Feat_TargetList->clear();
	if (!FeaturesList) return;
	if (opticalflow_local_weight==0.0) return;
	float sqrt_o_weight=(float)sqrt(opticalflow_local_weight);

	if (show_debug_information) cout<<"-> Optical Flow correspondences\n";
	// Texture Information
	Array<typename Vector<2>> out_a(100);
	Array<typename Vector<2>> out_b(100);

	if (start_frame==to_estimate_frame) return;

	for(int i=0;i<num_views;i++) {
		int saveIndex=Feat_TargetList->numElements();

		// Get
		if (!(Views[i].Enable&VIEW_ENABLE_OPTICAL_FLOW)) {
			if (show_debug_information) cout<<"     founded: 0 (not active)\n";
			continue;
		}
		FeaturesList[i]->GetCommonFeatures(start_frame,to_estimate_frame,&out_a,&out_b);
		if (out_a.numElements()==0) continue;

		
		ResetBufferStatus();
		RenderDepthImage(i);
		float z_lasco=buffer_status.z_tollerance;



		// Project all point
		Vector<2> pr_point;
		Vector<3> pr_pointz;
		int *Indexes=new int[out_a.numElements()];
		double *Distances=new double[out_a.numElements()];
		for(int h=0;h<out_a.numElements();h++) {
			Indexes[h]=-1;
			Distances[h]=DBL_MAX;
		}



		for(int j=0;j<mesh->num_p;j++) {
			pr_pointz=Views[i].ProjectionZ(mesh->Points[j]);			// TODOSS
			if (pr_pointz[2]<=buffer_status.min_z_plane) continue;

			pr_point[0]=pr_pointz[0];
			pr_point[1]=pr_pointz[1];
			float z=DepthImage->Point(Approx(pr_pointz[0]),Approx(pr_pointz[1]));
			
			if (pr_pointz[2]<=z+z_lasco) {
				for(int h=0;h<out_a.numElements();h++) {
					double x=(out_a[h]-pr_point).Norm2();
					if (x<Distances[h]) {
						Distances[h]=x;
						Indexes[h]=j;
					}
				}
			}
		}
		
		// set View at (t)
		double MAX_ALLOWED_DISTANCE=max(Views[i].width,Views[i].height)*(max_of_info_distance/100.0);

		for(int h=0;h<out_a.numElements();h++) {
			if (Indexes[h]!=-1) {
				if (Distances[h]<MAX_ALLOWED_DISTANCE) {
					TargetList_2D_Item_type tmp;
					tmp.VertexIndex=Indexes[h];
					tmp.View=&Views[i];
					tmp.Source=Views[i].Projection(mesh->Points[Indexes[h]]);
					tmp.Target=out_b[h];
					tmp.sqrt_weight=sqrt_o_weight*((float)mesh->AreaPoint[tmp.VertexIndex]);   // TODOAPPROX mettere qui added_sqrt bla bla bal
					tmp.type=2;


					//
					// Extra weight
					//
					int b_a=mesh->GetBoneIDAttachedtoVertex(tmp.VertexIndex,0.0);
					if (b_a!=-1) {
						Bone *tmpBone=mesh->BoneRefTable[b_a];
						bool in_list=false;
						for(int i=0;i<ext_bone_num;i++) {
							if (strncmp(tmpBone->GetName(),ext_bone_names[i],strlen(ext_bone_names[i]))==0) {
								in_list=true;
								break;
							}
						}
						if (in_list) tmp.sqrt_weight*=sqrt(ext_weight_optical_flow);
					}
					

					Feat_TargetList->append(tmp);
				}
			}
		}

		delete Indexes;
		delete Distances;





		//
		// Calcola la media degli spostamenti
		//
		int old_corresp_num=Feat_TargetList->numElements();
		for(int i=0;i<mesh->num_bones;i++) {
			mean_move[i]=0.0;
			mean_move_num[i]=0;
		}
		for(int i=saveIndex;i<Feat_TargetList->numElements();i++) {
			int b_a=mesh->GetBoneIDAttachedtoVertex((*Feat_TargetList)[i].VertexIndex,0.0);
			if (b_a==-1) {
				Feat_TargetList->del(i);
				i--;
				continue;
			}
			Bone *tmpBone=mesh->BoneRefTable[b_a];
			if (!tmpBone->CanRotateOrTranslate()) {
				do {
					do {
						tmpBone=static_cast<Bone *>(tmpBone->Father);
					} while((tmpBone!=NULL) && (!tmpBone->CanRotateOrTranslate()));
					b_a=mesh->GetBoneID(tmpBone);
				} while((tmpBone!=NULL) && (b_a==-1));
				if (b_a==-1) {
					Feat_TargetList->del(i);
					i--;
					continue;
				}
			}
			
			mean_move[b_a]+=((*Feat_TargetList)[i].Source-(*Feat_TargetList)[i].Target).Norm2();
			mean_move_num[b_a]++;
		}
		for(int i=0;i<mesh->num_bones;i++) {
			if (mean_move_num[i]!=0) mean_move[i]/=mean_move_num[i];
			if (mean_move[i]<MEAN_MIN_PIXELS) mean_move[i]=MEAN_MIN_PIXELS;
		}


		// Impone che gli spostamenti di entità elevata rispetto alla
		// media e alla soglia SMOOTH_MOVEMENT siano esclusi,
		// ritenuti outliers.
		for(int i=saveIndex;i<Feat_TargetList->numElements();i++) {
			int b_a=mesh->GetBoneIDAttachedtoVertex((*Feat_TargetList)[i].VertexIndex,0.0);

			Bone *tmpBone=mesh->BoneRefTable[b_a];
			if (!tmpBone->CanRotateOrTranslate()) {
				do {
					tmpBone=static_cast<Bone *>(tmpBone->Father);
				} while(!tmpBone->CanRotateOrTranslate());
				b_a=mesh->GetBoneID(tmpBone);
			}

			double mov_ent=((*Feat_TargetList)[i].Source-(*Feat_TargetList)[i].Target).Norm2();
			if (mean_move_num[b_a]==1) {
				// Un'unica informazione di tessitura puo' essere sbagliata, meglio escluderla
				Feat_TargetList->del(i);
				i--;
				continue;
			}
			if (mean_move_num[b_a]<NUMERO_MIN_PER_MEDIA_VALIDA) continue;
			if (mov_ent>caotic_smooth_tollerance*mean_move[b_a]) {
				Feat_TargetList->del(i);
				i--;
				continue;
			}
		}
		if (show_debug_information) cout<<"     founded: "<<(Feat_TargetList->numElements()-saveIndex)<<" (outliers "<<(old_corresp_num-Feat_TargetList->numElements())<<")\n";
	}
}







































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Residual ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::SilhouetteEdgesSalientPointsOpticalFlow_CorrespondencesResidual(double &residual_S,double &residual_E,double &residual_P,double &residual_O,int *n_observations_S,int *n_observations_E,int *n_observations_P,int *n_observations_O) {
	Vector<2> PP2;
	Vector<3> tmp_p;
	Vector<3> *Points=mesh->Points.getMem();
	double *AreaPoint=mesh->AreaPoint.getMem();
	residual_S=0.0;
	residual_E=0.0;
	residual_P=0.0;
	residual_O=0.0;
	*n_observations_S=0;
	*n_observations_E=0;
	*n_observations_P=0;
	*n_observations_O=0;

	for(int h=0;h<TargetList_2D->numElements();h++) {
		int point_index=(*TargetList_2D)[h].VertexIndex;
		tmp_p=Points[point_index];
		double Target_Weight=AreaPoint[point_index];

		PP2=(*TargetList_2D)[h].View->Projection(tmp_p)-(*TargetList_2D)[h].Target;
		switch ((*TargetList_2D)[h].type) {
			case 0:
				residual_S+=Target_Weight*Target_Weight*(PP2*PP2);
				(*n_observations_S)++;
				break;
			case 1:
				residual_E+=Target_Weight*Target_Weight*(PP2*PP2);
				(*n_observations_E)++;
				break;
			case 2:
				residual_O+=Target_Weight*Target_Weight*(PP2*PP2);
				(*n_observations_O)++;
				break;
			case 3:
				residual_P+=Target_Weight*Target_Weight*(PP2*PP2);
				(*n_observations_P)++;
				break;
		};
	}
}
double Articulated_ICP::Depth_CorrespondencesResidual(int *n_observations) {
	Vector<2> PP2;
	Vector<3> tmp_p;
	Vector<3> *Points=mesh->Points.getMem();
	double residual=0.0;

	for(int h=0;h<TargetList_1D->numElements();h++) {
		int point_index=(*TargetList_1D)[h].VertexIndex;
		tmp_p=Points[point_index];
		tmp_p=(*TargetList_1D)[h].View->ProjectionZ(tmp_p);
		double dist=(tmp_p[2])-(*TargetList_1D)[h].Target;
		dist=(*TargetList_1D)[h].sqrt_weight*dist;
		residual+=(dist*dist);
	}

	(*n_observations)=TargetList_1D->numElements();
	return residual;
}
double Articulated_ICP::Texture_CorrespondencesResidual(int *n_observations) {
	(*n_observations)=0;
	return 0.0; // TODO
}


double Articulated_ICP::Feat_3D_CorrespondencesResidual(int *n_observations) {
	Vector<3> tmp;
	Vector<3> *Points=mesh->Points.getMem();
	double residual=0.0;

	for(int h=0;h<TargetList_3D->numElements();h++) {
		int point_index1=(*TargetList_3D)[h].VertexIndex;
		int point_index2=(*TargetList_3D)[h].VertexIndex_Target;
		tmp=Points[point_index1];
		if (point_index2!=-1) tmp=tmp-Points[point_index2];
		tmp=tmp-(*TargetList_3D)[h].Target;
		tmp=(*TargetList_3D)[h].sqrt_weight*tmp;
		residual+=(tmp*tmp);
	}

	(*n_observations)=TargetList_3D->numElements();
	return residual;
}


































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Info ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::ShowCorrespondencesInfo(int Level) {
	IKSolver *current_ik_solver;
	if (Level==0) current_ik_solver=IK_Solver;
	else current_ik_solver=IK_Solver_Pyramid_Level_1;
	
	// IK variable weights
	double current_opticalflow_weight=(current_ik_solver->added_sqrt_weight)*(current_ik_solver->added_sqrt_weight);
	double current_collision_weight=current_ik_solver->GetCollisionWeight();
	double current_depth_weight;
	if (Level==0) current_depth_weight=depth_local_weight_l0;
	else current_depth_weight=depth_local_weight_l1;

	double Residual_S=0.0,
		   Residual_E=0.0,
		   Residual_P=0.0,
		   Residual_O=0.0,
		   Residual_C=0.0,
		   Residual_D=0.0,
		   Residual_T=0.0,
		   Residual_N=0.0;
	int    n_observations_S=0,
		   n_observations_E=0,
		   n_observations_P=0,
		   n_observations_O=0,
		   n_observations_C=0,
		   n_observations_D=0,
		   n_observations_T=0,
		   n_observations_N=0;

	
	SilhouetteEdgesSalientPointsOpticalFlow_CorrespondencesResidual(Residual_S,Residual_E,Residual_P,Residual_O,&n_observations_S,&n_observations_E,&n_observations_P,&n_observations_O);
	Residual_P*=salient_points_local_weight;
	Residual_O*=current_opticalflow_weight;
	Residual_C=current_collision_weight*current_ik_solver->Collision_Residual(&n_observations_C);
	Residual_D=current_depth_weight*Depth_CorrespondencesResidual(&n_observations_D);
	Residual_T=texture_local_weight*Texture_CorrespondencesResidual(&n_observations_T);
	Residual_N=Feat_3D_CorrespondencesResidual(&n_observations_N);

	// Totals
	double Residual=Residual_S+Residual_E+Residual_P+Residual_O+Residual_C+Residual_D+Residual_T+Residual_N;
	int    n_observations=n_observations_S+n_observations_E+n_observations_P+n_observations_O+n_observations_C+n_observations_D+n_observations_T+n_observations_N;

	double s_ratio=100.0*Residual_S/Residual;
	double e_ratio=100.0*Residual_E/Residual;
	double p_ratio=100.0*Residual_P/Residual;
	double o_ratio=100.0*Residual_O/Residual;
	double c_ratio=100.0*Residual_C/Residual;
	double d_ratio=100.0*Residual_D/Residual;
	double t_ratio=100.0*Residual_T/Residual;
	double n_ratio=100.0*Residual_N/Residual;
	double s_t_ratio=(100.0*n_observations_S)/n_observations;
	double e_t_ratio=(100.0*n_observations_E)/n_observations;
	double p_t_ratio=(100.0*n_observations_P)/n_observations;
	double o_t_ratio=(100.0*n_observations_O)/n_observations;
	double c_t_ratio=(100.0*n_observations_C)/n_observations;
	double d_t_ratio=(100.0*n_observations_D)/n_observations;
	double t_t_ratio=(100.0*n_observations_T)/n_observations;
	double n_t_ratio=(100.0*n_observations_N)/n_observations;
	if (Residual==0.0) {
		s_ratio=0.0;
		e_ratio=0.0;
		p_ratio=0.0;
		o_ratio=0.0;
		c_ratio=0.0;
		d_ratio=0.0;
		t_ratio=0.0;
		n_ratio=0.0;
	}
	if (n_observations==0.0) {
		s_t_ratio=0.0;
		e_t_ratio=0.0;
		p_t_ratio=0.0;
		o_t_ratio=0.0;
		c_t_ratio=0.0;
		d_t_ratio=0.0;
		t_t_ratio=0.0;
		n_t_ratio=0.0;
	}

		printf("           Residual            Observations\n");
	if (n_observations_S!=0.0) {
		printf("    S = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_S,s_ratio,n_observations_S,s_t_ratio);
	}
	if (n_observations_E!=0.0) {
		printf("    E = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_E,e_ratio,n_observations_E,e_t_ratio);
	}
	if ((salient_points_local_weight!=0.0) && (n_observations_P!=0.0)) {
		printf("    P = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_P,p_ratio,n_observations_P,p_t_ratio);
	}
	if ((current_opticalflow_weight!=0.0) && (n_observations_O!=0.0)) {
		printf("    O = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_O,o_ratio,n_observations_O,o_t_ratio);
	}
	if ((current_depth_weight!=0.0) && (n_observations_D!=0.0)) {
		printf("    D = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_D,d_ratio,n_observations_D,d_t_ratio);
	}
	if ((texture_local_weight!=0.0) && (n_observations_T!=0.0)) {
		printf("    T = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_T,t_ratio,n_observations_T,t_t_ratio);
	}
	if ((collision_detector!=NULL) && (current_collision_weight!=0)) {
		printf("    C = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_C,c_ratio,n_observations_C,c_t_ratio);
	}
	if (n_observations_N!=0.0) {
		printf("    N = %-10.4g  (%3.0f%%)     %4d (%3.0f%%)\n",Residual_N,n_ratio,n_observations_N,n_t_ratio);
	}
}









































/*
	if (db==-1) {
		if (df==-1) {
			return;
		}
	} else {
		if (df==-1) {
			df=db;
			p2=p3;
			
			TargetList_2D_Item_type NewTarget;
			NewTarget.VertexIndex=vertex_index;
			NewTarget.View=c_v;
			NewTarget.Source=p1;
			NewTarget.Target=p2;
			TargetList_2D->append(NewTarget);
		} else {
			if (db==-1) {
				df=db;
				p2=p3;
				
				TargetList_2D_Item_type NewTarget;
				NewTarget.VertexIndex=vertex_index;
				NewTarget.View=c_v;
				NewTarget.Source=p1;
				NewTarget.Target=p2;
				TargetList_2D->append(NewTarget);
			} else {
			
				TargetList_2D_Item_type NewTarget;
				NewTarget.VertexIndex=vertex_index;
				NewTarget.View=c_v;
				NewTarget.Source=p1;
				NewTarget.Target=p2;
				TargetList_2D->append(NewTarget);

				NewTarget.VertexIndex=vertex_index;
				NewTarget.View=c_v;
				NewTarget.Source=p1;
				NewTarget.Target=p3;
				TargetList_2D->append(NewTarget);
			}
		}
	}
*/
	
	/*
	Vector<2> p;
	p[0]=x;
	p[1]=y;
	

	TargetList_2D_Item_type NewTarget;
	NewTarget.VertexIndex=vertex_index;
	NewTarget.View=c_v;
	NewTarget.Source=p;
	
#define AICP_HOLE_PX 2 // 2 px away


	
	
	Vector<2> t1,t2;
	double d1=-1.0,d2=-1.0;
	int edge_type_1,edge_type_2;
	
	ui_EdgeStruct->target_type=-1;
	if (CheckSegment(p,ui_EdgeStruct->gradient,max_length_search,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) {
		if (ui_EdgeStruct->target_type!=-1) {
			t1=ui_EdgeStruct->target;
			d1=(t1-p).Norm2()+1.0;
			edge_type_1=ui_EdgeStruct->target_type;
		}
	}
	ui_EdgeStruct->target_type=-1;
	if (CheckSegment(p,-1.0*ui_EdgeStruct->gradient,max_length_search,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) {
		if (ui_EdgeStruct->target_type!=-1) {
			t2=ui_EdgeStruct->target;
			d2=(t2-p).Norm2()+1.0;
			edge_type_2=ui_EdgeStruct->target_type;
		}
	}




	double closest_match_dist;
	if (d1!=-1.0) {
		if (d2!=-1.0) {
			closest_match_dist=min(d1,d2);
			
			NewTarget.Target=t1;
			NewTarget.sqrt_weight=(float)(closest_match_dist/d1)*(closest_match_dist/d1);
			ADJUST_WEIGHT(edge_type_1);
			TargetList_2D->append(NewTarget);

			NewTarget.Target=t2;
			NewTarget.sqrt_weight=(float)(closest_match_dist/d2)*(closest_match_dist/d2);
			ADJUST_WEIGHT(edge_type_2);
			TargetList_2D->append(NewTarget);

		} else {
			closest_match_dist=d1;
			NewTarget.Target=t1;
			NewTarget.sqrt_weight=1.0;
			ADJUST_WEIGHT(edge_type_1);
			TargetList_2D->append(NewTarget);
		}
	} else {
		if (d2!=-1.0) {
			closest_match_dist=d2;
			NewTarget.Target=t2;
			NewTarget.sqrt_weight=1.0;
			ADJUST_WEIGHT(edge_type_2);
			TargetList_2D->append(NewTarget);
		}
	}
/*
	double d;
	Vector<2> t;
	Vector<2> dir;
	double spanned_distance;

	if (d1!=-1.0) {
		dir=ui_EdgeStruct->gradient;
		t=t1;
		d=d1;
		spanned_distance=0.0;

		while (true) {
			spanned_distance+=d-1.0+AICP_HOLE_PX;
			if (spanned_distance>max_length_search) break;

			ui_EdgeStruct->target_type=-1;
			if (!CheckSegment(t+(AICP_HOLE_PX*dir),dir,max_length_search-spanned_distance,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) break;
			if (ui_EdgeStruct->target_type==-1) break;
			
			t=ui_EdgeStruct->target;
			d=(t-p).Norm2()+1.0;

			NewTarget.Target=t;
			NewTarget.sqrt_weight=(float)(closest_match_dist/d)*(closest_match_dist/d);
			ADJUST_WEIGHT(ui_EdgeStruct->target_type);
			TargetList_2D->append(NewTarget);
		}
	}

	if (d2!=-1.0) {
		dir=-1.0*ui_EdgeStruct->gradient;
		t=t2;
		d=d2;
		spanned_distance=0.0;

		while (true) {
			spanned_distance+=d-1.0+AICP_HOLE_PX;
			if (spanned_distance>max_length_search) break;

			ui_EdgeStruct->target_type=-1;
			if (!CheckSegment(t+(AICP_HOLE_PX*dir),dir,max_length_search-spanned_distance,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) break;
			if (ui_EdgeStruct->target_type==-1) break;
			
			t=ui_EdgeStruct->target;
			d=(t-p).Norm2()+1.0;

			NewTarget.Target=t;
			NewTarget.sqrt_weight=(float)(closest_match_dist/d);
			ADJUST_WEIGHT(ui_EdgeStruct->target_type);
			TargetList_2D->append(NewTarget);
		}
	}
*/

	
			// (area^2)*( ( (1/2)^i * (1/rel_dist) )^2 )   
			//
			//  i         = 1 if incompatible edges
			//  rel_dist  = relative distance from the first match
			//



/*

	double f_dist=-1.0;
	for(int i=0;i<2;i++) {
		sp=p;
		double spanned_distance=0.0;
		
		do {
			ui_EdgeStruct->target_type=-1;
			if (!CheckSegment(sp,dir,max_length_search-spanned_distance,(CK_SEG_CALLBACK)FindCoherentEdge,ui_EdgeStruct)) break;
			if (ui_EdgeStruct->target_type==-1) break;
			
			NewTarget.Target=ui_EdgeStruct->target;

			// Compute NewTarget.sqrt_weight

				// Edge type inconsistency
				if (ui_EdgeStruct->gradient_type==ui_EdgeStruct->target_type) NewTarget.sqrt_weight=1.0;
				else NewTarget.sqrt_weight=0.5;
				// Relative distance 
				double distance=(ui_EdgeStruct->target-p).Norm2();
				if (f_dist==-1.0) f_dist=distance;
				else NewTarget.sqrt_weight*=(float)(f_dist/distance);        // puo essere piu' piccola!!! anche 0!!
				//  area
				NewTarget.sqrt_weight*=((float)mesh->AreaPoint[vertex_index]);

			
			TargetList_2D->append(NewTarget);

			sp=ui_EdgeStruct->target+(AICP_HOLE_PX*ui_EdgeStruct->gradient);
			spanned_distance+=distance+AICP_HOLE_PX;
		
		} while (spanned_distance<max_length_search);
		
		dir=-1.0*dir;
	}
*/


	// outliers that use weights!!




	/*
	//bool                 *taken_q=new bool[n_col];   // 3d points
	//bool                 *taken_m=new bool[n_row];   // 2d points
	//float                *min_dist_q=new float[n_col];   // 3d points
	//int                   multiple_targets=0;

	for(int q=0;q<n_col;q++) taken_q[q]=false;
	for(int m=0;m<n_row;m++) taken_m[m]=false;
	while(true) {
		int index=find_min(graph,n_row*n_col);
		int q=index%n_col;
		int m=index/n_col;

		if (graph[index]>salient_points_th) break;

		
		if (salient_points_association_method==1) {
			if (Uniform_int_rand(0,1)==1) {
				cout<<"changed";
				graph[index]=FLT_MAX;
				int index_m;
				while(true) {
					index_m=find_min(graph+q,n_row,n_col);
					if (graph[q+(index_m*n_col)]>salient_points_th) break;
					if (taken_m[index_m]==false) break;
					graph[q+(index_m*n_col)]=FLT_MAX;
				} 
				if ((graph[q+(index_m*n_col)]<=salient_points_th) && (taken_m[index_m]==false)) {
					index=q+(index_m*n_col);
					m=index_m;
				}
			}
		}
		// TODO

		if ((taken_q[q]==false) && (taken_m[m]==false)) {
			taken_q[q]=true;
			taken_m[m]=true;
			min_dist_q[q]=graph[index];
			TargetList_2D_Item_type tmp;
			tmp.VertexIndex=(*salient_points_vertices)[q];
			tmp.View=c_v;
			tmp.Source=c_v->Projection(MeshPoints[tmp.VertexIndex]);
			tmp.Target=(*points_2D)[m];
			tmp.sqrt_weight=sqrt_s_weight*((float)mesh->AreaPoint[tmp.VertexIndex]);
			tmp.type=3;
			TargetList_2D->append(tmp);
		} else {
			// Multiple targets
			bool to_add=true;
			if (taken_q[q]==true) {
				if ((100.0f*(graph[index]-min_dist_q[q])/min_dist_q[q])>40.0f) to_add=false;	
			}
			if (to_add) {
				if (taken_q[q]==false) {
					taken_q[q]=true;
					min_dist_q[q]=graph[index];
				}
				taken_m[m]=true;
				TargetList_2D_Item_type tmp;
				tmp.VertexIndex=(*salient_points_vertices)[q];
				tmp.View=c_v;
				tmp.Source=c_v->Projection(MeshPoints[tmp.VertexIndex]);
				tmp.Target=(*points_2D)[m];
				tmp.sqrt_weight=sqrt_s_weight*((float)mesh->AreaPoint[tmp.VertexIndex]);
				tmp.type=3;
				TargetList_2D->append(tmp);
				multiple_targets++;
			}
		}

		graph[index]=FLT_MAX;
	}
	*/
	//delete []taken_q;
	//delete []taken_m;
	//delete []min_dist_q;

	// TODO
