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

using namespace std;
#pragma warning(3:4244)

HyperMesh<3> *BoneMesh=NULL;
int link_incremental_index=0;



Bone::Bone(char *Name,Bone *Father,double length,int Number_Of_Frames) : Tree(BONE_MAX_CHILD,Father) {
	if (strlen(Name)!=0) strcpy(this->Name,Name);
	else sprintf(this->Name,"Link%i",link_incremental_index++);
	this->length=length;
	
	this->Number_Of_Frames=Number_Of_Frames;
	TrasformationT=new Vector<3>[Number_Of_Frames];
	RelativeTrasformationT=new Vector<3>[Number_Of_Frames];
	TrasformationMatrix=new Matrix*[Number_Of_Frames];
	RelativeTrasformationMatrix=new Matrix*[Number_Of_Frames];
	for(int i=0;i<Number_Of_Frames;i++) {
		TrasformationMatrix[i]=new Matrix(3,3);
		TrasformationMatrix[i]->SetIdentity();
		RelativeTrasformationMatrix[i]=new Matrix(3,3);
	}

	// Set default limits
	MinT[0]=MinT[1]=MinT[2]=-FLT_MAX;
	MaxT[0]=MaxT[1]=MaxT[2]=FLT_MAX;
	MinR[0]=MinR[1]=MinR[2]=-BONE_DEF_LIMITS;
	MaxR[0]=MaxR[1]=MaxR[2]=BONE_DEF_LIMITS;
	
	// Default behaviour
	LockX=true;
	LockY=true;
	LockZ=true;
	LockRX=false;
	LockRY=false;
	LockRZ=false;
	AllowPropagation_=true;
}
Bone::Bone(Bone &original) : Tree (original.num_child,NULL) {
	NOT_IMPLEMENTED
}
Bone::Bone(Bone *original) : Tree (original->num_child,NULL) {
	length=original->length;
	strcpy(Name,original->Name);
	Number_Of_Frames=original->Number_Of_Frames;

	TrasformationT=new Vector<3>[Number_Of_Frames];
	RelativeTrasformationT=new Vector<3>[Number_Of_Frames];
	TrasformationMatrix=new Matrix*[Number_Of_Frames];
	RelativeTrasformationMatrix=new Matrix*[Number_Of_Frames];

	for(int i=0;i<Number_Of_Frames;i++) {
		TrasformationMatrix[i]=new Matrix(3,3);
		RelativeTrasformationMatrix[i]=new Matrix(3,3);

		TrasformationT[i]=original->TrasformationT[i];
		RelativeTrasformationT[i]=original->RelativeTrasformationT[i];
		TrasformationMatrix[i]->Set(original->TrasformationMatrix[i]);
		RelativeTrasformationMatrix[i]->Set(original->RelativeTrasformationMatrix[i]);
	}

	LockX=original->LockX;
	LockY=original->LockY;
	LockZ=original->LockZ;
	LockRX=original->LockRX;
	LockRY=original->LockRY;
	LockRZ=original->LockRZ;
	AllowPropagation_=original->AllowPropagation_;

	MinT=original->MinT;
	MaxT=original->MaxT;
	MinR=original->MinR;
	MaxR=original->MaxR;
}

Bone::~Bone() {
	for(int i=0;i<Number_Of_Frames;i++) {
		delete TrasformationMatrix[i];
		delete RelativeTrasformationMatrix[i];
	}
	delete TrasformationMatrix;
	delete RelativeTrasformationMatrix;
	delete []TrasformationT;
	delete []RelativeTrasformationT;
}

bool Bone::is(char *Name) {
	return (_stricmp(Name,this->Name)==0);
}

bool Bone::isEndBone() {
	if (strstr(this->Name,"EndBone_Tip")!=NULL) return true;
	if (strstr(this->Name,"Nub")!=NULL) return true;
	return false;
}

double Bone::GetLength() {
	return length;
}

bool Bone::PrintRel_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int *FrameIndex=static_cast<int *>(In);

	cout<<c->Name<<",  ";
	if (c->CanTranslate()) cout<<c->RelativeTrasformationT[*FrameIndex][0]<<","<<c->RelativeTrasformationT[*FrameIndex][1]<<","<<c->RelativeTrasformationT[*FrameIndex][2]<<",";

	Matrix Actual_Rotation(3,3);
	RelativeRotation(c->RelativeTrasformationMatrix[*FrameIndex],c->RelativeTrasformationMatrix[0],&Actual_Rotation);
	Vector<3> Rot=RotationMatrix2AngleAxis(&Actual_Rotation);
	Rot=(180.0/M_PI)*Rot;
	if (abs(Rot[0])<10e-10) Rot[0]=0;
	if (abs(Rot[1])<10e-10) Rot[1]=0;
	if (abs(Rot[2])<10e-10) Rot[2]=0;
	cout<<Rot[0]<<","<<Rot[1]<<","<<Rot[2]<<"\n";
	
	return false;
}

