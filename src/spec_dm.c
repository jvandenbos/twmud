
#include "config.h"

#include <stdio.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "db.h"
#include "comm.h"
#include "find.h"
#include "spec.h"
#include "spells.h"
#include "util_str.h"
#include "util_num.h"
#include "constants.h"
#include "multiclass.h"
#include "vnum_mob.h"
#include "act.h"

#define MAX_DM_OBJ	30
#define MIL		* 1000000
#define K		* 1000

/* special apply values... */
#define APPLY_DOES_SPELL        0	

typedef enum 
{
    dm_void,
    dm_hp,
    dm_mana,
    dm_str,
    dm_int,
    dm_wis,
    dm_dex,
    dm_con,
    dm_imm_hunger,
    dm_imm_thirst,
    DM_MAX_TYPE
} dm_type;

struct dm_obj_info
{
    int	rnum;
    int	mod;
};

typedef struct
{
    int		mob_vnum;
    dm_type	type;
    int		affect;
    int		min_affect;
    EXP		exp;
    int		gold;
    char*	demand;
    int		obj_count;
    struct dm_obj_info obj_info[MAX_DM_OBJ];
} dm_info;

dm_info dm_table[] =
{
    {
	VMOB_HITDM,			/* arnold reddenbacker */
	dm_hp, APPLY_HIT, 20,
	4 MIL, 900 K,		/* 2 mil exp, 1 mil gold */
	"Give me $p!",
	0
    },
    {
	VMOB_MANADM,			/* high mentat */
	dm_mana, APPLY_MANA, 20,
	5 MIL, 900 K,
	"I need $p.",
	0
    },
    {
	VMOB_STRDM,			/* body builder */
	dm_str, APPLY_STR, 2,
	25 MIL, 900 K,
	"$p can make you stronger!",
	0
    },
    {
        VMOB_INTDM,			/* master wizard */
	dm_int, APPLY_INT, 1,
	25 MIL, 900 K,
	"$p can make you smarter!",
	0
    },
    {
	VMOB_WISDM,			/* master cleric */
	dm_wis, APPLY_WIS, 1,
	25 MIL, 900 K,
	"$p can make you wiser!",
	0
    },
    {
	VMOB_DEXDM,			/* master juggler */
	dm_dex, APPLY_DEX, 1,
	25 MIL, 900 K,
	"$p can make you agile!",
	0
    },
    {
	VMOB_CONDM,			/* Mule Gibbous */
	dm_con, APPLY_CON, 1,
	25 MIL, 900 K,
	"$p increases your constitution",
	0
    },
    {
	VMOB_HUNDM,			/* Aged Guru */
	dm_imm_hunger, APPLY_DOES_SPELL, SPELL_H_FEAST,
	9 MIL, 900 K,
	"$p removes your hunger.",
	0
    },
    {
        VMOB_THRDM,			/* Patrick Duffee */
	dm_imm_thirst, APPLY_DOES_SPELL, SPELL_H_FEAST,
	9 MIL, 900 K,
	"$p removes your thirst.",
	0
    },
    {
	dm_void
    }
};

/* note that these multipliers are all / 2, so 5 in this table is * 2.5 */
static int dm_mult[DM_MAX_TYPE][MAX_LEVEL_IND + 1] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	/* dm_void */
    { 2, 2, 5, 3, 4, 2, 2, 4, 4, 2, 2 },	/* dm_hp */
    { 5, 5, 2, 2, 3, 4, 5, 3, 5, 2, 2 },	/* dm_mana */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_str */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_int */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_wis */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_dex */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_con */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },	/* dm_imm_hunger */
    { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }		/* dm_imm_thirst */
};

static void dm_do_change(struct char_data* ch, dm_info* info, int rnum);
static int dm_do_idle(struct char_data* dm);
static int dm_do_give(struct char_data* ch, char* arg);
static int dm_do_list(struct char_data* dm, struct char_data* ch);
static int dm_scale_buy(struct char_data* ch, dm_info* info,
			int rnum, int value);
static void dm_setup_items(dm_info* info);
static int dm_add_item(dm_info* info, int rnum, int mod);
static int dm_pick_item(dm_info* info);

static dm_info* find_dm(struct char_data* ch);


SPECIAL(dungeon_master)
{
    struct char_data *dm = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);
  
    if((GET_POS(dm) == POSITION_FIGHTING) || (GET_POS(dm) < POSITION_RESTING))
	return FALSE;

    switch(cmd)
    {
    case 72:
	return dm_do_give(ch, arg);

    case 59:
	return dm_do_list(dm, ch);
	
    case 0:
	return dm_do_idle(dm);
    }

    return FALSE;
}

