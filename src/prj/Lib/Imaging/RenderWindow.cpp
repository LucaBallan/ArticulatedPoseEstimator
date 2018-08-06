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





#include "stdInclude.h"
#include "common.h"
#include "StdLib/IObuffer.h"
#include "Imaging/image.h"
#include "StdLib/Word.h"
#include "StdLib/ListEntities.h"
#include "StdLib/List.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Space.h"
#include "Math/HyperMesh.h"
#include <time.h>

#include "glew.h"
#include "wglew.h"
#ifdef _WIN64
	#include "..\gl\glut64.h"
#else
	#include "..\gl\glut.h"
#endif
#include "Imaging/RenderWindow.h"
#include "Imaging/Shader.h"
#include "stdlib/Crypt.h"

//#define USE_NVIDIA_API		// add "..\..\..\Common Libs\nvapi\x86\nvapi.lib" or "..\..\..\Common Libs\nvapi\amd64\nvapi64.lib"
#ifdef USE_NVIDIA_API
	#include "nvapi.h"
#endif


// Private shared method
void get_format(int channel,GLenum &format,GLint &internalformat,int &alter_options);
extern bool EnableRenderWindows;
void InitGammaLib(char *app_title, int argc = 0, char* argv[] = NULL);

// Checks
#ifdef _DEBUG
	#define CHECK_UNIFORM_NAMES
#endif


#include "AdvancedRenderWindows.hpp"
#include "GPGPU.hpp"


const Vector<2> ZERO_2;
const Vector<3> ZERO_3;
#include <time.h>
#include "Stdlib\MathAddOn.h"
#include "stdlib\IoAddOn.h"


using namespace std;
#pragma warning(3:4244)
// Istanze
//template class RenderWindow<2>;
template class RenderWindow<3>;
template LRESULT WINAPI RenderWindow_WindowProc<2>(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);  
template LRESULT WINAPI RenderWindow_WindowProc<3>(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);  


// Parametri
#define ZBUFFER_BITS				   32
#define STENCIL_BITS				   10
#define   ALPHA_BITS					8

#define LEN_ARROW					1/5.0
#define CHAR_SET			 "Calibri" //"Courier New"			//"Arial" or "Calibri" or "Times New Roman"
#define FONT_SIZE					   96 //18
#define ROTATION_CIRCLE_RADIUS		  0.9
#define ROTAZIONI_IN_UNO_SCHERMO		5





/*

	TODO 	glPushAttrib(GL_ENABLE_BIT);

*/


//***********************************************************************************************************************
//*************************** RenderWindow ******************************************************************************
//***********************************************************************************************************************

HPALETTE hPalette=NULL;
HINSTANCE Current_Instance=NULL;
HCURSOR C_rt,C_zoom,C_move;
int RenderWindowClassCounter=0;





//***********************************************************************************************************************
//
//							Gestione Window
//
//***********************************************************************************************************************

void UpdateClient(HWND hWnd,bool Redraw) {
	RECT Rect;
	GetClientRect(hWnd,&Rect);
	InvalidateRect(hWnd,&Rect,Redraw);
}

void GetClientSize(HWND hWnd,int &w,int &h) {					
	RECT Rect;
	GetClientRect(hWnd,&Rect);
	w=Rect.right-Rect.left;    // da msdn questo mi da la dimensione esatta (senza il +1)
	h=Rect.bottom-Rect.top;
}

void SetClientSize(HWND hWnd,int w,int h) {
	RECT rcClient,rcWindow;
	POINT ptDiff;
	GetClientRect(hWnd,&rcClient);
	GetWindowRect(hWnd,&rcWindow);
	ptDiff.x=(rcWindow.right-rcWindow.left)-rcClient.right;
	ptDiff.y=(rcWindow.bottom-rcWindow.top)-rcClient.bottom;
	MoveWindow(hWnd,rcWindow.left,rcWindow.top,w+ptDiff.x,h+ptDiff.y,true);
}


#define GET_WH int w,h;{RECT Rect;GetClientRect(Curr_RenderWindow->hWnd,&Rect);w=Rect.right-Rect.left;h=Rect.bottom-Rect.top;}

template <int dim> 
LRESULT WINAPI RenderWindow_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
	// Retrive info
	RenderWindow<dim> *Curr_RenderWindow=(RenderWindow<dim> *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	
	// Check if it is a virtual Window
	if ((Curr_RenderWindow) && (Curr_RenderWindow->virtual_window)) return DefWindowProc(hWnd, uMsg, wParam, lParam); 


	// Proceed with the normal tasks
	int mx = LOWORD(lParam);
	int my = HIWORD(lParam);
	if (mx & 1 << 15) mx -= (1 << 16);						// Win32 is pretty braindead about the x, y position that it returns when the mouse is off the left or top edge of the window 
	if (my & 1 << 15) my -= (1 << 16);						// (due to them being unsigned). therefore, roll the Win32's 0..2^16 pointer co-ord range to the more amenable (and useful) 0..+/-2^15. 

	
	if ((Curr_RenderWindow!=NULL) && (!Curr_RenderWindow->lock)) {
		switch(uMsg) {
			case WM_SIZE:
				Curr_RenderWindow->w=mx;
				Curr_RenderWindow->h=my;
				UpdateClient(hWnd);
				return 0;

			case WM_RBUTTONDOWN:
				if (Curr_RenderWindow->StandardMouseController) {
					GET_WH;
					if (Curr_RenderWindow->InsideControllerArea(mx/(float)w,my/(float)h)) {
						SetCapture(hWnd);
						Curr_RenderWindow->mouse.x=mx;
						Curr_RenderWindow->mouse.y=my;
						Curr_RenderWindow->mouse.buttons[0]=1;
						Curr_RenderWindow->Savedeye=Curr_RenderWindow->eye;
						SetCursor(C_zoom);
					}
				}
				break;

			case WM_LBUTTONDOWN:
				if (Curr_RenderWindow->StandardMouseController) {
					GET_WH;
					if (Curr_RenderWindow->InsideControllerArea(mx/(float)w,my/(float)h)) {
						SetCapture(hWnd);
						Curr_RenderWindow->mouse.x=mx;
						Curr_RenderWindow->mouse.y=my;
						Curr_RenderWindow->mouse.buttons[1]=1;
						Curr_RenderWindow->SavedTMatrix.Set(&(Curr_RenderWindow->TMatrix));
						{
							// Controlla se il click è avvenuto al di fuori del Cerchio di rotazione	// TODO aggiustare con la controllable area!!!
							int dx,dy,r;
							r=(int)(ROTATION_CIRCLE_RADIUS*min((w/2),(h/2)));
							dx=(mx-(w/2));
							dy=(my-(h/2));
							if (((dx*dx)+(dy*dy))>(r*r)) Curr_RenderWindow->mouse.buttons[1]=2;
						}
						SetCursor(C_rt);
					}
				}
				break;

			case WM_MBUTTONDOWN:
				if (Curr_RenderWindow->StandardMouseController) {
					GET_WH;
					if (Curr_RenderWindow->InsideControllerArea(mx/(float)w,my/(float)h)) {
						SetCapture(hWnd);
						Curr_RenderWindow->mouse.x=mx;
						Curr_RenderWindow->mouse.y=my;
						Curr_RenderWindow->mouse.buttons[2]=1;
						Curr_RenderWindow->Savedeye=Curr_RenderWindow->eye;
						SetCursor(C_move);
					}
				}
				break;

			case WM_RBUTTONUP:
				if (Curr_RenderWindow->StandardMouseController) {
					Curr_RenderWindow->mouse.buttons[0]=0;
					ReleaseCapture();
				}
				break;
			case WM_LBUTTONUP:
				if (Curr_RenderWindow->StandardMouseController) {
					Curr_RenderWindow->mouse.buttons[1]=0;
					ReleaseCapture();
				}
				break;
			case WM_MBUTTONUP:
				if (Curr_RenderWindow->StandardMouseController) {
					Curr_RenderWindow->mouse.buttons[2]=0;
					ReleaseCapture();
				}
				break;

			case WM_MOUSEMOVE:
				if (Curr_RenderWindow->StandardMouseController) {
					GET_WH;

					if (Curr_RenderWindow->mouse.buttons[0]) {
						// Zoom
						Vector<3> scale;
						if (Curr_RenderWindow->UseLocalCoords) scale=get_map_scale(&(Curr_RenderWindow->SavedTMatrix));
						else {
							scale[0]=scale[1]=scale[2]=Curr_RenderWindow->LocalCoordsRadius;
						}

						Curr_RenderWindow->eye[2]=Curr_RenderWindow->Savedeye[2]+40*scale[2]*((my - Curr_RenderWindow->mouse.y)/(float)h);
						UpdateClient(hWnd);
					}
					if (Curr_RenderWindow->mouse.buttons[1]) {
						// Rotate
						float rot0=0.0,rot1=0.0,rot2=0.0;
						if (Curr_RenderWindow->mouse.buttons[1]==2) {
							rot2=-3.1415f*((my - Curr_RenderWindow->mouse.y)/(float)h)*ROTAZIONI_IN_UNO_SCHERMO;
						} else {
							rot0=3.1415f*((my - Curr_RenderWindow->mouse.y)/(float)h)*ROTAZIONI_IN_UNO_SCHERMO;
							rot1=3.1415f*((mx - Curr_RenderWindow->mouse.x)/(float)w)*ROTAZIONI_IN_UNO_SCHERMO;
						}
						Matrix TmpRot1(4,4),TmpRot2(4,4),TmpRot3(4,4);
						TmpRot3.Set(&(Curr_RenderWindow->SavedTMatrix));
						RotationMatrix(&TmpRot1,rot0,1,0,0);
						Multiply(&TmpRot1,&TmpRot3,&TmpRot2);
						RotationMatrix(&TmpRot1,rot1,0,1,0);
						Multiply(&TmpRot1,&TmpRot2,&TmpRot3);
						RotationMatrix(&TmpRot1,rot2,0,0,1);
						Multiply(&TmpRot1,&TmpRot3,&(Curr_RenderWindow->TMatrix));
						UpdateClient(hWnd);
					}
					if (Curr_RenderWindow->mouse.buttons[2]) {
						// Translate
						Vector<3> scale;
						if (Curr_RenderWindow->UseLocalCoords) scale=get_map_scale(&(Curr_RenderWindow->SavedTMatrix));
						else {
							scale[0]=scale[1]=scale[2]=Curr_RenderWindow->LocalCoordsRadius;
						}
						
						Curr_RenderWindow->eye[0]=Curr_RenderWindow->Savedeye[0]-4*scale[0]*((mx - Curr_RenderWindow->mouse.x)/(float)w);
						Curr_RenderWindow->eye[1]=Curr_RenderWindow->Savedeye[1]+4*scale[1]*((my - Curr_RenderWindow->mouse.y)/(float)h);
						UpdateClient(hWnd);
					} 
				}
				break;
		}
	}

	switch(uMsg) {
		case WM_PAINT:
			if (Curr_RenderWindow!=NULL) Curr_RenderWindow->draw();
			else {
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
			}
			return false;

		case WM_SIZE:
			UpdateClient(hWnd);
			return false;

		case WM_PALETTECHANGED:
			if (hWnd == (HWND)wParam) break;
			/* fall through to WM_QUERYNEWPALETTE */
		case WM_QUERYNEWPALETTE:
			if (Curr_RenderWindow!=NULL) return Curr_RenderWindow->palette();
			else return false;
	}

	
	
	// Controllo Locale
	if (Curr_RenderWindow!=NULL) {
		if (Curr_RenderWindow->command!=NULL) {
			if (Curr_RenderWindow->command(hWnd,uMsg,wParam,lParam,Curr_RenderWindow->userdata)) {
				UpdateClient(hWnd);
				return true;
			}
		}
	}
	
	
	// Controllo di Default
	if (uMsg!=WM_CLOSE) return DefWindowProc(hWnd, uMsg, wParam, lParam); 
	return 0;
} 



