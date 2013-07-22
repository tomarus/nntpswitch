/*
 * NNTPSwitch common functions
 *
 * $Id: common.c,v 1.31 2009-10-07 13:02:25 tommy Exp $
 */

#include "aconfig.h"
#include "log.h"
#include "nntpd.h"
#include "aprotos.h"

MASTER *master = NULL;
CLIENT *client = NULL;


PROTO void die(const char *str, ...)
{
	va_list ap;
	char s[1024];

	va_start(ap, str);
	vsprintf(s, str, ap);

	if ( errno )
	{
		syslog(LOG_ERR, "died: %s (%s)", s, strerror(errno));
		fprintf(stderr, "%d: died: %s (%s)\n", getpid (), s, strerror(errno));
	} else {
		syslog(LOG_ERR, "died: %s", s);
		fprintf(stderr, "%d: died: %s\n", getpid (), s);
	}

	va_end(ap);
	syslog_close ();
	exit(1);
}

PROTO void info(const char *str, ...)
{
	va_list ap;
	char s[1024];

	va_start(ap, str);
	vsnprintf(s, sizeof(s), str, ap);

	syslog(LOG_NOTICE, "%s", s);
//	fprintf(stderr, "info: %s\n", s);
	va_end(ap);
}

PROTO void error(const char *str, ...)
{
	va_list ap;
	char s[1024];

	va_start(ap, str);
	vsnprintf(s, sizeof (s), str, ap);

	syslog(LOG_ERR, "%s", s);
	fprintf(stderr, "%d: error: %s\n", getpid (), s);
	va_end(ap);
}

PROTO char* strtolower(char *str)
{
	int i=0;
	while(str[i]) 
	{
		str[i] = tolower(str[i]); 
		i++;
	}
	return str;
}


/*
 * strip \r\n combinations
 */
PROTO char* striprn(const char *s)
{
	char *str, *end;

	str = strdup(s);
	end = str;

	str += strlen(end) - 1;
	while( *str == '\r' || *str == '\n' )
	{
		*str = '\0';
		str--;
	}
	return end;
}


/*
 * strip \r\n combinations, replace with something
 */
PROTO char* replacern(char *s, char r)
{
	char *str, *end;

	str = strdup(s);
	end = str;

	str += strlen(end) - 1;
	while( str != end )
	{
		if ( *str == '\r' || *str == '\n' )
		{
			*str = r;
		}
		str--;
	}
	return end;
}


PROTO int nullstr(const char *s)
{
	if ( s == NULL ) return 1;
	if ( s[0] == '\0' ) return 1;
	return 0;
}


PROTO void chop(char *buf)
{
	char *s;

	s = buf;
	s += strlen(buf) -1;
	while ( *s == '\r' || *s == '\n' || *s == ' ' )
	{
		*s = '\0';
		s--;
	}
}


/*
 * Strip "pattern" from buffer
 * tommy@news-service.com,*@news-service.com gets tommy
 * news-service@tommy,news-service@* gets tommy
 */
PROTO char* strippat(char *buf, char *pat)
{
	char *s;
	int l = strlen(buf)-1;
	int j = strlen(pat)-1;

	if ( pat[0] == '*' )
	{
		s = strdup(buf);

		while( pat[j] != '*' && l > 0 )
		{
			s[l] = 0;
			j--;
			l--;
		}
		return s;
	}
	else if ( pat[j] == '*' )
	{
		s = strdup(buf);

		while( *pat++ != '*' )
			s++;
		return s;
	}
	return NULL;
}


/*
 * checkbuf functions
 * check for \r\n.\r\n end of article in a buffer
 */
PROTO void checkbuf_init(char *buf)
{
	buf[0] = '\0';
	buf[1] = '\0';
	buf[2] = '\0';
	buf[3] = '\r';
	buf[4] = '\n';
}


PROTO int checkbuf_isend(char *buf, char *rcvbuf, int bread)
{
	char chkbuf[5];
	int j, k;

	if ( bread < 5 )
	{
		k=0;

		/* compose the buf from our checkbuf and received data */
		for (j=0; j<5-bread; j++ )
			chkbuf[k++] = buf[j+bread];
		for (j=0; j<bread; j++)
			chkbuf[k++] = rcvbuf[j];

		/* check if it's the end.. */
		if (
			chkbuf[0] == '\r' &&
			chkbuf[1] == '\n' &&
			chkbuf[2] == '.'  &&
			chkbuf[3] == '\r' &&
			chkbuf[4] == '\n'
		)
			return 1;

	} else {
		/* check if perhaps this is the end already */
		if (
			rcvbuf[bread-5] == '\r' &&
			rcvbuf[bread-4] == '\n' &&
			rcvbuf[bread-3] == '.'  &&
			rcvbuf[bread-2] == '\r' &&
			rcvbuf[bread-1] == '\n'
		)
			return 1;

		/* copy the last 5 bytes of the receive buf to checkbuf */
		for (j=0; j<5; j++)
			buf[j] = rcvbuf[bread-5+j];

	}
	return 0;
}


/*
 * escape ' in strings
 * free() string after use
 */
PROTO char * str_escape(char *instr)
{
	char buf[MAX_STRING];
	char *p;
	int i = 0;

	for( p=instr; *p!=0; p++ )
	{
		if ( *p == '\'' )
			buf[i++] = '\\';

		if ( *p == '\\' )
			buf[i++] = '\\';

		buf[i++] = *p;
	}

	buf[i++] = 0;
	return strdup(buf);
}


/*
 * Simple Email Check
 * Check for atleast one . and one @
 * Return 1 on OK, 0 on Fail
 */
PROTO int check_email(const char *addr)
{
    const char *c;
    int dot=0, ap=0;

    for (c=addr; *c; c++)
    {
        if ( *c == '.' )
            dot=1;

        else if ( *c == '@' )
            ap=1;
    }

    return dot & ap;
}

/*
 * Simple Message-ID Check
 * Check for < and > and atleast one . and one @
 * Return 1 on OK, 0 on Fail
 */
PROTO int check_message_id(const char *addr)
{
    const char *c;
    int ap=0;

    if ( *addr != '<' )
        return 0;

    for (c=addr; *c; c++)
        if ( *c == '@' )
            ap=1;

    c--;
    if ( *c != '>' )
        return 0;

    return ap;
}

