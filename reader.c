/*
 * Miscelaneous commands for reading
 * date, quit, help, listgroup etc etc
 *
 * $Id: reader.c,v 1.109 2011-06-02 17:50:34 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

#include "semaphore.h"

#define CLEARBUF client->bbuf[0] = 0;
#define CLEARPASS memset(&client->password, 0, MAX_USERPASS);


/*
 * Quit and disconnect from a server
 */

PROTO void client_quit(void)
{
        if ( client->connected == 1 )
        {
                closegroup();
                disconnect_server(client);
        }
}


/*
 * Print the current date to the client
 */

PROTO int send_date(void)
{
        struct timeval tv;
        struct tm *ptm;
        char timestr[40];

        gettimeofday(&tv,NULL);
        ptm = gmtime(&tv.tv_sec);
        strftime(timestr, sizeof(timestr), "%Y%m%d%H%M%S",ptm);

        return writeclient(client,"111 %s\r\n",timestr);
}


/*
 * passthru a command from a server to the client
 */

PROTO int passcmd(char *command, char *args, CMDINFO *cmd)
{
        int     i = 1;
        char    chkbuf[5];
        ssize_t bread = 0;
        int     ret;
        CLEARBUF

        if ( ! writeserver(client, "%s %s\r\n", command, args) ) goto sdown;
        if ( (readserverline(client->bbuf, cfg.BufSize)) == NULL ) goto sdown;

        /* logic to retry command if article not found */
        int j=0;
        while ( ( cmd->num == cmd_body || cmd->num == cmd_article ) 
                && ( atoi(client->bbuf) == 423 || atoi(client->bbuf) == 430 )
                && j++ < cfg.NoSuchArticleRetries)
        {
                if ( ! writeserver(client, "%s %s\r\n", command, args) ) goto sdown;
                if ( (readserverline(client->bbuf, cfg.BufSize)) == NULL ) goto sdown;
                if ( atoi(client->bbuf) < 400 )
                {
                        if ( client->group == NULL )
                                syslog(LOG_ERR, "No such article, found in %d retries on server %s msgid %s",
                                                j,
                                                client->groupserver->Name,
                                                args);
                        else
                                syslog(LOG_ERR, "No such article, found in %d retries on server %s group %s msgid %s",
                                                j,
                                                client->groupserver->Name,
                                                client->group->newsgroup,
                                                args);
                }
        }

        if ( swriteclient(client, client->bbuf) ) goto sdown;

        ret = atoi(client->bbuf);
        if ( ret == 0 ) {
                syslog(LOG_ERR, "ret==0 cmd=%s args=%s server=%s", command, args, client->currserver->Name);
        }
        if ( ret > 399 ) i=0;

        if ( ret == 430 && cfg.LogMissing )
                syslog(LOG_ERR, "missing_article: %s %s", client->hostname, args);
        if ( ret == 423 && cfg.LogMissing )
                syslog(LOG_ERR, "missing_article: %s %s:%s", client->hostname, client->group->newsgroup, args);

        checkbuf_init(chkbuf);
        while (i && ! client->error)
        {
                if ( (bread = nreadserver(client, client->bbuf, cfg.BufSize)) == 0 )
                        goto sdown;

                if ( checkbuf_isend(chkbuf, client->bbuf, bread) )
                        i = 0;

                if ( slwriteclient(client, client->bbuf, bread, cmd->limit) )
                        return -1;

                if ( cmd->acct == 1 ) {
                        client->bytes += (ulong64)bread;
                        client->rbytes += (ulong64)bread;
                }

                if ( cmd->acct == 1 && client->user->bytesleft > 0 )
                        client->user->bytesleft -= (ulong64)bread;

                client->groupbytes  += (ulong64)bread;
        }

        return 0;

sdown:
        syslog(LOG_ERR, "Server down during passcmd, sending MSG_SERVER_DOWN");
        swriteclient(client, MSG_SERVER_DOWN);
        disconnect_server(client);
        return 0;
}


/*
 * close group, log stats and reset counters
 */

