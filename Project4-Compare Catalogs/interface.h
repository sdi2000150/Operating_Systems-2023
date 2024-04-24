/* interface.h - Structs and Functions declaration */

#ifndef CODE_INTERFACE_H
#define CODE_INTERFACE_H

#define BUFFER_SIZE 4096

/* output files metadata */
typedef struct {
    int dirA;                                                                                   // number of differences in the first directory
    int dirB;                                                                                   // number of differences in the second directory
} Metadata;

/* functions */
////////////////////////////////////////////////////////////////        INITIALIZATION        /////////////////////////////////////////////////////////////////
char* createFilePath(const char *, const char *);
int initializeFilesAndDescriptors(char *, FILE **, FILE **, FILE **, FILE **, char **, char **, char **, char **);

////////////////////////////////////////////////////////////////     COMPARE  FUNCTIONS       /////////////////////////////////////////////////////////////////
int compareDirectories(const char *, const char *, FILE *, FILE *, Metadata *, FILE *, FILE*);
int traverseSame(const char *, FILE *, FILE *);
int traversedirA(const char *, const char *, FILE *, FILE *, Metadata *, FILE*, FILE*);
int traversedirB_extras(const char *, const char *, FILE *, Metadata *, FILE*);
int subdir_update(const char *, char *, FILE *, FILE *, Metadata *);
int updateFiles(char *, ino_t, char *, FILE *, FILE *, Metadata *);
int compareFileContent(const char *, const char *);
int entityExists(const char *);
void extractEntityName(const char *, char *);
char *extendPath(const char *, const char *);

////////////////////////////////////////////////////////////////       MERGE  FUNCTIONS       /////////////////////////////////////////////////////////////////
int mergeDirectories(FILE *, const char *, FILE *, const char *, const char *, FILE *, const char *, const char *);
int copyEntities(FILE *, const char *, const char *, FILE *, const char *, const char *);
ino_t check_for_hard_link_on_second_file(FILE *, char *, ino_t, const char *, const char *, const char *);
ino_t copyFile(char *, const char *, const char *, const char *);
char *changeParentPath(const char *, const char *, const char *);

#endif //CODE_INTERFACE_H