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
#include "common.h"
#include "IOBuffer.h"
#include "StdLib/ListEntities.h"
#include "List.h"
#include "StdLib/MathAddOn.h"
#include "IoAddOn.h"
#include <sys/types.h>
#include <sys/stat.h>
#pragma warning(3:4244)

int PackBits(BYTE *src,UINT32 *out,int n_bytes) {
	#define NN_BITS 32
	
	UINT32 *old_out=out;
	UINT32 val=0;
	int index=0;
	for(int i=0;i<n_bytes;i++) {
		if ((*src)&0x80) val=val|(1<<index);
		index++;
		src++;
		if (index>=NN_BITS) {
			(*out)=val;
			index=0;
			val=0;
			out++;
		}
	}
	if (index!=0) {
		(*out)=val;
		out++;
	}
	
	return (out-old_out);
}

#ifdef _WIN64
void UnPackBits(UINT32 *src,BYTE *out_,int n_bytes) {

	int nn_bits;
	
	for(int i=0;i<n_bytes;i+=NN_BITS) {
		
		nn_bits=NN_BITS;
		if (i+NN_BITS>n_bytes) nn_bits=n_bytes-i;

		UINT32  val=*src;

		for(int i=0;i<nn_bits;i++) {
			if (val&0x1) *out_=255;
			else *out_=0;
			val=val>>1;
			out_++;
		}
		src++;
	}
}
#else
void UnPackBits(UINT32 *src,BYTE *out_,int n_bytes) {
	
	int nn_bits;
	for(int i=0;i<n_bytes;i+=NN_BITS) {
		nn_bits=NN_BITS;
		if (i+NN_BITS>n_bytes) nn_bits=n_bytes-i;
		__asm {
			mov ecx,nn_bits
			mov edx,src
			mov ebx,out_
			mov eax,[edx]
		 loop_qui:
			xor dl,dl
			shr eax,1
			jnc salto_qui
			mov dl,255
		 salto_qui:
			mov [ebx],dl
			inc ebx
			loop loop_qui
			mov out_,ebx
		}
		src++;
	}

}
#endif


// ------------------------------------------------------------------
// IFileBuffer
// ------------------------------------------------------------------

/* Note:

ASCII
\n 10
\r 13


Windows
\r\n

UNIX
\n

*/


IFileBuffer::IFileBuffer(char *FileName) {
	
	File=fopen(FileName,"rb");
	wordwidth=50;
	
	if (File!=NULL) {
		fseek(File,0L,SEEK_END);
		dim=ftell(File);
		fseek(File,0L,SEEK_SET);
	} else dim=0;

	set_word_separators();
}

IFileBuffer::~IFileBuffer() {
	if (File!=NULL) fclose(File);
}

int IFileBuffer::NumberOfByte() {
	return dim;
}

bool IFileBuffer::IsFinished() {
	if (File==NULL) return true;
	long Pos=ftell(File);
	return (Pos>=dim);
}

bool IFileBuffer::ReadLine(char *Block,int &len) {
	if (File!=NULL) {
		if (!IsFinished()) {
			len=(int)fread(Block,1,len,File);
			if (len==0) return false;
			return true;
		} 
	} 
	
	return false;
}

bool IFileBuffer::ReadBitLine(char *Block,int &len,UINT32 *aux_buffer) {
	int len_readed,len_;

	len_readed=len_=(int)(ceil(len/32.0)*4);
	bool ret=ReadLine((char*)aux_buffer,len_readed);
	if ((!ret) || (len_readed<len_)) return false;

	UnPackBits(aux_buffer,(BYTE*)Block,len);
	return true;
}



bool IFileBuffer::ReadPharse(char *Block,int &len) {
	if (File==NULL) return false;
	int i;
	long Pos=ftell(File);
	
	if (!ReadLine(Block,len)) return false;
	
	for(i=0;i<len;i++) {
		if (Block[i]=='\r') break;
		if (Block[i]=='\n') break;
	}


	if ((i<len) && (Block[i]=='\r')) {
		// Probably Windows Style
		if (i+1<len) {
			if (Block[i+1]=='\n') {
				// Window Style \r\n
				fseek(File,Pos+i+2,SEEK_SET);
			} else {
				// Erroneous Style \r
				fseek(File,Pos+i+1,SEEK_SET); 
			}
		} else {
			// Buffer Overrun correggibile
		    // dovrei caricare l'altro blocco e togliere il /r se c'e'
			// fread(1,1) e controllo....
		}
	} else {
		if (i==len) {
			// Buffer Overrun
		} else {
			// Linux Style (\n)
			fseek(File,Pos+i+1,SEEK_SET);
		}
	}

    Block[i]=0;
    len=i;
    return true;
}

