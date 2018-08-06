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


OFileBuffer &operator<<(OFileBuffer &os,TargetList_1D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
IFileBuffer &operator>>(IFileBuffer &os,TargetList_1D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_1D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
OFileBuffer &operator<<(OFileBuffer &os,TargetList_2D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
IFileBuffer &operator>>(IFileBuffer &os,TargetList_2D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_2D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
OFileBuffer &operator<<(OFileBuffer &os,TargetList_3D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
IFileBuffer &operator>>(IFileBuffer &os,TargetList_3D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_3D_Item_type &v) {
	NOT_IMPLEMENTED;
	return os; 
}
bool TargetList_1D_Item_type::operator== (const TargetList_1D_Item_type &v) const {
	if (VertexIndex==v.VertexIndex) return true;
	return false;
}
bool TargetList_2D_Item_type::operator== (const TargetList_2D_Item_type &v) const {
	if (VertexIndex==v.VertexIndex) return true;
	return false;
}
bool TargetList_3D_Item_type::operator== (const TargetList_3D_Item_type &v) const {
	if (VertexIndex==v.VertexIndex) return true;
	return false;
}




OFileBuffer &operator<<(OFileBuffer &os,TargetList_2D_Item_type_Frame_Info &v) {
	NOT_IMPLEMENTED;
	return os; 
}
IFileBuffer &operator>>(IFileBuffer &os,TargetList_2D_Item_type_Frame_Info &v) {
	NOT_IMPLEMENTED;
	return os;
}
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_2D_Item_type_Frame_Info &v) {
	NOT_IMPLEMENTED;
	return os; 
}
bool TargetList_2D_Item_type_Frame_Info::operator== (const TargetList_2D_Item_type_Frame_Info &v) const {
	if ((frame==v.frame) && (VertexIndex==v.VertexIndex)) return true;
	return false;
}














View::View() : Int(3,3), Ext(3,4), P(3,4), ExtOGL(3,4), InvM(3,3) {
	Enable=(VIEW_ENABLE_SILHOUETTE|VIEW_ENABLE_OPTICAL_FLOW);
	width=height=0;
	kc[0]=kc[1]=kc[2]=kc[3]=kc[4]=0.0;
	
	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) i_rgb[i]=NULL;
	PDE=NULL;

	SetChannelTypes("CGF");
}
View::~View() {
	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) DeleteChannel(i);
	SDELETE(PDE);
}


void View::SetChannelTypes(char *channel_types) {
	if (strlen(channel_types)>VIEW_MAX_N_CHANNELS) ErrorExit("The number of channels is too high.");
	
	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) {
		if (channel_types[i]==0) {
			for(;i<VIEW_MAX_N_CHANNELS;i++) ChannelTypes[i]=' ';
			break;
		}
		if ((i_rgb[i]) && (ChannelTypes[i]!=channel_types[i])) ErrorExit("Cannot change the type of a non NULL channel.");
		ChannelTypes[i]=' ';
		if (channel_types[i]=='C') ChannelTypes[i]='C';
		if (channel_types[i]=='G') ChannelTypes[i]='G';
		if (channel_types[i]=='F') ChannelTypes[i]='F';
	}
}
void View::CreateChannel(int i) {
	if ((i<0) || (i>=VIEW_MAX_N_CHANNELS)) return;
	if (i_rgb[i]) ErrorExit_int("Channel %i already created.",i);
	if (ChannelTypes[i]=='C') i_rgb[i]   = new Bitmap<ColorRGB>  (width,height);
	if (ChannelTypes[i]=='G') i_grey[i]  = new Bitmap<GreyLevel> (width,height);
	if (ChannelTypes[i]=='F') i_float[i] = new Bitmap<ColorFloat>(width,height);
	if (!(i_rgb[i])) ErrorExit_int("Cannot create channel %i.",i);
}
void View::DeleteChannel(int i) {
	if ((i<0) || (i>=VIEW_MAX_N_CHANNELS)) return;
	if (ChannelTypes[i]=='C') SDELETE(i_rgb[i]);
	if (ChannelTypes[i]=='G') SDELETE(i_grey[i]);
	if (ChannelTypes[i]=='F') SDELETE(i_float[i]);
	if (i_rgb[i]) ErrorExit_int("Cannot delete channel %i.",i);
}




void View::CreateProjectionMatrix() {
	Multiply(&Int,&Ext,&P);
	ExtOGL.Set(&Ext);
	CorrectExtrinsic_For_OpenGL3D_Visualization(&ExtOGL);
}
void View::CreateUnProjectionMatrix() {
	InvM[0][0]=P[0][0];
	InvM[0][1]=P[0][1];
	InvM[0][2]=P[0][2];
	InvM[1][0]=P[1][0];
	InvM[1][1]=P[1][1];
	InvM[1][2]=P[1][2];
	InvM[2][0]=P[2][0];
	InvM[2][1]=P[2][1];
	InvM[2][2]=P[2][2];
	O[0]=P[0][3];
	O[1]=P[1][3];
	O[2]=P[2][3];
	
	InvM.Inversion();

	Vector<3> g0,g1;
	InvM.GetColumn(0,g0.GetList());
	InvM.GetColumn(1,g1.GetList());

	PrjPlaneNormal=(g0^g1).Versore();

	GetCPfromExt(&Ext,&PrjCenter);
}
Vector<3> View::UnProjection(Vector<3> x) {
	Vector<3> Unprj;
	
	x[0]=x[0]*x[2];
	x[1]=x[1]*x[2];
	Multiply(&InvM,x-O,Unprj.GetList());

	return Unprj;
}
Vector<3> View::UnProjection(Vector<2> x) {
	Vector<3> tmp;
	Vector<3> Unprj;

	tmp[0]=x[0];
	tmp[1]=x[1];
	tmp[2]=1;

	Multiply(&InvM,tmp-O,Unprj.GetList());

	return Unprj;
}
inline Vector<3> View::UnTrasform(Vector<3> x) {
	Vector<3> UnTr;

	Multiply(&InvM,x-O,UnTr.GetList());

	return UnTr;
}
Vector<2> View::VectorProjection(Vector<3> v,Vector<3> p) {
	
	Vector<2> p1=Projection(p);
	Vector<2> p2=Projection(p+v);

	return (p2-p1);
}
inline Vector<2> View::Projection(Vector<3> x) {
	Vector<2> point;

	Vector<3> tmp=Trasform(x);

	point[0]=tmp[0]/tmp[2];
	point[1]=tmp[1]/tmp[2];
	return point;
}
Vector<3> View::ProjectionZ(Vector<3> x) {
	Vector<3> point;

	Vector<3> tmp=Trasform(x);

	point[0]=tmp[0]/tmp[2];
	point[1]=tmp[1]/tmp[2];
	point[2]=tmp[2];
	return point;
}
Vector<2> View::ProjectionToImage(Vector<3> x) {
	Vector<2> point=Projection(x);

	point[0]=Approx(point[0]);
	point[1]=Approx(point[1]);
	return point;
}
void View::ProjectionToImage(Vector<3> x,int *p2D) {
	Vector<2> point=Projection(x);

	p2D[0]=Approx(point[0]);
	p2D[1]=Approx(point[1]);
}
inline Vector<3> View::Trasform(Vector<3> x) {
	Vector<3> tmp;
	Vector<4> omo;
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1.0f;

	Multiply(&P,omo,tmp.GetList());
	
	return tmp;
}
Vector<3> View::VectorTrasform(Vector<3> x) {
	Vector<3> tmp;
	Vector<4> omo;
	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=0.0f;

	Multiply(&P,omo,tmp.GetList());
	
	return tmp;
}


void View::GetNearFarPlane(HyperMesh<3> *mesh,double &min_z,double &max_z) {
	Vector<3> *P=mesh->Points.getMem();
	Vector<3> p;
	min_z=DBL_MAX;
	max_z=-DBL_MAX;

	for(int i=mesh->num_p;i>0;i--,P++) {
		p=ProjectionZ(*P);
		if ((p[0]<0) || (p[0]>=width) || (p[1]<0) || (p[1]>=height)) continue;
		max_z=max(max_z,p[2]);
		min_z=min(min_z,p[2]);
	}
}



















#define GENERATE_VIDEO_SRC(src,type,path) {								\
	if (CompareFileExtension(path,"AVI")) {								\
		(src)=new AviVideoIn_OCV<type>(path,false);						\
	} else {															\
		(src)=new ImageSequence<type>(path,0);							\
	}																	\
	w=(src)->width;														\
	h=(src)->height;													\
}

