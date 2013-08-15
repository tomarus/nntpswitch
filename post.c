#include <unistd.h>
#include <sys/time.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

#include "semaphore.h"

#define BUF_SIZE 524288

#define CLEARBUF buffer[0]='\0';

// TODO die() is niet goed 


/* wait for client to finish sending data */
static void wait_finish_client(CLIENT *client, char *buffer, int bufsize)
{
	int i = 1;

	while (client->error == 0
			&& i == 1
			&& readclient(client, buffer, bufsize, 0) != NULL)
	{ 
		if (buffer[0] == '.' && (buffer[1] == '\r' || buffer[1] == '\n'))
			i=0;
	}
}


/*
 * walk through newsgroup lists and set emailaddress
 * of first moderated newsgroups. return groupname
 * if the group is unknown, NULL if group is valid
 */
static char *check_moderated(char *groups, char **mailto, int *numgroups)
{
	char *p, *g;
	ACTIVE *grp;
	char *tokbuf;
        tokbuf = NULL;
	*numgroups = 0;
	
	p=strtok_r(groups, ",", &tokbuf);
	do {
		(*numgroups)++;

		if ((grp = getgroup(p)) == NULL)
			return p;

		if (grp != NULL && grp->mode == 'm')
		{
			for (g=p; *g!='\0'; g++)
				if (*g == '.') *g = '-';

			*mailto = malloc (strlen (p) + strlen (cfg.ModeratorDomain) + 2);
			if (*mailto == NULL) die ("No room for moderatordomain");

			strcpy (*mailto, p);
			strcat (*mailto, "@");
			strcat (*mailto, cfg.ModeratorDomain);
			return NULL;
		}
	} while ( (p=strtok_r(NULL, ",", &tokbuf)) );

	return p;
}


/*
 * Send the email to moderator address.
 */
static int send_email(CLIENT *client, HLIST *phlist, char *msgid, char *postbuf, int postsize, char *address)
{
	char tmp[19] = "/tmp/nntpsw.XXXXXX";
	int fd;
	char buf[MAX_HEADER + 512];
	char *sysbuf;

	if ((sysbuf=calloc (1, MAX_STRING)) == NULL) 
		die("no room for sysbuf in send_email");

	if ((fd=mkstemp (tmp)) == -1) 
		die("Cant write spool file for moderator");

	/* spool the headers and body first */

	while (phlist != NULL)
	{
		if (phlist->hdr == NULL && phlist->val == NULL)
		{
			syslog(LOG_ERR, "Skipping empty header during post mail command?");
			phlist = phlist->next;
			continue;
		}

		sprintf(buf, "%s %s\r\n", phlist->hdr, phlist->val);
		if ((write(fd, buf, strlen (buf))) == -1)
		{
			writeclient(client, "500 Spooling for moderator failed (headers)");
			return 1;
		}
		phlist = phlist->next;
	}

	sprintf(buf, "To: %s\r\n\r\n", address);

	if ((write(fd, buf, strlen (buf))) == -1)
	{
		writeclient(client, "500 Spooling for moderator failed (separator)");
		return 1;
	}

	if (! write (fd, postbuf, postsize))
	{
		writeclient(client, "500 Spooling for moderator failed (body)");
		disconnect_server(client);
		return 1;
	}

	close (fd);

	/* finally, mail the spooled post */

	sprintf(sysbuf, "%s %s < %s", cfg.MailerPath, address, tmp);

	int r = WEXITSTATUS(system(sysbuf));
	if (r)
	{
		syslog(LOG_WARNING, "Can't send email to moderator, sendmail exit status: %d", r);
		unlink(tmp);
		free(sysbuf);
		return writeclient(client, MSG_POST_NOMAIL);
	}

	free(sysbuf);
	unlink(tmp);
	return writeclient(client, MSG_POST_MAILED);
}


/*
 * connect to the post server and send the message using ihave command
 * the message is a header list and raw data of the body.
 * return 1 on error 
 */
