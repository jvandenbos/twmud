#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#if USE_unistd
#include <unistd.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "act.h"
#include "utility.h"
#include "recept.h"
#include "constants.h"
#include "multiclass.h"
#include "fight.h"
#include "modify.h"
#include "spec.h"
#include "spelltab.h"
#include "cmdtab.h"
#include "util_str.h"
#include "find.h"
#include "interpreter.h"
#include "state.h"
#include "ringlog.h"
#include "ansi.h"
#include "spell_util.h"
#include "proto.h"
#include "signals.h"
#include "varfunc.h"
#include "trackchar.h"

/* for the TEMPLE command
   remember to change this if you renumber or so
   */

room_num TEMPLE_ROOMS[] = { 7, /* the portal of creation */
     3001, /* the OTHER portal... doh!!! */
		       13505, /* altar in NT */
		       16127, /* priest in gypsies */
		       8026, /* village square in orun */
		       3857, /* aucan elders gathering place */
		       2913, /* small hut opposite healer in vikings */
		       6235, /* town square in souk */
		       3577, /* cloud giant shaman */
		       0 /* null terminator */
};




/* external declarations we shouldn't have to make... */
/*
 #ifdef __cplusplus
 extern "C"
 {
 #endif
     int strcasecmp(const char*, const char*);
     int strncasecmp(const char*, const char*, int n);
 #ifdef __cplusplus
 }
 #endif
*/

/* forward declarations */
void FailSteal(struct char_data* ch, struct char_data* victim, int percent,
	       char* mVict, char* mOthers);
bool CheckSpaceFree(void);

void do_gain(struct char_data *ch, char *argument, int cmd)
{

}

void do_guard(struct char_data *ch, char *argument, int cmd)
{
  if(IS_PC(ch))
  {
    send_to_char("Sorry. you can't just put your brain on autopilot!\n\r",ch);
    return;
  }

  for(;isspace(*argument); argument++);

  if (!*argument) {
    if (IS_SET(ch->specials.mob_act, ACT_GUARDIAN)) {
      act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You relax.\n\r",ch);
      REMOVE_BIT(ch->specials.mob_act, ACT_GUARDIAN);
    } else {
      SET_BIT(ch->specials.mob_act, ACT_GUARDIAN);
      act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
      act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      send_to_char("You snap to attention\n\r", ch);
    }
  } else {
     if (!str_cmp(argument,"on")) {
      if (!IS_SET(ch->specials.mob_act, ACT_GUARDIAN)) {
         SET_BIT(ch->specials.mob_act, ACT_GUARDIAN);
         act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
         act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
         send_to_char("You snap to attention\n\r", ch);
       }
     } else if (!str_cmp(argument,"off")) {
       if (IS_SET(ch->specials.mob_act, ACT_GUARDIAN)) {
         act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char("You relax.\n\r",ch);
         REMOVE_BIT(ch->specials.mob_act, ACT_GUARDIAN);
       }
     }
  }

  return;
}


void do_beep(struct char_data *ch, char *argument, int cmd)
{
  if (ch->desc)
    send_to_char("\007", ch);
}


void do_cls(struct char_data *ch, char *argument, int cmd)
{
  if (ch->desc)
    send_to_char("\033[H\033[J", ch);
}


void do_junk(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH], buf[100], newarg[MAX_INPUT_LENGTH];
  struct obj_data *tmp_object;
  int num, p, count;

/*
 *   get object name & verify
 */

  only_argument(argument, arg);
  if (*arg) {
    if (getall(arg,newarg)) {
      num = -1;
      strcpy(arg,newarg);
    } else if ((p = getabunch(arg,newarg))) {
      num = p;
      strcpy(arg,newarg);
    } else {
      num = 1;
    }

    count = 0;
    while (num != 0) {
      tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
      if (tmp_object) {
	if (IS_OBJ_STAT(tmp_object,ITEM_NODROP)) {
	   send_to_char
		("You can't let go of it, it must be CURSED!\n\r", ch);
	   return;
	}
	obj_from_char(tmp_object);
	extract_obj(tmp_object);
	if (num > 0) num--;
	count++;
      } else {
	if (count > 1) {
	  sprintf(buf, "You junk %s (%d).\n\r", arg, count);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.\n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else if (count == 1) {
	  sprintf(buf, "You junk %s \n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.\n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else {
	  send_to_char("You don't have anything like that\n\r", ch);
	}
	return;
      }
    }
  } else {
    send_to_char("Junk what?", ch);
    return;
  }
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	return;
}

void do_kil(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You have to type kill - no less, to kill!\n\r", ch);
  return;
}

void do_hi(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You have to type hit - no less, to hit!\n\r", ch);
  return;
}

void do_title(struct char_data *ch, char *argument, int cmd)
{
   char buf[512];

   if (IS_NPC(ch) || !ch->desc)
       return;

  for(;isspace(*argument); argument++)  ;

  if (*argument) {

    if (strlen(argument) > 78) {
      send_to_char("Line too long, truncated\n", ch);
      *(argument + 79) = '\0';
    }
    sprintf(buf, "Your title has been set to : <%s>\n\r", argument);
    send_to_char_formatted(buf, ch);

    ss_free(ch->player.title);
    ch->player.title = ss_make(argument);
  }

}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM))
	return;

    if (GET_POS(ch) == POSITION_FIGHTING) {
	send_to_char("No way! You are fighting.\n\r", ch);
	return;
    }

    if (GET_POS(ch) < POSITION_STUNNED) {
	send_to_char("You die before your time!\n\r", ch);
	die(ch);
	return;
    }

    if (auction-> item != NULL && ((ch == auction->buyer) ||
				   (ch == auction->seller))) {
      send_to_char("Wait till you have sold/bought the item on auction.\n\r",ch);
      return;
    }

    act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);

