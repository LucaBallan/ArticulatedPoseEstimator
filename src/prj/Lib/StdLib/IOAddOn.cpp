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





#include "../stdInclude.h"
#include "../common.h"
#include <string.h>
#include "Stack.h"
#include "IOBuffer.h"
#include "ListEntities.h"
#include "List.h"
#include "MathAddOn.h"
#include "IOAddOn.h"

#ifdef SYSTEM_WINDOWS
	#include <Commdlg.h>
#endif
#pragma warning(3:4244)


ConsoleProgressBarr::ConsoleProgressBarr() {
	printf("[                              ]   0%%");
	RelativeDisplayPos=36;
	WritedPercent=0;
	WritedPosition=0;
	RemainTText[0]=0;
}

ConsoleProgressBarr::~ConsoleProgressBarr() {
	for(UINT i=0;i<RelativeDisplayPos-36;i++) printf("\x08");
	printf("   Completed.");
	UINT CurDispPos=36+8+13;
	for(UINT i=CurDispPos;i<RelativeDisplayPos;i++) printf(" ");
}


void ConsoleProgressBarr::Aggiorna(UINT Pos,UINT Max) {
	Speed.CheckPoint(Pos);

	char Out[70];
	UINT CurDispPos;
	bool AggiornaPercent=false;
	bool AggiornaPosition=false;
	bool AggiornaRemainTime=false;

	double x=Pos/(double)Max;
	Speed.RemainTime(Out,Max);

	
	if (strcmp(RemainTText,Out)) {
		AggiornaRemainTime=true;
		strcpy(RemainTText,Out);
	}

	if (WritedPercent != (unsigned int)Approx(100 * x)) {
		AggiornaPercent=true;
		WritedPercent=Approx(100*x);
	}

	if (WritedPosition != (unsigned int)Approx(30 * x)) {		// TODO??
		AggiornaPosition=true;
		WritedPosition=Approx(30*x);	
	}
    

	if (AggiornaPosition) {
		for(UINT i=0;i<RelativeDisplayPos;i++) printf("\x08");
		for(UINT i=0;i<WritedPosition;i++) printf("=");
		for(UINT i=WritedPosition;i<30;i++) printf(" ");
		printf("] ");
		printf("%3u%%",WritedPercent);
		printf("   %s left",RemainTText);
		CurDispPos=36+8+(int)strlen(RemainTText);
		if (CurDispPos>RelativeDisplayPos) RelativeDisplayPos=CurDispPos;
		else for(UINT i=CurDispPos;i<RelativeDisplayPos;i++) printf(" ");
	} else {
		if (AggiornaPercent) {
			for(UINT i=0;i<RelativeDisplayPos-32;i++) printf("\x08");
			printf("%3u%%",WritedPercent);
			printf("   %s left",RemainTText);
			CurDispPos=36+8+(int)strlen(RemainTText);
			if (CurDispPos>RelativeDisplayPos) RelativeDisplayPos=CurDispPos;
			else for(UINT i=CurDispPos;i<RelativeDisplayPos;i++) printf(" ");
		} else {
			if (AggiornaRemainTime) {		
				for(UINT i=0;i<RelativeDisplayPos-36;i++) printf("\x08");
				printf("   %s left",RemainTText);
				CurDispPos=36+8+(int)strlen(RemainTText);
				if (CurDispPos>RelativeDisplayPos) RelativeDisplayPos=CurDispPos;
				else for(UINT i=CurDispPos;i<RelativeDisplayPos;i++) printf(" ");
			}
		}
	}
}


void PrintTime(UINT seconds) {
	char Out[50];
	PrintTime(Out,seconds);
	printf(Out);
}


char *PluralSingular(int num,char *singular,char *plural) {
	if (num==1) return singular;
	return plural;
}

