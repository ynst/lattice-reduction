//********************************************************************************
//********************************************************************************
// SPLP_create_and_reduce.cpp
// Constructs random SPLP instances in which M facility and N client locations
// are uniformly distributed points on a square.
// Facility costs are uniformly distributed within a specified range, whose limits
// are expressed as a proportion of sqrt{N} times the width of the square.
// Assignment costs are Euclidean distances.
// All key parameters are listed as global variables near the start.
// Once the instance has been created, the maximum degree is computed, the first
// pre-processing procedure is applied, and the remaining locations for each client
// are placed in an array. The locations (but not the costs) are then sorted, and
// the second pre-procesing routine is applied. The reduced instance is then output
// in "sparse" format:
//    <number of locations> <number of clients> <'s', for 'sparse'>
//    <row containing one cost per facility>
//    <row containing the degree of each client>
//    <one row for each client j, containing pairs i, c_ij >
// Generally, we use i for location indices and j for client indices.
//********************************************************************************
//********************************************************************************

// PREPROCESSING DIRECTIVES 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>

#define BIG_INT 1000000

// KEY GLOBAL VARIABLES

unsigned M = 1000; // number of locations
unsigned N = 1000; // number of clients
unsigned width = 5000; // width and height of the square
unsigned random_seed = 1; // seed for random number generator
double lower_ratio = 0.001; // lowest facility cost, as a proportion of sqrt{n}.
double upper_ratio = 0.001; // highest facility cost, as a proportion of sqrt{n}.

/**********************FUNCTION COMPUTE_DEGREES*************************/

unsigned compute_degrees(unsigned *facility_cost, unsigned **assignment_cost, unsigned *degree)
{
   // variable declarations
   unsigned i, j; // general-purpose counters
   unsigned best_cost; // the best value of f_i + c_{ij} found so far for client j
   unsigned max_degree; // to store maximum client degree

   // initialise max_degree at zero
   max_degree = 0;

   // for each client, compute degree, and update max_degree if necessary
   for (j = 1; j <= N; j++)
   {
      // find the facility i that minimises f_i + c_{ij}
      best_cost = BIG_INT;
      for (i = 1; i <= M; i++)
		  if (facility_cost[i] + assignment_cost[i][j] < best_cost)
			  best_cost = facility_cost[i] + assignment_cost[i][j];
      // now compute degree
	  degree[j] = M;
      for (i = 1; i <= M; i++)
         if (assignment_cost[i][j] > best_cost)  // if variable can be eliminated
            degree[j]--; // update degree of client j
      // check if max_degree needs updating
      if (degree[j] > max_degree)
         max_degree = degree[j];
   } // end of for j

   return max_degree;
}
/**********END OF FUNCTION COMPUTE_DEGREES**********************************/


/********************FUNCTION FAST_PREPROCESS**********************/

unsigned fast_preprocess(unsigned *facility_cost, unsigned **assignment_cost,
						 unsigned **sorted_location, unsigned *degree)
{
   // variable declarations
   unsigned i, j, k; // general-purpose counters
   unsigned best_cost; // the current minimum value of f_i + c_{ij}
   unsigned num_eliminated; // number of x variables eliminated by the procedure

   // initially no variables have been eliminated
   num_eliminated = 0;
   
   // scan each client in turn
   for (j = 1; j <= N; j++)
   {
     // find the facility i that minimises f_i + c_{ij}
      best_cost = BIG_INT;
      for (i = 1; i <= M; i++)
		  if (facility_cost[i] + assignment_cost[i][j] < best_cost)
			  best_cost = facility_cost[i] + assignment_cost[i][j];
	  // now fill up the location array (will be sorted later)
	  k = 1; // position in location array
      for (i = 1; i <= M; i++)
	  {
         if (assignment_cost[i][j] <= best_cost) // if the edge should be retained
         {
            sorted_location[k][j] = i;
			k++;
		  } // end if
		 else // the edge should not be retained
			num_eliminated++;
	  } // end of for i
   } // end of for j

   // this function returns the number of edges that have been eliminated
   return num_eliminated;

} // end of function fast_preprocess