template <int dim> 
ATOM RenderWindow<dim>::InitWindowClass(int RenderWindowProperties) {

	if (Current_Instance==NULL) {
		Current_Instance = GetModuleHandle(NULL);
		C_rt=LoadCursorFromFile(DATA_DIRECTORY"Rotate.cur");
		C_zoom=LoadCursorFromFile(DATA_DIRECTORY"Zoom.cur");
		C_move=LoadCursorFromFile(DATA_DIRECTORY"Pan.cur");
	}


	WNDCLASSEX wc;
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style=CS_OWNDC;
	if (RenderWindowProperties&RENDERWINDOW_DBLCLICK) wc.style|=CS_DBLCLKS;
	wc.lpfnWndProc=(WNDPROC)RenderWindow_WindowProc<dim>;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=Current_Instance;
	wc.hIcon=(HICON)LoadImage(Current_Instance,DATA_DIRECTORY"AppIcon.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=NULL;
	wc.lpszMenuName=NULL;
	wc.hIconSm=NULL;
	
	char Name[256];
	sprintf(Name,"RenderWindow_%06i%",RenderWindowClassCounter);
	RenderWindowClassCounter++;
	wc.lpszClassName=Name;
	return RegisterClassEx(&wc);
}

template <int dim> 
HWND RenderWindow<dim>::GethWnd() {
	return hWnd;
}
template <int dim> 
HDC RenderWindow<dim>::GethDC() {
	return hDC;
}
template <int dim> 
void RenderWindow<dim>::Set_Program_0() {
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
}

template <int dim> 
bool RenderWindow<dim>::palette() {
	if (hPalette) {
		UnrealizeObject(hPalette);
		SelectPalette(hDC, hPalette,false);
		RealizePalette(hDC);
		return true;
	}
	return false;
}
template <int dim> 
int RenderWindow<dim>::width() {
	return w;
}
template <int dim> 
int RenderWindow<dim>::height() {
	return h;
}
template <int dim> 
double RenderWindow<dim>::aspect_ratio() {
	return (1.0*w/h);
}
template <int dim> 
double RenderWindow<dim>::clip_min() {
	return clipping_min;
}
template <int dim> 
double RenderWindow<dim>::clip_max() {
	return clipping_max;
}













//***********************************************************************************************************************
//
//							SetUp 3D
//
//***********************************************************************************************************************

template <int dim> 
RenderWindow<dim>::RenderWindow(char *title,
				 void(*display)(void*),
				 bool(*command)(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *),
				 void *userdata,
				 int Background_Color,
				 HMENU hMenu,
				 DWORD WindowStyle,
				 int RenderWindowProperties)  : TMatrix(4,4),SavedTMatrix(4,4),LockingProjectionMatrix(4,4) {
	this->InsideContext=0;
	this->EnableDraw_=false;
	this->display=display;
	this->command=command;
	this->userdata=userdata;
	this->Background_Color_r=((Background_Color>>16)&0xFF)/255.0f;
	this->Background_Color_g=((Background_Color>>8)&0xFF)/255.0f;
	this->Background_Color_b=(Background_Color&0xFF)/255.0f;
	this->Background_Color_a=((Background_Color>>24)&0xFF)/255.0f;
	virtual_window=(display==NULL);
		if (RenderWindowProperties&RENDERWINDOW_STEREO) this->stereo_view=true;
		else this->stereo_view=false;
		this->display_buffer=0;
		this->display_right=display;
		this->userdata_right=userdata;
		if (RenderWindowProperties&RENDERWINDOW_FULLSCREEN) this->fullscreen=true;
		else this->fullscreen=false;
	

	// Window
	hdcold=NULL;
	hglrcold=NULL;
	hWnd=NULL;
	hRC=NULL;
	hDC=NULL;
	SET_PROGRAM_0=false;

	// Default for no-init
	w=h=0;
	UseLocalCoords=false;
	LocalCoordsCenter.SetZero();
	LocalCoordsRadius=1.0;
	clipping_min=0.1;
	clipping_max=20;
	fontbase=NULL;
	DepthBits=0;
	StencilBits=0;
	for(int i=0;i<USED_CHARACTERS_NUMBER;i++) font_size[i].size_x=font_size[i].size_y=font_size[i].step_x=0;
	SpoilerScreenBehaviour=2;
	init_time=0;
	CriticalSection=CreateMutex(NULL,FALSE,NULL);
	SceneCenter.SetZero();
	SceneInverseDiameter=1.0;

	// init Interaction
	lock=false;
	lock_p=false;
	mouse.buttons[0]=0;
	mouse.buttons[1]=0;
	mouse.buttons[2]=0;
	mouse.x = 0;
	mouse.y = 0;
	Savedeye[0] = 0.0f;   eye[0] = 0.0f;
	Savedeye[1] = 0.0f;   eye[1] = 0.0f;
	Savedeye[2] = 3.0f;   eye[2] = 3.0f;
	TMatrix.SetIdentity();
	SavedTMatrix.SetIdentity();
	StandardMouseController=true;	
	MouseControllerArea.P0=VECTOR(0.0,0.0);
	MouseControllerArea.P1=VECTOR(1.0,1.0);


	// Init Window
	if (!EnableRenderWindows) InitGammaLib("GammaLib: Computer Vision library");
	ATOM class_n = InitWindowClass(RenderWindowProperties);
	
	char *title_;
	if (title==NULL) title_=getTitle("");
	else title_=getTitle(title);
		CreateRenderWindow(title_,class_n,hMenu,WindowStyle,(RenderWindowProperties&RENDERWINDOW_DO_NOT_SHOW_INFO)?false:true);
	delete []title_;
}

template <int dim> 
RenderWindow<dim>::~RenderWindow() {
    CloseHandle(CriticalSection);
    wglDeleteContext(hRC);
	ReleaseDC(hWnd,hDC);
    DestroyWindow(hWnd);
	if (fontbase!=NULL) glDeleteLists(fontbase,96);
}

template <int dim> 
void RenderWindow<dim>::CreateRenderWindow(char* title,
										   ATOM class_n,
										   HMENU hMenu,
										   DWORD WindowStyles,bool show_rw_information) {
	RECT Rect;
	int pf;
    LOGPALETTE* lpPal;
    PIXELFORMATDESCRIPTOR pfd;
	int curr_w,curr_h;

	curr_w=GetSystemMetrics(SM_CXSCREEN);
	curr_h=GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen) {
		WindowStyles=WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_POPUP|WS_MAXIMIZE;
	} else {
		curr_w=(int)(0.75*curr_w);
		curr_h=(int)(0.75*curr_h);
	}

	//
	// Create Window
	//
	if (!virtual_window) hWnd=CreateWindow((LPCTSTR)class_n,title,WindowStyles,0,0,curr_w,curr_h,NULL,hMenu,Current_Instance,NULL);
	else hWnd=CreateWindow((LPCTSTR)class_n,NULL,WindowStyles,0,0,curr_w,curr_h,NULL,NULL,Current_Instance,NULL);
	if (hWnd==NULL) ErrorExit("Can't create window.");


	ShowWindow(hWnd,SW_HIDE);
	SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)this);
	GetClientRect(hWnd,&Rect);
	w=Rect.right-Rect.left;
	h=Rect.bottom-Rect.top;
	hDC=GetDC(hWnd);


	//
	// Choose a pixel format (metto il massimo, tanto non viene allocato nulla, il frame buffer alla fine lo fara' nel caso di virtual window)
	//
	memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	if (!virtual_window) pfd.dwFlags = pfd.dwFlags | PFD_DRAW_TO_WINDOW;
	if (stereo_view) pfd.dwFlags = pfd.dwFlags | PFD_STEREO;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cDepthBits   = ZBUFFER_BITS;
    pfd.cColorBits   = 24;
	pfd.cAlphaBits   = ALPHA_BITS;
	pfd.cStencilBits = STENCIL_BITS;

	pf = ChoosePixelFormat(hDC, &pfd);
	if (pf==0) ErrorExit("ChoosePixelFormat() failed.");
	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) ErrorExit("SetPixelFormat() failed.");
    DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (show_rw_information) {
		cout<<"----------------------------\n";
		cout<<"RenderWindow properties:\n";
		cout<<"     color   bits: "<<((int)pfd.cColorBits)<<"\n";
		cout<<"     depth   bits: "<<((int)pfd.cDepthBits)<<"\n";
		cout<<"     alpha   bits: "<<((int)pfd.cAlphaBits)<<"\n";
		cout<<"     stencil bits: "<<((int)pfd.cStencilBits)<<"\n";
		if (pfd.dwFlags&PFD_STEREO) cout<<"     stereo enabled\n";
		cout<<"----------------------------\n";
	}
	if (!(pfd.dwFlags&PFD_STEREO)) stereo_view=false;
	this->DepthBits=pfd.cDepthBits;
	this->StencilBits=pfd.cStencilBits;


	if (pfd.dwFlags & PFD_NEED_PALETTE || pfd.iPixelType == PFD_TYPE_COLORINDEX) {
		if (!hPalette) {
			int n = 1 << pfd.cColorBits;
			if (n > 256) n = 256;

			lpPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
			if (lpPal == NULL) ErrorExit("Internal Error.");
			memset(lpPal, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
			lpPal->palVersion = 0x300;
			lpPal->palNumEntries = (WORD)n;

			GetSystemPaletteEntries(hDC, 0, n, &lpPal->palPalEntry[0]);
			if (pfd.iPixelType == PFD_TYPE_RGBA) {
				int redMask = (1 << pfd.cRedBits) - 1;
				int greenMask = (1 << pfd.cGreenBits) - 1;
				int blueMask = (1 << pfd.cBlueBits) - 1;
				int i;
				for (i = 0; i < n; ++i) {
				lpPal->palPalEntry[i].peRed = (BYTE)(
					(((i >> pfd.cRedShift)   & redMask)   * 255) / redMask);
				lpPal->palPalEntry[i].peGreen = (BYTE)(
					(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask);
				lpPal->palPalEntry[i].peBlue = (BYTE)(
					(((i >> pfd.cBlueShift)  & blueMask)  * 255) / blueMask);
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
				SelectPalette(hDC, hPalette, FALSE);
				RealizePalette(hDC);
			}
			free(lpPal);
		} else {
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
		}
	}


	//
	// SetUp OPENGL
	//
	hRC = wglCreateContext(hDC);
	Activate();
	


	//
	// SetUp Fonts
	//
	SetCurrentFont(CHAR_SET,FONT_SIZE);

	// Get Out
	DeActivate();
}

template <int dim> 
void RenderWindow<dim>::Activate() {
	ENTER_C;								// Assicura che un solo task ha il possesso del contesto
											//		Se lo stesso task richiede Activate() -> passa questa controllo senza problemi
											//		Se un task diverso lo richiede allora aspetta che il primo lo deactivi
	
	if (InsideContext==0) {
		hdcold=wglGetCurrentDC();
		hglrcold=wglGetCurrentContext();
		wglMakeCurrent(hDC,hRC);
	}
	InsideContext++;
}
template <int dim> 
void RenderWindow<dim>::DeActivate() {
	if (InsideContext==0) return;

	InsideContext--;
	if (InsideContext==0) wglMakeCurrent(hdcold,hglrcold);
	EXIT_C;
}



























//***********************************************************************************************************************
//
//							Gestione primitive di disegno 3D
//
//***********************************************************************************************************************
#include "DrawPrimitive.hpp"











//***********************************************************************************************************************
//
//							Gestione Redraw
//
//***********************************************************************************************************************
template <int dim> 
void RenderWindow<dim>::DrawInitScreen() {
	if (SET_PROGRAM_0) glUseProgram(0);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);

	SetImageCoords();
	Vector<dim> P;
	P[0]=-0.2;
	P[1]=-0.5;
	glColor4f(1.0,1.0,1.0,1.0);
	char str_c[]="\xd\x11\x9\x2\x26\x2\x0\x7e\x5b\x53\x41\x62\x1\x52\x3d\x17\x54\x44\x2b\x1d\x54\x77\x1c\x1d\xd\x1\x10\xe\x5a\x46\x65\x20";int len_c=32;
	Print(P,0xFFFFFFFF,crypt_strX(str_c,len_c));
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::ResetToOriginalCoords() {
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	if (!lock_p) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0,(GLdouble)w/(GLdouble)h,clipping_min,clipping_max);			//TODO: clipping_min == 0.1 ???
																						// r=clipping_max/clipping_min è il passo di precisione del depth buffer r grande -> tanti bit necessari
	} else SetProjectionMatrix(&LockingProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(-eye[0],-eye[1],-eye[2]);
	double Rotazione[16];
	TMatrix.GetCM(Rotazione);
	glMultMatrixd(Rotazione);
}

template <int dim> 
void RenderWindow<dim>::draw_opengl(void(*display_fnc)(void*),void *local_userdata) {
	
	//
	// Clear
	//
	glClearStencil(0);
	glClearColor(Background_Color_r,Background_Color_g,Background_Color_b,Background_Color_a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  
	//
	// Imposta View
	//
	ResetToOriginalCoords();	



	//
	// Default Settings
	//
	glEnable(GL_DEPTH_TEST);					// Depth
    glDepthRange(0.0,1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);						// Cull Face
	glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);						// Light
    glEnable(GL_LIGHT0);
	glEnable(GL_SMOOTH);						// Model Light
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// Filling
	glEnable(GL_BLEND);							// 2D
	glLineWidth(1.0);
	if (SET_PROGRAM_0) glUseProgram(0);


	display_fnc(local_userdata);
}

template <int dim> 
void RenderWindow<dim>::draw() {
	PAINTSTRUCT ps;

	// Disegna sul back buffer e poi swappa 
	if (EnableDraw_) {
		Activate();
		
		//
		// display
		//
		if (stereo_view) {
			glDrawBuffer(GL_BACK_LEFT);
			draw_opengl(display,userdata);					// Disegna sul back left buffer
			display_buffer=1;
			glDrawBuffer(GL_BACK_RIGHT);
			draw_opengl(display_right,userdata_right);		// Disegna sul back right buffer
			display_buffer=0;
		} else {
			// glDrawBuffer(GL_BACK);						// TODO**
			draw_opengl(display,userdata);					// Disegna sul back (left) buffer
		}
		


		//
		// Spoiler Screen
		//
		if (SpoilerScreenBehaviour!=0) {
			if (SpoilerScreenBehaviour==2) {
				init_time=clock();
				SpoilerScreenBehaviour=1;
			}
			clock_t now=clock();
			if (((now-init_time)*1.0/CLOCKS_PER_SEC)<INIT_TIME_SEC) {
				if (stereo_view) {
					glDrawBuffer(GL_BACK_LEFT);DrawInitScreen();
					glDrawBuffer(GL_BACK_RIGHT);DrawInitScreen();
				} else {
					// glDrawBuffer(GL_BACK);						// TODO**
					DrawInitScreen();
				}
			} else {
				SpoilerScreenBehaviour=0;
			}
		}


		//
		// Write to the front buffer
		//
		glFlush();
		SwapBuffers(hDC);
		DeActivate();	
	}

	BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
}


template <int dim> 
void RenderWindow<dim>::EnableDraw(bool enable,bool control_window_visibility) {
	if (virtual_window) return;
	EnableDraw_=enable;
	if (control_window_visibility) {
		if (EnableDraw_) ShowWindow(hWnd,SW_SHOW);
		else ShowWindow(hWnd,SW_HIDE);
	}
}
template <int dim> 
void RenderWindow<dim>::DefineLocalCoords(Vector<dim> Center,double radius,bool uselocalcoords,double max_distance_rel) {
	LocalCoordsCenter=Center;
	LocalCoordsRadius=radius;
	UseLocalCoords=uselocalcoords;

	clipping_min=(LocalCoordsRadius/(2*max_distance_rel));
	clipping_max=(LocalCoordsRadius*max_distance_rel);			// Deve essere cosi' anche se UseLocalCoords è falso.. non capisco il perche' nn si possa diminuire TODO
}
template <int dim> 
double RenderWindow<dim>::DefineNearFarPlane(double near_plane,double far_plane) {
	clipping_min=near_plane;
	clipping_max=far_plane;
	
	double z_center=(far_plane+near_plane)/2.0;
	double delta=z_center*z_center/(near_plane*(1<<DepthBits)-z_center);
	return delta;
}
template <int dim> 
double RenderWindow<dim>::DefineLocalCoords(Vector<dim> Center,double radius,bool uselocalcoords,double distance_closest_camera_to_scene_center,double distance_furthest_camera_to_scene_center) {
	LocalCoordsCenter=Center;
	LocalCoordsRadius=radius;
	UseLocalCoords=uselocalcoords;
	
	clipping_min=distance_closest_camera_to_scene_center-(radius*1.1);			// 10% more
	if (clipping_min<=0) ErrorExit("clipping plane too close to the camera.");
	clipping_max=distance_furthest_camera_to_scene_center+(radius*1.2);			// 20% more

	double z_center=(distance_closest_camera_to_scene_center+distance_furthest_camera_to_scene_center)/2;
	double delta=z_center*z_center/(clipping_min*(1<<DepthBits)-z_center);		// work only if clipping_min<<clipping_max

	return delta;
}
template <int dim> 
Vector<3> RenderWindow<dim>::GetEye() {
	return eye;
}
template <int dim> 
void RenderWindow<dim>::SetEye(Vector<3> x) {
	eye=x;
}
template <int dim> 
void RenderWindow<dim>::SetRightEyeFunction(void(*display_right)(void*),void *userdata_right) {
	this->display_right=display_right;
	this->userdata_right=userdata_right;
}
template <int dim> 
void RenderWindow<dim>::EnableStandardMouseController(bool StandardMouseController) {
	if (StandardMouseController) {
		if (!this->StandardMouseController) {
			// resetto mouse state
			ReleaseCapture();
			mouse.buttons[0]=0;
			mouse.buttons[1]=0;
			mouse.buttons[2]=0;
			mouse.x = 0;
			mouse.y = 0;

			// Enable StandardMouseController
			this->StandardMouseController=true;
		}
	} else {
		if (this->StandardMouseController) SetCursor(LoadCursor(NULL,IDC_ARROW));
		this->StandardMouseController=false;	
	}
}

template <int dim> 
bool RenderWindow<dim>::InsideControllerArea(float x,float y) {
	if (StandardMouseController) {
		if ((x>=MouseControllerArea.P0[0]) && (x<=MouseControllerArea.P1[0]) &&
			(y>=MouseControllerArea.P0[1]) && (y<=MouseControllerArea.P1[1]))		return true;
	} 
	return false;
}
template <int dim> 
void RenderWindow<dim>::SetMouseControllerArea(HyperBox<2> MouseControllerArea) {
	this->MouseControllerArea=MouseControllerArea;
}





























//************************************************************************************************************
//**  
//**  COORDINATES
//**  
//************************************************************************************************************

template <int dim> 
void RenderWindow<dim>::SetLocalCoords() {
	if (UseLocalCoords) {
		glMatrixMode(GL_MODELVIEW);
		glScaled(1/LocalCoordsRadius,1/LocalCoordsRadius,1/LocalCoordsRadius);
		glTranslated(-LocalCoordsCenter[0],-LocalCoordsCenter[1],-LocalCoordsCenter[2]);
	}
}

template <int dim> 
void RenderWindow<dim>::SetRT(Matrix *R,Vector<3> *T,Vector<3> *Scale) {
	int i,j;
	GLdouble m[16];

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (!Scale) {
		if (!R) {
			for(i=0;i<3;i++)
				for(j=0;j<3;j++) 
					m[i*4+j]=0.0;
			m[0*4+0]=1.0;
			m[1*4+1]=1.0;
			m[2*4+2]=1.0;
		} else {
			for(i=0;i<3;i++)
				for(j=0;j<3;j++)
					m[i*4+j]=(*R)[j][i];
		}
	} else {
		if (!R) {
			for(i=0;i<3;i++)
				for(j=0;j<3;j++) 
					m[i*4+j]=0.0;
			m[0*4+0]=(*Scale)[0];
			m[1*4+1]=(*Scale)[1];
			m[2*4+2]=(*Scale)[2];
		} else {
			for(i=0;i<3;i++)
				for(j=0;j<3;j++)
					m[i*4+j]=((*Scale)[i])*(*R)[j][i];
		}
	}

	if (!T) {
		m[3*4+0]=0.0;
		m[3*4+1]=0.0;
		m[3*4+2]=0.0;
	} else {
		m[3*4+0]=(*T)[0];
		m[3*4+1]=(*T)[1];
		m[3*4+2]=(*T)[2];
	}

	m[0*4+3]=0.0;
	m[1*4+3]=0.0;
	m[2*4+3]=0.0;
	m[3*4+3]=1.0;

	glMultMatrixd(m);
}

template <int dim> 
void RenderWindow<dim>::SetRTm(Matrix *RT,Vector<3> *Scale) {

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	GLdouble m[16];
	int i,j;

	if (!Scale) {
		for(i=0;i<3;i++)
			for(j=0;j<3;j++)
				m[i*4+j]=(*RT)[j][i];
	} else {
		for(i=0;i<3;i++)
			for(j=0;j<3;j++)
				m[i*4+j]=(*Scale)[i]*(*RT)[j][i];
	}
	
	m[3*4+0]=(*RT)[0][3];
	m[3*4+1]=(*RT)[1][3];
	m[3*4+2]=(*RT)[2][3];
	m[0*4+3]=0.0;
	m[1*4+3]=0.0;
	m[2*4+3]=0.0;
	m[3*4+3]=1.0;
	
	glMultMatrixd(m);
}

template <int dim> 
void RenderWindow<dim>::SetCameraCoords() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

template <int dim> 
void RenderWindow<dim>::ClearRT() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

template <int dim> 
void RenderWindow<dim>::SetIE(Matrix *Intrinsic,int w,int h,Matrix *Extrinsic) {
	Matrix PM(4,4);
	RenderWindow<dim>::Convert_Intrinsic(Intrinsic,w,h,clipping_min,clipping_max,&PM);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	SetProjectionMatrix(&PM);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	SetExtrinsic(Extrinsic);
}

template <int dim> 
void RenderWindow<dim>::SetImageCoords() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

template <int dim> 
void RenderWindow<dim>::SetImageCoords01() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	Matrix I(4,4);
	I.Set(2.0, 0.0, 0.0,-1.0,
		  0.0, 2.0, 0.0,-1.0,
		  0.0, 0.0, 1.0, 0.0,
		  0.0, 0.0, 0.0, 1.0);
	SetProjectionMatrix(&I);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

template <int dim> 
void RenderWindow<dim>::ClearImageCoords() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

































//***********************************************************************************************************************
//
//							Gestione Texture
//
//***********************************************************************************************************************


inline UINT create_texture(int options) {

	UINT texture;
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	
	// Options
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	
	if (options&TEXTURE_MIN_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (options&TEXTURE_MAG_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
	if (options&TEXTURE_CLAMP_X) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	} else {
		if (options&TEXTURE_CLAMP_EDGE_X) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		}
	}
	
	if (options&TEXTURE_CLAMP_Y) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	} else {
		if (options&TEXTURE_CLAMP_EDGE_Y) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
	}

	return texture;
}

