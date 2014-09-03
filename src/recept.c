#include "config.h"

#include <stdio.h>
#if USE_unistd
#include <unistd.h>
#endif
#if USE_stdlib
#include <stdlib.h>
#endif
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "db.h"
#include "recept.h"
#include "utils.h"
#include "utility.h"
#include "comm.h"
#include "spec.h"
#include "race.h"
#include <time.h>
#include "master.h"
#include "util_str.h"
#include "magicutils.h"
#include "multiclass.h"
#include "handler.h"
#include "act.h"
#include "interpreter.h"
#include "vnum.h"
#include "hero.h"
#include "cmdtab.h"
#include "proto.h"
#include "trackchar.h"
#include "spell_events.h"

#ifdef __cplusplus
extern "C"
{
#endif

  //    int time(int*);
    char* crypt(const char*, const char*);
    
#ifdef __cplusplus
}
#endif

SPECIAL(HOUSE);
int receptionist(void *me, struct char_data *ch, int cmd, char *arg);
static void drop_unrented_obj(struct char_data* ch, struct obj_data* obj);
bool excluded_item(struct obj_data *obj);

typedef struct 
{
  int	in_room;
  int	normal;
  int	limited;
} rent_info_t;

rent_info_t rent_info[] = 
{
  {	 3008,	 	 0,	0	}, /* midgaard */
  {	16124,	 	 0,	0	}, /* gypsy */
  {	11026,	 	 0,	0	}, /* python */
  {	 3601,	 	 0,     0	}, /* new thalos */
  {	18215,	 	 0,     0	}, /* mordilnia */
  { AUTO_RENT,		 0,	0	},
  { RM_MORGUE_HOLDING,         30,   60,      },
    
  {	    0,		 0,	0 	}
};
  
int SaveChar(struct char_data* ch, int in_room, int get_offer)
{
    int status = 0;
    struct char_data*	rch;
    struct obj_data*	was_equip[MAX_WEAR];
    int	i;
    char file[256], new_file[256], bak_file[256], buf[256], eq_file[256];
    char *name;
    FILE* fp, *eqfp=NULL;
    int hit, mana, move, old_room;
    spevent_list *sp;
    
    if(!IS_PC(ch))
	return 1;
    
    rch = real_character(ch);
    hit = GET_HIT(ch);
    mana = GET_MANA(ch);
    move = GET_MOVE(ch);
    old_room = ch->in_room;
    
    if(get_offer)
    {
	struct obj_cost cost;
	
	OfferChar(ch, &cost, in_room, FALSE);
    }
    
    if(rch != ch)
    {
	for(i = 0 ; i < MAX_WEAR ; ++i)
	    was_equip[i] = ch->equipment[i];
	
	SwitchStuff(ch, rch);
	
	rch->desc = ch->desc;
    }
    
    name = lower(GET_IDENT(rch));
    sprintf(file, "players.d/%c/%s", *name, name);
    sprintf(new_file, "players.d/%c/%s.tmp", *name, name);
    sprintf(bak_file, "players.d/%c/%s.bak", *name, name);
    sprintf(eq_file,  "eqlist.d/%c/%s", *name, name);
    
    if(!(fp = fopen(new_file, "w")))
    {
	sprintf(buf, "Can't create %s", new_file);
	perror(buf);
	log_msg(buf);
	status = -1;
	goto cleanup;
    }
    eqfp = fopen(eq_file, "w");
    
    setvbuf(fp, 0, _IOFBF, 8192);
	
    ch->in_room = in_room;

    /* update host information */
    if (ch->desc)
    {
	if (ch->specials.hostname)
	    FREE(ch->specials.hostname);
	ch->specials.hostname = strdup(ch->desc->host);
    }

    //spell events before save
    for(sp=ch->sp_list;sp;sp=sp->next) {
       if(sp->sp_event->bsave_func) {
	  (sp->sp_event->bsave_func)((event_t *)sp->sp_event, 0);
       }
    }

    /*
      This call was put here for a reason sometime, but as far as I can tell it's not necessary.
      Actually it would just be a great bugger to find when certain skills (like pulse)
      crash, since their event-code invoked save implicitly during a call to damage-function.
      --Mnemosync

      Yup, very bad place to put this. Mud crashes on charge elements if you do not remove spevents
      when you rent or die though. Make sure raw_kill, OfferChar and receptionist rent removes events
      before saving char. -- Raist
    */
    //spevent_remove_all_char(ch); 
   
    if(write_player(rch, fp, eqfp) || (fflush(fp), ferror(fp)))
    {
	sprintf(buf, "Error saving %s", GET_NAME(ch));
	log_msg(buf);
	status = -1;
    }
    if (TRUST(ch) > 0) //we have a god, lets save them
    {
       char tempBuf[256];
       sprintf(tempBuf, "cp %s gods.d/%c/%s", file, *name, name);
       system(tempBuf);
    }
    fclose(fp);
    if(eqfp) fclose(eqfp);
   
    //spell events after save
    for(sp=ch->sp_list;sp;sp=sp->next) {
       if(sp->sp_event->asave_func) {
	  (sp->sp_event->asave_func)((event_t *)sp->sp_event, 0);
       }
    }
    
 cleanup:

    ch->in_room = old_room;
    
    if(rch != ch)
    {
	SwitchStuff(rch, ch);
	rch->desc = 0;
	
	for(i = 0 ; i < MAX_WEAR ; ++i)
	{
	    if(was_equip[i])
	    {
		obj_from_char(was_equip[i]);
		raw_equip(ch, was_equip[i], i);
	    }
	}
    }
    
    GET_HIT(ch) = hit;
    GET_MOVE(ch) = move;
    GET_MANA(ch) = mana;
    
    if(status)
	return status;
    
    if(!access(bak_file, F_OK) && unlink(bak_file))
    {
	sprintf(buf, "Can't unlink %s", bak_file);
	log_msg(buf);
	perror(buf);
	return -1;
    }
    
    if(!access(file, F_OK) && rename(file, bak_file))
    {
	sprintf(buf, "Can't rename(%s,%s)", file, bak_file);
	log_msg(buf);
	perror(buf);
	return -1;
    }
    
    if(rename(new_file, file))
    {
	sprintf(buf, "Can't rename(%s, %s)", new_file, file);
	log_msg(buf);
	perror(buf);
	return -1;
    }

    if(!access(bak_file, F_OK) && unlink(bak_file))
    {
      sprintf(buf, "Can't unlink(%s) after save", bak_file);
      log_msg(buf);
      perror(buf);
    }
    
    return 0;
}