bool Bone::CalcRel_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int *FrameIndex=static_cast<int *>(In);

	if (c->Father==NULL) {
		c->RelativeTrasformationT[*FrameIndex]=c->TrasformationT[*FrameIndex];
		c->RelativeTrasformationMatrix[*FrameIndex]->Set(c->TrasformationMatrix[*FrameIndex]);
		return false;
	}
	Bone *Fc=static_cast<Bone *>(c->Father);
	
	Matrix C(4,4);
	Matrix FC(4,4);
	Matrix R(4,4);
	Vector<3> End;
	RTtoP(&C,c->TrasformationMatrix[*FrameIndex],&(c->TrasformationT[*FrameIndex]));
	Fc->GetEndPoint(*FrameIndex,&End);
	RTtoP(&FC,Fc->TrasformationMatrix[*FrameIndex],&End);
	FC.Inversion();
	Multiply(&FC,&C,&R);

	PtoRT(&R,c->RelativeTrasformationMatrix[*FrameIndex],&(c->RelativeTrasformationT[*FrameIndex]));
	return false;
}

bool Bone::CalcAbs_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int *FrameIndex=static_cast<int *>(In);

	if (c->Father==NULL) {
		c->TrasformationT[*FrameIndex]=c->RelativeTrasformationT[*FrameIndex];
		c->TrasformationMatrix[*FrameIndex]->Set(c->RelativeTrasformationMatrix[*FrameIndex]);
		return false;
	}
	Bone *Fc=static_cast<Bone *>(c->Father);
	
	Matrix C(4,4);
	Matrix FC(4,4);
	Matrix R(4,4);
	Vector<3> End;
	Fc->GetEndPoint(*FrameIndex,&End);
	RTtoP(&FC,Fc->TrasformationMatrix[*FrameIndex],&End);
	RTtoP(&C,c->RelativeTrasformationMatrix[*FrameIndex],&(c->RelativeTrasformationT[*FrameIndex]));
	Multiply(&FC,&C,&R);

	PtoRT(&R,c->TrasformationMatrix[*FrameIndex],&(c->TrasformationT[*FrameIndex]));
	return false;
}

bool Bone::CopyAbs_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int *src=static_cast<int *>(In);
	int *dst=static_cast<int *>(Out);

	c->TrasformationT[*dst]=c->TrasformationT[*src];
	c->TrasformationMatrix[*dst]->Set(c->TrasformationMatrix[*src]);
	
	return false;
}

void Bone::PrintRelativeInformation(int FrameIndex) {
	Traversal(PrintRel_Fnc,&FrameIndex,NULL);
}

bool Bone::PrintTree_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	
	for(int i=0;i<c->Level;i++) cout<<"|  ";
	cout<<"- "<<c->Name<<"       ["<<(c->CanRotate()?"R":"_")<<(c->CanTranslate()?"T":"_")<<"]\n";
	return false;
}

bool Bone::AddPrefix_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	char *prefix=static_cast<char *>(In);
	
	int ori_num=(int)strlen(c->Name);
	int num=(int)strlen(prefix);
	
	for(int i=ori_num;i>=0;i--) c->Name[i+num]=c->Name[i];
	for(int i=0;i<num;i++) c->Name[i]=prefix[i];
	return false;
}

bool Bone::RemovePostfix_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	char *postfix=static_cast<char *>(In);
	
	int l=(int)strlen(c->Name);
	int l_post=(int)strlen(postfix);
	
	if (l>l_post) {
		if (_stricmp(c->Name+(l-l_post),postfix)==0) {
			c->Name[l-l_post]=0;
		}
	}
	return false;
}

bool Bone::SubStPrefix_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	char *prefix=static_cast<char *>(In);
	
	int num=(int)strlen(prefix);
	for(int i=0;i<num;i++) c->Name[i]=prefix[i];

	return false;
}

void Bone::PrintTree() {
	Traversal(PrintTree_Fnc,0,NULL);
}

void Bone::AddPrefix(char *prefix) {
	Traversal(AddPrefix_Fnc,prefix,NULL);
}

void Bone::SubStPrefix(char *prefix) {
	Traversal(SubStPrefix_Fnc,prefix,NULL);
}

void Bone::RemovePostfix(char *postfix) {
	Traversal(RemovePostfix_Fnc,postfix,NULL);
}

void Bone::CalcRelativeFromAbsolute(int FrameIndex) {
	Traversal(CalcRel_Fnc,&FrameIndex,NULL);
}

void Bone::CalcAbsoluteFromRelative(int FrameIndex) {
	Traversal(CalcAbs_Fnc,&FrameIndex,NULL);
}

void Bone::Copy(int src_frame,int dst_frame) {
	if (src_frame!=dst_frame) Traversal(CopyAbs_Fnc,&src_frame,&dst_frame);
	CalcRelativeFromAbsolute(dst_frame);
}


void Bone::GetIKChain(Array<Bone*> *Chain) {
	// Ritorna   1   ...   n
	//         padre ... current
	//
	Chain->clear();
	
	Bone *Current=this;
	Chain->append(Current);
	while (Current->Father!=NULL) {
		Current=static_cast<Bone *>(Current->Father);
		Chain->append(Current);
	}
	Chain->reverse();
}

Array<Bone*> *Bone::Enumerate() {
	Array<Tree*> *c_=Tree::Enumerate();
	Array<Bone*> *c=reinterpret_cast<Array<Bone*> *>(c_);	
	return c;
}

