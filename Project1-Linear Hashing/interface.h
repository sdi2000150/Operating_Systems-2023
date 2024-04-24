#include "structs.h"


/***************** Functions for the Linear Hash Table *****************/

/* Function for creating the LHT */
LinearHashTable* lht_createtable(int, int);


/* Functions for the insertions of entries into the LHT */
int lht_insertvoter(LinearHashTable*, int, char*, char*, int);

struct Bucket* insert_overflown(struct Bucket*, int, int, char*, char*, int);


/* Functions for the splitting of buckets of the LHT */
void lht_check_for_split(LinearHashTable*);

void split_overflown (struct Bucket*, struct Bucket*, int, int);


/* Functions for printing the whole LHT */
void lht_printtable(LinearHashTable*);

void print_overflown(struct Bucket*);


/* Functions for freeing the whole LHT */
void lht_free(LinearHashTable*);

void free_overflown(struct Bucket*);



/****************** Functions for the Inverted Linked List ******************/

InvertedLinkedList* list_create(void);

void list_insert(InvertedLinkedList*, Voter*, int, int);

Zipcode* check_next_node(Zipcode*, Voter*, int, int);

Entry* insert_next_entry(Entry*, Voter*, int);

void list_print(Zipcode*);

void print_next_entry(Entry*);

void list_free(Zipcode*);

void list_free_entry(Entry*);



/****************** Functions for auxiliary procedures ******************/

Voter* return_voter_address(LinearHashTable*, int);

Voter* return_voter_address_overflown(Bucket*, int);

void print_voter(Voter*);

int power(int, int);