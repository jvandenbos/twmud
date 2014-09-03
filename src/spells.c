#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "poly.h"
#include "spell_procs.h"
#include "spell_util.h"
#include "multiclass.h"
#include "act.h"
#include "constants.h"
#include "find.h"
#include "spelltab.h"
#include "utility.h"
#include "util_str.h"
#include "statistic.h"
#include "vnum_mob.h"

void do_spell(ubyte level, struct char_data* ch,
              int type, int spell_no,
              struct char_data *victim, struct obj_data *obj,
              void (*proc)(ubyte level, struct char_data* ch, int type,
                           struct char_data* victim,
                           struct obj_data* obj))
{
  char buf[MAX_STRING_LENGTH];
  struct spell_info* spell;
  
  spell = spell_by_number(spell_no);
  // TMP: log all spells cast
  char buff[255];
  
  if (IS_PC(ch)) {
    if (victim)
      sprintf(buff, "%s,%s,%s\n\r", GET_NAME(ch), spell_name(spell_no), GET_NAME(victim));
    else if (obj)
      sprintf(buff, "%s,%s,%s\n\r", GET_NAME(ch), spell_name(spell_no), OBJ_NAME(obj));
    else
      sprintf(buff, "%s,%s\n\r", GET_NAME(ch), spell_name(spell_no));
    file_log(buff, "actions/spells.log");
  }
  
  switch(type) {
    
  case SPELL_TYPE_SPELL :	/* affect the target */
  case SPELL_TYPE_SCROLL:	/* or */
  case SPELL_TYPE_WAND  :	/* affect caster if no target */
    if (victim || obj)
      ;
    else if(!obj)
      victim = ch;
    break;
    
  case SPELL_TYPE_POTION:	/* affect the quaffer */
    victim = ch;
    obj = 0;
    break;
    
  case SPELL_TYPE_STAFF:	/* area affect */
    if(!(spell = spell_by_number(spell_no)) ||
       IS_SET(spell->targets, TAR_AREA))
      (*proc)(level, ch, type, victim, obj);
    else
      do_area_attack(level, ch, proc, type);
    return;
    
  default:
    sprintf(buf, "Screw-up in do_spell : spell=%d, type=%d",
	    spell_no, type);
    log_msg(buf);
    return;
  }
  
  (*proc)(level, ch, type, victim, obj);
}

/**************/
/*** spells ***/
/**************/

void cast_dispel_magic(ubyte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_DISPEL_MAGIC,
		    tar_ch, tar_obj, spell_dispel_magic);
}

void cast_light(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_LIGHT,
                    tar_ch, tar_obj, spell_light);
}

void cast_moonbeam(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_MOONBEAM,
                    tar_ch, tar_obj, spell_moonbeam);
}

void cast_create_food(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CREATE_FOOD,
                    tar_ch, tar_obj, spell_create_food);
}

void cast_goodberry(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_GOODBERRY,
                    tar_ch, tar_obj, spell_goodberry);
}

void cast_create_water(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CREATE_WATER,
                    tar_ch, tar_obj, spell_create_water);
}

void cast_succor(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SUCCOR,
                    tar_ch, tar_obj, spell_succor);
}

void cast_knowledge(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_KNOWLEDGE,
                    tar_ch, tar_obj, spell_knowledge);
}