//    Solaar: Too much drama caused by this function.
//    StuffToRoom(ch, ch->in_room);

//    spevent_remove_all_char(ch);
//    drop_unrented(ch);
    if(SaveChar(ch, AUTO_RENT, TRUE))
    {
	send_to_char("!!! SAVE FAILED !!!\n\r", ch);
	return;
    }

    TrackingSystem.UpdateCharFull(ch);
    extract_char(ch);
}



void do_save(struct char_data *ch, char *argument, int cmd)
{

    /*
      if you change this to just if (IS_NPC(ch)) it will fix the poly save
      for sure, but item duplication is still prevalent unless I find someway
      to autorent link dead polies OR to save them with with their current
      stuff...
      I think I fixed it! if the original body, with no stuff is in room 3,
      then dont save him/her!!
      */

    if(!IS_PC(ch) || (ch->in_room == 3))
	return;

    if(cmd)
	send_to_char("Saving.\n\r", ch);

    if(SaveChar(ch, AUTO_RENT, TRUE) && cmd) // --Mnemosync
	send_to_char("!!!SAVE FAILED!!!\n\r", ch);
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}

void ShowPracs(char_data *, int, int);

void do_practice(struct char_data *ch, char *arg, int cmd)
{
  int index, level;
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  for ( ; isspace(*arg); arg++);

  switch(*arg) {
  case 'M': case 'm':
    {
      index=MAGE_LEVEL_IND;
      sprintf(buf, "Mage");
      break;
    }
  case 'C': case 'c':
    {
      index=CLERIC_LEVEL_IND;
      sprintf(buf, "Cleric");
      break;
    }
  case 'W': case 'w':
    {
      index=WARRIOR_LEVEL_IND;
      sprintf(buf, "Warrior");
      break;
    }
  case 'T': case 't':
    {
      index=THIEF_LEVEL_IND;
      sprintf(buf, "Thief");
      break;
    }
  case 'K': case 'k':
    {
      index=PALADIN_LEVEL_IND;
      sprintf(buf, "Knight");
      break;
    }
  case 'D': case 'd':
    {
      index=DRUID_LEVEL_IND;
      sprintf(buf, "Druid");
      break;
    }
  case 'S': case 's':
    {
      index=PSI_LEVEL_IND;
      sprintf(buf, "Psionist");
      break;
    }
  case 'R': case 'r':
    {
      index=RANGER_LEVEL_IND;
      sprintf(buf, "Rangers");
      break;
    }
  case 'H': case 'h':
    {
      index=SHIFTER_LEVEL_IND;
      sprintf(buf, "Shifter");
      break;
    }
  case 'O': case 'o':
    {
      index=MONK_LEVEL_IND;
      sprintf(buf, "Monk");
      break;
    }
  case 'B': case 'b':
    {
      index=BARD_LEVEL_IND;
      sprintf(buf, "Bard");
      break;
    }
  default:
    {
//      if(IS_MULTI(ch)) {
	 send_to_char("Which class? Try <m|c|w|t|k|d|s|r|h|o|b>\n\r", ch);
	 return;
//      } else {

//      }
    }
  }

  level = GET_LEVEL(ch, index);

  if (level)
  {
    sprintf(buf2, " You have the following skills/spells as a level %d %s:\n\r", level, buf);
    ShowPracs(ch, MAX_MORT, index);
#if 0
    sprintf(buf2, " You have the following skills/spells as a level %d %s:\n\r", level, buf);
    send_to_char(buf2,ch);
    send_to_char("Level  Spell/Skill\n\r", ch);

    for(splvl=0; splvl <= MIN(level, MAX_MORT); splvl++) {
       for(i=0, spell=spell_list; i <= spell_count; i++, spell++) {
	  if(spell->name && (spell->min_level[index] == splvl)) {
	     if(ch->skills) {
		sprintf(buf,"[%-3d]   $Cy%-25s$CN %s\n\r", splvl,
			spell->name, how_good(ch->skills[spell->number].learned));
	     } else {
		sprintf(buf,"[%-3d]   $Cy%-25s$CN\n\r", splvl, spell->name);
	     }
	     send_to_char_formatted(buf, ch);
	  }
       }
    }

#if 0
    for (i = 0, spell = spell_list; i <= spell_count; i++, spell++)
    {
      if (spell->name && (!(GET_LEVEL(ch, index) < spell->min_level[index])) &&
	  (spell->min_level[index] <= MAX_MORT))
      {
	if (GET_LEVEL(ch, index) && ch->skills)
	  sprintf(buf,"[%2d]   %-20s %s\n\r", spell->min_level[index],
		  spell->name, how_good(ch->skills[spell->number].learned));
	else
	  sprintf(buf,"[%2d]   %-20s\n\r", spell->min_level[index], spell->name);
	send_to_char(buf, ch);
      }
    }
#endif
#endif
  }
  else
    send_to_char("You do not have that class.",ch);
}


void do_idea(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
	      send_to_char
		("That doesn't sound like a good idea to me.. Sorry.\n\r",ch);
		return;
	}
	if (!(fl = fopen(IDEA_FILE, "a")))	{
		perror ("do_idea");
		send_to_char("Could not open the idea-file.\n\r", ch);
		return;
	}

	sprintf(str, "%s: %s\n", GET_REAL_NAME(ch), argument);

	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n\r", ch);
}







