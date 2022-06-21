#ifndef PCP_BUFFER_H
#define PCP_BUFFER_H

#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>

typedef struct PCPBuffer PCPBuffer;

PCPBuffer* pcpbuffer_create(size_t packet_size, size_t packet_limit);
// GETTER
size_t pcpbuffer_get_packet_size(const PCPBuffer* buff);
size_t pcpbuffer_get_current_packets(const PCPBuffer* buff);
size_t pcpbuffer_get_packet_limit(const PCPBuffer* buff);
// STATE CHECKS
bool pcpbuffer_is_full(const PCPBuffer* buff);
bool pcpbuffer_is_empty(const PCPBuffer* buff);
// BUFFER HANDLERS
void pcpbuffer_put(PCPBuffer* restrict buff, const uint8_t packet[], size_t packet_size);
uint8_t* pcpbuffer_get(PCPBuffer* buff);
// PTHREAD LOCKS/CONDS
void pcpbuffer_lock(PCPBuffer* buff);
void pcpbuffer_unlock(PCPBuffer* buff);
void pcpbuffer_wake_producer(PCPBuffer* buff);
void pcpbuffer_wake_consumer(PCPBuffer* buff);
void pcpbuffer_wait_for_producer(PCPBuffer* buff);
void pcpbuffer_wait_for_consumer(PCPBuffer* buff);
void pcpbuffer_destroy(PCPBuffer* buff);

#endif // !PCP_BUFFER_H
