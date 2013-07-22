/*
 * socket.c  Lowlevel socket server/client routines
 * 
 * $Id: socket.c,v 1.60 2010-12-14 11:31:43 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

#undef RL_DEBUG 


PROTO void init_ratelimiter_prof(CLIENT *cl) 
{
	cl->RateIntervalUs  = 1000000 / cfg.RatePeriod;
	gettimeofday(&cl->profile->rl_start, NULL);
	cl->profile->rl_sleep = RL_INITVAL;
}


PROTO int check_ratelimits_prof(CLIENT *cl) 
{
	struct timeval 	sleep, now;
	unsigned long 	timetaken, bytesdone;
	double 		bps;

	PROFILE *profile = cl->profile;

	if ( cl->profile->MaxProfileBPS == 0 ) 
		return 0;

	gettimeofday(&now, NULL);
	timersub(&now, &profile->rl_start, &sleep);
	timetaken = (sleep.tv_sec * 1000000) + sleep.tv_usec;

	if ( timetaken > cl->RateIntervalUs ) 
	{
		gettimeofday(&profile->rl_start, NULL);

		bytesdone = profile->rl_curbytes - profile->rl_lastbytes;
		profile->rl_lastbytes = profile->rl_curbytes;

		/* calculate a reasonable average bps */
		bps = ((double)1000000 / timetaken) * bytesdone; 
		profile->rl_curbps = ((profile->rl_curbps * 2) + bps ) / 3;

		/*
		 * Now just flip/flop the delay. If we're going too fast,
		 * sleep 1.5 times more. If we're going too slow, sleep 1.5 
		 * times less. Eventually this value will tune itself.
		 */
		if ( profile->rl_curbps > cl->profile->MaxProfileBPS ) 
		{
			profile->rl_sleep *= profile->ProfileRLFactorD;
#ifdef RL_DEBUG
			syslog(LOG_DEBUG, "sleep cl - %lf, bps = %lf, tt = %lu, bd = %lu", 
				user->rl_sleep, user->rl_curbps, timetaken, bytesdone);
#endif
		}else{
			profile->rl_sleep /= profile->ProfileRLFactorU;
#ifdef RL_DEBUG
			syslog(LOG_DEBUG, "sleep cl + %lf, bps = %lf, tt = %lu, bd = %lu", 
				user->rl_sleep, user->rl_curbps, timetaken, bytesdone);
#endif
		}
	}

	/*
	 * Finally do the sleeping part, overruns can occur when unaccounted
	 * data is sent to the client. Underruns can occur when the process is often
	 * idle for example when reading text groups.
	*/

	if ( profile->rl_sleep > 3000000 ) 
	{
		profile->rl_sleep = 1000000;
	}else if ( profile->rl_sleep < 4000 ) 
	{
		profile->rl_sleep = RL_INITVAL;
	}
	usleep(profile->rl_sleep);

	return 0;
}


PROTO void init_ratelimiter(CLIENT *cl) 
{
	cl->RateIntervalUs  = 1000000 / cfg.RatePeriod;
	gettimeofday(&cl->user->rl_start, NULL);
	cl->user->rl_sleep = RL_INITVAL;
}


PROTO int check_ratelimits(CLIENT *cl) 
{
	struct timeval 	sleep, now;
	unsigned long 	timetaken, bytesdone;
	double 		bps;
	USER *user    = cl->user;

	if ( cl->user->MaxUserBPS == 0 ) 
		return 0;

	gettimeofday(&now, NULL);
	timersub(&now, &user->rl_start, &sleep);
	timetaken = (sleep.tv_sec * 1000000) + sleep.tv_usec;

	if ( timetaken > cl->RateIntervalUs ) 
	{
		gettimeofday(&user->rl_start, NULL);

		bytesdone = user->rl_curbytes - user->rl_lastbytes;
		user->rl_lastbytes = user->rl_curbytes;

		/* calculate a reasonable average bps */
		bps = ((double)1000000 / timetaken) * bytesdone; 
		user->rl_curbps = ((user->rl_curbps * 2) + bps ) / 3;

		/*
		 * Now just flip/flop the delay. If we're going too fast,
		 * sleep 1.5 times more. If we're going too slow, sleep 1.5 
		 * times less. Eventually this value will tune itself.
		 */
		if ( user->rl_curbps > cl->user->MaxUserBPS ) 
		{
			user->rl_sleep *= cl->profile->UserRLFactorD;
#ifdef RL_DEBUG
			syslog(LOG_DEBUG, "sleep cl - %lf, bps = %lf, tt = %lu, bd = %lu", 
				user->rl_sleep, user->rl_curbps, timetaken, bytesdone);
#endif
		}else{
			user->rl_sleep /= cl->profile->UserRLFactorU;
#ifdef RL_DEBUG
			syslog(LOG_DEBUG, "sleep cl + %lf, bps = %lf, tt = %lu, bd = %lu", 
				user->rl_sleep, user->rl_curbps, timetaken, bytesdone);
#endif
		}
	}

	/*
	 * Finally do the sleeping part, overruns can occur when unaccounted
	 * data is sent to the client. Underruns can occur when the process is often
	 * idle for example when reading text groups.
	*/

	if ( user->rl_sleep > (RL_INITVAL*2) ) {
		user->rl_sleep = RL_INITVAL;
	}else if ( user->rl_sleep < 100 ) {
		user->rl_sleep = 100;
	}
	usleep(user->rl_sleep);

	return 0;
}