inline void get_format(int channel,GLenum &format,GLint &internalformat,int &alter_options) {
	switch(channel) {
		case 0: internalformat=GL_LUMINANCE;
				format=GL_LUMINANCE;
				break;
		case 5: internalformat=GL_LUMINANCE16;
				format=GL_LUMINANCE;
				break;
		case 6: internalformat=GL_LUMINANCE16;
				format=GL_LUMINANCE;
				break;
		case 1: internalformat=GL_RGB;
				format=GL_RED;
				break;
		case 2: internalformat=GL_RGB;
				format=GL_GREEN;
				break;
		case 3: internalformat=GL_RGB;
				format=GL_BLUE;
				break;
		case 4: internalformat=GL_RGBA;
				format=GL_ALPHA;
				break;
		case 7: internalformat=GL_RGB8;
				format=GL_RGB;
				break;
		case 8: internalformat=GL_RGBA8;
				format=GL_RGBA;
				break;
		case 9: internalformat=GL_TEXTURE_DEPTH_TYPE_ARB;
				format=GL_TEXTURE_DEPTH_TYPE_ARB;
				//alter_options=TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_X|TEXTURE_CLAMP_Y;
				break;
		case 10:internalformat=GL_LUMINANCE16_ALPHA16;
				format=GL_LUMINANCE_ALPHA;
				alter_options=TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_X|TEXTURE_CLAMP_Y;
				break;
		case 11:internalformat=GL_RGBA8;
				format=GL_RGBA;
				alter_options=TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_X|TEXTURE_CLAMP_Y;
				break;
		case 12:internalformat=GL_LUMINANCE32F_ARB;
				format=GL_RED;
				alter_options=TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_X|TEXTURE_CLAMP_Y;
				break;
		case 13:internalformat=GL_RGBA32F_ARB;
				format=GL_RGBA;
				alter_options=TEXTURE_MAG_NEAREST|TEXTURE_MIN_NEAREST|TEXTURE_CLAMP_X|TEXTURE_CLAMP_Y;
				break;
		default:
				internalformat=GL_LUMINANCE;
				format=GL_LUMINANCE;
				break;
	};
}

