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


HyperMeshPartitionTree::HyperMeshPartitionTree(int num_child,HyperMesh<3> *mesh) : Tree(num_child,NULL) {
	this->mesh=mesh;
	this->FaceList=NULL;
}

HyperMeshPartitionTree::~HyperMeshPartitionTree() {
	SDELETE(FaceList);
}

HyperMeshPartitionTree *HyperMeshPartitionTree::Generate_rec(HyperMesh<3> *mesh,Array<int> *face_index_list,int level) {
	if (level==0) {
		HyperMeshPartitionTree *leaf=new HyperMeshPartitionTree(2,mesh);
		leaf->FaceList=new Array<int>(*face_index_list);
		return leaf;
	}
	
	HyperBox<3> box=mesh->BoundingBox(face_index_list);
	Vector<3>     d=(box.P0-box.P1).abs();

	double max=-Element_Type_Max;
	int max_index=0;
	for(int i=0;i<3;i++) MAX_I(d[i],max,i,max_index);

	double middle=(box.P0[max_index]+box.P1[max_index])*0.5;
	int num_ele=face_index_list->numElements();
	Array<int> face_index_list_above(max((int)(num_ele*0.60),1));
	Array<int> face_index_list_below(max((int)(num_ele*0.60),1));
	for(int i=0;i<num_ele;i++) {
		if (((mesh->Faces[(*face_index_list)[i]].Baricentro(&(mesh->Points)))[max_index])>middle) {
			face_index_list_above.append((*face_index_list)[i]);
		} else {
			face_index_list_below.append((*face_index_list)[i]);
		}
	}

	if ((face_index_list_above.numElements()==0) || (face_index_list_below.numElements()==0)) {
		HyperMeshPartitionTree *leaf=new HyperMeshPartitionTree(2,mesh);
		leaf->FaceList=new Array<int>(*face_index_list);
		return leaf;
	}

	HyperMeshPartitionTree *leaf_above=Generate_rec(mesh,&face_index_list_above,level-1);
	HyperMeshPartitionTree *leaf_below=Generate_rec(mesh,&face_index_list_below,level-1);

	HyperMeshPartitionTree *root=new HyperMeshPartitionTree(2,mesh);
	root->AddChild(leaf_above);
	root->AddChild(leaf_below);

	return root;
}

HyperMeshPartitionTree *HyperMeshPartitionTree::Generate(HyperMesh<3> *mesh,int levels) {
	Array<int> face_index_list(mesh->num_f);
	for(int i=0;i<mesh->num_f;i++) face_index_list.append(i);

	return Generate_rec(mesh,&face_index_list,levels);
}

HyperMeshPartitionTree *HyperMeshPartitionTree::Generate(HyperMesh<3> *mesh,int start_level_range,int end_level_range,bool *intersection_mask,float &fps,int &selected_level) {
	Array<ordered_pair> *intersecting_faces=new Array<ordered_pair>(mesh->num_f);

	int    n_levels=start_level_range;
	double last_timer_seconds=DBL_MAX;

	std::cout<<"Collision Detector: ";
	for(;n_levels<=end_level_range;n_levels++) {
		HyperMeshPartitionTree *collision_tree=HyperMeshPartitionTree::Generate(mesh,n_levels);
		intersecting_faces->clear();
		std::cout<<n_levels<<".. ";
		TIMER_INIT
		collision_tree->UpdateBoundingBoxes();
		collision_tree->SelfIntersection_BinaryTree_Skinned_Mesh(intersecting_faces,intersection_mask);
		TIMER_QUERY
		delete collision_tree;
		if (timer_seconds>last_timer_seconds) {
			n_levels--;
			break;
		}
		last_timer_seconds=timer_seconds;
	}
	delete intersecting_faces;
	fps=1.0f/((float)last_timer_seconds);

	selected_level=n_levels;
	std::cout<<"\n";
	return HyperMeshPartitionTree::Generate(mesh,n_levels);
}