static int send_ihave(CLIENT *client, HLIST *phlist, char *msgid, char *postbuf, int postsize)
{
	char b[256];
	int oldsock;

	oldsock = client->serversock;

	if ( client->postsock == 0 )
	{
		int sock;
		if ( (sock=connect_server_nntp(client, cfg.PostServer, cfg.PostServerPort, cfg.ServerReadTimeout)) == -1 )
		{
			writeclient(client, MSG_SERVER_DOWN);
			return 0;
		}
		client->postsock = sock;
	}
	client->serversock = client->postsock;

	if (! writeserver(client,"IHAVE %s\r\n", msgid)) return 1;
	if ( (readserver(client, b, 255)) == NULL) return 1;

	if (atoi(b) < 400 )
		syslog(LOG_NOTICE,"%s: post server responded %s",client->hostname, b);
	else {
		syslog(LOG_ERR,"%s: post server responded %s",client->hostname, b);
		swriteclient(client, b);
		goto disconnect;
	}

	/* post the headers */

	while (phlist != NULL)
	{
		if (phlist->hdr == NULL && phlist->val == NULL)
		{
			syslog(LOG_ERR, "Skipping empty header during ihave command?");
			phlist=phlist->next;
			continue;
		}

		if (! writeserver(client, "%s %s", phlist->hdr, phlist->val))
			return 1;

		phlist = phlist->next;
	}

	/* post the body */

	syslog(LOG_DEBUG, "%s: posting body %d bytes", client->hostname, postsize);

	if (! writeserver (client,"\r\n")) return 1;
	if (! write(client->serversock, postbuf, postsize))
	{
		syslog(LOG_ERR, "cant write to server %m");
		goto disconnect;
	}

	syslog(LOG_DEBUG, "%s: post body done checking for result", client->hostname);

	if ((readserver(client, b, 255)) == NULL) return 1;
	if (atoi(b) > 299)
	{
		syslog(LOG_ERR, "%s: post server responded %s", client->hostname, b);
		writeclient(client, b);
		goto disconnect;
	}

	syslog(LOG_DEBUG, "%s: post received, telling client msg_post_received", client->hostname);

	writeclient(client, MSG_POST_RECEIVED);

	client->serverarts++;
	client->serverbytes += strlen(postbuf);

	client->serversock = oldsock;
	return 0;

disconnect:
//	syslog(LOG_ERR,"%s: closing on disconnect()", client->hostname);
	client->serversock = client->postsock;
	disconnect_server(client);
	client->serversock = 0;
	client->postsock = 0;
	client->connected = 0;
	return 0;
}


