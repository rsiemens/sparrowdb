all:
	cc -std=c99 -O3 -Wall src/*.c -o sparrow

debug:
	cc -std=c99 -Wall -O0 -g src/*.c -o sparrow

clean:
	rm sparrow
	rm -r *.db
	rm -r sparrow.dSYM
