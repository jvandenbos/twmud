#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "spell_procs.h"
#include "utility.h"
#include "util_num.h"
#include "multiclass.h"
#include "act.h"
#include "spec.h"

void breath_weapon(struct char_data *ch, int mana_cost, breath_func func)
{
    struct char_data *tmp;

    act("You rear back and inhale...", TRUE,ch,0,0,TO_CHAR);
    act("$n rears back and inhales...",TRUE,ch,0,0,TO_ROOM);

    if (func) {
        /* give the suckers a chance to get away */
        for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
           if (tmp!=ch && !IS_IMMORTAL(tmp) && AWAKE(tmp))
               do_flee(tmp, "", 0);
        }

        act("You breath...", TRUE,ch,0,0,TO_CHAR);
        act("$n breathes...",TRUE,ch,0,0,TO_ROOM);

        (func)(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
        GET_MANA(ch) -= mana_cost;
    } else {
        act("You breath... cough and sputter...", TRUE,ch,0,0,TO_CHAR);
        act("$n breathes... coughs and sputters...",TRUE,ch,0,0,TO_ROOM);
    }
}


SPECIAL(BreathWeapon)
{
    struct char_data *self = (struct char_data *) me;

    switch (type)
    {
    case SPEC_INIT:
	return (FALSE);

    case SPEC_FIGHT:
        do_breath(self, "", 0);
        return (TRUE);
    }

    return (FALSE);
}
