/*
 * getopt replacement
 *
 * Tommy van Leeuwen <tommy@chiparus.org>
 * 2004 March, 6th   First Version, written because some OS'es lack getopt
 *
 * Create an array like this:
 *
 * struct option opts[] = {
 *	{ "help", 0, NULL, 'h', 0 },
 *	{ "config", 1, NULL, 'c', 0 },
 *	// Leave this last line 
 *	{ NULL, 0, NULL, 0, 0 },
 * };
 *
 * int main(int argc, char **argc) {
 *	tgetopt(argc, argv);
 *	int   = toptset('h');
 *	char* = toptargs('c');
 *	tfreeopt();  // clean up argument mem
 *      ...
 * }
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tgetopt.h"

char * toptargs(int opt) {
	int j;

	for( j=0; ;j++ ) {
		if ( opts[j].longname == NULL && opts[j].shortname == 0 )
			break;
		if ( opts[j].shortname == opt ) 
			return opts[j].args;
	}
	return NULL;
}

int toptset(int opt) {
	int j;

	for( j=0; ;j++ ) {
		if ( opts[j].longname == NULL && opts[j].shortname == 0 )
			break;
		if ( opts[j].shortname == opt ) 
			return opts[j].isset;
	}
	return 0;
}

void tgetopt(int argc, char **argv) {
	int i,j;
	char c;
	char *p;

	for(i=1; i<argc; i++) {
		if ( argv[i][0] == '-' && argv[i][1] == '-' ) {
			//longopt
			p = argv[i]+2;
			for( j=0; ;j++ ) {
				if ( opts[j].longname == NULL && opts[j].shortname == 0 )
					break;
				if ( strcasecmp(opts[j].longname, p) == 0 ) {
					//foundopt
					if ( opts[j].hasargs == 1 ) {
						if ( argv[i+1] == NULL || argv[i+1][0] == '-' ) {
							fprintf(stderr,"Argument for option %s missing\n", 
									opts[j].longname);
							exit(0);
						}
						opts[j].args = strdup(argv[++i]);
					}
					opts[j].isset++;
				}
			}
		}else if ( argv[i][0] == '-' ) {
			//shortopt
			c = argv[i][1];
			for( j=0; ;j++ ) {
				if ( opts[j].longname == NULL && opts[j].shortname == 0 )
					break;
				if ( opts[j].shortname == c ) {
					//foundopt
					if ( opts[j].hasargs == 1 ) {
						if ( argv[i][2] != 0 ) {
							opts[j].args = strdup(argv[i]+2);
						}else
						if ( argv[i+1] != NULL && argv[i+1][0] != '-' ) {
							opts[j].args = strdup(argv[++i]);
						}else{
							fprintf(stderr,"Argument for option %c missing\n", 
									opts[j].shortname);
							exit(0);
						}
					}
					opts[j].isset++;
				}
			}
		}else {
			fprintf(stderr, "getopt: Skipping unknown option %s\n", argv[i]);
		}
	}
}

void tfreeopt(void) {
	int j;

	for( j=0; ;j++ ) {
		if ( opts[j].longname == NULL && opts[j].shortname == 0 )
			break;
		if ( opts[j].args != NULL )
			free(opts[j].args);
	}
}

