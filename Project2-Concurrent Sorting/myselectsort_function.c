#include <string.h>     //strcmp() usage (by SelectSort) 
#include "myselectsort_prototype.h"

/* Sorting Algorithm: Select Sort */
void SelectSort(Record* records, int num_of_records) { 
    Record temp;
    int i, j, min;

    //Traverse the array, and each time move to the first unsorted position the smallest element
    for (i = 1; i < num_of_records; i++) {
        min = i-1; //let current minimum be the i-1 element

        //Traverse the unsorted region starting from i to the end of the array.
        for (j = i; j < num_of_records; j++) {
            
            //Check if any element is less than so far minimum and make it the new minimum
            if (strcmp(records[j].lname, records[min].lname) < 0) { // SORT BASED ON LAST NAME
                min = j;
            } else if (strcmp(records[j].lname, records[min].lname) == 0) { // IF EQUAL, SORT BASED ON FIRST NAME
                if (strcmp(records[j].fname, records[min].fname) < 0) {
                    min = j;
                } else if (strcmp(records[j].fname, records[min].fname) == 0) { // IF EQUAL TOO, SORT BASED ON ID
                    if (records[j].ID < records[min].ID) {
                        min = j;
                    }
                }
            }
        }
        //Exchange minimum with i-1 element
        temp = records[i-1];
        records[i-1] = records[min];
        records[min] = temp;
    } 
}