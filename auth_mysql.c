/*
 * NNTPSwitch MySQL Authenticator
 * $Id: auth_mysql.c,v 1.6 2006-08-08 14:00:09 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"
 
PROTO void auth_mysql(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	MYSQL demo_db;
	MYSQL_RES *res;
	char *p, *ap;

	char query[256];
	char dbname[32] = "\0";
	char hostname[128] = "\0";
	char username[32] = "\0";
	char password[32] = "\0";

	ap = strdup(args);
	p = strtok(ap, ",: ");
	do {
		if ( strncasecmp(p, "dbname", 6) == 0 )
			strncpy(dbname, p+7, 32);
		else
		if ( strncasecmp(p, "hostname", 8) == 0)
			strncpy(hostname, p+9, 128);
		else
		if ( strncasecmp(p, "username", 8) == 0)
			strncpy(username, p+9, 32);
		else
		if ( strncasecmp(p, "password", 8) == 0)
			strncpy(password, p+9, 32);
		else
			syslog(LOG_ERR, "auth_mysql: unknown connect string keyword %s", p);
	} while( (p=strtok(NULL,",: ")) );

	if ( dbname[0] == 0 || hostname[0] == 0 ) 
	{
		syslog(LOG_ERR, "auth_mysql: Invalid argument syntax, atleast hostname and dbname are required");
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	mysql_init(&demo_db);

	if( ! mysql_real_connect(&demo_db, hostname, username, password, dbname, 0, NULL, CLIENT_FOUND_ROWS) ) 
	{
		syslog(LOG_ERR, "auth_mysql: could not connect to mysql %s", mysql_error(&demo_db));
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}
	
	sprintf(query, "SELECT login FROM users WHERE login = '%s' AND password = '%s'",
			authres->username,
			authres->password);

	if( mysql_real_query(&demo_db, query, 255)) 
	{
		syslog(LOG_ERR, "auth_mysql: could not select query: %s", mysql_error(&demo_db));
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	res = mysql_store_result(&demo_db);
	if ( mysql_num_rows(res) == 0 ) 
	{
		mysql_free_result(res);
		mysql_close(&demo_db);
		authres->message = strdup(MSG_AUTH_REJ);
		return;
	}

	mysql_free_result(res);
	mysql_close(&demo_db);

	authres->authenticated = true;
	authres->message = strdup(MSG_AUTH_OK);
	return;
}
