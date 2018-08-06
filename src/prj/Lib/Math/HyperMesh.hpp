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





#pragma warning(3:4244)

//***************************************************************************************
//***************************************************************************************
//***************************************************************************************
//***************************************************************************************
//***************************************************************************************
//***************************************************************************************

template <int dim> 
HyperMesh<dim>::HyperMesh() : Points(INITIAL_POINT_NUMBER),
							  Normals(INITIAL_POINT_NUMBER),
							  Laplacians(INITIAL_POINT_NUMBER),
							  Biharmonics(INITIAL_POINT_NUMBER),
							  CurvatureNormals(INITIAL_POINT_NUMBER),
							  Faces(INITIAL_FACE_NUMBER),
							  Neighbours(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),
							  NumNeighbours(INITIAL_POINT_NUMBER),
							  Edge_Faces(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),
							  NeighbourFaces(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),
							  NumNeighbourFaces(INITIAL_POINT_NUMBER),
							  AreaPoint(INITIAL_POINT_NUMBER) {
	// Struttura 3D Init
	num_p=0;
	num_f=0;
	update_neighbours_structure=true;

	// Textures
	num_textures=0;
	for(int i=0;i<MAX_TEXTURES;i++) {
		Texture_FileName[i]=NULL;
		Texture_FaceInterval[i]=0;
		Texture_GLMap[i]=INVALID_TEXTURE;
		Texture_Map[i]=NULL;
	}
	Texture_Point=NULL;
	Texture_Face=NULL;
	TextureUVW=NULL;
	
	ResetVolatileProperties();
}

template <int dim> 
HyperMesh<dim>::HyperMesh(int initial_point_number,int initial_face_number,int initial_neighbours_number) : 
							  Points(initial_point_number),
							  Normals(initial_point_number),
							  Laplacians(initial_point_number),
							  Biharmonics(initial_point_number),
							  CurvatureNormals(initial_point_number),
							  Faces(initial_face_number),
							  Neighbours(initial_point_number,initial_neighbours_number),
							  NumNeighbours(initial_point_number),
							  Edge_Faces(initial_point_number,initial_neighbours_number),
							  NeighbourFaces(initial_point_number,initial_neighbours_number),
							  NumNeighbourFaces(initial_point_number),
							  AreaPoint(initial_point_number) {
	// Struttura 3D Init
	num_p=0;
	num_f=0;
	update_neighbours_structure=true;

	// Textures
	num_textures=0;
	for(int i=0;i<MAX_TEXTURES;i++) {
		Texture_FileName[i]=NULL;
		Texture_FaceInterval[i]=0;
		Texture_GLMap[i]=INVALID_TEXTURE;
		Texture_Map[i]=NULL;
	}
	Texture_Point=NULL;
	Texture_Face=NULL;
	TextureUVW=NULL;

	ResetVolatileProperties();
}

template <int dim> 
HyperMesh<dim>::HyperMesh(HyperMesh<dim> &original) : Points(INITIAL_POINT_NUMBER),Normals(INITIAL_POINT_NUMBER),Laplacians(INITIAL_POINT_NUMBER),Biharmonics(INITIAL_POINT_NUMBER),CurvatureNormals(INITIAL_POINT_NUMBER),Faces(INITIAL_FACE_NUMBER),Neighbours(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),NumNeighbours(INITIAL_POINT_NUMBER),Edge_Faces(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),NeighbourFaces(INITIAL_POINT_NUMBER,INITIAL_NUM_NEIGHBOURS),NumNeighbourFaces(INITIAL_POINT_NUMBER),AreaPoint(INITIAL_POINT_NUMBER) {
	ErrorExit("Operatore di copia tra HyperMesh implementato solo nella forma a puntatore");
}

#define POINT_NUMBER_COPY (original->update_neighbours_structure?original->num_p:INITIAL_POINT_NUMBER)

template <int dim> 
HyperMesh<dim>::HyperMesh(HyperMesh<dim> *original) : 
							  Points(original->num_p),
							  Normals(original->num_p),
							  Laplacians(original->num_p),
							  Biharmonics(original->num_p),
							  CurvatureNormals(original->num_p),
							  Faces(original->num_f),
							  Neighbours(POINT_NUMBER_COPY,INITIAL_NUM_NEIGHBOURS),
							  NumNeighbours(POINT_NUMBER_COPY),
							  Edge_Faces(POINT_NUMBER_COPY,INITIAL_NUM_NEIGHBOURS),
							  NeighbourFaces(POINT_NUMBER_COPY,INITIAL_NUM_NEIGHBOURS),
							  NumNeighbourFaces(POINT_NUMBER_COPY),
							  AreaPoint(original->num_p) {
							  
	// Standard Init
	// Struttura 3D Init
	num_p=0;
	num_f=0;
	update_neighbours_structure=original->update_neighbours_structure;

	// Textures
	num_textures=0;
	for(int i=0;i<MAX_TEXTURES;i++) {
		Texture_FileName[i]=NULL;
		Texture_FaceInterval[i]=0;
		Texture_GLMap[i]=INVALID_TEXTURE;
		Texture_Map[i]=NULL;
	}
	Texture_Point=NULL;
	Texture_Face=NULL;
	TextureUVW=NULL;
	
	ResetVolatileProperties();
	

	// Copia original
	// Struttura 3D
	for(int i=0;i<original->num_p;i++) AddPoint(original->Points[i]);
	for(int i=0;i<original->num_f;i++) AddFace(original->Faces[i]);

	
	// Tessiture
	for(int i=0;i<original->num_textures;i++) {
		Texture_FileName[i]=strcln(original->Texture_FileName[i]);
		Texture_FaceInterval[i]=original->Texture_FaceInterval[i];
		Texture_GLMap[i]=original->Texture_GLMap[i];
		Texture_Map[i]=original->Texture_Map[i];
	}
	num_textures=original->num_textures;

	if (original->Texture_Point) {
		Texture_Point=new Array<typename Vector<2>>(original->Texture_Point->numElements());
		for(int i=0;i<original->Texture_Point->numElements();i++) 
			Texture_Point->append((*(original->Texture_Point))[i]);
	}
	if (original->Texture_Face) {
		Texture_Face=new Array<FaceType>(original->Texture_Face->numElements());
		for(int i=0;i<original->Texture_Face->numElements();i++) 
			Texture_Face->append((*(original->Texture_Face))[i]);
	}

	if (original->TextureUVW) {
		TextureUVW=new Array<typename Vector<3>>(original->TextureUVW->numElements());
		for(int i=0;i<original->TextureUVW->numElements();i++) 
			TextureUVW->append((*(original->TextureUVW))[i]);
	}

}

template <int dim> 
void HyperMesh<dim>::ResetVolatileProperties() {
	Total_Area=INVALID_MESH_PROPERTY_VALUE;
	Average_Area=INVALID_MESH_PROPERTY_VALUE;
	Average_Stress=INVALID_MESH_PROPERTY_VALUE;
	Min_Stress=INVALID_MESH_PROPERTY_VALUE;
	Average_Curvature=INVALID_MESH_PROPERTY_VALUE;
	Average_Curvature_Variation=INVALID_MESH_PROPERTY_VALUE;
	Average_Mean_Curvature=INVALID_MESH_PROPERTY_VALUE;
	Max_Curvature=INVALID_MESH_PROPERTY_VALUE;
	Max_Curvature_Variation=INVALID_MESH_PROPERTY_VALUE;
	Max_Mean_Curvature=INVALID_MESH_PROPERTY_VALUE;
}

template <int dim> 
void HyperMesh<dim>::SetUpdateNeighboursStructure(bool update_neighbours_structure) {
	this->update_neighbours_structure=update_neighbours_structure;
}

template <int dim> 
HyperMesh<dim>::~HyperMesh() {
	for(int i=0;i<MAX_TEXTURES;i++) SDELETEA(Texture_FileName[i]);
	SDELETE(Texture_Face);
	SDELETE(Texture_Point);
	SDELETE(TextureUVW);
}


// Aggiunge un punto con nuormale nulla e nessun vicino
template <int dim> 
int HyperMesh<dim>::AddPoint(Vector<dim> P) {
	Points[num_p]=P;
	if (update_neighbours_structure) {
		NumNeighbours[num_p]=0;
		NumNeighbourFaces[num_p]=0;
	}
	return num_p++;
}

template <int dim> 
void HyperMesh<dim>::AddNeighbour(int point,int Neighbour,int face) {
	int free,i_tmp;

	// Get free point: ovvero quel vertice che nn appartiene all'egde -> sempre -1 per dim=2
	i_tmp=Faces[face].free_point3(point,Neighbour);
	if (i_tmp==-1) free=-1;
	else free=Faces[face].Point[i_tmp];


	// Controlla se il vicino e' gia' stato inserito
	for(int i=0;i<NumNeighbours[point];i++) {
		if (Neighbours[point][i]==Neighbour) {
			if (Edge_Faces[point][i].free2!=-1) {
				//MessageBox(NULL,"double edged","warning",0);
			}
			Edge_Faces[point][i].free2=free;
			return;
		}
	}
	
	// Se e' un nuovo vicino
	Neighbours[point][NumNeighbours[point]]=Neighbour;
	Edge_Faces[point][NumNeighbours[point]].free1=free;
	Edge_Faces[point][NumNeighbours[point]].free2=-1;

	NumNeighbours[point]++;
}


// Aggiunge una faccia e aggiorna i vicini di un punto
template <int dim> 
int HyperMesh<dim>::AddFace(HyperFace<dim> F) {
	Faces[num_f]=F;
	
	if (update_neighbours_structure) {
		// aggiunge i punti vicini e gli edge vicini
		for(int i=0;i<dim;i++) {
			for(int j=0;j<dim;j++) {
				// aggiunge ai vicini di F.Point[i] i punti F.Point[j] (j!=i)
				if (i!=j) AddNeighbour(F.Point[i],F.Point[j],num_f);
			}
		}
	
		// aggiunge le facce vicine
		for(int i=0;i<dim;i++) {
			NeighbourFaces[F.Point[i]][NumNeighbourFaces[F.Point[i]]]=num_f;
			NumNeighbourFaces[F.Point[i]]++;
		}
	}

	return num_f++;
}

template <int dim> 
int HyperMesh<dim>::Nearest_from_axis(Vector<dim> O,Vector<dim> D) {
	D=D.Versore();

	double d_min=Element_Type_Max;
	int i_min=0;
	for(int i=0;i<num_p;i++) {
		double a=(Points[i]-O)*D;

		double d=(Points[i]-O-(a*D)).Norm2();
		

		if (d<d_min) {
			d_min=d;
			i_min=i;
		}
	}
	return i_min;
}

template <int dim> 
void HyperMesh<dim>::Scale(double factor) {
	Vector<dim> Baricentro;

	for(int i=0;i<num_p;i++) Baricentro+=Points[i];
	Baricentro*=(1.0/num_p);
	
	factor=1/factor;
	for(int i=0;i<num_p;i++) Points[i]=((Points[i]-Baricentro)*factor)+Baricentro;
}

void HyperMesh<2>::Rotate(Vector<2> angle_axis,Vector<2> center) {
	ErrorExit("Rotate: Not implemented.");
}

void HyperMesh<3>::Rotate(Vector<3> angle_axis,Vector<3> center) {
	Matrix R(3,3);
	Vector<dim> tmp;

	double angle=angle_axis.Norm2();
	if (IS_ZERO(angle,DBL_EPSILON)) return;
	
	AngleAxis2RotationMatrix(&R,angle_axis);

	for(int i=0;i<num_p;i++) {
		Multiply(&R,(Points[i]-center),tmp.GetList());
		Points[i]=tmp+center;
	}
}


template <int dim> 
double PointDistanceSemplified(Vector<dim> P) {

	int nearest=0;
	double dist=(Points[0]-P).Norm2();
	
	// Trovo il vertice piu' vicino
	for(int i=0;i<num_p;i++) {
		double dist_tmp=(Points[i]-P).Norm2();
		if (dist_tmp<dist) {
			dist_tmp=dist;
			nearest=i;
		}
	}

	return (P-Points[nearest])*Normals[nearest];

}

