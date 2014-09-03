#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#if USE_unistd
#include <unistd.h>
#endif

#include <time.h>

#include "structs.h"

#include "comm.h"
#include "fight.h"
#include "stats.h"
#include "util_str.h"
#include "interpreter.h"
#include "handler.h"

#include <sys/time.h>

void stat_collector(int signo);

static int collector_running = 0;

void do_stats(struct char_data* ch, char* args, int cmd)
{
    long		uptime;
    char		buf[256];
    
    args = one_argument(args, buf);

    if(!buf[0])
    {
	uptime = time(0);
	uptime -= Uptime;
    
	sprintf(buf, "Time up:    %ld:%02ld\n\r",
		uptime / 60 / 60, uptime /60 % 60);
	send_to_char(buf, ch);

	sprintf(buf, "Input:      %-8ld  %ld/second\n\r",
		bytes_read, bytes_read / uptime);
	send_to_char(buf, ch);
    
	sprintf(buf, "Output:     %-8ld  %ld/second\n\r",
		bytes_written, bytes_written / uptime);
	send_to_char(buf, ch);

	sprintf(buf, "Connected:  %d\n\r",
		connected);
	send_to_char(buf, ch);
	
	sprintf(buf, "Combatants: %d\n\r",
		combatants);
	send_to_char(buf, ch);

	sprintf(buf, "Collector:  %s\n\r",
		collector_running ? "running" : "stopped");
	send_to_char(buf, ch);
    }
    else if(is_abbrev(buf, "stop"))
    {
	if(!collector_running)
	{
	    send_to_char("Collector is already stopped\n\r", ch);
	    return;
	}
	
	stop_stats();
    }
    else if(is_abbrev(buf, "start"))
    {
	int interval;
	
	if(collector_running)
	{
	    send_to_char("Collector is already running.\n\r", ch);
	    return;
	}
	
	args = one_argument(args, buf);
	interval = buf[0] ? atoi(buf) : 300;

	start_stats(interval);
    }
    else if(is_abbrev(buf, "reset"))
    {
	if(collector_running)
	{
	    send_to_char("You must stop the collector first.\n\r", ch);
	    return;
	}
	unlink("stats");
    }
    else 
	send_to_char("Stats [stop|start [freq]|reset]\n\r", ch);
}   


void stat_collector(int signo)
{
    static long		last_time;
    static long		last_read, last_written;
    static FILE*	stat_file;
    
    if(signo == 0)
    {
	if((stat_file = fopen("stats", "a+")) == NULL)
	    perror("stats");
	last_read = bytes_read;
	last_written = bytes_written;
	last_time = time(0);
    }
    else if(signo == -1)
    {
	if(stat_file)
	    fclose(stat_file);
    }
    else if(stat_file)
    {
	fprintf(stat_file, "%9ld %9ld %8ld %8ld %4d %4d\n",
		time(0), time(0) - last_time,
		bytes_read - last_read, bytes_written - last_written,
		connected, combatants);
	fflush(stat_file);
	last_time = time(0);
	last_read = bytes_read;
	last_written = bytes_written;
    }
}

void stop_stats(void)
{
    struct itimerval timer;

    timerclear(&timer.it_value);
    timerclear(&timer.it_interval);

    stat_collector(-1);
    collector_running = 0;
	
    setitimer(ITIMER_REAL, &timer, NULL);
}

void start_stats(int interval)
{
    struct itimerval timer;

    timer.it_value.tv_sec = interval;
    if(!timer.it_value.tv_sec)
	timer.it_value.tv_sec = 5 * 60;
    timer.it_value.tv_usec = 0;
    timer.it_interval = timer.it_value;

    setitimer(ITIMER_REAL, &timer, NULL);

#ifndef LINUX
    signal(SIGALRM, stat_collector);
#endif

    stat_collector(0);
    collector_running = 1;
}



