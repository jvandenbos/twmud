#include "config.h"
#include <stdio.h>
#include <stdlib.h> /* for rand functions */

#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "race.h"
#include "db.h"
#include "opinion.h"
#include "comm.h"
#include "hash.h"
#include "area.h"
#include "state.h"
#include "multiclass.h"
#include "utility.h"
#include "handler.h"
#include "fight.h"
#include "constants.h"
#include "act.h"
#include "trap.h"
#include "mobact.h"
#include "sound.h"
#include "magicutils.h"
#include "recept.h"
#include "spell_procs.h"
#include "spelltab.h"
#include "channels.h"
#include "util_str.h"
#include "track.h"
#include "vnum_mob.h"
#include "find.h"
#include "spell_util.h"
#include "hash.h"
#include "varfunc.h"

#include "hero.h"
#include "proto.h"
#include "char_create.h"
#include "interpreter.h"

// turn on group level restrictions

#define GROUP_LEVEL_RESTRICTS




/* the big buffer */

char buf[256];

/* forward declarations */
int RecCompObjNum(struct obj_data* o, int obj_num);

/*
inline int IS_DARK(int room)
{
  struct room_data *r=real_roomp(room);

  return ((r->light<=0) && ((IS_SET(r->room_flags, DARK)) || (r->dark)));
}


inline int IS_LIGHT(int room)
{
  struct room_data *r=real_roomp(room);

  return (r->light>0 || (!IS_SET(r->room_flags, DARK) || !r->dark));
}
*/

int GetItemClassRestrictions(struct obj_data *obj)
{
    int total=0;

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
	total += CLASS_MAGIC_USER;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
	total += CLASS_THIEF;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
	total += CLASS_WARRIOR;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
	total += CLASS_CLERIC;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
	total += CLASS_PALADIN;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
	total += CLASS_DRUID;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI)) {
	total += CLASS_PSI;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
	total += CLASS_RANGER;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_SHIFTER)) {
        total += CLASS_SHIFTER;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MONK)) {
      total += CLASS_MONK;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARD)) {
      total += CLASS_BARD;
    }
    return(total);

}

/* Note - This is the inverse function of the above because the setting of the
 * ITEM_PURE_CLASS flag now indicates to REVERSE the meanings of the
 * ANTI_CLASS flags -- Min
 */

ush_int PureItemClass(struct obj_data *obj) {

  long flag;

  flag = obj->obj_flags.extra_flags; /* saves typing later */

  if (IS_SET(flag, ITEM_ANTI_CLERIC))
    return CLASS_CLERIC;
  if (IS_SET(flag, ITEM_ANTI_MAGE))
    return CLASS_MAGIC_USER;
  if (IS_SET(flag, ITEM_ANTI_THIEF))
    return CLASS_THIEF;
  if (IS_SET(flag, ITEM_ANTI_FIGHTER))
    return CLASS_WARRIOR;
  if (IS_SET(flag, ITEM_ANTI_PALADIN))
    return CLASS_PALADIN;
  if (IS_SET(flag, ITEM_ANTI_DRUID))
    return CLASS_DRUID;
  if (IS_SET(flag, ITEM_ANTI_PSI))
    return CLASS_PSI;
  if (IS_SET(flag, ITEM_ANTI_RANGER))
    return CLASS_RANGER;
  if (IS_SET(flag, ITEM_ANTI_BARD))
    return CLASS_BARD;
  if (IS_SET(flag, ITEM_ANTI_MONK))
    return CLASS_MONK;
  if (IS_SET(flag, ITEM_ANTI_SHIFTER))
      return CLASS_SHIFTER;
  return 0;
}

int IsLevelOk(struct char_data* ch, struct obj_data* obj)
{
    if(GetMaxLevel(ch) < obj->obj_flags.level && !IS_IMMORTAL(ch))
    {
      act("You feel far too puny to use $p.", FALSE, ch, obj, 0, TO_CHAR);
      return 0;
    }

    return 1;
}

int CountFollowers(struct char_data *ch)
{
    int count=0;
    int checknum=0;
    struct follow_type *k;

    for(k=ch->followers;k;k=k->next) {
	count++;
    }

    if(ch->in_room==500)
	checknum=9;
    else
	checknum=49;

    if(count>checknum) {
	send_to_char("You can not possibly have any more followers than you do now!\n\r", ch);
	return(FALSE);
    } else return(TRUE);

}

struct descriptor_data * getdescriptor(struct char_data *ch)
/* also by alex: returns descriptor_data of player or -1 */
{
    struct descriptor_data *d;

    if (ch == NULL) return (NULL);
    if(!ch->desc) return(NULL);
    if (!IS_PC(ch)) return(NULL);

    EACH_DESCRIPTOR(d_iter, d)
    {
        if (d->character && GET_REAL_NAME(d->character))
            if(strcmp(GET_REAL_NAME(ch), GET_REAL_NAME(d->character))==0)
               return(d);

    }
    END_ITER(d_iter);

    return(NULL);
}
/* maarek */

void LearnFromMistake(struct char_data *ch, int sknum, int silent, int max)
{
    if (!ch->skills) return;

    if (ch->skills[sknum].learned < max && ch->skills[sknum].learned > 0) {
	if (ch->skills[sknum].learned < 90 &&
	    (number(1, 101) > ch->skills[sknum].learned/2)) {
	    if(!silent)
		send_to_char("You learn from your mistake.\n\r", ch);
	    ch->skills[sknum].learned += 2;
	    if (ch->skills[sknum].learned >= max)
	    {
		ch->skills[sknum].learned = 90;
		if(!silent)
		    send_to_char("You are now learned in this skill!\n\r", ch);
	    }
	}
    }
}

// Rewritten 2005 by raist
// Takes a set of plants to grow and a chance that they grow
// Collects a list of plants that should grow, and scatter them across
// all rooms of sector type: field, forest, hill, mountain or desert
void GrowPlants(int nothing)
{
  int MAX_PLANTS = 100;

  struct obj_data *obj;
  struct obj_data obj_list[MAX_PLANTS];
  char buf[150];
  struct room_data *roomp, *targ_roomp;
  int room, targ_room;

  int mod = 2;

  static struct
  {
    int reagent;
    int chance;
  } *plant, plants[] = 
    {
    //reagent  change (1/X)
      { 8716, 14*mod}, // vampshield    nightshade
      { 8717, 6*mod},  // sight          midnight eye
      { 8718, 6*mod},  // healing        ginseng
      { 8719, 5*mod},  // mana           goosefoot
      { 8720, 10*mod}, // energyshield  cockscomb
      { 8721, 6*mod},  // protection     hellebore
      { 8722, 4*mod},  // fly            mistletoe
      { 8723, 11*mod}, // gprotect      stoneweeb
      { 8724, 9*mod},  // paladin bless  reed straw
      { 8725, 11*mod}, // poisonshield  black lotus
      { 8726, 12*mod}, // manashield    crimson weeb
      { 8727, 12*mod}, // moveshield    camphor
      { 8728, 8*mod},  // gfly           silverrose
      { 8729, 5*mod},  // sanctuary      juniper
      { 8730, 9*mod},  // gsight         jewel weed
      { 8731, 7*mod},  // fireshield     fire weed
      { 8732, 7*mod},  // heroes feast   dogwood plant
      { 8733, 5*mod},  // invisibility   ghost orchid
      { 8734, 4*mod},  // waterbreath    dwarf clover
      { 8735, 8*mod},  // coldshield     ice plant
      { 8736, 8*mod},  // ginvisibility  swamp lily
      { 8737, 6*mod},  // gwaterbreath   blue coral
      { 8738, 9*mod},  // elecshield     venus flytrap
      { 8739, 13*mod}, // firebreath     
      { 8740, 13*mod}, // frostbreath     
      { 8741, 13*mod}, // acidbreath  
      { 8742, 13*mod}, // poisongasbreath   
      { 8743, 13*mod}, // lightningbreath
      { 8751, 8*mod}, // broken pine branch
      { 8752, 8*mod}, // broken oak branch
      { 8753, 8*mod}, // severed cherry wood
      { 8754, 8*mod}, // bamboo stock
      { 0, 0}
    };

  int i = 0;
  // Create list of plants to scatter
  for(plant = plants ; plant->reagent ; plant++, i++)
    {
      if(i > MAX_PLANTS) continue;
      
      if(number(1, plant->chance) == 1)
	{
	  if(!(obj = make_object(plant->reagent, VIRTUAL)))
	    {
	      sprintf(buf, "trying to grow non-existent plant: %d",
		      plant->reagent);
	      log_msg(buf);
	      continue;
	    }

	  // Ensure object is configured correctly before placing it
	  obj->eq_pos = -1;
	  obj->equipped_by = 0;
	  obj->in_room = NOWHERE;
	  
	  do {  /* Find target room */
	    targ_room = randomnum(top_of_world);
	    targ_roomp = real_roomp(targ_room);
	  } while ( !targ_room || !targ_roomp ||
		    IS_SET(targ_roomp->room_flags, (DEATH |
						    IMMORT_RM | GOD_RM
						    |  BRUJAH_RM)) ||
		    targ_roomp->sector_type < SECT_FIELD || targ_roomp->sector_type > SECT_MOUNTAIN );
	  sprintf(buf, "You notice %s is in bloom in this area\r\n", OBJ_SHORT(obj));
	  obj_to_room(obj, targ_room);
	  send_to_room(buf, targ_room);
	  sprintf(buf, "%s was just grown in room number %d", OBJ_SHORT(obj), targ_roomp->number );
	  log_msg(buf);
	}
    }
}


int CheckMeditating(struct char_data *s)
{
    struct room_data *rp;
    int psilev=0;

    if(!IS_AFFECTED(s, AFF_MEDITATE))
	return(0);
    if((GET_POS(s)!=POSITION_RESTING) && (GET_POS(s)!=POSITION_SITTING))
	return(0);
    rp=real_roomp(s->in_room);
    if(!rp) return(0);
    psilev=GetMaxLevel(s);
    if(rp->sector_type==SECT_FIELD || rp->sector_type==SECT_FOREST ||
       rp->sector_type==SECT_HILLS || rp->sector_type==SECT_MOUNTAIN) {
	if((s->skills[SKILL_MEDITATE].learned > number(1,101)) &&
	   (psilev>0)) {
	    if(number(1,2)==2)
		return(psilev/4);
	    else
		return(psilev/7);
	}
	else
	    return(0);
    }
    else
	return(0);

}


