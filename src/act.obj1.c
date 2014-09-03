#include "config.h"
#include <stdio.h>
#include <string.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "trap.h"
#include "multiclass.h"
#include "utility.h"
#include "constants.h"
#include "act.h"
#include "fight.h"
#include "find.h"
#include "util_str.h"
#include "recept.h"
#include "spells.h"
#include "statistic.h"
#include "channels.h"
#include "proto.h"
#include "bet.h"
#include "mobprog2.h"

int get(struct char_data *ch, struct obj_data *obj_object,
	struct obj_data *sub_object)
{
  char buffer[256], buf[MAX_STRING_LENGTH];

  if (sub_object)
  {

    if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {

      obj_from_obj(obj_object);
      obj_to_char(obj_object, ch);

      act("You get $p from $P.",0,ch,obj_object,sub_object,TO_CHAR);
      act("$n gets $p from $P.",1,ch,obj_object,sub_object,TO_ROOM);

      if (IS_PC_CORPSE(sub_object) && \
	  (ss_data(sub_object->char_name)!=GET_REAL_NAME(ch)) && \
	  (ss_data(sub_object->char_name)) ) {
	sprintf(buf, "CORPSE LOOT:  %s just got %s the corpse of %s.",
		GET_REAL_NAME(ch), OBJ_SHORT(obj_object),
		ss_data(sub_object->char_name) );
	slog(buf);
      }
  }
    else {
      act("$P must be opened first.",1,ch,0,sub_object,TO_CHAR);
      return 0;
    }
  }
  else
  {
    obj_from_room(obj_object);
    obj_to_char(obj_object, ch);
    act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
    act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
  }

  if (obj_object->obj_flags.type_flag == ITEM_MONEY)
  {
      obj_from_char(obj_object);
      sprintf(buffer, "There was %d coins.\n\r",
	      obj_object->obj_flags.value[0]);
      send_to_char(buffer,ch);
      GET_GOLD(ch) += obj_object->obj_flags.value[0];
      if (GET_GOLD(ch) > 500000 && obj_object->obj_flags.value[0] > 100000 &&
	  (TRUST(ch) < TRUST_IMP))
      {
	  char buf[MAX_INPUT_LENGTH];
	  sprintf(buf,"%s just got %d coins",
		  GET_REAL_NAME(ch),obj_object->obj_flags.value[0]);
	  log_msg(buf);
      }

      if (IS_SET(ch->specials.flags, PLR_AUTOSPLIT) &&
	  IS_AFFECTED(ch, AFF_GROUP))
      {
	  sprintf(buffer, " %d", obj_object->obj_flags.value[0]);
	  do_split(ch, buffer, 258);
      }
      extract_obj(obj_object);
      return 1;
  }

  oprog_get_trigger(obj_object, ch);
  return 1;
}


int try_to_get_object(struct char_data *ch, struct obj_data *obj)
{
  char buffer[MAX_STRING_LENGTH];
  obj_data *in_object;
  char_data *carrying;

  in_object = obj->in_obj;

  if (in_object)
   carrying = in_object->carried_by;
  else
   carrying = NULL;

  if (!IsLevelOk(ch, obj))
    return FALSE;

  if (CheckForAnyTrap(ch, obj))
    return FALSE;

  if ((IS_CARRYING_N(ch) + 1) > CAN_CARRY_N(ch)) {
    sprintf(buffer,"%s : You can't carry that many items.\n\r",
	    OBJ_SHORT(obj));
    send_to_char(buffer, ch);
    return FALSE;
  }

  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch) &&
       carrying != ch) {
    sprintf(buffer,"%s : You can't carry that much weight.\n\r",
	    OBJ_SHORT(obj));
    send_to_char(buffer, ch);
    return FALSE;
  }

  if (!CAN_WEAR(obj,ITEM_TAKE) && (TRUST(ch) < TRUST_SAINT))
  {
      send_to_char("You can't take that!\n\r", ch);
      return FALSE;
  }

  if (((obj->obj_flags.type_flag == ITEM_MONEY) && (GET_GOLD(ch)>= 2000000))
      && (TRUST(ch) < TRUST_SAINT))
  {
    send_to_char("You can't possible carry any more coins!  Visit the bank.\n\r",ch);
    return FALSE;
  }
  return TRUE;
}


int try_to_get(struct char_data* ch, struct obj_data* obj,
		   struct obj_data* sub_object)
{
  char buf[256];

