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





enum mf_player_state {
	Closed=0,       // No session.
	Ready,          // Session was created, ready to open a file
	OpenPending,    // Session is opening a file
	Paused,         // Session is paused/stopped  (ready to play)
	Playing,        // Session is playing
	Ended,          // Play has reach the end of the media (ready to play)
	Closing         // Application has closed the session, but is waiting for MESessionClosed
};


class MediaPlayer {
public:
	mf_player_state          state;
	HANDLE                   CloseEvent;
	HANDLE                   ReadyEvent;
	IMFMediaSession         *Session;
	IMFMediaSource          *Source;
	IMFVideoDisplayControl  *VideoControl;
	IMFAudioStreamVolume    *VolumeControl;
	IMFRateControl          *RateControl;
	void                    *callback;


public:
	MediaPlayer();
    ~MediaPlayer();
	

    // General
	bool    Open (WCHAR *filename,HWND VideoHWND=NULL);      // return false if the file has a video but no VideoHWND is specified
	bool    Play (double seconds);
	bool    Stop ();
	bool    Close();
	bool    WaitReadyToPlay(DWORD max_time_ms);
	double  PlaybackTime();                                  // time in seconds
	void    SetVolume(float volume);                         // volume [0.0 - 1.0]
	float   GetVolume();									 // for stereo it reads only the left
	void    SetRate(float rate=1.0);
	float   GetRate();

	// Video
	bool    HasVideo();                                      // valid only after being in Paused state (not in OpenPending)
	bool    Repaint();
	bool    ResizeVideo(WORD width,WORD height);
	bool    loop;
};


