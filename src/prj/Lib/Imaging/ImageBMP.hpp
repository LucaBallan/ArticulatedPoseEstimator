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





#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#define BMP_HEADERSIZE (3 * 2 + 4 * 12)
#define BMP_BYTESPERLINE (width, bits) ((((width) * (bits) + 31) / 32) * 4)
#define BMP_PIXELSIZE(width, height, bits) (((((width) * (bits) + 31) / 32) * 4) * height)


char *m_errorText;
DWORD m_bytesRead;

#ifndef SYSTEM_WINDOWS

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;

typedef struct tagBITMAP {
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    LPVOID      bmBits;
} BITMAP;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

#endif


////////////////////////////////////////////////////////////////////////////
//	load a .BMP file - 1,4,8,24 bit
//
//	allocates and returns an RGB buffer containing the image.
//	modifies width and height accordingly - NULL, 0, 0 on error

BYTE * LoadBMP(char *fileName, 
			   UINT *width, 
			   UINT *height) 
{

	BITMAP inBM;

	BYTE m1,m2;
    long filesize;
    short res1,res2;
    long pixoff;
    long bmisize;                    
    long compression;
    unsigned long sizeimage;
    long xscale, yscale;
    long colors;
    long impcol;
    

	BYTE *outBuf=NULL;
	
	// for safety
	*width=0; *height=0;

	// init
	m_errorText="OK";
	m_bytesRead=0;

	FILE *fp;
	
	fp=fopen(fileName,"rb");
	if (fp==NULL) {
		m_errorText="Can't open file for reading";
		return NULL;
	} else {
	    size_t rc;
		rc=fread((BYTE  *)&(m1),1,1,fp); m_bytesRead+=1;
		if (rc==-1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((BYTE  *)&(m2),1,1,fp); m_bytesRead+=1;
		if (rc==-1) m_errorText="Read Error!";
		/*if ((m1!='B') || (m2!='M')) {
			m_errorText="Not a valid BMP File";
			fclose(fp);
			return NULL;
        }*/
        
		////////////////////////////////////////////////////////////////////////////
		//
		//	read a ton of header stuff

		rc=fread((long  *)&(filesize),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((int  *)&(res1),2,1,fp); m_bytesRead+=2;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((int  *)&(res2),2,1,fp); m_bytesRead+=2;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(pixoff),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(bmisize),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(inBM.bmWidth),4,1,fp);	 m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(inBM.bmHeight),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((int  *)&(inBM.bmPlanes),2,1,fp); m_bytesRead+=2;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((int  *)&(inBM.bmBitsPixel),2,1,fp); m_bytesRead+=2;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(compression),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(sizeimage),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(xscale),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(yscale),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(colors),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		rc=fread((long  *)&(impcol),4,1,fp); m_bytesRead+=4;
		if (rc!=1) {m_errorText="Read Error!"; fclose(fp); return NULL;}

		////////////////////////////////////////////////////////////////////////////
		//	i don't do RLE files

		if (compression!=BI_RGB) {
	    	m_errorText="This is a compressed file.";
	    	fclose(fp);
	    	return NULL;
	    }

		if (colors == 0) {
			colors = 1 << inBM.bmBitsPixel;
		}


		////////////////////////////////////////////////////////////////////////////
		// read colormap

		RGBQUAD *colormap = NULL;

		switch (inBM.bmBitsPixel) {
		case 24:
			break;
			// read pallete 
		case 1:
		case 4:
		case 8:
			colormap = new RGBQUAD[colors];
			if (colormap==NULL) {
				fclose(fp);
				m_errorText="Out of memory";
				return NULL;
			}

			int i;
			for (i=0;i<colors;i++) {
				BYTE r,g,b, dummy;

				rc=fread((BYTE *)&(b),1,1,fp);
				m_bytesRead++;
				if (rc!=1) {
					m_errorText="Read Error!";	
					delete [] colormap;
					fclose(fp);
					return NULL;
				}

				rc=fread((BYTE  *)&(g),1,1,fp); 
				m_bytesRead++;
				if (rc!=1) {
					m_errorText="Read Error!";	
					delete [] colormap;
					fclose(fp);
					return NULL;
				}

				rc=fread((BYTE  *)&(r),1,1,fp); 
				m_bytesRead++;
				if (rc!=1) {
					m_errorText="Read Error!";	
					delete [] colormap;
					fclose(fp);
					return NULL;
				}


				rc=fread((BYTE  *)&(dummy),1,1,fp); 
				m_bytesRead++;
				if (rc!=1) {
					m_errorText="Read Error!";	
					delete [] colormap;
					fclose(fp);
					return NULL;
				}

				colormap[i].rgbRed=r;
				colormap[i].rgbGreen=g;
				colormap[i].rgbBlue=b;
			}
			break;
		}


		if ((long)m_bytesRead>pixoff) {
			fclose(fp);
			m_errorText="Corrupt palette";
			delete [] colormap;
			fclose(fp);
			return NULL;
		}

		while ((long)m_bytesRead<pixoff) {
			char dummy;
			fread(&dummy,1,1,fp);
			m_bytesRead++;
		}

		int w=inBM.bmWidth;
		int h=inBM.bmHeight;

		// set the output params
		*width=w;
		*height=h;

		long row_size = w * 3;

		long bufsize = (long)w * 3 * (long)h;

		////////////////////////////////////////////////////////////////////////////
		// alloc our buffer

		outBuf=(BYTE *) new BYTE [bufsize];
		if (outBuf==NULL) {
			m_errorText="Memory alloc Failed";
		} else {

			////////////////////////////////////////////////////////////////////////////
			//	read it

			long row=0;
			long rowOffset=0;

			// read rows in reverse order
			for (row=inBM.bmHeight-1;row>=0;row--) {

				// which row are we working on?
				rowOffset=(long unsigned)row*row_size;						      

				if (inBM.bmBitsPixel==24) {

					for (int col=0;col<w;col++) {
						long offset = col * 3;
						char pixel[3];

						if (fread((void  *)(pixel),1,3,fp)==3) {
							// we swap red and blue here
							*(outBuf + rowOffset + offset + 0)=pixel[2];		// r
							*(outBuf + rowOffset + offset + 1)=pixel[1];		// g
							*(outBuf + rowOffset + offset + 2)=pixel[0];		// b
						}

					}

					m_bytesRead+=row_size;
					
					// read DWORD padding
					while ((m_bytesRead-pixoff)&3) {
						char dummy;
						if (fread(&dummy,1,1,fp)!=1) {
							m_errorText="Read Error";
							delete [] outBuf;
							fclose(fp);
							return NULL;
						}

						m_bytesRead++;
					}
 
					
				} else {	// 1, 4, or 8 bit image

					////////////////////////////////////////////////////////////////
					// pixels are packed as 1 , 4 or 8 bit vals. need to unpack them

					int bit_count = 0;
					UINT mask = (1 << inBM.bmBitsPixel) - 1;

					BYTE inbyte=0;

					for (int col=0;col<w;col++) {
						
						int pix=0;

						// if we need another byte
						if (bit_count <= 0) {
							bit_count = 8;
							if (fread(&inbyte,1,1,fp)!=1) {
								m_errorText="Read Error";
								delete [] outBuf;
								delete [] colormap;
								fclose(fp);
								return NULL;
							}
							m_bytesRead++;
						}

						// keep track of where we are in the bytes
						bit_count -= inBM.bmBitsPixel;
						pix = ( inbyte >> bit_count) & mask;

						// lookup the color from the colormap - stuff it in our buffer
						// swap red and blue
						*(outBuf + rowOffset + col * 3 + 2) = colormap[pix].rgbBlue;
						*(outBuf + rowOffset + col * 3 + 1) = colormap[pix].rgbGreen;
						*(outBuf + rowOffset + col * 3 + 0) = colormap[pix].rgbRed;
					}

					// read DWORD padding
					while ((m_bytesRead-pixoff)&3) {
						char dummy;
						if (fread(&dummy,1,1,fp)!=1) {
							m_errorText="Read Error";
							delete [] outBuf;
							if (colormap)
								delete [] colormap;
							fclose(fp);
							return NULL;
						}
						m_bytesRead++;
					}
				}
			}
		
		}

		if (colormap) {
			delete [] colormap;
		}

		fclose(fp);

    }

	return outBuf;
}



//
//	swap Rs and Bs
//
//	Note! this does its stuff on buffers with a whole number of pixels
//	per data row!!
//

BOOL BGRFromRGB(BYTE *buf, UINT widthPix, UINT height)
{
	if (buf==NULL)
		return FALSE;

	UINT col, row;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 3 + col * 3;
			pGrn = buf + row * widthPix * 3 + col * 3 + 1;
			pBlu = buf + row * widthPix * 3 + col * 3 + 2;

			// swap red and blue
			BYTE tmp;
			tmp = *pRed;
			*pRed = *pBlu;
			*pBlu = tmp;
		}
	}
	return TRUE;
}

