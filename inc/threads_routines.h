#ifndef THREADS_ROUTINES_H
#define THREADS_ROUTINES_H

#include "../inc/reader.h"
#include "../inc/watchdog.h"
#include "../inc/pcp_buffer.h"
#include "../inc/analyzer.h"
#include "../inc/logger.h"
#include "../inc/printer.h"
#include <unistd.h>

void signal_exit(void);
void sigterm_graceful_exit(int signum);
void run_threads(void);

#endif // !THREADS_ROUTINES_H