PROTO void set_errormsg(char *str, ...) 
{
	va_list ap;

	syslog(LOG_DEBUG, "set_errormsg called: %s", str);

	if ( client->error > 0 ) 
	{
		/* we probably had a broken pipe somewhere by now.. */
		syslog(LOG_ERR, "set_errormsg called twice %s", str);
		return;
	}

	if ( (client->errstr = calloc(1,512)) == NULL )
		die("No memory for errstr");

	client->error++;
	va_start(ap, str);
	vsprintf(client->errstr, str, ap);
	va_end(ap);

	if ( client->connected == 1 )
	{
		client->connected = 0;
		if ( client->groupserver != NULL )
			client->groupserver->connections--;
		client->groupserver = NULL;
	}
}


PROTO ssize_t read_socket(int socket, char *buf, int size, int timeout) 
{
	fd_set rdfs;
	struct timeval tv;

	FD_ZERO(&rdfs);
	FD_SET(socket, &rdfs);

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	if ( select(socket+1, &rdfs, NULL, NULL, &tv) == 0 ) 
	{
		syslog(LOG_DEBUG, "read_socket: select timeout");
		return -1;
	}

	ssize_t nread =	read(socket, buf, size);
	return nread;
}


PROTO ssize_t client_read_socket(CLIENT* client, char *buf, int size, int timeout) 
{
	if ( client->useSSL )
	{
	    return ssl_read_client_timeout (client, buf, size, timeout);
	}
	else
	{
	    fd_set rdfs;
	    struct timeval tv;

	    FD_ZERO(&rdfs);
	    FD_SET(client->socket, &rdfs);

	    tv.tv_sec  = timeout;
	    tv.tv_usec = 0;

	    if ( select(client->socket+1, &rdfs, NULL, NULL, &tv) == 0 ) 
	    {
		syslog(LOG_DEBUG, "client_read_socket: select timeout");
		return -1;
	    }

	    ssize_t nread = read(client->socket, buf, size);
	
	    return nread;
	}
}


PROTO ssize_t write_socket(int socket, char *buf, int size, int timeout) 
{
	fd_set rdfs;
	struct timeval tv;

	FD_ZERO(&rdfs);
	FD_SET(socket, &rdfs);

	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	if ( select(socket+1, NULL, &rdfs, NULL, &tv) == 0 ) 
	{
		syslog(LOG_DEBUG, "write_socket: select timeout");
		return -1;
	}

	return write(socket, buf, size);
}

PROTO char* fgetsrn(char* ptr, int maxlen, int fd)
{
	int	n, rc;
	char	c;

	for (n = 1; n < maxlen; n++) 
	{
		if ( (rc = read(fd, &c, 1)) == 1) 
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (n == 1)
				return(NULL);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		} else {
			if ( errno ) set_errormsg("fgetsrn: %m");
			return(NULL);	/* error */
		}
	}

	*ptr = 0;
	return(ptr);
}


/* 
 * readserver() read a single line from the currently selected server
 */
PROTO char *readserver(CLIENT *client, char *buf, int size) 
{
	fd_set rdfs;
	struct timeval tv;
	size_t bread;

	FD_ZERO(&rdfs);
	FD_SET(client->serversock, &rdfs);

	tv.tv_sec  = cfg.ServerReadTimeout;
	tv.tv_usec = 0;

	if ( select(client->serversock+1, &rdfs, NULL, NULL, &tv) == 0 ) 
	{
		set_errormsg("remote server %s read timeout", client->currserver->Name);
		return NULL;
	}

	bread = read(client->serversock, buf, size);
	if ( bread == -1 ) 
	{
		set_errormsg("cant read from server %s %m", client->currserver->Name);
		bread=0;
	}
	buf[bread] = 0;

	if ( cfg.LogReadserver )
		syslog(LOG_DEBUG,"readserver: %s", buf);

	client->serverbytes += (ulong)bread;
	return buf;
}