int MountEgoCheck(struct char_data *ch, struct char_data *horse)
{
    int ride_ego, drag_ego, align, check;

    if (GET_RACE(horse) == RACE_DRAGON) {
	if (ch->skills) {
	    drag_ego = GetMaxLevel(horse)*2;
	    if (IS_SET(horse->specials.mob_act, ACT_AGGRESSIVE) ||
		IS_SET(horse->specials.mob_act, ACT_META_AGG)) {
		drag_ego += GetMaxLevel(horse);
	    }
	    ride_ego = ch->skills[SKILL_RIDE].learned/10 +
		GetMaxLevel(ch)/2;
	    ride_ego += ((GET_INT(ch) + GET_WIS(ch))/2);
	    align = GET_ALIGNMENT(ch) - GET_ALIGNMENT(horse);
	    if (align < 0) align = -align;
	    align/=100;
	    align -= 5;
	    drag_ego += align;
	    if (GET_HIT(horse) > 0)
		drag_ego -= GET_MAX_HIT(horse)/GET_HIT(horse);
	    else
		drag_ego = 0;
	    if (GET_HIT(ch) > 0)
		ride_ego -= GET_MAX_HIT(ch)/GET_HIT(ch);
	    else
		ride_ego = 0;

	    check = drag_ego+number(1,10)-(ride_ego+number(1,10));
	    return(check);

	} else {
	    return(-GetMaxLevel(horse));
	}
    } else {
	if (!ch->skills) return(-GetMaxLevel(horse));

	drag_ego = GetMaxLevel(horse);

	if (drag_ego > 15)
	    drag_ego *= 2;

	ride_ego = ch->skills[SKILL_RIDE].learned/10 +
	    GetMaxLevel(ch);

	ride_ego += (GET_INT(ch) + GET_WIS(ch));
	check = drag_ego+number(1,5)-(ride_ego+number(1,10));
	return(check);
    }
}

int RideCheck( struct char_data *ch, int mod)
{
    if (ch->skills) {
	if (number(1, 101) > ch->skills[SKILL_RIDE].learned+mod) {
	    LearnFromMistake(ch, SKILL_RIDE, 0, 90);
	    return(FALSE);
	}
	return(TRUE);
    } else {
	return(FALSE);
    }
}
void FallOffMount(struct char_data *ch, struct char_data *h)
{
    act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
    act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
    act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);
}


int CheckColor(struct char_data *s)
{
    return IS_SET(s->specials.flags, PLR_COLOR);
}

int CAN_SEE_OBJ(struct char_data *ch, struct obj_data *obj)
{
  if (IS_IMMORTAL(ch))
    return (TRUE);

  if (IS_AFFECTED(ch, AFF_BLIND))
    return (FALSE);

  if (IS_AFFECTED(ch, AFF_GREAT_SIGHT))
    return (TRUE);

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) &&
      !IS_AFFECTED(ch, AFF_DETECT_INVISIBLE))
    return (FALSE);

  if (!IS_LIGHT(real_roomp(ch->in_room)))
    return (FALSE);

  return (TRUE);
}



int CAN_SEE(struct char_data *s, struct char_data *o)
{

  if(HAS_GCMD(s,GCMD_HIGH))
    return(TRUE);

  if (o->invis_level > TRUST(s) &&  (s->specials.see_invis_level < o->invis_level)) /* multiple */
    return FALSE;		/* levels of invis */

  if (IS_IMMORTAL(s))
    return(TRUE);


  if (!o || s->in_room <= NOWHERE || o->in_room <= NOWHERE)
    return(FALSE);

  if(IS_SET(o->specials.flags, PLR_MASK))
    return(FALSE);

  if (IS_SET(AFF_FLAGS(o) | AFF_FLAGS(s),
	     AFF_BLIND | AFF_GREAT_SIGHT | AFF_HIDE |
	     AFF_TRUE_SIGHT | AFF_INVISIBLE ))
  {
     if (IS_AFFECTED(s, AFF_BLIND))
      return (FALSE);

     if (IS_AFFECTED(o, AFF_HIDE) &&
         IS_PURE_CLASS(o) && HasClass(o, CLASS_THIEF))
	if(IS_AFFECTED(s, AFF_GREAT_SIGHT) &&
           IS_PURE_CLASS(s) && HasClass(s, CLASS_PSI))
		return (TRUE);
	else
		return (FALSE);

    if (IS_AFFECTED(s, AFF_GREAT_SIGHT))
      return(TRUE);

    if (IS_AFFECTED(o, AFF_HIDE))
      return(FALSE);

    if (IS_AFFECTED(s, AFF_TRUE_SIGHT))
      return(TRUE);

    if (IS_AFFECTED(o, AFF_INVISIBLE))
      if(!IS_AFFECTED(s, AFF_DETECT_INVISIBLE) ||
	  (IS_AFFECTED(s, AFF_DETECT_INVISIBLE) &&
	  (GetMaxLevel(s) < GetMaxLevel(o) - 30)))
          return(FALSE);
   }

  if (((IS_DARK(real_roomp(s->in_room))) || (IS_DARK(real_roomp(o->in_room)))) &&
      (!IS_AFFECTED(s, AFF_INFRAVISION)))
    return(FALSE);

  return(TRUE);
}


int exit_ok(struct room_direction_data	*exit, struct room_data **rpp)
{
  struct room_data* rp = NULL;

  if (exit)
    rp = real_roomp(exit->to_room);

  if(rpp)
    *rpp = rp;

  return rp != NULL;
}

int WeaponImmune(struct char_data *ch)
{
    if (IS_SET(IMM_NONMAG, ch->M_immune) ||
	IS_SET(IMM_PLUS1, ch->M_immune) ||
	IS_SET(IMM_PLUS2, ch->M_immune) ||
	IS_SET(IMM_PLUS3, ch->M_immune) ||
	IS_SET(IMM_PLUS4, ch->M_immune))
	return(TRUE);
    return(FALSE);

}

int IsImmune(struct char_data *ch, int bit)
{
    return(IS_SET(bit, ch->M_immune));
}

int IsResist(struct char_data *ch, int bit)
{
    return(IS_SET(bit, ch->immune));
}

int IsSusc(struct char_data *ch, int bit)
{
    return(IS_SET(bit, ch->susc));
}

void WriteToImmort(const char *str, int level, int loglevel)
{
    static char buf[500];
    struct descriptor_data *i;

    if (str)
	sprintf(buf,"[%s]\n\r",str);
    EACH_DESCRIPTOR(d_iter, i)
    {
	if ((!i->connected) && (HAS_GCMD(i->character,GCMD_GEN)) &&
	    (TRUST(i->character) >= level) &&
	    (!IS_SET(i->character->channels, COM_LOG)) &&
	    (!IS_WRITING(i->character)) &&
	    (i->character->log_flags & loglevel))
	    write_to_q(buf, &i->output);
    }
    END_ITER(d_iter);
}

// writes a string to a logfile
void file_log(const char* str, const char* filename) {

  FILE* fp;
  char buff[255];

  sprintf(buff, "../logs/%s", filename);
  if ((fp = fopen(buff,"a+")) == NULL) {
    sprintf(buff, "Error in fil_log! Couldn't open filepointer to %s", filename);
    log_msg(buff);
    return;
  }

  sprintf(buff, "%s\n", str);
  fprintf(fp, buff);

  fclose(fp);
}

/* writes a string to the log */
void log_msg(const char *str, int level)
{
    slog(str);
    WriteToImmort(str, 0, level);
}

/* writes a string to the log */
void implog(const char *str, int level=LOG_ERROR)
{
    slog(str);
    WriteToImmort(str, TRUST_IMP, level);
}

/* writes a string to the logchannel but not to log file */
void nolog(const char *str, int level)
{
    WriteToImmort(str, TRUST_GRUNT, level);
}

void slog(const char *str)
{
    long ct;
    char *tmstr;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    fprintf(stderr, "%s :: %s\n", tmstr, str);

}

void dlog(const char *format, ...)
{
  char debug_buf[MAX_STRING_LENGTH];
  va_list args;
  va_start(args, format);
  vsprintf (debug_buf, format, args);

  log_msg(debug_buf);
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR*now.hours;

    now.day = (secs/SECS_PER_REAL_DAY);	/* 0..34 days  */
    secs -= SECS_PER_REAL_DAY*now.day;

    now.month = -1;
    now.year  = -1;

    return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_MUD_HOUR) % 24; /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR*now.hours;

    now.day = (secs/SECS_PER_MUD_DAY) % 30; /* 0..29 days  */
    secs -= SECS_PER_MUD_DAY*now.day;

    now.month = (secs/SECS_PER_MUD_MONTH) % 12; /* 0..11 months */
    secs -= SECS_PER_MUD_MONTH*now.month;

    now.year = (secs/SECS_PER_MUD_YEAR); /* 0..XX? years */

    return now;
}

struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0),ch->player.time.birth);

    return(player_age);
}

int in_group ( struct char_data *ch1, struct char_data *ch2)
{
    /*
      three possibilities ->
      1.  char is char2's master
      2.  char2 is char's master
      3.  char and char2 follow same.


      otherwise not true.

      */
    if (ch1 == ch2)
	return(1);

    if ((!ch1) || (!ch2))
	return(0);

    if ((!ch1->master) && (!ch2->master))
	return(0);

    if (!IS_AFFECTED(ch1, AFF_GROUP) || !IS_AFFECTED(ch2, AFF_GROUP))
        return(0);

    if (ch2->master)
/* PAC These string compares aren't needed,
   pointer comparisons are much faster */
/*	if (!strcmp(GET_NAME(ch1),GET_NAME(ch2->master)))*/
      if (ch1 == ch2->master) {
	    return(1);
	}

    if (ch1->master)
/*	if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2)))*/
      if (ch1->master == ch2) {
	    return(1);
	}

    if ((ch2->master) && (ch1->master))
/*	if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2->master)))*/
      if (ch1->master == ch2->master) {
	    return(1);
	}

    if (MOUNTED(ch1) == ch2 || RIDDEN(ch1) == ch2)
	return(1);

    return(0);

}

