/* cmpcat.c - main function of the program */

#define _GNU_SOURCE     // ino_t, ... usage
#include <stdio.h>      // FILE*, printf(), fopen() fclose(), fprintf(), rewind(), fscanf()
#include <stdlib.h>     // exit(), free() usage
#include <string.h>     // strcmp() usage
#include <sys/stat.h>   // mkdir() usage
#include <stdbool.h>    // bool type usage

#include "interface.h"

int main(int argc, char *argv[]){
    if((argc != 4) && (argc != 6)){
        printf("Malformed input, usage: ./cmpcat -d dirA dirB [-s dirC].\n");
        exit(1);
    }

    const char *dirA = NULL;                                                                    // first directory to compare
    const char *dirB = NULL;                                                                    // second directory to compare
    const char *dirC = NULL;                                                                    // combined directory
    int i = 1;                                                                                  // 0 refers to the executable's name
    bool flag_d = false;                                                                        // to check if -d exists
    bool flag_s = false;                                                                        // to check if -s exists (when argc==6)
    while(i < argc){
        if(!strcmp(argv[i], "-d")){                                                             // flag for first and second directory
            dirA = argv[i+1];                                                                   // get the first directory
            if(!entityExists(dirA)){                                                            // check if it exists
                printf("The first directory for compare given doesn't exist.\nProgram will now exit.\n");
                exit(1);
            }
            dirB = argv[i+2];                                                                   // get the second directory
            if(!entityExists(dirB)){                                                            // check if it exists
                printf("The second directory for compare given doesn't exist.\nProgram will now exit.\n");
                exit(1);
            }
            flag_d = true;                                                                      // flag for dirA and dirB was succesfully found
        }else if(!strcmp(argv[i], "-s")){
            dirC = argv[i+1];                                                                   // get the name of the combined directory
            if(entityExists(dirC)){                                                             // check if it exists
                printf("Directory '%s' given for merging already exists. Delete it and call the program again.\n", dirC);
                exit(1);
            }
            if (mkdir(dirC, 0777) == -1) {                                                      //create a new empty directory
                printf("Error creating new directory.\n");
                exit(1);
            }
            flag_s = true;                                                                      // flag for dirC was succesfully found
        }
        i++;
        if((i == argc) && (flag_d == false)){                                                   // if all args parsed but no -d flag found
            printf("Not correct flag for \"dirA dirB\" found, need to give -d before it, exiting.\n");
            exit(1);
        }
        if((i == 6) && (flag_s == false)){                                                      // if all args parsed but no -s flag found (when argc==6)
            printf("Not correct flag for \"dirC\" found, need to give -s before it, exiting.\n");
            exit(1);
        }
    }

    // 1) Directories (dirA & dirB) compare operations
    char *dirA_Differences, *dirB_Differences, *dirA_Entities, *dirB_Entities;
    FILE *fd_diffsA, *fd_diffsB, *fd_A, *fd_B;
    if(initializeFilesAndDescriptors(argv[0], &fd_diffsA, &fd_diffsB, &fd_A, &fd_B, &dirA_Differences, &dirB_Differences, &dirA_Entities, &dirB_Entities)){
        printf("ERROR\n");
        exit(1);
    }                                                                                           // initialize filenames and file descriptors

    Metadata header;                                                                            // initialize our metadata header
    header.dirA=0;
    header.dirB=0;
    fprintf(fd_diffsA, "%-20d\n", header.dirA);                                                 // write the header to the output file
    fprintf(fd_diffsB, "%-20d\n", header.dirB);                                                 // write the header to the output file

    compareDirectories(dirA, dirB, fd_diffsA, fd_diffsB, &header, fd_A, fd_B);                  // compare the two directories

    rewind(fd_diffsA);
    fprintf(fd_diffsA, "%-20d\n", header.dirA);                                                 // write the updated metadata back to the file
    rewind(fd_diffsB);
    fprintf(fd_diffsB, "%-20d\n", header.dirB);                                                 // write the updated metadata back to the file

    if((header.dirA!=0)||(header.dirB!=0)){
        char ftype[2];                                                                          // store the entity type indicator
        char path[1024];                                                                        // path size
        ino_t ino1, ino2;                                                                       // inodes
        if(header.dirA!=0){                                                                     // there are differences in dirA
            printf("\nIn %s:\n", dirA);
            while(fscanf(fd_diffsA, "%s %s %lu %lu\n", ftype, path, &ino1, &ino2)>0){
                printf("        %s\n", path);
            }
        }

        if(header.dirB!=0){                                                                     // there are differences in dirB
            printf("\nIn %s:\n", dirB);
            while(fscanf(fd_diffsB, "%s %s %lu %lu\n", ftype, path, &ino1, &ino2)>0){
                printf("        %s\n", path);
            }
        }
    }

    // 2) New directory (dirC) merge operations
    if(argc == 6){                                                                              // if -s dirC arguments given, then we do the merge
        rewind(fd_diffsA);
        rewind(fd_diffsB);
        rewind(fd_A);
        rewind(fd_B);

        printf("\n");
        if(header.dirA > header.dirB){                                                          // core directory is dirA (because it has more differences)
            mergeDirectories(fd_A, dirA_Entities, fd_diffsB, dirB_Differences, dirC, fd_B, dirA, dirB);
        }else{                                                                                  // core directory is dirB (because it has more differences)
            mergeDirectories(fd_B, dirB_Entities, fd_diffsA, dirA_Differences, dirC, fd_A, dirB, dirA);
        }
    }

    fclose(fd_diffsA);
    fclose(fd_diffsB);
    fclose(fd_A);
    fclose(fd_B);

    free(dirA_Differences);
    free(dirB_Differences);
    free(dirA_Entities);
    free(dirB_Entities);;

    return 0;
}