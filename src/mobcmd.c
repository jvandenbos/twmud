/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "find.h"
#include "interpreter.h"
#include "comm.h"
#include "spells.h"
#include "proto.h"
#include "utility.h"
#include "modify.h"
#include "act.h"
#include "util_str.h"
#include "track.h"
#include "config.h"
#include "constants.h"
#include "opinion.h"
#include "varfunc.h"

/* extern struct index_data *mob_index; */

/*
 * Local functions.
 */

/* attaches mob's name and vnum to msg and sends it to script_log */
void mob_log(struct char_data *mob, char *msg)
{
  char buf[MAX_INPUT_LENGTH + 100];


  sprintf(buf, "Mob (%s, VNum %d): %s",
	    GET_NAME(mob), GET_MOB_VNUM(mob), msg);
  log_msg(buf);
}


/* string prefix routine */

bool str_prefix(const char *astr, const char *bstr)
{
  if (!astr) {
    log_msg("Strn_cmp: null astr.");
    return TRUE;
  }
  if (!bstr) {
    log_msg("Strn_cmp: null astr.");
    return TRUE;
  }
  for(; *astr; astr++, bstr++) {
    if(LOWER(*astr) != LOWER(*bstr)) return TRUE;
  }
  return FALSE;
}

/*
 * The following command now validates whether its a true mob performing an mpcommand 
 */


int mpcando(struct char_data *ch) {

  if (!IS_NPC(ch)) {
    send_to_char("Huh?!?\r\n", ch);
    return FALSE;
  }
  
  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("Sorry, you're not in control of your faculties....\n\r",ch);
    return FALSE;
  }

  if (IS_PC(ch)) { /* now takes into account polies */
    send_to_char("Uh oh... you can't do that... you're not a monster!\n\r",ch);
    return FALSE;
  }

  return TRUE; /* ok to exec command */
}

  



/* mob commands */
/* let the mobile kill safely */

ACMD(do_mpkill) {
  char argum[MAX_INPUT_LENGTH];
  struct char_data *victim;
  

  if (!mpcando(ch))
    return;
  
  one_argument(arg, argum); /* argum is the local variable (not param) */

  if (!*argum) {
    mob_log(ch, "mkill called with no argument");
    return;
  }

  if (!(victim = get_char_room_vis(ch, argum))) {
    mob_log(ch, "mkill: victim no in room");
    return;
  }

  if (victim == ch) {
    mob_log(ch, "mkill: victim is self");
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
    mob_log(ch, "mkill: charmed mob attacking master");
    return;
  }

  if (GET_POS(ch) == POSITION_FIGHTING) {	
    mob_log(ch, "mkill: already fighting");
    return;
  }

  hit(ch, victim, TYPE_UNDEFINED);
  return;
}


/*
 * lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them
 */
ACMD(do_mpjunk)
{
  char argum[MAX_INPUT_LENGTH];
  int pos;
  struct obj_data *obj;
  struct obj_data *obj_next;
  
  if (!mpcando(ch))
    return;

  one_argument(arg, argum);
   skip_spaces(argum);
  
  if (!*argum) {
    mob_log(ch, "mjunk called with no argument");
    return;
  }
  
  if (find_all_dots(argum) != FIND_INDIV) {
    if ((obj=get_object_in_equip_vis(ch,argum,ch->equipment,&pos))!= NULL) {
      unequip_char(ch, pos);
      extract_obj(obj);
      return;
    }
    if ((obj = get_obj_in_list_vis(ch,argum, ch->carrying)) != NULL )
      extract_obj(obj);
    return;
  } else {
    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      if (argum[3] == '\0' || isname(argum+4, ss_data(obj->name))) {
	extract_obj(obj);
      }
    }
    while ((obj=get_object_in_equip_vis(ch,argum,ch->equipment,&pos)) != NULL) {
      unequip_char(ch, pos);
      extract_obj(obj);
    }   
  }

  return;
}


/*
 * lets the mobile load an item or mobile.  All items
 * are loaded into inventory, unless it is NO-TAKE. 
 */
