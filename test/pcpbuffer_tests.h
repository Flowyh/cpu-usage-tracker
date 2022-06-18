#include "../inc/pcp_buffer.h"
#include "unittest.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void pcpbuffer_create_test(void)
{
  // NULL CHECKS
  PCPBuffer* buffer;
  buffer = pcpbuffer_create(0, 1);
  assert(buffer == NULL);
  buffer = pcpbuffer_create(1, 0);
  assert(buffer == NULL);
  buffer = pcpbuffer_create(8, 10);
  assert(buffer != NULL);
  size_t current_packets = pcpbuffer_get_current_packets(buffer);
  printf("Current packets: %zu\n", current_packets);
  assert(current_packets == 0);

  size_t packet_size = pcpbuffer_get_packet_size(buffer);
  printf("Packet size: %zu\n", packet_size);
  assert(packet_size == 8);
  
  size_t packet_limit = pcpbuffer_get_packet_limit(buffer);
  printf("Packet limit: %zu\n", packet_limit);
  assert(packet_limit == 10);
  pcpbuffer_destroy(buffer);
}

static void pcpbuffer_full_test(void)
{
  PCPBuffer* buffer;
  buffer = pcpbuffer_create(1, 1);
  assert(buffer != NULL);
  uint8_t packet[1] = { 0x2 };
  pcpbuffer_put(buffer, packet, 1);
  // FULL CHECK
  bool is_full = pcpbuffer_is_full(buffer);
  printf("Is full? %d\n", is_full);
  assert(is_full);
  // EMPTY CHECK
  bool is_empty = pcpbuffer_is_empty(buffer);
  printf("Is empty? %d\n", is_empty);
  assert(!is_empty);
  pcpbuffer_destroy(buffer);
}

static void pcpbuffer_empty_test(void)
{
  PCPBuffer* buffer;
  buffer = pcpbuffer_create(1, 1);
  assert(buffer != NULL);
  // FULL CHECK
  bool is_full = pcpbuffer_is_full(buffer);
  printf("Is full? %d\n", is_full);
  assert(!is_full);
  // EMPTY CHECK
  bool is_empty = pcpbuffer_is_empty(buffer);
  printf("Is empty? %d\n", is_empty);
  assert(is_empty);
  pcpbuffer_destroy(buffer);
}

static void pcpbuffer_put_get_test(void)
{
  PCPBuffer* buffer;
  buffer = pcpbuffer_create(1, 1);
  assert(buffer != NULL);
  // FULL CHECK
  bool is_full = pcpbuffer_is_full(buffer);
  printf("Is full? %d\n", is_full);
  assert(!is_full);
  // EMPTY CHECK
  bool is_empty = pcpbuffer_is_empty(buffer);
  printf("Is empty? %d\n", is_empty);
  assert(is_empty);

  // PUT TEST
  uint8_t packet[1] = { 0x2 };
  printf("Putting %d . . .\n", packet[0]);
  pcpbuffer_put(buffer, packet, 1);
  // FULL CHECK
  is_full = pcpbuffer_is_full(buffer);
  printf("Is full? %d\n", is_full);
  assert(is_full);
  // EMPTY CHECK
  is_empty = pcpbuffer_is_empty(buffer);
  printf("Is empty? %d\n", is_empty);
  assert(!is_empty);

  // GET TEST
  uint8_t* packet2;
  printf("Getting . . .\n");
  packet2 = pcpbuffer_get(buffer);
  // FULL CHECK
  is_full = pcpbuffer_is_full(buffer);
  printf("Is full? %d\n", is_full);
  assert(!is_full);
  // EMPTY CHECK
  is_empty = pcpbuffer_is_empty(buffer);
  printf("Is empty? %d\n", is_empty);
  assert(is_empty);
  // CHECK WHETHER PACKETS ARE THE SAME
  bool same_packets = packet[0] == packet2[0];
  printf("Are packets the same? %d\n", same_packets);
  assert(same_packets);

  free(packet2);
  pcpbuffer_destroy(buffer);
}

void pcpbuffer_tests(void);
void pcpbuffer_tests(void)
{
  RUN_TEST(pcpbuffer_create_test);
  RUN_TEST(pcpbuffer_full_test);
  RUN_TEST(pcpbuffer_empty_test);
  RUN_TEST(pcpbuffer_put_get_test);
}
