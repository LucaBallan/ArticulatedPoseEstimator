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




CameraParameters::CameraParameters() : Int(3,3), real_Ext(3,4), OGL_Ext(3,4) {
	Int.SetIdentity();
	real_Ext.SetIdentity();
	OGL_Ext.SetIdentity();
}
CameraParameters::CameraParameters(int w,int h,Matrix *Int,Matrix *real_Ext,Matrix *OGL_Ext) : Int(3,3), real_Ext(3,4), OGL_Ext(3,4) {
	this->w=w;
	this->h=h;
	this->Int.Set(Int);
	this->real_Ext.Set(real_Ext);
	
	if (OGL_Ext) {
		this->OGL_Ext.Set(OGL_Ext);
	} else {
		this->OGL_Ext.Set(real_Ext);
		CorrectExtrinsic_For_OpenGL3D_Visualization(&(this->OGL_Ext));
	}
}
CameraParameters::~CameraParameters() {
}















char StdVShader[] = 
"varying vec4 VertexPosition;"
"void main( void ) {"
"	VertexPosition = gl_Vertex;"
"	gl_Position = ftransform();"
"}";
char DepthShader[] = 
"varying vec4 VertexPosition;"
"uniform mat4 CameraMatProj;"
"void main(void) {"
"	vec4 P=VertexPosition; "
"	P.w=1.0;"
"	vec4 up=CameraMatProj*P;"
"	gl_FragColor=vec4(up.z,up.z,up.z,up.z);"
"}";
char DepthShaderOGL[] = 
"varying vec4 VertexPosition;"
"uniform mat4 CameraMatM1;"
"uniform mat4 CameraMatP1;"
"float MapVertex(mat4 CameraMatM,mat4 CameraMatP,vec4 VertexP) {"
"	vec4 C4D=CameraMatP*CameraMatM*VertexP;"
"	vec3 C3D=vec3(C4D.x/C4D.w,C4D.y/C4D.w,C4D.z/C4D.w);"
"	C3D.z=(C3D.z+1.0)/2.0;"
"	return C3D.z;"
"}"
"void main(void) {"
"	float z=MapVertex(CameraMatM1,CameraMatP1,VertexPosition);"
"	gl_FragColor=vec4(z,z,z,z);"
"}";
char RTVShader[] = 
"varying vec4 VertexPosition;"
"uniform mat4 RT_matrix;"
"void main( void ) {"
"	VertexPosition = RT_matrix*gl_Vertex;"
"	gl_Position = gl_ModelViewProjectionMatrix*VertexPosition;"
"}";
char CameraSpaceDepthShader[] = 
"varying vec4 VertexPosition;"
"uniform mat4 CameraMatM1;"
"void main(void) {"
"	vec4 cam_space_p=CameraMatM1*VertexPosition;"
"	float z=cam_space_p.z;"
"	gl_FragColor=vec4(z,z,z,z);"
"}";


DepthMapRenderer::DepthMapRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_DepthFloat=w->CreateShaderFromCode(StdVShader,DepthShader,false);
	Shader_DepthFloatOGL=w->CreateShaderFromCode(StdVShader,DepthShaderOGL,false);
	Shader_DepthFloatCameraSpace=w->CreateShaderFromCode(RTVShader,CameraSpaceDepthShader,false);
	set_empty_pixel_to_infinity=false;
}

DepthMapRenderer::~DepthMapRenderer() {
	w->DeleteShader(Shader_DepthFloat);
	w->DeleteShader(Shader_DepthFloatOGL);
	w->DeleteShader(Shader_DepthFloatCameraSpace);
}

void DepthMapRenderer::RenderDepthMap(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth) {
	RenderDepthMap(fb,camera->w,camera->h,&(camera->Int),&(camera->OGL_Ext),&(camera->real_Ext),mesh,depth);
}
void DepthMapRenderer::RenderDepthMapOGL(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth) {
	RenderDepthMapOGL(fb,camera->w,camera->h,&(camera->Int),&(camera->OGL_Ext),mesh,depth);
}
void DepthMapRenderer::RenderDepthMap(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);	
	w->SetReadBuffer(0);
	glDepthMask(1);
	if (set_empty_pixel_to_infinity) {
		SetStandardOGLEnvironment(true,false,false);
		glClearColor(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	} else {
		SetStandardOGLEnvironment(true,false,true);
	}
	glDisable(GL_CULL_FACE);

	//
	Matrix P_(3,4),P(4,4);
	Multiply(Int,real_Ext,&P_);
	EtoP(&P_,&P);
	//
	
		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
		w->SetShader(Shader_DepthFloat);
			w->SetUniformVariable(Shader_DepthFloat,"CameraMatProj",&P);
			
			w->DrawHyperMesh_Flat(mesh);
		w->SetShader(0);
		w->ClearImageCoords();
		if (depth) w->RetrieveFrameBuffer(fb,depth);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}





void DepthMapRenderer::RenderDepthMapCameraSpace(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,GLdouble *RT_matrix,Bitmap<ColorFloat> *depth) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	GLdouble tmp[16];
	if (set_empty_pixel_to_infinity) {
		SetStandardOGLEnvironment(true,false,false);
		glClearColor(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	} else {
		SetStandardOGLEnvironment(true,false,true);
	}
	glDisable(GL_CULL_FACE);

	
		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
		w->SetShader(Shader_DepthFloatCameraSpace);
			PtoGL(real_Ext,tmp);
			w->SetUniformVariable(Shader_DepthFloatCameraSpace,"CameraMatM1",tmp);
			if (!RT_matrix) {
				Matrix P(4,4);
				P.SetIdentity();
				PtoGL(&P,tmp);
				w->SetUniformVariable(Shader_DepthFloatCameraSpace,"RT_matrix",tmp);
			} else {
				w->SetUniformVariable(Shader_DepthFloatCameraSpace,"RT_matrix",RT_matrix);
			}

			w->DrawHyperMesh_Flat(mesh);
		w->SetShader(0);
		w->ClearImageCoords();
		if (depth) w->RetrieveFrameBuffer(fb,depth);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}








































void DepthMapRenderer::RenderDepthMapOGL(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorFloat> *depth) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	if (set_empty_pixel_to_infinity) {
		SetStandardOGLEnvironment(true,false,false);
		glClearColor(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	} else {
		SetStandardOGLEnvironment(true,false,true);
	}
	glDisable(GL_CULL_FACE);

	OpenGL_State X1;
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int,OGL_Ext,width,height,&X1,w->clip_min(),w->clip_max());

		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
		w->SetShader(Shader_DepthFloatOGL);
			w->SetUniformVariable(Shader_DepthFloatOGL,"CameraMatM1",X1.ModelView);
			w->SetUniformVariable(Shader_DepthFloatOGL,"CameraMatP1",X1.Projection);

			w->DrawHyperMesh_Flat(mesh);
		w->SetShader(0);
		w->ClearImageCoords();
		if (depth) w->RetrieveFrameBuffer(fb,depth);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}

FrameBuffer_struct DepthMapRenderer::BuildDepthFrameBuffer(int width,int height) {
	UINT Depth_Map=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&Depth_Map,1,TEXTURE_CH_32,0);

	return fb;
}


























TextureRenderer::TextureRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_TextureRenderer=w->CreateShader(DATA_DIRECTORY"StdShader.vert.c",DATA_DIRECTORY"TextureRenderer.frag.c",false);
}

TextureRenderer::~TextureRenderer() {
	w->DeleteShader(Shader_TextureRenderer);
}

void TextureRenderer::RenderTexture(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,UINT Image,UINT OGL_Depth_Map,UINT MaskImage,float Z_Discrepancy,Bitmap<ColorRGBA> *texture) {
	RenderTexture(fb,camera->w,camera->h,&(camera->Int),&(camera->OGL_Ext),mesh,Image,OGL_Depth_Map,MaskImage,Z_Discrepancy,texture);
}

void TextureRenderer::RenderTexture(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,UINT Image,UINT OGL_Depth_Map,UINT MaskImage,float Z_Discrepancy,Bitmap<ColorRGBA> *texture) {

	OpenGL_State X1;
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int,OGL_Ext,width,height,&X1,w->clip_min(),w->clip_max());


	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(0);									// NB: it doesn't have a depth buffer!!
	SetStandardOGLEnvironment(false,false,true);
	glDisable(GL_CULL_FACE);

	w->SetImageCoords01();
		w->SetShader(Shader_TextureRenderer);
			w->SetUniformVariable(Shader_TextureRenderer,"CameraMatM1",X1.ModelView);
			w->SetUniformVariable(Shader_TextureRenderer,"CameraMatP1",X1.Projection);
			Vector<4> tmp;tmp[0]=X1.ViewPort[0];tmp[1]=X1.ViewPort[1];tmp[2]=X1.ViewPort[2];tmp[3]=X1.ViewPort[3];
			w->SetUniformVariable(Shader_TextureRenderer,"ViewPort1",&tmp);
			w->SetUniformVariable(Shader_TextureRenderer,"Src1",0);
			w->SetUniformVariable(Shader_TextureRenderer,"Depth1",1);
			w->SetUniformVariable(Shader_TextureRenderer,"Mask1",2);
			if (MaskImage==INVALID_TEXTURE) w->SetUniformVariable(Shader_TextureRenderer,"CheckMask",(int)0);
			else w->SetUniformVariable(Shader_TextureRenderer,"CheckMask",(int)1);
			w->SetUniformVariable(Shader_TextureRenderer,"Z_Discrepancy",(double)Z_Discrepancy);

			
			UINT Textures[3];
			Textures[0]=Image;
			Textures[1]=OGL_Depth_Map;
			Textures[2]=MaskImage;
			List<Vector<3>> *TextureUVW[]={&(mesh->Points)};
			List<HyperMesh<3>::FaceType> *Texture_face[]={&(mesh->Faces)};
			int num_used_textures;
			if (MaskImage==INVALID_TEXTURE) num_used_textures=2;
			else num_used_textures=3;
			w->DrawHyperMesh_MultiTexture(mesh->num_f,mesh->TextureUVW,mesh->Texture_Face,NULL,num_used_textures,1,Textures,TextureUVW,Texture_face);
	
		w->SetShader(0);
	w->ClearImageCoords();
	if (texture) w->RetrieveFrameBuffer(fb,texture);
	glDepthMask(1);
	w->ClearFrameBuffer(fb);
}

FrameBuffer_struct TextureRenderer::BuildTextureFrameBuffer(int width,int height) {
	UINT Tex_Map=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&Tex_Map,1,TEXTURE_CH_RGBA,TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);

	return fb;
}



















MeanStdRenderer::MeanStdRenderer(AdvancedRenderWindow *w,int width,int height,int default_buffer_options) {
	this->w=w;
	Shader_MeanStd=w->CreateShader(DATA_DIRECTORY"2DShader.vert.c",DATA_DIRECTORY"MeanStd.frag.c",true);
	Shader_MeanStd_From_MeanPow=w->CreateShader(DATA_DIRECTORY"2DShader.vert.c",DATA_DIRECTORY"MeanPowMeanStd.frag.c",true);
	MeanStd[0]=NULL;MeanStd[1]=NULL;MeanStd[2]=NULL;MeanStd[3]=NULL;

	fb=w->CreateFrameBuffer(width,height,MeanStd,4,TEXTURE_CH_32_32_32_32,default_buffer_options,false);

	Current_MeanStd_indexes[0]=0;
	Current_MeanStd_indexes[1]=1;
	tmp_MeanStd_indexes    [0]=2;
	tmp_MeanStd_indexes    [1]=3;

	// Clear the first two, i.e., Current_MeanStd_indexes [0] e [1]
	w->SetFrameBuffer(&fb);
	w->SetDrawBuffers(2);
	SetStandardOGLEnvironment(false,false,true);
	w->SetDrawBuffer(0);
	w->ClearFrameBuffer(&fb);
}

MeanStdRenderer::~MeanStdRenderer() {
	w->DeleteFrameBuffer(&fb);
	w->DeleteTexture((MeanStd[0]));
	w->DeleteTexture((MeanStd[1]));
	w->DeleteTexture((MeanStd[2]));
	w->DeleteTexture((MeanStd[3]));

	w->DeleteShader(Shader_MeanStd);
	w->DeleteShader(Shader_MeanStd_From_MeanPow);
}

void MeanStdRenderer::Update(UINT image_to_consider) {
	UINT Current_MeanStd[3];
	Current_MeanStd[0]=MeanStd[Current_MeanStd_indexes[0]];
	Current_MeanStd[1]=MeanStd[Current_MeanStd_indexes[1]];
	Current_MeanStd[2]=image_to_consider;

	w->SetFrameBuffer(&fb);
	w->SetDrawBuffers(2,tmp_MeanStd_indexes);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
		w->SetShader(Shader_MeanStd);
			w->SetUniformVariable(Shader_MeanStd,"CurrentSum",0);
			w->SetUniformVariable(Shader_MeanStd,"CurrentSumSquare",1);
			w->SetUniformVariable(Shader_MeanStd,"CurrentImage",2);

			w->Run2DShader(3,Current_MeanStd);
		w->SetShader(0);

	glDepthMask(1);
	w->SetDrawBuffer(0);
	w->ClearFrameBuffer(&fb);

	swap(tmp_MeanStd_indexes[0],Current_MeanStd_indexes[0]);
	swap(tmp_MeanStd_indexes[1],Current_MeanStd_indexes[1]);
}

void MeanStdRenderer::ExtractMeanStd(UINT &Mean,UINT &Std,int reset,Bitmap<ColorRGBA> *RGBA_Mean,Bitmap<ColorFloat> **C4_Mean,Bitmap<ColorFloat> **C4_Std) {
	UINT Current_MeanStd[2];
	Mean=MeanStd[tmp_MeanStd_indexes[0]];
	Std=MeanStd[tmp_MeanStd_indexes[1]];
	Current_MeanStd[0]=MeanStd[Current_MeanStd_indexes[0]];
	Current_MeanStd[1]=MeanStd[Current_MeanStd_indexes[1]];


	w->SetFrameBuffer(&fb);
	w->SetDrawBuffers(2,tmp_MeanStd_indexes);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
		w->SetShader(Shader_MeanStd_From_MeanPow);
			w->SetUniformVariable(Shader_MeanStd_From_MeanPow,"CurrentSum",0);
			w->SetUniformVariable(Shader_MeanStd_From_MeanPow,"CurrentSumSquare",1);

			w->Run2DShader(2,Current_MeanStd);
		w->SetShader(0);

	
	if (RGBA_Mean) {
		w->SetReadBuffer(tmp_MeanStd_indexes[0]);
		w->RetrieveFrameBuffer(&fb,RGBA_Mean);
	}
	if (C4_Mean) {
		w->SetReadBuffer(tmp_MeanStd_indexes[0]);
		w->RetrieveFrameBuffer(&fb,C4_Mean[0],GL_RED);
		w->RetrieveFrameBuffer(&fb,C4_Mean[1],GL_GREEN);
		w->RetrieveFrameBuffer(&fb,C4_Mean[2],GL_BLUE);
		w->RetrieveFrameBuffer(&fb,C4_Mean[3],GL_ALPHA);
	}
	if (C4_Std) {
		w->SetReadBuffer(tmp_MeanStd_indexes[1]);
		w->RetrieveFrameBuffer(&fb,C4_Std[0],GL_RED);
		w->RetrieveFrameBuffer(&fb,C4_Std[1],GL_GREEN);
		w->RetrieveFrameBuffer(&fb,C4_Std[2],GL_BLUE);
		w->RetrieveFrameBuffer(&fb,C4_Std[3],GL_ALPHA);
	}
	glDepthMask(1);
	w->SetDrawBuffer(0);
	w->ClearFrameBuffer(&fb);
	
	// result			= tmp
	// current_estimate = current

	if (reset==1) {
		// current_estimate = result
		swap(tmp_MeanStd_indexes[0],Current_MeanStd_indexes[0]);
		swap(tmp_MeanStd_indexes[1],Current_MeanStd_indexes[1]);
	}
	if (reset==2) {
		// current_estimate = RESET
		w->SetFrameBuffer(&fb);
		w->SetDrawBuffers(2,Current_MeanStd_indexes);
		SetStandardOGLEnvironment(false,false,true);
		w->SetDrawBuffer(0);
		w->ClearFrameBuffer(&fb);
	}
}

















CameraProjectionRenderer::CameraProjectionRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_CameraProject=w->CreateShader(DATA_DIRECTORY"StdShader.vert.c",DATA_DIRECTORY"Shader_CameraProject.frag.c");
	Shader_CameraProject_alpha=w->CreateShader(DATA_DIRECTORY"StdShader.vert.c",DATA_DIRECTORY"Shader_CameraProject_alpha.frag.c");
}