char Buffer[BUFFER_DIM];

bool IFileBuffer::FlushToTheBeginningOfNextWord(const char *separators) {
	if (File==NULL) return false;
	long Pos=ftell(File);
	
	int tmplen=BUFFER_DIM;
	if (!ReadLine(Buffer,tmplen)) return false;

	int start=(int)strspn(Buffer,separators);
	fseek(File,Pos+start,SEEK_SET);

	return true;
}

bool IFileBuffer::GetWord(char *Block,int &len,const char *separators) {
	if (File==NULL) return false;
	long Pos=ftell(File);
	
	int tmplen=BUFFER_DIM;
	if (!ReadLine(Buffer,tmplen)) return false;

	int start=(int)strspn(Buffer,separators);
	tmplen=(int)strcspn(Buffer+start,separators);
	if (tmplen<len) len=tmplen;

	int stop_len=len;
	if ((Buffer+start)[stop_len]=='\r') stop_len++;
	if ((Buffer+start)[stop_len]=='\n') stop_len++;

	// al max start+len = BUFFER_DIM  -> in tal caso il puntatore nn si muove
	fseek(File,Pos+start+stop_len,SEEK_SET);

	(Buffer+start)[len]=0;
	strcpy(Block,Buffer+start);
    return true;
}

bool IFileBuffer::GetWord_i(char *Block,int &len,char *allowed_char) {
	if (File==NULL) return false;
	long Pos=ftell(File);
	
	int tmplen=BUFFER_DIM;
	if (!ReadLine(Buffer,tmplen)) return false;

	int start=(int)strcspn(Buffer,allowed_char);
	tmplen=(int)strspn(Buffer+start,allowed_char);
	if (tmplen<len) len=tmplen;

	int stop_len=len;
	if ((Buffer+start)[stop_len]=='\r') stop_len++;
	if ((Buffer+start)[stop_len]=='\n') stop_len++;

	// al max start+len = BUFFER_DIM  -> in tal caso il puntatore nn si muove
	fseek(File,Pos+start+stop_len,SEEK_SET);

	(Buffer+start)[len]=0;
	strcpy(Block,Buffer+start);
    return true;
}

IFileBuffer& IFileBuffer::operator>>(BYTE &x) {
	char tmp[30+1];
	int len=30;
	
	saveState();
	GetWord_i(tmp,len,"0123456789.+-");
	x=(BYTE)BYTECLAMP(atoi(tmp));
	FlushToNextRow();

	return *this;
}

IFileBuffer& IFileBuffer::operator>>(int &x) {
	char tmp[30+1];
	int len=30;
	
	saveState();
	GetWord_i(tmp,len,"0123456789.+-");
	x=atoi(tmp);
	FlushToNextRow();

	return *this;
}

IFileBuffer& IFileBuffer::operator>>(UINT &x) {
	char tmp[30+1];
	int len=30;

	saveState();
	GetWord_i(tmp,len,"0123456789.+-");
	x=strtoul(tmp,NULL,10);
	FlushToNextRow();

	return *this;
}

IFileBuffer& IFileBuffer::operator>>(bool &x) {
	char tmp[30+1];
	int len=30;
	
	saveState();
	GetWord_i(tmp,len,"0123456789.+-");
	if (atoi(tmp)==0) x=false;
	else x=true;
	FlushToNextRow();

	return *this;
}

IFileBuffer& IFileBuffer::operator>>(double &x) {
	char tmp[30+1];
	int len=30;
	
	saveState();
	GetWord_i(tmp,len,"0123456789.+-eE");
	x=atof(tmp);
	FlushToNextRow();

	return *this;
}

IFileBuffer& IFileBuffer::operator>>(float &x) {
	char tmp[30+1];
	int len=30;
	
	saveState();
	GetWord_i(tmp,len,"0123456789.+-eE");
	x=(float)atof(tmp);
	FlushToNextRow();

	return *this;
}

