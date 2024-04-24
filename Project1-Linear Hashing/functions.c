#include <stdio.h>
#include <stdlib.h>
#include "functions.h"


/**************************************** Functions the program can execute by command line ****************************************/

/*////////////////////////////////////////////////////////   1) l <pin>   /////////////////////////////////////////////////////////*/
/* Function for "l <pin>" command */
int l_function(LinearHashTable* hashtable, int pin) {
    int key = pin % hashtable->h_curr;      //h_i(k) = k % 2^i*m   
    if (key < hashtable->pointer) {         //if smaller than the current position of p
        key = pin % hashtable->h_next;      //h_i+1(k) = k % 2^(i+1)*m
    }

    // Search the bucket to check if entry exists
    for (int j = 0; j < hashtable->buckets[key]->bucket_size; j++) {
        if (hashtable->buckets[key]->bucketentries[j] == pin) {
            printf("%d %s %s %d %d\n", hashtable->buckets[key]->pvoter[j]->PIN, hashtable->buckets[key]->pvoter[j]->lname, 
            hashtable->buckets[key]->pvoter[j]->fname, hashtable->buckets[key]->pvoter[j]->zipcode, hashtable->buckets[key]->pvoter[j]->status);
            return 0;   //entry was found
        }
    }

    // Search the overflown pages too, to check if entry exists
    if (hashtable->buckets[key]->overflown != NULL) {
        return l_check_overflown(hashtable, hashtable->buckets[key]->overflown, pin);
    }

    return -1;   //entry was not found
}

