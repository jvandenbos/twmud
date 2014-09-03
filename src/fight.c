#include "config.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "spell_events.h"
#include "fight.h"
#include "utility.h"
#include "recept.h"
#include "magicutils.h"
#include "opinion.h"
#include "multiclass.h"
#include "limits.h"
#include "act.h"
#include "spec.h"
#include "spell_util.h"
#include "constants.h"
#include "mobact.h"
#include "spelltab.h"
#include "channels.h"
#include "util_str.h"
#include "interpreter.h"
#include "statistic.h"
#include "find.h"
#include "vnum.h"
#include "ansi.h"
#include "newsaves.h"
#include "hero.h"
#include "ringlog.h"
#include "proto.h"
#include "cmdtab.h"
#include "scrap.h"
#include "state.h"
#include "race.h"
#include "mobprog2.h"
#include "varfunc.h"


#define PKILL_LOG "pkill/death.log"

/* forward declarations */
void BreakLifeSaverObj( struct char_data *ch);
int BrittleCheck(struct char_data *ch, int dam);
int SkipImmortals(struct char_data *v, int amnt);
int Getw_type(struct obj_data *wielded);
void WeaponSpell(struct char_data *c, struct char_data *v,
		 int type, struct obj_data* w);
void RemNonRent(struct char_data *ch);

void make_fun_body_pieces(struct char_data *ch, struct char_data *killer);
void brag(struct char_data * ch, struct char_data * victim);
int corpse_timer(object_event*,long);
Variable *get_mob_var(struct char_data *ch, char *name);

/* Structures */
typedef int (*damage_func)(struct char_data *ch, struct char_data *v,
			   int dam, int type);

extern struct room_data *world;

int combatants = 0;
struct char_data *combat_list = 0;   /* head of l-list of fighting chars    */
struct char_data *missile_list = 0;   /* head of l-list of fighting chars    */
struct char_data *charging_list = 0;  /* head of l-list of charging chars */
struct char_data *charging_next_dude = 0; /* Next dude charging */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */

char PeacefulFlag = 1;

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",    "hits"},            /* TYPE_HIT      */
  {"pound",  "pounds"},          /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash",  "slashes"},         /* TYPE_SLASH    */
  {"whip",   "whips"},           /* TYPE_WHIP     */
  {"claw",   "claws"},           /* TYPE_CLAW     */
  {"bite",   "bites"},           /* TYPE_BITE     */
  {"sting",  "stings"},          /* TYPE_STING    */
  {"crush",  "crushes"},         /* TYPE_CRUSH    */
  {"cleave", "cleaves"},
  {"stab",   "stabs"},
  {"smash",  "smashes"},
  {"smite",  "smites"},
  {"impale", "impales"}          /* TYPE_RANGEWEAPON */
};

/* The Fight related routines */

void appear(struct char_data *ch)
{
  act("$n suddenly fades into existence.", FALSE, ch,0,0,TO_ROOM);

  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE);
}

char * show_dam_check(struct char_data *ch, char *buf, int dam)
{
  static char line[400];
  if (IS_SET(ch->specials.flags, PLR_SHOW_DAM)){
    int c = IS_SET(ch->specials.flags, PLR_COLOR) ? 1 : 0;
    char *o = (char *) (c ? ANSI_CYAN : "");
    char *n = (char *) (c ? ANSI_NORMAL : "");
    char *d;
    if (c){
      if (dam>1000) d=ANSI_RED;
      else if (dam <400) d=ANSI_WHITE;
      else d=ANSI_MAGENTA;
    }else d="";

    sprintf(line,"%s %s-=%s%s%d%s%s=-%s", buf, o, n, d, dam, n, o, n);
    return(line);
  }else{
    return(buf);
  }
}

void load_messages(void)
{
  FILE *f1;
  int i,type;
  struct message_type *messages;
  char chk[100];

  if (!(f1 = fopen(MESS_FILE, "r"))){
    perror("read messages");
    exit(0);
  }

  /*
    find the memset way of doing this...
    */

  for (i = 0; i < MAX_MESSAGES; i++)	{
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks=0;
    fight_messages[i].msg = 0;
  }

  fscanf(f1, " %s \n", chk);

  i = 0;

  while(*chk == 'M')	{
    fscanf(f1," %d\n", &type);
    
    if(i>=MAX_MESSAGES){
      log_msg("Too many combat messages.");
      exit(0);
    }

    CREATE(messages,struct message_type,1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type=type;
    messages->next=fight_messages[i].msg;
    fight_messages[i].msg=messages;

    messages->die_msg.attacker_msg      = fread_string(f1);
    messages->die_msg.victim_msg        = fread_string(f1);
    messages->die_msg.room_msg          = fread_string(f1);
    messages->miss_msg.attacker_msg     = fread_string(f1);
    messages->miss_msg.victim_msg       = fread_string(f1);
    messages->miss_msg.room_msg         = fread_string(f1);
    messages->hit_msg.attacker_msg      = fread_string(f1);
    messages->hit_msg.victim_msg        = fread_string(f1);
    messages->hit_msg.room_msg          = fread_string(f1);
    messages->god_msg.attacker_msg      = fread_string(f1);
    messages->god_msg.victim_msg        = fread_string(f1);
    messages->god_msg.room_msg          = fread_string(f1);
    fscanf(f1, " %s \n", chk);
    i++;
  }

  fclose(f1);
}


void update_pos( struct char_data *victim )
{

  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
    return;
  } else if (GET_HIT(victim) > 0 ) {
    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
      if(!MOUNTED(victim))
        GET_POS(victim) = POSITION_STANDING;
      else
        GET_POS(victim) = POSITION_MOUNTED;
    } else {
      GET_POS(victim) = POSITION_STUNNED;
    }
  } else if (GET_HIT(victim) <= -11) {
    GET_POS(victim) = POSITION_DEAD;
  } else if (GET_HIT(victim) <= -6) {
    GET_POS(victim) = POSITION_MORTALLYW;
  } else if (GET_HIT(victim) <= -3) {
    GET_POS(victim) = POSITION_INCAP;
  } else {
    GET_POS(victim) = POSITION_STUNNED;
  }
}


int check_peaceful(struct char_data *ch, const char *msg)
{
    struct room_data *rp;

    if (!PeacefulFlag) return 0;
    rp = real_roomp(ch->in_room);
    if (rp && rp->room_flags&PEACEFUL) {
	send_to_char(msg, ch);
	return 1;
    }
    return 0;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  struct char_data *mast;

    if(!ch || !vict)
	return;

    if (affected_by_spell(ch, SKILL_CHAMELEON)) {
      affect_from_char(ch, SKILL_CHAMELEON);
      send_to_char("You reveal yourself...\n\r", ch);
      act("$n reveals $mself...",TRUE,ch,0,0,TO_ROOM);
    }

    if (affected_by_spell(vict, SKILL_CHAMELEON)) {
      affect_from_char(vict, SKILL_CHAMELEON);
      send_to_char("You reveal yourself...\n\r", vict);
      act("$n reveals $mself...",TRUE,vict,0,0,TO_ROOM);
    }

    if (affected_by_spell(ch, SPELL_CAMOUFLAGE)) {
      affect_from_char(ch, SPELL_CAMOUFLAGE);
      send_to_char("You reveal yourself...\n\r", ch);
      act("$n reveals $mself...",TRUE,ch,0,0,TO_ROOM);
    }

    if (affected_by_spell(vict, SPELL_CAMOUFLAGE)) {
      affect_from_char(vict, SPELL_CAMOUFLAGE);
      send_to_char("You reveal yourself...\n\r", vict);
      act("$n reveals $mself...",TRUE,vict,0,0,TO_ROOM);
    }

    if (IS_AFFECTED(ch, AFF_HIDE)) {
      REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
      send_to_char("You come out of hiding...\n\r", ch);
      act("$n comes out of hiding...",TRUE,ch,0,0,TO_ROOM);
    }

    if (IS_AFFECTED(vict, AFF_HIDE)) {
      REMOVE_BIT(AFF_FLAGS(vict), AFF_HIDE);
      send_to_char("You come out of hiding...\n\r", vict);
      act("$n comes out of hiding...",TRUE,vict,0,0,TO_ROOM);
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
      REMOVE_BIT(AFF_FLAGS(ch), AFF_SNEAK);
      send_to_char("You stop sneaking...\n\r", ch);
      act("$n stops sneaking...",TRUE,ch,0,0,TO_ROOM);
    }

    if (IS_AFFECTED(vict, AFF_SNEAK)) {
      REMOVE_BIT(AFF_FLAGS(vict), AFF_SNEAK);
      send_to_char("You stop sneaking...\n\r", vict);
      act("$n stops sneaking...",TRUE,vict,0,0,TO_ROOM);
    }

    if (ch->specials.fighting) {
        char buf[256];
	sprintf(buf, "Fighting %s set to fighting another.",
		GET_REAL_NAME(ch));
	log_msg(buf, LOG_MPROG);
	return;
    }

    if (vict->attackers <= 126 ) {
	vict->attackers+=1;
    } else {
	log_msg("more than 126 people attacking one target", LOG_MPROG);
    }
    combatants++;
    ch->next_fighting = combat_list;
    combat_list = ch;

    if(IS_AFFECTED(ch,AFF_SLEEP))
	affect_from_char(ch,SPELL_SLEEP);

    ch->specials.fighting = vict;
    GET_POS(ch) = POSITION_FIGHTING;

    if( (IS_PC(ch) ||
	 (IS_NPC(ch) && ( (mast=ch->master)!= NULL) && (IS_PC(mast)))) &&
       !IS_SET(vict->specials.mob_act, ACT_PATTACK))
    {
	SET_BIT(ch->specials.mob_act, ACT_PATTACK);
    }
}


/* stop_opponents - stop anybody that's fighting this character */
void stop_opponents(struct char_data* ch, int was_room)
{
  struct room_data* rp;
  struct char_data* pers;

  if(!(rp = real_roomp(was_room)))
    return;

  for(pers = rp->people ; pers ; pers = pers->next_in_room)
  {
    if(pers->specials.fighting == ch)
      stop_fighting(pers);
  }
}

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
    struct char_data **tmp;

    REMOVE_BIT(ch->specials.mob_act, ACT_PATTACK);

    if(IS_AFFECTED(ch, AFF_BERSERK))
    {
	REMOVE_BIT(AFF_FLAGS(ch), AFF_BERSERK);
	if(GET_POS(ch) > POSITION_INCAP)
	{
	  send_to_char("You come to your senses.\n\r", ch);
	  if(ch->in_room > 0)
	    act("$n comes to $s senses.\n\r", FALSE, ch, 0, 0, TO_ROOM);
	}
    }

    if (ch->specials.fighting)
    {
	ch->specials.fighting->attackers-=1;
	if (ch->specials.fighting->attackers < 0) {
	    log_msg("too few people attacking", LOG_MPROG);
	    ch->specials.fighting->attackers = 0;
	}
	ch->specials.fighting = 0;
    }

    if (ch == combat_next_dude)
	combat_next_dude = ch->next_fighting;

    for(tmp = &combat_list ; *tmp ; tmp = &(*tmp)->next_fighting)
    {
	if(*tmp == ch)
	{
	    *tmp = ch->next_fighting;
	    break;
	}
    }
    ch->next_fighting = 0;

    if (ch->specials.binding)
    {
	ch->specials.binding->specials.binded_by=0;
	ch->specials.binding=0;
    }

    if (ch->specials.binded_by)
    {
	ch->specials.binded_by->specials.binding=0;
	ch->specials.binded_by=0;
    }

    if(MOUNTED(ch))
	GET_POS(ch) = POSITION_MOUNTED;
    else
	GET_POS(ch) = POSITION_STANDING;

    update_pos(ch);
}

void RemNonRent(struct char_data *ch) {
   struct obj_data *obj, *dobj[30];
   int ndel=0, a;

   for(obj=ch->carrying;obj;obj=obj->next_content)
     if(obj->obj_flags.cost_per_day < 0)
	dobj[ndel++] = obj;

   for(a=0;a<ndel;a++) {
      obj_from_char(dobj[a]);
      obj_to_room(dobj[a], ch->in_room);
   }

   for(a=0;a<MAX_WEAR;a++)
     if(ch->equipment[a])
       if(ch->equipment[a]->obj_flags.cost_per_day < 0) {
	  obj=unequip_char(ch, a);
	  obj_to_room(obj, ch->in_room);
       }
}

#define NUM_OF_FUN_PARTS 21
struct fun_body_piece {
   int number;          /* this parts number */
   char name[40];       /* names of this part */
   int  nname;          /* some parts you couldn't trace to an owner*/
   char sdesc[128];     /* short desc: that of inventory  */
   char rdesc[128];     /* room desc: when on ground */
   int  take;           /* some body parts don't transfer well */
   char actout[128];    /* what people in room see upon death, using act()*/
   char actkil[128];    /* what the killer sees upon dismemberment, using act() */
};

struct fun_body_piece parts[NUM_OF_FUN_PARTS] = {
{0,"eyeball eye",1,"the eye of %s","$CmThe eyeball of %s is lying here.$CN",
 1,"$Cm$n's attack knocks an eye out of $N!$CN",
   "$CmYour attack knocks an eye out of $N!$CN"},
{1,"liver",1,"the liver of %s","$Cm%s's liver is lying here.$CN",
 1,"$Cm$n's cruel attack blows $N's liver out!$CN",
   "$CmYour cruel attack blows $N's liver out!$CN"},
{2,"arm",1,"one of %s's arms","$Cm%s'arm is lying here on the ground.$CN",
 1,"$Cm$n's removes $N's arm!$CN",
   "$CmYou remove $N's arm!$CN"},
{3,"bowels",1,"%s's bowels","$CmIck. %s's bowels are lying here.$CN",
 1,"$Cm$n disembowels $N!$CN",
   "$CmYou disembowel $N!$CN"},
{4,"tush butt rear ass",1,"%s's rear end",
   "$CmSome one cut of %s's butt and left it here.$CN",
 1,"$Cm$n laughs as $s attack severs $N's rear end!$CN",
   "$CmYou laugh as you sever $N's rear end!$CN"},
{5,"right leg",1,"%s's right leg","$Cm%s's right leg is here.$CN",
 1,"$Cm$N gracefully cuts his leg off! $n chortles merrily!$CN",
   "$CmYou watch in awe as $N cuts his leg off!$CN"},
{6,"left leg",1,"the left leg of %s","$CmThe left leg of %s is lying here.$CN",
 1,"$Cm$n's screams and strikes $N leg off at the hip!$CN",
   "$CmWith a scream of rage, you strike $N's leg off!$CN"},
{7,"head",1,"%s's ugly head","$Cm%s's head is lying here, staring at you.$CN",
 1,"$Cm$n severs $N's in a move composed of speed and grace!$CN",
   "$CmWith speed and grace, you sever $N's head!$CN"},
{8,"thumb",1,"%s's thumb","$CmOne of %s's thumbs is lying here.$CN",
 1,"$Cm$n's attack severs a thumb from $N!$CN",
   "$CmYour attack severs a thumb from $N!$CN"},
{9,"finger",1,"%s's finger","$CmOne of %s fingers is lying here.$CN",
 1,"$Cm$n's attack severs a finger from $N!$CN",
   "$CmYour attack severs a finger from $N!$CN"},
{10,"stomach",1,"%s's stomach","$Cm%s lost his stomach here.$CN",
 1,"$CmWith animal force, $n tears $N's stomach out!$CN",
   "$CmWith animal force, you tear $N's stomach out!$CN"},
{11,"heart",1,"the once beating heart of %s",
"$Cm%s's once beating heart lies here.$CN",
 1,"$Cm$n's uses pure strength to eviscirate $N!$CN",
   "$CmYou depend upon your fierce strength, and eviscerate $N!$CN"},
{12,"spine",1,"the spine of %s","$CmThe spine of %s is lying here.$CN",
 1,"$Cm$n's attack shatters $N's spine!$CN",
   "$CmYour attack shatters $N's spine!$CN"},
{13,"intestine",0,"An icky pile of intestines",
"$CmAn icky pile of intestines is here - colon and all.$CN",
 0,"$Cm$n hits so hard, that $N pukes up his intestines!$CN",
   "$CmYou hit $N so hard that he pukes up his intestines!$CN"},
{14,"puke vomit",0,"chunky vomit","$CySome one upchucked on the floor here.$CN",
 0,"$Cy$N throws up all over $n!$CN",
   "$Cy$N throws up all over you!$CN"},
{15,"pool blood",0,"A pool of blood","$CrBlood has formed a pool on the ground.$CN",
 0,"$Cr$N bleeds horrendously!$CN",
   "$Cr$N bleeds horrendously!$CN"},
{16,"riblet",1,"a meaty riblet from %s","$CmA meaty riblet from %s is lying here.$CN",
 1,"$Cm$n's explodes $N's chest with a barrage of attacks!$CN",
   "$CmYou cause $N's chest to explode from a barrage of attacks!$CN"},
{17,"nose",1,"%s's nose","$Cm%s lost his nose here.$CN",
 1,"$Cm$n cackles gleefuly as $s attack removes $N's nose!$CN",
   "$CmYou cackle as you sever $N's nose!$CN"},
{18,"ear",1,"%s's ear","$Cm%'s bloody severed ear is here.$CN",
 1,"$Cm$n's grabs $N's ear and rips it off!$CN",
   "$CmYou rip off $N's ear!$CN"},
{19,"brain",1,"the jiggly brain of %s","$CmThe squishy brain of %s is here.$CN",
 1,"$Cm$n shatters $N's skull, knocking the brain on the ground!$CN",
   "$CmYou shatter $N's skull, knocking the brain on the ground!$CN"},
{20,"lung",1,"a bloody lung from %s","$CmA blood soaked lung from %s lies here.$CN",
 1,"$Cm$N screams $s last as $n removes a lung!$CN",
   "$Cm$N's screams are cut short as you remove a lung!$CN"}
};


