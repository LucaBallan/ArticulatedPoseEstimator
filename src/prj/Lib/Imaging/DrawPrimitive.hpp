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





//***********************************************************************************************************************
//
//							Gestione primitive di disegno 3D
//
//***********************************************************************************************************************

inline void SetColor_(int color) {
	glColor3ubv((GLubyte*)(&color));
}

inline void SetColor_(int color,float transparency) {
	transparency*=255.0f;
	if (transparency>255.0f) transparency=255.0f;
	if (transparency<0.0f) transparency=0.0f;

	glColor4ub(((color>>16)&0xFF),((color>>8)&0xFF),(color&0xFF),(BYTE)transparency);
}

template <int dim> 
void RenderWindow<dim>::SetColor(int color,float transparency) {
	transparency*=255.0f;
	if (transparency>255.0f) transparency=255.0f;
	if (transparency<0.0f) transparency=0.0f;

	glColor4ub(((color>>16)&0xFF),((color>>8)&0xFF),(color&0xFF),(BYTE)transparency);
}

template <> 
inline void RenderWindow<2>::Vertex(Vector<2> P) {
	glVertex2d(P[0],P[1]);
}

template <> 
inline void RenderWindow<3>::Vertex(Vector<3> P) {
	glVertex3d(P[0],P[1],P[2]);
}

template <int dim> 
void RenderWindow<dim>::DrawPoint(Vector<dim> P,int Color,float size) {
	glPointSize(size);
	glColor4ubv((GLubyte*)(&Color));
	glBegin(GL_POINTS);
	Vertex(P);
	glEnd();
}

template <int dim> 
void RenderWindow<dim>::DrawLine(Vector<dim> A,Vector<dim> B,int Color,float size) {
	glLineWidth(size);
	glColor3ubv((GLubyte*)(&Color)); 
	glBegin(GL_LINES);
	Vertex(A);
	Vertex(B);
	glEnd();
}
template <int dim> 
void RenderWindow<dim>::DrawCircle(Vector<dim> C,double radius,int BorderColor,int FillColor,float border_size,Vector<dim> dir_x,Vector<dim> dir_y,float precision) {
	Vector<dim> tmp;

	glColor3ubv((GLubyte*)(&FillColor)); 
    glBegin(GL_TRIANGLE_FAN);
	Vertex(C);    
	for(float i=0.0;i<=2*M_PI+precision;i+=precision) {
		tmp=sin(i)*radius*dir_x+cos(i)*radius*dir_y;
		Vertex(C+tmp);
	}
    glEnd();

	glDepthFunc(GL_LEQUAL);
	glLineWidth(border_size);
	glColor3ubv((GLubyte*)(&BorderColor)); 
	glBegin(GL_LINE_LOOP);
	for(float i=0.0;i<2*M_PI;i+=precision) {
		tmp=sin(i)*radius*dir_x+cos(i)*radius*dir_y;
		Vertex(C+tmp);
	}
	glEnd();
	glDepthFunc(GL_LESS);

	glLineWidth(1.0);
}

template <int dim> 
void RenderWindow<dim>::DrawLine(Vector<dim> A,Vector<dim> B,float size) {
	glLineWidth(size);
	glBegin(GL_LINES);
	Vertex(A);
	Vertex(B);
	glEnd();
}

template <> 
void RenderWindow<3>::DrawHyperMesh(HyperMesh<3> *mesh) {
	//List<typename Vector<3>> *P=&mesh->Points;
	//List<typename Vector<3>> *N=&mesh->Normals;
	HyperFace<3> *f=mesh->Faces.getMem();
	Vector<3> *P=mesh->Points.getMem();
	Vector<3> *N=mesh->Normals.getMem();
	
	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++,f++) {
		//f=&mesh->Faces[i];
		//glNormal3dv((*N)[f->Point[0]].GetList());
		//Vertex((*P)[f->Point[0]]);
		//glNormal3dv((*N)[f->Point[1]].GetList());
		//Vertex((*P)[f->Point[1]]);
		//glNormal3dv((*N)[f->Point[2]].GetList());
		//Vertex((*P)[f->Point[2]]);

		glNormal3dv(N[f->Point[0]].GetList());
		Vertex(P[f->Point[0]]);
		glNormal3dv(N[f->Point[1]].GetList());
		Vertex(P[f->Point[1]]);
		glNormal3dv(N[f->Point[2]].GetList());
		Vertex(P[f->Point[2]]);
	}
	glEnd();
}

template <> 
void RenderWindow<3>::DrawHyperMesh_Flat(HyperMesh<3> *mesh) {
	HyperFace<3> *f=mesh->Faces.getMem();
	List<typename Vector<3>> *P_list=&mesh->Points;
	Vector<3> *P=mesh->Points.getMem();

	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++,f++) {
		//f=&mesh->Faces[i];
		glNormal3dv(f->Normal(P_list).GetList());
		//Vertex((*P)[f->Point[0]]);
		//Vertex((*P)[f->Point[1]]);
		//Vertex((*P)[f->Point[2]]);
		Vertex(P[f->Point[0]]);
		Vertex(P[f->Point[1]]);
		Vertex(P[f->Point[2]]);
	}
	glEnd();
}

template <> 
void RenderWindow<3>::DrawHyperMesh(HyperMesh<3> *mesh,void(*vertex_properties)(RenderWindow<3> *w,HyperMesh<3> *mesh,int vertex_index,void *data),void *data) {
	HyperFace<3> *f;
	List<typename Vector<3>> *P=&mesh->Points;
	List<typename Vector<3>> *N=&mesh->Normals;

	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		
		glNormal3dv((*N)[f->Point[0]].GetList());
		vertex_properties(this,mesh,f->Point[0],data);
		Vertex((*P)[f->Point[0]]);
		glNormal3dv((*N)[f->Point[1]].GetList());
		vertex_properties(this,mesh,f->Point[1],data);
		Vertex((*P)[f->Point[1]]);
		glNormal3dv((*N)[f->Point[2]].GetList());
		vertex_properties(this,mesh,f->Point[2],data);
		Vertex((*P)[f->Point[2]]);
	}
	glEnd();
}

template <> 
void RenderWindow<3>::DrawHyperMesh_Flat(HyperMesh<3> *mesh,void(*face_properties)(RenderWindow<3> *w,HyperMesh<3> *mesh,int face_index,void *data),void *data) {
	HyperFace<3> *f;
	List<typename Vector<3>> *P=&mesh->Points;
	
	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		
		glNormal3dv(f->Normal(P).GetList());
		face_properties(this,mesh,i,data);
		Vertex((*P)[f->Point[0]]);
		Vertex((*P)[f->Point[1]]);
		Vertex((*P)[f->Point[2]]);
	}
	glEnd();
}



