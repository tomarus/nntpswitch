/*
 * auth.c - Authentication and Accounting Configuration
 *
 * $Id: auth.c,v 1.18 2008-08-26 11:45:46 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "memory.h"
#include "aprotos.h"
#include "modmap.h"


/* 
 * AuthResult structure operations
 */

PROTO AUTHRESULT * new_authresult(CLIENT * client)
{
	AUTHRESULT *a;

	if ( (a=malloc(sizeof(AUTHRESULT)) ) == NULL )
	{
		return NULL;
	}

	a->username = strdup(client->username);
	a->password = strdup(client->password);
	a->username_s = strippat(a->username, client->auth->mask);
	a->logname = NULL;

	a->hostname = strdup(client->hostname);
	a->port = client->id + 1;

	a->in_addr = client->addr.sin_addr;

	a->authenticated = false;
	a->message       = NULL;
	a->profile       = NULL;
	a->posting       = client->acl->post;
	a->bytes         = 0;
	a->userkbit      = 0;
	a->readpat       = NULL;
	a->postpat       = NULL;
	a->timeleft      = 0;    /* MaxSessionTime */
	a->overquota     = false;
	a->disabled      = false;
	a->posthost      = NULL;
	a->postabuse     = NULL;
	a->postorg       = NULL;

	a->args = master->args;
	a->numargs = master->numargs;

	return a;
}


PROTO void free_authresult(AUTHRESULT *a)
{
	free(a->username);
	free(a->password);
	free(a->hostname);

	/* FIXME: the strings from strippat can not be freed, so this is a memleak */
	// if ( a->username_s != NULL ) free(a->username_s);
	
	if ( a->logname != NULL ) free(a->logname);
	if ( a->profile != NULL ) free(a->profile);
	if ( a->message != NULL ) free(a->message);
	if ( a->readpat != NULL ) free(a->readpat);
	free(a);
}


/*
 * Authentication/Accounting Modules Operations
 */

PROTO void (*find_auth_mod(char *name))()
{
	int i;

	for(i = 0; i < sizeof(authmap) / sizeof(authmap[0]); i++) 
	{
		if (strcmp(name, authmap[i].name) == 0)
			return authmap[i].ptr;
	}

	return NULL;
}


PROTO void (*find_acct_mod(char *name))()
{
	int i;

	for(i = 0; i < sizeof(acctmap) / sizeof(acctmap[0]); i++) 
	{
		if (strcmp(name, acctmap[i].name) == 0)
			return acctmap[i].ptr;
	}

	return NULL;
}

