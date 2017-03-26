// libraries
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
# include <chrono> /* library to calculate the time elapsed */

//default arguments 
int NUM_FACILITIES = 5000; //M in Letchford
int NUM_CLIENTS = 5000; //N

// 1/10 is large fixed costs, 1/100 is medium and 1/1000 refers to small fixed costs
double FIXED_COST_RATIO = 0.001;

using namespace std;

vector<vector<float> > profit_matrix (NUM_FACILITIES);
vector<float> fixed_cost (NUM_FACILITIES);

vector<vector<int> > sup_best_supplier_tracker (NUM_FACILITIES, vector<int>(0));
vector<vector<int> > sup_second_best_supplier_tracker (NUM_CLIENTS, vector<int>(0));

vector<vector<int> > inf_best_supplier_tracker (NUM_CLIENTS, vector<int>(0));

int num_prof_calls;

float randomFloat(float min, float max)
{
    float random = ((float) rand()) / (float) RAND_MAX;

    //example : generate float between 1 and 3
    // generate (in your case) a float between 0 and (3-1)
    // then add 1, producing a float between 1  and 3
    float range = max - min;
    return (random*range) + min;
}

float inf_profit_fxn(vector<int> decision_vector)
{
	float profit =0;
	for (int j = 0; j < NUM_CLIENTS; j++)
	{
		float best_supplier_profit = 0;
		int best_supplier_index=0;
		int best_supp_exists = 0;
		for (int i = 0; i < NUM_FACILITIES; i++)
		{
			if (decision_vector[i] == 1 &&
				profit_matrix[i][j] >= best_supplier_profit)
			{
				best_supp_exists = 1;
				best_supplier_profit =profit_matrix[i][j];
				best_supplier_index = i;
			}
		}
		if (best_supp_exists == 1){
			profit+=best_supplier_profit;
			inf_best_supplier_tracker[j].push_back(best_supplier_index);
		}
	}
	num_prof_calls++;
	return profit;
}

double getProfitWithFirm(vector<int> decision_vector, int firm_index, double inf_profit){
	double profit_with_firm = inf_profit;
	//find new suppliers for the clients that firm_index supplied to
	for(int j = 0; j < NUM_CLIENTS; j++){
		if (inf_best_supplier_tracker[j].size() > 0){
			if (profit_matrix[firm_index][j]>=profit_matrix[inf_best_supplier_tracker[j][0]][j]){
				int popped_out_index = inf_best_supplier_tracker[j][0];
				// inf_best_supplier_tracker[j].pop_back();
				profit_with_firm += profit_matrix[firm_index][j] - profit_matrix[popped_out_index][j];
			}
		}
		else{
			profit_with_firm += profit_matrix[firm_index][j];
		}
	}

	return profit_with_firm;
}

float sup_profit_fxn(vector<int> decision_vector)
{
	float profit =0;
	for (int j = 0; j < NUM_CLIENTS; j++)
	{
		float best_supplier_profit = 0;
		int best_supplier_index=0;
		float second_best_supplier_profit = 0;
		int second_best_supplier_index=0;
		int best_supp_exists = 0, second_best_supp_exists = 0;
		for (int i = 0; i < NUM_FACILITIES; i++)
		{
			if (decision_vector[i] == 1)
			{
				if (profit_matrix[i][j] >= best_supplier_profit){
					best_supp_exists = 1;
					second_best_supplier_index= best_supplier_index;
					second_best_supplier_profit = best_supplier_profit;
					best_supplier_profit =profit_matrix[i][j];
					best_supplier_index = i;
				}
				else if (profit_matrix[i][j] >= second_best_supplier_profit){
					if (best_supp_exists == 1){
						second_best_supp_exists = 1;
					}
					second_best_supplier_profit = profit_matrix[i][j];
					second_best_supplier_index = i;
				}
			}
		}
		profit+=best_supplier_profit;
		if (best_supp_exists == 1){
			sup_best_supplier_tracker[best_supplier_index].push_back(j);
		}
		if (second_best_supp_exists == 1){
			sup_second_best_supplier_tracker[j].push_back(second_best_supplier_index);
		}
	}
	num_prof_calls++;
	return profit;
}

double getProfitWoutFirm(vector<int> decision_vector, int firm_index, double sup_profit){
	double profit_wout_firm = sup_profit;
	//find new suppliers for the clients that firm_index supplied to
	for(int i = 0; i < sup_best_supplier_tracker[firm_index].size(); i++){
		int client2resupply = sup_best_supplier_tracker[firm_index][i];
		// sup_best_supplier_tracker[firm_index].pop_back();
		double next_prof = 0;
		if(sup_second_best_supplier_tracker[client2resupply].size()>0){
			next_prof = profit_matrix[sup_second_best_supplier_tracker[client2resupply][0]][client2resupply];
		}
		// sup_second_best_supplier_tracker[client2resupply].pop_back();
		// int best_supplier_index;
		// double best_supplier_profit;
		// for (int j = 0; j < NUM_FACILITIES; j++){
		// 	if (decision_vector[j] == 1 && j != firm_index&&
		// 		profit_matrix[j][client2resupply] >= best_supplier_profit){
		// 		best_supplier_profit = profit_matrix[j][client2resupply];
		// 		best_supplier_index = j;
		// 	}
		// }
		profit_wout_firm += next_prof - profit_matrix[firm_index][client2resupply];
	}

	return profit_wout_firm +fixed_cost[firm_index];
}

