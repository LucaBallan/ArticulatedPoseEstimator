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





//***********************************************************************************************************************
//
//							Advance Render Window
//
//***********************************************************************************************************************



AdvancedRenderWindow::AdvancedRenderWindow(char *title,
										   void(*display)(void*),
										   bool(*command)(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *),
										   void *userdata,
										   int Background_Color,
										   HMENU hMenu,
										   DWORD WindowStyle,
										   int RenderWindowProperties) : RenderWindow<3>(title,display,command,userdata,Background_Color,hMenu,WindowStyle,RenderWindowProperties) {
	
	// Init
	ToonShader=0;
	HatchingShader=0;
	HatchingShader2=0;
	for(int i=0;i<8;i++) HatchingText[i]=0;


	Activate();
	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0")) {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	if ((RenderWindowProperties&RENDERWINDOW_VSYNC) || (stereo_view)) wglSwapIntervalEXT(1);

	#ifdef USE_CUDA
	    cudaGLSetGLDevice(cutGetMaxGflopsDeviceId());
	#endif

	SET_PROGRAM_0=true;
	DeActivate();
}

AdvancedRenderWindow::~AdvancedRenderWindow() {
	DeleteShader(ToonShader);
	if (HatchingShader) {
		DeleteShader(HatchingShader);
		DeleteShader(HatchingShader2);
	
		DeleteTexture(HatchingText[0]);DeleteTexture(HatchingText[1]);DeleteTexture(HatchingText[2]);
		DeleteTexture(HatchingText[3]);DeleteTexture(HatchingText[4]);DeleteTexture(HatchingText[5]);
		DeleteTexture(HatchingText[6]);DeleteTexture(HatchingText[7]);
	}
}













//***********************************************************************************************************************
//
//							Shader Compiler
//
//***********************************************************************************************************************

bool CheckShaderLog(GLuint obj,bool print_log,char *file_text) {
    int infologLength = 0;
    int charsWritten  = 0;
	int compiled = 0;
    char *infoLog;

	if (print_log) {
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
		if (infologLength > 1) {
	        infoLog = (char *)malloc(infologLength);
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("\n%s\n",infoLog);
			if ((strstr(infoLog,"warning")!=NULL) || (strstr(infoLog,"error")!=NULL)) std::cout<<file_text<<"\n\n";
			free(infoLog);
		} else {
			printf(" Done.\n");
		}
	}

	glGetShaderiv(obj, GL_COMPILE_STATUS,&compiled);
	return (compiled==GL_TRUE);
}

bool CheckLinkerLog(GLuint obj,bool print_log) {
    int infologLength = 0;
    int charsWritten  = 0;
    int linked = 0;
	char *infoLog;

	if (print_log) {
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&infologLength);
	    if (infologLength > 1) {
		    infoLog = (char *)malloc(infologLength);
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
			printf("\n%s\n",infoLog);
			free(infoLog);
		} else {
			printf(" Done.\n");
		}
	}

	glGetProgramiv(obj,GL_LINK_STATUS,&linked);
	return (linked==GL_TRUE);
}

