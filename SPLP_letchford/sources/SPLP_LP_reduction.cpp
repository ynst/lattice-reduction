/*
SPLP_LP_reduction.cpp: a program that takes a sparse instance
of the Simple Plant Location Problem, and uses LP to make it even
sparser.

The overall structure is as follows:
 - read an instance in sparse format and set up arrays
 - solve the LP relaxation and record useful parts of the primal and dual solutions 
 - use modified drop to get an initial upper bound
 - use iterated rounding to obtain a usually better upper bound
 - eliminate x variables using 1st reduction rule
 - write reduced instance (in sparse format) to a file
 - eliminate x variables using 2nd reduction rule
 - write the reduced instance (in CPLEX format) to a file

The two reduction rules are:

1. Eliminate x_ij using reduced costs of x_ij and y_i
2. Replace x_ij with y_i if the dual price of the VUB is high.

The program uses functions from the CPLEX callable library
to solve various LPs.

We use i for location indices, j for client indices,
and k for the kth closest facility to a given client.

The input file should have the following format:
<number of locations> <number of clients> <'s', for 'sparse'>
<row containing one cost per facility>
<row containing the degree of each client>
<one row for each client j, containing pairs i, c_ij >

The first output file will be similar.
*/


/*********************HEADER FILES***********************/

#include <stdio.h>
#include <stdlib.h>
#include "C:\CPLEX_Studio124\cplex\include\ilcplex\cplex.h"
#include <time.h>
#include <sys/timeb.h>

/***************GLOBAL CONSTANTS AND VARIABLES*****************/

#define MAX_SIZE 20000 // maximum number of clients or facilities
#define BIG_INT 100000 // arbitrary large integer
unsigned M;	// number of facilities
unsigned N;	// number of clients
unsigned MAX_DEG; // maximum degree of a client
unsigned current_upper_bound, best_upper_bound; // upper bounds from primal heuristics
unsigned current_num_edges, num_eliminated; // to keep track of number of edges
double lower_bound; // lower bound from LP relaxation
unsigned i, j, k; // counters: i for locations, j for clients, k for position in list for given client

//********************************************************************************
//********** Function WRITE_TEMP_LP_FILE ************************************************
//********************************************************************************

void write_temp_LP_file(char *LPfilename, unsigned * facility_cost, unsigned ** sorted_assignment_cost, unsigned ** sorted_location, unsigned * degree)
{
   // variable declaration
   FILE *filepointer; // pointer for handling file
   
   // open LP file for writing

   if ((filepointer = fopen(LPfilename, "w"))== NULL)
	{
		fprintf(stderr, "Cannot open temporary LP file for writing!\n");
		getchar();
		exit(1);
	}

	// print objective function

	fprintf(filepointer, "minimize %u y1", facility_cost[1]);
	
	for(i=2;i<=M;i++)
	{
		fprintf(filepointer, " + %u y%u", facility_cost[i], i);
		if (i % 10 == 0)
			fprintf(filepointer, "\n");
	}
	fprintf(filepointer, "\n");

	for(j=1;j<=N;j++)
	{
	   for(k=1;k<=degree[j];k++)
	   {
	      fprintf(filepointer, " + %u x%ut%u", sorted_assignment_cost[k][j], sorted_location[k][j], j);
		  if (k % 10 == 0)
			 fprintf(filepointer, "\n");
       }
	   fprintf(filepointer, "\n");
	}

	fprintf(filepointer,"subject to\n");

	// the assignment constraints

   for (j=1;j<=N;j++)
   {
      fprintf(filepointer, "ass%u: x%ut%u", j, sorted_location[1][j], j);
      for (k = 2; k <= degree[j]; k++)
      {	
	     fprintf(filepointer, " + x%ut%u", sorted_location[k][j], j);
		 if (k % 10 == 0)
		    fprintf(filepointer, "\n");
	  }
	  fprintf(filepointer, " = 1\n");
   }

	// the variable upper bounds

   for (j=1;j<=N;j++)
      for (k=1;k<=degree[j];k++)
		  fprintf(filepointer, "vub%ut%u: y%u - x%ut%u >= 0\n", sorted_location[k][j], j, sorted_location[k][j], sorted_location[k][j],j);

	// it is probably better not to insert upper bounds of one on the y variables,
    // because we want the reduced costs of the x and y variables, and the dual prices
    // of the variable upper bounds, to be as large as possible

   //fprintf(filepointer,"bounds\n");
   //for (i = 1; i <= M; i++)
   //   fprintf(filepointer, "y%u <= 1\n", i);

   	// finish off

   fprintf(filepointer,"end");

	// close the LP file
	fclose(filepointer);

}   // End of function write_temp_LP_file


