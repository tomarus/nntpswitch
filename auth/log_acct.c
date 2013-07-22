/*
 * $Id: log_acct.c,v 1.5 2009-06-02 12:07:51 tommy Exp $
 * (c) 2006 News-Service.com
 * NNTPSwitch provide uniform logging of client usage (with md5)
 */

#include <arpa/inet.h>
#include <openssl/md5.h> // -lcrypto
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "crypto.h"
#include "log.h"
#include "log_acct.h"
#include "nntpd.h"

/* Compute an md5 hash over the input with some randomness added. */
static const char *hash (const char *str, unsigned long len
		, char digest[MD5_DIGEST_LENGTH * 2 + 1])
{
	// MD5Data ((unsigned char *)text, len, digest); // libmd
	// libcrypto : From openssl; Has better availibility than libmd.
	unsigned char md5_bytes[MD5_DIGEST_LENGTH];
	struct timeval now;
	MD5_CTX md5_ctx;

	MD5_Init(&md5_ctx);
	gettimeofday(&now, 0);

	pid_t pid = getpid();

	MD5_Update(&md5_ctx, &now.tv_sec, sizeof (now.tv_sec));
	MD5_Update(&md5_ctx, &now.tv_usec, sizeof (now.tv_usec));
	MD5_Update(&md5_ctx, &pid, sizeof (pid));
	MD5_Update(&md5_ctx, (const unsigned char *)str, len);

	MD5_Final(md5_bytes, &md5_ctx);

	return md5_bytes2digest(digest, md5_bytes);
}

void syslog_client_usage_md5 (int option, int facility, int level
		, const CLIENT *client, const char *prefix)
{
	struct syslog_status sst = syslog_status_get();
	char addr[INET_ADDRSTRLEN];

	if (! inet_ntop(AF_INET, &client->addr.sin_addr, addr, INET_ADDRSTRLEN))
		sprintf (addr, "0.0.0.0");

	if ( client->bytes == 0 && client->postbytes == 0 && 
	     client->articles == 0 && client->posts == 0 )
		return;

	/* "%s %s %s %s %llu %u %u %u %llu" max length is ...
	// for client->profile->name <= MAX_NAME
	// and client->hostname <= MAX_HOSTNAME
	// and client->username <= MAX_USERPASS
	// and %llu <= 20 and %u <= 9
	*/
	char logbuf[MAX_NAME + MAX_HOSTNAME + INET_ADDRSTRLEN + MAX_USERPASS
				+21+9+9+9+20+2+1];

	int len = snprintf(logbuf, sizeof (logbuf)
			, "r=%s i=%s h=%s u='%s' a=%u b=%llu p=%u y=%llu"
			, client->profile->Name
			, addr
			, client->hostname
			, client->logname ? client->logname : client->username
			, client->articles
			, client->bytes
			, client->posts
			, client->postbytes);

	/* TODO prefix refactoring */
	syslog_open("nntpswitchd", option, facility);

	char digest[MD5_DIGEST_LENGTH * 2 + 1];
	syslog(level, "%s m=%s %s", prefix, hash(logbuf, len, digest), logbuf);

	syslog_status_set(sst);
}