BOOL BGRAFromRGBA(BYTE *buf, UINT widthPix, UINT height)
{
	if (buf==NULL)
		return FALSE;

	UINT col, row;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 4 + col * 4;
			pGrn = buf + row * widthPix * 4 + col * 4 + 1;
			pBlu = buf + row * widthPix * 4 + col * 4 + 2;

			// swap red and blue
			BYTE tmp;
			tmp = *pRed;
			*pRed = *pBlu;
			*pBlu = tmp;
		}
	}
	return TRUE;
}

//
//	vertically flip a buffer 
//	note, this operates on a buffer of widthBytes bytes, not pixels!!!
//

BOOL VertFlipBuf(BYTE  * inbuf, 
	             UINT widthBytes, 
				 UINT height)
{   
	BYTE  *tb1;
	BYTE  *tb2;

	if (inbuf==NULL)
		return FALSE;

	UINT bufsize;

	bufsize=widthBytes;

	tb1= (BYTE *)new BYTE[bufsize];
	if (tb1==NULL) {
		return FALSE;
	}

	tb2= (BYTE *)new BYTE [bufsize];
	if (tb2==NULL) {
		delete [] tb1;
		return FALSE;
	}
	
	UINT row_cnt;     
	ULONG off1=0;
	ULONG off2=0;

	for (row_cnt=0;row_cnt<(height+1)/2;row_cnt++) {
		off1=row_cnt*bufsize;
		off2=((height-1)-row_cnt)*bufsize;   
		
		memcpy(tb1,inbuf+off1,bufsize);
		memcpy(tb2,inbuf+off2,bufsize);	
		memcpy(inbuf+off1,tb2,bufsize);
		memcpy(inbuf+off2,tb1,bufsize);
	}	

	delete [] tb1;
	delete [] tb2;

	return TRUE;
}        