  if(sub_object && IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED))
  {
    sprintf(buf, "%s is closed.\n\r",OBJ_SHORT(sub_object));
    send_to_char(buf,ch);
    return 0;
  }

  if (try_to_get_object(ch, obj))
    return get(ch, obj, sub_object);
  else
    return 0;
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *sub_object;
    struct obj_data *obj_object;
    struct obj_data *next_obj;
    bool found = FALSE, foundone = FALSE;
    bool fail  = FALSE;
    int type   = 3;
    char newarg[MAX_INPUT_LENGTH];
    int num, p;

    argument_interpreter(argument, arg1, arg2);

    /* get type */
    if (!*arg1) {
	type = 0;
    }
    if (*arg1 && !*arg2) {
	if (!str_cmp(arg1,"all")) {
	    type = 1;
	} else {
	    type = 2;
	}
    }
    if (*arg1 && *arg2) {
	if (!str_cmp(arg1,"all")) {
	    if (!str_cmp(arg2,"all")) {
		type = 3;
	    } else {
		type = 4;
	    }
	} else {
	    if (!str_cmp(arg2,"all")) {
		type = 5;
	    } else {
		type = 6;
	    }
	}
    }

    switch (type) {
	/* get */
    case 0:{
	send_to_char("Get what?\n\r", ch);
    } break;
	/* get all */
    case 1:{
	sub_object = 0;
	found = FALSE;
	fail = FALSE;
	for(obj_object = real_roomp(ch->in_room)->contents;
	    obj_object;
	    obj_object = next_obj) {
	    next_obj = obj_object->next_content;
	    if (CAN_SEE_OBJ(ch,obj_object)) {
		found = TRUE;
		fail = try_to_get(ch, obj_object, NULL);
	    }
	}
	if (found) {
	    send_to_char("OK.\n\r", ch);
	} else {
	    if (!fail) send_to_char("You see nothing here.\n\r", ch);
	}
    } break;
	/* get ??? (something) */
    case 2:{
	sub_object = 0;
	found = FALSE;
	fail = FALSE;
	if (getall(arg1,newarg)) {
	    strcpy(arg1,newarg);
	    num = -1;
	} else if ((p = getabunch(arg1,newarg))) {
	    strcpy(arg1,newarg);
	    num = p;
	} else {
	    num = 1;
	}

	while (num != 0) {
	    obj_object = get_obj_in_list_vis(ch, arg1,
					     real_roomp(ch->in_room)->contents);
	    if (obj_object){
		found = TRUE;
		if(try_to_get(ch, obj_object, NULL)){
		    if (num > 0) num--;
		}else{
		    num = 0;
		    fail = TRUE;
		}
	    }
	    else
		num = 0;
	}
	if(!found && !fail){
	    sprintf(buffer, "You don't see %s\n\r", arg1);
	    send_to_char(buffer, ch);
	}
    } break;
	/* get all all */
    case 3:{
	send_to_char("You must be joking?!\n\r", ch);
    } break;
	/* get all ??? */
    case 4:{
	found = FALSE;
	fail	= FALSE;
	sub_object = (struct obj_data *) get_obj_vis_accessible(ch, arg2);
	if (sub_object) {
	    if (GET_ITEM_TYPE(sub_object)==ITEM_CONTAINER){
		for(obj_object = sub_object->contains;
		    obj_object;
		    obj_object = next_obj) {
		    next_obj = obj_object->next_content;
		    /* check for trap (jdb - 11/9) */
		    if(CAN_SEE_OBJ(ch, obj_object)){
			found = TRUE;
		        fail = !try_to_get(ch, obj_object, sub_object);
			foundone = !fail;
		    }

		}
      if ( foundone && (IS_PC_CORPSE(sub_object)) && \
	  (ss_data(sub_object->char_name)!=GET_REAL_NAME(ch)) && \
	  (ss_data(sub_object->char_name)) ) {
		      sprintf(buffer,"PLAYER LOOT:  %s just looted from %s", GET_REAL_NAME(ch), OBJ_SHORT(sub_object));
		      log_msg(buffer);
		    }
		if (found && IS_PC_CORPSE(sub_object)) {
		    remove("corpsedata.dat");
		}
		if (!found && !fail) {
		    sprintf(buffer,"You do not see anything in %s.\n\r",
			    OBJ_SHORT(sub_object));
		    if (cmd)
		        send_to_char(buffer, ch);
		    fail = TRUE;
		}
	    } else {
		sprintf(buffer,"%s is not a container.\n\r",
			OBJ_SHORT(sub_object));
		send_to_char(buffer, ch);
		fail = TRUE;
	    }
	} else {
	    sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	}
    } break;
    case 5:{
	send_to_char("You can't take a thing from more than one container.\n\r",
		     ch);
    } break;
	/*
	   take ??? from ???   (is it??)
	   */

    case 6:{
      found = FALSE;
      fail	= FALSE;
      sub_object = (struct obj_data *)
	get_obj_vis_accessible(ch, arg2);
      if (sub_object) {
	if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	  if (getall(arg1,newarg)) {
	    num = -1;
	    strcpy(arg1,newarg);
	  } else if ((p = getabunch(arg1,newarg))) {
	    num = p;
	    strcpy(arg1,newarg);
	  } else {
	    num = 1;
	  }

	  while (num != 0) {

	    obj_object = get_obj_in_list_vis(ch, arg1,
					     sub_object->contains);
	    if (obj_object){
	      found = TRUE;
	      if(try_to_get(ch, obj_object, sub_object)){
		if(num>0) num--;
	      }else{
		fail = TRUE;
		num = 0;
	      }
	    }
	    else
	      num = 0;
	  }
      if ( !fail && found && (IS_PC_CORPSE(sub_object)) && \
	  (ss_data(sub_object->char_name)!=GET_REAL_NAME(ch)) && \
	  (ss_data(sub_object->char_name)) ) {
	    sprintf(buffer,"PLAYER LOOT:  %s just looted from %s", GET_REAL_NAME(ch), OBJ_SHORT(sub_object));
	    log_msg(buffer);
	  }

	  if(!fail && found && IS_PC_CORPSE(sub_object)) {
	    remove("corpsedata.dat");
	  }

	  if(!found && !fail && cmd){
	    sprintf(buffer,"%s does not contain the %s.\n\r",
		    OBJ_SHORT(sub_object), arg1);
	    send_to_char(buffer, ch);
	  }
	} else {
	  sprintf(buffer,"%s is not a container.\n\r", OBJ_SHORT(sub_object));
	  send_to_char(buffer, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
	send_to_char(buffer, ch);
	fail = TRUE;
      }
    } break;
  }
}