ACMD(do_mpload) {
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int r_num, number = 0;
  struct char_data *mob;
  struct obj_data *object;
  
  if (!mpcando(ch))
    return;
  
  if (IS_AFFECTED(ch, AFF_CHARM))
    return;
  
  two_arguments(arg, arg1, arg2);
  
  if (!*arg1 || !*arg2 || !is_number(arg2) || ((number = atoi(arg2)) < 0)) {
    mob_log(ch, "mpload: bad syntax");
    return;
  }

  if (is_abbrev(arg1, "mobile")) {
    /* following code copied from do_make god function much safer */

    number = real_mobile(number);
    if (number < 0 || number > top_of_mobt) {
      mob_log(ch,"mmload: bad mob vnum");
      return;
    }

    if (!(mob = make_mobile(number, REAL))) { /* do the magic */
      mob_log(ch,"Internal Error - Unable to create mob");
      return;
    }
    char_to_room(mob, ch->in_room);
  }
  else if (is_abbrev(arg1, "object")) { /* uhh guess it wasn't a mob */
    if ((r_num = real_object(number)) < 0) {
      mob_log(ch, "mpload: bad object vnum");
      return;
    }
    
   if (((obj_index[r_num].number>=obj_index[number].limit)
	|| (obj_index[number].limit < 0))
       && (TRUST(ch) < TRUST_IMP)) {
     mob_log(ch,"mpload: tried to create a restricted object!");
     return;
   }
   if (!(object = make_object(r_num, REAL))) {
     mob_log(ch,"mpload: Internal error creating object!");
     return;
   }
   
   obj_to_char(object, ch);
   /* might want to think about putting an ACT in here */
    /* Note:  I changed this to always put the object in the mobs
       inventory b/c i think that someone writing mobprogs should
       be aware enough of what a mob can/can't carry! if not... 
       slay em! */


  }
  else
    mob_log(ch, "mload: bad type");
}


/*
 * lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room.  It can purge
 *  itself, but this will be the last command it does.
 */

ACMD(do_mppurge)
{
  char argum[MAX_INPUT_LENGTH];
  struct char_data *victim;
  struct obj_data  *obj;
  
  if (!mpcando(ch)) 
    return;
  
  one_argument(arg, argum);
  skip_spaces(argum);
  
  if (!*argum) {
    /* 'purge' */
    struct char_data *vnext;
    struct obj_data  *onext;
    
    
    /* code to purge all mobs from a room */
    /* copied from do_purge for safety reasons */

    for (victim = real_roomp(ch->in_room)->people; victim; victim = vnext) {
      vnext = victim->next_in_room;
      if (!IS_PC(victim) && victim != ch)
	purge_char(victim);
    }


    /* now purge all da objects from da room */
    for (obj = real_roomp(ch->in_room)->contents; obj; obj = onext) {
      onext = obj->next_content;
      extract_obj(obj);
    }
  } else {
    if ((victim = get_char_room_vis( ch, argum)) == NULL) {
      if ((obj = get_obj_vis(ch, argum))) {
	extract_obj(obj);
      } else { 
	mob_log(ch, "mpurge: bad argument");
      }
    } else {
      if (!IS_NPC(victim)) {
	mob_log(ch, "mpurge: purging a PC");
	return;
      }
  
      /* Mob should not purge himself */
//      if (victim != ch)
	purge_char(victim);
    }
  }
}


