#sources
XSON_SRC = main.c
#object files
XSON_OBJ = $(XSON_SRC:.c=.o)
#executable
PROGRAM = main
#compiler
CC = gcc

#includes
#INCLUDE = 
#linker params
LINKPARAMS = -g -O0 -lxson
#options for development
#CFLAGS = -g -O0 -Wall -fpic
#options for release
CFLAGS = -g -O2 -Wall -Werror

all:
	$(CC) $(CFLAGS) -o $(PROGRAM) $(XSON_SRC) $(LINKPARAMS)

clean:
	rm $(PROGRAM)
	rm *.o