void DelChar(const char* name, const byte trust)
{
  char          file[256], buf[256], file2[256], buf2[256], file3[256];
  char          *ln;
    
  ln = lower(name);
  sprintf(file, "players.d/%c/%s", *ln, ln);
  sprintf(file2, "gods.d/%c/%s", *ln, ln);
  sprintf(file3, "eqlist.d/%c/%s", *ln, ln);

  if (trust > 0)
  {  
    if (remove(file2))
      sprintf(buf2,"Could not delete %s",file2);
    else
      sprintf(buf2,"Deleting %s",file2);
    log_msg(buf2);
  };

  if (remove(file))
    sprintf(buf,"Could not delete %s",file);
  else
    sprintf(buf,"Deleting %s",file);
   
  remove(file3);

  log_msg(buf);
  TrackingSystem.DeleteCharConst(name);
}

struct char_data* LoadChar(struct char_data* ch, const char* name, int parts)
{
    FILE*	fp;
    char	file[256], buf[256];
    char*	ln;
    
    ln = lower(name);
    sprintf(file, "players.d/%c/%s", *ln, ln);
    
    if(!(fp = fopen(file, "r")))
	return NULL;
    
    setvbuf(fp, 0, _IOFBF, 8192);
	
    if(!ch)
    {
	CREATE(ch, struct char_data, 1);
	clear_char(ch);
    }
    
    if(!ch->aliases)
	clear_aliases(ch);

    if(!ch->skills)
	SpaceForSkills(ch);
    
    if(read_player(ch, fp, parts))
    {
	sprintf(buf, "Error reading char %s", name);
	perror(buf);
	log_msg(buf);
	free_char(ch);
	ch = NULL;
    }