void cast_knock(ubyte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj)
{
    int door, other_room;
    char dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char otype[MAX_INPUT_LENGTH];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    switch(type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND: {

	argument_interpreter(arg, otype, dir);

	if (!otype) {
	    send_to_char("Knock on what?\n\r",ch);
	    return;
	}

	if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

	    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
                sprintf(buf,"%s is not a container.\n\r ",OBJ_NAME(obj));
	    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
                sprintf(buf, "Silly! %s isn't even closed!\n\r ",
			OBJ_NAME(obj));
	    } else if (obj->obj_flags.value[2] < 0) {
                sprintf(buf,"%s doesn't have a lock...\n\r",OBJ_NAME(obj));
	    } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
                sprintf(buf,"Hehe.. %s wasn't even locked.\n\r",OBJ_NAME(obj));
	    } else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
                sprintf(buf,"%s resists your magic.\n\r",OBJ_NAME(obj));
	    } else {
		REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
		sprintf(buf,"<Click>\n\r");
		act("$n magically opens $p", FALSE, ch, obj, 0, TO_ROOM);
	    }
	    send_to_char(buf,ch);
	    return;
	} else if ((door = find_door(ch, otype, dir)) >= 0) {

	    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	       	send_to_char("That's absurd.\n\r", ch);
	    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
		send_to_char("You realize that the door is already open.\n\r", ch);
	    else if (EXIT(ch, door)->key < 0)
		send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
	    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	       	send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
	    else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
		send_to_char("You seem to be unable to knock this...\n\r", ch);
	    else {
		REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
		if (EXIT(ch, door)->keyword)
		    act("$n magically opens the lock of the $F.", 0, ch, 0,
			EXIT(ch, door)->keyword, TO_ROOM);
		else
		    act("$n magically opens the lock.", TRUE, ch, 0, 0, TO_ROOM);
		send_to_char("The lock quickly yields to your skills.\n\r", ch);
		if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
		    if ((back = real_roomp(other_room)->dir_option[rev_dir[door]]))
			if (back->to_room == ch->in_room)
			    REMOVE_BIT(back->exit_info, EX_LOCKED);
	    }
	}
    }
	break;
    default:
	log_msg("serious error in Knock.");
	break;
    }
}

void cast_clone(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CLONE,
                    tar_ch, tar_obj, spell_clone);
}

#define LONG_SWORD   3022
#define SHIELD       3042
#define BAG          3032
#define WATER_BARREL 6013
#define MUSHROOM     4052

void cast_creation(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    char buffer[MAX_STRING_LENGTH];
    int obj;
    struct obj_data *o;

    one_argument(arg,buffer);

    if (!str_cmp(buffer, "sword")) {
        obj = LONG_SWORD;
    } else if (!str_cmp(buffer, "shield")) {
        obj=SHIELD;
    } else if (!str_cmp(buffer, "bag")) {
        obj=BAG;
    } else if (!str_cmp(buffer, "barrel")) {
        obj=WATER_BARREL;
    } else if (!str_cmp(buffer, "food")) {
        obj=MUSHROOM;
    } else {
        send_to_char("That is beyond your powers to create.\n\r", ch);
        return;
    }

    /*    if (!(o=make_object(obj, NORAND))) {*/
    if (!(o=make_object(obj, VIRTUAL|NORAND))) {
       log_msg("screwup in minor create spell.");
       send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
       return;
    }

    spell_creation(level, ch, type,0, o);
}

void cast_infravision(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_INFRAVISION,
                    tar_ch, tar_obj, spell_infravision);
}

void cast_invisibility(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_INVISIBLE,
                    tar_ch, tar_obj, spell_invisibility);
}

void cast_detect_invisibility(ubyte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_DETECT_INVISIBLE,
                    tar_ch, tar_obj, spell_detect_invisibility);
}

void cast_sense_life(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SENSE_LIFE,
                    tar_ch, tar_obj, spell_sense_life);
}

void cast_true_seeing(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_TRUE_SIGHT,
                    tar_ch, tar_obj, spell_true_seeing);
}

void cast_sense_aura(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SENSE_AURA,
                    tar_ch, tar_obj, spell_sense_aura);
}

void cast_detect_poison(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_DETECT_POISON,
                    tar_ch, tar_obj, spell_detect_poison);
}

void cast_identify(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_IDENTIFY,
                    tar_ch, tar_obj, spell_identify);
}

void cast_mana(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_MANA,
                    tar_ch, tar_obj, spell_mana);
}

void cast_armor(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ARMOR,
                    tar_ch, tar_obj, spell_armor);
}

void cast_fireshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FIRESHIELD,
                    tar_ch, tar_obj, spell_fireshield);
}

void cast_elecshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ELECSHIELD,
                    tar_ch, tar_obj, spell_elecshield);
}

void cast_coldshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_COLDSHIELD,
                    tar_ch, tar_obj, spell_coldshield);
}

void cast_poisonshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_POISONSHIELD,
                    tar_ch, tar_obj, spell_poisonshield);
}

void cast_energyshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ENERGYSHIELD,
                    tar_ch, tar_obj, spell_energyshield);
}

void cast_acidshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ACIDSHIELD,
                    tar_ch, tar_obj, spell_acidshield);
}

void cast_vampshield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SKILL_VAMPSHIELD,
                    tar_ch, tar_obj, spell_vampshield);
}

