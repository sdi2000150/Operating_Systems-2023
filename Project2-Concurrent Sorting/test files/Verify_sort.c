// Author: A. Delis ad@di.uoa.gr
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SIZEofBUFF 20
#define SSizeofBUFF 6

typedef struct{
	int  	ID;
	char 	lname[SIZEofBUFF];
	char 	fname[SIZEofBUFF];
	char	zipcode[SSizeofBUFF];
} MyRecord;




/* Sorting Algorithm: Bubble Sort */
void BubbleSort(MyRecord* records, int num_of_records) {
    int i, j;
    MyRecord temp;
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

int main (int argc, char** argv) {
   FILE *fpb;

   long lSize;
   int numOfrecords, i;
   
   if (argc!=2) {
      	printf("Correct syntax is: %s BinaryFile\n", argv[0]);
      	return(1);
   	}
   fpb = fopen (argv[1],"rb");
   if (fpb==NULL) {
      	printf("Cannot open binary file\n");
      	return 1;
   	}
   
   // check number of records
   fseek (fpb , 0 , SEEK_END);
   lSize = ftell (fpb);
   rewind (fpb);
   numOfrecords = (int) lSize/sizeof(MyRecord);
    MyRecord* records = malloc(numOfrecords*sizeof(MyRecord));
   
//    printf("Records found in file %d \n", numOfrecords);
   
   for (i=0; i<numOfrecords ; i++) {
      	fread(&records[i], sizeof(MyRecord), 1, fpb);
   	}

    BubbleSort(records, numOfrecords);
   
    //Writing sorted records into file-desc 1, which is pipefd[Write] when executed by ./mysort parent (or stdout when executed by its own)
    for (int i = 0; i < numOfrecords; i++) {
        printf("%-12s %-12s %-6d %s\n", records[i].lname, records[i].fname, records[i].ID, records[i].zipcode);
    }   
    free(records);

   fclose (fpb);
}