#define DONATION_ROOM 2625
void do_donate(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *object;
  struct obj_data *tmp_object;
  bool donated;
  int room;

  only_argument(argument, arg);

  if (!*arg) {
    send_to_char("What do you wish to donate?\n\r", ch);
    return;
  }

  if (!(object=get_obj_in_list_vis(ch, arg, ch->carrying))) {
    send_to_char("You can't donate what you don't have.\n\r", ch);
    return;
  }

  if (IS_SET(object->obj_flags.extra_flags, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
    send_to_char("You can't donate it, it must be CURSED!\n\r", ch);
    return;
  }

  if(object->obj_flags.cost<1) {  /*can't donate corpses, worthless stuff*/
    send_to_char("Try donating something with a bit of value.\n\r",ch);
    return;
  }

  room=ch->in_room;
  char_from_room(ch);
  char_to_room(ch, DONATION_ROOM);
  act("$p materializes thanks to $n's generosity.",TRUE,ch,object,0,TO_ROOM);

  char_from_room(ch);
  char_to_room(ch, room);
  act("$n donates $p.", TRUE, ch, object, 0, TO_ROOM);
  act("You generously donate $p.", TRUE, ch, object, 0, TO_CHAR);

  object->obj_flags.cost = -1; // Donated item will not be of any value!

  donated = FALSE;

  for(tmp_object = real_roomp(DONATION_ROOM)->contents;
      (tmp_object && !donated);
      tmp_object = tmp_object->next_content) {
    switch (GET_ITEM_TYPE(object)) {
    case ITEM_WEAPON:
    case ITEM_MISSILE:
    case ITEM_FIREWEAPON:
      if (!(strcmp(OBJ_NAME(tmp_object), "chest donate weapon"))) {
	obj_from_char(object);
	obj_to_obj(object, tmp_object);
	donated = TRUE;
      }
      break;
    case ITEM_ARMOR:
      if (!(strcmp(OBJ_NAME(tmp_object), "chest donate armor"))) {
	obj_from_char(object);
	obj_to_obj(object, tmp_object);
	donated = TRUE;
      }
      break;
    default:
      if (!(strcmp(OBJ_NAME(tmp_object), "chest donate misc"))) {
	obj_from_char(object);
	obj_to_obj(object, tmp_object);
	donated = TRUE;
      }
      break;
    }
  }
}


void do_drop(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  int amount;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  bool test = FALSE;
  char newarg[MAX_INPUT_LENGTH];
  char *s;
  int num, p;

  s=one_argument(argument, arg);
  if(is_number(arg))	{
    amount = atoi(arg);
    strcpy(arg, s);

    if (0!=str_cmp("coins",arg) && 0!=str_cmp("coin",arg))  {
      send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
      return;
    }
    if(amount<0)  	{
      send_to_char("Sorry, you can't do that!\n\r",ch);
      return;
    }
    if(GET_GOLD(ch)<amount)	 {
      send_to_char("You haven't got that many coins!\n\r",ch);
      return;
    }
    send_to_char("OK.\n\r",ch);
    if(amount==0)
      return;

    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
    tmp_object = create_money(amount);
    obj_to_room(tmp_object,ch->in_room);
    GET_GOLD(ch)-=amount;
    return;
  } else {
    only_argument(argument, arg);
  }

  if (*arg) {
    if (!str_cmp(arg,"all")) {
      for(tmp_object = ch->carrying;
	  tmp_object;
	  tmp_object = next_obj) {
	next_obj = tmp_object->next_content;
	if ((! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) ||
            IS_IMMORTAL(ch))
	{
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You drop %s.\n\r", OBJ_SHORT(tmp_object));
	    send_to_char(buffer, ch);
	  } else {
	    send_to_char("You drop something.\n\r", ch);
	  }
	  act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	  obj_from_char(tmp_object);
	  obj_to_room(tmp_object,ch->in_room);
	  oprog_drop_trigger(tmp_object, ch);
	  test = TRUE;
	} else {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You can't drop  %s, it must be CURSED!\n\r",
		    OBJ_SHORT(tmp_object));
	    send_to_char(buffer, ch);
	    test = TRUE;
	  }
	}
      }
      if (!test) {
	send_to_char("You do not seem to have anything.\n\r", ch);
      }
#if   NODUPLICATES
      do_save(ch, "", 0);
#endif
    } else {
      /* &&&&&& */
      if (getall(arg,newarg)) {
	num = -1;
	strcpy(arg,newarg);
      } else if ((p = getabunch(arg,newarg))) {
	num = p;
	strcpy(arg,newarg);
      } else {
	num = 1;
      }

      while (num != 0) {
	tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	if (tmp_object) {
	  if ((! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) ||
              IS_IMMORTAL(ch))
	  {
	    sprintf(buffer, "You drop %s.\n\r", OBJ_SHORT(tmp_object));
	    send_to_char(buffer, ch);
	    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	    obj_from_char(tmp_object);
	    obj_to_room(tmp_object,ch->in_room);
	    oprog_drop_trigger(tmp_object, ch);
	  } else {
	    send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
	    num = 0;
	  }
	} else {
	  if (num > 0)
	    send_to_char("You do not have that item.\n\r", ch);

	  num = 0;
	}
	if (num > 0) num--;
      }
#if   NODUPLICATES
      do_save(ch, "", 0);
#endif
    }
  } else {
    send_to_char("Drop what?\n\r", ch);
  }
}



void do_put(struct char_data *ch, char *argument, int cmd)
{
  char buffer[256];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj_object;
  struct obj_data *sub_object;
  struct char_data *tmp_char;
  int bits;
  char newarg[MAX_INPUT_LENGTH];
  int num, p;

  argument_interpreter(argument, arg1, arg2);

  if (*arg1) {
    if (*arg2) {

      if (getall(arg1,newarg)) {
	num = -1;
	strcpy(arg1,newarg);
      } else if ((p = getabunch(arg1,newarg))) {
	num = p;
	strcpy(arg1,newarg);
      } else {
	num = 1;
      }

      if (!strcmp(arg1,"all")) {

	send_to_char("sorry, you can't do that (yet)\n\r",ch);
	return;

      } else {
	while (num != 0) {
#if 1
	  bits = generic_find(arg1, FIND_OBJ_INV,
			      ch, &tmp_char, &obj_object);
#else
	  obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif

	  if (obj_object) {
	    if ((IS_OBJ_STAT(obj_object,ITEM_NODROP)) &&
                IS_IMMORTAL(ch))
	    {
		send_to_char
		    ("You can't let go of it, it must be CURSED!\n\r", ch);
		return;
	    }
	    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
				ch, &tmp_char, &sub_object);
	    if (sub_object) {
	      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
		if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
		  if (obj_object == sub_object) {
		    send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		    return;
		  }
		  if ((sub_object->obj_flags.weight +
		       obj_object->obj_flags.cont_weight) <
		      sub_object->obj_flags.value[0]) {
		    act("You put $p in $P",TRUE, ch, obj_object, sub_object, TO_CHAR);
		    if (bits==FIND_OBJ_INV) {
		      obj_from_char(obj_object);
		      obj_to_obj(obj_object, sub_object);
		    } else {
		      obj_from_room(obj_object);
		      obj_to_obj(obj_object, sub_object);
		    }

		    act("$n puts $p in $P",TRUE, ch, obj_object, sub_object, TO_ROOM);
		    num--;
		  } else {
		    send_to_char("It won't fit.\n\r", ch);
		    num = 0;
		  }
		} else {
		  send_to_char("It seems to be closed.\n\r", ch);
		  num = 0;
		}
	      } else {
		sprintf(buffer,"%s is not a container.\n\r",
			OBJ_SHORT(sub_object));
		send_to_char(buffer, ch);
		num = 0;
	      }
	    } else {
	      sprintf(buffer, "You don't have the %s.\n\r", arg2);
	      send_to_char(buffer, ch);
	      num = 0;
	    }
	  } else {
	    if ((num > 0) || (num == -1)) {
	      sprintf(buffer, "You don't have the %s.\n\r", arg1);
	      send_to_char(buffer, ch);
	    }
	    num = 0;
	  }
	}
#if   NODUPLICATES
      do_save(ch, "", 0);
#endif
      }
    } else {
      sprintf(buffer, "Put %s in what?\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } else {
    send_to_char("Put what in what?\n\r",ch);
  }
}

