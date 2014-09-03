
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#if USE_stdlib
#include <stdlib.h>
#endif
    
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "act.h"
#include "utility.h"
#include "constants.h"
#include "limits.h"
#include "multiclass.h"
#include "spelltab.h"
#include "find.h"
#include "periodic.h"
#include "util_str.h"
#include "proto.h"
#include "mobprog2.h"

/* forward declarations */
    int remove_equip(struct char_data* ch, int index, int verbose);


void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if((obj->obj_flags.weight + weight) < 1) 
    {
	weight = 1 - obj->obj_flags.weight;
	slog("Bad weight change on an object");
    }

    obj->obj_flags.weight += weight;
    
    if((tmp_ch = obj->carried_by))
	IS_CARRYING_W(tmp_ch) += weight;

    for(tmp_obj = obj->in_obj ; tmp_obj ; tmp_obj = tmp_obj->in_obj)
	tmp_obj->obj_flags.cont_weight += weight;
}



void name_from_drinkcon(struct obj_data *obj)
{
  int i;
  sstring_t* new_name;
  
  for(i = 0 ; OBJ_NAME(obj)[i] != ' ' && OBJ_NAME(obj)[i] ; ++i)
      ;
  
  if(OBJ_NAME(obj)[i] == ' ')
  {
    new_name = ss_make(OBJ_NAME(obj) + i + 1);
    ss_free(obj->name);
    obj->name = new_name;
  }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
  char buf[256];
  
  sprintf(buf, "%s %s", drinknames[type], OBJ_NAME(obj));
  ss_free(obj->name);
  obj->name = ss_make(buf);
}



