#include <stdio.h>
#include "watchdog_tests.h"
#include "pcpbuffer_tests.h"
#include "reader_tests.h"
#include "analyzer_tests.h"

int main() {
  watchdog_tests();
  pcpbuffer_tests();
  reader_tests();
  analyzer_tests();
  return 0;
}