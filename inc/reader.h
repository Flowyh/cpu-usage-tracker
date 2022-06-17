#ifndef READER_H
#define READER_H

#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>

typedef struct Reader
{
  size_t read_interval;
  FILE* f;
} Reader;

typedef uint8_t* (*reader_func_t)(const Reader*);

Reader* reader_create(const char* path, const size_t read_interval);
void reader_rewind(const Reader* reader);
// void reader_read_once(const Reader* reader, const reader_func_t reader_func);
void reader_destroy(Reader* reader);

#endif // !READER_H