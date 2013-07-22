/*
 * NNTPSwitch Server Configuration
 *
 * $Id: server.c,v 1.20 2011-02-16 07:21:08 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "memory.h"
#include "aprotos.h"

int linenr;
char lastgroups[MAX_CFGLINE];
int lastlevel;


/* sort function */
int cmp_server_level(const void *a, const void *b)
{
	const SERVER *s1 = *((const SERVER **) a);
	const SERVER *s2 = *((const SERVER **) b);

	if ( s1->Level > s2->Level )
		return -1;

	if ( s1->Level < s2->Level )
		return 1;

	return 0;
}


PROTO void clear_servers(void)
{
	master->numservers = 0;
}


PROTO void reload_servers(void)
{
	struct stat _stat;

	if ( stat(cfg.ServerFile, &_stat) == -1 )
		die("Can't stat server config file %s", cfg.ServerFile);

	if ( master->laststatserver < _stat.st_mtime )
	{
		clear_servers();
		load_servers();
	}
}


PROTO void load_servers(void)
{
	FILE *f;
	struct stat _stat;
	char buf[MAX_CFGLINE];
	SERVER *def;
	int i,s;

	if ( (f=fopen(cfg.ServerFile,"r")) == NULL )
		die("Can't open server config %s", cfg.ServerFile);

	linenr = 0;
	lastgroups[0] = 0;
	lastlevel = 0;

	while( fgets(buf, MAX_CFGLINE, f) )
	{
		chop(buf);
		linenr++;

		if ( buf[0] == ';' || buf[0] == '#' || buf[0] == 0 )
			continue;

		if ( strncasecmp(buf, "server", 6) == 0 ) 
			parsecfg_server(buf,f);
		else
			die("Syntax error in %s on line %d", cfg.ServerFile, linenr);
	}
	fclose(f);

	def = getserver("default");
	if ( def  == NULL )
		die("Server \"default\" not defined");

	/* Sort the servers by level which we'll use later */
	s=0;
	for (i=0; i<master->numservers; i++ )
		if ( (master->servers+i) != def )
			master->lservers[s++] = (master->servers+i);

	qsort(master->lservers, master->numservers-1, sizeof(SERVER*), cmp_server_level);
                                                                                                                             
	stat(cfg.ServerFile,&_stat);
	master->laststatserver = _stat.st_mtime;

	/* clear the balance position list */
	for ( i=0; i< MAX_LEVEL; i++ )
		master->balance_pos[i] = 0;

        check_server_redundancy();

	info("Loaded %d Servers", master->numservers);
}