/*
template <> 
double HyperMesh<3>::PointFaceDistance(Vector<3> P,int f) {

	for(int i=0;i<num_f;i++) {
		Vector<3> N=Faces[i].Normal(&Points);


		Vector<3> a=Faces[i].Point[1]-Faces[i].Point[0];			// p1 rispetto a p0
		Vector<3> b=Faces[i].Point[2]-Faces[i].Point[0];			// p2 rispetto a p0
		Vector<3> P_O=P-Faces[i].Point[0];							// P rispetto a p0
			
		
		double dist=(P_O*N);	// Positivo o negativo

		Vector<3> PP_O=(P_O*a)*a+(P_O*b)*b;							// P proiettato nel piano rispetto a p0

		/*
				0				a
									
									PP_0


				b
		*/
/*
		double k1=(a^PP_0)*N;
		double k2=((b-a)^(PP_0-a))*N;
		double k3=((-1*b)^(PP_0-b))*N;


		Vector<3> N=Faces[i].Normal(&Points);
		Vector<3> P2P1=Faces[i].Point[1]-Faces[i].Point[0];
		Vector<3> P3P1=Faces[i].Point[2]-Faces[i].Point[0];
		Vector<3> P3P2=Faces[i].Point[2]-Faces[i].Point[1];

		Vector<3> P_O=P-Faces[i].Point[0];
		Vector<3> P0P1=(P_O*P2P1)*P2P1+(P_O*P3P1)*P3P1;

		Vector<3> V1=P2P1.Versore()+P3P1.Versore();
		Vector<3> V2=P3P2.Versore()-P2P1.Versore();
		Vector<3> V3=-P3P1.Versore()-P3P2.Versore();

		double f1=(V1^(-P0P1))*N;
		double f2=(V2^(-P0P1))*N;
		double f3=(V3^(-P0P1))*N;

		





	}
}
*/

template <int dim> 
void HyperMesh<dim>::ComputaNormals() {
	
	for(int i=0;i<num_p;i++) {
		for(int j=0;j<dim;j++) Normals[i][j]=0;
	}

	for(int i=0;i<num_f;i++) {
		Vector<dim> N=Faces[i].Normal(&Points);
		for(int j=0;j<dim;j++) {
			Normals[Faces[i].Point[j]]+=N;
		}
	}
	for(int i=0;i<num_p;i++) {
		Normals[i]=Normals[i].Versore();
	}

}

//#define SCALE_INDIPENDENT

#ifdef SCALE_INDIPENDENT

template <int dim> 
Vector<dim> HyperMesh<dim>::Laplacian(int i) {

	Vector<dim> tmp;
	Vector<dim> r;
	double E=0;
	double e;

	for(int j=0;j<NumNeighbours[i];j++) {
		tmp=(Points[Neighbours[i][j]]-Points[i]);
		e=tmp.Norm2();
		if (e!=0) r+=tmp*(1/e);
		E+=e;
	}

	if (E==0) return 0*r;

	r*=(1/E);
	
	return r;
}

template <int dim> 
Vector<dim> HyperMesh<dim>::Biharmonic(int i) {

	Vector<dim> tmp;
	Vector<dim> r;
	double E=0;
	double e;

	for(int j=0;j<NumNeighbours[i];j++) {
		tmp=(Points[Neighbours[i][j]]-Points[i]);
		e=tmp.Norm2();
		if (e!=0) r+=(Laplacians[Neighbours[i][j]]-Laplacians[i]).Versore()*(1/e);
		E+=e;
	}

	if (E==0) return 0*r;

	r*=(1/E);

	return r;
}

#else


template <int dim> 
Vector<dim> HyperMesh<dim>::Laplacian(int i) {

	Vector<dim> r;
	
	for(int j=0;j<NumNeighbours[i];j++) r+=Points[Neighbours[i][j]];

	r*=1.0/NumNeighbours[i];
	r-=Points[i];
	
	return r;
}

template <int dim> 
Vector<dim> HyperMesh<dim>::Biharmonic(int i) {

	Vector<dim> r;
	double cost=0;

	for(int j=0;j<NumNeighbours[i];j++) {
		int p_j=Neighbours[i][j];
		r+=Laplacians[p_j];
		cost+=(1.0/NumNeighbours[p_j]);
	}

	r*=1.0/NumNeighbours[i];
	r-=Laplacians[i];

	cost*=1.0/NumNeighbours[i];
	r*=1.0/(1+cost);

	return r;
}

#endif


double cotg(Vector<2> A,Vector<2> B) {
	return 0;
}

double cotg(Vector<3> A,Vector<3> B) {

	double dot=A*B;
	double den=sqrt((A*A)*(B*B)-(dot*dot));
	
	if (den==0) {
		return 0;
	}
	return dot/den;
}

template <int dim> 
Vector<dim> HyperMesh<dim>::CurvatureNormal(int i) {
	Vector<dim> r;
	
	for(int j=0;j<NumNeighbours[i];j++) {
		// Points[i]							1
		// Points[Neighbours[i][j]]				2  edge
		// Points[Edge_Faces[i][j].free1]
		// Points[Edge_Faces[i][j].free2]

		if (Edge_Faces[i][j].free2==-1) continue;	// It's a Border face -> sometimes it is a hole

		Vector<dim> xi=Points[i];
		Vector<dim> xj=Points[Neighbours[i][j]];
		Vector<dim> xj_1=Points[Edge_Faces[i][j].free1];
		Vector<dim> xj_2=Points[Edge_Faces[i][j].free2];
		
		Vector<dim> A1=xi-xj_1;
		Vector<dim> A2=xi-xj_2;
		Vector<dim> B1=xj-xj_1;
		Vector<dim> B2=xj-xj_2;

		r+=((cotg(A1,B1)+cotg(A2,B2))*(xj-xi));
	}
	
	double A=0;
	for(int j=0;j<NumNeighbourFaces[i];j++) A+=Faces[NeighbourFaces[i][j]].Area;

	if (A==0) return (0*r);
	return (r*(1.0/(4*A)));
}



template <int dim> 
void HyperMesh<dim>::ComputaCurvatureNormal() {

	for(int i=0;i<num_p;i++) CurvatureNormals[i]=CurvatureNormal(i);
}

template <int dim> 
void HyperMesh<dim>::ComputaLaplacians() {

	for(int i=0;i<num_p;i++) Laplacians[i]=Laplacian(i);

}


template <int dim> 
void HyperMesh<dim>::ComputaBiharmonics() {

	for(int i=0;i<num_p;i++) Biharmonics[i]=Biharmonic(i);

}

template <int dim> 
double HyperMesh<dim>::AverageStress() {

	double average=0;
	int num=0;
	for(int i=0;i<num_f;i++) {
		for(int j=0;j<dim;j++) {
			for(int k=j+1;k<dim;k++) {
				average+=(Points[Faces[i].Point[j]]-Points[Faces[i].Point[k]]).Norm2();
				num++;
			}			
		}
	}
	
	Average_Stress=average/num;

	return Average_Stress;
}

template <int dim> 
double HyperMesh<dim>::MinStress() {

	if (num_f==0) return (Min_Stress=0);
	Min_Stress=(Points[Faces[0].Point[0]]-Points[Faces[0].Point[1]]).Norm2();
	
	for(int i=0;i<num_f;i++) {
		for(int j=0;j<dim;j++) {
			for(int k=j+1;k<dim;k++) {
				double tmp=(Points[Faces[i].Point[j]]-Points[Faces[i].Point[k]]).Norm2();
				if (tmp<Min_Stress) Min_Stress=tmp;
			}			
		}
	}
		
	return Min_Stress;
}

template <int dim> 
double HyperMesh<dim>::AverageCurvature() {

	double average=0;
	for(int i=0;i<num_p;i++) average+=Laplacians[i].Norm2();

	Average_Curvature=average/num_p;
	return Average_Curvature;
}

template <int dim> 
double HyperMesh<dim>::AverageCurvatureVariation() {

	double average=0;
	for(int i=0;i<num_p;i++) average+=Biharmonics[i].Norm2();

	Average_Curvature_Variation=average/num_p;

	return Average_Curvature_Variation;
}

template <int dim> 
double HyperMesh<dim>::AverageMeanCurvature() {

	double average=0;
	for(int i=0;i<num_p;i++) average+=CurvatureNormals[i].Norm2();

	Average_Mean_Curvature=average/num_p;

	return Average_Mean_Curvature;
}

template <int dim> 
double HyperMesh<dim>::MaxCurvature() {

	if (num_p==0) return (Max_Curvature=0);
	double max=Laplacians[0].Norm2();
	for(int i=1;i<num_p;i++) {
		double r=Laplacians[i].Norm2();
		if (r>max) max=r;
	}
		
	Max_Curvature=max;
	return max;
}

template <int dim> 
double HyperMesh<dim>::MaxCurvatureVariation() {

	if (num_p==0) return (Max_Curvature_Variation=0);
	double max=Biharmonics[0].Norm2();
	for(int i=1;i<num_p;i++) {
		double r=Biharmonics[i].Norm2();
		if (r>max) max=r;
	}
	Max_Curvature_Variation=max;
		
	return max;
}


template <int dim> 
double HyperMesh<dim>::MaxMeanCurvature() {

	if (num_p==0) return (Max_Mean_Curvature=0);
	double max=CurvatureNormals[0].Norm2();
	for(int i=1;i<num_p;i++) {
		double r=CurvatureNormals[i].Norm2();
		if (r>max) max=r;
	}
		
	Max_Mean_Curvature=max;
	return max;
}









// Necessita del calcolo delle normali...
template <int dim> 
Array<int> *HyperMesh<dim>::Get_Apparent_Contour(Vector<dim> Point_of_view) {

	double f;
	Array<int> *contour=new Array<int>((int)(0.05*num_p));   // il 5% dei punti attuali

	for(int i=0;i<num_p;i++) {
		f=((Points[i]-Point_of_view).Versore())*Normals[i];
		if ((f<0.05) && (f>-0.05)) contour->append(i);
	}

	return contour;
}

template <int dim> 
Vector<dim> HyperMesh<dim>::GetCenter() {
	Vector<dim> c;

	for(int i=0;i<num_p;i++) c+=Points[i];
	c=c*(1.0/num_p);
	
	return c;
}

template <int dim> 
double HyperMesh<dim>::GetRadius(Vector<dim> center) {
	double r=0;

	for(int i=0;i<num_p;i++) {
		double v=(Points[i]-center).Norm2();
		if (v>r) r=v;
	}
	
	return r;
}


template <int dim> 
Vector<dim> HyperMesh<dim>::GetPrincipalAxes(Matrix *x,bool use_baricenter=true) {
	// Get center of mass
	Vector<dim> Center;
	if (use_baricenter) {
		for(int k=0;k<num_p;k++) Center+=Points[k];
		Center=(1.0/num_p)*Center;
	} else {
		HyperBox<dim> g=BoundingBox();
		Center=g.Center();
	}

	for(int i=0;i<3;i++) {
		for(int j=0;j<=i;j++) {
			(*x)[i][j]=0.0;
			for(int k=0;k<num_p;k++) {
				Vector<dim> p=Points[k]-Center;
				if (i==j) (*x)[i][j]+=p.Norm2()-p[i]*p[j];
				else (*x)[i][j]+=-p[i]*p[j];
			}
		}
	}
	(*x)[0][1]=(*x)[1][0];
	(*x)[0][2]=(*x)[2][0];
	(*x)[1][2]=(*x)[2][1];
	
	Matrix U(3,3),D(3,3),V(3,3);
	SVD(x,&U,&D,&V);
	*x=V;
	return Center;
}

template <int dim> 
HyperBox<dim> HyperMesh<dim>::BoundingBox() {
	
	HyperBox<dim> box;
	
	box.P0.SetVal(Element_Type_Max);
	box.P1.SetVal(-Element_Type_Max);

	for(int i=0;i<num_p;i++) {
		for(int j=0;j<dim;j++) {
			if (Points[i][j]<box.P0[j]) box.P0[j]=Points[i][j];
			if (Points[i][j]>box.P1[j]) box.P1[j]=Points[i][j];
		}
	}
	
	return box;
}