GLuint AdvancedRenderWindow::CreateShader(char *VertexFile,char *FragmentFile,bool PrintLog) {
	GLuint v,f;
	char *vs = NULL,*fs = NULL;


	Activate();

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	

	if (PrintLog) printf("Compiling %s:",VertexFile);
	vs = TextFileRead_Include(VertexFile);
	if (vs==NULL) {
		if (PrintLog) {printf(" File not found.\n");ErrorExit("A shader cannot be created.");}
		if (vs) free(vs);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	const char * vv = vs;
	glShaderSource(v, 1, &vv,NULL);
	glCompileShader(v);
	if (!CheckShaderLog(v,PrintLog,vs)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		if (vs) free(vs);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	

	if (PrintLog) printf("Compiling %s:",FragmentFile);
	fs = TextFileRead_Include(FragmentFile);
	if (fs==NULL) {
		if (PrintLog) {printf(" File not found.\n");ErrorExit("A shader cannot be created.");}
		if (vs) free(vs);
		if (fs) free(fs);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	const char * ff = fs;
	glShaderSource(f, 1, &ff,NULL);
	glCompileShader(f);
	if (!CheckShaderLog(f,PrintLog,fs)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		if (vs) free(vs);
		if (fs) free(fs);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}

	free(vs);free(fs);

	GLuint prog = glCreateProgram();
	glAttachShader(prog,v);
	glAttachShader(prog,f);

	if (PrintLog) printf("Linking:");
	glLinkProgram(prog);
	if (!CheckLinkerLog(prog,PrintLog)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		glDeleteProgram(prog);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	
	DeActivate();	

	return prog;
}

GLuint AdvancedRenderWindow::CreateShaderFromCode(char *VertexCode,char *FragmentCode,bool PrintLog) {
	GLuint v,f;


	Activate();

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	
	if (PrintLog) printf("Compiling:");
	const char *vv = VertexCode;
	glShaderSource(v,1,&vv,NULL);
	glCompileShader(v);
	if (!CheckShaderLog(v,PrintLog,VertexCode)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	
	if (PrintLog) printf("Compiling:");
	const char *ff = FragmentCode;
	glShaderSource(f,1,&ff,NULL);
	glCompileShader(f);
	if (!CheckShaderLog(f,PrintLog,FragmentCode)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog,v);
	glAttachShader(prog,f);

	if (PrintLog) printf("Linking:");
	glLinkProgram(prog);
	if (!CheckLinkerLog(prog,PrintLog)) {
		if (PrintLog) ErrorExit("A shader cannot be created.");
		glDeleteProgram(prog);
		glDeleteShader(v);
		glDeleteShader(f);
		DeActivate();
		return 0;
	}
	
	DeActivate();

	return prog;
}

void AdvancedRenderWindow::DeleteShader(GLuint shader) {
	if (!shader) return;
	
	Activate();
	
	GLuint shaders[10];
	GLsizei n_s=0;
	glGetAttachedShaders(shader,10,&n_s,shaders);
	for(int i=0;i<n_s;i++) {
		glDetachShader(shader,shaders[i]);
		glDeleteShader(shaders[i]);
	}
	glDeleteProgram(shader);
	
	DeActivate();
}

void AdvancedRenderWindow::SetShader(GLuint shader) {
	glUseProgram(shader);
}



void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,int value) {
	GLint x=glGetUniformLocation(shader,var_name);
#ifdef CHECK_UNIFORM_NAMES
	if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
#endif
	glUniform1i(x,value);
}

void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,double value) {
	GLint x=glGetUniformLocation(shader,var_name);
#ifdef CHECK_UNIFORM_NAMES
	if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
#endif
	glUniform1f(x,value);
}

void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,Vector<3> *value) {
	GLint x=glGetUniformLocation(shader,var_name);
#ifdef CHECK_UNIFORM_NAMES
	if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
#endif
	glUniform3f(x,(*value)[0],(*value)[1],(*value)[2]);	
}

void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,Vector<4> *value) {
	GLint x=glGetUniformLocation(shader,var_name);
#ifdef CHECK_UNIFORM_NAMES
	if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
#endif
	glUniform4f(x,(*value)[0],(*value)[1],(*value)[2],(*value)[3]);
}

void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,Matrix *value) {
	float tmp[16];
	value->GetCM(tmp);

	GLint x=glGetUniformLocation(shader,var_name);
	#ifdef CHECK_UNIFORM_NAMES
		if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
	#endif	

	if ((value->r==4) && (value->c==4)) glUniformMatrix4fv(x,1,false,tmp);
	else {
		if ((value->r==3) && (value->c==3)) glUniformMatrix3fv(x,1,false,tmp);
		else {
			ErrorExit("SetUniformVariable not implemented.");
		}
	}
}

void AdvancedRenderWindow::SetUniformVariable(GLuint shader,char *var_name,GLdouble *value) {
	float tmp[16];
	for(int i=0;i<16;i++) tmp[i]=(float)value[i];

	GLint x=glGetUniformLocation(shader,var_name);
	#ifdef CHECK_UNIFORM_NAMES
		if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
	#endif
	glUniformMatrix4fv(x,1,false,tmp);
}

void AdvancedRenderWindow::SetUniformArray(GLuint shader,char *var_name,int count,float *values) {
	GLint x=glGetUniformLocation(shader,var_name);
	#ifdef CHECK_UNIFORM_NAMES
		if (x==-1) ErrorExit_str("Uniform variable \"%s\" does not exist.",var_name);
	#endif
	glUniform1fv(x,count,values);
}

inline void AdvancedRenderWindow::ActivateTexture(int level) {
	glActiveTextureARB(GL_TEXTURE0_ARB+level);
}

inline void AdvancedRenderWindow::TexCoord(int level,Vector<2> uv) {
	glMultiTexCoord2dARB(GL_TEXTURE0_ARB+level,uv[0],uv[1]);
}
inline void AdvancedRenderWindow::TexCoord(int level,Vector<3> uvw) {
	glMultiTexCoord3dARB(GL_TEXTURE0_ARB+level,uvw[0],uvw[1],uvw[2]);
}

void AdvancedRenderWindow::Run2DShader(UINT InTexture) {
	UINT InTextures[1];
	InTextures[0]=InTexture;
	Run2DShader(1,InTextures);
}

