#include "../inc/reader.h"
#include <stdlib.h>
#include <sys/stat.h>

Reader* reader_create(register const char* const restrict path, register const size_t read_interval)
{
  if (path == NULL)
    return NULL;

  if (read_interval == 0)
    return NULL;

  Reader* restrict reader;
  reader = malloc(sizeof(*reader));

  if (reader == NULL)
    return NULL;

  // Check if file exists
  struct stat buffer; 
  if (stat(path, &buffer) != 0) // It doesn't
  {
    free(reader);
    return NULL;
  }

  *reader = (Reader){.f = fopen(path, "r"),
                     .read_interval = read_interval,
                     .path = path,
                    };

  return reader;
}

void reader_rewind(Reader* restrict reader)
{
  // https://stackoverflow.com/a/16267995/18870209
  // Apparently fseeking/rewinding doesn't help with rereading updated /proc/stat file
  // You have to manually close and reopen the file to read new contents.
  fclose(reader->f);
  reader->f = fopen(reader->path, "r");
  // rewind(reader->f);
  // fseek(reader->f, 0, SEEK_SET);
}

void reader_destroy(Reader* restrict reader)
{
  if (reader == NULL)
    return;

  fclose(reader->f);
  free(reader);
}