// Added a new check as a sort of weak group check.
// Returns true if the 2 chars are joined in anyway, regardless
// of whether they are grouped or not. Just at long as they are
// both following the same group leader. -- raist
int is_following ( struct char_data *ch1, struct char_data *ch2)
{
    if (ch1 == ch2)
	return(0);

    if ((!ch1) || (!ch2))
	return(0);

    if (ch2->master)
      if (ch1 == ch2->master) {
	return(1);
      }

    if (ch1->master)
      if (ch1->master == ch2) {
	return(1);
      }

    if ((ch2->master) && (ch1->master))
      if (ch1->master == ch2->master) {
	return(1);
      }

    if (MOUNTED(ch1) == ch2 || RIDDEN(ch1) == ch2)
      return(1);

    return(0);
}

// Added check for damage from area fx. All new area fx skills and spells should
// use this check for damage to other players.
// Area fx restrictions are subject to change online, so IMMS can set it up as needed
// - raist
int can_hurt( struct char_data *attacker, struct char_data *victim)
{
  // Imms are never hurt
  if(IS_IMMORTAL(victim)) {
    send_to_char( "Some puny mortal tries to hurt you.\n\r", victim);
    return 0;
  }
  
  // Killing yourself is reserved for people with int 3 or less
  if (attacker == victim) return 0;  

  // If Area FX can kill players in the same room check for following properties
  if (AREA_FX_DEADLY == 2) {
    if (is_following(victim, attacker)) return 0;
    
    // Always damage if not following and in the same room and area fx set to deadly
    return 1;
  }

  // If Area FX cannot kill players - and the players cannot kill each other
  if (IS_PC(victim) && !can_pkill(attacker, victim)) return 0;

  // If the victim is a mob, we need to check if it is to be hurt
  if (is_following(victim, attacker)) return 0;

  // If we made it through all these checks, the victim has got to die!
  return 1;
}

/*
  more new procedures
*/
void down_river( int pulse )
{
    struct char_data *ch;
    struct obj_data *obj_object, *next_obj;
    int rd, or1;
    char buf[80];
    struct room_data *rp;

    if (pulse < 0)
	return;

    EACH_CHARACTER(iter, ch)
    {
	if (!IS_NPC(ch)) {
	    if (ch->in_room != NOWHERE) {
		if (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM)
		    if ((real_roomp(ch->in_room))->river_speed > 0) {
			if ((pulse % (real_roomp(ch->in_room))->river_speed)==0) {
			    if (((real_roomp(ch->in_room))->river_dir<=5)&&((real_roomp(ch->in_room))->river_dir>=0)) {
				rd = (real_roomp(ch->in_room))->river_dir;
				for (obj_object = (real_roomp(ch->in_room))->contents;
				     obj_object; obj_object = next_obj) {
				    next_obj = obj_object->next_content;
				    if ((real_roomp(ch->in_room))->dir_option[rd]) {
					obj_from_room(obj_object);
					obj_to_room(obj_object, (real_roomp(ch->in_room))->dir_option[rd]->to_room);
				    }
				}
				/*
				  flyers don't get moved
				  */
				if (!IS_AFFECTED(ch,AFF_FLYING) && !MOUNTED(ch)) {
				    rp = real_roomp(ch->in_room);
				    if (rp && rp->dir_option[rd] &&
					rp->dir_option[rd]->to_room &&
					(EXIT(ch, rd)->to_room != NOWHERE)) {
					if (ch->specials.fighting) {
					    stop_fighting(ch);
					}
					sprintf(buf, "You drift %s...\n\r", dirs[rd]);
					send_to_char(buf,ch);
					if (RIDDEN(ch))
					    send_to_char(buf,RIDDEN(ch));

					or1 = ch->in_room;
					char_from_room(ch);
					if (RIDDEN(ch))  {
					    char_from_room(RIDDEN(ch));
					    char_to_room(RIDDEN(ch), (real_roomp(or1))->dir_option[rd]->to_room);
					}
					char_to_room(ch,(real_roomp(or1))->dir_option[rd]->to_room);

					do_look(ch, "",15);
					if (RIDDEN(ch)) {
					    do_look(RIDDEN(ch), "\0",15);
					}

					if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
					    !IS_IMMORTAL(ch)) {
					    do_death_trap(ch);
					}
				    }
				}
			    }
			}
		    }
	    }
	}
    }
    END_AITER(iter);
}

int access_check(struct char_data* ch, char* file)
{
  char buf[256];

  /* PAC -- modified index check to fail only if / occurs more than once */
  if((TRUST(ch) < TRUST_LORD) && ( index(file, '/') != rindex(file, '/')))
  {
    sprintf(buf, "Illegal file name: %s\n", file);
    send_to_char(buf, ch);
    return 1;
  }

  return 0;
}

void RoomSave(struct char_data *ch, room_num start, room_num end, const char *fname)
{
   char temp[2048], buf[80];
   int rstart, rend, i, j, x;
   size_t k;
   struct extra_descr_data *exptr;
   FILE *fp;
   struct room_data	*rp;
   struct room_direction_data	*rdd;

   sprintf(buf, "world/%s", fname ? fname : GET_IDENT(ch));
   if(access_check(ch, buf))
     return;

   if ((fp = fopen(buf,"w")) == NULL) {
       send_to_char("Can't write to disk now..try later \n\r",ch);
       return;
   }

   rstart = start;
   rend = end;

   if (((rstart <= -1) || (rend <= -1)) ||
       ((rstart > 40000) || (rend > 40000))){
    send_to_char("I don't know those room #s.  make sure they are all\n\r",ch);
    send_to_char("contiguous.\n\r",ch);
    fclose(fp);
    return;
   }

   send_to_char("Saving\n",ch);

   for (i=rstart;i<=rend;i++) {

     rp = real_roomp(i);
     if (rp==NULL)
       continue;

/*
   strip ^Ms from description
*/
     x = 0;

     if (!rp->description)
       rp->description = strdup("Empty");

     for (k = 0; k <= strlen(rp->description); k++)
     {
       if (rp->description[k] != 13)
	 temp[x++] = rp->description[k];
     }
     temp[x] = '\0';

     fprintf(fp,"#%ld\n%s~\n%s~\n",rp->number,rp->name,temp);

     if (!rp->tele_targ) {
        fprintf(fp,"%d %ld %d",rp->zone, rp->room_flags, rp->sector_type);
      } else {
	if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
	   fprintf(fp, "%d %ld -1 %d %d %d %d", rp->zone, rp->room_flags,
		rp->tele_time, rp->tele_targ,
		rp->tele_mask, rp->sector_type);
	} else {
	   fprintf(fp, "%d %ld -1 %d %d %d %d %d", rp->zone, rp->room_flags,
		rp->tele_time, rp->tele_targ,
		rp->tele_mask, rp->tele_cnt, rp->sector_type);
	}
      }
     if ((rp->sector_type == SECT_WATER_NOSWIM) ||
	 (rp->sector_type == SECT_UNDERWATER)) {
        fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
     }

     if (rp->room_flags & TUNNEL) {
       fprintf(fp, " %d ", (int)rp->moblim);
     }

     fprintf(fp,"\n");

     for (j=0;j<6;j++) {
       rdd = rp->dir_option[j];
       if (rdd) {
          fprintf(fp,"D%d\n",j);

	  if (rdd->general_description) {
	   if (strlen(rdd->general_description) > 0)
              x = 0;

              for (k = 0; k <= strlen(rdd->general_description); k++) {
                 if (rdd->general_description[k] != 13)
	            temp[x++] = rdd->general_description[k];
              }
	      temp[x] = '\0';

            fprintf(fp,"%s~\n", temp);
	  } else {
	    fprintf(fp,"~\n");
	  }

	  if (rdd->keyword) {
	   if (strlen(rdd->keyword)>0)
	     fprintf(fp, "%s~\n",rdd->keyword);
	  } else {
	    fprintf(fp, "~\n");
	  }

	  fprintf(fp, "%d %d %ld\n",
		  rdd->exit_info & ~EX_CLOSED, rdd->key, rdd->to_room);
       }
     }

/*
  extra descriptions..
*/

   for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
     x = 0;

    if (exptr->description) {
      for (k = 0; k <= strlen(exptr->description); k++) {
       if (exptr->description[k] != 13)
	 temp[x++] = exptr->description[k];
      }
      temp[x] = '\0';

     fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
    }
   }

   fprintf(fp,"S\n");

   }

   fclose(fp);
   send_to_char("\n\rDone\n\r",ch);
}


void RoomLoad( struct char_data *ch, room_num start, room_num end, const char *fname)
{
    FILE *fp;
    int found = FALSE, x;
    room_num vnum;
    char buf[256], *ptr;
    struct room_data *rp, dummy;

    sprintf(buf, "world/%s", fname ? fname : GET_IDENT(ch));
    if(access_check(ch, buf))
      return;

    if ((fp = fopen(buf,"r")) == NULL)
    {
      send_to_char("You don't appear to have an area...\n\r",ch);
      return;
    }

    send_to_char("Searching and loading rooms\n\r",ch);

    while ((!found) && ((x = feof(fp)) != TRUE)) {

	if(!(ptr = fgets(buf, sizeof(buf), fp)))
	    break;

	if(*ptr == '#')
	{
	    ptr++;
	    if(!parse_number(buf, "vnum", vnum, (const char**) &ptr, &vnum))
	    {
		log_msg("while looking for next room");
		continue;
	    }

	    if ((vnum >= start) && (vnum <= end))
	    {
		if (vnum == end)
		    found = TRUE;

		if ((rp=real_roomp(vnum)) == 0) { /* empty room */
		    CREATE(rp, struct room_data, 1);
		    room_enter(vnum, rp);
		    send_to_char("+",ch);
		} else {
		    if (rp->people) {
			act("$n reaches down and scrambles reality.",
			    FALSE, ch, NULL, rp->people, TO_ROOM);
		    }
		    cleanout_room(rp);
		    send_to_char("-",ch);
		}

		rp->number = vnum;
		load_one_room(fp, rp);

	    }
	    else
	    {
		send_to_char(".",ch);
		/*  read w/out loading */
		dummy.number = vnum;
		load_one_room(fp, &dummy);
		cleanout_room(&dummy);
	    }
	}
	else if(!strncmp(ptr, "$~", 2))
	    break;
	else
	{
	    char logbuf[256];
	    if((ptr = strchr(buf, '\n')))
		*ptr = 0;
	    sprintf(logbuf, "after #%ld, skipping: \"%s\"\n\r", vnum, buf);
	    send_to_char(logbuf, ch);
	}
    }
    fclose(fp);

    if (!found) {
	send_to_char("\n\rThe room number(s) that you specified could not all be found.\n\r",ch);
    } else {
	send_to_char("\n\rDone.\n\r",ch);
    }

}