void AdvancedRenderWindow::Run2DShader(int num_textures_maps,UINT *InTextures,Vector<2> *uv_tl,Vector<2> *uv_br) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	for(int map_index=0;map_index<num_textures_maps;map_index++) {
		ActivateTexture(map_index);
		glBindTexture(GL_TEXTURE_2D,InTextures[map_index]);
	}

	SetImageCoords();
		
	glBegin(GL_QUADS);
		for(int map_index=0;map_index<num_textures_maps;map_index++) glMultiTexCoord2dARB(GL_TEXTURE0_ARB+map_index,uv_tl[map_index][0],uv_tl[map_index][1]);
		glVertex2f(-1.0, -1.0);
		for(int map_index=0;map_index<num_textures_maps;map_index++) glMultiTexCoord2dARB(GL_TEXTURE0_ARB+map_index,uv_br[map_index][0],uv_tl[map_index][1]);
		glVertex2f(1.0, -1.0);
		for(int map_index=0;map_index<num_textures_maps;map_index++) glMultiTexCoord2dARB(GL_TEXTURE0_ARB+map_index,uv_br[map_index][0],uv_br[map_index][1]);
		glVertex2f(1.0, 1.0);
		for(int map_index=0;map_index<num_textures_maps;map_index++) glMultiTexCoord2dARB(GL_TEXTURE0_ARB+map_index,uv_tl[map_index][0],uv_br[map_index][1]);
		glVertex2f(-1.0, 1.0);
		for(int map_index=0;map_index<num_textures_maps;map_index++) glMultiTexCoord2dARB(GL_TEXTURE0_ARB+map_index,uv_tl[map_index][0],uv_tl[map_index][1]);
		glVertex2f(-1.0, -1.0);
	glEnd();

	ClearImageCoords();
	glPopAttrib();
}

void AdvancedRenderWindow::Run2DShader(int num_textures_maps,UINT *InTextures) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	for(int map_index=0;map_index<num_textures_maps;map_index++) {
		ActivateTexture(map_index);
		glBindTexture(GL_TEXTURE_2D,InTextures[map_index]);
	}

	SetImageCoords();
		
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, 1.0f);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, 1.0f);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();

	ClearImageCoords();
	glPopAttrib();
}

void AdvancedRenderWindow::RetrieveDepthBuffer(UINT texture_index) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texture_index);
	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],0);
	glDisable(GL_TEXTURE_2D);
}









FrameBuffer_struct AdvancedRenderWindow::CreateFrameBuffer(int width,int height,UINT *TextureArray,int num_textures,int default_buffer_format,int default_buffer_options,bool generate_depth_buffer) {
	FrameBuffer_struct fbs;
	fbs.fbo=NULL;
	fbs.fbt=INVALID_TEXTURE;
	fbs.fbd=INVALID_TEXTURE;
	fbs.InputTexture=INVALID_TEXTURE;
	fbs.write_buffer=0;
	fbs.w=width;
	fbs.h=height;
	fbs.delete_fbt=false;
	fbs.valid=true;

	Activate();
		
		if ((num_textures==1) && (TextureArray[0]==INVALID_TEXTURE)) fbs.delete_fbt=true;

		for(int i=0;i<num_textures;i++) {
			if (TextureArray[i]==INVALID_TEXTURE) TextureArray[i]=CreateTexture(width,height,default_buffer_options,default_buffer_format);
		}

		glGenFramebuffersEXT(1,&fbs.fbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs.fbo);

		fbs.fbt=TextureArray[0];
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbs.fbt, 0);

		for(int i=1;i<num_textures;i++) {
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, TextureArray[i], 0);
		}
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);


		if (generate_depth_buffer) {
			glGenRenderbuffersEXT(1, &fbs.fbd);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbs.fbd);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbs.fbd);
		}

		// Check errors
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			ErrorExit("glCheckFramebufferStatusEXT error.");
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	DeActivate();
	return fbs;

}

FrameBuffer_struct AdvancedRenderWindow::CreateFrameBuffer(int width,int height,int buffer_format,bool ping_pong_fbo,bool generate_depth_buffer) {
	FrameBuffer_struct fbs;
	fbs.fbo=NULL;
	fbs.fbt=INVALID_TEXTURE;
	fbs.fbd=INVALID_TEXTURE;
	fbs.InputTexture=INVALID_TEXTURE;
	fbs.write_buffer=0;
	fbs.w=width;
	fbs.h=height;
	fbs.delete_fbt=true;
	fbs.valid=true;

	GLenum format;GLint internalformat;int tmp;
	get_format(buffer_format,format,internalformat,tmp);

	Activate();
		
		glGenFramebuffersEXT(1,&fbs.fbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs.fbo);

		// Now setup a texture to render to
		glGenTextures(1, &fbs.fbt);
		glBindTexture(GL_TEXTURE_2D, fbs.fbt);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat,  width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		// Attach it to the FBO so we can render to it
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbs.fbt, 0);

		// Attach ping pong texture
		if (ping_pong_fbo) {
			glGenTextures(1, &fbs.InputTexture);
			glBindTexture(GL_TEXTURE_2D, fbs.InputTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, internalformat,  width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, fbs.InputTexture, 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}

		if (generate_depth_buffer) {
			glGenRenderbuffersEXT(1, &fbs.fbd);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbs.fbd);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, fbs.fbd);
		}

		// Check errors
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			ErrorExit("glCheckFramebufferStatusEXT error.");
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


	DeActivate();
	return fbs;
}

