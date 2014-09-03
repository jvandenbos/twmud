#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <ctype.h>
#if USE_unistd
#include <unistd.h>
#endif
#include <memory.h>
#include <signal.h>
#include <strings.h>
#if USE_time
#include <time.h>
#endif

#include "structs.h"
#include "proto.h"
#include "comm.h"
#include "interpreter.h"
#include "utils.h"
#include "db.h"
#include "state.h"
#include "utility.h"
#include "signals.h"
#include "whod.h"
#include "page.h"
#include "events.h"
#include "modify.h"
#include "editor.h"
#include "fight.h"
#include "weather.h"
#include "spell_util.h"
#include "modify.h"
#include "statistic.h"
#include "ident.h"
#include "periodic.h"
#include "mobprog2.h"

char last_known_state[MAX_STRING_LENGTH];

static void game_loop(int s);
static void run_the_game(int port);
struct timeval timediff(struct timeval *a, struct timeval *b);
struct timeval timeadd(struct timeval *a, struct timeval *b);

#ifdef __cplusplus
extern "C"
{
#endif
/* some unix stuff we shouldn't have to declare, but doesn't seem
to be anywhere I can find */
int select(int width, fd_set* readfds, fd_set* writefds,
	   fd_set* exceptfds, struct timeval* timeout);

#ifdef __cplusplus
};
#endif

/* local globals */
extern int maxdesc, avail_descs, real_avail_descs;

int lawful = 0;		/* work like the game regulator */
int port;

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

#if !defined(AUX) && !defined(SPARC)
int __main ()
{
  return(1);
}
#endif

#define OPT_USEC (1000000 / PULSE_PER_REAL_SEC)

int main (int argc, char **argv)
{
  int pos=1;
  char buf[512], *dir;
  struct rlimit rl;
  int res;

#if USE_profile
  moncontrol(0);
#endif

  port = DFLT_PORT;
  dir = DFLT_DIR;


   /* init the random() number generator */
   srandom( (unsigned int) time(NULL) );
   init_mm( ); /* init the xrand generator */

/*
 *  this block sets the max # of connections.
 *  i had to make it optional, because RLIMIT_NOFILE is not defined on my
 *  test-site (sigh)
 */

#ifdef RLIMIT_NOFILE
   res = getrlimit(RLIMIT_NOFILE, &rl);

   sprintf(buf, "Setting file-limit from %ld to %d (%ld).",
	   rl.rlim_cur, MAX_OPEN_FILES, rl.rlim_max);
   log_msg(buf);

   rl.rlim_cur = MAX_OPEN_FILES;
   res = setrlimit(RLIMIT_NOFILE, &rl);
#endif

  WizLock = FALSE;
  CHAOS = FALSE;
  PKILLABLE = 1;
  GROUP_RES = 2;
  AREA_FX_DEADLY = 1;
  LOAD_PLAYER_FILES = FALSE;

  open_statlog("stat.log");

  while ((pos < argc) && (*(argv[pos]) == '-'))	{
    switch (*(argv[pos] + 1))  {
    case 'l':
      lawful = 1;
      log_msg("Lawful mode selected.");
      break;
    case 'w':
      WizLock = TRUE;
      log_msg("WizLock engaged");
      break;
    case 'x':
      CHAOS = TRUE;
      log_msg("CHAOS engaged");
      break;
    case 'd':

      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else   	{
	log_msg("Directory arg expected after option -d.");
	exit(0);
      }
      break;
    case 'a':
      log_msg("Player authorization turned off.");
      AUTH_PLAYER=0;
      break;
    case 's':
      no_specials = 1;
      log_msg("Suppressing assignment of special routines.");
      break;
    case 'p':
       LOAD_PLAYER_FILES=1;
      sprintf(buf, "Loading pfiles and generating logs.\n");
      log_msg(buf);
      break;
    default:
      sprintf(buf, "Unknown option -% in argument string.",
	      *(argv[pos] + 1));
      log_msg(buf);
      break;
    }
    pos++;
  }

  if (pos < argc)
    if (!isdigit(*argv[pos]))      	{
      fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [-w] [-x] [ port # ]\n",
	      argv[0]);
      exit(0);
    }  else if ((port = atoi(argv[pos])) <= 1024)  {
      printf("Illegal port #\n");
      exit(0);
    }

  Uptime = time(0);

  sprintf(buf, "Running game on port %d.", port);
  log_msg(buf);

  if (chdir(dir) < 0)	{
    perror(dir);
    exit(0);
  }

  sprintf(buf, "Using %s as data directory.", dir);
  log_msg(buf);

#ifdef HAVE_srand
  srand(time(0));
#else
  srandom(time(0));
#endif

  ident_init();

  run_the_game(port);
  return(0);
}



#define PROFILE(x)


