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





// NB: 
//
// - ERRORI FREQUENTI:
//				- Usare Activate(), DeActivate() per accedere all'Opengl
//
// - command return true if it has to be redraw
//
//


#ifdef SYSTEM_WINDOWS


#define DRAW_EDGES				 2
#define DRAW_FILLED              1


#define TEXTURE_BUILD_MIPMAPS    1
#define TEXTURE_REPEAT_X         0
#define TEXTURE_REPEAT_Y         0
#define TEXTURE_CLAMP_X          2
#define TEXTURE_CLAMP_Y          4
#define TEXTURE_CLAMP_EDGE_X    32
#define TEXTURE_CLAMP_EDGE_Y    64
#define TEXTURE_MIN_LINEAR       0
#define TEXTURE_MIN_NEAREST      8
#define TEXTURE_MAG_LINEAR       0
#define TEXTURE_MAG_NEAREST     16

#define TEXTURE_CH_LUMINANCE	 0
#define TEXTURE_CH_LUMINANCE8	 0
#define TEXTURE_CH_LUMINANCE16	 6
#define TEXTURE_CH_RED			 1
#define TEXTURE_CH_GREEN		 2
#define TEXTURE_CH_BLUE			 3
#define TEXTURE_CH_ALPHA		 4
#define TEXTURE_CH_DEPTH		 5
#define TEXTURE_CH_RGB			 7
#define TEXTURE_CH_RGBA			 8
#define TEXTURE_CH_DEPHT		 9
#define TEXTURE_CH_16_16		10
#define TEXTURE_CH_8_8_8_8		11
#define TEXTURE_CH_32			12
#define TEXTURE_CH_32_32_32_32	13
#define TEXTURE_CH_24			 9			// TODO: presumo sia 24 bit!!


#define RENDERWINDOW_FULLSCREEN			  1 
#define RENDERWINDOW_VSYNC				  2
#define RENDERWINDOW_STEREO				  4
#define RENDERWINDOW_DO_NOT_SHOW_INFO	  8
#define RENDERWINDOW_DBLCLICK			 16


// constant
#define MAX_NUMBER_TEXTURE_PARAMETERIZATIONS	20



template <int dim> 
LRESULT WINAPI RenderWindow_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct mouse_input_t {
  int buttons[3];
  int x, y;
};

struct OpenGL_State {
	GLdouble Projection[16];
	GLdouble ModelView[16];
	GLint ViewPort[4];
};

#define USED_CHARACTERS_NUMBER 96
struct FontSize {
	int size_x,size_y,step_x,delta_x;
};

class Shader;

//
//
// MULTI-TASK: Implementato e (da testare ma dovrebbe funzionare)
//				* Il rendering context (activate and deactivate) deve essere usato da un thread alla volta		(gestito automaticamente dalla classe)
//			    * E' consigliabile avere 2 finestre (magari virtuali) per il multiprocessing nel caso
//				  non si debbano condividere texture o buffer tra i processi. (cosi' l'uno non deve aspettare
//				  l'altro durante un rendering).
//
//

