# Makefile for mysort program

# To BUILD the program, in tty: 
# 1) Type "make", for the separate compilation to take place

# To RUN the program, in tty:
# 2) Type "./mysort -i <DataFile> -k <NumofChildren> -e1 <sorting1> -e2 <sorting2>", to run the program
# 
#	To run some automated executions as examples, in tty:
# 	2.1) Type "make run", to run an example with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort
#  	or
# 	2.2) Type "make time", to run an example & display timing info with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort
#  	or
# 	2.3) Type "make valgrind", to run an example using valgrind with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort
#  	or
# 	2.4) Type "make trace", to run an example using valgrind & trace & track children with <DataFile> = voters50.bin, <NumofChildren> = 3, <sorting1> = mybubblesort, <sorting2> = myselectsort
 
#Notes:
# a) <DataFile> is a file having the elements of records, <NumofChildren> is an integer - based on which number of children will be created, 
#    <sorting1> is an executable program for sorting, <sorting2> is another executable program for sorting
# b) The "-i <DataFile>", "-k <NumofChildren>", "-e1 <sorting1>", "-e2 <sorting2>" arguments can be given with any order among them (as groups of two, flag + argument)
# c) The CORRECT WAY TO RUN the program is 2) (the 2.1), 2.2), 2.3), 2.4) are just examples for testing with specific DataFile(s), arguments and tools).

# To CLEAN all the .o files as well as the executable files, created after build/make, in tty:
# 3) Type "make clean"

# define color codes (for using them in @echo)
GREEN = \033[0;32m
BRIGHT_GREEN = \033[1;32m
YELLOW = \033[0;33m
MAGENTA = \033[0;35m
CYAN = \033[0;36m
RESET = \033[0m

# 1) Build All executables together
all: mysort mybubblesort myselectsort

# build mysort (executable program)
mysort: mysort.o mysort_functions.o
	gcc -Wall -Wextra -g3 -o mysort mysort.o mysort_functions.o -lm

# Build mysort.c
mysort.o: mysort_prototypes.h
	@echo "$(GREEN)>Building $(YELLOW)mysort$(GREEN) (main program):$(RESET)"
	gcc -Wall -Wextra -g3 -c mysort.c -o mysort.o

# build mysort_functions.c
mysort_functions.o: mysort_prototypes.h
	gcc -Wall -Wextra -g3 -c mysort_functions.c -o mysort_functions.o

# build mybubblesort (executable program)
mybubblesort: mybubblesort.o mybubblesort_function.o
	gcc -Wall -Wextra -g3 -o mybubblesort mybubblesort.o mybubblesort_function.o

# Build mybubblesort.c
mybubblesort.o: mybubblesort_prototype.h
	@echo "$(GREEN)>Building $(MAGENTA)mybubblesort$(GREEN) (sorting program):$(RESET)"
	gcc -Wall -Wextra -g3 -c mybubblesort.c -o mybubblesort.o

# build mybubblesort_function.c
mybubblesort_function.o: mybubblesort_prototype.h
	gcc -Wall -Wextra -g3 -c mybubblesort_function.c -o mybubblesort_function.o

# build myselectsort (executable program)
myselectsort: myselectsort.o myselectsort_function.o
	gcc -Wall -Wextra -g3 -o myselectsort myselectsort.o myselectsort_function.o

# Build myselectsort.c
myselectsort.o: myselectsort_prototype.h
	@echo "$(GREEN)>Building $(CYAN)myselectsort$(GREEN) (sorting program):$(RESET)"
	gcc -Wall -Wextra -g3 -c myselectsort.c -o myselectsort.o

# build myselectsort_function.c
myselectsort_function.o: myselectsort_prototype.h
	gcc -Wall -Wextra -g3 -c myselectsort_function.c -o myselectsort_function.o

# 2.1) Example execution (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)
run:
	@echo "$(GREEN)>Example $(BRIGHT_GREEN)execution$(GREEN) (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)$(RESET)"
	./mysort -i voters50000.bin -k 4 -e1 mybubblesort -e2 myselectsort

# 2.2) Example execution with time (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)
time:
	@echo "$(GREEN)>Example $(BRIGHT_GREEN)execution with time$(GREEN) (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)$(RESET)"
	@/usr/bin/time -f "\nTiming Information:\n real\t%Es\n user\t%Us\n sys\t%Ss" ./mysort -i voters50000.bin -k 4 -e1 mybubblesort -e2 myselectsort

# 2.3) Example execution using valgrind (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)
valgrind:
	@echo "$(GREEN)>Example $(BRIGHT_GREEN)execution using valgrind$(GREEN) (with <DataFile> = voters50000.bin, <NumofChildren> = 4, <sorting1> = mybubblesort, <sorting2> = myselectsort)$(RESET)"
	valgrind ./mysort -i voters50000.bin -k 4 -e1 mybubblesort -e2 myselectsort

# 2.4) Example execution using valgrind & trace & track children (with <DataFile> = voters50.bin, <NumofChildren> = 3, <sorting1> = mybubblesort, <sorting2> = myselectsort)
trace:
	@echo "$(GREEN)>Example $(BRIGHT_GREEN)execution using valgrind & trace & track children$(GREEN) (with <DataFile> = voters50.bin, <NumofChildren> = 3, <sorting1> = mybubblesort, <sorting2> = myselectsort)$(RESET)"
	valgrind --trace-children=yes --track-origins=yes -s ./mysort -i voters50.bin -k 3 -e1 mybubblesort -e2 myselectsort

# 3) Clean .o and executable files
clean:
	rm -f mysort mysort.o mysort_functions.o mybubblesort mybubblesort.o mybubblesort_function.o myselectsort myselectsort.o myselectsort_function.o