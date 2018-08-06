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





#include "lib.h"
using namespace std;


#ifdef USE_OPENNI

#ifdef _WIN64
	#pragma comment(lib,"..\\..\\..\\Common Libs\\OpenNI2_64\\Lib\\OpenNI2.lib")
	#include "..\..\..\Common Libs\OpenNI2_64\Include\OpenNI.h"
#else
	#pragma comment(lib,"..\\..\\..\\Common Libs\\OpenNI2_32\\Lib\\OpenNI2.lib")
	#include "..\..\..\Common Libs\OpenNI2_32\Include\OpenNI.h"
#endif


class DepthVideoIn: public BufferedVideo<ColorFloat> {
	void *m_device;
	void *m_depth;
	void *m_depthFrame;
    LONG last_requested_n;
	BYTE *last_requested_output;

	Bitmap<ColorFloat> *tmp_buffer;

public:
	DepthVideoIn(int device_number);
	~DepthVideoIn();

	bool GetFrame(LONG n,Bitmap<ColorFloat> *img);
	BYTE *GetFrameReadOnly_(LONG n);
	Bitmap<ColorFloat> *GetFrameReadOnly(LONG n);
	UINT LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options=0);
};



using namespace openni;

bool initialized=false;
openni::Array<DeviceInfo> deviceInfoList;


bool DepthVideoIn::GetFrame(LONG n,Bitmap<ColorFloat> *img) {
	openni::DepthPixel *pDepthRow=(openni::DepthPixel*)GetFrameReadOnly_(n);
	if (pDepthRow==NULL) return false;

	if (img->width!=width) return false;
	if (img->height!=height) return false;

	ColorFloat *out=img->getBuffer();

	openni::VideoFrameRef *depthf = static_cast<openni::VideoFrameRef*>(m_depthFrame);
	int rowSize=depthf->getStrideInBytes()/sizeof(openni::DepthPixel);

	for (int y=0;y<height;++y) {
		openni::DepthPixel *pDepth=pDepthRow;
		for (int x=0;x<width;++x,++pDepth,++out) *out=*pDepth;
		pDepthRow+=rowSize;
	}

	return true;
}
BYTE *DepthVideoIn::GetFrameReadOnly_(LONG n) {
	if (n==last_requested_n) return last_requested_output;
	last_requested_n=n;
	last_requested_output=NULL;

	openni::VideoStream   *depth  = static_cast<openni::VideoStream*>(m_depth);
	openni::VideoFrameRef *depthf = static_cast<openni::VideoFrameRef*>(m_depthFrame);
	if 	(depth==NULL) return NULL;

	int changedIndex;
	openni::Status rc=openni::OpenNI::waitForAnyStream(&depth,1,&changedIndex,TIMEOUT_FOREVER); // TODO***
	if (rc!=openni::STATUS_OK) {
		printf("Wait failed\n");
		return NULL;
	}
	if (changedIndex!=0) {
		printf("Wait error\n");
		return NULL;
	}

	depth->readFrame(depthf);

	if (!depthf->isValid()) return NULL;

	last_requested_output=const_cast<BYTE*>(reinterpret_cast<const BYTE*>(depthf->getData()));
	return last_requested_output;
}

Bitmap<ColorFloat> *DepthVideoIn::GetFrameReadOnly(LONG n) {
	NOT_IMPLEMENTED;
	return NULL;
}
UINT DepthVideoIn::LoadFrameToTexture(LONG n,RenderWindow<3> *w,int options) {
	NOT_IMPLEMENTED;
	return 0;
}

