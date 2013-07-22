/*
 * Test memcache buggyness.
 * gcc -lmemcached -otest test.c
 */

#include <stdio.h>
#include <string.h>
#include <libmemcached/memcached.h>

static memcached_st *memc_as = NULL;
// const char *servername = "127.0.0.1:11211";     /* always OK */
const char *servername = "172.24.8.100:11211";
const int BUFSIZE = 512;


int init_cache(void)
{
    memcached_server_st *servers;
    memc_as = memcached_create(NULL);

    servers = memcached_servers_parse(servername);
    memcached_server_push(memc_as, servers);
    memcached_server_list_free(servers);

    if ( memc_as == NULL ) {
        fprintf(stderr, "memcached_as: Failed to initialize memcached!");
        return 0;
    }
    return 1;
}


int set_key(char *key, char *val, time_t expire)
{
    memcached_return rc;
    rc = memcached_set(memc_as, key, strlen(key), val, strlen(val), expire, 0);

    if ( rc == MEMCACHED_SUCCESS ) {
        return 1;
    }
    else {
        fprintf(stderr, "set_key: error: %s", memcached_strerror(memc_as, rc));
        if ( memc_as->cached_errno )
            fprintf(stderr, "set_key: system error: %s", strerror(memc_as->cached_errno));
    }
    return 0;
}


/* Try increment and if the key is not found, set it. */
int try_incr(char *key)
{
    memcached_return rc;
    unsigned long int res;
    time_t expire;

    expire = time(NULL) + (2*86400);

    rc = memcached_increment(memc_as, key, strlen(key), 1, &res);
    if ( rc == MEMCACHED_SUCCESS ) {
        return 1;
    }
    else if ( rc == MEMCACHED_NOTFOUND ) {
        /* Not found, set value to "1". */
        return set_key(key, "1", expire);
    }
    return 0;
}


int try_decr(char *key)
{
    memcached_return rc;
    unsigned long int res;

    rc = memcached_decrement(memc_as, key, strlen(key), 1, &res);
    if ( rc == MEMCACHED_SUCCESS ) {
        return 1;
    } else {
        fprintf(stderr, "try_decr error: %s (key=%s) (res=%lu)", memcached_strerror(memc_as, rc), key, res);
        if ( memc_as->cached_errno )
            fprintf(stderr, "try_decr system error: %s (key=%s)", strerror(memc_as->cached_errno), key);
    }
    return 0;
}


int delete_key(char *key)
{
    memcached_return rc;
    rc = memcached_delete(memc_as, key, strlen(key), (time_t)0);

    if ( rc == MEMCACHED_SUCCESS ) {
        return 1;
    } else {
        fprintf(stderr, "delete_key error: %s", memcached_strerror(memc_as, rc));
        if ( memc_as->cached_errno )
            fprintf(stderr, "delete_key system error: %s", strerror(memc_as->cached_errno));
    }
    return 0;
}


int main(int argc, char **argv)
{
    int i;
    char keyname[BUFSIZE];
    char buf[BUFSIZE];
    time_t expire;
    int res;

    fprintf(stderr, "Starting..\n");
    if ( memc_as == NULL )
        if ( ! init_cache() )
            return;

    expire = time(NULL) + (2*86400);

    for (i=0; i<1000000; i++) {
        sprintf(keyname, "%s%d", "tommy", i);

        /* Try increment or insert some key. */
        sprintf(buf, "%s.conns", keyname);
        if ( try_incr(buf) == 0 )
            fprintf(stderr, "try_incr failed\n");

        /* Try to decrease the value. */
        if ( try_decr(buf) == 0 )
            fprintf(stderr, "try_decr failed\n");

        /* Cleanup. */
        if ( delete_key(buf) == 0 )
            fprintf(stderr, "delete failed\n");
    }
    fprintf(stderr, "All tests done.\r\n");
}

