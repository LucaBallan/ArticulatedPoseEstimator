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





#define MAX_LEVELS 90

#define DIM_X 0
#define DIM_Y 1
#define DIM_Z 2
#define DIM_G 3
#define MOVE_DIR_POS 1
#define MOVE_DIR_NEG 0



//***************************************************************************************
//***************************** OcTree **************************************************
//***************************************************************************************


template <int dim> 
class OcTree_Node: public Tree {
	virtual OcTree_Node<dim> *CreateNode(OcTree_Node<dim> *Father,int type);
	virtual bool Load(char *filename);

public:
	Vector<dim> Center;
	Vector<dim> d;




	OcTree_Node(OcTree_Node *Father);
	OcTree_Node(Vector<dim> d);						// d is an half-hedge -> creato da -d to d
	OcTree_Node(char *filename);
	virtual ~OcTree_Node();




	OcTree_Node *NodeFromPoint(Vector<dim> x,Vector<dim> *Errore=NULL);
	OcTree_Node *CreateNodeFromPoint(Vector<dim> x,int Level);
	OcTree_Node *GetNeighbour(int move_dim,int move_dir);

	OcTree_Node *Get_Next_Enum_Leaf();			// La prima chiamata deve essere root, non sono ammessi sottoalberi

	void Save(char *filename);			        // Deve essere root, non salva i sottoalberi


};



class Voxel3D: public OcTree_Node<3> {
	virtual Voxel3D *CreateNode(Voxel3D *Father,int type);
	virtual bool Load(char *filename);

public:
	bool empty;
	float v[8];

	Voxel3D(Vector<3> d) : OcTree_Node<3>(d) {empty=true;}
	Voxel3D(Voxel3D *Father) : OcTree_Node<3>(Father) {empty=true;}

	Vector<3> GetVertexCoords(int type);
};



