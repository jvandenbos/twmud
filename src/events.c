
#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <time.h>
#include <string.h>

#include "structs.h"
#include "events.h"
#include "utils.h"
#include "comm.h"
#include "utility.h"
#include "util_str.h"

/*This is temporary -- db.a won't compile with this included*/
#include "state.h"

/* global variables */
list_head pulse_events[PULSE_PER_MUD_HOUR];
list_head real_events;

/* forward declarations */
static event_t* queue_event(list_head* head, event_t* theEvent,
			long when, event_func proc, char* tag);
static int time_sorter(event_t* eventA, event_t* eventB);
static int handle_event(event_t* event, long now);

/* external routines */
void event_init(void)
{
    int i;
    
    for(i = 0 ; i < PULSE_PER_MUD_HOUR ; ++i)
	list_init(&pulse_events[i], (list_sort_func) time_sorter);

    list_init(&real_events, (list_sort_func) time_sorter);
}


event_t* event_queue_pulse(event_t* theEvent, long when,
			 event_func proc, char* tag)
{
  while(when < 0) when += PULSE_PER_MUD_HOUR;
  when = when % PULSE_PER_MUD_HOUR;

  if(when == pulse)
    when = next_pulse(-1);
    
  return queue_event(&pulse_events[when], theEvent, when, proc, tag);
}

event_t* event_queue_real(event_t* theEvent, long when,
			  event_func proc, char* tag)
{
    return queue_event(&real_events, theEvent, when, proc, tag);
}

void event_cancel(event_t* theEvent, int do_free)
{
  if(!theEvent)
    return;
  
  if(theEvent->list.list)
    list_delete(theEvent->list.list, &theEvent->list);

  if(do_free)
    event_free(theEvent);
}

void event_free(event_t* theEvent)
{
    if(theEvent->tag)	FREE(theEvent->tag);
    FREE(theEvent);
}

void event_process(int pulse)
{
  char buf[MAX_STRING_LENGTH];
  long rtime = time(0);
  
  sprintf(buf,"event_process pulse %d", pulse);
    
  STATE0(buf);
    list_find(&pulse_events[pulse],
	      (list_find_func) handle_event,
	      (void*) pulse);

  sprintf(buf,"event_process real %ld", rtime);
  STATE0(buf);
    list_find(&real_events,
	      (list_find_func) handle_event,
	      (void*) rtime);
}

void event_display(struct char_data* ch)
{
}

/* internal routines */
static event_t* queue_event(list_head* head, event_t* theEvent,
			  long when, event_func proc, char* tag)
{
    if(!theEvent)
	CREATE(theEvent, event_t, 1);

    theEvent->when = when;
    theEvent->proc = proc;
    if(tag)
	theEvent->tag = strdup(tag);
    
    list_insert(head, &theEvent->list);

    return theEvent;
}

static int time_sorter(event_t* eventA, event_t* eventB)
{
    return eventB->when - eventA->when;
}

static int handle_event(event_t* event, long now)
{
  STATE0("handle_event : testing when");
  if(now < event->when)
    return 0;

  char_data *caster=NULL;
  char_data *holder=NULL;
  char buf[1024];
  affected_type tmp;
/*
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  STATE0("handle_event : generate bufs");
  if(event->tag)
  {
    sprintf(buf,"event_cancel %s : %s", event->tag,last_known_state);
    sprintf(buf2,"event_proc %s : %s",event->tag,last_known_state);
  }
  else {
    strcpy(buf,"unknown");
    strcpy(buf2,"unknown");
  }
*/

/*  STATE0(buf);*/
  STATE0("event_cancel");
  event_cancel(event, 0);

/*  STATE0(buf2);*/
  if (!event) {
    DLOG(("No event at %s %d!", __FILE__, __LINE__));
  }
  STATE1("event_proc: %s", (event->tag)?event->tag:"");
  
  if(event->tag && !strncmp(event->tag,"affect", 6)) {
     tmp = *(affected_type*)event;
     caster = ((affected_type*)event)->caster;
     STATE2("event_prog: %s (%p)", event->tag, caster);
  }
  (event->proc)(event, now);

  STATE0("finished handle_event");
  return 1;
}

typedef struct
{
  struct char_data* ch;
  char* mask;
  int mask_len;
} de_info;

static int display_event(event_t* event, de_info* info)
{
    char buf[256];

    if(!info->mask_len ||
       (event->tag && !strncmp(info->mask, event->tag, info->mask_len)))
    {
      sprintf(buf, "%p %10.10ld %s\n\r",
	      event, event->when, event->tag ? event->tag : "no tag");
      send_to_char(buf, info->ch);
    }

    return 1;
}

void do_events(struct char_data* ch, char* argument, int cmd)
{
    int i;
    char	buf[256];
    char	type[256];
    char	mask[256];
    de_info	info;

    argument = one_argument(argument, type);
    argument = one_argument(argument, mask);
	
    info.ch = ch;
    info.mask = mask;
    info.mask_len = strlen(mask);
    
    if(is_abbrev(type, "pulse"))
    {
	sprintf(buf, "Pulse Events: (%d)\n\r", pulse);
	send_to_char(buf, ch);

	for(i = 0 ; i < PULSE_PER_MUD_HOUR ; ++i)
	{
	    list_find(&pulse_events[i],
		      (list_find_func) display_event,
		      (void*) &info);
	}
    }
    else if(is_abbrev(type, "real"))
    {
	long	now = time(0);
	
	sprintf(buf, "Real Events: (%ld)\n\r", now);
	send_to_char(buf, ch);

	list_find(&real_events,
		  (list_find_func) display_event,
		  (void*) &info);
    }
    else
	send_to_char("events <pulse|real> [mask]\n\r", ch);
}

int next_pulse(int delta)
{
  while(delta < 0)
    delta += PULSE_PER_MUD_HOUR;
  return (pulse + delta) % PULSE_PER_MUD_HOUR;
}
