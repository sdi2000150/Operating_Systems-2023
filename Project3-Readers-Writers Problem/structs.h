#include <stdbool.h>

#define MaxActiveProcesses 1000
/* Struct that will be the Shared Memory Segment */
struct shm_seg {
    int readcount;          // Number of Active Readers reading in file concurrently
    int active_Readers[MaxActiveProcesses];    // Array of Active Readers (PIDs)
    bool extra_Readers;                 // is true if number of Active Readers > MaxActiveProcesses
    int active_Writers[MaxActiveProcesses];    // Array of Active Writers (PIDs)
    bool extra_Writers;                 // is true if number of Active Writers > MaxActiveProcesses
    
    int Readers_finished;           // 1) Number of Readers who worked on the file
    long double Readers_times;      // 2) Sum of times in which Readers worked (can be used with Readers_finished to find the Average_time)
    int Writers_finished;           // 3) Number of Writers who worked on the file
    long double Writers_times;      // 4) Sum of times in which Writers worked (can be used with Writers_finished to find the Average_time)
    long double max_waiting_time;   // 5) Max waiting latency before starting working (of Reader or Writer)
    int Records_processed;          // 6) Number of Records that were accessed (for Reading or Writing)
};

#define SizeofName 20
/* Struct Account */
struct Account {
    int recID;                      //Customer ID
    char lname[SizeofName];         //Last Name
    char fname[SizeofName];         //First Name
    int balance;                    //Account Balance
};