void fake_setup_dir(FILE *fl, int room, int dir)
{
	int tmp;
	char buf[256], *temp;

	temp = (char *)buf;

	temp =	fread_string(fl); /* descr */
	temp =	fread_string(fl); /* key */

	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
}


int IsHumanoid( struct char_data *ch)
{
  /* these are all very arbitrary */

  if(!IS_NPC(ch)) return TRUE;

  switch(GET_RACE(ch))
  {
  case RACE_HUMAN:
  case RACE_GNOME:
  case RACE_ELF:
  case RACE_DWARF:
  case RACE_HOBBIT:
  case RACE_FELIS:
  case RACE_CANIS:
  case RACE_DEMI:
  case RACE_GOLEM:
  case RACE_ORC:
  case RACE_LYCANTH:
  case RACE_UNDEAD:
  case RACE_UNDERWORLD:
  case RACE_GIANT:
  case RACE_GOBLIN:
  case RACE_TROLL:
  case RACE_ENFAN:
  case RACE_PATRYN:
  case RACE_SARTAN:
  case RACE_TREANT:
  case RACE_OGRE:
  case RACE_GITHYANKI:
  case RACE_DROW:
  case RACE_FOREST_ELF:
  case RACE_HILL_GIANT:
  case RACE_HALF_ORC:
  case RACE_PIXIE:
  case RACE_HALF_ELF:
  case RACE_MINOTAUR:
  case RACE_CYCLOPS:
  case RACE_MINDFLAYER:
  case RACE_SKEXIE:
  case RACE_TROGLODYTE:
  case RACE_GNOLL:
  case RACE_BUGBEAR:
  case RACE_QUICKLING:
  case RACE_TYTAN:
  case RACE_KOBOLD:
  case RACE_STONE_GIANT:
  case RACE_FROST_GIANT:
  case RACE_FIRE_GIANT:
  case RACE_CLOUD_GIANT:
  case RACE_STORM_GIANT:
  case RACE_SHERRINPIP:
    return(TRUE);
    break;

  default:
    return(FALSE);
    break;
  }

}