void AdvancedRenderWindow::SetFrameBuffer(FrameBuffer_struct *fb) {
	Activate();
	glGetIntegerv(GL_VIEWPORT,fb->old_viewport);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb->fbo);
	glViewport(0,0,fb->w,fb->h);
}
void AdvancedRenderWindow::SetDrawBuffer(int index) {
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+index);
}
void AdvancedRenderWindow::SetReadBuffer(int index) {
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT+index);
}
void AdvancedRenderWindow::SetDrawBuffers(int num) {
	GLenum buffers[30];
	for(int i=0;i<num;i++) buffers[i]=GL_COLOR_ATTACHMENT0_EXT+i;
	
	glDrawBuffers(num,buffers);
}
void AdvancedRenderWindow::SetDrawBuffers(int num_buffers,int *nums) {
	GLenum buffers[30];
	for(int i=0;i<num_buffers;i++) buffers[i]=GL_COLOR_ATTACHMENT0_EXT+nums[i];
	
	glDrawBuffers(num_buffers,buffers);
}


void AdvancedRenderWindow::PingPong(FrameBuffer_struct *fb) {
	if (fb->InputTexture==INVALID_TEXTURE) ErrorExit("The current FrameBuffer has not the ping-pong capability.");
	
	if (fb->write_buffer==0) {
		glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
		fb->write_buffer=1;
	} else {
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		fb->write_buffer=0;
	}
	swap(fb->InputTexture,fb->fbt);
}

void AdvancedRenderWindow::RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<GreyLevel> *I) {
	glReadPixels(0,0,fb->w,fb->h,GL_RED,GL_UNSIGNED_BYTE,I->getBuffer());
}
void AdvancedRenderWindow::RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorRGB> *I) {
	glReadPixels(0,0,fb->w,fb->h,GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer());
}
void AdvancedRenderWindow::RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorRGBA> *I) {
	glReadPixels(0,0,fb->w,fb->h,GL_RGBA,GL_UNSIGNED_BYTE,I->getBuffer());
}
void AdvancedRenderWindow::RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorFloat> *I,int channel) {
	glReadPixels(0,0,fb->w,fb->h,channel,GL_FLOAT,I->getBuffer());
}


void AdvancedRenderWindow::ClearFrameBuffer(FrameBuffer_struct *fb) {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(fb->old_viewport[0],fb->old_viewport[1],fb->old_viewport[2],fb->old_viewport[3]);
	DeActivate();
}

void AdvancedRenderWindow::DeleteFrameBuffer(FrameBuffer_struct *fb) {
	if (!fb->valid) return;

	Activate();

	if (fb->delete_fbt) {
		if (fb->fbt!=INVALID_TEXTURE) {
			glDeleteTextures(1,&(fb->fbt));
			fb->fbt=INVALID_TEXTURE;
		}
		fb->delete_fbt=false;
	}
	if (fb->InputTexture!=INVALID_TEXTURE) {
		glDeleteTextures(1,&(fb->InputTexture));
		fb->InputTexture=INVALID_TEXTURE;
	}
	if (fb->fbd!=INVALID_TEXTURE) {
		glDeleteRenderbuffersEXT(1,&(fb->fbd));
		fb->fbd=INVALID_TEXTURE;
	}
	if (fb->fbo!=INVALID_TEXTURE) {
		glDeleteFramebuffersEXT(1,&(fb->fbo));
		fb->fbo=INVALID_TEXTURE;
	}
	fb->valid=false;

	DeActivate();
}








//***********************************************************************************************************************
//
//							Draw Primitives
//
//***********************************************************************************************************************

void AdvancedRenderWindow::DrawHyperMesh_MultiTexture(HyperMesh<3> *mesh,int num_textures_maps,int num_textures_parameterizations,UINT *Textures,List<HyperMesh<3>::PointType> **TextureUVW,List<HyperMesh<3>::FaceType> **Texture_Face) {
	Debug_Assert(num_textures_maps<MAX_NUMBER_TEXTURE_PARAMETERIZATIONS,"Too many texture parameterizations.");

	Vector<3> texpoint;
	HyperFace<3> *texf[MAX_NUMBER_TEXTURE_PARAMETERIZATIONS];
	HyperFace<3> *f=mesh->Faces.getMem();
	Vector<3>    *P=mesh->Points.getMem();
	Vector<3>    *N=mesh->Normals.getMem();
	
	
	glEnable(GL_TEXTURE_2D);
	for(int map_index=0;map_index<num_textures_maps;map_index++) {
		ActivateTexture(map_index);
		glBindTexture(GL_TEXTURE_2D,Textures[map_index]);
	}
	
	

	glBegin(GL_TRIANGLES);

	for (int i=0;i<mesh->num_f;i++,f++) {

		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texf[map_index]=&(*(Texture_Face[map_index]))[i];
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[0]];
			TexCoord(map_index,texpoint);
		}
		glNormal3dv(N[f->Point[0]].GetList());
		Vertex(P[f->Point[0]]);


		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[1]];
			TexCoord(map_index,texpoint);
		}
		glNormal3dv(N[f->Point[1]].GetList());
		Vertex(P[f->Point[1]]);


		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[2]];
			TexCoord(map_index,texpoint);
		}
		glNormal3dv(N[f->Point[2]].GetList());
		Vertex(P[f->Point[2]]);

	}
	glEnd();

	ActivateTexture(0);
	glDisable(GL_TEXTURE_2D);
}

