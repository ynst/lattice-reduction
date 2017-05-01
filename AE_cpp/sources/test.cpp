#include "ae.h"

void errorExit(AE AE_instance, string errorMessage){
	ofstream myfile;
	myfile.open ("error.txt");
	myfile << errorMessage;
	myfile.close();

	vector<int> tempv(AE_instance.NUM_FACILITIES, 0); // to dump profit function	
	AE_instance.dumpProfitFunction(tempv, 0);  //output to the profits to file
}

int testAE(profit_function profit){

	// stop printing anything to cout

	vector<AE> AEobjects;
	vector<int> AEorEAE;

	//test cases
	AEobjects.push_back(AE(profit, 3));
	AEorEAE.push_back(1);
	AEobjects.push_back(AE(profit, 3));
	AEorEAE.push_back(1);
	AEobjects.push_back(AE(profit, 5));
	AEorEAE.push_back(1);
	AEobjects.push_back(AE(profit, 10));
	AEorEAE.push_back(1);
	AEobjects.push_back(AE(profit, 5));
	AEorEAE.push_back(0);


	for(int i = 0; i < AEobjects.size(); i++){
		vector<int> v (AEobjects[i].NUM_FACILITIES);

		vector<int> solution = bruteForce(v, 0, AEobjects[i].NUM_FACILITIES ); // correct solution

		if(AEorEAE[i] == 1){
			AEobjects[i].applyReduction(); 
		}
		else{
			AEobjects[i].applyFullReduction(); 
		}

		for (int j = 0; j < solution.size(); ++j)
		{
			if (!AEobjects[i].isAmbiguous[j] && // AE found the result
				AEobjects[i].decisions[j] != solution[j])
			{
				// sometimes AE and brute force would identify solutions with different decision
				// vector, yet they would have almost the same profit. This is ok.
				if (std::abs(profit(AEobjects[i].decisions) - profit(solution)) <= 0.0001){
					continue;
				}

				std::stringstream ss;
				ss << "ERROR in testing";
				ss << solution[j] << "\n";
				ss << AEobjects[i].decisions[j];
				errorExit(AEobjects[i], ss.str());
				return 0;
			}
		}
	}

	std::cout.clear(); // restore the original stream buffer

	cout<< "––––––––––––– Passed test cases –––––––––––––\n";
	return 1;
}