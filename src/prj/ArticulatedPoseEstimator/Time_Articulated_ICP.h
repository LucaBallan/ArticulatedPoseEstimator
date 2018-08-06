//
// Articulated Pose Estimator on videos
//
//    Copyright (C) 2005-2015 Luca Ballan <ballanlu@gmail.com> http://lucaballan.altervista.org/
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






class MultiFrameSkinnedMesh {
	Skinned_Mesh  *mesh;
	Skinned_Mesh  *MultiFrameMesh;
	int            window_size;
	int           *obj_face_index;

public:
	// not parallelizable
	MultiFrameSkinnedMesh(Skinned_Mesh *mesh,int window_size);
	~MultiFrameSkinnedMesh();

	void SetFrames(int start_frame);			// TODO**
	void RetrieveFrames(int start_frame);		// TODO**

	Skinned_Mesh  *GetMesh();
};



// Resto ok


class Time_Articulated_ICP : public Articulated_ICP {
	MultiFrameSkinnedMesh *MF_Mesh;
	int                    time_window_size;

	Skinned_Mesh * InitConstructor(Skinned_Mesh *mesh,int window_size);
	void           RetrieveObservations(int Level,int frame_index); // TODO

	


public: 
	Time_Articulated_ICP(Skinned_Mesh *mesh,Vector<3> RoomCenter,double RoomRadius,
						 Features **FeaturesList,View *Views,int num_views,
						 int window_size,
						 Degrees_of_Freedom_Table_Entry *Constraint_level0=NULL,Degrees_of_Freedom_Table_Entry *Constraint_level1=NULL,
						 int debug_level=2);

	//
	//  Note: NVIDIA goes in deadlock here if a task that was using this class still exists.
	//        Before deleting the Time_Articulated_ICP class, please delete the tasks.
	virtual ~Time_Articulated_ICP();




};




