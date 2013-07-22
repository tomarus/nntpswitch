/*
 * Statistics Commands
 */

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"
#include "semaphore.h"
#include "json-c/json.h"
#include "curl.h"


/*
 * Load stats file, if available
 */
void cmdstatistics_loadfile(void)
{
    FILE *f;
    char buf[MAX_CFGLINE];

    char profile[MAX_NAME];
    int conns, users, real;
    ulong64 bytes, arts;
    PROFILE *pr;

    if ( (f=fopen(cfg.StatsFile, "r")) == NULL )
        return;

    while( fgets(buf, MAX_CFGLINE, f) )
    {
        if ( buf[0] == '\r' || buf[0] == '\n' || buf[0] == '#' || buf[0] == ';' || buf[0] == '-' )
            continue;

        if (sscanf (buf, "profile %255s %d %d %d %llu %llu", profile, &conns, &users, &real, &bytes, &arts) == 6) {
            pr = getprofile(profile);
            if ( pr == NULL ) {
                printf("Unknown profile while loading statsfile: %s\n", profile);
            } else {
                pr->bytes = bytes;
                pr->articles = arts;
            }
        }
    }

    fclose(f);
}


PROTO void load_statsfile(void)
{
    time_t now = time(NULL);
    struct stat fstat;

    if ( !(stat(cfg.StatsFile, &fstat)) )
        if ( fstat.st_mtime > (now-cfg.StatsFilePeriod) )
            cmdstatistics_loadfile();
}

/*
 * Write to file, called from timer event
 */
PROTO void cmdstatistics_writestats(void)
{
    FILE *f;
    int i;
    char tmpfile[MAX_STRING];

    sprintf(tmpfile, "%s.%d", cfg.StatsFile, (int)getpid() );

    if ( (f=fopen(tmpfile, "w")) == NULL )
        return;

    fprintf(f, "uptime %lu\n", time(NULL) - master->serverstart);
    fprintf(f, "nrforks %llu\n", master->nrforks);
    fprintf(f, "forkspersec %.2f\n", (float)master->nrforks / (float)(time(NULL) - master->serverstart) );
    fprintf(f, "nrlocks %llu\n", master->nrlocks);
    fprintf(f, "nrunlocks %llu\n", master->nrunlocks);
    fprintf(f, "-- name, connections, users, realusers, bytes, arts, postbytes, postarts\n");

    for (i=0; i<master->numprofiles; i++) {
        fprintf(f, "profile %s %u %u %u %llu %lu %llu %lu\n",
                (master->profiles+i)->Name,
                (master->profiles+i)->connections,
                (master->profiles+i)->numusers,
                (master->profiles+i)->realusers,
                (master->profiles+i)->bytes,
                (master->profiles+i)->articles,
                (master->profiles+i)->postbytes,
                (master->profiles+i)->postarticles);
    }

    fprintf(f, "-- Server connections:\n");
    for ( i=0; i<master->numservers; i++ )
        fprintf(f, "server %s %d\n", (master->servers+i)->Name, (master->servers+i)->connections );

    fclose(f);
    rename(tmpfile, cfg.StatsFile);
}


/*
 * write connection details to a file
 */
PROTO void cmdstatistics_write(void)
{
    FILE *f;
    CLIENT *c;
    int i;
    time_t nu = time(NULL);
    time_t s;
    char tmpfile[MAX_STRING];

    sprintf(tmpfile, "%s.%d", cfg.ConnDumpFile, (int)getpid() );

    if ( (f=fopen(tmpfile, "w")) == NULL ) {
        syslog(LOG_ERR, "Can't write %s (%m)", cfg.ConnDumpFile);
        return;
    }

    for ( i=0; i<MAX_PROC; i++ ) {
        if ( (master->clients+i)->inuse > 0 ) {
            c = (master->clients+i);
            s = nu - c->connectsince;

            if ( c->authenticated )
                fprintf(f, "%d h='%s' r='%s' s='%d' u='%s' P='%d'\n", c->id, c->hostname, c->profile->Name, (int)s, c->username, (int)c->pid );
            else
                fprintf(f, "%d h='%s' r='%s' s='%d' u='%s' P='%d'\n", c->id, c->hostname, c->profile->Name, (int)s, "", (int)c->pid );
        }
    }

    fclose(f);
    rename(tmpfile, cfg.ConnDumpFile);
}


