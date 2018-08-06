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





#define INITIAL_POINT_NUMBER 1000
#define INITIAL_FACE_NUMBER 1000
#define INITIAL_NUM_NEIGHBOURS 5
#define MAX_TEXTURES 50

#define INVALID_TEXTURE               0
#define INVALID_MESH_PROPERTY_VALUE   (-DBL_MAX)

#define AREA_SUBDIVISION_METHOD       1
#define CURVATURE_SUBDIVISION_METHOD  2
#define HALF_SUBDIVISION_METHOD	      3
#define FACE_LIST_SUBDIVISION_METHOD  4


template <int dim> 
class HyperFace {
public:
	typedef Vector<dim> PointType;

	int Point[dim];
	double Area;

	Vector<dim> Normal(List<PointType> *Points);

	double ComputaArea(List<PointType> *Points);
	Vector<dim> Baricentro(List<PointType> *Points);

	int find_point(int p);					// ritorna l'index del point cercato
	int free_point3(int a,int b);			// ritorna il primo index del point che nn e' ne a ne b -> -1 se nn c'e'
};



template <int dim> 
class HyperBox {
public:
	Vector<dim> P0;
	Vector<dim> P1;
	Matrix *Rot;
	Vector<dim> C2;

	Vector<dim> Center();
	Vector<dim> d();                // ritorna i lati/2 nelle varie dimensioni: Estensione centro[i]+-d[i]
									// definisce i raggi dell'elissoide che lo contiene
									// la sfera che lo contiene ha raggio d().Norm2;
	double GetRadius();
	void order();					// make P0 the left-most and P1 the right-most (P0 e' il piu' piccolo)
	void scale(double factor);


	HyperBox() : Rot(NULL) {};
	HyperBox(const HyperBox<dim> &x);
	const HyperBox<dim> &operator =(const HyperBox<dim> &x);
	~HyperBox();

	Vector<dim> getVertex(int index);

	void Set(Vector<dim> Center,double radius);
	void Set(Vector<dim> Center,Vector<dim> d);

	bool Intersects(const HyperBox<dim> &x) const {
		return  (P0[0] < x.P1[0]) && (P1[0] > x.P0[0]) &&
				(P0[1] < x.P1[1]) && (P1[1] > x.P0[1]) &&
				(P0[2] < x.P1[2]) && (P1[2] > x.P0[2]);
	}
	void Include(HyperBox<dim> &x);
};


class Coupled_Face {				// Coppia di facce che condividono un edge
public:
	int free1;						// sono i vertici liberi dalla condivisione
	int free2;
};

class Edge {
public:
	int V1,V2;
	int center;
};



template <int dim> 
class HyperMesh {
	void AddNeighbour(int point,int Neighbour,int face);
	int  AddEdge(Array<Edge> *Edges,int i,int j);
	int  AddEdgeSorted(Array<Edge> *Edges,int i,int j);

	void ReadText(char *filename);
	void ReadBinary(char *filename);
	void ReadSMF(char *filename);
	void SaveText(char *filename);
	void SaveBinary(char *filename);
	void SaveVRML(char *filename);
	void SaveSMF(char *filename);
	void SaveOBJ(char *filename);
	void ReadOBJ(char *filename);

	bool update_neighbours_structure;
	void ResetVolatileProperties();
public:
	typedef Vector<dim> PointType;
	typedef HyperFace<dim> FaceType;




	// 3D structure
	int num_p;
	int num_f;
	List<PointType>				Points;
	List<FaceType>				Faces;

	

	// UV/UVW Maps
	Array<typename Vector<2>>   *Texture_Point;
	Array<FaceType>			    *Texture_Face;
	int							 Texture_FaceInterval[MAX_TEXTURES];                 // Texture_FaceInterval[i] = first face to have Texture_GLMap[i]
	int						 	 num_textures;                                       // Texture_FaceInterval[0] = 0 always
	Array<typename Vector<3>>   *TextureUVW;



	// Texture
	UINT					 	 Texture_GLMap[MAX_TEXTURES];						 // the destructor do not delete them
	Bitmap<ColorRGBA>		 	*Texture_Map  [MAX_TEXTURES];						 // the destructor do not delete them
	char					    *Texture_FileName[MAX_TEXTURES];
	


