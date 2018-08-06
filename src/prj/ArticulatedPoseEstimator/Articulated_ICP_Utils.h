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






////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// GENERIC ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool Nearest_Bkg_Pixel(Bitmap<GreyLevel> *Rendered,int x,int y) {
	GreyLevel *c_p=Rendered->getBuffer()+(x+(y*Rendered->width));

	// non controlla il vertice al centro perche' dovrebbe essere pieno 255 
	// controlla se i suoi vicini sono background per definirlo border
	// occhio al concetto dei bordi netti!!! 3dMax mettere !=255 è diverso da ==0

	if (*(c_p+1)==0) return true;
	if (*(c_p-1)==0) return true;

	c_p=c_p-Rendered->width;

	if (*c_p==0) return true;
	if (*(c_p+1)==0) return true;
	if (*(c_p-1)==0) return true;

	c_p=c_p+(Rendered->width<<1);

	if (*c_p==0) return true;
	if (*(c_p+1)==0) return true;
	if (*(c_p-1)==0) return true;

	return false;
}

bool GetCloserPointZero_NCB(int x,int y,Bitmap<GreyLevel> *im,int &rx,int &ry,int d_max) {
	rx=x;
	ry=y;
	if (im->Point_NCB(rx,ry)==0) return true;

	for(int d=1;d<d_max;d++) {
		for(rx=x-d,ry=y-d;rx<=x+d;rx++) {
			if (im->Point_NCB(rx,ry)==0) return true;
		}
		for(rx=x-d,ry=y+d;rx<=x+d;rx++) {
			if (im->Point_NCB(rx,ry)==0) return true;
		}
		for(ry=y-d+1,rx=x-d;ry<=y+d-1;ry++) {
			if (im->Point_NCB(rx,ry)==0) return true;
		}
		for(ry=y-d+1,rx=x+d;ry<=y+d-1;ry++) {
			if (im->Point_NCB(rx,ry)==0) return true;
		}
	}
	return false;
}

int GetMeshContourVertex(View *view,HyperMesh<3> *mesh,int *VertexList,Bitmap<GreyLevel> *mesh_image,double clip_min) {
	int v[2];
	int num=0;
	Vector<3> pr_pointz;
	Vector<3> *m_p=mesh->Points.getMem();
	

	for(int i=0;i<mesh->num_p;i++,m_p++) {
		pr_pointz=view->ProjectionZ(*m_p);
		if (pr_pointz[2]<=clip_min) continue;

		v[0]=Approx(pr_pointz[0]);
		v[1]=Approx(pr_pointz[1]);

		// Nota: Opengl disegna le faccie un po' tagliate (i loro edge sono accurati + o - un pixel) 
		//       questo causa problemi se la mesh nn e' molto fitta
		//       quindi uso una finestra di 2*r+1 pixel
		if (mesh_image->isOutside(v[0],v[1],1)) continue;
		if (Nearest_Bkg_Pixel(mesh_image,v[0],v[1])) VertexList[num++]=i;
	}
	
	return num;
}




inline double INTERNAL_AICP_SSD(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B,int &n_observations) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes do not match.");

	int         size_p = A->width*A->height;
	ColorFloat    *p_A = (ColorFloat*)A->getBuffer();
	ColorFloat    *p_B = (ColorFloat*)B->getBuffer();

	double sum=0.0;
	int used_pixels=0;
	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if (((*p_A)!=FLT_MAX) && ((*p_B)!=FLT_MAX)) {
			double tmp=(*p_A)-(*p_B);
			sum+=tmp*tmp;
			used_pixels++;
		}
	}

	n_observations=used_pixels;
	if (used_pixels!=0) return (sum/used_pixels);
	else return 0.0;
}

inline double INTERNAL_AICP_SAD(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *B,int &n_observations) {
	if ((A->width!=B->width) || (A->height!=B->height)) ErrorExit("Bitmap sizes do not match.");

	int         size_p = A->width*A->height;
	ColorFloat    *p_A = (ColorFloat*)A->getBuffer();
	ColorFloat    *p_B = (ColorFloat*)B->getBuffer();

	double sum=0.0;
	int used_pixels=0;
	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if (((*p_A)!=FLT_MAX) && ((*p_B)!=FLT_MAX)) {
			double tmp=(*p_A)-(*p_B);
			sum+=fabs(tmp);
			used_pixels++;
		}
	}

	n_observations=used_pixels;
	if (used_pixels!=0) return (sum/used_pixels);
	else return 0.0;
}

