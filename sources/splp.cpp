#include "ae.h"

double profit(vector<int> decision_vector){

	int NUM_CLIENTS = decision_vector.size(), NUM_FACILITIES = decision_vector.size();

	vector<vector<float> > profit_matrix (NUM_FACILITIES);
	vector<float> fixed_cost (NUM_FACILITIES);

	vector<vector<int> > sup_best_supplier_tracker (NUM_FACILITIES, vector<int>(0));
	vector<vector<int> > sup_second_best_supplier_tracker (NUM_CLIENTS, vector<int>(0));

	vector<vector<int> > inf_best_supplier_tracker (NUM_CLIENTS, vector<int>(0));

	// resizing the dimension of the profit matrix and the tracker
	// with given num clients
	for (int i = 0 ; i < NUM_FACILITIES ; i++){
		profit_matrix[i].resize(NUM_CLIENTS);
	}

	//fixed costs are loading here
	for (int i = 0; i < NUM_FACILITIES ; i++){
		fixed_cost[i] = sqrt(NUM_CLIENTS) * 0.1;
	}

	vector<float> facility_x (NUM_FACILITIES);
	vector<float> facility_y (NUM_FACILITIES);

	vector<float> client_x (NUM_CLIENTS);
	vector<float> client_y (NUM_CLIENTS);

	srand (1);// seed for the random number


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
			profit += best_supplier_profit;
		}
	}

	for (int i = 0; i < NUM_FACILITIES; ++i)
	{
		if (decision_vector[i] == 1){
			profit -= fixed_cost[i];
		}
	}

	return profit;
}