/* lets the mobile goto any location it wishes that is not private */
ACMD(do_mpgoto)  {

  char buf[MAX_INPUT_LENGTH];
  room_num loc_nr, location;
  struct char_data *target_mob, *v = NULL;
  struct obj_data *target_obj;
  
  if (!mpcando(ch))
    return;

  only_argument(arg,buf);
  
  if (!*buf)	{
    send_to_char_formatted("$CRYou must supply a room number or a name or type goto home.\n\r", ch);
    return;
  }
  
  if (isdigit(*buf) && !strchr(buf, '.')) {
    loc_nr = atoi(buf);
    if (NULL==real_roomp(loc_nr)) {
      if (TRUST(ch) < TRUST_CREATOR || loc_nr < NOWHERE) {
	send_to_char_formatted("$CRNo room exists with that number.\n\r", ch);
	return;
      }
      else {
#if HASH
#else
	if (loc_nr < WORLD_SIZE) {
#endif
	  send_to_char_formatted("$CRYou form order out of chaos.\n\r", ch);
	  CreateOneRoom(loc_nr);
	  
#if HASH
#else
	} else {
	  send_to_char_formatted("$CRSorry, that room # is too large.\n\r", ch);
	  return;
	}
#endif
      }
    }
    /* a location has been found. */
    location = loc_nr;
  }
  else if (is_abbrev(buf,"home")) {
    location = GET_HOME(ch);
    if(!location)
      location = GET_HOME(ch) = 20;
    if(!real_roomp(location)) {
      send_to_char_formatted("$CRBetter create your home first bozo...\n\r", ch);
      return;
    }
  }
  else if ((target_mob = get_char_vis_world(ch, buf, NULL))) {
    location = target_mob->in_room;
  }
  else if ((target_obj=get_obj_vis_world(ch, buf, NULL))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("The object is not available.\n\r", ch);
      send_to_char("Try where #.object to nail its room number.\n\r", ch);
      return;
    }
  }
  else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }
  
  if (!real_roomp(location))  {
    log_msg("Massive error in do_mpgoto. Everyone Off NOW.");
    return;
  }
  
  if (IS_SET(real_roomp(location)->room_flags, IMMORT_RM) && !IS_GOD(ch))
  {
    send_to_char("Who do you think you are, an Immortal?\n\r", ch);
    return;
  }

  if (IS_SET(real_roomp(location)->room_flags, GOD_RM) &&
      (TRUST(ch) < TRUST_LORD)) {
    send_to_char("Who do you think you are, a God?\n\r", ch);
    return;
  }
  
  if (IS_SET(ch->specials.flags, PLR_STEALTH) && (ch->invis_level)
      /* PAC --  && (ch->invis_level < 55) I really hated this */ ) {
    for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
      if ((ch != v) && (TRUST(v) >= ch->invis_level)) {
	act("$n steps into the shadows.", 
	      FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else /*if (ch->invis_level < 55)*/ {
    act("$Cg$n$Cb steps into the shadows.", 
	FALSE, ch, 0, 0, TO_ROOM);
  }
  
  if (ch->specials.fighting)
    stop_fighting(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  
  if (IS_SET(ch->specials.flags, PLR_STEALTH) && (ch->invis_level > 50)
      /* PAC && (ch->invis_level < 55) I really hated this */) {
    for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
      if ((ch != v) && (TRUST(v) >= ch->invis_level)) {
	act("$Cg$n$Cb slowly fades into existence.", 
	    FALSE, ch, 0,v,TO_VICT);	
      }
    }
  } else /*if (ch->invis_level < 55)*/ {
    act("$Cg$n$Cb slowly fades into existence.", 
	FALSE, ch, 0, 0,TO_ROOM);	
  }
  do_look(ch, "",15);
  
}


/* lets the mobile do a command at another location. Very useful */
ACMD(do_mpat)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
  room_num loc_nr, location, original_loc;
  struct char_data *target_mob;
  struct obj_data *target_obj;

  if (!mpcando(ch)) 
    return;
  
  half_chop(arg,loc_str,command);

  if (!*loc_str) {
    send_to_char_formatted("$CRYou must supply a room number of a name.\n\r",ch);
    return;
  }

  if (isdigit(*loc_str) && !strchr(loc_str, '.'))
    {
	loc_nr = atoi(loc_str);
	if (NULL==real_roomp(loc_nr)) {
	    send_to_char_formatted("$CRNo room exists with that number.\n\r", ch);
	    return;
	}
	location = loc_nr;
    } else if ((target_mob = get_char_vis(ch, loc_str))) {
	if (IS_NPC(target_mob))
	    location = target_mob->in_room;
	else if (!CAN_SEE(ch,target_mob)) {
	    send_to_char_formatted("$CRNo such creature or object around.\n\r", ch);
	    return;
	}
	else location = target_mob->in_room;
    } else if ((target_obj=get_obj_vis_world(ch, loc_str, NULL)))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else
	{
	    send_to_char_formatted("$CRhe object is not available.\n\r", ch);
	    return;
	}
    else
    {
	send_to_char_formatted("$CRNo such creature or object around.\n\r", ch);
	return;
    }
  
    /* check for peeking */
    if (IS_SET(real_roomp(location)->room_flags, GOD_RM) &&
	(TRUST(ch) < TRUST_LORD)) {
	send_to_char_formatted(
"$CRIf they wanted you nosing around it wouldn't be GOD_ROOM, now would it?\n\r",
		     ch);
	return;
    }

    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);

    command_interpreter(ch, command, 1);
  
    /* check if the guy's still there */
    for (target_mob = real_roomp(location)->people; target_mob; target_mob =
	 target_mob->next_in_room)
      if (ch == target_mob) {
	char_from_room(ch);
	char_to_room(ch, original_loc);
	break;
      }
    
  return;
}

/*
 * lets the mobile transfer people.  the all argument transfers
 * everyone in the current room to the specified location
 */
ACMD(do_mpteleport)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  room_num target;
  struct char_data *vict, *next_ch;
  
  if (!mpcando(ch))
    return;

  arg = two_arguments(arg, arg1, arg2);
  
  if (!*arg1 || !*arg2) {
    mob_log(ch, "mteleport: bad syntax");
    return;
  }

  target = find_target_room(ch, arg2);

  if (target == NOWHERE)
    mob_log(ch, "mteleport target is an invalid room");

  else if (!str_cmp(arg1, "all")) {
    if (target == IN_ROOM(ch)) {
      mob_log(ch, "mteleport all target is itself");
      return;
    }

    for (vict = real_roomp(ch->in_room)->people; vict; vict = next_ch) {
      next_ch = vict->next_in_room;
      if (!IS_IMMORTAL(vict)) {
	char_from_room(vict);
	char_to_room(vict, target);
      }
    }
  }

  else {
    if ((vict = get_char_room_vis(ch, arg1))) {
      if (!IS_IMMORTAL(vict)) {
	char_from_room(vict);
	char_to_room(vict, target);
      }
    }

    else
      mob_log(ch, "mteleport: no target found");
  }
}


