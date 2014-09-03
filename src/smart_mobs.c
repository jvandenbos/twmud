#include "config.h"

#include <ctype.h>

#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "engine.h"
#include "smart_mobs.h"
#include "spec.h"
#include "multiclass.h"

/*** smart procs for rivendell mobs ***/

SPECIAL(rivendell_guard)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BASH, SKILL_KICK, SKILL_HOLY_WARCRY, SKILL_DISARM, 0},
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_CRITIC,
       SPELL_CURE_SERIOUS, SKILL_LAY_ON_HANDS, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_SERIOUS,
       SPELL_CURE_CRITIC, SKILL_LAY_ON_HANDS, SPELL_TRUE_SIGHT, 0},
      {0}
   };
   struct char_data *guard = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(guard, 100, 100);
       init_skills(guard, PALADIN_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
       if(attack_evil(guard, 0, cmd, arg, type))
	   return (TRUE);
       /* fall through if no evil to attack */

   case SPEC_FIGHT:
       return (do_abilities(guard, fight, peace, cmd));
       break;
   }
   
   return (FALSE);
}


SPECIAL(rivendell_gandalf)
{
   static int fight[][MAX_SET_SIZE] = { 
      {SPELL_ICE_STORM, SPELL_CHAIN_ELECTROCUTION,
       SPELL_ACID_RAIN, SPELL_LAVA_STORM, 0},
      {SPELL_DISPEL_MAGIC, 0},
      {SPELL_FROST_CLOUD, SPELL_ACID_BLAST, SPELL_FIREBALL,
       SPELL_ELECTRIC_FIRE, SPELL_ENERGY_DRAIN, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_SANCTUARY, SPELL_FIRESHIELD, 0},
      {SPELL_HEAL, SPELL_TRUE_SIGHT, 0},
      {0},
   };
   struct char_data *gandalf = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gandalf, 200, 100);
       init_skills(gandalf, CLERIC_LEVEL_IND);
       init_skills(gandalf, MAGE_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gandalf, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(nightmare)
{
   static int fight[][MAX_SET_SIZE] = {
      {SPELL_CREEPING_DOOM, SPELL_LAVA_STORM, SPELL_ICE_STORM, 
       SPELL_ACID_RAIN, SPELL_POISON_GAS, SPELL_CHAIN_ELECTROCUTION, 0},
      {SPELL_DISPEL_MAGIC, SKILL_STUN , 0},
      {SPELL_FROST_CLOUD, SPELL_ACID_BLAST, SPELL_FIREBALL,
       SPELL_ELECTRIC_FIRE, SPELL_VAMPYRIC_TOUCH, SPELL_DISINTEGRATE,
       SKILL_PSIONIC_BLAST, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_SANCTUARY, SPELL_FIRESHIELD, SPELL_HEAL, 0},
      {SPELL_INVISIBLE, SPELL_TRUE_SIGHT, 0},
      {SKILL_HIDE, 0},
      {0},
   };
   struct char_data *mare = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(mare, 200, 200);
       init_skills(mare, DRUID_LEVEL_IND);
       init_skills(mare, MAGE_LEVEL_IND);
       init_skills(mare, PSI_LEVEL_IND);
       init_skills(mare, CLERIC_LEVEL_IND);
       init_skills(mare, RANGER_LEVEL_IND);
       init_skills(mare, THIEF_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(mare, fight, peace, cmd));
   }

   return (FALSE);
}

/*** smart procs for guildmasters ***/

