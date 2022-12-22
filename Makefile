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
	rm sparrow || true
	rm run_tests || true
	rm *.o  || true
	rm *.db || true
	rm -r sparrow.dSYM || true