CameraProjectionRenderer::~CameraProjectionRenderer() {
	w->DeleteShader(Shader_CameraProject);
	w->DeleteShader(Shader_CameraProject_alpha);
}

void CameraProjectionRenderer::ProjectCamera(FrameBuffer_struct *fb,
											 int width_src,int height_src,
											 Matrix *Int_src,Matrix *OGL_Ext_src,
											 UINT image_src,UINT ogl_depth_src,UINT mask_src,
											 int width_trg,int height_trg,
											 Matrix *Int_trg,Matrix *OGL_Ext_trg,
											 double Z_Discrepancy,
											 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *projection) {

	// Get information
	Vector<4> tmp;
	OpenGL_State X1;
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int_src,OGL_Ext_src,width_src,height_src,&X1,w->clip_min(),w->clip_max());
	


	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	SetStandardOGLEnvironment(true,false,true);
	

	// Set Target Camera
	w->SetIE(Int_trg,width_trg,height_trg,OGL_Ext_trg);
	glViewport(0,0,width_trg,height_trg);


	w->SetShader(Shader_CameraProject);
	w->SetUniformVariable(Shader_CameraProject,"CameraMatM1",X1.ModelView);
	w->SetUniformVariable(Shader_CameraProject,"CameraMatP1",X1.Projection);
	tmp[0]=X1.ViewPort[0];tmp[1]=X1.ViewPort[1];tmp[2]=X1.ViewPort[2];tmp[3]=X1.ViewPort[3];
	w->SetUniformVariable(Shader_CameraProject,"ViewPort1",&tmp);
	w->SetUniformVariable(Shader_CameraProject,"Src1",0);
	w->SetUniformVariable(Shader_CameraProject,"Depth1",1);
	w->SetUniformVariable(Shader_CameraProject,"Mask1",2);
	if (mask_src==INVALID_TEXTURE) w->SetUniformVariable(Shader_CameraProject,"CheckMask",(int)0);
	else w->SetUniformVariable(Shader_CameraProject,"CheckMask",(int)1);
	w->SetUniformVariable(Shader_CameraProject,"Z_Discrepancy",Z_Discrepancy);

	UINT Textures[3];
	Textures[0]=image_src;
	Textures[1]=ogl_depth_src;
	Textures[2]=mask_src;
	int num_used_textures;
	if (mask_src==INVALID_TEXTURE) num_used_textures=2;
	else num_used_textures=3;
	w->DrawHyperMesh_MultiTexture(mesh,num_used_textures,0,Textures,NULL,NULL);
	
	w->SetShader(0);
	w->ClearImageCoords();
	if (projection) w->RetrieveFrameBuffer(fb,projection);

	w->ClearFrameBuffer(fb);
}

void CameraProjectionRenderer::ProjectCamera_Alpha(FrameBuffer_struct *fb,
											 int width_src,int height_src,
											 Matrix *Int_src,Matrix *OGL_Ext_src,
											 UINT image_src,UINT ogl_depth_src,UINT mask_src,
											 int width_trg,int height_trg,
											 Matrix *Int_trg,Matrix *OGL_Ext_trg,
											 double Z_Discrepancy,
											 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *projection) {

	// Get information
	Vector<4> tmp;
	OpenGL_State X1;
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int_src,OGL_Ext_src,width_src,height_src,&X1,w->clip_min(),w->clip_max());
	// Get Projection Center
		Vector<3> proj_c;
		Matrix E(3,4);
		ExttoE(OGL_Ext_src,&E);
		E.GetColumn(3,proj_c.GetList());


	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	SetStandardOGLEnvironment(true,false,true);
	

	// Set Target Camera
	w->SetIE(Int_trg,width_trg,height_trg,OGL_Ext_trg);
	glViewport(0,0,width_trg,height_trg);


	w->SetShader(Shader_CameraProject);
	w->SetUniformVariable(Shader_CameraProject,"CameraMatM1",X1.ModelView);
	w->SetUniformVariable(Shader_CameraProject,"CameraMatP1",X1.Projection);
	w->SetUniformVariable(Shader_CameraProject,"CameraOrigin",&proj_c);
	tmp[0]=X1.ViewPort[0];tmp[1]=X1.ViewPort[1];tmp[2]=X1.ViewPort[2];tmp[3]=X1.ViewPort[3];
	w->SetUniformVariable(Shader_CameraProject,"ViewPort1",&tmp);
	w->SetUniformVariable(Shader_CameraProject,"Src1",0);
	w->SetUniformVariable(Shader_CameraProject,"Depth1",1);
	w->SetUniformVariable(Shader_CameraProject,"Mask1",2);
	if (mask_src==INVALID_TEXTURE) w->SetUniformVariable(Shader_CameraProject,"CheckMask",(int)0);
	else w->SetUniformVariable(Shader_CameraProject,"CheckMask",(int)1);
	w->SetUniformVariable(Shader_CameraProject,"Z_Discrepancy",Z_Discrepancy);

	UINT Textures[3];
	Textures[0]=image_src;
	Textures[1]=ogl_depth_src;
	Textures[2]=mask_src;
	int num_used_textures;
	if (mask_src==INVALID_TEXTURE) num_used_textures=2;
	else num_used_textures=3;
	w->DrawHyperMesh_MultiTexture(mesh,num_used_textures,0,Textures,NULL,NULL);
	
	w->SetShader(0);
	w->ClearImageCoords();
	if (projection) w->RetrieveFrameBuffer(fb,projection);

	w->ClearFrameBuffer(fb);
}

FrameBuffer_struct CameraProjectionRenderer::BuildProjectCameraFrameBuffer(int width_trg,int height_trg,int default_buffer_options) {
	UINT texture=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width_trg,height_trg,&texture,1,TEXTURE_CH_RGBA,default_buffer_options);

	return fb;
}

void CameraProjectionRenderer::VBR(FrameBuffer_struct *vdr_fb,
									 int width_src1,int height_src1,
									 Matrix *Int_src1,Matrix *OGL_Ext_src1,
									 UINT image_src1,UINT ogl_depth_src1,UINT mask_src1,
									 int width_src2,int height_src2,
									 Matrix *Int_src2,Matrix *OGL_Ext_src2,
									 UINT image_src2,UINT ogl_depth_src2,UINT mask_src2,
									 int width_trg,int height_trg,
									 Matrix *Int_trg,Matrix *OGL_Ext_trg,
									 double Z_Discrepancy,
									 HyperMesh<3> *mesh,Bitmap<ColorRGBA> *vdr_image) {

	
	// Get information
	Vector<4> tmp;
	OpenGL_State X1,X2;
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int_src1,OGL_Ext_src1,width_src1,height_src1,&X1,w->clip_min(),w->clip_max());
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int_src2,OGL_Ext_src2,width_src2,height_src2,&X2,w->clip_min(),w->clip_max());
	// Get Projection Center
		Vector<3> proj_c1,proj_c2;
		Matrix E(3,4);
		ExttoE(OGL_Ext_src1,&E);
		E.GetColumn(3,proj_c1.GetList());
		ExttoE(OGL_Ext_src2,&E);
		E.GetColumn(3,proj_c2.GetList());


	w->SetFrameBuffer(&(vdr_fb[0]));
		w->SetDrawBuffers(3);

		// continuare qui


		w->SetDrawBuffer(0);
	w->ClearFrameBuffer(&(vdr_fb[0]));
}





char L0Norm_VertexShader[] = 
"void main( void ) {\n"
"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"gl_Position = ftransform();\n"
"}\n";
char L0Norm_FragmentShader[] = 
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"uniform float     factor;\n"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 pos_r=gl_TexCoord[0].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos=pos_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		vec4 b=texture2D(map2,pos);\n"
"		if (a.r!=b.r) sum = sum + 1.0;\n" 
"		pos.s=pos.s+offset_x;\n"
"	}\n"
"   pos_r.t=pos_r.t+offset_y;\n"
"}\n"
"sum=sum/factor;\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";


L0NormRenderer::L0NormRenderer(AdvancedRenderWindow *w,int width,int height) {
	this->w=w;

	div_x=2;
	while (true) {
		int r_w=width/div_x;
		if (r_w==0) break;
		if (r_w*div_x!=width) break;
		div_x*=2;
		if (div_x>16) break;
	}
	div_x/=2;
	div_y=2;
	while (true) {
		int r_h=height/div_y;
		if (r_h==0) break;
		if (r_h*div_y!=height) break;
		div_y*=2;
		if (div_y>16) break;
	}
	div_y/=2;

	// NOTE: 1/256 not supported in opengl unless we consider another variable type!! INT32 or INT16
	while(div_x*div_y>=256) div_x/=2; 
	log_factor=(int)(log((double)div_x*(double)div_y)/log(2.0));


	this->offset_x=(1.0/width);
	this->offset_y=(1.0/height);

	this->width=width/div_x;
	this->height=height/div_y;
	
	UINT texture=NULL;
	fb=w->CreateFrameBuffer(this->width,this->height,&texture,1,TEXTURE_CH_LUMINANCE8,TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);

	Shader_L0Norm=w->CreateShaderFromCode(L0Norm_VertexShader,L0Norm_FragmentShader,false);
	out=new Bitmap<GreyLevel>(this->width,this->height);
}

L0NormRenderer::~L0NormRenderer() {
	w->DeleteShader(Shader_L0Norm);
	w->DeleteFrameBuffer(&fb);
	delete out;
}

unsigned int L0NormRenderer::L0Norm(UINT tex1,UINT tex2) {
	w->SetFrameBuffer(&fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	int area=div_x*div_y;

	w->SetShader(Shader_L0Norm);
	w->SetUniformVariable(Shader_L0Norm,"map1",0);
	w->SetUniformVariable(Shader_L0Norm,"map2",1);
	w->SetUniformVariable(Shader_L0Norm,"div_x",div_x);
	w->SetUniformVariable(Shader_L0Norm,"div_y",div_y);
	w->SetUniformVariable(Shader_L0Norm,"offset_x",(double)offset_x);
	w->SetUniformVariable(Shader_L0Norm,"offset_y",(double)offset_y);
	w->SetUniformVariable(Shader_L0Norm,"factor",(double)area);
	UINT InTextures[]={tex1,tex2};
	w->Run2DShader(2,InTextures);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb);


	BYTE *x=out->getBuffer();
	unsigned int sum=0;
	for(int i=0;i<out->width*out->height;i++,x++) {
		int v=((int)(*x))<<log_factor;
		v=(v>>8)+((v>>7)&0x1);
		
		// equivalent to Approx((*x/255.0)*area);
		sum+=v;  
	}

	return sum;
}


char R360VertexShader[] = 
"uniform float a;\n"
"uniform float b;\n"
"void main( void ) {\n"
"gl_FrontColor = gl_Color;\n"
"vec4  pos4 = gl_Vertex;\n"
"vec3  pos3 = vec3(pos4.x/pos4.w,pos4.y/pos4.w,pos4.z/pos4.w);\n"
"float d    = sqrt(dot(pos3,pos3));\n" 
"float y    = 2.0*((asin(pos3.y/d)/3.14159265358979323846)+0.5)-1.0;\n" 
"float x    = 2.0*((atan(pos3.x,pos3.z)/(2.0*3.14159265358979323846))+0.5)-1.0;\n" 
"gl_Position = vec4(x,y,(a*d+b),1.0);\n" 
"}\n";
char R360FragmentShader[] = 
"void main(void) {"
"gl_FragColor=gl_Color;"
"}";
char StdVShaderDepth[] = 
"varying vec3 Normal;\n"
"varying vec4 VertexPosition;\n"
"void main( void ) {\n"
"	VertexPosition = gl_Vertex;\n"
"   Normal = normalize(gl_Normal.xyz);\n"
"	gl_Position = ftransform();\n"
"}\n";
char EdgeShader[] = 
"varying vec3 Normal;"
"varying vec4 VertexPosition;"
"uniform vec3 CameraOrigin;"
"uniform float threshold;"
"void main(void) {"
"	vec3 ViewDirection = normalize(CameraOrigin - vec3(VertexPosition));"
"	vec3 vNormal = normalize(Normal);"
"	float fDot=abs(dot(vNormal,ViewDirection));"
"	if (fDot<threshold) gl_FragData[0]=vec4(1.0,1.0,1.0,1.0);"
"	else gl_FragData[0]=vec4(0.0,0.0,0.0,1.0);"
"}";
	

ModelRenderer::ModelRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_NormalRenderer=w->CreateShader(DATA_DIRECTORY"StdShader.vert.c",DATA_DIRECTORY"Shader_NormalRenderer.frag.c");
	Shader_edge_renderer=w->CreateShaderFromCode(StdVShaderDepth,EdgeShader,false);
	Shader_360=w->CreateShaderFromCode(R360VertexShader,R360FragmentShader,false);
}

ModelRenderer::~ModelRenderer() {
	w->DeleteShader(Shader_NormalRenderer);
	w->DeleteShader(Shader_edge_renderer);
	w->DeleteShader(Shader_360);
}

void ModelRenderer::RenderTexRGBA(FrameBuffer_struct *fb,CameraParameters *camera,HyperMesh<3> *mesh,Bitmap<ColorRGBA> *image) {
	RenderTexRGBA(fb,camera->w,camera->h,&(camera->Int),&(camera->OGL_Ext),mesh,image);
}

#define ModelRenderer_INIT									\
	w->SetFrameBuffer(fb);									\
	w->SetDrawBuffer(0);									\
	w->SetReadBuffer(0);									\
	glDepthMask(1);											\
	SetStandardOGLEnvironment(true,false,true);				\
		w->SetIE(Int,width,height,OGL_Ext);					\
		glViewport(0,0,fb->w,fb->h);						\
			glDisable(GL_LIGHTING);							\
			glDisable(GL_BLEND);							\
			glEnable(GL_DEPTH_TEST);						\
			glColor4f(1.0,1.0,1.0,1.0);						

#define ModelRenderer_EXIT									\
		w->ClearImageCoords();								\
		if (image) w->RetrieveFrameBuffer(fb,image);		\
	w->ClearFrameBuffer(fb);

void ModelRenderer::RenderTexRGBA(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorRGBA> *image) {
		ModelRenderer_INIT
		
			w->DrawHyperMesh_Textured_Flat(mesh);

		ModelRenderer_EXIT
}

void ModelRenderer::RenderTexRGB(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<ColorRGB> *image) {
		ModelRenderer_INIT
		
			w->DrawHyperMesh_Textured_Flat(mesh);

		ModelRenderer_EXIT
}
void ModelRenderer::RenderNormals(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image) {
		ModelRenderer_INIT

			Vector<3> proj_c;
			Matrix E(3,4);
			ExttoE(OGL_Ext,&E);
			E.GetColumn(3,proj_c.GetList());

			w->SetIE(Int,width,height,OGL_Ext);
			glViewport(0,0,fb->w,fb->h);
			w->SetShader(Shader_NormalRenderer);
				w->SetUniformVariable(Shader_NormalRenderer,"CameraOrigin",&proj_c);

				w->DrawHyperMesh_Flat(mesh);
			w->SetShader(0);

		ModelRenderer_EXIT
}


void ModelRenderer::RenderBox(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Vector<3> *center,Matrix *axes,Vector<3> *sizes,Bitmap<GreyLevel> *image) {
		ModelRenderer_INIT
		
			w->Draw3DHyperBox(center,axes,sizes,0xFFFFFFFF,1.0,0xFFFFFF,DRAW_FILLED);

		ModelRenderer_EXIT
}

inline void TransformVertex(Vector<3> x,int width,int height,Matrix *P) {
	Vector<3> V;
	Vector<2> T;
	Vector<4> omo;
	
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1;
	Multiply(P,omo,V.GetList());

	T[0]=V[0]/V[2];
	T[1]=V[1]/V[2];

	T[0]-=(width-1)/2.0;					// NB: Confermato visivamente per essere coerente con projection
	T[1]-=(height-1)/2.0;					//     della classe View; ovvero Approx(Central_Projection(P*x))
											// La conferma è stata data con GL_POINT, ovviamente le faccie (dato l'algoritmo interno di opengl)
											// possono risultare un po' traslate ma la proiezione dei loro punti è esatta.
	glVertex2d(T[0],T[1]);
}

