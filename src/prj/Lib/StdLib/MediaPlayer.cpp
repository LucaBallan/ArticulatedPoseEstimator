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





#include "stdInclude.h"
#include "common.h"
#include <shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>

#include "MediaPlayer.h"



HRESULT CreateTopologyFromSource(IMFTopology **ppTopology,IMFMediaSession *m_pSession,IMFMediaSource *m_pSource,HWND m_hwndVideo);


template <class T> 
void SRELEASE(T **ppT) {
	if (*ppT) {
		(*ppT)->Release();
		*ppT=NULL;
	}
}


class int_callback_class: public IMFAsyncCallback {

	long          RefCount;
	MediaPlayer  *father;

public:
	int_callback_class(MediaPlayer *father) {RefCount=1;this->father=father;};

	// IUnknown methods
	STDMETHODIMP_(ULONG) AddRef()                                {return InterlockedIncrement(&RefCount);}
	STDMETHODIMP_(ULONG) Release()                               {ULONG c=InterlockedDecrement(&RefCount);if (c==0) delete this; return c;};
	STDMETHODIMP         QueryInterface(REFIID riid,void **ppv)  {static const QITAB qit[]={QITABENT(int_callback_class,IMFAsyncCallback),{0}};return QISearch(this,qit,riid,ppv);};

	// IMFAsyncCallback methods
	STDMETHODIMP         GetParameters(DWORD*,DWORD*)            {return E_NOTIMPL;}
	STDMETHODIMP         Invoke(IMFAsyncResult* pAsyncResult);
};


