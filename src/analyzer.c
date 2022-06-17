#include "../inc/analyzer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

AnalyzerPacket* analyzerpacket_create(const char* name)
{
  if (name == NULL)
    return NULL;

  size_t name_len = strlen(name);
  if (name_len > CORE_NAME_LENGTH || name_len == 0)
    return NULL;

  AnalyzerPacket* packet;
  packet = malloc(sizeof(*packet));

  strcpy(&packet->core_name[0], name);

  // memcpy(&packet->core_name[0], &name[0], name_len);
  packet->cpu_percentage = 0.0;

  return packet;
}

void analyzerpacket_destroy(AnalyzerPacket* packet)
{
  free(packet);
}

AnalyzerPacket* analyzer_cpu_usage_packet(const ProcStatWrapper* prev_stats, const ProcStatWrapper* curr_stats)
{
  if (prev_stats == NULL)
    return NULL;

  AnalyzerPacket* packet = analyzerpacket_create(prev_stats->core_name); 

  if (curr_stats == NULL) // This is the first packet, return core name and 0
    return packet;

  if (strcmp(prev_stats->core_name, curr_stats->core_name) != 0) // If core names don't match
    return NULL;

  // PrevIdle = previdle + previowait
  // Idle = idle + iowait

  // PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
  // NonIdle = user + nice + system + irq + softirq + steal

  // PrevTotal = PrevIdle + PrevNonIdle
  // Total = Idle + NonIdle

  // # differentiate: actual value minus the previous one
  // totald = Total - PrevTotal
  // idled = Idle - PrevIdle

  // CPU_Percentage = (totald - idled)/totald

  double prev_idle = (double) prev_stats->idle + (double) prev_stats->iowait;
  double curr_idle = (double) curr_stats->idle + (double) curr_stats->iowait;

  double prev_nonidle = (double) prev_stats->user + (double) prev_stats->nice + 
                        (double) prev_stats->system + (double) prev_stats->irq + 
                        (double) prev_stats->softirq + (double) prev_stats->steal;
                        
  double curr_nonidle = (double) curr_stats->user + (double) curr_stats->nice + 
                        (double) curr_stats->system + (double) curr_stats->irq + 
                        (double) curr_stats->softirq + (double) curr_stats->steal;
                        
  double prev_total = prev_idle + prev_nonidle;
  double curr_total = curr_idle + curr_nonidle;

  double total_diff = curr_total - prev_total;
  double idle_diff = curr_idle - prev_idle;

  double cpu_core_percentage = (total_diff - idle_diff) / total_diff;
  
  if (total_diff == 0.0)
    cpu_core_percentage = 0.0;

  packet->cpu_percentage = cpu_core_percentage;

  return packet;
}

void analyzerpacket_print(register const AnalyzerPacket* const packet)
{
  if (packet == NULL)
    return;
  printf("%s %f\n", packet->core_name, packet->cpu_percentage);
}