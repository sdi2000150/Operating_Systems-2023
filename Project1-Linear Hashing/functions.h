#include "interface.h"

/**************** Functions the program can execute by command line ****************/

// 1) l <pin>
int l_function(LinearHashTable*, int);
int l_check_overflown(LinearHashTable*, Bucket*, int);

// 2) i <pin> <lname> <fname> <zip>
//used lht_insertvoter, so i_function is not needed

// 3) m <pin>
int m_function(LinearHashTable*, int, InvertedLinkedList*);
int m_check_overflown(LinearHashTable*, Bucket*, int, InvertedLinkedList*);

// 4) bv <fileofkeys>
//used m_function, so bv_function is not needed

// 5) v  &  6) perc
void v_and_perc_function(LinearHashTable*, int*, int*);
void v_and_perc_overflown(struct Bucket*, int*, int*);

// 7) z <zipcode>
int z_function(Zipcode*, int);
void z_next_entry(Entry*);

// 8) o
int o_function(InvertedLinkedList*);
int return_num_of_zipcodes_voted(InvertedLinkedList*);
int count_next_node(Zipcode*);

// 9) exit
void exit_function(LinearHashTable*, InvertedLinkedList*);

// 10) print
void print_function(LinearHashTable*, InvertedLinkedList*);