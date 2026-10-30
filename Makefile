NAME = alexanderIsHot
CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c11

all: $(NAME)

$(NAME): bin/main.o
	$(CC) $(CFLAGS) bin/main.o -o $(NAME)

bin/main.o: bin/main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) bin/main.o

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re