#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "handler.h"
#include "db.h"
#include "race.h"
#include "spells.h"
#include "spell_util.h"
#include "multiclass.h"
#include "util_num.h"
#include "utility.h"
#include "fight.h"
#include "limits.h"
#include "comm.h"
#include "periodic.h"

// For tweaking mana, hit and move gains for druids in the wilderness
// Defines how much ticks before fully healed - Raist
#define DRUID_GAIN_SPEED 4

extern int DamageTrivia(struct char_data *ch, struct char_data *vict, int dam, int type);

void gain_condition(struct char_data *ch,int condition,int value);

void give_regens( int pulse )
{
    register struct char_data *i;
    struct room_data *rp;

    /* first, give everybody their regens... */
    EACH_CHARACTER(m_iter, i)
    {
        if (i->master) {
            if (real_roomp(i->in_room)->zone !=
                real_roomp((i->master)->in_room)->zone)
                stop_follower(i);
        }

	if (GET_POS(i) >= POSITION_STUNNED)
	{
	    GET_HIT(i) += hit_gain(i);
	    if(GET_HIT(i) > hit_limit(i))
		GET_HIT(i) = hit_limit(i);
	    GET_HIT(i) = MAX(GET_HIT(i), 1);

	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	    if (GET_POS(i) == POSITION_STUNNED)
		update_pos( i );
	}
	else if (GET_POS(i) == POSITION_INCAP)
	{
	    damage(i, i, 0, TYPE_SUFFERING);
	}
	else if (IS_PC(i) && (GET_POS(i) == POSITION_MORTALLYW))
	{
	    damage(i, i, 1, TYPE_SUFFERING);
	}
	if(IS_PC(i))
	{
	    update_char_objects(i);
	    if (TRUST(i) < TRUST_DEMIGOD)
		check_idling(i);
	}
	if (IS_PC(i))
	{
	    rp = real_roomp(i->in_room);
	    if (rp)
	    {
		if (rp->sector_type == SECT_WATER_SWIM ||
		    rp->sector_type == SECT_WATER_NOSWIM)
		{
		    gain_condition(i,FULL,-1);
		    gain_condition(i,DRUNK,-1);
		}
		else if (rp->sector_type == SECT_DESERT)
		{
		    gain_condition(i,FULL,-1);
		    gain_condition(i,DRUNK,-2);
		    gain_condition(i,THIRST,-2);
		}
		else if (rp->sector_type == SECT_MOUNTAIN ||
			 rp->sector_type == SECT_HILLS)
		{
		    gain_condition(i,FULL,-2);
		    gain_condition(i,DRUNK,-2);
		}
		else
		{
		    gain_condition(i,FULL,-1);
		    gain_condition(i,DRUNK,-1);
		    gain_condition(i,THIRST,-1);
		}
	    }
	}
    }
    END_AITER(m_iter);
}

/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
    int gain;
    struct room_data *rp;

    if(IS_NPC(ch) && !IS_SET(ch->specials.mob_act, ACT_SHIFTER) && !IS_SET(ch->specials.mob_act, ACT_POLYSELF))
        gain = MAX(5, (int)(GetMaxLevel(ch) * .60));
    else
	gain = MAX(5,(int)(age(ch).year * .60));


	/* Class calculations */

	/* Skill/Spell calculations */

	/* Position calculations    */
	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
	    gain += gain;
	    break;
	case POSITION_RESTING:
	    gain+= (gain>>1);	/* Divide by 2 */
	    break;
	case POSITION_SITTING:
	    gain += (gain>>2);	/* Divide by 4 */
	    break;
        case POSITION_STANDING:
            gain += (gain>>6);  /* Divide by 12 */

	}

	if (HasClass(ch, CLASS_MAGIC_USER) ||
	    HasClass(ch, CLASS_CLERIC)     ||
	    HasClass(ch, CLASS_DRUID)      ||
            HasClass(ch, CLASS_SHIFTER)    ||
	    HasClass(ch, CLASS_PSI)        ||
	    HasClass(ch, CLASS_PALADIN))
	    gain += gain;


    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;

    if((GET_COND(ch,FULL)==0) ||
       (GET_COND(ch,THIRST)==0) ||
       (IS_SET(ch->specials.mob_act,ACT_IT) && IS_PC(ch)))
	gain >>= 2;

    rp=real_roomp(ch->in_room);
    if(rp->sector_type==SECT_FIELD || rp->sector_type==SECT_FOREST ||
       rp->sector_type==SECT_HILLS || rp->sector_type==SECT_MOUNTAIN) {
	if(HasClass(ch,CLASS_DRUID))
	    gain = GET_MAX_MANA(ch) / DRUID_GAIN_SPEED;
	if(HasClass(ch,CLASS_RANGER))
	    gain+=(gain>>3);
    }

    if (time_info.month == 15)
	gain += gain;	/* if it is the month of the Renegade they gain it
			back no matter what!                            */

    if((HasClass(ch,CLASS_PSI)) || (HasClass(ch,CLASS_MONK))) {
	gain+=CheckMeditating(ch);
    }

    /*if ((HasClass(ch,CLASS_CLERIC)) || (HasClass(ch, CLASS_MAGIC_USER))
        || (HasClass(ch, CLASS_PALADIN)) || (HasClass(ch, CLASS_BARD)) ) {
	    gain+= gain;
    }*/
    return (gain);
}


