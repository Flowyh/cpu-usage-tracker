#include "../inc/procstat_wrapper.h"
#include <stdlib.h>
#include <stdio.h>

ProcStatWrapper* procstatwrapper_create(void)
{
  ProcStatWrapper* restrict stats;
  stats = malloc(sizeof(*stats));
  return stats;
}

void procstatwrapper_destroy(ProcStatWrapper* restrict stats)
{
  free(stats);
}

void procstatwrapper_print(register const ProcStatWrapper* const restrict stats_wrapper)
{
  printf("%s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n", 
    stats_wrapper->core_name,
    stats_wrapper->user,
    stats_wrapper->nice,
    stats_wrapper->system,
    stats_wrapper->idle,
    stats_wrapper->iowait,
    stats_wrapper->irq,
    stats_wrapper->softirq,
    stats_wrapper->steal,
    stats_wrapper->guest,
    stats_wrapper->guest_nice);
}