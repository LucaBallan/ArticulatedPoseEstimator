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
// Configure FFDShow 
//
//    Use: VFW Configuration and enable the needed codecs..
//
//




#define NOT_VALID_FRAME -10000


template <class color_type>
class Video {
public:
	int width,height,bpp;
	LONG num_frames;
	double fps;
	
	Video();																// num_frames = -1  -> Error in the constructor
	virtual ~Video() {};													// num_frames =  0  -> File exists but it has 0 frames or information not avaiable (in most of the classes)

	virtual bool GetFrame(LONG n,Bitmap<color_type> *img)=0;				// n:       -> 0 to num_frames-1
																			// false    -> frames does not exists
};

template <class color_type>
class BufferedVideo: public Video<color_type> {
public:
	virtual BYTE *GetFrameReadOnly_(LONG n)=0;										// BYTE * (NULL -> frames does not exists)
	virtual Bitmap<color_type> *GetFrameReadOnly(LONG n)=0;							// (NULL -> frames does not exists)
	virtual UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0)=0;		// Carica il frame dentro un oggetto RenderWindow
};





//
// Classe video 
//   - si comporta come una sorgente video
//   - gestisce una cache interna dei frame
//	 - Multi-Task ok (GetFrameReadOnly_,GetFrameReadOnly non garantiscono l'integrita' del frame ottenuto, qualcuno subito dopo la chiamata puo' cancellarlo)
//   - ha come input un BufferedVideo<color_type> (generica)
//
//														   CACHE								NO CACHE
//   *  -> costo primo accesso al frame         O(copia_immagine*+lettura_immagine)         O(lettura_immagine)				// NOTA: copia nella cache il frame letto
//		-> costo successivi accessi al frame    O(0)                                        O(lettura_immagine)
//                                      
//   * Creazione del buffer solo alla primo accesso allo slot della cache
//
//
//   NOTE: La modalita' di costruzione non rallenta in alcun modo le classi
//		   video sottostanti. La copia_immagine dovuta ad una chiamata 
//		   GetFrame alla classe di input sembrerebbe evitabile se si volesse solo un 
//		   GetFrameReadOnly. Cio' pero' non e' affatto vero perche', in generale,
//		   la chamata GetFrameReadOnly (sia in OPENGL che in GrayScaleVideoSequence)
//		   effettuata una copia_immagine ad un buffer interno. Quello che facciamo
//		   con questa classe e' sostituire il buffer interno con quello del ObjectContainer.
//
//
template <class color_type>
class CachedBufferedVideo: public BufferedVideo<color_type> {
	HANDLE CriticalSection;

	BufferedVideo<color_type> *in;
	ObjectContainer_class *cache;
	static void *internal_LoadFrame(UINT index,CachedBufferedVideo<color_type> *src);
	//static void *internal_LoadFrameReadOnly(UINT index,CachedBufferedVideo<color_type> *src);
	static void *internal_ReplaceFrame(Bitmap<color_type> *old_obj,UINT index,CachedBufferedVideo<color_type> *src);
	static void internal_DeleteFrame(Bitmap<color_type> *obj,UINT index,CachedBufferedVideo<color_type> *src);

public:
	CachedBufferedVideo(BufferedVideo<color_type> *in,int policy,int max_cached_elements);		// si occupa della distruzione di (*in)
	~CachedBufferedVideo();
	
	
	bool GetFrame(LONG n,Bitmap<color_type> *img);							// n:       -> 0 to num_frames-1
																			// false    -> frames does not exists
	
	BYTE *GetFrameReadOnly_(LONG n);										// BYTE * (NULL -> frames does not exists)
	Bitmap<color_type> *GetFrameReadOnly(LONG n);							// (NULL -> frames does not exists)
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow
};

//
// Classe video texture
//   - si comporta come una sorgente video texture
//   - gestisce una cache interna dei frame
//	 - Multi-Task ok (pero' qualcuno subito dopo la chiamata a LoadFrameToTexture puo' cancellare la tessitura)
//   - ha come input un BufferedVideo<color_type> (generica)
//
//															 TEX_CACHE									CACHE													NO CACHE
//   *  -> costo primo accesso alla texture         O(lettura_immagine+load_to_tex)				O(copia_immagine+lettura_immagine+load_to_tex)         O(lettura_immagine+load_to_tex)
//		-> costo successivi accessi alla texture    O(0)                                        O(load_to_tex)										   O(lettura_immagine+load_to_tex)
//                                      
//
template <class color_type>
class TexCachedBufferedVideo: public BufferedVideo<color_type> {
	BufferedVideo<color_type> *in;
	ObjectContainer_class *cache;
	RenderWindow<3> *w;
	int options;
	static void *internal_LoadFrame(UINT index,TexCachedBufferedVideo<color_type> *src);
	static void internal_DeleteFrame(void *obj,UINT index,TexCachedBufferedVideo<color_type> *src);

public:
	TexCachedBufferedVideo(BufferedVideo<color_type> *in,int policy,int max_cached_elements,				// si occupa della distruzione di (*in)
						   RenderWindow<3> *w,int options=0);		
	~TexCachedBufferedVideo();
	
	
	bool GetFrame(LONG n,Bitmap<color_type> *img);							// sempre false
	BYTE *GetFrameReadOnly_(LONG n);										// sempre NULL
	Bitmap<color_type> *GetFrameReadOnly(LONG n);							// sempre NULL
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow		NOTA: w e options ignorati
};