/********************FUNCTION QUICKSORT**********************/

void Quicksort(unsigned *ArrayIndexes,	unsigned *ArrayCoefs, int FirstIndex, int LastIndex)
{
    int KeyElement=0;
    int i=0;
    int j=0;
    int t=0;
    unsigned ValueLastElement = 0;
 
   if(FirstIndex < LastIndex)
    {
       ValueLastElement = ArrayCoefs[ArrayIndexes[LastIndex-1]];
	   i = FirstIndex-2;
	   for(j = FirstIndex-1; j < LastIndex-1; j++)
	   {
		  if(ArrayCoefs[ArrayIndexes[j]] < ValueLastElement)
		  {
			 i=i+1;
			 t = ArrayIndexes[i];
			 ArrayIndexes[i] = ArrayIndexes[j];
			 ArrayIndexes[j] = t;
		  }	
	   }
	   t = ArrayIndexes[i+1];
	   ArrayIndexes[i+1] = ArrayIndexes[LastIndex-1];
	   ArrayIndexes[LastIndex-1] = t;
	   KeyElement = i+2;
	   Quicksort(ArrayIndexes, ArrayCoefs, FirstIndex, KeyElement-1);
       Quicksort(ArrayIndexes, ArrayCoefs, KeyElement+1, LastIndex);
    }
} // end of function quicksort


/********************FUNCTION SORT_LOCATIONS**********************/

void sort_locations(unsigned **assignment_cost, unsigned **sorted_location, unsigned *degree, unsigned max_degree)
{
   // variable declarations
   unsigned j, k; // j for client, k for "level"
   unsigned * sorting_indices;
   unsigned * sorting_coefs; // these arrays are for the quicksort routine
   unsigned * original_locations;

   // allocate memory for the arrays used by quicksort function

   try
   {
      sorting_indices = new unsigned[max_degree+1];
      sorting_coefs = new unsigned[max_degree+1];
      original_locations = new unsigned[max_degree+1];
   }
   catch (...)
   {
      printf("\nCan't allocate memory for arrays used for quicksort\n");
      getchar();
   }

   // Sort the locations for each client

   for (j = 1; j <= N; j++)
   {
      if (degree[j] > 1)
      {
         // fill up the temporary arrays for the given client
         for (k = 1; k <= degree[j]; k++)
         {
            original_locations[k] = sorted_location[k][j];
            sorting_indices[k-1] = k-1;
            sorting_coefs[k-1] = assignment_cost[sorted_location[k][j]][j];
         }
      // call quicksort
      Quicksort(sorting_indices, sorting_coefs, 1, degree[j]);
      // put the sorted locations into the array that we actually want to use
      for (k = 1; k <= degree[j]; k++)
         sorted_location[k][j] = original_locations[sorting_indices[k-1]+1];
	   } // end if
   } // end for j

   // free up memory
   delete sorting_indices;
   delete sorting_coefs;
   delete original_locations;

} // end of function sort_locations


/********************FUNCTION ALPHA_CHECK**********************/

