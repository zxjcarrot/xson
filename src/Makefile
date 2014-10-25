#sources
XSON_SRC = parser.c fsm_number.c fsm_string.c list.c common.c pair_ht.c types.c xmalloc.c array.c number.c string.c pair.c root.c object.c null.c bool.c
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
CFLAGS = -O2 -Wall -Werror -fpic -DNDEBUG

#default page size
XSON_PAGE_SIZE=2048

include config

start:
	echo $(XSON_PAGE_SIZE)

all: $(PROGRAM)

$(PROGRAM): compile
	$(CC) -o $(PROGRAM) $(LINKPARAMS) $(XSON_OBJ)
.SUFFIXES: .c.o

compile:
	$(CC)  -I$(INCLUDE) $(CFLAGS) -DXSON_PAGE_SIZE=$(XSON_PAGE_SIZE) -c $(XSON_SRC) $<

clean:
	-rm *.o
	-rm $(PROGRAM)

install:
	cp $(PROGRAM) /usr/lib
	cp -r $(INCLUDE) /usr/include

uninstall:
	rm -rf /usr/include/$(INCLUDE)
	rm /usr/lib/$(PROGRAM)