Matrix **Bone::GetABSRotationArray() {
	return TrasformationMatrix;
}
Vector<3> *Bone::GetABSTranslationArray() {
	return TrasformationT;
}
Matrix **Bone::GetRELRotationArray() {
	return RelativeTrasformationMatrix;
}
Vector<3> *Bone::GetRELTranslationArray() {
	return RelativeTrasformationT;
}

bool Bone::SearchBone_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	char *name=static_cast<char *>(In);
	Bone **out=static_cast<Bone **>(Out);
	(*out)=c;

	if (!strcmp(name,c->Name)) return true;
	return false;
}

Bone *Bone::SearchBone(const char *Name) {
	Bone *Finded;
	if (!Traversal(SearchBone_Fnc,const_cast<void *>(reinterpret_cast<const void *>(Name)),&Finded)) return NULL;
	return Finded;
}


bool Bone::DrawBone_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	RenderWindow<3> *m=static_cast<RenderWindow<3> *>(In);
	int *FI=static_cast<int *>(Out);
	if (c->isEndBone()) return false;

	Vector<3> Scale;
	Scale[0]=Scale[1]=Scale[2]=c->length;
	m->SetRT(c->TrasformationMatrix[*FI],&(c->TrasformationT[*FI]),&Scale);
	m->DrawHyperMesh_Flat(BoneMesh);
#ifdef DRAW_REF_SYSTEM	
	m->DrawRefSystem();
#endif	
	m->ClearRT();
	return false;
}

void Bone::DrawAll(int FrameIndex,RenderWindow<3> *x) {
	Traversal(DrawBone_Fnc,x,&FrameIndex);
}

void Bone::GetPivotAndDirection(int FrameIndex,Vector<3> *Pivot,Vector<3> *Dir) {
	(*Dir)[0]=(*(TrasformationMatrix[FrameIndex]))[0][0];
	(*Dir)[1]=(*(TrasformationMatrix[FrameIndex]))[1][0];
	(*Dir)[2]=(*(TrasformationMatrix[FrameIndex]))[2][0];
	(*Dir)=length*(Dir->Versore());
	(*Pivot)=TrasformationT[FrameIndex];
}

void Bone::GetEndPoint(int FrameIndex,Vector<3> *End) {
	Vector<3> Dir;
	Dir[0]=(*(TrasformationMatrix[FrameIndex]))[0][0];
	Dir[1]=(*(TrasformationMatrix[FrameIndex]))[1][0];
	Dir[2]=(*(TrasformationMatrix[FrameIndex]))[2][0];
	Dir=Dir.Versore();
	(*End)=(TrasformationT[FrameIndex])+length*Dir;
}

void Bone::InitBoneMesh() {
	if (BoneMesh==NULL) {
		BoneMesh=new HyperMesh<3>();
		BoneMesh->Read("BoneMesh.objbin");
	}
}





void Bone::LoadSkullFrame(IFileBuffer *In,Bone *Root,int FrameIndex) {

	int maxlen;
	Matrix Tmp(3,3);Vector<3> TmpT;
	char TmpName[BONE_MAX_NAME_LEN+1];
	
	while (!(*In).IsFinished()) {
		maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
		Bone *Current=Root->SearchBone(TmpName);
		if (Current==NULL) {
			(*In)>>Tmp;
			(*In)>>TmpT;
			continue;
		}
		(*In)>>(*(Current->TrasformationMatrix[FrameIndex]));
		(*In)>>(Current->TrasformationT[FrameIndex]);
	}
}

void Bone::LoadSkullMotion(IFileBuffer *In,Bone *Root,int local_num_of_frames) {
	int len;
	int maxlen;
	float buffer[12];
	Matrix Tmp(3,3);Vector<3> TmpT;
	char TmpName[BONE_MAX_NAME_LEN+1];


	if (local_num_of_frames==0) local_num_of_frames=Root->Number_Of_Frames;
	int real_num_of_frames=Root->Number_Of_Frames;


	while (!(*In).IsFinished()) {
		maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
		Bone *Current=Root->SearchBone(TmpName);
		if (Current==NULL) {
			Warning_str("Bone %s does not exist.",TmpName);
			for(int t=0;t<local_num_of_frames;t++) {
				len=9*4;In->ReadLine((char*)&buffer,len);
				len=3*4;In->ReadLine((char*)&buffer,len);
			}
			continue;
		}
		for(int t=0;t<local_num_of_frames;t++) {
			len=9*4;In->ReadLine((char*)&buffer,len);
			if (t<real_num_of_frames) Current->TrasformationMatrix[t]->Set(buffer);
			len=3*4;In->ReadLine((char*)&buffer,len);
			if (t<real_num_of_frames) Current->TrasformationT[t].SetList(buffer);
		}
	}
}

void Bone::LoadSkullPose(IFileBuffer *In,Bone *Root) {

	int maxlen;
	Matrix Tmp(3,3);Vector<3> TmpT;
	char TmpName[BONE_MAX_NAME_LEN+1];
	
	int len;
	float buffer[12];

	while (!(*In).IsFinished()) {
		maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
		Bone *Current=Root->SearchBone(TmpName);
		if (Current==NULL) {
			len=9*4;In->ReadLine((char*)&buffer,len);
			len=3*4;In->ReadLine((char*)&buffer,len);
			continue;
		}
		len=9*4;In->ReadLine((char*)&buffer,len);
		Current->TrasformationMatrix[0]->Set(buffer);
		len=3*4;In->ReadLine((char*)&buffer,len);
		Current->TrasformationT[0].SetList(buffer);
	}
}