inline double INTERNAL_AICP_SAD_RGBA(Bitmap<ColorRGBA> *A,Bitmap<ColorRGB> *B,Bitmap<GreyLevel> *MB,int &n_observations) {
	if ((A->width!=B->width)  || (A->height!=B->height))  ErrorExit("Bitmap sizes do not match.");
	if ((A->width!=MB->width) || (A->height!=MB->height)) ErrorExit("Bitmap sizes do not match.");

	int         size_p  = A->width*A->height;
	ColorRGBA     *p_A  = (ColorRGBA*)A->getBuffer();
	ColorRGB      *p_B  = (ColorRGB* )B->getBuffer();
	GreyLevel     *p_MB = (GreyLevel*)MB->getBuffer();

	double sum=0.0;
	int used_pixels=0;
	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if ((ALPHA_C(*p_A)==255) && ((*p_MB)==0)) {  // TODOTEXTURE is zero ok in ambedue i casi?
			int tmp_r=((int)RED_C(*p_A))   - ((int)(p_B->r));
			int tmp_g=((int)GREEN_C(*p_A)) - ((int)(p_B->g));
			int tmp_b=((int)BLUE_C(*p_A))  - ((int)(p_B->b));
			sum+=sqrt((double)tmp_r*tmp_r+(double)tmp_g*tmp_g+(double)tmp_b*tmp_b);
			used_pixels++;
		}
	}

	n_observations=used_pixels;
	if (used_pixels!=0) return (sum/used_pixels);
	else return 0.0;
}

inline double INTERNAL_AICP_SSD_RGBA(Bitmap<ColorRGBA> *A,Bitmap<ColorRGB> *B,Bitmap<GreyLevel> *MB,int &n_observations) {
	if ((A->width!=B->width)  || (A->height!=B->height))  ErrorExit("Bitmap sizes do not match.");
	if ((A->width!=MB->width) || (A->height!=MB->height)) ErrorExit("Bitmap sizes do not match.");

	int         size_p  = A->width*A->height;
	ColorRGBA     *p_A  = (ColorRGBA*)A->getBuffer();
	ColorRGB      *p_B  = (ColorRGB* )B->getBuffer();
	GreyLevel     *p_MB = (GreyLevel*)MB->getBuffer();

	double sum=0.0;
	int used_pixels=0;
	for(int i=0;i<size_p;i++,p_A++,p_B++) {
		if ((ALPHA_C(*p_A)==255) && ((*p_MB)==0)) {  // TODOTEXTURE is ok in ambedue i casi? 
			int tmp_r=((int)RED_C(*p_A))   - ((int)(p_B->r));
			int tmp_g=((int)GREEN_C(*p_A)) - ((int)(p_B->g));
			int tmp_b=((int)BLUE_C(*p_A))  - ((int)(p_B->b));
			sum+=(double)tmp_r*tmp_r+(double)tmp_g*tmp_g+(double)tmp_b*tmp_b;
			used_pixels++;
		}
	}

	n_observations=used_pixels;
	if (used_pixels!=0) return (sum/used_pixels);
	else return 0.0;
}




















////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// INTERNAL BUFFERS //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::ResetBufferStatus() {
	buffer_status.SilhouetteReady=false;
	buffer_status.DepthReady=false;
	buffer_status.EdgesReady=false;
	buffer_status.TextureReady=false;
	buffer_status.NearFarPlaneReady=false;
	buffer_status.max_z_plane=-1.0;
	buffer_status.min_z_plane=-1.0;
	buffer_status.z_tollerance=-1.0f;
	buffer_status.edge_threshold_r=-1.0f;
}

void Articulated_ICP::SetNearFarPlane(int v_i) {

	if (!buffer_status.NearFarPlaneReady) {
		double min_z,max_z;
		GetNearFarPlane(mesh,&(Views[v_i].Ext),min_z,max_z);
		max_z=max_z+max_z*0.001;
		min_z=min_z-min_z*0.001;
		if (min_z<0.0) {
			Warning("One of the cameras intersect the object. Clipping plane set to 0.1.");
			min_z=0.1;
		}
		buffer_status.min_z_plane=min_z;
		buffer_status.max_z_plane=max_z;
		buffer_status.z_tollerance=(float)((max_z-min_z)*AICP_ZBUFFER_TOLLERANCE);
		buffer_status.NearFarPlaneReady=true;
	} 
	
	GPU_OBJ->DefineNearFarPlane(buffer_status.min_z_plane,buffer_status.max_z_plane);
}

void Articulated_ICP::RenderSilhouetteImage(int v_i) {
	// TODOSS si puo' semplificare estraendolo dallo Z-Buffer!!!
	if (!buffer_status.SilhouetteReady) {
		SetNearFarPlane(v_i);
		Adapt_Silhouette_FBO_size(Views[v_i].width,Views[v_i].height);
		ModelRender->RenderFootPrint(&Silhouette_FBO,Views[v_i].width,Views[v_i].height,&(Views[v_i].Int),&(Views[v_i].ExtOGL),&(Views[v_i].Ext),mesh,SilhouetteImage);
		SilhouetteImage->Flip_Vertical();

		buffer_status.SilhouetteReady=true;
	}
}

void Articulated_ICP::RenderDepthImage(int v_i) {
	
	if (!buffer_status.DepthReady) {
		SetNearFarPlane(v_i);
		Adapt_Depth_FBO_size(Views[v_i].width,Views[v_i].height);
		DepthRender->RenderDepthMap(&Depth_FBO,Views[v_i].width,Views[v_i].height,&(Views[v_i].Int),&(Views[v_i].ExtOGL),&(Views[v_i].Ext),mesh,DepthImage);
		DepthImage->Flip_Vertical();

		buffer_status.DepthReady=true;
	}
}