#define LOAD_VIDEO_SRC(input,type,output) {											\
	if (!(output)) {																\
		(output)=new Bitmap<type>(view->width,view->height);						\
	} else {																		\
		if ((output->width!=view->width) || (output->height!=view->height))	{		\
			DELETE_VIDEO_CHANNELS;													\
			ErrorExit("Incompatible bitmap sizes.");								\
		}																			\
	}																				\
	status=(input)->GetFrame(frame_index,output);									\
}

#define DELETE_VIDEO_CHANNELS														\
	for(int q=0;q<VIEW_MAX_N_CHANNELS;q++) {										\
		if (view->ChannelTypes[q]=='C') SDELETE(src_rgb[q]);						\
		if (view->ChannelTypes[q]=='G') SDELETE(src_grey[q]);						\
		if (view->ChannelTypes[q]=='F') SDELETE(src_float[q]);						\
		if (src_rgb[q]) Warning("Unknow channel type.");							\
	}


ViewStreamer::ViewStreamer(View *view,char **ch_src) {
	this->view=view;
	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) src_rgb[i]=NULL;
	
	int w,h;
	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) {
		if (ch_src[i]==NULL) break;
		if ((view->ChannelTypes[i]!=' ') && (ch_src[i][0]!=0)) {
			if (view->ChannelTypes[i]=='C') GENERATE_VIDEO_SRC(src_rgb[i],ColorRGB,ch_src[i]);
			if (view->ChannelTypes[i]=='G') GENERATE_VIDEO_SRC(src_grey[i],GreyLevel,ch_src[i]);
			if (view->ChannelTypes[i]=='F') GENERATE_VIDEO_SRC(src_float[i],ColorFloat,ch_src[i]);
			if (!(src_rgb[i])) {
				DELETE_VIDEO_CHANNELS;
				ErrorExit("Unknow channel type, terminating."); 
			}
			if (src_rgb[i]->num_frames==-1) {
				DELETE_VIDEO_CHANNELS;
				ErrorExit_str("Cannot open stream: %s.",ch_src[i]); 
			}
			
			if ((view->width==0) && (view->height==0)) {
				view->width=w;
				view->height=h;
			} else {
				if ((w!=view->width) || (h!=view->height)) {
					DELETE_VIDEO_CHANNELS;
					ErrorExit("Image sizes are different across view channels.");
				}
			}
		}
	}
}

