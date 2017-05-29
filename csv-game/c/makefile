CFLAGS=-Werror -Wall -pedantic -std=c11 -g -O3 -fpic -march=native

.PHONY: all

all: csv csv-count

csv: csv.c
	$(CC) $(CFLAGS) ./libcsv.o $< -o $@

csv-count: csv-count.c
	$(CC) $(CFLAGS) ./libcsv.o $< -o $@

clean: 
	rm csv csv-count
