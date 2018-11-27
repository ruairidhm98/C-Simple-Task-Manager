CC=clang
CFLAGS=-Wall -Werror -std=c99 -g
PTHREAD=-lpthread
EXE=system
OBJ=queue.o task_system.o

# Creates executable file from object code
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $(PTHREAD) $^ -o $@ 

# Compile C source code into object code
$(OBJ) : task_system.c queue.c
	$(CC) -c $^

# Deletes object code and executable
clean:
	rm *.o
	rm $(EXE)