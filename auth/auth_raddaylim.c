/*
 * radius authorization for nntpswitch
 * original by ads of voicenet. big thanks!
 */

#include <radlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

#define PREFIX "auth_raddaylimit: "

PROTO void auth_raddaylim(AUTHRESULT *authres, CONFIG *cf, char *args)
{
    bool strip = true;

    char *radargs = get_module_config_val(args, "rad");
    char *dbargs = get_module_config_val(args, "db");
    if ( radargs == NULL || dbargs == NULL ) {
        syslog(LOG_ERR, PREFIX "radargs or dbargs not found.");
        return;
    }

    auth_radius(authres, cf, radargs);

    /* if radius is successfull, set the daylimits */
    if ( authres->authenticated ) {
        char *user;
        if ( strip ) {
                user = str_escape(authres->username_s);
                authres->logname = strdup(user);
        }
        else
                user = str_escape(authres->username);

        if ( nullstr(authres->profile) ) {
            authres->message = strdup(MSG_AUTH_ERR);
            authres->authenticated = false;
            syslog(LOG_DEBUG, "auth_raddaylim: Configuration-Token (profile) not supplied by radius server.");
            free(user);
            return;
        }
        authres->userkbit = get_userkbit(cf, dbargs, user, getprofile(authres->profile));
        syslog(LOG_DEBUG, PREFIX "setting userkbit to %d", authres->userkbit);
        free(user);
    }
}

