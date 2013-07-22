/*
 * NNTPSwitch PostgreSQL Authenticator
 * $Id: auth_postgres.c,v 1.5 2006-08-08 14:00:09 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <postgresql/libpq-fe.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"
 

PROTO void auth_postgres(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	PGconn     *conn;
	PGresult   *res;
	char	   tmp[512];

	/* Make a connection to the database */
	conn = PQconnectdb(args);

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK)
	{
		syslog(LOG_ERR, "auth_postgres: connect to database %s failed: %s", PQdb(conn), PQerrorMessage(conn));
		PQfinish(conn);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	sprintf(tmp, "SELECT login FROM users WHERE login = '%s' AND password = '%s'",
			authres->username,
			authres->password);

	res = PQexec(conn, tmp);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		syslog(LOG_ERR, "auth_postgres: select command failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( PQntuples(res) == 0 ) {
		PQclear(res);
		PQfinish(conn);
		authres->message = strdup(MSG_AUTH_REJ);
		return;
	}

	PQclear(res);
	PQfinish(conn);

	authres->authenticated = true;
	authres->message = strdup(MSG_AUTH_OK);
	return;
}