/* Auxiliary function (recursive) used by l_function */
int l_check_overflown(LinearHashTable* hashtable, Bucket* overflown_page, int pin) {
    for(int j = 0; j < overflown_page->bucket_size; j++) {
        if (overflown_page->bucketentries[j] == pin) {
            printf("%d %s %s %d %d\n", overflown_page->pvoter[j]->PIN, overflown_page->pvoter[j]->lname, 
            overflown_page->pvoter[j]->fname, overflown_page->pvoter[j]->zipcode, overflown_page->pvoter[j]->status);
            return 0;   //entry was found
        }
    }

    if (overflown_page->overflown != NULL) {
        return l_check_overflown(hashtable, overflown_page->overflown, pin);
    }

    return -1;   //entry was not found
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*////////////////////////////////////////////////////////   3) m <pin>   /////////////////////////////////////////////////////////*/
/* Function for "m <pin>" command */
int m_function(LinearHashTable* hashtable, int pin, InvertedLinkedList* invertedlist)  {
    int key = pin % hashtable->h_curr;      //h_i(k) = k % 2^i*m   
    if (key < hashtable->pointer) {         //if smaller than the current position of p
        key = pin % hashtable->h_next;      //h_i+1(k) = k % 2^(i+1)*m
    }

    // Search the bucket to check if entry exists
    for (int j = 0; j < hashtable->buckets[key]->bucket_size; j++) {
        if (hashtable->buckets[key]->bucketentries[j] == pin) {
            if (hashtable->buckets[key]->pvoter[j]->status == 1) {  //1 means "Y"
                printf("|%d| Already Marked Voted\n", hashtable->buckets[key]->pvoter[j]->PIN);
            } else {
                hashtable->buckets[key]->pvoter[j]->status = 1; //status changed to 1 (1 means "Y", has voted)
                printf("|%d| Marked Voted\n", hashtable->buckets[key]->pvoter[j]->PIN);
                
                // Update the Inverted Linked List too
                Voter* voter_address = return_voter_address(hashtable, pin);    //returns the address(in heap) in which the voter-with-pin is
                list_insert(invertedlist, voter_address, pin, hashtable->buckets[key]->pvoter[j]->zipcode);     //inserts the voter-with-pin(his heap address) into the linked list
            }
            return 0;   //entry was found
        }
    }

    // Search the overflown pages too, to check if entry exists
    if (hashtable->buckets[key]->overflown != NULL) {
        return m_check_overflown(hashtable, hashtable->buckets[key]->overflown, pin, invertedlist);
    }


    return -1;   //entry was not found
}

/* Auxiliary function (recursive) used my m_function */
int m_check_overflown(LinearHashTable* hashtable, Bucket* overflown_page, int pin, InvertedLinkedList* invertedlist) {
    for(int j = 0; j < overflown_page->bucket_size; j++) {
        if (overflown_page->bucketentries[j] == pin) {
            if (overflown_page->pvoter[j]->status == 1) {  //1 means "Y"
                printf("|%d| Already Marked Voted\n", overflown_page->pvoter[j]->PIN);
            } else {
                overflown_page->pvoter[j]->status = 1; //status changed to 1 (1 means "Y", has voted)
                printf("|%d| Marked Voted\n", overflown_page->pvoter[j]->PIN);
                
                // Update the Inverted Linked List too
                Voter* voter_address = return_voter_address(hashtable, pin);    //returns the address(in heap) in which the voter-with-pin is
                list_insert(invertedlist, voter_address, pin, overflown_page->pvoter[j]->zipcode);     //inserts the voter-with-pin(his heap address) into the linked list
            }
            return 0;   //entry was found
        }
    }

    if (overflown_page->overflown != NULL) {
        return m_check_overflown(hashtable, overflown_page->overflown, pin, invertedlist);
    }

    return -1;   //entry was not found
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*///////////////////////////////////////////////   5) v  &  6) perc   ////////////////////////////////////////////////////////////*/
/* Function for "v" and "perc" commands */
void v_and_perc_function(LinearHashTable* hashtable, int* have_voted, int* sum_of_all) {
    // Search the whole Hash Table, and find the sum of all voters and the sum of voters who have voted("Y")
    for (int i = 0; i < hashtable->number_of_buckets; i++) {
        for(int j = 0; j < hashtable->buckets[i]->bucket_size; j++) {
            if (hashtable->buckets[i]->bucketentries[j] != -1) {
                (*sum_of_all)++;
                if (hashtable->buckets[i]->pvoter[j]->status == 1) {
                    (*have_voted)++; 
                }
            }
        }

        if (hashtable->buckets[i]->overflown != NULL) {
            v_and_perc_overflown(hashtable->buckets[i]->overflown, have_voted, sum_of_all);
        }
    }
}

/* Auxiliary function (recursive) used by v_and_perc_function */
void v_and_perc_overflown(struct Bucket* overflown_page, int* have_voted, int* sum_of_all) {
        for(int j = 0; j < overflown_page->bucket_size; j++) {
            if (overflown_page->bucketentries[j] != -1) {
                (*sum_of_all)++;
                if (overflown_page->pvoter[j]->status == 1) {
                    (*have_voted)++; 
                }
            }
        }

        if (overflown_page->overflown != NULL) {
            v_and_perc_overflown(overflown_page->overflown, have_voted, sum_of_all);
        }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*//////////////////////////////////////////////////////   7) z <zipcode>   ///////////////////////////////////////////////////////*/
/* Function (recursive) for "z <zipcode>" command */
int z_function(Zipcode* zipcode_node, int zipcode) {
    if (zipcode_node != NULL) {
        //Check the current zipcode node
        if (zipcode_node->zipcode == zipcode) {
            if (zipcode_node->entry != NULL) {
                printf("%d\n", zipcode_node->entry->key);
                z_next_entry(zipcode_node->entry->next_entry);
            }
            return zipcode_node->number_of_voters;

        //Check the next zipcode node
        } else {
            return z_function(zipcode_node->next, zipcode);
        }
    } else {
        printf("Zipcode given has no participants who have voted, or doesn't exist\n");
        return -1;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Auxiliary function (recursive) used by z_function */
void z_next_entry(Entry* entry) {
    if (entry != NULL) {
        printf("%d\n", entry->key);
        z_next_entry(entry->next_entry);
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*///////////////////////////////////////////////////////////   8) o   ////////////////////////////////////////////////////////////*/
/* Function for "o" command */
int o_function(InvertedLinkedList* invertedlist) {
    if (invertedlist->is_empty == 1) {
        return -1;
    } else {
        //Assigning to num_of_rows the number of zipcodes inside the Inverted Linked List
        int num_of_rows = return_num_of_zipcodes_voted(invertedlist);

        //Creating a 2D array with number of rows as many as the zipcodes inside the Inverted Linked List, and 2 columns (1st: zipcode, 2nd: number of voters)
        int** Zipcodes_descending = malloc(num_of_rows * sizeof(int*));
        for (int i = 0; i < num_of_rows; i++) {
            Zipcodes_descending[i] = malloc(2 * sizeof(int)); //2 columns(fixed)
        }

        //Filling the first line
        Zipcodes_descending[0][0] = invertedlist->head->zipcode;
        Zipcodes_descending[0][1] = invertedlist->head->number_of_voters; 

        //Filling the rest lines
        Zipcode* next_node;
        if (invertedlist->head->next != NULL) { //loop is scheduled not to start if NULL
            next_node = invertedlist->head->next;
        }
        for (int i = 1; i < num_of_rows; i++) {
            Zipcodes_descending[i][0] = next_node->zipcode;
            Zipcodes_descending[i][1] = next_node->number_of_voters;
            
            if (next_node->next != NULL) {  //loop is scheduled to end here if NULL
                next_node = next_node->next;
            }
        }

        //Sorting of 2D array using Bubble Sort algorithm (sorted in descending order by taking into account the 2nd column (num_of_voters))
        int i, j, temp1, temp2;
        for (i = 1 ; i <= (num_of_rows-1) ; i++) {
            for(j = (num_of_rows-1) ; j >= i ; j--) {
                if (Zipcodes_descending[j-1][1] < Zipcodes_descending[j][1]) {
                    temp1 = Zipcodes_descending[j-1][0];
                    temp2 = Zipcodes_descending[j-1][1];
                    Zipcodes_descending[j-1][0] = Zipcodes_descending[j][0];
                    Zipcodes_descending[j-1][1] = Zipcodes_descending[j][1];
                    Zipcodes_descending[j][0] = temp1;
                    Zipcodes_descending[j][1] = temp2;
                }
            }
        }

        //Printing the, sorted, 2D array ("<zipcode> <num>" for every zipcode)
        for (int i = 0; i < num_of_rows; i++) {
            printf("%d %d\n", Zipcodes_descending[i][0], Zipcodes_descending[i][1]);
        }

        //Freeing the 2D array created in this function
        for (int i = 0; i < num_of_rows; i++) {
            free(Zipcodes_descending[i]);
        }
        free(Zipcodes_descending);

        return 0;
    }
}

/* Auxiliary function used by o_function */
int return_num_of_zipcodes_voted(InvertedLinkedList* invertedlist) {
    //Returning the number of zipcodes inside the Inverted Linked List (or -1 if list is empty)
    if (invertedlist->is_empty == 1) {
        return -1;
    } else {
        int counter = 1;
        counter += count_next_node(invertedlist->head->next);
        return counter;
    }
}

/* Auxiliary function (recursive) used by return_num_of_zipcodes_voted (& o_function) */
int count_next_node(Zipcode* zipcode_node) {
    if (zipcode_node != NULL) {
        int counter = 1;
        counter += count_next_node(zipcode_node->next);
        return counter;
    } else {
        return 0;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////   9) exit   ///////////////////////////////////////////////////////////*/
/* Function for "exit" command */
void exit_function(LinearHashTable* hashtable, InvertedLinkedList* invertedlist) {
    //Freeing all the mallocs
    
    //list free
    list_free(invertedlist->head);
    free(invertedlist);

    //linear hash table free
    lht_free(hashtable);
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*////////////////////////////////////////////////////////   10) print   //////////////////////////////////////////////////////////*/
/* Function for "print" command */
void print_function(LinearHashTable* hashtable, InvertedLinkedList* invertedlist) {
    // Printing the Linear Hash Table
    lht_printtable(hashtable);  

    // Printing the Inverted Linked List
    printf("\n>Printing Inverted Linked List:\n");
    if (invertedlist->is_empty == 1) {
        printf("List is empty\n");
    }
    list_print(invertedlist->head);
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/