void AdvancedRenderWindow::DrawHyperMesh_MultiTexture(int num_f,
									List<HyperMesh<3>::PointType> *Points,
									List<HyperMesh<3>::FaceType>  *Faces,
									List<HyperMesh<3>::PointType> *Normals,
									int num_textures_maps,int num_textures_parameterizations,
									UINT *Textures,
									List<HyperMesh<3>::PointType> **TextureUVW,
									List<HyperMesh<3>::FaceType>  **Texture_Face) {
	
	Debug_Assert(num_textures_maps<MAX_NUMBER_TEXTURE_PARAMETERIZATIONS,"Too many texture parameterizations.");

	Vector<3> texpoint;
	HyperFace<3> *texf[MAX_NUMBER_TEXTURE_PARAMETERIZATIONS];
	HyperFace<3> *f=Faces->getMem();
	Vector<3>    *P=Points->getMem();
	Vector<3>    *N=NULL;
	if (Normals)  N=Normals->getMem();
	
	
	glEnable(GL_TEXTURE_2D);
	for(int map_index=0;map_index<num_textures_maps;map_index++) {
		ActivateTexture(map_index);
		glBindTexture(GL_TEXTURE_2D,Textures[map_index]);
	}
	
	

	glBegin(GL_TRIANGLES);

	for (int i=0;i<num_f;i++,f++) {

		
		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texf[map_index]=&(*(Texture_Face[map_index]))[i];
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[0]];
			TexCoord(map_index,texpoint);
		}
		if (N) glNormal3dv(N[f->Point[0]].GetList());
		else glNormal3dv(f->Normal(Points).GetList());
		Vertex(P[f->Point[0]]);


		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[1]];
			TexCoord(map_index,texpoint);
		}
		if (N) glNormal3dv(N[f->Point[1]].GetList());
		Vertex(P[f->Point[1]]);


		for(int map_index=0;map_index<num_textures_parameterizations;map_index++) {
			texpoint=(*(TextureUVW[map_index]))[texf[map_index]->Point[2]];
			TexCoord(map_index,texpoint);
		}
		if (N) glNormal3dv(N[f->Point[2]].GetList());
		Vertex(P[f->Point[2]]);

	}
	glEnd();

	ActivateTexture(0);
	glDisable(GL_TEXTURE_2D);
}








//***********************************************************************************************************************
//
//							Toon Shader 
//
//***********************************************************************************************************************

void AdvancedRenderWindow::InitToonShader() {
	if (ToonShader) return;

	ToonShader=CreateShader(DATA_DIRECTORY"Toon_v.vert.c",DATA_DIRECTORY"Toon_f.frag.c",false);
}

void AdvancedRenderWindow::DrawHyperMesh_Toon(HyperMesh<3> *mesh,int LineColor,float LineWidth,bool Flat,float EdgeCreaseAngle) {

	// Toon Shading
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glUseProgram(ToonShader);
	if (Flat) DrawHyperMesh_Flat(mesh);
	else DrawHyperMesh(mesh);


	// Draw Silhouette & Restore Settings
	glUseProgram(0);
	DrawEdgesMacro(mesh,LineColor,LineWidth,EdgeCreaseAngle);
}




























//***********************************************************************************************************************
//
//							Hatching Shader 
//
//***********************************************************************************************************************

void AdvancedRenderWindow::InitHatchingShader(float TextScale) {
	if (HatchingShader) return;


	HatchingShader=CreateShader(DATA_DIRECTORY"Hatching_v.vert.c",DATA_DIRECTORY"Hatching_f.frag.c",false);
	HatchingShader2=CreateShader(DATA_DIRECTORY"Hatching_v.vert.c",DATA_DIRECTORY"Hatching2_f.frag.c",false);

	Activate();

	HatchingText[0]=LoadTexture(DATA_DIRECTORY"Stoke1.bmp");
	HatchingText[1]=LoadTexture(DATA_DIRECTORY"Stoke2.bmp");
	HatchingText[2]=LoadTexture(DATA_DIRECTORY"Stoke3.bmp");
	HatchingText[3]=LoadTexture(DATA_DIRECTORY"Stoke4.bmp");
	HatchingText[4]=LoadTexture(DATA_DIRECTORY"Stoke5.bmp");
	HatchingText[5]=LoadTexture(DATA_DIRECTORY"Stoke1a.bmp");
	HatchingText[6]=LoadTexture(DATA_DIRECTORY"Stoke2a.bmp");
	HatchingText[7]=LoadTexture(DATA_DIRECTORY"Stoke3a.bmp");


	glUseProgram(HatchingShader);
	glUniform1f(glGetUniformLocation(HatchingShader,"TextureZoom"),TextScale);
	glUniform1i(glGetUniformLocation(HatchingShader,"Stoke1"),0);
	glUniform1i(glGetUniformLocation(HatchingShader,"Stoke2"),1);
	glUniform1i(glGetUniformLocation(HatchingShader,"Stoke3"),2);
	glUniform1i(glGetUniformLocation(HatchingShader,"Stoke4"),3);
	glUniform1i(glGetUniformLocation(HatchingShader,"Stoke5"),4);

	glUseProgram(HatchingShader2);
	glUniform1f(glGetUniformLocation(HatchingShader2,"TextureZoom"),TextScale);
	glUniform1i(glGetUniformLocation(HatchingShader2,"Stoke1a"),0);
	glUniform1i(glGetUniformLocation(HatchingShader2,"Stoke2a"),1);
	glUniform1i(glGetUniformLocation(HatchingShader2,"Stoke3a"),2);


	DeActivate();
}


