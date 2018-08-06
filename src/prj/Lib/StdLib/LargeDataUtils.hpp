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





/*
#define DATA_ABSENT DBL_MAX

Rec_Evolution::Rec_Evolution(UINT NumData,UINT MaxNumDataToRec,UINT MaxRecLenght) {

	this->NumData=NumData;

	if (NumData>MaxNumDataToRec) NumDataToRec=MaxNumDataToRec;
	else NumDataToRec=NumData;

	Data=new double*[NumDataToRec];
	if (Data==NULL) ErrorExit("Rec_Evolution: Out of memory.");
	
	for(UINT i=0;i<NumDataToRec;i++) {
		Data[i]=new double[MaxRecLenght];
		if (Data[i]==NULL) ErrorExit("Rec_Evolution: Out of memory.");
		
		for(UINT j=0;j<MaxRecLenght;j++) Data[i][j]=DATA_ABSENT;
	}
	
	Index=new int[NumData];
	if (Index==NULL) ErrorExit("Rec_Evolution: Out of memory.");
	
	for(UINT i=0;i<NumData;i++) Index[i]=-1;

	if (NumData==NumDataToRec) {
			for(UINT i=0;i<NumDataToRec;i++) Index[i]=i;
	} else {
		for(UINT i=0;i<NumDataToRec;i++) {
			int Sel;
		
			do {Sel=Uniform_int_rand(0,NumData-1);
			} while (Index[Sel]!=-1);

			Index[Sel]=i;
		}
	}

	Time=0;
}

Rec_Evolution::~Rec_Evolution() {
	if (Data!=NULL) {
		for(UINT i=0;i<NumDataToRec;i++) 
			if (Data[i]!=NULL) delete [](Data[i]);
		delete []Data;
	}

	if (Index!=NULL) delete []Index;
}

void Rec_Evolution::AddData(UINT index,double value) {
	if (Index[index]==-1) return;

	Data[Index[index]][Time]=value;
}

void Rec_Evolution::NextTime() {
	Time++;
}

void Rec_Evolution::Save(char *filename) {
	char Str[70];
	UINT j;

	OFileBuffer *Out=new OFileBuffer(filename);
	
	for(UINT i=0;i<NumDataToRec;i++) {
		for(j=0;j<NumData;j++) {
			if (Index[j]==i) break;
		}

		sprintf(Str,"%i:",j);
		Out->WriteLine(Str,(int)strlen(Str));
		
		for(j=0;j<Time;j++) {
			if (Data[i][j]!=DATA_ABSENT) sprintf(Str,"	%f",Data[i][j]);
			else sprintf(Str,"	DATA_ABSENT");
			
			Out->WriteLine(Str,(int)strlen(Str));
		}

		sprintf(Str,"\r\n");
		Out->WriteLine(Str,(int)strlen(Str));
	}
	
	delete Out;	
}
*/


int  Statistics::NumStatInstances=0;
bool Statistics::StatBoxOpen=false;
int  Statistics::GlobalIndex=1;


Statistics::Statistics(int Estimated_Number_of_Elements,char *StatFileName) {
	this->Data=new Array<float>(Estimated_Number_of_Elements);
	this->LocalIndex=GlobalIndex;
	this->StatFileName=StatFileName;

	NumStatInstances++;
	GlobalIndex++;
}

Statistics::~Statistics() {
	if (StatFileName!=NULL) {
		char TmpFileName[500];
		sprintf(TmpFileName,StatFileName,LocalIndex);
		OFileBuffer Out(TmpFileName);
		for(int i=0;i<Data->numElements();i++) Out<<((*Data)[i])<<"\r\n";
		Out.Close();
	} 
	Print();
	delete Data;
}

void Statistics::AddData(float value) {
	Data->append(value);
}

void Statistics::Print() {
	
	double average=0; 
	for(int i=0;i<Data->numElements();i++) average+=(*Data)[i];
	if (Data->numElements()!=0) average/=Data->numElements();

	double std=0,tmp; 
	for(int i=0;i<Data->numElements();i++) {
		tmp=(*Data)[i]-average;
		std+=tmp*tmp;
	}
	if (Data->numElements()>1) std=sqrt(std/(Data->numElements()-1));
	else std=0;
	
	if (StatBoxOpen==false) {
		StatBoxOpen=true;
		cout<<"\n";
		cout<<"----- Statistics -----\n";
		cout<<"|  Mean  |  Std Dev  |\n";
		cout<<"----------------------\n";
	}
	cout<<average<<"  "<<std<<"\n";
	NumStatInstances--;
	if (NumStatInstances==0) {
		cout<<"----------------------\n";
	}
}









Estimator::Estimator(int history_len) {
	this->history_len=history_len;
	this->LastEntries=NULL;

	if (history_len>0) this->LastEntries=new Array<double>(history_len);
}
Estimator::~Estimator() {
	SDELETE(LastEntries);
}
void Estimator::update(double val) {
	if (LastEntries->numElements()<history_len) {
		LastEntries->append(val);
	} else {
		LastEntries->del(0);
		LastEntries->append(val);
	}
}



MedianEstimator::MedianEstimator(int history_len) : Estimator(history_len) {
}
MedianEstimator::~MedianEstimator() {
}
int MedianEstimator_compare_double(const void *a,const void *b) {
	double *a_=(double *)a;
	double *b_=(double *)b;
	
	if (*a_>*b_) return 1;
	if (*a_==*b_) return 0;
	return -1;
}
double MedianEstimator::getEstimate() {
	if (LastEntries->numElements()!=history_len) return DBL_MAX;
	
	/*double sum=0;
	for(int i=0;i<LastEntries->numElements();i++) sum+=(*LastEntries)[i];
	sum/=LastEntries->numElements();*/
	
	Array<double> tmp(*LastEntries);
	tmp.sort(MedianEstimator_compare_double);
	double sum=tmp[LastEntries->numElements()/2];

	return sum;
}