void IFileBuffer::set_wordwidth(int wordwidth) {
	this->wordwidth=wordwidth;
}

void IFileBuffer::set_word_separators(const char *word_separators) {
	this->word_separators=word_separators;
}

IFileBuffer& IFileBuffer::operator>>(char *word) {
	saveState();
	int tmplen=wordwidth;
	GetWord(word,tmplen,word_separators);
	FlushToNextRow();
	return *this;
}

long IFileBuffer::GetPosition() {
	long curr_pos=ftell(File);
	return curr_pos;
}

void IFileBuffer::SetPosition(long pos) {
	long curr_pos=ftell(File);
	if (curr_pos!=pos) fseek(File,pos,SEEK_SET);
}

void IFileBuffer::saveState() {
	saved_pos=ftell(File);
}
void IFileBuffer::back() {
	fseek(File,saved_pos,SEEK_SET);
}

void IFileBuffer::FlushToNextRow() {
	if (File==NULL) return;
	long Pos=ftell(File);

	int tmplen=BUFFER_DIM;
	if (!ReadLine(Buffer,tmplen)) return;
	
	int start=(int)strspn(Buffer," \t");
	if (Buffer[start]=='\r') {
		start++;
		if (Buffer[start]=='\n') start++;
		// This is next line
		fseek(File,Pos+start,SEEK_SET);
		return;
	}
	
	if (Buffer[start]=='\n') {
		start++;
		// This is next line
		fseek(File,Pos+start,SEEK_SET);
		return;
	} 

	// I was just in the next line or the current line contains other characthers...
	fseek(File,Pos,SEEK_SET);
}


// ------------------------------------------------------------------
// OutBuffer
// ------------------------------------------------------------------

OutBuffer::OutBuffer() {
	sprintf(Double_Format,"%%.10g");
}

bool OutBuffer::WriteString(const char *String) {
	return WriteLine(String,(int)strlen(String));
}

OutBuffer& OutBuffer::operator<<(char *st) {
	WriteString(st);
	return *this;
}

OutBuffer& OutBuffer::operator<<(const char *st) {
	WriteString(st);
	return *this;
}

OutBuffer& OutBuffer::operator<<(char x) {
	char text[2];
	text[0]=x;text[1]=0;
	WriteString(text);
	return *this;
}

void OutBuffer::setPrecision(int num_decimal,bool use_exp) {
	
	if (use_exp) {
		if (num_decimal==-1) sprintf(Double_Format,"%%g");
		else sprintf(Double_Format,"%%.%ig",num_decimal);
	} else {
		if (num_decimal==-1) sprintf(Double_Format,"%%f");
		else sprintf(Double_Format,"%%.%if",num_decimal);
	}
}

OutBuffer& OutBuffer::operator<<(double x) {
	char text[30];
	sprintf(text,Double_Format,x);
	WriteString(text);
	return *this;
}

OutBuffer& OutBuffer::operator<<(float x) {
	char text[30];
	sprintf(text,Double_Format,(double)x);
	WriteString(text);
	return *this;
}

OutBuffer& OutBuffer::operator<<(int x) {
	char text[30];
	sprintf(text,"%i",x);
	WriteString(text);
	return *this;
}

OutBuffer& OutBuffer::operator<<(unsigned int x) {
	char text[30];
	sprintf(text,"%u",x);
	WriteString(text);
	return *this;
}

OutBuffer& OutBuffer::operator<<(void *x) {
	char text[30];
	sprintf(text,"%p",x);
	WriteString(text);
	return *this;
}



// ------------------------------------------------------------------
// OFileBuffer
// ------------------------------------------------------------------

OFileBuffer::OFileBuffer(char *FileName) {
	File=fopen(FileName,"w+b");
	Bytes=0;
}

OFileBuffer::~OFileBuffer() {
	Close();
}

void OFileBuffer::Flush() {
	if (File!=NULL) fflush(File);
}

void OFileBuffer::Close() {
	if (File!=NULL) {
		fclose(File);
		File=NULL;
	}
}

bool OFileBuffer::WriteLine(const char *Block,int len) {
	if (File!=NULL) {
		int bt=(int)fwrite(Block,1,len,File);
		Bytes+=bt;
		
		if (bt!=len) return false;
		return true;
	}
	return false;
}