PROTO void parsecfg_server(char *buf, FILE *f)
{
	char key[64], val[MAX_CFGLINE];
	int i;
	SERVER *def;

	if ( (sscanf(buf, "%s %s", key, val)) != 2 )
		die("Server Syntax error on line %d of %s", linenr, cfg.ServerFile);

	/* Some basic initialization */
	(master->servers+master->numservers)->MaxConnections = 0;

	if ( strcmp(val, "default") != 0 ) 
	{
		/* we inherit (copy) from default server */
		if ( (def = getserver("default")) == NULL ) 
			die("Could not retrieve default server, the default server must be listed first.");

		memcpy((master->servers+master->numservers), def, sizeof(SERVER));
	}
	strcpy((master->servers+master->numservers)->Name, val);

        if ( getserver(val) != NULL )
            die("Servername \"%s\" already defined online %d of %s", val, linenr, cfg.ServerFile);

	i=0;
	while( !i && fgets(buf, MAX_CFGLINE, f) ) 
	{
		chop(buf);

		linenr++;
		if ( buf[0] == ';' || buf[0] == '#' || buf[0] == 0 )
			continue;

		if ( strncasecmp(buf, "end", 3) == 0 )
		{
			i++;
			break;
		}

		if ( sscanf(buf, "%*[ \t]%64s%*[ \t]%16384s[^\r\n]%*[\t\r\n]", key, val) != 2 )
			die("Error on line %d of %s (%s - %s)", linenr, cfg.ServerFile, key, val);

		/* start scanning parameters */

		if ( strcasecmp(key, "hostname") == 0 )
			strcpy((master->servers+master->numservers)->Hostname, val);
		else
		if ( strcasecmp(key, "groups") == 0 )
		{
			if ( lastgroups == NULL || strcmp(lastgroups, val) )
			{
				strcpy(lastgroups, val);
				lastlevel ++;
			}
			strcpy((master->servers+master->numservers)->Groups, val);
			(master->servers+master->numservers)->Level = lastlevel;
		}
		else
		if ( strcasecmp(key, "Username") == 0 )
			strcpy((master->servers+master->numservers)->Username, val);
		else
		if ( strcasecmp(key, "Password") == 0 )
			strcpy((master->servers+master->numservers)->Password, val);
		else
		if ( strcasecmp(key, "timeout") == 0 )
			(master->servers+master->numservers)->Timeout = atoi(val);
		else
		if ( strcasecmp(key, "port") == 0 )
			(master->servers+master->numservers)->Port = atoi(val);
		else
		if ( strcasecmp(key, "maxconnections") == 0 )
			(master->servers+master->numservers)->MaxConnections = atoi(val);
		else
		if ( strcasecmp(key, "type") == 0 )
		{
			if ( strcasecmp(val, "spool") == 0 )
				(master->servers+master->numservers)->ServerType = type_spool;
			else
			if ( strcasecmp(val, "xover") == 0 )
				(master->servers+master->numservers)->ServerType = type_xover;
			else
			if ( strcasecmp(val, "post") == 0 )
				(master->servers+master->numservers)->ServerType = type_post;
			else
				die("Unknown server type on line %d of %s", linenr, cfg.ServerFile);
		} 
		else
		if ( strcasecmp(key, "policy") == 0 )
		{
			if ( strcasecmp(val, "single") == 0 )
				(master->servers+master->numservers)->Policy = policy_single;
			else
			if ( strcasecmp(val, "backup") == 0 )
				(master->servers+master->numservers)->Policy = policy_backup;
			else
			if ( strcasecmp(val, "balance") == 0 )
				(master->servers+master->numservers)->Policy = policy_balance;
			else
				die("Unknown server policy on line %d of %s", linenr, cfg.ServerFile);
		}
		else
		if ( strcasecmp(key, "activetimes") == 0 )
		{
			if ( strcasecmp(val, "false") == 0 )
				(master->servers+master->numservers)->ActiveTimes = 0;
			else
			if ( strcasecmp(val, "true") == 0 )
				(master->servers+master->numservers)->ActiveTimes = 1;
			else
				(master->servers+master->numservers)->ActiveTimes = atoi(val);
		}
		else
		if ( strcasecmp(key, "descriptions") == 0 )
		{
			if ( strcasecmp(val, "false") == 0 )
				(master->servers+master->numservers)->Descriptions = 0;
			else
			if ( strcasecmp(val, "true") == 0 )
				(master->servers+master->numservers)->Descriptions = 1;
			else
				(master->servers+master->numservers)->Descriptions = atoi(val);
		}
		else
		if ( strcasecmp(key, "splitlist") == 0 )
		{
			if ( strcasecmp(val, "false") == 0 )
				(master->servers+master->numservers)->SplitList = 0;
			else 
			if ( strcasecmp(val, "true") == 0 )
				(master->servers+master->numservers)->SplitList = 1;
			else
				(master->servers+master->numservers)->SplitList = atoi(val);
		}
		else
			info("Unknown servers.conf option: %s %s", key, val);
	}

	master->numservers++;
}


PROTO SERVER* getserver(const char *name) 
{
	int i;

	for (i=0; i<master->numservers; i++)
		if ( strcmp((master->servers+i)->Name, name) == 0 )
			return (master->servers+i);

	return NULL;
}


/* 
 * count the number of servers with the same level as us 
 */

int count_server_level(int level) 
{
	int i;
	int num = 0;

	for (i=0; i<master->numservers; i++)
		if ( (master->servers+i)->Level == level )
			num++;

	return num;
}


/*
 * Increase the current server number 
 */

void add_level_usage(int lv)
{
	int max = count_server_level(lv);

	/* roll the number */
	master->balance_pos[lv]++;
	if ( master->balance_pos[lv] >= max )
		master->balance_pos[lv] = 0;
}


/*
 * get the next server from the list
 */

