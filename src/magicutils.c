/*
**  magicutils -- stuff that makes the magic files easier to read.
*/

#include "config.h"

#include <stdio.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "multiclass.h"
#include "utility.h"
#include "fight.h"
#include "db.h"
#include "act.h"

void SwitchStuff( struct char_data *giver, struct char_data *taker)
{
  struct obj_data *obj, *next;
  double ratio;
  int j;
  
  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */
  
  for (j = 0; j< MAX_WEAR; j++) {
    if (giver->equipment[j]) {
      obj = unequip_char(giver, j);
      obj_to_char(obj, taker);
    }
  }
  
  for (obj = giver->carrying; obj; obj = next) {
    next = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, taker);
  }
  
  /*
   *    gold...
   */

  GET_GOLD(taker) = GET_GOLD(giver);
  
  /*
   *   hit point ratio
   */

   ratio = (double) GET_HIT(giver) / (double) GET_MAX_HIT(giver);
   if(ratio > 1.0) ratio = 1.0;
   GET_HIT(taker) = (int) (ratio * (double) GET_MAX_HIT(taker));
   if(GET_HIT(taker)<1)
     GET_HIT(taker)=1;

  /*
   * experience
   */

   GET_EXP(taker) = GET_EXP(giver);

  /*
   *  humanoid monsters can cast spells
   */

  if (IS_NPC(taker)) {
    taker->player.clss = giver->player.clss;
    if (!taker->skills)
      SpaceForSkills(taker);
    for (j = 0; j< MAX_SKILLS; j++) {
      taker->skills[j].learned = giver->skills[j].learned;
      taker->skills[j].recognise = giver->skills[j].recognise;
    }
    for (j = 0;j<=MAX_LEVEL_IND;j++) {
      taker->player.level[j] = giver->player.level[j];
    }
    UpdateMaxLevel(taker);
    UpdateMinLevel(taker);
  }

  /*
   * aliases
   */
  taker->aliases = giver->aliases;
  giver->aliases = 0;

 /* 
  * guild info
  */

  taker->player.guildinfo = giver->player.guildinfo;
  GET_HOME(taker) = GET_HOME(giver);
  
  GET_MANA(taker) = GET_MANA(giver);
  GET_MOVE(taker)=GET_MOVE(giver);
  GET_ALIGNMENT(taker) = GET_ALIGNMENT(giver);
  taker->specials.conditions[0]=giver->specials.conditions[0];
  taker->specials.conditions[1]=giver->specials.conditions[1];
  taker->specials.conditions[2]=giver->specials.conditions[2];

  if (!IS_NPC(taker))
    taker->specials.timer=0;
/* set the timer to 0 to avoid any autorent timer conflictions... */

  taker->specials.flags = giver->specials.flags;

  trans_affects(giver, taker);
}


void FailCharm(struct char_data *victim, struct char_data *ch)
{
  if (IS_NPC(victim)) {
    if (!victim->specials.fighting) {
      set_fighting(victim,ch);
    } 
  } else {
    send_to_char("You feel charmed, but the feeling fades.\n\r",victim);
  }
}

void FailSleep(struct char_data *victim, struct char_data *ch)
{
  
  send_to_char("You feel sleepy for a moment,but then you recover\n\r",victim);
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}


void FailPara(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel frozen for a moment,but then you recover\n\r",victim);
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}

void FailCalm(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel happy and easygoing, but the effect soon fades.\n\r",victim);
  if (IS_NPC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim,ch);
}

