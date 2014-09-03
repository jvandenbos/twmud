
#include "config.h"
#include <stdio.h>
#include <assert.h>

#if USE_stdlib
#include <stdlib.h>
#endif
    
#include "structs.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "race.h"
#include "constants.h"
#include "multiclass.h"
#include "utility.h"
#include "db.h"
#include "fight.h"
#include "handler.h"
#include "act.h"
#include "recept.h"
#include "spec.h"
    

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..100 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
    if (age < 15)
	return(p0);					/* < 15   */	
    else if (age <= 29) 
	return (int) (p1+(((age-15)*(p2-p1))/15));	/* 15..29 */
    else if (age <= 44)
	return (int) (p2+(((age-30)*(p3-p2))/15));	/* 30..44 */
    else if (age <= 59)
	return (int) (p3+(((age-45)*(p4-p3))/15));	/* 45..59 */
    else if (age <= 100)
	return (int) (p4+(((age-60)*(p5-p4))/20));	/* 60..100 */
    else
	return(p6);					/* >= 100 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
    int max;
  
    max = 100;

#ifdef JANWORK
    if (IS_PC(ch) && IS_SET(ch->specials.mob_act, ACT_POLYSELF) &&
        !IS_SET(ch->specials.mob_act, ACT_SHIFTER))
        return(max);
#endif

    if (HasClass(ch, CLASS_MAGIC_USER)) {
	max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 10;
    } else if (HasClass(ch, CLASS_CLERIC)) {
	max += GET_LEVEL(ch, CLERIC_LEVEL_IND) * 8;
    } else if (HasClass(ch, CLASS_PSI)) {
	max += GET_LEVEL(ch, PSI_LEVEL_IND) * 8;
    } else if (HasClass(ch, CLASS_BARD)) {
        max += GET_LEVEL(ch, BARD_LEVEL_IND) * 7;
    } else if (HasClass(ch, CLASS_DRUID)) {
	max += GET_LEVEL(ch, DRUID_LEVEL_IND) * 6;
    } else if (HasClass(ch, CLASS_PALADIN)) {
	max += GET_LEVEL(ch, PALADIN_LEVEL_IND) * 5;
    } else if (HasClass(ch, CLASS_RANGER)) {
	max += GET_LEVEL(ch, RANGER_LEVEL_IND) * 4;
    } else if (HasClass(ch, CLASS_SHIFTER)) {
        max += GET_LEVEL(ch, SHIFTER_LEVEL_IND) * 6;
    }  else if (HasClass(ch, CLASS_MONK)) {
        max += GET_LEVEL(ch, MONK_LEVEL_IND) * 7;
    } else { 
	    max = 100;
    }

    max += ch->points.max_mana;	/* bonus mana */
  
    return(max);
}


int hit_limit(struct char_data *ch)
{
    int max;
  
    if (!IS_NPC(ch) && !IS_SET(ch->specials.mob_act, ACT_SHIFTER))
	max = (ch->points.max_hit) +
	    (graf(age(ch).year, 2,4,17,14,8,4,3));
    else 
	max = (ch->points.max_hit);
  
  
    /* Class/Level calculations */
  
    /* Skill/Spell calculations */
  
    return (max);
}


int move_limit(struct char_data *ch)
{
    int max;
  
    if (IS_PC(ch))
	max = 100 + age(ch).year + GetTotLevel(ch);
    else
	max = ch->points.max_move;

    if (IsRideable(ch))
	max *= 2;
  
    if (GET_RACE(ch) == RACE_DWARF)
	max -= 30;
    else if (GET_RACE(ch) == RACE_ELF)
	max += 25;

    max += ch->points.max_move;	/* move bonus */
  
    return (max);
}

int IsRideable( struct char_data *ch)
{
    if (IS_SET(ch->specials.mob_act, ACT_STEED))
      return (TRUE);

    return (FALSE);
}

