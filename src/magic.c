#include "config.h"
#include <stdio.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "time.h"
#include "utility.h"
#include "fight.h"
#include "spell_util.h"
#include "multiclass.h"
#include "act.h"
#include "trap.h"
#include "magicutils.h"
#include "opinion.h"
#include "constants.h"
#include "newsaves.h"
#include "spelltab.h"
#include "util_str.h"
#include "recept.h"
#include "race.h"
#include "periodic.h"
#include "spell_procs.h"
#include "poly.h"
#include "vnum.h"
#include "find.h"
#include "statistic.h"
#include "vnum_mob.h"
#include "proto.h"
#include "cmdtab.h"
#include "interpreter.h"
#include "spell_events.h"

/* following are the modifieres for the spells, based on the level they are provided to the classes */
const int webSpellMod = 2;
const int weaknessSpellMod = 0;
const int curseSpellMod = 0;
const int faeriefireSpellMod= 0;
const int paraSpellMod = 4;
const int blindnessSpellMod = 0;
const int sleepSpellMod = 4;
const int turnSpellMod = 0;
const int terrorSpellMod = -1;
const int fearSpellMod = 0;
const int silenceSpellMod = -1;
const int shockSpellMod = 0;
const int elecSpellMod = 0;
const int elecFireSpellMod = 0;
const int burningHandsSpellMod = 0;
const int fireWindSpellMod = 0;
const int flamestrikeSpellMod = -1;
const int fireballSpellMod = -1;
const int chillTouchSpellMod = 0;
const int frostCloudSpellMod = 0;
const int harmfulTouchSpellMod =0 ;
const int decaySpellMod = 0;
const int ruptureSpellMod = -1;
const int implodeSpellMod =-1 ;
const int disintegrateSpellMod = -2;
const int acidBlastSpellMod = 0;
const int vampyricSpellMod = -2;
const int poisonSpellMod = 4;
const int callLightningSpellMod =0 ;
const int thornSpellMod =0 ;
const int vineSpellMod = 0;
const int dispellSpellMod = -2;
const int energyDrainSpellMod = -1;
/*** support routines ***/

void fix_mob_bits(struct char_data *mob);

/*  The following routine calculates the chance of a spell missing the target.
    It is based on level alone.  If levels are equal the chance is 65% chance
    of hitting.  If they are 35+ level between caster and target then the
    chance is 99%.  If the difference is -65 then chance is 0%  This is
    to allow for targeted spells to miss. 

    UPDATE - Novak: Obsolute, now we are using Saving Throws to determine
    if someone is hit by a spell. See spell_parser.c and ImpSaveSpell
    to see how this is done.

int Chance_Spell_Hit(struct char_data *ch, struct char_data *victim, int chaos)
{
    int chance = chaos;

    chance = GetMaxLevel(ch) - GetMaxLevel(victim);
    chance += 65;

    chance = MIN(chance, 100);

    if(number(1,101) < chance)
       return 1;
    else
       return 0;
}
*/     
void MakeAffect(struct char_data* caster, struct char_data* target,
		int spell_type,
		int aff_type, int modifier, int location, int bitvector,
		int duration, int mana_cost,
		bool add_affect, bool avg_dur, bool avg_mod,
                expire_proc proc)
{
    struct affected_type af;
    
    af.type = aff_type;
    af.modifier = modifier;
    af.location = location;
    af.bitvector = bitvector;
    af.expire_proc_pointer = proc;
    af.save_bonus = 0;
    
    if(duration && mana_cost)
    {
      if(spell_type == SPELL_TYPE_SPELL) {
	duration = 0;
      } else {
	mana_cost = 0;
	caster = 0;
      }
    }
    
    af.duration = duration;
    af.mana_cost = mana_cost;
    af.caster = caster;
    
    if(add_affect)
    {
      DLOG(("Calling affect_to_char from MakeAffect magic.c line 103\r\n"));
      affect_to_char(target, &af);
    }
    else
      affect_join(target, &af, avg_dur, avg_mod);
}

void MakeCharmed(struct char_data* caster, struct char_data* victim,
		 int level, int spell_type, int bonus)
{
    struct affected_type *af;
    
    /* create and initialize the affect */
    CREATE(af, struct affected_type, 1);
    af->type = spell_type;
    af->duration = (GetMaxLevel(caster) - GET_INT(victim)) / 2;
    if (af->duration < 2) 
       af->duration = 2;
    af->save_bonus = bonus;
    af->modifier = 0;
    af->location = 0;
    af->bitvector = AFF_CHARM;
    af->mana_cost = CHARM_COST(victim, caster);
    af->caster = caster;
    af->holder = victim;

    /* put it in the affect list */
    af->next = victim->affected;
    victim->affected = af;

    /* actually apply it */
    affect_modify(victim, af->location, af->modifier, af->bitvector, TRUE);

    /* queue the event */
    event_queue_pulse(&af->timer,
		      next_pulse(0),
		      (event_func) affect_event,
		      "charm");

    /* make sure the charmee doesn't still hate the charmer */
    if(!IS_PC(victim))
      RemHated(victim, caster);
}

int wilderness(struct char_data *ch) 
{
   switch (real_roomp(ch->in_room)->sector_type) {
      case SECT_FIELD:
      case SECT_FOREST:
      case SECT_HILLS:
      case SECT_MOUNTAIN:
      case SECT_DESERT:
         return (TRUE);
      default:
         return (FALSE);
   }
}

int is_hold_instrument(struct char_data *ch, int spell_number) {
  struct obj_data *obj;
  struct spell_info *spell;
  char buffer[MAX_STRING_LENGTH];

  /* make the table of instrument numbers here */
  int MAX_INSTRUMENTS=10;
  int instruments[]={
  32311,              /*flute 1-10*/
  32312,              /*Harp 11-20*/
  32313,              /*Mandolin 21-30*/
  32314,              /*Lute 31-40*/
  32315,              /*Lyre 41-50*/
  32316,              /*Stratocaster 51-60?*/
  6705,               /* humming yabree */
  6720,               /* a black wand */
  6722,               /* Brown ornate staff */
  6778,               /* The Orb of Sounds */
  };
  int level_of_instrument=0;

   /*find out what the player is holding, and if its an instrument*/

  obj=ch->equipment[17];

  if (!obj) {
     send_to_char("You must hold an instrument to play your songs!\n\r",ch);
     return (FALSE);
  }

  for (int inst_list=0;inst_list != (MAX_INSTRUMENTS-1); inst_list++) {
    if (obj_index[(obj->item_number)].virt == instruments[inst_list]) {
      level_of_instrument=inst_list;
    }
  }
  if (level_of_instrument == 0) {
    sprintf(buffer,"You cannot play on %s!!!\n\r",ss_data(obj->short_description));
    act(buffer,TRUE,ch,0,0,TO_CHAR);
    return FALSE;
  }

  spell=spell_by_number(spell_number);
  if (spell->min_level[BARD_LEVEL_IND] <= 10 ||
               level_of_instrument > 0) {
    return (TRUE);
  }
  else if (spell->min_level[BARD_LEVEL_IND] <= 20 ||
               level_of_instrument > 1) {
    return (TRUE);
  }
  else if (spell->min_level[BARD_LEVEL_IND] <= 30 ||
               level_of_instrument > 2) {
    return (TRUE);
  }
  else if (spell->min_level[BARD_LEVEL_IND] <= 40 ||
               level_of_instrument > 3) {
    return (TRUE);
  }
  else if (spell->min_level[BARD_LEVEL_IND] <= 50 ||
               level_of_instrument > 4) {
    return (TRUE);
  }
  else if (spell->min_level[BARD_LEVEL_IND] <= 60 ||
               level_of_instrument > 5) {
    return (TRUE);
  }
  else {
    sprintf(buffer,"%s is not the correct instrument for that song!\n\r",ss_data(obj->name));
    act(buffer,TRUE,ch,0,0,TO_CHAR);
    return (FALSE);
  }
}





#ifdef COMMENTEDOUT
/*** all successful poison spells have the extra capacity of stunning ***/
/*** the target, besides just poisoning them.                         ***/
void poison_effect(ubyte level, struct char_data *ch, struct char_data *victim)
{
  char buf[256];
  int diceroll, percenthit;

  diceroll = number(1,99);
  percenthit = GetMaxLevel(ch) - GetMaxLevel(victim);
  if (percenthit < 1)
    percenthit = 5;
  if (percenthit < diceroll)
    if (NewSkillSave(ch, victim, SPELL_POISON, 0, IMM_POISON))
      if (!saves_spell(victim, SAVING_SPELL, IMM_HOLD))
 	return;


  send_to_char_formatted("$CrDeadly poison runs through your body!$CN\n\r",
			 victim); 
  act("$Cr$n is poisoned!$CN",TRUE,victim,0,0,TO_ROOM);
  MakeAffect(0, victim, SPELL_TYPE_POTION,
	     SPELL_POISON, -1, APPLY_STR, AFF_POISON,
	     level, 0, TRUE, FALSE, FALSE, NULL);


  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (!saves_spell(victim, SAVING_SPELL, IMM_HOLD))
      if (!saves_spell(victim, SAVING_SPELL, IMM_POISON))
	if (GET_CON(victim) > percenthit)
	  { 
	    send_to_char_formatted("$CRThe poison overcomes you!$CN", victim);
	    MakeAffect(0, victim, SPELL_TYPE_POTION,
		       SPELL_PARALYSIS, 0, APPLY_NONE, AFF_PARALYSIS,
		       0, 0, FALSE, FALSE, FALSE, NULL);
	    act("$CR$n is paralyzed by the poison!$CN",
		TRUE,victim,0,0,TO_ROOM);
	    victim->specials.position=POSITION_STUNNED;
	  }
  }
}

#endif

/*** all successful poison spells have the extra capacity of stunning ***/
/*** the target, besides just poisoning them.                         ***/
void poison_effect(ubyte level, struct char_data *ch, struct char_data *victim)
{
  int percenthit;
  int dam, len;

  // If they save against poison, they are fine 
  if (ImpSaveSpell(victim, SAVING_PARA, 4))
	 return;

  //This is cumulative damage!
  dam = (int)((MAX_MORT/4)*sqrt((double)level/MAX_MORT));
   if(HasClass(ch, CLASS_CLERIC)) {
     if(IS_PURE_CLASS(ch))
       dam *= 5;
     else
       dam *= 2;
  }
  dam = ApplyImmuneToDamage(victim, dam, IMM_POISON);
  len=level/50+1;
   
  send_to_char_formatted("$CrDeadly poison runs through your body!$CN\n\r",
			 victim);
  act("$Cr$n is poisoned!$CN",TRUE,victim,0,0,TO_ROOM);
  MakeAffect(0, victim, SPELL_TYPE_POTION,
	     SPELL_POISON, dam, APPLY_DRAIN_LIFE, AFF_POISON,
	     len, 0, FALSE, TRUE, FALSE, NULL);

  //If they are immune Poison, they cannot be stunned
  if (!IsImmune(victim, IMM_POISON)) {
  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) { 
   if (ImpSaveSpell(victim, SAVING_PARA, 5))
        return;
    if (percent() < MAX(20 - GET_CON(victim), 1)) /* poison time */
      { 
	send_to_char_formatted("$CRThe poison overcomes you!$CN", victim);
	MakeAffect(0, victim, SPELL_TYPE_POTION,
		   SPELL_PARALYSIS, 0, APPLY_NONE, AFF_PARALYSIS,
		   1, 0, FALSE, FALSE, FALSE, NULL);
	act("$CR$n is paralyzed by the poison!$CN",
		           TRUE,victim,0,0,TO_ROOM);
	victim->specials.position=POSITION_STUNNED;
      }
  }
  }
}