void HyperMeshPartitionTree::UpdateBoundingBoxes() {
	if (FaceList!=NULL) {
		BoundingBox=mesh->BoundingBox(FaceList);
		return;
	}

	BoundingBox.P0.SetVal(Element_Type_Max);
	BoundingBox.P1.SetVal(-Element_Type_Max);
	for(int j=0;j<num_child;j++) {
		if (Child[j]!=NULL) {
			HyperMeshPartitionTree *c=static_cast<HyperMeshPartitionTree *>(Child[j]);
			c->UpdateBoundingBoxes();
			BoundingBox.Include(c->BoundingBox);
		}
	}
}


bool HyperMeshPartitionTree::Intersection(HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces) {
	if (A!=B) {
		// A!=B && intesect(A,B)==empty -> no intersection
		if (!A->BoundingBox.Intersects(B->BoundingBox)) return false;
	} else {
		if (A->FaceList!=NULL) {
			// A==B e foglia -> controllo le facce con se stesse (evitare di controllarle due volte x,y e y,x) (x,x non ammesso perche' faccie)
			int A_n=A->FaceList->numElements();
			bool intersection=false;

			#ifdef HM_PARTITION_TREE_USE_OPENMP
			#pragma omp parallel for 
			#endif
			for(int i=0;i<A_n;i++) {
				for(int j=i+1;j<A_n;j++) {
					bool inter_s=Intersection_Mesh(A->mesh,(*(A->FaceList))[i],(*(A->FaceList))[j]);
					if (inter_s) {
						ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(A->FaceList))[j];
						#ifdef HM_PARTITION_TREE_USE_OPENMP
						#pragma omp critical
						#endif
						{
						intersection=true;
						intersecting_faces->append(tmp);
						}
					}
				}
			}
			
			return intersection;
		}
	}

	if ((A->FaceList!=NULL) && (B->FaceList!=NULL)) {
		// A!=B && A,B foglie
		int A_n=A->FaceList->numElements();
		int B_n=B->FaceList->numElements();
		bool intersection=false;

		#ifdef HM_PARTITION_TREE_USE_OPENMP
		#pragma omp parallel for
		#endif
		for(int i=0;i<A_n;i++) {
			for(int j=0;j<B_n;j++) {
				bool inter_s=Intersection_Mesh(A->mesh,B->mesh,(*(A->FaceList))[i],(*(B->FaceList))[j]);
				if (inter_s) {
					ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(B->FaceList))[j];
					#ifdef HM_PARTITION_TREE_USE_OPENMP
					#pragma omp critical
					#endif
					{
					intersection=true;
					intersecting_faces->append(tmp);
					}
				}
			}
		}
		
		return intersection;
	}

	bool intersection=false;

	if (A->FaceList!=NULL) {
		// A!=B && A foglia
		for(int j=0;j<B->num_child;j++) {
			if (B->Child[j]!=NULL) {
				HyperMeshPartitionTree *c=static_cast<HyperMeshPartitionTree *>(B->Child[j]);
				intersection|=Intersection(A,c,intersecting_faces);
			}
		}
	} else {
		if (B->FaceList!=NULL) {
			// A!=B && B foglia
			for(int i=0;i<A->num_child;i++) {
				if (A->Child[i]!=NULL) {
					HyperMeshPartitionTree *c=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
					intersection|=Intersection(c,B,intersecting_faces);
				}
			}
		} else {
			if (A==B) {
				// A==B && A,B non foglie -> evita di controllare x,y e y,x (x,x ammesso perche' blocchi)
				for(int i=0;i<A->num_child;i++) {
					if (A->Child[i]!=NULL) {
						for(int j=i;j<A->num_child;j++) {
							if (A->Child[j]!=NULL) {
								HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
								HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(A->Child[j]);
								intersection|=Intersection(a,b,intersecting_faces);
							}
						}
					}
				}
			} else {
				// A!=B && A,B non foglie
				for(int i=0;i<A->num_child;i++) {
					if (A->Child[i]!=NULL) {
						for(int j=0;j<B->num_child;j++) {
							if (B->Child[j]!=NULL) {
								HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
								HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(B->Child[j]);
								intersection|=Intersection(a,b,intersecting_faces);
							}
						}
					}
				}
			}
		}
	}

	return intersection;
}






