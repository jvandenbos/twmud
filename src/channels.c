#include "config.h"
#include <stdio.h>
#include "structs.h"
#include "utils.h"
#include "channels.h"
#include "db.h"
#include "util_str.h"
#include "comm.h"
#include "utility.h"
#include "util_str.h"
#include "multiclass.h"
#include "proto.h"
#include "cmdtab.h"
#include <string.h>
 
/* game variable for quest control */
int questinfo = 0;

typedef struct
{
    int  cmd_no;
    int  (*filter_proc)(struct char_data* sender, 
			struct char_data* receiver);
    int  mana_cost;
    char *mess_fmt;
    char *channel_name;
    int	noisy;
    int  flag;
} channel_info;

/* forward declarations */
static int flt_think(struct char_data* sender, struct char_data* receiver);
static int flt_imp(struct char_data* sender, struct char_data* receiver);
static int flt_shout(struct char_data* sender, struct char_data* receiver);
static int flt_auction(struct char_data* sender, struct char_data* receiver);
static int flt_vote(struct char_data* sender, struct char_data* receiver);
static int flt_rumor(struct char_data* sender, struct char_data* receiver);
static int flt_guild(struct char_data* sender, struct char_data* receiver);
static int flt_holler(struct char_data* sender, struct char_data* receiver);
static int flt_quest(struct char_data* sender, struct char_data*receiver);     
static int flt_swear(struct char_data* sender, struct char_data*receiver);      
static int flt_trivia(struct char_data* sender, struct char_data*receiver);
static int flt_rprumor(struct char_data* sender, struct char_data*receiver);
static int flt_brujah(struct char_data* sender, struct char_data*receiver);
static int flt_newbie(struct char_data* sender, struct char_data*receiver);

static void tell_channel(struct char_data* ch,
			 channel_info* channel,
			 char* argument);
static void who_channel(struct char_data* ch, channel_info* channel);

static channel_info channels[] =
{
    { 195, flt_think,   0, "$Cc%s%s $CNthinks '%s'\n\r",
	  "think",   0, COM_THINK},
    { 500, flt_think,   0, "$Cc%s%s $CNthinks '%s'\n\r",
	  "think",   0, COM_THINK},
    { 399, flt_imp,     0, "$Cy%s%s: $CN%s\n\r",
          "imp",     0, COM_IMP},
    {  18, flt_shout,   10, "%s%s $Cmshouts $CN'%s'\n\r",
	   "shout",  1, COM_SHOUT},
    { 415, flt_auction, 0, "%s%s auctions '%s'\n\r",
	  "auction", 1, COM_AUCTION},
    { 416, flt_rumor,   0, "%s%s $Cmrumors $CN'%s'\n\r",
	  "rumor",   1, COM_RUMOR},
    { 346, flt_think,   0, "[%s %s]\n\r",
	  "log",     1, COM_LOG},
    { 441, flt_guild,   0, "%s%s guild speaks '%s'\n\r",
	  "gcomm",     1, COM_GUILD},
    { CMD_HOLLER, flt_holler, 10, "%s%s $Cmhollers $CN'%s'\n\r",
          "holler", 1, COM_HOLLER},
    { 503, flt_quest,   10, "$Cg%s%s $CNquests '%s'\n\r",
          "quest",   0, COM_QUEST},
    { 504, flt_swear,   10, "$Cg%s%s $Crswears $CN'%s'\n\r",
          "swear",   0, COM_SWEAR},
    { 505, flt_trivia,  0,  "%s%s $Cctrivias $CN'%s'\n\r",
	  "trivia",  0, COM_TRIVIA}, 
    { 542, flt_rprumor,  0,  "%s%s $Cgrumors (IC) $CN'%s'\n\r",
	  "rumor (IC)",  0, COM_RPRUMOR}, 
    { 546, flt_brujah,   0,  "$Cw%s%s mindlinks $Cr'%s'$CN\n\r",
          "brujah",      0, COM_BRUJAH}, 
    { 548, flt_newbie,	 0, "$Cw%s%s newbies $CN'%s'\n\r",
	  "newbie",	 0, COM_NEWBIE},
    { 565, flt_vote, 0, "%s%s votes '%s'\n\r",
	  "vote", 1, COM_VOTE},
    {   0, 0,           0, 0,
	   0,        0, 0}
};

channel_info *find_channel (int cmd)
{
    channel_info* channel;
    
    for(channel = channels ; channel->cmd_no ; channel++)
	if(channel->cmd_no == cmd)
	    return channel;
    return NULL;
}

int flt_guild (struct char_data *ch, struct char_data *vict)
{
  
  /* guild == 0 means you can't hear guild at all */
  if (!vict->player.guildinfo.inguild() || 
	!ch->player.guildinfo.inguild())
    return 0;
  
if (ch->player.guildinfo.inguild() ==
	vict->player.guildinfo.inguild()) return 1;
else return 0;

}