PROTO void cmdstatistics_write_bytes_json(void)
{
    CLIENT *c;
    PROFILE *p;
    int i;

    struct json_object *js_obj = NULL;
    struct json_object *js_user_arr = NULL;
    struct json_object *js_prof_arr = NULL;

    js_obj = json_object_new_object();
    if ( js_obj == NULL ) {
        syslog(LOG_ERR, "Can't create json_object for statistics.");
        return;
    }
    js_user_arr = json_object_new_array();
    js_prof_arr = json_object_new_array();
    if ( js_user_arr == NULL || js_prof_arr == NULL ) {
        syslog(LOG_ERR, "Can't create js_user_arr or js_prof_arr for statistics.");
        return;
    }

    for ( i=0; i<MAX_PROC; i++ ) {
        if ( (master->clients+i)->inuse > 0 ) {
            c = (master->clients+i);
            char * ukey = c->authenticated ? c->username : c->hostname;

            struct json_object * jobj = json_object_new_object();
            struct json_object * js_bytes = json_object_new_int64(c->rbytes);
            if ( jobj == NULL || js_bytes == NULL ) {
                syslog(LOG_ERR, "Can't create jsobj or js_bytes for user statistics.");
                return;
            }
            json_object_object_add(jobj, ukey, js_bytes);
            json_object_array_add(js_user_arr, jobj);
            c->rbytes = 0;
        }
    }

    for ( i=0; i<MAX_PROFILES; i++ ) {
        p = (master->profiles+i);
        if ( p->bytes > 0 ) {
            struct json_object * jobj = json_object_new_object();
            struct json_object * js_bytes = json_object_new_int64(p->rbytes);
            if ( jobj == NULL || js_bytes == NULL ) {
                syslog(LOG_ERR, "Can't create jsobj or js_bytes for profile statistics.");
                return;
            }
            json_object_object_add(jobj, p->Name, js_bytes);
            json_object_array_add(js_prof_arr, jobj);
            p->rbytes = 0;
        }
    }

    json_object_object_add(js_obj, "u", js_user_arr);
    json_object_object_add(js_obj, "p", js_prof_arr);


    CURL * curl;
    struct curlMemStruct curlData;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if ( !curl ) {
        syslog(LOG_ERR, "Cant initialize curl during statistics");
        return;
    }
    char postdata[524288];
    char hostname[128];
    gethostname(hostname, 128);
    sprintf(postdata, "pid=%s:%d&js=%s", hostname, getpid(), json_object_to_json_string(js_obj));

    char * ret = curlPostCurlURL(cfg.RLURL, &curlData, curl, postdata);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if ( ret == NULL ) {
        return;
    }
    if ( curlData.mem[0] != 'O' || curlData.mem[1] != 'K' ) {
        syslog(LOG_ERR, "Statistics post data didn't return OK, but %s", curlData.mem);
    }
}


/* 
 * profile dumper
 */

void cmdstatistics_pdump(PROFILE *p)
{
    writeclient(client, "name: %s", p->Name);
    writeclient(client, "ReadPat: %s", p->ReadPat);
    writeclient(client, "PostPat: %s", p->PostPat);
    writeclient(client, "MaxConnections: %d", p->MaxConnections);
    writeclient(client, "MaxUserConcurrent: %d", p->MaxUserConcurrent);
    writeclient(client, "MaxHostConcurrent: %d", p->MaxHostConcurrent);
    writeclient(client, "Slave: %d", p->Slave);
    writeclient(client, "MaxUsers: %d", p->MaxUsers);
    writeclient(client, "MaxSessionTime: %d", p->MaxSessionTime);
    writeclient(client, "MaxSessionBytes: %lld", p->MaxSessionBytes);
    writeclient(client, "MaxUserBPS: %u", p->MaxUserBPS);
    writeclient(client, "MaxProfileBPS: %u", p->MaxProfileBPS);
    writeclient(client, "XComplaintsTo: %s", p->XComplaintsTo);
    writeclient(client, "Organization: %s", p->Organization);
    writeclient(client, "ForceOrganization: %d", p->ForceOrganization);
    writeclient(client, "AddNNTPPostingHost: %d", p->AddNNTPPostingHost);
    writeclient(client, "Hostname: %s", p->Hostname);
    writeclient(client, "FooterFile: %s", p->FooterFile);
    writeclient(client, "Footer: %s", p->Footer);
    writeclient(client, "FooterSize: %d", p->FooterSize);
    writeclient(client, "NoControl: %d", p->NoControl);
    writeclient(client, "ArticleSearch: %d", p->ArticleSearch);
    writeclient(client, "ClientReadTimeout: %d", p->ClientReadTimeout);

    writeclient(client, "-- accounting detail");
    writeclient(client, "Accounting/name: %s", p->Accounting->name);
    writeclient(client, "Accounting/lib: %s", p->Accounting->lib);
    writeclient(client, "Accounting/args: %s", p->Accounting->args);
    writeclient(client, "Limits: %d", p->NumLimits);

    writeclient(client, "-- statistics");
    writeclient(client, "connections: %d", p->connections);
    writeclient(client, "numusers: %d", p->numusers);
    writeclient(client, "realusers: %d", p->realusers);
    writeclient(client, "bytes: %llu", p->bytes);
    writeclient(client, "articles: %llu", p->articles);

    writeclient(client, "-- ratelimiter");
    writeclient(client, "rl_curbytes: %llu", p->rl_curbytes);
    writeclient(client, "rl_lastbytes: %llu", p->rl_lastbytes);
    writeclient(client, "rl_sleep: %lf", p->rl_sleep);
    writeclient(client, "rl_curbps: %lf", p->rl_curbps);
    writeclient(client, "UserRLFactorU: %lf", p->UserRLFactorU);
    writeclient(client, "UserRLFactorD: %lf", p->UserRLFactorD);
    writeclient(client, "ProfileRLFactorU: %lf", p->ProfileRLFactorU);
    writeclient(client, "ProfileRLFactorD: %lf", p->ProfileRLFactorD);
}


