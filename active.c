/*
 * active.c - Active file, newsgroups file and overview.fmt functions
 *
 * $Id: active.c,v 1.28 2006-10-05 15:04:25 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "memory.h"
#include "aprotos.h"

ACTIVE *newsgroups = NULL;
char *overviewfmt[MAX_OVERVIEWFMT];


PROTO void loadoverviewfmt(void)
{
	FILE *f;
	char buf[128];
	int i=0;

	if ( (f=fopen(cfg.OverviewFmtFile,"r")) == NULL )
		die("Can't open %s", cfg.OverviewFmtFile);

	while( fgets(buf,128,f) )
		overviewfmt[i++] = strdup(buf);

	fclose(f);
	info("Loaded %d overview.fmt headers", i);
	overviewfmt[i++] = NULL;
}


PROTO int isinoverviewfmt(const char *hdr)
{
	int i=0;

	while(overviewfmt[i] != NULL)
	{
		if ( strncasecmp(hdr, overviewfmt[i], strlen(hdr)) == 0 )
			return 1;
		i++;
	}
	return 0;
}


PROTO int listoverviewfmt(CLIENT *client)
{
	int i=0;

	swriteclient(client, MSG_OVERVIEWFMT);

	while(overviewfmt[i] != NULL)
	{
		swriteclient(client, overviewfmt[i]);
		i++;
	}

	swriteclient(client, ".\n");
	return 0;
}


PROTO int reloadactive(void)
{
	struct stat _stat;

	if ( stat(cfg.ActiveFile,&_stat) == -1 )
		die("Can't stat active file %s", cfg.ActiveFile);

	if ( master->laststatactive < _stat.st_mtime )
	{
		clearactive();
		return loadactive();
	}
	return 0;
}


/*
 * Read the nntpswitch active file into memory. 
 * Make sure the active file is sorted, but updategroups should take care of that
 */
PROTO int loadactive(void)
{
	FILE *actf;
	struct stat _stat;
	char group[MAX_GROUP], hi[11], lo[11], mode[2], svr[MAX_SERVER];
	char searchstr[100];
	int groups=0;
	unsigned int tim;
	int n;
	int linenr=0;

	if ( (newsgroups=memmap(sizeof(ACTIVE) * MAX_GROUPS)) == NULL )
		die("Cant mmap room for newsgroups");

	memset(newsgroups, '\0', sizeof(ACTIVE) * MAX_GROUPS);

	if ( (actf=fopen(cfg.ActiveFile,"r")) == NULL )
		die("Can't open active file %s", cfg.ActiveFile);

	sprintf(searchstr, "%%%ds %%10s %%10s %%1s %%%ds %%d", MAX_GROUP-1, MAX_SERVER-1);
	while( (n = fscanf(actf, searchstr, group, hi, lo, mode, svr, &tim)) > 0 )
	{
		linenr++;
		if ( n != 6 )
	    		info("Skipping invalid active entry on line %d, read %d fields", linenr, n);
		else {
			strncpy((newsgroups+groups)->newsgroup, group, MAX_GROUP);
			strncpy((newsgroups+groups)->server, svr, MAX_SERVER);
			(newsgroups+groups)->hi = atol(hi);
			(newsgroups+groups)->lo = atol(lo);
			(newsgroups+groups)->mode = mode[0];
			(newsgroups+groups)->times = tim;
			(newsgroups+groups)->id = groups;

			if ( getserver(svr) == NULL )
				die("Inconsistency detected, server %s (group %s) is in active:%d but not in servers.conf, run updategroups",
					svr, group, linenr);

			groups++;
			if ( groups >= MAX_GROUPS )
				die("Too many newsgroups (%d) increase MAX_GROUPS in nntpd.h", groups);
		}
	}

	fclose(actf);
	master->numgroups = groups;

	stat(cfg.ActiveFile,&_stat);
	master->laststatactive = _stat.st_mtime;

	info("Loaded %d newsgroups", groups);
	return groups;
}


PROTO void clearactive(void)
{

	memunmap(newsgroups, sizeof(ACTIVE) * MAX_GROUPS);

	master->numgroups = 0;
	info("Active file freed");
}


PROTO int cmp_getgroup(const void *a, const void *b)
{
                                                                                                                              
	const ACTIVE *s1 = (const ACTIVE *) a;
	const ACTIVE *s2 = (const ACTIVE *) b;

	return strcmp(s1->newsgroup, s2->newsgroup);
}


PROTO ACTIVE* getgroup(const char *groupname)
{
	ACTIVE ng;

	memset(&ng, 0, sizeof(ACTIVE));
	strncpy(ng.newsgroup, groupname, MAX_GROUP);

	return bsearch(&ng, newsgroups, master->numgroups, sizeof(ACTIVE), cmp_getgroup);
}


