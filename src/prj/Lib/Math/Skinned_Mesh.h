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





struct Degrees_of_Freedom_Table_Limits_Entry {
	float MinX,MaxX;
	float MinY,MaxY;
	float MinZ,MaxZ;
};

class Skinned_Mesh;

struct Degrees_of_Freedom_Table_Entry {
	char *Name;
	bool Translable_X;
	bool Translable_Y;
	bool Translable_Z;
	// Note: Le rotazioni sono riferite al sistema di Riferimento del Padre!!!
	bool Rotable_X;
	bool Rotable_Y;
	bool Rotable_Z;
	Degrees_of_Freedom_Table_Limits_Entry LimitsT;
	Degrees_of_Freedom_Table_Limits_Entry LimitsR;
	
	void set_block(bool set_zero_all_limits=true);

	static Degrees_of_Freedom_Table_Entry *SearchDoFT(const char *Name,Degrees_of_Freedom_Table_Entry *Table);			// Search for the table entry "<prefix>Name" (usefull for limits)
	static void Save(char *FileName,Degrees_of_Freedom_Table_Entry *Table);
	static Degrees_of_Freedom_Table_Entry *Load(char *FileName,Skinned_Mesh *Model,int frame__ref);
	static Degrees_of_Freedom_Table_Entry *Copy(Degrees_of_Freedom_Table_Entry *Table);
};



class Skinned_Mesh: public HyperMesh<3> {
	Array<PointType> *Pose_Points;
public:
	int num_bones;
	Bone *BoneRoot;
	Bone **BoneRefTable;
	double **Weigth;




	//
	// Costruttori
	//
	Skinned_Mesh();
	Skinned_Mesh(Skinned_Mesh &original);
	Skinned_Mesh(Skinned_Mesh *original);								// Funziona perfettamente
	virtual ~Skinned_Mesh();




	void Read(char *BaseFileName);
	void Save(char *BaseFileName);										// Save: - absolute matrices must be ready
																		//       - current skin resetted to base pose

	void AddMesh(Skinned_Mesh *input,int force_n_frames=0);				// todo** sembra funzionare  solo force_n_frames to test


	



	//
	// Skin
	//
	void Apply_Skin(int FrameIndex);														// FrameIndex = 0 1 ..... getNumberOfFrames-1
																							//              0 = Pose Frame   (not editable)
	double **transfer_skin(HyperMesh<3> *model);



	//
	// Information
	//
	int getNumberOfFrames();
	void GetCurrentBoneState(int frame_index,double *values);								// GetCurrentBoneState: Rel Matrices must be Ready



	//
	// Utility IK
	//
	void      SetConstraints(Degrees_of_Freedom_Table_Entry *Table);
	void      ApplyConstraints(int frame_index);
	int       GetBoneID(Bone *bone);
	int       GetBoneID(char *Name);
	Vector<3> GetPointRelativeCoordinates(int point_index,int bone_index);
	int       GetBoneIDAttachedtoVertex(int point_index,double att_th=0.8);					// -1  if it is attached less than the thereshold
																							//  i  bone_index
																							// if att_th == 0.0 -> -1 solo quando non è attacato a nessuno
	




	//
	// Transformation affect Pose
	//
	void MovePose(Vector<3> T);
	void ScalePose(double factor);
	void RotatePose(Vector<3> Axis);
	void Scale(double factor,int frame_index);
	double *CreateConstantEnlargeFactors(double value);
	void CustomEnlargeFactors(double *EnlargeFactors,char *BoneName,double factor);
	void Enlarge(double *EnlargeFactors);													// Ingrandisce seguendo i bones
																							// NOTE: TODO Non considera il fatto che il bone sia un segmento e che quindi la direzione
																							//       di enlarge non è sempre perpendicolare al suo asse.




	//
	// Transformation affect Motion 
	//
	void Move(Vector<3> T,int frame_index);
	void Rotate(Vector<3> angle_axis,int frame_index);
};







//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intersezioni
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Intersection_Mesh(Skinned_Mesh *Mesh,int face1,int face2,bool *intersection_mask);						// Self Intersection of a skinned mesh

bool *LoadIntersectionMask(Skinned_Mesh *Mesh,char *IntersectionMask_filename);								// IntersectionMask_filename can be NULL


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////