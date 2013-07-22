/*
 * access.c 
 * NNTPSwitch read and parse access.conf
 *
 * $Id: access.c,v 1.54 2011-06-02 17:50:34 tommy Exp $
 */

#include "aconfig.h"
#include "nntpd.h"
#include "memory.h"
#include "aprotos.h"

int linenr;
char acfile[MAX_PATH];

/*
 * Loading functions.
 *
 * Load the access.conf and parse each recognized statement
 */

static void parsecfg_auth (char *buf, FILE *file)
{
	char mask[MAX_STRING] = "";
	char alias[MAX_NAME] = "";
	char profile[MAX_NAME] = "";

	if (sscanf (buf, "auth%*[ \t]%255s %31s %31s", mask, alias, profile) <= 0)
		die ("Parse error on line %d of %s", linenr, acfile);

	ALIAS *aliasref;
	if (nullstr (alias))
		aliasref = getalias (DEFAULT_AUTH);
	else
		aliasref = getalias (alias);

	if (aliasref == NULL)
		die ("Undefined Alias \"%s\" on line %d of %s", alias, linenr, acfile);

	PROFILE *profileref;
	if (nullstr (profile))
		profileref = getprofile (DEFAULT_PROFILE);
	else
		profileref = getprofile (profile);

	if (profileref == NULL)
		die ("Undefined Profile on line %d of %s", linenr, acfile);

	AUTH *auth = getauthmask (mask);

	if (auth == NULL) {
		strcpy ( (master->auths+master->numauths)->mask, mask);
		(master->auths+master->numauths)->auth = aliasref;
		(master->auths+master->numauths)->profile = profileref;
		master->numauths++;
	} else {
		auth->auth = aliasref;
		auth->profile = profileref;
	}
}


PROTO void load_access_conf(void) 
{
	int i;

	if ( cfg.RemoteConfig )
		rconf_getwmats();

	strcpy(acfile, cfg.AccessFile);
	read_acs_file();

	/* do some config checks */
	if ( getprofile("default") == NULL )
		die("Profile \"default\" not defined");

	if ( getalias(DEFAULT_AUTH) == NULL )
		die("Alias \"%s\" not defined", DEFAULT_AUTH);

	if ( getalias(DEFAULT_ACCT) == NULL )
		die("Alias \"%s\" not defined", DEFAULT_ACCT);

	for (i = 0; i < master->numauths; i++) {
		if (find_auth_mod (master->auths[i].auth->lib) == NULL) {
			die ("Unknown authorization module specified in access.conf (%s)"
					, master->auths[i].auth->lib);
		}
	}

	for (i = 0; i < master->numprofiles; i++) {
		if (find_acct_mod (master->profiles[i].Accounting->lib) == NULL) {
			die ("Unknown accounting module specified in access.conf (%s)"
					, master->profiles[i].Accounting->lib);
		}
	}

	info("Loaded %d Wildmats, %d Aliases, %d ACLs, %d Profiles, %d Authenticators, %d Arguments, %d VHosts", 
			master->numwildmats, 
			master->numaliases, 
			master->numacls, 
			master->numprofiles, 
			master->numauths,
			master->numargs,
			master->numvhosts);
}

/*
 * Configuration parts parser functions
 */