void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

	if (!IS_PC(ch))	{
		send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("I beg your pardon?\n\r", 	ch);
		return;
	}
	if (!(fl = fopen(TYPO_FILE, "a")))	{
		perror ("do_typo");
		send_to_char("Could not open the typo-file.\n\r", ch);
		return;
	}

	sprintf(str, "%s[%ld]: %s\n",
		GET_REAL_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. thanks.\n\r", ch);

}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_INPUT_LENGTH+20];

    if (!IS_PC(ch))	{
	send_to_char("You are a monster! Bug off!\n\r", ch);
	return;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)	{
	send_to_char("Pardon?\n\r",ch);
	return;
    }
    if (!(fl = fopen(BUG_FILE, "a")))	{
	perror ("do_bug");
	send_to_char("Could not open the bug-file.\n\r", ch);
	return;
    }

    sprintf(str, "%s[%ld]: %s\n", GET_REAL_NAME(ch), ch->in_room, argument);
    fputs(str, fl);
    fclose(fl);
    send_to_char("Ok.\n\r", ch);
}

void do_afk(struct char_data *ch, char *argument, int cmd)
{
    char  message[80] = "";
    struct char_data *v;
    int stealth;

    if (IS_SET(ch->specials.flags, PLR_STEALTH))
        stealth = 1;
    else
        stealth = 0;

    if(!IS_AFK(ch))
    {
        send_to_char("You are now AFK.\n\r", ch);
        SET_BIT(ch->specials.flags, PLR_AFK);
        strcpy(message, "$n steps away from the keyboard.");
    } else {
        send_to_char("You return to the keyboard.\n\r", ch);
        REMOVE_BIT(ch->specials.flags, PLR_AFK);
        strcpy(message, "$n returns to the keyboard.");
    }

    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room)
    {
        if (ch == v)
            continue;
        else if (stealth && (TRUST(v) < ch->invis_level))
            continue;
        else
            act(message, FALSE, ch, 0, v, TO_VICT);
    }
}


/* Another fun command, just for fun, by Brett */
void do_mask(struct char_data *ch, char *argument, int cmd)
{
    if (IS_SET(ch->specials.flags, PLR_MASK))    {
	send_to_char("You are now longer in mask mode.\n\r", ch);
	REMOVE_BIT(ch->specials.flags, PLR_MASK);
    } else {
	if (IS_SET(ch->specials.pmask, BIT_POOF_IN)) {
	    send_to_char("You are now in mask mode, this is an a very FUN command.\n\r", ch);
	    send_to_char("Your POOFIN is used to MASK you in a room.\n\r", ch);
	    send_to_char("When someone types look in a room and if\n\r", ch);
	    send_to_char("your poofin is: A piece of bread lies here, then\n\r",ch);
	    send_to_char("when people walk into the room, this is what they will\n\r", ch);
	    send_to_char("see! You can mask yourself as ANYTHING, kind of like snooping\n\r", ch);
	    send_to_char("but much more fun!  Remember to be silent while in the room, or it will say\n\r", ch);
	    send_to_char("your name!  Also, you will NOT appear on the who god list to mortals\n\r", ch);
	    send_to_char("while in masked mode!\n\r", ch);
	    send_to_char("Do not go wizinvis while masked, or people will not be able to see\n\r", ch);
	    send_to_char("you in masked mode.  Besides, you DO NOT show up on WHO GOD while masked!\n\r", ch);
	    SET_BIT(ch->specials.flags, PLR_MASK);
	}
	else
	    send_to_char("You must have a POOFIN before you can MASK.\n\r",ch);
    }
}


void do_flags(struct char_data *ch, char *argument, int cmd)
{
  char buf [512];

  send_to_char("Player selected flags currently affecting your character:\n\r", ch);
  sprintbit(ch->specials.flags & ~PLR_LOGALL, player_bits, buf);
  send_to_char(buf, ch);
  sprintf(buf, "\n\r");
  send_to_char(buf, ch);
}

void do_color(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];

    /* mobs can do this now too */
    if(IS_SET(ch->specials.flags, PLR_COLOR))
    {
	send_to_char("ANSI color mode off.\n\r", ch);
	REMOVE_BIT(ch->specials.flags, PLR_COLOR);
    }
    else
    {
	sprintf(buf,"ANSI %sc%so%sl%so%sr%s mode on.\n\r",
		ANSI_RED,ANSI_CYAN,ANSI_BLUE,ANSI_ORANGE,
		ANSI_VIOLET,ANSI_NORMAL);
	send_to_char(buf, ch);
	SET_BIT(ch->specials.flags, PLR_COLOR);
    }
}


void do_tag(struct char_data *ch, char *argument, int cmd)
{
    char *arg;
    char name[MAX_STRING_LENGTH];
    struct char_data *mob;

    if ((!IS_SET(ch->specials.mob_act, ACT_IT)) && (TRUST(ch) < TRUST_SAINT))
    {
	send_to_char("You can't tag someone unless you're it, Bozo!\n\r",ch);
	return;
    }
    arg = one_argument(argument, name);
    mob = get_char_vis(ch, name);
    if (!mob) {
	send_to_char("You can't seem to find anyone like that in the game!\n\r",ch);
	return;
    }
    if(mob==ch) {
	send_to_char("Whoopee!  You tag yourself!  You are still IT!\n\r", ch);
	return;
    }
    if (mob->in_room!=ch->in_room) {
	send_to_char("You have to catch someone before you can tag them!\n\r",ch);
	return;
    }

    if(IS_SET(ch->specials.mob_act,ACT_IT))
	REMOVE_BIT(ch->specials.mob_act, ACT_IT);
    if(IS_SET(mob->specials.mob_act,ACT_IT))
	REMOVE_BIT(mob->specials.mob_act,ACT_IT);
    else
	SET_BIT(mob->specials.mob_act,ACT_IT);
    act("$n tags $N and yells 'You're IT!' with insane glee.",FALSE,ch,0,mob,TO_ROOM);
    act("You tag $N and yell 'You're it!'.",FALSE,ch,0,mob,TO_CHAR);
}

