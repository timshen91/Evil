all:
	clang -std=c11 -Wall -g *.c -o Evil
clean :
	rm -f Evil

.PHONY : all clean
