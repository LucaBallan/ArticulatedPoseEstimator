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






#include "lib.h"
#include "SalientPoints.h"

#define AVG_POINTS_PER_FRAME 10

SalientPoints::SalientPoints(char *filename,int n_frames,double threshold_confidence) {
	this->points=NULL;
	this->n_frames=n_frames;
	SNEWA_P(points,Array<Vector<2>>*,n_frames);

	Vector<2>     pt;
	int           frame_id;
	double        confidence; 
	IFileBuffer   in(filename);
	
	while(!in.IsFinished()) {
		in>>frame_id;
		in>>pt[0];
		in>>pt[1];
		in>>confidence; 
		if (confidence<threshold_confidence) continue;
		if (frame_id>=n_frames) continue;
		if (frame_id<0) continue;
		SNEW(points[frame_id],Array<Vector<2>>(AVG_POINTS_PER_FRAME));

		points[frame_id]->append(pt);
	}
}

SalientPoints::~SalientPoints() {
	SDELETEA_REC(points,n_frames);
}

Array<Vector<2>> *SalientPoints::get_tracks_read_only(int frame_id) {
	return points[frame_id];
}

double SalientPoints::find_closest_point(int frame_id,Vector<2> *pt,Vector<2> *c_pt) {
	if ((frame_id<0) || (frame_id>=n_frames)) return false;
	if (!(points[frame_id])) return false;
	
	double d_min=DBL_MAX;
	for(int i=0;i<points[frame_id]->numElements();i++) {
		double d=((*(points[frame_id]))[i]-(*pt)).Norm2();
		if (d<d_min) {
			d_min=d;
			*c_pt=(*(points[frame_id]))[i];
		}
	}
	return d_min;
}