void ModelRenderer::RenderFootPrint(FrameBuffer_struct *fb,int width,int height,Matrix *real_P,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	SetStandardOGLEnvironment(false,false,true);

		// opengl int/ext 
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		GLdouble mat[16];
		mat[0]=2.0/width;mat[1]=0;mat[2]=0;mat[3]=0;			// NB: Confermato visivamente per essere coerente con projection
		mat[4]=0;mat[5]=2.0/height;mat[6]=0;mat[7]=0;			//     della classe View; ovvero Approx(Central_Projection(P*x))
		mat[8]=0;mat[9]=0;mat[10]=0;mat[11]=0;					// La conferma è stata data con GL_POINT, ovviamente le facce (dato l'algoritmo interno di opengl)
		mat[12]=0;mat[13]=0;mat[14]=0;mat[15]=1;				// possono risultare un po' traslate ma la proiezione dei loro punti è esatta.
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixd(mat);
		
		glViewport(0,0,fb->w,fb->h);
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glShadeModel(GL_FLAT);
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
			glColor4f(1.0,1.0,1.0,1.0);
			
			{
				HyperFace<3> *f=mesh->Faces.getMem();
				Vector<3>    *P=mesh->Points.getMem();
			
				glBegin(GL_TRIANGLES);
				for (int i=0;i<mesh->num_f;i++,f++) {
					TransformVertex(P[f->Point[0]],width,height,real_P);
					TransformVertex(P[f->Point[1]],width,height,real_P);
					TransformVertex(P[f->Point[2]],width,height,real_P);
				}
				glEnd();
			}
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		if (image) w->RetrieveFrameBuffer(fb,image);
	w->ClearFrameBuffer(fb);
}
void ModelRenderer::RenderFootPrint(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	SetStandardOGLEnvironment(false,false,true);

	glDisable(GL_CULL_FACE);

		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glShadeModel(GL_FLAT);
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
			glColor4f(1.0,1.0,1.0,1.0);

			HyperFace<3> *f=mesh->Faces.getMem();
			Vector<3> *P=mesh->Points.getMem();
			glBegin(GL_TRIANGLES);
			for (int i=0;i<mesh->num_f;i++,f++) {
				w->Vertex(P[f->Point[0]]);
				w->Vertex(P[f->Point[1]]);
				w->Vertex(P[f->Point[2]]);
			}
			glEnd();

		w->ClearImageCoords();
		if (image) w->RetrieveFrameBuffer(fb,image);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}







void ModelRenderer::RenderEdges(FrameBuffer_struct *fb,int width,int height,float threshold,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,Vector<3> prj_center,HyperMesh<3> *mesh,Bitmap<GreyLevel> *image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	SetStandardOGLEnvironment(true,false,true);

	glDisable(GL_CULL_FACE);

		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
			
			w->SetShader(Shader_edge_renderer);
			w->SetUniformVariable(Shader_edge_renderer,"CameraOrigin",&prj_center);
			w->SetUniformVariable(Shader_edge_renderer,"threshold",threshold);

			HyperFace<3> *f=mesh->Faces.getMem();
			Vector<3>    *P=mesh->Points.getMem();
			Vector<3>    *N=mesh->Normals.getMem();
			Vector<3>    *p,*n;

			glBegin(GL_TRIANGLES);
			for (int i=0;i<mesh->num_f;i++,f++) {
				n=&(N[f->Point[0]]);
				p=&(P[f->Point[0]]);
				glNormal3d((*n)[0],(*n)[1],(*n)[2]);
				glVertex3d((*p)[0],(*p)[1],(*p)[2]);
				
				n=&(N[f->Point[1]]);
				p=&(P[f->Point[1]]);
				glNormal3d((*n)[0],(*n)[1],(*n)[2]);
				glVertex3d((*p)[0],(*p)[1],(*p)[2]);
				
				n=&(N[f->Point[2]]);
				p=&(P[f->Point[2]]);
				glNormal3d((*n)[0],(*n)[1],(*n)[2]);
				glVertex3d((*p)[0],(*p)[1],(*p)[2]);
			}
			glEnd();

		w->ClearImageCoords();
		w->SetShader(0);

		if (image) w->RetrieveFrameBuffer(fb,image);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}






// Angle accuracy in image coordinates 
#define RENDER_MODEL360_ACCURACY 0.05   // 5%   
#define RENDER_MODEL360_LOOP_TH 0.7

inline void Rect2Polar(Vector<3> *p,Vector<3> *r) {
	(*r)[2]=sqrt((*p)[0]*(*p)[0]+(*p)[1]*(*p)[1]+(*p)[2]*(*p)[2]);
	(*r)[1]=2.0*((asin((*p)[1]/(*r)[2])/3.14159265358979323846)+0.5)-1.0;
	(*r)[0]=2.0*((atan2((*p)[0],(*p)[2])/(2.0*3.14159265358979323846))+0.5)-1.0;
}
inline void Rect2Polar(Vector<3> *p,Vector<2> *r) {
	double d=sqrt((*p)[0]*(*p)[0]+(*p)[1]*(*p)[1]+(*p)[2]*(*p)[2]);
	(*r)[1]=2.0*((asin((*p)[1]/d)/3.14159265358979323846)+0.5)-1.0;
	(*r)[0]=2.0*((atan2((*p)[0],(*p)[2])/(2.0*3.14159265358979323846))+0.5)-1.0;
}
void RenderFace(AdvancedRenderWindow *w,Vector<3> *p0,Vector<3> *p1,Vector<3> *p2) {
	Vector<2> r0,r1,r2;

	Rect2Polar(p0,&r0);
	Rect2Polar(p1,&r1);
	Rect2Polar(p2,&r2);

	if (fabs(r0[0])>=0.99999) return;
	if (fabs(r1[0])>=0.99999) return;
	if (fabs(r2[0])>=0.99999) return;

	if ((fabs(r0[0]-r1[0])>RENDER_MODEL360_LOOP_TH) || (fabs(r1[0]-r2[0])>RENDER_MODEL360_LOOP_TH) || (fabs(r2[0]-r0[0])>RENDER_MODEL360_LOOP_TH)) return;


	if (((r0-r1).Norm2()>RENDER_MODEL360_ACCURACY) || ((r1-r2).Norm2()>RENDER_MODEL360_ACCURACY) || ((r2-r0).Norm2()>RENDER_MODEL360_ACCURACY)) {
		Vector<3> np0,np1,np2;
		np0=*p0;
		np1=0.5*(*p0+*p1);
		np2=0.5*(*p0+*p2);
		RenderFace(w,&np0,&np1,&np2);

		np0=0.5*(*p1+*p2);
		RenderFace(w,&np0,&np2,&np1);

		np2=*p1;
		RenderFace(w,&np0,&np1,&np2);

		np1=*p2;
		np2=0.5*(*p0+*p2);
		RenderFace(w,&np0,&np1,&np2);
	}

	glVertex3d((*p0)[0],(*p0)[1],(*p0)[2]);
	glVertex3d((*p1)[0],(*p1)[1],(*p1)[2]);
	glVertex3d((*p2)[0],(*p2)[1],(*p2)[2]);
}
void RenderLine(AdvancedRenderWindow *w,Vector<3> *p0,Vector<3> *p1) {
	Vector<2> r0,r1;

	Rect2Polar(p0,&r0);
	Rect2Polar(p1,&r1);

	if (fabs(r0[0])>=0.99999) return;
	if (fabs(r1[0])>=0.99999) return;

	if (fabs(r0[0]-r1[0])>RENDER_MODEL360_LOOP_TH) return;

	if ((r0-r1).Norm2()>RENDER_MODEL360_ACCURACY) {
		Vector<3> np0,np1;
		np0=*p0;
		np1=0.5*(*p0+*p1);
		RenderLine(w,&np0,&np1);

		np0=*p1;
		RenderLine(w,&np1,&np0);
	}
	
	w->DrawLine(*p0,*p1,0xFFFFFF,4.0);
}

void ModelRenderer::RenderModel360(FrameBuffer_struct *fb,int width,int height,float near_plane,float far_plane,HyperMesh<3> *mesh,Bitmap<ColorRGB> *image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	SetStandardOGLEnvironment(true,false,true);


	float a=2.0f/(far_plane-near_plane);
	float b=1.0f-a*far_plane;

	glDisable(GL_CULL_FACE);
		w->SetShader(Shader_360);
		w->SetUniformVariable(Shader_360,"a",a);
		w->SetUniformVariable(Shader_360,"b",b);
		glViewport(0,0,fb->w,fb->h);
			HyperFace<3> *f=mesh->Faces.getMem();
			List<typename Vector<3>> *P_list=&mesh->Points;
			Vector<3> *P=mesh->Points.getMem();

			glDepthMask(0);
			for (int i=0;i<mesh->num_f;i++,f++) {
				if ((P[f->Point[0]].Norm2()>far_plane) || (P[f->Point[1]].Norm2()>far_plane) || (P[f->Point[2]].Norm2()>far_plane)) continue;
				RenderLine(w,&P[f->Point[0]],&P[f->Point[1]]);
				RenderLine(w,&P[f->Point[1]],&P[f->Point[2]]);
				RenderLine(w,&P[f->Point[0]],&P[f->Point[2]]);
			}
			glDepthMask(1);

			glColor4f(1.0,0.0,0.0,1.0);

			f=mesh->Faces.getMem();

			glBegin(GL_TRIANGLES);
			for (int i=0;i<mesh->num_f;i++,f++) {
				if ((P[f->Point[0]].Norm2()>far_plane) || (P[f->Point[1]].Norm2()>far_plane) || (P[f->Point[2]].Norm2()>far_plane)) continue;
				glNormal3dv(f->Normal(P_list).GetList());
				RenderFace(w,&P[f->Point[0]],&P[f->Point[1]],&P[f->Point[2]]);
			}
			glEnd();

			Vector<3> p1,p2,o1,o2,h_,f_;

			// TODOLUCA shift
			b=b-0.0001f;
			w->SetUniformVariable(Shader_360,"b",b);

			glDepthFunc(GL_LEQUAL);
			double EdgeCreaseAngle=1.0;
			for(int i=0;i<mesh->num_p;i++) {
				p1=mesh->Points[i];
				if (p1.Norm2()>far_plane) continue;
				for(int j=0;j<mesh->NumNeighbours[i];j++) {
					//   o1
					// i -- p2
					//   o2
					p2=mesh->Points[mesh->Neighbours[i][j]];
					o1=mesh->Points[mesh->Edge_Faces[i][j].free1];
					o2=mesh->Points[mesh->Edge_Faces[i][j].free2];

					h_=((p2-o2)^(p1-o2)).Versore();
					f_=((p1-o1)^(p2-o1)).Versore();

					if (fabs(h_*f_)<0.5) {
						RenderLine(w,&p1,&p2);
					}
				}
			}

		w->SetShader(0);
		w->ClearImageCoords();

		if (image) w->RetrieveFrameBuffer(fb,image);
		
	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}







FrameBuffer_struct ModelRenderer::BuildRenderFrameBufferTex(int width,int height,int default_buffer_options) {
	UINT texture=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&texture,1,TEXTURE_CH_RGBA,default_buffer_options);

	return fb;
}

FrameBuffer_struct ModelRenderer::BuildRenderFrameBufferFootPrint(int width,int height,int default_buffer_options) {
	UINT texture=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&texture,1,TEXTURE_CH_LUMINANCE8,default_buffer_options);

	return fb;
}


void ModelRenderer::RenderWireFrame(FrameBuffer_struct *fb,int width,int height,Matrix *Int,Matrix *OGL_Ext,Matrix *real_Ext,HyperMesh<3> *mesh,double EdgeCreaseAngle,Bitmap<ColorRGB> *image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	glDepthMask(1);
	SetStandardOGLEnvironment(true,false,true);

	glDisable(GL_CULL_FACE);

		w->SetIE(Int,width,height,OGL_Ext);
		glViewport(0,0,fb->w,fb->h);
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
			glColor4f(1.0,0.0,0.0,1.0);

			w->DrawHyperMesh(mesh);

			Vector<3> p1,p2,o1,o2,h,f;

			for(int i=0;i<mesh->num_p;i++) {
				p1=mesh->Points[i];
				for(int j=0;j<mesh->NumNeighbours[i];j++) {
					//   o1
					// i -- p2
					//   o2
					p2=mesh->Points[mesh->Neighbours[i][j]];
					o1=mesh->Points[mesh->Edge_Faces[i][j].free1];
					o2=mesh->Points[mesh->Edge_Faces[i][j].free2];

					h=((p2-o2)^(p1-o2)).Versore();
					f=((p1-o1)^(p2-o1)).Versore();

					if (h*f<EdgeCreaseAngle) 
						w->DrawLine(p1,p2,0xFFFFFF,4.0);
				}
			}
			

		w->ClearImageCoords();
		if (image) w->RetrieveFrameBuffer(fb,image);

	glEnable(GL_CULL_FACE);
	w->ClearFrameBuffer(fb);
}




















































ImageOperationsRenderer::ImageOperationsRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	
	Shader_uint8RGBA2floatHCY= w->CreateShader(DATA_DIRECTORY"2DShader.vert.c", DATA_DIRECTORY"ColorConversion.frag.c");
	Shader_MatchingYH=w->CreateShader(DATA_DIRECTORY"2DShader.vert.c",DATA_DIRECTORY"Matching_YH.frag.c",true);
}

ImageOperationsRenderer::~ImageOperationsRenderer() {
	w->DeleteShader(Shader_uint8RGBA2floatHCY);
	w->DeleteShader(Shader_MatchingYH);
}

void ImageOperationsRenderer::RGBA2HCYA(FrameBuffer_struct *fb, UINT RGBA_Image) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
		w->SetShader(Shader_uint8RGBA2floatHCY);
			w->SetUniformVariable(Shader_uint8RGBA2floatHCY, "ImageSrc", (int)0);
			w->Run2DShader(RGBA_Image);
			w->SetShader(0);

		glDepthMask(1);
	w->ClearFrameBuffer(fb);
}

void ImageOperationsRenderer::InvertTexture(FrameBuffer_struct *fb,UINT Texture) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
			w->SetImageCoords();
		
			glEnable(GL_TEXTURE_2D);
			w->Set_Program_0();
			glBindTexture(GL_TEXTURE_2D,Texture);
			glDisable(GL_CULL_FACE);
			
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, 1.0);
				glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, 1.0);
				glTexCoord2f(1.0f, 1.0);glVertex2f(1.0, -1.0);
				glTexCoord2f(0.0f, 1.0);glVertex2f(-1.0, -1.0);
				glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, 1.0);
			glEnd();

			w->ClearImageCoords();
		
		glDepthMask(1);
	w->ClearFrameBuffer(fb);
}


void ImageOperationsRenderer::YHHAMatching(FrameBuffer_struct *fb,UINT YHHA_Image1,UINT YHHA_Image2,int win_size,int dim_search,double min_var,double max_var,Bitmap<GreyLevel> *matches) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);

			w->SetShader(Shader_MatchingYH);
			w->SetUniformVariable(Shader_MatchingYH,"Image1",(int)0);
			w->SetUniformVariable(Shader_MatchingYH,"Image2",(int)1);
			w->SetUniformVariable(Shader_MatchingYH,"image_step_x",(double)(1.0/fb->w));
			w->SetUniformVariable(Shader_MatchingYH,"image_step_y",(double)(1.0/fb->h));
			w->SetUniformVariable(Shader_MatchingYH,"win_size",(int)win_size);
			w->SetUniformVariable(Shader_MatchingYH,"dim_search",(int)dim_search);
			w->SetUniformVariable(Shader_MatchingYH,"max_var",(double)max_var);
			w->SetUniformVariable(Shader_MatchingYH,"min_var",(double)min_var);

			UINT Textures[2];Textures[0]=YHHA_Image1;Textures[1]=YHHA_Image2;
			w->Run2DShader(2,Textures);
			w->SetShader(0);

		glDepthMask(1);
		if (matches) w->RetrieveFrameBuffer(fb,matches);
	w->ClearFrameBuffer(fb);
}

FrameBuffer_struct ImageOperationsRenderer::BuildYHHAConversionFrameBuffer(int width,int height,int default_buffer_options) {
	UINT YHHA_Map=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&YHHA_Map,1,TEXTURE_CH_32_32_32_32,default_buffer_options,false);

	return fb;
}




















