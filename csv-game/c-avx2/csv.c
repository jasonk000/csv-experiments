#include "csv.h"
#include <stdio.h>
#include <string.h>

int parseBuffer(char* input, char* output, unsigned char* char1, unsigned char* char2, unsigned char* char3, int len);

void field_count(void* str, size_t str_len, void* data) {
    int* count = (int*)data;
    *count += 1;
}

const int READ_SZ = 1024 * 1024;

unsigned char newline = '\n';
unsigned char separator = ' ';
unsigned char quote = '"';

int main (int argc, char* argv[]) {
    struct csv_parser parser = {0};
    csv_init(&parser, CSV_APPEND_NULL);
    FILE* f = fopen(argv[1], "r");
    char *buf = (char*)malloc(READ_SZ);
    char *avxbuf = (char*)malloc(READ_SZ/8);
    size_t buflen = READ_SZ;
    int count = 0;
    while((buflen = fread(buf, 1, READ_SZ, f))  > 0){
        parseBuffer(buf, avxbuf, &separator, &newline, &quote, buflen);
        csv_parse(&parser, buf, 0, buflen, field_count, 0, &count);
    }
    printf("%d\n", count);
    fclose(f);
    free(buf);
    csv_free(&parser);
    return EXIT_SUCCESS;
}
