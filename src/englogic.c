#include "config.h"

#include <stdio.h>

#include "constants.h"
#include "structs.h"
#include "fight.h"
#include "utils.h"
#include "engine.h"
#include "spell_util.h"
#include "spells.h"
#include "db.h"
#include "utility.h"
#include "multiclass.h"
#include "handler.h"

#ifdef __cplusplus
extern "C"
#endif
    double exp(double);

/* --------------------------- SUPPORT ROUTINES ------------------------ */

int wilderness(struct char_data *ch);

int logic_damage(struct char_data* vict, int imm_bit)
{
    if(IsImmune(vict, imm_bit))
	return 1;
    else if(IsResist(vict, imm_bit))
	return 2;
    else if(IsSusc(vict, imm_bit))
	return 4;
    return 3;
}

/* simulate the saves of the victim, return appropriate value */
int logic_saves (struct char_data *vict, sh_int save_type, long immune_type)
{
   int save;
  
   if (IsImmune(vict,immune_type))
      return (1);

   if (IsSusc(vict,immune_type))
      return (3);

   save = vict->specials.apply_saving_throw[save_type];

   if (!IS_NPC(vict))
      save += saving_throws[BestMagicClass(vict)][save_type][(int)GET_LEVEL(vict,BestMagicClass(vict))];

   save = MAX(1, save);

   if (save>=20)
      return (3);

   save = MIN(19, save);

   if (IsResist(vict,immune_type))
      return ((save/5)-1);

   return (save/5);
}

/* simulate the skill saves of the victim, return appropriate value */
int logic_skill_saves (int clss, struct char_data *ch, struct char_data *vict)
{
   int save;
  
   save=60;
   if (ch->player.level[clss]>GetMaxLevel(vict))
      save=70;
   else if (ch->player.level[clss]<GetMaxLevel(vict))
      save=40;

   if (GET_INT(ch)>GET_WIS(vict))
      save += 5;
   else if (GET_INT(ch)<GET_WIS(vict))
      save -= 10;

   if (GET_DEX(vict)>10)
      save -= 5;
   else if (GET_DEX(vict)<10)
      save += 5;

   return ((save-1)/25);
}

/* simulate the new skill saves of the victim, return appropriate value */
int logic_new_skill_saves (struct char_data *ch, struct char_data *vict,
                           int skill, int extra, long immune)
{
   int val;

   if (IsImmune(vict, immune))
      return (1);
   if (IsSusc(vict, immune))
      return (3);

   val=(int)(100.0*(0.35*exp(0.055*(double)(GetMaxLevel(ch)-GetMaxLevel(vict)))));
   val=MAX(MIN(val,95),6);

   return (val/25);
}

/* ---------------------------- LOGIC ROUTINES ------------------------- */

int logic_acid_rain (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_ACID));
}

int logic_chill_touch (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_COLD));
}

int logic_burning_hands (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_FIRE));
}

int logic_shocking_grasp (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_ELEC));
}

int logic_earthquake (struct char_data *ch, struct char_data *vict)
{
   if (!OUTSIDE(ch))
      return(0);
   return(2);
}

int logic_implode (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_ENERGY));
}

int logic_chain_electrocution (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_ELEC));
}

int logic_harmful_touch (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_ENERGY));
}

int logic_fireball (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_FIRE));
}

int logic_call_lightning (struct char_data *ch, struct char_data *vict)
{
   if (OUTSIDE(ch) && weather_info.sky>=SKY_RAINING)
      return (logic_damage(vict,IMM_ELEC));
   return (0);
}

int logic_energy_drain (struct char_data *ch, struct char_data *vict)
{
   return(logic_saves(vict,SAVING_SPELL,IMM_DRAIN));
}

int logic_rupture (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_ENERGY));
}

int logic_teleport (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict,SAVING_SPELL,IMM_HOLD));
}

int logic_blindness (struct char_data *ch, struct char_data *vict)
{
   if (!affected_by_spell(vict, SPELL_BLINDNESS))
      return (logic_saves(vict,SAVING_SPELL,0));
   return (0);
}

int logic_cure_blind (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING &&
       affected_by_spell(ch, SPELL_BLINDNESS))
      return (2);
   return (0);
}