void cast_manashield(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SKILL_MANASHIELD,
                    tar_ch, tar_obj, spell_manashield);
}

void cast_moveshield(ubyte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_MOVESHIELD,
                    tar_ch, tar_obj, spell_moveshield);
}

void cast_sanctuary(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SANCTUARY,
                    tar_ch, tar_obj, spell_sanctuary);
}

void cast_stone_skin(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_STONE_SKIN,
                    tar_ch, tar_obj, spell_stone_skin);
}

void cast_petrify(ubyte level, struct char_data *ch, char *arg, int type,
                  struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_PETRIFY,
                    tar_ch, tar_obj, spell_petrify);
}

void cast_shield(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SHIELD,
                    tar_ch, tar_obj, spell_shield);
}

void cast_fear(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FEAR,
                    tar_ch, tar_obj, spell_fear);
}

void cast_turn(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_TURN,
                    tar_ch, tar_obj, spell_turn);
}

void cast_flying(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FLY,
                    tar_ch, tar_obj, spell_flying);
}

void cast_windwalk(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_WINDWALK,
                    tar_ch, tar_obj, spell_windwalk);
}

void cast_water_breath(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_WATER_BREATH,
                    tar_ch, tar_obj, spell_water_breath);
}

void cast_bless(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_BLESS,
                    tar_ch, tar_obj, spell_bless);
}

void cast_blessing(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
  do_spell(level, ch, type, SKILL_BLESSING,
	   tar_ch, tar_obj, spell_blessing);
}

void cast_strength(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_STRENGTH,
                    tar_ch, tar_obj, spell_strength);
}

void cast_cure_blind(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CURE_BLIND,
                    tar_ch, tar_obj, spell_cure_blind);
}

void cast_remove_curse(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_REMOVE_CURSE,
                    tar_ch, tar_obj, spell_remove_curse);
}

void cast_ray_of_purification(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_RAY_OF_PURIFICATION,
                    tar_ch, tar_obj, spell_ray_of_purification);
}

void cast_remove_poison(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_REMOVE_POISON,
                    tar_ch, tar_obj, spell_remove_poison);
}

void cast_remove_paralysis(ubyte level, struct char_data *ch, char *arg, int type,
                           struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_REMOVE_PARALYSIS,
                    tar_ch, tar_obj, spell_remove_paralysis);
}

void cast_resurrection(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_RESURRECTION,
                    tar_ch, tar_obj, spell_resurrection);
}

void cast_refresh(ubyte level, struct char_data *ch, char *arg, int type,
                  struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_REFRESH,
                    tar_ch, tar_obj, spell_refresh);
}

void cast_minor_track(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (type==SPELL_TYPE_POTION)
      tar_ch=ch;
    spell_track (level, ch, type, tar_ch, 0);
}

void cast_major_track(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (type==SPELL_TYPE_POTION)
      tar_ch=ch;
    spell_track (level, ch, type, tar_ch, 1);
}

void cast_calm(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CALM,
                    tar_ch, tar_obj, spell_calm);
}

int GetPolyAge(const struct PolyType* list, int vnum)
{
    int x, age = 25;

    for(x=0 ; age!= 0;age=list[x].age, ++x)
     if(list[x].number == vnum)
	    return list[x].age;
    return 25; //default age if nothing found
}
int FindPoly(const struct PolyType* list, const char* name, int level)
{
    int x, index = 0;

    if(*name)
    {
	for(x = 0 ; list[x].name ; ++x)
	    if(isname(name, list[x].name) &&
	       list[x].level <= level)
		break;

	if (!list[x].name)
	{
	    return 0;
	}

	return list[x].number;
    }
    else
    {
	for(x = 0 ; list[x].name ; ++x)
	    if(list[x].level <= level)
		index = x;
	    else
		break;

	return list[index].number;
    }
}

