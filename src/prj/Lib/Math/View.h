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





#define VIEW_ENABLE_SILHOUETTE   1   // TODO: enable non e' un concetto generico
#define VIEW_ENABLE_OPTICAL_FLOW 2



class View;
struct TargetList_1D_Item_type {
	int       VertexIndex;
	View     *View;
	int       ProbabilityMapIndex;
	float     Target;
	float     sqrt_weight;
	int       type;					// 0 Depth
									// 1 ProbabilityMap

	bool operator== (const TargetList_1D_Item_type &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,TargetList_1D_Item_type &v);
IFileBuffer &operator>>(IFileBuffer &os,TargetList_1D_Item_type &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_1D_Item_type &v);

struct TargetList_2D_Item_type {
	int        VertexIndex;
	View      *View;
	Vector<2>  Source;
	Vector<2>  Target;
	float      sqrt_weight;
	int        type;					// 0 Silhouette
										// 1 Edge
										// 2 Optical Flow
										// 3 Salient Points

	bool operator== (const TargetList_2D_Item_type &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,TargetList_2D_Item_type &v);
IFileBuffer &operator>>(IFileBuffer &os,TargetList_2D_Item_type &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_2D_Item_type &v);

struct TargetList_3D_Item_type {
	int        VertexIndex;
	int        VertexIndex_Target;		// == -1 -> Target=P[VertexIndex] 
	Vector<3>  Target;					// != -1 -> Target=P[VertexIndex]-P[VertexIndex_Target]
	float      sqrt_weight;
	int        ttl;
	int        face_i,face_j;

	bool operator== (const TargetList_3D_Item_type &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,TargetList_3D_Item_type &v);
IFileBuffer &operator>>(IFileBuffer &os,TargetList_3D_Item_type &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_3D_Item_type &v);


struct TargetList_2D_Item_type_Frame_Info {
	int VertexIndex;
	View *View;
	Vector<2> Source;
	Vector<2> Target;
	int frame;
	bool Weighted;

	bool operator== (const TargetList_2D_Item_type_Frame_Info &v) const;
};
OFileBuffer &operator<<(OFileBuffer &os,TargetList_2D_Item_type_Frame_Info &v);
IFileBuffer &operator>>(IFileBuffer &os,TargetList_2D_Item_type_Frame_Info &v);
STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,TargetList_2D_Item_type_Frame_Info &v);


#define VIEW_MAX_N_CHANNELS  50

class View {
	Matrix InvM;
	Vector<3> O;
public:	
	int Enable;

	// Gemetry
	int     width,height;												// 0 means NOT SET
	Matrix  Int;
	Matrix  Ext;
	float   kc[5];

	// Derivates (CreateUnProjectionMatrix)
	Matrix    P;
	Matrix    ExtOGL;
	Vector<3> PrjPlaneNormal;
	Vector<3> PrjCenter;

	
	// Channels 
	char ChannelTypes[VIEW_MAX_N_CHANNELS];								// C ColorRGB, G GreyLevel, F ColorFloat
	void SetChannelTypes(char *channel_types);
	void CreateChannel(int i);                                          // width and height must be set
	void DeleteChannel(int i);
	union {
		struct {
			Bitmap<ColorRGB>    *Img;
			Bitmap<GreyLevel>   *Mask;
			Bitmap<ColorFloat>  *Depth;
			Bitmap<GreyLevel>   *Edges;
		};
		Bitmap<ColorRGB>   *i_rgb    [VIEW_MAX_N_CHANNELS];
		Bitmap<GreyLevel>  *i_grey   [VIEW_MAX_N_CHANNELS];
		Bitmap<ColorFloat> *i_float  [VIEW_MAX_N_CHANNELS];
	};


	// Statistics
	Statistics *PDE;


	// Costructors
	View();
	virtual ~View();
	
	
	// Projection	
	void CreateProjectionMatrix();
		Vector<3> Trasform(Vector<3> x);	
		Vector<3> VectorTrasform(Vector<3> x);

		Vector<2> ProjectionToImage(Vector<3> x);				// Projection to pixel
		void      ProjectionToImage(Vector<3> x,int *p2D);		// Projection to pixel
		Vector<2> Projection(Vector<3> x);						// Projection to ImagePlane         (no integer approximation)
		Vector<3> ProjectionZ(Vector<3> x);						// Projection to ImagePlane with Z  (no integer approximation)
		Vector<2> VectorProjection(Vector<3> v,Vector<3> p);



	// GetNearFarPlane
	void GetNearFarPlane(HyperMesh<3> *mesh,double &min_z,double &max_z);			// Cropping considered. -> if not use RenderWindow function
																					// Remeber to check
																					//    a) mesh->num_p == 0  
																					//    b) min_z<0,      in this case the camera intersect the object
																					//    a) min_z>max_z,  no visible point

