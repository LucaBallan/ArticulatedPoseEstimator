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





class CameraParameters {
public:	
	int w,h;
	Matrix Int;
	Matrix real_Ext;
	Matrix OGL_Ext;

	CameraParameters();
	CameraParameters(int w,int h,Matrix *Int,Matrix *real_Ext,Matrix *OGL_Ext=NULL);
	~CameraParameters();
};




class DepthMapRenderer {
	UINT Shader_DepthFloat;
	UINT Shader_DepthFloatOGL;
	UINT Shader_DepthFloatCameraSpace;
	AdvancedRenderWindow *w;
public:
	DepthMapRenderer(AdvancedRenderWindow *w);
	~DepthMapRenderer();

	bool set_empty_pixel_to_infinity;


	//
	// Render the depth in the "image" space (Intrinsic and Extrinsic are both used)
	// the ogl version makes the depth compatible with the shader function MapVertex and Z_Check
	//
	void RenderDepthMap   (FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth=NULL);
	void RenderDepthMapOGL(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth=NULL);
	void RenderDepthMap   (FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth=NULL);
	void RenderDepthMapOGL(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth=NULL);
						//
						// fb.Texture[0]  = depth						(fb->w,fb->h) (in OGL coords if RenderDepthMapOGL otherwise in real coords)
						// depth		  = depth						(fb->w,fb->h) (if depth != NULL)
						// width,height   = camera width and height     (the one related to the the Int matrix, can be different from the one of the fb)
						// Int			  = camera intrinsic
						// OGL_Ext	      = camera OpenGl extrinsic
						// real_Ext		  = Camera extrinsic
						// 
						// NB:  1) in the OGL version, clip_min, clip_max of w should be setted (use DefineNearFarPlane)
						//

	//
	// Render the depth in the camera space (TODO still do not understand the difference except for RT)
	//
	void RenderDepthMapCameraSpace(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,GLdouble *RT_matrix=NULL,Bitmap<ColorFloat> *depth=NULL);
						// RT_matrix	   = R,T applied to the pixels (can be NULL)

	FrameBuffer_struct BuildDepthFrameBuffer(int width,int height);
};





class ModelRenderer {
	UINT Shader_NormalRenderer,Shader_360,Shader_edge_renderer;
	AdvancedRenderWindow *w;
public:
	ModelRenderer(AdvancedRenderWindow *w);
	~ModelRenderer();


	void RenderTexRGBA(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorRGBA> *image=NULL);
	void RenderTexRGBA(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorRGBA> *image=NULL);
	void RenderTexRGB (FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorRGB> *image=NULL);
						//
						// fb.Texture[0]  = image						(fb->w,fb->h)
						// depth		  = image						(fb->w,fb->h)
						// width,height   = camera width and height     (the one related to the the Int matrix, can be different from the one of the fb)
						// Int			  = camera intrinsic
						// OGL_Ext	      = camera OpenGl extrinsic
						// 
						// NB:  1) clip_min, clip_max of w should be setted for the correct rendering
						//
	void RenderBox(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Vector<3> *center,Matrix *axes,Vector<3> *sizes,Bitmap<GreyLevel> *image=NULL);
				// center	=	center of the Box
				// axes		=	the coloums are the axes of the box (PstoRT)
				// sizes	=	indicate the lenghts of the box edges

	void RenderNormals  (FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image=NULL);
	void RenderWireFrame(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,double EdgeCreaseAngle=-1.0,Bitmap<ColorRGB> *image=NULL);



	// Mixed Opengl/Software rendering
	//		Render object footprint without taking into account for problems like
	//			- far/near plane clipping          -> very usefull   (do not need DefineNearFarPlane)
	//			- depth accuracy and face culling
	//			- already fliped vertically        -> not like all the other opengl functions
	//
	void RenderFootPrint(FrameBuffer_struct *fb,int width,int height,Matrix *real_P,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image=NULL);


	// Full Opengl approax 
	//		NB:  1) clip_min, clip_max of w should be setted to the same value used for 
	//			     the depthmap in input
	void RenderFootPrint(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image=NULL);


	FrameBuffer_struct BuildRenderFrameBufferTex(int width,int height,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);
	FrameBuffer_struct BuildRenderFrameBufferFootPrint(int width,int height,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);


