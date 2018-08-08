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





#pragma warning(3:4244)
#include "lib.h"
#pragma warning(3:4244)
using namespace std;


// Template Code
#include "StdLib/List.hpp"
#include "Math/Vector.hpp"
#include "Math/HyperMesh.hpp"
#include "Imaging/KdTree.hpp"
#include "Math/OcTree.hpp"
#include "Math/SpaceAddOn.hpp"
#include "StdLib/LargeDataUtils.hpp"



#define CREATE_SPACE(Dimension)																						\
				template class Vector<Dimension>;																	\
				template class Space<Dimension>;																	\
				template const Vector<Dimension> operator*(const double &a, const Vector<Dimension> &p);			\
				template void Multiply(Matrix *A,Vector<Dimension> b,Element_Type *c);								\
				template Vector<Dimension> Max(Vector<Dimension> a,Vector<Dimension> b);							\
				template Vector<Dimension> Min(Vector<Dimension> a,Vector<Dimension> b);							\
				template OFileBuffer &operator<<(OFileBuffer &os, const Vector<Dimension> &p);						\
				template STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os, const Vector<Dimension> &p);				\
				template IFileBuffer &operator>>(IFileBuffer &os, Vector<Dimension> &p);


#define CREATE_ADV_SPACE(Dimension)											                           \
				template class HyperFace<Dimension>;						                           \
				template class HyperBox<Dimension>;							                           \
				template class HyperMesh<Dimension>;						                           \
				template class kdTree_Node<Dimension>;				                                   \
				template kdTree_Node<Dimension> *Build(Array<Vector<Dimension>> *P);				   \
				template class OcTree_Node<Dimension>;												   \
				template class List<OcTree_Node<Dimension> *>;										   \
				template class Array<OcTree_Node<Dimension> *>;										   


#define OUTPUT_TXT(type)																	\
				template OFileBuffer &operator<<(OFileBuffer &os,type &v);					\
				template IFileBuffer &operator>>(IFileBuffer &os,type &v);					\
				template STANDARD_OUTPUT &operator<<(STANDARD_OUTPUT &os,type &v);



#define LISTS(type)											                                \
				template class List<type>;													\
				template class Array<type>;													\
				OUTPUT_TXT(Array<type>)														
				

				





// Spaces
CREATE_SPACE(2)
	CREATE_ADV_SPACE(2)
CREATE_SPACE(3)
	CREATE_ADV_SPACE(3)
CREATE_SPACE(4)


typedef Vector<2> Point2d;
typedef Vector<3> Point3d;
typedef Vector<4> Point4d;





// Lists
LISTS(BYTE)
LISTS(int)
LISTS(UINT)
LISTS(double)
LISTS(Point2d)
LISTS(Point3d)
LISTS(Point4d)
LISTS(ordered_pair)
LISTS(weighted_index)
LISTS(Pointer)
LISTS(ObjectIndex_couple)
LISTS(Task)
LISTS(RepetitiveTask)
LISTS(RepetitiveTaskClassInfo)
LISTS(Bone *)
LISTS(Tree *)
LISTS(feat_element_type *)
LISTS(TargetList_1D_Item_type)
LISTS(TargetList_2D_Item_type)
LISTS(TargetList_3D_Item_type)


template class DoubleList<int>;
template class DoubleList<Coupled_Face>;





#include "Math/ExternalEntities.hpp"




