#ifndef READER_H
#define READER_H

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>

typedef struct Reader
{
  size_t read_interval;
  FILE* f;
  const char* path;
} Reader;

typedef uint8_t* (*reader_func_t)(const Reader*);

Reader* reader_create(const char* path, const size_t read_interval);
void reader_rewind(Reader* reader);
void reader_destroy(Reader* reader);

#endif // !READER_H
