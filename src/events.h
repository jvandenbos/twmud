
#ifndef __EVENTS__
#define __EVENTS__

#include "list.h"

typedef struct event event_t;
typedef int (*event_func)(event_t* theEvent, long now);

struct event
{
    list_element	list;
    char*		tag;
    long		when;
    event_func		proc;
};

struct char_data;

extern void event_init(void);
extern event_t* event_queue_pulse(event_t* theEvent, long pulse,
				  event_func proc, char* tag);
extern event_t* event_queue_real(event_t* theEvent, long when,
				 event_func proc, char* tag);
extern void event_cancel(event_t* theEvent, int do_free);
extern void event_free(event_t* theEvent);
extern void event_process(int pulse);
extern void event_display(struct char_data* ch);

extern int next_pulse(int delta);

#endif