void AdvancedRenderWindow::DrawHyperMesh_Hatching(HyperMesh<3> *mesh,int mode,int LineColor,float LineWidth,bool Flat,float EdgeCreaseAngle) {

	// Hatching Shading
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	if (mode) {
		glUseProgram(HatchingShader2);
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[5]); 
		glEnable(GL_TEXTURE_2D); 
		glActiveTextureARB(GL_TEXTURE1_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[6]); 
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE2_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[7]); 
		glEnable(GL_TEXTURE_2D);
	} else {
		glUseProgram(HatchingShader);
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[0]); 
		glEnable(GL_TEXTURE_2D); 
		glActiveTextureARB(GL_TEXTURE1_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[1]); 
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE2_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[2]); 
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE3_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[3]); 
		glEnable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE4_ARB); 
		glBindTexture(GL_TEXTURE_2D,HatchingText[4]); 
		glEnable(GL_TEXTURE_2D);
	}

	if (Flat) DrawHyperMesh_Textured_Flat(mesh);
	else DrawHyperMesh_Textured(mesh);


	// Draw Silhouette & Restore Settings
	glUseProgram(0);
	DrawEdgesMacro(mesh,LineColor,LineWidth,EdgeCreaseAngle);
	
	// TODO: Disabilitare tutte le texture altrimenti dopo di qua disegna tutto nero (a tutti i livelli)!!!
}



















//***********************************************************************************************************************
//
//							Common 
//
//***********************************************************************************************************************

void AdvancedRenderWindow::DrawEdgesMacro(HyperMesh<3> *mesh,int LineColor,float LineWidth,float EdgeCreaseAngle) {
	GLfloat OldLineWidth;

	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glGetFloatv(GL_LINE_WIDTH,&OldLineWidth);
	glLineWidth(LineWidth);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	glPolygonMode(GL_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	glColor3ubv((GLubyte*)(&LineColor));
	DrawHyperMesh(mesh);
	
	if (EdgeCreaseAngle!=-1.0f) {
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

				if (h*f<EdgeCreaseAngle) DrawLine(p1,p2,LineColor);
			}
		}

	}

	// Restore settings
	glEnable(GL_LIGHTING);
	glPolygonMode(GL_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glLineWidth(OldLineWidth);
	glDepthFunc(GL_LESS);
}





















//***********************************************************************************************************************
//
//							Cast Shadows
//
//***********************************************************************************************************************

