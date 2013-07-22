#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

/* If this is not NULL then a configuration file is 
 * specified on the commandline. */

char* config_file = NULL;

const char *paths[] = 
{
	"./nntpswitch.conf",
	"/etc/nntpswitch.conf",
	"/etc/nntpswitch/nntpswitch.conf",
	"/usr/local/etc/nntpswitch.conf",
	"/usr/local/etc/nntpswitch/nntpswitch.conf",
	"/usr/local/nntpswitch/nntpswitch.conf",
	"/usr/local/nntpswitch/etc/nntpswitch.conf",
	"/opt/etc/nntpswitch.conf",
	"/opt/etc/nntpswitch/nntpswitch.conf",
	"/opt/nntpswitch/nntpswitch.conf",
	"/opt/nntpswitch/etc/nntpswitch.conf",
	NULL
};


/*
 * add basepath to file if not already present.
 * overrides *filename;
 */
PROTO void check_path(char *filename)
{
	char tmp[MAX_STRING];

	if ( filename[0] == '/' )
		return;

	sprintf(tmp, "%s/%s", cfg.BasePath, filename);
	strncpy(filename, tmp, MAX_STRING);
}


/* replace filename */
char* replace_path(char *filename)
{
	char tmp[MAX_STRING];

	if ( filename[0] == '/' )
		return filename;

	sprintf(tmp, "%s/%s", cfg.BasePath, filename);
	return strdup(tmp);
}


PROTO int loadconfig(void)
{
	char buf[576];
	char a0[64], a1[512];
	int lnr = 0;
	struct cfentry *cfent;
	int i=0, ok;
	FILE *cf = NULL;

	if ( config_file == NULL )
	{
		for( i=0,ok=0; paths[i] != NULL && ok==0; i++ )
			if ( (cf=fopen(paths[i], "r")) != NULL )
			{
				ok++;
				config_file = (char *)paths[i];
			}

		if ( ok == 0 )
			die("Could not find any configuration file!");
	} else {
		cf = fopen(config_file, "r");
	}

	if ( cf == NULL ) 
		die("Could not open configuration file %s (%s)", config_file, strerror(errno));

	info("Using configuration %s", config_file);

	errno = 0;
	while( fgets(buf,sizeof(buf),cf) )
	{
		lnr++;

		if ( buf[0] == '\r' || buf[0] == '\n' || buf[0] == '#' || buf[0] == ';' ) 
			continue;

		if ( sscanf(buf,"%64s%*[ \t]%512[^\r\n]",a0,a1) != 2 ) 
			die("Error on line %d of %s", lnr, config_file);

		for (cfent = confentry; cfent->name; cfent++)
			if ( !strcasecmp(cfent->name, a0) )
				goto match;

		die("Unknown variable on line %d of %s", lnr, config_file);

match:
		switch( cfent->type )
		{
			case type_str:
				*(char **) cfent->addr = strdup (a1);
				break;

			case type_int:
				*(int *) cfent->addr = atoi(a1);
				break;

			default:
				die("Error on line %d of %s", lnr, config_file);
		}
	}

	fclose(cf);

	cfg.PidFile         = replace_path(cfg.PidFile);
	cfg.AccessFile      = replace_path(cfg.AccessFile);
	cfg.ServerFile      = replace_path(cfg.ServerFile);
	cfg.OverviewFmtFile = replace_path(cfg.OverviewFmtFile);
	cfg.ActiveFile      = replace_path(cfg.ActiveFile);
	cfg.NewsgroupsFile  = replace_path(cfg.NewsgroupsFile);
	cfg.DisableFile     = replace_path(cfg.DisableFile);

	return 0;
}