void Articulated_ICP::RenderEdgesImage(int v_i) {
	if (!buffer_status.EdgesReady) {

		if (EdgesSource&0x2) {
			RenderTextureImage(v_i);
			// TODOTEXTURE non penso funzioni!!!

			Adapt_Edges_FBO_size(Views[v_i].width,Views[v_i].height);
			// EdgeFilter standard usato per il codice ...-> ***(&Edges_FBO,Texture_FBO.fbt,EdgesImage);
			// EdgesImage->Flip_Vertical();
			buffer_status.edge_threshold_r=(float)textured_model_edge_threshold;
		} else {
			RenderDepthImage(v_i);

			Adapt_Edges_FBO_size(Views[v_i].width,Views[v_i].height);
			EdgesRenderer->FilterDepth(&Edges_FBO,Depth_FBO.fbt,EdgesImage);
			EdgesImage->Flip_Vertical();
			
			buffer_status.edge_threshold_r=(float)(depth_model_edge_threshold*buffer_status.z_tollerance);
		}

		buffer_status.EdgesReady=true;
	}
}

void Articulated_ICP::RenderTextureImage(int v_i) {

	if (!buffer_status.TextureReady) {
		if (mesh->Texture_GLMap[0]==INVALID_TEXTURE) ErrorExit("Cannot render a mesh without texture");

		RenderDepthImage(v_i);

		Adapt_Texture_FBO_size(Views[v_i].width,Views[v_i].height);
		SetNearFarPlane(v_i);
		ModelRender->RenderTexRGBA(&Texture_FBO,Views[v_i].width,Views[v_i].height,&(Views[v_i].Int),&(Views[v_i].ExtOGL),mesh,TextureImage);
		TextureImage->Flip_Vertical();

		buffer_status.TextureReady=true;
	}
}




























////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// DISTANCES /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Articulated_ICP::PrepareFrameForSilhouetteResidual() {
	if (!DistanceTransform_Buffers) SNEWA_P(DistanceTransform_Buffers,SilhouetteDistance_Buffer*,num_views);
	
	for(int i=0;i<num_views;i++) {
		if (Views[i].Mask!=NULL) {
			if (DistanceTransform_Buffers[i]==NULL) SNEW(DistanceTransform_Buffers[i],SilhouetteDistance_Buffer(Views[i].width,Views[i].height));
			DistanceTransform_Buffers[i]->PrepareImageA(Views[i].Mask,false,-1);
			
			// Mask out DistanceTransform_Buffers[i]->iia using Views[i].i_grey[4]
			if (Views[i].i_grey[4]!=NULL) {
				GreyLevel *invalid_areas=Views[i].i_grey[4]->getBuffer();
				for(int y=0;y<Views[i].height;y++) {
					for(int x=0;x<Views[i].width;x++,invalid_areas++) {
						if (*invalid_areas==255) {
							cvSet2D(DistanceTransform_Buffers[i]->iia,y,x,cvScalar(0,0,0,0));
						}
					}
				}
			}

		}
	}
}



/*
// TODOAPPROX
void Articulated_ICP::ComputeGlobalResiduals(double *residuals,double *n_observations) {
	int used_views[3];
	for(int i=0;i<3;i++) used_views[i]=0;

	for(int i=0;i<5;i++) {
		residuals[i]=0.0;
		n_observations[i]=0.0;
	}
	for(int i=0;i<num_views;i++) {
		ResetBufferStatus();
		
		if (SilhouetteResidual(i,residuals[0],n_observations[0])) used_views[0]++;
		if (DepthResidual     (i,residuals[1],n_observations[1])) used_views[1]++;
		if (TextureResidual   (i,residuals[2],n_observations[2])) used_views[2]++;
	}

	OpticalFlowResidual         (residuals[3],n_observations[3]);
	CollisionResidual           (residuals[4],n_observations[4]);


	for(int i=0;i<3;i++) 
		if (used_views[i]!=0) residuals[i]/=used_views[i];

}

bool Articulated_ICP::TextureResidual(double &residual, double &n_observations) {
	int tmp_n_observations;


	if ((Views[i].Img!=NULL) && (Views[i].Mask!=NULL)) {
		RenderTextureImage(i); // Same rules here!!!

		#ifdef AICP_USE_L2_INSTEAD
		residual+=INTERNAL_AICP_SSD_RGBA(TextureImage,Views[i].Img,Views[i].Mask,tmp_n_observations);
		#else
		residual+=INTERNAL_AICP_SAD_RGBA(TextureImage,Views[i].Img,Views[i].Mask,tmp_n_observations);
		#endif
		
		n_observations+=(double)tmp_n_observations;
		
		return true;
	}

	return false;
}

*/


