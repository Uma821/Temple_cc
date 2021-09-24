CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

Temple_cc: $(OBJS)
		$(CC) -o Temple_cc $(OBJS) $(LDFLAGS)

$(OBJS): Temple_cc.h

test: Temple_cc
		./test.sh

clean:
		rm -f Temple_cc *.o *~ tmp*

.PHONY: test clean
