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





#ifdef _DEBUG
	#define ErrorExit(TEXT) {printf("Error: %s",(TEXT));printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
	#define ErrorExit_int(TEXT,INT_VAL) {printf("Error: ");printf((TEXT),INT_VAL);printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
	#define ErrorExit_str(TEXT,STRING) {printf("Error: ");printf((TEXT),STRING);printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
	#define Warning(TEXT) {printf("Warning: %s\n",(TEXT));}
	#define Warning_int(TEXT,INT_VAL) {printf("Warning: ");printf((TEXT),INT_VAL);printf("\n");}
	#define Warning_str(TEXT,STRING) {printf("Warning: ");printf((TEXT),STRING);printf("\n");}
	#define Warning_int2(TEXT,INT_VAL,INT_VAL2) {printf("Warning: ");printf((TEXT),INT_VAL,INT_VAL2);printf("\n");}	
	#define PerformanceWarning(TEXT) {printf("PerformanceWarning: %s\n",(TEXT));}
	#define Debug_Assert(x,TEXT) {if (!(x)) {ErrorExit(TEXT);};}
#else
	#ifdef FINAL_RELEASE
		#define ErrorExit(TEXT) {printf("Error: %s",(TEXT));exit(1);}
		#define ErrorExit_int(TEXT,INT_VAL) {printf("Error: ");printf((TEXT),INT_VAL);exit(1);}
		#define ErrorExit_str(TEXT,STRING) {printf("Error: ");printf((TEXT),STRING);exit(1);}
		#define Warning(TEXT) {}
		#define Warning_int(TEXT,INT_VAL) {}
		#define Warning_str(TEXT,STRING) {}
		#define Warning_int2(TEXT,INT_VAL,INT_VAL2) {}
		#define PerformanceWarning(TEXT) {}
		#define Debug_Assert(x,TEXT) {}
	#else
		#define ErrorExit(TEXT) {printf("Error: %s",(TEXT));printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
		#define ErrorExit_int(TEXT,INT_VAL) {printf("Error: ");printf((TEXT),INT_VAL);printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
		#define ErrorExit_str(TEXT,STRING) {printf("Error: ");printf((TEXT),STRING);printf("\nFunction: %s\nFile: %s\nLine:%i\n",__FUNCTION__,__FILE__,__LINE__);exit(1);}
		#define Warning(TEXT) {printf("Warning: %s\n",(TEXT));}	
		#define Warning_int(TEXT,INT_VAL) {printf("Warning: ");printf((TEXT),INT_VAL);printf("\n");}
		#define Warning_str(TEXT,STRING) {printf("Warning: ");printf((TEXT),STRING);printf("\n");}
		#define Warning_int2(TEXT,INT_VAL,INT_VAL2) {printf("Warning: ");printf((TEXT),INT_VAL,INT_VAL2);printf("\n");}
		#define PerformanceWarning(TEXT) {printf("PerformanceWarning: %s\n",(TEXT));}
		#define Debug_Assert(x,TEXT) {if (!(x)) {ErrorExit(TEXT);};}
	#endif
#endif


#define TODO(TEXT) {printf("TODO (%s): %s\n",__FUNCTION__,(TEXT));}
#define Assert(cond,TEXT) {if (!(cond)) ErrorExit(TEXT);}
#define NOT_IMPLEMENTED ErrorExit("Function not implemented.");
#define VOID2INT(x) (reinterpret_cast<int>(x))


#define PRINT(x)         {cout<<#x<<" = "<<(x)<<"\n";}
#define PRINTA(x,n)      {cout<<#x<<"["<<(n)<<"] = ";{for(int tmp_i_def=0;tmp_i_def<(n);tmp_i_def++) cout<<(x)[tmp_i_def]<<", ";}cout<<"\n";}
#define QUOTEOBJ_NAME(x) #x
#define QUOTEOBJ(x)      QUOTEOBJ_NAME(x)


// SNEW deve avere Obj inizializzato a NULL!!!
#define SNEW(Obj,type) {if (!(Obj)) {(Obj)=new type;if (!(Obj)) {ErrorExit("Not enought memory.");}}}
#define SNEWA(Obj,type,num) {if (!(Obj)) {(Obj)=new type[num];if (!(Obj)) {ErrorExit("Not enought memory.");}}}
#define SNEWA_I(Obj,pointer_type,num,InitVal) {pointer_type *tmp_obj=(Obj);if (tmp_obj==NULL) {(Obj)=tmp_obj=new pointer_type[num];if (!tmp_obj) {ErrorExit("Not enought memory.");} else {for(int i=0;i<(num);i++) tmp_obj[i]=InitVal;}}}
#define SNEWA_P(Obj,pointer_type,num) SNEWA_I(Obj,pointer_type,num,NULL)
#define SNEWA_INCREASE(Obj,type,new_size,old_size) {if (!(Obj)) {(Obj)=new type[new_size];if (!(Obj)) {ErrorExit("Not enought memory.");};(old_size)=(new_size);}if (new_size>old_size) {SDELETEA(Obj);SNEWA(Obj,type,new_size);(old_size)=(new_size);}}