int logic_cure_critic (struct char_data *ch, struct char_data *vict)
{
   if (GET_HIT(ch) < (GET_MAX_HIT(ch)-50))
      return (2);
   return (0);
}

int logic_cure_light (struct char_data *ch, struct char_data *vict)
{
   if (GET_HIT(ch) < (GET_MAX_HIT(ch)-12))  /* 12 comes from 3d4 */
      return (1);
   return (0);
}

int logic_curse (struct char_data *ch, struct char_data *vict)
{
   if (affected_by_spell(vict, SPELL_CURSE))
      return (0);
   return (logic_saves(vict,SAVING_SPELL,0));
}

int logic_fire_wind (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_FIRE));
}

int logic_frost_cloud (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_COLD));
}

int logic_heal (struct char_data *ch, struct char_data *vict)
{
   if (GET_HIT(ch) < (GET_MAX_HIT(ch)-100))
      return (3);
   return (0);
}

int logic_invisibility (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)<POSITION_STANDING ||
       affected_by_spell(ch,SPELL_INVISIBLE) ||
       IS_AFFECTED(ch, AFF_INVISIBLE))
      return (0);
   return (2);
}

int logic_poison (struct char_data *ch, struct char_data *vict)
{
    return (logic_saves(vict,SAVING_SPELL,IMM_POISON));
}

int logic_poison_gas (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict,SAVING_SPELL,IMM_POISON));
}

int logic_remove_curse (struct char_data *ch, struct char_data *vict)
{
    if ((GET_POS(ch) >= POSITION_STANDING) &&
	affected_by_spell(ch, SPELL_CURSE))
	return (2);
    return (0);
}

int logic_sanctuary (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING &&
       !affected_by_spell(ch, SPELL_SANCTUARY) &&
       !IS_AFFECTED(ch,AFF_SANCTUARY))
      return (3);
   return (0);
}

int logic_sleep (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, IMM_SLEEP));
   return (0);
}

int logic_true_sight (struct char_data *ch, struct char_data *vict)
{
    if (GET_POS(ch)>=POSITION_STANDING &&
	!affected_by_spell(ch, SPELL_TRUE_SIGHT) &&
       !IS_AFFECTED(ch, AFF_TRUE_SIGHT))
      return (2);
   return (0);
}

int logic_lullabye (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, IMM_SLEEP));
   return (0);
}

int logic_despair (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, IMM_SLEEP));
   return (0);
}

int logic_inspire (struct char_data *ch, struct char_data *vict)
{
   return (0);
}

int logic_slow (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, IMM_HOLD));
   return (0);
}

int logic_terror (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, IMM_HOLD));
   return (0);
}

int logic_silence (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)>=POSITION_STANDING)
      return (logic_saves(vict, SAVING_SPELL, 0));
   return (0);
}

int logic_electrocute (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict,SAVING_SPELL,IMM_ELEC));
}

int logic_remove_poison (struct char_data *ch, struct char_data *vict)
{
   if ((GET_POS(ch)>=POSITION_STANDING) && 
       affected_by_spell(ch, SPELL_POISON))
      return (2);
   return (0);
}

int logic_flamestrike (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict, SAVING_SPELL, IMM_FIRE));
}

int logic_electric_fire (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict, SAVING_SPELL, IMM_ELEC));
}

int logic_weakness (struct char_data *ch, struct char_data *vict)
{
   if (!affected_by_spell(vict, SPELL_WEAKNESS))
      return (logic_saves(vict, SAVING_SPELL, IMM_DRAIN));
   return (0);
}