/*
 * lets the mobile force someone to do something.  must be mortal level
 * and the all argument only affects those in the room with the mobile
 */
ACMD(do_mpforce) {

  struct descriptor_data *i;
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH+40];

  if (!mpcando(ch))
    return;

  half_chop(arg, name, to_force);

  if (!*name || !*to_force)
    send_to_char("Who do you wish to force to do what?\n\r",ch);
  else if (*(to_force) == '!') {
    send_to_char("You can't force someone to do that!\n\r",ch);
    return;
  }
  else if (str_cmp("all",name)) {
    if (!(vict = get_char_vis(ch, name)) ||
	(!IS_NPC(vict) && (!CAN_SEE(ch,vict)))) 
      send_to_char("No-one by that name here..\n\r", ch);
    else {
      if (IS_GOD(vict)) /* attempt to force a god */
	send_to_char("Oh no you don't!!\n\r", ch);
      else {
	sprintf(buf, "$n has forced you to '%s'.", to_force);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	send_to_char("Ok.\n\r", ch);
	command_interpreter(vict, to_force, 1);
      }
    }
  }
  else {			/* force all */
      EACH_DESCRIPTOR(d_iter, i)
	{
	if (i->character != ch && !i->connected) {
	  vict = i->character;
	  if (TRUST(vict) || (vict->in_room != ch->in_room))
	      /* null statement/no action */ ;
	  else {
	    sprintf(buf, "$n has forced you to '%s'.", to_force);
		    act(buf, FALSE, ch, 0, vict, TO_VICT);
		    command_interpreter(vict, to_force, 1);
		}
	    }
	}
	END_ITER(d_iter);
	send_to_char("Ok.\n\r", ch);
  }
  return;
}
  
/* change the target's gold */
ACMD(do_mpgold)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH], amount[MAX_INPUT_LENGTH];

  if (!mpcando(ch))
     return;
 
  two_arguments(arg, name, amount);

  if (!*name || !*amount) {
     mob_log(ch, "mgold: too few arguments");
     return;
  }

  if (!(victim = get_char_vis(ch, name))) {
     mob_log(ch, "mgold: target not found");
     return;
  }  
  
  if (is_number(amount)) {
	 GET_GOLD(victim) = MAX(GET_GOLD(victim) + atoi(amount), 0);
  } else {
    mob_log(ch, "mgold: amount is not a number");
    return;
  }
 
}

/* increases the target's exp */
ACMD(do_mpexp)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH], amount[MAX_INPUT_LENGTH];

  if (!mpcando(ch))
    return;

  two_arguments(arg, name, amount);

  if (!*name || !*amount) {
    mob_log(ch, "mexp: too few arguments");
    return;
  }

  if (!(victim = get_char_vis(ch, name))) {
    mob_log(ch, "mexp: target not found");
    return;
  }
  
  gain_exp(victim, atoi(amount));      
}


