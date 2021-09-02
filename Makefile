OS=$(shell uname)
SHELL=bash

.SUFFIXES=.c .h .o .a

NAME=libjson.a

SRCD=src
INCD=inc
OBJD=.obj

SRC=$(wildcard $(SRCD)/*$(word 1, $(.SUFFIXES)))
INC=$(wildcard $(INCD)/*$(word 2, $(.SUFFIXES)))
OBJ=$(SRC:$(SRCD)/%$(word 1, $(.SUFFIXES))=$(OBJD)/%$(word 3, $(.SUFFIXES)))

CC=gcc
CFLAGS=-Wall -Werror -Wextra -I $(INCD)

all: $(NAME)

$(NAME): $(OBJ)
	ranlib $@

$(OBJD)/%$(word 3, $(.SUFFIXES)): $(SRCD)/%$(word 1, $(.SUFFIXES)) $(INC)
	$(CC) $(CFLAGS) -c $< -o $@
	ar rc $(NAME) $@

clean:
	rm -f $(OBJD)/*$(word 3, $(.SUFFIXES))

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

debug:
	@echo 'src: $(SRC)'
	@echo 'inc: $(INC)'
	@echo 'obj: $(OBJ)'