void make_fun_body_pieces(struct char_data *ch, struct char_data *killer)
{
  struct obj_data *piece;
  object_event* event;
  char buffer[MAX_STRING_LENGTH];
  char buffer2[MAX_STRING_LENGTH];
  int i;

  /*lets check and see if we even GET body parts eh - i mean, they're
    fun, but it wouldn't be quite as fun if they were always there!*/

  if (number(1,20) < 19)
    return;

              /*Then Horray! We's got parts! */
                   /* But which part? */
  i=number(0,20);   /* 20 pieces should be okay*/
  piece = create_object();

  /*now, everything we have should be in the structures neh?*/
  /*name first*/
    ss_free(piece->name);
    piece->name=ss_make(parts[i].name);
  /*then lets see about the descs */
  if (parts[i].nname) {
    sprintf(buffer, parts[i].sdesc, GET_NAME(ch));
    ss_free(piece->short_description);
    piece->short_description=ss_make(buffer);

    sprintf(buffer2, parts[i].rdesc, GET_NAME(ch));
    ss_free(piece->description);
    piece->description=ss_make(buffer2);

  }
  else {
    ss_free(piece->short_description);
    piece->short_description=ss_make(parts[i].sdesc);
    ss_free(piece->description);
    piece->description=ss_make(parts[i].rdesc);
  }
  /*well, now we know how it looks, lets see if we wanna take it.*/
  if (parts[i].take) {
  GET_OBJ_WEAR(piece) = ITEM_TAKE;
  }
 /*  and lets see how it got here in the first place neh? */
  act(parts[i].actout,FALSE,killer,0,ch,TO_ROOM);
  act(parts[i].actkil,FALSE,killer,0,ch,TO_CHAR);

/* setup the rest of the stats any object needs */
  piece->item_number = NOTHING;
  piece->in_room = NOWHERE;
  GET_OBJ_TYPE(piece) = ITEM_TRASH;
  GET_OBJ_VAL(piece, 0) = 0;            /* You can't store stuff in a corpse */
  GET_OBJ_VAL(piece, 2) = NPC_CORPSE;   /* corpse identifier */
  (piece)->obj_flags.weight = 10;
  GET_OBJ_RENT(piece) = -1;
/* Note - you may have some trouble with corpse decay here -
   improper settings WILL cause the mud to crash if you do not correcly
   decay.  Right now, the pieces are setup as a corpse, so if you made any
   changes to your corpse identifiers, fix it above.
*/

  GET_OBJ_TIMER(piece) = MAX_NPC_CORPSE_TIME;

  CREATE(event, object_event, 1);

  event->object = piece;

  sprintf(buffer, "body part %s", OBJ_NAME(piece));
  piece->corpse_timer = event_queue_pulse((event_t*) event,
                                          pulse,
                                          (event_func) corpse_timer,
	       		                  buffer);

  /* and thats all folks! */
  obj_to_room(piece, ch->in_room);
}


void stop_charging(struct char_data *ch)
{
  struct char_data **tmp;

  if (ch == charging_next_dude)
    charging_next_dude = ch->next_charging;

  for(tmp = &charging_list ; *tmp ; tmp = &(*tmp)->next_charging)
  {
    if(*tmp == ch)
    {
      *tmp = ch->next_charging;
      break;
    }
  }
  ch->next_charging = 0;
}

int corpse_timer(object_event* event, long now)
{
    struct obj_data* j = event->object;

    if(--j->obj_flags.timer <= 0)
    {
	if (j->carried_by)
	    act("$p dissolves.",
		FALSE, j->carried_by, j, 0, TO_CHAR);
	if (j->equipped_by)
	    act("$p dissolves.",
		FALSE, j->equipped_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) &&
		 (real_roomp(j->in_room)->people))
	{
	    act("$p dissolves.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	    act("$p dissolves.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}
	j->corpse_timer = 0;
	event_free((event_t*) event);
	ObjFromCorpse(j);
	return 1;
    }
    else
    {
      event_queue_pulse((event_t*) event,
			  pulse,
			  (event_func) corpse_timer,
			  NULL);

	return 1;
    }
}


int pc_corpse_timer(object_event* event, long now)
{
    struct obj_data* j = event->object;

    if(--j->obj_flags.timer <= 0)
    {
	if (j->carried_by)
	    act("$p dissolves.",
		FALSE, j->carried_by, j, 0, TO_CHAR);
	if (j->equipped_by)
	    act("$p dissolves.",
		FALSE, j->equipped_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) &&
		 (real_roomp(j->in_room)->people))
	{
	    act("$p dissolves.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	    act("$p dissolves.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}
	j->corpse_timer = 0;
	event_free((event_t*) event);
	if (j->in_room == RM_MORGUE_HOLDING)
	  extract_obj(j);
	else
	  ObjFromCorpse(j);
	return 1;
    }
    else
    {
      event_queue_pulse((event_t*) event,
			  pulse,
			  (event_func) pc_corpse_timer,
			  NULL);

	return 1;
    }

}

void make_corpse(struct char_data *ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  object_event* event;
  char buf[MAX_INPUT_LENGTH*2];
  int i, ADeadBody=FALSE;

  corpse = make_object(21, VIRTUAL|NORAND);

  corpse->in_room = NOWHERE;

  if (!IS_NPC(ch) || ((!IsUndead(ch)) && (!IsUnsamplable(ch)))) {
    sprintf(buf, "corpse %s",GET_REAL_NAME(ch));
    corpse->char_name = ss_make(GET_REAL_NAME(ch));
    corpse->name = ss_make(buf);

    sprintf(buf, "The corpse of %s is lying here.", GET_NAME(ch));
    corpse->description = ss_make(buf);

    sprintf(buf, "the corpse of %s", GET_NAME(ch));
    corpse->short_description = ss_make(buf);

    ADeadBody = TRUE;

    corpse->char_name = ss_share(ch->player.name);
  } else if (IsUndead(ch) || (GET_RACE(ch) == RACE_HEAVEN)) {
    corpse->name = ss_make("dust pile");
    corpse->description = ss_make("A pile of dust is here.");
    corpse->short_description = ss_make("a pile of dust");
  } else if (GET_RACE(ch) == RACE_SHERRINPIP) {
    sprintf(buf, "carcass %s",GET_REAL_NAME(ch));
    corpse->char_name = ss_make(GET_REAL_NAME(ch));
    corpse->name = ss_make(buf);

    sprintf(buf, "The petrified carcass of %s is lying here.", GET_NAME(ch));
    corpse->description = ss_make(buf);

    sprintf(buf, "the carcass of %s", GET_NAME(ch));
    corpse->short_description = ss_make(buf);

    ADeadBody = TRUE;

    corpse->char_name = ss_share(ch->player.name);
  }
  else if ((GET_RACE(ch) == RACE_GOLEM) || (GET_RACE(ch) == RACE_ELEMENTAL))
  {
    sprintf(buf, "remains %s",GET_REAL_NAME(ch));
    corpse->char_name = ss_make(GET_REAL_NAME(ch));
    corpse->name = ss_make(buf);

    sprintf(buf, "The material remains of %s is lying here.", GET_NAME(ch));
    corpse->description = ss_make(buf);

    sprintf(buf, "the remains of %s", GET_NAME(ch));
    corpse->short_description = ss_make(buf);

    ADeadBody = TRUE;

    corpse->char_name = ss_share(ch->player.name);
  }

#ifdef MORGUE_ON
  if (!IS_PC(ch) || GetMaxLevel(ch) > 125)
  {
#endif
    if(GET_GOLD(ch)>0) {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch)=0;
      obj_to_obj(money,corpse);
    }
#ifdef MORGUE_ON
  }
  else
  {
    (GET_BANK(ch))+=(GET_GOLD(ch));
    (GET_GOLD(ch))=0;
  }
#endif

  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.extra_flags = ITEM_ORGANIC;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  corpse->obj_flags.weight = ADeadBody ? GET_WEIGHT(ch) : 1;
  corpse->obj_flags.cost_per_day = -1;
  corpse->obj_flags.sample_level = GetMaxLevel(ch);
  if (IS_NPC(ch)) {
    corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
    corpse->obj_flags.value[2] = NPC_CORPSE;
  }
#ifdef MORGUE_ON
  else
  {
    corpse->obj_flags.value[3]=ch->rent_cost/10 + 1;
    corpse->obj_flags.value[2]=PC_CORPSE;

    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
  }
#else
  else
  {
    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
    corpse->obj_flags.value[2] = PC_CORPSE;
  }
#endif


  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), corpse);

  while((o = ch->carrying))
  {
    obj_from_char(o);
    obj_to_obj(o, corpse);
  }

  if (IS_NPC(ch)) {
    corpse->char_vnum = mob_index[ch->nr].virt;
  } else {
    corpse->char_vnum = 0;
  }
  corpse->carried_by = 0;
  corpse->equipped_by = 0;

  array_insert(&object_list, corpse);

  if(IS_PC(ch))
    savecorpse(corpse, ch->in_room);
/* Temp change by Novak during saving spell testing */
  if(IS_PC(ch) && (GetMaxLevel(ch) <= 90 ||
     ch->player.pkillinfo.status))
  {
#ifdef MORGUE_ON
    if(real_roomp(RM_MORGUE_HOLDING))
      i=RM_MORGUE_HOLDING;
    else
#endif
      if(real_roomp(3001))
      i=3001;
    else if(real_roomp(0))
      i=0;
    else
    {
      log_msg("could not place a corpse in any room -- using old method");
      i=ch->in_room;
    }
    obj_to_room(corpse,i);
    sprintf(buf,"In a puff of smoke, a demon arrives with %s and says, \"Here, we only need the souls.\"\n\r"
	    "After depositing it on the ground,  the demon leaves in a flash of light.\n\r"
	    "You smell the feint odor of brimstone.\n\r",ss_data(corpse->short_description));
    send_to_room(buf,i);
  }
  else
    obj_to_room(corpse, ch->in_room);

#ifdef MORGUE_ON
  if(!IS_PC(ch))
  {
#endif
    CREATE(event, object_event, 1);
    event->object = corpse;

    sprintf(buf, "corpse %s", OBJ_NAME(corpse));
    corpse->corpse_timer = event_queue_pulse((event_t*) event,
					     pulse,
					     (event_func) corpse_timer,
					     buf);
#ifdef MORGUE_ON
  }
  else {
    CREATE(event, object_event, 1);
    event->object = corpse;

    sprintf(buf, "pc corpse %s", OBJ_NAME(corpse));
    corpse->corpse_timer = event_queue_pulse((event_t*) event,
					     pulse,
					     (event_func) pc_corpse_timer,
					     buf);
 }
#endif
}


#define ALIGN_MODIFIER 5

/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int align;

    if(!IS_PC(ch)) return;

    if(IS_EVIL(ch))
    {
	if(IS_GOOD(victim))
	    align = GET_ALIGNMENT(victim) / (200 * ALIGN_MODIFIER);
	else if(IS_EVIL(victim))
	    align = GET_ALIGNMENT(victim) / (50 * ALIGN_MODIFIER);
	else
	    align = GET_ALIGNMENT(victim) / (100 * ALIGN_MODIFIER);
    }
    else if(IS_GOOD(ch))
    {
	if(IS_EVIL(victim))
	    align = GET_ALIGNMENT(victim) / (200 * ALIGN_MODIFIER);
	else if(IS_GOOD(victim))
	    align = GET_ALIGNMENT(victim) / (50 * ALIGN_MODIFIER);
	else
	    align = GET_ALIGNMENT(victim) / (100 * ALIGN_MODIFIER);
    }
    else
	align = GET_ALIGNMENT(victim) / (100 * ALIGN_MODIFIER);

    if (HasClass(ch, CLASS_PALADIN))
      if(align < 0)
         align *= 3;

    GET_ALIGNMENT(ch) -= align;
    if(GET_ALIGNMENT(ch) < -1000)
	GET_ALIGNMENT(ch) = -1000;
    else if(GET_ALIGNMENT(ch) > 1000)
	GET_ALIGNMENT(ch) = 1000;
}


void death_cry(struct char_data *ch)
{
  int door, was_in;
  char brett1[MAX_STRING_LENGTH];
  char brett2[MAX_STRING_LENGTH];
  struct obj_data *o;
  object_event *event;

  if (ch->in_room == NOWHERE)
    return;
  switch (number(0, 13)) {
    case 0:
     act("$Cg$n falls to the ground, splattering blood on your armor.$CN", FALSE, ch,0,0,TO_ROOM);
     break;
    case 1:
     act("$CgYou freeze as $n wails a death cry.$CN", FALSE, ch,0,0,TO_ROOM);
     break;
    case 2:
     act("$CgYour blood becomes cold as $n dies.$CN", FALSE, ch,0,0,TO_ROOM);
     break;
    case 3:
     act("$CgYour heart races as you see $n fall to the ground... dead.$CN", FALSE, ch,0,0,TO_ROOM);
     break;
    case 4:
     act("$CgBlood spurts out of $n's chest as they hit the ground... $CRdead.$CN", FALSE, ch,0,0,TO_ROOM);
     break;
    case 5:
     act("$Cg$n's head is severed and falls to the ground with a clunk.  $n is dead.$CN",FALSE,ch,0,0,TO_ROOM);
     /* make a head! */
     if(!(o=make_object(15,VIRTUAL|NORAND)))
	 break;
     sprintf(brett1,"the bloody head of %s",GET_NAME(ch));
     ss_free(o->short_description);
     o->short_description = ss_make(brett1);
     ss_free(o->action_description);
     o->action_description = ss_make(brett1);
     sprintf(brett2,"%s is lying on the ground.",brett1);
     ss_free(o->description);
     o->description = ss_make(brett2);
     o->obj_flags.value[0]=0;
     o->obj_flags.value[3]=HEAD_ITEM; /* triggers it to decompose-be a corpse*/
     o->obj_flags.cost_per_day = -1;
     o->obj_flags.timer=MAX_HEAD_TIME;
     obj_to_room(o,ch->in_room);

     CREATE(event, object_event, 1);
     event->object = o;

     sprintf(brett1, "head %s", OBJ_NAME(o));
     o->corpse_timer = event_queue_pulse((event_t*) event,
					   pulse,
					   (event_func) corpse_timer,
					   brett1);
     break;
    case 6:
     act("$CgDeath carries $n to the afterlife!$CN",FALSE,ch,0,0,TO_ROOM);
     break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
     act("$CgYour blood freezes as you hear $n's death cry.$CN",FALSE,ch,0,0,TO_ROOM);
     break;
  }
  was_in = ch->in_room;

  for (door = 0; door <= 5; door++) {
    if (CAN_GO(ch, door))	{
      ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
      switch (number(0, 13)) {
        case 0:
         act("$CwBlood splatters into the room.$CN", FALSE, ch,0,0,TO_ROOM);
         break;
        case 1:
         act("$CwDeath cackles with glee... nearby!!$CN", FALSE, ch,0,0,TO_ROOM);
         break;
        case 2:
         act("$CwWhat was that...a death scream?$CN", FALSE, ch,0,0,TO_ROOM);
         break;
        case 3:
         act("$CwDeath claims another victim...on TW$CN", FALSE, ch,0,0,TO_ROOM);
         break;
        case 4:
         act("$CwNearby...a body falls to the ground!$CN", FALSE, ch,0,0,TO_ROOM);
         break;
        case 5:
         act("$CwYou hear a loud thud, nearby!$CN",FALSE,ch,0,0,TO_ROOM);
         break;
        case 6:
         act("$CwYou hear a fierce struggle, nearby.$CN",FALSE,ch,0,0,TO_ROOM);
         break;
        case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
         act("$CwYour blood freezes as someone dies!$CN",FALSE,ch,0,0,TO_ROOM);
         break;
      }
      ch->in_room = was_in;
    }
  }
}

void raw_kill(struct char_data *ch)
{
    struct obj_cost cost; /* This is thrown away, used in OfferChar() call
			     which sets the ch->rent_cost value used to
			     set cost of corpse in make_corpse() */

    if (ch->specials.fighting)
	stop_fighting(ch);
    if (ch->specials.charging)
      stop_charging(ch);

    death_cry(ch);

    /*
      remove the problem with poison, and other spells
      */
    RemAllAffects(ch);

    if(CHAOS==1)
    {
	GET_HIT(ch) = GET_MAX_HIT(ch);
	GET_MANA(ch) = GET_MAX_MANA(ch);
	GET_MOVE(ch) = GET_MAX_MOVE(ch);
    }
    else
	GET_HIT(ch) = 1;
    update_pos(ch);

    /* fully restore the nerds mainly for chaos night purposes */

    /*
      give them some food and water so they don't whine.
      */
    if (GET_COND(ch,THIRST) != -1)
	GET_COND(ch,THIRST) = 24;
    if (GET_COND(ch,FULL) != -1)
	GET_COND(ch,FULL) = 24;

    /*
     *   return them from polymorph
     */

#if NEWPKILL
    remove_witness(GET_REAL_NAME(ch));
#endif

#ifdef MORGUE_ON
    if (IS_PC(ch))
      OfferChar(ch,&cost,RM_MORGUE_HOLDING,FALSE);
#endif

    /* maarek */
    make_corpse(ch);

    /* PAC -- this is redundant really <StuffToRoom>,
     * since its already handled in make_corpse
     */
    StuffToRoom(ch, ch->in_room);

    // Remove all spell events before saving char and dying -- raist
    spevent_remove_all_char(ch); 
    SaveChar(ch, AUTO_RENT, TRUE);

    extract_char(ch);
}