PROTO void read_acs_file(void) 
{
	FILE *f;
	char buf[MAX_CFGLINE], b2[MAX_CFGLINE];
	int oldlinenr;
	char oldacfile[MAX_PATH];

	if ( (f=fopen(acfile,"r")) == NULL )
		die("Can't open profile config %s", acfile);

	linenr=0;
	while( fgets(buf, MAX_CFGLINE, f) ) 
	{
		chop(buf);
		linenr++;

		if ( buf[0] != '\r' && buf[0] != '\n' && buf[0] != ';' && buf[0] != '#' && buf[0] != 0 ) 
		{
			if ( strncasecmp(buf, "profile", 7) == 0 )
				parsecfg_profile(buf, f);
			else 
			if ( strncasecmp(buf, "wildmat", 7) == 0 ) {
				if ( ! cfg.RemoteConfig )
					parsecfg_wildmat(buf,f);
			} else 
			if ( strncasecmp(buf, "alias", 5) == 0 ) 
				parsecfg_alias(buf,f);
			else 
			if ( strncasecmp(buf, "acl", 3) == 0 ) 
				parsecfg_acl(buf,f);
			else 
			if ( strncasecmp(buf, "auth", 4) == 0 ) 
				parsecfg_auth(buf,f);
			else 
			if ( strncasecmp(buf, "args", 4) == 0 ) 
				parsecfg_args(buf,f);
			else 
			if ( strncasecmp(buf, "include", 7) == 0 ) 
			{
				oldlinenr = linenr;
				strncpy(oldacfile, acfile, MAX_PATH-1);

				sscanf(buf, "%s %s", b2, acfile);
				check_path(acfile);
				read_acs_file();

				linenr = oldlinenr;
				strncpy(acfile, oldacfile, MAX_PATH-1);
			}
			else 
			if ( strncasecmp(buf, "pfxmap", 3) == 0 )
				parsecfg_map(buf, f, linenr, acfile);
			else
			if ( strncasecmp(buf, "vhost", 5) == 0 ) 
				parsecfg_vhost(buf,f);
			else 
				die("Syntax error in %s on line %d", acfile, linenr);
		}
	}
	fclose(f);
}


