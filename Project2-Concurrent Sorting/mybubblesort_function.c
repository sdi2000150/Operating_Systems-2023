#include <string.h>     //strcmp() usage (by BubbleSort) 
#include "mybubblesort_prototype.h"

/* Sorting Algorithm: Bubble Sort */
void BubbleSort(Record* records, int num_of_records) {
    int i, j;
    Record temp;
    for (i = 1; i <= (num_of_records-1); i++) {
        for (j = (num_of_records-1); j >= i; j--) {
            // SORT BASED ON LAST NAME
            if (strcmp(records[j].lname, records[j-1].lname) < 0) {
                temp = records[j-1];
                records[j-1] = records[j];
                records[j] = temp;
            } else if (strcmp(records[j].lname, records[j-1].lname) == 0) {
                // IF EQUAL, SORT BASED ON FIRST NAME
                if (strcmp(records[j].fname, records[j-1].fname) < 0) {
                    temp = records[j-1];
                    records[j-1] = records[j];
                    records[j] = temp;
                } else if (strcmp(records[j].fname, records[j-1].fname) == 0) {
                    // IF EQUAL TOO, SORT BASED ON ID
                    if (records[j].ID < records[j-1].ID) {
                        temp = records[j-1];
                        records[j-1] = records[j];
                        records[j] = temp;
                    }
                }
            }
        }
    }
}