bool OFileBuffer::WriteBitLine(const char *Block,int len,UINT32 *aux_buffer) {
	int dw_size=PackBits((BYTE*)Block,aux_buffer,len);
	return WriteLine((const char*)aux_buffer,(dw_size<<2));
}

int OFileBuffer::NumberOfByte() {
	return Bytes;
}








//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


int NumLineInFile(char *FileName) {
	IFileBuffer *In=new IFileBuffer(FileName);

	char Block[MAX_ROW_DIM];
	int len,count=0;
	
	while (!In->IsFinished()) {
		len=MAX_ROW_DIM;
		In->ReadPharse(Block,len);
		count++;
	}
	
	delete In;

	return count;

}

char *TextFileRead_Include(const char *FileName) {
	#define INCLUDE_STRING		"#include \""
	char *text=TextFileRead(FileName);
	if (text==NULL) return NULL;
	
	do {
		char *finded=strstr(text,INCLUDE_STRING);
		if (finded==NULL) break;
		
		size_t len_include=strlen(INCLUDE_STRING);
		char *end_finded=strchr(finded+len_include,'\"');
		if (end_finded==NULL) ErrorExit("Not a valid include file.");

		size_t size=end_finded-(finded+len_include);
		char *tmp_file_name=new char[size+1];
		memcpy(tmp_file_name,finded+len_include,size);
		tmp_file_name[size]=0;

		char *Path=Get_File_Path(FileName);
		char *full_file_name=Set_File_Path(Path,tmp_file_name);
		delete []tmp_file_name;
		if (Path!=NULL) delete []Path;
		char *To_Add=TextFileRead_Include(full_file_name);
		delete []full_file_name;
		
		size_t len_text=strlen(text);
		size_t len_toadd=strlen(To_Add);
		char *new_text=new char[len_text+len_toadd+1];

		memcpy(new_text,text,finded-text);
		memcpy(new_text+(finded-text),To_Add,len_toadd);
		memcpy(new_text+(finded-text)+len_toadd,end_finded+1,strlen(end_finded)+1);

		delete []To_Add;
		delete []text;
		text=new_text;

	} while (true);

	return text;
}