DepthVideoIn::DepthVideoIn(int device_number) {
	openni::Status rc;

	num_frames=-1;
	m_device=NULL;
	m_depth=NULL;
	m_depthFrame=NULL;
    last_requested_n=-1;
	last_requested_output=NULL;

	if (!initialized) {
		rc=openni::OpenNI::initialize();
		if (rc!=openni::STATUS_OK) {
			printf("OpenNI-> init error:\n%s\n", openni::OpenNI::getExtendedError());
			return;
		} 
		initialized=true;
	}


	// Enumerate devices
	if (deviceInfoList.getSize()==0) openni::OpenNI::enumerateDevices(&deviceInfoList);
	if (device_number>=deviceInfoList.getSize()) return;
	


	m_device                    = new openni::Device;
	m_depth                     = new openni::VideoStream;
	m_depthFrame                = new openni::VideoFrameRef;
	openni::Device      *device = static_cast<openni::Device*>(m_device);
	openni::VideoStream *depth  = static_cast<openni::VideoStream*>(m_depth);

	const char *deviceURI=deviceInfoList[device_number].getUri();
	rc=device->open(deviceURI);
	if (rc!=openni::STATUS_OK) {
		printf("OpenNI-> Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		return;
	}


	rc=depth->create(*device,openni::SENSOR_DEPTH);
	if (rc==openni::STATUS_OK) {
		rc=depth->start();
		if (rc!=openni::STATUS_OK) {
			printf("OpenNI-> Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
		}
	} else {
		printf("OpenNI-> Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}


	/*rc=color.create(device, openni::SENSOR_COLOR);
	if (rc==openni::STATUS_OK) {
		rc=color.start();
		if (rc!=openni::STATUS_OK) {
			printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
		}
	} else {
		printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}*/

	if (!depth->isValid()) {
		printf("OpenNI-> No valid streams.\n");
		return;
	}

	openni::VideoMode depthVideoMode = depth->getVideoMode();
	width  = depthVideoMode.getResolutionX();
	height = depthVideoMode.getResolutionY();
	bpp    = 32;
	fps    = 30.0; // TODO***

	num_frames=0;
}

DepthVideoIn::~DepthVideoIn() {

	openni::Device        *device =static_cast<openni::Device*>(m_device);
	openni::VideoStream   *depth  =static_cast<openni::VideoStream*>(m_depth);
	openni::VideoFrameRef *depthf = static_cast<openni::VideoFrameRef*>(m_depthFrame);
	// TODO*** some problems in these destructors
	// todo*** delete device
	if 	(depth!=NULL) {
		depth->destroy();
		delete depth;
	}
	m_depth=NULL;

	if 	(depthf!=NULL) {
		delete depthf;
	}
	depthf=NULL;

	if 	(device!=NULL) {
		delete device;
	}
	m_device=NULL;

	//openni::OpenNI::shutdown();
	//initialized=false;
}



#define MAX_DEPTH 10000

enum DisplayModes
{
	DISPLAY_MODE_OVERLAY,
	DISPLAY_MODE_DEPTH,
	DISPLAY_MODE_IMAGE
};

class SampleViewer
{
public:
	SampleViewer(const char* strSampleName, openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color);
	virtual ~SampleViewer();

	virtual openni::Status init(int argc, char **argv);
	virtual openni::Status run();	//Does not return

protected:
	virtual void display();
	virtual void displayPostDraw(){};	// Overload to draw over the screen image

	virtual void onKey(unsigned char key, int x, int y);

	virtual openni::Status initOpenGL(int argc, char **argv);
	void initOpenGLHooks();

	openni::VideoFrameRef		m_depthFrame;
	openni::VideoFrameRef		m_colorFrame;

	openni::Device&			m_device;
	openni::VideoStream&			m_depthStream;
	openni::VideoStream&			m_colorStream;
	openni::VideoStream**		m_streams;

private:
	SampleViewer(const SampleViewer&);
	SampleViewer& operator=(SampleViewer&);

	static SampleViewer* ms_self;
	static void glutIdle();
	static void glutDisplay();
	static void glutKeyboard(unsigned char key, int x, int y);

	float			m_pDepthHist[MAX_DEPTH];
	char			m_strSampleName[ONI_MAX_STR];
	unsigned int		m_nTexMapX;
	unsigned int		m_nTexMapY;
	DisplayModes		m_eViewState;
	openni::RGB888Pixel*	m_pTexMap;
	int			m_width;
	int			m_height;
};



#ifndef _CRT_SECURE_NO_DEPRECATE 
	#define _CRT_SECURE_NO_DEPRECATE 1
#endif


#if (ONI_PLATFORM == ONI_PLATFORM_MACOSX)
        #include <GLUT/glut.h>
#else
        #include <GL/glut.h>
#endif

#ifdef WIN32
#include <conio.h>
int wasKeyboardHit()
{
	return (int)_kbhit();
}

#else // linux

#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
int wasKeyboardHit()
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	// don't echo and don't wait for ENTER
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	
	// make it non-blocking (so we can check without waiting)
	if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK))
	{
		return 0;
	}

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf))
	{
		return 0;
	}

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

void Sleep(int millisecs)
{
	usleep(millisecs * 1000);
}
#endif // WIN32

void calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame)
{
	const openni::DepthPixel* pDepth = (const openni::DepthPixel*)frame.getData();
	// Calculate the accumulative histogram (the yellow display...)
	memset(pHistogram, 0, histogramSize*sizeof(float));
	int restOfRow = frame.getStrideInBytes() / sizeof(openni::DepthPixel) - frame.getWidth();
	int height = frame.getHeight();
	int width = frame.getWidth();

	unsigned int nNumberOfPoints = 0;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++pDepth)
		{
			if (*pDepth != 0)
			{
				pHistogram[*pDepth]++;
				nNumberOfPoints++;
			}
		}
		pDepth += restOfRow;
	}
	for (int nIndex=1; nIndex<histogramSize; nIndex++)
	{
		pHistogram[nIndex] += pHistogram[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (int nIndex=1; nIndex<histogramSize; nIndex++)
		{
			pHistogram[nIndex] = (256 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
		}
	}
}



#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	1024
#define TEXTURE_SIZE	512

#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

SampleViewer* SampleViewer::ms_self = NULL;

void SampleViewer::glutIdle()
{
	glutPostRedisplay();
}
void SampleViewer::glutDisplay()
{
	SampleViewer::ms_self->display();
}
void SampleViewer::glutKeyboard(unsigned char key, int x, int y)
{
	SampleViewer::ms_self->onKey(key, x, y);
}






SampleViewer::SampleViewer(const char* strSampleName, openni::Device& device, openni::VideoStream& depth, openni::VideoStream& color) :
	m_device(device), m_depthStream(depth), m_colorStream(color), m_streams(NULL), m_eViewState(DEFAULT_DISPLAY_MODE), m_pTexMap(NULL)

{
	ms_self = this;
	strncpy(m_strSampleName, strSampleName, ONI_MAX_STR);
}
SampleViewer::~SampleViewer()
{
	delete[] m_pTexMap;

	ms_self = NULL;

	if (m_streams != NULL)
	{
		delete []m_streams;
	}
}

openni::Status SampleViewer::init(int argc, char **argv)
{
	openni::VideoMode depthVideoMode;
	openni::VideoMode colorVideoMode;

	if (m_depthStream.isValid() && m_colorStream.isValid())
	{
		depthVideoMode = m_depthStream.getVideoMode();
		colorVideoMode = m_colorStream.getVideoMode();

		int depthWidth = depthVideoMode.getResolutionX();
		int depthHeight = depthVideoMode.getResolutionY();
		int colorWidth = colorVideoMode.getResolutionX();
		int colorHeight = colorVideoMode.getResolutionY();

		if (depthWidth == colorWidth &&
			depthHeight == colorHeight)
		{
			m_width = depthWidth;
			m_height = depthHeight;
		}
		else
		{
			printf("Error - expect color and depth to be in same resolution: D: %dx%d, C: %dx%d\n",
				depthWidth, depthHeight,
				colorWidth, colorHeight);
			return openni::STATUS_ERROR;
		}
	}
	else if (m_depthStream.isValid())
	{
		depthVideoMode = m_depthStream.getVideoMode();
		m_width = depthVideoMode.getResolutionX();
		m_height = depthVideoMode.getResolutionY();
	}
	else if (m_colorStream.isValid())
	{
		colorVideoMode = m_colorStream.getVideoMode();
		m_width = colorVideoMode.getResolutionX();
		m_height = colorVideoMode.getResolutionY();
	}
	else
	{
		printf("Error - expects at least one of the streams to be valid...\n");
		return openni::STATUS_ERROR;
	}

	m_streams = new openni::VideoStream*[2];
	m_streams[0] = &m_depthStream;
	m_streams[1] = &m_colorStream;

	// Texture map init
	m_nTexMapX = MIN_CHUNKS_SIZE(m_width, TEXTURE_SIZE);
	m_nTexMapY = MIN_CHUNKS_SIZE(m_height, TEXTURE_SIZE);
	m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];

	return initOpenGL(argc, argv);

}
openni::Status SampleViewer::run()	//Does not return
{
	glutMainLoop();

	return openni::STATUS_OK;
}
void SampleViewer::display()
{
	int changedIndex;
	openni::Status rc = openni::OpenNI::waitForAnyStream(m_streams, 2, &changedIndex);
	if (rc != openni::STATUS_OK)
	{
		printf("Wait failed\n");
		return;
	}

	switch (changedIndex)
	{
	case 0:
		m_depthStream.readFrame(&m_depthFrame); break;
	case 1:
		m_colorStream.readFrame(&m_colorFrame); break;
	default:
		printf("Error in wait\n");
	}

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	if (m_depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	// check if we need to draw image frame to texture
	if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
		m_eViewState == DISPLAY_MODE_IMAGE) && m_colorFrame.isValid())
	{
		const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + m_colorFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = m_colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

		for (int y = 0; y < m_colorFrame.getHeight(); ++y)
		{
			const openni::RGB888Pixel* pImage = pImageRow;
			openni::RGB888Pixel* pTex = pTexRow + m_colorFrame.getCropOriginX();

			for (int x = 0; x < m_colorFrame.getWidth(); ++x, ++pImage, ++pTex)
			{
				*pTex = *pImage;
			}

			pImageRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	// check if we need to draw depth frame to texture
	if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
		m_eViewState == DISPLAY_MODE_DEPTH) && m_depthFrame.isValid())
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + m_depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = m_depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

		for (int y = 0; y < m_depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + m_depthFrame.getCropOriginX();

			for (int x = 0; x < m_depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = m_pDepthHist[*pDepth];
					pTex->r = nHistValue;
					pTex->g = nHistValue;
					pTex->b = 0;
				}
			}

			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);

	int nXRes = m_width;
	int nYRes = m_height;

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();

	// Swap the OpenGL display buffers
	glutSwapBuffers();

}

