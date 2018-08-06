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





#define MAX_ROW_DIM 300
#define BUFFER_DIM 300


// Simplified functions (Note: do not work for files bigger than 2Gb) (da modificare per farlo lavorare con files piu' grandi)
char *TextFileRead(const char *FileName);
char *TextFileRead_Include(const char *FileName);
int   NumLineInFile(char *FileName);


// Basic file operations
void Copy(char *src,char *dst);
void mkdir(char *dir);
void rmdir(char *dir);
void del_all_files_in_dir(char *dir);
void del_file(char *file_name);

// Directory listing
//			functions = NULL -> no operation
void list_all_files_by_template(char *dirname,char *file_template,void (*F_Dir)(char *dirname,char *subdirname,char *file_template),void (*F_File)(char *dirname,char *filename,char *file_template));		
int  count_all_files_by_template(char *full_filename_template,int *min_index=NULL,int *max_index=NULL);
void directory_list(char *dirname,void (*F_Dir)(char *dirname,char *subdirname,char *file_template),void (*F_File)(char *dirname,char *filename,char *file_template));


// Format converter
void convert_printf_wildcard_int(char *file_template,char *file_wildcard);  // convert printf format %...i to wildcard string $$$ or *
																			// integer is the only format supported



// File parsing
bool fileExists(char *FileName);
bool ParseIncrementalFileName(char *SourceFileName,int i,char *out);					// return true if it is only one file
char **GetFileList(char *SourceFileName,int start_index,int &num,int step);

// BitPacking
int PackBits(BYTE *src,UINT32 *out,int n_bytes);				// pack boolean src[n_bytes]    -> out[n] DWORDs   (n is returned)
void UnPackBits(UINT32 *src,BYTE *out_,int n_bytes);			// unpack       out[n] DWORDs   -> src[n_bytes]    (n is internally computed)



class InBuffer {
public:
	InBuffer() {}
	virtual ~InBuffer() {}
	
	virtual int NumberOfByte() = 0;					    // >=0 Bytes
													    // -1  I don't know
	virtual bool IsFinished() = 0;
	virtual bool ReadLine(char *Block,int &len) = 0;	// len specify the maximum size
														// false --> Non ha letto nulla
};

class OutBuffer {
	char Double_Format[10];
public:
	OutBuffer();
	virtual ~OutBuffer() {}
	
	virtual int NumberOfByte() = 0;						// Numero di bytes scritti
	virtual bool WriteLine(const char *Block,int len) = 0;    // Ritorna true solo se e' riuscito a scriverli tutti -> altrimenti controllare NumberOfByte()
	
	bool WriteString(const char *String);
    OutBuffer& operator<<(const char *st);
    OutBuffer& operator<<(char *st);
	OutBuffer& operator<<(char x);
	void setPrecision(int num_decimal,bool use_exp);	// -1 auto_decimal
	OutBuffer& operator<<(double x);
	OutBuffer& operator<<(float x);
	OutBuffer& operator<<(int x);
	OutBuffer& operator<<(unsigned int x);
	OutBuffer& operator<<(void *x);
};


class OStdBuffer:public OutBuffer {
	bool active;
public:
	OStdBuffer();
	virtual ~OStdBuffer() {};

	void Active(bool active);
	
	int NumberOfByte();
	bool WriteLine(const char *Block,int len);
};

class IFileBuffer: public InBuffer {
protected:
	FILE        *File;
	long         dim;
	long         saved_pos;
	int          wordwidth;
	const char  *word_separators;
	
	void  saveState();
	void  FlushToNextRow();

public:

	IFileBuffer(char *FileName);
	virtual ~IFileBuffer();

	int NumberOfByte();
	bool IsFinished();
	

	// Nota: Tutte le funzioni ritornano false se non hanno letto nulla!!!
	

// ---- Binary Access ----
	bool ReadLine(char *Block,int &len);									// Block di size = len
	bool ReadBitLine(char *Block,int &len,UINT32 *aux_buffer);				// len in this case is the lenght of the unpacked data
																			// aux_buffer  must have a size >= (int)ceil(len/32.0) dwords (i.e., (int)ceil(len/32.0)*4 bytes)
	long GetPosition();														// position in bytes inside the file
	void SetPosition(long pos);												//
	

// ---- Text Access ----
	bool ReadPharse(char *Block,int &len);				                    // Block di size = len+1   Per tener conto dello 0 alla fine  (len = lunghezza stringa effettiva)


	// ---- Word Access ----
	void back();										                    // torna indietro all'ultima richiesta di tipo >>
	void set_wordwidth(int wordwidth);                                      // wordwidth= dimBuffer - 1
	void set_word_separators(const char *word_separators=" \t\r\n");        // word_separators
	IFileBuffer& operator>>(BYTE &x);
	IFileBuffer& operator>>(int &x);
	IFileBuffer& operator>>(UINT &x);
	IFileBuffer& operator>>(double &x);
	IFileBuffer& operator>>(float &x);
	IFileBuffer& operator>>(bool &x);
	IFileBuffer& operator>>(char *word);
	
	bool GetWord(char *Block,int &len,const char *separator=" \t\r\n");		// Block di size = len+1   Per tener conto dello 0 alla fine
	bool FlushToTheBeginningOfNextWord(const char *separators=" \t\r\n");
	bool GetWord_i(char *Block,int &len,char *allowed_char);
};




class OFileBuffer:public OutBuffer {
private:
	FILE *File;
	int Bytes;

public:
	OFileBuffer(char *FileName);
	virtual ~OFileBuffer();
	
	int NumberOfByte();
	bool WriteLine(const char *Block,int len);
	bool WriteBitLine(const char *Block,int len,UINT32 *aux_buffer);		// len         is the size of the unpacked data
																			// aux_buffer  must have a size >= (int)ceil(len/32.0) dwords (i.e., (int)ceil(len/32.0)*4 bytes)

	// Added
	void Flush();
	void Close();
};