PROTO void profile_copycfg(char *buf, FILE *f, PROFILE *pf) 
{
	char key[256], val[256];
	int i=0;

	while( i == 0 && fgets(buf, MAX_CFGLINE, f) ) 
	{
		chop(buf);
		linenr++;

		if ( buf[0] == ';' || buf[0] == '#' || buf[0] == 0 )
			continue;

		if ( strncasecmp(buf, "end", 3) == 0 ) 
		{
			i++;
			break;
		}

		key[0] = 0;
		val[0] = 0;

		if ( sscanf(buf, "%*[ \t]%64s%*[ \t]%192[^\r\n]%*[\t\r\n]", key, val) != 2 )
			die("Error on line %d of %s (%s - %s)", linenr, acfile, key, val);

		if ( strcasecmp(key, "ReadPat") == 0 ) 
		{
			if ( getwildmat(val) == NULL )
				die("Unknown wildmat on line %d of %s (%s)", linenr, acfile, val);
			strcpy(pf->ReadPat, val);
		} else 
		if ( strcasecmp(key, "PostPat") == 0 ) 
		{
			if ( getwildmat(val) == NULL )
				die("Unknown wildmat on line %d of %s (%s)", linenr, acfile, val);
			strcpy(pf->PostPat, val);
		}else 
		if ( strcasecmp(key, "Banner") == 0 ) 
                        syslog(LOG_NOTICE, "Deprecation Warning: Banner config is moved from profile to vhosts. Profile: %s", pf->Name);
		else 
		if ( strcasecmp(key, "ReplaceBanner") == 0 ) 
                        syslog(LOG_NOTICE, "Deprecation Warning: Banner config is moved from profile to vhosts. Profile: %s", pf->Name);
		else 
		if ( strcasecmp(key, "MaxConnections") == 0 ) 
			pf->MaxConnections = atoi(val);
		else 
		if ( strcasecmp(key, "MaxUserConcurrent") == 0 ) 
			pf->MaxUserConcurrent = atoi(val);
		else 
		if ( strcasecmp(key, "MaxHostConcurrent") == 0 ) 
			pf->MaxHostConcurrent = atoi(val);
		else 
		if ( strcasecmp(key, "ASMaxConns") == 0 ) 
			pf->ASMaxConns = atoi(val);
                else
		if ( strcasecmp(key, "ASMaxHosts") == 0 ) 
			pf->ASMaxHosts = atoi(val);
                else
		if ( strcasecmp(key, "Slave") == 0 ) 
			pf->Slave = atoi(val);
		else 
		if ( strcasecmp(key, "MaxUsers") == 0 )
			pf->MaxUsers = atoi(val);
		else 
		if ( strcasecmp(key, "MaxSessionTime") == 0 ) 
			pf->MaxSessionTime = atoi(val);
		else 
		if ( strcasecmp(key, "MaxSessionBytes") == 0 )
			pf->MaxSessionBytes = atoll(val);
		else 
		if ( strcasecmp(key, "MaxUserBPS") == 0 ) 
			pf->MaxUserBPS = atoi(val);
		else 
		if ( strcasecmp(key, "MaxUserKbit") == 0 ) 
			pf->MaxUserBPS = atoi(val) * 1000 / 8;
		else 
		if ( strcasecmp(key, "MaxUserMbit") == 0 ) 
			pf->MaxUserBPS = atoi(val) * 1000000 / 8;
		else 
		if ( strcasecmp(key, "MaxProfileBPS") == 0 ) 
			pf->MaxProfileBPS = atoi(val);
		else 
		if ( strcasecmp(key, "MaxProfileKbit") == 0 ) 
			pf->MaxProfileBPS = atoi(val) * 1000 / 8;
		else 
		if ( strcasecmp(key, "MaxProfileMbit") == 0 ) 
			pf->MaxProfileBPS = atoi(val) * 1000000 / 8;
		else 
		if ( strcasecmp(key, "XComplaintsTo") == 0 ) 
			strcpy(pf->XComplaintsTo, val);
		else 
		if ( strcasecmp(key, "Organization") == 0 ) 
			strcpy(pf->Organization, val);
		else 
		if ( strcasecmp(key, "ForceOrganization") == 0 ) 
			pf->ForceOrganization = atoi(val);
		else 
		if ( strcasecmp(key, "AddNNTPPostingHost") == 0 ) 
			pf->AddNNTPPostingHost = atoi(val);
		else 
		if ( strcasecmp(key, "MaxCross") == 0 ) 
			pf->MaxCross = atoi(val);
		else 
		if ( strcasecmp(key, "NoControl") == 0 ) 
			pf->NoControl = atoi(val);
		else 
		if ( strcasecmp(key, "ArticleSearch") == 0 ) 
			pf->ArticleSearch = atoi(val);
		else 
		if ( strcasecmp(key, "Hostname") == 0 ) 
			strcpy(pf->Hostname, val);
		else 
		if ( strcasecmp(key, "FooterFile") == 0 ) 
			strcpy(pf->FooterFile, val);
		else 
		if ( strcasecmp(key, "UserRLFactorU") == 0 ) 
			pf->UserRLFactorU = atof(val);
		else 
		if ( strcasecmp(key, "UserRLFactorD") == 0 ) 
			pf->UserRLFactorD = atof(val);
		else 
		if ( strcasecmp(key, "ProfileRLFactorD") == 0 ) 
			pf->ProfileRLFactorD = atof(val);
		else 
		if ( strcasecmp(key, "ProfileRLFactorU") == 0 ) 
			pf->ProfileRLFactorU = atof(val);
		else 
		if ( strcasecmp(key, "ClientReadTimeout") == 0 ) 
	    		pf->ClientReadTimeout = atoi(val);
		else 
		if ( strcasecmp(key, "Limits") == 0 ) 
			parsecfg_limits(val, pf);
		else 
		if ( strcasecmp(key, "Accounting") == 0 ) 
		{
	    		pf->Accounting = getalias(val);
			if ( pf->Accounting == NULL )
				die("Unknown Accounting \"%s\" for profile \"%s\"", val, pf->Name);
		}
                else
                if ( strcasecmp(key, "AccountingPrefix") == 0 )
                        strcpy(pf->AccountingPrefix, val);
		else
		if ( strcasecmp(key, "Disabled") == 0 ) 
			strcpy(pf->Disabled, val);
		else 
		if ( strcasecmp(key, "Retention") == 0 )
			pf->Retention = atoi(val);
		else 
		if ( strcasecmp(key, "XHeaderSSLUpload") == 0 ) 
			strcpy(pf->XHeaderSSLUpload, val);
		else 
		if ( strcasecmp(key, "AllowSSL") == 0 ) 
			pf->AllowSSL = atoi(val);
		else
		if ( strcasecmp(key, "PrefixMap") == 0 )
		{
			pf->PrefixMap = getpfxmap(val);
			if ( pf->PrefixMap == NULL )
				die("Unknown prefixmap \"%s\" for profile \"%s\"", val, pf->Name);
		}
		else
		if ( strcasecmp(key, "FailProfile") == 0 )
		{
			pf->FailProfile = getprofile(val);
			if ( pf->FailProfile == NULL )
				die("Unknown failprofile \"%s\" for profile \"%s\"", val, pf->Name);
		}
		else
			die("Unknown profile option: %s %s", key, val);
	}

	if ( pf->UserRLFactorD == 0 )    pf->UserRLFactorD = 1.4;
	if ( pf->UserRLFactorU == 0 )    pf->UserRLFactorU = 1.4;
	if ( pf->ProfileRLFactorD == 0 ) pf->ProfileRLFactorD = 1.4;
	if ( pf->ProfileRLFactorU == 0 ) pf->ProfileRLFactorU = 1.4;
	if ( pf->ClientReadTimeout == 0 ) pf->ClientReadTimeout = cfg.ClientReadTimeout;
	pf->rl_sleep = RL_INITVAL;

	if ( pf->Hostname[0] == 0 ) 
		die("Hostname not defined for profile %s", pf->Name);

	cfg_load_footer(pf);
}


