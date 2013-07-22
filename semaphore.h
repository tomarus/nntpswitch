#ifndef NNTPSWITCHD_SEMAPHORE_H
#define NNTPSWITCHD_SEMAPHORE_H
/* $Id: semaphore.h,v 1.1 2006-09-18 12:28:27 mjo Exp $ */

#include <sys/sem.h>

int semlock_init (key_t SEMKEY);
void semlock_lock (int semid);
void semlock_unlock (int semid);
void semlock_close (int semid);

#endif