ViewStreamer::~ViewStreamer() {
	DELETE_VIDEO_CHANNELS;
}

bool ViewStreamer::LoadChannels(int frame_index) {
	bool global_status=true;

	for(int i=0;i<VIEW_MAX_N_CHANNELS;i++) {
		if (src_rgb[i]) {
			bool status=false;
			if (view->ChannelTypes[i]=='C') LOAD_VIDEO_SRC(src_rgb[i],ColorRGB,view->i_rgb[i]);
			if (view->ChannelTypes[i]=='G') LOAD_VIDEO_SRC(src_grey[i],GreyLevel,view->i_grey[i]);
			if (view->ChannelTypes[i]=='F') LOAD_VIDEO_SRC(src_float[i],ColorFloat,view->i_float[i]);
			if (!status) {
				Warning_int2("Cannot load channel %i at frame %i.",i,frame_index);
				global_status=false;
			}
		}
	}
	return global_status;
}
































	//void InitContourExtractor();						// call after all view parameters are setted
	// ZBuffer
	/*
	SoftZBuffer *ZMask;									
	void InitZBuffer();
	void RenderDepthObj(HyperMesh<3> *mesh);
	float getZ(int x,int y);
	double ZMask_Lasco;																	// Settato automaticamente dopo il rendering (si puo' cambiare)
	bool isVisible(int i_vertex,HyperMesh<3> *mesh,Vector<2> *Pr_Point=NULL);			// Dice se il vertice i della mesh appena disegnata è visibile o meno
	
	//int  getNearestReproiectionPoint(int i_vertex,HyperMesh<3> *mesh,Array<typename Vector<2>> PointList);
	*/
