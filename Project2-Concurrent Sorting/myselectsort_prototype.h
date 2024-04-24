#define SizeofName 20
#define SizeofZipcode 6

//Struct Record
typedef struct {
    int ID;                         //ID
    char lname[SizeofName];         //last name
    char fname[SizeofName];         //first name
    char zipcode[SizeofZipcode];    //zipcode
} Record;

//SelectSort prototype
void SelectSort(Record*, int);