#include "ae.h"

double jia(vector<int> decision_vector){
	// for (vector<int>::const_iterator i = decision_vector.begin(); i != decision_vector.end(); ++i)
	//     cout << *i << ' ';


	// INITIALIZE RANDOM SEED TO ENSURE ARRAY IS THE SAME EVERY TIME
	srand ( time(NULL) );

	const double rand_array[7] = {2.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0};

	vector<double> fixed_costs(0);
	for(int i  = 0; i < decision_vector.size(); i++){
		int rand_index = rand() % 7;
		fixed_costs.push_back(rand_array[rand_index]);
	}

	vector<double> distances(decision_vector.size()*decision_vector.size(), -1.0);
	double delta =1;

	double prof2return = 0;

	for (int i = 0; i < decision_vector.size(); ++i)
	{
		if(decision_vector[i]){
			prof2return+=fixed_costs[i] * sqrt(decision_vector.size());
			for (int j = 0; j < decision_vector.size(); ++j)
			{
				if(j!=i && decision_vector[j]){
					prof2return+=delta/distances[i*decision_vector.size()+ j];
				}
			}
		}
	}
	// printf("%.3f\n", prof2return);
	return prof2return;
}