	void RenderModel360(FrameBuffer_struct *fb,int width,int height,float near_plane,float far_plane,HyperMesh<3> *mesh,Bitmap<ColorRGB> *image=NULL);

	void RenderEdges(FrameBuffer_struct *fb,int width,int height,float threshold,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,Vector<3> prj_center,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image=NULL);
};



class L0NormRenderer {
	UINT Shader_L0Norm;
	AdvancedRenderWindow *w;
	FrameBuffer_struct fb;
	Bitmap<GreyLevel> *out;

	int div_x,div_y;
	int width,height;
	int log_factor;
	double offset_x,offset_y;
public:
	L0NormRenderer(AdvancedRenderWindow *w,int width,int height);
	~L0NormRenderer();

	unsigned int L0Norm(UINT tex1,UINT tex2);																// tex1, tex2 can be of any type BYTE, FLOAT etc..
};


class L1NormRenderer {
	UINT Shader_L1Norm,Shader_L1Norm2,Shader_L1Norm3;
	AdvancedRenderWindow *w;
	FrameBuffer_struct fb;
	Bitmap<ColorFloat> *out;

	int div_x,div_y;
	int width,height;
	double offset_x,offset_y;
public:
	L1NormRenderer(AdvancedRenderWindow *w,int width,int height,int max_xy_division=16);					// increasing max_xy_division reduce precision
	~L1NormRenderer();

	
	float L1Norm(UINT tex1);																				// tex1, tex2 can be of any type BYTE, FLOAT, single channel
	float L1Norm(UINT tex1,UINT tex2);			// 1 channel
	float L1Norm3C(UINT tex1,UINT tex2);		// 3 channel
};


class RobustL1NormRenderer {
	UINT                  Shader_RL1Norm;
	UINT                  Shader_RL1Norm3C;
	UINT                  Shader_RDiff3C;
	UINT                  Shader_RFlow3C;
	FrameBuffer_struct    fb1Pass;
	FrameBuffer_struct    fb2Pass;

	L1NormRenderer        *L1Norm;
	AdvancedRenderWindow  *w;
	Bitmap<ColorFloat>    *out;


	int     div_x,div_y;
	int     width,height;
	int     width_patch,height_patch;
	int     max_xy_division;
	float  *offset_x_2;
	float  *offset_y_2;
	float   max_flow;
	int     old_c;
	int     old_width_2,old_height_2;

	void Init1Pass();
	void Init2Pass();
	void Init2Flow();
	void InitFilter(UINT sh,int max_offset,int width_2,int height_2);

public:
	struct ROI {
		int x_1,y_1;
		int x_2,y_2;
		int width_img_1,height_img_1;
		int width_img_2,height_img_2;
	};

	RobustL1NormRenderer(AdvancedRenderWindow *w,int width,int height,int max_xy_division=16);				// increasing max_xy_division reduce precision
	~RobustL1NormRenderer();																				// width, height refers to the size of the patch to compare with

	//
	// Note: both of these measures are not simmetric
	//
	// To symmetrize them use:
	//			m_(t1,t2) = m(t1,t2) + m(t2,t1)


	//
	// Compute
	//    diff(p) = min_{q \in O} |tex1(p)-tex2(q)|
	//    out     = sum(diff(p))
	//
	//    out of 3C is 3 times higher than the cost of 1C
	//
	float RobustL1Norm1C(UINT tex1,UINT tex2,ROI *roi,int max_offset);										// 1 channel,  any type BYTE, FLOAT etc.. 
	float RobustL1Norm3C(UINT tex1,UINT tex2,ROI *roi,int max_offset);										// 3 channels, any type BYTE, FLOAT etc.. 
	float RobustL1Norm3C_2Pass(UINT tex1,UINT tex2,ROI *roi,int max_offset);								// 3 channels, any type BYTE, FLOAT etc.. 
																											//     2 pass -> Check before witch is faster between 1 pass or 2 pass
																										    //               depends on the max_offset and size. 
																											//               at the first call they are both slow due to initialization