/* 
 * nreadserver() same as readserver but return number of bytes read (or 0)
 */
PROTO int nreadserver(CLIENT *client, char *buf, int size) 
{
	fd_set rdfs;
	struct timeval tv;
	size_t bread;

	FD_ZERO(&rdfs);
	FD_SET(client->serversock, &rdfs);

	tv.tv_sec  = cfg.ServerReadTimeout;
	tv.tv_usec = 0;

	if ( select(client->serversock+1, &rdfs, NULL, NULL, &tv) == 0 ) 
	{
		set_errormsg("remote server %s read timeout", client->currserver->Name);
		return 0;
	}

	bread = read(client->serversock, buf, size);
	if ( bread == -1 ) 
	{
		set_errormsg("cant read from server %s %m", client->currserver->Name);
		return 0;
	}
	buf[bread] = 0;

	if ( cfg.LogReadserver )
		syslog(LOG_DEBUG,"readserver: %s", buf);

	client->serverbytes += (ulong)bread;
	return bread;
}

PROTO char *readserverline(char *buf, int size) 
{
	fd_set rdfs;
	struct timeval tv;

	FD_ZERO(&rdfs);
	FD_SET(client->serversock, &rdfs);

	tv.tv_sec  = cfg.ServerReadTimeout;
	tv.tv_usec = 0;

	if (select(client->serversock+1, &rdfs, NULL, NULL, &tv) == 0) 
	{
		set_errormsg("remote server %s read timeout", client->currserver->Name);
		return NULL;
	}

	return fgetsrn(buf,size,client->serversock);
}

/* 
 * readclient() read a single line from the client
 */
PROTO char *readclient(CLIENT *client, char *buf, int size, int nowait) 
{
    if (client->useSSL)
    {
	return ssl_readclient (client, buf, size, nowait);
    }
    else
    {
	fd_set rdfs;
	struct timeval tv;

	if ( ! nowait ) 
	{
		FD_ZERO(&rdfs);
		FD_SET(client->socket, &rdfs);

		tv.tv_sec  = client->profile->ClientReadTimeout;
		tv.tv_usec = 0;

		if ( select(client->socket+1, &rdfs, NULL, NULL, &tv) == 0 ) 
		{
			client->timeout++;
			return NULL;
		}
	}
	return fgetsrn(buf,size,client->socket);
    }
}

 
/* linelen (str) 
// return the length of str
// without the trailing \r, \n or \r\n (if any)
*/
static int linelen(const char str[])
{
	int len = strlen(str);
	if (len == 0) return 0;

	char last = str[len - 1];

	if (last == '\n' || last == '\r')
	{
		if (len > 1) 
		{
			char prelast = str[len - 2];
			if ((last != prelast) && (prelast == '\n' || prelast == '\r'))
				len--;
		}
		len--;
	}

	return len;
}

/* writeln (fd, str, len)
// writes len characters from str plus \r\n to fd
// return number of characters written
// which should be len + 2
*/
static int writeln(int fd, const char str[], int len)
{
	struct iovec msg[2];

	msg[0].iov_base = (void *)str;
	msg[0].iov_len = len;

	msg[1].iov_base = "\r\n";
	msg[1].iov_len = 2;

	return writev(fd, msg, 2);
}

/* writeserver (client, fmt, ...)
// write a line to the currently selected server
// return the number of bytes written
*/
PROTO int writeserver(CLIENT *client, const char fmt[], ...) 
{
	va_list ap;
	char request[cfg.BufSize];
	int nwrite;

	va_start(ap, fmt);
	vsprintf(request, fmt, ap);
	va_end(ap);

	if (cfg.LogWriteserver)
		syslog(LOG_DEBUG, "writeserver %s", request);

	nwrite = writeln(client->serversock, request, linelen(request));
	if (nwrite != linelen(request) + 2)
	{
		set_errormsg("cant write to server %s %m", client->currserver->Name);
		return 0;
	}

	return nwrite;
}

