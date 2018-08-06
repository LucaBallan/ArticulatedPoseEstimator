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





#include "stdInclude.h"
#include "common.h"
#include "StdLib/IObuffer.h"
#include "StdLib/Word.h"
#include "StdLib/stack.h"
#include "Imaging/image.h"
#include "StdLib/ListEntities.h"
#include "StdLib/List.h"
#include "StdLib/MathAddOn.h"
#include "StdLib/IOAddOn.h"
#include "StdLib/Tree.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Space.h"
#include "Math/HyperMesh.h"
#include "Math/OcTree.h"
#include "Imaging/KdTree.h"
#include "Imaging/Contour.h"
#include "Imaging/ImageUtils.h"
#include "Math/Math_libs.h"
#include "StdLib/ObjectContainer.h"
#include "StdLib/LargeDataUtils.h"
#include "Math/MultiDimensionalVector.h"
#include "StdLib/Processes.h"
#include "Math/interpolation.h"
#include "Math/MeshProcessing.h"
#include "Math/Arithmetic.h"


// Opengl										// opengl32.lib 
#ifdef _WIN64
#include "gl\glut64.h"							// glut64.lib 
#else
#include "gl\glut.h"							// glut32.lib 
#endif
#include "Imaging/RenderWindow.h"				// glew32.lib
#include "Imaging/Shader.h"
#include "Imaging/ActiveImageRegion.h"

#include "GL/glh_extensions.h"
#include "Imaging/Textel/Mapping.h"
#include "Imaging/Renderers.h"

// OpenCV
#include "Imaging/OpenCVWrapper.h"
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#include "Imaging/Video.h"
#include "Imaging/KLT.h"
#include "Imaging/EdgeDetector.h"


// MediaPlayer
#include "StdLib/MediaPlayer.h"

// Skull
#include "Math/Bone.h"
#include "Math/Skinned_Mesh.h"

// Wavelet
#include "Math/Wavelet.h"
#include "Math/Blend.h"



// 3D
#include "Math/Features.h"
#include "Math/View.h"
#include "Imaging/SoftZBuffer.h"


// External 
#include "Math/ExternalEntities.h"



const Vector<2> ZERO_2;
const Vector<3> ZERO_3;

#ifdef MAXMIN_REDEF
	#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
	#endif
	#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
	#endif
#endif


//
// Init Lib Functions
//
void InitGammaLib(char *app_title, int argc = 0, char* argv[] = NULL);



// Enable Warning
#pragma warning(3:4244)
