#include "../inc/watchdog.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

struct Watchdog
{
  pthread_mutex_t exit_flag_mutex;
  struct timespec alarm_clock;
  pthread_t id;
  double limit;
  bool exit_flag;
  char pad[7];
  const char* name;
};

Watchdog* watchdog_create(register const pthread_t tid, register const double limit, register const char* const name)
{
  if (limit < 0.0) // Invalid time limit
    return NULL;

  Watchdog* const watchdog = malloc(sizeof(*watchdog));
  
  if (watchdog == NULL)
    return NULL;

  *watchdog = (Watchdog){.id = tid,
                         .limit = limit,
                         .name = name,
                         .exit_flag = false,
                         .exit_flag_mutex = PTHREAD_MUTEX_INITIALIZER,
                        };
  (void) clock_gettime(CLOCK_MONOTONIC, &watchdog->alarm_clock);

  return watchdog;
}

void watchdog_snooze(Watchdog* wdog) {
  if (wdog == NULL)
    return; // Dog doesn't exist

  register const pthread_t self_id = pthread_self();
  if (wdog->id != self_id)
    return; // Snoozing wrong dog

  (void) clock_gettime(CLOCK_MONOTONIC, &wdog->alarm_clock);
}

static double timespec_to_seconds(register const struct timespec ts)
{
  return (double)(ts.tv_sec) + (double)(ts.tv_nsec) * 1e-9;
}

int watchdog_is_alarm_expired(register const Watchdog* const wdog)
{
  if (wdog == NULL)
    return -1;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  double diff = timespec_to_seconds(now) - timespec_to_seconds(wdog->alarm_clock);
  return (diff) >= wdog->limit; 
}

void watchdog_destroy(Watchdog* const wdog)
{
  if (wdog == NULL)
    return;
  
  pthread_mutex_destroy(&wdog->exit_flag_mutex);
  free(wdog);
}

bool watchdog_get_exit_flag(Watchdog* const wdog)
{
  if (wdog == NULL)
    return false;

  bool exit_flag;
  pthread_mutex_lock(&wdog->exit_flag_mutex);
  exit_flag = wdog->exit_flag;
  pthread_mutex_unlock(&wdog->exit_flag_mutex);
  return exit_flag;
}

void watchdog_enable_exit_flag(Watchdog* const wdog)
{
  if (wdog == NULL)
    return;

  pthread_mutex_lock(&wdog->exit_flag_mutex);
  wdog->exit_flag = true;
  pthread_mutex_unlock(&wdog->exit_flag_mutex);
}

struct WatchdogPack
{
  size_t size;
  size_t registered;
  Watchdog* pack[]; // FAM
};

WatchdogPack* watchdogpack_create(register const size_t wdogs_number)
{
  if (wdogs_number == 0)
    return NULL;
  
  WatchdogPack* const wdog_pack = malloc(sizeof(wdog_pack) + (wdogs_number + 1) * sizeof(Watchdog*));

  if (wdog_pack == NULL)
    return NULL;

  *wdog_pack = (WatchdogPack){.size = wdogs_number,
                              .registered = 0,
                             };

  for (size_t i = 0; i < wdogs_number; i++)
    wdog_pack->pack[i] = NULL;

  return wdog_pack;
}

size_t watchdogpack_get_registered(register const WatchdogPack* const wdog_pack) {
  return wdog_pack->registered;
}

size_t watchdogpack_get_size(register const WatchdogPack* const wdog_pack) {
  return wdog_pack->size;
}

const char* watchdogpack_get_dog_name(register const WatchdogPack* const wdog_pack, register const size_t wdog_id)
{
  if (wdog_pack->pack[wdog_id] != NULL)
    return wdog_pack->pack[wdog_id]->name;
  return NULL;
}

int watchdogpack_register(WatchdogPack* const wdog_pack, Watchdog* const wdog)
{
  if (wdog_pack->registered >= wdog_pack->size)
    return -1;
  wdog_pack->pack[wdog_pack->registered++] = wdog;
  return (int)(wdog_pack->registered - 1);
}

void watchdogpack_unregister(WatchdogPack* const wdog_pack, size_t wdog_id)
{
  if (wdog_pack->registered == 0)
    return;
  
  if (wdog_pack->pack[wdog_id] != NULL)
  {
    wdog_pack->pack[wdog_id] = NULL;
    wdog_pack->registered--;
  }
}

void watchdogpack_destroy(WatchdogPack* const wdog_pack)
{
  if (wdog_pack == NULL)
    return;

  for (size_t i = 0; i < wdog_pack->registered; i++)
    if (wdog_pack->pack[i] != NULL)
      watchdog_destroy(wdog_pack->pack[i]);
  free(wdog_pack);
}

int watchdogpack_check_alarms(register const WatchdogPack* const wdog_pack)
{ 
  for (size_t i = 0; i < wdog_pack->registered; i++)
  {
    if (wdog_pack->pack[i] != NULL)
      if (wdog_pack->pack[i]->exit_flag || watchdog_is_alarm_expired(wdog_pack->pack[i]) == 1)
        return (int) i;
  }
  return -1;
}

void watchdogpack_panic(WatchdogPack* const wdog_pack)
{
  if (wdog_pack == NULL)
    return;
  
  for (size_t i = 0; i < wdog_pack->registered; i++)
    if (wdog_pack->pack[i] != NULL)
      watchdog_enable_exit_flag(wdog_pack->pack[i]);
}