template <int dim> 
HyperBox<dim> HyperMesh<dim>::BoundingBox(Array<int> *face_index_list) {
	
	HyperBox<dim> box;
	
	box.P0.SetVal(Element_Type_Max);
	box.P1.SetVal(-Element_Type_Max);

	int *index=face_index_list->getMem();
	int num_ele=face_index_list->numElements();
	HyperFace<dim> *face_list=Faces.getMem();
	
	for(int i=0;i<num_ele;i++,index++) {
		int *p_list=face_list[(*index)].Point;
		for(int j=0;j<dim;j++,p_list++) {
			box.P0.Min(&(Points[(*p_list)]));
			box.P1.Max(&(Points[(*p_list)]));
		}
	}
	
	return box;
}

template <int dim> 
HyperBox<dim>::HyperBox(const HyperBox<dim> &x) {
	P0=x.P0;
	P1=x.P1;
	C2=x.C2;
	
	if (x.Rot!=NULL) {
		Rot=new Matrix(3,3);
		*Rot=*(x.Rot);
	} else Rot=NULL;
}

template <int dim> 
const HyperBox<dim> &HyperBox<dim>::operator =(const HyperBox<dim> &x) {
	P0=x.P0;
	P1=x.P1;
	C2=x.C2;

	if (x.Rot!=NULL) {
		Rot=new Matrix(3,3);
		*Rot=*(x.Rot);
	} else Rot=NULL;
	return *this;
}

template <int dim> 
HyperBox<dim>::~HyperBox() {
	if (Rot!=NULL) delete Rot;
	Rot=NULL;
}

template <int dim> 
HyperBox<dim> HyperMesh<dim>::RotatedBoundingBox(bool use_baricenter) {
	HyperBox<dim> box;
	Matrix *Rot=new Matrix(3,3);
	Vector<dim> C=GetPrincipalAxes(Rot,use_baricenter);
	Orthonormalize(Rot);
	
	Matrix IR(3,3);
	IR.Set(Rot);
	IR.Traspose();

	box.P0.SetVal(Element_Type_Max);
	box.P1.SetVal(-Element_Type_Max);

	Vector<dim> x;
	for(int i=0;i<num_p;i++) {
		Multiply(&IR,(Points[i]-C),x.GetList());
		x+=C;
		for(int j=0;j<dim;j++) {
			if (x[j]<box.P0[j]) box.P0[j]=x[j];
			if (x[j]>box.P1[j]) box.P1[j]=x[j];
		}
	}
	box.Rot=Rot;
	box.C2=C;

	return box;
}

template <int dim> 
void HyperMesh<dim>::Read(char *filename) {
	char *Ext=Get_File_Extension(filename);
	if (!Ext) return;
	
	if (!_stricmp(Ext,"OBJ")) return ReadOBJ(filename);
	if (!_stricmp(Ext,"OBJV")) return ReadText(filename);
	if (!_stricmp(Ext,"OBJBIN")) return ReadBinary(filename);
	if (!_stricmp(Ext,"SMF")) return ReadSMF(filename);
}
template <int dim> 
void HyperMesh<dim>::NormalizeUVCoords() {
	Assert(Texture_Point,"No texture UV coordinates");
	int num=Texture_Point->numElements();

	HyperBox<2> box;
	box.P0.SetVal(Element_Type_Max);
	box.P1.SetVal(-Element_Type_Max);

	for(int i=0;i<num;i++) {
		Vector<2> p=(*Texture_Point)[i];
		for(int j=0;j<2;j++) {
			if (p[j]<box.P0[j]) box.P0[j]=p[j];
			if (p[j]>box.P1[j]) box.P1[j]=p[j];
		}
	}
	Vector<2> c=box.Center();
	Vector<2> d=box.d();
	box.P1=c+1.02*d;
	box.P0=c-1.02*d;
	Vector<2> m=box.P1-box.P0;
	if (m[0]!=0.0) m[0]=1.0/m[0];
	if (m[1]!=0.0) m[1]=1.0/m[1];
	for(int i=0;i<num;i++) {
		Vector<2> p=(*Texture_Point)[i]-box.P0;
		p[0]=p[0]*m[0];
		p[1]=p[1]*m[1];
		(*Texture_Point)[i]=p;
	}
}
template <int dim> 
void HyperMesh<dim>::CreateUVWfromUV() {
	Assert(Texture_Point,"No texture UV coordinates");
	int num=Texture_Point->numElements();
	
	if (!TextureUVW) {
		TextureUVW=new Array<typename Vector<3>>(num);

		Vector<3> tmp;
		for(int i=0;i<num;i++) {
			tmp[0]=(*Texture_Point)[i][0];
			tmp[1]=(*Texture_Point)[i][1];
			tmp[2]=0.0;
			TextureUVW->append(tmp);
		}
	} else {
		for(int i=0;i<num;i++) {
			(*TextureUVW)[i][0]=(*Texture_Point)[i][0];
			(*TextureUVW)[i][1]=(*Texture_Point)[i][1];
			(*TextureUVW)[i][2]=0.0;
		}
	}
}

template <int dim> 
void HyperMesh<dim>::ReadText(char *filename) {
	
	char text[50+1];
	Vector<dim> r;
	Vector<dim+1> t;
	HyperFace<dim> f;

	IFileBuffer In(filename);
	if (In.IsFinished()) return;
	In.set_wordwidth(50);
	
	while(1) {
		In>>text;
		if (!strcmp(text,"faces")) break;
		In.back();
		
		In>>r;
		AddPoint(r);
	}

	while(1) {
		In>>text;
		if (!strcmp(text,"end")) break;
		In.back();
		
		In>>t;
		for(int i=0;i<dim;i++) f.Point[i]=(int)t[i];
		AddFace(f);
	}
	
	if (In.IsFinished()) return;
	In>>text;
	if (!strcmp(text,"map")) {
		Vector<2> PointBuffer;
		Vector<4> t2d;

		In>>text;
		Texture_FileName[0]=new char[strlen(text)+1];
		strcpy(Texture_FileName[0],text);
		Texture_Point=new Array<typename Vector<2>>(INITIAL_POINT_NUMBER);
		Texture_Face=new Array<FaceType>(num_f);
		
		while(1) {
			In>>text;
			if (!strcmp(text,"faces")) break;
			In.back();
			
			In>>PointBuffer;
			(*Texture_Point).append(PointBuffer);
		}
		
		while(1) {
			In>>text;
			if (!strcmp(text,"end")) break;
			In.back();
			
			In>>t2d;
			for(int i=0;i<3;i++) f.Point[i]=(int)t2d[i];
			(*Texture_Face).append(f);
		}	

	}

}

template <int dim> 
void HyperMesh<dim>::ReadOBJ(char *filename) {
	
	Vector<dim> r;
	HyperFace<dim> f;
	r.SetZero();
	for(int k=0;k<dim;k++) f.Point[k]=0;

	IFileBuffer In(filename);
	if (In.IsFinished()) return;

	char LineBuffer[500];
	char word[500];
	int buffer_len=500;
	double tmp[3];
	int tmp_int[4];
	int first_point=num_p;

	#define CHECK_READ_OBJ if (!data) {cout<<"OBJ importer detects a format error.\n";return;}

	while (!In.IsFinished()) {
		int len=500-1;In.ReadPharse(LineBuffer,len);

		char *data=GetSpacedWord(LineBuffer,word,buffer_len,' ');
		if (!data) continue;
		if (strlen(word)>1) continue;

		if (word[0]=='v') {
			size_t size_data=strlen(data);
			for(size_t i=0;i<size_data;i++) if (data[i]==',') data[i]='.';
				
			data=GetSpacedWord(data,word,buffer_len,' ');
			CHECK_READ_OBJ;
			tmp[0]=atof(word);
			data=GetSpacedWord(data,word,buffer_len,' ');
			CHECK_READ_OBJ;
			tmp[1]=atof(word);
			data=GetSpacedWord(data,word,buffer_len,' ');
			CHECK_READ_OBJ;
			tmp[2]=atof(word);

			for(int k=0;k<min(dim,3);k++) r[k]=tmp[k];
			AddPoint(r);
		} else {
			if (word[0]=='f') {
				data=GetSpacedWord(data,word,buffer_len,' ');
				CHECK_READ_OBJ;
				tmp_int[0]=atoi(word)-1;
				data=GetSpacedWord(data,word,buffer_len,' ');
				CHECK_READ_OBJ;
				tmp_int[1]=atoi(word)-1;
				data=GetSpacedWord(data,word,buffer_len,' ');
				CHECK_READ_OBJ;
				tmp_int[2]=atoi(word)-1;

				for(int k=0;k<min(dim,3);k++) f.Point[k]=tmp_int[k]+first_point;
				AddFace(f);

				data=GetSpacedWord(data,word,buffer_len,' ');
				if (data) {
					tmp_int[3]=atoi(word)-1;

					if (dim>=3) {
						f.Point[0]=tmp_int[2]+first_point;
						f.Point[1]=tmp_int[3]+first_point;
						f.Point[2]=tmp_int[0]+first_point;
						AddFace(f);
					}
				}
			}
		}

	}

}

template <int dim> 
void HyperMesh<dim>::AddMesh(HyperMesh<dim> *input) {
	
	HyperFace<dim>  f;
	int             first_point   =num_p;
	int             org_num_f     =num_f;
	int             first_texture =num_textures;
	Vector<dim>    *InputPoint    =input->Points.getMem();
	HyperFace<dim> *InputFace     =input->Faces.getMem();
	
	
	for(int i=0;i<input->num_p;i++,InputPoint++) AddPoint(*InputPoint);
	
	for(int i=0;i<input->num_f;i++,InputFace++) {
		for(int k=0;k<dim;k++) f.Point[k]=InputFace->Point[k]+first_point;
		AddFace(f);
	}
	
	// Check integrity
	if ((org_num_f!=0) && (input->num_f!=0)) {
		if ((num_textures!=0) ^ (input->num_textures!=0)) ErrorExit("Cannot mix textured and non-textured mesh models.");
	} 

	if (input->num_textures!=0) {
		if (!Texture_Point) Texture_Point=new Array<typename Vector<2>>(input->num_p);
		if (!Texture_Face)  Texture_Face=new Array<FaceType>(input->num_f);
		num_textures+=input->num_textures;

		int first_t_point=Texture_Point->numElements();
		int first_t_face =Texture_Face->numElements();
		if (first_t_face!=org_num_f) ErrorExit("The number of faces mismatch with the number of texture faces in the original mesh.");

		for(int i=0;i<input->num_textures;i++) {
			if (i+first_texture>=MAX_TEXTURES) ErrorExit("Maximum number of texture reached.");
			Texture_GLMap[i+first_texture]=input->Texture_GLMap[i];
			Texture_Map[i+first_texture]=input->Texture_Map[i];
			Texture_FileName[i+first_texture]=strcln(input->Texture_FileName[i]);
			Texture_FaceInterval[i+first_texture]=input->Texture_FaceInterval[i]+first_t_face;
		}

		Vector<2>      *InputTPoint   =input->Texture_Point->getMem();
		HyperFace<dim> *InputTFace    =input->Texture_Face->getMem();
		int             num_t_p       =input->Texture_Point->numElements();
		int             num_t_f       =input->Texture_Face->numElements();

		for(int i=0;i<num_t_p;i++,InputTPoint++) Texture_Point->append(*InputTPoint);

		for(int i=0;i<num_t_f;i++,InputTFace++) {
			for(int k=0;k<dim;k++) f.Point[k]=InputTFace->Point[k]+first_t_point;
			Texture_Face->append(f);
		}
	}

	SDELETE(TextureUVW);
	ResetVolatileProperties();
}

