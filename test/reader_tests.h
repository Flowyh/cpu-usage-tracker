#include "../inc/reader.h"
#include "unittest.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

static void reader_invalid_create_test(void)
{
  // Non-existent file
  Reader* reader = reader_create("./nonexistent", 2);
  bool is_null = reader == NULL;
  printf("Is null? %d\n", is_null);
  assert(is_null);
  // Invalid read interval
  reader = reader_create("./nonexistent", 0);
  is_null = reader == NULL;
  printf("Is null? %d\n", is_null);
  assert(is_null);
  // NULL path
  reader = reader_create(NULL, 1);
  is_null = reader == NULL;
  printf("Is null? %d\n", is_null);
  assert(is_null);
  // NULL path, invalid interval
  reader = reader_create(NULL, 0);
  is_null = reader == NULL;
  printf("Is null? %d\n", is_null);
  assert(is_null);
  reader_destroy(reader);
}

static void reader_valid_path_test(void)
{
  Reader* reader = reader_create("./test/reader_test", 2);
  bool is_null = reader == NULL;
  printf("Is null? %d\n", is_null);
  assert(!is_null);
  reader_destroy(reader);
}

static void reader_rewind_test(void)
{
  Reader* reader = reader_create("./test/reader_test", 2);
  fseek(reader->f, 0, SEEK_END);
  size_t reader_cursor = (size_t) ftell(reader->f);
  printf("Current reader position: %zu\n", reader_cursor);
  assert(reader_cursor == 8);
  printf("Rewinding . . .\n");
  reader_rewind(reader);
  reader_cursor = (size_t) ftell(reader->f);
  printf("Current reader position: %zu\n", reader_cursor);
  assert(reader_cursor == 0);
  reader_destroy(reader);
}

static void reader_dummy_func_test(void)
{
  Reader* reader = reader_create("./test/reader_test", 2);

  char test[5];
  int one_two_three[1];
  int scan = fscanf(reader->f, "%s %d", test, one_two_three);
  (void) scan;
  printf("Test: %s, 123: %d\n", test, one_two_three[0]);
  assert(one_two_three[0] == 123);

  reader_destroy(reader);
}

void reader_tests(void);
void reader_tests(void)
{
  RUN_TEST(reader_invalid_create_test);
  RUN_TEST(reader_valid_path_test);
  RUN_TEST(reader_rewind_test);
  RUN_TEST(reader_dummy_func_test);
}
