
#include "config.h"

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "utility.h"
#include "act.h"
#include "multiclass.h"
#include "util_str.h"
#include "find.h"
#include "ctype.h"
#include "proto.h"
#include "proto.h"
#include "ansi.h"
#include "channels.h"
#include "mobprog2.h"

void do_say(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[2*MAX_INPUT_LENGTH+20]="\0\0\0\0";
    
    if (apply_soundproof(ch))
	return;
    
    for (i = 0; *(argument + i) == ' '; i++);
    
   if (!*(argument + i))
     send_to_char("You must type something to say.\n\r", ch);
   else	{
      sprintf(buf,"$Cg$n$CN says $CY'%s'$CN",escape(argument + i));
      MOBTrigger = FALSE; /* for mobprogs */
      act(buf,FALSE,ch,0,0,TO_ROOM);
      if (IS_SET(ch->specials.flags, PLR_ECHO)) {
	 sprintf(buf,"$CgYou say$CN$Cy '%s'\n\r", argument + i);
	 send_to_char_formatted(buf, ch);
      }
      mprog_speech_trigger(argument, ch); /* trigger for things players say */
      mprog_speech_trigger2(argument, ch);
      rprog_speech_trigger(argument, ch);
   }
}


void tell_one(struct char_data *ch, struct char_data *vict,
	      const char *message, int groupflag)
{
  char buf[MAX_INPUT_LENGTH+100];
  char buf2[MAX_INPUT_LENGTH+100];
  char* who;
    
  if (ch == vict) {
    if(!groupflag)
      send_to_char("You try to tell yourself something.\n\r", ch);
    return;
  } else if (GET_POS(vict) == POSITION_SLEEPING)	{
    if(!groupflag)
      act("$E is asleep, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
  } else if (IS_SET(vict->specials.flags, PLR_NOTELL) &&
	     ((TRUST(ch) < TRUST_CREATOR) || (TRUST(ch) < TRUST(vict))))
  {
    if(!groupflag)
      send_to_char("Sorry, that player is ignoring tells and can't hear you.\n\r",ch);
	
    return;
  } else if (IS_NPC(vict) && !(vict->desc)) {
    if(!groupflag)
      send_to_char("No-one by that name here..\n\r", ch);
    return;
  } else if (!vict->desc) {
    if(!groupflag)
      send_to_char("They can't hear you, link dead.\n\r", ch);
    return;
  }
    
  if (check_soundproof(vict)) {
    if(!groupflag)
      send_to_char("They can't hear you.\n\r", ch);
    return;
  }
    
  if(IS_WRITING(vict))
  {
    if(!groupflag)
      send_to_char("That person is writing a message, and can't hear you.\n\r", ch);
    return;
  }

  who = (char *) ( groupflag ? "the group" : "you");
    
  if(CheckColor(vict)) {
    sprintf(buf,"%s%s ", ANSI_RED, GET_NAME(ch));
    if(IS_SET(ch->specials.mob_act, ACT_POLYSELF))
    {
      sprintf(buf2,"(%s) ", GET_NAME(real_character(ch)));
      strcat(buf,buf2);
    }
    sprintf(buf2,"%stells %s '%s'%s\n\r",
	    ANSI_GREEN, who, message, ANSI_NORMAL);
    strcat(buf,buf2);
  } else {
    sprintf(buf,"%s ", GET_NAME(ch));
    if(IS_SET(ch->specials.mob_act, ACT_POLYSELF))
    {
      sprintf(buf2,"(%s) ", GET_NAME(real_character(ch)));
      strcat(buf,buf2);
    }
    sprintf(buf2,"tells %s '%s'\n\r", who, message);
    strcat(buf,buf2);
  }
  send_to_char(buf, vict);
    
  if(IS_AFK(vict))
  {
    if(!groupflag)
      sprintf(buf, "Your message was delivered, but %s is AFK\n\r",
	      GET_NAME(vict));
    send_to_char(buf, ch);
  }
  else if (IS_SET(ch->specials.flags, PLR_ECHO) && !groupflag) { 
    sprintf(buf,"You tell %s '%s'\n\r", GET_NAME(vict), message);
    send_to_char(buf, ch);
  }
  if (IS_PC(ch)){
    if (vict->reply != NULL){
      vict->reply=NULL;
    }
    vict->reply=strdup(GET_IDENT(ch));
  } 
}

void do_tell_group(struct char_data *ch, char *message, int cmd)
{
    struct char_data *leader;
    struct follow_type *f;
    
    if(!IS_AFFECTED(ch, AFF_GROUP))
    {
	send_to_char("You should try to join a group first.\n", ch);
	return;
    }

    while(isspace(*message))
      message++;

    if(!*message)
    {
      send_to_char("Tell them what?\n\r", ch);
      return;
    }
    
    act("You tell the group '$T'",
	FALSE, ch, NULL, message, TO_CHAR);
    
    leader = ch->master;
    if(!leader)
	leader = ch;
    
    tell_one(ch, leader, message, 1);
    
    for(f = leader->followers; f; f = f->next)
	if(IS_AFFECTED(f->follower, AFF_GROUP))
	    tell_one(ch, f->follower, message, 1);
}

void do_reply(struct char_data *ch, char *argument, int cmd)
{    
    struct char_data *target;
    char buf[500];

    if (apply_soundproof(ch)){
      send_to_char("Its very quiet here, you can't make a sound.\n\r",ch);
      return;
    }

    if(IS_SET(ch->specials.flags, PLR_NOSHOUT)){
      send_to_char("You have been No Silenced from annoying people.\n\r",ch);
      return;
    }

    if (!*argument){
      send_to_char("Tell them what exactly ??\n\r",ch);
      return;
    }

    if (ch->reply == NULL){
      send_to_char("But you have no one to reply to!.\n\r",ch);
      return;
    }

    target=get_char(ch->reply);

    if (!target){
      sprintf(buf,"Player '%s' seems to have left the game.\n\r",ch->reply);
      send_to_char(buf,ch);
      ch->reply=NULL;
      return;
    }

    if (!(target = get_char_vis(ch, ch->reply))) {
      send_to_char("No-one by that name here..\n\r", ch);
      return;
    }
      
    tell_one(ch, target, argument+1, 0);
    return;
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH+20];
    
    if (apply_soundproof(ch))
	return;
    
    half_chop(argument,name,message);
    
    if(!*name || !*message) {
	send_to_char("Who do you wish to tell what??\n\r", ch);
	return;
    } else if(!strcmp(name, "group")) {
	do_tell_group(ch, message, -1);
	return;
    } else if (!(vict = get_char_vis(ch, name))) {
/*      if (ch->reply){ 
           if ((strcasecmp(name,ch->reply)==0) && (vict=find_player_in_world(name))){
              tell_one(ch, vict, message, 0);
              return;
           }
        } */
	send_to_char("No-one by that name here..\n\r", ch);
        return;
    } else {
	tell_one(ch, vict, message, 0);
	return;
    }
}



