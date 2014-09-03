#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>

#include "structs.h"
#include "fight.h"
#include "utils.h"
#include "spells.h"
#include "engine.h"
#include "utility.h"
#include "act.h"
#include "util_num.h"
#include "skills.h"

/*------------------------- SUPPORT ROUTINES  ------------------------*/
 
/*** test to see if npc is able to perform an ability ***/
int can_perform (struct char_data *ch, int cmd)
{
   if (GET_POS(ch)<POSITION_RESTING || IS_AFFECTED(ch,AFF_PARALYSIS))
      return (FALSE);
   return (TRUE);
}

/*** test if npc needs to stand up ***/
int need_to_stand (struct char_data *ch)
{
   if (GET_POS(ch)<POSITION_FIGHTING) {
       StandUp(ch);
       return (TRUE); 
   }
   return (FALSE);
}

/*** test to see if npc needs to rest up for mana ***/
int need_mana_rest (struct char_data *ch)
{
   int div=1;

   if(ch->hunt_info)
      div=2;

   if (GET_MANA(ch)<GET_MAX_MANA(ch)/div)
      return (TRUE);
   return (FALSE);
}

/*** test to see if npc needs to rest up for hp ***/
int need_hp_rest (struct char_data *ch)
{
   int div=1;

   if(ch->hunt_info)
      div=2;

   if (GET_HIT(ch)<GET_MAX_HIT(ch)/div)
      return (TRUE);
   return (FALSE);
}

/*** test to see if npc needs to rest up for move ***/
int need_move_rest (struct char_data *ch)
{
   int div=1;

   if (ch->hunt_info || !IS_SET(ch->specials.mob_act, ACT_SENTINEL))
      div=2;

   if (GET_MOVE(ch)<GET_MAX_MOVE(ch)/div)
      return (TRUE);
   return (FALSE);
}

/*** sit and rest, or meditate if npc is a psi and has that skill ***/
void need_to_rest (struct char_data *ch)
{
   if (!IS_AFFECTED(ch,AFF_MEDITATE) && GET_POS(ch)==POSITION_STANDING) {
      if ((ch->player.clss & CLASS_PSI) &&
	  ch->skills && ch->skills[SKILL_MEDITATE].learned)
	 do_meditation (ch, "", 0);
      else
	 do_rest(ch, "", 0);
   }
}

/*** test to see if ability is a spell or a skill ***/
int is_spell (struct spell_info *ability)
{
   if (IS_SET(ability->targets,TAR_SKILL))
      return (FALSE);
   return (TRUE);
}

/*** test to see if the npc has enough mana/move to perform the ability   ***/
int enough_mana_move (struct char_data *ch, struct spell_info *ability)
{
   if (ch->points.mana<ability->min_usesmana)
      return (FALSE);
   return (TRUE);
}

/*** test to see if ability can be done in the room the npc is in ***/
int can_do_ability_in_room (struct char_data *ch, struct spell_info *ability)
{
   if (is_spell(ability)) {
      if ((IS_SET(ability->targets,TAR_VIOLENT) && check_peaceful(ch,"")) ||
           check_soundproof(ch) || check_nomagic(ch))
	 return (FALSE);
      return (TRUE);     /*** right now all skills are permitted in sound ***/
   } else {              /*** proof rooms. sould be be changed eventually ***/ 
      if (IS_SET(ability->targets,TAR_VIOLENT) && check_peaceful(ch,""))
	 return (FALSE); 
      return (TRUE);
   }
}

/*** return a victim for the ability to be performed on if the ability ***/
/*** violent, the victim is whoever the npc is fighting or can fight,  ***/
/*** otherwise we assume it is a helpful ability and the npc wants to  ***/
/*** do it to itself ie: heal or restore sanctuary.                    ***/
struct char_data *get_a_victim (struct char_data *ch, struct spell_info *ability)
{
   if (IS_SET(ability->targets,TAR_VIOLENT) ||
       ability->number==SPELL_DISPEL_MAGIC)  /* assuming skills are   */
      return (FindVictim(ch));               /* defined in spell_list */
   return (ch);
}

/*** charges the npc the appropriate amount of mana ***/
void ability_cost (struct char_data *ch, struct spell_info *ability, int pass)
{
   if (is_spell(ability)) {
      if (pass)
         ch->points.mana -= ability->min_usesmana;
      else
         ch->points.mana -= ability->min_usesmana/2;
   }
}

/*** returns the lowest level value of the spell across all classes ***/ 
int bestlevel (int spellno)
{
   int x, lvl;
   struct spell_info *ability;

   ability=spell_by_number(spellno);

   lvl=ability->min_level[0];

   for (x=1; x<=MAX_LEVEL_IND; x++)
      lvl=MIN(ability->min_level[x], lvl);

   if (lvl <= ABS_MAX_LVL)
      return (lvl);
   return (0);
}