void cast_poly_self(ubyte level, struct char_data *ch, char *arg, int type,
		    struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[MAX_INPUT_LENGTH];
  int vnum;
  struct char_data *mob;

  one_argument(arg,buffer);

  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n\r",ch);
    return;
  }

  if (type!=SPELL_TYPE_SPELL) {
    log_msg("illegal call to polymorph");
    return;
  }

  if(!(vnum = FindPoly(PolyList, buffer, GetMaxLevel(ch))))
  {
    send_to_char("Could not find anything like that.\n\r", ch);
    return;
  }

  if (!(mob=make_mobile(vnum, VIRTUAL))) {
    log_msg("screwup in polymorph spell.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }


  spell_poly_self(level, ch, type, mob, 0, SPELL_POLY_SELF);
}

void cast_tree(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
  struct char_data *mob;
  struct room_data *rp;

  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n\r",ch);
    return;
  }

  rp = real_roomp(ch->in_room);
  switch(rp->sector_type)   {
  case SECT_FIELD:
  case SECT_FOREST:
  case SECT_HILLS:
  case SECT_MOUNTAIN:
    break;
  default:
    send_to_char("You must be in the wilderness.\n\r",ch);
    return;
  }

  if (type!=SPELL_TYPE_SPELL) {
    log_msg("illegal call to tree");
    return;
  }

  if (!(mob=make_mobile(VMOB_DRUID_TREE, VIRTUAL))) {
    log_msg("screwup in tree spell.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  spell_poly_self(level, ch, type, mob, 0, SPELL_TREE);
}

void cast_heroes_feast(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_H_FEAST,
                    tar_ch, tar_obj, spell_heroes_feast);
}

void cast_mount(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_MOUNT,
                    tar_ch, tar_obj, spell_mount);
}

void cast_locate_object(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    spell_locate_object(level, ch, type, 0, tar_obj);
}

void cast_harden_weapon(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_HARDEN_WEAPON,
             tar_ch, tar_obj, spell_harden_weapon);
}

void cast_enchant_weapon(ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ENCHANT_WEAPON,
                    tar_ch, tar_obj, spell_enchant_weapon);
}

void cast_continual_light(ubyte level,struct char_data *ch, char *arg, int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CONT_LIGHT,
                    tar_ch, tar_obj, spell_continual_light);
}

void cast_continual_dark(ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CONT_DARK,
                    tar_ch, tar_obj, spell_continual_dark);
}

void cast_blindness(ubyte level, struct char_data *ch, char *arg, int type,
		    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_BLINDNESS,
	     tar_ch, tar_obj, spell_blindness);
}

void cast_sunray(ubyte level, struct char_data *ch, char *arg, int type,
		 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SUNRAY,
	     tar_ch, tar_obj, spell_sunray);
}

void cast_paralyze(ubyte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_PARALYSIS,
	     tar_ch, tar_obj, spell_paralyze);
}

void cast_curse(ubyte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CURSE,
	     tar_ch, tar_obj, spell_curse);
}

void cast_faerie_fire(ubyte level, struct char_data *ch, char *arg, int type,
		      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FAERIE_FIRE,
	     tar_ch, tar_obj, spell_faerie_fire);
}

void cast_faerie_fog(ubyte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FAERIE_FOG,
	     tar_ch, tar_obj, spell_faerie_fog);
}

void cast_weakness(ubyte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_WEAKNESS,
	     tar_ch, tar_obj, spell_weakness);
}

void cast_web(ubyte level, struct char_data *ch, char *arg, int type,
	      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_WEB,
	     tar_ch, tar_obj, spell_web);
}

void cast_sleep(ubyte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_SLEEP,
	     tar_ch, tar_obj, spell_sleep);
}

void cast_cure_light(ubyte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CURE_LIGHT,
	     tar_ch, tar_obj, spell_cure_light);
}

void cast_cure_serious(ubyte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CURE_SERIOUS,
	     tar_ch, tar_obj, spell_cure_serious);
}

void cast_cure_critic(ubyte level, struct char_data *ch, char *arg, int type,
		        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CURE_CRITIC,
	     tar_ch, tar_obj, spell_cure_critic);
}

void cast_heal(ubyte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_HEAL,
	     tar_ch, tar_obj, spell_heal);
}

void cast_regen(ubyte level, struct char_data *ch, char *arg, int type,
                  struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_REGEN, tar_ch, tar_obj, spell_regen);
}

void cast_empathic_heal(ubyte level, struct char_data *ch, char *arg, int type,
		        struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_EMPATHIC_HEAL,
	     tar_ch, tar_obj, spell_empathic_heal);
}

void cast_astral_walk(ubyte level, struct char_data *ch, char *arg, int type,
		      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    spell_astral_walk(level, ch, type, tar_ch, 0);
}

