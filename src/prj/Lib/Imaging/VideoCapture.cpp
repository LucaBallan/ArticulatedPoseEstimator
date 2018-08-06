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





#include "stdafx.h"
#include "VideoCapture.h"


VideoCapture::VideoCapture() {
	hWndC=NULL;
}

VideoCapture::~VideoCapture() {
	StopCapture();
}

CaptureDeviceInfo *VideoCapture::EnumCaptureDevices() {
	CaptureDeviceInfo *DevInf=new CaptureDeviceInfo[MAX_CAPTURE_DRIVERS];
	for (int i=0;i<MAX_CAPTURE_DRIVERS;i++) strcpy(DevInf[i].szDeviceName,"NULL");

	char szDeviceName[80];
	char szDeviceVersion[80];

	for (i=0;i<MAX_CAPTURE_DRIVERS;i++) {
		if (capGetDriverDescription(i,szDeviceName,sizeof(szDeviceName),szDeviceVersion,sizeof(szDeviceVersion))) {
			strcpy(DevInf[i].szDeviceName,szDeviceName);
			strcpy(DevInf[i].szDeviceVersion,szDeviceVersion);
		}
	} 
	
	return DevInf;
}

static LRESULT PASCAL FrameCallbackProc(HWND hWnd,VIDEOHDR* hdr) { 

	if (!hWnd) return FALSE;

	VideoCapture *Source;
    Source=(VideoCapture *)capGetUserData(hWnd);
    
	Source->GrabbedFrame=hdr;

    return (LRESULT)TRUE; 
} 


// Rate and x,y!!!!
int VideoCapture::StartCapture(int DeviceIndex) {

	CAPDRIVERCAPS Caps;
	CAPTUREPARMS CParms;

	hWndC=capCreateCaptureWindow("CaptureWindow",WS_POPUP|WS_CHILD,0,0,640,480,0,0);
	
	if (!capDriverConnect(hWndC,DeviceIndex)) {
		DestroyWindow(hWndC);
		hWndC=NULL;
		return -1;
	}
	
	capDriverGetCaps(hWndC,&Caps,sizeof(CAPDRIVERCAPS));
	capCaptureGetSetup(hWndC,&CParms,sizeof(CAPTUREPARMS));
    capGetVideoFormat(hWndC,&CaptureFormat,sizeof(BITMAPINFO));


	capSetUserData(hWndC,this);						        // Ready for Callback
	capSetCallbackOnFrame(hWndC,FrameCallbackProc); 

	capPreviewScale(hWndC,FALSE);							// No Scaling
	capPreviewRate(hWndC,1);								// Display every 1 ms

	return 0;
}

void VideoCapture::GrabFrame() {
	capGrabFrameNoStop(hWndC);
}

char *VideoCapture::RetrieveFrame() {
	char *frame_data=(char*)GrabbedFrame->lpData;
	return frame_data;
}


void VideoCapture::StopCapture() {
	if (hWndC!=NULL) {
        capSetCallbackOnFrame(hWndC,NULL); 
		capDriverDisconnect(hWndC);
		DestroyWindow(hWndC);
	}
}

