
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "spells.h"
#include "utility.h"
#include "handler.h"
#include "modify.h"
#include "multiclass.h"
#include "constants.h"
#include "spell_util.h"
#include "act.h"
#include "fight.h"
#include "opinion.h"
#include "race.h"
#include "track.h"
#include "interpreter.h"
#include "newsaves.h"

int affect_event(struct affected_type* aff, int now);

void affect_modify(struct char_data *ch, byte loc, long mod,
		   long bitv, bool add)
{
    int i;
    
    switch(loc)
    {
    case APPLY_IMMUNE:
	if (add)
	    SET_BIT(ch->immune, mod);
	else
	    REMOVE_BIT(ch->immune, mod);
	break;
	
    case APPLY_SUSC:
	if (add)
	    SET_BIT(ch->susc, mod);
	else
	    REMOVE_BIT(ch->susc, mod);
	break;
	
    case APPLY_M_IMMUNE:
	if (add)
	    SET_BIT(ch->M_immune, mod);
	else
	    REMOVE_BIT(ch->M_immune, mod);
	break;
	
    case APPLY_SPELL:
	if (add)
	  SET_BIT(AFF_FLAGS(ch), mod);
	else
	  REMOVE_BIT(AFF_FLAGS(ch), mod);
	break;

    case APPLY_SPELL2:
	if (add)
	    SET_BIT(AFF2_FLAGS(ch), mod);
	else
	    REMOVE_BIT(AFF2_FLAGS(ch), mod);
	break;

    case APPLY_SEX:
	GET_SEX(ch) = (!(ch->player.sex-1))+1;
	break;
    case APPLY_AFF2:
         if (add) {
	     SET_BIT(AFF2_FLAGS(ch), bitv);
	  } else {
	     REMOVE_BIT(AFF2_FLAGS(ch),bitv);
	      mod = -mod;
          }
	  break;
    default:
	if (add) {
	    SET_BIT(AFF_FLAGS(ch), bitv);
	} else {
	    REMOVE_BIT(AFF_FLAGS(ch), bitv);
	    mod = -mod;
	}
	
	switch(loc)
	{
	case APPLY_NONE:
	case APPLY_WEAPON_SPELL:
	case APPLY_EAT_SPELL:
	case APPLY_CLASS:
	case APPLY_LEVEL:
	case APPLY_GOLD:
	case APPLY_EXP:
	case APPLY_BOOK_SPELL:
	case APPLY_DRAIN_LIFE:
	    break;
	    
	case APPLY_STR:
	  if(mod > 0)
	    for (i = 0; i < mod; i++)
	      if ((ch->abilities.str != 18) || (ch->abilities.str_add >= 100))
		ch->abilities.str++;
	      else
		ch->abilities.str_add += 10;
	  else
	    for (i = mod; i < 0; i++)
	      if ((ch->abilities.str != 18) || (ch->abilities.str_add <= 0))
		ch->abilities.str--;
	      else
		ch->abilities.str_add -= 10;
	  break;

	case APPLY_DEX:
	  ch->abilities.dex += mod;
	  break;

	case APPLY_CHA:
	  ch->abilities.cha += mod;
	  break;
	    
	case APPLY_INT:
	    ch->abilities.intel += mod;
	    break;
	    
	case APPLY_WIS:
	    ch->abilities.wis += mod;
	    break;
	    
	case APPLY_CON:
	    ch->abilities.con += mod;
	    break;
	    
	case APPLY_AGE:
	    ch->player.time.birth -= SECS_PER_MUD_YEAR*mod;
	    break;
	    
	case APPLY_CHAR_WEIGHT:
	    GET_WEIGHT(ch) += mod;
	    break;
	    
	case APPLY_CHAR_HEIGHT:
	    GET_HEIGHT(ch) += mod;
	    break;
	    
	case APPLY_MANA:
	    ch->points.max_mana += mod;
	    break;
	    
	case APPLY_HIT:
	    ch->points.max_hit += mod;
	    break;
	    
	case APPLY_MOVE:
	    ch->points.max_move += mod;
	    break;
	    
	case APPLY_AC:
	    GET_AC(ch) += mod;
	    break;
	    
	case APPLY_HITROLL:
	    GET_HITROLL(ch) += mod;
	    break;
	    
	case APPLY_DAMROLL:
	    GET_DAMROLL(ch) += mod;
	    break;
	    
	case APPLY_SAVING_PARA:
	    ch->specials.apply_saving_throw[0] += mod;
	    break;
	    
	case APPLY_SAVING_ROD:
	    ch->specials.apply_saving_throw[1] += mod;
	    break;
	    
	case APPLY_SAVING_PETRI:
	    ch->specials.apply_saving_throw[2] += mod;
	    break;
	    
	case APPLY_SAVING_BREATH:
	    ch->specials.apply_saving_throw[3] += mod;
	    break;
	    
	case APPLY_SAVING_SPELL:
	    ch->specials.apply_saving_throw[4] += mod;
	    break;
	    
	case APPLY_SAVE_ALL: 
	    {
		for (i=0;i<=4;i++)
		    ch->specials.apply_saving_throw[i] += mod;
	    }
	    break;
	    
	case APPLY_HITNDAM:
	    GET_HITROLL(ch) += mod;
	    GET_DAMROLL(ch) += mod;
	    break; 
	    
/* Warrior */
	    
        case APPLY_DOORBASH:
            if (!ch->skills) return;
            ch->skills[SKILL_DOORBASH].learned += mod;
            break;

	case APPLY_KICK:
	    if (!ch->skills) return;
	    ch->skills[SKILL_KICK].learned += mod;
	    break;
	    
	case APPLY_BASH:
	    if (!ch->skills) return;
	    ch->skills[SKILL_BASH].learned += mod;
	    break;

        case APPLY_SBLOCK:
            if (!ch->skills) return;
            ch->skills[SKILL_SHIELD_BLOCK].learned += mod;
            break;

        case APPLY_SPUNCH:
            if (!ch->skills) return;
            ch->skills[SKILL_SHIELD_PUNCH].learned += mod;
            break;

        case APPLY_FLAIL:
            if (!ch->skills) return;
            ch->skills[SKILL_FLAIL].learned += mod;
            break;

        case APPLY_PUSH:
            if (!ch->skills) return;
            ch->skills[SKILL_PUSH].learned += mod;
            break;

/* thief */

        case APPLY_BACKSTAB:
            if (!ch->skills) return;
            ch->skills[SKILL_BACKSTAB].learned += mod;
            break;

        case APPLY_DISARM:
            if (!ch->skills) return;
            ch->skills[SKILL_DISARM].learned += mod;
            break;

        case APPLY_DIVERT:
            if (!ch->skills) return;
            ch->skills[SKILL_DIVERT].learned += mod;
            break;

        case APPLY_GOUGE:
            if (!ch->skills) return;
            ch->skills[SKILL_GOUGE].learned += mod;
            break;

        case APPLY_THROW:
            if (!ch->skills) return;
            ch->skills[SKILL_THROW].learned += mod;
            break;

        case APPLY_SEARCH:
            if (!ch->skills) return;
            ch->skills[SKILL_SEARCH].learned += mod;
            break;

        case APPLY_SNEAK:
            if (!ch->skills) return;
            ch->skills[SKILL_SNEAK].learned += mod;
            break;

        case APPLY_HIDE:
            if (!ch->skills) return;
            ch->skills[SKILL_HIDE].learned += mod;
            break;

	case APPLY_PICK:
	    if (!ch->skills) return;
	    ch->skills[SKILL_PICK_LOCK].learned += mod;
	    break;

	case APPLY_STEAL:
	    if (!ch->skills) return;
	    ch->skills[SKILL_STEAL].learned += mod;
	    break;

        case APPLY_TRIP:
            if(!ch->skills) return;
            ch->skills[SKILL_TRIP].learned += mod;
            break;

/* Ranger */

        case APPLY_ARCHERY:
            if (!ch->skills) return;
            ch->skills[SKILL_ARCHERY].learned += mod;
            break;

        case APPLY_STUN:
            if (!ch->skills) return;
            ch->skills[SKILL_STUN].learned += mod;
            break;

        case APPLY_THRUST:
            if (!ch->skills) return;
            ch->skills[SKILL_THRUST].learned += mod;
            break;

	case APPLY_TRACK:
	    if (!ch->skills) return;
	    ch->skills[SKILL_HUNT].learned += mod;
	    break;

	case APPLY_RIDE:
	    if(!ch->skills) return;
	    ch->skills[SKILL_RIDE].learned += mod;
	    break;

/* Psi */

        case APPLY_MEDITATE:
            if (!ch->skills) return;
            ch->skills[SKILL_MEDITATE].learned += mod;
            break;

	case APPLY_HYPNO:
            if (!ch->skills) return;
            ch->skills[SKILL_HYPNOSIS].learned += mod;
            break;

	case APPLY_THOUGHT:
            if (!ch->skills) return;
            ch->skills[SKILL_THOUGHT_THROW].learned += mod;
            break;

	case APPLY_COMBUSTION:
            if (!ch->skills) return;
            ch->skills[SKILL_COMBUSTION].learned += mod;
            break;

	case APPLY_CONSTRICT:
            if (!ch->skills) return;
            ch->skills[SKILL_CONSTRICT].learned += mod;
            break;

	case APPLY_BLAST:
            if (!ch->skills) return;
            ch->skills[SKILL_PSIONIC_BLAST].learned += mod;
            break;

/* Paladin */	

        case APPLY_BLESSING:
            if (!ch->skills) return;
            ch->skills[SKILL_BLESSING].learned += mod;
            break;

        case APPLY_HEROIC_RESCUE:
            if (!ch->skills) return;
            ch->skills[SKILL_HEROIC_RESCUE].learned += mod;
            break;

	case APPLY_WARCRY:
	    if(!ch->skills) return;
	    ch->skills[SKILL_HOLY_WARCRY].learned += mod;
	    break;

        case APPLY_LAYHANDS:
            if(!ch->skills) return;
            ch->skills[SKILL_LAY_ON_HANDS].learned += mod;
            break;

/* Monk */

        case APPLY_PINCH:
            if (!ch->skills) return;
            ch->skills[SKILL_PINCH].learned += mod;
            break;

        case APPLY_NUM_DICE:
            ch->specials.damnodice += mod;
            if (IS_PC(ch) && IsHumanoid(ch) && !IS_IMMORTAL(ch) && ch->specials.damnodice > BHD_LIMIT)
            {
                //if they are not a shifter and are not a monk - BHD_LIMIT
	            if (!HasClass(ch, CLASS_SHIFTER) && !HasClass(ch, CLASS_MONK))
                {
                  ch->specials.damnodice = BHD_LIMIT;
                } else { //either monk or shifter...giving preference to shifter (which is higher)
                    if(HasClass(ch, CLASS_SHIFTER)) 
                    {
                        ch->specials.damnodice = BHD_LIMIT2;
                    } else {
                        ch->specials.damnodice = BHD_LIMIT3;
                    }
                }
            }
	        return;
            break;

        case APPLY_SIZE_DICE:
            ch->specials.damsizedice += mod;
            //modify for max barehand limits - shifters get BHD_LIMIT2, Monks BHD_LIMIT3, all else
            //get BHD_LIMIT
            if (IS_PC(ch) && IsHumanoid(ch) && !IS_IMMORTAL(ch) && ch->specials.damsizedice > BHD_LIMIT)
            {
                //if they are not a shifter and are not a monk - BHD_LIMIT
	            if (!HasClass(ch, CLASS_SHIFTER) && !HasClass(ch, CLASS_MONK))
                {
                    ch->specials.damsizedice = BHD_LIMIT;
                }
                else //either monk or shifter...giving preference to shifter (which is higher)
                {
                    if (HasClass(ch, CLASS_SHIFTER))
                    {
                        ch->specials.damsizedice = BHD_LIMIT2; //limit shifters to 25 (defined in structs.h
                    } else {
                        ch->specials.damsizedice = BHD_LIMIT3; //limit monks to 23 (defined in structs.h)
                    }
                }
            }
	        return;
            break;

        case APPLY_BHD:
            ch->specials.damnodice += mod;
            //modify for max barehand limits - shifters get BHD_LIMIT2, Monks BHD_LIMIT3, all else
            //get BHD_LIMIT
            if (IS_PC(ch) && IsHumanoid(ch) && !IS_IMMORTAL(ch) && ch->specials.damnodice > BHD_LIMIT)
            {
                //if they are not a shifter and are not a monk - BHD_LIMIT
	            if (!HasClass(ch, CLASS_SHIFTER) && !HasClass(ch, CLASS_MONK))
                {
                  ch->specials.damnodice = BHD_LIMIT;
                } else { //either monk or shifter...giving preference to shifter (which is higher)
                    if(HasClass(ch, CLASS_SHIFTER)) 
                    {
                        ch->specials.damnodice = BHD_LIMIT2;
                    } else {
                        ch->specials.damnodice = BHD_LIMIT3;
                    }
                }
            }
            ch->specials.damsizedice += mod;
            if (IS_PC(ch) && IsHumanoid(ch) && !IS_IMMORTAL(ch) && ch->specials.damsizedice > BHD_LIMIT)
            {
                //if they are not a shifter and are not a monk - BHD_LIMIT
	            if (!HasClass(ch, CLASS_SHIFTER) && !HasClass(ch, CLASS_MONK))
                {
                    ch->specials.damsizedice = BHD_LIMIT;
                }
                else //either monk or shifter...giving preference to shifter (which is higher)
                {
                    if (HasClass(ch, CLASS_SHIFTER))
                    {
                        ch->specials.damsizedice = BHD_LIMIT2; //limit shifters to 25 (defined in structs.h
                    } else {
                        ch->specials.damsizedice = BHD_LIMIT3; //limit monks to 23 (defined in structs.h)
                    }
                }
            }
	        return;
            break;

	case APPLY_GUILD:
	    if (add)
	    {
		ch->saved_guild = ch->saved_guild ? ch->saved_guild : ch->in_guild;
		ch->in_guild = mod;
	    }
	    else if (mod - ch->in_guild) /* if it is guild currently in */
		ch->in_guild = ch->saved_guild;

	    break;
	default:
	    {
		char buf[256];
		sprintf(buf, "bad apply: %s: %d", GET_NAME(ch), loc);
		log_msg(buf);
	    }
	    break;
	} /* switch */
    }
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
  char buf[80];
  struct affected_type *affected_alloc;
  
  affect_modify(ch, af->location, af->modifier,	af->bitvector, TRUE);
  CREATE(affected_alloc, struct affected_type, 1);
  
  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  sprintf(buf,"affect %d %d %ld %ld",
	  af->type, af->location, af->modifier, af->bitvector);

  event_queue_pulse(&affected_alloc->timer,
		    pulse,
		    (event_func) affect_event,
		    buf);

  affected_alloc->holder = ch;
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
  struct affected_type *hjp;
  
  assert(ch->affected);
  
  affect_modify(ch, af->location, af->modifier,
		af->bitvector, FALSE);
  
  
  /* remove structure *af from linked list */
  
  if (ch->affected == af) {
    /* remove head of list */
    ch->affected = af->next;
  } else {
    
    for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);
    
    if (hjp->next != af) {
      log_msg("Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
      return;
    }
    hjp->next = af->next; /* skip the af element */
  }

  event_cancel(&af->timer, 0);
  
  FREE ( af );
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, short skill)
{
    struct affected_type *hjp, *naf;
  
    for(hjp = ch->affected; hjp; hjp = naf)
    {
	naf = hjp->next;
	if (hjp->type == skill)
	    affect_remove( ch, hjp );
    }  
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, short skill )
{
  struct affected_type *hjp;
  
  for (hjp = ch->affected; hjp; hjp = hjp->next)
    if ( hjp->type == skill )
      return( TRUE );
  
  return( FALSE );
}