ACMD(do_mpstat) {
  char target[MAX_INPUT_LENGTH];
  int mobvirtual, mobreal;
  MPROG_DATA *mprg;
  char buf[MAX_STRING_LENGTH];
  Function *mprg2;

  if (!IS_IMMORTAL(ch))
    return; /* your not a taco, a burrito or ANYTHING */

  only_argument(arg, target); /* get the mobs name */

  if (isdigit(*target)) 
    mobvirtual = atoi(target);
  else
    mobvirtual = -1;

  if (mobvirtual < 0) {
    for (mobreal = 0; mobreal <=top_of_mobt ; mobreal++)
      if (isname(target,mob_index[mobreal].name))
	break;
  } 
  else 
    mobreal = real_mobile(mobvirtual);

  if (mobreal < 0 || mobreal > top_of_mobt) {
    send_to_char_formatted("$CRThere is no such monster.\n\r",ch);
    return;
  }

  sprintf(buf,"-------------------------------------------------------------\n\r"
	  "Mob: [%s]  vnum: [%d] realnum:[%d]\n\r"
	  "-----------------------------------------------------------------\n\r",
	  mob_index[mobreal].name,mob_index[mobreal].virt,
	  mobreal);
  page_string(ch->desc,buf,1);

  if (!mob_index[mobreal].mobprogs) {
    page_string(ch->desc,"Has no mobprograms attached to it\n\r\n\r"
		"-----------------------------------------------------------------\n\r", 0);
  }

  for (mprg = mob_index[mobreal].mobprogs; mprg; mprg = mprg->next) {
    sprintf(buf, "-------------------------------------------------------------\n\r"
	    "Type: [%s]  Arguments: [%s]\n\r"
	    "------------------------------------------------\n\r",
	    mprog_type_to_name(mprg->type),mprg->arglist);
    page_string(ch->desc,buf,1);
    sprintf(buf,mprg->comlist,1);
    strcat(buf, "\r");
    page_string(ch->desc,buf,1);
    page_string(ch->desc, "-----------------------------------------------------------------\n\r", 0);
  }
   
  if(!mob_index[mobreal].mobprogs2) {
     cprintf(ch,"Has no mobprograms2 attached to it\n\r\n\r");
     cprintf(ch,"--------------------------------------\n\r");
  } else {
     cprintf(ch,"Mobprogs2:\n\r\n\r");
     
     for(mprg2 = mob_index[mobreal].mobprogs2; mprg2; mprg2 = mprg2->next) {
	sprintf(buf,"Name: [%-20s], Num Args: [%-2d], Args: [%s]\n\r",
		mprg2->name, mprg2->NumArgs, mprg2->argbuf);
	send_to_char(buf, ch);
	cprintf(ch,"--------------------------------------------------\n\r");
	
	sprintf(buf,"%s\n\r", mprg2->code);
	send_to_char(buf, ch);
	cprintf(ch,"--------------------------------------------------\n\r");
     }
  }
   
  return;
}


ACMD(do_mpechoat) { 
   char vict_name[MAX_INPUT_LENGTH],*victimstr;
   char *echostring;
   struct char_data *victim;
   
   if (!mpcando(ch))
     return;
   
   echostring  = one_argument(arg, vict_name);
   echostring = skip_spaces(echostring);
   victimstr = skip_spaces(vict_name);
   
   if (!*victimstr) {
      mob_log(ch,"mpechoat called with no argument! doh!");
      return;
   }
   
   if (!(victim = get_char_room_vis(ch, victimstr))) {
      mob_log(ch, "mpecho: victim does not exist");
      return;
   }
      
   act(echostring, FALSE, ch, NULL, victim, TO_VICT);
   return;
}

