#include "config.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif
#include <time.h>

#include "structs.h"
#include "comm.h"
#include "constants.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "race.h"
#include "board.h"
#include "master.h"
#include "act.h"
#include "utility.h"
#include "cmdtab.h"
#include "multiclass.h"
#include "handler.h"
#include "page.h"
#include "recept.h"
#include "util_str.h"
#include "fight.h"
#include "spells.h"
#include "ringlog.h"
#include "periodic.h"
#include "proto.h"
#include "char_create.h"
#include "language.h"
#include "spec.h"
#include "mobprog2.h"
#include "trackchar.h"

/* forward declarations */
int special(struct char_data *ch, int cmd, char *arg);
void enter_game(struct descriptor_data* d, int to_room);

#ifdef __cplusplus
extern "C"
{
#endif
  long time(long*);
  char* crypt(const char*, const char*);
#ifdef __cplusplus
};
#endif

extern struct fedit_data fedit;
char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
int WizLock;
int plr_tick_count=0;
int MAX_EXP_REIMB = 100000;
int CHAOS = 0;
int LOAD_PLAYER_FILES = 0;
int CASINO_BANK = 300000;
int PAWN_SHOP = 0;
int PIRATE = 0;
int PIRATENUM = 0;
int PIRATEQST = 0;
int PKILLABLE = 0;
int DISASTER = 0;
int DISASTERNUM = 0;
int FOUNTAIN_LEVEL = 3;
int FOUNTAIN_GOLD = 0;
int GROUP_RES = 2; // group restrictions
int AREA_FX_DEADLY = 1; // area fx switch
#ifdef PLAYER_AUTH
int AUTH_PLAYER = 1;
#else
int AUTH_PLAYER = 0;
#endif
#undef SITELOCK


char* AUTH_MESSAGE =
"--------------->>>>>>>>>>>>>>>>>>>ATTENTION<<<<<<<<<<<<<<<<<----------------\n\r\n\r"

"New Players must wait! for an Immortal to authorize their entrance into\n\r"
"Thieves World\n\r\n\r"

"By pressing the return key you are paging any available Immortal that you\n\r"
"wish to enter the game. If you get no response then either your name is\n\r"
"inappropiate for Thieves World or no Immortal is currently available.\n\r"
"Please try again because we value you as a player of Thieves World and\n\r"
"look forward to seeing you again!\n\r\n\r"

"PLEASE BE PATIENT AND ALLOW AN IMMORTAL TO REVIEW YOUR CASE AND AUTHORIZE YOU!\n\r\n\r";

int TimeCheck(struct descriptor_data* desc);

void hide_test(struct char_data *ch, int cmd)
{
  /* for now legal commands include tell/gtell, can be removed later */
  static int legal[]={8 /*exits*/, 15 /*look*/, 16 /*score*/, 19 /*tell*/,
                      20 /*inventory*/, 38 /*help*/, 39 /*who*/,
                      54 /*news*/, 55 /*equipment*/, 62 /*weather*/,
                      69 /*save*/, 76 /*time*/, 168 /*info*/,
                      174 /*levels*/, 199 /*brief*/, 200 /*wizlist*/,
                      201 /*consider*/, 212 /*credits*/, 213 /*compact*/,
                      215 /*deafen*/, 217 /*wimpy*/, 233 /*title*/,
                      236 /*attribute*/, 237 /*world*/, 238 /*allspells*/,
                      259 /*spells*/, 320 /*commands*/, 323 /*gtell*/,
                      324 /*alias*/, 325 /*unalias*/, 337 /*display*/,
                      338 /*aggressive*/, 341 /*color*/, 347 /*notell*/,
                      348 /*flags*/, 350 /*autoexit*/, 351 /*helptopics*/,
                      400 /*motd*/, 404 /*page*/, 417 /*channels*/,
                      234 /*whozone*/, 153 /*hide*/, 41 /*echo*/,
		      253 /*leeches*/, 556 /*gold*/, 557 /*affects */,
		      558 /*summon*/, 0};
  int x=0;

  if (!IS_AFFECTED(ch, AFF_HIDE) || affected_by_spell(ch, SKILL_CHAMELEON))
    return;

  while (legal[x]) {
    if (legal[x]==cmd)
      return;
    x++;
  }

  REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  act("Your actions reveal you from hiding.",TRUE,ch,0,0,TO_CHAR);
  act("You notice $n as $s actions reveal $m from hiding.",TRUE,ch,0,0,TO_ROOM);
}