MedianSoftRenderer::MedianSoftRenderer(AdvancedRenderWindow *w,int width,int height,int predicted_n_samples,int initial_n_samples) {
	this->w=w;
	this->width=width;
	this->height=height;
	this->Images=NULL;
	this->n_samples=predicted_n_samples;

	SNEWA_P(Images,Array<BYTE>*,(3*width*height));
	for(int i=0;i<(3*width*height);i++) {
		Images[i]=new Array<BYTE>(initial_n_samples);
	}
}
MedianSoftRenderer::~MedianSoftRenderer() {
	SDELETEA_REC(Images,(3*width*height));
}
void MedianSoftRenderer::Update(Bitmap<ColorRGBA> *image_to_consider) {
	int dim          =width*height;
	ColorRGBA   *buff=image_to_consider->getBuffer();
	Array<BYTE>   **m=Images;

	for(int i=0;i<dim;i++,buff++,m+=3) {
		
		ColorRGBA tmp=*buff;
		int alpha=ALPHA_C(tmp);
		if (alpha!=0) {

			BYTE r=RED_C(tmp);
			BYTE g=GREEN_C(tmp);
			BYTE b=BLUE_C(tmp);
			
			(*(m))->enlarge_to_at_least(n_samples);
			(*(m+1))->enlarge_to_at_least(n_samples);
			(*(m+2))->enlarge_to_at_least(n_samples);

			(*(m))->append(r);
			(*(m+1))->append(g);
			(*(m+2))->append(b);
		}
	}
}
void MedianSoftRenderer::ExtractMedian(Bitmap<ColorRGBA> *Median) {
	int dim          =width*height;
	ColorRGBA   *buff=Median->getBuffer();
	Array<BYTE>   **m=Images;

	for(int i=0;i<dim;i++,buff++,m+=3) {
		int num_elements=(*m)->numElements();
		
		if (num_elements!=0) {
			(*(m))->sort(compare_byte);
			(*(m+1))->sort(compare_byte);
			(*(m+2))->sort(compare_byte);

			int middle=Approx(num_elements/2.0);
			
			(*buff)=RGBA2ColorRGBA((*(*(m)))[middle],
								   (*(*(m+1)))[middle],
								   (*(*(m+2)))[middle],num_elements);
		} else {
			(*buff)=0x00FFFFFF;
		}
	}
}

void MedianSoftRenderer::ExtractMedian(Bitmap<ColorRGBA> *Median,Bitmap<ColorRGBA> *std) {
	int dim              =width*height;
	ColorRGBA   *buff    =Median->getBuffer();
	ColorRGBA   *std_buff=std->getBuffer();
	Array<BYTE>   **m    =Images;

	for(int i=0;i<dim;i++,buff++,std_buff++,m+=3) {
		int num_elements=(*m)->numElements();
		
		if (num_elements!=0) {
			(*(m))->sort(compare_byte);
			(*(m+1))->sort(compare_byte);
			(*(m+2))->sort(compare_byte);

			int middle=Approx(num_elements/2.0);
			
			BYTE m_r=(*(*(m)))[middle];
			BYTE m_g=(*(*(m+1)))[middle];
			BYTE m_b=(*(*(m+2)))[middle];
			(*buff)=RGBA2ColorRGBA(m_r,m_g,m_b,num_elements);

			float s_r=0.0,s_g=0.0,s_b=0.0;
			if (num_elements!=1) {
				float tmp;
				for(int j=0;j<num_elements;j++) {
					tmp=(float)((*(*(m)))[j])  -(float)m_r;
					s_r+=tmp*tmp;
					tmp=(float)((*(*(m+1)))[j])-(float)m_g;
					s_g+=tmp*tmp;
					tmp=(float)((*(*(m+2)))[j])-(float)m_b;
					s_b+=tmp*tmp;
				}
				s_r=sqrt(s_r/(num_elements-1.0f));
				s_g=sqrt(s_g/(num_elements-1.0f));
				s_b=sqrt(s_b/(num_elements-1.0f));
			}

			(*std_buff)=RGBA2ColorRGBA(s_r,s_g,s_b,num_elements);
		} else {
			(*buff)=0x00FFFFFF;
			(*std_buff)=0x00FFFFFF;
		}
	}
}



MeanStdSoftRenderer::MeanStdSoftRenderer(AdvancedRenderWindow *w,int width,int height) {
	this->w=w;
	this->Mean=NULL;
	this->Std=NULL;


	SNEWA_P(Mean,Bitmap<ColorFloat> *,4);
	for(int i=0;i<4;i++) {
		SNEW(Mean[i],Bitmap<ColorFloat>(width,height));
		Mean[i]->Clear();
	}
	SNEWA_P(Std,Bitmap<ColorFloat> *,3);
	for(int i=0;i<3;i++) {
		SNEW(Std[i],Bitmap<ColorFloat>(width,height));
		Std[i]->Clear();
	}
	
}

MeanStdSoftRenderer::~MeanStdSoftRenderer() {
	SDELETEA_REC(Mean,4);
	SDELETEA_REC(Std,3);
}

void MeanStdSoftRenderer::Clear() {
	for(int i=0;i<4;i++) {
		Mean[i]->Clear();
	}
	for(int i=0;i<3;i++) {
		Std[i]->Clear();
	}
}

void MeanStdSoftRenderer::Update(Bitmap<ColorRGBA> *image_to_consider) {
	int dim=image_to_consider->width*image_to_consider->height;
	ColorRGBA *buff=image_to_consider->getBuffer();
	ColorFloat *m_r=Mean[0]->getBuffer();
	ColorFloat *m_g=Mean[1]->getBuffer();
	ColorFloat *m_b=Mean[2]->getBuffer();
	ColorFloat *m_a=Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();
	ColorFloat *s_g= Std[1]->getBuffer();
	ColorFloat *s_b= Std[2]->getBuffer();

	for(int i=0;i<dim;i++,buff++,m_r++,m_g++,m_b++,m_a++,s_r++,s_g++,s_b++) {
		
		ColorRGBA tmp=*buff;
		int alpha=ALPHA_C(tmp);
		if (alpha!=0) {

			int r=RED_C(tmp);
			int g=GREEN_C(tmp);
			int b=BLUE_C(tmp);
			
			(*m_r)+=r;
			(*m_g)+=g;
			(*m_b)+=b;
			(*m_a)+=1.0;
			
			(*s_r)+=r*r;
			(*s_g)+=g*g;
			(*s_b)+=b*b;
		}
	}

}

void MeanStdSoftRenderer::Update(Bitmap<ColorFloat> *image_to_consider,ColorFloat no_data_value) {
	int dim=image_to_consider->width*image_to_consider->height;
	ColorFloat *buff=image_to_consider->getBuffer();
	ColorFloat *m_r=Mean[0]->getBuffer();
	ColorFloat *m_a=Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();

	for(int i=0;i<dim;i++,buff++,m_r++,m_a++,s_r++) {
		ColorFloat tmp=*buff;
		if (tmp!=no_data_value) {
			(*m_r)+=tmp;
			(*m_a)+=1.0;
			(*s_r)+=tmp*tmp;
		}
	}

}

void MeanStdSoftRenderer::ExtractMeanStd(Bitmap<ColorFloat> *MeanOut,Bitmap<ColorFloat> *StdOut,ColorFloat no_data_value) {
	int dim=Mean[0]->width*Mean[0]->height;

	ColorFloat *m_r= Mean[0]->getBuffer();
	ColorFloat *m_a= Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();
	ColorFloat *m  = MeanOut->getBuffer();
	ColorFloat *s  = StdOut->getBuffer();

	for(int i=0;i<dim;i++,m++,s++,m_r++,m_a++,s_r++) {
		float alpha=(*m_a);
		if (alpha!=0.0) {
			float r=(*m_r)/alpha;
			float r2=sqrt(((*s_r)-(alpha*r*r))/(alpha-1.0f));
			(*m)=r;
			(*s)=r2;
		} else {
			(*m)=no_data_value;
			(*s)=0.0;
		}
	}

}

void MeanStdSoftRenderer::ExtractMeanStd(Bitmap<ColorRGBA> *MeanOut,Bitmap<ColorRGBA> *StdOut) {
	int dim=Mean[0]->width*Mean[0]->height;

	ColorFloat *m_r=Mean[0]->getBuffer();
	ColorFloat *m_g=Mean[1]->getBuffer();
	ColorFloat *m_b=Mean[2]->getBuffer();
	ColorFloat *m_a=Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();
	ColorFloat *s_g= Std[1]->getBuffer();
	ColorFloat *s_b= Std[2]->getBuffer();
	ColorRGBA  *m  = MeanOut->getBuffer();
	ColorRGBA  *s  = StdOut->getBuffer();

	for(int i=0;i<dim;i++,m++,s++,m_r++,m_g++,m_b++,m_a++,s_r++,s_g++,s_b++) {
		float alpha=(*m_a);
		if (alpha!=0.0) {
			float r=(*m_r)/alpha;
			float g=(*m_g)/alpha;
			float b=(*m_b)/alpha;

			(*m)=RGBA2ColorRGBA(r,g,b,alpha);

			float r2=sqrt(((*s_r)-(alpha*r*r))/(alpha-1.0f));
			float g2=sqrt(((*s_g)-(alpha*g*g))/(alpha-1.0f));
			float b2=sqrt(((*s_b)-(alpha*b*b))/(alpha-1.0f));
			
			(*s)=RGBA2ColorRGBA(r2,g2,b2,alpha);
		} else {
			(*m)=RGBA2ColorRGBA(255,255,255,0.0);
			(*s)=RGBA2ColorRGBA(255,255,255,0.0);
		}
	}

}























































//////////////////////////////////////////////////////////
MeanStdSampleMaxSoftRenderer::MeanStdSampleMaxSoftRenderer(int width,int height,int max_number_of_samples) {
	this->Mean=NULL;
	this->Std=NULL;
	this->max_number_of_samples=max_number_of_samples;

	SNEWA_P(Mean,Bitmap<ColorFloat> *,4);
	for(int i=0;i<4;i++) {
		SNEW(Mean[i],Bitmap<ColorFloat>(width,height));
		Mean[i]->Clear();
	}
	SNEWA_P(Std,Bitmap<ColorFloat> *,4);
	for(int i=0;i<4;i++) {
		SNEW(Std[i],Bitmap<ColorFloat>(width,height));
		if (i==3) Std[i]->Clear(FLT_MAX);
		else Std[i]->Clear();
	}
	
}

MeanStdSampleMaxSoftRenderer::~MeanStdSampleMaxSoftRenderer() {
	SDELETEA_REC(Mean,4);
	SDELETEA_REC(Std,4);
}

bool MeanStdSampleMaxSoftRenderer::Update(float sample_index,Bitmap<ColorRGBA> *sample,int &num_pixel_not_filled,double &not_completed_percentage) {
	int dim=sample->width*sample->height;
	ColorRGBA  *buff=sample->getBuffer();
	ColorFloat *m_r=Mean[0]->getBuffer();
	ColorFloat *m_g=Mean[1]->getBuffer();
	ColorFloat *m_b=Mean[2]->getBuffer();
	ColorFloat *m_a=Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();
	ColorFloat *s_g= Std[1]->getBuffer();
	ColorFloat *s_b= Std[2]->getBuffer();
	ColorFloat *s_a= Std[3]->getBuffer();

	int filled=0,max_filled=0;
	for(int i=0;i<dim;i++,buff++,m_r++,m_g++,m_b++,m_a++,s_r++,s_g++,s_b++,s_a++) {
		
		if ((*m_a)<max_number_of_samples) {
			ColorRGBA tmp=*buff;
			int alpha=ALPHA_C(tmp);
			if (alpha!=0) {

				int r=RED_C(tmp);
				int g=GREEN_C(tmp);
				int b=BLUE_C(tmp);
				
				(*m_r)+=r;
				(*m_g)+=g;
				(*m_b)+=b;
				(*m_a)+=1.0;
				
				(*s_r)+=r*r;
				(*s_g)+=g*g;
				(*s_b)+=b*b;
				(*s_a)=min((*s_a),sample_index);
			}
		} 
		
		max_filled+=(int)(*m_a);
		if ((*m_a)!=0.0) filled++;
	}

	num_pixel_not_filled=dim-filled;
	not_completed_percentage=((dim*max_number_of_samples)-max_filled)*1.0/(dim*max_number_of_samples);

	if (num_pixel_not_filled==0) return true;
	return false;
}

void MeanStdSampleMaxSoftRenderer::ExtractMeanStd(Bitmap<ColorRGBA> *MeanOut,Bitmap<ColorRGBA> *StdOut) {
	int dim=Mean[0]->width*Mean[0]->height;

	ColorFloat *m_r=Mean[0]->getBuffer();
	ColorFloat *m_g=Mean[1]->getBuffer();
	ColorFloat *m_b=Mean[2]->getBuffer();
	ColorFloat *m_a=Mean[3]->getBuffer();
	ColorFloat *s_r= Std[0]->getBuffer();
	ColorFloat *s_g= Std[1]->getBuffer();
	ColorFloat *s_b= Std[2]->getBuffer();
	ColorFloat *s_a= Std[3]->getBuffer();
	ColorRGBA  *m  = MeanOut->getBuffer();
	ColorRGBA  *s  = StdOut->getBuffer();

	for(int i=0;i<dim;i++,m++,s++,m_r++,m_g++,m_b++,m_a++,s_r++,s_g++,s_b++,s_a++) {
		float alpha=(*m_a);
		if (alpha!=0.0) {
			float r=(*m_r)/alpha;
			float g=(*m_g)/alpha;
			float b=(*m_b)/alpha;

			(*m)=RGBA2ColorRGBA(r,g,b,alpha);

			float r2=sqrt(((*s_r)-(alpha*r*r))/(alpha-1.0f));
			float g2=sqrt(((*s_g)-(alpha*g*g))/(alpha-1.0f));
			float b2=sqrt(((*s_b)-(alpha*b*b))/(alpha-1.0f));
			
			(*s)=RGBA2ColorRGBA(r2,g2,b2,(*s_a));
		} else {
			(*m)=RGBA2ColorRGBA(255,255,255,0.0);
			(*s)=RGBA2ColorRGBA(255,255,255,255);
		}
	}

}

void MeanStdSampleMaxSoftRenderer::Clear() {
	for(int i=0;i<4;i++) Mean[i]->Clear();
	for(int i=0;i<4;i++) {
		if (i==3) Std[i]->Clear(FLT_MAX);
		else Std[i]->Clear();
	}
}



















MedianStdSampleMaxSoftRenderer::MedianStdSampleMaxSoftRenderer(int width,int height,int max_number_of_samples) {
	this->width=width;
	this->height=height;
	this->Images=NULL;
	this->n_samples=max_number_of_samples;
	this->max_number_of_samples=max_number_of_samples;

	SNEWA_P(Images,Array<BYTE>*,(3*width*height));
	for(int i=0;i<(3*width*height);i++) {
		Images[i]=new Array<BYTE>(max_number_of_samples);
	}
}
MedianStdSampleMaxSoftRenderer::~MedianStdSampleMaxSoftRenderer() {
	SDELETEA_REC(Images,(3*width*height));
}
bool MedianStdSampleMaxSoftRenderer::Update(float sample_index,Bitmap<ColorRGBA> *image_to_consider,int &num_pixel_not_filled,double &not_completed_percentage) {
	int dim          =width*height;
	ColorRGBA   *buff=image_to_consider->getBuffer();
	Array<BYTE>   **m=Images;

	int filled=0,max_filled=0;
	for(int i=0;i<dim;i++,buff++,m+=3) {
		
		int num_ele_c=(*(m))->numElements();
		
		if (num_ele_c<max_number_of_samples) {
			ColorRGBA tmp=*buff;
			int alpha=ALPHA_C(tmp);
			if (alpha!=0) {

				BYTE r=RED_C(tmp);
				BYTE g=GREEN_C(tmp);
				BYTE b=BLUE_C(tmp);
			
				(*(m))->enlarge_to_at_least(n_samples);
				(*(m+1))->enlarge_to_at_least(n_samples);
				(*(m+2))->enlarge_to_at_least(n_samples);

				(*(m))->append(r);
				(*(m+1))->append(g);
				(*(m+2))->append(b);
				
				num_ele_c++;
			}
		}

		max_filled+=num_ele_c;
		if (num_ele_c!=0.0) filled++;
	}

	num_pixel_not_filled=dim-filled;
	not_completed_percentage=((dim*max_number_of_samples)-max_filled)*1.0/(dim*max_number_of_samples);

	if (num_pixel_not_filled==0) return true;
	return false;
}
void MedianStdSampleMaxSoftRenderer::ExtractMeanStd(Bitmap<ColorRGBA> *Median,Bitmap<ColorRGBA> *std) {
	int dim              =width*height;
	ColorRGBA   *buff    =Median->getBuffer();
	ColorRGBA   *std_buff=std->getBuffer();
	Array<BYTE>   **m    =Images;

	for(int i=0;i<dim;i++,buff++,std_buff++,m+=3) {
		int num_elements=(*m)->numElements();
		
		if (num_elements!=0) {
			(*(m))->sort(compare_byte);
			(*(m+1))->sort(compare_byte);
			(*(m+2))->sort(compare_byte);

			int middle=Approx(num_elements/2.0);
			
			BYTE m_r=(*(*(m)))[middle];
			BYTE m_g=(*(*(m+1)))[middle];
			BYTE m_b=(*(*(m+2)))[middle];
			(*buff)=RGBA2ColorRGBA(m_r,m_g,m_b,num_elements);

			float s_r=0.0,s_g=0.0,s_b=0.0;
			if (num_elements!=1) {
				float tmp;
				for(int j=0;j<num_elements;j++) {
					tmp=(float)((*(*(m)))[j])  -(float)m_r;
					s_r+=tmp*tmp;
					tmp=(float)((*(*(m+1)))[j])-(float)m_g;
					s_g+=tmp*tmp;
					tmp=(float)((*(*(m+2)))[j])-(float)m_b;
					s_b+=tmp*tmp;
				}
				s_r=sqrt(s_r/(num_elements-1.0f));
				s_g=sqrt(s_g/(num_elements-1.0f));
				s_b=sqrt(s_b/(num_elements-1.0f));
			}

			(*std_buff)=RGBA2ColorRGBA(s_r,s_g,s_b,num_elements);
		} else {
			(*buff)=0x00FFFFFF;
			(*std_buff)=0x00FFFFFF;
		}
	}
}