template <int dim> 
UINT RenderWindow<dim>::CreateTexture(int w,int h,int options,int channel) {
	GLenum format;
	GLint internalformat;

	Activate();
	get_format(channel,format,internalformat,options);
	UINT texture=create_texture(options);


	glTexImage2D(GL_TEXTURE_2D,0,internalformat,w,h,0,format,GL_UNSIGNED_BYTE,NULL); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
UINT RenderWindow<dim>::LoadTexture(char *FileName,int options) {
	Bitmap<ColorRGB> Img(FileName);

	Activate();
   	UINT texture=create_texture(options);

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,3,Img.width,Img.height,GL_RGB,GL_UNSIGNED_BYTE,Img.getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,Img.width,Img.height,0,GL_RGB,GL_UNSIGNED_BYTE,Img.getBuffer()); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
UINT RenderWindow<dim>::LoadTexture(Bitmap<ColorRGBA> *img,int options) {
	if (img==NULL) return INVALID_TEXTURE;

	Activate();
	UINT texture=create_texture(options);

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,img->width,img->height,GL_RGBA,GL_UNSIGNED_BYTE,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->width,img->height,0,GL_RGBA,GL_UNSIGNED_BYTE,img->getBuffer()); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
UINT RenderWindow<dim>::LoadTexture(Bitmap<GreyLevel> *img,int options,int channel) {
	if (img==NULL) return INVALID_TEXTURE;
	
	GLenum format;
	GLint internalformat;

	Activate();
	get_format(channel,format,internalformat,options);
	UINT texture=create_texture(options);
	

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,internalformat,img->width,img->height,format,GL_UNSIGNED_BYTE,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,internalformat,img->width,img->height,0,format,GL_UNSIGNED_BYTE,img->getBuffer()); 
		
	DeActivate();	
	return texture;
}