/* returns 0 if client can continue */
PROTO int post(CLIENT *client)
{
	int i=1;
	int gotnewsgroups=0, gotfrom=0, gotsubject=0, gotmsgid=0, gotdate=0;
	int gotorg=0, gotcontrol=0;
	int numgroups=0;
	int isapproved=0;
	int lines;
	char *postbuf = NULL;
	int postsize=0, s;
	char b[256];
	struct timeval tv;
	struct tm *ptm;
	char msgid[64], realmsgid[128], timestr[64];
	char hedr[64], val[MAX_HEADER];
	char newsgroups[MAX_HEADER];
	char *mailto = NULL;
	char chkbuf[5];
	char *p;
	HLIST *hdr = NULL;
	char *buffer = NULL;

	if ((buffer=malloc(BUF_SIZE)) == NULL) 
		die ("no room for post buffer"); // FIXME: die()

	writeclient(client, MSG_POST_OK);

	/* make Path the first header */
	sprintf(b, "%s!not-for-mail", client->profile->Hostname);
	hdr = insert_hlist(hdr, "Path:", b);

	/* Read headers from the client and check if we have everything 
	 * we need. Also skip system headers if we find them.
	 */
	while ( i && (readclient(client, buffer, BUF_SIZE, 0)) != NULL)
	{
		chop(buffer);

		if (buffer[0] == 0)
		{
			i=0; 
			break; 
		}

		if (buffer[0] == '.' && buffer[1] == 0)
			return swriteclient(client, MSG_POST_CANCEL);

		if (strlen(buffer) >= MAX_HEADER)
		{
			wait_finish_client(client, buffer, BUF_SIZE);
			return swriteclient(client, MSG_POST_HDRTOOLONG);
		}

		hedr[0] = 0;
		val[0] = 0;
		/* we have a multi-line header */
		if ((sscanf(buffer,"%63[^\t ] %1023[^\r\n]", hedr, val)) == 0)
			strncpy(val, buffer, MAX_HEADER-1);

		/* skip empty headers */
		if (val[0] == '\0')
			continue;

		/* headers to skip from client */
		if (!strncasecmp(hedr, "NNTP-Posting-Host:",	18)) continue;
		if (!strncasecmp(hedr, "NNTP-Posting-Date:",	18)) continue;
		if (!strncasecmp(hedr, "Complaints-To:",	14)) continue;
		if (!strncasecmp(hedr, "X-Complaints-To:",	16)) continue;
		if (!strncasecmp(hedr, "X-Trace:",		8)) continue;
		if (!strncasecmp(hedr, "Path:",			5)) continue;
		if (!strncasecmp(hedr, "To:",		 	3)) continue;
		if (!strncasecmp(hedr, "Cc:",		 	3)) continue;
		if (!strncasecmp(hedr, "Bcc:",			4)) continue;
		if (!strncasecmp(hedr, "X-NNTP-Posting",	14)) continue;
		if (!strncasecmp(hedr, "X-Abuse",		7)) continue;
		if (!strncasecmp(hedr, "Lines",			5)) continue;
		if (!strncasecmp(hedr, "Injection-Info",	14)) continue;
		if (!strncasecmp(hedr, "Injection-Date",	14)) continue;

		/* if no permission to post approved posts, skip header */
		if (!strncasecmp(hedr, "Approved:", 9))
		{
			if (client->acl->apost == 1)
				isapproved++;
			else
				continue;
		}

		/* if forced organization overwrite, just skip it here */
		if (client->profile->ForceOrganization && !strncasecmp(hedr, "Organization:", 13)) 
			continue;

		/* check and store the newsgroups line */
		if (!strncasecmp(hedr, "Newsgroups:", 11))
		{
			gotnewsgroups++;
			strncpy(newsgroups, val, MAX_HEADER-1);
		}

		/* check some required headers */
		if (!strncasecmp(hedr, "From:",		5)) gotfrom++;
		if (!strncasecmp(hedr, "Subject:",	8)) gotsubject++;
		if (!strncasecmp(hedr, "Date:",		5)) gotdate++;
		if (!strncasecmp(hedr, "Organization:", 13)) gotorg++;
		if (!strncasecmp(hedr, "Control:",	8)) gotcontrol++;
		if (!strncasecmp(hedr, "Message-ID:",	11))
		{
			gotmsgid++;
			strcpy(realmsgid, val);
		}

		insert_hlist(hdr, hedr, striprn (val));
	}

	/* Continue reading the body from the client */

	i=1;
	checkbuf_init(chkbuf);

	while (i == 1
		&& client->error == 0 && client->timeout == 0
		&& (s=client_read_socket(client, buffer, BUF_SIZE, client->profile->ClientReadTimeout)) > 0)
	{
		if (checkbuf_isend(chkbuf, buffer, s))
			i=0;

		postbuf = (char*)realloc(postbuf, postsize+s);
		if (postbuf == NULL)
			die("no room for postbuffer + s"); // FIXME this leaks
		memcpy(postbuf+postsize, buffer, s);
		postsize += s;
	}

	/* Now we should have all data from the client. */

	if (postbuf == NULL || i == 1)
	{
		syslog(LOG_ERR, "%s: Post Aborted or No body received", client->hostname);
		i = 1;
		goto finalize;
	}

	/* check required headers */
	if (gotnewsgroups == 0 || gotfrom == 0 || gotsubject == 0)
	{
		i = swriteclient(client, MSG_POST_WRONGHEADER);
		goto finalize;
	}

	p = hlist_get_value(hdr, "From:");
	if ( ! (p != NULL && check_email(p)) )
	{
		i = swriteclient(client, MSG_POST_WRONGEMAIL);
		goto finalize;
	}
    
	/* check group permission */
	if (! match_expression((const unsigned char *)newsgroups, (const unsigned char *)getwildmat(client->user->postpat), 0))
	{
		i = swriteclient(client, MSG_NOPOSTING);
		goto finalize;
	}

	/* check permission to send control messages */
	if (client->profile->NoControl == 1 && gotcontrol == 1)
	{
		i = swriteclient(client, MSG_NOPOSTING);
		goto finalize;
	}

	/* create and insert message-id */
	gettimeofday(&tv, NULL);
	sprintf(msgid,"%.5lx%lx%lx%.5d", tv.tv_usec, tv.tv_sec, client->ip4addr^0xA5A5A5A5L, (int)getpid());

	if (! gotmsgid)
	{
		sprintf(realmsgid,"<%lx$%lx$%lx$%d@%s>",
				tv.tv_usec, tv.tv_sec, client->ip4addr,
				(int)getpid (), client->profile->Hostname);
		insert_hlist(hdr, "Message-ID:", realmsgid);
	}
	else 
	{
		p = hlist_get_value(hdr, "Message-Id:");
		if ( ! (p != NULL && check_message_id(p)) )
		{
			i = swriteclient(client, MSG_POST_WRONGMSGID);
			goto finalize;
		}
	}

	/* fixup system headers */

	insert_hlist(hdr, "X-Complaints-To:", client->profile->XComplaintsTo);
	if (gotorg == 0 && nullstr(client->profile->Organization) == 0)
	{
		sprintf(b,"%s\r\n", client->profile->Organization);
		insert_hlist(hdr, "Organization:", client->profile->Organization);
	}

	ptm = localtime(&tv.tv_sec);
	strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S %z",ptm);

	if (!gotdate)
		insert_hlist(hdr, "Date:", timestr);
	else
	{
		p = hlist_get_value(hdr, "Date:");
		if ( p == NULL )
			syslog(LOG_ERR, "Cant get Date header for date check?");

		time_t now, d=parsedate(p);
		time(&now);

		if ( d == 0 || d == -1 )
		{
			d = time(NULL);
			hlist_replace_value(hdr, "Date:", timestr);
		}

		if ( (now-d) > MAX_POST_AGE )
		{
			i = swriteclient(client, MSG_POST_TOOOLD);
			goto finalize;
		}

		if ( (now-d) < -MAX_POST_AGE )
		{
			i = swriteclient(client, MSG_POST_TOONEW);
			goto finalize;
		}
	}

	/* insert footer if applicable */
	if (client->profile->FooterFile[0] != 0)
	{
		postbuf = (char*)realloc(postbuf , postsize + client->profile->FooterSize + 1);
		if (postbuf == NULL)
			die("no room for postbuffer + FooterSize");
		/* -3 for .\r\n (FIXME: shouldnt we allocate this?) */
		memcpy(postbuf + postsize - 3, client->profile->Footer, client->profile->FooterSize);
		postsize += client->profile->FooterSize;
		memcpy(postbuf + postsize - 3, ".\r\n", 3);
	}

	/* calculate nr of lines */
	lines = 0;
	for ( i=0; i<postsize-3; i++)
	{
		if (postbuf[i] == '\r' && postbuf[i+1] == '\n')
			lines++;
	}
	sprintf(hedr, "%d", lines);
	insert_hlist(hdr, "Lines:", hedr);

        if ( client->useSSL && client->profile->XHeaderSSLUpload[0] != '\0' ) {
            insert_hlist(hdr, "X-Upload:", client->profile->XHeaderSSLUpload);
        }

	/* log and insert trace headers */

	if (client->profile->AddNNTPPostingHost)
	{
		sprintf(b, "%s (%s)", client->hostname, inet_ntoa(client->addr.sin_addr));
		insert_hlist(hdr, "NNTP-Posting-Host:", b);
	}

	insert_hlist(hdr, "NNTP-Posting-Date:", timestr);

	if ( cfg.PostXTraceHeader )
		insert_hlist(hdr, "X-Trace:", msgid);

	if (client->authenticated == 1)
		syslog(LOG_NOTICE, "X-Trace: id=\"%s\" msgid=\"%s\" host=\"%s\" profile=\"%s\" user=\"%s\"",
				    msgid, realmsgid, client->hostname, client->profile->Name, client->username );
	else
		syslog(LOG_NOTICE, "X-Trace: id=\"%s\" msgid=\"%s\" host=\"%s\" profile=\"%s\"",
				    msgid, realmsgid, client->hostname, client->profile->Name );
	
	if ((p=check_moderated(newsgroups, &mailto, &numgroups)) != NULL)
	{
		syslog(LOG_NOTICE,"%s: Post to nonexistent group %s", client->hostname, p);
		i = writeclient(client, MSG_NOSUCHGROUP);
		goto finalize;
	}

        if ( numgroups > client->profile->MaxCross ) {
            i = writeclient(client, MSG_POST_TOOMANYGROUPS);
            goto finalize;
        }

	if (isapproved == 0 && mailto != NULL)
		i = send_email(client, hdr, realmsgid, postbuf, postsize, mailto);
	else
		i = send_ihave(client, hdr, realmsgid, postbuf, postsize);

	client->posts++;
	client->postbytes += postsize;
	client->profile->postarticles++;
	client->profile->postbytes += postsize;

finalize:
	if (buffer) free(buffer);
	if (postbuf) free(postbuf);
	if (hdr) free_hlist(hdr);

	return i;
}


