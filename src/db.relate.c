
#include "config.h"

#include <stdio.h>
#include <assert.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "constants.h"
#include "utility.h"
#include "handler.h"
#include "act.h"
#include "multiclass.h"
#include "trap.h"
#include "fight.h"
#include "statistic.h"

/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj;
  
    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;
    /*  
       (jdb)  hopefully this will fix the object problem   
       */
    obj->carried_by = 0;
    obj->equipped_by = 0;
  
    for(tmp_obj = obj_to ; tmp_obj ; tmp_obj = tmp_obj->in_obj)
    {
	tmp_obj->obj_flags.cont_weight += GET_OBJ_WEIGHT(obj);

	if(tmp_obj->carried_by)
	    IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
    }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
    struct obj_data *tmp, *obj_from;

    assert(!obj->carried_by && !obj->equipped_by && obj->in_room == NOWHERE);
  
    if (obj->in_obj) {
	obj_from = obj->in_obj;
	if (obj == obj_from->contains) /* head of list */
	    obj_from->contains = obj->next_content;
	else {
	    for (tmp = obj_from->contains; 
		 tmp && (tmp->next_content != obj);
		 tmp = tmp->next_content); /* locate previous */
      
	    if (!tmp) {
		perror("Fatal error in object structures.");
		abort();
	    }
      
	    tmp->next_content = obj->next_content;
	}
        
	for(tmp = obj->in_obj ; tmp ; tmp = tmp->in_obj)
	{
	    tmp->obj_flags.cont_weight -= GET_OBJ_WEIGHT(obj);

	    if(tmp->carried_by)
		IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
	}

	obj->in_obj = 0;
	obj->next_content = 0;
    } else {
	perror("Trying to object from object when in no object.");
	abort();
    }
}



/* put an object in a room */
void obj_to_room(struct obj_data *object, room_num room)
{
    struct room_data* roomp;
  
    if (room == NOWHERE)
	room = 4;

    assert(!object->equipped_by);
    assert(object->eq_pos == -1);

    if(!(roomp = real_roomp(room)))
    {
	char buf[256];
	sprintf(buf, "obj_to_room: room doesn't exist: %ld", room);
	log_msg(buf);
	return;
    }
  
    if (object->in_room > NOWHERE) {
	obj_from_room(object);
    }

    object->next_content = roomp->contents;
    roomp->contents = object;
    object->in_room = room;
    object->carried_by = 0;
    object->equipped_by = 0;	/* should be unnecessary */
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *i;
  
    /* remove object from room */

    if (object->in_room <= NOWHERE) {
	if (object->carried_by || object->equipped_by) {
	    log_msg("Eek.. an object was just taken from a char, instead of a room");
	    abort();
	}
	return;			/* its not in a room */
    }
  
    if (object == real_roomp(object->in_room)->contents) /* head of list */
	real_roomp(object->in_room)->contents = object->next_content;
  
    else			/* locate previous element in list */
    {
	for (i = real_roomp(object->in_room)->contents; i && 
	     (i->next_content != object); i = i->next_content);
      
	if (i) {
	    i->next_content = object->next_content;
	} else {
	    log_msg("Couldn't find object in room");
	    abort();
	}
    }
  
    object->in_room = NOWHERE;
    object->next_content = 0;
}


void raw_equip(struct char_data* ch, struct obj_data* obj, int pos)
{
    int j;
    
    ch->equipment[pos] = obj;
    obj->equipped_by = ch;
    obj->eq_pos = pos;
  
    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) -= apply_ac(ch, pos);
  
    for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier, 0, TRUE);

    if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
	/* some nifty manuevering for strength */
	if (IS_NPC(ch) && !IS_SET(ch->specials.mob_act, ACT_POLYSELF))
	    GiveMinStrToWield(obj, ch);
    }
}    

struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int j;
    struct obj_data *obj;
    char buf[150];
  
    assert(pos>=0 && pos<MAX_WEAR);
    /* assert(ch->equipment[pos]); */
    if(!ch->equipment[pos]) {
	sprintf(buf,"Trying to unequip something that is not there from %s position: %d",GET_NAME(ch),pos);
	log_msg(buf);
	return NULL;
    }
  
    obj = ch->equipment[pos];

    assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) += apply_ac(ch, pos);
  
    ch->equipment[pos] = 0;
    obj->equipped_by = 0;
    obj->eq_pos = -1;
  
    for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier, 0, FALSE);
  
    return(obj);
}


