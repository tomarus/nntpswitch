#include <sys/types.h>
#include <sys/mman.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

#ifdef MAP_ANON
#define MAPFLAGS_ANON_SHARED MAP_ANON | MAP_SHARED
#else
#define MAPFLAGS_ANON_SHARED MAP_SHARED
#endif

void *memmap(int);
int memunmap(void *, int);

