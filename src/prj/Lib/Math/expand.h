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
// Maschera
//    ... 0         Non Presente   -1 Clean
//                                 -2 Ricoperto
//    0 ... 1.0     Presente       = Peso (solo con selezione 1 altrimenti va fino a inf)
//   

#define RICOPERTO -2


void Expand(Bitmap<ColorRGB> *TextureSimple,Bitmap<ColorFloat> *TextureMask,float Empty_flag=-1.0,float Filled_flag=RICOPERTO) {

	// Orizontal
	for(int j=0;j<TextureSimple->height;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->width;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(i,j)!=Empty_flag) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint(i-1,j,Filled_flag);
						TextureSimple->SetPoint(i-1,j,TextureSimple->Point(i,j));
					}
				}
			} else {
				if (TextureMask->Point(i,j)==Empty_flag) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(i,j,Filled_flag);
						TextureSimple->SetPoint(i,j,TextureSimple->Point(i-1,j));
					}
				}
			
			}
		}
	}


	// Vertical
	for(int j=0;j<TextureSimple->width;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->height;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(j,i)!=Empty_flag) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint(j,i-1,Filled_flag);
						TextureSimple->SetPoint(j,i-1,TextureSimple->Point(j,i));
					}
				}
			} else {
				if (TextureMask->Point(j,i)==Empty_flag) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(j,i,Filled_flag);
						TextureSimple->SetPoint(j,i,TextureSimple->Point(j,i-1));
					}
				}
			
			}
		}
	}

}


int Expand(Bitmap<ColorRGB> *TextureSimple,Bitmap<GreyLevel> *TextureMask) {
	int Collision=0;

	// Orizontal
	for(int j=0;j<TextureSimple->height;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->width;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(i,j)) {
					StatoIn=true;
					if (i!=0) {
						if (!TextureMask->Point(i-1,j)) {
							TextureMask->SetPoint(i-1,j,100);
							TextureSimple->SetPoint(i-1,j,TextureSimple->Point(i,j));
						} else Collision++;
					}
				}
			} else {
				if (!TextureMask->Point(i,j)) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(i,j,100);
						TextureSimple->SetPoint(i,j,TextureSimple->Point(i-1,j));
					}
				}
			
			}
		}
	}


	// Vertical
	for(int j=0;j<TextureSimple->width;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->height;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(j,i)) {
					StatoIn=true;
					if (i!=0) {
						if (!TextureMask->Point(j,i-1)) {
							TextureMask->SetPoint(j,i-1,100);
							TextureSimple->SetPoint(j,i-1,TextureSimple->Point(j,i));
						} else Collision++;
					}
				}
			} else {
				if (!TextureMask->Point(j,i)) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(j,i,100);
						TextureSimple->SetPoint(j,i,TextureSimple->Point(j,i-1));
					}
				}
			
			}
		}
	}

	return Collision;
}


void Expand(Bitmap<WI_Type> *TextureSimple,Bitmap<ColorFloat> *TextureMask) {

	// Orizontal
	for(int j=0;j<TextureMask->height;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureMask->width;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(i,j)!=-1.0) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint((i-1),j,RICOPERTO);
						TextureSimple->SetPoint(3*(i-1),j,TextureSimple->Point(3*i,j));
						TextureSimple->SetPoint(3*(i-1)+1,j,TextureSimple->Point(3*i+1,j));
						TextureSimple->SetPoint(3*(i-1)+2,j,TextureSimple->Point(3*i+2,j));
					}
				}
			} else {
				if (TextureMask->Point(i,j)==-1.0) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(i,j,RICOPERTO);
						TextureSimple->SetPoint(3*i,j,TextureSimple->Point(3*(i-1),j));
						TextureSimple->SetPoint(3*i+1,j,TextureSimple->Point(3*(i-1)+1,j));
						TextureSimple->SetPoint(3*i+2,j,TextureSimple->Point(3*(i-1)+2,j));
					}
				}
			
			}
		}
	}


	// Vertical
	for(int j=0;j<TextureMask->width;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureMask->height;i++) {
			if (!StatoIn) {
				if (TextureMask->Point(j,i)!=-1.0) {
					StatoIn=true;
					if (i!=0) {
						TextureMask->SetPoint(j,(i-1),RICOPERTO);
						TextureSimple->SetPoint(3*j,i-1,TextureSimple->Point(3*j,i));
						TextureSimple->SetPoint(3*j+1,i-1,TextureSimple->Point(3*j+1,i));
						TextureSimple->SetPoint(3*j+2,i-1,TextureSimple->Point(3*j+2,i));
					}
				}
			} else {
				if (TextureMask->Point(j,i)==-1.0) {
					StatoIn=false;
					if (i!=0) {
						TextureMask->SetPoint(j,i,RICOPERTO);
						TextureSimple->SetPoint(3*j,i,TextureSimple->Point(3*j,i-1));
						TextureSimple->SetPoint(3*j+1,i,TextureSimple->Point(3*j+1,i-1));
						TextureSimple->SetPoint(3*j+2,i,TextureSimple->Point(3*j+2,i-1));

					}
				}
			
			}
		}
	}

}