bool alpha_check(unsigned *facility_cost, unsigned **assignment_cost, unsigned **sorted_location,
				 unsigned *degree, unsigned k, unsigned alpha_level, bool * facility_open,
				 unsigned * best_assignment_cost)
{
  // This function returns "true" if some elimination can take place
  // for the given client k and the given alpha_level

  // variable declarations
   unsigned i, j; // general-purpose counters
   unsigned saving; // stores the potential saving for a given facility
   unsigned current_facility;

   // only the facilities that are far away from client k are open

   for (i = 1; i <= M; i++)
      facility_open[i] = true;
   for (i = 1; i < alpha_level; i++)
      facility_open[sorted_location[i][k]] = false;

   // compute cost of assigning each client to nearest open facility

   for (j = 1; j <= N; j++)
   {
      // try to find nearest open facility to client j
      for (i = 1; i <= degree[j]; i++)
	  {
         if (facility_open[sorted_location[i][j]])
			 break;
	  }
      // if client j has nowhere to go, we can exit imediately
      if (i > degree[j])
		 return true;
      if (i == degree[j] && !facility_open[sorted_location[i][j]])
		 return true;
	  // otherwise, find cost of assigning client j to nearest open facility
	  best_assignment_cost[j] = assignment_cost[sorted_location[i][j]][j];
   } // end of for j

   // check each closed facility to see if a saving could be made by opening it

   for (i = 1; i < alpha_level; i++)
   {
      saving = 0;
	  current_facility = sorted_location[i][k];
	  // compute the potential saving
      for (j = 1; j <= N; j++)
         if (best_assignment_cost[j] > assignment_cost[current_facility][j])
		  saving = saving + best_assignment_cost[j] - assignment_cost[current_facility][j];
	  // if the net saving is positive, we can exit immediately
      if (facility_cost[current_facility] <= saving)
		 return true;
   } // end of for i

   // if we have had no success by now, return false
   return false;

} // end of function alpha_check


/********************FUNCTION SLOW_PREPROCESS**********************/

unsigned slow_preprocess(unsigned * facility_cost, unsigned ** assignment_cost,
	unsigned ** sorted_location, unsigned *degree)
{
   // variable declarations
   unsigned j; // client counter
   unsigned alpha_level, alpha_LB, alpha_UB; // alpha level and lower and upper bounds on it
   unsigned num_eliminated; // number of x variables eliminated
   bool status; // true if alpha check successful
   bool * facility_open; // for the alpha checks. True if facility open
   unsigned * best_assignment_cost; // for the alpha checks. Cost of assigning client to nearest open facility

   // allocate memory for the arrays

   try
   {
      facility_open = new bool[M+1];
	  best_assignment_cost = new unsigned[N+1];
   }
   catch (...)
   {
      printf("\nCan't allocate memory for arrays in function slow_preprocess!\n");
	  getchar();
   }

   // initially no variables have been eliminated
   num_eliminated = 0;

   // do an alpha search for each client
   for (j = 1; j <= N; j++)
   {
      if (j % 100 == 0)
		  printf("Alpha checks on client %u.\n", j);
	  if (degree[j] > 1)
      {
		  // initial limits on the alpha level
         alpha_LB = 2;
	     alpha_UB = degree[j]+1;
         alpha_level = (alpha_UB + alpha_LB)/2; // try moving halfway from lower to upper
		 // we do binary search to pin down the right alpha level
         while (alpha_UB > alpha_LB && alpha_level <= degree[j])
         {
		    status = alpha_check(facility_cost, assignment_cost, sorted_location,
				degree, j, alpha_level, facility_open, best_assignment_cost);
            // check whether lower level should go up, or upper level should come down
            if (status)
               alpha_UB = alpha_level;
	        else
		       alpha_LB = alpha_level+1;
		    alpha_level = (alpha_UB + alpha_LB)/2; // try moving halfway from lower to upper
         } // end while
		 // eliminate variables if possible
		 if (alpha_level <= degree[j])
	     {
		    num_eliminated = num_eliminated + degree[j] - alpha_level + 1;
            degree[j] = alpha_level-1;
	     } // end inner if
	  } // end outer if
   } // end for j

   // free up memory
   delete facility_open;
   delete best_assignment_cost;

   // this function returns the number of eliminated variables
   return num_eliminated;

} // end of function slow_preprocess


/********START OF MAIN FUNCTION**********************************************/