void SampleViewer::onKey(unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
	case 27:
		m_depthStream.stop();
		m_colorStream.stop();
		m_depthStream.destroy();
		m_colorStream.destroy();
		m_device.close();
		openni::OpenNI::shutdown();

		exit (1);
	case '1':
		m_eViewState = DISPLAY_MODE_OVERLAY;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
		break;
	case '2':
		m_eViewState = DISPLAY_MODE_DEPTH;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
		break;
	case '3':
		m_eViewState = DISPLAY_MODE_IMAGE;
		m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
		break;
	case 'm':
		m_depthStream.setMirroringEnabled(!m_depthStream.getMirroringEnabled());
		m_colorStream.setMirroringEnabled(!m_colorStream.getMirroringEnabled());
		break;
	}

}

openni::Status SampleViewer::initOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow (m_strSampleName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	initOpenGLHooks();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	return openni::STATUS_OK;

}
void SampleViewer::initOpenGLHooks()
{
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);
}
/*

void glutIdle() {
	glutPostRedisplay();
}
void glutDisplay() {
	int changedIndex;
	openni::Status rc = openni::OpenNI::waitForAnyStream(m_streams, 2, &changedIndex);
	if (rc != openni::STATUS_OK)
	{
		printf("Wait failed\n");
		return;
	}

	switch (changedIndex)
	{
	case 0:
		m_depthStream.readFrame(&m_depthFrame); break;
	case 1:
		m_colorStream.readFrame(&m_colorFrame); break;
	default:
		printf("Error in wait\n");
	}

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	if (m_depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	// check if we need to draw image frame to texture
	if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
		m_eViewState == DISPLAY_MODE_IMAGE) && m_colorFrame.isValid())
	{
		const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + m_colorFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = m_colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

		for (int y = 0; y < m_colorFrame.getHeight(); ++y)
		{
			const openni::RGB888Pixel* pImage = pImageRow;
			openni::RGB888Pixel* pTex = pTexRow + m_colorFrame.getCropOriginX();

			for (int x = 0; x < m_colorFrame.getWidth(); ++x, ++pImage, ++pTex)
			{
				*pTex = *pImage;
			}

			pImageRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	// check if we need to draw depth frame to texture
	if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
		m_eViewState == DISPLAY_MODE_DEPTH) && m_depthFrame.isValid())
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + m_depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = m_depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

		for (int y = 0; y < m_depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + m_depthFrame.getCropOriginX();

			for (int x = 0; x < m_depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					int nHistValue = m_pDepthHist[*pDepth];
					pTex->r = nHistValue;
					pTex->g = nHistValue;
					pTex->b = 0;
				}
			}

			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);

	int nXRes = m_width;
	int nYRes = m_height;

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();

	// Swap the OpenGL display buffers
	glutSwapBuffers();
}*/

void ChangeDet(Bitmap<ColorFloat> *A,Bitmap<ColorFloat> *Mean,Bitmap<ColorFloat> *Std,Bitmap<ColorFloat> *CD,ColorFloat sigma_multiplier_th,ColorFloat no_data_value=0.0,ColorFloat no_change_value=-1.0) {
	int size_p=A->width*A->height;

	ColorFloat *p_A=A->getBuffer();
	ColorFloat *p_B=Mean->getBuffer();
	ColorFloat *p_S=Std->getBuffer();
	ColorFloat *p_C=CD->getBuffer();

	for(int i=0;i<size_p;i++,p_A++,p_B++,p_S++,p_C++) {
		if (((*p_A)==no_data_value) || ((*p_B)==no_data_value)) {
			(*p_C)=no_data_value;
			continue;
		}
		ColorFloat tmp;
		if ((*p_A)>(*p_B)) tmp=(*p_A)-(*p_B);
		else tmp=(*p_B)-(*p_A);
		if (tmp>(*p_S)*sigma_multiplier_th) {
			*p_C=*p_A;
		} else {
			*p_C=no_change_value;
		}
	}
}