template <int dim> 
void HyperMesh<dim>::ReadBinary(char *filename) {
	
	Vector<dim> r;
	HyperFace<dim> f;

	int len;
	__int32 pnum,fnum;
	float   buffer[dim];
	__int32 buffer2[dim];
	int     first_point=num_p;

	IFileBuffer In(filename);
	if (In.IsFinished()) return;

	len=4;
	In.ReadLine((char*)&pnum,len);

	for(int i=0;i<pnum;i++) {
		len=dim*4;
		In.ReadLine((char*)&buffer,len);
		for(int k=0;k<dim;k++) r[k]=buffer[k];
		AddPoint(r);
	}

	len=4;
	In.ReadLine((char*)&fnum,len);

	for(int i=0;i<fnum;i++) {
		len=4*dim;
		In.ReadLine((char*)&buffer2,len);
		
		for(int k=0;k<dim;k++) f.Point[k]=buffer2[k]+first_point;
		AddFace(f);
	}
	
	if (In.IsFinished()) return;
	
	// Texture
	char      FileName[201];
	float     PointBuffer[2];
	Vector<2> VectPointBuffer;

	len=200;In.ReadPharse(FileName,len);
	Texture_FileName[num_textures]=new char[strlen(FileName)+1];
	strcpy(Texture_FileName[num_textures],FileName);

	len=4;In.ReadLine((char*)&pnum,len);

	if (!Texture_Point) Texture_Point=new Array<typename Vector<2>>(pnum);
	if (!Texture_Face) Texture_Face=new Array<FaceType>(num_f);
	Texture_FaceInterval[num_textures]=Texture_Face->numElements();
	num_textures++;


	int first_t_point=Texture_Point->numElements();
	for(int i=0;i<pnum;i++) {
		len=2*4;
		In.ReadLine((char*)PointBuffer,len);
		VectPointBuffer[0]=PointBuffer[0];
		VectPointBuffer[1]=PointBuffer[1];
		(*Texture_Point).append(VectPointBuffer);
	}

	for(int i=0;i<fnum;i++) {
		len=4*dim;
		In.ReadLine((char*)&buffer2,len);
		
		for(int k=0;k<dim;k++) f.Point[k]=buffer2[k]+first_t_point;
		(*Texture_Face).append(f);
	}
	
	if (In.IsFinished()) return;

	len=4;In.ReadLine((char*)&pnum,len);
	int texture_face_starting_point=Texture_FaceInterval[num_textures-1];
	Texture_FaceInterval[num_textures-1]=texture_face_starting_point+pnum;
	while(!In.IsFinished()) {
		len=200;In.ReadPharse(FileName,len);
		Texture_FileName[num_textures]=new char[strlen(FileName)+1];
		strcpy(Texture_FileName[num_textures],FileName);
		len=4;In.ReadLine((char*)&pnum,len);
		Texture_FaceInterval[num_textures]=texture_face_starting_point+pnum;

		num_textures++;
	}

}

template <int dim> 
void HyperMesh<dim>::Move(Vector<dim> T) {
	for(int i=0;i<num_p;i++) Points[i]=Points[i]+T;
}

/*
template <> 
double PointFaceDistance(Vector<dim> P,int f) {

}*/

template <int dim> 
void HyperMesh<dim>::Save(char *filename) {
	char *Ext=Get_File_Extension(filename);
	if (!Ext) return;
	
	if (!_stricmp(Ext,"OBJV")) return SaveText(filename);
	if (!_stricmp(Ext,"OBJ"))  return SaveOBJ(filename);
	if (!_stricmp(Ext,"OBJBIN")) return SaveBinary(filename);
	if (!_stricmp(Ext,"VRML")) return SaveVRML(filename);
	if (!_stricmp(Ext,"WRL")) return SaveVRML(filename);	
	if (!_stricmp(Ext,"SMF")) return SaveSMF(filename);
}

template <int dim> 
void HyperMesh<dim>::SaveText(char *filename) {

	OFileBuffer Out(filename);

	for(int i=0;i<num_p;i++) {
		Out<<Points[i];
		if (i!=num_p-1) Out<<",";
		Out<<"\r\n";
	}

	Out<<"faces\r\n";

	for(int i=0;i<num_f;i++) {
		for(int j=0;j<dim;j++) Out<<Faces[i].Point[j]<<",";
		Out<<"-1";
		if (i!=num_f-1) Out<<",";
		Out<<"\r\n";
	}

	Out<<"end\r\n";
	

	if (Texture_FileName[0]) {
		Out<<"map\r\n";
		Out<<Texture_FileName[0]<<"\r\n";

		for(int i=0;i<(*Texture_Point).numElements();i++) {
			Out<<(*Texture_Point)[i];
			if (i!=((*Texture_Point).numElements())-1) Out<<",";
			Out<<"\r\n";
		}

		Out<<"faces\r\n";

		for(int i=0;i<num_f;i++) {
			for(int j=0;j<dim;j++) Out<<(*Texture_Face)[i].Point[j]<<",";
			Out<<"-1";
			if (i!=num_f-1) Out<<",";
			Out<<"\r\n";
		}

		Out<<"end\r\n";
	}

	Out.Close();
}

template <int dim> 
void HyperMesh<dim>::SaveOBJ(char *filename) {

	OFileBuffer Out(filename);

	for(int i=0;i<num_p;i++) {
		Out<<"v "<<Points[i];
		if (i!=num_p-1) Out<<"\r\n";
	}

	for(int i=0;i<num_f;i++) {
		Out<<"f ";
		for(int j=0;j<dim;j++) Out<<Faces[i].Point[j]<<" ";
		if (i!=num_f-1) Out<<"\r\n";
	}

	Out.Close();
}

template <int dim> 
void HyperMesh<dim>::SaveBinary(char *filename) {

	__int32 num;
	float buffer[dim];
	OFileBuffer Out(filename);

	num=num_p;
	Out.WriteLine((char *)&num,4);
	
	for(int i=0;i<num_p;i++) {
		Points[i].SerializeFloat(buffer);
		Out.WriteLine((char *)buffer,dim*4);
	}

	num=num_f;
	Out.WriteLine((char *)&num,4);

	for(int i=0;i<num_f;i++) {
		for(int j=0;j<dim;j++) {
			__int32 fp=Faces[i].Point[j];
			Out.WriteLine((char *)&(fp),4);
		}
	}

	
	// Texture
	if (Texture_FileName[0]) {
		Out.WriteLine(Texture_FileName[0],(int)strlen(Texture_FileName[0]));
		Out.WriteLine("\n",1);

		num=(*Texture_Point).numElements();
		Out.WriteLine((char *)&num,4);
		
		for(int i=0;i<num;i++) {
			(*Texture_Point)[i].SerializeFloat(buffer);
			Out.WriteLine((char *)buffer,2*4);
		}

		for(int i=0;i<num_f;i++) {
			for(int k=0;k<dim;k++) {
				__int32 fp=(*Texture_Face)[i].Point[k];
				Out.WriteLine((char *)&(fp),4);
			}
		}

	}

	if (num_textures>1) {
		num=Texture_FaceInterval[0];
		Out.WriteLine((char *)&num,4);

		for(int i=1;i<num_textures;i++) {
			Out.WriteLine(Texture_FileName[i],(int)strlen(Texture_FileName[i]));
			Out.WriteLine("\n",1);
			num=Texture_FaceInterval[i];
			Out.WriteLine((char *)&num,4);
		}
	}

	Out.Close();
}


template <int dim> 
void HyperMesh<dim>::SaveSMF(char *filename) {

	OFileBuffer Out(filename);
	Out<<"#$SMF 1.0\n#$vertices "<<num_p<<"\n#$faces "<<num_f<<"\n";

	Out.setPrecision(7,false);

	for(int i=0;i<num_p;i++) Out<<"v "<<Points[i]<<"\n";

	for(int i=0;i<num_f;i++) {
		Out<<"t ";
		for(int j=0;j<dim;j++) Out<<(Faces[i].Point[j]+1)<<" ";
		Out<<"\n";
	}

	Out<<"\n";

	Out.Close();
}

template <int dim> 
void HyperMesh<dim>::ReadSMF(char *filename) {
	
	char text[50+1];
	Vector<dim> r;
	Vector<dim> t;
	HyperFace<dim> f;

	IFileBuffer In(filename);
	if (In.IsFinished()) return;
	In.set_wordwidth(50);

	In>>text;
	
	while(1) {
		In>>text;
		if (!strcmp(text,"f")) break;
		
		In>>r;
		AddPoint(r);
	}

	In.back();

	while(1) {
		In>>text;
		if (!strcmp(text,"end")) break;
		
		In>>t;
		for(int i=0;i<dim;i++) f.Point[i]=((int)t[i])-1;
		AddFace(f);
	}
	
}

template <int dim> 
void HyperMesh<dim>::SaveVRML(char *filename) {

	OFileBuffer Out(filename);

Out<<"#VRML V2.0 utf8\r\nBackground {\r\nskyColor [0.007843 0.3922 0.6157, ]\r\n\
groundColor [0.702 0.6 0.3843, ]}\r\n\
Viewpoint {\r\n\
position 0.017944 -0.011094 0.528541}\r\n\
Shape {\r\n\
appearance Appearance {\r\n\
material Material {diffuseColor 0.9 0.9 0.8}}\r\n\
geometry IndexedFaceSet {\r\n\
coord Coordinate {\r\n\
point [\r\n";

	for(int i=0;i<num_p;i++) {
		Out<<Points[i];
		if (i!=num_p-1) Out<<",";
		Out<<"\r\n";
	}

	Out<<"]\r\n}\r\nsolid FALSE\r\ncoordIndex [\r\n";

	for(int i=0;i<num_f;i++) {
		for(int j=0;j<dim;j++) Out<<Faces[i].Point[j]<<",";
		Out<<"-1";
		if (i!=num_f-1) Out<<",";
		Out<<"\r\n";
	}

	Out<<"]}}";
	////
	char text[200+1];
	int len;
	IFileBuffer In("ToAdd.wrl");
	while (!In.IsFinished()) {
		len=200;
		In.ReadPharse(text,len);
		Out<<text<<"\r\n";
	}
	//
	Out.Close();
}

template <int dim> 
double HyperMesh<dim>::AverageArea() {
	Average_Area=0;

	for(int i=0;i<num_f;i++) Average_Area+=Faces[i].Area;
	Average_Area/=num_f;

	return Average_Area;
}

template <int dim> 
double HyperMesh<dim>::ComputaArea() {
	double tmp_area;
	Total_Area=0;

	for(int j=0;j<num_p;j++) AreaPoint[j]=0.0;

	for(int i=0;i<num_f;i++) {
		tmp_area=Faces[i].ComputaArea(&Points);
		
		for(int j=0;j<dim;j++) AreaPoint[Faces[i].Point[j]]+=tmp_area;
		
		Total_Area+=tmp_area;
	}
	
	for(int j=0;j<num_p;j++) AreaPoint[j]/=Total_Area;
	
	return Total_Area;
}

template <int dim> 
int HyperMesh<dim>::PlaneSmallFaces(double limite_area) {
	int planed=0;

	// Make plane some verticles
	for(int i=0;i<num_f;i++) {
		if (Faces[i].Area<limite_area) {
			planed++;
			for(int j=0;j<dim;j++) {
				Vector<dim> average=Points[Faces[i].Point[j]];
				for(int n=0;n<NumNeighbours[Faces[i].Point[j]];n++) {
					average+=Points[Neighbours[Faces[i].Point[j]][n]];
				}
				average*=(1.0/(NumNeighbours[Faces[i].Point[j]]+1));
				Points[Faces[i].Point[j]]=average;
			}
		}
	}
	return planed;
}

#define NUM 60
int Isto[NUM];

template <int dim> 
double HyperMesh<dim>::Qequ() {
	double qequ=0,t;
	for(int i=0;i<NUM;i++) Isto[i]=0;

	for(int i=0;i<num_f;i++) {
		double a=(Points[Faces[i].Point[0]]-Points[Faces[i].Point[1]]).Norm2();
		double b=(Points[Faces[i].Point[1]]-Points[Faces[i].Point[2]]).Norm2();
		double c=(Points[Faces[i].Point[2]]-Points[Faces[i].Point[0]]).Norm2();

		double max=a;
		if ((a>=b) && ((a>=c))) max=a;
		if ((b>=a) && ((b>=c))) max=b;
		if ((c>=a) && ((c>=b))) max=c;
		
		double s=0.5*(a+b+c);
		t=3.464101615*Faces[i].Area/(s*max);
		qequ+=t;
		
		if (t>1) printf("Errore: t>1!!\n");
		Isto[(int)(t*NUM)]++;
	}
	qequ/=num_f;

	printf("Qequ:\n");
	for(int i=0;i<NUM;i++) {
		printf("%f          %i\n",i*1.0/NUM+0.5/NUM,Isto[i]);
	}
	return qequ;
}