/*** all successful cold spells have the extra capacity of freezing the ***/
/*** target this is simulated by reducing the strength of the victim by ***/ 
/*** one point. ***/
void cold_effect(ubyte level, struct char_data *victim)
{
  if (ImpSaveSpell(victim, SAVING_SPELL,0))
    return;

  send_to_char_formatted("$CBYou are frozen by the biting cold!$CN\n\r", victim);
  act("$CB$n's strength is sapped by the biting cold!$CN",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(0, victim, SPELL_TYPE_POTION,
             SPELL_WEAKNESS, -3, APPLY_STR, 0,
             level, 0, TRUE, FALSE, FALSE, NULL);
}

void wind_effect(ubyte level, struct char_data *victim)
{
  if (saves_spell(victim, SAVING_PARA, 0))
    return;

  send_to_char_formatted("Y$CMou're knocked to the ground by a blast of wind!\n\r$CN", victim);
  act ("$CM$n is knocked to the ground by a blast of wind!$CN",TRUE,victim,0,0,TO_ROOM);

  GET_POS(victim)=POSITION_SITTING;
}

/***************************/
/*** generic area affect ***/
/***************************/
void area_affect(int level, int dam, struct char_data *ch,
                 long spell, long imm_type,
                 char mes1[MAX_STRING_LENGTH], char mes2[MAX_STRING_LENGTH])
{
  struct char_data *vict;
  int under_water_electric_attack = FALSE;
    
  act(mes1, FALSE, ch, 0, 0, TO_CHAR);
  act(mes1, TRUE, ch, 0, 0, TO_ROOM);
    
  /* Only really care if under water if is an Electric attack, due
     to electricities conductivity in water, Chance to hit everyone */

  if (imm_type == IMM_ELEC)
    under_water_electric_attack = UNDERWATER(ch);

  EACH_CHARACTER(iter, vict)
    {
      if (ch->in_room==vict->in_room)
	{
	  if(can_hurt(ch,vict))
	    if(!ImpSaveSpell(vict, SAVING_BREATH, -2))
	      {
		if (!damage(ch, vict, dam, spell))
		  {
		    if (spell==SPELL_ICE_STORM)
		      cold_effect (level, vict);
		    else if (spell==SPELL_WIND_STORM)
		      wind_effect (level, vict);
		    else if (spell==SPELL_POISON_GAS)
		      poison_effect(level, ch,vict);
		    mprog_spell_trigger(vict, ch, spell);
		  }
	      }
	} else if(real_roomp(ch->in_room)->zone ==
		  real_roomp(vict->in_room)->zone)
	  act(mes2, FALSE, vict, 0, 0, TO_CHAR);
    }
  END_AITER(iter);
}

// Monk charge elements skill stuff:
// This adds a 5-second "residue" to the caster
// that a monk may pick up & absorb with the
// charge elements skill.

static int add_charge_expire(spevent_type*,long);

void add_charge_spell(struct char_data *ch, struct char_data *vict, int spnum) {
   spevent_list *slist, *snext;
   SPEVENT_CHECK_CHAR(slist,snext,ch,spnum) {
      SPEVENT_CAST(slist,snext,ch,spnum);
      
      return;
   }
   
   spevent_type *se;
   
   se = spevent_new();
   
   se->caster = ch;
   se->victim = vict;
   se->type = spnum;
   se->duration = 5;
   se->expire_func = (event_func) add_charge_expire;
   
   spevent_depend_char(se, ch);
   spevent_start(se);
}

//The residual energy from the spell has expired.
int add_charge_expire(spevent_type *se, long now) {
   spevent_destroy(se);
   return 0;
}

/*********************/
/*** create spells ***/
/*********************/
void spell_light (ubyte level, struct char_data *ch, int type,
                 struct char_data *victim,  struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);

  /*  if (!(tmp_obj = make_object(BALL_OF_LIGHT, NORAND))) {*/
  if (!(tmp_obj = make_object(BALL_OF_LIGHT, VIRTUAL|NORAND))) {
    log_msg("screwup in create light spell.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  if (HasClass(ch,CLASS_BARD)) {
    if (!is_hold_instrument(ch,SPELL_LIGHT)) {
      return;
    }
    act("$n's song takes on solidarity and form - it colasces into $p.",TRUE,ch,tmp_obj,0,TO_ROOM);
    act("Your song becomes solid, and colasces into $p.",
         FALSE,ch,tmp_obj,0,TO_CHAR);
  }
  else {
    act("$n waves $s hands and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
    act("You wave your hands and $p suddenly appears.",
       FALSE,ch,tmp_obj,0,TO_CHAR);
  }
  
  tmp_obj->obj_flags.value[2] = -1;  /* eternal light */
  obj_to_char(tmp_obj,ch);

}

void spell_moonbeam(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim,  struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);

  if (!OUTSIDE(ch)) {
    send_to_char("You must be outdoors to call upon the moon.\n\r",ch);
    return;
  }

  if (!weather_info.sunlight==SUN_DARK) {
    send_to_char("The moon is not out to help you.\n\r",ch);
    return;
  }

  /*  if (!(tmp_obj = make_object(MOONBEAM_LIGHT, NORAND))) {*/
  if (!(tmp_obj = make_object(MOONBEAM_LIGHT, VIRTUAL|NORAND))) {
    log_msg("screwup in moonbeam spell.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  tmp_obj->obj_flags.value[2] = -1;  /* eternal light */
  obj_to_char(tmp_obj,ch);

  send_to_char("You creates a lantern from beams of moonlight.\n\r",ch);
  act("$n creates a lantern from beams of moonlight.",TRUE,ch,0,0,TO_ROOM);
}

void spell_create_food(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
  

  /*  if(!(tmp_obj=make_object(MUSHROOM, NORAND))) {*/
  if(!(tmp_obj=make_object(MUSHROOM, VIRTUAL|NORAND))) {
    log_msg("screwup in create food.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  tmp_obj->obj_flags.value[0]=MAX(5, level/2); /* 5 to 25 hours */
  tmp_obj->obj_flags.value[3]=0; /* not poisoned */
  obj_to_char(tmp_obj,ch); 

  act("$n waves $s hands and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You wave your hands and $p suddenly appears.",FALSE,ch,tmp_obj,0,TO_CHAR);
}

void spell_goodberry(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
  

  if (!wilderness(ch)) {
      send_to_char ("You are not near any fruit bearing plants.\n\r",ch);
      return;
  }

  if(!(tmp_obj=make_object(GOODBERRY, VIRTUAL|NORAND))) {
    log_msg("screwup in goodberry.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  tmp_obj->obj_flags.value[0]=MAX(5, level); /* 5 to 50 hours */
  tmp_obj->obj_flags.value[3]=0; /* not poisoned */
  obj_to_char(tmp_obj,ch);

  send_to_char("You wave your hands and receive a special berry from a fruit bearing plant.\n\r",ch);
  act("$n waves $s hands and a fruit bearing plant offers a special berry.",TRUE,ch,0,0,TO_ROOM);
}

void spell_create_water(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
  

  if(!(tmp_obj=make_object(WATER_BARREL, VIRTUAL|NORAND))) {
    log_msg("screwup in create water.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  tmp_obj->obj_flags.value[0]=tmp_obj->obj_flags.value[1]=MAX(10,level);
  tmp_obj->obj_flags.value[2]=0; /* water */
  obj_to_char(tmp_obj,ch);

  act("$n waves $s hands and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You wave your hands and $p suddenly appears.",FALSE,ch,tmp_obj,0,TO_CHAR);
}

void spell_succor(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);

  if(!(tmp_obj=make_object(SCROLL_RECALL, VIRTUAL|NORAND))) {
    log_msg("screwup in succor.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  obj_to_char(tmp_obj,ch);

  act("$Cy$n waves $s hands and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$CyYou wave your hands and $p suddenly appears.",FALSE,ch,tmp_obj,0,TO_CHAR); 
}

void spell_knowledge(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);

  if(!(tmp_obj=make_object(SCROLL_IDENTIFY, VIRTUAL|NORAND))) {
    log_msg("screwup in knowledge.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  obj_to_char(tmp_obj,ch);

  act("$Cy$n waves $s hands and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$CyYou wave your hands and $p suddenly appears.",FALSE,ch,tmp_obj,0,TO_CHAR); 
}

void spell_clone(ubyte level, struct char_data *ch, int type,
                 struct char_data *vict, struct obj_data *obj)
{
    static FILE* clone_log = 0;
    int success, limit, count;

    assert(ch && (vict || obj));
    

    if (obj) {

        struct obj_data* clone;

        if(IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
          send_to_char("This spell can't be cast while polied.\n\r", ch);
          return;
        }

        if(GET_HIT(ch)<GET_MAX_HIT(ch) || GET_MANA(ch)<GET_MAX_MANA(ch)) {
          send_to_char("You must be at full health for this spell.\n\r", ch);
          return;
        }

        if(GET_CON(ch) <= 3) {
            send_to_char("You aren't healthy enough for this spell.\n\r", ch);
            return;
        }

        limit = obj_index[obj->item_number].limit;
        count = obj_index[obj->item_number].number;
        if(limit == -1)
            success = 5;
        else if(limit == ONCE_PER_REBOOT)
            success = 25;
        else if(limit < count)
            success = 5;
        else
            success = (limit - count) * 70 / limit;

        GET_HIT(ch) = 1;
        GET_MANA(ch) = 1;
        ch->abilities.con--;

        if(number(1,100) > success) {
            if(clone_log)
                fprintf(clone_log, "Clone: %s destroys %s, %d/%d=%d%%\n",
                        GET_NAME(ch), OBJ_NAME(obj),
                        count, limit, success);

            act("$n takes out some twine and bubble gum and destroys $p!",
                TRUE, ch, obj, 0, TO_ROOM);
            act("You fail to clone $p and destroy it!",
                TRUE, ch, obj, 0, TO_CHAR);

            obj_from_char(obj);
            extract_obj(obj);

        } else {

            if(clone_log)
                fprintf(clone_log, "Clone: %s clones %s, %d/%d=%d%%\n",
                        GET_NAME(ch), OBJ_NAME(obj),
                        count, limit, success);

            if(!(clone = clone_object(obj)))
            {
                send_to_char("Serious error in clone_object\n", ch);
                return;
            }

            act("Out of twine and bubble gum, $n duplicates $p!",
                TRUE, ch, obj, 0, TO_ROOM);
            act("You clone $p!", TRUE, ch, obj, 0, TO_CHAR);

            obj_to_char(clone, ch);
        }

    } else if (vict) {

        char buf[256];
        sprintf(buf, "%s tried to clone %s\n", GET_NAME(ch), GET_NAME(vict));
        log_msg(buf);
        send_to_char("Sorry, can't clone people yet.\n\r", ch);
    }
}

void spell_creation(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  assert(ch && obj);

  obj_to_char(obj,ch);

  act("$n waves $s hands and $p suddenly appears.",TRUE,ch,obj,0,TO_ROOM);
  act("You wave your hands and $p suddenly appears.",FALSE,ch,obj,0,TO_CHAR);
}

/*********************************************/
/*** vision, detect, and associated spells ***/
/*********************************************/

void spell_infravision(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  

  if (IS_AFFECTED(victim, AFF_INFRAVISION)) {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
  }

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  MakeAffect(ch, victim, type,
	     SPELL_INFRAVISION, 0, APPLY_NONE, AFF_INFRAVISION,
	     level, 1, FALSE, FALSE, FALSE, NULL);
  
  send_to_char_formatted("$CrYour eyes glow red.\n\r", victim);
  act("$Cg$n's$Cr eyes glow red.\n\r", FALSE, victim, 0, 0, TO_ROOM);
}

void spell_invisibility(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  assert((ch && obj) || victim);
  
  if (type == SPELL_TYPE_POTION) {
    level = level / 40;
  }

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
      act("$Cw$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
      act("$Cw$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
    return;
  }

  if (IS_AFFECTED(victim, AFF_INVISIBLE)) {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return; 
  }

  act("$Cw$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
  send_to_char_formatted("$CwYou vanish.\n\r", victim);

  MakeAffect(ch, victim, type,
	     SPELL_INVISIBLE, 0, 0, AFF_INVISIBLE,
	     level * 2, 1, FALSE, FALSE, FALSE, NULL);
}

void spell_detect_invisibility(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  
  if (type == SPELL_TYPE_POTION) {
    level = level / 10;
  }

  if (IS_AFFECTED(victim, AFF_DETECT_INVISIBLE)) {
    act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);
    return;
  }

  MakeAffect(ch, victim, type,
	     SPELL_DETECT_INVISIBLE, 0, APPLY_NONE, AFF_DETECT_INVISIBLE,
	     level / 2, 1, FALSE, FALSE, FALSE, NULL);

  act("$CY$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char_formatted("$CYYour eyes tingle.\n\r", victim);
}

void spell_sense_life(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert(ch);
  
  if (type == SPELL_TYPE_POTION) {
    level = level / 10;
  }

  if (IS_AFFECTED(ch, AFF_SENSE_LIFE)) {
    act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);
    return;
  }

  MakeAffect(ch, victim, type,
	     SPELL_SENSE_LIFE, 0, APPLY_NONE, AFF_SENSE_LIFE,
	     level / 2, 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("Your feel your awareness improve.\n\r", ch);
  act("$n seems to be more aware.\n\r", FALSE, victim, 0, 0, TO_ROOM);
}

void spell_true_seeing(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  
  if (IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
    act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);
    return;
  }

  if (type == SPELL_TYPE_POTION) {
    level = level / 10;
  }

  MakeAffect(ch, victim, type,
	     SPELL_TRUE_SIGHT, 0, APPLY_NONE, AFF_TRUE_SIGHT,
	     level / 2, 2, FALSE, FALSE, FALSE, NULL);

  send_to_char("Your eyes glow silver.\n\r", victim);
  act("$n's eyes glow silver.\n\r", FALSE, victim, 0, 0, TO_ROOM);
}

void spell_terror(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (!is_hold_instrument(ch,SPELL_TERROR)) {
    return;
  }

  if (IsUndead(victim))
    return;
  
  if (!ImpSaveSpell(victim, SAVING_PETRI,terrorSpellMod)) {
    act("$n flees in terror!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You flee in terror!",victim);
    do_flee(victim,"",0);
    return;
  }
  
}


void spell_sense_aura(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  

  if (IS_AFFECTED(victim, AFF_SENSE_AURA)) {
    act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);
    return;
  }

  if (type == SPELL_TYPE_POTION) {
    level = level / 10;
  }

  MakeAffect(ch, victim, type,
	     SPELL_SENSE_AURA, 0, APPLY_NONE, AFF_SENSE_AURA,
	     level / 2, 1, FALSE, FALSE, FALSE, NULL);

  send_to_char_formatted("$CyYour eyes blur and then refocus.\n\r", victim);
  act("$n's eyes blur and then refocus.\n\r", FALSE, victim, 0, 0, TO_ROOM);
}

void spell_detect_poison(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood!\n\r", ch);
      else
        send_to_char("You sense no poison.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $N is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
        act("You sense that $N is not poisoned.",FALSE,ch,0,victim,TO_CHAR);
      }
    return;
  }

  if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
    (obj->obj_flags.type_flag == ITEM_FOOD)) {
    if (obj->obj_flags.value[3])
      send_to_char("Poisonous properties are revealed!",ch);
    else
      send_to_char("You sense nothing harmful.\n\r", ch);
  }
}

/***************************/
/*** non-castable spells ***/
/***************************/

void spell_identify(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH],
       buf3[MAX_STRING_LENGTH];
  int i;
  bool found;
  struct obj_affected_type *aff;
  struct time_info_data age(struct char_data *ch);
 
  assert(ch);

  if (!obj) {
    send_to_char("This spell requires and object to identify.\n\r", ch);
    return;
  }
 
  send_to_char("You feel informed:\n\r", ch);
   
  sprintf(buf, "Object '%s', Item type: %s ", OBJ_NAME(obj),GET_WPN_DMG_TYPE(obj));
  sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
  strcat(buf,buf2); strcat(buf,"\n\r");
  send_to_char(buf, ch);
   
  send_to_char("Item is: ", ch);
  if (IS_PURE_ITEM(obj)) 
    sprintbit(obj->obj_flags.extra_flags,extra_bits_pure,buf);
  else
    sprintbit( obj->obj_flags.extra_flags,extra_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
   
  sprintf(buf,"Weight: %d, Value: %d, Rent cost: %d\n\r",
          obj->obj_flags.weight, obj->obj_flags.cost,
          obj->obj_flags.cost_per_day);
  send_to_char(buf, ch);

  sprintf(buf,"Level: %ld, Durability: %ld, Encumbrance: %ld\n\r",
          obj->obj_flags.level, obj->obj_flags.durability,
          obj->obj_flags.encumbrance);
  send_to_char(buf, ch);

  switch (GET_ITEM_TYPE(obj)) {
    case ITEM_SCROLL :
    case ITEM_POTION :
      sprintf(buf, "Level %d spells of:\n\r",   obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      if (obj->obj_flags.value[1] >= 1) {
        sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[1]));
        send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[2] >= 1) {
        sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[2]));
        send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[3] >= 1) {
        sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[3]));
        send_to_char(buf, ch);
      }
      break;

    case ITEM_WAND  :
    case ITEM_STAFF :
      sprintf(buf, "Has %d charges, with %d charges left.\n\r",
              obj->obj_flags.value[1],
              obj->obj_flags.value[2]);
      send_to_char(buf, ch);

      sprintf(buf, "Level %d spell of:\n\r",    obj->obj_flags.value[0]);
      send_to_char(buf, ch);

      if (obj->obj_flags.value[3] >= 1) {
        sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[3]));
        send_to_char(buf, ch);
      }
      break;

    case ITEM_WEAPON :
      sprintf(buf, "Damage Dice is '%dD%d'\n\r",
              obj->obj_flags.value[1],
              obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      break;

    case ITEM_FIREWEAPON :
      sprintf(buf, "Hit is '%d'\n\rMax range/Dam is '%d'\n\rType of bow[must match arrow type #] is '%d'\n\r",
              obj->obj_flags.value[1],
              obj->obj_flags.value[2],
              obj->obj_flags.value[3]);
      send_to_char(buf, ch);
      break;

    case ITEM_MISSILE :
      sprintf(buf, "Change of breaking on impact[Percentage] is '%d'\n\rDamage Dice is '%dD%d'\n\rType of arrow[must match bow type #] is '%d'\n\r",
              obj->obj_flags.value[0],
              obj->obj_flags.value[1],
              obj->obj_flags.value[2],
              obj->obj_flags.value[3]);
      send_to_char(buf, ch);
      break;

    case ITEM_ARMOR :
      sprintf(buf, "AC-apply is %d\n\r",
              obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      break;
    case ITEM_SPELLBOOK :
       for(i=0; i < 4; i++)
       {
          if(obj->obj_flags.value[i] <= 0) {
            sprintf(buf, "Page %d has not been written on yet.\n\r",i);
          } else if (obj->obj_flags.value[i] <= 250) {
            sprintf(buf, "Page %d was inscribed by a child.\n\r",i);
          } else if (obj->obj_flags.value[i] <= 500) {
            sprintf(buf, "Page %d was inscribed by an apprentice. \n\r", i);
          } else if (obj->obj_flags.value[i] <= 750) {
            sprintf(buf, "Page %d was inscribed by an journeyman. \n\r", i);
          } else if (obj->obj_flags.value[i] <= 1000) {
            sprintf(buf, "Page %d was inscribed by a true master. \n\r", i);
          } else {
            sprintf(buf, "Page %d was inscribed by the Gods themselves.\n\r", i);
          }
          send_to_char(buf, ch);
       }
       break;
  }

  found = FALSE;
  for (aff = obj->affected, i=0 ; i<MAX_OBJ_AFFECT ; aff++, i++) {
    if((aff->location != APPLY_NONE) && (aff->modifier != 0)) {
      if (!found) {
        send_to_char("Can affect you as :\n\r", ch);
        found = TRUE;
      }

      sprinttype(aff->location,apply_types,buf2);
      switch(aff->location) {
        case APPLY_SPELL:
          sprintbit(aff->modifier, affected_bits, buf3);
          sprintf(buf, "    Affects : %s By %s\n\r", buf2, buf3);
          send_to_char(buf, ch);
          break;

        case APPLY_SPELL2:
          sprintbit(aff->modifier, affected2_bits, buf3);
          sprintf(buf, "    Affects : %s By %s\n\r", buf2, buf3);
          send_to_char(buf, ch);
          break;

        case APPLY_WEAPON_SPELL:
        case APPLY_EAT_SPELL:
          sprintf(buf, "    Affects : %s By %s\n\r", buf2,
                  spell_name(aff->modifier));
          send_to_char(buf, ch);
          break;

        case APPLY_IMMUNE:
        case APPLY_M_IMMUNE:
        case APPLY_SUSC:
          sprintbit(aff->modifier, immunity_names, buf3);
          sprintf(buf, "    Affects : %s By %s\n\r", buf2, buf3);
          send_to_char(buf, ch);
          break;
        case APPLY_BOOK_SPELL:
          sprintf(buf, "    Contains text of : %s\n\r", spell_name(aff->modifier));
          send_to_char(buf, ch);
          break;
        default:
          sprintf(buf, "    Affects : %s By %ld\n\r", buf2,aff->modifier);
          send_to_char(buf, ch);
      }
    }
  }
}

void spell_mana(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  int dam;
  char buf[256];

  assert(ch && victim);

  dam = 50;
  if (GET_MANA(victim)+dam > GET_MAX_MANA(victim))
    GET_MANA(victim) = GET_MAX_MANA(victim);
  else
    GET_MANA(victim) += dam;

  sprintf(buf,"%s restores some of your arcane powers.\n\r",GET_NAME(ch));
  send_to_char(buf,victim);
  sprintf(buf,"You sacrifice some of your mana to restore %s's\r\n",
	  GET_NAME(victim));
  send_to_char(buf,ch);
}

/*****************************/
/*** misc defensive spells ***/
/*****************************/

void spell_armor(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(victim);

  if (affected_by_spell(victim, SPELL_ARMOR))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  MakeAffect(ch, victim, type,
	     SPELL_ARMOR, -10 - level, APPLY_AC, 0,
	     (level + 4) / 5, 2, FALSE, FALSE, FALSE, NULL);

  send_to_char_formatted("$CwYou feel less vulnerable now.\n\r", victim);
  act("$Cg$n $Cwis less vulnerable now.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_fireshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED(victim, AFF_FIRESHIELD))
	  return;

  if (type == SPELL_TYPE_POTION) {
    send_to_char("You start glowing red.\n\r", victim);
    act("$n is surrounded by a flaming red aura.",TRUE,victim,0,0,TO_ROOM);
    
    MakeAffect(ch, victim, type,
	       SPELL_FIRESHIELD, 0, APPLY_NONE, AFF_FIRESHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  send_to_char("You start glowing red.\n\r", victim);
  act("$n is surrounded by a flaming red aura.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type,
	     SPELL_FIRESHIELD, 0, APPLY_NONE, AFF_FIRESHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_elecshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_ELECSHIELD))
    return;

  if (type == SPELL_TYPE_POTION) {
    send_to_char("You start electrifying.\n\r", victim);
    act("$n is surrounded by sparks.",TRUE,victim,0,0,TO_ROOM);
    
    MakeAffect(ch, victim, type,
	       SPELL_ELECSHIELD, 0, APPLY_AFF2, AFF2_ELECSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  send_to_char("You start electrifying.\n\r", victim);
  act("$n is surrounded by sparks.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type,
	     SPELL_ELECSHIELD, 0, APPLY_AFF2, AFF2_ELECSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_coldshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_COLDSHIELD))
    return;

  if (type == SPELL_TYPE_POTION) {
    send_to_char("You start freezing.\n\r", victim);
    act("$n is surrounded by a wall of ice.",TRUE,victim,0,0,TO_ROOM);
    
    MakeAffect(ch, victim, type,
	       SPELL_COLDSHIELD, 0, APPLY_AFF2, AFF2_COLDSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  send_to_char("You start freezing.\n\r", victim);
  act("$n is surrounded by a wall of ice.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type,
	     SPELL_COLDSHIELD, 0, APPLY_AFF2, AFF2_COLDSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_energyshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_ENERGYSHIELD))
    return;

  if (type == SPELL_TYPE_POTION) {
    send_to_char("You start energyzing.\n\r", victim);
    act("$n is surrounded by energy.",TRUE,victim,0,0,TO_ROOM);
    
    MakeAffect(ch, victim, type,
	       SPELL_ENERGYSHIELD, 0, APPLY_AFF2, AFF2_ENERGYSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }


  send_to_char("You start energyzing.\n\r", victim);
  act("$n is surrounded by energy.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type,
	     SPELL_ENERGYSHIELD, 0, APPLY_AFF2, AFF2_ENERGYSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_acidshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_ACIDSHIELD))
    return;

  if (type == SPELL_TYPE_POTION) {
    send_to_char("You start eroding.\n\r", victim);
    act("$n is surrounded by erosion.",TRUE,victim,0,0,TO_ROOM);
    
    MakeAffect(ch, victim, type,
	       SPELL_ACIDSHIELD, 0, APPLY_AFF2, AFF2_ACIDSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  send_to_char("You start eroding.\n\r", victim);
  act("$n is surrounded by erosion.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type,
	     SPELL_ACIDSHIELD, 0, APPLY_AFF2, AFF2_ACIDSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_poisonshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_POISONSHIELD))
    return;

  send_to_char("You start growing snakes from your body.\n\r", victim);
  act("$n is surrounded by hundreds of snakes.",TRUE,victim,0,0,TO_ROOM);

  if(!IsImmune(victim, IMM_POISON))
    poison_effect (level, ch, victim);

  if (type == SPELL_TYPE_POTION) {
    MakeAffect(ch, victim, type,
	       SPELL_POISONSHIELD, 0, APPLY_AFF2, AFF2_POISONSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }
  
  MakeAffect(ch, victim, type,
	     SPELL_POISONSHIELD, 0, APPLY_AFF2, AFF2_POISONSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_vampshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_VAMPSHIELD))
    return;

  send_to_char("You start growing fangs.\n\r", victim);
  act("$n has huge fangs.",TRUE,victim,0,0,TO_ROOM);

  if (type == SPELL_TYPE_POTION) {
    MakeAffect(ch, victim, type,
	       SKILL_VAMPSHIELD, 0, APPLY_AFF2, AFF2_VAMPSHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  MakeAffect(ch, victim, type,
	     SKILL_VAMPSHIELD, 0, APPLY_AFF2, AFF2_VAMPSHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_manashield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED2(victim, AFF2_MANASHIELD))
    return;


  send_to_char("You start vibrating.\n\r", victim);
  act("$n is surrounded by a purple aura.",TRUE,victim,0,0,TO_ROOM);

  if (type == SPELL_TYPE_POTION) {

    MakeAffect(ch, victim, type,
	       SKILL_MANASHIELD, 0, APPLY_AFF2, AFF2_MANASHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  MakeAffect(ch, victim, type,
	     SKILL_MANASHIELD, 0, APPLY_AFF2, AFF2_MANASHIELD,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_moveshield(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert (victim);

  if (IS_AFFECTED2(victim, AFF2_MOVESHIELD))
    return;

  send_to_char("You start shimmering.\n\r", victim);
  act("$n is surrounded by a neon aura.",TRUE,victim,0,0,TO_ROOM);
  
  if (type == SPELL_TYPE_POTION) {
    MakeAffect(ch, victim, type,
	       SPELL_MOVESHIELD, 0, APPLY_AFF2, AFF2_MOVESHIELD,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  MakeAffect(ch, victim, type,
             SPELL_MOVESHIELD, 0, APPLY_AFF2, AFF2_MOVESHIELD,
             3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_sanctuary(ubyte level, struct char_data *ch, int type,
		     struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    return;

  send_to_char_formatted("$CwYou start glowing white.\n\r", victim);
  act("$Cw$n is surrounded by a glowing white aura.",TRUE,victim,0,0,TO_ROOM);

  if (type == SPELL_TYPE_POTION) {
    MakeAffect(ch, victim, type,
	       SPELL_SANCTUARY, 0, APPLY_NONE, AFF_SANCTUARY,
	       level / 20, 10, FALSE, FALSE, FALSE, NULL);
    return;
  }

  MakeAffect(ch, victim, type,
	     SPELL_SANCTUARY, 0, APPLY_NONE, AFF_SANCTUARY,
	     3, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_stone_skin(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  

  if (affected_by_spell(victim, SPELL_STONE_SKIN))
    return;

  if (type == SPELL_TYPE_POTION) {
    MakeAffect(ch, victim, type,
	       SPELL_STONE_SKIN, -10 - level, APPLY_AC, 0,
	       level / 20, 1, TRUE, FALSE, FALSE, NULL);

    if (level > 75)
      {
	/* resistance to piercing weapons */
	MakeAffect(ch, victim, type,
		   SPELL_STONE_SKIN, IMM_PIERCE, APPLY_IMMUNE, 0,
		   level / 20, 3, TRUE, FALSE, FALSE, NULL);
      }  
    send_to_char("Your skin hardens into a stone like substance.\n\r",victim);
    act("$n's skin hardens into a stone like substance.", TRUE, victim, 0, 0, TO_ROOM);
    return;
  }

  /* improve armor class */
  MakeAffect(ch, victim, type,
	     SPELL_STONE_SKIN, -10 - level, APPLY_AC, 0,
	     6, 1, TRUE, FALSE, FALSE, NULL);

  if (level > 75)
  {
    /* resistance to piercing weapons */
    MakeAffect(ch, victim, type,
	     SPELL_STONE_SKIN, IMM_PIERCE, APPLY_IMMUNE, 0,
	     6, 3, TRUE, FALSE, FALSE, NULL);
  }  
  send_to_char("Your skin hardens into a stone like substance.\n\r",victim);
  act("$n's skin hardens into a stone like substance.", TRUE, victim, 0, 0, TO_ROOM);
  
}

void spell_petrify(ubyte level, struct char_data *ch, int type,
                   struct char_data *victim, struct obj_data *obj)
{
  assert(ch);
  

  if (affected_by_spell(ch, SPELL_PETRIFY))
    return;

  /* improve armor class */
  MakeAffect(ch, victim, type,
	     SPELL_PETRIFY, -10 - level, APPLY_AC, 0,
	     6, 1, TRUE, FALSE, FALSE, NULL);

  if (level > 75)
  {
    /* resistance to piercing weapons */
    MakeAffect(ch, victim, type,
	     SPELL_PETRIFY, IMM_PIERCE, APPLY_IMMUNE, 0,
	     6, 3, TRUE, FALSE, FALSE, NULL);

    send_to_char("Your skin becomes very hard and petrifies.\n\r",ch);
    act("$n's skin becomes very hard and petrifies.", TRUE, ch, 0, 0, TO_ROOM);
  }
}

void spell_shield(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  assert(victim && ch);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_SHIELD))
    return;

  if (type == SPELL_TYPE_POTION)
  {
    /* resistance to piercing weapons */
    if(IS_PURE_CLASS(ch)) {
      MakeAffect(ch, victim, type,
		 SPELL_SHIELD, -20 - level, APPLY_AC, 0,
		 level / 20, 1, FALSE, FALSE, FALSE, NULL);
    } else {
      MakeAffect(ch, victim, type,
		 SPELL_SHIELD, -10 - level, APPLY_AC, 0,
		 level / 20, 1, FALSE, FALSE, FALSE, NULL);
    }
    return;
  }

  if(IS_PURE_CLASS(ch)) {
     MakeAffect(ch, victim, type,
	     SPELL_SHIELD, -20 - level, APPLY_AC, 0,
	     10, 1, FALSE, FALSE, FALSE, NULL);
  } else {
     MakeAffect(ch, victim, type,
	     SPELL_SHIELD, -10 - level, APPLY_AC, 0,
	     10, 1, FALSE, FALSE, FALSE, NULL);
  }	     

  send_to_char("You are surrounded by a force shield.\n\r",victim);
  act("$n is surrounded by a force shield.",TRUE,victim,0,0,TO_ROOM);
}

void spell_fear(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (IsUndead(victim))
    return;

    if (!ImpSaveSpell(victim, SAVING_PETRI,fearSpellMod)) {
      act("$n flees in terror!", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You flee in terror!",victim);
      do_flee(victim,"",0);
      return;
    }
  
}

void spell_turn(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  int dam=0, mobalign, chalign, aligncase=0;

  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (!IsUndead(victim))
    return;

  //diff=level-GetMaxLevel(victim)+1;

  //for (i=1; i<=diff; i++) 
  mobalign = victim->specials.alignment;
  chalign = ch->specials.alignment;

  if(mobalign < -350) aligncase = 0;
  if(mobalign >= -350 && mobalign < 350) aligncase = 1;
  if(mobalign >= 350) aligncase = 2;

  switch(aligncase) {
	case 0 :  {
		if(chalign > mobalign) 
			dam = (dice(2, (int)((chalign - mobalign) /50)));
		else 
			dam = GetMaxLevel(ch);
		break;}
	case 1 : {
		if(chalign > mobalign)
			dam = (dice(2, (int)((chalign - mobalign) /100)));
		else if(chalign < mobalign)
			dam = (dice(2,(int) ((mobalign- chalign) / 100)));
		else
			dam = GetMaxLevel(ch);
		break; }
	case 2 : {
		if(chalign > mobalign)
			dam = (dice(2,(int) ((chalign - mobalign) / 200)));
		else if(chalign <= -350)
			dam = (dice(2, (int) ((mobalign - chalign) / 50)));
		else
			dam = GetMaxLevel(ch);
		break;
			 }
		}


    if (!ImpSaveSpell(victim, SAVING_PETRI, turnSpellMod)) 
	{
		damage(ch, victim, dam, SPELL_TURN);
		act("$n is hit with a bolt from your deity!", TRUE, victim, 0, 0,
			TO_CHAR);
		act("$Cg$n calls upon the gods, and they answer!",TRUE, ch, 0,0,
			TO_ROOM);
		act("$CgThe hand of $CYGOD $Cgreaches out and strikes you!", TRUE,
			victim, 0, 0, TO_VICT);
                WAIT_STATE(ch, PULSE_VIOLENCE*2);
		return;
    }
	else {
		act("$n tries to turn $N!", TRUE, ch, 0, victim, TO_ROOM);
        send_to_char("The gods smile down on you today",victim);
	}

    WAIT_STATE(ch, PULSE_VIOLENCE*2);
}
		
            

/***************************/
/*** misc helpful spells ***/
/***************************/

void spell_flying(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if(IS_AFFECTED(victim, AFF_FLYING))
    return;

  if (type == SPELL_TYPE_POTION) {
    
    MakeAffect(ch, victim, type,
	       SPELL_FLY, 0, 0, AFF_FLYING,
	       level, 2, FALSE, FALSE, FALSE, NULL);
    
    send_to_char("Your feet rise up off the ground.\n\r", victim);
    act("$n's feet rise off the ground.", TRUE, victim, 0, 0, TO_ROOM);
    return;
  }

  if (HasClass(ch,CLASS_BARD)) {
    if (!is_hold_instrument(ch,SPELL_FLY)) {
      return;
    }

    MakeAffect(ch, victim, type,
               SPELL_FLY, 0, 0, AFF_FLYING,
               level, 2, FALSE, FALSE, FALSE, NULL);

    send_to_char("Your song causes you to float off the ground.\n\r", victim);
    act("$n's feet rise off the ground.", TRUE, victim, 0, 0, TO_ROOM);
  }

  if(IS_AFFECTED(victim, AFF_FLYING))
    return;

  MakeAffect(ch, victim, type,
	     SPELL_FLY, 0, 0, AFF_FLYING,
	     level, 2, FALSE, FALSE, FALSE, NULL);

  send_to_char("Your feet rise up off the ground.\n\r", victim);
  act("$n's feet rise off the ground.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_windwalk(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  assert(ch);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (!OUTSIDE(ch)) {
    send_to_char("You must be outdoors to call upon the winds.\n\r",ch);
    return;
  }

  send_to_char("You call upon the winds for assistance!\n\r",ch);
  act("$n calls upon the winds for assistance!",TRUE,ch,0,0,TO_ROOM);

  if(IS_AFFECTED(ch, AFF_FLYING))
    return;

  MakeAffect(ch, ch, type,
             SPELL_WINDWALK, 0, 0, AFF_FLYING,
             level, 2, FALSE, FALSE, FALSE, NULL);

  send_to_char("You rise up off the ground.\n\r", ch);
  act("$n rises off the ground.", TRUE, ch, 0, 0, TO_ROOM);
}

void spell_water_breath(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_AFFECTED(victim, AFF_WATERBREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 10;
  }

  MakeAffect(ch, victim, type,
	     SPELL_WATER_BREATH, 0, 0, AFF_WATERBREATH,
	     level, 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("You make a face like a fish.\n\r",victim);
  act("$n makes a face like a fish.",TRUE,victim,0,0,TO_ROOM);
}

// Very similar to the paladins skill bless, its here for use as a potion only
void spell_blessing(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  int rating,factor,mana;
  struct char_data *test;

  if (type != SPELL_TYPE_POTION) {
    log_msg("A paladins blessing was cast as a spell, this is a serious bug - fix immediately");
    return;
  }

  factor=1;
  if (ch->specials.alignment > 350)
    factor++;
  if (ch->specials.alignment == 1000)
    factor++;
  if (ch->specials.alignment < 0)
    factor--;
  if (ch->specials.alignment < 350)
    factor--;

  rating = (level * ch->specials.alignment) / 1000 + factor;
  rating += MIN(factor,3);

  // By quaffing a potion you will only get a restricted blessing
  // You can never make a potion as powerful as a real paladins blessing - raist
  if (type == SPELL_TYPE_POTION) {
    rating = MAX( rating - 35, 34);
  }

  if (rating<0)
      {
	send_to_char("You are so despised by your god that he punishes you!\n\r",ch);
	spell_blindness(level,ch,SPELL_TYPE_SPELL,ch,0);
	spell_paralyze(level,ch,SPELL_TYPE_SPELL,ch,0);
      }
    else
      {
	if (rating==0)
	  {
	    send_to_char("The potion seems to have no effect on you\n\r",ch);
	    return;
	  }
	if (!(affected_by_spell(ch,SPELL_BLESS)))
	  spell_bless(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>1)
	  if (!(affected_by_spell(ch,SPELL_ARMOR)))
	    spell_armor(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>4)
	  if (!(affected_by_spell(ch,SPELL_STRENGTH)))
	    spell_strength(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>6)
	  spell_refresh(level,ch,SPELL_TYPE_SPELL, ch,0);
	if (rating>9)
	  if (!(affected_by_spell(ch,SPELL_SENSE_LIFE)))
	    spell_sense_life(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>14)
	  if (!(affected_by_spell(ch,SPELL_TRUE_SIGHT)))
	    spell_true_seeing(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>19)
	  spell_cure_critic(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>24)
	  if (!(affected_by_spell(ch,SPELL_SANCTUARY)))
	    spell_sanctuary(level,ch,SPELL_TYPE_SPELL,ch,0);
	if(rating>29)
	  spell_heal(level,ch,SPELL_TYPE_SPELL,ch,0);
	if(rating>34)
	  {
	    spell_remove_poison(level,ch,SPELL_TYPE_SPELL,ch,0);
	    spell_remove_paralysis(level,ch,SPELL_TYPE_SPELL,ch,0);
	  }
	if (rating>39)
	  spell_heal(level,ch,SPELL_TYPE_SPELL,ch,0);
	if (rating>44)
	  {
	    if (ch->specials.conditions[2] != -1)
	      ch->specials.conditions[2] = 24;
	    if (ch->specials.conditions[1] != -1)
	      ch->specials.conditions[1] = 24;
	  }
	if (rating>54)
	  {
	    spell_heal(level,ch,SPELL_TYPE_SPELL,ch,0);
	    send_to_char ("An awesome feeling of holy power overcomes you!\n\r",ch);
	  }
	act ("$n has just been blessed by the gods!",TRUE,ch,0,0,TO_NOTVICT);
	act ("You have been truly blessed!",TRUE,ch,0,0,TO_CHAR);
	update_pos (ch);
      }
  WAIT_STATE(ch,PULSE_VIOLENCE*1);

}

void spell_bless(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  if (victim->specials.fighting ||  affected_by_spell(victim, SPELL_BLESS))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  MakeAffect(ch, victim, type,
	     SPELL_BLESS, -(MAX(1, level/10)), APPLY_SAVING_SPELL, 0,
	     level, 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("You feel truly blessed against magical energy.\n\r", victim);
  act("$n is truly blessed against magical energy.", TRUE, victim, 0, 0, TO_ROOM);

  if (GetMaxLevel(ch) >= 50)
  {
	  MakeAffect(ch, victim, type,
		     SPELL_BLESS, -(MAX(1, level/10)), APPLY_SAVING_PARA, 0,
		     level, 1, FALSE, FALSE, FALSE, NULL);

	  send_to_char("You feel truly blessed by movement.\n\r", victim);
	  act("$n is truly blessed with a new gracefulness.", TRUE, victim, 0, 0, TO_ROOM);
  }
  if (GetMaxLevel(ch) >= 75)
  {
	  MakeAffect(ch, victim, type,
		     SPELL_BLESS, -(MAX(1, level/10)), APPLY_SAVING_PETRI, 0,
		     level, 1, FALSE, FALSE, FALSE, NULL);

	  send_to_char("You feel truly blessed and less fearful.\n\r", victim);
	  act("$n is truly blessed with courage.", TRUE, victim, 0, 0, TO_ROOM);
  }
  if (GetMaxLevel(ch) >= 100)
  {
	  MakeAffect(ch, victim, type,
		     SPELL_BLESS, -(MAX(1, level/10)), APPLY_SAVING_ROD, 0,
		     level, 1, FALSE, FALSE, FALSE, NULL);
	  send_to_char("You feel truly blessed and protected against instilled magiks.\n\r", victim);
	  act("$n is truly blessed with a protection against instilled magiks.\n\r", TRUE, victim, 0, 0, TO_ROOM);
  }
  if (GetMaxLevel(ch) >= 115)
  {
	  MakeAffect(ch, victim, type,
		     SPELL_BLESS, -(MAX(1, level/10)), APPLY_SAVING_BREATH, 0,
		     level, 1, FALSE, FALSE, FALSE, NULL);
	  send_to_char("You feel truly blessed and are protected from engulfing affects.\n\r", victim);
	  act("$n is truly blesed and protected from engulfing forces.\n\r", TRUE, victim, 0, 0, TO_ROOM);
  }
}

void spell_strength(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
    int mod;
    
    assert(victim);
    

    if (affected_by_spell(victim,SPELL_STRENGTH)) {
	act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);
	return;
    }

    if (HasClass(victim, CLASS_WARRIOR) ||
	HasClass(victim, CLASS_PALADIN) ||
	HasClass(victim, CLASS_RANGER))
	mod = MAX(level/6,1);
    else
	mod = MAX(level/11,1);

    // for duration only
    if (type == SPELL_TYPE_POTION) {
      level = level / 20;
    }

    MakeAffect(ch, victim, type,
	       SPELL_STRENGTH, mod, APPLY_STR, 0,
	       MAX(level, 10), 1, FALSE, FALSE, FALSE, NULL);

    act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);
    act("$n seems stronger.\n\r", FALSE, victim, 0, 0, TO_ROOM);
}

void spell_cure_blind(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
    assert(victim);


#ifdef JANWORK   
    assert((level >= 0) && (level <= ABS_MAX_LVL));
#endif
    


    if (!affected_by_spell(victim, SPELL_BLINDNESS))
	return;

    affect_from_char(victim, SPELL_BLINDNESS);

    send_to_char("Your clarity of vision returns.\n\r", victim);
    act("$n's clarity of vision returns.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_remove_curse(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  assert(ch && (victim || obj));

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
      act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      act("$p is not cursed!", FALSE, ch, obj, 0, TO_CHAR);
    }
    return;
  }

  if (affected_by_spell(victim, SPELL_CURSE) ) {
      affect_from_char(victim, SPELL_CURSE);
      send_to_char("You feel better now.\n\r",victim);
      act("$n is not cursed anymore.",TRUE, victim, 0, 0, TO_ROOM);
  } else
      act("$N is not cursed!", FALSE, ch, 0, victim, TO_CHAR);
}

void spell_ray_of_purification(ubyte level, struct char_data *ch, int type,
			       struct char_data *victim, struct obj_data *obj)
{

  struct obj_data  *tmp;
  struct obj_data  *n;
  int j;

  if (!(ch && victim)) {
     send_to_char("No victim!?!?\n\r",ch);
     return;
   } else { 
     act("$n raises a hand and a cleansing ray of light strikes $N.", FALSE, ch, 0, victim, TO_ROOM);
     act("You call upon the power of your deity to purify $N", FALSE,
	 ch, 0, victim, TO_CHAR);
   }

  for (j=0; j<MAX_WEAR; j++) {
    if (victim->equipment[j]) {
      tmp = victim->equipment[j];
      if (IS_SET(tmp->obj_flags.extra_flags, ITEM_NODROP)) {
	REMOVE_BIT(tmp->obj_flags.extra_flags, ITEM_NODROP);
	act("$p briefly glows blue for a moment.", FALSE, ch, tmp, 0, TO_CHAR);
	act("$p briefly glows blue for a moment.", FALSE, ch, tmp, 0, TO_ROOM);
      }
    }
  }
    
  for (tmp=victim->carrying; tmp; tmp=n) {
    n=tmp->next_content;
      
    if (tmp) {
      if (IS_SET(tmp->obj_flags.extra_flags, ITEM_NODROP)) {
	REMOVE_BIT(tmp->obj_flags.extra_flags, ITEM_NODROP);
	act("$p briefly glows blue for a moment.", FALSE, ch, tmp, 0, TO_CHAR);
	act("$p briefly glows blue for a moment.", FALSE, ch, tmp, 0, TO_ROOM);
      }
    }
  }
}

void spell_remove_poison(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  assert(ch && (victim || obj));

  if (obj) {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("$p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
      act("$p steams briefly.",FALSE,ch,obj,0,TO_ROOM);
    }
    return;
  }

  if (!IS_AFFECTED(victim,AFF_POISON))
    return;

  affect_from_char(victim,SPELL_POISON);

  send_to_char("A pleasant feeling runs through your body.\n\r",victim);
  act("$n looks much healthier.",TRUE, victim, 0, 0, TO_ROOM);
}

void spell_remove_paralysis(ubyte level, struct char_data *ch, int type,
                            struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);

  if (!IS_AFFECTED(victim,AFF_PARALYSIS))
    return;

  /*
   * To me, it seems a little foolish to say affect_from_char()
   * and then specify a skill and not an affect, but who am i
   * to argue with 70k lines of code.
   * Anyways, I made it to remove stun also.
   */
  affect_from_char(victim,SPELL_PARALYSIS);
  affect_from_char(victim,SKILL_STUN);

  send_to_char("A warm feeling penetrates your body and you are able to move again.", victim);
  act("$n is able to move again.",TRUE,victim,0,0,TO_ROOM);
}

/*
 **   requires the sacrifice of 15k coins, victim no longer loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */
void spell_resurrection(ubyte level, struct char_data *ch, int type,
			struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *obj_object, *next_obj;
  struct char_data* tmp_ch;
  int cost = 0;
 
  if (!obj) return;
 
  if (!CountFollowers(ch))
    return;

  if (IS_CORPSE(obj))
  {
    if (obj->char_vnum) {	/* corpse is a npc */
      victim = make_mobile(obj->char_vnum, VIRTUAL);

      if(!IS_GOD(ch))
      {
	cost = GetMaxLevel(victim) * 1000; /* Originally: 2000 */
		
	if (GET_GOLD(ch) < cost)
	{
	  send_to_char(
		       "The gods are not happy with your sacrifice.\n\r",
		       ch);
	  extract_char(victim);
	  return;
	}

	GET_GOLD(ch) -= cost;
      }	    

      char_to_room(victim, ch->in_room);
      GET_GOLD(victim)=0;
      GET_EXP(victim)=0;
      GET_HIT(victim)=1;
      victim->points.mana=0;
      GET_POS(victim)=POSITION_STUNNED;

      act("With mystic power, $n resurrects a corpse.", TRUE, ch,
	  0, 0, TO_ROOM);

      act("$N slowly rises from the ground.",
	  FALSE, ch, 0, victim, TO_ROOM);

      /*
	 should be charmed and follower ch
	 */

      if((NewSkillSave(ch, victim, SPELL_RESURRECTION, +7, IMM_CHARM)))
      {
	do_say(victim, "Thank you very much!", 0);
	GET_HIT(victim) = GET_MAX_HIT(victim) / 2;
	update_pos(victim);
      }
      else
      {
	MakeCharmed(ch, victim, level, SPELL_RESURRECTION, -4);
	fix_mob_bits(victim);
	add_follower(victim, ch, 0);
      }

      IS_CARRYING_W(victim) = 0;
      IS_CARRYING_N(victim) = 0;

      /*
	 take all from corpse, and give to person
	 */

      for (obj_object=obj->contains; obj_object; obj_object=next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, victim);
      }

      /*
	 get rid of corpse
	 */
      extract_obj(obj);
    } else {			/* corpse is a pc  */
      int	in_game = 0;
      struct descriptor_data* desc;

      remove("corpsedata.dat");

      /* find them already playing */
      if((tmp_ch = find_player_in_world(ss_data(obj->char_name))))
      {
	in_game = 1;
      }
      else
      {				/* find them at the menu */
	EACH_DESCRIPTOR(d_iter, desc)
	{
	  if(desc->character &&
	     GET_IDENT(desc->character) &&
	     !str_cmp(GET_IDENT(desc->character),
		      ss_data(obj->char_name)))
	  {
	    tmp_ch = desc->character;
	    in_game = 1;
	  }
	}
	END_ITER(d_iter);
      }

      /* find them in the file */
      if(!tmp_ch &&
	 !(tmp_ch = LoadChar(0, ss_data(obj->char_name), READ_ALL)))
      {
	char buf[256];

	sprintf(buf, "Ressing non existent char: %s",
		ss_data(obj->char_name));
	log_msg(buf);
	send_to_char("That's doesn't appear to be a real pc.\n\r", ch);
	return;
      }

      if(!tmp_ch->res_info.valid)
      {
	send_to_char("The gods like him dead...\n\r", ch);
	goto done;
      }

      /* Code to make Resurrection only work in Temples after level 50 */ 
      if(GetMaxLevel(tmp_ch) > 50 && TRUST(ch) < TRUST_DEMIGOD)
      {
        struct room_data *room;
        int  room_max_level = 0, i = 0;
        room = real_roomp(IN_ROOM(ch));
        if(!IS_TEMPLE(room)) {
          send_to_char("The gods have no pity on this corpse.  Your sacrifice was in vain.\n\r",ch);
          goto done;
        }else {
          for(i = 0; res_altar_rooms[i].room != 0; i++)
          {
             if(res_altar_rooms[i].room == IN_ROOM(ch))
                room_max_level = res_altar_rooms[i].maxlvl;
          }
          if(GetMaxLevel(tmp_ch) > room_max_level) { 
             send_to_char("The gods of this temple are unable to retrieve this bodies soul.\n\r",ch);
             goto done;
          }
        }

        
      }

      if(TRUST(ch) < TRUST_DEMIGOD)
      {
        if (GetMaxLevel(tmp_ch) > 50)
          cost = GetMaxLevel(tmp_ch) * 1000; /* Originally 15,000 */
        else
          cost = 15000; /* Originally: 150,000 */

	if (GET_GOLD(ch) < cost)
	{
	  send_to_char(
		       "The gods are not happy with your sacrifice.\n\r",
		       ch);
	  goto done;
	}

	GET_GOLD(ch) -= cost;
      }
	    
      if(GET_CON(tmp_ch) <= 3)
      {
	send_to_char(
		     "The body does not have the strength to be recreated.\n\r", ch);
	goto done;
      }


      tmp_ch->points.exp = tmp_ch->res_info.exp;
      GET_LEVEL(tmp_ch, (int)tmp_ch->res_info.clss) =
	tmp_ch->res_info.level;
      UpdateMaxLevel(tmp_ch);
      UpdateMinLevel(tmp_ch);
      tmp_ch->res_info.valid = 0;

      if(GetMaxLevel(tmp_ch) >=50){
        if((number(1,3) == 1)) {
          tmp_ch->abilities.con -= 0; /* 0 was 1 (Con lost due to resurrection) - changed due to level not dropping after death */
        }
      } else {
         tmp_ch->abilities.con -= 0;
      }
      SaveChar(tmp_ch, AUTO_RENT, 0);

      act("A clear bell rings throughout the heavens",
	  TRUE, ch, 0, 0, TO_CHAR);
      act("A ghostly spirit smiles, and says 'Thank you'",
	  TRUE, ch, 0, 0, TO_CHAR);
      act("A clear bell rings throughout the heavens",
	  TRUE, ch, 0, 0, TO_ROOM);
      act("A ghostly spirit smiles, and says 'Thank you'",
	  TRUE, ch, 0, 0, TO_ROOM);
      act("$p dissappears in the blink of an eye.",
	  TRUE, ch, obj, 0, TO_ROOM);
      act("$p dissappears in the blink of an eye.",
	  TRUE, ch, obj, 0, TO_ROOM);
      GET_MANA(ch) = 1;
      GET_MOVE(ch) = 1;
      GET_HIT(ch) = 1;

      GET_POS(ch) = POSITION_STUNNED;
      act("$n collapses from the effort!",TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You collapse from the effort\n\r",ch);
      ObjFromCorpse(obj);

      if(in_game &&
	 tmp_ch->desc &&
	 tmp_ch->desc->connected == CON_PLYNG)
      {
	char_from_room(tmp_ch);
	char_to_room(tmp_ch, ch->in_room);

	send_to_char(
		     "You feel strangely disembodied as your world spins around you.\n\r", tmp_ch);
      }

    done:
      if(!in_game)
	extract_char(tmp_ch);
    }
  } 
}

void spell_refresh(ubyte level, struct char_data *ch, int type,
                   struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = 50 * MAX(1, level/10);

  if ((dam+GET_MOVE(victim)) > move_limit(victim))
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel refreshed.\n\r", victim);
  act("$n feels refreshed.",TRUE, victim, 0, 0, TO_ROOM);
}

void spell_track(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, int major)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(major)
      MakeAffect(ch, victim, type,
		 SPELL_MAJOR_TRACK, 0, APPLY_NONE, 0,
		 level / 10, 2, FALSE, FALSE, FALSE, NULL);
  else
      MakeAffect(ch, victim, type,
		 SPELL_MINOR_TRACK, 0, APPLY_NONE, 0,
		 level / 10, 1, FALSE, FALSE, FALSE, NULL);
      
  send_to_char("You feel your awareness grow!\n\r", victim);
  act("$n's eyes become brighter and more alert!",FALSE,victim,0,0,TO_NOTVICT);
}

/* really only practical for a poly to cast, but oh well */
void spell_calm(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  int i, diff;

  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  if (victim->specials.fighting)
    return;

  if (!IS_SET(victim->specials.mob_act, ACT_AGGRESSIVE)) {
    act("$N is not aggressive to begin with.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  if (GetMaxLevel(victim)>level ||
      IS_SET(victim->specials.mob_act, ACT_META_AGG)) {
    act("You have no hope of calming $N.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  diff=level-GetMaxLevel(victim)+1;

  for (i=1; i<=diff; i++) {
    if (!saves_spell(victim, SAVING_SPELL, 0)) {
      REMOVE_BIT(victim->specials.mob_act, ACT_AGGRESSIVE);
      send_to_char("You feel much more calm.\n\r", victim);
      act("$n is not aggressive anymore.",TRUE,victim,0,0,TO_ROOM);
      return;
    }
  }

  if (!victim->specials.fighting)
    set_fighting(victim, ch);
}

void spell_poly_self(ubyte level, struct char_data *ch, int type,
   struct char_data *mob, struct obj_data *obj, int poly_type)
{
    char buf[256];
    int att, curhp, hp, hroll, droll, ndice, sdice, mob_age;
    int mstate;

    /* added by min to support better polies */
    sh_int charhp, charmana, charmove;
    sh_int charmaxhp, charmaxmana, charmaxmove;
    
    assert(ch);
    
    if(!ch->desc)
	return;
    
/*
  if (!IS_NPC(ch) && (IS_SET(ch->specials.mob_act,ACT_IT))) {
  send_to_char("You're IT!  You cannot polymorph until you tag someone!\n\r", ch);
  return;
  }
  */
    hroll = (int) ((GetMaxLevel(ch)/15)+25);
    droll = (int) ((GetMaxLevel(ch)/15)+20);
    ndice = (int) (GetMaxLevel(ch)/20) + 1;
    sdice = (int) (GetMaxLevel(ch)/20) + 10;
    att=MAX(2, (int)(GetMaxLevel(ch)/20));
    hp=(int) (0.75 * GET_MAX_HIT(ch)); /* used only for spell tree - not
                                          very clean - Min */
    mob_age = GetPolyAge(PolyList,mob_index[mob->nr].virt); 
    mob_age *= SECS_PER_MUD_YEAR;
	    
    /* stop following whomever we were */
    if (ch->master)
	stop_follower(ch);
    stop_all_followers(ch);
    
    /* bring the poly form into the room */
    char_to_room(mob, ch->in_room);
    
    if(poly_type == SPELL_TREE){
      mob->specials.damnodice = ndice;
      mob->specials.damsizedice = sdice;
    }
    /* give all the chars eq/etc to the poly */
    SwitchStuff(ch, mob);
   

    curhp=(hp*GET_HIT(ch))/GET_MAX_HIT(ch);

    charhp = GET_HIT(ch);
    charmana = GET_MANA(ch);
    charmove = GET_MOVE(ch);

    charmaxhp = GET_MAX_HIT(ch);
    charmaxmana= GET_MAX_MANA(ch);
    charmaxmove= GET_MAX_MOVE(ch);
    
    mstate=ch->player.mpstate;
   
    act("$n polymorphs into $N",TRUE,ch,0,mob,TO_ROOM);
    act("You polymorph into $N",TRUE,ch,0,mob,TO_CHAR);
    
    /* put the char's original form in storage */
    char_from_room(ch);
    char_to_room(ch, 3);
    
    /* switch into the mob */
    push_character(ch, mob);
    
    /* combine names of poly and player */
    sprintf(buf,"%s %s", GET_IDENT(mob), GET_IDENT(ch));
    ss_free(mob->player.name);
    mob->player.name = ss_make(buf);
    
    /* set the polymorphed flag on the player */
    SET_BIT(mob->specials.mob_act, ACT_POLYSELF);
    mob->specials.flags = ch->specials.flags;

    if (poly_type==SPELL_TREE) {
	// Druid buffing - Raist
        att=MAX(2, (int)(GetMaxLevel(ch)/15));
	mob->mult_att=att;

	mob->points.hit=curhp;
	mob->points.max_hit=hp;
        mob->points.hitroll=hroll;
        mob->points.damroll = droll;
	/* With saving throw being more important, this constitues
	   an unfair advantage - Raist */
	//mob->specials.apply_saving_throw[0] = -2;
	//mob->specials.apply_saving_throw[1] = 3;
	//mob->specials.apply_saving_throw[2] = 0;
	//mob->specials.apply_saving_throw[3] = 0;
	//mob->specials.apply_saving_throw[4] = -3;	
    }
   /* Lets do some calculations */

/* added by min for test work */

    mob->points.max_hit = MAX(mob->points.max_hit, charmaxhp);
    mob->points.hit = MAX(mob->points.hit, charhp);

    mob->points.max_mana = MAX(mob->points.max_mana, charmaxmana);
    mob->points.mana = MAX(mob->points.mana, charmana);

    mob->points.max_move = MAX(mob->points.max_move, charmaxmove);
    mob->points.move = MAX(mob->points.move, charmove);

    mob->player.mpstate = mstate;
    mob->player.time.birth = time(0);
    mob->player.time.birth -= mob_age;
/* ------------------------ */

    /* this is necessary because the caller is still referring to the old */
    /* char_data, so the caller won't take care of this crap for us.      */
    GET_MANA(mob) = MAX((GET_MANA(mob)-30), 70); 

    WAIT_STATE(mob, PULSE_VIOLENCE*2);
}

void spell_heroes_feast(ubyte level, struct char_data *ch, int type,
   struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);
  

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if ((in_group(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
      if (GET_COND(tch, FULL) != -1)
        gain_condition(tch,FULL,24);
      if (GET_COND(tch, THIRST) != -1)
        gain_condition(tch,THIRST,24);
      if (GET_HIT(tch) < GET_MAX_HIT(tch))
        spell_cure_light(level, ch, type, tch, obj);
      if (GET_MOVE(tch) < GET_MAX_MOVE(tch))
        spell_refresh(level, ch, type, tch, obj);
      send_to_char("You partake of a magnificent feast!\n\r", tch);
    }
  }
}

void spell_mount(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  int mount=VMOB_MNT_ONE;
  struct char_data *m;

  assert(ch);
  

  if (level < 30) {
    if (level < 12) 
      mount++;
    if (level < 18) 
      mount++;
    if (level < 24)
      mount++;
  } else if (IS_EVIL(ch)) {
    mount = VMOB_MNT_EVIL;
  } else if (IS_GOOD(ch)) {
    mount = VMOB_MNT_GOOD;
  } else {
    mount = VMOB_MNT_NEUT;
  }

  if (!(m=make_mobile(mount, VIRTUAL))) {
    log_msg("screwup in mount spell.");
    send_to_char("This spell is experiencing technical difficulty.\n\r", ch);
    return;
  }

  m->points.mana=0;
  char_to_room(m, ch->in_room);
  act("In a flash of light, $n appears.", FALSE, m, 0, 0, TO_ROOM);
  send_to_char("You hop on your mount's back.\n\r", ch);

  MOUNTED(ch) = m;
  RIDDEN(m)   = ch;
  GET_POS(ch) = POSITION_MOUNTED;
}

/*******************/
/*** item spells ***/
/*******************/

void spell_locate_object(ubyte level, struct char_data *ch, int type,
			 struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *i;
    char name[256];
    char buf[MAX_STRING_LENGTH];
    int j;
  
    assert(ch);

    strcpy(name, ss_data(obj->name));
    ss_free(obj->name);
    FREE(obj);

    j=level>>1;


    sprintf(buf,"You call upon the spirits of the dead to tell you where %s is located.\n\r",name);
    send_to_char_formatted(buf,ch);

    EACH_OBJECT(iter, i)
    {
	if (isname(name, OBJ_NAME(i)) && CAN_SEE_OBJ(ch, i))
	{
//	   if(GET_OBJ_EXTRA(i) & ITEM_NO_LOCATE)
//	     continue;
	   
	    if(i->carried_by) {
		if (strlen(PERS(i->carried_by, ch))>0) {
		    sprintf(buf,"%s carried by %s.\n\r",
			    OBJ_SHORT(i),PERS(i->carried_by,ch));
		    send_to_char(buf,ch);
		}
	    } else if(i->equipped_by) {
		if (strlen(PERS(i->equipped_by, ch))>0) {
		    sprintf(buf,"%s equipped by %s.\n\r",
			    OBJ_SHORT(i),PERS(i->equipped_by,ch));
		    send_to_char(buf,ch);
		}
	    } else if (i->in_obj) {
		sprintf(buf,"%s in %s.\n\r",
			OBJ_SHORT(i), OBJ_SHORT(i->in_obj));
		send_to_char(buf,ch);
	    } else {
		sprintf(buf,"%s in %s.\n\r",OBJ_SHORT(i),
			(i->in_room == NOWHERE ?
			 "use but uncertain." : real_roomp(i->in_room)->name));
		send_to_char(buf,ch);
	    }      
	    if(--j <= 0)
		break;
	}
    }
    END_AITER(iter);
    
    if(j<=0)
	send_to_char("You are very confused.\n\r",ch);
    if(j==level>>1)
	send_to_char("No such object.\n\r",ch);
}  


void spell_continual_dark(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  int i;

  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 1);

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    send_to_char("This item will not accept your spell effect.\n\r", ch);
    return;
  }

  /* make sure there is a free slot */
  i=getFreeAffSlot(obj);
  if (i==-1) {
    send_to_char("This item will not accept your spell effect.\n\r", ch);
    return;
  }

  SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

  obj->affected[i].location = APPLY_SPELL;
  obj->affected[i].modifier = AFF_CONTINUAL_DARK;

  act("$p flares strangely and everything goes black!",FALSE,ch,obj,0,TO_CHAR);
  act("$p flares strangely and everything goes black!",TRUE,ch,obj,0,TO_ROOM);
}

void spell_continual_light(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  int i;

  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 1);

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    send_to_char("This item will not accept your spell effect.\n\r", ch);
    return;
  }

  /* make sure there is a free slot */
  i=getFreeAffSlot(obj);
  if (i==-1) {
    send_to_char("This item will not accept your spell effect.\n\r", ch);
    return;
  }

  SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

  obj->affected[i].location = APPLY_SPELL;
  obj->affected[i].modifier = AFF_CONTINUAL_LIGHT;

  act("$p flares and shines brightly!",FALSE,ch,obj,0,TO_CHAR);
  act("$p flares and shines brightly!",TRUE,ch,obj,0,TO_ROOM);
}

void spell_enchant_weapon(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  int i;

  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 1);

  // Solaar: added IMMORTAL condition
  if ((GET_ITEM_TYPE(obj) != ITEM_WEAPON) && !(IS_IMMORTAL(ch))) {
    send_to_char("You can't enchant that!\n\r", ch);
    return;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    send_to_char("This weapon is already enchanted!\n\r", ch);
    return;
  }

  /* make sure there is a free slot */
  i=getFreeAffSlot(obj);
  if (i==-1) {
    send_to_char("This weapon already has too many effects!\n\r", ch);
    return;
  }

  SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

  obj->affected[i].location = APPLY_HITNDAM;
  obj->affected[i].modifier = 1;
  if (level > 20)
    obj->affected[i].modifier += 1;
  if (level > 40)
    obj->affected[i].modifier += 1;
  if (level > 80)
    obj->affected[i].modifier += 1;
  if (level > 120)
    obj->affected[i].modifier += 1;
  if (level > MAX_MORT)
    obj->affected[i].modifier += 1;

  if (IS_GOOD(ch)) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
    act("$p glows briefly with a light aura!",FALSE,ch,obj,0,TO_CHAR);
    act("$p glows briefly with a light aura!",TRUE,ch,obj,0,TO_ROOM);
  } else if (IS_EVIL(ch)) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    act("$p glows briefly with a dark aura!",FALSE,ch,obj,0,TO_CHAR);
    act("$p glows briefly with a dark aura!",TRUE,ch,obj,0,TO_ROOM);
  } else {
    act("$p glows for a brief moment!",FALSE,ch,obj,0,TO_CHAR);
    act("$p glows for a brief moment!",TRUE,ch,obj,0,TO_ROOM);
  }
}

void spell_harden_weapon(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  int chance = 0;
  int charlevel = 0;

  assert(ch && obj);

  if (GET_ITEM_TYPE(obj) != ITEM_FIREWEAPON) {
       send_to_char("You can't make that any harder!\n\r", ch);
       return;
   }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_HARDEN)) {
        send_to_char("This weapon is already hard enough don't you think?!\n\r", ch);
        return;
  }

  chance = number(1, 101);
  charlevel = (GET_LEVEL(ch, CLASS_RANGER));

  if(charlevel > 50)
       chance += 1;
  if(charlevel > 70)
      chance += 1;
  if(charlevel > 90)
      chance += 1;
  if(charlevel > 110)
      chance += 1;
  if(charlevel > 124)
      chance += 1;
  if(charlevel > MAX_MORT)
    	chance = 100;

  if(chance > 20)
  {
     SET_BIT(obj->obj_flags.extra_flags, ITEM_HARDEN);
     act("$p glows briefly and becomes stronger!",FALSE,ch,obj,0,TO_CHAR);
     act("$p glows briefly and becomes stronger!",TRUE,ch,obj,0,TO_ROOM);
  } else { /* failed */
/*       SET_BIT(obj->obj_flags.extra_flags, ITEM_BRITTLE);
      act("$p glows briefly and becomes completely unflexable!",FALSE,ch,obj,0,TO_CHAR);
      act("$p glows briefly and becomes completely unflexable!",TRUE,ch,obj,0,TO_ROOM);
  */
   }
}

/*****************************/
/*** misc offensive spells ***/
/*****************************/

void spell_blindness(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)

{
  int chance;
  assert(ch && victim);
  

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (affected_by_spell(victim, SPELL_BLINDNESS))
    return;

  if (ImpSaveSpell(victim, SAVING_SPELL,blindnessSpellMod))
  {
    send_to_char("You avoid being blinded!\n\r", victim);
    act("$n avoids being blinded!", TRUE, victim, 0, 0, TO_ROOM);
    return;
  }

  MakeAffect(ch, victim, type,
             SPELL_BLINDNESS, -MAX((int)(level/15), 1), APPLY_HITNDAM, AFF_BLIND,
	     10, 1, FALSE, FALSE, FALSE, NULL);
             send_to_char("You have been blinded!\n\r", victim);
             act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_sunray(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  

  if (!OUTSIDE(ch)) {
    send_to_char("You must be outdoors to call upon the sun.\n\r",ch);
    return;
  }

  if (weather_info.sunlight==SUN_DARK) {
    send_to_char("The sun is not out to help you.\n\r",ch);
    return;
  }

  send_to_char("You call upon the sun for assistance!\n\r",ch);
  act("$n calls upon the sun for assistance!",TRUE,ch,0,0,TO_ROOM);

  spell_blindness(level, ch, type, victim, obj);
}

void spell_paralyze(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  assert(victim);

  if (IS_AFFECTED(victim, AFF_PARALYSIS))
    return;

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (ImpSaveSpell(victim, SAVING_PARA,paraSpellMod )) {
      send_to_char("Your attempt to paralyze failed!\n\r", ch);
      return;
   }
/*   MakeAffect(ch, victim, SPELL_TYPE_POTION,
	     SPELL_PARALYSIS, 0, APPLY_NONE, AFF_PARALYSIS,
	     1, 0, FALSE, FALSE, FALSE, NULL);
*/
    stun_opponent(ch, victim, SPELL_PARALYSIS, paraSpellMod);
    act("Your limbs freeze in place!",FALSE,victim,0,0,TO_CHAR);
    act("$n is paralyzed!",TRUE,victim,0,0,TO_ROOM);
    victim->specials.position=POSITION_STUNNED;
  
}

void spell_curse(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && (victim || obj));
  

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    act("$p briefly glows with a red aura.", FALSE, ch, obj, 0, TO_CHAR);
    act("$p briefly glows with a red aura.", TRUE, ch, obj, 0, TO_ROOM);
    return;
  }

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (affected_by_spell(victim, SPELL_CURSE))
    return;

  if (ImpSaveSpell(victim, SAVING_SPELL,curseSpellMod))
    return;

        MakeAffect(ch, victim, type,
                   SPELL_CURSE, MAX(level/10, 1), APPLY_SAVE_ALL, 0,
                   10, 1, FALSE, FALSE, FALSE, NULL);
  
  send_to_char("You feel very uncomfortable!\n\r", victim);
  act("$n is cursed!", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_faerie_fire (ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE))
    return;

  if (ImpSaveSpell(victim,SAVING_SPELL,faeriefireSpellMod))
    return;

  MakeAffect(ch, victim, 0,
	     SPELL_FAERIE_FIRE, 10 + level, APPLY_AC, 0,
	     10, 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are briefly surrounded by a redish glow!\n\r",victim);
  act("$n is briefly surrounded by a redish glow!",TRUE,victim,0,0,TO_ROOM);
}

void spell_faerie_fog (ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int found;
  struct char_data *tch;

  assert(ch);
  

  if (!real_roomp(ch->in_room))
    return;

  send_to_char("A redish glowing fog envelops the area.\n\r",ch);
  act("A redish glowing fog envelops the area.",FALSE,ch,0,0,TO_ROOM);

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (!in_group(tch, ch) && !IS_IMMORTAL(tch)) {
      found=FALSE;
      if (affected_by_spell(tch,SPELL_INVISIBLE)) {
        affect_from_char(tch,SPELL_INVISIBLE);
        found=TRUE;
      }
      if (IS_AFFECTED(tch, AFF_INVISIBLE)) {
        REMOVE_BIT(AFF_FLAGS(tch), AFF_INVISIBLE);
        found=TRUE;
      }
      if (affected_by_spell(tch,SKILL_CHAMELEON)) {
        affect_from_char(tch,SKILL_CHAMELEON);
        found=TRUE;
      }
      if (IS_AFFECTED(tch, AFF_HIDE)) {
        REMOVE_BIT(AFF_FLAGS(tch), AFF_HIDE);
        found=TRUE;
      }
      if (affected_by_spell(tch,SKILL_SNEAK)) {
        affect_from_char(tch,SKILL_SNEAK);
        found=TRUE;
      }
      if (IS_AFFECTED(tch, AFF_SNEAK)) {
        REMOVE_BIT(AFF_FLAGS(tch), AFF_SNEAK);
        found=TRUE;
      }
      if (found)
        send_to_char("You are revealed!\n\r", tch);
        act("$n is revealed!",TRUE,tch,0,0,TO_ROOM);
    }
  }
}

void spell_weakness(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (affected_by_spell(victim,SPELL_WEAKNESS))
    return;

  if (ImpSaveSpell(victim, SAVING_SPELL,weaknessSpellMod))
    return;

  MakeAffect(ch, victim, type,
	     SPELL_WEAKNESS, -MAX(1, level / 10), APPLY_STR, 0,
	     1, 1, TRUE, FALSE, FALSE, NULL);

  send_to_char("You feel weakend!\n\r",victim);
  act("$n is weakened!",TRUE, victim, 0, 0, TO_ROOM);
}

void spell_web(ubyte level, struct char_data *ch, int type,
               struct char_data *victim, struct obj_data *obj)
{
  int dam;
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);

  if (ImpSaveSpell(victim,SAVING_PARA, webSpellMod)) {
    send_to_char("You avoid some sticky webbing!\n\r",victim);
    act("$n avoids getting encased in a sticky webbing!\n\r",
        TRUE,victim,0,0,TO_ROOM);
    return;
  } 

  dam = ApplyImmuneToDamage(victim, level, IMM_HOLD);
  MakeAffect(ch, victim, type,
	     SPELL_WEB, dam, APPLY_MOVE, 0,
	     level, 1, TRUE, FALSE, FALSE, NULL);

  if (GET_MOVE(victim)+dam > GET_MAX_MOVE(victim))
    GET_MOVE(victim) = GET_MAX_MOVE(victim);
  else
    GET_MOVE(victim) -= dam;

  send_to_char("You are encased in a sticky webbing!\n\r",victim);
  act("$n is encased in a sticky webbing!",TRUE,victim,0,0,TO_ROOM);
}

void spell_sleep(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (GET_POS(victim)<=POSITION_SLEEPING)
    return;

  if(ImpSaveSpell(victim, SAVING_SPELL,sleepSpellMod)) {
    if (!victim->specials.fighting)
      set_fighting(victim, ch);
    return;
  }
  if (victim->specials.fighting)
  {
    stop_opponents(victim, victim->in_room);
    stop_fighting(victim);
  }
    

  MakeAffect(ch, victim, type,
	     SPELL_SLEEP, 0, APPLY_NONE, AFF_SLEEP,
	     MIN(5, level), 0, FALSE, FALSE, FALSE, NULL);

  send_to_char("You suddenly feel very sleepy... zzz...\n\r",victim);
  act("$n lets out a big yawn and goes to sleep.",TRUE,victim,0,0,TO_ROOM);
  victim->specials.position=POSITION_SLEEPING;
}

void spell_inspire(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_INSPIRE)) {
    return;
  }                                    

  if (!SkillChance(ch, victim, 50, 0, SPLMOD_INT | SPLMOD_CHA | SPLMOD_WIS,
		   SPELL_INSPIRE)) {
    act("$N fails to be inspired by your music.",FALSE,ch,0,victim,TO_CHAR);
    act("$n sings a song of inspiration for you.",TRUE,ch,0,victim,TO_VICT);
    act("$N ignores $n's lousy attempt at singing.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }
  
  if (affected_by_spell(victim,SKILL_ADRENALIZE)) {
    act("Ooo I think $N is inspired enough already!.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  MakeAffect(ch, victim, SPELL_TYPE_SPELL,
             SKILL_ADRENALIZE, GetMaxLevel(ch)/10, APPLY_HITNDAM,
  	     0, 0, 4, FALSE,FALSE,FALSE,NULL);

  send_to_char("You feel inspired!\n\r",victim);
  act("$n suddenly gets a wild look in his eyes!.",TRUE,victim,0,0,TO_ROOM);
}

void spell_slow(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_SLOW)) {
    return;
  }                              
      
  if (victim->specials.fighting) {
    act("$N cannot enjoy your music right now.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  
  if (!SkillChance(ch, victim, 30, IMM_BARD, SPLMOD_DEX | SPLMOD_INT |
		   SPLMOD_CHA, SPELL_SLOW)) {
    act("$N fails to enjoy your music.",FALSE,ch,0,victim,TO_CHAR);
    act("$n sings a languid song - with no effect.",TRUE,ch,0,victim,TO_VICT);
    act("$N ignores $n's slowing song.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }

  if (IS_AFFECTED2(victim, AFF2_SLOW)) {
    act("$N is already moving like a turtle....",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  
  MakeAffect(ch, victim, type,
	     SPELL_SLOW, 0, APPLY_AFF2, AFF2_SLOW,
	     MIN(5, level), 1, FALSE, FALSE, FALSE, NULL);
  
  send_to_char("You suddenly feel very legtharic.. you ..slow... down....\n\r",victim);
  act("$n seems to be moving much more slowly now.",TRUE,victim,0,0,TO_ROOM);
}

void spell_haste(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);

  if (!is_hold_instrument(ch,SPELL_HASTE)) {
    return;
  }                              
      
  if (victim->specials.fighting) {
    act("$N cannot enjoy your music right now.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  
  if (!SkillChance(ch, victim, 30, IMM_BARD, SPLMOD_DEX | SPLMOD_INT |
		   SPLMOD_CHA, SPELL_HASTE)) {
    act("$N fails to enjoy your music.",FALSE,ch,0,victim,TO_CHAR);
    act("$n's song does not seem to hasten you.",TRUE,ch,0,victim,TO_VICT);
    act("$N ignores $n's hastening song.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }

  if (IS_AFFECTED2(victim, AFF2_HASTE)) {
    act("$N is already moving as fast as possible",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  
  MakeAffect(ch, victim, type,
	     SPELL_HASTE, 0, APPLY_AFF2, AFF2_HASTE,
	     MIN(5, level), 1, FALSE, FALSE, FALSE, NULL);
  
  send_to_char("You suddenly feel yourself speeding up.\n\r",victim);
  act("$n seems to be moving much more quickly now.",TRUE,victim,0,0,TO_ROOM);
}

void spell_despair(ubyte level, struct char_data *ch, int type,
		   struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_DESPAIR)) {
    return;
  }                                    
  if (GET_POS(victim)<=POSITION_SLEEPING || victim->specials.fighting) {
    act("$N cannot enjoy your music right now.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  if (!SkillChance(ch,victim,25,IMM_BARD,SPLMOD_INT |
		   SPLMOD_WIS, SPELL_DESPAIR)) {
    act("$N fails to enjoy your music.",FALSE,ch,0,victim,TO_CHAR);
    act("$n sings a saddening song - with no effect.",TRUE,ch,0,victim,TO_VICT);
    act("$N ignores $n's depressive song.",TRUE,ch,0,victim,TO_ROOM);
    return;
    }

  if (IS_AFFECTED2(victim,AFF2_DESPAIR)) {
    act("$N is already crying their eyes out!",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  MakeAffect(ch, victim, type,
             SPELL_DESPAIR, 0, APPLY_AFF2, AFF2_DESPAIR,
             MIN(5, level), 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("You feel the weight of despair on your shoulders.\n\r",victim);
  act("$n's shoulders bend with despair.",TRUE,victim,0,0,TO_ROOM);
}


void spell_lullabye(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_LULLABYE)) {
    return;
  }                                    
  if (GET_POS(victim)<=POSITION_SLEEPING || victim->specials.fighting) {
    act("$N cannot enjoy your music right now.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  if(!IS_IMMORTAL(ch) && ImpSaveSpell(victim, SAVING_SPELL, sleepSpellMod)) {
    if (number(1,125)<=GetMaxLevel(victim)) {
      act("The now angry $N does not like your lullabye!",FALSE,ch,0,victim,TO_CHAR);
      act("You are angered by $n's attempt to sing you to sleep!",TRUE,ch,0,victim, TO_VICT);
      act("$N becomes very critical of $n's singing - and attacks!",TRUE,ch,0,victim, TO_ROOM);
       AddHated(victim, ch);
      if (!victim->specials.fighting)
        SetVictFighting(ch, victim);
    } else {
      act("$N fails to enjoy your music.",FALSE,ch,0,victim,TO_CHAR);
      act("$n sings an annyoing lullabye at you - with no effect.",TRUE,ch,0,victim, TO_VICT);
      act("$N ignores $n's lullabye.",TRUE,ch,0,victim,TO_ROOM);
    }
    return;
  }

  MakeAffect(ch, victim, type,
             SPELL_LULLABYE, 0, APPLY_NONE, AFF_SLEEP,
             MIN(5, level), 1, FALSE, FALSE, FALSE, NULL);

  send_to_char("You suddenly feel very sleepy... zzz...\n\r",victim);
  act("$n lets out a big yawn and goes to sleep.",TRUE,victim,0,0,TO_ROOM);
  victim->specials.position=POSITION_SLEEPING;
}

void spell_blur(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_LULLABYE)) {
    return;
  }


  if (number(1,3) != 1) {
     act("Your attempt at singing left something to be desired.",FALSE,ch,0,victim, TO_CHAR);
     return;
  }

  MakeAffect(ch, victim, type,
             SKILL_ILLUSIONARY_SHROUD, 0, APPLY_NONE, AFF_ILLUSION,
             25, 10, FALSE, FALSE, FALSE, NULL);

  send_to_char("Your shape starts to waver and blur!\n\r",victim);
  act("$n starts to waver and blur!",TRUE,victim,0,0,TO_ROOM);
}

void spell_silence(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!is_hold_instrument(ch,SPELL_LULLABYE)) {
    return;
  }

  if (affected_by_spell(victim, SPELL_SILENCE)) {
    act("$N is already silenced.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  if (ImpSaveSpell(victim, SAVING_SPELL, silenceSpellMod)) {
     act("$N seems disinterested in your lousy attempt at singing.",FALSE,ch,0,victim,TO_CHAR);
     act("$N seems disinterested in $s's singing.",FALSE,ch,0,victim,TO_ROOM);
     return;
  }
  MakeAffect(ch, victim, type,
             SPELL_SILENCE, 0, APPLY_NONE, AFF_SILENCE,
             25, 10, FALSE, FALSE, FALSE, NULL);
  send_to_char("Your mouth suddenly seems sealed shut - you cannot speak!\n\r",victim);
  act("$n suddenly has trouble speaking!",TRUE,victim,0,0,TO_ROOM);
}

void spell_unweave(ubyte level, struct char_data *ch, int type,
                    struct char_data *vict, struct obj_data *obj)
{

  assert(ch);
     
  if (!is_hold_instrument(ch,SPELL_UNWEAVE)) {
    return;
  }

  EACH_CHARACTER(iter, vict)
    {
      if (ch->in_room==vict->in_room)
	  {
	   if (ch!=vict) 
	   {
	    if(IS_IMMORTAL(vict))
	      send_to_char( "Some puny mortal tries to hurt you.\n\r", vict);
	    else
              spell_dispel_magic(level,ch, type, vict, obj); 
	     
	   }
	   }
	    else if(real_roomp(ch->in_room)->zone ==
		real_roomp(vict->in_room)->zone)
         act("You here a song off in the distance",FALSE, vict, 0,0,TO_CHAR);
        
    }
  END_AITER(iter);

  GET_MANA(ch) -= 50;
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

	
				   

/**********************/
/*** healing spells ***/
/**********************/

void spell_cure_light(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  int healpoints;
  int gain;
  
  assert(victim);
  if(HasClass(ch, CLASS_CLERIC))
     healpoints = MAX(10,dice(GET_LEVEL(ch, CLERIC_LEVEL_IND), 1));
  else
     healpoints = dice(2,3);

  if (!IsUndead(victim) || (ch == victim) || IS_PC(real_character(victim))) {
    if ((healpoints+GET_HIT(victim)) > hit_limit(victim))
    {
      gain = 4* ( hit_limit(victim) - GET_HIT(victim));
      GET_HIT(victim) = hit_limit(victim);
    }
    else
    {
      gain = healpoints * 4;
      GET_HIT(victim) += healpoints;
    }

    update_pos(victim);

    send_to_char("You feel a little better.\n\r", victim);
    act("$n feels a little better.", TRUE, victim, 0, 0, TO_ROOM);
    if(HasClass(ch, CLASS_CLERIC))
        gain_exp_heal(ch, gain);
  } else {
    struct room_data *rp = real_roomp(victim->in_room);
    if (rp && (rp->room_flags & PEACEFUL)) {
      send_to_char("Your heal spell has no effect in this room.\n\r", ch);
      act("The heal spell quickly dissipates upon touching $n's undead flesh.\n\r", TRUE, victim, 0, 0, TO_ROOM);
      return;
    }
     
    healpoints *= 4;
    send_to_char("Pain rushes through your undead body!\n\r", victim);
    act("$n is hurt by healing forces!", TRUE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, healpoints, SPELL_HEAL);
  }
}

void spell_cure_serious(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  int healpoints;
  int gain;
  assert(victim);
  
  if(HasClass(ch, CLASS_CLERIC))
     healpoints = MAX(25,dice(GET_LEVEL(ch, CLERIC_LEVEL_IND), 2));
  else
     healpoints = 25;


  if(!IsUndead(victim) || (ch == victim) || IS_PC(real_character(victim))) {
    if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    {
      gain = (hit_limit(victim) - GET_HIT(victim)) * 3;
      GET_HIT(victim) = hit_limit(victim);
    }
    else
    {
      gain = healpoints *3;
      GET_HIT(victim) += healpoints;
    }
    update_pos(victim);

    send_to_char("You feel better.\n\r", victim);
    act("$n feels better.", TRUE, victim, 0, 0, TO_ROOM);
    if(HasClass(ch, CLASS_CLERIC))
    	gain_exp_heal(ch, gain);
  } else {
    struct room_data *rp = real_roomp(victim->in_room);
    if (rp && (rp->room_flags & PEACEFUL)) {
      send_to_char("Your heal spell has no effect in this room.\n\r", ch);
      act("The heal spell quickly dissipates upon touching $n's undead flesh.\n\r", TRUE, victim, 0, 0, TO_ROOM);
      return;
    }
       
    healpoints *= 4;
    send_to_char("Pain rushes through your undead body!\n\r", victim);
    act("$n is hurt by healing forces!", TRUE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, healpoints, SPELL_HEAL);
  }
}

void spell_cure_critic(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int healpoints;
  int gain;
  
  assert(victim);

  if(HasClass(ch, CLASS_CLERIC))
    healpoints = MAX(50, dice(GET_LEVEL(ch, CLERIC_LEVEL_IND), 3));
  else
    healpoints = 50;
    
  if (!IsUndead(victim) || (ch == victim) || IS_PC(real_character(victim))) {
    if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    {
      gain = 2 * (hit_limit(victim) - GET_HIT(victim));
      GET_HIT(victim) = hit_limit(victim);
    }
    else
    {
      gain = healpoints * 2;
      GET_HIT(victim) += healpoints;
    }
    update_pos(victim);

    send_to_char("You feel much better.\n\r", victim);
    act("$n feels much better.", TRUE, victim, 0, 0, TO_ROOM);
    if(HasClass(ch, CLASS_CLERIC))
      gain_exp_heal(ch, gain);
  } else {
    struct room_data *rp = real_roomp(victim->in_room);
    if (rp && (rp->room_flags & PEACEFUL)) {
      send_to_char("Your heal spell has no effect in this room.\n\r", ch);
      act("The heal spell quickly dissipates upon touching $n's undead flesh.\n\r", TRUE, victim, 0, 0, TO_ROOM);
      return;
    }
    
    healpoints *= 4;
    send_to_char("Pain rushes through your undead body!\n\r", victim);
    act("$n is hurt by healing forces!", TRUE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, healpoints, SPELL_HEAL);
  }
}

void spell_heal(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  int healpoints;
  int gain;

  assert(victim);

  spell_cure_blind(level, ch, type, victim, obj);
  spell_remove_poison(level, ch, type, victim, obj);

  if(type == SPELL_TYPE_POTION) {
    healpoints = MAX(100,dice(level,4));
  }
  else if(HasClass(ch, CLASS_CLERIC))
     healpoints = MAX(100,dice(GET_LEVEL(ch, CLERIC_LEVEL_IND), 4));
  else
     healpoints = 100;
     
  if (!IsUndead(victim) || (ch == victim) || IS_PC(real_character(victim))) {
    if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) ) {
      gain = hit_limit(victim) - GET_HIT(victim);
      GET_HIT(victim) = hit_limit(victim);
      }
    else {
      gain = healpoints;
      GET_HIT(victim) += healpoints;
    }
   
    update_pos(victim);
  
    send_to_char("A warm feeling fills your body.\n\r", victim);
    act("$n is healed.", TRUE, victim, 0, 0, TO_ROOM);

    if(HasClass(ch, CLASS_CLERIC))
      gain_exp_heal(ch, gain);
  } else {
    struct room_data *rp = real_roomp(victim->in_room);
    if (rp && (rp->room_flags & PEACEFUL)) {
      send_to_char("Your heal spell has no effect in this room.\n\r", ch);
      act("The heal spell quickly dissipates upon touching $n's undead flesh.\n\r", TRUE, victim, 0, 0, TO_ROOM);
      return;
    }
       
    healpoints *= 4;
    send_to_char("Pain rushes through your undead body!\n\r", victim);
    act("$n is hurt by healing forces!", TRUE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, healpoints, SPELL_HEAL);
  }
}

void spell_regen(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  assert (victim);
  

  if (IS_AFFECTED(victim, AFF_REGENERATE))
    return;

  send_to_char("You feel a regenerative force enter your body.\n\r", victim);
  act("A regenerative force is channeled into $n.",TRUE,victim,0,0,TO_ROOM);

  MakeAffect(ch, victim, type, SPELL_REGEN, 0, APPLY_NONE, AFF_REGENERATE,
             3, 5, FALSE, FALSE, FALSE, NULL);
}

void spell_empathic_heal(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
    int max_heal, reqd_heal, will_heal;
    float heal_percent;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    max_heal=GET_HIT(ch)-1;

    if (max_heal<10) {
	send_to_char("You cannot possibly assist in your condition!\n\r",ch);
	return;
    }

    reqd_heal=GET_MAX_HIT(victim)-GET_HIT(victim);
    if (reqd_heal>max_heal)
	will_heal=max_heal;
    else
	will_heal=reqd_heal;

    send_to_char("Healing power courses through you.\n\r",victim);
    act("$n uses empathic heal on $N.", TRUE, ch, 0, victim, TO_ROOM);
    act("You use empathic healing to aid $N.", FALSE, ch, 0, victim, TO_CHAR);
    GET_HIT(victim) += will_heal;
    spell_cure_blind(level, ch, type, victim, obj);
    spell_remove_poison(level, ch, type, victim, obj);
    update_pos(victim);

    if (!IS_IMMORTAL(ch))
    {
	GET_HIT(ch) -= will_heal;
	if (GET_HIT(ch)<200) {
	    act("You collapse from the great physical and mental effort of healing.",FALSE, ch, 0, 0, TO_CHAR);
            act("$n collapses from the great physical and mental effort of healing.", TRUE, ch, 0, 0, TO_ROOM);
	    WAIT_STATE(ch, PULSE_VIOLENCE*6);
	    ch->specials.position = POSITION_STUNNED;
	}
    }

    heal_percent = (float) will_heal /
      (GET_MAX_HIT(ch) ? GET_MAX_HIT(ch) : -1);
    GET_ALIGNMENT(ch) =
      MIN(1000, (GET_ALIGNMENT(ch) + (int) (25.0 * heal_percent)));
}

/****************************************/
/*** transportation and summon spells ***/
/****************************************/

void spell_astral_walk(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[256];
  struct room_data *rp;
  int rnum;
  assert(ch);

  if(!(rp = real_roomp(ch->in_room)) || IS_SET(rp->room_flags, NO_TRAVEL_OUT))
    {
      send_to_char("An unseen force prevents you from entering the Astral Plane.\n\r", ch);
      return;
    }

  switch(number(0,3))
  {
  case 0: rnum=7601; break;
  case 1: rnum=7618; break;
  case 2: rnum=7633; break;
  case 3: rnum=7650; break;
  default: rnum=0; log_msg("Error in astral walk -- undefined target. Sending char to void"); break;
  }

  rp=real_roomp(rnum);
  if (!rp)
  {
      sprintf(buf, "Astral Walk Failed -- Target room (%d) does not exist.",
	      rnum);
      log_msg(buf);
      send_to_char("I'm sorry, this spell is having technical difficulties.\n\r",ch);
    return;
  }
  
  act("$n opens a rift in space and steps in!!",TRUE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, rnum);
  act("You are blinded for a moment as $n appears in a flash of light!",TRUE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}

void spell_word_of_recall(ubyte level, struct char_data *ch, int type,
			  struct char_data *victim, struct obj_data *obj)
{
  int location;
  struct room_data* rp;
  
  assert(ch);

  if (!IS_PC(ch)) {
    send_to_char("Monsters cannot recall!\n\r", ch);
    return;
  }

  if (GET_HOME(ch))
    location = GET_HOME(ch);
  else
    location = 7;

  if ((rp = real_roomp(location)) == NULL)
  {
    send_to_char("You are completely lost!\n\r", victim);
    return;
  }

  if((IS_SET(rp->room_flags, IMMORT_RM) && !IS_GOD(ch)) ||
     (IS_SET(rp->room_flags, GOD_RM) && TRUST(ch) < TRUST_LORD))
  {
    send_to_char("An unknown force keeps you where you are.\n\r", ch);
    return;
  }

  if(!(rp = real_roomp(ch->in_room)) || IS_SET(rp->room_flags, NO_RECALL))
  {
    send_to_char("An unknown force keeps you here.\n\r", ch);
    return;
  }
    
  act("$n disappears in a puff of smoke!", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  act("$n suddenly appears in a puff of smoke!", TRUE, ch, 0, 0, TO_ROOM);
  do_look(ch, "",15);
}

void spell_nature_walk(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  struct room_data *rp;

  assert(ch && victim);

  if (!wilderness(ch)) {
    send_to_char("You must be outdoors to go on a nature walk.\n\r",ch);
    return;
  }

  /*
    Removed this check to allow nature walk to work like psi's gate,
    although only outdoors - raist
  */
  /*
  if (!IS_PC(victim)){
    act("You have no natural awareness of $N.  The nature portal fails.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }
  */

  rp = real_roomp(victim->in_room);
  if (!wilderness(victim)) {
    send_to_char("You cannot walk there since it is not outdoors.\n\r",ch);
    return;
  }

  if(!travel_check(ch, victim))
  {
    send_to_char("An unknown force prevents you from travelling to this creature.\n\r", ch);
    return;
  }

  act("Nature comes to the assistance of $n and whisks them away!",
      TRUE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, victim->in_room);

  act("You feel a rush of wind as $n suddenly appears!",TRUE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}

void spell_teleport(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  int to_room;
  struct room_data *room;

  assert(ch && victim);

  if (victim != ch) {
    if (saves_spell(victim,SAVING_SPELL, IMM_HOLD) || IS_IMMORTAL(victim)) {
      send_to_char("Your spell has no effect.\n\r",ch);
      if (!IS_PC(victim)) {
        if (!victim->specials.fighting)
          set_fighting(victim, ch);
      }
      return;
    } else {
      send_to_char("You feel momentarily disoriented.\n\r",victim);
    }
  } else {
    victim=ch;			/* the caster is the actual target then */
  }

  if(!(room = real_roomp(ch->in_room)) ||
     IS_SET(room->room_flags, NO_TRAVEL_OUT))
  {
    send_to_char("Nothing happens.\n", ch);
    return;
  }
  
  /*** find a non-private room to dump them in ***/
  do {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
  } while (!room || IS_SET(room->room_flags, NO_TRAVEL_IN));

  act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);

  act("$n slowly fades in to existence.", TRUE, victim,0,0,TO_ROOM);
  do_look(victim, "", 0);

  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && !IS_GOD(victim))
    do_death_trap(victim);
}

void spell_summon(ubyte level, struct char_data *ch, int type,
  struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp;
  int count=0, percent;
  char buf[MAX_STRING_LENGTH];

  assert(ch && victim);

  if (victim->in_room == NOWHERE) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if(IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
  {
     RawSummon(victim, ch);
     return;
  }


  if(!travel_check(victim, ch))
  {
    send_to_char("An unseen presence in the room stops you from calling others here.\n\r", ch);
    return;
  }

  if (IS_SET(victim->specials.flags, PLR_SUMMON))
  {
    sprintf(buf, "%s cannot be summoned.\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    sprintf(buf, "%s just tried to summon you.\r\n", GET_NAME(ch));
    send_to_char(buf, victim);
    return;
  }
 
  if(IS_AFFECTED2(victim, AFF2_NOSUMMON) && !IS_IMMORTAL(ch))
  {
    sprintf(buf, "%s cannot be summoned.\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    sprintf(buf, "%s just tried to summon you.\r\n", GET_NAME(ch));
    send_to_char(buf, victim);
    return;
  }

  /* Percent change is based on the following rules:
  *   50% chance if equal in level
  *   5% increase/decrease for each level difference
  *   at 100 or 0%, give 99 or 1% chance.
  */
  percent = GetMaxLevel(ch) - GetMaxLevel(victim);
  percent *= 5;
  percent += 50;
  if (percent > 99) percent = 99;
  if (percent < 1) percent = 1;
  
  if(number(1,100) > percent)  
  {
    send_to_char("It is rude to attempt to summon someone so much greater than you.\r\n", ch);
    return;
  }
    
  if (IS_PC(victim) && !IS_IMMORTAL(victim)) {
    RawSummon(victim, ch);
    return;
  }

  for(tmp=real_roomp(victim->in_room)->people; tmp; tmp = tmp->next_in_room)
    count++;

  if (count==0) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  /* pick a victim at random from within the room of the target */
  count=number(1,count);
  for (tmp=real_roomp(victim->in_room)->people; (--count>0) && tmp;  tmp = tmp->next_in_room);

  if (tmp)
    RawSummon(tmp, ch);
  else {
    send_to_char("You failed.\n\r", ch);
    return;
  }
}

void RawSummon( struct char_data *victim, struct char_data *ch)
{
  room_num target;
  struct char_data *tmp;
  struct obj_data *obj, *n;
  int j;
  int will_kill=FALSE;
  char buf[MAX_STRING_LENGTH];

  if (IS_IMMORTAL(victim)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if(!IS_PC(victim)) {
    if(NewSkillSave(ch, victim, SPELL_SUMMON, 0, IMM_HOLD)) {
      send_to_char("You failed.\n\r", ch);
      return;
    }

    if(GetMaxLevel(victim) > (GetMaxLevel(ch)+3)) {

      /*** destroy any items the mob has on them ***/
      act("$N struggles violently, and all of $S items are destroyed!",
          TRUE,ch,0,victim,TO_CHAR);

      for (j=0; j<MAX_WEAR; j++) {
        if (victim->equipment[j]) {
          obj=unequip_char(victim, j);
          extract_obj(obj);
        }
      }
      for (obj=victim->carrying; obj; obj=n) {
        n=obj->next_content;
        obj_from_char(obj);
        extract_obj(obj);
      }

      /*** our mob is not happy being summoned by such a lowly mortal ***/
      AddHated(victim, ch);
      will_kill=TRUE;
    }

    if(IS_SET(victim->specials.mob_act, ACT_AGGRESSIVE) ||
       IS_SET(victim->specials.mob_act, ACT_META_AGG)) {
      sprintf(buf, "%s summoned aggressive : %s",
              GET_IDENT(ch), GET_NAME(victim));
      log_msg(buf);
    }
  }

  if(!will_kill && !IS_IMMORTAL(ch)) {
    send_to_char("A wave of nausea overcomes you.  You collapse!\n\r",ch);
    WAIT_STATE(ch, PULSE_VIOLENCE*6);
    GET_POS(ch) = POSITION_STUNNED;
  }

  act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);
  target = ch->in_room;
  char_from_room(victim);
  char_to_room(victim,target);

  act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
  act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
  do_look(victim,"",15);

  for (tmp=real_roomp(victim->in_room)->people; tmp; tmp = tmp->next_in_room) {
    if(!IS_PC(tmp) &&
       (IS_SET(tmp->specials.mob_act, ACT_AGGRESSIVE) ||
        IS_SET(tmp->specials.mob_act, ACT_META_AGG))) {
      act("$n growls at you furiously!", 1, tmp, 0, ch, TO_VICT);
      act("$n growls furiously at $N!", 1, tmp, 0, ch, TO_NOTVICT);
      if (number(0,6)==0 && CAN_SEE(tmp,ch))
        hit(tmp, ch, TYPE_UNDEFINED);
    }
  }
}

/***********************/
/*** follower spells ***/
/***********************/

void fix_mob_bits(struct char_data *mob)
{
    /* remove any nasty bits the pet may have set */
    if (IS_SET(mob->specials.mob_act, ACT_AGGRESSIVE))
	REMOVE_BIT(mob->specials.mob_act, ACT_AGGRESSIVE);
    if (IS_SET(mob->specials.mob_act, ACT_META_AGG))
	REMOVE_BIT(mob->specials.mob_act, ACT_META_AGG);
    if (!IS_SET(mob->specials.mob_act, ACT_SENTINEL))
	SET_BIT(mob->specials.mob_act, ACT_SENTINEL);
}

void spell_monsum(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
    assert(ch);
    

    victim->points.mana=0;
    char_to_room(victim, ch->in_room);

    act("$n waves $s hand, and $N appears!", FALSE, ch, 0, victim, TO_ROOM);
    act("You wave your hand, and $N appears!", FALSE, ch, 0, victim, TO_CHAR);

    fix_mob_bits(victim);
    add_follower(victim, ch, 0);

    victim->player.level[WARRIOR_LEVEL_IND] = MIN((ubyte) GetMaxLevel(ch)/2, MAX_MORT);
    UpdateMaxLevel(victim);

    /* make the pet charmed */
    MakeCharmed(ch, victim, level, SPELL_MONSUM, -4);
}

void spell_ansum(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  assert(ch);
  

  victim->points.mana=0;
  char_to_room(victim, ch->in_room);

  act("$n waves $s hand, and summons $N from the wilds!",
      FALSE,ch,0,victim,TO_ROOM);
  act("You summon $N from the wilds!",FALSE,ch,0,victim,TO_CHAR);

  fix_mob_bits(victim);
  add_follower(victim, ch, 0);

  victim->player.level[WARRIOR_LEVEL_IND] = MIN((ubyte) GetMaxLevel(ch)/2, MAX_MORT);
  UpdateMaxLevel(victim);
  /* make the pet charmed */
  MakeCharmed(ch, victim, level, SPELL_ANSUM, -4);
}

void spell_animate_dead(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *corpse)
{
    struct char_data *mob;
    struct obj_data *obj_object, *next_obj;
    char buf[MAX_STRING_LENGTH];
    int align_mod = 0;

    /* make sure it's a corpse */
    if (!IS_CORPSE(corpse)) {
	send_to_char("That is not a corpse!\n\r",ch);
	return;
    }

    if(!CountFollowers(ch))
	return;

    /* load up a zombie and set some things on it */
    if(!(mob=make_mobile(VMOB_ZOM_PET, VIRTUAL))) {
	log_msg("screwup in animate dead.");
	send_to_char("This spell is experiencing technical difficulty.\n\r",
		     ch);
	return;
    }

    act("With mystic power, $n animates a corpse.",TRUE,ch,0,0,TO_ROOM);
    act("$N slowly rises from the ground.", TRUE, ch, 0, mob, TO_ROOM);

    mob->points.mana=0;
    char_to_room(mob, ch->in_room);
    fix_mob_bits(mob);
    add_follower(mob, ch, 0);

    sprintf(buf,"%s is here, slowly animating\n\r",OBJ_SHORT(corpse));
    ss_free(mob->player.long_descr);
    mob->player.long_descr = ss_make(buf);
    
    //align modifier
    if (GET_ALIGNMENT(ch) > 0)
       align_mod = -1*number(GET_ALIGNMENT(ch)/4, GET_ALIGNMENT(ch)/2);
    else
       align_mod = number(GET_ALIGNMENT(ch)/-4, GET_ALIGNMENT(ch)/-2);
    /* Modify hitpoints based off how evil they are to begin with */
    mob->points.max_hit = MAX(1, dice((level+1),8) + align_mod);
    mob->points.hit = (int)(mob->points.max_hit/2);
    mob->player.level[WARRIOR_LEVEL_IND] = MIN ((ubyte)GetMaxLevel(ch)/2, MAX_MORT);
    UpdateMaxLevel(mob);
    GET_RACE(mob) = RACE_UNDEAD;
    mob->player.sex = 0;
    SET_BIT(AFF_FLAGS(mob), AFF_CHARM);

    /* transfer items from the corpse to the zombie */
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    for (obj_object=corpse->contains; obj_object; obj_object=next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, mob);
    }
    
    /* Change align for doing such evil things! -Novak */
    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 10);
    /* get rid of corpse */
    extract_obj(corpse);
}

void spell_possession(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *corpse)
{
  struct char_data *mob;
  struct obj_data *obj_object, *next_obj;
  char buf[MAX_STRING_LENGTH];
  int align_mod = 0;
  /* make sure it's a corpse */
  if (!IS_CORPSE(corpse)) {
    send_to_char("That is not a corpse!\n\r",ch);
    return;
  }

  if(!CountFollowers(ch))
    return;

  /* load up a possessor and set some things on it */
  if(!(mob=make_mobile(VMOB_POSS_PET, VIRTUAL))) {
    log_msg("screwup in possession.");
    send_to_char("This spell is experiencing technical difficulty.\n\r",
		 ch);
    return;
  }

  act("$n summons a spirit from another plane to possess the corpse.",
      TRUE,ch,0,0,TO_ROOM);
  act("$N rises and lets out a furiously insane shriek.",
      FALSE,ch,0,mob,TO_ROOM);

  mob->points.mana=0;
  char_to_room(mob, ch->in_room);
  fix_mob_bits(mob);
  add_follower(mob, ch, 0);

  sprintf(buf,"%s is here, grinning evilly\n\r",OBJ_SHORT(corpse));
  ss_free(mob->player.long_descr);
  mob->player.long_descr = ss_make(buf);

  
  //align modifier
  if (GET_ALIGNMENT(ch) > 0)
     align_mod = -1*number(GET_ALIGNMENT(ch)/2, GET_ALIGNMENT(ch));
  else
     align_mod = number(GET_ALIGNMENT(ch)/-2, GET_ALIGNMENT(ch)*-1);
  
  mob->points.max_hit = MAX(1, dice((level+1),16) + align_mod);
  mob->points.hit = mob->points.max_hit/2; /* one half hp to start */
  mob->points.hitroll=mob->points.damroll=GetMaxLevel(ch)/5;
  mob->specials.damnodice=mob->specials.damsizedice=GetMaxLevel(ch)/10;
  mob->mult_att=MAX(1, GetMaxLevel(ch)/10);
  mob->player.level[WARRIOR_LEVEL_IND] = MIN ((ubyte)GetMaxLevel(ch), MAX_MORT);
  UpdateMaxLevel(mob);
  
  GET_RACE(mob) = RACE_UNDEAD;
  mob->player.sex = 0;
  SET_BIT(AFF_FLAGS(mob), AFF_CHARM);

  /* transfer items from the corpse to the possesser */
  IS_CARRYING_W(mob) = 0;
  IS_CARRYING_N(mob) = 0;
  for (obj_object=corpse->contains; obj_object; obj_object=next_obj) {
    next_obj = obj_object->next_content;
    obj_from_obj(obj_object);
    obj_to_char(obj_object, mob);
  }
  GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 20);
  /* get rid of corpse */
  extract_obj(corpse);
}

/*** victim is the type of elemental, obj is the stone used ***/
void spell_conjure_elemental(ubyte level, struct char_data *ch, int type,
                             struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim && obj);

  /*** stone:		elemental:
    red      	fire
    pale blue	water
    grey		earth
    clear		air	***/

  act("$n gestures, and $p dissolves into a large cloud of smoke!",
      TRUE,ch,obj,0,TO_ROOM);
  act("You gesture, and $p dissolves into a large cloud of smoke!",
      TRUE,ch,obj,0,TO_CHAR);

  obj_from_char(obj);
  extract_obj(obj);

  victim->points.mana=0;
  char_to_room(victim, ch->in_room);
  act("The cloud quickly dissipates revealing $n!",TRUE,victim,0,0,TO_ROOM);
  fix_mob_bits(victim);
  victim->player.level[WARRIOR_LEVEL_IND] = MIN ((ubyte) GetMaxLevel(ch)/2, MAX_MORT);
  UpdateMaxLevel(victim);
  victim->points.max_hit = victim->points.hit = GetMaxLevel(ch)*10;
  add_follower(victim, ch, 0);

  /* charm elemental for a while */
  MakeCharmed(ch, victim, level, SPELL_CONJURE_ELEMENTAL, -2);
}

void spell_golem(ubyte level, struct char_data *ch, int type,
		 struct char_data *victim, struct obj_data *obj)
{
  struct char_data *gol;
  struct obj_data *helm, *jacket, *leggings, *sleeves, *gloves, *boots, *o;
  struct room_data *rp;
  int armor, count=0;
  
  if (!wilderness(ch)) {
    send_to_char("You have to be in the wilderness to use mother nature's creation power!", ch);
    return;
  }

  helm=jacket=leggings=sleeves=gloves=boots=0;
  rp=real_roomp(ch->in_room);

  for (o=rp->contents; o; o=o->next_content) {
    if (ITEM_TYPE(o) == ITEM_ARMOR) {
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HEAD)) {
	if (!helm) {
	  count++;
	  helm = o;
	  continue;/* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET)) {
	if (!boots) {
	  count++;
	  boots = o;
	  continue;/* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_BODY)) {
	if (!jacket) {
	  count++;
	  jacket = o;
	  continue;/* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS)) {
	if (!leggings) {
	  count++;
	  leggings = o;
	  continue;/* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS)) {
	if (!sleeves) {
	  count++;
	  sleeves = o;
	  continue;/* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS)) {
	if (!gloves) {
	  count++;
	  gloves = o;
	  continue;/* next item */
	}
      }
    }
  }

  if (!boots || !sleeves || !gloves || !helm || !jacket || !leggings) {
    send_to_char("You don't have all the correct pieces of armor!\n\r", ch);
    return;
  }

  if(!(gol=make_mobile(VMOB_GOLEM, VIRTUAL))) {
    log_msg("screwup in golem.");
    send_to_char("This spell is experiencing technical difficulty.\n\r",ch);
    return;
  }
  gol->points.mana=0;
  char_to_room(gol, ch->in_room);

  /* add up the armor values in the pieces */
  armor = boots->obj_flags.value[0] + helm->obj_flags.value[0] +
          gloves->obj_flags.value[0] + (leggings->obj_flags.value[0]*2) +
          (sleeves->obj_flags.value[0]*2) + (jacket->obj_flags.value[0]*3);

  GET_AC(gol)-=armor;
  gol->points.max_hit=gol->points.hit=dice(armor/5, GetMaxLevel(ch));
  gol->points.hitroll=gol->points.damroll=armor/10;
  gol->specials.damnodice=gol->specials.damsizedice=GetMaxLevel(ch)/10;
  gol->mult_att=MAX(1, GetMaxLevel(ch)/15);
  gol->player.level[WARRIOR_LEVEL_IND]=MIN((ubyte)GetMaxLevel(ch), MAX_MORT);
  UpdateMaxLevel(gol);
  
  act("$n waves $s hands over a pile of armor on the ground...",TRUE,ch,0,0,TO_ROOM);
  act("You wave your hands over a pile of armor...", TRUE, ch, 0, 0, TO_CHAR);
  act("The armor flys together to form a humanoid figure!",TRUE,ch,0,0,TO_ROOM);
  act("$N is quickly assembled from the pieces of armor!",TRUE,ch,0,gol,TO_CHAR);

  AddAffects(gol,boots);
  AddAffects(gol,gloves);
  AddAffects(gol,jacket);
  AddAffects(gol,sleeves);
  AddAffects(gol,leggings);
  AddAffects(gol,helm);
  extract_obj(helm);
  extract_obj(boots);
  extract_obj(gloves);
  extract_obj(leggings);
  extract_obj(sleeves);
  extract_obj(jacket);

  fix_mob_bits(gol);
  SET_BIT(AFF_FLAGS(gol), AFF_CHARM);
  add_follower(gol, ch, 0);
}

void spell_charm(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);

  send_to_char("The gods have damned the art of charming.\n\r",ch);
  send_to_char("You have lost the power to control people!\n\r",ch);
  return;
/*

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (IS_PC(ch) && (IS_PC(victim))) {
      send_to_char("YOU CANT CHARM PLAYERS!\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("They are already CHARMED!\n\r", ch);
    return;
  }

  if (GET_POS(victim)<=POSITION_SLEEPING) {
    send_to_char("They are not paying any attention to you.\n\r", ch);
    return;
  }

  if (circle_follow(victim, ch)) {
    send_to_char("Sorry, following in circles cannot be allowed.\n\r", ch);
    return;
  }

  if(!CountFollowers(ch))
    return;
*/
  /* new stuff for align checks on charm */

//  if (IS_GOOD(ch) && IS_EVIL(victim)) { /* good charming bad */
  /*  act("$N tells you, 'I shall not follow you, your motives are inferior!!'",
	FALSE, ch, 0, victim, TO_CHAR);
    act("$n attempts to charm you, but does not follow your ways!",
	TRUE, ch, 0, victim, TO_VICT);
    act("$n fails to charm $N!", TRUE, ch, 0, victim, TO_ROOM);
*/
   // if ((rand() % 4) == 2) { /* 25 % chance of aggression */
  /*    act("$CMUH OH!.", FALSE, ch, 0, victim, TO_CHAR);
      act("$N shouts, 'EVIL SHALL RULE THE LAND!!!!! BANZAI!!! SPOON!!!'",
	  FALSE, ch, 0, victim, TO_CHAR);
      act("You decide to fight for your cause and attack $n!",
	  TRUE, ch, 0, victim, TO_VICT);
      act("$N shouts, 'EVIL SHALL RULE THE LAND!!!!! BANZAI!!!! SPOON!!!!'",
	  TRUE, ch, 0, victim, TO_ROOM);
      AddHated(victim, ch);
      if (!victim->specials.fighting)
	SetVictFighting(ch, victim);
    }
    return;
  }
*/
 // if (IS_EVIL(ch) && IS_GOOD(victim)) { /* bad charming good */
   /*  act("$N tells you, 'I shall not follow thee, thou art evil in they ways!!'",
	FALSE, ch, 0, victim, TO_CHAR);
    act("$n attempts to charm you, but does not follow your ways!",
	TRUE, ch, 0, victim, TO_VICT);
    act("$n fails to charm $N!", TRUE, ch, 0, victim, TO_ROOM);
*/
  //  if ((rand() % 4) == 2) { /* 25 % chance of aggression */
   /*   act("$CMUH OH!.", FALSE, ch, 0, victim, TO_CHAR);
      act("$N shouts, 'EVIL MUST BE VANQUISHED FROM THE LAND!!!'",
	  FALSE, ch, 0, victim, TO_CHAR);
      act("You decide to fight for your cause and attack $n!",
	  TRUE, ch, 0, victim, TO_VICT);
      act("$N shouts, 'EVIL MUST BE VANQUISHED FROM THE LAND!!!''",
	  TRUE, ch, 0, victim, TO_ROOM);
      AddHated(victim, ch);
      if (!victim->specials.fighting)
	SetVictFighting(ch, victim);
    }
    return;
  }

	if (GetMaxLevel(victim) > 35) 
	  {
		  send_to_char("Uh oh - I think they're too experienced for that.\n\r", ch);
		  AddHated(victim,ch);
		  if (!victim->specials.fighting)
		      SetVictFighting(ch, victim);
		  return;
	  }
	
	
	if (GetMaxLevel(victim) > GetMaxLevel(ch))
     {
	leveldiff = GetMaxLevel(ch) / 5;
	if ((GetMaxLevel(victim) - GetMaxLevel(ch)) > leveldiff)
	  {
	     send_to_char("They just give you a disinterest look.\n\r", ch);
	     return;
	  }
     }
   
    if((percent() < GetMaxLevel(victim)) || (NewSkillSave(ch, victim, SPELL_CHARM, +7, IMM_CHARM)))  { 
    if (number(1,100)<=GetMaxLevel(victim)+30) {
      act("You fail to charm the now angry $N!",FALSE,ch,0,victim,TO_CHAR);
      act("You are angered by $n's attempt to charm you!",TRUE,ch,0,victim,TO_VICT);
      act("$n fails to charm the now angry $N!",TRUE,ch,0,victim,TO_ROOM);
      AddHated(victim, ch);
      if (!victim->specials.fighting)
	SetVictFighting(ch, victim);
    } else {
      act("You fail to charm $N.",FALSE,ch,0,victim,TO_CHAR);
      act("$n attempts to charm you.",TRUE,ch,0,victim,TO_VICT);
      act("$n fails to charm $N.",TRUE,ch,0,victim,TO_ROOM);
    }
    return;
  }

  act("You are overwhelmed by $n's personality.",FALSE,ch,0,victim,TO_VICT);
  act("$n is just charmed to pieces.", FALSE, victim, 0, 0, TO_ROOM);

  if (!IS_PC(victim))
    fix_mob_bits(victim);
  if (victim->master)
    stop_follower(victim);
  add_follower(victim, ch, 0);

  MakeCharmed(ch, victim, level, SPELL_CHARM, 0);
*/}

void spell_friends(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);

  if (!is_hold_instrument(ch,SPELL_FRIENDS)) {
    return;
  }

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (IS_PC(ch) && (IS_PC(victim))) {
      send_to_char("YOU CANT CHARM PLAYERS!\n\r",ch);
      return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("They are already CHARMED!\n\r", ch);
    return;
  }

  if (GET_POS(victim)<=POSITION_SLEEPING) {
    send_to_char("They are not paying any attention to you.\n\r", ch);
    return;
  }

  if (circle_follow(victim, ch)) {
    send_to_char("Sorry, following in circles cannot be allowed.\n\r", ch);
    return;
  }

#ifdef TEMPUSWORK
  if(!CountFollowers(ch))
    return;
#endif
  if ((GetMaxLevel(victim) > GetMaxLevel(ch)) || 
     (!SkillChance(ch, victim, 15, IMM_CHARM, SPLMOD_INT | SPLMOD_CHA, SPELL_FRIENDS))) {
      if (percent() < 33) {
        act("$N thinks your a PHONY and attacks!",FALSE,ch,0,victim,TO_CHAR);
        act("You see through $n's attempt to be your friend!",TRUE,ch,0,victim,TO_VICT);
        act("$N doesn't like $n anymore.. as a matter of fact...!",TRUE,ch,0,victim,TO_ROOM);
        AddHated(victim, ch);
        if (!victim->specials.fighting)
          SetVictFighting(ch, victim);
      }
      else {
        act("$N just gives you a funny look.",FALSE,ch,0,victim,TO_CHAR);
        act("$n tries to be friends with you.",TRUE,ch,0,victim,TO_VICT);
        act("$n fails to make friends with $N.",TRUE,ch,0,victim,TO_ROOM);
      }
      return;
  }

  act("You are overwhelmed by $n's personality.",FALSE,ch,0,victim,TO_VICT);
  act("$n is just charmed to pieces.", FALSE, victim, 0, 0, TO_ROOM);

  if (!IS_PC(victim))
    fix_mob_bits(victim);
  if (victim->master)
    stop_follower(victim);
  add_follower(victim, ch, 0);

  MakeCharmed(ch, victim, level, SPELL_CHARM, 0);
}


/**************************/
/*** electricity spells ***/
/**************************/

void spell_shocking_grasp(ubyte level, struct char_data *ch, int type,
                          struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/10), 1);
  num = MIN(num, 6);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND) num=1;
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, SAVING_SPELL, shockSpellMod)) {
       if (IS_PURE_CLASS(ch))
		   dam=dice(level, 5);
       else
		dam=dice(level, 3);

	   /* Check to see if you are underwater */
     if (!UNDERWATER(ch)){
       if (damage(ch, victim, dam, SPELL_SHOCKING_GRASP))
          i=num;
     }     
     else
       area_affect (level, (int)dam/3, ch, SPELL_SHOCKING_GRASP, IMM_ELEC,
	   "Electrical current flows around you!",
	   "You feel a slight tingling around you!");
	}
	else {
		act("You try and $CgShock$CN $N with your grasp, but you fail!",FALSE,ch,0,victim,TO_CHAR);
		act("$n tried to $CgShock$CN $N but failed!",FALSE,ch,0,victim,TO_ROOM);
		act("$n tried to $CgShock$CN you, but failed.",FALSE,ch,0,victim,TO_VICT);
	}
  }
}

void spell_electrocute(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/15), 1);
  num = MIN(num, 5); 
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND) num=1;
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, SAVING_SPELL, elecSpellMod)) {
      if(IS_PURE_CLASS(ch))
		  dam=dice(level, 8);
	  else
		 dam=dice(level, 6);
     /* Check to see if you are underwater */
     if (!UNDERWATER(ch))
     {
       if(damage(ch, victim, dam, SPELL_ELECTROCUTE))
         i=num;
     }   
     else
       area_affect (level, (int)dam/3, ch, SPELL_ELECTROCUTE, IMM_ELEC,
		 "Electrical current flows around you!",
		 "You feel a slight tingling of energy!");
       
     add_charge_spell(ch, victim, SPELL_ELECTROCUTE);
   }
   else {
	act("You send out a bolt of $CgElectricity$CN, but it misses $N",FALSE,ch,0,victim,TO_CHAR);
	act("$n sends out a bolt of $CgElectricity$CN toward $N, but misses!",FALSE,ch,0,victim,TO_ROOM);
	act("A bolt of $CgElectricity$CN goes flying by!",FALSE,ch,0,victim,TO_VICT);
   }
  }

}

