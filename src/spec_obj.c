#include "config.h"

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "db.h"
#include "find.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "util_str.h"
#include "utility.h"
#include "handler.h"
#include "multiclass.h"
#include "act.h"
#include "hero.h"
#include "cmdtab.h"
#include "spell_util.h"
#include "fight.h"


/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(soap)
{
    struct char_data *t;
    struct obj_data *obj = (struct obj_data *) me;
    char dummy[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
  
    if (cmd != 172) return(FALSE);
  
    if (obj != ch->equipment[HOLD]) return(FALSE);

    arg = one_argument(arg, dummy);
    if(!(*dummy)) return(FALSE);
    only_argument(arg, name);
    if(!(*name)) return(FALSE);
  
    if (!(t = get_char_room_vis(ch, name))) return(FALSE);

    if (affected_by_spell(t,SPELL_WEB)) {
	affect_from_char(t,SPELL_WEB);
	act("$n washes some webbing off $N with $p.",TRUE,ch,obj,t,TO_ROOM);
	act("You wash some webbing off $N with $p.",FALSE,ch,obj,t,TO_CHAR);
    }
    else {
	act("$n gives $N a good lathering with $p.",TRUE,ch,obj,t,TO_ROOM);
	act("You give $N a good lathering with $p.",FALSE,ch,obj,t,TO_CHAR);
    }
  
    obj->obj_flags.value[0]--;
    if(!obj->obj_flags.value[0]) {
	act("That used up $p.",FALSE,ch,obj,t,TO_CHAR);
	extract_obj(obj);
    }
    return FALSE;
}  


SPECIAL(nodrop)
{
    struct char_data *t = NULL;
    struct obj_data *i, *obj;
    char buf[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
    char vict_name[MAX_INPUT_LENGTH], *name;
    bool do_all;
    int j, num;
    spec_proc_func knowdrop;
  
    switch(cmd) {
    case 10:			/* Get */
    case 60:			/* Drop */
    case 72:			/* Give */
    case 156:			/* Steal */
	break;
    default:
	return(FALSE);
    }
  
    knowdrop = nodrop;

    arg = one_argument(arg, obj_name);
    if (!*obj_name) return(FALSE);

    obj = 0x0;
    do_all = FALSE;

    if(!(strncmp(obj_name,"all",3))) {
	do_all = TRUE;
	num = IS_CARRYING_N(ch);
    }
    else {
	strcpy(buf,obj_name);
	name = buf;
	if(!(num = get_number(&name))) return(FALSE);
    }

    /* Look in the room first, in get case */
    if(cmd == 10)
	for (i=real_roomp(ch->in_room)->contents,j=1;i&&(j<=num);i=i->next_content)
	    if (i->item_number>=0)
		if (do_all || isname(name, OBJ_NAME(i)))
		    if(do_all || j == num) {
			if (obj_index[i->item_number].func == knowdrop) {
			    obj = i;
			    break;
			}
		    }
		    else ++j;
  
    /* Check the character's inventory for give, drop, steal. */
    if(!obj)
	/* Don't bother with get anymore */
	if(cmd == 10) return(FALSE);
    for (i = ch->carrying,j=1;i&&(j<=num);i=i->next_content)
	if (i->item_number>=0)
	    if (do_all || isname(name, OBJ_NAME(i)))
		if(do_all || j == num) {
		    if (obj_index[i->item_number].func == knowdrop) {
			obj = i;
			break;
		    }
		    else if(!do_all) return(FALSE);
		}
		else ++j;
  
    /* Musta been something else */
    if(!obj) return(FALSE);
  
    if((cmd == 72) || (cmd == 156)) {
	only_argument(arg, vict_name);
	if((!*vict_name) || (!(t = get_char_room_vis(ch, vict_name))))
	    return(FALSE);
    }
  
    switch(cmd) {
    
    case 10:
	if(!IS_GOD(ch)) {
	    act("$p disintegrates when you try to pick it up!",
		FALSE, ch, obj, 0, TO_CHAR);
	    act("$n tries to get $p, but it disintegrates in his hand!",
		FALSE, ch, obj, 0, TO_ROOM);
	    extract_obj(obj);
	    if(do_all) return(FALSE);
	    else return(TRUE);
	}
	else return(FALSE);

    case 60:
	if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
	    act("You drop $p to the ground, and it shatters!",
		FALSE, ch, obj, 0, TO_CHAR);
	    act("$n drops $p, and it shatters!", FALSE, ch, obj, 0, TO_ROOM);
	    i = make_object(30, VIRTUAL);
	    sprintf(buf, "Scraps from %s lie in a pile here.",
		    OBJ_SHORT(obj));
	    ss_free(i->description);
	    i->description = ss_make(buf);
	    obj_to_room(i, ch->in_room);
	    obj_from_char(obj);
	    extract_obj(obj);
	    if(do_all) return(FALSE);
	    else return(TRUE);
	}
	else return(FALSE);
    
    case 72:
	if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
	    if(!IS_GOD(ch)) {
		act("You try to give $p to $N, but it vanishes!",
		    FALSE, ch, obj, t, TO_CHAR);
		act("$N tries to give $p to you, but it fades away!",
		    FALSE, t, obj, ch, TO_CHAR);
		act("As $n tries to give $p to $N, it vanishes!",
		    FALSE, ch, obj, t, TO_ROOM);
		extract_obj(obj);
		if(do_all) return(FALSE);
		else return(TRUE);
	    }
	    else return(FALSE);
	}
	else return(FALSE);
    
    case 156:			/* Steal */
	if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
	    act("You cannot seem to steal $p from $N.",
		FALSE, ch, obj, t, TO_CHAR);
	    act("$N tried to steal something from you!",FALSE,t,obj,ch,TO_CHAR);
	    act("$N tried to steal something from $n!",FALSE,t,obj,ch,TO_ROOM);
	    return(TRUE);
	}
	else return(FALSE);
    
    default:
	return(FALSE);
    }
  
    return(FALSE);
}