void do_drink(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];
  struct obj_data *temp;
  struct affected_type af;
  int amount;
  
  
  only_argument(argument,buf);
  
  if(!(temp = get_obj_vis_accessible(ch,buf)))	{
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if (temp->obj_flags.type_flag!=ITEM_DRINKCON)	{
    act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0)) {
    /* The pig is drunk */
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  
  if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
    {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if (temp->obj_flags.type_flag==ITEM_DRINKCON){
    if (temp->obj_flags.value[1]>0)  /* Not empty */ {
      sprintf(buf,"$n drinks %s from $p",drinks[temp->obj_flags.value[2]]);
      act(buf, TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
      send_to_char(buf,ch);
      
      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
	amount = (25-GET_COND(ch,THIRST))/
	  drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
	amount = number(3,10);
      
      amount = MIN(amount,temp->obj_flags.value[1]);
      /* Subtract amount, if not a never-emptying container */
      if (!IS_SET(temp->obj_flags.value[3],DRINK_PERM) &&
	  (temp->obj_flags.value[0] > 20))
	weight_change_object(temp, -amount);
      
      gain_condition(ch,DRUNK,(int)((int)drink_aff
				    [temp->obj_flags.value[2]][DRUNK]*amount)/4
);
      
      if(GET_COND(ch, FULL) != -1)
	gain_condition(ch,FULL,(int)((int)drink_aff
				     [temp->obj_flags.value[2]][FULL]*amount)/4);
      
      if(GET_COND(ch, THIRST) != -1)
	gain_condition(ch,THIRST,(int)((int)drink_aff
				       [temp->obj_flags.value[2]][THIRST]*amount)
/4);
      
      if(GET_COND(ch,DRUNK)>10)
	act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
      
      if(GET_COND(ch,THIRST)>20)
	act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
      
      if(GET_COND(ch,FULL)>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

      /* The shit was poisoned ! */      
      if(IS_SET(temp->obj_flags.value[3],DRINK_POISON))
	{
	  act("Oops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
	  act("$n chokes and utters some strange sounds.",
	      TRUE,ch,0,0,TO_ROOM);
	  af.type = SPELL_POISON;
	  af.duration = amount*3;
	  af.modifier = 0;
	  af.location = APPLY_NONE;
	  af.bitvector = AFF_POISON;
	  af.mana_cost = 0;
	  af.caster = 0;
	  affect_join(ch,&af, FALSE, FALSE);
	}
      
      /* empty the container, and no longer poison. */
      if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
	temp->obj_flags.value[1]-= amount;
      if(!temp->obj_flags.value[1]) {  /* The last bit */
	temp->obj_flags.value[2]=0;
	temp->obj_flags.value[3]=0;
	name_from_drinkcon(temp);
      }
#ifdef PURGE_EMPTY_DRINKCON
      if (temp->obj_flags.value[1] < 1) {  /* its empty */
	if (temp->obj_flags.value[0] < 20) {  
	  extract_obj(temp);  /* get rid of it */
	}
      }
#endif
      return;
      
    }
    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
    
    return;
    
  }
  
}



void do_eat(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int j;
	struct obj_data *temp;
	struct affected_type af;

	one_argument(argument,buf);

	if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))	{
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if((temp->obj_flags.type_flag != ITEM_FOOD) && 
	   (TRUST(ch) < TRUST_DEMIGOD))	{
       	    act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

if(GET_COND(ch,FULL)>20)  /* Stomach full */ {
		act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
	act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

	if(GET_COND(ch, FULL) != -1)
	  gain_condition(ch,FULL,temp->obj_flags.value[0]);

	if(GET_COND(ch,FULL)>20)
		act("You are full.",FALSE,ch,0,0,TO_CHAR);

        for(j=0; j<MAX_OBJ_AFFECT; j++)
	    if (temp->affected[j].location == APPLY_EAT_SPELL) {
	      struct spell_info *spell;

	      if(!(spell = spell_by_number(temp->affected[j].modifier)))
	      {
		char buf[256];
		sprintf(buf, "Illegal EAT_SPELL: %ld on %s",
			temp->affected[j].modifier, OBJ_NAME(temp));
		log_msg(buf);
	      }
	      else
	      {
		/* hit 'em with the spell */
		(spell->spell_pointer)(6, ch, "",
					SPELL_TYPE_POTION, ch, 0);
	      }
	    }
	

	if(temp->obj_flags.value[3] && !IS_IMMORTAL(ch))
	{
	    act("That tasted rather strange !!",FALSE,ch,0,0,TO_CHAR);
	    act("$n coughs and utters some strange sounds.",
		FALSE,ch,0,0,TO_ROOM);

	    af.type = SPELL_POISON;
	    af.duration = temp->obj_flags.value[0]*2;
	    af.modifier = 0;
	    af.location = APPLY_NONE;
	    af.bitvector = AFF_POISON;
	    af.mana_cost = 0;
	    af.caster = 0;
	    affect_join(ch,&af, FALSE, FALSE);
	}

	extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[256];
    struct obj_data *from_obj;
    struct obj_data *to_obj;
    int temp;
  
    argument_interpreter(argument, arg1, arg2);
  
    if(!*arg1)			/* No arguments */	{
	act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))	{
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)	{
	act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(from_obj->obj_flags.value[1]==0)	{
	act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
	return;
    }
  
    if(!*arg2)	{
	act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(!str_cmp(arg2,"out")) {
	act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
	act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);
    
	weight_change_object(from_obj, -from_obj->obj_flags.value[1]);
    
	from_obj->obj_flags.value[1]=0;
	from_obj->obj_flags.value[2]=0;
	from_obj->obj_flags.value[3]=0;
	name_from_drinkcon(from_obj);
    
	return;
    
    }
  
    if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))  {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)	{
	act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if((to_obj->obj_flags.value[1]!=0)&&
       (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2])) {
	act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))	{
	act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(to_obj == from_obj)
    {
	sprintf(buf, "You juggle %s a bit.\n", OBJ_SHORT(to_obj));
	send_to_char(buf, ch);
	return;
    }
    
    sprintf(buf,"You pour the %s into the %s.",
	    drinks[from_obj->obj_flags.value[2]],arg2);
    send_to_char(buf,ch);
  
    /* New alias */
    if (to_obj->obj_flags.value[1]==0) 
	name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);
  
    /* First same type liq. */
    to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];
    
    /*
      the new, improved way of doing this...
      */
    temp = from_obj->obj_flags.value[1];
    from_obj->obj_flags.value[1] = 0;
    to_obj->obj_flags.value[1] += temp;
    temp = to_obj->obj_flags.value[1] - to_obj->obj_flags.value[0];
  
    if (temp>0) {
	from_obj->obj_flags.value[1] = temp;
    } else {
	name_from_drinkcon(from_obj);
    }
  
    if (from_obj->obj_flags.value[1] > from_obj->obj_flags.value[0])
	from_obj->obj_flags.value[1] = from_obj->obj_flags.value[0];
  
    /* Then the poison boogie */
    to_obj->obj_flags.value[3]=
	(to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);
  
    return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;
  
  one_argument(argument,arg);
  
  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
    {
      act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
      act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
      return;
    }
  
  if(!temp->obj_flags.value[1])  /* Empty */
    {
      act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
  send_to_char(buf,ch);
  
  gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));
  
  if (GET_COND(ch,FULL) != -1)
    gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));
  
  if (GET_COND(ch,THIRST) != -1)
    gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));
  
  if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM) ||
     (temp->obj_flags.value[0] > 19))
    weight_change_object(temp, -1);  /* Subtract one unit, unless permanent */
  
  if(GET_COND(ch,DRUNK)>10)
    act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
  
  if(GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
  
  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);
  
  if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)
     && !IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
      act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);
      
      af.type = SPELL_POISON;
      af.duration = 3;
      af.modifier = 50;
      af.location = APPLY_DRAIN_LIFE;
      af.bitvector = AFF_POISON;
      af.mana_cost = 0;
      af.caster = 0;
      DLOG(("Calling affect_to_char from do_sip: act.obj2.c line 464\n\r"));
      affect_to_char(ch,&af);
    }
  
  if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
    temp->obj_flags.value[1]--;
  
  if(!temp->obj_flags.value[1])  /* The last bit */
    {
      temp->obj_flags.value[2]=0;
      temp->obj_flags.value[3]=0;
      name_from_drinkcon(temp);
    }
  
  return;
  
}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_INPUT_LENGTH];
    struct obj_data *temp;
  
    one_argument(argument,arg);
  
    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    if(temp->obj_flags.type_flag==ITEM_DRINKCON)
    {
	do_sip(ch,argument,0);
	return;
    }
  
    if(!(temp->obj_flags.type_flag==ITEM_FOOD))
    {
	act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
	return;
    }
  
    act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);
  
    if (GET_COND(ch,FULL) != -1)
	gain_condition(ch,FULL,1);
  
    if(GET_COND(ch,FULL)>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);
  
    if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
	act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);
      
	af.type = SPELL_POISON;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	af.mana_cost = 0;
	af.caster = 0;
	DLOG(("Calling affect_to_char from do_taste: act.obj2.c line 528\n\r"));
	affect_to_char(ch,&af);
    }
  
    temp->obj_flags.value[0]--;
  
    if(!temp->obj_flags.value[0]) { /* Nothing left */
	act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
	extract_obj(temp);
    }
  
    return;

}



