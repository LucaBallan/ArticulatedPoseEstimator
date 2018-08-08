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
	
	Array<double> tmp(*LastEntries);
	tmp.sort(MedianEstimator_compare_double);
	double sum=tmp[LastEntries->numElements()/2];

	return sum;
}
