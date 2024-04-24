# Makefile for mvote program

# To BUILD the program, in tty: 
# 1) Type "make", for the separate compilation to take place

# To RUN the program, in tty:
# 2.1) Type "./mvote -f <registeredvoters> -b <bucketentries>", to run the program
#  or 
# 2.2) Type "valgrind ./mvote -f <registeredvoters> -b <bucketentries>", to run the program using valgrind
#  or
# 2.3) Type "make run", to run the program with <registeredvoters> = voters50.csv, and <bucketentries> = 2
#  or
# 2.4) Type "make valgrind", to run the program using valgrind with <registeredvoters> = voters50.csv, and <bucketentries> = 2
# Notes:
# (<registeredvoters> is a file with each line having the elements of a voter, <bucketentries> is an integer - max number of keys a bucket has)
# (the "-f <registeredvoters>" and "-b <bucketentries>" arguments can be given with any order between them)

# To CLEAN all the .o files as well as the executable file, created after build/make, in tty:
# 3) Type "make clean"

# build mvote (executable program)
mvote: mvote.o functions.o implementation.o
	gcc -o mvote mvote.o functions.o implementation.o

# build mvote.c
mvote.o: structs.h interface.h functions.h mvote.c
	gcc -c mvote.c -o mvote.o

# build functions.c
functions.o: structs.h interface.h functions.h functions.c
	gcc -c functions.c -o functions.o

# build implementation.c
implementation.o: structs.h interface.h implementation.c
	gcc -c implementation.c -o implementation.o

# example execution (with <registeredvoters> = voters50.csv, and <bucketentries> = 2)
run:
	@echo "example execution (with <registeredvoters> = voters50.csv, and <bucketentries> = 2)"
	./mvote -f voters50.csv -b 2

# example execution using valgrind (with <registeredvoters> = voters50.csv, and <bucketentries> = 2)
valgrind:
	@echo "example execution using valgrind (with <registeredvoters> = voters50.csv, and <bucketentries> = 2)"
	valgrind ./mvote -f voters50.csv -b 2

# clean
clean:
	rm -f mvote mvote.o functions.o implementation.o