
#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <ctype.h>

#include "structs.h"
#include "spells.h"
#include "parse.h"
#include "spelltab.h"
#include "spell_procs.h"
#include "eng_procs.h"
#include "utils.h"
#include "utility.h"

/* forward declarations */
extern int spell_hash(char);
void init_spells();
void init_skills();
void assign_spell_levels(void);

/* our globals... */
short spell_count;
parse_table *spell_table;
struct spell_info spell_list[MAX_SPL_LIST];

void assign_spell_pointers(void)
{
    struct spell_info* info;
    int i;

    init_spells();
    init_skills();
    assign_spell_levels();

    spell_table = MakeParseTable(spell_hash(0), spell_hash);
  
    for(spell_count = 0, i = 0; i < MAX_SPL_LIST; i++)
	if ((info = &spell_list[i])->name)
	{
	    AddParseEntry(spell_table, info->name, (void*) info);
	    if(info->number > spell_count)
		spell_count = info->number;
	}
}

struct spell_info* spell_by_number(int spellNo)
{
  if((spellNo <= 0) || (spellNo > spell_count))
    return 0;
  
    return spell_list[spellNo].name ? &spell_list[spellNo] : 0;
}

const char* spell_name(int spellNo)
{
    struct spell_info* spell;
    static char buf[256];
  
    if((spell = spell_by_number(spellNo)))
	return spell->name;
    sprintf(buf, "Illegal Spell #%d", spellNo);
    return buf;
}

struct spell_info* locate_spell(char* name, int exact)
{
    return (struct spell_info*) FindParseEntry(spell_table, name, exact);
}

int spell_hash(char data)
{
    // Solaar: added null condition
    if(data == NULL) 
    { 
      slog( "ERROR: null value passed to command_hash in spelltab.c.");
      return 69;
    }
    switch(data)
    {
    case '\'':    return 63;
    case '@':     return 64;
    case ' ':     return 65;
    case ':':     return 66;
    case '*':     return 67;
    case '_':     return 68;
    default:
	if(islower(data))
	    return data - 'a';
	else if(isupper(data))
	    return data - 'A' + 26;
	else if(isdigit(data))
	    return data - '0' + 52;

	return 69;
    case 0:       return 70;
    }
}


/*****************************************************************
 * spello() - Assign a spell entry to the spell table
 *
 * Fields:
 *   name             - name of skill/spell
 *   number           - number of skill/spell
 *   beats            - lag after casting
 *   minimum_position - minimum position for casting
 *   min_usesmana     - mana used to cast spell
 *   spell_pointer    - function to cast spell
 *   logic_pointer    - logic function (smart mob calc)
 *   pre_call_pointer - smart mob function call
 *   targets          - What spell can be cast on: TARG_x
 *   modifiers        - Things modifing success
 *   learn_rate       - % of cance of learning skill /10
 *****************************************************************/

void spello(const char *name, ush_int number, byte beats,
	    byte minimum_position, ush_int min_usesmana,
	    spell_proc spell_pointer, logic_proc logic_pointer,
	    pre_call_proc pre_call_pointer, int targets, int modifiers,
	    ubyte learn_rate)
{
    spell_list[number].name = name;
    spell_list[number].number = number;
    spell_list[number].beats = beats;
    spell_list[number].minimum_position = minimum_position;
    spell_list[number].min_usesmana = min_usesmana;
    spell_list[number].spell_pointer = spell_pointer;
    spell_list[number].logic_pointer = logic_pointer;
    spell_list[number].pre_call_pointer = pre_call_pointer;
    spell_list[number].targets = targets;
    spell_list[number].modifiers = modifiers;
    spell_list[number].learn_rate = learn_rate;
}


void unused_spell(ush_int number)
{
    int i;

    spell_list[number].name = NULL;
    spell_list[number].number = number;
    spell_list[number].beats = 0;
    spell_list[number].minimum_position = 0;
    spell_list[number].min_usesmana = 0;
    spell_list[number].spell_pointer = NULL;
    spell_list[number].logic_pointer = NULL;
    spell_list[number].pre_call_pointer = NULL;
    spell_list[number].targets = 0;
    spell_list[number].modifiers = 0;
    spell_list[number].learn_rate = 0;

    for (i = 0; i < MAX_LEVEL_IND + 1; i++)
	spell_list[number].min_level[i] = MAX_MORT+1;
    for (i = 0; i < MAX_SPELL_COMPONENTS; i++)
	spell_list[number].components[i] = -1;
}


void spell_level(int spell, int clss, int level)
{
    spell_list[spell].min_level[clss] = level;
}


/*
 * The following code replaces the old spell table
 *   ... work by Min and Thrytis 1996
 *
 * initialization of a spell takes the form:
 * spello("spellname", Spell_Define (from spells.h),
 *        Spell Beats (Pulses spell takes),
 *        Spell Position, Spell Mana Cost
 *        Spell function, Spell Logic function, Spell Precall Function,
 *        Targets
 *        Modifiers (int, wis, etc) that spell depends on
 *        Percentage chance for learning (divided by 10)
 *            (ie: 10 = 1% chance of learning)
 */

/* Herin Lieth the spells... may the forever wreack havoc upon the land! */