void MedianStdSampleMaxSoftRenderer::Clear() {
	int dim              =width*height;
	Array<BYTE>   **m    =Images;

	for(int i=0;i<dim;i++,m+=3) {
		(*(m))->clear();
		(*(m+1))->clear();
		(*(m+2))->clear();
	}
}














char StdVertexShader[] = 
"void main( void ) {"
"gl_TexCoord[0] = gl_MultiTexCoord0;"
"gl_Position = ftransform();"
"}";
char StereoFragmentShader[] = 
"uniform sampler2D map1;"
"uniform sampler2D map2;"
"void main(void) {"
"vec4 color1 = texture2D(map1,gl_TexCoord[0].st);"
"vec4 color2 = texture2D(map2,gl_TexCoord[0].st);"
"gl_FragColor=vec4(color1.r,(color1.g+color2.g)/2.0,color2.b,color1.a);"
"}";

AnaglyphsRenderer::AnaglyphsRenderer(AdvancedRenderWindow *w,int width,int height) {
	this->w=w;
	Shader_Stereo=w->CreateShaderFromCode(StdVertexShader,StereoFragmentShader,true);
	map_left=w->CreateTexture(width,height,0,GL_RGBA);		// TODOSS options can be improved?
	map_right=w->CreateTexture(width,height,0,GL_RGBA);		// TODOSS options can be improved?
	this->c_width=width;
	this->c_height=height;
}

AnaglyphsRenderer::~AnaglyphsRenderer() {
	w->DeleteTexture(map_right);
	w->DeleteTexture(map_left);
	w->DeleteShader(Shader_Stereo);
}

void AnaglyphsRenderer::InitAndGetLeft(int width,int height) {
	if ((c_width!=width) && (c_height!=height)) {
		w->DeleteTexture(map_right);
		w->DeleteTexture(map_left);
		map_left=w->CreateTexture(width,height,0,GL_RGBA);		// TODOSS options can be improved?
		map_right=w->CreateTexture(width,height,0,GL_RGBA);		// TODOSS options can be improved?
		this->c_width=width;
		this->c_height=height;
	}
	w->SetReadBuffer(0);
	w->GetBuffer(map_left,GL_RGBA);
}

void AnaglyphsRenderer::GetRightAndProcess() {
	w->SetReadBuffer(0);
	w->GetBuffer(map_right,GL_RGBA);

	w->SetDrawBuffer(0);
	w->SetShader(Shader_Stereo);
	w->SetUniformVariable(Shader_Stereo,"map1",0);
	w->SetUniformVariable(Shader_Stereo,"map2",1);
	UINT InTextures[]={map_left,map_right};
	w->Run2DShader(2,InTextures);
	w->SetShader(0);
}