	// Derivates
	DoubleList<int>				Neighbours;				// [i] = {...} lista di vertici che condividono un edge con il vertice i. se o appartiene alla lista allora (o,i) e' un edge della struttura
	List<int>					NumNeighbours;			// [i] = cardinalita' della lista qui sopra
	DoubleList<Coupled_Face>	Edge_Faces;			    // Ogni neighbours determina un edge: qui sono contenute le facce a cui questo edge appartiene
													    // Edge_Faces[i][j] = faccia a cui appartiene l'edge (i,Neighbours[i][j])

	DoubleList<int>				NeighbourFaces;			// [i] = {...} lista delle facce che usano il vertice i
	List<int>					NumNeighbourFaces;		// [i] = cardinalita' della lista qui sopra


	
	// Cache
	List<double>				AreaPoint;
	List<PointType>				Normals;
	List<PointType>				Laplacians;
	List<PointType>				Biharmonics;
	List<PointType>				CurvatureNormals;
	

	
	// Mesh Properties (Volatile)
	double Total_Area;
	double Average_Area;
	double Average_Stress;
	double Min_Stress;
	double Average_Curvature;
	double Average_Curvature_Variation;
	double Average_Mean_Curvature;
	double Max_Curvature;
	double Max_Curvature_Variation;
	double Max_Mean_Curvature;




	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////


	// Init	
	HyperMesh();
	HyperMesh(int initial_point_number,int initial_face_number,int initial_neighbours_number);
	HyperMesh(HyperMesh<dim> &original);
	HyperMesh(HyperMesh<dim> *original);					// operatore di copia (testato molto accuratamente, funziona)
															// Cache & Mesh Properties are not updated in the new mesh
	virtual ~HyperMesh();

	
	void SetUpdateNeighboursStructure(bool update_neighbours_structure=true);		// warning: false is dangerous -> non of NeighboursStructure arrays is valid!!!
  


	// Modifica Struttura
	int  AddPoint(Vector<dim> P);
	int  AddFace (HyperFace<dim> F);
	void AddMesh (HyperMesh<dim> *input);					// Cache & Mesh Properties are not updated in the new mesh (CHECKED)
															// UVW map is not created 

    
	// Importa/Esporta					(NB: No multiple read are allowed)
	void Read(char *filename);
	void Save(char *filename);


    // Crea oggetti
	void create_plane(Vector<2> TL,Vector<2> BR,int r,int c);
	int PlaneVertexIndex(Vector<2> TL,Vector<2> BR,int r,int c,Vector<3> p);
	Vector<3> PlaneVertexFromIndex(Vector<2> TL,Vector<2> BR,int r,int c,int index);


	// Obj manipulation
	void Move(Vector<dim> T);
	void Scale(double factor);
	void Rotate(Vector<dim> angle_axis,Vector<dim> center);



	// Update Cache 
	void				ComputaNormals();
	double				ComputaArea();
	Vector<dim>			Laplacian(int i);						// Laplacian/Umbrella operator sul punto i (needs neighbours_structure)
	void				ComputaLaplacians();					//										   (needs neighbours_structure)
	Vector<dim>			Biharmonic(int i);						// Biharmonic operator sul punto i         (needs ComputaLaplacians)
	void				ComputaBiharmonics();					//                                         (needs ComputaLaplacians)
	Vector<dim>			CurvatureNormal(int i);					//										   (needs ComputaArea)
	void				ComputaCurvatureNormal();				//										   (needs ComputaArea)

	

	
	// UV map
	void CreateUVWfromUV();
	void NormalizeUVCoords();				// Normalize UV coord between 0 and 1



	// Global properties
	double AverageArea();
	double AverageStress();					// Lunghezza media degli edge
	double MinStress();						// Lunghezza minima degli edge
	double AverageCurvature();				// derivata prima media
											// Necessita di ComputaLaplacians
	double AverageCurvatureVariation();		// derivata seconda media
											// Necessita di ComputaBiharmonic
	double AverageMeanCurvature();			// Necessita di ComputaCurvatureNormal
	double MaxCurvature();					// derivata prima massima
											// Necessita di ComputaLaplacians
	double MaxCurvatureVariation();		    // derivata seconda massima
											// Necessita di ComputaBiharmonic
	double MaxMeanCurvature();				// Necessita di ComputaCurvatureNormal
	double Qequ();							// Necessita di computa area
	double Qnor();							


