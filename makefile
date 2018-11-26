CC=clang
CFLAGS=-Wall -Werror -std=c99 -g
EXE=system
OBJ=queue.o

# Creates executable file from object code
$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 

# Compile C source code into object code
%.o : %.c
	$(CC) -c $^

# Deletes object code and executable
clean:
	rm *.o
	rm $(EXE)