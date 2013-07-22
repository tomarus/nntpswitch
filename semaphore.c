/*
 * NNTPSwitch SYSV Semaphore functions
 *
 * No errorhandling on lock/unlock and close functions. If for some reason
 * the semaphore has gone we won't care, because the master process
 * has lost it counters anyway.
 *
 * $Id: semaphore.c,v 1.6 2010-12-21 09:41:43 tommy Exp $
 */

#include <syslog.h>
#include <sys/errno.h>
#include "semaphore.h"
#include "nntpd.h"

int semlock_init (key_t SEMKEY)
{
	return semget (SEMKEY, 1, IPC_CREAT | 0666);
}

void semlock_lock(int semid)
{
	struct sembuf sops[2];

	sops[0].sem_num = 0;			/* We only use one track */
	sops[0].sem_op = 0;			/* wait for semaphore flag to become zero */
	sops[0].sem_flg = SEM_UNDO;		/* take off semaphore asynchronous  */

	sops[1].sem_num = 0;
	sops[1].sem_op = 1;			/* increment semaphore -- take control of track */
	sops[1].sem_flg = SEM_UNDO;		/* take off semaphore */

	int r;
	do {
		r = semop(semid, sops, 2);
	} while ( r < 0 && errno == EINTR );
        master->nrlocks++;
}
       		
void semlock_unlock(int semid)
{
	struct sembuf sops[1];

	sops[0].sem_num = 0;
	sops[0].sem_op = -1;			/* Give UP COntrol of track */
	sops[0].sem_flg = SEM_UNDO;		/* take off semaphore, asynchronous  */

	int r;
	do {
		r = semop(semid, sops, 1);
	} while ( r < 0 && errno == EINTR );
        master->nrunlocks++;
}

void semlock_close(int semid)
{
	semctl(semid, 0, IPC_RMID);
}

