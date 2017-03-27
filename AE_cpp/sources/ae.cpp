#include "ae.h"

// main constructor with given prof function and num facilities
AE::AE(profit_function given_prof_func, int size){
	profit_fxn = given_prof_func;
	NUM_FACILITIES = size;
	num_prof_calls = 0;

	vector<bool> temp(NUM_FACILITIES, true);
	isAmbiguous = temp;

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


double AE::getProfitWithFirm(vector<int> decision_vector, int firm_index){
	int temp = decision_vector[firm_index];

	// case when the firm is closed
	decision_vector[firm_index] = 1;

	double profit_with_firm = profit_fxn(decision_vector);	

	decision_vector[firm_index] = temp;

	num_prof_calls++; // one call to get the marginal value

	return profit_with_firm;
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

double AE::getProfitWoutFirm(vector<int> decision_vector, int firm_index){
	// for (vector<int>::const_iterator i = decision_vector.begin(); i != decision_vector.end(); ++i)
	//     cout << *i << ' ';
	int temp = decision_vector[firm_index];

	// case when the firm is closed
	decision_vector[firm_index] = 0;

	double profit_wout_firm = profit_fxn(decision_vector);	

	decision_vector[firm_index] = temp;

	num_prof_calls++; // one call to get the marginal value

	// printf("profit with firm %f profit without firm %f\n", profit_w_firm, profit_wout_firm);

	return profit_wout_firm;
}

vector<int> AE::applyReduction() {	
	clock_t start_time, end_time; // time at start and end of routine

	vector<int> decisions (NUM_FACILITIES);
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
			if (sup_profit - getProfitWoutFirm(lattice_sup_decision_vector, i) >= 0)
			{
				isAmbiguous[i]=false;
				new_decisions[i] = 1;
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

		double inf_profit = profit_fxn(lattice_inf_decision_vector);
		num_prof_calls++;

		for (int i = 0 ; i<NUM_FACILITIES; i++)
		{
			if (isAmbiguous[i] == false){
				continue;
			}
			if (getProfitWithFirm(lattice_inf_decision_vector, i) - inf_profit <= 0)
			{
				isAmbiguous[i]=false;
				new_decisions[i] = 0;
				num_changed++;
			}
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
	return decisions;
}

float AE::bruteForce(int index){

	if (index >= this->NUM_FACILITIES){
		return profit_fxn(decisions);
	}

	AE instance_with_0 = AE (*this);
	instance_with_0.decisions[index] = 0;

	AE instance_with_1 = AE (*this);
	instance_with_1.decisions[index] = 1;

	if (instance_with_0.bruteForce(index + 1) 
		> instance_with_1.bruteForce(index + 1)){
		return instance_with_0.bruteForce(index + 1);
	} else {
		return instance_with_1.bruteForce(index + 1);
	}
}

vector<int> AE::applyFullReduction(){

	applyReduction();

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

			num_prof_calls += instance_with_0.num_prof_calls + instance_with_1.num_prof_calls;

			break;
		}
	}

	return decisions;
}

int main(int argc, char* argv[]){


	int num_facilities;

	if (argc > 1){
		num_facilities = stoi (argv[1],nullptr,0);	//read number of facilities
	} else{
		num_facilities = 10;
	}

	AE ae_instance(profit, num_facilities);

	char c1, c2, c3, c4; // for user responses 

	cout << "Run AE? y or n" << endl;
	scanf(" %c", &c1);

	if (c1 == 'y'){
		ae_instance.applyReduction(); // apply AE
	}

	cout << "Run Extended AE? y or n " << endl;
	scanf(" %c", &c2);

	if (c2 == 'y'){
		clock_t start_time, end_time; // time at start and end of routine

		start_time = clock(); // start time
		cout << endl << "Starting EAE" << endl;

		ae_instance.applyFullReduction(); // apply EAE

		end_time = clock(); // finish time

		double total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

		printf("Total time EAE took is %.3f s\n", total_time);

		// cout << "# profit calculations\t\t" << ae_instance.num_prof_calls << endl;
	}

	cout << "Run brute force? y or n " << endl;
	scanf(" %c", &c3);

	if (c3 == 'y'){
		clock_t start_time, end_time; // time at start and end of routine

		start_time = clock(); // start time
		cout << endl << "Starting brute force" << endl;

		printf("Result is %.3f\n", ae_instance.bruteForce(0));

		end_time = clock(); // finish time

		double total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

		printf("Total time brute force took is %.3f s\n", total_time);

		// cout << "# profit calculations\t\t" << ae_instance.num_prof_calls << endl;
	}

	// for (vector<bool>::const_iterator i = ae_instance.isAmbiguous.begin(); i != ae_instance.isAmbiguous.end(); ++i)
	//     cout << *i << ' ';

	cout << "Do you want to print out profit function? y or n" << endl;
	scanf(" %c", &c4);

	if (c4 == 'y'){
		vector<int> tempv(ae_instance.NUM_FACILITIES, 0); // initialize temp vector
		ae_instance.dumpProfitFunction(tempv, 0);	
	}

	return 0;
}