/********************FUNCTION MODIFIED DROP**********************/

unsigned modified_drop(unsigned *facility_cost, unsigned **sorted_assignment_cost, unsigned **sorted_location,
					   double *primal_value, unsigned *degree)
{
   // variable declarations
   int saving; // to compute the potential saving gained by closing a facility
   unsigned number_open; // number of facilities left open
   unsigned biggest_index, temp; // for the sorting routine
   unsigned current_upper_bound; // for updating best upper bound
   unsigned current_facility; // facility we are thinking of closing
   bool * open; // records which facilities are open in primal solution
   unsigned * facility_list; // stores the list of facilities (to be sorted)
   unsigned * nearest; // stores the level of the nearest open facility to each client
   unsigned * second_nearest; // stores the level of the second nearest open facility to each client
   double biggest_y_value; // for the sorting routine
   double * temp_y_value; // also for the sorting routine

   // allocate memory for the arrays
   try
   {
      facility_list = new unsigned[M+1];
      open = new bool[M+1];
      nearest = new unsigned[N+1];
      second_nearest = new unsigned[N+1];
      temp_y_value = new double[N+1];
   }
   catch (...)
   {
      printf("\nCan't allocate memory for arrays in modified_drop function!\n");
	  getchar();
   }

   // initialise the facility list array

   for (i=1; i <= M; i++)
	   facility_list[i] = i;

   // initialise the temporary y value array

   for (i=1; i <= M; i++)
		  temp_y_value[i] = primal_value[i-1];

   // for this version of modified drop, we sort the facilities in non-increasing order of y value

   for (i = 1; i < M; i++)
   {
      biggest_y_value = temp_y_value[facility_list[i]];
      biggest_index = i;
      for (k=i+1; k<=M; k++)
         if (temp_y_value[facility_list[k]] > biggest_y_value)
         {
            biggest_y_value = temp_y_value[facility_list[k]];
            biggest_index = k;
		 }
      temp = facility_list[i];
      facility_list[i] = facility_list[biggest_index];
      facility_list[biggest_index] = temp;
   } // end 'for i'

   // start by provisionally opening every facility
   for (i=1;i<=M;i++)
   	  open[i] = true;
   number_open = M;

   // allocate each client to closest facility
   for (j=1;j<=N;j++)
   {
	   nearest[j] = 1;
	   second_nearest[j] = 2;
   }

   // compute cost of initial primal solution
   current_upper_bound = 0;
   for (i=1;i<=M;i++)
   	  current_upper_bound += facility_cost[i];
   for (j=1;j<=N;j++)
	   current_upper_bound += sorted_assignment_cost[1][j];

   // close all facilities that have zero y value
  
   for (i=1;i<=M;i++)
      if (temp_y_value[facility_list[i]] < 0.001)
	  {
         current_facility = facility_list[i]; // facility currently under consideration
         // compute effect on cost of closing the current facility
         saving = facility_cost[current_facility];
         for (j=1;j<=N;j++)
            if (sorted_location[nearest[j]][j] == current_facility)
		    {
			   saving += sorted_assignment_cost[nearest[j]][j];
               saving -= sorted_assignment_cost[second_nearest[j]][j];
            }
         // close the facility
         open[current_facility] = false; 
		 number_open --;
         // improve the upper bound
		 current_upper_bound -= saving; 
         for (j=1;j<=N;j++) // check which clients are involved
		 {
            // check if nearest needs to be updated
			 if (sorted_location[nearest[j]][j] == current_facility)
			 {
				 nearest[j] = second_nearest[j];
				 // find new second nearest facility
				 for (k=second_nearest[j]+1; k <= degree[j]; k++)
					 if (open[sorted_location[k][j]])
						 break;
                 second_nearest[j] = k; 
			 } // end if
             // check if second_nearest needs to be updated
			 if (sorted_location[second_nearest[j]][j] == current_facility)
			 {
				 // find new second nearest facility
				 for (k=second_nearest[j]+1; k <= degree[j]; k++)
					 if (open[sorted_location[k][j]])
						 break;
                 second_nearest[j] = k; 
			 } // end if
		 } // end of for j
	  } // end if

   // Now we wish to consider the remaining facilities for closure

   for (i=M; i >= 1; i--) // first skip the facilities with zero y value
      if (temp_y_value[facility_list[i]] > 0.001)
         break;

   // now we can look at the remaining facilities
   for (; i >= 1; i--)
   {
      current_facility = facility_list[i];  // get next facility in sorted list
      saving = facility_cost[current_facility]; // if we close the facility, we will make a saving
      // compute effect on cost of closing the given facility
      for (j=1;j<=N;j++)
	  {
		  // if client j has nowhere to go, we musn't close the facility!
		  if (sorted_location[nearest[j]][j] == current_facility && sorted_assignment_cost[second_nearest[j]][j] == BIG_INT)
         {
			  saving = -1;
			  break;
         }
         if (sorted_location[nearest[j]][j] == current_facility)
		  {
			  saving += sorted_assignment_cost[nearest[j]][j];
	          saving -= sorted_assignment_cost[second_nearest[j]][j];
		  }
	  }
      // check if it is a good idea to close the given facility

	  if (saving > 0)
      {
         open[current_facility] = false; // close the facility
		 number_open --;
		 if (number_open == 1)
			 break;
         current_upper_bound -= saving; // improve the upper bound
         for (j=1;j<=N;j++) // check which clients are involved
		 {
            // check if nearest needs to be updated
			 if (sorted_location[nearest[j]][j] == current_facility)
			 {
				 nearest[j] = second_nearest[j];
				 // find new second nearest facility
				 for (k=second_nearest[j]+1; k <= degree[j]; k++)
					 if (open[sorted_location[k][j]])
						 break;
                 second_nearest[j] = k; 
			 } // end if

             // check if second_nearest needs to be updated
			 if (sorted_location[second_nearest[j]][j] == current_facility)
			 {
				 // find new second nearest facility
				 for (k=second_nearest[j]+1; k <= degree[j]; k++)
					 if (open[sorted_location[k][j]])
						 break;
                 second_nearest[j] = k; 
			 } // end if
		 } // end of for j
	  } // end if
   } // end of for i

   // remember to free up the memory!

   delete facility_list;
   delete open;
   delete nearest;
   delete second_nearest;
   delete temp_y_value;

   // this function returns an upper bound

   return (unsigned)(current_upper_bound+0.1);

} // end of function modified drop

