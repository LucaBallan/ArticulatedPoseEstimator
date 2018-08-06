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





#pragma once

#include <tchar.h>
#include <float.h>
#include <crtdbg.h>

// Common Includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <assert.h>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <exception>
#include <map>
#include <list>
#include <queue>
#include <limits>


// newmat
#include "newmat.h"
#include "newmatap.h"

#include "stdInclude.h"
#include "common.h"
#include "Math/Matrix.h"
#include "Math_libs.h"
#pragma warning(3:4244)

void SVD(Matrix *A,Matrix *U,Matrix *D,Matrix *V) {
	NEWMAT::Matrix A_(A->r,A->c);
	NEWMAT::DiagonalMatrix D_(A->c);
	NEWMAT::Matrix U_(A->r,A->c);
	NEWMAT::Matrix V_(A->c,A->c);
	
	for(int i=0;i<A->r;i++)
		for(int j=0;j<A->c;j++) A_[i][j]=(*A)[i][j];
	
	NEWMAT::SVD(A_,D_,U_,V_);

	for(int i=0;i<A->r;i++)
		for(int j=0;j<A->c;j++) (*U)[i][j]=U_[i][j];

	for(int i=0;i<A->c;i++)
		for(int j=0;j<A->c;j++) (*V)[i][j]=V_[i][j];

	D->SetZero();
	for(int i=0;i<A->c;i++) (*D)[i][i]=D_[i];

}

double double_eps(double X) {
	X=fabs(X);
	
	if (X<DBL_MIN) {
		return ldexp((double)1.0,-1074);
	}
	if (X>DBL_MAX) {
		return numeric_limits<double>::signaling_NaN();
	}

	int e=(int)floor(log10((double)X)/log10((double)2.0));
	if (e>1024-1) e=1024-1;

	return ldexp((double)1.0,e-52);
}

int PseudoInversa(Matrix *A,Matrix *I) {
	int rango;
	NEWMAT::Matrix A_(A->r,A->c);
	NEWMAT::DiagonalMatrix D_(A->c);
	NEWMAT::Matrix U_(A->r,A->c);
	NEWMAT::Matrix V_(A->c,A->c);

	// Caso Matrice Vuota
	if ((A->r*A->c)==0) ErrorExit("Empty matrix.");
	
	// Caso colonne>righe    (Basta passare la trasposta)
	if (A->c>A->r) ErrorExit("Matrix row>=coloum condition is not satisfied. Use the transpose of the matrix instead of the matrix itself.");

	for(int i=0;i<A->r;i++)
		for(int j=0;j<A->c;j++) A_[i][j]=(*A)[i][j];
	
	NEWMAT::SVD(A_,D_,U_,V_);

	
	// Computate the Tollerance (matlab style)
	double maxD_=0.0;
	for(int i=0;i<A->c;i++) 
		if (D_[i]>maxD_) maxD_=D_[i];
	
	double tol=max(A->r,A->c)*double_eps(maxD_);

	// Calcolo D_+
	rango=0;
	for(int i=0;i<A->c;i++) {
		if (D_[i]>tol) {
			D_[i]=1/D_[i];
			rango++;
		} else D_[i]=0.0;
	}
		
	NEWMAT::Matrix B_(A->c,A->r);
	B_=V_*D_*U_.t();
	
	for(int i=0;i<A->c;i++)
		for(int j=0;j<A->r;j++) (*I)[i][j]=B_[i][j];
	
	return rango;
}

void SolveRBA(Matrix *A,Matrix *B,Matrix *R) {
	int num_c=A->c;
	Assert((A->r==3)&&(B->r==3)&&(num_c==B->c)&&(R->r==R->c)&&(R->r==3),"Dimensioni delle matrici non corrette.");

	// Solve |A-RB|
	Matrix AT(num_c,3);
	A->Traspose(&AT);
	Multiply(B,&AT,R);			// R=B*(A^T)


	Matrix U(3,3),V(3,3),D(3,3);
	SVD(R,&U,&D,&V);			// B*(A^T)=UD(V^T)
	
	U.Traspose();
	Multiply(&V,&U,R);			// R=V*(U^T)
	double r=R->Determinante(); // det(V*(U^T))

	R->SetIdentity();
	(*R)[2][2]=r;				// R=diagonal(1,1,det(V*(U^T)))

	PreMultiply(&V,R);			// R=V*R
	Multiply(R,&U);				// R=R*(U^T)
}



int SolveAXB(Matrix *A,Matrix *B,Matrix *X,Matrix *pinvA) {
	int return_state=0;


// Calcolo delle variabili libere
	int effective_unknowns=0;
	bool *Valid_Variable=new bool[A->c];
	for(int j=0;j<A->c;j++) {
		bool Valid=false;
		for(int i=0;i<A->r;i++) {
			if (!IS_ZERO(((*A)[i][j]),SOLVE_ZERO_THRESHOLD)) {
				Valid=true;
				break;
			}
		}
		if (Valid) {
			Valid_Variable[j]=true;
			effective_unknowns++;
		} else Valid_Variable[j]=false;
	}
	if (effective_unknowns<A->c) return_state+=SOLVE_WARNING_FREEVARIABLES;

	
	
// effective_unknowns>Rango?                       (usando A->r>=Rango)
	if (effective_unknowns>A->r) {
		// Sicuramente UnderDeterminated (ci sono piu' incognite che equazioni)
		return_state+=SOLVE_ERROR_UNDERDETERMINATED;
		delete[]Valid_Variable;
		return return_state;
	}

	
	
// Calcolo Rango e pinv
	bool pinv_to_delete=false;
	if (pinvA==NULL) {
		pinvA=new Matrix(A->c,A->r);
		pinv_to_delete=true;
	}
	int rango=PseudoInversa(A,pinvA);
	

// effective_unknowns==Rango?           (non puo' essere mai < se non per errori di precisione nel calcolo del rango)
	if (effective_unknowns<=rango) {
		if (effective_unknowns==A->r) return_state+=SOLVE_UNIQUESOLUTION;            // Sistema quadrato e determinante != 0 -> Una sola soluzione esiste
		else return_state+=SOLVE_OVERDETERMINED;                                     // Sistema OverDetermined: Settare il bit SOLVE_WARNING_NOT_CONSISTENT nel caso di inconsistenza
		Multiply(pinvA,B,X);

		// Assicuro a 0 le variabili libere  (possono essere !=0 per errori di approssimazione, normalmente di 10^-14)
		for(int j=0;j<A->c;j++) {
			if (!Valid_Variable[j]) {
				if ((*X)[j][0]!=0.0) {
					cout<<"Warning: correzione non dovrebbe esserci. Valore: "<<(*X)[j][0]<<"  (Controllare EPS!)\n";
					(*X)[j][0]=0.0;
				}			
			}
		}
	} else return_state+=SOLVE_ERROR_UNDERDETERMINATED;								// Sistema UnderDeterminated: Il numero di equazioni L.I. non è sufficiente a coprire le variabili.


// Delete	
	if (pinv_to_delete) delete pinvA;
	delete[]Valid_Variable;
	return return_state;
}



// TODO: Perche' SVD Diversa da quella trovata con matlab (di un errore alla 10^-14)???? cmq è accettabile...




/*
int Rank(Matrix *M) {
s = svd(A);
if nargin==1
   tol = max(size(A)') * eps(max(s));
end
r = sum(s > tol);
}
*/