template <int dim> 
int HyperMesh<dim>::FaceNeighbour(int edge,int face) {
	int p1=Faces[face].Point[edge];
	int p2=Faces[face].Point[(edge+1)%dim];

	int i;
	for(i=0;i<NumNeighbourFaces[p1];i++) {
		int f=NeighbourFaces[p1][i];
		if (f==face) continue;
		
		if (Faces[f].find_point(p2)!=-1) break;
	}
	if (i>=NumNeighbourFaces[p1]) return -1;

	return NeighbourFaces[p1][i];
}


template <int dim> 
double HyperMesh<dim>::Qnor() {
	double tot=0;
	for(int i=0;i<NUM;i++) Isto[i]=0;

	for(int i=0;i<num_f;i++) {
		Vector<dim> b=Faces[i].Normal(&Points);
		double sum=0;
		int count=0;
		for(int j=0;j<3;j++) {
			int f=FaceNeighbour(j,i);
			if (f!=-1) {
				Vector<dim> a=Faces[f].Normal(&Points);
				sum+=fabs(a*b);
				count++;
			}
		}
		if (count!=0) sum/=count;
		else sum=0;
		tot+=sum;

#define TRASLA 0.96

		if (sum>1) printf("Errore: %f>1!!\n",sum);
		sum-=TRASLA;
		sum/=(1.0-TRASLA);
		if (sum>=0) {
			Isto[(int)(sum*NUM)]++;
		}
	}

	printf("Qnor:\n");
	for(int i=0;i<NUM;i++) {
		printf("%f          %i\n",TRASLA+(i*1.0/NUM)*(1.0-TRASLA)+(1.0-TRASLA)*0.5/NUM,Isto[i]);
	}
	return tot/num_f;
}

template <int dim> 
void HyperMesh<dim>::RegolarizeFaceNormals() {
	for(int i=0;i<num_f;i++) {
		Vector<dim> Fn;
		for(int j=0;j<dim;j++) Fn+=Normals[Faces[i].Point[j]];
		
		if ((Fn*Faces[i].Normal(&Points))<0) {
			int tmp=Faces[i].Point[2];
			Faces[i].Point[2]=Faces[i].Point[1];
			Faces[i].Point[1]=tmp;
		}
	}

}

// cancello le facce con doppi edge...
// controllo se vi sono vertici inutilizzati


template <int dim> 
void HyperMesh<dim>::DeleteBounduaryEdges() {
	
	// cancello le facce di controrno

	Array<int> ToDeleteface(100);
	
	int first,second;
	bool toClose;

	for(int i=0;i<num_p;i++) {
		first=second=-1;
		toClose=true;
		for(int j=0;j<NumNeighbours[i];j++) {
			// edge i -> Neighbours[i][j]
			if (Edge_Faces[i][j].free2!=-1) continue;

			// edge di confine
			if (first==-1) {
				first=Neighbours[i][j];
				continue;
			}
			if (second==-1) {
				second=Neighbours[i][j];
				continue;
			}
		}
		
		if ((second==-1) && (first!=-1)) {
			// i -> first e' un edge di confine non chiudibile
			for(int j=0;j<NumNeighbourFaces[i];j++) {
				if (Faces[NeighbourFaces[i][j]].find_point(first)!=-1) {
					// NeighbourFaces[i][j] e' una faccia di bordo
					ToDeleteface.append(NeighbourFaces[i][j]);
					break;
				}
			}
		}
	}

	// Creating new faces 
	int old_num_p=num_p;
	int old_num_f=num_f;
	List<PointType> old_Points(Points);
	List<FaceType> old_Faces(Faces);

	// Reset
	num_p=0;
	num_f=0;

	for(int i=0;i<old_num_p;i++) AddPoint(old_Points[i]);
	
	for(int i=0;i<old_num_f;i++) 
		if (ToDeleteface.search(i)==-1) AddFace(old_Faces[i]);

}

template <int dim> 
void HyperMesh<dim>::RemoveZeroEdgedFaces() {
	
	// cancello le facce di controrno

	Array<int> ToDeleteface(100);
	

	for(int i=0;i<num_f;i++) {
		double a=(Points[Faces[i].Point[0]]-Points[Faces[i].Point[1]]).Norm2();
		double b=(Points[Faces[i].Point[1]]-Points[Faces[i].Point[2]]).Norm2();
		double c=(Points[Faces[i].Point[2]]-Points[Faces[i].Point[0]]).Norm2();
		if ((a==0) || (b==0) || (c==0)) ToDeleteface.append(i);
	}

	printf("-> Finded %i zero edged faces.\n",ToDeleteface.numElements());
	// Creating new faces 
	int old_num_p=num_p;
	int old_num_f=num_f;
	List<PointType> old_Points(Points);
	List<FaceType> old_Faces(Faces);

	// Reset
	num_p=0;
	num_f=0;

	for(int i=0;i<old_num_p;i++) AddPoint(old_Points[i]);
	
	for(int i=0;i<old_num_f;i++) 
		if (ToDeleteface.search(i)==-1) AddFace(old_Faces[i]);

}

template <int dim> 
void HyperMesh<dim>::CloseSmallHoles(int &closed,int &multipleholes,int &bounduaryedge) {

	closed=0;
	multipleholes=0;
	bounduaryedge=0;

	int first,second;
	bool toClose;

	for(int i=0;i<num_p;i++) {
		first=second=-1;
		toClose=true;
		for(int j=0;j<NumNeighbours[i];j++) {
			// edge i -> Neighbours[i][j]
			if (Edge_Faces[i][j].free2!=-1) continue;
			// edge di confine
			if (first==-1) {
				first=Neighbours[i][j];
				continue;
			}
			if (second==-1) {
				second=Neighbours[i][j];
				continue;
			}
			// ci sono piu' di 2 confini
			multipleholes++;
			toClose=false;
			break;
		}
		
		
		// nn chiude se nn c'e' nessun confine, ce ne e' uno (mmm), ce ne sono piu' di 2
		if (second==-1) {
			toClose=false;
			if (first!=-1) bounduaryedge++;
		}
		if (!toClose) continue;
	
		HyperFace<dim> F;
		F.Point[0]=i;
		F.Point[1]=first;
		F.Point[2]=second;
		
		if ((Normals[i]*F.Normal(&Points))<0) {
			F.Point[2]=first;
			F.Point[1]=second;
		}

		AddFace(F);
		closed++;
	}

}




#define ADD_EDGE    AddEdgeSorted
#define SEARCH_EDGE SearchEdgeSorted


inline int SearchEdge(Array<Edge> *Edges,int i,int j) {
	Edge *curr_edge=Edges->getMem();
	int num_edges=Edges->numElements();
	for(int k=0;k<num_edges;k++,curr_edge++) {
		if (curr_edge->V1==i) {
			if (curr_edge->V2!=j) continue;
			// i-j trovato
			return k;
		}
		if (curr_edge->V1==j) {
			if (curr_edge->V2!=i) continue;
			// i-j trovato
			return k;
		}
	}
	return -1;
}

int edge_compare(Edge *a,Edge *b) {
	if (a->V1<b->V1) return -1;
	if (a->V1>b->V1) return +1;
	return 0;
}

inline int SearchEdgeSorted(Array<Edge> *Edges,int i,int j) {
	int s,k;
	Edge *edge_list=Edges->getMem();
	int   num_edges=Edges->numElements();
	if (num_edges==0) return -1;

	Edge tmp_s;
	tmp_s.V1=tmp_s.V2=i;
	k=Edges->binary_search(edge_compare,&tmp_s);
	if (k!=-1) {
		s=k;
		while((s>=0) && (edge_list[s].V1==i)) {
			if (edge_list[s].V2==j) return s;
			s--;
		}
		s=k;
		while((s<=num_edges-1) && (edge_list[s].V1==i)) {
			if (edge_list[s].V2==j) return s;
			s++;
		}
	}
	
	tmp_s.V1=tmp_s.V2=j;
	k=Edges->binary_search(edge_compare,&tmp_s);
	if (k!=-1) {
		s=k;
		while((s>=0) && (edge_list[s].V1==j)) {
			if (edge_list[s].V2==i) return s;
			s--;
		}
		s=k;
		while((s<=num_edges-1) && (edge_list[s].V1==j)) {
			if (edge_list[s].V2==i) return s;
			s++;
		}
	}

	return -1;
}

template <int dim> 
inline int HyperMesh<dim>::AddEdgeSorted(Array<Edge> *Edges,int i,int j) {

	int edge=SearchEdgeSorted(Edges,i,j);
	if (edge!=-1) return edge;

	Vector<dim> e;
	e=(Points[i]+Points[j])*0.5;
	int i_e=AddPoint(e);

	Edge t;
	t.V1=i;
	t.V2=j;
	t.center=i_e;

	return Edges->append_sorted(t,edge_compare);
}


template <int dim> 
inline int HyperMesh<dim>::AddEdge(Array<Edge> *Edges,int i,int j) {

	int edge=SearchEdge(Edges,i,j);
	if (edge!=-1) return edge;

	Vector<dim> e;
	e=(Points[i]+Points[j])*0.5;
	int i_e=AddPoint(e);

	Edge t;
	t.V1=i;
	t.V2=j;
	t.center=i_e;
	return (Edges->append(t));
}