SPECIAL(generic_guildmasters)
{
    int level_i;

    /* It uses mage skills for smart processes.  Too bad we don't have a
       "generic" smart mob without making it multi-class. */
  
    static int fight[][MAX_SET_SIZE] = { 
	{SPELL_ICE_STORM, SPELL_CHAIN_ELECTROCUTION, SPELL_ACID_RAIN,
	 SPELL_LAVA_STORM, 0},
	{SPELL_DISPEL_MAGIC, 0},
	{SPELL_ELECTRIC_FIRE, SPELL_ACID_BLAST, SPELL_FIREBALL,
	 SPELL_FROST_CLOUD, 0},
	{SPELL_CHILL_TOUCH, SPELL_BURNING_HANDS, SPELL_SHOCKING_GRASP,
	 SPELL_ENERGY_DRAIN, SPELL_ELECTROCUTE, SPELL_FIRE_WIND,
	 SPELL_FLAMESTRIKE, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_FIRESHIELD, 0},
	{0}
    };
    struct char_data *gm = (struct char_data *) me;

    switch (type)
    {
    case SPEC_INIT:
	init_atts(gm, GetMaxLevel(gm)*10, 100);
	init_skills(gm, MAGE_LEVEL_IND);
	return (TRUE);
	
    case SPEC_CMD:
	switch(cmd)
	{
	    
	case 164: case 170: case 243:
	    for ( ; isspace(*arg); arg++);
	    
	    if (!arg)
	    {
		send_to_char("Which class? Try <m|c|w|t|k|d|s|r|h>\n\r",ch);
		return TRUE;
	    }
	    
	    switch(*arg)
	    {
	    case 'M': case 'm':
		level_i = MAGE_LEVEL_IND;
		break;
		
	    case 'C': case 'c':
		level_i = CLERIC_LEVEL_IND;
		break;
		
	    case 'W': case 'w':
		level_i = WARRIOR_LEVEL_IND;
		break;
		
	    case 'T': case 't':
		level_i = THIEF_LEVEL_IND;
		break;
		
	    case 'K': case 'k':
		level_i = PALADIN_LEVEL_IND;
		break;
		
	    case 'D': case 'd':
		level_i = DRUID_LEVEL_IND;
		break;
		
	    case 'S': case 's':
		level_i = PSI_LEVEL_IND;
		break;
		
	    case 'R': case 'r':
		level_i = RANGER_LEVEL_IND;
		break;
		
	    case 'H': case 'h':
		level_i = SHIFTER_LEVEL_IND;
		break;
		
	    case 'O': case 'o':
		level_i = MONK_LEVEL_IND;
		break;
		
	    case 'B': case 'b':
		level_i = BARD_LEVEL_IND;
		break;
		
	    default:
		send_to_char("Which class? Try <m|c|w|t|k|d|s|r|h|o|b>\n\r",
			     ch);
		return TRUE;
	    }
	    break;
	    
	default:
	    return FALSE;
	}

	if (GuildMaster(ch, cmd, arg+1, level_i, gm))
	    return TRUE;
	else
	    return FALSE;

    case SPEC_IDLE:
    case SPEC_FIGHT:
	return (do_abilities(gm, fight, peace, cmd));
    }

    return FALSE;
}