char FragmentFilter[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"uniform sampler2D  ImageSrc;"
"uniform float		filter[MAX_FILTER_SIZE];"
"uniform float		offset_x[MAX_FILTER_SIZE];"
"uniform float		offset_y[MAX_FILTER_SIZE];"
"uniform int		n_elements;"
"void main(void) {"
"vec2 coords_cur,coords;"
"coords_cur=gl_TexCoord[0].st;"
"float val=0.0;"
"for(int i=0;i<n_elements;i++) {"
"	coords.s=coords_cur.s+offset_x[i];"
"	coords.t=coords_cur.t+offset_y[i];"
"	val+=filter[i]*(texture2D(ImageSrc,coords).r);"
"}"
"gl_FragColor=vec4(val,val,val,val);"
"}";
char FragmentNormFxFyFilter[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"uniform sampler2D  ImageSrc;"
"uniform float		filter[MAX_FILTER_SIZE];"
"uniform float		offset_x[MAX_FILTER_SIZE];"
"uniform float		offset_y[MAX_FILTER_SIZE];"
"uniform int		n_elements;"
"void main(void) {"
"vec2 coords_cur,coords_x,coords_y;"
"coords_cur=gl_TexCoord[0].st;"
"float val_x=0.0;"
"float val_y=0.0;"
"for(int i=0;i<n_elements;i++) {"
"	coords_x.s=coords_cur.s+offset_x[i];"
"	coords_x.t=coords_cur.t+offset_y[i];"
"	coords_y.s=coords_cur.s+offset_y[i];"
"	coords_y.t=coords_cur.t+offset_x[i];"
"	float vx=texture2D(ImageSrc,coords_x).r;"
"	float vy=texture2D(ImageSrc,coords_y).r;"
"	val_x+=filter[i]*vx;"
"	val_y+=filter[i]*vy;"
"}"
"float val=sqrt(val_x*val_x+val_y*val_y);"
"gl_FragData[0]=vec4(val_x,val_x,val_x,val_x);"
"gl_FragData[1]=vec4(val_y,val_y,val_y,val_y);"
"gl_FragData[2]=vec4(val,val,val,val);"
"}";
char FragmentNormFilter[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"uniform sampler2D  ImageSrc;"
"uniform float		filter[MAX_FILTER_SIZE];"
"uniform float		offset_x[MAX_FILTER_SIZE];"
"uniform float		offset_y[MAX_FILTER_SIZE];"
"uniform int		n_elements;"
"void main(void) {"
"vec2 coords_cur,coords_x,coords_y;"
"coords_cur=gl_TexCoord[0].st;"
"float val_x=0.0;"
"float val_y=0.0;"
"for(int i=0;i<n_elements;i++) {"
"	coords_x.s=coords_cur.s+offset_x[i];"
"	coords_x.t=coords_cur.t+offset_y[i];"
"	coords_y.s=coords_cur.s+offset_y[i];"
"	coords_y.t=coords_cur.t+offset_x[i];"
"	float vx=texture2D(ImageSrc,coords_x).r+texture2D(ImageSrc,coords_x).g+texture2D(ImageSrc,coords_x).b;"
"	float vy=texture2D(ImageSrc,coords_y).r+texture2D(ImageSrc,coords_y).g+texture2D(ImageSrc,coords_y).b;"
"	val_x+=filter[i]*vx;"
"	val_y+=filter[i]*vy;"
"}"
"float val=sqrt(val_x*val_x+val_y*val_y);"
"gl_FragColor=vec4(val,val,val,val);"
"}";
char FragmentAngleFilter[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"uniform sampler2D  ImageSrc;"
"uniform float		filter[MAX_FILTER_SIZE];"
"uniform float		offset_x[MAX_FILTER_SIZE];"
"uniform float		offset_y[MAX_FILTER_SIZE];"
"uniform int		n_elements;"
"void main(void) {"
"vec2 coords_cur,coords_x,coords_y;"
"coords_cur=gl_TexCoord[0].st;"
"float val_x=0.0;"
"float val_y=0.0;"
"int border=0;"
"for(int i=0;i<n_elements;i++) {"
"	coords_x.s=coords_cur.s+offset_x[i];"
"	coords_x.t=coords_cur.t+offset_y[i];"
"	coords_y.s=coords_cur.s+offset_y[i];"
"	coords_y.t=coords_cur.t+offset_x[i];"
"	float vx=texture2D(ImageSrc,coords_x).r;"
"	float vy=texture2D(ImageSrc,coords_y).r;"
"	if ((vx>1e30) || (vy>1e30)) {border=1;break;}"
"	val_x+=filter[i]*vx;"
"	val_y+=filter[i]*vy;"
"}"
"if (border==1) {"
"	val_x=0.0;"
"	val_y=0.0;"
"	for(int i=0;i<n_elements;i++) {"
"		coords_x.s=coords_cur.s+offset_x[i];"
"		coords_x.t=coords_cur.t+offset_y[i];"
"		float vx=texture2D(ImageSrc,coords_x).r;"
"		if (vx>1e30) {"
"			val_x+=offset_x[i];"
"			val_y+=offset_y[i];"
"		}"
"	}"
"}"
"float val=atan(val_y,val_x);"
"gl_FragColor=vec4(val,val,val,val);"
"}";
char FragmentMedianFilter[] = 
"#define t2(a,b)	 temp=v[a]; v[a]=min(v[a],v[b]); v[b]=max(temp,v[b]);\n"
"#define t24(a,b,c,d,e,f,g,h)			 t2(a,b) t2(c,d) t2(e,f) t2(g,h)\n"
"#define t25(a,b,c,d,e,f,g,h,i,j)		t2(a,b) t2(c,d) t2(e,f) t2(g,h) t2(i,j)\n"
"uniform sampler2D  ImageSrc;\n"
"uniform float		offset_x[25];\n"
"uniform float		offset_y[25];\n"
"void main() {\n"
"  float v[25];\n"
"  for(int i=0;i<25;i++) {\n"
"		v[i] = (texture2D(ImageSrc,gl_TexCoord[0].xy+vec2(offset_x[i],offset_y[i]))).r;\n"
"  }\n"
"  float temp;\n"
"  t25(0, 1,		3, 4,		2, 4,		2, 3,		6, 7);\n"
"  t25(5, 7,		5, 6,		9, 7,		1, 7,		1, 4);\n"
"  t25(12, 13,		11, 13,		11, 12,		15, 16,		14, 16);\n"
"  t25(14, 15,		18, 19,		17, 19,		17, 18,		21, 22);\n"
"  t25(20, 22,		20, 21,		23, 24,		2, 5,		3, 6);\n"
"  t25(0, 6,		0, 3,		4, 7,		1, 7,		1, 4);\n"
"  t25(11, 14,		8, 14,		8, 11,		12, 15,		9, 15);\n"
"  t25(9, 12,		13, 16,		10, 16,		10, 13,		20, 23);\n"
"  t25(17, 23,		17, 20,		21, 24,		18, 24,		18, 21);\n"
"  t25(19, 22,		8, 17,		9, 18,		0, 18,		0, 9);\n"
"  t25(10, 19,		1, 19,		1, 10,		11, 20,		2, 20);\n"
"  t25(2, 11,		12, 21,		3, 21,		3, 12,		13, 22);\n"
"  t25(4, 22,		4, 13,		14, 23,		5, 23,		5, 14);\n"
"  t25(15, 24,		6, 24,		6, 15,		7, 16,		7, 19);\n"
"  t25(3, 11,		5, 17,		11, 17,		9, 17,		4, 10);\n"
"  t25(6, 12,		7, 14,		4, 6,		4, 7,		12, 14);\n"
"  t25(10, 14,		6, 7,		10, 12,		6, 10,		6, 17);\n"
"  t25(12, 17,		7, 17,		7, 10,		12, 18,		7, 12);\n"
"  t24(10, 18,		12, 20,		10, 20,		10, 12);\n"
"  gl_FragColor=vec4(v[12],v[12],v[12],v[12]);\n"
"}\n";
char FragmentRGB2YCBCR[] = 
"uniform sampler2D  ImageSrc;"
"void main(void) {"
"vec3 c=texture2D(ImageSrc,gl_TexCoord[0].st).rgb;"
"float val;"
"val=((0.256788235294118*255.0*c.r)+(0.504129411764706*255.0*c.g)+(0.0979058823529412*255.0*c.b)+16.0)/255.0;"
"gl_FragData[0]=vec4(val,val,val,val);"
"val=((-0.148223529411765*255.0*c.r)-(0.290992156862745*255.0*c.g)+(0.439215686274510*255.0*c.b)+128.0)/255.0;"
"gl_FragData[1]=vec4(val,val,val,val);"
"val=((0.439215686274510*255.0*c.r)-(0.367788235294118*255.0*c.g)-(0.0714274509803922*255.0*c.b)+128.0)/255.0;"
"gl_FragData[2]=vec4(val,val,val,val);"
"}";
char Fragmentuint8RGBA2doubleLAB[] = 
"uniform sampler2D ImageSrc;\n"
"void main(void) {\n"
"	vec4 color=texture2D(ImageSrc,gl_TexCoord[0].st);\n"
"	color.rgb=((color.rgb*255.0)+1.0)/256.0;\n"
"	vec4 LMS,LAB;\n"
"	LMS.r=0.3811*color.r+0.5783*color.g+0.0402*color.b;\n"
"	LMS.g=0.1967*color.r+0.7244*color.g+0.0782*color.b;\n"
"	LMS.b=0.0241*color.r+0.1288*color.g+0.8444*color.b;\n"
"	LMS.r=log(LMS.r);\n"
"	LMS.g=log(LMS.g);\n"
"	LMS.b=log(LMS.b);\n"
"	LAB.r=0.577350*LMS.r+0.577350*LMS.g+0.577350*LMS.b;\n"
"	LAB.g=0.408248*LMS.r+0.408248*LMS.g-0.816497*LMS.b;\n"
"	LAB.b=0.707107*LMS.r-0.707107*LMS.g+     0.0*LMS.b;\n"
"	LAB.a=color.a;\n"
"	gl_FragColor=LAB;\n"
"}\n";
char FragmentdoubleLAB2uint8RGBA[] = 
"uniform sampler2D ImageSrc;\n"
"uniform float k[3];\n"
"uniform float t[3];\n"
"void main(void) {\n"
"	vec4 LAB=texture2D(ImageSrc,gl_TexCoord[0].st);\n"
"	vec4 RGB,LMS;\n"
"	LAB.r=LAB.r*k[0]+t[0];\n"
"	LAB.g=LAB.g*k[1]+t[1];\n"
"	LAB.b=LAB.b*k[2]+t[2];\n"
"	LMS.r=0.577350*LAB.r+0.408248*LAB.g+0.707107*LAB.b;\n"
"	LMS.g=0.577350*LAB.r+0.408248*LAB.g-0.707107*LAB.b;\n"
"	LMS.b=0.577350*LAB.r-0.816497*LAB.g+	 0.0*LAB.b;\n"
"	LMS.r=exp(LMS.r);\n"
"	LMS.g=exp(LMS.g);\n"
"	LMS.b=exp(LMS.b);\n"
"	RGB.r= 4.468670*LMS.r-3.588676*LMS.g+0.119604*LMS.b;\n"
"	RGB.g=-1.219717*LMS.r+2.383088*LMS.g-0.162630*LMS.b;\n"
"	RGB.b= 0.058508*LMS.r-0.261078*LMS.g+1.205666*LMS.b;\n"
"	RGB.rgb=((RGB.rgb*256.0)-1.0)/255.0;\n"
"	RGB.r=max(min(RGB.r,1.0),0.0);\n"
"	RGB.g=max(min(RGB.g,1.0),0.0);\n"
"	RGB.b=max(min(RGB.b,1.0),0.0);\n"
"	RGB.a=LAB.a;\n"
"	gl_FragColor=RGB;\n"
"}\n";
char Fragmentuint8RGBA2uint8YHHA[] = 
"uniform sampler2D ImageSrc;\n"
"vec4 RGB2YH(vec4 color) {\n"
"	// color belongs between [0,1]\n"
"	vec4 color_out;\n"
"	\n"
"	float M=max(max(color.r,color.g),color.b);\n"
"	float m=min(min(color.r,color.g),color.b);\n"
"	\n"
"	float s=M-m;														// s belongs to [0,+1]\n"
"	color_out.r=(0.2125*color.r)+(0.7154*color.g)+(0.0721*color.b);		// r belongs to [0,+1]\n"
"	\n"
"	float crx=(color.r-((color.g+color.b)/2.0));\n"
"	float cry=(sqrt(3.0)*(color.b-color.g)/2.0);\n"
"	float cr=(sqrt((crx*crx)+(cry*cry)));\n"
"	\n"
"	if (cr==0.0) {\n"
"		color_out.g=0.5;\n"
"		 color_out.b=0.5;\n"
"	} else {\n"
"		color_out.g=s*(crx/cr);					//	(g,b) norm belongs to [0,+1]\n"
"		color_out.b=-s*(cry/cr);				// -> g,b belongs to [-1,+1]\n"
"		color_out.g=(color_out.g+1.0)/2.0;		// -> g,b belongs to [0,+1]\n"
"		color_out.b=(color_out.b+1.0)/2.0;		// -> black is at [0.5,0.5]\n"
"	}\n"
"	color_out.a=color.a;\n"
"	\n"
"	return color_out;\n"
"}\n"
"void main(void) {\n"
"	vec4 color = texture2D(ImageSrc,gl_TexCoord[0].st);\n"
"	color = RGB2YH(color);\n"
"	gl_FragColor = color;\n"
"}\n";
char FragmentExpandFilter[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"uniform sampler2D  ImageSrc;"
"uniform float		offset_x[MAX_FILTER_SIZE];"
"uniform float		offset_y[MAX_FILTER_SIZE];"
"uniform int		  n_elements;"
"void main(void) {"
"vec2 coords_cur,coords;" 
"coords_cur=gl_TexCoord[0].st;"
"vec3  color=vec3(0.0,0.0,0.0);"
"float color_num=0.0;"
"for(int i=0;i<n_elements;i++) {"
"	coords.s=coords_cur.s+offset_x[i];"
"	coords.t=coords_cur.t+offset_y[i];"
"	vec4 col=texture2D(ImageSrc,coords);"
"	color=color+(col.a*col.rgb);"
"	color_num=color_num+col.a;"
"}"
"if (color_num>0.0) {"
"	vec4 col=texture2D(ImageSrc,coords_cur);"
"	if (col.a!=1.0) {"
"		color=color*(1.0/color_num);"
"		gl_FragData[0]=vec4(color.r,color.g,color.b,1.0);"
"	} else {"
"		gl_FragData[0]=vec4(col.r,col.g,col.b,1.0);"
"	}"
"} else {"
"	gl_FragData[0]=vec4(0.0,0.0,0.0,0.0);"
"}"
"}";



ImageProcessingRenderer::ImageProcessingRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_Filter=w->CreateShaderFromCode(StdVertexShader,FragmentFilter,false);
	Shader_NormFxFyFilter=w->CreateShaderFromCode(StdVertexShader,FragmentNormFxFyFilter,false);
	Shader_RGB2YCBCR=w->CreateShaderFromCode(StdVertexShader,FragmentRGB2YCBCR,false);
	Shader_uint8RGBA2uint8YHHA=w->CreateShaderFromCode(StdVertexShader,Fragmentuint8RGBA2uint8YHHA,false);
	Shader_uint8RGBA2doubleLAB=w->CreateShaderFromCode(StdVertexShader,Fragmentuint8RGBA2doubleLAB,false);
	Shader_doubleLAB2uint8RGBA=w->CreateShaderFromCode(StdVertexShader,FragmentdoubleLAB2uint8RGBA,false);
	Shader_AngleFilter=w->CreateShaderFromCode(StdVertexShader,FragmentAngleFilter,false);
	Shader_NormFilter=w->CreateShaderFromCode(StdVertexShader,FragmentNormFilter,false);
	Shader_MedianFilter=w->CreateShaderFromCode(StdVertexShader,FragmentMedianFilter,false);
	Shader_ExpandFilter=w->CreateShaderFromCode(StdVertexShader,FragmentExpandFilter,false);
}

ImageProcessingRenderer::~ImageProcessingRenderer() {
	w->DeleteShader(Shader_Filter);
	w->DeleteShader(Shader_NormFxFyFilter);
	w->DeleteShader(Shader_RGB2YCBCR);
	w->DeleteShader(Shader_uint8RGBA2uint8YHHA);
	w->DeleteShader(Shader_uint8RGBA2doubleLAB);
	w->DeleteShader(Shader_doubleLAB2uint8RGBA);
	w->DeleteShader(Shader_AngleFilter);
	w->DeleteShader(Shader_NormFilter);
	w->DeleteShader(Shader_MedianFilter);
	w->DeleteShader(Shader_ExpandFilter);
}





//
// Basic functions
//
inline void ImageProcessingRenderer::FilterStart(FrameBuffer_struct *fb,UINT shader,float *filter,int n_elements,int output_buffer_index) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(output_buffer_index);
	w->SetReadBuffer(output_buffer_index);
	SetStandardOGLEnvironment(false,false,false);
	w->SetShader(shader);
	w->SetUniformArray(shader,   "filter",n_elements,filter);
	w->SetUniformVariable(shader,"n_elements",(int)n_elements);
	w->SetUniformVariable(shader,"ImageSrc",(int)0);
}
inline void ImageProcessingRenderer::FilterSetOffset(FrameBuffer_struct *fb,UINT shader,int r,int c,int n_elements) {
	float *offset_x=new float[c*r];
	float *offset_y=new float[c*r];

	float *ox_tmp=offset_x;
	float *oy_tmp=offset_y;
	float cx=(c-1.0f)/2.0f;
	float cy=(r-1.0f)/2.0f;
	for(int i=0;i<r;i++) {
		for(int j=0;j<c;j++) {
			*(ox_tmp++)=(j-cx)/fb->w;
			*(oy_tmp++)=(i-cy)/fb->h;
		}
	}

	w->SetUniformArray(shader,   "offset_x",n_elements,offset_x);
	w->SetUniformArray(shader,   "offset_y",n_elements,offset_y);

	delete[]offset_x;
	delete[]offset_y;
}
inline void ImageProcessingRenderer::FilterEnd(FrameBuffer_struct *fb,UINT input_image,Bitmap<ColorFloat> *output) {
	w->Run2DShader(input_image);
	w->SetShader(0);
	if (output) w->RetrieveFrameBuffer(fb,output);
	w->ClearFrameBuffer(fb);
}



//
// Advanced functions
//
void ImageProcessingRenderer::FilterDefine(FrameBuffer_struct *fb,UINT Shader,Matrix *Filter,int output_buffer_index) {
	int    n_elements =Filter->c*Filter->r;
	float *filter     =new float[n_elements];
	if (n_elements>IPR_FILTER_MAX_DIMENSION) ErrorExit("Filter dimension not supported.");

	Filter->Get(filter);
	
	FilterStart(fb,Shader,filter,n_elements,output_buffer_index);
	FilterSetOffset(fb,Shader,Filter->r,Filter->c,n_elements);

	delete[]filter;
}
void ImageProcessingRenderer::FilterProcess(FrameBuffer_struct *fb,UINT input,Bitmap<ColorFloat> *output) {
	FilterEnd(fb,input,output);
}






//
// Filter Image
//
void ImageProcessingRenderer::FilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,int output_buffer_index,Bitmap<ColorFloat> *filtered) {
	FilterDefine(fb,Shader_Filter,Filter,output_buffer_index);
	FilterProcess(fb,input_image,filtered);
}
void ImageProcessingRenderer::FilterImage(FrameBuffer_struct *fb,UINT input_image,float *Filter,int filter_len,int output_buffer_index,Bitmap<ColorFloat> *filtered) {
	int    n_elements =filter_len*filter_len;
	float *filter     =new float[n_elements];
	if (n_elements>IPR_FILTER_MAX_DIMENSION) ErrorExit("Filter dimension not supported.");

	float *filter_p=filter;
	for(int i=0;i<filter_len;i++) {
		for(int j=0;j<filter_len;j++) {
			*filter_p=Filter[i]*Filter[j];
			filter_p++;
		}
	}

	FilterStart(fb,Shader_Filter,filter,n_elements,output_buffer_index);
	FilterSetOffset(fb,Shader_Filter,filter_len,filter_len,n_elements);
	delete[]filter;

	FilterProcess(fb,input_image,filtered);
}









//
// NormAngle Filter Image
//
void ImageProcessingRenderer::NormFxFyFilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,
												   int output_buffer_index_fx,
												   int output_buffer_index_fy,
												   int output_buffer_index_norm,
												   Bitmap<ColorFloat> *fx,
												   Bitmap<ColorFloat> *fy,
												   Bitmap<ColorFloat> *norm) {
	
	int DrawBuffers[3];
	DrawBuffers[0]=output_buffer_index_fx;
	DrawBuffers[1]=output_buffer_index_fy;
	DrawBuffers[2]=output_buffer_index_norm;


	FilterDefine(fb,Shader_NormFxFyFilter,Filter,0);
	w->SetDrawBuffers(3,DrawBuffers);


	w->Run2DShader(input_image);
	w->SetShader(0);
	w->SetDrawBuffer(0);
	if (fx) {
		w->SetReadBuffer(output_buffer_index_fx);
		w->RetrieveFrameBuffer(fb,fx);
	}
	if (fy) {
		w->SetReadBuffer(output_buffer_index_fy);
		w->RetrieveFrameBuffer(fb,fy);
	}
	if (norm) {
		w->SetReadBuffer(output_buffer_index_norm);
		w->RetrieveFrameBuffer(fb,norm);
	}
	w->SetReadBuffer(0);
	w->ClearFrameBuffer(fb);
}


//
// Expand Filter Image
//
void ImageProcessingRenderer::ExpandImage(FrameBuffer_struct *fb,UINT input_image,
										   int output_buffer_index_image,
										   Bitmap<ColorRGBA>  *image) {
	
	
	int r=3;int c=3;
	int n_elements=r*c;

	if (n_elements>IPR_FILTER_MAX_DIMENSION) ErrorExit("Filter dimension not supported.");

	w->SetFrameBuffer(fb);
		SetStandardOGLEnvironment(false,false,false);
		w->SetShader(Shader_ExpandFilter);
		w->SetUniformVariable(Shader_ExpandFilter,"n_elements",(int)n_elements);
		w->SetUniformVariable(Shader_ExpandFilter,"ImageSrc",(int)0);
		FilterSetOffset(fb,Shader_ExpandFilter,r,c,n_elements);

		w->SetDrawBuffers(output_buffer_index_image);
		w->Run2DShader(input_image);

		w->SetShader(0);
		w->SetDrawBuffer(0);
		if (image) {
			w->SetReadBuffer(output_buffer_index_image);
			w->RetrieveFrameBuffer(fb,image);
		}
		w->SetReadBuffer(0);
	w->ClearFrameBuffer(fb);
}


//
// Color Coversion
//
void ImageProcessingRenderer::RGBtoYCbCr(FrameBuffer_struct *fb,UINT input_image,
										int output_buffer_index_Y,
										int output_buffer_index_Cb,
										int output_buffer_index_Cr,
										Bitmap<ColorFloat> *Y,
										Bitmap<ColorFloat> *Cb,
										Bitmap<ColorFloat> *Cr) {

	int DrawBuffers[3];
	DrawBuffers[0]=output_buffer_index_Y;
	DrawBuffers[1]=output_buffer_index_Cb;
	DrawBuffers[2]=output_buffer_index_Cr;


	w->SetFrameBuffer(fb);
	SetStandardOGLEnvironment(false,false,false);
	w->SetShader(Shader_RGB2YCBCR);
	w->SetUniformVariable(Shader_RGB2YCBCR,"ImageSrc",(int)0);

	w->SetDrawBuffers(3,DrawBuffers);


	w->Run2DShader(input_image);
	w->SetShader(0);
	w->SetDrawBuffer(0);
	if (Y) {
		w->SetReadBuffer(output_buffer_index_Y);
		w->RetrieveFrameBuffer(fb,Y);
	}
	if (Cb) {
		w->SetReadBuffer(output_buffer_index_Cb);
		w->RetrieveFrameBuffer(fb,Cb);
	}
	if (Cr) {
		w->SetReadBuffer(output_buffer_index_Cr);
		w->RetrieveFrameBuffer(fb,Cr);
	}
	w->SetReadBuffer(0);
	w->ClearFrameBuffer(fb);
}


void ImageProcessingRenderer::RGBA2YHHA(FrameBuffer_struct *fb,UINT input_image,Bitmap<ColorRGB> *out) {
	w->SetFrameBuffer(fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
			w->SetShader(Shader_uint8RGBA2uint8YHHA);
			w->SetUniformVariable(Shader_uint8RGBA2uint8YHHA,"ImageSrc",(int)0);
			w->Run2DShader(input_image);
			w->SetShader(0);

			if (out) w->RetrieveFrameBuffer(fb,out);
		glDepthMask(1);
	w->ClearFrameBuffer(fb);
}


void image_stat(Bitmap<ColorFloat> *img,double &mean,double &std) {
	mean_std_data A;
	int img_size=img->width*img->height;

	ColorFloat *c=img->getBuffer();
	for(int i=0;i<img_size;i++,c++) {
		mean_std_i(*c,&A);
	}
	mean_std_r(&A,mean,std);
}

void ImageProcessingRenderer::ColorTransfer_Step1(FrameBuffer_struct *fb,UINT input_image,int width,int height,Vector<3> *m,Vector<3> *s) {
	w->SetFrameBuffer(fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
			w->SetShader(Shader_uint8RGBA2doubleLAB);
			w->SetUniformVariable(Shader_uint8RGBA2doubleLAB,"ImageSrc",(int)0);
			w->Run2DShader(input_image);
			w->SetShader(0);

			Bitmap<ColorFloat> out(width,height);
			w->RetrieveFrameBuffer(fb,&out,GL_RED);
			image_stat(&out,(*m)[0],(*s)[0]);
			w->RetrieveFrameBuffer(fb,&out,GL_GREEN);
			image_stat(&out,(*m)[1],(*s)[1]);
			w->RetrieveFrameBuffer(fb,&out,GL_BLUE);
			image_stat(&out,(*m)[2],(*s)[2]);

		glDepthMask(1);
	w->ClearFrameBuffer(fb);
}

void ImageProcessingRenderer::ColorTransfer_Step2(FrameBuffer_struct *fb,UINT fb1_texture,Vector<3> m_s,Vector<3> d_s,Vector<3> m_t,Vector<3> d_t,Bitmap<ColorRGB> *out) {
	Vector<3> k;
	Vector<3> t;
	k[0]=(d_t[0]/d_s[0]);
	k[1]=(d_t[1]/d_s[1]);
	k[2]=(d_t[2]/d_s[2]);
	t[0]=(m_t[0]-(m_s[0]*k[0]));
	t[1]=(m_t[1]-(m_s[1]*k[1]));
	t[2]=(m_t[2]-(m_s[2]*k[2]));


	w->SetFrameBuffer(fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
		SetStandardOGLEnvironment(false,false,false);
		glDepthMask(0);
		
			w->SetShader(Shader_doubleLAB2uint8RGBA);
			w->SetUniformVariable(Shader_doubleLAB2uint8RGBA,"ImageSrc",(int)0);
			float tmp[3];
			k.GetList(tmp);
			w->SetUniformArray(Shader_doubleLAB2uint8RGBA,"k",3,tmp);
			t.GetList(tmp);
			w->SetUniformArray(Shader_doubleLAB2uint8RGBA,"t",3,tmp);
			w->Run2DShader(fb1_texture);
			w->SetShader(0);

			if (out) w->RetrieveFrameBuffer(fb,out);
		glDepthMask(1);
	w->ClearFrameBuffer(fb);
}













// TODO***
void ImageProcessingRenderer::NormFilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,Bitmap<ColorFloat> *norm) {
	int    n_elements =Filter->c*Filter->r;
	float *filter     =new float[n_elements];
	if (n_elements>IPR_FILTER_MAX_DIMENSION) ErrorExit("Filter dimension not supported.");

	Filter->Get(filter);

	FilterStart(fb,Shader_NormFilter,filter,n_elements,0);
	FilterSetOffset(fb,Shader_NormFilter,Filter->r,Filter->c,n_elements);
	FilterEnd(fb,input_image,norm);

	delete[]filter;
}
void ImageProcessingRenderer::AngleFilterImage(FrameBuffer_struct *fb,UINT input_image,Matrix *Filter,Bitmap<ColorFloat> *angles) {
	int    n_elements =Filter->c*Filter->r;
	float *filter     =new float[n_elements];
	if (n_elements>IPR_FILTER_MAX_DIMENSION) ErrorExit("Filter dimension not supported.");

	Filter->Get(filter);
	
	FilterStart(fb,Shader_AngleFilter,filter,n_elements,0);
	FilterSetOffset(fb,Shader_AngleFilter,Filter->r,Filter->c,n_elements);
	FilterEnd(fb,input_image,angles);

	delete[]filter;
}
void ImageProcessingRenderer::MedianFilterImage(FrameBuffer_struct *fb,UINT input_image,Bitmap<ColorFloat> *output) {

	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	SetStandardOGLEnvironment(false,false,false);
	w->SetShader(Shader_MedianFilter);
	w->SetUniformVariable(Shader_MedianFilter,"ImageSrc",(int)0);
	
	FilterSetOffset(fb,Shader_MedianFilter,5,5,25);
	FilterEnd(fb,input_image,output);

}
FrameBuffer_struct ImageProcessingRenderer::BuildFloatFrameBuffer(int width,int height) {
	UINT Float_Map=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&Float_Map,1,TEXTURE_CH_32,0,false);

	return fb;
}

FrameBuffer_struct ImageProcessingRenderer::Build3byteFrameBuffer(int width,int height,int default_buffer_options) {
	UINT texture=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&texture,1,TEXTURE_CH_RGBA,default_buffer_options,false);

	return fb;
}

FrameBuffer_struct ImageProcessingRenderer::Build3floatFrameBuffer(int width,int height,int default_buffer_options) {
	UINT texture=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&texture,1,TEXTURE_CH_32_32_32_32,default_buffer_options,false);

	return fb;
}















