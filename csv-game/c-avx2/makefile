CFLAGS=-Werror -Wall -pedantic -std=c11 -g -fpic -march=native

.PHONY: all

all: csv csv-count

csv: csv.c
	$(CC) $(CFLAGS) ./libcsv.o ./avxparse.o $< -o $@

csv-count: csv-count.c
	$(CC) $(CFLAGS) ./libcsv.o ./avxparse.o $< -o $@

clean: 
	rm -f csv csv-count
