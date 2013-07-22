/*
 * Main daemon loop, main () and signal handlers
 * $Id: daemon.c,v 1.69 2010-06-29 09:05:18 tommy Exp $
 */

#include "aconfig.h"
#include "memory.h"
#include "nntpd.h"
#include "tgetopt.h"
#include "aprotos.h"

#include "log.h"
#include "semaphore.h"
#include "setproctitle.h" 

static CURL * curl;
static long int MASTER_SEMKEY = 3333L;

struct option opts[] = 
{
	{ "basedir",1, NULL, 'b', 0 },
	{ "config", 1, NULL, 'c', 0 },
	{ "help",   0, NULL, 'h', 0 },
	{ "stay",   0, NULL, 's', 0 },
	{ "test",   0, NULL, 't', 0 },
	{ NULL,     0, NULL,  0,  0 },
};

static const char usage[] = "Usage: %s [options]\n"
	"   -b, --basedir DIRECTORY\tiSpecify base directory - will change directory to that\n"
	"   -c, --config CONFIGFILE\tSpecify configfile instead of searching for it.\n"
	"   -h, --help             \tCommand usage.\n"
	"   -s, --stay             \tStay in forground.\n"
	"   -t, --test             \tTest configuration and exit.\n" ;

static int test_only /* = 0 */;
static int opt_stay /* = 0 */;
static volatile int reload /* = 0 */;
static volatile int terminate /* = 0 */;
static pid_t timerpid = (pid_t)0;


static int writepidfile (void)
{
	FILE *pidf;
	struct passwd *pwent;

	if ((pidf = fopen (cfg.PidFile, "w")) == NULL)
		die ("Can't write pidfile");

	fprintf (pidf, "%d", (int)getpid ());
	fclose (pidf);

	/* change ownership, so we can overwrite it later when we're
	 * forked under a different user (probably news) */

	if ((pwent = getpwnam (cfg.RunAsUser)) == NULL)
		die ("unknown user %s", cfg.RunAsUser);

	chown (cfg.PidFile, pwent->pw_uid, pwent->pw_gid);

	return 0;
}

static void closeall (fd_set keepfds)
{
	for (int fd = 0; fd < FD_SETSIZE; fd++) {
		if (!FD_ISSET (fd, &keepfds))
			close (fd);
	}
}

/* daemon () - detach process from user and disappear into the background  */
/* returns -1 on failure, but you can't do much except exit in that case, */
/* since we may already have forked. This is based on the BSD version,    */
/* so the caller is responsible for things like the umask, etc.           */
/* believed to work on all Posix systems */
static pid_t init_daemon (fd_set keepfds) 
{
	pid_t pid;
	switch ((pid = fork ())) {
		case 0:  break;
		case -1: return -1;
		default: _exit (0);	/* exit the original process */
	}

	closeall (keepfds);

	if (setsid () < 0)		/* shoudn't fail */
		return -1;

	return pid;
}

static int init_socket (uint16_t port)
{
	struct sockaddr_in ssin;
	int sock, flag = 1;

	info ("Socket on %s:%d", cfg.BindAddress, port);
	errno = 0;

	if ((sock = socket (PF_INET, SOCK_STREAM, 0)) == -1)
		die ("Can't create socket");

	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag)) < 0)
		die ("Can't set socket options");

	ssin.sin_family = AF_INET;
	ssin.sin_port = htons (port);
	ssin.sin_addr.s_addr = inet_addr (cfg.BindAddress);

	if (test_only) return sock;

	if (bind (sock, (struct sockaddr *)&ssin, sizeof (ssin)) < 0)
		die ("Bind failed");

	if (listen (sock, cfg.ListenBackLog) < 0)
		die ("Listen failed");

	info ("Listening on %s:%d", cfg.BindAddress, port);
	return sock;
}

static void init_sockets_from_port_list (const char* port_list, 	/* list of ports to connect */
                                         const char* param_name,	/* configuration parameter name */
					 fd_set *serverfds,	    	/* set of file descriptors */
					 int *maxfd,			/* max value of fd */
					 fd_set *sslfds )         	/* set of SSL descriptors - 
									   it adds ds to this set if it is not NULL 
									*/
{
	const char *ptr = port_list;
	char *end;

	while (*ptr) {
		unsigned long int ul = strtoul (ptr, &end, 10);
		if (end == ptr) die ("not a port at %s %s\n", param_name, ptr);
		if (ul <= 0) die ("port is negative, at %s %s\n", param_name, ptr);
		if (ul > 65536 ) die ("port is too high, at %s %s\n", param_name, ptr);
		ptr = end + strspn (end, " .,:/;");

		uint16_t port = (uint16_t)ul;
		int sock = init_socket (port);

		if (sock < 0) die ("init socket failed - on port %d", port);

		FD_SET (sock, serverfds);
		if (sslfds) FD_SET(sock, sslfds);
		
		if (sock > *maxfd) *maxfd = sock;
	}
}

