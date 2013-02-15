all:
	gcc -Wall -g *.c -o yaScheme
clean :
	rm -f yaScheme

.PHONY : all clean
