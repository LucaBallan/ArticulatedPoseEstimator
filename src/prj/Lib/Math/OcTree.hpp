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





// Private Stack
Stack stack(MAX_LEVELS);		// TODO: metterlo dentro classe e dimensione dinamica


template <int dim>
OcTree_Node<dim>::OcTree_Node(OcTree_Node<dim> *Father) : Tree((1<<dim),Father) {
	// Init Center=0, d=0
}


template <int dim>
OcTree_Node<dim>::~OcTree_Node() {
// TODO: SICURO CHE SI DISTUGGE???
}

template <int dim>
OcTree_Node<dim>::OcTree_Node(Vector<dim> d) : Tree((1<<dim),NULL) {
	// Init Center=0
	this->d=d;
}

template <int dim>
OcTree_Node<dim>::OcTree_Node(char *filename) : Tree((1<<dim),NULL) {
	// Init Center=0

	// Init d e resto
	Load(filename);
}




template <int dim> 
OcTree_Node<dim> *OcTree_Node<dim>::CreateNode(OcTree_Node<dim> *Father,int type) {
	// this non e' attendibile


	OcTree_Node<dim> *o=new OcTree_Node<dim>(Father);

	o->d=Father->d*0.5;
	o->Center=Father->Center;
	
	for(int j=0;j<dim;j++) {
		if (Bit_Read(type,j)==1) o->Center[j]+=d[j]*0.5;
		else o->Center[j]-=d[j]*0.5;
	}

	return o;
}


Voxel3D *Voxel3D::CreateNode(Voxel3D *Father,int type) {
	// this non e' attendibile

	Voxel3D *o=new Voxel3D(Father);

	o->d=Father->d*0.5;
	o->Center=Father->Center;
	
	for(int j=0;j<3;j++) {
		if (Bit_Read(type,j)==1) o->Center[j]+=d[j]*0.5;
		else o->Center[j]-=d[j]*0.5;
	}

	return o;
}










//***************************************************************************************



// Type:
//     TREE_INVALID_NODE -> Errore nella struttura o il nodo e' root
//     int x
//
// x:
//       LSB
//       0123456789... dimensione
//           |       
//           x(i)
//
//       per ogni x(i)
//           x(i)=1 se  > a dx nella dimensione i-esima rispetto al centro(i) del padre
//           x(i)=0 se <= a sx nella dimensione i-esima rispetto al centro(i) del padre
//
//




// d[i]= 0.5 * lunghezza dimensione i
// Ogni nodo si estende tra [d,-d]+centro


// Suppongo che x sia interno all'hyperrettangolo di centro Center e dimensione d
template <int dim> 
OcTree_Node<dim> *OcTree_Node<dim>::NodeFromPoint(Vector<dim> x,Vector<dim> *Errore) {
	
	if (IsLeaf()) {
		if (Errore!=NULL) (*Errore)=x-Center;
		return this;
	}
	
	unsigned int Figlio=0;
	for(int i=0;i<dim;i++) {
		if (x[i]>Center[i]) Figlio|=(1<<i);
	}

	return (dynamic_cast <OcTree_Node<dim> *>(Child[Figlio]))->NodeFromPoint(x,Errore);
}


// Suppongo che x sia interno all'hyperrettangolo di centro Center e dimensione d
template <int dim> 
OcTree_Node<dim> *OcTree_Node<dim>::CreateNodeFromPoint(Vector<dim> x,int Level) {
	
	OcTree_Node<dim> *nodo=NodeFromPoint(x);
	if (nodo->Level>=Level) return nodo;

	nodo->ExpanNode();
	return nodo->CreateNodeFromPoint(x,Level);
}









