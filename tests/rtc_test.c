#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "lifepo4wered-data.h"
static time_t host_time=1792617192;
static int32_t rtc;
static int reads,sets,writes,mode;
static time_t fake_time(time_t *out) {if(out)*out=host_time;return host_time;}
static int fake_clock(clockid_t id,const struct timespec *ts) {assert(id==CLOCK_REALTIME);assert(ts->tv_sec>=1577836800);sets++;return 0;}
static int fake_sleep(const struct timespec *a,struct timespec *b) {(void)a;(void)b;return 0;}
bool access_lifepo4wered(enum eLiFePO4weredVar v,uint8_t m) {(void)v;(void)m;return mode!=4;}
int32_t read_lifepo4wered(enum eLiFePO4weredVar v) {assert(v==RTC_TIME);reads++;if(mode==2 && reads>1)return -2;return rtc+((mode==1 && reads>1)?1:0);}
int32_t write_lifepo4wered(enum eLiFePO4weredVar v,int32_t val) {(void)v;(void)val;writes++;return val;}
#define time fake_time
#define clock_settime fake_clock
#define nanosleep fake_sleep
#define main upstream_main
#undef _DEFAULT_SOURCE
#undef _XOPEN_SOURCE
#include "lifepo4wered-daemon.c"
#undef main
static void reset(int m,int32_t r) {mode=m;rtc=r;reads=sets=writes=0;host_time=1792617192;}
int main(void) {
 reset(0,20467741);system_time_from_rtc();assert(sets==0 && reads==1);
 reset(0,-2);system_time_from_rtc();assert(sets==0 && reads==1);
 reset(1,1792616000);system_time_from_rtc();assert(sets==1 && reads==2);
 reset(0,1792617192);system_time_from_rtc();assert(sets==0 && reads==1);
 reset(0,1792616000);system_time_from_rtc();assert(sets==0 && reads==41);
 reset(2,1792616000);system_time_from_rtc();assert(sets==0 && reads==2);
 reset(4,1792616000);system_time_from_rtc();assert(sets==0 && reads==0);
 reset(0,1792616000);host_time=20467741;system_time_to_rtc();assert(writes==0);
 reset(0,1792616000);host_time=2200000000LL;system_time_to_rtc();assert(writes==0);
 puts("RTC_INVALID_VALID_STALLED_IO_ERROR_AND_SAVE_GUARDS_OK");return 0;
}