static dm_info* find_dm(struct char_data* ch)
{
    int			vnum;
    char		buf[256];
    dm_info*		info;
    
    vnum = mob_index[ch->nr].virt;
    
    for(info = dm_table ; info->type != dm_void ; info++)
	if(info->mob_vnum == vnum)
	{
	    dm_setup_items(info);
	    
	    return info;
	}
    
    sprintf(buf, "unknown dungeon_master (%d) in room: %ld",
	    vnum, ch->in_room);
    log_msg(buf);
    
    return NULL;
}

static int dm_do_list(struct char_data *dm, struct char_data* ch)
{
    dm_info*		info;
    char		buf[256];
    char		buf2[256];
    int			i;

#ifdef JANWORK
   if(TRUST(ch) < TRUST_LORD)
	return FALSE;
#endif
    
    if(!(info = find_dm(dm)))
	return TRUE;

    if(!dm->act_ptr)
	dm->act_ptr = dm_pick_item(info);

    sprinttype(info->affect, apply_types, buf2);
    
   if (TRUST(ch) < TRUST_LORD) 
     send_to_char("I take the following items: \n\r", ch);
   else  {
      
      sprintf(buf, "obj_count:%d  affect:%s  min_affect:%d\n\r",
	      info->obj_count, buf2, info->min_affect);
      send_to_char(buf, ch);
      
      send_to_char("number     modifier  vnum count limit name\n\r", ch);
   }
   
   for(i = 0 ; i < info->obj_count ; ++i)
     {
	struct dm_obj_info* obj = &info->obj_info[i];
	struct index_data* idx = &obj_index[obj->rnum];
	
	if(TRUST(ch) < TRUST_LORD) 
	  sprintf(buf,"%s\n\r",OBJ_NAME((struct obj_data*) idx->proto));
	else
	  sprintf(buf, "%6d %c %10d %5d %5d %5d %s\n\r",
		  i, obj->rnum == dm->act_ptr ? '*' : ' ',
		  obj->mod, idx->virt, idx->number, idx->limit,
		  OBJ_NAME((struct obj_data*) idx->proto));
       send_to_char(buf, ch);
     }
   
   return TRUE;
}

static int dm_do_give(struct char_data* ch, char* arg)
{
    dm_info*		info;
    struct char_data*	dm;
    struct obj_data*	obj;
    char		obj_name[256];
    char		dm_name[256];
    int 	i;
    
    split_last(arg, obj_name, dm_name);
    
    if(!dm_name[0] ||
       !(dm = get_char_room_vis(ch, dm_name)))
    {
	send_to_char("To whom?\n\r", ch);
	return FALSE;
    }

    if(!(IS_NPC(dm)) || (mob_index[dm->nr].func != dungeon_master))
    {
      return FALSE;
    }

    /* From here on down, return true to prevent obj going to dm by accident */
    if(!obj_name[0] ||
       (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))))
    {
	send_to_char("Give what?\n\r", ch);
	return TRUE;
    }

    if(!(info = find_dm(dm)))
    {
      log_msg("found a dm with no info");
      return TRUE;
    }

    if(!dm->act_ptr)
	dm->act_ptr = dm_pick_item(info);

    if(IS_NPC(ch))
    {
	do_say(dm, "Go away mutt!  You bother me.", 0);
	return TRUE;
    }
   
    //For new hp gaining system (Quilan)
    if((info->affect == APPLY_HIT) && (ch->player.hpsystem!=2)) {
        do_say(dm, "I'm sorry, HP dming has been done away with...", 0);
        return TRUE;
    }
    
    if ((IS_OBJ_STAT(obj, ITEM_NODROP)) && (!IS_IMMORTAL(ch)))
    {
	send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
	return TRUE;
    }

    act("You give $p to $N.",TRUE,ch,obj,dm,TO_CHAR);
    act("$n gives $p to $N.",TRUE,ch,obj,dm,TO_ROOM);	
    act("$n gives you $p.",TRUE,ch,obj,dm,TO_VICT);	

    obj_from_char(obj);

#ifdef SWAP_ZONES
    for(i = info->obj_count-1; i >= 0; i--)
      if (obj->item_number==info->obj_info[i].rnum)
      {
	dm->act_ptr=info->obj_info[i].rnum;
	break;
      }