static void init_sockets (fd_set *serverfds, fd_set *sslfds, int *maxfd)
{
    FD_ZERO (serverfds);
    FD_ZERO (sslfds);
    *maxfd = -1;
    init_sockets_from_port_list (cfg.ListenPorts, "ListenPorts", serverfds, maxfd, NULL);
    if (cfg.EnableSSL) 
	init_sockets_from_port_list (cfg.ListenSSLPorts, "ListenSSLPorts", serverfds, maxfd, sslfds);
}

static CLIENT *access_check (int rc, struct sockaddr_in ssin)
{
	struct stat _stat;
	ulong ipaddr;
	ACL *acl;
	CLIENT *retval;
	int curconcur;

	if (master->connections > cfg.MaxConnections) {
		write (rc, MSG_SERVERFULL, strlen (MSG_SERVERFULL));
		info ("Too many global connections, increase cfg.MaxConnections");
		return NULL;
	}

	if (stat (cfg.DisableFile, &_stat) == 0) {
		write (rc, cfg.DisableText, strlen (cfg.DisableText));
		write (rc, "\r\n", 2);
		return NULL;
	}

	errno = 0;

	ipaddr = ntohl (ssin.sin_addr.s_addr);

	if ( cfg.JSONACL ) {
		if ( (acl = gethttpacl(inet_ntoa(ssin.sin_addr), curl)) == NULL )
		{
			syslog(LOG_ERR, "Cant get JSON ACL.");
			write(rc, MSG_NOACL, strlen(MSG_NOACL));
			return NULL;
		}
	} else {
		if ( (acl = getacl(ipaddr)) == NULL )
		{
			syslog(LOG_ERR, "Cant get default ACL, define \"acl 0/0\" in access.conf");
			write(rc, MSG_NOACL, strlen(MSG_NOACL));
			return NULL;
		}
	}

	if ( ! nullstr(acl->profile->Disabled) )
	{
		write(rc, "400 ", 4);
		write(rc, acl->profile->Disabled, strlen(acl->profile->Disabled));
		return NULL;
	}

	if (acl->deny) {
		write(rc, MSG_NOPERM, strlen(MSG_NOPERM));
		info("denied connection from %s", inet_ntoa(ssin.sin_addr));
		return NULL;
	}

	if (acl->profile->connections > acl->profile->MaxConnections) {
		write(rc, MSG_TOOMANYCONNSPRF, strlen(MSG_TOOMANYCONNSPRF));
		return NULL;
	}

	semlock_lock (master->semid);
	retval = find_slot (ipaddr, &curconcur, acl->unlimit ? 0 : acl->profile->MaxHostConcurrent);
	semlock_unlock (master->semid);

	if (retval == NULL) {
		char buf[128];
		sprintf (buf, MSG_TOOMANY_CONCURRENT, acl->profile->MaxHostConcurrent);
		write (rc, buf, strlen (buf));
		syslog(LOG_DEBUG, "Too many concurrent connections from %s"
				, inet_ntoa (ssin.sin_addr));
		return NULL;
	}

	retval->acl = acl;
	memcpy (&retval->addr, &ssin, sizeof (ssin));
	retval->socket = rc;

	return retval;
}

static void daemon_reload ()
{
	reload_servers ();
	reloadactive ();
	reload_access_conf ();
	loadconfig ();
	reload = 0;
}

static void daemon_terminate ()
{
	if ( timerpid > 0 )
		kill(timerpid, SIGTERM);

	semlock_close (master->semid);
	syslog_close ();
	_exit (1);
}

static void daemon_accept (int fd, fd_set socketfds, fd_set sslfds, int maxfd)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof (addr);
	CLIENT *initclient;
	int rc = -1;

        rc = accept (fd, (struct sockaddr *)&addr, &addrlen);
	if (rc < 0) return;

	initclient = access_check (rc, addr);