/*
// Afferma se nelle vicinanza di (x,y) l'immagine im presenta dei pixel di foreground
bool IsLocallyForeground(int x,int y,Bitmap<GreyLevel> *im,GreyLevel Bk_Color,int dim_mask) {
	int d=(dim_mask-1)/2;
	Vector<2> dir;

	for(int i=-d+x;i<=x+d;i++) {
		for(int j=-d+y;j<=y+d;j++) {
			if (im->Point(i,j)!=Bk_Color) return true;
		}
	}

	return false;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int saved_w;
int saved_h;
Bitmap<GreyLevel> *ContourMask=NULL;
OffScreenRenderer *Renderer=NULL;

void View::InitContourExtractor() {
	if (ContourMask==NULL) {
		ContourMask=new Bitmap<GreyLevel>(width,height);
		saved_w=width;
		saved_h=height;
		Renderer=new OffScreenRenderer(width,height,1);
		if ((ContourMask==NULL) || (Renderer==NULL)) ErrorExit("Not enough memory.");
	} else {
		if ((saved_w!=width) || (saved_h!=height)) ErrorExit("Inconsistent view dimensions.");
	}
}
*/

/*
void View::InitZBuffer() {
	if (ZMask==NULL) ZMask=new SoftZBuffer(width,height,this);
}

void View::RenderDepthObj(HyperMesh<3> *mesh) {
	ZMask->BufferZ->Clear(-FLT_MAX);
	ZMask->DrawObj(mesh);
	ZMask_Lasco=(ZMask->ZMax-ZMask->ZMin)*0.02;
}

float View::getZ(int x,int y) {
	return ZMask->BufferZ->Point(x,y);
}

bool View::isVisible(int i_vertex,HyperMesh<3> *mesh,Vector<2> *Pr_Point) {
		
	Vector<3> D=ProjectionZ(mesh->Points[i_vertex]);
	if (Pr_Point!=NULL) {
		(*Pr_Point)[0]=D[0];
		(*Pr_Point)[1]=D[1];
	}
	float z=getZ(Approx(D[0]),Approx(D[1]));

	if (D[2]>=z-ZMask_Lasco) {
		//ZMask->BufferZ->SetPoint(D[0],D[1],FLT_MIN);
		return true;
	}
	return false;
}
*/
/*
int getNearestReproiectionPoint(int i_vertex,HyperMesh<3> *mesh,Array<typename Vector<2>> PointList) {
	return 1;
}
*/







