int hit_gain(struct char_data *ch)
     /* Hitpoint gain pr. game hour */
{
    int gain;
    int sender=0;
    struct affected_type *aff;
    char buf[512];

    if(IS_NPC(ch) && !IS_SET(ch->specials.mob_act, ACT_SHIFTER)) {
	/* based on mob levels now bh */
	if (IS_AFFECTED(ch, AFF_CHARM))
	    gain = number(3,6);	/* mobs dont super heal while charmed */
	else if (IS_SET(ch->specials.mob_act, ACT_POLYSELF))
	    gain = number(3,6);
	else if (GetMaxLevel(ch) < 10)
	    gain = number(3,6);
	else if (GetMaxLevel(ch) < 20)
	    gain = number(5,15);
	else if (GetMaxLevel(ch) < 30)
	    gain = number(7,25);
	else if (GetMaxLevel(ch) < 40)
	    gain = number(10,40);
	/* not like there are more than 50 levels, but just in case */
	else
	    gain = number(15,65);
    } else {

	if (GET_POS(ch) == POSITION_FIGHTING) {
	    gain = 0;
	} else {
	    gain = graf(age(ch).year, 2,5,10,18,6,4,2);
	}

	/* Class/Level calculations */

	/* Skill/Spell calculations */

	/* Position calculations    */

	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
	    gain += gain>>1;
	    break;
	case POSITION_RESTING:
	    gain+= gain>>2;
	    break;
	case POSITION_SITTING:
	    gain += gain>>3;
	    break;
	}

    }

    if (GET_RACE(ch) == RACE_DWARF)
	gain += 3;

    if((GET_COND(ch,FULL)==0) ||
       (GET_COND(ch,THIRST)==0) ||
       (IS_SET(ch->specials.mob_act,ACT_IT) && !IS_PC(ch)))
	gain >>= 2;

    if (wilderness(ch)) {
	if(HasClass(ch,CLASS_DRUID)) {
	  gain = GET_MAX_HIT(ch) / DRUID_GAIN_SPEED;
	  sender=1;
	}
	if(HasClass(ch,CLASS_RANGER)) {
            gain+=(gain>>3);
	    sender=1;
	}
	if(sender==1)
            send_to_char("Being in your natural environment causes you to feel stronger.\n\r", ch);
    }

    if(time_info.month == 15)
	gain+= gain>>1;
    /* the month of the renegade, they gain more no matter what */
    if( (HasClass(ch,CLASS_PSI)) || (HasClass(ch,CLASS_MONK)) )
	gain += CheckMeditating(ch);

    if(IS_AFFECTED(ch, AFF_REGENERATE))
        gain *= 2;

    if(IS_AFFECTED(ch, AFF_POISON)) {
    }

    return (gain);
}



int move_gain(struct char_data *ch)
     /* move gain pr. game hour */
{
    int gain;

    if(IS_NPC(ch) && !IS_PC(ch)) {
	if(IsRideable(ch))
	    gain=50;
	else
	    gain=25;
	/* Neat and fast */
    } else {
	if (GET_POS(ch) != POSITION_FIGHTING)
	    gain = 5 + GET_CON(ch);
	else
	    gain = 0;

	/* Position calculations    */
	switch (GET_POS(ch)) {
	case POSITION_SLEEPING:
	    gain += (gain>>1);	/* Divide by 2 */
	    break;
	case POSITION_RESTING:
	    gain+= (gain>>2);	/* Divide by 4 */
	    break;
	case POSITION_SITTING:
	    gain += (gain>>3);	/* Divide by 8 */
	    break;
	}
    }


    if (GET_RACE(ch) == RACE_DWARF)
	gain += 8;

    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;

    if((GET_COND(ch,FULL)==0) ||
       (GET_COND(ch,THIRST)==0) ||
       (IS_SET(ch->specials.mob_act,ACT_IT) && !IS_PC(ch)))
	gain >>= 2;

    if(OUTSIDE(ch)) {
      if(HasClass(ch,CLASS_DRUID)) {
	gain = GET_MAX_MOVE(ch) / DRUID_GAIN_SPEED;
      }
	if(HasClass(ch,CLASS_RANGER))
	    gain+=(gain>>3);
    }

    if(time_info.month == 15)
	gain += (gain>>1);
    /* the month of the Renegade they gain more no matter what */
    if( (HasClass(ch,CLASS_PSI)) || (HasClass(ch,CLASS_MONK)) )
	gain += CheckMeditating(ch);

    return (gain);
}

void gain_condition(struct char_data *ch,int condition,int value)
{
    bool intoxicated;

    if(GET_COND(ch, condition)==-1) /* No change */
	return;

    intoxicated=(GET_COND(ch, DRUNK) > 0);

    GET_COND(ch, condition)  += value;

    GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
    GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));

    if(GET_COND(ch,condition))
	return;

    switch(condition){
    case FULL :
	{
	    send_to_char("You are hungry.\n\r",ch);
	    return;
	}
    case THIRST :
	{
	    send_to_char("You are thirsty.\n\r",ch);
	    return;
	}
    case DRUNK :
	{
	    if(intoxicated)
		send_to_char("You are now sober.\n\r",ch);
	    return;
	}
	default : break;
    }

}

void drain_life_pulse(int pulse) {
   struct char_data *ch;
   struct affected_type *aff;
   int dam;

   EACH_CHARACTER(m_iter, ch) {
      if(IS_AFFECTED(ch, AFF_POISON)) {
	 if(!IS_FIGHTING(ch) && (pulse%5)) continue;

	 for(aff = ch->affected; aff; aff = aff->next) {
	    if((aff->type == SPELL_POISON) && (aff->location == APPLY_DRAIN_LIFE)) {
	       dam = DamageTrivia(ch,ch,aff->modifier, SPELL_POISON);
	       if(dam) {
		  cprintf(ch, "$CrPoison courses through your veins!$CN\n\r");
		  GET_HIT(ch) = MAX(GET_HIT(ch)-dam, 1);
	       }
	    }
	 }
      }
   } END_AITER(m_iter);
}