	//
	// Compute
	//    diff(p)  =     min_{q \in O} |tex1(p)-tex2(q)|
	//    match(p) = arg_min_{q \in O} |tex1(p)-tex2(q)|
	//    flow(p)  = |match(p)-p|
	//    score(p) = (1.0 - flow(p)/max_flow) * clamp( 1.0 - diff(p)/sigma_diff ,0.0,1.0)
	//    out      = sum(1.0 - score(p))
	//
	//    multiply flow and diff together
	//
	float RobustFlow3C_2Pass(UINT tex1,UINT tex2,ROI *roi,int max_offset,float sigma_diff=(10.0/255.0));
};



class TextureRenderer {
	UINT Shader_TextureRenderer;
	AdvancedRenderWindow *w;
public:
	TextureRenderer(AdvancedRenderWindow *w);
	~TextureRenderer();


	void RenderTexture(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,UINT Image,UINT OGL_Depth_Map,UINT MaskImage,float Z_Discrepancy=0.0002,Bitmap<ColorRGBA> *texture=NULL);
	void RenderTexture(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,UINT Image,UINT OGL_Depth_Map,UINT MaskImage,float Z_Discrepancy=0.0002,Bitmap<ColorRGBA> *texture=NULL);
						//
						// fb.Texture[0]  = texture					(RGBA) (Alpha = 0.0 dato invalido, = x dato valido)
						// texture		  = texture					(if texture != NULL)
						// OGL_Depth_Map  = Depth_Map obtained using RenderDepthMapOGL
						// MaskImage	  = can be NULL				ALPHA CHANNEL IS THE VALUE (act as alpha channel, 0.0 means dato invalido)
						//
						// NB:  1) clip_min, clip_max of w should be setted to the same value used for 
						//		   the depthmap in input (use DefineNearFarPlane)
						//		2) mesh->NormalizeUV and mesh->TextureUVW must be prepared
						//      3) Z_Discrepancy has to setted on the base of the radius of the object to texture
						//      4) For the mask -> use RenderTexture again with as input a normal rendered image of the scene
						//
	FrameBuffer_struct BuildTextureFrameBuffer(int width,int height);
};



class CameraProjectionRenderer {
	UINT Shader_CameraProject,Shader_CameraProject_alpha;
	AdvancedRenderWindow *w;

public:
	CameraProjectionRenderer(AdvancedRenderWindow *w);
	~CameraProjectionRenderer();
	
	
	void ProjectCamera(FrameBuffer_struct *fb,
											 int width_src,int height_src,
											 Matrix *Int_src,Matrix *OGL_Ext_src,
											 UINT image_src,UINT ogl_depth_src,UINT mask_src,
											 int width_trg,int height_trg,
											 Matrix *Int_trg,Matrix *OGL_Ext_trg,
											 double Z_Discrepancy,
											 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *projection=NULL);
						//
						// Img vista da trg riproiettando da src 
						//
						// fb.Texture[0]  = texture					(RGBA) (Alpha = 0.0 dato invalido, = x dato valido x dipendente solo da vignetting)
						// ogl_depth_src  = Depth_Map obtained using RenderDepthMapOGL
						// mask_src		  = can be NULL				ALPHA CHANNEL IS THE VALUE (act as alpha channel, 0.0 means dato invalido)
						//
						// NB:  1) clip_min, clip_max of w should be setted to the same value used for 
						//		   the depthmap in input

	
	
	// TODO> LA FUNZIONE SEGUENTE NON e' stata TESTATA!
	void ProjectCamera_Alpha(FrameBuffer_struct *fb,
											 int width_src,int height_src,
											 Matrix *Int_src,Matrix *OGL_Ext_src,
											 UINT image_src,UINT ogl_depth_src,UINT mask_src,
											 int width_trg,int height_trg,
											 Matrix *Int_trg,Matrix *OGL_Ext_trg,
											 double Z_Discrepancy,
											 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *projection=NULL);
						//
						// Img vista da trg riproiettando da src 
						//
						// fb.Texture[0]  = texture					(RGBA) (Alpha = 0.0 dato invalido, = x dato valido x dipendente dalla normale alla superficie, normals must be computed)