/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    char buf[MAX_INPUT_LENGTH];
    struct char_data *i;
    struct room_data *rp;
  
    if (ch->in_room == NOWHERE) {
	log_msg("NOWHERE extracting char from room (handler.c, char_from_room)");
	return;
    }
  
    rp = real_roomp(ch->in_room);
    if (rp==NULL) {
	sprintf(buf, "ERROR: char_from_room: %s was not in a valid room (%ld)",
		GET_NAME(ch), ch->in_room);
	log_msg(buf);
	return;
    }
  
    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) {
		/* Light is ON */
		rp->light--;
		if(rp->light < 0)
		    rp->light = 0;
	    }

      
    if(IS_AFFECTED(ch, AFF_CONTINUAL_LIGHT))
    {
      rp->light--;
      if(rp->light < 0)
	rp->light=0;
    }
 
    if (ch == rp->people)	/* head of list */
	rp->people = ch->next_in_room;
  
    else {			/* locate the previous element */
	for (i = rp->people; i && i->next_in_room != ch; i = i->next_in_room)
	    ;
	if (i)
	    i->next_in_room = ch->next_in_room;
	else {
	    sprintf(buf, "SHIT, %s was not in people list of his room %ld!",
		    GET_NAME(ch), ch->in_room);
	    log_msg(buf);
	}
    }
  
    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}


typedef struct 
{
    event_t	event;
    int		room;
} tele_event_t;

int tele_event_proc(tele_event_t* event, long now)
{
    struct room_data*	rp;
    struct room_data*	dest;
    struct obj_data* obj;
    struct char_data* ch;
    char buf[256];
    
    if(!(rp = real_roomp(event->room)))
    {
	sprintf(buf, "invalid teleport source (%d)",
		rp->tele_targ);
	log_msg(buf);
	goto kill_event;
    }
    
    if(!(dest = real_roomp(rp->tele_targ)))
    {
	sprintf(buf, "invalid teleport target (%d) from room %d",
		rp->tele_targ, event->room);
	log_msg(buf);
	goto kill_event;
    }

    if(event->room == rp->tele_targ)
    {
	sprintf(buf, "invalid teleport to self (%d)",
		rp->tele_targ);
	log_msg(buf);
	goto kill_event;
    }
    
    while((obj = rp->contents))
    {
	obj_from_room(obj);
	obj_to_room(obj, rp->tele_targ);
    }
    
    while((ch = rp->people))
    {
	char_from_room(ch);
	char_to_room(ch, rp->tele_targ);

	if(IS_SET(rp->tele_mask, TELE_LOOK))
	    do_look(ch, "", 15);

	if(IS_SET(dest->room_flags, DEATH) && !IS_IMMORTAL(ch))
	    do_death_trap(ch);
    }

    if(IS_SET(rp->tele_mask, TELE_RANDOM))
    {
	event_queue_pulse((event_t*) event,
			  next_pulse(number(1, 10) * 100),
			  (event_func) tele_event_proc,
			  NULL);

	return 1;
    }
    else
    {
    kill_event:
      event_free(rp->tele_event);
      rp->tele_event = 0;
    }

    return 0;
}

int river_event_proc(tele_event_t* event, long now)
{
    struct room_data*	rp;
    int			rd;
    char		buf[256];
    struct char_data*	ch;
    struct char_data*	c;
    struct char_data*	next_ch;
    struct obj_data*	obj;
    
    if(!(rp = real_roomp(event->room)) ||
       ((rp->sector_type != SECT_WATER_NOSWIM) &&
	(rp->sector_type != SECT_UNDERWATER)) ||
       rp->river_speed <= 0)
      goto kill_event;

    if(((rd = rp->river_dir) > 5) || (rd < 0))
    {
	sprintf(buf, "ILLEGAL RIVER (%d): bad dir %d",
		event->room, rp->river_dir);
	log_msg(buf);

    clear_river:
	rp->river_speed = 0;
	goto kill_event;
    }
    
    if(!rp->dir_option[rd])
    {
	sprintf(buf, "ILLEGAL RIVER (%d): no exit %d",
		event->room, rp->river_dir);
	log_msg(buf);
	goto clear_river;
    }

    sprintf(buf, "You drift %s\n\r", dirs[rd]);
	
    if(!real_roomp(rd = rp->dir_option[rd]->to_room))
    {
	sprintf(buf, "ILLEGAL RIVER (%d): no destination %d",
		event->room, rd);
	log_msg(buf);
	goto clear_river;
    }

    while((obj = rp->contents))
    {
	obj_from_room(obj);
	obj_to_room(obj, rd);
    }

    for(ch = rp->people ; ch ; ch = next_ch)
    {
	next_ch = ch->next_in_room;

	if(MOUNTED(ch) ||	/* wait for mount */
	   IS_AFFECTED(ch, AFF_FLYING)) /* flyers don't drift */
	    continue;

	for(c = ch ; c ; c = RIDDEN(c))
	{
	    if(c->specials.fighting) /* stop combat */
		stop_fighting(c);
	    
	    send_to_char(buf, c);

	    if(c == next_ch)
		next_ch = c->next_in_room;

	    char_from_room(c);
	    char_to_room(c, rd);
	    do_look(c, "", 15);
	}
    }

    if(rp->people)		/* if anybody still in room resched */
    {
	event_queue_pulse((event_t*) event,
			  next_pulse(rp->river_speed),
			  (event_func) river_event_proc,
			  NULL);

	return 1;
    }
    else
    {
    kill_event:
      event_free(rp->river_event);
      rp->river_event = NULL;
    }

    return 0;
}

