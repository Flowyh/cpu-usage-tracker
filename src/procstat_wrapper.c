#include "../inc/procstat_wrapper.h"
#include <stdlib.h>
#include <stdio.h>

ProcStatWrapper* procstatwrapper_create(void)
{
  ProcStatWrapper* const stats = malloc(sizeof(*stats));

  if (stats == NULL)
    return NULL;

  *stats = (ProcStatWrapper){
                            .user = 0.0,
                            .nice = 0.0,
                            .system = 0.0,
                            .idle = 0.0,
                            .iowait = 0.0,
                            .irq = 0.0,
                            .softirq = 0.0,
                            .steal = 0.0,
                            .guest = 0.0,
                            .guest_nice = 0.0
                            };
  return stats;
}

void procstatwrapper_destroy(ProcStatWrapper* const stats)
{
  if (stats == NULL)
    return;

  free(stats);
}

void procstatwrapper_print(register const ProcStatWrapper* const stats_wrapper)
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
