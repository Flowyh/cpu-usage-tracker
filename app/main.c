#include "../inc/threads_routines.h"
#include <signal.h>
#include <string.h>

#define VER 0.1

int main(int argc, char const *argv[])
{
  (void)argc;
  (void)argv;

  printf("==========CPU usage tracker v%.1f=========\n", VER);

  // Assign SIGTERM/SIGINT handlers
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigterm_graceful_exit;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  run_threads();
  return 0;
}
