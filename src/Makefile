#sources
XSON_SRC = parser.c fsm_number.c fsm_string.c list.c xson_common.c xson_pair_ht.c xson_types.c xmalloc.c
#object files
XSON_OBJ = $(XSON_SRC:.c=.o)
#executable
PROGRAM = libxson.so
#compiler
CC = gcc

#includes
INCLUDE = xson
#linker params
LINKPARAMS = -g -O0 -shared
#options for development
#CFLAGS = -g -O0 -Wall -fpic
#options for release
CFLAGS = -g -O2 -Wall -Werror -fpic

all: $(PROGRAM)

$(PROGRAM): compile
	$(CC) -o $(PROGRAM) $(LINKPARAMS) $(XSON_OBJ)
.SUFFIXES: .c.o

compile:
	$(CC)  -I$(INCLUDE) $(CFLAGS) -c $(XSON_SRC) $<

clean:
	-rm *.o
	-rm $(PROGRAM)

install:
	cp $(PROGRAM) /usr/lib
	cp -r $(INCLUDE) /usr/include

uninstall:
	rm -rf /usr/include/$(INCLUDE)
	rm /usr/lib/$(PROGRAM)