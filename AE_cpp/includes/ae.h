#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <chrono> /* library to calculate the time elapsed */
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath> // for abs()

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

	vector<bool> isAmbiguous; // vector to hold the ambiguous terms 

	vector<int> decisions;

	int NUM_FACILITIES;
private:
	profit_function profit_fxn;

	int num_prof_calls;
};

double profit (vector<int> decision_vector); 

int testAE(profit_function profit_fxn);
vector<int> bruteForce (vector<int>, int, int);

float randomFloat(float min, float max);
float randomFrechet (float);
void displayProgressBar(float progress);

#endif