void command_interpreter(struct char_data *ch, char *argument, int expand)
{
  char buf[200];
  struct command_info* cmd;
  int index;
  char command[256];
  char* arg;
  char* ptr;
  int real_level;
  struct char_data *rchar;

  ring_log("%s: %s\n", GET_NAME(ch), argument);

  if (MOUNTED(ch)) {
    if (ch->in_room != MOUNTED(ch)->in_room)
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
  }

  if(fedit.ch == ch) fedit.ch = 0;

  if(ch->board)
    {
      /*
      **  what about trans/flee?  board_kludge_board (later)
      */
      board_save_board(ch->board);
      ch->board->writer = 0;
      ch->board = 0;
    }

  if(ch->freeze)
    {
      send_to_char("You can't do anything, you are frozen from "
		   "head to toe!\n\r", ch);
      return;
    }

  /* Find first non blank */
  while(isspace(*argument))
    argument++;

  if(*argument == 0)
    return;

#if 0
  if ((*argument == '!') && (strlen(argument)>1))
    return;
#endif

  /* Find length of first word, special case ' for say here so that first
     word of ' doesn't get lowercased */
  if((*argument == '\'') || (*argument == ':') || (*argument == ';'))
    {
      if (*argument == ':')
	strcpy(command,":");
      else
        if (*argument == ';')
	  strcpy(command,";");
	else
          strcpy(command, "'");
          arg = argument + 1;
    }
  else
    {
      for(arg = argument, ptr = command ; *arg && !isspace(*arg) ; arg++)
	*ptr++ = LOWER(*arg);
      *ptr = 0;
    }

  if(ch->desc && expand && IS_PC(ch) && ch->aliases)
    {
      index = search_block(command, (const char**)(ch->aliases->pattern), TRUE);

      if((index != -1) && strcmp(ch->aliases->pattern[index], "!"))
	{			/* Alias found, expand it */
	  if(strlen(ch->aliases->alias[index]) + 2 + strlen(arg) > sizeof(buf))
	    {
	      send_to_char("alias expansion overflowed internal buffer\n\r",
			   ch);
	    }
	  else
	    {
	      strcpy(buf, ch->aliases->alias[index]);
	      strcat(buf, " ");
	      strcat(buf, arg);

	      command_interpreter(ch, buf, 0);
	    }

	  return;
	}
    }

  if(rprog_command_trigger(command, arg, ch)) return;
  if(mprog_command_trigger(command, arg, ch)) return;
  if(oprog_command_trigger(command, arg, ch)) return;

  cmd = lookup_command(command, 0);

  if(!cmd)
    {
      send_to_char("Wanna try typing that again?\n\r", ch);
      return;
    }

  if(IS_AFK(ch))
    {
      do_afk(ch, "", 420);
      if(cmd->index == 420)
	return;
    }

  real_level = GetRealLevel(ch);

  rchar = real_character(ch);

  if ((cmd->min_trust && (!HAS_GCMD(rchar,cmd->min_trust))) ||
      (real_level < cmd->minimum_level && !IS_GOD(rchar)))
    {
      send_to_char("Wanna try typing that again?\n\r", ch);
    }
  else if ((IS_AFFECTED(ch, AFF_PARALYSIS)) &&
	   (cmd->minimum_position > POSITION_STUNNED))
    {
      send_to_char(" You are paralyzed, you can't do much of anything!\n\r", ch);
    }
  else if(GET_POS(ch) < cmd->minimum_position )
    {
      switch(GET_POS(ch))
	{
	case POSITION_DEAD:
	  send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
	  break;
	case POSITION_INCAP:
	case POSITION_MORTALLYW:
	  send_to_char("You are in a pretty bad shape, unable to do anything!\n\r",ch);
	  break;

	case POSITION_STUNNED:
	  send_to_char("All you can do right now, is think about the stars!\n\r",ch);
	  break;
	case POSITION_SLEEPING:
	  send_to_char("In your dreams, or what?\n\r", ch);
	  break;
	case POSITION_RESTING:
	  send_to_char("Nah... You feel too relaxed to do that..\n\r", ch);
	  break;
	case POSITION_SITTING:
	  send_to_char("Maybe you should get on your feet first?\n\r",ch);
	  break;
	case POSITION_FIGHTING:
	  send_to_char("No way! You are fighting for your life!\n\r", ch);
	  break;
	case POSITION_STANDING:
	  send_to_char("You can't do that right now.\n\r", ch);
	  break;
	}
    }
  else if(cmd->command_pointer == 0)
    {
      send_to_char("Sorry, but that command has yet to be implemented...\n\r",
		   ch);
    }
  else if(cmd->index <= 6 || no_specials || !special(ch, cmd->index, arg))
    {
      if(cmd->index > 6)
	{
	  if (IS_SET(ch->specials.flags, PLR_LOGALL))
	    {
	      sprintf(buf, "%s:%s (logall)", GET_REAL_NAME(ch), argument);
	      slog(buf);
	    }
            else if( (TRUST(ch) >= LOW_LOG_ALL) &&
                     (TRUST(ch) <= HIGH_LOG_ALL) )
	    {
	      sprintf(buf,"%s:%s (std log)",GET_REAL_NAME(ch),argument);
	      slog(buf);
	    }
	}

      hide_test(ch, cmd->index);

      (*cmd->command_pointer)(ch, arg, cmd->index);
    }
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
  char* arg;

  do
    {
      /* Find first non blank */
      while(isspace(*argument))
	argument++;

      /* Find length of first word */
      for(arg = first_arg ;
	  *argument && !isspace(*argument) ;
	  arg++, argument++)
	*arg = LOWER(*argument);
      *arg = '\0';
    }
  while( fill_word(first_arg));

  do
    {
      /* Find first non blank */
      while(isspace(*argument))
	argument++;

      /* Find length of first word */
      for(arg = second_arg ;
	  *argument && !isspace(*argument) ;
	  arg++, argument++)
	*arg = LOWER(*argument);
      *arg = '\0';
    }
  while( fill_word(second_arg));
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *obj;
  register struct obj_data **objP;
  register struct char_data *mob;
  struct room_data *rp;
  int j;

  if (ch->in_room == NOWHERE) {
    char_to_room(ch, 3001);
    return 0;
  }

  /* special in room? */
  if ((rp = real_roomp(ch->in_room))->funct &&
      ((rp->funct)(rp, ch, cmd, arg, SPEC_CMD)))
    return(1);

  /* special in equipment list? */
  for(j = MAX_WEAR, objP = ch->equipment ; j ; --j, objP++)
    if((obj = *objP) &&
       (obj->item_number >= 0) &&
       (obj_index[obj->item_number].func) &&
       ((*obj_index[obj->item_number].func)(obj, ch, cmd, arg, SPEC_CMD)))
      return(1);

  /* special in inventory? */
  for (obj = ch->carrying ; obj ; obj = obj->next_content)
    if((obj->item_number >= 0) &&
       (obj_index[obj->item_number].func) &&
       ((*obj_index[obj->item_number].func)(obj, ch, cmd, arg, SPEC_CMD)))
      return(1);

  /* special in mobile present? */
  for (mob = real_roomp(ch->in_room)->people ; mob ; mob = mob->next_in_room)
    if(IS_MOB(mob) && mob_index[mob->nr].func &&
       IS_SET(mob->specials.mob_act, ACT_SPEC) &&
       ((*mob_index[mob->nr].func)(mob, ch, cmd, arg, SPEC_CMD)))
      return(1);

  /* special in object present? */
  for (obj = real_roomp(ch->in_room)->contents ; obj ; obj = obj->next_content)
    if((obj->item_number>=0) &&
       (obj_index[obj->item_number].func) &&
       ((*obj_index[obj->item_number].func)(obj, ch, cmd, arg, SPEC_CMD)))
      return(1);

  return(0);
}


/* *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */


int _parse_name(const char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if ((*arg <0) || !isalpha(*arg) || i > 15)
      return(1);

  if (!i)
    return(1);

  return(0);
}


void update_hostname(struct descriptor_data* desc, struct char_data* ch)
{
  if(ch->specials.hostname)
    {
      char buf[256];
      sprintf(buf, "\n\rLast connected from: %s\n\r", ch->specials.hostname);
      SEND_TO_Q(buf, desc);
      if (ch->specials.hostname)
	FREE(ch->specials.hostname);
    }

  ch->specials.hostname = strdup(desc->host);
}

/* Not used anywhere at the moment */
int find_plyr_race(struct char_data *ch)
{
  int i;

  for(i=0;i<MAX_PLAYER_RACE;i++)
    if (playerraces[i]==ch->race)
      return i;

  return -1;
}


/* This function simulates a roll of a 6 sided die 3 times, it is done this
   way so as to create a more statistical shaped bell curve, over a simple
   random number between 3 and 18. */
void roll_stats (struct char_data *ch)
{
  ch->abilities.str = rollD6 + rollD6 + rollD6;
  ch->abilities.intel = rollD6 + rollD6 + rollD6;
  ch->abilities.wis = rollD6 + rollD6 + rollD6;
  ch->abilities.dex = rollD6 + rollD6 + rollD6;
  ch->abilities.con = rollD6 + rollD6 + rollD6;
  ch->abilities.cha = rollD6 + rollD6 + rollD6;
}

/* This function checks the table located in char_create.c to see if a
   particular character meets racial requirements. */
bool meets_race_requirements(struct char_ability_data *stats, int race)
{
  return ((stats->str >= plyr_race_stats[race].min_str) &&
	  (stats->str <= plyr_race_stats[race].max_str) &&
	  (stats->intel >= plyr_race_stats[race].min_int) &&
	  (stats->intel <= plyr_race_stats[race].max_int) &&
	  (stats->wis >= plyr_race_stats[race].min_wis) &&
	  (stats->wis <= plyr_race_stats[race].max_wis) &&
	  (stats->dex >= plyr_race_stats[race].min_dex) &&
	  (stats->dex <= plyr_race_stats[race].max_dex) &&
	  (stats->con >= plyr_race_stats[race].min_con) &&
	  (stats->con <= plyr_race_stats[race].max_con) &&
	  (stats->cha >= plyr_race_stats[race].min_cha) &&
	  (stats->cha <= plyr_race_stats[race].max_cha)) ? TRUE : FALSE;
}