#define HERO_EXP_RESTORE 10000000
SPECIAL(obj_hero_ring)
{
  struct char_data *t = NULL;
  struct obj_data *obj, *i;
  char buf[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  char vict_name[MAX_INPUT_LENGTH], *name;
  bool do_all;
  int j, num, action;
  spec_proc_func hero_ring;
  
  switch(cmd) {
  case 10:			/* Get */
/*  case 60:	*/		/* Drop */
  case 72:			/* Give */
  case 156:			/* Steal */
  case 172:			/* Use */
    break;
  default:
    return(FALSE);
  }
  
  hero_ring = obj_hero_ring;

  arg = one_argument(arg, obj_name);
  if (!*obj_name) return(FALSE);

  obj = 0x0;
  do_all = FALSE;

  if(!(strncmp(obj_name,"all",3))) {
    do_all = TRUE;
    num = IS_CARRYING_N(ch);
  }
  else {
    strcpy(buf,obj_name);
    name = buf;
    if(!(num = get_number(&name))) return(FALSE);
  }

  /* Look in the room first, in get case */
  if(cmd == 10)
    for (i=real_roomp(ch->in_room)->contents,j=1;i&&(j<=num);i=i->next_content)
      if (i->item_number>=0)
	if (do_all || isname(name, OBJ_NAME(i)))
	  if(do_all || j == num) {
	    if (obj_index[i->item_number].func == hero_ring) {
	      obj = i;
	      break;
	    }
	  }
	  else ++j;
  
  /* Check the character's inventory for give, drop, steal. */
  if(!obj)
    /* Don't bother with get anymore */
    if(cmd == 10) return(FALSE);
  for (i = ch->carrying,j=1;i&&(j<=num);i=i->next_content)
    if (i->item_number>=0)
      if (do_all || isname(name, OBJ_NAME(i)))
	if(do_all || j == num) {
	  if (obj_index[i->item_number].func == hero_ring) {
	    obj = i;
	    break;
	  }
	  else if(!do_all) return(FALSE);
	}
	else ++j;
  
  /* Musta been something else unless cmd==USE of course.*/
  if( (!obj) && (cmd!=172) ) return(FALSE);
  
  if((cmd == 72) || (cmd == 156)) {
    only_argument(arg, vict_name);
    if((!*vict_name) || (!(t = get_char_room_vis(ch, vict_name))))
      return(FALSE);
  }
  
  switch(cmd) {
    
  case 10:
    if(!IS_GOD(ch)) {
      act("$p disintegrates when you try to pick it up!",
	  FALSE, ch, obj, 0, TO_CHAR);
      act("$n tries to get $p, but it disintegrates in his hand!",
	  FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
      if(do_all) return(FALSE);
      else return(TRUE);
    }
    else return(FALSE);

/*  case 60:
    if (!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) 
    {
      act("You drop $p to the ground, and it shatters!",
	  FALSE, ch, obj, 0, TO_CHAR);
      act("$n drops $p, and it shatters!", FALSE, ch, obj, 0, TO_ROOM);
      i = make_object(30, VIRTUAL);
      sprintf(buf, "Scraps from %s lie in a pile here.",
	      OBJ_SHORT(obj));
      ss_free(i->description);
      i->description = ss_make(buf);
      obj_to_room(i, ch->in_room);
      obj_from_char(obj);
      extract_obj(obj);
      if(do_all) return(FALSE);
      else return(TRUE);
    }
    else return(FALSE);
  */  
  case 72:
    if (!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP))
    {
      if(!IS_GOD(ch)) {
	act("You try to give $p to $N, but it vanishes!",
	    FALSE, ch, obj, t, TO_CHAR);
	act("$N tries to give $p to you, but it fades away!",
	    FALSE, t, obj, ch, TO_CHAR);
	act("As $n tries to give $p to $N, it vanishes!",
	    FALSE, ch, obj, t, TO_ROOM);
	extract_obj(obj);
	if(do_all) return(FALSE);
	else return(TRUE);
      }
      else return(FALSE);
    }
    else return(FALSE);
    
  case 156:			/* Steal */
    if(!IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP)) {
      act("You cannot seem to steal $p from $N.",
	  FALSE, ch, obj, t, TO_CHAR);
      act("$N tried to steal something from you!",FALSE,t,obj,ch,TO_CHAR);
      act("$N tried to steal something from $n!",FALSE,t,obj,ch,TO_ROOM);
      return(TRUE);
    }
    else return(FALSE);

  case 172:
    /*
     *  If the ring is not being held, abort.
     */
	
    if(!(obj=ch->equipment[HOLD])) return(FALSE);
    if(obj_index[obj->item_number].func != hero_ring) return(FALSE);

    if(GET_EXP(ch)<HERO_EXP_RESTORE)
      action=0;
    else
      action=1;

    if (IS_IMMORTAL(ch))
      action=1;

    switch(action)
    {
    case 0:
      /*
       * Not enough exp to power the ring.
       */
      act("As you hold the $o up, nothing happens...",TRUE,ch,obj,NULL,TO_CHAR);
      act("As $N holds $S $o up, $s gets an unhappy look upon $m face.",TRUE,ch,obj,ch,TO_ROOM);
      break;

    case 1:
      /*
       *  act(you twist the ring or somethign like that)
       *  restore the wearer to full hps/mana/move
       */
      if(!IS_IMMORTAL(ch))
	GET_EXP(ch)-=HERO_EXP_RESTORE;
      act("As you hold the $o up towards the heavens,",
	  TRUE,ch,obj,NULL,TO_CHAR);
      act("you feel drained of energy but then...",
	  TRUE,ch,obj,NULL,TO_CHAR);
      act("you feel a rush of power flow through you.",
	  TRUE,ch,obj,NULL,TO_CHAR);
      act("As $N holds $S $o up towards the sky, an aura of power forms about $m.",TRUE,ch,obj,ch,TO_ROOM);

      GET_MANA(ch)=GET_MAX_MANA(ch);
      GET_HIT(ch)=GET_MAX_HIT(ch);
      GET_MOVE(ch)=GET_MAX_MOVE(ch);
      add_char_to_hero_list(ch);
      break;
    }
    return(TRUE);
  default:
    return(FALSE);
  }
}