#define IS_GENERIC_CACHED_VIDEO(x,color_type)	(IS_CACHED_VIDEO_TEX(x,color_type) || IS_CACHED_VIDEO(x,color_type))
#define IS_CACHED_VIDEO_TEX(x,color_type)		(typeid(x)==typeid(TexCachedBufferedVideo<color_type>))
#define IS_CACHED_VIDEO(x,color_type)			(typeid(x)==typeid(CachedBufferedVideo<color_type>))









template <class color_type>
class ImageSequence: public BufferedVideo<color_type> {
protected:	
	char *filename;
	int start_index;
	Bitmap<color_type> *tmp_buffer;

	virtual void LoadBitmap(LONG n);
public:
	ImageSequence();
	ImageSequence(char *filename,int start_index=0);
	~ImageSequence();



	bool GetFrame(LONG n,Bitmap<color_type> *img);
	BYTE *GetFrameReadOnly_(LONG n);										// Buffer di Bitmap<color_type>
	Bitmap<color_type> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow
};


template <class color_type>
class CachedImageSequence: public ImageSequence<color_type> {
	ObjectContainer_class *cache;

	static void *internal_LoadFrame(UINT index,CachedImageSequence<color_type> *src);
	static void internal_DeleteFrame(Bitmap<color_type> *obj,UINT index,CachedImageSequence<color_type> *src);
	void LoadBitmap(LONG n);
public:
	CachedImageSequence(char *filename,int start_index);
	~CachedImageSequence();

};


class MonochromeVideoSequence: public BufferedVideo<GreyLevel> {
protected:	
	IFileBuffer *source;
	Bitmap<GreyLevel> *tmp_buffer;
	UINT32 *buffer;
	int offset;

public:
	MonochromeVideoSequence(char *filename);
	~MonochromeVideoSequence();



	bool GetFrame(LONG n,Bitmap<GreyLevel> *img);
	BYTE *GetFrameReadOnly_(LONG n);										// Buffer di Bitmap<GreyLevel>
	Bitmap<GreyLevel> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow
};


class GrayScaleVideoSequence: public BufferedVideo<GreyLevel> {
protected:	
	IFileBuffer *source;
	Bitmap<GreyLevel> *tmp_buffer;
	int offset;

public:
	GrayScaleVideoSequence(char *filename);
	~GrayScaleVideoSequence();


	bool GetFrame(LONG n,Bitmap<GreyLevel> *img);
	BYTE *GetFrameReadOnly_(LONG n);										// Buffer di Bitmap<GreyLevel>
	Bitmap<GreyLevel> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow
};






#ifdef USE_OPENGL_LIBRARY

template <class color_type>
class AviVideoIn_OCV: public BufferedVideo<color_type> {
	void *cv_cap_filter;
	Bitmap<color_type> *tmp_buffer;

	// NULL frames
	BYTE *NULLBYTE_FRAME;
	Bitmap<color_type> *NULLBITMAP_FRAME;

	void *_GetFrame(LONG n);
public:
	AviVideoIn_OCV(char *filename,bool use_null_frames=false);				// num_frames = -1  -> File not exists
	~AviVideoIn_OCV();														// num_frames =  0  -> File exists but it has 0 frames or information not avaiable!!


	bool GetFrame(LONG n,Bitmap<color_type> *img);
	BYTE *GetFrameReadOnly_(LONG n);										// (BGR line aligned by 4 bytes)
	Bitmap<color_type> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow

	IplImage *GetFrameReadOnlyCV(LONG n);
};


//#define USE_FFMPEG_LIBRARY
#ifdef USE_FFMPEG_LIBRARY

template <class color_type>
class AviVideoIn_FFMPEG: public BufferedVideo<color_type> {
	void             *ffmpeg_info;

	void *_GetFrame(LONG n);
public:
	AviVideoIn_FFMPEG(char *filename,bool use_null_frames=false);			// num_frames = -1  -> File not exists
	~AviVideoIn_FFMPEG();													// num_frames =  0  -> File exists but it has 0 frames or information not avaiable!!



	bool GetFrame(LONG n,Bitmap<color_type> *img);
	BYTE *GetFrameReadOnly_(LONG n);										// (BGR line aligned by 4 bytes)
	Bitmap<color_type> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);		// Carica il frame dentro un oggetto RenderWindow
};

#endif
#endif






#define VIDEOTYPE_XVID_AVI       0 
#define VIDEOTYPE_MJPG_AVI		 1
#define VIDEOTYPE_LAGS_AVI       2
#define VIDEOTYPE_HFYU_AVI       3
#define VIDEOTYPE_PROMPT_AVI     4
#define VIDEOTYPE_MONOC_AVI      9 
#define VIDEOTYPE_BW_AVI		10  
#define VIDEOTYPE_IMAGE_SEQ     11 


class VideoWriter {
	int w,h;
	bool color;
	double fps;
	char *path;
	int type;

	void *video;
	int counter;
	Bitmap<GreyLevel> *tmpG;
	Bitmap<ColorRGB>  *tmpC;
	UINT32			  *tmpM;
	void			  *iplD;

public:
	VideoWriter(int w,int h,double fps,int num_frames,bool color,char *path,int type);
	~VideoWriter();

	bool isValid();
	void Save(Bitmap<ColorRGB> *I);
	void Save(Bitmap<GreyLevel> *I);
	void Close();
};




