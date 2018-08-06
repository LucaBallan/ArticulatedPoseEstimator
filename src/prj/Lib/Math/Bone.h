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





#define BONE_MAX_CHILD 8
#define BONE_MAX_NAME_LEN 70
#define DEFAULT_NUMBER_OF_FRAMES 2
//#define DRAW_REF_SYSTEM	
#define CHECK_CONSTRAINT

#define BONE_DEF_LIMITS FLT_MAX
#define BONE_DMRA  180
#define LIMITS_DEF {-BONE_DMRA,BONE_DMRA,-BONE_DMRA,BONE_DMRA,-BONE_DMRA,BONE_DMRA}
#define LIMITS_DEF_FREE {-BONE_DEF_LIMITS,BONE_DEF_LIMITS,-BONE_DEF_LIMITS,BONE_DEF_LIMITS,-BONE_DEF_LIMITS,BONE_DEF_LIMITS}
#define LIMITS_DEF_T {-FLT_MAX,FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX,FLT_MAX}


class Skinned_Mesh;
class IKSolver;

class Bone : public Tree {
	friend class Skinned_Mesh;
	friend class IKSolver;
	static bool Save_Fnc(Tree *x,void *In,void *Out);
	static bool CalcRel_Fnc(Tree *x,void *In,void *Out);
	static bool CalcAbs_Fnc(Tree *x,void *In,void *Out);
	static bool DrawBone_Fnc(Tree *x,void *In,void *Out);
	static bool SearchBone_Fnc(Tree *x,void *In,void *Out);
	static bool CopyAbs_Fnc(Tree *x,void *In,void *Out);
	static bool PrintRel_Fnc(Tree *x,void *In,void *Out);
	static bool AddPrefix_Fnc(Tree *x,void *In,void *Out);
	static bool SubStPrefix_Fnc(Tree *x,void *In,void *Out);
	static bool RemovePostfix_Fnc(Tree *x,void *In,void *Out);
	static bool PrintTree_Fnc(Tree *x,void *In,void *Out);
	static bool Filter_Fnc(Tree *x,void *In,void *Out);
	static bool Orthonormalize_Fnc(Tree *x,void *In,void *Out);


	// Static properties
	double length;
	char Name[BONE_MAX_NAME_LEN+1];
	int Number_Of_Frames;


	// Dynamic properties
	Vector<3> *TrasformationT;
	Matrix **TrasformationMatrix;						// 3x3 matrix which contains the vectors of the base system of the bone. 
														// TrasformationMatrix(:,0) is the direction of the bone

	// Relative Dynamic properties
	Matrix **RelativeTrasformationMatrix;
	Vector<3> *RelativeTrasformationT;					// Note: the origin is RelativeTrasformationT + father->End_Bone


public:
	// Static
	static void InitBoneMesh();
	static Bone *LoadSkull(IFileBuffer *In,int set_num_frames=0,int *old_num_frames=NULL);
	static void SaveSkull(OFileBuffer *Out,Bone *BoneRoot);
	static void LoadSkullFrame(IFileBuffer *In,Bone *Root,int FrameIndex);
	static void LoadSkullMotion(IFileBuffer *In,Bone *Root,int local_num_of_frames=0);
	static void LoadSkullPose(IFileBuffer *In,Bone *Root);
	static void SaveSkullMotion(OFileBuffer *Out,Bone *Root);
	static void SaveSkullMotion_Constant(OFileBuffer *Out,Bone *Root,int num_frames);	// Salva un movimento statico di num_frames tutti uguali al frame 1
	int getNumberOfFrames();

	
	// Cosructors
	Bone(char *Name,Bone *Father,double length,int Number_Of_Frames=DEFAULT_NUMBER_OF_FRAMES);
	Bone(Bone &original);
	Bone(Bone *original);											// copy only static properties, no hyerarchy
	virtual ~Bone();