template <> 
void RenderWindow<2>::DrawHyperMesh(HyperMesh<2> *mesh) {
	HyperFace<2> *f;
	List<typename Vector<2>> *P=&mesh->Points;
	
	glBegin(GL_LINES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		Vertex((*P)[f->Point[0]]);
		Vertex((*P)[f->Point[1]]);
	}
	glEnd();
}

template <> 
void RenderWindow<2>::DrawHyperMesh_Flat(HyperMesh<2> *mesh) {
	HyperFace<2> *f;
	List<typename Vector<2>> *P=&mesh->Points;
	
	glBegin(GL_LINES);
	for (int i=0;i<mesh->num_f;i++) {
		f=&mesh->Faces[i];
		Vertex((*P)[f->Point[0]]);
		Vertex((*P)[f->Point[1]]);
	}
	glEnd();
}


template <> 
void RenderWindow<3>::DrawHyperMesh_Textured(HyperMesh<3> *mesh) {
	GLboolean sav_state_gltexture2D=glIsEnabled(GL_TEXTURE_2D);
	
	Vector<2> texpoint;
	HyperFace<3> *f,*texf;
	List<typename Vector<3>> *P=&mesh->Points;
	List<typename Vector<3>> *N=&mesh->Normals;
	
	int map_index=0;
	if (mesh->Texture_GLMap[map_index]!=INVALID_TEXTURE) {
		glEnable(GL_TEXTURE_2D);
		if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,mesh->Texture_GLMap[map_index]);
		map_index++;
	} else return;


	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		if ((map_index<mesh->num_textures) && (i>=mesh->Texture_FaceInterval[map_index])) {
			if (mesh->Texture_GLMap[map_index]!=INVALID_TEXTURE) {
				glEnd();
				glBindTexture(GL_TEXTURE_2D,mesh->Texture_GLMap[map_index]);
				glBegin(GL_TRIANGLES);
				map_index++;
			} else {
				glEnd();
				SetAttrib(GL_TEXTURE_2D,sav_state_gltexture2D);
				return;
			}
		}
		f=&mesh->Faces[i];
		texf=&(*(mesh->Texture_Face))[i];
		
		texpoint=(*mesh->Texture_Point)[texf->Point[0]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[0]].GetList());
		Vertex((*P)[f->Point[0]]);

		texpoint=(*mesh->Texture_Point)[texf->Point[1]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[1]].GetList());
		Vertex((*P)[f->Point[1]]);

		texpoint=(*mesh->Texture_Point)[texf->Point[2]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		glNormal3dv((*N)[f->Point[2]].GetList());
		Vertex((*P)[f->Point[2]]);
	}
	glEnd();
	SetAttrib(GL_TEXTURE_2D,sav_state_gltexture2D);
}