int flt_brujah (struct char_data *ch, struct char_data *vict)
{

  if (!IS_SET(ch->specials.flags, PLR_BRUJAH) ||
        (!IS_SET(vict->specials.flags, PLR_BRUJAH)))
     return 0;

  if (IS_SET(ch->specials.flags, PLR_BRUJAH) ||
	(IS_SET(vict->specials.flags, PLR_BRUJAH)))
     return 1;
  else return 0;
}

int flt_think (struct char_data *ch, struct char_data *vict)
{
    return IS_GOD(vict);
}

int flt_imp (struct char_data *ch, struct char_data *vict)
{
    return TRUST(vict) > TRUST_GRGOD;
}

int flt_shout (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;
    
    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (rp = real_roomp(vict->in_room)) &&
	   !IS_SET(rp->room_flags, SILENCE) &&
	   (GET_POS(vict) >= POSITION_RESTING));
}

int flt_auction (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;
    
    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (rp = real_roomp(vict->in_room)) &&
	   !IS_SET(rp->room_flags, SILENCE));
}

int flt_vote (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;
    
    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (rp = real_roomp(vict->in_room)) &&
	   !IS_SET(rp->room_flags, SILENCE));
}

int flt_trivia (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;

    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
           (rp = real_roomp(vict->in_room)) &&
           !IS_SET(rp->room_flags, SILENCE) &&
           (GET_POS(vict) >= POSITION_RESTING));
}

int flt_rumor
(struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;
    
    return 
	   (GetMaxLevel(ch) > 15) &&
	   (!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (rp = real_roomp(vict->in_room)) &&
	   !IS_SET(rp->room_flags, SILENCE) &&
	   (GET_POS(vict) >= POSITION_RESTING));
}

int flt_newbie
(struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;

    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
           (rp = real_roomp(vict->in_room)) &&
           !IS_SET(rp->room_flags, SILENCE) &&
           (GET_POS(vict) >= POSITION_RESTING));
}


int flt_holler (struct char_data *ch, struct char_data *vict)
{
    struct room_data *crp = NULL, *vrp = NULL;

    if (!ch)
      return(TRUE);

    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (vrp = real_roomp(vict->in_room)) &&
	   (crp = real_roomp(ch->in_room)) &&
	   !IS_SET(vrp->room_flags, SILENCE) &&
	   (GET_POS(vict) >= POSITION_RESTING) &&
	   (vrp->zone == crp->zone));
}

int flt_quest (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;

  if (questinfo == 0) return 0;
  else {

    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
           (rp = real_roomp(vict->in_room)) &&
           !IS_SET(rp->room_flags, SILENCE) &&
           (GET_POS(vict) >= POSITION_RESTING));
       }
}

int flt_swear (struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;

    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
           (rp = real_roomp(vict->in_room)) &&
           !IS_SET(rp->room_flags, SILENCE) &&
           (GET_POS(vict) >= POSITION_RESTING));
}

int flt_rprumor
(struct char_data *ch, struct char_data *vict)
{
    struct room_data* rp;
    
    return(!IS_AFFECTED(vict, AFF_SILENCE) &&
	   (rp = real_roomp(vict->in_room)) &&
	   !IS_SET(rp->room_flags, SILENCE) &&
	   (GET_POS(vict) >= POSITION_RESTING));
}

void do_comm(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];
  channel_info *channel;
    
  if (ch->master && IS_AFFECTED(ch, AFF_CHARM))
  {
    send_to_char("You are charmed!", ch);
    return;
  }
  if(IS_SET(ch->specials.flags, PLR_NOSHOUT))
  {
    send_to_char("You've been noshouted, remember?", ch);
    return;
  }
    
  channel = find_channel(cmd);

  if(!channel)
  {
    sprintf(buf, "illegal cmd (%d) calling comm function", cmd);
    log_msg(buf);
    send_to_char("Channels are experiencing technical difficulty.", ch);
    return;
  }

  for (; *argument==' '; argument++);

  if(IS_SET(ch->channels, channel->flag))
  {
    REMOVE_BIT(ch->channels, channel->flag);
    sprintf(buf, "You will now hear the %s channel.\n\r",
	    channel->channel_name);
    send_to_char(buf, ch);
  }
  else if (!*argument)
  {
    if(!IS_SET(ch->channels, channel->flag))
    {
      SET_BIT(ch->channels, channel->flag);
      sprintf(buf, "You will not hear the %s channel anymore.\n\r",
	      channel->channel_name);
      send_to_char(buf ,ch);
    }
  }

  if (!(*channel->filter_proc)(ch, ch))
  {
    send_to_char("You wouldn't be able to hear yourself.\n\r", ch);
    return;
  }

  if(!strcmp(argument, "who"))
  {
    who_channel(ch, channel);
  }
  else if(*argument)
  {
    tell_channel(ch, channel, argument);
  }
}

