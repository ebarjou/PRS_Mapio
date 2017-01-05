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

// Return number of elapsed �sec since... a long time ago
static unsigned long get_time (void)
{
	struct timeval tv;

	gettimeofday (&tv ,NULL);

	// Only count seconds since beginning of 2016 (not jan 1st, 1970)
	tv.tv_sec -= 3600UL * 24 * 365 * 46;
	
	return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

typedef struct evenement {
	void *param;
	unsigned t;
	
} event;
int current_event;

event** next_events;
int event_size;

pthread_t daemon;

int set_current_event()
{
	event* closest = NULL;
	for (int i = 0; i < event_size; ++i){
		if (next_events[i] != NULL && (closest == NULL || closest->t > next_events[i]->t)) {
			current_event = i;
			closest = next_events[i];
		}
	}
	if (closest != NULL){
		unsigned t = closest->t - get_time();
		if (t <= 0) t = 0;
		struct itimerval timer;
		timer.it_value.tv_sec = (int)((double)t/1000000.0);
		timer.it_value.tv_usec = t%1000000;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
			perror("Erreur à l'appel de setitimer()");
			exit(1);
		}
	}
	return 1;
}

void traitant_event(int sig){
	sdl_push_event(next_events[current_event]->param);
	free(next_events[current_event]);
	next_events[current_event] = NULL;
	set_current_event();
}

void *catch_alrms(void *p) {
	sigset_t masque;
	sigfillset(&masque);
	sigdelset(&masque, SIGALRM);
	struct sigaction s;
	s.sa_handler = traitant_event;
	sigemptyset(&s.sa_mask);
	s.sa_flags=0;
	sigaction(SIGALRM, &s, NULL);
	sigsuspend(&masque);
	return NULL;
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
	pthread_create(&daemon, NULL, catch_alrms, NULL);
	event_size = 10;
	next_events = malloc(sizeof(event*)*event_size);
	for (int i = 0; i < event_size; ++i) next_events[i] = NULL;
	return 1;
}

// Add a timer to the current list
int add_timer(event *e)
{
	for (int i = 0; i < event_size; ++i){
		if (next_events[i] == NULL) {
			next_events[i] = e;
			return 1;
		}
	}
	event **rlc_events = malloc(sizeof(event*)*event_size*2);
	for (int i = 0; i < event_size*2; ++i) rlc_events[i] = NULL;
	for (int i = 0; i < event_size; ++i) rlc_events[i] = next_events[i];
	event_size *= 2;
	free(next_events);
	next_events = rlc_events;
	add_timer(e);
	return 1;
}

void timer_set (Uint32 delay, void *param)
{
	event *new_event = malloc(sizeof(event));
	new_event->param = param;
	new_event->t = get_time()+delay*1000;
	add_timer(new_event);

	set_current_event();
}

#endif
