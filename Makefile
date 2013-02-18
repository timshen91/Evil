all:
	gcc -std=c99 -Wall -g *.c -o Evil
clean :
	rm -f Evil

.PHONY : all clean
