
NAME		= packer

SRC		= src/packer.c		\
		  src/cypher_code.c	\
		  src/write_file.c	\
		  src/insert_section.c	\
		  src/map_elf.c

SRC_ASM		= src/loader.asm

OBJ_ASM		= $(SRC_ASM:.asm=.o)

OBJ		= $(SRC:.c=.o)

INCLUDES	= ./includes

CC		= gcc

NASM		= nasm

RM		= rm -f

override CFLAGS	+= -Wall -Wextra -std=c99 -I $(INCLUDES)

override NFLAGS	+= -f elf64

$(NAME):	$(OBJ) $(OBJ_ASM)
		$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(OBJ_ASM)

$(OBJ_ASM):	$(SRC_ASM)
		$(NASM) $(NFLAGS) -o $@ $<

all:		$(NAME)

clean:
		$(RM) $(OBJ) $(OBJ_ASM)

fclean:		clean
		$(RM) $(NAME) exec

re:		fclean all

.PHONY:		all clean fclean re