void cast_astral_group(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ASTRAL_GROUP,
                    victim, tar_obj, spell_astral_group);
}

void cast_word_of_recall(ubyte level, struct char_data *ch, char *arg, int type,
		         struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_WORD_OF_RECALL,
	     tar_ch, tar_obj, spell_word_of_recall);
}

void cast_nature_walk(ubyte level, struct char_data *ch, char *arg, int type,
		      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if(!tar_ch) {
      send_to_char("Who do you wish to nature walk to?\n\r",ch);
      return;
    }

    spell_nature_walk(level, ch, type, tar_ch, 0);
}

void cast_teleport(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_TELEPORT,
             tar_ch, tar_obj, spell_teleport);
}

void cast_summon(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (type!=SPELL_TYPE_SPELL) {
      log_msg("screwup in summon");
      return;
    }

    spell_summon(level, ch, type, tar_ch, tar_obj);
}

void cast_monsum(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    struct char_data *mob;
    int vnum;
    char buffer[40];

    if(!CountFollowers(ch))
	return;

    one_argument(arg, buffer);

    if(!(vnum = FindPoly(monsum_list, buffer, GetMaxLevel(ch))))
    {
	send_to_char("Could not find anything like that.\n\r", ch);
	return;
    }

    if(!(mob = make_mobile(vnum, VIRTUAL)))
    {
	log_msg("bad mob vnum in monsum");
	return;
    }

    if (IS_SET(mob->specials.mob_act, ACT_HUGE) &&
        IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You attempt to summon a creature but this area is too small for it to fit!\n\r", ch);
        extract_char(mob);
        return;
    }

    spell_monsum(GetMaxLevel(ch), ch, type, mob, 0);
}


void cast_ansum(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    struct char_data *mob;
    int vnum;
    char buffer[40];
    struct room_data* rp;

    if(!CountFollowers(ch))
	return;

    rp=real_roomp(ch->in_room);
    switch (rp->sector_type) {
    case SECT_FIELD:
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
	break;
    default:
	send_to_char ("You must be in the wilderness to summon your animals friends.\n\r",ch);
	return;
    }

    one_argument(arg, buffer);

    if(!(vnum = FindPoly(ansum_list, buffer, GetMaxLevel(ch))))
    {
	send_to_char("Could not find anything like that.\n\r", ch);
	return;
    }

    if(!(mob = make_mobile(vnum, VIRTUAL)))
    {
	log_msg("bad mob vnum in ansum");
	return;
    }

    if (IS_SET(mob->specials.mob_act, ACT_HUGE) &&
        IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You attempt to summon a creature but this area is too small for it to fit!\n\r", ch);
        extract_char(mob);
        return;
    }

    spell_ansum(GetMaxLevel(ch), ch, type, mob, 0);
}

void cast_animate_dead(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_ANIMATE_DEAD,
             tar_ch, tar_obj, spell_animate_dead);
}

void cast_possession(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_POSSESSION,
             tar_ch, tar_obj, spell_possession);
}

#define RED_STONE       5233
#define PALE_BLUE_STONE 5230
#define CLEAR_STONE     5243
#define GREY_STONE      5239
void cast_conjure_elemental(ubyte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    char buffer[40];
    int mob, obj;
    struct obj_data *sac;
    struct char_data *el;

    one_argument(arg,buffer);

    if (!str_cmp(buffer, "fire")) {
      mob = VMOB_FIRE;
      obj = RED_STONE;
    } else if (!str_cmp(buffer, "water")) {
      mob = VMOB_WATER;
      obj = PALE_BLUE_STONE;
    } else if (!str_cmp(buffer, "air")) {
      mob = VMOB_AIR;
      obj = CLEAR_STONE;
    } else if (!str_cmp(buffer, "earth")) {
      mob = VMOB_EARTH;
      obj = GREY_STONE;
    } else {
      send_to_char("There are no elementals of that type available.\n\r", ch);
      return;
    }
    if (!ch->equipment[HOLD]) {
      send_to_char("You must be holding the correct elemental stone.\n\r",ch);
      return;
    }

    sac = unequip_char(ch, HOLD);
    if (sac) {
      obj_to_char(sac, ch);
      if (ObjVnum(sac) != obj) {
        send_to_char("You must have the correct elemental stone to do this.\n\r",ch);
        return;
      }
      el = make_mobile(mob, VIRTUAL);
      if (!el) {
        send_to_char("There are no elementals of that type available.\n\r",ch);
        return;
      }
    } else {
      send_to_char("You must be holding the correct elemental stone..\n\r",ch);
      return;
    }

    spell_conjure_elemental(level, ch, type, el, sac);
}

