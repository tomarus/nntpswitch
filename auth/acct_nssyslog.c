/*
 * NNTPSwitch Syslog Accounting Module for NSE
 * $Id: acct_nssyslog.c,v 1.1 2008-05-27 10:14:54 tommy Exp $
 *
 * Parameters from access.conf can be full "facility.priority" or "facility"
 * only. If no parameter is specified it defaults to auth.info.
 *
 */

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"
#include "../log.h"
#include "log_acct.h"

#define NSE_ACCT_MODULE "nssyslog"
#define DEFAULT_FACILITY  LOG_AUTH
#define DEFAULT_PRIORITY  LOG_INFO


PROTO void acct_nssyslog(CLIENT *client, CONFIG *cfg, char *args)
{
    char fac[32], prio[32];
    char addr[INET_ADDRSTRLEN];

    int ifac  = DEFAULT_FACILITY;
    int iprio = DEFAULT_PRIORITY;

    fac[0] = 0;
    prio[0] = 0;
    if ( args != NULL && strchr(args, '.') != NULL ) {
	if ( sscanf(args, "%32[^.].%32s", fac, prio) != 2 ) {
	    syslog(LOG_ERR, "Wrong format for facility.priority in syslog accounting (%s)", args);
	}else{
	    ifac  = get_facility(fac);
	    if ( ifac == -1 ) {
		syslog(LOG_ERR, "Invalid facility in syslog accounting (%s) using default", args);
	        ifac  = DEFAULT_FACILITY;
	    }
	    iprio = get_priority(prio);
	    if ( iprio == -1 ) {
		syslog(LOG_ERR, "Invalid priority in syslog accounting (%s) using default", args);
	        iprio = DEFAULT_PRIORITY;
	    }
	}
    }else if ( ! nullstr(args) ) {
	ifac = get_facility(args);
	if ( ifac == -1 ) {
	    syslog(LOG_ERR, "Invalid facility in syslog accounting (%s) using default", args);
	    ifac = DEFAULT_FACILITY;
	}
    }

    /* Do all the accounting itself */

    // do nse acct first
    NSE_ACCT_LOG (NSE_ACCT_MODULE, client);

    if (! inet_ntop(AF_INET, &client->addr.sin_addr, addr, INET_ADDRSTRLEN))
	sprintf(addr, "0.0.0.0");

    syslog_status sst = syslog_status_get();
    if ( client->profile->AccountingPrefix[0] != '\0' )
        syslog_open(client->profile->AccountingPrefix, 0, ifac);
    else
        syslog_open("nntpswitchd", 0, ifac);
    syslog(iprio, "u='%s' r=%s i=%s b=%llu a=%u p=%u y=%llu", 
			client->username, client->profile->Name, addr, client->bytes, client->articles, client->posts, client->postbytes);
    syslog_close();
    syslog_status_set(sst);
}

