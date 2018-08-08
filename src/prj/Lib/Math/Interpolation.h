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





double     R_Lin_Interp(double a,double b,double t,double t0=0.0,double t1=1.0);
double     R_Cos_Interp(double a,double b,double t,double t0=0.0,double t1=1.0);
double     R_Ripple_Interp(double V00,double V10,double t,double V05,double t0=0.0,double t1=0.0);
double     R_Poly_Interp(double V00,double V10,double t,double V05,double D00,double D05,double D10,double t0=0.0,double t1=0.0);
double     Cyclic_Lin_Interp(double a,double b,double t,double module,double t0=0.0,double t1=1.0);


Vector<3>  R3_Lin_Interp(Vector<3> *a,Vector<3> *b,double t,double t0=0.0,double t1=1.0);
Vector<2>  R2_Lin_Interp(Vector<2> *a,Vector<2> *b,double t,double t0=0.0,double t1=1.0);
Vector<3>  R3_Cyl_Interp(Vector<3> *a,Vector<3> *b,double t,Vector<3> *axis,Vector<3> *o,double t0=0.0,double t1=1.0);


void       SO3_Lin_Interp(Matrix *a,Matrix *b,double t,Matrix *c,double t0=0.0,double t1=1.0);


void       SE3_Lin_Interp(Matrix *a,Matrix *b,double t,Matrix *c,double t0=0.0,double t1=1.0);
void       SE3_Cyl_Interp(Matrix *a,Matrix *b,double t,Matrix *c,Vector<3> *axis,Vector<3> *o,double t0=0.0,double t1=1.0);
void       SE3_2DStabilize_Interp(Matrix *a,Matrix *b,double t,Matrix *c,Vector<3> *axis,Vector<3> *o,double t0=0.0,double t1=1.0);



// SE3 Filter
void	   FilterSE3(Matrix **M,int num_elements,int filter_dim,double *weights,double oulier_detection_threshold);		
																	// 
																	// filter_dim		 = odd   (min 3)
																	//					 = 0     non filtra
																	// weights			 = [..]  impone dei pesi appartenenti a [0,inf),  0.0 significa dato non considerato
																	//					 = NULL  ignorato
																	// oulier_detection  = soglia relativa a sigma (Es: 2.0)
																	//					 = 0.0    non applica il detector
																	//


void FilterSO3(Matrix **In,Matrix **Out,int num_elements,int filter_dim,double *weights);
void FilterSO2(double *angles,int num_elements,int filter_dim,double *weights);
