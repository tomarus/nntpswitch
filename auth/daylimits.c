/*
 * $Id: daylimits.c,v 1.1 2010-11-23 13:45:23 tommy Exp $
 * (c) 2010 News-Service Europe bv
 *
 * Generic daylimit functions for use in other acct/auth modules.
 */

#include "../aconfig.h"
#include "../log.h"
#include "../nntpd.h"
#include "../aprotos.h"
#include "log_acct.h"
#include <postgresql/libpq-fe.h>

static void dl_unlock(void)
{
	master->dllock = 0;
}

static void dl_waitlock(void)
{
	while(master->dllock > 0){usleep(200);};
	master->dllock++;
}

PROTO int get_kbit_from_gb(int gb, PROFILE *pf)
{
	int i;

	for (i=pf->NumLimits-1; i>=0; i--)
		if ( gb >= pf->Limits[i].gb )
			return pf->Limits[i].kbit;

	syslog(LOG_ERR, "get_kbit_from_gb: GB used is not configured (%d gb)", gb);
	return 0;
}

/* Update miniac daylimit accounting table */
PROTO void do_acct_daylimit(CLIENT *client, CONFIG *cfg, char *args)
{
	PGconn *conn;
	PGresult *res;
	char tmp[512];

	/* Make a connection to the database */
	conn = PQconnectdb(args);

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		syslog(LOG_ERR, "do_acct_daylimit: connect to database %s failed: %s", PQdb(conn), PQerrorMessage(conn));
		PQfinish(conn);
		return;
	}

	dl_waitlock();
	sprintf(tmp, "SELECT login FROM miniac WHERE login = '%s'", client->username);

	res = PQexec(conn, tmp);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		syslog(LOG_ERR, "do_acct_daylimit: select command failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		return;
	}

	if ( PQntuples(res) == 0 ) {
		PQclear(res);
		/* no account found, insert new into database */
		sprintf(tmp, "INSERT INTO miniac(login, bytes, profile) VALUES('%s', %lld, '%s')",
			client->username, client->bytes, client->profile->Name);
		res = PQexec(conn, tmp);

	} else {
		PQclear(res);
		/* account exists, update into database */
		sprintf(tmp, "UPDATE miniac SET bytes = bytes + %lld WHERE login = '%s'", client->bytes, client->username);
		res = PQexec(conn, tmp);
	}

	dl_unlock();
	PQclear(res);
	PQfinish(conn);
}


/* Get current kbit from miniac table */
PROTO int get_userkbit(CONFIG *cfg, char *args, char *user, PROFILE *profile)
{
	PGconn *conn;
	PGresult *res;
	char tmp[512];
        int userkbit = 0;

	/* Make a connection to the database */
	conn = PQconnectdb(args);

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		syslog(LOG_ERR, "get_userkbit: connect to database %s failed: %s", PQdb(conn), PQerrorMessage(conn));
		PQfinish(conn);
		return 0;
	}

	/* ispid 15 is demo account */
	sprintf(tmp, "SELECT bytes FROM miniac WHERE login = '%s'", user);

	res = PQexec(conn, tmp);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		syslog(LOG_ERR, "get_userkbit: select(1) command failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		return 0;
	}

	if ( PQntuples(res) == 1 ) {
	        userkbit = get_kbit_from_gb( atoll(PQgetvalue(res, 0, 0)) / (1000*1000*1000), profile);
	}

	PQclear(res);
	PQfinish(conn);
	return userkbit;
}