int IsAnimal( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_MARINE:
    case RACE_HERBIVORE:
    case RACE_REPTILE:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsUndead( struct char_data *ch)
{

  switch(GET_RACE(ch)) {
  case RACE_UNDEAD:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsUnsamplable( struct char_data *ch)
{

  switch(GET_RACE(ch)) {
  case RACE_SHERRINPIP:
  case RACE_GOLEM:
  case RACE_ELEMENTAL:
  case RACE_HEAVEN:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsLycanthrope( struct char_data *ch)
{
  switch (GET_RACE(ch)) {
  case RACE_LYCANTH:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsDiabolic( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_PLANAR:
    case RACE_UNDERWORLD:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsReptile( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
}

int HasHands( struct char_data *ch)
{

  if (IsHumanoid(ch))
    return(TRUE);
  if (IsUndead(ch))
    return(TRUE);
  if (IsLycanthrope(ch))
    return(TRUE);
  if (IsDiabolic(ch))
    return(TRUE);
  return(FALSE);
}

int IsPerson( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_ELF:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_GNOME:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;

    }

}


int IsExtraPlanar( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
  case RACE_PLANAR:
  case RACE_UNDERWORLD:
  case RACE_HEAVEN:
  case RACE_ELEMENTAL:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }
}

void SetHunting( struct char_data *ch, struct char_data *tch)
{
  int dist;
  char buf[256];
  int align;

#if JANWORK
  return;
#endif

  if(ch->hunt_info)		/* if already hunting, ignore */
    return;

  SET_BIT(ch->specials.mob_act, ACT_ROAM);

  dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
  if (Hates(ch, tch))
    dist *=2;
  if(dist < 0)
    dist = -dist;
  if(dist < 50)
    dist = 50;

  ch->hunt_info =
    path_to_char(ch->in_room, tch, dist, HUNT_THRU_DOORS);

  ch->persist =  GetMaxLevel(ch);
  align = GET_ALIGNMENT(ch);
  if(align < 0)
    align = -align;
  if(align)
    ch->persist += ch->persist * 1000 / align;

  if (ch->hunt_info && IS_GOD(tch))
  {
    sprintf(buf, ">>%s is hunting you from %s\n\r",
	    GET_NAME(ch),
	    (real_roomp(ch->in_room))->name);
    send_to_char(buf, tch);
  }
}

/* same as sethunting, but no restricts or align garbage */

void SetHuntingNoRestrict( struct char_data *ch, struct char_data *tch,int dist)
{

  char buf[256];

  if(ch->hunt_info)		/* if already hunting, ignore */
    return;

  SET_BIT(ch->specials.mob_act, ACT_ROAM);

  ch->hunt_info =
    path_to_char(ch->in_room, tch, dist, HUNT_THRU_DOORS);

  ch->persist =  GetMaxLevel(ch) * 5;

  if (ch->hunt_info && IS_GOD(tch))
  {
    sprintf(buf, ">>%s is hunting you from %s\n\r",
	    GET_NAME(ch),
	    (real_roomp(ch->in_room))->name);
    send_to_char(buf, tch);
  }
}


void CallForGuard(struct char_data *ch, struct char_data *vict,
		  int lev, int area)
{
    struct char_data *i;
    int type1, type2;

    if(!vict)
	return;

    switch(area) {
    case MIDGAARD:
	type1 = VMOB_70;
	type2 = VMOB_72;
	break;
    case NEWTHALOS:
	type1 = VMOB_93;
	type2 = VMOB_94;
	break;
    case TROGCAVES:
	type1 = VMOB_95;
	type2 = VMOB_96;
	break;
    case OUTPOST:
	type1 = VMOB_GhostSoldier;
	type2 = VMOB_GhostLieutenant;
	break;
    default:
	type1 = VMOB_70;
	type2 = VMOB_72;
	break;
    }

    EACH_CHARACTER(iter, i)
    {
	if (!IS_PC(i) && (i != ch) &&
	    !i->specials.fighting && !i->hunt_info)
	{
	    if (mob_index[i->nr].virt == type1)
	    {
		if (number(1,6) == 1)
		{
		    SetHunting(i, vict);
		}
	    }
	    else if (mob_index[i->nr].virt == type2)
	    {
		if (number(1,6) == 1) {
		    SetHunting(i, vict);
		}
	    }
	}
    }
    END_AITER(iter);
}

void StandUp (struct char_data *ch)
{
   if ((GET_POS(ch)<POSITION_STANDING) &&
       (GET_POS(ch)>POSITION_STUNNED))  {
       if (ch->points.hit > (ch->points.max_hit / 2))
         act("$n quickly stands up.", 1, ch,0,0,TO_ROOM);
       else if (ch->points.hit > (ch->points.max_hit / 6))
         act("$n slowly stands up.", 1, ch,0,0,TO_ROOM);
       else
         act("$n gets to $s feet very slowly.", 1, ch,0,0,TO_ROOM);
       GET_POS(ch)=POSITION_STANDING;
 }
}


void FighterMove( struct char_data *ch)
{
  int num;

  if (!ch->skills)
    SpaceForSkills(ch);
  num = number(1,4);
  if (num == 2) {
      if (!ch->skills[SKILL_BASH].learned)
         ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch)*4;
      do_bash(ch, GET_NAME(ch->specials.fighting), 0);
    } else if (num == 1) {
      if ((!IS_SET(ch->specials.mob_act,ACT_WIMPY)) ||
          (number(1,15)==5)) {
        if (!ch->skills[SKILL_BERSERK].learned)
	   ch->skills[SKILL_BERSERK].learned = 10 + GetMaxLevel(ch)*4;
	do_berserk(ch, "", 0);
      }
    } else if (num == 3) {
      if (ch->equipment[WIELD]) {
         if (!ch->skills[SKILL_DISARM].learned)
            ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch)*4;
         do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
       } else {
         if (!ch->skills[SKILL_DISARM].learned)
            ch->skills[SKILL_DISARM].learned = 60 + GetMaxLevel(ch)*4;
         do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
       }
   } else {
      if (!ch->skills[SKILL_KICK].learned)
         ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
   }
}


void DevelopHatred( struct char_data *ch, struct char_data *v)
{
   int diff, patience, var;

   if (Hates(ch, v))
     return;

  if (ch == v)
    return;

  diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
  if(diff < 0)
     diff = -diff;

  diff /= 20;

  if (GET_MAX_HIT(ch)) {
     patience = (int) (100 * ((double) GET_HIT(ch) /
			      (double) GET_MAX_HIT(ch)));
  } else {
     patience = 10;
  }

  var = number(1,40) - 20;

  if (patience+var < diff)
     assert(ch && v);
     AddHated(ch, v);

}

int HasObject( struct char_data *ch, int ob_num)
{
    int j, found;
    struct obj_data *i;

    /*
	equipment too
	    */

    found = 0;

    for (j=0; j<MAX_WEAR; j++)
	if (ch->equipment[j])
	    found += RecCompObjNum(ch->equipment[j], ob_num);

    if (found > 0)
	return(TRUE);

	/* carrying */
    for (i = ch->carrying; i; i = i->next_content)
	found += RecCompObjNum(i, ob_num);

    if (found > 0)
	return(TRUE);
    else
	return(FALSE);
}


struct char_data *char_holding(struct obj_data *obj)
{
    if(obj->in_room != NOWHERE)
	return NULL;
    else if (obj->carried_by)
	return obj->carried_by;
    else if (obj->equipped_by)
	return obj->equipped_by;
    else if (obj->in_obj)
	return char_holding(obj->in_obj);
    else
	return NULL;
}

int RecCompObjNum( struct obj_data *o, int obj_num)
{

int total=0;
struct obj_data *i;

  if (obj_index[o->item_number].virt == obj_num)
    total = 1;

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (i = o->contains; i; i = i->next_content)
      total += RecCompObjNum( i, obj_num);
  }
  return(total);

}

void RestoreChar(struct char_data *ch)
{

  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  if (!IS_GOD(ch)) {
    if (GET_COND(ch,THIRST) != -1)
      GET_COND(ch,THIRST) = 24;
    if (GET_COND(ch,FULL) != -1)
      GET_COND(ch,FULL) = 24;
  } else {
    GET_COND(ch,THIRST) = -1;
    GET_COND(ch,FULL) = -1;
  }

}


void RemAllAffects(struct char_data *ch)
{
    cast_dispel_magic(ABS_MAX_LVL, ch, 0, SPELL_TYPE_SPELL, ch, 0);

    if (affected_by_spell(ch,SPELL_POISON))
      affect_from_char(ch,SPELL_POISON);

    if (affected_by_spell(ch,SPELL_PARALYSIS))
      affect_from_char(ch,SPELL_PARALYSIS);

    if (affected_by_spell(ch,SPELL_CHARM))
      affect_from_char(ch,SPELL_CHARM);
}

int CheckForBlockedMove(struct char_data *ch, int cmd,
			char *arg, room_num room, int dir, int clss)
{

  char buf[256], buf2[256];

  if (cmd>6 || cmd<1)
    return(FALSE);

  strcpy(buf,  "The guard humiliates you, and block your way.\n\r");
  strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

  if ((IS_NPC(ch) && (IS_POLICE(ch))) || (TRUST(ch) >= TRUST_DEMIGOD) ||
      (IS_AFFECTED(ch, AFF_SNEAK)))
    return(FALSE);

  if ((ch->in_room == room) && (cmd == dir+1)) {
    if (!HasClass(ch,clss))  {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  }
  return FALSE;

}


#if 0
void TeleportPulseStuff(int pulse)
{

  /*
    check_mobile_activity(pulse);
    Teleport(pulse);
    */

  register struct char_data *ch;
  struct char_data *tmp, *pers;
  int tick, tm, or;
  struct room_data *rp, *dest;
  struct obj_data *obj_object, *temp_obj;

  tm = pulse % PULSE_MOBILE;    /* this is dependent on P_M = 3*P_T */

  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT*2) {
    tick = 2;
  }

  EACH_CHARACTER(iter, ch)
  {
    if (IS_MOB(ch)) {
      if (ch->specials.tick == tick) {
	mobile_activity(ch);
      }
    }

#if !MOBS_TRIGGER_TELEPORT
    if (IS_PC(ch))
#endif
    {
	rp = real_roomp(ch->in_room);
	if (rp &&
	    (rp)->tele_targ > 0 &&
	    rp->tele_targ != rp->number &&
	    (rp)->tele_time > 0 &&
	    (pulse % (rp)->tele_time)==0) {

	    dest = real_roomp(rp->tele_targ);
	    if (!dest) {
		char buf[256];
		sprintf(buf, "invalid teleport target from room %d",
			ch->in_room);
		log_msg(buf);
		continue;
	    }

	    STATE2("Moving objects: %d->%d", rp->number, rp->tele_targ);
	    obj_object = (rp)->contents;
	    while (obj_object) {
		temp_obj = obj_object->next_content;
		obj_from_room(obj_object);
		obj_to_room(obj_object, (rp)->tele_targ);
		obj_object = temp_obj;
	    }

	    STATE2("Moving people: %d->%d", rp->number, rp->tele_targ);
	    while(rp->people	/* should never fail */) {

		tmp = rp->people; /* work through the list of people */
		if (!tmp) break;

		or = tmp->in_room;
		char_from_room(tmp); /* the list of people in the room has changed */
		char_to_room(tmp, rp->tele_targ);

		if (IS_SET(TELE_LOOK, rp->tele_mask) && IS_PC(tmp)) {
		    do_look(tmp, "\0",15);
		}

		if (IS_SET(dest->room_flags, DEATH) && (!IS_IMMORTAL(tmp))) {
		    do_death_trap(tmp);
		    continue;
		}
	    }

	    if (IS_SET(TELE_COUNT, rp->tele_mask)) {
		rp->tele_time = 0; /* reset it for next count */
	    }
	    if (IS_SET(TELE_RANDOM, rp->tele_mask)) {
		rp->tele_time = number(1,10)*100;
	    }
	}
    }
  }
  END_ITER(iter);
}

void RiverPulseStuff(int pulse)
{
    register struct char_data *ch, *c;
    register struct obj_data *obj;
    int rd, dest;
    char buf[256];
    struct room_data *room;

    if (pulse < 0)
	return;

    EACH_CHARACTER(iter, ch)
    {
	if(!IS_PC(ch) && (ch->player.sounds) && (number(0,5)==0))
	{
	    if (ch->specials.default_pos > POSITION_SLEEPING)
	    {
		if (GET_POS(ch) > POSITION_SLEEPING)
		    MakeNoise(ch->in_room,
			      ss_data(ch->player.sounds),
			      ss_data(ch->player.distant_snds));
		else if (GET_POS(ch) == POSITION_SLEEPING)
		{
		    /*
		     * snore
		     */
		    sprintf(buf, "%s snores loudly.\n\r", GET_NAME(ch));
		    MakeNoise(ch->in_room, buf,
			      "You hear a loud snore nearby.\n\r");
		}
	    }
	    else if (GET_POS(ch) == ch->specials.default_pos)
	    {
		/*
		 * Make the sound
		 */
		MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
	    }
	}
    }

    END_ITER(iter);
}
#endif


/*
**  Apply soundproof is for ch making noise
*/
int apply_soundproof(struct char_data *ch)
{
  struct room_data *rp;

  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    send_to_char("You are silenced, you can't make a sound!\n\r", ch);
    return(TRUE);
  }

  rp = real_roomp(ch->in_room);

  if (!rp) return(FALSE);

  if (IS_SET(rp->room_flags, SILENCE) && !IS_IMMORTAL(ch)) {
     send_to_char("You are in a silence zone, you can't make a sound!\n\r",ch);
     return(TRUE);   /* for shouts, emotes, etc */
  }
  return(FALSE);

}

int check_nomagic(struct char_data* ch)
{
    struct room_data* rp;

    if(IS_IMMORTAL(ch))
	return 0;

    if(!(rp = real_roomp(ch->in_room)))
	return 0;

    if(IS_SET(rp->room_flags, NO_MAGIC))
    {
	send_to_room("The magic fades away.\n\r", ch->in_room);
	return 1;
    }

    return 0;
}

/*
**  check_soundproof is for others making noise
*/
int check_soundproof(struct char_data *ch)
{
  struct room_data *rp;

  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    return(TRUE);
  }

  rp = real_roomp(ch->in_room);

  if (!rp) return(FALSE);

  if (IS_SET(rp->room_flags, SILENCE)) {
     return(TRUE);   /* for shouts, emotes, etc */
  }
  return(FALSE);
}

int MobCountInRoom( struct char_data *list)
{
  int i;
  struct char_data *tmp;

  for (i=0, tmp = list; tmp; tmp = tmp->next_in_room, i++)
    ;

  return(i);

}

/*
  returns, extracts, switches etc.. anyone.
*/
void NailThisSucker( struct char_data *ch)
{

  struct char_data *pers;

  death_cry(ch);

  if (IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
    /*
     *   take char from storage, to room
     */
    pers = pop_character(ch);

    char_from_room(pers);
    char_to_room(pers, ch->in_room);
    SwitchStuff(ch, pers);
    extract_char(ch);
    ch = pers;
  }
  extract_char(ch);
}



int get_average_level(struct char_data* ch)
{
  int c, l, i;

  for(i = c = l = 0 ; i <= MAX_LEVEL_IND ; ++i)
  {
    if(GET_LEVEL(ch, i))
    {
      c++;
      l += GET_LEVEL(ch, i);
    }
  }

  return c ? (l / c) : 0;
}

void do_area_attack(int level, struct char_data* ch,
		    void (*proc)(ubyte level, struct char_data* ch, int type,
				 struct char_data* victim,
				 struct obj_data* obj), int type)
{
    struct char_data* victim;
    struct char_data* next;
    struct room_data* roomp;

    if(!(roomp = real_roomp(ch->in_room)))
    {
	char buf[256];
	sprintf(buf, "%s used area affect in non-existent room: %ld",
		GET_NAME(ch), ch->in_room);
	log_msg(buf);
    }

    for (victim = roomp->people ; victim ; victim = next)
    {
	next = victim->next_in_room;
	if (!IS_IMMORTAL(victim) && !in_group(ch, victim))
	    (*proc)(level, ch, type, victim, 0);
    }
}

void do_area_spell(int level, struct char_data* ch,
		   void (*proc)(ubyte level, struct char_data* ch, int type,
				struct char_data* victim,
				struct obj_data* obj), int type)
{
    struct char_data* victim;
    struct char_data* next;
    struct room_data* roomp;

    if(!(roomp = real_roomp(ch->in_room)))
    {
	char buf[256];
	sprintf(buf, "%s used area affect in non-existent room: %ld",
		GET_NAME(ch), ch->in_room);
	log_msg(buf);
    }

    for (victim = roomp->people ; victim ; victim = next)
    {
	next = victim->next_in_room;
	if(victim != ch)
	    (*proc)(level, ch, type, victim, 0);
    }
}


/*** perform align test to see if char can wear this item ***/
/*** the reason for putting this in a seperate routine instead of   ***/
/*** inside MobScavenge() is so that spec_procs such as the janitor ***/
/*** and my trolls :) can make use of it as well. i will submit     ***/
/*** updated versions of their spec_procs right away. ***/

int can_wear_test(struct char_data *ch, struct obj_data *obj)
{
   if (((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) &&
	(!IS_IMMORTAL(ch)))
      return (FALSE);

   return (TRUE);
}

void push_character(struct char_data* ch, struct char_data* new_ch)
{
    struct char_data* god;

    if(ch->desc)
    {
	ch->desc->character = new_ch;

	if((god = ch->desc->snoop.snoop_by))
	    god->desc->snoop.snooping = new_ch;

	if(ch->desc->snoop.snooping)
	    ch->desc->snoop.snooping = new_ch;
    }

    new_ch->desc = ch->desc;
    ch->desc = 0;

    new_ch->orig = ch;
    ch->curr = new_ch;
}

struct char_data* pop_character(struct char_data* ch)
{
    struct char_data* new_ch;
    struct char_data* god;

    if((new_ch = ch->orig))
    {
	if(ch->desc)
	{
	    if((god = ch->desc->snoop.snoop_by))
		god->desc->snoop.snooping = new_ch;

	    if(ch->desc->snoop.snooping)
		ch->desc->snoop.snooping = new_ch;

	    ch->desc->character = new_ch;
	}

	new_ch->desc = ch->desc;
	ch->desc = 0;

	ch->orig = 0;
	new_ch->curr = 0;
    }

    return new_ch;
}

struct char_data* curr_character(struct char_data* ch)
{
    if(!ch)
	return NULL;

    while(ch->curr)
	ch = ch->curr;

    return ch;
}

void set_descriptor(struct char_data* ch, struct descriptor_data* desc)
{
    ch->desc = desc;
}

/* drop everything a character has in a room.  Typically because they
   quit or dt'd... */
void StuffToRoom(struct char_data* ch, room_num in_room)
{
    struct obj_data*	obj;
    int			i;

    for(i = 0 ; i < MAX_WEAR ; ++i)
    {
	if(ch->equipment[i])
	    obj_to_room(unequip_char(ch, i), in_room);
    }

    while((obj = ch->carrying))
    {
	obj_from_char(obj);
	obj_to_room(obj, in_room);
    }
}

char* escape(const char* str)
{
    static char buf[512];
    char* dst;

    for(dst = buf ; *str ; str++, dst++)
    {
	if(*str == '$')
	    *dst++ = '$';
	*dst = *str;
    }

    *dst = 0;

    return buf;
}


void ObjFromCorpse( struct obj_data *c)
{
  struct obj_data *jj, *next_thing;

  for(jj = c->contains; jj; jj = next_thing) {
    next_thing = jj->next_content;/* Next in inventory */
    if (jj->in_obj) {
      obj_from_obj(jj);
      if (c->in_obj)
	obj_to_obj(jj,c->in_obj);
      else if (c->carried_by)
	obj_to_room(jj,c->carried_by->in_room);
      else if (c->in_room != NOWHERE)
	obj_to_room(jj,c->in_room);
      else
	assert(FALSE);
    } else {
      /*
       **  hmm..  it isn't in the object it says it is in.
       **  don't extract it.
       */
      c->contains = 0;
      log_msg("Memory lost in ObjFromCorpse.");
    }
  }
  extract_obj(c);
}




/* Lose in various points */
/*
** Damn tricky for multi-class...
*/

void drop_level(struct char_data *ch, int clss)
{
  int add_hp;

  if (GetMaxLevel(ch) == 1)
    return;

  add_hp = con_app[GET_CON(ch)].hitp;

  switch(clss) {

  case MAGE_LEVEL_IND : {
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
      add_hp += number(3, 6);
    else
      add_hp += 1;
  } break;

  case CLERIC_LEVEL_IND : {
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12)
      add_hp += number(3, 10);
    else
      add_hp += 3;
  } break;

  case THIEF_LEVEL_IND : {
    if (!(HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_MAGIC_USER)))
      if(!HasClass(ch, CLASS_WARRIOR))
	add_hp += number(8,15);
      else
	add_hp += number(6,12);
    else if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 12)
      add_hp += number(3,8);
    else
      add_hp += 2;
  } break;

  case PALADIN_LEVEL_IND : {
    add_hp += (number(3,9));
  } break;

  case DRUID_LEVEL_IND : {
    add_hp += (number(2,6));
  } break;

  case PSI_LEVEL_IND : {
    add_hp += (number(2,4));
  } break;

  case RANGER_LEVEL_IND : {
    add_hp += (number(3,8));
  } break;

  case SHIFTER_LEVEL_IND : {
    add_hp += (number(2,6));
  } break;

  case WARRIOR_LEVEL_IND : {
    if (!(HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_MAGIC_USER)))
      if(!HasClass(ch, CLASS_THIEF))
	add_hp += number(10,18);
      else
	add_hp += number(6, 12);
    else if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 12)
      add_hp += number(3,10);
    else
      add_hp += 4;
  } break;
  }

  add_hp /= HowManyClasses(ch);
  add_hp += number(1,15);

  GET_LEVEL(ch, clss) -= 1;

  if (GET_LEVEL(ch, clss) < 1) {
    GET_LEVEL(ch, clss) = 1;
  }

  ch->points.max_hit -= MAX(1,add_hp);
  if (ch->points.max_hit < 1)
    ch->points.max_hit = 1;

  ch->specials.spells_to_learn -= MAX(2, wis_app[GET_WIS(ch)].bonus);
  if (ch->specials.spells_to_learn < 1)
    ch->specials.spells_to_learn = 1;

  ch->points.exp =
    MIN(exp_table[(int)GET_LEVEL(ch, clss)], GET_EXP(ch));
  if (GET_EXP(ch) < 1)
    GET_EXP(ch) = 1;

}