bool HyperMeshPartitionTree::Intersection_BinaryLeveledTree(HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces) {
	// A and B at the same level and A,B binary
	
	if (A!=B) {
		// A!=B && intesect(A,B)==empty -> no intersection
		if (!A->BoundingBox.Intersects(B->BoundingBox)) return false;
	} else {
		if (A->FaceList!=NULL) {
			// A==B e foglia -> controllo le facce con se stesse (evitare di controllarle due volte x,y e y,x) (x,x non ammesso perche' faccie)
			int A_n=A->FaceList->numElements();
			bool intersection=false;

			#ifdef HM_PARTITION_TREE_USE_OPENMP		
			#pragma omp parallel for 
			#endif
			for(int i=0;i<A_n;i++) {
				for(int j=i+1;j<A_n;j++) {
					bool inter_s=Intersection_Mesh(A->mesh,(*(A->FaceList))[i],(*(A->FaceList))[j]);
					if (inter_s) {
						ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(A->FaceList))[j];
						#ifdef HM_PARTITION_TREE_USE_OPENMP
						#pragma omp critical
						#endif
						{
						intersection=true;
						intersecting_faces->append(tmp);
						}
					}
				}
			}
			
			return intersection;
		}
	}

	if (A->FaceList!=NULL) {
		// A!=B && A,B foglie
		int A_n=A->FaceList->numElements();
		int B_n=B->FaceList->numElements();
		bool intersection=false;

		#ifdef HM_PARTITION_TREE_USE_OPENMP		
		#pragma omp parallel for 
		#endif
		for(int i=0;i<A_n;i++) {
			for(int j=0;j<B_n;j++) {
				bool inter_s=Intersection_Mesh(A->mesh,(*(A->FaceList))[i],(*(B->FaceList))[j]);
				if (inter_s) {
					ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(B->FaceList))[j];
					#ifdef HM_PARTITION_TREE_USE_OPENMP
					#pragma omp critical
					#endif
					{
					intersection=true;
					intersecting_faces->append(tmp);
					}
				}
			}
		}
		
		return intersection;
	}

	bool intersection=false;

	if (A==B) {
		// A==B && A,B non foglie -> evita di controllare x,y e y,x (x,x ammesso perche' blocchi)
		HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		intersection|=Intersection_BinaryLeveledTree(a,b,intersecting_faces);
		a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		intersection|=Intersection_BinaryLeveledTree(a,b,intersecting_faces);
		a=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		intersection|=Intersection_BinaryLeveledTree(a,b,intersecting_faces);
	} else {
		// A!=B && A,B non foglie
		for(int i=0;i<2;i++) {
			for(int j=0;j<2;j++) {
				HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
				HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(B->Child[j]);
				intersection|=Intersection_BinaryLeveledTree(a,b,intersecting_faces);
			}
		}
	}

	return intersection;
}




bool HyperMeshPartitionTree::SelfIntersection(Array<ordered_pair> *intersecting_faces) {
	return HyperMeshPartitionTree::Intersection(this,this,intersecting_faces);
}
bool HyperMeshPartitionTree::SelfIntersection_BinaryTree_Skinned_Mesh(Array<ordered_pair> *intersecting_faces,bool *intersection_mask) {
	return HyperMeshPartitionTree::Intersection_BinaryTree_Skinned_Mesh(this,this,intersecting_faces,intersection_mask);
}
bool HyperMeshPartitionTree::SelfIntersection_BinaryLeveledTree(Array<ordered_pair> *intersecting_faces) {
	return HyperMeshPartitionTree::Intersection_BinaryLeveledTree(this,this,intersecting_faces);
}
bool HyperMeshPartitionTree::SelfIntersection_BinaryLeveledTree_Skinned_Mesh(Array<ordered_pair> *intersecting_faces,bool *intersection_mask) {
	return HyperMeshPartitionTree::Intersection_BinaryLeveledTree_Skinned_Mesh(this,this,intersecting_faces,intersection_mask);
}




























