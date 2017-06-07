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
#include <limits.h>

//the following are UBUNTU/LINUX ONLY terminal color codes.

// by shuttle87 at //stackoverflow.com/questions/9158150/colored-output-in-c
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

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

	int AE_CASE;
	int NUM_FACILITIES;
	int num_splits; // however many EAE is applied
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