/* 
 * LiFePO4wered/Pi daemon: communicate with LiFePO4wered/Pi to provide
 * proper boot and shutdown behavior
 *
 * Copyright (C) 2015-2017 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <time.h>
#include "lifepo4wered-data.h"


/* Time difference (s) for the system time to be updated from the RTC */

#define RTC_SET_DIFF    10

/* Delay (ns) between attempts to check for RTC second rollover */

#define RTC_CHECK_DELAY 50000000

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

  action.sa_handler = term_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGTERM, &action, NULL);
}

/* Shut down the system */

void shutdown(void) {
  syslog(LOG_INFO, "Triggering system shutdown");
  char *params[3] = {"init", "0", NULL};
  execv("/sbin/init", params);
}

/* If the LiFePO4wered module has RTC functionality and the current
 * system time is off more than the limit of time difference, set
 * the system time from the RTC */

void system_time_from_rtc(void) {
  /* Make sure the connected LiFePO4wered module has RTC functionality */
  if (!access_lifepo4wered(RTC_TIME, ACCESS_READ))
    return;
  /* Is the time different enough? */
  if (abs(read_lifepo4wered(RTC_TIME) - (int32_t)time(NULL)) >= RTC_SET_DIFF) {
    /* Wait until the RTC time changes */
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = RTC_CHECK_DELAY;
    int32_t now_time, start_time = read_lifepo4wered(RTC_TIME);
    do {
      nanosleep(&ts, NULL);
      now_time = read_lifepo4wered(RTC_TIME);
    } while(now_time == start_time);
    /* Set the system time to the RTC time */
    stime((time_t *)&now_time);
    /* Log message */
    syslog(LOG_INFO, "System time restored from RTC: %d", now_time);
  }
}

/* If the LiFePO4wered module has RTC functionality, save the current
 * system time to the RTC */

void system_time_to_rtc(void) {
  /* Make sure the connected LiFePO4wered module has RTC functionality */
  if (!access_lifepo4wered(RTC_TIME, ACCESS_WRITE))
    return;
  /* Wait until the system time changes */
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = RTC_CHECK_DELAY;
  time_t now_time, start_time = time(NULL);
  do {
    nanosleep(&ts, NULL);
    now_time = time(NULL);
  } while(now_time == start_time);
  /* Save the system time to the RTC */
  write_lifepo4wered(RTC_TIME, (int32_t)now_time);
  /* Log message */
  syslog(LOG_INFO, "System time saved to RTC: %d", (int32_t)now_time);
}

/* Main program */

int main(int argc, char *argv[]) {
  bool trigger_shutdown = false;

  /* Fork and detach to run as daemon */
  if (daemon(0, 0))
    return 1;

  /* Open the syslog */
  openlog("LiFePO4wered", LOG_PID|LOG_CONS, LOG_DAEMON);
  syslog(LOG_INFO, "LiFePO4wered daemon started");

  /* Set handler for TERM signal */
  set_term_handler();

  /* Set LiFePO4wered/Pi running flag */
  write_lifepo4wered(PI_RUNNING, 1);
  running = 1;

  /* If available and necessary, restore the system time from the RTC */
  system_time_from_rtc();

  /* Sleep while the Pi is on, until this daemon gets a signal
   * to terminate (which might be because the LiFePO4wered/Pi
   * running flag is reset) */
  while (running) {
    /* Start shutdown if the LiFePO4wered/Pi running flag is reset */
    if (read_lifepo4wered(PI_RUNNING) == 0) {
      syslog(LOG_INFO, "Signal from LiFePO4wered module to shut down");
      trigger_shutdown = true;
      running = 0;
    }
    
    /* Sleep most of the time */
    sleep(1);
  }

  /* If available, save the system time to the RTC */
  system_time_to_rtc();

  /* Tell the LiFePO4wered/Pi we're shutting down */
  write_lifepo4wered(PI_RUNNING, 0);
  syslog(LOG_INFO, "Signaling LiFePO4wered module that system is shutting down");

  /* If we need to trigger a shutdown, do it now */
  if (trigger_shutdown)
    shutdown();

  /* Close the syslog */
  closelog();

  return 1;
}
