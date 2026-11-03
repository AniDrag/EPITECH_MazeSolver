NAME = solver
CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c11 -O2
CPPFLAGS = -Iinclude
OBJ = bin/main.o bin/maze_parser.o

#Helps Build all the dependancies
all: $(NAME)
#Links all object in to the name! 
$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

# Compile main.c if main.o is missing or main.c is newer.
bin/main.o: bin/main.c include/maze_parser.h include/utility.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
# same as above
bin/maze_parser.o: bin/maze_parser.c include/maze_parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)
# instead of re: fcelan all this one makes clean run before make, \
and finishes before make starts.
re: fclean 
	$(MAKE) all
# Specifies all, clean, fclean and re are actions.
.PHONY: all clean fclean re

#NOTES \
#$() a way to tell the compiler to substitute a val aka "THis is a variable"!\
CFLAGS:Options passed to the compiler.\
-g for debug information, \
-Wall -Wextra to enable warnings, \
-Werror to treat warnings as errors, \
and -std=c11 to select the C11 standard.\
SRC	A list of C source files.\
RM - sucseeds clenup even if no file exits\