#endif
    if(obj->item_number != dm->act_ptr)
    {
	do_say(dm, "Thank you, but that's useless to me.", 0);
    toss:
#if 0
	obj_to_room(obj, ch->in_room);
	act("$N drops $p on the floor.",
	    TRUE, ch, obj, dm, TO_CHAR + TO_ROOM);
	act("You drop $p on the floor.", TRUE, dm, obj, NULL, TO_CHAR);
#else
	do_say(dm, "I think I'll be nice today.", 0);
	obj_to_char(obj,ch);
	act("$n gives $p to $N.", TRUE, dm, obj, ch, TO_ROOM);
	act("$n gives you $p.", TRUE, dm, obj, ch, TO_VICT);
	act("You give $p to $N.", TRUE, dm, obj, ch, TO_CHAR);
#endif
	return TRUE;
    }
/*
    if (IS_MULTI(ch) && (info->type == dm_hp))
      {
	do_say(dm,"You are too diverse for me to help you. -- Type HELP MULTI",0);
	goto toss;
      }
*/    
    if(GET_EXP(ch) < info->exp)
    {
	do_say(dm,
	       "You need to be more experienced before I can help you.",
	       0);
	goto toss;
    }
	
    if(GET_GOLD(ch) < info->gold)
    {
	do_say(dm,
	       "You can't afford my help.",
	       0);
	goto toss;
    }

    dm_do_change(ch, info, obj->item_number);

    GET_EXP(ch) -= info->exp;
    GET_GOLD(ch) -= info->gold;

    act("$N waves his hands and $p turns to dust.",
	TRUE, ch, obj, dm, TO_CHAR + TO_ROOM);
    act("You destroy $p in helping $N",
	TRUE, dm, obj, ch, TO_CHAR);

    extract_obj(obj);

    dm->act_ptr = 0;

    return TRUE;
}

static int dm_do_idle(struct char_data* dm)
{
    dm_info*	info;
    char	buf[256];
    
    if(!(info = find_dm(dm)))
	return FALSE;
    
    if(!dm->act_ptr)
	dm->act_ptr = dm_pick_item(info);

    if(check_soundproof(dm) || (number(0, 100) > 25))
	return FALSE;

    if(dm->act_ptr <= 0)
    {
	do_say(dm, "I can't help you right now.", 0);
    }
    else
    {
	sprintf(buf, "$n says, '%s'", info->demand);
	act(buf, FALSE, dm, (struct obj_data *) obj_index[dm->act_ptr].proto,
	    NULL, TO_ROOM);
	sprintf(buf, "You say, '%s'", info->demand);
	act(buf, FALSE, dm, (struct obj_data *) obj_index[dm->act_ptr].proto,
	    NULL, TO_CHAR);
#ifdef SWAP_ZONES    
	dm->act_ptr=dm_pick_item(info);
#endif    
    }
    
    return TRUE;
}

#define INC18(stat, amt)	(stat = MIN(18, stat + amt))

static void dm_do_change(struct char_data* ch, dm_info* info, int rnum)
{
    int k;
    char buf[256];
    
    switch(info->type)
    {
    case dm_hp:
	k = ch->points.max_hit;
	if (k<600) {
	k = 1 + 3*(k<200) + 2*(k<300) + (k<400) + (k<500)
	    + (k<600) + (k<800) + (k<1000) + (k<1200) + (k<1500);
	ch->points.max_hit += dm_scale_buy(ch, info, rnum, k);
	} else {
	sprintf(buf, "%s just tried to dm hp with over 600 hp.\n\r", GET_NAME(ch));
	log_msg(buf);
	}
	break;
    case dm_mana:
	k = ch->points.max_mana;
	k = 1 + (k<1) + (k<51) + (k<101) + (k<151);
	ch->points.max_mana += dm_scale_buy(ch, info, rnum, k);
	break;
    case dm_str:
	if((k = ch->abilities.str) < 18)
	{
	    k = dm_scale_buy(ch, info, rnum, 1);
	    INC18(ch->abilities.str, k);
	}
	else
	{
	    k = dm_scale_buy(ch, info, rnum, 10);
	    ch->abilities.str_add = MIN(100, ch->abilities.str_add + k);
	}
	break;
    case dm_dex:
	k = dm_scale_buy(ch, info, rnum, 1);
	INC18(ch->abilities.dex, k);
	break;
    case dm_int:
	k = dm_scale_buy(ch, info, rnum, 1);
	INC18(ch->abilities.intel, k);
	break;
    case dm_wis:
	k = dm_scale_buy(ch, info, rnum, 1);
	INC18(ch->abilities.wis, k);
	break;
    case dm_con:
	k = dm_scale_buy(ch, info, rnum, 1);
	INC18(ch->abilities.con, k);
	break;
    case dm_imm_hunger:
	ch->specials.conditions[1] = -1;
	break;
    case dm_imm_thirst:
	ch->specials.conditions[2] = -1;
	break;
    default:
	sprintf(buf, "Unknown dm_type in room %ld: %d\n",
		ch->in_room, info->type);
	log_msg(buf);
    }
}

