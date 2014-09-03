#include "config.h"
#include <stdio.h>
#include "engine.h"
#include "spells.h"
#include "smart_mobs.h"
#include "multiclass.h"
#include "spec.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "find.h"
#include "util_str.h"
#include "utility.h"
#include "act.h"
#include "handler.h"
#include "periodic.h"
#include "vnum_mob.h"
#include "proto.h"
#include "string.h"


SPECIAL(astral_pool)
{
    int x, index, exit, old_room;
    char buf[256];
    struct char_data *pool = (struct char_data *) me;

    /* here are the destinations for the pools. you can have up to 10
       exits for each pool, and you must terminate the set with a 0.
       when a pc enters a pool, an exit is choosen at random from the
       set and that is where the pc will come out. */

   static int pool_exits[][11]=
   {
      {21838, 21817, 21820, 21837, 21809, 0}, /* 7600 sauria */
      {25079, 25078, 25094, 25033, 25049, 0}, /* 7601 abyss */
      {1562, 1568, 1570, 1575, 1561, 0},      /* 7602 tower sorcery */
      {23140, 23135, 23136, 23130, 23126, 0}, /* 7603 cthulhu */
      {13776, 13759, 13760, 13757, 13762, 0}, /* 7604 hades */
      {539, 543, 573, 592, 536, 0},           /* 7615 troy */
      {2704, 2722, 2709, 2729, 2785, 0},      /* 7616 tyrsis */
      {4202, 4205, 4206, 4214, 4246, 0},      /* 7617 monguls */
      {5129, 5127, 5121, 5122, 5100, 0},      /* 7618 drow city */
      {6615, 6610, 6607, 6603, 6604, 0},      /* 7619 camelot */
      {9201, 9216, 9215, 9231, 9254, 0},      /* 7620 hill giants */
      {10052, 10080, 10076, 10053, 10055, 0}, /* 7621 galaxy */
      {16204, 16205, 16206, 16210, 16219, 0}, /* 7622 undercaves I */
      {19201, 19204, 19220, 19221, 19214, 0}, /* 7623 mistamere */
      {21039, 21020, 21021, 21028, 21038, 0}, /* 7624 orshingal */
   };

   if (type == SPEC_INIT)
       return (FALSE);
   
   if (cmd!=7 && cmd!=15 && cmd!=166)
       return (FALSE);
    one_argument(arg, buf);
    if (!*buf)
	return (FALSE);
    if (pool != get_char_room_vis(ch, buf))
	return (FALSE);
    index=mob_index[pool->nr].virt-VMOB_FIRST_POOL;
    x=0;
    while (pool_exits[index][x])
	x++;
    exit=pool_exits[index][number(0,x-1)];
    if (!real_roomp(exit))
    {
	sprintf (buf, "illegal room in color pools at index %d, room %d.",
		 index, exit);
	log_msg(buf);
	send_to_char("This color pool is experiencing technical "
		     "difficulties.\n\r", ch);
	return TRUE;
    }
    if (cmd==7)
    {
	act("$n slowly wades into the pool and is enveloped by the lights.",
	    0, ch, 0, 0, TO_ROOM);
	send_to_char("You slowly wade into the shimmering lights of "
		     "the pool...\n\r", ch);
	send_to_char("A floating sensation overcomes you as the lights grow "
		     "intense all around....\n\r",ch);
	send_to_char("The feeling fades as the lights dissipate and you "
		     "arrive...\n\r\n\r",ch);
    } else
    {
	send_to_char("As you gaze into the pool, you become mesmerized by the "
		     "swirling lights.\n\r",ch);
	send_to_char("An image begins to emerge as you look more "
		     "closely...\n\r\n\r", ch);
	act("$n gazes into the swirling lights and color of the pool.",
	    0, ch, 0, 0, TO_ROOM);
    }
    old_room=ch->in_room;
    if (cmd==7)
    {
	char_from_room(ch);
	char_to_room(ch, exit);
	do_look(ch, "", 0);
    } else
	send_to_char(real_roomp(exit)->description, ch);
    return (TRUE);
}


/* Smart mobs for astral plane */

SPECIAL(githyanki_knight)
{
    struct char_data *knight = (struct char_data *) me;

    static int fight[][MAX_SET_SIZE] = { 
	{SPELL_DISPEL_MAGIC, SKILL_BERSERK, 0},
	{SKILL_BASH, SKILL_KICK, SKILL_DISARM, SKILL_DODGE, 0},
	{SKILL_THRUST, SKILL_STUN, SPELL_SANCTUARY, SPELL_HEAL, 0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_SANCTUARY, 0},
	{SPELL_HEAL, 0},
	{0},
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(knight, 300, 300);
	init_skills(knight, CLERIC_LEVEL_IND);
	init_skills(knight, RANGER_LEVEL_IND);
	init_skills(knight, WARRIOR_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(knight, fight, peace, cmd));
    }

    return (FALSE);
}