Camera::Camera(char *CameraFile,char *ValidityFile,int filter_dim,double oulier_detection_threshold) {
	Int_=NULL;
	Ext_=NULL;
	Ext_Original_=NULL;
	validity=NULL;
	P=NULL;
	IPR=NULL;
	PT=NULL;
	Image_=NULL;
	Mask_=NULL;
	MaskAux_=NULL;
	Silhouette_=NULL;
	Channels_=NULL;
	width_=-1;
	height_=-1;
	n_channels=1;

	SNEWA_P(Channels_,Bitmap<GreyLevel>*,n_channels);
	

	IFileBuffer In(CameraFile);
	if (In.IsFinished()) {
		cout<<CameraFile<<" not found.\n";
		exit(-1);
	}
	In>>num_frames_;
	if (num_frames_<=0) {
		cout<<CameraFile<<" invalid format.\n";
		exit(-1);
	}

	Int_=new Matrix*[num_frames_];
	Ext_=new Matrix*[num_frames_];
	Ext_Original_=new Matrix*[num_frames_];

	Matrix tmp_R(3,3);	// TODO**
	Vector<3> tmp_T;	// TODO**

	for(int i=0;i<num_frames_;i++) {
		Int_[i]=new Matrix(3,3);
		Ext_[i]=new Matrix(3,4);
		Ext_Original_[i]=new Matrix(3,4);

		In>>*(Int_[i]);
		In>>*(Ext_Original_[i]);

		// TODO**
		PtoRT(Ext_Original_[i],&tmp_R,&tmp_T);
		if (tmp_R.Determinante()==0.0) {
			tmp_R.SetIdentity();
			RTtoP(Ext_Original_[i],&tmp_R,&tmp_T);
		}
		//
		

		// Crea Ext_[i]
		Ext_[i]->Set(Ext_Original_[i]);
		CorrectExtrinsic_For_OpenGL3D_Visualization(Ext_[i]);
	}


	SNEWA(validity,char,num_frames_);
	memset(validity,(int)'1',num_frames_);

	IFileBuffer VIn(ValidityFile);
	if (VIn.IsFinished()) {
		Warning_int("%s not found.",ValidityFile);
	} else {
		int len=num_frames_;
		VIn.ReadLine(validity,len);
		if (len!=num_frames_) cout<<ValidityFile<<" corrupted.\n";
	}

	Filter(filter_dim,oulier_detection_threshold);
}

Camera::~Camera() {
	SDELETEA_REC(Int_,num_frames_);
	SDELETEA_REC(Ext_,num_frames_);
	SDELETEA(validity);

	SDELETEA_REC(P,num_frames_);
	SDELETEA_REC(IPR,num_frames_);
	SDELETEA(PT);
	SDELETEA(Channels_);		// PS: non ne deve cancellare il contenuto
}









void Camera::SetSize(int width_,int height_) {
	this->width_=width_;
	this->height_=height_;
}
void Camera::Save(char *filename) {
	OFileBuffer Out(filename);
	Out<<num_frames_<<"\n";

	for(int i=0;i<num_frames_;i++) {
		Out<<*(Int_[i]);
		Out<<*(Ext_Original_[i]);
	}
	
	Out.Close();
}
void Camera::Filter(int filter_dim,double oulier_detection_threshold) {
	if ((filter_dim==0) && (oulier_detection_threshold==0.0)) return;
	if ((filter_dim!=0) && (oulier_detection_threshold!=0.0)) {
		cout<<"Filtering/Outlier detection camera poses ("<<filter_dim<<","<<oulier_detection_threshold<<")\n";
	} else {
		if (filter_dim!=0) cout<<"Filtering camera poses ("<<filter_dim<<")\n";
		if (oulier_detection_threshold!=0.0) cout<<"Outlier detection on camera poses ("<<oulier_detection_threshold<<")\n";
	}

	// Filtra Ext_Original_
	double *weights=NULL;
	if (validity) {
		SNEWA(weights,double,num_frames_);
		for(int i=0;i<num_frames_;i++) {
			if (validity[i]=='1') weights[i]=1.0;
			else weights[i]=0.0;
		}
	}
	FilterSE3(Ext_Original_,num_frames_,filter_dim,weights,oulier_detection_threshold);
	SDELETEA(weights);


	// Crea Ext_
	for(int i=0;i<num_frames_;i++) {
		Ext_[i]->Set(Ext_Original_[i]);
		CorrectExtrinsic_For_OpenGL3D_Visualization(Ext_[i]);
	}
}
