void arena_die(struct char_data *ch)
{
    if (ch->specials.fighting)
	stop_fighting(ch);
    if (ch->specials.charging)
      stop_charging(ch);

    death_cry(ch);

    if(affected_by_spell(ch, SPELL_BLINDNESS))
      affect_from_char(ch, SPELL_BLINDNESS);
    if(affected_by_spell(ch, SPELL_PARALYSIS))
      affect_from_char(ch, SPELL_PARALYSIS);
    if(affected_by_spell(ch, SPELL_POISON))
      affect_from_char(ch, SPELL_POISON);

    GET_HIT(ch) = 1;
    update_pos(ch);

    if(zprog_arena_trigger(ch)) {
       update_pos(ch);
       SaveChar(ch, AUTO_RENT, TRUE);
       return;
    }

   GET_HIT(ch) = 1;
   RemAllAffects(ch);
   char_from_room(ch);
   char_to_room(ch, 3001);
   send_to_char("\n\r"
		"Your soul drifts away from your body, letting you watch the last breath\n\r"
		"before you die.\n\r\n\r"
		"A man in black appears before you and flips though a large stack of papers.\n\r"
		"Death says, 'Hm... you don't appear on any of my lists.  I'm afraid you will\n\r"
		"just have to make another appointment.'\r\n\r\n", ch);

   update_pos(ch);

   do_look(ch, "", 15);

   SaveChar(ch, AUTO_RENT, TRUE);
}


void die(struct char_data *ch)
{
    struct char_data *pers, *rch=real_character(ch);
    int i, clss, lev;
    int pop_poly=1;
    int dresult;

    RemNonRent(ch);

    pop_poly = !((ch->in_room >= 33300) && (ch->in_room <= 33399) && IS_SET(RM_FLAGS(ch->in_room), ARENA));

    if(pop_poly && (pers = pop_character(ch)))
    {
	if (IS_SET(ch->specials.mob_act, ACT_POLYSELF))
	{
	    char_from_room(pers);
	    char_to_room(pers, ch->in_room);
	    SwitchStuff(ch, pers);
	    extract_char(ch);
	    ch = pers;
	}
    }

    dresult = zprog_death_trigger(ch);
    if ((IS_SET(RM_FLAGS(ch->in_room), ARENA) && IS_PC(rch)) ||
	(rch->player.pkillinfo.status && IS_PC(rch) && PKILLABLE==2) ||
	(dresult == 1))
    {
	arena_die(ch);
	return;
    }


    for(i = 0, clss = -1, lev = 0 ; i <= MAX_LEVEL_IND ; ++i)
    {
	if(GET_LEVEL(ch, i) > lev)
	{
	    clss = i;
	    lev = GET_LEVEL(ch, i);
	}
    }

    if(IS_PC(ch) && (lev > 1) && (lev <= MAX_MORT))
    {
	/* store away info for a res */
	ch->res_info.valid = 1;
	ch->res_info.clss = clss;
	ch->res_info.level = lev;
	ch->res_info.exp = GET_EXP(ch);

	/* lose a level */
 	GET_LEVEL(ch, clss) = lev - 0;  /* Disabling for now... no loss of level at death */

	/* if we could gain before, set exp to 1 shy of gaining, otherwise 0 */
	if(GET_EXP(ch) > exp_table[lev])
	    GET_EXP(ch) = exp_table[lev - 0] - 1;  /* Disabling level loss at death, otherwise GET_EXP(ch) = exp_table[lev - 1] - 1; */
	else
	    GET_EXP(ch) = 0;

	/* reset title */
	set_title(ch);
	add_char_to_hero_list(ch);
	UpdateMaxLevel(ch);
	UpdateMinLevel(ch);
    }

    DeleteHatreds(ch);
    DeleteFears(ch);
    raw_kill(ch);

}


void group_gain(struct char_data *ch, struct char_data *victim) {
   int max_lvl, tot_lvl, share, num;
   struct char_data *k;
   struct follow_type *f;
   long total, min_level;

   min_level = 50;   //before cutoff starts to effect.

   if (!(k=ch->master)) /* start exp distribution from master */
     k = ch;

   if (!IS_NPC(victim)) /* no experience for killing players */
      return;

   if (k->followers==NULL || !IS_AFFECTED(k, AFF_GROUP))  { /* this is leader, so give exp */
      gain_exp(k,GET_EXP(victim)); /* makes it quick and also doesn't */
      change_alignment(k,victim); /* force player to group self */
      return;
   }

   if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room) && IS_PC(k)) {
     tot_lvl = max_lvl = MAX(GetAvgLevel(k), min_level); /* get avg level of master */
     num=1;
   } else {
     num = tot_lvl = max_lvl = 0; /* if master not in room or not grouped */
   }

   for (f=k->followers; f; f=f->next) {
     if (IS_AFFECTED(f->follower, AFF_GROUP) && /* if follower is grouped */
	 (f->follower->in_room == ch->in_room)) {/* and in same room */
       if (IS_PC(f->follower)) {
  	 /* add to avg sum of all foll */
         max_lvl = MAX(max_lvl, MAX(GetAvgLevel(f->follower), min_level));
	 tot_lvl += MAX(GetAvgLevel(f->follower), min_level);
  	 num++;
       }
     }
   }

   if (num >= 1)
     share = (GET_EXP(victim)*(num*num-num+4))/3; /*Originally /4 */
   else
     share = 0;

   if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
      total = (int)((double)share * (double)tot_lvl / (double)(num * num * max_lvl));
      total = (int)((double)total * pow((double)MAX(GetAvgLevel(k), min_level)/(double)max_lvl,2));
      gain_exp(k, total); /* exp gain for master */
      change_alignment(k, victim);
   }

   for (f=k->followers; f; f=f->next)  {
     if (IS_AFFECTED(f->follower, AFF_GROUP)
	 &&	(f->follower->in_room == ch->in_room)) {
       if (IS_PC(f->follower))  { /* only give exp to players */
         total = (int)((double)share * (double)tot_lvl / (double)(num * num * max_lvl));
	 total = (int)((double)total * pow((double)MAX(GetAvgLevel(f->follower), min_level)/(double)max_lvl,2));
	 gain_exp(f->follower, total);
	 change_alignment(f->follower, victim);
       }
     }
   }
 }




char *replace_string(char *str, char *weapon, char *weapon_s)
{
  static char buf[256];
  char *cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch(*(++str)) {
      case 'W' :
	for (; *weapon; *(cp++) = *(weapon++));
	break;
      case 'w' :
	for (; *weapon_s; *(cp++) = *(weapon_s++));
	break;
	default :
	  *(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }

    *cp = 0;
  } /* For */

  return(buf);
}



void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
    struct obj_data *wield;
    char *buf;

    static struct dam_weapon_type {
	int max_dam;		/* max damage for which to use this message */
	char *to_room;
	char *to_char;
	char *to_victim;
    } dam_weapons[] = {
        {
            2,
            "$n $CMtickles$CN $N with $s #w.",
            "You $CMtickle$CN $N as you #w $M.",
            "$n $CMtickles$CN you as $e #W you." },

        {
            5,
            "$n $CMnicks$CN $N.",
            "You $CMnick$CN $N.",
            "$n $CMnicks$CN you."       },

        {
            7,
            "$n $CMbarely scratches$CN $N with $s #w.",
            "You $CMbarely scratch$CN $N as you #w $M.",
            "$n $CMbarely scratches$CN you as $e #W you." },

        {
            10,
            "$n $CW#W$CN $N.",
            "You $CW#w$CN $N.",
            "$n $CW#W$CN you."},

        {
            25,
            "$n $CW#W$CN $N hard.",
            "You $CW#w$CN $N hard.",
            "$n $CW#W$CN you hard."},

        {
            50,
            "$n $CW#W$CN $N very hard.",
            "You $CW#w$CN $N very hard.",
            "$n $CW#W$CN you very hard."},

        {
            75,
            "$n $CW#W$CN $N really hard.",
            "You $CW#w$CN $N really hard.",
            "$n $CW#W$CN you really hard."},


        {
            99,
            "$n $CW#W$CN $N extremely hard.",
            "You $CW#w$CN $N extremely hard.",
            "$n $CW#W$CN you extremely hard."},

        {
            150,
            "$n $Cg-massacres-$CN $N with $s powerful #w .",
            "You $Cg-massacre-$CN $N with your powerful #w.",
            "$n $Cg-massacres-$CN you with $s powerful #w."},

        {
            199,
            "$n $Cg-MASSACRES-$CN $N with $s powerful #w .",
            "You $Cg-MASSACRE-$CN $N with your powerful #w.",
            "$n $Cg-MASSACRES-$CN you with $s powerful #w."},

       {
            250,
            "$n $CG=pulverizes=$CN $N with $s astounding #w.",
            "You $CG=pulverize=$CN $N with your astounding #w.",
            "$n $CG=pulverizes=$CN you with $s astounding #w."},

       {
            299,
            "$n $CG=PULVERIZES=$CN $N with $s astounding #w.",
            "You $CG=PULVERIZE=$CN $N with your astounding #w.",
            "$n $CG=PULVERIZES=$CN you with $s astounding #w."},

        {
            350,
            "$n $Cy-=annihilates=-$CN $N with $s deadly #w.",
            "You $Cy-=annihilate=-$CN $N with your deadly #w.",
            "$n $Cy-=annihilates=-$CN you with $s deadly #w."},

        {
            399,
            "$n $Cy-=ANNIHILATES=-$CN $N with $s deadly #w.",
            "You $Cy-=ANNIHILATE=-$CN $N with your deadly #w.",
            "$n $Cy-=ANNIHILATES=-$CN you with $s deadly #w."},

        {
            450,
            "$n $CY==obliterates==$CN $N with $s unbelievable #w.",
            "You $CY==obliterate==$CN $N with your unbelievable #w.",
            "$n $CY==obliterates==$CN you with $s unbelievable #w."},

        {
            499,
            "$n $CY==OBLITERATES==$CN $N with $s unbelievable #w.",
            "You $CY==OBLITERATE==$CN $N with your unbelievable #w.",
            "$n $CY==OBLITERATES==$CN you with $s unbelievable #w."},

        {
            550,
            "$N $Cm*staggers*$CN under $n's amazing #w!",
            "You $Cm*stagger*$CN $N with your amazing #w!",
            "You $Cm*staggers*$CN from $n's amazing #w!"},

        {
            599,
            "$N $Cm*STAGGERS*$CN under $n's amazing #w!",
            "You $Cm*STAGGER*$CN $N with your amazing #w!",
            "You $Cm*STAGGERS*$CN from $n's amazing #w!"},
        {
            650,
            "$n $Cr-*brutally mauls*-$CN $N with $s incredible #w!",
            "You $Cr-*brutally maul*-$CN $N with your incredible #w!",
            "$n $Cr-*brutally mauls*-$CN you with $s incredible #w!"},

        {
            699,
            "$n $Cr-*BRUTALLY MAULS*-$CN $N with $s incredible #w!",
            "You $Cr-*BRUTALLY MAUL*-$CN $N with your incredible #w!",
            "$n $Cr-*BRUTALLY MAULS*-$CN you with $s incredible #w!"},

        {
            750,
            "$n $CR=*demolishes*=$CN $N with $s lethal #w!",
            "You $CR=*demolish*=$CN $N with your lethal #w!",
            "$n $CR=*demolishes*=$CN you with $s lethal #w!"},

        {
            799,
            "$n $CR=*DEMOLISHES*=$CN $N with $s lethal #w!",
            "You $CR=*DEMOLISH*=$CN $N with your lethal #w!",
            "$n $CR=*DEMOLISHES*=$CN you with $s lethal #w!"},

        {
            850,
            "$n $Cc-=*pulpifies*=-$CN $N with $s fatal #w!",
            "You $Cc-=*pulpify*=-$CN $N with your fatal #w!",
            "$n $Cc-=*pulpifies*=-$CN you with $s fatal #w!"},

        {
            899,
            "$n $Cc-=*PULPIFIES*=-$CN $N with $s fatal #w!",
            "You $Cc-=*PULPIFY*=-$CN $N with your fatal #w!",
            "$n $Cc-=*PULPIFIES*=-$CN you with $s fatal #w!"},

        {
            950,
            "$n $CC==*liquefies*==$CN $N with $s baneful #w!",
            "You $CC==*liquefy*==$CN $N with your baneful #w!",
            "$n $CC==*liquefies*==$CN you with $s baneful #w!"},

        {
            999,
            "$n $CC==*LIQUEFIES*==$CN $N with $s baneful #w!",
            "You $CC==*LIQUEFY*==$CN $N with your baneful #w!",
            "$n $CC==*LIQUEFIES*==$CN you with $s baneful #w!"},

        {
            1050,
            "$n $Cg**devestates**$CN $N with $s virulent #w!",
            "You $Cg**devestate**$CN $N with your virulent #w!",
            "$n $Cg**devestates**$CN you with $s virulent #w!"},

        {
            1099,
            "$n $Cg**DEVESTATES**$CN $N with $s virulent #w!",
            "You $Cg**DEVESTATE**$CN $N with your virulent #w!",
            "$n $Cg**DEVESTATES**$CN you with $s virulent #w!"},

        {
            1150,
            "$n $CG-**thrashs**-$CN $N with $s virulent #w!",
            "You $CG-**thrash**-$CN $N with your virulent #w!",
            "$n $CG-**thrashs**-$CN you with $s virulent #w!"},

        {
            1199,
            "$n $CG-**THRASHS**-$CN $N with $s virulent #w!",
            "You $CG-**THRASH**-$CN $N with your virulent #w!",
            "$n $CG-**THRASHS**-$CN you with $s virulent #w!"},

        {
            1250,
            "$n $Cy=**shatters**=$CN $N with $s virulent #w!",
            "You $Cy=**shatter**=$CN $N with your virulent #w!",
            "$n $Cy=**shatters**=$CN you with $s virulent #w!"},

        {
            1299,
            "$n $Cy=**SHATTERS**=$CN $N with $s virulent #w!",
            "You $Cy=**SHATTER**=$CN $N with your virulent #w!",
            "$n $Cy=**SHATTERS**=$CN you with $s virulent #w!"},

        {
            1350,
            "$n $CY-=**dismantles**=-$CN $N with $s virulent #w!",
            "You $CY-=**dismantle**=-$CN $N with your virulent #w!",
            "$n $CY-=**dismantles**=-$CN you with $s virulent #w!"},

        {
            1399,
            "$n $CY-=**DISMANTLES**=-$CN $N with $s virulent #w!",
            "You $CY-=**DISMANTLE**=-$CN $N with your virulent #w!",
            "$n $CY-=**DISMANTLES**=-$CN you with $s virulent #w!"},

	{
	    1450,
	    "$n $CB==**cripples**==$CN $N with $s virulent #w!",
            "You $CB==**cripple**==$CN $N with your virulent #w!",
	    "$n $CB==**cripples**==$CN you with $s virulent #w!"},

	{
	    1499,
	    "$n $CB==**CRIPPLES**==$CN $N with $s virulent #w!",
	    "You $CB==**CRIPPLE**==$CN $N with your virulent #w!",
	    "$n $CB==**CRIPPLES**==$CN you with $s virulent #w!"},

        {
            1550,
            "$n $Cm***levels***$CN $N with $s virulent #w!",
            "You $Cm***level***$CN $N with your virulent #w!",
            "$n $Cm***levels***$CN you with $s virulent #w!"},

        {
            1599,
            "$n $Cm***LEVELS***$CN $N with $s virulent #w!",
            "You $Cm***LEVEL***$CN $N with your virulent #w!",
            "$n $Cm***LEVELS***$CN you with $s virulent #w!"},

        {
            1650,
            "$n $Cr-***butchers***-$CN $N with $s virulent #w!",
            "You $Cr-***butcher***-$CN $N with your virulent #w!",
            "$n $Cr-***butchers***-$CN you with $s virulent #w!"},

        {
            1699,
            "$n $Cr-***BUTCHERS***-$CN $N with $s virulent #w!",
            "You $Cr-***BUTCHER***-$CN $N with your virulent #w!",
            "$n $Cr-***BUTCHERS***-$CN you with $s virulent #w!"},

        {
	    1750,
	    "$n $CR=***creams***=$CN $N with $s virulent #w!",
            "You $CR=***cream***=$CN $N with your virulent #w!",
            "$n $CR=***creams***=$CN you with $s virulent #w!"},

        {
            1799,
            "$n $CR=***CREAMS***=$CN $N with $s virulent #w!",
            "You $CR=***CREAM***=$CN $N with your virulent #w!",
            "$n $CR=***CREAMS***=$CN you with $s virulent #w!"},

	{
	    1850,
            "$n $Cc-=***cremates***=-$CN $N with $s virulent #w!",
  	    "You $Cc-=***cremate***=-$CN $N with your virulent #w!",
    	    "$n $Cc-=***cremates***=-$CN you with $s virulent #w!"},

        {
            1899,
            "$n $Cc-=***CREMATES***=-$CN $N with $s virulent #w!",
            "You $Cc-=***CREMATE***=-$CN $N with your virulent #w!",
            "$n $Cc-=***CREMATES***=-$CN you with $s virulent #w!"},

        {
	    1950,
	    "$n $CC==***mutilates***==$CN $N with $s virulent #w!",
            "You $CC==***mutilate***==$CN $N with your virulent #w!",
   	    "$n $CC==***mutilates***==$CN you with $s virulent #w!"},

        {
            1999,
            "$n $CC==***MUTILATES***==$CN $N with $s virulent #w!",
            "You $CC==***MUTILATE***==$CN $N with your virulent #w!",
            "$n $CC==***MUTILATES***==$CN you with $s virulent #w!"},

	{
	    2050,
	    "$n $CB****destroys****$CN $N with $s virulent #w!",
	    "You $CB****destroy****$CN $N with your virulent #w!",
	    "$n $CB****destroys****$CN you with $s virulent #w!"},

        {
            2099,
            "$n $CB****DESTROYS****$CN $N with $s virulent #w!",
            "You $CB****DESTROY****$CN $N with your virulent #w!",
            "$n $CB****DESTROYS****$CN you with $s virulent #w!"},

	{
	    2150,
	    "$n $Cb-****destroys****-$CN $N with $s virulent #w!",
	    "You $Cb-****destroy****-$CN $N with your virulent #w!",
	    "$n $Cb-****destroys****-$CN you with $s virulent #w!"},

        {
            2199,
            "$n $Cb-****DESTROYS****-$CN $N with $s virulent #w!",
            "You $Cb-****DESTROY****-$CN $N with your virulent #w!",
            "$n $Cb-****DESTROYS****-$CN you with $s virulent #w!"},

        {
            -1,
            "$n "
            "*$CB*$Cb*$Cg*$CG*$CyD$CYE$CmS$CMT$CRR$CrO$CcY$CCS$CB*$Cb*$Cg*$CG*$CN* $N "
            "with $s virulent #w!",
            "You $Cg*$CG*$CyD$CYE$CmS$CrT$CRR$CcO$CCY$CB*$Cb*$CN $N with "
            "your virulent #w!",
            "$n $Cg*$CG*$CyD$CYE$CmS$CMT$CRR$CrO$CcY$CCS$CB*$Cb*$CN you "
            "with $s virulent #w!"},
    };

    struct dam_weapon_type misses[] =
    {
	{
	    0,
	    "$n misses $N.",
	    "You miss $N.",
	    "$n misses you."	},

	{
	    0,
	    "$n swings in the direction of $N.",
	    "You swing at $N.",
	    "$n swings at you."	},

        {
            0,
            "$n aims for a head shot but $N ducks!",
            "You aim for $N's head but $E ducks!",
            "$n aims for your head but you duck out of the way!."   },

        {
            0,
            "$n tries to #w $N, but just misses.",
            "You try to #w $N, but just miss $M.",
            "You feel a slight breeze as $n barely misses you."    }

    };
    struct dam_weapon_type *ptr;

    w_type -= TYPE_HIT;		/* Change to base of table with text */
    wield = ch->equipment[WIELD];

    if(dam == 0)
	ptr = &misses[number(0, (sizeof(misses) / sizeof(*misses)) - 1)];
    else
	for(ptr = dam_weapons ;
	    (ptr->max_dam < dam) && (ptr->max_dam != -1) ;
	    ptr++)
	    ;

    buf = replace_string(ptr->to_room,
			 attack_hit_text[w_type].plural,
			 attack_hit_text[w_type].singular);
    act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
    buf = replace_string(ptr->to_char,
			 attack_hit_text[w_type].plural,
			 attack_hit_text[w_type].singular);
  /*  act(buf, FALSE, ch, wield, victim, TO_CHAR); */
   /* show_dam_check(ch,buf,dam);  */
    act(show_dam_check(ch,buf,dam), FALSE, ch, wield, victim, TO_CHAR);

    buf = replace_string(ptr->to_victim,
			 attack_hit_text[w_type].plural,
			 attack_hit_text[w_type].singular);
  /*  act(buf, FALSE, ch, wield, victim, TO_VICT); */
    /* show_dam_check(ch,buf,dam); */
    act(show_dam_check(ch,buf,dam), FALSE, ch, wield, victim, TO_VICT);
}

