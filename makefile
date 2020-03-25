all:
	gcc -Wall banker.c -std=c99 -o banker -lpthread