void throw_weapon(struct obj_data *o, int dir, struct char_data *targ,
                  struct char_data *ch, int att_type)
{
    char buf[MAX_STRING_LENGTH];
    int range, maxrange, found;
    room_num room;

    maxrange=1; /* default for skill throw */
    if (att_type==SKILL_ARCHERY) /* fired missile from weapon */
      maxrange=ch->equipment[WIELD]->obj_flags.value[2];

    range=0;
    found=FALSE;
    room=ch->in_room;

    while (range<maxrange && !found) {
      range++;
      if ( real_roomp(room)->dir_option[dir] ) // Min's Throw crash fix - No Null dirs
	{
	  room=real_roomp(room)->dir_option[dir]->to_room;
	  if (room!=targ->in_room) {
	    sprintf(buf,"%s from %s flies by!\n\r",OBJ_SHORT(o),dir_from[dir]);
	    send_to_room(buf,room);
	  } else {
	    found=TRUE;
	  }
	}
    }

    if (!found) {
      send_to_char("Your attempt falls short of your target.\n\r", ch);
      act("$n's attempt falls short of $s target.\n\r",TRUE,ch,0,0,TO_ROOM);
      if (att_type==SKILL_ARCHERY && number(1, 100) < o->obj_flags.value[0]) {
        sprintf(buf,"%s from %s hits the ground and snaps.\n\r",
		OBJ_SHORT(o),dir_from[dir]);
        extract_obj(o);
      } else {
        sprintf(buf,"%s from %s hits the ground.\n\r",
		OBJ_SHORT(o),dir_from[dir]);
        obj_to_room(o, room);
      }
      send_to_room(buf,room);
      return;
    }

    range_hit(ch, targ, o, dir, range);
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
   struct obj_data *fw, *ms;
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];

   fw = ch->equipment[WIELD];
   if ((!fw)||(fw->obj_flags.type_flag!=ITEM_FIREWEAPON)) {
send_to_char("You must wield the projectile weapon you want to load.\n\r",ch);
      return;
   }
   if ((GET_STR(ch)+(GET_ADD(ch)/3))<fw->obj_flags.value[0]) {
 send_to_char("You aren't strong enough to draw such a mighty weapon.\n\r",ch);
      return;
   }
   if (ch->equipment[LOADED]) {
      if (CAN_CARRY_N(ch)!=IS_CARRYING_N(ch)) {
         ms = unequip_char(ch, LOADED);
         act("You first unload $p.",TRUE,ch,ms,0,TO_CHAR);
         obj_to_char(ms, ch);
         act("$n unloads $p.",FALSE,ch,ms,0,TO_ROOM);
      } else {
         send_to_char("Your hands are to full to unload.\n\r",ch);
         return;
      }
   }

   half_chop(argument, arg1, arg2);
   if (!*arg1) {
      send_to_char("You must specify the projectile to load!\n\r",ch);
      return;
   }
   ms = get_obj_in_list_vis(ch, arg1, ch->carrying);
   if (!ms) {
      send_to_char("You don't seem to have that.\n\r",ch);
      return;
   }
   if (ms->obj_flags.type_flag!=ITEM_MISSILE) {
      act("$p is not a valid missile.",TRUE,ch,ms,0,TO_CHAR);
      return;
   }
   if (ms->obj_flags.value[3]!=fw->obj_flags.value[3]) {
      act("You can't load $p in that sort of weapon.",TRUE,ch,ms,0,TO_CHAR);
      return;
   }

   obj_from_char(ms);
   equip_char(ch,ms,LOADED);
   act("You load $p.",TRUE,ch,ms,0,TO_CHAR);
   act("$n loads $p.",FALSE,ch,ms,0,TO_ROOM);
   WAIT_STATE(ch, PULSE_VIOLENCE);
}

int clearpath(struct char_data *ch, room_num room, int direc)
{
   int opdir[] = {2, 3, 0, 1, 5, 4};
   struct room_direction_data	*exitdata;

   exitdata = (real_roomp(room)->dir_option[direc]);

   if ((exitdata) &&
      (!real_roomp(exitdata->to_room))) return 0;
   if (!CAN_GO(ch, direc)) return 0;
   if (!real_roomp(room)->dir_option[direc]) return 0;
   if (real_roomp(room)->dir_option[direc]->to_room<1) return 0;
   if (real_roomp(room)->zone!=real_roomp(real_roomp(room)->dir_option[direc]->to_room)->zone)
      return 0;
   if (IS_SET(real_roomp(room)->dir_option[direc]->exit_info,EX_CLOSED)) return 0;
   if ((!IS_SET(real_roomp(room)->dir_option[direc]->exit_info,EX_ISDOOR))&&
      (real_roomp(room)->dir_option[direc]->exit_info>0)) return 0;
   /* One-way windows are allowed... no see through 1-way exits */
   if (!real_roomp(real_roomp(room)->dir_option[direc]->to_room)->dir_option[opdir[direc]]) return 0;

   if (real_roomp(real_roomp(room)->dir_option[direc]->to_room)->dir_option[opdir[direc]]->to_room<1) return 0;

   if (real_roomp((real_roomp(room)->dir_option[direc]->to_room))->dir_option[opdir[direc]]->to_room!=room) return 0;


   return real_roomp(room)->dir_option[direc]->to_room;
}

