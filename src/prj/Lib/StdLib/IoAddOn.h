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





//
// String operation
//
char    *strcln(const char *str);		                             // Clone a string
bool     str_begin(const char *str,const char *begin);				 // Ritorna true se str comincia con begin
void     str_replace(char **str,const char *new_text);
wchar_t *CharToWChar(char *str);									 // to delete with delete[]


//
// Text
//
char *OrdinalNumbersPostfix(int num);


//
// Filenames
//
	// Extension
	void Add_File_Extension(char *filename,char *ext);
	char *Get_File_Extension(char *filename);						// reposiziona il puntatore, NULL se non esiste
	bool CompareFileExtension(char *filename,char *ext);
	
	// Path
	char *Get_File_Path(const char *filename);						// da cancellare se non e' NULL
	char *Set_File_Path(char *Path,char *filename);					// da cancellare





//
// Binary Piping Functions
// 
//	format spec:	c	char
//					i	int
//					d	double			(float non esiste con la funzione ...)
//					p	void *			<- Cast necessario nella maggior parte dei casi
//
BYTE *bsprintf(char *format,...);								// create a binary buffer containing the information described by format  
void  bscanf(BYTE *buffer,char *format,...);					// retrieve the information from the pipe
void  bscanf_pointers(BYTE *buffer,char *format,...);			// retrieve the pointer to the information from the pipe
																//		Es:
																//			buffer=bsprintf("i",(int)10);
																//			int *x; bscanf_pointers(buffer,"i",&x);
																//			(*x)=5;										// scrivo sul buffer
																//





//
// Screen Output
//
void Write_Over(const char *fmt, ...);
void CenteredCout(char *text,int scr_width);
void AsciiArtCout(char *text);
void Reset_Write_Over();
void PrintTime(char *Out,UINT seconds);
void PrintTime(UINT seconds);


class ConsoleProgressBarr {
	UINT WritedPercent;
	UINT WritedPosition;
	UINT RelativeDisplayPos;
	SpeedTester Speed;
	char RemainTText[70];
	
public:
	ConsoleProgressBarr();
	~ConsoleProgressBarr();
	void Aggiorna(UINT Pos,UINT Max);

	// Disegna una ProgressBarr dalla posizione corrente del cursore.
	// Per qualsiasi altro output, il cursore si troverà alla fine della ProgressBarr.
};











//
// Input
//
class CommandParser {
	int num_elements;
	char **elements;
	char used_ID[500];
	bool invalid_option_parameters;

	int GetOptionIndex(char ID);
	int GetNumOptionParameters(int option_index);
	bool GetParameter(char ID,int param_index,char **&Out);
public:
	CommandParser(int argc,char* argv[]);
	~CommandParser();
	
	bool GetFlag(char ID);
	bool GetParameter(char ID,char *type,...);			// type example: "siid" -> string integer integer double
														// s char*
														// c char*
														// i int*
														// f float*
														// d double*

	bool GetParameterE(char ID,char *type,...);			// type example: "si[ii][s][i]"
														// valid inputs -> si
														//              -> siii
														//              -> siiis
														//              -> siiisi
	
	bool CheckInvalid();
};






//
// Console
//
PCHAR* GetArgvArgc(int* _argc);
void getConsole();
void closeConsole();




//
// Windows 
//
#ifdef SYSTEM_WINDOWS
#define GFD_OPEN 1
#define GFD_SAVE 2
#define BMP_FILTER "Bitmap (*.Bmp)\0*.Bmp\0\0\0"
bool Get_File_Dialog(HWND hWnd,char *filename,int bufflen,char *filter,int action);

void PopUp(HWND hWnd,LPCTSTR Title,LPCTSTR fmt,...);
void WindowsErrorExit();


class MouseEvent_struct {
public:
// Private
	double last_pos_x,last_pos_y;
	bool TrackEvent;

// Public
	// Variable
	int Event[4];			// L,M,R,MouseMove   +1 Pressed, -1 Released, 0 Nothing
	double pos_x,pos_y;
	int pos_x_pxs,pos_y_pxs;
	int DBLClick[3];		// L,M,R			 +1 DblClick, 0 Nothing				(only for CS_DBLCLKS windows)

	bool start_dragging;	
	int drag_bottom;		// 0,1,2,-1			 L,M,R,Nothing
	bool dropped;	
		double delta_x,delta_y;
	

	// Static
	int ButtonState[3];		// L,M,R			 +1 Pressed, -1 Released

	MouseEvent_struct();
};


bool GetMouseState(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,MouseEvent_struct *MouseEvents);	
	// true		mouse information stored in MouseEvents
	// false	no mouse information
	//
	// NOTA:	puo' chiamare il gestore di messaggi e quindi la funzione
	//			chiamante. Nessun problema se la si usa propriamente.
	//
#endif
