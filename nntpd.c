/*
 * nntpd.c  General client allocation and command parser routines
 * $Id: nntpd.c,v 1.110 2010-12-14 11:31:43 tommy Exp $
 */

#define _GNU_SOURCE
#include <sched.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"
#include "log.h"
#include "setproctitle.h"
#include "semaphore.h"

static char helptxt[] = "100 Supported Commands\r\n"
		"   MODE READER\r\n"
		"   AUTHINFO USER <username>\r\n"
		"   AUTHINFO PASS <password>\r\n"
		"   LIST <active|newsgroups|overview.fmt|active.times> <pattern>\r\n"
		"   XGTITLE <pattern>\r\n"
		"   GROUP <newsgroup>\r\n"
		"   LISTGROUP <newsgroup>\r\n"
		"   NEWGROUPS <yy><yymmdd> <hhmmss>\r\n"
		"   OVER <range|msgid>\r\n"
		"   XOVER <range|msgid>\r\n"
		"   XZVER <range|msgid>\r\n"
		"   XHDR <header> <range|msgid>\r\n"
		"   XZHDR <header> <range|msgid>\r\n"
		"   XPAT <header> <range|msgid> <pattern> <pattern..>\r\n"
		"   NEWNEWS <newsgroup> <yymmdd> <hhmmss> <gmt|utc>\r\n"
		"   STAT <msgid|number>\r\n"
		"   HEAD <msgid|number>\r\n"
		"   BODY <msgid|number>\r\n"
		"   ARTICLE <msgid|number>\r\n"
		"   POST\r\n"
		"   NEXT\r\n"
		"   LAST\r\n"
		"   HELP\r\n"
		"   DATE\r\n"
		"   QUIT\r\n"
		"The NEWNEWS command is only partly supported for single newsgroups only.\r\n"
		"\r\n"
		"NNTPSwitch-%s (c) Tommy van Leeuwen and contributors 2002-2012\r\n"
		".\r\n";

static CMDINFO commands[] = {       /* auth, acct, limit, usecount */
	{ "MODE",	cmd_mode,	1, 0, 0, 0 },
	{ "SLAVE",	cmd_slave,	0, 0, 0, 0 }, /* not in help */
	{ "AUTHINFO",	cmd_authinfo,	0, 0, 0, 0 },
	{ "LIST",	cmd_list,	1, 0, 0, 0 },
	{ "XGTITLE",	cmd_xgtitle,	1, 0, 0, 0 },
	{ "GROUP",	cmd_group,	1, 0, 0, 0 },
	{ "LISTGROUP",	cmd_listgroup,	1, 0, 0, 0 },
	{ "NEWGROUPS",	cmd_newgroups,	1, 0, 0, 0 },
	{ "OVER",	cmd_over,	1, 0, 0, 0 },
	{ "XOVER",	cmd_xover,	1, 0, 0, 0 },
	{ "XZVER",	cmd_xzver,	1, 0, 0, 0 },
	{ "XHDR",	cmd_xhdr,	1, 0, 0, 0 },
	{ "XZHDR",	cmd_xzhdr,	1, 0, 0, 0 },
	{ "XPAT",	cmd_xpat,	1, 0, 0, 0 },
	{ "NEWNEWS",	cmd_newnews,	1, 0, 0, 0 },
	{ "STAT",	cmd_stat,	1, 0, 0, 0 },
	{ "HEAD",	cmd_head,	1, 1, 1, 0 },
	{ "BODY",	cmd_body,	1, 1, 1, 0 },
	{ "ARTICLE",	cmd_article,	1, 1, 1, 0 },
	{ "POST",	cmd_post,	1, 0, 0, 0 },
	{ "NEXT",	cmd_next,	1, 0, 0, 0 },
	{ "LAST",	cmd_last,	1, 0, 0, 0 },
	{ "HELP",	cmd_help,	0, 0, 0, 0 },
	{ "DATE",	cmd_date,	0, 0, 0, 0 },
	{ "QUIT",	cmd_quit,	0, 0, 0, 0 },
	{ "STATISTICS", cmd_statistics, 0, 0, 0, 0 },
	{ "IHAVE",	cmd_ihave,	0, 0, 0, 0 }, /* not in help */
	{ "GET",	cmd_http,	0, 0, 0, 0 },
	{ NULL,		cmd_unsupported,0, 0, 0, 0 },
};



