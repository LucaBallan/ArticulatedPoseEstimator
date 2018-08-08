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





TextelRenderer::TextelRenderer(int width, int height, int n_input) {
	this->width=width;
	this->height=height;
	this->n_input=n_input;

	edge_l=new int[height];
	edge_r=new int[height];
	edge_lvc=new float*[height];
	edge_rvc=new float*[height];
	vc=new float[n_input];

	for(int i=0;i<height;i++) {
		edge_lvc[i]=new float[n_input];
		edge_rvc[i]=new float[n_input];
	}

}


TextelRenderer::~TextelRenderer() {
	if (edge_l!=NULL) delete []edge_l;
	if (edge_r!=NULL) delete []edge_r;

	for(int i=0;i<height;i++) {
		if (edge_lvc[i]!=NULL) delete []edge_lvc[i];
		if (edge_rvc[i]!=NULL) delete []edge_rvc[i];
	}

	if (edge_lvc!=NULL) delete []edge_lvc;
	if (edge_rvc!=NULL) delete []edge_rvc;

	if (vc!=NULL) delete []vc;
}



inline void TextelRenderer::getVC(float d,float *vc1,float *vc2) {
	for(int i=0;i<n_input;i++) {
		vc[i]=(vc2[i]-vc1[i])*d+(vc1[i]);
	}
}

inline void TextelRenderer::setVC(float d,float *vc1,float *vc2,float *vc_) {
	for(int i=0;i<n_input;i++) {
		vc_[i]=(vc2[i]-vc1[i])*d+(vc1[i]);
	}
}


void TextelRenderer::ScanEdge(int x1,int y1,int x2,int y2,
								    float *vc1,float *vc2) {


	int rx;
	int count;
	float g,mg;
	float x,mx;
	


	if (y2<y1) {
		swap(y1,y2);
		swap(x1,x2);
		swap(vc1,vc2);
	}

	if (y2!=y1) {
		mg = 1.0f/(y2-y1);
		mx = (x2-x1)*mg;
	} else mg=mx=0;

	
	x  = (float)x1;
	g  = 0.0f;
	

	if (y1<0) {
		x-=mx*y1;
		g-=mg*y1;
		y1=0;
	}
	if (y2>=height) y2=height-1;


	for(count=y1;count<=y2;count++) {
		rx=Approx(x);

		if (rx<edge_l[count]) {
			edge_l[count]=rx;
			setVC(g,vc1,vc2,edge_lvc[count]);
		}
		if(rx>edge_r[count]) {
			edge_r[count]=rx;
			setVC(g,vc1,vc2,edge_rvc[count]);
		}

		x+=mx;
		g+=mg;
	}

}



void TextelRenderer::DrawTextel(Triangle<typename TPoint2D<int>> *D,Triangle<float *> *Vect) {
	
	int i;
	int int_tmp;
	int min_y=height,max_y=0;


	for(i=0;i<3;i++) {
		int_tmp=D->Point[i].y;
		if (int_tmp<min_y) min_y=int_tmp;
		if (int_tmp>max_y) max_y=int_tmp;
	}

	if (min_y<0) min_y=0;
	if (max_y>=height) max_y=height-1;

	for(i=min_y;i<=max_y;i++) {
		edge_l[i]=width;
		edge_r[i]=0;
	}



	ScanEdge(D->Point[0].x,D->Point[0].y,D->Point[1].x,D->Point[1].y,Vect->Point[0],Vect->Point[1]);
	ScanEdge(D->Point[1].x,D->Point[1].y,D->Point[2].x,D->Point[2].y,Vect->Point[1],Vect->Point[2]);
	ScanEdge(D->Point[2].x,D->Point[2].y,D->Point[0].x,D->Point[0].y,Vect->Point[2],Vect->Point[0]);


	int count;
	float g,mg;

	for(i=min_y;i<=max_y;i++) {
		int sx=edge_l[i];
		int ex=edge_r[i];
	
		if(sx>ex) break;
		

		g=0.0f;
		if(sx!=ex) mg=1.0f/(ex-sx);
		else mg=0.0f;


		if (sx<0) {
			g-=mg*sx;
			sx=0;								
		}
		if (ex>=width) ex=width-1;
		
		
		for(count=sx;count<=ex;count++) {
			getVC(g,edge_lvc[i],edge_rvc[i]);
			PixelShader(count,i,vc);

			g+=mg;
		}
		
	}

}


