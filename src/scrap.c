/*************************************************************************
 * File: scrap.c                                                         *
 *                                                                       *
 * Usage: Functions involving scrapping                                  *
 *************************************************************************/

#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "spells.h"
#include "db.h"
#include "comm.h"
#include "constants.h"

extern int PKILLABLE;

/* function protos used in this file */
int GetItemDamageType(int type);
int DamagedByAttack(struct obj_data *i, int dam_type);
int DamageItem(struct char_data *ch, struct obj_data *o, int num);



int scraps_timer(object_event* event, long now)
{
    struct obj_data* j = event->object;

    if(--j->obj_flags.timer <= 0)
    {
        if(j->carried_by)
	    act("$p falls apart in your hands.", 
		FALSE, j->carried_by, j, 0, TO_CHAR);
        if(j->equipped_by)
	    act("$p falls apart in your hands.", 
		FALSE, j->equipped_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && 
		 (real_roomp(j->in_room)->people))
	{
	    act("$p falls apart.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	    act("$p falls apart.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}
	j->corpse_timer = 0;
	event_free((event_t*) event);
	ObjFromCorpse(j);
	return 1;
    }
    else
    {
	event_queue_pulse((event_t*) event,
			  pulse,
			  (event_func) scraps_timer,
			  NULL);

	return 1;
    }
}

int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj)
{
  int num;
  char buf[256];

  if (IS_SET(RM_FLAGS(ch->in_room), ARENA) || (PKILLABLE==2))
      return FALSE;

  if (ITEM_TYPE(obj) == ITEM_SPELLBOOK)
     return FALSE;

  num = DamagedByAttack(obj, dam_type);
  if (num != 0) {
    sprintf(buf, "%s is %s.\n\r",OBJ_SHORT(obj), 
	    ItemDamType[dam_type-1]);
    send_to_char(buf,ch);
    if (num == -1) {  /* destroy object*/

#if 0
      if (!ItemSave(obj, dam_type))
#endif
	  return(TRUE);


    } else {   /* "damage item"  (armor), (weapon) */
      if (DamageItem(ch, obj, num)) {
	return(TRUE);
      }
    }
  }
  return(FALSE);
  
}


void MakeScrap( struct char_data *ch, struct obj_data *obj, int dam_type)
{
  char buf[200];
  object_event *event;
  struct obj_data *t, *obj2, *next;

/*
  if (GET_ITEM_TYPE(obj)==ITEM_FIREWEAPON)
  {
     if(IS_SET(obj->obj_flags.extra_flags, ITEM_HARDEN))
        return;
     
  }
*/  
  if (GET_ITEM_TYPE(obj)==ITEM_CONTAINER) {
    if (obj->contains) {
      act("The contents of $p spill out.",TRUE,ch,obj,0,TO_CHAR);
      act("The contents of $p spill out.",TRUE,ch,obj,0,TO_ROOM);
      for(obj2=obj->contains; obj2; obj2=next) {
	next=obj2->next_content;
	obj_from_obj(obj2);
	obj_to_char(obj2, ch);
	if (DamageOneItem(ch, dam_type, obj2))
	  MakeScrap(ch, obj2, dam_type);
	else {
	  act("$p falls to the ground.",TRUE,ch,obj2,0,TO_CHAR);
	  act("$p falls to the ground.",TRUE,ch,obj2,0,TO_ROOM);
	  obj_from_char(obj2);
	  obj_to_room(obj2, ch->in_room);
	}
      }
    }
  }

 
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);
  
  if(!(t = make_object(30, VIRTUAL)))
    return;

  ss_free(t->description);
  sprintf(buf, "%s scraps lie here.", OBJ_SHORT(obj));
  t->description = ss_make(buf);

  if (obj->carried_by)
  {
    obj_from_char(obj);
  }
  else if (obj->equipped_by)
  {
    obj=unequip_char(ch, obj->eq_pos);
  }
  extract_obj(obj);
  
  t->item_number = NOTHING;
  t->in_room = NOWHERE;
  
  t->obj_flags.timer = MAX_SCRAP_TIME;
  t->obj_flags.value[3] = SCRAP_ITEM;

  obj_to_room(t, ch->in_room);

  CREATE(event, object_event, 1);
  event->object = t;

  sprintf(buf, "scraps %s", OBJ_NAME(t));
  t->corpse_timer = event_queue_pulse((event_t*) event,
					   pulse,
					   (event_func) scraps_timer,
					   buf);
}