template <> 
void RenderWindow<3>::DrawHyperMesh_Textured_Flat(HyperMesh<3> *mesh) {
	GLboolean sav_state_gltexture2D=glIsEnabled(GL_TEXTURE_2D);

	Vector<2> texpoint;
	HyperFace<3> *f,*texf;
	List<typename Vector<3>> *P=&mesh->Points;
	

	int map_index=0;
	if (mesh->Texture_GLMap[map_index]!=INVALID_TEXTURE) {
		glEnable(GL_TEXTURE_2D);
		if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,mesh->Texture_GLMap[map_index]);
		map_index++;
	} else return;

	glBegin(GL_TRIANGLES);
	for (int i=0;i<mesh->num_f;i++) {
		if ((map_index<mesh->num_textures) && (i>=mesh->Texture_FaceInterval[map_index])) {
			if (mesh->Texture_GLMap[map_index]!=INVALID_TEXTURE) {
				glEnd();
				glBindTexture(GL_TEXTURE_2D,mesh->Texture_GLMap[map_index]);
				glBegin(GL_TRIANGLES);
				map_index++;
			} else {
				glEnd();
				SetAttrib(GL_TEXTURE_2D,sav_state_gltexture2D);
				return;
			}
		}
		f=&mesh->Faces[i];
		texf=&(*(mesh->Texture_Face))[i];

		glNormal3dv(f->Normal(P).GetList());
		
		texpoint=(*mesh->Texture_Point)[texf->Point[0]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		Vertex((*P)[f->Point[0]]);

		texpoint=(*mesh->Texture_Point)[texf->Point[1]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		Vertex((*P)[f->Point[1]]);

		texpoint=(*mesh->Texture_Point)[texf->Point[2]];
		glTexCoord2d(texpoint[0],texpoint[1]);
		Vertex((*P)[f->Point[2]]);
	}
	glEnd();
	SetAttrib(GL_TEXTURE_2D,sav_state_gltexture2D);
}


template <int dim> 
void RenderWindow<dim>::SetMaterial(Vector<4> Diffuse_Color,float modulate_ambient,float modulate_specular,float shinness) {
	float ambient[4],diffuse[4],specular[4],emission[4];
	float alpha=(float)Diffuse_Color[3];

	glDisable(GL_COLOR_MATERIAL);
	Diffuse_Color=(1.0/255.0)*Diffuse_Color;
	

	diffuse[0]=(float)Diffuse_Color[0];
	diffuse[1]=(float)Diffuse_Color[1];
	diffuse[2]=(float)Diffuse_Color[2];
	diffuse[3]=alpha;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
	ambient[0]=min(1.0f,(float)(modulate_ambient*Diffuse_Color[0]));
	ambient[1]=min(1.0f,(float)(modulate_ambient*Diffuse_Color[1]));
	ambient[2]=min(1.0f,(float)(modulate_ambient*Diffuse_Color[2]));
	ambient[3]=alpha;
	glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
	specular[0]=min(1.0f,(float)(modulate_specular*Diffuse_Color[0]));
	specular[1]=min(1.0f,(float)(modulate_specular*Diffuse_Color[1]));
	specular[2]=min(1.0f,(float)(modulate_specular*Diffuse_Color[2]));
	specular[3]=alpha;
	glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
	glMaterialf(GL_FRONT,GL_SHININESS,shinness);
	emission[0]=0.0f;
	emission[1]=0.0f;
	emission[2]=0.0f;
	emission[3]=1.0f;
	glMaterialfv(GL_FRONT,GL_EMISSION,emission);
}

template <int dim> 
void RenderWindow<dim>::SetMaterial(Vector<4> Diffuse_Color,Vector<3> Ambient_Color,Vector<3> Specular_Color,float shinness) {
	float ambient[4],diffuse[4],specular[4],emission[4];
	float alpha=(float)Diffuse_Color[3];

	glDisable(GL_COLOR_MATERIAL);
	Diffuse_Color=(1.0/255.0)*Diffuse_Color;
	Ambient_Color=(1.0/255.0)*Ambient_Color;
	Specular_Color=(1.0/255.0)*Specular_Color;

	diffuse[0]=(float)Diffuse_Color[0];
	diffuse[1]=(float)Diffuse_Color[1];
	diffuse[2]=(float)Diffuse_Color[2];
	diffuse[3]=alpha;
	glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
	ambient[0]=(float)Ambient_Color[0];
	ambient[1]=(float)Ambient_Color[1];
	ambient[2]=(float)Ambient_Color[2];
	ambient[3]=alpha;
	glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
	specular[0]=(float)Specular_Color[0];
	specular[1]=(float)Specular_Color[1];
	specular[2]=(float)Specular_Color[2];
	specular[3]=alpha;
	glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
	glMaterialf(GL_FRONT,GL_SHININESS,shinness);
	emission[0]=0.0f;
	emission[1]=0.0f;
	emission[2]=0.0f;
	emission[3]=1.0f;
	glMaterialfv(GL_FRONT,GL_EMISSION,emission);
}

template <int dim> 
void RenderWindow<dim>::SetLightOff(int index) {
	int r_index=-1;
	switch(index) {
		case 0: r_index=GL_LIGHT0;break;
		case 1: r_index=GL_LIGHT1;break;
		case 2: r_index=GL_LIGHT2;break;
		case 3: r_index=GL_LIGHT3;break;
		case 4: r_index=GL_LIGHT4;break;
		case 5: r_index=GL_LIGHT5;break;
		default:r_index=GL_LIGHT5;
	};
	glDisable(r_index);
}

template <int dim> 
void RenderWindow<dim>::SetLight(int index,Vector<3> pos,bool relative,float diffuse,float ambient,Vector<3> *Color) {
	int r_index=-1;
	switch(index) {
		case 0: r_index=GL_LIGHT0;break;
		case 1: r_index=GL_LIGHT1;break;
		case 2: r_index=GL_LIGHT2;break;
		case 3: r_index=GL_LIGHT3;break;
		case 4: r_index=GL_LIGHT4;break;
		case 5: r_index=GL_LIGHT5;break;
		default:r_index=GL_LIGHT5;
	};

	GLfloat ambient_[4]  = {ambient,ambient,ambient,1.0f};
    GLfloat diffuse_[4]  = {diffuse,diffuse,diffuse,1.0f};
	GLfloat light_pos[4]= {(float)pos[0],(float)pos[1],(float)pos[2],1.0f};

	if (Color) {
		ambient_[0]=(GLfloat)(ambient_[0]*((*Color)[0]/255.0));
		ambient_[1]=(GLfloat)(ambient_[1]*((*Color)[1]/255.0));
		ambient_[2]=(GLfloat)(ambient_[2]*((*Color)[2]/255.0));
		diffuse_[0]=(GLfloat)(diffuse_[0]*((*Color)[0]/255.0));
		diffuse_[1]=(GLfloat)(diffuse_[1]*((*Color)[1]/255.0));
		diffuse_[2]=(GLfloat)(diffuse_[2]*((*Color)[2]/255.0));
	}

	glEnable(r_index);
	glLightfv(r_index,GL_AMBIENT,ambient_);
    glLightfv(r_index,GL_DIFFUSE,diffuse_);
	if (relative) {
		SetCameraCoords();
		glLightfv(r_index,GL_POSITION,light_pos);
		ClearRT();
	} else glLightfv(r_index,GL_POSITION,light_pos);
	glLightf(r_index,GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(r_index,GL_LINEAR_ATTENUATION, 0.0f);
    glLightf(r_index,GL_QUADRATIC_ATTENUATION, 0.0f);
}


template <int dim> 
void RenderWindow<dim>::SetAlphaMask(UINT index,bool invert_y=false) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,index);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR,GL_ZERO);
	glColorMask(0,0,0,1);

	float tex_y;
	if (!invert_y) tex_y=-1.0f;
	else tex_y=1.0f;

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();
		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColorMask(1,1,1,1);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::DrawTexture(UINT index,double transparency,bool invert_y) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,index);
	
	glEnable(GL_BLEND);
	glColor4d(1.0,1.0,1.0,transparency);

	float tex_y;
	if (!invert_y) tex_y=-1.0f;
	else tex_y=1.0f;

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();
		
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::Draw2DHyperBox(HyperBox<2> *Location,int color,double transparency) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	SetColor_(color,(float)transparency);
	
	glBegin(GL_QUADS);
		glVertex2d(Location->P0[0],Location->P0[1]);
		glVertex2d(Location->P1[0],Location->P0[1]);
		glVertex2d(Location->P1[0],Location->P1[1]);
		glVertex2d(Location->P0[0],Location->P1[1]);
		glVertex2d(Location->P0[0],Location->P0[1]);
	glEnd();
		
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::Draw3DHyperBox(Vector<dim> *C,Matrix *axes,Vector<dim> *sizes,int color,double transparency,int edge_color,int what_to_draw) {
	SetColor_(color,(float)transparency);

	SetRT(axes,C,sizes);

	if (what_to_draw&DRAW_FILLED) {
		glBegin(GL_QUADS);
			// Top Face
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
			// Bottom Face
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			// Front Face
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
			// Back Face
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
			// Right face
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			// Left Face
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
		glEnd();
	}

	if (what_to_draw&DRAW_EDGES) {
		SetColor_(edge_color,(float)transparency);

		glBegin(GL_LINE_STRIP);
			// Top Face
			glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glVertex3f(-0.5f,  0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
		glEnd();
		glBegin(GL_LINE_STRIP);
			// Bottom Face
			glVertex3f(-0.5f, -0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glVertex3f( 0.5f, -0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
		glEnd();
		glBegin(GL_LINE_STRIP);
			// Front Face
			glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
			glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
			glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
		glEnd();
		glBegin(GL_LINE_STRIP);
			// Back Face
			glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
			glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
			glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
		glEnd();
		glBegin(GL_LINE_STRIP);	
			// Right face
			glVertex3f( 0.5f, -0.5f, -0.5f);	// Bottom Right Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f, -0.5f);	// Top Right Of The Texture and Quad
			glVertex3f( 0.5f,  0.5f,  0.5f);	// Top Left Of The Texture and Quad
			glVertex3f( 0.5f, -0.5f,  0.5f);	// Bottom Left Of The Texture and Quad
		glEnd();
		glBegin(GL_LINE_STRIP);	
			// Left Face
			glVertex3f(-0.5f, -0.5f, -0.5f);	// Bottom Left Of The Texture and Quad
			glVertex3f(-0.5f, -0.5f,  0.5f);	// Bottom Right Of The Texture and Quad
			glVertex3f(-0.5f,  0.5f,  0.5f);	// Top Right Of The Texture and Quad
			glVertex3f(-0.5f,  0.5f, -0.5f);	// Top Left Of The Texture and Quad
		glEnd();
	}

	ClearRT();
}

/* USARE DRAW OBJ!!
template <int dim> 
void RenderWindow<dim>::DrawHyperBoxFill(Vector<dim> center,Vector<dim> d,int Color,double transparency) {
	glEnable(GL_BLEND);
	SetColor_(color,transparency);

	int free_dim=dim-2;
	// Choose free_dim dimension from num dimensions
	Positional_Number Choice(free_dim,num);
	Positional_Number X(free_dim,2);
	Positional_Number Face(2,2);
	Vector<dim> P;

	while (!Choice.RoundUp) {
		if (Choice.HasAllDifferentElements()) {
			// try all the possible for two dimensions
			X.SetZero();
			while (!X.RoundUp) {
				// Draw the face
				glBegin(GL_QUADS);
				Face.SetZero();
				while (!Face.RoundUp) {
					for(int j=0,o=0,l=0;j<dim;j++) {
						if (Choice.IsInside(j)) {
							P[j]=X[l++]*d[j];
						} else {
							P[j]=Face[o++]*d[j];
						}
					}
					Vertex(P+center);
					Face++;
				}
				glEnd();

				X++;
			}
		}
		Choice++;
	}
}
*/

template <int dim> 
void RenderWindow<dim>::Draw2DRectangle(HyperBox<2> *Location,int color,float size,double transparency) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glLineWidth(size);
	SetColor_(color,(float)transparency);

	glEnable(GL_BLEND);

	glBegin(GL_LINE_LOOP);
		glVertex2d(Location->P0[0],Location->P0[1]);
		glVertex2d(Location->P1[0],Location->P0[1]);
		glVertex2d(Location->P1[0],Location->P1[1]);
		glVertex2d(Location->P0[0],Location->P1[1]);
	glEnd();
		
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::DrawTexture(UINT index,HyperBox<2> *Location,double transparency,bool invert_y) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,index);
	
	glEnable(GL_BLEND);
	glColor4d(1.0,1.0,1.0,transparency);

	float tex_y;
	if (!invert_y) tex_y=-1.0f;
	else tex_y=1.0f;

	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2d(Location->P0[0],Location->P0[1]);
		glTexCoord2f(1.0f, 0.0f);glVertex2d(Location->P1[0],Location->P0[1]);
		glTexCoord2f(1.0f, tex_y);glVertex2d(Location->P1[0],Location->P1[1]);
		glTexCoord2f(0.0f, tex_y);glVertex2d(Location->P0[0],Location->P1[1]);
		glTexCoord2f(0.0f, 0.0f);glVertex2d(Location->P0[0],Location->P0[1]);
	glEnd();
		
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
void RenderWindow<dim>::DrawTexture_Blur(UINT index,bool invert_y,double radius) {
	SetImageCoords();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,index);
	

	float tex_y;
	if (!invert_y) tex_y=-1.0f;
	else tex_y=1.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ZERO);
	//glColor4d(1.0,1.0,1.0,1.0/9.0);
	glColor4d(1.0,1.0,1.0,1.0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();
		
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	//glColor4d(1.0,1.0,1.0,1.0/9.0);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4d(1.0,1.0,1.0,0.6);
	for(int turn=0;turn<2;turn++) {
		for(int i=0;i<8;i++) {
			float d_x=(float)((turn+1.0)*radius*cos((i*1.0/8.0)*2*M_PI));
			float d_y=(float)((turn+1.0)*radius*sin((i*1.0/8.0)*2*M_PI));
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0f+d_x, -1.0f+d_y);
				glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0f+d_x, -1.0f+d_y);
				glTexCoord2f(1.0f, tex_y);glVertex2f(1.0f+d_x, 1.0f+d_y);
				glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0f+d_x, 1.0f+d_y);
				glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0f+d_x, -1.0f+d_y);
			glEnd();
		}
	}

	glColor4d(1.0,1.0,1.0,0.5);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();


	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	ClearImageCoords();
}

