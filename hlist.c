//#include <unistd.h>
//#include <stdlib.h>
//#include <string.h>



#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"


PROTO void free_hlist(HLIST *h)
{
        HLIST *h2 = h;
        HLIST *h3 = NULL;

        do
        {
                free(h2->val);
                free(h2->hdr);
                h3 = h2;
                h2 = h2->next;
                free(h3);
        }
        while( h2 != NULL );
}

PROTO HLIST * insert_hlist(HLIST *first, char *hdr, char *val)
{
	HLIST *i = first, *last = NULL;
	while (i != NULL) { last=i; i=i->next; }

	i = malloc(sizeof(HLIST));
	if (i == NULL) { die("No room for HLIST"); }

	if (last != NULL) last->next = i;
	i->hdr = strdup(hdr);
	i->val = strdup(val);
	i->next = NULL;
	return i;
}

PROTO char * hlist_get_value(HLIST *first, char *hdr)
{
	HLIST *i = first;

	while ( i != NULL && strcasecmp(i->hdr, hdr) )
		i = i->next;

	if ( i == NULL )
		return NULL;

	return i->val;
}

PROTO char * hlist_replace_value(HLIST *first, char *hdr, char *replace)
{
	HLIST *i = first;

	while ( i != NULL && strcasecmp(i->hdr, hdr) )
		i = i->next;

	if ( i == NULL )
		return NULL;

	free(i->val);
	i->val = strdup(replace);
	
	if ( i->val == NULL )
		syslog(LOG_ERR, "Out of memory for hlist replace");

	return i->val;
}