char *TextFileRead(const char *FileName) {

	FILE *fp;
	int count=0;
	char *content = NULL;

	if (FileName != NULL) {
		fp = fopen(FileName,"rt");

		if (fp != NULL) {
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = (int)fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}

	return content;
}

void Copy(char *src,char *dst) {
	char *Text=new char[strlen(src)+strlen(dst)+40];
	sprintf(Text,"copy \"%s\" \"%s\" > nul 2> nul",src,dst);
	system(Text);
	delete []Text;
}

void mkdir(char *dir) {
	char *Text=new char[strlen(dir)+40];
	sprintf(Text,"mkdir \"%s\" 2> nul",dir);
	system(Text);
	delete []Text;
}
void rmdir(char *dir) {
	char *Text=new char[strlen(dir)+40];
	sprintf(Text,"rmdir /S /Q \"%s\" 2> nul",dir);
	system(Text);
	delete []Text;
}

void del_all_files_in_dir(char *dir) {
	char *Text=new char[strlen(dir)+40];
	sprintf(Text,"del \"%s\\*.*\" /q 2> nul",dir);
	system(Text);
	delete[]Text;
}

void del_file(char *file_name) {
	char *Text=new char[strlen(file_name)+40];
	sprintf(Text,"del \"%s\" /q 2> nul",file_name);
	system(Text);
	delete[]Text;
}

void list_all_files_by_template(char *dirname,char *file_template,void (*F_Dir)(char *dirname,char *subdirname,char *file_template),void (*F_File)(char *dirname,char *filename,char *file_template)) {
	WIN32_FIND_DATA ffd;

	char *command=new char[strlen(dirname)+strlen(file_template)+10];
	strcpy(command,dirname);
	if (strlen(command)!=0) {
		if (command[strlen(command)-1]!='\\') strcat(command,"\\");
	}
	strcat(command,file_template);
	HANDLE hFind=FindFirstFile(command,&ffd);
	delete []command;

	if (hFind==INVALID_HANDLE_VALUE) return;

	do {
		if ((ffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && (F_Dir)) F_Dir(dirname,ffd.cFileName,file_template);
		else if (F_File) F_File(dirname,ffd.cFileName,file_template);
	} while (FindNextFile(hFind,&ffd));

	FindClose(hFind);
}

void directory_list(char *dirname,void (*F_Dir)(char *dirname,char *subdirname,char *file_template),void (*F_File)(char *dirname,char *filename,char *file_template)) {
	list_all_files_by_template(dirname,"*",F_Dir,F_File);
}

int count_F_File_num;
int count_F_File_min;
int count_F_File_max;
void count_F_File(char *dirname,char *filename,char *file_template) {
	int start_ss=(int)(strlen(file_template)-strlen(filename));
	char *first_q=strchr(file_template,'?');
	if (first_q) {
		int start_index=(int)(first_q-file_template);
		if (start_index-start_ss>=0) {
			int o=0;
			char index_map[100];
			for(int i=start_index;i<(int)strlen(file_template);i++,o++) {
				if (file_template[i]!='?') break;
				index_map[o]=filename[i-start_ss];
			}
			index_map[o]=0;
			int index=atoi(index_map);
			if (index<count_F_File_min) count_F_File_min=index;
			if (index>count_F_File_max) count_F_File_max=index;
		}
	} 

	count_F_File_num++;
}

int count_all_files_by_template(char *full_filename_template,int *min_index,int *max_index) {
	count_F_File_num=0;
	count_F_File_min=INT_MAX;
	count_F_File_max=-INT_MAX;
	list_all_files_by_template("",full_filename_template,NULL,count_F_File);
	if (min_index) *min_index=count_F_File_min;
	if (max_index) *max_index=count_F_File_max;
	return count_F_File_num;
}


void convert_printf_wildcard_int(char *file_template,char *file_wildcard) {
	char *first_q=strchr(file_template,'%');
	if (first_q) {
		int first_index=(int)(first_q-file_template);
		char *end_q=strchr(first_q,'i');
		if (end_q) {
			int last_index=(int)(end_q-file_template);
			strcpy(file_wildcard,file_template);
			file_wildcard[last_index]=0;

			int len=atoi(file_wildcard+first_index+1);
			file_wildcard[first_index]=0;
			if (len!=0) {
				for(int i=0;i<len;i++) strcat(file_wildcard,"?");
			} else {
				strcat(file_wildcard,"*");
			}
			strcat(file_wildcard,file_template+last_index+1);
			return;
		}
	} 
	strcpy(file_wildcard,file_template);
}



int find0star(char *s,int &second) {
	int i;
	int len=(int)strlen(s);

	for(i=len-1;i>=0;i--) {
		if (s[i]=='?') break;
	}
	if (i<0) return -1;
	second=i;

	for(;i>=0;i--) {
		if (s[i]!='?') break;
	}
	
	return (i+1);
}

bool ParseIncrementalFileName(char *SourceFileName,int i,char *out) {
	char tmp[500];
	out[0]=0;

	int second;
	int first=find0star(SourceFileName,second);
	if (first<0) {
		strcpy(out,SourceFileName);
		return true;
	}
	int n_len=(second-first+1);
	memcpy(out,SourceFileName,first);
	out[first]=0;
	sprintf(tmp,"%s%%0%ii%s",out,n_len,SourceFileName+second+1);
	sprintf(out,tmp,i);
	return false;
}
bool fileExists(char *FileName) {
	struct _stat buffer;
	if (_stat(FileName,&buffer)==-1) return false;
	return true;
}
char **GetFileList(char *SourceFileName,int start_index,int &num,int step) {
	char out[500];
	int i=start_index,o_i=0;
	char **index=new char*[1000];
	
	while (true) {
		bool unique=ParseIncrementalFileName(SourceFileName,i,out);
		if (!fileExists(out)) break;
		index[o_i]=new char[strlen(out)+1];
		strcpy(index[o_i],out);
		i+=step;
		o_i++;
		if (unique) break;
	}
	num=o_i;
	return index;
}


OStdBuffer::OStdBuffer() {
	active=true;
}

void OStdBuffer::Active(bool active) {
	this->active=active;
}

int OStdBuffer::NumberOfByte() {
	return 0;
}
bool OStdBuffer::WriteLine(const char *Block,int /*len*/) {
	std::cout<<Block;
	return true;
}






