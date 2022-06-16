#include <stdio.h>
#include "watchdog_tests.h"
#include "pcpbuffer_tests.h"
#include "reader_test.h"

int main() {
  watchdog_tests();
  pcpbuffer_tests();
  reader_tests();
  return 0;
}