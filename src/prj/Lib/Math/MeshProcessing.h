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





class HyperMeshPartitionTree : public Tree {
	static bool Intersection_BinaryLeveledTree               (HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces);
	static bool Intersection_BinaryLeveledTree_Skinned_Mesh  (HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces,bool *intersection_mask);
	static bool Intersection_BinaryTree_Skinned_Mesh         (HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces,bool *intersection_mask);
	static HyperMeshPartitionTree *Generate_rec(HyperMesh<3> *mesh,Array<int> *face_index_list,int level);
public:
	HyperBox<3> BoundingBox;
	Array<int> *FaceList;
	HyperMesh<3> *mesh;



	HyperMeshPartitionTree(int num_child,HyperMesh<3> *mesh);
	virtual ~HyperMeshPartitionTree();




	//*************************************************************************************************************
	// Generate a binary partitioning tree 
	//*************************************************************************************************************
	static HyperMeshPartitionTree *Generate(HyperMesh<3> *mesh,int levels);												// HyperMesh and Skinned_Mesh
	
	static HyperMeshPartitionTree *Generate(HyperMesh<3> *mesh,															// only Skinned_Mesh
											int start_level_range,int end_level_range,									//     - finds the fastest solution for the mesh in that configuration
											bool *intersection_mask,float &fps,int &selected_level);					//     - fps is returned as the number of collision test per sec
																												



	
	//*************************************************************************************************************
	// Compute collisions
	//    - call from root
	//*************************************************************************************************************
	void UpdateBoundingBoxes();
	// Generic trees
	bool SelfIntersection(Array<ordered_pair> *intersecting_faces);
	// Binary trees of skinned meshes
	bool SelfIntersection_BinaryTree_Skinned_Mesh(Array<ordered_pair> *intersecting_faces,bool *intersection_mask);     // very well tested: restituisce solo coppie (i,j) unique -> elimina a priori le doppie e le coppie (i,j) - (j,i)
	// Binary trees with branches all of the same depth
	bool SelfIntersection_BinaryLeveledTree(Array<ordered_pair> *intersecting_faces);
	// Binary trees of skinned meshes with branches all of the same depth
	bool SelfIntersection_BinaryLeveledTree_Skinned_Mesh(Array<ordered_pair> *intersecting_faces,bool *intersection_mask);



	// Generic trees
	static bool Intersection (HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces);

};










