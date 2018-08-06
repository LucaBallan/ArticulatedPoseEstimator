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





#define REQUIRED_EXTENSIONS "WGL_ARB_pbuffer " \
                            "WGL_ARB_pixel_format "


LRESULT CALLBACK OGLWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
		return DefWindowProc(hWnd, message, wParam, lParam);
}

void init_opengl() {
	//////
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= OGLWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "Protected";
	wcex.hIconSm		= NULL;
	ATOM Class=RegisterClassEx(&wcex);

	HWND hWnd = CreateWindow("Protected", "-",0,CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, wcex.hInstance, NULL);
	ShowWindow(hWnd,SW_HIDE);
	
	//HDC hDC=GetDC(NULL);
	HDC hDC=GetDC(hWnd);
	//////
	


	int pf;
	PIXELFORMATDESCRIPTOR pfd;
	
	
	
	memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cDepthBits   = 32;
    pfd.cColorBits   = 32;

	pf = ChoosePixelFormat(hDC, &pfd);
	if (!SetPixelFormat(hDC, pf, &pfd)) WindowsErrorExit();

	HGLRC hRC = wglCreateContext(hDC);
	if (hRC==NULL) {
        printf("Unable to load the OpenGl extension(s)\n");
		exit(1);
	}
	wglMakeCurrent(hDC,hRC);

	if (!glh_init_extensions(REQUIRED_EXTENSIONS)) {
        printf("Unable to load the following extension(s): %s\n\nExiting...\n", 
               glh_get_unsupported_extensions());
		exit(1);
    }

	//wglDeleteContext(hRC);	
}


OpenGL_FastTextelRenderer::OpenGL_FastTextelRenderer(int width,int height,int src_w,int src_h) : pbuffer("rgb") {
	this->width=width;
	this->height=height;
	this->src_w=src_w;
	this->src_h=src_h;
	
	pbuffer.Initialize(width,height,false,true);

	pbuffer.Activate();
	glEnable(GL_TEXTURE_2D);
    glClearColor( 0.5, 0.5, 0.5, 0.5 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glViewport(0,0,width,height);


	////////////////////////////////////////////
	glEnable(GL_POLYGON_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
	////////////////////////////////////////////

	pbuffer.Deactivate();
}

bool OpenGL_FastTextelRenderer::IsValid() {
	return ((pbuffer.GetWidth()==width) && (pbuffer.GetHeight()==height));
}

OpenGL_FastTextelRenderer::~OpenGL_FastTextelRenderer() {
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
	// Free ????????????????????????????????????????????????????????????????????????
}


UINT OpenGL_FastTextelRenderer::LoadTexture(Bitmap<ColorRGBA> *tex) {
    pbuffer.Activate();

	UINT texture;
	// allocate a texture name
	glGenTextures(1,&texture);

	glBindTexture(GL_TEXTURE_2D,texture);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	// when texture area is small, bilinear filter the closest MIP map
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	// when texture area is large, bilinear filter the first MIP map
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	gluBuild2DMipmaps(GL_TEXTURE_2D,4,tex->width,tex->height,GL_RGBA,GL_UNSIGNED_BYTE,(BYTE*)tex->getBuffer());

	pbuffer.Deactivate();
	return texture;
}


UINT OpenGL_FastTextelRenderer::LoadTexture(Bitmap<ColorRGB> *tex) {
    pbuffer.Activate();

	UINT texture;
	// allocate a texture name
	glGenTextures(1,&texture);

	glBindTexture(GL_TEXTURE_2D,texture);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	gluBuild2DMipmaps(GL_TEXTURE_2D,3,tex->width,tex->height,GL_RGB,GL_UNSIGNED_BYTE,(BYTE*)tex->getBuffer());

	pbuffer.Deactivate();
	return texture;
}

void OpenGL_FastTextelRenderer::DeleteTexture(UINT index) {
	glDeleteTextures(1,&index);
}



void OpenGL_FastTextelRenderer::DrawTextel(Triangle<TPoint2D<int>> *D,Triangle<TPoint2D<int>> *S,UINT Src) {
	glBindTexture(GL_TEXTURE_2D,Src);
	glBegin(GL_TRIANGLES);
	
	glTexCoord2d(S->Point[0].x*1.0/src_w,S->Point[0].y*1.0/src_h);
	glVertex2d(2*D->Point[0].x*1.0/width-1.0,2*D->Point[0].y*1.0/height-1.0);
	
	glTexCoord2d(S->Point[1].x*1.0/src_w,S->Point[1].y*1.0/src_h);
	glVertex2d(2*D->Point[1].x*1.0/width-1.0,2*D->Point[1].y*1.0/height-1.0);
	
	glTexCoord2d(S->Point[2].x*1.0/src_w,S->Point[2].y*1.0/src_h);
	glVertex2d(2*D->Point[2].x*1.0/width-1.0,2*D->Point[2].y*1.0/height-1.0);
	
	glEnd();
}

void OpenGL_FastTextelRenderer::DrawFace(Triangle<TPoint2D<int>> *D,int color) {

	glBegin(GL_TRIANGLES);
	
	glColor3bv((GLbyte*)(&color));
	glVertex2d(2*D->Point[0].x*1.0/width-1.0,2*D->Point[0].y*1.0/height-1.0);
	glVertex2d(2*D->Point[1].x*1.0/width-1.0,2*D->Point[1].y*1.0/height-1.0);
	glVertex2d(2*D->Point[2].x*1.0/width-1.0,2*D->Point[2].y*1.0/height-1.0);
	
	glEnd();
}

void OpenGL_FastTextelRenderer::StartDraw() {
    pbuffer.Activate();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void OpenGL_FastTextelRenderer::EndDraw() {
	pbuffer.Deactivate();
}


void OpenGL_FastTextelRenderer::GetBuffer(BYTE *mem) {
    pbuffer.Activate();
	glReadPixels(0,0,pbuffer.GetWidth(),pbuffer.GetHeight(),GL_RGB,GL_UNSIGNED_BYTE,mem);
	pbuffer.Deactivate();
}

void OpenGL_FastTextelRenderer::GetBuffer(int x,int y,int lx,int ly,BYTE *mem) {
    pbuffer.Activate();
	glReadPixels(x,y,lx,ly,GL_RGB,GL_UNSIGNED_BYTE,mem);
	pbuffer.Deactivate();
}


float Triangle2D::Area() {
	// Heron's formula
	float a=(float)(Point[0]-Point[1]).Norm2();
	float b=(float)(Point[1]-Point[2]).Norm2();
	float c=(float)(Point[2]-Point[0]).Norm2();

	float s=0.5f*(a+b+c);
	float Area=sqrt(s*(s-a)*(s-b)*(s-c));
	return Area;
}