void PrintTime(char *Out,UINT seconds) {
	UINT minutes,hours,days,years;
	
	minutes=seconds/60;
	seconds%=60;
	hours=minutes/60;
    minutes%=60;
	days=hours/24;
	hours%=24;
	years=days/365;
	days%=365;

	if (years!=0) {
		sprintf(Out,"%u %s, %u %s",years,PluralSingular(years,"year","years"),days,PluralSingular(days,"day","days"));
	} else {
		if (days!=0) {
			sprintf(Out,"%u %s, %u %s",days,PluralSingular(days,"day","days"),hours,PluralSingular(hours,"hour","hours"));
		} else {
			if (hours!=0) {
				sprintf(Out,"%u %s, %u min",hours,PluralSingular(hours,"hour","hours"),minutes);
			} else {
				if (minutes!=0) sprintf(Out,"%u min, %u sec",minutes,seconds);
				else {
					sprintf(Out,"%u sec",seconds);
				}
			}
		}
	}
}

void Add_File_Extension(char *filename,char *ext) {
	size_t pos=strcspn(filename,".");
	if (pos==strlen(filename)) {
		strcat(filename,".");
		strcat(filename,ext);
	}
}

char *Get_File_Extension(char *filename) {
	char *Ext=strrchr(filename,'.');
	if (Ext==NULL) return NULL;
	Ext++;
	return Ext;
}

char *Get_File_Path(const char *filename) {
	const char *Ext=strrchr(filename,'\\');
	if (Ext==NULL) return NULL;

	char *Path=new char[Ext-filename+1];
	memcpy(Path,filename,Ext-filename);
	Path[Ext-filename]=0;
	return Path;
}

char *Set_File_Path(char *Path,char *filename) {
	if (Path==NULL) {
		char *filename_ext=new char[strlen(filename)+1];
		strcpy(filename_ext,filename);
		return filename_ext;
	}
	char *filename_ext=new char[strlen(Path)+strlen(filename)+2];
	strcpy(filename_ext,Path);
	strcat(filename_ext,"\\");
	strcat(filename_ext,filename);
	return filename_ext;
}

bool CompareFileExtension(char *filename,char *ext) {
	char *Ext=Get_File_Extension(filename);
	if (Ext==NULL) return false;
	if (!_stricmp(Ext,ext)) return true;
	
	return false;
}

void CenteredCout(char *text,int scr_width) {
	int len=(int)strlen(text);
	if (scr_width>=len) {
		int padd=(scr_width-len)/2;
		for(int i=0;i<padd;i++) std::cout<<" ";
		std::cout<<text;
		for(int i=padd+len;i<scr_width;i++) std::cout<<" ";
	} else std::cout<<text;
}


#ifdef USE_FIGLET
#include "D:\Developer\Luca\Common Libs\figlet\figlet.hpp"
void AsciiArtCout(char *text) {
	Figlet::standard.print(text);
}
#else
void AsciiArtCout(char *text) {
	cout << text;
}
#endif

#ifdef SYSTEM_WINDOWS


MouseEvent_struct::MouseEvent_struct() {
	Event[0]=Event[1]=Event[2]=Event[3]=0;
	ButtonState[0]=ButtonState[1]=ButtonState[2]=-1;
	pos_x=pos_y=0.0;
	pos_x_pxs=pos_y_pxs=0;
	last_pos_x=last_pos_y=0.0;
	drag_bottom=-1;
	dropped=false;
	delta_x=delta_y=0.0;
	TrackEvent=false;
}