SPECIAL(astral_cleric)
{
    struct char_data *cleric = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = { 
	{SPELL_DISPEL_MAGIC, SPELL_EARTHQUAKE, 0},
	{SPELL_ANIMATE_DEAD, SPELL_HEAL, SPELL_SANCTUARY, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_SANCTUARY, 0},
	{SPELL_HEAL, SPELL_ANIMATE_DEAD, 0},
	{0},
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(cleric, 200, 200);
	init_skills(cleric, CLERIC_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(cleric, fight, peace, cmd));
    }

    return (FALSE);
}


SPECIAL(astral_psionist)
{
    struct char_data *psi = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = {
	{SKILL_PSIONIC_BLAST, SKILL_PHANTASMAL, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SKILL_SPELL_SHIELD, SKILL_INVIS, SKILL_ILLUSIONARY_SHROUD, 0},
	{SPELL_SHIELD, SPELL_HEAL, SPELL_SANCTUARY, 0},
	{0},
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(psi, 300, 300);
	init_skills(psi, PSI_LEVEL_IND);
	init_skills(psi, CLERIC_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(psi, fight, peace, cmd));
    }

    return (FALSE);
}


SPECIAL(githyanki_mage)
{
    struct char_data *mage = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = { 
	{SPELL_ICE_STORM, SPELL_EARTHQUAKE, 0},
	{SPELL_DISPEL_MAGIC, SPELL_WEB, 0},
	{SPELL_CHILL_TOUCH, SPELL_DISINTEGRATE, SPELL_ACID_BLAST, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_SANCTUARY, SPELL_FIRESHIELD, 0},
	{SPELL_HEAL, SPELL_SHIELD, SPELL_ARMOR, 0},
	{0},
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(mage, 300, 300);
	init_skills(mage, CLERIC_LEVEL_IND);
	init_skills(mage, MAGE_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(mage, fight, peace, cmd));
    }

    return (FALSE);
}


SPECIAL(astral_beholder)
{
    struct char_data *beholder = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = {
	{SPELL_DISPEL_MAGIC, SKILL_STUN, SKILL_BASH, SKILL_BERSERK,
	 SKILL_DISARM, SKILL_DODGE, SKILL_SPELL_SHIELD, 0},
	{SKILL_PSIONIC_BLAST, SPELL_ENERGY_DRAIN, SPELL_DISINTEGRATE, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_SANCTUARY, SPELL_FIRESHIELD, SPELL_INVISIBLE, 0},
	{SPELL_HEAL, SPELL_SHIELD, SPELL_ARMOR, 0},
	{0},
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(beholder, 500, 500);
	init_skills(beholder, MAGE_LEVEL_IND);
	init_skills(beholder, WARRIOR_LEVEL_IND);
	init_skills(beholder, PSI_LEVEL_IND);
	init_skills(beholder, CLERIC_LEVEL_IND);
	init_skills(beholder, RANGER_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(beholder, fight, peace, cmd));
    }

    return (FALSE);
}


/*
   defining thieves is pointless since they have no offensive spells/skills
   unless they are agg/turbo and use backstab (which guildmasters are not
   of course). however, we'll give them disarm for now.
*/

SPECIAL(astral_spy)
{
    struct char_data *spy = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = {
	{SKILL_DISARM, SKILL_BACKSTAB, SKILL_DODGE, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SKILL_HIDE, SKILL_SNEAK, 0},
	{0}
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(spy, 200, 200);
	init_skills(spy, THIEF_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(spy, fight, peace, cmd));
    }

    return (FALSE);
}




SPECIAL(astral_ranger)
{
    struct char_data *ranger = (struct char_data *) me;
    static int fight[][MAX_SET_SIZE] = {
	{SKILL_STUN, SKILL_BERSERK, SPELL_DISPEL_MAGIC, SKILL_DODGE, 0}, 
	{SKILL_THRUST, SKILL_KICK, SKILL_DISARM, SPELL_HEAL, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_HEAL, 0},
	{SPELL_SANCTUARY, SPELL_ARMOR, 0},
	{0}
    };

    switch (type)
    {
    case SPEC_INIT:
	init_atts(ranger, 400, 400);
	init_skills(ranger, RANGER_LEVEL_IND);
	init_skills(ranger, CLERIC_LEVEL_IND);
	init_skills(ranger, WARRIOR_LEVEL_IND);
	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(ranger, fight, peace, cmd));
    }

    return (FALSE);
}


	       /* Getting to astral plane */
