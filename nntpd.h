#ifndef NNTPD_H
# define NNTPD_H
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>
#include <pwd.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <curl/curl.h>
#include <openssl/ssl.h>

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef bool
#define bool unsigned char
#endif

#ifndef ulong64
typedef unsigned long long ulong64;
#endif

#ifndef long64
typedef long long long64;
#endif

#define MAX_HOSTNAME 256	/* max hostname length */
#define MAX_USERPASS 64		/* max length of username or password */
#define MAX_ACLS 2048
#define MAX_ARGS 64
#define MAX_ALIASES 256
#define MAX_AUTHS 2048
#define MAX_PROFILES 512
#define MAX_SERVERS 128		/* let me know if you have more! */
#define MAX_VHOSTS 128
#define MAX_GROUP 160		/* max groupname length */
#define MAX_SERVER 32		/* max servername length (hostname and short id name in servers.conf) */
#define MAX_SERVERRSP 512	/* max length of server reply */
#define MAX_GROUPS 128000
#define MAX_PROC 8192		/* max of the max, config via nntpswitch.conf */
#define MAX_USERS MAX_PROC
#define MAX_HEADER 1024		/* max size of single header */
#define MAX_FOOTER 1024		/* max length of appended footer */
#define MAX_WILDMATS 32		/* max N wildmat pattern (access.conf) */
#define MAX_OVERVIEWFMT 16	/* max lines */
#define MAX_LEVEL 1000		/* max Level in server config */
#define MAX_CFGLINE 16384	/* max line length in config files */
#define MAX_DAYLIMITS 16	/* max size of profile Limits array */
#define MAX_POST_AGE 86400  	/* oldest (or newest) post we accept, max 1d old or 1d new */
#define MAX_PATH 128		/* max path+filename length */
#define MAX_NFS 64              /* max nr of nntpswitch servers running */
#define MAX_CLIENTHOSTS 50  /* Allow maximum 50 host for unique users counting. */

/* The following might be hardcoded in sscanf's */
#define MAX_STRING 256		/* generally max string length */
#define MAX_NAME 32		/* general max length of a name for something */

#define MSG_OVERVIEWFMT		"215 Overview.fmt follows\r\n"
#define MSG_POST_RECEIVED	"240 Thanks!\r\n"
#define MSG_POST_MAILED		"240 Thanks, post mailed to moderator\r\n"
#define MSG_POST_OK		"340 Try Me!\r\n"
#define MSG_POST_WRONGHEADER	"441 Newsgroups, From or Subject missing\r\n"
#define MSG_POST_NOMAIL		"441 Couldn't mail your post to the moderator, please try again\r\n"
#define MSG_POST_CANCEL		"441 Post Cancelled\r\n"
#define MSG_POST_HDRTOOLONG	"441 Header too long\r\n"
#define MSG_POST_TOOOLD		"441 Post is too old, check your system clock\r\n"
#define MSG_POST_TOONEW		"441 Post is too new, check your system clock\r\n"
#define MSG_POST_WRONGEMAIL	"441 From address not in internet syntax\r\n"
#define MSG_POST_WRONGMSGID	"441 Incorrect Message-ID syntax\r\n"
#define MSG_POST_TOOMANYGROUPS  "441 Too many newsgroups\r\n"
#define MSG_AUTH_OK		"281 Authentication Accepted\r\n"
#define MSG_MOREAUTHREQ		"381 More Authentication Required\r\n"
#define MSG_SESSIONLIMIT	"400 Session Time Limit Reached (please reconnect)\r\n"
#define MSG_BYTELIMIT		"400 Session Byte Limit Reached (please reconnect)\r\n"
#define MSG_AUTHREQUIRED	"480 Authentication Required\r\n"
#define MSG_AUTH_REJ		"482 Authentication Rejected\r\n"
#define MSG_AUTH_ERR		"482 Authentication Temporarily Unavailable\r\n"
#define MSG_AUTH_TOOMANYCON	"482 Too many connections in your class\r\n"
#define MSG_AUTH_TOOMANYUSR	"482 Too many users in your class\r\n"
#define MSG_AUTH_USERFULL	"482 Too many connections for your user (nntpswitch)\r\n"
#define MSG_AUTH_USERFULLREMOTE	"482 Too many connections for your user (remote)\r\n"
#define MSG_AUTH_HOSTFULL	"482 You are already connected from a different host\r\n"
#define MSG_AUTH_HOSTMAX	"482 Already connected from maximum number of different hosts (%d)\r\n"
#define MSG_AUTH_OVERQUOTA	"482 You have reached your download limit.\r\n"
#define MSG_AUTH_OVERTIME	"482 This account can only be used between %s and %s.\r\n"
#define MSG_AUTH_TWICE		"281 You are already authenticated (please disconnect first)\r\n"
#define MSG_NOSUCHGROUP		"411 No such group\r\n"
#define MSG_NOGROUP		"412 No group selected\r\n"
#define MSG_ARTICLECANT		"430 Cant retrieve article, try joining a newsgroup first\r\n"
#define MSG_ARTICLENOPERM	"430 Cant retrieve article, permission denied\r\n"
#define MSG_NOPOSTING		"440 Posting Denied\r\n"
#define MSG_SERVER_DOWN		"403 Remote Server Unavailable\r\n"
#define MSG_COMMAND_HTTP	"500 Disconnected due to HTTP command\r\n"
#define MSG_NONEWNEWS		"501 NEWNEWS only supports single newsgroup names\r\n"
#define MSG_NOPATINDEX		"501 Sorry, your search for this header type has been disabled\r\n"
#define MSG_SERVERFULL		"400 Sorry, server is full at the moment, try later\r\n"
#define MSG_TOOMANYUSERS	"400 Too many users\r\n"
#define MSG_TOOMANYUSERSPRF	"400 Too many users in your class\r\n"
#define MSG_TOOMANYCONNSPRF	"400 Too many connections in your class\r\n"
#define MSG_TOOMANY_CONCURRENT	"400 Too many concurrent connections, you are allowed %d\r\n"
#define MSG_GENERIC_ERROR	"502 %s\r\n"
#define MSG_UNSUPPORTED		"500 Unsupported\r\n"
#define MSG_NOTIMPLEMENTED	"500 Not Implemented\r\n"
#define MSG_NOACL		"500 Can't get ACL\r\n"
#define MSG_NOPERM		"500 Permission Denied\r\n"
#define MSG_AUTH_UNKNOWN	"501 Unsupported AUTHINFO command\r\n"
#define MSG_AUTH_NOMEM		"501 Out of memory for authentication\r\n"