template <int dim> 
void RenderWindow<dim>::UpdateTexture(UINT &id,Bitmap<GreyLevel> *img,int options,int channel) {
	if (id==INVALID_TEXTURE) {
		id=LoadTexture(img,options,channel);
		return;
	}
	if (img==NULL) {
		DeleteTexture(id);
		return;
	}
	
	GLenum format;
	GLint internalformat;

	Activate();
	get_format(channel,format,internalformat,options);
	glBindTexture(GL_TEXTURE_2D,id);

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,internalformat,img->width,img->height,format,GL_UNSIGNED_BYTE,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,internalformat,img->width,img->height,0,format,GL_UNSIGNED_BYTE,img->getBuffer()); 
		
	DeActivate();	
}

template <int dim> 
UINT RenderWindow<dim>::LoadTexture(Bitmap<ColorFloat> *img,int options,int channel) {
	if (img==NULL) return INVALID_TEXTURE;
	
	GLenum format;
	GLint internalformat;

	Activate();
	get_format(channel,format,internalformat,options);
	UINT texture=create_texture(options);


	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,internalformat,img->width,img->height,format,GL_FLOAT,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,internalformat,img->width,img->height,0,format,GL_FLOAT,img->getBuffer()); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
void RenderWindow<dim>::SetUnPackAlignment(int bytes) {
	Activate();
	glPixelStorei(GL_UNPACK_ALIGNMENT,bytes);
	DeActivate();
}

template <int dim> 
UINT RenderWindow<dim>::LoadTexture(Bitmap<ColorRGB> *img,int options) {
	if (img==NULL) return INVALID_TEXTURE;

	Activate();
	UINT texture=create_texture(options);

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,img->width,img->height,GL_RGB,GL_UNSIGNED_BYTE,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,img->width,img->height,0,GL_RGB,GL_UNSIGNED_BYTE,img->getBuffer()); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
void RenderWindow<dim>::UpdateTexture(UINT &id,Bitmap<ColorRGB> *img,int options) {
	if (id==INVALID_TEXTURE) {
		id=LoadTexture(img,options);
		return;
	}
	if (img==NULL) {
		DeleteTexture(id);
		return;
	}
	

	Activate();
	glBindTexture(GL_TEXTURE_2D,id);

	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,img->width,img->height,GL_RGB,GL_UNSIGNED_BYTE,img->getBuffer());
	else glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,img->width,img->height,0,GL_RGB,GL_UNSIGNED_BYTE,img->getBuffer()); 
		
	DeActivate();	
}


template <int dim> 
UINT RenderWindow<dim>::LoadTexture(BYTE *bgr_data,int w,int h,int options) {
	Activate();
	UINT texture=create_texture(options);


	if (options&TEXTURE_BUILD_MIPMAPS) gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,w,h,GL_BGR_EXT,GL_UNSIGNED_BYTE,bgr_data);
	else glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,bgr_data); 
		
	DeActivate();	
	return texture;
}

template <int dim> 
void RenderWindow<dim>::DeleteTexture(UINT &index) {
	GLuint textures[1];
	textures[0]=index;
	
	if (index!=INVALID_TEXTURE) {
		Activate();
		glDeleteTextures(1,textures);
		DeActivate();
		index=INVALID_TEXTURE;
	}
}





















template <int dim> 
void RenderWindow<dim>::Redraw() {
	draw();
}

template <int dim> 
void RenderWindow<dim>::RedrawAlternative(void(*alt_display)(void*),Bitmap<ColorRGB> *I,void *alt_userdata) {
	if (EnableDraw_) ErrorExit("Cannot use RedrawAlternative when EnableDraw is true");
	Activate();
	  draw_opengl(alt_display,alt_userdata);
	  glFlush();
	  glReadPixels(0,0,I->width,I->height,GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer());
	  I->Flip_Vertical();
	DeActivate();
}