/*
void Articulated_ICP::SilhouetteEdgesSalientPointsResidual(double &residual_SE,double &residual_P,double *n_observations_SE,double *n_observations_P) {
	int tmp_n_observations;

	int used_views=0;
	double distance=0.0;
	(*n_observations)=0.0;
	for(int i=0;i<num_views;i++) {
		if (Views[i].Mask!=NULL) {
			if ((EdgesSource&0x1) && (Views[i].Edges!=NULL) && (c_v->Enable&VIEW_ENABLE_SILHOUETTE)) {
				
				// TODOLUCA salient anche se solo silhouette

				// TODOLUCA
				//double Residual_S=0.0,
				//	   Residual_E=0.0,
				//	   Residual_P=0.0,
				//	   Residual_O=0.0;
				//int    n_observations_S=0,
				//	   n_observations_E=0,
				//	   n_observations_P=0,
				//	   n_observations_O=0;
				//bool tmp_sdi=show_debug_information;
				//show_debug_information=false;
				//{
				//	TargetList_2D->clear();
				//	ResetBufferStatus();
				//	GetEdgesCorrespondences(i,int Level);
				//	GetSalientPointCorrespondences(i,int Level,int frame_index);
				//	SilhouetteEdgesSalientPointsOpticalFlow_CorrespondencesResidual(Residual_S,Residual_E,Residual_P,Residual_O,&n_observations_S,&n_observations_E,&n_observations_P,&n_observations_O);
				//	// n_observations_S = 0 (sempre)
				//	// n_observations_O = 0 (sempre)
				//}
				//show_debug_information=tmp_sdi;
				
					
				// TODOLUCA


				
				// Edges + Silhouettes
				ResetBufferStatus();					 // TODOSS All these can be speeded up!!!!
				RenderSilhouetteImage(i);
				RenderDepthImage(i);
				RenderEdgesImage(i);
				
				// Actual rendered edges (internal or external)
				Bitmap<GreyLevel> Rendered(EdgesImage->width,EdgesImage->height);
				for(int y=0;y<EdgesImage->height;y++) {
					for(int x=0;x<EdgesImage->width;x++) {
						if (EdgesImage->Point_NCB(x,y)>buffer_status.edge_threshold_r) Rendered.SetPoint(x,y,0);
						else Rendered.SetPoint(x,y,255);
					}
				}

				// True Edges (discard the one outside the mask) can be done as pre processing TODOSS
				int win_d_for_input_mask=max((GRADIENT_WINDOW_SIZE-1)>>1,edge_mask_max_distance);
				Bitmap<GreyLevel> *c_m=Views[i].Mask;
				Bitmap<GreyLevel> *c_e=Views[i].Edges;
				Bitmap<GreyLevel> Source(EdgesImage->width,EdgesImage->height);
				
				for(int y=0;y<EdgesImage->height;y++) {
					for(int x=0;x<EdgesImage->width;x++) {
						if (c_e->Point_NCB(x,y)>input_edges_images_edge_threshold) {
							int Rx,Ry;
							if (!(c_e->isOutside(x,y,win_d_for_input_mask))) {
								if (GetCloserPointZero_NCB(x,y,c_m,Rx,Ry,win_d_for_input_mask)) Source.SetPoint(x,y,0);
								else Source.SetPoint(x,y,255);
							} else Source.SetPoint(x,y,255);
						} else Source.SetPoint(x,y,255);
					}
				}

				// TODOLUCA 
				Source.Save("ImSource.jpg");
				Rendered.Save("ImDest.jpg");

				// TODOLUCA funziona sta cosa?
				DistanceTransform_Buffers[i]->PrepareImageA(&Source,false,-1);   // TODOEDGE Skrew up everything
				DistanceTransform_Buffers[i]->PrepareImageB(&Rendered,false,-1); // TODOEDGE

				cvSaveImage("ImSourceD.jpg",DistanceTransform_Buffers[i]->iia);
				cvSaveImage("ImDestD.jpg",DistanceTransform_Buffers[i]->iib);

				#ifdef AICP_USE_L2_INSTEAD
				distance+=SilhouetteDistanceMT_mse(DistanceTransform_Buffers[i],&tmp_n_observations);
				#else
				distance+=SilhouetteDistanceMT(DistanceTransform_Buffers[i],&tmp_n_observations);
				#endif

				
				//cout<<distance<<"\n";
				(*n_observations)+=(double)tmp_n_observations;
				
				//// Edges contano 30 volte di piu'
				//DistanceTransform_Buffers[i]->PrepareImageA(Views[i].Mask,false,-1);  // Skrew up everything
				//DistanceTransform_Buffers[i]->PrepareImageB(SilhouetteImage,true,128);

				//#ifdef AICP_USE_L2_INSTEAD
				//distance+=SilhouetteDistanceMT_mse(DistanceTransform_Buffers[i],&tmp_n_observations);
				//#else
				//distance+=SilhouetteDistanceMT(DistanceTransform_Buffers[i],&tmp_n_observations);
				//#endif
				//
				//cout<<distance;
				//(*n_observations)+=(double)tmp_n_observations;
				used_views++;

			} else {
				// Silhouettes
				ResetBufferStatus();
				RenderSilhouetteImage(i);

				DistanceTransform_Buffers[i]->PrepareImageB(SilhouetteImage,true,128);
				#ifdef AICP_USE_L2_INSTEAD
				distance+=SilhouetteDistanceMT_mse(DistanceTransform_Buffers[i],&tmp_n_observations);
				#else
				distance+=SilhouetteDistanceMT(DistanceTransform_Buffers[i],&tmp_n_observations);
				#endif
				(*n_observations)+=(double)tmp_n_observations;
				used_views++;
			}
		}
	}
	if (used_views!=0) return (distance/used_views);
	else return 0.0;
}
*/




