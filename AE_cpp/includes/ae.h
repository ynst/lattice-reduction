// libraries
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <chrono> /* library to calculate the time elapsed */

using namespace std;

#ifndef AE_H
#define AE_H

typedef double (*profit_function)(vector<int>); // type for conciseness

class AE{
public:
	AE(profit_function given_prof_function, int size);//main constructor
	AE(const AE& other); // copy constructor

	void dumpProfitFunction (vector<int>, int);
	vector<int> applyReduction ();
	vector<int> applyFullReduction ();
	float bruteForce ( int);

	// vector to hold the ambiguous terms 
	vector<bool> isAmbiguous;

	vector<int> decisions;

	int NUM_FACILITIES;
private:
	profit_function profit_fxn;

	double getProfitWoutFirm (vector<int>, int);
	double getProfitWithFirm (vector<int>, int);

	int num_prof_calls;
};

double profit(vector<int> decision_vector);


#endif