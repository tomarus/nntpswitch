/*
 * NNTPSwitch LDAP Authenticator
 * $Id: auth_ldap.c,v 1.6 2009-10-07 13:06:00 tommy Exp $
 *
 * Original Idea:
 *
 * checkpassword implementation that searches an LDAP database
 * for the uid first.
 *
 * Copyright (C) 2003 Scott James Remnant <scott@netsplit.com>.
 * #Id: checkpassword-ldap.c 33 2003-08-26 16:29:34Z scott #
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL SOFTWARE IN THE PUBLIC INTEREST, INC. BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LDAP_DEPRECATED 1
#include <ldap.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

/* Customise these to change the behaviour.
 * LDAP_HOST    hostname of your LDAP server
 * LDAP_BASE    base under which entry must exist
 * LDAP_SCOPE   search scope relative to base
 * LDAP_FILTER  filter to use, must contain a single %s which is replaced
 *              with the login name
 * LDAP_UID     name of field containing uid
 */
#define LDAP_HOST       "localhost"
#define LDAP_BASE       "ou=People,dc=support,dc=net"
#define LDAP_SCOPE      LDAP_SCOPE_SUBTREE
#define LDAP_FILTER     "(&(objectClass=posixAccount)(uid=%s))"
#define LDAP_UID        "uid"

PROTO void auth_ldap(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	char *attrs[] = { NULL };
	char *filter, *dn;
	LDAP *ld;
	LDAPMessage *res, *entry;
	int ret;

	ld = ldap_init(LDAP_HOST, LDAP_PORT);
	if (!ld) 
	{
		syslog(LOG_ERR, "auth_ldap: unable to initialize ldap connection");
		ldap_unbind(ld);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	filter = malloc(sizeof(LDAP_FILTER) + strlen(authres->username));
	sprintf(filter, LDAP_FILTER, authres->username);

	ret = ldap_search_s(ld, LDAP_BASE, LDAP_SCOPE, filter, attrs, 0, &res);
	if (ret) 
	{
		syslog(LOG_ERR, "auth_ldap: ldap search failed: %s", ldap_err2string(ret));
		ldap_unbind(ld);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	entry = ldap_first_entry(ld, res);
	if (!entry) 
	{
		/* probably unknown username */
		ldap_unbind(ld);
		authres->message = strdup(MSG_AUTH_REJ);
		return;
	}

	dn = ldap_get_dn(ld, res);
	ldap_msgfree(res);

	ret = ldap_simple_bind_s(ld, dn, authres->password);
	if (ret) 
	{
		/* probably wrong password */
		ldap_memfree(dn);
		ldap_unbind(ld);
		authres->message = strdup(MSG_AUTH_REJ);
		return;
	}

	ldap_memfree(dn);
	ldap_unbind(ld);

	authres->authenticated = true;
	authres->message = strdup(MSG_AUTH_OK);
	return;
}