/* This function first checks to see if which particular class we are
   checking, then checks the appropriate class_requirement table found
   in char_create.c to see if these stats qualify. */
bool meets_class_requirements(struct char_ability_data *stats,
  const struct allowed_class *current_class)
{
  int class_ind;

  for (class_ind = 0; class_ind <= MAX_LEVEL_IND; class_ind++)
    if (current_class->class_type & (1 << class_ind)) /* Has class */
      if  ((stats->str < plyr_class_requirements[class_ind].str_req) ||
	  (stats->intel < plyr_class_requirements[class_ind].int_req) ||
	  (stats->wis < plyr_class_requirements[class_ind].wis_req) ||
	  (stats->dex < plyr_class_requirements[class_ind].dex_req) ||
	  (stats->con < plyr_class_requirements[class_ind].con_req) ||
	  (stats->cha < plyr_class_requirements[class_ind].cha_req))
	return FALSE;   /* Didn't qualify */
  return TRUE;
}

/* Generate the list of races the rolls qualify for, and let the player
   select his race from the choices presented to them. */
void choose_race(struct descriptor_data *d)
{
  int i,j;
  int count = 0;               /* The counter of displayed races */
  int race[MAX_PLAYER_RACE];   /* Maps choice number to race number */
  char buf[80];

  SEND_TO_Q("You qualify to be:\n\r",d);
  for (i=0; i < MAX_PLAYER_RACE; i++)
    {
      if (meets_race_requirements(&(d->character->abilities),i))
	{                                /* Qualifies to be this race */
	  race[count] = playerraces[i];  /* Map choice to Race number */
	  sprintf(buf,"%i) %s",++count,RaceName[playerraces[i]]);
	  for (j = strlen(buf);j < 40;j++)
	    strcat(buf," ");
	  SEND_TO_Q(buf,d);
	  if (!(count%2))
	    SEND_TO_Q("\n\r",d);
	}
    }
  SEND_TO_Q("\n\rFor help type '?'. \n\r RACE:  ", d);
}

