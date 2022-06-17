#include "../inc/threads_routines.h"
#include "../inc/procstat_wrapper.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>


#define MAIN_BUFFERS_LIMIT 10
#define LOGGER_BUFFER_LIMIT 50
#define LOGGER_PACKET_SIZE 1024
#define THREADS_COUNT 5
#define WATCHDOG_LIMIT 2
#define WATCHDOG_SLEEP 1
#define READER_SLEEP 1
#define ANALYZER_SLEEP 1

#define logger_put(buffer, type, size, __format) \
    do { \
      snprintf(buffer, size, __format); \
      logger_put_to_buffer(buffer, type); \
      memset(buffer, 0, size); \
    } while(0)

#define logger_put_args(buffer, type, size, __format, ...) \
    do { \
      snprintf(buffer, size, __format, __VA_ARGS__); \
      logger_put_to_buffer(buffer, type); \
      memset(buffer, 0, size); \
    } while(0)


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
// Reader func
static uint8_t* reader_read_proc_stat(register const Reader* const reader);

void signal_exit(void)
{
  pcpbuffer_wake_consumer(reader_analyzer_buffer);
  pcpbuffer_wake_producer(reader_analyzer_buffer);
  pcpbuffer_wake_consumer(analyzer_printer_buffer);
  pcpbuffer_wake_producer(analyzer_printer_buffer);
  pcpbuffer_wake_consumer(logger_buffer);
  pcpbuffer_wake_producer(logger_buffer);
  exit_flag = true;
}

// Buffer should be padded to LOGGER_PACKET_SIZE
void logger_put_to_buffer(char buffer[], const enum LogType level)
{
  uint8_t* packet = malloc(LOGGER_PACKET_SIZE);
  packet[0] = (uint8_t) level;
  memcpy(&packet[1], &buffer[0], LOGGER_PACKET_SIZE - 1);
  pcpbuffer_lock(logger_buffer);
  if (pcpbuffer_is_full(logger_buffer))
  {
    pcpbuffer_wait_for_consumer(logger_buffer);
  }
  pcpbuffer_put(logger_buffer, packet, LOGGER_PACKET_SIZE);
  pcpbuffer_wake_consumer(logger_buffer);
  pcpbuffer_unlock(logger_buffer);
  free(packet);
}

void sigterm_graceful_exit(int signum)
{
  char buffer[LOGGER_PACKET_SIZE];
  logger_put_args(buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "Signal caught! Signum: %d.\n", signum);
  printf("Signal caught! Signum. %d\n", signum);
  signal_exit();
}

static size_t reader_get_packet_size(void)
{
  size_t cores = sysconf(_SC_NPROCESSORS_ONLN);
  return (sizeof(ProcStatWrapper)) * (cores + 1);
}

static size_t analyzer_get_packet_size(void)
{
  size_t cores = sysconf(_SC_NPROCESSORS_ONLN);
  return (sizeof(AnalyzerPacket) * (cores + 1));
}

static uint8_t* reader_read_proc_stat(register const Reader* const reader)
{
  ProcStatWrapper* stats_wrapper = procstatwrapper_create();
  uint8_t* packet = malloc(reader_packet_size);
  size_t one_core = sizeof(ProcStatWrapper);

  int fscan_result;
  for (size_t i = 0; i < cpu_cores + 1; i++)
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

    // procstatwrapper_print(stats_wrapper);

    (void)fscan_result;
    memcpy(&packet[one_core * i], &stats_wrapper[0], one_core);
  }
  procstatwrapper_destroy(stats_wrapper);
  return packet;
}

