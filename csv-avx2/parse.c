#include <stdio.h>
#include <stdbool.h>
#include <zlib.h>
#include <string.h>
#include <x86intrin.h>
#include <fcntl.h>
#include <errno.h>

#define CHUNK 64*1024

void zerr(int ret);
int countChars(unsigned char* input, unsigned char* character, int len);
int parseBuffer(unsigned char* input, unsigned char* output, unsigned char* char1, unsigned char* char2, int len);

const char *byte_to_binary(int x)
{
  static char b[9];
  b[0] = '\0';

  int z;
  for (z = 128; z > 0; z >>= 1)
  {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

int currentField = 1;
int startOfField = 0;
void nextDelimiter(unsigned char* input, int position) {
  // printf("       %c %d %d\n", input[position], input[position], position);
  if (input[position] == ' ') currentField++;
  if (input[position] == '\n') currentField = 0;
  if (currentField == 1) {
    // print the first field to stdout then exit
    fwrite(&input[startOfField], 1, position - startOfField, stdout);
    printf("\n");
  }

  // update next start position
  startOfField = position + 1; 
}

void lookForDelimiters(unsigned char* input, unsigned char* delimiters, int length) {
  int delimlength = length/8;
  for(int i = 0; i < delimlength; i++) {
    unsigned char delim = delimiters[i];
    while (delim != 0) {
      long firstBit = _tzcnt_u32(delim);
      if (firstBit >= 8) {
        break;
      }

      int position = (8 * i) + (firstBit);

      // output the delimiter
      nextDelimiter(input, position);

      // clear the bit we just read and go again
      delim = delim & ~(1 << firstBit);
    }
  }
}

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

      // clear anything above haveInflated, so the buffer is not consumed
      memset(&out[haveInflated], 0, stream.avail_out);

      // just counting newlines
      // int result = countChars(out, &newline, CHUNK);
      // total += result;

      // parse to delimiters, print delimiters
      int result = parseBuffer(out, delimiters, &separator, &newline, CHUNK);
      // printf("parse %d\n", result);
      lookForDelimiters(out, delimiters, haveInflated);
    } while (stream.avail_out == 0);
  } while (ret != Z_STREAM_END);

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

