#include <stdio.h>
#include <string.h>     //for strcmp(), strcpy()
#include <semaphore.h>  //for sem_unlink()
#include <stdlib.h>     //for exit()
#include <sys/mman.h>   //for shm_unlink()

#include "structs.h"    //for Shared Memory Segment and Account structs

/* Destructor of POSIX Semaphores and POSIX Shared Memory Segment that were created by constructor */
int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: ./destructor -s shmname\n"); 
        exit(1);
    }

    // Keep "shmname" given, which is the name of the POSIX Shared Memory Segment that will be destroyed
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

    // Destroy the Shared Memory Segment (that was created by constructor)
    if (shm_unlink(shmname) == -1) {
        perror("Destructor failed on shm_unlink");
        exit(1);
    }
    // Destroy the Semaphores (that were created by constructor)
    if (sem_unlink("sem_mutex") == -1) {
        perror("Destructor failed on sem_unlink(sem_mutex)");
        exit(1);
    }
    if (sem_unlink("sem_wrt") == -1) {
        perror("Destructor failed on sem_unlink(sem_wrt)");
        exit(1);
    }
    if (sem_unlink("sem_in") == -1) {
        perror("Destructor failed on sem_unlink(sem_in)");
        exit(1);
    }

    printf("\nDestructor deleted the Semaphores: \"sem_mutex\", \"sem_wrt\", \"sem_in\"\n\
and the Shared Memory Segment with name: \"%s\"\n\n", shmname);

    return 0;   //successful end of destructor
}