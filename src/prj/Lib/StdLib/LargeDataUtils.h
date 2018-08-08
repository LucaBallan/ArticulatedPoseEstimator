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






class Statistics {
	static int  NumStatInstances;
	static bool StatBoxOpen;
	static int  GlobalIndex;

	Array<float> *Data;
	char *StatFileName;
	int LocalIndex;
	
	void Print();
public:	

	Statistics(int Estimated_Number_of_Elements,char *StatFileName=NULL);
	~Statistics();

	void AddData(float value);
};


// Modo 1:
//			create all the stat instance
//			save data
//			delete all the stat instance
// Modo 2:
//			create instance
//			save data
//			delete instance
//			create instance
//			save data
//			delete instance




class Estimator {
protected:
	Array<double> *LastEntries;
	int history_len;
public:
	Estimator(int history_len);
	virtual ~Estimator();

	virtual void update(double val);				// update the estimate
	virtual double getEstimate() = 0;				// DBL_MAX if the estimate is not ready
};

class MedianEstimator : public Estimator {
public:
	MedianEstimator(int history_len);
	virtual ~MedianEstimator();

	double getEstimate();
};