double Articulated_ICP::ComputaEdgesResidual(double *n_observations) {
	int tmp_n_observations;

	int used_views=0;
	double distance=0.0;
	(*n_observations)=0.0;
	for(int i=0;i<num_views;i++) {
		if (Views[i].Mask!=NULL) {
			if ((EdgesSource&0x1) && (Views[i].Edges!=NULL) && (Views[i].Enable&VIEW_ENABLE_SILHOUETTE)) {
				
				// Edges + Silhouettes
				ResetBufferStatus();					 // TODOSS All these can be speeded up!!!!
				RenderSilhouetteImage(i);
				RenderDepthImage(i);
				RenderEdgesImage(i);
				
				// Actual rendered edges (internal or external)
				Bitmap<GreyLevel> Rendered(EdgesImage->width,EdgesImage->height);
				for(int y=0;y<EdgesImage->height;y++) {
					for(int x=0;x<EdgesImage->width;x++) {
						if (EdgesImage->Point_NCB(x,y)>buffer_status.edge_threshold_r) Rendered.SetPoint(x,y,0);
						else Rendered.SetPoint(x,y,255);
					}
				}

				// True Edges (discard the one outside the mask) can be done as pre processing TODOSS
				int win_d_for_input_mask=max((GRADIENT_WINDOW_SIZE-1)>>1,edge_mask_max_distance);
				Bitmap<GreyLevel> *c_m=Views[i].Mask;
				Bitmap<GreyLevel> *c_e=Views[i].Edges;
				Bitmap<GreyLevel> Source(EdgesImage->width,EdgesImage->height);
				
				for(int y=0;y<EdgesImage->height;y++) {
					for(int x=0;x<EdgesImage->width;x++) {
						if (c_e->Point_NCB(x,y)>input_edges_images_edge_threshold) {
							int Rx,Ry;
							if (!(c_e->isOutside(x,y,win_d_for_input_mask))) {
								if (GetCloserPointZero_NCB(x,y,c_m,Rx,Ry,win_d_for_input_mask)) Source.SetPoint(x,y,0);
								else Source.SetPoint(x,y,255);
							} else Source.SetPoint(x,y,255);
						} else Source.SetPoint(x,y,255);
					}
				}

				// TODOLUCA 
				//Source.Save("ImSource.jpg");
				//Rendered.Save("ImDest.jpg");

				// TODOLUCA funziona sta cosa?
				DistanceTransform_Buffers[i]->PrepareImageA(&Source,false,-1);   // TODOEDGE Skrew up everything
				DistanceTransform_Buffers[i]->PrepareImageB(&Rendered,false,-1); // TODOEDGE

				//cvSaveImage("ImSourceD.jpg",DistanceTransform_Buffers[i]->iia);
				//cvSaveImage("ImDestD.jpg",DistanceTransform_Buffers[i]->iib);

				#ifdef AICP_USE_L2_INSTEAD
				distance+=SilhouetteDistanceMT_mse(DistanceTransform_Buffers[i],&tmp_n_observations);
				#else
				distance+=SilhouetteDistanceMT(DistanceTransform_Buffers[i],&tmp_n_observations);
				#endif

				
				(*n_observations)+=(double)tmp_n_observations;
				used_views++;
			}
		}
	}
	if (used_views!=0) return (distance/used_views);
	else return 0.0;
}

double Articulated_ICP::ComputaSilhouetteResidual(double *n_observations) {
	int tmp_n_observations;

	int used_views=0;
	double distance=0.0;
	(*n_observations)=0.0;
	for(int i=0;i<num_views;i++) {
		if ((Views[i].Mask!=NULL) && (Views[i].Enable&VIEW_ENABLE_SILHOUETTE)) {
			ResetBufferStatus();
			RenderSilhouetteImage(i);

			DistanceTransform_Buffers[i]->PrepareImageB(SilhouetteImage,true,128);
			#ifdef AICP_USE_L2_INSTEAD
			distance+=SilhouetteDistanceMT_mse(DistanceTransform_Buffers[i],&tmp_n_observations);
			#else
			distance+=SilhouetteDistanceMT(DistanceTransform_Buffers[i],&tmp_n_observations);
			#endif
			(*n_observations)+=(double)tmp_n_observations;
			used_views++;
		}
	}
	if (used_views!=0) return (distance/used_views);
	else return 0.0;
}

double Articulated_ICP::ComputaDepthResidual(double *n_observations) {
	int tmp_n_observations;

	int used_views=0;
	double mean_distance=0.0;
	(*n_observations)=0.0;
	for(int i=0;i<num_views;i++) {
		if (Views[i].Depth!=NULL) {
			ResetBufferStatus();
			RenderDepthImage(i);

			#ifdef AICP_USE_L2_INSTEAD
			mean_distance+=INTERNAL_AICP_SSD(DepthImage,Views[i].Depth,tmp_n_observations);
			#else
			mean_distance+=INTERNAL_AICP_SAD(DepthImage,Views[i].Depth,tmp_n_observations);
			#endif
			(*n_observations)+=(double)tmp_n_observations;
			used_views++;
		}
	}
	if (used_views!=0) return (mean_distance/used_views);
	else return 0.0;
}