char *nanny_stats(struct char_data *ch,char *buf)
{
  sprintf(buf,"\t Str:%i/%i  Int:%i  Wis:%i  Dex:%i  Con:%i  Cha:%i  Sz:Undefined\n\r",
	  ch->abilities.str, ch->abilities.str_add,ch->abilities.intel,
	  ch->abilities.wis, ch->abilities.dex, ch->abilities.con,
	  ch->abilities.cha);
  return buf;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, const char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int counter =0, count=0;
  char tmp_name[20];
  struct char_data *tmp_ch;
  struct descriptor_data *k;
  int mastered;
  int num = 0, lnum=0;
  int i;
  int alignment = 0;
  struct char_ability_data *stats;
  const struct allowed_class *current_class;
  struct class_choice {     /* Maps choice number to race number */
    int classes;
    int playernum;
  };
  struct class_choice *class_list = NULL;

  ring_log("nanny: name=%s: state=%d: %s\n",
	   d->character ? GET_NAME(d->character) : "unnamed",
	   STATE(d), STATE(d) == CON_PWDNRM ? ">>SECRET<<" : arg);

  write(d->descriptor, echo_on, 6);

  switch (STATE(d))
    {

    case CON_NME:			/* wait for input of name        */
      if (!d->character) {
	CREATE(d->character, struct char_data, 1);
	clear_char(d->character);
	clear_aliases(d->character);
	set_descriptor(d->character, d);
      }

      for (; isspace(*arg); arg++)  ;
      if (!*arg)
	close_socket(d, TRUE);
      else {
	if(_parse_name(arg, tmp_name))         {
	  SEND_TO_Q("Illegal name, please try another.", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}

	/* Check if already playing */
	EACH_DESCRIPTOR(d_iter, k)
	  {
	    if ((k->character != d->character) && k->character) {
	      if (GET_REAL_NAME(k->character) &&
		  !str_cmp(GET_REAL_NAME(k->character), tmp_name))
		{
		  SEND_TO_Q("Already playing, to Csocket current character and reconnect.\n\r", d);
		  SEND_TO_Q("Enter Password:",d);
		  LoadChar(d->character, tmp_name, READ_FIRST);
		  d->character->desc = d;
		  write(d->descriptor, echo_off, 4);
		  STATE(d) = CON_PWDCSCKT;
		  return;
		}
	    }
	  }
	END_ITER(d_iter);

	/* Not playing load character */
	if(LoadChar(d->character, tmp_name, READ_FIRST))
	  {
	    d->character->desc = d;

	    SEND_TO_Q("Password: ", d);
	    write(d->descriptor, echo_off, 4);
	    STATE(d) = CON_PWDNRM;
	    d->pwd_attempt = 0;
	  }
	else
	  {
	    /* player unknown gotta make a new */
	    if(TimeCheck(d))
	      {
		STATE(d) = CON_WIZLOCK;
	      }
	    else if (!WizLock && (AUTH_PLAYER >= 0)) {
	      int n;

	      /* Make sure its not a mobile */
	      for (n=0; n<=top_of_mobt; n++)
		if(isname(tmp_name,mob_index[n].name))
		  {
		    SEND_TO_Q("Illegal name, please try another.", d);
		    SEND_TO_Q("Name: ", d);
		    STATE(d)=CON_NME;
		    return;
		  }
	      d->character->player.name = ss_make(CAP(tmp_name));
	      d->character->player.short_descr =
		ss_share(d->character->player.name);
	      sprintf(buf, "Did I get that right, %s (Y/N)? ",
		      tmp_name);
	      SEND_TO_Q(buf, d);
	      STATE(d) = CON_NMECNF;
	    } else {
	      sprintf(buf,
		      "Sorry, %s, no new characters at this time\n\r",
		      tmp_name);
	      SEND_TO_Q(buf,d);
	      STATE(d) = CON_WIZLOCK;
	    }
	  }
      }

      break;

    case CON_PWDNRM:		/* get pwd for known player        */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      if (!*arg)
	close_socket(d, TRUE);
      else  {
	mastered = 0;
	if(!d->character->pwd)
	  {
	    SEND_TO_Q("Illegal password.  Please contact an imp.\n\r", d);
	    return;
	  }
	if(strncmp(crypt(arg, d->character->pwd), d->character->pwd, 10))
	  {
	    if(strncmp(crypt(arg, MASTER_PASSWORD), MASTER_PASSWORD, 10))
	      {
		if(++d->pwd_attempt > 2)
		  {
		    sprintf(buf, "Too many attempts: %s (%s)",
			    GET_IDENT(d->character), d->host);
		    log_msg(buf);
		    SEND_TO_Q("Too many attempts.\n\r", d);
		    STATE(d) = CON_WIZLOCK;
		  }
		else
		  {
		    SEND_TO_Q("Wrong password.\n\r", d);
		    SEND_TO_Q("Password: ", d);
		    write(d->descriptor, echo_off, 4);
		  }
		return;
	      }
	    mastered = 1;
	  }

	if(TimeCheck(d))
	  {
	    STATE(d) = CON_WIZLOCK;
	    return;
	  }

      Reconnect:   /* Label used for Csocket hack,
		      just use code for reconnection */
	EACH_CHARACTER(iter, tmp_ch)
	  {
	    if(!tmp_ch->desc &&
	       (tmp_ch != d->character) &&
	       IS_PC(tmp_ch) &&
	       !str_cmp(GET_IDENT(d->character), GET_REAL_NAME(tmp_ch)))
	      break;
	  }
	END_AITER(iter);

	if(tmp_ch)
	  {
	    write(d->descriptor, echo_on, 6);
	    SEND_TO_Q("Reconnecting.\n\r", d);

	    tmp_ch = curr_character(tmp_ch);

	    free_char(d->character);
	    set_descriptor(tmp_ch, d);
	    d->character = tmp_ch;
	    tmp_ch->specials.timer = 0;
	    if (!IS_IMMORTAL(tmp_ch))
	      tmp_ch->invis_level = 0;

	    STATE(d) = CON_PLYNG;
	    connected++;
	    if (connected>max_connected)
	      max_connected=connected;

	    if(TRUST(d->character) <= TRUST_LORD) {
	      act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	    }
	    sprintf(buf, "%s(%s) has %ceconnected.",
		    GET_REAL_NAME(d->character), d->host,mastered ? 'R' : 'r');
	    slog(buf);
	    WriteToImmort(buf, d->character->invis_level, LOG_CONNECT);

	    if(IS_IMMORTAL(d->character))
	      list_pages(d->character, 0);

	    update_hostname(d, d->character);

	    /* try to put the character back in their original room */

	    if (d->character->specials.was_in_room != NOWHERE) {
	      char_from_room(d->character);
	      char_to_room(d->character,d->character->specials.was_in_room);
	    }

	    return;
	  }


	sprintf(buf, "%s(%s) has %connected.",
		GET_IDENT(d->character), d->host,
		mastered ? 'C' : 'c');
	slog(buf);
	WriteToImmort(buf, d->character->invis_level, LOG_CONNECT);
	if ((d->character->player.trust > 0) &&
		 strcmp("Lansing",GET_NAME(d->character)))
	{
	  char new_imotd[MAX_STRING_LENGTH];
	  format_string(imotd, new_imotd, CheckColor(d->character));
	  SEND_TO_Q(new_imotd, d);
	}
	else
        {
          char new_motd[MAX_STRING_LENGTH];
	  format_string(motd, new_motd, CheckColor(d->character));
	  SEND_TO_Q(new_motd, d);
	}

	update_hostname(d, d->character);

	SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);

#ifdef SITELOCK
	STATE(d) = CON_SITELOCK;
#else
	STATE(d) = CON_RMOTD;
#endif
      }
      break;


    case CON_PWDCSCKT:               /* Socket and reconnect player */
      /* Added by Jim for Password protected player Sockets */
      /* Lots of logging, to try and track down the infrequent Seg
	 violation */
      for (; isspace(*arg); arg++);
      if (!*arg)
	{
	  slog("Zero length password in Csocket stuff");
	  SEND_TO_Q("Sorry zero length password is illegal.\n\r", d);
	  close_socket(d, TRUE);
	}
      else {
	if(!d->character->pwd)
	  {
	    slog("Illegal password in CSocket Code");
	    SEND_TO_Q("Illegal password.  Please contact an imp.\n\r", d);
	    close_socket(d, TRUE);
	    return;
	  }

	if (strncmp(crypt(arg, d->character->pwd), d->character->pwd, 10))
	  {
	    sprintf(buf, "(%s) failed to CSOCKET  %s.",
		    d->host, GET_IDENT(d->character));
	    slog(buf);
	    WriteToImmort(buf, d->character->invis_level, LOG_PLAYER);
	    close_socket(d, TRUE);
	    return;
	  }
	else     /* Gave the correct password */
	  {
	    EACH_DESCRIPTOR(d_iter, k)
	      {
		if ((k->character != d->character) && k->character
		    && d->character)
		  if ((!strcmp(GET_IDENT(d->character),
			       GET_REAL_NAME(k->character))) && (d != k))
		    {  /* If is character, and not same socket */
		      if (STATE(k) != CON_PLYNG)
			{
			  sprintf(buf, "%s closing non-playing socket.",
				  GET_IDENT(d->character));
			  slog(buf);
			  close_socket(k, TRUE);
			  WriteToImmort(buf, d->character->invis_level, LOG_PLAYER);
			}
		      else
			{
			  SEND_TO_Q("Csocketed.\n\r", d);
			  act("$n suddenly keels over in pain, surrounded by a "
			      "white aura...\r\n$n's body has been taken over by"
			      " a new spirit!", TRUE, k->character, 0, 0, TO_ROOM);
			  close_socket(k, FALSE);
			  sprintf(buf, "%s was CSOCKETED by (%s).",
				  GET_IDENT(d->character), d->host);
			  slog(buf);

			  WriteToImmort(buf, d->character->invis_level, LOG_PLAYER);
			  break;
			}
		    }
	      }
	    END_ITER(d_iter);
	    mastered = 1;
	    slog ("Beginning Reconnection");
	    goto Reconnect;
	  }
      }

    case CON_NMECNF:		/* wait for conf. of new name        */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (*arg == 'y' || *arg == 'Y')        {
	SEND_TO_Q("New character.\n\r", d);
	if (isbanned(d->host) >= BAN_NEW) {
	  sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		  GET_NAME(d->character), d->host);
	  log_msg(buf);
	  SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
	  SEND_TO_Q("Please mail tw@tw.imaxx.net if you have questions.\n\r", d);
	  STATE(d) = CON_CLOSE;
	  break;
	}

	sprintf(buf,"Give me a password for %s: ",GET_IDENT(d->character));

	SEND_TO_Q(buf, d);
	write(d->descriptor, echo_off, 4);
	STATE(d) = CON_PWDGET;
      }
      else  {
	if (*arg == 'n' || *arg == 'N') {   /* Ok need to re-enter name */
	  SEND_TO_Q("Ok, what IS it, then? ", d);
	  ss_free(d->character->player.name);
	  STATE(d) = CON_NME;
	} else {			/* Please do Y or N */
	  SEND_TO_Q("Please type Yes or No? ", d);
	}
      }
      break;


    case CON_PWDGET:	    /* Confirmed new name, get pwd for new player */

      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (!*arg || strlen(arg) > 10)         {
	write(d->descriptor, echo_on, 6);
	SEND_TO_Q("Illegal password.\n\r", d);
	SEND_TO_Q("Password: ", d);

	write(d->descriptor, echo_off, 4);
	return;
      }

      d->character->pwd = strdup((char*) crypt(arg,
					       GET_IDENT(d->character)));
      write(d->descriptor, echo_on, 6);
      SEND_TO_Q("Please retype password: ", d);
      write(d->descriptor, echo_off, 4);
      STATE(d) = CON_PWDCNF;
      break;

    case CON_PWDCNF:		/* get confirmation of new pwd */

      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (strncmp(crypt(arg, d->character->pwd), d->character->pwd, 10)) {
	write(d->descriptor, echo_on, 6);

	SEND_TO_Q("Passwords don't match.\n\r", d);
	SEND_TO_Q("Retype password: ", d);
	STATE(d) = CON_PWDGET;
	write(d->descriptor, echo_off, 4);
	return;
      } else {
	write(d->descriptor, echo_on, 6);

	goto Roll;
      Reroll:	       /* Something to discourage but not prevent */
	/*sleep (4);     rerolling by players. */
      Roll:
	roll_stats(d->character);

      set_state_race:
        choose_race(d);
	STATE(d) = CON_QRACE;
      }
      break;

    case CON_QRACE:    /* Check to see if they entered a valid race */
      for (; isspace(*arg); arg++)  ;
      if(is_number(arg))
	num=atoi(arg);
      else
	if ((*arg) == '?')
	  {
	    SEND_TO_Q("\n\rSorry there is no help available at the moment\n\r", d);
	    goto set_state_race;
	  }
	else
	  {
	    SEND_TO_Q("\n\rThat's not a valid race, please try again.\n\r", d);
	    goto set_state_race;
	  }

      counter = 0;             /* The counter of displayed races */

      struct {
	int racenum;
	int playernum;
      } race[MAX_PLAYER_RACE];   /* Maps choice number to race number */

      stats = &(d->character->abilities);

      /* Ok, this is a hack, but I couldn't think of anyway to do this
	 without rewriting all the socket stuff, so I just refigure it
	 all. */
      for (i=0; i < MAX_PLAYER_RACE; i++)
	if (meets_race_requirements(stats, i))
	  {	 /* Map choice to Race number */
	    race[counter].racenum = playerraces[i];
	    race[counter++].playernum = i;
	  }

      if ( (num < 1) || (num > counter) )
	{
	  SEND_TO_Q("\n\rThat's not a valid race, please try again.\n\r", d);
	  goto set_state_race;
	}

      GET_RACE(d->character) = race[--num].racenum;

      stats->str += plyr_race_mods[race[num].playernum].str_mod;
      stats->intel += plyr_race_mods[race[num].playernum].int_mod;
      stats->wis += plyr_race_mods[race[num].playernum].wis_mod;
      stats->dex += plyr_race_mods[race[num].playernum].dex_mod;
      stats->con += plyr_race_mods[race[num].playernum].con_mod;
      stats->cha += plyr_race_mods[race[num].playernum].cha_mod;

      for(i=0;i<MAX_TONGUE;i++) {
	 if(tongue_info[i].race == GET_RACE(d->character))
	   switch(stats->intel) {
  	    case 0: case 1: case 2: case 3: case 4: case 5: case 6:
	      lnum=1;
	      break;
	    case 7: case 8: case 9: case 10: case 11: case 12: case 13:
	      lnum=2;
	      break;
	    case 14: case 15:
	      lnum=3;
	      break;
	    case 16: case 17:
	      lnum=4;
	      break;
	    case 18:
	      lnum=5;
	      break;
	    default:
	      lnum=MAX_TONGUE;
	   }
      }

      i=0;

//      while((tongue_info[race[num].racenum].languages[i] > 0) && (i<lnum)) {
//	 d->character->player.talks[tongue_info[race[num].racenum].languages[i++]] = 1;
//      }

      /* For exceptional strength */
      if (stats->str < 18)
	stats->str_add = 0;
      else if (stats->str > 18)
	stats->str_add = 100;
      else
	stats->str_add = number(0,100);

      SEND_TO_Q("What is your sex? M)ale F)emale\n\r", d);
      STATE(d) = CON_QSEX;
      break;

    case CON_QSEX:		/* query sex of new user        */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      switch (*arg)
	{
	case 'm':
	case 'M':
	  /* sex MALE */
	  d->character->player.sex = SEX_MALE;
	  break;

	case 'f':
	case 'F':
	  /* sex FEMALE */
	  d->character->player.sex = SEX_FEMALE;
	  break;

	default:
	  SEND_TO_Q("That's not a sex..\n\r", d);
	  SEND_TO_Q("What IS your sex? :", d);
	  return;
	  break;
	}

      /* Ok, we know the race and sex of the player lets give them
	 characteristics. */
      for (i=0; GET_RACE(d->character) != Race_Characteristic[i].Race;
	   i++);

      if (d->character->player.sex == SEX_MALE)
	{
	  d->character->player.weight =
	    Race_Characteristic[i].Male_Weight.base +
	    Race_Characteristic[i].Male_Weight.min_mod *
	    number(1,Race_Characteristic[i].Male_Weight.max_mod);
	  d->character->player.height =
	    Race_Characteristic[i].Male_Height.base +
	    Race_Characteristic[i].Male_Height.min_mod *
	    number(1,Race_Characteristic[i].Male_Height.max_mod);
	}
      else
	{
	  d->character->player.weight =
	    Race_Characteristic[i].Female_Weight.base +
	    Race_Characteristic[i].Female_Weight.min_mod *
	    number(1,Race_Characteristic[i].Female_Weight.max_mod);
	  d->character->player.height =
	    Race_Characteristic[i].Female_Height.base +
	    Race_Characteristic[i].Female_Height.min_mod *
	    number(1,Race_Characteristic[i].Female_Height.max_mod);
	}

       d->character->player.time.birth = time(0);
       d->character->player.time.birth -= SECS_PER_MUD_YEAR *
	 (Race_Characteristic[i].Age.base +
	  Race_Characteristic[i].Age.min_mod *
	  number(1,Race_Characteristic[i].Age.max_mod));

      count = 0;

    print_classes:
      SEND_TO_Q("\n\rYou can choose to be a:\n\r", d);

      /* Ok there is currently no way to save state information, so
	 what I have done is recalculate info we know from the
	 previous state.  What really should be done, when there is
	 time, is write some functions to save states.  i is the
	 magic offset index into the table for the appropriate race. */
      for (i=0; GET_RACE(d->character) != Classes_allowed_by_Race[i].race;
	   i++);

    set_state_class:

      /* Initialize the first element */
      current_class = Classes_allowed_by_Race[i].Classes;

      do {
	if (meets_class_requirements(&(d->character->abilities),
				     current_class))
	  {                           /* Qualifies to be this class */
	    sprintf(buf,"%d) %s",++count,
		    current_class->class_name);
	    for (int j = strlen(buf);j < 40;j++)  /* My tabs don't print */
	      strcat(buf," ");                    /* as tabs */
	    SEND_TO_Q(buf,d);
	    if (!(count%2))
	      SEND_TO_Q("\n\r",d);
	  }
	current_class++;
      } while (current_class->class_name);  /* Not done with classes */

      /* Make sure they qualify for atleast one class */
      if (count == 0)
	{   /* Don't Qualify for a class, lets reroll without telling */
	  slog("REAL BAd Stats");
	  do {
	    roll_stats(d->character);
	  } while (!meets_race_requirements(&(d->character->abilities),i));
	  goto set_state_class;
	}
      /*      give_some_info(d->character);*/
/* This version has no stats viewable ---      DaBoss    */
/*      SEND_TO_Q("\n\rFor help type '?'.  To reroll type 'R'.\n\rClass:  ", d);    */

      /*This version allows stats to be viewed during creation and   */
      /*permits players to reroll before completing character creation*/
      /*Coded by Mtr March 2005 ----- Sanctioned by Harrier           */
      SEND_TO_Q("\n\r\n\rYour current attributes are:\n\r", d);
      sprintf(buf, "\n\r  STR=%i,  INT=%i,  WIS=%i,  DEX=%i,  CON=%i,  CHA=%i\n\r",
              d->character->abilities.str, d->character->abilities.intel,
              d->character->abilities.wis, d->character->abilities.dex,
              d->character->abilities.con, d->character->abilities.cha);
      SEND_TO_Q(buf, d);
      SEND_TO_Q("\n\rIf you are not happy with your stats or the classes available type 'R' to reroll.\n\rOtherwise enter the class you would like or type '?' for help.\n\rClass:  ", d);
      /*Mtr's code ends here*/

      STATE(d) = CON_QCLASS;
      break;





/* 	set_state_age12: */
/* 	  sprintf(buf,"At age 12 (%s) your statistics were:\n\r",  */
/* 		  RaceName[GET_RACE(d->character)]); */
/* 	  SEND_TO_Q(buf, d); */
/* 	  SEND_TO_Q(nanny_stats(d->character, buf), d); */
/* 	  SEND_TO_Q("During the next year you:\n\r A) Adventured  B) Apprenticed" */
/* 		    "  C) Prayed  D) Studied  E) Trained F) Traveled\n\rChoose one (A-F):", d);  */
/* 	  STATE(d) = CON_QAGE12; */
/* 	  return; */

    case CON_QCLASS :
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if(is_number(arg))
	num=atoi(arg);
      else
	switch(*arg)
	  {
	  case '?':
	    SEND_TO_Q("\n\rSorry there is no help available at the moment\n\r", d);
	    goto print_classes;
	    break;
	  case 'r':
	  case 'R':
	    SEND_TO_Q("\n\rYou decide the shell the gods prepared for you is unworthy for your soul.\n\rYou begin to wait, as they grumblingly begin to forge yet another shell for\n\ryour soul to occupy.....\n\r",d);
	    goto Reroll;
	    break;
	  default:
	    SEND_TO_Q("\n\rThat's not a valid class, please try again.\n\r", d);
	    goto print_classes;
	    break;
	  }

      sprintf(buf, "%s: ", GET_IDENT(d->character));
      d->character->player.clss = 0;


      int i,j;
      counter = 0;             /* The counter of displayed races */

      /* Ok there is currently no way to save state information, so
	 what I have done is recalculate info we know from the
	 previous state.  What really should be done, when there is
	 time, is write some functions to save states.  i is the
	 magic offset index into the table for the appropriate race. */
      for (i=0; GET_RACE(d->character) != Classes_allowed_by_Race[i].race;
	   i++);

      current_class = Classes_allowed_by_Race[i].Classes;

      j=0;
      do {                  /* Figure out how much space to allocate */
	j++;
	current_class++;
      } while (current_class->class_name);  /* Not done with classes */

      current_class = Classes_allowed_by_Race[i].Classes;

      CREATE(class_list, struct class_choice, j);
      do {
	if (meets_class_requirements(&(d->character->abilities),
				     current_class))
	  {                           /* Qualifies to be this class */
	    i++;
	    (class_list + counter)->classes = current_class->class_type;
	    (class_list + counter ++)->playernum = i;
	  }
	current_class++;
      } while (current_class->class_name);  /* Not done with classes */


      if ( (num < 1) || (num > counter) )
	{
	  SEND_TO_Q("\n\rNumber out of range, please try again.\n\r", d);
	  goto print_classes;
	}

      /* Set the classes appropriately */
      d->character->player.clss = class_list[num-1].classes;
      FREE(class_list);

    set_state_align:
      /* Give the available alignment choices to the character */
      alignment = 0;

      /* Set up Allowed alignment Choices based on Race */
      for (i=0; i < MAX_PLAYER_RACE; i++)
	if (GET_RACE(d->character) == Race_Align[i].race)
	  alignment = Race_Align[i].align;

      /* Set up Allowed alignment Choices based on Classes */
      for (i=0; i <= MAX_LEVEL_IND; i++)
	if (IS_SET(d->character->player.clss,Class_Align[i].class_type))
	  {
	    alignment = alignment & Class_Align[i].align;
	  }

      if (alignment == 0)
	{
	  slog ("Allowed Race and Class combo has no allowed alignment");
	  alignment = NEUTRAL;   /* If we some how get here make em neut */
	}

      SEND_TO_Q("How did you find yourself behaving as you grew up?\n\r",d);
      count = 1;
      if (IS_SET(alignment,EXTREME_EVIL))
	{
	  sprintf(buf,"%d) Extremely Evil             ",count++);
	  SEND_TO_Q(buf,d);
	}
      if (IS_SET(alignment,EVIL))
	{
	  sprintf(buf,"%d) Evil                       ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}
      if (IS_SET(alignment,NEUTRAL_EVIL))
	{
	  sprintf(buf,"%d) Neutral Evil               ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}
      if (IS_SET(alignment,NEUTRAL))
	{
	  sprintf(buf,"%d) Neutral                    ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}
      if (IS_SET(alignment,NEUTRAL_GOOD))
	{
	  sprintf(buf,"%d) Neutral Good               ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}
      if (IS_SET(alignment,GOOD))
	{
	  sprintf(buf,"%d) Good                       ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}
      if (IS_SET(alignment,EXTREME_GOOD))
	{
	  sprintf(buf,"%d) Extremely Good             ",count);
	  SEND_TO_Q(buf,d);
	  if (!(count % 2))
	    SEND_TO_Q("\n\r",d);
	  count++;
	}

      SEND_TO_Q("\n\rBehavior:",d);
      STATE(d) = CON_QALIGN;
      return;
      break;



    case CON_QALIGN:
      slog("Align");

      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if(is_number(arg))
	num=atoi(arg);
      else
	{
	  SEND_TO_Q("\n\rThat's not a valid choice, try again.\n\r", d);
	  goto set_state_align;
	}


      /* Set up Allowed alignment Choices based on Race */
      for (i=0; i < MAX_PLAYER_RACE; i++)
	if (GET_RACE(d->character) == Race_Align[i].race)
	  alignment = Race_Align[i].align;

      /* Set up Allowed alignment Choices based on Classes */
      for (i=0; i <= MAX_LEVEL_IND; i++)
	if (IS_SET(d->character->player.clss, Class_Align[i].class_type))
	  alignment = alignment & Class_Align[i].align;


      if (alignment == 0)
	{
	  slog ("Allowed Race and Class combo has no allowed alignment");
	  alignment = NEUTRAL;   /* If we some how get here make em neut */
	}

      count = 0;
      for (i=0; i <= NUM_OF_ALIGN_CHOICES; i++)
	if (IS_SET(alignment,(1 << i)))
	  {
	    count++;
	    if (num == count)
	      {
		alignment = (1 << i);
		break;
	      }
	  }

      switch (alignment)
	{
	case EXTREME_EVIL:
	  d->character->specials.alignment = -1000;
	  break;
	case EVIL:
	  d->character->specials.alignment = -600;
	  break;
	case NEUTRAL_EVIL:
	  d->character->specials.alignment = -350;
	  break;
	case NEUTRAL:
	  d->character->specials.alignment = 0;
	  break;
	case NEUTRAL_GOOD:
	  d->character->specials.alignment = 350;
	  break;
	case GOOD:
	  d->character->specials.alignment = 600;
	  break;
	case EXTREME_GOOD:
	  d->character->specials.alignment = 1000;
	  break;
	default:
	  SEND_TO_Q("That is not a possible selection.\n\r", d);
	  goto set_state_align;
	  return;
	  break;
	}

      d->character->points.gold = number(15000,25000);   /* Forgot about gold */

      STATE(d) = CON_AUTH;


      if(STATE(d) == CON_AUTH)
	{
	  if(AUTH_PLAYER > 0)
	    d->character->act_ptr = NEWBIE_REQUEST+NEWBIE_CHANCES;
	  SEND_TO_Q("\n\rHit Enter\n\r", d);
	}
      break;

    case CON_AUTH:
      if (!AUTH_PLAYER || (player_count < 1) ||
	  (d->character->act_ptr == NEWBIE_START))
	{
	  SaveChar(d->character, AUTO_RENT, FALSE);

          if ((d->character->player.trust > 0) &&
			strcmp("Lansing",GET_NAME(d->character)))
            SEND_TO_Q(imotd, d);
          else
            SEND_TO_Q(motd, d);
	  SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
#ifdef SITELOCK
	  STATE(d) = CON_SITELOCK;
#else
	  STATE(d) = CON_RMOTD;
#endif
	}
      else if (d->character->act_ptr >= NEWBIE_REQUEST)
	{
	  sprintf(buf, "%s (%s) new player.",
		  GET_IDENT(d->character), d->host);
	  log_msg(buf);

	  sprintf(buf,"type Auth(orize) %s yes, to allow into game.",
		  GET_IDENT(d->character));
	  log_msg(buf);
	  /*	    log_msg("type 'Help Authorize' for other commands"); */

	  /*
	  **  enough for gods.  now player is told to shut up.
	  */
	  d->character->act_ptr--;	/* NEWBIE_START == 3 == 3 chances */

	  SEND_TO_Q(AUTH_MESSAGE, d);

	  sprintf(buf, "You have %d requests remaining.\n\r",
		  d->character->act_ptr);
	  SEND_TO_Q(buf, d);

	  if (d->character->act_ptr == 0)
	    {
	      SEND_TO_Q("Goodbye.", d);
	      free_char(d->character);
	      d->character = 0;
	      close_socket(d, TRUE);
	    }
	  else
	    {
	      SEND_TO_Q("Please Wait.\n\r", d);
	      STATE(d) = CON_AUTH;
	    }
	}
      else
	{				/* Axe them */
	  free_char(d->character);
	  d->character = 0;
	  close_socket(d, TRUE);
	}
      break;

    case CON_RMOTD:		/* read CR after printing motd        */

      /* let the start */
      SEND_TO_Q(MENU, d);

      STATE(d) = CON_SLCT;

      if (WizLock && !IS_GOD(d->character))
	{
	  sprintf(buf,
		  "Sorry, the game is locked because it is full or being reebooted\n\r"
		  "for repairs.  Please do NOT keep trying to log in.  If you do keep\n\r"
		  "trying, you will be banned from playing here anymore.\n\r");
	  SEND_TO_Q(buf,d);
	  STATE(d) = CON_WIZLOCK;
	}
      break;


      /* by MIN 1996 for BAN code and other */

    case CON_CLOSE:
      close_socket(d, TRUE);
      break;

    case CON_WIZLOCK:
      close_socket(d, TRUE);
      break;

    case CON_DELETE:    /* Check to see if they really wish to delete
			   themselves. */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (*arg == 'y' || *arg == 'Y')    /* They really want to end it all */
	{
	  DelChar(GET_REAL_NAME(d->character), TRUST(d->character));
	  close_socket(d, TRUE);
	}
      else
	{
	  if (*arg == 'n' || *arg == 'N') {
	    SEND_TO_Q("Ok, you won't be deleted then.\n\r", d);
	    SEND_TO_Q(MENU, d);
	    STATE(d) = CON_NME;
	  }
	  else /* Please do Y or N */
	    {
	      SEND_TO_Q("Please type Yes or No? ", d);
	      STATE(d) = CON_DELETE;
	    }
	}
      break;

    case CON_CITY_CHOICE:
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      if (d->character->in_room != NOWHERE) {
	SEND_TO_Q("This choice is only valid when you have been auto-saved\n\r",d);
	STATE(d) = CON_SLCT;
      } else {
	int		to_room;
	int		min_lev;

	switch (*arg)
	  {
	  case '1':
	    to_room = 3001;
	    min_lev = -1;
	  enter:
	    if(GetMaxLevel(d->character) > min_lev)
	      {
		enter_game(d, to_room);
		STATE(d) = CON_PLYNG;
	      }
	    else
	      {
		SEND_TO_Q("You aren't powerful enough for there.\n\r", d);
		STATE(d) = CON_SLCT;
	      }
	    break;
	  case '2':
	    to_room = 1102;
	    min_lev = -1;
	    goto enter;
	  case '3':
	    to_room = 18221;
	    min_lev = 5;
	    goto enter;
	  case '4':
	    to_room = 3606;
	    min_lev = 5;
	    goto enter;
	  case '5':
	    to_room = 16107;
	    min_lev = 5;
	    goto enter;
	  default:
	    SEND_TO_Q("That was an illegal choice.\n\r", d);
	    STATE(d) = CON_SLCT;
	    break;
	  }

      }
      break;

    case CON_SLCT:		/* get selection from main menu        */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);
      switch (*arg)          {
      case '0':
	close_socket(d, TRUE);
	break;

      case '1':
	{
	  int in_room = d->character->in_room;

	  if(in_room == NOWHERE ||
	     in_room == AUTO_RENT ||
	     in_room == 0 ||
	     !real_roomp(in_room))
	    in_room = GET_HOME(d->character);
	  enter_game(d, in_room);
	}

      break;

      case '2':
	SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n\r", d);
	SEND_TO_Q("Terminate with a '@'.\n\r", d);
	if (d->character->player.description)
	  {
	    SEND_TO_Q("Old description :\n\r", d);
	    SEND_TO_Q(ss_data(d->character->player.description), d);
	    ss_free(d->character->player.description);
	    d->character->player.description = 0;
	  }
	d->sstr = &d->character->player.description;
	d->max_str = 240;
	STATE(d) = CON_EXDSCR;
	break;

      case '3':
	SEND_TO_Q(story, d);
	STATE(d) = CON_SLCT;
	break;

      case '4':
	SEND_TO_Q("Enter a new password: ", d);

	write(d->descriptor, echo_off, 4);

	STATE(d) = CON_PWDNEW;
	break;

      case '5':
	if(IS_SET(d->character->delete_flag, PROTECT)) {
	  SEND_TO_Q("Your character is protected from deletion, please see a god to have this removed.\n\r", d);
	  SEND_TO_Q(MENU, d);
	  STATE(d) = CON_SLCT;
	} else {
	  REMOVE_BIT(d->character->delete_flag, DELETE);
	  SEND_TO_Q("Do you really wish to delete your character and equipment?.\n\r", d);
	  STATE(d) = CON_DELETE;
	}
	break;

      case '6':
	SEND_TO_Q("Where would you like to enter?\n\r", d);
	SEND_TO_Q("1.    Sanctuary\n\r", d);
	SEND_TO_Q("2.    Shire\n\r",    d);
	if (GetMaxLevel(d->character) > 5)
	  SEND_TO_Q("3.    Mordilnia\n\r", d);
	if (GetMaxLevel(d->character) > 10)
	  SEND_TO_Q("4.    New  Thalos\n\r", d);
	if (GetMaxLevel(d->character) > 20)
	  SEND_TO_Q("5.    The Gypsy Village\n\r", d);
	SEND_TO_Q("Your choice? ",d);
	STATE(d) = CON_CITY_CHOICE;
	break;

      default:
	SEND_TO_Q("Invalid option.\n\r", d);
	SEND_TO_Q(MENU, d);
	break;
      }
      break;

    case CON_PWDNEW:
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (!*arg || strlen(arg) > 10)      {
	write(d->descriptor, echo_on, 6);

	SEND_TO_Q("Illegal password.\n\r", d);
	SEND_TO_Q("Password: ", d);

	write(d->descriptor, echo_off, 4);


	return;
      }

      d->character->pwd = strdup((char*) crypt(arg,
					       GET_IDENT(d->character)));
      write(d->descriptor, echo_on, 6);

      SEND_TO_Q("Please retype password: ", d);

      STATE(d) = CON_PWDNCNF;
      write(d->descriptor, echo_off, 4);


      break;
    case CON_PWDNCNF:
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (strncmp(crypt(arg, d->character->pwd), d->character->pwd, 10))
	{
	  write(d->descriptor, echo_on, 6);
	  SEND_TO_Q("Passwords don't match.\n\r", d);
	  SEND_TO_Q("Retype password: ", d);
	  write(d->descriptor, echo_off, 4);

	  STATE(d) = CON_PWDNEW;
	  return;
	}
      write(d->descriptor, echo_on, 6);

      SEND_TO_Q(
		"\n\rDone. You must enter the game to make the change final\n\r",
		d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
      break;
    default:
      log_msg("Nanny: illegal state of con'ness");
      abort();
      break;
    }
}

void enter_game(struct descriptor_data* d, int to_room)
{
  char buf[256];
  int time_off;
  struct char_data* ch = d->character;

  sprintf(buf, "Loading %s's equipment", GET_IDENT(ch));
  slog(buf);
  WriteToImmort(buf, ch->invis_level, LOG_PLAYER);

  ch->desc = d;

  if (IS_SET( ch->specials.flags, PLR_DENY ))
  {
    send_to_char("\n\rYou have been denied access.\n\r", ch);
    close_socket(ch->desc, TRUE);
    return;
  } /* maarek */

  if(ChargeRent(ch))
    /* if they run out of rent, just load the aliases */
    LoadChar(ch, GET_IDENT(ch), READ_ALIASES);
  else
    /* otherwise we load everything else in the file */
    LoadChar(ch, GET_IDENT(ch), READ_SECOND);

  /* if we crashed or shutdown while they were link dead, the invis
     level will have been saved as WizInvis, fix it here */
  if(!IS_IMMORTAL(ch))
    ch->invis_level = 0;

  /* if been gone more than 24 hours kill all the affects */
  time_off = time(0) - ch->player.time.logon;
  if(time_off >= (24 *60 * 60))
    affect_remove_all(ch);

  /* if they rented out heal them for the time they've been gone */
  time_off /= SECS_PER_MUD_HOUR;
  if((ch->in_room > 0) && (time_off > 0))
    {
      int gain, limit, gained = 0;

      time_off *= 2;		/* double normal healing rate */

      GET_POS(ch) = POSITION_SLEEPING;

      gain = mana_gain(ch) * time_off + GET_MANA(ch);
      limit = mana_limit(ch);
      if(GET_MANA(ch) != limit)
	{
	  GET_MANA(ch) = MIN(limit, gain);
	  gained = 1;
	}

      gain = hit_gain(ch) * time_off + GET_HIT(ch);
      limit = hit_limit(ch);
      if(GET_HIT(ch) != limit)
	{
	  GET_HIT(ch) = MIN(limit, gain);
	  gained = 1;
	}

      gain = move_gain(ch) * time_off + GET_MOVE(ch);
      limit = move_limit(ch);
      if(GET_MOVE(ch) != limit)
	{
	  GET_MOVE(ch) = MIN(limit, gain);
	  gained = 1;
	}

      GET_POS(ch) = POSITION_STANDING;

      if(gained)
	send_to_char("You recovered while you rested.\n\r", ch);
    }

  /* save to make sure everything's still consistent */
  SaveChar(ch, AUTO_RENT, TRUE);

  send_to_char(WELC_MESSG, ch);

  /* let them know what the pkill status is */
  if(PKILLABLE==2)
    send_to_char("Arena PKILL is turned On! Go House!\n\r",ch);

  array_insert(&character_list, ch);

  if((to_room == NOWHERE) ||
     (to_room == AUTO_RENT) ||
     (to_room == 0) ||
     (to_room == 3) ||
     !real_roomp(to_room))
    to_room = IS_GOD(ch) ? 1000 : 3001;

  char_to_room(ch, to_room);

  ch->specials.tick = plr_tick_count++;
  if (plr_tick_count == PLR_TICK_WRAP)
    plr_tick_count=0;

  if(GET_HIT(ch) < 1)
    {
      GET_HIT(ch) = 1;
      update_pos(ch);
    }

  REMOVE_BIT(ch->specials.mob_act, ACT_PATTACK);
  REMOVE_BIT(ch->delete_flag, HOUSED);

  act("$n has entered the game.",
      TRUE, ch, 0, 0, TO_ROOM);
  if (!GetMaxLevel(ch))
    do_start(ch);
  do_look(ch, "",15);

  send_to_char("\n\r", ch);
  if(IS_IMMORTAL(ch))
    list_pages(ch, 0);
  do_mail_check(ch, "", 0);
  d->prompt_mode = 1;

  STATE(d) = CON_PLYNG;
  connected++;
  if (connected>max_connected)
    max_connected=connected;

  switch(ch->drop_count) {
   case 1:
     cprintf(ch, "You auto-rented after dropping link last session!\n\r"
	         "Be certian to rent out this time.\n\r");
     break;
   case 2:
     cprintf(ch, "You have auto-rented 2 times in a row!\n\r"
	         "Next time you will lose items! Please rent now!\n\r");
     break;
   case 3:
     cprintf(ch, "Due to abuse of the auto-rent system, you have\n\r"
	         "lost items from your character. Rent next time.\n\r");
     break;
  }

/* Un-comment the next secition if you wish to log the logging in of multis. */

/*

  if(!IS_GOD(ch)
     && ( ( (HowManyClasses(ch) > 1) && (ch->player.clss>15) )
	  || (HowManyClasses(ch) > 3) ) )
    {
      sprintf(buf, "Dog-pile on the multi!  %s", GET_IDENT(ch));
      log_msg(buf);
      send_to_char("A chill runs down your spine.  Perhaps the gods are displeased?\n\r",   ch);
    }

    */

  //check for character entering.
  TrackingSystem.CheckChar(ch);
}

static char TimeMessage[] =
"Unfortunately, Drexel has required that we not accept connections\n\r"
"from mortals during the hours of 8 AM - 6 PM, Mon - Fri.  While these\n\r"
"restrictions are in place, rent on items that are not limited has\n\r"
"eliminated.  Please call back after 6.\n\r";

int TimeCheck(struct descriptor_data* desc)
{
  time_t	now;
  struct tm*	tm;
  char* 	tstr;
  extern int	lawful;

  if(!lawful)
    return 0;

  if(desc->character && IS_GOD(desc->character))
    return 0;

  time(&now);
  tm = localtime(&now);

  if((tm->tm_wday > 0) &&	/* sunday */
     (tm->tm_wday < 6) &&	/* saturday */
     (tm->tm_hour > 8) &&	/* 8 AM */
     (tm->tm_hour < 18))	/* 6 PM */
    {
      write(desc->descriptor, TimeMessage, sizeof(TimeMessage) - 1);
      tstr = ctime(&now);
      write(desc->descriptor, tstr, strlen(tstr));
      write(desc->descriptor, "\n\r\n\r", 4);
      return 1;
    }

  return 0;
}


/* added by Min 1996 for BAN code */

/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg);
  /* :-) */
}