void gain_exp_heal(struct char_data *ch, int gain)
{
  int i, count=0;
  char buf[256];
  int maxed = 0;
  int wasted = 0;                   /* Should not get Long amount of exp at
				       any one time */
  unsigned again;                   /* Gain should never be negative */
  EXP exp, next, need;              /* The big honkin numbers */
  int max = 0;                      /* Max exp you can get at one time */
  int level = 0;


  if (!IS_IMMORTAL(ch))
  {
    if (gain >= 0)
    {
      if (GetMaxLevel(ch) < 5)
	gain *= 2;

      exp = GET_EXP(ch);

//	gain /= 2*classes-1;

      /* Most experience you can get based on # classes */
      max = MAXEXP;
      again = MIN(max, gain);        /* What you actually gonna get */

      next = exp + again;            /* What exp you will end up with */
      if ((next < exp) && (next < again))
      {
	slog ("Overflow in charcter experience occured");
	sprintf(buf, "  Due to overflow, you lost$CY %Ld$CN experience.\n\r",
		next+1);
	send_to_char_formatted(buf,ch);
	next = ULONG_MAX;
      }

      if(IS_PC(ch))
      {
	wasted = gain - again;

	level = GetMinLevel(ch);

	/* Possible over flow here ? */
	need = exp_table[level] + exp_table[level + 1];
	if ((need < exp_table[level]) && (need < exp_table[level + 1]))
	{
	  wasted = need + 1; /* Lost due to overflow */
	  need = ULONG_MAX;
	}
	else if (next > need)
	{
	  maxed=1;
	  wasted += (int) (next - (need - 1));
	  next = need - 1;
	}

	  sprintf(buf,"$CwYou get$CG %d$Cw experience.$CN\n\r",again);
	  send_to_char_formatted(buf,ch);
	  if (wasted > 0)  {
	    sprintf(buf,"  Due to a maximum of 300k exp per kill and/or reaching maximum experience,\n\r"
		    "  You lost$CY %d$CN experience.\n\r",wasted);
	    send_to_char_formatted(buf,ch);
	  }

	if(maxed)
	  send_to_char_formatted("$CGYou must go gain before you can gain any more experience.$CN\n\r", ch);

	for(i = 0 ; i <= MAX_LEVEL_IND ; ++i) {
	  int level = GET_LEVEL(ch, i);

	  if(level &&
	     (exp < exp_table[level]) &&
	     (next >= exp_table[level]))
	    {
	      if (!count)
		sprintf(buf, "$CgYou have received enough exp to be a(an)$CG %s$CN\n\r",
			ch->player.sex == SEX_FEMALE ?
			titles[i][level + 1].title_f :
			titles[i][level + 1].title_m);
	      else
		sprintf(buf, "$Cg  Or a(an)$CG %s$CN\n\r",
			ch->player.sex == SEX_FEMALE ?
			titles[i][level + 1].title_f :
			titles[i][level + 1].title_m);
	      send_to_char_formatted(buf, ch);
	      count++;

	    }
	}
      }
      GET_EXP(ch)=next;
    }
    else if (gain < 0)
      {
	exp = GET_EXP(ch);
	GET_EXP(ch) += gain;
	if ((GET_EXP(ch) > exp) || (GET_EXP(ch)< 0))
	  GET_EXP(ch) = 0;
       }
  }
  if(!IS_NPC(ch)) {
    add_char_to_hero_list(ch);
    do_save(ch, "", 0);
  }
}