	FrameBuffer_struct BuildProjectCameraFrameBuffer(int width_trg,int height_trg,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);

	
	// TODO LA FUNZIONE SEGUENTE NON e' IMPLEMENTATA
	void VBR(FrameBuffer_struct *vdr_fb,
			 int width_src1,int height_src1,
			 Matrix *Int_src1,Matrix *OGL_Ext_src1,
			 UINT image_src1,UINT ogl_depth_src1,UINT mask_src1,
			 int width_src2,int height_src2,
			 Matrix *Int_src2,Matrix *OGL_Ext_src2,
			 UINT image_src2,UINT ogl_depth_src2,UINT mask_src2,
			 int width_trg,int height_trg,
			 Matrix *Int_trg,Matrix *OGL_Ext_trg,
			 double Z_Discrepancy,
			 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *vdr_image=NULL);

};




class MeanStdRenderer {
	UINT Shader_MeanStd,Shader_MeanStd_From_MeanPow;
	UINT MeanStd[4];
		int Current_MeanStd_indexes[2];
		int tmp_MeanStd_indexes[2];
	FrameBuffer_struct fb;
	AdvancedRenderWindow *w;
public:
	MeanStdRenderer(AdvancedRenderWindow *w,int width,int height,int default_buffer_options=0);
	~MeanStdRenderer();


	void Update(UINT image_to_consider);
	void ExtractMeanStd(UINT &Mean,UINT &Std,int reset=0,Bitmap<ColorRGBA> *RGBA_Mean=NULL,Bitmap<ColorFloat> **C4_Mean=NULL,Bitmap<ColorFloat> **C4_Std=NULL);
						//
						// image_to_consider  = image			(RGBA)   (Alpha = 0.0 dato invalido, else valido con peso 1.0)
						// Mean			      = mean image				 (Alpha = valid count)
						// Std			      = std image 
						// 
						// *) Mean,Std can be used only before an update call
						//
						// reset  = 0      -> Update can be called again for computing the mean&std considering all the history
						// reset  = 1      -> Update can be called again for computing the mean&std considering as initial map the current mean&std
						// reset  = 2      -> Update can be called again for computing the mean&std considering from a complete resetted state
};


class MeanStdSoftRenderer {
	Bitmap<ColorFloat> **Mean,**Std;
	AdvancedRenderWindow *w;
public:
	MeanStdSoftRenderer(AdvancedRenderWindow *w,int width,int height);
	~MeanStdSoftRenderer();


	void Clear();
	void Update(Bitmap<ColorRGBA> *image_to_consider);
	void ExtractMeanStd(Bitmap<ColorRGBA> *MeanOut,Bitmap<ColorRGBA> *StdOut);
						//
						// image_to_consider  = image			(RGBA)   (Alpha = 0.0 dato invalido, else valido con peso 1.0)
						// Mean			      = mean image				 (Alpha = valid count)
						// Std			      = std image 
						// 
	void Update(Bitmap<ColorFloat> *image_to_consider,ColorFloat no_data_value);
	void ExtractMeanStd(Bitmap<ColorFloat> *MeanOut,Bitmap<ColorFloat> *StdOut,ColorFloat no_data_value);
};

class MedianSoftRenderer {
	int width,height;
	int n_samples;
	Array<BYTE> **Images;
	AdvancedRenderWindow *w;

public:
	MedianSoftRenderer(AdvancedRenderWindow *w,int width,int height,int predicted_n_samples,int initial_n_samples);
	~MedianSoftRenderer();


	void Update(Bitmap<ColorRGBA> *image_to_consider);
	void ExtractMedian(Bitmap<ColorRGBA> *Median);
	void ExtractMedian(Bitmap<ColorRGBA> *Median,Bitmap<ColorRGBA> *std);
						//
						// image_to_consider  = image			(RGBA)   (Alpha = 0.0 dato invalido, else valido con peso 1.0)
						// Median		      = median image	(RGBA)	 (Alpha = valid count)
						// 
};



class MeanStdSampleMaxSoftRenderer {
	Bitmap<ColorFloat> **Mean,**Std;
	int max_number_of_samples;
public:
	MeanStdSampleMaxSoftRenderer(int width,int height,int max_number_of_samples);
	~MeanStdSampleMaxSoftRenderer();