#define MSG_CANNOTSSL           "500 Cannot setup SSL connection\r\n"
#define MSG_SSLDISABLED         "502 SSL connection is disabled for this user\r\n"

#define UMSG_SERVERTIMEOUT	"Remote Server Timeout"

#define DEFAULT_AUTH		"auth_default"
#define DEFAULT_ACCT		"acct_default"
#define DEFAULT_PROFILE		"default"
#define DEFAULT_WILDMAT		"*"

#define RL_INITVAL 100000	/* this much usec as start for rate-limits */

/* If this is defined, send accounting record after this many bytes
 * instead of waiting for end of connection
 */
#define INTERMEDIATE_ACCT 500000000

#define MAX_PFXMAPS 4
#define PFX_MAPSIZE 0x3FFFFF

enum article_result 
{
	CHECK_ARTICLE_OK,
	CHECK_ARTICLE_FAIL,
	CHECK_ARTICLE_NOPERM
};


enum connect_result 
{
	CONNECT_OK,
	CONNECT_ERR,
	CONNECT_DOWN
};


typedef struct
{
	char		name[32];
	char		map[PFX_MAPSIZE];
} PFXMAP;


struct _hlist
{
        char *hdr;
        char *val;
        struct _hlist *next;
};
#define HLIST struct _hlist


typedef struct 
{
	char		name[MAX_STRING];
	char		lib[MAX_STRING];
	char		args[MAX_STRING];
} ALIAS;


typedef struct 
{
	char		name[MAX_STRING];
	char		args[MAX_STRING];
} ARGS;


typedef struct
{
	int	gb;
	int	kbit;
} LIMITS;


struct _profile
{
	char		Name[MAX_NAME];
	char		ReadPat[MAX_STRING];
	char		PostPat[MAX_STRING];
	int		MaxConnections;
	int		MaxUserConcurrent;
	int		MaxHostConcurrent;
	int		Slave;
	int		ASMaxConns;
	int		ASMaxHosts;
	int		MaxUsers;
	int		MaxSessionTime;
	ulong64		MaxSessionBytes;
	uint		MaxUserBPS;
	uint		MaxProfileBPS;
	char		XComplaintsTo[MAX_STRING];
	char		Organization[MAX_STRING];
	int		ForceOrganization;
	int		AddNNTPPostingHost;
        int             MaxCross;
	char		Hostname[MAX_STRING];
	char		FooterFile[MAX_STRING];
	char		Footer[MAX_FOOTER];
	int		FooterSize;
	int		NoControl;
	int		ArticleSearch;
	int		ClientReadTimeout;
	ALIAS*		Accounting;
	char		AccountingPrefix[MAX_STRING];
	LIMITS		Limits[MAX_DAYLIMITS];
	int		NumLimits;
	char		Disabled[MAX_STRING];
	int             AllowSSL;

