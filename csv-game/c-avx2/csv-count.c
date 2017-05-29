#include "csv.h"
#include <stdio.h>
#include <string.h>

struct ctx { 
    const size_t sum_field;
    int count;
    size_t col;
};

void field_count(void* str, size_t str_len, void* data) {
    struct ctx* ctx = (struct ctx*)data;
    if (ctx->col == ctx->sum_field) {
        if (((const char*)str)[0] == CSV_QUOTE) {
            ctx->count += atol(((const char*)str) + 1);
        } else {
            ctx->count += atol((const char*)str);
        }
    }
    ctx->col++;
}

void reset_line(int line, void* data) {
    struct ctx* ctx = (struct ctx*)data;
    ctx->col = 1;
}

const int READ_SZ = 1024 * 1024;

int main (int argc, char* argv[]) {
    struct csv_parser parser = {0};
    csv_init(&parser, CSV_APPEND_NULL);
    FILE* f = fopen(argv[2], "r");
    size_t field = atol(argv[1]);
    char *buf = (char*)malloc(READ_SZ);
    size_t buflen = READ_SZ;
    struct ctx ctx = {field, 0, 1};
    while((buflen = fread(buf, 1, READ_SZ, f))  > 0){
        csv_parse(&parser, buf, 0, buflen, field_count, reset_line, &ctx);
    }
    printf("%d\n", ctx.count);
    fclose(f);
    free(buf);
    csv_free(&parser);
    return EXIT_SUCCESS;
}