int logic_dispel_magic (struct char_data *ch, struct char_data *vict)
{
   if (GetMaxLevel(ch)>=GetMaxLevel(vict))
      if (IS_AFFECTED(vict, AFF_SANCTUARY) ||
          affected_by_spell(vict, SPELL_SANCTUARY) ||
          affected_by_spell(vict, SPELL_ELECSHIELD) ||
          affected_by_spell(vict, SPELL_COLDSHIELD) ||
          affected_by_spell(vict, SPELL_POISONSHIELD) ||
          affected_by_spell(vict, SPELL_ENERGYSHIELD) ||
          affected_by_spell(vict, SKILL_VAMPSHIELD) ||
          affected_by_spell(vict, SKILL_MANASHIELD) ||
	  affected_by_spell(vict, SPELL_MOVESHIELD) ||
          affected_by_spell(vict, SPELL_ACIDSHIELD) ||
          IS_AFFECTED(vict, AFF_FIRESHIELD) ||
          affected_by_spell(vict, SPELL_FIRESHIELD))

      return (4);
   return (0);
}

int logic_paralyze (struct char_data *ch, struct char_data *vict)
{
   if (IS_AFFECTED(vict, AFF_PARALYSIS))
      return (0);
   return (logic_saves(vict, SAVING_PARA, IMM_HOLD));
}

int logic_fear (struct char_data *ch, struct char_data *vict)
{
   if (GetMaxLevel(ch)<=GetMaxLevel(vict) || IsUndead(vict))
      return (0);
   return (logic_saves(vict, SAVING_SPELL, IMM_HOLD));
}

int logic_acid_blast (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict, SAVING_SPELL, IMM_ACID));
}

int logic_decay (struct char_data *ch, struct char_data *vict)
{
   return (logic_saves(vict, SAVING_SPELL, IMM_ENERGY));
}

int logic_wind_storm (struct char_data *ch, struct char_data *vict)
{
   if (!OUTSIDE(ch))
      return(0);
   return (2);
}

int logic_ice_storm (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_COLD));
}

int logic_vampyric_touch (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_DRAIN));
}

int logic_monsum (struct char_data *ch, struct char_data *vict)
{
   vict=FindVictim(ch);

   if (!CountFollowers(ch) || !vict)
      return (0);
   return (logic_damage(vict,IMM_BLUNT));
}

int logic_fireshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED(ch, AFF_FIRESHIELD) &&
       !affected_by_spell(ch, SPELL_FIRESHIELD))
      return (3);
   return (0);
}

int logic_elecshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_ELECSHIELD) &&
       !affected_by_spell(ch, SPELL_ELECSHIELD))
      return (3);
   return (0);
}

int logic_coldshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_COLDSHIELD) &&
       !affected_by_spell(ch, SPELL_COLDSHIELD))
      return (3);
   return (0);
}

int logic_poisonshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_POISONSHIELD) &&
       !affected_by_spell(ch, SPELL_POISONSHIELD))
      return (3);
   return (0);
}

int logic_energyshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_ENERGYSHIELD) &&
       !affected_by_spell(ch, SPELL_ENERGYSHIELD))
      return (3);
   return (0);
}

int logic_vampshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_VAMPSHIELD) &&
       !affected_by_spell(ch, SKILL_VAMPSHIELD))
      return (3);
   return (0);
}

int logic_manashield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_MANASHIELD) &&
       !affected_by_spell(ch, SKILL_MANASHIELD))
      return (3);
   return (0);
}

int logic_moveshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_MOVESHIELD) &&
       !affected_by_spell(ch, SPELL_MOVESHIELD))
      return (3);
   return (0);
}

int logic_acidshield (struct char_data *ch, struct char_data *vict)
{
   if (!IS_AFFECTED2(ch, AFF2_ACIDSHIELD) &&
       !affected_by_spell(ch, SPELL_ACIDSHIELD))
      return (3);
   return (0);
}

int logic_cure_serious (struct char_data *ch, struct char_data *vict)
{
   if (GET_HIT(ch) < (GET_MAX_HIT(ch)-25))
      return (2);
   return (0);
}

int logic_refresh (struct char_data *ch, struct char_data *vict)
{
   if (GET_MOVE(ch)<move_limit(ch))
      return (2);
   return (0);
}

int logic_turn (struct char_data *ch, struct char_data *vict)
{
   if (GetMaxLevel(ch)<=GetMaxLevel(vict) || !IsUndead(vict))
      return (0);
   return (logic_saves(vict, SAVING_SPELL, IMM_HOLD));
}

