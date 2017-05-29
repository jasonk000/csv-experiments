parse: parse.c avxparse.s
	nasm -f elf64 avxparse.s
	gcc -Wall -march=native -o parse parse.c avxparse.o -lz

