#include "../inc/procstat_wrapper.h"
#include <stdlib.h>

ProcStatWrapper* procstatwrapper_create(void)
{
  ProcStatWrapper* stats;
  stats = malloc(sizeof(*stats));
  return stats;
}

void procstatwrapper_destroy(ProcStatWrapper* stats)
{
  free(stats);
}