static void client_error(char *txt) 
{
	syslog(LOG_NOTICE, "%s: client error %s ",client->hostname, txt);
	writeclient(client,MSG_GENERIC_ERROR,txt);
	close_child(true);
	syslog_close ();
	exit(0);
}


/*
 * Log command usage statistics about the leaving client
 */

PROTO void client_log_statistics(void)
{
	CMDINFO *cmd;
	char buf[1024] = "\0";

	syslog(LOG_DEBUG, "%s: connection closed articles %u bytes %llu groups %u posts %u postbytes %llu",
			client->hostname, client->articles, client->bytes,
			client->groups, client->posts, client->postbytes);

	for( cmd = commands; cmd->command; cmd++ )
		sprintf(buf, "%s %s %d", buf, cmd->command, cmd->usecount);

	syslog(LOG_DEBUG, "%s: commands usage%s UNKNOWN %d", client->hostname, buf, cmd->usecount);

        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        syslog(LOG_DEBUG, "rusage: systime %lu utime %lu",
            (ru.ru_stime.tv_sec * 1000000) + ru.ru_stime.tv_usec,
            (ru.ru_utime.tv_sec * 1000000) + ru.ru_utime.tv_usec
        );
}


/* this is called directly after forking a child and after client is allocated */
PROTO void run_child(CLIENT *cl)
{
	char *rc = NULL;
	struct hostent *hp = NULL;
	int socksize = cfg.SockBufSize;
	client = cl;

	setproctitle("nntpswitchd %u: %s initializing", client->id, inet_ntoa(client->addr.sin_addr) );

	/* 
	 * Initialize Client
	 */
	master->nrforks++;
	master->connections++;

	client->errstr		= NULL;
	client->group		= NULL;
	client->groupserver	= NULL;
	client->profile		= client->acl->profile;
	client->posting		= client->acl->post;
	client->connectsince	= time(NULL);
	client->pid		= getpid();


#if defined(_SC_NPROCESSORS_ONLN)
	if ( cfg.CoreBind > 0 )
	{
		/* Bind to CPU core */
		unsigned int mask = 1 << (unsigned int)client->numcore;
//		info("CPU: numcore = %d and mask = %x", client->numcore, mask);
		if ( sched_setaffinity(0, sizeof(mask), (cpu_set_t*)&mask) < 0 )
			die("Cant set CPU Affinity");
	}
#endif

	PROFILE *p;
	p = prof_iplookup(client->acl->profile, client->addr.sin_addr);
	if ( p != NULL )
		client->profile = p;
	
	client->ip4addr = ntohl(client->addr.sin_addr.s_addr);
	if ( cfg.DNSLookups == 1 )
	{
		hp = gethostbyaddr((char*)&client->addr.sin_addr.s_addr, sizeof(client->addr.sin_addr.s_addr), AF_INET);
		strncpy(client->hostname, hp != NULL ? (char *)hp->h_name : inet_ntoa(client->addr.sin_addr), MAX_HOSTNAME-1);
	} else {
		strncpy(client->hostname, inet_ntoa(client->addr.sin_addr), MAX_HOSTNAME-1);
	}

	/*
	 * Find a *USER based on the profile we're connected to. Check
	 * for any connection or user limits.
	*/

	semlock_lock(master->semid);
	client->profile->connections++;
	client->user = find_user(client->hostname, client->profile);
	semlock_unlock(master->semid);

        /* Find host:service client connects to. */
        char serv[NI_MAXSERV];
        char host[NI_MAXHOST];
        if ( tcp_sock2host(client->socket, inet_ntoa(client->addr.sin_addr), NI_MAXHOST, serv, host) == -1 ) {
                /* Client has disconnected already here */
                close_child(true);
                return;
        }
        syslog(LOG_DEBUG, "Client connection to %s:%d", host, atoi(serv));
        client->vhost = getvhost(host);
        if ( client->vhost == NULL ) client->vhost = getvhost("default");

	if (client->useSSL)
	{
	    client->ssl = ssl_init_accept(client->socket, client->vhost->ssl_ctx);
	    if (! client->ssl)
	    {
		syslog(LOG_ERR, "Cant set up SSL connection");
		close_child(true);
		return;
	    }
	}	    

	if ( client->user == NULL )
	{
		sleep(cfg.RetryDelay);
		swriteclient(client, MSG_SERVERFULL);
		syslog(LOG_ERR, "Too many global users, increase MAX_USERS");
		close_child(true);
		return;
	}

	if ( ! nullstr(client->profile->Disabled) )
	{
		sleep(cfg.RetryDelay);
		writeclient(client, "400 %s", client->profile->Disabled);
		close_child(true);
		return;
	}

	if (client->useSSL && !client->profile->AllowSSL  )
	{
		swriteclient(client, MSG_SSLDISABLED);
		syslog(LOG_ERR, "502 SSL is not allowed for this profile");
		close_child(true);
		return;
	}

	if ( (client->bbuf  = valloc(cfg.BufSize+1)) == NULL )
		client_error("No room for bbuf");

	if ( cfg.SockBufSize != 0 )
		if ( setsockopt(client->socket, SOL_SOCKET, SO_SNDBUF, (char *) &socksize, sizeof(socksize)) < 0 )
			client_error("cant set so_sendbuf");

	syslog(LOG_NOTICE, "connect from %s: clientid %u clientnum %u total %u",
			client->hostname, client->id, client->concur, master->connections);

	if ( client->acl->unlimit != 1 &&
		client->user->connections > client->profile->MaxUserConcurrent )
	{
		sleep(cfg.RetryDelay);
		writeclient(client, MSG_TOOMANY_CONCURRENT, client->profile->MaxUserConcurrent);
		close_child(true);
		return;
	}

#ifdef USE_REALUSER
	if ( client->profile->realusers >= client->profile->MaxUsers )
#else
	if ( client->profile->numusers >= client->profile->MaxUsers )
#endif
	{
		sleep(cfg.RetryDelay);
		swriteclient(client, MSG_TOOMANYUSERSPRF);
		close_child(true);
		return;
	}

#ifdef USE_REALUSER
	if ( client->user->realuser == 0  )
	{
		client->profile->realusers++;
		client->user->realuser = 1;
	}
#endif

	init_ratelimiter(client);
	
	client->user->MaxUserBPS = client->profile->MaxUserBPS;

	if ( client->vhost->ReplaceBanner == 1 )
	{
		writeclient(client, "%u %s", client->posting ? 200 : 201, client->vhost->Banner);
	} else {
		writeclient(client, "%u %s NNTPSwitch-%s, %u groups available, %s, slot %u, connections %u\r\n",
				client->posting ? 200 : 201,
				client->vhost->Banner,
				VERSION,
				master->numgroups,
				client->posting ? "posting allowed" : "no posting",
				client->id + 1,
				client->concur
		);
	}

	setproctitle("nntpswitchd %u: %s idle", client->id, client->hostname);

	/* main client loop */
	errno=0;

	while (
		client->error == 0 &&
		(rc=readclient(client, client->bbuf, cfg.BufSize, 0)) != NULL &&
		time(NULL) - client->starttime < client->user->timeleft &&
		client->user->bytesleft >= 0
	) 
	{
		if ( parsecmd(client->bbuf) )
			break;
		setproctitle("nntpswitchd %u: %s idle", client->id, client->hostname);
		errno=0;
	}

	/*
	 * check what kind of errors we have or had en exit the client based on that
	 * this code in general and way of errorhandling is worse than lame
	 */

	if ( client->error > 0 )
	{
		syslog(LOG_NOTICE,"%s: timeout or error %s", client->hostname, client->errstr);
	}
	else if (client->user->bytesleft < 0)
	{
		syslog(LOG_NOTICE,"%s: session byte limit reached (bytes=%lld, MaxSessionBytes=%llu, bytesleft=%lld)"
				, client->user->key, client->bytes, client->profile->MaxSessionBytes, client->user->bytesleft);
		swriteclient(client, MSG_BYTELIMIT);
	}
	else if ( time(NULL) - client->starttime >= client->user->timeleft )
	{
		syslog(LOG_NOTICE,"%s: session time limit reached %s"
				, client->hostname, client->timeout ? "(timeout)" : "(normal)");
		swriteclient(client, MSG_SESSIONLIMIT);
	}
	else if ( rc == NULL && client->timeout == 1 )
	{
		syslog(LOG_NOTICE,"%s: client timeout after %d seconds", client->hostname, client->profile->ClientReadTimeout);
		writeclient(client, "205 Timeout after %d seconds, articles %u, bytes %llu, groups %u, posts %u, postbytes %llu\r\n",
				client->profile->ClientReadTimeout,
				client->articles,
				client->bytes,
				client->groups,
				client->posts,
				client->postbytes
		);
	}
	else if ( errno )
	{
		syslog(LOG_NOTICE,"%s: timeout or error fixme %m", client->hostname);
		writeclient(client, "205 Timeout/Error articles %u, bytes %llu, groups %u, posts %u, postbytes %llu\r\n",
				client->articles,
				client->bytes,
				client->groups,
				client->posts,
				client->postbytes
		);
	}
	else
	{
		syslog(LOG_NOTICE,"%s: eof from client", client->hostname);
	}

	if ( client->postsock > 0 )
	{
		syslog(LOG_DEBUG, "Finishing old post connection (1)");
		int oldsock = client->serversock;
		client->serversock = client->postsock;
		disconnect_server(client);
		client->serversock = oldsock;
	}

	client_quit();
	close_child(true);
	return;
}