void cmdstatistics_user(USER *u)
{
    writeclient(client, "-- user information");
    writeclient(client, "key: %s", u->key);
    writeclient(client, "id: %u", u->id);

    writeclient(client, "MaxUserBPS: %u", u->MaxUserBPS);
    writeclient(client, "bytesleft: %llu", u->bytesleft);
    writeclient(client, "connections: %u", u->connections);
    writeclient(client, "realuser: %u", u->realuser);
    writeclient(client, "timeleft: %u", u->timeleft);

    writeclient(client, "rl_curbytes: %llu", u->rl_curbytes);
    writeclient(client, "rl_lastbytes: %llu", u->rl_lastbytes);
    writeclient(client, "rl_sleep: %lf", u->rl_sleep);
    writeclient(client, "rl_curbps: %lf", u->rl_curbps);
}


/*
 * nntp statistics command
 */
PROTO void cmdstatistics(char *opt)
{
    int i;

    if ( client->acl->stats != 1 ) {
        swriteclient(client, MSG_UNSUPPORTED);
        return;
    }

    semlock_lock(master->semid);
    if ( strcasecmp(opt, "write") == 0 ) {
        cmdstatistics_write();
    }

    if ( strcasecmp(opt, "pdump") == 0 ) {
        writeclient(client, "100 Profile Dump\r\n");
        cmdstatistics_pdump(client->profile);
        cmdstatistics_user(client->user);
        writeclient(client, ".");
        semlock_unlock(master->semid);
        return;
    }

    writeclient(client, "100 Statistics\r\n");
    writeclient(client, "uptime %lu", time(NULL) - master->serverstart);
    writeclient(client, "nrforks %llu", master->nrforks);
    writeclient(client, "forkspersec %.2f\n", (float)master->nrforks / (float)(time(NULL) - master->serverstart) );
    writeclient(client, "nrlocks %llu\n", master->nrlocks);
    writeclient(client, "nrunlocks %llu\n", master->nrunlocks);
    writeclient(client, "-- name, connections, users, realusers, bytes, arts, postbytes, postarts");

    for (i=0; i<master->numprofiles; i++) {
        writeclient(client, "profile %s %u %u %u %llu %lu %llu %lu\r\n",
                (master->profiles+i)->Name,
                (master->profiles+i)->connections,
                (master->profiles+i)->numusers,
                (master->profiles+i)->realusers,
                (master->profiles+i)->bytes,
                (master->profiles+i)->articles,
                (master->profiles+i)->postbytes,
                (master->profiles+i)->postarticles);
    }

    swriteclient(client, "-- Server connections:");
    for ( i=0; i<master->numservers; i++ ) {
        writeclient(client, "server %s %d", 
                    (master->servers+i)->Name, 
                    (master->servers+i)->connections
        );
    }

    writeclient(client, ".");
    semlock_unlock(master->semid);
    return;
}

