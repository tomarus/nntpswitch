/*
 * $Id: acct_daylimit.c,v 1.3 2010-11-23 13:45:23 tommy Exp $
 * (c) 2006 News-Service Europe bv
 */

#include "../aconfig.h"
#include "../log.h"
#include "../nntpd.h"
#include "../aprotos.h"
#include "log_acct.h"
#include <postgresql/libpq-fe.h>

#define NSE_ACCT_MODULE "acct_daylimit"

PROTO void acct_daylimit(CLIENT *client, CONFIG *cfg, char *args)
{
	syslog_status sst = syslog_status_get ();
	syslog_open ("nntpswitchd", LOG_PID, LOG_AUTH);
        NSE_ACCT_LOG (NSE_ACCT_MODULE, client);
	syslog_status_set (sst);

        do_acct_daylimit(client, cfg, args);
}

