#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HP_UX

static void alarmhandler()
{
}

sleepms(int msec)
{
        struct itimerval value,ovalue;
        struct sigvec vec;
	long savemask, sigblock(), sigpause();
	
	vec.sv_handler = alarmhandler;
	vec.sv_mask = 0x0;
	vec.sv_flags = 0;
	sigvector(SIGALRM, &vec, &vec); /* Set up alarmhandler for SIGALRM */
	savemask = sigblock((long)(1L << (SIGALRM - 1)));
	
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_sec = msec/1000;
	value.it_value.tv_usec = (msec%1000)*1000;
	setitimer(ITIMER_REAL,&value,&ovalue);
	
	(void)sigpause(0L);
	(void)sigsetmask(savemask);
	
	sigvector(SIGALRM, &vec, NULL); /* Restore previous signal handler */
}

#else

void sleepms(int msec)
{
  /*        static struct timeval tv;
	fprintf(stderr,"sleepms: %d\n",msec);
	tv.tv_sec = 0;
	tv.tv_usec = msec*1000;
	select(0,NULL,NULL,NULL, &tv);*/
usleep((long)msec*1000l);
}

#endif