SERVER * get_next_server(int lv)
{
	int i, num;

	/* otherwise we have to pick one according to the balancer */
	num = master->balance_pos[lv];

	for (i=0; i<master->numservers-1; i++)
	{
		if ( master->lservers[i]->Level == lv )
		{

			if ( num == 0 )
			{
				add_level_usage(lv);

				uint max = master->lservers[i]->MaxConnections;
				uint cur = master->lservers[i]->connections;
				if ( max != 0 && cur >= max )
					return get_next_server(lv);

				return master->lservers[i];
			}
			num--;
		}
	}

	return NULL;
}


/*
 * select first server based on some servername
 */

PROTO SERVER* selectserver_primary(const char *name) 
{
	SERVER *s;

	s = getserver(name);
	if ( (int)s->connections < 0 ) {
		syslog(LOG_NOTICE, "Server %s has < 0 connections (%d)", s->Name, s->connections);
	}

	/* in case of backup scheme, check for maxconnections on server */
	if ( s->Policy == policy_backup && (s->MaxConnections != 0 && s->connections >= s->MaxConnections) )
		return get_next_server(s->Level);

	/* single or backup policy's just pick the first server */
	if ( s == NULL || s->Policy == policy_single || s->Policy == policy_backup ) 
		return s;

	s = get_next_server(s->Level);

	if ( s == NULL )
		syslog(LOG_DEBUG, "Hmm, could not find the other balancing server for %s", name);

	return s;
}


/*
 * Find the next server with the same level. This most likely
 * is a backup servers. Give backup servers the same level
 * in the servers.conf. 
 */
PROTO SERVER* selectserver_backup(SERVER *current, SERVER *primary)
{
	SERVER *s;

	/* dont check single server policies */
	if ( current == NULL || current->Policy == policy_single )
		return NULL;

	s = get_next_server(primary->Level);

	/* too bad, no backup server */
	if ( s == NULL )
		syslog(LOG_DEBUG, "No next server for policy in %s", current->Name);

	return s;
}


/*
 * Connect to a backend server taking the backup and balancing
 * policies in account. Return false on some error, true on
 * success.
 */
PROTO int connect_groupserver(CLIENT *cl, char *server)
{
	SERVER *svr, *primary;

	svr = getserver(server);
	if ( cl->connected == 1 && strcmp(cl->groupserver->Groups, svr->Groups) == 0 )
		return CONNECT_OK;

	/* disconnect first if we have to change server */
	if ( cl->connected == 1 && strcmp(cl->groupserver->Name, server) != 0 )
		disconnect_server(cl);

	if ( cl->connected != 1 ) 
	{
		/* Get the real server and take balancing stuff in account */
		if ( (svr=selectserver_primary(server)) == NULL )
			return CONNECT_ERR;

		primary = svr;  /* remember where we started with */
		cl->groupserver = svr;

		if ( connect_server(cl, svr) )
		{
			if ( errno ) 
				errno = 0;

			/* On server down find backup servers and try to conect to them */
			while( cl->connected != 1 && (svr = selectserver_backup(cl->groupserver, primary)) != NULL )
			{
				cl->groupserver = svr;
				if ( connect_server(cl, svr) )
					if ( errno )
						errno = 0;
			}

			if ( cl->connected != 1 )
			{
				cl->group = NULL;
				cl->groupserver = NULL;
				return CONNECT_DOWN;
			}
		}
	}

	return CONNECT_OK;
}


/*
 * Check if servers configured with backup actually do have a backup server configured.
 */

PROTO void check_server_redundancy(void)
{
	int i;
        SERVER *s;
        int lev = -1;

	for (i=0; i<master->numservers-1; i++)
	{
                if ( master->lservers[i]->Level == lev )
                    continue;

		if (  master->lservers[i]->Policy == policy_backup
                   || master->lservers[i]->Policy == policy_balance )
		{
                    lev = master->lservers[i]->Level;
                        info("Checking server: %s", master->lservers[i]->Name);
                        add_level_usage(lev);
                        s = get_next_server( master->lservers[i]->Level );
                        if ( s == NULL || s == master->lservers[i] ) {
                                die("Configuration error, no secondary server found, for server %s", master->lservers[i]->Name);
                        }
		}
	}
}