void spell_electric_fire(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/20), 1);
  num = MIN(num, 4);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND) num=1;
/* Check to see if you are underwater */
  if (!UNDERWATER(ch))
  {
   for(i=0; i < num; i++)
   {
    if(!ImpSaveSpell(victim, SAVING_SPELL, elecFireSpellMod))
    {

     if(IS_PURE_CLASS(ch))
        dam=dice(level,11);
     else
        dam=dice(level, 9);
       if(damage(ch, victim, dam, SPELL_ELECTRIC_FIRE)) i=num;
    }
    else {
      act("You send a bolt of $CyElectric Fire$CN right past $N!",FALSE,ch,0,victim,TO_CHAR);
      act("$n misses $N with $s bolt of $CyElectric Fire$CN.",FALSE,ch,0,victim,TO_ROOM);
      act("$n missed you with $s bolt of $CyElectric Fire$CN.",FALSE,ch,0,victim,TO_VICT);
    }
   }
  }
  else {

  if(IS_PURE_CLASS(ch))
   dam=dice(level,11);
  else
   dam=dice(level, 9);
    area_affect (level, (int)dam/3, ch, SPELL_ELECTRIC_FIRE, IMM_ELEC,
		 "Electricity surges through the water!",
		 "You wonder why your hair is standing up!");
   }
}

