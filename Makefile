all:
	clang -std=c11 -Wall -g *.c -o Evil
release:
	clang -std=c11 -O2 -Wall -g *.c -o Evil
profile:
	gcc -std=c11 -pg -Wall -g *.c -o Evil
coverage:
	gcc -std=c11 -fprofile-arcs -ftest-coverage -Wall -g *.c -o Evil
clean :
	rm -f Evil *.gcno *.gcda gmon.out *.gcov

.PHONY : all release profile clean