bool HyperMeshPartitionTree::Intersection_BinaryLeveledTree_Skinned_Mesh(HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces,bool *intersection_mask) {
	// A and B at the same level and A,B binary
	
	if (A!=B) {
		// A!=B && intesect(A,B)==empty -> no intersection
		if (!A->BoundingBox.Intersects(B->BoundingBox)) return false;
	} else {
		if (A->FaceList!=NULL) {
			// A==B e foglia -> controllo le facce con se stesse (evitare di controllarle due volte x,y e y,x) (x,x non ammesso perche' faccie)
			Skinned_Mesh *sk_mesh=static_cast<Skinned_Mesh *>(A->mesh);
			int A_n=A->FaceList->numElements();
			bool intersection=false;

			#ifdef HM_PARTITION_TREE_USE_OPENMP		
			#pragma omp parallel for 
			#endif
			for(int i=0;i<A_n;i++) {
				for(int j=i+1;j<A_n;j++) {
					bool inter_s=Intersection_Mesh(sk_mesh,(*(A->FaceList))[i],(*(A->FaceList))[j],intersection_mask);
					if (inter_s) {
						ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(A->FaceList))[j];
						#ifdef HM_PARTITION_TREE_USE_OPENMP
						#pragma omp critical
						#endif
						{
						intersection=true;
						intersecting_faces->append(tmp);
						}
					}
				}
			}
			
			return intersection;
		}
	}

	if (A->FaceList!=NULL) {
		// A!=B && A,B foglie
		Skinned_Mesh *sk_mesh=static_cast<Skinned_Mesh *>(A->mesh);
		int A_n=A->FaceList->numElements();
		int B_n=B->FaceList->numElements();
		bool intersection=false;

		#ifdef HM_PARTITION_TREE_USE_OPENMP		
		#pragma omp parallel for 
		#endif
		for(int i=0;i<A_n;i++) {
			for(int j=0;j<B_n;j++) {
				bool inter_s=Intersection_Mesh(sk_mesh,(*(A->FaceList))[i],(*(B->FaceList))[j],intersection_mask);
				if (inter_s) {
					ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(B->FaceList))[j];
					#ifdef HM_PARTITION_TREE_USE_OPENMP
					#pragma omp critical
					#endif
					{
					intersection=true;
					intersecting_faces->append(tmp);
					}
				}
			}
		}
		
		return intersection;
	}

	bool intersection=false;

	if (A==B) {
		// A==B && A,B non foglie -> evita di controllare x,y e y,x (x,x ammesso perche' blocchi)
		HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		intersection|=Intersection_BinaryLeveledTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
		a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
		b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		intersection|=Intersection_BinaryLeveledTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
		a=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
		intersection|=Intersection_BinaryLeveledTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
	} else {
		// A!=B && A,B non foglie
		for(int i=0;i<2;i++) {
			for(int j=0;j<2;j++) {
				HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
				HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(B->Child[j]);
				intersection|=Intersection_BinaryLeveledTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
			}
		}
	}

	return intersection;
}


