int main()
{

// variable declarations

unsigned facility_lower, facility_upper; // to define range of facility costs
unsigned xdist, ydist; // horizontal and vertical distance between a client and a facility
unsigned max_degree; // maximum degree over all clients
unsigned num_just_eliminated, num_remaining; // to keep track of number of edges eliminated
unsigned i,j,k; // general-purpose counters
double quantity; // quantity before rounding
double total_time; // time taken by preprocessing routine
double percentage_remaining; // percentage of edges not eliminated
char *output_file_name; // to store name of output file
char *degree_file_name; // to store name of temporary degree file
FILE *fout;   // output file pointer
clock_t start_time, end_time; // time at start and end of preprocessing routine

// variable initialisations

quantity = sqrt(double (M))*(double (width))*lower_ratio; // smallest facility cost, unrounded
facility_lower = unsigned (quantity); // smallest facility cost, rounded
quantity = sqrt(double (M))*(double (width))*upper_ratio; // largest facility cost, unrounded
facility_upper = unsigned (quantity); // highest facility cost, rounded
output_file_name = "SPLP1s.txt";
degree_file_name = "temp_degree_file.txt";
srand(random_seed);

// stuff for arrays

double * facility_x; // x co-ordinates of locations
double * facility_y; // y co-ordinates of locations
double * client_x; // x co-ordinates of clients
double * client_y; // y co-ordinates of clients
unsigned * facility_cost; // cost of opening facilites
unsigned * degree; // will store degrees of clients in the reduced instance
unsigned * * assignment_cost; // stores assignment costs --- never sorted
unsigned * * sorted_location; // stores list of locations for each client --- gets sorted

// print some stuff to screen

printf("PROBLEM DATA:\n", M, N);
printf("M = %u and N = %u.\n", M, N);
printf("Square size = %u.\n", width);
printf("Lower and upper facility cost ratios: %.3f and %.3f.\n", lower_ratio, upper_ratio);
printf("Random number seed: %u.\n", random_seed);
printf("Press any key to continue.\n");
getchar();

printf("CREATING SPLP INSTANCE.\n", M, N);

// allocate memory for 1-dimensional arrays

try
{
   facility_x = new double[M+1];
   facility_y = new double[M+1];
   client_x = new double[N+1];
   client_y = new double[N+1];
   facility_cost = new unsigned[M+1];
}
catch (...)
{
   printf("\nCan't allocate memory for coordinate and facility cost arrays\n");
   getchar();
}

// create random facility costs

for (i=1;i<=M;i++){
   facility_cost[i] = facility_lower+(unsigned)((facility_upper-facility_lower+1)*rand()/(RAND_MAX+1.0));
}

// create random co-ordinates

for (i=1;i<=M;i++)
{   
	facility_x[i]= (width+1)*(float)rand()/(RAND_MAX+1.0);
   facility_x[i]-= (unsigned) facility_x[i];
	facility_y[i]= (width+1)*(float)rand()/(RAND_MAX+1.0);
   facility_y[i]-= (unsigned) facility_y[i];
}

for (j=1;j<=N;j++)
{   
	client_x[j]= (width+1)*rand()/(RAND_MAX+1.0);
   client_x[i]-= (unsigned) client_x[i];
	client_y[j]= (width+1)*rand()/(RAND_MAX+1.0);
   client_y[i]-= (unsigned) client_y[i];
}

// allocate memory for assignment cost array

try
{
   assignment_cost = new unsigned*[M+1];    // set up rows
   for (i = 0; i <= M+1; i++)
       assignment_cost[i] = new unsigned[N+1];  // set up columns
}
catch (...)
{
   printf("\nCan't allocate memory for assignment cost array!\n");
   getchar();
}

// fill up assignment cost array
for (i = 1; i <= M; i++)
   for (j = 1; j <= N; j++)
   {
         xdist = facility_x[i] - client_x[j];
         ydist = facility_y[i] - client_y[j];
         assignment_cost[i][j] = (unsigned)sqrt((double)(xdist*xdist+ydist*ydist));
   }

// we no longer need the coordinate arrays

delete facility_x;
delete facility_y;
delete client_x;
delete client_y;

// The instance has been created
printf("Done. Press any key to continue.\n");
getchar();

// Next, it is helpful to compute the client degrees

// Allocate memory for client degree array

try
{
   degree = new unsigned[M+1];
}
catch (...)
{
   printf("\nCan't allocate memory for client degree array\n");
   getchar();
}

// Compute client degrees *before* actually doing the preprocessing

max_degree = compute_degrees(facility_cost, assignment_cost, degree);

// allocate memory for the location array

try
{
   sorted_location = new unsigned*[max_degree+2];    // set up rows
   for (i = 0; i <= max_degree+1; i++)
       sorted_location[i] = new unsigned[N+1];  // set up columns
}
catch (...)
{  
   printf("\nCan't allocate memory for the location array!\n");
   getchar();
}

// perform the first preprocessing procedure and time it

printf("\nSTARTING THE FAST PREPROCESSING PROCEDURE.\n");
start_time = clock();
num_just_eliminated = fast_preprocess(facility_cost, assignment_cost, sorted_location, degree);
end_time = clock();
total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

// write to screen

num_remaining = M*N - num_just_eliminated;
printf("Finished in %.3f seconds.\n", total_time);
printf("Percentage of edges eliminated: %.3f.\n", (double) num_just_eliminated*100/(M*N));
printf("%u edges remaining.\n", num_remaining);
printf("The maximum degree is now %u.\n", max_degree);
printf("Press any key to continue.\n");
getchar();

// sort the locations for each client

printf("\nSORTING REMAINING LOCATIONS FOR EACH CLIENT.\n");
sort_locations(assignment_cost, sorted_location, degree, max_degree);
printf("Done. Press any key to continue.\n");
getchar();

// THEN WE NEED TO APPLY THE SECOND, SLOWER PRE-PROCESSING PROCEDURE

printf("\nSTARTING THE SLOW PREPROCESSING PROCEDURE.\n");
start_time = clock();
num_just_eliminated = slow_preprocess(facility_cost, assignment_cost, sorted_location, degree);
end_time = clock();
total_time = (end_time - start_time) / (double)CLOCKS_PER_SEC;

// compute the new maximum degree

max_degree = 0;
for(j = 1; j <= N; j++)
   if (degree[j] > max_degree)
      max_degree = degree[j];

// write to screen

num_remaining -= num_just_eliminated;
printf("Finished in %.3f seconds.\n", total_time);
percentage_remaining = 100.0*(double)num_remaining/(M*N);
printf("Cumulative percentage of edges eliminated: %.3f.\n", 100.0-percentage_remaining);
printf("%u edges still remaining.\n", num_remaining);
printf("The maximum degree is now %u.\n", max_degree);
printf("Press any key to continue.\n");
getchar();

// write the reduced instance into a file

printf("WRITING INSTANCE TO FILE.\n");

// open output file for writing

if ((fout = fopen(output_file_name, "w")) == NULL)
{
   printf("\nCannot open output file!\n");
   getchar();
}


// print first row of output file

fprintf(fout,"%u %u s\n",M,N);

// print facility costs to output file

for (i=1; i<=M; i++)
   fprintf(fout, "%u ", facility_cost[i]);
fprintf(fout,"\n");

// print client degrees to output file

for (j=1;j<=N;j++)
   fprintf(fout, "%u ", degree[j]);
fprintf(fout,"\n");

// print list of locations and assignment costs

for (j=1;j<=N;j++)
{
	for (k=1;k<=degree[j];k++)
     fprintf(fout,"%u %u ", sorted_location[k][j], assignment_cost[sorted_location[k][j]][j]);
   fprintf(fout,"\n");
} // end of for j

// close output file
   fclose(fout);

printf("Done. Press any key to continue.\n");
getchar();

// free up memory

delete facility_cost;
//delete assignment_cost;
delete degree;
delete sorted_location;

return 1;
}
/********END OF MAIN FUNCTION**********************************************/