#include <stdio.h>
#include <stdbool.h>
#include <zlib.h>
#include <string.h>
#include "csv.h"

#define CHUNK 512*1024

void zerr(int ret);
int processBuffer(unsigned char* buffer, unsigned char* avxbuffer, int available, struct csv_parser* p);
int parseBuffer(unsigned char* input, unsigned char* output, unsigned char* char1, unsigned char* char2, unsigned char* char3, int len);

int count = 0;

void fieldCb (void *strbuff, size_t size, void *data) {
  char *str = (char *) strbuff;
}

void rowCb (int c, void *data) {
  count++;
}

unsigned char newline = '\n';
unsigned char separator = ' ';
unsigned char quote = '"';

int main(int argc, char** argv) {
  // initial unzip variables
  z_stream stream;
  int haveInflated = 0;
  unsigned char in[CHUNK] = {0};
  unsigned char out[CHUNK] = {0};
  unsigned char avxbuff[CHUNK/8] = {0};
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = 0;
  stream.next_in = Z_NULL;

  // initialise csv parser
  struct csv_parser parser;
  if (csv_init(&parser, 0) != 0) return -1;
  csv_set_delim(&parser, separator);

  // use the second form to process a gzip file
  // ret = inflateInit(&stream);
  int ret = inflateInit2(&stream, 16+MAX_WBITS);
  if (ret != Z_OK) {
    return ret;
  }

  // open file
  FILE* inputFile;
  inputFile = fopen("input_346869.esclfcust_S.201607241200-2400-0.gz", "rb");

  // load file data to inflate
  do {
    stream.avail_in = fread(in, 1, CHUNK, inputFile);
    if (ferror(inputFile)) {
      (void)inflateEnd(&stream);
      return Z_ERRNO;
    }
    if (stream.avail_in == 0) {
      break;
    }
    stream.next_in = in;
    // inflate the data
    do {
      stream.avail_out = CHUNK;
      stream.next_out = out;

      ret = inflate(&stream, Z_NO_FLUSH);
      switch(ret) {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR; // fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
          printf("failed to inflate\n");
          (void)inflateEnd(&stream);
          zerr(ret);
          return -1;
      }
      haveInflated = CHUNK - stream.avail_out;

      // 0 out any areas of the buffer not filled
      if (stream.avail_out > 0)
        memset(&out[haveInflated], 0, stream.avail_out);

      parseBuffer(out, avxbuff, &separator, &newline, &quote, CHUNK);

      processBuffer(out, avxbuff, haveInflated, &parser);
    } while (stream.avail_out == 0);
  } while (ret != Z_STREAM_END);

  printf("%d\n", count);
  csv_fini(&parser, fieldCb, rowCb, 0);
  fflush(stdout);
  (void)inflateEnd(&stream);
  fclose(inputFile);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int processBuffer(unsigned char* buffer, unsigned char* avxbuffer, int available, struct csv_parser* p) {
  int ret = csv_parse(p, buffer, avxbuffer, available, fieldCb, rowCb, 0);
  if (ret != available) {
    fprintf(stderr, "got return %d vs available %d", ret, available);
    fprintf(stderr, "error while parsing %s\n", csv_strerror(csv_error(p)));
    return -1;
  }
  return 0;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
  switch (ret) {
    case Z_ERRNO:
      if (ferror(stdin))
        fputs("error reading stdin\n", stderr);
      if (ferror(stdout))
        fputs("error writing stdout\n", stderr);
      break;
    case Z_STREAM_ERROR:
      fputs("Z_STREAM_ERROR invalid compression level\n", stderr);
      break;
    case Z_DATA_ERROR:
      fputs("Z_DATA_ERROR invalid or incomplete deflate data\n", stderr);
      break;
    case Z_MEM_ERROR:
      fputs("Z_MEM_ERROR out of memory\n", stderr);
      break;
    case Z_VERSION_ERROR:
      fputs("Z_VERSION_ERROR zlib version mismatch!\n", stderr);
  }
}

