#include "ae.h"

// main constructor with given prof function and num facilities
AE::AE(profit_function given_prof_func, int size){
	profit_fxn = given_prof_func;
	NUM_FACILITIES = size;
	num_prof_calls = 0;

	// initially all are ambiguous
	vector<bool> temp(NUM_FACILITIES, true);
	isAmbiguous = temp;

	// initially all decisions are 0, closed
	// but this assumption does not affect how the program runs
	vector<int> tempdecisions(NUM_FACILITIES, 0);
	decisions = tempdecisions;
}

// copy constructor
AE::AE(const AE& other){
	profit_fxn = other.profit_fxn;
	NUM_FACILITIES = other.NUM_FACILITIES;
	num_prof_calls = other.num_prof_calls;
	isAmbiguous = other.isAmbiguous;
	decisions = other.decisions;
}

void AE::dumpProfitFunction (vector<int> v, int n){

	if (n == NUM_FACILITIES){
		for (vector<int>::size_type i = 0; i < v.size(); ++i){
			cout << v[i] << ' ';
		}

		cout << profit_fxn (v);
		cout << endl;
		return;
	}

	v[n] = 0;
	AE::dumpProfitFunction (v, n+1);

	v[n] = 1;
	AE::dumpProfitFunction (v, n+1);
}

vector<int> AE::applyReduction() {	
	clock_t start_time, end_time; // time at start and end of routine

	// vector<int> decisions (NUM_FACILITIES);
	vector<int> new_decisions (NUM_FACILITIES);

	start_time = clock(); // start time

	//num of entries changed in each while loop iteration
	int num_changed;

	do
	{
		new_decisions = decisions;
		num_changed = 0;

		// checking for the supremum of the lattice
		vector<int> lattice_sup_decision_vector (NUM_FACILITIES);
		lattice_sup_decision_vector=decisions;
		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i]){
				lattice_sup_decision_vector [i] = 1;
			}
		}

		double sup_profit = profit_fxn(lattice_sup_decision_vector);
		num_prof_calls++;

		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i] == false){
				continue;
			}

			// what if the facility is closed?
			lattice_sup_decision_vector[i] = 0;

			if (AE_CASE == 1) // submodular
			{
				// if closing increases your profit then close it for good, using
				// AE assumption
				if (sup_profit - profit_fxn(lattice_sup_decision_vector) >= 0){
					isAmbiguous[i]=false;
					new_decisions[i] = 1;
					num_changed++;
				}
				num_prof_calls++;
			}
			else {
				// if closing increases your profit then close it for good, using
				// AE assumption
				if (sup_profit - profit_fxn(lattice_sup_decision_vector) <= 0){
					isAmbiguous[i]=false;
					new_decisions[i] = 0;
					num_changed++;
				}
				num_prof_calls++;
			}

			// return to default state
			lattice_sup_decision_vector[i] = 1;
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

		double inf_profit = profit_fxn(lattice_inf_decision_vector);
		num_prof_calls++;

		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i] == false){
				continue;
			}

			// what if facility was open?
			lattice_inf_decision_vector[i] = 1;

			if (AE_CASE == 1) // submodular
			{
				// if opening the facility increases your profit, use AE asssumption
				if (profit_fxn(lattice_inf_decision_vector)  - inf_profit <= 0)
				{
					isAmbiguous[i]=false;
					new_decisions[i] = 0;
					num_changed++;
				}
				num_prof_calls++;
			}
			else {
				// if opening the facility increases your profit, use AE asssumption
				if (profit_fxn(lattice_inf_decision_vector)  - inf_profit >= 0)
				{
					isAmbiguous[i]=false;
					new_decisions[i] = 1;
					num_changed++;
				}
				num_prof_calls++;
			}

			// return to default state
			lattice_inf_decision_vector[i] = 0;
		}
		decisions = new_decisions;
	} while(num_changed != 0);

	end_time = clock(); // finish time

	int num_ambigious_decisions = 0;

	
	for (int i = 0; i < NUM_FACILITIES; ++i)
	{
		if(NUM_FACILITIES < 50){
			printf("%i, decision: %d, is ambiguous: %d\n",i, decisions[i], isAmbiguous[i]==true);
		}

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

	this->decisions = decisions; // write decision vector to the class

	return decisions;
}