//***************************************************************************************
// Se nn si ha a che fare con nodi vicini all'origine e' piu' veloce, con la ricerca node_from_point
// confrontare.. Si deve fermare al livello = a quello dell'origine...
template <int dim> 
OcTree_Node<dim> *OcTree_Node<dim>::GetNeighbour(int move_dim,int move_dir) {

	int pos;
	OcTree_Node<dim> *current_node=this;
	stack.empty();


	// Ascend
	while(true) {
		if (current_node->Father==NULL) return NULL;
		pos=current_node->GetType();
		
		if (move_dir==1) {
			if (Bit_Read(pos,move_dim)==0) {
				current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Father->Child[Bit_Set(pos,move_dim,1)]));
				break;
			}
		}
		if (move_dir==0) {
			if (Bit_Read(pos,move_dim)==1) {
				current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Father->Child[Bit_Set(pos,move_dim,0)]));
				break;
			}
		}
		
		stack.push(pos);
		current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Father));
	}

	while (!stack.isEmpty()) {
		pos=stack.pop();
		if (move_dir==0) pos=Bit_Set(pos,move_dim,1);
		else pos=Bit_Set(pos,move_dim,0);
		if (current_node->IsLeaf()) return current_node;
		current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Child[pos]));
	}
	return current_node;
}

//***************************************************************************************
// La prima chiamata va con root Assoluta... nn sono ammessi i sottoalberi
// Enumera le foglie. Finisce con NULL

// Caso particolare una root senza figli -> ritorna subito NULL. nn enumera root


template <int dim>
OcTree_Node<dim> *OcTree_Node<dim>::Get_Next_Enum_Leaf() {
	
	int pos;
	OcTree_Node<dim> *current_node=this;


	if (current_node->Father!=NULL) {   // se non e' la prima chiamata
		
		// Ascend
		while(true) {
			if (current_node->Father==NULL) {
				// nn c'e' niente piu' a dx -> li ho passati tutti
				return NULL;
			}
			
			pos=current_node->GetType();
			pos++;

			if (pos<(1<<dim)) {
				// il suo vicino a dx esiste -> mi sposto per di li e inizio la discesa
				current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Father->Child[pos]));
				break;
			}

			// Altrimenti salgo ancora
			current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Father));
		}

	} else {
		// La prima chiamata
		if (current_node->IsLeaf()) return NULL;  // evita un loop
	}
	
	while(!current_node->IsLeaf()) {
		current_node=(dynamic_cast <OcTree_Node<dim> *> (current_node->Child[0]));
	}
	
	return current_node;
}






//***************************************************************************************
// Va chiamata con root Assoluta... nn sono ammessi i sottoalberi
template <int dim> 
void OcTree_Node<dim>::Save(char *filename) {

	/*OFileBuffer file(filename);

	file<<d<<"\r\n";

	OcTree_Node<dim> *L=Get_Next_Enum_Leaf();
	while (L!=NULL) {
		
		// Salva la foglia
		
		file<<L->Center<<"\r\n";
		file<<L->Level<<"\r\n";
		file<<L->f<<"\r\n";
		file<<L->Grad<<"\r\n";
		file<<L->F<<"\r\n";
		file<<L->F_t<<"\r\n";
		
		L=L->Get_Next_Enum_Leaf();
	}

	file.Close();*/
}


template <int dim> 
bool OcTree_Node<dim>::Load(char *filename) {
	
	double tmpdbl;
	Vector<dim> tmp;

	int level;
	Vector<dim> Point;
	OcTree_Node<dim> *o;

	IFileBuffer file(filename);

	
	if (file.IsFinished()) return false;
	
	file>>d;

	while (!file.IsFinished()) {
		file>>Point;
		file>>level;

		o=CreateNodeFromPoint(Point,level);
		
		file>>tmpdbl;
		file>>tmp;
		file>>tmp;
		file>>tmp;
	}

	return true;
}




bool Voxel3D::Load(char *filename) {
	
	double tmpdbl;
	Vector<3> tmp;

	int level;
	Vector<3> Point;
	Voxel3D *o;

	IFileBuffer file(filename);
	
	if (file.IsFinished()) return false;
	
	file>>d;

	while (!file.IsFinished()) {
		file>>Point;
		file>>level;

		o=dynamic_cast<Voxel3D *>(CreateNodeFromPoint(Point,level));
		
		file>>tmpdbl;
		if (tmpdbl>0) o->empty=false;
		file>>tmp;
		file>>tmp;
		file>>tmp;
	}

	return true;
}



