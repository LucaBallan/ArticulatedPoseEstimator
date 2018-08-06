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






void Articulated_ICP::WriteDebugImages(int Level, int frame_index) {
	if (Debug_Type!=4) {

		TargetList_1D->clear();
		TargetList_2D->clear();
		TargetList_3D->clear();

		for(int i=0;i<num_views;i++) {
			ResetBufferStatus();
			
			// Silhouette, Edges, Salient Points and Optical Flow
			{
				bool tmp_sdi=show_debug_information;
				show_debug_information=false;

				if (Debug_Type!=6) { // Get Correspondences
					TargetList_2D->clear(); // (speed up)
					TargetList_2D->append(Feat_TargetList);
					GetSilhouettesCorrespondences(i,Level);
					GetEdgesCorrespondences(i,Level);
					GetSalientPointCorrespondences(i,Level,frame_index);
				}
				
				
				show_debug_information=tmp_sdi;
				DebugSilhouette(i,Level);
			}
			
			// Depth
			DebugDepth(i,Level);

			// Texture
			// DebugTexture(i,Level);
		}
	}
}



void Articulated_ICP::AdaptInternalDebugBuffers(int width,int height) {
	if (DebugImage) {
		if ((width!=DebugImage->width) || (height!=DebugImage->height)) {
			SDELETE(DebugImage);
			SNEW(DebugImage,Bitmap<ColorRGB>(width,height));  // TODOSS: il resize tiene tanto -> nn usare diverse risoluzioni
		}
	} else {
		SNEW(DebugImage,Bitmap<ColorRGB>(width,height));
	}
}



