	bool Update(float sample_index,Bitmap<ColorRGBA> *sample,int &num_pixel_not_filled,double &not_completed_percentage);
						// true if it is all filled (even with not all the max samples!!)
						// not_completed_percentage = percentuale non completata!!
	void ExtractMeanStd(Bitmap<ColorRGBA> *MeanOut,Bitmap<ColorRGBA> *StdOut);
						//
						// image_to_consider  = image			(RGBA)   (Alpha = 0.0 dato invalido, else valido con peso 1.0)
						// Mean			      = mean image				 (Alpha = valid count)
						// Std			      = std image				 (Alpha = minimun sample_index for each pixel solo se alpha != 0.0)
						//
	void Clear();
};




class MedianStdSampleMaxSoftRenderer {
	int width,height;
	int n_samples;
	int max_number_of_samples;
	Array<BYTE> **Images;

public:
	MedianStdSampleMaxSoftRenderer(int width,int height,int max_number_of_samples);
	~MedianStdSampleMaxSoftRenderer();


	bool Update(float sample_index,Bitmap<ColorRGBA> *image_to_consider,int &num_pixel_not_filled,double &not_completed_percentage);
						// true if it is all filled (even with not all the max samples!!)
						// not_completed_percentage = percentuale non completata!!
	void ExtractMeanStd(Bitmap<ColorRGBA> *Median,Bitmap<ColorRGBA> *std);
						//
						// image_to_consider  = image			(RGBA)   (Alpha = 0.0 dato invalido, else valido con peso 1.0)
						// Mean			      = mean image				 (Alpha = valid count)
						// Std			      = std image				 (Alpha = minimun sample_index for each pixel solo se alpha != 0.0)
						//
	void Clear();
};





class ImageOperationsRenderer {
	UINT Shader_uint8RGBA2floatHCY, Shader_MatchingYH;
	AdvancedRenderWindow *w;
public:
	ImageOperationsRenderer(AdvancedRenderWindow *w);
	~ImageOperationsRenderer();

	void RGBA2HCYA(FrameBuffer_struct *fb, UINT RGBA_Image);
						//
						// fb.Texture[0]  = YHHA_Image
						//
	void YHHAMatching(FrameBuffer_struct *fb,UINT YHHA_Image1,UINT YHHA_Image2,int win_size,int dim_search,double min_var,double max_var,Bitmap<GreyLevel> *matches=NULL);
						//
						// fb.Texture[0]  = matches
						// matches		  = matches
						//
	void InvertTexture(FrameBuffer_struct *fb,UINT Texture);
						//
						// fb.Texture[0]  = inverted texture
						// 
						// NB: fb.w,fb.h must be the same as the texture
						//

	FrameBuffer_struct BuildYHHAConversionFrameBuffer(int width,int height,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);
};






class AnaglyphsRenderer {
	UINT Shader_Stereo;
	AdvancedRenderWindow *w;
	UINT map_left,map_right;
	int c_width,c_height;

public:
	AnaglyphsRenderer(AdvancedRenderWindow *w,int width,int height);
	~AnaglyphsRenderer();

	void InitAndGetLeft(int width,int height);
	void GetRightAndProcess();
};




#define IPR_FILTER_MAX_DIMENSION 169
	

class ImageProcessingRenderer {
	UINT Shader_Filter,Shader_NormFxFyFilter,Shader_RGB2YCBCR,Shader_AngleFilter,Shader_NormFilter,Shader_MedianFilter,Shader_ExpandFilter,Shader_uint8RGBA2uint8YHHA,Shader_uint8RGBA2doubleLAB,Shader_doubleLAB2uint8RGBA;
	AdvancedRenderWindow *w;
	void FilterStart(FrameBuffer_struct *fb,UINT shader,float *filter,int n_elements,int output_buffer_index);
	void FilterSetOffset(FrameBuffer_struct *fb,UINT shader,int r,int c,int n_elements);
	void FilterEnd(FrameBuffer_struct *fb,UINT input_image,Bitmap<ColorFloat> *output);
public:
	ImageProcessingRenderer(AdvancedRenderWindow *w);
	~ImageProcessingRenderer();

