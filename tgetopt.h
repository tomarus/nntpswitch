#include <stdio.h>

struct option {
	char	*longname;
	int	hasargs;
	char	*args;
	char	shortname;
	int	isset;
};

extern struct option opts[];

char * toptargs(int opt);
int toptset(int opt);
void tgetopt(int argc, char **argv);
void tfreeopt(void);

