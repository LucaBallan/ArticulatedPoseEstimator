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
//							FREE GPGPU CONTEXT
//
//***********************************************************************************************************************

HDC GPGPU::Context_hDC=NULL;
HGLRC GPGPU::Context_hRC=NULL;

bool GPGPU::CreateContext() {
	if (Context_hDC!=NULL) return true;
	if (Context_hRC!=NULL) return true;
	
	int n, pf;
    LOGPALETTE* lpPal;
    PIXELFORMATDESCRIPTOR pfd;
    DWORD flags = PFD_DOUBLEBUFFER;
    BYTE  type  = PFD_TYPE_RGBA;
    WNDCLASS wc;
	HINSTANCE Current_Instance = GetModuleHandle(NULL);
	HPALETTE hPalette = NULL;

	//
	// Windows Class
	//
	wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = Current_Instance;
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "GPGPU_WClass";
	if (!RegisterClass(&wc)) return false;


	//
	// Create Window
	//
	HWND hWnd = CreateWindow("GPGPU_WClass", "GPGPU", WS_OVERLAPPEDWINDOW,
			0,0,CW_USEDEFAULT,CW_USEDEFAULT, NULL, NULL, Current_Instance, NULL);
	if (hWnd==NULL) return false;
	Context_hDC=GetDC(hWnd);

	//
	// Choose a pixel format
	//
	memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType   = type;
    pfd.cDepthBits   = 32;
    pfd.cColorBits   = 32;
	pfd.cStencilBits = 10;			// TODO
	pf = ChoosePixelFormat(Context_hDC, &pfd);
	if (pf==0) ErrorExit("ChoosePixelFormat() failed.");
    if (SetPixelFormat(Context_hDC, pf, &pfd) == FALSE) ErrorExit("SetPixelFormat() failed.");
    DescribePixelFormat(Context_hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (pfd.dwFlags & PFD_NEED_PALETTE || pfd.iPixelType == PFD_TYPE_COLORINDEX) {
		if (!hPalette) {
			n = 1 << pfd.cColorBits;
			if (n > 256) n = 256;

			lpPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
			if (lpPal == NULL) ErrorExit("Internal error.");
			memset(lpPal, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
			lpPal->palVersion = 0x300;
			lpPal->palNumEntries = n;

			GetSystemPaletteEntries(Context_hDC, 0, n, &lpPal->palPalEntry[0]);
			if (pfd.iPixelType == PFD_TYPE_RGBA) {
				int redMask = (1 << pfd.cRedBits) - 1;
				int greenMask = (1 << pfd.cGreenBits) - 1;
				int blueMask = (1 << pfd.cBlueBits) - 1;
				int i;
				for (i = 0; i < n; ++i) {
				lpPal->palPalEntry[i].peRed = 
					(((i >> pfd.cRedShift)   & redMask)   * 255) / redMask;
				lpPal->palPalEntry[i].peGreen = 
					(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
				lpPal->palPalEntry[i].peBlue = 
					(((i >> pfd.cBlueShift)  & blueMask)  * 255) / blueMask;
				lpPal->palPalEntry[i].peFlags = 0;
				}
			} else {
				lpPal->palPalEntry[0].peRed = 0;
				lpPal->palPalEntry[0].peGreen = 0;
				lpPal->palPalEntry[0].peBlue = 0;
				lpPal->palPalEntry[0].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[1].peRed = 255;
				lpPal->palPalEntry[1].peGreen = 0;
				lpPal->palPalEntry[1].peBlue = 0;
				lpPal->palPalEntry[1].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[2].peRed = 0;
				lpPal->palPalEntry[2].peGreen = 255;
				lpPal->palPalEntry[2].peBlue = 0;
				lpPal->palPalEntry[2].peFlags = PC_NOCOLLAPSE;
				lpPal->palPalEntry[3].peRed = 0;
				lpPal->palPalEntry[3].peGreen = 0;
				lpPal->palPalEntry[3].peBlue = 255;
				lpPal->palPalEntry[3].peFlags = PC_NOCOLLAPSE;
			}
			hPalette = CreatePalette(lpPal);
			if (hPalette) {
				SelectPalette(Context_hDC, hPalette, FALSE);
				RealizePalette(Context_hDC);
			}
			free(lpPal);
		} else {
			SelectPalette(Context_hDC, hPalette, FALSE);
			RealizePalette(Context_hDC);
		}
	}


	//
	// SetUp OPENGL
	//
	Context_hRC = wglCreateContext(Context_hDC);
	if (Context_hRC==NULL) {
		DestroyWindow(hWnd);
		Context_hDC=NULL;
		return false;
	}
	ShowWindow(hWnd,SW_HIDE);
	
	return true;
}













//***********************************************************************************************************************
//
//							GPGPU
//
//***********************************************************************************************************************

GPGPU::GPGPU(int width, int height,int channels) {
	n_texture=0;


	// Init OPENGL
	if (!CreateContext()) ErrorExit("GPGPU: Framework not initializated.");
	wglMakeCurrent(Context_hDC, Context_hRC);


	glewInit();
	if (!glewIsSupported("GL_VERSION_2_0")) {
		ErrorExit("OpenGL 2.0 not supported.");
	}
	if (!glewIsSupported("GL_EXT_framebuffer_object")) {
		ErrorExit("FBO not supported.");
	}

	this->width=width;
	this->height=height;

	// Init
	glGenFramebuffersEXT(1,&fbo);
	Activate();

		// Now setup a texture to render to
		glGenTextures(1, &OutTexture);
		glBindTexture(GL_TEXTURE_2D, OutTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Attach it to the FBO so we can render to it
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, OutTexture, 0);

	// Check errors
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		ErrorExit("glCheckFramebufferStatusEXT error.");
	}

	// Disabilita un po' di controlli
	glDisable(GL_DEPTH_TEST);	// TODO: *** altro...
}

GPGPU::~GPGPU() {
};

void GPGPU::EnsureContext() {
	wglMakeCurrent(Context_hDC, Context_hRC);
}

GLuint GPGPU::LoadBitmap(Bitmap<ColorRGB> *I) {
   	UINT texture;
	glGenTextures(1,&texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		// o LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		// o LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,I->width,I->height,0,GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer()); 
		
	return texture;
}

GLuint GPGPU::LoadBitmap(Bitmap<GreyLevel> *I) {
   	UINT texture;
	glGenTextures(1,&texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		// o LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);		// o LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,I->width,I->height,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,I->getBuffer()); 
		
	return texture;
}

void GPGPU::Activate() {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
}
void GPGPU::DeActivate() {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GPGPU::GetResult(Bitmap<ColorRGB> *I) {
	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer());
}
void GPGPU::GetResult(Bitmap<GreyLevel> *I) {
	glReadPixels(0,0,width,height,GL_LUMINANCE,GL_UNSIGNED_BYTE,I->getBuffer());
}

void GPGPU::Run() {
	// Set Viewpoint
	glViewport(0,0,width,height);	
	//
	// x=xnd*(width/2)+(width/2)+1
	// y=ynd*(height/2)+(height/2)-1
	// 
	// ynd           y
	//   0         h/2-1
	//  -1+2/h       0
	//  +1           h


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	// Clear Screen And Depth Buffer
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	


	// TODO
	glActiveTextureARB(GL_TEXTURE0_ARB); 
	glBindTexture(GL_TEXTURE_2D, WorkingImages[0]);
	glEnable(GL_TEXTURE_2D); 
	glActiveTextureARB(GL_TEXTURE1_ARB); 
	glBindTexture(GL_TEXTURE_2D, WorkingImages[1]);
	glEnable(GL_TEXTURE_2D); 
	/*glActiveTextureARB(GL_TEXTURE2_ARB); 
	glBindTexture(GL_TEXTURE_2D, WorkingImages[2]);
	glEnable(GL_TEXTURE_2D); */

	//


	double pW,pH;
	pW=2.0/width;
	pH=2.0/height;
		
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, 1.0f);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, 1.0f);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();

	
/*	
 *		TEST DI PROIEZIONE
 *
	glBegin(GL_QUADS);
		glColor4f(0.0f,1.0f,0.0f,1.0f);
		glVertex2f(-1.5, -1.5);
		glVertex2f(1.5, -1.5);
		glVertex2f(1.5, 1.5);
		glVertex2f(-1.5, 1.5);
		glVertex2f(-1.5, -1.5);
	glEnd();
	glBegin(GL_LINE_STRIP);
		glColor4f(0.0f,0.0f,0.0f,1.0f);
		glVertex2f(-1.0, -1.0+pH);
		glVertex2f(1.0-pW, -1.0+pH);
		glVertex2f(1.0-pW, 1.0);
		glVertex2f(-1.0, 1.0);
		glVertex2f(-1.0, -1.0+pH);
	glEnd();
*/

/*  
 *       PIXEL TO PIXEL MAP
 *
    GLdouble mat[16];
	mat[0]=2.0/width;mat[1]=0;mat[2]=0;mat[3]=0.0;
	mat[4]=0;mat[5]=-2.0/height;mat[6]=0;mat[7]=0.0;
	mat[8]=0.0;mat[9]=0.0;mat[10]=0;mat[11]=0;
	mat[12]=-1.0;mat[13]=1.0;mat[14]=0;mat[15]=1;
	glLoadMatrixd(mat);
*/

}

void GPGPU::SetProgram(GLuint prg) {
	glUseProgram(prg);
	glUniform1i(glGetUniformLocation(prg,"WorkingImage1"),0);
	glUniform1i(glGetUniformLocation(prg,"WorkingImage2"),1);
	glUniform1i(glGetUniformLocation(prg,"WorkingImage3"),2);
	glUniform1i(glGetUniformLocation(prg,"WorkingImage4"),3);
	glUniform1i(glGetUniformLocation(prg,"WorkingImage5"),4);			// TODO: supporta solo 5 texture contemporanee!!!

	glUniform1i(glGetUniformLocation(prg,"Row"),256);					// TODO!!!!!!!!!!***********
}


bool GPGPU::PushImage(GLuint texture) {
	WorkingImages[n_texture]=texture;
	n_texture++;
	return true;
}

void GPGPU::ClearImageList() {
	n_texture=0;
}

GLuint GPGPU::CreateShader(char *VertexFile,char *FragmentFile,bool PrintLog) {
		GLuint v,f;
	char *vs = NULL,*fs = NULL;

	vs = TextFileRead(VertexFile);
	fs = TextFileRead(FragmentFile);

	if ((vs==NULL)||(fs==NULL)) {
		if (vs) free(vs);
		if (fs) free(fs);
		return 0;
	}


	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);


	if (PrintLog) printf("Compiling %s:",VertexFile);
	glCompileShader(v);
	CheckShaderLog(v,PrintLog,vs);

	if (PrintLog) printf("Compiling %s:",FragmentFile);
	glCompileShader(f);
	CheckShaderLog(f,PrintLog,fs);

	free(vs);free(fs);

	GLuint prog = glCreateProgram();
	glAttachShader(prog,v);
	glAttachShader(prog,f);

	if (PrintLog) printf("Linking:");
	glLinkProgram(prog);
	CheckLinkerLog(prog,PrintLog);
	
	return prog;
}

