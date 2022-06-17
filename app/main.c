#include "../inc/threads_routines.h"
#include <signal.h>
#include <string.h>

int main(int argc, char const *argv[])
{
  (void)argc;
  (void)argv;

  // Assign SIGTERM/SIGINT handlers
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigterm_graceful_exit;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  run_threads();
  return 0;
}