PROTO void closegroup(void)
{
        if ( client->group != NULL && (client->grouparts > 0 || client->groupbytes > 0) )
        {
                syslog(LOG_NOTICE, "%s: group %s closed articles %u bytes %llu",
                                        client->hostname,
                                        client->group->newsgroup,
                                        client->grouparts,
                                        client->groupbytes);
                client->grouparts = 0;
                client->groupbytes = 0;
        }
}


/*
 * select and/or connect to a newsgroup
 */

PROTO int group(const char *group, char nogroupecho)
{
        char b[MAX_SERVERRSP];
        ACTIVE *newgroup;
        SERVER *svr;
        CLEARBUF

        if ( nullstr(group) )
        {
                swriteclient(client,"500 You must select a group\r\n");
                return 1;
        }
  
        /* only change group if it has changed */
        if ( client->group == NULL || strcmp(client->group->newsgroup, group) != 0 ) 
        {
                /* check for read permission */
                if ( ! match_expression((unsigned char *)group
                                        , (unsigned char *)getwildmat(client->user->readpat)
                                        , 0) ) 
                {
                        if ( ! nogroupecho )
                                swriteclient(client, MSG_NOSUCHGROUP);
                        return 1;
                }

                /* find and load our new group */
                if ( (newgroup = getgroup(group)) == NULL ) 
                {
                        if ( ! nogroupecho )
                                swriteclient(client, MSG_NOSUCHGROUP);
                        return 1;
                }

                if ( (svr=getserver(newgroup->server)) == NULL )
                        return swriteclient(client, "500 Cant find server for group\r\n");

                /* disconnect first if we have to change server */
                if ( client->groupserver != NULL &&
                        client->connected == 1 &&
                        strcmp(svr->Groups, client->groupserver->Groups) != 0 )
                {
                        disconnect_server(client);
                }

                /* close group and reconnect */
                closegroup();
                client->group = newgroup;

                switch ( connect_groupserver(client, svr->Name) )
                {
                        case CONNECT_ERR:
                                return swriteclient(client, "500 Error selecting server\r\n");

                        case CONNECT_DOWN:
                                sleep(cfg.DownDelay);
                                return writeclient(client, MSG_SERVER_DOWN);

                        default:
                                break;
                }

                /* reset by connect_groupserver (via disconnect_server FIXME) */
                client->group = newgroup;

                client->groups++;
        }

        if ( ! writeserver(client, "GROUP %s\r\n", group) ) return -1;
        if ( ! readserver(client, b, MAX_SERVERRSP-1) ) return -1;

        syslog(LOG_NOTICE, "%s: group %s", client->hostname, group);

        if ( ! nogroupecho ) return swriteclient(client,b);
        return 0;
}


/*
 * List commands (active, active.times, newsgroups etc)
 */

PROTO int cmdlist(char *args)
{
        char cmd[16];

        if ( strlen(args) == 0 )
                return listactive(client,"*");

        CLEARBUF
        sscanf(args,"%16[^\r\n\t ] %256[^\r\n]", cmd, client->bbuf);
        if ( client->bbuf[0] == 0 ) strcpy(client->bbuf,"*");

        if ( !strncasecmp(cmd,"active.times",12) )
        {
                listactivetimes(client);
        }
        else if ( !strncasecmp(cmd,"active",6) )
        {
                listactive(client,client->bbuf);
        }
        else if ( !strncasecmp(cmd,"newsgroups",10))
        {
                listnewsgroups(client, client->bbuf);
        }
        else if ( !strncasecmp(cmd,"overview",8))
        {
                listoverviewfmt(client);
        }
        else
        {
                return swriteclient(client,"500 Unsupported LIST command\r\n");
        }
        return 0;
}


/*
 * XGTITLE implementation
 */

PROTO int cmdxgtitle(char *args)
{
        if ( strlen(args) == 0 )
                return listxgtitle(client,"*");

        CLEARBUF
        sscanf(args,"%256[^\r\n]", client->bbuf);
        if ( client->bbuf[0] == 0 ) strcpy(client->bbuf,"*");

        listxgtitle(client, client->bbuf);
        return 0;
}


/*
 * All of the authinfo processing routine
 */

