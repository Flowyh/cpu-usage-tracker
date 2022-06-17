#include "../inc/watchdog.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
struct Watchdog
{
  struct timespec alarm_clock; // 16
  pthread_t id; // 8
  double limit; // 8
  const char* name;
};

Watchdog* watchdog_create(register const pthread_t tid, register const double limit, register const char* const name)
{
  if (limit < 0.0) // Invalid time limit
    return NULL;

  Watchdog* watchdog = malloc(sizeof(*watchdog));
  if (watchdog == NULL)
    return NULL;

  *watchdog = (Watchdog){.id = tid,
                         .limit = limit,
                         .name = name,
                        };
  (void) clock_gettime(CLOCK_MONOTONIC, &watchdog->alarm_clock);

  return watchdog;
}

void watchdog_snooze(Watchdog* wdog) {
  if (wdog == NULL)
    return; // Dog doesn't exist

  pthread_t self_id = pthread_self();
  if (wdog->id != self_id)
    return; // Snoozing wrong dog

  (void) clock_gettime(CLOCK_MONOTONIC, &wdog->alarm_clock);
}

static double timespec_to_seconds(register const struct timespec ts)
{
  return (double)(ts.tv_sec) + (double)(ts.tv_nsec) * 1e-9;
}

// static double watchdog_get_alarm_timestamp(const Watchdog* wdog)
// {
//   if (wdog == NULL)
//     return -1;

//   return timespec_to_seconds(wdog->alarm_clock);
// }

int watchdog_is_alarm_expired(register const Watchdog* const wdog)
{
  if (wdog == NULL)
    return -1;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  double diff = timespec_to_seconds(now) - timespec_to_seconds(wdog->alarm_clock);
  return (diff) >= wdog->limit; 
}

void watchdog_destroy(Watchdog* wdog)
{
  free(wdog);
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
  
  WatchdogPack* wdog_pack;

  wdog_pack = malloc(sizeof(wdog_pack) + (wdogs_number + 1) * sizeof(Watchdog*));

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

int watchdogpack_register(WatchdogPack* wdog_pack, register const Watchdog* const wdog)
{
  if (wdog_pack->registered >= wdog_pack->size)
    return -1;
  wdog_pack->pack[wdog_pack->registered++] = (Watchdog*) wdog;
  return wdog_pack->registered - 1;
}

void watchdogpack_unregister(WatchdogPack* wdog_pack, size_t wdog_id)
{
  if (wdog_pack->registered == 0)
    return;
  
  if (wdog_pack->pack[wdog_id] != NULL)
  {
    wdog_pack->pack[wdog_id] = NULL;
    wdog_pack->registered--;
  }
}

void watchdogpack_destroy(WatchdogPack* wdog_pack)
{
  free(wdog_pack);
}

int watchdogpack_check_alarms(register const WatchdogPack* const wdog_pack)
{ 
  for (size_t i = 0; i < wdog_pack->registered; i++)
  {
    if (wdog_pack->pack[i] != NULL && watchdog_is_alarm_expired(wdog_pack->pack[i]) == 1)
    {
      return i;
    }
  }
  return -1;
}