/********************FUNCTION CHECK_INTEGRALITY**********************/

bool check_integrality(double * primal_value)
{
	// if a y variable is fractional, return false immediately
	for (i = 1; i <= M; i++)
      if (primal_value[i-1] > 0.001 && primal_value[i-1] < 0.999)
         return false;

   // if solution is integral, return true

   return true;

} // end of function check_integrality


/********************FUNCTION ITERATED_ROUNDING**********************/

unsigned iterated_rounding(CPXENVptr env, CPXLPptr lp, double * primal_value)
{
   // variable declarations
   int to_round; // the variable to be rounded to 1
   double biggest_fraction; // highest fractional value in LP solution
   double upper_bound; // this function returns an upper bound

   // stuff for CPLEX
   int status = 0; // status
   int cnt = 1; // we round one variable at a time
   int indices[1]; // trivial array containing one variable index
   char lu[1]; // L for lower bound
   double bd[1]; // We round to one
   lu[0] = 'L'; // L for lower bound
   bd[0] = 1.0; // We round to one

   // iteratively round variables to one until the solution is integral

   do
   {
      // find a fractional y variable with highest value
      biggest_fraction = 0.0;
      to_round = 0;
      for (i=1;i<=M;i++)
	     if (primal_value[i-1] > biggest_fraction && primal_value[i-1] < 0.9999)
	     {
		    biggest_fraction = primal_value[i-1];
		    to_round = i;
	     }


      // if solution already integral, break
      if (to_round == 0)
		  break;

      // otherwise fix that y variable to one
	  indices[0] = to_round-1;
      CPXchgbds (env, lp, cnt, indices, lu, bd);

	  // reoptimise the LP
   
	  CPXdualopt(env,lp); // dual simplex probably best
      CPXgetx(env, lp, primal_value, 0, M-1);
   }
   while (true);

   // get upper bound
   CPXgetobjval (env,lp,&upper_bound);

   // this function returns an upper bound

   return (unsigned)(upper_bound+0.1);

} // end of function iterated_rounding