/*
// swriteclient (client, str)
// write str plus \r\n to the client
// return 0 on success, -1 on failure
*/
PROTO int swriteclient(CLIENT *client, const char str[]) 
{
	if (cfg.LogWriteclient && atoi(str) > 199)
		syslog(LOG_DEBUG, "swriteclient %s", str);

	int ret = 0;
	if (client->useSSL) 
	{
	    ret = ssl_writeln(client->ssl, (char *)str, linelen(str));
	}
	else
	{
	    ret = writeln(client->socket, str, linelen(str));
	}	    

	if (ret != linelen(str) + 2) 
	{
    	    set_errormsg("cant swrite to client %m");
	    return -1;
	}

	return 0;
}

/*
 * writeclient() write to the client
 * return 0 on success, -1 on failure
 */
PROTO int writeclient(CLIENT *client, char *str, ...) 
{
	va_list ap;
	char response[cfg.BufSize];

	va_start (ap, str);
	vsprintf (response, str, ap);
	va_end (ap);

	return swriteclient(client, response);
}

/*
 * slwriteclient() statically write to the client and ratelimit
 * return 0 on success, -1 on failure
 * no vargs on this one
 */
PROTO int slwriteclient(CLIENT *client, char *str, size_t size, int limit) 
{
	fd_set rdfs;
	struct timeval tv;

	FD_ZERO(&rdfs);
	FD_SET(client->socket, &rdfs);

	tv.tv_sec  = client->profile->ClientReadTimeout;
	tv.tv_usec = 0;

	if ( select(client->socket+1, NULL, &rdfs, NULL, &tv) == 0 )
	{
		set_errormsg("slwrite timeout");
		return -1;
	}

	if ( client->useSSL )
	{
	    if ( ssl_write (client->ssl, str, size) <= 0 ) 
	    {
		return -1;
	    }
	}
	else
	{
	    if ( (write(client->socket, str, size)) == -1 ) 
	    {
		set_errormsg("cant slwrite to client %m");
		return -1;
	    }
	}

	client->profile->rl_curbytes += size;
	client->user->rl_curbytes +=size;
	client->profile->bytes += size;
	client->profile->rbytes += size;

	/* rate limit */
	if ( limit )
	{
		check_ratelimits(client);
		check_ratelimits_prof(client);
	}
	return 0;
}

/**
 **
 **  Server Connection Functions
 **
 **/

int alarm_flag = 0;

void alarm_func(int sig)
{
	alarm_flag++;
}

PROTO int connect_socket(char *server, int port) 
{
	struct sockaddr_in name;
	struct hostent *hostinfo;
	int sock;
	struct sigaction alrm;

	syslog(LOG_DEBUG, "connecting to server %s", server);

	sock = socket(PF_INET, SOCK_STREAM, 0);

	name.sin_family = AF_INET;
	if ( (hostinfo = gethostbyname(server)) == NULL ) 
	{
		close(sock);
		return -1;
	}

	name.sin_addr = *((struct in_addr *)hostinfo->h_addr);
	name.sin_port = htons(port);

	/* setup alarm for connect() timeout */
	memset(&alrm,0,sizeof(alrm));
	alrm.sa_handler = &alarm_func;
	sigaction(SIGALRM, &alrm, NULL);
	alarm_flag = 0;
	alarm(cfg.ServerConnectTimeout);

	if ( connect(sock, (struct sockaddr *) &name, sizeof(struct sockaddr)) == -1 ) 
	{
		if ( alarm_flag > 0 )
			syslog(LOG_ERR, "connect to server %s failed: timeout received", server);
		else
			syslog(LOG_ERR, "connect to server %s failed: %m", server);
		close(sock);
		alarm(0);
		signal(SIGALRM, SIG_DFL);
		return -1;
	}

	alarm(0);
	signal(SIGALRM, SIG_DFL);
	return sock;
}

PROTO char *handshake_nntp (int sock, char response[cfg.BufSize], int timeout) 
{
	int n;

	if ((n = read_socket (sock, response, cfg.BufSize, timeout)) <= 0)
		return "Connection Timed Out";

	response[n] = 0;

	if (atoi (response) >= 300)
		return response;

	return NULL;
}		     

static char *connect_auth_nntp(CLIENT *client, SERVER *server
		, char sreply[cfg.BufSize])
{
	writeserver(client, "AUTHINFO USER %s", server->Username);
	if (readserver(client, sreply, cfg.BufSize) == NULL)
		return MSG_AUTH_ERR;

	writeserver(client, "AUTHINFO PASS %s", server->Password);
	if (readserver(client, sreply, cfg.BufSize) == NULL)
		return MSG_AUTH_ERR;

	if (atoi(sreply) != 281) 
		return sreply;

	return NULL;
}