void spell_chain_electrocution(ubyte level, struct char_data *ch, int type,
                               struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
	  dam=dice(level, 13);
  else
	  dam=dice(level, 11);

  /* Check to see if you are underwater */
  if (!UNDERWATER(ch))
    area_affect (level, dam, ch, SPELL_CHAIN_ELECTROCUTION, IMM_ELEC,
         "Blinding bursts of electricity fly wildly everywhere!",
         "You hear the loud crackle of intense electricity nearby!");
  else
    area_affect (level, dam, ch, SPELL_CHAIN_ELECTROCUTION, IMM_ELEC,
	 "Electrical Energy surges through the water around you!",
	 "You feel a slight tingling of electricity in the water around you!");
}

/*******************/
/*** fire spells ***/
/*******************/

void spell_burning_hands(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));


  num = MAX((int)(GetMaxLevel(ch)/15), 1);
  num = MIN(num, 5);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND) num=1;
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, SAVING_SPELL, burningHandsSpellMod)) {
      if(IS_PURE_CLASS(ch))
	   dam=dice(level, 8);
      else 
	   dam=dice(level, 6);
  /* Check to see if you are underwater */
  if (UNDERWATER(ch))
    {
      send_to_char("You notice the water seems to be cooling your hands.\n\r", ch);
      act("You notice the water is keeping $n's hands from burning as bright.",TRUE,ch,0,0,TO_ROOM);
    }
    if(damage(ch, victim, dam, SPELL_BURNING_HANDS)) i=num;
	}
	else {
		act("Your $CrBurning Hands$CN can't get a hold of $N",FALSE,ch,0,victim,TO_CHAR);
		act("$n reaches out with $CrBurning HAnds$CN, but misses $N",FALSE,ch, 0, victim, TO_ROOM);
		act("$n tried to $CrBurn$CN you, but missed.",FALSE,ch,0,victim,TO_VICT);
	}
  }
}