//
//
// Projections P and inv(P)
//
//
void Camera::InitProjections() {
	Vector<3> tmp;

	if (P) return;
	P=new Matrix*[num_frames_];
	IPR=new Matrix*[num_frames_];
	PT=new Vector<3>[num_frames_];

	for(int i=0;i<num_frames_;i++) {
		P[i]=new Matrix(3,4);
		IPR[i]=new Matrix(3,3);
		
		Multiply(Int_[i],Ext_Original_[i],P[i]);
		PtoRT(P[i],IPR[i],&(PT[i]));
		IPR[i]->Inversion();
	}
}
bool Camera::ProjectToImage(int t,Vector<3> x,Vector<2> &point,double &z) {
	Vector<4> omo;
	Vector<3> tmp;

	omo[0]=x[0];omo[1]=x[1];omo[2]=x[2];omo[3]=1.0f;
	Multiply(P[t],omo,tmp.GetList());
	z=tmp[2];
	if (z<FLT_EPSILON) return false;

	point[0]=Approx(tmp[0]/z);
	point[1]=Approx(tmp[1]/z);

	if (point[0]>=width_) return false;
	if (point[0]<0) return false;
	if (point[1]>=height_) return false;
	if (point[1]<0) return false;

	return true;	
}
void Camera::UnProject(int t,Vector<2> point,double z,Vector<3> &x) {
	Vector<3> tmp;
	tmp[0]=point[0]*z;
	tmp[1]=point[1]*z;
	tmp[2]=z;

	tmp=tmp-PT[t];
	Multiply(IPR[t],tmp,x.GetList());
}
















//
//
// Validity
//
//
bool Camera::Valid(int t) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	return (validity[t]=='1');
}
bool Camera::Valid(int t0,int t1) {
	t0=min(max(0,t0),(num_frames_-1));
	t1=min(max(0,t1),(num_frames_-1));
	
	if (t0<=t1) {
		for(int i=t0;i<=t1;i++) {
			if (validity[i]=='0') return false;
		}
		return true;
	} else {
		for(int i=t0;i<num_frames_;i++) {
			if (validity[i]=='0') return false;
		}
		for(int i=0;i<=t1;i++) {
			if (validity[i]=='0') return false;
		}
		return true;
	}
}
int Camera::PickFirstValid(int t,int dir) {						// 0 both, 1 forward, -1 backward 
	if (dir>0) {
		for(int i=t;i<num_frames_;i++) {
			if (validity[i]=='1') return i;
		}
		for(int i=0;i<t;i++) {
			if (validity[i]=='1') return i;
		}
		ErrorExit("No valid calibrations");
		return t;
	}
	if (dir<0) {
		for(int i=t;i>=0;i--) {
			if (validity[i]=='1') return i;
		}
		for(int i=num_frames_-1;i>t;i--) {
			if (validity[i]=='1') return i;
		}
		ErrorExit("No valid calibrations");
		return t;
	}
	
	int f=PickFirstValid(t,1);
	int b=PickFirstValid(t,-1);
	
	if (ModuleDistance(f,t,num_frames_)>ModuleDistance(b,t,num_frames_)) return b;
	else return f;
}














//
//
// Geometric parameters
//
//
int Camera::width() {
	return width_;
}
int Camera::height() {
	return height_;
}
double Camera::aspect_ratio() {
	return ((1.0*width_)/height_);
}
int Camera::num_frames() {
	return num_frames_;
}
Matrix *Camera::Int(int t) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	return Int_[t];
}
Matrix *Camera::Ext(int t) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	return Ext_[t];
}
Vector<3> Camera::ProjectionCenter(int t) {
	Vector<3> c;
	Matrix E(3,4);

	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	ExttoE(Ext_[t],&E);
	E.GetColumn(3,c.GetList());

	return c;
}
Matrix *Camera::Ext_Original(int t) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	return Ext_Original_[t];
}
Matrix *Camera::P_Original(int t) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	return P[t];
}
void Camera::OGLState(int t,OpenGL_State *ogl,double clip_min,double clip_max) {
	if ((t<0) || (t>=num_frames_)) ErrorExit_int("Frame %i doesn't exist.",t);
	
	RenderWindow<3>::Retrieve_OGLState_From_Calibration(Int_[t],Ext_[t],width_,height_,ogl,clip_min,clip_max);
}