void Bone::SaveSkullMotion(OFileBuffer *Out,Bone *Root) {
	Matrix Tmp(3,3);Vector<3> TmpT;
	
	int len;
	float buffer[12];

	Array<Bone*> *o=Root->Enumerate();
	for(int i=0;i<o->numElements();i++) {
		Out->WriteString((char*)((*o)[i]->GetName()));
		Out->WriteString("\n");
		
		for(int t=0;t<Root->Number_Of_Frames;t++) {
			(*o)[i]->TrasformationMatrix[t]->Get(buffer);
			len=9*4;Out->WriteLine((char*)buffer,len);
			(*o)[i]->TrasformationT[t].GetList(buffer);
			len=3*4;Out->WriteLine((char*)buffer,len);
		}
	}
	delete o;
}



void Bone::SaveSkullMotion_Constant(OFileBuffer *Out,Bone *Root,int num_frames) {

	Matrix Tmp(3,3);Vector<3> TmpT;
	
	int len;
	float buffer[12];

	Array<Bone*> *o=Root->Enumerate();
	for(int i=0;i<o->numElements();i++) {
		Out->WriteString((char*)((*o)[i]->GetName()));
		Out->WriteString("\n");
		
		for(int t=0;t<=1;t++) {
			(*o)[i]->TrasformationMatrix[t]->Get(buffer);
			len=9*4;Out->WriteLine((char*)buffer,len);
			(*o)[i]->TrasformationT[t].GetList(buffer);
			len=3*4;Out->WriteLine((char*)buffer,len);
		}

		for(int t=2;t<num_frames;t++) {
			(*o)[i]->TrasformationMatrix[1]->Get(buffer);
			len=9*4;Out->WriteLine((char*)buffer,len);
			(*o)[i]->TrasformationT[1].GetList(buffer);
			len=3*4;Out->WriteLine((char*)buffer,len);
		}
	}
	delete o;
}
Bone *Bone::CloneBoneTree(Bone *original,Bone *Father) {
	Bone *x=new Bone(original);
	x->Father=Father;
	if (Father==NULL) x->Level=0;
	else x->Level=Father->Level+1;

	for(int i=0;i<original->num_child;i++) {
		if (original->Child[i]!=NULL) {
			Bone *o_c=static_cast<Bone *>(original->Child[i]);
			x->Child[i]=CloneBoneTree(o_c,x);
		}
	}

	return x;
}
Bone *Bone::CloneBone(Bone *original,int number_of_frames) {
	Bone *x=NULL;
	SNEW(x,Bone(original->Name,NULL,original->length,number_of_frames));
	
	int i=0;
	for(;i<min(number_of_frames,original->Number_Of_Frames);i++) {
		x->TrasformationT[i]=original->TrasformationT[i];
		x->RelativeTrasformationT[i]=original->RelativeTrasformationT[i];
		x->TrasformationMatrix[i]->Set(original->TrasformationMatrix[i]);
		x->RelativeTrasformationMatrix[i]->Set(original->RelativeTrasformationMatrix[i]);
	}
	int last_valid=i-1;
	if (i<0) ErrorExit("CloneBone: cannot clone a bone with zero frames");
	for(;i<number_of_frames;i++) {
		x->TrasformationT[i]=original->TrasformationT[last_valid];
		x->RelativeTrasformationT[i]=original->RelativeTrasformationT[last_valid];
		x->TrasformationMatrix[i]->Set(original->TrasformationMatrix[last_valid]);
		x->RelativeTrasformationMatrix[i]->Set(original->RelativeTrasformationMatrix[last_valid]);
	}

	x->LockX=original->LockX;
	x->LockY=original->LockY;
	x->LockZ=original->LockZ;
	x->LockRX=original->LockRX;
	x->LockRY=original->LockRY;
	x->LockRZ=original->LockRZ;
	x->AllowPropagation_=original->AllowPropagation_;

	x->MinT=original->MinT;
	x->MaxT=original->MaxT;
	x->MinR=original->MinR;
	x->MaxR=original->MaxR;

	return x;
}
Bone *Bone::CloneBoneTree(Bone *original,Bone *Father,int number_of_frames) {
	Bone *x=CloneBone(original,number_of_frames);
	x->Father=Father;
	if (Father==NULL) x->Level=0;
	else x->Level=Father->Level+1;

	for(int i=0;i<original->num_child;i++) {
		if (original->Child[i]!=NULL) {
			Bone *o_c=static_cast<Bone *>(original->Child[i]);
			x->Child[i]=CloneBoneTree(o_c,x,number_of_frames);
		}
	}

	return x;
}
int Bone::getNumberOfFrames() {
	return Number_Of_Frames;
}