static int dm_scale_buy(struct char_data* ch, dm_info* info,
			int rnum, int buy)
{
    int	mult, idx, bit;

    for(mult = 0xffff, idx = 0, bit = 1 ;
	idx <= MAX_LEVEL_IND ;
	++idx, bit <<= 1)
    {
	if(HasClass(ch, bit) && (dm_mult[info->type][idx] < mult))
	    mult = dm_mult[info->type][idx];
    }

    if(mult == 0xffff)
    {
	log_msg("Error in dm_scale_buy, char has no class?");
	return buy;
    }

    for(idx = info->obj_count - 1 ; idx >= 0 ; idx--)
	if(info->obj_info[idx].rnum == rnum)
	{
	    mult *= info->obj_info[idx].mod;
	    break;
	}

    if(idx < 0)
    {
	log_msg("Error in dm_scale_buy, object not in table");
	return buy;
    }
    
    buy = buy * mult / (2 * info->min_affect);

    return MAX(buy, 1);
}

static void dm_setup_items(dm_info* info)
{
  int			rnum, afc;
  struct index_data*	obj;
  char 		buf[256];
  struct obj_affected_type* aff;
  struct obj_data*	o;
    
  if(info->obj_count)
    return;
    
  for(rnum = 0, obj = obj_index ; rnum <= top_of_objt ; rnum++, obj++)
  {
#ifndef SWAP_ZONES
    if(!obj->proto)
      continue;
	
    if(obj->limit == ONCE_PER_REBOOT)
    {
//      if(obj->number < 5)
//	continue;
    }
    else if(obj->limit <= 10)
      continue;

    o = (struct obj_data*) obj->proto;
#else

//    if( (obj->limit < 0) || (obj->number < 5) )
//      continue;
    if( (obj->limit <= 10) )
        continue;
     
    o = make_object(rnum, NORAND);
#endif	

    if(info->affect == APPLY_DOES_SPELL)
    {
      switch(o->obj_flags.type_flag)
      {
      case ITEM_WAND:
      case ITEM_STAFF:
	if((o->obj_flags.value[3] == info->min_affect) &&
	   dm_add_item(info, rnum, 0))
	{
	  extract_obj(o);
	  return;
	}
	break;
      }
    }
    else
    {
      for(aff = o->affected, afc = 0 ; afc < MAX_OBJ_AFFECT ;
	  afc++, aff++)
      {
	if((aff->location == info->affect) &&
	   (aff->modifier >= info->min_affect) &&
	   dm_add_item(info, rnum, aff->modifier))
	{
	  extract_obj(o);
	  return;
	}
      }
    }
    extract_obj(o);
  }

  if(info->obj_count == 0)
  {
    info->obj_count = -1;
    sprintf(buf, "Mob %d has 0 dm_objs", info->mob_vnum);
    log_msg(buf);
  }
}

static int dm_add_item(dm_info* info, int rnum, int mod)
{
    char buf[256];
    
    if(info->obj_count == MAX_DM_OBJ)
    {
	sprintf(buf, "Too many dm_objs for mob %d", info->mob_vnum);
	log_msg(buf);
	return 1;
    }
		
    info->obj_info[info->obj_count].mod = mod;
    info->obj_info[info->obj_count++].rnum = rnum;

    return 0;
}

#define BITS_PER_LONG	(sizeof(long) * 8)

static int dm_pick_item(dm_info* info)
{
  long*		map;
  int		start;
    
  if(info->obj_count < 0)
    return -1;
   
 
  CREATE(map, long, (top_of_objt + BITS_PER_LONG) / BITS_PER_LONG);
  if(!map)
    return info->obj_info[number(0, info->obj_count - 1)].rnum;

  start = number(0, info->obj_count - 1);

#if 0
  EACH_OBJECT(o_iter, obj)
  {
    if(obj->item_number >= 0)
      map[obj->item_number / BITS_PER_LONG] |=
	1 << obj->item_number % BITS_PER_LONG;
  }
  END_AITER(o_iter);

  for(idx = start ; ; )
  {
    rnum = info->obj_info[idx].rnum;
	
    if(map[rnum / BITS_PER_LONG] & (1 << rnum % BITS_PER_LONG))
    {
      FREE(map);
      return rnum;
    }

    idx = (idx == info->obj_count - 1) ? 0 : (idx + 1);
    
    if(idx == start)
    {
      FREE(map);
      return 0;
    }
  }
#else
  return info->obj_info[start].rnum;
#endif
}        