    if (TRUST(ch) > 0)
    {
      char tempBuf[256];

      name = GET_NAME(ch);
      sprintf(tempBuf, "cp %s gods.d/%c/%s", file, *ln, ln);
      system(tempBuf);
    }
    /* for the TWO Handed Stuff */

    if (ch->equipment[HOLD] && IS_OBJ_STAT(ch->equipment[HOLD],ITEM_TWO_HANDED))
      obj_to_char(unequip_char(ch,HOLD), ch);
    
    if (ch->equipment[WIELD] && IS_OBJ_STAT(ch->equipment[WIELD],ITEM_TWO_HANDED)) 
      if (ch->equipment[HOLD]) 
	obj_to_char(unequip_char(ch,HOLD), ch);

    fclose(fp);

    if(!IS_NPC(ch))
	add_char_to_hero_list(ch);

    return ch;
}

bool excluded_item(struct obj_data *obj) {
   
   int vnum;
   
   vnum = obj_index[obj->item_number].virt;
   
  if (vnum == 22006) /* QUEST TOKEN */
    return true; 
  if (vnum == 22051) /* Trivia token */
   return true;
  if (GET_ITEM_TYPE(obj) == ITEM_FOOD)
    return true;
  if (GET_ITEM_TYPE(obj) == ITEM_POTION)
    return true;
  if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON)
    return true;
  if (GET_ITEM_TYPE(obj) == ITEM_SCROLL)
    return true;
  
   if ((vnum >= 5230) &&
       (vnum <= 5244))
     return true;
  
   if ((vnum > 8715) && /* PLANTS */
       (vnum < 8750))
     return true;
   
  return false; /* not an excluded item */
}

void add_obj_cost(struct char_data *ch, struct char_data *re,
                  struct obj_data *obj, struct obj_cost *cost,
		  int rent_reg, int rent_lim)
{
    char buf[MAX_INPUT_LENGTH];
    int  temp;
    struct obj_data* next_obj;
    
    /* Add cost for an item and it's contents, and next->contents */
    
    if (obj)
    {
	next_obj = obj->next_content;
	if(obj->obj_flags.cost_per_day < 0)
	{
	    if (re)
	    {
		act("$n tells you 'I refuse storing $p'",
		    FALSE,re,obj,ch,TO_VICT);
	    }
	}	    
	else
	{
	    temp = obj->obj_flags.cost_per_day;
	    if(IS_SET(obj->obj_flags.extra_flags, ITEM_RARE))
	    {
		temp = temp * rent_lim / 100;
		cost->lims++;
	    }
	    else
		temp = temp * rent_reg / 100;
	    cost->total_cost += temp;
	    if (re)
	    {
		sprintf(buf, "%30s : %d coins/day", OBJ_SHORT(obj), temp);
		if(IS_SET(obj->obj_flags.extra_flags, ITEM_RARE))
		    strcat(buf,"    (limited item)");
		strcat(buf,"\n\r");
		send_to_char(buf, ch);
	    }
	    if (!excluded_item(obj))
	      cost->no_carried++;
	    add_obj_cost(ch, re, obj->contains, cost, rent_reg, rent_lim);
	}
	add_obj_cost(ch, re, next_obj, cost, rent_reg, rent_lim);
    }
}