template <int dim> 
UINT RenderWindow<dim>::ReadStencilPixel(int x,int y,int height) {
	int val;
	glReadPixels(x,(height-1)-y,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_INT,&val);
	return val;
}

template <int dim> 
void RenderWindow<dim>::GetBuffer(UINT texture_index,int internalFormat) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,texture_index);
	glCopyTexImage2D(GL_TEXTURE_2D,0,internalFormat,ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],0);
	glDisable(GL_TEXTURE_2D);
}


template <int dim> 
void RenderWindow<dim>::GetBuffer(Bitmap<ColorRGB> *I) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	glReadPixels(ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer());
}

template <int dim> 
void RenderWindow<dim>::GetBuffer(Bitmap<ColorRGBA> *I) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	glReadPixels(ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],GL_RGBA,GL_UNSIGNED_BYTE,I->getBuffer());
}

template <int dim> 
void RenderWindow<dim>::GetBuffer(Bitmap<ColorFloat> *I,int channel=0) {
	GLint ViewPort[4];
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	GLenum format;
	switch(channel) {
		case 0: format=GL_LUMINANCE;
				break;
		case 6: format=GL_LUMINANCE;
				break;
		case 1: format=GL_RED;
				break;
		case 2: format=GL_GREEN;
				break;
		case 3: format=GL_BLUE;
				break;
		case 4: format=GL_ALPHA;
				break;
		case 5: format=GL_DEPTH_COMPONENT;
				break;
		default:format=GL_LUMINANCE;
				break;
	};
	glReadPixels(ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],format,GL_FLOAT,I->getBuffer());
}