void do_give(struct char_data *ch, char *argument, int cmd)
{
    char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH], buf[132];
    char arg[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
    int amount, num, p;
    struct char_data *vict=NULL;
    struct obj_data *obj;

    argument=one_argument(argument,obj_name);
    if(is_number(obj_name))
    {
	amount = atoi(obj_name);
	argument=one_argument(argument, arg);
	if (str_cmp("coins",arg) && str_cmp("coin",arg))
	{
	    send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
	    return;
	}
	if(amount<0)
	{
	    send_to_char("Sorry, you can't do that!\n\r",ch);
	    return;
	}
	if((GET_GOLD(ch)<amount) &&
	   (IS_NPC(ch) || (TRUST(ch) < TRUST_DEMIGOD)))
	{
	    send_to_char("You haven't got that many coins!\n\r",ch);
	    return;
	}

	argument=one_argument(argument, vict_name);

	if(!*vict_name)
	{
	    send_to_char("To who?\n\r",ch);
	    return;
	}

	if (!(vict = get_char_room_vis(ch, vict_name)))	{
	    send_to_char("To who?\n\r",ch);
	    return;
	}

	    MOBTrigger = FALSE;
        if (IS_SET(vict->specials.mob_act, ACT_LIQUID)) {
          send_to_char("You can't give anything to that.\n\r", ch);
          return;
        }

	if((GET_GOLD(vict)<2000000) ||
	   (TRUST(vict)>TRUST_SAINT) ||
	   (TRUST(ch)>TRUST_SAINT))
	{
	    send_to_char("Ok.\n\r",ch);
	    sprintf(buf,"%s gives you %d gold coins.\n\r",
		    PERS(ch,vict),amount);
	    send_to_char(buf,vict);
	    act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
	    if (IS_NPC(ch) || (TRUST(ch) < TRUST_DEMIGOD))
		GET_GOLD(ch)-=amount;
	    GET_GOLD(vict)+=amount;

	    SaveChar(ch, AUTO_RENT, 0);

	    mprog_bribe_trigger(vict, ch, amount);

	    if ((GET_GOLD(vict) > 500000) &&
		(amount > 100000) &&
		(TRUST(ch) < TRUST_IMP))
	    {
		sprintf(buf, "%s gave %d coins to %s",
			GET_REAL_NAME(ch), amount, GET_REAL_NAME(vict));
		log_msg(buf);
	    }
	}
	else
	{
	    send_to_char("That person cant carry any more coins.\n\r",ch);
	    send_to_char("You cant carry any more coins!  Visit a bank.\n\r",
			 vict);
	}
    }
    else
    {
	argument=one_argument(argument, vict_name);

	if (!*obj_name || !*vict_name)	{
	    send_to_char("Give what to who?\n\r", ch);
	    return;
	}
	/* &&&& */
	if (getall(obj_name,newarg)) {
	    num = -1;
	    strcpy(obj_name,newarg);
	} else if ((p = getabunch(obj_name,newarg))) {
	    num = p;
	    strcpy(obj_name,newarg);
	} else {
	    num = 1;
	}

	while (num != 0)
	{

	    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
	    {
		if (num >= -1)
		    send_to_char("You do not seem to have anything like that.\n\r",ch);
		return;
	    }
	    if ((IS_OBJ_STAT(obj, ITEM_NODROP)) && (!IS_IMMORTAL(ch))) {
		send_to_char
		    ("You can't let go of it, it must be CURSED!\n\r", ch);
		return;
	    }
	    if (!(vict = get_char_room_vis(ch, vict_name)))	{
		send_to_char("No one by that name around here.\n\r", ch);
		return;
	    }
            if (IS_SET(vict->specials.mob_act, ACT_LIQUID)) {
              send_to_char("You can't give anything to that.\n\r", ch);
              return;
            }

	    if (vict == ch) {
		send_to_char("Ok.\n\r", ch);
		return;
	    }

	    if((obj->obj_flags.level > 40) &&
	       (GetMaxLevel(vict) < obj->obj_flags.level - 10)) {
	       cprintf(ch, "That person can't use an object of that much power.\n\r");
	       return;
	    }

	    if ((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))	{
		MOBTrigger = FALSE;
		act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
		return;
	    }
	    if ((GET_OBJ_WEIGHT(obj)+IS_CARRYING_W(vict)) >
		CAN_CARRY_W(vict))   {
		act("$E can't carry that much weight.",
		    0, ch, 0, vict, TO_CHAR);
		return;
	    }
	    obj_from_char(obj);
	    obj_to_char(obj, vict);
	    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
	    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
	    act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);

	    mprog_give_trigger(vict, ch, obj);
	    mprog_give_trigger2(vict, ch, obj);

	    if (num > 0) num--;

	}
#if   NODUPLICATES
	do_save(ch, "", 0);
	do_save(vict, "", 0);
#endif

    }
}

// Nominate a player for a heroes point
ACMD(do_nominate) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct char_data* vict;
  int i;

  arg = one_argument (arg, arg1); /* parse off first arg -> arg1 */

  if (arg1[0] == '\0') { /* no parameters */
    send_to_char_formatted("You must nominate a player\r\n", ch);
    return;
  } else {
    // If char is present in the world
    if (vict = get_char(arg1)) {

      // Check that char is not already nominated
      for (i = 0; i < vote->total_nominees; i++) {
	if (vict == vote->nominees[i]->nominee) {
	  send_to_char_formatted("That player has already been nominated\r\n", ch);
	  return;
	}
      }

      // Add player
      CREATE(nomi,NOMINEE_DATA,1);
      nomi->nominee = vict;
      nomi->votes = 0;
      vote->nominees[vote->total_nominees] = nomi;
      vote->total_nominees++;
      sprintf(buf, "Player %s added to nominees\r\n", GET_REAL_NAME(vict));
      send_to_char_formatted(buf, ch);
      return;
    } else {
      send_to_char_formatted("That player is not present in this world\r\n", ch);
    }
  }

  return;
}

/* FILE: act_obj.c - main procedure of the package */