#define MIRROR_TARG_ROOM 30036

SPECIAL(mirror)
{
    char buf[MAX_INPUT_LENGTH];
    struct room_data *room;
    int old_room;

    if (cmd!=7 && cmd!=15 && cmd!=166)
        return (FALSE);

    one_argument(arg, buf);

    if ((!*buf) || (strcmp(buf, "mirror")))
	return (FALSE);

    if ((room = real_roomp(MIRROR_TARG_ROOM)) == NULL)
    {
        sprintf(buf, "illegal room %d for mirror target.",
		MIRROR_TARG_ROOM);
	log_msg(buf);
	send_to_char("This mirror is experiencing technical difficulties.\n\r",
		     ch);
	return TRUE;
    }

    if (cmd==7) /* enter */
    {
	act("$n steps into the mirror.", 0, ch, 0, 0, TO_ROOM);
	send_to_char("You step through the mirror.\n\r", ch);
    }
    else /* look, examine */
    {
	send_to_char("As you gaze into the mirror, you become mesmerized by "
		     "the swirling lights.\n\r", ch);
	send_to_char("An image begins to emerge as you look more closely"
		     "...\n\r\n\r", ch);
	act("$n gazes into the swirling lights and color of the mirror.",
	    0, ch, 0, 0, TO_ROOM);
    }

    old_room = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, room->number);
    do_look(ch, "", 0);
    act("$n appears from a cloud of swirling mist.",
	TRUE, ch, NULL, NULL, TO_ROOM);
    if (cmd != 7)
    {
        char_from_room(ch);
	char_to_room(ch, old_room);
    }

    return (TRUE);
}

