/*
 * Timer Process
 * Used for a timer to write statistics to disk. These can be imported on
 * daemon startup to improve on smoother statistics flow.
 * $Id: timerproc.c,v 1.5 2010-07-06 10:55:35 tommy Exp $
 */

#include "aconfig.h"
#include "memory.h"
#include "nntpd.h"
#include "tgetopt.h"
#include "aprotos.h"

#include "log.h"
#include "semaphore.h"
#include "setproctitle.h" 

static void term_handler(int sig)
{
        syslog(LOG_NOTICE, "sigterm received");
	exit(0);
}


static void timer_loop(void)
{
        struct timeval tv;
	int scount = 0;

	for (;;) 
	{
        	tv.tv_sec  = 10;	// cfg.StatsFilePeriod;
	        tv.tv_usec = 0;

		errno = 0;
		if ( (select(0, NULL, NULL, NULL, &tv)) == -1) 
			syslog(LOG_DEBUG, "timer error: errno=%m");
		else  {
			/* each 6 * 10 sec */
			if ( scount++ % 6 == 0 ) {
				cmdstatistics_writestats();
				cmdstatistics_write();
				if ( cfg.JSONRL )
					cmdstatistics_write_bytes_json();
			}
		}
	} 
}

PROTO pid_t run_timer_loop(void)
{
        struct sigaction sigterm;
	pid_t pid;

	info("Launching statistics timer..");

        switch ( (pid=fork()) ) 
	{
        case 0:
                syslog_open ("nntpswitch-timer", LOG_PID, LOG_NEWS);
		setproctitle("nntpswitchd: timer polling each %u seconds", cfg.StatsFilePeriod );
		memset(&sigterm, 0, sizeof (sigterm));
		sigterm.sa_handler = &term_handler;
		sigaction(SIGTERM, &sigterm, NULL);

                timer_loop();
                syslog_close();	// never reached
                _exit (0);
        case -1:
                die("couldnt fork timer child %s", strerror (errno));
        default:
		// back to parent
		return pid;
        }
}