void affect_join( struct char_data *ch, struct affected_type *af,
		 bool avg_dur, bool avg_mod )
{
  struct affected_type *hjp;
  
  for (hjp = ch->affected; hjp; hjp = hjp->next) {
    if ( hjp->type == af->type && hjp->location==af->location) {
      
      af->duration += hjp->duration;
      if (avg_dur)
	af->duration /= 2;
      
      af->modifier += hjp->modifier;
      if (avg_mod)
	af->modifier /= 2;
      
      affect_remove(ch, hjp);
      DLOG(("Calling affect_to_char from affect_join handler.c line 399\r\n"));
      affect_to_char(ch, af);
      return;
    }
  }
  DLOG(("Calling affect_to_char from affect_join handler.c line 405\r\n"));
  affect_to_char(ch, af);
}

void affect_remove_all(struct char_data* ch)
{
    struct affected_type* aff;
    
    while((aff = ch->affected))
    {
	affect_modify(ch, aff->location, aff->modifier,
		      aff->bitvector, FALSE);
	ch->affected = aff->next;

	event_cancel(&aff->timer, 0);
	
	FREE(aff);
    }
}
    

/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
  assert(ch->equipment[eq_pos]);
  
  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
    return 0;
  
  switch (eq_pos) {
    
  case WEAR_BODY:
    return (3*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 30% */
  case WEAR_HEAD:
    return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
  case WEAR_LEGS:
    return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
  case WEAR_FEET:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_HANDS:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_ARMS:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  case WEAR_SHIELD:
    return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  }
  return 0;
}



