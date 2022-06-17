#ifndef PROCSTAT_WRAPPER_H
#define PROCSTAT_WRAPPER_H

#include <unistd.h>

typedef struct ProcStatWrapper {
  char core_name[16];
  size_t user;
  size_t nice;
  size_t system;
  size_t idle;
  size_t iowait;
  size_t irq;
  size_t softirq;
  size_t steal;
  size_t guest;
  size_t guest_nice;
} ProcStatWrapper;

ProcStatWrapper* procstatwrapper_create(void);
void procstatwrapper_destroy(ProcStatWrapper* stats);
void procstatwrapper_print(const ProcStatWrapper* stats_wrapper);

#endif // !PROCSTAT_WRAPPER_H