#define STONES_TARG_ROOM 30006

SPECIAL(stones)
{
    char buf[MAX_INPUT_LENGTH];
    struct room_data *room;
    int old_room;

    if (cmd!=7 && cmd!=15 && cmd!=166)
        return (FALSE);

    one_argument(arg, buf);

    if ((!*buf) || (strcmp(buf, "pool")))
	return (FALSE);

    if ((room = real_roomp(STONES_TARG_ROOM)) == NULL)
    {
        sprintf(buf, "illegal room %d for stones target.",
		STONES_TARG_ROOM);
	log_msg(buf);
	send_to_char("This stone pool is experiencing technical difficulties.\n\r",
		     ch);
	return TRUE;
    }

    if (cmd==7) /* enter */
    {
	act("$n wades into the pool of stones.", 0, ch, 0, 0, TO_ROOM);
	send_to_char("You wade into the pool of stones.\n\r", ch);
    }
    else /* look, examine */
    {
	send_to_char("As you gaze into the pool, you become mesmerized by "
		     "the swirling lights.\n\r", ch);
	send_to_char("An image begins to emerge as you look more closely"
		     "...\n\r\n\r", ch);
	act("$n gazes into the swirling lights and colors of the stone pool.",
	    0, ch, 0, 0, TO_ROOM);
    }

    old_room = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, room->number);
    do_look(ch, "", 0);
    act("$n wades his way out of the pool of stones.",
	TRUE, ch, NULL, NULL, TO_ROOM);
    if (cmd != 7)
    {
        char_from_room(ch);
	char_to_room(ch, old_room);
    }

    return (TRUE);
}