	uint		connections;		/* current connection counter */
	uint		numusers;		/* current user counter */
	uint		realusers;		/* active users counter (i.e. after auth) */
	ulong64		bytes;			/* for rrd */
	ulong64		rbytes;			/* reset after output (json stats) */
	ulong		articles;
	ulong64		postbytes;
	ulong		postarticles;

	/* rate limiter */
	uint		rl_curbytes;
	uint		rl_lastbytes;
	double		rl_sleep;
	struct timeval	rl_start;
	double		rl_curbps;

	double		UserRLFactorU;
	double		UserRLFactorD;
	double		ProfileRLFactorU;
	double		ProfileRLFactorD;

	uint		Retention;
        char            XHeaderSSLUpload[MAX_HEADER];
	PFXMAP *	PrefixMap;
	struct _profile	* FailProfile;
};
#define PROFILE struct _profile


typedef struct
{
	char		pattern[1024];
	char		name[MAX_NAME];
} WILDMAT;


typedef struct
{
	char		mask[128];
	ALIAS*		auth;
	PROFILE*	profile;
} AUTH;


typedef struct
{
	/* username and password are set by the main nntpswitch authenticator */
	char *		username;
	char *		password;		/* cleartext */
	char *		username_s;		/* username with matched pattern stripped */

	int		port;			/* port (slot) id */
	char *		hostname;		/* client hostname */
	struct in_addr	in_addr;		/* client ip adres */

	/* Those are copied from the MASTER struct */
	ARGS *		args;
	int		numargs;

	/* 
	 * the following fields should be set by the authentication module 
	 * if left blank (untouched by the module), the default from the 
	 * currently selected ACL is used. The struct including strings are
	 * free()'d after authentication.
	 */
	bool		authenticated;		/* succeeded or not */
	char *		message;		/* txt message like '482 no access' or '281 welcome' */
	char *		profile;		/* profile name */
	bool		posting;		/* posting allowed */
	ulong64		bytes;			/* bytes remaining */
	int		userkbit;		/* max user kbit */
	char *		logname;		/* username for accounting, eg stripped username */
	char *		readpat;
	char *		postpat;
	int		timeleft;
	bool		overquota;		/* Not used. */
	bool		disabled;		/* Not used. */
	char *		posthost;
	char *		postabuse;
	char *		postorg;
} AUTHRESULT;


typedef struct
{
	ulong		start;
	ulong		end;
	ulong		size;

	PROFILE *	profile;

	uint		read:1;
	uint		post:1;
	uint		apost:1;
	uint		deny:1;
	uint		auth:1;
	uint		stats:1;
	uint		unlimit:1;
} ACL;


typedef struct
{
	ulong		hi;
	ulong		lo;
	ulong		times;
	ulong		id;
	char		mode;
	char		newsgroup[MAX_GROUP];
	char		server[32];
} ACTIVE;


enum _cmdnum
{
	cmd_mode,  cmd_slave, cmd_authinfo,
	cmd_help,  cmd_date,  cmd_quit,
	cmd_list,  cmd_group, cmd_listgroup, cmd_newgroups, cmd_xgtitle,
	cmd_over,  cmd_xover, cmd_xzver,     cmd_xhdr, cmd_xzhdr, cmd_xpat, cmd_newnews, cmd_post,
	cmd_stat,  cmd_head,  cmd_body,  cmd_article,  cmd_next,  cmd_last,
	cmd_ihave, cmd_http,  cmd_statistics, cmd_unsupported
};


typedef struct
{
	char*		command;
	enum _cmdnum 	num;
	uint		auth:1;			/* auth needed for command */
	uint		acct:1;			/* account this command */
	uint		limit:1;		/* rate-limit this command */
	uint		usecount;
} CMDINFO;


enum servertype
{
	type_spool, type_xover, type_post
};


enum serverpol
{
	policy_single, policy_backup, policy_balance
};


typedef struct
{
	char		Name[MAX_NAME];
	char		Hostname[MAX_HOSTNAME];
	char		Groups[MAX_CFGLINE];
	char		Username[MAX_USERPASS];
	char		Password[MAX_USERPASS];
	uint		Port;
	uint		Timeout;
	uint		Level;
	enum servertype	ServerType;
	enum serverpol	Policy;
	bool		ActiveTimes;
	bool		Descriptions;
	bool		SplitList;
	uint		MaxConnections;

	uint		connections;
} SERVER;