Vector<3> Voxel3D::GetVertexCoords(int type) {
	
	Vector<3> Coords=Center;
	
	for(int j=0;j<3;j++) {
		if (Bit_Read(type,j)==1) Coords[j]+=d[j];
		else Coords[j]-=d[j];
	}
	
	return Coords;
}





/*
Array<void *> Neighbours(100);
int common_level;
double d_min;
#define INT_MAX 0x7FFFFF;

template <int dim> 
const Array<void *> *OcTree_Node<dim>::GetNeighbours(int move_dim,int move_dir) {
	Neighbours.clear();
	
	OcTree_Node<dim> *SubTree=GetNeighbour(move_dim,move_dir);
	if (SubTree==NULL) return &Neighbours;

	if (move_dir) move_dir=0;
	else move_dir=1;

	common_level=0;// INT_MAX
	SubTree->RecGetNeighbours(move_dim,move_dir);
	
	// Normalize level
		
	return &Neighbours;
}

template <int dim> 
void OcTree_Node<dim>::RecGetNeighbours(int move_dim,int move_dir) {
	if (IsLeaf()) {										// POTREI BLOCCARE IL LIVELLO GIA' QUI PER VELOCIZZARE
		if (common_level<Level) common_level=Level;
		if (d_min>d[move_dim]) d_min=d[move_dim];
		Neighbours.append(this);
		return;
	}

	for(int i=0;i<(1<<dim);i++) {
		if (Bit_Read(i,move_dim)==move_dir) Child[i]->RecGetNeighbours(move_dim,move_dir);
	}

	return;
}

*/






/*
template <int dim>
void OcTree_Node<dim>::f_edge(int move_dim,int move_dir,double &value,double &d_edge) {
	double Interp;

	GetNeighbours(move_dim,move_dir);
	
	if (Neighbours.numElements()==0) {
		value=f;
		d_edge=0;
		return;
	} 
	if (Neighbours.numElements()==1) {
		//uno vicino di dimensioni uguali o maggiori
		d_edge=d_min+d[move_dim];
		
		// Sicuro?
		//distNeig=(((OcTree_Node<dim> *)(Neighbours[0]))->d[move_dim]);
		//value=(((OcTree_Node<dim> *)(Neighbours[0]))->f);
		//value=LinearInterpolation(f,value,-d[move_dim],distNeig,d_min);
//		OcTree_Node<dim> *Vicino;
		OcTree_Node<dim> *bigger=(OcTree_Node<dim> *)(Neighbours[0]);
		Vector<dim> S=bigger->Center-Center;

		double Interp_value=bigger->f;
		
		
		value=Interp_value;
		return;
	}
	
	// Piu' vicini piccoli sull'edge
	value=0;
	for(int i=0;i<Neighbours.numElements();i++) {
		Interp=LinearInterpolation(f,-d[move_dim],(((OcTree_Node<dim> *)(Neighbours[i]))->f),(((OcTree_Node<dim> *)(Neighbours[i]))->d[move_dim]),d_min);
		value+=Interp*2*((OcTree_Node<dim> *)(Neighbours[i]))->d[move_dim];
	}
	value*=(1.0/(d[move_dim]*2));
	d_edge=d_min+d[move_dim];
}


template <int dim>
void OcTree_Node<dim>::F_edge(int move_dim,int move_dir,Vector<dim> &value,double &d_edge) {
	Vector<dim> Interp;

	GetNeighbours(move_dim,move_dir);
	
	if (Neighbours.numElements()==0) {
		value=F;
		d_edge=0;
		return;
	} 
	if (Neighbours.numElements()==1) {
		// uno vicino di dimensioni uguali o maggiori
		d_edge=d_min+d[move_dim];
		// Interpolazione??
		value=(((OcTree_Node<dim> *)(Neighbours[0]))->F);
		return;
	}
	
	for(int i=0;i<dim;i++) value[i]=0;
	for(int i=0;i<Neighbours.numElements();i++) {
		Interp=(((OcTree_Node<dim> *)(Neighbours[i]))->F);
		value+=Interp*2*((OcTree_Node<dim> *)(Neighbours[i]))->d[move_dim];
	}
	value*=(1.0/(d[move_dim]*2));
	d_edge=d_min+d[move_dim];
}
*/