template <int dim> 
class RenderWindow {
	friend LRESULT WINAPI RenderWindow_WindowProc<dim>(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend class Shader;

protected:
	// Windows Property
	bool virtual_window;
	HWND hWnd;
	HGLRC hRC;
	HDC hDC;
	int w,h;
	float Background_Color_r,Background_Color_g,Background_Color_b,Background_Color_a;
	unsigned int fontbase;
	FontSize font_size[USED_CHARACTERS_NUMBER];
	Vector<dim> SceneCenter;
	double SceneInverseDiameter;
	bool EnableDraw_;
	double clipping_min,clipping_max;
	bool UseLocalCoords;
	int DepthBits;
	int StencilBits;
	bool stereo_view;
		int display_buffer;
		void(*display_right)(void*);
		void *userdata_right;
	bool fullscreen;


	// Spoiler Screen
		int SpoilerScreenBehaviour;
		clock_t init_time;
		void DrawInitScreen();


	// Interactions
	bool lock,lock_p;
	Vector<3> eye,Savedeye;
	Matrix TMatrix;
	Matrix SavedTMatrix;
	Matrix LockingProjectionMatrix;
	mouse_input_t mouse;
	void SetInitialModelViewMatrix (Matrix *P);						// Setta/Legge TMatrix & eye: la ModelViewMatrix durante 
	void ReadInitialModelViewMatrix(Matrix *P);						// la visualizzatore sara' uguale alla matrice *P. Accetta (3x4) o (4x4)


	// Windows Update on interaction
	bool StandardMouseController;
	HyperBox<2> MouseControllerArea;
	bool InsideControllerArea(float x,float y);


	// LocalCoords
	double LocalCoordsRadius;
	Vector<dim> LocalCoordsCenter;


	// Context Property
	HDC hdcold;
	HGLRC hglrcold;

	// MutiTasking
	HANDLE CriticalSection;				// Prevent multiple task to access to the same context		   TODO: Not used!!
	int InsideContext;					// (number of nested functions called by the same task)
	bool SET_PROGRAM_0;
	void Activate();
	void DeActivate();
	


	// Internals
	ATOM InitWindowClass(int RenderWindowProperties);
	void CreateRenderWindow(char* title,
							ATOM class_n,
							HMENU hMenu,
							DWORD WindowStyles,
							bool show_rw_information);
	void draw();
	void draw_opengl(void(*display_fnc)(void*),void *userdata);
	bool palette();
	void _print(const char *text,Vector<dim> P);
	void ConvertEtoLocalCoords(Matrix *wanted_extrinsic,Matrix *extrinsic_to_lock_in);				// Converte wanted_extrinsic (3x4 o 4x4) in extrinsic_to_lock_in (3x4 o 4x4) to be used in LockInMatrix with the SetLocalCoords on	
	static void Convert_Intrinsic(Matrix *Intrinsic,int width,int height,
						          double clip_min,double clip_max,Matrix *ProjectionMatrix);		// Converte gli Intrinseci in ProjectionMatrix (4x4)


	// CallBacks
	void(*display)(void*);
	void *userdata;
	bool(*command)(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *);

public:	
	
	
	RenderWindow(char *title,
				 void(*display)(void*)=NULL,														// NULL means virtual window (always hide)
				 bool(*command)(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *)=NULL,
				 void *userdata=NULL,
				 int Background_Color=0x000000,
				 HMENU hMenu=NULL,
				 DWORD WindowStyle=WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
				 int RenderWindowProperties=0);
	~RenderWindow();
	
	//
	// Glossario:
	//
	// (NV)  valida solo se la finestra non e' virtuale
	// (D)   significa usabili solo a contesto attivato
	// (ND)  significa usabili solo a contesto disattivato
	// (--)  significa usabili in ogni caso
	//



	//*************************************************************************************************************
	// Settings (ND)
	//    DefineNearFarPlane MANDATORY
	//*************************************************************************************************************
		void   DefineLocalCoords(Vector<dim> Center,double radius,									// Center and radius of the object (valid only if uselocalcoords is true)
							   bool uselocalcoords=false,double max_distance_rel=10.0);				// uselocalcoords -> comprime l'oggetto in una sfera di raggio 1 centrata in 0 
							   // uselocalcoords DEPRECATED 												// Facilita l'uso delle luci
																											// La rotazione è centrata su center altrimenti è cnetrata su 0,0,0 (Si puo' aggiustare)
																											// Non sono stati rilevati errori di approssimazione quando si fa LockInMatrix
																											// max_distance_rel massima distanza consentita dall'oggetto rispetto alla suo raggio (10 volte il suo raggio)
		double DefineLocalCoords(Vector<dim> Center,double radius,									
							   bool uselocalcoords,double distance_closest_camera_to_scene_center,	// distance_closest_camera_to_scene_center = distance of the closest camera to the scene center
							   double distance_furthest_camera_to_scene_center);					// distance_furthest_camera_to_scene_center = distance of the furthest camera to the scene center
																									// radius = max radius of the volume of the scene (if the scene is a rectangle, it will be the diagonal)
																									// return approximate accuracy on the depth of the cental element of the scene

		double DefineNearFarPlane(double near,double far);											// Return an approximate value for z_step precision (precise only if clipping_min<<clipping_max)
																									// (settare prima di LockProjectionMatrix)
		void SetRightEyeFunction(void(*display_right)(void*),void *userdata_right);


		Vector<3> GetEye();
		void      SetEye(Vector<3> x);





	//*************************************************************************************************************
	// Base Functionality
	//*************************************************************************************************************
		void EnableDraw(bool enable=true,bool control_window_visibility=true);			      // (NV) Abilità la finestra/display/controllo   (una virtual window e' sempre disabilitata)
		void Redraw();																		  // (NV) Aggiorna la finestra
		void SetLocalCoords();																  // (D)  Prima riga della funzione display() 
		
		




	//*************************************************************************************************************
	// Window properties
	//*************************************************************************************************************
		int width();					// (NV)
		int height();					// (NV)
		double aspect_ratio();			// (NV)
		double clip_min();				// (--)
		double clip_max();				// (--)

		// Low Level Interaction
		HWND GethWnd();					// (--)
		HDC  GethDC();					// (--)
		void Set_Program_0();			// (D)



	// Modo Offscreen (limitato alla risoluzione della finestra) DEPRECATED 
		// In EnableDraw(false)
		void RedrawAlternative(void(*alt_display)(void*),Bitmap<ColorRGB> *I,void *userdata=NULL);                    // chiama la funzione alt_display 1 volta 
		void RedrawAlternative(void(*alt_display)(void*),Bitmap<GreyLevel> *I,int channel,void *userdata=NULL);       // Salva il contenuto del backbuffer nel bitmap -> solo la parte 0,0,I->w,I->h   <- Posso lavorare su buffer piu' piccoli!
		void RedrawAlternative(void(*alt_display)(void*),Bitmap<ColorFloat> *I,int channel,void *userdata=NULL);	  // LUMINANCE8, Red, Green, Blue, Alpha, Depth
	// ------------------



	// Statiche
		static void Extrinsic2ModelViewMatrix(Matrix *E,Matrix *model_view_matrix);						// Converte gli Estrinseci (3x4) in ModelViewMatrix (4x4) e viceversa
		static void ModelViewMatrix2Extrinsic(Matrix *model_view_matrix,Matrix *E);						// TODO: tutto il sistema con view si puo' cambiare!!) -> connessi ci sono EtoExt e Ext2E e tutte la classe view e tutte le funzioni sotto!!
																										







	//*************************************************************************************************************
	// Funzioni che controllano lo stato iniziale degli Intrinseci e degli Estrinseci nella funzione display
	// (da chiamare esternamente a display) (ND)
	//*************************************************************************************************************
		void EnableStandardMouseController(bool StandardMouseController=true);	// Abilita/Disabilita il controllo di questi parametri da parte del mouse
		void SetMouseControllerArea(HyperBox<2> MouseControllerArea);			//			true  -> modo interattivo
																				//			false -> modo non interattivo
		// Extrinsic
		void SetInitialExtrinsic (Matrix *Extrinsic);							// (3x4) or (4x4)
		void ReadInitialExtrinsic(Matrix *Extrinsic);							// (3x4) or (4x4)
			// Lock
			void LockInMatrix(Matrix *Extrinsic);								// Locka i parametri estrinseci a *Extrinsic  (TODO: non li blocca)
			void UnLock();														// unlock extrinseci						  (TODO: non li blocca)
		// Intrinsic
		void LockWindowSize(int width,int height);								// Locka la dimensione della finestra a width x height
		void LockProjectionMatrix(Matrix *Intrinsic,int width,int height);		// Locka i parametri intrinseci a *Intrinsic. PS: sono necessarie anche le dimensioni del piano immagine relativo a *Intrinsic
																				// TODO Testato Ok senza Localcoords.. non so con Localcoords

		static void Retrieve_OGLState_From_Calibration(Matrix *Intrinsic,		 // Ritorna lo stato OGL che il sistema avra' se si settano 
			                                    Matrix *Extrinsic,int width,     // questi specifici valori di calibrazione.
												int height,OpenGL_State *ogl,    // i.e., se si chiama LockInMatrix e LockProjectionMatrix
												double clip_min=0,				 // TODO: controllare che clip_min, clip_max non siano rilevanti per Project
												double clip_max=0);	
	
	//************************************************

	
	

	//************************************************
	// Texture (ND)
	//************************************************
		void SetUnPackAlignment(int bytes);																	// Remember to set it to 1 for very small images (in rows)
		UINT LoadTexture   (char *FileName,int options=0);
		UINT LoadTexture   (Bitmap<ColorRGB> *img,int options=0);
		UINT LoadTexture   (Bitmap<ColorRGBA> *img,int options=0);
		UINT LoadTexture   (BYTE *bgr_data,int w,int h,int options=0);													// BGR data 
		UINT LoadTexture   (Bitmap<ColorFloat> *img,int options=0,int channel=TEXTURE_CH_LUMINANCE8);					// TEXTURE_CH_LUMINANCE8 force the image between 0.0 and 1.0
		UINT LoadTexture   (Bitmap<GreyLevel> *img,int options=0,int channel=TEXTURE_CH_LUMINANCE8);
		void UpdateTexture (UINT &id,Bitmap<GreyLevel> *img,int options=0,int channel=TEXTURE_CH_LUMINANCE8);			// options = only MIP_MAPS are accepted, it does not modify the original ones
		void UpdateTexture (UINT &id,Bitmap<ColorRGB> *img,int options=0);												// options = only MIP_MAPS are accepted, it does not modify the original ones
		UINT CreateTexture (int w,int h,int options,int channel);
		void DeleteTexture (UINT &index);												 
	//************************************************
	




	//************************************************
	// 2D Primitives (D)
	//************************************************
	// Parameters
		void SetColor(int color,float transparency=1.0);
		void SetMaterial(Vector<4> Diffuse_Color,float modulate_ambient=0.5f,float modulate_specular=1.0f,float shinness=100.0f);
		void SetMaterial(Vector<4> Diffuse_Color,Vector<3> Ambient_Color,Vector<3> Specular_Color,float shinness=100.0f);
		void SetLight(int index,Vector<3> pos,bool relative,float diffuse,float ambient=0.0,Vector<3> *Color=NULL);
		void SetLightOff(int index);
	// Base Primitives
		void Vertex(Vector<dim> P);
		void DrawPoint(Vector<dim> P,int Color,float size=1.0);										// Occhio Disabilitare Lighting -> Altrimenti non e' il colore giusto
		void DrawLine(Vector<dim> A,Vector<dim> B,float size=1.0);									// Occhio Disabilitare Lighting -> Altrimenti non e' il colore giusto
		void DrawLine(Vector<dim> A,Vector<dim> B,int Color,float size=1.0);						// Occhio Disabilitare Lighting -> Altrimenti non e' il colore giusto
		void DrawCircle(Vector<dim> C,double radius,int BorderColor,int FillColor,float border_size,
			            Vector<dim> dir_x,Vector<dim> dir_y,float precision=0.1);	                // Occhio Disabilitare Lighting -> Altrimenti non e' il colore giusto
		void DrawHyperMesh(HyperMesh<dim> *mesh);													// Need normal per vertex
		void DrawHyperMesh_Flat(HyperMesh<dim> *mesh);
		void DrawHyperMesh_Textured(HyperMesh<dim> *mesh);
		void DrawHyperMesh_Textured_Flat(HyperMesh<3> *mesh);
	// Print
		void SetCurrentFont(char *FontName,int FontSize);
		void GetPrintedStringSize(char *String,int width,int height,double &txt_w,double &txt_h);
		double GetPrintedStringSize(char *String,int width,int height);
		void Print(Vector<dim> P,const char *fmt, ...);
		void Print(Vector<dim> P,int color,const char *fmt, ...);
		void Print_ra_ic(Vector<dim> P,int width,int height,int color,const char *fmt, ...);		// right aligned
	// Advanced Primitives 
		void DrawHyperMesh(HyperMesh<3> *mesh,void(*vertex_properties)(RenderWindow<dim> *w,HyperMesh<dim> *mesh,int vertex_index,void *data),void *data=NULL);	// vertex_properties/face_properties 
		void DrawHyperMesh_Flat(HyperMesh<3> *mesh,void(*face_properties)(RenderWindow<dim> *w,HyperMesh<dim> *mesh,int face_index,void *data),void *data=NULL);	// setta il colore/trasparenza
		void DrawHyperBox(HyperBox<dim> B,int Color,int Axis_Color=0xFF000000);
		void DrawHyperBox(Vector<dim> center,Vector<dim> d,int Color);		// d is an half-edge
		void DrawArrow(Vector<dim> Origin,Vector<dim> Point,Vector<dim> DirArrow,int Color,float LineWidth=1.0);
		void DrawRefSystem(double scale=1.0);
		void DrawRefSystem(Matrix *RT,double scale=1.0);
		void DrawCamera(Matrix *OGL_Ext,int ID,double scale=1.0,bool draw_axis=true);				// Disegna una camera {obsoleto)
		void DrawCameraObject(Matrix *Ext,int ID,double scale=1.0,bool draw_axis=true);				// Disegna una camera
		void DrawImagePlane(Matrix *Intrinsic,Matrix *Extrinsic,									// Disegna il piano immagine di una camera
							int width,int height,UINT index,
							double distance,double transparency,
							double front_color,double back_color,bool draw_frame);

		void DrawObj(int type,Vector<dim> *C,Matrix *SRef,Vector<dim> *Scale,int n_u,int n_v,double par2);
																									// Ref  = NULL -> Use the canonical base
																									// Scale = NULL -> (1.0,1.0,1.0)
																									// C     = NULL -> (0,0,0)
		void Draw2DHyperBox(HyperBox<2> *Location,int Color,double transparency);
		void Draw2DRectangle(HyperBox<2> *Location,int color,float size,double transparency);
		void Draw3DHyperBox(Vector<dim> *C,Matrix *axes,Vector<dim> *sizes,int color,double transparency,int edge_color,int what_to_draw);
																									// C		=	center of the Box
																									// axes		=	the coloums are the axes of the box (PstoRT)
																									// sizes	=	indicate the lenghts of the box edges
																									// C		=	NULL -> (0,0,0)
																									// axes		=	NULL -> Use the canonical base
																									// sizes	=	NULL -> (1.0,1.0,1.0)
	// Texture Primitives
		void DrawTexture(UINT index,double transparency,bool invert_y=false);						// Disegna un'intera tessitura di fronte allo schermo. 
																									// La tessitura verrà stretchata per fittare l'inter schermo.
																									// Per mantenere le precise stesse dimensioni usare LockWindowSize(Tex_dimX,Tex_dimY);
																									// invert_y da' la possibilità di riflettere la tessitura up/down (utile per le tessiture estratte con glCopyTexImage2D)
		void DrawTexture(UINT index,HyperBox<2> *Location,double transparency,bool invert_y=false);
		void DrawTexture(UINT index,bool invert_y,UINT mask_index,bool mask_invert_y);				// Identico ma con maskera
		void DrawTexture_Blur(UINT index,bool invert_y,double radius);

	//************************************************







	//********************************************************
	// Coordinate R/W   (D)    (current coords during draw)
	//********************************************************
		void SetExtrinsic(Matrix *Extrinsic);							    //		Setta il distema di riferimento indicato da *Extrinsic
		void GetExtrinsic(Matrix *Extrinsic);							    //		Legge il corrente sistema di riferimento 
		Vector<3> GetProjectionCenter();									//		Legge il corrente centro di proiezione
		void ResetToOriginalCoords();										// (NV) Resetta le coordinate Int (se lock) ed Ext
		void Correct4CurrentEyeCoordinate(double eye_distance);				//      Set the coordinates for the current display_buffer
	//************************************************






	//************************************************
	// Coordinate Temporanee (D)
	//************************************************
		// Enter (Setta coordinate locali per il disegno -> tutto quello che seguira' sara' ruotato e traslato)
			void SetRT(Matrix *R,Vector<3> *T,Vector<3> *Scale=NULL);					// R     [NULL] indica il sistema di riferimento 
																						// T     [NULL] indica l'origine del sistema di riferimento
																						// Scale [NULL] indica la scala nei vari assi
			void SetRTm(Matrix *RT,Vector<3> *Scale=NULL);								// RT           indica il sistema di riferimento comprensivo di origine
			void SetCameraCoords();														// Sistema di riferimento della camera -> SetRT(I,0,1);
		// Exit
			void ClearRT();

		
		// Enter
			void SetIE(Matrix *Intrinsic,int w,int h,Matrix *Extrinsic);
			void SetImageCoords();														// Coordinate finestra [-1,+1]
			void SetImageCoords01();													// Coordinate finestra [0,+1]
		// Exit
			void ClearImageCoords();
	//************************************************






	//************************************************
	// Get/Set current readable buffer (D)   (UPSIDE DOWN BITMAPs)
	//         use glReadBuffer to specify the curren one
	//************************************************
		void GetBuffer(UINT texture_index,int internalFormat=GL_RGB);
		void GetBuffer(Bitmap<ColorRGB> *I);
		void GetBuffer(Bitmap<ColorRGBA> *I);
		void GetBuffer(Bitmap<ColorFloat> *I,int channel=0);
		void GetBuffer(Bitmap<GreyLevel> *I,int channel=0);
		void SetBuffer(Bitmap<ColorRGB> *I);
		void SetBuffer(Bitmap<ColorFloat> *I);
		UINT ReadStencilPixel(int x,int y,int height);
		void SetAlphaMask(UINT index,bool invert_y=false);								// Setta la maschera alpha dell'intera immagine ad essere uguale
																						// a quella specificata dalla tessitura index
	//************************************************


	//************************************************
	// Get/Set current readable buffer (ND)   (Correctly oriented BITMAPs)
	//         use glReadBuffer to specify the curren one
	//************************************************
	void RetrieveImage(Bitmap<ColorRGB> *I);
	void RetrieveImage(Bitmap<GreyLevel> *I,int channel=0);
	void RetrieveImage(Bitmap<ColorFloat> *I,int channel=0);
	//************************************************






	// Funzioni di vertex/face painting predefinite
	static void vertex_color_normal(RenderWindow<dim> *w,HyperMesh<dim> *mesh,int vertex_index,void *vector_origine);  
																					// Il colore del vertice è piu' chiaro se la sua 
																					// normale è parallela al raggio che congiunge origine al 
																					// punto stesso.


	// Clickable Window Objects functions (it uses the stencil buffer)
	void StartDefiningClickableObjects(bool clear_stencil);
	UINT getClickableObjectsMaxIndexValue();
	void DefineClickableObject(UINT index);											// 1 ... max
	void StopDefiningClickableObjects();
	UINT GetClickableObjectIndex(int x,int y,int width,int height);					// callable also outside the drawable context, the buffer should be ready and not busy
																					// 0       nothing
																					// 1..max  object index
																					// width & height must be the size of the framebuffer (pay attention)

};





//***********************************************************************************************************************
//							Primitive Opengl Indipendenti
//***********************************************************************************************************************
bool SetDisplayResolution_FullScreen(int w,int h,int bits,int refresh_freq);
void ResetDisplayResolution_FullScreen();

void ReadOpenGLState(OpenGL_State *ogl);							// (D) Read all the opengl state
void SetProjectionMatrix(Matrix *I);								// (D) Set I (3x4 or 4x4) to ProjectionView
void ReadProjectionMatrix(Matrix *P);								// (D) Read P (3x3 or 4x4) from Projection Matrix
void SetModelViewMatrix(Matrix *P);									// (D) Set P (3x4 or 4x4) to ModelView
void ReadModelViewMatrix(Matrix *P);								// (D) Read P (3x4 or 4x4) from ModelView

void SetAttrib(GLenum cap,GLboolean state);							// (D) Set variables cap at state = state
void SetStandardOGLEnvironment(bool use_depth,bool use_stencil,		// (D) Initialize the OGL Environment clearing the buffer
							   bool clear_buffer=true);				//     and setting the standard variables settings for operate in it



void Matrix_Translate(Matrix *P,Vector<3> T);						// (--) Translate P (4x4) like gltranslate
void Matrix_Mult(Matrix *P,Matrix *T);								// (--) Multiply P (4x4) by T(4,4) like glMult
void Matrix_scale(Matrix *P,Vector<3> S);							// (--) Scale P (4x4) like glScale


void SetClientSize(HWND hWnd,int w,int h);			    // Set Client size
void GetClientSize(HWND hWnd,int &w,int &h);			// Get Client Size  -> client space: [0,w-1]x[0,h-1]
void UpdateClient(HWND hWnd,bool Redraw=true);


//***********************************************************************************************************************
// Utility for
//  static RenderWindow<3>::Retrieve_OGLState_From_Calibration(...)
//***********************************************************************************************************************
Vector<2> Project(OpenGL_State *ogl,Vector<3> p,bool opengl_coords=true);	// Proietta p nel piano immagine 
																			// opengl_coords / pixel_coords
Vector<3> Project3(OpenGL_State *ogl,Vector<3> p,bool opengl_coords=true);	// Proietta p nel piano immagine 
																			// opengl_coords / pixel_coords
void ProjectVector(OpenGL_State *ogl,Vector<3> o3,Vector<3> v3,Vector<2> *o2,Vector<2> *v2,bool opengl_coords=true,bool o2_isknown=false);
																			// Proietta il vettore applicato (o3,v3) nel piano immagine -> (o2,v2)
																			// o2_isknown -> significa che o2 e' la proiezione di o3
																			// opengl_coords / pixel_coords
void UnProject(OpenGL_State *ogl,Vector<2> p,bool p_in_opengl_coords,Vector<3> *o,Vector<3> *d);
																			// Ritorna il raggio (o,d) su cui giace p





// Trova le coordinate globali di p posto nel sistema di riferimento [R,T]
template<int dim>
Vector<dim> Transform(Matrix *R,Vector<dim> origin,Vector<dim> p);			// R*p+T           (R dimxdim)
template<int dim>
Vector<dim> Transform(Matrix *RT,Vector<dim> p);
template<int dim>
Vector<dim> TransformVector(Matrix *RT,Vector<dim> v);




//
// GetNearFarPlane utility
//
// Fast routine to get the min and max z given a camera
// 1) No image cropping, see View class function for this
// 2) It works always. Remember to check 
//			a) mesh->num_p == 0
//          b) min_z<0, in this case the camera intersect the object
//
void GetNearFarPlane(HyperMesh<3> *mesh,Matrix *Ext,double &min_z,double &max_z);








struct FrameBuffer_struct {
	int w,h;
	GLuint fbo;					// frame buffer object
	GLuint fbt;					// DrawBuffer[0] 
	GLuint fbd;					// DepthBuffer
	bool valid;					// Indicate that there are valid elements and must be destroied by DeleteFrameBuffer
	
