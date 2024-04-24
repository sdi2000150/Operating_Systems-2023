#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"


#define MAX_NAME_LENGTH 35              //MAX_NAME_LENGTH for lname & fname

#define INITIAL_NUMBER_OF_BUCKETS 2     //INITIAL_NUMBER_OF_BUCKETS >= 1


/* Auxiliary function for error detection regarding the names given in prompt */
int is_name(char* str) {
    if (str == NULL || *str == '\0') {
        return -1;  //empty string is not a valid name
    }
    int i = 0;
    while (str[i] != '\0') {
        if ((str[i] < 'A' || str[i] > 'Z') && (str[i] < 'a' || str[i] > 'z')) {
            return -1;  //invalid character found
        }
        i++;
    }
    return 0;  //all characters are valid for a name
}

/* Auxiliary function for error detection regarding the integers given in prompt */
int is_integer(char* str, char end) {
    if (str == NULL || *str == '\0') {
        return -1;  //empty string is not an integer
    }
    int i = 0;
    while (str[i] != end) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;  //non-digit character found
        }
        i++;
    }
    return 0;   //all characters are digits, so it's an integer
}


int main(int argc, char* argv[]) {


    /* Linear Hash Table */
    int m = INITIAL_NUMBER_OF_BUCKETS;              //initial number of buckets (defined m = 2)
    if (m <= 0) {
        printf("Malformed INITIAL_NUMBER_OF_BUCKETS value definition, it must be >= 1\n");
        return -1;                                  //exit
    }
    int bucketentries;
    int i = 1;
    bool done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-b") == 0) {           //flag for "bucketentries"
            bucketentries = atoi(argv[i+1]);        //number of keys a bucket has (given by user)
            done = 1;
            if (bucketentries <= 0) {
                printf("Malformed <bucketentries> value given, it must be >= 1\n");
                return -1;                          //exit
            }
        }
        i++;
    }
    LinearHashTable* hashtable;
    hashtable = lht_createtable(m, bucketentries);  // Create Linear Hash Table



    /* Inverted Linked List */
    InvertedLinkedList* invertedlist;
    invertedlist = list_create();                   // Create Inverted Linked List



    /* Insertion of initial set of voters (from file) into the Linear Hash Table */
    FILE *file;
    i = 1;
    done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-f") == 0) {           //flag for file "registeredvoters"
            file = fopen(argv[i+1], "r");           //open the file in read mode
            done = 1;
            if (file == NULL) {
                perror("Malformed Input File from command line, fopen source-file error");
                exit_function(hashtable, invertedlist);     //free the allocated structs
                return -1;                                  //exit
            }
        }
        i++;
    }
    int pin, zipcode;
    char lname[MAX_NAME_LENGTH];
    char fname[MAX_NAME_LENGTH];
    // Read data from the file
    while (fscanf(file, "%d %s %s %d", &pin, lname, fname, &zipcode) == 4) {
        // Insert the entry inside the Linear Hash Table
        if (lht_insertvoter(hashtable, pin, lname, fname, zipcode) == -1) {
            printf("Entry with PIN: %d already exists. Didn't entered\n", pin);
        }
    }
    fclose(file);   //close the file



    /* Functions of mvote, handled according to the formatted output */
    char command[5];    //for the command
    char arg[100];      //for the arguments of the command
    char* token1;
    char* token2;
    char* token3; 
    char* token4;
    char* check;
    // Prompt
    printf("Give me a command\n");
    scanf("%s", command);
    fgets (arg, 100, stdin);
    /* Command Handler */
    while (strcmp(command, "exit") != 0) {

        /* l <pin> */
        if (strcmp(command, "l") == 0) {
            // Get the token <pin>
            token1 = strtok(arg, " ");

            // Check if token is missing - malformed input
            if (*token1 == '\n') {
                printf("Malformed Input (<pin> not given)\n");

            } else {
                // Check for extra tokens, or white space(s) after command - malformed input
                check = strtok(NULL, " ");
                if (check != NULL) {
                    printf("Malformed Input (extra token or space(s) given)\n");

                // Check if <pin> is not valid (not an int) - malformed input
                } else if (is_integer(token1, '\n') == -1) {
                    printf("Malformed Pin (not valid)\n");

                // Call the l_function
                } else {
                    pin = atoi(token1);
                    if (l_function(hashtable, pin) == -1) {
                        printf("Participant %d not in cohort\n", pin);
                    }
                }
            }
            printf("\n");

        /* i <pin> <lname> <fname> <zip> */
        } else if (strcmp(command, "i") == 0) {
            // Get the token <pin>
            token1 = strtok(arg, " ");

            // Get the token <lname>
            token2 = strtok(NULL, " ");

            // Get the token <fname>
            token3 = strtok(NULL, " ");

            // Get the token <zip>
            token4 = strtok(NULL, " ");

            // Check if any token is missing - malformed input
            if (*token1 == '\n'|| token2 == NULL || token3 == NULL || token4 == NULL || *token4 == '\n') {
                printf("Malformed Input (some of the <pin>|<lname>|<fname>|<zip> not given)\n");

            } else {
                // Check for extra tokens, or white space(s) after command - malformed input
                check = strtok(NULL, " ");
                if (check != NULL) {
                    printf("Malformed Input (extra token or space(s) given)\n");

                // Check if <pin>|<lname>|<fname>|<zip> are not valid - malformed input
                } else if (is_integer(token1, '\0') == -1 || is_name(token2) == -1 || is_name(token3) == -1 || is_integer(token4, '\n') == -1) { 
                    printf("%s %s %s %s\n", token1, token2, token3, token4);
                    printf("Malformed Input (some of the <pin>|<lname>|<fname>|<zip> not valid)\n");

                // Insert entry into Linear Hash Table
                } else {
                    pin = atoi(token1);
                    strcpy(lname, token2);
                    strcpy(fname, token3);
                    zipcode = atoi(token4);
                    if (lht_insertvoter(hashtable, pin, lname, fname, zipcode) == -1) {
                        printf("%d already exist\n", pin);
                    } else {
                        printf("Inserted %d %s %s %d %d\n", pin, lname, fname, zipcode, 0); //when inserted, an entry has not voted (status = 0, 0 means "N")
                    }
                }
            }
            printf("\n");

        /* m <pin> */
        } else if (strcmp(command, "m") == 0) {
            // Get the token <pin>
            token1 = strtok(arg, " ");

            // Check if token is missing - malformed input
            if (*token1 == '\n') {
                printf("Malformed Input (<pin> not given)\n");

            } else {
                // Check for extra tokens, or white space(s) after command - malformed input
                check = strtok(NULL, " ");
                if (check != NULL) {
                    printf("Malformed Input (extra token or space(s) given)\n");

                // Check if <pin> is not valid (not an int) - malformed input
                } else if (is_integer(token1, '\n') == -1) {
                    printf("Malformed Pin (not valid)\n");

                // Call the m_function
                } else {
                    pin = atoi(token1);
                    if (m_function(hashtable, pin, invertedlist) == -1) {
                        printf("%d does not exist\n", pin);
                    }
                }
            }
            printf("\n");

        /* bv <fileofkeys> */
        } else if (strcmp(command, "bv") == 0) {
            // Get the token <fileofkeys>
            token1 = strtok(arg, " \n");

            // Check if token is missing - malformed input
            if (token1 == NULL) {
                printf("Malformed Input (<fileofkeys> not given)\n");

            } else {
                // Check for extra tokens, or white space(s) after command - malformed input
                check = strtok(NULL, " ");
                if (check != NULL) {
                    printf("Malformed Input (extra token or space(s) given)\n");

                } else {
                    // Open the file in read mode
                    file = fopen(token1, "r");
                    if (file == NULL) {
                        printf("%s could not be opened\n", token1);
                    } else {
                        // Read data from the file
                        while (fscanf(file, "%d", &pin) == 1) {
                            // Call the m_function for each entry
                            if (m_function(hashtable, pin, invertedlist) == -1) {
                                printf("%d does not exist\n", pin);
                            }
                        }
                        // Close the file
                        fclose(file);
                    }
                }
            }
            printf("\n");

        /* v */
        } else if (strcmp(command, "v") == 0) {
            int have_voted = 0;     //sum of participants who have voted (status = 1, meaning "Y"), initialized to 0
            int sum_of_all = 0;     //sum of all participants, initialized to 0

            // Call the v_and_perc_function, from which we need only the have_voted counter
            v_and_perc_function(hashtable, &have_voted, &sum_of_all);
            printf("Voted So Far %d\n", have_voted);
            printf("\n");

        /* perc */
        } else if (strcmp(command, "perc") == 0) {
            int have_voted = 0;     //sum of participants who have voted (status = 1, meaning "Y"), initialized to 0
            int sum_of_all = 0;     //sum of all participants, initialized to 0

            // Call the v_and_perc_function, from which we need both the have_voted and the sum_of_all counters
            v_and_perc_function(hashtable, &have_voted, &sum_of_all);
            int voted_perc = ((double)have_voted/(double)sum_of_all) * 100;
            printf("%d/%d\n", voted_perc, 100);
            printf("\n");

        /* z <zipcode> */
        } else if (strcmp(command, "z") == 0) {
            // Get the token <zipcode>
            token1 = strtok(arg, " ");

            // Check if token is missing - malformed input
            if (*token1 == '\n') {
                printf("Malformed Input (<zipcode> not given)\n");

            } else {
                // Check for extra tokens, or white space(s) after command - malformed input
                check = strtok(NULL, " ");
                if (check != NULL) {
                    printf("Malformed Input (extra token or space(s) given)\n");

                // Check if <zipcode> is not valid (not an int) - malformed input
                } else if (is_integer(token1, '\n') == -1) {
                    printf("Malformed Zipcode (not valid)\n");

                // Call the z_function
                } else {
                    zipcode = atoi(token1);
                    int number_voted_in_zipcode = z_function(invertedlist->head, zipcode);
                    if (number_voted_in_zipcode != -1) {
                        printf("%d voted in %d\n", number_voted_in_zipcode, zipcode);
                    }
                }
            }
            printf("\n");
            
        /* o */
        } else if (strcmp(command, "o") == 0) {
            // Call the o_function
            o_function(invertedlist);
            printf("\n");

        /* print */
        } else if (strcmp(command, "print") == 0) {
            // Call the print_function
            print_function(hashtable, invertedlist);
            printf("\n");
        
        } else {
            printf("Command given doesn't exist. Try again\n");
            printf("\n");
        }

        // Prompt
        printf("Give me a command\n");
        scanf("%s", command);
        fgets (arg, 100, stdin);
    }
    /* exit */
    // Call the exit_function, in order to free all the allocated space
    exit_function(hashtable, invertedlist);


    return 0;
}