PROTO int cmdauthinfo(char *args)
{
        char subcmd[17], subcmdargs[65];
        USER *newuser;
        PROFILE *oldprf;
        PROFILE *newprf;
        int change_profile;
        AUTHRESULT *authres;

        sscanf (args, "%16[^\r\n\t ] %64[^\r\n]", subcmd, subcmdargs);

        if ( client->authenticated )
                return swriteclient(client, MSG_AUTH_TWICE);

        if (!strcasecmp(subcmd,"user")) 
        {
                if (client->authenticated && strncmp(client->username, subcmdargs, 64))
                        return swriteclient (client, MSG_AUTH_TWICE);
                if ( client->vhost->AddSuffix[0] != '\0' ) {
                    snprintf(client->username, MAX_USERPASS-1, "%s@%s", subcmdargs, client->vhost->AddSuffix);
                } else {
                    strncpy(client->username, subcmdargs, MAX_USERPASS-1);
                }
        } else if ( !strcasecmp(subcmd, "pass") ) {
                strncpy(client->password, subcmdargs, MAX_USERPASS-1);
        } else {
                return swriteclient(client, MSG_AUTH_UNKNOWN);
        }

        if ( client->username[0] == 0 || client->password[0] == 0 )
                return swriteclient(client, MSG_MOREAUTHREQ);

        client->auth = getauth(client->username);

        if ( (authres=new_authresult(client)) == NULL )
                return swriteclient(client, MSG_AUTH_NOMEM);

        void (*authmod)() = find_auth_mod(client->auth->auth->lib);

        if (!authmod) {
                CLEARPASS
                syslog(LOG_ERR, "Can't find authmap: %s", client->auth->auth->lib);
                swriteclient(client, MSG_AUTH_ERR);
                return 0;
        }

        client->authenticated = 0;
        (*authmod)(authres, &cfg, client->auth->auth->args);
        CLEARPASS

        if ( authres->authenticated == true )
        {
                semlock_lock(master->semid);
                client->authenticated++;

                if ( authres->logname != NULL )
                        strcpy(client->logname, authres->logname);

                /* auth module specified profile */
                if ( authres->profile != NULL ) 
                {
                        if ( (newprf = find_user_profile(client->username)) == NULL )
                                if ( (newprf=getprofile(authres->profile)) == NULL )
                                {
                                        client->authenticated--;
                                        semlock_unlock(master->semid);
                                        syslog(LOG_ERR, "Missing profile information from authenticator, rejecting user %s",
                                                        authres->username);
                                        swriteclient(client, MSG_AUTH_ERR);
                                        free_authresult(authres);
                                        return 0;

                                }
                } else {
                        newprf = client->auth->profile;
                }

                PROFILE *p;
                if ( (p = prof_iplookup(newprf, client->addr.sin_addr)) != NULL )
                        newprf = p;

                if ( ! nullstr(newprf->Disabled) )
                {
                        client->authenticated--;
                        semlock_unlock(master->semid);
                        if ( cfg.LogAuthInfo )
                                syslog(LOG_NOTICE, "%s: authinfo login: %s result: 480 %s", 
                                        client->hostname, authres->username, newprf->Disabled);
                        return writeclient(client, "482 %s", newprf->Disabled);
                }

                /* Check and verify we are allowed in our new profile */

                change_profile = 0;
                oldprf = client->profile;
                if ( client->profile != newprf )
                {
                        if ( newprf->connections > newprf->MaxConnections ) 
                        {
                                client->authenticated--;
                                semlock_unlock(master->semid);
                                if ( cfg.LogAuthInfo )
                                        syslog(LOG_NOTICE, "%s: authinfo login: %s result: %s", 
                                                client->hostname, authres->username, MSG_AUTH_TOOMANYCON);
                                return swriteclient(client, MSG_AUTH_TOOMANYCON);
                        } 
                        else if ( newprf->numusers > newprf->MaxUsers )
                        {
                                client->authenticated--;
                                semlock_unlock(master->semid);
                                if ( cfg.LogAuthInfo )
                                        syslog(LOG_NOTICE, "%s: authinfo login: %s result: %s", 
                                                client->hostname, authres->username, MSG_AUTH_TOOMANYUSR);
                                return swriteclient(client, MSG_AUTH_TOOMANYUSR);
                        } else {
                                change_profile = 1;
                        }
                }


                /* Find our other connections from this user to see if we are allowed */

                newuser = find_user(client->username, newprf);
                if ( newuser != client->user && newuser != NULL )
                {
                        if ( newuser->connections > newprf->MaxUserConcurrent )
                        {
                                client->authenticated--;
                                drop_user(newuser->id, newprf);
                                semlock_unlock(master->semid);
                                sleep(cfg.RetryDelay);
                                if ( cfg.LogAuthInfo )
                                        syslog(LOG_NOTICE, "%s: authinfo login: %s result: %s", 
                                                client->hostname, authres->username, MSG_AUTH_USERFULL);
                                free_authresult(authres);
                                return swriteclient(client, MSG_AUTH_USERFULL);
                        }
                }

                if ( newuser == NULL )
                        syslog(LOG_ERR, "newuser == NULL");


                /* check number of different connected hosts from new user */

                int maxhosts = newprf->ASMaxHosts;
                int maxconns = newprf->ASMaxConns;
                if ( maxconns > 0 || maxhosts > 0 ) {
                    char httpresult[2048];
                    semlock_unlock(master->semid);
                    if ( ! check_http_accountsharing(client->username, client->hostname, maxhosts, maxconns, newprf->Name, httpresult) ) {
                            syslog(LOG_DEBUG, "HTTP account sharing returned false. maxhosts=%d, maxconns=%d, user=%s, host=%s, newprf=%s, result=%s", 
                                              maxhosts, maxconns, client->username, client->hostname, newprf->Name, httpresult);
                            semlock_lock(master->semid);
                            client->authenticated--;
                            drop_user(newuser->id, newprf);
                            semlock_unlock(master->semid);
                            sleep(cfg.RetryDelay);
                            free_authresult(authres);
                            return writeclient(client, "482 %s", httpresult);
                    }
                    semlock_lock(master->semid);
                }

                if ( change_profile == 1 )
                {
                        if ( client->profile->connections == 0 )
                                syslog(LOG_ERR, "Trying to drop 0 connections profile from auth: %s", client->profile->Name);
                        else
                                client->profile->connections--;

                        client->profile = newprf;
                        client->profile->connections++;
                }

                drop_user(client->user->id, oldprf);
                client->user = newuser;
                client->user->bytesleft = authres->bytes; // client->initbytes;
                client->posting = authres->posting;

                if ( authres->userkbit > 0 )
                        client->user->MaxUserBPS = authres->userkbit * 1000 / 8;
                else
                        client->user->MaxUserBPS = client->profile->MaxUserBPS;

                if ( authres->timeleft > 0 ) {
                    client->user->timeleft = authres->timeleft;
                }

                if ( authres->readpat ) {
                    strcpy(client->user->readpat, authres->readpat);
                }
                if ( authres->postpat ) {
                    strcpy(client->user->postpat, authres->postpat);
                }


//              syslog(LOG_DEBUG, "%s: assigning-change %s userkey %s", client->hostname, client->profile->Name, client->user->key);

                /* init ratelimiter */
                init_ratelimiter(client);
                semlock_unlock(master->semid);
        }

        if ( cfg.LogAuthInfo )
                syslog(LOG_NOTICE, "%s: authinfo login: %s result: %s", 
                        client->hostname, authres->username, authres->message ? authres->message : "UNKNOWN");
        swriteclient(client, authres->message ? authres->message : MSG_AUTH_ERR);

        free_authresult(authres);
        return 0;
}

