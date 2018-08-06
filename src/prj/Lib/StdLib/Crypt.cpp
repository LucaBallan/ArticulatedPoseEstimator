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
#include "StdLib/Crypt.h"
using namespace std;



//
// Crittografia Vigenere (asimmetrica)
// 
void generate_map(int len,char first) {
	int map[256];
	
	srand(1000);

	for(int i=0;i<len;i++) {
		bool valid;
		int next;
		do {
			next=Approx(((len-1)*rand())/(RAND_MAX+1.0));
			valid=true;
			for(int j=0;j<i;j++) 
				if (map[j]==next) valid=false;
		} while(!valid); 
		map[i]=next;
		std::cout<<((char)(map[i]+first));
	}
}

char crypt[] = ")8mH$27]4=^3zsYaB!GrqR0(&JX6Ku+PE%jio/w{;AQ\\5`Txl[.Od,C'vhf-bpWM?*Vye@IFc#ZS|}t~\":gL1N< n9UD>k_";

char ascii_2_tb(char x) {
	if (x<' ') return x;
	if (x>'~') return x;
	// (x-' ') -> 0 -- 94
	return crypt[x-' '];
}

void crypt_strV(char *content) {
	//generate_map(95,' ');
	for(int i=0;i<(int)strlen(content);i++) 
		content[i]=ascii_2_tb(content[i]);
}





//
// Crittografia Xor (simmetrica)
// 
char *crypt_strX(const char *content,int count) {
	if (count==-1) count=(int)strlen(content);
	char *tmp_buffer=new char[count+1];
	char Pass[]="OpenGl 2.0 Error: Not using the correct FrameBuffer";
	int len=(int)strlen(Pass);
	for(int i=0;i<count;i++) tmp_buffer[i]=content[i]^Pass[i%len];
	tmp_buffer[count]=0;
	return tmp_buffer;
}

char *crypt_strX_readable(const char *content,int count,char *var_name) {
	char tmp[10];
	char Pass[]="OpenGl 2.0 Error: Not using the correct FrameBuffer";
	int len=(int)strlen(Pass);
	if (count==-1) count=(int)strlen(content);
	if (var_name==NULL) var_name="";
	char *out=new char[4*count+120+2*strlen(var_name)];
	out[0]=0;
	strcat(out,"char ");
	strcat(out,var_name);
	strcat(out,"[]=\"");
	for(int i=0;i<count;i++) {
		strcat(out,"\\x");
		_itoa(((int)content[i]^Pass[i%len]),tmp,16);
		strcat(out,tmp);
	}
	strcat(out,"\";\nint len_");
	strcat(out,var_name);
	strcat(out,"=");
	_itoa(count,tmp,10);
	strcat(out,tmp);
	strcat(out,";\n\n");
	
	return out;
}
char *crypt_file_readable(const char *filename,char *var_name) {
	char *content=TextFileRead(filename);
	if (!content) ErrorExit_str("File %s does not exists.",filename);
	char *crypted=crypt_strX_readable(content,-1,var_name);
	delete []content;
	return crypted;
}
char *crypt_file_include_readable(const char *filename,char *var_name) {
	char *content=TextFileRead_Include(filename);
	if (!content) ErrorExit_str("File %s does not exists.",filename);
	char *crypted=crypt_strX_readable(content,-1,var_name);
	delete []content;
	return crypted;
}












//
// Crittografia shaders
// Usage: 	e.g. CryptShaders("D:\\Luca\\Mot64\\data\\shaders"); -> shaders_lib.hpp
//
#define CRYPT(NAME,VAR_NAME) {sprintf(filename,"%s\\%s",directory,NAME);char *tmp=crypt_file_include_readable(filename,VAR_NAME);strcat(buffer,tmp);}

void CryptShaders(char *directory) {
	char filename[512];
	char *buffer=new char[1*1000000];
	buffer[0]=0;
	CRYPT("2DShader.vert.c","2DShader");
	CRYPT("ColorTransfer.frag.c","ColorTransfer");
	CRYPT("RT_Shader.vert.c","RT_Shader");
	CRYPT("fast_foreground.frag.c","fast_foreground");
	CRYPT("Convolution_O.frag.c","Convolution_O");
	CRYPT("Convolution_V.frag.c","Convolution_V");
	CRYPT("Alpha_Convolution_O.frag.c","Alpha_Convolution_O");
	CRYPT("Alpha_Convolution_V.frag.c","Alpha_Convolution_V");
	CRYPT("Morphological.frag.c","Morphological");
	CRYPT("Alpha_Morphological.frag.c","Alpha_Morphological");
	CRYPT("Alpha_Morphological_E.frag.c","Alpha_Morphological_E");
	CRYPT("fast_foreground_map.frag.c","fast_foreground_map");
	CRYPT("StdShader.vert.c","StdShader");
	CRYPT("OnLine_depth.frag.c","OnLine_depth");
	CRYPT("OnLine.frag.c","OnLine");
	CRYPT("pass_accurate.frag.c","pass_accurate");
	CRYPT("Image_Blend.frag.c","Image_Blend");
	CRYPT("MotionBlurShader.frag.c","MotionBlurShader");
	CRYPT("BlackHalo.frag.c","BlackHalo");
	CRYPT("TextureRenderer.frag.c","TextureRenderer");
	CRYPT("MeanPowMeanStd.frag.c","MeanPowMeanStd");
	CRYPT("MeanStd.frag.c","MeanStd");
	CRYPT("Shader_CameraProject.frag.c","Shader_CameraProject");
	CRYPT("ColorConversion.frag.c","ColorConversion");
	CRYPT("Matching_YH.frag.c","Matching_YH");
	CRYPT("Hatching_v.vert.c","Hatching_v");
	CRYPT("Hatching_f.frag.c","Hatching_f");
	CRYPT("Hatching2_f.frag.c","Hatching2_f");
	CRYPT("Toon_v.vert.c","Toon_v");
	CRYPT("Toon_v.frag.c","Toon_f");

	OFileBuffer X("shaders_lib.hpp");
	X.WriteString(buffer);
	X.Close();
	delete []buffer;
}