int DamApplyProt(struct char_data *v, int dam, int type)
{
    if(IS_AFFECTED(v,AFF_SANCTUARY))
      dam=MAX((int)(dam/2),1);
    //else {
	if(IS_AFFECTED2(v, AFF2_TOLERANCE))
		dam=MAX((int)(dam/2),1);
    //}
    if(IsImmune(v, type))
      dam =IMMUNE_DAMAGE(dam);
    else if (IsResist(v, type))
     dam = RESIST_DAMAGE(dam);
    else if (IsSusc(v, type))
      dam = SUSC_DAMAGE(dam);

   return dam;

}
int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
  struct room_data *rp;
  char buf[MAX_INPUT_LENGTH];

  if(GET_POS(victim) <= POSITION_DEAD)
  {
    sprintf(buf, "DamCheckDeny called for dead victim: %s",
	    GET_NAME(victim));

    log_msg(buf);
    return(TRUE);
  }

  rp = real_roomp(ch->in_room);
  if (rp && (rp->room_flags&PEACEFUL) && type!=SPELL_POISON && type!=SKILL_PULSE) {
    sprintf(buf, "damage(,,,%d) called in PEACEFUL room, room number = %d", type, rp->number);
    log_msg(buf);
    return(TRUE); /* true, they are denied from fighting */
  }
  return(FALSE);

}

int DamDetailsOk( struct char_data *ch, struct char_data *v, int dam, int type)
{
  if (dam < 0) return(FALSE);
  if (type != TYPE_RANGEWEAPON && type!=SKILL_PULSE)
    if (ch->in_room != v->in_room) return(FALSE);
  if (ch == v) return(FALSE);

  if (MOUNTED(ch)) {
    if (MOUNTED(ch) == v) {
      FallOffMount(ch, v);
      Dismount(ch, MOUNTED(ch), POSITION_SITTING);
    }
  }

  return(TRUE);

}


int SetCharFighting(struct char_data *ch, struct char_data *v)
{
  if (GET_POS(ch) > POSITION_STUNNED) {
    if (!(ch->specials.fighting)) {
       set_fighting(ch, v);
       GET_POS(ch) = POSITION_FIGHTING;
    } else {
       return(FALSE);
    }
  }
  return(TRUE);

}


int SetVictFighting(struct char_data *ch, struct char_data *v)
{
    if(!IS_SET(v->specials.mob_act, ACT_PATTACK))
	SET_BIT(ch->specials.mob_act, ACT_PATTACK);
    if ((v != ch) && (GET_POS(v) > POSITION_STUNNED) &&
	(!(v->specials.fighting))) {
	if (ch->attackers < 6) {
	    set_fighting(v, ch);
	    GET_POS(v) = POSITION_FIGHTING;
	}
    } else {
	return(FALSE);
    }
    return(TRUE);
}

int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type)
{

  if (v->master == ch)
    stop_follower(v);

  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
  }

  if (IS_AFFECTED(v, AFF_SANCTUARY)) {
    if (IS_PC(v))
      dam = MAX((int)(dam * 2 / 3), 0);
    else
      dam = MAX((int)(dam/2), 0);/* sanc = 1/2 damage */
  } //else
  if (IS_AFFECTED2(v, AFF2_TOLERANCE)) {
	  if (IS_PC(v))
		  dam = MAX((int)(dam * 2 / 3), 0);
	  else
		  dam = MAX((int)(dam/2), 0); /* tolerance = 1/2 damage */
  }

  if (GET_MOVE(ch)<(GET_MAX_MOVE(ch)*15/100)) {
    dam = dam * 2 / 3;/* low mv = 2/3's normal damage */
    if(number(1,2)==2)
      send_to_char("You're exhausted!  You dont hit as hard.\n\r", ch);
  }
  if ((GET_HIT(ch)<(GET_MAX_HIT(ch)*15/100)) && (GET_HIT(ch)<101)){
    dam = dam * 3 / 2; /* low HPs = 3/2's damage */
    if(number(1,2)==2)
      send_to_char("LOW HIT POINTS!!  You fight HARDER!\n\r", ch);
  }

  dam = PreProcDam(v,type,dam);

  dam = WeaponCheck(ch, v, type, dam);

  DamageStuff(v, type, dam);

  dam=MAX(dam,0);

  /*
   *  check if this hit will send the target over the edge to -hits
   */
  if (GET_HIT(v)-dam < 1) {
    if (IS_AFFECTED(v, AFF_LIFE_PROT)) {
      BreakLifeSaverObj(v);
      return 0;
    }
  }

  if (MOUNTED(v)) {
    if (!RideCheck(v, -(dam/2))) {
      FallOffMount(v, MOUNTED(v));
      WAIT_STATE(v, PULSE_VIOLENCE*2);
      Dismount(v, MOUNTED(v), POSITION_SITTING);
    }
  } else if (RIDDEN(v)) {
    if (!RideCheck(RIDDEN(v), -dam)) {
      FallOffMount(RIDDEN(v), v);
      WAIT_STATE(RIDDEN(v), PULSE_VIOLENCE*2);
      Dismount(RIDDEN(v), v, POSITION_SITTING);
    }
  }

  return(dam);
}

/* critical damage is calculated separetely, since this damage should be included in the damage msg
   makes for more fun and varied damage messages - raist */
int CriticalDamage(struct char_data *ch, struct char_data *v, int dam, int type)
{
  int brett;

  brett=number(1,60);
  if(brett==55 && dam>0 && type!=SKILL_PULSE) {
    dam *= 2;
    act("$Cr$n severely wounds you doing double damage!$CN",
	FALSE,ch,0,v,TO_VICT);
    act("$n $CRdevastates$CN $N with a double damage attack!",
	TRUE,ch,0,v,TO_NOTVICT);
    act("$CRYou severely wound $N doing double damage!$CN",
	FALSE,ch,0,v,TO_CHAR);
  }
  if(brett==50 && dam>0 && type!=SKILL_PULSE) {
    dam = dam * 3 / 2;
    act("$n $CRcritically$CN hits you!",
	FALSE,ch,0,v,TO_VICT);
    act("$n strikes $N with a $CRcritical$CN hit!",
	TRUE,ch,0,v,TO_NOTVICT);
    act("You $CRcritically$CN hit $N!",
	FALSE,ch,0,v,TO_CHAR);
  }
  return dam;
}

