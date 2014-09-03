#include "config.h"
#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "structs.h"
#include "utils.h"
#include "race.h"
#include "spells.h"
#include "comm.h"
#include "handler.h"
#include "hash.h"
#include "db.h"
#include "interpreter.h"
#include "fight.h"
#include "utility.h"
#include "constants.h"
#include "multiclass.h"
#include "skills.h"
#include "parse.h"
#include "spell_util.h"
#include "act.h"
#include "modify.h"
#include "magicutils.h"
#include "spelltab.h"
#include "newsaves.h"
#include "spell_procs.h"
#include "spec.h"
#include "trap.h"
#include "util_str.h"
#include "find.h"
#include "track.h"
#include "statistic.h"
#include "vnum.h"
#include "vnum_mob.h"
#include "opinion.h"
#include "interpreter.h"
#include "cmdtab.h"
#include "proto.h"
#include "spell_events.h"

extern void area_affect(int level, int dam, struct char_data *ch, long spell, long imm_type, char mes1[MAX_STRING_LENGTH], char mes2[MAX_STRING_LENGTH]);
extern int DamageTrivia(struct char_data *ch, struct char_data *vict, int dam, int type);
extern int DamageEpilog(struct char_data *ch, struct char_data *vict, int type);
extern int DoDamage(struct char_data *ch, struct char_data *vict, int dam, int type);
extern list_head real_events;

/*** forward declarations ***/
#define USE_MANA(ch, si) (si->min_usesmana)
void sing_song( struct char_data *ch, struct spell_info* spell);
int CanLearn( struct char_data *ch, struct spell_info* spell);

void fix_mob_bits(struct char_data *mob);
void stun_opponent(struct char_data* ch,
		   struct char_data* victim,
		   int skill,
		   int duration);

/*
 * This function, heading in the direction of a general skill comparison
 * compares the levels of the characters + some other factors and
 * determines success of a skill based on a base percentage
 */

bool SkillChance(struct char_data *ch, /* the attacker */
		 struct char_data *victim, /* the victim */
		 int basechance, /* base chance of success for skill */
		 int immunity, /* immunity, ie: IMM_POISON, ignored if 0 */
		 int modifiers, /* modifiers to calculation, ie: SPLMOD_DEX */
		 int SKILLNUM) { /* skill number */

  int chance = basechance;

  if(victim) {
    chance += GetMaxLevel(ch) - GetMaxLevel(victim); /* use diff in levels */

    if (modifiers & SPLMOD_DEX)
      chance += GET_DEX(ch) - GET_DEX(victim);
    if (modifiers & SPLMOD_INT)
      chance += GET_INT(ch) - GET_INT(victim);
    if (modifiers & SPLMOD_WIS)
      chance += GET_WIS(ch) - GET_WIS(victim);
    if (modifiers & SPLMOD_CHA)
      chance += GET_CHA(ch) - GET_CHA(victim);
  }

  if(victim) {
    if (modifiers & SPLMOD_CON)
      chance -= GET_CON(victim) - 13; /* 5% less chance if con 18, 5% more if con 8 */
    if (modifiers & SPLMOD_STR)
      chance += GET_STR(ch) - GET_STR(victim);
    if (modifiers & SPLMOD_AC) /* armor class 10% less for -20 ac, 10% more for 20ac */
      chance += (MAX(-20, MIN(20, (victim->points.armor+dex_app[GET_DEX(victim)].defensive)/10))) / 10;
  }

  if (SKILLNUM) /* calculate in the percentage learned */
    chance += skill_chance(ch, SKILLNUM);


  if (immunity && victim) {
    if (IsImmune(victim,immunity)) /* no chance if immune */
      chance /= 4;
    else if (IsResist(victim,immunity)) /* half chance if resist */
      chance /= 2;
    else if (IsSusc(victim,immunity)) /* double the chance if susceptible */
      chance *= 2;
  }

  chance = MIN(SUCCESS_MAX, chance); // They can't have a better than SUCCESS_MAX chance (95% chance)
  chance = MAX(chance, 5);
  if (percent() < chance)
    return TRUE; /* successful skill */
  else
    return FALSE; /* failed skill */
}


/*** support routines ***/
bool SkillSave (int clss,struct char_data *ch,struct char_data *target)
{
  int rand,border;

  if (ch->player.level[clss]>GetMaxLevel(target))
    border = 70;
  else
    if (ch->player.level[clss]<GetMaxLevel(target))
      border = 40;
    else
      border = 60;

  if (GET_INT(ch)>GET_WIS(target))
    border += 5;
  else
    if (GET_INT(ch)<GET_WIS(target))
      border -= 10;

  rand = dice (1,20);
  if (rand>GET_DEX(target))
    border += 5;
  else
    if (rand<GET_DEX(target))
      border -=5;


  if (dice(1,101)<border)
    return (TRUE);
  else
    return (FALSE);
}

int can_do(struct char_data *ch, int skill)
{

  // TMP: log all skills performed
  char buff[255];

  if (IS_PC(ch)) {
    sprintf(buff, "%s,%s\n\r", GET_NAME(ch), spell_name(skill));
    file_log(buff, "actions/skills.log");
  }

  struct spell_info *ability;
  int x, flag=FALSE;
  int is_switched = 0;

  if (ch->orig != NULL)
    if (IS_IMMORTAL(ch->orig))
      is_switched = 1;

  if (IS_AFFECTED(ch, AFF_CHARM) && !is_switched) {
    send_to_char("You are CHARMED.\n\r",ch);
    return(FALSE);
  }

  if (!ch->skills || !ch->skills[skill].learned) {
    send_to_char_formatted("$CcYou lack the training to do it.$CN\n\r",ch);
    return(FALSE);
  }

  ability=spell_by_number(skill);
  for (x=0; x<=MAX_LEVEL_IND && !flag; x++) {
    if (GET_LEVEL(ch, x)>=ability->min_level[x])
      flag=TRUE;
  }

  if (IS_NPC(ch))
    flag = TRUE;

  if (!flag) {
    send_to_char_formatted("$CcYou've lost the ability to do this skill.$CN\n\r", ch);
    return (FALSE);
  }

  if (IS_SET(ability->targets,TAR_VIOLENT) && check_peaceful(ch,"")) {
    send_to_char_formatted("$CwIt is too peaceful here for such an action.$CN\n\r", ch);
    return (FALSE);
  }

  if (GET_MANA(ch) < ability->min_usesmana) {
    send_to_char_formatted("$CcYou don't have the mental strength to do it.\n\r$CN",ch);
    return (FALSE);
  }

  return(TRUE);
}

struct char_data *set_target(struct char_data *ch, char *arg, int skill)
{
  struct char_data *victim;
  struct obj_data *obj;
  struct spell_info *ability;
  char name[MAX_STRING_LENGTH];

  only_argument(arg,name);

  ability=spell_by_number(skill);
  if (!spell_target(ch, ability->targets, name, &victim, &obj))
    return (NULL);

  if (victim->in_room != ch->in_room) {
    send_to_char_formatted("$CrYour victim is no longer here.$CN\n\r", ch);
    return (NULL);
  }

  return (victim);
}

void charge_mana(struct char_data *ch, int skill, int success)
{
  struct spell_info *ability;

  ability=spell_by_number(skill);

  if (success)
    GET_MANA(ch) -= ability->min_usesmana;
  else
    GET_MANA(ch) -= (ability->min_usesmana/2);
}

/*** skill procs ***/