int OfferChar(struct char_data* ch, struct obj_cost* cost, int room, int noisy)
{
  char buf[256];
  struct char_data*	recep;
  int i, everything;
  rent_info_t* ri;
  rent_info_t* ri2;  /* Added for AutoRent calculation if noisy -- PAC */
  struct obj_cost* autocost;
  
  if(noisy)
  {
    struct room_data*	roomp = real_roomp(room);
	
    if(roomp && (roomp->funct == (spec_proc_func) House))
      recep = FindMobInRoomWithFunction(3008, receptionist);
    else
      recep = FindMobInRoomWithFunction(room, receptionist);
    if(!recep)
    {
      send_to_char("You can't seem to find a receptionist\n\r", ch);
      return -1;
    }
  }
  else
    recep = NULL;
    
  cost->total_cost = 0;
  cost->no_carried = 0;
  cost->lims = 0;
  CREATE(autocost, struct obj_cost ,1);
  autocost->total_cost=0;
  autocost->no_carried=0;
  autocost->lims=0;

  for(ri = rent_info ; ri->in_room && (ri->in_room != room) ; ri++)
    ;

  for(ri2 = rent_info ; ri2->in_room && (ri2->in_room != AUTO_RENT) ; ri2++)
    ;
  
  for(i = 0 ; i < MAX_WEAR ; ++i)
  {
    if(ch->equipment[i])
    {
      add_obj_cost(ch, recep, ch->equipment[i], cost, ri->normal, ri->limited);
      add_obj_cost(ch, 0, ch->equipment[i], autocost, ri2->normal,
		   ri2->limited);
    }
  }
    
  if(ch->carrying)
  {
    add_obj_cost(ch, recep, ch->carrying, cost, ri->normal, ri->limited);
    add_obj_cost(ch, 0, ch->carrying, autocost, ri2->normal, ri2->limited);
  }
    
  if(recep)
  {
    sprintf(buf, "$n tells you 'Rent is now at %d%% for regular items, %d%% for limited items!'", ri->normal, ri->limited);
    act(buf, FALSE, recep, 0, ch, TO_VICT);
    sprintf(buf, "$n tells you 'It will cost you %d coins per day'\n"
	    "$n tells you 'or %d to checkin for long-term storage'\n"
	    "$n tells you 'or %d coins to autorent per day'",
	    cost->total_cost, cost->total_cost * LONG_TERM_MULT,
	    autocost->total_cost);
    act(buf,FALSE,recep,0,ch,TO_VICT);
  }
  FREE(autocost);
  
  ch->rent_cost = cost->total_cost;
    
#if PLUNDER_BANK
  everything = GET_GOLD(ch) + GET_BANK(ch);
#else
  everything = GET_GOLD(ch);
#endif
  if (cost->total_cost > everything)
  {
    if (!IS_GOD(ch))
    {
      if(recep)
	act("$n tells you 'Which I can see you can't afford'",
	    FALSE,recep,0,ch,TO_VICT);
      return 1;
    }
    else
    {
      if(recep)
	act("$n tells you 'Well, since you're a God, I guess it's okay'",
	    FALSE,recep,0,ch,TO_VICT);
      cost->total_cost = 0;
    }
  }
    
  if (cost->no_carried > MAX_OBJ_SAVE)
  {
    if (recep) {
      sprintf(buf,
	      "$n tells you 'Sorry, but I can't store more than %d items. You currently have %d.",
	      MAX_OBJ_SAVE, cost->no_carried);
      act(buf,FALSE,recep,0,ch,TO_VICT);
    }
    return 2;
  }
  
  return 0;
}

/*
  house routine for saved items.
  */

SPECIAL(House)
{
    char buf[100];
    struct obj_cost cost;

    if (type == SPEC_INIT)
	return (FALSE);
  
    if (IS_NPC(ch)) return(FALSE);
  
    /* if (cmd != rent) ignore */
    if (cmd != 92)
	return(FALSE);
    
    /*  verify the owner */
    if (strncmp(GET_IDENT(ch), real_roomp(ch->in_room)->name, 
		strlen(GET_IDENT(ch)))) {
	send_to_char("Sorry, you'll have to find your own house.\n\r",ch);
	return(FALSE);
    }

    if(IS_SET(ch->specials.mob_act,ACT_IT))
    {
	send_to_char("You can not rent now!  Tag someone first!!\n\r", ch);
	return TRUE;
    }
    
    OfferChar(ch, &cost, ch->in_room, 0);
    
    if (cost.lims > 22)
    {
	send_to_char("I'm sorry, but you have more than 22 \n\r", ch);
	send_to_char("limited items.  Due to hoarding problems\n\r", ch);
	send_to_char("I have been forced to restrict the houses\n\r", ch);
	return TRUE;
    }

    if (cost.total_cost > 1000000)
    {
	send_to_char("You only get 1.0 million coins worth for free\n\r",ch);
	cost.total_cost -= 1000000;
	sprintf(buf, "It will cost you %d coins per day\n\r", cost.total_cost);
	send_to_char(buf, ch);
    }
    else
	cost.total_cost = 0;

    ch->rent_cost = cost.total_cost;
    
    // Remove all spell events before saving char and renting
    spevent_remove_all_char(ch); 
    if(SaveChar(ch, ch->in_room, FALSE))
	return TRUE;
   
    drop_unrented(ch);
    
    extract_char(ch);

    return TRUE;
}