	// ping-pong info
	GLuint InputTexture;		// deprecated
	int write_buffer;			// deprecated

	// private
	GLint old_viewport[4];
	bool delete_fbt;			// Indicates that DrawBuffer[0] was created by CreateFrameBuffer and has to be deleted by DeleteFrameBuffer

};








class AdvancedRenderWindow : public RenderWindow<3> {
	GLuint ToonShader;									// TODO Metterli static o dentro il cpp
	GLuint HatchingShader;
	GLuint HatchingShader2;
	GLuint HatchingText[8];

	void DrawEdgesMacro(HyperMesh<3> *mesh,int LineColor,float LineWidth,float EdgeCreaseAngle);
	void ExtendVolume(HyperFace<3> *f,List<typename Vector<3>> *Points,int localLight,GLfloat *lightPosition,float extendDistance);

public:	





	AdvancedRenderWindow(char *title,
						 void(*display)(void*)=NULL,														// NULL means virtual window (always hide)
						 bool(*command)(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *)=NULL,
						 void *userdata=NULL,
						 int Background_Color=0x000000,
						 HMENU hMenu=NULL,
						 DWORD WindowStyle=WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
						 int RenderWindowProperties=0);
	~AdvancedRenderWindow();


	void RetrieveDepthBuffer(UINT texture_index);









