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





// NB: Newmat must be linked. 
// Configure [] access
//           double type

double double_eps(double X);
void SVD(Matrix *A,Matrix *U,Matrix *D,Matrix *V);		// A(r,c), U(r,c), V(c,c), D(c,c) ->  A=UD(V^T)


int PseudoInversa(Matrix *A,Matrix *I);					// Return the rank of A



// Soglia di 0 per le componenti del sistema lineare di Solve
#define SOLVE_ZERO_THRESHOLD DBL_EPSILON


//
// Solve  Problema Procustiano Ortogonale
// Tale denominazione è attinta dalla mitologia greca. Damaste, o Procrustes, era un gigante che 
// infieriva sulle sue vittime costringendole a distendersi su un giaciglio di ferro e ad adeguare 
// alle dimensioni di questo la loro statura. Se la persona era più corta del letto le sue membra 
// venivano allungate a martellate, se era più lunga la parte eccedente veniva mozzata. In entrambi 
// i casi la vittima moriva. 
//
// min(|A-RB|^2) con R ortonormale e ||Frobenius
//
void SolveRBA(Matrix *A,Matrix *B,Matrix *R);




//
// Solve  AX = B  <-> min(|AX-B|^2)   || norma che tipo?
//
#define SOLVE_ERROR_UNDERDETERMINATED   0x1
#define SOLVE_WARNING_FREEVARIABLES     0x2
#define SOLVE_UNIQUESOLUTION            0x4
#define SOLVE_OVERDETERMINED            0x8

int SolveAXB(Matrix *A,Matrix *B,Matrix *X,Matrix *pinvA=NULL);
//
// SOLVE_ERROR_UNDERDETERMINATED   = SottoDeterminato: il problema esatto ha piu' di una soluzione (figurati quello non)
//                                   Non sono programmato per risolvere il problema. (le equazioni l.i. non sono sufficienti a coprire le effective_unknowns)
//                                   Questo caso comprende anche il sistema inconsistente (dato che eq l.d. possono avere B diversi)
//
// SOLVE_WARNING_FREEVARIABLES     = Esistono delle variabili non influenti, nella soluzione verranno settate a 0
//
// SOLVE_UNIQUESOLUTION            = Esiste ed è unica la soluzione al sistema (escludendo le variabili libere se SOLVE_WARNING_FREEVARIABLES)
//
//

//
// Caso "Sistema con dati esatti":
//   Le equazioni L.I. sono sufficienti a coprire le variabili e quelle L.D. sono consistenti con B.
//
//     - SOLVE_UNIQUESOLUTION: (OK) Il risultato è certo e unico.
//     - SOLVE_OVERDETERMINED: (OK) Il risultato è certo e unico. In questo caso puo' esserci un SOLVE_WARNING_NOT_CONSISTENT.
//   Errori che violano le ipotesi:
//     - SOLVE_ERROR_UNDERDETERMINATED: (ERRORE) Il numero di equazioni L.I. non è sufficiente a coprire le variabili.
//     - SOLVE_WARNING_NOT_CONSISTENT: (WARNING NON CONTROLLATO) Le equazioni L.D. non sono consistenti con B
//
// Caso "Sistema con dati misurati":
//   Le equazioni L.I. sono sufficienti a coprire le variabili mentre quelle L.D. possono essere o meno consistenti con B, la soluzione trovata sarà ai minimi quadrati.
//
//     - SOLVE_OVERDETERMINED: (OK) Il risultato ai minimi quadrati è ok.
//     - SOLVE_UNIQUESOLUTION: (OK) Il risultato ai minimi quadrati è ok. (caso particolare, siamo al limite dei dati, sarebbe preferibile aumentare le equazioni)
//   Errori che violano le ipotesi:
//     - SOLVE_ERROR_UNDERDETERMINATED:  (ERRORE) Il numero di equazioni L.I. non è sufficiente a coprire le variabili.
//