static void tell_channel(struct char_data* ch,
			 channel_info* channel,
			 char* argument)
{
  char plyr_buf[MAX_STRING_LENGTH];
  char gods_buf[MAX_STRING_LENGTH];
  char some_buf[MAX_STRING_LENGTH];
  char poly_buf[50];
  struct descriptor_data* i;
  struct char_data* vict;

  if (channel->noisy && !IS_IMMORTAL(ch) && apply_soundproof(ch))
    return;

  if(!IS_IMMORTAL(ch))
  {
    if (channel->mana_cost && (GET_MANA(ch)<channel->mana_cost))
    {
      send_to_char("You are too low on energy to do that.\n\r", ch);
      return;
    }
    GET_MANA(ch) -= channel->mana_cost;
  }

  if (IS_SET(ch->specials.flags, PLR_ECHO))
  {
    sprintf(plyr_buf, "$CgYou %s$Cy '%s'$CN\n\r", channel->channel_name, 
argument);
    send_to_char_formatted(plyr_buf, ch);
  }

  argument = escape(argument);
    
  if(IS_SET(ch->specials.mob_act, ACT_POLYSELF) || IS_NPC(ch))
    sprintf(poly_buf, "$Cg (%s)$CN", GET_NAME(ch));
  else
    poly_buf[0] = 0;

  if(IS_NPC(ch))
    sprintf(plyr_buf, channel->mess_fmt, GET_NAME(ch), "$Cm",       argument);
  else
    if(!IS_IMMORTAL(ch))
      sprintf(plyr_buf, channel->mess_fmt, GET_REAL_NAME(ch), "$Cm", argument);
    else
      sprintf(plyr_buf, channel->mess_fmt, GET_NAME(ch), "$Cm", argument);
  sprintf(gods_buf, channel->mess_fmt, GET_REAL_NAME(ch), poly_buf, argument);
  sprintf(some_buf, channel->mess_fmt, "Someone",    "$Cm",       argument);

  EACH_DESCRIPTOR(d_iter, i)
  {
    vict = i->character;
    if (vict != ch && !i->connected &&
	(*channel->filter_proc)(ch, vict) &&
	!IS_SET(vict->channels, channel->flag) &&
	!IS_WRITING(vict))
    {
      if(!CAN_SEE(vict, ch))
	send_to_char_formatted(some_buf, vict);
      else if(IS_IMMORTAL(vict))
	send_to_char_formatted(gods_buf, vict);
      else
	send_to_char_formatted(plyr_buf, vict);
    }
  }
  END_ITER(d_iter);
}

static void who_channel(struct char_data* ch, channel_info* channel)
{
  struct descriptor_data* i;
  struct char_data* vict;
  char buf[256];
  int	god_flag = IS_GOD(ch);
  
  EACH_DESCRIPTOR(d_iter, i)
  {
    vict = i->character;
    if (!i->connected &&
	!IS_SET(vict->channels, channel->flag) &&
	(*channel->filter_proc)(ch, vict) &&
	CAN_SEE(ch, vict))
    {
      if(IS_NPC(vict))
      {
	if(god_flag)
	  sprintf(buf, "%s (%s)\n\r", GET_NAME(vict), GET_REAL_NAME(vict));
	else
	  sprintf(buf, "%s\n\r", GET_REAL_NAME(vict));
      }
      else
	sprintf(buf, "%s %s\n\r", GET_REAL_NAME(vict), GET_TITLE(vict));
      send_to_char(buf, ch);
    }
  }
  END_ITER(d_iter);
}

void do_channels(struct char_data* ch, char* argument, int cmd)
{
    char		buf[256];
    channel_info*	channel;

    for(channel = channels ; channel->cmd_no ; channel++)
    {
	if(channel->filter_proc(ch, ch))
	{
	    sprintf(buf, "%-15s %s\n\r", channel->channel_name,
		    IS_SET(ch->channels, channel->flag) ? "blocked" : "heard");
	    send_to_char(buf, ch);
	}
    }
}

void do_deafen(struct char_data* ch, char* argument, int cmd)
{
    if(ch->channels & COM_DEAF_MASK)
    {
	ch->channels &= ~COM_DEAF_MASK;
	send_to_char("You will hear mortal channels now\n\r", ch);
    }
    else
    {
	ch->channels |= COM_DEAF_MASK;
	send_to_char("You will not hear mortal channels now\n\r", ch);
    }
}

void do_shout(struct char_data* ch, char* arg, int cmd)
{
    do_comm(ch, arg, 18);
}

ACMD(do_qcontrol) {
  char field[20];

  if(!*arg) {
    do_qcontrol(ch,"help",666);
    return;
  } else {
    arg = one_argument(arg,field);
  }

  if (is_abbrev(field,"help")) {
    send_to_char("qcontrol help -- Shows this page.\n\r",ch);
    send_to_char("qcontrol activate -- Turns quest functions on.\n\r",ch);
    send_to_char("qcontrol deactivate -- Turns quest functions off.\n\r",ch);
    return;
  } else if (is_abbrev(field,"activate")) {
    questinfo = 1;
    send_to_char("The quest functions are now activated.\n\r",ch);
  } else if (is_abbrev(field,"deactivate")) {
    questinfo = 0;
    send_to_char("The quest functions are now de-activated.\n\r",ch);     
    return;
  }
}

