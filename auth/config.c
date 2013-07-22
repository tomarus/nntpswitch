/*
 * (c) News-Service.com 2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"


/*
 * Get a value from an arg which are configured with "arg var value" config lines.
 */
static char *get_args_value(ARGS *args, int numargs, const char *name)
{
        int i;
        for (i=0; i<numargs; i++)
                if ( strcmp((args+i)->name, name) == 0 )
                        return (args+i)->args;

        return NULL;
}

/*
 * Retrieve value from an "alias module var=value" string.
 * Put value of var in **value.
 * Return true if found, false if not.
 */
static bool get_module_config_arg(char *confargs, const char *var, char *value)
{
        char *ap, *p;

        ap = strdup(confargs);
        p = strtok(ap, ",: ");
        do {
                int varlen = strlen(var);
                if ( strncasecmp(p, var, varlen) == 0 ) {
                        strncpy(value, p+varlen+1, MAX_NAME-1);
                        return true;
                }
        }
        while( (p=strtok(NULL,",: ")) );
        return false;
}

/* Get actual configured value from arg specified in module config.
 * alias testmod db=proddb rad=prodradius
 * get_module_config_val(mod_config_line, "db");
 * Will return actual db connect string.
 */
PROTO char *get_module_config_val(char *confargs, const char *var)
{
        char argval[MAX_NAME];

        get_module_config_arg(confargs, var, argval);
        return get_args_value(master->args, master->numargs, argval);
}

