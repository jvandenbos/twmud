
#include "config.h"
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#if USE_stdlib
#include <stdlib.h>
#endif
    
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "utility.h"
#include "signals.h"
    
void shutdown_request(int signo);
void hupsig(int signo);
void logsig(int signo);
void panic_save(int signo);
void checkpointing(int signo);


Sigfunc *signal_intr(int signo, Sigfunc * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}

void signal_setup(void)
{
    struct itimerval itime;
    struct timeval interval;

    signal_intr(SIGUSR2, shutdown_request);

    /* just to be on the safe side: */

    signal(SIGPIPE, SIG_IGN);

    signal_intr(SIGHUP, hupsig);
    signal_intr(SIGINT, hupsig);
    signal_intr(SIGALRM, logsig);
    signal_intr(SIGTERM, hupsig);

    /* borrowed from Sloth */

    /* use signal instead of signal_intr to prevent recursion */
    signal(SIGABRT,  panic_save);
    signal(SIGBUS,   panic_save);
    signal(SIGSEGV,  panic_save);
    signal(SIGFPE,   panic_save); 

    /* Ignore the following signals */
    signal(SIGPIPE, SIG_IGN);

    /* set up the deadlock-protection */

    interval.tv_sec = 900;    /* 15 minutes */
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, 0);
    signal_intr(SIGVTALRM, checkpointing);
}



void checkpointing(int signo)
{
	if (!tics)
	{
		log_msg("CHECKPOINT shutdown: tics not updated");
		abort();
	}
	else
		tics = 0;

	/*
	 * reseting handler seems necessary on linux to prevent
	 * program termination on second signal received. - EJG 4/96
	 */
	signal(SIGVTALRM, checkpointing);
}




void shutdown_request(int signo)
{
	log_msg("Received USR2 - shutdown request");
	goaway = 1;
}



/* kick out players etc */
void hupsig(int signo)
{
	log_msg("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");

	goaway = 1;
}



void logsig(int signo)
{
	log_msg("Signal received. Ignoring.");
}


void dump_mud(int signo)
{
    log_msg("Dumping core");
    signal(SIGABRT, SIG_DFL); /* so we don't recurse */
    abort();
}
