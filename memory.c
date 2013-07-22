#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

#ifdef MAP_ANON
#define MAPFLAGS_ANON_SHARED MAP_ANON | MAP_SHARED
#else
#define MAPFLAGS_ANON_SHARED MAP_SHARED
#endif

void *memmap(int size)
{
    caddr_t ptr;
    int fd = -1;

#ifndef MAP_ANON
    fd = open("/dev/zero", O_RDWR);
    if (fd < 0)
        return NULL;
#endif

    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                           MAPFLAGS_ANON_SHARED,
                           fd, 0);

#ifndef MAP_ANON
    close(fd);
#endif

    if (ptr == MAP_FAILED)
        return NULL;

    return ptr;
}

int memunmap(void *ptr, int size) {
    return munmap(ptr,size);
}

