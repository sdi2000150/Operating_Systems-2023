/* functions.c - Functions definition */

#define _GNU_SOURCE     // ino_t, ... usage
#include <stdio.h>      // FILE*, printf(), fopen(), fclose(), fread(), fwrite(), fprintf(), fscanf() usage
#include <stdlib.h>     // malloc(), free(), realloc() usage
#include <string.h>     // strrchr(), strlen(), strncpy(), strcat(), strcmp(), memcmp(), strstr() usage
#include <unistd.h>     // readlink(), link(), access(), symlink(), close() usage
#include <fcntl.h>      // open() usage
#include <dirent.h>     // DIR*, opendir(), readdir(), closedir() usage
#include <sys/stat.h>   // stat(), lstat(), mkdir(), chmod(), S_ISREG(), S_ISDIR(), S_ISLNK() usage
#include <stdbool.h>    // bool type usage

#include "interface.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////        INITIALIZATION        /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that creates a file path in the samed directory of the executable, using the argv[0] argument */
char *createFilePath(const char *argv0, const char *filename){
    const char *lastSeparator = strrchr(argv0, '/');                                            // get last occurrence of '/' in argv[0]
    if(lastSeparator == NULL){                                                                  // '/' not found
        return NULL;
    }

    size_t newLength = strlen(argv0) - strlen(lastSeparator) + 1 + strlen(filename) + 1;        // reserve space for '/' and '\0'

    char *newName = (char *)malloc(newLength);                                                  // allocate memory for new filename
    if(newName == NULL){
        printf("Memory allocation failed.\n");
        return NULL;
    }

    strncpy(newName, argv0, strlen(argv0) - strlen(lastSeparator) + 1);                         // copy the path up to the last separator
    newName[lastSeparator - argv0 + 1] = '\0';                                                  // null-terminate the copied portion
    strcat(newName, filename);                                                                  // append the filename

    return newName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that initializes filenames and file descriptors */
int initializeFilesAndDescriptors(char *argv0, FILE **fd_diffsA, FILE **fd_diffsB, FILE **fd_A, FILE **fd_B,
    char **dirA_Differences, char **dirB_Differences, char **dirA_Entities, char **dirB_Entities){
    *dirA_Differences = createFilePath(argv0, "dirA_Differences.txt");
    if(*dirA_Differences==NULL){
        printf("Could not create file dirA_Differences.txt.\n");
        return 1;
    }
    *fd_diffsA = fopen(*dirA_Differences, "w+");
    if(*fd_diffsA == NULL){
        printf("Could not open file dirA_Differences.txt.\n");
        free(*dirA_Differences);
        return 1;
    }
    *dirB_Differences = createFilePath(argv0, "dirB_Differences.txt");
    if(*dirB_Differences==NULL){
        printf("Could not create file dirB_Differences.txt.\n");
        free(*dirA_Differences);
        return 1;
    }
    *fd_diffsB = fopen(*dirB_Differences, "w+");
    if(*fd_diffsB == NULL){
        printf("Could not open file dirB_Differences.txt.\n");
        fclose(*fd_diffsA);
        free(*dirA_Differences);
        free(*dirB_Differences);
        return 1;
    }

    *dirA_Entities = createFilePath(argv0, "dirA_Entities.txt");
    if(*dirA_Entities==NULL){
        printf("Could not create file dirA_Entities.txt.\n");
        fclose(*fd_diffsA);
        fclose(*fd_diffsB);
        free(*dirA_Differences);
        free(*dirB_Differences);
        return 1;
    }
    *fd_A = fopen(*dirA_Entities, "w+");
    if(*fd_A == NULL){
        printf("Could not open file dirA_Entities.txt.\n");
        fclose(*fd_diffsA);
        fclose(*fd_diffsB);
        free(*dirA_Differences);
        free(*dirB_Differences);
        free(*dirA_Entities);
        return 1;
    }
    *dirB_Entities = createFilePath(argv0, "dirB_Entities.txt");
    if(*dirB_Entities==NULL){
        printf("Could not create file dirB_Entities.txt.\n");
        fclose(*fd_diffsA);
        fclose(*fd_diffsB);
        fclose(*fd_A);
        free(*dirA_Differences);
        free(*dirB_Differences);
        free(*dirA_Entities);
        return 1;
    }
    *fd_B = fopen(*dirB_Entities, "w+");
    if(*fd_B == NULL){
        printf("Could not open file dirB_Entities.txt.\n");
        fclose(*fd_diffsA);
        fclose(*fd_diffsB);
        fclose(*fd_A);
        free(*dirA_Differences);
        free(*dirB_Differences);
        free(*dirA_Entities);
        free(*dirB_Entities);
        return 1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////     COMPARE  FUNCTIONS       /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* recursive function that compares 2 directories */
int compareDirectories(const char *dirPath1, const char *dirPath2, FILE *fd_diffsA, FILE *fd_diffsB, Metadata *header, FILE* fd_A, FILE* fd_B){
    char dirName1[BUFFER_SIZE];
    char dirName2[BUFFER_SIZE];
    extractEntityName(dirPath1, dirName1);
    extractEntityName(dirPath2, dirName2);

    if((strcmp(dirPath1, dirPath2) == 0) && (strcmp(dirName1, dirName2) == 0)){                 // check if they are the same directory
        // printf("Directories are equal\n");
        traverseSame(dirPath1, fd_A, fd_B);                                                     // traverse the entities of the first dir (they are the same)
        return 0;
    }

    traversedirA(dirPath1, dirPath2, fd_diffsA, fd_diffsB, header, fd_A, fd_B);                 // traverse each entity of dirA
    traversedirB_extras(dirPath1, dirPath2, fd_diffsB, header, fd_B);                           // traverse each extra entity of dirB

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* recursive function that traverses same directories */
int traverseSame(const char *dirPath, FILE *fd_A, FILE *fd_B){
    DIR *dir;                                                                                   // declare a directory
    struct dirent *entity;                                                                      // directory's entity(s)

    if((dir=opendir(dirPath)) == NULL){                                                         // attempt to open the directory
        printf("Failed to open the first directory.\n");
        return 1;
    }

    while((entity=readdir(dir)) != NULL){                                                       // check every entity inside given dir
        if((entity->d_type == DT_DIR)&&((strcmp(entity->d_name, ".") == 0) || (strcmp(entity->d_name, "..") == 0))){
            continue;                                                                           // skip "." and ".." directory entities
        }

        char *relative_path = extendPath(dirPath, entity->d_name);                              // path: dirPath/entity->name

        updateFiles(relative_path, entity->d_ino, "A", NULL, fd_A, NULL);                       // write entity to Entities file
        updateFiles(relative_path, entity->d_ino, "B", NULL, fd_B, NULL);                       // write entity to Entities file

        if(entity->d_type == DT_DIR){                                                           // entity is DIRECTORY
            traverseSame(relative_path, fd_A, fd_B);                                            // recursively traverse dir content
            // check next entity
        }
        free(relative_path);
    }
    closedir(dir);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that traverses a directory and checks if there are differences with another directory */
int traversedirA(const char *dirPath1, const char *dirPath2, FILE *fd_diffsA, FILE *fd_diffsB, Metadata *header, FILE* fd_A, FILE* fd_B){
    DIR *dirA, *dirB;                                                                           // declare a directory
    struct dirent *entity;                                                                      // directory's entity(s)

    if((dirA=opendir(dirPath1)) == NULL){                                                       // attempt to open the directory
        printf("Failed to open the first directory.\n");
        return 1;
    }
    if((dirB=opendir(dirPath2)) == NULL){                                                       // attempt to open the directory
        printf("Failed to open the second directory.\n");
        closedir(dirA);
        return 1;
    }

    while((entity=readdir(dirA)) != NULL){                                          // check if every entity inside dirA exists in dirB, and compare them
        if((entity->d_type == DT_DIR)&&((strcmp(entity->d_name, ".") == 0) || (strcmp(entity->d_name, "..") == 0))){
            continue;                                                                           // skip "." and ".." directory entities
        }

        char *relative_path1 = extendPath(dirPath1, entity->d_name);                            // path: dirPath1/entity->name
        char *relative_path2 = extendPath(dirPath2, entity->d_name);                            // path: dirPath2/entity->name

        updateFiles(relative_path1, entity->d_ino, "A", NULL, fd_A, header);                    // write entity to Entities file

        if(entity->d_type == DT_DIR){                                                           // entity is DIRECTORY
            if(!entityExists(relative_path2)){                                                  // entity with same name but not in same path
                updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);       // write difference(s) to Differences file
                subdir_update(relative_path1, "A", fd_diffsA, NULL, header);                    // write directory's contents to Differences file
            }else{                                                                              // entity with same name and in the same relative path
                struct stat statbuf;                                                            // check if they are the same entity type
                lstat(relative_path2, &statbuf);
                if((statbuf.st_mode & S_IFMT) == S_IFDIR){                                      // both entities are directories
                    updateFiles(relative_path2, statbuf.st_ino, "B", NULL, fd_B, header);       // write entity to Entities file
                    compareDirectories(relative_path1, relative_path2, fd_diffsA, fd_diffsB, header, fd_A, fd_B);   // recursively compare their content
                    // check next entity
                }else{                                                                          // entity in second directory not a directory
                    updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                    updateFiles(relative_path2, statbuf.st_ino, "B", fd_diffsB, NULL, header);
                    updateFiles(relative_path2, statbuf.st_ino, "B", NULL, fd_B, header);       // write entity to Entities file
                }
            }
        }

        if(entity->d_type == DT_REG){                                                           // entity is FILE (or HARDLINK)
            if(!entityExists(relative_path2)){                                                  // entity with same name but not in same path
                updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);       // write difference(s) to Differences file
            }else{                                                                              // entity with same name and in the same relative path
                struct stat statbuf2;                                                           // check if they are the same entity type
                lstat(relative_path2, &statbuf2);
                if((statbuf2.st_mode & S_IFMT) == S_IFREG){                                     // both entities are files with the same name
                    struct stat statbuf1;                                                       // check if they have the same size
                    stat(relative_path1, &statbuf1);
                    if(statbuf1.st_size == statbuf2.st_size){                                   // both files have the same size
                        if(compareFileContent(relative_path1, relative_path2) == 0){            // check if they have the same content
                            updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                            // both files are the same
                            // check next entity
                        }else{                                                                          // files have different content
                            updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                            updateFiles(relative_path2, statbuf2.st_ino, "B", fd_diffsB, NULL, header);
                            updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                        }
                    }else{                                                                          // files have different size
                        updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                        updateFiles(relative_path2, statbuf2.st_ino, "B", fd_diffsB, NULL, header);
                        updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                    }
                }else{                                                                          // entity in second directory not a file
                    updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                    updateFiles(relative_path2, statbuf2.st_ino, "B", fd_diffsB, NULL, header);
                    updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                }
            }
        }

        if(entity->d_type == DT_LNK){                                                           // entity is LINK (symbolic/soft link)
            if(!entityExists(relative_path2)){                                                  // entity with same name but not in same path
                updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);       // write difference(s) to Differences file
            }else{                                                                              // entity with same name and in the same relative path
                struct stat statbuf2;                                                           // check if they are the same entity type
                lstat(relative_path2, &statbuf2);
                if((statbuf2.st_mode & S_IFMT) == S_IFLNK){                                     // both entities are links with the same name
                    ssize_t len1;
                    char linkbuf1[BUFFER_SIZE];
                    if((len1 = readlink(relative_path1, linkbuf1, sizeof(linkbuf1))) != -1){
                        linkbuf1[len1]='\0';                                                    // null-terminate the string
                    }else{
                        printf("Readlink error.\n");
                        return 1;
                    }
                    ssize_t len2;
                    char linkbuf2[BUFFER_SIZE];
                    if((len2 = readlink(relative_path2, linkbuf2, sizeof(linkbuf2))) != -1){
                        linkbuf2[len2]='\0';                                                    // null-terminate the string
                    }else{
                        printf("Readlink error.\n");
                        return 1;
                    }
                    if(strcmp(linkbuf1, linkbuf2) == 0){                                        // check if they link to the same files
                        updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);  // write entity to Entities file
                        // both links links are the same
                        // check next entity
                    }else{                                                                          // links link different files
                        updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                        updateFiles(relative_path2, statbuf2.st_ino, "B", fd_diffsB, NULL, header);
                        updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                    }
                }else{                                                                          // entity in second directory not a link
                    updateFiles(relative_path1, entity->d_ino, "A", fd_diffsA, NULL, header);   // write difference(s) to Differences file
                    updateFiles(relative_path2, statbuf2.st_ino, "B", fd_diffsB, NULL, header);
                    updateFiles(relative_path2, statbuf2.st_ino, "B", NULL, fd_B, header);      // write entity to Entities file
                }
            }
        }
        free(relative_path1);
        free(relative_path2);
    }
    closedir(dirA);
    closedir(dirB);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that traverses a directory and checks if there are entities that were not found in the previous traversal */
int traversedirB_extras(const char *dirPath1, const char *dirPath2, FILE *fd_diffs, Metadata *header, FILE* fd_B){
    DIR *dirA, *dirB;                                                                           // declare a directory
    struct dirent *entity;                                                                      // directory's entity(s)

    if((dirA=opendir(dirPath1)) == NULL){                                                       // attempt to open the directory
        printf("Failed to open the first directory.\n");
        return 1;
    }
    if((dirB=opendir(dirPath2)) == NULL){                                                       // attempt to open the directory
        printf("Failed to open the second directory.\n");
        closedir(dirA);
        return 1;
    }

    while((entity=readdir(dirB))!=NULL){                                            // check if every entity inside dirB exists in dirA, and compare them
        if((entity->d_type == DT_DIR)&&((strcmp(entity->d_name, ".") == 0) || (strcmp(entity->d_name, "..") == 0))){
            continue;                                                                           // skip "." and ".." directory entities
        }

        char *relative_path1 = extendPath(dirPath1, entity->d_name);                            // path: dirPath1/entity->name
        char *relative_path2 = extendPath(dirPath2, entity->d_name);                            // path: dirPath2/entity->name

        // updateFiles(relative_path2, entity->d_ino, "B", NULL, fd_B, header);                    // write entity to Entities file

        if(entity->d_type == DT_DIR){                                                           // entity is DIRECTORY
            if(!entityExists(relative_path1)){                                                  // entity with same name but not in same path
                updateFiles(relative_path2, entity->d_ino, "B", fd_diffs, fd_B, header);        // write contents to Differences and Entities file
                subdir_update(relative_path2, "B", fd_diffs, NULL, header);                     // write directory's contents to Differences file
                subdir_update(relative_path2, "B", NULL, fd_B, header);                         // write directory's contents to Entities file
            }else{
                // an entity with the same name exists in the same relative path of the first directory, which means we have already checked it
            }
        }

        if(entity->d_type == DT_REG){                                                           // entity is FILE (or HARDLINK)
            if(!entityExists(relative_path1)){                                                  // entity with same name but not in same path
                updateFiles(relative_path2, entity->d_ino, "B", fd_diffs, fd_B, header);        // write contents to Differences and Entities file
            }else{
                // an entity with the same name exists in the same relative path of the first directory, which means we have already checked it
            }
        }

        if (entity->d_type == DT_LNK) {                                                         // entity is LINK (symbolic/soft link)
            if(!entityExists(relative_path1)){                                                  // entity with same name but not in same path
                updateFiles(relative_path2, entity->d_ino, "B", fd_diffs, fd_B, header);        // write contents to Differences and Entities file
            }else{
                // an entity with the same name exists in the same relative path of the first directory, which means we have already checked it
            }
        }
        free(relative_path1);
        free(relative_path2);
    }
    closedir(dirA);
    closedir(dirB);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* recursive function that traverses a directory and stores its content in a given file */
int subdir_update(const char *dirPath, char *instance, FILE *fd_diffs, FILE *fd_log, Metadata *header){
    DIR* subdir;
    struct dirent *entity;                                                                      // directory's entity(s)

    if((subdir=opendir(dirPath)) == NULL){                                                      // attempt to open the directory
        printf("Failed to open the first directory.\n");
        return 1;
    }

    while((entity=readdir(subdir))!=NULL){
        if((entity->d_type == DT_DIR)&&((strcmp(entity->d_name, ".") == 0) || (strcmp(entity->d_name, "..") == 0))){
            continue;                                                                           // skip "." and ".." directory entities
        }

        char *relative_path = extendPath(dirPath, entity->d_name);                              // path: dirPath/entity->name

        if(entity->d_type == DT_DIR){                                                           // entity is DIRECTORY
            updateFiles(relative_path, entity->d_ino, instance, fd_diffs, fd_log, header);      // write difference(s) to Differences file
            subdir_update(relative_path, instance, fd_diffs, fd_log, header);                   // write directory's contents to Differences file
        }else{                                                                                  // entity is FILE (or HARDLINK) or LINK (symbolic/soft link)
            updateFiles(relative_path, entity->d_ino, instance, fd_diffs, fd_log, header);      // write difference(s) to Differences file
        }
        free(relative_path);
    }
    closedir(subdir);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that updates our assissting files' contents */
int updateFiles(char *path, ino_t ino, char *instance, FILE *dif, FILE *log, Metadata *header){
    if(dif!=NULL){                                                                              // Differences file descriptor is given
        if(!strcmp(instance, "A")){                                                             // concerns the first directory
            header->dirA++;                                                                     // increase the header's value
            struct stat statbuf;
            lstat(path, &statbuf);                                                              // check entity's type
            if(S_ISREG(statbuf.st_mode)){                                                       // entity is FILE (or HARDLINK)
                fprintf(dif, "%s %s %lu %-20lu\n", "f", path, ino, (ino_t)0);
            }else if(S_ISDIR(statbuf.st_mode)){                                                 // entity is DIRECTORY
                fprintf(dif, "%s %s %lu %-20lu\n", "d", path, ino, (ino_t)0);
            }else if(S_ISLNK(statbuf.st_mode)){                                                 // entity is LINK (symbolic/soft link)
                fprintf(dif, "%s %s %lu %-20lu\n", "l", path, ino, (ino_t)0);
            }
        }else if(!strcmp(instance, "B")){                                                       // concerns the second directory
            header->dirB++;                                                                     // increase the header's value
            struct stat statbuf;
            lstat(path, &statbuf);                                                              // check entity's type
            if(S_ISREG(statbuf.st_mode)){                                                       // entity is FILE (or HARDLINK)
                fprintf(dif, "%s %s %lu %-20lu\n", "f", path, ino, (ino_t)0);
            }else if(S_ISDIR(statbuf.st_mode)){                                                 // entity is DIRECTORY
                fprintf(dif, "%s %s %lu %-20lu\n", "d", path, ino, (ino_t)0);
            }else if(S_ISLNK(statbuf.st_mode)){                                                 // entity is LINK (symbolic/soft link)
                fprintf(dif, "%s %s %lu %-20lu\n", "l", path, ino, (ino_t)0);
            }
        }else{
            printf("Unrecognised character.\n");
            return 1;
        }
        fflush(dif);
    }

    if(log!=NULL){                                                                              // Entities file descriptor is given
        struct stat statbuf;
        lstat(path, &statbuf);                                                                  // check entity's type
        if(S_ISREG(statbuf.st_mode)){                                                           // entity is FILE (or HARDLINK)
            fprintf(log, "%s %s %lu %-20lu\n", "f", path, ino, (ino_t)0);
        }else if(S_ISDIR(statbuf.st_mode)){                                                     // entity is DIRECTORY
            fprintf(log, "%s %s %lu %-20lu\n", "d", path, ino, (ino_t)0);
        }else if(S_ISLNK(statbuf.st_mode)){                                                     // entity is LINK (symbolic/soft link)
            fprintf(log, "%s %s %lu %-20lu\n", "l", path, ino, (ino_t)0);
        }
        fflush(log);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that compares the content of 2 files (checking their bytes) */
int compareFileContent(const char *file1, const char *file2){
    FILE *fd1, *fd2;                                                                            // files' file descriptors
    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];                                            // files' buffers
    size_t bytesRead1, bytesRead2;                                                              // files' read bytes

    fd1=fopen(file1, "rb");                                                                     // attempt to open the first file
    if(fd1==NULL){
        printf("Couldn't open the first file.\n");
        return -99;
    }

    fd2=fopen(file2, "rb");                                                                     // attempt to open the second file
    if(fd2==NULL){
        printf("Couldn't open the second file.\n");
        fclose(fd1);
        return -99;
    }

    do{                                                                                         // try at least once
        bytesRead1=fread(buffer1, 1, sizeof(buffer1), fd1);                                     // attempt to read a portion of the file
        bytesRead2=fread(buffer2, 1, sizeof(buffer2), fd2);

        if((bytesRead1!=bytesRead2)||(memcmp(buffer1, buffer2, bytesRead1)!=0)){                // files are different
            // printf("The files are different.\n");
            fclose(fd1);
            fclose(fd2);
            return 1;
        }

    }while(bytesRead1>0);                                                                       // try until the first file hasn't got any more bytes

    if(fread(buffer2, 1, sizeof(buffer2), fd2)>0){                                              // check for remaining bytes in the second file
        // printf("The files are different.\n");
        fclose(fd1);
        fclose(fd2);
        return 1;
    }                                                                                           // else the files are identical

    // printf("The files are the same.\n");
    fclose(fd1);
    fclose(fd2);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that checks whether an FS entity (file, directory, link) exists */
int entityExists(const char *fs_entity){
    struct stat buffer;                                                                         // if exists returns 0, so 0 == 0 = true => returns 1
    return(lstat(fs_entity, &buffer) == 0);                                                     // if not exists returns -1, so -1 == 0 = false => returns 0
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that extracts the entity-name from a given full path */
void extractEntityName(const char* fullPath, char* entityName){
    char* last_slash=strrchr(fullPath, '/');                                                    // find the last occurrence of '/'

    if(last_slash!=NULL){
        strcpy(entityName, last_slash+1);                                                       // copy the substring after the last '/'
    }else{
        strcpy(entityName, fullPath);                                                           // no '/' found, copy the full string
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that extends the current path with a new entity (path/entity_name) */
char *extendPath(const char *path, const char *extension){
    char *extended_path=malloc(strlen(path)+2);                                             // allocate memory for the path (+2 for '/' and '\0' characters)
    strcpy(extended_path, path);
    strcat(extended_path, "/");

    extended_path=realloc(extended_path, strlen(extended_path)+(strlen(extension)+1));          // reallocate memory to fit the current entity's path
    strcat(extended_path, extension);

    return extended_path;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////       MERGE  FUNCTIONS       /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that merges two given directories */
int mergeDirectories(FILE *fd_core, const char *fd_core_name, FILE *fd2_diffs, const char *fd_diffs_name, 
    const char *dirC, FILE *fd2_entities, const char *core_name, const char *diff_name){
    // printf("\nDirectory for final merge is: %s\n\n", dirC);

    // Copy the entities of one directory to the new one
    copyEntities(fd_core, fd_core_name, dirC, NULL, NULL, core_name);                           // pass NULL as this isn't the differences file

    // Copy the differences of the other directory to the new one
    copyEntities(fd2_diffs, fd_diffs_name, dirC, fd2_entities, core_name, diff_name);           // pass true as this is the differences file

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that copies the contents of a given directory to a new one */
int copyEntities(FILE *fd, const char *fd_name, const char *dirC, FILE *fd2, const char *core_name, const char *name){
    char current_type[2];                                                                       // current entity type indicator
    char current_path[1024];                                                                    // current path size
    ino_t current_ino, current_temp, new_inode;                                                 // inodes

    if(fd2 != NULL){                                                                            // if it is a differences file
        int header_int;
        fscanf(fd, "%d", &header_int);                                                          // ignore the first integer (of the header)
        if(header_int==0){
            return 0;
        }
    }

    while(fscanf(fd, "%s %s %lu %lu", current_type, current_path, &current_ino, &current_temp) != EOF){
        if(strcmp(current_type, "f") == 0){                                                     // Entity is FILE or HARD LINK
            int newfd = open(fd_name, O_RDONLY);                                                // use of open() to create a 2nd file desc into the file
            FILE *newfdstream = fdopen(newfd, "r");                                             // use of fdopen() to access this 2nd fd as a FILE*
            char type[2];                                                                       // temp entity type indicator
            char path[1024];                                                                    // temp path size
            ino_t ino, temp;                                                                    // temp inodes
            bool flag = false;                                                                  // checks if the inode was found in the file
            if(fd2 != NULL){                                                                    // we traverse the differences file
                int header_int;
                fscanf(newfdstream, "%d", &header_int);                                         // ignore the first integer (of the header)
            }
            while((fscanf(newfdstream, "%s %s %lu %20lu\n", type, path, &ino, &temp) != EOF) && (flag == false)){
                if((ino == current_ino) && (temp == 0)){                                        // entity is FILE (our file's temp inode is 0)
                    if(fd2 != NULL){
                        new_inode = check_for_hard_link_on_second_file(fd2, current_path, current_ino, dirC, core_name, name);
                    }else{
                        new_inode = copyFile(path, dirC, core_name, name);                      // get the inode in the new directory
                    }
                    if(new_inode){                                                              // update our file with the inode in the new directory
                        fseek(fd, -1, SEEK_CUR);                                                // seek back at the beginning of the "0"
                        fprintf(fd, "%lu", new_inode);                                          // and replace it/write the new inode
                    }
                    flag = true;                                                                // inode found
                }else if((ino == current_ino) && (temp != 0)){                                  // entity is HARD LINK (our file's temp inode is not 0)
                    char *linkpath = changeParentPath(current_path, dirC, name);                // create the path of the new hard link inside dirC
                    char *filepath = changeParentPath(path, dirC, name);                        // create/find the path of the file inside dirC

                    link(filepath, linkpath);                                                   // create the hard link
                    struct stat linkStat;
                    lstat(linkpath, &linkStat);                                                 // get new inode
                    new_inode = linkStat.st_ino;

                    if(new_inode){                                                              // update our file with the inode in the new directory
                        fseek(fd, -1, SEEK_CUR);                                                // seek back at the beginning of the "0"
                        fprintf(fd, "%lu", new_inode);                                          // and replace it/write the new inode
                    }

                    free(linkpath);
                    free(filepath);
                    flag = true;                                                                // inode found
                }
            }
            fclose(newfdstream);
            close(newfd);
        }else if(strcmp(current_type, "d") == 0){                                               // Entity is DIRECTORY
            char *subdirpath = changeParentPath(current_path, dirC, name);

            struct stat sourceStat;
            stat(current_path, &sourceStat);                                                    // get source permissions
            mkdir(subdirpath, sourceStat.st_mode);                                              // set destination permissions to match source
            
            struct stat statbuf;
            stat(subdirpath, &statbuf);
            new_inode = statbuf.st_ino;                                                         // get the inode in the new directory
            if(new_inode){                                                                      // update our file with the inode in the new directory
                fseek(fd, -1, SEEK_CUR);                                                        // seek back at the beginning of the "0"
                fprintf(fd, "%lu", new_inode);                                                  // and replace it/write the new inode
            }

            free(subdirpath);
        } 
    }

    rewind(fd);                                                                                 // rewind the file desctiptor to check for soft links
    if(fd2 != NULL){                                                                            // if it is a differences file
        int header_int;
        fscanf(fd, "%d", &header_int);                                                          // ignore the first integer (of the header)
    }
    while(fscanf(fd, "%s %s %lu %lu", current_type, current_path, &current_ino, &current_temp) != EOF){
        if(strcmp(current_type, "l") == 0){                                                     // Entity is SOFT LINK
            char linkedfile[1024];
            ssize_t len;
            if((len = readlink(current_path, linkedfile, sizeof(linkedfile)-1)) != -1){
                linkedfile[len] = '\0';                                                         // null-terminate the string
            }else{
                printf("Readlink error.\n");
                return 1;
            }
            char *softlinkpath = changeParentPath(current_path, dirC, name);                    // create the path where the soft link will be stored
            if(access(softlinkpath, F_OK)!=-1){                                                 // check if the same link exists in the same path
                struct stat sourceStat;
                lstat(current_path, &sourceStat);
                char *corepath=changeParentPath(current_path, core_name, name);
                struct stat coreStat;
                lstat(corepath, &coreStat);                                                     // get destination last modified date
                free(corepath);
                if(coreStat.st_mtime>=sourceStat.st_mtime){                                     // keep the file with the most recent last modified date
                    free(softlinkpath);
                    continue;
                }
            }
            symlink(linkedfile, softlinkpath);                                                  // create the soft link
            struct stat linkStat;
            lstat(softlinkpath, &linkStat);
            new_inode = linkStat.st_ino;                                                        // get new inode

            if(new_inode){                                                                      // update our file with the inode in the new directory
                fseek(fd, -1, SEEK_CUR);                                                        // seek back at the beginning of the "0"
                fprintf(fd, "%lu", new_inode);                                                  // and replace it/write the new inode
            }
            
            free(softlinkpath);
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that checks whether a file in our differences.txt is a hardlink to an already copied file */
ino_t check_for_hard_link_on_second_file(FILE *fd2, char *current_path, ino_t current_ino, const char *dirC, const char *core_name, const char *name){
    ino_t new_inode = 0;

    struct stat fileStat;
    lstat(current_path, &fileStat);                                                             // get source permissions
    if(fileStat.st_nlink > 1){                                                                  // check if the file has more than 1 hard links
        char extratype[2];                                                                      // temp entity type indicator
        char extrapath[1024];                                                                   // temp path size
        ino_t extraino, extratemp;                                                              // temp inodes
        bool extraflag = false;                                                                 // checks if the inode was found in the file
        while((fscanf(fd2, "%s %s %lu %20lu\n", extratype, extrapath, &extraino, &extratemp) != EOF) && (extraflag == false)){
            if((extraino == current_ino) && (strcmp(extrapath, current_path) != 0)){            // check if there is a different file with the same inode
                extraflag = true;                                                               // same inode found in a different file
                break;
            }
        }
        if(extraflag == true){
            char *linkpath = changeParentPath(current_path, dirC, name);                        // create the path of the new hard link inside dirC
            char *filepath = changeParentPath(extrapath, dirC, name);                           // create/find the path of the file inside dirC
            if(entityExists(filepath)){                                                         // we can create a hard link to an already copied inode
                link(filepath, linkpath);                                                       // create the hard link
                struct stat linkStat;
                lstat(linkpath, &linkStat);                                                     // get new inode
                new_inode = linkStat.st_ino;
            }else{                                                                              // there is no instance of this inode
                new_inode = copyFile(current_path, dirC, core_name, name);                      // get new inode
            }
            free(linkpath);
            free(filepath);
        }else{                                                                                  // there is no other file with the same inode
            new_inode = copyFile(current_path, dirC, core_name, name);                          // get new inode
        }
        rewind(fd2);
    }else{                                                                                      // there are no hard links to this file
        new_inode = copyFile(current_path, dirC, core_name, name);                              // get new inode
    }
    return new_inode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that creates a copy of the given path to the given directory */
ino_t copyFile(char *path, const char *dirC, const char *core_name, const char *oldparent){
    char *destinationPath=changeParentPath(path, dirC, oldparent);

    FILE* sourceFile = fopen(path, "rb");                                                       // attempt to open the source file for reading
    if(sourceFile == NULL){
        printf("Couldn't open the source file.\n");
        free(destinationPath);
        return 0;
    }

    struct stat sourceStat;
    if(lstat(path, &sourceStat) != 0){                                                          // get source last modified date & permissions
        printf("Error getting source file permissions.\n");
        fclose(sourceFile);
        free(destinationPath);
        return 0;
    }

    if(access(destinationPath, F_OK)!=-1){                                                      // check if the same file exists in the same path
        char *corepath=changeParentPath(path, core_name, oldparent);
        struct stat coreStat;
        if(lstat(corepath, &coreStat) != 0){                                                    // get destination last modified date
            printf("Error getting source file permissions.\n");
            free(corepath);
            fclose(sourceFile);
            free(destinationPath);
            return 0;
        }
        free(corepath);
        if(coreStat.st_mtime>=sourceStat.st_mtime){                                             // keep the file with the most recent last modified date
            fclose(sourceFile);
            free(destinationPath);
            return 0;
        }
    }

    FILE* destinationFile = fopen(destinationPath, "wb");                                       // attempt to open the destination file for writing
    if(destinationFile == NULL){
        printf("Couldn't open the destination file.\n");
        fclose(sourceFile);
        free(destinationPath);
        return 0;
    }

    char buffer[BUFFER_SIZE];                                                                   // file's buffer
    size_t bytesRead;                                                                           // file's read bytes
    while((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0){                      // copy file contents
        fwrite(buffer, 1, bytesRead, destinationFile);
    }

    if(chmod(destinationPath, sourceStat.st_mode) != 0){                                        // set destination permissions to match source
        printf("Error setting destination file permissions.\n");
        fclose(sourceFile);
        fclose(destinationFile);
        free(destinationPath);
        return 0;
    }

    // printf("File '%s' copied to '%s'\n", path, destinationPath);

    struct stat statbuf;
    stat(destinationPath, &statbuf);

    fclose(sourceFile);
    fclose(destinationFile);
    free(destinationPath);

    return statbuf.st_ino;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* function that changes the parent path of a full path, with a new parent path (e.g. dirA/... to dirC/...) */
char *changeParentPath(const char *fullpath, const char *newparent, const char *oldparent){
    char *rest_path = strstr(fullpath, oldparent);                                              // find the first occurrence of the previous path
    if(rest_path == NULL){
        printf("Invalid path inside changeParentPath: %s\n", fullpath);
        return NULL;
    }

    size_t oldparent_len = strlen(oldparent);
    size_t rest_path_len = strlen(rest_path);
    unsigned long newpathsize = strlen(newparent) + 1 + (rest_path_len - oldparent_len) + 1;    // size of the new path: newparent + '/' + rest_path + '\0'

    char *newpath = (char *)malloc(newpathsize);                                                // allocate memory for the new path
    if(newpath == NULL){
        printf("Memory allocation failed.\n");
        return NULL;
    }

    strncpy(newpath, fullpath, strlen(fullpath) - strlen(rest_path));                           // copy the part of the path before oldparent
    strcpy(newpath + (rest_path - fullpath), newparent);                                        // append the new parent
    strcat(newpath, rest_path + oldparent_len);                                                 // append the remaining part of the path after oldparent

    return newpath;
}