template <int dim> 
void RenderWindow<dim>::GetBuffer(Bitmap<GreyLevel> *I,int channel=0) {
	GLint ViewPort[4];
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	GLenum format;
	switch(channel) {
		case 0: format=GL_LUMINANCE;
				break;
		case 6: format=GL_LUMINANCE;
				break;
		case 1: format=GL_RED;
				break;
		case 2: format=GL_GREEN;
				break;
		case 3: format=GL_BLUE;
				break;
		case 4: format=GL_ALPHA;
				break;
		case 5: format=GL_DEPTH_COMPONENT;
				break;
		default:format=GL_LUMINANCE;
				break;
	};
	glReadPixels(ViewPort[0],ViewPort[1],ViewPort[2],ViewPort[3],format,GL_UNSIGNED_BYTE,I->getBuffer());
}

template <int dim> 
void RenderWindow<dim>::SetBuffer(Bitmap<ColorRGB> *I) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	glDrawPixels(ViewPort[2],ViewPort[3],GL_RGB,GL_UNSIGNED_BYTE,I->getBuffer());
}

template <int dim> 
void RenderWindow<dim>::SetBuffer(Bitmap<ColorFloat> *I) {
	GLint ViewPort[4];	
	glGetIntegerv(GL_VIEWPORT,ViewPort);

	if ((I->width!=ViewPort[2]) || (I->height!=ViewPort[3])) ErrorExit("Buffer size missmatch.");
	
	glDrawPixels(ViewPort[2],ViewPort[3],GL_LUMINANCE,GL_FLOAT,I->getBuffer());
}


template <int dim> 
void RenderWindow<dim>::DrawTexture(UINT index,bool invert_y,UINT mask_index,bool mask_invert_y) {
	float tex_y;

	SetImageCoords();
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
	
	glBindTexture(GL_TEXTURE_2D,mask_index);
	if (!mask_invert_y) tex_y=-1.0f;
	else tex_y=1.0f;
	glColor4d(1.0,1.0,1.0,1.0);
	glColorMask(0,0,0,1);
	glBlendFunc(GL_ONE,GL_ZERO);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D,index);
	if (!invert_y) tex_y=-1.0f;
	else tex_y=1.0f;
	glColor4d(1.0,1.0,1.0,1.0);
	glColorMask(1,1,1,1);
	glBlendFunc(GL_DST_ALPHA,GL_ONE_MINUS_DST_ALPHA);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
		glTexCoord2f(1.0f, 0.0f);glVertex2f(1.0, -1.0);
		glTexCoord2f(1.0f, tex_y);glVertex2f(1.0, 1.0);
		glTexCoord2f(0.0f, tex_y);glVertex2f(-1.0, 1.0);
		glTexCoord2f(0.0f, 0.0f);glVertex2f(-1.0, -1.0);
	glEnd();
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	ClearImageCoords();
}

#define CAMERA_FRAME_COLOR 0.176470588,0.615686275,1.0


template <> 
void RenderWindow<3>::DrawImagePlane(Matrix *Intrinsic,Matrix *Extrinsic,int width,int height,UINT index,double distance,double transparency,double front_color,double back_color,bool draw_frame) {
	Matrix Ext_(3,4);
	
	//Vector<3> Direction;
	//Ext_.Set(Extrinsic);
	//Invert3x4Matrix(&Ext_);	
	//Ext_.GetColumn(0,Direction.GetList());
	//Direction=-1.0*Direction;
	//Ext_.SetColumn(0,Direction.GetList());
	//Invert3x4Matrix(&Ext_);

	Ext_[0][0]=-(*Extrinsic)[0][0];
	Ext_[0][1]=-(*Extrinsic)[0][1];
	Ext_[0][2]=-(*Extrinsic)[0][2];
	Ext_[0][3]=-(*Extrinsic)[0][3];
	Ext_[1][0]=(*Extrinsic)[1][0];
	Ext_[1][1]=(*Extrinsic)[1][1];
	Ext_[1][2]=(*Extrinsic)[1][2];
	Ext_[1][3]=(*Extrinsic)[1][3];
	Ext_[2][0]=(*Extrinsic)[2][0];
	Ext_[2][1]=(*Extrinsic)[2][1];
	Ext_[2][2]=(*Extrinsic)[2][2];
	Ext_[2][3]=(*Extrinsic)[2][3];

	// Ipotesi: 
	//    A distanza focale dal centro ottico il piano immagine misura px*width
	//    La dismensione del piano immagine è direttamente proporzionale alla distanza d dal centro ottico
	// Tesi:
	//    size_image = (distanza / focale) * (px*width)
	//               = (distanza / Int[0][0]) * (width)
	//
	// Posizione immagine 
	//    A distanza focale l'immagine è traslata di (cx*px)
	//    La dimensione di un un pixel px è proporzionale alla distanza d dal centro ottico
	//
	//    size_pixel  = (distanza / focale)*px
	//    size_pixel  = size_image / width
	//    traslazione = cx*size_pixel
	//
	
	double size_px=(distance/(*Intrinsic)[0][0]);
	double size_py=(distance/(*Intrinsic)[1][1]);
	double size_image_x=size_px*width;
	double size_image_y=size_py*height;
	double trasl_x=size_px*((*Intrinsic)[0][2]-(width/2));
	double trasl_y=size_py*((*Intrinsic)[1][2]-(height/2));
	double minx=(-0.5*size_image_x)-trasl_x;
	double maxx=(0.5*size_image_x)-trasl_x;
	double miny=(-0.5*size_image_y)-trasl_y;
	double maxy=(0.5*size_image_y)-trasl_y;

	SetRTm(&Ext_);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		if (SET_PROGRAM_0) glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,index);
		
		if (back_color!=0.0) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glColor4d(back_color,back_color,back_color,transparency);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);glVertex3d(minx,miny,distance);
				glTexCoord2f(0.0f, 1.0f);glVertex3d(minx,maxy,distance);
				glTexCoord2f(1.0f, 1.0f);glVertex3d(maxx,maxy,distance);
				glTexCoord2f(1.0f, 0.0f);glVertex3d(maxx,miny,distance);
				glTexCoord2f(0.0f, 0.0f);glVertex3d(minx,miny,distance);
			glEnd();
			glCullFace(GL_BACK);
		}

		glColor4d(front_color,front_color,front_color,transparency);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);glVertex3d(minx,miny,distance);
			glTexCoord2f(0.0f, 1.0f);glVertex3d(minx,maxy,distance);
			glTexCoord2f(1.0f, 1.0f);glVertex3d(maxx,maxy,distance);
			glTexCoord2f(1.0f, 0.0f);glVertex3d(maxx,miny,distance);
			glTexCoord2f(0.0f, 0.0f);glVertex3d(minx,miny,distance);
		glEnd();

		if (draw_frame) {
			glDisable(GL_TEXTURE_2D);
			glColor4d(CAMERA_FRAME_COLOR,transparency);
			glLineWidth(2.5);
			glBegin(GL_LINE_LOOP);
			glVertex3d(minx,miny,distance);
			glVertex3d(minx,maxy,distance);
			glVertex3d(maxx,maxy,distance);
			glVertex3d(maxx,miny,distance);
			glEnd();
			glBegin(GL_LINES);
			glVertex3d(minx,miny,distance);
			glVertex3d(0.0,0.0,0.0);
			glVertex3d(minx,maxy,distance);
			glVertex3d(0.0,0.0,0.0);
			glVertex3d(maxx,maxy,distance);
			glVertex3d(0.0,0.0,0.0);
			glVertex3d(maxx,miny,distance);
			glVertex3d(0.0,0.0,0.0);
			glVertex3d(minx,miny,distance);
			glVertex3d(0.0,0.0,0.0);
			glEnd();
		}

	ClearRT();
}