bool GetMouseState(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,MouseEvent_struct *MouseEvents_) {
	bool mouse_event=false;

	MouseEvent_struct MouseEvents=(*MouseEvents_);
	MouseEvents.Event[0]=MouseEvents.Event[1]=MouseEvents.Event[2]=MouseEvents.Event[3]=0;
	MouseEvents.pos_x=MouseEvents.pos_y=0.0;
	MouseEvents.pos_x_pxs=MouseEvents.pos_y_pxs=0;
	MouseEvents.start_dragging=false;
	MouseEvents.dropped=false;
	MouseEvents.delta_x=MouseEvents.delta_y=0.0;	
	MouseEvents.DBLClick[0]=MouseEvents.DBLClick[1]=MouseEvents.DBLClick[2]=0;

	switch(uMsg) {
		case WM_RBUTTONDOWN:
			MouseEvents.Event[2]=1;
			MouseEvents.ButtonState[2]=1;
			mouse_event=true;
			break;
		case WM_LBUTTONDOWN:
			MouseEvents.Event[0]=1;
			MouseEvents.ButtonState[0]=1;
			mouse_event=true;
			break;
		case WM_MBUTTONDOWN:
			MouseEvents.Event[1]=1;
			MouseEvents.ButtonState[1]=1;
			mouse_event=true;
			break;
		case WM_RBUTTONUP:
			MouseEvents.Event[2]=-1;
			MouseEvents.ButtonState[2]=-1;
			mouse_event=true;
			break;
		case WM_LBUTTONUP:
			MouseEvents.Event[0]=-1;
			MouseEvents.ButtonState[0]=-1;
			mouse_event=true;
			break;
		case WM_MBUTTONUP:
			MouseEvents.Event[1]=-1;
			MouseEvents.ButtonState[1]=-1;
			mouse_event=true;
			break;
		case WM_MOUSELEAVE:
			MouseEvents.Event[3]=1;
			mouse_event=true;
			MouseEvents.TrackEvent=false;
			break;
		case WM_MOUSEMOVE:
			if (!MouseEvents.TrackEvent) {
				MouseEvents.TrackEvent=true;
				TRACKMOUSEEVENT tm;
				tm.cbSize=sizeof(TRACKMOUSEEVENT);
				tm.dwFlags=TME_LEAVE;
				tm.hwndTrack=hWnd;
				tm.dwHoverTime=0;
				TrackMouseEvent(&tm);
			}
			MouseEvents.Event[3]=1;
			mouse_event=true;
			break;
		case WM_LBUTTONDBLCLK:
			MouseEvents.DBLClick[0]=1;
			mouse_event=true;
			break;
		case WM_MBUTTONDBLCLK:
			MouseEvents.DBLClick[1]=1;
			mouse_event=true;
			break;
		case WM_RBUTTONDBLCLK:
			MouseEvents.DBLClick[2]=1;
			mouse_event=true;
			break;

	}
	if (mouse_event) {
		int mx=LOWORD(lParam);
		int my=HIWORD(lParam);
		if (mx & 1 << 15) mx -= (1 << 16);
		if (my & 1 << 15) my -= (1 << 16);
		RECT Rect;int w,h;
		GetClientRect(hWnd,&Rect);
		w=Rect.right-Rect.left;
		h=Rect.bottom-Rect.top;
		MouseEvents.pos_x=(mx-(w/2.0))/(w/2.0);
		MouseEvents.pos_y=(-my+(h/2.0))/(h/2.0);
		MouseEvents.pos_x_pxs=mx;
		MouseEvents.pos_y_pxs=my;

		if (MouseEvents.drag_bottom==-1) {
			if (MouseEvents.Event[1]==1) MouseEvents.drag_bottom=1;
			if (MouseEvents.Event[2]==1) MouseEvents.drag_bottom=2;
			if (MouseEvents.Event[0]==1) MouseEvents.drag_bottom=0;
				
			if (MouseEvents.drag_bottom!=-1) {
				MouseEvents.last_pos_x=MouseEvents.pos_x;
				MouseEvents.last_pos_y=MouseEvents.pos_y;
				MouseEvents.start_dragging=true;
				SetCapture(hWnd);
			}
		} else {
			MouseEvents.delta_x=MouseEvents.pos_x-MouseEvents.last_pos_x;
			MouseEvents.delta_y=MouseEvents.pos_y-MouseEvents.last_pos_y;
			if (MouseEvents.Event[MouseEvents.drag_bottom]==-1) {
				MouseEvents.dropped=true;
				MouseEvents.drag_bottom=-1;
				ReleaseCapture();
			}
		}
		(*MouseEvents_)=MouseEvents;
	}

	return mouse_event;
}