void do_report(struct char_data *ch, char *argument, int cmd)
{
    int  i;
    char info[160];
    char buf[MAX_STRING_LENGTH];

    if(apply_soundproof(ch))
	return;

    sprintf(info, "%d/%dHp %d/%dMana %d/%dMove", GET_HIT(ch), GET_MAX_HIT(ch),
	    GET_MANA(ch), GET_MAX_MANA(ch), GET_MOVE(ch), GET_MAX_MOVE(ch));

    for (i = 0; *(argument + i) == ' '; i++)
	;

    if (!*(argument + i)) {
	sprintf(buf, "$n reports '%s'", info);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "You report '%s'", info);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
    } else {
	char* arg = escape(argument + i);

	sprintf(buf, "$n reports '%s. %s'", info, arg);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "You report '%s. %s'", info, arg);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
}

/*
** A mechanical list of all the commands.
*/
void do_commands(struct char_data *ch, char *arg, int cmd)
{
  char	buf[16384], tmp[32];
  int no, i;
  struct command_info* c;

  if (IS_NPC(ch))
    return;

  send_to_char("The following commands are available:\n\r\n\r", ch);
  *buf = '\0';

  for(i = 0, no = 1, c = cmd_list ; c->name ; c++)
  {
    if ((GetMaxLevel(ch) >= c->minimum_level || IS_GOD(ch)) && !c->min_trust)
    {
      i++;
      sprintf(tmp, "%-13s", c->name);
      if((strlen(tmp) + strlen(buf) + 2) > sizeof(buf))
      {
	send_to_char("Command list to long, truncated list follows\n\r", ch);
	break;
      }
      strcat(buf, tmp);
      if (!(no % 6))
	strcat(buf, "\n\r");
      no++;
    }
  }

  strcat(buf, "\n\r");
  sprintf(buf + strlen(buf), "Total number of commands on system: %d\n\r", i);
  page_string(ch->desc, buf, 1);
}