/*
 * Check header permissions.
 * Does newsgroup checks if ReadPat is specified
 * Does Retention check if retention > 0
 */
static int check_headperm(char *msgid)
{
        char    *ng;
        time_t  msgdate;
        HLIST   *hdr = NULL, *allhdrs = NULL;
        char    header[MAX_HEADER];
        char    value[MAX_HEADER];
        char    *hval;
        CLEARBUF

        if ( cfg.LocalDreader == 1 && 
             strcmp(client->user->readpat, "all") == 0 && 
             client->profile->Retention == 0)
                return CHECK_ARTICLE_OK;

        if ( writeserver(client, "HEAD %s\r\n", msgid) == false ) 
                return CHECK_ARTICLE_FAIL;

        if ( (readserverline(client->bbuf, cfg.BufSize)) == NULL ) 
                return CHECK_ARTICLE_FAIL;

        if ( atoi(client->bbuf) > 399 )
                return CHECK_ARTICLE_FAIL;

        /* read headers from server */
        while( readserverline(client->bbuf, cfg.BufSize) != NULL ) 
        {
                if ((sscanf(client->bbuf, "%63[^\t ] %1023[^\r\n]", header, value)) == 0)
                        strncpy(value, client->bbuf, MAX_HEADER-1);

                hdr = insert_hlist(hdr, header, value);
                if ( allhdrs == NULL ) allhdrs = hdr;

                if ( header[0] == '.' )
                        break;
        }
        hdr = allhdrs;

        /* check retention time first */
        if ( client->profile->Retention > 0 )
        {
                msgdate = 0;

                if ( (hval=hlist_get_value(hdr, "NNTP-Posting-Date:")) != NULL )
                        msgdate = parsedate(hval);
                else
                if ( (hval=hlist_get_value(hdr, "Date:")) != NULL )
                        msgdate = parsedate(hval);

                if ( msgdate < (time(NULL) - (client->profile->Retention * 86400)) ) 
                {
                        free_hlist(hdr);
                        return CHECK_ARTICLE_NOPERM;
                }
        }

        /* check newsgroup permission */
        if ( (ng=hlist_get_value(hdr, "Newsgroups:")) != NULL )
        {
                free_hlist(hdr);
                /* now we've found the article and have the newsgroups header */
                if ( match_expression((unsigned char *)ng
                                        , (unsigned char *)getwildmat(client->user->readpat)
                                        , 0) ) 
                        return CHECK_ARTICLE_OK;
                else
                        return CHECK_ARTICLE_NOPERM;
        }

        /* If we reach this, the article wouldn't have a newsgroups header. */

        free_hlist(hdr);
        return CHECK_ARTICLE_FAIL;
}


