#include "../inc/analyzer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

AnalyzerPacket* analyzerpacket_create(register const char* const restrict name)
{
  if (name == NULL)
    return NULL;

  register const size_t name_len = strlen(name);
  if (name_len > CORE_NAME_LENGTH || name_len == 0)
    return NULL;

  AnalyzerPacket* restrict packet;
  packet = malloc(sizeof(*packet));

  strncpy(&packet->core_name[0], name, CORE_NAME_LENGTH);

  packet->cpu_percentage = 0.0;

  return packet;
}

void analyzerpacket_destroy(AnalyzerPacket* restrict packet)
{
  free(packet);
}

AnalyzerPacket* analyzer_cpu_usage_packet(register const ProcStatWrapper* const restrict prev_stats, register const ProcStatWrapper* const restrict curr_stats)
{
  if (prev_stats == NULL)
    return NULL;

  AnalyzerPacket* restrict packet = analyzerpacket_create(prev_stats->core_name); 

  if (curr_stats == NULL) // This is the first packet, return core name and 0
    return packet;

  if (strcmp(prev_stats->core_name, curr_stats->core_name) != 0) // If core names don't match
    return NULL;

  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux

  // I've typecasted everything just to be sure that everything will be ok
  register const double prev_idle = (double) prev_stats->idle + (double) prev_stats->iowait;
  register const double curr_idle = (double) curr_stats->idle + (double) curr_stats->iowait;

  register const double prev_nonidle = (double) prev_stats->user + (double) prev_stats->nice + 
                        (double) prev_stats->system + (double) prev_stats->irq + 
                        (double) prev_stats->softirq + (double) prev_stats->steal;
                        
  register const double curr_nonidle = (double) curr_stats->user + (double) curr_stats->nice + 
                        (double) curr_stats->system + (double) curr_stats->irq + 
                        (double) curr_stats->softirq + (double) curr_stats->steal;
                        
  register const double prev_total = prev_idle + prev_nonidle;
  register const double curr_total = curr_idle + curr_nonidle;

  register const double total_diff = curr_total - prev_total;
  register const double idle_diff = curr_idle - prev_idle;

  double cpu_core_percentage = (total_diff - idle_diff) / total_diff;
  
  if (total_diff == 0.0) // Div by zero case
    cpu_core_percentage = 0.0;

  packet->cpu_percentage = cpu_core_percentage;

  return packet;
}

void analyzerpacket_print(register const AnalyzerPacket* const restrict packet)
{
  if (packet == NULL)
    return;
  printf("%s %f\n", packet->core_name, packet->cpu_percentage);
}