/*
 TODO: Ottimizzare!!!!!!!!!!
	il problema della sparizione dell ombre penso sia dovuto al fatto che i poligoni che dovrebbero essere delle linee
	nn vengano disegnati perche' troppo stretti mentre alti si -> si creano dei vuoti d'ombra... perche' allora nn si 
	creano dei pieni d'ombra? per lo stesso principio dovrebbero crearsi no? nn e' qualcosa del depht?
	tentativo nn disegnare le sup che visibilmate hanno area nulla oppure comandare opengl a disegnare anche i bordi!!
	cosa molto piu' simple.. cosi' siamo sicuri che funziona...
	A favore della prima ipotesi vi e' il fatto che l'errore si manifesta in certi angoli indipendentemente dalla scala..
	Cio' significa che i poligoni sono messi di lato e alcuni un po' meno di lato ->i primi nn vengon disegnati mentre i seconsi si..
*/
#define SOGLIA 0.0
void AdvancedRenderWindow::ExtendVolume(HyperFace<3> *f,List<typename Vector<3>> *Points,int localLight,GLfloat *lightPosition,float extendDistance) {
	Vector<3> *Point1,*Point2;
	Vector<3> Vertex1,Vertex2;
	Vector<3> extendedVertex1,extendedVertex2;
	Vector<3> extendDirection;
	Vector<3> MinuslightPosition;

	MinuslightPosition[0] = -lightPosition[0];
	MinuslightPosition[1] = -lightPosition[1];
	MinuslightPosition[2] = -lightPosition[2];

	glFrontFace(GL_CCW);
	for(int i=0;i<3;i++) {
		Point1=&((*Points)[f->Point[i]]);
		Point2=&((*Points)[f->Point[(i+1)%3]]);

		if (!localLight) extendDirection = MinuslightPosition;
		else extendDirection = (*Point1) + MinuslightPosition;
		extendDirection=extendDirection.Versore();
		Vertex1 = (*Point1) + extendDirection*SOGLIA;
		extendedVertex1 = (*Point1) + extendDirection*extendDistance;
		
		if ((f->Normal(Points)*extendDirection)>0) glFrontFace(GL_CCW);
		else glFrontFace(GL_CW);

		if (!localLight) extendDirection = MinuslightPosition;
		else extendDirection = (*Point2) + MinuslightPosition;
		extendDirection=extendDirection.Versore();
		Vertex2 = (*Point2) + extendDirection*SOGLIA;
		extendedVertex2 = (*Point2) + extendDirection*extendDistance;



		glBegin(GL_TRIANGLES);
			Vector<3> N=(extendedVertex1-(*Point1))^((*Point2)-(*Point1));        // Vertex1 circa = Point1 PENSO TODO!!!
			N=-1*N.Versore();
			glNormal3d(N[0],N[1],N[2]);

			glVertex3d(Vertex1[0],Vertex1[1],Vertex1[2]);
			glVertex3d(Vertex2[0],Vertex2[1],Vertex2[2]);
			glVertex3d(extendedVertex1[0],extendedVertex1[1],extendedVertex1[2]);
		glEnd();

		glBegin(GL_TRIANGLES);
			N=(extendedVertex1-(*Point2))^(extendedVertex2-(*Point2));
			N=-1*N.Versore();
			glNormal3d(N[0],N[1],N[2]);

			glVertex3d(Vertex2[0],Vertex2[1],Vertex2[2]);
			glVertex3d(extendedVertex2[0],extendedVertex2[1],extendedVertex2[2]);
			glVertex3d(extendedVertex1[0],extendedVertex1[1],extendedVertex1[2]);
		glEnd();

	}


	glFrontFace(GL_CCW);
}

/*
void AdvancedRenderWindow::ExtendVolume(HyperFace<3> *f,List<typename Vector<3>> *Points,int localLight,GLfloat *lightPosition,float extendDistance) {
	Vector<3> Dir;
	Vector<3> *Point1,*Point2;
	Vector<3> extendedVertex1,extendedVertex2;
	GLfloat extendDirection[3];

	glFrontFace(GL_CCW);
	for(int i=0;i<3;i++) {
		Point1=&((*Points)[f->Point[i]]);
		Point2=&((*Points)[f->Point[(i+1)%3]]);

		if (!localLight) {
			extendDirection[0] = -lightPosition[0];
			extendDirection[1] = -lightPosition[1];
			extendDirection[2] = -lightPosition[2];
		} else {
			extendDirection[0] = (*Point1)[0] - lightPosition[0];
			extendDirection[1] = (*Point1)[1] - lightPosition[1];
			extendDirection[2] = (*Point1)[2] - lightPosition[2];
		}
		extendedVertex1[0] = (*Point1)[0] + extendDirection[0] * extendDistance;
		extendedVertex1[1] = (*Point1)[1] + extendDirection[1] * extendDistance;
		extendedVertex1[2] = (*Point1)[2] + extendDirection[2] * extendDistance;
		
		Dir[0]=extendDirection[0];
		Dir[1]=extendDirection[1];
		Dir[2]=extendDirection[2];
		
		//	possibile soluzione al problema delle ombre sulla superficie (nn funziona!!!!)
		//Dir=Dir.Versore();	// serve solo qui
		//if (((f->Normal(Points)*Dir)<0.2) && ((f->Normal(Points)*Dir)>-0.2)) return;	
		//
		// 2o modo
		/*(*Point1)[0]=(*Point1)[0]+ extendDirection[0] * 0.2;
		(*Point1)[1]=(*Point1)[1]+ extendDirection[1] * 0.2;
		(*Point1)[2]=(*Point1)[2]+ extendDirection[2] * 0.2;
		(*Point2)[0]=(*Point2)[0]+ extendDirection[0] * 0.2;
		(*Point2)[1]=(*Point2)[1]+ extendDirection[1] * 0.2;
		(*Point2)[2]=(*Point2)[2]+ extendDirection[2] * 0.2;*/
		//