	//************************************************
	// Shaders Manager (D & ND)
	//************************************************
		GLuint CreateShader(char *VertexFile,char *FragmentFile,bool PrintLog=false);
		GLuint CreateShaderFromCode(char *VertexCode,char *FragmentCode,bool PrintLog=false);
		void DeleteShader(GLuint shader);
		void SetShader(GLuint shader);
		void SetUniformVariable(GLuint shader,char *var_name,int value);
		void SetUniformVariable(GLuint shader,char *var_name,double value);
		void SetUniformVariable(GLuint shader,char *var_name,Vector<3> *value);
		void SetUniformVariable(GLuint shader,char *var_name,Vector<4> *value);
		void SetUniformVariable(GLuint shader,char *var_name,Matrix *value);		// 4x4 matrix
		void SetUniformVariable(GLuint shader,char *var_name,GLdouble *value);		// 4x4 matrix
		void SetUniformArray(GLuint shader,char *var_name,int count,float *values);
		void Run2DShader(int num_textures_maps,UINT *InTextures,Vector<2> *uv_tl,Vector<2> *uv_br);		// specify the ROI on each texture as top_left and botton_right coordinates 
		void Run2DShader(int num_textures_maps,UINT *InTextures);
		void Run2DShader(UINT InTexture);

		void ActivateTexture(int level);
		void TexCoord(int level,Vector<2> uv);
		void TexCoord(int level,Vector<3> uvw);
	//************************************************

	