void Bone::GetABSTrasformMatrix(int FrameIndex,Matrix *Out) {
	Out->SetMinor(3,3,TrasformationMatrix[FrameIndex]);
	(*Out)[0][3]=TrasformationT[FrameIndex][0];
	(*Out)[1][3]=TrasformationT[FrameIndex][1];
	(*Out)[2][3]=TrasformationT[FrameIndex][2];
	(*Out)[3][3]=1.0;
	(*Out)[3][0]=(*Out)[3][1]=(*Out)[3][2]=0.0;
}

void Bone::GetRelTrasformMatrix(int FrameIndex,Matrix *Out) {
	Out->SetMinor(3,3,RelativeTrasformationMatrix[FrameIndex]);

	Vector<3> Origin;
	// Origin = RelativeTrasformationT + Father->End_Bone
	Origin=RelativeTrasformationT[FrameIndex];
	if (Father!=NULL) {
		Bone *Fc=static_cast<Bone *>(Father);
		Origin[0]+=Fc->length;
	}

	(*Out)[0][3]=Origin[0];
	(*Out)[1][3]=Origin[1];
	(*Out)[2][3]=Origin[2];
	(*Out)[3][3]=1.0;
	(*Out)[3][0]=(*Out)[3][1]=(*Out)[3][2]=0.0;
}





bool Bone::CanTranslate() {
	if (!LockX) return true;
	if (!LockY) return true;
	if (!LockZ) return true;
	return false;
}

bool Bone::CanRotate() {
	if (!LockRX) return true;
	if (!LockRY) return true;
	if (!LockRZ) return true;
	return false;
}

bool Bone::CanRotateOrTranslate() {
	if (CanRotate()) return true;
	if (CanTranslate()) return true;
	return false;
}

bool Bone::AllowPropagation() {
	return AllowPropagation_;
}


bool Bone::Rotate(int FrameIndex,Vector<3> angle_axis,Vector<3> EffectiveRotation) {
	bool violated=false;
	Matrix R(3,3);
	Matrix Tot(3,3);

	double TMP_ANGLE=angle_axis.Norm2();
	if (FrameIndex==0) ErrorExit("The pose frame is not editable.");
	if (LockRX) angle_axis[0]=0.0;
	if (LockRY) angle_axis[1]=0.0;
	if (LockRZ) angle_axis[2]=0.0;
	double angle=angle_axis.Norm2();
	if (IS_ZERO(angle,DBL_EPSILON)) return false;
	
	if (TMP_ANGLE!=angle) {
		cout<<"ccicicicicicii";
	}

	AngleAxis2RotationMatrix(&R,angle_axis);
	Multiply(&R,RelativeTrasformationMatrix[FrameIndex],&Tot);
	
	
	// Check contraints
	#ifdef CHECK_CONSTRAINT
		// Trovo il moto realtivo dal tempo 0 a ora
		Matrix Actual_Rotation(3,3);
		RelativeRotation(&Tot,RelativeTrasformationMatrix[0],&Actual_Rotation);
		Vector<3> Rot=RotationMatrix2AngleAxis(&Actual_Rotation);
		Rot=(180.0/M_PI)*Rot;

		if (!LockRX) {
			if (Rot[0]>MaxR[0]) {
				Rot[0]=MaxR[0];
				violated=true;
			}
			if (Rot[0]<MinR[0]) {
				Rot[0]=MinR[0];
				violated=true;
			}
		}
		if (!LockRY) {
			if (Rot[1]>MaxR[1]) {
				Rot[1]=MaxR[1];
				violated=true;
			}
			if (Rot[1]<MinR[1]) {
				Rot[1]=MinR[1];
				violated=true;
			}
		}
		if (!LockRZ) {
			if (Rot[2]>MaxR[2]) {
				Rot[2]=MaxR[2];
				violated=true;
			}
			if (Rot[2]<MinR[2]) {
				Rot[2]=MinR[2];
				violated=true;
			}
		}

		if (violated) {
			Matrix Corretta(3,3);

			Rot=(M_PI/180.0)*Rot;
			AngleAxis2RotationMatrix(&Corretta,Rot);
			Multiply(&Corretta,RelativeTrasformationMatrix[0],&Tot);
			
			RelativeRotation(&Tot,RelativeTrasformationMatrix[FrameIndex],&Actual_Rotation);
			EffectiveRotation=RotationMatrix2AngleAxis(&Actual_Rotation);
			
			// Evito errori di calcolo
			if (LockRX) EffectiveRotation[0]=0.0;
			if (LockRY) EffectiveRotation[1]=0.0;
			if (LockRZ) EffectiveRotation[2]=0.0;
		}
	#endif

	RelativeTrasformationMatrix[FrameIndex]->Set(&Tot);
	return violated;
}

void PrintConstraintsWarning(Bone *b,int axis,double v) {
	v=fabs(v);
	if (v>0.001) {
		char c_axis='X';
		if (axis==1) c_axis='Y';
		if (axis==2) c_axis='Z';
		cout<<"Warning: "<<b->GetName()<<" has violated the constraints on axes "<<c_axis<<" of about "<<v<<" degrees.\n";
	}
}