int logic_faerie_fire (struct char_data *ch, struct char_data *vict)
{
   if (affected_by_spell(vict, SPELL_FAERIE_FIRE))
      return (0);
   return (logic_saves(vict, SAVING_SPELL, 0));
}

int logic_web (struct char_data *ch, struct char_data *vict)
{
   if (GET_MOVE(vict)>move_limit(vict)/4)
      return (logic_saves(vict, SAVING_PARA, IMM_HOLD));
   return (0);
}

int logic_disintegrate (struct char_data *ch, struct char_data *vict)
{
   return (logic_damage(vict,IMM_ENERGY));
}

int logic_lava_storm (struct char_data *ch, struct char_data *vict)
{
   return(logic_damage(vict, IMM_FIRE));
}

/* ------------------------------------------------------------------ */

int logic_sneak (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)<POSITION_STANDING || IS_AFFECTED(ch, AFF_SNEAK))
      return (0);

   return (3);
}

int logic_hide (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)<POSITION_STANDING || IS_AFFECTED(ch, AFF_HIDE) ||
      !IS_SET(ch->specials.mob_act, ACT_SENTINEL))
      return (0);

   return (3);
}

int logic_trip (struct char_data *ch, struct char_data *vict)
{
   int val;

   if (ch->attackers>3 ||
       vict->attackers>5 ||
       MOUNTED(vict))
      return (0);

   val=50;
   val -= dex_app[GET_DEX(ch)].reaction*10;
   val += dex_app[GET_DEX(vict)].reaction*10;
   if (GetMaxLevel(vict)>12)
      val += ((GetMaxLevel(vict)-10)*5);

   val=MIN(75,100-val);
   return (val/25);
}

int logic_backstab (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)<POSITION_STANDING)
      return (0);

   if (IS_SET(vict->specials.mob_act, ACT_HUGE) || !ch->equipment[WIELD] ||
       ch->attackers || vict->attackers>=3 || ch->specials.fighting ||
       MOUNTED (vict) || IsImmune(vict, IMM_PIERCE))
      return (0);

   if (ch->equipment[WIELD]->obj_flags.value[3] != 5 &&
       ch->equipment[WIELD]->obj_flags.value[3] != 6 &&
       ch->equipment[WIELD]->obj_flags.value[3] != 7 &&
       ch->equipment[WIELD]->obj_flags.value[3] != 8 &&
       ch->equipment[WIELD]->obj_flags.value[3] != 9 )
      return (0);

   return (3);
}

int logic_kick (struct char_data *ch, struct char_data *vict)
{
   int val;

   if (ch->attackers>2 ||
       vict->attackers>2 ||
       MOUNTED(vict))
      return (0);

   val=((10-(GET_AC(vict)/10))<<1)+50;
   val=MIN(75,100-val);

   return (val/25)+1;
}

int logic_bash (struct char_data *ch, struct char_data *vict)
{
   int val;

   if (!ch->equipment[WEAR_SHIELD]) 
       return 0;
   
   if (ch->attackers>3 ||
       vict->attackers>5 ||
       MOUNTED(vict))
      return (0);

   val=50;
   val -= dex_app[GET_DEX(ch)].reaction*10;
   val += dex_app[GET_DEX(vict)].reaction*10;
   if (GetMaxLevel(vict)>12)
      val += ((GetMaxLevel(vict)-10)*5);

   val=MIN(75,100-val);
   return (val/25);
}

int logic_sunray (struct char_data *ch, struct char_data *vict)
{
   if (!OUTSIDE(ch) ||
      weather_info.sunlight==SUN_DARK ||
      affected_by_spell(vict,SPELL_BLINDNESS)) 
      return (0);

   return (logic_saves(vict, SAVING_SPELL, 0));   
}

int logic_ansum (struct char_data *ch, struct char_data *vict)
{
   if (!wilderness(ch))
      return (0); 

   vict=FindVictim(ch);

   if (!CountFollowers(ch) || !vict)
      return (0);

   return (logic_damage(vict,IMM_BLUNT));   
}

int logic_thorn (struct char_data *ch, struct char_data *vict)
{
   if (!wilderness(ch))
      return (0);

   return (2);
}