//***********************************************************************************************************************

template <>
void RenderWindow<3>::vertex_color_normal(RenderWindow<3> *w,HyperMesh<3> *mesh,int vertex_index,void *vector_origine) {
	Vector<3> n=mesh->Normals[vertex_index];
	Vector<3> p=mesh->Points[vertex_index];
	Vector<3> *origin=(Vector<3> *)vector_origine;

	double x=n.Versore()*(((*origin)-p).Versore());
	// x:   1.0         ->   0.0
	//		0°			->   90°
	//      allineato        perpendicolare
	x=fabs(acos(x)/(M_PI/2));
	if (x>1.0) x=1.0;
	//      0           ->   1.0

	x=1.0-x;
	glColor3d(x,x,x);
}

















//***********************************************************************************************************************
//
//							Gestione primitive Avanzate di disegno 3D
//
//***********************************************************************************************************************

template <int dim> 
void RenderWindow<dim>::DrawArrow(Vector<dim> Origin,Vector<dim> Point,Vector<dim> DirArrow,int Color,float LineWidth) {

	Vector<dim> V=Point-Origin;
	if (V.isZero()) return;

	Vector<dim> V_versore=V.Versore();

	// se e' Zero, calcolo una direzione deterministica nn parallela a V
	if (DirArrow.isZero()) {
		for(int i=0;i<dim;i++) DirArrow[i]+=V[i]+1;
	}

	// DirArrow deve essere ortogonale a Origin-Point
	DirArrow=DirArrow-((DirArrow*V_versore)*V_versore);

	double len_arrow=(LEN_ARROW)*sqrt((V*V));  // 1/10 della lunghezza del vettore V
	
	DirArrow=len_arrow*(DirArrow.Versore());
	Vector<dim> d=len_arrow*V_versore;

	DrawLine(Origin,Point,Color,LineWidth);
	DrawLine(Point,Point+DirArrow-d,Color,LineWidth);
	DrawLine(Point,Point+(DirArrow*(-1))-d,Color,LineWidth);
}

template <int dim> 
void RenderWindow<dim>::DrawCamera(Matrix *OGL_Ext,int ID,double scale,bool draw_axis) {
	GLboolean LE;
	Matrix Ext_(3,4);
	Vector<3> Origin,Direction,DirArrow;

	glGetBooleanv(GL_LIGHTING,&LE);
	glDisable(GL_LIGHTING);

	Ext_.Set(OGL_Ext);
	Invert3x4Matrix(&Ext_);	
	Ext_.GetColumn(0,Direction.GetList());
	Direction=-1.0*Direction;
	Ext_.SetColumn(0,Direction.GetList());
	Invert3x4Matrix(&Ext_);
	
	scale=(LocalCoordsRadius/10.0)*scale;
	Ext_.GetColumn(3,Origin.GetList());
	Ext_.GetColumn(0,Direction.GetList());
	
	if (draw_axis) {
		DrawArrow(Origin,0.5*scale*Direction+Origin,DirArrow,0x0000FF);
		Ext_.GetColumn(2,Direction.GetList());
		DrawArrow(Origin,scale*Direction+Origin,DirArrow,0xFF0000);
		Ext_.GetColumn(1,Direction.GetList());
		DrawArrow(Origin,0.5*scale*Direction+Origin,DirArrow,0xFFFF00);
	}

	char Text[15];
	sprintf(Text,"Camera %02i",ID);
	Print(Origin-(scale/10)*Direction,0xFFA0A0A0,Text);

	if (LE==GL_TRUE) glEnable(GL_LIGHTING);
}

template <int dim> 
void RenderWindow<dim>::DrawCameraObject(Matrix *Ext,int ID,double scale,bool draw_axis) {
	GLboolean LE;
	Matrix Ext_(3,4);
	Vector<3> Origin,Direction,DirArrow;

	glGetBooleanv(GL_LIGHTING,&LE);
	glDisable(GL_LIGHTING);

	Ext_.Set(Ext);
	Invert3x4Matrix(&Ext_);	
	Ext_.GetColumn(0,Direction.GetList());
	Ext_.GetColumn(3,Origin.GetList());

	if (draw_axis) {
		DrawArrow(Origin,0.5*scale*Direction+Origin,DirArrow,0x0000FF);
		Ext_.GetColumn(2,Direction.GetList());
		DrawArrow(Origin,scale*Direction+Origin,DirArrow,0xFF0000);
		Ext_.GetColumn(1,Direction.GetList());
		DrawArrow(Origin,0.5*scale*Direction+Origin,DirArrow,0xFFFF00);
	}

	char Text[15];
	sprintf(Text,"Camera %02i",ID);
	Print(Origin-(scale/10)*Direction,0xFFA0A0A0,Text);

	if (LE==GL_TRUE) glEnable(GL_LIGHTING);
}