/********************FUNCTION FIRST_REDUCTION**********************/

void first_reduction(unsigned **sorted_assignment_cost, unsigned **sorted_location,
						 unsigned *degree, double *primal_value, double *dual_value, double *reduced_cost)
{
   // variable declaration
   double slope; // used for eliminating x variables
   unsigned passed_so_far; // for a given client, the number of x variables retained so far 

   // run through x variables to see if any can be eliminated using reduced costs
   // update number of edges and degrees

   num_eliminated = 0;
   for (j = 1; j <= N; j++)
   {
      passed_so_far = 0;
      for (k = 1; k <= degree[j]; k++)
	  {
		  // the reduced cost of x_ij is max {0,c_ij - v_j}
         if (sorted_assignment_cost[k][j] > dual_value[j-1])
			 slope = (double)sorted_assignment_cost[k][j] - dual_value[j-1];
		 else
			 slope = 0.0;
		 // if y_i is non-basic at zero, we can also add the reduced cost of y_i
		 if (primal_value[sorted_location[k][j]] < 0.1)
            slope += reduced_cost[sorted_location[k][j]-1];
		 // check if x_ij can be eliminated
         if (slope + lower_bound > best_upper_bound + 0.001) // then we can eliminate
		 {
		    num_eliminated++;
			current_num_edges--;
		 } // end if
		 else // we should retain
		 {
            passed_so_far++;
			sorted_assignment_cost[passed_so_far][j] = sorted_assignment_cost[k][j];
			sorted_location[passed_so_far][j] = sorted_location[k][j];
		 }
	  } // end for k
      // update degree
      degree[j] = passed_so_far;
   } // end for j
} // end of function first_reduction


/********************FUNCTION WRITE_REDUCED_INSTANCE**********************/

void write_reduced_instance(unsigned *facility_cost, unsigned **sorted_assignment_cost, unsigned **sorted_location,
						 unsigned *degree, FILE *filepointer, char *reducedfilename)
{
	// open output file for writing
   if ((filepointer = fopen(reducedfilename, "w"))== NULL)
   {
      printf("\nCannot open output file!\n");
	  getchar();
   }

   // print first row of output file
   fprintf(filepointer,"%u %u s\n",M,N);

   // print facility costs to output file
   for (i=1; i<=M; i++)
      fprintf(filepointer, "%u ", facility_cost[i]);
   fprintf(filepointer,"\n");

   // print client degrees to output file
   for (j=1;j<=N;j++)
      fprintf(filepointer, "%u ", degree[j]);
   fprintf(filepointer,"\n");

// print one row for each client to output file
   for (j = 1; j <= N; j++)
   {
      for (i = 1; i <= degree[j]; i++)
         fprintf(filepointer,"%u %u ", sorted_location[i][j], sorted_assignment_cost[i][j]);
      fprintf(filepointer,"\n");
   }

   // close the output file
   fclose(filepointer);

} // end of function write_reduced_instance


