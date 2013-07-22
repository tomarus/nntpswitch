/* $Id: acct_raddaylim.c,v 1.1 2010-11-23 13:45:23 tommy Exp $
 * (c) 2010 News-Service Europe bv
 * NNTPSwitch Radius + Daylimits Accounting Module
 */
/* Link with: libradius.so */

#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <radlib.h>

#include "../aconfig.h"
#include "../nntpd.h"
#include "../aprotos.h"
#include "log_acct.h"

#define NSE_ACCT_MODULE "raddaylim"
#define PREFIX "raddaylim: "


PROTO void acct_raddaylim(CLIENT *client, CONFIG *cfg, char *args)
{
        char *radargs = get_module_config_val(args, "rad");
        char *dbargs = get_module_config_val(args, "db");

        if ( radargs == NULL || dbargs == NULL ) {
                syslog(LOG_ERR, PREFIX "radargs or dbargs not found.");
                return;
        }

        /* Do the radius part */
        acct_radns(client, cfg, radargs);

        /* Do the daylimit part */
        do_acct_daylimit(client, cfg, dbargs);
}