typedef struct
{
	char            ipaddress[MAX_HOSTNAME];
        char            Banner[MAX_STRING];
        char            AddSuffix[MAX_USERPASS];
        int             ReplaceBanner;
        char            SSLKeyFile[MAX_PATH];
        char            SSLCAList[MAX_PATH];
        char            SSLPassword[MAX_USERPASS];
        SSL_CTX *       ssl_ctx;
} VHOST;


typedef struct
{
	char		key[128];
	uint		id;			/* this is the position in memory */

	uint		MaxUserBPS;		/* copied from profile or authresult */
	long64		bytesleft;		/* initialized on connect or authenticate */
	uint		connections;		/* number of client structs refering to us */
	uint		realuser;		/* set if client is connected, ready for commands etc */
	char		readpat[MAX_STRING];
	char		postpat[MAX_STRING];
        int             timeleft;

	/* rate limiter */
	uint		rl_curbytes;
	uint		rl_lastbytes;
	double		rl_sleep;
	struct timeval	rl_start;
	double		rl_curbps;
} USER;


typedef struct
{
	int		socket;
	int             useSSL;
	SSL*            ssl;		
	time_t		connectsince;
	pid_t		pid;
	int		numcore;		/* proceccor core we're bound to */

	int		serversock;		/* primary server socket */
	int		postsock;		/* post server socket */
	SERVER *	groupserver;
	ACTIVE *	group;
	SERVER *	currserver;		/* replacement for lastserver */

	char		hostname[MAX_HOSTNAME];
	ulong		ip4addr;
	struct sockaddr_in addr;

	ACL *		acl;
	AUTH *		auth;
	CMDINFO *	command;
	PROFILE *	profile;
	USER *		user;
        VHOST *         vhost;

	char		username[MAX_USERPASS];
	char		logname[MAX_USERPASS];	/* for postfix stripped usernames */
	char		password[MAX_USERPASS];
	uint		id;
	uint		concur;			/* this is the clients N'th session */
	char *		bbuf;			/* client buffer */

	int		error;			/* set if we got error and have to exit */
	int		timeout;		/* set if timeout was caught */
	char *		errstr;

	/* rate limiter */
	int		RateIntervalUs;		/* 1/cfg.Period * 100000 */

	/* statistics */
	ulong64		bytes;
	ulong64		rbytes;			/* reset after output (json stats) */
	uint		groups;
	uint		articles;
	uint		posts;
	ulong64		postbytes;
	uint		grouparts;
	ulong64		groupbytes;
	uint		serverarts;
	ulong64		serverbytes;
	uint		starttime;
	long64		initbytes;		/* copied to user after auth */

	uint		connected:1;
	uint		authenticated:1;
	uint		inuse:1;
	uint		posting:1;		/* copied form acl and/or authres */
} CLIENT;


typedef struct
{
	int		numgroups;		/* total groups in active file */
	int		numaliases;
	int		numargs;
	int		numauths;
	int		numprofiles;
	int		numacls;		/* number of access.conf entries */
	int		numwildmats;		/* number of wildmat entries */
	int		numservers;
	int		numvhosts;
	time_t		laststatactive;		/* remember filedate using stat() */
	time_t		laststatserver;
	uint		serverstart;		/* uptime */
	ulong64		nrforks;
	ulong64		nrlocks;
	ulong64		nrunlocks;
	int		numcores;		/* Number of cores in system */
	int		currcore;		/* Current core counter */

	int		connections;		/* total connections, also key for *clients */
	CLIENT		clients[MAX_PROC];

	int		numusers;		/* total users, also key for *users */
//	ulong		userid_id;		/* just incremental counter */
	USER		users[MAX_USERS];

	SERVER *	lservers[MAX_SERVERS];	/* servers sorted by level */
	SERVER		servers[MAX_SERVERS];

	AUTH		auths[MAX_AUTHS];
	PROFILE		profiles[MAX_PROFILES];
	ALIAS		aliases[MAX_ALIASES];
	WILDMAT		wildmats[MAX_WILDMATS];
	ACL		acls[MAX_ACLS];
	ARGS		args[MAX_ARGS];
	VHOST		vhosts[MAX_VHOSTS];

	/* The BalanceID is a counter from 0 to N where N is the number of servers 
	 * with the same level. This is used for the load balancing policy. */
	int		balance_pos[MAX_LEVEL];

	/* ipc semaphore */
	int		semid;

	uint		dllock;			/* used for daylimits accounting lock */

	uint		num_pfxmaps;
	PFXMAP		prefixmap[MAX_PFXMAPS] __attribute__ ((aligned(4)));
        int             nflist[MAX_NFS];
} MASTER;


extern CLIENT *client;
extern MASTER *master;
extern char* config_file;

time_t parsedate(char *);

#endif
/* vim: set ts=8 noexpandtab */