/*
 * passthrough ihave command. used in slave mode
 */
PROTO int ihave(CLIENT *client, char *msgid) 
{
	char b[256];
	int i;
	char chkbuf[5];
	int postsize=0, s;
	ACTIVE *oldgrp;
	char *buffer;

	if ((buffer=malloc (BUF_SIZE)) == NULL) 
		die ("no room for post buffer");

	oldgrp = client->group;
	if (client->connected == 1) disconnect_server (client);

	if (connect_server_nntp (client, cfg.PostServer, cfg.PostServerPort, cfg.ServerReadTimeout)) 
	{
		writeclient (client, MSG_SERVER_DOWN);
		return 0;
	}

	if (! writeserver (client,"IHAVE %s\r\n", msgid)) return 1;
	if ( (readserver (client, b, 255)) == NULL) return 1;

	if (atoi (b) > 399)
	{
		syslog (LOG_ERR, "%s: post server responded %s", client->hostname, b);
		writeclient (client,b);
		disconnect_server (client);
		return 0;
	} else {
		syslog (LOG_NOTICE, "%s: post server responded %s", client->hostname, b);
	}
	writeclient (client, b);

	/* post the headers + body */

	i=1;
	checkbuf_init (chkbuf);

	while (i == 1 && 
		client->error == 0 && 
		client->timeout == 0 && 
		(s=client_read_socket(client, buffer, BUF_SIZE , client->profile->ClientReadTimeout)) > 0 ) 
	{
		if (checkbuf_isend (chkbuf, buffer, s))
			i=0;

		write_socket (client->serversock, buffer, s, cfg.ServerReadTimeout);
		postsize += s;
	}

	/* result */

	if ((readserver (client, b, 255)) == NULL) return 1;
	if (atoi (b) > 299)
	{
		syslog (LOG_ERR,"%s: post server responded %s",client->hostname, b);
		writeclient (client,b);
		disconnect_server (client);
		return 0;
	}

	writeclient (client, b);

	client->serverarts++;
	client->serverbytes += postsize;
	disconnect_server (client);

	/* so now we can restore the old group with this lame workaround */
	/* actually, if we just could have a 2nd server connection this  */
	/* would have been unneeded. so, FIXME */
	if (oldgrp != NULL) 
		group (oldgrp->newsgroup, true);

	return 0;
}
