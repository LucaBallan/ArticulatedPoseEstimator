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

/*
OffScreenRenderer::OffScreenRenderer(int width, int height,int channels) : GPGPU(width,height,channels), P(3,4) {
}

void OffScreenRenderer::InitFrame(View *V) {
	if ((V->width!=width) || (V->height!=height)) ErrorExit("View object must have the same size of the OffScreenRenderer object.");

	EnsureContext();
	Activate();
	glViewport(0,0,width,height);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
	// Standard ... no depth

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLdouble mat[16];
	mat[0]=2.0/width;mat[1]=0;mat[2]=0;mat[3]=0;			// NB: Confermato visivamente per essere coerente con projection
	mat[4]=0;mat[5]=2.0/height;mat[6]=0;mat[7]=0;			//     della classe View; ovvero Approx(Central_Projection(P*x))
	mat[8]=0;mat[9]=0;mat[10]=0;mat[11]=0;					// La conferma è stata data con GL_POINT, ovviamente le faccie (dato l'algoritmo interno di opengl)
	mat[12]=0;mat[13]=0;mat[14]=0;mat[15]=1;				// possono risultare un po' traslate ma la proiezione dei loro punti è esatta.

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(mat);
	P=V->P;
}

inline Vector<2> OffScreenRenderer::Transform(Vector<3> x) {
	Vector<3> V;
	Vector<2> T;
	Vector<4> omo;
	
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1;
	Multiply(&P,omo,V.GetList());

	T[0]=V[0]/V[2];
	T[1]=V[1]/V[2];

	T[0]-=(width-1)/2.0;					// NB: Confermato visivamente per essere coerente con projection
	T[1]-=(height-1)/2.0;					//     della classe View; ovvero Approx(Central_Projection(P*x))
											// La conferma è stata data con GL_POINT, ovviamente le faccie (dato l'algoritmo interno di opengl)
											// possono risultare un po' traslate ma la proiezione dei loro punti è esatta.
	return T;
}

inline void OffScreenRenderer::Vertex(Vector<2> P) {
	glVertex2d(P[0],P[1]);
}

void OffScreenRenderer::RenderMaskObject(View *V,HyperMesh<3> *mesh,Bitmap<GreyLevel> *I) {
	InitFrame(V);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glShadeModel(GL_FLAT);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(1.0,1.0,1.0);	
	{
		HyperFace<3> *f;
		List<typename Vector<3>> *P=&mesh->Points;
	
		glBegin(GL_TRIANGLES);
		for (int i=0;i<mesh->num_f;i++) {
			f=&mesh->Faces[i];
			Vertex(Transform((*P)[f->Point[0]]));
			Vertex(Transform((*P)[f->Point[1]]));
			Vertex(Transform((*P)[f->Point[2]]));
		}
		glEnd();
	}
	GetResult(I);
	DeActivate();
	wglMakeCurrent(NULL,NULL);
}






*/













/*
Vector<3> OffScreenRenderer::TransformZ(Vector<3> x) {
	Vector<3> V;
	Vector<3> T;
	Vector<4> omo;
	
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1;
	Multiply(&P,omo,V.GetList());

	T[0]=V[0]/V[2];
	T[1]=V[1]/V[2];
	T[2]=V[2];

	T[0]-=(width-1)/2.0;					// NB: Confermato visivamente per essere coerente con projection
	T[1]-=(height-1)/2.0;					//     della classe View; ovvero Approx(Central_Projection(P*x))
											// La conferma è stata data con GL_POINT, ovviamente le faccie (dato l'algoritmo interno di opengl)
											// possono risultare un po' traslate ma la proiezione dei loro punti è esatta.
	return T;
}

void OffScreenRenderer::RenderFrame(Bitmap<ColorRGB> *I) {
	GetResult(I);
	wglMakeCurrent(NULL,NULL);
}
void OffScreenRenderer::RenderFrame(Bitmap<GreyLevel> *I) {
	GetResult(I);
	wglMakeCurrent(NULL,NULL);
}
void OffScreenRenderer::DrawMesh(HyperMesh<3> *mesh) {
	HyperFace<3> *f;
	List<typename Vector<3>> *P=&mesh->Points;
	
	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		glNormal3dv(f->Normal(P).GetList());
		Vertex(Transform((*P)[f->Point[0]]));
		Vertex(Transform((*P)[f->Point[1]]));
		Vertex(Transform((*P)[f->Point[2]]));
	}
	glEnd();
}

void OffScreenRenderer::RenderTexturedObject(View *V,HyperMesh<3> *mesh,GLuint Map,Bitmap<ColorRGB> *I) {
	InitFrame(V);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glShadeModel(GL_SMOOTH);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,Map); 
	
	Vector<2> texpoint;
	HyperFace<3> *f,*texf;
	List<typename Vector<3>> *P=&mesh->Points;
	List<typename Vector<3>> *N=&mesh->Normals;
	
	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		texf=&(*(mesh->Texture_Face))[i];
		
		texpoint=(*mesh->Texture_Point)[texf->Point[0]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[0]].GetList());
		Vertex(Transform((*P)[f->Point[0]]));

		texpoint=(*mesh->Texture_Point)[texf->Point[1]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[1]].GetList());
		Vertex(Transform((*P)[f->Point[1]]));

		texpoint=(*mesh->Texture_Point)[texf->Point[2]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[2]].GetList());
		Vertex(Transform((*P)[f->Point[2]]));
	}
	glEnd();
	RenderFrame(I);
}
*/