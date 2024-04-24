#include <stdio.h>
#include <string.h>     //for strcmp(), strcpy()
#include <fcntl.h>      //for flags of sem_open() and shm_open()
#include <semaphore.h>  //for sem_t, sem_open(), sem_close(), sem_unlink(), ...
#include <stdlib.h>     //for exit()
#include <sys/mman.h>   //for shm_open(), shm_unlink(), mmap(), munmap(), ...
#include <sys/stat.h>   //for SEM_PERMS
#include <unistd.h>     //for ftruncate(), close()

#include "structs.h"    //for Shared Memory Segment and Account structs

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)   //permissions for semaphores

/* Constructor of POSIX Semaphores and POSIX Shared Memory Segment */
int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: ./constructor -s shmname\n"); 
        exit(1);
    }

    // Keep "shmname" given, which is the name of the POSIX Shared Memory Segment that will be created and initialized
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


    // Create and Initilize the POSIX Semaphores (named)
    sem_t *sem_mutex = sem_open("sem_mutex", O_CREAT | O_EXCL, SEM_PERMS, 1);
    if (sem_mutex == SEM_FAILED) {
        perror("Constructor failed on sem_open(sem_mutex)");
        exit(1);
    }
    sem_t *sem_wrt = sem_open("sem_wrt", O_CREAT | O_EXCL, SEM_PERMS, 1);
    if (sem_wrt == SEM_FAILED) {
        perror("Constructor failed on sem_open(sem_wrt)");
        exit(1);
    }
    sem_t *sem_in = sem_open("sem_in", O_CREAT | O_EXCL, SEM_PERMS, 1);
    if (sem_in == SEM_FAILED) {
        perror("Constructor failed on sem_open(sem_in)");
        exit(1);
    }
    // Close them as they are not going to be used by the constructor
    if (sem_close(sem_mutex) == -1) {
        perror("Constructor failed on sem_close(sem_mutex)");
        if (sem_unlink("sem_mutex") == -1) {
            perror("Constructor failed on sem_unlink(sem_mutex) too");
        }
        exit(1);
    }
    if (sem_close(sem_wrt) == -1) {
        perror("Constructor failed on sem_close(sem_wrt)");
        if (sem_unlink("sem_wrt") == -1) {
            perror("Constructor failed on sem_unlink(sem_wrt) too");
        }
        exit(1);
    }
    if (sem_close(sem_in) < 0) {
        perror("Constructor failed on sem_close(sem_in)");
        if (sem_unlink("sem_in") == -1) {
            perror("Constructor failed on sem_unlink(sem_in) too");
        }
        exit(1);
    }


    // Create and Initilialize the POSIX Shared Memory Segment
    int SharedMemorySegmentfd;
    SharedMemorySegmentfd = shm_open(shmname, O_CREAT | O_RDWR, 0600);
    if (SharedMemorySegmentfd == -1) {
        perror("Constructor failed on shm_open");
        exit(1);
    }

    // Set its size to the size of the shm_seg struct
    if (ftruncate(SharedMemorySegmentfd, sizeof(struct shm_seg)) == -1) {
        perror("Constructor failed on ftruncate");
        exit(1);
    }

    struct shm_seg *shmsegment;
    // Map it to the logical address space of the constructor
    shmsegment = mmap(NULL, sizeof(*shmsegment), PROT_READ | PROT_WRITE, MAP_SHARED, SharedMemorySegmentfd, 0);
    if (shmsegment == MAP_FAILED) {
        perror("Constructor failed on mmap");
        exit(1);
    }
    // 'SharedMemorySegmentfd' is no longer needed by constructor
    if (close(SharedMemorySegmentfd) == -1) {
        perror("Constructor failed on close");
        if (shm_unlink(shmname) == -1) {
            perror("Constructor failed on shm_unlink too");
        }
        exit(1);
    }

    // Initialize the elements of the shared memory segment
    shmsegment->readcount = 0;
    for (int i = 0; i < MaxActiveProcesses; i++) {
        shmsegment->active_Readers[i] = 0;
        shmsegment->active_Writers[i] = 0;
    }
    shmsegment->extra_Readers = false;
    shmsegment->extra_Writers = false;
    shmsegment->Readers_finished = 0;
    shmsegment->Readers_times = 0;
    shmsegment->Writers_finished = 0;
    shmsegment->Writers_times = 0;
    shmsegment->max_waiting_time = 0;
    shmsegment->Records_processed = 0;


    // Unmap the shared memory segment from the logical address space of the constructor
    if (munmap(shmsegment, sizeof(*shmsegment)) == -1) {
        perror("Constructor failed on munmap");
        exit(1);
    }

    printf("\nConstructor created and initialized the Semaphores: \"sem_mutex\", \"sem_wrt\", \"sem_in\"\n\
and the Shared Memory Segment with name: \"%s\"\n\n", shmname);
    printf("Readers-Writers (\"reader\" and \"writer\" programs) can be now called simultaneously and multiple times,\n\
(from different ttys) using the \"%s\" as their shared memory segment\n\n", shmname);
    printf("In order to see the statistics after the execution of readers-writers, call the \"results\" program\n\n");
    printf("To delete the semaphores, and the shared memory segment \"%s\", after the completion of all operations desired, call the \"destructor\" program\n\n", shmname);

    return 0;   //successful end of contructor
}