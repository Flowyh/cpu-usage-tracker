#ifndef THREADS_ROUTINES_H
#define THREADS_ROUTINES_H

#include "../inc/reader.h"
#include "../inc/watchdog.h"
#include "../inc/pcp_buffer.h"
#include <unistd.h>

void signal_exit(void);
void sigterm_graceful_exit(int signum);
void* watchdog_thread(void* arg);
void* reader_thread(void* arg);

void run_threads(void);
// void* analyzer_thread(void* arg);
// void* printer_thread(void* arg);
// void* logger_thread(void* arg);

#endif // !THREADS_ROUTINES_H