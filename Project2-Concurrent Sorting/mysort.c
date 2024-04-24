#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/wait.h>   //wait()
#include <fcntl.h>      //open()
#include <unistd.h>     //close(), lseek(), getpid(), getppid(), pipe(), fork(), dup2(), execlp(), read(), write()
#include <string.h>     //strcmp(), strcpy(), strcat
#include <math.h>       //fmod() and ceil() usage (for helping in slicing the file)
#include <stdbool.h>    //bool type usage (for "done" variable)
#include <signal.h>     //kill() usage (for sending "SIGUSR1"), signal() usage (for "SIGUSR1" and "SIGUSR2" handling)
#include <errno.h>      //errno number usage (for "EINTR" error handling)
#include "mysort_prototypes.h"

#define READ 0      //read-end for pipes
#define WRITE 1     //write-end for pipes

// Main function - Coordinator of hierarchy of processes
int main(int argc, char *argv[]){

    if (argc != 9) {
        printf("Usage: ./mysort -i DataFile -k NumofChildren -e1 sorting1 -e2 sorting2\n"); 
        exit(1);
    }

    // Keep k, which is the NumofChildren Coordinator will create (and each Splitter/Merger will create variably based on that)
    int k;
    int i = 1;
    bool done = 0;
    while (i < argc && done == 0) {
        if (strcmp(argv[i], "-k") == 0) {       //flag for "NumofChildren"
            k = atoi(argv[i+1]);                //k = Number of Children (given by user)
            done = 1;
            if (k < 1) {
                printf("Malformed <NumofChildren> value given, it must be >= 1\n");
                exit(1);
            }
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for NumofChildren found, need to be give -k before it, exiting\n");
            exit(1);
        }
    }

    // Open file, and keep the file descriptor
    int DataFilefd;         //int, to keep the file descriptor
    char DataFileName[25];  //fixed-length string to keep the name of DataFile
    i = 1;
    done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-i") == 0) {               //flag for file "DataFile"
            DataFilefd = open(argv[i+1], O_RDONLY);     //open the file in read-only mode
            done = 1;
            if (DataFilefd == -1) {
                perror("Coordinator failed to open DataFile"); 
                exit(1);
            }
            strcpy(DataFileName, argv[i+1]);            //name of DataFile
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for DataFile found, need to give -i before it, exiting\n");
            exit(1);
        }
    }

    // Keep sorting1, which is the 1st sorting program that will be used
    char sorting1[25];  //fixed-length string to keep the name of 1st sorting program
    i = 1;
    done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-e1") == 0) {  //flag for "sorting1"
            strcpy(sorting1, argv[i+1]);    //1st sorting program
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for sorting program 1 found, need to give -e1 before it, exiting\n");
            close(DataFilefd);
            exit(1);
        }
    }

    // Keep sorting2, which is the 2nd sorting program that will be used
    char sorting2[25];  //fixed-length string to keep the name of 2nd sorting program
    i = 1;
    done = 0;
    while(i < argc && done == 0) {
        if (strcmp(argv[i], "-e2") == 0) {  //flag for "sorting2"
            strcpy(sorting2, argv[i+1]);    //2nd sorting program
            done = 1;
        }
        i++;
        if (i == argc && done == 0) {   //no argument is the flag we are looking for
            printf("Not correct flag for sorting program 2 found, need to give -e2 before it, exiting\n");
            close(DataFilefd);
            exit(1);
        }
    }


    // Move to the end of the file, and keep the size of file
    int filesize = lseek(DataFilefd, 0, SEEK_END);
    if (filesize == -1) {
        perror("Coordinator failed to seek into DataFile");
        close(DataFilefd);
        exit(1);
    }

    // Keep the number of records the file has
    int num_of_records = filesize/sizeof(Record);

    // Move back to the beginning of the file
    if (lseek(DataFilefd, 0, SEEK_SET) == -1) {
        perror("Coordinator failed to seek into DataFile");
        close(DataFilefd);
        exit(1);
    }

    // Close file
    if (close(DataFilefd) == -1) {
        perror("Coordinator failed to close DataFile"); 
        exit(1);
    }

    int j;
    // Creating TWO 2D ARRAYS, mapped between them, to keep the STARTING & ENDING POSITION in file, in which each Sorter will sort:
    int** starting_pos = malloc(k * sizeof(int*));          //2D dynamic array keeping the starting positions 
    int** ending_pos = malloc(k * sizeof(int*));            //2D dynamic array keeping the ending positions
    int num_of_records_in_splitter[k];                  //creating an array for keeping the number of records each Splitter/Merger will deal with
    for (i = 0; i < k; i++) {
        starting_pos[i] = malloc((k-i) * sizeof(int));      //k-i columns(not fixed)
        ending_pos[i] = malloc((k-i) * sizeof(int));        //k-i columns(not fixed)
        num_of_records_in_splitter[i] = 0;                  //initialized to 0 (will be filled as a sum
    }
    double slice;                       //percentage of file, each child-Sorter will sort
    int sliced_num_of_recs;             //number of records, each child-Sorter will sort
    int sliced_segment;                 //number of bytes, each child-Sorter will sort
    int end_prev = 0;                   //keeping each time the ending position in file of the last segment
    double spare_part = 0;              //sum of potential fractional parts of doubles casted to integers
    for (i = 0; i < k; i++) {
        // Slice of records, each child-Sorter of i Splitter/Merger will sort
        slice = (1/(double)k)/(double)(k-i);
        sliced_num_of_recs = slice*(double)num_of_records;
        sliced_segment = sliced_num_of_recs*sizeof(Record);

        for (j = 0; j < k-i; j++) {  
            starting_pos[i][j] = end_prev + j*sliced_segment;
            ending_pos[i][j] = end_prev + (j+1)*sliced_segment;

            //if records are not perfectly divided to all Sorters, but with fractional parts,
            //variable spare_part keeps all the fractional parts remaining, after bottom rounding into integer of each sliced_num_of_recs,
            spare_part += fmod(slice*(double)num_of_records,(double)sliced_num_of_recs);
            if (i == k-1) {
                //and it is added to the last child-Sorter(so, maybe he will sort a few more records than he should)
                ending_pos[i][j] += (int)ceil(spare_part) * sizeof(Record);
            }
            num_of_records_in_splitter[i] += (ending_pos[i][j] - starting_pos[i][j])/sizeof(Record);
        }
        end_prev = ending_pos[i][j-1];
    }


    // Creating TWO ARRAYS for the PIPES that will be created:
    // One 2D array for keeping the pipes for the return communication between Splitters/Mergers and Coordinator
    int** merger_pipe = malloc(k*sizeof(int*));
    // One 3D array for keeping the pipes for the return communication between Sorters and Splitters/Mergers
    int*** sorter_pipe = malloc(k*sizeof(int**));
    for (i = 0; i < k; i++) {
        merger_pipe[i] = malloc(2 * sizeof(int));       //2 columns(fixed), for read fd & write fd
        sorter_pipe[i] = malloc((k-i)*sizeof(int*));
        for (j = 0; j < k-i; j++) {
            sorter_pipe[i][j] = malloc(2 * sizeof(int));  //2 "columns"(fixed), for read fd & write fd
        }
    }

    // Creating TWO ARRAYS for the SORTED RECORDS returned by children-processes:
    // One 2D array for keeping the sorted & merged records returned by Splitters/Mergers to Coordinator 
    Record** merged_records = malloc(k*sizeof(Record*));
    // One 3D array for keeping the sorted records returned by Sorters to Splitters/Mergers
    Record*** records = malloc(k*sizeof(Record**));
    for (i = 0; i < k; i++) {
        merged_records[i] = malloc(num_of_records_in_splitter[i] * sizeof(Record));
        records[i] = malloc((k-i)*sizeof(Record*));
        for (j = 0; j < k-i; j++) {
            int num_of_records_in_sorter = (ending_pos[i][j] - starting_pos[i][j])/sizeof(Record);
            records[i][j] = malloc(num_of_records_in_sorter*sizeof(Record));
        }
    }

    //Creating one 3D ARRAY for keeping the 2 TIMES of every sorter's execution
    long double*** times = malloc(k*sizeof(long double**));
    for (i = 0; i < k; i++) {
        times[i] = malloc((k-i)*sizeof(long double*));
        for (j = 0; j < k-i; j++) {
            times[i][j] = malloc(2 * sizeof(long double));
        }
    }

    //Signal handling of signals USR1 and USR2 send by children, Splitters/Mergers and Sorters, to Coordinator
    signal(SIGUSR1, USR1_handler);
    signal(SIGUSR2, USR2_handler);



    /* Some process_no counters and prints, for testing. Commented out */
    // int process_no = 0;
    // int process_sorter_no = 0;
    // printf("I am process no %d(Coordinator) with PID %d and PPID %d\n", process_no, getpid(), getppid());
    int splitter_pid , sorter_pid, status;
    int Coordinator_PID = getpid();

    /* Coordinator creates k Splitters/Mergers*/
    for (i = 0; i < k; i++) {
        // process_no++;

        if (pipe(merger_pipe[i]) == -1) {
            perror("Process Coordinator failed create a pipe");
            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
            exit(1);
        }

        fflush(stdout); //needs to be here(before fork) to avoid printing the last printf when output of program is written in another file
        splitter_pid = fork();
        if (splitter_pid == -1) {
            perror("Process Coordinator failed to fork");
            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
            exit(1);

        } else if (splitter_pid == 0) { //child-Splitter/Merger
            // printf(" I am process no %d(Splitter/Merger) with PID %d and PPID %d\n", process_no, getpid(), getppid());

            // Will send sorted records to parent-Coordinator
            close(merger_pipe[i][READ]);    //close read-end
            dup2(merger_pipe[i][WRITE], 3); //write-end duplicated to file-desc 3, so child(Splitter/Merger) will write his sorted records to fd 3
            close(merger_pipe[i][WRITE]);   //and initial write-end is closed too

            /* Each Splitter/Merger creates k-i Sorters */
            for (j = 0; j < k-i; j++) {
                // process_sorter_no++;

                if (pipe(sorter_pipe[i][j]) == -1) {
                    perror(" Process Splitter/Merger failed create a pipe");
                    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                    exit(1);
                }

                fflush(stdout); //needs to be here(before fork) to avoid printing the last printf, when output of program is written in another file
                sorter_pid = fork();
                if (sorter_pid == -1) {
                    perror(" Process Splitter/Merger failed to fork");
                    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                    exit(1);

                } else if (sorter_pid == 0) { //child-Sorter
                    // printf("  I am process no %d.%d(Sorter) with PID %d and PPID %d\n", process_no, process_sorter_no, getpid(), getppid());

                    char starting_pos_str[100];
                    char ending_pos_str[100];
                    sprintf(starting_pos_str, "%d", starting_pos[i][j]);  //convert the integer to a string (in order to be passed into exec as an argument)
                    sprintf(ending_pos_str, "%d", ending_pos[i][j]);      //convert the integer to a string (in order to be passed into exec as an argument)

                    char Coordinator_PID_str[50];
                    sprintf(Coordinator_PID_str, "%d", Coordinator_PID);    //convert the integer to a string (in order to be passed into exec as an argument)
                    
                    fflush(stdout); //needs to be here(before dup and exec) to print the last printf elements of stdout, before overwritting it and before executing another program

                    // Will send sorted records to parent-Splitter/Merger i
                    close(sorter_pipe[i][j][READ]);     //close read-end
                    dup2(sorter_pipe[i][j][WRITE], 1);  //write-end duplicated to file-desc 1, so child's(Sorter's) stdout is now overwritten as pipe's write-end 
                    close(sorter_pipe[i][j][WRITE]);    //and initial write-end is closed too

                    /* Each Sorter execs (alternately with sorting1 or sorting2 program)*/
                    if (j % 2 == 0) {   //if j is even exec the sorting1 program
                        char path[100] = "./";      //fixed-length string to keep the path of 1st sorting program
                        strcat(path, sorting1);     //and name of sorting program is attached to the end of the path, in order for the path to be complete
                        //exec the 1st sorting program, with arguments: the DataFile, the starting position and the ending position in file in which Sorter will sort
                        if (execlp(path, sorting1, DataFileName, starting_pos_str, ending_pos_str, Coordinator_PID_str, (char*) NULL) == -1) {
                            perror("  Process-Sorter failed to exec into sorting1 program");
                            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                            exit(1);
                        }
                    } else {    //if j is odd exec the sorting2 program
                        char path[100] = "./";      //fixed-length string to keep the path of 2nd sorting program
                        strcat(path, sorting2);     //and name of sorting program is attached to the end of the path, in order for the path to be complete
                        //exec the 2nd sorting program, with arguments: the DataFile, the starting position and the ending position in file in which Sorter will sort
                        if (execlp(path, sorting2, DataFileName, starting_pos_str, ending_pos_str, Coordinator_PID_str, (char*) NULL) == -1) {
                            perror("  Process-Sorter failed to exec into sorting2 program");
                            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                            exit(1);
                        }
                    }

                } else { //parent-Splitter/Merger
                    //does nothing for now, just continues with the for loop in order to create all his children-Sorters 
                }
            }
            //Only Splitter/Merger continues here:
            //1) Receives sorted records by children-Sorters
            for (j = 0; j < k-i; j++) {
                // Receiving sorted records by child-Sorter j
                close(sorter_pipe[i][j][WRITE]);    //close write-end
                int num_of_records_in_sorter = (ending_pos[i][j] - starting_pos[i][j])/sizeof(Record);
                for (int z = 0; z < num_of_records_in_sorter; z++) {
                    while (read(sorter_pipe[i][j][READ], &records[i][j][z], sizeof(Record)) == -1) {   //read records from read-end of pipe
                        if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                            continue;           //repeat the read syscall
                        } else {
                            perror(" Process-Splitter/Merger failed to read record(s) from pipe with his child-Sorter");
                            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                            exit(1);
                        }
                    }
                }
                // Receiving times, run_time and cpu_usage, by child-Sorter j
                while (read(sorter_pipe[i][j][READ], &times[i][j][0], sizeof(long double)) == -1) {   //read run_time from read-end of pipe
                    if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                        continue;           //repeat the read syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to read the run_time from pipe with his child-Sorter");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
                while (read(sorter_pipe[i][j][READ], &times[i][j][1], sizeof(long double)) == -1) {   //read cpu_usage from read-end of pipe
                    if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                        continue;           //repeat the read syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to read the cpu_usage from pipe with his child-Sorter");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
                close(sorter_pipe[i][j][READ]); //close read-end
            }

            //2) Waits for all his children-Sorters
            for (j = 0; j < k-i; j++) {
                while (wait(&status) == -1) {   //wait for a child-Sorter
                    if (errno == EINTR) {   //if wait syscall was interrupted by a signal before reaping the status of the child-Sorter,
                        continue;           //repeat the wait syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to wait for his child-Sorter");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
            }

            //3) Merges all sorted slices taken by children-Sorters
            int* records_sizes = malloc((k-i)*sizeof(int));
            for (j = 0; j < k-i; j++) {
                int num_of_records_in_sorter = (ending_pos[i][j] - starting_pos[i][j])/sizeof(Record);
                records_sizes[j] = num_of_records_in_sorter;
            }
            
            Record* medial_merged_records = MergeSort(records[i], records_sizes, k-i);

            //writing merged/sorted records into file-desc 3, which is pipefd[Write] between him and Coordinator
            int merged_size = 0;
            for (j = 0; j < k-i; j++) {
                merged_size += records_sizes[j];
            }
            for (j = 0; j < merged_size; j++) {
                while (write(3, &medial_merged_records[j], sizeof(Record)) == -1) {   //write into fd 3(write-end) of pipe
                    if (errno == EINTR) {   //if write syscall was interrupted by a signal before any data was written,
                        continue;           //repeat the write syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to write into pipe with his parent-Coordinator");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
            }
            //and writing the times too, received from children-Sorters
            for (j = 0; j < k-i; j++) {
                while (write(3, &times[i][j][0], sizeof(long double)) == -1) {   //write into fd 3(write-end) of pipe
                    if (errno == EINTR) {   //if write syscall was interrupted by a signal before any data was written,
                        continue;           //repeat the write syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to write into pipe with his parent-Coordinator");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
                while (write(3, &times[i][j][1], sizeof(long double)) == -1) {   //write into fd 3(write-end) of pipe
                    if (errno == EINTR) {   //if write syscall was interrupted by a signal before any data was written,
                        continue;           //repeat the write syscall
                    } else {
                        perror(" Process-Splitter/Merger failed to write into pipe with his parent-Coordinator");
                        free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                        exit(1);
                    }
                }
            }
            close(3);   //close file-desc 3, which was overwritten as write-end of the pipe communication between Splitter/Merger - Coordinator

            //4) Sends a USR1 signal to Coordinator
            int Coordinator_PID = getppid();
            if (kill(Coordinator_PID, SIGUSR1) == -1) {
                perror(" Process-Splitter/Merger failed send a USR1 signal to his parent-Coordinator");
                free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                exit(1);
            }

            //5) Exits
            free(medial_merged_records);
            free(records_sizes);
            free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
            exit(0);

        } else { //parent-Coordinator
            //does nothing for now, just continues with the for loop in order to create all his children-Splitters/Mergers
        }
    }
    //Only Coordinator continues here:
    //1) Receives sorted records by children-Splitters/Mergers
    for (i = 0; i < k; i++) {
        // Receiving sorted records by child-Splitter/Merger i
        close(merger_pipe[i][WRITE]);   //close write-end
        for (int z = 0; z < num_of_records_in_splitter[i]; z++) {
            while (read(merger_pipe[i][READ], &merged_records[i][z], sizeof(Record)) == -1) {   //read records from read-end of pipe
                if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                    continue;           //repeat the read syscall
                } else {
                    perror("Process-Coordinator failed to read record(s) from pipe with his child-Splitter/Merger");
                    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                    exit(1);
                }
            }
        }
        for (j = 0; j < k-i; j++) {
            // Receiving times, run_time and cpu_usage, by each child-Sorter j
            while (read(merger_pipe[i][READ], &times[i][j][0], sizeof(long double)) == -1) {   //read run_time from read-end of pipe
                if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                    continue;           //repeat the read syscall
                } else {
                    perror("Process-Coordinator failed to read the run_time from pipe with his child-Splitter/Merger");
                    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                    exit(1);
                }
            }
            while (read(merger_pipe[i][READ], &times[i][j][1], sizeof(long double)) == -1) {   //read cpu_usage from read-end of pipe
                if (errno == EINTR) {   //if read syscall was interrupted by a signal before any data was read,
                    continue;           //repeat the read syscall
                } else {
                    perror("Process-Coordinator failed to read the cpu_usage from pipe with his child-Splitter/Merger");
                    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                    exit(1);
                }
            }
        }
        close(merger_pipe[i][READ]);    //close read-end
    }

    //2) Waits for all his children-Splitters/Mergers
    for (i = 0; i < k; i++) {
        while (wait(&status) == -1) {   //wait for a child-Splitter/Merger
            if (errno == EINTR) {   //if wait syscall was interrupted by a signal before reaping the status of the child-Splitter/Merger,
                continue;           //repeat the wait syscall
            } else {
                perror("Process-Coordinator failed to wait for his child-Splitter/Merger");
                free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
                exit(1);
            }
        }
    }

    //3) Merges all sorted slices taken by children-Splitters/Mergers
    int* merged_records_sizes = malloc(k*sizeof(int));
    for (i = 0; i < k; i++) {
        merged_records_sizes[i] = num_of_records_in_splitter[i];
    }
    Record* final_merged_records = MergeSort(merged_records, merged_records_sizes, k);
    int final_merged_size = 0;
    for (i = 0; i < k; i++) {
        final_merged_size += merged_records_sizes[i];
    }

    //4) Prints all the, sorted, records of the file
    printf("1.Printing all the sorted records, based on last name in ascending order (and secondarily on first name, and ID):\n\n");
    for (i = 0; i < final_merged_size; i++) {
        printf("%-12s %-12s %-6d %s\n", final_merged_records[i].lname, final_merged_records[i].fname, final_merged_records[i].ID, final_merged_records[i].zipcode);
    }
    printf("\n\n");

    //5) Prints the time consumed by every Sorter
    printf("2.Printing the time needed by every Sorter to finish its operations:\n\n");
    for (i = 0; i < k; i++) {
        printf("Splitter/Merger no_%d:\n", i+1);
        for (j = 0; j < k-i; j++) { 
            printf("    Sorter no_%d: run time was %Lf sec (REAL time), although he used the CPU for %Lf sec (CPU time)\n", j+1, times[i][j][0], times[i][j][1]);
        }
    }
    printf("\n\n");

    //6) Prints the number of USR1 and USR2 signals received
    printf("3.Printing the number of signals Coordinator received by all his children:\n");
    printf("(as unreliable, non queued, signals, some of them may be lost, so the numbers vary among different executions)\n\n");
    printf("Number of USR1 signals received (sent from Splitters/Mergers): %d\n", USR1_counter);
    printf("Number of USR2 signals received (sent from Sorters): %d\n", USR2_counter);

    //7) Exits
    free(final_merged_records);
    free(merged_records_sizes);
    free_function(starting_pos, ending_pos, k, merger_pipe, sorter_pipe, merged_records, records, times);
    return 0;
}