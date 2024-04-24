#include <stdio.h>
#include <string.h>     //for strcmp(), strcpy()
#include <fcntl.h>      //for open(), flags of open() and flags of shm_open()
#include <semaphore.h>  //for sem_t, sem_open(), sem_wait(), sem_post(), sem_close(), ...
#include <stdlib.h>     //for exit(), atoi(), srand(), rand(), malloc()
#include <sys/mman.h>   //for shm_open(), mmap(), munmap(), ...
#include <unistd.h>     //for sysconf(), close(), getpid(), pread(), sleep(), ...
#include <sys/times.h>  //for times() (and tms tb1 & tb2)

#include "structs.h"    //for Shared Memory Segment and Account structs

// Auxiliary function for a certain error handling (when giving recid > num_of_accounts in file)
int extractInteger(char*);

/* Reader of records inside file of accounts */
int main(int argc, char* argv[]) {

    if (argc != 9 && argc != 10) {
        printf("Usage: ./reader -f filename -l recid1[ recid2] -d time -s shmname\n"); 
        exit(1);
    }

    // Starting the counter of real time passing
    double t1, t2;
    struct tms tb1, tb2;
    double ticspersec;
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    // Open file "filename" given, and keep the file descriptor
    int DataFilefd;         //int, to keep the file descriptor
    char DataFileName[25];  //fixed-length string to keep the name of "filename"
    int i = 1;
    bool done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-f") == 0) {               //flag for file "filename"
            DataFilefd = open(argv[i+1], O_RDONLY);     //open the file in read-only mode
            if (DataFilefd == -1) {
                perror("Reader failed to open \"filename\""); 
                exit(1);
            }
            strcpy(DataFileName, argv[i+1]);            //name of "filename"
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //if no argument is the flag we are looking for
            printf("Not correct flag for \"filename\" found, need to give -f before it, exiting\n");
            exit(1);
        }
    }
    int num_of_accounts = extractInteger(DataFileName);

    // Keep "recid1[ recid2]" given, which is either one recid or a set of recids that will be read
    int recid2 = 0;
    int recid1 = recid2; //just to avoid warning when not given recid2 (and not used)
    i = 1;
    done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-l") == 0) {       //flag for recid(s) ("recid1[ recid2]")
            recid1 = atoi(argv[i+1]);         
            if (argc == 10) {       //if number of arguments is 10, we have recid2 too
                recid2 = atoi(argv[i+2]);
                if (recid2 < 1 || recid2 > num_of_accounts) {
                    printf("Malformed \"recid2\" value given, it must be >= 1 and <= %d\n", num_of_accounts);
                    exit(1);
                }
                if(recid2 < recid1) {
                    printf("Malformed \"recid2\" value given, it must be >= recid1\n");
                    exit(1);
                }
            }
            if (recid1 < 1 || recid1 > num_of_accounts) {
                printf("Malformed \"recid1\" value given, it must be >= 1 and <= %d\n", num_of_accounts);
                exit(1);
            }
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"recid1[ recid2]\" found, need to be give -l before it, exiting\n");
            exit(1);
        }
    }

    // Keep "time" given, which is the maximum working time (sleeping) of the reader
    int time_given;
    i = 1;
    done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-d") == 0) {       //flag for "time"
            time_given = atoi(argv[i+1]);
            if (time_given < 1) {
                printf("Malformed \"time\" value given, it must be >= 1\n");
                exit(1);
            }
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"time\" found, need to be give -d before it, exiting\n");
            exit(1);
        }
    }
    // Seed the random number generator using current time
    srand(times(NULL));
    // Generate a random number between 1 and "time" given
    int random_sleep_time = (rand() % time_given) + 1;

    // Keep "shmname" given, which is the name of the POSIX Shared Memory Segment that will be opened and accessed
    char shmname[25];  //fixed-length string to keep the name of the shared memory segment
    i = 1;
    done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-s") == 0) {  //flag for "shmname"
            strcpy(shmname, argv[i+1]);
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"shmname\" (shared memory segment) found, need to give -s before it, exiting\n");
            close(DataFilefd);
            exit(1);
        }
    }

    // Open the existing Shared Memory Segment of name given
    int SharedMemorySegmentfd;      //int, to keep the file descriptor
    struct shm_seg* shmsegment;     //shared memory segment struct
    SharedMemorySegmentfd = shm_open(shmname, O_RDWR, 0);   //open shared memory segment in read-write mode
    if (SharedMemorySegmentfd == -1) {
        perror("Reader failed on shm_open");
        exit(1);
    }
    // Map it to the logical address space of the Reader
    shmsegment = mmap(NULL, sizeof(*shmsegment), PROT_READ | PROT_WRITE, MAP_SHARED, SharedMemorySegmentfd, 0);
    if (shmsegment == MAP_FAILED) {
        perror("Reader failed on mmap");
        exit(1);
    }
    // 'SharedMemorySegmentfd' is no longer needed by Reader
    if (close(SharedMemorySegmentfd) == -1) {
        perror("Reader failed on close");
    }
    printf("Reader-PID %ld: Shared Memory Segment \"%s\" has been attached at address \"%p\"\n", (long) getpid(), shmname, shmsegment);


    // Open the existing Semaphores "sem_mutex", "sem_wrt", "sem_in"
    sem_t *sem_mutex = sem_open("sem_mutex", O_RDWR);
    if (sem_mutex == SEM_FAILED) {
        perror("Reader failed on sem_open(sem_mutex)");
        exit(1);
    }
    sem_t *sem_wrt = sem_open("sem_wrt", O_RDWR);
    if (sem_wrt == SEM_FAILED) {
        perror("Reader failed on sem_open(sem_wrt)");
        exit(1);
    }
    sem_t *sem_in = sem_open("sem_in", O_RDWR);
    if (sem_in == SEM_FAILED) {
        perror("Reader failed on sem_open(sem_in)");
        exit(1);
    }

