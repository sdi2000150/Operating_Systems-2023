#include <stdio.h>
#include <string.h>     //for strcmp(), strcpy()
#include <fcntl.h>      //for flags of shm_open()
#include <stdlib.h>     //for exit()
#include <sys/mman.h>   //for shm_open(), mmap(), munmap(), ...
#include <unistd.h>     //for close()

#include "structs.h"    //for Shared Memory Segment and Account structs

/* Program "results" for printing the statistics after the execution of readers-writers */
int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: ./results -s shmname\n"); 
        exit(1);
    }

    // Keep "shmname" given, which is the name of the POSIX Shared Memory Segment that will be opened and accessed
    char shmname[25];  //fixed-length string to keep the name of the shared memory segment
    int i = 1;
    bool done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-s") == 0) {  //flag for "shmname"
            strcpy(shmname, argv[i+1]);
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for \"shmname\" (shared memory segment) found, need to give -s before it, exiting\n");
            exit(1);
        }
    }

    // Open the existing Shared Memory Segment of name given
    int SharedMemorySegmentfd;      //int, to keep the file descriptor
    struct shm_seg *shmsegment;     //shared memory segment struct
    SharedMemorySegmentfd = shm_open(shmname, O_RDONLY, 0);   //open shared memory segment in read mode
    if (SharedMemorySegmentfd == -1) {
        perror("Program \"results\" failed on shm_open");
        exit(1);
    }
    // Map it to the logical address space of the Program "results"
    shmsegment = mmap(NULL, sizeof(*shmsegment), PROT_READ, MAP_SHARED, SharedMemorySegmentfd, 0);
    if (shmsegment == MAP_FAILED) {
        perror("Program \"results\" failed on mmap");
        exit(1);
    }
    // 'SharedMemorySegmentfd' is no longer needed by Program "results"
    if (close(SharedMemorySegmentfd) == -1) {
        perror("Program \"results\" failed on close");
    }

    printf("\nStatistics (stored inside shared memory segment \"%s\") of the until-now execution of readers-writers:\n\n", shmname);

    printf("1) Number of Readers who worked on the file: %d\n", shmsegment->Readers_finished);
    long double avg_readers_time = shmsegment->Readers_times / (double)shmsegment->Readers_finished;
    printf("2) Average time in which Readers worked: %Lf\n", avg_readers_time);
    printf("3) Number of Writers who worked on the file: %d\n", shmsegment->Writers_finished);
    long double avg_writers_time = shmsegment->Writers_times / (double)shmsegment->Writers_finished;
    printf("4) Average time in which Writers worked: %Lf\n", avg_writers_time);
    printf("5) Max waiting latency before starting working (of Reader or Writer): %Lf\n", shmsegment->max_waiting_time);
    printf("6) Number of Records that were accessed (for Reading or Writing): %d\n", shmsegment->Records_processed);
    printf("\n");

    // Unmap shared memory segment from the logical address space of the Program "results"
    if (munmap(shmsegment, sizeof(*shmsegment)) == -1) {
        perror("Program \"results\" failed on munmap");
        exit(1);
    }

    return 0;   //successful end of Program "results"
}