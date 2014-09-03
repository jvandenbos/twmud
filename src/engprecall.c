#include <stdio.h>
#include <string.h>

#include "config.h"

#include "structs.h"
#include "fight.h"
#include "spells.h"
#include "utils.h"
#include "comm.h"
#include "engine.h"
#include "utility.h"
#include "spell_procs.h"
#include "multiclass.h"
#include "act.h"
#include "handler.h"
#include "db.h"

/*** ----------------------------- SPELLS ----------------------------- ***/

void pre_acid_rain (struct char_data *ch, struct char_data *vict)
{
   act("The skies darken as $n utters the words 'acid rain'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_acid_rain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_chill_touch (struct char_data *ch, struct char_data *vict)
{
   act("The fingers of $n turn ice blue with an invocation of 'chill touch'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_burning_hands (struct char_data *ch, struct char_data *vict)
{
   act("The hands of $n blast searing flames with the words 'burning hands'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_shocking_grasp (struct char_data *ch, struct char_data *vict)
{
   act("Electricity crackles from the hands of $n with the words 'shocking grasp'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_earthquake (struct char_data *ch, struct char_data *vict)
{
   act("The ground trembles as $n calls upon natural forces with 'earthquake'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_earthquake(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_implode (struct char_data *ch, struct char_data *vict)
{
   act("$n conjure crushing forces with the words 'implode'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_implode(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_chain_electrocution (struct char_data *ch, struct char_data *vict)
{
   act("Sparks fly as $n utters the words 'chain electrocution'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_chain_electrocution(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_harmful_touch (struct char_data *ch, struct char_data *vict)
{
   act("$n reaches out with a black hand after uttering 'harmful touch'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_harmful_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_fireball (struct char_data *ch, struct char_data *vict)
{
   act("With power of mind and hand, $n lets fly a massive 'fireball'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_call_lightning (struct char_data *ch, struct char_data *vict)
{
   act("Electricity flies as $n beckons to the skies with 'call lightning'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_energy_drain (struct char_data *ch, struct char_data *vict)
{
   act("The eyes of $n become dark and sinister with the words 'energy drain'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_rupture (struct char_data *ch, struct char_data *vict)
{
   act("$n conjure explosive force with the word 'rupture'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_rupture(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_teleport (struct char_data *ch, struct char_data *vict)
{
   act("$n creates a rip in the fabric of space with the word 'teleport'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_blindness (struct char_data *ch, struct char_data *vict)
{
   act("$n conjures a blast of intense light with an utterance of 'blindness'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_cure_blind (struct char_data *ch, struct char_data *vict)
{
   act("Clarity returns to the vision of $n with the words 'cure blindness'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_cure_critic (struct char_data *ch, struct char_data *vict)
{
   act("$n rejuvenates with the words 'cure critic'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_cure_light (struct char_data *ch, struct char_data *vict)
{
   act("$n rejuvenates with the words 'cure light'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_curse (struct char_data *ch, struct char_data *vict)
{
   act("$n spits and froths while screaming the word 'curse'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_fire_wind (struct char_data *ch, struct char_data *vict)
{
   act("$n creates a blast of flaming wind with the words 'fire wind'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_fire_wind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_frost_cloud (struct char_data *ch, struct char_data *vict)
{
   act("$n creates a cloud of searing cold with the words 'frost cloud'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_frost_cloud(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_heal (struct char_data *ch, struct char_data *vict)
{
   act("$n casts 'heal'.",TRUE, ch, 0, 0, TO_ROOM);
   cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_invisibility (struct char_data *ch, struct char_data *vict)
{
   act("$n vigorously gestures, becoming 'invisible'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF_FLAGS(ch), AFF_INVISIBLE); /* perm invis */
}

void pre_poison (struct char_data *ch, struct char_data *vict)
{
   act("$n calls upon all that is noxious with an utterance of 'poison'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_poison_gas (struct char_data *ch, struct char_data *vict)
{
   act("$n exhales a thick noxious fog with an utterance of 'poison gas'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_poison_gas(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_remove_curse (struct char_data *ch, struct char_data *vict)
{
   act("$n recovers with the words 'remove curse'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_sanctuary (struct char_data *ch, struct char_data *vict)
{
   act("Drawing upon inner strength, $n is now protected by 'sanctuary'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF_FLAGS(ch), AFF_SANCTUARY); /* perm sanc restored */
}

void pre_sleep (struct char_data *ch, struct char_data *vict)
{
   act("With a soothing and relaxed voice, $n softly utters the word 'sleep'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_sleep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_true_sight (struct char_data *ch, struct char_data *vict)
{
    act("$N's eyes fill with a ghostly glow as $n utters the words 'true sight'.",
      TRUE, ch, 0, 0, TO_ROOM);
    cast_true_seeing(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_lullabye (struct char_data *ch, struct char_data *vict)
{
   act("With a soothing and relaxed voice, $n sings a soft lullabye.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_lullabye(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_inspire (struct char_data *ch, struct char_data *vict)
{
   act("$n strums out a strident marching tune.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_inspire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_despair (struct char_data *ch, struct char_data *vict)
{
   act("$n sings a dreadful dirge.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_despair(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_slow (struct char_data *ch, struct char_data *vict)
{
   act("With a soothing and relaxed voice, $n sings a lilting tune.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_slow(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_blur(struct char_data *ch, struct char_data *vict)
{
   act("With an odd cant and a wavery voice, $n sings a song of illusion.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_blur(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_terror (struct char_data *ch, struct char_data *vict)
{
   act("With a horrific and terrifying voice, $n sings a song of pain and terror!.",
      TRUE, ch, 0, 0, TO_ROOM);      
   cast_terror(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_silence (struct char_data *ch, struct char_data *vict)
{
   act("With a fading lilt, $n sings you into silence.",
        TRUE, ch, 0, 0, TO_ROOM);      
   cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_electrocute (struct char_data *ch, struct char_data *vict)
{
   act("$n builds up a great charge of electricity with the word 'electrocute'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_electrocute(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_remove_poison (struct char_data *ch, struct char_data *vict)
{
   act("$n drives off the impurities within with the words 'remove poison'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_flamestrike (struct char_data *ch, struct char_data *vict)
{
   act("$n brings forth a blast of fire with an utterance of 'flamestrike'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_electric_fire (struct char_data *ch, struct char_data *vict)
{
   act("$n creates a mass of crackling flame with an utterance of 'electric fire'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_electric_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_weakness (struct char_data *ch, struct char_data *vict)
{
   act("$n drains the strength of others with an utterance of 'weakness'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_dispel_magic (struct char_data *ch, struct char_data *vict)
{
   act("$n dissolves some magic with an invocation of 'dispel magic'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_paralyze (struct char_data *ch, struct char_data *vict)
{
   act("The air seems to thicken and solidify as $n utters the word 'paralyze'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_acid_blast (struct char_data *ch, struct char_data *vict)
{
   act("The air reeks with noxious fumes as $n utters the words 'acid blast'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_decay (struct char_data *ch, struct char_data *vict)
{
   act("$n attacks the living flesh with the word 'decay'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_decay(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_wind_storm (struct char_data *ch, struct char_data *vict)
{
   act("The winds reach hurricane proportions as $n utters the words 'wind storm'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_wind_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_ice_storm (struct char_data *ch, struct char_data *vict)
{
   act("The air freezes with flying ice as $n utters the words 'ice storm'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_vampyric_touch (struct char_data *ch, struct char_data *vict)
{
   act("$n grins evilly and reaches out with clawed hands after uttering 'vampyric touch'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_vampyric_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_monsum (struct char_data *ch, struct char_data *vict)
{
   act("$n conjures assistance from another plane with an utterance of 'monsum'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_monsum(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
   do_order(ch,"followers guard on",0);
}

void pre_fireshield (struct char_data *ch, struct char_data *vict)
{
   act("$n throws up a wall of flame, and is now protected by a 'fireshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF_FLAGS(ch), AFF_FIRESHIELD);  /* perm fireshield */
}

void pre_elecshield (struct char_data *ch, struct char_data *vict)
{
   act("$n throws up a wall of sparks, and is now protected by a 'elecshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_ELECSHIELD);  /* perm elecshield */
}

void pre_coldshield (struct char_data *ch, struct char_data *vict)
{
   act("$n throws up a wall of ice, and is now protected by a 'coldshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_COLDSHIELD);  /* perm coldshield */
}

void pre_poisonshield (struct char_data *ch, struct char_data *vict)
{
   act("$n throws up a wall of snakes, and is now protected by a 'poisonshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_POISONSHIELD);  /* perm poisonshield */
}

void pre_energyshield (struct char_data *ch, struct char_data *vict)
{
   act("$n starts generating power, and is now protected by a 'energyshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_ENERGYSHIELD);  /* perm energyshield */
}

void pre_vampshield (struct char_data *ch, struct char_data *vict)
{
   act("$n grows some deadly fangs, and is now protected by a 'vampshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_VAMPSHIELD);  /* perm vampshield */
}

void pre_manashield (struct char_data *ch, struct char_data *vict)
{
   act("$n starts magnetism, and is now protected by a 'manashield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_MANASHIELD);  /* perm manashield */
}

void pre_moveshield (struct char_data *ch, struct char_data *vict)
{
   act("$n starts radiating, and is now protected by a 'moveshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_MOVESHIELD);  /* perm moveshield */
}

void pre_acidshield (struct char_data *ch, struct char_data *vict)
{
   act("$n throws up an erroding barrier, and is now protected by a 'acidshield'.",
      TRUE, ch, 0, 0, TO_ROOM);
   SET_BIT(AFF2_FLAGS(ch), AFF2_ACIDSHIELD);  /* perm acidfireshield */
}


void pre_cure_serious (struct char_data *ch, struct char_data *vict)
{
   act("$n rejuvenates with the words 'cure serious'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_refresh (struct char_data *ch, struct char_data *vict)
{
   act("$n grows livelier with an utterance of 'refresh'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_fear (struct char_data *ch, struct char_data *vict)
{
   act("$n exploits hidden terrors with the word 'fear'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_turn (struct char_data *ch, struct char_data *vict)
{
   act("$n repels the undead with the word 'turn'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_turn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
}

void pre_faerie_fire (struct char_data *ch, struct char_data *vict)
{
   act("$n calls upon magical faeries for aid with the words 'faerie fire'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_faerie_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_web (struct char_data *ch, struct char_data *vict)
{
   act("Fine sticky strands fly outward as $n utters the word 'web'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_disintegrate (struct char_data *ch, struct char_data *vict)
{
   act("$n wreaks havoc at the molecular level with the word 'disintegrate'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_disintegrate(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_lava_storm (struct char_data *ch, struct char_data *vict)
{
    act("Lava erupts from below.",TRUE, ch, 0, 0, TO_ROOM);
    cast_lava_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}


/*** ----------------------------- SKILLS ----------------------------- ***/


void pre_sneak (struct char_data *ch, struct char_data *vict)
{
   do_sneak (ch, "", 0);
}

void pre_hide (struct char_data *ch, struct char_data *vict)
{
   do_hide (ch, "", 0);
}

void pre_backstab (struct char_data *ch, struct char_data *vict)
{
   do_backstab (ch, GET_REAL_NAME(vict), 0);
}

void pre_trip (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_trip (ch, "", 375);
   else
      do_trip (ch, GET_REAL_NAME(vict), 375);
}

void pre_kick (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)    /* looks redundant, but is needed because */
      do_kick (ch, "", 0);      /* of poly's having the same name */
   else
      do_kick (ch, GET_REAL_NAME(vict), 0);
}

void pre_bash (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_bash (ch, "", 157);
   else
      do_bash (ch, GET_REAL_NAME(vict), 157);
}

void pre_sunray (struct char_data *ch, struct char_data *vict)
{
   cast_sunray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_ansum (struct char_data *ch, struct char_data *vict)
{
   act("$n conjures assistance from the wilds with an utterance of 'ansum'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_ansum(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
   do_order(ch,"followers guard on",0);
}

void pre_thorn (struct char_data *ch, struct char_data *vict)
{
   act("$n calls to the plants for aid with the word 'thorn'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_thorn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_vine (struct char_data *ch, struct char_data *vict)
{
   act("The ground rumbles as $n utters the word 'vine'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_vine(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_creeping_doom (struct char_data *ch, struct char_data *vict)
{
   cast_creep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_camouflage (struct char_data *ch, struct char_data *vict)
{
   act("$n fades into the foliage and treads silently with the word 'camouflage'.",
      TRUE, ch, 0, 0, TO_ROOM);
   cast_camouflage(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_geyser (struct char_data* ch, struct char_data* vict)
{
    cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
}

void pre_vanish (struct char_data *ch, struct char_data *vict)
{
   if (ch->skills[SKILL_INVIS].learned<number(1,101))
      ch->points.mana-=5;
   else {
      ch->points.mana -=10;
      act ("$n suddenly disappears!",TRUE,ch,0,0,TO_ROOM);
      SET_BIT(AFF_FLAGS(ch), AFF_INVISIBLE); /* perm invis */
   }
   WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void pre_illusionary_shroud(struct char_data *ch, struct char_data *vict)
{
   do_illusionary_shroud(ch, "", 0);
}

void pre_phantasmal (struct char_data *ch, struct char_data *vict)
{
   do_phantasmal_killer (ch, "", 0);
   do_order(ch,"followers guard on",0);
}

void pre_spell_shield (struct char_data *ch, struct char_data *vict)
{
   struct affected_type af;
   memset(&af, 0, sizeof(af));

   if (ch->skills[SKILL_SPELL_SHIELD].learned < dice (1,101))
      ch->points.mana -= 15;
   else {
      act ("$n is surounded by a magical globe which is tranparent.",TRUE,ch,0,0,TO_ROOM);
      ch->points.mana -= 30;
 
      af.type      = SKILL_SPELL_SHIELD;
      af.location  = APPLY_IMMUNE;
      af.duration  = 0;          /* give longer duration for our mobs :) */
      af.modifier  = IMM_FIRE | IMM_ELEC | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_ENERGY;
      af.bitvector = 0;
      af.mana_cost = 10;
      af.caster    = ch;
      DLOG(("Calling affect_to_char from pre_spell_shield. engprecall.c line 600.\r\n"));
      affect_to_char (ch,&af);
   }
   WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void pre_psionic_blast (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_psi_attack (ch, "", 389);
   else
      do_psi_attack (ch, GET_REAL_NAME(vict), 389);
}

void pre_lay_on_hands (struct char_data *ch, struct char_data *vict)
{
   do_lay_on_hands (ch, GET_REAL_NAME(ch), 0);
}

void pre_holy_warcry (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_holy_warcry (ch, "", 0);
   else
      do_holy_warcry (ch, GET_REAL_NAME(vict), 0);
}

void pre_thrust (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_thrust (ch, "", 0);
   else
      do_thrust (ch, GET_REAL_NAME(vict), 0);
}

void pre_stun (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_stun (ch, "", 0);
   else
      do_stun (ch, GET_REAL_NAME(vict), 0);
}

void pre_disarm (struct char_data *ch, struct char_data *vict)
{
   if(ch->specials.fighting)
      do_disarm (ch, "", 0);
   else
      do_disarm (ch, GET_REAL_NAME(vict), 0);
}

void pre_berserk (struct char_data *ch, struct char_data *vict)
{
   do_berserk (ch, "", 0);
}

void pre_tolerance(struct char_data *ch, struct char_data *vict)
{
   do_tolerance(ch, "", 0);
}