/////////////////////////////////////// READER ALGORITHM ////////////////////////////////////////
    if (sem_wait(sem_in) == -1) {                               //P(sem_in)
        perror("Reader failed on sem_wait(sem_in)");
    }
        if (sem_wait(sem_mutex) == -1) {                        //P(sem_mutex)
            perror("Reader failed on sem_wait(sem_mutex)");
        }
        /**************************Critical Section**************************/
            shmsegment->readcount++;                            //readcount++
            i = 0;
            bool flag = false;
            // Add reader (pid) into the array of active readers
            while (i < MaxActiveProcesses && flag == false) {
                if (shmsegment->active_Readers[i] == 0) {
                    shmsegment->active_Readers[i] = getpid();
                    flag = true;
                }
                i++;
                // If array of active readers is full, then a flag tells us 
                // that readers at some point exceeded the limit of max num_of_active_readers
                if (i == MaxActiveProcesses && flag == false) {
                    shmsegment->extra_Readers = true;
                }
            }
            printf("Reader-PID %ld: readcount is %d\n", (long) getpid(), shmsegment->readcount);
            if (shmsegment->readcount == 1) {                   //if (readcount == 1)
                if (sem_wait(sem_wrt) == -1) {                  //P(sem_wrt)
                    perror("Reader failed on sem_wait(sem_wrt)");
                }
            }
        /********************************************************************/
        if (sem_post(sem_mutex) == -1) {                        //V(sem_mutex)
            perror("Reader failed on sem_post(sem_mutex)");
        }
    if (sem_post(sem_in) == -1) {                               //V(sem_in)
        perror("Reader failed on sem_post(sem_in)");
    }