PROTO bool parsecfg_limits(char *args, PROFILE *pf)
{
	char *p;
	int num = 0;

	p = strtok(args, ",: ");
	do {
		pf->Limits[num].gb = atoi(p);
		p = strtok(NULL, ",: ");
		pf->Limits[num].kbit = atoi(p);
		num++;
	}
	while( (p=strtok(NULL,",: ")) );

	pf->NumLimits = num;
	return true;
}


/*
* If FooterFile is specified in the profile configuration
* read it here and put it into Footer[] for later use
* in the post() command.
*/

PROTO void cfg_load_footer(PROFILE *pf) 
{
	FILE *ff;
	char buf[MAX_CFGLINE];
	char tmp[MAX_FOOTER];
	char *ptr;

	if ( pf->FooterFile[0] != 0 )
	{
		pf->Footer[0] = 0;
		if ( (ff=fopen(pf->FooterFile, "r")) != NULL )
		{
			while( fgets(buf, MAX_CFGLINE, ff) )
			{
				ptr = striprn(buf);
				strcpy(tmp, pf->Footer);
				snprintf(pf->Footer, MAX_FOOTER, "%s%s\r\n", tmp, ptr);
				free(ptr);
			}

			fclose(ff);
			pf->FooterSize = strlen(pf->Footer);
		}
		else
		{
			die("Can't open FooterFile %s for Profile %s, %m", pf->FooterFile, pf->Name);
		}
	}
}


/*
 * Independent configuration parsers
 */

PROTO void parsecfg_profile(char *buf, FILE *f) 
{
	char key[256], val[256];
	PROFILE *defp;
	key[0] = 0;
	val[0] = 0;

	if ( (sscanf(buf, "%s %s", key, val)) != 2 )
		die("Profile Syntax error on line %d of %s", linenr, acfile);

	/* if we exist already, just overwrite us */
	if ( (defp = getprofile(val)) != NULL ) 
	{
		profile_copycfg(buf, f, defp);
		return;
	}

	/* nope didnt exist, create new profile */
	if ( strcmp(val, "default") != 0 ) 
	{
		if ( (defp = getprofile("default")) == NULL ) 
			die("Could not retrieve default profile, the default profile must be listed first.");

		/* copy from default profile */
		memcpy(master->profiles+master->numprofiles, defp, sizeof(PROFILE));

		/* reset counters since default profile might had some connections */
		(master->profiles+master->numprofiles)->connections = 0;
		(master->profiles+master->numprofiles)->numusers = 0;
		(master->profiles+master->numprofiles)->realusers = 0;
		(master->profiles+master->numprofiles)->bytes = 0;
		(master->profiles+master->numprofiles)->articles = 0;
	}

	strcpy((master->profiles+master->numprofiles)->Name, val);
	profile_copycfg(buf, f, master->profiles+master->numprofiles);

	master->numprofiles++;
	if ( master->numprofiles > MAX_PROFILES ) 
		die("Too many profiles (%d) increase MAX_PROFILES in nntpd.h", master->numprofiles);
}