//	uint port = ntohs (addr.sin_port);

	if (! initclient) {
		close (rc);
		return;
	}

	initclient->useSSL = FD_ISSET(fd, &sslfds);
	initclient->ssl    = NULL;

#if defined(_SC_NPROCESSORS_ONLN)
	if ( cfg.CoreBind > 0 )
	{
		/* Select CPU core to bind to */
		master->currcore++;
		if ( master->currcore >= master->numcores )
			master->currcore = 0;
		initclient->numcore = master->currcore;
	}
#endif

	switch (fork ()) {
	case 0:
		syslog_open ("nntpswitchd", LOG_PID, LOG_NEWS);
		for (int i = 0; i <= maxfd; i++) {
			if (FD_ISSET (i, &socketfds))
				close (i);
		}
		run_child (initclient);
		syslog_close ();
		_exit (0);
	case -1: 
		info ("couldnt fork child %s", strerror (errno));
	default:
                if ( cfg.JSONACL == 1 && initclient->acl != NULL ) { 
                    freehttpacl(initclient->acl);
                }
		close (rc); 
	}
}

static void daemon_select_loop (fd_set serverfds, fd_set sslfds, int maxfd)
{
	fd_set rdfds;
	int nselect;

	for (;;) {
		if (reload) daemon_reload ();
		if (terminate) daemon_terminate ();

		rdfds = serverfds;

		errno = 0;
		nselect = select (maxfd + 1, &rdfds, NULL, NULL, NULL);

		if (nselect == -1) {
			switch (errno)
			{
				case EINTR:
					break;
				default: 
					syslog(LOG_DEBUG, "select=%d err=%m", nselect); break;
			}
		} else { // nselect == 0 doesn't happen
			for (int fd = 0; fd <= maxfd && nselect > 0; fd++) {
				if (! FD_ISSET (fd, &rdfds)) continue;

				nselect--;
				daemon_accept (fd, serverfds, sslfds, maxfd);
			}
		}
	} 
}

static void daemon_chdir ()
{
	char *wd;

	if (toptset ('b')) wd = toptargs ('b');
	else wd = cfg.BasePath;

	if (!wd) return;
	
	if (chdir (wd) == -1)
		die ("chdir(%s) failed: %m", wd);

	info ("chdir(%s)", wd);
}

/* This is the daemon's main work - listen for connections and spawn.  */
static void run_daemon (void) 
{
	pid_t pid;
	fd_set serverfds;
	fd_set sslrds;
	int maxfd;

	loadconfig ();
	init_sockets (&serverfds, &sslrds, &maxfd);
	if (cfg.EnableSSL) 
	    ssl_init();

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if ( !curl ) {
		syslog(LOG_ERR, "curlGetURL: cant initialize curl!");
		return;
	}

	if (! test_only) {
		writepidfile ();

		if (getuid () == 0) {
			struct passwd *pwent;
			if ((pwent = getpwnam (cfg.RunAsUser)) == NULL)
				die ("Unknown user %s, check configuration", cfg.RunAsUser);

			if (setuid (pwent->pw_uid))
				die ("Cant setuid %s", cfg.RunAsUser);
		}
		errno = 0;
	}

	daemon_chdir ();

	load_servers();
	loadactive();
	loadoverviewfmt();
	load_access_conf();
	load_statsfile();
//	init_cache();

	master->serverstart = time (NULL);
	master->nrforks = 0;

#if defined(_SC_NPROCESSORS_ONLN)
	master->numcores = sysconf(_SC_NPROCESSORS_ONLN);
	info("Found %d CPU cores", master->numcores);
#else
	info("No CPU core binding support");
#endif

	if ((master->semid = semlock_init (MASTER_SEMKEY)) == -1) 
		die ("semlock_init: semget failed: %m");

	if (test_only) {
		info ("Startup Test Successfull, Exiting..");
		syslog_close ();
		exit (0);
	}

	info ("NNTP Server Starting..");

	if (!opt_stay) {
		syslog_close ();

		pid = init_daemon (serverfds);

		syslog_open ("nntpswitchd", LOG_PID, LOG_NEWS);

		if (pid < 0) die ("Can't fork");

		/* 2nd time, with the right pid, as user news */
		writepidfile ();
	}

	// start the timer process for statistics
	if ( cfg.StatsFilePeriod > 0 )
		timerpid = run_timer_loop();

	info("Server running new pid %d uid %d euid %d timerpid %d"
			, (int)getpid(), (int)getuid(), (int)geteuid(), (int)timerpid);

	setproctitle("nntpswitchd: waiting for connections");

	daemon_select_loop (serverfds, sslrds, maxfd);
}

