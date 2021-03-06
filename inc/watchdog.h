#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <pthread.h>
#include <stdbool.h>

typedef struct Watchdog Watchdog;

Watchdog* watchdog_create(pthread_t tid, double limit, const char* name);
void watchdog_snooze(Watchdog* wdog);
int watchdog_is_alarm_expired(const Watchdog* wdog);
void watchdog_destroy(Watchdog* wdog);
bool watchdog_get_exit_flag(Watchdog* wdog);
void watchdog_enable_exit_flag(Watchdog* wdog);

typedef struct WatchdogPack WatchdogPack;

WatchdogPack* watchdogpack_create(size_t wdogs_number);
size_t watchdogpack_get_registered(const WatchdogPack* wdog_pack);
size_t watchdogpack_get_size(const WatchdogPack* wdog_pack);
const char* watchdogpack_get_dog_name(const WatchdogPack* wdog_pack, size_t wdog_id);
int watchdogpack_register(WatchdogPack* wdog_pack, Watchdog* wdog);
void watchdogpack_unregister(WatchdogPack* wdog_pack, size_t wdog_id);
void watchdogpack_destroy(WatchdogPack* wdog_pack);
int watchdogpack_check_alarms(const WatchdogPack* wdog_pack);
void watchdogpack_panic(WatchdogPack* wdog_pack);

#endif // !WATCHDOG_H