void Bone::GetRelativeAngleAndTranslation(int FrameIndex,Vector<3> *R,Vector<3> *T) {
	Matrix Actual_Rotation(3,3);
	RelativeRotation(RelativeTrasformationMatrix[FrameIndex],RelativeTrasformationMatrix[0],&Actual_Rotation);
	(*R)=RotationMatrix2AngleAxis(&Actual_Rotation);
	(*R)[0]=(*R)[0]*180/M_PI;
	(*R)[1]=(*R)[1]*180/M_PI;
	(*R)[2]=(*R)[2]*180/M_PI;
	
	(*T)=RelativeTrasformationT[FrameIndex]-RelativeTrasformationT[0];
}

void Bone::ApplyConstraints(int FrameIndex) {
#ifdef CHECK_CONSTRAINT
	bool adjusted=false;
	double a,b,c;
	
	Vector<3> Rabc,Tabc;
	GetRelativeAngleAndTranslation(FrameIndex,&Rabc,&Tabc);
	a=Rabc[0];b=Rabc[1];c=Rabc[2];

	if (a>MaxR[0]) {
		PrintConstraintsWarning(this,0,a-MaxR[0]);
		a=MaxR[0];
		adjusted=true;
	}
	if (b>MaxR[1]) {
		PrintConstraintsWarning(this,1,b-MaxR[1]);
		b=MaxR[1];
		adjusted=true;
	}
	if (c>MaxR[2]) {
		PrintConstraintsWarning(this,2,c-MaxR[2]);
		c=MaxR[2];
		adjusted=true;
	}
	if (a<MinR[0]) {
		PrintConstraintsWarning(this,0,a-MinR[0]);
		a=MinR[0];
		adjusted=true;
	}
	if (b<MinR[1]) {
		PrintConstraintsWarning(this,1,b-MinR[1]);
		b=MinR[1];
		adjusted=true;
	}
	if (c<MinR[2]) {
		PrintConstraintsWarning(this,2,c-MinR[2]);
		c=MinR[2];
		adjusted=true;
	}

	if (adjusted) {
		Matrix Corretta(3,3);
		a=a*M_PI/180.0;
		b=b*M_PI/180.0;
		c=c*M_PI/180.0;
		Vector<3> Rot;
		Rot[0]=a;Rot[1]=b;Rot[2]=c;
		AngleAxis2RotationMatrix(&Corretta,Rot);

		Matrix Actual_Rotation(3,3);
		Multiply(&Corretta,RelativeTrasformationMatrix[0],&Actual_Rotation);
		RelativeTrasformationMatrix[FrameIndex]->Set(&Actual_Rotation);
	}

#endif
}




void Bone::Rotate(int FrameIndex,Vector<3> angle_axis) {
	Matrix R(3,3);
	Matrix Tot(3,3);

	double angle=angle_axis.Norm2();
	if (IS_ZERO(angle,DBL_EPSILON)) return;
	
	AngleAxis2RotationMatrix(&R,angle_axis);
	Multiply(&R,RelativeTrasformationMatrix[FrameIndex],&Tot);
	RelativeTrasformationMatrix[FrameIndex]->Set(&Tot);
}
void Bone::Translate(int FrameIndex,Vector<3> T) {
	RelativeTrasformationT[FrameIndex]+=T;
}

void Bone::Rotate(int FrameIndex,Matrix *M) {
	Matrix Tot(3,3);
	::Orthonormalize(M);
		
	Multiply(M,RelativeTrasformationMatrix[FrameIndex],&Tot);
	RelativeTrasformationMatrix[FrameIndex]->Set(&Tot);
}