/* put an item on auction, or see the stats on the current item or bet */
ACMD(do_vote) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf_mortal[MAX_STRING_LENGTH];
  char buf_imm[MAX_STRING_LENGTH];
  int minbet;
  int i;

  arg = one_argument (arg, arg1); /* parse off first arg -> arg1 */

  if (!IS_PC(ch)) /* NPC's don't get to vote! */
    return;

  if (arg1[0] == '\0') { /* no parameters */

    send_to_char_formatted(   "\r\n  ***** And the nominees are *****\n\r", ch);

    if (vote->total_nominees > 0) {
      for (i = 0; i < vote->total_nominees; i++) {
	sprintf (buf_mortal, "  * %s\n\r", GET_REAL_NAME( (vote->nominees[i])->nominee), vote->nominees[i]->votes);
	sprintf (buf_imm, "  * %d votes: %s\n\r", vote->nominees[i]->votes, GET_REAL_NAME( (vote->nominees[i])->nominee), vote->nominees[i]->votes);

	if (IS_IMMORTAL(ch)) {
	  send_to_char_formatted(buf_imm, ch);
	} else {
	  send_to_char_formatted(buf_mortal, ch);
	}
      }
    } else {
      send_to_char_formatted("  * No nominees on this hero vote.\n\r",ch);
    }

    send_to_char_formatted(   "  ***********************************\n\r", ch);

    return;
  }

  // Cancel vote
  if (IS_IMMORTAL(ch) && (!str_cmp(arg1,"stop") || !str_cmp(arg1,"cancel"))) {

    if (vote->vote_start) { /* Vote has started */
      sprintf (buf,"$CcThis vote has been cancelled by the Gods, no points awarded\n\r");
      talk_vote (buf);

      clear_voting();
    } else {
      send_to_char_formatted("There is no vote going on you can stop.\n\r",ch);
    }

    return;
  }

  // Clear vote
  if (IS_IMMORTAL(ch) && !str_cmp(arg1,"clear")) {

    if (vote->vote_start) { /* Vote has started */
      send_to_char_formatted("You can't clear a vote while it's in progress. Type 'vote stop' to cancel vote\n\r", ch);
    } else {
      send_to_char_formatted("Vote has been cleared\n\r", ch);
      clear_voting();
    }

    return;
  }


  if (!str_cmp(arg1,"help")) {
    send_to_char_formatted("$CCVote help---\n\r$CN",ch);
    send_to_char("   vote   (no parameters)   -- will show the list of nominees\n\r",ch);
    send_to_char("   vote <player>            -- Vote for one of the nominees\n\r",ch);
    if (IS_IMMORTAL(ch)) {
      send_to_char("   nominate <player>        -- Nominate a player for the hero point\n\r",ch);
      send_to_char("   vote start               -- Starts a vote with the current setup\n\r",ch);
      send_to_char("   vote clear               -- Clear current vote setup\n\r",ch);
      send_to_char("   vote stop/cancel         -- Stop current vote in progress\n\r",ch);
    }
    send_to_char("NB: You can only vote once and only during the progressing of a vote\n\r",ch);
    return;
  }

  if (IS_IMMORTAL(ch) && !str_cmp(arg1, "start")) {
    if(vote->total_nominees < 1) {
      send_to_char_formatted("But no candidates have been nominated yet\r\n", ch);
      return;
    }

    if (vote->vote_start) {
      send_to_char_formatted("Easy now, a vote is already in progress", ch);
      return;
    } else {
      vote->vote_start = TRUE;
      vote->pulse = PULSE_AUCTION;
      vote->going = 0;

      talk_vote ("$CcA new vote has started, type 'vote' to see nominees.\n\r");
      talk_vote("$CwPlease vote for the most worthy candidate now.\n\r");
    }
  }

  // Vote for someone
  int vote_given = 0;
  int j;
  if (vote->vote_start) { /* Vote has started */

    // If char has already voted, reject vote
    for(j = 0; j < vote->total_votes; j++) {
      if (vote->voters[j] == ch) {
	send_to_char_formatted("Oh no you don't, you have already voted once!\r\n", ch);
	return;
      }
    }

    // Add vote
    for (i = 0; i < vote->total_nominees; i++) {
      if (get_char(arg1) == vote->nominees[i]->nominee) {

	// Make sure you cannot vote for yourself
	if(vote->nominees[i]->nominee == ch) {
	  send_to_char_formatted("You cannot vote for yourself!\r\n", ch);
	  return;
	}

	vote->nominees[i]->votes++;
	vote_given = 1;
	send_to_char_formatted("Thank you for your vote\r\n", ch);
      }
    }

    // Add voter to list of people who have already voted
    if (vote_given) {
      vote->voters[vote->total_votes] = ch;
      vote->total_votes++;
    }

  } else {
    send_to_char_formatted ("$CwVote for who? No vote in session, wait for it.\n\r",ch);
  }

  return;
}

void clear_voting() {
  char buf[MAX_STRING_LENGTH];
  int i;

  for (i = 0; i < vote->total_nominees; i++) {
    FREE(vote->nominees[i]);
  }

  for (i = 0; i < vote->total_votes; i++) {
    vote->voters[i] = NULL;
  }

  vote->vote_start = FALSE;
  vote->total_votes = 0;
  vote->total_nominees = 0;

  return;
}