void gain_exp(struct char_data *ch, int gain)
{
  int i, count=0;
  char buf[MAX_INPUT_LENGTH];
  int maxed = 0;
  int classes, wasted = 0;          /* Should not get Long amount of exp at
				       any one time */
  unsigned again;                   /* Gain should never be negative */
  EXP exp, next, need;              /* The big honkin numbers */
  int max = 0;                      /* Max exp you can get at one time */
  int level = 0;

  WAIT_STATE(ch,PULSE_VIOLENCE*2);  /* TRYING TO FIX A BUG... CHARACTER IS
				       TRYING TO GET FROM CORPSE WHILE IT IS
				       BEING MADE, I THINK...  DAMN CLIENTS! */
/*** Commented out by Solaar to remove group level restrictions.
#ifdef GROUP_LEVEL_RESTRICTS
  // ------------ new group restricts code - tanga (c)2004 -------------------
  char_data *leader;
  int max_group_level;
  struct follow_type *foll_idx;

if (GROUP_RES == 2) { // only if they are active ingame - raist
  if (GetMaxLevel(ch) > 20)  // group level restricts only apply above lvl 20
    {
      if (IS_AFFECTED(ch, AFF_GROUP))
	{
	  if (ch->master) // set leader to characters master
	    leader = ch->master;
	  else
	    leader = ch;  // character IS the master

	  // iterate through followers

	  max_group_level = GetMaxLevel(leader);  // might be the highest lvl!

	  for (foll_idx = leader->followers; foll_idx; foll_idx=foll_idx->next) // check out all the followers
	    if ( IS_AFFECTED(foll_idx->follower, AFF_GROUP) ) // member of group?
	      if (GetMaxLevel(foll_idx->follower) > max_group_level)
		max_group_level = GetMaxLevel(foll_idx->follower); // new group max level

	  if ( (max_group_level - GetMaxLevel(ch)) > 30 ) // uh oh
	    {
	      sprintf(buf, "  $CYYou have received no experience$CN.\n\r"
		           "  You must be under Lvl 21, or within 30 levels of the highest member\n\r"
		           "  of the group to get experience\n\r");
	      send_to_char_formatted(buf,ch);
	      return;
	    }
	}
    }
} // end if group res active
  // --- End of test for grouping levels --------------------------------------

#endif
***/


  if (IS_PC(ch) && !IS_IMMORTAL(ch))
  {
    if (gain >= 0)
    {
      if (GetMaxLevel(ch) < 5)
	gain *= 2;

      exp = GET_EXP(ch);

      /* If multiclass get less exp */
      if ((classes = HowManyClasses(ch)) > 0)
      {
         if(classes >= 3){
            gain = (int)(gain * .65);
            max = MAXEXP;
            max = (int)(max * .65);
         }
         else if(classes >=2) {
            gain = (int)(gain *.75);
            max = MAXEXP;
            max = (int)(max * .75);
         }
         else
            max = MAXEXP;
      }

      again = MIN(max, gain);        /* What you actually gonna get */

      next = exp + again;            /* What exp you will end up with */
      if ((next < exp) && (next < again))
      {
	slog ("Overflow in charcter experience occured");
	sprintf(buf, "  Due to overflow, you lost$CY %Ld$CN experience.\n\r",
		next+1);
	send_to_char_formatted(buf,ch);
	next = ULONG_MAX;
      }

      if(IS_PC(ch))
      {
	wasted = gain - again;

	level = GetMinLevel(ch);

	/* Possible over flow here ? */
	need = exp_table[level] + exp_table[level + 1];
	if ((need < exp_table[level]) && (need < exp_table[level + 1]))
	{
	  wasted = need + 1; /* Lost due to overflow */
	  need = ULONG_MAX;
	}
	else if (next > need)
	{
	  maxed=1;
	  wasted += (int) (next - (need - 1));
	  next = need - 1;
	}

	if ((ch->followers==NULL) && (ch->master==NULL))  {
	  sprintf(buf,"$CwYou get$CG %d$Cw experience.$CN\n\r",again);
	  send_to_char_formatted(buf,ch);
	  if (wasted > 0)  {
	    sprintf(buf,"  Due to a maximum of 300k exp per kill and/or reaching maximum experience,\n\r"
		    "  You lost$CY %d$CN experience.\n\r",wasted);
	    send_to_char_formatted(buf,ch);
	  }
	}
	else if(IS_AFFECTED(ch, AFF_GROUP)) {
	  sprintf(buf, "$CwYou get your share of$CG %d$Cw experience.$CN\n\r", again);
	  send_to_char_formatted(buf, ch);
	  if (wasted > 0) {
	    sprintf(buf, "  Due to maximum of 300k exp per share and/or to reaching maximum exp.\n\r"
		    "  You lost$CY %d$CN experience.\n\r", wasted);
	    send_to_char_formatted(buf,ch);
	  }
	}

	if(maxed)
	  send_to_char_formatted("$CGYou must go gain before you can gain any more experience.$CN\n\r", ch);

	for(i = 0 ; i <= MAX_LEVEL_IND ; ++i) {
	  int level = GET_LEVEL(ch, i);

	  if(level &&
	     (exp < exp_table[level]) &&
	     (next >= exp_table[level]))
	    {
	      if (!count)
		sprintf(buf, "$CgYou have received enough exp to be a(an)$CG %s$CN\n\r",
			ch->player.sex == SEX_FEMALE ?
			titles[i][level + 1].title_f :
			titles[i][level + 1].title_m);
	      else
		sprintf(buf, "$Cg  Or a(an)$CG %s$CN\n\r",
			ch->player.sex == SEX_FEMALE ?
			titles[i][level + 1].title_f :
			titles[i][level + 1].title_m);
	      send_to_char_formatted(buf, ch);
	      count++;

	    }
	}
      }


      GET_EXP(ch)=next;
    } // IS_PC end
    else if (gain < 0)
      {
	exp = GET_EXP(ch);
	GET_EXP(ch) += gain;
	if ((GET_EXP(ch) > exp) || (GET_EXP(ch)< 0))
	  GET_EXP(ch) = 0;
       }
  }

  if(IS_PC(ch) && !IS_IMMORTAL(ch)) {
    add_char_to_hero_list(ch);
    do_save(ch, "", 0);
  }
}


void gain_exp_regardless(struct char_data *ch, int gain, int clss)
{
  int i;
  bool is_altered = FALSE;
  EXP exp;                                /*Players current exp */

  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i=0;(i<ABS_MAX_LVL) &&(exp_table[i] <= GET_EXP(ch)); i++) {
	if (i > GET_LEVEL(ch,clss)) {
	  send_to_char_formatted("$CBYou raise a level$CN\n\r", ch);
	  GET_LEVEL(ch,clss) = i;
	  advance_level(ch,clss);
	  is_altered = TRUE;
	}
      }
    }
    else if (gain < 0)
      {
	exp = GET_EXP(ch);
	GET_EXP(ch) += gain;
	if ((GET_EXP(ch) > exp) || (GET_EXP(ch)< 0))
	  GET_EXP(ch) = 0;
       }
  }
  if (is_altered)
    set_title(ch);
  if (!IS_NPC(ch))
      add_char_to_hero_list(ch);
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
  if (list) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}

int check_convict(struct char_data* ch, struct char_data* keeper)
{
    if(IS_SET(ch->specials.flags, PLR_PKILLER))
    {
	if(keeper)
	{
	    act("You look pointedly between $N and a poster on the wall.",
		TRUE, keeper, NULL, ch, TO_CHAR);
	    act("$n looks pointedly between you and a poster on the wall.",
		TRUE, keeper, NULL, ch, TO_VICT);
	    act("$n looks pointedly between $N and a poster on the wall.",
		TRUE, keeper, NULL, ch, TO_NOTVICT);
	}

	if(number(1, 100) < 10)
	    launch_hunter(GetMaxLevel(ch));

	return 1;
    }

    return 0;
}

#define GUARD_HALL 3007

void launch_hunter(int level)
{
    struct hunter_info
    {
	int		level;
	int		vnum;
    };
    static struct hunter_info hunters[] =
    {
	{ 10,		VMOB_56 },
	{ 20,		VMOB_57 },
	{ 30,		VMOB_58 },
	{ 40,		VMOB_59 },
	{ 49,		VMOB_60 },
	{ 50,		VMOB_55 },	/* terminator */
	{ 0,		0 }
    };
    struct hunter_info* hunter;
    struct room_data* rp;
    struct char_data* mob;
    char buf[512];

    for(hunter = hunters ; hunter->level && hunter->level < level ; hunter++)
	;

    if(!hunter->level)
	return;

    if((rp = real_roomp(GUARD_HALL)) == NULL)
    {
	log_msg("No guard hall for bounty hunters");
	return;
    }

    if((mob = make_mobile(hunter->vnum, VIRTUAL)) == NULL)
    {
	sprintf(buf, "Problem loading bounty hunter: %d", hunter->vnum);
	log_msg(buf);
    }

    char_to_room(mob, GUARD_HALL);
    act("You hear a soft 'pop!'", FALSE, mob, NULL, NULL, TO_ROOM);

    sprintf(buf, "Launching bounty hunter %d", hunter->vnum);
    log_msg(buf);
}

EXP total_exp(int level)
{
    int		i;
    EXP         exp;

    for(i = 0, exp = 0 ; i < level ; ++i)
      exp += exp_table[i];

    return exp;
}

int travel_check(struct char_data* ch, struct char_data* vict)
{
    struct room_data* rp;

    if(!(rp = real_roomp(ch->in_room)) ||
       IS_SET(rp->room_flags, NO_TRAVEL_OUT))
	return 0;

    if(!(rp = real_roomp(vict->in_room)) ||
       IS_SET(rp->room_flags, NO_TRAVEL_IN) ||
       (IS_SET(rp->room_flags, IMMORT_RM) &&
	IS_GOD(ch)) ||
       (IS_SET(rp->room_flags, BRUJAH_RM) &&
        (IS_SET(ch->specials.flags, PLR_BRUJAH))) ||
       (IS_SET(rp->room_flags, GOD_RM) &&
	(TRUST(ch) < TRUST_LORD)) ||
       (IS_SET(rp->room_flags, TUNNEL) &&
	(MobCountInRoom(rp->people)>rp->moblim)) ||
       (IS_SET(ch->specials.mob_act, ACT_HUGE) &&
	IS_SET(rp->room_flags, INDOORS)))
	return 0;

    return 1;
}

int track_to_room(struct char_data* ch, room_num room_nr,
		  int depth, int flags)
{
    if(ch->hunt_info)
    {
	if(ch->hunt_info->dest == room_nr)
	    return 0;
	path_kill(ch->hunt_info);
    }

    ch->hunt_info = path_to_room(ch->in_room, room_nr, depth, flags);
    ch->persist = depth;

    return 1;
}

int track_to_char(struct char_data* ch, struct char_data* vict,
		  int depth, int flags)
{
  if(ch->hunt_info)
  {
    if(ch->hunt_info->victim == vict)
      return 0;
    path_kill(ch->hunt_info);
  }

  ch->hunt_info = path_to_char(ch->in_room, vict, depth, flags);
  ch->persist = depth;

  return 1;
}

int GetRealLevel(struct char_data* ch)
{
  return GetMaxLevel(real_character(ch));
}

/* return a number between 1 and ... you guessed it... 100 */
int percent(void)
{
    return number_percent();
}

/* this function returns a number from 0 to the parameter
   (inclusive, ie: randomnum(10) = 0-10) */
/* redone june 20 to use the xrand stuff */

