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





#define LEN_FILE	400
#define MAXWORDLEN  300
#define SPACE       ' '



bool GetSpacedWord(char *line,char *&start,char *&end,char WORD_SEPARATOR);			// Get the pointers for the first and the last characters of the first word, end+1 is the remaining sentance (it is always a string terminating with 0)
																					// Return true if the word exist
char *GetSpacedWord(char *line,char WORD_SEPARATOR);								// Return the first word of the sentance, NULL if it does not exist
																					// to be deleted with delete[]
char *GetSpacedWord(char *line,char *word,int buffer_dim,char WORD_SEPARATOR);		// Get the first word of the sentance
																					// return NULL if it does not exist, the word is truncated at the (buffer_dim-1)
																					// otherwise it returns the remaining sentance

// Find n° word:  (n start with 1 = first)
//  			  true  -> Ok
//		 	      false -> doesn't exist				
bool GetWord(char *Word,char *LP,unsigned int n,char END_WORD=SPACE);

// Get Remaint from i° word:  true  -> Ok
//							  false -> doesn't exist				
bool GetRemaint(char *RetPh,char *LP,unsigned int n,char END_WORD=SPACE);

// Compare Formatted Phrase: true = Equal
bool CompareFmPhrase(char *Ft,char *WD);

// Get Argv,Argc
char **CommandLineToArgv(char *CommandLine,int &argc);


int Bit_Set(int array,int index,int value);
int Bit_Read(int array,int index);