MediaPlayer::MediaPlayer() {
	state = Closed;
	loop = false;
	CloseEvent=NULL;
	Session=NULL;
	Source=NULL;
	callback=NULL;
	VideoControl=NULL;
	VolumeControl=NULL;
	RateControl=NULL;


	HRESULT hr;
	hr=MFStartup(MF_VERSION);
	if (FAILED(hr)) ErrorExit("Cannot initialize Windows Media Foundation.");

	ReadyEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	CloseEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	callback=new int_callback_class(this);
}
MediaPlayer::~MediaPlayer() {
	Close();

	((int_callback_class*)callback)->Release();

	MFShutdown();
	CloseHandle(CloseEvent);
	CloseHandle(ReadyEvent);
}
bool MediaPlayer::Open(WCHAR *filename,HWND VideoHWND) {
	HRESULT            hr;
	IMFSourceResolver *tmp_SourceResolver = NULL;
	IUnknown          *tmp_source         = NULL;
	IMFTopology       *tmp_topology       = NULL;
	MF_OBJECT_TYPE     ObjectType         = MF_OBJECT_INVALID;


	Close();
    if (state!=Closed) return false;
	ResetEvent(ReadyEvent);


	// Session
	hr=MFCreateMediaSession(NULL,&Session);
    if (FAILED(hr)) return false;
    state=Ready;

    hr=Session->BeginGetEvent((IMFAsyncCallback*)callback,NULL);
	if (FAILED(hr)) {Close();return false;}



	// Source
	hr=MFCreateSourceResolver(&tmp_SourceResolver);
	if (FAILED(hr)) {
		Close();
		return false;
	}

	hr=tmp_SourceResolver->CreateObjectFromURL(filename,MF_RESOLUTION_MEDIASOURCE,NULL,&ObjectType,&tmp_source);
	if (FAILED(hr)) {
		SRELEASE(&tmp_SourceResolver);
		Close();
		return false;	
	}

	hr=tmp_source->QueryInterface(IID_PPV_ARGS(&Source));
	if (FAILED(hr)) {
		SRELEASE(&tmp_SourceResolver);
		SRELEASE(&tmp_source);
		Close();
		return false;	
	}
	SRELEASE(&tmp_SourceResolver);
	SRELEASE(&tmp_source);



	// Topology
	hr=CreateTopologyFromSource(&tmp_topology,Session,Source,VideoHWND);
	if (FAILED(hr))	{
		Close();
		return false;
	}

	hr=Session->SetTopology(0,tmp_topology);
	if (FAILED(hr))	{
		SRELEASE(&tmp_topology);
		Close();
		return false;
	}
	SRELEASE(&tmp_topology);

	state=OpenPending;

	return true;
}
bool MediaPlayer::Close() {
	if (state==Closed) return true;
	Stop();

	HRESULT hr;

    SRELEASE(&VideoControl);
	SRELEASE(&VolumeControl);
	SRELEASE(&RateControl);

    if (Session) {
		mf_player_state tmp=state;

		ResetEvent(CloseEvent);
		state=Closing;
        hr=Session->Close();
		if (FAILED(hr)) {state=tmp;return false;}

        WaitForSingleObject(CloseEvent,5000);
		ResetEvent(CloseEvent);
    }

    if (Source)     Source->Shutdown();
    if (Session)    Session->Shutdown();
    
	SRELEASE(&Source);
    SRELEASE(&Session);
	state=Closed;

	return true;
}
bool MediaPlayer::Play(double seconds) {
    if ((state!=Paused) && (state!=Ended)) return false;

	HRESULT hr;
    PROPVARIANT varStart;

	
	PropVariantInit(&varStart);
    varStart.vt=VT_EMPTY;
	varStart.vt=VT_I8;
	varStart.hVal.QuadPart=(__int64)((seconds*1000.0*1000.0*1000.0)/100);
	hr=Session->Start(&GUID_NULL, &varStart);
    PropVariantClear(&varStart);

	if (FAILED(hr)) return false;

	state=Playing;
	return true;
}
bool MediaPlayer::Stop() {
    if (state!=Playing) return false;
	if (state==Paused)  return true;
	
	HRESULT hr;
	hr=Session->Pause();
	if (FAILED(hr)) return false;
	
	state=Paused;
	return true;
}
bool MediaPlayer::WaitReadyToPlay(DWORD max_time_ms) {
	if (WaitForSingleObject(ReadyEvent,max_time_ms)==WAIT_TIMEOUT) return false;
	ResetEvent(ReadyEvent);
	return true;
}
double MediaPlayer::PlaybackTime() {
	if (state!=Playing) return -1.0;
	
	IMFClock             *clock=NULL;
	IMFPresentationClock *pclock=NULL;
	MFTIME                mf_time;

	Session->GetClock(&clock);
	clock->QueryInterface(IID_PPV_ARGS(&pclock));
	pclock->GetTime(&mf_time);
	pclock->Release();
	clock->Release();

	return (mf_time/(10.0*1000.0*1000.0));
}
void MediaPlayer::SetVolume(float volume) {
	UINT32 num_ch;

	if (VolumeControl) {
		VolumeControl->GetChannelCount(&num_ch);
		for(UINT32 i=0;i<num_ch;i++) VolumeControl->SetChannelVolume(i,volume);
	}
}
void MediaPlayer::SetRate(float rate) {
	if (RateControl) {
		RateControl->SetRate(false,rate);
	}
}
float MediaPlayer::GetRate() {
	if (!RateControl) return 1.0f;
	float rate;
	BOOL thin;
	RateControl->GetRate(&thin,&rate);
	return rate;
}
float MediaPlayer::GetVolume() {
	if (!VolumeControl) return 0.0f;
	float volume;
	VolumeControl->GetChannelVolume(0,&volume);
	return volume;
}
HRESULT int_callback_class::Invoke(IMFAsyncResult *pResult) {
	HRESULT          hr;
	IMFMediaEvent   *tmp_event      = NULL;
	HRESULT          hr_status      = S_OK;
	MF_TOPOSTATUS    topo_status    = MF_TOPOSTATUS_INVALID; 
	bool             get_next_event = false;
	MediaEventType   event_type;
	

	hr=father->Session->EndGetEvent(pResult,&tmp_event);
	if (FAILED(hr)) goto done;

	hr=tmp_event->GetType(&event_type);
	if (FAILED(hr)) goto done;

	get_next_event=true;

	hr=tmp_event->GetStatus(&hr_status);
	if (FAILED(hr))        goto done;
	if (FAILED(hr_status)) goto done;

	
	switch(event_type) {
		case MESessionClosed:
			SetEvent(father->CloseEvent);
			get_next_event=false;
			break;
		
		case MESessionTopologyStatus:
			hr=tmp_event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS,(UINT32*)&topo_status);
			if (FAILED(hr))	break;

			switch (topo_status) {
				case MF_TOPOSTATUS_READY:
				    SRELEASE(&father->VideoControl);
					SRELEASE(&father->VolumeControl);
					SRELEASE(&father->RateControl);
					MFGetService(father->Session,MR_VIDEO_RENDER_SERVICE,IID_PPV_ARGS(&father->VideoControl));
					MFGetService(father->Session,MR_STREAM_VOLUME_SERVICE,IID_PPV_ARGS(&father->VolumeControl));
					MFGetService(father->Session,MF_RATE_CONTROL_SERVICE,IID_PPV_ARGS(&father->RateControl));
					father->state=Paused;
					SetEvent(father->ReadyEvent);
					break;
				default:
					break;
			}
			break;
		case MEEndOfPresentation:
		    father->state=Ended;
			if (father->loop) father->Play(0.0);
			break;
		default:
			break;
	}