PROTO void parsecfg_wildmat(char *buf, FILE *f) 
{
	char wmname[128], wmval[1024];
	wmname[0] = 0;
	wmval[0] = 0;
	WILDMAT *wm;

	if ( sscanf(buf, "wildmat%*[ \t]%128[^ \t]%*[ \t]%1024s", wmname, wmval) != 2 ) 
		die("Wildmat config syntax error on line %d of %s", linenr, acfile);

	wm = getwildmatptr(wmname);
	if ( wm == NULL )
	{
		strcpy( (master->wildmats+master->numwildmats)->name, wmname);
		strcpy( (master->wildmats+master->numwildmats)->pattern, wmval);

		master->numwildmats++;
		if ( master->numwildmats > MAX_WILDMATS ) 
			die("Too many Wildmat entries (%d) increase MAX_WILDMATS in nntpd.h", master->numwildmats);
	} else {
		strcpy( wm->name, wmname);
		strcpy( wm->pattern, wmval);
	}

}


PROTO void parsecfg_alias(char *buf, FILE *f) 
{
	char name[MAX_STRING], lib[MAX_STRING], args[MAX_STRING];
	name[0] = 0;
	lib[0] = 0;
	args[0] = 0;
	ALIAS * ali;

	if ( sscanf(buf, "alias%*[ \t]%256[^ \t]%*[ \t]%256[^ \t]%*[ \t]%256[^\r\n]", name, lib, args) < 2 ) 
		die("Alias config syntax error on line %d of %s", linenr, acfile);

	if ( (ali = getalias(name)) == NULL )
	{
		strcpy( (master->aliases+master->numaliases)->name, name);
		strcpy( (master->aliases+master->numaliases)->lib,  lib);
		strcpy( (master->aliases+master->numaliases)->args, args);
 
		master->numaliases++;
		if ( master->numaliases > MAX_ALIASES ) 
			die("Too many aliases (%d) increase MAX_ALIASES in nntpd.h", master->numaliases);
	} else {
		strcpy( ali->name, name);
		strcpy( ali->lib,  lib);
		strcpy( ali->args, args);
	}
}


PROTO void parsecfg_args(char *buf, FILE *f) 
{
	char name[MAX_STRING], args[MAX_STRING];
	name[0] = 0;
	args[0] = 0;
	ARGS * arg;

	if ( sscanf(buf, "args%*[ \t]%256[^ \t]%*[ \t]%256[^\r\n]", name, args) < 2 ) 
		die("Alias config syntax error on line %d of %s", linenr, acfile);

	if ( (arg = get_args(name)) == NULL )
	{
		strcpy( (master->args+master->numargs)->name, name);
		strcpy( (master->args+master->numargs)->args, args);
 
		master->numargs++;
		if ( master->numargs > MAX_ARGS ) 
			die("Too many args (%d) increase MAX_ARGS in nntpd.h", master->numargs);
	} else {
		strcpy( arg->name, name);
		strcpy( arg->args, args);
	}
}