char Fragment_DepthEdgeFilter[] = 
"#define FLT_MAX    3.402823466e+38\n"
"uniform sampler2D  ImageSrc;\n"
"uniform float      offset_x;\n"
"uniform float      offset_y;\n"
"void main(void) {\n"
"	float val_x,val_y;\n"
"	float vf,vb;\n"
"	vf=texture2D(ImageSrc,gl_TexCoord[0].st+vec2( offset_x,0)).r;\n"
"	vb=texture2D(ImageSrc,gl_TexCoord[0].st+vec2(-offset_x,0)).r;\n"
"	val_x=0.5*vf-0.5*vb;\n"
"	if ((vf==FLT_MAX)^^(vb==FLT_MAX)) {gl_FragColor=vec4(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);return;}\n"
"	vf=texture2D(ImageSrc,gl_TexCoord[0].st+vec2(0, offset_y)).r;\n"
"	vb=texture2D(ImageSrc,gl_TexCoord[0].st+vec2(0,-offset_y)).r;\n"
"	val_y=0.5*vf-0.5*vb;\n"
"	if ((vf==FLT_MAX)^^(vb==FLT_MAX)) {gl_FragColor=vec4(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);return;}\n"
"	float val=sqrt(val_x*val_x+val_y*val_y);\n"
"	gl_FragColor=vec4(val,val,val,val);\n"
"}\n";

DepthEdgesDetectorRenderer::DepthEdgesDetectorRenderer(AdvancedRenderWindow *w) {
	this->w=w;
	Shader_DepthEdgeFilter=w->CreateShaderFromCode(StdVertexShader,Fragment_DepthEdgeFilter,false);
}
DepthEdgesDetectorRenderer::~DepthEdgesDetectorRenderer() {
	w->DeleteShader(Shader_DepthEdgeFilter);
}
void DepthEdgesDetectorRenderer::FilterDepth(FrameBuffer_struct *fb,UINT depth_map,Bitmap<ColorFloat> *output) {
	w->SetFrameBuffer(fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	SetStandardOGLEnvironment(false,false,false);
	w->SetShader(Shader_DepthEdgeFilter);
	w->SetUniformVariable(Shader_DepthEdgeFilter,"ImageSrc",(int)0);
	w->SetUniformVariable(Shader_DepthEdgeFilter,"offset_x",(double)(1.0/fb->w));
	w->SetUniformVariable(Shader_DepthEdgeFilter,"offset_y",(double)(1.0/fb->h));
	
	w->Run2DShader(depth_map);
	w->SetShader(0);
	if (output) w->RetrieveFrameBuffer(fb,output);
	w->ClearFrameBuffer(fb);
}
FrameBuffer_struct DepthEdgesDetectorRenderer::BuildFloatFrameBuffer(int width,int height) {
	UINT Float_Map=NULL;
	FrameBuffer_struct fb=w->CreateFrameBuffer(width,height,&Float_Map,1,TEXTURE_CH_32,0,false);

	return fb;
}













char L1Norm_VertexShader[] = 
"void main( void ) {\n"
"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"gl_Position = ftransform();\n"
"}\n";
char L1Norm_FragmentShader[] = 
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 pos_r=gl_TexCoord[0].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos=pos_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		vec4 b=texture2D(map2,pos);\n"
"		sum = sum + abs(a.r-b.r);\n" 
"		pos.s = pos.s + offset_x;\n"
"	}\n"
"   pos_r.t=pos_r.t+offset_y;\n"
"}\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";
char L1Norm3_FragmentShader[] = 
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 pos_r=gl_TexCoord[0].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos=pos_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		vec4 b=texture2D(map2,pos);\n"
"		sum = sum + (abs(a.r-b.r) + abs(a.g-b.g) + abs(a.b-b.b))/3.0;\n" 
"		pos.s = pos.s + offset_x;\n"
"	}\n"
"   pos_r.t=pos_r.t+offset_y;\n"
"}\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";
char L1Norm2_FragmentShader[] = 
"uniform sampler2D map1;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 pos_r=gl_TexCoord[0].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos=pos_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		sum = sum + a.r;\n" 
"		pos.s = pos.s + offset_x;\n"
"	}\n"
"   pos_r.t=pos_r.t+offset_y;\n"
"}\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";


L1NormRenderer::L1NormRenderer(AdvancedRenderWindow *w,int width,int height,int max_xy_division) {
	this->w=w;

	div_x=2;
	while (true) {
		int r_w=width/div_x;
		if (r_w==0) break;
		if (r_w*div_x!=width) break;
		div_x*=2;
		if (div_x>max_xy_division) break;
	}
	div_x/=2;
	div_y=2;
	while (true) {
		int r_h=height/div_y;
		if (r_h==0) break;
		if (r_h*div_y!=height) break;
		div_y*=2;
		if (div_y>max_xy_division) break;
	}
	div_y/=2;


	this->offset_x=(1.0/width);
	this->offset_y=(1.0/height);

	this->width=width/div_x;
	this->height=height/div_y;
	
	UINT texture=NULL;
	fb=w->CreateFrameBuffer(this->width,this->height,&texture,1,TEXTURE_CH_32,TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);

	Shader_L1Norm=w->CreateShaderFromCode(L1Norm_VertexShader,L1Norm_FragmentShader,false);
	Shader_L1Norm2=w->CreateShaderFromCode(L1Norm_VertexShader,L1Norm2_FragmentShader,false);
	Shader_L1Norm3=w->CreateShaderFromCode(L1Norm_VertexShader,L1Norm3_FragmentShader,false);
	out=new Bitmap<ColorFloat>(this->width,this->height);
}

L1NormRenderer::~L1NormRenderer() {
	w->DeleteShader(Shader_L1Norm);
	w->DeleteShader(Shader_L1Norm2);
	w->DeleteShader(Shader_L1Norm3);
	w->DeleteFrameBuffer(&fb);
	delete out;
}

float L1NormRenderer::L1Norm(UINT tex1,UINT tex2) {
	w->SetFrameBuffer(&fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_L1Norm);
	w->SetUniformVariable(Shader_L1Norm,"map1",0);
	w->SetUniformVariable(Shader_L1Norm,"map2",1);
	w->SetUniformVariable(Shader_L1Norm,"div_x",div_x);
	w->SetUniformVariable(Shader_L1Norm,"div_y",div_y);
	w->SetUniformVariable(Shader_L1Norm,"offset_x",(double)offset_x);
	w->SetUniformVariable(Shader_L1Norm,"offset_y",(double)offset_y);
	UINT InTextures[]={tex1,tex2};
	w->Run2DShader(2,InTextures);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb);

	float *x=out->getBuffer();
	float sum=0.0;
	for(int i=0;i<out->width*out->height;i++,x++) {
		sum+=(*x);
	}

	return sum;
}

float L1NormRenderer::L1Norm3C(UINT tex1,UINT tex2) {
	w->SetFrameBuffer(&fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_L1Norm3);
	w->SetUniformVariable(Shader_L1Norm3,"map1",0);
	w->SetUniformVariable(Shader_L1Norm3,"map2",1);
	w->SetUniformVariable(Shader_L1Norm3,"div_x",div_x);
	w->SetUniformVariable(Shader_L1Norm3,"div_y",div_y);
	w->SetUniformVariable(Shader_L1Norm3,"offset_x",(double)offset_x);
	w->SetUniformVariable(Shader_L1Norm3,"offset_y",(double)offset_y);
	UINT InTextures[]={tex1,tex2};
	w->Run2DShader(2,InTextures);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb);

	float *x=out->getBuffer();
	float sum=0.0;
	for(int i=0;i<out->width*out->height;i++,x++) {
		sum+=(*x);
	}

	return sum;
}

float L1NormRenderer::L1Norm(UINT tex1) {
	w->SetFrameBuffer(&fb);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_L1Norm2);
	w->SetUniformVariable(Shader_L1Norm2,"map1",0);
	w->SetUniformVariable(Shader_L1Norm2,"div_x",div_x);
	w->SetUniformVariable(Shader_L1Norm2,"div_y",div_y);
	w->SetUniformVariable(Shader_L1Norm2,"offset_x",(double)offset_x);
	w->SetUniformVariable(Shader_L1Norm2,"offset_y",(double)offset_y);
	UINT InTextures[]={tex1};
	w->Run2DShader(1,InTextures);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb);

	float *x=out->getBuffer();
	float sum=0.0;
	for(int i=0;i<out->width*out->height;i++,x++) {
		sum+=(*x);
	}

	return sum;
}










