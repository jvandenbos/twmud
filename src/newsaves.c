
#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "newsaves.h"
#include "utility.h"
#include "utils.h"
#include "spell_util.h"
#include "multiclass.h"

/* cant include math.h, so we have to declare exp here... */
#ifdef __cplusplus
extern "C"
#endif
    double exp(double);

#define SCALER			.055
#define EVEN_CHANCE		.50

/* positive extraMods makes the save easier, decreasing
   the chance of successful skill usage */

int NewSkillSave(struct char_data* ch, struct char_data* vic,
		 int skillNo, int extraMods, long immunes)
{
    int raw, chance, skill, diff, tries;
    double ePart;

//    if(vic)
//    if(IsImmune(vic, immunes))	/* if victim is immune don't bother */
//      return TRUE;		/* with anything more... */	
    
     if(!ch->skills || !(skill = skill_chance(ch, skillNo)))
      return TRUE;		/* if they don't have the skill yet */
				/* they automatically fail */
    if(vic)
      diff=GetMaxLevel(ch) - (GetMaxLevel(vic) + extraMods);
    
    ePart = ((double)(skill)/90.0) * EVEN_CHANCE *
	    exp(SCALER * (double)(diff));
	     
    raw = (int) (100. * ePart);
	
    chance = 100 - MAX(MIN(raw, 95), 6);

    tries = 1;
    if(immunes && IsImmune(vic, immunes)) {
      chance = MIN(chance * 2, 95);
      tries += 2;
    } else if(immunes && IsResist(vic, immunes)) {
      tries++;
    }
    if(immunes && IsSusc(vic, immunes))
      tries--;
    
    while(tries-- > 0) {
      if (number(1,100) < chance)
	return TRUE;
    }
    
    return FALSE;
}