PROTO int parsecfg_acl(char *buf, FILE *file) 
{
	char ip[16], pfx[3], acs[64], profile[MAX_NAME];
	struct in_addr inaddr;
	ulong inaddrt;
	ulong ipaddrs;
	char *p;
	ACL *acl;
	int new = 0;

	/*
	* Read and store ip ranges for access control
	*/
	profile[0] = 0;
	if ( sscanf(buf, "acl%*[ \t]%15[0-9.]%*[/]%2[0-9] %64s%*[ \t]%32s", ip, pfx, acs, profile ) < 3 )
		die("Wrong ACL entry \"%s\" on line %d of %s", buf, linenr, acfile);

	inet_aton(ip, &inaddr);

	inaddrt = ntohl(inaddr.s_addr);
	if ( atol(pfx) == 0 )
		ipaddrs = 0xFFFFFFFF;
	else
		ipaddrs = (1 << (32-atol(pfx)))-1;

	if ( ! nullstr(profile) && getprofile(profile) == NULL ) 
		die("Undefined profile on line %d of %s", linenr, acfile);

	if ( (acl = getacl_real(inaddrt, ipaddrs)) == NULL ) 
	{
		acl = master->acls+master->numacls;
		new++;
	}

	acl->profile = nullstr(profile) ? getprofile("default") : getprofile(profile);
	acl->read = 0;
	acl->stats = 0;
	acl->post = 0;
	acl->apost = 0;
	acl->deny = 0;
	acl->auth = 0;
	acl->unlimit = 0;

	p = strtok(acs,",");
	do 
	{
		if ( !strcasecmp(p,"read") ) 
			acl->read = 1;
		else 
		if ( ! strcasecmp(p,"stats") ) 
			acl->stats = 1;
		else 
		if ( ! strcasecmp(p,"post") ) 
			acl->post = 1;
		else 
		if ( ! strcasecmp(p,"apost") ) 
			acl->apost = 1;
		else 
		if ( ! strcasecmp(p,"deny") )
			acl->deny = 1;
		else 
		if ( ! strcasecmp(p,"auth") )
			acl->auth = 1;
		else 
		if ( ! strcasecmp(p,"unlimit") )
			acl->unlimit = 1;
		else 
			die("ACL Syntax Error \"%s\" on line %d of %s", p, linenr, acfile);
	}
	while( (p=strtok(NULL,",")) );

	acl->start = inaddrt;
	acl->end   = inaddrt + ipaddrs;
	acl->size  = ipaddrs;

	if ( new )
		master->numacls++;
	if ( master->numacls > MAX_ACLS ) 
		die("Too many access entries (%d) increase MAX_ACLS in nntpd.h");

	return 0;
}


PROTO void vhost_copycfg(char *buf, FILE *f, VHOST *vh) 
{
	char key[256], val[256];
	int i=0;

	while( i == 0 && fgets(buf, MAX_CFGLINE, f) ) 
	{
		chop(buf);
		linenr++;

		if ( buf[0] == ';' || buf[0] == '#' || buf[0] == 0 )
			continue;

		if ( strncasecmp(buf, "end", 3) == 0 ) 
		{
			i++;
			break;
		}

		key[0] = 0;
		val[0] = 0;

		if ( sscanf(buf, "%*[ \t]%64s%*[ \t]%192[^\r\n]%*[\t\r\n]", key, val) != 2 )
			die("Error on line %d of %s (%s - %s)", linenr, acfile, key, val);

		if ( strcasecmp(key, "Banner") == 0 ) 
			strcpy(vh->Banner, val);
		else 
		if ( strcasecmp(key, "ReplaceBanner") == 0 ) 
			vh->ReplaceBanner = atoi(val);
		else 
		if ( strcasecmp(key, "AddSuffix") == 0 ) 
			strcpy(vh->AddSuffix, val);
		else 
		if ( strcasecmp(key, "SSLKeyfile") == 0 ) 
			strcpy(vh->SSLKeyFile, val);
		else 
		if ( strcasecmp(key, "SSLCAList") == 0 ) 
			strcpy(vh->SSLCAList, val);
		else 
		if ( strcasecmp(key, "SSLPassword") == 0 ) 
			strcpy(vh->SSLPassword, val);
		else 
			die("Unknown vhost option: %s %s", key, val);
	}
        if ( cfg.EnableSSL ) 
                vh->ssl_ctx = ssl_init_ctx(vh->SSLKeyFile, vh->SSLPassword, vh->SSLCAList);
        else
                vh->ssl_ctx = NULL;
}

