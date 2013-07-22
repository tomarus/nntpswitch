/* $Id: log.c,v 1.3 2006-09-18 12:28:27 mjo Exp $ */

#include <syslog.h>
#include <stdlib.h>

#include "nntpd.h"
#include "log.h"

/* As a reminder - see: man 3 syslog
// openlog (ident, option, default_facility);
// option can be LOG_PID, LOG_PERROR etc.
// default_facility can be LOG_NEWS, LOG_AUTHPRIV etc.
//
// syslog (priority, format, ...);
// priority = facility | level
// level can be LOG_INFO, LOG_ERR etc.
*/

static syslog_status syslog_status_current = {
	0, "nntpswitchd", LOG_PID, LOG_NEWS
};

syslog_status syslog_status_get ()
{
	return syslog_status_current;
}

void syslog_status_set (syslog_status sst)
{
	if (sst.is_open)
		syslog_open (sst.prefix, sst.option, sst.default_facility);
	else
		syslog_status_current = sst;
}

/* Closes syslog if it had been opened earlier with syslog_open.
// Returns previous status.
*/ 
syslog_status syslog_close ()
{
	syslog_status sst = syslog_status_get ();

	if (! syslog_status_current.is_open) return sst;

	syslog_status_current.is_open = 0;
	closelog ();
	return sst;
}

/* Changes the prefix/option/facility with which syslog operates
// and remembers the previous prefix, option and facility.
// Ensures closelog() gets called which seems to be necessary on Solaris.
// TODO check whether prefix is allowed to be NULL.
*/
void syslog_open (const char *prefix, int option, int default_facility)
{
	syslog_close ();

	openlog (prefix, option, default_facility);

	syslog_status_current.is_open = 1;
	syslog_status_current.prefix = prefix;
	syslog_status_current.option = option;
	syslog_status_current.default_facility = default_facility;
}

