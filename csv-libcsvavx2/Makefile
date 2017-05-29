

parse: parse.c libcsv-3.0.3/libcsv.o avxparse.o
	gcc -Wall -mavx2 -O2 -o parse parse.c libcsv-3.0.3/libcsv.o avxparse.o -lz

libcsv-3.0.3/libcsv.o:
	cd libcsv-3.0.3 && gcc -c -Wall -mavx2 -O2 -o libcsv.o libcsv.c

avxparse.o:
	nasm -f elf64 avxparse.s

clean:
	$(MAKE) -C libcsv-3.0.3 clean
	rm -f ./avxparse.o
	rm -f ./libcsv-3.0.3/libcsv.o
	rm -f ./parse