//
// copies BYTE buffer into DWORD-aligned BYTE buffer
// return addr of new buffer
//

BYTE * MakeDwordAlignedBuf(BYTE *dataBuf,
						   UINT widthPix,				// pixels!!
						   UINT height,
						   UINT *uiOutWidthBytes)		// bytes!!!
{
	if (dataBuf==NULL)
		return NULL;

	////////////////////////////////////////////////////////////
	// how big is the smallest DWORD-aligned buffer that we can use?
	UINT uiWidthBytes;
	uiWidthBytes = WIDTHBYTES(widthPix * 24);

	DWORD dwNewsize=(DWORD)((DWORD)uiWidthBytes * 
							(DWORD)height);
	BYTE *pNew;

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
	pNew=(BYTE *)new BYTE[dwNewsize];
	if (pNew==NULL) {
		return NULL;
	}
	
	////////////////////////////////////////////////////////////
	// copy row-by-row
	UINT uiInWidthBytes = widthPix * 3;
	UINT uiCount;
	for (uiCount=0;uiCount < height;uiCount++) {
		BYTE * bpInAdd;
		BYTE * bpOutAdd;
		ULONG lInOff;
		ULONG lOutOff;

		lInOff=uiInWidthBytes * uiCount;
		lOutOff=uiWidthBytes * uiCount;

		bpInAdd= dataBuf + lInOff;
		bpOutAdd= pNew + lOutOff;

		memcpy(bpOutAdd,bpInAdd,uiInWidthBytes);
	}

	*uiOutWidthBytes=uiWidthBytes;
	return pNew;
}


//
// Reverse of MakeDwordAlignedBuf
// 
/*
BYTE *ReverseDwordAligned(BYTE *inBuf,
						  UINT widthPix,
						  UINT height)
{
	if (inBuf==NULL)
		return NULL;


	BYTE *tmp;
	tmp=(BYTE *)new BYTE[height * widthPix * 3];
	if (tmp==NULL)
		return NULL;

	UINT row;

	for (row=0;row<height;row++) {
		for (UINT k=0;k<widthPix;k++) {
			inBuf[(row*widthPix*4)+
		}
	}

/*	for (row=0;row<height;row++) {
		memcpy((tmp+row * widthPix * 3), 
				(inBuf + row * widthPix * 4), 
				widthPix * 3);
	}

	return tmp;
}*/

BYTE *DIB2RGB(BYTE *Buf,UINT width,UINT height) {
	
	//BYTE *nBuf=ReverseDwordAligned(Buf,width,height);
	BYTE *nBuf=new BYTE[3*width*height];
	memcpy(nBuf,Buf,3*width*height);

	VertFlipBuf(nBuf, width*3, height);
	
	BGRFromRGB(nBuf, width, height);
	
	return nBuf;
}

