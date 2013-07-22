/*
 * NNTPSwitch client port and user handling routines
 * $Id: client.c,v 1.29 2010-07-06 11:25:53 tommy Exp $
 */

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

#include "log.h"
#include "semaphore.h"

/*
 * Free the port the client was connected to
 */

PROTO void free_client(int clientid)
{
	memset(master->clients+clientid, '\0', sizeof(CLIENT));
}

PROTO ulong get_ip_from_user(USER *user)
{
	int i;
	for (i=0; i<MAX_PROC; i++) {
		if ( (master->clients+i)->user == user ) {
			return (master->clients+i)->ip4addr;
		}
	}

	return 0;
}


/*
 * Find and allocate a client slot from the client's pool accepting
 * the concurrent connection limits. A client slot needs to be
 * reserved before we can fork() the client in it's own process.
 */

PROTO CLIENT* find_slot(ulong ip4addr, int *concur, int maxcon)
{
	int i;
	int cc=0;
	CLIENT *cl;

	/* check for both a free slot and the number of
	 * concurrent sessions at once to speed things up */

	for (i=0; i<MAX_PROC; i++) {
		if ( (master->clients+i)->ip4addr == ip4addr )
		cc++;

		if ( (master->clients+i)->inuse == 0 ) {
			cl = (master->clients+i);   // set client pointer to one from the list
			memset(cl, '\0', sizeof(CLIENT));
			cl->inuse++;
			cl->id = i;
			cl->ip4addr = ip4addr;

			/* check for more concurrent connections */
			for ( ; i<MAX_PROC; i++) {
				if ( (master->clients+i)->ip4addr == ip4addr )
					cc++;

				if ( maxcon > 0 && cc > maxcon ) {
					(*concur) = cc;
					cl->inuse = 0;
					cl->id = 0;
					cl->ip4addr = 0;
					return NULL;
				}
			}

			(*concur) = cc;
			cl->concur = cc;
			cl->starttime = time(NULL);
			return cl;
		}
	}
	syslog(LOG_ERR, "Too many slots requested, increase MAX_PROC in nntpd.h");
	return NULL;
}


/*
 * Find a user id matching the key or create a new one
 * if he isn't connected yet.
 */

PROTO USER* find_user(char *ky, PROFILE *prf)
{
	int i, id = -1;
	USER *u;

	/* check if its a known user */
	for (i=0; i<MAX_USERS; i++)
		if ( strcmp((master->users+i)->key, ky) == 0 ) {
			((master->users)+i)->connections++;
			return (master->users+i);
		}

	/* find first empty user slot */
	for (i=0; i<MAX_USERS; i++)
		if ( (master->users+i)->key[0] == 0 ) {
			id = i;
			break;
		}

	/* we're full */
	if ( id == -1 )
		return NULL;

	/* initialize the slot */
	u = (master->users)+id;
	memset(u, '\0', sizeof(USER));

	strcpy(u->key, ky);
	u->connections++;
	u->id = id;
	u->bytesleft = prf->MaxSessionBytes;
	strcpy(u->readpat, prf->ReadPat);
	strcpy(u->postpat, prf->PostPat);
	u->timeleft = prf->MaxSessionTime;

	prf->numusers++;
	master->numusers++;

	return u;
}

/*
 * Drop a connection from a user, see if the user reaches 0 connections
 * and optionally dropping the user itself.
 */

PROTO void drop_user(uint id, PROFILE *prf)
{
	USER *u;

	u = (master->users+id);
	u->connections--;

	if ( u->connections == 0 ) {
#ifdef USE_REALUSER
		if ( u->realuser == 1 )
			prf->realusers--;
#endif
		memset(u, '\0', sizeof(USER));
		prf->numusers--;
		master->numusers--;
	}
}


/*
 * Close a connection. Do accounting and logging stuff
 */

PROTO void close_child(bool doacct)
{
	if ( doacct == true )
	{
		client_log_statistics();

		if ( nullstr(client->username) )
			strcpy(client->username, client->hostname);

		if ( nullstr(client->logname) )
			strcpy(client->logname, client->username);

		void (*acctmod)() = find_acct_mod(client->profile->Accounting->lib);

		if ( acctmod != NULL ) {
			(*acctmod)(client, &cfg, client->profile->Accounting->args);
			/* force syslog reopen 
			 * FIXME: hoeft niet meer sinds geen modules meer enzo, ff nalopen */
			syslog_open("nntpswitchd", LOG_PID, LOG_NEWS);
		} else {
			syslog(LOG_ERR, "Can't find acctmap: %s"
					, client->profile->Accounting->lib);
			syslog(LOG_NOTICE,"%s: accounting vanished", client->hostname);
		}

		/* If client reaches max bytes, sleep a few sec in order for acct to be processed */
		if (client->user->bytesleft < 0) {
		    sleep(cfg.AccountingDelay);
		}
	}

//	syslog(LOG_DEBUG, "%s: dropping connection profile %s", client->hostname, client->profile->Name);

	if (client->useSSL) {
		if (client->ssl) {
			ssl_close(client->ssl);
		}
	}

	if ( (client->profile->ASMaxConns > 0 || client->profile->ASMaxHosts > 0) && client->authenticated > 0 ) {
		http_accountsharing_dropuser(client->username, client->hostname);
	}

	if ( cfg.JSONACL ) {
	    freehttpacl(client->acl);
	}

	semlock_lock(master->semid);

	if ( client->profile->connections > 0 )
		client->profile->connections--;
	else
		syslog(LOG_ERR, "Trying to drop 0 connections profile: %s", client->profile->Name);

	if ( client->user != NULL )
		drop_user(client->user->id, client->profile);
	else
		syslog(LOG_ERR, "close_child: user == NULL");
	free_client(client->id);
	master->connections--;

	semlock_unlock(master->semid);

	return;
}


/*
 * Find the profile a CLIENT has given a USER key.
 */

PROTO PROFILE * find_user_profile(char *key)
{
	int i,j;

	for ( i=0; i<MAX_USERS; i++) {
		if ( strcmp((master->users+i)->key, key) == 0 ) {
			for ( j=0; j<MAX_PROC; j++ ) {
				if ( (master->clients+j)->user == master->users+i ) {
					return (master->clients+j)->profile;
				}
			}
		}
	}

	return NULL;
}

