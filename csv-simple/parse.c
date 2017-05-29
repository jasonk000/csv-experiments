#include <stdio.h>
#include <stdbool.h>
#include <zlib.h>
#include <string.h>
#include <x86intrin.h>

#define CHUNK 32*1024

void zerr(int ret);
int countChars(unsigned char* input, unsigned char* character, int len);
int parseBuffer(unsigned char* input, unsigned char* output, unsigned char* char1, unsigned char* char2, int len);

int main(int argc, char** argv) {

  // avx parser variables
  unsigned char newline = '\n';
  unsigned char separator = ' ';
  unsigned char delimiters[CHUNK/8] = {0};

  // initial variables
  z_stream stream;
  int haveInflated = 0;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  // initialise engine
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = 0;
  stream.next_in = Z_NULL;

  // use the second form to process a gzip file
  // ret = inflateInit(&stream);
  int ret = inflateInit2(&stream, 16+MAX_WBITS);
  if (ret != Z_OK) {
    return ret;
  }

  int newlines = 0;
  int spaces = 0;

  // open file
  FILE* inputFile;
  inputFile = fopen("input_346869.esclfcust_S.201607241200-2400-0.gz", "rb");

  // load file data to inflate
  int total = 0;
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

      // parse to delimiters, print delimiters
      for(int i = 0; i < haveInflated; i++) {
        if (out[i] == ' ') spaces++;
        if (out[i] == '\n') newlines++;
      }
    } while (stream.avail_out == 0);
  } while (ret != Z_STREAM_END);

  printf("newlines %d\nspaces %d\n", newlines, spaces);
  fflush(stdout);
  (void)inflateEnd(&stream);
  fclose(inputFile);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
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

