#include "../inc/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

struct Logger
{
  FILE* f;
  enum LogType level;
  char pad[4];
};

char* datetime_to_str(void)
{
  char* datetime_str = malloc(50);

  if (datetime_str == NULL)
    return NULL;
  
  const time_t now = time(NULL);
  struct tm* t = localtime(&now);

  strftime(datetime_str, 49, "%d-%m-%Y %H:%M:%S", t);
  return datetime_str;
}

static const char* logtype_to_str(register const enum LogType type) {
  if (type < LOGTYPE_ERROR || type > LOGTYPE_DEBUG)
    return "????";
  // Jump table (sort of)
  const char* restrict logtype_str[] = { "ERROR", "INFO", "DEBUG" };
  return logtype_str[type];
}

static void print_log_prefix(register const Logger* const logger, register const enum LogType type) {
  char* datetime_str = datetime_to_str();
  fprintf(logger->f, "[%s][%s]", logtype_to_str(type), datetime_str);
  free(datetime_str);
}

Logger* logger_create(register const char* const path, enum LogType level)
{
  Logger* const logger = malloc(sizeof(*logger));
  
  if (logger == NULL)
    return NULL;

  // Create ./log dir if it doesn't exist

  struct stat st = {0};
  if (stat("./log", &st) == -1)
    mkdir("./log", 0700);

  *logger = (Logger){.f = fopen(path, "a+"),
                     .level = level,
                    };
  return logger;
}

void logger_set_level(Logger* const logger, register const enum LogType level) {
  logger->level = level;
}

void logger_log(register const Logger* const logger, register const enum LogType level, register const char* const msg) {
  if (logger->level < level)
    return;

  print_log_prefix(logger, level);
  fprintf(logger->f, "%s", msg);
  fflush(logger->f);
}

void logger_destroy(Logger* const logger)
{
  if (logger == NULL)
    return;
  
  if (logger->f != NULL)
    fclose(logger->f);
  free(logger);
}
