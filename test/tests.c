#include <stdio.h>
#include "watchdog_tests.h"
#include "pcpbuffer_tests.h"

int main() {
  watchdog_tests();
  pcpbuffer_tests();
  return 0;
}