vector<int> bruteForce(vector<int> v, int index, int num_facilities){

	if (index >= num_facilities){
		return v;
	}

	vector<int> v0 =v, v1=v; 
	v0[index] = 0;
	v1[index] = 1;

	if (profit(bruteForce(v0, index+1, num_facilities)) >
		profit(bruteForce(v1, index+1, num_facilities))){
		return bruteForce(v0, index+1, num_facilities);
	} else {
		return bruteForce(v1, index+1,num_facilities);
	}
}

vector<int> AE::applyFullReduction(){
	(*this).applyReduction();

	for (int i = 0; i < NUM_FACILITIES; ++i)
	{
		if (isAmbiguous[i] == 1){

			AE instance_with_0 = AE (*this);
			instance_with_0.decisions[i] = 0;
			instance_with_0.isAmbiguous[i] = 0;
			instance_with_0.applyFullReduction();

			AE instance_with_1 = AE (*this);
			instance_with_1.decisions[i] = 1;
			instance_with_1.isAmbiguous[i] = 0;
			instance_with_1.applyFullReduction();

			if (profit_fxn(instance_with_1.decisions) > profit_fxn(instance_with_0.decisions)){
				decisions = instance_with_1.decisions;
				isAmbiguous = instance_with_1.isAmbiguous;
			} else {
				decisions = instance_with_0.decisions;
				isAmbiguous = instance_with_0.isAmbiguous;
			}

			num_prof_calls += instance_with_0.num_prof_calls + instance_with_1.num_prof_calls + 2;

			break; // enough to set one
		}
	}

	return decisions;
}

int main(int argc, char* argv[]){


	int num_facilities, 
		reductionScheme = 0, //AE: 0, EAE: 1, Brute force: 2, Print function: 3
		AE_CASE=1; 

	if (argc > 1){
		num_facilities = stoi (argv[1],nullptr,0);	//read number of facilities
		if (argc > 2){
			reductionScheme = stoi (argv[2],nullptr,0);
			if (argc > 3){
				AE_CASE = stoi (argv[3],nullptr,0); // 1 submodular, 0 supermod
			}
		}
	} else{
		num_facilities = 10;
	}

	cout << BOLDBLUE << "Testing AE..." << endl << RESET;

	if (!testAE(profit)){
		return 0;
	}

	cout << endl << endl << BLUE << "Running\n" << RESET;

	// create an instance of AE
	AE ae_instance(profit, num_facilities);

	ae_instance.AE_CASE = AE_CASE;

	if(reductionScheme == 0){
		ae_instance.applyReduction(); 	// apply AE
	}

	if (reductionScheme == 1){ // apply EAE
		clock_t start_time, end_time; // time at start and end of routine

		start_time = clock(); // start time
		cout << endl << "Starting EAE" << endl;

		ae_instance.applyFullReduction(); // apply EAE

		end_time = clock(); // finish time

		double total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

		printf("Total time EAE took is %.3f s\n", total_time);
	}

	if (reductionScheme == 0 || reductionScheme == 1){
		int num_open = 0;

		for (int i = 0; i < NUM_FACILITIES; ++i){
			if (ae_instance.decisions[i] && !ae_instance.isAmbiguous[i]){
				num_open++;
			}
		}

		cout << "# open facilities\t\t" << num_open << endl;
	}

	if (reductionScheme == 2){ // run brute force
		clock_t start_time, end_time; // time at start and end of routine

		start_time = clock(); // start time
		cout << endl << "Starting brute force" << endl;

		vector<int> v (ae_instance.NUM_FACILITIES);

		printf("Result is %.3f\n", profit(bruteForce(v, 0, ae_instance.NUM_FACILITIES)));

		end_time = clock(); // finish time

		double total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

		printf("Total time brute force took is %.3f s\n", total_time);
	}

	if (reductionScheme == 3){ // print function
		vector<int> tempv(ae_instance.NUM_FACILITIES, 0); // initialize temp vector
		ae_instance.dumpProfitFunction(tempv, 0);	
	}

	cout << BOLDGREEN << "Completed successfully!" << endl << RESET;

	return 0;
}