/* 	preprocesses the profit matrix according to Lemma 1 in Letchford 
	profit matrix -> indexed by facility
*/
vector<vector<float> > preprocessProfitMatrix (vector<vector<float> > profit_matrix, 
	vector<float> fixed_cost){

	// holds the delta vector from Letchford's article
	vector<float> delta_vector (NUM_CLIENTS, numeric_limits<float>::min());

	for (int j = 0; j < NUM_CLIENTS; j++){
		for (int i = 0; i < NUM_FACILITIES; i++){
			if (profit_matrix[i][j] - fixed_cost[i] >= delta_vector[j]){
				delta_vector[j] = profit_matrix[i][j] - fixed_cost[i];
			}
		}
	}

	int num_preprocess_reduced = 0;

	vector<vector<float> > preprocessed_profit_matrix (NUM_FACILITIES);
	for (int i = 0 ; i < NUM_FACILITIES ; i++){
		preprocessed_profit_matrix[i].resize(NUM_CLIENTS);
	}

	for (int i = 0; i<NUM_FACILITIES; i++){
		for (int j = 0; j < NUM_CLIENTS; j++){
			if (delta_vector[j] >= profit_matrix[i][j]){
				profit_matrix[i][j] = numeric_limits<float>::min();
				num_preprocess_reduced++;
			}
			else{
				preprocessed_profit_matrix[i][j] = profit_matrix[i][j];
			}
		}
	}

	cout << "# reduction through preprocessing in profit matrix: " << num_preprocess_reduced << "\n";
	return preprocessed_profit_matrix;
}

