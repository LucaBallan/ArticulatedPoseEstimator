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

ActiveImageRegion::ActiveImageRegion() {
	ImageRegion=NULL;
	enable=NULL;
	Status=NULL;
	n_regions=0;
}
ActiveImageRegion::~ActiveImageRegion() {
	if (ImageRegion) delete []ImageRegion;
	if (enable) delete []enable;
	if (Status) delete []Status;
}
void ActiveImageRegion::SetNumberOfRegions(int n_regions) {
	if ((ImageRegion) || (enable)) ErrorExit("Changing the number of regions is not allowed.");

	this->n_regions=n_regions;
	ImageRegion=new HyperBox<2>[n_regions];
	enable=new bool[n_regions];
	Status=new bool[n_regions];

	for(int i=0;i<n_regions;i++) enable[i]=false;
}
void ActiveImageRegion::SetRegion(int i,HyperBox<2> *image_region,bool enable) {
	if ((i>=n_regions) || (i<0)) ErrorExit("This region does not exist.");
	ImageRegion[i]=*image_region;
	this->enable[i]=enable;
}
void ActiveImageRegion::EnableRegion(int i,bool enable) {
	if ((i>=n_regions) || (i<0)) ErrorExit("This region does not exist.");
	this->enable[i]=enable;
}
int ActiveImageRegion::GetActive(double x,double y) {
	for(int i=0;i<n_regions;i++) {
		if (enable[i]) {
			double x_m=min(ImageRegion[i].P0[0],ImageRegion[i].P1[0]);
			double x_M=max(ImageRegion[i].P0[0],ImageRegion[i].P1[0]); 
			double y_m=min(ImageRegion[i].P0[1],ImageRegion[i].P1[1]);
			double y_M=max(ImageRegion[i].P0[1],ImageRegion[i].P1[1]);
			if ((x>=x_m) && (x<=x_M) && (y>=y_m) && (y<=y_M)) {
				return i;
			}
		}
	}
	return -1;
}
bool *ActiveImageRegion::GetActiveRegionStatus(double x,double y) {
	for(int i=0;i<n_regions;i++) {
		if (enable[i]) {
			double x_m=min(ImageRegion[i].P0[0],ImageRegion[i].P1[0]);
			double x_M=max(ImageRegion[i].P0[0],ImageRegion[i].P1[0]); 
			double y_m=min(ImageRegion[i].P0[1],ImageRegion[i].P1[1]);
			double y_M=max(ImageRegion[i].P0[1],ImageRegion[i].P1[1]);
			if ((x>=x_m) && (x<=x_M) && (y>=y_m) && (y<=y_M)) Status[i]=true;
			else Status[i]=false;
		} else Status[i]=false;
	}
	return Status;
}
