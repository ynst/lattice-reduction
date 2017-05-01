#include "ae.h"

void errorExit(AE AE_instance, string str){
	FILE * pFile;
	pFile = fopen ("error.txt","rw");
	if (pFile==NULL)
	{
		printf ("Error opening file");
		exit (EXIT_FAILURE);
	}
	else
	{
		ofstream myfile;
		myfile.open ("error.txt");
		fwrite (str.c_str() , sizeof(char), sizeof(str), pFile);
		myfile.close();
	}

	fclose(pFile);

	vector<int> tempv(AE_instance.NUM_FACILITIES, 0); // to dump profit function	
	AE_instance.dumpProfitFunction(tempv, 0);  //output to the profits to file
}

int testAE(profit_function profit){

	// stop printing anything to cout

	vector<AE> AEobjects;
	vector<int> AEorEAE;

	AEobjects.push_back(AE(profit, 3));
	AEorEAE.push_back(1);
	AEobjects.push_back(AE(profit, 3));
	AEorEAE.push_back(0);
	AEobjects.push_back(AE(profit, 5));
	AEorEAE.push_back(1);


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
				std::stringstream ss;
				ss << "ERROR in testing";
				ss << solution[j] << "\n";
				ss << AEobjects[i].decisions[j];
				errorExit(AEobjects[i], ss.str());
				return 0;
			}
		}
	}

	cout << "a\n";

	std::cout.clear(); // restore the original stream buffer

	cout<< "––––––––––––– Passed test cases –––––––––––––\n";
	return 1;
}