int logic_vine (struct char_data *ch, struct char_data *vict)
{
   if (!wilderness(ch))
      return (0);

   return (2);
}

int logic_creeping_doom (struct char_data *ch, struct char_data *vict)
{
   if (!wilderness(ch))
      return (0);
   return (2);
}

int logic_camouflage (struct char_data *ch, struct char_data *vict)
{
   if (!wilderness(ch))
      return (0);

   return (logic_sneak(ch, vict));
}

int logic_geyser (struct char_data *ch, struct char_data *vict)
{
   if (!OUTSIDE(ch))
      return(0);
   return(2);
}

int logic_vanish (struct char_data *ch, struct char_data *vict)
{
   if (GET_POS(ch)<POSITION_STANDING ||
       affected_by_spell(ch,SPELL_INVISIBLE) ||
       IS_AFFECTED(ch, AFF_INVISIBLE))
      return (0);

   return (3);
}

int logic_illusionary_shroud(struct char_data *ch, struct char_data *vict)
{
   return (3);
}

int logic_blur(struct char_data *ch, struct char_data *vict)
{
   return (3);
}


int logic_phantasmal (struct char_data *ch, struct char_data *vict)
{
   vict=FindVictim(ch);

   if (!CountFollowers(ch) || !vict)
      return (0);

   return (logic_damage(vict,IMM_BLUNT));
}

int logic_spell_shield (struct char_data *ch, struct char_data *vict)
{
  if (affected_by_spell(ch,SKILL_SPELL_SHIELD) ||
      !IS_SET(ch->specials.mob_act, ACT_SENTINEL))
      return (0);
   return (3);
}

int logic_psionic_blast (struct char_data *ch, struct char_data *vict)
{
   return (3);   
}

int logic_lay_on_hands (struct char_data *ch, struct char_data *vict)
{
   if (ch->specials.alignment<350)
      return (0);

   if ((ch->points.max_hit - ch->points.hit) <
       (ch->player.level[PALADIN_LEVEL_IND]*3))
      return (0);
   return (3); 
}

int logic_holy_warcry (struct char_data *ch, struct char_data *vict)
{
   int diff;

   if (ch->specials.alignment<350)
      return (0);
   
   diff=ch->player.level[PALADIN_LEVEL_IND]-GetMaxLevel(vict);
   if (diff>19)
      return (3);
   if (diff<=-11)
      return (0);

   if (diff>10)
      diff=1;
   else if (diff>-6)
      diff=0;
   else 
      diff=-1;

   diff += logic_skill_saves(PALADIN_LEVEL_IND, ch, vict);
   diff=MIN(3,diff);
   diff=MAX(0,diff);
   
   return (diff);
}

int logic_thrust (struct char_data *ch, struct char_data *vict)
{
   return (logic_skill_saves(RANGER_LEVEL_IND, ch, vict));
}

int logic_stun (struct char_data *ch, struct char_data *vict)
{
   if (ch->points.move<30 || IS_AFFECTED(vict,AFF_PARALYSIS))
      return (0);
   return (logic_new_skill_saves(ch, vict, SKILL_STUN, 0, IMM_HOLD));   
}

int logic_disarm (struct char_data *ch, struct char_data *vict)
{
   int val;

   if (ch->attackers>3 || !vict->equipment[WIELD])
      return (0);

   val=50;
   val -= dex_app[GET_DEX(ch)].reaction*10;
   val += dex_app[GET_DEX(vict)].reaction*10;
   if (!ch->equipment[WIELD])
      val -= 50;
   if (GetMaxLevel(vict)>20)
      val += ((GetMaxLevel(vict)-18)*5);

   val=MIN(75,100-val);

   ch->skills[SKILL_DISARM].learned=90; /* give them the skill since it's
                                           learned from the ninja */   
   return (val/25);
}

int logic_berserk (struct char_data *ch, struct char_data *vict)
{
   if (!ch->specials.fighting ||
       IS_AFFECTED(ch, AFF_BERSERK) ||
       GET_MANA(ch)<17 ||
       GET_MOVE(ch)<17)
      return (0);

   return (3);
}

int logic_tolerance(struct char_data *ch, struct char_data *vict)
{
   return (3);
}