template <int dim> 
void RenderWindow<dim>::DrawRefSystem(Matrix *RT,double scale=1.0) {
	GLboolean LE;
	Vector<3> Origin,Direction,DirArrow;

	glGetBooleanv(GL_LIGHTING,&LE);
	glDisable(GL_LIGHTING);

	RT->GetColumn(3,Origin.GetList());
	RT->GetColumn(0,Direction.GetList());
	DrawArrow(Origin,scale*Direction+Origin,DirArrow,0x0000FF); // X Red
	RT->GetColumn(1,Direction.GetList());
	DrawArrow(Origin,scale*Direction+Origin,DirArrow,0x00FF00); // Y Green
	RT->GetColumn(2,Direction.GetList());
	DrawArrow(Origin,scale*Direction+Origin,DirArrow,0xFF0000); // Z Blue

	if (LE==GL_TRUE) glEnable(GL_LIGHTING);
}

template <int dim> 
void RenderWindow<dim>::DrawRefSystem(double scale) {
	GLboolean LE;
	Vector<3> Dir;

	glGetBooleanv(GL_LIGHTING,&LE);
	glDisable(GL_LIGHTING);

	Dir[0]=scale;
	DrawArrow(ZERO_3,Dir,ZERO_3,0x0000FF); // X Red
	Dir[0]=0.0;Dir[1]=scale;
	DrawArrow(ZERO_3,Dir,ZERO_3,0x00FF00); // Y Green
	Dir[1]=0.0;Dir[2]=scale;
	DrawArrow(ZERO_3,Dir,ZERO_3,0xFF0000); // Z Blue

	if (LE==GL_TRUE) glEnable(GL_LIGHTING);
}


/*template <int dim> 
void RenderWindow<dim>::DrawImage(Matrix *Ext,Matrix *Int,Bitmap<Color_RGB> *Img,bool borders) {
	Vector<3> Origin,Direction;
	Matrix Ext_(3,4);
	Ext_.Set(Ext);

	Invert3x4Matrix(&Ext_);	
	Ext_.GetColumn(0,Direction.GetList());
	Direction=-1.0*Direction;
	Ext_.SetColumn(0,Direction.GetList());
	Invert3x4Matrix(&Ext_);
	Ext_.GetColumn(3,Origin.GetList());
	Ext_.GetColumn(2,Direction.GetList());
	
	
	focal_len=1.0;
	DrawLine(Origin,Origin+focal_len*Direction,0x222200);
	Vector<3> Box[4];
	DrawLine(Box[1]


	HyperMesh<3> ImageFrame(4,2,4);
	ImageFrame.AddPoint(Box[0]);
	ImageFrame.AddPoint(Box[1]);
	ImageFrame.AddPoint(Box[2]);
	ImageFrame.AddPoint(Box[3]);
	
}
*/


template <int dim> 
int Vector2Binary(Vector<dim> v) {
	int val=0;
	for(int i=0;i<dim;i++) {
		if (v[i]>0) val+=1<<i;
	}
	return val;
}

template <int dim> 
Vector<dim> Binary2Vector(int val) {
	Vector<dim> s;
	for(int i=0;i<dim;i++) {
		if (Bit_Read(val,i)) s[i]=1;
	}
	return s;
}

template <int dim> 
void RenderWindow<dim>::DrawObj(int type,Vector<dim> *C,Matrix *SRef,Vector<dim> *Scale,int n_u,int n_v,double par2) {
	SetRT(SRef,C,Scale);
	
	switch(type) {
		case 0:glutSolidSphere(1.0,n_u,n_v);
			break;
		case 8:glutWireSphere(1.0,n_u,n_v);
			break;
		case 1:glutSolidCube(1.0);
			break;
		case 9:glutWireCube(1.0);
			break;
		case 2:glutSolidCone(1.0,par2,n_u,n_v);
			break;
		case 10:glutWireCone(1.0,par2,n_u,n_v);
			break;
		case 3:glutSolidTorus(par2,1.0,n_u,n_v);
			break;
		case 11:glutWireTorus(par2,1.0,n_u,n_v);
			break;
		case 4:glutSolidIcosahedron();
			break;
		case 12:glutWireIcosahedron();
			break;
		case 5:glutSolidDodecahedron();
			break;
		case 13:glutWireDodecahedron();
			break;
		case 6:glutSolidTeapot(1.0);
			break;
		case 14:glutWireTeapot(1.0);
			break;
	};
	ClearRT();
}




template <int dim> 
void RenderWindow<dim>::DrawHyperBox(Vector<dim> center,Vector<dim> d,int Color) {
	Vector<dim> Offset;
	for(int i=0;i<dim;i++) Offset[i]=0.5;
	
	Vector<dim> End;
	Vector<dim> I,F;

	for(int i=0;i<(1<<dim);i++) {
		Vector<dim> Start;
		Start=Binary2Vector<dim>(i);
		// Devo disegnare tutte le linee che partono da questo vertice
		// Esse, arrivano a tutti i vertici che hanno solo un bit != da Start
		for(int j=0;j<dim;j++) {
			End=Start;
			End[j]=1;
			// Se va a finire in qualcosa < di start -> lo ha gia' disegnato
			if (Vector2Binary<dim>(End)<=Vector2Binary<dim>(Start)) continue;

			I=Start-Offset;
			F=End-Offset;
			for(int q=0;q<dim;q++) I[q]*=(2*d[q]);
			for(int q=0;q<dim;q++) F[q]*=(2*d[q]);

			DrawLine(I+center,F+center,Color);
		}
	}

}


template <int dim> 
void RenderWindow<dim>::DrawHyperBox(HyperBox<dim> B,int Color,int Axis_Color) {
	Vector<dim> Offset;
	for(int i=0;i<dim;i++) Offset[i]=0.5;

	Vector<dim> center=B.Center();
	Vector<dim> d=B.d();

	Vector<dim> End;
	Vector<dim> I,F;

	for(int i=0;i<(1<<dim);i++) {
		Vector<dim> Start;
		Start=Binary2Vector<dim>(i);
		// Devo disegnare tutte le linee che partono da questo vertice
		// Esse, arrivano a tutti i vertici che hanno solo un bit != da Start
		for(int j=0;j<dim;j++) {
			End=Start;
			End[j]=1;
			// Se va a finire in qualcosa < di start -> lo ha gia' disegnato
			if (Vector2Binary<dim>(End)<=Vector2Binary<dim>(Start)) continue;
			
			I=Start-Offset;
			F=End-Offset;
			
			for(int q=0;q<dim;q++) {
				if (I[q]<0) I[q]=B.P1[q]-center[q];
				else I[q]=B.P0[q]-center[q];
			}
			for(int q=0;q<dim;q++) {
				if (F[q]<0) F[q]=B.P1[q]-center[q];
				else F[q]=B.P0[q]-center[q];
			}

			if (B.Rot==NULL) DrawLine(I+center,F+center,Color);
			else {
				Vector<3> a,b;
				Multiply(B.Rot,I,a.GetList());
				Multiply(B.Rot,F,b.GetList());
				DrawLine(a+center,b+center,Color);
			}
		}
	}

	if ((B.Rot!=NULL) && (Axis_Color!=0xFF000000)) {
		Vector<3> v;
		v[0]=(*B.Rot)[0][0];
		v[1]=(*B.Rot)[1][0];
		v[2]=(*B.Rot)[2][0];
		DrawLine(center,(d.Norm2())*v+center,Axis_Color);
	}
}