void* reader_thread(void* arg)
{
  (void)(arg);
  Reader* proc_stat_reader = reader_create("/proc/stat", READER_SLEEP);
  char log_buffer[LOGGER_PACKET_SIZE];
  

  if (proc_stat_reader == NULL)
    return NULL;

  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Creating watchdog.\n");

  pthread_t tid = pthread_self();
  Watchdog* wdog = watchdog_create(tid, WATCHDOG_LIMIT, "Reader");

  if (wdog == NULL)
    return NULL;

  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Registering watchdog.\n");

  int wdog_register_status = watchdogpack_register(wdog_pack, wdog);
  
  if (wdog_register_status == -1)
    return NULL;


  while(true)
  {
    if (exit_flag)
      break;

    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[READER] Snoozing watchdog.\n");

    watchdog_snooze(wdog);

    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[READER] Rewinding reader.\n");

    reader_rewind(proc_stat_reader);
    if (proc_stat_reader->f == NULL)
    { 
      logger_put_args(log_buffer, LOGTYPE_ERROR, LOGGER_PACKET_SIZE, "[READER] Error while reopening %s file.\n", proc_stat_reader->path);
      break;
    }

    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Reading from file.\n");

    uint8_t* packet = reader_read_proc_stat(proc_stat_reader);
    
    pcpbuffer_lock(reader_analyzer_buffer);
    if (pcpbuffer_is_full(reader_analyzer_buffer))
    {
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Buffer is full, waiting for consumer.\n");
      pcpbuffer_wait_for_consumer(reader_analyzer_buffer);
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Consumer signaled. Continuing.\n");
    }
    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Sending packet.\n");
    pcpbuffer_put(reader_analyzer_buffer, packet, reader_packet_size);
    pcpbuffer_wake_consumer(reader_analyzer_buffer);
    pcpbuffer_unlock(reader_analyzer_buffer);

    free(packet);
    logger_put_args(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[READER] Sleeping for %zu.\n", proc_stat_reader->read_interval);
    sleep(proc_stat_reader->read_interval);
  }
  
  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[READER] Exit signaled. Exitting.\n");
  printf("[READER] Exit signaled. Exitting.\n");
  reader_destroy(proc_stat_reader);
  watchdogpack_unregister(wdog_pack, wdog_register_status);
  watchdog_destroy(wdog);
  return NULL;
}

void* watchdog_thread(void* arg)
{
  (void)arg;
  int alarm_check;
  char log_buffer[LOGGER_PACKET_SIZE];

  while(true)
  {
    if (exit_flag)
      break;

    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[WATCHDOG] Checking alarms.\n");
    
    alarm_check = watchdogpack_check_alarms(wdog_pack);
    if (alarm_check != -1)
    {;
      logger_put_args(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, 
        "[WATCHDOG] A dog with name %s has barked.\n", watchdogpack_get_dog_name(wdog_pack, alarm_check));
      break;
    }
    sleep(WATCHDOG_SLEEP);
  }
  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[WATCHDOG] Exit signaled. Exitting.\n");
  printf("[WATCHDOG] Exit signaled. Exitting.\n");
  signal_exit();
  return NULL;
}

void* analyzer_thread(void* arg)
{
  (void)arg;
  char log_buffer[LOGGER_PACKET_SIZE];

  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Creating watchdog.\n");

  pthread_t tid = pthread_self();
  Watchdog* wdog = watchdog_create(tid, WATCHDOG_LIMIT, "Analyzer");

  if (wdog == NULL)
    return NULL;

  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Registering watchdog.\n");

  int wdog_register_status = watchdogpack_register(wdog_pack, wdog);
  
  if (wdog_register_status == -1)
    return NULL;

  size_t core_info_size = sizeof(ProcStatWrapper);
  size_t analyzed_core_size = sizeof(AnalyzerPacket);

  uint8_t* prev = malloc(reader_packet_size);
  bool prev_set = false;

  while(true)
  {
    if (exit_flag)
      break;

    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[ANALYZER] Snoozing watchdog.\n");

    watchdog_snooze(wdog);

    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Reading from buffer.\n");

    // Consumer - get packet from reader-analyzer buffer
    pcpbuffer_lock(reader_analyzer_buffer);
    if (pcpbuffer_is_empty(reader_analyzer_buffer))
    {
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Buffer is empty, waiting for producer.\n");
      pcpbuffer_wait_for_producer(reader_analyzer_buffer);
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Producer signaled. Continuing.\n");
    }

    if (exit_flag) // Possible bottleneck 
      break;
    
    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Acquiring packet.\n");
    
    uint8_t* curr = pcpbuffer_get(reader_analyzer_buffer);
    pcpbuffer_wake_producer(reader_analyzer_buffer);
    pcpbuffer_unlock(reader_analyzer_buffer);

    // Analyze read packet
    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[ANALYZER] Analyzing read packet.\n");
    uint8_t* analyzer_packet = malloc(analyzer_packet_size);

    for (size_t i = 0; i < cpu_cores + 1; i++)
    {
      ProcStatWrapper* curr_stats = procstatwrapper_create();
      ProcStatWrapper* prev_stats = procstatwrapper_create();
      memcpy(&curr_stats[0], &curr[i * core_info_size], core_info_size);

      // procstatwrapper_print(curr_stats);

      if (!prev_set)
        memcpy(&prev[0], &curr[0], reader_packet_size);

      memcpy(&prev_stats[0], &prev[i * core_info_size], core_info_size);

      AnalyzerPacket* analyzed_core = analyzer_cpu_usage_packet(prev_stats, curr_stats);
      memcpy(&analyzer_packet[i * analyzed_core_size], &analyzed_core[0], analyzed_core_size); 
      // analyzerpacket_print(analyzed_core);

      analyzerpacket_destroy(analyzed_core);
      procstatwrapper_destroy(curr_stats);
      procstatwrapper_destroy(prev_stats);
    }


    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Sending to next buffer.\n");

    // Producer - put packet in analyzer-printer buffer
    pcpbuffer_lock(analyzer_printer_buffer);
    if (pcpbuffer_is_full(analyzer_printer_buffer))
    {
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Next buffer is full, waiting for consumer.\n");
      pcpbuffer_wait_for_consumer(analyzer_printer_buffer);
      logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Consumer signaled. Continuing.\n");
    }
    logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Sending packet.\n");
    pcpbuffer_put(analyzer_printer_buffer, analyzer_packet, analyzer_packet_size);
    pcpbuffer_wake_consumer(analyzer_printer_buffer);
    pcpbuffer_unlock(analyzer_printer_buffer);
    
    // Copy to previous state
    logger_put(log_buffer, LOGTYPE_DEBUG, LOGGER_PACKET_SIZE, "[ANALYZER] Copying to previous state.\n");
    memcpy(&prev[0], &curr[0], reader_packet_size);
    // One time flag
    prev_set = true;
    free(analyzer_packet);
    free(curr);
  }
  
  logger_put(log_buffer, LOGTYPE_INFO, LOGGER_PACKET_SIZE, "[ANALYZER] Exit signaled. Exitting.\n");
  printf("[ANALYZER] Exit signaled. Exitting.\n");
  free(prev);
  watchdogpack_unregister(wdog_pack, wdog_register_status);
  watchdog_destroy(wdog);
  return NULL;
}

void* logger_thread(void* arg)
{
  Logger* logger = *(Logger**)arg;

  pthread_t tid = pthread_self();
  Watchdog* wdog = watchdog_create(tid, WATCHDOG_LIMIT, "Logger");

  if (wdog == NULL)
    return NULL;

  int wdog_register_status = watchdogpack_register(wdog_pack, wdog);
  
  if (wdog_register_status == -1)
    return NULL;

  while(true)
  {
    if (exit_flag && pcpbuffer_is_empty(logger_buffer))
      break;

    watchdog_snooze(wdog);
    
    // Consumer
    pcpbuffer_lock(logger_buffer);
    if (pcpbuffer_is_empty(logger_buffer))
      pcpbuffer_wait_for_producer(logger_buffer);
    
    if (exit_flag && pcpbuffer_is_empty(logger_buffer))
      break;

    char* curr = (char*) pcpbuffer_get(logger_buffer);
    pcpbuffer_wake_producer(logger_buffer);
    pcpbuffer_unlock(logger_buffer);

    char* buffer = malloc(LOGGER_PACKET_SIZE - 1);
    memcpy(&buffer[0], &curr[1], LOGGER_PACKET_SIZE - 1);

    logger_log(logger, (enum LogType) curr[0], buffer);
    free(curr);
    free(buffer);
    if (pcpbuffer_is_empty(logger_buffer))
      sleep(1);
  }

  printf("LOGGER: Exit signaled. Exitting.\n");
  watchdogpack_unregister(wdog_pack, wdog_register_status);
  watchdog_destroy(wdog);
  return NULL; 
}

void run_threads(void)
{
  // Init statics
  cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
  reader_packet_size = reader_get_packet_size();
  reader_analyzer_buffer = pcpbuffer_create(reader_packet_size, MAIN_BUFFERS_LIMIT);
  analyzer_packet_size = analyzer_get_packet_size();
  analyzer_printer_buffer = pcpbuffer_create(analyzer_packet_size, MAIN_BUFFERS_LIMIT);
  logger_buffer = pcpbuffer_create(LOGGER_PACKET_SIZE, LOGGER_BUFFER_LIMIT);
  wdog_pack = watchdogpack_create(THREADS_COUNT);

  char log_name[256];
  char* datetime_str = datetime_to_str();
  snprintf(log_name, 256, "./log/cpu_usage_tracker_%s.log", datetime_str);
  free(datetime_str);
    
  Logger* logger = logger_create(log_name, LOGTYPE_INFO);

  pthread_create(&tid[0], NULL, watchdog_thread, NULL);
  pthread_create(&tid[1], NULL, reader_thread, NULL);
  pthread_create(&tid[2], NULL, analyzer_thread, NULL);
  pthread_create(&tid[4], NULL, logger_thread, (void*)&logger);

  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);
  pthread_join(tid[4], NULL);

  logger_destroy(logger);
  pcpbuffer_destroy(reader_analyzer_buffer);
  pcpbuffer_destroy(analyzer_printer_buffer);
  pcpbuffer_destroy(logger_buffer);
  watchdogpack_destroy(wdog_pack);
  pthread_cancel(tid[2]);
}