void do_group(struct char_data *ch, char *argument, int cmd)
{
  char name[256];
  char buf[512];
  char title[MAX_STRING_LENGTH];
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found;

  only_argument(argument, name);

  if (!*name) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
    } else {
      send_to_char("Your group consists of:\n\r", ch);
      sprintf(buf, "%10s %15s %10s %6s %6s %6s\n\r",
	      "", "Name", "Level", "HP", "Mana", "Move");
      send_to_char(buf, ch);
      if (ch->master)
	k = ch->master;
      else
	k = ch;

      if (IS_AFFECTED(k, AFF_GROUP)) {
	sprintf(buf, "$CG%-10s %15s $CY%10d $CR%6d $CM%6d $CC%6d\n\r",
		"Leader:",
		GET_NAME(k),
		GetMaxLevel(k),
		GET_HIT(k),
		GET_MANA(k),
		GET_MOVE(k));
	act(buf,FALSE,ch, 0, k, TO_CHAR);
      }

      for(f=k->followers; f; f=f->next) {
	if(IS_AFFECTED(f->follower, AFF_GROUP)) {
	  sprintf(buf,"$CG%26s $CY%10d $CR%6d $CM%6d $CC%6d",
		  GET_NAME(f->follower),
		  GetMaxLevel(f->follower),
		  GET_HIT(f->follower),
		  GET_MANA(f->follower),
		  GET_MOVE(f->follower));
	  act(buf,FALSE,ch, 0, f->follower, TO_CHAR);
	}
      }
    }

    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("No one here by that name.\n\r", ch);
  } else {

    if (ch->master) {
      act("You can not enroll group members without being head of a group.",
	  FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

    found = FALSE;

    if (victim == ch)
      found = TRUE;
    else {
      for(f=ch->followers; f; f=f->next) {
	if (f->follower == victim) {
	  found = TRUE;
	  break;
	}
      }
    }

    if (found) {
      if (IS_AFFECTED(victim, AFF_GROUP)) {
	act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
	act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
	act("$N is no longer in your group!", FALSE, ch, 0, victim, TO_CHAR);
	REMOVE_BIT(AFF_FLAGS(victim), AFF_GROUP);
      } else {
	if (IS_GOD(victim) && (TRUST(victim) < TRUST_IMP)) {
	  act("You really don't want $n in your group.",
	      FALSE, victim, 0, 0, TO_CHAR);
	  return;
	}
	if (IS_GOD(ch) && (TRUST(ch) < TRUST_IMP)) {
	  act("Now now.  That would be CHEATING!",FALSE,ch,0,0,TO_CHAR);
	  return;
	}
	act("$n is now a member of $N's group.",
	    FALSE, victim, 0, ch, TO_ROOM);
	act("You are now a member of $N's group.",
	    FALSE, victim, 0, ch, TO_CHAR);
	if(victim != ch)
	  act("$N is now in your group!", FALSE, ch, 0, victim, TO_CHAR);
	SET_BIT(AFF_FLAGS(victim), AFF_GROUP);
      }
    } else {
      act("$N must follow you, to enter the group",
	  FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}

void do_split(struct char_data *ch, char *argument, int cmd)
{
  char                buf[MAX_INPUT_LENGTH];
  int                 money, members;
  struct char_data   *leader;
  struct follow_type *f;

  only_argument(argument, buf);

  if(!IS_AFFECTED(ch, AFF_GROUP))
  {
    send_to_char("You must be member of a group to split !\n\r", ch);
    return;
  }

  sscanf(buf, " %d ", &money);
  if(money < 0 || money > GET_GOLD(ch))
  {
    send_to_char("You can't split more than you have !\n\r", ch);
    return;
  }

  members = 0;
  leader = ch->master;
  if(!leader) leader = ch;

  /* count the members of our group in the room */

  if(IS_AFFECTED(leader, AFF_GROUP) && leader->in_room == ch->in_room)
    members++;
  for(f = leader->followers; f; f = f->next)
    if(IS_AFFECTED(f->follower, AFF_GROUP) &&
       f->follower->in_room == ch->in_room)
      members++;

  if(!members)
  {
    send_to_char("Attempt to split among 0 players.\n\rPlease report !\n\r",
		 ch);
    return;
  }

  if(money>500000) {
    sprintf(buf,"%s just split %d coins.", GET_NAME(ch),money);
    log_msg(buf);
  }

  sprintf(buf, "You split %d coins.\n\r", money);
  send_to_char(buf, ch);

  sprintf(buf, "%s splits %d coins.\n\rYou receive %d of them.\n\r",
	  GET_NAME(ch), money, money/members);

  GET_GOLD(ch) -= money;

  /* give every member his share */

  if(IS_AFFECTED(leader, AFF_GROUP) && leader->in_room == ch->in_room)
  {
    GET_GOLD(leader) += money/members;
    if(ch != leader)
      send_to_char(buf, leader);
  }
  for(f = leader->followers; f; f = f->next)
    if(IS_AFFECTED(f->follower, AFF_GROUP) &&
       f->follower->in_room == ch->in_room)
    {
      if((GET_GOLD(f->follower)<2000000) || (TRUST(ch) > TRUST_SAINT) ||
	 (TRUST(f->follower) > TRUST_SAINT)) {
        GET_GOLD(f->follower) += money/members;
        if(ch != f->follower)
	  send_to_char(buf, f->follower);
      } else {
	send_to_char("You can not possibly carry any more coins!  Visit a bank.\n\r", f->follower);

	if(ch!=f->follower)
	  sprintf(buf,"%s can not possibly carry any more coins!\n\r",
		  GET_NAME(f->follower));
	send_to_char(buf,ch);
	GET_GOLD(ch) += money/members;
      }
    }
}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *temp;
  int i;
  bool equipped;

  equipped = FALSE;

  only_argument(argument,buf);

  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp==0) || !isname(buf, OBJ_NAME(temp))) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
/*
  if (!IS_IMMORTAL(ch)) {
    if (GET_COND(ch,FULL)>20) {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    } else {
      if(GET_COND(ch, FULL) != -1)
	GET_COND(ch, FULL)+=1;
    }
  }
*/
  if (temp->obj_flags.type_flag!=ITEM_POTION) {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE, ch, temp,0, TO_CHAR);

  /*  my stuff */
  if (ch->specials.fighting) {
    if (equipped) {
      if (number(1,20) > GET_DEX(ch)) {
	act("$n is jolted and drops $p!  It shatters!",
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1,20) > (GET_DEX(ch) - 4)) {
	act("$n is jolted and drops $p!  It shatters!",
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	extract_obj(temp);
	return;
      }
    }
  }

  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
    {
      struct spell_info* spell;
      if(!(spell = spell_by_number(temp->obj_flags.value[i])) ||
	 !spell->spell_pointer)
      {
	char buf[256];
	sprintf(buf, "Illegal Potion Spell: %d on %s",
		temp->obj_flags.value[i], OBJ_NAME(temp));
	log_msg(buf);
      }
      else if(!IS_SET(temp->obj_flags.extra_flags, ITEM_MAGIC) ||
	   !check_nomagic(ch))
      {
	(*spell->spell_pointer)((byte) temp->obj_flags.value[0],
				ch, "", SPELL_TYPE_POTION, ch, temp);
      }
    }

  if (equipped)
    temp = unequip_char(ch, HOLD);

  extract_obj(temp);

  WAIT_STATE(ch, PULSE_VIOLENCE);

}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    struct obj_data *scroll, *obj;
    struct char_data *victim;
    int i, bits, equipped;
    struct spell_info* spell;

    equipped = FALSE;
    obj = 0;
    victim = 0;

    if (!ch->skills)
	return;

    if(check_soundproof(ch))
    {
      send_to_char("It's way to quiet here for that.\n\r", ch);
      return;
    }

    argument = one_argument(argument,buf);

    if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
	scroll = ch->equipment[HOLD];
	equipped = TRUE;
	if ((scroll==0) || !isname(buf, OBJ_NAME(scroll))) {
	    act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
	    return;
	}
    }

    if (scroll->obj_flags.type_flag!=ITEM_SCROLL)  {
	act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    bits = -1;
    for(i = 1 ; i <= 3 ; ++i)
    {
	if(scroll->obj_flags.value[i] < 1)
	    continue;
	if(!(spell = spell_by_number(scroll->obj_flags.value[i])) ||
	   !spell->spell_pointer)
	{
	    sprintf(buf, "Illegal spell %d on %s\n",
		    scroll->obj_flags.value[i], OBJ_NAME(scroll));
	    log_msg(buf);
	}
	else
	    bits &= spell->targets;
    }

    if (!HasClass(ch, CLASS_MAGIC_USER) &&
	!HasClass(ch, CLASS_CLERIC) && (!HasClass(ch, CLASS_DRUID)) &&
	(!HasClass(ch, CLASS_PALADIN)) && (!HasClass(ch, CLASS_PSI)) &&
	(!HasClass(ch, CLASS_RANGER)) && (!HasClass(ch, CLASS_SHIFTER)) &&
        (!HasClass(ch, CLASS_BARD)) && !HasClass(ch, CLASS_MONK)) {
	if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL && scroll->obj_flags.value[1] != SPELL_IDENTIFY) {
	    send_to_char("You can't understand this...\n\r",ch);
	    return;
	}
    }

    if(!spell_target(ch, bits, argument, &victim, &obj))
	return;

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    act("You recite $p which bursts into flame.",FALSE,ch,scroll,0,TO_CHAR);

    if(!check_nomagic(ch))
    {
	for (i=1; i<4; i++)
	    if (scroll->obj_flags.value[i] >= 1) {
		struct spell_info* spell;
		if((spell = spell_by_number(scroll->obj_flags.value[i])) &&
		   spell->spell_pointer)
		{
		    if(IS_SET(spell->targets, TAR_VIOLENT) &&
		       check_peaceful(ch, "Impolite magic is banned here.\n\r"))
			continue;
		    (*spell->spell_pointer)((byte) scroll->obj_flags.value[0],
					    ch, "", SPELL_TYPE_SCROLL, victim, obj);
		}
	    }
    }
    if (equipped)
	scroll = unequip_char(ch, HOLD);

    extract_obj(scroll);
}

