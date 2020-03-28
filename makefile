all:
	gcc -Wall -Wextra banker.c -std=c99 -o banker -pthread