template <int dim> 
void RenderWindow<dim>::Print(Vector<dim> P,const char *fmt, ...) {
	char text[256];
	va_list ap;

	if (fmt == NULL) return;

	va_start(ap,fmt);									// Parses The String For Variables
	vsprintf(text,fmt,ap);								// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	_print(text,P);
}

template <int dim> 
void RenderWindow<dim>::Print(Vector<dim> P,int color,const char *fmt, ...) {
	char text[256];
	va_list ap;

	if (fmt == NULL) return;

	va_start(ap,fmt);									// Parses The String For Variables
	vsprintf(text,fmt,ap);								// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glColor4ubv((GLubyte*)(&color));
	_print(text,P);
}

template <int dim> 
void RenderWindow<dim>::Print_ra_ic(Vector<dim> P,int width,int height,int color,const char *fmt, ...) {
	char text[256];
	va_list ap;

	if (fmt == NULL) return;

	va_start(ap,fmt);									// Parses The String For Variables
	vsprintf(text,fmt,ap);								// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glColor4ubv((GLubyte*)(&color));
	double lenght=GetPrintedStringSize(text,width,height);
	P[0]-=lenght;
	_print(text,P);
}

template <int dim> 
void RenderWindow<dim>::SetCurrentFont(char *FontName,int FontSize) {
	if (fontbase!=NULL) glDeleteLists(fontbase,96);

	
	fontbase=glGenLists(96);
	HFONT font=CreateFont(-FontSize,0,0,0,0,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_DONTCARE|DEFAULT_PITCH,FontName);
	HFONT oldfont=(HFONT)SelectObject(hDC,font);
	
	// Compute char size
	MAT2 tmp;GLYPHMETRICS tmp_size;
	tmp.eM11.value=1;tmp.eM11.fract=0;
	tmp.eM21.value=0;tmp.eM21.fract=0;
	tmp.eM12.value=0;tmp.eM12.fract=0;
	tmp.eM22.value=1;tmp.eM22.fract=0;
	for(int i=32;i<32+USED_CHARACTERS_NUMBER;i++) {
		GetGlyphOutline(hDC,i,GGO_METRICS,&tmp_size,0,NULL,&tmp);
		font_size[i-32].delta_x=tmp_size.gmptGlyphOrigin.x;
		font_size[i-32].size_x=tmp_size.gmBlackBoxX;
		font_size[i-32].size_y=tmp_size.gmBlackBoxY;
		font_size[i-32].step_x=tmp_size.gmCellIncX;
	}

	wglUseFontBitmaps(hDC,32,96,fontbase);
	SelectObject(hDC,oldfont);
	DeleteObject(font);
}

template <int dim> 
void RenderWindow<dim>::GetPrintedStringSize(char *String,int width,int height,double &txt_w,double &txt_h) {
	size_t len=strlen(String);
	

	txt_w=0.0;
	txt_h=0.0;

	if (len==0) return;
	
	
	if (len==1) {
		int x=String[0];
		if (x>32) {
			txt_w=font_size[x-32].delta_x-1+font_size[x-32].size_x+1;
			txt_h=font_size[x-32].size_y;
		}

	} else {
		int x=String[0];
		if (x>32) {
			txt_w=font_size[x-32].delta_x+font_size[x-32].step_x;
			txt_h=font_size[x-32].size_y;
		}

		for(size_t i=1;i<len-1;i++) {
			x=String[i];
			if (x>32) {
				txt_w+=font_size[x-32].step_x;
				txt_h=max(txt_h,font_size[x-32].size_y);
			}
		}
		x=String[len-1];
		if (x>32) {
			txt_w+=font_size[x-32].size_x;
			txt_h=max(txt_h,font_size[x-32].size_y);
		}

	}
	txt_w=2.0*(txt_w/width);
	txt_h=2.0*(txt_h/height);
}

template <int dim> 
double RenderWindow<dim>::GetPrintedStringSize(char *String,int width,int height) {
	double txt_w,txt_h;
	GetPrintedStringSize(String,width,height,txt_w,txt_h);
	return txt_w;
}

template <> 
void RenderWindow<2>::_print(const char *text,Vector<2> P) {
	glRasterPos2d(P[0],P[1]);
	
	glPushAttrib(GL_LIST_BIT);
		glListBase(fontbase - 32);
		glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

template <> 
void RenderWindow<3>::_print(const char *text,Vector<3> P) {
	glRasterPos3d(P[0],P[1],P[2]);

	glPushAttrib(GL_LIST_BIT);
		glListBase(fontbase - 32);
		glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}








// Clickable Window Objects functions (it uses the stencil buffer)
template <int dim> 
void RenderWindow<dim>::StartDefiningClickableObjects(bool clear_stencil) {
	if (clear_stencil) glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFFFFFFFF);
	glStencilFunc(GL_ALWAYS,0x00,0xFFFFFFFF);
	glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
}
template <int dim> 
UINT RenderWindow<dim>::getClickableObjectsMaxIndexValue() {
	UINT max_index=((UINT)2)<<StencilBits;
	return max_index;
}
template <int dim> 
void RenderWindow<dim>::DefineClickableObject(UINT index) {
	glStencilFunc(GL_ALWAYS,index,0xFFFFFFFF);
}
template <int dim> 
void RenderWindow<dim>::StopDefiningClickableObjects() {
	glDisable(GL_STENCIL_TEST);
}
template <int dim> 
UINT RenderWindow<dim>::GetClickableObjectIndex(int x,int y,int width,int height) {
	if ((x<0) || (y<0) || (x>=width) || (y>=height)) return 0;
	Activate();
	UINT val=ReadStencilPixel(x,y,height);
	DeActivate();
	return val;
}