template <int dim> 
void RenderWindow<dim>::RedrawAlternative(void(*alt_display)(void*),Bitmap<GreyLevel> *I,int channel,void *alt_userdata) {
	if (EnableDraw_) ErrorExit("Cannot use RedrawAlternative when EnableDraw is true");
	GLenum format;

	switch(channel) {
		case 0: {
					Bitmap<ColorRGB> tmp(I->width,I->height);
					RedrawAlternative(alt_display,&tmp,alt_userdata);
					I->CopyFrom(&tmp);
				}
				return;
		case 1: format=GL_RED;
				break;
		case 2: format=GL_GREEN;
				break;
		case 3: format=GL_BLUE;
				break;
		case 4: format=GL_ALPHA;
				break;
		case 5: format=GL_DEPTH_COMPONENT;
				break;
		default:
				format=GL_RED;
				break;
	};

	Activate();
	  draw_opengl(alt_display,alt_userdata);
	  glFlush();
	  glReadPixels(0,0,I->width,I->height,format,GL_UNSIGNED_BYTE,I->getBuffer());	
	  I->Flip_Vertical();
	DeActivate();
}

template <int dim> 
void RenderWindow<dim>::RedrawAlternative(void(*alt_display)(void*),Bitmap<ColorFloat> *I,int channel,void *alt_userdata) {
	if (EnableDraw_) ErrorExit("Cannot use RedrawAlternative when EnableDraw is true");
	GLenum format;

	switch(channel) {
		case 0: {
					Bitmap<ColorRGB> tmp(I->width,I->height);
					RedrawAlternative(alt_display,&tmp,alt_userdata);
					I->CopyFrom(&tmp);
				}
				return;
		case 1: format=GL_RED;
				break;
		case 2: format=GL_GREEN;
				break;
		case 3: format=GL_BLUE;
				break;
		case 4: format=GL_ALPHA;
				break;
		case 5: format=GL_DEPTH_COMPONENT;
				break;
		default:
				format=GL_RED;
				break;
	};

	Activate();
	  draw_opengl(alt_display,alt_userdata);
	  glFlush();
	  glReadPixels(0,0,I->width,I->height,format,GL_FLOAT,I->getBuffer());	
	  I->Flip_Vertical();
	DeActivate();
}




template <int dim> 
void RenderWindow<dim>::RetrieveImage(Bitmap<ColorRGB> *I) {
	Activate();
	GetBuffer(I);
	I->Flip_Vertical();
	DeActivate();
}

template <int dim> 
void RenderWindow<dim>::RetrieveImage(Bitmap<GreyLevel> *I,int channel) {
	Activate();
	GetBuffer(I,channel);
	I->Flip_Vertical();
	DeActivate();
}

template <int dim> 
void RenderWindow<dim>::RetrieveImage(Bitmap<ColorFloat> *I,int channel) {
	Activate();
	GetBuffer(I,channel);
	I->Flip_Vertical();
	DeActivate();
}




























//***********************************************************************************************************************
//
//							Gestione Modo non Interattivo
//
//***********************************************************************************************************************
template <int dim> 
void RenderWindow<dim>::Convert_Intrinsic(Matrix *Intrinsic,int width,int height,double clip_min,double clip_max,Matrix *ProjectionMatrix) {
	Matrix Int(3,3),Scale(3,3);
	Scale.SetIdentity();
	
	Int.Set(Intrinsic);

	Scale[0][0]=width/2.0;
	Scale[1][1]=height/2.0;
	Scale[0][2]=width/2.0;
	Scale[1][2]=height/2.0;
	Scale.Inversion();
	PreMultiply(&Scale,&Int);

	ProjectionMatrix->SetZero();
	(*ProjectionMatrix)[0][0]=Int[0][0];
	(*ProjectionMatrix)[1][1]=Int[1][1];
	(*ProjectionMatrix)[0][2]=-Int[0][2];
	(*ProjectionMatrix)[1][2]=Int[1][2];
	(*ProjectionMatrix)[2][2]=(clip_min+clip_max)/(clip_min-clip_max);
	(*ProjectionMatrix)[3][2]=-1;
	(*ProjectionMatrix)[2][3]=(2*clip_max*clip_min)/(clip_min-clip_max);
}

template <int dim> 
void RenderWindow<dim>::Extrinsic2ModelViewMatrix(Matrix *E,Matrix *model_view_matrix) {
	Matrix Ext_(3,4);
	Ext_.Set(E);

	Invert3x4Matrix(&Ext_);	
	(*model_view_matrix)[0][0]=-Ext_[0][0];
	(*model_view_matrix)[1][0]=Ext_[1][0];
	(*model_view_matrix)[2][0]=Ext_[2][0];

	(*model_view_matrix)[0][1]=Ext_[0][1];
	(*model_view_matrix)[1][1]=-Ext_[1][1];
	(*model_view_matrix)[2][1]=-Ext_[2][1];

	(*model_view_matrix)[0][2]=Ext_[0][2];
	(*model_view_matrix)[1][2]=-Ext_[1][2];
	(*model_view_matrix)[2][2]=-Ext_[2][2];

	(*model_view_matrix)[0][3]=Ext_[0][3];
	(*model_view_matrix)[1][3]=-Ext_[1][3];
	(*model_view_matrix)[2][3]=-Ext_[2][3];

	(*model_view_matrix)[3][0]=0.0;
	(*model_view_matrix)[3][1]=0.0;
	(*model_view_matrix)[3][2]=0.0;
	(*model_view_matrix)[3][3]=1.0;
}

template <int dim> 
void RenderWindow<dim>::ModelViewMatrix2Extrinsic(Matrix *ModelView,Matrix *Extrinsic) {
	Matrix Ext_(3,4);

	Ext_[0][0]=-(*ModelView)[0][0];
	Ext_[1][0]=(*ModelView)[1][0];
	Ext_[2][0]=(*ModelView)[2][0];
	Ext_[0][1]=(*ModelView)[0][1];
	Ext_[1][1]=-(*ModelView)[1][1];
	Ext_[2][1]=-(*ModelView)[2][1];
	Ext_[0][2]=(*ModelView)[0][2];
	Ext_[1][2]=-(*ModelView)[1][2];
	Ext_[2][2]=-(*ModelView)[2][2];
	Ext_[0][3]=(*ModelView)[0][3];
	Ext_[1][3]=-(*ModelView)[1][3];
	Ext_[2][3]=-(*ModelView)[2][3];
	Invert3x4Matrix(&Ext_);	

	GtoG(&Ext_,Extrinsic);
}

template <> 
void RenderWindow<3>::ConvertEtoLocalCoords(Matrix *wanted_extrinsic,Matrix *extrinsic_to_lock_in) {
	Matrix P(4,4),LocalCoords(4,4),tmp(4,4);

	EtoP(wanted_extrinsic,&P);

	LocalCoords.SetIdentity();
	Matrix_scale(&LocalCoords,VECTOR(1/LocalCoordsRadius,1/LocalCoordsRadius,1/LocalCoordsRadius));
	Matrix_Translate(&LocalCoords,-1.0*LocalCoordsCenter);
	LocalCoords.Inversion();

	Multiply(&P,&LocalCoords,&tmp);
	GtoG(&tmp,extrinsic_to_lock_in);
}


template <> 
void RenderWindow<3>::SetInitialModelViewMatrix(Matrix *P) {
	
	TMatrix.SetIdentity();
	TMatrix[0][0]=(*P)[0][0];
	TMatrix[0][1]=(*P)[0][1];
	TMatrix[0][2]=(*P)[0][2];
	TMatrix[1][0]=(*P)[1][0];
	TMatrix[1][1]=(*P)[1][1];
	TMatrix[1][2]=(*P)[1][2];
	TMatrix[2][0]=(*P)[2][0];
	TMatrix[2][1]=(*P)[2][1];
	TMatrix[2][2]=(*P)[2][2];
	
	eye[0]=-(*P)[0][3];
	eye[1]=-(*P)[1][3];
	eye[2]=-(*P)[2][3];
	
	Savedeye=eye;
	SavedTMatrix.Set(&TMatrix);
}

template <> 
void RenderWindow<3>::ReadInitialModelViewMatrix(Matrix *P) {
	if (P->c!=4) ErrorExit("Not an 3x4 or a 4x4 matrix");
	if ((P->r!=3) && (P->r!=4)) ErrorExit("Not an 3x4 or a 4x4 matrix");

	(*P)[0][0]=TMatrix[0][0];
	(*P)[0][1]=TMatrix[0][1];
	(*P)[0][2]=TMatrix[0][2];
	(*P)[1][0]=TMatrix[1][0];
	(*P)[1][1]=TMatrix[1][1];
	(*P)[1][2]=TMatrix[1][2];
	(*P)[2][0]=TMatrix[2][0];
	(*P)[2][1]=TMatrix[2][1];
	(*P)[2][2]=TMatrix[2][2];
	
	(*P)[0][3]=-eye[0];
	(*P)[1][3]=-eye[1];
	(*P)[2][3]=-eye[2];
	
	if (P->r==4) {
		(*P)[3][0]=0.0;
		(*P)[3][1]=0.0;
		(*P)[3][2]=0.0;
		(*P)[3][3]=1.0;
	}
}