template <int dim> 
void HyperMesh<dim>::SubDivision(double limite,int Method,Array<int> *face_list) {
	bool divide=false;
	FaceType *curr_face;
	if (face_list!=NULL) Method=FACE_LIST_SUBDIVISION_METHOD;
	if ((face_list==NULL) && (Method==FACE_LIST_SUBDIVISION_METHOD)) ErrorExit("face_list is NULL");


	printf("-- Subdivision start\n"); 


	UINT expected_no_edges=0;	
	
	if (Method!=FACE_LIST_SUBDIVISION_METHOD) {
		curr_face=Faces.getMem();
		for(int i=0;i<num_f;i++,curr_face++) {
			switch(Method) {
				case AREA_SUBDIVISION_METHOD:
					divide=(curr_face->Area>limite);
					break;
				case CURVATURE_SUBDIVISION_METHOD:
					{
						double t=CurvatureNormals[curr_face->Point[0]].Norm2()+CurvatureNormals[curr_face->Point[1]].Norm2()+CurvatureNormals[curr_face->Point[2]].Norm2();
						t/=3;
						divide=(t>limite);
					}
					break;
				case HALF_SUBDIVISION_METHOD:
					{
						Vector<dim> x=curr_face->Baricentro(&Points);
						divide=x[0]<0;
					}
					break;
				default:
					break;
			};
			if (divide) expected_no_edges+=3;
		}
	} else {
		expected_no_edges=face_list->numElements()*3;
	}


	// Memory Allocation
	Array<Edge> Edges(expected_no_edges+20);




	printf("-> Subdividing %u edges.\n",expected_no_edges); 

	if (Method!=FACE_LIST_SUBDIVISION_METHOD) {
		curr_face=Faces.getMem();
		for(int i=0;i<num_f;i++,curr_face++) {
			switch(Method) {
				case AREA_SUBDIVISION_METHOD:
					divide=(curr_face->Area>limite);
					break;
				case CURVATURE_SUBDIVISION_METHOD:
					{
						double t=CurvatureNormals[curr_face->Point[0]].Norm2()+CurvatureNormals[curr_face->Point[1]].Norm2()+CurvatureNormals[curr_face->Point[2]].Norm2();
						t/=3;
						divide=(t>limite);
					}
					break;
				case HALF_SUBDIVISION_METHOD:
					{
						Vector<dim> x=curr_face->Baricentro(&Points);
						divide=x[0]<0;
					}
					break;
				default:
					divide=false;
					break;
			};

			if (divide) {
				ADD_EDGE(&Edges,curr_face->Point[0],curr_face->Point[1]);
				ADD_EDGE(&Edges,curr_face->Point[1],curr_face->Point[2]);
				ADD_EDGE(&Edges,curr_face->Point[2],curr_face->Point[0]);
			}
		}
	} else {
		curr_face=Faces.getMem();
		int *cur_index=face_list->getMem();
		int num_ele=face_list->numElements();
		for(int i=0;i<num_ele;i++,cur_index++) {
			ADD_EDGE(&Edges,curr_face[(*cur_index)].Point[0],curr_face[(*cur_index)].Point[1]);
			ADD_EDGE(&Edges,curr_face[(*cur_index)].Point[1],curr_face[(*cur_index)].Point[2]);
			ADD_EDGE(&Edges,curr_face[(*cur_index)].Point[2],curr_face[(*cur_index)].Point[0]);
		}
	}

	printf("-> Edges subdivided.\n");





	// Creating new faces 
	int old_num_f=num_f;
	List<FaceType>  old_Faces(Faces);

	// Reset mesh faces
	num_f=0;

	// Reset neigboring structure
	if (update_neighbours_structure) {
		int *tmp_m1=NumNeighbours.getMem();
		int *tmp_m2=NumNeighbourFaces.getMem();
		for(int i=0;i<num_p;i++,tmp_m1++,tmp_m2++) {*tmp_m1=0;*tmp_m2=0;}
	}


	FaceType *curr_old_face=old_Faces.getMem();
	for(int i=0;i<old_num_f;i++,curr_old_face++) {
		int i1=SEARCH_EDGE(&Edges,curr_old_face->Point[0],curr_old_face->Point[1]);
		int i2=SEARCH_EDGE(&Edges,curr_old_face->Point[1],curr_old_face->Point[2]);
		int i3=SEARCH_EDGE(&Edges,curr_old_face->Point[2],curr_old_face->Point[0]);

		// Casi:
		int s=((i1==-1)?0:1)+((i2==-1)?0:1)+((i3==-1)?0:1);
		if (s==0) AddFace(old_Faces[i]);
		if (s==3) {
			HyperFace<dim> N;
			N.Point[0]=curr_old_face->Point[0];
			N.Point[1]=Edges[i1].center;
			N.Point[2]=Edges[i3].center;
			AddFace(N);
			N.Point[0]=Edges[i1].center;
			N.Point[1]=curr_old_face->Point[1];
			N.Point[2]=Edges[i2].center;
			AddFace(N);
			N.Point[0]=Edges[i2].center;
			N.Point[1]=curr_old_face->Point[2];
			N.Point[2]=Edges[i3].center;
			AddFace(N);
			N.Point[0]=Edges[i1].center;
			N.Point[1]=Edges[i2].center;
			N.Point[2]=Edges[i3].center;
			AddFace(N);
		}
		if (s==1) {
			HyperFace<dim> N;
			if (i1!=-1) {
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=Edges[i1].center;
				N.Point[2]=curr_old_face->Point[2];
				AddFace(N);
				N.Point[0]=curr_old_face->Point[1];
				N.Point[1]=curr_old_face->Point[2];
				N.Point[2]=Edges[i1].center;
				AddFace(N);
			}
			if (i2!=-1) {
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=curr_old_face->Point[1];
				N.Point[2]=Edges[i2].center;
				AddFace(N);
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=Edges[i2].center;
				N.Point[2]=curr_old_face->Point[2];
				AddFace(N);
			}
			if (i3!=-1) {
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=curr_old_face->Point[1];
				N.Point[2]=Edges[i3].center;
				AddFace(N);
				N.Point[0]=curr_old_face->Point[1];
				N.Point[1]=curr_old_face->Point[2];
				N.Point[2]=Edges[i3].center;
				AddFace(N);
			}
		}
		if (s==2) {
			HyperFace<dim> N;
			if (i1!=-1) {
				if (i2!=-1) {
					N.Point[0]=curr_old_face->Point[0];
					N.Point[1]=Edges[i1].center;
					N.Point[2]=curr_old_face->Point[2];
					AddFace(N);
					N.Point[0]=curr_old_face->Point[1];
					N.Point[1]=Edges[i2].center;
					N.Point[2]=Edges[i1].center;
					AddFace(N);
					N.Point[0]=Edges[i1].center;
					N.Point[1]=Edges[i2].center;
					N.Point[2]=curr_old_face->Point[2];
					AddFace(N);
				}
				if (i3!=-1) {
					N.Point[0]=curr_old_face->Point[1];
					N.Point[1]=curr_old_face->Point[2];
					N.Point[2]=Edges[i1].center;
					AddFace(N);
					N.Point[0]=curr_old_face->Point[0];
					N.Point[1]=Edges[i1].center;
					N.Point[2]=Edges[i3].center;
					AddFace(N);
					N.Point[0]=Edges[i1].center;
					N.Point[1]=curr_old_face->Point[2];
					N.Point[2]=Edges[i3].center;
					AddFace(N);
				}			
			}		
			if ((i2!=-1) && (i3!=-1)) {
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=curr_old_face->Point[1];
				N.Point[2]=Edges[i2].center;
				AddFace(N);
				N.Point[0]=curr_old_face->Point[0];
				N.Point[1]=Edges[i2].center;
				N.Point[2]=Edges[i3].center;
				AddFace(N);
				N.Point[0]=Edges[i3].center;
				N.Point[1]=Edges[i2].center;
				N.Point[2]=curr_old_face->Point[2];
				AddFace(N);
			}
		} // if
	} // for 

	printf("-- Subdivision finished.\n"); 

}

template <int dim> 
void HyperMesh<dim>::Optimize(double limite_area) {
	
	printf("-- Optimizer start\n"); 

	int todelete=PlaneSmallFaces(limite_area);
	PlaneSmallFaces(limite_area);			// utera piu' volte cosi si syabilizza su un piano
	PlaneSmallFaces(limite_area);

	printf("-> %i faces to delete.\n",todelete); 

	char text[200];
	sprintf(text,"qslim -M smf -t %i -o temp2.smf temp.smf",num_f-todelete); 

	SaveSMF("temp.smf");
	system(text);
	

	num_p=0;
	num_f=0;
	ReadSMF("temp2.smf");
	system("del temp.smf");
	system("del temp2.smf");

	printf("-- Optimizer finished.\n"); 
}

template <int dim> 
void HyperMesh<dim>::Simplify(int to_num_faces) {
	
	printf("-- Simplify start\n"); 

	char text[200];
	sprintf(text,"qslim -M smf -t %i -o temp2.smf temp.smf",to_num_faces); 

	SaveSMF("temp.smf");
	system(text);
	

	num_p=0;
	num_f=0;
	ReadSMF("temp2.smf");
	system("del temp.smf");
	system("del temp2.smf");

	printf("-- Simplify finished.\n"); 
}

template <> 
int HyperMesh<3>::PlaneVertexIndex(Vector<2> TL,Vector<2> BR,int r,int c,Vector<3> p) {

	int i_c=Approx((p[0]-TL[0])/((1.0/c)*(BR[0]-TL[0])));
	if ((i_c<0) || (i_c>c)) return -1;

	int i_r=Approx((p[1]-TL[1])/((1.0/r)*(BR[1]-TL[1])));
	if ((i_r<0) || (i_r>r)) return -1;

	return (i_c+i_r*(c+1));
}

template <> 
int HyperMesh<2>::PlaneVertexIndex(Vector<2> TL,Vector<2> BR,int r,int c,Vector<3> p) {
	ErrorExit("PlaneVertexIndex: Not implemented.");
	return 0;
}

template <> 
Vector<3> HyperMesh<3>::PlaneVertexFromIndex(Vector<2> TL,Vector<2> BR,int r,int c,int index) {

	int i_r=index/(c+1);
	int i_c=index-(i_r*(c+1));
	if ((i_c<0) || (i_c>c)) ErrorExit("This vertex does not exists");
	if ((i_r<0) || (i_r>r)) ErrorExit("This vertex does not exists");

	Vector<3> P;
	P[0]=TL[0]+(i_c*(1.0/c)*(BR[0]-TL[0]));
	P[1]=TL[1]+(i_r*(1.0/r)*(BR[1]-TL[1]));

	return P;
}

template <> 
Vector<3> HyperMesh<2>::PlaneVertexFromIndex(Vector<2> TL,Vector<2> BR,int r,int c,int index) {
	ErrorExit("PlaneVertexFromIndex: Not implemented.");
	return ZERO_3;
}

template <> 
void HyperMesh<2>::create_plane(Vector<2> TL,Vector<2> BR,int r,int c) {
	ErrorExit("create_plane: Not implemented.");
}

template <> 
void HyperMesh<3>::create_plane(Vector<2> TL,Vector<2> BR,int r,int c) {
	// TODO: CANCELLARE STRUTTURA PRECEDENTE!!!!
	
	Vector<3> P,P_row;
	Vector<3> step_x,step_y;

	step_x[0]=(1.0/c)*(BR[0]-TL[0]);
	step_x[1]=0.0;
	step_y[0]=0.0;
	step_y[1]=(1.0/r)*(BR[1]-TL[1]);
	
	P_row[0]=TL[0];
	P_row[1]=TL[1];

	for(int i=0;i<=r;i++) {
		P=P_row;
		for(int j=0;j<=c;j++) {
			AddPoint(P);
			P=P+step_x;
		}
		P_row=P_row+step_y;
	}

	HyperFace<3> F;
	for(int i=0;i<r;i++) {
		for(int j=0;j<c;j++) {
			F.Point[0]=i*(c+1)+j;
			F.Point[1]=(i+1)*(c+1)+j;
			F.Point[2]=i*(c+1)+j+1;
			AddFace(F);
			F.Point[0]=i*(c+1)+j+1;
			F.Point[1]=(i+1)*(c+1)+j;
			F.Point[2]=(i+1)*(c+1)+j+1;
			AddFace(F);
		}
	}

	// TODO: incrementare il tutto!!!
	num_textures=1;						// TODO: Inizializzazione, di tutte le variabili, corretta? per un eventuale salvataggio del modello...
	Texture_FaceInterval[0]=0;
	Texture_FileName[0]=new char[2];
	strcpy(Texture_FileName[0],"");
	Texture_GLMap[0]=INVALID_TEXTURE;

	Texture_Point=new Array<typename Vector<2>>(num_p);
	Texture_Face=new Array<FaceType>(num_f);

	{
		Vector<2> P,P_row;
		Vector<2> step_x,step_y;

		step_x[0]=(1.0/c);
		step_x[1]=0.0;
		step_y[0]=0.0;
		step_y[1]=(1.0/r);
		
		P_row[0]=0.0;
		P_row[1]=0.0;

		for(int i=0;i<=r;i++) {
			P=P_row;
			for(int j=0;j<=c;j++) {
				Texture_Point->append(P);
				P=P+step_x;
			}
			P_row=P_row+step_y;
		}
	}

	for(int i=0;i<r;i++) {
		for(int j=0;j<c;j++) {
			F.Point[0]=i*(c+1)+j;
			F.Point[1]=(i+1)*(c+1)+j;
			F.Point[2]=i*(c+1)+j+1;
			Texture_Face->append(F);
			F.Point[0]=i*(c+1)+j+1;
			F.Point[1]=(i+1)*(c+1)+j;
			F.Point[2]=(i+1)*(c+1)+j+1;
			Texture_Face->append(F);
		}
	}

}



//***************************************************************************************
//***************************************************************************************
//***************************************************************************************


template <> 
Vector<2> HyperFace<2>::Normal(List<HyperFace<2>::PointType> *Points) {
	
	Vector<3> A,B,C;
	Vector<2> a;
	a=(*Points)[Point[0]]-(*Points)[Point[1]];
	A[0]=a[0];
	A[1]=a[1];
	B[0]=B[1]=0;
	B[2]=1;
	C=A^B;

	Vector<2> n;
	n[0]=C[0];
	n[1]=C[1];
	return n.Versore();
}

template <> 
Vector<3> HyperFace<3>::Normal(List<HyperFace<3>::PointType> *Points) {
	
	Vector<3> A,B,n;
	A=(*Points)[Point[1]]-(*Points)[Point[0]];
	B=(*Points)[Point[2]]-(*Points)[Point[0]];

	n=A^B;

	return n.Versore();
}

template <> 
double HyperFace<2>::ComputaArea(List<HyperFace<2>::PointType> *Points) {
	Area=((*Points)[Point[0]]-(*Points)[Point[1]]).Norm2();
	return Area;
}

