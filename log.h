#ifndef NNTPSWITCHD_LOG_H
# define NNTPSWITCHD_LOG_H
/* $Id: log.h,v 1.2 2006-09-18 12:28:27 mjo Exp $ */

typedef struct syslog_status {
	int is_open; /* tracks syslog_open/syslog_close */
	const char *prefix;
	int option;
	int default_facility;
} syslog_status;

/* Closes syslog if it had been opened earlier with syslog_open.
// Returns previous status
*/ 
extern syslog_status syslog_close ();

/* Changes the prefix/option/facility with which syslog operates.
// Ensures closelog() gets called which seems to be necessary on Solaris.
// TODO check whether prefix is allowed to be NULL.
*/
extern void syslog_open (const char *prefix, int option, int default_facility);

extern syslog_status syslog_status_get ();
extern void syslog_status_set (syslog_status sst);

#endif