ACMD(do_mptransfer) {
  struct char_data *victim;
  room_num target;
  char buf[MAX_INPUT_LENGTH];
  
  if (!mpcando(ch))
    return;
  
  one_argument(arg,buf);


  only_argument(arg,buf);

  if (!*buf) {
    send_to_char("Who do you wish to transfer?\n\r",ch);
    return;
  }
  else {
    if (!(victim = get_char_vis_world(ch,buf,NULL)) ||
	(!IS_NPC(victim) && !CAN_SEE(ch,victim)))
      send_to_char("No-one by that name around.\n\r",ch);
    else {
      act("$Cg$n$CB disapears with a loud bang!",
	  FALSE, victim, 0, 0, TO_ROOM);
      target = ch->in_room;
      char_from_room(victim);
      char_to_room(victim,target);
      act("$Cg$n$CB arrives with a blinding flash of green light.",
	  FALSE, victim, 0, 0, TO_ROOM);
      act("$Cg$n$CB has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      do_look(victim,"",15);
      send_to_char("Ok.\n\r",ch);
    }
  }
  return;

      

}

/* the new MPSEND series of functions - Min
 * I found the old ones very confusing and hard to understand
 * the set is now:
 * 
 * mpsend character string   -- sends msg only to this char (Act)
 * mpsend_except character string  - everyone in room BUT character
 * mpsend_around string - send string to all characters AROUND room
 * 
 * Hope this helps :)
 * 
 */

ACMD(do_mpsend) {
   char send_type[MAX_INPUT_LENGTH];
   char send_string[MAX_INPUT_LENGTH];
   char send_victim[MAX_INPUT_LENGTH];
   
   int door;
   struct room_data *rp, *orp;
   struct char_data *target;
   
   
   if (!mpcando(ch))
     return;

   if (!arg) return;
   
   if (*arg == '\0') {
      log_msg("MPSEND with no arguments");
      return;
   }
   
   send_type[0] = '\0';
   send_victim[0] = '\0';
   send_string[0] = '\0';
   
   arg = one_argument(arg, send_type); /* what type? */

   if (is_abbrev(skip_spaces(send_type),"around") ||
       is_abbrev(skip_spaces(send_type),"here")) 
      strcat(send_string,skip_spaces(arg));
   
   if (is_abbrev(skip_spaces(send_type),"here")) {
      act(send_string,TRUE,ch,NULL,NULL,TO_ROOM);
      return;
   }
   
   
   if (is_abbrev(skip_spaces(send_type),"around")) { /* send AROUND the room */
      rp = real_roomp(ch->in_room);
      if (!rp) {
	 log_msg("Mpsend around: bad mobroom!");
	 return;
      }
      
      for (door = 0; door <= (NUM_OF_DIRS-1); door++) {
	 if (rp->dir_option[door] &&
	     (orp = real_roomp(rp->dir_option[door]->to_room))) 
	    for (target = orp->people; target; target = target->next_in_room) 
	       if (IS_PC(target) && !IS_AFFECTED(target, AFF_SILENCE) &&
		   (GET_POS(target) > POSITION_SLEEPING)) 
		 send_to_char_formatted(send_string,target);
      }
      
      return;
   } /* end of mpsend around */
   
   if (strlen(arg) < 1) {
      log_msg("mpsend called with too few arguments");
      return;
   }
   
   arg = one_argument(arg, send_victim); /* read the victim */
   
   if (strlen(arg) < 1) {
      log_msg("mpsend called with too few arguments");
      return;
   }

   strcat(send_string,skip_spaces(arg)); /* this is the string to send */
   
   target = get_char_room(skip_spaces(send_victim), ch->in_room);
   if (!target) {
      log_msg("mprog: not a valid target");
      return;
   }
   
   if (is_abbrev(send_type,"char")) {
      act(send_string,TRUE,ch, NULL, target,TO_VICT);
      return;
   }
   
   if (is_abbrev(send_type,"room")) {
      act(send_string,TRUE, ch, NULL, target, TO_NOTVICT);
      return;
   }
   
   return;
}

char *parse_paren(char *pt, char *bpt) {
   int count;
   
   pt++;
   count=0;
   
   while(*pt) {
      if(*pt == '(') count++;
      if(*pt != ')')
	 *bpt++ = *pt;
      else {
	 if(!count--)
	   break;
	 else
	   *bpt++ = *pt;
      }
      pt++;
   }
   *bpt='\0';

   return pt;
}

static int MVCount=0;
static char *moprs = "+-*/&^|=";

double MVarEval(struct char_data *ch, char *lhsp, char opr, char *rhsp) {
   double lhs=0, rhs=0;
   int count=0;
   char buf[MAX_INPUT_LENGTH];
   char nlhs[MAX_INPUT_LENGTH];
   char buf2[MAX_INPUT_LENGTH];
   char nopr='\0';
   char *nlp=nlhs, *rp=rhsp, *auxp=buf2;
   
   MVCount++;
   if(MVCount > 40) {
      MVCount--;
      send_to_char("ERROR: MVarEval has recurssively executed WAY too many times...aborting\n\r", ch);
      return 0;
   }
   
//   sprintf(buf, "eval %i: %s, %c, %s", MVCount, lhsp, opr, rhsp);
//   log_msg(buf);
   
   //if the left side is null
   if(!*lhsp) {
      lhs = MVarEval(ch, "0", '+', rhsp);
      MVCount --;
      return lhs;
   }
   
   if(is_number(lhsp))
     //if the left side is a simple number
     lhs = atof(lhsp);
   else
     //if the left side is an expr, send it off as the right side, so I don't
     //have do duplicate the code for left & right :p
     lhs = MVarEval(ch, "0", '+', lhsp);


   //if the right is null, just return what we got for the left side.
   if(!*rhsp) {
      MVCount--;
      return lhs;
   }
   
   if(is_number(rhsp)) {
     //if the right side is a pure number
     rhs=atof(rhsp);
     goto DoFinalEval;
   } else {
      /*Now to break up the rhs...always going to be the icky one...      
        Our plan of attack is this - parse the new lhs, get opr, and use
        what's left for rhs. If a parenthese occurs in the formula,
        it simply goes to what would logically be the closing parenthese.
        nlhs is the lhs for the recursive call, nopr is the operator,
        rp is being used as the new rhs.
      */
      
      //if there's a opening ( then go until the next logical )
      if(*rp == '(') {
	 rp=parse_paren(rp, nlp);
	 
	 if(*rp)
	   rp++;
	 else {
	   sprintf(buf, "No ending parentheses found in %s\n\r", rhsp);
	   send_to_char(buf, ch);
	 }
      } else if(!strncasecmp(rp, "sin", 3)) {
	 rp+=3;
	 if(*rp == '(') {
	    rp=parse_paren(rp, auxp);
	    if(*rp)
	      rp++;
	    else {
	       sprintf(buf, "No ending parentheses found in SIN function: %s\n\r", buf2);
	       send_to_char(buf, ch);
	       MVCount--;
	       return lhs;
	    }
	 } else {
	    send_to_char("SIN must be followed by a pair of ()'s\n\r", ch);
	    MVCount--;
	    return lhs;
	 }
	 MVCount--;
	 return sin(MVarEval(ch, "0", '+', buf2));
      } else if(!strncasecmp(rp, "cos", 3)) {
	 rp+=3;
	 if(*rp == '(') {
	    rp=parse_paren(rp, auxp);
	    if(*rp)
	      rp++;
	    else {
	       sprintf(buf, "No ending parenthese found in COS function: %s\n\r", buf2);
	       send_to_char(buf, ch);
	       MVCount--;
               return lhs;
	    }
	 } else {
	    send_to_char("COS must be followed by a pair of ()'s\n\r", ch);
	    MVCount--;
	    return lhs;
	 }
	 MVCount--;
	 return cos(MVarEval(ch, "0", '+', buf2));
      } else {
	 //Just go until you get to the first operator
	 while(*rp) {
	    if(!index(moprs, *rp))
	      *nlp++ = *rp;
	    else
	      break;
	    rp++;
	 }
	 *nlp = '\0';
	 if(!*rp) {
	   if(!strcasecmp(nlhs, "pi")) {
	      MVCount--;
	      return 3.1415926535;
	   }
	   if(!strcasecmp(nlhs, "e")) {
	      MVCount--;
	      return 2.7182818284;
	   }
	   if(!is_number(nlhs)) {
  	      sprintf(buf, "Non-Number or opr: %s\n\r", rhsp);
  	      send_to_char(buf, ch);
	      MVCount--;
	      return lhs;
	   } else {
 	      MVCount--;
	      return atof(nlhs);
	   }
	 }
      }
   }

   //if there was no operator
   if(!*rp) {
      rhs = MVarEval(ch, "0", '+', nlhs);
      goto DoFinalEval;
   } else
      //get the operator
      nopr = *rp++;


   //If this is a conditional statement. i.e. ((cond)?(if T):(if F))
   if(nopr == '?') {
      count = 0;
      while(*rp) {
	 if(*rp == '(') count++;
	 if((*rp != ':')) {
	    if(*rp == ')') count--;
  	    *auxp++ = *rp;
	 }
	 else {
	    if(count==0)
	      break;
	    else
	      *auxp++ = *rp;
	 }
	 rp++;
      }
      if(*rp)
        rp++;
      
      *auxp = '\0';
      
      if(!*rp) {
	 //q123
	 sprintf(buf, "Syntax error in MVarEval: No ':' with '?' operator.\n\r");
	 send_to_char(buf, ch);
	 return lhs;
      }
      
      if(MVarEval(ch, "0",'+',nlhs)) {
	 //if the conditional returned true
	 lhs = MVarEval(ch, "0", '+', buf2);
	 MVCount--;
	 return lhs;
      } else {
	 //if the conditional returned false
	 lhs = MVarEval(ch, "0", '+', rp);
	 MVCount--;
	 return lhs;
      }
   }
   
   //not a conditional, and has a left expr, and a right expr.
   rhs = MVarEval(ch, nlhs, nopr, rp);

DoFinalEval:
//   sprintf(buf, "Final results %i: %f%c%f", MVCount, lhs, opr, rhs);
//   log_msg(buf);
   
   MVCount--;
   //do the basic math
   switch(opr) {
    case '+':
      return lhs + rhs;
    case '-':
      return lhs - rhs;
    case '*':
      return lhs * rhs;
    case '/':
      if(rhs == 0) rhs=0.00000000001;
      return lhs / rhs;
    case '&':
      return (long)lhs & (long)rhs;
    case '|':
      return (long)lhs | (long)rhs;
    case '^':
      return pow(lhs, rhs);
    case '%':
      return (long)lhs % (long)rhs;
    case '=':
      return lhs == rhs;
    default:
      sprintf(buf, "Illegal operator in MVarEval: %c\n\r", opr);
      send_to_char(buf, ch);
   }
   
   return 0;
}

double MVarMath(struct char_data *ch, char *expr) {
   char buf[MAX_INPUT_LENGTH];
   char *ptr=buf, *ptr2=expr;
   float Res;
   
   //Remove all spaces from the forumla
   while(*ptr2) {
     if(*ptr2 != ' ')
       *ptr++ = *ptr2;
     ptr2++;
   }
   *ptr = '\0';

   Res = MVarEval(ch, "0", '+', buf);
   
   return Res;
}

ACMD(do_mpset) {
   struct char_data *vict;
   char buf[MAX_INPUT_LENGTH];
   char vname[MAX_INPUT_LENGTH], command[MAX_INPUT_LENGTH], vbuf[MAX_INPUT_LENGTH];
   char *vbufp;
   int var=0;

   memset(vname, '\0', MAX_INPUT_LENGTH);
   memset(vbuf, '\0', MAX_INPUT_LENGTH);
   
   if(!mpcando(ch)) return;

   while(*arg == ' ') arg++;

   if(!*arg) {
     sprintf(buf, "%s had no argument in mpset", GET_NAME(ch));
     log_msg(buf, LOG_MPROG);
     return;
   }
   
   arg=one_argument(arg, command);
   arg=one_argument(arg, vname);
   only_argument(arg, vbuf);

   if(!(vict = get_char_vis(ch, vname))) {
     sprintf(buf, "%s couldn't find %s in mpset", GET_NAME(ch), vname);
     log_msg(buf, LOG_MPROG);
     return;
   }

   if(!strcmp(command, "fighting")) {
      DeleteHatreds(vict);
      
      if(vict->specials.fighting) {
	 stop_fighting(vict->specials.fighting);
	 DeleteHatreds(vict->specials.fighting);
         stop_fighting(vict);
      }
   }
   
   if(!strcmp(command, "state")) {
     vbufp = vbuf;
     switch(*vbufp) {
      case '+':
        var=atoi(++vbufp);
        vict->player.mpstate += var;
	break;
      case '-':
        var=atoi(++vbufp);
        vict->player.mpstate -= var;
        break;
      case '*':
        var=atoi(++vbufp);
        vict->player.mpstate *= var;
        break;
      case '/':
        var=atoi(++vbufp);
        vict->player.mpstate /= var;
        break;
      default:
        var=atoi(vbufp);
        vict->player.mpstate = var;
	break;
     }
   }

   if(!strcmp(command, "qnum")) {
     var = atoi(vbuf);
     vict->player.qnum = var;
   }
   
   if(!strcmp(command, "var")) {
      vbufp=one_argument(vbuf, vname)+1;
      Variable *vtmp;
      char *vnamep=vname+1;
      long vn;
      
      if(vname[0] != '~') {
	 log_msg("Illegal name format in mpset: no ~ at variable name");
	 return;
      }

      for(vtmp=vict->player.vars;vtmp;vtmp=vtmp->next)
	 if(!strcasecmp(vtmp->name, vnamep))
	    break;
      
      if(!vtmp) {
	 vtmp = new Variable;
	 vtmp->SetName(vnamep);
	 vtmp->next = vict->player.vars;
      }
      
      if((strlen(vbufp) == 1) || (*vbufp != '%')) {
	if(is_number(vbufp)) {
  	  vtmp->SetData(atol(vbufp));
	} else {
	  vtmp->SetData(vbufp);
	}
        return;
      }
      if(*vbufp == '%' && *(vbufp+1) == '%') {
	vtmp->SetData(vbufp+1);
	return;
      }

      //format for var math = %<expr> (always use vprint for variables)
      
      vn=(long)MVarMath(ch, vbufp+1);
      vtmp->SetData(vn);
      return;
   }
}

ACMD(do_mptrack) {
   char name[MAX_INPUT_LENGTH];
   struct char_data *vict;
   struct obj_data *obj;
   
   if(!mpcando(ch)) return;
   
   while(*arg == ' ') arg++;
   
   only_argument(arg, name);

   if(!(vict = get_char_vis(ch, name)) && (cmd == 1)) {
      send_to_char("You are unable to find traces of one.\n\r", ch);
      return;
   }

   if(!(obj = get_obj_vis(ch, name)) && (cmd == 2)) {
      send_to_char("You are unable to find traces of one.\n\r", ch);
      return;
   }
   
   if(ch->hunt_info)
     path_kill(ch->hunt_info);
   
   if(cmd == 1)
     ch->hunt_info = path_to_char(ch->in_room, vict, MAX_ROOMS, HUNT_THRU_DOORS);
   else if(cmd == 2)
     ch->hunt_info = path_to_obj(ch->in_room, obj, MAX_ROOMS, HUNT_THRU_DOORS);

   ch->persist = MAX_ROOMS;
}

ACMD(do_mptrackgo) {
   char name[MAX_INPUT_LENGTH];
   int dir;

   if(!mpcando(ch)) return;
   
   while(*arg == ' ') arg++;
   
   only_argument(arg, name);
   if(!ch->persist)
     ch->persist = MAX_ROOMS;

//   for(a=0;a<3;a++)
      if((dir = track(ch)) == -1) {
	 if(cmd==532)
           do_mptrack(ch, arg, 1);
	 else if(cmd==533)
	   do_mptrack(ch, arg, 2);
      }
      else {
//         strcpy(buf, dirs[dir]);
//         command_interpreter(ch, buf, 0);
      }

}

ACMD(do_mpquestup) {
   if(!mpcando(ch)) return;
   
   do_questup(ch, arg, cmd);
}