/*** selects the ability which received the highest rank. if there is ***/
/*** more than one skill with the highest rank, one of those is       ***/
/*** choosen based on the level of it.                                ***/
int best_ability (int size, int rank[MAX_SET_SIZE], int storage[MAX_SET_SIZE])
{
    int index, tindex, rtemp[MAX_SET_SIZE], stemp[MAX_SET_SIZE];
   
    rtemp[0] = rank[0];
    stemp[0] = storage[0];
    tindex = 1;
   
    for(index = 1 ; index <= size ; ++index) {
	if (rank[index]==rtemp[0]) {
            if (bestlevel(storage[index])>bestlevel(stemp[0])) {
		rtemp[0]=rank[index];
		stemp[0]=storage[index];
		tindex=1;
            } else if (bestlevel(storage[index])==bestlevel(stemp[0])) {
		rtemp[tindex]=rank[index];
		stemp[tindex]=storage[index];
		tindex++;
            }
	} else if(rank[index] > rtemp[0]) {
            rtemp[0]=rank[index];
	    stemp[0]=storage[index];
	    tindex=1;
	}
    }

    return (stemp[number(0,tindex-1)]);
}

int pass_concentration (struct char_data *ch, struct spell_info *ability)
{
   if (is_spell(ability))
      if (number(1,101) > ch->skills[ability->number].learned)
         return (FALSE);
   return (TRUE);
}

/*-------------------------- ENGINE ROUTINES --------------------------*/

/*** takes the selected ability set and determines which ability in the ***/
/*** set is the best in the current situation, checking for enough mana ***/
/*** and/or move at the same time.                                      ***/ 
int ability_routine (struct char_data *ch, int ability_set[MAX_SET_SIZE])
{
    int x, index, num, val, storage[MAX_SET_SIZE], rank[MAX_SET_SIZE];
    struct char_data  *vict;
    struct spell_info *ability;

    x=index=0;
    while (ability_set[x]) {
	num=ability_set[x];
	if(!(ability=spell_by_number(abs(num))))
	{
	    char buf[256];
	    sprintf(buf, "Illegal ability in smart mob: %d", num);
	    log_msg(buf);
	}
	else 
	  if ( (ch->skills[ability->number].learned) || /* know spell? */
	       (!IS_PC(ch)) ) /* All mobs know all skills/spells */
	  {
	      if (enough_mana_move(ch,ability) &&
		  can_do_ability_in_room(ch,ability)) {
		  vict=get_a_victim(ch, ability);
		  if (ability->logic_pointer && vict) {
		      if (num<0)
			  val=2;
		      else
			  val=(ability->logic_pointer)(ch, vict);
		      if (val>0) {
			  storage[index]=abs(num);
			  rank[index]=val;
			  index++;
		      }
		  }
	      }
	  }         
	x++;
    }

    if (index>0) {
	if (!need_to_stand(ch)) {
	    ability=spell_by_number(best_ability(index-1, rank, storage));
            vict=get_a_victim(ch, ability);
            if (vict) {
	    if (pass_concentration(ch, ability)) {
	          (ability->pre_call_pointer)(ch, vict);   
		  ability_cost(ch, ability, TRUE);
               } else
                  ability_cost(ch, ability, FALSE);
	    }
	}
	return (TRUE);
    }
    return (FALSE);
}

/*** goes through the sets of abilities, selecting until an ability has ***/
/*** been performed, or no abilities are left to select from            ***/
int ability_engine (struct char_data *ch, int abilities[][MAX_SET_SIZE])
{
   int x=0;
   int flag=FALSE;

   while (!flag && abilities[x][0]) {
      flag=ability_routine(ch, abilities[x]);
      x++;
   }
   return (flag);
}

/*** this routine makes sure the npc is able to perform its abilities   ***/
/*** (ie: not immobilized). tt also calls the ability_engine with the   ***/
/*** correct set of spells if the npc is fighting or not. and as a last ***/
/*** feature, it sets the npc resting if mana or hp are below max       ***/
int do_abilities (struct char_data *ch, int fight[][MAX_SET_SIZE],
		  int peace[][MAX_SET_SIZE], int cmd)
{
#ifndef NO_SMART_MOBS
   if(IS_AFFECTED(ch, AFF_CHARM))
       return (FALSE);
   
   if (!can_perform(ch, cmd))
      return (FALSE);

   if (FindVictim(ch)) {
      if (need_to_stand(ch))
	 return (TRUE); 
      return (ability_engine(ch, fight));
   }

   if(ch->hunt_info)
      return (FALSE);

   if (ability_engine(ch, peace))
      return (TRUE);
/*
   if (need_hp_rest(ch) || need_mana_rest(ch) || need_move_rest(ch)) {
      if (GET_POS(ch)!=POSITION_RESTING) {
        need_to_rest(ch);
        return (TRUE);
      }
      return (FALSE);
   }
*/
   return (need_to_stand(ch));
#else
   return (FALSE);
#endif   
}