/* 
 * return true to exit, false to continue accepting commands
 */
PROTO int parsecmd(char *buf)
{
	char a0[512],a1[512];
	CMDINFO *cmd;
    int wr;
    char * wp;

	a0[0] = 0; a1[0] = 0;
	sscanf(buf, "%31[^\r\n\t ] %480[^\r\n]", a0, a1);

	if (a0[0] == '\0')
		return 0;

	for( cmd = commands; cmd->command; cmd++ )
		if ( !strcasecmp(a0,cmd->command) )
			break;

	client->command = cmd;
	cmd->usecount++;

	if ( cmd->num == cmd_authinfo )
	{
		if ( cfg.LogCommands )
			syslog(LOG_DEBUG, "%s: command %s", client->hostname, a0);
		setproctitle("nntpswitchd %u: %s command %s", client->id, client->hostname,a0);
	} else {
		if ( cfg.LogCommands )
			syslog(LOG_DEBUG, "%s: command %s %s", client->hostname, a0, a1);
		setproctitle("nntpswitchd %u: %s command %s %s", client->id, client->hostname,a0,a1);
	}

	if ( cmd->auth == 1 && client->acl->auth == 1 && client->authenticated == 0 )
		return swriteclient(client, MSG_AUTHREQUIRED);

	if ( cmd->num != cmd_post && client->postsock > 0 )
	{
		syslog(LOG_DEBUG, "Finishing old post connection");
        	close(client->postsock);
		client->postsock = 0;
	}

	switch( cmd->num ) 
	{

		case cmd_quit:
			writeclient(client,"205 Exit articles %u, bytes %llu, groups %u, posts %u, postbytes %llu\r\n",
					client->articles,
					client->bytes,
					client->groups,
					client->posts,
					client->postbytes
			);
			client_quit();
			close_child(true);
			syslog_close ();
			_exit(0);

		case cmd_http:
			swriteclient(client, MSG_COMMAND_HTTP);
			client_quit();
			close_child(true);
			syslog_close ();
			_exit(0);

		case cmd_help:
			return writeclient(client, helptxt, VERSION);

		case cmd_date:
			send_date();
			return 0;

		case cmd_mode:
			if ( !strcasecmp(a1,"reader") )
			{
				if ( client->connected == 1 )
				{
					if ( ! writeserver(client,"MODE READER\r\n") ) return 1;
					if ( (readserver(client, a1, MAX_SERVERRSP-1)) == NULL ) return 1;
					if ( atoi(a1) > 299 ) 
					{
						syslog(LOG_ERR, "%s: mode reader check server %s response %s",
								client->hostname,
								client->currserver->Name,
								a1
						);
						writeclient(client,a1);
						return 1;
					}
				}
				return writeclient(client,"%u Sure!\r\n", client->posting ? 200 : 201);
			} else
				return swriteclient(client,"500 Unsupported MODE command\r\n");

		case cmd_slave:
			return swriteclient(client,"202 Sure!\r\n");

		case cmd_authinfo:
			return cmdauthinfo(a1);

		case cmd_list:
			return cmdlist(a1);

		case cmd_xgtitle:
			return cmdxgtitle(a1);

		case cmd_group:
			group(a1, false);
			return 0;

		case cmd_newgroups:
			newgroups(client, a1);
			return 0;

		case cmd_listgroup:
			if ( strlen(a1) > 0 )
			{
				// groupname has been specified by user 
				if ( group(a1, true) == 0 )
					return passcmd(a0, a1, cmd);
				else
					return swriteclient(client, MSG_NOSUCHGROUP);
			}
			else
			{
				if ( client->group == NULL )
					// no current group selected 
					return swriteclient(client, MSG_NOGROUP);
				else
					// no group given, use current selected group 
					return passcmd(a0, "", cmd); 
			}
			return 0;

		case cmd_over:
		case cmd_xover:
		case cmd_xzver:
			if ( client->group == NULL )
				return swriteclient(client, MSG_NOGROUP);
			else {
                if ( client->groupserver == NULL ) {
                    /* disconnected due to server error probably */
                    group(client->group->newsgroup, true);
                }
				return passcmd(a0, a1, cmd);
            }

		case cmd_xpat:
		case cmd_xhdr:
		case cmd_xzhdr:
			if ( client->group == NULL )
				return swriteclient(client, MSG_NOGROUP);
			if ( cfg.DenyNoIndexField )
			{
				sscanf(a1,"%128[^\r\n\t ]",buf);

				if ( isinoverviewfmt(buf) )
					return passcmd(a0, a1, cmd);
				else
					return swriteclient(client, MSG_NOPATINDEX);
			}
			return passcmd(a0, a1, cmd);

		case cmd_head:
			if ( catch_art_server(a1) == false )
				return 0;
			return passcmd(a0, a1, cmd);

		case cmd_article:
		case cmd_body:
			if ( catch_art_server(a1) == false )
				return 0;

			client->articles++;
			client->serverarts++;
			client->grouparts++;
			client->profile->articles++;
			return passcmd(a0, a1, cmd);

		case cmd_newnews:
			sscanf(a1,"%256[^\r\n\t ] %256[^\r\n]",a0,buf);
			if ( !strcmp(a0,"newnews") || strchr(a0,'*') != NULL || strchr(a0,'!') != NULL )
			{
				return swriteclient(client, MSG_NONEWNEWS);
			} else {
				if ( group(a0, true) == 0 ) 
					return passcmd("NEWNEWS", a1, cmd);
				else
					return swriteclient(client, MSG_NOSUCHGROUP);
			}
			return 0;

		case cmd_stat:
			if ( catch_art_server(a1) == false )
				return 0;

			wr = writeserver(client,"%s %s\r\n",a0,a1);
			wp = readserver(client, buf, 256);
            
			if ( wp == NULL ) {
				    client_error(UMSG_SERVERTIMEOUT);
                    break;
            }
			if ( wr <= 0 || buf[0] == '\0' ) {
                    syslog(LOG_ERR, "Server down during cmd_stat, sending MSG_SERVER_DOWN");
                    swriteclient(client, MSG_SERVER_DOWN);
                    quiet_disconnect_server(client);
                    break;
            }
            return swriteclient(client,buf);

		case cmd_last:
		case cmd_next:
			if ( client->group == NULL )
				return swriteclient(client, MSG_NOGROUP);

            wr = writeserver(client,"%s %s\r\n",a0,a1);
		    wp = readserver(client, buf, 256);
			
			if ( wp == NULL ) {
				    client_error(UMSG_SERVERTIMEOUT);
                    break;
            }
			if ( wr <= 0 || buf[0] == '\0' ) {
                    syslog(LOG_ERR, "Server down during cmd_next/cmd_last, sending MSG_SERVER_DOWN");
                    swriteclient(client, MSG_SERVER_DOWN);
                    quiet_disconnect_server(client);
                    break;
            }
            return swriteclient(client,buf);

		case cmd_post:
			if ( client->posting == 0 )
				return swriteclient(client, MSG_NOPOSTING);
			else
				return post(client);

		case cmd_ihave:
			if ( client->profile->Slave == 0 )
				return swriteclient(client, MSG_UNSUPPORTED);
			else
				return ihave(client, a1);

		case cmd_unsupported:
			if ( cmd->usecount >= cfg.DropUnknown )
			{
				writeclient(client, "500 Dropped after %d invalid commands", cfg.DropUnknown);
				client_quit();
				close_child(true);
				syslog_close ();
				_exit(0);
			}
			return swriteclient(client, MSG_UNSUPPORTED);

		case cmd_statistics:
			cmdstatistics(a1);
			return 0;

		default:
			return swriteclient(client, MSG_NOTIMPLEMENTED);
	}
	return 0;
}