BYTE *RGB2DIB(BYTE *Buf,UINT width,UINT height,BITMAPINFOHEADER &bmiHeader,bool VertFlip) {

	UINT widthDW;

    if (Buf==NULL) return NULL;
	
		// a 24-bit DIB is DWORD-aligned, vertically flipped and 
		// has Red and Blue bytes swapped. we already did the 
		// RGB->BGR and the flip when we read the images, now do
		// the DWORD-align

	BYTE *nBuf=(BYTE *)new BYTE[width*height*3];
	memcpy(nBuf,Buf,width*height*3);

	BGRFromRGB(nBuf, width, height);
	
	if (VertFlip) VertFlipBuf(nBuf, width * 3, height);

    BYTE *tmp;
	// DWORD-align for display
	tmp = MakeDwordAlignedBuf(nBuf,
							  width,
							  height,
							  &widthDW);

	delete [] nBuf;

		// set up a DIB 
		bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiHeader.biWidth = width;
		bmiHeader.biHeight = height;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 24;
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = 0;
		bmiHeader.biXPelsPerMeter = 0;
		bmiHeader.biYPelsPerMeter = 0;
		bmiHeader.biClrUsed = 0;
		bmiHeader.biClrImportant = 0;

	return tmp;
}



////////////////////////////////////////////////////////////////////////////
//	write a 24-bit BMP file
//
//

void SaveBMP(char *fileName,		    // output path
			 BYTE * buf,				
			 UINT width,				// pixels
			 UINT height)
{
	
	BGRFromRGB(buf, width, height);
	VertFlipBuf(buf, width * 3, height);

	

	short res1=0;
    short res2=0;
    long pixoff=54;
    long compression=0;
    long cmpsize=0;
    long colors=0;
    long impcol=0;
	char m1='B';
	char m2='M';

	m_errorText="OK";

	DWORD widthDW = WIDTHBYTES(width * 24);

	long bmfsize=sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
  							widthDW * height;	
	long byteswritten=0;

	BITMAPINFOHEADER header;
  	header.biSize=40; 						// header size
	header.biWidth=width;
	header.biHeight=height;
	header.biPlanes=1;
	header.biBitCount=24;					// RGB encoded, 24 bit
	header.biCompression=BI_RGB;			// no compression
	header.biSizeImage=0;
	header.biXPelsPerMeter=0;
	header.biYPelsPerMeter=0;
	header.biClrUsed=0;
	header.biClrImportant=0;

	FILE *fp;	
	fp=fopen(fileName,"wb");
	if (fp==NULL) {
		m_errorText="Can't open file for writing";
		return;
	}

	// should probably check for write errors here...
	
	fwrite((BYTE  *)&(m1),1,1,fp); byteswritten+=1;
	fwrite((BYTE  *)&(m2),1,1,fp); byteswritten+=1;
	fwrite((long  *)&(bmfsize),4,1,fp);	byteswritten+=4;
	fwrite((int  *)&(res1),2,1,fp); byteswritten+=2;
	fwrite((int  *)&(res2),2,1,fp); byteswritten+=2;
	fwrite((long  *)&(pixoff),4,1,fp); byteswritten+=4;

	fwrite((BITMAPINFOHEADER *)&header,sizeof(BITMAPINFOHEADER),1,fp);
	byteswritten+=sizeof(BITMAPINFOHEADER);
	
	long row=0;
	long rowidx;
	long row_size;
	row_size=header.biWidth*3;
    size_t rc;
	for (row=0;row<header.biHeight;row++) {
		rowidx=(long unsigned)row*row_size;						      

		// write a row
		rc=fwrite((void  *)(buf+rowidx),row_size,1,fp);
		if (rc!=1) {
			m_errorText="fwrite error\nGiving up";
			break;
		}
		byteswritten+=row_size;	

		// pad to DWORD
		for (DWORD count=row_size;count<widthDW;count++) {
			char dummy=0;
			fwrite(&dummy,1,1,fp);
			byteswritten++;							  
		}

	}
           
	fclose(fp);
	
	VertFlipBuf(buf, width * 3, height);
	BGRFromRGB(buf, width, height);
}
/*
ColorRGBA *RGB2RGBA(ColorRGB *Img24bit,int width,int height) {
	BYTE *nBuf=new BYTE[width*height*4];

	UINT k=0;
	for(UINT j=0;j<height;j++) {
		for(UINT i=0;i<width;i++) {
			ColorRGB C=Img24bit[i+j*width];
			
			nBuf[k++]=C.r;
			nBuf[k++]=C.g;
			nBuf[k++]=C.b;
			nBuf[k++]=0;
		}
	}

	return (ColorRGBA*) nBuf;
}

ColorRGB *RGBA2RGB(ColorRGBA *Img32bit,int width,int height) {
	ColorRGB *nBuf=new ColorRGB[width*height];

	UINT k=0;
	for(UINT j=0;j<height;j++) {
		for(UINT i=0;i<width;i++) {
			ColorRGBA C=Img32bit[i+j*width];
			
			nBuf[k].r=C>>16;
			nBuf[k].g=(C>>8)&0xFF;
			nBuf[k].b=C&0xFF;
			k++;
		}
	}

	return nBuf;
}
*/