int main (int argc, char * argv[]) {

	srand (time(NULL));// seed for the random number
	clock_t start_time, end_time; // time at start and end of preprocessing routine

	// ERROR: print instructions for args
	if (argc < 4){
		printf("usage: ./a.out <num of facilities> <num of clients> <facility cost ratio>\n\n");
		if (argc > 1){
			if (strcmp(argv[1], "help") == 0){
				printf("Facility cost ratios in Letchford's test cases:\n");
				printf("0.001 for small, 0.01 for medium, and 0.1 for large\n");
			}
		}
		return 0;
	}

	// read args 
	NUM_FACILITIES = stoi (argv[1],nullptr,0);
	NUM_CLIENTS = stoi (argv[2],nullptr,0);
	FIXED_COST_RATIO = strtod(argv[3], NULL);

	printf("Number of facilities and clients are %d and %d\n", 
		NUM_FACILITIES, NUM_CLIENTS);
	printf("The facility cost ratio is %.3f\n", FIXED_COST_RATIO);

	profit_matrix.resize(NUM_FACILITIES);
	fixed_cost.resize(NUM_FACILITIES);

	sup_best_supplier_tracker.resize(NUM_FACILITIES, vector<int>(0));
	sup_second_best_supplier_tracker.resize(NUM_CLIENTS, vector<int>(0));

	inf_best_supplier_tracker.resize(NUM_CLIENTS, vector<int>(0));

	// resizing the dimension of the profit matrix and the tracker
	// with given num clients
	for (int i = 0 ; i < NUM_FACILITIES ; i++){
		profit_matrix[i].resize(NUM_CLIENTS);
	}

	//fixed costs are loading here
	for (int i = 0; i < NUM_FACILITIES ; i++){
		fixed_cost[i] = sqrt(NUM_CLIENTS)* FIXED_COST_RATIO;
		if(i==0)
			printf("first fixed cost is %.3f\n", fixed_cost[i]);
	}

	vector<float> facility_x (NUM_FACILITIES);
	vector<float> facility_y (NUM_FACILITIES);

	vector<float> client_x (NUM_CLIENTS);
	vector<float> client_y (NUM_CLIENTS);

	for (int i = 0; i < NUM_FACILITIES ; i++)
	{
		facility_x[i] = randomFloat(0,1);
		facility_y[i] = randomFloat(0,1);
	}

	for (int i = 0; i < NUM_CLIENTS ; i++)
	{
		client_x[i] = randomFloat(0,1);
		client_y[i] = randomFloat(0,1);
	}

	float max_distance = 0;
	// max_distance is used to apply Hansen and Thisse's trick
	// turning minimization into maximisation

	float x_diff, y_diff;

	for (int i = 0; i< NUM_FACILITIES; i++)
	{
		for (int j = 0; j < NUM_CLIENTS; j++)
		{
			x_diff = (client_x[i] - facility_x[j]) * (client_x[i] - facility_x[j]);
			y_diff = (client_y[i] - facility_y[j]) * (client_y[i] - facility_y[j]);
			profit_matrix[i][j]= sqrt(x_diff+y_diff);
			if (max_distance <= profit_matrix[i][j]){
				max_distance = profit_matrix[i][j];
			}
		}
	}

	// printf("profit matrix\n");
	// min to max is done here
	for (int i = 0; i< NUM_FACILITIES; i++)
	{
		for (int j = 0; j < NUM_CLIENTS; j++)
		{
			profit_matrix[i][j]= max_distance - profit_matrix[i][j];
			// printf("%.3f ", profit_matrix[i][j]);
		}
		// printf("\n");
	}

	//preprocessing the profit matrix according to Lemma 1 in Letchford
	profit_matrix = preprocessProfitMatrix(profit_matrix, fixed_cost);

	vector<int> decisions (NUM_FACILITIES);
	vector<int> new_decisions (NUM_FACILITIES);

	//vector to hold the ambiguous terms 
	vector<bool> isAmbiguous (NUM_FACILITIES);
	for (int i = 0; i < NUM_FACILITIES; i++)
		isAmbiguous[i] = true;

	start_time = clock(); // start time

	//num of entries changed in each while loop iteration
	// initially -1 to get the while loop started
	int num_changed;

	do
	{
		num_changed = 0;

		// make sure the trackers are clean
		for (int i = 0; i < NUM_FACILITIES; ++i)
		{
			if (sup_best_supplier_tracker[i].size()>0){
				sup_best_supplier_tracker[i].clear();
			}
		}

		for (int i = 0; i < NUM_CLIENTS; ++i)
		{
			if (sup_second_best_supplier_tracker[i].size()>0){
				sup_second_best_supplier_tracker[i].clear();
			}
			if (inf_best_supplier_tracker[i].size()>0){
				inf_best_supplier_tracker[i].clear();
			}
		}

		// checking for the supremum of the lattice
		vector<int> lattice_sup_decision_vector (NUM_FACILITIES);
		lattice_sup_decision_vector=decisions;
		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i]){
				lattice_sup_decision_vector [i] = 1;
			}
		}

		double sup_profit = sup_profit_fxn(lattice_sup_decision_vector);

		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i] == false){
				continue;
			}
			// printf("supprofit: %f without %d profit: %f\n", sup_profit, i, getProfitWoutFirm(lattice_sup_decision_vector, i, sup_profit));
			if (sup_profit - getProfitWoutFirm(lattice_sup_decision_vector, i, sup_profit) >= 0)
			{
				isAmbiguous[i]=false;
				new_decisions[i] = 1;
				// SUBMODULAR idea: if you result in positive profit when everyone is open
				// then you should always be open
				num_changed++;
			}
		}

		// checking for the infimum of the lattice
		vector<int> lattice_inf_decision_vector (NUM_FACILITIES);
		lattice_inf_decision_vector= decisions;
		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i]){
				lattice_inf_decision_vector [i] = 0;
			}
		}

		double inf_profit = inf_profit_fxn(lattice_inf_decision_vector);

		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i] == false){
				continue;
			}
			// printf("infprofit: %f without %d profit: %f\n", getProfitWithFirm(lattice_inf_decision_vector, i, inf_profit), i, inf_profit+ fixed_cost[i]);
			if (getProfitWithFirm(lattice_inf_decision_vector, i, inf_profit) - inf_profit- fixed_cost[i]  <= 0)
			{
				isAmbiguous[i]=false;
				new_decisions[i] = 0;
				num_changed++;
			}
		}
		decisions = new_decisions;
		// printf("health check\n");
		// for (int i = 0; i < NUM_FACILITIES; ++i)
		// {
		// 	printf("%i, decision: %d, is ambiguous: %d\n",i, decisions[i], isAmbiguous[i]==true);
		// }
	} while(num_changed != 0);

	end_time = clock(); // finish time

	int num_ambigious_decisions = 0;

	for (int i = 0; i < NUM_FACILITIES; ++i)
	{
		// printf("%i, decision: %d, is ambiguous: %d\n",i, decisions[i], isAmbiguous[i]==true);
		if (isAmbiguous[i] == 1){
			num_ambigious_decisions++;
		}
	}

	double total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

	printf("Total time AE took is %.3f s\n", total_time);
	printf("Percentage of decisions eliminated: %.3f.\n", 
		(double) (NUM_FACILITIES - num_ambigious_decisions) / NUM_FACILITIES *100);

	cout << "# ambigious decisions\t\t" << num_ambigious_decisions << endl;

	cout << "# profit calculations\t\t" << num_prof_calls << endl;
	return 0;
}