/********************FUNCTION SECOND_REDUCTION**********************/

void second_reduction(unsigned **sorted_assignment_cost, unsigned **sorted_location, unsigned *degree, double *dual_value)
{
   // run through x variables to see if any can be eliminated using dual prices of VUBs

   num_eliminated = 0;
   for (j = 1; j <= N; j++)
   {
      for (k = 1; k <= degree[j]; k++)
	  {
		  // the dual price of the VUB is max {0,v_j-c_ij}
		 // check if x_ij can be eliminated
         if (dual_value[j-1] - sorted_assignment_cost[k][j] > best_upper_bound - lower_bound + 0.001)
		 {
		    num_eliminated++;
			current_num_edges--;
		 } // end if
	  } // end for i
   } // end for j

} // end of function second_reduction

//********************************************************************************
//********** Function WRITE_FINAL_LP_FILE ****************************************
//********************************************************************************

void write_final_LP_file(char *LPfilename, unsigned * facility_cost, unsigned ** sorted_assignment_cost,
						 unsigned ** sorted_location, unsigned * degree, double * dual_value)
{
   // variable declaration
   FILE *filepointer; // pointer for handling file
   
   // open LP file for writing

   if ((filepointer = fopen(LPfilename, "w"))== NULL)
	{
		fprintf(stderr, "Cannot open temporary LP file for writing!\n");
		getchar();
		exit(1);
	}

	// print objective function

	fprintf(filepointer, "minimize %u y1", facility_cost[1]);
	
	for(i=2;i<=M;i++)
	{
		fprintf(filepointer, " + %u y%u", facility_cost[i], i);
		if (i % 10 == 0)
			fprintf(filepointer, "\n");
	}
	fprintf(filepointer, "\n");

   for(j=1;j<=N;j++)
   {
      for(k=1;k<=degree[j];k++)
      {
         // check whether x_ij or y_i should be included
         if (dual_value[j-1] - sorted_assignment_cost[k][j] <= best_upper_bound - lower_bound)
            fprintf(filepointer, " + %u x%ut%u", sorted_assignment_cost[k][j], sorted_location[k][j], j);
		 else // include y_i instead
            fprintf(filepointer, " + %u y%u", sorted_assignment_cost[k][j], sorted_location[k][j]);
         // include line break occasionally
		 if (k % 10 == 0)
			 fprintf(filepointer, "\n");
       }
	   fprintf(filepointer, "\n");
	}

	fprintf(filepointer,"subject to\n");

	// the assignment constraints

   for (j=1;j<=N;j++)
   {
      fprintf(filepointer, "ass%u:", j);
      for (k = 1; k <= degree[j]; k++)
      {	
         // again, check whether x_ij or y_i should be included
         if (dual_value[j-1] - sorted_assignment_cost[k][j] <= best_upper_bound - lower_bound)
            fprintf(filepointer, " + x%ut%u", sorted_location[k][j], j);
		 else // include y_i instead
			 fprintf(filepointer, " + y%u", sorted_location[k][j]);
         // include line break occasionally
		 if (k % 10 == 0)
		    fprintf(filepointer, "\n");
	  }
	  fprintf(filepointer, " = 1\n");
   }

	// only include variable upper bounds if the x variable is still present

   for (j=1;j<=N;j++)
      for (k=1;k<=degree[j];k++)
         if (dual_value[j-1] - sorted_assignment_cost[k][j] <= best_upper_bound - lower_bound)
            fprintf(filepointer, "vub%ut%u: y%u - x%ut%u >= 0\n", sorted_location[k][j], j, sorted_location[k][j], sorted_location[k][j],j);

	// we will want the y variables to be binary

   fprintf(filepointer,"binaries\n");
   for (i = 1; i <= M; i++)
      fprintf(filepointer, "y%u\n", i);

   	// finish off

   fprintf(filepointer,"end");

	// close the LP file
	fclose(filepointer);

}   // End of function write_final_LP_FILE


	/********************MAIN FUNCTION***********************/