void KillAffectObjOnChar(struct char_data *ch, long mod)
{
  int i,j,k;
  struct obj_affected_type *affected;

  assert(ch);

  AFF_FLAGS(ch) ^= mod;
  for(i=0; i<MAX_WEAR; i++)
  {
    if(ch->equipment[i])
    {
      affected=ch->equipment[i]->affected;
      for(j=0;j<MAX_OBJ_AFFECT;j++)
      {
    	if(affected[j].location==APPLY_SPELL)
	{
	  if(IS_SET(affected[j].modifier,mod))
	  {
	    affected[j].modifier^=mod;
	    if(affected[j].modifier==0)
	    {
	      for(k=MAX_OBJ_AFFECT-1;k>j;k--)
	      {
		affected[k].location=affected[k+1].location;
		affected[k].modifier=affected[k+1].modifier;
	      }
	      affected[MAX_OBJ_AFFECT].location=0;
	      affected[MAX_OBJ_AFFECT].modifier=0;
	    }
	    act("Some magic fades away from $p.",FALSE,ch,ch->equipment[i],0,
		TO_CHAR);
	  }
	}
      }
    }
  }
}

void AffRoomContLight(struct char_data *ch, struct room_data *rp)
{
  struct char_data *chp;
  int killaffect=0;
  char buf[MAX_STRING_LENGTH];

  if(!rp)
  {
    sprintf(buf,"%s in bogus room(%ld) in AffRoomContLight",GET_REAL_NAME(ch),
	    ch->in_room);
    log_msg(buf);
  } 
  if(!ch)
  {
    log_msg("bogus ch in AffRoomContLight");
  }
 
  for(chp=ch; chp; chp=chp->next_in_room)
  {
    if(IS_AFFECTED(chp, AFF_CONTINUAL_DARK))
    {
      killaffect=1;
      KillAffectObjOnChar(chp, AFF_CONTINUAL_DARK);
    }
  }
      
  if(killaffect)
    KillAffectObjOnChar(ch, AFF_CONTINUAL_LIGHT);
  else if (rp)
  {
    if(rp->light < 0)
      rp->light=0;
    rp->light++;
  }
}

void AffRoomContDark(struct char_data *ch, struct room_data *rp)
{
  struct char_data *chp;
  int killaffect=0;
  char buf[MAX_STRING_LENGTH];

  if(!rp)
  {
    sprintf(buf,"%s in bogus room (%ld) in AffRoomContDark",GET_REAL_NAME(ch),
	    ch->in_room);
    log_msg(buf);
  } 
  if(!ch)
  {
    log_msg("bogus ch in AffRoomContDark");
  }
  for(chp=ch; chp; chp=chp->next_in_room)
  {
    if(IS_AFFECTED(chp, AFF_CONTINUAL_LIGHT))
    {
      killaffect=1;
      KillAffectObjOnChar(chp, AFF_CONTINUAL_LIGHT);
    }
  }
      
  if(killaffect)
    KillAffectObjOnChar(ch, AFF_CONTINUAL_DARK);
  else if(rp)
    rp->light=0;
}

