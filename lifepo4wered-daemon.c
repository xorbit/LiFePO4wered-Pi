/* 
 * LiFePO4wered/Pi daemon: communicate with LiFePO4wered/Pi to provide
 * proper boot and shutdown behavior
 *
 * Copyright (C) 2015 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _XOPEN_SOURCE

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "lifepo4wered-data.h"


/* Running flag */

volatile sig_atomic_t running;

/* TERM signal handler */

void term_handler(int signum)
{
    running = 0;
}

/* Set up TERM signal handler */

void set_term_handler(void) {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = term_handler;
  sigaction(SIGTERM, &action, NULL);
}

/* Shut down the system */

void shutdown(void) {
  //char *params[4] = {"shutdown", "-h", "now", NULL};
  char *params[3] = {"init", "0", NULL};
  execv("/sbin/init", params);
}

int main(int argc, char *argv[]) {
  /* Fork and detach to run as daemon */
  daemon(0, 0);

  /* Set LiFePO4wered/Pi running flag */
  write_lifepo4wered(PI_RUNNING, 1);
  running = 1;

  /* Set handler for TERM signal */
  set_term_handler();

  /* Sleep while the Pi is on, until this daemon gets a signal
   * to terminate (which might be because the LiFePO4wered/Pi
   * running flag is reset */
  while (running) {
    /* Start shutdown if the LiFePO4wered/Pi running flag is reset */
    if (read_lifepo4wered(PI_RUNNING) == 0)
      shutdown();
    
    /* Sleep most of the time */
    sleep(1);
  }

  /* Tell the LiFePO4wered/Pi we're shutting down */
  write_lifepo4wered(PI_RUNNING, 0);

  return 1;
}
