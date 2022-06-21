#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>

typedef struct Logger Logger;

enum LogType {
  LOGTYPE_ERROR,
  LOGTYPE_INFO,
  LOGTYPE_DEBUG,
};

Logger* logger_create(const char* path, enum LogType level);
void logger_set_level(Logger*, enum LogType level);
void logger_log(const Logger*, enum LogType level, const char* msg);
void logger_destroy(Logger* logger);
char* datetime_to_str(void);

#endif // !LOGGER_H