//
// Colors
//
Bitmap<ColorRGB> *Camera::Image() {
	return Image_;
}
inline void Camera::SetImage(Bitmap<ColorRGB> *img,bool delete_old) {
	if ((delete_old) && (Image_)) delete Image_;

	if (img!=NULL) {
		Image_=img;
		width_=Image_->width;
		height_=Image_->height;
	} else Warning("Image does not exist.");
}
Bitmap<GreyLevel> *Camera::GetChannel(int channel) {
	if (channel==0) ErrorExit("Problems");
	if (channel==1) return Mask_;
	if (channel==2) return Silhouette_;
	if (channel==3) return MaskAux_;

	if (channel>=CAMERA_N_STATIC_CHANNELS) return Channels_[channel-CAMERA_N_STATIC_CHANNELS];
	return NULL;
}
inline void Camera::SetMask(Bitmap<GreyLevel> *msk,bool delete_old) {
	if ((delete_old) && (Mask_)) delete Mask_;
	
	if (msk!=NULL) Mask_=msk;
	else Warning("Mask does not exist.");
}
inline void Camera::SetSilhouette(Bitmap<GreyLevel> *sil,bool delete_old) {
	if ((delete_old) && (Silhouette_)) delete Silhouette_;
	
	if (sil!=NULL) Silhouette_=sil;
	else Warning("Silhouette does not exist.");
}
inline void Camera::SetMaskAux(Bitmap<GreyLevel> *msk,bool delete_old) {
	if ((delete_old) && (MaskAux_)) delete MaskAux_;
	
	if (msk!=NULL) MaskAux_=msk;
	else Warning("Mask aux does not exist.");
}
void Camera::SetChannel(int channel,Bitmap<GreyLevel> *image,bool delete_old) {
	if (channel==0) ErrorExit("Problems");
	if (channel==1) SetMask(image,delete_old);
	if (channel==2) SetSilhouette(image,delete_old);
	if (channel==3) SetMaskAux(image,delete_old);
	if (channel>=CAMERA_N_STATIC_CHANNELS) {
		if ((delete_old) && (Channels_[channel-CAMERA_N_STATIC_CHANNELS])) delete Channels_[channel-CAMERA_N_STATIC_CHANNELS];

		if (image!=NULL) Channels_[channel-CAMERA_N_STATIC_CHANNELS]=image;
		else Warning_int("Channel #%i doesn't exist.",(channel+1));
	}
}





































Vector<3> Triangulation(int num_cam,Camera **Cameras,int time,Vector<2> *p) {

	Matrix A(3*num_cam,4);

	for(int i=0;i<num_cam;i++) {
		Matrix tmp33(3,3);
		CrossMatrix(p[i],&tmp33);
	

		Matrix *I=Cameras[i]->Int(time);
		Matrix *E=Cameras[i]->Ext_Original(time);
		
		Matrix tmp34(3,4);
		Multiply(I,E,&tmp34);
		PreMultiply(&tmp33,&tmp34);

		Vector<4> tmpline;
		A.SetRow(3*i  ,tmp34.GetRow(0,tmpline.GetList()));
		A.SetRow(3*i+1,tmp34.GetRow(1,tmpline.GetList()));
		A.SetRow(3*i+2,tmp34.GetRow(2,tmpline.GetList()));
	}

	Matrix U(3*num_cam,4),V(4,4),D(4,4);
	SVD(&A,&U,&D,&V);

	Vector<4> P;
	V.GetColumn(3,P.GetList());

	Vector<3> P_;
	P_[0]=P[0]/P[3];
	P_[1]=P[1]/P[3];
	P_[2]=P[2]/P[3];

	return P_;
}





