template <> 
void RenderWindow<3>::Correct4CurrentEyeCoordinate(double eye_distance) {
	if (display_buffer!=0) {
		//
		// Get Extrinsic
		//
		Matrix P(4,4),Ext_(3,4);
		ReadModelViewMatrix(&P);

		if (UseLocalCoords) {
			Matrix LocalCoords(4,4),tmp(4,4);

			LocalCoords.SetIdentity();
			Matrix_scale(&LocalCoords,VECTOR(1/LocalCoordsRadius,1/LocalCoordsRadius,1/LocalCoordsRadius));
			Matrix_Translate(&LocalCoords,-1.0*LocalCoordsCenter);

			Multiply(&P,&LocalCoords,&tmp);
			P.Set(&tmp);
		} 

		Ext_[0][0]=-P[0][0];
		Ext_[1][0]=P[1][0];
		Ext_[2][0]=P[2][0];
		Ext_[0][1]=P[0][1];
		Ext_[1][1]=-P[1][1];
		Ext_[2][1]=-P[2][1];
		Ext_[0][2]=P[0][2];
		Ext_[1][2]=-P[1][2];
		Ext_[2][2]=-P[2][2];
		Ext_[0][3]=P[0][3];
		Ext_[1][3]=-P[1][3];
		Ext_[2][3]=-P[2][3];
		

		Matrix E(3,4);
		Vector<3> Direction;

		E.Set(&Ext_);
		E.GetColumn(0,Direction.GetList());
		Direction=-1.0*Direction;
		E.SetColumn(0,Direction.GetList());
		Invert3x4Matrix(&E);
		
		// E = [X,Y,Z,T]

		Vector<3> X,O;
		E.GetColumn(3,O.GetList());
		E.GetColumn(0,X.GetList());

		O=O+(eye_distance*X);
		E.SetColumn(3,O.GetList());

		//
		ExttoE(&E,&Ext_);
		SetExtrinsic(&Ext_);
	}
}


template <> 
void RenderWindow<3>::SetExtrinsic(Matrix *Extrinsic) {
	Matrix model_view_matrix(4,4);

	if (UseLocalCoords) {
		Matrix tmp(4,4);
		RenderWindow<3>::Extrinsic2ModelViewMatrix(Extrinsic,&tmp);
		ConvertEtoLocalCoords(&tmp,&model_view_matrix);
	} else {
		RenderWindow<3>::Extrinsic2ModelViewMatrix(Extrinsic,&model_view_matrix);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(model_view_matrix[0][3],model_view_matrix[1][3],model_view_matrix[2][3]);
	model_view_matrix[0][3]=0.0;
	model_view_matrix[1][3]=0.0;
	model_view_matrix[2][3]=0.0;
	double Rotazione[16];
	model_view_matrix.GetCM(Rotazione);
	glMultMatrixd(Rotazione);
}

template <> 
void RenderWindow<3>::GetExtrinsic(Matrix *Extrinsic) {
	Matrix P(4,4),Ext_(3,4);

	ReadModelViewMatrix(&P);

	if (UseLocalCoords) {
		Matrix LocalCoords(4,4),tmp(4,4);

		LocalCoords.SetIdentity();
		Matrix_scale(&LocalCoords,VECTOR(1/LocalCoordsRadius,1/LocalCoordsRadius,1/LocalCoordsRadius));
		Matrix_Translate(&LocalCoords,-1.0*LocalCoordsCenter);

		Multiply(&P,&LocalCoords,&tmp);
		P.Set(&tmp);
	} 

	Ext_[0][0]=-P[0][0];
	Ext_[1][0]=P[1][0];
	Ext_[2][0]=P[2][0];
	Ext_[0][1]=P[0][1];
	Ext_[1][1]=-P[1][1];
	Ext_[2][1]=-P[2][1];
	Ext_[0][2]=P[0][2];
	Ext_[1][2]=-P[1][2];
	Ext_[2][2]=-P[2][2];
	Ext_[0][3]=P[0][3];
	Ext_[1][3]=-P[1][3];
	Ext_[2][3]=-P[2][3];
	Invert3x4Matrix(&Ext_);	

	GtoG(&Ext_,Extrinsic);
}

template <> 
Vector<3> RenderWindow<3>::GetProjectionCenter() {
	Matrix Extrinsic(3,4),E(3,4);
	Vector<3> center_of_projection;

	GetExtrinsic(&Extrinsic);
	ExttoE(&Extrinsic,&E);
	
	E.GetColumn(3,center_of_projection.GetList());

	return center_of_projection;
}

template <> 
void RenderWindow<3>::SetInitialExtrinsic(Matrix *Extrinsic) {
	Matrix model_view_matrix(4,4);

	if (UseLocalCoords) {
		Matrix tmp(4,4);
		RenderWindow<3>::Extrinsic2ModelViewMatrix(Extrinsic,&tmp);
		ConvertEtoLocalCoords(&tmp,&model_view_matrix);
	} else {
		RenderWindow<3>::Extrinsic2ModelViewMatrix(Extrinsic,&model_view_matrix);
	}

	SetInitialModelViewMatrix(&model_view_matrix);

}

template <> 
void RenderWindow<3>::ReadInitialExtrinsic(Matrix *Extrinsic) {
	Matrix model_view_matrix(4,4);

	ReadInitialModelViewMatrix(&model_view_matrix);

	if (UseLocalCoords) {
		NOT_IMPLEMENTED;
	} else {
		RenderWindow<3>::ModelViewMatrix2Extrinsic(&model_view_matrix,Extrinsic);
	}
}

template <> 
void RenderWindow<3>::LockInMatrix(Matrix *Extrinsic) {
	SetInitialExtrinsic(Extrinsic);

	// this->lock=lock; // TODO** forse nn serve neppure questa variabile
	// salva lo stato prima
	// EnableStandardMouseController(false);
	UpdateClient(hWnd);
}


template <> 
void RenderWindow<2>::LockInMatrix(Matrix *Extrinsic) {
	NOT_IMPLEMENTED;
}

template <int dim> 
void RenderWindow<dim>::UnLock() {
	this->lock=false;
	// EnableStandardMouseController(stato precedente );
	UpdateClient(hWnd);
}

template <int dim> 
void RenderWindow<dim>::LockWindowSize(int width,int height) {
	SetClientSize(hWnd,width,height);
	w=width;h=height;

	// TODO: bloccare il resizing della window
}


template <int dim> 
void RenderWindow<dim>::LockProjectionMatrix(Matrix *Intrinsic,int width,int height) {
	
	RenderWindow<dim>::Convert_Intrinsic(Intrinsic,width,height,clipping_min,clipping_max,&LockingProjectionMatrix);
	
	lock_p=true;
	UpdateClient(hWnd);
}


template <int dim> 
void RenderWindow<dim>::Retrieve_OGLState_From_Calibration(Matrix *Intrinsic,Matrix *Extrinsic,int width,int height,OpenGL_State *ogl,double clip_min,double clip_max) {
	Matrix projection_matrix(4,4),model_view_matrix(4,4);
	
	if ((clip_min==0) && (clip_max==0)) {clip_min=0.1;clip_max=1000.0;}
		
	RenderWindow<dim>::Convert_Intrinsic(Intrinsic,width,height,clip_min,clip_max,&projection_matrix);
	RenderWindow<3>::Extrinsic2ModelViewMatrix(Extrinsic,&model_view_matrix);

	PtoGL(&projection_matrix,ogl->Projection);
	PtoGL(&model_view_matrix,ogl->ModelView);
	ogl->ViewPort[0]=0;
	ogl->ViewPort[1]=0;
	ogl->ViewPort[2]=width;
	ogl->ViewPort[3]=height;
}












//***********************************************************************************************************************
//
//							Primitive Opengl Indipendenti
//
//***********************************************************************************************************************

void SetProjectionMatrix(Matrix *I) {
	bool valid=false;
	GLdouble m[16];

	if ((I->r==4) && (I->c==4)) {PtoGL(I,m);valid=true;}
	if ((I->r==3) && (I->c==4)) {
		Matrix P(4,4);
		EtoP(I,&P);
		P[3][0]=0;
		P[3][1]=0;
		P[3][2]=-1;
		P[3][3]=0;
		PtoGL(&P,m);
		valid=true;
	}
	if (!valid) ErrorExit("Not an 3x4 or a 4x4 matrix");
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixd(m);
}

void SetModelViewMatrix(Matrix *P) {
	GLdouble m[16];

	PtoGL(P,m);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixd(m);
}

void ReadModelViewMatrix(Matrix *P) {
	int i,j;
	GLdouble m[16];

	glGetDoublev(GL_MODELVIEW_MATRIX,m);

	for(i=0;i<4;i++)
		for(j=0;j<3;j++)
			(*P)[j][i]=m[i*4+j];

	if (P->r>3) {
		j=3;
		for(i=0;i<4;i++) (*P)[j][i]=m[i*4+j];
	}
}

void ReadProjectionMatrix(Matrix *P) {
	int i,j;
	GLdouble m[16];

	glGetDoublev(GL_PROJECTION_MATRIX,m);

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			(*P)[j][i]=m[i*4+j];

	if (P->r>3) {
		j=3;
		for(i=0;i<4;i++) (*P)[j][i]=m[i*4+j];
	}
	if (P->c>3) {
		i=3;
		for(j=0;j<4;j++) (*P)[j][i]=m[i*4+j];
	}
}

void ReadOpenGLState(OpenGL_State *ogl) {
	glGetDoublev(GL_MODELVIEW_MATRIX,ogl->ModelView);
	glGetDoublev(GL_PROJECTION_MATRIX,ogl->Projection);
	glGetIntegerv(GL_VIEWPORT,ogl->ViewPort);
}

bool SetDisplayResolution_FullScreen(int w,int h,int bits,int refresh_freq) {
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
	dmScreenSettings.dmSize=sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth=w;
	dmScreenSettings.dmPelsHeight=h;
	dmScreenSettings.dmBitsPerPel=bits;
	dmScreenSettings.dmDisplayFrequency=refresh_freq;
	dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
	if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) return false;
	return true;
}
void ResetDisplayResolution_FullScreen() {
	ChangeDisplaySettings(NULL,0);
}