void bash(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  int mods;
  char* verb;
  int skill = cmd == 157 ? SKILL_BASH : SKILL_TRIP;

  verb = (char *) ((cmd == 157) ? "bash" : "trip");

  // minwork -= the above is dangerous
  // if we write to verb later we're screwed
  // but since i'm just hacking a fix...
  // a better way would be:

  // strcpy(verb, ((cmd == 157) ? "bash" : "trip"));

  if (!can_do(ch, skill))
    return;

  if (!(victim=set_target(ch, argument, skill)))
    return;

  if (ch->attackers > 3) {
    act("$CYThere's no room to $T!$CN",
	TRUE, ch, 0, verb, TO_CHAR);
    return;
  }

  if (victim->attackers >= 6) {
    act("$CYYou can't get close enough to them to $T!",
	TRUE, ch, 0, verb, TO_CHAR);
    return;
  }

  if (MOUNTED(ch)) {
    act("$CYYou can't $T someone while you are mounted.",
	TRUE, ch, 0, verb, TO_CHAR);
    return;
  }

  if (MOUNTED(victim)) {
    act("$CYYou can't $T someone while they are mounted.",TRUE,ch,0,verb,TO_CHAR);
    return;
  }

  if (IS_SET(victim->specials.mob_act, ACT_HUGE)) {
    act("$CYYou can't possibly $T such a huge being.",TRUE,ch,0,verb,TO_CHAR);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    act("$CYYou don't have enough mobility to try and $T.",TRUE,ch,0,verb,TO_CHAR);
    return;
  }

  SetVictFighting(ch, victim);
  SetCharFighting(ch, victim);

  mods = dex_app[GET_DEX(victim)].reaction - dex_app[GET_DEX(ch)].reaction;
  if (IS_IMMORTAL(victim) || ImpSaveSpell(victim, SAVING_PARA, 4) || percent() < 25) {
    GET_MOVE(ch) -= 10;
    if (GET_POS(victim) > POSITION_DEAD) {
      DamageMessages(ch, victim, 0, skill);
      if(percent() < ch->skills[SKILL_BALANCE].learned*9/10) {
	cprintf(ch, "You quickly regain your balance, and right yourself.\n");
      } else {
	ch->specials.position=POSITION_SITTING;
	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
	return;
      }
    }
  } else {
    GET_MOVE(ch) -= 10;
    if (GET_POS(victim) > POSITION_DEAD) {
      DamageMessages(ch, victim, 1, skill);
      victim->specials.position=POSITION_SITTING;
      if(skill == SKILL_BASH) {
	act("$n bashes $N!", 1, ch, 0, victim, TO_NOTVICT);
	act("$n bashes you to the ground!", 1, ch, 0, victim, TO_VICT);
	act("You bash $N!", 1,ch,0,victim,TO_CHAR);
      } else {
	act("$n trips $N!", 1, ch, 0, victim, TO_NOTVICT);
	act("$n trips you to the ground!", 1, ch, 0, victim, TO_VICT);
	act("You trip $N!", 1,ch,0,victim,TO_CHAR);
      }
      stun_opponent(ch, victim, skill, 4);
      WAIT_STATE(victim, PULSE_VIOLENCE*6);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_bash(struct char_data *ch, char *argument, int cmd)
{

  bash(ch,argument,cmd);
}

void do_taunt(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;

  if (!can_do(ch, SKILL_TAUNT))
    return;

  if (!(victim=set_target(ch, argument, SKILL_TAUNT)))
    return;

  if (ch->specials.fighting == victim) {
    send_to_char("How can you taunt someone you are trying to kill?\n\r",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char_formatted("$CwYou can't get close enough for them to hear you!\n\r", ch);
    return;
  }

  for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
	 (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if (GET_MANA(ch) < 20) {
    act("You don't have enough energy to taunt them",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  if (number(1, 101) > ch->skills[SKILL_TAUNT].learned) {
    GET_MANA(ch) -= 10;
    act("$CYYou try to rescue $N but fail utterly.",FALSE,ch,0,victim,TO_CHAR);
    act("$CY$n tries to rescue you but fails utterly.",TRUE,ch,0,victim,TO_VICT);
    act("$CY$n tries to rescue $N but fails utterly.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }

  GET_MANA(ch) -= 20;
  send_to_char_formatted("$CGBanzai!$Cw To the rescue...$CN\n\r", ch);
  act("$CGYou are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
  act("$Cg$n $CGheroically rescues $Cg$N.$CN", FALSE, ch, 0, victim, TO_NOTVICT);

  if (victim->specials.fighting == tmp_ch)
    stop_fighting(victim);
  if (tmp_ch->specials.fighting)
    stop_fighting(tmp_ch);
  if (ch->specials.fighting)
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(victim, 2*PULSE_VIOLENCE);
}

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;

  if (!can_do(ch, SKILL_RESCUE))
    return;

  if (!(victim=set_target(ch, argument, SKILL_RESCUE)))
    return;

  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char_formatted("$CwYou can't get close enough to them to rescue!\n\r", ch);
    return;
  }

  for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
	 (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    act("You don't have enough mobility to attempt a rescue.",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  if (number(1, 101) > ch->skills[SKILL_RESCUE].learned) {
    GET_MOVE(ch) -= 5;
    act("$CYYou try to rescue $N but fail utterly.",FALSE,ch,0,victim,TO_CHAR);
    act("$CY$n tries to rescue you but fails utterly.",TRUE,ch,0,victim,TO_VICT);
    act("$CY$n tries to rescue $N but fails utterly.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }

  GET_MOVE(ch) -= 10;
  send_to_char_formatted("$CGBanzai!$Cw To the rescue...$CN\n\r", ch);
  act("$CGYou are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
  act("$Cg$n $CGheroically rescues $Cg$N.$CN", FALSE, ch, 0, victim, TO_NOTVICT);

  if (victim->specials.fighting == tmp_ch)
    stop_fighting(victim);
  if (tmp_ch->specials.fighting)
    stop_fighting(tmp_ch);
  if (ch->specials.fighting)
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(victim, 2*PULSE_VIOLENCE);
}

void do_kick(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char dbuf[MAX_STRING_LENGTH];
  float nmdice;

  if (!can_do(ch, SKILL_KICK))
    return;

  if (!(victim=set_target(ch, argument, SKILL_KICK)))
    return;

  if (ch->attackers > 3) {
    send_to_char_formatted("$CYThere's no room to kick!\n\r",ch);
    return;
  }

  if (victim->attackers > 3) {
    send_to_char_formatted("$CYYou can't get close enough to them to kick!\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("You can't kick someone while you are mounted.\n\r", ch);
    return;
  }

  if (MOUNTED(victim)) {
    send_to_char("What are you going to do?  Jump up in the air, and kick them off their mount?\n\r",
		 ch);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    act("$CYYou don't have enough mobility to kick.",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  GET_MOVE(ch) -=10; /* remove the Move no matter if success or failure */

  if (victim->in_room != ch->in_room) {
    sprintf(dbuf, "do_kick found %s fighting mob that's gone",GET_NAME(ch));
    log_msg(dbuf);
    send_to_char("It seems they didn't wait around for you to kick them...\n\r", ch);
    return;
  }


  if (SkillChance(ch,victim,40,IMM_BLUNT,SPLMOD_DEX | SPLMOD_STR | SPLMOD_AC, SKILL_KICK)) {
    /* send messages first to make sure the victim is still around */
    act("$n KICKS $N!",1,ch,0,victim,TO_NOTVICT);
    act("$n KICKS you!",1,ch,0,victim,TO_VICT);
    act("You kick $N!",1,ch,0,victim,TO_CHAR);
    nmdice = 8*((float)GetMaxLevel(ch)/125)+2;
    if (!damage(ch, victim, dice((int)nmdice, GetMaxLevel(ch)), SKILL_KICK)) // Not dead
      if (!ch->specials.fighting)
	set_fighting(ch,victim);
  }
  else { /* doh MISSED! */
    act("You fail your kick.",1,ch,0,victim,TO_CHAR);
    act("$n just tried to kick you!!",1,ch,0,victim,TO_VICT);
    act("$n attempts to kick $N!",1,ch,0,victim,TO_NOTVICT);
    if (!ch->specials.fighting)
      set_fighting(ch,victim);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void push_message(struct char_data *ch, struct char_data *victim, int cmd, bool succeed)
{

  char buf[MAX_STRING_LENGTH];
  if (succeed) {
    sprintf(buf,"$n throws $mself against you, pushing you %s!",
	    dir_desc[cmd]);
    act(buf,FALSE,ch,0,victim,TO_VICT);
    sprintf(buf,"You throw yourself against $N and push $M %s!",
	    dir_desc[cmd]);
    act(buf,FALSE,ch,0,victim,TO_CHAR);
    sprintf(buf,"$Cb$n throws $mself against $N and pushes $M out of the room %s!",dir_desc[cmd]);
    act(buf,FALSE,ch,0,victim,TO_NOTVICT);
  } else {
    sprintf(buf,"$n throws $mself against you, but fails to accomplish anything.");
    act(buf,FALSE,ch,0,victim,TO_VICT);
    sprintf(buf,"You throw yourself against $N and fail to push $M anywhere.");
    act(buf,FALSE,ch,0,victim,TO_CHAR);
    sprintf(buf,"$n throws $mself against $N and bounces off.");
    act(buf,FALSE,ch,0,victim,TO_NOTVICT);
  }
}

void do_push(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  int dr, ch_str, ch_wt, vi_wt;

  if (!can_do(ch, SKILL_PUSH))
    return;

  half_chop(argument, name, dir);

  if (!(victim=set_target(ch, name, SKILL_PUSH)))
    return;

  if (ch->specials.fighting) {
    send_to_char("You're too busy fighting to try and push someone!\n\r", ch);
    return;
  }

  if (victim->attackers > 0) {
    send_to_char("You can't get close enough to them to push!\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("You can't push someone while you are mounted.\n\r", ch);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    act("You don't have enough mobility to push.",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  if (IS_SET(real_roomp(ch->in_room)->room_flags, NO_PUSH)) {
    send_to_char_formatted("$CBSome mystical force prevents you from pushing here.$CN\n\r", ch);
    return;
  }

  if (GET_POS(victim)!=POSITION_STANDING) {
    send_to_char("That person is not in any position to be pushed.\n\r", ch);
    return;
  }

  if (victim->specials.ridden_by) {
    send_to_char("That creature is being ridden, and is unpushable.\n\r", ch);
    return;
  }

  if (!*dir) {
    send_to_char("Which direction do you wish to push them in?\n\r", ch);
    return;
  }

  dr=search_block(dir, dirs, FALSE);
  if (dr==-1) {
    send_to_char("Push which way?\n\r", ch);
    return;
  }

  if (!CAN_GO(ch, dr) || EXIT(ch, dr)->to_room==NOWHERE ||
      IS_SET(EXIT(ch, dr)->exit_info, EX_CLOSED)) {
    send_to_char("There's no exit in that direction.\n\r", ch);
    return;
  }

  GET_MOVE(ch)-=10;

  if (!IS_PC(victim)) {
    if (GetMaxLevel(victim)>GetMaxLevel(ch) ||
	number(1,101)>ch->skills[SKILL_PUSH].learned) {
      send_to_char_formatted("$CrYou try to push, but just manage to annoy your victim!\n\r", ch);
      act("You become very angry as $n tries to push you around!",TRUE,ch,0,victim,TO_VICT);
      act("$Cr$n tries to push $N but only manages to anger $M!", TRUE,ch,0,victim,TO_ROOM);
      if (!victim->specials.fighting)
        set_fighting(victim, ch);
      return;
    }
  }

  ch_str = GET_STR(ch) + GET_ADD(ch)/25;
  ch_wt = GET_WEIGHT(ch);
  vi_wt = GET_WEIGHT(victim);
  if ((ch_wt/30)>ch_str)
    ch_str = ch_wt/30;

  if (number(1,30)<((15+ch_str-vi_wt/30))) {
    if (number(1,9)<4 && !IS_PC(victim)) {
      act("$n is tired of being pushed around and attacks!",TRUE,victim,0,0,TO_ROOM);
      send_to_char("You're tired of being pushed around and attack!\n\r", victim);
      if (!victim->specials.fighting)
        set_fighting(victim, ch);
      return;
    }
    push_message(ch,victim,dr,TRUE);
    MoveOne(victim,dr);
    return;
  }

  push_message(ch,victim,dr,FALSE);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_berserk(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];

  if (!can_do(ch, SKILL_BERSERK))
    return;

  if(IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("You are already berserk!\n\r", ch);
    return;
  }

  if (!ch->specials.fighting) {
    send_to_char("That would be a waste since you are not fighting.\n\r", ch);
    return;
  }

  if (ch->specials.fighting->in_room != ch->in_room) {
    send_to_char("Your opponent isn't here anymore...\n\r", ch);
    return;
  }

  if (GET_MOVE(ch)<10) {
    send_to_char("You are too low on movement to berserk.\n\r", ch);
    return;
  }

  if (number(1, 101) > ch->skills[SKILL_BERSERK].learned) {
    act("$n tries to berserk but fails.",TRUE,ch,0,0,TO_ROOM);
    send_to_char_formatted("$CYYou try to go berserk but fail.$CN\n\r", ch);
    charge_mana(ch, SKILL_BERSERK, FALSE);
    GET_MOVE(ch) -= 3;
  } else {
    charge_mana(ch, SKILL_BERSERK, TRUE);
    GET_MOVE(ch)-=6;
    SET_BIT(AFF_FLAGS(ch), AFF_BERSERK);
    send_to_char_formatted("$CwYou go $CRBERSERK!$CN\n\r", ch);
    act("$Cg$n $Cwgoes $CRBERSERK!$CN\n\r",TRUE,ch,0,0,TO_ROOM);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_retreat(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *fighting;
  char direction[MAX_INPUT_LENGTH];
  int dir;
  int did, was_room;

  if (!can_do(ch, SKILL_RETREAT))
    return;

  if (!(fighting=ch->specials.fighting)) {
    send_to_char("Why would you need to retreat? You're not fighting.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char_formatted("$CYRetreat like a coward?! $CRNever!\n\r", ch);
    return;
  }

  if (ch->specials.binded_by || affected_by_spell(ch, SPELL_WEB)) {
    send_to_char("You can't retreat, you're all tied up!\n\r", ch);
    return;
  }

  only_argument(argument, direction);
  if (!direction) {
    send_to_char("Retreat yes, but which way?\n\r", ch);
    return;
  }

  if (number(1, 101) > ch->skills[SKILL_RETREAT].learned) {
    act("You $CRFAIL$CN to retreat.",FALSE,ch,0,fighting,TO_CHAR);
    act("You block $n's retreat.",FALSE,ch,0,fighting,TO_VICT);
    act("$n's retreat is blocked by $N!",TRUE,ch,0,fighting,TO_ROOM);
    WAIT_STATE(ch, 1*PULSE_VIOLENCE);
    return;
  }

  if((dir = search_block(direction, dirs, 0)) == -1)
    {
      send_to_char("Retreat yes, but which way?\n\r", ch);
      return;
    }

  if (!ValidMove(ch, dir))
    return;

  act("$CyYou make a strategic retreat...",FALSE,ch,0,0,TO_CHAR);
  act("$Cy$n makes a strategic retreat...",TRUE,ch,0,0,TO_ROOM);

  was_room = ch->in_room;

  if (RIDDEN(ch))
    did = MoveOne(RIDDEN(ch), dir);
  else
    did = MoveOne(ch, dir);

  if(did)
    {
      stop_opponents(ch, was_room);
      stop_fighting(ch);
    }

  WAIT_STATE(ch, 3*PULSE_VIOLENCE);
}

void do_flail (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  int dam,w_type;
  struct obj_data *wielded;

  if (!can_do(ch, SKILL_FLAIL))
    return;

  if (!ch->equipment[WIELD]) {
    send_to_char("You swing around with your fingers extended...tickle, tickle\n\r", ch);
    return;
  }

  w_type = GetWeaponType (ch,&wielded);
  dam = 0;

  if(!wielded) {
    cprintf(ch, "You must have a proper weapon equipped to flail.\n\r");
    return;
  }

  if (number(1,101)>ch->skills[SKILL_FLAIL].learned)
    dam=0;
  else  {
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
    dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
    if(IS_AFFECTED(ch, AFF_BERSERK) && ch->skills)
      dam += (int)(dam * ((GetMaxLevel(ch)/100.0) + (1.0-(GET_WIS(ch)/25.0))));
    dam += number(1, 101) * 5;
    dam = MAX(1, dam);
  }

  act("You stretch out your arms, your weapon fully extended and start swinging in a circle!",
      TRUE,ch,0,0,TO_CHAR);
  act("$n stretches out $s arms and start swinging around like a madman!",TRUE,ch,0,0,TO_ROOM);

  GET_MOVE(ch) -= 40;



  EACH_CHARACTER(iter, vict)
    {
      if (ch->in_room==vict->in_room)
	{
	  if(can_hurt(ch,vict))
	    {
	      act("$n whirls around in a circle, hitting everyone with $s weapon!",
		  TRUE,ch,0,0,TO_ROOM);
	      act("Everyone around you gasp in pain from your insane flail!",
		  TRUE, ch, 0, 0, TO_CHAR);
	      damage(ch, vict, dam, w_type);
	    }
	}
      else if(real_roomp(ch->in_room)->zone == real_roomp(vict->in_room)->zone)
	act("Your a bit dizzy now.",FALSE,vict,0,0,TO_CHAR);
    }
  END_AITER(iter);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

/*** thief skills ***/

void do_trip(struct char_data *ch, char *argument, int cmd)
{
  bash(ch,argument,cmd);
}

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  int mods;

  if (cmd==154) {
    if (!can_do(ch, SKILL_BACKSTAB))
      return;
  } else {
    if (!can_do(ch, SKILL_CIRCLE))
      return;
  }

  if (!(victim=set_target(ch, argument, SKILL_BACKSTAB)))
    return;

  if (cmd != 376) {
    if (ch->attackers) {
      send_to_char("There's no way to reach that back while you're fighting!\n\r", ch);
      return;
    }
    if (ch->specials.fighting) {
      send_to_char("You're too busy to backstab.\n\r", ch);
      return;
    }
    if (GET_MOVE(ch)<10) {
      send_to_char("You are too low on movement to sneak up and backstab.\n\r", ch);
      return;
    }
  } else {
    if (victim->specials.fighting == ch) {
      act("$E's paying far too much attention to you.",FALSE,ch,0,victim,TO_CHAR);
      return;
    }
    if (!ch->specials.fighting) {
      send_to_char("You would be better off to backstab first.\n\r", ch);
      return;
    }
    if (GET_MOVE(ch)<15) {
      send_to_char("You are too low on movement to circle around for another backstab.\n\r", ch);
      return;
    }
  }

  if (IS_SET(victim->specials.mob_act, ACT_HUGE)) {
    act("$N is MUCH too large to backstab",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You poke him in the ribs with your finger.\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("You couldn't possibly sneak up on someone while riding this beast!\n\r", ch);
    return;
  }

  if (MOUNTED(victim)) {
    send_to_char("You can't possibly jump over their mount and backstab!\n\r", ch);
    return;
  }

  if (victim->attackers >= 4) {
    send_to_char("You can't get close enough to them to backstab!\n\r", ch);
    return;
  }

  if (ch->equipment[WIELD]->obj_flags.value[3] != 5 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 6 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 7 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 8 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 9 ) {
    send_to_char("Only piercing or stabbing weapons can be used to backstab.\n\r", ch);
    return;
  }

  if (cmd == 376) {
    act("$n circles around behind $N for a backstab...",
	TRUE,ch,0,victim,TO_NOTVICT);
    act("You circle around behind $N for a backstab...",
	TRUE,ch,0,victim,TO_CHAR);
    send_to_char("You notice some movement out of the corner of your eye.\n\r", victim);
  }

  mods = 0;

  if (victim->specials.fighting)
    mods += 4;

  if (!CAN_SEE(victim, ch))
    mods -= 4;

  if (ch->specials.fighting)
    stop_fighting(ch);
  set_fighting(ch, victim);

  if ((cmd != 376) ||
      !NewSkillSave(ch, victim, SKILL_CIRCLE,2+victim->attackers, 0))
    {
      if (ch->skills && ch->skills[SKILL_BACKSTAB].learned &&
	  (!AWAKE(victim) || !NewSkillSave(ch,victim,SKILL_BACKSTAB,mods,0))) {
	GET_MOVE(ch) -= 10;
	hit(ch,victim,SKILL_BACKSTAB);
      } else {
	GET_MOVE(ch) -= 5;
	hit(ch, victim, TYPE_UNDEFINED);
      }
    }

  /* check to make sure we didn't already kill them... */
  if(ch->specials.fighting == victim)
    {
      if (!victim->specials.binded_by &&
	  (number(1,99) <= (100/MAX(1, victim->attackers))))
	{
	  act("$N is infuriated by your attack and turns to face you!",
	      FALSE, ch, 0, victim, TO_CHAR);
	  act("$N is infuriated and turns to face $n!",
	      TRUE, ch, 0, victim, TO_NOTVICT);
	  act("You are infuriated by $N's attack and turn to face them!",
	      FALSE, victim, 0, ch, TO_CHAR);
	  if (victim->specials.fighting)
	    stop_fighting(victim);
	  set_fighting(victim, ch);
	}

      AddHated(victim, ch);
    }

  WAIT_STATE(ch, PULSE_VIOLENCE);
}


void do_throw(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *missile;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
  int rng, dr;
  struct char_data *targ;

  if (!can_do(ch, SKILL_THROW))
    return;

  half_chop(argument, arg1, arg2);
  strcpy(argument, arg2);
  half_chop(argument, arg2, arg3);

  if(is_abbrev(arg3, "north"))
    dr = 0;
  else if (is_abbrev(arg3, "east"))
    dr = 1;
  else if (is_abbrev(arg3, "south"))
    dr = 2;
  else if (is_abbrev(arg3, "west"))
    dr = 3;
  else if (is_abbrev(arg3, "up"))
    dr = 4;
  else if (is_abbrev(arg3, "down"))
    dr = 5;
  else
    dr = 6;

  if ((!*arg1)||(!*arg2)) {
    send_to_char("Throw what at whom?\n\r", ch);
    return;
  }

  if(!(missile=get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    send_to_char("You can't throw what you don't have.\n\r", ch);
    return;
  }

  if(dr == 6) // means they didn't pick a direction for the throw
    {
      if(!(targ=get_char_linear(ch, arg2, &rng, &dr))) {
	send_to_char("There's nobody around by that name.\n\r", ch);
	return;
      }
    }
  else {
    if(!(targ=get_char_dir(ch, arg2, &rng, dr)))
      {
	send_to_char("There's noboody around by that name.\n\r", ch);
	return;
      }
  }

  if(rng!=1) {
    if (HasClass(ch, CLASS_WARRIOR)) {
      send_to_char("You can only throw into adjacent rooms.\n\r", ch);
      return;
    }
  }

  if(!IS_SET(missile->obj_flags.wear_flags,ITEM_THROW)) {
    send_to_char("You cant throw that.\n\r",ch);
    return;
  }

  if(check_peaceful(targ,"")) {
    send_to_char("You can't throw into a peaceful room.\n\r", ch);
    return;
  }

  obj_from_char(missile);
  if (number(1,101) < ch->skills[SKILL_THROW].learned) {
    act("You throw $p!",FALSE,ch,missile,0,TO_CHAR);
    act("$n throws $p!",TRUE,ch,missile,0,TO_ROOM);
    throw_weapon(missile,dr,targ,ch, SKILL_THROW);
  } else {
    act("You try to throw $p but clumsily drop it.",
	FALSE, ch, missile, 0, TO_CHAR);
    act("$n tries to throw $p but clumsily drops it.",
	TRUE, ch, missile, 0, TO_ROOM);
    obj_to_room(missile, ch->in_room);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_pick(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data      *rp;

  if (!can_do(ch, SKILL_PICK_LOCK))
    return;

  argument_interpreter(argument, type, dir);
  percent=number(1,101); /* 101% is a complete failure */

  if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
    charge_mana(ch, SKILL_PICK_LOCK, FALSE);
    send_to_char("You failed to pick the lock.\n\r", ch);
    return;
  }

  if (!*type) {
    send_to_char("Pick what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                          ch, &victim, &obj)) {

    /* this is an object */
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oho! This thing is NOT locked!\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
      send_to_char("It resists your attempts at picking it.\n\r", ch);
    else
      {
        REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
        send_to_char("*Click*\n\r", ch);
        act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You realize that the door is already open.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_PICKPROOF))
      send_to_char("You seem to be unable to pick this lock.\n\r", ch);
    else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword)
        act("$n skillfully picks the lock of the $F.", 0, ch, 0,
            exitp->keyword, TO_ROOM);
      else
        act("$n picks the lock.",TRUE,ch,0,0,TO_ROOM);
      charge_mana(ch, SKILL_PICK_LOCK, TRUE);
      send_to_char("The lock quickly yields to your skills.\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if (rp &&
          (back = rp->dir_option[rev_dir[door]]) &&
          back->to_room == ch->in_room )
        REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

void FailSteal(struct char_data* ch, struct char_data* victim, int percent,
               char* mVict, char* mOthers)
{
  act("Oops..", FALSE, ch,0,0,TO_CHAR);

  if(IS_PC(victim))
    SET_BIT(ch->specials.flags, PLR_THIEF);

  switch(GET_POS(victim))
    {
    case POSITION_STUNNED:
      if(percent > (ch->skills[SKILL_STEAL].learned + 40))
        {
	  update_pos(victim);
	  if(GET_POS(victim) > POSITION_STUNNED)
	    act("In your clumsiness, you arouse $N!",
		TRUE, ch, 0, victim, TO_CHAR);
        }
      break;

    case POSITION_SLEEPING:
      if(percent > (ch->skills[SKILL_STEAL].learned + 20))
        {
	  if(IS_AFFECTED(victim, SPELL_SLEEP))
	    affect_from_char(victim, SPELL_SLEEP);
	  GET_POS(victim) = POSITION_SITTING;
	  act("In your clumsiness, you awaken $N!",
	      TRUE, ch, 0, victim, TO_CHAR);
        }

      break;
    }

  act(mOthers, FALSE, ch, 0, victim, TO_NOTVICT);

  if(AWAKE(victim))
    {
      act(mVict, FALSE, ch, 0, victim, TO_VICT);

      if(IS_SET(ch->specials.mob_act, ACT_POLYSELF))
        {
	  struct char_data* o = real_character(ch);

	  if(GET_NAME(o) && IS_PC(victim))
            {
	      char buf[256];
	      sprintf(buf,"You recognize that mobile as %s!\n\r",
		      GET_NAME(o));
	      send_to_char(buf,victim);
            }
        }
      if (IS_SET(victim->specials.mob_act, ACT_NICE_THIEF)) {
	char buf[256];
	sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
	do_shout(victim, buf, 0);
	send_to_char("Don't you ever do that again!\n\r", ch);
      } else if(IS_NPC(victim)) {
	hit(victim, ch, TYPE_UNDEFINED);
      }
    }
}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char obj_name[240];
  char buf[240];
  int percent = 100;
  int gold, eq_pos = -1;

  if (!can_do(ch, SKILL_STEAL) && !IS_IMMORTAL(ch))
    return;

  argument = one_argument(argument, obj_name);

  if (!(victim=set_target(ch, argument, SKILL_STEAL)))
    return;

  if(MOUNTED(ch)) {
    send_to_char("You couldn't steal candy from a baby while mounted on this beast.\n\r", ch);
    return;
  }

  if (GET_MOVE(ch)<10) {
    send_to_char("You are too low on movement to try and steal.\n\r", ch);
    return;
  }


  if ((GetMaxLevel(ch) < 5) && (IS_PC(victim))) {
    send_to_char("Due to misuse of steal, you can't steal from other players\n\r",ch);
    send_to_char("unless you are at least 5th level. \n\r", ch);
    return;
  }

  if (IS_SET(RM_FLAGS(ch->in_room), NOSTEAL)) {
    send_to_char("You are unable to steal in this room.\n\r", ch);
    return;
  }

  if ((!victim->desc) && (!IS_NPC(victim)))
    return;

  /* 101% is a complete failure */
  percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if(AWAKE(victim))
    percent += 3*(MIN(GetMaxLevel(victim), 175) - GetMaxLevel(ch));

  if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    percent = 101;          /* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {

    if (!(obj = get_obj_in_list_vis(ch, obj_name, victim->carrying)))
      {
	for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	  if (victim->equipment[eq_pos] &&
	      (isname(obj_name, OBJ_NAME(victim->equipment[eq_pos]))) &&
	      CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
	    obj = victim->equipment[eq_pos];
	    break;
	  }

	if (!obj) {
	  act("$E does not have that item.",FALSE,ch,0,victim,TO_CHAR);
	  return;
	} else {
	  switch(GET_POS(victim)) {
	  case POSITION_STUNNED :
	    {
	      send_to_char("Way to hit them while they are down!\n\r", ch);
	      percent -= 2;
	      break;
	    }
	  case POSITION_SLEEPING :
	    {
	      send_to_char("Better not wake them!\r\n", ch);
	      percent -= 1;
	      break;
	    }
	  case POSITION_RESTING :
	  case POSITION_SITTING :
	    {
	      send_to_char("Watch it! They aren't asleep.\r\n", ch);
	      percent += 4;
	      break;
	    }
	  case POSITION_FIGHTING :
	  case POSITION_MOUNTED  :
	    {
	      send_to_char("No way can you steal anything now.\r\n", ch);
	      percent += 200;
	      break;
	    }
	  case POSITION_STANDING :
	    {
	      send_to_char("If they see you, surely you are dead!\r\n", ch);
	      if(CAN_SEE(victim, ch))
		percent += 20;
	      else
		percent += 10;
	      break;
	    }
	  default  :
	    {
	      send_to_char("Looting the dead, what honor you have!\r\n", ch);
	      percent -= 50;
	      break;
	    }
	  }
	}
      }

    if (IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
      send_to_char("You can't steal it, it must be CURSED!\n\r", ch);
      return;
    }

    percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

    if(percent > (ch->skills[SKILL_STEAL].learned / 2) && !IS_IMMORTAL(ch))
      {
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
	FailSteal(ch, victim, percent,
		  "$n tried to steal something from you!",
		  "$n tried to steal something from $N!");
      } else {                /* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) &&
	    ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)))
	  {
	    if(eq_pos > -1) // Means they are wearing it
	      {
		switch (eq_pos) {
		case WEAR_LIGHT :
		case WEAR_FINGER_R :
		case WEAR_FINGER_L :
		case WEAR_WAISTE :
		case WEAR_WRIST_R :
		case WEAR_WRIST_L :
		case WEAR_NECK_1 :
		case WEAR_NECK_2 :
		case WEAR_ARMS :
		case HOLD :
		  obj = unequip_char(victim, eq_pos);
		  obj_to_char(obj, ch);
		  break;
		default :
		  if (IS_IMMORTAL(ch)) {
		    obj = unequip_char(victim, eq_pos);
		    obj_to_char(obj, ch);
		  } else {
		    send_to_char("You can't steal an item from there!\n\r",ch);
		  }
		  break;
		}
	      } else {
		obj_from_char(obj);
		obj_to_char(obj, ch);
	      }

	    if (IS_PC(victim))  { /* log_msgsteals from players */
	      sprintf(buf,"PLAYER STEAL: %s stole %s from %s.",
		      ss_data(ch->player.name),
		      ss_data(obj->name),
		      ss_data(victim->player.name));
	      log_msg(buf);
	    }
	    send_to_char("Got it!\n\r", ch);
#if NODUPLICATES
	    do_save(ch, "", 0);
	    do_save(victim, "", 0);
#endif
	  } else
	    send_to_char("You cannot carry that much.\n\r", ch);
      }
  } else {                    /* Steal some coins */

    if((percent > (ch->skills[SKILL_STEAL].learned / 2)) && !IS_IMMORTAL(ch))
      {
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
	FailSteal(ch, victim, percent,
		  "You discover that $n has $s hands in your wallet.",
		  "$n tries to steal gold from $N.");
      }
    else
      {
	/* Steal some gold coins */
	gold = (int) ((GET_GOLD(victim)*number(1,50))/100);
	if (gold > 0) {
	  GET_GOLD(ch) += gold;
	  GET_GOLD(victim) -= gold;
	  sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
	  send_to_char(buf, ch);
	} else {
	  send_to_char("You couldn't get any gold...\n\r", ch);
	}
      }
  }
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
  if (!can_do(ch, SKILL_HIDE))
    return;

  if(MOUNTED(ch)) {
    send_to_char("You can't possibly hide while riding this beast.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_HIDE)) {
    REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
    act("You quietly step out the shadows.",TRUE,ch,0,0,TO_CHAR);
    act("$n quietly steps out the shadows.",TRUE,ch,0,0,TO_ROOM);
    return;
  }

  if ((number(1,101) < ch->skills[SKILL_HIDE].learned) || IS_PURE_CLASS(ch)) {
    act("You hide in the shadows.",TRUE,ch,0,0,TO_CHAR);
    SET_BIT(AFF_FLAGS(ch), AFF_HIDE);
  }
  else {
    act("You think you are hidden in the shadows.",TRUE,ch,0,0,TO_CHAR);
    act("$n tries to slip into the shadows but trips.",TRUE,ch,0,0,TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }

  /* This line defeats the hide skill */
  /*  act("$n steps into the shadows and hides.",TRUE,ch,0,0,TO_ROOM); */
}

void do_sneak(struct char_data *ch, char *arg, int cmd)
{
  if (!can_do(ch, SKILL_SNEAK))
    return;

  if(MOUNTED(ch)) {
    send_to_char("You can't possibly sneak while riding this beast?\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    REMOVE_BIT(AFF_FLAGS(ch), AFF_SNEAK);
    act("You stop sneaking about.",TRUE,ch,0,0,TO_CHAR);
    act("$n stops sneaking about.",TRUE,ch,0,0,TO_ROOM);
    return;
  }

  if ((number(1,101) < ch->skills[SKILL_SNEAK].learned) || IS_PURE_CLASS(ch)) {
    act("You begin to sneak about.",TRUE,ch,0,0,TO_CHAR);
    SET_BIT(AFF_FLAGS(ch), AFF_SNEAK);
  }
  else {
    act("You think you are being sneaky.",TRUE,ch,0,0,TO_CHAR);
    act("$n attempts to be sneaky.",TRUE,ch,0,0,TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }

  /* This line defeats the sneak skill */
  /*  act("$n starts being sneaky.",TRUE,ch,0,0,TO_ROOM); */

}

int search_object(struct char_data* ch, struct obj_data* obj, int chance);
void search_room(struct char_data* ch, int chance);
int search_exit(struct char_data* ch, struct room_data* rp,
                int dir, int chance);

void do_search(struct char_data *ch, char *arg, int cmd)
{
  char                item[256];
  struct obj_data*    obj;
  struct room_data*   rp;
  int                 chance;
  int                 dir;

  if (!can_do(ch, SKILL_SEARCH))
    return;

  only_argument(arg, item);

  if(IS_IMMORTAL(ch))
    chance = 200;
  else
    {
      chance = ((50 + GetMaxLevel(ch)) *
		ch->skills[SKILL_SEARCH].learned / 100);
      chance += dex_app_skill[GET_DEX(ch)].traps;
    }

  if(!*item)
    search_room(ch, chance);
  else if((dir = search_block(item, dirs, 0)) >= 0)
    {
      if((rp = real_roomp(ch->in_room)) == NULL)
	send_to_char("Where are you anyway?\n\r", ch);
      else if(!search_exit(ch, rp, dir, chance))
	send_to_char("You find nothing.\n\r", ch);
    }
  else if((obj = get_obj_vis(ch, item)) == NULL)
    send_to_char("You can't find it!\n\r", ch);
  else if(!search_object(ch, obj, chance))
    send_to_char("It appears to be safe!\n\r", ch);

  charge_mana(ch, SKILL_SEARCH, TRUE);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

int search_object(struct char_data* ch, struct obj_data* obj, int chance)
{
  if(!CAN_SEE_OBJ(ch, obj))
    return 0;

  if((obj->obj_flags.type_flag == ITEM_TRAP) &&
     (number(1, 100) < chance))
    {
      act("$p is trapped!", TRUE, ch, obj, NULL, TO_CHAR);
      return 1;
    }

  return 0;
}

void search_room(struct char_data* ch, int chance)
{
  struct room_data*   rp;
  struct obj_data*    obj;
  int                 dir;
  int                 found = 0;

  if((rp = real_roomp(ch->in_room)) == NULL)
    {
      send_to_char("Where are you anyway?\n\r", ch);
      return;
    }

  if((rp->tele_targ > 0) &&
     (rp->tele_time > 0) &&
     (number(1, 100) < chance))
    send_to_char("This is a teleport room!  Better move quick.\n\r", ch);

  chance = chance / 2;        /* reduce chance if not search a specific */

  for(dir = 0 ; dir < 6 ; ++dir)
    if(search_exit(ch, rp, dir, chance))
      found++;

  for(obj = rp->contents ; obj ; obj = obj->next_content)
    if(search_object(ch, obj, chance))
      found++;

  if(!found)
    send_to_char("You find nothing!\n\r", ch);
}

int search_exit(struct char_data* ch, struct room_data* rp,
                int dir, int chance)
{
  char                        buf[256];
  int                         found = 0;
  struct room_direction_data* exit;

  if((exit = rp->dir_option[dir]) != NULL)
    {
      struct room_data*       dest;
      int                     seen = 1;

      /* is the exit secret? */
      if((exit->exit_info & EX_SECRET) ||
	 (exit->keyword && isname("secret", exit->keyword)))
        {
	  if((seen = (number(1,100) < chance)))
            {
	      sprintf(buf, "You find a secret exit to the %s.\n\r",
		      dirs[dir]);
	      send_to_char(buf, ch);
	      found++;
            }
        }

      /* what's on the other side? */
      if(seen && !(exit->exit_info & EX_CLOSED))
        {
	  if((dest = real_roomp(exit->to_room)) == NULL)
	    send_to_char("It leads to the void!\n\r", ch);
	  else if((dest->room_flags & DEATH) &&
		  (number(1, 100) < chance))
            {
	      sprintf(buf, "You find a fatal trap to the %s.\n\r",
		      dirs[dir]);
	      send_to_char(buf, ch);
	      found++;
            }
	  else if((dest->tele_targ > 0) &&
		  (dest->tele_time > 0) &&
		  (number(1, 100) < chance))
            {
	      sprintf(buf, "You find a teleporter to the %s.\n\r",
		      dirs[dir]);
	      send_to_char(buf, ch);
	      found++;
            }
        }
    }

  return found;
}

void do_palm (struct char_data *ch, char *argument, int cmd)
{
  struct room_data *rp;
  struct char_data *to;
  struct obj_data *object;
  int success, roll, success_tmp;
  char buf[MAX_STRING_LENGTH];
  bool seesit;

  if (!can_do(ch, SKILL_PALM))
    return;

  if(!*argument)
    {
      send_to_char("Yes, but palm what?\n\r",ch);
      return;
    }

  rp = real_roomp(ch->in_room);
  object=get_obj_in_list_vis(ch, argument, rp->contents);

  if(!object)
    {
      send_to_char("You do not see that item here.\n\r",ch);
      return;
    }

  /*  if (object->in_room == NOWHERE) {
      object->in_room = ch->in_room;
      sprintf(buf, "palm: Couldn't find object %s in %d",
      OBJ_SHORT(object), ch->in_room);
      log_msg(buf);
      } */

  if (!try_to_get_object(ch,object))
    return;

  obj_from_room(object);
  obj_to_char(object,ch);

  success = (ch->skills[SKILL_PALM].learned)+ ((GET_DEX(ch)-10)*5) +
    (GET_LEVEL(ch,THIEF_LEVEL_IND));
  success_tmp = success; // store in tmp var so success dont change for each player in loop

  roll = number(1,101);

  sprintf(buf, "You grab %s from the ground as secretly as possible.\n\r",OBJ_SHORT(object));
  send_to_char(buf, ch);

  for (to = rp->people ; to ; to=to->next_in_room)
    {
      seesit = FALSE;
      success = success_tmp; // reset success for each char in room

      if (to != ch)
	{
	  success -= (GetMaxLevel(to)*2 + ((GET_WIS(to)-10)*3));
	  if ( (IS_IMMORTAL(to) || IS_IMMORTAL(ch)))
	    {
	      if (GetMaxLevel(ch) > GetMaxLevel(to))
		seesit=FALSE;
	      else
		seesit=TRUE;
	    }

	  if ( (roll > MIN(ch->skills[SKILL_PALM].learned,success)) )
	       // || (IS_AFFECTED(ch,AFF_GREAT_SIGHT)) )
	    seesit=TRUE;

	  if (seesit)
	    act_to_char(to, "You notice $n trying to secretly grab $p from the ground.\n\r", FALSE, ch, object, 0);
	}
    }
}


void do_divert (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  struct char_data *tmp_ch;

  if(!can_do(ch, SKILL_DIVERT))
    return;

  if(!ch->specials.fighting) {
    send_to_char("You have to be fighting someone to divert them!",ch);
    return;
  }

  if (!(vict=set_target(ch, arg, SKILL_DIVERT)))
    return;

  if(IS_PC(vict)) {
    send_to_char_formatted("$CgHey, no diverting to players!!!\n\r",ch);
    return;
  }

  tmp_ch = ch->specials.fighting;

  if (!tmp_ch)
    return;

  if(tmp_ch == vict) {
    send_to_char("You can't make someone fight themselves!",ch);
    return;
  }

  if(MOUNTED(vict)) {
    send_to_char_formatted("$CYThey can't fight someone on a horse!\n\r", ch);
    return;
  }

  if(ch->attackers > 5) {
    send_to_char_formatted("$CYThere are too many people watching you!\n\r", ch);
    return;
  }

  if(vict->attackers > 5){
    send_to_char_formatted("$CYThey can't fight anyone else!\n\r", ch);
    return;
  }

  //if (number(1, 101) > ch->skills[SKILL_DIVERT].learned) {
  if (SkillChance(ch,tmp_ch,10,0,SPLMOD_DEX | SPLMOD_INT | SPLMOD_WIS, SKILL_DIVERT))
    {
      GET_MOVE(ch) -= 15;
      send_to_char_formatted("$CGYaa!$Cw they fell for it!$CN\n\r", ch);
      act("$CG$n have turned to $N!", FALSE, tmp_ch, 0, vict, TO_CHAR);
      act("$Cg$n $CGdiverts the fight to $Cg$N.$CN", FALSE, ch, 0, vict, TO_NOTVICT);
    }
  else {
    GET_MOVE(ch) -= 10;
    act("$CYYou try to divert the fight to $N but fail utterly.",FALSE,ch,0,vict,TO_CHAR);
    act("$CY$n tries to divert the fight over your way, but failed!.",TRUE,ch,0,vict,TO_VICT);
    act("$CY$n tries to divert the fight to $N but fails utterly.",TRUE,ch,0,vict,TO_ROOM);
    WAIT_STATE(ch, 2*PULSE_VIOLENCE);
    return;
  }

  if (tmp_ch->specials.fighting == ch)
    stop_fighting(tmp_ch);

  if(ch->specials.fighting)
    stop_fighting(ch);

  set_fighting(vict, tmp_ch);
  set_fighting(tmp_ch, vict);

  WAIT_STATE(ch, 2*PULSE_VIOLENCE);
}

void do_gouge( struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  int chance, level;
  level = GetMaxLevel(ch);
  chance = 20;

  if(!can_do(ch, SKILL_GOUGE))
    return;

  if (!(vict=set_target(ch, arg, SKILL_GOUGE)))
    return;

  if (IS_AFFECTED(vict, AFF_BLIND)) {
    send_to_char("They don't have any eyes to gouge out!",ch);
    return;
  }

  if(MOUNTED(ch)) {
    send_to_char("You will have to get down first.",ch);
    return;
  }

  if(MOUNTED(vict)) {
    send_to_char("You can't reach them while they are on that horse!",ch);
    return;
  }

  if (ch->equipment[WIELD] && 
      ch->equipment[WIELD]->obj_flags.value[3] != 5 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 6 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 7 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 8 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 9 ) {
    send_to_char("Only piercing or stabbing weapons can be used to gouge.\n\r", ch);
    return;
  }

  if(vict->attackers > 5)
    chance += 10;
  if(affected_by_spell(ch, SKILL_SNEAK))
    chance += 10;

  if (SkillChance(ch,vict,chance,10,SPLMOD_DEX | SPLMOD_STR | SPLMOD_CON, SKILL_GOUGE)) {
    GET_MOVE(ch) -= 10;
    act("$CYYou tried to gouge $N's eyes out, but failed!", FALSE, ch, 0, vict, TO_CHAR);
    act("$CY$n tried to gouge your eyes out!",TRUE,ch,0,vict,TO_VICT);
    act("$CY$n tried to gouge $N eyes out!", TRUE, ch, 0, vict, TO_ROOM);

  }
  else {
    act("$CYYou gouge $N's eyes!",FALSE,ch,0,vict,TO_CHAR);
    act("$CY$n gouges your eyes out! You can't see!",TRUE,ch,0,vict,TO_VICT);
    act("$CY$n gouged $N's eyes out!",TRUE,ch,0,vict,TO_ROOM);
    if(vict->skills && vict->skills[SKILL_BLIND_FIGHTING].learned)
      {
	chance = (vict->skills[SKILL_BLIND_FIGHTING].learned);
	if(number(1, 101) < chance)
	  {
	    MakeAffect(ch, vict, SPELL_TYPE_SPELL,
		       SPELL_BLINDNESS, -MAX((int)(level/15), 1), APPLY_HITNDAM, AFF_BLIND,
		       10, 1, FALSE, FALSE, FALSE, NULL);
	    send_to_char_formatted("$CgThough blinded, they still fight well!\n\r",ch);
	  }
	else {
	  MakeAffect(ch, vict, SPELL_TYPE_SPELL,
		     SPELL_BLINDNESS, -MAX((int)(level/5), 1), APPLY_HITNDAM, AFF_BLIND,
		     10, 1, FALSE, FALSE, FALSE, NULL);
	}
      }
    else {
      MakeAffect(ch, vict, SPELL_TYPE_SPELL,
		 SPELL_BLINDNESS, -MAX((int)(level/5), 1), APPLY_HITNDAM, AFF_BLIND,
		 10, 1, FALSE, FALSE, FALSE, NULL);
    }
  }

  if (!ch->specials.fighting)
    set_fighting(ch, vict);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}



/*** druid skills ***/

void do_brew (struct char_data *ch, char *argument, int cmd)
{
  static struct brew_stuff
  {
    int index;
    char *name;
    int reagent;
    int level;
    char* desc;
  } brews[] =
    {
      //index  name               reagent    level    desc
      { 1, "healing",	         8718,	     1,	"Druidic potion of healing" },
      { 2, "mana",               8719,      10,     "Druidic potion of mana" },
      { 3, "sanctuary",          8729,      12,     "A druidic sanctuary" },
      { 4, "fireshield",         8731,      20,     "A druidic fire shield" },
      { 5, "coldshield",         8735,      30,     "A druidic cold shield" },
      { 6, "elecshield",         8738,      40,     "A druidic electric shield" },
      { 7, "energyshield",       8720,      45,     "A druidic energy shield" },
      { 8, "poisonshield",       8725,      50,     "A druidic poison shield" },
      { 9,  "manashield",        8726,      60,     "A druidic mana shield" },
      { 10, "moveshield",        8727,      70,     "A druidic move shield" },
      { 11, "vampshield",        8716,      80,     "A druidic vampyric shield" },
      { 12, "sight",	         8717,       1,     "The sight of druids" },
      { 13, "bless",             8724,       1,     "A druids blessings" },
      { 14, "protection",        8721,       1,     "Protection of druids" },
      { 15, "invisibility",      8733,       1,     "A druids invisibility" },
      { 16, "hero",              8732,      20,     "Heroes feast on a bottle" },
      { 17, "fly",               8722,       1,     "Druidic potion of fly" },
      { 18, "waterbreath",       8737,       1,     "Druidic potion of waterbreath" },
      { 19, "gprotect",          8723,      20,     "Group protection of druids" },
      { 20, "gsight",            8730,      20,     "Group sight of druids" },
      { 21, "gfly",              8728,      20,     "Druidic potion of group fly" },
      { 22, "ginvisibility",     8736,      20,     "A druids group invisibility" },
      { 23, "gwaterbreath",      8737,      20,     "Druidic potion of group waterbreath" },
      { 24, "firebreath",        8739,     120,     "A druidic potion of dragonsbreath" },
      { 25, "frostbreath",       8740,      95,     "A druidic potion of dragonsbreath" },
      { 26, "acidbreath",        8741,     115,     "A druidic potion of dragonsbreath" },
      { 27, "poisongasbreath",   8742,      85,     "A druidic potion bof dragonsbreath" },
      { 28, "lightningbreath",   8743,     105,     "A druidic potion of dragonsbreath" },
      { 0,			0,	 0,      0 }
    };

  int compnum;
  struct obj_data *obj, *reagent;
  char name[MAX_INPUT_LENGTH], buf[500];
  struct brew_stuff* ptr;
  int druid_modifier = 4;

  if (!can_do(ch, SKILL_BREW))
    return;

  only_argument(argument,name);

  if(is_abbrev(name, "help")) {
    send_to_char("The following brews are available at listed levels:\n\r",ch);
    for(ptr = brews ; ptr->name ; ptr++) {
      sprintf(buf, "%2d %8s  %s\r\n", ptr->level, ptr->name, ptr->desc);
      send_to_char(buf, ch);
    }
    return;
  }

  // Find potion in brew_stuff array
  for(ptr=brews ; ptr->name ; ptr++) {
    if(is_abbrev(name, ptr->name))
      break;
  }

  // If potion name not foudn
  if(!ptr->name) {
    send_to_char("Have you been drinking your messed up brews again?\n\r",ch);
    send_to_char("Try typing \"brew help\" to get a listing.\n\r",ch);
    return;
  }

  // Check level restricts
  if(ch->player.level[DRUID_LEVEL_IND] < ptr->level) {
    send_to_char("You can't make that potion yet.\n\r", ch);
    return;
  }

  // Check if user has reagent
  compnum = real_object(ptr->reagent);
  reagent = get_obj_in_list_num(compnum, ch->carrying);
  if (!reagent)
    {
      send_to_char("You lack the proper reagent to brew.\n\r",ch);
      return;
    }

  // Remove used reagent
  obj_from_char(reagent);
  extract_obj(reagent);

  // Check for success
  if (number(1,101)>ch->skills[SKILL_BREW].learned && !IS_IMMORTAL(ch)) {
    charge_mana(ch, SKILL_BREW, FALSE);
    send_to_char("Oops... you really messed it up this time.\n\r",ch);
    if (number(1,10)==1) {
      send_to_char("In fact, it blows up right in your face!\n\r",ch);
      act("$n brews a volatile potion which blows up!",TRUE,ch,0,0,TO_ROOM);
      damage(ch, ch, dice(GetMaxLevel(ch),5), SKILL_BREW);
      if(ch->specials.position>POSITION_STUNNED)
	ch->specials.position=POSITION_STUNNED;
    }
    WAIT_STATE(ch,PULSE_VIOLENCE*2);
  }

  charge_mana(ch, SKILL_BREW, TRUE);
  if(!(obj=make_object(BREW_INDEX, VIRTUAL|NORAND))) {
    send_to_char("This skill is experiencing technical difficulty, contact a member of the staff.\n\r", ch);
    sprintf(buf, "screwup in brew, potion #%d", BREW_INDEX);
    log_msg(buf);
    return;
  }

  // These potions always only require level 1 to use (not to brew though!)
  obj->obj_flags.level = 1;

  switch (ptr->index)
    {
    case 1: // healing
      obj->name = ss_make("potion vial blue");
      obj->description = ss_make("A small beautiful blue vial has been carelessly left here");
      obj->short_description = ss_make("a blue vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_HEAL;
      break;
    case 2: // mana
      obj->name = ss_make("potion vial white");
      obj->description = ss_make("A small beautiful white vial has been carelessly left here");
      obj->short_description = ss_make("a white vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_MANA;
      obj->obj_flags.value[2] = SPELL_MANA;
      obj->obj_flags.value[3] = SPELL_MANA;
      break;
    case 3: // sanctuary
      obj->name = ss_make("potion vial purple");
      obj->description = ss_make("A small beautiful purple vial has been carelessly left here");
      obj->short_description = ss_make("a purple vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_SANCTUARY;
      break;
    case 4: //"fireshield":
      obj->name = ss_make("potion vial orange");
      obj->description = ss_make("A small beautiful purple vial has been carelessly left here");
      obj->short_description = ss_make("a orange vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_FIRESHIELD;
      break;
    case 5: //"coldshield":
      obj->name = ss_make("potion vial blue white");
      obj->description = ss_make("A small beautiful vial in white and blue with a frost rim on the edge is lying here");
      obj->short_description = ss_make("a white and blue vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_COLDSHIELD;
      break;
    case 6: //"elecshield":
      obj->name = ss_make("potion vial sparkling");
      obj->description = ss_make("A small beautiful sparkling blue vial has been carelessly left here");
      obj->short_description = ss_make("a sparkling vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_ELECSHIELD;
      break;
    case 7: //"energyshield":
      obj->name = ss_make("potion vial yellow");
      obj->description = ss_make("A small beautiful yellow vial has been carelessly left here");
      obj->short_description = ss_make("a yellow vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_ENERGYSHIELD;
      break;
    case 8: //"poisonshield":
      obj->name = ss_make("potion vial black");
      obj->description = ss_make("A small dark vial is lying here casting a very long dark shadow");
      obj->short_description = ss_make("a purple vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POISONSHIELD;
      break;
    case 9: //"manashield":
      obj->name = ss_make("potion vial clear");
      obj->description = ss_make("A small beautiful clear vial has been carelessly left here");
      obj->short_description = ss_make("a clear vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SKILL_MANASHIELD;
      break;
    case 10: //"moveshield":
      obj->name = ss_make("potion vial green");
      obj->description = ss_make("A small beautiful green vial is lying here, rumbling like it is going to explode!");
      obj->short_description = ss_make("a purple vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_MOVESHIELD;
      break;
    case 11: //"vampshield":
      obj->name = ss_make("potion vial blood black");
      obj->description = ss_make("A small black vial is lying here, there appears to be bloodstains on it");
      obj->short_description = ss_make("a bloody vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SKILL_VAMPSHIELD;
      break;
    case 12: // sight
      obj->name = ss_make("potion vial bright");
      obj->description = ss_make("A small bright vial is lying here, shining extremely brigthly");
      obj->short_description = ss_make("a bright vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;

      if (ch->player.level[DRUID_LEVEL_IND] < 21) {
	obj->obj_flags.value[1] = SPELL_DETECT_INVISIBLE;
      } else if (ch->player.level[DRUID_LEVEL_IND] < 41) {
	obj->obj_flags.value[1] = SPELL_DETECT_INVISIBLE;
	obj->obj_flags.value[2] = SPELL_TRUE_SIGHT;
      } else {
	obj->obj_flags.value[1] = SPELL_CURE_BLIND;
	obj->obj_flags.value[2] = SPELL_DETECT_INVISIBLE;
	obj->obj_flags.value[3] = SPELL_TRUE_SIGHT;
      }
      break;
    case 13: // bless
      obj->name = ss_make("potion vial bless");
      obj->description = ss_make("A small clear white vial has been carelessly left here");
      obj->short_description = ss_make("a clear white vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SKILL_BLESSING; // a paladins blessing
      break;
    case 14: // protection
      obj->name = ss_make("potion vial brown");
      obj->description = ss_make("A large solid brown vial is standing here");
      obj->short_description = ss_make("a solid brown vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;

      if (ch->player.level[DRUID_LEVEL_IND] < 21) {
	obj->obj_flags.value[1] = SPELL_ARMOR;
	obj->obj_flags.value[2] = SPELL_SHIELD;
      } else {
	obj->obj_flags.value[1] = SPELL_ARMOR;
	obj->obj_flags.value[2] = SPELL_SHIELD;
	obj->obj_flags.value[3] = SPELL_STONE_SKIN;
      }
      break;
    case 15: // Invisibility
      obj->name = ss_make("potion vial empty");
      obj->description = ss_make("A small seemingly empty vial has been carelessly left here");
      obj->short_description = ss_make("an empty vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;

      if (ch->player.level[DRUID_LEVEL_IND] < 21) {
	obj->obj_flags.value[1] = SPELL_INVISIBLE;
      } else {
	obj->obj_flags.value[1] = SPELL_INVISIBLE;
	obj->obj_flags.value[2] = SPELL_CAMOUFLAGE;
      }
      break;
    case 16: // Heroes
      obj->name = ss_make("potion vial green");
      obj->description = ss_make("A small dark green vial has been carelessly left here");
      obj->short_description = ss_make("a dark green vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_H_FEAST;
      break;
    case 17: // Fly
      obj->name = ss_make("potion vial grey");
      obj->description = ss_make("A small clear white vial has been carelessly left here");
      obj->short_description = ss_make("a clear white vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_FLY;
      break;
    case 18: // waterbreath
      obj->name = ss_make("potion vial ocean");
      obj->description = ss_make("A small vial in the color of the ocean is lying here. Waves are splashing around inside.");
      obj->short_description = ss_make("an ocean colored vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_WATER_BREATH;
      break;
    case 19: // group protect
      obj->name = ss_make("potion vial black");
      obj->description = ss_make("A large solid black vial is standing here");
      obj->short_description = ss_make("a solid black vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;

      obj->obj_flags.value[1] = SPELL_ARMOR_GROUP;

      break;
    case 20: // group sight
      obj->name = ss_make("potion vial bright");
      obj->description = ss_make("A small bright vial is lying here. It is shining extremely brightly");
      obj->short_description = ss_make("a bright vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;

      obj->obj_flags.value[1] = SPELL_TRUE_SEEING_GROUP;
      obj->obj_flags.value[1] = SPELL_DINVIS_GROUP;
      break;
    case 21: // group fly
      obj->name = ss_make("potion vial grey");
      obj->description = ss_make("A small grey vial has been carelessly left here");
      obj->short_description = ss_make("a grey vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_FLY_GROUP;

      break;
    case 22: // group invisibility
      obj->name = ss_make("potion vial empty");
      obj->description = ss_make("A small seemingly empty vial has been carelessly left here");
      obj->short_description = ss_make("an empty vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_INVIS_GROUP;
      break;
    case 23: // group waterbreathsp
      obj->name = ss_make("potion vial ocean");
      obj->description = ss_make("A small vial in the color of the ocean is lying here. Waves are splashing around inside.");
      obj->short_description = ss_make("an ocean colored vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_WATERBREATH_GROUP;
      break;
    case 24: // firebreath
      obj->name = ss_make("potion vial fire");
      obj->description = ss_make("A small vial is lying here, it seems to be burning inside");
      obj->short_description = ss_make("a burning vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POTION_FIRE_BREATH;
      break;
    case 25: // frostbreath
      obj->name = ss_make("potion vial frostbreath");
      obj->description = ss_make("A small icy vial is lying here, it seems to be made completely from ice");
      obj->short_description = ss_make("an icy vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POTION_FROST_BREATH;
      break;
    case 26: // acidbreath
      obj->name = ss_make("potion vial green acidbreath");
      obj->description = ss_make("A small green vial is lying here, it seems to be sizzling inside");
      obj->short_description = ss_make("a sizzling vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POTION_ACID_BREATH;
      break;
    case 27: // poisongasbreath
      obj->name = ss_make("potion vial poisongasbreath darkgreen");
      obj->description = ss_make("A small darkgreen vial is lying here, it seems to contain some sort of gas");
      obj->short_description = ss_make("a darkgreen vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POTION_POISON_GAS_BREATH;
      break;
    case 28: // lightningbreath
      obj->name = ss_make("potion vial lightningbreath electric");
      obj->description = ss_make("A small blue vial is lying here, it seems to be shooting small lightning bolts if you get too close");
      obj->short_description = ss_make("a blue vial marked by druids");
      obj->obj_flags.value[0] = ch->player.level[DRUID_LEVEL_IND] * druid_modifier;
      obj->obj_flags.value[1] = SPELL_POTION_LIGHTNING_BREATH;
      break;

    default:
      send_to_char("This skill is experiencing technical difficulty, contact a member of the staff.\n\r", ch);
      sprintf(buf, "screwup in brew, potion #%d", BREW_INDEX);
      log_msg(buf);
    }

  obj_to_char(obj,ch);
  act("$n mixes some reagents together and brews $p!",TRUE,ch,obj,0,TO_ROOM);
  act("You mix some reagents together and brew $p!\n\r",FALSE,ch,obj,0,TO_CHAR);

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_tan (struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *obj, *corpse;
  char name[MAX_INPUT_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  char buffer2[MAX_STRING_LENGTH];
  int level = 0;
  int item=-1;
  int count=0;
  int i=0;

  bool type_found=FALSE;
  bool char_corpse=FALSE;
  bool room_corpse=FALSE;

  const char *tan_types[]={
    "leggings",
    "sleeves",
    "helm",
    "gauntlets",
    "boots",
    "hauberk",
    "shield",
    "whip",
    "\n"
  };

  if (!can_do(ch, SKILL_TAN))
    return;

  only_argument(argument,name);
  if (!name) {
    send_to_char("Tan what?\n\r",ch);
    return;
  }

  while (!type_found && (count<8)) {
    if (!strcmp(name,tan_types[count])) {
      type_found=TRUE;
      item=count;
    } else
      count++;
  }

  if (item==-1) {
    send_to_char("The following are your choices:\n\r",ch);
    send_to_char("leggings, sleeves, helm, gauntlets, boots, hauberk, shield, or whip\n\r",ch);
    return;
  }

  /*search room*/
  corpse=get_obj_in_list_vis(ch,"corpse",(real_roomp(ch->in_room)->contents));
  if (corpse)
    room_corpse=TRUE;
  else {
    /* search char */
    corpse=get_obj_in_list_vis(ch,"corpse",ch->carrying);
    if (!corpse) {
      send_to_char("You need a corpse to tan hides, bub.\n\r",ch);
      return;
    }
    char_corpse=TRUE;
  }

  sprintf(buffer ,"a %s made out of %s hide",tan_types[item],
	  OBJ_SHORT(corpse)+14);
  sprintf(buffer2,"A %s made of %s hide is lying on the ground.",
	  tan_types[item],OBJ_SHORT(corpse)+14);

  ObjFromCorpse(corpse);	/* loot the corpse 920306 - DWB */

  if (number(1,101) > ch->skills[SKILL_TAN].learned) {
    charge_mana(ch, SKILL_TAN, FALSE);
    send_to_char("You botch the effort miserably.\n\r",ch);
  } else {
    charge_mana(ch, SKILL_TAN, TRUE);
    level = MAX((int)(GetMaxLevel(ch)/3), 1);

    item+=6641;
    obj=make_object(item,VIRTUAL|NORAND);
    if(!obj) {
      send_to_char("Error occurred in tan.\n", ch);
      return;
    }
    obj->obj_flags.cost_per_day = level*100;
    obj->obj_flags.level = (ubyte)(GetMaxLevel(ch)/4);
    if (item<6648) {
      obj->obj_flags.value[0]=level;
      obj->obj_flags.value[1]=level;
    } else {
      if (item == 6648) {
	obj->obj_flags.value[1] = MAX( (int) (GetMaxLevel(ch)/(number(10,25))), 3);
	obj->obj_flags.value[2] = MIN(( (int) (level/2)-2), 6);

      }
    }

    count = 0;
    if(level > 5) count++;
    if(level > 10) count++;
    if(level > 15) count++;
    if(level > 20) count++;
    if(level > 30) count++;
    if(level > 40) count++;

    i=getFreeAffSlot(obj);

    if(!(i ==-1)) {
      if(number(1,5) == 1)
	switch(number(1,count*2)){
	case 1 : {
	  obj->affected[i].location = APPLY_HITNDAM;
	  obj->affected[i].modifier = number(1,3);
	  break;}
	case 2: {
	  obj->affected[i].location = APPLY_HIT;
	  obj->affected[i].modifier = number(1, level);
	  break; }
	case 3 : {
	  obj->affected[i].location = APPLY_TRACK;
	  obj->affected[i].modifier = number(1, level);
	  break; }
	case 4 : {
	  obj->affected[i].location = APPLY_CHA;
	  obj->affected[i].modifier = number(1, 3);
	  break; }
	case 5 : {
	  obj->affected[i].location = APPLY_DEX;
	  obj->affected[i].modifier = number(1,3);
	  break; }
	case 6 : {
	  obj->affected[i].location = APPLY_STR;
	  obj->affected[i].modifier = number(1, 3);
	  break; }
	case 7 : {
	  obj->affected[i].location = APPLY_CON;
	  obj->affected[i].modifier = number(1, 3);
	  break; }
	case 8 : {
	  obj->affected[i].location = APPLY_SNEAK;
	  obj->affected[i].modifier = number(1, level/2);
	  break; }
	case 9 : {
	  obj->affected[i].location = APPLY_SAVE_ALL;
	  obj->affected[i].modifier = -1;
	  break; }
	case 10 : {
	  obj->affected[i].location = APPLY_HITROLL;
	  obj->affected[i].modifier = number(1,3);
	  break; }
	case 11 : {
	  obj->affected[i].location = APPLY_DAMROLL;
	  obj->affected[i].modifier = number(1,3);
	  break; }
	case 12 : {
	  obj->affected[i].location = APPLY_HIDE;
	  obj->affected[i].modifier = number(1, level/2);
	  break; }
	default : {
	  obj->affected[i].location = APPLY_AGE;
	  obj->affected[i].modifier = number(1, level/2);
	  break; }
	}
    }
    ss_free(obj->short_description);
    obj->short_description=ss_make(buffer);
    ss_free(obj->action_description);
    obj->action_description=ss_make(buffer);
    ss_free(obj->description);
    obj->description=ss_make(buffer2);

    obj_to_char(obj,ch);
    act("$n carefully crafts $p out of dead flesh.", TRUE, ch, obj, 0, TO_ROOM);
    act("You ply your trade, and the result is $p.", TRUE, ch, obj, 0, TO_CHAR);
  }
  WAIT_STATE(ch,PULSE_VIOLENCE*4);
}

/*** psionist skills ***/

void do_hypnosis (struct char_data *ch, char *argument, int command)
{
  struct char_data *victim;
  char buf[256];
  int leveldiff;

  if (!can_do(ch, SKILL_HYPNOSIS))
    return;

  if (!(victim=set_target(ch, argument, SKILL_HYPNOSIS)))
    return;

  if (IS_PC(ch) && IS_PC(victim))  {
    act("$CYYou can't seem to hypnotize $N.",FALSE,ch,0,victim,TO_CHAR);
    act("$CY$n tries to hypnotizes you, but can't.",TRUE,ch,0,victim,TO_VICT);
    act("$CY$n tries to hypnotize $N but fails.",TRUE,ch,0,victim,TO_ROOM);
    return;
  }

  if (circle_follow(victim, ch)) {
    send_to_char("Sorry, following in circles cannot be allowed.\n\r", ch);
    return;
  }

  if (GET_POS(victim)<=POSITION_SLEEPING) {
    send_to_char("They are not paying any attention to you.\n\r", ch);
    return;
  }

  if(!CountFollowers(ch))
    return;

  if (ch->skills[SKILL_HYPNOSIS].learned<number(1,101)) {
    send_to_char ("Your attempt at hypnosis was laughable.\n\r",ch);
    charge_mana(ch, SKILL_HYPNOSIS, FALSE);
  } else {

    charge_mana(ch, SKILL_HYPNOSIS, TRUE);

    int mod = 0;

    mod += (GetMaxLevel(victim) - GetMaxLevel(ch)) / 5;
    mod += victim->abilities.intel - ch->abilities.intel;

    if(IsImmune(victim,IMM_CHARM)) {
      mod += 6;
    } else if(IsResist(victim,IMM_CHARM)) {
      mod += 3;
    } else if(IsSusc(victim,IMM_CHARM)) {
      mod += -3;
    }

    /* From now on mobs cannot be hypnoed if they are IMM */
    if(ImpSaveSpell(victim, SAVING_SPELL, mod)) {
      if(number(1,GetMaxLevel(ch))<=GetMaxLevel(victim)) {
	act ("$n fails to hypnotize a now $CRangry$CN $N!",TRUE,ch,0,victim,TO_ROOM);
	act ("You are $CRangered$CN by $n's attempt to hypnotize you!",TRUE,ch,0,victim,TO_VICT);
	act ("You fail to hypnotize a now $CRangry$CN $N!",TRUE,ch,0,victim,TO_CHAR);
	AddHated(victim, ch);
	if (!victim->specials.fighting)
	  SetVictFighting(ch, victim);
      } else {
	act("You fail to hypnotize $N.",FALSE,ch,0,victim,TO_CHAR);
	act("$n attempts to hypnotize you.",TRUE,ch,0,victim,TO_VICT);
	act("$n fails to hypnotize $N.",TRUE,ch,0,victim,TO_ROOM);
      }

      /* When hypno fails...
	 first time mob become RES
	 second time the mob gets IMM
      */
      if (!IsResist(victim, IMM_CHARM) && !IsImmune(victim, IMM_CHARM) && !IS_PC(victim)) {
	SET_BIT(victim->immune, IMM_CHARM);
      } else if (!IsImmune(victim, IMM_CHARM) && !IS_PC(victim)) {
	SET_BIT(victim->M_immune, IMM_CHARM);
      }
    } else {

      sprintf(buf," %s (%d) HYPNOTIZES %s (%d).",
	      GET_REAL_NAME(ch),GetMaxLevel(ch),
	      GET_REAL_NAME(victim),GetMaxLevel(victim));
      stat_log(buf,0);

      act ("$n hypnotizes $N.",TRUE,ch,0,victim,TO_ROOM);
      act ("You are hypnotized by $n.",TRUE,ch,0,victim,TO_VICT);
      act ("You hypnotize $N.",TRUE,ch,0,victim,TO_CHAR);
      if (!IS_PC(victim))
	fix_mob_bits(victim);
      if (victim->master)
	stop_follower(victim);
      add_follower (victim,ch, 0);
      MakeCharmed(ch, victim, GetMaxLevel(ch), SKILL_HYPNOSIS, 0);
    }
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}


void do_meditation (struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_MEDITATE))
    return;

  if ( ch->skills[SKILL_MEDITATE].learned < dice (1,101) ) {
    send_to_char ("You can't clear your mind at this time.\n\r",ch);
  } else {
    if (ch->specials.conditions[1] == 0 /*hungry or*/
	|| ch->specials.conditions[2] == 0 /*thirsty or*/
	|| ch->specials.conditions[0] > 7) /*alcohol in blood*/
      {
	send_to_char("You cannot meditate while your body needs sustenance.\n\r",ch);
	return;
      }
    if(IS_AFFECTED(ch,AFF_MEDITATE)) {
      send_to_char("You are already in a meditative state of mind.\n\r", ch);
      return;
    }
    ch->specials.position=POSITION_RESTING;/* is meditating */
    send_to_char ("You sit down and start resting and clear your mind of all thoughts.\n\r",ch);
    act ("$n sits down and starts chanting relaxing mantras.",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SCROLL,
               SKILL_MEDITATE, 0, 0, AFF_MEDITATE,
               2, 0, FALSE, FALSE, FALSE, NULL);
  }
  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_scry (struct char_data *ch, char *argument, int command)
{
  char target_name[MAX_INPUT_LENGTH];
  struct char_data *target;
  int location,old_location;
  struct room_data *rp;

  if (!can_do(ch, SKILL_SCRY))
    return;

  only_argument(argument,target_name);
  if ( !(target=get_char_vis_world(ch,target_name,NULL)) ) {
    send_to_char ("You can't sense that person anywhere.\n\r",ch);
    return;
  }

  old_location = ch->in_room;
  location = target->in_room;
  rp = real_roomp(location);

  if (IS_IMMORTAL(target) || !rp) {
    send_to_char("Your mind is not strong enough.\n\r", ch);
    return;
  }

  if (dice(1,101) > ch->skills[SKILL_SCRY].learned) {
    send_to_char("You cannot open a window at this time.\n\r", ch);
    charge_mana(ch, SKILL_SCRY, FALSE);
  } else {
    charge_mana(ch, SKILL_SCRY, TRUE);
    send_to_char("You close your eyes and envision your target...\n\r",ch);
    char_from_room(ch);
    char_to_room(ch, location);
    do_look(ch, "",15);
    char_from_room(ch);
    char_to_room(ch, old_location);
  }
  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_adrenalize (struct char_data *ch, char *argument, int command)
{
  struct char_data *target;

  if (!can_do(ch, SKILL_ADRENALIZE))
    return;

  if (!(target=set_target(ch, argument, SKILL_ADRENALIZE)))
    return;

  if(affected_by_spell(target, SKILL_ADRENALIZE))
    {
      if (ch==target)
	send_to_char("You are already adrenalized.\n\r", ch);
      else
	act("$N is already adrenalized.",TRUE,ch,0,target,TO_CHAR);
      return;
    }

  if (ch->skills[SKILL_ADRENALIZE].learned < dice (1,101)) {
    act("You try to adrenalize but fail.",FALSE,ch,0,0,TO_CHAR);
    act("$n tries to adrenalize but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_ADRENALIZE, FALSE);
  } else {
    charge_mana(ch, SKILL_ADRENALIZE, TRUE);

    MakeAffect(ch, target, SPELL_TYPE_SPELL,
	       SKILL_ADRENALIZE, GetMaxLevel(ch)/10, APPLY_HITNDAM, 0,
	       0, 4, FALSE, FALSE, FALSE, NULL);

    if (ch==target) {
      act ("You adrenalize yourself!",TRUE,ch,0,target,TO_CHAR);
      act ("$n concentrates and gets a wild look in $s eyes!",TRUE,ch,0,target,TO_ROOM);
    } else {
      act ("You adrenalize $N!",TRUE,ch,0,target,TO_CHAR);
      act ("$n concentrates and looks at $N...",TRUE,ch,0,target,TO_NOTVICT);
      act ("$N suddenly gets a wild look in $s eyes!",TRUE,ch,0,target,TO_NOTVICT);
      act ("$n adrenalizes you!",TRUE,ch,0,target,TO_VICT);
    }
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_levitate (struct char_data *ch, char *argument, int command)
{
  struct char_data *target;

  if (!can_do(ch, SKILL_LEVITATE))
    return;

  if (!(target=set_target(ch, argument, SKILL_LEVITATE)))
    return;

  if (IS_AFFECTED(target, AFF_FLYING)) {
    if (ch==target)
      send_to_char ("You are already levitating.\n\r",ch);
    else
      act("$N is already levitating.",FALSE,ch,0,target,TO_CHAR);
    return;
  }

  if (ch->skills[SKILL_LEVITATE].learned<number(1,101)) {
    send_to_char ("You try to levitate but fail.\n\r",ch);
    act("$n attempts to levitate but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_LEVITATE, FALSE);
  } else {
    charge_mana(ch, SKILL_LEVITATE, TRUE);
    send_to_char("You concentrate intensely for a moment...\n\r", ch);
    act("$n concentrates intensely for a moment...",TRUE,ch,0,0,TO_ROOM);
    send_to_char("You slowly levitate above the ground.\n\r", target);
    act("$n slowly rises above the ground.",TRUE,target,0,0,TO_ROOM);
    MakeAffect(ch, target, SPELL_TYPE_SPELL,
               SKILL_LEVITATE, 0, 0, AFF_FLYING,
               0, 1, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_invisibility (struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_INVIS))
    return;

  if (IS_AFFECTED(ch, AFF_INVISIBLE)) {
    send_to_char ("You're already invisible.\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_INVIS].learned<number(1,101)) {
    send_to_char ("You try to vanish but only fade for a moment.\n\r",ch);
    act("$n attempts to vanish, but only fades for a moment.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_INVIS, FALSE);
  } else {
    charge_mana(ch, SKILL_INVIS, TRUE);
    act ("$n suddenly vanishes from sight!",TRUE,ch,0,0,TO_ROOM);
    send_to_char ("You fade from everyone's sight.\n\r",ch);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_INVIS, 0, 0, AFF_INVISIBLE,
               0, 1, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_canibalize (struct char_data *ch, char *argument, int command)
{

  int hit_points,mana_points;
  char number[MAX_INPUT_LENGTH];
  int count;
  bool num_found=FALSE;

  if (!can_do(ch, SKILL_CANIBALIZE))
    return;

  only_argument (argument,number);
  for (count=0;(!num_found) && (count<9);count++)
    if ((number[count]>='1') && (number[count]<='9'))
      num_found=TRUE;
  if (!num_found)
    {
      send_to_char ("Please include a number after the command.\n\r",ch);
      return;
    }
  sscanf (number,"%d",&hit_points);
  if(hit_points < 0)
    {
      send_to_char("nice try bozo\n\r", ch);
      return;
    }
  mana_points = (hit_points * 3) / 2;

  if (GET_HIT(ch) < (hit_points+5)) {
    send_to_char ("You don't have enough physical stamina to canibalize.\n\r",ch);
    return;
  }
  if ( (GET_MANA(ch)+mana_points) > (GET_MAX_MANA(ch)) )
    {
      send_to_char ("Your mind cannot handle that much extra energy.\n\r",ch);
      return;
    }
  if (ch->skills[SKILL_CANIBALIZE].learned < dice(1,101)) {
    send_to_char ("You try to canibalize your stamina but the energy escapes before you can harness it.\n\r",ch);
    GET_HIT(ch) -= hit_points;
  }
  else
    {
      send_to_char ("You sucessfully convert your stamina to Mental power.\n\r",ch);
      GET_HIT(ch) -= hit_points;
      GET_MANA(ch) += mana_points;
    }
  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_illusionary_shroud(struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_ILLUSIONARY_SHROUD))
    return;

  if (IS_AFFECTED(ch, AFF_ILLUSION)) {
    send_to_char ("You are already enveloped in an illusionary shroud.\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_ILLUSIONARY_SHROUD].learned < number(1,101)) {
    send_to_char ("You failed and blur for a moment.\n\r",ch);
    act("$n tries to create an illusionary shroud but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_ILLUSIONARY_SHROUD, FALSE);
  } else {
    send_to_char ("You are enveloped in an illusionary shroud!\n\r",ch);
    act ("$n creates an illusionary shroud and blurs in and out of existence!",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_ILLUSIONARY_SHROUD, 0, APPLY_NONE, AFF_ILLUSION,
               5, 10, FALSE, FALSE, FALSE, NULL);
  }
  WAIT_STATE (ch,PULSE_VIOLENCE*2);
}

void do_phantasmal_killer (struct char_data *ch, char *argument, int command)
{
  char buf[256];
  struct char_data *killer;

  if(!can_do(ch,SKILL_PHANTASMAL))
    return;

  if(!CountFollowers(ch))
    return;

  if (ch->skills[SKILL_PHANTASMAL].learned<dice(1,101)) {
    send_to_char ("You attempt to summon a ghost but you fail.\n\r",ch);
    charge_mana(ch, SKILL_PHANTASMAL, FALSE);
  } else {
    charge_mana(ch, SKILL_PHANTASMAL, TRUE);
    if(!(killer=make_mobile(VMOB_MOB_KILL, VIRTUAL))) {
      log_msg("screwup in phantasmal killer.");
      send_to_char("This skill is experiencing technical difficulty.\n\r",ch);
      return;
    }
    killer->points.mana=0;

    char_to_room (killer,ch->in_room);
    fix_mob_bits(killer);

    act ("You've summoned $N.\n\r",FALSE,ch,0,killer,TO_CHAR);
    act ("$n summons $N from the spirit world!",TRUE,ch,0,killer,TO_ROOM);



    sprintf(buf,"%s (%d) summons a PHANTASM",
	    GET_REAL_NAME(ch),GetMaxLevel(ch));
    stat_log(buf,0);

    add_follower (killer,ch, 0);
    MakeCharmed(ch, killer, GetMaxLevel(ch), SKILL_PHANTASMAL, -4);

    killer->player.clss = CLASS_WARRIOR;
    killer->points.max_hit = GetMaxLevel(ch)*5;
    killer->points.hit = GetMaxLevel(ch)*5;
    killer->points.hitroll = (sh_int) GetMaxLevel(ch)/5;
    killer->specials.damnodice = (sh_int) GetMaxLevel(ch)/5;
    killer->mult_att = (sh_int) GetMaxLevel(ch)/10;
    killer->player.level[WARRIOR_LEVEL_IND] = MIN
      ((ubyte)GetMaxLevel(ch)/2, MAX_MORT);
    UpdateMaxLevel(killer);
  }
  WAIT_STATE (ch,PULSE_VIOLENCE*2);
}

void do_drain_mana(struct char_data *ch, char *argument, int command)
{
  int dam, mana_before, mana_diff;
  struct char_data *target;

  if (!can_do(ch, SKILL_DRAIN_MANA))
    return;

  if (!(target=set_target(ch, argument, SKILL_DRAIN_MANA)))
    return;

  dam=dice(2, GetMaxLevel(ch)+GetMaxLevel(target));
  if (IsImmune(target, IMM_DRAIN) || ch->skills[SKILL_DRAIN_MANA].learned<dice(1,101))
    dam=0;
  else if (IsResist(target, IMM_DRAIN))
    dam /=2;

  mana_before=GET_MANA(target);
  GET_MANA(target)=MAX(0, GET_MANA(target)-dam);
  mana_diff=mana_before-GET_MANA(target);

  if (!mana_diff) {
    act("You try to drain mana from $N but utterly fail!",FALSE,ch,0,target,TO_CHAR);
    act("$n tries to drain mana from $N but utterly fails!",TRUE,ch,0,target,TO_ROOM);
    act("$n tries to drain mana from you but utterly fails!",TRUE,ch,0,target,TO_VICT);
  } else {
    act("You drain precious mana from $N!",FALSE,ch,0,target,TO_CHAR);
    act("$n drains precious mana from $N!",TRUE,ch,0,target,TO_ROOM);
    act("$n drains precious mana from you!",TRUE,ch,0,target,TO_VICT);
    GET_MANA(ch)=MIN(GET_MANA(ch)+(mana_diff/2), GET_MAX_MANA(ch));
  }

  if (!target->specials.fighting)
    set_fighting(target, ch);

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_great_sight(struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_GREAT_SIGHT))
    return;

  if (IS_AFFECTED(ch, AFF_GREAT_SIGHT)) {
    send_to_char ("You have already achieved great sight.\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_GREAT_SIGHT].learned < dice (1,101)) {
    act("You try to achieve great sight but fail.",FALSE,ch,0,0,TO_CHAR);
    act("$n tries to achieve great sight but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_GREAT_SIGHT, FALSE);
  } else {
    charge_mana(ch, SKILL_GREAT_SIGHT, TRUE);
    act("Your eyes flare brightly as you achieve great sight.",FALSE,ch,0,0,TO_CHAR);
    act("$n's eyes flare brightly as $e achieves great sight.",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_GREAT_SIGHT, 0, APPLY_NONE, AFF_GREAT_SIGHT,
               GetMaxLevel(ch)/2, 5, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_spell_shield (struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_SPELL_SHIELD))
    return;

  if (affected_by_spell(ch,SKILL_SPELL_SHIELD)) {
    send_to_char ("You already have spell shield in operation.\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_SPELL_SHIELD].learned < dice(1,101)) {
    send_to_char ("You failed to errect a spell barrier!\n\r",ch);
    charge_mana(ch, SKILL_SPELL_SHIELD, FALSE);
  } else {
    charge_mana(ch, SKILL_SPELL_SHIELD, TRUE);
    send_to_char ("You erect a transparent anti-magic sphere about your person.\n\r",ch);
    act ("$n is surounded by a magical globe which is tranparent.",TRUE,ch,0,0,TO_ROOM);

    MakeAffect(ch, ch, SPELL_TYPE_SPELL, SKILL_SPELL_SHIELD,
	       IMM_FIRE | IMM_ELEC | IMM_DRAIN | IMM_SLEEP | IMM_HOLD | IMM_ENERGY,
	       APPLY_IMMUNE, 0,
	       0, 5, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_gateway(struct char_data *ch, char *argument, int cmd)
{
  char tar_name[256];
  struct char_data *target, *gate1, *gate2 = NULL;
  int skill;
  char buf[256];

  if (cmd==383)
    skill=SKILL_LESSER_GATE;
  else
    skill=SKILL_GREATER_GATE;

  if (!can_do(ch, skill))
    return;

  only_argument(argument,tar_name);
  if (!(target=get_char_vis_world(ch,tar_name,NULL))) {
    send_to_char ("You cannot open a gateway to someone that does not exist.\n\r",ch);
    return;
  }

  if (ch->in_room == target->in_room) {
    act("That would be a waste of energy, $N is right here.",FALSE,ch,0,target,TO_CHAR);
    return;
  }
  /*  if (!IS_PC(target))
      {
      act("You have no psychic link with $N.  The gateway fails.",FALSE,ch,0,target,TO_CHAR);
      return;
      }
  */
  if(!travel_check(ch, target) && !HAS_GCMD(ch,GCMD_TRANS) ||
     (IS_SET(real_roomp(target->in_room)->room_flags, BRUJAH_RM)))
    {
      send_to_char("An unseen force prevents you from opening a gateway.\n\r", ch);
      charge_mana(ch, skill, FALSE);
      return;
    }

  if(!travel_check(ch, target) && !HAS_GCMD(ch,GCMD_TRANS) ||
     (IS_SET(real_roomp(target->in_room)->room_flags, GOD_RM) && (TRUST(ch) < TRUST_LORD)))
    {
      send_to_char("An unseen force prevents you from opening a gateway.\n\r", ch);
      charge_mana(ch, skill, FALSE);
      return;
    }

  if (number(1,101) > ch->skills[skill].learned) {
    act("You lose your concentration and fail to open a dimensional gateway.",FALSE,ch,0,0,TO_CHAR);
    act("$n loses $s concentration and fails to open a dimensional gateway.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, skill, FALSE);
    return;
  }

  if(!(gate1=make_mobile(VMOB_GATE_MOB, VIRTUAL)) || !(gate2=make_mobile(VMOB_GATE_MOB, VIRTUAL))) {
    log_msg("screwup in gateway.");
    send_to_char("Gateways are currently experiencing technical difficulty.\n\r", ch);
    return;
  }

  charge_mana(ch, skill, TRUE);

  if(IS_IMMORTAL(ch) && (skill == SKILL_GREATER_GATE)) skill=0;
  GET_EXP(gate1)=GET_EXP(gate2)=skill; /* yes, it's a kludge - but act_ptr is already being */
  gate1->act_ptr=gate2->act_ptr=0; /* used to time collapse of the gate */
  gate1->other_gate=gate2;
  gate2->other_gate=gate1;
  char_to_room(gate1, ch->in_room);
  char_to_room(gate2, target->in_room);

  sprintf(buf,"%s(%d) GATES to %s(%d)",
	  GET_REAL_NAME(ch),GetMaxLevel(ch),GET_REAL_NAME(target),GetMaxLevel(target));
  stat_log(buf,0);

  act("You close your eyes and concentrate intensely...",FALSE,ch,0,0,TO_CHAR);
  act("$n closes $s eyes and concentrates intensely...",TRUE,ch,0,0,TO_ROOM);
  act("$CBIn a blinding flash of light a dimensional gateway appears!$CN",FALSE,gate1,0,0,TO_ROOM);
  act("$CBIn a blinding flash of light a dimensional gateway appears!$CN",FALSE,gate2,0,0,TO_ROOM);

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_psi_attack (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  int dam, skill;
  bool save_success;

  switch (cmd) {
  case 389: skill=SKILL_PSIONIC_BLAST;
    dam=dice(GetMaxLevel(ch),9);
    break;
  case 422: skill=SKILL_COMBUSTION;
    dam=dice(GetMaxLevel(ch),7);
    break;
  case 423: skill=SKILL_THOUGHT_THROW;
    dam=dice(GetMaxLevel(ch),5);
    break;
  default:  skill=SKILL_CONSTRICT;
    dam=dice(GetMaxLevel(ch),3);
    break;
  }

  if (!can_do(ch, skill))
    return;

  if (!(victim=set_target(ch, arg, skill)))
    return;

  if (skill==SKILL_COMBUSTION)
    {
      /* Check to see if you are underwater */
      if(UNDERWATER(ch))
	{
	  act("You have trouble igniting $N, with all the water.",TRUE,ch,0,victim,TO_CHAR);
	  act("You notice the flames do not seem to catch as fast.",TRUE,ch,0,0,TO_ROOM);
	}
    }

  act("You stare at $N intensely for a moment...",TRUE,ch,0,victim,TO_CHAR);
  act("$n stares at you intensely for a moment...",TRUE,ch,0,victim,TO_VICT);
  act("$n stares intensely at $N for a moment...",TRUE,ch,0,victim,TO_ROOM);

  charge_mana(ch, skill, TRUE);
  if (number(1,101) > ch->skills[skill].learned)
    dam=0;

  save_success = saves_spell(victim, SAVING_SPELL, 0);

  if (dam && skill==SKILL_THOUGHT_THROW && !save_success) {
    GET_POS(victim)=POSITION_SITTING;
  }

  if (save_success)
    dam /=2;

  damage(ch, victim, dam, skill);

  /* PAC took out the "*2" */
  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_brainstorm (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  int dam, d;

  if (!can_do(ch, SKILL_BRAINSTORM))
    return;


  charge_mana(ch, SKILL_BRAINSTORM, TRUE);

  if (number(1,101)>ch->skills[SKILL_BRAINSTORM].learned)
    dam=0;
  else
    dam=dice(GetMaxLevel(ch), 9);

  act("You close your eyes and muster a blast of enormous psychic energy...",TRUE,ch,0,0,TO_CHAR);
  act("$n closes $s eyes and musters a blast of enormous psychic energy...",TRUE,ch,0,0,TO_ROOM);

  EACH_CHARACTER(iter, vict)
    {
      if (ch->in_room==vict->in_room) {
	if(can_hurt(ch,vict)) {
	  if (saves_spell(vict, SAVING_SPELL, 0))
	    d = dam / 2;
	  else
	    d = dam;
	  damage(ch, vict, d, SKILL_BRAINSTORM);
	}
      } else if(real_roomp(ch->in_room)->zone ==
		real_roomp(vict->in_room)->zone)
        act("Your head throbs painfully for a moment.",FALSE,vict,0,0,TO_CHAR);
    }
  END_AITER(iter);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_tolerance(struct char_data *ch, char *arg, int command)
{
  if (!can_do(ch, SKILL_TOLERANCE))
    return;

  if (IS_AFFECTED2(ch, AFF2_TOLERANCE)) {
    send_to_char ("You are already encased in a psi particle shield!\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_TOLERANCE].learned < number(1,101)) {
    send_to_char ("A psi particle shield of tolerance appears, but collapses.\n\r",ch);
    act("$n tries to create a psi particle shield of tolerance but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_TOLERANCE, FALSE);
  } else {
    send_to_char ("You are encased in a psi particle shield!\n\r",ch);
    act ("$n creates a psi particle shield of tolerance that encases them!",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_TOLERANCE, 0, APPLY_AFF2, AFF2_TOLERANCE,
               5, 15, FALSE, FALSE, FALSE, NULL);
  }
  WAIT_STATE (ch,PULSE_VIOLENCE*2);
}

void do_ego_whip (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *target;

  if (!can_do(ch, SKILL_EGO_WHIP))
    return;

  if (!(target=set_target(ch, arg, SKILL_EGO_WHIP)))
    return;

  if(ch->specials.fighting) {
    send_to_char_formatted("$CgYou can't do that when you are fighting!\n\r",ch);
    return;
  }

  if(ch==target)
    {
      send_to_char("You want to whip yourself? Not a good idea..\n\r", ch);
      return;
    }

  if(affected_by_spell(target, SKILL_EGO_WHIP))
    {
      if(ch==target)
	send_to_char("You want to whip yourself? Not a good idea..\n\r", ch);
      else
	act("$N is already feeling pretty lousy.",TRUE,ch,0,target,TO_CHAR);
      return;
    }

  if (ch->skills[SKILL_EGO_WHIP].learned < dice (1,101)) {
    act("You try to whip $N emotionally, but fail.",FALSE,ch,0,target,TO_CHAR);
    act("$n tries to whip $N but fails.",TRUE,ch,0,target,TO_ROOM);
    charge_mana(ch, SKILL_EGO_WHIP, FALSE);
  } else {
    charge_mana(ch, SKILL_EGO_WHIP, TRUE);

    MakeAffect(ch, target, SPELL_TYPE_SPELL,
	       SKILL_EGO_WHIP, -GetMaxLevel(ch)/10, APPLY_HITNDAM, 0,
	       0, 4, FALSE, FALSE, FALSE, NULL);

    MakeAffect(ch, target, SPELL_TYPE_SPELL,
	       SKILL_EGO_WHIP, -GetMaxLevel(ch)/20, APPLY_DEX, 0,
	       0, 4, FALSE, FALSE, FALSE, NULL);

    act ("You demoralize $N!",TRUE,ch,0,target,TO_CHAR);
    act ("$n concentrates and emotionally whips $N...",TRUE,ch,0,target,TO_NOTVICT);
    act ("$N suddenly gets a depressed look in $s eyes!",TRUE,ch,0,target,TO_NOTVICT);
    act ("$n emotionally $CgDestroys $CYYOU!",TRUE,ch,0,target,TO_VICT);
  }


  if(!ch->specials.fighting)
    set_fighting(ch, target);


  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_awe(struct char_data *ch, char *arg, int cmd)
{
  char_data *vict;

  if(!can_do(ch, SKILL_AWE))
    return;

  if (!(vict=set_target(ch, arg, SKILL_AWE)))
    return;

  if(!ch->specials.fighting) {
    send_to_char_formatted("$CrYou must be fighting someone!\n\r",ch);
    return;
  }

  if (SkillChance(ch,vict,30,IMM_ENERGY,SPLMOD_INT | SPLMOD_WIS | SPLMOD_CON, SKILL_AWE))
    {
      GET_MANA(ch) -= 20;
      act("$CbYou subdue $N, they no longer want to fight you!",TRUE,ch,0,vict,TO_CHAR);
      act("$Cr$n is $cbamazing, $Cwyou don't want to fight anymore.",FALSE,ch,0,vict,TO_VICT);
      act("$Cr$n subdues $N with $s mind!",TRUE,ch,0,vict,TO_NOTVICT);

      stop_fighting(vict);

    }
  else {
    GET_MANA(ch) -= 10;
    act("$CbYou try to subdue $N, but you fail!",FALSE, ch, 0, vict,TO_CHAR);
    act("$Cr$n tried to subdue you, but failed!",FALSE, ch, 0, vict, TO_VICT);
    act("$Cr$n tried to subdue $ with $s mind, but failed!",FALSE,ch,0,vict, TO_NOTVICT);
    set_fighting(vict, ch);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}
/*
  void do_cell_adjustment(struct char_data *ch, char *arg, int cmd)
  {
  int level = 0;
  struct obj_data *obj = NULL;

  if(!can_do(ch, SKILL_CELL_ADJUSTMENT)
  return;

  level = GetMaxLevel(ch);

  if (ch->skills[SKILL_CELL_ADJUSTMENT].learned < number(1,101)) {
  send_to_char ("You failed.\n\r",ch);
  act("$n tried to rearrange the cells of $N, but failed.",TRUE,ch,0,0,TO_ROOM);
  charge_mana(ch, SKILL_CELL_ADJUSTMENT, FALSE);
  } else {
  send_to_char ("You are enveloped in a wave of energy, you are healed!\n\r",ch);
  act ("$n creates a wave of energy and rearranges $N's cells.",TRUE,ch,0,0,TO_ROOM);

  spell_cure_blind(level, ch, SPELL_TYPE_SPELL, ch, obj);
  spell_remove_poison(level, ch, SPELL_TYPE_SPELL, ch, obj);


  }
  WAIT_STATE (ch,PULSE_VIOLENCE*2)
  }
*/
/*** ranger skills ***/

void do_hunt (struct char_data *ch, char *argument, int command)
{
  char name[256], buf[256];
  int dist;

#if NOTRACK
  send_to_char("Sorry, tracking is disabled. Try again after reboot.\n\r",ch);
  return;
#endif

  if (!can_do(ch, SKILL_HUNT))
    return;

  only_argument(argument, name);
  if(!get_char(name)) {
    send_to_char("You are unable to find traces of one.\n\r", ch);
    return;
  }

  if (!IS_LIGHT(real_roomp(ch->in_room)))
    {
      path_kill(ch->hunt_info);
      ch->hunt_info = NULL;
      send_to_char("It's too dark in here to track...\n\r",ch);
      return;
    }

  if (IS_GOD(ch))
    dist = MAX_ROOMS;
  else
    {
      dist = ch->player.level[RANGER_LEVEL_IND]*10;

      switch(GET_RACE(ch))
	{
	case RACE_CANIS:
	case RACE_FELIS:
	  dist = dist * 5 / 2;
	  break;
	case RACE_ELF:
	  dist *= 2;		/* even better */
	  break;
	case RACE_UNDERWORLD:
	  dist = MAX_ROOMS;	/* as good as can be */
	  break;
	default:
	  break;
	}
    }

  ch->hunt_info =
    path_to_name(ch->in_room, name, dist, HUNT_THRU_DOORS | HUNT_GLOBAL);

  WAIT_STATE(ch, PULSE_VIOLENCE*1);

  if(!ch->hunt_info)
    {
      send_to_char("You are unable to find traces of one.\n\r", ch);
    }
  else
    {
      sprintf(buf, "You see traces of your quarry to the %s\n\r",
	      dirs[path_dir(ch->in_room, ch->hunt_info)]);
      send_to_char(buf,ch);
    }
}

void do_thrust (struct char_data *ch, char *argument, int command)
{
  struct char_data *target;
  int dam,w_type;
  struct obj_data *wielded;

  if (!can_do(ch, SKILL_THRUST))
    return;

  if (!(target=set_target(ch, argument, SKILL_THRUST)))
    return;

  if(ch->in_room != target->in_room)
    {
      send_to_char("They aren't here any more.\n\r", ch);
      return;
    }

  if (GET_MOVE(ch) < 10) {
    act("$CYYou don't have enough mobility to thrust.",TRUE,ch,0,0,TO_CHAR);
    return;
  }

  GET_MOVE(ch) -=10; /* remove the Move no matter if success or failure */

  if (ch->skills[SKILL_THRUST].learned < dice (1,101)) {
    WAIT_STATE (ch,PULSE_VIOLENCE);
    send_to_char ("You've botched the thrust, and run past your target!\n\r",ch);
    act ("$n takes a wild lunge at $N but misses!",TRUE,ch,0,target,TO_NOTVICT);
    act ("$n lunges at you but misses and runs past you!",TRUE,ch,0,target,TO_VICT);
    if (GET_POS(target)>POSITION_SLEEPING) {
      act ("$n returns the attack!",TRUE,target,0,ch,TO_ROOM);
      w_type = GetWeaponType (target,&wielded);
      dam = GetWeaponDam (target,ch,wielded);
      damage(target, ch, dam, w_type);
    }
  }

  else
	{
      w_type = GetWeaponType (ch,&wielded);
      if (!(SkillSave (RANGER_LEVEL_IND,ch,target))) // only normal damage dealt
	  {
	    dam = GetWeaponDam (ch,target,wielded);
        act ("You take a wild lunge, but $N absorbs your blow!",TRUE,ch,0,target,TO_CHAR);
        act ("$n takes a wild lunge, but $N absorbs the blow!",TRUE,ch,0,target,TO_ROOM);
		act ("$n lunges at you, but you absorb the blow!",TRUE,ch,0,target,TO_VICT);
	  }
	  else //total success, double damage dealt as planned
	  {
        dam = GetWeaponDam (ch,target,wielded) *2;
        act ("You take a wild lunge at $N!",TRUE,ch,0,target,TO_CHAR);
        act ("$n takes a wild lunge at $N!",TRUE,ch,0,target,TO_ROOM);
        act ("$n lunges at you!",TRUE,ch,0,target,TO_VICT);
	  }
      WAIT_STATE (ch,2*PULSE_VIOLENCE);
      damage(ch, target, dam, w_type);
    }
}

void do_ration(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *corpse;
  struct obj_data *food;
  int i;

  if (!can_do(ch, SKILL_RATION))
    return;

  half_chop(argument,arg1,arg2);
  corpse=get_obj_in_list_vis(ch,arg1,(real_roomp(ch->in_room)->contents));
  if(!corpse)
    {
      send_to_char("That's not here.\n\r",ch);
      return;
    }
  if (!IS_CORPSE(corpse))
    {
      send_to_char("You can't carve that!\n\r",ch);
      return;
    }
  if(corpse->obj_flags.weight<70)
    {
      send_to_char("There is no good meat left on it.",ch);
      return;
    }

  if (ch->skills[SKILL_RATION].learned < dice(1,101)) {
    send_to_char ("You can't seem to locate the choicest parts of the corpse.\n\r",ch);
    charge_mana(ch, SKILL_RATION, FALSE);
  } else {
    charge_mana(ch, SKILL_RATION, TRUE);
    act("$n carves up the $p and creates a healthy ration.",FALSE,ch,corpse,0,TO_ROOM);
    send_to_char("You carve up a fat ration.\n\r",ch);
    if(!(food=make_object(544,VIRTUAL|NORAND))) {
      send_to_char("Sorry, I can't create a ration\n\r", ch);
      return;
    }

    ss_free(food->name);
    food->name = ss_make("ration slice filet");
    sprintf(buffer,"a Ration%s",OBJ_SHORT(corpse)+10);
    ss_free(food->short_description);
    food->short_description=ss_make(buffer);
    ss_free(food->action_description);
    food->action_description=ss_make(buffer);
    sprintf(arg2,"A %s is lying on the ground.",buffer);
    ss_free(food->description);
    food->description=ss_make(arg2);
    corpse->obj_flags.weight=corpse->obj_flags.weight-50;
    food->obj_flags.level = 1;
    food->obj_flags.type_flag = 19;
    food->obj_flags.wear_flags = ITEM_TAKE;
    food->obj_flags.extra_flags = ITEM_ORGANIC | ITEM_MAGIC;
    food->obj_flags.cost = 100;
    food->obj_flags.value[0] = 0;
    food->obj_flags.value[3] = 0;
    food->obj_flags.value[1] = 0;
    food->obj_flags.value[2] = 0;

    for(i=0;i<MAX_OBJ_AFFECT;i++) {
      food->affected[i].location = 0;
      food->affected[i].modifier = 0;
    }
    i=number(1,12);
    if(i==6)
      food->obj_flags.value[3]=1;
    if(i==7){
      food->affected[0].location = 29;
      food->affected[0].modifier = 103;
    }

    obj_to_room(food,ch->in_room);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_fire(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *weapon, *missile;
  char arg[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
  int rng, dr;
  struct char_data *targ;

  if (!can_do(ch, SKILL_ARCHERY))
    return;

  half_chop(argument, arg, arg2);
  if (!*arg) {
    send_to_char("Fire at whom?\n\r", ch);
    return;
  }
  if(*arg2)
    {
      if(is_abbrev(arg2, "north"))
	dr = 0;
      else if (is_abbrev(arg2, "east"))
	dr = 1;
      else if (is_abbrev(arg2, "south"))
	dr = 2;
      else if (is_abbrev(arg2, "west"))
	dr = 3;
      else if (is_abbrev(arg2, "up"))
	dr = 4;
      else if (is_abbrev(arg2, "down"))
	dr = 5;
      else
	dr = 6;
    }
  else
    dr = 6;

  weapon=ch->equipment[WIELD];
  if (!weapon || weapon->obj_flags.type_flag!=ITEM_FIREWEAPON) {
    send_to_char("You must be using a projectile weapon to fire.\n\r",ch);
    return;
  }

  if (!ch->equipment[LOADED]) {
    act("$p is not loaded.",TRUE,ch,weapon,0,TO_CHAR);
    return;
  }

  if(dr == 6)
    {
      if(!(targ=get_char_linear(ch, arg, &rng, &dr))) {
	send_to_char("There's nobody around by that name.\n\r", ch);
	return;
      }
    } else {
      if(!(targ=get_char_dir(ch, arg, &rng, dr)))
	{
	  send_to_char("There's nobody around by that name.\n\r", ch);
	  return;
	}
    }
  if(rng==0) {
    send_to_char("You can only fire at distant targets.\n\r", ch);
    return;
  }

  if(check_peaceful(targ,"")) {
    send_to_char("You can't fire into a peaceful room.\n\r", ch);
    return;
  }

  missile=unequip_char(ch, LOADED);

  if (number(1,101) < ch->skills[SKILL_ARCHERY].learned) {
    act("You fire $p!",FALSE,ch,weapon,0,TO_CHAR);
    act("$n fires $p!",TRUE,ch,weapon,0,TO_ROOM);
    throw_weapon(missile,dr,targ,ch, SKILL_ARCHERY);
  } else {
    act("You try to fire but clumsily drop $p.",FALSE,ch,missile,0,TO_CHAR);
    act("$n tries to fire but clumsily drops $p.",TRUE,ch,missile,0,TO_ROOM);
    obj_to_room(missile, ch->in_room);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

// this is an attempt to determine how long someone should be paralyzed by a particular act
int CalcStunDuration(struct char_data *ch, struct char_data *victim, int skill)
{
  const int strMod   = 1;
  const int dexMod   = 2;
  const int sizeMod  = 4;
  const int intMod   = 8;
  const int wisMod   = 16;
  const int conMod   = 32;

  int duration = 0;
  int durMod = 0;
  int diff = 0;

  switch(skill) {
  case SKILL_BASH :
    durMod += strMod +  sizeMod;
    break;
  case SKILL_STUN :
    durMod += strMod + dexMod;
    break;
  case SKILL_TRIP :
    //this should be based on the size of the char vs the victim
    //and a strength and dex modification
    durMod += dexMod + sizeMod + strMod;
    break;
  case SPELL_PARALYSIS :
    durMod += intMod + wisMod + conMod;
    break;
  case SKILL_PINCH :
    durMod += conMod + strMod;
    break;
  }

  if (durMod & strMod) //figure out strength modification
    {
      //For strength: difference in strength 1 round per 3 points and yes
      //negative numbers are okay...means you save a round due to strength
      diff = (GET_STR(ch) - GET_STR(victim)) / 3;
      duration += diff;
    }

  if (durMod & dexMod) //figure out dex modification
    {
      diff = (GET_DEX(ch) - GET_DEX(victim)) / 3;
      duration += diff;

    }

  if (durMod & sizeMod) //figure out size modification
    {
      diff = (GET_WEIGHT(ch) - GET_WEIGHT(victim))/20;
      duration += diff;
      diff = (GET_HEIGHT(ch) - GET_HEIGHT(victim))/50;
      duration += diff;
    }

  if (durMod & intMod) //figure out intel modification
    {
      diff = (GET_INT(ch) - GET_INT(victim))/3;
      duration += diff;
    }

  if (durMod & wisMod) //figure out Wisdom modification
    {
      diff = (GET_WIS(ch) - GET_WIS(victim))/3;
      duration += diff;
    }

  if (durMod & conMod) //figure out con modification
    {
      diff =(GET_CON(ch) - GET_CON(victim))/3;
      duration += diff;
    }
  duration = MAX(duration, 1);
  if (IsImmune(victim,IMM_HOLD))
     duration = duration/2;
  if (IsResist(victim, IMM_HOLD))
     duration /=4;
  if (IsSusc(victim, IMM_HOLD))
     duration = (int)duration * 1.5;

  return duration;
}
int stun_event(struct affected_type* aff, int now)
{
  if(!aff->caster || aff->duration <= 0 || ((aff->save_bonus <4) &&
					    ImpSaveSpell(aff->holder, SAVING_PARA, aff->save_bonus) ))
    {				/* recovery */
      struct char_data* opp;

      update_pos(aff->holder);

      if((opp = FindAnAttacker(aff->holder)))
	set_fighting(aff->holder, opp);

      affect_remove(aff->holder, aff);
    }
  else
    {				/* failure */
      aff->save_bonus--;
      aff->duration--;
      event_queue_pulse(&aff->timer,
			next_pulse(PULSE_VIOLENCE),
			(event_func) stun_event,
			NULL);
    }
  return 0;
}

void stun_opponent(struct char_data* ch,
		   struct char_data* victim,
		   int skill,
		   int bonus)
{
  struct affected_type *af;
  /* if already paralyzed, ignore all this */
  if(IS_SET(AFF_FLAGS(victim), AFF_PARALYSIS))
    {
      /* slapping around only wakes him up... */
      for(af = victim->affected ; af ; af = af->next)
	{
	  if(af->bitvector & AFF_PARALYSIS)
	    af->duration--;
	}
      return;
    }

  /* create and initialize the affect */
  CREATE(af, struct affected_type, 1);
  af->type = skill;
  af->duration = 0;
  af->save_bonus = 4;
  af->modifier = 0;
  af->location = 0;
  af->bitvector = AFF_PARALYSIS;
  af->mana_cost = 0;
  af->caster = ch;
  af->holder = victim;

  af->duration = CalcStunDuration(ch, victim, skill);
  /*    switch(skill) {
	case SKILL_BASH : af->duration = 1;
	break;
	case SKILL_TRIP : af->duration = 1;
	break;
	case SKILL_PINCH : af->duration = 1;
	break;
	case SKILL_STUN : af->duration = 2;
	break;
	}
  */
  /* put it in the affect list */
  af->next = victim->affected;
  victim->affected = af;

  /* actually apply it */
  affect_modify(victim, af->location, af->modifier, af->bitvector, TRUE);

  /* queue the event */
  event_queue_pulse(&af->timer,
		    next_pulse(PULSE_VIOLENCE),
		    (event_func) stun_event,
		    "stun");
}

/*
 * Touch this code and die... I fixed it now for the 50 billionth time....
 * Some idiot keeps modifying it to multiply chance of success by skill[stun].LEARNED
 * not LEARNED / 100 (so chance is multipled by 100)
 * its now fixed and based around a 30% success rate.
 * yes the players will complain... some idiot gave them 100% success stun for god knows
 * how long...
 * touch it and die -- Min
 */

void do_pinch(struct char_data *ch, char *argument, int command) {
  struct char_data *target;
  struct obj_data *wielded;
  int dam,w_type;

  if (!can_do(ch, SKILL_PINCH))
    return;

  if (!(target=set_target(ch, argument, SKILL_PINCH)))
    return;

  if (GET_MOVE(ch) < 20) {
    send_to_char ("You don't have enough energy left to perform this manuever.\n\r", ch);
    return;
  }

  if (ch->equipment[WIELD]) {
    send_to_char ("You can not pinch while wielding a weapon.\n\r", ch);
    return;
  }

  GET_MOVE(ch) -= 20;

#ifdef JANWORK
  percenthit = (30 + GET_DEX(ch) - GET_CON(target) + GetMaxLevel(ch) - GetMaxLevel(target))*ch->skills[SKILL_PINCH].learned;
  if (IsImmune(target,IMM_HOLD))
    percenthit = 0;
  if (IsResist(target,IMM_HOLD))
    percenthit = percenthit / 2;
  diceroll = number(0,99);
  if (diceroll<percenthit)
#endif
    //if (SkillChance(ch,target,30,IMM_HOLD,SPLMOD_DEX | SPLMOD_CON, SKILL_PINCH))
    if (!ImpSaveSpell(target, SAVING_PARA, 4))
      {
	act("$n stuns $N with a nerve pinch!",1,ch,0,target,TO_NOTVICT);
	act("$n stuns you with a nerve pinch!",1,ch,0,target,TO_VICT);
	act("You stun $N with a nerve pinch!",1,ch,0,target,TO_CHAR);
	stun_opponent(ch,target,SKILL_PINCH,4);
	target->specials.position = POSITION_STUNNED;
	if (!ch->specials.fighting)
	  set_fighting(ch,target);
	WAIT_STATE(ch,PULSE_VIOLENCE*2);
      }
    else
      {
	act("You fail to stun $N with a nerve pinch!",1,ch,0,target,TO_CHAR);
	act("$n just tried to stun you with a nerve pinch!",1,ch,0,target,TO_VICT);
	act("$n fails to stun $N with a nerve pinch!",1,ch,0,target,TO_NOTVICT);
	if (!target->specials.fighting) set_fighting(target,ch);
	if ((GET_POS(target)>POSITION_STUNNED)&&(!IS_AFFECTED(target,SKILL_PINCH)))
	  {
	    w_type = GetWeaponType (target,&wielded);
	    dam = GetWeaponDam (target,ch,wielded);
	    damage(target, ch, dam, w_type);
	  }
	WAIT_STATE(ch,PULSE_VIOLENCE*3);
      }
}


//Monk's fury skill.
/*With this skill, the monk is able to use his stamina reserves to
increase the number of attacks for one round, bringing the true fury on his foe!
However, there's always a chance that the monk fails to control his fury and
loses additional stamina*/
void do_fury(struct char_data *ch, char *argument, int command) {
  struct char_data *victim;
  int percent;

  if (!can_do(ch, SKILL_FURY))
    return;

  if (!(victim=set_target(ch, argument, SKILL_FURY)))
    return;

  if (GET_MOVE(ch) < 155)
  {
    send_to_char("Entering the state of fury with such low mobility would be too dangerous!\n\r.",ch);
    return;
  }

  if (ch->attackers > 5)
  {
    send_to_char("There is no room to enter a state of fury!\n\r", ch);
    return;
  }

  if (MOUNTED(ch))
  {
    send_to_char("That would be funny to see while you are mounted.\n\r", ch);
    return;
  }

  GET_MOVE(ch) -= 55;//the basic cost for trying to use this skill

  percent=number(1,101)+35;	/* 101% is a complete failure */
  percent -= dex_app[GET_DEX(ch)].reaction*5;
  percent += dex_app[GET_DEX(victim)].reaction*5;
  percent -= dex_app[GET_WIS(ch)].reaction*5;
  percent += dex_app[GET_WIS(victim)].reaction*5;
  percent += ((GetMaxLevel(victim)-GetMaxLevel(ch))*3);

  if (percent > ch->skills[SKILL_FURY].learned)
  {
    act("You enter the state of fury, but waste additional stamina!",
        FALSE, ch, 0, victim, TO_CHAR);
    GET_MOVE(ch) -= 100;
  }
  else
	act("You enter the state of fury against $N!", FALSE, ch, 0, victim, TO_CHAR);
  act("$n enters the state of fury!", FALSE, ch, 0, 0, TO_ROOM);
  act("$N Enters the state of fury against you!", FALSE, ch, 0, victim, TO_VICT);

  ch->specials.bonus_attks+=GetMaxLevel(ch)/12;
  ch->specials.bonus_attks+=MAX(ch->mult_att,1);

  if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
      (!victim->specials.fighting))
    set_fighting(victim, ch);

  WAIT_STATE(ch, PULSE_VIOLENCE*1);
}// Monk's fury skill end


void do_stun(struct char_data *ch, char *argument, int command) {
  struct char_data *target;
  struct obj_data *wielded;
  int dam,w_type;

  if (!can_do(ch, SKILL_STUN))
    return;

  if (!(target=set_target(ch, argument, SKILL_STUN)))
    return;

   if (GET_MOVE(ch) < 20) {
    send_to_char ("You don't have enough energy left to perform this manuever.\n\r", ch);
    return;
  }
  GET_MOVE(ch) -= 20;

#ifdef JANWORK
  percenthit = (30 + GET_DEX(ch) - GET_CON(target) + GetMaxLevel(ch) - GetMaxLevel(target))*ch->skills[SKILL_STUN].learned;
  if (IsImmune(target,IMM_HOLD))
    percenthit = 0;
  if (IsResist(target,IMM_HOLD))
    percenthit = percenthit / 2;
  diceroll = number(0,99);
  if (diceroll<percenthit)
#endif
   // if (SkillChance(ch,target,30,IMM_HOLD,SPLMOD_DEX | SPLMOD_CON, SKILL_STUN))
    if (!ImpSaveSpell(target, SAVING_PARA, 4))
      {
	act("$n stuns $N!",1,ch,0,target,TO_NOTVICT);
	act("$n stuns you!",1,ch,0,target,TO_VICT);
	act("You stun $N!",1,ch,0,target,TO_CHAR);
	stun_opponent(ch,target,SKILL_STUN,4);
	target->specials.position = POSITION_STUNNED;
	if (!ch->specials.fighting)
	  set_fighting(ch,target);
	WAIT_STATE(ch,PULSE_VIOLENCE*2);
      }
    else
      {
	act("You fail to stun $N!",1,ch,0,target,TO_CHAR);
	act("$n just tried to stun you!",1,ch,0,target,TO_VICT);
	act("$n fails to stun $N!",1,ch,0,target,TO_NOTVICT);
	if (!target->specials.fighting) set_fighting(target,ch);
	if ((GET_POS(target)>POSITION_STUNNED)&&(!IS_AFFECTED(target,SKILL_STUN)))
	  {
	    w_type = GetWeaponType (target,&wielded);
	    dam = GetWeaponDam (target,ch,wielded);
	    damage(target, ch, dam, w_type);
	  }
	WAIT_STATE(ch,PULSE_VIOLENCE*3);
      }
}

/* ============================== */


void do_animal_friendship(struct char_data *ch, char *argument,int command)
{
  struct char_data *target;

  if (!can_do(ch, SKILL_ANIMAL_FRIENDSHIP))
    return;

  if (!(target=set_target(ch, argument, SKILL_RESCUE)))
    return;

  if (circle_follow(target, ch)) {
    send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
    return;
  }

  if(!CountFollowers(ch))
    return;

  switch (target->race) {
  case RACE_REPTILE:
  case RACE_INSECT:
  case RACE_AVIAN:
  case RACE_MARINE:
  case RACE_AMPHIBIAN:
  case RACE_CARNIVORE:
  case RACE_OMNIVORE:
    break;
  default: send_to_char ("Sorry, that's not an animal.\n\r",ch);
    return;
  }

  if (GET_POS(target)<=POSITION_SLEEPING) {
    send_to_char("They are not paying any attention to you.\n\r", ch);
    return;
  }

  if (GetMaxLevel(ch)<GetMaxLevel(target)) {
    send_to_char ("This beast is too powerful for you to befriend!\n\r",ch);
    return;
  }

  if (IS_AFFECTED(target,AFF_CHARM)) {
    send_to_char ("That creature is already someone's friend!\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_ANIMAL_FRIENDSHIP].learned<number(1,101) ||
      saves_spell(target, SAVING_SPELL, IMM_CHARM)) {
    charge_mana(ch, SKILL_ANIMAL_FRIENDSHIP, FALSE);
    if (number(1,100)<=GetMaxLevel(target)) {
      act ("You try to make friends with $N but get attacked for your efforts!",TRUE,ch,0,target,TO_CHAR);
      act ("$n puts out $s hands to make friends with $N but gets attacked!",TRUE,ch,0,target,TO_ROOM);
      AddHated(target, ch);
      if (!target->specials.fighting)
        set_fighting(target, ch);
    } else {
      act ("You try to make friends with $N but are unsuccesful.",TRUE,ch,0,target,TO_CHAR);
      act ("$n puts out $s hands to make friends with $N but is unsuccesful.",TRUE,ch,0,target,TO_ROOM);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  charge_mana(ch, SKILL_ANIMAL_FRIENDSHIP, TRUE);
  act ("You make friends with $N!",TRUE,ch,0,target,TO_CHAR);
  act ("$n puts out $s hands to make friends with $N and is successful!",TRUE,ch,0,target,TO_ROOM);

  if (target->master)
    stop_follower(target);

  add_follower (target,ch, 0);
  fix_mob_bits (target);
  SET_BIT(AFF_FLAGS(target), AFF_CHARM);

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

/*** paladin skills ***/

void do_heroic_rescue(struct char_data *ch, char *arguement, int command)
{
  struct char_data *dude, *enemy;

  if (!can_do(ch, SKILL_HEROIC_RESCUE))
    return;

  for (dude=real_roomp(ch->in_room)->people;dude && !(dude->specials.fighting);dude=dude->next_in_room);
  if (!dude)
    {
      send_to_char("But there is no battle here!?!?\n\r",ch);
      return;
    }

  if (GET_MOVE(ch)<10) {
    send_to_char("You are too low on mobility to attempt a rescue.\n\r", ch);
    return;
  }

  if (ch->skills[SKILL_HEROIC_RESCUE].learned<number(1,101)) {
    GET_MOVE(ch) -= 5;
    send_to_char ("You try to plow your way to the front of the battle but stumble.\n\r",ch);
  } else {
    GET_MOVE(ch) -= 10;
    act("$n leaps heroically to the front of the battle!",FALSE,ch,0,0,TO_ROOM);
    send_to_char("You heroically leap to the front of the battle!\n\r", ch);

    for (enemy=real_roomp(ch->in_room)->people;enemy;enemy=enemy->next_in_room)
      if(enemy != ch && 			/* if it isn't me */
	 (dude = enemy->specials.fighting) &&	/* is fighting */
	 (dude != ch) &&			/* isn't fighting me */
	 !in_group(ch, enemy) &&		/* not in my group */
	 in_group(dude, ch))		/* fighting my group */
	{
	  int align;

	  act("$n leaps to your rescue, you are confused!",
	      TRUE,ch,0,dude,TO_VICT);
	  act("You rescue $N!",TRUE,ch,0,dude,TO_CHAR);
	  act("$n rescues $N!",TRUE,ch,0,dude,TO_NOTVICT);
	  stop_fighting(dude);
	  stop_fighting(enemy);
	  set_fighting (enemy,ch);

	  /* figure relative alignments */
	  align = GET_ALIGNMENT(dude) - GET_ALIGNMENT(enemy) / 2;
	  GET_ALIGNMENT(ch) =
	    MAX(-1000, MIN(1000, GET_ALIGNMENT(ch) + (align / 50)));

	  WAIT_STATE (dude,2*PULSE_VIOLENCE);
	}
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_blessing(struct char_data *ch, char *argument, int cmd)
{
  int rating,factor,level,mana;
  struct char_data *test, *dude;

  if (!can_do(ch, SKILL_BLESSING))
    return;

  if (!(dude=set_target(ch, argument, SKILL_BLESSING)))
    return;

  mana = GetMaxLevel(ch);

  if (ch->specials.alignment<350) {
    send_to_char ("Your diety doesn't like the way you've behaved.\n\r",ch);
    return;
  }

  if (dude->specials.alignment<0) {
    send_to_char ("That person is too evil to deserve blessing.\n\r",ch);
    return;
  }

  if (GET_MANA(ch) < mana) {
    send_to_char("You haven't the spiritual resources to do that now.\n\r",ch);
    return;
  }

  if (number(1,101)>ch->skills[SKILL_BLESSING].learned) {
    send_to_char ("You fail in the bestowal of your blessing\n\r",ch);
    GET_MANA(ch) -= (mana/2);
  } else {
    GET_MANA(ch) -= mana;
    factor=0;
    if (ch==dude)
      factor++;
    if (dude->specials.alignment > 350)
      factor++;
    if (dude->specials.alignment == 1000)
      factor++;
    level = ch->player.level[PALADIN_LEVEL_IND];
    rating = (level * ch->specials.alignment) / 1000 + factor;
    factor=0;
    for (test=real_roomp(ch->in_room)->people;test;test=test->next_in_room)
      {
	if (test!=ch && in_group(ch, test))
	  factor++;
      }
    rating += MIN(factor,3);
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
	    send_to_char("There's no one in your group to bless\n\r",ch);
	    return;
	  }
	if (!(affected_by_spell(dude,SPELL_BLESS)))
	  spell_bless(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>1)
	  if (!(affected_by_spell(dude,SPELL_ARMOR)))
	    spell_armor(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>4)
	  if (!(affected_by_spell(dude,SPELL_STRENGTH)))
	    spell_strength(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>6)
	  spell_refresh(level,ch,SPELL_TYPE_SPELL, dude,0);
	if (rating>9)
	  if (!(affected_by_spell(dude,SPELL_SENSE_LIFE)))
	    spell_sense_life(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>14)
	  if (!(affected_by_spell(dude,SPELL_TRUE_SIGHT)))
	    spell_true_seeing(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>19)
	  spell_cure_critic(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>24)
	  if (!(affected_by_spell(dude,SPELL_SANCTUARY)))
	    spell_sanctuary(level,ch,SPELL_TYPE_SPELL,dude,0);
	if(rating>29)
	  spell_heal(level,ch,SPELL_TYPE_SPELL,dude,0);
	if(rating>34)
	  {
	    spell_remove_poison(level,ch,SPELL_TYPE_SPELL,dude,0);
	    spell_remove_paralysis(level,ch,SPELL_TYPE_SPELL,dude,0);
	  }
	if (rating>39)
	  spell_heal(level,ch,SPELL_TYPE_SPELL,dude,0);
	if (rating>44)
	  {
	    if (dude->specials.conditions[2] != -1)
	      dude->specials.conditions[2] = 24;
	    if (dude->specials.conditions[1] != -1)
	      dude->specials.conditions[1] = 24;
	  }
	if (rating>54)
	  {
	    spell_heal(level,ch,SPELL_TYPE_SPELL,dude,0);
	    send_to_char ("An awesome feeling of holy power overcomes you!\n\r",dude);
	  }
	act ("$n blesses $N!",TRUE,ch,0,dude,TO_NOTVICT);
	act ("You bless $N!",TRUE,ch,0,dude,TO_CHAR);
	act ("$n blesses you!",TRUE,ch,0,dude,TO_VICT);
	update_pos (dude);
      }
  }
  WAIT_STATE(ch,PULSE_VIOLENCE*1);
}

void do_lay_on_hands (struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  int wounds, healing;

  if (!can_do(ch, SKILL_LAY_ON_HANDS))
    return;

  if (!(victim=set_target(ch, argument, SKILL_LAY_ON_HANDS)))
    return;

  if (ch->specials.alignment < 350) {
    send_to_char("You better shape up or no help from your diety.\n\r",ch);
    return;
  }

  wounds=victim->points.max_hit-victim->points.hit;
  if (!wounds) {
    send_to_char("Don't try to heal what ain't hurt!\n\r",ch);
    return;
  }

  if (ch->skills[SKILL_LAY_ON_HANDS].learned<number(1,101)) {
    send_to_char("You never did get your First Aid Merit Badge did you?\n\r",ch);
    charge_mana(ch, SKILL_LAY_ON_HANDS, FALSE);
  } else {
    charge_mana(ch, SKILL_LAY_ON_HANDS, TRUE);
    act ("$n lays hands on $N.",FALSE,ch,0,victim, TO_NOTVICT);
    act ("You lay hands on $N.",FALSE,ch,0,victim,TO_CHAR);
    act ("$n lays hands on you.",FALSE,ch,0,victim,TO_VICT);

    if (victim->specials.alignment<0) {
      act ("You are too evil to benefit from this treatment.",FALSE,ch,0,victim,TO_VICT);
      act ("$n is too evil to benefit from this treatment.",FALSE,victim,0,ch,TO_ROOM);
    } else {
      if (victim->specials.alignment<350)
	healing = ch->player.level[PALADIN_LEVEL_IND];
      else
	healing = ch->player.level[PALADIN_LEVEL_IND]*3;

      if (healing>wounds)
	victim->points.hit=victim->points.max_hit;
      else
	victim->points.hit +=healing;
      update_pos (victim);
    }
  }
  WAIT_STATE (ch,PULSE_VIOLENCE*1);
}



void do_holy_warcry (struct char_data *ch, char *argument, int cmd)
{
  int dam, dif,level;
  struct char_data *dude=NULL;
  double c1, c2;

  if (!can_do(ch, SKILL_HOLY_WARCRY))
    return;

  while(*argument == ' ') argument++;
  if(!strcasecmp(argument, "all")) {
    send_to_char("Your lungs are way too small for that kind of attack.\n\r", ch);
    return;
  }

  if (!(dude=set_target(ch, argument, SKILL_HOLY_WARCRY)))
    return;

  c1 = pow(4.0, GET_ALIGNMENT(ch) / 1000.0);
  c2 = pow(4.0,-GET_ALIGNMENT(dude) / 1000.0);
  if(c1>0) c1 /= 2.0;
  if(c2>0) c2 /= 2.0;

  if (ch->skills[SKILL_HOLY_WARCRY].learned<number(1,101)) {
    charge_mana(ch, SKILL_HOLY_WARCRY, FALSE);
    send_to_char("Your mighty warcry emerges from your throat as a tiny squeak.\n\r",ch);
  } else {
    charge_mana(ch, SKILL_HOLY_WARCRY, TRUE);

    dif=((level=GetMaxLevel(ch))-GetMaxLevel(dude));

    if ((100 - 5*dif) < number(1,101))
      {
	spell_paralyze(0,ch,SPELL_TYPE_SPELL,dude,0);
      }

    dam = dice(level, 10);
    dam = (int)((double)dam*c1*c2);

    damage(ch, dude, dam, SKILL_HOLY_WARCRY);
  }

  WAIT_STATE (ch,PULSE_VIOLENCE*2);
}

/***
 *** skills learnable by multiple classes
 ***/

void do_feint(struct char_data *ch, char *argument, int cmd)
{
  int percent, attackloss, attackloss1;
  struct char_data *victim;

  if (!can_do(ch, SKILL_FEINT))
    return;

  if (!(victim=set_target(ch, argument, SKILL_FEINT)))
    return;

  if (!ch->equipment[WIELD]) {
    send_to_char("You need a weapon first with which to feint.\n\r", ch);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough mobility to try and feint\n\r.",ch);
    return;
  }

  if (ch->attackers > 5) {
    send_to_char("There is no room to feint!\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("That would be funny to see while you are mounted.\n\r", ch);
    return;
  }

  percent=number(1,101)+25;	/* 101% is a complete failure */
  percent -= dex_app[GET_DEX(ch)].reaction*5;
  percent += dex_app[GET_DEX(victim)].reaction*5;
  percent += ((GetMaxLevel(victim)-GetMaxLevel(ch))*3);

  if (percent > ch->skills[SKILL_FEINT].learned) {
    GET_MOVE(ch) -= 5;
    act("You try to make a feinting attack but fail miserably.",
        FALSE, ch, 0, victim, TO_CHAR);
    act("$n executes an odd fighting move.", FALSE, ch, 0, 0, TO_ROOM);
    act("You see an openining in $n's defenses!.",
	FALSE, ch, 0, victim, TO_VICT);
    victim->specials.bonus_attks+=MAX(victim->mult_att/2,1);
    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
        (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  } else {
    GET_MOVE(ch) -= 15;
    act("$n makes a feinting attack.", FALSE, ch, 0, 0, TO_ROOM);
    act("You make a feinting attack and confuse $N.",
	FALSE, ch, 0, victim, TO_CHAR);
    act("$N deftly dodges your attack. You are on the defensive!",
	FALSE, ch, 0, victim, TO_VICT);
    WAIT_STATE(victim, PULSE_VIOLENCE*2);

    attackloss = victim->specials.bonus_attks;
    victim->specials.bonus_attks-=MAX(victim->mult_att/2,1);
    if (victim->specials.bonus_attks < 0)
      victim->specials.bonus_attks=0;
    attackloss1 = attackloss - victim->specials.bonus_attks;
  }
  if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
      (!victim->specials.fighting)) {
    set_fighting(victim, ch);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*1);

}

void do_disarm(struct char_data *ch, char *argument, int cmd)
{
  int percent;
  struct char_data *victim;
  struct obj_data *w;

  if (!can_do(ch, SKILL_DISARM))
    return;

  if (!(victim=set_target(ch, argument, SKILL_DISARM)))
    return;

  if (!ch->equipment[WIELD]) {
    send_to_char("You need a weapon for this maneuver.\n\r", ch);
    return;
  }

  if (GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough mobility to try and disarm.\n\r",ch);
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There is no room to disarm!\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("That would be funny to see while you are mounted.\n\r", ch);
    return;
  }

  percent = (GetMaxLevel(victim)-GetMaxLevel(ch));
  if(percent < 0) {
    percent /= 3;
  } else {
    percent *= 3;
  }
  percent += number(1,101);
  percent -= dex_app[GET_DEX(ch)].reaction*10;
  percent += dex_app[GET_DEX(victim)].reaction*10;
  /*
    if (GetMaxLevel(victim) > 20) {
    percent += ((GetMaxLevel(victim)-18) * 4);
    }
  */
  if (percent > ch->skills[SKILL_DISARM].learned) {
    GET_MOVE(ch) -= 5;
    act("You try to disarm $N, but fail miserably.",
        FALSE, ch, 0, victim, TO_CHAR);
    act("$n does a nifty fighting move, but then falls on $s butt.",
        FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
        (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  } else {
    GET_MOVE(ch) -= 10;
    if (victim->equipment[WIELD]) {
      w = unequip_char(victim, WIELD);
      act("$n makes an impressive fighting move.", FALSE, ch, 0, 0, TO_ROOM);
      act("You send $p flying from $N's grasp.", FALSE, ch, w, victim, TO_CHAR);
      act("$p flies from your grasp.", FALSE, ch, w, victim, TO_VICT);
      obj_to_room(w, victim->in_room);
      WAIT_STATE(victim, PULSE_VIOLENCE*2);
      if (victim->equipment[LOADED]) {
        w=unequip_char(victim, LOADED);
        obj_to_room(w, victim->in_room);
      }
    } else {
      act("You try to disarm $N, but $E doesn't have a weapon.",
          FALSE, ch, 0, victim, TO_CHAR);
      act("$n makes an impressive fighting move, but does little more.",
          FALSE, ch, 0, 0, TO_ROOM);
    }
    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
        (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*1);

  }
}

void do_track(struct char_data *ch, char *argument, int cmd)
{
  char name[256], buf[256];
  int dist;
  int flags;

#if NOTRACK
  send_to_char("Sorry, tracking is disabled. Try again after reboot.\n\r",ch);
  return;
#endif

  act("$n kneels upon one knee and looks for traces of $s quarry.",TRUE,ch,0,0,TO_ROOM);
  send_to_char("You kneel upon one knee and look for traces of your quarry.\n\r", ch);


  only_argument(argument, name);

  if(!get_char(name))
    {
      send_to_char("You are unable to find traces of one.\n\r", ch);
      return;
    }

  if(!IS_LIGHT(real_roomp(ch->in_room)))
    {
      send_to_char("It's much too dark in here to track...\n\r", ch);
      return;
    }

  if (IS_GOD(ch))
    dist = MAX_ROOMS;
  else if (affected_by_spell(ch, SPELL_MINOR_TRACK))
    dist = GetMaxLevel(ch) * 50;
  else if (affected_by_spell(ch, SPELL_MAJOR_TRACK))
    dist = GetMaxLevel(ch) * 100;
  else
    {
      if (!ch->skills)
	dist = 10;
      else
	dist = ch->skills[SKILL_HUNT].learned;

      if (IS_SET(ch->player.clss, CLASS_THIEF))
	dist *= 3;
      else if (IS_SET(ch->player.clss, CLASS_RANGER))
	dist *= 10;
      else if (IS_SET(ch->player.clss, CLASS_WARRIOR))
	dist *= 2;
      else if (IS_SET(ch->player.clss, CLASS_MAGIC_USER))
	dist += GET_LEVEL(ch, MAGE_LEVEL_IND);
      else if (IS_SET(ch->player.clss, CLASS_DRUID))
	dist += GET_LEVEL(ch, DRUID_LEVEL_IND);

      switch(GET_RACE(ch)){
      case RACE_CANIS:
      case RACE_FELIS:
	dist = dist * 5 / 2;
	break;
      case RACE_ELF:
	dist *= 2;		/* even better */
	break;
      case RACE_UNDERWORLD:
	dist = MAX_ROOMS;	/* as good as can be */
	break;
      default:
	break;
      }
    }

  flags = ((GetMaxLevel(ch) >= MIN_GLOB_TRACK_LEV) ||
	   affected_by_spell(ch, SPELL_MINOR_TRACK) ||
	   affected_by_spell(ch, SPELL_MAJOR_TRACK)) & HUNT_GLOBAL;

  ch->hunt_info =
    path_to_name(ch->in_room, name, dist, flags | HUNT_THRU_DOORS);

  if(!ch->hunt_info)
    {
      send_to_char("You are unable to find traces of one.\n\r", ch);
    }
  else
    {
      sprintf(buf, "You see traces of your quarry to the %s\n\r",
	      dirs[path_dir(ch->in_room, ch->hunt_info)]);
      send_to_char(buf,ch);
    }

  WAIT_STATE(ch, PULSE_VIOLENCE*.6);
}

int track(struct char_data *ch)
{
  char buf[256];
  struct char_data* vict;
  int code;

  if(!ch) {
    sprintf(buf, "!ch returned true\r\n");
    return -1;
  }

  if(!ch->hunt_info) {
    sprintf(buf, "!ch->hunt_info returned true\r\n");
    return -1;
  }

  vict = ch->hunt_info->victim;

  if (vict ? ch->in_room == vict->in_room
      : ch->in_room == ch->hunt_info->dest)
    {
      send_to_char("##You have found your target!\n\r",ch);
    killtrack:
      if(ch->hunt_info)
	{
	  path_kill(ch->hunt_info);
	  ch->hunt_info = 0;
	}
      return -1;
    }

  if((code = path_dir(ch->in_room, ch->hunt_info)) == -1)
    {
      send_to_char("##You have lost the trail.\n\r",ch);
      goto killtrack;
    }

  sprintf(buf, "##You see a faint trail to the %s\n\r", dirs[code]);
  send_to_char(buf, ch);

  return code;
}


int go_direction(struct char_data *ch, int dir, int tracking)
{
  struct room_data *new_room;
  if (ch->specials.fighting)
    return 0;
  if (!IS_PC(ch))
    {
      new_room=real_roomp(real_roomp(ch->in_room)->dir_option[dir]->to_room);
      if (!tracking && IS_SET(new_room->room_flags, NO_MOB))
	return 0;
      if (IS_SET(ch->specials.mob_act, ACT_STAY_ZONE) &&
	  new_room->zone != real_roomp(ch->in_room)->zone)
	return 0;
    }
  if (!IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED))
    {
      do_move(ch, "", dir+1);
      return 1;
    }
  else if ( IsHumanoid(ch) && !IS_SET(EXIT(ch,dir)->exit_info, EX_LOCKED) )
    {
      open_door(ch, dir);
      return 1;
    }
  return 0;
}

/*** doorbash procs ***/

void raw_unlock_door( struct char_data *ch,
		      struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;
  char buf[128];

  REMOVE_BIT(exitp->exit_info, EX_LOCKED);
  /* now for unlocking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
  } else {
    sprintf(buf, "Inconsistent door locks in rooms %ld->%ld",
	    ch->in_room, exitp->to_room);
    log_msg(buf);
  }
}

void raw_open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back = NULL;
  struct room_data	*rp;
  char	buf[MAX_INPUT_LENGTH];

  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    sprintf(buf, "NULL rp in open_door() for %s.", PERS(ch,ch));
    log_msg(buf);
  }

  exitp = rp->dir_option[dir];

  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))    {
    REMOVE_BIT(back->exit_info, EX_CLOSED);
    if (back->keyword && (strcmp("secret", fname(back->keyword))))	{
      sprintf(buf, "The %s is opened from the other side.\n\r",
	      fname(back->keyword));
      send_to_room(buf, exitp->to_room);
    } else {
      send_to_room("The door is opened from the other side.\n\r",
		   exitp->to_room);
    }
  }
}

void slam_into_wall( struct char_data *ch, struct room_direction_data *exitp)
{
  char doorname[128];
  char buf[256];

  if (exitp->keyword && *exitp->keyword) {
    if ((strcmp(fname(exitp->keyword), "secret")==0) ||
	(IS_SET(exitp->exit_info, EX_SECRET))) {
      strcpy(doorname, "wall");
    } else {
      strcpy(doorname, fname(exitp->keyword));
    }
  } else {
    strcpy(doorname, "barrier");
  }
  sprintf(buf, "You slam against the %s with no effect\n\r", doorname);
  send_to_char(buf, ch);
  send_to_char("OUCH!  That REALLY Hurt!\n\r", ch);
  sprintf(buf, "$n crashes against the %s with no effect\n\r", doorname);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  GET_HIT(ch) -= number(1, 10)*2;
  if (GET_HIT(ch) < 0)
    GET_HIT(ch) = 0;
  GET_POS(ch) = POSITION_STUNNED;
  return;
}

void do_doorbash( struct char_data *ch, char *arg, int cmd)
{
  int dir;
  struct room_direction_data *exitp;
  int was_in, roll;
  char buf[256], type[MAX_INPUT_LENGTH], direction[MAX_INPUT_LENGTH];

  if (!can_do(ch, SKILL_DOORBASH))
    return;

  if (GET_MOVE(ch) < 10) {
    send_to_char("You don't have enough mobility to doorbash.\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("Yeah right... while mounted.\n\r", ch);
    return;
  }

  for (;*arg == ' '; arg++);
  argument_interpreter(arg, type, direction);

  if ((dir = find_door(ch, type, direction)) < 0) {
    act("$n looks around, bewildered.", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  exitp = EXIT(ch, dir);
  if (!exitp) {
    send_to_char("You shouldn't have gotten here.\n\r", ch);
    return;
  }

  if (dir == UP) {
    if (real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
	!IS_AFFECTED(ch, AFF_FLYING)) {
      send_to_char("You have no way of getting there!\n\r", ch);
      return;
    }
  }

  sprintf(buf, "$n charges %swards.", dirs[dir]);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "You charge %swards.\n\r", dirs[dir]);
  send_to_char(buf, ch);

  if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
    was_in = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, exitp->to_room);
    do_look(ch, "", 0);

    DisplayMove(ch, dir, was_in, 1);
    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
	!IS_GOD(ch)) {
      do_death_trap(ch);
      return;
    } else {
      WAIT_STATE(ch, PULSE_VIOLENCE*3);
      GET_MOVE(ch) -= 10;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    GET_MOVE(ch) -= 10;
    return;
  }

  GET_MOVE(ch) -= 10;

  if (IS_SET(exitp->exit_info, EX_LOCKED) &&
      IS_SET(exitp->exit_info, EX_PICKPROOF)) {
    slam_into_wall(ch, exitp);
    return;
  }

  /*
    now we've checked for failures, time to check for success;
  */
  if (ch->skills) {
    if (ch->skills[SKILL_DOORBASH].learned) {
      roll = number(1, 100);
      if (roll > ch->skills[SKILL_DOORBASH].learned) {
	slam_into_wall(ch, exitp);
	LearnFromMistake(ch, SKILL_DOORBASH, 0, 90);
      } else {
	/*
	  unlock and open the door
	*/
	sprintf(buf, "$n slams into the %s, and it bursts open!",
		fname(exitp->keyword));
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	sprintf(buf, "You slam into the %s, and it bursts open!\n\r",
		fname(exitp->keyword));
	send_to_char(buf, ch);
	raw_unlock_door(ch, exitp, dir);
	raw_open_door(ch, dir);
	GET_HIT(ch) -= number(1,5);
	/*
	  Now a dex check to keep from flying into the next room
	*/
	roll = number(1, 20);
	if (roll > GET_DEX(ch)) {
	  was_in = ch->in_room;

	  char_from_room(ch);
	  char_to_room(ch, exitp->to_room);
	  do_look(ch, "", 0);
	  DisplayMove(ch, dir, was_in, 1);
	  if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
	      !IS_GOD(ch)) {
	    do_death_trap(ch);
	    return;
	  }
	  WAIT_STATE(ch, PULSE_VIOLENCE*3);
	  GET_MOVE(ch) -= 10;
	  return;
	} else {
	  WAIT_STATE(ch, PULSE_VIOLENCE*1);
	  GET_MOVE(ch) -= 5;
	  return;
	}
      }
    } else {
      send_to_char("You just don't know the nuances of door-bashing.\n\r", ch);
      slam_into_wall(ch, exitp);
      return;
    }
  } else {
    send_to_char("You're just a goofy mob.\n\r", ch);
    return;
  }
}

void do_shield_punch(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  struct obj_data *shield;
  int dam=0, weight_obj=0, weight_diff=0,level=0;

  if(!can_do(ch, SKILL_SHIELD_PUNCH))
    return;

  if (!(vict=set_target(ch, arg, SKILL_SHIELD_PUNCH)))
    return;

  if (vict->in_room != ch->in_room) {
    DLOG(("do_shield_punch found %s fighting mob that's gone",GET_NAME(ch)));
    send_to_char("It seems they didn't wait around for you to punch them...\n\r", ch);
    return;
  }

  /* DEBUG CODE */

  if (ch && vict) {
    DLOG(("Shield Punch:  ch = %s, vict = %s, arg = %s", GET_IDENT(ch), GET_IDENT(vict), arg));
  } else {
    DLOG(("Shield Punch without char or vict."));
    return;
  }

  /* END DEBUG CODE */


  if (!ch->equipment[WEAR_SHIELD])
    {
      send_to_char("How can you do a shield punch without a shield!?  \n\r",ch);
      return;
    }

  shield = ch->equipment[WEAR_SHIELD];
  level = GetMaxLevel(ch);
  weight_obj = shield->obj_flags.weight;
  weight_diff = MAX(ch->player.weight - vict->player.weight, 1);


  /* DEBUG CODE */
  DLOG(("Second arg of call to function dice is: %d", (weight_obj + weight_diff)));
  /* END DEBUG CODE */

  dam = dice((int)(level/2), (weight_obj + weight_diff));

  if (SkillChance(ch,vict,10,IMM_BLUNT,SPLMOD_DEX | SPLMOD_STR | SPLMOD_AC,
		  SKILL_SHIELD_PUNCH))
    {
      act(show_dam_check(ch,"$CrPOW!$CN What a $CyPUNCH!$CN",dam),TRUE,ch, 0, vict, TO_CHAR);
      act("$n hits $N with $s shield! $CrPOW!$CN",TRUE, ch, 0, vict, TO_NOTVICT);
      act("$n just $CyPUNCHED$CN you with $s shield! $CrPOW!$CN",TRUE,ch,0,vict, TO_VICT);
      GET_MOVE(ch) -= 20;
      if (!damage(ch, vict, dam, SKILL_SHIELD_PUNCH))  // NOT  DEAD
	if(!ch->specials.fighting)
	  set_fighting(ch, vict);
    }
  else
    {
      GET_MOVE(ch) -= 10;
      act(show_dam_check(ch,"$CgYou tried to punch $N with your shield but you failed!$CN",0),FALSE,ch, 0, vict, TO_CHAR);
      act("$Cg$n tried to punch $N with $s shield, but failed!$CN",FALSE,ch, 0, vict, TO_NOTVICT);
      act("$Cg$n just tried to punch YOU!$CN",FALSE, ch, 0, vict, TO_VICT);
      if (!ch->specials.fighting)
	set_fighting(ch,vict);
    }

  WAIT_STATE(ch, PULSE_VIOLENCE);
}



/*** shapeshifter skills ***/

void do_form(struct char_data *ch, char *argument, int cmd)
{
  static struct form_stuff
  {
    char *name;
    int att_type;
  } forms[] =
    {
      { "hammers", TYPE_CRUSH },
      { "blades",  TYPE_SLASH },
      { "lances",  TYPE_PIERCE },
      { "claws",   TYPE_CLAW },
      { 0,         0 }
    };
  int i;
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct form_stuff* ptr;

  only_argument(argument,name);

  if (!*name || affected_by_spell(ch, SKILL_FORM)) {
    if(cmd>=0)
      {
	send_to_char("Your arms and hands melt and shift, returning to normal.\n\r", ch);
	act("$n's arms and hands melt and shift, returning to normal.",TRUE,ch,0,0,TO_ROOM);
	affect_from_char(ch, SKILL_FORM);
      }
    ch->specials.attack_type=TYPE_HIT;
    return;
  }

  if (!can_do(ch, SKILL_FORM))
    return;

  if(is_abbrev(name, "help")) {
    send_to_char("The following forms are possible:\n\r",ch);
    for(ptr = forms ; ptr->name ; ptr++) {
      sprintf(buf, "%s\n\r", ptr->name);
      send_to_char(buf, ch);
    }
    return;
  }

  for(ptr=forms ; ptr->name ; ptr++) {
    if(is_abbrev(name, ptr->name))
      break;
  }

  if(!ptr->name) {
    send_to_char("Sorry, that is not an available form.\n\r",ch);
    send_to_char("Try typing \"form help\" to get a listing.\n\r",ch);
    return;
  }

  if (!HasHands(ch)) {
    send_to_char("You grow some arms and hands to form into weapons...\n\r", ch);
    act("$n grows some arms and hands to form into weapons...",TRUE,ch,0,0,TO_ROOM);
  }

  if (number(1,101)>ch->skills[SKILL_FORM].learned) {
    send_to_char("You try to form your arms and hands but lose your concentration.\n\r",ch);
    act("$n tries to form $s arms and hands but loses $s concentration.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_FORM, FALSE);
    return;
  }

  charge_mana(ch, SKILL_FORM, TRUE);

  if (affected_by_spell(ch, SKILL_FORM)) {
    do_form(ch, "", -1);
  }

  for (i=0; i<MAX_WEAR; i++) {
    if (shifter_form_test(i) && ch->equipment[i])
      obj_to_char(unequip_char(ch, i), ch);
  }

  MakeAffect(ch, ch, SPELL_TYPE_SPELL,
	     SKILL_FORM, MAX(1, GetMaxLevel(ch)/10), APPLY_BHD, 0,
	     0, 3, FALSE, FALSE, FALSE, do_form);
  ch->specials.attack_type=ptr->att_type;
  MakeAffect(ch, ch, SPELL_TYPE_SPELL,
	     SKILL_FORM, MAX(1, GetMaxLevel(ch)/10), APPLY_HITNDAM, 0,
	     0, 2, TRUE, FALSE, FALSE, do_form);

  sprintf(buf, "Your arms and hands melt and shift, reforming as deadly %s!\n\r", ptr->name);
  send_to_char(buf, ch);
  sprintf(buf, "$n's arms and hands melt and shift, reforming as deadly %s!", ptr->name);
  act(buf,TRUE,ch,0,0,TO_ROOM);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_chameleon(struct char_data *ch, char *argument, int cmd)
{
  if (affected_by_spell(ch, SKILL_CHAMELEON)) {
    if (cmd>=0) {
      send_to_char("You become visible as your skin returns to normal.\n\r", ch);
      act("$n becomes visible as $s skin returns to normal.",TRUE,ch,0,0,TO_ROOM);
      affect_from_char(ch, SKILL_CHAMELEON);
    }
    return;
  }

  if (!can_do(ch, SKILL_CHAMELEON))
    return;

  if (number(1,101)>ch->skills[SKILL_CHAMELEON].learned) {
    send_to_char("You try to meld into the background but fail.\n\r",ch);
    act("$n tries to meld into the background but fails.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_CHAMELEON, FALSE);
    return;
  }

  charge_mana(ch, SKILL_CHAMELEON, TRUE);

  send_to_char("Your skin takes on chameleon properties and melds into the scenery.\n\r",ch);
  act("$n's skin takes on chameleon properties and melds into the scenery.",TRUE,ch,0,0,TO_ROOM);

  MakeAffect(ch, ch, SPELL_TYPE_SPELL,
	     SKILL_CHAMELEON, 0, 0, AFF_HIDE,
	     0, 5, FALSE, FALSE, FALSE, do_chameleon);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_limb(struct char_data *ch, char *argument, int cmd)
{
  if (affected_by_spell(ch, SKILL_LIMB)) {
    if(cmd>=0)
      {
	send_to_char("You reabsorb all your limbs.\n\r", ch);
	act("$n reabsorbs all $s limbs and they disappear.",TRUE,ch,0,0,TO_ROOM);
      }
    if (ch->equipment[WEAR_LIGHT])
      obj_to_char(unequip_char(ch, WEAR_LIGHT), ch);
    if (ch->equipment[WEAR_SHIELD])
      obj_to_char(unequip_char(ch, WEAR_SHIELD), ch);
    if (ch->equipment[WIELD])
      obj_to_char(unequip_char(ch, WIELD), ch);
    if (ch->equipment[HOLD])
      obj_to_char(unequip_char(ch, HOLD), ch);

    send_to_char("As your extra hands disappear and your normal hands reform, "
		 "your held items fall into your inventory.\n\r",ch);
    if(cmd>=0)
      affect_from_char(ch, SKILL_LIMB);

    return;
  }

  if (!can_do(ch, SKILL_LIMB))
    return;

  if (GET_LEVEL(ch,SHIFTER_LEVEL_IND) < 50)
    {

      if (number(1,101)>ch->skills[SKILL_LIMB].learned) {
	send_to_char("You try to grow an extra arm, but fail.", ch);
	act("$n tries to grow an extra arm, but fails.", TRUE,ch,0,0,TO_ROOM);
	charge_mana(ch,SKILL_LIMB, FALSE);
	return;
      }
      charge_mana(ch, SKILL_LIMB, TRUE);
      send_to_char("An extra arm protrudes from your body.\n\r",ch);
      act("An extra arm protrudes from $n's body.\n\r",TRUE,ch,0,0,TO_ROOM);
    }
  else
    {
      if (number(1,101) > (ch->skills[SKILL_LIMB].learned))
	{
	  send_to_char("You try to grow two extra arms, but fail.", ch);
	  act("$n tries to grow two extra arms, but fails.",TRUE,ch,0,0,TO_ROOM);
	  charge_mana(ch,SKILL_LIMB,TRUE);
	  return;
	}
      charge_mana(ch, SKILL_LIMB, TRUE);
      charge_mana(ch, SKILL_LIMB, TRUE);
      send_to_char("Two extra arms protrude from your body.\n\r",ch);
      act("Two extra arms protrude from $n's body.\n\r",TRUE,ch,0,0,TO_ROOM);
    }
  MakeAffect(ch, ch, SPELL_TYPE_SPELL,
	     SKILL_LIMB, 0, 0, 0,
	     0, 5, TRUE, FALSE, FALSE, do_limb);
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_melt(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *mob, *temp_ch;
  struct obj_data *obj, *nextobj;
  char buf[256];

  if (!IS_PC(ch))
    return;

  if (!can_do(ch, SKILL_MELT))
    return;

  if (IS_SET(ch->specials.mob_act, ACT_LIQUID)) {
    send_to_char("You are already in a liquid state.\n\r", ch);
    return;
  }

  if (number(1,101) > ch->skills[SKILL_MELT].learned) {
    send_to_char("You attempt to liquify but lose your concentration.\n\r", ch);
    act("$n tries to liquify but loses $s concentration.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_MELT, FALSE);
    return;
  }

  if (!(mob=make_mobile(VMOB_68, VIRTUAL))) {
    log_msg("screwup in melt skill.");
    send_to_char("This skill is experiencing technical difficulty.\n\r", ch);
    return;
  }

  if (ch->specials.fighting)
    stop_fighting(ch);

  shifter_normalize(ch);

  if (IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
    temp_ch=ch->orig;
    do_return(ch, "", -1);  /* cmd=-1 indicates no messages from do_return */
    ch=temp_ch;
  }

  charge_mana(ch, SKILL_MELT, TRUE);

  if (ch->master)
    stop_follower(ch);
  stop_all_followers(ch);

  char_to_room(mob, ch->in_room);
  SwitchStuff(ch, mob);

  send_to_char("Your flesh liquifies and melts into a formless pool!\n\r",ch);
  act("$n's flesh liquifies and melts into a formless pool!",TRUE,ch,0,0,TO_ROOM);

  for(obj = mob->carrying; obj; obj = nextobj) { /* dump any items on  */
    nextobj = obj->next_content;                 /* the player in room */
    obj_from_char(obj);
    obj_to_room(obj, mob->in_room);
  }
  if (GET_GOLD(mob)>0) {
    obj_to_room(create_money(GET_GOLD(mob)), mob->in_room);
    GET_GOLD(mob)=0;
  }

  /* put the char's original form in storage */
  char_from_room(ch);
  char_to_room(ch, 3);

  /* switch into the pool */
  push_character(ch, mob);

  /* combine names of pool and player */
  sprintf(buf,"%s %s", GET_IDENT(mob), GET_IDENT(ch));
  ss_free(mob->player.name);
  mob->player.name = ss_make(buf);

  /* set the polymorphed flag on the player */
  SET_BIT(mob->specials.mob_act, ACT_POLYSELF);
  SET_BIT(mob->specials.mob_act, ACT_SHIFTER);
  mob->specials.flags = ch->specials.flags;

  GET_MANA(mob)=GET_MANA(ch);
  mob->points.max_mana=ch->points.max_mana;
  GET_MOVE(mob)=GET_MOVE(ch);
  mob->points.max_move=ch->points.max_move;
  GET_HIT(mob)=GET_HIT(ch);
  mob->points.max_hit=GET_MAX_HIT(ch);
  GET_RACE(mob)=GET_RACE(ch);
  mob->player.sex=ch->player.sex;
  mob->player.time=ch->player.time;
  mob->abilities.str=ch->abilities.str;
  mob->abilities.str_add=ch->abilities.str_add;
  mob->abilities.wis=ch->abilities.wis;
  mob->abilities.intel=ch->abilities.intel;
  mob->abilities.dex=ch->abilities.dex;
  mob->abilities.con=ch->abilities.con;
  mob->player.time.birth=ch->player.time.birth;

  WAIT_STATE(mob, PULSE_VIOLENCE*2);
}

void do_sample(struct char_data *ch, char *arg, int cmd)
{
  struct room_data *rp;
  struct obj_data *sample, *corpse;
  sh_int learned, chance, i;
  char corpsename[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];


  if(!can_do(ch, SKILL_SAMPLE))
    return;

  if (!arg)
    {
      send_to_char("Which corpse do you wish to sample?\n\r",ch);
      return;
    }

  one_argument(arg, corpsename);

  rp=real_roomp(ch->in_room);
  corpse = get_obj_in_list_vis(ch,corpsename,rp->contents);

  if(!corpse)
    {
      send_to_char("You do not see a corpse by that name here.\n\r",ch);
      return;
    }
  if (IS_CARCASS(corpse))
    {
      send_to_char("This carcass is much too tough to sample.\n\r",ch);
      return;
    }
  if (!IS_CORPSE(corpse))
    {
      send_to_char("You can only get samples from corpses.\n\r",ch);
      return;
    }

  if ( ((GET_LEVEL(ch, SHIFTER_LEVEL_IND) < 40) &&
	(corpse->obj_flags.value[2] > 2)) ||
       ((GET_LEVEL(ch, SHIFTER_LEVEL_IND) < 50) &&
	(corpse->obj_flags.value[2] > 3)) ||
       (corpse->obj_flags.value[2] > 4))
    {
      send_to_char("There seems to be no more good samples on this corpse.\n\r",ch);
      WAIT_STATE(ch,PULSE_VIOLENCE);
      return;
    }

  /*  if(GET_LEVEL(ch, SHIFTER_LEVEL_IND) < 40)
      corpse->obj_flags.value[2] += 6;
      else if (GET_LEVEL(ch, SHIFTER_LEVEL_IND) < 50)
      corpse->obj_flags.value[2] += 3;
      else
      corpse->obj_flags.value[2] += 2;
  */
  corpse->obj_flags.value[2]++;

  learned = ch->skills[SKILL_SAMPLE].learned;
  chance = (learned/2) + int_app[GET_INT(ch)].learn/2;

  if ( number(1,101) > MIN(learned, chance) )
    {
      send_to_char("You fail at your attempt to make a sample from this corpse.\n\r",ch);
      act("$n fails at his attempt to make a sample.",FALSE,ch,0,0,TO_ROOM);
      charge_mana(ch, SKILL_SAMPLE, FALSE);
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
      return;
    }

  send_to_char("You examine the corpse for a minute, then thrust your hand into the corpse\n\r",ch);
  send_to_char("and cut out a clean wafer.\n\r",ch);
  act("$n deftly cuts out a sample from the corpse.",FALSE,ch,0,0,TO_ROOM);

  charge_mana(ch, SKILL_SAMPLE, TRUE);

  if (!(sample = make_object(544,VIRTUAL|NORAND)))
    {
      send_to_char("Sorry, I cannot seem to make a sample.\n\r",ch);
      log_msg("Sample: Couldn't find object 544.");
      return;
    }

  sprintf(buf, "sample %s", ss_data(corpse->char_name));
  sample->name = ss_make(buf);

  sample->char_name = ss_make(ss_data(corpse->char_name));

  sample->description = ss_make("A small wafer of a dried hide lies here.");

  sprintf(buf, "a sample of %s", ss_data(corpse->short_description));
  sample->short_description = ss_make(buf);

  for(i=0;i<MAX_OBJ_AFFECT;i++) {
    sample->affected[i].location = 0;
    sample->affected[i].modifier = 0;
  }

  sample->obj_flags.type_flag = ITEM_OTHER;
  sample->obj_flags.wear_flags = ITEM_TAKE;
  sample->obj_flags.extra_flags = ITEM_ORGANIC | ITEM_MAGIC;
  sample->obj_flags.value[3] = SAMPLE_CORPSE;
  sample->obj_flags.weight = 1;
  sample->obj_flags.cost = 100;
  sample->obj_flags.cost_per_day = 0;
  sample->obj_flags.value[2] = corpse->char_vnum;
  sample->obj_flags.level = corpse->obj_flags.sample_level;

  sample->carried_by=0;
  sample->equipped_by=0;

  obj_to_char(sample,ch);

  array_insert(&object_list, corpse);

  WAIT_STATE(ch,PULSE_VIOLENCE*3);

}

void do_cocoon(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *mob, *temp_ch;
  char buf[256];

  if (!IS_PC(ch))
    return;

  if (!can_do(ch, SKILL_REGEN))
    return;

  if (IS_SET(ch->specials.mob_act, ACT_LIQUID)) {
    send_to_char("You are already in a regenerative state.\n\r", ch);
    return;
  }

  if (number(1,101) > ch->skills[SKILL_REGEN].learned) {
    send_to_char("You attempt to cocoon but lose your concentration.\n\r", ch);
    act("$n tries to cocoon but loses $s concentration.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_REGEN, FALSE);
    return;
  }

  if (!(mob=make_mobile(VMOB_69, VIRTUAL))) {
    log_msg("screwup in cocoon skill.");
    send_to_char("This skill is experiencing technical difficulty.\n\r", ch);
    return;
  }

  if (ch->specials.fighting)
    stop_fighting(ch);

  shifter_normalize(ch);

  if (IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
    temp_ch=ch->orig;
    do_return(ch, "", -1);
    ch=temp_ch;
  }

  charge_mana(ch, SKILL_REGEN, TRUE);

  if (ch->master)
    stop_follower(ch);
  stop_all_followers(ch);

  char_to_room(mob, ch->in_room);
  SwitchStuff(ch, mob);

  send_to_char("You liquify and withdraw into an opaque cocoon!\n\r",ch);
  act("$n liquifies and withdraws into an opaque cocoon!",TRUE,ch,0,0,TO_ROOM);

  /* put the char's original form in storage */
  char_from_room(ch);
  char_to_room(ch, 3);

  /* switch into the pool */
  push_character(ch, mob);

  /* combine names of pool and player */
  sprintf(buf,"%s %s", GET_IDENT(mob), GET_IDENT(ch));
  ss_free(mob->player.name);
  mob->player.name = ss_make(buf);

  /* set the polymorphed flag on the player */
  SET_BIT(mob->specials.mob_act, ACT_POLYSELF);
  SET_BIT(mob->specials.mob_act, ACT_SHIFTER);
  mob->specials.flags = ch->specials.flags;

  GET_MANA(mob)=GET_MANA(ch);
  mob->points.max_mana=ch->points.max_mana;
  GET_MOVE(mob)=GET_MOVE(ch);
  mob->points.max_move=ch->points.max_move;
  GET_HIT(mob)=GET_HIT(ch);
  mob->points.max_hit=GET_MAX_HIT(ch);
  GET_RACE(mob)=GET_RACE(ch);
  mob->player.sex=ch->player.sex;
  mob->player.time=ch->player.time;
  mob->abilities.str=ch->abilities.str;
  mob->abilities.str_add=ch->abilities.str_add;
  mob->abilities.wis=ch->abilities.wis;
  mob->abilities.intel=ch->abilities.intel;
  mob->abilities.dex=ch->abilities.dex;
  mob->abilities.con=ch->abilities.con;
  mob->player.time.birth=ch->player.time.birth;

  WAIT_STATE(mob, PULSE_VIOLENCE*2);
}

void do_shift(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data  *corpse;
  struct char_data *mob, *temp_ch;
  char buf[256];

  if (!IS_PC(ch))
    return;

  if (!can_do(ch, SKILL_SHIFT))
    return;

  if (!*argument) {
    send_to_char("What corpse do you wish to shift into?\n\r", ch);
    return;
  }

  corpse=get_obj_in_list_vis(ch, argument,
			     real_roomp(ch->in_room)->contents);

  if (!corpse) {
    send_to_char("There's no corpse of that here.\n\r", ch);
    return;
  }

  if (!(IS_CORPSE(corpse) || (corpse->obj_flags.value[3] == SAMPLE_CORPSE))) {
    send_to_char("That's not a corpse.\n\r", ch);
    return;
  }

  if (number(1,101)>ch->skills[SKILL_SHIFT].learned ||
      !(corpse->char_vnum || corpse->obj_flags.value[2])) {
    send_to_char("You attempt to shift but lose your concentration.\n\r", ch);
    act("$n atempts to shift but loses $s concentration.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_SHIFT, FALSE);
    return;
  }

  if (corpse->obj_flags.value[3] == SAMPLE_CORPSE)
    mob=make_mobile(corpse->obj_flags.value[2], VIRTUAL);
  else
    mob=make_mobile(corpse->char_vnum,VIRTUAL);

  if (!mob) {
    log_msg("screwup in shift skill.");
    send_to_char("This skill is experiencing technical difficulty.\n\r", ch);
    return;
  }

  char_to_room(mob, ch->in_room);

  SET_BIT(mob->specials.mob_act, ACT_POLYSELF);

  if (GetMaxLevel(mob)>GetMaxLevel(ch)) {
    send_to_char("You skill is not powerful enough, yet.\n\r", ch);
    act("$n attempts to shift but is not powerful enough.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_SHIFT, FALSE);
    extract_char(mob);
    return;
  }

  if ((corpse->obj_flags.value[3]!= SAMPLE_CORPSE) && (corpse->obj_flags.value[2] >= 3))
    {
      send_to_char("It seems that you cannot find enough material to shift into.\n\r",ch);
      extract_char(mob);
      return;
    }

  charge_mana(ch, SKILL_SHIFT, TRUE);

  mob_index[mob->nr].number--;

  shifter_normalize(ch);

  if (IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
    temp_ch=ch->orig;
    do_return(ch, "", -1);
    ch=temp_ch;
  }

  if (ch->master)
    stop_follower(ch);
  stop_all_followers(ch);

  act("You liquify and flow over $p, shifting!",TRUE,ch,corpse,0,TO_CHAR);
  act("$n shifts into $p!",TRUE,ch,corpse,0,TO_ROOM);
  extract_obj(corpse);

  mob->player.mpstate=ch->player.mpstate;

  /*mob->specials.damnodice=1; */
  /*mob->specials.damsizedice=2; */
  /*mob->points.armor=100; */
  /*mob->points.hitroll=mob->points.damroll=0; */
  mob->specials.apply_saving_throw[0]=0;
  mob->specials.apply_saving_throw[1]=0;
  mob->specials.apply_saving_throw[2]=0;
  mob->specials.apply_saving_throw[3]=0;
  mob->specials.apply_saving_throw[4]=0;

  if (IsHumanoid(mob))
    {
      mob->points.armor=100;
      mob->points.hitroll=mob->points.damroll=0;
      mob->specials.damnodice=1;
      mob->specials.damsizedice=2;
    }

  SwitchStuff(ch, mob);

  /* put the char's original form in storage */
  char_from_room(ch);
  char_to_room(ch, 3);
  push_character(ch, mob);

  /* combine names, etc... */
  sprintf(buf,"%s %s", GET_IDENT(mob), GET_IDENT(ch));
  ss_free(mob->player.name);
  mob->player.name = ss_make(buf);

  /* set the polymorphed flag on the player */
  mob->specials.mob_act=0;
  SET_BIT(mob->specials.mob_act, ACT_POLYSELF);
  SET_BIT(mob->specials.mob_act, ACT_SHIFTER);
  SET_BIT(mob->specials.mob_act, ACT_ISNPC);

  mob->specials.flags = ch->specials.flags;

  GET_POS(mob)=POSITION_STANDING;

  GET_MANA(mob)=GET_MANA(ch);
  mob->points.max_mana=ch->points.max_mana;
  GET_MOVE(mob)=GET_MOVE(ch);
  mob->points.max_move=ch->points.max_move;
  GET_HIT(mob)=GET_HIT(ch);
  mob->points.max_hit=GET_MAX_HIT(ch);
  /*  GET_RACE(mob)=GET_RACE(ch); */ /* can't have no-eq forms wearing eq */
  mob->player.sex=ch->player.sex;
  mob->player.time.birth=ch->player.time.birth;

  /*  MakeAffect(mob, mob, SPELL_TYPE_SPELL,
      SKILL_SHIFT, 0, 0, 0,
      0, 10, FALSE, FALSE, FALSE, do_return);*/

  WAIT_STATE(mob, PULSE_VIOLENCE*2);
}

#if 0
void do_chaos_hammer(struct char_data *ch, char *arg, int cmd)
{
  struct room_data *room;
  struct char_data *victim;
  int weight, level, dex, damage;

  weight = 0;
  level = 0;
  dex = 0;
  damage = 0;

  if(!ch)
    return;

  if(!can_do(ch, SKILL_CHAOS_HAMMER))
    return;

  /*  player must have formed hammers first */
  if(ch->specials.attack_type == TYPE_HIT || ch->specials.attack_type == TYPE_CRUSH) {
    /* Get the players weight, level, and dex */
    weight = GET_WEIGHT(ch);
    level = GetMaxLevel(ch);
    dex = GET_DEX(ch);

    /* Damage is calculated based on the weight, level, dex and ability to hit the victim
       The victim gets advantages vs Dex and Flying.
    */
    if(!(victim=set_target(ch, arg, SKILL_CHAOS_HAMMER)))
      return;

    if(NewSkillSave(ch, victim, SKILL_CHAOS_HAMMER, 0, 0)) {
      // strings to send people when failed
    } else {
      // strings to send and damage to do
    }
  } else {
    // THey don't have hammers...so tell them to form them.
  }

}
#endif
void do_bind (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;

  if (!can_do(ch, SKILL_BIND))
    return;

  if (ch->specials.fighting && ch->specials.binding) {
    act("You release $N from your binding grip!",FALSE,ch,0,ch->specials.binding,TO_CHAR);
    act("$n releases you from $s binding grip!",FALSE,ch,0,ch->specials.binding,TO_VICT);
    act("$n releases $N from $s binding grip!",TRUE,ch,0,ch->specials.binding,TO_ROOM);
    ch->specials.binding->specials.binded_by=0;
    ch->specials.binding=0;
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
  }

  if (ch->specials.binded_by) {
    send_to_char("You're a little too tied up yourself to try that.\n\r", ch);
    return;
  }

  if (!(victim=set_target(ch, arg, SKILL_BIND)))
    return;

  if (victim->specials.binded_by) {
    act("$N is already bound.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  charge_mana(ch, SKILL_BIND, TRUE);

  if (ch->specials.fighting)
    stop_fighting(ch);
  set_fighting(ch, victim);

  if (NewSkillSave(ch, victim, SKILL_BIND, 0, IMM_HOLD)) {
    act("You try to bind $N but $E throws you off!",FALSE,ch,0,victim,TO_CHAR);
    act("$n tries to bind you but you throw $m off!",FALSE,ch,0,victim,TO_VICT);
    act("$n tries to bind $N but gets thrown off!",TRUE,ch,0,victim,TO_ROOM);
    GET_POS(ch)=POSITION_SITTING;
  } else {
    act("You pounce on $N and bind $M in a crushing grip!",FALSE,ch,0,victim,TO_CHAR);
    act("$n pounces on you and binds you in a crushing grip!",FALSE,ch,0,victim,TO_VICT);
    act("$n pounces on $N and binds $M in a crushing grip!",TRUE,ch,0,victim,TO_ROOM);

    if (victim->specials.fighting)
      stop_fighting(victim);
    set_fighting(victim, ch);

    ch->specials.binding=victim;
    victim->specials.binded_by=ch;
  }

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_contract (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  int dam;

  if (!can_do(ch, SKILL_CONTRACT))
    return;

  if (!ch->specials.binding) {
    send_to_char("You need to be binding your victim first.\n\r", ch);
    return;
  }

  victim=ch->specials.binding;

  charge_mana(ch, SKILL_CONTRACT, TRUE);

  dam=dice(10, GetMaxLevel(ch));

  send_to_char("You contract...\n\r", ch);
  act("$n contracts...",TRUE,ch,0,0,TO_ROOM);

  if (number(1,101) > ch->skills[SKILL_CONTRACT].learned)
    dam=0;
  if (saves_spell(victim, SAVING_SPELL, 0))
    dam/=2;

  damage(ch, victim, dam, SKILL_CONTRACT);

  WAIT_STATE(ch,PULSE_VIOLENCE);
}

void do_plate(struct char_data *ch, char *arg, int cmd)
{
  int i;

  if (affected_by_spell(ch, SKILL_PLATE)) {
    if (cmd>=0) {
      send_to_char("Your armor plating returns to normal skin.\n\r", ch);
      act("$n's armor plating return to normal skin.",TRUE,ch,0,0,TO_ROOM);
      affect_from_char(ch, SKILL_PLATE);
    }
    return;
  }

  if (!can_do(ch, SKILL_PLATE))
    return;

  if (number(1,101)>ch->skills[SKILL_PLATE].learned) {
    send_to_char("You try to form your skin into armor plating but lose your concentration.\n\r",ch);
    act("$n tries to form $s skin into armor plating but loses $s concentration.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_PLATE, FALSE);
    return;
  }

  charge_mana(ch, SKILL_PLATE, TRUE);

  for (i=0; i<MAX_WEAR; i++) {
    if (shifter_plate_test(i) && ch->equipment[i])
      obj_to_char(unequip_char(ch, i), ch);
  }

  MakeAffect(ch, ch, SPELL_TYPE_SPELL,
	     SKILL_PLATE, -25*MAX(1, (GetMaxLevel(ch)/10)), APPLY_AC, 0,
	     0, 5, FALSE, FALSE, FALSE, do_plate);

  send_to_char("You form your skin into defensive armor plating.\n\r", ch);
  act("$n forms $s skin into defensive armor plating.",TRUE,ch,0,0,TO_ROOM);

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

int app_rand(int num, int percent)
{
  float rnum;
  int rval;

  rnum=(number(0,1) ? 1.0 :-1.0);
  rval=num - (int)(num * (rnum * (float)(percent/100.0)));
  return(rval);
}

void do_appraise(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[240], buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int BitMask, percent;
  struct obj_data *obj;

  if (!can_do(ch,SKILL_APPRAISE))
    return;

  only_argument(argument, obj_name);

  /* if object exists in inventory */
  if((obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)));
  /* if object exists in room */
  else obj = get_obj_in_list_vis(ch, obj_name,
				 real_roomp(ch->in_room)->contents);

  if (!obj) {
    send_to_char("Yes, but what is it you are appraising?.\n\r", ch);
    return;
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*10); /* they're gonna have to wait. */

  /* Lower numbers are better */
  percent=100 - ch->skills[SKILL_APPRAISE].learned +
    number(1,3) * (GET_WIS(ch)-17),0;

  BitMask = GetItemClassRestrictions(obj);

  if ( !HasClass(ch, CLASS_THIEF) &&
       ((IsRestricted(BitMask, ch->player.clss)) || !(IsLevelOk(ch,obj))))
    {
      send_to_char("You examine the object closely and discover that you haven't a clue what "
		   "it is.\n\r", ch);
      return;
    }

  send_to_char("You examine the object closely and discover:\n\r", ch);

  if (IS_PURE_CLASS(ch) && HasClass(ch, CLASS_THIEF)) {
    spell_identify(LVL_GOD - 1, ch, 0, NULL, obj);
    return;
  }

  sprintf(buf, "Object '%s', Item type: %s ", OBJ_NAME(obj),GET_WPN_DMG_TYPE(obj));
  sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
  strcat(buf,buf2); strcat(buf,"\n\r");
  send_to_char(buf, ch);

  if (HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_DRUID))
    {
      send_to_char("Item is: ", ch);
      if (IS_PURE_ITEM(obj))
	sprintbit(obj->obj_flags.extra_flags,extra_bits_pure,buf);
      else
	sprintbit( obj->obj_flags.extra_flags,extra_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
    }


  sprintf(buf,"Weight: %d", obj->obj_flags.weight);
  if (HasClass(ch, CLASS_THIEF))
    {
      sprintf(buf2,", Value: %d, Rent cost: %d\n\r",
	      MAX(app_rand(obj->obj_flags.cost,percent),-1),
	      MAX(app_rand(obj->obj_flags.cost_per_day,percent),-1));
      strcat(buf,buf2);
    }
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  switch (GET_ITEM_TYPE(obj)) {
  case ITEM_SCROLL :
  case ITEM_POTION :
    if (HasClass(ch, CLASS_MAGIC_USER))
      {
	sprintf(buf, "Level %d spells of:\n\r",
		MAX(app_rand(obj->obj_flags.value[0],percent),1));
	send_to_char(buf, ch);
	if (obj->obj_flags.value[1] >= 1) {
	  if(number(0,101) < percent)
	    send_to_char("Unknown\n\r",ch);
	  else
	    {
	      sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[1]));
	      send_to_char(buf, ch);
	    }
	}

	if (obj->obj_flags.value[2] >= 1) {
	  if(number(0,101) < percent)
	    send_to_char("Unknown\n\r",ch);
	  else
	    {
	      sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[2]));
	      send_to_char(buf, ch);
	    }

	}
	if (obj->obj_flags.value[3] >= 1) {
	  if(number(0,101) < percent)
	    send_to_char("Unknown\n\r",ch);
	  else
	    {
	      sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[3]));
	      send_to_char(buf, ch);
	    }
	}
      }
    break;

  case ITEM_WAND  :
  case ITEM_STAFF :
    if (HasClass(ch, CLASS_MAGIC_USER))
      {
	sprintf(buf, "Has %d charges, with %d charges left.\n\r",
		MAX(app_rand(obj->obj_flags.value[1],percent),1),
		MAX(app_rand(obj->obj_flags.value[2],percent),1));
	send_to_char(buf, ch);

	sprintf(buf, "Level %d spell of:\n\r",
		MAX(app_rand(obj->obj_flags.value[0],percent),1));
	send_to_char(buf, ch);

        if(number(0,101) < percent)
	  send_to_char("Unknown\n\r",ch);
	else
	  {
	    if (obj->obj_flags.value[3] >= 1) {
	      sprintf(buf, "  %s\n\r", spell_name(obj->obj_flags.value[3]));
	      send_to_char(buf, ch);
	    }
	  }
      }
    break;

  case ITEM_WEAPON :
    if(HasClass(ch,CLASS_WARRIOR) || HasClass(ch,CLASS_PALADIN) ||
       HasClass(ch,CLASS_RANGER))
      {
	sprintf(buf, "Damage Dice is '%dD%d'\n\r",
		MAX(app_rand(obj->obj_flags.value[1],percent),1),
		MAX(app_rand(obj->obj_flags.value[2],percent),1));
	send_to_char(buf, ch);
      }
    break;

  case ITEM_FIREWEAPON :
    if(HasClass(ch,CLASS_RANGER))
      {
	sprintf(buf, "Hit is '%d'\n\rMax range/Dam is '%d'\n\r"
		"Type of bow[must match arrow type #] is '%d'\n\r",
		MAX(app_rand(obj->obj_flags.value[1],percent),1),
		MAX(app_rand(obj->obj_flags.value[2],percent),1),
		obj->obj_flags.value[3]);
	send_to_char(buf, ch);
      }
    break;

  case ITEM_MISSILE :
    if(HasClass(ch,CLASS_RANGER))
      {
	sprintf(buf, "Change of breaking on impact[Percentage] is '%d'\n\r"
		"Damage Dice is '%dD%d'\n\r"
		"Type of arrow[must match bow type #] is '%d'\n\r",
		MAX(app_rand(obj->obj_flags.value[0],percent),0),
		MAX(app_rand(obj->obj_flags.value[1],percent),1),
		MAX(app_rand(obj->obj_flags.value[2],percent),1),
		obj->obj_flags.value[3]);
	send_to_char(buf, ch);
      }
    break;

  case ITEM_ARMOR :
    if(HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_RANGER) ||
       HasClass(ch, CLASS_PALADIN))
      {
	sprintf(buf, "AC-apply is %d\n\r",
		MAX(app_rand(obj->obj_flags.value[0],percent),1));
	send_to_char(buf, ch);
      }
    break;
  }
  /*
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

    default:
    sprintf(buf, "    Affects : %s By %d\n\r", buf2,aff->modifier);
    send_to_char(buf, ch);
    }
    }
    }*/
}

void do_wings(struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_WINGS))
    return;

  if (affected_by_spell(ch, SKILL_WINGS)) {
    if (command>=0) {
      send_to_char ("You reasorb your mighty wings!\n\r",ch);
      act("$n's mighty wings are reasorbed into his body.",TRUE,ch,0,0,TO_ROOM);
      affect_from_char(ch, SKILL_WINGS);
      return;
    }
  }

  if (ch->skills[SKILL_WINGS].learned < dice (1,101)) {
    act("You try to form wings, but can't seem to concentrate.",FALSE,ch,0,0,TO_CHAR);
    act("Wings start to sprout from $n's back, but then they suddenly retract.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_WINGS, FALSE);
  } else {
    charge_mana(ch, SKILL_WINGS, TRUE);
    act("Powerful wings sprout from your back.",FALSE,ch,0,0,TO_CHAR);
    act("Powerful wings sprout from $n's back.",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_WINGS, 0, APPLY_NONE, AFF_FLYING,
               GetMaxLevel(ch)/2, 5, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

void do_gills(struct char_data *ch, char *argument, int command)
{
  if (!can_do(ch, SKILL_GILLS))
    return;

  if (affected_by_spell(ch, SKILL_GILLS)) {
    if(command>=0) {
      send_to_char ("You breathe easier as your gills close up!\n\r",ch);
      act("$n's gills are reasorbed into his neck.",TRUE,ch,0,0,TO_ROOM);
      affect_from_char(ch, SKILL_GILLS);
      return;
    }
  }

  if (ch->skills[SKILL_GILLS].learned < dice (1,101)) {
    act("You form scaly gills on your neck.",FALSE,ch,0,0,TO_CHAR);
    act("$n forms scaly gills on %s neck.",TRUE,ch,0,0,TO_ROOM);
    charge_mana(ch, SKILL_GILLS, FALSE);
  } else {
    charge_mana(ch, SKILL_GILLS, TRUE);
    act("You form scaly gills on your neck.",FALSE,ch,0,0,TO_CHAR);
    act("$n forms scaly gills on $s neck.",TRUE,ch,0,0,TO_ROOM);
    MakeAffect(ch, ch, SPELL_TYPE_SPELL,
               SKILL_GILLS, 0, APPLY_NONE, AFF_WATERBREATH,
               GetMaxLevel(ch)/2, 5, FALSE, FALSE, FALSE, NULL);
  }

  WAIT_STATE(ch,PULSE_VIOLENCE*2);
}

/* Bard Singing stuff... */
void do_sing(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  struct spell_info *spell;
  char        buffer[256];
  char        *ptr;

  if (!(*argument)) {
    send_to_char("You raise your clear (?) voice towards the sky.\n\r",ch);
    act("SEEK SHELTER AT ONCE! $n has begun to sing.",TRUE,ch,0,0,TO_ROOM);
    return;
  }
  if (IS_NPC(ch) && (IS_AFFECTED(ch, AFF_CHARM)))
    {
      send_to_char("You're a charmed mob, and you wanna sing!?!\n\r", ch);
      return;
    }

  if (!IsHumanoid(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
    return;
  }

  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r",ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to robbing and killing...\n\r",ch);
      return;
    } else if (BestMagicClass(ch) == RANGER_LEVEL_IND) {
      send_to_char("Think you should stick to whistling in the woods...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == PSI_LEVEL_IND) {
      send_to_char("You couldn't carry a tune in a bucket!\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == DRUID_LEVEL_IND) {
      send_to_char("Which type of plant have you been smoking!\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == PALADIN_LEVEL_IND) {
      send_to_char("You should have been a choir boy - No Chance!\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == SHIFTER_LEVEL_IND) {
      send_to_char("Try shifting into Pavorati first!\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
      send_to_char("Nahhhh - Singing is way too much fun!\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == MAGE_LEVEL_IND) {
      send_to_char("You can barely chant - and you want to sing?\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == CLERIC_LEVEL_IND) {
      send_to_char("You should probably just stick to hymns!\n\r", ch);

      return;
    }
  }

  if (apply_soundproof(ch))
    return;

  argument = skip_spaces(argument);

  /* If there is no chars in argument
     if (!(*argument)) {
     send_to_char("Sing about what? where?\n\r", ch);
     return;
     }
  */

  if (*argument++ != '\'') {
    send_to_char("Lyrics should be encased by the musical symbols : '\n\r",ch);
    return;
  }

  /* Locate the last quote && lowercase the magic words (if any) */
  for(ptr = buffer ; *argument && *argument != '\'' ; argument++)
    *ptr++ = LOWER(*argument);
  *ptr = 0;

  if(*argument++ != '\'')
    {
      send_to_char("Lyrics should be encased by the musical symbols : '\n\r",ch);
      return;
    }
  argument = skip_spaces(argument);

  spell = locate_spell(buffer, 0);

  if (!*buffer)
    {
      send_to_char("Your lips do not move, no melody is heard.\n\r",ch);
    }
  /* not sure if this serves a purpose for now..
     else if (!ch->skills)
     {
     }
  */
  else if(!spell || !spell->spell_pointer)
    {
      send_to_char("Maybe you should stick to humming.. or mumbling.\n\r",ch);
    }
  else if (GET_POS(ch) < spell->minimum_position)
    {
      switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
	send_to_char("You sing in your sleep.\n\r", ch);
	break;
      case POSITION_RESTING :
	send_to_char("You're too relaxed to carry a tune.\n\r",ch);
	break;
      case POSITION_SITTING :
	send_to_char("You can't do this sitting!\n\r", ch);
	break;
      case POSITION_FIGHTING :
	send_to_char("Uhm. You're a bit busy to play right now.\n\r", ch);
	break;
      default:
	send_to_char("It seems like you're in pretty bad shape!\n\r",ch);
	break;
      }
    }
  else if(!IS_IMMORTAL(ch) && !CanLearn(ch,spell))
    {
      send_to_char("Sorry, you can't do that.\n\r", ch);
    }
  else if((IS_SET(spell->targets, TAR_VIOLENT) &&
	   check_peaceful(ch, "Do you like rotten eggs and vegatables?\n\r")) ||
	  !spell_target(ch, spell->targets, argument, &tar_char, &tar_obj))
    {
    }
  else if (!IS_IMMORTAL(ch) && (GET_MANA(ch) < USE_MANA(ch, spell)))
    {
      send_to_char("You can't summon enough mana to play.\n\r", ch);
    }
  else
    {
      sing_song(ch, spell);
      if (!IS_IMMORTAL(ch))
        {
	  WAIT_STATE(ch, spell->beats);

	  if (number(1,101) > skill_chance(ch, spell->number) &&
	      !IS_IMMORTAL(ch))
            {                   /* 101% is failure */

	      send_to_char("You lost your concentration!\n\r", ch);
	      return;
            }
        }


      if(tar_char && (GET_POS(tar_char) == POSITION_DEAD))
        {
	  send_to_char("The magic fizzles against the dead body\n", ch);
	  return;
        }

      send_to_char("Ok.\n\r",ch);

      if(!check_nomagic(ch))
        {
	  (*spell->spell_pointer)(GET_LEVEL(ch, BestMagicClass(ch)), ch,
				  argument, SPELL_TYPE_SPELL, tar_char, tar_obj);
        }
    }
}

void sing_song( struct char_data *ch, struct spell_info* spell)
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  struct char_data *temp_char;

  strcpy(buf, "");
  strcpy(splwd, spell->name);
  /* put in a check here */
  /* for bards, for now


  if (!(instrument = get_obj_in_list_vis(ch, "harp",
  ch->carrying))) {
  sprintf(buf3, "You have no harp - how do you expect to play your %s?\n\r",splwd);
  send_to_char(buf3, ch);
  return;
  }
  if(!(instrument=ch->equipment[HOLD])) {
  sprintf(buf3,"Its hard to play on the harp with out holding it!");
  }
  */

  sprintf(buf, "You raise your voice to the sky and sing '%s'.", splwd);
  sprintf(buf2,"$n sings the song '%s'.", splwd);

  for(temp_char = real_roomp(ch->in_room)->people;
      temp_char;
      temp_char = temp_char->next_in_room) {
    if (temp_char != ch) {
      act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
    }
    else {
      act(buf, FALSE, ch, 0, temp_char, TO_VICT);
    }
  }
}

void do_mysongs(struct char_data *ch, char *argument, int cmd)
{
  char buf[16384], temp[256];
  struct spell_info* spell;
  int i;

  if (!IS_PC(ch))    {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }

  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to robbing and killing...\n\r",ch);
      return;

    } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
      send_to_char("Think you should stick to kicking ass...\n\r",ch);
      return;

    } else if (BestMagicClass(ch) == MAGE_LEVEL_IND) {
      send_to_char("You can barely chant, and you want to sing songs?...\n\r",ch);
      return;

    } else if (BestMagicClass(ch) == CLERIC_LEVEL_IND) {
      send_to_char("Sorry, the only songs you know are hymns.\n\r", ch);
      return;

    } else if (BestMagicClass(ch) == PALADIN_LEVEL_IND) {
      send_to_char("If you prayed REALLY hard, you might sing well.. nah.\n\r", ch);
      return;

    } else if (BestMagicClass(ch) == PSI_LEVEL_IND) {
      send_to_char("You can't think of any songs ...\n\r", ch);
      return;

    } else if (BestMagicClass(ch) == SHIFTER_LEVEL_IND) {
      send_to_char("You should stick to imitating Elvis...\n\r", ch);
      return;

    } else if (BestMagicClass(ch) == DRUID_LEVEL_IND) {
      send_to_char("You should stick to hugging trees...\n\r", ch);
      return;

    } else if (BestMagicClass(ch) == RANGER_LEVEL_IND) {
      send_to_char("You can barely whistle in tune...\n\r", ch);
      return;
    }
  }

  *buf=0;
  for (i = 0, spell = spell_list ; i <= spell_count; i++, spell++)
    {
      if (spell->name &&
	  CanLearn(ch, spell) && (ch->skills[spell->number].learned > 0))
        {
	  sprintf(temp, "%-20s  Mana: %3d,%s\n\r",
		  spell->name, USE_MANA(ch, spell),
		  how_good(ch->skills[spell->number].learned));
	  if((strlen(temp) + strlen(buf) + 1) > sizeof(buf))
            {
	      send_to_char("You just know too many songs...\n\r", ch);
	      break;
            }
	  strcat(buf, temp);
        }
    }

  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_suggestion (struct char_data *ch, char *arg, int cmd)
{
  struct command_info* suggest_cmd;
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH],
    buf[MAX_INPUT_LENGTH+40],buf2[MAX_INPUT_LENGTH+40];
  int itr=0;
  int suggest_array[278]={1,2,3,4,5,6,7,8,9,17,18,19,22,23,24,25,26,27,
			  28,29,30,31,32,33,34,35,36,37,40,42,43,44,45,46,49,53,75,83,84,86,
			  94,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
			  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
			  128,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
			  144,145,146,147,151,154,155,156,157,158,159,160,161,162,163,165,
			  169,171,172,176,178,179,180,181,182,183,184,185,186,187,188,
			  189,190,191,192,197,198,206,207,209,214,232,233,239,
			  245,246,248,249,252,257,260,262,263,264,265,266,267,268,269,
			  270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,
			  286,287,288,289,290,291,292,293,294,295,296,297,298,299,301,
			  302,303,304,305,306,307,308,309,311,312,313,314,315,316,317,318,
			  319,323,327,329,331,332,334,338,355,356,357,358,359,360,361,362,
			  363,364,367,375,376,389,390,391,392,393,394,395,396,397,410,411,
			  412,413,414,416,422,423,424,425,426,427,431,434,436,437,456,
			  464,465,466,467,468,469,470,471,472,473,474,475,476,477,478,479,
			  480,485,486,487,10000};
  /* 456,457,458,459,460,461,462,463 Removed because there are two sets */
  if (!can_do(ch, SKILL_SUGGESTION))
    return;

  charge_mana(ch, SKILL_SUGGESTION, TRUE);

  if (!is_hold_instrument(ch,SKILL_SUGGESTION)) {
    return;
  }

  half_chop(arg, name, to_force);

  if (!*name || !*to_force) {
    send_to_char("Whom do you wish to suggest to do what?\n\r", ch);
    return;
  }

  vict=get_char_room_vis(ch,name);

  if (!vict) {
    send_to_char("Try suggesting to someone who is here!\n\r",ch);
    return;
  }

  if (IS_IMMORTAL(vict)) {
    send_to_char("Do you really think you can influence an immortal?\n\r",
		 ch);
    return;
  }


  if (!IS_NPC(vict)) {
    send_to_char("You can just keep your suggestions to yourself!\n\r",ch);
    return;
  }

  one_argument(to_force, buf2);
  suggest_cmd=lookup_command(buf2, 0);

  if (!suggest_cmd) {
    send_to_char("Whom do you wish to suggest to do what?\n\r", ch);
    return;
  }

  while (suggest_cmd->index != suggest_array[itr] &&
	 itr !=277) {
    itr++;
  }
  if (GET_POS(vict)<=POSITION_SLEEPING || vict->specials.fighting) {
    act("You decide against persuading a sleeping $N.",FALSE,ch,0,vict,TO_CHAR);
    return;
  }
  if (!SkillChance(ch,vict,20,IMM_BARD,SPLMOD_INT | SPLMOD_CHA,SKILL_SUGGESTION)) {
    if (percent() < 33) {
      act("The now angry $N does not like your attempt at persuasion!",FALSE,ch,0,vict,TO_CHAR);
      act("You are angered by $n's attempt to persuade you!",TRUE,ch,0,vict, TO_VICT);
      act("$N becomes very critical of $n's suggestions - and attacks!",TRUE,ch,0,vict, TO_ROOM);
      AddHated(vict, ch);
      SetVictFighting(ch, vict);
    }
    else {
      act("$N is not persuaded.",FALSE,ch,0,vict,TO_CHAR);
      act("$n tries to suggest something improper.",TRUE,ch,0,vict,TO_VICT);
      act("$N ignores $n's suggestion.",TRUE,ch,0,vict,TO_ROOM);
    }
    return;
  }

  if (suggest_cmd->index == suggest_array[itr])
    {
      if (!(vict = get_char_room_vis(ch, name)) ||
	  (!IS_NPC(vict) && (!CAN_SEE(ch,vict))))
	send_to_char("No-one by that name here..\n\r", ch);
      else
	{
	  sprintf(buf, "$n has suggested that you '%s'.",to_force);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
	  send_to_char("Ok.\n\r", ch);
	  command_interpreter(vict, to_force, 1);
	  act("$N is persuaded.",FALSE,ch,0,vict,TO_CHAR);
	  sprintf(buf,"$n has suggested that $N should '%s'.",to_force);
	  act(buf,TRUE,ch,0,vict,TO_ROOM);
	}
    }
  else {
    send_to_char("You can't suggest that someone will do that!\n\r",ch);
    return;
  }
}

void do_explode(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict, *lastvict=0;
  int dam, enemies;

  if (!can_do(ch, SKILL_EXPLODE))
    return;

  dam = 0;

  if (number(1,101)>ch->skills[SKILL_EXPLODE].learned)
    dam=0;
  else {
    dam += dice(GetMaxLevel(ch), 3);
    dam = MAX(1, dam);
  }

  act("You get an evil look in your eyes, as shards of your body fly around the room.",
      TRUE,ch,0,0,TO_CHAR);
  act("$n sends pieces of $s body flying in every direction!",TRUE,ch,0,0,TO_ROOM);

  enemies=0;

  charge_mana(ch, SKILL_BRAINSTORM, TRUE);
  GET_MOVE(ch) -= (!dam)?10:20;

  EACH_CHARACTER(iter, vict)
    {
      if (ch->in_room==vict->in_room)
	{
	  if(can_hurt(ch, vict))
	    {
	      act("Pieces of $n's body fly around the room!",
		  TRUE,ch,0,0,TO_ROOM);
	      act("Shards of your flesh fly around the room!",
		  TRUE, ch, 0, 0, TO_CHAR);
	      damage(ch, vict, dam, SKILL_EXPLODE);
	      enemies+=GetMaxLevel(vict);
	      lastvict=vict;
	    }
	}
      else if(real_roomp(ch->in_room)->zone == real_roomp(vict->in_room)->zone)
	act("You hear the sound of exploding flesh nearby.",FALSE,vict,0,0,TO_CHAR);
    }
  END_AITER(iter);

  if(enemies) {
    dam = enemies * 50 / GetMaxLevel(ch);
    send_to_char("\n\rYou are hurt from the effort of exploding yourself.\n\r", ch);
    GET_HIT(ch) -= DamageTrivia(ch, ch, dam, SKILL_EXPLODE);
    if(GET_HIT(ch) < 0) GET_POS(ch) = POSITION_DEAD;
    DamageEpilog(ch, ch, SKILL_EXPLODE);
  }
  if(ch->specials.fighting == ch) stop_fighting(ch);
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

struct pulse_extra {
  int left;
};

int pulse_effect(struct spevent_type *ps, long now) {
  int dam=0;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  struct pulse_extra *psex = (pulse_extra *) ps->extra;

  if(!ps) return 0;

  switch(psex->left) {
  case 1:
    /* pulse effect output room */
    if(ps->caster->in_room == ps->in_room)
      send_to_char("Your energy pulse fills the room with radiance!\n\r", ps->caster);
    send_to_room_except("Energy pulses throughout the room!\n\r", ps->in_room, ps->caster);
    break;
  case 0:
    /* pulse effect output room */
    send_to_room_formatted("$CrAll of the energy in the room is sucked into the pulse!$CN\n\r", ps->in_room);
    break;
  case -1:
    /* pulse effect output room */
    send_to_room("The room erupts into a ball of pure energy!\n\r", ps->in_room);
    break;
  default:
    log_msg("Unknown SKILL_PULSE event [output]");
    break;
  }

  EACH_CHARACTER(iter, vict) {
    if (ps->in_room == vict->in_room) {
	if(can_hurt(ps->caster,vict))
	  switch(psex->left) {
	  case 1:
	    /* first wave */
	    if (number(1,101)>ps->caster->skills[SKILL_PULSE].learned) {
	      dam = 0;

	      if (ps->caster->in_room == ps->in_room) {
		act("You avoid the effect of the $CWenergy pulse$CN.",FALSE, ps->caster,0,vict,TO_VICT);
		act("The $CWenergy pulse$CN has no effect on $N!",TRUE,ps->caster,0,vict,TO_NOTVICT);
		act(show_dam_check(ps->caster,"The $CWenergy pulse$CN has no effect on $N!",dam),FALSE,ps->caster,0,vict,TO_CHAR);
	      } else {
		stop_fighting(ps->caster);
		sprintf(buf, "The $CWenergy pulse$CN has no effect on %s!\n\r", GET_NAME(vict));
		send_to_room_except_formatted(buf,ps->in_room,vict);
		send_to_char_formatted("You avoid the effect of the $CWenergy pulse$CN.\n\r", vict);
	      }

	    } else {
	      dam = dice(GetMaxLevel(ps->caster), 10);

	      if (ps->caster->in_room == ps->in_room) {
		act("You are enveloped by the $CWenergy pulse$CN.",FALSE, ps->caster,0,vict,TO_VICT);
		act("The $CWenergy pulse$CN envelops $N!",TRUE,ps->caster,0,vict,TO_NOTVICT);
		act(show_dam_check(ps->caster,"The $CWenergy pulse$CN envelops $N!",dam),FALSE,ps->caster,0,vict,TO_CHAR);
	      } else {
		stop_fighting(ps->caster);
		sprintf(buf, "The $CWenergy pulse$CN envelops %s!\n\r", GET_NAME(vict));
		send_to_room_except_formatted(buf,ps->in_room,vict);
		send_to_char_formatted("You are enveloped by the $CWenergy pulse$CN.\n\r", vict);
	      }
	    }

	    if (!IS_PC(vict)) {
	      /* experimental */
	      AddHated(vict, ps->caster);
	      //SetHunting(vict, ps->caster);
	      //vict->persist = MAX_ROOMS;
	    }

	    damage(ps->caster, vict, dam, SKILL_PULSE);

	    break;
	  case 0:
	    /* second wave */
	    dam = dice(GetMaxLevel(ps->caster), 5);

	      if (ps->caster->in_room == ps->in_room) {
		act("You are hit by the shockwave!",FALSE, ps->caster,0,vict,TO_VICT);
		act("$N is hit by the shockwave!",TRUE,ps->caster,0,vict,TO_NOTVICT);
		act(show_dam_check(ps->caster,"$N is hit by the shockwave!",dam),FALSE,ps->caster,0,vict,TO_CHAR);
	      } else {
		stop_fighting(ps->caster);
		sprintf(buf, "%s is hit by the shockwave!\n\r", GET_NAME(vict));
		send_to_room_except_formatted(buf,ps->in_room,vict);
		send_to_char_formatted("You are hit by the shockwave!\n\r", vict);
	      }

	    damage(ps->caster,vict,dam,SKILL_PULSE);
	    break;
	  case -1:
	    /* third wave */
	    if (number(1,101)>ps->caster->skills[SKILL_PULSE].learned) {
	      dam = 0;

	      if (ps->caster->in_room == ps->in_room) {
		act("You barely escape the sudden release of energy!",FALSE, ps->caster,0,vict,TO_VICT);
		act("$N barely escape the sudden release of energy!",TRUE,ps->caster,0,vict,TO_NOTVICT);
		act(show_dam_check(ps->caster,"$N barely escape the sudden release of energy!",dam),FALSE,ps->caster,0,vict,TO_CHAR);
	      } else {
		stop_fighting(ps->caster);
		sprintf(buf, "%s barely escape the sudden release of energy!\n\r", GET_NAME(vict));
		send_to_room_except_formatted(buf,ps->in_room,vict);
		send_to_char_formatted("You barely escape the sudden release of energy!\n\r", vict);
	      }
	    } else {
	      dam = dice(GetMaxLevel(ps->caster), 20);

	      if (ps->caster->in_room == ps->in_room) {
		act("$CRYour flesh is ripped apart by the sudden release of energy!$CN",FALSE, ps->caster,0,vict,TO_VICT);
		act("$CR$N's flesh is ripped apart by the sudden release of energy!$CN",TRUE,ps->caster,0,vict,TO_NOTVICT);
		act(show_dam_check(ps->caster,"$CR$N's flesh is ripped apart by the sudden release of energy!$CN",dam),FALSE,ps->caster,0,vict,TO_CHAR);
	      } else {
		stop_fighting(ps->caster);
		sprintf(buf, "$CR%s's flesh is ripped apart by the sudden release of energy!$CN\n\r", GET_NAME(vict));
		send_to_room_except_formatted(buf,ps->in_room,vict);
		send_to_char_formatted("$CRYour flesh is ripped apart by the sudden release of energy!$CN\n\r", vict);
	      }
	    }
	    damage(ps->caster,vict,dam,SKILL_PULSE);
	    break;

	  default:
	    log_msg("Unknown SKILL_PULSE event.");
	    break;
	  } // end of switch
    } else {
      /* Show Pulse effect in zone */
      if(real_roomp(ps->in_room)->zone == real_roomp(vict->in_room)->zone) {
	if(psex->left > 0)
	  act("You hear a deafening boom in the distance.",
	      FALSE,vict,0,0,TO_CHAR);
	else if(psex->left == 0)
	  act("You hear a large whooshing sound in the distance.",
	      FALSE,vict,0,0,TO_CHAR);
	else if(psex->left == -1)
	  act("The floor jumps as an earth shattering boom rocks the area.",
	      FALSE,vict,0,0,TO_CHAR);
      }
    }
  }

  END_AITER(iter);

  switch(psex->left) {
  case 1:
    psex->left--;
    spevent_renew(ps, 2);
    return 0;
    break;
  case 0:
    psex->left--;
    spevent_renew(ps, 5);
    return 0;
    break;
  case -1:
    spevent_destroy(ps);
    return 0;
    break;
  default:
    break;
  }

  return 0;
}

ACMD(do_pulse_skill) {
  char buf[MAX_INPUT_LENGTH];

  if(!(can_do(ch, SKILL_PULSE)))
    return;


  if(IS_SET(RM_FLAGS(ch->in_room), PEACEFUL)) {
    send_to_char_formatted("$CWIt's way too peaceful here for such an action.$CN\n\r", ch);
    return;
  }

  if(!SkillChance(ch, ch, 100, 0, 0, SKILL_PULSE)) {
    send_to_char("You fail to create a pulse of energy.\n\r", ch);
    charge_mana(ch, SKILL_PULSE, FALSE);
  } else {
    charge_mana(ch, SKILL_PULSE, TRUE);

    spevent_type *se;

    se = spevent_new();

    CREATE(se->extra, pulse_extra, 1);
    pulse_extra *sex = (pulse_extra *) se->extra;

    send_to_char("You let off a rather large pulse of energy\n\r", ch);
    sprintf(buf, "%s lets off a large pulse of energy!\n\r", GET_NAME(ch));
    send_to_room_except(buf, ch->in_room, ch);

    se->caster = ch;
    se->type = SKILL_PULSE;
    se->in_room = ch->in_room;
    se->duration = 3;
    se->expire_func = (event_func) pulse_effect;
    sex->left = 1;

    spevent_depend_char(se, ch);
    spevent_start(se);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*4);
}

struct aura_skill_struct;

typedef int (*aura_dam_func)(struct spevent_type *as, struct char_data *vict);

struct aura_skill_struct {
  char name[MAX_INPUT_LENGTH];
  char miss[MAX_INPUT_LENGTH];
  char hit[MAX_INPUT_LENGTH];
  char miss_room[MAX_INPUT_LENGTH];
  char hit_room[MAX_INPUT_LENGTH];
  char miss_caster[MAX_INPUT_LENGTH];
  char hit_caster[MAX_INPUT_LENGTH];
  int chance;
  int tic;
  int leech;
  aura_dam_func dam_func;
};

static int fire_aura_event (struct spevent_type *as, long now);
static int  ice_aura_event (struct spevent_type *as, long now);
static int acid_aura_event (struct spevent_type *as, long now);
static int decomp_aura_event (struct spevent_type *as, long now);
static int protect_aura_event (struct spevent_type *as, long now);
static int light_aura_event (struct spevent_type *as, long now);
static int reflect_aura_event (struct spevent_type *as, long now);
static int nova_aura_event (struct spevent_type *as, long now);

static int fire_aura_dam (struct spevent_type *as, struct char_data *vict);
static int  ice_aura_dam (struct spevent_type *as, struct char_data *vict);
static int acid_aura_dam (struct spevent_type *as, struct char_data *vict);
static int decomp_check  (struct spevent_type *as);
static int protect_check (struct spevent_type *as);
static int light_aura_dam (struct spevent_type *as, struct char_data *vict);
static int reflect_check (struct spevent_type *as);
static int nova_aura_dam (struct spevent_type *as, struct char_data *vict);

int error_aura_event(struct spevent_type *as, long now) {
  /* this function is so it doesn't crash the mud on an illegal event... */
  send_to_char("Ahh! ERROR. Please contact an admin and tell him how you did this...\n\r", as->caster);

  spevent_destroy(as);
  return 0;
}

//change this if you want to allow players to have more than 1 aura
//at a time.
#define MULTIPLE_AURAS 0

ACMD(do_aura) {
  struct spevent_list *tempas=NULL;
  char buf[MAX_INPUT_LENGTH];
  int cc=0;

  struct aura_choice_type {
    int level;
    char name[80];
    char hit[80];
    char miss[80];
    char hit_room[80];
    char miss_room[80];
    char hit_caster[80];
    char miss_caster[80];
    int init_mana;
    int mana;
    int snum;
    aura_dam_func dam_func;
    event_func proc;
  } choices[] = {

    { 20, "fire",
      "$CrYou are enveloped in $n's fire aura!$CN",
      "$Cr%s's fire aura swirls around you harmlessly!$CN",
      "$Cr$N is enveloped in $n's fire aura!$CN",
      "$Cr%s's fire aura swirls around $N harmlessly!$CN",
      "$Cr$N is enveloped in your fire aura!$CN",
      "$CrYour fire aura swirls around $N harmlessly!$CN",
      10, 05, SKILL_FIRE_AURA,
      fire_aura_dam, (event_func) fire_aura_event },

    { 30, "ice",
      "$CbYou are chilled by $n's ice aura!$CN",
      "$Cb$n's ice aura melts away from you harmlessly!$CN",
      "$Cb$N are chilled by $n's ice aura!$CN",
      "$Cb$n's ice aura melts away from $N harmlessly!$CN",
      "$Cb$N are chilled by your ice aura!$CN",
      "$CbYour ice aura melts away from $N harmlessly!$CN",
      15, 10, SKILL_ICE_AURA,
      ice_aura_dam,  (event_func) ice_aura_event },

    { 40, "acid",
      "$CyYour skin corrodes from $n's acid aura!$CN",
      "$CyYou are protected from $n's acid aura!$CN",
      "$Cy$N's skin corrodes from $n's acid aura!$CN",
      "$Cy$N's are protected from $n's acid aura!$CN",
      "$Cy$N's skin corrodes from your acid aura!$CN",
      "$Cy$N's are protected from your acid aura!$CN",
      20, 15, SKILL_ACID_AURA,
      acid_aura_dam, (event_func) acid_aura_event },

    { 50, "decompose",
      "","",
      "","",
      "","",
      25, 5, SKILL_DECOMPOSE_AURA,
      NULL, (event_func) decomp_aura_event },

    { 60, "protection",
      "","",
      "","",
      "","",
      30, 10, SKILL_PROTECT_AURA,
      NULL, (event_func) protect_aura_event },

    { 70, "lightning",
      "$CyYour skin electrifies from $n's lightning aura!$CN",
      "$CyYou are protected from $n's lightning aura!$CN",
      "$Cy$N's skin electrifies from $n's lightning aura!$CN",
      "$Cy$N is protected from $n's lightning aura!$CN",
      "$Cy$N's skin electrifies from your lightning aura!$CN",
      "$Cy$N is protected from your lightning aura!$CN",
      35, 20, SKILL_LIGHT_AURA,
      light_aura_dam, (event_func) light_aura_event },

    { 80, "reflection",
      "","",
      "","",
      "","",
      40, 15, SKILL_REFLECT_AURA,
      NULL, (event_func) reflect_aura_event },

    { 100, "nova",
      "$CyYour skin fries from $n's nova aura!$CN",
      "$CyYou are protected from $n's nova aura!$CN",
      "$Cy$N's skin fries from $n's nova aura!$CN",
      "$Cy$N is protected from $n's nova aura!$CN",
      "$Cy$N's skin fries from your nova aura!$CN",
      "$Cy$N is protected from your nova aura!$CN",
      45, 25, SKILL_NOVA_AURA,
      nova_aura_dam, (event_func) nova_aura_event },

    //leave this one last...
    { 0, "error",
      "", "",
      "", "",
      "", "",
      0, 0, 0,
      (aura_dam_func) NULL, (event_func) error_aura_event }
  };

  if (!can_do(ch, SKILL_AURA))
    return;

  while(*arg == ' ') arg++;

  if(!*arg) {
    send_to_char("You must specify which aura you'd like to activate:\n\r", ch);
    cprintf(ch, "%-12s %-12s %-7s %-7s %-7s\n\r", "Name", "Status", "Min Lvl", "Mana", "Leech");
    while(choices[cc].level) {
      tempas = spevent_on_char(ch, choices[cc].snum);
      sprintf(buf, "%s%-12s %-12s %-7i %-7i %-7i$CN\n\r",
	      (GetMaxLevel(ch) > choices[cc].level)?"$Cw":"$Cr",
	      choices[cc].name,
	      (tempas)?"*Activated*":"", choices[cc].level,
	      choices[cc].init_mana, choices[cc].mana);
      send_to_char_formatted(buf, ch);
      cc++;
    }
    return;
  }

  int found = 2;
  int tcc=0;
  cc=0;
  while(choices[tcc].level) {
    tempas = spevent_on_char(ch, choices[tcc].snum);
    if(is_abbrev(arg, choices[tcc].name)) {
      if(tempas)
	found=1;
      else
	found=0;
      cc = tcc;
    } else {
      if (tempas && !IS_IMMORTAL(ch) && !MULTIPLE_AURAS) {
	found=3;
	break;
      }
    }
    tcc++;
  }

  if(found==3) {
    send_to_char("You may not have multiple auras on yourself.\n\r", ch);
    return;
  }

  if(found==1) {
    sprintf(buf, "De-activating %s aura.\n\r", choices[cc].name);
    send_to_char(buf, ch);

    spevent_destroy(spevent_on_char(ch, choices[cc].snum)->sp_event);

    return;
  } else if(found==2) {
    do_aura(ch, "", 0);
    return;
  }

  if(GetMaxLevel(ch) < choices[cc].level) {
    send_to_char("You're not high enough to create that aura.\n\r", ch);
    return;
  }

  if(GET_MANA(ch) < choices[cc].mana) {
    cprintf(ch, "You do not have enough mana to bring up the aura.\n\r", ch);
    return;
  }

  GET_MANA(ch) -= choices[cc].mana;
  GET_MANA(ch) = MAX(GET_MANA(ch), 0);

  spevent_type *asp;

  asp = spevent_new();

  CREATE(asp->extra, aura_skill_struct, 1);
  aura_skill_struct *aspex = (aura_skill_struct *) asp->extra;

  asp->caster = ch;
  asp->type = choices[cc].snum;
  asp->is_pulse = 1;
  asp->duration = 20;
  asp->in_room = ch->in_room;
  asp->expire_func = (event_func) choices[cc].proc;

  aspex->chance = ch->skills[SKILL_AURA].learned;
  aspex->dam_func = choices[cc].dam_func;
  aspex->leech = choices[cc].mana;
  aspex->tic = 0;

  strcpy(aspex->name, choices[cc].name);
  strcpy(aspex->miss, choices[cc].miss);
  strcpy(aspex->hit, choices[cc].hit);
  strcpy(aspex->miss_room, choices[cc].miss_room);
  strcpy(aspex->hit_room, choices[cc].hit_room);
  strcpy(aspex->miss_caster, choices[cc].miss_caster);
  strcpy(aspex->hit_caster, choices[cc].hit_caster);

  cprintf(ch, "You create a %s aura around your body.\n\r", choices[cc].name);
  sprintf(buf, "%s creates a %s aura around %s body.\n\r", GET_NAME(ch), choices[cc].name, HSHR(ch));
  send_to_room_except(buf, ch->in_room, ch);

  spevent_depend_char(asp, ch);
  spevent_start(asp);
}

int general_aura_event(struct spevent_type *as, long now) {
  struct char_data *vict=NULL, *next=NULL;
  char buf[MAX_INPUT_LENGTH];
  int dam;
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;

  if(!as) {
    log_msg("For some reason, as = NULL (general_aura_event)");
    return 0;
  }

  if(as->caster->in_room < 0) {
    return 0;
  }

  if(GET_MANA(as->caster) < asex->leech) {
    cprintf(as->caster, "You no longer have enough mana to upkeep your %s aura.\n\r", asex->name);
    return 0;
  }

  if((real_roomp(as->caster->in_room)->zone != real_roomp(as->in_room)->zone) && !IS_IMMORTAL(as->caster))
    return 0;

  if(!check_peaceful(as->caster, "")) {
    for(vict=real_roomp(as->caster->in_room)->people; vict; vict=next) {
      next=vict->next_in_room;
	if(can_hurt(as->caster, vict)) {
	  if(asex->dam_func)
	    dam = (asex->dam_func)(as, vict);
	  else {
	    log_msg("aura has no damage function?!!");
	    dam = 0;
	  }
	  if(!DamageTrivia(as->caster, vict, dam, as->type)) {
	    act(asex->miss,FALSE, as->caster,0,vict,TO_VICT);
	    act(asex->miss_room,TRUE,as->caster,0,vict,TO_NOTVICT);
	    act(show_dam_check(as->caster,asex->miss_caster,dam),FALSE,as->caster,0,vict,TO_CHAR);
	  } else {
	    act(asex->hit,FALSE, as->caster,0,vict,TO_VICT);
	    act(asex->hit_room,TRUE,as->caster,0,vict,TO_NOTVICT);
	    act(show_dam_check(as->caster,asex->hit_caster,dam),FALSE,as->caster,0,vict,TO_CHAR);
	  }
	  damage(as->caster, vict, dam, as->type);
	}
    }
  } else {
    cprintf(as->caster, "Do to the peaceful nature of the area, your %s aura dissipates.\n\r", asex->name);
    return 0;
  }

  asex->tic++;
  asex->tic %= 3;
  if(!asex->tic) {
    GET_MANA(as->caster) -= asex->leech;
    GET_MANA(as->caster) = MAX(GET_MANA(as->caster),0);
  }
  return 1;
}

static int fire_aura_dam(struct spevent_type *as, struct char_data *vict) {
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;

  int dam;

  dam = dice(GetMaxLevel(as->caster), 3) / 2;
  dam = MAX(dam, 1);

  if(number(1,101) > asex->chance)
    dam = 0;

  return dam;
}

static int fire_aura_event(struct spevent_type *as, long now) {
  if(general_aura_event(as, now)) {
    spevent_renew(as, 20);
  } else {
    send_to_char("Your fire aura has been extinguished.\n\r", as->caster);
    spevent_destroy(as);
  }

  return 0;
}

int ice_aura_dam(struct spevent_type *as, struct char_data *vict) {
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;

  int dam;

  dam = dice(GetMaxLevel(as->caster), 4) / 2;
  dam = MAX(dam, 1);

  if(number(1,101) > asex->chance)
    dam = 0;

  return dam;
}

static int ice_aura_event(struct spevent_type *as, long now) {
  if(general_aura_event(as, now)) {
    spevent_renew(as, 24);
  } else {
    send_to_char("Your ice aura has melted away.\n\r", as->caster);
    spevent_destroy(as);
  }

  return 0;
}

static int acid_aura_dam(struct spevent_type *as, struct char_data *vict) {
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;
  int dam;

  dam = dice(GetMaxLevel(as->caster), 5) / 2;
  dam = MAX(dam, 1);

  if(number(1,101) > asex->chance)
    dam = 0;

  return dam;
}

int acid_aura_event(struct spevent_type *as, long now) {
  if(general_aura_event(as, now)) {
    spevent_renew(as, 28);
  } else {
    send_to_char("Your acid aura has been neutralized.\n\r", as->caster);
    spevent_destroy(as);
  }
  return 0;
}

static int light_aura_dam(struct spevent_type *as, struct char_data *vict)
{
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;

  int dam;

  dam = dice(GetMaxLevel(as->caster), 7) / 2;
  dam = MAX(dam, 1);

  if(number(1,101) > asex->chance)
    dam = 0;

  return dam;
}

int light_aura_event(struct spevent_type *as, long now) {
  if(general_aura_event(as, now)) {
    spevent_renew(as, 28);
  } else {
    send_to_char("Your lightning aura has been neutralized.\n\r", as->caster);
    spevent_destroy(as);
  }
  return 0;
}

static int nova_aura_dam(struct spevent_type *as, struct char_data *vict)
{
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;

  int dam;

  dam = dice(GetMaxLevel(as->caster), 10) / 2;
  dam = MAX(dam, 1);

  if(number(1,101) > asex->chance)
    dam = 0;

  return dam;
}

int nova_aura_event(struct spevent_type *as, long now) {
  if(general_aura_event(as, now)) {
    spevent_renew(as, 28);
  } else {
    send_to_char("Your nova aura has been neutralized.\n\r", as->caster);
    spevent_destroy(as);
  }
  return 0;
}

int protect_aura_event(struct spevent_type *as, long now) {
  if(decomp_check(as)) {
    spevent_renew(as, 20);
  } else {
    cprintf(as->caster, "Your protection aura has ceased it's work.\n\r");
    spevent_destroy(as);
  }
  return 0;
}

int reflect_aura_event(struct spevent_type *as, long now) {
  if(decomp_check(as)) {
    spevent_renew(as, 20);
  } else {
    cprintf(as->caster, "Your reflection aura has ceased it's work.\n\r");
    spevent_destroy(as);
  }
  return 0;
}

int decomp_aura_event(struct spevent_type *as, long now) {
  if(decomp_check(as)) {
    spevent_renew(as, 20);
  } else {
    cprintf(as->caster, "Your decomposition aura has ceased it's work.\n\r");
    spevent_destroy(as);
  }
  return 0;
}

int decomp_check(struct spevent_type *as) {
  struct aura_skill_struct *asex = (aura_skill_struct *) as->extra;
  struct obj_data *obj, *toclean[256];
  int cleancount=0, typeclean[256], i;
  char buf[MAX_INPUT_LENGTH];

  if(!as) {
    log_msg("For some reason, as = NULL (decomp_check)");
    return 0;
  }

  if(GET_MANA(as->caster) < asex->leech) {
    cprintf(as->caster, "You no longer have enough mana to upkeep your decompose aura.\n\r");
    return 0;
  }

  asex->tic++;
  asex->tic %= 3;
  if(!asex->tic) {
    GET_MANA(as->caster) -= asex->leech;
    GET_MANA(as->caster) = MAX(GET_MANA(as->caster), 0);
  }

  for(obj=real_roomp(as->caster->in_room)->contents; obj; obj=obj->next_content) {
    if(IS_NPC_CORPSE(obj)) {
      toclean[cleancount] = obj;
      typeclean[cleancount] = 1;
      cleancount++;
    } else if(GET_ITEM_TYPE(obj) == ITEM_TRASH) {
      toclean[cleancount] = obj;
      typeclean[cleancount] = 2;
      cleancount++;
    } else if(obj_index[obj->item_number].virt == 15) {
      toclean[cleancount] = obj;
      typeclean[cleancount] = 2;
      cleancount++;
    }
    if(cleancount > 255) break;
  }

  for(i=0;i<cleancount;i++) {
    obj=toclean[i];
    switch(typeclean[i]) {
    case 1:
      ObjFromCorpse(obj);
      break;
    case 2:
      extract_obj(obj);
      break;
    }
    GET_HIT(as->caster) += GetMaxLevel(as->caster)/5;
    GET_HIT(as->caster) = MIN(GET_MAX_HIT(as->caster), GET_HIT(as->caster));
  }

  if(cleancount) {
    cprintf(as->caster, "$CyYour decompose aura breaks down all of the dead matter in the room.\n\r");
    sprintf(buf, "$CyThe dead matter in the room breaks down because of %s's influence.\n\r", GET_NAME(as->caster));
    send_to_room_except_formatted(buf, as->caster->in_room, as->caster);
  }

  return 1;
}

static int sleep_trap(spevent_type *se, long now);
static int fire_trap(spevent_type *se, long now);
static int poison_trap(spevent_type *se, long now);

ACMD(do_set_trap) {
  char buf[MAX_INPUT_LENGTH];
  int cc, found;
  struct char_data *vict;

  struct trap_info_struct {
    char name[MAX_INPUT_LENGTH];
    int ingr;
    int delay;
    event_func func;
  } trap_info[] = {
    { "sleep",  NOTHING, 10, (event_func) sleep_trap },
    { "fire",   NOTHING, 12, (event_func) fire_trap },
    { "poison", NOTHING,  8, (event_func) poison_trap },
    { "" },
  };

#if 0
  if (!can_do(ch, SKILL_SET_TRAP))
    return;
#endif

  while(*arg == ' ') arg++;

  cc=0;

  if(!*arg) {
    send_to_char("You have to pick a trap type to set:\n\r", ch);
    while(*trap_info[cc].name) {
      sprintf(buf, "\t%s trap\n\r", trap_info[cc].name);
      send_to_char(buf, ch);
      cc++;
    }
    return;
  }

  if(spevent_on_char(ch, SKILL_SET_TRAP) && !IS_IMMORTAL(ch)) {
    send_to_char("You can't lay more than 1 trap at once.\n\r", ch);
    return;
  }

  found=0;
  cc=0;
  while(*trap_info[cc].name) {
    if(!strcmp(trap_info[cc].name, arg)) {
      found=1;
      break;
    }
    cc++;
  }

  if(!found) {
    do_set_trap(ch, "", 0);
    return;
  }

  spevent_type *se;

  se = spevent_new();

  se->in_room = ch->in_room;
  se->caster = ch;
  se->duration = trap_info[cc].delay;
  se->expire_func = trap_info[cc].func;

  spevent_depend_char(se, ch);
  spevent_start(se);

  sprintf(buf, "You carefully lay a %s trap in the room.\n\r", trap_info[cc].name);
  send_to_char(buf, ch);

  sprintf(buf, "You glimpse %s creating a trap in the room!\n\r", GET_NAME(ch));
  for(vict=real_roomp(ch->in_room)->people;vict;vict=vict->next_in_room) {
    if((ch!=vict) && (1==1))
      send_to_char(buf, vict);
  }

  return;
}

static int sleep_trap(spevent_type *se, long now) {
  send_to_char("Your sleep trap goes off...\n\r", se->caster);
  spevent_destroy(se);
  return 0;
}

static int fire_trap(spevent_type *se, long now) {
  send_to_char("Your fire trap goes off...\n\r", se->caster);
  spevent_destroy(se);
  return 0;
}

static int poison_trap(spevent_type *se, long now) {
  send_to_char("Your poison trap goes off...\n\r", se->caster);
  spevent_destroy(se);
  return 0;
}

static int probe_check(spevent_type *se, long now);

void probe_info(char_data *ch, char_data *vict) {
  int lvl=GetMaxLevel(ch);
  char buf[MAX_STRING_LENGTH];

  charge_mana(ch, SKILL_PROBE, TRUE);

  cprintf(ch, "Probing %s:\n\r\n\r", GET_NAME(vict));
  if(lvl < 20) {
    if(10*GET_HIT(ch) < GET_HIT(vict)) {
      cprintf(ch, "HP: $CwMore than you could get in a while...$CN\n\r");
    } else if(5*GET_HIT(ch) < GET_HIT(vict)) {
      cprintf(ch, "HP: $CwA -REAL- lot more than you.$CN\n\r");
    } else if(2*GET_HIT(ch) < GET_HIT(vict)) {
      cprintf(ch, "HP: $CwA lot more than you.$CN\n\r");
    } else if(GET_HIT(ch) < GET_HIT(vict)) {
      cprintf(ch, "HP: $CwMore than you.$CN\n\r");
    } else if(GET_HIT(ch) > 2*GET_HIT(vict)) {
      cprintf(ch, "HP: $CwA lot less than you.$CN\n\r");
    } else if(GET_HIT(ch) > GET_HIT(vict)) {
      cprintf(ch, "HP: $CwLess than you.$CN\n\r");
    }
  } else if(lvl < 50) {
    cprintf(ch, "HP: $Cw%i$CN\n\r", GET_HIT(vict));
  } else {
    cprintf(ch, "HP: $Cw%i / %i$CN\n\r", GET_HIT(vict), GET_MAX_HIT(vict));
  }

  if(lvl >= 30) {
    if(!IS_SET(vict->specials.flags, PLR_WIMPY)) {
      cprintf(ch, "Wimpy: $CBWon't run!$CN\n\r");
    } else {
      cprintf(ch, "Wimpy: $CBDefault$CN\n\r");
    }
    cprintf(ch, "\n\r");
    cprintf(ch, "Str Dex Int Con Wis Cha\n\r");
    cprintf(ch, "$CR%3i %3i %3i %3i %3i %3i$CN\n\r",
	    GET_STR(vict), GET_DEX(vict), GET_INT(vict),
	    GET_CON(vict), GET_WIS(vict), GET_CHA(vict));
  }

  if(lvl >= 40) {
    cprintf(ch, "\n\r");
    cprintf(ch, "Align: $CW%i$CN\n\r", GET_ALIGNMENT(vict));
    cprintf(ch, "Gold: $CY%i$CN\n\r", GET_GOLD(vict));
    cprintf(ch, "Exp: $CM%i$CN\n\r", GET_EXP(vict));
  }

  if(lvl >= 50) {
    cprintf(ch, "\n\r");
    cprintf(ch, "Carrying:\n\r");
    list_obj_in_heap(vict->carrying, ch);
  }

  if(lvl >= 75) {
    cprintf(ch, "\n\r");
    cprintf(ch, "Affects:\n\r----------------\n\r");
    if(AFF_FLAGS(vict)) {
      sprintbit(AFF_FLAGS(vict), affected_bits, buf);
      cprintf(ch, "%s", buf);
    }
    if(AFF2_FLAGS(vict)) {
      sprintbit(AFF2_FLAGS(vict), affected2_bits, buf);
      cprintf(ch, "%s", buf);
    }
    if(!AFF_FLAGS(vict) && !AFF2_FLAGS(vict)) {
      cprintf(ch, "NONE");
    }
    cprintf(ch, "\n\r");
  }

  if(lvl >= 100) {
    cprintf(ch, "\n\r");
    cprintf(ch, "Saving Throws:\n\r");
    cprintf(ch, " Paralyzation:  %2i\n\r", vict->specials.apply_saving_throw[0]);
    cprintf(ch, " Rod:           %2i\n\r", vict->specials.apply_saving_throw[1]);
    cprintf(ch, " Petrification: %2i\n\r", vict->specials.apply_saving_throw[2]);
    cprintf(ch, " Breath:        %2i\n\r", vict->specials.apply_saving_throw[3]);
    cprintf(ch, " Spell:         %2i\n\r", vict->specials.apply_saving_throw[4]);
  }

  if(lvl >= 115) {
    cprintf(ch, "\n\r");
    sprintbit(vict->M_immune, immunity_names, buf);
    cprintf(ch, "Immune to:\n\r  $Cw%s$CN\n\r", buf);
    sprintbit(vict->immune, immunity_names, buf);
    cprintf(ch, "Resistant to:\n\r  $Cw%s$CN\n\r", buf);
    sprintbit(vict->susc, immunity_names, buf);
    cprintf(ch, "Susceptible to:\n\r  $Cw%s$CN\n\r", buf);
  }

  return;
}

ACMD(do_probe) {
  struct char_data *vict, *caster[10];
  char target[MAX_INPUT_LENGTH];
  int casters, i;
  spevent_list *slist, *snext;

  if(!can_do(ch, SKILL_PROBE))
    return;

  if(spevent_on_char(ch, SKILL_PROBE)) {
    cprintf(ch, "You're currently using probe right now.\n\r");
    return;
  }

  casters = 0;
  for(vict=real_roomp(ch->in_room)->people; vict; vict=vict->next_in_room) {
    SPEVENT_CHECK_CHAR(slist,snext,vict,SKILL_PROBE) {
      SPEVENT_CAST(slist,snext,vict,SKILL_PROBE);
      if(casters >= 10) break;
      caster[casters++] = vict;
      break;
    }
  }

  if(!casters) {
    while(*arg==' ') arg++;

    if(!*arg) {
      cprintf(ch, "Syntax is: probe <monster name>\n\r");
      return;
    }

    one_argument(arg, target);

    if(!(vict=get_char_room_vis(ch, target))) {
      cprintf(ch, "You can't find %s here!\n\r", target);
      return;
    }

    if(IS_PC(vict) || IS_POLY_PC(vict)) {
      cprintf(ch, "You can't use probe on other players!\n\r");
      return;
    }
  }

  if(!SkillChance(ch, vict, 100, 0,
		  SPLMOD_INT | SPLMOD_WIS | SPLMOD_SPELLSAVE,
		  SKILL_PROBE)) {
    cprintf(ch, "You try to probe %s, but fail probe their conscience.\n\r", GET_NAME(vict));
    charge_mana(ch, SKILL_PROBE, FALSE);
    return;
  }

  if(casters > 0) {
    SPEVENT_CHECK_CHAR(slist,snext,caster[0],SKILL_PROBE) {
      SPEVENT_CAST(slist,snext,caster[0],SKILL_PROBE);
      vict = slist->sp_event->victim;
      break;
    }

    for(i=0;i<casters;i++) {
      probe_info(caster[i], vict);
      charge_mana(caster[i], SKILL_PROBE, TRUE);

      SPEVENT_CHECK_CHAR(slist,snext,caster[i],SKILL_PROBE) {
	SPEVENT_CAST(slist,snext,caster[i],SKILL_PROBE);
	spevent_destroy(slist->sp_event);
      }

      WAIT_STATE(caster[i], PULSE_VIOLENCE*2);
    }

    probe_info(ch, vict);
    charge_mana(ch, SKILL_PROBE, TRUE);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    return;
  }

  spevent_type *se;

  se = spevent_new();

  se->type = SKILL_PROBE;
  se->in_room = ch->in_room;
  se->caster = ch;

  if(casters) {
    SPEVENT_CHECK_CHAR(slist,snext,caster[0],SKILL_PROBE) {
      SPEVENT_CAST(slist,snext,caster[0],SKILL_PROBE);
      se->victim = slist->sp_event->victim;
      break;
    }
  } else {
    se->victim = vict;
  }

  se->is_pulse = 1;
  se->duration = 1;
  se->expire_func = (event_func) probe_check;

  spevent_depend_char(se, ch);
  spevent_depend_char(se, vict);
  spevent_start(se);

  cprintf(ch, "Waiting for another psi to probe %s.\n\r", GET_NAME(vict));
}

static int probe_check(spevent_type *se, long now) {
  if(se->caster->in_room == se->in_room) {
    spevent_renew(se);
  } else {
    spevent_destroy(se);
  }

  return 0;
}

static int group_attack_check(spevent_type*,long);

ACMD(do_group_attack) {
  char buf[MAX_STRING_LENGTH];
  char_data *vict, *attackers[100];
  int num_attackers, szdice, avglvl, i, dam;
  int dead;
  spevent_list *slist, *snext;

  if(!can_do(ch, SKILL_GROUP_ATTACK))
    return;

  num_attackers=1;
  avglvl=GetMaxLevel(ch);
  for(vict=real_roomp(ch->in_room)->people;vict;vict=vict->next_in_room) {
    if(!in_group(ch, vict))
      continue;

    SPEVENT_CHECK_CHAR(slist,snext,vict,SKILL_GROUP_ATTACK) {
      SPEVENT_CAST(slist,snext,vict,SKILL_GROUP_ATTACK);
      if(num_attackers >= 100) break;

      avglvl += GetMaxLevel(vict);
      attackers[num_attackers-1] = vict;
      num_attackers++;

      break;
    }
  }
  avglvl /= num_attackers;

  while(*arg == ' ') arg++;

  if(!SkillChance(ch, ch, 100, 0, 0, SKILL_GROUP_ATTACK)) {
    cprintf(ch, "You try to execute an ambush, but fail utterly.\n\r");
    charge_mana(ch, SKILL_GROUP_ATTACK, FALSE);
    return;
  }

  if(*arg) {
    only_argument(arg, buf);

    if(!(vict = get_char_room_vis(ch, buf))) {
      send_to_char("Nobody here by that name.\n\r", ch);
      return;
    }

    if((ch == vict) || in_group(ch, vict) || !can_pkill(ch, vict)) {
      cprintf(ch, "You can't attack that person.\n\r");
      return;
    }

    szdice = 3*num_attackers;
    dam = dice(avglvl, szdice);

    cprintf(ch, "$CBYou rush out to ambush %s!$CN\n\r", GET_NAME(vict));
    cprintf(vict, "$CB%s rushes out and attacks you!$CN\n\r", GET_NAME(ch));
    sprintf(buf, "$CB%s rushes out and ambushes %s!$CN\n\r", GET_NAME(ch), GET_NAME(vict));
    send_to_room_except_two_formatted(buf, ch->in_room, ch, vict);
    dead=damage(ch, vict, dam, SKILL_GROUP_ATTACK);
    charge_mana(ch, SKILL_GROUP_ATTACK, TRUE);
    WAIT_STATE(ch, PULSE_VIOLENCE*2);

    for(i=0;i<num_attackers-1;i++) {
      SPEVENT_CHECK_CHAR(slist,snext,attackers[i],SKILL_GROUP_ATTACK) {
	SPEVENT_CAST(slist,snext,attackers[i],SKILL_GROUP_ATTACK);

	spevent_destroy(slist->sp_event);
      }

      if(attackers[i] == ch) continue;

      if(dead) continue;
      dam = dice(avglvl, szdice);
      cprintf(attackers[i], "$CBYou rush out to ambush %s$CN!\n\r", GET_NAME(vict));
      sprintf(buf, "$CB%s rushes out and ambushes %s!$CN\n\r", GET_NAME(attackers[i]), GET_NAME(vict));
      send_to_room_except_two_formatted(buf, ch->in_room, attackers[i], vict);
      cprintf(vict, "$CB%s rushes out and attacks you!$CN\n\r", GET_NAME(attackers[i]));
      dead=damage(ch, vict, dam, SKILL_GROUP_ATTACK);
      charge_mana(attackers[i], SKILL_GROUP_ATTACK, TRUE);
      WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }

    return;
  }

  SPEVENT_CHECK_CHAR(slist,snext,ch,SKILL_GROUP_ATTACK) {
    SPEVENT_CAST(slist,snext,ch,SKILL_GROUP_ATTACK);
    cprintf(ch, "You are already preparing to group attack!\n\r");
    return;
  }

  cprintf(ch, "You are now ready to group attack someone.\n\r");

  spevent_type *se;

  se = spevent_new();

  se->type = SKILL_GROUP_ATTACK;
  se->in_room = ch->in_room;
  se->caster = ch;
  se->duration = 1;
  se->extra = NULL;
  se->expire_func = (event_func) group_attack_check;

  spevent_depend_char(se, ch);
  spevent_start(se);
}

int group_attack_check(spevent_type *se, long now) {
  if(se->caster->in_room == se->in_room) {
    spevent_renew(se);
  } else {
    spevent_destroy(se);
  }

  return 0;
}

static int charge_elements_check(spevent_type*,long);

struct charge_elements_charges {
  int num;
  int lvl;
  int str;
};

struct charge_elements_extra {
  int on;
  int tics;
  charge_elements_charges charge[10];
  int num_charges;
};

ACMD(do_charge_elements) {
  spevent_list *slist;
  spevent_type *se=NULL;
  charge_elements_extra *sex=NULL;
  int i;
  spell_info *spell;

  if(!can_do(ch, SKILL_CHARGE_ELEMENTS))
    return;

  slist = spevent_on_char(ch, SKILL_CHARGE_ELEMENTS);
  if(slist) {
    se=slist->sp_event;
    sex = (charge_elements_extra *) se->extra;
  }

  while(*arg == ' ') arg++;

  if(!*arg) {
    if(se && sex->on) {
      cprintf(ch, "You are currently absorbing elemental spells.\n\r");
    } else {
      cprintf(ch, "You are not currently absorbing elemental spells.\n\r");
    }

    cprintf(ch, "For help, type: $CRcharge help$CN.\n\r");
    return;
  }

  if(!strcmp(arg, "help")) {
    cprintf(ch, "To start absorbing elemental spells, type $CRcharge on$CN.\n\r");
    cprintf(ch, "To stop absorbing elemental spells, type $CRcharge off$CN.\n\r");
    cprintf(ch, "To see what spells you have charged, type $CRcharge rep$CN.\n\r");
    cprintf(ch, "For this help screen, type $CRcharge help$CN.\n\r");
    return;
  }

  if(!strcmp(arg, "off")) {
    if(se && sex->on) {
      sex->on = 0;
      cprintf(ch, "You are no longer absorbing elemental spells.\n\r");
    } else {
      cprintf(ch, "You weren't absorbing elemental spells in the first place!\n\r");
    }
    return;
  }

  if(!strcmp(arg, "rep")) {
    if(!se) {
      cprintf(ch, "You aren't absorbing elemental spells, so you have no charges.\n\r");
      return;
    }

    cprintf(ch, "You have absorbed the following charges:\n\r");
    if(!sex->num_charges) {
      cprintf(ch, "  You have no charges yet.\n\r");
    } else {
      for(i=0;i<sex->num_charges;i++) {
	spell = spell_by_number(sex->charge[i].num);
	if(!spell)
	  cprintf(ch, "  Illegal spell number! (%d)\n\r", sex->charge[i].num);
	else
	  cprintf(ch, "  %s%s%s\n\r",
		  (sex->charge[i].str==2)?"$CR":"",
		  spell->name,
		  (sex->charge[i].str==2)?"$CN":"");
      }
    }

    return;
  }

  if(!strcmp(arg, "on")) {
    if(se && sex->on) {
      cprintf(ch, "You are already absorbing elemental spells.\n\r");
      return;
    }

    if(!SkillChance(ch, ch, 100, 0, 0, SKILL_CHARGE_ELEMENTS)) {
      cprintf(ch, "You try to absorb elemental spells, but fail horribly.\n\r");
      charge_mana(ch, SKILL_CHARGE_ELEMENTS, FALSE);
      return;
    }

    if(!se) {
      se = spevent_new();

      se->type = SKILL_CHARGE_ELEMENTS;
      se->caster = ch;
      se->is_pulse = 1;
      se->duration = 1;
      se->expire_func = (event_func) charge_elements_check;

      CREATE(se->extra, charge_elements_extra, 1);
      sex = (charge_elements_extra *) se->extra;

      sex->tics = 0;
      sex->num_charges = 0;

      spevent_depend_char(se, ch);
      spevent_start(se);
    }

    sex->on = 1;

    charge_mana(ch, SKILL_CHARGE_ELEMENTS, TRUE);
    cprintf(ch, "You are now absorbing elemental spells.\n\r");
    WAIT_STATE(ch, PULSE_VIOLENCE*1);
    return;
  }

  cprintf(ch, "Illegal argument: type $CRcharge help$CN for help.\n\r");
}

static int charge_list[][2] = {
  { SPELL_FLAMESTRIKE, 12 },
  { SPELL_FIREBALL, 18 },

  { SPELL_FROST_CLOUD, 15 },

  { SPELL_ACID_BLAST, 15 },

  { SPELL_ELECTROCUTE, 12 },
  { SPELL_CALL_LIGHTNING, 20 },
  { 0, 0 },
};

int charge_elements_check(spevent_type *se, long now) {
  spevent_list *slist, *snext;
  charge_elements_extra *sex = (charge_elements_extra *) se->extra;
  struct char_data *ch;
  struct spell_info *spell;
  int i, max_charges, slvl;

  if(!sex->on) {
    spevent_renew(se);
    return 0;
  }

  if(!((++sex->tics)%PULSE_PER_MUD_HOUR)) {
    sex->tics=0;

    charge_mana(se->caster, SKILL_CHARGE_ELEMENTS, TRUE);
    if(GET_MANA(se->caster) <= 0) {
      cprintf(se->caster, "Your elemental absorbtion has run out!\n\r");
      spevent_destroy(se);
      return 0;
    }
  }

  if(IS_GOD(se->caster)) {
    max_charges = 10;
  } else {
    max_charges = 10 * GetMaxLevel(se->caster) / 125;
  }

  if(sex->num_charges < max_charges) {
    for(i=0;charge_list[i][0];i++) {
      spell = spell_by_number(charge_list[i][0]);
      slvl = MAX(MIN(spell->min_level[MAGE_LEVEL_IND], spell->min_level[DRUID_LEVEL_IND]), 0);

      if(slvl > GetMaxLevel(se->caster)) continue;

      for(ch=real_roomp(se->caster->in_room)->people;ch;ch=ch->next_in_room) {
	if(se->caster == ch) continue;

	SPEVENT_CHECK_CHAR(slist,snext,ch,charge_list[i][0]) {
	  SPEVENT_CAST(slist,snext,ch,charge_list[i][0]);

	  if(!in_group(ch,se->caster) &&
	     !(slist->sp_event->victim==se->caster)) continue;

	  sex->charge[sex->num_charges].num = charge_list[i][0];
	  sex->charge[sex->num_charges].lvl = slvl;
	  sex->charge[sex->num_charges].str = (slist->sp_event->victim == se->caster)?2:1;
	  sex->num_charges++;

	  if(slist->sp_event->victim == se->caster) {
	    cprintf(se->caster, "You absorb the full brunt of %s's %s!\n\r",
		    GET_NAME(ch), spell->name);
	  } else {
	    cprintf(se->caster, "You absorb %s's left over %s energy.\n\r",
		    GET_NAME(ch), spell->name);
	  }
	  spevent_destroy(slist->sp_event);
	}
	if(sex->num_charges >= max_charges) {
	  cprintf(se->caster, "$CRYou can't absorb any more elemental spells.$CN\n\r");
	  break;
	}
      }
      if(sex->num_charges >= max_charges) break;
    }
  }

  spevent_renew(se);
  return 0;
}

ACMD(do_mkick) {
  spevent_list *slist;
  spevent_type *ce=NULL;
  charge_elements_extra *cex=NULL;
  int i, dam, szdice, doall, level, dead;
  char victbuf[512], chargebuf[512];
  char_data *vict;
  spell_info *spell;

  if(!can_do(ch, SKILL_MONK_KICK))
    return;

  slist=spevent_on_char(ch, SKILL_CHARGE_ELEMENTS);
  if(slist) {
    ce=slist->sp_event;
    cex=(charge_elements_extra*) ce->extra;
  }

  while(*arg == ' ') arg++;

  arg=one_argument(arg,chargebuf);

  doall = !strcmp(chargebuf, "all");

  if(doall) {
    while(*arg == ' ') arg++;
    only_argument(arg, victbuf);
  } else
    strcpy(victbuf, chargebuf);

  if(!(vict=get_char_room_vis(ch,victbuf))) {
    cprintf(ch, "Nobody here by that name.\n\r");
    return;
  }

  if(ch == vict) {
    cprintf(ch, "It would be very unwise to kick yourself...\n\r");
    return;
  }

  if(in_group(ch, vict) || !can_pkill(ch, vict)) {
    cprintf(ch, "You can't attack that person.\n\r");
    return;
  }

  if(!SkillChance(ch, vict, 100, IMM_BLUNT,
		  SPLMOD_DEX | SPLMOD_STR,
		  SKILL_MONK_KICK)) {
    cprintf(ch, "You attempt to kick %s, but fall flat on your face.\n\r", GET_NAME(vict));
    charge_mana(ch, SKILL_MONK_KICK, FALSE);
    return;
  }
  charge_mana(ch, SKILL_MONK_KICK, TRUE);

  level = GetMaxLevel(ch);
  szdice = (15*level)/125+1;

  dam=dice(level, szdice);

  dead=damage(ch, vict, dam, SKILL_MONK_KICK);

  if(!dead && ce && cex->num_charges) {
    do {
      for(i=0;charge_list[i][0];i++)
	if(charge_list[i][0] == cex->charge[0].num)
	  break;

      spell = spell_by_number(cex->charge[0].num);
      szdice = charge_list[i][1]+1;
      dam = cex->charge[0].str * dice(level*3/4, szdice);
      dead = damage(ch, vict, dam, cex->charge[0].num);

      for(i=1;i<cex->num_charges;i++) {
	cex->charge[i-1].num=cex->charge[i].num;
	cex->charge[i-1].lvl=cex->charge[i].lvl;
	cex->charge[i-1].str=cex->charge[i].str;
      }
      cex->num_charges--;
    } while(doall && cex->num_charges && !dead);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}