#define SDELETE(Obj) {if (Obj) {delete (Obj);(Obj)=NULL;}}
#define SDELETEA(Obj) {if (Obj) {delete [](Obj);(Obj)=NULL;}}
#define SDELETEA_REC(Obj,num) {if ((Obj)!=NULL) {for(int tmp_i_def=0;tmp_i_def<(num);tmp_i_def++) SDELETE((Obj)[tmp_i_def]);delete [](Obj);(Obj)=NULL;}}
#define SDELETEA_RECA(Obj,num) {if ((Obj)!=NULL) {for(int tmp_i_def=0;tmp_i_def<(num);tmp_i_def++) SDELETEA((Obj)[tmp_i_def]);delete [](Obj);(Obj)=NULL;}}

// MIN and MAX Indexed
#define MIN_I(v,min_v,i,best_i) {if ((v)<=(min_v)) {(min_v)=(v);(best_i)=(i);};}
#define MAX_I(v,max_v,i,best_i) {if ((v)>=(max_v)) {(max_v)=(v);(best_i)=(i);};}
#define SORT(a,b) {if (a>b) ::swap(a,b);}


// Utils
#define BYTECLAMP(x)    (min(max(0,(int)x),255))
#define FILE_PATH_SIZE  270

#define IMAGE_SAVE_I(Image,Str,index) {char Text[100];sprintf(Text,Str,index);Image->Save(Text);}
#define IMAGE_SAVE_2I(Image,Str,index,index2) {char Text[100];sprintf(Text,Str,index,index2);Image->Save(Text);}
#define IMAGE_SAVE_3I(Image,Str,index,index2,index3) {char Text[100];sprintf(Text,Str,index,index2,index3);Image->Save(Text);}
#define IMAGE_SAVE_4I(Image,Str,index,index2,index3,index4) {char Text[100];sprintf(Text,Str,index,index2,index3,index4);Image->Save(Text);}

// 
#define FRAME_BUFFER_IMAGE_SAVE_I(world,frame,color_type,w,h,Str,index) {world->SetFrameBuffer(frame);Bitmap<color_type> Img(w,h);world->RetrieveFrameBuffer(frame,&Img);IMAGE_SAVE_I((&Img),Str,index);world->ClearFrameBuffer(frame);}




//
// Shader standard commands
//
#define START_SHADER(shader,w_)		{AdvancedRenderWindow *w=w_;UINT curr_shader=shader;w->SetShader(curr_shader);
#define SHADER_P(par,val)			w->SetUniformVariable(curr_shader,par,val);
#define SHADER_PA(par,count,val)	w->SetUniformArray(curr_shader,par,count,val);
#define END_SHADER()				w->SetShader(0);}




//
// Syncronization
//
#define ENTER_C					WaitForSingleObject(CriticalSection,INFINITE);	
#define EXIT_C					ReleaseMutex(CriticalSection);
#define ENTER_C_(C)				WaitForSingleObject((C),INFINITE);	
#define EXIT_C_(C)				ReleaseMutex((C));
#define WAIT_EVENT(M)			WaitForSingleObject((M),INFINITE);
#define TEST_EVENT(M,state)		{DWORD s_tmp=WaitForSingleObject((M),0);if (s_tmp==WAIT_OBJECT_0) (state)=1; else {if (s_tmp==WAIT_TIMEOUT) (state)=0; else (state)=-1;}}
#define WAIT_2EVENT(M1,M2)		{HANDLE s_tmp[2];s_tmp[0]=(M1);s_tmp[1]=(M2);WaitForMultipleObjects(2,s_tmp,FALSE,INFINITE);}
#define EVENT_SIGNALED(M)		(WaitForSingleObject((M),0)==WAIT_OBJECT_0)				// Note: !EVENT_SIGNALED(M) either means not signaled or M does not exist




#ifdef LIB_DEBUG
#define ASSERT(condition) { if (!(condition)) ErrorExit("Assertion condition failed"); }
#else
#define ASSERT(condition) {}
#endif


typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned char* LPBYTE;
typedef int BOOL;
typedef void *LPVOID;
#define STANDARD_OUTPUT std::basic_ostream<char,struct std::char_traits<char>>

#define FALSE   0
#define TRUE    1

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ------------------ Errori di approssimazione ------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
#define IS_ZERO(a,th) (::abs(a)<th)






// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ---------------------- Macro ----------------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

#define FILL_MEMORY(start,number_dword)								\
								__asm XOR EAX,EAX					\
								__asm MOV ECX,number_dword			\
								__asm MOV EDI,start					\
								__asm REP STOSD						

#define COPY_MEMORY(source,dest,number_dword)						\
								__asm MOV ESI,source				\
								__asm MOV ECX,number_dword			\
								__asm MOV EDI,dest					\
								__asm REP MOVSD










// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// ------------------- Fast Template -----------------------------------
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

template <class T>
inline void swap(T &a,T &b) {
	T temp=a;
	a=b;
	b=temp;
}