void DamageAllStuff( struct char_data *ch, int dam_type)
{
  int j;
  struct obj_data *obj, *next;
  
  /* this procedure takes all of the items in equipment and inventory
     and damages the ones that should be damaged */
  
  /* equipment */
  for (j = 0; j < MAX_WEAR; j++) {
    if (ch->equipment[j] && ch->equipment[j]->item_number>=0) {
      obj = ch->equipment[j];
      if (DamageOneItem(ch, dam_type, obj)) { /* TRUE == destroyed */
	if ((obj = unequip_char(ch,j))!=NULL) {
	  MakeScrap(ch, obj, dam_type);
	} else {
	  log_msg("hmm, really wierd!");
	}
      }
    }
  }
  
  /* inventory */
  obj=ch->carrying;
  while (obj) {
    next=obj->next_content;
    if (obj->item_number >= 0) {
      if (DamageOneItem(ch, dam_type, obj)) {
	MakeScrap(ch, obj, dam_type);
      }
    }
    obj = next;
  }
}

int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
  /*  damage weapons or armor */
  
/*  if (ITEM_TYPE(o) == ITEM_ARMOR) {
    o->obj_flags.value[0] -= num;
    if (o->obj_flags.value[0] < 0) {
      return(TRUE);
    }    
  } else if (ITEM_TYPE(o) == ITEM_WEAPON) {
    o->obj_flags.value[2] -= num;
    if (o->obj_flags.value[2] <= 0) {
      return(TRUE);
    }
  } */
  return(FALSE);
}

int ItemSave( struct obj_data *i, int dam_type) 
{
  int num, j;

  if (IS_OBJ_STAT(i,ITEM_BRITTLE)) {
    return(FALSE);
  }

  if (IS_OBJ_STAT(i,ITEM_HARDEN)) {
    return TRUE;
  }
   
  num = number(1,20);
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if ((i->affected[j].location == APPLY_SAVING_SPELL) || 
	(i->affected[j].location == APPLY_SAVE_ALL)) {
      num -= i->affected[j].modifier;
    }
  if (i->affected[j].location != APPLY_NONE) {
    num += 1;
  }
  if (i->affected[j].location == APPLY_HITROLL) {
    num += i->affected[j].modifier;
  }
  
  if (ITEM_TYPE(i) != ITEM_ARMOR)
    num += 1;
  
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i)-1][dam_type-1]) {
    return(TRUE);
  } else {
    return(FALSE);
  }
}



int DamagedByAttack(struct obj_data *i, int dam_type)
{
  int num = 0;
  
  if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)){
    while (!ItemSave(i,dam_type)) {
      num+=1;
      if (num > 75)
	return(num);  /* 10+ ac points = !destroyed */ 
    }
    return(num);
  } else {
    if (ItemSave(i, dam_type)) {
      return(0);
    } else {
      return(-1);
    }
  }
}


void DamageStuff(struct char_data *v, int type, int dam)
{
  int num, dam_type;
  struct obj_data *obj;
  
  if(dam <= 0)
    return;
 
  if (percent() < 25) { 
    if (type >= TYPE_HIT && type <= TYPE_SMITE) {
      num = number(3,17);/* wear_neck through hold */
      if (v->equipment[num]) {
        if ((type == TYPE_BLUDGEON && dam > 10) ||
	  (type == TYPE_CRUSH && dam > 5) ||
	  (type == TYPE_SMASH && dam > 10) ||
	  (type == TYPE_BITE && dam > 15) ||
	  (type == TYPE_CLAW && dam > 20) ||
	  (type == TYPE_SLASH && dam > 30) ||
	  (type == TYPE_SMITE && dam > 10) ||
	  (type == TYPE_HIT && dam > 20)) {
  	  if (DamageOneItem(v, BLOW_DAMAGE, v->equipment[num])) {
	    if ((obj = unequip_char(v,num))!=NULL) {
	      MakeScrap(v, obj, BLOW_DAMAGE);
	    }
	  }
        }
      }
    }
    else
    {
      dam_type = GetItemDamageType(type);
      if (dam_type) {
        DamageAllStuff(v, dam_type);
      }
    }
  }
}


int GetItemDamageType(int type)
{
  switch(type) {
  case SPELL_BURNING_HANDS:
  case SPELL_FIRE_WIND:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIREBALL:
  case SPELL_LAVA_STORM:
  case SPELL_FIRE_BREATH:
  case SKILL_FIRE_AURA:
    return(FIRE_DAMAGE);
    break;
    
  case SPELL_CALL_LIGHTNING:
  case SPELL_SHOCKING_GRASP:
  case SPELL_ELECTROCUTE:
  case SPELL_ELECTRIC_FIRE:
  case SPELL_CHAIN_ELECTROCUTION:
  case SPELL_LIGHTNING_BREATH:
    return(ELEC_DAMAGE);
    break;

  case SPELL_ACID_BLAST:
  case SPELL_ACID_RAIN:    
  case SPELL_ACID_BREATH:
  case SKILL_ACID_AURA:
    return(ACID_DAMAGE);
  default:
    return(0);
    break;  
  }
  
}