SPECIAL(mage_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = { 
      {SPELL_ICE_STORM, SPELL_CHAIN_ELECTROCUTION, SPELL_ACID_RAIN,
       SPELL_LAVA_STORM, 0},
      {SPELL_DISPEL_MAGIC, 0},
      {SPELL_ELECTRIC_FIRE, SPELL_ACID_BLAST, SPELL_FIREBALL,
       SPELL_FROST_CLOUD, 0},
      {SPELL_CHILL_TOUCH, SPELL_BURNING_HANDS, SPELL_SHOCKING_GRASP,
       SPELL_ENERGY_DRAIN, SPELL_ELECTROCUTE, SPELL_FIRE_WIND,
       SPELL_FLAMESTRIKE, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_FIRESHIELD, 0},

      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*10, 100);
       init_skills(gm, MAGE_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, MAGE_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}

SPECIAL(cleric_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = { 
      {SPELL_POISON_GAS, 0},
      {SPELL_DISPEL_MAGIC, 0},
      {SPELL_HARMFUL_TOUCH, SPELL_WITHER, SPELL_RUPTURE, SPELL_IMPLODE,
       SPELL_DISINTEGRATE, 0}, 
      {SPELL_PARALYSIS, SPELL_WEAKNESS, SPELL_BLINDNESS, SPELL_POISON,
       SPELL_CURSE, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_SANCTUARY, 0},
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_SERIOUS,
       SPELL_CURE_CRITIC, SPELL_HEAL, SPELL_REMOVE_CURSE, SPELL_TRUE_SIGHT, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*10, 100);
       init_skills(gm, CLERIC_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, CLERIC_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(thief_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_DISARM, SKILL_KICK, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SKILL_HIDE, SKILL_SNEAK, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, THIEF_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, THIEF_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(bard_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_KICK, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_FLY, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, BARD_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
      return (GuildMaster(ch, cmd, arg, BARD_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(warrior_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BERSERK, 0},
      {SKILL_BASH, SKILL_KICK, SKILL_DISARM, 0}, 
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, WARRIOR_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
      return (GuildMaster(ch, cmd, arg, WARRIOR_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(paladin_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BASH, SKILL_KICK, SKILL_HOLY_WARCRY, SKILL_DISARM, 0},
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_CRITIC,
       SPELL_CURE_SERIOUS, SKILL_LAY_ON_HANDS, 0},  
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_SERIOUS,
       SPELL_CURE_CRITIC, SKILL_LAY_ON_HANDS, SPELL_TRUE_SIGHT, 0}, 
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*5, 100);
       init_skills(gm, PALADIN_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, PALADIN_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(druid_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SPELL_CREEPING_DOOM, SPELL_GEYSER, SPELL_EARTHQUAKE,
       SPELL_WIND_STORM, 0}, 
      {SPELL_CALL_LIGHTNING, SPELL_THORN, SPELL_VINE, SPELL_SUNRAY,
       SPELL_FAERIE_FIRE, SPELL_ANSUM, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_CAMOUFLAGE, SPELL_TRUE_SIGHT, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*10, 100);
       init_skills(gm, DRUID_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, DRUID_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(psi_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_PSIONIC_BLAST, SPELL_FEAR, SKILL_PHANTASMAL, 0},  
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SKILL_SPELL_SHIELD, SKILL_INVIS, SKILL_ILLUSIONARY_SHROUD, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*10, 100);
       init_skills(gm, PSI_LEVEL_IND);
       return (TRUE);

   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, PSI_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(ranger_guildmasters)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_STUN, 0}, 
      {SKILL_THRUST, SPELL_WEB, SPELL_FAERIE_FIRE, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_CAMOUFLAGE, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, GetMaxLevel(gm)*5, GetMaxLevel(gm)*5);
       init_skills(gm, RANGER_LEVEL_IND);
       return (TRUE);
       
   case SPEC_CMD:
       return (GuildMaster(ch, cmd, arg, RANGER_LEVEL_IND, gm));

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(shifter_guildmasters)
{
    struct char_data *gm = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (GuildMaster(ch, cmd, arg, SHIFTER_LEVEL_IND, gm));
    }

    return (FALSE);
}

SPECIAL(monk_guildmasters)
{
   struct char_data *gm = (struct char_data *) me;
   
   switch(type) {
      case SPEC_CMD:
	return (GuildMaster(ch, cmd, arg, MONK_LEVEL_IND, gm));
   }
   
   return (FALSE);
}

/*** generic procs for use with standard single class mobs ***/

SPECIAL(generic_mage)
{
   static int fight[][MAX_SET_SIZE] = { 
      {SPELL_ICE_STORM, SPELL_CHAIN_ELECTROCUTION, SPELL_ACID_RAIN,
       SPELL_LAVA_STORM, 0},
      {SPELL_DISPEL_MAGIC, 0},
      {SPELL_ELECTRIC_FIRE, SPELL_ACID_BLAST, SPELL_FIREBALL,
       SPELL_FROST_CLOUD, 0},
      {SPELL_CHILL_TOUCH, SPELL_BURNING_HANDS, SPELL_SHOCKING_GRASP,
       SPELL_ENERGY_DRAIN, SPELL_ELECTROCUTE, SPELL_FIRE_WIND,
       SPELL_FLAMESTRIKE,SPELL_SLEEP, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_FIRESHIELD, 0},
      {SPELL_ELECSHIELD, SPELL_COLDSHIELD, SKILL_VAMPSHIELD, 0},
      {SPELL_STONE_FIST, SPELL_STONE_SKIN, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, MAGE_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}

SPECIAL(old_generic_cleric)
{
   static int fight[][MAX_SET_SIZE] = { 
      {SPELL_POISON_GAS, 0},
      {SPELL_DISPEL_MAGIC, 0},
      {SPELL_HARMFUL_TOUCH, SPELL_WITHER, SPELL_RUPTURE, SPELL_IMPLODE,
       SPELL_DISINTEGRATE, 0}, 
      {SPELL_PARALYSIS, SPELL_WEAKNESS, SPELL_BLINDNESS, SPELL_POISON,
       SPELL_CURSE, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_SANCTUARY, 0},
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_SERIOUS,
       SPELL_CURE_CRITIC, SPELL_HEAL, SPELL_REMOVE_CURSE, SPELL_TRUE_SIGHT, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, CLERIC_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(generic_thief)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BACKSTAB, SKILL_DISARM, SKILL_TRIP, 0},
      {SKILL_GOUGE, 0}, 
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SKILL_HIDE, SKILL_SNEAK, 0},
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, THIEF_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}

SPECIAL(generic_warrior)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BASH, SKILL_DISARM, 0},
      {SKILL_BERSERK, SKILL_FLAIL, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {0}
   };
   struct char_data *gm = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gm, 100, 100);
       init_skills(gm, WARRIOR_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gm, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(generic_paladin)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_BASH, SKILL_KICK, SKILL_HOLY_WARCRY, SKILL_DISARM, 0},
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_CRITIC,
       SPELL_CURE_SERIOUS, SKILL_LAY_ON_HANDS, 0},  
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_REMOVE_POISON, SPELL_CURE_LIGHT, SPELL_CURE_SERIOUS,
       SPELL_CURE_CRITIC, SKILL_LAY_ON_HANDS, SPELL_TRUE_SIGHT, 0}, 
       {SKILL_BLESSING, SPELL_MOVESHIELD, SPELL_ENERGYSHIELD, 0},
       {0}
   };
   struct char_data *gen = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gen, 100, 100);
       init_skills(gen, PALADIN_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gen, fight, peace, cmd));
   }

   return (FALSE);
}