/* Init sockets, run game, and cleanup sockets */
void run_the_game(int port)
{
  int s;

  list_init(&descriptor_list, 0);

  log_msg("Signal trapping.");
  signal_setup();
  log_msg("Opening mother connection.");
  init_whod(port);
  s = init_socket(port);

  init_pages();

  boot_db();

  log_msg("Entering game loop.");

  {
    extern int end;
    char buf[256];
    sprintf(buf, "Final sbrk = %p", sbrk(0));
    log_msg(buf);
  }

  game_loop(s);
  close_sockets(s);
  close_whod();

  PROFILE(monitor(0);)

  if (lc_reboot)  {
    log_msg("Rebooting.");
    exit(52);           /* what's so great about HHGTTG, anyhow? */
  }

  log_msg("Normal termination of game.");
}


/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void game_loop(int s)
{
    fd_set input_set, output_set, exc_set;
    struct timeval next_time, now, timeout, null_time, opt_time;
    char comm[MAX_INPUT_LENGTH];
    struct descriptor_data *point;
    int mask;
    void (*f)(struct char_data *, char *);
    struct char_data *mob;

    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    opt_time.tv_usec = OPT_USEC; /* Init time values */
    opt_time.tv_sec = 0;
    gettimeofday(&next_time, (struct timezone *) 0);
    next_time = timeadd(&next_time, &opt_time);

    maxdesc = s;
    /* !! Change if more needed !! */
    real_avail_descs = getdtablesize() -2; /* I think this is now a
					      dummy var BH */
    avail_descs = 127;

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
      sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
      sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

    /* Main loop */
    while (!goaway)  {
      STATE0("top of event loop");

      /* Check what's happening out there */
      FD_ZERO(&input_set);
      FD_ZERO(&output_set);
      FD_ZERO(&exc_set);
      FD_SET(s, &input_set);
      EACH_DESCRIPTOR(d_iter, point)
	{
	  FD_SET(point->descriptor, &input_set);
	  FD_SET(point->descriptor, &exc_set);
	  FD_SET(point->descriptor, &output_set);
	}
      END_ITER(d_iter);

      /* check out the time */
      gettimeofday(&now, (struct timezone *) 0);
      timeout = timediff(&next_time, &now);
      next_time = timeadd(&next_time, &opt_time);

      sigsetmask(mask);

      /* Daemon for WHO calls */
      STATE0("whod_loop");
      whod_loop();

      STATE0("select poll");
      if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time)
	  < 0)   	{
	perror("Select poll");
	return;
      }

      STATE0("select select");
      if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
	perror("Select sleep");
	/*exit(1);*/
      }

      sigsetmask(0);

      /* Respond to whatever might be happening */

      /* New connection? */
      STATE0("checking new connection");
      if (FD_ISSET(s, &input_set))
	if (new_descriptor(s) < 0)
	  perror("New connection");

      /* kick out the freaky folks */
      STATE0("checking link dead");
      EACH_DESCRIPTOR(d_iter, point)
	{
	  if (FD_ISSET(point->descriptor, &exc_set))  {
	    STATE1("dumping descriptor: %s", STATE_NAME(point));
	    FD_CLR(point->descriptor, &input_set);
	    FD_CLR(point->descriptor, &output_set);
	    close_socket(point, TRUE);
	  }
	}
      END_ITER(d_iter);

      STATE0("checking ident progress");
      EACH_DESCRIPTOR(d_iter, point)
	{
	  STATE1("check ident progress from %s", STATE_NAME(point));

	  if (waiting_for_ident(point))
	    ident_check(point);
	}
      END_ITER(d_iter);

      STATE0("checking input");
      EACH_DESCRIPTOR(d_iter, point)
	{
	  if (FD_ISSET(point->descriptor, &input_set))
	    {
	      STATE1("getting input from: %s", STATE_NAME(point));
	      if (process_input(point) < 0)
		close_socket(point, TRUE);
	    }
	}
      END_ITER(d_iter);

      /* process_commands; */
      STATE0("processing commands");
      EACH_DESCRIPTOR(d_iter, point)
	{
	  STATE1("check line from %s", STATE_NAME(point));

	  if (!waiting_for_ident(point) && (--(point->wait) <= 0) &&
	      get_from_q(&point->input, comm, sizeof(comm)))
	    {
	      if (point->character && point->connected == CON_PLYNG &&
		  point->character->specials.was_in_room != NOWHERE)
		{

		  point->character->specials.was_in_room = NOWHERE;
		  act("$n has returned.", TRUE, point->character, 0, 0, TO_ROOM);
		}

	      point->wait = 1;
	      if (point->character)
		point->character->specials.timer = 0;
	      point->prompt_mode = 1;

	      if (point->str || point->sstr)
		{
		  STATE2("add input: %s: \"%s\"", STATE_NAME(point), comm);
		  string_add(point, comm);
		}
	      else if (!point->connected)
		{
		  if (point->showstr_point)
		    {
		      STATE2("show_string: %s: %s", STATE_NAME(point), comm);
		      show_string(point, comm);
		    }
		  else if((f = point->input_fun))
		    {
		      STATE2("input func: %s: %s", STATE_NAME(point), comm);
		      point->input_fun = NULL;
		      f(point->character, comm);
		    }
		  else
		    {
		      STATE2("do command: %s: %s", STATE_NAME(point), comm);
		      command_interpreter(point->character, comm, 1);
		    }
		}
	      else
		{
		  STATE3("nanny(%d): %s: %s", point->connected,
			 STATE_NAME(point), comm);
		  nanny(point, comm);
		}
	    }
	}
      END_ITER(d_iter);

      /* either they are out of the game */
      /* or they want a prompt.          */

      STATE0("process output");
      EACH_DESCRIPTOR(d_iter, point)
	{
	  if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
	    if (process_output(point) < 0)
	      close_socket(point, TRUE);
	    else
	      point->prompt_mode = 1;
	}
      END_ITER(d_iter);

      STATE0("present prompts");
      /* give the people some prompts  */
      EACH_DESCRIPTOR(d_iter, point)
	{
	  if (point->prompt_mode) {
	    if ((point->str || point->sstr) && !point->showstr_point)
	      editor_prompt(point);
	    else if (!point->connected)
	      {
		if (point->showstr_point)
		  write_to_descriptor(point->descriptor,
	      "*** Press return to continue / Any other key to exit ***");
		else
		  player_prompt(point);
	      }
	    point->prompt_mode = 0;
	  }
	}
      END_ITER(d_iter);

      /* handle heartbeat stuff */
      /* Note: pulse now changes every 1/4 sec  */

      if (++pulse >= (PULSE_PER_MUD_HOUR)){
	pulse = 0;

	STATE0("weather_and_time");
	weather_and_time(1);

	STATE0("give_regens");
	give_regens(pulse); /* things have been sped up by combining */

	if ( time_info.hours == 1 )
	  {
	    STATE0("update_time");
	    update_time();
	  }
      }


      /* MOB Delay
       * This is a large inner-loop. It has do be here tho, and is seems
       * to be performing OK.
       * --Mnemosync
       */
      STATE0("mob fight_delay");
      EACH_CHARACTER(iter, mob) {
	if (IS_NPC(mob)) {
	  if (mob->fight_delay > 0) {
	    mob->fight_delay--;
	  } else if (mob->fight_delay < 0) {
	    log_msg("Mob fight_delay is less than 0!");
	  }
	}
      }
      END_AITER(iter);

      STATE0("beware_lightning");
      if (!(pulse % 25))
      {
        beware_lightning();
      }

      STATE0("event_pulse");
      event_process(pulse);

      STATE0("mobile_activity");
      if(!(pulse % PULSE_MOBILE))
	check_mobile_activity(pulse);

      STATE0("zone_update");
      if (!(pulse % PULSE_ZONE))  {
	zone_update();
      }

      STATE0("drain life updates");
      if(!(pulse % PULSE_DRAIN_LIFE)) {
	 drain_life_pulse(pulse);
      }

      STATE0("object & room random triggers");
      if(!(pulse % PULSE_MOBILE)) {
	oprog_random_trigger();
	rprog_random_trigger();
      }

      STATE0("perform_violence");
      if (!(pulse % PULSE_VIOLENCE))
	perform_violence( pulse );

      STATE0("update auction");
      if (!(pulse % PULSE_AUCTION))                /* 20 seconds  */
	auction_update();

      STATE0("update auction");
      if (!(pulse % PULSE_AUCTION))                /* 20 seconds  */
	vote_update();

      tics++;			/* tics since last checkpoint signal */
    }

    STATE0("bottom of loop");
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
  struct timeval rslt;

  rslt = *a;
  rslt.tv_usec -= b->tv_usec;
  while(rslt.tv_usec < 0)
    {
      rslt.tv_usec += 1000000;
      rslt.tv_sec -= 1;
    }

  if((rslt.tv_sec -= b->tv_sec) < 0)
    {
      rslt.tv_sec = 0;
      rslt.tv_usec = 0;
    }

  return rslt;
}


struct timeval timeadd(struct timeval* a, struct timeval* b)
{
  struct timeval rslt = *a;

  rslt.tv_usec += b->tv_usec;
  rslt.tv_sec += b->tv_sec;

  while(rslt.tv_usec > 1000000)
    {
      rslt.tv_usec -= 1000000;
      rslt.tv_sec += 1;
    }

  return rslt;
}