done:
	if (get_next_event)	father->Session->BeginGetEvent(this,NULL);
	SRELEASE(&tmp_event);
	return S_OK;
}
bool MediaPlayer::Repaint() {
	if (VideoControl) {
		if (VideoControl->RepaintVideo()==S_OK) return true;
		return false;
	}
	return true;
}
bool MediaPlayer::ResizeVideo(WORD width,WORD height) {
	if (VideoControl) {
		RECT rcDest = {0,0,width,height};
		if (VideoControl->SetVideoPosition(NULL, &rcDest) == S_OK) return true;
		return false;
	}
	return true;
}
bool MediaPlayer::HasVideo() {
	if (VideoControl) return true;
	return false;
}





//  Creates a source-stream node for a stream.
//
//  pSource: Pointer to the media source that contains the stream.
//  pSourcePD: Presentation descriptor for the media source.
//  pSourceSD: Stream descriptor for the stream.
//  ppNode: Receives a pointer to the new node.

HRESULT CreateSourceStreamNode(IMFMediaSource *pSource,IMFPresentationDescriptor *pSourcePD,IMFStreamDescriptor *pSourceSD,IMFTopologyNode **ppNode) {
	if (!pSource || !pSourcePD || !pSourceSD || !ppNode) {
		return E_POINTER;
	}

	HRESULT hr;
	IMFTopologyNode *pNode = NULL;
	

	// Create the source-stream node.
	hr=MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	if (FAILED(hr)) goto done;

	// Set attribute: Pointer to the media source.
	hr=pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
	if (FAILED(hr)) goto done;

	// Set attribute: Pointer to the presentation descriptor.
	hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pSourcePD);
	if (FAILED(hr)) goto done;

	// Set attribute: Pointer to the stream descriptor.
	hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSourceSD);
	if (FAILED(hr)) goto done;

	// Return the IMFTopologyNode pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

	done:
	SRELEASE(&pNode);
	return hr;
}




//  Creates an output node for a stream.
//
//  pSourceSD: Stream descriptor for the stream.
//  ppNode: Receives a pointer to the new node.
//
//  Notes:
//  This function does the following:
//  1. Chooses a renderer based on the media type of the stream.
//  2. Creates an IActivate object for the renderer.
//  3. Creates an output topology node.
//  4. Sets the IActivate pointer on the node.

