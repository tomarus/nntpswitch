/*
 * Timelimit functions, mainly the parser.
 * (c) 2011 News-Service Europe bv
 */

#include "../aconfig.h"
#include "../log.h"
#include "../nntpd.h"
#include "../aprotos.h"

static const int daylen = 60*24;


/*
 * Check for valid format in the form "HH:MM"
 */
static bool timeformatok(const char *time)
{
    if (
        (time[0]<0 && time[0]>2)  ||
        (time[1]<0 && time[1]>9)  ||
        (time[2] != ':')          ||
        (time[3]<0 && time[2]>5)  ||
        (time[4]<0 && time[3]>9) 
       ) return false;
    return true;
}


static bool timebetweenms(char *starttime, char *endtime, int nowhr, int nowmin, int *timeleft)
{
    int starthr  = atoi(starttime);
    int startmin = atoi(starttime+3);
    int startsec = (starthr*60) + startmin;

    int endhr  = atoi(endtime);
    int endmin = atoi(endtime+3);
    int endsec = (endhr*60) + endmin;

    int nowsec = (nowhr*60) + nowmin;

    if ( startsec > endsec ) {
        endsec += daylen;
        if ( nowsec < startsec ) { 
            nowsec += daylen;
        }
    }

    /* Calculate if we are in between the period, and 
       set timeleft to seconds left until next period. */
    if ( startsec <= nowsec && endsec >= nowsec ) {
        *timeleft = (endsec - nowsec) * 60;
        if ( *timeleft == 0 ) *timeleft = 1;
        return true;
    } else {
        if ( startsec >= nowsec ) {
            *timeleft = (startsec - nowsec) * 60;
        } else {
            *timeleft = (daylen - nowsec + startsec) * 60;
        }
        if ( *timeleft == 0 ) *timeleft = 1;
        return false;
    }
}


/*
 * Calculate if /now/ lies between starttime and endtime.
 *
 * @param string starttime XX:XX
 * @param string endtime XX:XX
 * @param epoch now
 * @param ptr to int timeleft
 * @return bool true if in between, false if outside or on errors
 */
PROTO bool timebetween(char *starttime, char *endtime, time_t now, int *timeleft)
{
    if ( ! (timeformatok(starttime) && timeformatok(endtime)) )
        return false;

    struct tm *tmp;
    tmp = localtime(&now);
    if ( tmp == NULL ) {
        syslog(LOG_ERR, "Can't localtime()");
        return false;
    }

    return timebetweenms(starttime, endtime, tmp->tm_hour, tmp->tm_min, timeleft);
}


PROTO void test_timebetween(void)
{
    printf("Starting timebetween test.\n");
    if ( ! timeformatok("00:00") )  /* must be valid time */
        printf("00:00 NOT OK\n");
    if ( ! timeformatok("23:59") )  /* must be valid time */
        printf("23:59 NOT OK\n");
    if ( timeformatok("53:92") )    /* must not be validated */
        printf("53:92 NOT OK\n");

    int left;

    char * start1 = "03:00";
    char * end1   = "19:00";
    printf("Testing between 03:00-19:00\n");
    for(int i=0; i<24; i++) {
      for(int j=0; j<60; j+=5) {
        if ( timebetweenms(start1, end1, i, j, &left) )    /* 30min over XX:30 */
            printf("hour: %.2d:%.2d BETWEEN left %d\n", i, j, left);
        else
            printf("hour: %.2d:%.2d OUTSIDE left %d\n", i, j, left);
      }
    }

    char * start2 = "10:25";
    char * end2   = "10:45";
    printf("Testing between 20:00-07:00\n");
    for(int i=0; i<24; i++) {
      for(int j=0; j<60; j+=5) {
        if ( timebetweenms(start2, end2, i, j, &left) )    /* 30min over XX:30 */
            printf("hour: %.2d:%.2d BETWEEN left %d\n", i, j, left);
        else
            printf("hour: %.2d:%.2d OUTSIDE left %d\n", i, j, left);
      }
    }
    
    printf("Timebetween test finished.\n");
}

