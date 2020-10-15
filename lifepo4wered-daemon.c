/* 
 * LiFePO4wered/Pi daemon: communicate with LiFePO4wered/Pi to provide
 * proper boot and shutdown behavior
 *
 * Copyright (C) 2015-2020 Patrick Van Oosterwijck
 * Released under the GPL v2
 */

#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lifepo4wered-data.h"

#ifdef SYSTEMD
#include <systemd/sd-daemon.h>
#endif

/* Time difference (s) for the system time to be updated from the RTC */

#define RTC_SET_DIFF    10

/* Delay (ns) between attempts to check for RTC second rollover */

#define RTC_CHECK_DELAY 50000000

/* Running flag */

volatile sig_atomic_t running;

/* Running in foreground flag */
bool foreground = false;

#define log_info(args...) do { \
  if (foreground) {\
    fprintf(stdout, args); \
    fprintf(stdout, "\n"); \
    fflush(stdout); \
  } else \
    syslog(LOG_INFO, args); \
} while (0)

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

void shut_down(void) {
  log_info("Triggering system shutdown");
#ifdef BALENA
  char *params[4] = {"sh", "-c", "curl -X POST " \
    "\"$BALENA_SUPERVISOR_ADDRESS/v1/shutdown?" \
    "apikey=$BALENA_SUPERVISOR_API_KEY\"", NULL};
  execv("/bin/sh", params);
#else
  char *params[3] = {"init", "0", NULL};
  execv("/sbin/init", params);
#endif
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
    struct timespec new_ts = {0};
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = RTC_CHECK_DELAY;
    int32_t start_time = read_lifepo4wered(RTC_TIME);
    do {
      nanosleep(&ts, NULL);
      new_ts.tv_sec = read_lifepo4wered(RTC_TIME);
    } while(new_ts.tv_sec == start_time);
    /* Set the system time to the RTC time */
    clock_settime(CLOCK_REALTIME, &new_ts);
    /* Log message */
    log_info("System time restored from RTC: %li", new_ts.tv_sec);
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
  log_info("System time saved to RTC: %d", (int32_t)now_time);
}

/* Main program */

int main(int argc, char *argv[]) {
  bool trigger_shutdown = false;

#ifdef SYSTEMD
  sd_notify(0, "STATUS=Startup");
#endif
  /* Run in foreground if -f flag is passed */
  if (argc == 2 && strcmp(argv[1], "-f") == 0)
    foreground = true;
  /* Otherwise fork and detach to run as daemon */
  else if (daemon(0, 0))
    return 1;

  /* Open the syslog if we need to */
  if (!foreground)
    openlog("LiFePO4wered", LOG_PID|LOG_CONS, LOG_DAEMON);

  log_info("LiFePO4wered daemon started");

  /* Set handler for TERM signal */
  set_term_handler();

  /* Set LiFePO4wered/Pi running flag */
  write_lifepo4wered(PI_RUNNING, 1);
  running = 1;

  /* If available and necessary, restore the system time from the RTC */
  system_time_from_rtc();

#ifdef SYSTEMD
  sd_notify(0, "READY=1");
  sd_notify(0, "STATUS=Active");
#endif

  /* Sleep while the Pi is on, until this daemon gets a signal
   * to terminate (which might be because the LiFePO4wered/Pi
   * running flag is reset) */
  while (running) {
    /* Start shutdown if the LiFePO4wered/Pi running flag is reset */
    if (read_lifepo4wered(PI_RUNNING) == 0) {
      log_info("Signal from LiFePO4wered module to shut down");
      trigger_shutdown = true;
      running = 0;
    }
    
    /* Sleep most of the time */

#ifdef SYSTEMD
    sd_notify(0, "WATCHDOG=1");
#endif
    sleep(1);
  }

#ifdef SYSTEMD
  sd_notify(0, "STOPPING=1");
  sd_notify(0, "STATUS=Shutdown");
#endif

  /* If available, save the system time to the RTC */
  system_time_to_rtc();

  /* Do we need to trigger system shutdown?
   * (The LiFePO4wered/Pi triggered it) */
  if (trigger_shutdown) {
    /* Then trigger a system shutdown */
    shut_down();
  } else {
    /* Otherwise tell the LiFePO4wered/Pi we're shutting down */
    write_lifepo4wered(PI_RUNNING, 0);
    log_info("Signaling LiFePO4wered module that system is shutting down");
  }

  /* Close the syslog */
  closelog();

  return 1;
}
