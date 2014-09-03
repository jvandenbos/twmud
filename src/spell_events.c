#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "spell_events.h"
#include "utils.h"
#include "utility.h"
#include "proto.h"
#include "comm.h"

spevent_type *spevent_new() {
   struct spevent_type *se;
   
   CREATE(se, spevent_type, 1);
   spevent_clear(se);
   
   return se;
}

void spevent_clear(struct spevent_type *se) {
   memset(se, 0, sizeof(*se));
}

void spevent_start(struct spevent_type *se) {
   char buf[80];
   
   sprintf(buf,"spevent %d", se->type);
   
   if(!se->is_pulse) {
      event_queue_real((event_t *) se,
		       time(0) + se->duration,
		       (event_func) se->expire_func,
		       buf);
   } else {
      event_queue_pulse((event_t *) se,
			(pulse + se->duration)%(PULSE_PER_MUD_HOUR),
			(event_func) se->expire_func,
			buf);
   }
}

void spevent_renew(struct spevent_type *se, long dur, event_func newfunc) {
   if(dur)
     se->duration = dur;
   
   if(newfunc)
     se->expire_func = newfunc;
   
   spevent_start(se);
}

void spevent_destroy(struct spevent_type *se) {
   struct spevent_list *next;
   struct char_data *ch;
   struct obj_data *obj;
   struct room_data *room;
   struct generic_list *gnode=NULL, *gnext=NULL;
   struct spevent_list *snode=NULL, *sprev=NULL;
   
   if(se->char_list) {
      for(gnode=se->char_list;gnode;gnode=gnext) {
	 ch = (char_data *)gnode->item;
	 
	 while(ch->sp_list && (ch->sp_list->sp_event == se)) {
	    next=ch->sp_list->next;
	    FREE(ch->sp_list);
	    ch->sp_list=next;
	 }
	 sprev=NULL;
	 for(snode=ch->sp_list;snode;snode=next) {
	    next=snode->next;
	    if(snode->sp_event == se) {
	       next=snode->next;
	       FREE(snode);
	       
	       if(sprev)
	         sprev->next = next;
	    } else {
	       sprev=snode;
	    }
	 }
	 gnext=gnode->next;
	 FREE(gnode);
      }
   }
   
   if(se->obj_list) {
      for(gnode=se->obj_list;gnode;gnode=gnext) {
	 obj = (obj_data *)gnode->item;
	 
	 while(obj->sp_list && (obj->sp_list->sp_event == se)) {
	    next=obj->sp_list->next;
	    FREE(obj->sp_list);
	    obj->sp_list=next;
	 }
	 sprev=NULL;
	 for(snode=obj->sp_list;snode;snode=next) {
	    next=snode->next;
	    if(snode->sp_event == se) {
	       next=snode->next;
	       FREE(snode);
	       
	       if(sprev)
		 sprev->next = next;
	    } else {
	       sprev=snode;
	    }
	 }
	 gnext=gnode->next;
	 FREE(gnode);
      }
   }
   
   if(se->room_list) {
      for(gnode=se->room_list;gnode;gnode=gnext) {
	 room = (room_data *)gnode->item;
	 
	 while(room->sp_list && (room->sp_list->sp_event == se)) {
	    next=room->sp_list->next;
	    FREE(room->sp_list);
	    room->sp_list=next;
	 }
	 sprev=NULL;
	 for(snode=room->sp_list;snode;snode=next) {
	    next=snode->next;
	    if(snode->sp_event == se) {
	       next=snode->next;
	       FREE(snode);
	       
	       if(sprev)
		 sprev->next = next;
	    } else {
	       sprev=snode;
	    }
	 }
	 gnext=gnode->next;
	 FREE(gnode);
      }
   }
   
   if(se->free_func)
     (se->free_func)((event_t*)se,0);
   if(se->extra)
     FREE(se->extra);
   
   event_cancel((event_t*)se,1);
}

spevent_list *spevent_on_char(struct char_data *ch, short skill, spevent_list *se) {
   struct spevent_list *node, *snode;
   snode=(se)?se->next:ch->sp_list;
   
   for(node=snode;node;node=node->next)
     if(node->sp_event->type == skill)
       return node;
   
   return NULL;
}

spevent_list *spevent_on_room(struct room_data *rm, short skill, spevent_list *se) {
   struct spevent_list *node, *snode;
   snode=(se)?se->next:rm->sp_list;
   
   for(node=snode;node;node=node->next)
     if(node->sp_event->type == skill)
       return node;
   
   return NULL;
}

spevent_list *spevent_on_obj (struct obj_data *obj, short skill, spevent_list *se) {
   struct spevent_list *node, *snode;
   snode=(se)?se->next:obj->sp_list;
   
   for(node=snode;node;node=node->next)
     if(node->sp_event->type == skill)
       return node;
   
   return NULL;
}

void spevent_remove_all_char(struct char_data *ch) {
   struct spevent_list *snode;
   
   while((snode = ch->sp_list))
     spevent_destroy(snode->sp_event);
}

void spevent_remove_all_obj(struct obj_data *obj) {
   struct spevent_list *snode;
   
   while((snode = obj->sp_list))
     spevent_destroy(snode->sp_event);
}

void spevent_depend_char(struct spevent_type *se, struct char_data *ch) {
   struct spevent_list *snode;
   struct generic_list *gnode;
   
   for(snode=ch->sp_list;snode;snode=snode->next) {
      if(snode->sp_event == se) {
	 //allready been set up as dependant, so we can leave
	 return;
      }
   }
   
   CREATE(snode, spevent_list, 1);
   CREATE(gnode, generic_list, 1);
   
   snode->sp_event = se;
   snode->next = ch->sp_list;
   ch->sp_list = snode;
   
   gnode->item = ch;
   gnode->next = se->char_list;
   se->char_list = gnode;
}

void spevent_depend_room(struct spevent_type *se, struct room_data *rm) {
   struct spevent_list *snode;
   struct generic_list *gnode;
   
   for(snode=rm->sp_list;snode;snode=snode->next) {
      if(snode->sp_event == se) {
	 return;
      }
   }
   
   CREATE(snode, spevent_list, 1);
   CREATE(gnode, generic_list, 1);
   
   snode->sp_event = se;
   snode->next = rm->sp_list;
   rm->sp_list = snode;
   
   gnode->item = rm;
   gnode->next = se->room_list;
   se->room_list = gnode;
}

void spevent_depend_obj(struct spevent_type *se, struct obj_data *obj) {
   struct spevent_list *snode;
   struct generic_list *gnode;
   
   for(snode=obj->sp_list;snode;snode=snode->next) {
      if(snode->sp_event == se) {
	 return;
      }
   }
   
   CREATE(snode, spevent_list, 1);
   CREATE(gnode, generic_list, 1);
   
   snode->sp_event = se;
   snode->next = obj->sp_list;
   obj->sp_list = snode;
   
   gnode->item = obj;
   gnode->next = se->obj_list;
   se->obj_list = gnode;
}