static void chld_handler (int sig)
{
	pid_t pid;
	int i, status;

	while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
		for (i = 0; i < MAX_PROC; i++) {
			if ((master->clients+i)->pid == pid && (master->clients+i)->id != 0)
			{
				syslog(LOG_ERR, "waitpid found undeinitialized client, pid %d, id %d"
						, pid, i);
				return;
			}
		}
	}
}

static void hup_handler(int sig)
{
	syslog(LOG_NOTICE, "sighup received");
	reload++;
}

static void segv_handler(int sig)
{
	syslog(LOG_ERR, "sigsegv received");
	if (client != NULL) 
		close_child(false);
	syslog_close();
	_exit(1);
}

static void pipe_handler(int sig)
{
	static int inpipe = 0;
	inpipe++;
	if (inpipe == 1) syslog(LOG_NOTICE, "sigpipe received");
	inpipe--;
}

static void int_handler(int sig)
{
	syslog(LOG_ERR, "sigint received");
	if (client != NULL) 
		close_child(false);
	syslog_close();
	_exit(1);
}

static void term_handler(int sig)
{
	syslog(LOG_NOTICE, "sigterm received");
	if (client != NULL) {
		client_quit();
		close_child(true);
		syslog_close();
		_exit(1);
	} else {
		terminate = true;
	}
}

static void ill_handler(int sig)
{
	syslog(LOG_ERR, "sigill received");
	if (client != NULL) 
		close_child(false);
	syslog_close();
	_exit(1);
}

static void abrt_handler(int sig)
{
	syslog(LOG_ERR, "sigabrt received");
	if (client != NULL) 
		close_child(false);
	syslog_close();
	_exit(1);
}

static void init_sighandlers()
{
	struct sigaction sighup, sigsegv, sigpipe, sigint;
	struct sigaction sigterm, sigill, sigabrt, sigchld;

	memset(&sigchld, 0, sizeof (sigchld));
	sigchld.sa_handler = &chld_handler;
	sigaction(SIGCHLD, &sigchld, NULL);

	memset(&sighup, 0, sizeof (sighup));
	sighup.sa_handler = &hup_handler;
	sigaction(SIGHUP, &sighup, NULL);

	memset(&sigsegv, 0, sizeof (sigsegv));
	sigsegv.sa_handler = &segv_handler;
	sigaction(SIGSEGV, &sigsegv, NULL);

	memset(&sigpipe, 0, sizeof (sigpipe));
	if (opt_stay) {
		info("install pipe_handler");
		sigpipe.sa_handler = &pipe_handler;
	} else {
		sigpipe.sa_handler = SIG_IGN;
	}
	sigaction(SIGPIPE, &sigpipe, NULL);

	memset(&sigint, 0, sizeof (sigint));
	sigint.sa_handler = &int_handler;
	sigaction(SIGINT, &sigint, NULL);

	memset(&sigterm, 0, sizeof (sigterm));
	sigterm.sa_handler = &term_handler;
	sigaction(SIGTERM, &sigterm, NULL);

	memset(&sigill, 0, sizeof (sigill));
	sigill.sa_handler = &ill_handler;
	sigaction(SIGILL, &sigill, NULL);

	memset(&sigabrt, 0, sizeof (sigabrt));
	sigabrt.sa_handler = &abrt_handler;
	sigaction(SIGABRT, &sigabrt, NULL);
}

static void init_options (int argc, char *argv[])
{
	tgetopt (argc, argv);

	if (toptset ('h')) {
		info (usage, argv[0]);
		exit (0);
	}

	if (toptset ('c')) config_file = toptargs ('c');
	if (toptset ('s')) opt_stay++;
	if (toptset ('t')) test_only++;
}

int main (int argc, char *argv[])
{
	syslog_open("nntpswitchd", LOG_PID, LOG_NEWS);

	init_options(argc, argv);

	init_sighandlers();
	initproctitle(argc, argv);

	if ((master = memmap(sizeof (MASTER))) == NULL)
		die("Can't allocate master memory");

	info("Server starting up..");

	run_daemon();

	syslog_close();
	
	return 0;
}