void do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH],*mess;
    
    if (apply_soundproof(ch))
	return;
    
    half_chop(argument,name,message);
    
    if(!*name || !*message)
	send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\n\r", ch);
    else if (vict == ch) {
	act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
	send_to_char
	    ("You can't seem to get your mouth close enough to your ear...\n\r",ch);
    }  else    {
	if (check_soundproof(vict))
	    return;
	
	mess = escape(message);

	if(CheckColor(vict))
	    sprintf(buf,"%s$n%s whispers to you, '%s'%s", ANSI_RED, ANSI_GREEN,
		    mess, ANSI_NORMAL);
	else
	    sprintf(buf,"$n whispers to you, '%s'",mess);
	act(buf, FALSE, ch, 0, vict, TO_VICT);

	if(IS_AFK(vict))
	{
	    sprintf(buf, "Your message was delivered, but %s is AFK\n\r",
	        GET_NAME(vict));
	    send_to_char(buf, ch);
	}
	else if (IS_SET(ch->specials.flags, PLR_ECHO)) {
    sprintf(buf,"You whisper to %s, '%s'\n\r", GET_NAME(vict), mess);
	    send_to_char(buf, ch);
	}
	act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH],*mess;
    
    if (apply_soundproof(ch))
	return;
    
    half_chop(argument,name,message);
    
    if(!*name || !*message)
	send_to_char("Who do you want to ask something.. and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\n\r", ch);
    else if (vict == ch)	{
	act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
	send_to_char("You think about it for a while...\n\r", ch);
    }  else	{
	if (check_soundproof(vict))
	    return;
	
	mess = escape(message);

	if(CheckColor(vict))
	    sprintf(buf,"%s$n%s asks you '%s'%s", ANSI_RED, ANSI_GREEN,
		    mess, ANSI_NORMAL);
	else
	    sprintf(buf,"$n asks you '%s'", mess);
	act(buf, FALSE, ch, 0, vict, TO_VICT);

	if(IS_AFK(vict))
	{
	    sprintf(buf, "Your message was delivered, but %s is AFK\n\r",
		GET_NAME(vict));
	    send_to_char(buf, ch);
	}
	else if(IS_SET(ch->specials.flags, PLR_ECHO))
	{
	    sprintf(buf,"You ask %s, '%s'\n\r", GET_NAME(vict), mess);
	    send_to_char(buf, ch);
	}
	act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
    }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0, *pen = 0;
    char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
    buf[MAX_STRING_LENGTH];
    
    argument_interpreter(argument, papername, penname);
    
    if (!ch->desc)
	return;
    
    if (!*papername)  /* nothing was delivered */    {   
	send_to_char("write (on) papername (with) penname.\n\r", ch);
	return;
    }
    
    if (!*penname) {
	send_to_char("write (on) papername (with) penname.\n\r", ch);
	return;
    }
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))	{
	sprintf(buf, "You have no %s.\n\r", papername);
	send_to_char(buf, ch);
	return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))	{
	sprintf(buf, "You have no %s.\n\r", papername);
	send_to_char(buf, ch);
	return;
    }
    
    /* ok.. now let's see what kind of stuff we've found */
    if (pen->obj_flags.type_flag != ITEM_PEN) {
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
    } else if (paper->obj_flags.type_flag != ITEM_NOTE)    {
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    } else if (*OBJ_ACTION(paper)) {
	send_to_char("There's something written on it already.\n\r", ch);
	return;
    } else {
	/* we can write - hooray! */
	send_to_char
	    ("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
	act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
	ch->desc->sstr = &paper->action_description;
	ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}

void do_more(struct char_data *ch, char *argument, int cmd)
{
    ch->specials.flags ^= PLR_CONTINUOUS;

    if(!IS_SET(ch->specials.flags, PLR_CONTINUOUS))
	send_to_char("Some output will now be paged.\n\r", ch);
    else
	send_to_char("No output will be paged.\n\r", ch);
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */


void talk_auction (char *argument)
{
    struct descriptor_data *desc;
    char buf[MAX_STRING_LENGTH];

    sprintf (buf,"$CC[AUCTION]:$CN %s", argument);

    EACH_DESCRIPTOR(d_iter,desc)
    {
      if ((desc->connected == CON_PLYNG) && 
	  !IS_SET(desc->character->channels,COM_AUCTION))
	send_to_char_formatted(buf,desc->character);
    }
    END_ITER(d_iter);
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */


void talk_vote (char *argument)
{
    struct descriptor_data *desc;
    char buf[MAX_STRING_LENGTH];

    sprintf (buf,"$CC[VOTE]: $CN %s", argument);

    EACH_DESCRIPTOR(d_iter,desc)
    {
      if ((desc->connected == CON_PLYNG) && 
	  !IS_SET(desc->character->channels,COM_VOTE))
	send_to_char_formatted(buf,desc->character);
    }
    END_ITER(d_iter);
}

void do_rpsay(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[2*MAX_INPUT_LENGTH+20]="\0\0\0\0";

    if (apply_soundproof(ch))
        return;

    for (i = 0; *(argument + i) == ' '; i++);

   if (!*(argument + i))
     send_to_char("You must type something to say in character.\n\r", ch);
   else {
      sprintf(buf,"$Cg$n$CN says $Cm(IC) $CY'%s'$CN",escape(argument + i));
      MOBTrigger = FALSE; /* for mobprogs */
      act(buf,FALSE,ch,0,0,TO_ROOM);
      if (IS_SET(ch->specials.flags, PLR_ECHO)) {
         sprintf(buf,"$CgYou say (IC)$CN$Cy '%s'\n\r", argument + i);
         send_to_char_formatted(buf, ch);
      }
      mprog_speech_trigger(argument, ch); /* trigger for things players say */
      mprog_speech_trigger2(argument, ch);
      rprog_speech_trigger(argument, ch);
   }
}