SPECIAL(receptionist)
{
    struct obj_cost cost;
    struct char_data *recep = (struct char_data *) me;
    sh_int action_table[65] =
    {
	23,24,36,105,106,109,111,142,147,264,265,266,267,268,269,
	270,271,272,273,274,275,276,277,278,279,280,281,282,283,
	284,285,286,287,288,289,290,291,292,293,294,295,296,297,
	298,299,300,301,302,303,304,305,306,307,308,309,310,311,
	312,313,314,315,316,317,318,319
    };
  

    switch (type)
    {
    case SPEC_CMD:
	if(cmd != 92 && cmd != CMD_CHECKIN && cmd != 93)
	    return FALSE;
    
	if (!AWAKE(recep))
	{
	    act("$e isn't able to talk to you...",
		FALSE, recep, 0, ch, TO_VICT);
	    return(TRUE);
	}
	/* 
	   if (!CAN_SEE(recep, ch)) 
	   {
	   act("$n says, 'I don't deal with people I can't see!'",
	   FALSE, recep, 0, 0, TO_ROOM);
	   return(TRUE);
	   }
	   */
	
	if(IS_NPC(ch))
	{
	    act("$n says, 'I don't deal with monsters!'",
		FALSE, recep, 0, 0, TO_ROOM);
	    return TRUE;
	}
   
	/* the auction safety check */
	
	if (auction->item != NULL && ((ch == auction->buyer) ||
				      (ch == auction->seller)))
	{
	    send_to_char("Wait till you have sold/bought the item on auction.\n\r",
			 ch);
	    return TRUE;
	}
   
    
	switch(cmd)
	{
	case 92:			/* rent */
	    if(IS_SET(ch->specials.mob_act, ACT_IT))
	    {
		send_to_char("You can not rent now!  Tag someone first!\n\r",
			     ch);
		return TRUE;
	    }
	    if(!OfferChar(ch, &cost, ch->in_room, TRUE))
	    {
		act("$n stores your stuff in the safe, and helps you into your chamber.",
		    FALSE, recep, 0, ch, TO_VICT);
		act("$n helps $N into $S private chamber.",
		    FALSE, recep,0,ch,TO_NOTVICT);
	       
	        ch->drop_count = 0;
		
		// Remove all spell events before saving char and renting
		spevent_remove_all_char(ch); 
		if(SaveChar(ch, ch->in_room, FALSE))
		    return TRUE;
		
	        TrackingSystem.UpdateCharFull(ch);
	       
		drop_unrented(ch);
		    
		send_to_char("\n\n\rThrough this portal lies the real world.\n\r"
			     "You come when you can, you leave when you must.\n\r"
			     "Such is the endless game of life.\n\r"
			     "                     - Unknown\n\r\n\r", ch);
		
		extract_char(ch);
	    }
	    break;

	case CMD_CHECKIN:
	    if(IS_SET(ch->specials.mob_act, ACT_IT))
	    {
		send_to_char("You can not check in now!  Tag someone first!\n\r",
			     ch);
		return TRUE;
	    }
	    if(!OfferChar(ch, &cost, ch->in_room, TRUE))
	    {
		/* skip whitespaces */
		for (; isspace(*arg); arg++);
		
		if (strncmp(crypt(arg, MASTER_PASSWORD), MASTER_PASSWORD, 10))
		{
		    if(GET_GOLD(ch) < cost.total_cost)
		    {
			act("$n tells you 'You can't afford long-term rent now.'",
			    FALSE, recep, 0, ch, TO_VICT);
			return TRUE;
		    }
		    GET_GOLD(ch) -= cost.total_cost;
		}
		ch->rent_cost = 0;
		
		act("$n tells you 'You will be put into long-term rent now.'",
		    FALSE, recep, 0, ch, TO_VICT);
		act("$n stores your stuff in the safe, and helps you into your chamber.",
		    FALSE, recep, 0, ch, TO_VICT);
		act("$n helps $N into $S private chamber.",
		    FALSE, recep,0,ch,TO_NOTVICT);
		
		if(!IS_SET(ch->delete_flag, HOUSED))
		    SET_BIT(ch->delete_flag, HOUSED);
		
	        ch->drop_count = 0;
	       
		// Remove all spell events before saving char and renting
		spevent_remove_all_char(ch); 
		if(SaveChar(ch, ch->in_room, FALSE))
		    return TRUE;
	       
	        TrackingSystem.UpdateCharFull(ch);
		
		drop_unrented(ch);
		
		extract_char(ch);
	    }
	    break;
	    
	case 93:
	    OfferChar(ch, &cost, ch->in_room, TRUE);
	    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
	    break;
	}
	
	return(TRUE);

    case SPEC_IDLE:
	if(!number(0, 30)) {
	    do_action(recep, "", action_table[number(0,
			  (sizeof(action_table)/sizeof(*action_table)) - 1)]);
	    return TRUE;
	}
	break;

    default:
	return citizen(recep, 0, 0, "", type);
    }

    return FALSE;
}