/*
 * Find article server based on message id. Return true
 * on success, false on error.
 */
static int find_article_server(char *msgid)
{
        SERVER *svr, *olds;
        int i, oldlev = -1, x;

        if ( msgid[0] != '<' )
                return CHECK_ARTICLE_FAIL;

        olds = client->groupserver;
        if ( client->groupserver != NULL )
        {
                svr = client->groupserver;
                oldlev = svr->Level;

                x = check_headperm(msgid);
                if ( x == CHECK_ARTICLE_OK || x == CHECK_ARTICLE_NOPERM )
                        return x;
        }

        for( i=0; i<master->numservers; i++ )
        {
                if ( (master->servers+i)->Level != oldlev &&
                        (master->servers+i) != olds &&
                        strcmp((master->servers+i)->Name, "default") != 0 )
                {
                        svr = (master->servers+i);
                        oldlev = svr->Level;

                        if ( connect_groupserver(client, svr->Name) == CONNECT_OK )
                        {
                                x = check_headperm(msgid);
                                if ( x == CHECK_ARTICLE_OK || x == CHECK_ARTICLE_NOPERM )
                                        return x;
                        }
                }
        }

        /* no server has article */
        return CHECK_ARTICLE_FAIL;
}


/*
 * Basically a wrapper around find_article_server but
 * return some descriptive message to the client if either
 * the article is not found or we dont have permission
 * to retrieve it
 */

PROTO bool catch_art_server(char *msgid)
{
        int x;

        if ( client->group != NULL && msgid[0] != '<' )
                return true;

//      if ( client->group == NULL )
//      {
                if ( msgid[0] != '<' )
                {
                        swriteclient(client, MSG_ARTICLECANT);
                        return false;
                }

                if ( client->profile->ArticleSearch == 0 )
                {
                        swriteclient(client, MSG_ARTICLENOPERM);
                        return false;
                }

                x = find_article_server(msgid);

                switch (x)
                {
                        case CHECK_ARTICLE_FAIL:
                                swriteclient(client, MSG_ARTICLECANT);
                                return false;

                        case CHECK_ARTICLE_NOPERM:
                                swriteclient(client, MSG_ARTICLENOPERM);
                                return false;

                        case CHECK_ARTICLE_OK:
                        default:
                                return true;
                }
//      }
        return true;
}