template <> 
double HyperFace<3>::ComputaArea(List<HyperFace<3>::PointType> *Points) {
	// Heron's formula
	double a=((*Points)[Point[0]]-(*Points)[Point[1]]).Norm2();
	double b=((*Points)[Point[1]]-(*Points)[Point[2]]).Norm2();
	double c=((*Points)[Point[2]]-(*Points)[Point[0]]).Norm2();

	double s=0.5*(a+b+c);
	s=s*(s-a)*(s-b)*(s-c);
	if (s<=0.0) Area=0.0;		// Handle Irregular triangles
	else Area=sqrt(s);
	return Area;
}

template <int dim> 
Vector<dim> HyperFace<dim>::Baricentro(List<PointType> *Points) {
	Vector<dim> o;

	for(int i=0;i<dim;i++) o+=(*Points)[Point[i]];
	o*=(1.0/dim);
	
	return o;
}

template <int dim> 
int HyperFace<dim>::free_point3(int a,int b) {
	for(int i=0;i<dim;i++) {
		if (Point[i]==a) continue;
		if (Point[i]==b) continue;
		return i;
	}
	return -1;
}

template <int dim> 
int HyperFace<dim>::find_point(int p) {
	for(int i=0;i<dim;i++) {
		if (Point[i]==p) return i;
	}
	return -1;
}


HyperMesh<2> *Elipse(Vector<2> center,double rx,double ry,double angle_step) {
	
	int old_p,p,first,count;
	Vector<2> A;
	HyperFace<2> F;

	count=((int)(3.1415/angle_step))+1;

	HyperMesh<2> *Circle=new HyperMesh<2>(count,count,2);

	A[0]=rx;A[1]=0;
	A+=center;
	
	first=old_p=Circle->AddPoint(A);
	for(double a=angle_step;a<2*3.1415;a+=angle_step) {
		A[0]=rx*cos(a);
		A[1]=ry*sin(a);
		A+=center;

		p=Circle->AddPoint(A);
		F.Point[0]=old_p;
		F.Point[1]=p;

		Circle->AddFace(F);
		old_p=p;
	}	

	F.Point[0]=old_p;
	F.Point[1]=first;

	Circle->AddFace(F);

	return Circle;
}




















//***************************************************************************************
//***************************************************************************************
//***************************************************************************************

Element_Type Intersection_LineSegment(Vector<2> o,Vector<2> d,Vector<2> p1,Vector<2> p2) {
	Vector<2>    vd=(p2-p1);
	Element_Type n2=vd*vd;
	if (n2==0.0)  return Element_Type_Max;
	Vector<2>     v=GetOrthogonalVersor(vd*(1/sqrt(n2)));

	Element_Type parallel=(v*d);
	if (IS_ZERO(parallel,INTERSECTION_NUMERICAL_TOLLERANCE)) return Element_Type_Max;

	Element_Type t=((v*p1)-(v*o))/parallel;
	Vector<2> p=(t*d)+o;

	Element_Type l=(p-p1)*(vd)/n2;
	if ((l>=0.0) && (l<=1.0)) return t;
	return Element_Type_Max;
}

bool ScreenBorderIntersection(Vector<2> p1,Vector<2> p2,int x0,int y0,int x1,int y1,Vector<2> &border_point) {
	double    t;
	Vector<2> d=p2-p1;

	t=Intersection_LineSegment(p1,d,VECTOR(x0,y0),VECTOR(x1,y0));
	if ((t!=Element_Type_Max) && (t>=0)) {border_point=(p1+(t*d));return true;}
	t=Intersection_LineSegment(p1,d,VECTOR(x0,y0),VECTOR(x0,y1));
	if ((t!=Element_Type_Max) && (t>=0)) {border_point=(p1+(t*d));return true;}
	t=Intersection_LineSegment(p1,d,VECTOR(x1,y0),VECTOR(x1,y1));
	if ((t!=Element_Type_Max) && (t>=0)) {border_point=(p1+(t*d));return true;}
	t=Intersection_LineSegment(p1,d,VECTOR(x0,y1),VECTOR(x1,y1));
	if ((t!=Element_Type_Max) && (t>=0)) {border_point=(p1+(t*d));return true;}
	
	return false;
}

Element_Type Intersection_LineSegment(Vector<3> o,Vector<3> d,Vector<3> p1,Vector<3> p2) {
	Warning("Intersection_LineSegment: not tested");

// http://mathworld.wolfram.com/Line-LineIntersection.html

	Element_Type coplanarity=(p1-o)*((d-o)^(p2-p1));

	if (IS_ZERO(coplanarity,INTERSECTION_NUMERICAL_TOLLERANCE)) {
		Vector<3> ab=(d-o)^(p2-p1);
		Element_Type n2=(ab*ab);
		Element_Type l=-(((o-p1)^(d-o))*ab)/n2;
		if ((l>=0.0) && (l<=1.0)) {		
			Element_Type k=(((p1-o)^(p2-p1))*ab)/n2;
			return k;
		}
	}
	return Element_Type_Max;

}

Element_Type Intersection_LineTriangle(Vector<3> o,Vector<3> d,HyperMesh<3> *Mesh,int face) {

	Vector<3> V0=Mesh->Points[Mesh->Faces[face].Point[0]];
	Vector<3> V1=Mesh->Points[Mesh->Faces[face].Point[1]];
	Vector<3> V2=Mesh->Points[Mesh->Faces[face].Point[2]];

	Vector<3> U=V1-V0;
	Vector<3> V=V2-V0;
	Vector<3> n=U^V;
	if (IS_ZERO(n.Norm2(),INTERSECTION_NUMERICAL_TOLLERANCE)) {
		// the trinagle is a line
		Element_Type distance_o;
		if (U.Norm2()>V.Norm2()) distance_o=Intersection_LineSegment(o,d,V0,V1);
		else distance_o=Intersection_LineSegment(o,d,V0,V2);
		return distance_o;
	}

	n=n.Versore();

	Vector<3> PI;
	Element_Type distance_o=Intersection_LinePlane(o,d,n,V0,&PI);
	if (distance_o==Element_Type_Max) return Element_Type_Max;
	

    Element_Type uu,uv,vv,wu,wv,D;
    uu = U*U;
    uv = U*V;
    vv = V*V;
    Vector<3> w = PI - V0;
    wu = w*U;
    wv = w*V;
    D = uv * uv - uu * vv;
	

    Element_Type s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0) return Element_Type_Max;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0) return Element_Type_Max;

    return distance_o;
}


bool Intersection_TriangleTriangle_v1(HyperMesh<3> *Mesh1,int face1,HyperMesh<3> *Mesh2,int face2) {
	Element_Type dist;

	Vector<3> V0=Mesh1->Points[Mesh1->Faces[face1].Point[0]];
	Vector<3> V1=Mesh1->Points[Mesh1->Faces[face1].Point[1]];
	Vector<3> V2=Mesh1->Points[Mesh1->Faces[face1].Point[2]];

	Vector<3> U=V1-V0;
	Vector<3> V=V2-V0;
	Vector<3> W=V2-V1;

	// V0 - V1
	dist=Intersection_LineTriangle(V0,U,Mesh2,face2);
	if ((dist>=0.0) && (dist<=1.0)) return true;
	// V0 - V2
	dist=Intersection_LineTriangle(V0,V,Mesh2,face2);
	if ((dist>=0.0) && (dist<=1.0)) return true;
	// V1 - V2
	dist=Intersection_LineTriangle(V1,W,Mesh2,face2);
	if ((dist>=0.0) && (dist<=1.0)) return true;


	// Check the opposite 
	V0=Mesh2->Points[Mesh2->Faces[face2].Point[0]];
	V1=Mesh2->Points[Mesh2->Faces[face2].Point[1]];
	V2=Mesh2->Points[Mesh2->Faces[face2].Point[2]];

	U=V1-V0;
	V=V2-V0;
	W=V2-V1;

	// V0 - V1
	dist=Intersection_LineTriangle(V0,U,Mesh1,face1);
	if ((dist>=0.0) && (dist<=1.0)) return true;
	// V0 - V2
	dist=Intersection_LineTriangle(V0,V,Mesh1,face1);
	if ((dist>=0.0) && (dist<=1.0)) return true;
	// V1 - V2
	dist=Intersection_LineTriangle(V1,W,Mesh1,face1);
	if ((dist>=0.0) && (dist<=1.0)) return true;

	return false;
}

#include "Intersection.hpp"

bool Intersection_TriangleTriangle_v2(HyperMesh<3> *Mesh1,int face1,HyperMesh<3> *Mesh2,int face2) {
	Element_Type V0[3],V1[3],V2[3],U0[3],U1[3],U2[3];


	Mesh1->Points[Mesh1->Faces[face1].Point[0]].GetList(V0);
	Mesh1->Points[Mesh1->Faces[face1].Point[1]].GetList(V1);
	Mesh1->Points[Mesh1->Faces[face1].Point[2]].GetList(V2);

	Mesh2->Points[Mesh2->Faces[face2].Point[0]].GetList(U0);
	Mesh2->Points[Mesh2->Faces[face2].Point[1]].GetList(U1);
	Mesh2->Points[Mesh2->Faces[face2].Point[2]].GetList(U2);

	return NoDivTriTriIsect(V0,V1,V2,U0,U1,U2);
}

bool Intersection_Mesh(HyperMesh<3> *Mesh,int face1,int face2) {
	if (face1==face2) return false;
	
	int *f1_p=Mesh->Faces[face1].Point;
	int *f2_p=Mesh->Faces[face2].Point;


	// check face adiacency  
	//    avoid intersections between faces sharing one or more vertices
	bool is_neigh=false;
	for(int j=0;j<3;j++) {
		int tmp_index=f1_p[j];
		for(int q=0;q<3;q++) {
			if (tmp_index==f2_p[q]) {
				is_neigh=true;
				break;
			}
		}
	}
	if (is_neigh) return false;

	return Intersection_TriangleTriangle_v2(Mesh,face1,Mesh,face2);
}

bool Intersection_Mesh(HyperMesh<3> *Mesh1,HyperMesh<3> *Mesh2,int face1,int face2) {
	if ((Mesh1==Mesh2) && (face1==face2)) return false;
	
	int *f1_p=Mesh1->Faces[face1].Point;
	int *f2_p=Mesh2->Faces[face2].Point;


	// check face adiacency  
	//    avoid intersections between faces sharing one or more vertices
	bool is_neigh=false;
	for(int j=0;j<3;j++) {
		int tmp_index=f1_p[j];
		for(int q=0;q<3;q++) {
			if (tmp_index==f2_p[q]) {
				is_neigh=true;
				break;
			}
		}
	}
	if (is_neigh) return false;

	return Intersection_TriangleTriangle_v2(Mesh1,face1,Mesh2,face2);
}

Array<int> *Mesh_to_Mesh_Intersection(HyperMesh<3> *Mesh1,HyperMesh<3> *Mesh2) {
	Element_Type  V0[3],V1[3],V2[3],U0[3],U1[3],U2[3];
	Vector<3>    *p1=Mesh1->Points.getMem();
	Vector<3>    *p2=Mesh1->Points.getMem();
	HyperFace<3> *curr_face1=Mesh1->Faces.getMem();

	
	Array<int> *Mesh1_Intersecting_faces=new Array<int>(1000);


	for(int i=0;i<Mesh1->num_f;i++,curr_face1++) {
		
		HyperFace<3> *curr_face2=Mesh2->Faces.getMem();
		for(int j=0;j<Mesh2->num_f;j++,curr_face2++) {
			p1[curr_face1->Point[0]].GetList(V0);
			p1[curr_face1->Point[1]].GetList(V1);
			p1[curr_face1->Point[2]].GetList(V2);

			p2[curr_face2->Point[0]].GetList(U0);
			p2[curr_face2->Point[1]].GetList(U1);
			p2[curr_face2->Point[2]].GetList(U2);

			if (NoDivTriTriIsect(V0,V1,V2,U0,U1,U2)) {
				Mesh1_Intersecting_faces->append(i);
				break;
			}
		}
	}

	return Mesh1_Intersecting_faces;
}


//***************************************************************************************
//***************************************************************************************
//***************************************************************************************

template <int dim> 
Vector<dim> HyperBox<dim>::Center() {
	if (Rot==NULL) return ((P0+P1)*0.5);
	else return C2;
}

