#define SizeofName 20
#define SizeofZipcode 6

//Struct Record
typedef struct {
    int ID;                         //ID
    char lname[SizeofName];         //last name
    char fname[SizeofName];         //first name
    char zipcode[SizeofZipcode];    //zipcode
} Record;

//USR1 & USR2 handler prototypes
void USR1_handler();
extern int USR1_counter;    //global declaration of USR1_counter
void USR2_handler();
extern int USR2_counter;    //global declaration of USR2_counter

//MergeSort prototype
Record* MergeSort(Record**, int*, int);

//free_function prototype
void free_function(int**, int**, int, int**, int***, Record**, Record***, long double***);