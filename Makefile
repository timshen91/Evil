all:
	gcc -Wall -g *.c -o Evil
clean :
	rm -f Evil

.PHONY : all clean