PROTO int connect_server(CLIENT *client, SERVER *server)
{
	int sock;

	if ( (sock=connect_server_nntp(client, server->Hostname, server->Port, server->Timeout)) == -1 )
		return 1;

	client->serversock = sock;
	client->connected = 1;
	client->serverarts = 0;
	client->serverbytes = 0;

	client->currserver = server;

	if (server->Username[0] != 0 && server->Password[0] != 0) 
	{
		char sreply[cfg.BufSize];

		if (connect_auth_nntp(client, server, sreply) != NULL) 
		{
			syslog(LOG_ERR, "%s: remote server %s auth response %s"
					, client->hostname, server->Hostname, sreply);

			close(client->serversock);

			if (client->connected) 
			{
				client->connected = 0;
				server->connections--;
			}

			client->currserver = NULL;
			return 1;
		}
	}

	server->connections++;
	return 0;
}

PROTO int connect_server_nntp(CLIENT *client, char *server, int port, int timeout) 
{
	int sock;

	if ((sock = connect_socket(server, port)) == -1 )
		return -1;

	char response[cfg.BufSize];
	response[0] = '\0';
	char *err = handshake_nntp(sock, response, timeout);

	if (err != NULL) 
	{
		syslog(LOG_ERR , "%s: error remote server %s:%d %s"
				, client->hostname, server, port, err);
		close(sock);
		return -1;
	}

	if (cfg.SockBufSize != 0) 
	{
		int socksize = cfg.SockBufSize;

		if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF
					, (char *)&socksize, sizeof (socksize)) < 0)
		{
			syslog (LOG_ERR, "%s: cant set server so_rcvbuf for %s (%m)"
					, server, client->hostname);
			close (sock);
			return -1;
		}
	}

	return sock;
}

static void send_quit(CLIENT *client) 
{
	if ( client->connected == 1 ) 
	{
		if ( ! writeserver(client, "QUIT") ) return;
		if ( ! readserver(client, client->bbuf, cfg.BufSize) ) return;
	}
}


/*
 * disconnect_server() disconnect from currently selected backend news server
 */
PROTO int disconnect_server(CLIENT *client) 
{
	if ( client->connected == 0 ) return 0;

	send_quit(client);

	if ( client->groupserver != NULL )
	{
		client->groupserver->connections--;
		client->groupserver = NULL;
	}

	client->group = NULL;

	if ( close(client->serversock) == -1 )
		syslog(LOG_ERR, "close() failed on server close");

	client->connected=0;
	syslog(LOG_DEBUG, "disconnected from server %s articles %u bytes %llu", 
		client->currserver->Name, client->serverarts, client->serverbytes);
	return 0;
}

PROTO int quiet_disconnect_server(CLIENT *client) 
{
	if ( client->groupserver != NULL ) {
		client->groupserver->connections--;
		client->groupserver = NULL;
	}

	close(client->serversock);
	client->connected=0;
	return 0;
}

/*
 * Resolve host:service client is connecting to.
 */
PROTO int tcp_sock2host(int fd, char * buf, int buflen, char * servbuf, char * hostbuf)
{
    struct sockaddr_storage ss_peer, ss_sock;
    socklen_t sslen = sizeof(struct sockaddr_storage);
    int n;

    /* FIXME: nr of open file in system can break this. return -1 but program crashes on client_close() */

    /* get client ip address */
    if ( getpeername(fd, (struct sockaddr *)&ss_peer, &sslen) == -1 ) {
        syslog(LOG_CRIT, "getpeername() failed: %m");
        return -1;
    }
    if ( (n=getnameinfo((struct sockaddr *)&ss_peer, sslen, buf, buflen, NULL, 0, NI_NUMERICHOST|NI_NUMERICSERV) != 0) ) {
        syslog(LOG_CRIT, "peername getnameinfo() failed: %m");
        return -1;
    }

    /* get server connection port */
    if ( getsockname(fd, (struct sockaddr *)&ss_sock, &sslen) == -1 ) {
        syslog(LOG_CRIT, "getsockname() failed: %m");
        return -1;
    }
    if ( (n=getnameinfo((struct sockaddr *)&ss_sock, sslen, hostbuf, NI_MAXHOST, servbuf, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV) != 0) ) {
        syslog(LOG_CRIT, "sockname getnameinfo() failed: %m");
        return -1;
    }
    return n;
}

/* vim: set ts=8 noexpandtab :*/