template <int dim> 
Vector<dim> HyperBox<dim>::getVertex(int index) {
	if (index<0) ErrorExit("This vertex doesn't exist");
	if (index>=(int)(pow(2.0,dim))) ErrorExit("This vertex doesn't exist");

	Vector<dim> out;
	for(int i=0;i<dim;i++) {
		if (index%2) out[i]=P1[i];
		else out[i]=P0[i];

		index=(index>>1);
	}
	
	return out;
}

template <int dim> 
void HyperBox<dim>::Set(Vector<dim> Center,double radius) {
	P0=P1=Center;
	
	for(int i=0;i<dim;i++) {
		P0[i]-=radius;
		P1[i]+=radius;
	}
}

template <int dim> 
void HyperBox<dim>::Include(HyperBox<dim> &x) {
	P0.Min(&(x.P0));
	P1.Max(&(x.P1));
}

template <int dim> 
void HyperBox<dim>::Set(Vector<dim> Center,Vector<dim> d) {
	P0=P1=Center;
	
	for(int i=0;i<dim;i++) {
		P0[i]-=d[i];
		P1[i]+=d[i];
	}
}

template <int dim> 
Vector<dim> HyperBox<dim>::d() {
	return (((P1-P0).abs())*0.5);
}

template <int dim> 
double HyperBox<dim>::GetRadius() {
	Vector<dim> x=d();
	double max=x[0];
	for(int i=1;i<dim;i++) 
		if (x[i]>max) max=x[i];
	return max;
}

template <int dim> 
void HyperBox<dim>::order() {
	for(int i=0;i<dim;i++) {
		if (P0[i]>P1[i]) {
			double tmp=P1[i];
			P1[i]=P0[i];
			P0[i]=tmp;
		}
	}
}

template <int dim> 
void HyperBox<dim>::scale(double factor) {
	Vector<dim> C=Center();
	
	P0=(P0-C)*factor+P0;
	P1=(P1-C)*factor+P1;
}



/*
template <int dim> 
void HyperMesh<dim>::DivideBigFaces2(double limite_area) {
	Vector<dim> Baricentro;

	// Creating new faces 
	int old_num_p=num_p;
	int old_num_f=num_f;
	List<PointType> old_Points(Points);
	List<FaceType> old_Faces(Faces);

	// Reset
	num_p=0;
	num_f=0;

	for(int i=0;i<old_num_p;i++) AddPoint(old_Points[i]);
	
	for(int i=0;i<old_num_f;i++) {
		if (old_Faces[i].Area>limite_area) {
			Vector<dim> e1,e2,e3;
			e1=(old_Points[old_Faces[i].Point[0]]+old_Points[old_Faces[i].Point[1]])*0.5;
			e2=(old_Points[old_Faces[i].Point[1]]+old_Points[old_Faces[i].Point[2]])*0.5;
			e3=(old_Points[old_Faces[i].Point[2]]+old_Points[old_Faces[i].Point[0]])*0.5;

			int i_e1,i_e2,i_e3;
			i_e1=AddPoint(e1);
			i_e2=AddPoint(e2);
			i_e3=AddPoint(e3);

			HyperFace<dim> N;
			N.Point[0]=old_Faces[i].Point[0];
			N.Point[1]=i_e1;
			N.Point[2]=i_e3;
			AddFace(N);
			N.Point[0]=i_e1;
			N.Point[1]=old_Faces[i].Point[1];
			N.Point[2]=i_e2;
			AddFace(N);
			N.Point[0]=i_e2;
			N.Point[1]=old_Faces[i].Point[2];
			N.Point[2]=i_e3;
			AddFace(N);
			N.Point[0]=i_e1;
			N.Point[1]=i_e2;
			N.Point[2]=i_e3;
			AddFace(N);

		} else AddFace(old_Faces[i]);
	}

}

template <int dim> 
void HyperMesh<dim>::DivideBigFaces1(double limite_area) {
	Vector<dim> Baricentro;

	// Creating new faces 
	int old_num_p=num_p;
	int old_num_f=num_f;
	List<PointType> old_Points(Points);
	List<FaceType> old_Faces(Faces);

	// Reset
	num_p=0;
	num_f=0;

	for(int i=0;i<old_num_p;i++) AddPoint(old_Points[i]);
	
	for(int i=0;i<old_num_f;i++) {
		if (old_Faces[i].Area>limite_area) {
			Baricentro=old_Faces[i].Baricentro(&old_Points);
			int num_bari=AddPoint(Baricentro);

			HyperFace<dim> N;
			N.Point[0]=old_Faces[i].Point[0];
			N.Point[1]=old_Faces[i].Point[1];
			N.Point[2]=num_bari;
			AddFace(N);
			N.Point[0]=old_Faces[i].Point[1];
			N.Point[1]=old_Faces[i].Point[2];
			N.Point[2]=num_bari;
			AddFace(N);
			N.Point[0]=old_Faces[i].Point[2];
			N.Point[1]=old_Faces[i].Point[0];
			N.Point[2]=num_bari;
			AddFace(N);

		} else AddFace(old_Faces[i]);
	}

}
*/


/*

template <int dim> 
void HyperMesh<dim>::ComputaNeighbours() {
	int num_n;
	int *tmp=new int[num_p];
	
	Neighbours=new int*[num_p];
	NumNeighbours=new int[num_p];

// NOTE:
// un vertice se nn ha almeno un vicino -> neig=NULL
//

	for(int i=0;i<num_p;i++) {
		// per ogni punto
		num_n=0;
		
		for(int j=0;j<num_f;j++) {
			// Controlla ogni faccia
			for(int k=0;k<dim;k++) {
				// Ogni vertice della faccia
				if (Faces[j].Point[k]==i) {
					// Coincide -> tutti i vertici della faccia son suoi vicini.. tranne se stesso
					for(int a=0;a<dim;a++) {
						int y=Faces[j].Point[a];
						
						if (y!=i) { // nn aggiungere se stesso
	
							// Aggiungilo alla lista Faces[j].Point[a]
							// Controlla se e' gia' stato aggiunto
							bool tt=true;
						
							for(int l=0;l<num_n;l++) {
								if (tmp[l]==y) tt=false;
							}
							if (tt) tmp[num_n++]=y;
						}
					}			
				}
			}
		}
		


		// Crea e copia in Neighbours
		if (num_n!=0) Neighbours[i]=new int[num_n];
		for(int l=0;l<num_n;l++) Neighbours[i][l]=tmp[l];
		NumNeighbours[i]=num_n;
	}

	delete tmp;

}

template <int dim> 
void HyperMesh<dim>::SubDivision(double limite,int Method) {

	bool divide;
	Array<Edge> Edges(200);

	printf("Subdivision start\n"); 

	for(int i=0;i<num_f;i++) {
		if (Method==AREA_SUBDIVISION_METHOD) divide=(Faces[i].Area>limite);
		else divide=(Biharmonics[i].Norm2()>limite);
		if (divide) {
			Vector<dim> e1,e2,e3;
			e1=(Points[Faces[i].Point[0]]+Points[Faces[i].Point[1]])*0.5;
			e2=(Points[Faces[i].Point[1]]+Points[Faces[i].Point[2]])*0.5;
			e3=(Points[Faces[i].Point[2]]+Points[Faces[i].Point[0]])*0.5;

			int i_e1,i_e2,i_e3;
			i_e1=AddPoint(e1);
			i_e2=AddPoint(e2);
			i_e3=AddPoint(e3);
			
			Edge t;
			t.V1=Faces[i].Point[0];
			t.V2=Faces[i].Point[1];
			t.center=i_e1;
			Edges.append(t);
			t.V1=Faces[i].Point[1];
			t.V2=Faces[i].Point[2];
			t.center=i_e2;
			Edges.append(t);
			t.V1=Faces[i].Point[2];
			t.V2=Faces[i].Point[0];
			t.center=i_e3;
			Edges.append(t);
		}
	}

	// Creating new faces 
	int old_num_p=num_p;
	int old_num_f=num_f;
	List<PointType> old_Points(Points);
	List<FaceType> old_Faces(Faces);

	// Reset
	num_p=0;
	num_f=0;

	for(int i=0;i<old_num_p;i++) AddPoint(old_Points[i]);
	
	for(int i=0;i<old_num_f;i++) {
		int i1=SearchEdge(&Edges,old_Faces[i].Point[0],old_Faces[i].Point[1]);
		int i2=SearchEdge(&Edges,old_Faces[i].Point[1],old_Faces[i].Point[2]);
		int i3=SearchEdge(&Edges,old_Faces[i].Point[2],old_Faces[i].Point[0]);

		// Casi:
		int s=((i1==-1)?0:1)+((i2==-1)?0:1)+((i3==-1)?0:1);
		if (s==0) AddFace(old_Faces[i]);
		if (s==3) {
			HyperFace<dim> N;
			N.Point[0]=old_Faces[i].Point[0];
			N.Point[1]=Edges[i1].center;
			N.Point[2]=Edges[i3].center;
			AddFace(N);
			N.Point[0]=Edges[i1].center;
			N.Point[1]=old_Faces[i].Point[1];
			N.Point[2]=Edges[i2].center;
			AddFace(N);
			N.Point[0]=Edges[i2].center;
			N.Point[1]=old_Faces[i].Point[2];
			N.Point[2]=Edges[i3].center;
			AddFace(N);
			N.Point[0]=Edges[i1].center;
			N.Point[1]=Edges[i2].center;
			N.Point[2]=Edges[i3].center;
			AddFace(N);
		}
		if (s==1) {
			HyperFace<dim> N;
			if (i1!=-1) {
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=Edges[i1].center;
				N.Point[2]=old_Faces[i].Point[2];
				AddFace(N);
				N.Point[0]=old_Faces[i].Point[1];
				N.Point[1]=old_Faces[i].Point[2];
				N.Point[2]=Edges[i1].center;
				AddFace(N);
			}
			if (i2!=-1) {
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=old_Faces[i].Point[1];
				N.Point[2]=Edges[i2].center;
				AddFace(N);
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=Edges[i2].center;
				N.Point[2]=old_Faces[i].Point[2];
				AddFace(N);
			}
			if (i3!=-1) {
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=old_Faces[i].Point[1];
				N.Point[2]=Edges[i3].center;
				AddFace(N);
				N.Point[0]=old_Faces[i].Point[1];
				N.Point[1]=old_Faces[i].Point[2];
				N.Point[2]=Edges[i3].center;
				AddFace(N);
			}
		}
		if (s==2) {
			HyperFace<dim> N;
			if (i1!=-1) {
				if (i2!=-1) {
					N.Point[0]=old_Faces[i].Point[0];
					N.Point[1]=Edges[i1].center;
					N.Point[2]=old_Faces[i].Point[2];
					AddFace(N);
					N.Point[0]=old_Faces[i].Point[1];
					N.Point[1]=Edges[i2].center;
					N.Point[2]=Edges[i1].center;
					AddFace(N);
					N.Point[0]=Edges[i1].center;
					N.Point[1]=Edges[i2].center;
					N.Point[2]=old_Faces[i].Point[2];
					AddFace(N);
				}
				if (i3!=-1) {
					N.Point[0]=old_Faces[i].Point[1];
					N.Point[1]=old_Faces[i].Point[2];
					N.Point[2]=Edges[i1].center;
					AddFace(N);
					N.Point[0]=old_Faces[i].Point[0];
					N.Point[1]=Edges[i1].center;
					N.Point[2]=Edges[i3].center;
					AddFace(N);
					N.Point[0]=Edges[i1].center;
					N.Point[1]=old_Faces[i].Point[2];
					N.Point[2]=Edges[i3].center;
					AddFace(N);
				}			
			}		
			if ((i2!=-1) && (i3!=-1)) {
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=old_Faces[i].Point[1];
				N.Point[2]=Edges[i2].center;
				AddFace(N);
				N.Point[0]=old_Faces[i].Point[0];
				N.Point[1]=Edges[i2].center;
				N.Point[2]=Edges[i3].center;
				AddFace(N);
				N.Point[0]=Edges[i3].center;
				N.Point[1]=Edges[i2].center;
				N.Point[2]=old_Faces[i].Point[2];
				AddFace(N);
			}
		} // if
	} // for 

	printf("Subdivision finished.\n"); 

}
*/
