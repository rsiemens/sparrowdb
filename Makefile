CFLAGS := -std=c99 -Wall

all:
	cc $(CFLAGS) -O2 src/*.c -o sparrow

debug:
	cc $(CFLAGS) -O0 -g src/*.c -o sparrow

test:
	cc -c $(CFLAGS) -O2 src/page.c -o page.o
	cc $(CFLAGS) -O2 -Isrc/ page.o tests/*.c -o run_tests
	./run_tests

clean:
	rm -f sparrow
	rm -f run_tests
	rm -f *.o
	rm -f *.db
	rm -f sparrow.dSYM