void spell_fire_wind(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int savingType = SAVING_SPELL;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));


  num = MAX((int)(GetMaxLevel(ch)/20), 1);
  num = MIN(num, 4);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND)
  {
      num=1;
      savingType = SAVING_ROD;
  }
  for(i=0; i < num; i++)
  {
    
    if(!ImpSaveSpell(victim, savingType, fireWindSpellMod)) {
      if(IS_PURE_CLASS(ch))
		  dam=dice(level, 11);
	  else
		  dam=dice(level, 9);
  /* Check to see if you are underwater */
  if (UNDERWATER(ch))
    {
      send_to_char("You notice the wind's flames do not seem as high.\n\r", ch);
      act("You notice the water attacks the fire wind's strength.",TRUE,ch,0,0,TO_ROOM);
    }
     if( damage(ch, victim, dam, SPELL_FIRE_WIND)) i = num;
	}
	else {
		act("Your $CrFire Wind$CN blows right past $N", FALSE, ch,0,victim,TO_CHAR);
		act("$n's $CrWind of Fire$CN blows past $N",FALSE,ch,0,victim, TO_ROOM);
		act("A blast of $Crhot air$CN just misses you.",FALSE,ch,0,victim,TO_VICT);
	}
   }
}

void spell_flamestrike(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int savingType = SAVING_SPELL;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/25), 1);
  num = MIN(num, 4);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND){
	  num=1;
	  savingType = SAVING_ROD;
   }
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, savingType, flamestrikeSpellMod)) {
      if(IS_PURE_CLASS(ch))
        dam=dice(level, 14);
      else
        dam=dice(level, 12);
   /* Check to see if you are underwater */
   if (UNDERWATER(ch))
     {
       send_to_char("You notice the flames are being reduced by the water.\n\r", ch);
       act("The water seems to be sapping the flames' strength.",TRUE,ch,0,0,TO_ROOM);
     }
     if(damage(ch, victim, dam, SPELL_FLAMESTRIKE)) i=num;
       
     add_charge_spell(ch, victim, SPELL_FLAMESTRIKE);
   }
   else {
     act("Your $Crflames$CN go flying by $N, missing completely.", FALSE,ch,0,victim,TO_CHAR);
     act("$n tried to $Crflame$CN $N, but missed.",FALSE,ch,0,victim,TO_ROOM);
     act("$n tried to $Crburn$CN you with their $Crflamestrike$CN.", FALSE,ch,0,victim,TO_VICT);
   }
  }
}