/* put an item on auction, or see the stats on the current item or bet */
ACMD(do_auction) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int minbet;

  arg = one_argument (arg, arg1); /* parse off first arg -> arg1 */

  if (!IS_PC(ch)) /* NPC can be extracted at any time and thus can't auction! */
    return;

  if (arg1[0] == '\0') /* no parameters */
    if (auction->item != NULL) { /* and an item in auction */
      /* show item data here */
      if (auction->bet > 0)
	sprintf (buf, "$CcCurrent bid on this item is$CY %d$Cc gold.\n\r",auction->bet);
      else
	sprintf (buf, "$CcNo bids on this item have been received.\n\r");
      send_to_char_formatted (buf,ch);
      spell_identify(LVL_GOD - 1, ch, 0, NULL, auction->item);
      return;
    }
    else
      {
	send_to_char_formatted ("$CwAuction WHAT?\n\r",ch);
	return;
      }

  if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    if (auction->item == NULL) {
      send_to_char ("There is no auction going on you can stop.\n\r",ch);
      return;
    }
    else {/* stop the auction */

      sprintf (buf,"$CcSale of$CG %s$Cc has been stopped by God. Item confiscated.\n\r",
	       ss_data(auction->item->short_description));
      talk_auction (buf);
      obj_to_char (auction->item, ch);
      auction->item = NULL;
      if (auction->buyer != NULL) { /* return money to the buyer */
	GET_GOLD(auction->buyer) += auction->bet;
	send_to_char_formatted ("$CcYour money has been returned.\n\r",auction->buyer);
      }
      return;
    }

  if (!str_cmp(arg1,"listen")) {
    REMOVE_BIT(ch->channels,COM_AUCTION);
    send_to_char_formatted("$CcYou will be able to monitor auctions now.\n\r",ch);
    return;
  }

  if (!str_cmp(arg1,"deaf")) {
    SET_BIT(ch->channels,COM_AUCTION);
    send_to_char_formatted("$CcYou are deaf to the auctioneer now.\n\r",ch);
    return;
  }

  if (!str_cmp(arg1,"help")) {
    send_to_char_formatted("$CCAuction Help---\n\r$CN",ch);
    send_to_char("   auction (no parameters)     -- will show the current item stats\n\r",ch);
    send_to_char("   auction <itemname> <minbid> -- auctions item starting at minbid\n\r",ch);
    send_to_char("   auction bid <bidamount>     -- Bids on the current item\n\r",ch);
    send_to_char("   auction listen              -- turns the channel on\n\r",ch);
    send_to_char("   auction deafen              -- turns the channel off\n\r",ch);
    send_to_char("   auction help                -- this list\n\r",ch);
    if (IS_IMMORTAL(ch))
      send_to_char(" * auction stop                -- stops the auction and gives item to you.\n\r",ch);
    return;
  }



  if (!str_cmp(arg1,"bid") )
    if (auction->item != NULL) {
      int newbet;

      if (auction->seller == ch) { /* seller cant be buyer */
	send_to_char_formatted("$CcSure Sure... try and bid on your own item!\n\r",ch);
	return;
      }

      if((auction->item->obj_flags.level > 10) &&
	 (GetMaxLevel(ch) < auction->item->obj_flags.level*4/5)) {
	cprintf(ch, "I'm afraid you're too low level to bid on this.\n");
	return;
      }

      /* make - perhaps - a bet now */
      if (arg[0] == '\0')
	{
	  send_to_char_formatted ("$CcBid how much?\n\r",ch);
	  return;
	}
#ifdef JANWORK
      newbet = parsebet(auction->bet, arg);

      printf ("Bid: %d\n\r",newbet);
#endif
      newbet = atoi(arg);

      if (newbet < (auction->bet + 100))
	{
	  send_to_char_formatted ("$CwYou must bid at least 100  coins over the current bid.\n\r",ch);
	  return;
	}

      if (newbet > GET_GOLD(ch))
	{
	  send_to_char_formatted ("$CwYou don't have that much money!\n\r",ch);
	  return;
	}

      /* the actual bet is OK! */

      /* return the gold to the last buyer, if one exists */
      if (auction->buyer != NULL)
	GET_GOLD(auction->buyer) += auction->bet;

      GET_GOLD(ch) -= newbet; /* substract the gold - important :) */
      auction->buyer = ch;
      auction->bet   = newbet;
      auction->going = 0;
      auction->pulse = PULSE_AUCTION; /* start the auction over again */

      sprintf (buf,"$CcA bid of$CY %d$Cc gold has been received on $CG%s.\n\r",newbet,
	       ss_data(auction->item->short_description));
      talk_auction (buf);
      return;
    }
    else {
      send_to_char_formatted ("$CcThere isn't anything being auctioned right now.\n\r",ch);
      return;
    }

  /* finally... */


  obj = get_obj_in_list_vis(ch, arg1, ch->carrying); /* does char have the item ? */
  arg = one_argument(arg,arg2);

  if (*arg2=='\0') minbet = 0;
  else
    minbet = atoi(arg2);

  if (minbet < 0) {
    send_to_char_formatted("$CYExqueeeeze me? $CcWhat kind of minimum bid is that?\n\r",ch);
    return;
  }


  if (obj == NULL) {
    send_to_char_formatted ("$CcEither you aren't carrying that, or you can't see it.\n\r",ch);
    return;
  }

  if ((IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) &&
      (TRUST(ch) < TRUST_IMP)) {
    send_to_char_formatted("$CcOh sure... Try to auction a cursed item!\n\r",ch);
    return;
  }

  if (auction->item == NULL)
    switch (GET_ITEM_TYPE(obj))
      {

      default:
	if (TRUST(ch) < TRUST_IMP) {
	  /* Don't let anyone but imms auction just anything */
	  send_to_char_formatted("$CRYou cannot auction that!\n\r",ch);
	  return;
	}
	/* else we fall through */
      case ITEM_WEAPON:
      case ITEM_ARMOR:
      case ITEM_STAFF:
      case ITEM_WAND:
      case ITEM_SCROLL:
	obj_from_char (obj);
	auction->item = obj;
	auction->bet = minbet;
	auction->buyer = NULL;
	auction->seller = ch;
	auction->pulse = PULSE_AUCTION;
	auction->going = 0;

	sprintf (buf, "$CcA new item has been received:$CG %s.\n\r", ss_data(obj->short_description));
	talk_auction (buf);
	sprintf(buf,"$CwMinimum bid on item:$CY %d$Cw coins\n\r",auction->bet);
	talk_auction(buf);

	return;

      } /* switch */
  else {
    send_to_char_formatted("$CcSomeone frowns and says 'One auction at a time please.'\n\r",ch);
    return;
  }
}

