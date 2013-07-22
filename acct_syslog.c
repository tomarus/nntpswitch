/*
 * NNTPSwitch Syslog Accounting Module
 * $Id: acct_syslog.c,v 1.12 2008-06-24 12:19:28 tommy Exp $
 *
 * Parameters from auth.conf can be full "facility.priority" or "facility"
 * only. If no parameter is specified it defaults to auth.info.
 *
 */

#include "aconfig.h"
#include "log.h"
#include "nntpd.h"
#include "aprotos.h"

#define SYSLOG_NAMES
#include "syslognames.h"

#define DEFAULT_FACILITY  LOG_AUTH
#define DEFAULT_PRIORITY  LOG_INFO


// prototyped for use in nse modules
PROTO int get_facility(char *name) {
    CODE *f;

    for (f = facilitynames; f->c_name; f++)
	if ( strcasecmp(name, f->c_name) == 0 )
   	    return (f->c_val);

    return (-1);
}


PROTO int get_priority(char *name) {
    CODE *p;

    for (p = prioritynames; p->c_name; p++)
	if ( strcasecmp(name, p->c_name) == 0 )
   	    return (p->c_val);

    return (-1);
}


PROTO void acct_syslog(CLIENT *client, CONFIG *cfg, char *args) {
    char fac[32], prio[32];

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

	syslog_status sst = syslog_status_get ();
    syslog_open ("nntpswitchd", LOG_PID, ifac);
    syslog(iprio, "accounting_entry: %s %llu %u %u %u %llu", 
		    nullstr(client->username) ? client->hostname : client->username,
		    client->bytes, 
		    client->articles, 
		    client->groups, 
		    client->posts,
		    client->postbytes);
    syslog_status_set (sst);
}