//void Bone::RotoTranslate(int FrameIndex,Vector<3> M,Vector<3> T,bool IsRelative) {
//	if (FrameIndex==0) ErrorExit("The pose frame is not editable.");
//	
//	double angle=M.Norm2();
//
//	if (!IS_ZERO(angle,DBL_EPSILON)) {
//		// Ruota
//		Matrix R(3,3);
//		Matrix Tot(3,3);
//		AngleAxis2RotationMatrix(&R,M);
//		Multiply(&R,RelativeTrasformationMatrix[FrameIndex],&Tot);
//		
//		// Vincoli
//	
//		// Aggiorno
//		RelativeTrasformationMatrix[FrameIndex]->Set(&Tot);
//	}
//	
//	// Trasla
//
//
///*
//	// Trovo l'asse di rotazione relativo (rispetto il sistema di riferimento del padre)
//	Vector<3> Relative_axis;
//	if (IsRelative) {
//		Relative_axis=axis;
//	} else {
//		if (Father==NULL) {
//			Relative_axis=axis;
//		} else {
//			Bone *Fc=static_cast<Bone *>(Father);
//			Matrix ITrasf(3,3);
//			ITrasf.Set(Fc->TrasformationMatrix[FrameIndex]);
//			ITrasf.Inversion();
//			Multiply(&ITrasf,axis,Relative_axis.GetList());
//			Relative_axis=Relative_axis.Versore();
//		}
//	}
//	
//
//	// Ruoto la relativa (Relative_axis) -> Final
//	Matrix R(4,4);
//	RotationMatrix(&R,angle,Relative_axis[0],Relative_axis[1],Relative_axis[2]);
//	Matrix R3X3(3,3);
//	R.GetMinor(3,3,&R3X3);
//	Matrix Final(3,3);
//	Multiply(&R3X3,RelativeTrasformationMatrix[FrameIndex],&Final);
//
//
////#define CHECK_CONSTRAINT
//
//#ifdef CHECK_CONSTRAINT
//	// Trovo il moto realtivo dal tempo 0 a ora
//	Matrix Actual_Rotation(3,3);
//	RelativeRotation(&Final,RelativeTrasformationMatrix[0],&Actual_Rotation);
//	double a,b,c;
//	RotationMatrixToEuler(&Actual_Rotation,a,b,c);
//	//cout<<"\n"<<Actual_Rotation<<"\n\n";
//	//cout<<"\n"<<a<<"  "<<b<<"  "<<c<<"\n\n";
//	a=a*180/M_PI;
//	b=b*180/M_PI;
//	c=c*180/M_PI;
//	Vector<3> Trel;
//	Trel=(RelativeTrasformationT[FrameIndex]+T-RelativeTrasformationT[0]);
//
//	//  problemi con conversione da eulero... >0 puo' partire via....
//	if (a>MaxR[0]) a=MaxR[0];
//	if (b>MaxR[1]) b=MaxR[1];
//	if (c>MaxR[2]) c=MaxR[2];
//	if (a<MinR[0]) a=MinR[0];
//	if (b<MinR[1]) b=MinR[1];
//	if (c<MinR[2]) c=MinR[2];
//	if (Trel[0]<MinT[0]) Trel[0]=MinT[0];
//	if (Trel[1]<MinT[1]) Trel[1]=MinT[1];
//	if (Trel[2]<MinT[2]) Trel[2]=MinT[2];
//	if (Trel[0]>MaxT[0]) Trel[0]=MaxT[0];
//	if (Trel[1]>MaxT[1]) Trel[1]=MaxT[1];
//	if (Trel[2]>MaxT[2]) Trel[2]=MaxT[2];
//
//	Matrix Corretta(3,3);
//	a=a*M_PI/180.0;
//	b=b*M_PI/180.0;
//	c=c*M_PI/180.0;
//	EulerToRotationMatrix(&Corretta,a,b,c);
//	//RotationMatrixToEuler(&Corretta,a,b,c);
//	//cout<<"\n"<<Corretta<<"\n\n";
//	//cout<<"-> "<<a<<"  "<<b<<"  "<<c<<"\n";
//
//	// Setto la nuova matrice
//	Matrix NuovaMatrice(3,3);
//	Multiply(&Corretta,RelativeTrasformationMatrix[0],&NuovaMatrice);
//	RelativeTrasformationMatrix[FrameIndex]->Set(&NuovaMatrice);
//	RelativeTrasformationT[FrameIndex]=Trel+RelativeTrasformationT[0];
//#else
//	RelativeTrasformationMatrix[FrameIndex]->Set(&Final);
//#endif
//*/
//
//
//
//
//
//	// Aggiorno il subTree
//	CalcAbsoluteFromRelative(FrameIndex);
//}

void Bone::RelativeMotion(int FrameIndex,int RefFrameIndex,Vector<3> *T,Vector<3> *R) {
	// FinalR=Rotation*InitialR
	// FinalT=T+InitialT
	
	Matrix Actual_Rotation(3,3);
	RelativeRotation(RelativeTrasformationMatrix[FrameIndex],RelativeTrasformationMatrix[RefFrameIndex],&Actual_Rotation);
	*R=RotationMatrix2AngleAxis(&Actual_Rotation);
	*T=RelativeTrasformationT[FrameIndex]-RelativeTrasformationT[RefFrameIndex];
}

