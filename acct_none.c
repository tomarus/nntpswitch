#include <syslog.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

PROTO void acct_none(CLIENT *client, CONFIG *cfg, char *args) {
	return;
}