double Articulated_ICP::ComputaEdgeResidual(double *n_observations) {
	(*n_observations)=0.0;
	if (!(EdgesSource&0x1)) return 0.0;

	TargetList_2D->clear();
	bool tmp_sdi=show_debug_information;
	show_debug_information=false;
	for(int i=0;i<num_views;i++) {
		ResetBufferStatus();
		GetEdgesCorrespondences(i,0);
	}
	show_debug_information=tmp_sdi;

	Vector<2> PP2;
	Vector<3> tmp_p;
	Vector<3> *Points=mesh->Points.getMem();
	double    *AreaPoint=mesh->AreaPoint.getMem();
	double     mean_distance=0.0;
	double     Total_Weight=0.0;

	for(int h=0;h<TargetList_2D->numElements();h++) {
		int point_index=(*TargetList_2D)[h].VertexIndex;
		tmp_p=Points[point_index];
		double Target_Weight=AreaPoint[point_index];

		PP2=(*TargetList_2D)[h].View->Projection(tmp_p)-(*TargetList_2D)[h].Target;
		#ifdef AICP_USE_L2_INSTEAD
		mean_distance+=Target_Weight*(PP2*PP2);
		#else
		mean_distance+=Target_Weight*(PP2.Norm2());
		#endif
		Total_Weight+=Target_Weight;
	}

	(*n_observations)=(double)TargetList_2D->numElements();
	if (Total_Weight!=0.0) return (mean_distance/Total_Weight);
	else return 0.0;
}

double Articulated_ICP::ComputaSalientPointResidual(double *n_observations,int frame_index) {
	(*n_observations)=0.0;
	
	TargetList_2D->clear();
	bool tmp_sdi=show_debug_information;
	show_debug_information=false;
	for(int i=0;i<num_views;i++) {
		ResetBufferStatus();
		GetSalientPointCorrespondences(i,0,frame_index);
	}
	show_debug_information=tmp_sdi;


	Vector<2> PP2;
	Vector<3> tmp_p;
	Vector<3> *Points=mesh->Points.getMem();
	double    *AreaPoint=mesh->AreaPoint.getMem();
	double     mean_distance=0.0;
	double     Total_Weight=0.0;

	for(int h=0;h<TargetList_2D->numElements();h++) {
		int point_index=(*TargetList_2D)[h].VertexIndex;
		tmp_p=Points[point_index];
		double Target_Weight=AreaPoint[point_index];

		PP2=(*TargetList_2D)[h].View->Projection(tmp_p)-(*TargetList_2D)[h].Target;
		#ifdef AICP_USE_L2_INSTEAD
		mean_distance+=Target_Weight*(PP2*PP2);
		#else
		mean_distance+=Target_Weight*(PP2.Norm2());
		#endif
		Total_Weight+=Target_Weight;
	}

	(*n_observations)=(double)TargetList_2D->numElements();
	if (Total_Weight!=0.0) return (mean_distance/Total_Weight);
	else return 0.0;
}

double Articulated_ICP::ComputaOpticalFlowResidual(double *n_observations) {
	Vector<2> PP2;
	Vector<3> tmp_p;
	Vector<3> *Points=mesh->Points.getMem();
	double    *AreaPoint=mesh->AreaPoint.getMem();
	double     mean_distance=0.0;
	double     Total_Weight=0.0;

	for(int h=0;h<Feat_TargetList->numElements();h++) {
		int point_index=(*Feat_TargetList)[h].VertexIndex;
		tmp_p=Points[point_index];
		double Target_Weight=AreaPoint[point_index];

		PP2=(*Feat_TargetList)[h].View->Projection(tmp_p)-(*Feat_TargetList)[h].Target;
		#ifdef AICP_USE_L2_INSTEAD
		mean_distance+=Target_Weight*(PP2*PP2);
		#else
		mean_distance+=Target_Weight*(PP2.Norm2());
		#endif
		Total_Weight+=Target_Weight;
	}

	(*n_observations)=Feat_TargetList->numElements();
	if (Total_Weight!=0.0) return (mean_distance/Total_Weight);
	else return 0.0;
}

double Articulated_ICP::ComputaCollisionsResidual() {
	if (!collision_detector) return 0.0;

	collision_detector->UpdateBoundingBoxes();
	collision_intersecting_faces->clear();
	collision_detector->SelfIntersection_BinaryTree_Skinned_Mesh(collision_intersecting_faces,collision_mask);
	int num_face_pairs=collision_intersecting_faces->numElements();

	return (double)(num_face_pairs);
}