PROTO void parsecfg_vhost(char *buf, FILE *f) 
{
	char key[256], val[256];
	VHOST *defp;
	key[0] = 0;
	val[0] = 0;

	if ( (sscanf(buf, "%s %s", key, val)) != 2 )
		die("Vhost Syntax error on line %d of %s", linenr, acfile);

	/* if we exist already, just overwrite us */
	if ( (defp = getvhost(val)) != NULL ) 
	{
		vhost_copycfg(buf, f, defp);
		return;
	}

	/* nope didnt exist, create new vhost */
	if ( strcmp(val, "default") != 0 ) 
	{
		if ( (defp = getvhost("default")) == NULL ) 
			die("Could not retrieve default vhost, the default vhost must be listed first.");

		/* copy from default profile */
		memcpy(master->vhosts+master->numvhosts, defp, sizeof(VHOST));
	} else {
		/* initialize default profile */
		bzero(master->vhosts+master->numvhosts, sizeof(VHOST));
	}

	strcpy((master->vhosts+master->numvhosts)->ipaddress, val);
	vhost_copycfg(buf, f, master->vhosts+master->numvhosts);

	master->numvhosts++;
	if ( master->numvhosts > MAX_VHOSTS ) 
		die("Too many vhosts (%d) increase MAX_VHOSTS in nntpd.h", master->numvhosts);
}


/*
 * Reloading Functions
 */

PROTO void reload_access_conf(void) 
{
	load_access_conf();
}


/*
 * ACL Functions
 */

PROTO ACL* getacl(ulong ipaddr) 
{
	int i;

	for (i=0; i<master->numacls; i++) 
		if ( ipaddr >= (master->acls+i)->start && ipaddr <= (master->acls+i)->end )
			return master->acls+i;

	return NULL;
}


PROTO ACL* getacl_real(ulong ipaddr, ulong size) 
{
	int i;

	for (i=0; i<master->numacls; i++) 
		if ( ipaddr == (master->acls+i)->start && size == (master->acls+i)->size )
			return master->acls+i;

	return NULL;
}


/*
 * Wildmat Functions
 */

PROTO const char* getwildmat(const char* name) 
{
	int i;

	if ( nullstr(name) ) 
		return DEFAULT_WILDMAT;

	for (i=0; i<master->numwildmats; i++)
		if ( ! strcmp( (master->wildmats+i)->name, name) )
			return (master->wildmats+i)->pattern;

	return NULL;
}

PROTO WILDMAT * getwildmatptr(const char* name) 
{
	int i;

	for (i=0; i<master->numwildmats; i++)
		if ( ! strcmp( (master->wildmats+i)->name, name) )
			return master->wildmats+i;

	return NULL;
}


/*
 * Alias Functions
 */

PROTO ALIAS* getalias(const char *name) 
{
	int i;

	for (i=0; i<master->numaliases; i++)
		if ( strcmp((master->aliases+i)->name, name) == 0 )
			return master->aliases+i;

	return NULL;
}


/*
 * Args Functions
 */

PROTO ARGS * get_args(const char *name) 
{
	int i;

	for (i=0; i<master->numargs; i++)
		if ( strcmp((master->args+i)->name, name) == 0 )
			return master->args+i;

	return NULL;
}


/*
 * Profiles Functions
 */

PROTO PROFILE* getprofile(const char *name) 
{
	int i;

	for (i=0; i<master->numprofiles; i++)
		if ( strcmp((master->profiles+i)->Name, name) == 0 )
			return master->profiles+i;

	return NULL;
}


/*
 * Auth Functions
 */

PROTO AUTH* getauth(const char *username) 
{
	int i;

	for (i=0; i<master->numauths; i++)
		if ( uwildmat_simple(username, (master->auths+i)->mask) )
			return master->auths+i;

	return NULL;
}

PROTO AUTH* getauthmask(const char *mask) 
{
	int i;

	for (i=0; i<master->numauths; i++)
		if ( strcmp( (master->auths+i)->mask, mask ) == 0 )
			return master->auths+i;

	return NULL;
}

/*
 * VHost Functions
 */

PROTO VHOST* getvhost(const char * ipaddress) 
{
	int i;

	for (i=0; i<master->numvhosts; i++)
		if ( strcmp((master->vhosts+i)->ipaddress, ipaddress) == 0 )
			return master->vhosts+i;

	return NULL;
}


/*
 * Auth Functions
 */


