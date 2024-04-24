#include <stdlib.h>
#include <string.h>     //strcmp() usage (by MergeSort) 
#include <signal.h>     //signal() usage (for "SIGUSR1" and "SIGUSR2" handling)
#include "mysort_prototypes.h"

//global definition of USR1_counter:
int USR1_counter = 0;   //number of USR1 signals sent to Coordinator (initialized to 0)
//global definition of USR2_counter:
int USR2_counter = 0;   //number of USR2 signals sent to Coordinator (initialized to 0)

//Signal USR1 handler function
void USR1_handler() {
    signal(SIGUSR1, USR1_handler);  //reestablish the signal handler
    USR1_counter++; //number of USR1 signals sent to Coordinator increases by 1
}

//Signal USR2 handler function
void USR2_handler() {
    signal(SIGUSR2, USR2_handler);  //reestablish the signal handler
    USR2_counter++; //number of USR2 signals sent to Coordinator increases by 1
}


//MergeSort implementation, for merging multiple sorted arrays into one sorted array
Record* MergeSort(Record** records, int* records_sizes, int num_of_arrays) {
    //Calculate the total size ("merged_size") needed for the merged array by summing up the sizes of all the individual arrays
    int merged_size = 0;
    for (int j = 0; j < num_of_arrays; j++) {
        merged_size += records_sizes[j];
    }

    Record* merged_records = malloc(merged_size*sizeof(Record));

    //Array "indexes" helps to keep track of the current index in each individual array (initialized to 0)
    int indexes[num_of_arrays];
    for (int i = 0; i < num_of_arrays; i++) {
        indexes[i] = 0;
    }

    //Iteration to fill the final merged array ("merged_records")
    for (int j = 0; j < merged_size; j++) {
        
        //Each time search for the smallest record among the multiple arrays
        //and update the "min_index" to the index of the array containing the smallest record based on lname (and secondarily on fname or ID)
        int min_index = -1; //"min_index" initialized to -1 as an indicator
        for (int i = 0; i < num_of_arrays; i++) {

            //Check if the current index is within the array's bounds
            if (indexes[i] < records_sizes[i]) {

                //If first iteration, make "min_index" equal to i
                if (min_index == -1) { 
                    min_index = i;

                //Else compare the smallest record of the array i based on lname, fname, and ID fields to find if it is smaller than current smallest one
                } else if (strcmp(records[i][indexes[i]].lname, records[min_index][indexes[min_index]].lname) < 0) {
                    min_index = i;
                } else if (strcmp(records[i][indexes[i]].lname, records[min_index][indexes[min_index]].lname) == 0) {
                    if (strcmp(records[i][indexes[i]].fname, records[min_index][indexes[min_index]].fname) < 0) {
                        min_index = i;
                    } else if (strcmp(records[i][indexes[i]].fname, records[min_index][indexes[min_index]].fname) == 0) {
                        if (records[i][indexes[i]].ID < records[min_index][indexes[min_index]].ID) {
                            min_index = i;
                        }
                    }
                } 

            }
        }

        //After finding the smallest record, 
        if (min_index != -1) {
            merged_records[j] = records[min_index][indexes[min_index]]; //copy that record to the merged array
            indexes[min_index]++;   //increment the index of the array from which the smallest record was selected
        }
    }

    //Return the merged_records array containing the merged and sorted records from all the input arrays
    return merged_records;
}


//free_function implementation, for freeing all mallocs, before each exit
void free_function(int** starting_pos, int** ending_pos, int k, int** merger_pipe, int*** sorter_pipe, Record** merged_records, Record*** records, long double*** times) {
    int i, j;
    // Freeing the ARRAY for the 2 TIMES of every sorter's execution
    for (i = 0; i < k; i++) {
        for (j = 0; j < k-i; j++) {
            free(times[i][j]);
        }
        free(times[i]);
    }
    free(times);

    // Freeing the TWO ARRAYS for the SORTED RECORDS returned by children-processes
    for (i = 0; i < k; i++) {
        for (j = 0; j < k-i; j++) {
            free(records[i][j]);
        }
        free(records[i]);
        free(merged_records[i]);
    }
    free(records);
    free(merged_records);
    
    // Freeing the TWO ARRAYS for the PIPES that were created
    for (i = 0; i < k; i++) {
        for (j = 0; j < k-i; j++) {
            free(sorter_pipe[i][j]);
        }
        free(sorter_pipe[i]);
        free(merger_pipe[i]);
    }
    free(sorter_pipe);
    free(merger_pipe);

    // Freeing the TWO ARRAYS keeping the STARTING & ENDING POSITION in file, in which each Sorter sorted
    for (i = 0; i < k; i++) {
        free(ending_pos[i]);
        free(starting_pos[i]);
    }
    free(ending_pos);
    free(starting_pos);
}