bool Bone::Filter_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int FrameIndex=*(static_cast<int*>(In));
	int Filter_Extension=*(static_cast<int*>(Out));
	
	//
	// Exclude Root bones
	//
	if (str_begin(c->GetName(),"Root_")) return false;



	//
	// Generate the Filter
	//
	double sum=0;
	double *peso_t=new double[2*Filter_Extension+1];
	for(int i=-Filter_Extension;i<=Filter_Extension;i++) 
		peso_t[i+Filter_Extension]=exp(-2.0*(i/(double)Filter_Extension)*(i/(double)Filter_Extension));
	for(int i=-Filter_Extension;i<=Filter_Extension;i++) sum+=peso_t[i+Filter_Extension];
	for(int i=-Filter_Extension;i<=Filter_Extension;i++) peso_t[i+Filter_Extension]/=sum;

	double *peso_r=new double[Filter_Extension+1];
	for(int i=0;i<=Filter_Extension;i++) 
		peso_r[i]=exp(-2.0*(i/(double)Filter_Extension)*(i/(double)Filter_Extension));


	//
	// Filtra translazione
	//
	Vector<3> Current_t;
	for(int i=-Filter_Extension;i<=Filter_Extension;i++) {
		int use_i=FrameIndex+i;
		if (use_i<1) use_i=1;
		if (use_i>=c->getNumberOfFrames()) use_i=c->getNumberOfFrames()-1;
		Current_t=Current_t+(peso_t[i+Filter_Extension]*c->RelativeTrasformationT[use_i]);
	}
	c->RelativeTrasformationT[FrameIndex]=Current_t;



	//
	// Filter using SLERP
	//
		////////////////////////// CHECK for OUTLIERS and ANGLE AXIS problems /////////////////
		for(int i=-Filter_Extension;i<=Filter_Extension;i++) {
			int use_i=FrameIndex+i;
			if (use_i<1) continue;
			if (use_i>=c->getNumberOfFrames()) continue;
			Vector<3> to_check=RotationMatrix2AngleAxis(c->RelativeTrasformationMatrix[use_i]);
			Matrix tmp(3,3);
			AngleAxis2RotationMatrix(&tmp,to_check);

			double error=0.0;
			for(int q=0;q<3;q++) {
				for(int h=0;h<3;h++) {
					error+=fabs(tmp[q][h]-(*(c->RelativeTrasformationMatrix[use_i]))[q][h]);
				}
			}
			if (error>0.005) {
				int use_i2=use_i-1;
				if (use_i2<1) use_i2=use_i+1;
				c->RelativeTrasformationMatrix[use_i]->Set(c->RelativeTrasformationMatrix[use_i2]);
				cout<<use_i<<" -> outlier "<<error<<endl;
			}
		}
		//////////////////////////

		Vector<3> Current=RotationMatrix2AngleAxis(c->RelativeTrasformationMatrix[FrameIndex]);
		Quaternion current(Current);
		Quaternion FinalR=current;

		for(int i=1;i<=Filter_Extension;i++) {
			int use_i_b=FrameIndex-i;
			if (use_i_b<1) use_i_b=1;
			if (use_i_b>=c->getNumberOfFrames()) use_i_b=c->getNumberOfFrames()-1;

			int use_i_f=FrameIndex+i;
			if (use_i_f<1) use_i_f=1;
			if (use_i_f>=c->getNumberOfFrames()) use_i_f=c->getNumberOfFrames()-1;
			
			Vector<3> B=RotationMatrix2AngleAxis(c->RelativeTrasformationMatrix[use_i_b]);
			Vector<3> F=RotationMatrix2AngleAxis(c->RelativeTrasformationMatrix[use_i_f]);

			Quaternion b(B),f(F);
			Quaternion m=Quaternion::Interpolation(b,f,0.5,current);
			
			FinalR=Quaternion::Interpolation(FinalR,m,peso_r[i],current);
		}

		Vector<3> angle_axis=FinalR.Angle_Axis();
		Matrix tmp(3,3);
		AngleAxis2RotationMatrix(&tmp,angle_axis);
		c->RelativeTrasformationMatrix[FrameIndex]->Set(&tmp);
		

	delete []peso_t;
	delete []peso_r;
	return false;
}

void Bone::Filter(int FrameIndex,int Filter_Extension) {
	Traversal(Filter_Fnc,&FrameIndex,&Filter_Extension);
}

bool Bone::Orthonormalize_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	int FrameIndex=*(static_cast<int*>(In));

	::Orthonormalize(c->RelativeTrasformationMatrix[FrameIndex]);
	return false;
}

void Bone::Orthonormalize(int FrameIndex) {
	Traversal(Orthonormalize_Fnc,&FrameIndex,NULL);
}


bool Bone::Save_Fnc(Tree *x,void *In,void *Out) {
	Bone *c=static_cast<Bone *>(x);
	OFileBuffer *Out_=static_cast<OFileBuffer *>(Out);
	
	if (c->Father==NULL) (*Out_)<<"NULL\r\n";
	else {
		Bone *Fc=static_cast<Bone *>(c->Father);
		(*Out_)<<Fc->GetName()<<"\r\n";
	}
	(*Out_)<<c->GetName()<<"\r\n";
	(*Out_)<<c->length<<"\r\n";
	return false;
}

void Bone::SaveSkull(OFileBuffer *Out,Bone *BoneRoot) {
	(*Out)<<BoneRoot->Number_Of_Frames<<"\r\n";
	BoneRoot->Traversal(Save_Fnc,NULL,Out);
}

Bone *Bone::LoadSkull(IFileBuffer *In,int set_num_frames,int *old_num_frames) {
	
	int maxlen;
	int n_frames;
	double tmplen;
	char TmpName[BONE_MAX_NAME_LEN+1];
	
	if ((*In).IsFinished()) return NULL;
	(*In)>>n_frames;
	if (old_num_frames!=NULL) (*old_num_frames)=n_frames;
	if (set_num_frames!=0) n_frames=set_num_frames;
	maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
	maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
	(*In)>>tmplen;
	Bone *Root=new Bone(TmpName,NULL,tmplen,n_frames);


	while (!(*In).IsFinished()) {
		maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
		Bone *Father=Root->SearchBone(TmpName);
		maxlen=BONE_MAX_NAME_LEN;(*In).ReadPharse(TmpName,maxlen);
		(*In)>>tmplen;
		Bone *Current=new Bone(TmpName,Father,tmplen,n_frames);
		Father->AddChild(Current);
	}

	return Root;
}

Vector<3> Bone::GetJointPosition(int FrameIndex) {
	return TrasformationT[FrameIndex];
}


OFileBuffer &operator<<(OFileBuffer &os,Bone *b) {
	os<<b->GetName();
	return os; 
}
IFileBuffer &operator>>(IFileBuffer &os,Bone *b) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,Bone *b) {
	os<<b->GetName();
	return os; 
}