int randomnum(int maximum) {
  return number_range( 0, maximum);
}




/* MORE CHEAP HACKS OF CIRCLE CODE (Thanks for a Wicked Job JEREMY!)
 * Added to support mobprogs by Min 1996... may be used elsewhere
 */

/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg) {
  if (!strcmp(arg,"all"))
    return FIND_ALL;
  else if (!strncmp(arg,"all.", 4)) {
    strcpy(arg,arg+4);
    return FIND_ALLDOT;
  } else
    return FIND_INDIV;
}

/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_num find_target_room(struct char_data * ch, char *rawroomstr) {

  room_num location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    location = atoi(roomstr);
    if ((real_roomp(location)) == NULL ) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (TRUST(ch) < TRUST_GRGOD) {
    if (ROOM_FLAGGED(location, GOD_RM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, IMMORT_RM) &&
	real_roomp(ch->in_room)->people &&
	real_roomp(ch->in_room)->people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }
    /* I'm leaving the following code in so as to allow for houses :) */
#ifdef JANWORK
    if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
	!House_can_enter(ch, world[location].number)) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return NOWHERE;
    }
#endif
  }
  return location;
}

/* create a duplicate of string (from Circle Code) */
char *str_dup(const char *source) {
  char *nw;

  CREATE(nw, char, strlen(source) + 1);
  return (strcpy(nw, source));
}

/* different ways of reading a file  FROM MERC.. thanks guyz!) */

char fread_letter (FILE *fp ) {
  char c;

  do {
    c = getc(fp);
  } while ( isspace(c) && (feof(fp) == 0));

  if (feof(fp)) c = (char) -1;

  return c;
}


int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) ) {
      log_msg("fread_number: bad format");
      exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}




/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    if (feof(fp))
     return;

    do {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' && !feof(fp));

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp ) {
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do {
	cEnd = getc( fp );
    } while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }
    log_msg("fread_word: word too long.");

    return NULL;
}


#define SKILL_MODIFIERS(skill)    spell_list[skill].modifiers

/* returns the skill chance modified by stats */
byte skill_chance (struct char_data *ch, int skill)
{
    int x, flag = 0, chance = 0, mods = 0;
    struct spell_info *ability;
    int stat_mod[] =  {
	-17,   -15,   -13,   -12,   -10,   /* 4  */
	-8,    -6,    -4,    -2,     0,    /* 9  */
	 0,     0,     0,     1,     4,    /* 14 */
	 6,     8,     12,    14,    16,   /* 19 */
	 18,    20,    20,    20,    20,   /* 24 */
	 20
    };

    /* first check if character can use the skill */
    if (!ch->skills || !ch->skills[skill].learned)
	return 0;

    ability = spell_by_number(skill);
    for (x = 0; x <= MAX_LEVEL_IND && !flag; x++)
	if (GET_LEVEL(ch, x) >= ability->min_level[x])
	    flag = TRUE;

    if (!flag)
	return 0;

    chance = ch->skills[skill].learned;

    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_STR))
	chance += stat_mod[GET_STR(ch)];
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_INT))
	chance += stat_mod[GET_INT(ch)];
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_WIS))
	chance += stat_mod[GET_WIS(ch)];
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_DEX))
	chance += stat_mod[GET_DEX(ch)];
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_CON))
	chance += stat_mod[GET_CON(ch)];
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_CHA))
	chance += stat_mod[GET_CHA(ch)];

    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_GOOD))
	ch->specials.alignment > 350 ? mods++ : mods--;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_EVIL))
	ch->specials.alignment < -350 ? mods++ : mods--;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_FEMALE))
	ch->player.sex == SEX_FEMALE ? mods++ : mods--;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_MALE))
	ch->player.sex == SEX_MALE ? mods++ : mods--;

    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_HUMAN) &&
	ch->race == RACE_HUMAN)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_DWARF) &&
	ch->race == RACE_DWARF)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_ELF) &&
	ch->race == RACE_ELF);
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_HOBBIT) &&
	ch->race == RACE_HOBBIT)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_GNOME) &&
	ch->race == RACE_GNOME)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_FELIS) &&
	ch->race == RACE_FELIS)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_CANIS) &&
	ch->race == RACE_CANIS)
	mods++;
    if (IS_SET(SKILL_MODIFIERS(skill), SPLMOD_DOPPLE) &&
	ch->race == RACE_LYCANTH)
	mods++;

    chance += (mods * 5);

    return MIN(chance, 99);
}


void skill_learn(struct char_data *ch, int skill)
{
    char buf[MAX_INPUT_LENGTH];
    struct spell_info *ability;
    int x, flag = 0;
    int learn_prcnt[] = {
	0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, /* 10 */
	2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, /* 21 */
	6, 6, 6, 6
    };

    /* first check if character can use the skill */
    if (!ch->skills || !ch->skills[skill].learned)
	return;

    ability = spell_by_number(skill);
    for (x = 0; x <= MAX_LEVEL_IND && !flag; x++)
	if (GET_LEVEL(ch, x) >= ability->min_level[x])
	    flag = TRUE;

    if (!flag)
	return;

    if ((ch->skills[skill].learned < LEARNED(ability)) &&
	(ability->learn_rate > number(0, 1000)))
    {
	ch->skills[skill].learned =
	  MIN(learn_prcnt[GET_INT(ch)] + ch->skills[skill].learned,
	      LEARNED(ability));
	sprintf(buf, "$CrYou have improved in %s!$CN\n\r", ability->name);
	send_to_char_formatted(buf, ch);
    }
}

int race_att_hash (char);

#define RACE_ATT_TABLE_SIZE 13    /* The size of the hash table, a prime # */

static struct hash_header race_att_table;

/* This function creates a has table that contains the max attrributes
   available in each stat based on the characters race. */
void assign_max_race_attributes()
{
  int i;
  struct player_race_max_atts *info;

  slog ("Creating Race Attribute Hash Table");
  init_hash_table(&race_att_table, sizeof(struct player_race_max_atts),
		  RACE_ATT_TABLE_SIZE);

  slog("Initializing Hash Table with Race Values");
  for (i=0; i< MAX_PLAYER_RACE; i++)
    {
      CREATE(info, struct player_race_max_atts, 1);

      info->str_max = plyr_race_stats[i].max_str + plyr_race_mods[i].str_mod;
      info->dex_max = plyr_race_stats[i].max_dex + plyr_race_mods[i].dex_mod;
      info->con_max = plyr_race_stats[i].max_con + plyr_race_mods[i].con_mod;
      info->int_max = plyr_race_stats[i].max_int + plyr_race_mods[i].int_mod;
      info->wis_max = plyr_race_stats[i].max_wis + plyr_race_mods[i].wis_mod;
      info->cha_max = plyr_race_stats[i].max_cha + plyr_race_mods[i].cha_mod;

      hash_enter(&race_att_table, playerraces[i], (void*) info);
    }
  slog("Finished filling table");
}

struct player_race_max_atts *Find_Att_by_Race(struct char_data *ch)
{
  return (struct player_race_max_atts *) hash_find(&race_att_table, ch->race);
}

int Is_Offensive_Spell(int spellnum) {
  switch(spellnum) {
   case 2:   case 4:   case 5:   case 6:   case 7:   case 8:   case 10:
   case 17:  case 18:  case 20:  case 22:  case 23:  case 25:  case 26:
   case 27:  case 30:  case 32:  case 33:  case 34:  case 37:  case 38:
   case 41:  case 55:  case 57:  case 58:  case 59:  case 60:  case 64:
   case 66:  case 67:  case 70:  case 71:  case 72:  case 74:  case 77:
   case 78:  case 79:  case 82:  case 87:  case 106: case 109: case 110:
   case 126: case 127: case 128: case 130: case 146: case 148: case 151:
   case 154: case 157: case 159: case 160: case 161: case 170: case 171:
   case 173: case 185: case 204: case 205: case 206: case 210: case 212:
   case 213: case 325: case 327: case 330: case 331: case 332: case 334:
   case 335:
     return 1;
   default:
     return 0;
  }
}

int Is_AreaFX_Spell(int spell_num) {
   switch(spell_num) {
    case 10:  case 23:  case 32:  case 34:  case 55:  case 71:  case 72:
    case 110: case 127: case 173: case 325: case 334: case 335:
      return 1;
    default:
      return 0;
   }
}

int Is_Heal_Spell(int spell_num) {
   switch(spell_num) {
    case 15:  case 16:  case 28:  case 56:  case 83:
      return 1;
    default:
      return 0;
   }
}

/*
 * This function is responsible for making sure that a spell book
 * does not get a spell inscribed in it for more than 4 affects.
 * The book uses the obj_flags.value[4] so it is limited to 4 spells.
 * and they have to be in the first 4 affect slots.
*/
int can_inscribe(struct obj_data *book)
{
  int i = 0;

  i = getFreeAffSlot(book);
  if(i == -1 || i > 3 || (ITEM_TYPE(book) != ITEM_SPELLBOOK))
    return 0;
  else
    return 1;
}

/*
 * This does the writing of the spell into the book.  This sets
 * the quality that will get decremented depending on the value
 * in a function(UpdateBook).
*/
void inscribe_book(struct obj_data *book, int spellnum, int quality)
{
   int i = 0;
   can_inscribe(book);

   if(can_inscribe(book))
   {
      i = getFreeAffSlot(book);
      book->affected[i].location = APPLY_BOOK_SPELL;
      book->affected[i].modifier = spellnum;
      book->obj_flags.value[i] = quality;
   }
}

Variable *get_mob_var(struct char_data *ch, char *name) {
   Variable *vtmp;

   for(vtmp=ch->player.vars;vtmp;vtmp = vtmp->next)
      if(!strcasecmp(vtmp->name, name))
	 return vtmp;

   return NULL;
}

int ApplyImmuneToDamage(struct char_data *victim, int cur_damage, int immuneBit )
{
   int adjustedDamage = cur_damage;
   if (IsImmune(victim, immuneBit))
      adjustedDamage /= 4;
   else if (IsResist(victim, immuneBit))
      adjustedDamage /= 2;
   else if (IsSusc(victim, immuneBit))
      adjustedDamage *= 2;

   return adjustedDamage;
}
