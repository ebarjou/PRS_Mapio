#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
	struct timeval tv;

	gettimeofday (&tv ,NULL);

	// Only count seconds since beginning of 2016 (not jan 1st, 1970)
	tv.tv_sec -= 3600UL * 24 * 365 * 46;
	
	return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

void *catch_alrms(void *p) {
	sigset_t masque;
	while (1) {
		sigfillset(&masque);
		sigdelset(&masque, SIGALRM);
		sigsuspend(&masque)
	}
	return NULL;
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
	pthread_t pid;
	pthread_create(&pid, NULL, catch_alrms, NULL);
	// TODO Implémenter
	return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
	// TODO
}

#endif