bool Get_File_Dialog(HWND hWnd,char *filename,int bufflen,char *filter,int action) {
	OPENFILENAME ofn;
	ZeroMemory(filename, bufflen);

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = bufflen;
	ofn.lpstrFilter = filter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex = 1;
	ofn.nFileOffset = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lpfnHook = NULL;
	ofn.lCustData = 0;
	ofn.lpTemplateName = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	
	if (action==GFD_OPEN) {
		ofn.lpstrTitle="Open...";
		ofn.Flags=ofn.Flags|OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn)) return true;
	} else {
		ofn.Flags=ofn.Flags|OFN_OVERWRITEPROMPT;
		ofn.lpstrTitle="Save...";
		if (GetSaveFileName(&ofn)) return true;
	}

	return false;
}
void PopUp(HWND hWnd,LPCTSTR Title,LPCTSTR fmt,...) {
	char text[400];
	va_list ap;

	if (fmt == NULL) return;

	va_start(ap,fmt);									// Parses The String For Variables
	vsprintf(text,fmt,ap);								// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	MessageBox(hWnd,text,Title,MB_OK);
}
#endif





int Write_Over_last_index=-1;
void Write_Over(const char *fmt, ...) {
	char text[256];
	va_list ap;
	if (fmt == NULL) return;
	va_start(ap,fmt);
	vsprintf(text,fmt,ap);
	va_end(ap);
	if (Write_Over_last_index!=-1) {
		for(int i=0;i<Write_Over_last_index;i++) printf("\x08 \x08");
	}
	Write_Over_last_index=(int)strlen(text);
	printf("%s",text);
}
void Reset_Write_Over() {
	Write_Over_last_index=-1;
}



CommandParser::CommandParser(int argc,char* argv[]) {
	num_elements=argc-1;
	elements=argv+1;
	used_ID[0]=0;
	invalid_option_parameters=false;
}
CommandParser::~CommandParser() {
}

