#include "../inc/analyzer.h"
#include "../inc/procstat_wrapper.h"
#include "unittest.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void analyzerpacket_create_test(void)
{
  // NULL NAME
  AnalyzerPacket* packet;
  packet = analyzerpacket_create(NULL);
  assert(packet == NULL);
  printf("Is null? %d\n", packet == NULL);
  // STRLEN(NAME) = 0
  packet = analyzerpacket_create("");
  assert(packet == NULL);
  printf("Is null? %d\n", packet == NULL);
  // LONGER THAN CODE_NAME_LENGTH
  packet = analyzerpacket_create("00000000000000000000");
  assert(packet == NULL);
  printf("Is null? %d\n", packet == NULL);
  packet = analyzerpacket_create("SuperTest");
  assert(strcmp(packet->core_name, "SuperTest") == 0);
  printf("Does packet name match? %d\n", strcmp(packet->core_name, "SuperTest") == 0);
  assert(packet->cpu_percentage == 0.0);
  printf("Does packet cpu_percentage match? %d\n", packet->cpu_percentage == 0.0);
  // DESTROY
  analyzerpacket_destroy(packet);
}

static void analyzer_cpu_usage_test(void)
{
  // VARS
  AnalyzerPacket* cpu_usage_packet;

  // NULL CHECKS
  cpu_usage_packet = analyzer_cpu_usage_packet(NULL, NULL);
  assert(cpu_usage_packet == NULL);
  printf("Is null? %d\n", cpu_usage_packet == NULL);

  // CORE NAME LENGTH MISMATCH
  ProcStatWrapper* prev = procstatwrapper_create();
  prev->core_name[0] = '\0';
  strcat(prev->core_name, "CpuTest");

  ProcStatWrapper* curr1 = procstatwrapper_create();
  curr1->core_name[0] = '\0';
  strcat(curr1->core_name, "CpuTest1");

  cpu_usage_packet = analyzer_cpu_usage_packet(NULL, NULL);
  assert(cpu_usage_packet == NULL);
  printf("Is null? %d\n", cpu_usage_packet == NULL);
  procstatwrapper_destroy(curr1);

  // PREV STATS ONLY
  AnalyzerPacket* cpu_usage_packet1 = analyzer_cpu_usage_packet(prev, NULL);
  assert(strcmp(cpu_usage_packet1->core_name, "CpuTest") == 0);
  printf("Does packet name match? %d\n", strcmp(cpu_usage_packet1->core_name, "CpuTest") == 0);
  assert(cpu_usage_packet1->cpu_percentage == 0.0);
  printf("Does packet cpu_percentage match? %d\n", cpu_usage_packet1->cpu_percentage == 0.0);
  analyzerpacket_destroy(cpu_usage_packet1);

  // CHECK ZERO PERCENTAGE
  ProcStatWrapper* curr2 = procstatwrapper_create();
  curr2->core_name[0] = '\0';
  strcat(curr2->core_name, "CpuTest");
  strcpy(prev->core_name, "CpuTest");
  AnalyzerPacket* cpu_usage_packet2 = analyzer_cpu_usage_packet(prev, curr2);
  assert(strcmp(cpu_usage_packet2->core_name, "CpuTest") == 0);
  printf("Does packet name match? %d\n", strcmp(cpu_usage_packet2->core_name, "CpuTest") == 0);
  assert(cpu_usage_packet2->cpu_percentage == 0.0);
  printf("Does packet cpu_percentage match? %d\n", cpu_usage_packet2->cpu_percentage == 0.0);
  analyzerpacket_destroy(cpu_usage_packet2);
  procstatwrapper_destroy(prev);
  procstatwrapper_destroy(curr2);
}

void analyzer_tests(void)
{
  RUN_TEST(analyzerpacket_create_test);
  RUN_TEST(analyzer_cpu_usage_test);
}