	//************************************************
	// FrameBuffer (D & ND)
	//************************************************
		//			Draw			= TextureArray[0], TextureArray[1], TextureArray[2], ...		<- If they are INVALID they will be created using default_buffer_format and default_buffer_options
		//			Depth			= DepthBuffer
		//			InputTexture	= usable only for ping_pong_fbo
		//
		// 
		// NOTE: the textures in TextureArray must be destroyed by the caller (even the one created by CreateFrameBuffer when initially INVALID)
		//       only when num_textures=1 and TextureArray is INVALID -> DeleteFrameBuffer will destroy it
		//
		//
		FrameBuffer_struct CreateFrameBuffer(int width,int height,UINT *TextureArray,int num_textures,int default_buffer_format=TEXTURE_CH_RGBA,int default_buffer_options=TEXTURE_MAG_LINEAR|TEXTURE_MIN_LINEAR|TEXTURE_CLAMP_EDGE_X|TEXTURE_CLAMP_EDGE_Y,bool generate_depth_buffer=true);
		FrameBuffer_struct CreateFrameBuffer(int width,int height,int buffer_format=TEXTURE_CH_RGBA,bool ping_pong_fbo=false,bool generate_depth_buffer=true);
			// Usabile sia in area draw che fuori
			void SetFrameBuffer(FrameBuffer_struct *fb);
				
