CFLAGS=-std=c11 -g -static

Temple_cc: Temple_cc.c

clean:
	rm -f Temple_cc *.o *~ tmp*

.PHONY: test clean
