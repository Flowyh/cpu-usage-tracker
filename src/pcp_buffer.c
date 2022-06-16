#include "../inc/pcp_buffer.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct PCPBuffer
{
  size_t packet_size;
  size_t packet_limit;
  size_t current_packets;
  size_t head;
  size_t tail;
  pthread_mutex_t mutex;
  pthread_cond_t can_produce;
  pthread_cond_t can_consume;
  uint8_t buffer[]; // FAM
};

#include <stdio.h>

PCPBuffer* pcpbuffer_create(register const size_t packet_size, register const size_t packet_limit)
{
  if (packet_size == 0 || packet_limit == 0)
    return NULL;
  
  PCPBuffer* buffer;
  size_t buffer_size = packet_size * packet_limit; // packet_size equals number of bytes for each packet

  buffer = malloc(sizeof(*buffer) + sizeof(uint8_t) * buffer_size);

  if (buffer == NULL)
    return NULL;

  *buffer = (PCPBuffer){.packet_size = packet_size,
                        .packet_limit = packet_limit,
                        .current_packets = (size_t) 0,
                        .head = (size_t) 0,
                        .tail = (size_t) 0,
                       };

  pthread_mutex_init(&buffer->mutex, NULL);
  pthread_cond_init(&buffer->can_produce, NULL);
  pthread_cond_init(&buffer->can_consume, NULL);

  return buffer;
}

size_t pcpbuffer_get_packet_size(register const PCPBuffer* buff)
{
  return buff->packet_size;
}

size_t pcpbuffer_get_current_packets(const PCPBuffer* buff)
{
  return buff->current_packets;
}

size_t pcpbuffer_get_packet_limit(const PCPBuffer* buff)
{
  return buff->packet_limit;
}

bool pcpbuffer_is_full(const PCPBuffer* buff)
{
  return buff->current_packets == buff->packet_limit;
}

bool pcpbuffer_is_empty(const PCPBuffer* buff)
{
  return buff->current_packets == 0;
}

void pcpbuffer_put(PCPBuffer* buff, register const uint8_t packet[], register const size_t packet_size)
{
  if (pcpbuffer_is_full(buff))
    return;

  if (packet_size != buff->packet_size)
    return;

  size_t packet_start = buff->packet_size * buff->head;

  // Copy packet to buffer  
  memcpy(&buff->buffer[packet_start], &packet[0], buff->packet_size);
  buff->head = (buff->head + 1) % buff->packet_limit;
  buff->current_packets++;
}

uint8_t* pcpbuffer_get(PCPBuffer* buff)
{
  if (pcpbuffer_is_empty(buff))
    return NULL;

  uint8_t* packet;
  packet = malloc(sizeof(packet) * buff->packet_size);

  size_t packet_start = buff->packet_size * buff->tail;

  memcpy(&packet[0], &buff->buffer[packet_start], buff->packet_size);
  buff->tail = (buff->tail + 1) % buff->packet_limit;
  buff->current_packets--;

  return packet;
}

void pcpbuffer_lock(PCPBuffer* buff)
{
  pthread_mutex_lock(&buff->mutex);
}

void pcpbuffer_unlock(PCPBuffer* buff)
{
  pthread_mutex_unlock(&buff->mutex);
}

void pcpbuffer_wake_producer(PCPBuffer* buff)
{
  pthread_cond_signal(&buff->can_produce);
}

void pcpbuffer_wake_consumer(PCPBuffer* buff)
{
  pthread_cond_signal(&buff->can_consume);
}

void pcpbuffer_wait_for_producer(PCPBuffer* buff)
{
  pthread_cond_wait(&buff->can_produce, &buff->mutex);
}

void pcpbuffer_wait_for_consumer(PCPBuffer* buff)
{
  pthread_cond_wait(&buff->can_consume, &buff->mutex);
}

void pcpbuffer_destroy(PCPBuffer* buff)
{
  pthread_mutex_destroy(&buff->mutex);
  pthread_cond_destroy(&buff->can_produce);
  pthread_cond_destroy(&buff->can_consume);
  free(buff);
}