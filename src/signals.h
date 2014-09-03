#ifndef SIGNALS_H
#define SIGNALS_H

typedef void Sigfunc(int);

void signal_setup(void);

void dump_mud(int signo);

#endif
