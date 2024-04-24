#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interface.h"

int previousones;   //extern variable for keeping the previous number of buckets(starting from the initial one), of the previous round 


/*********************************************** Functions for the Linear Hash Table ***********************************************/

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function which creates a Linear Hash Table */
LinearHashTable* lht_createtable(int number_of_buckets, int bucketentries) {
    /* Create and Initialize Hash Table */
    LinearHashTable* table = malloc(sizeof(LinearHashTable));   //dynamically allocate the LinearHashTable struct
    table->pointer = 0;                             //next bucket to be split is no 0
    table->round = 0;                               //hash function for round 0: h1(k) = k % 2 and h2(k) = k % 4
    table->load_factor = 0;                         //load factor is 0 in the start (0/key_capacity_non_overflown)
    table->number_of_keys = 0;                      //number of keys(voters) in hash table initialized to 0
    table->number_of_buckets = number_of_buckets;   //initialized size of HashTable (in terms of buckets it has)
    table->m =  number_of_buckets;                  //initial number of buckets(fixed)
    table->h_curr = power(2,table->round) * table->m;       //h_i (corresponding hash function of the current round)
    table->h_next = power(2,table->round + 1) * table->m;   //h_i+1 (hash function of the next round)
    previousones = table->number_of_buckets;                //previous number of buckets initialized to the initial one
    table->buckets = malloc(sizeof(Bucket*)*number_of_buckets); //dynamically allocate the array of pointers to buckets

    /* Create and Initialize each Bucket of the Hash Table */
    for (int i = 0; i < table->number_of_buckets; i++) {
        table->buckets[i] = malloc(sizeof(Bucket));             //dynamically allocate each bucket
        table->buckets[i]->bucket_size = bucketentries;         //number of keys of the bucket(bucketentries)
        table->buckets[i]->bucketentries = malloc(sizeof(int*) * (table->buckets[i]->bucket_size)); //dynamically allocate array of keys
        table->buckets[i]->pvoter = malloc(sizeof(Voter*) * (table->buckets[i]->bucket_size));      //dynamically allocate array of voters
        table->buckets[i]->overflown = NULL;

        // Initialize each key of the Bucket to -1 (meaning it's empty)
        // And every pointer to voter to NULL (meaning it's empty)
        for(int j = 0; j < table->buckets[i]->bucket_size; j++) {
            table->buckets[i]->bucketentries[j] = -1;
            table->buckets[i]->pvoter[j] = NULL;
        }
    }

    return table;
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function which takes an entry, and inserts it into the Linear Hash Table */
int lht_insertvoter(LinearHashTable* hashtable, int pin, char* fname, char* lname, int zipcode) {
    int key = pin % hashtable->h_curr;      //h_i(k) = k % 2^i*m   
    if (key < hashtable->pointer) {         //if smaller than the current position of p
        key = pin % hashtable->h_next;      //h_i+1(k) = k % 2^(i+1)*m
    }

    int full = 0;       //counter of full entries of bucket
    int j = 0;
    bool inserted = 0;  //entry has not been inserted
    // 1) Search the bucket in order for the entry to be inserted
    while ((j < hashtable->buckets[key]->bucket_size) && (inserted == 0)) {
        if (hashtable->buckets[key]->bucketentries[j] == pin) {         //check for duplicates
            return -1;
        }
        if ((hashtable->buckets[key]->bucketentries[j]) == -1) {        //bucket entry is empty
            //key can enter
            hashtable->buckets[key]->bucketentries[j] = pin;

            //voter enters too
            hashtable->buckets[key]->pvoter[j] = malloc(sizeof(Voter)); 
            hashtable->buckets[key]->pvoter[j]->PIN = pin;

            hashtable->buckets[key]->pvoter[j]->lname = malloc(strlen(lname) + 1); 
            strcpy(hashtable->buckets[key]->pvoter[j]->lname, lname);
            hashtable->buckets[key]->pvoter[j]->fname = malloc(strlen(fname) + 1);  
            strcpy(hashtable->buckets[key]->pvoter[j]->fname, fname);
            
            hashtable->buckets[key]->pvoter[j]->zipcode = zipcode;
            hashtable->buckets[key]->pvoter[j]->status = 0; //0 means "N"
            
            inserted = 1;   //entry has been inserted
            hashtable->number_of_keys++;    //number of keys in hash table increases by 1
        } else {
            full++;     //bucket entry is full, counter of full entries of bucket increases by 1 
        }
        j++;
    }

    // 2) If key wasn't entered (all bucket entries were full)
    if ((inserted == 0) && (full == hashtable->buckets[key]->bucket_size)) { 
        // Search the overflown page of the bucket in order for the entry to be inserted
        hashtable->buckets[key]->overflown = insert_overflown(hashtable->buckets[key]->overflown, hashtable->buckets[key]->bucket_size, pin, fname, lname, zipcode);
        if (hashtable->buckets[key]->overflown == NULL) {   //check for duplicates
            return -1;
        } else {    //entry inserted, so number of keys in hash table increases by 1
            hashtable->number_of_keys++;    
        }
    }         
        
    // 3) Check whether a bucket has to be split
    lht_check_for_split(hashtable);

    return 0;
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function which takes an entry, and inserts it into the overflown page that it has to go */
struct Bucket* insert_overflown(struct Bucket* overflown_page, int bucket_size, int pin, char* fname, char* lname, int zipcode) {

    // 1) If overflown page exists
    if (overflown_page != NULL) {
        int full = 0;  //counter of full entries of overflown page
        // Check if it is full
        for(int k = 0; k < overflown_page->bucket_size; k++) {  
            if (overflown_page->bucketentries[k] != -1) {
                full++;     //overflown page entry is full
            }
        }

        // 1.1) If overflown page has space
        if (full < overflown_page->bucket_size) { 

            // Insert entry in this overflown page
            int k = 0;
            bool inserted = 0;  //entry has not been inserted
            // Search the overflown page in order for the entry to be inserted
            while((k < bucket_size) && (inserted == 0)) {
                if (overflown_page->bucketentries[k] == pin) {  //check for duplicates
                    return NULL;
                }
                if (overflown_page->bucketentries[k] == -1) {  //overflown page entry is empty
                    //key can enter
                    overflown_page->bucketentries[k] = pin;

                    //voter enters too
                    overflown_page->pvoter[k] = malloc(sizeof(Voter));     
                    overflown_page->pvoter[k]->PIN = pin;

                    overflown_page->pvoter[k]->lname = malloc(strlen(lname) + 1);  
                    strcpy(overflown_page->pvoter[k]->lname, lname);
                    overflown_page->pvoter[k]->fname = malloc(strlen(fname) + 1);  
                    strcpy(overflown_page->pvoter[k]->fname, fname);

                    overflown_page->pvoter[k]->zipcode = zipcode;
                    overflown_page->pvoter[k]->status = 0;  //0 means "N"

                    inserted = 1;   //entry has been inserted
                }
                k++;
            }  
            return overflown_page;

        // 1.2) If overflown page has NO space 
        } else {
            // Search the overflown page of the overflown page (recursively) in order for the entry to be inserted  
            overflown_page->overflown = insert_overflown(overflown_page->overflown, overflown_page->bucket_size, pin, fname, lname, zipcode);
            return overflown_page;
        }

    // 2) If there is NO overflown page
    } else {

        // Create a new overflown page
        overflown_page = malloc(sizeof(Bucket));                     
        overflown_page->bucket_size = bucket_size;
        overflown_page->bucketentries = malloc(sizeof(int*) * (bucket_size)); 
        overflown_page->pvoter = malloc(sizeof(Voter*) * (bucket_size)); 
        overflown_page->overflown = NULL;

        // Initialize each key of the overflown page to -1 (meaning it's empty)
        // And every pointer to voter to NULL (meaning it's empty)
        for(int j = 0; j < bucket_size; j++) {
            overflown_page->bucketentries[j] = -1;
            overflown_page->pvoter[j] = NULL;
        }

        // Insert entry in this overflown page
        int k = 0;
        bool inserted = 0;  //entry has not been inserted
        // Search the overflown page in order for the entry to be inserted
        while((k < bucket_size) && (inserted == 0)) {
            if (overflown_page->bucketentries[k] == pin) { //check for duplicates
                return NULL;
            }
            if (overflown_page->bucketentries[k] == -1) {  //overflown page entry is empty
                //key can enter
                overflown_page->bucketentries[k] = pin;

                //voter enters too
                overflown_page->pvoter[k] = malloc(sizeof(Voter));     
                overflown_page->pvoter[k]->PIN = pin;

                overflown_page->pvoter[k]->lname = malloc(strlen(lname) + 1);  
                strcpy(overflown_page->pvoter[k]->lname, lname);
                overflown_page->pvoter[k]->fname = malloc(strlen(fname) + 1); 
                strcpy(overflown_page->pvoter[k]->fname, fname);

                overflown_page->pvoter[k]->zipcode = zipcode;
                overflown_page->pvoter[k]->status = 0;  //0 means "N"

                inserted = 1;   //entry has been inserted
            }
            k++;
        }  
        return overflown_page;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function to check whether a bucket has to be split, and execute the split if needed */
void lht_check_for_split(LinearHashTable* hashtable) {
    // Compute the current load factor
    double number_of_keys_in_ht = (double) (hashtable->number_of_keys);
    double key_capacity_in_non_overflown_buckets = (double) ((hashtable->number_of_buckets) * (hashtable->buckets[0]->bucket_size)); //we use the bukcet_size of first bucket, but it is the same for all
    hashtable->load_factor = number_of_keys_in_ht / key_capacity_in_non_overflown_buckets;

    // if load factor comes closer to 1, a bucket has to be split
    if (hashtable->load_factor > 0.75) { 
        
        // A new bucket will be created
        hashtable->number_of_buckets++; 
        hashtable->buckets = realloc(hashtable->buckets, sizeof(Bucket*) * (hashtable->number_of_buckets));

        // Create the new bucket and initialize it
        int new_bucket = hashtable->number_of_buckets - 1;
        hashtable->buckets[new_bucket] = malloc(sizeof(Bucket));
        hashtable->buckets[new_bucket]->bucket_size = hashtable->buckets[0]->bucket_size;   //we use the bucket_size e.g. of first bucket, as it is the same for all
        hashtable->buckets[new_bucket]->bucketentries = malloc(sizeof(int*) * (hashtable->buckets[new_bucket]->bucket_size));
        hashtable->buckets[new_bucket]->pvoter = malloc(sizeof(Voter*) * (hashtable->buckets[new_bucket]->bucket_size));
        hashtable->buckets[new_bucket]->overflown = NULL;

        for(int j = 0; j < hashtable->buckets[new_bucket]->bucket_size; j++) {
            hashtable->buckets[new_bucket]->bucketentries[j] = -1;
            hashtable->buckets[new_bucket]->pvoter[j] = NULL;
        }

        // Search the bucket that will be split, in order to rehash the elements
        for(int j = 0; j < hashtable->buckets[hashtable->pointer]->bucket_size; j++) {
            // Check for each entry, where PIN % (2^(i+1)*m) hashes 
            int pin;
            int key;
            if (hashtable->buckets[hashtable->pointer]->bucketentries[j] != -1) {
                pin = hashtable->buckets[hashtable->pointer]->pvoter[j]->PIN;
                key = pin % hashtable->h_next;

                // If entry doesn't hash to current bucket anymore, needs to be moved to the new bucket created
                if (key != hashtable->pointer) {
                    int k = 0;
                    bool inserted = 0;  //entry has not been moved
                    while ((k < hashtable->buckets[new_bucket]->bucket_size) && (inserted == 0)) {
                        if ((hashtable->buckets[new_bucket]->bucketentries[k]) == -1) { //bucket entry is empty
                            //key can be moved
                            hashtable->buckets[new_bucket]->bucketentries[k] = hashtable->buckets[hashtable->pointer]->bucketentries[j];
                            hashtable->buckets[hashtable->pointer]->bucketentries[j] = -1;  //entry in previous bucket becomes empty (-1)

                            //voter moves too
                            hashtable->buckets[new_bucket]->pvoter[k] = hashtable->buckets[hashtable->pointer]->pvoter[j];
                            hashtable->buckets[hashtable->pointer]->pvoter[j] = NULL;       //pointer to voter in previous bucket becomes empty (NULL)

                            inserted = 1;   //entry has been moved
                        }
                        k++;
                    }
                }
            }

            // Search the overflown page too of the bucket that will be split, in order to rehash all the elements
            if (hashtable->buckets[hashtable->pointer]->overflown != NULL) {
                split_overflown(hashtable->buckets[hashtable->pointer]->overflown, hashtable->buckets[new_bucket], hashtable->h_next, hashtable->pointer);

            }
        }

        // After each split
        // If the hash table has double in terms of number of buckets
        if (hashtable->number_of_buckets == 2*previousones) {
            previousones = hashtable->number_of_buckets;        //previous number of buckets becomes the current one

            // Pointer and Round change
            hashtable->pointer = 0;                     //pointer moves to the first bucket
            hashtable->round++;                         //and we proceed to the next Round

            // Utilized family of functions changes:
            hashtable->h_curr = hashtable->h_next;                              //h_i = 2^i*m
            hashtable->h_next = power(2,hashtable->round + 1) * hashtable->m;   //h_i+1 = 2^(i+1)*m

        // Else, pointer moves one bucket over
        } else {
            hashtable->pointer++;  
        }

        // Because a split occured, compute the updated load factor
        number_of_keys_in_ht = (double) (hashtable->number_of_keys);
        key_capacity_in_non_overflown_buckets = (double) ((hashtable->number_of_buckets) * (hashtable->buckets[0]->bucket_size)); //we use the bukcet_size of first bucket, but it is the same for all
        hashtable->load_factor = number_of_keys_in_ht / key_capacity_in_non_overflown_buckets;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function to check overflown buckets too, of the bucket that is being split */
void split_overflown (struct Bucket* overflown_page, struct Bucket* new_bucket, int lht_h_next, int lht_pointer) {
    // Search the overflown page of the bucket that will be split, in order to rehash the elements
    for(int j = 0; j < overflown_page->bucket_size; j++) {
        // Check for each entry, where PIN % (2^(i+1)*m) hashes 
        int pin;
        int key;
        if (overflown_page->bucketentries[j] != -1) {
            pin = overflown_page->pvoter[j]->PIN;
            key = pin % lht_h_next;

            // If entry doesn't hash to current bucket anymore, needs to be moved to the new bucket created
            if (key != lht_pointer) {
                int k = 0;
                bool inserted = 0;   //entry has not been moved
                while ((k < new_bucket->bucket_size) && (inserted == 0)) {
                    if ((new_bucket->bucketentries[k]) == -1) { //bucket entry is empty
                        //key can be moved
                        new_bucket->bucketentries[k] = overflown_page->bucketentries[j];
                        overflown_page->bucketentries[j] = -1;  //entry in previous bucket becomes empty (-1)

                        //voter moves too
                        new_bucket->pvoter[k] = overflown_page->pvoter[j];
                        overflown_page->pvoter[j] = NULL;       //pointer to voter in previous bucket becomes empty (NULL)

                        inserted = 1;   //entry has been moved
                    }
                    k++;
                }
            }
        }

        // Search the overflown page too of the overflown page of the bucket that will be split (recursively), in order to rehash all the elements
        if (overflown_page->overflown != NULL) {
            split_overflown(overflown_page->overflown, new_bucket, lht_h_next, lht_pointer);
        }
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function which prints the elements of the Linear Hash Table */
void lht_printtable(LinearHashTable* table) {
    printf("\nPrinting Linear Hash Table:\n\n");

    printf("Number of Buckets: %d\n", table->number_of_buckets);
    printf("Next Bucket to be split: %d\n", table->pointer);
    printf("Current Round: %d\n", table->round);
    printf("Load Factor current value: %f\n", table->load_factor);
    printf("Size(number of keys) of each Bucket/Overflown_page: %d\n", table->buckets[0]->bucket_size); //we use the bukcet_size of first bucket, but it is the same for all

    printf(">Printing each Bucket of the Hash Table:\n");
    for (int i = 0; i < table->number_of_buckets; i++) {
        printf("Bucket no. %2d: ", i);
        for(int j = 0; j < table->buckets[i]->bucket_size; j++) {
            if (table->buckets[i]->bucketentries[j] != -1) { 
                printf("|%6d|", table->buckets[i]->bucketentries[j]);
            } else {
                printf("|      |");
            }
            // //Print voter's elements too
            // if (table->buckets[i]->bucketentries[j] != -1) {
            //     print_voter(table->buckets[i]->pvoter[j]);
            // }
        }

        if (table->buckets[i]->overflown != NULL) {
            printf(" -->");
            print_overflown(table->buckets[i]->overflown);
        }
        printf("\n");
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function for printing the overflown pages of a bucket */
void print_overflown(struct Bucket* overflown_page) {
    for(int j = 0; j < overflown_page->bucket_size; j++) {
        if (overflown_page->bucketentries[j] != -1) {
            printf("|%6d|", overflown_page->bucketentries[j]);
        } else {
            printf("|      |");
        }
        // //Print voter's elements too
        // if (overflown_page->bucketentries[j] != -1) {
        //     print_voter(overflown_page->pvoter[j]);
        // }
    }

    if (overflown_page->overflown != NULL) {
        printf(" -->");
        print_overflown(overflown_page->overflown);
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function for freeing the Linear Hash Table */
void lht_free(LinearHashTable* hashtable) {
    for (int i = 0; i < hashtable->number_of_buckets; i++) {
        if (hashtable->buckets[i]->overflown != NULL) {
            free_overflown(hashtable->buckets[i]->overflown);
        }
        for(int j = 0; j < hashtable->buckets[i]->bucket_size; j++) {         
            if ((hashtable->buckets[i]->bucketentries[j]) != -1) { //There is a key
                free(hashtable->buckets[i]->pvoter[j]->lname);
                free(hashtable->buckets[i]->pvoter[j]->fname);
                
                free(hashtable->buckets[i]->pvoter[j]);
            }
        }
        free(hashtable->buckets[i]->bucketentries);
        free(hashtable->buckets[i]->pvoter);
        free(hashtable->buckets[i]);
    }
    free(hashtable->buckets);
    free(hashtable);
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function for freeing of the overflown pages of a bucket */
void free_overflown(struct Bucket* overflown_page) {
        if (overflown_page->overflown != NULL) {
            free_overflown(overflown_page->overflown);
        }
        for(int j = 0; j < overflown_page->bucket_size; j++) {
            if (overflown_page->bucketentries[j] != -1) {
                free(overflown_page->pvoter[j]->lname);
                free(overflown_page->pvoter[j]->fname);

                free(overflown_page->pvoter[j]);
            }
        }
        free(overflown_page->pvoter);
        free(overflown_page->bucketentries);
        free(overflown_page);
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/********************************************* Functions for the Inverted Linked List **********************************************/

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function which creates an Inverted Linked List */
InvertedLinkedList* list_create(void) {
    InvertedLinkedList* invertedlist = malloc(sizeof(InvertedLinkedList));
    invertedlist->is_empty = 1; //list has no zipcode nodes
    invertedlist->head = NULL;

    return invertedlist;
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function which inserts the voter-with-pin(his heap address) into the Inverted Linked List */
void list_insert(InvertedLinkedList* invertedlist, Voter* voter_address, int pin, int zipcode) {
    // 1) If List is empty (has no zipcode nodes)
    if (invertedlist->is_empty == 1) {
        //Create a new zipcode node
        invertedlist->head = malloc(sizeof(Zipcode));
        invertedlist->head->zipcode = zipcode;

        //Create a new entry node and insert the voter's pin and address in it
        invertedlist->head->entry = malloc(sizeof(Entry));
        invertedlist->head->entry->key = pin;
        invertedlist->head->entry->voter_address = voter_address;
        invertedlist->head->entry->next_entry = NULL;
        invertedlist->head->number_of_voters = 1;

        invertedlist->is_empty = 0; //list isn't empty anymore

        invertedlist->head->next = NULL;
        
    // 2) If List isn't empty (has zipcode node(s))
    } else {
        //Check first zipcode node
        if (invertedlist->head->zipcode == zipcode) {
            //voter-with-pin(his heap address) will be inserted here
            //Check first next entry node (because the current one will deffinitely be full, as zipcode node exists)
            if (invertedlist->head->entry->next_entry == NULL) {
                invertedlist->head->entry->next_entry = malloc(sizeof(Entry));
                invertedlist->head->entry->next_entry->key = pin;
                invertedlist->head->entry->next_entry->voter_address = voter_address;
                invertedlist->head->entry->next_entry->next_entry = NULL;
            
            //Check rest next entry nodes
            } else {    
                invertedlist->head->entry->next_entry->next_entry = insert_next_entry(invertedlist->head->entry->next_entry->next_entry, voter_address, pin);
            }
            invertedlist->head->number_of_voters++;

        //Check next zipcode nodes
        } else {    
            invertedlist->head->next = check_next_node(invertedlist->head->next, voter_address, pin, zipcode);
        }
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function which searches for the zipcode node(or creates one if not existed) for the insertion of the voter-with-pin(his heap address) */
Zipcode* check_next_node(Zipcode* zipcode_node, Voter* voter_address, int pin, int zipcode) {
    // 1) If zipcode node is empty
    if (zipcode_node == NULL) {
        //Create a new zipcode node
        zipcode_node = malloc(sizeof(Zipcode));
        zipcode_node->zipcode = zipcode;

        //Create a new entry node and insert the voter's pin and address in it
        zipcode_node->entry = malloc(sizeof(Entry));
        zipcode_node->entry->key = pin;
        zipcode_node->entry->voter_address = voter_address;
        zipcode_node->entry->next_entry = NULL;
        zipcode_node->number_of_voters = 1;

        zipcode_node->next = NULL;

        return zipcode_node;

    // 2) Else if zipcode node isn't empty and it's equal to the one we want to insert
    } else if (zipcode_node->zipcode == zipcode) {
        //voter-with-pin(his heap address) will be inserted here
        //Check first next entry node (because the current one will deffinitely be full, as zipcode node exists)
        if (zipcode_node->entry->next_entry == NULL) {
            zipcode_node->entry->next_entry = malloc(sizeof(Entry));
            zipcode_node->entry->next_entry->key = pin;
            zipcode_node->entry->next_entry->voter_address = voter_address;
            zipcode_node->entry->next_entry->next_entry = NULL;
        
        //Check rest next entry nodes
        } else {    
            zipcode_node->entry->next_entry->next_entry = insert_next_entry(zipcode_node->entry->next_entry->next_entry, voter_address, pin);
        }
        zipcode_node->number_of_voters++;

        return zipcode_node;

    // 3) Else check next zipcode nodes
    } else {
        zipcode_node->next = check_next_node(zipcode_node->next, voter_address, pin, zipcode);
        return zipcode_node;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function which searches for the entry node to be created, for the insertion of the voter-with-pin(his heap address) */
Entry* insert_next_entry(Entry* entry, Voter* voter_address, int pin) {
    // If entry node is empty
    if (entry == NULL) {
        //Create a new entry node and insert the voter's pin and address in it
        entry = malloc(sizeof(Entry));
        entry->key = pin;
        entry->voter_address = voter_address;
        entry->next_entry = NULL;
        return entry;

    // Else check rest next entry nodes
    } else {
        entry->next_entry = insert_next_entry(entry->next_entry, voter_address, pin);
        return entry;
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function which prints the elements of the Inverted Linked List */
void list_print(Zipcode* zipcode_node) {
    if (zipcode_node != NULL) {
        printf("Zipcode Node no. %d: -->", zipcode_node->zipcode);
        printf ("|%d|", zipcode_node->entry->key);

        // //Print voter's elements too
        // print_voter(zipcode_node->entry->voter_address); 
        if (zipcode_node->entry->next_entry != NULL) {
            print_next_entry(zipcode_node->entry->next_entry);
        }
        printf("\n");
        if (zipcode_node->next != NULL) {
            printf("↓\n");
            list_print(zipcode_node->next);
        }
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function for printing the rest entry nodes of a zipcode node */
void print_next_entry(Entry* entry) {
    printf ("-->|%d|", entry->key);

    // //Print voter's elements too
    // print_voter(entry->voter_address); 
    if (entry->next_entry != NULL) {
        print_next_entry(entry->next_entry);
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function for freeing the Zipcode nodes of the Inverted List List */
void list_free(Zipcode* zipcode_node) {
    if (zipcode_node != NULL) {
        if (zipcode_node->next != NULL) {
            list_free(zipcode_node->next);  //free next zipcode node
        }
        list_free_entry(zipcode_node->entry);   //free the entry nodes of the zipcode
        free(zipcode_node);     //free current zipcode node
    }
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function for freeing the entry nodes of a zipcode */
void list_free_entry(Entry* entry) {
    if (entry->next_entry != NULL) {
        list_free_entry(entry->next_entry); //free next entry node
    }
    free(entry);    //free current entry node
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*********************************************** Functions for auxiliary procedures ************************************************/

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function for returning the address(in heap) in which the voter-with-pin is */
Voter* return_voter_address(LinearHashTable* hashtable, int pin) {
    int key = pin % hashtable->h_curr;      //h_i(k) = k % 2^i*m   
    if (key < hashtable->pointer) {         //if smaller than the current position of p
        key = pin % hashtable->h_next;      //h_i+1(k) = k % 2^(i+1)*m
    }

    // Search the bucket to check if entry exists
    for (int j = 0; j < hashtable->buckets[key]->bucket_size; j++) {
        if (hashtable->buckets[key]->bucketentries[j] == pin) {
            return hashtable->buckets[key]->pvoter[j];   //entry was found, return the address
        }
    }
    if (hashtable->buckets[key]->overflown != NULL) {
        // Search the overflown page
        return return_voter_address_overflown(hashtable->buckets[key]->overflown, pin);
    }
    return NULL;   //entry was not found    
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Recursive function(searching the overflown pages) for returning the address(in heap) in which the voter-with-pin is */
Voter* return_voter_address_overflown(Bucket* overflown_page, int pin) {
    // Search the overflown page to check if entry exists
    for(int j = 0; j < overflown_page->bucket_size; j++) {
        if (overflown_page->bucketentries[j] == pin) {
            return overflown_page->pvoter[j];   //entry was found, return the address
        }
    }
    if (overflown_page->overflown != NULL) {
        // Search the next overflown page
        return return_voter_address_overflown(overflown_page->overflown, pin);
    }
    return NULL; //entry was not found
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Function for printing the elements of a voter */
void print_voter(Voter* voter) {
    printf("->[Voter's PIN:%d, lname:%s, fname:%s, zipcode:%d, vote:%d]", 
            voter->PIN, voter->lname, voter->fname, voter->zipcode, voter->status);
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* Power function to compute base^n */
int power(int base, int n)
{ int p;
  for (p=1 ; n > 0 ; n--)   //base^n equals to
    p *= base;              //base * base * ... * base (n times)
  return p;                 //return result
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


