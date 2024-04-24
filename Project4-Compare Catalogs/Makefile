# Makefile for cmpcat program

# To BUILD the program, in tty: 
# 1) Type "make", for the separate compilation to take place

# To RUN the program, in tty:
# Type "./cmpcat -d dirA dirB [-s dirC]

# To CLEAN all the .ο, executable, .txt files and the "tests" directory, created after build/make, in tty:
# 2) Type "make clean"

# To do a fast git add-commit-push (ignore this-used while working as a team on the project), in tty:
# 3) Type "make git"


#EXAMPLES/TESTS EXECUTION: 

# To untar the tests.tar, in order to have access to directories examples, in tty:
# 4) Type "tar -xvf tests.tar"

# Define the variable for the subdirectory to be used for the example:
# (this can be one of the subdirectories inside the "tests" directory that was untared):
EXAMPLE_DIR = GivenExample


# Run a fast example of the program (first define your desired EXAMPLE_DIR to run on), in tty:
# 5) Type "make run"

# Run a fast example of the program using valgrind (first define your desired EXAMPLE_DIR to run on), in tty:
# 6) Type "make valgrind"


# 1) Build all executables together:
all: cmpcat

cmpcat: cmpcat.o functions.o
	gcc -Wall -Wextra -g3 -o cmpcat cmpcat.o functions.o

cmpcat.o: cmpcat.c interface.h
	gcc -Wall -Wextra -g3 -c cmpcat.c -o cmpcat.o

functions.o: functions.c interface.h
	gcc -Wall -Wextra -g3 -c functions.c -o functions.o

# 2) Clean executable, .o, .txt files and "tests" directory:
clean:
	rm -rf cmpcat cmpcat.o functions functions.o dirA_Differences.txt dirB_Differences.txt dirA_Entities.txt dirB_Entities.txt tests

# 3) Fast git add-commit-push (used by us while working on the project):
git:
	git add -A
	git commit -m "fast update"
	git push

# 4) Untar the tests.tar, in order to have access to directories examples:
tests:
	tar -xvf tests.tar

# 5) Fast example run with "-d tests/$(EXAMPLE_DIR)/dirA tests/$(EXAMPLE_DIR)/dirB -s tests/$(EXAMPLE_DIR)/dirC":
run:
	rm -rf tests/$(EXAMPLE_DIR)/dirC
	./cmpcat -d tests/$(EXAMPLE_DIR)/dirA tests/$(EXAMPLE_DIR)/dirB -s tests/$(EXAMPLE_DIR)/dirC

# 6) Fast example run using valgrind with "-d tests/$(EXAMPLE_DIR)/dirA tests/$(EXAMPLE_DIR)/dirB -s tests/$(EXAMPLE_DIR)/dirC":
valgrind:
	rm -rf tests/$(EXAMPLE_DIR)/dirC
	valgrind -s --leak-check=full --track-origins=yes ./cmpcat -d tests/$(EXAMPLE_DIR)/dirA tests/$(EXAMPLE_DIR)/dirB -s tests/$(EXAMPLE_DIR)/dirC