void build_bg(Video<ColorFloat> *video_in,Bitmap<ColorFloat> *buffer,int start_frame_index,int n_samples,int sleep_time_ms,Bitmap<ColorFloat> *BGMean,Bitmap<ColorFloat> *BGStd,ColorFloat no_data_value) {
	
	MeanStdSoftRenderer Bg(NULL,video_in->width,video_in->height);
	for(int i=0;i<n_samples;i++,Sleep(sleep_time_ms)) {
		video_in->GetFrame(start_frame_index+i,buffer);
		Bg.Update(buffer,no_data_value);
	}
	Bg.ExtractMeanStd(BGMean,BGStd,no_data_value);
}


void Kinect_Test() {
	/*
	openni::Status rc = openni::STATUS_OK;
	openni::Device device;
	openni::VideoStream depth, color;
	const char* deviceURI = openni::ANY_DEVICE;

	rc = openni::OpenNI::initialize();

	printf("After initialization:\n%s\n", openni::OpenNI::getExtendedError());

	rc = device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		printf("SimpleViewer: Device open failed:\n%s\n", openni::OpenNI::getExtendedError());
		openni::OpenNI::shutdown();
		return;
	}

	rc = depth.create(device, openni::SENSOR_DEPTH);
	if (rc == openni::STATUS_OK)
	{
		rc = depth.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start depth stream:\n%s\n", openni::OpenNI::getExtendedError());
			depth.destroy();
		}
	}
	else
	{
		printf("SimpleViewer: Couldn't find depth stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	rc = color.create(device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = color.start();
		if (rc != openni::STATUS_OK)
		{
			printf("SimpleViewer: Couldn't start color stream:\n%s\n", openni::OpenNI::getExtendedError());
			color.destroy();
		}
	}
	else
	{
		printf("SimpleViewer: Couldn't find color stream:\n%s\n", openni::OpenNI::getExtendedError());
	}

	if (!depth.isValid() || !color.isValid())
	{
		printf("SimpleViewer: No valid streams. Exiting\n");
		openni::OpenNI::shutdown();
		return;
	}

	SampleViewer sampleViewer("Simple Viewer", device, depth, color);

	int argc=1;
	char **argv=new char*[3];
	argv[0]=new char[1];
	argv[0][0]=0;
	argv[1]=NULL;
	rc = sampleViewer.init(argc,argv);
	if (rc != openni::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		return;
	}
	sampleViewer.run();
	openni::OpenNI::shutdown();
	*/
	
	
	ColorFloat no_data_value=0.0;
	DepthVideoIn M(0);
	
	Bitmap<ColorFloat> buffer(M.width,M.height);
	Bitmap<ColorFloat> BGMean(M.width,M.height);
	Bitmap<ColorFloat> BGStd(M.width,M.height);
	Bitmap<ColorFloat> CD(M.width,M.height);

	Bitmap<ColorFloat> BGMean2(M.width,M.height);
	Bitmap<ColorFloat> BGStd2(M.width,M.height);

	getch();
	Sleep(2000);
	build_bg(&M,&buffer,0,100,50,&BGMean,&BGStd,no_data_value);
	BGMean.Save("mean.png");
	BGStd.Save("std.png");

	DepthVideoIn *video_in=&M;
	cout<<"go"<<endl;
	getch();
	MeanStdSoftRenderer Smooth(NULL,video_in->width,video_in->height);
	for(int i=0;i<100;i++) {
		Smooth.Clear();
		for(int j=0;j<3;j++) {
			M.GetFrame(j,&buffer);
			Smooth.Update(&buffer,no_data_value);
		}
		Smooth.ExtractMeanStd(&BGMean2,&BGStd2,no_data_value);
		char data[100];
		sprintf(data,"raw%0i.png",i);
		BGMean2.Save(data);

		// TODO ** window based comparison
		ChangeDet(&BGMean2,&BGMean,&BGStd,&CD,7.0,0.0,-1.0);

		// TODO** filter blobs
		// graphcut ** edges and connectivity
		// erode and dilate
		sprintf(data,"luca%0i.png",i);
		CD.SAVE_FLOAT_MIN_VAL=0.0;
		//CD.SAVE_FLOAT_MAX_VAL=
		CD.Save(data);
	}


	/*

	int argc=1;
	char **argv=new char*[3];
	argv[0]=new char[1];
	argv[0][0]=0;
	argv[1]=NULL;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow (m_strSampleName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	//glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glutMainLoop();*/

}





#endif



