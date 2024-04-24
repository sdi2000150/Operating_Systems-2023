#include <stdbool.h>


/********* Linear Hash Table Structs **********/

// Struct Voter
typedef struct {
    int PIN;        //PIN
    char* lname;    //last name
    char* fname;    //first name
    int zipcode;    //zipcode
    bool status;    //status (0 if "N", 1 if "Y")
} Voter;

// Struct Bucket
typedef struct Bucket {
    int bucket_size;            //number of keys of a bucket(bucketentries)
    int* bucketentries;         //array of keys of a Bucket
    Voter** pvoter;             //array of voters of a Bucket
    struct Bucket* overflown;   //overflown page(s)
} Bucket;

// Struct Linear Hash Table
typedef struct {
    int number_of_buckets;  //size of HashTable
    Bucket** buckets;       //array of pointers to buckets
    int pointer;            //next bucket to be split
    int round;              //Round
    double load_factor;     //load factor
    
    int number_of_keys;     //number of keys(voters) in hash table
    int m;                  //initial number of buckets
    int h_curr;             //h_i (corresponding hash function of the current round)
    int h_next;             //h_i+1 (hash function of the next round)
} LinearHashTable;


/********** Inverted Linked List Structs **********/

// Struct Entry
typedef struct Entry {
    int key;                    //key of entry (pin)
    Voter* voter_address;       //address to the data of voter with pin == key
    struct Entry* next_entry;   //next entry/ies
} Entry;

//Struct Zipcode
typedef struct Zipcode {
    int zipcode;            //zipcode
    int number_of_voters;   //number of voters(who have voted) in this zipcode
    struct Entry* entry;    //start of the list with entries in this zipcode
    struct Zipcode* next;   //next zipcode(s)
} Zipcode;

//Struct Inverted Linked List
typedef struct {
    struct Zipcode* head;   //pointer to the first node (start) of the inverted linked list
    bool is_empty;          //is_empty (1 if list is empty, 0 if list is not empty)
} InvertedLinkedList;   