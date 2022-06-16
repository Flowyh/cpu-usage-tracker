#include "../inc/watchdog.h"
#include "unittest.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

static void watchdog_expiration_test(void)
{
  // EXPIRATION NULL CHECK
  assert(watchdog_is_alarm_expired(NULL) == -1);
  // CREATE WATCHDOG
  Watchdog* wdog = watchdog_create(pthread_self(), 2);
  assert(wdog != NULL);
  // NOT YET EXPIRED TEST
  sleep(1);
  assert(watchdog_is_alarm_expired(wdog) == 0);
  printf("Is expired after 1 second: %d\n", watchdog_is_alarm_expired(wdog));
  // EXPIRED TEST
  sleep(1);
  assert(watchdog_is_alarm_expired(wdog) == 1);
  printf("Is expired after 2 seconds: %d\n", watchdog_is_alarm_expired(wdog));
  // DESTROY
  watchdog_destroy(wdog);
}

static void watchdog_snoozing_test(void)
{
  // CREATE WATCHDOG
  Watchdog* wdog = watchdog_create(pthread_self(), 2);
  assert(wdog != NULL);
  // EXPIRED TEST
  sleep(2);
  assert(watchdog_is_alarm_expired(wdog) == 1);
  printf("Is expired after 2 seconds: %d\n", watchdog_is_alarm_expired(wdog));
  // SNOOZING TEST
  watchdog_snooze(wdog);
  assert(watchdog_is_alarm_expired(wdog) == 0);
  printf("Is expired after snoozing: %d\n", watchdog_is_alarm_expired(wdog));
  // DESTROY
  watchdog_destroy(wdog);
}

static void watchdogpack_test(void)
{
  // CREATE PACK
  WatchdogPack* pack = watchdogpack_create(0);
  assert(pack == NULL);
  pack = watchdogpack_create(3);
  assert(pack != NULL);
  assert(watchdogpack_get_size(pack) == 3);
  // REGISTER TEST
  Watchdog* dog1 = watchdog_create(pthread_self(), 2);
  size_t dog1_id = watchdogpack_register(pack, dog1);
  assert(watchdogpack_get_registered(pack) == 1);

  Watchdog* dog2 = watchdog_create(pthread_self(), 3);
  size_t dog2_id = watchdogpack_register(pack, dog2);
  assert(watchdogpack_get_registered(pack) == 2);
  
  Watchdog* dog3 = watchdog_create(pthread_self(), 1);
  size_t dog3_id = watchdogpack_register(pack, dog3);
  assert(watchdogpack_get_registered(pack) == 3);
  // ALARMS CHECK TEST
  size_t res = watchdogpack_check_alarms(pack);
  assert(res == watchdogpack_get_size(pack) + 1);
  printf("Dog with id %zu barked.\n", res);
  
  sleep(1);
  res = watchdogpack_check_alarms(pack);
  printf("Dog with id %zu barked.\n", res);
  assert(res == 2);
  
  sleep(2);
  res = watchdogpack_check_alarms(pack);
  printf("Dog with id %zu barked.\n", res);
  assert(res == 0);
  // UNREGISTER TEST
  watchdogpack_unregister(pack, dog1_id);
  assert(watchdogpack_get_registered(pack) == 2);
  watchdogpack_unregister(pack, dog3_id);
  assert(watchdogpack_get_registered(pack) == 1);
  watchdogpack_unregister(pack, dog2_id);
  assert(watchdogpack_get_registered(pack) == 0);
  // DESTROY
  watchdogpack_destroy(pack);
  watchdog_destroy(dog1);
  watchdog_destroy(dog2);
  watchdog_destroy(dog3);
}

void watchdog_tests(void)
{
  RUN_TEST(watchdog_expiration_test);
  RUN_TEST(watchdog_snoozing_test);
  RUN_TEST(watchdogpack_test);
}