void drop_unrented(struct char_data* ch)
{
  int i;
  
  drop_unrented_obj(ch, ch->carrying);

  for(i = 0 ; i < MAX_WEAR ; ++i)
  {
    if(ch->equipment[i])
      drop_unrented_obj(ch, ch->equipment[i]);
  }
}

static void drop_unrented_obj(struct char_data* ch, struct obj_data* obj)
{
  struct obj_data* no;

  while(obj)
  {
    no = obj->next_content;
    if(obj->obj_flags.cost_per_day < 0)
    {
      if(obj->in_obj)
	obj_from_obj(obj);
      else if(obj->carried_by)
	obj_from_char(obj);
      else if(obj->equipped_by)
	unequip_char(obj->equipped_by, obj->eq_pos);
      obj_to_room(obj, ch->in_room);
    }
    else
      drop_unrented_obj(ch, obj->contains);
    obj = no;
  }
}    

int ChargeRent(struct char_data* ch)
{
    long		time_off;
    long		total_rent;
    double		hours_off;
    double		days_off;
    char		buf[256];
    int			gold;
    
    time_off = time(0) - ch->player.time.logon;
    hours_off = ((double) time_off) / (60. * 60.);
    if(IS_SET(ch->delete_flag, HOUSED))
    {
      total_rent = 0;
      sprintf(buf, "Char reconnected from check-in");
    }
    else if((hours_off < 1.0) && (ch->in_room == AUTO_RENT))
    {
	total_rent = 0;
	sprintf(buf, "Char reconnecting after auto-rent");
    }

    else
    {
	days_off = hours_off / 24.;
	total_rent = (long) ((double) ch->rent_cost * days_off);
	sprintf(buf, "Rent %ld for %.1f hours", total_rent, hours_off);
    }
   
    slog(buf);
    WriteToImmort(buf, ch->invis_level, LOG_CONNECT);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

#if PLUNDER_BANK
    gold = GET_GOLD(ch) + GET_BANK(ch);
#else
    gold = GET_GOLD(ch);
#endif

    if((GET_GOLD(ch) + GET_BANK(ch)) > 10000000)
    {
	sprintf(buf, "Char has more than 10 mil coins (%d)",
		GET_GOLD(ch) + GET_BANK(ch));

	slog(buf);
	WriteToImmort(buf, ch->invis_level, LOG_PLAYER);
    }
    
    if(total_rent > gold)
    {
	send_to_char("You dead beat!  You stiffed me on my rent!\n\r", ch);
	slog("Char ran out of money in rent");
	WriteToImmort("Char ran out of money in rent", ch->invis_level, LOG_PLAYER);

	GET_GOLD(ch) = 0;
#if PLUNDER_BANK
	GET_BANK(ch) = 0;
#endif
	return TRUE;
    }

#if PLUNDER_BANK
    if(total_rent > GET_GOLD(ch))
    {
	GET_GOLD(ch) += GET_BANK(ch);
	GET_BANK(ch) = 0;
	send_to_char("Your bank account has been emptied to pay your rent\n\r",
		     ch);
    }
#endif

    GET_GOLD(ch) -= total_rent;

    return FALSE;
}

