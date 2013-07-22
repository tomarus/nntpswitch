#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <math.h>

#include <../nntpd.h>
#include "../aconfig.h"
#include "../aprotos.h"


#undef MAPDEBUG


/*
 * Prefix Maps work by stripping the least 8 bits of an ip range and dividing
 * the prefix itself by a /24. This means each /24 network holds 1 bit. For the
 * total ipv4 range with a resolution of /24 this means a 2 MB map.
 * Turn on MAPDEBUG to see debug output.
 */


PROTO void parsecfg_map(char *buf, FILE *f, int linenr, char *acfile)
{
        char mapname[128], mappath[1024];
        mapname[0] = 0;
        mappath[0] = 0;
        int pfxnum;
	PFXMAP *pmap;

        if ( sscanf(buf, "pfxmap%*[ \t]%128[^ \t]%*[ \t]%1024s", mapname, mappath) != 2 )
                die("Prefixmap config syntax error on line %d of %s", linenr, acfile);

	pmap = getpfxmap(mapname);

	if ( pmap == NULL )
	{
        	strcpy((master->prefixmap+master->num_pfxmaps)->name, mapname);
        	pfxnum = load_prefixmap( (master->prefixmap+master->num_pfxmaps)->map, mappath);
        	info("Loaded pfxmap %s with %d prefixes", mapname, pfxnum);

        	master->num_pfxmaps++;
        	if ( master->num_pfxmaps >= MAX_PFXMAPS )
                	die("Too many prefixmaps, increase MAX_PFXMAPS");
	} else {
        	pfxnum = load_prefixmap(pmap->map, mappath);
        	info("Reloaded pfxmap %s with %d prefixes (%d maps)", mapname, pfxnum, master->num_pfxmaps);
	}
}



PROTO int load_prefixmap(char *ptr, char *filename)
{
	FILE *f;
	char buf[128];
	char ip[16], pfx[3];
	int line = 0;

	if ( (f=fopen(filename, "r")) == NULL )
		die("Cant open file %s (%m)", filename);

	memset(ptr, 0, PFX_MAPSIZE);

	while( fgets(buf, 128, f) )
	{
		line++;

		if ( sscanf(buf, "%15[0-9.]%*[/]%2[0-9]", ip, pfx ) < 2 )
			syslog(LOG_ERR, "Wrong entry on line %d of %s", line, filename);
		else
			parse_prefix(ip, pfx, ptr);
	}

	return line;
}


PROTO void parse_prefix(char *ip, char *pfx, char *ptr)
{
	struct in_addr inaddr;
	unsigned long ipstart;
	unsigned long size;
	unsigned long i;

	inet_aton(ip, &inaddr);

	ipstart = ntohl(inaddr.s_addr) >> 8;
	size = (1 << (32-atol(pfx))) >> 8;

	for (i=0; i<size; i++)
	{
		unsigned int offset = (ipstart + i) >> 3;
		unsigned int bit = ( ipstart + i ) & 7;
		ptr[offset] |= 1<<bit;
	}
}


/*
 * get map based on name
 */
PROTO PFXMAP* getpfxmap(const char *name)
{
        int i;

        for (i=0; i<master->num_pfxmaps; i++)
                if ( strcmp((master->prefixmap+i)->name, name) == 0 )
                        return master->prefixmap+i;

        return NULL;
}


PROTO PROFILE * prof_iplookup(PROFILE *p, struct in_addr inaddr)
{
	if ( p->PrefixMap == NULL )
		return NULL;

	if ( pfx_iplookup(inaddr, p->PrefixMap) )
		return NULL;
	
	return p->FailProfile;
}


PROTO void auth_iplookup(AUTHRESULT * authres)
{
	PROFILE *p;

	p = getprofile(authres->profile);
	if ( p == NULL )
	{
		syslog(LOG_ERR, "Cant find profile in auth_iplookup (profile=%s)", authres->profile);
		return;
	}

	if ( p->PrefixMap != NULL )
	{
		if ( pfx_iplookup(authres->in_addr, p->PrefixMap) )
			return;
		else
			authres->profile = strdup(p->FailProfile->Name);
	}
}


PROTO int pfx_iplookup(struct in_addr inaddr, PFXMAP *map)
{
	unsigned long offset;
	unsigned char bit;

	offset = ntohl(inaddr.s_addr) >> 11;
	bit = 1 << (ntohl(inaddr.s_addr) >> 8 & 7);

	if ( map->map[offset] & bit )
		return 1;
	else
		return 0;
}


#if 0
int test_prefixes(void)
{
	int i;

	load_prefixmap();

	printf("Random IP lookups:\n");
	lookup_ip("10.10.10.10");
	lookup_ip("195.114.228.129");
	lookup_ip("194.109.21.4");

	printf("Doing 10.000.000 lookups:\n");
	for (i=0; i<10000000; i++)
	{
		lookup_ip("100.100.100.100");
	}
	
	return(0);
}
#endif