SPECIAL(genie_lamp)
{
    struct obj_data *lamp = (struct obj_data *) me;
    struct char_data *genie;
    char name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    struct follow_type *f;

    if (!cmd || (cmd != CMD_RUB && cmd != CMD_REMOVE))
	return FALSE;

    strcpy(argument, arg);
    one_argument(arg, name);

    if (lamp != get_obj_in_equip_vis(ch, name))
	return FALSE;

    switch (cmd)
    {
    case CMD_RUB:
	/* check if there is already a genie */
	for (f = ch->followers; f; f = f->next)
	    if (IS_MOB(f->follower) &&
		mob_index[f->follower->nr].virt == lamp->obj_flags.value[3])
	    {
		act("You polish $p.", FALSE, ch, lamp, NULL, TO_CHAR);
		act("$N polishes your home.",
		    FALSE, ch, lamp, f->follower, TO_VICT);
		strcpy(buf, "What is your bidding, my master?");
		do_say(f->follower, buf, CMD_SAY);

		return TRUE;
	    }

	if (lamp->obj_flags.value[2] <= 0)
	{
	    act("You polish $p.", FALSE, ch, lamp, NULL, TO_CHAR);
	    act("$n polishes $p to a bright shine.",
		TRUE, ch, lamp, NULL, TO_ROOM);
	}

	else
	{
	    lamp->obj_flags.value[2]--;
	    
	    act("You rub $p.", FALSE, ch, lamp, NULL, TO_CHAR);
	    act("$n rubs $p.", TRUE, ch, lamp, NULL, TO_ROOM);

	    if (!(genie = make_mobile(lamp->obj_flags.value[3], VIRTUAL)))
	    {
		send_to_char("This lamp messed up.  Please report.\n\r", ch);
		return TRUE;
	    }
	    
	    char_to_room(genie, ch->in_room);
	    act("\n\rA stream of smoke billows forth from $p...\n\r"
		"The smoke becomes more dense, collating into $n.\n\r",
		FALSE, genie, lamp, 0, TO_ROOM);

	    add_follower(genie, ch, 0);
	    SET_BIT(AFF_FLAGS(genie), AFF_CHARM);

	    strcpy(buf, GET_REAL_NAME(ch));
	    do_action(genie, buf, CMD_BOW);
	    strcpy(buf, "Your wish is my command, Master");
	    do_say(genie, buf, CMD_SAY);
	}

	return TRUE;

    case CMD_REMOVE:
	do_remove(ch, argument, CMD_REMOVE);

	if (lamp != get_obj_in_equip_vis(ch, name))
	{
	    /* lamp has been removed */

	    /* check for genie */
	    for (f = ch->followers; f; f = f->next)
		if (IS_MOB(f->follower) &&
		   mob_index[f->follower->nr].virt == lamp->obj_flags.value[3])
		{
		    act("$N dissapates into a cloud of smoke, which is sucked "
			"into $p.",
			FALSE, ch, lamp, f->follower, TO_CHAR|TO_ROOM);
		    act("$n commands you back into your lamp.",
			FALSE, ch, lamp, f->follower, TO_VICT);

		    if (IS_FIGHTING(f->follower))
			stop_fighting(f->follower);

		    extract_char(f->follower);
		    break;
		}
	}

	return TRUE;
    }

    return FALSE;
}

SPECIAL(portal)   
{
   struct obj_data *portal = (struct obj_data *) me;   
   char buf[MAX_INPUT_LENGTH];   
   struct room_data *room;   
   int old_room;   
       
   if (cmd!=7 && cmd!=15 && cmd!=166)
     return (FALSE);   
        
   one_argument(arg, buf);   
        
   if ((!*buf) || (strcmp(buf, "portal")))   
     return (FALSE);   
        
   if ((room = real_roomp(portal->obj_flags.value[2])) == NULL) {
      sprintf(buf, "illegal room %d for portal target.",   
              portal->obj_flags.value[2]);   
      log_msg(buf);   
      send_to_char("This portal is experiencing technical difficulties.\n\r",   
			                            ch);   
      return TRUE;   
   }
        
   if (cmd==7) /* enter */   
   {   
      act("$n steps into the portal.", 0, ch, 0, 0, TO_ROOM);   
      send_to_char("You step through the portal.\n\r", ch);   
   }   
   else /* look, examine */   
   {   
      send_to_char("As you gaze into the portal, you become mesmerized by "   
	           "the swirling lights.\n\r", ch);   
      send_to_char("An image begins to emerge as you look more closely"   
                   "...\n\r\n\r", ch);   
      act("$n gazes into the swirling lights and color of the portal.",   
          0, ch, 0, 0, TO_ROOM);   
   }
        
   old_room = ch->in_room;   
   char_from_room(ch);   
   char_to_room(ch, room->number);   
   do_look(ch, "", 0);   
   act("$n appears from a cloud of swirling mist.",   
       TRUE, ch, NULL, NULL, TO_ROOM);   
   if (cmd != 7)   
   {   
      char_from_room(ch);   
      char_to_room(ch, old_room);   
   }   
       
   return (TRUE);   
} 
