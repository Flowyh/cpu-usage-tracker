#ifndef ANALYZER_H
#define ANALYZER_H

#include "../inc/procstat_wrapper.h"
#include <stdint.h>

typedef struct AnalyzerPacket {
  char core_name[CORE_NAME_LENGTH];
  double cpu_percentage;
} AnalyzerPacket;

AnalyzerPacket* analyzerpacket_create(const char* name);
void analyzerpacket_destroy(AnalyzerPacket* packet);
AnalyzerPacket* analyzer_cpu_usage_packet(const ProcStatWrapper* prev_stats, const ProcStatWrapper* curr_stats);
void analyzerpacket_print(const AnalyzerPacket* packet);

#endif // !ANALYZER_H