int DoDamage(struct char_data *ch, struct char_data *v, int dam, int type)
{
  int brett;

  if (ch->in_room==v->in_room) { //Checking because damage could be done in other rooms!
    if((number(1,5)==3) && (IS_AFFECTED(ch, AFF_FIRESHIELD))) {
      /* Check to see if you are underwater */
      brett = dice(GetMaxLevel(ch), 3);
      if (UNDERWATER(ch))
	brett /= 2;
      brett = DamApplyProt(v, brett, IMM_FIRE);
      DamageStuff(v,SPELL_FIRESHIELD,brett);
      if(!IS_IMMORTAL(v))
        dam += brett;
      if(!UNDERWATER(ch))
	{
	  act("$Cr$n shield of fire engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
 	  act("$Cr$n's fireshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CrYour shield of fire activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
      else
	{
	  act("$Cr$n shield of fire engulfs you, luckily the water seems to reduce its's effectiveness.$CN",FALSE,ch,0,v,TO_VICT);
	  act("$Cr$n's fireshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CrYour shield of fire activates, but you notice the water seems to limit its potency.$CN",dam),FALSE,ch,0,v,TO_CHAR);
	}
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_ELECSHIELD))) {
      /* Check to see if you are underwater */
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_ELEC);
      if (UNDERWATER(ch))
	brett *= 2;
      DamageStuff(v,SPELL_ELECSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      if(!UNDERWATER(ch))
	{
	  act("$Cy$n shield of electricity engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
 	  act("$Cy$n's elecshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CyYour shield of electricity activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
      else
	{
	  act("$Cy$n shield of electricity engulfs you, the water seems to intensify its potency.$CN",FALSE,ch,0,v,TO_VICT);
	  act("$Cy$n's elecshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CyYour shield of electricity activates, you notice the water seems to intensify its potency.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_COLDSHIELD))) {
      /* Check to see if you are underwater */
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_COLD);
      if (UNDERWATER(ch))
	brett *= 2;
      DamageStuff(v,SPELL_COLDSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      if(!UNDERWATER(ch))
	{
	  act("$Cc$n shield of cold engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
 	  act("$Cc$n's coldshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch,"$CcYour shield of cold activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
      else
	{
	  act("$Cc$n shield of cold engulfs you, the water seems to increase its's effectiveness.$CN",FALSE,ch,0,v,TO_VICT);
	  act("$Cc$n's coldshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CcYour shield of cold activates, you notice the water seems to intensify its potency.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_POISONSHIELD))) {
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_POISON);
      if ((number(1,10)==4)){
	//place poison affect
	poison_effect(GetMaxLevel(v),ch,v);
      }
      DamageStuff(v,SPELL_POISONSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      act("$Cg$n shield of poison engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
      act("$Cg$n's poisonshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
      act(show_dam_check(ch, "$CgYour shield of poison activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_ENERGYSHIELD)) && ch->in_room==v->in_room) {
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_ENERGY);
      DamageStuff(v,SPELL_ENERGYSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      act("$Cr$n shield of energy engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
      act("$Cr$n's energyshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
      act(show_dam_check(ch, "$CrYour shield of energy activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
    }


    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_ACIDSHIELD))) {
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_ACID);
      if (UNDERWATER(ch))
	brett /= 2;
      DamageStuff(v,SPELL_ACIDSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      /* Check to see if you are underwater */
      if(!UNDERWATER(ch))
	{
	  act("$Cb$n shield of acid engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
 	  act("$Cb$n's acidshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch, "$CbYour shield of acid activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
      else
	{
	  act("$Cb$n shield of acid engulfs you, luckily the water seems to reduce its's effectiveness.$CN",FALSE,ch,0,v,TO_VICT);
	  act("$Cb$n's acidshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
	  act(show_dam_check(ch,"$CbYour shield of acid activates, but you notice the water seems to limit its potency.$CN",brett),FALSE,ch,0,v,TO_CHAR);
	}
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_VAMPSHIELD))) {
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_DRAIN);
      DamageStuff(v,SKILL_VAMPSHIELD,brett);
      if(!IS_IMMORTAL(v))
	dam += brett;
      act("$Cr$n shield of vampires engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
      act("$Cr$n's vampshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
      act(show_dam_check(ch, "$CrYour shield of vampires  activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
      if(brett > 0){
	act("$CrA stream of power enters you through your shield of vampires.$CN", FALSE, ch, 0, v, TO_CHAR);
	if(GET_HIT(ch) + (brett/2) <= GET_MAX_HIT(ch)){
	  GET_HIT(ch) += (brett/2);
	} else {
	  GET_HIT(ch) = GET_MAX_HIT(ch);
	}
      }
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_MANASHIELD))) {
      brett=dice(GetMaxLevel(ch),3);
      brett = DamApplyProt(v, brett, IMM_DRAIN);
      act("$Cw$n shield of mana engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
      act("$Cw$n's manashield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
      act(show_dam_check(ch,"$CwYour shield of mana activates.$CN",brett),FALSE,ch,0,v,TO_CHAR);
      if(!IS_IMMORTAL(v)) {
	if (GET_MANA(v) < brett) brett = GET_MANA(v);
	if(GET_MANA(v) >= brett) {
	  act("$CwYour magical energies are replenished by your manashield.$CN",FALSE,ch,0,v,TO_CHAR);
	  GET_MANA(v) -= brett;
	  if(GET_MANA(ch) + (brett/2) <= GET_MAX_MANA(ch)) {
	    GET_MANA(ch) += brett/2;
	  } else {
	    GET_MANA(ch) = GET_MAX_MANA(ch);
	  }
	}
      }
    }

    if((number(1,5)==3) && (IS_AFFECTED2(ch, AFF2_MOVESHIELD))) {
      brett=dice(GetMaxLevel(ch),1);
      brett = DamApplyProt(v, brett, IMM_HOLD);
      act("$Cy$n shield of movement engulfs you!$CN",FALSE,ch,0,v,TO_VICT);
      act("$Cy$n's moveshield envelops $N.$CN",FALSE,ch,0,v,TO_NOTVICT);
      act("$CyYour shield of movement activates.$CN",FALSE,ch,0,v,TO_CHAR);
      if(!IS_IMMORTAL(v)) {
	if (GET_MOVE(v) < brett) brett = GET_MOVE(v);
	if(GET_MOVE(v) >= brett) {
	  act("$CyYour movement is replenished by your moveshield.$CN",FALSE, ch,0,v,TO_CHAR);
	  GET_MOVE(v) -= brett;
	  if(GET_MOVE(ch) + (brett/2) <= GET_MAX_MOVE(ch)) {
	    GET_MOVE(ch) += brett/2;
	  } else {
	    GET_MOVE(ch) = GET_MAX_MOVE(ch);
	  }
	}
      }
    }
  }
  if (IS_AFFECTED2(ch, AFF2_DESPAIR))
      dam = (int) dam/2;

  GET_HIT(v)-=dam;
  update_pos(v);

  return(FALSE);

}


void DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype)
{
  int nr, max_hit, i, j;
  struct message_type *messages;
  char buf[255];

  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_RANGEWEAPON)) {
    dam_message(dam, ch, v, attacktype);
    if (ch->equipment[WIELD])
      BrittleCheck(ch, dam);
  } else {  /* spell attack */

    for(i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) {
	//	sprintf(buf, "Got here with attacktype = %d\r\n", attacktype);
	//	log_msg(buf);
	nr=dice(1,fight_messages[i].number_of_attacks);

	for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	  messages=messages->next;

	if (!IS_NPC(v) && IS_IMMORTAL(v)) {
	  act(show_dam_check(ch,messages->god_msg.attacker_msg, dam),
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(show_dam_check(ch,messages->god_msg.victim_msg,dam),
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->god_msg.room_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
/*	  act(messages->god_msg.attacker_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->god_msg.victim_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->god_msg.room_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT); */
	} else if (dam != 0) {
	  if (GET_POS(v) == POSITION_DEAD) {
	    act(show_dam_check(ch,messages->die_msg.attacker_msg, dam),
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(show_dam_check(ch,messages->die_msg.victim_msg, dam),
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->die_msg.room_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
/*	    act(messages->die_msg.attacker_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->die_msg.victim_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->die_msg.room_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT); */
	  } else {
	    act(show_dam_check(ch,messages->hit_msg.attacker_msg, dam),
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(show_dam_check(ch,messages->hit_msg.victim_msg, dam),
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->hit_msg.room_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
/*	    act(messages->hit_msg.attacker_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->hit_msg.victim_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->hit_msg.room_msg,
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT); */
	  }
	} else {		/* Dam == 0 */
	  act(show_dam_check(ch,messages->miss_msg.attacker_msg, dam),
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(show_dam_check(ch,messages->miss_msg.victim_msg, dam),
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->miss_msg.room_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
/*	  act(messages->miss_msg.attacker_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->miss_msg.victim_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->miss_msg.room_msg,
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT); */
	}
      }
      else {
	//	sprintf(buf, "fight_msg type = %d, attack type = %d", fight_messages[i].a_type, attacktype);
	//	log_msg(buf);
      }
    }
  }
  switch (GET_POS(v)) {
  case POSITION_MORTALLYW:
    act("$n is mortally wounded.",
	TRUE, v, 0, 0, TO_ROOM);
    act("You are mortally wounded.",
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_INCAP:
    act("$n is incapacitated.",
	TRUE, v, 0, 0, TO_ROOM);
    act("You are incapacitated.",
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_STUNNED:
    act("$n is stunned.",
	TRUE, v, 0, 0, TO_ROOM);
    act("You're stunned.",
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$CGYou are dead!$CN  Sorry...", FALSE, v, 0, 0, TO_CHAR);
    break;

  default:			/* >= POSITION SLEEPING */

    max_hit=hit_limit(v);

    if (dam > (max_hit/5)) {
      act("That Really $CRHURT!$CN",FALSE, v, 0, 0, TO_CHAR);
    }
    if ( (!GET_WIMPY(v) && (GET_HIT(v) < (max_hit/5) && GET_HIT(v) > 0)) ||
	(GET_WIMPY(v) && (GET_HIT(v) <= GET_WIMPY(v)) )) {
      if(number(1,2)==2)
        act("You wish that your wounds would stop $CRBLEEDING$CN so much!",
	    FALSE,v,0,0,TO_CHAR);

      if(IS_SET(v->specials.mob_act, ACT_WIMPY) ||
	 IS_SET(v->specials.flags, PLR_WIMPY))
	do_flee(v, "", 0);
    }
    if (MOUNTED(v)) {
      /* chance they fall off */
      RideCheck(v, -dam/2);
    }
    if (RIDDEN(v)) {
      /* chance the rider falls off */
      RideCheck(RIDDEN(v), -dam);
    }
    break;
  }
}


int DamageEpilog(struct char_data *ch, struct char_data *victim, int at_type)
{
  struct descriptor_data *i;
  int agg, arena, loot, a;
  int remains = 0;
  int carcass = 0;
  int undead = 0;
  int ispkill = 0;
  int ispoly = 0;
  EXP exp;
  //char buf[256];
  char buf[MAX_INPUT_LENGTH];
  struct char_data *mast, *victbackup;
  struct obj_data *obj;

  victbackup=victim; //quilan safety measure

  // Changed since if arena area players can kill mobs for xp
  // This is being exploited - Raist
  //  arena = IS_SET(RM_FLAGS(victim->in_room), ARENA) && IS_PC(victim);
  arena = IS_SET(RM_FLAGS(victim->in_room), ARENA);


#ifdef JANWORK
  if(IS_PC(victim) &&
     ((!victim->desc) && !IS_GOD(victim))) { /* if no descriptor */
    do_flee(victim, "", 0);
    act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);

    victim->specials.was_in_room = victim->in_room;

    if (victim->in_room != NOWHERE)
      char_from_room(victim);

    char_to_room(victim, 4);

    if (IS_NPC(victim)) {
      do_return(victim, "", 0);
    }
    if (!arena)
      {
	exp = exp_table[GetMaxLevel(victim)]; /* total amount of exp for level */
	exp /= 20; /* 5% of level experience lost for xlinkloses */
	sprintf(buf,"%s lost experience due to fleeing.",
		GET_REAL_NAME(victim));
	log (buf);
	if(GET_EXP(victim)<0) GET_EXP(victim)=0;
      }
      return(FALSE);
  }
#endif

  agg = IS_SET(victim->specials.mob_act, ACT_PATTACK) ? 0 : 1;

  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);

  if (GET_POS(victim) == POSITION_DEAD) {

    ring_log("%s killed %s (at_type=%d)\n",
	     GET_NAME(ch), GET_NAME(victim), at_type);

    if (ch->specials.fighting == victim)
      stop_fighting(ch);
    if(!IS_PC(victim) && !arena)
	{
	  if (IS_AFFECTED(ch, AFF_GROUP)) {
	    group_gain(ch, victim);
	  } else {
	    /* Calculate level-difference bonus */
	    exp = GET_EXP(victim);

	    /*	    exp = MAX(exp, 1);*/
	    exp = MIN(exp, 250000);
	    /*
	       exp = LevelMod(ch, victim);
	       */
	    gain_exp(ch, exp);
	    change_alignment(ch, victim);
	  }
	}
    else // Else player is victim who is by ch killed
      {
	struct room_data* room = real_roomp(victim->in_room);
	char* name, namebuf[256];
	char* pkill;
	char* where = (char *) (room ? room->name : "nowhere");
	char* vict, victbuf[256];
	int legalpkill=0;

	static char* death_messages[] =
	  {
	    "\n\r   ### %s was just killed by %s in cold blood.\n\r",
	    "\n\r   ### %s was just killed by %s mercilessly.\n\r",
	    "\n\r   ### %s was just killed by %s mercilessly in cold blood.\n\r",
	    "\n\r   ### %s was just massacred to death by %s.\n\r",
	    "\n\r   ### %s just became vitality impaired.\n\r",
	    "\n\r   ### %s has been murdered by %s in cold blood!\n\r",
	    "\n\r   ### Death has taken the life of %s.\n\r",
	    "\n\r   ### %s's life has been taken by %s.\n\r",
	    "\n\r   ### %s has been demolished by %s.\n\r",
	    "\n\r   ### The soul of %s has left the corpse and went to HELL!!!!\n\r",
	    "\n\r   ### %s has been minimized to dust.\n\r",
	    "\n\r   ### %s has left the mortal plane rather abruptly...\n\r"
	  };

	static char* arena_death_messages[] =
	  {
	    "\n\r   ### %s was just bested by %s in the arena.\n\r",
	    "\n\r   ### %s has been massacred to death in the Arena by %s.\n\r",
	    "\n\r   ### %s has just been added to the Arena death toll!\n\r"
	  };

	legalpkill = can_pkill(ch, victim);
	if(IS_PC(ch))
	  {
	    ispkill = (ch != victim);
	    if(!legalpkill)
		pkill = " -- <Illegal Player kill>";
	    else
		pkill = " -- <Legal Player Kill>";
	    sprintf(buf, "%s killed by %s at %s %s", GET_NAME(victim), GET_NAME(ch), where, pkill);
	    file_log(buf, PKILL_LOG);
	    if(agg && !arena &&
	       !IS_SET(victim->specials.flags, PLR_LOSER) &&
	       !legalpkill)
	      SET_BIT(ch->specials.flags, PLR_PKILLER);
	    if(IS_NPC(ch))
	      sprintf(name = namebuf, "%s (%s)",
		      GET_NAME(ch), GET_NAME(real_character(ch)));
	    else
	      name = GET_REAL_NAME(ch);
	  }
	else if (IS_NPC(ch) && ( (mast=ch->master)!= NULL) && (IS_PC(mast)))
	  {
	    ispkill = (ch != victim);
	    sprintf(buf, "%s killed by %s under the command of %s at %s %s", GET_NAME(victim), GET_NAME(ch), GET_NAME(mast), where, pkill);
	    file_log(buf, PKILL_LOG);
	    if(!legalpkill)
		pkill = " -- <Illegal Player kill>";
	    else
		pkill = " -- <Legal Player Kill>";
	    if(agg && !arena &&
	       !IS_SET(victim->specials.flags, PLR_LOSER) &&
	       !legalpkill)
	      SET_BIT(mast->specials.flags, PLR_PKILLER);
	    if(IS_NPC(mast))
	      sprintf(name = namebuf, "%s (%s)",
		      GET_NAME(mast), GET_NAME(real_character(mast)));
	    else
	      name = GET_REAL_NAME(mast);
	  }
	else
	  {
	    pkill = "";
	    name = GET_REAL_NAME(ch);
	  }

	if (!arena)
	  {
	    REMOVE_BIT(victim->specials.flags, PLR_PKILLER);
	    REMOVE_BIT(victim->specials.flags, PLR_LOSER);
	    REMOVE_BIT(victim->specials.flags, PLR_THIEF);
	  }

	if(IS_NPC(victim))
	  sprintf(vict = victbuf, "%s (%s)",
		  GET_NAME(victim),
		  GET_NAME(real_character(victim)));
	else
	  vict = GET_REAL_NAME(victim);

	if (IS_NPC(ch)) {
	  brag(ch, victim);    /* <- Insert brag call here */
	}

	if (!arena)
	  {

	    sprintf(buf, "%s killed by %s at %s%s",
		    vict, name, where, pkill);
	    log_msg(buf);
	  }

 	if (arena)
	  sprintf(buf, arena_death_messages[number(0,
						   sizeof(arena_death_messages) /
						   sizeof(*arena_death_messages)-1)],
		  vict, name);
	else
	  if (!arena)
	    sprintf(buf, death_messages[number(0, sizeof(death_messages) /
					       sizeof(*death_messages) - 1)],
		    vict, name);

	EACH_DESCRIPTOR(d_iter, i) // send announcement message to mud
	  {
            if (i->character != ch && i->character != victim &&
                !i->connected &&
                !IS_SET(i->character->channels, COM_SHOUT) &&
                !check_soundproof(i->character)) {
              if(CheckColor(i->character))
                send_to_char(ANSI_RED, i->character);
              send_to_char(buf, i->character);
              if(CheckColor(i->character))
                send_to_char(ANSI_NORMAL, i->character);
            }
	  }
	END_ITER(d_iter);
      }

    victim=victbackup;


    if (ch && IS_SET(ch->specials.flags, PLR_AUTOLOOT | PLR_AUTOGOLD) &&
	(!IS_PC(victim) || IS_MOB(ch)))
      {
	loot = TRUE;

	if (IsUndead(victim))
	  undead = TRUE;
	if ((GET_RACE(victim) == RACE_GOLEM) || (GET_RACE(victim) == RACE_ELEMENTAL))
	  remains = TRUE;
	if (GET_RACE(victim) == RACE_SHERRINPIP)
	  carcass = TRUE;
      }
    else
      loot = FALSE;


    victim=victbackup; //quilan safety measure

    // for Scavengers to Autoloot
    if (ch && IS_SET(ch->specials.mob_act, ACT_SCAVENGER) && !IS_PC(ch))
      loot = true;
    mprog_kill_trigger2(ch, victim);
    mprog_death_trigger(victim, ch);
    mprog_death_trigger2(victim, ch);
    act("$n is dead! $CGR.I.P.$CN", TRUE, victim, 0, 0, TO_ROOM);


    int gainedpoints = MAX(1, (GetMaxLevel(victim)-GetMaxLevel(ch))/5+1);
    if (ispkill && !arena) {
      if (IS_PC(ch))
	ch->player.pkillinfo.count += gainedpoints;

	if (IS_PC(victim))
	  victim->player.pkillinfo.killed++;
    }

    if (ispkill && !arena && PKILLABLE!=2) {
        int pos;
	int numeq=0, ecount=0;
	for(a=0;a<MAX_WEAR;a++)
	    if(victim->equipment[a])
		if(!victim->equipment[a]->obj_flags.no_loot)
		    numeq++;
        for(a=0;a<MIN(2,numeq);a++) {
            pos = number(0, MAX_WEAR);
            obj = victim->equipment[pos];
            if(obj) {
                if ((1+IS_CARRYING_N(ch)) > CAN_CARRY_N(ch)) {
                    send_to_char("You seem to have your hands full.\n\r", ch);
                    break;
                } else if (!obj->obj_flags.no_loot) {
		    remove_equip(victim, pos, FALSE);
	            obj_from_char(obj);
		    obj_to_char(obj, ch);
                    sprintf(buf, "You take $p from %s's dead corpse.\n\r",
                                  GET_NAME(victim));
		    act(buf, 0, ch, obj, 0, TO_CHAR);
		    sprintf(buf, "%s takes $p from %s's dead corpse.\n\r",
				  GET_NAME(ch), GET_NAME(victim));
		    act(buf, 1, ch, obj, 0, TO_ROOM);
		    if(++ecount > 2) break;	//fail safe method
                } else
		    a--;
            }
        }
    }

    //quilan
    victim=victbackup; //quilan safety measure

    if (IS_SET(victim->specials.mob_act, ACT_POLYSELF)) ispoly = 1;

    //some debugging stuff...
    struct char_data *v2=victim;

    v2=((ispoly==1) ? real_character(victim) : victim);
    v2->player.pkillinfo.status = (ispkill && !arena);

    make_fun_body_pieces(victim, ch);

    die(victim);

    //    v2->player.pkillinfo.status = 0;

    if(ispkill && !arena) {
      sprintf(buf, "You just gained $CW%d$CN pkill point%s.\n\r", gainedpoints, (gainedpoints==1)?"":"s");
      send_to_char_formatted(buf, ch);
    }

    if (loot && !ispkill)
      {
	sprintf(buf, " all%s %s",
		(IS_SET(ch->specials.flags, PLR_AUTOLOOT) ?
		 "" : ".gold-coins"), undead ? "dust" : remains
		? "remains" : carcass ?"carcass" : "corpse" );
	do_get(ch, buf, 0);
      }

    /*
     *  if the victim is dead, return TRUE.
     */
    return(TRUE);

  } else {
    return(FALSE);
  }
}

int MissileDamage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype)
{

   if (DamCheckDeny(ch, victim, attacktype))
     return(FALSE);

   dam = SkipImmortals(victim, dam);

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, attacktype);

   if (DamageEpilog(ch, victim, attacktype)) return(TRUE);

   SetVictFighting(ch, victim);

   return(FALSE);  /* not dead */
}

int damage(struct char_data *ch, struct char_data *victim,
	   int dam, int attacktype)
{
  int is_ctf;
  Variable *cv, *vv;

  ch->player.has_killed_victim = 1;

  if (DamCheckDeny(ch, victim, attacktype)) {
    ch->player.has_killed_victim = 0;
    return(FALSE);
  }

  cv = get_mob_var(ch, "ctf_team");
  vv = get_mob_var(victim, "ctf_team");
  if(cv && vv) {
     is_ctf=(((ch->in_room >= 33300) &&
   	      (ch->in_room <= 33398)) &&
	     (cv->Value() && vv->Value()) &&
	     (cv->Value() == vv->Value()));
  } else {
     is_ctf=0;
  }

  if(is_ctf) return FALSE;

  dam = SkipImmortals(victim, dam);

  if (!DamDetailsOk(ch, victim, dam, attacktype)) {
    ch->player.has_killed_victim = 0;
    return(FALSE);
  }

  dam = DamageTrivia(ch, victim, dam, attacktype);
  dam = CriticalDamage(ch, victim, dam, attacktype);

  if (DoDamage(ch, victim, dam, attacktype))
    return(TRUE);

  DamageMessages(ch, victim, dam, attacktype);

  if (DamageEpilog(ch, victim, attacktype)) {
    return(TRUE);
  }

  if (attacktype != TYPE_RANGEWEAPON) {
    SetVictFighting(ch, victim);
    SetCharFighting(ch, victim);
  }

  ch->player.has_killed_victim = 0;
  return(FALSE);/* not dead */
}

int GetWeaponType(struct char_data *ch, struct obj_data **wielded)
{
  int w_type;

  if (ch->equipment[WIELD] &&
      (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {

    *wielded = ch->equipment[WIELD];
    w_type = Getw_type(*wielded);

  } else {
    if (ch->specials.attack_type >= TYPE_HIT)
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;

    *wielded = 0;  /* no weapon */

  }
  return(w_type);

}

int Getw_type(struct obj_data *wielded)
{
  int w_type;

  if(wielded)
    switch (wielded->obj_flags.value[3]) {
    case 0  : w_type = TYPE_BLUDGEON; break;
    case 1  : w_type = TYPE_CRUSH;    break;
    case 2  : w_type = TYPE_SMASH;    break;
    case 3  : w_type = TYPE_SMITE;    break;
    case 4  : w_type = TYPE_WHIP;     break;
    case 5  : w_type = TYPE_STAB;     break;
    case 6  : w_type = TYPE_PIERCE;   break;
    case 7  : w_type = TYPE_CLAW;     break;
    case 8  : w_type = TYPE_BITE;     break;
    case 9  : w_type = TYPE_STING;    break;
    case 10 : w_type = TYPE_SLASH;    break;
    case 11 : w_type = TYPE_CLEAVE;   break;
    case 12 : w_type = TYPE_RANGEWEAPON; break;

    default : w_type = TYPE_HIT; break;
    }
  else w_type=TYPE_HIT;
  return(w_type);
}

int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
  struct room_data *rp;
  char buf[256];

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    sprintf(buf, "hit() called in PEACEFUL room [%ld]", ch->in_room);
    log_msg(buf);
    stop_fighting(ch);
    return(TRUE);
  }

  if (ch->in_room != victim->in_room) {
    sprintf(buf, "NOT in same room when fighting : %s, %s, [%ld, %ld]",
	GET_NAME(ch), GET_NAME(victim), ch->in_room, victim->in_room);
    slog(buf);
    stop_fighting(ch);
    return(TRUE);
  }

  if (victim->attackers >= 25 && ch->specials.fighting != victim) {
    send_to_char("You can't attack them,  no room!\n\r", ch);
    return(TRUE);
  }

/*
   if the character is already fighting several opponents, and he wants
   to hit someone who is not currently attacking him, then deny them.
   if he is already attacking that person, he can continue, even if they
   stop fighting him.
*/
  if ((ch->attackers >= 25) && (victim->specials.fighting != ch) &&
      ch->specials.fighting != victim) {
    send_to_char("There are too many other people in the way.\n\r", ch);
    return(TRUE);
  }

  if (victim == ch) {
    if (Hates(ch,victim)) {
      RemHated(ch, victim);
    }
    return(TRUE);
  }

  if (GET_POS(victim) == POSITION_DEAD)
    return(TRUE);

  if (MOUNTED(ch)) {
    if (!RideCheck(ch, -5)) {
      FallOffMount(ch, MOUNTED(ch));
      Dismount(ch, MOUNTED(ch), POSITION_SITTING);
      return(TRUE);
    }
  } else {
    if (RIDDEN(ch)) {
      if (!RideCheck(RIDDEN(ch),-10)) {
	FallOffMount(RIDDEN(ch), ch);
	Dismount(RIDDEN(ch), ch, POSITION_SITTING);
	return(TRUE);
      }
    }
  }


  return(FALSE);

}

/* caclulate thaco. the lower the better */
int CalcThaco(struct char_data *ch)
{
  int calc_thaco;

  if (!IS_NPC(ch))
   calc_thaco=thaco[BestFightingClass(ch)][(int)GET_LEVEL(ch,BestFightingClass(ch))];
  else
   calc_thaco = 30; /* mob thaco is in their hitroll */

  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  return(calc_thaco);
}

/* Return TRUE(1) = hit, FALSE(0) = miss, -1 = miss + no msg desired */

int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco)
{
  int percenthit, diceroll, tohit, ac_apply, dexmod;
  int difflevels, numattackers, dodge, roll, chance;
  float dodgechance;

/* check for automatic hit situations */
  if(IS_AFFECTED(victim, AFF_PARALYSIS) ||
      affected_by_spell(victim, SPELL_WEB) ||
      (GET_POS(victim) < POSITION_FIGHTING))
    return(1);

  /* factoring illusionary shroud into hit or miss */
  if(IS_AFFECTED(victim,AFF_ILLUSION))  {
      percenthit=number(1,(GetMaxLevel(victim)/10));
      dodge=number(1,5);

      if(percenthit==dodge)   {

	  act("$Cw$N attacks an illusion of $n$CN",FALSE,victim,0,ch,TO_NOTVICT);
	  act("$N misses",FALSE,victim,0,ch,TO_CHAR);
	  act("$CwYour blow lands on an illusion$CN",FALSE,victim,0,ch,TO_VICT);
	  return (-1);
      }
  }

  if(victim->skills && victim->skills[SKILL_SHIELD_BLOCK].learned)
   if(victim->equipment[WEAR_SHIELD])
   {
	  percenthit=number(1, (GetMaxLevel(victim)/20));
	  dodge=number(1,5);

	  if(percenthit==dodge)
	  {
	    act("$Cc$n blocks $N's hit with $s shield",FALSE,victim, 0, ch, TO_NOTVICT);
		act("$Cc$N is blocked by your shield",FALSE,victim, 0, ch, TO_CHAR);
		act("$CcYour blow is deflected by $n's shield",FALSE,victim,0,ch,TO_VICT);
		return(-1);
	  }
   }


  /* factoring dodge into a hit or miss */
  if(IS_AFFECTED(victim,AFF_DODGE) ||
     (victim->skills))   {

     dodgechance = .40; /* base 40% chance success of dodge */
     if (!(GET_DEX(victim) > 18)) /* dexterity modifier */
       dodgechance *= ((float) GET_DEX(victim)) / 18.0;
     if (!IS_AFFECTED(victim,AFF_DODGE)) /* must be a player skill */
       dodgechance *= ((float) victim->skills[SKILL_DODGE].learned) / 100.0;
     difflevels = GetMaxLevel(ch) - GetMaxLevel(victim);
     if (difflevels > 11)
	  dodgechance = .10; /* base 10% chance success if attacker > 10 levels above */
     if (difflevels < 0) /* victim is higher level than attacker */
	  difflevels = 0;
     dodgechance *= (float) (10-difflevels) / 10.0;
     dodgechance *= 100.0; /* number from 1 - 100 */


/*    if(IS_NPC(victim) || !victim->skills)
      dodge=GET_DEX(victim);
    else
      dodge=(victim->skills[SKILL_DODGE].learned)/(number(3,5));
    if(dodge > 30)
      dodge=30;
    percenthit=number(5,30);

    sprintf(buf,"%s attempts to dodge %s percenthit(5,30)=%d > %d",
            GET_REAL_NAME(victim),GET_REAL_NAME(ch),percenthit,dodge);
    stat_log(buf,0);
    if(percenthit > dodge && (GET_POS(victim) > POSITION_SITTING))   { */

    roll = percent();
    if ((int) dodgechance >= roll)  {

     act("$Cb$n dodges $N$CN",FALSE,victim,0,ch,TO_ROOM);
     act("$CbYou dodge $N$CN",FALSE,victim,0,ch,TO_CHAR);
     act("$Cb$n dodges$CN",FALSE,victim,0,ch,TO_VICT);
     return(-1);
    }
  }


  //don't let mobs at higher level get a guarantee hit...lets just make mobs smarter
  difflevels=(MIN(GetMaxLevel(ch), MAX_MORT)-MIN(GetMaxLevel(victim), MAX_MORT))*80/100;
  tohit=GET_HITROLL(ch)/5;
  dexmod=GET_DEX(ch) - GET_DEX(victim);
  ac_apply=MIN(20, MAX(-20, (GET_AC(victim))/10));
  numattackers=4+(victim->attackers);
  percenthit=(60+difflevels+tohit+dexmod+ac_apply)*5/numattackers;

  if (IS_AFFECTED(ch,AFF_BLIND)) {
	  if(ch->skills && ch->skills[SKILL_BLIND_FIGHTING].learned) {
		  chance = (ch->skills[SKILL_BLIND_FIGHTING].learned);
	      if(number(1, 101) > chance)
			   percenthit /= 2;
          else
			   percenthit = percenthit;
	  }
      else
		  percenthit /= 2;
  }
  //if (percenthit > 57) percenthit = 57;
  if (percenthit < 2) percenthit = 2;
  diceroll=number(1,100);
  if (diceroll < percenthit)   {

   return(TRUE);
  }
  else
  {

    return(FALSE);
  }
}

int GetWeaponDam(struct char_data *ch, struct char_data *v,
		 struct obj_data *wielded) {
  int dam;
  struct obj_data *obj;

  dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
  dam += GET_DAMROLL(ch);

  if (!wielded) {
    dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
  } else {
    if (wielded->obj_flags.value[2] > 0) {
      dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
    } else {
      act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_CHAR);
      act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_ROOM);
      if ((obj = unequip_char(ch, WIELD))!=NULL) {
	MakeScrap(ch, obj, BLOW_DAMAGE);
	dam += 1;
      }
    }
  }
  /* maarek */
  if(IS_AFFECTED(ch, AFF_BERSERK) && ch->skills)
    dam += (int)(dam * ((GetMaxLevel(ch)/100.0) ));


  if (GET_POS(v) <= POSITION_DEAD)
    return(0);

  dam = MAX(1, dam);  /* Not less than 0 damage */

  return(dam);

}


void MissVictim(struct char_data *ch, struct char_data *v, int type,
	       int w_type, struct obj_data* w, damage_func dam_func) {

  if (type <= 0) type = w_type;
  (*dam_func)(ch, v, 0, w_type);
}


void HitVictim(struct char_data *ch, struct char_data *victim,
	       int dam, int type, int w_type,
	       struct obj_data* w, damage_func dam_func)
{
  int dead;
  int mult;

  /* This code added by Min to check for Paladins doing naughty things */
 /*---
  | Commenting this out to bring paladins back into the game...
  -----
  if (HasClass(ch, CLASS_PALADIN))
    if (IS_GOOD(ch) && IS_GOOD(victim) && !HAS_GCMD(ch,GCMD_HIGH)
      {
	GET_ALIGNMENT(ch) = -350; // becomes an evil paladin
	send_to_char("\n\r",ch);
	act("$CRYou strike $N and realize your mistake!!!!", FALSE, ch, 0, victim, TO_CHAR);
	act("$CBA bolt of blue light strikes down from the heavens and hits you!",
	    FALSE, ch, 0, victim, TO_CHAR);
	act("$CYYour deity has forsaken you.", FALSE, ch, 0, victim, TO_CHAR);
	act("$CR$n has gone astray and has attacked you!", TRUE, ch, 0, victim, TO_VICT);
	act("$CR$n gets an evil look and strikes $N!", TRUE, ch, 0, victim, TO_ROOM);
	act("$CB$n bathes in the glow of a shaft of light from the heavens!",
	    TRUE, ch, 0, victim, TO_ROOM);
      }
  */


  if (type == SKILL_BACKSTAB) {
    mult = backstab_mult[(int)GET_LEVEL(ch, THIEF_LEVEL_IND)];
    dam = dam * mult - (mult-1) *
      ( str_app[STRENGTH_APPLY_INDEX(ch)].todam + GET_DAMROLL(ch));
    /* the old way was a tad powerful
       dam *= backstab_mult[GET_LEVEL(ch,THIEF_LEVEL_IND)];*/
    dead = (*dam_func)(ch, victim, dam, type);
  }
  else
    {
      dead = (*dam_func)(ch, victim, dam, w_type);
    }

  /*
   *  if the victim survives, lets hit him with a
   *  weapon spell
   */

  if (!dead && (dam > 0))
    if(number(1,10)<3)
      WeaponSpell(ch, victim, w_type, w);
}


void root_hit(struct char_data *ch, struct char_data *victim, int type, damage_func dam_func) {

  int w_type, thaco = 0, dam;
  struct obj_data *wielded=0;  /* this is rather important. */

  DLOG(("Beginning root_hit\n"));

  if (HitCheckDeny(ch, victim, type))
    return;

  DLOG(("Beginning dual_wield check in root_hit\n"));

  if(type == SKILL_DUAL_WIELD) {
    wielded = ch->equipment[HOLD];
    if(wielded && (wielded->obj_flags.type_flag != ITEM_WEAPON)) {
      char buf[256];
      sprintf(buf, "%s: using dual wield without a weapon...",
	      GET_REAL_NAME(ch));
      log_msg(buf);
      return;
    }
    w_type = Getw_type(wielded);
    thaco = +5;			// offhand has a -5 penalty
  }
  else {
    w_type = GetWeaponType(ch, &wielded);
    thaco = 0;
  }

  DLOG(("Calculating THAC0 in root_hit\n"));
  thaco += CalcThaco(ch);

  DLOG(("HitOrMiss in root_hit\n"));
  switch (HitOrMiss(ch, victim, thaco)) {
  case 1:
    if ((dam = GetWeaponDam(ch, victim, wielded)) > 0) {
      HitVictim(ch, victim, dam, type, w_type, wielded, dam_func);

    } else {
      MissVictim(ch, victim, type, w_type, wielded, dam_func);
    }
    break;
  case 0:
    MissVictim(ch, victim, type, w_type, wielded, dam_func);
    break;
  case -1:
    if(ch->attackers < 11)
      if (!victim->specials.fighting)
      	set_fighting(victim, ch);
    if (victim->attackers < 6)
      if(!ch->specials.fighting)
      	set_fighting(ch,victim);
    break;
  default:
    log_msg("Unknown return value from HitOrMiss in root_hit");
  }
  DLOG(("Done root_hit\n"));
}

void MissileHit(struct char_data *ch, struct char_data *victim, int type)
{
  root_hit(ch, victim, type, MissileDamage);
}

void hit(struct char_data *ch, struct char_data *victim, int type) {
  root_hit(ch, victim, type, damage);
}

void range_hit(struct char_data *ch, struct char_data *targ,
               struct obj_data *missile, int dir, int range)
{
   int calc_thaco, victim_ac, dam, cdir, cdr, rng;
   int opdir[] = {2, 3, 0, 1, 5, 4};
   char *dir_name[] = {
       "the north",
       "the east",
       "the south",
       "the west",
       "above",
       "below"};
   char buf[MAX_STRING_LENGTH];

   calc_thaco = CalcThaco(ch);
   calc_thaco -= GET_HITROLL(ch);
   calc_thaco += 2*range;
   if (GET_POS(targ)<=POSITION_SITTING)
     calc_thaco += 5;

   victim_ac  = GET_AC(targ);
   if (AWAKE(targ))
     victim_ac += dex_app[GET_DEX(targ)].defensive;
   victim_ac  = MAX(-20, MIN(20, victim_ac/10)); /* -20 to +20 */

   if ((calc_thaco-number(1,20))>victim_ac || IsImmune(ch,IMM_PIERCE)) {
     obj_to_room(missile, targ->in_room);
     act("You missed $N!", TRUE,ch,0,targ,TO_CHAR);
     act("$n narrowly misses $N!", TRUE,ch,0,targ,TO_ROOM);
     sprintf(buf,"$p from %s narrowly misses you!\n\r",dir_name[opdir[dir]]);
     act(buf, TRUE,targ,missile,0,TO_CHAR);
     sprintf(buf,"$p from %s narrowly misses $n!",dir_name[opdir[dir]]);
     act(buf, TRUE,targ,missile,0,TO_ROOM);

     if (!IS_PC(targ)) {
       AddHated(targ,ch);
       cdir = can_see_linear(targ,ch,&rng,&cdr);
       if (cdir!=-1 && GET_POS(targ)>POSITION_SLEEPING &&
             GET_POS(targ)<POSITION_MOUNTED && !targ->specials.fighting) {
         StandUp(targ);
         act("$n roars angrily and charges!",TRUE,targ,0,0,TO_ROOM);

         targ->specials.charging = ch;
         if (!ch || !targ->specials.charging) {
            sprintf(buf, "Problem with range_hit() -- not setting target's charging:  %s\n\r", GET_NAME(targ));
	    log_msg(buf);
         }
         targ->charge_dir = cdir;

	 targ->next_charging = charging_list;
	 charging_list = targ;
       }
     }
     return;
   }

   obj_to_char(missile, targ);
/* dam = dice(missile->obj_flags.value[1],missile->obj_flags.value[2]); */
dam=dice(missile->obj_flags.value[1],GetMaxLevel(ch));

   sprintf(buf,"$p from %s hits you!\n\r",dir_name[opdir[dir]]);
   act(buf, TRUE,targ,missile,0,TO_CHAR);
   sprintf(buf,"$p from %s hits $n!",dir_name[opdir[dir]]);
   act(buf, TRUE,targ,missile,0,TO_ROOM);
   sprintf(buf,"%s is dead! $CwR.I.P.$CN\n\r", GET_NAME(targ));

   if (!damage(ch, targ, dam, TYPE_RANGEWEAPON)) { /* didn't die so... */
     if (!IS_PC(targ)) {
       AddHated(targ,ch);
       cdir = can_see_linear(targ,ch,&rng,&cdr);
       if (cdir!=-1 && GET_POS(targ)>POSITION_SLEEPING &&
           GET_POS(targ)<POSITION_MOUNTED && !targ->specials.fighting) {
         StandUp(targ);
         act("$n roars angrily and charges!",TRUE,targ,0,0,TO_ROOM);
         targ->specials.charging = ch;
         targ->charge_dir = cdir;

	 targ->next_charging = charging_list;
	 charging_list = targ;
       }
     }
   } else {
     send_to_room_formatted(buf, ch->in_room);
   }

   return;
}

#if 0
/* control the fights going on */
struct attack_counts
{
    int class, level, low, high, degen;
}
attack_counts[] =
{
    {	WARRIOR_LEVEL_IND,	125,	6,	11,	101 	},
    {	WARRIOR_LEVEL_IND,	115,	6,	10,	101 	},
    {	WARRIOR_LEVEL_IND,	102,	5,	9,	101 	},
    {	WARRIOR_LEVEL_IND,	89,	4,	8,	101 	},
    {	WARRIOR_LEVEL_IND,	76,	3,	7,	101 	},
    {	WARRIOR_LEVEL_IND,	63,	3,	6,	101 	},
    {	WARRIOR_LEVEL_IND,	50,	2,	5,	101 	},
    {	WARRIOR_LEVEL_IND,	37,	2,	4,	101 	},
    {	WARRIOR_LEVEL_IND,	25,	2,	3,	101 	},
    {	WARRIOR_LEVEL_IND,	12,	2,	2,	101 	},

    {   MONK_LEVEL_IND,         50,     2,      5,      101     },
    {   MONK_LEVEL_IND,         37,     2,      4,      101     },
    {   MONK_LEVEL_IND,         25,     2,      3,      101     },
    {   MONK_LEVEL_IND,         12,     2,      2,      101     },

    {	RANGER_LEVEL_IND,	125,	5,	9,	101 	},
    {	RANGER_LEVEL_IND,	116,	4,	8,	101 	},
    {	RANGER_LEVEL_IND,	100,	3,	7,	101 	},
    {	RANGER_LEVEL_IND,	82,	3,	6,	101 	},
    {	RANGER_LEVEL_IND,	66,	2,	5,	101 	},
    {	RANGER_LEVEL_IND,	50,	2,	4,	101 	},
    {	RANGER_LEVEL_IND,	32,	2,	3,	101 	},
    {	RANGER_LEVEL_IND,	16,	1,	2,	101 	},

    {	PALADIN_LEVEL_IND,	125,	5,	9,	101 	},
    {	PALADIN_LEVEL_IND,	116,	4,	8,	101 	},
    {	PALADIN_LEVEL_IND,	100,	3,	7,	101 	},
    {	PALADIN_LEVEL_IND,	82,	2,	6,	101 	},
    {	PALADIN_LEVEL_IND,	66,	2,	5,	101 	},
    {	PALADIN_LEVEL_IND,	50,	2,	4,	101 	},
    {	PALADIN_LEVEL_IND,	32,	2,	3,	101 	},
    {	PALADIN_LEVEL_IND,	16,	1,	2,	101 	},

    {   BARD_LEVEL_IND,         125,     5,      9,      101     },
    {   BARD_LEVEL_IND,         116,     4,      8,      101     },
    {   BARD_LEVEL_IND,         100,     3,      7,      101     },
    {   BARD_LEVEL_IND,         82,     2,      6,      101     },
    {   BARD_LEVEL_IND,         66,     2,      5,      101     },
    {   BARD_LEVEL_IND,         50,     2,      4,      101     },
    {   BARD_LEVEL_IND,         32,     2,      3,      101     },
    {   BARD_LEVEL_IND,         16,     1,      2,      101     },

    {	THIEF_LEVEL_IND,	125,	5,	9,	101 	},
    {	THIEF_LEVEL_IND,	116,	4,	8,	101 	},
    {	THIEF_LEVEL_IND,	100,	3,	7,	101 	},
    {	THIEF_LEVEL_IND,	82,	2,	6,	101 	},
    {	THIEF_LEVEL_IND,	66,	2,	5,	101 	},
    {	THIEF_LEVEL_IND,	50,	2,	4,	101 	},
    {	THIEF_LEVEL_IND,	32,	2,	3,	101 	},
    {	THIEF_LEVEL_IND,	16,	1,	2,	101 	},

    {	CLERIC_LEVEL_IND,	125,	2,	6,	101 	},
    {	CLERIC_LEVEL_IND,	100,	2,	5,	101 	},
    {	CLERIC_LEVEL_IND,	75,	2,	4,	101 	},
    {	CLERIC_LEVEL_IND,	50,	2,	3,	101 	},
    {	CLERIC_LEVEL_IND,	25,	1,	2,	101 	},

    {	DRUID_LEVEL_IND,	125,	2,	6,	101 	},
    {	DRUID_LEVEL_IND,	100,	2,	5,	101 	},
    {	DRUID_LEVEL_IND,	75,	2,	4,	101 	},
    {	DRUID_LEVEL_IND,	50,	2,	3,	101 	},
    {	DRUID_LEVEL_IND,	25,	1,	2,	101 	},

    {	MAGE_LEVEL_IND, 	125,	2,	6,	101	},
    {	MAGE_LEVEL_IND, 	100,	2,	5,	101	},
    {	MAGE_LEVEL_IND, 	75,	2,	4,	101	},
    {	MAGE_LEVEL_IND, 	50,	2,	3,	101	},
    {	MAGE_LEVEL_IND, 	25,	1,	2,	101	},

    {   SHIFTER_LEVEL_IND,	125,     2,      6,      101	},
    {   SHIFTER_LEVEL_IND,	100,     2,      5,      101	},
    {   SHIFTER_LEVEL_IND,	75,     2,      4,      101	},
    {   SHIFTER_LEVEL_IND,	50,     2,      3,      101	},
    {   SHIFTER_LEVEL_IND,	25,     1,      2,      101	},


    {	PSI_LEVEL_IND,		125,	2,	6,	101 	},
    {	PSI_LEVEL_IND,		100,	2,	5,	101 	},
    {	PSI_LEVEL_IND,		75,	2,	4,	101 	},
    {	PSI_LEVEL_IND,		50,	2,	3,	101 	},
    {	PSI_LEVEL_IND,		25,	1,	2,	101 	},

    {	-1 }
};
#endif

int get_attack_count(struct char_data* ch)
{
  int chance;
  sh_int bonus_attks, attks=1;

  bonus_attks=ch->specials.bonus_attks;
  ch->specials.bonus_attks=0;

  if(!IS_PC(ch))
      return MAX(1, ch->mult_att) + bonus_attks;
  if(IS_NPC(ch)) // can only get here if they are a poly/shift/tree
      attks= ch->mult_att;

/* commented out to fix warnings in fight.c

#ifdef 0
  struct attack_counts *ptr;

  low = high = 1;
  degen = 0;

  for(ptr = attack_counts ; ptr->class != -1 ; ptr++)
  {
    if(HasClass(ch, 1 << ptr->class) &&
       (GET_LEVEL(ch, ptr->class) >= ptr->level))
    {
      if(ptr->high > high)
	high = ptr->high;
      if(ptr->low > low)
	low = ptr->low;
      if(ptr->degen > degen)
	degen = ptr->degen;
    }
  }
#endif
*/
  if (affected_by_spell(ch, SKILL_LIMB))
  {
    if (number(1,101)<ch->skills[SKILL_LIMB].learned)
      bonus_attks++;
    if ((GET_LEVEL(ch, SHIFTER_LEVEL_IND) > 49) && (number(1,101) < (ch->skills[SKILL_LIMB].learned)/2))
      bonus_attks++;
    if ((GET_LEVEL(ch, SHIFTER_LEVEL_IND) > 99) && (number(1,101) < (ch->skills[SKILL_LIMB].learned)/3))
      bonus_attks++;
 }
/*
#ifdef 0
  if(degen > 100) */		/* special case if we don't need to iterate */
/*    return high + bonus_attks;

  while(low < high)
  {
    if(number(1,101) >= degen)
      break;
    low++;
  }

 return low + bonus_attks;
#endif
*/
  if (ch->skills && (!IS_SET(ch->specials.mob_act, ACT_POLYSELF)))
  {
    chance= (ch->skills[SKILL_MELEE1].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE2].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE3].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE4].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE5].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE6].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE7].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE8].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE9].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE10].learned);
    if(number(1,101) < chance) attks++;

    chance= (ch->skills[SKILL_MELEE11].learned);
    if(number(1,101) < chance) attks++;
}

  if (IS_AFFECTED2(ch, AFF2_HASTE))
      attks += MAX(1, MIN(2, attks >> 1));

   attks += bonus_attks;

	/* Xaps addition for slow */

   if (IS_AFFECTED2(ch,AFF2_SLOW))  {
      if (attks % 2)  {
	 attks /= 2;
	 attks += 1;
      }
      else
      	attks /=2;
   }

  return attks;
}

// Used in perform violence.
void pv_binding(struct char_data *ch)
// PRE:  ch is defined and ch is binding
// POST: If ch is in no state to be binding,
//       then stop binding, else leave alone.
{
    if (!AWAKE(ch) ||
	ch->in_room!=ch->specials.binding->in_room ||
	IS_AFFECTED(ch, AFF_PARALYSIS) ||
	ch->specials.fighting!=ch->specials.binding ||
	ch->specials.binding->specials.binded_by!=ch)
    {
	ch->specials.binding->specials.binded_by=0;
	ch->specials.binding=0;
    }
}

// used in perform violence
void pv_binded_by(struct char_data *ch)
//PRE:  ch is defined and ch is being bound
{
    if (ch->in_room==ch->specials.binded_by->in_room &&
	ch->specials.fighting==ch->specials.binded_by &&
	ch->specials.binded_by->specials.binding==ch)
    {
	if (AWAKE(ch) && !IS_AFFECTED(ch, AFF_PARALYSIS))
	{
	    if (NewSkillSave(ch->specials.binded_by,ch,SKILL_BIND,0,IMM_HOLD))
	    {
		act("You are thrown to the ground as $N struggles free!",
		    FALSE,ch->specials.binded_by,0,ch,TO_CHAR);
		act("$n is thrown to the ground as you struggle free!",
		    FALSE,ch->specials.binded_by,0,ch,TO_VICT);
		act("$n is thrown to the ground as $N struggles free!",
		    TRUE,ch->specials.binded_by,0,ch,TO_ROOM);
		GET_POS(ch->specials.binded_by)=POSITION_SITTING;
		ch->specials.binded_by->specials.binding=0;
		ch->specials.binded_by=0;
	    }
	    else
	    {
	      act("$N thrashes violently to free $Mself from you...",
		  FALSE,ch->specials.binded_by,0,ch,TO_CHAR);
	      act("You thrash violently to free yourself from $n...",
		  FALSE,ch->specials.binded_by,0,ch,TO_VICT);
	      act("$N thrashes violently to free $Mself from $n...",
		  TRUE,ch->specials.binded_by,0,ch,TO_ROOM);
	      damage(ch, ch->specials.binded_by,
		       dice(3, GetMaxLevel(ch)), TYPE_UNDEFINED);
	    }
	}
        else
        {
            act("$n is completely immobilized...",TRUE,ch,0,0,TO_ROOM);
        }
    }
    else
    {
	ch->specials.binded_by->specials.binding=0;
	ch->specials.binded_by=0;
    }
}

// Used in perform violence
void pv_is_fighting(struct char_data *ch)
//PRE:  Ch is defined
{
    char buf[256];
    int i;
    struct char_data *vict;

    /* A few checks first */

    STATE0("pv_is_fighting - start");
    if(!(ch))
    {
       sprintf(buf, "entered perform violence with ch undefined, at pv_is_fighting\n");
       log_msg(buf);
       return;
    }
    if(!(ch->specials.fighting->in_room))
    {
       slog("Oh shit, this is bad.\n");
    }

    /* Then we make sure we hate the opponent */

    if (!IS_PC(ch)) {
   	if (ch->specials.fighting) {
	   DevelopHatred(ch, ch->specials.fighting);
	}
	else {
	   sprintf(buf, "ch->specials.fighting not defined at %d\n", __LINE__);
	   log_msg(buf);
	}
    }

    /* Then we fight as well as we are able to */

    if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room) &&
	(!IS_AFFECTED(ch, AFF_PARALYSIS)) && (ch->specials.position != POSITION_STUNNED)) /* End Buggy Code */
    {
	int offhand = 0, offhand_attks=0;

	/* Get number of normal attacks */
	i=get_attack_count(ch);
	/* If dual wielding, calculate skil and # offhand_attks */
	if( IS_PC(ch) &&
	    ch->skills && ch->skills[SKILL_DUAL_WIELD].learned &&
	    (!ch->equipment[WEAR_SHIELD]) &&
	    /* (!ch->equipment[WEAR_LIGHT]) &&  */
	    ((!ch->equipment[HOLD]) ||
	     (ch->equipment[HOLD] &&
	      (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON))))
	  {
	    /* if number attacks 3 or more, then 2 dual wield attks */
            if (IS_PURE_CLASS(ch))
              offhand_attks = i/2;
	    else
	      offhand_attks = (i>2) ? 2 : 1;

	    offhand = (GET_DEX(ch) - 12) * 3 +
	      ch->skills[SKILL_DUAL_WIELD].learned;
	    offhand = MAX(0, MIN(offhand, 95));
	    /* if #  normal attacks 1 or 3 then dual wield skill / 2 */
	    switch(i)
	      {
	      case 1:
	      case 3:
		offhand/=2;
		break;
	      case 2:
	      case 4:
	      case 5:
		break;
	      default:
		break;
	      }

	    /* If he's using his hands, chances are halved */
	    if(!ch->equipment[HOLD])
	      offhand /= 2;
	  }
	else
	  offhand_attks= 0;

	/* do melee fighting */
	for( ; (i > 0) ; --i)
	  {
	    if((vict = ch->specials.fighting) == NULL)
	      {		/* target has died, find another */
		if(((vict = FindAHatee(ch)) != NULL) &&
		   (vict->attackers < 6))
		  hit(ch, vict, TYPE_UNDEFINED);
		else if(((vict = FindAnAttacker(ch)) != NULL) &&
			(vict->attackers < 6))
		  hit(ch, vict, TYPE_UNDEFINED);
		else
		  break;
	      }
	    else
	      hit(ch, vict, TYPE_UNDEFINED);

	    if(offhand_attks &&
	       ((vict = ch->specials.fighting) != NULL))
	      {
		/* Decrement number of off hand attks */
		offhand_attks--;
		/* If number < offhand skill, then hit */
		if(number(1,100) < offhand)
		  hit(ch, vict, SKILL_DUAL_WIELD);
		else	/* If you miss one offhand, you loose an extra offhand */
		  offhand_attks = offhand_attks == 1 ? 0 : offhand_attks-1;
	      }
	  }
      }
    else
      {
	/* Not in same room or not awake */
	stop_fighting(ch);
      }

    STATE0("pv_is_fighting - middle");
    if (ch->specials.fighting)
      {

	if (!(ch->fight_delay)) {

	  /* NPC's actions in battle */

	  if (IS_SET(ch->specials.mob_act, ACT_SPEC) && !no_specials)
	    {
	      if (mob_index[ch->nr].func)
		{
		  STATE1("pv_is_fighting - mob func: %s", GET_NAME(ch));
		  (*mob_index[ch->nr].func)(ch, 0, 0, "", SPEC_FIGHT);
		}
	    }

	  STATE1("pv_is_fighting - mob mprogs: %s", GET_NAME(ch));

	  mprog_health_trigger(ch, ch->specials.fighting);
	  mprog_fight_trigger(ch, ch->specials.fighting);
	  mprog_fight_trigger2(ch, ch->specials.fighting);

	}

	oprog_fight_trigger(ch, ch->specials.fighting);
      }
    STATE0("pv_is_fighting - end");
}
//  Going to break down perform_violence to facilitate debuggin (Novak)
//  Actually -did- break down perform_violence for last_known_state

void perform_violence(int pulse)
{
    struct char_data *ch;

    STATE0("perform_violence - starting");
    for (ch = combat_list; ch; ch=combat_next_dude)
    {
	struct room_data *rp;

        STATE1("perform_violence - combat1: %s", GET_NAME(ch));
	combat_next_dude = ch->next_fighting;
        STATE1("perform_violence - combat2: %s", GET_NAME(ch));
	assert(ch->specials.fighting);
        STATE1("perform_violence - roomcheck: %s", GET_NAME(ch));
	rp = real_roomp(ch->in_room);
        STATE1("perform_violence - other checks: %s", GET_NAME(ch));
	if (rp && rp->room_flags&PEACEFUL)
	{
	    STATE1("perform_violence - peacecheck: %s", GET_NAME(ch));
	    char	buf[MAX_INPUT_LENGTH];
	    sprintf(buf,"perform_violence %s fighting in a PEACEFUL room.",
		    GET_IDENT(ch));
	    stop_fighting(ch);
	    /* log_msg(buf);*/ /* fighting in peaceful room */
	}
	else if (ch == ch->specials.fighting)
	{
	   STATE1("perform_violence - stop_fighting: %s", GET_NAME(ch));
		stop_fighting(ch);
	}
	else if (ch->specials.binding)
	{
	   STATE1("perform_violence - pv_binding: %s", GET_NAME(ch));
	   pv_binding(ch);
	}
	else if (ch->specials.binded_by)
	{
	   STATE1("perform_violence - pv_binded_by: %s", GET_NAME(ch));
	   pv_binded_by(ch);
	}
	else if (ch->specials.fighting)
	{
	   STATE1("perform_violence - pv_is_fighting: %s", GET_NAME(ch));
	   pv_is_fighting(ch);
	}
    }

    STATE0("perform_violence - check charging");
    for (; charging_list; charging_list=charging_next_dude)
    {
        STATE1("perform_violence - charging: %s", GET_NAME(charging_list));
	charging_next_dude = charging_list->next_charging;
	assert(charging_list->specials.charging);

	if (charging_list->specials.charging)
	{
	    STATE1("perform_violence - charging2: %s", GET_NAME(charging_list));
	    if (clearpath(charging_list, charging_list->in_room, charging_list->charge_dir))
		do_move(charging_list,"\0",charging_list->charge_dir+1);
	    if (charging_list->in_room == (charging_list->specials.charging)->in_room)
	    {
	        STATE1("perform_violence - charging3: %s", GET_NAME(charging_list));
		act("$n sees $N, and attacks!",
		    TRUE,charging_list,0,charging_list->specials.charging,TO_NOTVICT);
		act("$n sees you, and attacks!",
		    TRUE,charging_list,0,charging_list->specials.charging,TO_VICT);
		hit(charging_list,charging_list->specials.charging,TYPE_UNDEFINED);
	    } else {
		act("$n growls and starts searching for $N.",
		    TRUE,charging_list,0,charging_list->specials.charging,TO_ROOM);
		SetHunting(charging_list, charging_list->specials.charging);
	    }
	    charging_list->specials.charging=NULL;
	}
    }
    STATE0("perform_violence - done");
}

void BreakLifeSaverObj( struct char_data *ch)
{

    int found=-1, i, j;
    struct obj_data *o;
    char buff[200];
    char buf[200];
    int location;

    /*
     *  check eq for object with the effect
     */
    for (i = 0; i< MAX_WEAR; i++) {
	if (ch->equipment[i]) {
	    o = ch->equipment[i];
	    for (j=0; j<MAX_OBJ_AFFECT; j++) {
		if (o->affected[j].location == APPLY_SPELL) {
		    if (IS_SET(o->affected[j].modifier, AFF_LIFE_PROT)) {
		      found = i;
		    }
		}
	    }
	}
    }
    if (found != -1) {

	/*
	 *  break the fucker.
	 */

	act("$N disappears as $p shatters with a blinding flash of light!",
	    TRUE, ch, ch->equipment[found], ch, TO_NOTVICT);
	act("Your world shifts as $p shatters with a blinding flash of light!",
	    TRUE, ch, ch->equipment[found], ch, TO_CHAR);

	if ((o = unequip_char(ch, found)) != NULL) {
	    MakeScrap(ch, o, BLOW_DAMAGE);
	}

	/* now recall them */
	if (GET_HOME(ch))
	    location = GET_HOME(ch);
	else
	    location = 3001;

	if (!real_roomp(location))
	{
	    sprintf(buf, "Recalling %s to non-exist room %d\n\r",
		    GET_NAME(ch), location);
	    log_msg(buf);
	    location = 3001;
	}

	/* a location has been found. */

	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
	do_look(ch, "",15);
    }
    else
    {
	sprintf(buf, "%s has AFF_LIFE_PROT but no life saver", GET_NAME(ch));
	log_msg(buf);
    }
}

int BrittleCheck(struct char_data *ch, int dam)
{
    char buf[200];
    struct obj_data *obj;
    int pos=0;

    if (dam <= 0)
	return(FALSE);

    for(pos=WEAR_LIGHT;pos<=LOADED;pos++)
    if (ch->equipment[pos]) {
	if (IS_OBJ_STAT(ch->equipment[pos], ITEM_BRITTLE)) {
	    if ((obj = unequip_char(ch,pos))!=NULL) {
		sprintf(buf, "%s shatters.\n\r", OBJ_SHORT(obj));
		send_to_char(buf, ch);
		MakeScrap(ch, obj, BLOW_DAMAGE);
		return(TRUE);
	    }
	}
    }

    return FALSE;
}

int PreProcDam(struct char_data *ch, int type, int dam)
{

  unsigned Our_Bit;

  /* Check to see if you are underwater */
  int under_water = UNDERWATER(ch);

  /*
    long, intricate list, with the various bits and the various spells and
    such determined
    */

  switch (type) {
  case SPELL_FIRESHIELD:
  case SPELL_BURNING_HANDS:
  case SPELL_FIRE_WIND:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIREBALL:
  case SPELL_LAVA_STORM:
  case SPELL_FIRE_BREATH:
  case SKILL_COMBUSTION:
  case SKILL_FIRE_AURA:
    if (under_water)
      dam /= 2;
    Our_Bit = IMM_FIRE;
    break;

  case SPELL_ELECSHIELD:
  case SPELL_CALL_LIGHTNING:
  case SPELL_SHOCKING_GRASP:
  case SPELL_ELECTROCUTE:
  case SPELL_ELECTRIC_FIRE:
  case SPELL_CHAIN_ELECTROCUTION:
  case SPELL_LIGHTNING_BREATH:
  case SKILL_LIGHT_AURA:
    Our_Bit = IMM_ELEC;
    if (under_water)
      dam *= 2;
    break;

  case SPELL_COLDSHIELD:
  case SPELL_CHILL_TOUCH:
  case SPELL_FROST_CLOUD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
  case SKILL_ICE_AURA:
    Our_Bit = IMM_COLD;
    if (under_water)
      dam *= 2;
    break;

  case SPELL_ENERGYSHIELD:
  case SKILL_PSIONIC_BLAST:
  case SPELL_HARMFUL_TOUCH:
  case SPELL_WITHER:
  case SPELL_RUPTURE:
  case SPELL_IMPLODE:
  case SPELL_DISINTEGRATE:
  case SKILL_PULSE:
    Our_Bit = IMM_ENERGY;
    break;

  case SKILL_VAMPSHIELD:
  case SKILL_MANASHIELD:
  case SPELL_ENERGY_DRAIN:
  case SPELL_VAMPYRIC_TOUCH:
    Our_Bit = IMM_DRAIN;
    break;

  case SPELL_ACIDSHIELD:
  case SPELL_ACID_BLAST:
  case SPELL_ACID_RAIN:
  case SPELL_ACID_BREATH:
  case SKILL_ACID_AURA:
    Our_Bit = IMM_ACID;
    if (under_water)
      dam /= 2;
    break;

  case SKILL_BACKSTAB:
  case SPELL_THORN:
  case TYPE_PIERCE:
  case TYPE_STING:
  case TYPE_RANGEWEAPON:
  case TYPE_STAB:
    Our_Bit = IMM_PIERCE;
    break;

  case TYPE_SLASH:
  case TYPE_WHIP:
  case TYPE_CLEAVE:
  case TYPE_CLAW:
    Our_Bit = IMM_SLASH;
    break;

  case TYPE_BLUDGEON:
  case TYPE_HIT:
  case SKILL_KICK:
  case TYPE_CRUSH:
  case TYPE_BITE:
  case TYPE_SMASH:
  case TYPE_SMITE:
    Our_Bit = IMM_BLUNT;
    break;

  case SPELL_POISONSHIELD:
  case SPELL_POISON:
  case SPELL_POISON_GAS:
    Our_Bit = IMM_POISON;
    break;

  default:
    return(dam);
    break;
  }


  if (IsImmune(ch, Our_Bit))
  {
    dam = IMMUNE_DAMAGE(dam);
  } else if (IsResist(ch, Our_Bit))
  {
	 dam = RESIST_DAMAGE(dam);
  } else if (IsSusc(ch, Our_Bit))
  {
	 dam = SUSC_DAMAGE(dam);
  }
  return(dam);
}


int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
  int Immunity, total, j;

  Immunity = -1;
  if (IS_SET(v->M_immune, IMM_NONMAG)) {
    Immunity = 0;
  }
  if (IS_SET(v->M_immune, IMM_PLUS1)) {
    Immunity = 1;
  }
  if (IS_SET(v->M_immune, IMM_PLUS2)) {
    Immunity = 2;
  }
  if (IS_SET(v->M_immune, IMM_PLUS3)) {
    Immunity = 3;
  }
  if (IS_SET(v->M_immune, IMM_PLUS4)) {
    Immunity = 4;
  }

  if (Immunity < 0)
    return(dam);

  if ((type < TYPE_HIT) || (type > TYPE_RANGEWEAPON))  {
    return(dam);
  } else {
    if (type == TYPE_HIT || IS_NPC(ch)) {
      if (IS_NPC(ch) && (GetMaxLevel(ch) > (3*Immunity)+1)) {
	return(dam);
      } else {
	return(0);
      }
    } else {
      total = 0;
      if (!ch->equipment[WIELD])
	return(0);
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
	    (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) {
	  total += ch->equipment[WIELD]->affected[j].modifier;
	}
      if (total > Immunity) {
	return(dam);
      } else {
	return IMMUNE_DAMAGE(dam);
      }
    }
  }
}


int SkipImmortals(struct char_data *v, int amnt)
{
  /* You can't damage an immortal! */

  if (IS_IMMORTAL(v) && !IS_NPC(v))
    amnt = 0;

  /* special type of monster */
  if (IS_SET(v->specials.mob_act, ACT_IMMORTAL)) {
    amnt = -1;
  }
  return(amnt);

}


void WeaponSpell(struct char_data *c, struct char_data *v,
		 int type, struct obj_data* w)
{
    int j, num;

    if ((c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD)) {
	if(!w)
	    w = c->equipment[WIELD];
	if (w && ((type >= TYPE_BLUDGEON) && (type <= TYPE_RANGEWEAPON))) {
	    for(j=0; j<MAX_OBJ_AFFECT && w ; j++) {
	       /* This is a huge hack by Quilan. I had to do it, because
		* with items of multiple WS's, they would fire on a
		* FREE'd victim. This was NOT cool, so I had to make
		* this hack. Enjoy :). */
               if(c->player.has_killed_victim) break;

	       if(w->affected[j].location == APPLY_WEAPON_SPELL) {
		    struct spell_info* spell;

		    num = w->affected[j].modifier;
		    if(!(spell = spell_by_number(num)))
		    {
			char buf[256];
			sprintf(buf, "Illegal weapon spell: %d on %s",
				num, OBJ_NAME(w));
			log_msg(buf);
		    }
		    else if(!check_nomagic(c))
		    {
		      (*spell->spell_pointer)(GetMaxLevel(c), c, "", SPELL_TYPE_WAND, v, 0);
		    }
		}
	    }
	}
    }
}

void shoot( struct char_data *ch, struct char_data *victim)
{
  struct obj_data *bow, *arrow;
  int tohit=0, todam=0;

  /*
  **  check for bow and arrow.
  */

  bow = ch->equipment[HOLD];
  arrow = ch->equipment[WIELD];

  if (!bow) {
    send_to_char("You need a missile weapon (like a bow)\n\r", ch);
    return;
  } else if (!arrow) {
    send_to_char("You need a projectile to shoot!\n\r", ch);
  } else if (!bow && !arrow) {
    send_to_char("You need a bow-like item, and a projectile to shoot!\n\r",ch);
  } else {
    /*
    **  for bows:  value[0] = arror type
    **             value[1] = strength required
    **             value[2] = + to hit
    **             value[3] = + to damage
    */

    if (bow->obj_flags.value[0] != ObjVnum(arrow)) {
      send_to_char("That projectile does not fit in that projector.\n\r", ch);
      return;
    }
    /*
    **  check for str problem:  same as wield, but with bow.
    */
    if (bow->obj_flags.value[1] > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
      send_to_char("That bow is too heavy for you to use!\n\r", ch);
      return;
    }

    /*
    **  check for bonuses on the bow.
    */
    tohit = bow->obj_flags.value[2];
    todam = bow->obj_flags.value[3];

    /*
    **   temporarily add those bonuses.
    */
    GET_HITROLL(ch)+=tohit;
    GET_DAMROLL(ch)+=todam;

    /*
    **   fire the weapon.
    */
    MissileHit(ch, victim, TYPE_UNDEFINED);

    GET_HITROLL(ch)-=tohit;
    GET_DAMROLL(ch)-=todam;

  }
}

struct char_data *FindVictim(struct char_data *ch)
{
    struct char_data *tmp_ch, *vict=NULL;
    int ftot, ttot, ctot, mtot;
    int total=0, tmp_total;
    int fjump, cjump, mjump, tjump;

    if (ch->in_room<=NOWHERE)
      return (NULL);

    /* if we're already fighting somebody use them */
    if (ch->specials.fighting)
      return (ch->specials.fighting);

    if (IS_PC(ch) || !IS_SET(ch->specials.mob_act, ACT_AGGRESSIVE) ||
        IS_AFFECTED(ch, AFF_CHARM))
      return (NULL);

     /* choose best target based on int and multipliers */
/*    if (ch->abilities.intel <= 3) {
      fjump=6; cjump=1; tjump=4; mjump=1; }
    else if (ch->abilities.intel <= 9) {
      fjump=5; cjump=2; tjump=3; mjump=2; }
    else if (ch->abilities.intel <= 12) {
      fjump=3; cjump=3; tjump=3; mjump=3; }
    else if (ch->abilities.intel <= 15) {
      fjump=2; cjump=4; tjump=2; mjump=4; }
    else {
      fjump=1; cjump=5; tjump=1; mjump=5; }
*/

    /* PAC - The following is equivalent to the above block but a tad faster */
    switch(ch->abilities.intel)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      fjump=6;      cjump=1;      tjump=4;      mjump=1;      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      fjump=3;      cjump=2;      tjump=3;      mjump=2;      break;
    case 10:
    case 11:
    case 12:
      fjump=2;      cjump=3;      tjump=3;      mjump=3;      break;
    case 13:
    case 14:
    case 15:
      fjump=2;      cjump=4;      tjump=2;      mjump=4;      break;
    default:
      fjump=1;
      cjump=5;
      tjump=1;
      mjump=5;
      break;
    }

    for (tmp_ch=(real_roomp(ch->in_room))->people; tmp_ch;
	 tmp_ch=tmp_ch->next_in_room) {

      if (CAN_SEE(ch, tmp_ch) && tmp_ch!=ch &&
         ch->master!=tmp_ch && !in_group(ch, tmp_ch) &&
         !IS_SET(tmp_ch->specials.flags, PLR_NOHASSLE) &&
         !IS_AFFECTED(tmp_ch, AFF_SNEAK) &&
         !IS_SET(tmp_ch->specials.mob_act, ACT_LIQUID) &&
	  can_pkill(ch, tmp_ch)) {

        /* beat on the annoying guy first */
        if (IS_SET(tmp_ch->specials.mob_act, ACT_ANNOYING))
          return (tmp_ch);

        /* priority to those with agg set */
        if (IS_SET(tmp_ch->specials.flags, PLR_AGGR))
          return (tmp_ch);

        /* leave mobs/polys alone */
        if (!IS_NPC(tmp_ch)) {

          /* pick the easy fight */
//          if (IS_SET(ch->specials.mob_act, ACT_WIMPY) && !AWAKE(tmp_ch))
//            return (tmp_ch);

          ftot=ttot=ctot=mtot=0;

          /* set up vals for our victim to compare to others in the room */
          if (HasClass(tmp_ch, CLASS_WARRIOR) ||
	      HasClass(tmp_ch, CLASS_PALADIN))
            ftot=2;
          else if (HasClass(tmp_ch, CLASS_RANGER))
	    ftot=1;

          if (HasClass(tmp_ch, CLASS_CLERIC))
	    ctot=1;

          if (HasClass(tmp_ch,CLASS_MAGIC_USER) ||
              HasClass(tmp_ch,CLASS_PSI))
            mtot=2;
          else if (HasClass(tmp_ch,CLASS_SHIFTER) ||
	           HasClass(tmp_ch,CLASS_DRUID))
            mtot=1;

          if (HasClass(tmp_ch, CLASS_THIEF))
	    ttot=1;

          tmp_total=(fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(mjump*mtot);

          /* do we have a better candidate? */
          if (tmp_total>=total) {
            total=tmp_total;
            vict=tmp_ch;
          }
        }
      }
    }

    return (vict);
}

struct char_data *FindMetaVictim(struct char_data *ch)
{
    struct char_data *tmp_ch, *vict=NULL;
    int ftot, ttot, ctot, mtot;
    int total=0, tmp_total;
    int fjump, cjump, mjump, tjump;

    if (ch->in_room<=NOWHERE)
      return (NULL);

    /* if we're already fighting somebody use them */
    if (ch->specials.fighting)
      return (ch->specials.fighting);

    if (IS_PC(ch) || !IS_SET(ch->specials.mob_act, ACT_META_AGG) ||
        IS_AFFECTED(ch, AFF_CHARM))
      return (NULL);

    /* choose best target based on int and multipliers */
/*  if (ch->abilities.intel <= 3) {
      fjump=6; cjump=1; tjump=4; mjump=1; }
    else if (ch->abilities.intel <= 9) {
      fjump=5; cjump=2; tjump=3; mjump=2; }
    else if (ch->abilities.intel <= 12) {
      fjump=3; cjump=3; tjump=3; mjump=3; }
    else if (ch->abilities.intel <= 15) {
      fjump=2; cjump=4; tjump=2; mjump=4; }
    else {
      fjump=1; cjump=5; tjump=1; mjump=5; }
*/

    /* PAC - The following is equivalent to the above block but a tad faster */
    switch(ch->abilities.intel)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      fjump=6;      cjump=1;      tjump=4;      mjump=1;      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      fjump=3;      cjump=2;      tjump=3;      mjump=2;      break;
    case 10:
    case 11:
    case 12:
      fjump=2;      cjump=3;      tjump=3;      mjump=3;      break;
    case 13:
    case 14:
    case 15:
      fjump=2;      cjump=4;      tjump=2;      mjump=4;      break;
    default:
      fjump=1;
      cjump=5;
      tjump=1;
      mjump=5;
      break;
    }

    for (tmp_ch=(real_roomp(ch->in_room))->people; tmp_ch;
	 tmp_ch=tmp_ch->next_in_room) {

      if (CAN_SEE(ch, tmp_ch) && !SameRace(ch, tmp_ch) &&
         ch->master!=tmp_ch && !in_group(ch, tmp_ch) &&
         !IS_SET(tmp_ch->specials.flags, PLR_NOHASSLE) &&
         !IS_AFFECTED(tmp_ch, AFF_SNEAK) &&
         !IS_SET(tmp_ch->specials.mob_act, ACT_LIQUID)) {

        /* pick the easy fight */
        if (IS_SET(ch->specials.mob_act, ACT_WIMPY) && !AWAKE(tmp_ch))
          return (tmp_ch);

        /* priority to those with aggressive set */
        if (IS_SET(tmp_ch->specials.flags, PLR_AGGR))
          return(tmp_ch);

        ftot=ttot=ctot=mtot=0;

        /* set up vals for our victim to compare to others in the room */
        if (HasClass(tmp_ch, CLASS_WARRIOR) ||
	    HasClass(tmp_ch, CLASS_PALADIN))
          ftot=2;
        else if (HasClass(tmp_ch, CLASS_RANGER))
	  ftot=1;

	if (HasClass(tmp_ch, CLASS_CLERIC))
	  ctot=1;

        if (HasClass(tmp_ch,CLASS_MAGIC_USER) ||
            HasClass(tmp_ch,CLASS_PSI))
          mtot=2;
        else if (HasClass(tmp_ch,CLASS_SHIFTER) ||
	         HasClass(tmp_ch,CLASS_DRUID))
	  mtot=1;
        tmp_total=(fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(mjump*mtot);

        /* do we have a better candidate? */
        if (tmp_total>=total) {
          total=tmp_total;
          vict=tmp_ch;
        }
      }
    }

    return (vict);
}

struct char_data *FindAnAttacker(struct char_data *ch)
{
    struct char_data *tmp_ch, *vict=NULL;
    int ftot, ttot, ctot, mtot;
    int total=0, tmp_total;
    int fjump, cjump, mjump, tjump=0;

    if (ch->in_room<=NOWHERE)
      return (NULL);

    /* if we're already fighting somebody use them */
    if (ch->specials.fighting)
      return (ch->specials.fighting);

    /* choose best target based on int and multipliers */
    if (ch->abilities.intel <= 3) {
      fjump=6; cjump=1; tjump=4; mjump=1; }
    else if (ch->abilities.intel <= 9) {
      fjump=5; cjump=2; tjump=3; mjump=2; }
    else if (ch->abilities.intel <= 12) {
      fjump=3; cjump=3; tjump=3; mjump=3; }
    else if (ch->abilities.intel <= 15) {
      fjump=2; cjump=4; tjump=2; mjump=4; }
    else {
      fjump=1; cjump=5; tjump=1; mjump=5; }

    /* loop through those in the room */
    for (tmp_ch=(real_roomp(ch->in_room))->people; tmp_ch;
	 tmp_ch=tmp_ch->next_in_room) {

      if (tmp_ch->specials.fighting==ch && tmp_ch!=ch) {

        /* priority to those with aggressive set */
        if (IS_SET(tmp_ch->specials.flags, PLR_AGGR))
          return(tmp_ch);

        ftot=ttot=ctot=mtot=0;

        /* set up vals for our victim to compare to others in the room */
        vict=tmp_ch;
        if (HasClass(tmp_ch, CLASS_WARRIOR) ||
	    HasClass(tmp_ch, CLASS_PALADIN))
          ftot=2;
        else if (HasClass(tmp_ch, CLASS_RANGER))
	  ftot=1;

	if (HasClass(tmp_ch, CLASS_CLERIC))
	  ctot=1;

        if (HasClass(tmp_ch,CLASS_MAGIC_USER) ||
            HasClass(tmp_ch,CLASS_PSI))
          mtot=2;
        else if (HasClass(tmp_ch,CLASS_SHIFTER) ||
	         HasClass(tmp_ch,CLASS_DRUID))
	  mtot=1;

	if (HasClass(tmp_ch, CLASS_THIEF))
	  ttot=1;

        tmp_total=(fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(mjump*mtot);

        /* do we have a better candidate? */
        if (tmp_total>=total) {
          total=tmp_total;
          vict=tmp_ch;
        }
      }
    }

    return(vict);
}