int main(int argc, char **argv)
{
   // general variable declarations
   int num_rows, num_cols; // number of rows and columns in the LP
   char instance_type; // will always be 's' for sparse
   FILE *filepointer; // pointer for handling various files
   struct _timeb starttime, endtime;   // time before and after doing dual ascent
   double time;  // total time taken
   bool integral; // true if LP solution is integral

   // stuff for arrays

   unsigned * facility_cost; // costs of building facilities
   unsigned ** sorted_assignment_cost; // costs of assigning clients to locations 
   unsigned ** sorted_location; // list of feasible locations for each client 
   unsigned * degree; // to store degree of each client
   double * primal_value; // optimal solution to primal LP: y variables only
   double * dual_value; // optimal solution to dual LP: assignment constraints only
   double * reduced_cost; // reduced costs: y variables only

   // stuff for CPLEX

   CPXENVptr env = NULL; // environment pointer
   CPXLPptr lp = NULL; // LP pointer
   int status = 0; // status

   // assume default file names unless told otherwise

   char * inputfilename = "splps.txt";
   char * LPfilename = "splp-final.lp";
   char * reducedfilename = "splp-reduced.txt";

  // open input file for reading

   printf("OPENING INPUT FILE %s FOR READING.\n", inputfilename);

   if ((filepointer = fopen(inputfilename, "r"))== NULL)
   {
      printf("\nCannot open input file!\n");
	  getchar();
   }

 // get number of locations and clients

   fscanf(filepointer, "%u %u %c", &M, &N, &instance_type);
   if (instance_type != 's')
	   printf("The instance is not sparse!");

   if (M > MAX_SIZE || N > MAX_SIZE)
   {
      printf("\nProblem size is too large.\n");
	  getchar();
   }

   // allocate memory for the facility costs and degrees

   try
   {
      facility_cost = new unsigned[M+1];
      degree = new unsigned[N+1];
   }
   catch (...)
   {
      printf("\nCan't allocate memory for facility costs and degrees!\n");
	  getchar();
   }

   // read in facility costs from input file

   for (i=1; i<=M; i++)
      fscanf(filepointer, "%u", &facility_cost[i]);

   // read in client degrees and compute maximum degree and total number of edges
      current_num_edges = 0;
      for (j=1; j<=N; j++)
	  {
		  fscanf(filepointer, "%u", &degree[j]);
          current_num_edges += degree[j];	  
	  }
      MAX_DEG = 0;
      for (j=1; j<=N; j++)
      if (degree[j] > MAX_DEG)
         MAX_DEG = degree[j];

   // allocate memory for the assignment costs

   try
   {
      sorted_assignment_cost = new unsigned*[MAX_DEG+2];    // set up rows
      for (i = 0; i <= MAX_DEG+1; i++)
          sorted_assignment_cost[i] = new unsigned[N+1];  // set up columns
      sorted_location = new unsigned*[MAX_DEG+2];    // set up rows
      for (i = 0; i <= MAX_DEG+1; i++)
          sorted_location[i] = new unsigned[N+1];  // set up columns
   }
   catch (...)
   {  
      printf("\nCan't allocate memory for assignment cost and location arrays!\n");
	  getchar();
   }

   // read in assignment costs and their associated locations

      for (j=1; j<=N; j++)
         for (i=1; i<=degree[j]; i++)
            fscanf(filepointer, "%u %u", &sorted_location[i][j], &sorted_assignment_cost[i][j]);

   // fill in missing entries in arrays

   for (j=1; j<=N; j++)
      for (i=degree[j]+1; i<=MAX_DEG+1; i++)
	  {
         sorted_assignment_cost[i][j] = BIG_INT;
         sorted_location[i][j] = BIG_INT;
	  }

   // close the input file

   fclose(filepointer);

   printf("Done. There are %u locations and %u clients.\n", M, N);
   printf("There are %u edges out of a possible %u.\n", current_num_edges, M*N);
   printf("The maximum degree is %u. Press a key.\n", MAX_DEG);
   getchar();

   // write the linear program to a temporary text file

   write_temp_LP_file(LPfilename, facility_cost, sorted_assignment_cost, sorted_location, degree);

   // Initialize the CPLEX environment

   printf("INITIALISING CPLEX.\n");

   env = CPXopenCPLEX(&status);

   if ( env == NULL )
   {
      printf("Could not open CPLEX environment. Exiting.\n");
	  getchar();
      exit(1);
   }

   lp = CPXcreateprob(env, &status, "SPLP");

   if ( lp == NULL )
   {
      printf("Failed to create LP. Exiting.\n");
	  getchar();
      exit(1);
   }

   // Now read the file, and copy the data into the created lp

   status = CPXreadcopyprob(env, lp, LPfilename, NULL);
   if (status)
   {
      printf("Failed to read and copy the problem data. Exiting.\n");
	  getchar();
      exit(1);
   }

   // Check number of rows and columns in the LP

   num_rows = CPXgetnumrows(env,lp);
   num_cols = CPXgetnumcols(env,lp);
   printf("Done. The LP has %d variables and %d constraints. Press a key.\n\n",num_cols,num_rows);
   getchar();

   // get the time before running any algorithms
   _ftime( &starttime);

   // Optimize the problem and obtain solution

   printf("SOLVING THE LP RELAXATION.\n");
   status = CPXdualopt(env,lp); // dual simplex probably best

   if (status)
   {
      printf("Failed to optimize LP! Exiting.\n");
	  getchar();
      exit(1);
   }

   // get objective value

   status = CPXgetobjval (env,lp,&lower_bound);

   if (status)
   {
      printf("Failed to obtain objective value! Exiting.\n");
	  getchar();
      exit(1);
   }

   // print lower bound
   
   printf("Done. Lower bound: %.3f.\n\n",lower_bound);

   // allocate memory for the primal, dual and reduced cost arrays

   try
   {
      primal_value = new double[M];
      dual_value = new double[N];
	  reduced_cost = new double[M];
   }
   catch (...)
   {
      printf("\nCan't allocate memory for LP solution! Exiting.\n");
	  getchar();
      exit(1);
   }

   // get primal solution (y variables only)
   status = CPXgetx(env, lp, primal_value, 0, M-1);
   if (status)
   {
      printf("Failed to obtain primal solution! Exiting.\n");
	  getchar();
      exit(1);
   }

   // If the LP solution is already integral, quit

   integral = check_integrality(primal_value);
   //if (integral)
   //{
   //   printf("The LP relaxation is already integral. So the lower bound is optimal!\n");
   //   exit(1);
   //}

   // get reduced costs of y variables
   status = CPXgetdj(env, lp, reduced_cost, 0, M-1);
   if (status)
   {
      printf("Failed to obtain reduced costs! Exiting.\n");
	  getchar();
      exit(1);
   }

   // Run the modified drop heuristic, dropping facilities in non-decreasing order
   // of y value, to get an initial upper bound.

   printf("RUNNING MODIFIED DROP HEURISTIC.\n");
   best_upper_bound = modified_drop(facility_cost, sorted_assignment_cost, sorted_location, primal_value, degree);
   printf("Done. Upper bound = %u.\n", best_upper_bound);
   printf("Initial percentage gap: %.3f.\n\n", ((best_upper_bound/lower_bound)-1)*100.0);

   // Get dual solution (assignment constraints only)
   status = CPXgetpi(env, lp, dual_value, 0, N-1);
   if (status)
   {
      printf("Failed to obtain dual solution! Exiting.\n");
	  getchar();
      exit(1);
   }

   // run the iterative rounding heuristic and print stuff to screen
   printf("RUNNING ITERATIVE ROUNDING HEURISTIC.\n");
   current_upper_bound = iterated_rounding(env, lp, primal_value);
   printf("Done. Upper bound = %u.\n", current_upper_bound);
   // ensure best upper bound is reported 
   if (current_upper_bound < best_upper_bound)
      best_upper_bound = current_upper_bound; 
   printf("So best upper bound is: %u.\n", best_upper_bound);
   printf("The percentage gap is now %.3f.\n\n", ((best_upper_bound/lower_bound)-1)*100.0);

   // Free up the CPLEX memory

   if ( lp != NULL )
      status = CPXfreeprob (env, &lp);
   if ( status )
      fprintf (stderr, "CPXfreeprob failed, error code %d.\n", status);

   // get the time when the heuristics have finished
   _ftime( &endtime); 

   // print time to screen
   time =(endtime.time+(double)endtime.millitm/1000)-(starttime.time+(double)starttime.millitm/1000);
   printf("Time taken by primal heuristics: %.3f. Press a key.\n", time);
   getchar();

   // get the time again
   _ftime( &starttime);

   // apply first reduction rule, based on reduced costs of x and y variables

   printf("APPLYING FIRST REDUCTION PROCEDURE (BASED ON REDUCED COSTS).\n");
   first_reduction(sorted_assignment_cost, sorted_location, degree, primal_value, dual_value, reduced_cost);
   printf("Done. %u x variables eliminated, %u remaining.\n", num_eliminated, current_num_edges);
   printf("Assuming original instance was dense...\n");
   printf("...we have eliminated a total of %.3f percent of the x variables.\n", 100.0 - 100.0*(double)current_num_edges/((double)M * (double)N) );

   // update maximum degree
   MAX_DEG = 0;
   for (j=1; j<=N; j++)
      if (degree[j] > MAX_DEG)
         MAX_DEG = degree[j];
   printf("The maximum degree is now %u.\n", MAX_DEG);

   // get the time when the first reduction rule has finished
   _ftime( &endtime); 

   // print time to screen
   time =(endtime.time+(double)endtime.millitm/1000)-(starttime.time+(double)starttime.millitm/1000);
   printf("Time taken by first reduction rule: %.3f. Press a key.\n", time);
   getchar();

   // get the time again
   _ftime( &starttime);

   // write the reduced SPLP instance to a file
   printf("WRITING REDUCED SPLP INSTANCE TO FILE %s IN SPARSE FORMAT.\n", reducedfilename);
   write_reduced_instance(facility_cost, sorted_assignment_cost, sorted_location, degree, filepointer, reducedfilename);
   printf("Done. Press a key.\n");
   getchar();

   // apply second reduction rule, based on dual price of variable upper bounds

   printf("APPLYING SECOND REDUCTION PROCEDURE (BASED ON DUAL PRICES).\n");
   second_reduction(sorted_assignment_cost, sorted_location, degree, dual_value);
   printf("Done. %u x variables eliminated, %u remaining.\n", num_eliminated, current_num_edges);
   printf("Assuming original instance was dense...\n");
   printf("...we have eliminated a total of %.3f percent of the x variables.\n", 100.0 - 100.0*(double)current_num_edges/((double)M * (double)N) );

   // get the time when the second reduction rule has finished
   _ftime( &endtime);

   // print time to screen
   printf("Time taken by second reduction rule: %.3f. Press a key.\n", time);
   getchar();

   // write the reduced 0-1 LP to a file
   printf("WRITING REDUCED 0-1 LINEAR PROGRAM TO FILE %s.\n", LPfilename);
   write_final_LP_file(LPfilename, facility_cost, sorted_assignment_cost, sorted_location, degree, dual_value);

   printf("Done. Press a key.\n");
   getchar();

   // free up the memory that has been allocated

   delete facility_cost;
   delete sorted_assignment_cost;
   delete sorted_location;
   delete degree;
   delete primal_value;
   delete dual_value;

   // prompt user to end the program
   printf("\nThe program has finished.");
   getchar();

} // END OF MAIN