HRESULT CreateOutputNode(IMFStreamDescriptor *pSourceSD,HWND hwndVideo,IMFTopologyNode **ppNode) {
	IMFTopologyNode *pNode = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFActivate *pRendererActivate = NULL;

	GUID guidMajorType = GUID_NULL;

	// Get the stream ID.
	DWORD streamID = 0;
	pSourceSD->GetStreamIdentifier(&streamID); // Just for debugging, ignore any failures.

	// Get the media type handler for the stream.
	HRESULT hr = pSourceSD->GetMediaTypeHandler(&pHandler);
	if (FAILED(hr)) goto done;

	// Get the major media type.
	hr = pHandler->GetMajorType(&guidMajorType);
	if (FAILED(hr)) goto done;

	// Create a downstream node.
	hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	if (FAILED(hr)) goto done;

	// Create an IMFActivate object for the renderer, based on the media type.
	if (MFMediaType_Audio == guidMajorType) {
		// Create the audio renderer.
		hr = MFCreateAudioRendererActivate(&pRendererActivate);
	} else if (MFMediaType_Video == guidMajorType) {
		// Create the video renderer.
		hr = MFCreateVideoRendererActivate(hwndVideo, &pRendererActivate);
	} else { 
		hr = E_FAIL;
	}
	if (FAILED(hr)) goto done;
	
	// Set the IActivate object on the output node.
	hr = pNode->SetObject(pRendererActivate);
	if (FAILED(hr)) goto done;

	// Return the IMFTopologyNode pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

	done:
	SRELEASE(&pNode);
	SRELEASE(&pHandler);
	SRELEASE(&pRendererActivate);
	return hr;
}



//  Adds a topology branch for one stream.
//
//  pTopology: Pointer to the topology object.
//  pSourcePD: The source's presentation descriptor.
//  iStream: Index of the stream to render.
//
//  Pre-conditions: The topology must be created already.
//
//  Notes: For each stream, we must do the following:
//    1. Create a source node associated with the stream.
//    2. Create an output node for the renderer.
//    3. Connect the two nodes.
//  The media session will resolve the topology, so we do not have
//  to worry about decoders or other transforms.

HRESULT AddBranchToPartialTopology(IMFTopology *pTopology,IMFPresentationDescriptor *pSourcePD,DWORD iStream,IMFMediaSource *m_pSource,HWND m_hwndVideo) {
	IMFStreamDescriptor  *pSourceSD   = NULL;
	IMFTopologyNode      *pSourceNode = NULL;
	IMFTopologyNode      *pOutputNode = NULL;
	BOOL                  fSelected   = FALSE;

	HRESULT hr=pSourcePD->GetStreamDescriptorByIndex(iStream,&fSelected,&pSourceSD);
	if (FAILED(hr)) goto done;

	if (fSelected) {
		hr=CreateSourceStreamNode(m_pSource,pSourcePD,pSourceSD,&pSourceNode);
		if (FAILED(hr)) goto done;

		hr=CreateOutputNode(pSourceSD,m_hwndVideo,&pOutputNode);
		if (FAILED(hr)) goto done;

		hr=pTopology->AddNode(pSourceNode);
		if (FAILED(hr)) goto done;

		hr=pTopology->AddNode(pOutputNode);
		if (FAILED(hr)) goto done;

		hr=pSourceNode->ConnectOutput(0,pOutputNode,0);
	}

	done:
	SRELEASE(&pSourceSD);
	SRELEASE(&pSourceNode);
	SRELEASE(&pOutputNode);

	return hr;
}

HRESULT CreateTopologyFromSource(IMFTopology **ppTopology,IMFMediaSession *m_pSession,IMFMediaSource *m_pSource,HWND m_hwndVideo) {
	HRESULT                     hr;
	IMFTopology                *pTopology      = NULL;
	IMFPresentationDescriptor  *pSourcePD      = NULL;
	DWORD                       cSourceStreams = 0;


	hr=MFCreateTopology(&pTopology);
	if (FAILED(hr)) goto done;

	hr=m_pSource->CreatePresentationDescriptor(&pSourcePD);
	if (FAILED(hr)) goto done;

	hr=pSourcePD->GetStreamDescriptorCount(&cSourceStreams);
	if (FAILED(hr)) goto done;

	for(DWORD i=0;i<cSourceStreams;i++) {
		hr=AddBranchToPartialTopology(pTopology,pSourcePD,i,m_pSource,m_hwndVideo);
		if (FAILED(hr)) goto done;
	}

	*ppTopology=pTopology;
	(*ppTopology)->AddRef();

	done:
	SRELEASE(&pTopology);
	SRELEASE(&pSourcePD);
	
	return hr;
}
