/* $Id: acct_md5syslog.c,v 1.1.1.1 2006-08-30 12:07:27 mjo Exp $
 * (c) 2006 News-Service Europe bv
 * NNTPSwitch Syslog Accounting Module with md5
 */

#include "../aconfig.h"
#include "../nntpd.h"
#include "../aprotos.h"
#include "log_acct.h"

#define NSE_ACCT_MODULE "md5syslog"

PROTO void acct_md5syslog(CLIENT *client, CONFIG *cfg, char *args)
{
	NSE_ACCT_LOG (NSE_ACCT_MODULE, client);
}