/* place a character in a room */
void char_to_room(struct char_data *ch, room_num room)
{
  struct room_data *rp;
  char		buf[256];
    
  rp = real_roomp(room);
  if (!rp) {
    room = 0;
    rp = real_roomp(room);
    if (!rp) {
      slog("Unknown room number for char_to_room");
      exit(0);
    }
  }

#ifdef SWAP_ZONES
  /* if a non immortal player is moving then check if zone is swapped out */
  if (IS_PC(ch) && !IS_IMMORTAL(ch))
  { 
    /* if zone of destination room is swapped out, then load it */
    if (zone_table[rp->zone].swapped)
    {
      sprintf(buf,"SWAPPING IN zone (%d) %s with lifespan %dm",rp->zone, 
	      zone_table[rp->zone].name, zone_table[rp->zone].lifespan);
/* nolog(buf); */
stat_log(buf,0);
/* JON */
      reset_zone(rp->zone, 0);
    }
  }
#endif

  ch->next_in_room = rp->people;
  rp->people = ch;
  ch->in_room = room;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) {
	/* Light is ON */
	if(rp->light<0)
	  rp->light=0;
	rp->light++;
      }
 
  if(IS_AFFECTED(ch, AFF_CONTINUAL_DARK))
    AffRoomContDark(ch,rp);

  if(IS_AFFECTED(ch, AFF_CONTINUAL_LIGHT))
    AffRoomContLight(ch,rp);

#if !MOBS_TRIGGER_TELEPORT
  if (IS_PC(ch))
#endif
  {
    tele_event_t*	event;
    int		when;

    /* if this is a teleport room and not already armed, arm it */
    if((rp->tele_cnt || rp->tele_time) && !rp->tele_event)
    {
      if(IS_SET(rp->tele_mask, TELE_COUNT))
	when = next_pulse(rp->tele_cnt);
      else
	when = next_pulse(rp->tele_time);

      CREATE(event, tele_event_t, 1);

      event->room = room;

      sprintf(buf, "teleport %ld", room);
      rp->tele_event = event_queue_pulse((event_t*) event, when,
					 (event_func) tele_event_proc,
					 buf);
    }

    if(!rp->river_event &&
       ((rp->sector_type == SECT_WATER_NOSWIM) ||
	(rp->sector_type == SECT_UNDERWATER)) &&
       rp->river_speed)
    {
      when = next_pulse(rp->river_speed);
	    
      CREATE(event, tele_event_t, 1);
      event->room = room;

      sprintf(buf, "river %ld", room);
      rp->river_event = event_queue_pulse((event_t*) event,
					  when,
					  (event_func) river_event_proc,
					  buf);
    }
  }
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
    assert(!object->in_obj && !object->carried_by && !object->equipped_by &&
	   object->in_room == NOWHERE);
  
    object->next_content = ch->carrying;
  
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    object->equipped_by = 0;
    object->in_obj = 0;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp;
    int error = 0;

    if (!object) {
	log_msg("No object to be take from char.");
	error = 1;
    }
  
    if (!object->carried_by) {
	log_msg("this object is not carried by anyone");
	error = 1;
    }
  
    if (!object->carried_by->carrying) {
	log_msg("No one is carrying this object");
	error = 1;
    }

    if (object->in_obj) {
	log_msg("Obj in more than one place.");
	abort(); // don't know what to do with this - raist
    }

    if (object->equipped_by) {
	log_msg("Object is equipped by someone, removing equipment");
	// This is not an error
    }
  
    if (!error) {
      if (object->carried_by->carrying == object)	/* head of list */
	object->carried_by->carrying = object->next_content;
      
      else
	{
	  for (tmp = object->carried_by->carrying; 
	       tmp && (tmp->next_content != object); 
	       tmp = tmp->next_content); /* locate previous */
	  
	  if (!tmp) {
	    log_msg("Couldn't find object on character");
	    abort();
	  }
	  
	  tmp->next_content = object->next_content;
	}
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    if(IS_CARRYING_W(object->carried_by) < 0)
	IS_CARRYING_W(object->carried_by) = 0;
    if(--IS_CARRYING_N(object->carried_by) < 0)
	IS_CARRYING_N(object->carried_by) = 0;
  
    object->carried_by = 0;
    object->equipped_by = 0;	/* should be unnecessary, but, why risk it */
    object->next_content = 0;
    object->in_obj = 0;
}

int GiveMinStrToWield(struct obj_data *obj, struct char_data *ch)
{
    int str=0;

    ch->abilities.str = 16;	/* nice, semi-reasonable start */

    /* 
      will have a problem with except. str, that i do not care to solve
      */

    while ((ch->abilities.str <= 25) &&
	   (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w))
	ch->abilities.str++;

    return(str);
}

room_num room_of_object(struct obj_data *obj)
{
    if (obj->in_room != NOWHERE)
	return obj->in_room;
    else if (obj->carried_by)
	return obj->carried_by->in_room;
    else if (obj->equipped_by)
	return obj->equipped_by->in_room;
    else if (obj->in_obj)
	return room_of_object(obj->in_obj);
    else
	return NOWHERE;
}

