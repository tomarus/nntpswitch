/*
 * updategroups - Update grouplist. Run this command periodically
 *
 * $Id: updategroups.c,v 1.21 2007-11-06 13:33:55 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "log.h"
#include "aprotos.h"
#include "memory.h"
#include "tgetopt.h"


ACTIVE* gg(const char*, int);
void update_server(SERVER*);
void getlist_active(SERVER*, int, char*);
void getlist_activetimes(SERVER*, int);
void getlist_newsgroups(SERVER*, int, char*);
void write_active(void);
void write_newsgroups(void);
int cmp_active(const void *, const void *);

#define BUFSIZE MAX_CFGLINE
char buf[BUFSIZE];

ACTIVE *groups;
CLIENT uclient;

struct DESCR {
	char*	newsgroup;
	char*	description;
};
struct DESCR descriptions[MAX_GROUPS];
int numdescrs = 0;

struct option opts[] = {
	{ "config", 1, NULL, 'c', 0 },
	{ "help",   0, NULL, 'h', 0 },
	{ NULL,     0, NULL, 0,   0 },
};

static const char* usage = "Usage: %s [options]\n"
	"   -c, --config CONFIGFILE\tSpecify configfile instead of searching for it.\n"
	"   -h, --help             \tCommand usage.\n";


/*
 * Check if a group exists or find a group ptr
 */

ACTIVE* gg(const char *groupname, int max)
{
	ACTIVE ng;

	if ( max == 0 )
		max = master->numgroups;

	memset(&ng, 0, sizeof(ACTIVE));
	strncpy(ng.newsgroup, groupname, MAX_GROUP);

	return bsearch(&ng, groups, max, sizeof(ACTIVE), cmp_active);
}


struct DESCR* getdescr(const char *groupname, int max)
{
	int i;

	for (i=0; i<max; i++)
		if ( (descriptions+i)->newsgroup[0] == groupname[0] &&
				strcmp((descriptions+i)->newsgroup, groupname) == 0 )
			return (descriptions+i);
	return NULL;
}


int main(int argc, char **argv)
{
	int i, cur;

	tgetopt(argc, argv);

	if ( toptset('h') )
	{
		printf(usage, argv[0]);
		exit(0);
	}

	if ( toptset('c') ) 
		config_file = toptargs('c');

	syslog_open("updategroups", LOG_PID, LOG_NEWS);

	if ( (master = memmap(sizeof(MASTER))) == NULL )
		die("Can't allocate master memory");

	if ( (groups = memmap(sizeof(ACTIVE) * MAX_GROUPS)) == NULL )
		die("Can't allocate groups memory");

	loadconfig();
	load_servers();

	cur = -1;
	for (i=master->numservers-2; i>=0; i-- )
		if ( (master->lservers[i])->Level != cur )
		{
			cur = (master->lservers[i])->Level;
			update_server( master->lservers[i] );
		}

	write_active();
	write_newsgroups();

	memunmap(master, sizeof(MASTER));
	memunmap(groups, sizeof(ACTIVE) * MAX_GROUPS);

	syslog_close();

	exit(0);
}


void update_server(SERVER *svr) 
{
	int sock;
	char *p;
	char gbuf[BUFSIZE];
	SERVER *server;

	server = svr;

	int c = connect_groupserver(&uclient, server->Name);
	if ( c == CONNECT_DOWN || c == CONNECT_ERR )
		die("Failed to connect to server %s", server->Name);

	server = uclient.groupserver;
	sock = uclient.serversock;

	info("Connected to server: %s:%d", server->Hostname, server->Port);

	if ( server->SplitList == 1 )
	{
		strncpy(gbuf, server->Groups, BUFSIZE);
		p = strtok(gbuf, ",");
		do {
			getlist_active(server, sock, p);
			if ( server->Descriptions ) 
				getlist_newsgroups(server, sock, p);
		}
		while( (p = strtok(NULL, ",")) );
	}
	else
	{
		getlist_active(server, sock, server->Groups);
		if ( server->Descriptions ) 
			getlist_newsgroups(server, sock, server->Groups);
	}

	if ( server->ActiveTimes )
		getlist_activetimes(server, sock);

	sprintf(buf, "QUIT\r\n");
	if ( write_socket(sock, buf, strlen(buf), server->Timeout) == -1 )
		die("Cant write to server");

	uclient.connected = 0;

	close(sock);
}


