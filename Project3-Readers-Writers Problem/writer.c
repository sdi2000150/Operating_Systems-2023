#include <stdio.h>
#include <string.h>     //for strcmp(), strcpy()
#include <fcntl.h>      //for open(), flags of open() and flags of shm_open()
#include <semaphore.h>  //for sem_t, sem_open(), sem_wait(), sem_post(), sem_close(), ...
#include <stdlib.h>     //for exit(), atoi(), srand(), rand()
#include <sys/mman.h>   //for shm_open(), mmap(), munmap(), ...
#include <unistd.h>     //for sysconf(), close(), getpid(), lseek(), read(), write(), sleep(), ...
#include <sys/times.h>  //for times() (and tms tb1 & tb2)

#include "structs.h"    //for Shared Memory Segment and Account structs

// Auxiliary function for a certain error handling (when giving recid > num_of_accounts in file)
int extractInteger(char*);

/* Writer of records inside file of accounts */
int main(int argc, char* argv[]) {

    if (argc != 11) {
        printf("Usage: ./writer -f filename -l recid -v value -d time -s shmname\n"); 
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
            DataFilefd = open(argv[i+1], O_RDWR);       //open the file in read-and-write mode
            if (DataFilefd == -1) {
                perror("Writer failed to open \"filename\""); 
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

    // Keep "recid" given, which is a recid that will be written
    int recid;
    i = 1;
    done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-l") == 0) {       //flag for "recid"
            recid = atoi(argv[i+1]);         
            if (recid < 1 || recid > num_of_accounts) {
                printf("Malformed \"recid\" value given, it must be >= 1 and <= %d\n", num_of_accounts);
                exit(1);
            }
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"recid\" found, need to be give -l before it, exiting\n");
            exit(1);
        }
    }

    // Keep "value" given, which will be the value to modify recid's balance with
    int value;
    i = 1;
    done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-v") == 0) {       //flag for "value"
            value = atoi(argv[i+1]);         
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"value\" found, need to be give -v before it, exiting\n");
            exit(1);
        }
    }

    // Keep "time" given, which is the maximum working time (sleeping) of the writer
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
    struct shm_seg *shmsegment;     //shared memory segment struct
    SharedMemorySegmentfd = shm_open(shmname, O_RDWR, 0);   //open shared memory segment in read-write mode
    if (SharedMemorySegmentfd == -1) {
        perror("Writer failed on shm_open");
        exit(1);
    }
    // Map it to the logical address space of the Writer
    shmsegment = mmap(NULL, sizeof(*shmsegment), PROT_READ | PROT_WRITE , MAP_SHARED, SharedMemorySegmentfd, 0);
    if (shmsegment == MAP_FAILED) {
        perror("Writer failed on mmap");
        exit(1);
    }
    // 'SharedMemorySegmentfd' is no longer needed by Writer
    if (close(SharedMemorySegmentfd) == -1) {
        perror("Writer failed on close");
    }
    printf("Writer-PID %ld: Shared Memory Segment \"%s\" has been attached at address \"%p\"\n", (long) getpid(), shmname, shmsegment);


    // Open the existing Semaphores "sem_wrt", "sem_in"
    sem_t *sem_wrt = sem_open("sem_wrt", O_RDWR);
    if (sem_wrt == SEM_FAILED) {
        perror("Writer failed on sem_open(sem_wrt)");
        exit(1);
    }
    sem_t *sem_in = sem_open("sem_in", O_RDWR);
    if (sem_in == SEM_FAILED) {
        perror("Writer failed on sem_open(sem_in)");
        exit(1);
    }

//////////////////////////////////////////// WRITER ALGORITHM //////////////////////////////////////////
    if (sem_wait(sem_in) == -1) {                                       //P(sem_in)
        perror("Writer failed on sem_wait(sem_in)");
    }
        if (sem_wait(sem_wrt) == -1) {                                  //P(sem_wrt)
            perror("Writer failed on sem_wait(sem_wrt)");
        }
        /********************************Critical Section********************************/
            i = 0;
            bool flag = false;
            // Add writer (pid) into the array of active writers
            while (i < MaxActiveProcesses && flag == false) {
                if (shmsegment->active_Writers[i] == 0) {
                    shmsegment->active_Writers[i] = getpid();
                    flag = true;
                }
                i++;
                // If array of active writers is full, then a flag tells us 
                // that writers at some point exceeded the limit of max num_of_active_writers
                if (i == MaxActiveProcesses && flag == false) {
                    shmsegment->extra_Writers = true;
                }
            }