char RL1Norm_VertexShader[] = 
"void main( void ) {\n"
"gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"gl_TexCoord[1] = gl_MultiTexCoord1;\n"
"gl_Position = ftransform();\n"
"}\n";
char RL1Norm_FragmentShader[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"#define FLT_MAX    3.402823466e+38\n"
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"uniform float     offset2_x;\n"
"uniform float     offset2_y;\n"
"uniform float     offset_x_2[MAX_FILTER_SIZE];"
"uniform float     offset_y_2[MAX_FILTER_SIZE];"
"uniform int       n_offsets;"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 posv;\n"
"vec2 pos_r =gl_TexCoord[0].st;\n"
"vec2 pos2_r=gl_TexCoord[1].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"pos2_r.s=pos2_r.s-offset2_x*float(div_x/2)+(offset2_x/2.0);\n"
"pos2_r.t=pos2_r.t-offset2_y*float(div_y/2)+(offset2_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos =pos_r;\n"
"	vec2 pos2=pos2_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		float minimum=FLT_MAX;\n"
"		for(int s=0;s<n_offsets;s++) {\n"
"			posv.s=pos2.s+offset_x_2[s];\n"
"			for(int q=0;q<n_offsets;q++) {\n"
"				posv.t=pos2.t+offset_y_2[q];\n"
"				vec4 b=texture2D(map2,posv);\n"
"				float val=abs(a.r-b.r);\n"
"				minimum=min(minimum,val);\n"
"			}\n"
"		}\n"
"		sum = sum + minimum;\n" 
"		pos.s  = pos.s  + offset_x;\n"
"		pos2.s = pos2.s + offset2_x;\n"
"	}\n"
"   pos_r.t =pos_r.t +offset_y;\n"
"   pos2_r.t=pos2_r.t+offset2_y;\n"
"}\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";
char RL1Norm_FragmentShader3C[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"#define FLT_MAX    3.402823466e+38\n"
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform int       div_x;\n"
"uniform int       div_y;\n"
"uniform float     offset_x;\n"
"uniform float     offset_y;\n"
"uniform float     offset2_x;\n"
"uniform float     offset2_y;\n"
"uniform float     offset_x_2[MAX_FILTER_SIZE];"
"uniform float     offset_y_2[MAX_FILTER_SIZE];"
"uniform int       n_offsets;"
"void main(void) {\n"
"float sum=0.0;\n"
"vec2 posv;\n"
"vec2 pos_r =gl_TexCoord[0].st;\n"
"vec2 pos2_r=gl_TexCoord[1].st;\n"
"pos_r.s=pos_r.s-offset_x*float(div_x/2)+(offset_x/2.0);\n"
"pos_r.t=pos_r.t-offset_y*float(div_y/2)+(offset_y/2.0);\n"
"pos2_r.s=pos2_r.s-offset2_x*float(div_x/2)+(offset2_x/2.0);\n"
"pos2_r.t=pos2_r.t-offset2_y*float(div_y/2)+(offset2_y/2.0);\n"
"for(int i=0;i<div_y;i++) {\n"
"	vec2 pos =pos_r;\n"
"	vec2 pos2=pos2_r;\n"
"	for(int j=0;j<div_x;j++) {\n"
"		vec4 a=texture2D(map1,pos);\n"
"		float minimum=FLT_MAX;\n"
"		for(int s=0;s<n_offsets;s++) {\n"
"			posv.s=pos2.s+offset_x_2[s];\n"
"			for(int q=0;q<n_offsets;q++) {\n"
"				posv.t=pos2.t+offset_y_2[q];\n"
"				vec4 b=texture2D(map2,posv);\n"
"				float val=abs(a.r-b.r)+abs(a.g-b.g)+abs(a.b-b.b);\n"
"				minimum=min(minimum,val);\n"
"			}\n"
"		}\n"
"		sum = sum + minimum;\n" 
"		pos.s  = pos.s  + offset_x;\n"
"		pos2.s = pos2.s + offset2_x;\n"
"	}\n"
"   pos_r.t =pos_r.t +offset_y;\n"
"   pos2_r.t=pos2_r.t+offset2_y;\n"
"}\n"
"gl_FragColor=vec4(sum,sum,sum,sum);\n"
"}";
char RDiff_FragmentShader3C[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"#define FLT_MAX    3.402823466e+38\n"
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform float     offset_x_2[MAX_FILTER_SIZE];"
"uniform float     offset_y_2[MAX_FILTER_SIZE];"
"uniform int       n_offsets;"
"void main(void) {\n"
"	vec2  posv;\n"
"	float minimum=FLT_MAX;\n"
"	vec2  pos1=gl_TexCoord[0].st;\n"
"	vec2  pos2=gl_TexCoord[1].st;\n"
"	vec4 a=texture2D(map1,pos1);\n"
"	for(int s=0;s<n_offsets;s++) {\n"
"		posv.s=pos2.s+offset_x_2[s];\n"
"		for(int q=0;q<n_offsets;q++) {\n"
"			posv.t=pos2.t+offset_y_2[q];\n"
"			vec4 b=texture2D(map2,posv);\n"
"			float val=abs(a.r-b.r)+abs(a.g-b.g)+abs(a.b-b.b);\n"
"			minimum=min(minimum,val);\n"
"		}\n"
"	}\n"
"	gl_FragColor=vec4(minimum,minimum,minimum,minimum);\n"
"}";
char RFlow_FragmentShader3C[] = 
"#define MAX_FILTER_SIZE " QUOTEOBJ(IPR_FILTER_MAX_DIMENSION) " \n"
"#define FLT_MAX    3.402823466e+38\n"
"uniform sampler2D map1;\n"
"uniform sampler2D map2;\n"
"uniform float     offset_x_2[MAX_FILTER_SIZE];\n"
"uniform float     offset_y_2[MAX_FILTER_SIZE];\n"
"uniform int       n_offsets;\n"
"uniform float     max_flow;\n"
"uniform float     sigma_diff;\n"
"void main(void) {\n"
"	vec2  posv;\n"
"	float min_diff=FLT_MAX;\n"
"   int   flow_s,flow_q;\n"
"	vec2  pos1=gl_TexCoord[0].st;\n"
"	vec2  pos2=gl_TexCoord[1].st;\n"
"	vec4 a=texture2D(map1,pos1);\n"
"	for(int s=0;s<n_offsets;s++) {\n"
"		posv.s=pos2.s+offset_x_2[s];\n"
"		for(int q=0;q<n_offsets;q++) {\n"
"			posv.t=pos2.t+offset_y_2[q];\n"
"			vec4 b=texture2D(map2,posv);\n"
"			float val=abs(a.r-b.r)+abs(a.g-b.g)+abs(a.b-b.b);\n"
"			if (val<min_diff) {\n"
"				min_diff=val;\n"
"				flow_s=s;\n"
"				flow_q=q;\n"
"			}\n"
"		}\n"
"	}\n"
"	flow=sqrt((offset_x_2[flow_s]*offset_x_2[flow_s])+(offset_y_2[flow_q]*offset_y_2[flow_q]));\n"
"   flow=flow/max_flow;\n"
"   min_diff=min_diff/sigma_diff;\n"
"   float score=1.0-(clamp(1.0-flow,0.0,1.0)*clamp(1.0-min_diff,0.0,1.0));\n"
"	gl_FragColor=vec4(score,score,score,score);\n"
"}";



RobustL1NormRenderer::RobustL1NormRenderer(AdvancedRenderWindow *w,int width,int height,int max_xy_division) {
	this->w=w;
	this->width_patch=width;
	this->height_patch=height;
	this->max_xy_division=max_xy_division;

	div_x=2;
	while (true) {
		int r_w=width/div_x;
		if (r_w==0) break;
		if (r_w*div_x!=width) break;
		div_x*=2;
		if (div_x>max_xy_division) break;
	}
	div_x/=2;
	div_y=2;
	while (true) {
		int r_h=height/div_y;
		if (r_h==0) break;
		if (r_h*div_y!=height) break;
		div_y*=2;
		if (div_y>max_xy_division) break;
	}
	div_y/=2;

	this->width=width/div_x;
	this->height=height/div_y;
	
	fb1Pass.valid=false;
	fb2Pass.valid=false;
	Shader_RL1Norm=0;
	Shader_RL1Norm3C=0;
	Shader_RDiff3C=0;
	Shader_RFlow3C=0;
	L1Norm=NULL;
	out=NULL;
	offset_x_2=NULL;
	offset_y_2=NULL;
	old_c=-1;
	old_width_2=-1;
	old_height_2=-1;
}

inline void RobustL1NormRenderer::Init1Pass() {
	if (fb1Pass.valid==false) {
		UINT texture=NULL;
		fb1Pass=w->CreateFrameBuffer(this->width,this->height,&texture,1,TEXTURE_CH_32,TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);
	
		Shader_RL1Norm=w->CreateShaderFromCode(RL1Norm_VertexShader,RL1Norm_FragmentShader,false);
		Shader_RL1Norm3C=w->CreateShaderFromCode(RL1Norm_VertexShader,RL1Norm_FragmentShader3C,false);
		out=new Bitmap<ColorFloat>(this->width,this->height);
	}
}

inline void RobustL1NormRenderer::Init2Pass() {
	if (fb2Pass.valid==false) {
		UINT texture=NULL;
		fb2Pass=w->CreateFrameBuffer(this->width_patch,this->height_patch,&texture,1,TEXTURE_CH_32,TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);
	}	
	if (Shader_RDiff3C) Shader_RDiff3C=w->CreateShaderFromCode(RL1Norm_VertexShader,RDiff_FragmentShader3C,false);
	if (L1Norm==NULL)   L1Norm=new L1NormRenderer(w,this->width_patch,this->height_patch,max_xy_division);
}

inline void RobustL1NormRenderer::Init2Flow() {
	if (fb2Pass.valid==false) {
		UINT texture=NULL;
		fb2Pass=w->CreateFrameBuffer(this->width_patch,this->height_patch,&texture,1,TEXTURE_CH_32,TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,false);
	}	
	if (Shader_RFlow3C) Shader_RFlow3C=w->CreateShaderFromCode(RL1Norm_VertexShader,RFlow_FragmentShader3C,false);
	if (L1Norm==NULL)   L1Norm=new L1NormRenderer(w,this->width_patch,this->height_patch,max_xy_division);
}

inline void RobustL1NormRenderer::InitFilter(UINT sh,int max_offset,int width_2,int height_2) {
	int c,r;
	c=2*max_offset+1;
	if (c>IPR_FILTER_MAX_DIMENSION) {
		c=IPR_FILTER_MAX_DIMENSION;
		Warning("max_offset too big: max +-84 pixels"); 
	}
	r=c;

	bool rebuilt=false;
	if (old_c!=c) {
		SDELETEA(offset_x_2);
		SDELETEA(offset_y_2);
		offset_x_2=new float[c];
		offset_y_2=new float[r];
		old_c=c;
		rebuilt=true;
	}

	if ((rebuilt) || (old_width_2!=width_2) || (old_height_2!=height_2)) {
		float *ox_tmp=offset_x_2;
		float *oy_tmp=offset_y_2;
		float cx=(c-1.0f)/2.0f;
		float cy=(r-1.0f)/2.0f;

		for(int j=0;j<c;j++) *(ox_tmp++)=(j-cx)/(width_2);
		for(int i=0;i<r;i++) *(oy_tmp++)=(i-cy)/(height_2);

		// max with possible numerical errors
		max_flow=sqrt((offset_x_2[0]*offset_x_2[0])+(offset_y_2[0]*offset_y_2[0]));
	}

	w->SetUniformArray(sh,"offset_x_2",c,offset_x_2);
	w->SetUniformArray(sh,"offset_y_2",r,offset_y_2);
	w->SetUniformVariable(sh,"n_offsets",c);
}

RobustL1NormRenderer::~RobustL1NormRenderer() {
	w->DeleteShader(Shader_RL1Norm);
	w->DeleteShader(Shader_RL1Norm3C);
	w->DeleteShader(Shader_RDiff3C);
	w->DeleteShader(Shader_RFlow3C);
	w->DeleteFrameBuffer(&fb1Pass);
	w->DeleteFrameBuffer(&fb2Pass);
	SDELETE(L1Norm);
	SDELETE(out);
	SDELETEA(offset_x_2);
	SDELETEA(offset_y_2);
}

float RobustL1NormRenderer::RobustL1Norm1C(UINT tex1,UINT tex2,ROI *roi,int max_offset) {
	Init1Pass();

	w->SetFrameBuffer(&fb1Pass);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_RL1Norm);
	w->SetUniformVariable(Shader_RL1Norm,"map1",0);
	w->SetUniformVariable(Shader_RL1Norm,"map2",1);
	w->SetUniformVariable(Shader_RL1Norm,"div_x",div_x);
	w->SetUniformVariable(Shader_RL1Norm,"div_y",div_y);
	w->SetUniformVariable(Shader_RL1Norm,"offset_x",(double)(1.0/roi->width_img_1));
	w->SetUniformVariable(Shader_RL1Norm,"offset_y",(double)(1.0/roi->height_img_1));
	w->SetUniformVariable(Shader_RL1Norm,"offset2_x",(double)(1.0/roi->width_img_2));
	w->SetUniformVariable(Shader_RL1Norm,"offset2_y",(double)(1.0/roi->height_img_2));
	InitFilter(Shader_RL1Norm,max_offset,roi->width_img_2,roi->height_img_2);

	UINT InTextures[]={tex1,tex2};
	Vector<2> uv_tl[2],uv_br[2];
	uv_tl[0]=VECTOR(roi->x_1*1.0/roi->width_img_1,roi->y_1*1.0/roi->height_img_1);
	uv_br[0]=VECTOR((roi->x_1+width_patch)*1.0/roi->width_img_1,(roi->y_1+height_patch)*1.0/roi->height_img_1);
	uv_tl[1]=VECTOR(roi->x_2*1.0/roi->width_img_2,roi->y_2*1.0/roi->height_img_2);
	uv_br[1]=VECTOR((roi->x_2+width_patch)*1.0/roi->width_img_2,(roi->y_2+height_patch)*1.0/roi->height_img_2);
	w->Run2DShader(2,InTextures,uv_tl,uv_br);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb1Pass,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb1Pass);

	float *m=out->getBuffer();
	float sum=0.0;
	for(int i=0;i<out->width*out->height;i++,m++) {
		sum+=(*m);
	}

	return sum;
}
float RobustL1NormRenderer::RobustL1Norm3C(UINT tex1,UINT tex2,ROI *roi,int max_offset) {
	Init1Pass();

	w->SetFrameBuffer(&fb1Pass);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_RL1Norm3C);
	w->SetUniformVariable(Shader_RL1Norm3C,"map1",0);
	w->SetUniformVariable(Shader_RL1Norm3C,"map2",1);
	w->SetUniformVariable(Shader_RL1Norm3C,"div_x",div_x);
	w->SetUniformVariable(Shader_RL1Norm3C,"div_y",div_y);
	w->SetUniformVariable(Shader_RL1Norm,"offset_x",(double)(1.0/roi->width_img_1));
	w->SetUniformVariable(Shader_RL1Norm,"offset_y",(double)(1.0/roi->height_img_1));
	w->SetUniformVariable(Shader_RL1Norm,"offset2_x",(double)(1.0/roi->width_img_2));
	w->SetUniformVariable(Shader_RL1Norm,"offset2_y",(double)(1.0/roi->height_img_2));
	InitFilter(Shader_RL1Norm3C,max_offset,roi->width_img_2,roi->height_img_2);

	UINT InTextures[]={tex1,tex2};
	Vector<2> uv_tl[2],uv_br[2];
	uv_tl[0]=VECTOR(roi->x_1*1.0/roi->width_img_1,roi->y_1*1.0/roi->height_img_1);
	uv_br[0]=VECTOR((roi->x_1+width_patch)*1.0/roi->width_img_1,(roi->y_1+height_patch)*1.0/roi->height_img_1);
	uv_tl[1]=VECTOR(roi->x_2*1.0/roi->width_img_2,roi->y_2*1.0/roi->height_img_2);
	uv_br[1]=VECTOR((roi->x_2+width_patch)*1.0/roi->width_img_2,(roi->y_2+height_patch)*1.0/roi->height_img_2);
	w->Run2DShader(2,InTextures,uv_tl,uv_br);
	w->SetShader(0);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	w->RetrieveFrameBuffer(&fb1Pass,out);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	w->ClearFrameBuffer(&fb1Pass);

	float *m=out->getBuffer();
	float sum=0.0;
	for(int i=0;i<out->width*out->height;i++,m++) {
		sum+=(*m);
	}
	
	return sum;
}
float RobustL1NormRenderer::RobustL1Norm3C_2Pass(UINT tex1,UINT tex2,ROI *roi,int max_offset) {
	Init2Pass();

	w->SetFrameBuffer(&fb2Pass);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_RDiff3C);
	w->SetUniformVariable(Shader_RDiff3C,"map1",0);
	w->SetUniformVariable(Shader_RDiff3C,"map2",1);
	InitFilter(Shader_RDiff3C,max_offset,roi->width_img_2,roi->height_img_2);

	UINT InTextures[]={tex1,tex2};
	Vector<2> uv_tl[2],uv_br[2];
	uv_tl[0]=VECTOR(roi->x_1*1.0/roi->width_img_1,roi->y_1*1.0/roi->height_img_1);
	uv_br[0]=VECTOR((roi->x_1+width_patch)*1.0/roi->width_img_1,(roi->y_1+height_patch)*1.0/roi->height_img_1);
	uv_tl[1]=VECTOR(roi->x_2*1.0/roi->width_img_2,roi->y_2*1.0/roi->height_img_2);
	uv_br[1]=VECTOR((roi->x_2+width_patch)*1.0/roi->width_img_2,(roi->y_2+height_patch)*1.0/roi->height_img_2);
	w->Run2DShader(2,InTextures,uv_tl,uv_br);
	w->SetShader(0);

	w->ClearFrameBuffer(&fb2Pass);

	return L1Norm->L1Norm(fb2Pass.fbt);
}
float RobustL1NormRenderer::RobustFlow3C_2Pass(UINT tex1,UINT tex2,ROI *roi,int max_offset,float sigma_diff) {
	Init2Flow();
	
	// Init var
	sigma_diff=3.0f*sigma_diff;

	w->SetFrameBuffer(&fb2Pass);
	w->SetReadBuffer(0);
	w->SetDrawBuffer(0);
	SetStandardOGLEnvironment(false,false,false);

	w->SetShader(Shader_RFlow3C);
	w->SetUniformVariable(Shader_RFlow3C,"map1",0);
	w->SetUniformVariable(Shader_RFlow3C,"map2",1);
	w->SetUniformVariable(Shader_RFlow3C,"sigma_diff",sigma_diff);
	
	// compute max_flow
	InitFilter(Shader_RFlow3C,max_offset,roi->width_img_2,roi->height_img_2);
	w->SetUniformVariable(Shader_RFlow3C,"max_flow",max_flow);    // NB: before it was 9.9 which is impossible


	UINT InTextures[]={tex1,tex2};
	Vector<2> uv_tl[2],uv_br[2];
	uv_tl[0]=VECTOR(roi->x_1*1.0/roi->width_img_1,roi->y_1*1.0/roi->height_img_1);
	uv_br[0]=VECTOR((roi->x_1+width_patch)*1.0/roi->width_img_1,(roi->y_1+height_patch)*1.0/roi->height_img_1);
	uv_tl[1]=VECTOR(roi->x_2*1.0/roi->width_img_2,roi->y_2*1.0/roi->height_img_2);
	uv_br[1]=VECTOR((roi->x_2+width_patch)*1.0/roi->width_img_2,(roi->y_2+height_patch)*1.0/roi->height_img_2);
	w->Run2DShader(2,InTextures,uv_tl,uv_br);
	w->SetShader(0);

	w->ClearFrameBuffer(&fb2Pass);

	return L1Norm->L1Norm(fb2Pass.fbt);
}
























PatchMatch::PatchMatch(int n_particles,bool morph,AdvancedRenderWindow *w,UINT texture_1,UINT texture_2,int width,int height,int width_e,int height_e,HyperMesh<3> *M) : PSO_Solver((morph?((int)(2.0*(std::sqrt((double)M->num_p)-2)*(std::sqrt((double)M->num_p)-2))):5),n_particles) {
	this->w=w;
	this->M=M;
	this->texture_1=texture_1;
	this->texture_2=texture_2;
	this->width=width;
	this->height=height;
	this->use_morph=morph;
	this->save_particle_images=false;
	this->outputname=NULL;

	UINT fbt=NULL;	
	fb=w->CreateFrameBuffer(width,height,&fbt,1,TEXTURE_CH_RGBA,TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y);
	Mp=new HyperMesh<3>(M);
	Mp->Texture_GLMap[0]=texture_1;
	Mp->Texture_FaceInterval[0]=Mp->num_p;
	X=new L1NormRenderer(w,width,height);
	ratio_w=width_e*1.0/width;
	ratio_h=height_e*1.0/height;

	if (morph==false) {
		min_x[0]=-0.1;max_x[0]=0.1;
		min_x[1]=1.0;max_x[1]=1.0;
		min_x[2]=1.0;max_x[2]=1.0;
		min_x[3]=-3.0;max_x[3]=3.0;
		min_x[4]=-3.0;max_x[4]=3.0;
	}
}
PatchMatch::~PatchMatch() {
	w->DeleteFrameBuffer(&fb);
	SDELETEA(outputname);
	SDELETE(X);
	SDELETE(Mp);
}
void PatchMatch::setOutputName(char *outputname) {
	this->outputname=strcln(outputname);
}
int PatchMatch::getDOF() {
	return n_dof;
}
double PatchMatch::Evaluate(double *particle,bool generate_output) {
	// Renderer
	w->SetFrameBuffer(&fb);
	w->SetDrawBuffer(0);
	w->SetReadBuffer(0);
	SetStandardOGLEnvironment(false,false,true);
	glDepthMask(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	w->SetImageCoords();
	
	if (use_morph) {
		int o=0;
		for(int i=0;i<Mp->num_p;i++) {
			if ((abs(M->Points[i][0])==1) || (abs(M->Points[i][1])==1)) {
				Mp->Points[i][0]=M->Points[i][0]*ratio_w;
				Mp->Points[i][1]=M->Points[i][1]*ratio_h;
			} else {
				Mp->Points[i][0]=(M->Points[i][0]*ratio_w)+(2.0*particle[o++]/width);
				Mp->Points[i][1]=(M->Points[i][1]*ratio_h)+(2.0*particle[o++]/height);
			}
			Mp->Points[i][2]=0.0;
		}
	} else {
		Matrix T(3,3);
		Vector<3> t;
		double c=cos(particle[0]);
		double s=sin(particle[0]);
		T[0][0]=c*ratio_w*particle[1];
		T[0][1]=-s*ratio_h*particle[2];
		T[1][0]=s*ratio_w*particle[1];
		T[1][1]=c*ratio_h*particle[2];
		T[0][2]=0.0;
		T[1][2]=0.0;
		T[2][2]=1.0;
		T[2][0]=0.0;
		T[2][1]=0.0;
		t[0]=2.0*particle[3]/width;
		t[1]=2.0*particle[4]/height;
		t[2]=0.0;
		for(int i=0;i<Mp->num_p;i++) {
			Multiply(&T,&(M->Points[i]),&(Mp->Points[i]));
			Mp->Points[i]+=t;
		}
	}
	
	w->DrawHyperMesh_Textured(Mp);
	w->ClearImageCoords();
	glDepthMask(1);
	
	if (((generate_output) || (save_particle_images)) && (outputname!=NULL)) {
		Bitmap<ColorRGB> out(width,height);
		glPixelStorei(GL_PACK_ALIGNMENT,1);
		w->RetrieveFrameBuffer(&fb,&out);
		glPixelStorei(GL_PACK_ALIGNMENT,4);
		out.Save(outputname);
	} 
	w->ClearFrameBuffer(&fb);

	return (double)(X->L1Norm3C(fb.fbt,texture_2));
}