void vote_update (void)
{
  char buf[MAX_STRING_LENGTH];
  int i, max_votes;
  struct char_data *winner;
  int tie = 0;
  int rand_number;

  if (vote->vote_start) {
    vote->pulse = PULSE_AUCTION;
    switch (++vote->going) /* increase the going state */
      {
      case 1 : /* voting once */
	sprintf(buf, "$Cw[Countdown: 4] $CGPlease vote now - %d votes have already been placed\n\r$CN",
		vote->total_votes);
	talk_vote (buf);
	break;
      case 2 : /* voting twice */
	sprintf(buf, "$Cw[Countdown: 3] $CGPlease vote now - %d votes have already been placed\n\r$CN",
		vote->total_votes);
	talk_vote (buf);
	break;

      case 3 : /* voting thrice */
	sprintf(buf, "$Cw[Countdown: 2] $CGPlease vote now - %d votes have already been placed\n\r$CN",
		vote->total_votes);
	talk_vote (buf);
	break;


      case 4 : /* And the winner is .. */
	sprintf(buf, "$Cw[Countdown: 1] $CGAnd the winner is ..\n\r$CN");
	talk_vote (buf);
	break;

      case 5 : /* And the winner is .. */
	max_votes = -1;
	for(i = 0; i < vote->total_nominees; i++) {

	  if (vote->nominees[i]->votes == max_votes) {
	    // Gah, another person with the same amount of votes
	    tie++;
	  }
	  if (vote->nominees[i]->votes > max_votes) {
	    max_votes = vote->nominees[i]->votes;
	    winner = vote->nominees[i]->nominee;
	    tie = 0;
	  }
	}

	if (!winner) {
	  talk_vote ("$CBThe Winner has left TW, no reward can be given$CN\r\n");

	  clear_voting();
	  break;
	}

	// In case of a tie
	if (tie) {
	  talk_vote ("$CB***** We have a TIE! ****$CN\r\n");

	  rand_number = number(1,tie+1);

	  sprintf(buf, "tie = %i, rand_number = %i\r\n", tie, rand_number);
	  log_msg(buf);

	  tie = 1;
	  for(i = 0; i < vote->total_nominees; i++) {
	    if (vote->nominees[i]->votes == max_votes) {
	      if (rand_number == tie) {
		winner = vote->nominees[i]->nominee;
		tie++;
	      }
	    }
	  }

	  talk_vote ("$CwWinner chosen by random to be$CN\r\n");
	  sprintf(buf, "$CB**** WINNER $Cw%s $CBWINNER ****$CN\r\n", GET_REAL_NAME(winner));
	  talk_vote (buf);

	} else {
	  sprintf(buf, "$CB**** Winner $Cw%s $CBWinner ****$CN\n\r", GET_REAL_NAME(winner));
	  talk_vote (buf);
	}

	// The winner is awarded one hero point to use for hero list
	winner->points.hero_points++;

	// clear vote
	clear_voting();
	break;
      }
  }
}


/* the auction update - another very important part*/

void auction_update (void)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *shopkeeper;

  if (auction->item != NULL)
    {
      auction->pulse = PULSE_AUCTION;
	  switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
	      if (auction->bet > 0)
                sprintf (buf, "$CG%s:$Cc going %s for $CY%d.\n\r",
			 ss_data(auction->item->short_description),
			 ((auction->going == 1) ? "once" : "twice"), auction->bet);
	      else
                sprintf (buf, "$CG%s:$Cc going %s (no bid received yet).\n\r",
			 ss_data(auction->item->short_description),
			 ((auction->going == 1) ? "once" : "twice"));

	      talk_auction (buf);
	      break;

            case 3 : /* SOLD! */

	      if ((auction->bet > 0) && (auction->buyer)) /* FIXED */
		{
		  sprintf (buf, "$CG%s $Ccsold to %s for $CY%d.\n\r",
			   ss_data(auction->item->short_description),
			   IS_NPC(auction->buyer) ? ss_data(auction->buyer->player.short_descr) : ss_data(auction->buyer->player.name),
			   auction->bet);
		  talk_auction(buf);
		  obj_to_char (auction->item,auction->buyer);
		  act ("$CBThe auctioneer appears before you in a puff of smoke and hands you $p.\n\r",
		       TRUE,auction->buyer,auction->item,NULL,TO_CHAR);
		  act ("$CBThe auctioneer appears before $n, and hands $m $p\n\r",TRUE,
		       auction->buyer,auction->item,NULL,TO_ROOM);

		  talk_auction("$CCAuction Concluded.\n\r");
		  GET_GOLD(auction->seller) += auction->bet; /* give him the money */
		  /* if (GET_GOLD(auction->seller) > 2000000) {
		      send_to_char("You can't carry that much gold!!!  Your deity thanks you for the donation!\n\r",auction->seller);
		      GET_GOLD(auction->seller) = 2000000;
		   } */


		  auction->item = NULL; /* reset item */
		   auction->buyer = NULL;
		   auction->seller = NULL;

		}
	       else { /* not sold */

		  sprintf (buf, "$CcNo bids received for $CG%s$CN$Cc - Item Kept by Auction Inc.\n\r",
			   ss_data(auction->item->short_description));
		  talk_auction(buf);
#ifdef JANWORK
		  act ("$CBThe auctioneer appears before you to return $p to you.\n\r",TRUE,
		       auction->seller,auction->item,NULL,TO_CHAR);
		  act ("$CBThe auctioneer appears before $n to return $p to $m.\n\r",TRUE,
		       auction->seller,auction->item,NULL,TO_ROOM);
		  obj_to_char (auction->item,auction->seller);
		  if (auction->item)
		    extract_obj(auction->item); /* destroy the item */
#endif
		  if (!(shopkeeper = get_char("grocer")))
		    extract_obj(auction->item); /* dstroy the itme */
		  else
		    obj_to_char(auction->item,shopkeeper);
		  auction->item = NULL; /* clear auction */
		  auction->buyer=NULL;
		  auction->seller=NULL;

	       } /* else */

            } /* switch */
        } /* if */
} /* func */

void do_compare(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    struct obj_data *obj1;
    struct obj_data *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying)))
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content)
        {
            if ( !((obj2->obj_flags.type_flag == ITEM_WEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_FIREWEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_ARMOR) ||
                   (obj2->obj_flags.type_flag == ITEM_WORN))
            &&   CAN_SEE_OBJ(ch, obj2)
            &&   obj1->obj_flags.type_flag == obj2->obj_flags.type_flag
            && CAN_GET_OBJ(ch, obj2) )
                break;
        }

        if (!obj2)
        {
            send_to_char( "You aren't wearing anything comparable.\n\r", ch );
            return;
        }
    }
    else
    {
        if (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)))
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }
    }

    msg         = NULL;
    value1      = 0;
    value2      = 0;

    if (obj1 == obj2)
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->obj_flags.type_flag != obj2->obj_flags.type_flag )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch (obj1->obj_flags.type_flag)
        {
        default:
            msg = "You can't compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->obj_flags.value[0];
            value2 = obj2->obj_flags.value[0];
            break;

        case ITEM_WEAPON:
            value1 = obj1->obj_flags.value[1] + obj1->obj_flags.value[2];
            value2 = obj2->obj_flags.value[1] + obj2->obj_flags.value[2];
            break;
        }
    }

    if (!msg)
    {
             if (value1 == value2) msg = "$p and $P look about the same.";
        else if (value1  > value2) msg = "$p looks better than $P.";
        else                         msg = "$p looks worse than $P.";
    }

    act(msg, FALSE, ch, obj1, obj2, TO_CHAR);
    return;
}