	// Unprojection (nn tiene conto della distorsione)	
	void CreateUnProjectionMatrix();					        // call after all view parameters are setted (also after CreateProjectionMatrix())
		Vector<3> UnProjection(Vector<3> x);					// x[0],x[1] are the image coordinates x[2] is the depth
		Vector<3> UnProjection(Vector<2> x);					// x[2] is assumed to be 1 (the point is on the image plane)
		Vector<3> UnTrasform(Vector<3> x);



};










class ViewStreamer {
	View  *view;
	union {
		Video<ColorRGB>   *src_rgb   [VIEW_MAX_N_CHANNELS];
		Video<GreyLevel>  *src_grey  [VIEW_MAX_N_CHANNELS];
		Video<ColorFloat> *src_float [VIEW_MAX_N_CHANNELS];
	};
public:
	ViewStreamer(View *view,char **ch_src);				// ch_src    = NULL terminated list of char*
	~ViewStreamer();									// ch_src[i] = file name for the channel i of obj view (of type view.ChannelTypes[i]) 
														//             "" indicates no file available
														// Note: -) view.ChannelTypes should be setted before calling this constructur
														//       -) the video must exists otherwise it exits with an error

	bool LoadChannels(int frame_index);					// Create a local copy of the images in view
														//       -) Load the images related to the existing streams
														//       -) Autocreate the view buffer if it does not exist
														//		 -) Error in case the resolution does not match
														//       -) If a stream cannot be loaded return false 
														//          -> the resulting image in view is the old one
};


/*
class MovingView:public View {
	Matrix **Int_;
	Matrix **Ext_;
	int num_frames;
	ViewStreamer *channel_src;
public:
	MovingView(char *CameraFile);
	virtual ~MovingView();

	void SetTime(int t);
	View *GetViewAtTime(int t);

};
MovingView::MovingView(char *CameraFile) {
	Int_=new Matrix*[]
	set_time(0);
}
virtual ~MovingView::MovingView() {
}
void MovingView::set_time(int frame_num) {
	if ((frame_num<0) || (frame_num>=num_frames)) ErrorExit("Frame does not exist.");
	Int.Set(Int_[frame_num]);
	Ext.Set(Ext_[frame_num]);
	
	CreateProjectionMatrix();
	CreateUnProjectionMatrix();
}
*/


#define CAMERA_N_STATIC_CHANNELS		4

class Camera {
	// Fixed parameters
	int width_, height_;
	int num_frames_;

	int n_channels;

	// Variable parameters
	Matrix **Int_;
	Matrix **Ext_;
	Matrix **Ext_Original_;
	char *validity;

	// Current Images
	Bitmap<ColorRGB>  *Image_;
	Bitmap<GreyLevel> *Mask_;
	Bitmap<GreyLevel> *MaskAux_;
	Bitmap<GreyLevel> *Silhouette_;
	Bitmap<GreyLevel> **Channels_;

	// Projections
	Matrix **P;
	Matrix **IPR;Vector<3> *PT;

	// Deprecated
	void SetMask      (Bitmap<GreyLevel> *msk,bool delete_old=true);
	void SetMaskAux   (Bitmap<GreyLevel> *msk,bool delete_old=true);
	void SetSilhouette(Bitmap<GreyLevel> *sil,bool delete_old=true);
public:	


	Camera(char *CameraFile,char *ValidityFile,int filter_dim,double oulier_detection_threshold);
	~Camera();

	//
	// Init
	// 
	void SetSize(int width_,int height_);
	void Filter(int filter_dim,double oulier_detection_threshold);
	void Save(char *filename);
	


	//
	// Colors (temporary disable)
	//
	Bitmap<GreyLevel>	*GetChannel(int channel);
	void				 SetChannel(int channel,Bitmap<GreyLevel> *image,bool delete_old=true);
	void				 SetImage(Bitmap<ColorRGB> *img,bool delete_old=true);
	Bitmap<ColorRGB> *   Image();




	//
	// Geometric parameters
	//
	int			 width();
	int			 height();
	double		 aspect_ratio();
	int			 num_frames();

	Matrix		*Int(int t);
	Matrix		*Ext(int t);						// Sono corrette per OpenGl
	Matrix		*Ext_Original(int t);
	Matrix		*P_Original(int t);
	Vector<3>	 ProjectionCenter(int t);			// Sono corrette per OpenGl
	void		 OGLState(int t,OpenGL_State *ogl,double clip_min=0,double clip_max=0);



	//
	// Validity
	//
	bool Valid(int t);
	bool Valid(int t0,int t1);
	int  PickFirstValid(int t,int dir);						// 0 both, 1 forward, -1 backward 
	


	
	//
	// Projections P and inv(P)
	//
	void InitProjections();
	bool ProjectToImage(int t,Vector<3> x,Vector<2> &point,double &z);
	void UnProject(int t,Vector<2> point,double z,Vector<3> &x);

};



Vector<3> Triangulation(int num_cam,Camera **Cameras,int time,Vector<2> *p);





/* useless
	// Channels
		#define CHANNEL(v,n,T)    (static_cast<Bitmap<T> *>((v).Channel[n]))
		#define CHANNELP(vp,n,T)  (static_cast<Bitmap<T> *>((vp)->Channel[n]))
		#define CHANNEL_(v,n)     ((v).Channel[n])
		#define CHANNELP_(vp,n)   ((vp)->Channel[n])
		// Default channels
		#define IMG(v)            CHANNEL(v,0,ColorRGB)
		#define IMGP(vp)          CHANNELP(vp,0,ColorRGB)
		#define IMG_(v)           CHANNEL_(v,0)
		#define IMGP_(vp)         CHANNELP_(vp,0)
		#define MASK(v)           CHANNEL(v,1,GreyLevel)
		#define MASKP(vp)         CHANNELP(vp,1,GreyLevel)
		#define MASK_(v)          CHANNEL_(v,1)
		#define MASKP_(vp)        CHANNELP_(vp,1)
*/