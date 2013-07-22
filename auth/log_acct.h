#ifndef NNTPSWITCHD_LOG_ACCT_H
# define NNTPSWITCHD_LOG_ACCT_H

#include <syslog.h>

#include "../nntpd.h"

/* The string for accounting script to match. */
#define NSE_ACCT_PREFIX "acct:"

/* syslog of accounting messages; preserves the syslog status.
// The module argument varies between source files.
*/
#define NSE_ACCT_LOG(module, client) \
		do { \
			syslog_client_usage_md5 (LOG_PID, LOG_AUTH, LOG_WARNING, client \
					, NSE_ACCT_PREFIX module); \
		} while (0)

/* Uses syslog to log the client's byte usage.
// Restores previous syslog settings.
// Note that restoring only works if all syslog usage goes
// through syslog_open and syslog_close
// instead of openlog and closelog.
*/
extern void syslog_client_usage_md5 (int option, int facility, int level
		, const CLIENT *client, const char *prefix);

#endif