bool is128(ColorRGB x) {
	int r=(int)x.r-128;
	int g=(int)x.g-128;
	int b=(int)x.b-128;
	if (abs(r)+abs(g)+abs(b)<3*10) return true;
	return false;
}

int Expand(Bitmap<ColorRGB> *TextureSimple) {
	int Collision=0;

	// Orizontal
	for(int j=0;j<TextureSimple->height;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->width;i++) {
			if (!StatoIn) {
				if (!is128(TextureSimple->Point(i,j))) {
					StatoIn=true;
					if ((i>=1) && (i+1<TextureSimple->width)) {
						TextureSimple->SetPoint(i,j,TextureSimple->Point(i+1,j));
						TextureSimple->SetPoint(i-1,j,TextureSimple->Point(i,j));
					} 
				}
			} else {
				if (is128(TextureSimple->Point(i,j))) {
					StatoIn=false;
					if (i>=2) {
						TextureSimple->SetPoint(i,j,TextureSimple->Point(i-2,j));
						TextureSimple->SetPoint(i-1,j,TextureSimple->Point(i-2,j));
					}
				}
			
			}
		}
	}


	// Vertical
	for(int j=0;j<TextureSimple->width;j++) {
		bool StatoIn=false;
		for(int i=0;i<TextureSimple->height;i++) {
			if (!StatoIn) {
				if (!is128(TextureSimple->Point(j,i))) {
					StatoIn=true;
					if ((i>=1) && (i+1<TextureSimple->height)) {
						TextureSimple->SetPoint(j,i,TextureSimple->Point(j,i+1));
						TextureSimple->SetPoint(j,i-1,TextureSimple->Point(j,i));
					} 
				}
			} else {
				if (is128(TextureSimple->Point(j,i))) {
					StatoIn=false;
					if (i>=2) {
						TextureSimple->SetPoint(j,i,TextureSimple->Point(j,i-2));
						TextureSimple->SetPoint(j,i-1,TextureSimple->Point(j,i-2));
					}
				}
			
			}
		}
	}

	return Collision;
}


// Fill Holes 
/*
void ExpandFill(Bitmap<ColorRGB> *TextureSimple,Bitmap<ColorFloat> *TextureMask) {
	
	int H=TextureSimple->width;
	int W=TextureSimple->height;
	int h=0;
	int w=0;

	int line,ind;
	int start,end;

	for (line=h;line<H-h;line++) {
		ind=w;
		
		// Find start
		for(;ind<W-w;ind++)
			if (TextureMask->Point(line,ind)<0) break;
		if  (ind==W-w) continue;


		while(true) {	
			// Find Holes 
			for(;ind<W-w;ind++)
				if (TextureMask->Point(line,ind)>=0) break;
			if  (ind==W-w) break;
			start=ind;
			for(;ind<W-w;ind++)
				if (TextureMask->Point(line,ind)<0) break;
			if  (ind==W-w) break;
			end=ind-1;
			
			// [start,end]
			// [vstart,vend]
			float vstart=TextureMask->Point(line,start-1);
			float vend=TextureMask->Point(line,end+1);
			float  delta;
			
			if (start!=end) delta=(vend-vstart)/(end-start);
			else delta=0;

			for(ind=start;ind<=end;ind++) {
				//
				TextureMask->SetPoint(line,ind,(delta*(ind-start))+vstart);
				correct[line][ind]=true;
			}
		}
	}

	for (ind=w;ind<W-w;ind++) {
		line=h;
		
		// Find start
		for(;line<H-h;line++)
			if (disp[line][ind]>NOT_FOUND) break;
		if  (line==H-h) continue;


		while(true) {	
			// Find Holes 
			for(;line<H-h;line++)
				if (disp[line][ind]<=NOT_FOUND) break;
			if  (line==H-h) break;
			start=line;
			for(;line<H-h;line++)
				if (disp[line][ind]>NOT_FOUND) break;
			if  (line==H-h) break;
			end=line-1;
			
			// [start,end]
			// [vstart,vend]
			float vstart=disp[start-1][ind];
			float vend=disp[end+1][ind];
			float  delta;
			
			if (start!=end) delta=(vend-vstart)/(end-start);
			else delta=0;

			for(line=start;line<=end;line++) {
				disp[line][ind]=(delta*(line-start))+vstart;
				correct[line][ind]=true;
			}
		}
	}

	for (int times=0;times<20;times++) {
		for (line=h;line<H-h;line++) {
			for (ind=w;ind<W-w;ind++) {
				if (correct[line][ind]) {
					int num=0;
					float tot=0;
					for (int l=-1;l<=1;l++) {
						for (int g=-1;g<=1;g++) {
							if (disp[line+g][ind+l]>NOT_FOUND) {
								tot+=disp[line+g][ind+l];
								num++;
							}
						}
					}
					if (num!=0) disp[line][ind]=tot/num;
				}
			}
		}
	}
	
	swap(W,H);swap(w,h);
	
	for(int i=0;i<W;i++){
		delete []correct[i];
	}
	delete []correct;

	cout<<".\n";
}

*/