void cast_golem(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_GOLEM,
             tar_ch, tar_obj, spell_golem);
}

void cast_charm(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_CHARM,
             tar_ch, tar_obj, spell_charm);
}

void cast_shocking_grasp(ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_SHOCKING_GRASP,
             victim, tar_obj, spell_shocking_grasp);
}

void cast_electrocute(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ELECTROCUTE,
             victim, tar_obj, spell_electrocute);
}

void cast_electric_fire(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ELECTRIC_FIRE,
                    victim, tar_obj, spell_electric_fire);
}

void cast_chain_electrocution(ubyte level, struct char_data *ch, char *arg,
                        int type,
                        struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CHAIN_ELECTROCUTION,
                    victim, tar_obj, spell_chain_electrocution);
}

void cast_burning_hands(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_BURNING_HANDS,
                    victim, tar_obj, spell_burning_hands);
}

void cast_fire_wind(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FIRE_WIND,
                    victim, tar_obj, spell_fire_wind);
}

void cast_flamestrike(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FLAMESTRIKE,
                    victim, tar_obj, spell_flamestrike);
}

void cast_fireball(ubyte level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FIREBALL,
                    victim, tar_obj, spell_fireball);
}

void cast_lava_storm(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_LAVA_STORM,
                    victim, tar_obj, spell_lava_storm);
}

void cast_chill_touch(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CHILL_TOUCH,
                    victim, tar_obj, spell_chill_touch);
}

void cast_frost_cloud(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FROST_CLOUD,
                    victim, tar_obj, spell_frost_cloud);
}

void cast_ice_storm(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ICE_STORM,
                    victim, tar_obj, spell_ice_storm);
}

void cast_harmful_touch(ubyte level, struct char_data *ch, char *arg, int type,
                        struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_HARMFUL_TOUCH,
                    victim, tar_obj, spell_harmful_touch);
}

void cast_decay(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_WITHER,
                    victim, tar_obj, spell_decay);
}

void cast_rupture(ubyte level, struct char_data *ch, char *arg, int type,
                  struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_RUPTURE,
                    victim, tar_obj, spell_rupture);
}

void cast_implode(ubyte level, struct char_data *ch, char *arg, int type,
                  struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_IMPLODE,
                    victim, tar_obj, spell_implode);
}

void cast_disintegrate(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_DISINTEGRATE,
                    victim, tar_obj, spell_disintegrate);
}

void cast_acid_blast(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ACID_BLAST,
                    victim, tar_obj, spell_acid_blast);
}

void cast_acid_rain(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ACID_RAIN,
                    victim, tar_obj, spell_acid_rain);
}

void cast_energy_drain(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ENERGY_DRAIN,
                    victim, tar_obj, spell_energy_drain);
}

void cast_vampyric_touch(ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_VAMPYRIC_TOUCH,
                    victim, tar_obj, spell_vampyric_touch);
}

void cast_poison(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_POISON,
                    victim, tar_obj, spell_poison);
}

void cast_poison_gas(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_POISON_GAS,
                    victim, tar_obj, spell_poison_gas);
}

void cast_wind_storm(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_WIND_STORM,
                    victim, tar_obj, spell_wind_storm);
}

void cast_geyser(ubyte level, struct char_data *ch, char *arg, int type,
                 struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_GEYSER,
                    victim, tar_obj, spell_geyser);
}

void cast_earthquake(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_EARTHQUAKE,
                    victim, tar_obj, spell_earthquake);
}


void cast_call_lightning(ubyte level, struct char_data *ch, char *arg, int type,
                         struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CALL_LIGHTNING,
                    victim, tar_obj, spell_call_lightning);
}


void cast_thorn(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_THORN,
                    victim, tar_obj, spell_thorn);
}

void cast_vine(ubyte level, struct char_data *ch, char *arg, int type,
               struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_VINE,
                    victim, tar_obj, spell_vine);
}

void cast_creep(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CREEPING_DOOM,
                    victim, tar_obj, spell_creep);
}

void cast_camouflage(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CAMOUFLAGE,
                    victim, tar_obj, spell_camouflage);
}

