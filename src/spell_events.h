#ifndef SPELL_EVENTS_H
#define SPELL_EVENTS_H

#include "structs.h"

spevent_type *spevent_new();
void spevent_clear(spevent_type*);
void spevent_start(spevent_type*);
void spevent_renew(spevent_type*, long dur=0, event_func newfunc=NULL);
void spevent_destroy(spevent_type*);
spevent_list *spevent_on_char(char_data *ch, short skill, spevent_list *sl=NULL);
spevent_list *spevent_on_room(room_data *rm, short skill, spevent_list *sl=NULL);
spevent_list *spevent_on_obj (obj_data *obj, short shill, spevent_list *sl=NULL);
void spevent_remove_all_char(char_data *ch);
void spevent_remove_all_obj (obj_data *obj);
void spevent_depend_char(spevent_type *se, char_data *ch);
void spevent_depend_room(spevent_type *se, room_data *rm);
void spevent_depend_obj (spevent_type *se, obj_data *obj);

#define SPEVENT_CHECK_CHAR(list,next,ch,skill) \
   for(list=spevent_on_char(ch,skill);list;list=next)

#define SPEVENT_CHECK_ROOM(list,next,rm,skill) \
   for(list=spevent_on_room(rm,skill);list;list=next)

#define SPEVENT_NEXT_CHAR(list,next,ch,skill) \
   next=spevent_on_char(ch,skill,list);

#define SPEVENT_NEXT_ROOM(list,next,rm,skill) \
   next=spevent_on_room(rm,skill,list);

#define SPEVENT_CAST(list,next,ch,skill) \
   SPEVENT_NEXT_CHAR(list,next,ch,skill); \
   if(ch != list->sp_event->caster) continue;

#endif