void spell_fireball(ubyte level, struct char_data *ch, int type,
                    struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int i, num;
  int saveType = SAVING_SPELL;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/30), 1);
  num = MIN(num, 3);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND)
  {
	 saveType = SAVING_ROD;
	 num=1;
  }
  for(i=0; i < num; i++)
  {
   if(!ImpSaveSpell(victim, saveType, fireballSpellMod))
   {
     if(IS_PURE_CLASS(ch))
        dam=dice(level, 18);
     else
        dam=dice(level, 15);

     /* Check to see if you are underwater */
     if (UNDERWATER(ch))
       {
	 send_to_char("You notice your fireball does not seem as powerful as usual.\n\r", ch);
	 act("The water seems to be sapping the fireballs' strength with each foot it moves.", TRUE, 
	     ch, 0, 0, TO_ROOM);
       }

     if(damage(ch, victim, dam, SPELL_FIREBALL)) i=num;
     add_charge_spell(ch, victim, SPELL_FIREBALL);
   }
   else {
     act("You see your $Crfireball$CN fly by your intended target!\r\n",TRUE,ch,0,victim,TO_CHAR);
     act("$n sends a $Crfireball$CN through the room, missing $N by a mile.",TRUE,ch,0,victim,TO_ROOM);
     act("$n's $Crfireball$CN cinges your hair as it flies by, just missing you.",TRUE,ch,0,victim,
	 TO_VICT);
   }
  }  
}

void spell_lava_storm(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
/*  assert((level >= 1) && (level <= ABS_MAX_LVL));*/
  
  if(IS_PURE_CLASS(ch))
     dam=dice(level, 18);
  else
     dam=dice(level, 15);

  /* Check to see if you are underwater */
  if (!UNDERWATER(ch))
    area_affect (level, dam, ch, SPELL_LAVA_STORM, IMM_FIRE,
		 "The skies rain molten rock!",
        "You feel a blast of hot air as the temperature rises dramatically!");
  else
    area_affect (level, dam, ch, SPELL_LAVA_STORM, IMM_FIRE,
		 "The water hisses as lava drops from above!",
		 "You feel the water temperature rise dramatically!");
}

/*******************/
/*** cold spells ***/
/*******************/

void spell_chill_touch(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int saveType = SAVING_SPELL;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));


  num = MAX((int)(GetMaxLevel(ch)/10), 1);
  num = MIN(num, 6);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND)
  {
	 saveType = SAVING_ROD;
	 num=1;
  }
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, saveType, chillTouchSpellMod)) {
       if(IS_PURE_CLASS(ch))
         dam = dice(level, 5);
       else
         dam = dice(level, 3);
    if(!damage(ch, victim, dam, SPELL_CHILL_TOUCH))
       cold_effect (level, victim);
    else i=num;
	}
	else {
		act("You try and $Cbfreeze$CN $N with your touch, but miss!",FALSE,ch,0,victim,TO_CHAR);
		act("$n tried to $Cbfreeze$CN $N with a touch, but missed!",FALSE,ch,0,victim,TO_ROOM);
		act("$n tried to $Cbfreeze$CN you with a touch, but missed!",FALSE,ch,0,victim,TO_VICT);
    }
  }

}