void init_spells()
{
  int i;

  /* initialize the spell table */
  for (i = 0; i < MAX_SPL_LIST; i++)
      unused_spell(i);

  spello("acid rain", SPELL_ACID_RAIN, 24, POSITION_FIGHTING, 30, 
	 cast_acid_rain, logic_acid_rain, pre_acid_rain,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT | SPLMOD_EVIL, 20);	

  spello("chill touch",SPELL_CHILL_TOUCH, 12, POSITION_FIGHTING, 15,
	 cast_chill_touch, logic_chill_touch, pre_chill_touch,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, 
	 SPLMOD_INT, 20);

  spello("burning hands",SPELL_BURNING_HANDS, 12, POSITION_FIGHTING, 15,
	 cast_burning_hands, logic_burning_hands, pre_burning_hands,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO, 
	 SPLMOD_INT, 20);

  spello("shocking grasp", SPELL_SHOCKING_GRASP, 12, POSITION_FIGHTING, 15, 
	 cast_shocking_grasp, logic_shocking_grasp, pre_shocking_grasp,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL,20);

  spello("earthquake",SPELL_EARTHQUAKE, 24, POSITION_FIGHTING, 30,
	 cast_earthquake, logic_earthquake, pre_earthquake,
	 TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	 SPLMOD_INT | SPLMOD_DWARF | SPLMOD_GNOME, 50);

  spello("implode", SPELL_IMPLODE, 12, POSITION_FIGHTING, 40,
	 cast_implode,  logic_implode, pre_implode,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("chain electrocution", SPELL_CHAIN_ELECTROCUTION, 24, POSITION_FIGHTING, 45,
	 cast_chain_electrocution, logic_chain_electrocution, pre_chain_electrocution,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT, 50);

  spello("harmful touch", SPELL_HARMFUL_TOUCH, 12, POSITION_FIGHTING, 15,
	 cast_harmful_touch, logic_harmful_touch, pre_harmful_touch,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("fireball", SPELL_FIREBALL, 12, POSITION_FIGHTING, 15,
	 cast_fireball, logic_fireball, pre_fireball,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 10);

  spello("call lightning", SPELL_CALL_LIGHTNING, 12, POSITION_FIGHTING, 15,
	 cast_call_lightning, logic_call_lightning, pre_call_lightning,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_ELF, 30);

  spello("energy drain", SPELL_ENERGY_DRAIN, 12, POSITION_FIGHTING, 15,
	 cast_energy_drain, logic_energy_drain, pre_energy_drain,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 30);

  spello("rupture", SPELL_RUPTURE, 12, POSITION_FIGHTING, 30,
	 cast_rupture, logic_rupture, pre_rupture,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 10);

  spello("armor", SPELL_ARMOR, 12, POSITION_STANDING, 2,
	 cast_armor, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 10);

  spello("teleport", SPELL_TELEPORT, 12, POSITION_FIGHTING, 15,
	 cast_teleport, logic_teleport, pre_teleport,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_WIS, 40);

  spello("bless", SPELL_BLESS, 12, POSITION_STANDING, 2,
	 cast_bless, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 10);

  spello("blindness", SPELL_BLINDNESS, 12, POSITION_FIGHTING, 10,
	 cast_blindness, logic_blindness, pre_blindness,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 40);

  spello("charm", SPELL_CHARM, 12, POSITION_STANDING, 15,
	 cast_charm, 0, 0,
	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT,
	 SPLMOD_INT | SPLMOD_CHA | SPLMOD_FEMALE, 5);

  spello("clone", SPELL_CLONE, 12, POSITION_STANDING, 20,
	 cast_clone, 0,0,
	 TAR_OBJ_INV | TAR_OBJ_ROOM,
	 SPLMOD_INT | SPLMOD_WIS, 50);

  spello("control weather", SPELL_CONTROL_WEATHER, 24, POSITION_STANDING, 20,
	 cast_control_weather, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 50);

  spello("create food", SPELL_CREATE_FOOD, 12, POSITION_STANDING, 5,
	 cast_create_food, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("create water", SPELL_CREATE_WATER, 12, POSITION_STANDING, 5,
	 cast_create_water, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("cure blind", SPELL_CURE_BLIND, 12, POSITION_STANDING, 10,
	 cast_cure_blind, logic_cure_blind, pre_cure_blind,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("cure critic", SPELL_CURE_CRITIC, 12, POSITION_FIGHTING, 25,
	 cast_cure_critic, logic_cure_critic, pre_cure_critic,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 10);

  spello("cure light", SPELL_CURE_LIGHT, 12, POSITION_FIGHTING, 5,
	 cast_cure_light, logic_cure_light, pre_cure_light,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 50);

  spello("curse", SPELL_CURSE, 12, POSITION_FIGHTING, 10,
	 cast_curse, logic_curse, pre_curse,
	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV |
	 TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 40);

  spello("fire wind", SPELL_FIRE_WIND, 12, POSITION_FIGHTING, 15,
	 cast_fire_wind, logic_fire_wind, pre_fire_wind,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("detect invisibility", SPELL_DETECT_INVISIBLE, 12, POSITION_STANDING, 3,
	 cast_detect_invisibility, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("frost cloud", SPELL_FROST_CLOUD, 12, POSITION_FIGHTING, 15,
	 cast_frost_cloud, logic_frost_cloud, pre_frost_cloud,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("detect poison", SPELL_DETECT_POISON, 12, POSITION_STANDING, 10,
	 cast_detect_poison, 0,0,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);
         
  spello("enchant weapon", SPELL_ENCHANT_WEAPON, 12, POSITION_STANDING, 100,
	 cast_enchant_weapon, 0,0,
	 TAR_OBJ_INV | TAR_OBJ_ROOM,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("heal", SPELL_HEAL, 12, POSITION_FIGHTING, 50,
	 cast_heal, logic_heal, pre_heal,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 10);

  spello("regenerate", SPELL_REGEN, 12, POSITION_STANDING, 5, 
	 cast_regen, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("invisibility", SPELL_INVISIBLE, 12, POSITION_STANDING, 3,
	 cast_invisibility, logic_invisibility, pre_invisibility,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("locate object", SPELL_LOCATE_OBJECT, 12, POSITION_STANDING, 20,
	 cast_locate_object, 0,0,
	 TAR_NAME,
	 SPLMOD_INT, 20);

  spello("poison", SPELL_POISON, 12, POSITION_FIGHTING, 15,
	 cast_poison, logic_poison, pre_poison,
	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV |
	 TAR_FIGHT_VICT | TAR_VIOLENT,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("poison gas", SPELL_POISON_GAS, 24, POSITION_FIGHTING, 30,
	 cast_poison_gas, logic_poison_gas, pre_poison_gas,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("remove curse", SPELL_REMOVE_CURSE, 12, POSITION_STANDING, 10,
	 cast_remove_curse, logic_remove_curse, pre_remove_curse,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("sanctuary", SPELL_SANCTUARY, 12, POSITION_STANDING, 30,
	 cast_sanctuary, logic_sanctuary, pre_sanctuary,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("sleep", SPELL_SLEEP, 12, POSITION_FIGHTING, 15, 
	 cast_sleep, logic_sleep, pre_sleep,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("strength", SPELL_STRENGTH, 12, POSITION_STANDING, 2, 
	 cast_strength, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("summon", SPELL_SUMMON, 12, POSITION_STANDING, 20, 
	 cast_summon, 0,0,
	 TAR_CHAR_WORLD,
	 SPLMOD_INT, 20);

  spello("suggestion", SKILL_SUGGESTION, 12, POSITION_STANDING, 5,
	 0, 0,0,
	 TAR_CHAR_ROOM | TAR_VIOLENT,
	 SPLMOD_CHA, 20);

  spello("electrocute", SPELL_ELECTROCUTE, 12, POSITION_FIGHTING, 15,
	 cast_electrocute, logic_electrocute, pre_electrocute,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("word of recall", SPELL_WORD_OF_RECALL, 12, POSITION_STANDING, 20,
	 cast_word_of_recall, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20); 

  spello("remove poison", SPELL_REMOVE_POISON, 12, POSITION_STANDING, 10,
	 cast_remove_poison, logic_remove_poison, pre_remove_poison,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("sense life", SPELL_SENSE_LIFE, 12, POSITION_STANDING, 3,
	 cast_sense_life, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD | SPLMOD_DWARF, 20);

  spello("identify", SPELL_IDENTIFY, 12, POSITION_STANDING, 20,
	 cast_identify, 0,0,
	 TAR_OBJ_INV | TAR_OBJ_ROOM,
	 SPLMOD_INT, 20);

  spello("infravision", SPELL_INFRAVISION, 12, POSITION_STANDING, 3, 
	 cast_infravision, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("flamestrike", SPELL_FLAMESTRIKE, 12, POSITION_FIGHTING, 15, 
	 cast_flamestrike, logic_flamestrike, pre_flamestrike,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("electric fire", SPELL_ELECTRIC_FIRE, 12, POSITION_FIGHTING, 15, 
	 cast_electric_fire, logic_electric_fire, pre_electric_fire,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("weakness", SPELL_WEAKNESS, 12, POSITION_FIGHTING, 15, 
	 cast_weakness, logic_weakness, pre_weakness,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("dispel magic", SPELL_DISPEL_MAGIC, 3, POSITION_FIGHTING, 20, 
	 cast_dispel_magic, logic_dispel_magic, pre_dispel_magic,
	 TAR_VIOLENT | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 40);

  spello("knock", SPELL_KNOCK, 12, POSITION_STANDING, 10, 
	 cast_knock, 0, 0,
	 TAR_IGNORE,
	 SPLMOD_INT | SPLMOD_DEX, 20);

  spello("sense aura", SPELL_SENSE_AURA, 12, POSITION_STANDING, 3, 
	 cast_sense_aura, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("animate dead", SPELL_ANIMATE_DEAD, 12, POSITION_STANDING, 10, 
	 cast_animate_dead, 0, 0,
	 TAR_OBJ_ROOM,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("paralyze", SPELL_PARALYSIS, 12, POSITION_FIGHTING, 15, 
	 cast_paralyze, logic_paralyze, pre_paralyze,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("remove paralysis", SPELL_REMOVE_PARALYSIS, 12, POSITION_FIGHTING, 10, 
	 cast_remove_paralysis, 0, 0,
	 TAR_CHAR_ROOM,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("fear", SPELL_FEAR, 12, POSITION_FIGHTING, 15, 
	 cast_fear, logic_fear, pre_fear,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("acid blast", SPELL_ACID_BLAST, 12, POSITION_FIGHTING, 15, 
	 cast_acid_blast, logic_acid_blast, pre_acid_blast,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("waterbreath", SPELL_WATER_BREATH, 12, POSITION_STANDING, 3, 
	 cast_water_breath, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("fly", SPELL_FLY, 12, POSITION_STANDING, 3, 
	 cast_flying, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("wither", SPELL_WITHER, 12, POSITION_FIGHTING, 20, 
	 cast_decay, logic_decay, pre_decay,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 20);

  spello("wind storm", SPELL_WIND_STORM, 24, POSITION_FIGHTING, 15, 
	 cast_wind_storm, logic_wind_storm, pre_wind_storm,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT, 20);

  spello("ice storm", SPELL_ICE_STORM, 24, POSITION_FIGHTING, 30, 
	 cast_ice_storm, logic_ice_storm, pre_ice_storm,
	 TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	 SPLMOD_INT, 200);

  spello("shield", SPELL_SHIELD, 12, POSITION_STANDING, 3, 
	 cast_shield, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("vampyric touch", SPELL_VAMPYRIC_TOUCH, 12, POSITION_FIGHTING, 15, 
	 cast_vampyric_touch, logic_vampyric_touch, pre_vampyric_touch,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_EVIL, 15);

  spello("empathic heal", SPELL_EMPATHIC_HEAL, 12, POSITION_STANDING, 50, 
	 cast_empathic_heal, 0, 0,
	 TAR_CHAR_ROOM | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_GOOD, 15);

  spello("monsum", SPELL_MONSUM, 12, POSITION_FIGHTING, 20, 
	 cast_monsum, logic_monsum, pre_monsum,
	 TAR_IGNORE,
	 SPLMOD_INT, 40);

#if 0
  spello("heat metal", SPELL_HEAT_METAL, 12, POSITION_FIGHTING, 15, 
	 cast_heat_metal, 0, 0,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("rot", SPELL_ROT, 12, POSITION_FIGHTING, 15, 
	 cast_rot, 0, 0,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);
#endif

  spello("fireshield", SPELL_FIRESHIELD, 12, POSITION_STANDING, 20, 
	 cast_fireshield, logic_fireshield, pre_fireshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("elecshield", SPELL_ELECSHIELD, 12, POSITION_STANDING, 20, 
	 cast_elecshield, logic_elecshield, pre_elecshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("poisonshield", SPELL_POISONSHIELD, 12, POSITION_STANDING, 20, 
	 cast_poisonshield, logic_poisonshield, pre_poisonshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("coldshield", SPELL_COLDSHIELD, 12, POSITION_STANDING, 20, 
	 cast_coldshield, logic_coldshield, pre_coldshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);
 
  spello("moveshield", SPELL_MOVESHIELD, 12, POSITION_STANDING, 20,
	 cast_moveshield, logic_moveshield, pre_moveshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("energyshield", SPELL_ENERGYSHIELD, 12, POSITION_STANDING, 20, 
	 cast_energyshield, logic_energyshield, pre_energyshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("acidshield", SPELL_ACIDSHIELD, 12, POSITION_STANDING, 20, 
	 cast_acidshield, logic_acidshield, pre_acidshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("vampshield", SKILL_VAMPSHIELD, 12, POSITION_STANDING, 20, 
	 cast_vampshield, logic_vampshield, pre_vampshield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("manashield", SKILL_MANASHIELD, 12, POSITION_STANDING, 20, 
	 cast_manashield, logic_manashield, pre_manashield,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("cure serious", SPELL_CURE_SERIOUS, 12, POSITION_FIGHTING, 13, 
	 cast_cure_serious, logic_cure_serious, pre_cure_serious,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("refresh", SPELL_REFRESH, 12, POSITION_STANDING, 10, 
	 cast_refresh, logic_refresh, pre_refresh,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("turn", SPELL_TURN, 12, POSITION_FIGHTING, 15, 
	 cast_turn, logic_turn, pre_turn,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("succor", SPELL_SUCCOR, 12, POSITION_STANDING, 20, 
	 cast_succor, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("create light", SPELL_LIGHT, 12, POSITION_STANDING, 5, 
	 cast_light, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("calm", SPELL_CALM, 12, POSITION_FIGHTING, 15, 
	 cast_calm, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_NONO,
	 SPLMOD_INT | SPLMOD_GOOD, 20);
  
  spello("stone skin", SPELL_STONE_SKIN, 12, POSITION_STANDING, 10, 
	 cast_stone_skin, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("petrify", SPELL_PETRIFY, 12, POSITION_STANDING, 10, 
	 cast_petrify, 0,0,
	 TAR_SELF_ONLY,
	 SPLMOD_INT, 20);

  spello("conjure elemental", SPELL_CONJURE_ELEMENTAL, 12, POSITION_STANDING, 20,
	 cast_conjure_elemental, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("true sight", SPELL_TRUE_SIGHT, 12, POSITION_STANDING, 10, 
	 cast_true_seeing, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("minor creation", SPELL_MINOR_CREATE, 12, POSITION_STANDING, 20, 
	 cast_creation, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT | SPLMOD_WIS | SPLMOD_GOOD, 20);

  spello("faerie fire", SPELL_FAERIE_FIRE, 12, POSITION_FIGHTING, 10, 
	 cast_faerie_fire, logic_faerie_fire, pre_faerie_fire,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("faerie fog", SPELL_FAERIE_FOG, 12, POSITION_STANDING, 30, 
	 cast_faerie_fog, 0,0,
	 TAR_IGNORE | TAR_AREA,
	 SPLMOD_INT, 20);

  spello("harden weapon", SPELL_HARDEN_WEAPON, 12, POSITION_STANDING, 100,
         cast_harden_weapon, 0,0,
         TAR_OBJ_INV | TAR_OBJ_ROOM,
         SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("possession", SPELL_POSSESSION, 12, POSITION_STANDING, 40, 
	 cast_possession, 0,0,
	 TAR_OBJ_ROOM,
	 SPLMOD_INT | SPLMOD_EVIL | SPLMOD_WIS, 20);

  spello("polymorph self", SPELL_POLY_SELF, 12, POSITION_STANDING, 30, 
	 cast_poly_self, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("mana", SPELL_MANA, 12, POSITION_FIGHTING, 75, 
	 cast_mana, 0,0,
	 TAR_SELF_NONO | TAR_CHAR_ROOM, 
	 SPLMOD_INT, 20);

  spello("astral walk", SPELL_ASTRAL_WALK, 12, POSITION_STANDING, 100, 
	 cast_astral_walk, 0,0,
	 TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("nature walk", NATURE_WALK, 12, POSITION_STANDING, 100, 
	 cast_nature_walk, 0,0,
	 TAR_CHAR_WORLD,
	 SPLMOD_INT | SPLMOD_WIS | SPLMOD_ELF, 20);

  spello("resurrection", SPELL_RESURRECTION, 12, POSITION_STANDING, 30, 
	 cast_resurrection, 0,0,
	 TAR_OBJ_ROOM,
	 SPLMOD_INT | SPLMOD_GOOD, 20);

  spello("heroes feast", SPELL_H_FEAST, 12, POSITION_STANDING, 40, 
	 cast_heroes_feast, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group fly", SPELL_FLY_GROUP, 12, POSITION_STANDING, 50, 
	 cast_fly_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("web", SPELL_WEB, 12, POSITION_FIGHTING, 15, 
	 cast_web, logic_web, pre_web,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("minor track", SPELL_MINOR_TRACK, 12, POSITION_STANDING, 2, 
	 cast_minor_track, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_WIS, 50);

  spello("major track", SPELL_MAJOR_TRACK, 12, POSITION_STANDING, 4, 
	 cast_major_track, 0,0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_WIS, 20);

  spello("disintegrate", SPELL_DISINTEGRATE, 12, POSITION_FIGHTING, 50, 
	 cast_disintegrate, logic_disintegrate, pre_disintegrate,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("lava storm", SPELL_LAVA_STORM, 24, POSITION_FIGHTING, 30, 
	 cast_lava_storm, logic_lava_storm, pre_lava_storm,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT, 15);

  spello("knowledge", SPELL_KNOWLEDGE, 12, POSITION_STANDING, 40, 
	 cast_knowledge, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT | SPLMOD_WIS, 20);

  spello("group armor",SPELL_ARMOR_GROUP, 12, POSITION_STANDING, 50, 
	 cast_armor_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group sight", SPELL_TRUE_SEEING_GROUP, 12, POSITION_STANDING, 50, 
	 cast_true_seeing_group,0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group invisible", SPELL_INVIS_GROUP, 12, POSITION_STANDING, 50, 
	 cast_invis_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group detect invisible", SPELL_DINVIS_GROUP, 12, POSITION_STANDING, 50, 
	 cast_dinvis_group,0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group cure light", SPELL_CURE_LIGHT_GROUP, 12, POSITION_STANDING, 30, 
	 cast_cure_light_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group heal", SPELL_HEAL_GROUP, 12, POSITION_STANDING, 75,
         cast_heal_group, 0,0,
         TAR_IGNORE | TAR_PURE_CLASS,
         SPLMOD_INT, 20);

  spello("group waterbreath", SPELL_WATERBREATH_GROUP, 12, POSITION_STANDING, 50, 
	 cast_waterbreath_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("group recall", SPELL_RECALL_GROUP, 12, POSITION_STANDING, 50, 
	 cast_recall_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("golem", SPELL_GOLEM, 12, POSITION_STANDING, 30, 
	 cast_golem, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("mount", SPELL_MOUNT, 12, POSITION_STANDING, 25, 
	 cast_mount, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("tree", SPELL_TREE, 12, POSITION_STANDING, 30, 
	 cast_tree, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("fire ward", SPELL_WARD_FIRE, 12, POSITION_STANDING, 10, 
	 cast_ward_fire, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
         SPLMOD_INT, 20);

  spello("cold ward", SPELL_WARD_COLD, 12, POSITION_STANDING, 10, 
	 cast_ward_cold, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
         SPLMOD_INT, 20);

  spello("electric ward", SPELL_WARD_ELEC, 12, POSITION_STANDING, 10, 
	 cast_ward_elec, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
         SPLMOD_INT, 20);

  spello("energy ward", SPELL_WARD_ENERGY, 12, POSITION_STANDING, 10, 
	 cast_ward_energy, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
         SPLMOD_INT, 20);

  spello("acid ward", SPELL_WARD_ACID, 12, POSITION_STANDING, 10, 
	 cast_ward_acid, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("stone fist", SPELL_STONE_FIST, 12, POSITION_STANDING, 10, 
	 cast_stone_fist, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("geyser", SPELL_GEYSER, 24, POSITION_FIGHTING, 25,
	 cast_geyser, logic_geyser, pre_geyser,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT, 20);
  
/*   spello("fire breath", SPELL_FIRE_BREATH, 24, POSITION_FIGHTING, 30,  */
/* 	 cast_fire_breath, 0,0, */
/* 	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT, */
/* 	 SPLMOD_INT, 20); */
  
/*   spello("poison gas breath", SPELL_POISON_GAS_BREATH, 24, POSITION_FIGHTING, 30,  */
/* 	 cast_poison_gas_breath, 0,0, */
/* 	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT, */
/* 	 SPLMOD_INT, 20); */
  
/*   spello("frost breath", SPELL_FROST_BREATH, 24, POSITION_FIGHTING, 30,  */
/* 	 cast_frost_breath, 0,0, */
/* 	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT, */
/* 	 SPLMOD_INT, 20); */
  
/*   spello("acid breath", SPELL_ACID_BREATH, 24, POSITION_FIGHTING, 30,  */
/* 	 cast_acid_breath, 0,0, */
/* 	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT, */
/* 	 SPLMOD_INT, 20); */
  
/*   spello("lightning breath", SPELL_LIGHTNING_BREATH, 24, POSITION_FIGHTING, 30,  */
/* 	 cast_lightning_breath, 0,0, */
/* 	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT, */
/* 	 SPLMOD_INT, 20); */

  // Breath weapons, the potion edition
  spello("fire breath", SPELL_POTION_FIRE_BREATH, 24, POSITION_STANDING, 30,
	 cast_fire_breath_aff, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
  
  spello("poison gas breath", SPELL_POTION_POISON_GAS_BREATH, 24, POSITION_STANDING, 30,
	 cast_poison_gas_breath_aff, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
  
  spello("frost breath", SPELL_POTION_FROST_BREATH, 24, POSITION_STANDING, 30,
	 cast_frost_breath_aff, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
  
  spello("acid breath", SPELL_POTION_ACID_BREATH, 24, POSITION_STANDING, 30,
	 cast_acid_breath_aff, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
  
  spello("lightning breath", SPELL_POTION_LIGHTNING_BREATH, 24, POSITION_STANDING, 30,
	 cast_lightning_breath_aff, 0,0,
	 TAR_CHAR_ROOM | TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
  // END Breath potions
  
  spello("continual light", SPELL_CONT_LIGHT, 12, POSITION_STANDING, 100,
	 cast_continual_light, 0,0,
	 TAR_OBJ_INV | TAR_OBJ_ROOM,
	 SPLMOD_INT, 20);
	 
  spello("continual dark", SPELL_CONT_DARK, 12, POSITION_STANDING, 100,
         cast_continual_dark, 0,0,
	 TAR_OBJ_INV | TAR_OBJ_ROOM,
	 SPLMOD_INT, 20);

  spello("group astral", SPELL_ASTRAL_GROUP, 12, POSITION_STANDING, 50,
	 cast_astral_group, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("ray of purification", SPELL_RAY_OF_PURIFICATION, 12, POSITION_STANDING, 10, 
	 cast_ray_of_purification, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT | SPLMOD_GOOD, 20);
  
  spello("sunray", SPELL_SUNRAY, 12, POSITION_FIGHTING, 15, 
	 cast_sunray, logic_sunray, pre_sunray,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
	 TAR_SELF_NONO,
	 SPLMOD_INT, 20);
  
  spello("windwalk", SPELL_WINDWALK, 12, POSITION_STANDING, 10, 
	 cast_windwalk, 0,0,
	 TAR_SELF_ONLY,
	 SPLMOD_INT, 20);

  spello("moonbeam", SPELL_MOONBEAM, 12, POSITION_STANDING, 5, 
	 cast_moonbeam, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("goodberry", SPELL_GOODBERRY, 12, POSITION_STANDING, 5, 
	 cast_goodberry, 0,0,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("ansum", SPELL_ANSUM, 12, POSITION_FIGHTING, 25, 
	 cast_ansum, logic_ansum, pre_ansum,
	 TAR_IGNORE,
	 SPLMOD_INT, 20);

  spello("thorn", SPELL_THORN, 12, POSITION_FIGHTING, 15, 
	 cast_thorn, logic_thorn, pre_thorn,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("vine", SPELL_VINE, 12, POSITION_FIGHTING, 15, 
	 cast_vine, logic_vine, pre_vine,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	 SPLMOD_INT, 20);

  spello("creeping doom", SPELL_CREEPING_DOOM, 24, POSITION_FIGHTING, 40, 
	 cast_creep, logic_creeping_doom, pre_creeping_doom,
	 TAR_IGNORE | TAR_AREA | TAR_VIOLENT,
	 SPLMOD_INT, 20);

  spello("camouflage", SPELL_CAMOUFLAGE, 12, POSITION_FIGHTING, 10, 
	 cast_camouflage, logic_camouflage, pre_camouflage, 
	 TAR_SELF_ONLY,
	 SPLMOD_INT, 20);
	
  spello("slow", SPELL_SLOW, 12, POSITION_FIGHTING, 15,
	 cast_slow, logic_slow, pre_slow,
	 TAR_CHAR_ROOM | TAR_SELF_NONO |
	 TAR_FIGHT_VICT | TAR_VIOLENT,
	 SPLMOD_INT, 20);

  spello("haste", SPELL_HASTE, 12, POSITION_FIGHTING, 25,
	 cast_haste, 0, 0,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF, 
	 SPLMOD_INT, 20);

  spello("despair", SPELL_DESPAIR, 12, POSITION_FIGHTING, 15,
	 cast_despair, logic_despair, pre_despair,
	 TAR_CHAR_ROOM | TAR_SELF_NONO | 
	 TAR_FIGHT_VICT | TAR_VIOLENT,
	 SPLMOD_INT, 20);

  spello("inspire", SPELL_INSPIRE, 12, POSITION_FIGHTING, 15,
	 cast_inspire, logic_inspire, pre_inspire,
	 TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	 SPLMOD_INT, 20);

  spello("unweave", SPELL_UNWEAVE, 12, POSITION_STANDING, 50,
	   cast_unweave, 0, 0,
	   TAR_SKILL | TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	   SPLMOD_INT | SPLMOD_DEX, 20);
   
  spello("enforest", SPELL_ENFOREST, 30, POSITION_STANDING, 20,
	 cast_enforest, 0, 0,
	 TAR_IGNORE,
	 SPLMOD_INT | SPLMOD_DEX, 20);

  spello("gust", SPELL_GUST, 12, POSITION_FIGHTING, 15,
         cast_gust, 0, 0,
         TAR_VIOLENT | TAR_FIGHT_VICT |
         TAR_SELF_NONO | TAR_CHAR_ROOM,
         SPLMOD_INT, 20);
   
  spello("inferno", SPELL_INFERNO, 30, POSITION_STANDING, 70,
	 cast_inferno, 0, 0,
	 TAR_IGNORE | TAR_AREA,
	 SPLMOD_INT | SPLMOD_WIS, 20);
}

/* END OF SPELLS */

/* BEGINNING OF SKILLS INITIALIZATION */

/* skills init looks like this:
 * skillo(same as spells from here on)
 */

	      /* skills from here on down are mostly here to hold
		 space and to make sure that if a character gets
		 stuck with an affect or objects, it will show up
		 as something useful... */



void init_skills()
{
    spello("sneak", SKILL_SNEAK, 5, POSITION_STANDING, 0,
	   0, logic_sneak, pre_sneak,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_INT, 20);

    spello("hide", SKILL_HIDE, 5, POSITION_STANDING, 0,
	   0, logic_hide, pre_hide,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_INT, 20);

    spello("steal", SKILL_STEAL, 5, POSITION_STANDING, 0, 
	   0, 0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX | SPLMOD_INT, 20);

    spello("throw", SKILL_THROW, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_SELF_NONO | TAR_VIOLENT,
	   SPLMOD_DEX | SPLMOD_STR, 20);

    spello("backstab", SKILL_BACKSTAB, 5, POSITION_STANDING, 0, 
	   0, logic_backstab, pre_backstab,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX | SPLMOD_STR, 20);

    spello("pick lock", SKILL_PICK_LOCK, 5, POSITION_STANDING, 10,
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_INT, 20);

    spello("kick", SKILL_KICK, 5, POSITION_STANDING, 0, 
	   0, logic_kick, pre_kick,
	   TAR_SKILL | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX | SPLMOD_STR, 20);

    spello("bash", SKILL_BASH, 5, POSITION_STANDING, 0, 
	   0, logic_bash, pre_bash,
	   TAR_SKILL | TAR_FIGHT_VICT | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX | SPLMOD_STR, 20);

    spello("retreat", SKILL_RETREAT, 5, POSITION_STANDING, 0, 
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_DEX, 20);
    
    spello("rescue", SKILL_RESCUE, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_SELF_NONO | TAR_CHAR_ROOM,
	   SPLMOD_STR | SPLMOD_DEX, 20);

    spello("hypnosis", SKILL_HYPNOSIS, 5, POSITION_STANDING, 25, 
	   0, 0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT | SPLMOD_CHA, 20);

    spello("meditate", SKILL_MEDITATE, 5, POSITION_STANDING, 0, 
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_INT | SPLMOD_WIS, 20);

    spello("scry", SKILL_SCRY, 5, POSITION_STANDING, 20, 
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_INT | SPLMOD_WIS, 20);

    spello("adrenalize", SKILL_ADRENALIZE, 5, POSITION_STANDING, 5, 
	   0, 0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	   SPLMOD_INT, 20);

    spello("greater gate", SKILL_GREATER_GATE, 5, POSITION_STANDING, 120, 
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_INT | SPLMOD_WIS, 20);

    spello("vanish", SKILL_INVIS, 5, POSITION_STANDING, 5, 
	   0, logic_vanish, pre_vanish,
	   TAR_SKILL | TAR_SELF_ONLY,
	   SPLMOD_INT, 20);
	   
    spello("lesser gate", SKILL_LESSER_GATE, 5, POSITION_STANDING, 100,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("canibalize", SKILL_CANIBALIZE, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("illusionary shroud", SKILL_ILLUSIONARY_SHROUD, 5, POSITION_STANDING, 5,
	   0,logic_illusionary_shroud, pre_illusionary_shroud,
	   TAR_SKILL,
	   SPLMOD_INT, 20);
	   
    spello("phantasmal killer", SKILL_PHANTASMAL, 5, POSITION_STANDING, 25,
	   0, logic_phantasmal, pre_phantasmal,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("levitate", SKILL_LEVITATE, 5, POSITION_STANDING, 5,
          0,0,0,
          TAR_SKILL | TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
          SPLMOD_INT, 20);

    spello("great sight", SKILL_GREAT_SIGHT, 5, POSITION_STANDING, 10,
	   0,0,0,
	   TAR_SKILL | TAR_PURE_CLASS,
	   SPLMOD_INT, 20);

    spello("spell shield", SKILL_SPELL_SHIELD, 5, POSITION_STANDING, 50,
	   0, logic_spell_shield, pre_spell_shield,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("brew", SKILL_BREW, 5, POSITION_STANDING, 15,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("tan", SKILL_TAN, 5, POSITION_STANDING, 15,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("blast", SKILL_PSIONIC_BLAST, 5, POSITION_STANDING, 25,
	   0, logic_psionic_blast, pre_psionic_blast,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("brainstorm", SKILL_BRAINSTORM, 5, POSITION_STANDING, 50,
	   0, 0, 0,
	   TAR_SKILL | TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	   SPLMOD_INT, 20);

    spello("combustion", SKILL_COMBUSTION, 5, POSITION_STANDING, 20,
	   0,0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("thought throw", SKILL_THOUGHT_THROW, 5, POSITION_STANDING, 15,
	   0, 0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("constrict", SKILL_CONSTRICT, 5, POSITION_STANDING, 10,
	   0,0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_STR, 20);

    spello("drain mana", SKILL_DRAIN_MANA, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("layhands", SKILL_LAY_ON_HANDS, 5, POSITION_STANDING, 35,
	   0, logic_lay_on_hands, pre_lay_on_hands,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	   SPLMOD_INT, 20);

    spello("warcry", SKILL_HOLY_WARCRY, 5, POSITION_STANDING, 5,
	   0, logic_holy_warcry, pre_holy_warcry,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("blessing", SKILL_BLESSING, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL |  TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	   SPLMOD_INT, 20);

    // the potion version of paladins blessing
    spello("potionblessing", SKILL_BLESSING, 5, POSITION_STANDING, 0,
	   cast_blessing, 0, 0,
	   TAR_SKILL |  TAR_CHAR_ROOM | TAR_DEFAULT_SELF,
	   SPLMOD_INT, 20);
  
    spello("friends", SPELL_FRIENDS, 12, POSITION_STANDING, 15,
          cast_friends, 0, 0,
          TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT,
          SPLMOD_INT | SPLMOD_CHA | SPLMOD_FEMALE, 5);
 
    spello("lullabye", SPELL_LULLABYE, 12, POSITION_STANDING, 15,
         cast_lullabye, logic_lullabye, pre_lullabye,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
         TAR_SELF_NONO,
         SPLMOD_INT, 20);

    spello("terror", SPELL_TERROR, 12, POSITION_STANDING, 15,
         cast_terror, logic_terror, pre_terror,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
         TAR_SELF_NONO,
         SPLMOD_INT, 20);
 
    spello("blur", SPELL_BLUR, 5, POSITION_STANDING, 5,
         cast_blur, logic_blur, pre_blur,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_DEFAULT_SELF,
         SPLMOD_INT, 20);

    spello("silence", SPELL_SILENCE, 12, POSITION_STANDING, 15,
         cast_silence, 0, 0,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT |
         TAR_SELF_NONO,
         SPLMOD_INT, 20);

    spello("hrescue", SKILL_HEROIC_RESCUE, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_CHA, 30);

    spello("thrust", SKILL_THRUST, 5, POSITION_STANDING, 0,
	   0, logic_thrust, pre_thrust,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX, 30);

    spello("ration", SKILL_RATION, 5, POSITION_STANDING, 10,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT | SPLMOD_DEX, 30);

    spello("befriend", SKILL_ANIMAL_FRIENDSHIP, 5, POSITION_STANDING, 15,
	   0,0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);

    spello("stun", SKILL_STUN, 5, POSITION_STANDING, 0,
	   0, logic_stun, pre_stun,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX | SPLMOD_DEX, 20);

    spello("pinch", SKILL_PINCH, 5, POSITION_STANDING, 0,
           0, logic_stun, pre_stun,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
           SPLMOD_DEX | SPLMOD_DEX, 20);

    spello("taunt", SKILL_TAUNT, 5, POSITION_STANDING, 0,
           0, 0, 0,
           TAR_SKILL | TAR_SELF_NONO | TAR_CHAR_ROOM,
           SPLMOD_STR | SPLMOD_DEX, 20);

    spello("archery", SKILL_ARCHERY, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_STR, 20);

    spello("ride", SKILL_RIDE, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("doorbash", SKILL_DOORBASH, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL | TAR_NOTEACH,
	   SPLMOD_STR, 20);
	   
    spello("track", SKILL_HUNT, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("disarm", SKILL_DISARM, 5, POSITION_STANDING, 0,
	   0,logic_disarm, pre_disarm,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_STR | SPLMOD_INT, 20);

    spello("push", SKILL_PUSH, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL | TAR_VIOLENT | TAR_CHAR_ROOM | TAR_SELF_NONO,
	   SPLMOD_STR, 20);

    spello("dodge", SKILL_DODGE, 5, POSITION_STANDING, 0, 
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("berserk", SKILL_BERSERK, 5, POSITION_STANDING, 10,
	   0,logic_berserk, pre_berserk,
	   TAR_SKILL,
	   SPLMOD_STR, 20);

    spello("dual wield", SKILL_DUAL_WIELD, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_DEX, 5);

    spello("quick draw", SKILL_QUICK_DRAW, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_DEX, 50);

    spello("trip", SKILL_TRIP, 5, POSITION_STANDING, 0,
	   0, logic_trip, pre_trip,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_STR, 20);

    spello("circle", SKILL_CIRCLE, 5, POSITION_STANDING, 0,
	   0, 0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX, 30);

    spello("search", SKILL_SEARCH, 5, POSITION_STANDING, 10,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_INT, 30);

    spello("form", SKILL_FORM, 5, POSITION_FIGHTING, 5,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("wings", SKILL_WINGS, 5, POSITION_FIGHTING, 5,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("gills", SKILL_GILLS, 5, POSITION_FIGHTING, 5,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("trace", SKILL_TRACE, 5, POSITION_FIGHTING, 5,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("chameleon skin", SKILL_CHAMELEON, 5, POSITION_STANDING, 5,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("melt", SKILL_MELT, 5, POSITION_RESTING, 10,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("cocoon", SKILL_REGEN, 5, POSITION_STANDING, 0, 
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("shift", SKILL_SHIFT, 5, POSITION_STANDING, 20,
	   0, 0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("bind", SKILL_BIND, 5, POSITION_FIGHTING, 5,
	   0, 0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_STR | SPLMOD_DEX, 20);

    spello("contract", SKILL_CONTRACT, 5, POSITION_FIGHTING, 5, 
	   0, 0, 0,
	   TAR_SKILL | TAR_FIGHT_VICT | TAR_VIOLENT,
	   SPLMOD_STR, 20);

    spello("plate", SKILL_PLATE, 5, POSITION_STANDING, 5,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("appraise", SKILL_APPRAISE, 5, POSITION_STANDING, 10,
	   0, 0, 0,
	   TAR_SKILL | TAR_OBJ_INV | TAR_OBJ_ROOM,
	   SPLMOD_INT, 20);

    spello("feint", SKILL_FEINT, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_DEX, 30);

    spello("limb", SKILL_LIMB, 10, POSITION_STANDING, 30,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_INT, 30);

    spello("palm", SKILL_PALM, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL | TAR_OBJ_ROOM | TAR_SELF_NONO,
	   SPLMOD_DEX, 30);

    spello("sample", SKILL_SAMPLE, 5, POSITION_STANDING, 100,
	   0, 0, 0,
	   TAR_SKILL,
	   SPLMOD_DEX | SPLMOD_INT, 20);

    spello("melee1", SKILL_MELEE1, 5, POSITION_STANDING, 0, 
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30 );

    spello("melee2", SKILL_MELEE2, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("melee3", SKILL_MELEE3, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30 );

    spello("melee4", SKILL_MELEE4, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("melee5", SKILL_MELEE5, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("melee6", SKILL_MELEE6, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);


    spello("melee7", SKILL_MELEE7, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);


    spello("melee8", SKILL_MELEE8, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);


    spello("melee9", SKILL_MELEE9, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);


    spello("melee10", SKILL_MELEE10, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);


    spello("melee11", SKILL_MELEE11, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("flail", SKILL_FLAIL, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	   SPLMOD_DEX | SPLMOD_STR, 20);

    spello("divert", SKILL_DIVERT, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_SELF_NONO | TAR_CHAR_ROOM,
	   SPLMOD_STR | SPLMOD_DEX, 20);

    spello("gouge", SKILL_GOUGE, 5, POSITION_STANDING, 0,
	   0, 0, 0,
	   TAR_SKILL | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_CHAR_ROOM | TAR_VIOLENT,
	   SPLMOD_STR | SPLMOD_DEX, 20);
	
    spello("blind fighting", SKILL_BLIND_FIGHTING, 5, POSITION_STANDING, 0,
	   0,0,0,
	   TAR_SKILL,
	   SPLMOD_DEX, 30);

    spello("tolerance", SKILL_TOLERANCE, 5, POSITION_STANDING, 5,
	   0,logic_tolerance, pre_tolerance,
	   TAR_SKILL,
	   SPLMOD_INT, 20);

    spello("ego", SKILL_EGO_WHIP, 5, POSITION_STANDING, 5, 
	   0, 0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO,
	   SPLMOD_INT, 20);
	
    spello("awe", SKILL_AWE, 5, POSITION_STANDING, 5,
	   0,0, 0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO,
	   SPLMOD_INT | SPLMOD_WIS | SPLMOD_CON, 20);
	   
    spello("spunch", SKILL_SHIELD_PUNCH, 5, POSITION_STANDING, 5,
	   0,0,0,
	   TAR_SKILL | TAR_CHAR_ROOM | TAR_VIOLENT | TAR_SELF_NONO | TAR_FIGHT_VICT,
	   SPLMOD_STR | SPLMOD_DEX | SPLMOD_AC, 20);
	
    spello("sblock", SKILL_SHIELD_BLOCK, 5, POSITION_FIGHTING, 5,
	   0,0,0,
	   TAR_SKILL, SPLMOD_DEX, 20);

    spello("explode", SKILL_EXPLODE, 5, POSITION_STANDING, 20,
	   0,0,0,
	   TAR_SKILL | TAR_IGNORE | TAR_VIOLENT | TAR_AREA,
	   SPLMOD_DEX | SPLMOD_STR, 20);

   spello("pulse", SKILL_PULSE, 5, POSITION_FIGHTING, 120,
	  0,0,0,
	  TAR_SKILL | TAR_AREA | TAR_VIOLENT,
	  SPLMOD_DEX, 20);

   spello("aura", SKILL_AURA, 5, POSITION_FIGHTING, 20,
	  0,0,0,
	  TAR_SKILL,
	  0, 20);
   
   spello("probe", SKILL_PROBE, 5, POSITION_STANDING, 30,
	  0,0,0,
	  TAR_SKILL,
	  0,20);

   spello("gattack", SKILL_GROUP_ATTACK, 5, POSITION_STANDING, 5,
	  0,0,0,
	  TAR_SKILL | TAR_VIOLENT,
	  0,10);
   
   spello("charge elements", SKILL_CHARGE_ELEMENTS, 5, POSITION_STANDING, 5,
	  0,0,0,
	  TAR_SKILL,
	  0, 5);
   
   spello("mkick", SKILL_MONK_KICK, 5, POSITION_FIGHTING, 5,
	  0,0,0,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT |TAR_SKILL | TAR_VIOLENT,
	  0, 5);
   
   spello("balance", SKILL_BALANCE, 5, POSITION_DEAD, 5,
	  0,0,0,
	  TAR_SKILL | TAR_PURE_CLASS,
	  0, 5);

   // 02/04/05 by mtr
   spello("fury", SKILL_FURY, 5, POSITION_STANDING, 0,
      0,0,0,
      TAR_SKILL | TAR_PURE_CLASS | TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_SELF_NONO,
      SPLMOD_DEX | SPLMOD_WIS, 30);
}


void assign_spell_levels(void)
{
    /* Magic Users */
    spell_level(SKILL_APPRAISE,		MAGE_LEVEL_IND, 1);
    spell_level(SPELL_INFRAVISION,	MAGE_LEVEL_IND, 1);
    spell_level(SPELL_CHILL_TOUCH,	MAGE_LEVEL_IND, 1);
    spell_level(SPELL_SHOCKING_GRASP,	MAGE_LEVEL_IND, 2);
    spell_level(SPELL_SHIELD,		MAGE_LEVEL_IND, 3);
    spell_level(SPELL_SLEEP,		MAGE_LEVEL_IND, 4);
    spell_level(SPELL_MONSUM,		MAGE_LEVEL_IND, 5);
    spell_level(SPELL_FEAR,		MAGE_LEVEL_IND, 5);
   // spell_level(SPELL_CHARM,		MAGE_LEVEL_IND, 5);
    spell_level(SPELL_DETECT_INVISIBLE, MAGE_LEVEL_IND, 6);
    spell_level(SPELL_LIGHT,		MAGE_LEVEL_IND, 6);
    spell_level(SPELL_INVISIBLE,	MAGE_LEVEL_IND, 6);
    spell_level(SPELL_FLY,		MAGE_LEVEL_IND, 7);
    spell_level(SPELL_POLY_SELF,	MAGE_LEVEL_IND, 8);
    spell_level(SPELL_FROST_CLOUD,	MAGE_LEVEL_IND, 9);
    spell_level(SPELL_BURNING_HANDS, 	MAGE_LEVEL_IND, 10);
    spell_level(SPELL_WARD_ENERGY,	MAGE_LEVEL_IND, 11);
    spell_level(SPELL_ACID_BLAST,	MAGE_LEVEL_IND, 11);
    spell_level(SPELL_CONJURE_ELEMENTAL,MAGE_LEVEL_IND, 12);
    spell_level(SPELL_ELECTROCUTE,	MAGE_LEVEL_IND, 13);
    spell_level(SPELL_TELEPORT,		MAGE_LEVEL_IND, 14);
    spell_level(SPELL_ENERGY_DRAIN,	MAGE_LEVEL_IND, 15);
    spell_level(SPELL_ENCHANT_WEAPON,	MAGE_LEVEL_IND, 16);
    spell_level(SPELL_MINOR_TRACK,	MAGE_LEVEL_IND, 17);
    spell_level(SPELL_MINOR_CREATE,	MAGE_LEVEL_IND, 18);
    spell_level(SPELL_WARD_COLD,	MAGE_LEVEL_IND, 19);
    spell_level(SPELL_ICE_STORM,	MAGE_LEVEL_IND, 19);
    spell_level(SPELL_STONE_FIST,	MAGE_LEVEL_IND, 20);
    spell_level(SPELL_CONT_LIGHT,	MAGE_LEVEL_IND, 20);
    spell_level(SPELL_CONT_DARK,	MAGE_LEVEL_IND, 20);
    spell_level(SPELL_FIRE_WIND,	MAGE_LEVEL_IND, 20);
    spell_level(SPELL_ELECTRIC_FIRE,	MAGE_LEVEL_IND, 21);
    spell_level(SKILL_RIDE,		MAGE_LEVEL_IND, 22);
    spell_level(SPELL_KNOCK,		MAGE_LEVEL_IND, 22);
    spell_level(SPELL_WATER_BREATH,	MAGE_LEVEL_IND, 23);
    spell_level(SKILL_MELEE1,		MAGE_LEVEL_IND, 25);
    spell_level(SPELL_STONE_SKIN,      	MAGE_LEVEL_IND, 25);
    spell_level(SPELL_DISPEL_MAGIC,	MAGE_LEVEL_IND, 26);
    spell_level(SPELL_ASTRAL_WALK,	MAGE_LEVEL_IND, 28);
    spell_level(SPELL_FLAMESTRIKE,	MAGE_LEVEL_IND, 30);
    spell_level(SPELL_WARD_ELEC,	MAGE_LEVEL_IND, 31);
    spell_level(SPELL_CHAIN_ELECTROCUTION, MAGE_LEVEL_IND, 31);
    spell_level(SPELL_FIRESHIELD,	MAGE_LEVEL_IND, 35);
    spell_level(SPELL_CLONE,		MAGE_LEVEL_IND, 35);
    spell_level(SPELL_MAJOR_TRACK,	MAGE_LEVEL_IND, 37);
    spell_level(SPELL_KNOWLEDGE,	MAGE_LEVEL_IND, 38);
    spell_level(SPELL_FIREBALL,		MAGE_LEVEL_IND, 40);
    spell_level(SPELL_WARD_ACID,	MAGE_LEVEL_IND, 41);
    spell_level(SPELL_ACID_RAIN,	MAGE_LEVEL_IND, 41);
    spell_level(SPELL_VAMPYRIC_TOUCH,	MAGE_LEVEL_IND, 45);
    spell_level(SPELL_FLY_GROUP,	MAGE_LEVEL_IND, 47);
    spell_level(SPELL_INVIS_GROUP,	MAGE_LEVEL_IND, 48);
    spell_level(SPELL_DINVIS_GROUP,	MAGE_LEVEL_IND, 48);
    spell_level(SPELL_WATERBREATH_GROUP,MAGE_LEVEL_IND, 49);
    spell_level(SPELL_LAVA_STORM,	MAGE_LEVEL_IND, 50);
    spell_level(SPELL_WARD_FIRE,	MAGE_LEVEL_IND, 50);
    spell_level(SPELL_ASTRAL_GROUP,	MAGE_LEVEL_IND, 50);
    spell_level(SPELL_SUMMON,           MAGE_LEVEL_IND, 60);
    spell_level(SPELL_ELECSHIELD,	MAGE_LEVEL_IND, 63);
    spell_level(SPELL_COLDSHIELD,	MAGE_LEVEL_IND, 87);
    spell_level(SKILL_VAMPSHIELD,	MAGE_LEVEL_IND, 107);

    /* Clerics */
    spell_level(SKILL_APPRAISE,		CLERIC_LEVEL_IND, 1);
    spell_level(SPELL_SENSE_AURA,	CLERIC_LEVEL_IND, 1);
    spell_level(SPELL_CURE_LIGHT,	CLERIC_LEVEL_IND, 1);
    spell_level(SPELL_ARMOR,		CLERIC_LEVEL_IND, 2);
    spell_level(SPELL_CURE_BLIND,	CLERIC_LEVEL_IND, 4);
    spell_level(SPELL_BLINDNESS,	CLERIC_LEVEL_IND, 4);
    spell_level(SPELL_HARMFUL_TOUCH,	CLERIC_LEVEL_IND, 5);
    spell_level(SPELL_LIGHT,		CLERIC_LEVEL_IND, 6);
    spell_level(SPELL_CREATE_WATER,	CLERIC_LEVEL_IND, 7);
    spell_level(SPELL_CREATE_FOOD,	CLERIC_LEVEL_IND, 8);
    spell_level(SPELL_REMOVE_POISON,	CLERIC_LEVEL_IND, 9);
    spell_level(SPELL_DETECT_POISON,	CLERIC_LEVEL_IND, 9);
    spell_level(SPELL_CURE_SERIOUS,	CLERIC_LEVEL_IND, 10);
    spell_level(SPELL_TURN,		CLERIC_LEVEL_IND, 11);
    spell_level(SPELL_ANIMATE_DEAD,	CLERIC_LEVEL_IND, 11);
    spell_level(SPELL_SENSE_LIFE,	CLERIC_LEVEL_IND, 12);
    spell_level(SPELL_BLESS,		CLERIC_LEVEL_IND, 13);
    spell_level(SPELL_REMOVE_CURSE,	CLERIC_LEVEL_IND, 14);
    spell_level(SPELL_CURSE,		CLERIC_LEVEL_IND, 14);
    spell_level(SPELL_WITHER,		CLERIC_LEVEL_IND, 15);
    spell_level(SPELL_PARALYSIS,	CLERIC_LEVEL_IND, 16);
    spell_level(SPELL_REMOVE_PARALYSIS,	CLERIC_LEVEL_IND, 16);
    spell_level(SPELL_WORD_OF_RECALL,	CLERIC_LEVEL_IND, 17);
    spell_level(SPELL_WEAKNESS,		CLERIC_LEVEL_IND, 18);
    spell_level(SPELL_STRENGTH,		CLERIC_LEVEL_IND, 18);
    spell_level(SPELL_REFRESH,		CLERIC_LEVEL_IND, 19);
    spell_level(SPELL_MANA,		CLERIC_LEVEL_IND, 20);
    spell_level(SPELL_CURE_CRITIC,	CLERIC_LEVEL_IND, 20);
    spell_level(SPELL_RESURRECTION,	CLERIC_LEVEL_IND, 21);
    spell_level(SKILL_RIDE,		CLERIC_LEVEL_IND, 22);
    spell_level(SPELL_TRUE_SIGHT,	CLERIC_LEVEL_IND, 24);
    spell_level(SPELL_REGEN,		CLERIC_LEVEL_IND, 25);
    spell_level(SKILL_MELEE1,		CLERIC_LEVEL_IND, 25);
    spell_level(SPELL_RUPTURE,		CLERIC_LEVEL_IND, 25);
    spell_level(SPELL_SUCCOR,		CLERIC_LEVEL_IND, 29);
    spell_level(SPELL_HEAL,		CLERIC_LEVEL_IND, 30);
    spell_level(SPELL_POSSESSION,	CLERIC_LEVEL_IND, 31);
    spell_level(SPELL_IMPLODE,		CLERIC_LEVEL_IND, 35);
    spell_level(SPELL_LOCATE_OBJECT,	CLERIC_LEVEL_IND, 36);
    spell_level(SPELL_H_FEAST,		CLERIC_LEVEL_IND, 39);
    spell_level(SPELL_EMPATHIC_HEAL,	CLERIC_LEVEL_IND, 40);
    spell_level(SPELL_POISON,		CLERIC_LEVEL_IND, 40);
    spell_level(SPELL_SANCTUARY,	CLERIC_LEVEL_IND, 41);
    spell_level(SPELL_DISINTEGRATE,	CLERIC_LEVEL_IND, 45);
    spell_level(SPELL_CURE_LIGHT_GROUP,	CLERIC_LEVEL_IND, 46);
    spell_level(SPELL_ARMOR_GROUP,	CLERIC_LEVEL_IND, 47);
    spell_level(SPELL_TRUE_SEEING_GROUP,CLERIC_LEVEL_IND, 48);
    spell_level(SPELL_RECALL_GROUP,	CLERIC_LEVEL_IND, 49);
    spell_level(SPELL_POISON_GAS,	CLERIC_LEVEL_IND, 50);
    spell_level(SPELL_RAY_OF_PURIFICATION,CLERIC_LEVEL_IND, 50);
    spell_level(SKILL_MELEE2,		CLERIC_LEVEL_IND, 50);
    spell_level(SPELL_POISONSHIELD,	CLERIC_LEVEL_IND, 63);
    spell_level(SPELL_HEAL_GROUP,	CLERIC_LEVEL_IND, 69);
    spell_level(SKILL_MELEE3,		CLERIC_LEVEL_IND, 75);
    spell_level(SPELL_ELECSHIELD,	CLERIC_LEVEL_IND, 93);
    spell_level(SKILL_MELEE4,		CLERIC_LEVEL_IND, 100);
    spell_level(SKILL_MELEE5,		CLERIC_LEVEL_IND, 125);

    /* Warriors */
    spell_level(SKILL_APPRAISE,		WARRIOR_LEVEL_IND, 1);
    spell_level(SKILL_KICK,		WARRIOR_LEVEL_IND, 1);
    spell_level(SKILL_BASH,		WARRIOR_LEVEL_IND, 2);
    spell_level(SKILL_RETREAT,		WARRIOR_LEVEL_IND, 3);
    spell_level(SKILL_DODGE,		WARRIOR_LEVEL_IND, 4);
    spell_level(SKILL_PUSH,		WARRIOR_LEVEL_IND, 5);
    spell_level(SKILL_BERSERK,		WARRIOR_LEVEL_IND, 6);
    spell_level(SKILL_RESCUE,		WARRIOR_LEVEL_IND, 8);
    spell_level(SKILL_DOORBASH,		WARRIOR_LEVEL_IND, 8);
    spell_level(SKILL_DISARM,		WARRIOR_LEVEL_IND, 10);
    spell_level(SKILL_DUAL_WIELD,	WARRIOR_LEVEL_IND, 10);
    spell_level(SKILL_MELEE1,		WARRIOR_LEVEL_IND, 11);
    spell_level(SKILL_RIDE,		WARRIOR_LEVEL_IND, 17);
    spell_level(SKILL_MELEE2,		WARRIOR_LEVEL_IND, 22);
    spell_level(SKILL_MELEE3,		WARRIOR_LEVEL_IND, 34);
    spell_level(SKILL_SHIELD_PUNCH, 	WARRIOR_LEVEL_IND, 40);
    spell_level(SKILL_SHIELD_BLOCK, 	WARRIOR_LEVEL_IND, 40);
    spell_level(SKILL_MELEE4,		WARRIOR_LEVEL_IND, 45);
    spell_level(SKILL_THROW,            WARRIOR_LEVEL_IND, 50);
    spell_level(SKILL_MELEE5,		WARRIOR_LEVEL_IND, 63);
    spell_level(SKILL_FLAIL,		WARRIOR_LEVEL_IND, 75);
    spell_level(SKILL_BLIND_FIGHTING, 	WARRIOR_LEVEL_IND, 75);
    spell_level(SKILL_MELEE6,		WARRIOR_LEVEL_IND, 76);
    spell_level(SKILL_MELEE7,		WARRIOR_LEVEL_IND, 89);
    spell_level(SKILL_MELEE8,		WARRIOR_LEVEL_IND, 102);
    spell_level(SKILL_MELEE9,		WARRIOR_LEVEL_IND, 115);
    spell_level(SKILL_MELEE10,		WARRIOR_LEVEL_IND, 125);

/* Thieves */
    spell_level(SKILL_APPRAISE,		THIEF_LEVEL_IND, 1);
    spell_level(SKILL_HIDE,		THIEF_LEVEL_IND, 1);
    spell_level(SKILL_STEAL,		THIEF_LEVEL_IND, 2);
    spell_level(SKILL_PICK_LOCK,	THIEF_LEVEL_IND, 3);
    spell_level(SKILL_BACKSTAB,		THIEF_LEVEL_IND, 5);
    spell_level(SKILL_DODGE,		THIEF_LEVEL_IND, 7);
    spell_level(SKILL_TRIP,		THIEF_LEVEL_IND, 9);
    spell_level(SKILL_THROW,		THIEF_LEVEL_IND, 9);
    spell_level(SKILL_FEINT,		THIEF_LEVEL_IND, 10);
    spell_level(SKILL_PALM,		THIEF_LEVEL_IND, 10);
    spell_level(SKILL_SNEAK,		THIEF_LEVEL_IND, 12);
    spell_level(SKILL_DUAL_WIELD,	THIEF_LEVEL_IND, 15);
    spell_level(SKILL_MELEE1,		THIEF_LEVEL_IND, 15);
    spell_level(SKILL_DISARM,		THIEF_LEVEL_IND, 15);
    spell_level(SKILL_BALANCE,          THIEF_LEVEL_IND, 17);
    spell_level(SKILL_RIDE,		THIEF_LEVEL_IND, 17);
    spell_level(SKILL_QUICK_DRAW,	THIEF_LEVEL_IND, 18);
    spell_level(SKILL_SEARCH,		THIEF_LEVEL_IND, 22);
    spell_level(SKILL_CIRCLE,		THIEF_LEVEL_IND, 26);
    spell_level(SKILL_MELEE2,		THIEF_LEVEL_IND, 30);
    spell_level(SKILL_MELEE3,		THIEF_LEVEL_IND, 45);
    spell_level(SKILL_MELEE4,		THIEF_LEVEL_IND, 66);
    spell_level(SKILL_GOUGE,            THIEF_LEVEL_IND, 70);
    spell_level(SKILL_DIVERT,           THIEF_LEVEL_IND, 73);
    spell_level(SKILL_BLIND_FIGHTING,   THIEF_LEVEL_IND, 75);
    spell_level(SKILL_MELEE5,		THIEF_LEVEL_IND, 82);
    spell_level(SKILL_MELEE6,		THIEF_LEVEL_IND, 100);
    spell_level(SKILL_MELEE7,		THIEF_LEVEL_IND, 116);
    spell_level(SKILL_MELEE8,		THIEF_LEVEL_IND, 125);

/* Paladins */

    spell_level(SKILL_APPRAISE,		PALADIN_LEVEL_IND, 1);
    spell_level(SKILL_KICK,		PALADIN_LEVEL_IND, 2);
    spell_level(SKILL_BASH,		PALADIN_LEVEL_IND, 3);
    spell_level(SPELL_ARMOR,		PALADIN_LEVEL_IND, 4);
    spell_level(SKILL_HOLY_WARCRY,	PALADIN_LEVEL_IND, 5);
    spell_level(SKILL_PUSH,		PALADIN_LEVEL_IND, 7);
    spell_level(SKILL_DODGE,		PALADIN_LEVEL_IND, 8);
    spell_level(SKILL_HEROIC_RESCUE,	PALADIN_LEVEL_IND, 9);
    spell_level(SPELL_CURE_LIGHT,	PALADIN_LEVEL_IND, 10);
    spell_level(SKILL_DISARM,		PALADIN_LEVEL_IND, 11);
    spell_level(SKILL_DOORBASH,		PALADIN_LEVEL_IND, 12);
    spell_level(SPELL_BLESS,		PALADIN_LEVEL_IND, 14);
    spell_level(SKILL_MELEE1,		PALADIN_LEVEL_IND, 15);
    spell_level(SKILL_RIDE,		PALADIN_LEVEL_IND, 17);
    spell_level(SPELL_SENSE_AURA,	PALADIN_LEVEL_IND, 18);
    spell_level(SPELL_REMOVE_POISON,	PALADIN_LEVEL_IND, 19);
    spell_level(SKILL_AURA,		PALADIN_LEVEL_IND, 20);
    spell_level(SPELL_CURE_SERIOUS,	PALADIN_LEVEL_IND, 20);
    spell_level(SPELL_REMOVE_PARALYSIS,	PALADIN_LEVEL_IND, 22);
    spell_level(SPELL_CURE_CRITIC,	PALADIN_LEVEL_IND, 25);
    spell_level(SPELL_TRUE_SIGHT,	PALADIN_LEVEL_IND, 27);
    spell_level(SKILL_BLESSING,		PALADIN_LEVEL_IND, 30);
    spell_level(SKILL_MELEE2,		PALADIN_LEVEL_IND, 30);
    spell_level(SPELL_REGEN,		PALADIN_LEVEL_IND, 32);
    spell_level(SPELL_WORD_OF_RECALL,	PALADIN_LEVEL_IND, 34);
    spell_level(SKILL_LAY_ON_HANDS,	PALADIN_LEVEL_IND, 40);
    spell_level(SKILL_MELEE3,		PALADIN_LEVEL_IND, 45);
    spell_level(SPELL_REMOVE_CURSE,	PALADIN_LEVEL_IND, 47);
    spell_level(SKILL_SHIELD_PUNCH,     PALADIN_LEVEL_IND, 50);
    spell_level(SKILL_SHIELD_BLOCK,     PALADIN_LEVEL_IND, 50);
    spell_level(SPELL_RECALL_GROUP,     PALADIN_LEVEL_IND, 55);
    spell_level(SKILL_MELEE4,		PALADIN_LEVEL_IND, 65);
    spell_level(SKILL_BLIND_FIGHTING,   PALADIN_LEVEL_IND, 80);
    spell_level(SKILL_MELEE5,		PALADIN_LEVEL_IND, 82);
    spell_level(SPELL_ENERGYSHIELD,	PALADIN_LEVEL_IND, 93);
    spell_level(SKILL_MELEE6,		PALADIN_LEVEL_IND, 100);
    spell_level(SPELL_MOVESHIELD,	PALADIN_LEVEL_IND, 110);
    spell_level(SKILL_MELEE7,		PALADIN_LEVEL_IND, 116);
    spell_level(SKILL_MELEE8,		PALADIN_LEVEL_IND, 125);

    /* Druids */
    spell_level(SKILL_APPRAISE,		DRUID_LEVEL_IND, 1);
    spell_level(SPELL_CALM,		DRUID_LEVEL_IND, 1);
    spell_level(SPELL_INFRAVISION,	DRUID_LEVEL_IND, 1);
    spell_level(SPELL_THORN,		DRUID_LEVEL_IND, 1);
    spell_level(SKILL_ANIMAL_FRIENDSHIP,DRUID_LEVEL_IND, 2);
    spell_level(SPELL_SUNRAY,		DRUID_LEVEL_IND, 4);
    spell_level(SPELL_GOODBERRY,	DRUID_LEVEL_IND, 5);
    spell_level(SPELL_ANSUM,		DRUID_LEVEL_IND, 5);
    spell_level(SPELL_MOONBEAM,		DRUID_LEVEL_IND, 6);
    spell_level(SKILL_RIDE,		DRUID_LEVEL_IND, 7);
    spell_level(SPELL_WINDWALK,		DRUID_LEVEL_IND, 7);
    spell_level(SPELL_TREE,		DRUID_LEVEL_IND, 8);
    spell_level(SPELL_MOUNT,		DRUID_LEVEL_IND, 9);
    spell_level(SPELL_VINE,		DRUID_LEVEL_IND, 10);
    spell_level(SPELL_WIND_STORM,	DRUID_LEVEL_IND, 10);
    spell_level(SPELL_WEB,		DRUID_LEVEL_IND, 11);
    spell_level(SKILL_BREW,		DRUID_LEVEL_IND, 12);
    spell_level(SPELL_FAERIE_FIRE,	DRUID_LEVEL_IND, 14);
    spell_level(SPELL_CAMOUFLAGE,	DRUID_LEVEL_IND, 15);
    spell_level(SPELL_FAERIE_FOG,	DRUID_LEVEL_IND, 16);
    spell_level(SPELL_GOLEM,		DRUID_LEVEL_IND, 17);
    spell_level(SPELL_MINOR_TRACK,	DRUID_LEVEL_IND, 18);
    spell_level(SPELL_GEYSER,		DRUID_LEVEL_IND, 20);
    spell_level(SKILL_TAN,		DRUID_LEVEL_IND, 21);
    spell_level(SPELL_WATER_BREATH,	DRUID_LEVEL_IND, 23);
    spell_level(SPELL_CONJURE_ELEMENTAL,DRUID_LEVEL_IND, 24);
    spell_level(SPELL_TRUE_SIGHT,	DRUID_LEVEL_IND, 24);
    spell_level(SKILL_MELEE1,		DRUID_LEVEL_IND, 25);
    spell_level(SPELL_CONTROL_WEATHER,	DRUID_LEVEL_IND, 25);
    spell_level(SPELL_PETRIFY,		DRUID_LEVEL_IND, 26);
    spell_level(NATURE_WALK,		DRUID_LEVEL_IND, 28);
    spell_level(SPELL_EARTHQUAKE,	DRUID_LEVEL_IND, 30);
    spell_level(SPELL_MANA,		DRUID_LEVEL_IND, 30);
    spell_level(SPELL_ENFOREST,         DRUID_LEVEL_IND, 32);
    spell_level(SPELL_STONE_FIST,	DRUID_LEVEL_IND, 35);
    spell_level(SPELL_WATERBREATH_GROUP,DRUID_LEVEL_IND, 37);
    spell_level(SPELL_MAJOR_TRACK,	DRUID_LEVEL_IND, 38);
    spell_level(SPELL_CALL_LIGHTNING,	DRUID_LEVEL_IND, 40);
    spell_level(SKILL_MELEE2,           DRUID_LEVEL_IND, 40);
    spell_level(SPELL_CREEPING_DOOM,	DRUID_LEVEL_IND, 50);
    spell_level(SPELL_COLDSHIELD,       DRUID_LEVEL_IND, 58);
    spell_level(SKILL_MELEE3,		DRUID_LEVEL_IND, 75);
    spell_level(SPELL_POISONSHIELD,     DRUID_LEVEL_IND, 91);
    spell_level(SKILL_MELEE4,		DRUID_LEVEL_IND, 100);
    spell_level(SPELL_ACIDSHIELD,       DRUID_LEVEL_IND, 109);
    spell_level(SKILL_MELEE5,           DRUID_LEVEL_IND, 125);

/* Psis */
    spell_level(SKILL_APPRAISE,		PSI_LEVEL_IND, 1);
    spell_level(SKILL_CONSTRICT,	PSI_LEVEL_IND, 1);
    spell_level(SKILL_INVIS,		PSI_LEVEL_IND, 1);
    spell_level(SPELL_FEAR,		PSI_LEVEL_IND, 2);
    spell_level(SPELL_SENSE_AURA,	PSI_LEVEL_IND, 3);
    spell_level(SPELL_SLEEP,		PSI_LEVEL_IND, 4);
    spell_level(SKILL_HYPNOSIS,		PSI_LEVEL_IND, 5);
    spell_level(SKILL_LEVITATE,		PSI_LEVEL_IND, 6);
    spell_level(SKILL_SCRY,		PSI_LEVEL_IND, 7);
    spell_level(SKILL_LESSER_GATE,	PSI_LEVEL_IND, 8);
    spell_level(SKILL_MEDITATE,		PSI_LEVEL_IND, 9);
    spell_level(SKILL_THOUGHT_THROW,	PSI_LEVEL_IND, 10);
    spell_level(SKILL_GREAT_SIGHT,	PSI_LEVEL_IND, 12);
    spell_level(SKILL_PHANTASMAL,	PSI_LEVEL_IND, 13);
    spell_level(SKILL_PROBE,		PSI_LEVEL_IND, 15);
    spell_level(SKILL_ILLUSIONARY_SHROUD,PSI_LEVEL_IND, 15);
    spell_level(SKILL_CANIBALIZE,	PSI_LEVEL_IND, 17);
    spell_level(SKILL_SPELL_SHIELD,	PSI_LEVEL_IND, 18);
    spell_level(SKILL_COMBUSTION,	PSI_LEVEL_IND, 20);
    spell_level(SKILL_ADRENALIZE,	PSI_LEVEL_IND, 22);
    spell_level(SKILL_RIDE,		PSI_LEVEL_IND, 22);
    spell_level(SKILL_DRAIN_MANA,	PSI_LEVEL_IND, 23);
    spell_level(SKILL_MELEE1,		PSI_LEVEL_IND, 25);
    spell_level(SKILL_GREATER_GATE,	PSI_LEVEL_IND, 28);
    spell_level(SKILL_PSIONIC_BLAST,	PSI_LEVEL_IND, 30);
    spell_level(SKILL_BRAINSTORM,	PSI_LEVEL_IND, 40);
    spell_level(SKILL_MELEE2,		PSI_LEVEL_IND, 50);
    spell_level(SKILL_TOLERANCE,        PSI_LEVEL_IND, 55);
    spell_level(SKILL_EGO_WHIP,         PSI_LEVEL_IND, 60);
    spell_level(SKILL_MELEE3,		PSI_LEVEL_IND, 75);
    spell_level(SKILL_PULSE,	        PSI_LEVEL_IND, 75);
    spell_level(SKILL_AWE,              PSI_LEVEL_IND, 80);
    spell_level(SKILL_MANASHIELD,	PSI_LEVEL_IND, 92);
    spell_level(SKILL_MELEE4,		PSI_LEVEL_IND, 100);
    spell_level(SKILL_VAMPSHIELD,	PSI_LEVEL_IND, 106);
    spell_level(SKILL_MELEE5,		PSI_LEVEL_IND, 125);


/* Ranger */
    spell_level(SKILL_APPRAISE,		RANGER_LEVEL_IND, 1);
    spell_level(SPELL_INFRAVISION,	RANGER_LEVEL_IND, 1);
    spell_level(SKILL_THRUST,		RANGER_LEVEL_IND, 1);
    spell_level(SKILL_RATION,		RANGER_LEVEL_IND, 2);
    spell_level(SPELL_CALM,		RANGER_LEVEL_IND, 3);
    spell_level(SKILL_TAN,		RANGER_LEVEL_IND, 5);
    spell_level(SKILL_HUNT,		RANGER_LEVEL_IND, 5);
    spell_level(SKILL_DODGE,		RANGER_LEVEL_IND, 8);
    spell_level(SKILL_ANIMAL_FRIENDSHIP, RANGER_LEVEL_IND, 10);
    spell_level(SKILL_ARCHERY,		RANGER_LEVEL_IND, 10);
    spell_level(SKILL_RIDE,		RANGER_LEVEL_IND, 12);
    spell_level(SPELL_MOUNT,		RANGER_LEVEL_IND, 14);
    spell_level(SKILL_MELEE1,		RANGER_LEVEL_IND, 15);
    spell_level(SKILL_DISARM,           RANGER_LEVEL_IND, 15);
    spell_level(SPELL_WEB,		RANGER_LEVEL_IND, 16);
    spell_level(SPELL_CAMOUFLAGE,	RANGER_LEVEL_IND, 18);
    spell_level(SPELL_FAERIE_FIRE,	RANGER_LEVEL_IND, 19);
    spell_level(SPELL_FAERIE_FOG,	RANGER_LEVEL_IND, 21);
    spell_level(SKILL_FEINT,		RANGER_LEVEL_IND, 25);
    spell_level(SKILL_MELEE2,		RANGER_LEVEL_IND, 30);
    spell_level(SPELL_PETRIFY,		RANGER_LEVEL_IND, 31);
    spell_level(SPELL_HARDEN_WEAPON,    RANGER_LEVEL_IND, 40);
    spell_level(SKILL_STUN,   	        RANGER_LEVEL_IND, 40);
    spell_level(SKILL_MELEE3,		RANGER_LEVEL_IND, 45);
    spell_level(SKILL_SHIELD_BLOCK,     RANGER_LEVEL_IND, 50);
    spell_level(SKILL_MELEE4,		RANGER_LEVEL_IND, 66);
    spell_level(SKILL_BLIND_FIGHTING,   RANGER_LEVEL_IND, 80);
    spell_level(SKILL_MELEE5,		RANGER_LEVEL_IND, 82);
    spell_level(SPELL_ACIDSHIELD,	RANGER_LEVEL_IND, 90);
    spell_level(SKILL_MELEE6,		RANGER_LEVEL_IND, 100);
    spell_level(SKILL_MELEE7,		RANGER_LEVEL_IND, 116);
    spell_level(SKILL_MELEE8,		RANGER_LEVEL_IND, 125);
    spell_level(SPELL_POISONSHIELD,	RANGER_LEVEL_IND, 105);

/* Shifters */
    spell_level(SPELL_INFRAVISION,	SHIFTER_LEVEL_IND, 1);
    spell_level(SKILL_APPRAISE,		SHIFTER_LEVEL_IND, 1);
    spell_level(SKILL_MELT,		SHIFTER_LEVEL_IND, 1);
    spell_level(SKILL_FORM,		SHIFTER_LEVEL_IND, 3);
    spell_level(SKILL_PLATE,		SHIFTER_LEVEL_IND, 5);
    spell_level(SPELL_DETECT_INVISIBLE, SHIFTER_LEVEL_IND, 6);
    spell_level(SKILL_SHIFT,		SHIFTER_LEVEL_IND, 8);
    spell_level(SKILL_CHAMELEON,	SHIFTER_LEVEL_IND, 10);
    spell_level(SKILL_REGEN,		SHIFTER_LEVEL_IND, 15);
    spell_level(SKILL_BIND,		SHIFTER_LEVEL_IND, 20);
    spell_level(SKILL_RIDE,		SHIFTER_LEVEL_IND, 22);
    spell_level(SKILL_MELEE1,		SHIFTER_LEVEL_IND, 23);
    spell_level(SKILL_WINGS,		SHIFTER_LEVEL_IND, 25);
    spell_level(SKILL_CONTRACT,		SHIFTER_LEVEL_IND, 25);
    spell_level(SKILL_SAMPLE,		SHIFTER_LEVEL_IND, 30);
    spell_level(SKILL_GILLS,		SHIFTER_LEVEL_IND, 30);
    spell_level(SKILL_DODGE,		SHIFTER_LEVEL_IND, 35);
    spell_level(SPELL_STONE_FIST,	SHIFTER_LEVEL_IND, 35);
    spell_level(SKILL_LIMB,		SHIFTER_LEVEL_IND, 35);
    spell_level(SKILL_MELEE2,		SHIFTER_LEVEL_IND, 45);
    spell_level(SKILL_EXPLODE,		SHIFTER_LEVEL_IND, 65);
    spell_level(SKILL_MELEE3,		SHIFTER_LEVEL_IND, 75);
    spell_level(SKILL_MELEE4,		SHIFTER_LEVEL_IND, 100);
    spell_level(SKILL_MELEE5,		SHIFTER_LEVEL_IND, 125);

/* Monks */
    spell_level(SKILL_APPRAISE,		MONK_LEVEL_IND, 1);
    spell_level(SKILL_KICK,		MONK_LEVEL_IND, 2);
    spell_level(SPELL_CREATE_FOOD,	MONK_LEVEL_IND, 3);
    spell_level(SPELL_CREATE_WATER,	MONK_LEVEL_IND, 4);
    spell_level(SPELL_CURE_LIGHT,       MONK_LEVEL_IND, 5);
    spell_level(SPELL_GUST,             MONK_LEVEL_IND, 5);
    spell_level(SPELL_DETECT_POISON,	MONK_LEVEL_IND, 7);
    spell_level(SPELL_BURNING_HANDS,	MONK_LEVEL_IND, 8);
    spell_level(SKILL_GROUP_ATTACK,     MONK_LEVEL_IND, 10);
    spell_level(SKILL_PUSH,		MONK_LEVEL_IND, 10);
    spell_level(SKILL_DODGE,		MONK_LEVEL_IND, 12);
    spell_level(SPELL_CHILL_TOUCH,	MONK_LEVEL_IND, 14);
    spell_level(SKILL_MELEE1,		MONK_LEVEL_IND, 15);
    spell_level(SPELL_CURE_SERIOUS,     MONK_LEVEL_IND, 17);
    spell_level(SKILL_TAUNT,		MONK_LEVEL_IND, 18);
    spell_level(SKILL_MONK_KICK,        MONK_LEVEL_IND, 20);
    spell_level(SKILL_RETREAT,		MONK_LEVEL_IND, 20);
    spell_level(SPELL_REMOVE_POISON,	MONK_LEVEL_IND, 22);
    spell_level(SKILL_TRIP,		MONK_LEVEL_IND, 25);
    spell_level(SPELL_CURE_CRITIC,	MONK_LEVEL_IND, 27);
    spell_level(SKILL_MELEE2,		MONK_LEVEL_IND, 30);
    spell_level(SKILL_CHARGE_ELEMENTS,  MONK_LEVEL_IND, 31);
    spell_level(SPELL_STONE_FIST,	MONK_LEVEL_IND, 32);
    spell_level(SKILL_THROW,		MONK_LEVEL_IND, 35);
    spell_level(SPELL_HEAL,		MONK_LEVEL_IND, 37);
    spell_level(SKILL_HIDE,		MONK_LEVEL_IND, 40);
    spell_level(SPELL_MINOR_TRACK,	MONK_LEVEL_IND, 42);
    spell_level(SKILL_FURY,		MONK_LEVEL_IND, 44);// 02/04/05 by mtr
    spell_level(SKILL_MELEE3,		MONK_LEVEL_IND, 45);
    spell_level(SKILL_BERSERK,		MONK_LEVEL_IND, 50);
    spell_level(SPELL_STONE_SKIN,	MONK_LEVEL_IND, 52);
    spell_level(SKILL_PINCH,		MONK_LEVEL_IND, 56);
    spell_level(SKILL_MELEE4,		MONK_LEVEL_IND, 60);
    spell_level(SKILL_SNEAK,		MONK_LEVEL_IND, 70);
    spell_level(SKILL_MELEE5,		MONK_LEVEL_IND, 75);
    spell_level(SPELL_HEAL_GROUP,       MONK_LEVEL_IND, 80);
    spell_level(SKILL_MELEE6,		MONK_LEVEL_IND, 90);
    spell_level(SPELL_MOVESHIELD,	MONK_LEVEL_IND, 100);
    spell_level(SKILL_MELEE7,		MONK_LEVEL_IND, 125);
    spell_level(SPELL_POISONSHIELD,	MONK_LEVEL_IND, 115);


    /* Bards */
    spell_level(SKILL_APPRAISE,	      BARD_LEVEL_IND, 1);
    spell_level(SPELL_LIGHT,          BARD_LEVEL_IND, 4);
    spell_level(SKILL_KICK,           BARD_LEVEL_IND, 6);
    spell_level(SKILL_HUNT,           BARD_LEVEL_IND, 7);
    spell_level(SPELL_BLUR,           BARD_LEVEL_IND, 7);
    spell_level(SPELL_FLY,            BARD_LEVEL_IND, 10);
    spell_level(SPELL_LULLABYE,       BARD_LEVEL_IND, 12);
    spell_level(SKILL_MELEE1,         BARD_LEVEL_IND, 16);
    spell_level(SPELL_FRIENDS,        BARD_LEVEL_IND, 18);
    spell_level(SKILL_TRIP,	      BARD_LEVEL_IND, 19);
    spell_level(SPELL_INSPIRE,        BARD_LEVEL_IND, 25);
    spell_level(SPELL_TERROR,         BARD_LEVEL_IND, 30);
    spell_level(SKILL_MELEE2,         BARD_LEVEL_IND, 31);
    spell_level(SPELL_DESPAIR,        BARD_LEVEL_IND, 33);
    spell_level(SPELL_SLOW,           BARD_LEVEL_IND, 38);
    spell_level(SKILL_SUGGESTION,     BARD_LEVEL_IND, 40);
    spell_level(SPELL_SILENCE,        BARD_LEVEL_IND, 45);
    spell_level(SKILL_MELEE3,         BARD_LEVEL_IND, 46);
    spell_level(SPELL_HASTE,          BARD_LEVEL_IND, 52);
    spell_level(SKILL_MELEE4,         BARD_LEVEL_IND, 66);
    spell_level(SPELL_COLDSHIELD,     BARD_LEVEL_IND, 75);
    spell_level(SKILL_MELEE5,         BARD_LEVEL_IND, 82);
    spell_level(SKILL_BLIND_FIGHTING, BARD_LEVEL_IND, 85);
    spell_level(SPELL_UNWEAVE,        BARD_LEVEL_IND, 90);
    spell_level(SPELL_FIRESHIELD,     BARD_LEVEL_IND, 95);
    spell_level(SKILL_MELEE6,         BARD_LEVEL_IND, 100);
    spell_level(SPELL_POISONSHIELD,   BARD_LEVEL_IND, 108);
    spell_level(SKILL_MELEE7,         BARD_LEVEL_IND, 116);
    spell_level(SKILL_MELEE8,         BARD_LEVEL_IND, 125);





}

