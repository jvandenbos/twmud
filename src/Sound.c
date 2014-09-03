
/*
 **  create sounds on objects
 */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "utility.h"

void MakeNoise(int room, char *local_snd, char *distant_snd)
{
    int door;
    struct char_data *ch;
    struct room_data *rp, *orp;
    
    rp = real_roomp(room);
    
    if (rp) {
	for (ch = rp->people; ch; ch = ch->next_in_room) {
	    if (!IS_NPC(ch) &&
		!IS_AFFECTED(ch, AFF_SILENCE) &&
		(GET_POS(ch) > POSITION_SLEEPING)) {
		send_to_char(local_snd, ch);
	    }
	}
	for (door = 0; door <= 5; door++) {
	    if (rp->dir_option[door] &&
		(orp = real_roomp(rp->dir_option[door]->to_room)) ) {
		for (ch = orp->people; ch; ch = ch->next_in_room) {
		    if (IS_PC(ch) && !IS_AFFECTED(ch, AFF_SILENCE) &&
			(GET_POS(ch) > POSITION_SLEEPING)) {
			send_to_char(distant_snd, ch);
		    }
		}
	    }
	}
    }
}