//////Start of reading here (concurrently among readers, and no writers)
    printf("\nReader-PID %ld: reading from file\n", (long) getpid());

    // Statistic 5) Counting the time passed for the Reader to start reading in file
    t2 = (double) times(&tb2);
    long double real_waiting_time = (t2 - t1)/ticspersec;

    int num_of_records;
    if (recid2 > 0) {
        num_of_records = recid2 - recid1 + 1;
    } else {
        num_of_records = 1;
    }
    // Create a local array (in heap) of one or more records 
    // to retrieve and keep the desired record(s) from file
    struct Account* records = malloc(num_of_records*sizeof(struct Account));
    int bytes_read = pread(DataFilefd, records, num_of_records*sizeof(struct Account), (recid1-1)*sizeof(struct Account));
    if (bytes_read == -1) {
        perror("Reader failed to read from DataFile");
        close(DataFilefd);
        exit(1);
    }
    printf("Reader-PID %ld: reading the below record(s):\n", (long) getpid());
    int sum_balances = 0;
    for (i = 0; i < num_of_records; i++) {
        sum_balances += records[i].balance;
        printf("Reader-PID %ld: %d %s %s %d \n", (long) getpid(), records[i].recID, records[i].lname, records[i].fname, records[i].balance);
    }
    fflush(stdout);
    // Simulating the "working time"
    sleep(random_sleep_time);
    // Calculate the average balance of record(s) read
    double avg_balance = (double) sum_balances / (double) num_of_records;
    printf("Reader-PID %ld: average balance of the record(s) read: %f\n", (long) getpid(), avg_balance);

    printf("Reader-PID %ld: finished reading from file\n\n", (long) getpid());
//////End of reading here

    if (sem_wait(sem_mutex) == -1) {                            //P(sem_mutex)
        perror("Reader failed on sem_wait(sem_mutex)");
    }
    /****************************Critical Section****************************/
        shmsegment->readcount--;                                //readcount--
        i = 0;
        flag = false;
        // Remove reader (pid) from the array of active readers
        while (i < MaxActiveProcesses && flag == false) {
            if (shmsegment->active_Readers[i] == getpid()) {
                shmsegment->active_Readers[i] = 0;
                flag = true;
            }
            i++;
        }
        // Statistic 1) Number of Readers who worked on the file increases by 1
        shmsegment->Readers_finished++;

        // Statistic 2) Counting the time passed for the Reader to finish its operations
        t2 = (double) times(&tb2);
        long double real_whole_time = (t2 - t1)/ticspersec;
        shmsegment->Readers_times += real_whole_time;

        // Statistic 5) (cont) Comparing Reader's waiting_time with current max_waiting_time
        if (real_waiting_time > shmsegment->max_waiting_time) {
            shmsegment->max_waiting_time = real_waiting_time;
        }

        // Statistic 6) Number of Records that were accessed increases by num_of_records read
        shmsegment->Records_processed += num_of_records;

        printf("Reader-PID %ld: readcount is %d\n", (long) getpid(), shmsegment->readcount);
        if (shmsegment->readcount == 0) {                       //if (readcount == 0)
            if (sem_post(sem_wrt) == -1) {                      //V(sem_wrt)
                perror("Reader failed on sem_post(sem_wrt)");
            }
        }
    /************************************************************************/
    if (sem_post(sem_mutex) == -1) {                            //V(sem_mutex)
        perror("Reader failed on sem_post(sem_mutex)");
    }
////////////////////////////////////////////////////////////////////////////////////////////////////

    // Close the Semaphores "sem_mutex", "sem_wrt", "sem_in"
    if (sem_close(sem_mutex) == -1) {
        perror("Reader failed on sem_close(sem_mutex)");
        exit(1);
    }
    if (sem_close(sem_wrt) == -1) {
        perror("Reader failed on sem_close(sem_wrt)");
        exit(1);
    }
    if (sem_close(sem_in) < 0) {
        perror("Reader failed on sem_close(sem_in)");
        exit(1);
    }

    // Unmap the Shared Memory Segment from the logical address space of the reader
    if (munmap(shmsegment, sizeof(*shmsegment)) == -1) {
        perror("Reader failed on munmap");
        exit(1);
    }

    // Close file "filename" given
    if (close(DataFilefd) == -1) {
        perror("Reader failed to close DataFile"); 
        exit(1);
    }

    return 0;   //successful end of reader
}


// Function for extracting the integer from the DataFile name (e.g. number 50 from the name accounts50.bin)
int extractInteger(char* name) {
    int integer;
    sscanf(name, "accounts%d", &integer);
    return integer;
}