void cast_control_weather(ubyte level,struct char_data *ch, char *arg, int type,
                          struct char_data *tar_ch, struct obj_data *tar_obj)
{
    char buffer[MAX_STRING_LENGTH];

    switch (type) {
    case SPELL_TYPE_SPELL:

        one_argument(arg,buffer);

        if (str_cmp("better",buffer) && str_cmp("worse",buffer))
        {
            send_to_char("Do you want it to get better or worse?\n\r",ch);
            return;
        }
        if (!OUTSIDE(ch)) {
            send_to_char("You need to be outside.\n\r",ch);
        }

        if(!str_cmp("better",buffer)) {
            if (weather_info.sky == SKY_CLOUDLESS)
                return;
            if (weather_info.sky == SKY_CLOUDY) {
                send_to_outdoor("The clouds disappear.\n\r");
                weather_info.sky=SKY_CLOUDLESS;
            }
            if (weather_info.sky == SKY_RAINING) {
                if ((time_info.month>3)&&(time_info.month < 11))
                    send_to_outdoor("The rain has stopped.\n\r");
                else
                    send_to_outdoor("The snow has stopped. \n\r");
                weather_info.sky=SKY_CLOUDY;
            }
            if (weather_info.sky == SKY_LIGHTNING) {
                if ((time_info.month>3)&&(time_info.month<11))
                    send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
                else
                    send_to_outdoor("The blizzard is over, but it is still snowing.\n\r");
                weather_info.sky=SKY_RAINING;
            }
            return;
        } else {
            if (weather_info.sky == SKY_CLOUDLESS) {
                send_to_outdoor("The sky is getting cloudy.\n\r");
                weather_info.sky=SKY_CLOUDY;
                return;
            }
            if (weather_info.sky == SKY_CLOUDY) {
                if ((time_info.month > 3) && (time_info.month < 11))
                    send_to_outdoor("It starts to rain.\n\r");
                else
                    send_to_outdoor("It starts to snow. \n\r");
                weather_info.sky=SKY_RAINING;
            }
            if (weather_info.sky == SKY_RAINING) {
                if ((time_info.month> 3)&&(time_info.month <= 11))
                    send_to_outdoor("You are caught in lightning storm.\n\r");
                else
                    send_to_outdoor("You are caught in a blizzard. \n\r");
                weather_info.sky=SKY_LIGHTNING;
            }
            if (weather_info.sky == SKY_LIGHTNING) {
                return;
            }

            return;
        }
        break;

    default:
        log_msg("Serious screw-up in control weather!");
        break;
    }
}

void cast_fly_group(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FLY_GROUP,
                    victim, tar_obj, spell_fly_group);
}

void cast_armor_group(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ARMOR_GROUP,
                    victim, tar_obj, spell_armor_group);
}

void cast_invis_group(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_INVIS_GROUP,
                    victim, tar_obj, spell_invis_group);
}

void cast_dinvis_group(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_DINVIS_GROUP,
                    victim, tar_obj, spell_dinvis_group);
}

void cast_true_seeing_group(ubyte level, struct char_data *ch, char *arg, int type,
                            struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_TRUE_SEEING_GROUP,
                    victim, tar_obj, spell_true_seeing_group);
}

void cast_heal_group(ubyte level, struct char_data *ch, char *arg,
                    int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_HEAL_GROUP,
                    victim, tar_obj, spell_heal_group);
}

void cast_cure_light_group(ubyte level, struct char_data *ch, char *arg,
                    int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_CURE_LIGHT_GROUP,
                    victim, tar_obj, spell_cure_light_group);
}

void cast_waterbreath_group(ubyte level, struct char_data *ch, char *arg,
                    int type,
                    struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_WATERBREATH_GROUP,
                    victim, tar_obj, spell_waterbreath_group);
}

void cast_recall_group(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_RECALL_GROUP,
                    victim, tar_obj, spell_recall_group);
}

// Fire breath potion
void cast_fire_breath_aff(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FIRE_BREATH,
                    victim, tar_obj, spell_fire_breath_aff);
}

void cast_frost_breath_aff(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FROST_BREATH,
                    victim, tar_obj, spell_frost_breath_aff);
}

void cast_acid_breath_aff(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ACID_BREATH,
                    victim, tar_obj, spell_acid_breath_aff);
}

