#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>      //open()
#include <unistd.h>     //close(), pread(), write(), sysconf()
#include <sys/times.h>  //times() usage (and tms tb1 & tb2)
#include <signal.h>     //kill() usage (for sending "SIGUSR2")
#include "mybubblesort_prototype.h"


// Main function - Program for sorting records with BubbleSort
int main(int argc, char* argv[]) {

    if (argc != 5) {
        //print to stderr, because file-desc 1 (stdout,e.g. printf) is overwritten as pipefd[Write] when executed by ./mysort program
        fprintf(stderr, "   Usage: ./mybubblesort DataFile starting_pos ending_pos Coordinator_PID\n");
        exit(1);
    }

    //Starting the counters of time that mybubblesort program will need to finish its operations
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    int DataFilefd = open(argv[1], O_RDONLY);   //open read-only
    if (DataFilefd == -1) { 
        perror("    Program \"mybubblesort\" failed to open DataFile"); 
        exit(1);
    }

    int starting_pos = atoi(argv[2]);
    int ending_pos = atoi(argv[3]);

    int num_of_bytes_to_read = ending_pos - starting_pos + 1;
    int num_of_records = num_of_bytes_to_read/sizeof(Record);
    Record* records = malloc(num_of_records*sizeof(Record));

    int bytes_read = pread(DataFilefd, records, num_of_records*sizeof(Record), starting_pos);
    if (bytes_read == -1) {
        perror("    Program \"mybubblesort\" failed to read from DataFile");
        free(records);
        close(DataFilefd);
        exit(1);
    }
    if (close(DataFilefd) == -1) { 
        perror("    Program \"mybubblesort\" failed to close DataFile");
        free(records);
        exit(1);
    } 

    //Sorting slice of records
    BubbleSort(records, num_of_records);

    //Writing sorted records into file-desc 1, which is pipefd[Write] when executed by ./mysort parent (or stdout when executed by its own)
    for (int i = 0; i < num_of_records; i++) {
        if (write(1, &records[i], sizeof(Record)) == -1) {
            perror("    Program \"mybubblesort\" failed to write to file descriptor of pipe");
        }
    } 
    free(records);

    //Counting the time passed for the mybubblesort program to finish its operations,
    t2 = (double) times(&tb2);
    cpu_time = (double) ((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime));
    long double run_time = (t2 - t1)/ticspersec;
    long double cpu_usage = cpu_time/ticspersec;
    //and sending the times to parent-Splitter/Merger
    if (write(1,&run_time, sizeof(long double)) ==  -1 ) {
        perror("    Program \"mybubblesort\" failed to write to file descriptor of pipe");
    }
    if (write(1,&cpu_usage, sizeof(long double)) == -1) {
        perror("    Program \"mybubblesort\" failed to write to file descriptor of pipe");
    }
    close(1);

    //Sending a USR2 signal to Coordinator
    int Coordinator_PID = atoi(argv[4]);
    if (kill(Coordinator_PID, SIGUSR2) == -1) {
        perror("    Program \"mybubblesort\" failed to send a SIGUSR2 signal to Coordinator");
    }

    exit(0);
}