/*
		if ((f->Normal(Points)*Dir)>0) glFrontFace(GL_CCW);
		else glFrontFace(GL_CW);


		if (!localLight) {
			extendDirection[0] = -lightPosition[0];
			extendDirection[1] = -lightPosition[1];
			extendDirection[2] = -lightPosition[2];
		} else {
			extendDirection[0] = (*Point2)[0] - lightPosition[0];
			extendDirection[1] = (*Point2)[1] - lightPosition[1];
			extendDirection[2] = (*Point2)[2] - lightPosition[2];
		} 
		extendedVertex2[0] = (*Point2)[0] + extendDirection[0] * extendDistance;
		extendedVertex2[1] = (*Point2)[1] + extendDirection[1] * extendDistance;
		extendedVertex2[2] = (*Point2)[2] + extendDirection[2] * extendDistance;

		glBegin(GL_TRIANGLES);
			Vector<3> N=(extendedVertex1-(*Point1))^((*Point2)-(*Point1));
			N=-1*N.Versore();
			glNormal3d(N[0],N[1],N[2]);

			glVertex3d((*Point1)[0],(*Point1)[1],(*Point1)[2]);
			glVertex3d((*Point2)[0],(*Point2)[1],(*Point2)[2]);
			glVertex3d(extendedVertex1[0],extendedVertex1[1],extendedVertex1[2]);
		glEnd();

		glBegin(GL_TRIANGLES);
			N=(extendedVertex1-(*Point2))^(extendedVertex2-(*Point2));
			N=-1*N.Versore();
			glNormal3d(N[0],N[1],N[2]);

			glVertex3d((*Point2)[0],(*Point2)[1],(*Point2)[2]);	
			glVertex3d(extendedVertex2[0],extendedVertex2[1],extendedVertex2[2]);
			glVertex3d(extendedVertex1[0],extendedVertex1[1],extendedVertex1[2]);
		glEnd();

	}


	glFrontFace(GL_CCW);
}
*/

void AdvancedRenderWindow::InitCastShadow() {
	glDisable(GL_LIGHTING); 

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(0);

	glColorMask(0,0,0,0);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,0xFF);
	glStencilMask(~0u);

	glEnable(GL_CULL_FACE);
}

void AdvancedRenderWindow::CastShadow(HyperMesh<3> *mesh,int LightType,Vector<3> LightPosition,float MaxDistance) {
	GLfloat Light[4];
	Light[0]=LightPosition[0];
	Light[1]=LightPosition[1];
	Light[2]=LightPosition[2];
	Light[3]=1.0;

	CastShadow(mesh,LightType,Light,MaxDistance);
}
void AdvancedRenderWindow::CastShadow(HyperMesh<3> *mesh,int LightType,GLfloat *LightPosition,float MaxDistance) {
	glStencilOp(GL_KEEP, GL_KEEP,GL_INCR);
	glCullFace(GL_BACK);
	/*glDepthFunc(GL_LESS);
	DrawHyperMesh(mesh);  //Servono?? sembra di no
	glDepthFunc(GL_LEQUAL);*/
	for(int i=0;i<mesh->num_f;i++) {
		ExtendVolume(&mesh->Faces[i],&mesh->Points,LightType,LightPosition,MaxDistance);
	}
	
	glStencilOp(GL_KEEP, GL_KEEP,GL_DECR);
	glCullFace(GL_FRONT);
	/*glDepthFunc(GL_LESS);
	DrawHyperMesh(mesh);  //Servono?? sembra di no
	glDepthFunc(GL_LEQUAL);*/
	for(int i=0;i<mesh->num_f;i++) {
		ExtendVolume(&mesh->Faces[i],&mesh->Points,LightType,LightPosition,MaxDistance);
	}
}

void AdvancedRenderWindow::RenderDarkShadow(UINT ShadowColor) {

	glDepthFunc(GL_LEQUAL);
	glDepthMask(1);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	glColorMask(1,1,1,1);

	glEnable(GL_STENCIL_TEST); 
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glStencilMask(~0u);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 	glColor4ub(((ShadowColor>>16)&0xFF),((ShadowColor>>8)&0xFF),(ShadowColor&0xFF),((ShadowColor>>24)&0xFF));

	
	// Draw all the screen
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

			glBegin(GL_TRIANGLES);
				glVertex3d(-1,-1,0.5);
				glVertex3d(+1,+1,0.5);
				glVertex3d(-1,+1,0.5);
			glEnd();
			glBegin(GL_TRIANGLES);
				glVertex3d(+1,+1,0.5);
				glVertex3d(-1,-1,0.5);
				glVertex3d(+1,-1,0.5);
				glEnd();
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING); 
}

void AdvancedRenderWindow::Pass1_DeclareShadowOn(HyperMesh<3> *mesh) {
	// Azzero Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(1);
	glDepthRange(1,1);
	
	glEnable(GL_STENCIL_TEST); 
	glStencilMask(~0u);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glDisable(GL_CULL_FACE);

	DrawHyperMesh(mesh);
}

void AdvancedRenderWindow::Pass2_RenderShadow() {
	glDepthRange(0,1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(1);
	glDepthRange(0,1);

	glColorMask(1,1,1,1);

	glEnable(GL_STENCIL_TEST); 
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glStencilMask(~0u);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING); 
}

void AdvancedRenderWindow::Pass3_Close() {
	glDisable(GL_STENCIL_TEST);
}


















