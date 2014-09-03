#include "config.h"

#include <stdio.h>

#include "spells.h"
#include "engine.h"
#include "utils.h"
#include "multiclass.h"
#include "db.h"

/*------------------------ CLASS INIT ROUTINE -------------------------*/

void init_skills(struct char_data *ch, int clss)
{
#ifndef NO_SMART_MOBS  
    struct spell_info *spell;
    int i;
    
    if(!(ch->player.clss & (1 << clss)))
    {
	ch->player.clss |= 1 << clss;
	ch->player.level[clss] = GetMaxLevel(ch);
	
	if(!ch->skills)
	    SpaceForSkills(ch);

	for(i = 0, spell = spell_list ; i <= spell_count ; i++, spell++)
	{
	    if(spell->name && GET_LEVEL(ch, clss) >= spell->min_level[clss])
	    {
		ch->skills[spell->number].learned =
		    IS_SET(spell->targets, TAR_SKILL) ? 95 : 90;
	    }
	}
    }
#endif    
}

/*-------------------------- MOB INIT ROUTINE -------------------------*/

void init_atts(struct char_data *ch, int mana, int move)
{
#ifndef  NO_SMART_MOBS
   ch->points.max_mana=mana;      /* note: these values are in addition to */
   ch->points.max_move=move;      /* the mana and move mobs get based on   */
   GET_MANA(ch)=GET_MAX_MANA(ch); /* level and class. */
   GET_MOVE(ch)=GET_MAX_MOVE(ch);
#endif   
}