int CommandParser::GetOptionIndex(char ID) {
	int index=0;
	char ID_[2];ID_[0]=ID;ID_[1]=0;_strlwr(ID_);
	if (strcspn(used_ID,ID_)==strlen(used_ID)) strcat(used_ID,ID_);

	while (index<num_elements) {
		if (elements[index][0]=='-') {
			_strlwr(elements[index]);
			if (elements[index][1]==ID_[0]) break;
		}
		index++;
	}
	return index;
}
int CommandParser::GetNumOptionParameters(int option_index) {
	int i=1;
	while(true) {
		if (option_index+i>=num_elements) break;
		if (elements[option_index+i][0]=='-') break;
		i++;
	}
	i--;
	return i;
}
bool CommandParser::GetParameter(char ID,int param_index,char **&Out) {
	int index=GetOptionIndex(ID);
	if (index==num_elements) return false;
	if (param_index==0) {Out=NULL;return true;}

	for(int i=1;i<=param_index;i++) {
		if (index+i>=num_elements) return false;
		if (elements[index+i][0]=='-') return false;
	}

	Out=elements+index+param_index;
	return true;
}
bool CommandParser::GetParameter(char ID,char *type,...) {
	char **Out;
	int num=(int)strlen(type);

	if (!GetParameter(ID,num,Out)) return false;
	if (!GetParameter(ID,1,Out)) return false;
	
	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') return false;
		if (type[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (type[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (type[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (type[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (type[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	return true;
}
bool CommandParser::GetFlag(char ID) {
	char **Out;
	bool exists=GetParameter(ID,0,Out);
	if (exists) {
		if (GetNumOptionParameters(GetOptionIndex(ID))!=0) {
			printf("Invalid number of parameters for option -%c.\n",ID);
			invalid_option_parameters=true;
		}
	}
	return exists;
}
bool CommandParser::GetParameterE(char ID,char *type,...) {
	char **Out;

	int opt_index=GetOptionIndex(ID);
	if (opt_index==num_elements) return false;
	int n_param=GetNumOptionParameters(opt_index);


	int j=0;
	int lenght=(int)strlen(type);
	int num=-1;
	char *typeB=new char[strlen(type)+2];
	typeB[0]=0;

	for(int i=0;i<lenght;i++) {
		if (type[i]=='[') {
			if (j<=n_param) num=j;
			else break;
			continue;
		}
		if (type[i]==']') {
			if (j<=n_param) num=j;
			else break;
			continue;
		}
		typeB[j++]=type[i];
		typeB[j]=0;
	}
	if (j<=n_param) num=j;
	if (n_param!=num) {
		printf("Invalid number of parameters for option -%c.\n",ID);
		invalid_option_parameters=true;
		delete[]typeB;
		return false;
	}
	typeB[num]=0;

	if (!GetParameter(ID,1,Out)) {delete[]typeB;return false;}
	

	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') {delete[]typeB;return false;}
		if (typeB[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (typeB[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (typeB[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (typeB[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (typeB[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	delete[]typeB;
	return true;
}

bool CommandParser::CheckInvalid() {
	int index=0;

	if (invalid_option_parameters) return false;

	while (index<num_elements) {
		if (elements[index][0]=='-') {
			char ID_[2];ID_[0]=elements[index][1];ID_[1]=0;_strlwr(ID_);
			
			if (strcspn(used_ID,ID_)==strlen(used_ID)) {
				printf("Invalid option -%s.\n",ID_);
				return false;
			}
		}
		index++;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     VERSIONE ESTESA                 ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////   TODO: TESTARE!!                              ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
CommandParser::CommandParser(int argc,char* argv[]) {
	num_elements=argc-1;
	elements=argv+1;
	used_ID[0]=0;
}
CommandParser::~CommandParser() {
}

bool CommandParser::GetParameter(char *ID,int param_index,char **&Out,int ricorrenza=1) {
	int index=0;
	char *ID_=strcln(ID);strlwr(ID_);
	
	char *ID_S=strcln(ID_);strcat(ID_S,";");
	if (strcspn(used_ID,ID_S)==strlen(used_ID)) strcat(used_ID,ID_S);
	delete []ID_S;


	int ricorrenza_corrente=0;
	while (index<num_elements) {
		if (elements[index][0]=='-') {
			strlwr(elements[index]);
			if (!strcmp((elements[index])+1,ID_)) {
				ricorrenza_corrente++;
				if (ricorrenza_corrente==ricorrenza) break;
			}
		}
		index++;
	}
	delete []ID_;

	if (index==num_elements) return false;
	if (param_index==0) {Out=NULL;return true;}

	for(int i=1;i<=param_index;i++) {
		if (index+i>=num_elements) return false;
		if (elements[index+i][0]=='-') return false;
	}

	Out=elements+index+param_index;
	return true;
}
bool CommandParser::GetFlag(char *ID) {
	char **Out;
	return GetParameter(ID,0,Out);
}
// Note:  
//  Si puo' aggiungere un nome di parametro piu' lungo della singola lettera
//  Si puo' aggiungere  ricorrenza
//  CheckInvalid giusto?

bool CommandParser::GetParameter(char *ID,char *type,...) {
	char **Out;
	int num=(int)strlen(type);
	if (!GetParameter(ID,num,Out)) return false;
	if (!GetParameter(ID,1,Out)) return false;
	
	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') return false;
		if (type[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (type[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (type[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (type[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (type[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	return true;
}

bool CommandParser::GetParameterE(char *ID,char *type,...) {
	char **Out;

	int j=0;
	int lenght=(int)strlen(type);
	int num=-1;
	char *typeB=new char[strlen(type)+2];
	typeB[0]=0;

	for(int i=0;i<lenght;i++) {
		if (type[i]=='[') {
			if (GetParameter(ID,j,Out)) num=j;
			else break;
			continue;
		}
		if (type[i]==']') {
			if (GetParameter(ID,j,Out)) num=j;
			else break;
			continue;
		}
		typeB[j++]=type[i];
		typeB[j]=0;
	}
	if (GetParameter(ID,j,Out)) num=j;
	if (num==-1) {delete typeB;return false;}
	typeB[num]=0;

	if (!GetParameter(ID,1,Out)) {delete typeB;return false;}
	

	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') {delete typeB;return false;}
		if (typeB[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (typeB[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (typeB[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (typeB[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (typeB[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	delete typeB;
	return true;
}

bool CommandParser::GetParameter(char *ID,int recurrence,char *type,...) {
	char **Out;
	int num=(int)strlen(type);
	if (!GetParameter(ID,num,Out,recurrence)) return false;
	if (!GetParameter(ID,1,Out,recurrence)) return false;
	
	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') return false;
		if (type[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (type[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (type[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (type[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (type[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	return true;
}

bool CommandParser::GetParameterE(char *ID,int recurrence,char *type,...) {
	char **Out;

	int j=0;
	int lenght=(int)strlen(type);
	int num=-1;
	char *typeB=new char[strlen(type)+2];
	typeB[0]=0;

	for(int i=0;i<lenght;i++) {
		if (type[i]=='[') {
			if (GetParameter(ID,j,Out,recurrence)) num=j;
			else break;
			continue;
		}
		if (type[i]==']') {
			if (GetParameter(ID,j,Out,recurrence)) num=j;
			else break;
			continue;
		}
		typeB[j++]=type[i];
		typeB[j]=0;
	}
	if (GetParameter(ID,j,Out,recurrence)) num=j;
	if (num==-1) {delete typeB;return false;}
	typeB[num]=0;

	if (!GetParameter(ID,1,Out,recurrence)) {delete typeB;return false;}
	

	va_list marker;
	va_start(marker,type);
	for(int i=0;i<num;i++) {
		if (Out[i][0]=='-') {delete typeB;return false;}
		if (typeB[i]=='s') strcpy(va_arg(marker,char*),Out[i]);
		if (typeB[i]=='i') *va_arg(marker,int*)=atoi(Out[i]);
		if (typeB[i]=='c') *(va_arg(marker,char*))=Out[i][0];
		if (typeB[i]=='f') *(va_arg(marker,float*))=(float)atof(Out[i]);
		if (typeB[i]=='d') *(va_arg(marker,double*))=atof(Out[i]);
	}
	va_end(marker);
	delete typeB;
	return true;
}

bool CommandParser::CheckInvalid() {
	int index=0;

	while (index<num_elements) {
		if (elements[index][0]=='-') {
			char *ID_S=strcln((elements[index])+1);strcat(ID_S,";");strlwr(ID_S);
			
			if (strcspn(used_ID,ID_S)==strlen(used_ID)) {
				printf("Invalid -%s option.\n",(elements[index])+1);
				delete []ID_S;
				return false;
			}
			delete []ID_S;
		}
		index++;
	}
	return true;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////









//
// Text
//
char *OrdinalsPostFix[]={"st","nd","rd","th"};
char *OrdinalNumbersPostfix(int num) {
	switch(num) {
		case 1: 
			return OrdinalsPostFix[0];
		case 2: 
			return OrdinalsPostFix[1];
		case 3: 
			return OrdinalsPostFix[2];
		default: 
			return OrdinalsPostFix[3];
	}
}


//
// String Functions
// 
char *strcln(const char *str) {
	if (str==NULL) return NULL;
	char *x=new char[strlen(str)+1];
	strcpy(x,str);
	return x;
}

void str_replace(char **str,const char *new_text) {
	SDELETEA((*str));
	(*str)=strcln(new_text);
}

wchar_t *CharToWChar(char *str) {
	DWORD dwNum=MultiByteToWideChar(CP_ACP,0,str,-1,NULL,0);
	wchar_t *n=new wchar_t[dwNum];
	MultiByteToWideChar(CP_ACP,0,str,-1,n,dwNum);
	return n;
}


bool str_begin(const char *str,const char *begin) {
	// Ritorna true se str comincia con begin
	int size_begin=(int)strlen(begin);
	int size_str=(int)strlen(str);
	
	if (size_str<size_begin) return false;

	for(int i=0;i<size_begin;i++) 
		if (str[i]!=begin[i]) return false;

	return true;
}















//
// Buffer Functions
// 
BYTE *bsprintf(char *format,...) {
	char *p;
	size_t size,len;
	va_list marker;
	BYTE *buffer,*buffer_p;
	len=strlen(format);

	size=0;
	p=format;
	for(size_t i=0;i<len;i++) {
		if (*p=='c') size+=sizeof(char);
		if (*p=='i') size+=sizeof(int);
		if (*p=='d') size+=sizeof(double);
		if (*p=='p') size+=sizeof(void*);
		p++;
	}
	buffer_p=buffer=new BYTE[size];

	p=format;
	va_start(marker,format);
	for(size_t i=0;i<len;i++) {
		if (*p=='c') {
			*((char *)buffer_p)=va_arg(marker,char);
			buffer_p+=sizeof(char);
		}
		if (*p=='i') {
			*((int *)buffer_p)=va_arg(marker,int);
			buffer_p+=sizeof(int);
		}
		if (*p=='d') {
			*((double *)buffer_p)=va_arg(marker,double);
			buffer_p+=sizeof(double);
		}
		if (*p=='p') {
			*((void **)buffer_p)=va_arg(marker,void *);
			buffer_p+=sizeof(void *);
		}
		p++;
	}
	va_end(marker);

	return buffer;
}

void bscanf(BYTE *buffer,char *format,...) {
	char *p;
	size_t len=strlen(format);
	BYTE *buffer_p;

	buffer_p=buffer;
	p=format;
	va_list marker;
	va_start(marker,format);
	for(size_t i=0;i<len;i++) {
		if (*p=='c') {
			*(va_arg(marker,char *))=*((char *)buffer_p);
			buffer_p+=sizeof(char);
		}
		if (*p=='i') {
			*(va_arg(marker,int *))=*((int *)buffer_p);
			buffer_p+=sizeof(int);
		}
		if (*p=='d') {
			*(va_arg(marker,double *))=*((double *)buffer_p);
			buffer_p+=sizeof(double);
		}
		if (*p=='p') {
			*(va_arg(marker,void **))=*((void **)buffer_p);
			buffer_p+=sizeof(void *);
		}
		p++;
	}
	va_end(marker);
}

void bscanf_pointers(BYTE *buffer,char *format,...) {
	char *p;
	size_t len=strlen(format);
	BYTE *buffer_p;

	buffer_p=buffer;
	p=format;
	va_list marker;
	va_start(marker,format);
	for(size_t i=0;i<len;i++) {
		if (*p=='c') {
			*(va_arg(marker,char **))=((char *)buffer_p);
			buffer_p+=sizeof(char);
		}
		if (*p=='i') {
			*(va_arg(marker,int **))=((int *)buffer_p);
			buffer_p+=sizeof(int);
		}
		if (*p=='d') {
			*(va_arg(marker,double **))=((double *)buffer_p);
			buffer_p+=sizeof(double);
		}
		if (*p=='p') {
			*(va_arg(marker,void ***))=((void **)buffer_p);
			buffer_p+=sizeof(void *);
		}
		p++;
	}
	va_end(marker);
}










































// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ------------------- Console -----------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

void WindowsErrorExit()  { 
	LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;

    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+40)*sizeof(TCHAR)); 
    wsprintf((LPTSTR)lpDisplayBuf, 
        TEXT("Error %d: %s\n"), 
        dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}


PCHAR* GetArgvArgc(int* _argc) {
	
	PCHAR CmdLine=GetCommandLine();

	PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR   a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = (ULONG)strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
        i + (len+2)*sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( a = CmdLine[i] ) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}



#include <io.h>
#include <stdio.h>
#include <fstream>


void getConsole() {
	int fd;
	FILE *fp;
	HANDLE hCon;

	AllocConsole();

	// Make printf happy
	hCon = GetStdHandle(STD_OUTPUT_HANDLE);

	fd = _open_osfhandle((intptr_t)(hCon), 0);
	fp = _fdopen(fd, "w");

	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

}

void closeConsole() {
	FreeConsole();
}
