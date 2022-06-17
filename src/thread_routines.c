#include "../inc/threads_routines.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>

#define THREADS_COUNT 5
#define WATCHDOG_LIMIT 2
#define WATCHDOG_SLEEP 1
#define READER_INTERVAL 5

// BUFFERS
static PCPBuffer* restrict reader_analyzer_buffer;
static size_t reader_packet_size;

static PCPBuffer* restrict analyzer_printer_buffer;
static size_t analyzer_packet_size;

static PCPBuffer* restrict logger_buffer;
static size_t cpu_cores;
// WATCHDOG
static WatchdogPack* wdog_pack;
// THREADS
static pthread_t tid[THREADS_COUNT]; // 0 - watchdog, 1 - reader, 2 - analyzer, 3 - printer, 4 - logger
// EXIT FLAG
static bool exit_flag = false;

// STATIC DECLARATIONS
static size_t reader_get_packet_size(void);
static size_t analyzer_get_packet_size(void);
// /proc/stat READ WRAPPER
typedef struct ProcStatWrapper ProcStatWrapper;
static ProcStatWrapper* procstatwrapper_create(void);
static void procstatwrapper_destroy(ProcStatWrapper* stats);
// Reader func
static uint8_t* reader_read_proc_stat(register const Reader* const reader);

void signal_exit(void)
{
  exit_flag = true;
}

void sigterm_graceful_exit(int signum)
{
  printf("Signal caught! Signum: %d\n", signum);
  signal_exit();
}


static size_t reader_get_packet_size(void)
{
  size_t cores = sysconf(_SC_NPROCESSORS_ONLN);
  return (sizeof(char[10]) + sizeof(size_t) * 10) * (cores + 1);
}

static size_t analyzer_get_packet_size(void)
{
  size_t cores = sysconf(_SC_NPROCESSORS_ONLN);
  return (sizeof(char[10]) + sizeof(size_t)) * (cores + 1);
}

struct ProcStatWrapper {
  char core_name[10];
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
};

static ProcStatWrapper* procstatwrapper_create(void)
{
  ProcStatWrapper* stats;
  stats = malloc(sizeof(*stats));
  return stats;
}

static void procstatwrapper_destroy(ProcStatWrapper* stats)
{
  free(stats);
}

static uint8_t* reader_read_proc_stat(register const Reader* const reader)
{
  ProcStatWrapper* stats_wrapper = procstatwrapper_create();
  uint8_t* packet = malloc(reader_packet_size);
  size_t one_core = sizeof(char[10]) + sizeof(size_t) * 10;

  int fscan_result;
  for (size_t i = 0; i < cpu_cores; i++)
  {
    fscan_result = fscanf(reader->f, 
      "%s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
      stats_wrapper->core_name,
      &stats_wrapper->user,
      &stats_wrapper->nice,
      &stats_wrapper->system,
      &stats_wrapper->idle,
      &stats_wrapper->iowait,
      &stats_wrapper->irq,
      &stats_wrapper->softirq,
      &stats_wrapper->steal,
      &stats_wrapper->guest,
      &stats_wrapper->guest_nice
    );

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
    (void)fscan_result;
    memcpy(&packet[one_core * i], &stats_wrapper[0], one_core);
  }
  procstatwrapper_destroy(stats_wrapper);
  return packet;
}

void* reader_thread(void* arg)
{
  (void)(arg);
  Reader* proc_stat_reader = reader_create("/proc/stat", READER_INTERVAL);
  
  if (proc_stat_reader == NULL)
    return NULL;

  pthread_t tid = pthread_self();
  Watchdog* wdog = watchdog_create(tid, WATCHDOG_LIMIT);

  if (wdog == NULL)
    return NULL;

  int wdog_register_status = watchdogpack_register(wdog_pack, wdog);
  
  if (wdog_register_status == -1)
    return NULL;

  while(true)
  {
    if (exit_flag)
      break;

    printf("READING\n");
    watchdog_snooze(wdog);
    reader_rewind(proc_stat_reader);
    uint8_t* packet = reader_read_proc_stat(proc_stat_reader);
    
    pcpbuffer_lock(reader_analyzer_buffer);
    if (pcpbuffer_is_full(reader_analyzer_buffer))
    {
      pcpbuffer_wait_for_consumer(reader_analyzer_buffer);
    }
    pcpbuffer_put(reader_analyzer_buffer, packet, reader_packet_size);
    pcpbuffer_wake_consumer(reader_analyzer_buffer);
    pcpbuffer_unlock(reader_analyzer_buffer);

    free(packet);
    sleep(proc_stat_reader->read_interval);
  }
  
  printf("Reader: Exit signaled. Exitting\n");
  reader_destroy(proc_stat_reader);
  watchdogpack_unregister(wdog_pack, wdog_register_status);
  watchdog_destroy(wdog);
  return NULL;
}

void* watchdog_thread(void* arg)
{
  (void)arg;
  int alarm_check;

  while(true)
  {
    if (exit_flag)
      break;
    
    printf("WATCHING\n");
    alarm_check = watchdogpack_check_alarms(wdog_pack);
    if (alarm_check != -1)
    {
      printf("A dog with id=%d has barked\n", alarm_check);
      break;
    }
    sleep(WATCHDOG_SLEEP);
  }
  
  printf("Watchdog: Exit signaled. Exitting\n");
  signal_exit();
  return NULL;
}

void run_threads(void)
{
  // Init statics
  cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
  reader_packet_size = reader_get_packet_size();
  reader_analyzer_buffer = pcpbuffer_create(reader_packet_size, 10);
  analyzer_packet_size = analyzer_get_packet_size();
  analyzer_printer_buffer = pcpbuffer_create(analyzer_packet_size, 10);
  logger_buffer = pcpbuffer_create(1024, 5);
  wdog_pack = watchdogpack_create(THREADS_COUNT);

  pthread_create(&tid[0], NULL, watchdog_thread, NULL);
  pthread_create(&tid[1], NULL, reader_thread, NULL);

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);

  (void)logger_buffer;
  (void)analyzer_printer_buffer;
  pcpbuffer_destroy(reader_analyzer_buffer);
  pcpbuffer_destroy(analyzer_printer_buffer);
  pcpbuffer_destroy(logger_buffer);
  watchdogpack_destroy(wdog_pack);
}