double Articulated_ICP::ComputaTextureResidual(double *n_observations) {
	int tmp_n_observations;

	int used_views=0;
	double mean_distance=0.0;
	(*n_observations)=0.0;
	for(int i=0;i<num_views;i++) {
		if ((Views[i].Img!=NULL) && (Views[i].Mask!=NULL)) {
			ResetBufferStatus();
			RenderTextureImage(i);

			#ifdef AICP_USE_L2_INSTEAD
			mean_distance+=INTERNAL_AICP_SSD_RGBA(TextureImage,Views[i].Img,Views[i].Mask,tmp_n_observations);
			#else
			mean_distance+=INTERNAL_AICP_SAD_RGBA(TextureImage,Views[i].Img,Views[i].Mask,tmp_n_observations);
			#endif
			(*n_observations)+=(double)tmp_n_observations;
			used_views++;
		}
	}
	if (used_views!=0) return (mean_distance/used_views);
	else return 0.0;
}






























////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// OUTLIERS ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::Clean_TargetList_2D_FromOutliers(int first_element_to_check) {
	// Elimina Spostamenti troppo alti -> occhio devo farlo con gli altri...
	// Posso usare lo z-buffer per vedere se è ostruito da un'altro oggetto...
	// TODO: eliminare le forze la cui entità è molto alta rispetto alle altre del bone a cui sono associate.. (rispetto alla singola vista)

	//
	// Calcola la media degli spostamenti
	//
	for(int i=0;i<mesh->num_bones;i++) {
		mean_move[i]=0.0;
		mean_move_num[i]=0;
	}
	for(int i=first_element_to_check;i<TargetList_2D->numElements();i++) {
		int b_a=mesh->GetBoneIDAttachedtoVertex((*TargetList_2D)[i].VertexIndex,0.0);
		if (b_a==-1) {
			TargetList_2D->del(i);
			i--;
			continue;
		}
		Bone *tmpBone=mesh->BoneRefTable[b_a];
		if (!tmpBone->CanRotateOrTranslate()) {
			//if (tmpBone->AllowPropagation()) {
				do {
					do {
						tmpBone=static_cast<Bone *>(tmpBone->Father);
					} while((tmpBone!=NULL) && (!tmpBone->CanRotateOrTranslate()));
					b_a=mesh->GetBoneID(tmpBone);
				} while((tmpBone!=NULL) && (b_a==-1));
				if (b_a==-1) {
					TargetList_2D->del(i);
					i--;
					continue;
				}
			//} else {
			//	TargetList_2D->del(i);
			//	i--;
			//	continue;
			//}
		}
		
		mean_move[b_a]+=((*TargetList_2D)[i].Source-(*TargetList_2D)[i].Target).Norm2();
		mean_move_num[b_a]++;
	}
	for(int i=0;i<mesh->num_bones;i++) {
		if (mean_move_num[i]!=0) mean_move[i]/=mean_move_num[i];
		if (mean_move[i]<MEAN_MIN_PIXELS) mean_move[i]=MEAN_MIN_PIXELS;
	}


	// Impone che gli spostamenti di entità elevata rispetto alla
	// media e alla soglia SMOOTH_MOVEMENT siano esclusi,
	// ritenuti outliers.
	for(int i=first_element_to_check;i<TargetList_2D->numElements();i++) {
		int b_a=mesh->GetBoneIDAttachedtoVertex((*TargetList_2D)[i].VertexIndex,0.0);

		Bone *tmpBone=mesh->BoneRefTable[b_a];
		if (!tmpBone->CanRotateOrTranslate()) {
			do {
				tmpBone=static_cast<Bone *>(tmpBone->Father);
			} while(!tmpBone->CanRotateOrTranslate());
			b_a=mesh->GetBoneID(tmpBone);
		}

		double mov_ent=((*TargetList_2D)[i].Source-(*TargetList_2D)[i].Target).Norm2();
		if (mean_move_num[b_a]<NUMERO_MIN_PER_MEDIA_VALIDA) continue;
		if (mov_ent>caotic_smooth_tollerance*mean_move[b_a]) {
			
			if (tmpBone->Level<=3) continue;	// TODOLUCA

			TargetList_2D->del(i);
			i--;
			continue;
		}
	}
}

















