# Makefile for readers-writers program

# To BUILD the program, in tty: 
# 1) Type "make", for the separate compilation to take place

# To RUN the program, in tty:
# 2) Type (in this order):
#	2.1) "./constructor -s <shmname>"
#	2.2.1) "./reader -f <filename> -l <recid1[ recid2]> -d <time> -s <shmname>"
#	simultaneously (and multiple times by different terminals) with:
#	2.2.2) "./writer -f <filename> -l <recid> -v <value> -d <time> -s <shmname>"
#	2.3) "./results -s <shmname>"
#	2.4) "./destructor -s <shmname>"

# To CLEAN all the .ο and executable files, created after build/make, in tty:
# 3) Type "make clean"

# To have a fast lookup to the list of all named semaphores and shared memory segments active in the (Linux) system, in tty:
# 4) Type "make ls"

# 1) Build all executables together
all: constructor reader writer destructor results

constructor: constructor.o
	gcc -Wall -Wextra -g3 -o constructor constructor.o -lpthread -lrt 

constructor.o: constructor.c structs.h
	gcc -Wall -Wextra -g3 -c constructor.c -o constructor.o

reader: reader.o
	gcc -Wall -Wextra -g3 -o reader reader.o -lpthread -lrt

reader.o: reader.c structs.h
	gcc -Wall -Wextra -g3 -c reader.c -o reader.o

writer: writer.o
	gcc -Wall -Wextra -g3 -o writer writer.o -lpthread -lrt

writer.o: writer.c structs.h
	gcc -Wall -Wextra -g3 -c writer.c -o writer.o

results: results.o
	gcc -Wall -Wextra -g3 -o results results.o -lpthread -lrt

results.o: results.c structs.h
	gcc -Wall -Wextra -g3 -c results.c -o results.o

destructor: destructor.o
	gcc -Wall -Wextra -g3 -o destructor destructor.o -lpthread -lrt

destructor.o: destructor.c structs.h
	gcc -Wall -Wextra -g3 -c destructor.c -o destructor.o

# 3) Clean .o and executable files
clean:
	rm -f constructor constructor.o reader reader.o writer writer.o results results.o destructor destructor.o

# 4) Fast lookup to list of all named semaphores and shared memory segments in the (Linux) system
ls:
	ls /dev/shm