void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;
  struct spell_info* spell;

  argument = one_argument(argument,buf);

  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, OBJ_NAME(ch->equipment[HOLD]))) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(RIDDEN(ch))
    return;

  stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF)  {
    act("$n taps $p three times on the ground.",
	TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",
	FALSE,ch, stick, 0,TO_CHAR);

    if(stick->obj_flags.value[2] <= 0)
    {
      send_to_char("The staff seems powerless.\n\r", ch);
      return;
    }

    if(!(spell = spell_by_number(stick->obj_flags.value[3])) ||
       !spell->spell_pointer)
    {
      char buf[256];
      sprintf(buf, "Illegal Staff Spell: %d on %s",
	      stick->obj_flags.value[3], OBJ_NAME(stick));
      log_msg(buf);
      return;
    }

    if(IS_SET(spell->targets, TAR_VIOLENT) &&
       check_peaceful(ch, "Impolite magic is banned here.\n\r"))
      return;

    if(check_nomagic(ch))
      return;

    (*spell->spell_pointer)((byte) stick->obj_flags.value[0],
			    ch, "", SPELL_TYPE_STAFF, 0, 0);

    stick->obj_flags.value[2]--;

    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else if (stick->obj_flags.type_flag == ITEM_WAND)
  {
    if(!(spell = spell_by_number(stick->obj_flags.value[3])) ||
       !spell->spell_pointer)
    {
      sprintf(buf, "Illegal spell %d on %s\n",
	      stick->obj_flags.value[3], OBJ_NAME(stick));
      log_msg(buf);
    }

    if(!spell_target(ch, spell->targets, argument,
		     &tmp_char, &tmp_object))
      return;

    if (tmp_char) {
      act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
      act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
    } else if(tmp_object) {
      act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
      act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
    }

    if (IS_SET(spell->targets, TAR_VIOLENT) &&
	check_peaceful(ch, "Impolite magic is banned here.\n\r"))
      return;

    if (stick->obj_flags.value[2] <= 0) /* Is there any charges left? */
    {
      send_to_char("The wand seems powerless.\n\r", ch);
      return;
    }

    if(check_nomagic(ch))
      return;

    stick->obj_flags.value[2]--;

    (*spell->spell_pointer)((byte) stick->obj_flags.value[0],
			     ch, "", SPELL_TYPE_WAND,
			     tmp_char, tmp_object);

     WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else
    send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
}

void do_alias(struct char_data *ch, char *argument, int cmd)
{
    int  num, count;
    char *ptr1, *ptr2;
    char exclam[] = "!", buf[MAX_INPUT_LENGTH+5];

    if(!IS_PC(ch))
    {
	send_to_char("Alias can not be used by monsters.\n\r", ch);
	return;
    }

    for(ptr1 = argument; *ptr1 == ' '; ptr1++); /* find first non-blank */

    if(!ch->aliases)
	clear_aliases(ch);

    if(*ptr1 == '\0')
    {				/* show alias-list */
	send_to_char("Your aliases:\n\r", ch);

	count = 0;
	for(num = 0; num < MAX_ALIAS_SAVE; num++)
	{
	    if(*((ch->aliases->pattern)[num]) == '!')
		continue;

	    sprintf(buf, "%s: %s\n\r", (ch->aliases->pattern)[num],
		    (ch->aliases->alias)[num]);
	    send_to_char(buf, ch);
	    count ++;
	}
	sprintf(buf, "(%d/%d)\n\r", count, MAX_ALIAS_SAVE);
	send_to_char(buf, ch);
	return;
    }
    else if(*ptr1 == '!')
    {
	send_to_char("Aliases cannot begin with an !\n\r", ch);
	return;
    }

    for(ptr2 = ptr1; *ptr2 && *ptr2 != ' '; ptr2++); /* Find next blank */
    if(!*ptr2)
    {
	send_to_char("Use unalias <alias> to remove an alias.\n\r", ch);
	return;
    }
    *ptr2 = '\0';
    for(ptr2++; *ptr2 == ' '; ptr2++); /* Find next non-blank */
    if(!*ptr2)
    {
	send_to_char("Use unalias <alias> to remove an alias.\n\r", ch);
	return;
    }

    if((num = search_block(ptr1, (const char**)(ch->aliases->pattern),
			   TRUE)) != -1)
    {				/* alias exists -> replace */
	if(ch->aliases->alias[num])
	    FREE(ch->aliases->alias[num]);
	ch->aliases->alias[num] = strdup(ptr2);
    }
    else if((num = search_block(exclam, (const char**)(ch->aliases->pattern),
				TRUE))!= -1)
    {
	FREE(ch->aliases->pattern[num]);
	if(ch->aliases->alias[num])
	    FREE(ch->aliases->alias[num]);

	ch->aliases->pattern[num] = strdup(ptr1);
	ch->aliases->alias[num]   = strdup(ptr2);
    }
    else
    {
	send_to_char("Sorry, you have to remove another alias !\n\r", ch);
    }
}

void do_unalias(struct char_data *ch, char *argument, int cmd)
{
    int num;
    char buf[MAX_INPUT_LENGTH];

    if(!IS_PC(ch))
    {
	send_to_char("Unalias can not be used by monsters.\n\r", ch);
	return;
    }

    if(!ch->aliases)
    {
	send_to_char("You don't have any aliases!\n\r", ch);
	return;
    }

    one_argument(argument, buf);
    if(*buf == '\0' || *buf == '!')
	return;

    if((num = search_block(buf, (const char**)(ch->aliases->pattern),
			   TRUE)) == -1)
    {
	send_to_char("You don't have that alias defined !\n\r", ch);
	return;
    }

    FREE(ch->aliases->pattern[num]);
    FREE(ch->aliases->alias[num]);
    ch->aliases->alias[num] = NULL;
    ch->aliases->pattern[num] = strdup("!");

    return;
}


void Dismount(struct char_data *ch, struct char_data *h, int pos)
{
    MOUNTED(ch) = 0;
    RIDDEN(h) = 0;
    GET_POS(ch) = pos;
}

void do_mount(struct char_data *ch, char *arg, int cmd)
{
    char name[MAX_INPUT_LENGTH];
    int check;
    struct char_data *horse;

    if (cmd == 394 || cmd == 396) {
	only_argument(arg, name);

	if (!(horse = get_char_room_vis(ch, name))) {
	    send_to_char("Mount what?\n\r", ch);
	    return;
	}

	if (!IsHumanoid(ch)) {
	    send_to_char("You can't ride things!\n\r", ch);
	    return;
	}

	if (IsRideable(horse)) {

	    if (GET_POS(horse) < POSITION_STANDING) {
		send_to_char("Your mount must be standing.\n\r", ch);
		return;
	    }

	    if (RIDDEN(horse)) {
		send_to_char("This beast is already ridden.\n\r", ch);
		return;
	    } else if (MOUNTED(ch)) {
		send_to_char("This creature is already riding.\n\r", ch);
		return;
	    }

	    check = MountEgoCheck(ch, horse);
	    if (check > 5) {
		act("$N snarls and attacks!",
		    FALSE, ch, 0, horse, TO_CHAR);
		act("as $n tries to mount $N, $N attacks $n!",
		    FALSE, ch, 0, horse, TO_NOTVICT);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		hit(horse, ch, TYPE_UNDEFINED);
		return;
	    } else if (check > -1) {
		act("$N moves out of the way, and you fall on your butt.",
		    FALSE, ch, 0, horse, TO_CHAR);
		act("As $n tries to mount $N, but $N moves out of the way.",
		    FALSE, ch, 0, horse, TO_NOTVICT);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		GET_POS(ch) = POSITION_SITTING;
		return;
	    }


	    if (RideCheck(ch, 50)) {
		act("You hop on $N's back.", FALSE, ch, 0, horse, TO_CHAR);
		act("$n hops on $N's back.", FALSE, ch, 0, horse, TO_NOTVICT);
		act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
		MOUNTED(ch) = horse;
		RIDDEN(horse) = ch;
		GET_POS(ch) = POSITION_MOUNTED;
		REMOVE_BIT(AFF_FLAGS(ch), AFF_SNEAK);
	    } else {
		act("You try to ride $N, but falls on $s butt.",
		    FALSE, ch, 0, horse, TO_CHAR);
		act("$n tries to ride $N, but falls on $s butt.",
		    FALSE, ch, 0, horse, TO_NOTVICT);
		act("$n tries to ride you, but falls on $s butt.",
		    FALSE, ch, 0, horse, TO_VICT);
		GET_POS(ch) = POSITION_SITTING;
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
	    }
	} else {
	    send_to_char("You can't ride that!\n\r", ch);
	    return;
	}
    } else if (cmd == 395) {
	horse = MOUNTED(ch);

	act("You dismount from $N.", FALSE, ch, 0, horse, TO_CHAR);
	act("$n dismounts from $N.", FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
	Dismount(ch, MOUNTED(ch), POSITION_STANDING);
	return;
    }

}


void do_plrtog(struct char_data *ch, char *arg, int cmd)
{
    int flag;
    char *msgon, *msgoff, buf[MAX_INPUT_LENGTH];

    switch (cmd) {
    case 199:
	flag = PLR_BRIEF;
	msgon = "Brief mode on.\n\r";
	msgoff = "Brief mode off.\n\r";
	break;

    case 213:
	flag = PLR_COMPACT;
	msgon = "You are now in compact mode.\n\r";
	msgoff = "You are now in the uncompacted mode.\n\r";
	break;

    case 337:
	flag = PLR_DISPLAY;
	msgon = "You will now only see your hitpoints, mana, and movement when it is low.\n\r";
	msgoff = "You will now only see your hitpoints, mana, and movement when it is low.\n\r";
	break;

    case 338:
	flag = PLR_AGGR;
	msgon = "You are now aggressive towards all aggressive monsters, and will attack first!\n\r";
	msgoff = "You are not aggressive towards aggressive monsters anymore.\n\r";
	break;

    case 347:
	flag = PLR_NOTELL;
	msgon = "You will not receive tells from now on.\n\r";
	msgoff = "You will now recieve tells once again.\n\r";
	break;

    case 350:
	flag = PLR_AUTOEXIT;
	msgon = "Visible exits will now be automatically displayed.\n\r";
	msgoff = "Visible exits will not be automatically displayed anymore.\n\r";
	break;

    case 444:
	flag = PLR_AUTOLOOT;
	msgon = "You will now loot things you kill.\n\r";
	msgoff = "You will no longer loot things you kill.\n\r";
	break;

    case 454:
	flag = PLR_AUTOSPLIT;
	msgon = "You will now split any gold you get.\n\r";
	msgoff = "You will no longer split gold you get.\n\r";
	break;

    case 455:
	flag = PLR_AUTOGOLD;
	msgon = "You will now loot gold from things you kill.\n\r";
	msgoff = "You will no longer loot gold from things you kill.\n\r";
	break;

    case 558:
	flag = PLR_SUMMON;
	msgon = "You will no longer be able to be summoned.\n\r";
	msgoff = "You will now be able to be summoned.\n\r";
        break;

    case 560:
	flag = PLR_AUTOASST;
        msgon = "You will now assist the group leader automatically.\n\r";
	msgoff = "You will not longer assist the group leader automatically.\n\r";
	break;

    default:
	send_to_char("Sorry, this command is screwed up.\n\r", ch);
	sprintf(buf, "Error in do_autotog, invalid cmd %d.", cmd);
	log_msg(buf);
	return;
    }

    ch->specials.flags ^= flag;
    if (IS_SET(ch->specials.flags, flag))
	send_to_char(msgon, ch);
    else
	send_to_char(msgoff, ch);
}


/* Borrowed from Sloth */
void panic_save(int signo)
{
    struct descriptor_data *i;
    struct char_data *c;
    int dummy=0;
    char *dum=NULL;
    char buf[1024];

    /* reset signals to prevent recursions */
    signal(SIGABRT,  SIG_DFL);
    signal(SIGBUS,   SIG_DFL);
    signal(SIGSEGV,  SIG_DFL);
    signal(SIGFPE,   SIG_DFL);

    /* give 1 minute to panic save */
    signal(SIGALRM, dump_mud);
    alarm(60);

    sprintf(buf, "In panic save...  signo %d", signo);
    log_msg(buf);
    rl_dump();
    sprintf(buf, "Last Known State: %s", last_known_state);
    log_msg(buf);

    if(!CheckSpaceFree())
    {
	log_msg("must be an infinite loop");
	return;
    }
    log_msg("got past the CheckSpaceFree check");

    EACH_DESCRIPTOR(d_iter, i)
    {
	if ((i->connected==CON_PLYNG)&&(i->character)&&(IS_PC(i->character)))
	{
	    do_save(i->character,dum,dummy);
	}
    }
    END_ITER(d_iter);

    EACH_CHARACTER(c_iter, c)
    {
	if((IS_PC(c))&&(!c->desc)) {
	    do_save(c,dum,dummy);
	}
    }
    END_AITER(c_iter);

    log_msg("BUG BUG BUG BUG BUG BUG BUG BUG");
    signal(SIGIOT, SIG_DFL);
    abort();
}

bool CheckSpaceFree(void)
{
  int file_length;
  FILE *tmpfile;

  tmpfile=fopen("CheckSpace.junk","w");
  for (file_length=0;file_length<100000;file_length++)
    if (EOF==fputc('x',tmpfile))
    {
      fclose(tmpfile);
      tmpfile=fopen("CheckSpace.junk","w");
      fclose(tmpfile);
      return(FALSE);
    }
  fclose(tmpfile);
  fopen("CheckSpace.junk","w");
  fclose(tmpfile);
  return(TRUE);
}

/* the following hack allows players to set their home room
   to specially designated places elswhere in the mud
   so they don't always have to start at the portal
   */


void do_temple(struct char_data *ch, char *arg, int cmd) {

  int i;

  for (i=0; TEMPLE_ROOMS[i] != 0; ++i) {
    if (ch->in_room == TEMPLE_ROOMS[i]) { /* if the char is in a temple room */
      ch->player.hometown = ch->in_room; /* set new hometown */
      send_to_char("You have chosen this room as your new temple.\n\r",ch);
      SaveChar(ch,AUTO_RENT,TRUE); /* save the room */
      return;
    }
  }
  send_to_char("This is not a temple approved by the gods...\n\r",ch);
}

void do_vprint(struct char_data *ch, char *arg, int cmd) {
   char buf[MAX_STRING_LENGTH*3];
   char vname[MAX_INPUT_LENGTH];
   char *bufp=buf, *argp=arg, *vnp;
   Variable *vtmp;

   while(*argp == ' ') argp++;

   while(*argp) {
      if(*argp == '~' && *(argp+1) != '~') {
	 vnp = vname;
	 argp++;
	 while(isalnum(*argp) || (*argp == '_'))
	    *vnp++ = *argp++;

	 *vnp = '\0';
	 *bufp='\0';

	 for(vtmp=ch->player.vars;vtmp;vtmp=vtmp->next)
	   if(!strcasecmp(vtmp->name, vname))
	      break;

	 strcpy(vname, "");
	 if(vtmp) {
	    switch(vtmp->type) {
	     case 0:
	       sprintf(vname, "%ld", vtmp->Value());
	       break;
	     case 1:
	       sprintf(vname, "%s", vtmp->CValue());
	       break;
	     case 2:
	       sprintf(vname, "%s", GET_NAME((char_data*)vtmp->Value()));
	       break;
	     default:
	       sprintf(vname, "---");
	    }
 	    strcat(buf, vname);
	 }

	 bufp += strlen(vname);
      } else {
	 if(*argp == '~' && *(argp+1) == '~') argp++;
	 (*bufp++ = *argp++);
      }
   }

   *bufp = '\0';

   command_interpreter(ch, buf, 1);
}

ACMD(do_calculate) {
   double res=0;
   char buf[MAX_INPUT_LENGTH];
   char mbuf[MAX_INPUT_LENGTH];
   char *mbp=mbuf;
   char *mbp2=mbp;

   while(*arg==' ') arg++;

   if(!*arg) {
      send_to_char("You must type in an expression to calculate.\n\r", ch);
      return;
   }

   res=MVarMath(ch, arg);

   sprintf(mbuf, "%f", res);
   while(*mbp && (*mbp != '.')) mbp++;
   mbp2=mbp;
   if(!strcmp(mbp, ".000000")) *mbp='\0';
   while(*mbp) {
      if((*mbp != '0')) mbp2=mbp;
      mbp++;
   }
   if(*mbp2 && *(mbp2+1)) *(mbp2+1)='\0';

   sprintf(buf, "The result is: %s\n\r", mbuf);
   send_to_char(buf, ch);
}