void spell_frost_cloud(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int saveType = SAVING_SPELL;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/15), 1);
  num = MIN(num, 5);

  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND)
  {
	 saveType = SAVING_ROD;
	 num=1;
  }
  for(i=0; i < num; i++)
  {
    if(!ImpSaveSpell(victim, saveType, frostCloudSpellMod)) {
      if(IS_PURE_CLASS(ch)) 
       dam = dice(level, 8);
      else
       dam = dice(level, 6);
    if(!damage(ch, victim, dam, SPELL_FROST_CLOUD))
       cold_effect (level, victim);
    else i=num;
	}
	else {
		act("You send out a $CbFrost Cloud$CN toward $N, but you miss!",FALSE,ch,0,victim,TO_CHAR);
		act("$n sends out a $CbFrost Cloud$CN toward $N, but $N gets away.",FALSE,ch,0,victim,TO_ROOM);
		act("$n tried to engulf you with a $CbFrost Cloud$CN, but you escape!",FALSE,ch,0,victim,TO_VICT);
	}
  }
   
  add_charge_spell(ch, victim, SPELL_FROST_CLOUD);
}

void spell_ice_storm(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  
  if(IS_PURE_CLASS(ch))
    dam=dice(level, 10);
  else
    dam=dice(level, 8);
 
  area_affect (level, dam, ch, SPELL_ICE_STORM, IMM_COLD,
    "The air freezes as the surroundings are blasted with ice!",
    "You feel a blast of cold air as the temperature drops dramatically!");
}

/*********************/
/*** energy spells ***/
/*********************/

void spell_harmful_touch(ubyte level, struct char_data *ch, int type,
                         struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
   dam=dice(level, 5);
  else
   dam=dice(level, 3);

  if (ImpSaveSpell(victim, SAVING_SPELL, harmfulTouchSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_HARMFUL_TOUCH);
}

void spell_decay(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
    dam=dice(level, 7);
  else
    dam=dice(level, 5);

  if (ImpSaveSpell(victim, SAVING_SPELL,decaySpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_WITHER);
}

void spell_rupture(ubyte level, struct char_data *ch, int type,
                   struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
    dam=dice(level, 9);
  else
    dam=dice(level, 7);

  if (ImpSaveSpell(victim, SAVING_SPELL,ruptureSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_RUPTURE);
}

void spell_implode(ubyte level, struct char_data *ch, int type,
                   struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
    dam=dice(level, 11);
  else
    dam=dice(level, 9);

  if (ImpSaveSpell(victim, SAVING_SPELL,implodeSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_IMPLODE);
}

void spell_disintegrate(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
    dam=dice(level, 20);
  else
    dam=dice(level, 16);

  if (ImpSaveSpell(victim, SAVING_SPELL,disintegrateSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_DISINTEGRATE);
}

/*** acid spells ***/

void spell_acid_blast(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int saveType = SAVING_SPELL;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/25), 1);
  num = MIN(num, 3);

  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND){
	 saveType = SAVING_ROD;
	 num=1;
  }
  for(i=0; i < num; i++)
  {
     if(!ImpSaveSpell(victim, saveType, acidBlastSpellMod)) {
        if(IS_PURE_CLASS(ch))
           dam = dice(level, 14);
        else
           dam = dice(level, 12);
     if( damage(ch, victim, dam, SPELL_ACID_BLAST))
        i=num;
     }
     else {
        act("A wave of $CrAcid$CN goes flying by $N",FALSE,ch,0,victim,TO_CHAR);
        act("$n just misses $N with a wave of $CrAcid$CN!",FALSE,ch,0,victim,TO_ROOM);
        act("$n sends a wave of $CrAcid$CN at you, but misses!",FALSE,ch,0,victim,TO_VICT);
     }
  }
  add_charge_spell(ch, victim, SPELL_ACID_BLAST);
}

void spell_acid_rain(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(IS_PURE_CLASS(ch))
    dam=dice(level, 13);
  else
    dam=dice(level, 11);

  area_affect (level, dam, ch, SPELL_ACID_RAIN, IMM_ACID,
    "The surroundings are blackened and scarred by a downpour of acid!",
    "You are nearly overwhelmed by the pungent stench of acid nearby!");
}

/*** drain spells ***/

void spell_energy_drain(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  int dam, num, i;
  int saveType = SAVING_SPELL;

  assert(victim && ch);
  assert((level >= 1) && (level <=  ABS_MAX_LVL));

  num = MAX((int)(GetMaxLevel(ch)/20), 1);
  num = MIN(num, 4);

  if(GetMaxLevel(ch) == 125)
     num++;

  if(type==SPELL_TYPE_WAND) {
	 saveType = SAVING_ROD;
	 num=1;
  }
  for(i=0; i < num; i++)
  {
     if(!ImpSaveSpell(victim, saveType, energyDrainSpellMod)) {
        if(IS_PURE_CLASS(ch))
           dam = dice(level, 10);
        else
           dam = dice(level, 8);
        if( damage(ch, victim, dam, SPELL_ENERGY_DRAIN))
           i=num;
     }
     else {
        act("You tried to $CCdrain$CN $N's $Crenergy$CN, but you couldn't!",FALSE,ch,0,victim,TO_CHAR);
        act("$n tried to $CCdrain$CN $N, but failed!",FALSE,ch,0,victim,TO_ROOM);
        act("$n tried to $CCdrain$CN you.",FALSE,ch,0,victim,TO_VICT);
     }
  }
  add_charge_spell(ch, victim, SPELL_ENERGY_DRAIN);
}

/*** drains hp from the victim and adds them to the caster ***/
void spell_vampyric_touch(ubyte level, struct char_data *ch, int type,
                          struct char_data *victim, struct obj_data *obj)
{
  int dam, hp_before, hp_after, hp_gain, num, i;
  int saveType = SAVING_SPELL;
  
  assert(victim && ch);
  assert((level >= 1) && (level <=  ABS_MAX_LVL));

  if (!victim->specials.fighting)
    set_fighting(victim, ch);


 num = MAX((int)(GetMaxLevel(ch)/40), 1);
 num = MIN(num, 3);
  if(GetMaxLevel(ch) == 125) num++;

  if(type==SPELL_TYPE_WAND){
	  num=1;
	  saveType = SAVING_ROD;
  }
  for(i=0; i < num; i++)
  {
      if(IS_PURE_CLASS(ch))
       dam = dice(level,14);
      else
       dam=dice(level,10); 
      if (ImpSaveSpell(victim, saveType, vampyricSpellMod))
           dam /= 2;

        hp_before=GET_HIT(victim);
        if(!damage(ch, victim, dam, SPELL_VAMPYRIC_TOUCH))
          hp_after=GET_HIT(victim);
        else {
          hp_after = 0;
          i=num;
        }

        hp_gain=(hp_before-hp_after)/2;
        if (hp_gain <= 0)
          return;

        GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)-20);
        GET_HIT(ch) += hp_gain;
        if(GET_HIT(ch) > (hp_after = hit_limit(ch)))
           GET_HIT(ch) = hp_after;
  }
  //    else {
  //      act("You reach out your hand to $Cydrain$CN $N of energy, but you miss!",FALSE,ch,0,victim,TO_CHAR);
  //      act("$n tried to $Cydrain$CN $N's energy, but failed!",FALSE,ch,0,victim,TO_ROOM);
//	  act("$n tried to $Cydrain$CN your energy with a touch, but failed!",FALSE,ch,0,victim,TO_VICT);
 //     }

}

/*** poison spells ***/

void spell_poison(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  assert(victim || obj);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (obj) { /* object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      send_to_char("You poison it!\n\r",ch);
      obj->obj_flags.value[3] = 1;
    } else {
      send_to_char("You cannot poison that.\n\r",ch);
    }
    return;
  }

  if (!victim->specials.fighting)
    set_fighting(victim, ch);
  if (!ImpSaveSpell(victim, SAVING_PARA, poisonSpellMod))
     poison_effect (level, ch, victim);
}

/*** poison gas is an area affect poison spell ***/
void spell_poison_gas(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));
/*
    act("A billowing green cloud of poison gas envelops the area!",
	FALSE, ch, 0, 0, TO_CHAR);
    act("A billowing green cloud of poison gas envelops the area!",
	TRUE, ch, 0, 0, TO_ROOM);
*/

    area_affect(level,0,ch,SPELL_POISON_GAS,IMM_POISON,
                "A billowing green cloud of poison gas envelops the area!",
                "You feel nauseated as you catch the scent of poison gas!");
                
}

/*********************/
/*** nature spells ***/
/*********************/

void spell_wind_storm(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
    int dam;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (!OUTSIDE(ch)) {
	send_to_char("You must be outdoors to create a wind storm.\n\r",ch);
	return;
    }

    dam=dice(level, 3);

    area_affect (level, dam, ch, SPELL_WIND_STORM, 0,
        "A windstorm quickly builds and unleashes its fury!",
        "You feel a sudden blast of wind!");
}

void spell_geyser(ubyte level, struct char_data *ch, int type,
                  struct char_data *victim, struct obj_data *obj)
{
  int dam, num;
  
  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  
  num = MAX((int)(GetMaxLevel(ch)/60), 1);
  num = MIN(num, 2);
  
  if(GetMaxLevel(ch) == 125) num++;
  if (!OUTSIDE(ch)) {
    send_to_char("You must be outdoors to create a geyser.\n\r",ch);
    return;
  }
 
  int i = 0;
  for (i = 0; i < num; i++) {
    dam=dice(level, 12);
    
    area_affect (level, dam, ch, SPELL_GEYSER, 0,
		 "A Geyser erupts from the ground in a huge column of searing steam!",
		 "You feel a blast of hot humid air!");
  }
}

void spell_earthquake(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
    int dam;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam=dice(level, 12);

    area_affect (level, dam, ch, SPELL_EARTHQUAKE, 0,
		 "Debris flies as the earth cracks and heaves beneath your feet!",
		 "You almost lose your balance as violent tremors rock you!");
}

void spell_call_lightning(ubyte level, struct char_data *ch, int type,
                          struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int i, num;
  int saveType = SAVING_SPELL;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  
  //  assert(ch && victim);
  
  num = MAX((int)(GetMaxLevel(ch)/30), 1);
  num = MIN(num, 3);

  if(GetMaxLevel(ch) == 125) num++;
  
  if (!OUTSIDE(ch)) {
    send_to_char("You must be outdoors to call on lightning.\n\r",ch);
    return;
  }
  
  if (weather_info.sky<SKY_RAINING) {
    send_to_char("The weather is too good to allow lightning strikes.\n\r", ch);
    return;
  }
  
  if(type==SPELL_TYPE_WAND)
    {
      saveType = SAVING_ROD;
      num=1;
    }

  for(i=0; i < num; i++)
    {
      if(!ImpSaveSpell( victim, saveType, callLightningSpellMod))
	{
	  dam=dice(level, 22);
	  
	  /* Check to see if you are underwater */
	  if (!UNDERWATER(ch))
	    {
	      if (damage(ch, victim, dam, SPELL_CALL_LIGHTNING)) i=num;
	    }
	  else {
	    area_affect (level, dam, ch, SPELL_CALL_LIGHTNING, IMM_ELEC,
			 "Electrical Energy surges through the water around you!",
			 "You feel a slight tingling of electricity in the water around you!");
	  }
	}
      
      add_charge_spell(ch, victim, SPELL_CALL_LIGHTNING);
    }
}

void spell_thorn(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  assert(ch && victim);

  if (!wilderness(ch)) {
      send_to_char("You must be in the wilderness to call upon thorns.\n\r",ch);
      return;
  }

  dam = dice(level, 6);
  if (ImpSaveSpell(victim, SAVING_SPELL, thornSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_THORN);
}

void spell_vine(ubyte level, struct char_data *ch, int type,
                struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  assert(ch && victim);

  if (!wilderness(ch)) {
      send_to_char("You must be in the wilderness to call upon vines.\n\r",ch);
      return;
  }

  act("Vines erupt from the ground, grasping violently at $n!",TRUE,victim,0,0,TO_ROOM);
  send_to_char("Vines erupt from the ground, grasping at you!\n\r",victim);

  dam = dice(level, 9);
  if (ImpSaveSpell(victim, SAVING_SPELL, vineSpellMod))
    dam /=2;
  damage(ch, victim, dam, SPELL_VINE);
}

void spell_creep(ubyte level, struct char_data *ch, int type,
                 struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!wilderness(ch)) {
      send_to_char ("You must be in the wilderness to call upon your insect friends.\n\r",ch);
      return;
  }

  dam=dice(level, 22);

  if (ImpSaveSpell(victim, SAVING_BREATH, 0))
    dam /=2;
  area_affect (level, dam, ch, SPELL_CREEPING_DOOM, 0,
    "A huge swarm of voracious insects descend upon the unwary!",
    "You hear the buzzing of millions of insects nearby!");
}

void spell_control_weather(ubyte level, struct char_data *ch, int type,
                           struct char_data *victim, struct obj_data *obj)
{
  /* bummer! we're not set up to pass "better" or "worse" */
  /* to spell routines. the spell is defined in spells.c  */
}

void spell_camouflage(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (type == SPELL_TYPE_POTION) {
    level = level / 4;
  }

  if (!OUTSIDE(ch)) {
      send_to_char ("You must be outside to blend with the foliage.\n\r",ch);
      return;
  }
  
  if(MOUNTED(ch)) {
    send_to_char("You cannot camouflage yourself and your mount.\n\r", ch);
    return;
  }

  MakeAffect(ch, victim, type,
	     SPELL_CAMOUFLAGE, 0, APPLY_NONE, AFF_SNEAK,
	     level / 5, 1, FALSE, FALSE, FALSE, NULL);

  act("$n blends into the surrounding foliage and disappears!",TRUE,ch,0,0,TO_ROOM);
  send_to_char("You blend in with the surrounding foliage, moving undetected!\n\r", ch);
}

/********************/
/*** group spells ***/
/********************/

void spell_fly_group(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_flying(level, ch, type, tch, obj);
    }
  }
}

void spell_armor_group(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_armor (level, ch, type, tch, obj);
    }
  }
}

void spell_invis_group(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_invisibility (level, ch, type, tch, obj);
    }
  }
}

void spell_dinvis_group(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_detect_invisibility (level, ch, type, tch, obj);
    }
  }
}

void spell_true_seeing_group(ubyte level, struct char_data *ch, int type,
                             struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_true_seeing(level, ch, type, tch, obj);
    }
  }
}

void spell_heal_group(ubyte level, struct char_data *ch, int type,
                            struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_heal (level, ch, type, tch, obj);
    }
  }
}

void spell_cure_light_group(ubyte level, struct char_data *ch, int type,
                            struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;
  
  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_cure_light (level, ch, type, tch, obj);
    }
  }
}

void spell_waterbreath_group(ubyte level, struct char_data *ch, int type,
                             struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tch;

  assert(ch);

  if (!real_roomp(ch->in_room))
    return;

  for (tch=real_roomp(ch->in_room)->people; tch; tch=tch->next_in_room) {
    if (in_group(ch, tch)) {
      spell_water_breath (level, ch, type, tch, obj);
    }
  }
}

void spell_recall_group(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
    int location;
    struct char_data *tch, *next;
    struct room_data* rp;
    
    assert(ch);

    if (GET_HOME(ch))
	location = GET_HOME(ch);
    else
	location = 7;

    if((rp = real_roomp(location)) == NULL)
    {
	send_to_char("You are completely lost!\n\r", ch);
	return;
    }

    if((IS_SET(rp->room_flags, IMMORT_RM) && !IS_GOD(ch)) ||
       (IS_SET(rp->room_flags, GOD_RM) && TRUST(ch) < TRUST_LORD))
    {
	send_to_char("An unknown force keeps you where you are.\n\r", ch);
	return;
    }

    if(!(rp = real_roomp(ch->in_room)) || IS_SET(rp->room_flags, NO_RECALL))
      {
        send_to_char("An unknown force keeps you here.\n\r", ch);
        return;
      }
    

    for (tch=real_roomp(ch->in_room)->people; tch; tch=next)
    {
	next = tch->next_in_room;
	if (ch!=tch && in_group(ch, tch))
	{
	    if(tch->specials.fighting == NULL)
	    {
		act("$n disappears in a puff of smoke!",
		    TRUE, tch, 0, 0, TO_ROOM);
		char_from_room(tch);
		char_to_room(tch, location);
		act("$n suddenly appears in a puff of smoke!",
		    TRUE, tch, 0, 0, TO_ROOM);
		do_look(tch, "",15);
	    }
	}
    }

    if(ch->specials.fighting == NULL)
    {
	act("$n disappears in a puff of smoke!", TRUE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n suddenly appears in a puff of smoke!",
	    TRUE, ch, 0, 0, TO_ROOM);
	do_look(ch, "",15);
    }
}

/*********************/
/*** breath spells ***/
/*********************/

int breath_damage (ubyte level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
  int dam;

  dam=dice(level,level);

  if (IS_PC(ch))
    dam /= 2;

  return dam;
}