////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PREDICTION  ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Articulated_ICP::GetOnlyPredictionCorrespondences(Skinned_Mesh *ref_mesh,int base_frame_index) {
	TargetList_1D->clear();
	TargetList_2D->clear();
	TargetList_3D->clear();


	double            tmp_data[3];
	Array<Vector<3>>  Points_t_2(ref_mesh->num_p);
	Array<Vector<3>>  Points_t_1(ref_mesh->num_p);
	Array<Vector<3>>  Points_t_0(ref_mesh->num_p);

	float selective_step=0.0f;
	if ((ref_mesh->num_p)>max_prediction_correspondences)
		selective_step=(ref_mesh->num_p*1.0f/max_prediction_correspondences)-1.0f;


	ref_mesh->Apply_Skin(base_frame_index-2);
	for(int i=0;i<ref_mesh->num_p;i++) Points_t_2.append(ref_mesh->Points[i]);
	ref_mesh->Apply_Skin(base_frame_index-1);
	for(int i=0;i<ref_mesh->num_p;i++) Points_t_1.append(ref_mesh->Points[i]);
	ref_mesh->Apply_Skin(base_frame_index);
	for(int i=0;i<ref_mesh->num_p;i++) Points_t_0.append(ref_mesh->Points[i]);


	float accumulation=0.0;
	for(int i=0;i<ref_mesh->num_p;i++) {
		TargetList_3D_Item_type x;

		tmp_data[0]=Points_t_0[i][0];
		tmp_data[1]=Points_t_1[i][0];
		tmp_data[2]=Points_t_2[i][0];
		x.Target[0]=PolynomialPredictor(3,tmp_data);

		tmp_data[0]=Points_t_0[i][1];
		tmp_data[1]=Points_t_1[i][1];
		tmp_data[2]=Points_t_2[i][1];
		x.Target[1]=PolynomialPredictor(3,tmp_data);

		tmp_data[0]=Points_t_0[i][2];
		tmp_data[1]=Points_t_1[i][2];
		tmp_data[2]=Points_t_2[i][2];
		x.Target[2]=PolynomialPredictor(3,tmp_data);

		x.sqrt_weight=1.0;
		x.VertexIndex=i;
		x.VertexIndex_Target=-1;

		TargetList_3D->append(x);

		accumulation+=selective_step;
		if (accumulation>1.0) {
			int int_accu=(int)accumulation;
			i+=int_accu;
			accumulation-=int_accu;
		}
	}

}

double Articulated_ICP::PredictFrameMinimization(int frame_index) {
	int    iteration_no=0;
	int    tot_iterations=iteration_l0;
	bool   IK_result=true;
	double LastComputedResidual=DBL_MAX;
	IK_Solver->added_sqrt_weight=1.0;
	IK_Solver->last_weighted_features=0;
	

	for(int i=0;((i<iteration_l0)&&(IK_result));i++) {
		iteration_no++;
		if (show_minimal_debug_information) {
			cout<<"\n-> Iteration "<<iteration_no<<"\n";
			cout<<"------------------\n";
		}

		IK_result=IK_Solver->Perspective_Solve_LM(frame_index,TargetList_1D,TargetList_2D,TargetList_3D,internal_iteration,proximity_factor,Stop_Error_Th);
		LastComputedResidual=IK_Solver->LastComputedResidual;
	}

	if (DebugImages&PEDEBUG_FRAMES) {
		DebugIndex++;
		char Text[100];
		sprintf(Text,"%s\\motion\\Prediction_%04i.motion",OutDebugDir,frame_index);
		OFileBuffer Out(Text);
		Bone::SaveSkullMotion(&Out,mesh->BoneRoot);
		Out.Close();
	}

	return LastComputedResidual;
}

double Articulated_ICP::PredictFrame(int frame_index,int base_frame_index) {
	// paper dei tipi della Danimarca
	//    predictor on the pose of the vertex
	//

	if (base_frame_index<=4) {
		mesh->BoneRoot->Copy(base_frame_index,frame_index);
		return DBL_MAX;
	}
	

	GetOnlyPredictionCorrespondences(mesh,base_frame_index);

	mesh->BoneRoot->Copy(base_frame_index,frame_index);
	PrepareMesh(frame_index);

	return PredictFrameMinimization(frame_index);

}






































/*
void Articulated_ICP::Compute_Correspondences_Frame(int frame_index,int starting_frame_index,int n_iter_find_outliers,int outliers_thresh,double texture_weight,Array<TargetList_2D_Item_type_Frame_Info> *New_TargetList) {

	CopyPoseFromFrame(frame_index,starting_frame_index);				        // Crea il modello al frame frame_index copiandolo da starting_frame_index
	ReadFrame(frame_index);												// Legge i file al tempo    frame_index

	// Get Texture Information
	GetOpticalFlowCorrespondences(frame_index,starting_frame_index);
	int num_f_weighted=Feat_TargetList->numElements()-1;

	mesh->SetConstraints(Constraint_level0);
	GetSilhouetteCorrespondences(true,0);
	
	for(int h=0;h<TargetList_2D->numElements();h++) {
		TargetList_2D_Item_type_Frame_Info tmp;
		
		tmp.Source=((*TargetList_2D)[h]).Source;
		tmp.Target=((*TargetList_2D)[h]).Target;
		tmp.VertexIndex=((*TargetList_2D)[h]).VertexIndex;
		tmp.View=((*TargetList_2D)[h]).View;
		tmp.frame=frame_index;
		if (i<=num_f_weighted) tmp.Weighted=true;
		else tmp.Weighted=false;

		New_TargetList->append(tmp);
	}

}

void Articulated_ICP::Compute_W(int start,int end,int outliers_thresh,double texture_weight) {
	Array<TargetList_2D_Item_type_Frame_Info> *All_TargetList=new Array<TargetList_2D_Item_type_Frame_Info>();

	for(int i=start;i<=end;i++) {
		Compute_Correspondences_Frame(i,i,0,outliers_thresh,texture_weight,All_TargetList);
	}

	IK_Solver->added_sqrt_weight=sqrt(texture_weight);

	IK_Solver->Perspective_Solve_ALPHA_WEIGHTS(frame_index,TargetList_2D,internal_iteration,proximity_factor);



	delete All_TargetList;
}
*/






