bool HyperMeshPartitionTree::Intersection_BinaryTree_Skinned_Mesh(HyperMeshPartitionTree *A,HyperMeshPartitionTree *B,Array<ordered_pair> *intersecting_faces,bool *intersection_mask) {
	// A and B at the same level and A,B binary
	
	if (A!=B) {
		// A!=B && intesect(A,B)==empty -> no intersection
		if (!A->BoundingBox.Intersects(B->BoundingBox)) return false;
	} else {
		if (A->FaceList!=NULL) {
			// A==B e A,B foglia -> controllo le facce con se stesse (evitare di controllarle due volte x,y e y,x) (x,x non ammesso perche' faccie)
			Skinned_Mesh *sk_mesh=static_cast<Skinned_Mesh *>(A->mesh);
			int A_n=A->FaceList->numElements();
			bool intersection=false;

			#ifdef HM_PARTITION_TREE_USE_OPENMP		
			#pragma omp parallel for 
			#endif
			for(int i=0;i<A_n;i++) {
				for(int j=i+1;j<A_n;j++) {
					bool inter_s=Intersection_Mesh(sk_mesh,(*(A->FaceList))[i],(*(A->FaceList))[j],intersection_mask);
					if (inter_s) {
						ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(A->FaceList))[j];
						#ifdef HM_PARTITION_TREE_USE_OPENMP
						#pragma omp critical
						#endif
						{
						intersection=true;
						intersecting_faces->append(tmp);
						}
					}
				}
			}
			
			return intersection;
		}
	}

	if ((A->FaceList!=NULL) && (B->FaceList!=NULL)) {
		// A!=B && A,B foglie
		Skinned_Mesh *sk_mesh=static_cast<Skinned_Mesh *>(A->mesh);
		int A_n=A->FaceList->numElements();
		int B_n=B->FaceList->numElements();
		bool intersection=false;

		#ifdef HM_PARTITION_TREE_USE_OPENMP
		#pragma omp parallel for 
		#endif
		for(int i=0;i<A_n;i++) {
			for(int j=0;j<B_n;j++) {
				bool inter_s=Intersection_Mesh(sk_mesh,(*(A->FaceList))[i],(*(B->FaceList))[j],intersection_mask);
				if (inter_s) {
					ordered_pair tmp;tmp.i=(*(A->FaceList))[i];tmp.j=(*(B->FaceList))[j];
					#ifdef HM_PARTITION_TREE_USE_OPENMP
					#pragma omp critical
					#endif
					{
					intersection=true;
					intersecting_faces->append(tmp);
					}
				}
			}
		}
		
		return intersection;
	}

	bool intersection=false;


	if (A->FaceList!=NULL) {
		// A!=B && A foglia && B non foglia
		HyperMeshPartitionTree *c;
		
		c=static_cast<HyperMeshPartitionTree *>(B->Child[0]);
		intersection|=Intersection_BinaryTree_Skinned_Mesh(A,c,intersecting_faces,intersection_mask);
		c=static_cast<HyperMeshPartitionTree *>(B->Child[1]);
		intersection|=Intersection_BinaryTree_Skinned_Mesh(A,c,intersecting_faces,intersection_mask);
	} else {
		if (B->FaceList!=NULL) {
			// A!=B && B foglia && A non foglia
			HyperMeshPartitionTree *c;

			c=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
			intersection|=Intersection_BinaryTree_Skinned_Mesh(c,B,intersecting_faces,intersection_mask);
			c=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
			intersection|=Intersection_BinaryTree_Skinned_Mesh(c,B,intersecting_faces,intersection_mask);
		} else {
			if (A==B) {
				// A==B && A,B non foglie -> evita di controllare x,y e y,x (x,x ammesso perche' blocchi)
				HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
				HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
				intersection|=Intersection_BinaryTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
				a=static_cast<HyperMeshPartitionTree *>(A->Child[0]);
				b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
				intersection|=Intersection_BinaryTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
				a=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
				b=static_cast<HyperMeshPartitionTree *>(A->Child[1]);
				intersection|=Intersection_BinaryTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
			} else {
				// A!=B && A,B non foglie
				for(int i=0;i<2;i++) {
					for(int j=0;j<2;j++) {
						HyperMeshPartitionTree *a=static_cast<HyperMeshPartitionTree *>(A->Child[i]);
						HyperMeshPartitionTree *b=static_cast<HyperMeshPartitionTree *>(B->Child[j]);
						intersection|=Intersection_BinaryTree_Skinned_Mesh(a,b,intersecting_faces,intersection_mask);
					}
				}
			}
		}
	}


	return intersection;
}