void Matrix_Translate(Matrix *P,Vector<3> T) {
	Matrix tmp(4,4),original(4,4);
	tmp.SetIdentity();
	tmp[0][3]=T[0];
	tmp[1][3]=T[1];
	tmp[2][3]=T[2];
	original.Set(P);

	Multiply(&original,&tmp,P);
}

void Matrix_Translate(Matrix *P,Matrix *T) {
	Multiply(P,T);
}

void Matrix_scale(Matrix *P,Vector<3> S) {
	Matrix tmp(4,4),original(4,4);
	tmp.SetIdentity();
	tmp[0][0]=S[0];
	tmp[1][1]=S[1];
	tmp[2][2]=S[2];
	original.Set(P);

	Multiply(&original,&tmp,P);
}



Vector<2> Project(OpenGL_State *ogl,Vector<3> p,bool opengl_coords) {
	Vector<2> v;
	GLdouble x,y,z;

	gluProject(p[0],p[1],p[2],ogl->ModelView,ogl->Projection,ogl->ViewPort,&x,&y,&z);
	
	
	if (opengl_coords) {
		v[0]=x/ogl->ViewPort[2];
		v[1]=1.0-(y/ogl->ViewPort[3]);
	} else {
		v[0]=x;
		v[1]=y;
	}

	return v;
}

Vector<3> Project3(OpenGL_State *ogl,Vector<3> p,bool opengl_coords) {
	Vector<3> v;
	GLdouble x,y,z;

	gluProject(p[0],p[1],p[2],ogl->ModelView,ogl->Projection,ogl->ViewPort,&x,&y,&z);
	
	
	if (opengl_coords) {
		v[0]=x/ogl->ViewPort[2];
		v[1]=1.0-(y/ogl->ViewPort[3]);
	} else {
		v[0]=x;
		v[1]=y;
	}
	v[2]=z;

	return v;
}

void UnProject(OpenGL_State *ogl,Vector<2> p,bool p_in_opengl_coords,Vector<3> *o,Vector<3> *d) {
	GLdouble x,y;

	if (p_in_opengl_coords) {
		x=ogl->ViewPort[2]*p[0];
		y=ogl->ViewPort[3]*(1.0-p[1]);
	} else {
		x=p[0];
		y=p[1];
	}
	
	
	// Trova il centro di proiezione
	Matrix ModelView(4,4),Extrinsic(3,4),E(3,4);
	GLtoP(ogl->ModelView,&ModelView);
					// TODO**: use Local Coords?? Retrieve_OGLState_From_Calibration(...) lo assume false!!!!
	RenderWindow<3>::ModelViewMatrix2Extrinsic(&ModelView,&Extrinsic);			
	ExttoE(&Extrinsic,&E);
	E.GetColumn(3,o->GetList());


	GLdouble px,py,pz;
	gluUnProject(x,y,(GLdouble)1.0,ogl->ModelView,ogl->Projection,ogl->ViewPort,&px,&py,&pz);
	(*d)[0]=px-(*o)[0];
	(*d)[1]=py-(*o)[1];
	(*d)[2]=pz-(*o)[2];
	(*d)=d->Versore();
}

void ProjectVector(OpenGL_State *ogl,Vector<3> o3,Vector<3> v3,Vector<2> *o2,Vector<2> *v2,bool opengl_coords,bool o2_isknown) {

	if (!o2_isknown) (*o2)=Project(ogl,o3,opengl_coords);
	(*v2)=Project(ogl,(o3+v3),opengl_coords)-(*o2);
}


template<int dim>
Vector<dim> Transform(Matrix *R,Vector<dim> origin,Vector<dim> p) {
	Vector<dim> p_t;
	
	Multiply(R,p,p_t.GetList());
	return (p_t+origin);
}

template<int dim>
Vector<dim> Transform(Matrix *RT,Vector<dim> p) {
	Matrix R(dim,dim);
	Vector<dim> T;

	RT->GetMinor(dim,dim,&R);
	RT->GetColumn(dim,T.GetList());

	return Transform(&R,T,p);
}

template<int dim>
Vector<dim> TransformVector(Matrix *RT,Vector<dim> v) {
	Matrix R(dim,dim);
	Vector<dim> T;

	RT->GetMinor(dim,dim,&R);
	T.SetZero();

	return Transform(&R,T,v);
}

void SetAttrib(GLenum cap,GLboolean state) {
	if (state) glEnable(cap);
	else glDisable(cap);
}

void SetStandardOGLEnvironment(bool use_depth,bool use_stencil,bool clear_buffer) {
	int buffer_to_clear=GL_COLOR_BUFFER_BIT;
	if (use_stencil) {
		glClearStencil(0);
		buffer_to_clear|=GL_STENCIL_BUFFER_BIT;
	}
	if (use_depth) buffer_to_clear|=GL_DEPTH_BUFFER_BIT;
	glClearColor(0.0,0.0,0.0,0.0);
	if (clear_buffer) glClear(buffer_to_clear);
	
	if (use_depth) {
		glEnable(GL_DEPTH_TEST);
		glDepthRange(0.0,1.0);
		glDepthFunc(GL_LESS);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
	
	if (use_stencil) {
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	} else {
		glDisable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	}

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_CULL_FACE);								// Cull Face
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);			// Fill poligons
	glDisable(GL_LIGHTING);								// No light
	glEnable(GL_SMOOTH);						
	glShadeModel(GL_SMOOTH);							// Smooth rendering
	glDisable(GL_BLEND);								// Blend
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0);									// Lines
	glDisable(GL_TEXTURE_2D);							// Textures
	glEnable(GL_NORMALIZE);								// Normals
}


template Vector<3> Transform(Matrix *R,Vector<3> origin,Vector<3> p);
template Vector<2> Transform(Matrix *R,Vector<2> origin,Vector<2> p);
template Vector<3> TransformVector(Matrix *RT,Vector<3> v);
template Vector<2> TransformVector(Matrix *RT,Vector<2> v);
template Vector<3> Transform(Matrix *RT,Vector<3> p);
template Vector<2> Transform(Matrix *RT,Vector<2> p);







void GetNearFarPlane(HyperMesh<3> *mesh,Matrix *Ext,double &min_z,double &max_z) {
	Vector<3> *P=mesh->Points.getMem();
	double a,b,c,d;
	a=(*Ext)[2][0];
	b=(*Ext)[2][1];
	c=(*Ext)[2][2];
	d=(*Ext)[2][3];
	min_z=DBL_MAX;
	max_z=-DBL_MAX;

	for(int i=mesh->num_p;i>0;i--,P++) {
		double v=a*((*P)[0])+b*((*P)[1])+c*((*P)[2])+d;
		max_z=max(max_z,v);
		min_z=min(min_z,v);
	}
}





