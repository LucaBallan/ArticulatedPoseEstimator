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
#include <string.h>
#include "Word.h"
#pragma warning(3:4244)

bool GetSpacedWord(char *line,char *&start,char *&end,char WORD_SEPARATOR) {

	size_t size_line=strlen(line);
	for(size_t i=0;i<size_line;i++) {
		if (line[i]!=WORD_SEPARATOR) {
			start=line+i;
			i++;
			for(;i<size_line;i++) {
				if (line[i]==WORD_SEPARATOR) {
					end=line+(i-1);
					return true;
				}
			}
			end=line+(i-1);
			return true;
		}
	}
	return false;

}

char *GetSpacedWord(char *line,char WORD_SEPARATOR) {
	char *start,*end;

	if (!GetSpacedWord(line,start,end,WORD_SEPARATOR)) return NULL;
	
	int o=0;
	char *n=new char[end-start+1+1];
	for(char *pos=start;pos<=end;pos++) n[o++]=*pos;
	n[o]=0;

	return n;
}

char *GetSpacedWord(char *line,char *word,int buffer_dim,char WORD_SEPARATOR) {
	char *start,*end;

	if (!GetSpacedWord(line,start,end,WORD_SEPARATOR)) return NULL;
	
	int o=0;
	for(char *pos=start;pos<=end;pos++) {
		word[o++]=*pos;
		if (o>=buffer_dim-1) break;
	}
	word[o]=0;

	return (end+1);
}


bool GetWord(char *Word,char *LP,unsigned int n,char END_WORD) {

	// Find n° word
	unsigned int i=0,w=0,last=0;

	for(;;i++) {
		if ((i>=strlen(LP)) || (LP[i]==END_WORD)) {
			w++;
			if (w==n) {
				int cpy=i-last;
				if (cpy>MAXWORDLEN-1) cpy=MAXWORDLEN-1;
				if (cpy==0) return false;
				memcpy(Word,LP+last,cpy);
				Word[cpy]=0;
				return true;
			}
			if (i>=strlen(LP)) return false;
			last=i+1;
		}
	}
}

bool GetRemaint(char *RetPh,char *LP,unsigned int n,char END_WORD) {
	unsigned int i=0,w=0;
	n--;

	for(;;i++) {
		if (i>=strlen(LP)) return false;
		if (LP[i]==END_WORD) {
			w++;
			if (w==n) {
				strcpy(RetPh,LP+i+1);
				return true;
			}
		}
	}

}


bool CompareFmPhrase(char *Ft,char *WD) {
	if (strlen(Ft)==0) {
		if (strlen(WD)==0) return true;
			else return false;
	}
	
	char W[MAXWORDLEN];
	char Wft[MAXWORDLEN];
	
	int i=1;
	while (true) {
		if (!GetWord(Wft,Ft,i)) return true;
		if (!GetWord(W,WD,i)) return false;

		if (!_stricmp(Wft,"*")) {i++;continue;}
		if (_stricmp(Wft,W)) return false;
		
		i++;
	}
}

char **CommandLineToArgv(char *CommandLine,int &argc) {
	char Word[MAXWORDLEN];

	argc=1;
    while (GetWord(Word,CommandLine,argc++));
	argc--;argc--;

	char **argv=new char *[argc];
	
	for(int i=0;i<argc;i++) {
		GetWord(Word,CommandLine,i+1);
		argv[i]=new char[strlen(Word)+1];
		strcpy(argv[i],Word);
	}
		
	return argv;
}


int Bit_Set(int array,int index,int value) {
	if (value==1) return (array|(1<<index));
	return (array&(~(1<<index)));
}

int Bit_Read(int array,int index) {
	return ((array>>index)&1);
}