void getlist_active(SERVER *server, int sock, char *glist)
{
	char group[512], end[11], begin[11], mode[2];
	int num = 0;
	int i;
	int ng;

	sprintf(buf, "MODE READER\r\n");

	if ( write_socket(sock, buf, strlen(buf), server->Timeout) == -1 ) 
		die("Cant write to server");

	if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
		die("Cant read mode reader response");

	sprintf(buf, "LIST active %s\r\n", glist);

	if ( write_socket(sock, buf, strlen(buf), server->Timeout) == -1 ) 
		die("Cant write to server");

	if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
		die("Cant read list active response");

	if ( atoi(buf) != 215 )
		die("Invalid response from server: %s", buf);

	ng = master->numgroups;
	while ( 1 )
	{
		if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
			die("Cant read active file");

		if ( buf[0] == '.' && buf[1] == '\r' )
			break;

		if ( sscanf(buf, "%511s %10s %10s %2s", group, end, begin, mode) != 4 )
		{
			info("Invalid active entry: %s %d %d", buf, buf[0], buf[1]);
		}
		else
		{
			if ( strlen(group) >= MAX_GROUP )
			{
				info("Skipping too long groupname: %s", group);
				continue;
			}

			if ( gg(group, ng) == NULL )
			{
				if ( master->numgroups == MAX_GROUPS )
					die("Too many newsgroups, increase MAX_GROUPS in nntpd.h");

				i=master->numgroups;
				strncpy((groups+i)->newsgroup, group, 191);
				strncpy((groups+i)->server, server->Name, 31);	
				(groups+i)->hi = atol(end);
				(groups+i)->lo = atol(begin);
				(groups+i)->mode = mode[0];
				(groups+i)->times = 0;

				master->numgroups++;
				num++;
			}
		}
	}

	qsort(groups, master->numgroups, sizeof(ACTIVE), cmp_active);

	printf("Loaded %d groups, %d total\n", num, master->numgroups);
}


void getlist_activetimes(SERVER *server, int sock)
{
	char group[512], time[16], creator[256];
	int num = 0;
	ACTIVE *gr;

	sprintf(buf, "LIST active.times\r\n");
	if ( write_socket(sock, buf, strlen(buf), server->Timeout) == -1 )
		die("Cant write to server");

	if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
		die("Cant read list active.times response");

	if ( atoi(buf) != 215 )
		die("Invalid response from server: %s", buf);

	while ( 1 )
	{
		if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
			die("Cant read active.times file");

		if ( buf[0] == '.' && buf[1] == '\r' )
			break;

		if ( sscanf(buf, "%511s %15s %255s", group, time, creator) != 3 )
		{
			info("Invalid active.times entry: %s %d %d", buf, buf[0], buf[1]);
		}
		else
		{
			if ( (gr=gg(group, 0)) != NULL )
			{
				if ( gr->times == 0 )
				{
					gr->times = atol(time);
					num++;
				}
			}
		}
	}

	printf("Loaded active.times for %d groups\n", num);
}


void getlist_newsgroups(SERVER *server, int sock, char *glist)
{
	char group[512], descr[512];
	int num = 0, nd;
	struct DESCR *dsc;

	sprintf(buf, "LIST newsgroups %s\r\n", glist);
	if ( write_socket(sock, buf, strlen(buf), server->Timeout) == -1 )
		die("Cant write to server");

	if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
		die("Cant read list newsgroups response");

	if ( atoi(buf) != 215 )
		die("Invalid response from server: %s", buf);

	nd = numdescrs;
	while ( 1 )
	{
		if ( fgetsrn(buf, BUFSIZE, sock) == NULL )
			die("Cant read newsgroups file");

		if ( buf[0] == '.' && buf[1] == '\r' )
			break;

		if ( sscanf(buf, "%510s %510[^\r\n]", group, descr) == 2 )
		{
			/*
			 * check for garbage description like "???" or "No description" or such
			 * this will also match correct descriptions which just happen to have the
			 * word description in their name.
			 */
			if ( descr[0] != '?' && strstr(descr, "Description") == NULL && strstr(descr, "description") == NULL )
			{
				if ( (dsc=getdescr(group, nd)) == NULL )
				{
					(descriptions+numdescrs)->newsgroup   = strdup(group);
					(descriptions+numdescrs)->description = strdup(descr);
					num++;
					numdescrs++;
				}
			}
		}
	}

	printf("Loaded newsgroups for %d groups\n", num);
}


/*
 * Great, so this is the same as cmp_getgroup() from active.c but we would be
 * including too much other object files if we linked against active.o.
 * so this is left to be FIXME in some generic library or whatever
 */
int cmp_active(const void *a, const void *b)
{
	const ACTIVE *s1 = (const ACTIVE *) a;
	const ACTIVE *s2 = (const ACTIVE *) b;

	return strcmp(s1->newsgroup, s2->newsgroup);
}


void write_active(void)
{
	FILE *file;
	int i;

	info("Writing active file %s", cfg.ActiveFile);

	qsort(groups, master->numgroups, sizeof(ACTIVE), cmp_active);

	if ( (file=fopen(cfg.ActiveFile, "w")) == NULL )
		die("Cant write active file %s", cfg.ActiveFile);

	for (i=0; i<master->numgroups; i++)
	{
		fprintf(file, "%s %.10ld %.10ld %c %s %ld\n",
			(groups+i)->newsgroup,
			(groups+i)->hi,
			(groups+i)->lo,
			(groups+i)->mode,
			(groups+i)->server,
			(groups+i)->times);
	}

	fclose(file);
}


void write_newsgroups(void)
{
	FILE *file;
	int i;

	info("Writing newsgroups file %s", cfg.NewsgroupsFile);

	if ( (file=fopen(cfg.NewsgroupsFile, "w")) == NULL )
		die("Cant write newsgroups file %s", cfg.NewsgroupsFile);

	for (i=0; i<numdescrs; i++)
		fprintf(file, "%s %s\n", (descriptions+i)->newsgroup, (descriptions+i)->description);

	fclose(file);
}