////////////Start of writing here (one writer at a time, and no readers)
            printf("\nWriter-PID %ld: writing into file\n", (long) getpid());

            // Statistic 5) Counting the time passed for the Writer to start writing in file
            t2 = (double) times(&tb2);
            long double real_waiting_time = (t2 - t1)/ticspersec;

            // Create a local record to retrieve the desired record from file
            struct Account record;
            printf("Writer-PID %ld: going to modify by %d the balance of the below record:\n", (long) getpid(), value);
            // Seek into the correct position in file, where the desired record is
            if (lseek(DataFilefd, (recid-1)*sizeof(struct Account), SEEK_SET) == -1) {
                perror("Writer failed to seek into DataFile");
                close(DataFilefd);
                exit(1);
            }
            // Read the record from file into the local record
            if (read(DataFilefd, &record, sizeof(struct Account)) == -1) {
                perror("Writer failed to read from DataFile");
                close(DataFilefd);
                exit(1);
            }
            printf("Writer-PID %ld: %d %s %s %d\n", (long) getpid(), record.recID, record.lname, record.fname, record.balance);
            fflush(stdout);
            // Simulating the "working time"
            sleep(random_sleep_time);
            // Modify the balance of the record
            record.balance += value;
            printf("Writer-PID %ld: changed record, with modified balance:\n", (long) getpid());
            printf("Writer-PID %ld: %d %s %s %d\n", (long) getpid(), record.recID, record.lname, record.fname, record.balance);
            // Seek into the correct position in file (just to be sure)
            if (lseek(DataFilefd, (recid-1)*sizeof(struct Account), SEEK_SET) == -1) {
                perror("Writer failed to seek into DataFile");
                close(DataFilefd);
                exit(1);
            }
            // Write the modified record back into file, in the same position it was retrieved from
            if (write(DataFilefd, &record, sizeof(struct Account)) == -1) {
                perror("Writer failed to write into DataFile");
                close(DataFilefd);
                exit(1);
            }
            // Seek back to the beginning of the file
            if (lseek(DataFilefd, 0, SEEK_SET) == -1) {
                perror("Writer failed to seek into DataFile");
                close(DataFilefd);
                exit(1);
            }

            printf("Writer-PID %ld: finished writing into file\n\n", (long) getpid());
////////////End of writing here

            i = 0;
            flag = false;
            // Remove writer (pid) from the array of active writers
            while (i < MaxActiveProcesses && flag == false) {
                if (shmsegment->active_Writers[i] == getpid()) {
                    shmsegment->active_Writers[i] = 0;
                    flag = true;
                }
                i++;
            }
            // Statistic 3) Number of Writers who worked on the file increases by 1
            shmsegment->Writers_finished++;

            // Statistic 4) Counting the time passed for the Writer to finish its operations
            t2 = (double) times(&tb2);
            long double real_whole_time = (t2 - t1)/ticspersec;
            shmsegment->Writers_times += real_whole_time;

            // Statistic 5) (cont) Comparing Writer's waiting_time with current max_waiting_time
            if (real_waiting_time > shmsegment->max_waiting_time) {
                shmsegment->max_waiting_time = real_waiting_time;
            }

            // Statistic 6) Number of Records that were accessed increases by 1 (one record written)
            shmsegment->Records_processed += 1;
        /********************************************************************************/
        if (sem_post(sem_wrt) == -1) {                                  //V(sem_wrt)
            perror("Writer failed on sem_post(sem_wrt)");
        }
    if (sem_post(sem_in) == -1) {                                       //V(sem_in)
        perror("Writer failed on sem_post(sem_in)");
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////

    // Close the Semaphores "sem_wrt", "sem_in"
    if (sem_close(sem_wrt) == -1) {
        perror("Writer failed on sem_close(sem_wrt)");
        exit(1);
    }
    if (sem_close(sem_in) < 0) {
        perror("Writer failed on sem_close(sem_in)");
        exit(1);
    }

    // Unmap shared memory segment from the logical address space of the writer
    if (munmap(shmsegment, sizeof(*shmsegment)) == -1) {
        perror("Writer failed on munmap");
        exit(1);
    }

    // Close file "filename" given
    if (close(DataFilefd) == -1) {
        perror("Writer failed to close DataFile"); 
        exit(1);
    }

    return 0;   //successful end of writer
}


// Function for extracting the integer from the DataFile name (e.g. number 50 from the name accounts50.bin)
int extractInteger(char* name) {
    int integer;
    sscanf(name, "accounts%d", &integer);
    return integer;
}