	// Cloning
	static Bone *CloneBoneTree(Bone *original,Bone *Father=NULL);	// clone the tree where original is the father. Place as father of the root Father.
	static Bone *CloneBone(Bone *original,int number_of_frames);	// clone only static properties, father=NULL, no hyerarchy, force new number of frames
	static Bone *CloneBoneTree(Bone *original,						// clone the tree where original is the father. Place as father of the root Father, force new number of frames
							   Bone *Father,int number_of_frames);

	
	// Low level
	Matrix     **GetABSRotationArray();
	Vector<3>   *GetABSTranslationArray();
	Matrix     **GetRELRotationArray();
	Vector<3>   *GetRELTranslationArray();


	// Utility
	bool   is(char *Name);
	bool   isEndBone();
	double GetLength();
	const char *GetName() {return Name;};
	Array<Bone*> *Enumerate();																// to delete the returned array -> x=Enumerate(); ...; delete x;
	void GetABSTrasformMatrix(int FrameIndex,Matrix *Out);									// Chain Rule: ABSk = M1*M2*...*Mk   (where Mi is father of M(i+1))
	void GetRelTrasformMatrix(int FrameIndex,Matrix *Out);
	void GetPivotAndDirection(int FrameIndex,Vector<3> *Pivot,Vector<3> *Dir);
	void GetEndPoint(int FrameIndex,Vector<3> *End);
	void GetIKChain(Array<Bone*> *Chain);
	void GetRelativeAngleAndTranslation(int FrameIndex,Vector<3> *R,Vector<3> *T);

	// Applied to SubTree 
	Bone *SearchBone(const char *Name);
	void CalcRelativeFromAbsolute(int FrameIndex);
	void CalcAbsoluteFromRelative(int FrameIndex);
	void PrintRelativeInformation(int FrameIndex);
	void PrintTree();
	void AddPrefix(char *prefix);
	void SubStPrefix(char *prefix);
	void RemovePostfix(char *postfix);
	void Copy(int src_frame,int dst_frame);													// copy assolute and compute relative
	void DrawAll(int FrameIndex,RenderWindow<3> *x);
	


	// IK
	bool LockX;
	bool LockY;
	bool LockZ;
	bool LockRX;
	bool LockRY;
	bool LockRZ;
	bool AllowPropagation_;
	bool CanTranslate();
	bool CanRotate();
	bool CanRotateOrTranslate();
	bool AllowPropagation();																// Allow this bone to influence the parent bone if it reach its constraints (in an IK procedure)

	// IK contraints 
	Vector<3> MinR,MaxR;																	// alpha, beta, gamma limits
	Vector<3> MinT,MaxT;

	
	// EDIT:
	//  - Frame 0 is not editable
	//
	//  - ROTOTRANSLATION
	//     Applied only to Relative (Lavorano solo sulle relative, quest'ultime devono essere esatte) 
	//     Use CalcAbsoluteFromRelative to compute the absolute matrices
	void Rotate(int FrameIndex,Vector<3> angle_axis);
	void Translate(int FrameIndex,Vector<3> T);
	
	void Rotate(int FrameIndex,Matrix *M);
	bool Rotate(int FrameIndex,Vector<3> angle_axis,Vector<3> EffectiveRotation);		// angle_axis refers (as always) to the father's reference system
																						// it returns true if the constrain has been activated 
	
	void ApplyConstraints(int FrameIndex);	

	

	// Relative must be setted on both FrameIndex and RefFrameIndex
	Vector<3> GetJointPosition(int FrameIndex);											// absolute must be ready
	void RelativeMotion(int FrameIndex,int RefFrameIndex,Vector<3> *T,Vector<3> *R);
	void Filter(int FrameIndex,int Filter_Extension=1);
	void Orthonormalize(int FrameIndex);


	//obsolete
	//void RotoTranslate(int FrameIndex,Vector<3> M,Vector<3> T,bool IsRelative=false);			// I vincoli sono relativi al frame 0
};


OFileBuffer &operator<<(OFileBuffer &os,Bone *b);
IFileBuffer &operator>>(IFileBuffer &os,Bone *b);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Bone *b);