				void SetDrawBuffer(int index);										// Disegne in TextureArray[index]
				void SetDrawBuffers(int num);										// Disegne i primi num elementi di TextureArray
				void SetDrawBuffers(int num_buffers,int *nums);						// Disegne num_buffers elementi di TextureArray indicizzati da nums[..]
					// Seguire da SetDrawBuffer(0);
				void SetReadBuffer(int index);										// Legge TextureArray[index]
					// Seguire da SetReadBuffer(0);
				
				void PingPong(FrameBuffer_struct *fb);
				void RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorRGB> *I);
				void RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorRGBA> *I);
				void RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<GreyLevel> *I);
				void RetrieveFrameBuffer(FrameBuffer_struct *fb,Bitmap<ColorFloat> *I,int channel=GL_RED);
			void ClearFrameBuffer(FrameBuffer_struct *fb);
		void DeleteFrameBuffer(FrameBuffer_struct *fb);		


		// Draw primitives (advanced)
		void DrawHyperMesh_MultiTexture(HyperMesh<3> *mesh,
										int num_textures_maps,int num_textures_parameterizations,
										UINT *Textures,
										List<HyperMesh<3>::PointType> **TextureUVW,
										List<HyperMesh<3>::FaceType> **Texture_Face);
		void DrawHyperMesh_MultiTexture(int num_f,
										List<HyperMesh<3>::PointType> *Points,
										List<HyperMesh<3>::FaceType> *Faces,
										List<HyperMesh<3>::PointType> *Normals,							// it can be NULL -> Flat rendering
										int num_textures_maps,int num_textures_parameterizations,
										UINT *Textures,
										List<HyperMesh<3>::PointType> **TextureUVW,
										List<HyperMesh<3>::FaceType> **Texture_Face);
	//************************************************



	










	//************************************************
	// Specific shaders
	//************************************************
	// ToonShader
	void InitToonShader();													// Inizializzare prima di EnableDraw
	void DrawHyperMesh_Toon(HyperMesh<3> *mesh,								// Need normal per vertex
							int LineColor=0,float LineWidth=5.0f,
							bool Flat=false,float EdgeCreaseAngle=-1.0f);
			// GL_AMBIENT					Ambientale = Dark Diffuse
			// GL_DIFFUSE					Diffuse
			// GL_SPECULAR					Specular
			// GL_EMISSION[0]				Dark\Light Threshold			
			// GL_SHININESS					Specular Shininess




	// HatchingShader														// TODO: Dare la possibilità di disabilitare LIGHT0!!
	void InitHatchingShader(float TextScale=1.0);							// Inizializzare prima di EnableDraw
	void DrawHyperMesh_Hatching(HyperMesh<3> *mesh,int mode,				// Need normal per vertex
							int LineColor=0,float LineWidth=5.0f,
							bool Flat=false,float EdgeCreaseAngle=-1.0f);
			




	// Cast Shadows
	void InitCastShadow();
	void CastShadow(HyperMesh<3> *mesh,int LightType,GLfloat *LightPosition,float MaxDistance=4.0);
	void CastShadow(HyperMesh<3> *mesh,int LightType,Vector<3> LightPosition,float MaxDistance=4.0);
											// LightType =  0 DirectionalLight
											//				1 PointLight
	
	// 1° metodo <- Disegna le ombre di ShadowColor		(non considera l'highlighting)
	void RenderDarkShadow(UINT ShadowColor);
	
	// 2° metodo (considera tutto)
	void Pass1_DeclareShadowOn(HyperMesh<3> *mesh);						// Dichiaro chi subirà l'ombra
	void Pass2_RenderShadow();											// Inizializzo
																		// Renderizzo a luce spenta
	void Pass3_Close();													// Chiudo

	// TODO: 
	//   Per risolvere le ombre sulla superficie:
	//		A) non applicarle all'oggetto che ha castato le ombre -> Xo' nn ho le sue self shadow
	//      B) Vincolare ad un angolo specifico chi genera le ombre (c'e' un to-do in ExtendVolume)
	//
	//		C) (La piu' funzionale) -> Assicurarsi che il nero dello shadow sia = al colore ambientale del modello!!!!
	//				-> Cosi Ci si salva a meno di specularita' -> Essa dovra' fare l'effetto Glow che si disperde
	//				NB: le silhouette vanno rifatte se si usa TOON o Hatch
	//
	//		D) la piu' soft -> Far partire il volume shadow da un po' piu' sotto della faccia.. in modo da evitare la tangenza!!!!
	//			Bisogna fare un tuning.
	//
};
