SPECIAL(generic_druid)
{
   static int fight[][MAX_SET_SIZE] = {
      {SPELL_CREEPING_DOOM, SPELL_GEYSER, SPELL_EARTHQUAKE,
       SPELL_WIND_STORM, 0}, 
      {SPELL_CALL_LIGHTNING, SPELL_THORN, SPELL_VINE, SPELL_SUNRAY, 0},
      { SPELL_FAERIE_FIRE, SPELL_ANSUM, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_CAMOUFLAGE, SPELL_TRUE_SIGHT, 0},
      {SPELL_PETRIFY, SPELL_COLDSHIELD, SPELL_POISONSHIELD, 0},
      {SPELL_ACIDSHIELD, SPELL_STONE_FIST, 0},
      {0}
   };
   struct char_data *gen = (struct char_data *) me;


   switch (type)
   {
   case SPEC_INIT:
       init_atts(gen, 100, 100);
       init_skills(gen, DRUID_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gen, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(generic_psi)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_PSIONIC_BLAST, SKILL_PHANTASMAL, SPELL_FEAR, 0},  
      {SKILL_PULSE, SPELL_SLEEP, SKILL_DRAIN_MANA, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SKILL_SPELL_SHIELD, SKILL_INVIS, SKILL_ILLUSIONARY_SHROUD, 0},
      {SKILL_ADRENALIZE, SKILL_TOLERANCE, 0},
      {0}
   };
   struct char_data *gen = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gen, 100, 100);
       init_skills(gen, PSI_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gen, fight, peace, cmd));
   }

   return (FALSE);
}


SPECIAL(generic_ranger)
{
   static int fight[][MAX_SET_SIZE] = {
      {SKILL_STUN, 0}, 
      {SKILL_THRUST, SPELL_WEB, SPELL_FAERIE_FIRE, 0},
      {0}
   };
   static int peace[][MAX_SET_SIZE] = {
      {SPELL_CAMOUFLAGE, 0},
      {SPELL_PETRIFY, SPELL_ACIDSHIELD, SPELL_POISONSHIELD, 0},
      {0}
   };
   struct char_data *gen = (struct char_data *) me;

   switch (type)
   {
   case SPEC_INIT:
       init_atts(gen, 100, 100);
       init_skills(gen, RANGER_LEVEL_IND);
       return (TRUE);

   case SPEC_IDLE:
   case SPEC_FIGHT:
       return (do_abilities(gen, fight, peace, cmd));
   }

   return (FALSE);
}

SPECIAL(generic_shifter)
{
   struct char_data *gen = (struct char_data *) me;
   
   switch(type) {
   case SPEC_INIT:
      init_atts(gen, 500, 500);
      init_skills(gen, SHIFTER_LEVEL_IND);
      return (TRUE);
      
   case SPEC_IDLE:
   case SPEC_FIGHT:
      return (TRUE);
   }
   
   return (FALSE);
}

SPECIAL(all_classes)
{
   struct char_data *gen = (struct char_data *) me;
   
   switch(type) {
   case SPEC_INIT:
      init_atts(gen, 100, 100);
      init_skills(gen, MAGE_LEVEL_IND);
      init_skills(gen, CLERIC_LEVEL_IND);
      init_skills(gen, WARRIOR_LEVEL_IND);
      init_skills(gen, PSI_LEVEL_IND);
      init_skills(gen, DRUID_LEVEL_IND);
      init_skills(gen, BARD_LEVEL_IND);
      init_skills(gen, MONK_LEVEL_IND);
      init_skills(gen, SHIFTER_LEVEL_IND);
      init_skills(gen, RANGER_LEVEL_IND);
      init_skills(gen, THIEF_LEVEL_IND);
      init_skills(gen, PALADIN_LEVEL_IND);
      return (TRUE);
      
   case SPEC_IDLE:
   case SPEC_FIGHT:
      return (TRUE);
   }
   
   return (FALSE);
}

SPECIAL(ctfhunter) {
   
   struct char_data *ctf = (struct char_data *)me;
   switch(type) {
    case SPEC_INIT:
      init_atts(ctf, 100, 100);
      init_skills(ctf, MAGE_LEVEL_IND);
      return TRUE;
    case SPEC_IDLE:
      //do stuff
      return TRUE;
    case SPEC_FIGHT:
      return TRUE;
   }
   return FALSE;
}