	// 2D Filter
	void FilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,int output_buffer_index=0,Bitmap<ColorFloat> *filtered=NULL);
	// 2D Symmetric Filter
	void FilterImage(FrameBuffer_struct *fb,UINT input_image,float *Filter,int filter_len,int output_buffer_index=0,Bitmap<ColorFloat> *filtered=NULL);
	

	// Custom Shader Filter
	void FilterDefine (FrameBuffer_struct *fb,UINT Shader,Matrix *Filter,int output_buffer_index=0);
	void FilterProcess(FrameBuffer_struct *fb,UINT input,Bitmap<ColorFloat> *output=NULL);


	// NormFxFyFilter Filter (fb must have at least 3 textures)
	void NormFxFyFilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,
							  int output_buffer_index_fx=0,
							  int output_buffer_index_fy=1,
							  int output_buffer_index_norm=2,
							  Bitmap<ColorFloat> *fx=NULL,
							  Bitmap<ColorFloat> *fy=NULL,
							  Bitmap<ColorFloat> *norm=NULL);


	// Color Coversion
	void RGBtoYCbCr(FrameBuffer_struct *fb,UINT input_image,
					  int output_buffer_index_Y=0,
					  int output_buffer_index_Cb=1,
					  int output_buffer_index_Cr=2,
					  Bitmap<ColorFloat> *Y=NULL,
					  Bitmap<ColorFloat> *Cb=NULL,
					  Bitmap<ColorFloat> *Cr=NULL);

	
	void RGBA2YHHA(FrameBuffer_struct *fb_3byte,UINT input_image,Bitmap<ColorRGB> *out=NULL);
	
	
	// Global Color Transfer
	void ColorTransfer_Step1(FrameBuffer_struct *fb_3float,UINT input_image,int width,int height,Vector<3> *m,Vector<3> *s);
	void ColorTransfer_Step2(FrameBuffer_struct *fb,UINT fb1_texture,Vector<3> m_s,Vector<3> d_s,Vector<3> m_t,Vector<3> d_t,Bitmap<ColorRGB> *out=NULL);


	void ExpandImage(FrameBuffer_struct *fb,UINT input_image,
					   int output_buffer_index_image,
					   Bitmap<ColorRGBA>  *image);

	// TODO***
	void NormFilterImage (FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,Bitmap<ColorFloat> *norm);
	void AngleFilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,Bitmap<ColorFloat> *angles);
	void MedianFilterImage(FrameBuffer_struct *fb,UINT input_image,Bitmap<ColorFloat> *output);
			//			
			// Filter the image with Filter and Filter' and compute the angle between the two resulting images
			//
			
	FrameBuffer_struct BuildFloatFrameBuffer(int width,int height);
	FrameBuffer_struct Build3byteFrameBuffer(int width,int height,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);
	FrameBuffer_struct Build3floatFrameBuffer(int width,int height,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);
};




class DepthEdgesDetectorRenderer {
	UINT Shader_DepthEdgeFilter;
	AdvancedRenderWindow *w;
public:
	DepthEdgesDetectorRenderer(AdvancedRenderWindow *w);
	~DepthEdgesDetectorRenderer();

	void FilterDepth(FrameBuffer_struct *fb,UINT depth_map,Bitmap<ColorFloat> *output=NULL);

	FrameBuffer_struct BuildFloatFrameBuffer(int width,int height);
};




class PatchMatch : public PSO_Solver {
	AdvancedRenderWindow  *w;
	FrameBuffer_struct     fb;
	L1NormRenderer        *X;
	HyperMesh<3>          *M;
	HyperMesh<3>          *Mp;
	UINT                   texture_1,texture_2;
	double                 ratio_w,ratio_h;
	int                    width,height;
	bool                   use_morph;
	char                  *outputname;
public:


	// width_e, height_e = is the effective size of the images
	// width,   height   = is the portion of the images to compare
	PatchMatch(int n_particles,bool morph,AdvancedRenderWindow *w,UINT texture_1,UINT texture_2,int width,int height,int width_e,int height_e,HyperMesh<3> *M);
	~PatchMatch();

	double Evaluate(double *particle,bool generate_output);

	int   getDOF();
	bool  save_particle_images;
	void  setOutputName(char *outputname);
};