// Breath weapons, the potion edition
void spell_fire_breath_aff(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  assert (victim);  

  if (IS_AFFECTED2(ch, AFF2_FIRE_BREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  send_to_char("You are getting REALLY hot, sweat is trickling down your face and smoke follows when you breathe\n\r", victim);
  act("$n suddenly looks very hot, and not well at all. Smoke seems to be coming out of his mouth as he breathes.",TRUE,victim,0,0,TO_ROOM);
  
  MakeAffect(ch, ch, type,
	     SPELL_POTION_FIRE_BREATH, 0, APPLY_AFF2, AFF2_FIRE_BREATH,
	     level, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_frost_breath_aff(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  assert (victim);  

  if (IS_AFFECTED2(ch, AFF2_FROST_BREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  send_to_char("Your skin turns blue, your hair freezes and you feel really cold\n\r", victim);
  act("$n's skin seems to be turning blue. His teeth are chattering.",TRUE,victim,0,0,TO_ROOM);
  
  MakeAffect(ch, ch, type,
	     SPELL_POTION_FROST_BREATH, 0, APPLY_AFF2, AFF2_FROST_BREATH,
	     level, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_acid_breath_aff(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  assert (victim);  

  if (IS_AFFECTED2(ch, AFF2_ACID_BREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  send_to_char("You start drewling, you get a very bad taste in your mouth and you are coughing up green stuff, that potion was not good.\n\r", victim);
  act("$n starts drewling, and his saliva looks green - thats disgusting!.",TRUE,victim,0,0,TO_ROOM);
  
  MakeAffect(ch, ch, type,
	     SPELL_POTION_ACID_BREATH, 0, APPLY_AFF2, AFF2_ACID_BREATH,
	     level, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_poison_gas_breath_aff(ubyte level, struct char_data *ch, int type,
                             struct char_data *victim, struct obj_data *obj)
{
  assert (victim);  

  if (IS_AFFECTED2(ch, AFF2_POISONGAS_BREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }
  send_to_char("Phew, that stinks! You suddenly realize that the smell is your breath..\n\r", victim);
  act("$n obviously forgot to brush this morning, his breath is REALLY bad!.",TRUE,victim,0,0,TO_ROOM);
  
  MakeAffect(ch, ch, type,
	     SPELL_POTION_POISON_GAS_BREATH, 0, APPLY_AFF2, AFF2_POISONGAS_BREATH,
	     level, 10, FALSE, FALSE, FALSE, NULL);
}

void spell_lightning_breath_aff(ubyte level, struct char_data *ch, int type,
                            struct char_data *victim, struct obj_data *obj)
{
  assert (victim);  

  if (IS_AFFECTED2(ch, AFF2_LIGHTNING_BREATH))
    return;

  if (type == SPELL_TYPE_POTION) {
    level = level / 20;
  }

  send_to_char("OUCH! You feel like you just bit down hard on an electrical wire, that is painful!\n\r", victim);
  act("$n's teeth starts looking really weird, you look closer and realize that small lightning bolts are jumping between his teeth!",TRUE,victim,0,0,TO_ROOM);
  
  MakeAffect(ch, ch, type,
	     SPELL_POTION_LIGHTNING_BREATH, 0, APPLY_AFF2, AFF2_LIGHTNING_BREATH,
	     level, 10, FALSE, FALSE, FALSE, NULL);
}
// End breath weapon potions

void spell_fire_breath(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = breath_damage (level, ch, victim, obj);

  /* Check to see if you are underwater */
  if (!UNDERWATER(ch))
    area_affect (level, dam, ch, SPELL_FIRE_BREATH, IMM_FIRE,
         "$CRSearing $CYflames $CRscorch $CYeveryone $CRand $CYeverything!$CN",
	 "You feel a blast of hot air as the temperature rises dramatically!");
  else
    area_affect (level, dam, ch, SPELL_FIRE_BREATH, IMM_FIRE,
   "The water begins to boil from the heat scorching everyone and everything!",
		 "You feel the water temperature rise dramatically!");
}


void spell_frost_breath(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = breath_damage (level, ch, victim, obj);

  area_affect (level, dam, ch, SPELL_FROST_BREATH, IMM_COLD,
    "$CCNumbing $CBfrost $CCblasts $CBeveryone $CCand $CBeverything!$CN",
    "You feel a blast of cold air as the temperature drops dramatically!");
}

void spell_acid_breath(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = breath_damage (level, ch, victim, obj);

  area_affect (level, dam, ch, SPELL_ACID_BREATH, IMM_ACID,
    "$CGScorching $CCacid $CGburns $CCeveryone $CGand $CCeverything!$CN",
    "You are nearly overwhelmed by the pungent stench of acid nearby!");
}

void spell_poison_gas_breath(ubyte level, struct char_data *ch, int type,
                             struct char_data *victim, struct obj_data *obj)
{
  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  spell_poison_gas(level, ch, type, victim, obj);
}

void spell_lightning_breath(ubyte level, struct char_data *ch, int type,
                            struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = breath_damage (level, ch, victim, obj);

  /* Check to see if you are underwater */
  if (!UNDERWATER(ch))
  area_affect (level, dam, ch, SPELL_LIGHTNING_BREATH, IMM_ELEC,
         "$CWBlinding $CCbursts $CWof $CCelectricity $CWhit $CCeveryone $CWand $CCeverything!$CN",
         "You hear the loud crackle of intense electricity nearby!");
  else
    area_affect (level, dam, ch, SPELL_LIGHTNING_BREATH, IMM_ELEC,
	 "Electrical Energy surges through the water around you!",
	 "You feel a slight tingling of electricity in the water around you!");
}


/************/

void spell_dispel_magic(ubyte level, struct char_data *ch, int type,
                        struct char_data *victim, struct obj_data *obj)
{
    int remove[] = {
	SPELL_ARMOR, SPELL_BLESS, SPELL_BLINDNESS, SPELL_CURSE,
      SPELL_DETECT_INVISIBLE, SPELL_INVISIBLE, SPELL_POISON,
      SPELL_SANCTUARY, SPELL_STRENGTH, SPELL_SENSE_LIFE,
	SPELL_INFRAVISION, SPELL_REGEN, SPELL_WEAKNESS, 
      SPELL_SENSE_AURA, SPELL_WATER_BREATH, SPELL_FLY, 
      SPELL_SHIELD, SPELL_FIRESHIELD, SPELL_ELECSHIELD, 
      SPELL_ENERGYSHIELD, SPELL_ACIDSHIELD, SPELL_COLDSHIELD,
      SPELL_POISONSHIELD,
      SPELL_WARD_FIRE, SPELL_WARD_COLD, SPELL_WARD_ELEC,
      SPELL_WARD_ENERGY, SPELL_WARD_ACID, 
      SPELL_STONE_SKIN, SPELL_PETRIFY, SPELL_STONE_FIST,
	SPELL_TRUE_SIGHT, SPELL_FAERIE_FIRE, SPELL_WEB,
	SPELL_CAMOUFLAGE,  SPELL_SLEEP, SPELL_SLOW, SPELL_HASTE, 
	SPELL_DESPAIR, SPELL_SILENCE, SPELL_BLUR, SPELL_INSPIRE, 
      SPELL_SLOW, 0};
    struct affected_type *af, *naf;
    int i, flag, agg;
    
    assert(victim);

    agg = FALSE;

    flag = GetMaxLevel(victim)<=level;

    for (af=victim->affected; af; af=naf) {
	for(naf = af->next ; naf && naf->type == af->type ; naf = naf->next);
	
	if(level <= ABS_MAX_LVL)
	{
	    for(i = 0 ; remove[i] ; ++i)
		if (af->type==remove[i])
		    break;
	    if(!remove[i])
		continue;
	    if(af->type==SPELL_SANCTUARY || af->type==SPELL_FIRESHIELD)
		agg = TRUE;
	}
	
	if ((flag || !ImpSaveSpell(victim, SAVING_SPELL, dispellSpellMod)) &&
           !af->expire_proc_pointer) { 
	    act("Some of the magic around $n's body fades.",
		TRUE,victim,0,0,TO_ROOM);
	    if((af->type >= 0) &&
	       (af->type < MAX_SKILLS) &&
	       spell_wear_off_msg[af->type])
	    {
		send_to_char(spell_wear_off_msg[af->type], victim);
		send_to_char("\n\r", victim);
	    }
            affect_from_char(victim, af->type);
	}
    }

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
	if (flag || !ImpSaveSpell(victim, SAVING_SPELL, dispellSpellMod)) {
	    REMOVE_BIT(AFF_FLAGS(victim), AFF_SANCTUARY);
	    send_to_char("You don't feel so invulnerable anymore.\n\r",victim);
	    act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);
	}
	agg=TRUE;
    }
    if (IS_AFFECTED(victim, AFF_FIRESHIELD)) {
	if (flag || !ImpSaveSpell(victim, SAVING_SPELL,dispellSpellMod) ) {
	    REMOVE_BIT(AFF_FLAGS(victim), AFF_FIRESHIELD);
	    send_to_char("You don't feel so fiery anymore.\n\r",victim);
	    act("The red glow around $n's body fades.", FALSE, victim,
		0, 0, TO_ROOM);
	}
	agg=TRUE;
    }

    if (IS_AFFECTED2(victim, AFF2_HASTE)) {
	if (flag || !ImpSaveSpell(victim, SAVING_SPELL,dispellSpellMod) ) {
	    REMOVE_BIT(AFF2_FLAGS(victim), AFF2_HASTE);
	    send_to_char("You don't feel so fast anymore.\n\r",victim);
	    act("$n does not appear to be moving so fast.",FALSE, victim,
		0, 0, TO_ROOM);
	}
	agg=TRUE;
    }


  
    if (IS_AFFECTED2(victim, AFF2_ELECSHIELD)) {
       if(flag || !ImpSaveSpell(victim, SAVING_SPELL, dispellSpellMod) ) {
          REMOVE_BIT(AFF2_FLAGS(victim), AFF2_ELECSHIELD);
          send_to_char("You are not crackling anymore.\n\r",victim);
          act("$n does not appear to be crackling anymore.",FALSE,victim, 0,0,TO_ROOM);
       }
       agg=TRUE;
    }

    if (IS_AFFECTED2(victim, AFF2_POISONSHIELD)) {
       if(flag || !ImpSaveSpell(victim, SAVING_SPELL,dispellSpellMod) ) {
          REMOVE_BIT(AFF2_FLAGS(victim), AFF2_POISONSHIELD);
          send_to_char("You are not shielded in poison anymore.\n\r",victim);
          act("$n does not appear to be shielded with poison anymore.",FALSE,victim, 0,0,TO_ROOM);
       }
       agg=TRUE;
    }
    
    if (IS_AFFECTED2(victim, AFF2_ENERGYSHIELD)) {
       if(flag || !ImpSaveSpell(victim, SAVING_SPELL,dispellSpellMod) ) {
          REMOVE_BIT(AFF2_FLAGS(victim), AFF2_ENERGYSHIELD);
          send_to_char("You are not energized anymore.\n\r",victim);
          act("$n does not appear to be energized anymore.",FALSE,victim, 0,0,TO_ROOM);
       }
       agg=TRUE;
    }

    if (IS_AFFECTED2(victim, AFF2_COLDSHIELD)) {
       if(flag || !ImpSaveSpell(victim, SAVING_SPELL,dispellSpellMod) ) {
          REMOVE_BIT(AFF2_FLAGS(victim), AFF2_COLDSHIELD);
          send_to_char("You are not freezing anymore.\n\r",victim);
          act("$n does not appear to be freezing anymore.",FALSE,victim, 0,0,TO_ROOM);
       }
       agg=TRUE;
    }

    if (IS_AFFECTED2(victim, AFF2_ACIDSHIELD)) {
       if(flag || !ImpSaveSpell(victim, SAVING_SPELL, dispellSpellMod) ) {
          REMOVE_BIT(AFF2_FLAGS(victim), AFF2_ACIDSHIELD);
          send_to_char("You are not acidic anymore.\n\r",victim);
          act("$n does not appear to be acidic anymore.",FALSE,victim, 0,0,TO_ROOM);
       }
       agg=TRUE;
    }

    if (agg && !IS_PC(victim) && !victim->specials.fighting)
	set_fighting(victim, ch);
}

/***************************/
/*** wards of protection ***/
/***************************/

void spell_ward_fire(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int leech;
  long apply;

  assert(victim);
  

  if (IsImmune(victim, IMM_FIRE))
    return;
  apply=APPLY_IMMUNE; leech=10;
  if (affected_by_spell(victim, SPELL_WARD_FIRE)) {
   if((ch==victim) && IS_PURE_CLASS(ch)){
    affect_from_char(victim, SPELL_WARD_FIRE);
    apply=APPLY_M_IMMUNE; leech=20;
   }
   else {
    affect_from_char(victim, SPELL_WARD_FIRE);
    apply=APPLY_M_IMMUNE; leech=40;
   }
  }

  MakeAffect(ch, victim, type,
             SPELL_WARD_FIRE, IMM_FIRE, apply, 0,
             3, leech, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are protected by a fire ward.\n\r",victim);
  act("$n is protected by a fire ward.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_ward_cold(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int leech;
  long apply;

  assert(victim);
  

  if (IsImmune(victim, IMM_COLD))
    return;
  apply=APPLY_IMMUNE; leech=10;
  if (affected_by_spell(victim, SPELL_WARD_COLD)) {
    if((ch==victim) && IS_PURE_CLASS(ch)){
      affect_from_char(victim, SPELL_WARD_COLD);
      apply=APPLY_M_IMMUNE; leech=20;
    }else {
      affect_from_char(victim, SPELL_WARD_COLD);
      apply=APPLY_M_IMMUNE; leech=40;
    }
 
  }

  MakeAffect(ch, victim, type,
             SPELL_WARD_COLD, IMM_COLD, apply, 0,
             3, leech, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are protected by a cold ward.\n\r",victim);
  act("$n is protected by a cold ward.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_ward_elec(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int apply, leech;

  assert(victim);
  

  if (IsImmune(victim, IMM_ELEC))
    return;
  apply=APPLY_IMMUNE; leech=10;
  if (affected_by_spell(victim, SPELL_WARD_ELEC)) {
    if((ch==victim) && IS_PURE_CLASS(ch)){
      affect_from_char(victim, SPELL_WARD_ELEC);
      apply=APPLY_M_IMMUNE; leech=20;
    } else {
       affect_from_char(victim, SPELL_WARD_ELEC);
       apply=APPLY_M_IMMUNE; leech=40;
    }
  }

  MakeAffect(ch, victim, type,
             SPELL_WARD_ELEC, IMM_ELEC, apply, 0,
             3, leech, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are protected by an electric ward.\n\r",victim);
  act("$n is protected by an electric ward.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_ward_energy(ubyte level, struct char_data *ch, int type,
                       struct char_data *victim, struct obj_data *obj)
{
  int leech;
  long apply;

  assert(victim);
  

  if (IsImmune(victim, IMM_ENERGY))
    return;
  apply=APPLY_IMMUNE; leech=10;
  if (affected_by_spell(victim, SPELL_WARD_ENERGY)) {
   if((ch==victim) && IS_PURE_CLASS(ch)){
    affect_from_char(victim, SPELL_WARD_ENERGY);
    apply=APPLY_M_IMMUNE; leech=20;
   } else {
     affect_from_char(victim, SPELL_WARD_ENERGY);
     apply=APPLY_M_IMMUNE; leech=40;
   }

  }

  MakeAffect(ch, victim, type,
             SPELL_WARD_ENERGY, IMM_ENERGY, apply, 0,
             3, leech, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are protected by an energy ward.\n\r",victim);
  act("$n is protected by an energy ward.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_ward_acid(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  int leech;
  long apply;

  assert(victim);
  

  if (IsImmune(victim, IMM_ACID))
    return;
  apply=APPLY_IMMUNE; leech=10;
  if (affected_by_spell(victim, SPELL_WARD_ACID)) {
    if(ch==victim && IS_PURE_CLASS(ch)) {
      affect_from_char(victim, SPELL_WARD_ACID);
      apply=APPLY_M_IMMUNE; leech=20;
    } else {
      affect_from_char(victim, SPELL_WARD_ACID);
      apply=APPLY_M_IMMUNE; leech=20;
    }
  }

  MakeAffect(ch, victim, type,
             SPELL_WARD_ACID, IMM_ACID, apply, 0,
             3, leech, FALSE, FALSE, FALSE, NULL);

  send_to_char("You are protected by an acid ward.\n\r",victim);
  act("$n is protected by an acid ward.", TRUE, victim, 0, 0, TO_ROOM);
}

void spell_stone_fist(ubyte level, struct char_data *ch, int type,
                      struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
 

  if (affected_by_spell(victim, SPELL_STONE_FIST))
    return;

  MakeAffect(ch, victim, type,
             SPELL_STONE_FIST, (level/10)-1, APPLY_BHD, 0,
             3, 10, FALSE, FALSE, FALSE, NULL);

  send_to_char("Your fists become lethally hard and stone like.\n\r", victim);
  act("$n's fists become lethally hard and stone like.",TRUE,victim,0,0,TO_ROOM);
}

void spell_astral_group(ubyte level, struct char_data *ch, int type,
                     struct char_data *victim, struct obj_data *obj)
{
  struct char_data *k;
  char buf[MAX_STRING_LENGTH];
  struct room_data *rp;
  int rnum;

  assert(ch);

  if(!(rp = real_roomp(ch->in_room)) || IS_SET(rp->room_flags, NO_TRAVEL_OUT))
    {
      send_to_char("An unseen force prevents you from entering the Astral Plane.\n\r", ch);
      return;
    }

  switch(number(0,3))
  {
  case 0: rnum=7601; break;
  case 1: rnum=7618; break;
  case 2: rnum=7633; break;
  case 3: rnum=7650; break;
  default: rnum=0; log_msg("Error in astral walk -- undefined target. Sending char to void"); break;
  }

  act("$n opens a rift in space and steps in!!",TRUE,ch,0,0,TO_ROOM);

  if(!real_roomp(rnum))
  {
    sprintf(buf,"Astral Walk Failed -- Target room (%d) does not exist.",
	    rnum);
    log_msg(buf);
    send_to_char("I'm sorry, this spell is having technical difficulties.\n\r",ch);
    return;
  }

  for(k=real_roomp(ch->in_room)->people;k;k=k->next_in_room)
  {
    if(in_group(k,ch) && (ch!=k))
    {
      char_from_room(k);
      char_to_room(k, rnum);
      act("You are blinded for a moment as $n appears in a flash of light!",TRUE,k,0,0,TO_ROOM);
      do_look(k, "",15);
    }
  }

  char_from_room(ch);
  char_to_room(ch, rnum);
  act("You are blinded for a moment as $n appears in a flash of light!",TRUE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);

}

struct enforest_extra {
   int left;
   int prev_bits;
   int prev_sector;
};

static int enforest_done(spevent_type *se, long now);
static int enforest_keep(spevent_type *se, long now);
static int enforest_check(spevent_type *se, long now);
static int enforest_free(spevent_type *se, long now);

void spell_enforest(ubyte level, struct char_data *ch, int type,
		    struct char_data *victim, struct obj_data *obj) {
   struct char_data *vict, *caster[10];
   int casters, a, avglevel;
   spevent_list *slist,*snext;
   
   SPEVENT_CHECK_CHAR(slist,snext,ch,SPELL_ENFOREST) {
      SPEVENT_CAST(slist,snext,ch,SPELL_ENFOREST);
      
      send_to_char("You have already cast that spell...\n\r", ch);
      return;
   }
   
   casters=avglevel=0;
   for(vict=real_roomp(ch->in_room)->people;vict;vict=vict->next_in_room) {
      SPEVENT_CHECK_CHAR(slist,snext,vict,SPELL_ENFOREST) {
	 SPEVENT_CAST(slist,snext,vict,SPELL_ENFOREST);
	 
	 if(casters > 10) break;
	 
	 caster[casters++] = vict;
	 avglevel += GetMaxLevel(vict);
      }
   }
   
   avglevel += GetMaxLevel(ch);
   avglevel /= casters+1;
   
   spevent_type *se;
   
   if((casters >= 1) || (IS_IMMORTAL(ch))) {
      for(a=0;a<casters;a++) {
	 SPEVENT_CHECK_CHAR(slist,snext,caster[a],SPELL_ENFOREST) {
	    SPEVENT_CAST(slist,snext,caster[a],SPELL_ENFOREST);
	    
	    spevent_destroy(slist->sp_event);
	    slist=NULL;
	    
	    se = spevent_new();
	    
	    se->type = SPELL_ENFOREST;
	    se->caster = caster[a];
	    se->is_pulse = 1;
	    se->duration = 1;
	    se->expire_func = (event_func) enforest_keep;
	    CREATE(se->extra, enforest_extra, 1);
	    enforest_extra *sex = (enforest_extra *) se->extra;
	    sex->left = 5+(avglevel-32)/2;
	    sex->left *= 40;
	    
	    spevent_depend_char(se, caster[a]);
	    spevent_start(se);
	    
	    cprintf(caster[a], "You now will convert every room you"
		               " enter into an outdoor forest.\n\r");
	 }
      }
      
      se = spevent_new();
      
      CREATE(se->extra, enforest_extra, 1);
      enforest_extra *sex = (enforest_extra *) se->extra;
      
      se->type = SPELL_ENFOREST;
      se->caster = ch;
      se->is_pulse = 1;
      se->duration = 1;
      se->expire_func = (event_func) enforest_keep;
      sex->left = 5+(avglevel-32)/2;
      sex->left *= 40;
      
      spevent_depend_char(se, ch);
      spevent_start(se);
      
      cprintf(ch, "You now will convert every room you"
	          " enter into an outdoor forest.\n\r");
      
      return;
   }
   
   se = spevent_new();
   
   se->type = SPELL_ENFOREST;
   se->caster = ch;
   se->in_room = ch->in_room;
   se->is_pulse = 1;
   se->duration = 1;
   se->expire_func = (event_func) enforest_check;
   
   spevent_depend_char(se, ch);
   spevent_start(se);
   
   send_to_char("Waiting for another druid...\n\r", ch);
   return;
}

int enforest_keep(spevent_type *se, long now) {
   enforest_extra *sex = (enforest_extra *) se->extra;
   spevent_type *senew;
   enforest_extra *sexnew;
   room_data *rm;
   
   if(sex->left-- >= 0) {
      rm = real_roomp(se->caster->in_room);
      
      if(!spevent_on_room(rm,SPELL_ENFOREST)) {
	 senew=spevent_new();
	 
	 senew->in_room = se->caster->in_room;
	 senew->type = SPELL_ENFOREST;
	 senew->is_pulse = 1;
	 senew->duration = 1;
	 senew->expire_func = (event_func) enforest_done;
	 senew->free_func = (event_func) enforest_free;
	 
	 CREATE(sexnew, enforest_extra, 1);
	 senew->extra = sexnew;
	 sexnew->left = sex->left;
	 sexnew->prev_sector = rm->sector_type;
	 sexnew->prev_bits = rm->room_flags;
	 
	 rm->sector_type = SECT_FOREST;
	 REMOVE_BIT(rm->room_flags, INDOORS);
	 
	 spevent_depend_char(senew,se->caster);
	 spevent_depend_room(senew,rm);
	 spevent_start(senew);
      }
      
      spevent_renew(se);
   } else {
      cprintf(se->caster, "Your enforest spell has run out.\n\r");
      spevent_destroy(se);
   }
   return 0;
}

int enforest_done(spevent_type *se, long now) {
   enforest_extra *sex = (enforest_extra *) se->extra;
   
   if(sex->left-- > 0) {
      spevent_renew(se);
   } else {
      spevent_destroy(se);
   }
   
   return 0;
}

int enforest_free(spevent_type *se, long now) {
   enforest_extra *sex = (enforest_extra *) se->extra;
   
   send_to_room("The room suddenly returns back to it's original form.\n\r",se->in_room);
   real_roomp(se->in_room)->sector_type = sex->prev_sector;
   real_roomp(se->in_room)->room_flags = sex->prev_bits;
   
   return 0;
}
     
int enforest_check(spevent_type *se, long now) {
   if(se->caster->in_room != se->in_room) {
      spevent_destroy(se);
   } else {
      spevent_renew(se);
   }
   return 0;
}

static int gust_expire(spevent_type*,long);   
    
void spell_gust(ubyte level, struct char_data *ch, int type,
	       struct char_data *victim, struct obj_data *obj) {
   int dam;
   double szdice, maxsize;
   char_data *vict;
   spevent_list *spevt;

   if(!victim)
     return;

   if((spevt = spevent_on_char(ch, SPELL_GUST)) && !IS_IMMORTAL(ch)) {
      send_to_char("You have to wait a little bit before you can cast this spell again...\n\r", ch);
      return;
   }
   
   if(spevt)
     spevent_destroy(spevt->sp_event);

   maxsize = 20;                // max of lvl d(maxsize+1)
   szdice = maxsize*((float)level/125)+1;

   dam=dice(level, (int)szdice);
   
   if (SkillChance(ch, victim, SUCCESS_AVG, 0, SPLMOD_DEX | SPLMOD_STR, 0) )
       damage(ch, victim, dam, SPELL_GUST);

   if(level >= 50) {
      EACH_CHARACTER(iter, vict) {
         if (ch->in_room==vict->in_room) {
            if (ch!=vict && !in_group(ch,vict)) {
	       if(IS_IMMORTAL(vict))
	         send_to_char( "Some puny mortal tries to hurt you.\n\r", vict);
               else
	         if(can_pkill(ch, vict))
                   wind_effect(level, vict);
	    }
	 } else if(real_roomp(ch->in_room)->zone ==
		    real_roomp(vict->in_room)->zone)
            cprintf(vict, "You feel a gust of wind nearby.\n\r");
      }
      END_AITER(iter);
   }

/*    spevent_type *se; */

/*    se = spevent_new(); */

/*    se->type = SPELL_GUST; */
/*    se->in_room = ch->in_room; */
/*    se->caster = ch; */
/*    se->duration = (int) (7*((float)level/125)); */
/*    se->extra = NULL; */
/*    se->expire_func = (event_func) gust_expire; */

/*    spevent_depend_char(se, ch); */
/*    spevent_start(se); */
}

// This function fires when you can finally cast gust again
int gust_expire(spevent_type *se, long now) {
   send_to_char("The harsh wind calms...\n\r", se->caster);
   spevent_destroy(se);
   return 0;
}


struct pre_inferno_extra {
   int cast_pos;
};

static int inferno_check(spevent_type*, long);

void spell_inferno(ubyte level, struct char_data *ch, int type,
		   struct char_data *victim, struct obj_data *obj) {
   spevent_list *slist,*snext;
   pre_inferno_extra *spextra;
   char_data *caster[4], *vict;
   spevent_type *se;
   pre_inferno_extra *extra;
   int numcasters, i;
   int clss=0, pos=0;
   char cast_msg[MAX_STRING_LENGTH];
   char to_room_msg[MAX_STRING_LENGTH];
   
   /* casters should always be:
    * #1 - Mage
    * #2 - Cleric
    * #3 - Druid
    * #4 - Psionist
    */
   
   numcasters=0;
   for(vict=real_roomp(ch->in_room)->people;vict;vict=vict->next_in_room) {
      SPEVENT_CHECK_CHAR(slist,snext,vict,SPELL_INFERNO) {
	 SPEVENT_CAST(slist,snext,vict,SPELL_INFERNO);
	 
	 if(numcasters > 4) break;
	 
	 spextra = (pre_inferno_extra *)slist->sp_event->extra;
	 caster[spextra->cast_pos] = vict;
	 numcasters++;
      }
   }
   
   switch(numcasters) {
    case 0:
      clss = CLASS_MAGIC_USER;
      sprintf(cast_msg,
	      "You whisper the words $CW'ark tyragoth kyzanthumnus'$CN, and the room\n"
	      "emmits a soft red glow. You now wait for holy powers to bless the spell.\n");
      sprintf(to_room_msg,
	      "%s whispers soft arcane words of a powerful nature, and soft slivers\n"
	      "of red light slowly sift from his fingers to hold the room in a glow\n",
	      GET_NAME(ch));
      break;
    case 1:
      clss = CLASS_CLERIC;
      sprintf(cast_msg,
	      "You get down on your knees and pray to your god that he bless your\n"
	      "party's efforts, and a white glow emerges from your body. You now wait\n"
	      "for the call of nature to assist you in this spell.\n");
      sprintf(to_room_msg,
	      "%s gets down, and begins to pray to %s god for good blessings. As %s does,\n"
	      "A white glow begins to eminate from %s body!\n",
	      GET_NAME(ch), HSHR(ch), HSSH(ch), HSHR(ch));
      break;
    case 2:
      clss = CLASS_DRUID;
      sprintf(cast_msg,
	      "You raise your hands in the air, and call nature to your aid. In response,\n"
	      "the ground beneath you begins to tremble. You now await the final application\n"
	      "of the power of the mind.\n");
      sprintf(to_room_msg,
	      "%s raises %s hands and the room begins to tremble at the might of nature\n"
	      "held at bay. The room is now suffused with a green glow.",
	      GET_NAME(ch), HSHR(ch));
      break;
    case 3:
      clss = CLASS_PSI;
      sprintf(cast_msg,
	      "You focus the powers of your mind, and with much effort wield the powerful\n"
	      "forces at work in the room. The spell begins to take shape and form...\n");
      sprintf(to_room_msg,
	      "%s conentrates with all of %s willpower, and wields the spell into being.\n"
	      "A silvery glow erupts from %s body!\n",
	      GET_NAME(ch), HSHR(ch), HSHR(ch));
      break;
    default:
      log_msg("Illegal number of casters in inferno!");
   }
   
   if((slist = spevent_on_char(ch, SPELL_INFERNO))) {
      cprintf(ch, "You've already prepared to cast the spell...\n");
      spevent_destroy(slist->sp_event);
//      return;
   }
   
   if(!HasClass(ch, clss)) {
      cprintf(ch, "You cannot cast this spell at this moment.\n");
   }
   
   send_to_char_formatted(cast_msg, ch);
   send_to_room_except_formatted(to_room_msg, ch->in_room, ch);
   
   for(i=0;i<numcasters;i++) {
      if(!i) {
	 cprintf(ch, "Casters are:\n");
      }
      cprintf(ch, "%s\n", GET_NAME(caster[i]));
   }
   
   if(numcasters == 4) {
   } else {
      se = spevent_new();
      CREATE(extra, pre_inferno_extra, 1);
      se->extra = extra;
      extra->cast_pos = numcasters;
      
      se->type = SPELL_INFERNO;
      se->caster = ch;
      se->in_room = ch->in_room;
      se->is_pulse = 1;
      se->duration = 1;
      se->expire_func = (event_func) inferno_check;
      
      spevent_depend_char(se, ch);
      spevent_start(se);
      
      dlog("Inferno check started for %s %d", GET_NAME(ch), numcasters+1);
      return;
   }
}

int inferno_check(spevent_type *se, long now) {
   if(se->caster->in_room != se->in_room) {
      spevent_destroy(se);
   } else {
      spevent_renew(se);
   }
   return 0;
}
