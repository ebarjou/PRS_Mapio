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

struct evenement event{
	void *param;
}
event* current_event;

pthread_t daemon;

void traitant_event(int sig){
	// TODO 4.3 + vérifier que 4.1 et 4.2 sont fonctionnels
	if (pthread_self() != daemon) return;
	
	printf("Je suis le thread n°%d\n", daemon);
	printf ("sdl_push_event(%p) appelée au temps %ld\n",
		current_event->param, get_time ());
}

void *catch_alrms(void *p) {
	sigset_t masque;
	sigfillset(&masque);
	sigdelset(&masque, SIGALRM);
	struct sigaction s;
	s.sa_handler = traitant_event;
	sigemptyset(&s.sa_mask);
	s.sa_flags=0;
	while (1) {
		sigaction(SIGALRM, &s, NULL);
		sigsuspend(&masque);
	}
	return NULL;
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
	// Vérifier que 4.1 est fonctionnel
	pthread_create(&daemon, NULL, catch_alrms, NULL);
	return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
	// TODO 4.3 + vérifier que 4.2 est fonctionnel
	event *new_event = malloc(sizeof(event));
	new_event->param = param;
	current_event = new_event;
	
	struct itimerval timer;
	timer.it_value.tv_sec = delay/1000;
	timer.it_value.tv_usec = (delay*1000) % 1000000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
		perror("Erreur à l'appel de setitimer()");
		exit(1);
	}
}

#endif