PROTO int listactive(CLIENT *client, char *pattern)
{
	int i;

	swriteclient(client, "215 Active file follows\r\n");

	for (i=0; i<master->numgroups; i++)
	{
		if ( match_expression( (unsigned char *)(newsgroups+i)->newsgroup
					, (unsigned char *)getwildmat(client->user->readpat)
					, 0) )

			if ( uwildmat_simple( (newsgroups+i)->newsgroup, pattern) )

				if ( writeclient(client, "%s %10.10ld %10.10ld %c\r\n"
						, (newsgroups+i)->newsgroup, (newsgroups+i)->hi
						, (newsgroups+i)->lo, (newsgroups+i)->mode) )
					return -1;
	}
    
	return swriteclient(client,".\r\n");
}


PROTO int listactivetimes(CLIENT *client)
{
	int i;

	swriteclient(client,"215 Active.times follows\r\n");

	for (i=0; i<master->numgroups; i++)
		if ( (newsgroups+i)->times > 0 )
			if ( writeclient(client, "%s %ld usenet\r\n"
						, (newsgroups+i)->newsgroup
						, (newsgroups+i)->times) ) 
				return -1;

	return swriteclient(client,".\r\n");
}


PROTO int listnewsgroups(CLIENT *client, char *pattern)
{
	return listfile(client, 
			(char *)cfg.NewsgroupsFile, 
			"Newsgroups follows", 
			pattern, 
			215, 503);
}


PROTO int listxgtitle(CLIENT *client, char *pattern)
{
	return listfile(client, 
			(char *)cfg.NewsgroupsFile, 
			"Newsgroups follows", 
			pattern, 
			282, 481);
}


PROTO int listfile(CLIENT *client, char *filename, char *descr, char *pattern, int ok, int fail)
{
	char buffer[512];
	FILE *ngfile;

	if ( (ngfile=fopen(filename,"r")) == NULL )
	{
		syslog(LOG_INFO,"error opening %s %m",filename);
		return writeclient(client,"%d Error opening %s\r\n", fail, filename);
	}

	writeclient(client,"%d %s\r\n",ok, descr);

	while( fgets(buffer, 511, ngfile) )
		if ( uwildmat_simple( buffer, pattern) )
			if ( swriteclient(client, buffer) )
				return -1;

	swriteclient(client, ".\r\n");
	fclose(ngfile);
	return 0;
}


void newgroups_error(CLIENT *client, char *error)
{
	syslog(LOG_NOTICE, "Invalid date format in newgroup command, %s", error);
	writeclient(client, "501 Wrong command use %s\r\n", error);
}


PROTO int newgroups(CLIENT *client, char *args)
{
	struct tm tm;
	int i;
	char timebuf[40];
	ulong since;
	char datbuf[9];
	int y,m;

	memset(&tm, 0, sizeof(tm));

	if ( sscanf(args,"%8s %2d%2d%2d", datbuf, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) < 1 ) 
	{
		newgroups_error(client, "yyyymmdd hhmmss");
		return 1;
	}

	if ( strlen(datbuf) == 8 )
	{
		if ( sscanf(datbuf,"%4d%2d%2d",&y,&m,&tm.tm_mday) != 3 )
		{
			newgroups_error(client,"yyyymmdd");
			return 1;
		} else
			tm.tm_year = y-1900;
	}
	else if ( strlen(datbuf) == 7 )
	{
		if ( sscanf(datbuf,"%3d%2d%2d",&y,&m,&tm.tm_mday) != 3 )
		{
			newgroups_error(client,"yyymmdd");
			return 1;
		} 
	}
	else if ( strlen(datbuf) == 6 )
	{
		if ( sscanf(datbuf,"%2d%2d%2d",&y,&m,&tm.tm_mday) != 3 )
		{
			newgroups_error(client,"yymmdd");
			return 1;
		} else
			tm.tm_year = y < 70 ? y+100 : y;
	}
	else if ( strlen(datbuf) == 5 )
	{
		if ( sscanf(datbuf,"%1d%2d%2d",&y,&m,&tm.tm_mday) != 3 )
		{
			newgroups_error(client,"ymmdd");
			return 1;
		} else
			tm.tm_year = y + 100;
	}
	else
	{
		newgroups_error(client, "Couldn't parse date");
		return 1;
	}

	tm.tm_mon  = m - 1;

	strftime(timebuf, sizeof(timebuf), "%s",&tm);
	since = atoi(timebuf);

	swriteclient(client, "231 List of newgroups follow\r\n");

	for (i=0; i<master->numgroups; i++)
		if ( (newsgroups+i)->times >= since )
			if ( writeclient(client, 
					"%s %10.10ld %10.10ld %c\r\n", 
					(newsgroups+i)->newsgroup, 
					(newsgroups+i)->hi, 
					(newsgroups+i)->lo, 
					(newsgroups+i)->mode) )
				return -1;

	return swriteclient(client, ".\r\n");
}