void Articulated_ICP::DebugSilhouette(int view,int Level) {
	if (Views[view].Mask==NULL) return;
	AdaptInternalDebugBuffers(Views[view].width,Views[view].height);


	// Debug images
	//
	// -> Disegna solo quelle Inliers 
	//    della vista corrente.
	if (Debug_Type==0) {
		int count_err=0,count_area=0;
		for(int i=0;i<(int)DebugImage->width;i++) {
			for(int j=0;j<(int)DebugImage->height;j++) {
				BYTE real_c=255-Views[view].Mask->Point(i,j);
				BYTE model_c=SilhouetteImage->Point(i,j);
				ColorRGB colore;
				colore.g=colore.b=colore.r=~(model_c^real_c);
				DebugImage->SetPoint(i,j,colore);
				if (real_c) count_area++;
				if (!colore.g) count_err++;
			}
		}
		SNEW(Views[view].PDE,Statistics(500));
		Views[view].PDE->AddData(count_err/(1.0f*count_area));
	} 
	if (Debug_Type==1) {
		for(int i=0;i<(int)DebugImage->width;i++) {
			for(int j=0;j<(int)DebugImage->height;j++) {
				BYTE model_c=SilhouetteImage->Point(i,j);
				ColorRGB colore;
				colore.g=colore.b=colore.r=model_c;
				DebugImage->SetPoint(i,j,colore);
			}
		}
	} 
	if (Debug_Type==2) {
		for(int i=0;i<(int)DebugImage->width;i++) {
			for(int j=0;j<(int)DebugImage->height;j++) {
				BYTE real_c=255-Views[view].Mask->Point(i,j);
				BYTE model_c=SilhouetteImage->Point(i,j);
				ColorRGB colore;
				colore.g=colore.b=colore.r=~(model_c|(real_c>>1));
				DebugImage->SetPoint(i,j,colore);
			}
		}
	} 
	if (Debug_Type==6) {
		SetNearFarPlane(view);
		Adapt_Texture_FBO_size(Views[view].width,Views[view].height);
		ModelRender->RenderWireFrame(&Texture_FBO,Views[view].width,Views[view].height,&(Views[view].Int),&(Views[view].ExtOGL),&(Views[view].Ext),mesh,0.0001,DebugImage);
		DebugImage->Flip_Vertical();
	}
	if (Debug_Type==7) {
		SetNearFarPlane(view);
		Adapt_Texture_FBO_size(Views[view].width,Views[view].height);
		ModelRender->RenderWireFrame(&Texture_FBO,Views[view].width,Views[view].height,&(Views[view].Int),&(Views[view].ExtOGL),&(Views[view].Ext),mesh,0.0001,DebugImage);
		DebugImage->Flip_Vertical();

		ColorRGB tmp_colore;
		tmp_colore.r=255;
		tmp_colore.g=153;
		tmp_colore.b=51;
		for(int i=0;i<TargetList_3D_ContactPoints->numElements();i++) {
			// TODOAAA does it work?
			Vector<3> x=mesh->Points[(*TargetList_3D_ContactPoints)[i].VertexIndex];
			Vector<2> p_=Views[view].Projection(x);
			int p[2];p[0]=Approx(p_[0]);p[1]=Approx(p_[1]);
			DebugImage->SetPoint(p[0]+1,p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0]+1,p[1],tmp_colore);
			DebugImage->SetPoint(p[0]+1,p[1]+1,tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1],tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1]+1,tmp_colore);
			DebugImage->SetPoint(p[0],p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0],p[1],tmp_colore);
			DebugImage->SetPoint(p[0],p[1]+1,tmp_colore);
			x=mesh->Points[(*TargetList_3D_ContactPoints)[i].VertexIndex_Target];
			p_=Views[view].Projection(x);
			p[0]=Approx(p_[0]);p[1]=Approx(p_[1]);
			DebugImage->SetPoint(p[0]+1,p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0]+1,p[1],tmp_colore);
			DebugImage->SetPoint(p[0]+1,p[1]+1,tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1],tmp_colore);
			DebugImage->SetPoint(p[0]-1,p[1]+1,tmp_colore);
			DebugImage->SetPoint(p[0],p[1]-1,tmp_colore);
			DebugImage->SetPoint(p[0],p[1],tmp_colore);
			DebugImage->SetPoint(p[0],p[1]+1,tmp_colore);
		}
	}

	if ((Debug_Type==3) || (Debug_Type==5)) {
		if ((EdgesSource&0x1) && (Views[view].Edges!=NULL) && (Views[view].Enable&VIEW_ENABLE_SILHOUETTE)) {
			//
			// cyan   + red/green borders  = actual silhouette from the images (red externals, green internals)
			// orange + white borders      = silhouette of the current estimate
			// 
			// blue                        = rendered & detected silhouette overlap
			//
			// violet borders              = external borders on the original video
			// 
			//
			for(int i=0;i<(int)DebugImage->width;i++) {
				for(int j=0;j<(int)DebugImage->height;j++) {
					ColorRGB color;
					BYTE real_c=Views[view].Mask->Point(i,j);
					BYTE model_c=SilhouetteImage->Point(i,j);
					if (!real_c) {
						if (!model_c) {
							color.r=0;
							color.g=255;
							color.b=255;
						} else {
							color.r=0;
							color.g=0;
							color.b=255;
						}
					} else {
						if (!model_c) {
							color.r=0;
							color.g=0;
							color.b=0;
						} else {
							color.r=222;
							color.g=139;
							color.b=44;
						}
					}
					DebugImage->SetPoint(i,j,color);
				}
			}

			ColorRGB color(255,255,255);
			for(int i=0;i<DebugImage->width;i++) {
				for(int j=0;j<DebugImage->height;j++) {
					if (EdgesImage->Point_NCB(i,j)>buffer_status.edge_threshold_r) DebugImage->SetPoint(i,j,color);
				}
			}

			
			EdgeSearch_struct ui_EdgeStruct;
			ui_EdgeStruct.mask=Views[view].Mask;
			ui_EdgeStruct.edge_map=Views[view].Edges;
			ui_EdgeStruct.edge_threshold=input_edges_images_edge_threshold;
			ui_EdgeStruct.win_d=(GRADIENT_WINDOW_SIZE-1)>>1;
			ui_EdgeStruct.win_d_for_input_mask=max(ui_EdgeStruct.win_d,edge_mask_max_distance);
			DebugEdgeDetectorOnImages(&ui_EdgeStruct,DebugImage);

		} else {
			for(int i=0;i<(int)DebugImage->width;i++) {
				for(int j=0;j<(int)DebugImage->height;j++) {
					BYTE real_c=255-Views[view].Mask->Point(i,j);
					BYTE model_c=SilhouetteImage->Point(i,j);
					ColorRGB colore;
					colore.r=0;
					colore.g=real_c^model_c;
					colore.b=real_c;
					DebugImage->SetPoint(i,j,colore);
				}
			}
		}
		
		
		if (true) {
			DebugImage->Clear(ColorRGB(0,0,0));

			for(int i=0;i<TargetList_2D->numElements();i++) {
				if ((*TargetList_2D)[i].View!=&Views[view]) continue;

				Vector<2> a=(*TargetList_2D)[i].Source;
				Vector<2> b=(*TargetList_2D)[i].Target;
				
				switch ((*TargetList_2D)[i].type) {
					case 0:
					case 1:
						if (Debug_Type!=5) {
							ColorRGB color_a(0xFF,0xFF,0xFF);
							ColorRGB color_b(0xFF,0xFF,0xFF);
							DrawLine(a,b,color_a,color_b,DebugImage,1.0f);
						}
						break;
					case 2:
						if (Debug_Type!=5) {
							ColorRGB color_a(0xFF,0xFF,0xFF);
							ColorRGB color_b(0xFF,0xFF,0xFF);
							DrawLine(a,b,color_a,color_b,DebugImage,1.0f);
						}
						break;
					case 3:
						{
							ColorRGB color_a2(0xFF,0xFF,0xFF);
							ColorRGB color_b2(0xFF,0xFF,0xFF);
							DrawLine(a,b,color_a2,color_b2,DebugImage,1.0f);
						}
						break;
				}
			}
		} else {
			for(int i=0;i<TargetList_2D->numElements();i++) {
				if ((*TargetList_2D)[i].View!=&Views[view]) continue;

				Vector<2> a=(*TargetList_2D)[i].Source;
				Vector<2> b=(*TargetList_2D)[i].Target;
				
				switch ((*TargetList_2D)[i].type) {
					case 0:
					case 1:
						if (Debug_Type!=5) {
						ColorRGB color_a(0x30,0x00,0x00);
						ColorRGB color_b(0xFF,0x00,0x00);
						DrawLine(a,b,color_a,color_b,DebugImage,(float)R_Lin_Interp(0.1,1.0,(*TargetList_2D)[i].sqrt_weight*(1.0/mesh->AreaPoint[(*TargetList_2D)[i].VertexIndex]),0.0,1.0));
						}
						break;
					case 2:
						if (Debug_Type!=5) {
						ColorRGB color_a(0x00,0x00,0x00);
						ColorRGB color_b(0x00,0xFF,0x00);
						DrawLine(a,b,color_a,color_b,DebugImage,0.9);
						}
						break;
					case 3:
						{
						ColorRGB color_a(0x00,0x00,0x00);
						ColorRGB color_b(0x50,0x50,0x50);
						DrawLine(a+VECTOR(0,1),b+VECTOR(0,1),color_a,color_b,DebugImage,(float)R_Lin_Interp(0.1,1.0,(*TargetList_2D)[i].sqrt_weight*(1.0/mesh->AreaPoint[(*TargetList_2D)[i].VertexIndex]),0.0,1.0));
						DrawLine(a+VECTOR(1,0),b+VECTOR(1,0),color_a,color_b,DebugImage,(float)R_Lin_Interp(0.1,1.0,(*TargetList_2D)[i].sqrt_weight*(1.0/mesh->AreaPoint[(*TargetList_2D)[i].VertexIndex]),0.0,1.0));
						DrawLine(a+VECTOR(2,0),b+VECTOR(2,0),color_a,color_b,DebugImage,(float)R_Lin_Interp(0.1,1.0,(*TargetList_2D)[i].sqrt_weight*(1.0/mesh->AreaPoint[(*TargetList_2D)[i].VertexIndex]),0.0,1.0));
						ColorRGB color_a2(0x00,0xFF,0x00);
						ColorRGB color_b2(0xFF,0x00,0x00);
						DebugImage->SetPoint((int)(b[0]),(int)(b[1]),color_b2);
						DrawLine(a,b,color_a2,color_b2,DebugImage,(float)R_Lin_Interp(0.1,1.0,(*TargetList_2D)[i].sqrt_weight*(1.0/mesh->AreaPoint[(*TargetList_2D)[i].VertexIndex]),0.0,1.0));
						}
						break;
				}
			}
		}
	}
	char Text[300];
	if (Level==0) sprintf(Text,"%s\\%i\\LF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,view,DebugIndex); // LF 
	else sprintf(Text,"%s\\%i\\LF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,view,DebugIndex);          // L1 
	DebugImage->Save(Text);
}


void Articulated_ICP::DebugEdgeDetectorOnImages(EdgeSearch_struct *ui_Info,Bitmap<ColorRGB> *output) {
	Vector<2>   dir;

	//output->CopyFrom(ui_Info->mask);

	for(int Px=0;Px<ui_Info->mask->width;Px++) {
		for(int Py=0;Py<ui_Info->mask->height;Py++) {

			if (ui_Info->mask->isOutside(Px,Py,ui_Info->win_d_for_input_mask)) continue;

			// Controllo se edge (o bordo -> se assumo che tutti i bordi vengano detectati come edges)
			if (ui_Info->edge_map->Point_NCB(Px,Py)>ui_Info->edge_threshold) {
				
				ColorRGB color(0xFF,0x00,0xFF);
				output->SetPoint(Px,Py,color);

				// Check if the edge_map is valid in Px,Py
				//      - move to the point Rx,Ry in the mask closest to the edge (Px,Py) if this is outside the silhouette
				//      - if I can't then it is an edge ouside the mask
				//
				int Rx,Ry;
				if (GetCloserPointZero_NCB(Px,Py,ui_Info->mask,Rx,Ry,ui_Info->win_d_for_input_mask)) {
					if (ui_Info->mask->isOutside(Rx,Ry,ui_Info->win_d)) continue;

					ColorRGB color(0x00,0xFF,0xFF);
					output->SetPoint(Px,Py,color);

					// Check if external edge
					dir=GetExternalEdgeDirection_NCB(Rx,Ry,ui_Info->mask,(GreyLevel)0xFF,ui_Info->win_d);
					if ((dir[0]==0) && (dir[1]==0)) {
						// Internal
						dir=GetInternalEdgeDirection_NCB(Px,Py,ui_Info->edge_map,(GreyLevel)ui_Info->edge_threshold,ui_Info->win_d);

						ColorRGB color(0x10,0xFF,0x10);
						output->SetPoint(Px,Py,color);
					} else {
						// External 
						ColorRGB color(0xFF,0x10,0x10);
						output->SetPoint(Px,Py,color);
					}
				}
			}
		}
	}

}

void Articulated_ICP::DebugEdgeDetectorOnRenderedImages(int view,float edge_threshold,int win_d,Bitmap<ColorRGB> *output) {
	Vector<2>   dir;
	Vector<3>   pr_pointz;
	Vector<3>  *MeshPoints=mesh->Points.getMem();
	
	output->CopyFrom(DepthImage,true);

	for(int j=0;j<mesh->num_p;j++,MeshPoints++) {
		pr_pointz=Views[view].ProjectionZ(*MeshPoints);
		if (pr_pointz[2]<=buffer_status.min_z_plane) continue;

		int     x=Approx(pr_pointz[0]);
		int     y=Approx(pr_pointz[1]);

		if (EdgesImage->isOutside(x,y,0)) continue;

		float gradient=EdgesImage->Point_NCB(x,y);

		if (gradient>edge_threshold) {

			float visible_z=Nearest_Float_Not_Bkg_Pixel(DepthImage,x,y);
			if (visible_z==FLT_MAX) continue;
			
			if (pr_pointz[2]<=visible_z+buffer_status.z_tollerance) {		
				// j visibile
				// x,y,pr_pointz[2] or pr_pointz[1,2,3]
			
				if (gradient==FLT_MAX) {
					// External edge
					dir=GetExternalEdgeDirection(x,y,DepthImage,win_d);

					Vector<2> a;a[0]=x;a[1]=y;
					Vector<2> b=a+10.0*dir;
					ColorRGB color(0x00,0xFF,0x00);
					DrawLine(a,b,color,output,0.8);
				} else {
					// Internal edge
					dir=GetInternalEdgeDirection(x,y,EdgesImage,edge_threshold,win_d);

					Vector<2> a;a[0]=x;a[1]=y;
					Vector<2> b=a+10.0*GetOrthogonalVersor(dir);
					ColorRGB color(0xFF,0x00,0x00);
					DrawLine(a,b,color,output,0.8);
				}
			}
		}
	}
}



void Articulated_ICP::DebugDepth(int view,int Level) {
	float  sqrt_d_weight;

	if (Views[view].Depth==NULL) return;
	if (Level==0) sqrt_d_weight=(float)sqrt(depth_local_weight_l0);
	else          sqrt_d_weight=(float)sqrt(depth_local_weight_l1);
	if (sqrt_d_weight==0.0) return;

	
	RenderDepthImage(view);
	AdaptInternalDebugBuffers(Views[view].width,Views[view].height);
	Differences(DepthImage,Views[view].Depth);

	char Text[300];
	if (Level==0) sprintf(Text,"%s\\%i\\DF_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,view,DebugIndex);
	else sprintf(Text,"%s\\%i\\D1_%04i."POSE_ESTIMATOR_DEBUG_FILE_EXTENSION,OutDebugDir,view,DebugIndex);
	DepthImage->Save(Text);
}












































		/*
		#ifdef RENDER_ALL_OPTICAL_FLOW
		{
			Array<typename Vector<2>> out_a(100);
			Array<typename Vector<2>> out_b(100);
			FeaturesList[view]->GetCommonFeatures(LastUsed_Feature_StartFrame,LastUsed_Feature_EndFrame,&out_a,&out_b);
			for(int h=0;h<out_a.numElements();h++) {
				ColorRGB color(0x00,0xFF,0x00);
				DrawLine(out_a[h],out_b[h],color,DebugImage);
			}
		}
		#endif

		for(int h=0;h<Feat_TargetList->numElements();h++) {
			if ((*Feat_TargetList)[h].View==&Views[view]) {
				Vector<2> a=Views[view].Projection(mesh->Points[(*Feat_TargetList)[h].VertexIndex]);
				Vector<2> b=(*Feat_TargetList)[h].Target;
				ColorRGB color(0x00,0xFF,0x00);
				DrawLine(a,b,color,DebugImage,0.9);
			}
		}
		*/