	// aggiunte
	int FaceNeighbour(int edge,int face);
	int Nearest_from_axis(Vector<dim> O,Vector<dim> D);



	// Refinement
	void Simplify(int to_num_faces);
	void Optimize(double limite_area);
	void SubDivision(double limite,int Method=AREA_SUBDIVISION_METHOD,Array<int> *face_list=NULL);				// (needs ComputaArea,ComputaCurvatureNormal) 
														




	// Funzioni su dati gia' computati
	int  PlaneSmallFaces(double limite_area);			// necessita di ComputaArea();
	void RegolarizeFaceNormals();						// necessita di ComputaNormals();
	void CloseSmallHoles(int &closed,int &multipleholes,int &bounduaryedge); // necessita di ComputaNormals();
	void DeleteBounduaryEdges();
	void RemoveZeroEdgedFaces();
	//void RemoveNotIndexedVertices();

	Vector<dim> GetCenter();
	double GetRadius(Vector<dim> center);
	Vector<dim> GetPrincipalAxes(Matrix *x,bool use_baricenter=true);
														// Ritorna una matrice di rotazione dove in colonna vi sono gli assi principali
														// di inerzia. In particolare, sono ordinati in ordine decrescente per importanza.
														// I.E. La prima colonna indica il primo asse di inerzia.
														// It returns the center of the bounding box (parallel to canonical axis!!!) containing the object
														// use_baricenter=true  -> the center is the baricenter
														// use_baricenter=false -> the center is the bounding box center


	HyperBox<dim> BoundingBox();
	HyperBox<dim> BoundingBox(Array<int> *face_index_list);
	HyperBox<dim> RotatedBoundingBox(bool use_baricenter=true);

	Array<int> *Get_Apparent_Contour(Vector<dim> Point_of_view);   // D
	//void Draw(World<dim> *w);
	//void ComputaNeighbours();


	
//	double PointFaceDistance(Vector<dim> P,int f);
//	double PointDistanceSemplified(Vector<dim> P);

};




HyperMesh<2> *Elipse(Vector<2> center,double rx,double ry,double angle_step);










//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intersezioni
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Element_Type Intersection_LineSegment(Vector<2> o,Vector<2> d,Vector<2> p1,Vector<2> p2);								//              (double)        (signed distance of the founded intersection point from o, Element_Type_Max if no intersection)
Element_Type Intersection_LineSegment(Vector<3> o,Vector<3> d,Vector<3> p1,Vector<3> p2);								//              (double)        (signed distance of the founded intersection point from o, Element_Type_Max if no intersection)

Element_Type Intersection_LineTriangle(Vector<3> o,Vector<3> d,HyperMesh<3> *Mesh,int face);							// Intersection between
																														//       line   (o,d)           (passante per o e di direzione d)
																														//       face   (Mesh,face)		(face index face)
																														//              (double)        (signed distance (with respect to d) of the founded intersection point from o, Element_Type_Max if no intersection)

bool Intersection_TriangleTriangle_v1(HyperMesh<3> *Mesh1,int face1,HyperMesh<3> *Mesh2,int face2);						// compare each face with the edges of the other faces
																														// no exceptions
bool Intersection_TriangleTriangle_v2(HyperMesh<3> *Mesh1,int face1,HyperMesh<3> *Mesh2,int face2);						// "A Fast Triangle-Triangle Intersection Test"
																														// no exceptions

bool Intersection_Mesh(HyperMesh<3> *Mesh,int face1,int face2);															// Self Intersection of a mesh
bool Intersection_Mesh(HyperMesh<3> *Mesh1,HyperMesh<3> *Mesh2,int face1,int face2);									// Intersection of two faces of two meshes

Array<int> *Mesh_to_Mesh_Intersection(HyperMesh<3> *Mesh1,HyperMesh<3> *Mesh2);                                         // Mesh to Mesh Intersection
                                                                                                                        // Return the indexes of the faces in mesh1 intersecting with mesh2
                                                                                                                        // (the array must be deleted by the user)


bool ScreenBorderIntersection(Vector<2> p1,Vector<2> p2,int x0,int y0,int x1,int y1,Vector<2> &border_point);			// p1 point inside the screen, p2 point outside the screen
																														// true if intersection is found, border_point is fill with the intersection point



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