/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  char buf[MAX_STRING_LENGTH];
  
  switch(keyword) {
  case WEAR_LIGHT:
    sprintf(buf, "You light %s and hold it.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n lights $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_FINGER_R: 
  case WEAR_FINGER_L:
    sprintf(buf, "You slide %s onto your ",OBJS(obj_object,ch));
    if (keyword==WEAR_FINGER_R)
      strcat(buf, "right");
    else
      strcat(buf, "left");
    strcat(buf, " finger.\n\r");
    send_to_char(buf,ch);
    act("$n slides $p onto $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_NECK_1: 
  case WEAR_NECK_2:
    sprintf(buf, "You wrap %s around your neck.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n wraps $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_BODY: 
    sprintf(buf, "You pull %s over your body.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n pulls $p over $s body.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_HEAD: 
    sprintf(buf, "You place %s on your head.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n places $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_LEGS: 
    sprintf(buf, "You slip into %s.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n slips into $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_FEET: 
    sprintf(buf, "You place %s on your feet.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n places $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_HANDS: 
    sprintf(buf, "You put %s on your hands.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n puts $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_ARMS: 
    sprintf(buf, "You wear %s on your arms.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_ABOUT: 
    sprintf(buf, "You wrap %s about your body..\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n wraps $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_WAISTE: 
    sprintf(buf, "You tie %s around your waist.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n ties $p around $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_WRIST_R: 
  case WEAR_WRIST_L:
    sprintf(buf, "You slip %s around your ", OBJS(obj_object,ch));
    if (keyword==WEAR_WRIST_R)
      strcat(buf, "right");
    else
      strcat(buf, "left");
    strcat(buf, " wrist.\n\r");
    send_to_char(buf,ch);
    act("$n slips $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WIELD: 
    sprintf(buf, "You pull out %s and wield it.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n pulls out $p and wields it.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case HOLD: 
    sprintf(buf, "You grab onto %s.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case WEAR_SHIELD: 
    sprintf(buf, "You start using %s.\n\r",OBJS(obj_object,ch));
    send_to_char(buf,ch);
    act("$n starts using $p as shield.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  }
}


int IsRestricted(int Mask, int Class)
{
    return !(((Mask & Class) ^ Class) & CLASS_MASK);
}

int StrCanHold(struct obj_data* hold,
		struct char_data* ch)
{
    int weight = 0;

    if(hold)
	weight += GET_OBJ_WEIGHT(hold);

    return weight <= str_app[STRENGTH_APPLY_INDEX(ch)].wield_w;
    
}

int StrCanDualWield(struct obj_data* hold,
		struct char_data* ch)
{
    int weight = 0;

    if(hold)
	weight += GET_OBJ_WEIGHT(hold);

    return weight <= (str_app[STRENGTH_APPLY_INDEX(ch)].wield_w/2);
}

void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  char buffer[MAX_STRING_LENGTH];
  int BitMask, count, hands;
  


  if (!IS_IMMORTAL(ch)) {

    /* note i redid the check for if the player was pure class because really thats handled
     * by the pureitemclass number
     */
    
    if (IS_PURE_ITEM(obj_object)) { /* if its a PURE_CLASS ITEM */
      if (PureItemClass(obj_object) != ch->player.clss) { /* is this the right class? */
	send_to_char("You have no idea how to use that!\n\r", ch); /* nopers *BANG* */
	return;
      }
    }
    else { /* its not a pure class item so do regular class check */
      
      BitMask = GetItemClassRestrictions(obj_object);
      
      if (IsRestricted(BitMask, ch->player.clss) && IS_PC(ch))
	{
	  send_to_char("You are forbidden to do that.\n\r", ch);
	  return;
	}
    }
    
  } /* end of checking class restricts if not immortal */
  
  if (!IsHumanoid(ch) && IS_NPC(ch))
    {
      if ((keyword!=WEAR_LIGHT) || (!HasHands(ch)))
	{
	  send_to_char("You can't wear things!\n\r",ch);
	  return;
	}
    }
  
  if(!IsLevelOk(ch, obj_object))
    return;
  
  if (affected_by_spell(ch, SKILL_FORM) && shifter_form_test(keyword)) {
    send_to_char("You can't use that while your arms and hands are weapons.\n\r", ch);
    return;
  }

  if (affected_by_spell(ch, SKILL_PLATE) && shifter_plate_test(keyword)) {
    send_to_char("You can't use that while your skin is armor plating.\n\r", ch);
    return;
  }

  count = 0;
  hands = NUM_OF_HANDS;
  
  if (ch->equipment[WEAR_LIGHT]) count++;
  if (ch->equipment[WEAR_SHIELD]) count++;
  if (ch->equipment[HOLD]) count++;

  if (affected_by_spell(ch, SKILL_LIMB)) 
  {
    hands++;
    if (GET_LEVEL(ch, SHIFTER_LEVEL_IND) >= 50)
      hands++;
  }
  
  if (ch->equipment[WIELD])
    {
      count++;
      if (IS_OBJ_STAT(ch->equipment[WIELD],ITEM_TWO_HANDED))
	count++;
    }
  
  switch(keyword) {
  case WEAR_LIGHT:
    if(!CAN_WEAR(obj_object, ITEM_WEAR_LIGHT))
      send_to_char("That's not a light.\n\r", ch);
    else if (ch->equipment[WEAR_LIGHT])
      send_to_char("You are already holding a light source.\n\r", ch);
    else if (count > hands-1)
      send_to_char("You cannot hold any more items in your hands.\n\r", ch);
    else if(!StrCanHold(obj_object, ch))
      send_to_char("It is too heavy for you to use.\n\r",ch);
    else
    {
      if (obj_object->obj_flags.type_flag == ITEM_LIGHT &&
	  obj_object->obj_flags.value[2])
      {
	if (real_roomp(ch->in_room)->light < 0)
	  real_roomp(ch->in_room)->light=0; 
	real_roomp(ch->in_room)->light++;
      }
      goto equip;
    }
    break;
    
  case WEAR_FINGER_R:
  case WEAR_FINGER_L:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_FINGER))
      send_to_char("You can't wear that on your finger.\n\r", ch);
    else if (ch->equipment[WEAR_FINGER_L] &&
	     ch->equipment[WEAR_FINGER_R])
      send_to_char("You are already wearing something on your fingers.\n\r", ch);
    else
    {
      if (ch->equipment[WEAR_FINGER_L])
      {
	keyword=WEAR_FINGER_R;
	goto equip;
      }
      else
      {
	keyword=WEAR_FINGER_L;
	goto equip;
      }
      
    }
    break;

  case WEAR_NECK_1:
  case WEAR_NECK_2:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_NECK))
      send_to_char("You can't wear that around your neck.\n\r", ch);
    else if (ch->equipment[WEAR_NECK_1] && ch->equipment[WEAR_NECK_2])
      send_to_char("You can't wear any more around your neck.\n\r", ch);
    else
    {
      if (ch->equipment[WEAR_NECK_1])
      {
	keyword=WEAR_NECK_2;
	goto equip;
      }
      else
      {
	keyword=WEAR_NECK_1;
	goto equip;
      }
    }
    break;

  case WEAR_BODY:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_BODY))
      send_to_char("You can't wear that on your body.\n\r", ch);
    else if (ch->equipment[WEAR_BODY])
      send_to_char("You already wear something on your body.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_HEAD:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_HEAD))
      send_to_char("You can't wear that on your head.\n\r", ch);
    else if (ch->equipment[WEAR_HEAD])
      send_to_char("You already wear something on your head.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_LEGS:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_LEGS))
      send_to_char("You can't wear that on your legs.\n\r", ch);
    else if (ch->equipment[WEAR_LEGS])
      send_to_char("You already wear something on your legs.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_FEET:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_FEET))
      send_to_char("You can't wear that on your feet.\n\r", ch);
    else if (ch->equipment[WEAR_FEET])
      send_to_char("You already wear something on your feet.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_HANDS:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_HANDS))
      send_to_char("You can't wear that on your hands.\n\r", ch);
    else if (ch->equipment[WEAR_HANDS])
      send_to_char("You already wear something on your hands.\n\r", ch);
    else
      goto equip;
    break;
	
  case WEAR_ARMS:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_ARMS))
      send_to_char("You can't wear that on your arms.\n\r", ch);
    else if (ch->equipment[WEAR_ARMS])
      send_to_char("You already wear something on your arms.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_ABOUT:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_ABOUT))
      send_to_char("You can't wear that about your body.\n\r", ch);
    else if (ch->equipment[WEAR_ABOUT])
      send_to_char("You already wear something about your body.\n\r", ch);
    else
      goto equip;
    break;

  case WEAR_WAISTE:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_WAISTE))
      send_to_char("You can't wear that about your waist.\n\r", ch);
    else if (ch->equipment[WEAR_WAISTE])
      send_to_char("You already wear something about your waist.\n\r",
		   ch);
    else 
      goto equip;
    break;

  case WEAR_WRIST_R:
  case WEAR_WRIST_L:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_WRIST))
      send_to_char("You can't wear that around your wrist.\n\r", ch);
    else if (ch->equipment[WEAR_WRIST_L] && ch->equipment[WEAR_WRIST_R])
      send_to_char("You already wear something around both wrists.\n\r",
		   ch);
    else
    {
      if (ch->equipment[WEAR_WRIST_L]) {
	keyword=WEAR_WRIST_R;
	goto equip;
      } else {
	keyword=WEAR_WRIST_L;
	goto equip;
      }
    }
    break;
    
  case WIELD:
    if (!CAN_WEAR(obj_object,ITEM_WIELD))
      send_to_char("You can't wield that.\n\r", ch);
    else if (ch->equipment[WIELD])
      send_to_char("You are already wielding something.\n\r", ch);
    else if(!StrCanHold(obj_object, ch))
      send_to_char("It is too heavy for you to use.\n\r",ch);
    else if(count > hands-1)
      send_to_char("You cannot hold any more items in your hands.\n\r",ch);
    else if (IS_OBJ_STAT(obj_object,ITEM_TWO_HANDED) && ch->equipment[HOLD])
      send_to_char("You do not have both of your hands free.\n\r",ch);
    else if (HasClass(ch, CLASS_MONK) ) // monks aren't allowed to wield anything 
      send_to_char("Due to religious beliefs, you may not wield anything.\n\r", ch);
    else
      goto equip;
    break;
    
  case HOLD:
    if (!CAN_WEAR(obj_object,ITEM_HOLD) && !CAN_WEAR(obj_object, ITEM_WIELD))
      send_to_char("You can't hold this.\n\r", ch);
    else if (ch->equipment[HOLD])
      send_to_char("You are already holding something.\n\r", ch);
    else if((!CAN_WEAR(obj_object,ITEM_HOLD) && !StrCanDualWield(obj_object,ch))
	     || !StrCanHold(obj_object, ch))
      send_to_char("It is too heavy for you to use.\n\r",ch);
    else if(count > hands-1)
      send_to_char("You cannot use any more items in your hands.\n\r",ch);
    else if (IS_OBJ_STAT(obj_object,ITEM_TWO_HANDED))
      send_to_char("You must wield a two handed item, not hold it\n\r",ch);
    else if (ch->equipment[WIELD] && IS_OBJ_STAT(ch->equipment[WIELD],ITEM_TWO_HANDED))
      send_to_char("Sorry, your hands are full.\n\r",ch);

    else
      goto equip;
    break;

  case WEAR_SHIELD:
    if (!CAN_WEAR(obj_object,ITEM_WEAR_SHIELD))
      send_to_char("You can't use that as a shield.\n\r", ch);
    else if (ch->equipment[WEAR_SHIELD])
      send_to_char("You are already using a shield\n\r", ch);
    else if(!StrCanHold(obj_object, ch))
      send_to_char("It is too heavy for you to use.\n\r",ch);
    else if(count > hands-1)
      send_to_char("You can't hold any more items in your hands.\n\r", ch);
    else
      goto equip;
    break;

  case -1:
    sprintf(buffer,"Wear %s where?.\n\r", OBJ_SHORT(obj_object));
    send_to_char(buffer, ch);
    break;

  case -2:
    sprintf(buffer,"You can't wear %s.\n\r", OBJ_SHORT(obj_object));
    send_to_char(buffer, ch);
    break;

  default:
    log_msg("Unknown type called in wear.");
    break;
  }
  return;
 equip:
      perform_wear(ch,obj_object,keyword);
      obj_from_char(obj_object);
      equip_char(ch, obj_object, keyword);
      oprog_wear_trigger(obj_object, ch);
}


void do_wear(struct char_data *ch, char *argument, int cmd) 
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[256];
    char buffer[MAX_INPUT_LENGTH];
    struct obj_data *obj_object, *next_obj;
    int keyword;
    static const char *keywords[] = {
	"light",
	"finger", "finger",
	"neck", "neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"shield",
	"about",
	"waist",
        "wrist", "wrist",
        "wield",
        "hold",
	"\n"
	};
  
    argument_interpreter(argument, arg1, arg2);
    if (!*arg1)
	send_to_char("Wear what?\n\r", ch);
    else if (!strcmp(arg1,"all"))
    {
	for (obj_object = ch->carrying; obj_object; obj_object = next_obj)
	{
	    next_obj = obj_object->next_content;
	    keyword = -2;
	    if (CAN_SEE_OBJ(ch, obj_object))
	    {
	      if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = WEAR_SHIELD;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = WEAR_FINGER_R;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = WEAR_NECK_1;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = WEAR_WRIST_R;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = WEAR_WAISTE;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = WEAR_ARMS;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = WEAR_HANDS;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = WEAR_FEET;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = WEAR_LEGS;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = WEAR_ABOUT;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = WEAR_HEAD;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = WEAR_BODY;
	      if (CAN_WEAR(obj_object,ITEM_WIELD)) keyword = WIELD;
	      if (CAN_WEAR(obj_object,ITEM_HOLD)) keyword = HOLD;
	      if (CAN_WEAR(obj_object,ITEM_WEAR_LIGHT)) keyword = WEAR_LIGHT;
	    }
	    
	    if (keyword != -2)
	    {
		sprintf(buf,"%s :", OBJS(obj_object, ch));
		send_to_char(buf,ch);
		wear(ch, obj_object, keyword);
	    }
	}
    }
    else if((obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying)) == NULL)
    {
	sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	send_to_char(buffer,ch);
    }
    else if (*arg2)
    {
	keyword = search_block(arg2, keywords, FALSE);/* Partial Match */
	if (keyword == -1)
	{
	    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
	    send_to_char(buf, ch);
	}
	else
	{
	    sprintf(buf,"%s :", OBJS(obj_object,ch));
	    send_to_char(buf,ch);
	    wear(ch, obj_object, keyword);
	}
    }
    else
    {
	keyword = -2;
	if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = WEAR_SHIELD;
	if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = WEAR_FINGER_R;
	if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = WEAR_NECK_1;
	if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = WEAR_WRIST_R;
	if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = WEAR_WAISTE;
	if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = WEAR_ARMS;
	if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = WEAR_HANDS;
	if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = WEAR_FEET;
	if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = WEAR_LEGS;
	if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = WEAR_ABOUT;
	if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = WEAR_HEAD;
	if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = WEAR_BODY;
        if (CAN_WEAR(obj_object,ITEM_WIELD)) keyword = WIELD;
        if (CAN_WEAR(obj_object,ITEM_HOLD)) keyword = HOLD;
        if (obj_object->obj_flags.type_flag==ITEM_LIGHT)
            keyword=WEAR_LIGHT;
	  
	sprintf(buf,"%s :", OBJS(obj_object,ch));
	send_to_char(buf,ch);
	wear(ch, obj_object, keyword);
    }
}


void do_wield(struct char_data *ch, char *argument, int cmd) 
{
  char arg1[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  
  only_argument(argument, arg1);
  if (*arg1) {
    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object) {
      sprintf(buf,"%s :", OBJ_SHORT(obj_object));
      send_to_char(buf,ch);
      wear(ch, obj_object, WIELD);
      return;
    }
  }
  send_to_char("Wield what?\n\r", ch);
}

void do_quick_draw(struct char_data* ch, char* argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    struct obj_data* obj;
    
    one_argument(argument, arg1);

    if(!*arg1)
    {
	send_to_char("Quick draw what?\n\r", ch);
    }
    else if((obj = get_obj_in_list_vis(ch, arg1, ch->carrying)) == NULL)
    {
	act("You don't seem to have the $T.",
	    FALSE, ch, NULL, arg1, TO_CHAR);
    }
    else if(obj->obj_flags.type_flag != ITEM_WEAPON)
    {
	act("You can't wield $o.", FALSE, ch, obj, NULL, TO_CHAR);
    }
    else if(!ch->skills ||
	    (number(1,101) > (MIN(GET_DEX(ch), 20) *
			      ch->skills[SKILL_QUICK_DRAW].learned / 20)))
    {
	remove_equip(ch, WIELD, TRUE);
	send_to_char("You fumble the attempt to change weapons.\n\r", ch);
	act("$n fumbles an attempt to quickly change weapons.",
	    TRUE, ch, NULL, NULL, TO_ROOM);

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    }
    else
    {
	remove_equip(ch, WIELD, TRUE);
	if(!ch->equipment[WIELD])
	    wear(ch, obj, WIELD);
    }
}
    
void do_grab(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    struct obj_data *obj_object;
  
    only_argument(argument, arg1);
    if (*arg1)
    {
	obj_object = get_obj_in_list(arg1, ch->carrying);
	if (obj_object)
	{
	  if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	  {
	    sprintf(buf,"%s :", OBJS(obj_object, ch));
	    send_to_char(buf,ch);
	    wear(ch, obj_object, WEAR_LIGHT);
	  }
	  else if (CAN_SEE_OBJ(ch, obj_object))
	  {
	    sprintf(buf,"%s :", OBJ_SHORT(obj_object));
	    send_to_char(buf,ch);
	    wear(ch, obj_object, HOLD);
	  }
	  else
	    send_to_char("You don't have such an object.\n\r",ch);
	  return;
	}
    }
    send_to_char("Hold what?\n\r", ch);
}


void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH],*T,*P;
  char buffer[256];
  struct obj_data *obj_object = NULL;
  int j;
  
  one_argument(argument, arg1);
  
  if(!*arg1)
  {
    send_to_char("Remove what?\n\r", ch);
    return;
  }
  
  if (!strcmp(arg1,"all"))
  {
    for (j=0;j<MAX_WEAR;j++)
      if(ch->equipment[j])
	remove_equip(ch, j, FALSE);
    act("$n stops using $s equipment.",TRUE,ch,obj_object,0,TO_ROOM);
  }
  else if (isdigit(arg1[0]))
  {
    T = arg1;
      
    while (isdigit(*T) && (*T != '\0'))
    {
      if((P = strchr(T, ',')))
      {
	*P++ = '\0';
      }
      else
	P = T + strlen(T);

      j = atoi(T);
      if(j<0 || j>MAX_WEAR || !ch->equipment[j])
      {
	sprintf(buffer, "You aren't wearing %s\n\r", T);
	send_to_char(buffer,ch);
      }
      else
	remove_equip(ch, j, TRUE);

      T = P;
    }
  }
  else if(!(obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j)))
  {
    sprintf(buffer, "You aren't wearing %s\n\r", arg1);
    send_to_char(buffer, ch);
  }
  else
  {
    remove_equip(ch, j, TRUE);
  }
}

int remove_equip(struct char_data* ch, int index, int verbose)
{
    struct obj_data* obj_object;
    struct obj_data* missile;

    missile = (index == WIELD) ? ch->equipment[LOADED] : (struct obj_data *) NULL;
  
    if(!(obj_object = ch->equipment[index]))
	return 0;
  
    if (IS_OBJ_STAT(obj_object,ITEM_NODROP))
    {
	send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
	return 0;
    }
    if(missile && IS_OBJ_STAT(missile, ITEM_NODROP))
    {
	send_to_char("The missile is CURSED!  You can't let go of either.\n\r",
		     ch);
	return 0;
    }

    if (CAN_CARRY_N(ch) <= ((missile ? 1 : 0) + IS_CARRYING_N(ch)))
    {
	send_to_char("You can't carry that many items.\n\r", ch);
	return 0;
    }
   

    /* This makes Berserk Gear un-removeable during combat.
    /* MC Solaar - Removed per-request of Gaius.
    if (ch->specials.fighting) {
        for(int i=0;i<MAX_OBJ_AFFECT;i++) {
	   if(obj_object->affected[i].location == APPLY_SPELL &&
	      (obj_object->affected[i].modifier & AFF_BERSERK)) {
	      send_to_char("You can't remove that when fighting!\n\r", ch);
	      return 0;
	   }
	}
    }
    */
  
    obj_to_char(unequip_char(ch, index), ch);
	  
    if (index == WEAR_LIGHT &&
	obj_object->obj_flags.type_flag == ITEM_LIGHT &&
	obj_object->obj_flags.value[2])
    {
      if(real_roomp(ch->in_room)->light < 0)
	real_roomp(ch->in_room)->light=0;
      real_roomp(ch->in_room)->light--;
    }

    if(verbose)
    {
	act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
	act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);
    }

    if(missile) 
	remove_equip(ch, LOADED, FALSE);
  
    oprog_remove_trigger(obj_object, ch);
   
    return 1;
}