//
// Funzioni di protezione libreria
//
//char banner_c[]="\x2d\x9\x45\x2c\x26\x0\x4c\x53\x40\x10\x6c\x30\x11\x13\x4f\x40\xa\x10\x76";
//int  len_banner_c=19;
char banner_c[] = "\xc\x1f\x15\x17\x35\x5\x47\x5a\x5a\x10\x8\x6\x5b\x52\x5d\x42\xa\x15\x63\x5d\x44\x11\x40\x53\x25\x1b\x4\x41\x54\x2a\x4\x4c\xf\xe\x1c";
int  len_banner_c = 35;

char *getTitle(char *app_title) {
	char *x=crypt_strX(banner_c,len_banner_c);
	char *Text=new char[strlen(app_title)+len_banner_c+3];
	strcpy(Text,app_title);
	#ifndef NO_BANNER
	strcat(Text," ");
	strcat(Text,x);
	#endif
	return Text;
}


#include<conio.h>

bool InitGammaLibDone=false;
bool EnableRenderWindows=false;

void InitGammaLib(char *app_title, int argc, char* argv[]) {
	if (!InitGammaLibDone) {
		bool quit = false;
		bool info = false;
		for (int i = 1; i < argc; i++) {
			if ((stricmp(argv[i], "-help") == 0) ||
				(stricmp(argv[i], "--help") == 0) ||
				(stricmp(argv[i], "/?") == 0)) {
				info = true;
				quit = true;
			}
			if (stricmp(argv[i], "-?") == 0) {
				info = true;
			}
		}
		if (info) {
			cout << "  ________                                   .____     .__ ___.    " << endl;
			cout << " /  _____/ _____     _____    _____  _____   |    |    |__|\\_ |__  " << endl;
			cout << "/   \\  ___ \\__  \\   /     \\  /     \\ \\__  \\  |    |    |  | | __ \\ " << endl;
			cout << "\\\    \\_\\  \\ / __ \\_|  Y Y  \\|  Y Y  \\ / __ \\_|    |___ |  | | \\_\\ \\" << endl;
			cout << " \\______  /(____  /|__|_|  /|__|_|  /(____  /|_______ \\|__| |___  /" << endl;
			cout << "        \\/      \\/       \\/       \\/      \\/         \\/         \\/ " << endl;
			cout << endl;
			cout << endl;
			cout << "Copyright (C) 1998-2015 Luca Ballan   <ballanlu@gmail.com>" << endl;
			cout << "                                      http://lucaballan.altervista.org/" <<endl;
			cout << endl;
			cout << endl;
			cout << "Gammalib uses: ";
			#ifdef USE_OPENGL_LIBRARY
			cout << "OpenCV, ";
			#endif
			cout << "smctc, levmar, libelas, newmat, tesseract, lp_solve, gsl, glew, ";
			#ifdef USE_FIGLET
			cout << "figlet, ";
			#endif
			cout << "CLAPACK, ffmpeg." << endl;
			cout << endl;
			MediaPlayer sound;
			if (fileExists("HiroyukiIwatsuki.mp3")) {
				cout << "(Music by Hiroyuki Iwatsuki)" << endl;
				cout << endl;
				sound.loop = true;
				sound.Open(L"HiroyukiIwatsuki.mp3");
				sound.WaitReadyToPlay(INFINITE);
				sound.SetVolume(1.0);
				sound.Play(0.0);
			}
			_getch();
			sound.Stop();
		}
		if (quit) exit(0);

		if (!info) {
			int len = (int)strlen(app_title);
			int mask_size = max(len + 10, len_banner_c + 10);
			for (int i = 0; i < mask_size; i++) cout << "-";
			cout << "\n";
			cout << "--"; CenteredCout(app_title, mask_size - 4); cout << "--\n";
			#ifndef NO_BANNER
			char *x = crypt_strX(banner_c, len_banner_c);
			cout << "--"; CenteredCout(x, mask_size - 4); cout << "--\n";
			delete[]x;
			#endif
			for (int i = 0; i < mask_size; i++) cout << "-";
			cout << "\n";
		}

		InitGammaLibDone = true;

		// Standard Initialization
		Image::Init();
		Bone::InitBoneMesh();

		// Enable Lib Code
		EnableRenderWindows=true;
	}
}
