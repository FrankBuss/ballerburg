/* Soundausgabe */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include "bb.h"

void play(char *name)
{
        static pid_t id=-1;
        struct stat stat_buf;
	int statloc;
#ifdef DEBUG
	fprintf(stderr,"play: id=%d\n",id);
#endif
	switch (SoundMode) {
	    case 0:
	        break;  /* Kein Sound */
	    case 1:
                XBell(display,100);
		break;
	    case 2:
                if (lstat(name,&stat_buf)<0) return;
                if (S_ISREG(stat_buf.st_mode)) {
	                if (id>0) {
		                kill(id,SIGTERM);
			        wait(&statloc);
		        }
	                if ((id=fork())<0) return; else {
	                        if (id==0) {
		                        /* /dev/audio gibt es nicht mehr */
		                        execlp("paplay","paplay",name,NULL);
		                        execlp("pw-play","pw-play",name,NULL);
		                        if (execlp("aplay","aplay","-q",name,NULL)<0) {
                                                #ifdef DEBUG
			                        fprintf(stderr,"play: execlp error: %d");
				                perror(NULL);
                                                #endif
			                        exit(0);
					}
				}
			}
		}
		break;
	}
}