#define GPGPU_MAX_TEXTURE 4


/*
	GPGPU: 
		- Ad ogni processo è associato uno e un solo contesto libero chiamato "GPGPU Context", a cui ogni oggetto
		  di tipo GPGPU attinge.
		- Il contesto libero è un repositi di texure e deve essere attivato per essere usato (usando EnsureContext)


		- Ogni oggetto GPGPU è un processore che può eseguire vari task in maniera sequenziale.
		- Per essere usato ogni metodo(*) dell'oggetto GPU deve essere interno al suo contesto, per far ciò: 
					::EnsureContext();
					Activate();
		

		(*) solo i metodi con (*)
*/

// DEPRECATED class
class GPGPU {
protected:
	// GPGPU Free Context
	static HDC Context_hDC;
	static HGLRC Context_hRC;

	
	// 
	GLuint fbo;
	GLuint OutTexture;
	int width,height;


	int n_texture;
	GLuint WorkingImages[GPGPU_MAX_TEXTURE];

public:
	GPGPU(int width, int height,int channels=3);				// Entra nel contesto OPENGL_FREE (comune)
	~GPGPU();													// TODO: distruttore non implementato

	
	// GPGPU Free Context
	static bool CreateContext();								// (richiamato già dal costruttore)
	static void EnsureContext();								// Entra nel cntesto comune
	static GLuint LoadBitmap(Bitmap<ColorRGB> *I);				// Carica delle texture nel contesto comune
	static GLuint LoadBitmap(Bitmap<GreyLevel> *I);
	static GLuint CreateShader(char *VertexFile,char *FragmentFile,bool PrintLog=true);

	// Object Methods 
	void Activate();
	void DeActivate();
	void ClearImageList();
	bool PushImage(GLuint texture);
	

	// Object SubContext Methods (*)
	void SetProgram(GLuint prg);
	void Run();
	void GetResult(Bitmap<ColorRGB> *I);
	void GetResult(Bitmap<GreyLevel> *I);
};



#endif