void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  assert(pos>=0 && pos<MAX_WEAR);
  assert(!(ch->equipment[pos]));
  
  if (obj->carried_by) {
    log_msg("EQUIP: Obj is carried_by when equip.");
    abort();
  }
  
  if (obj->in_room!=NOWHERE) {
    log_msg("EQUIP: Obj is in_room when equip.");
    abort();
    return;
  }
  
  if (((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) &&
      !IS_GOD(ch)) {
    if (ch->in_room != NOWHERE) {
      
      act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
      obj_to_room(obj, ch->in_room);
      return;
    } else {
      log_msg("MAJOR ERROR: ch->in_room = NOWHERE when equipping char.");
      extract_obj(obj);
      return;
      /* abort(); */
    }
  }

  raw_equip(ch, obj, pos);
}



void update_object( struct obj_data *obj, int use){
  
  if (obj->obj_flags.timer > 0)	obj->obj_flags.timer -= use;
  if (obj->contains) update_object(obj->contains, use);
  if (obj->next_content) 
    if (obj->next_content != obj)
      update_object(obj->next_content, use);
}

void update_char_objects( struct char_data *ch )
{
  
  int i;
  
  if (ch->equipment[WEAR_LIGHT])
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
	(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;
  
  for(i = 0;i < MAX_WEAR;i++) 
    if (ch->equipment[i])
      update_object(ch->equipment[i],2);
  
  if (ch->carrying) update_object(ch->carrying,1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
    struct affected_type *af;
    struct obj_data *i;
    struct char_data *k;
    struct descriptor_data *t_desc;
    int was_in, j;
  
    if (ch->followers || ch->master)
	die_follower(ch);

    /* eliminate the character and all of it's polies... */
    while((k = pop_character(ch)))
    {
	if(IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
	    mob_index[ch->nr].number--;
	    extract_char(ch);
	}
	ch = k;
    }
    
    if(ch->desc)
    {
	/* Forget snooping */
	if ((ch->desc->snoop.snooping) && (ch->desc->snoop.snooping->desc))
	    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
    
	if (ch->desc->snoop.snoop_by) {
	    send_to_char("Your victim is no longer among us.\n\r",
			 ch->desc->snoop.snoop_by);
	    if (ch->desc->snoop.snoop_by->desc)
		ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
	}
    
	ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
    }
  
    /* Must remove from room before removing the equipment! */
    was_in = ch->in_room;
    if(real_roomp(was_in))
	char_from_room(ch);

    if(IS_PC(ch))
	count_objects = FALSE;
    
    /* purge carried objects */
    while (ch->carrying)
    {
	i = ch->carrying;
	extract_obj(i);
    }

    /*
      equipment too
      */
  
    for (j=0; j<MAX_WEAR; j++)
	if ((i = ch->equipment[j]))
	    extract_obj(unequip_char(ch, j));

    /* and affects */
    while((af = ch->affected))
	affect_remove(ch, af);

    /* and events */
    if(ch->sound_timer)
	event_cancel(ch->sound_timer, 1);
    ch->sound_timer = 0;
    
    
    if(IS_PC(ch))
	count_objects = TRUE;
    
    stop_fighting(ch);
  
    if (MOUNTED(ch))
	Dismount(ch, MOUNTED(ch), POSITION_STANDING);    

    if (RIDDEN(ch))
	Dismount(RIDDEN(ch), ch, POSITION_STANDING);

    if(ch->other_gate)
	ch->other_gate->other_gate = 0;
    
    EACH_CHARACTER(iter, k)
    {
	struct affected_type *af, *naf;

	if(k->specials.fighting == ch)
	    stop_fighting(k);

	if(k->specials.binding == ch)
	    k->specials.binding = 0;

	if(k->specials.binded_by == ch)
	    k->specials.binded_by = 0;
	
	if(k->hunt_info && k->hunt_info->victim == ch)
	{
	    path_kill(k->hunt_info);
	    k->hunt_info = 0;
	}

	if (IS_NPC(ch))
	{
	    if (Hates(k, ch))
		RemHated(k, ch);
	    if (Fears(k, ch))
		RemFeared(k, ch);
	}
	else
	{
	    if (Hates(k, ch))
		ZeroHatred(k, ch);
	    if (Fears(k, ch))
		ZeroFeared(k, ch);
	}

	if(RIDDEN(k) == ch)
	  Dismount(ch, k, POSITION_STANDING);

	if(MOUNTED(k) == ch)
	  Dismount(k, ch, POSITION_STANDING);
	
	for(af = k->affected ; af ; af = af->next)
	{
	    naf = af->next;
	    if(af->caster == ch)
		af->caster = 0;
	}
    }
    END_AITER(iter);
	   
    /* pull the char from the list */
    array_delete(&character_list, ch);
    
    GET_AC(ch) = 200;
  
    t_desc = ch->desc;

    affect_remove_all(ch);
    
    if (!IS_PC(ch))
    {
	if (ch->nr > -1)	/* if mobile */
	    mob_index[ch->nr].number--;
	mob_count--;
	free_char(ch);
    }
    else if(!t_desc)
    {
	free_char(ch);
    }
  
    if (t_desc)
    {
      connected--;
      t_desc->connected = CON_SLCT;
      SEND_TO_Q(MENU, t_desc);
    }
}


void AddAffects( struct char_data *ch, struct obj_data *o)
{
    int i;

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
	if (o->affected[i].location != APPLY_NONE) {
	    affect_modify(ch, o->affected[i].location,
			  o->affected[i].modifier, 0, TRUE);
	} else {
	    return;
	}
    }
}

int affect_event(struct affected_type* aff, int now)
{
  if(aff->mana_cost)
  {
    if(aff->caster &&
       (GET_MANA(aff->caster) > aff->mana_cost) &&
       same_zone(aff->holder, aff->caster))
      GET_MANA(aff->caster) -= aff->mana_cost;
    else
    {
      aff->duration = -1;
      if (aff->expire_proc_pointer) 
	(aff->expire_proc_pointer)(aff->holder, "", -1);
    }
  }
  else {
    if (!aff->expire_proc_pointer)
      aff->duration--;
  }

  if(aff->duration <= 0 && IS_SET(aff->bitvector, AFF_CHARM))
  {
    if(!aff->caster || !aff->holder ||
       NewSkillSave(aff->caster, aff->holder, aff->type,
		    aff->save_bonus, IMM_CHARM))
      aff->duration = -1;
    else
    {
      aff->save_bonus++;
      aff->duration = 0;
    }
  }
    
  if(aff->duration < 0)		/* worn off... */
  {
    if((aff->type > 0) && (aff->type <= MAX_SKILLS))
    {
      if (!aff->next || (aff->next->type != aff->type))
      {
	if (*spell_wear_off_msg[aff->type])
	{
	  send_to_char(spell_wear_off_msg[aff->type], aff->holder);
	  send_to_char("\n\r", aff->holder);
	}
	if(IS_SET(aff->bitvector, AFF_CHARM))
	{
	  stop_follower(aff->holder);
	  return 1;
	}
      }
    }
    /*	else if (aff->type>=FIRST_BREATH_WEAPON &&
	aff->type <=LAST_BREATH_WEAPON)
	{
	breath_func proc;
	log_msg("breath weapon affect");
	proc = bweapons[aff->type - FIRST_BREATH_WEAPON];
	
	(*proc)(-aff->modifier/2, aff->holder, "", SPELL_TYPE_SPELL,
	aff->holder, 0);
	if (!aff->holder->affected)
	return 1;
	} */

    affect_remove(aff->holder, aff);
  }
  else
  {
    /* not expired, requeue */
    event_queue_pulse(&aff->timer,
		      now,
		      (event_func) affect_event,
		      NULL);
  }

  return 1;
}    