void cast_poison_gas_breath_aff(ubyte level, struct char_data *ch, char *arg,
                      int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_POISON_GAS_BREATH,
                    victim, tar_obj, spell_poison_gas_breath_aff);
}

void cast_lightning_breath_aff(ubyte level, struct char_data *ch, char *arg,
                      int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_LIGHTNING_BREATH,
                    victim, tar_obj, spell_lightning_breath_aff);
}
// end fire breath potion

void cast_fire_breath(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FIRE_BREATH,
                    victim, tar_obj, spell_fire_breath);
}

void cast_frost_breath(ubyte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_FROST_BREATH,
                    victim, tar_obj, spell_frost_breath);
}

void cast_acid_breath(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_ACID_BREATH,
                    victim, tar_obj, spell_acid_breath);
}

void cast_poison_gas_breath(ubyte level, struct char_data *ch, char *arg,
                      int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_POISON_GAS_BREATH,
                    victim, tar_obj, spell_poison_gas_breath);
}

void cast_lightning_breath(ubyte level, struct char_data *ch, char *arg,
                      int type,
                      struct char_data *victim, struct obj_data *tar_obj )
{
    do_spell(level, ch, type, SPELL_LIGHTNING_BREATH,
                    victim, tar_obj, spell_lightning_breath);
}

void cast_ward_fire(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
   do_spell(level, ch, type, SPELL_WARD_FIRE,
            tar_ch, tar_obj, spell_ward_fire);
}

void cast_ward_cold(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
   do_spell(level, ch, type, SPELL_WARD_COLD,
            tar_ch, tar_obj, spell_ward_cold);
}

void cast_ward_elec(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
   do_spell(level, ch, type, SPELL_WARD_ELEC,
            tar_ch, tar_obj, spell_ward_elec);
}

void cast_ward_energy(ubyte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj)
{
   do_spell(level, ch, type, SPELL_WARD_ENERGY,
            tar_ch, tar_obj, spell_ward_energy);
}

void cast_ward_acid(ubyte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
   do_spell(level, ch, type, SPELL_WARD_ACID,
            tar_ch, tar_obj, spell_ward_acid);
}

void cast_stone_fist(ubyte level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
  do_spell(level, ch, type, SPELL_STONE_FIST,
           tar_ch, tar_obj, spell_stone_fist);
}

void cast_lullabye(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
       do_spell(level, ch, type, SPELL_LULLABYE,
            tar_ch, tar_obj, spell_lullabye);
}

void cast_haste(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
       do_spell(level, ch, type, SPELL_HASTE,
            tar_ch, tar_obj, spell_haste);
}

void cast_slow(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
       do_spell(level, ch, type, SPELL_SLOW,
            tar_ch, tar_obj, spell_slow);
}

void cast_despair(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
       do_spell(level, ch, type, SPELL_DESPAIR,
            tar_ch, tar_obj, spell_despair);
}
void cast_inspire(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
       do_spell(level, ch, type, SPELL_INSPIRE,
            tar_ch, tar_obj, spell_inspire);
}

void cast_blur(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
      do_spell(level, ch, type, SPELL_BLUR,
           tar_ch, tar_obj, spell_blur);
}

void cast_terror(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
      do_spell(level, ch, type, SPELL_TERROR,
           tar_ch, tar_obj, spell_terror);
}
void cast_silence(ubyte level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
      do_spell(level, ch, type, SPELL_SILENCE,
           tar_ch, tar_obj, spell_silence);
}

void cast_friends(ubyte level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_FRIENDS,
             tar_ch, tar_obj, spell_friends);
}

void cast_unweave(ubyte level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    do_spell(level, ch, type, SPELL_UNWEAVE,
             tar_ch, tar_obj, spell_unweave);
}

void cast_enforest(ubyte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj) {
   do_spell(level, ch, type, SPELL_ENFOREST,
	    tar_ch, tar_obj, spell_enforest);
}

void cast_gust(ubyte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj) {
   do_spell(level, ch, type, SPELL_GUST,
	    tar_ch, tar_obj, spell_gust);
}

void cast_inferno(ubyte level, struct char_data *ch, char *arg, int type,
		  struct char_data *tar_ch, struct obj_data *tar_obj) {
   do_spell(level, ch, type, SPELL_INFERNO,
	    tar_ch, tar_obj, spell_inferno);
}