#define BERRY_CHAR \
"\n\rMy, that berry had an unusual taste!\n\r"				\
"...UHO..I dont feel so good.\n\r"					\
"AARRRGGHHH!  You are torn out of reality as mortals know it.\n\r"	\
"You dift aimlessly in nothingness, for what seems like days.\n\r"	\
"When reality comes into focus...you are elsewhere?!?\n\r"

#define BERRY_ROOM \
"\n\rA strange fuzziness forms around $n!\n\r"				\
"A tear in the fabric of reality appears above you...\n\r"		\
"$n is sucked up into the tear!\n\r"					\
"The tear disappears as quickly as it formed.\n\r"			\
"$n IS GONE!!!\n\r"

#define BERRY_SHOUT \
"AARRRGGGGHHH, I'm being sucked in...."

SPECIAL(astral_berry)
{
    char check[MAX_INPUT_LENGTH], *tmp;

    if (type == SPEC_CMD && cmd == 12)
    {
	tmp = one_argument(arg, check);
	if (*check) 
	{
	    if (!strcmp(check, "berry"))
	    {
		if(GET_POS(ch) == POSITION_FIGHTING)
		{
		    send_to_char("You're much to busy for that.\n", ch);
		    return 1;
		}
		do_eat(ch, arg, 12);
                send_to_char(BERRY_CHAR, ch);
		do_shout(ch, BERRY_SHOUT, 0);
		act(BERRY_ROOM, TRUE, ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, 7601);
		do_look(ch, "", 15);
		return(1);
	    }
	}
    }
    
    return(0);
}


SPECIAL(mana_regen)
{
    struct char_data *self = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);
  
    if (cmd) return(FALSE);

    if (GET_MANA(self) < GET_MAX_MANA(self)) {
        GET_MANA(self) = MIN(GET_MANA(self)+50, GET_MAX_MANA(self));
        send_to_char("You regain mana!!\n\r", self);
	act("$n rejuvenates!", TRUE, self, 0, 0, TO_ROOM);
	return(TRUE);
    }
    return FALSE;
}


void destroy_gate(struct char_data *gate)
{
    if (gate->other_gate)
    {
	act("The gateway flares suddenly and then collapses in on itself!",
	    FALSE, gate->other_gate, 0, 0, TO_ROOM);
	extract_char(gate->other_gate);
    }
    act("The gateway flares suddenly and then collapses in on itself!",
	FALSE, gate, 0, 0, TO_ROOM);
    extract_char(gate);
}


SPECIAL(gate_proc)
{
    int old_room, tar_room;
    char buf[256];
    struct char_data *gate = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);
  
    if (!cmd) {
        if(GET_EXP(gate) > 0)
	  gate->act_ptr++;
       
	if (gate->act_ptr>10 || !gate->other_gate || gate->specials.fighting)
	    destroy_gate(gate);
	return (TRUE);
    }

    if (cmd!=7 && cmd!=15 && cmd!=166)
	return (FALSE);

    one_argument(arg, buf);
    if (!*buf)
	return (FALSE);

    if (gate != get_char_room_vis(ch, buf))
	return (FALSE);

    if(mob_index[gate->nr].func!=gate_proc)
	return (FALSE);

    old_room = ch->in_room;
    tar_room = 0;

    if (gate->other_gate) {
	if (real_roomp(gate->other_gate->in_room))
	    tar_room=gate->other_gate->in_room;
    }

    if (cmd==15 || cmd==166) {
	act("You shelter your eyes and peer into the gateway...",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n shelters $s eyes and peers into the gateway.",
	    TRUE, ch, 0, 0, TO_ROOM);
	send_to_char(real_roomp(tar_room)->description, ch);
	list_char_in_room(real_roomp(tar_room)->people, ch);
    }
    else if(ch->specials.fighting)
    {
	send_to_char("You're a little busy right now\n\r", ch);
	return TRUE;
    } else {
	act("You feel a violent lurch as you step into the gateway...",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("There is a surge of energy as $n steps into the gateway...",
	    TRUE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, tar_room);
	act("You stagger forward as you suddenly arrive at the other end...",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("There is a surge of energy from the gateway as $n suddenly "
	    "appears...", TRUE, ch, 0, 0, TO_ROOM);
	if (GET_EXP(gate)==SKILL_LESSER_GATE)
	    destroy_gate(gate);
	do_look(ch, "", 0);
    }
    
    return (TRUE);
}
