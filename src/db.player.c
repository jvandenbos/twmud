
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "race.h"
#include "constants.h"
#include "handler.h"
#include "spelltab.h"
#include "channels.h"
#include "multiclass.h"
#include "util_num.h"
#include "util_str.h"
#include "utility.h"
#include "spells.h"
#include "hero.h"
#include "statistic.h"
#include <assert.h>
#include "ansi.h"
#include "comm.h"
#include "opinion.h"
#include "track.h"
#include "proto.h"
#include "spell_events.h"
#include "varfunc.h"
#include "char_create.h"

#define UCHAR_MAX 255

static long file_version;

int player_count = 0;

char wizlist[MAX_STRING_LENGTH*2];    /* the wizlist */

/* forward declarations */
void clear_char(struct char_data *ch);
DIR* open_sub_dir(char* path, char sub);
char *ClassTitles(struct char_data *ch, char* buf);
struct obj_data* object_from_data(struct char_data* ch, struct obj_data* tmpl);

int map_add_object(object_map* map, struct obj_data* obj);
void map_add_ident(object_map* map, int index, struct obj_data* obj);
struct obj_data* map_map_ident(object_map* map, int index);

int v8_deleted_skills[] = 
{
    10, 18, 20, 22, 27, 30, 32, 34, 41, 55, 56, 58, 62,
    70, 71, 74, 75, 76, 77, 78, 79, 80, 82, 84, 86, 90,
    98, 105, 115, -1
    };

#define GOD_FLAGS	(PLR_NOHASSLE|PLR_STEALTH|PLR_MASK)

/*************************************************************************
*  stuff related to the save/load player system
*********************************************************************** */

/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
    struct obj_data* obj;
    Variable *tmp;
    
    int i;

    assert(!RIDDEN(ch) &&
	   !MOUNTED(ch) &&
	   !ch->followers &&
	   !ch->master);
    
    ss_free(ch->player.title);
    ss_free(ch->player.long_descr);
    ss_free(ch->player.description);
    ss_free(ch->player.sounds);
    ss_free(ch->player.distant_snds);

    FreeHates(ch);
    FreeFears(ch);

    if(ch->aliases)
    {
	for(i = 0; i < MAX_ALIAS_SAVE; i++)
	{
	    if(ch->aliases->pattern[i])
		FREE(ch->aliases->pattern[i]);
	    if(ch->aliases->alias[i])
		FREE(ch->aliases->alias[i]);
	}
	FREE(ch->aliases->pattern[MAX_ALIAS_SAVE]);
	FREE(ch->aliases);
    }

    while(ch->player.vars) {
       tmp=ch->player.vars->next;
       delete ch->player.vars;
       ch->player.vars = tmp;
    }
   
    if(ch->skills)
	FREE(ch->skills);

    if(ch->specials.hostname)
	FREE(ch->specials.hostname);

    while((obj = ch->carrying))
      extract_obj(obj);

    for(i = 0 ; i < MAX_WEAR ; ++i)
      if((obj = ch->equipment[i]))
	extract_obj(obj);

    affect_remove_all(ch);
    
    /* leave this for last for error messages */
    ss_free(ch->player.name);
    ss_free(ch->player.short_descr);

    /* make sure we don't leave events dangling around */
    event_cancel(ch->sound_timer, 1);
   
    /* free up all of it's spell events */
    spevent_remove_all_char(ch);
   
    FREE(ch);
   
   ch = NULL;
}

/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
    int i;
  
    for (i = 0; i < MAX_WEAR; i++) /* Initializing */
	ch->equipment[i] = 0;
  
    ch->followers = 0;
    ch->master = 0;
    ch->carrying = 0;
    ch->orig = 0;

    //SET_BIT(ch->channels, COM_SWEAR); // block swear channel
    
    ch->immune = 0;
    ch->M_immune = 0;
    ch->susc = 0;
    ch->mult_att = 0;
  
    if (!GET_RACE(ch))
	GET_RACE(ch) = RACE_HUMAN;
    AFF_FLAGS(ch) = 0;
    if (GET_RACE(ch) == RACE_ELF) {
            SET_BIT(AFF_FLAGS(ch), AFF_SENSE_AURA);
    }
    if (GET_RACE(ch) == RACE_FOREST_ELF) {
            SET_BIT(AFF_FLAGS(ch), AFF_SENSE_AURA);
    }
    if (GET_RACE(ch) == RACE_DROW) {
            SET_BIT(AFF_FLAGS(ch), AFF_INFRAVISION);
    }
    if (GET_RACE(ch) == RACE_HALF_ELF) {
            SET_BIT(AFF_FLAGS(ch), AFF_SENSE_AURA);
    }
    if (GET_RACE(ch) == RACE_DWARF) {
	if (!IS_AFFECTED(ch, AFF_INFRAVISION)) 
	    SET_BIT(AFF_FLAGS(ch), AFF_INFRAVISION);
    }
    if (GET_RACE(ch) == RACE_GNOME) {
            SET_BIT(AFF_FLAGS(ch), AFF_INFRAVISION);
    }
    if (GET_RACE(ch) == RACE_PIXIE) {
            SET_BIT(AFF_FLAGS(ch), AFF_FLYING);
    }
    if (GET_RACE(ch) == RACE_AVIAN) {
	    SET_BIT(AFF_FLAGS(ch), AFF_FLYING);
    }
    if (GET_RACE(ch) == RACE_CANIS) {
            SET_BIT(AFF_FLAGS(ch), AFF_SENSE_LIFE);
    }
    if (GET_RACE(ch) == RACE_FELIS) {
            SET_BIT(AFF_FLAGS(ch), AFF_DETECT_INVISIBLE);
    }
    if (GET_RACE(ch) == RACE_REPTILE) {
            SET_BIT(AFF_FLAGS(ch), AFF_REGENERATE);
    }
    if (GET_RACE(ch) == RACE_GOBLIN) {
            SET_BIT(AFF_FLAGS(ch), AFF_INFRAVISION);
    }
    if (GET_RACE(ch) == RACE_MINOTAUR) {
            SET_BIT(AFF_FLAGS(ch), AFF_SENSE_LIFE);
    }
    if (GET_RACE(ch) == RACE_CYCLOPS) {
            SET_BIT(AFF_FLAGS(ch), AFF_TRUE_SIGHT);
    }
    if (GET_RACE(ch) == RACE_SKEXIE) {
            SET_BIT(AFF_FLAGS(ch), AFF_FLYING);
    }
    if (GET_RACE(ch) == RACE_FROST_GIANT) {
            SET_BIT(AFF2_FLAGS(ch), AFF2_COLDSHIELD);
    }
    if (GET_RACE(ch) == RACE_FIRE_GIANT) {
            SET_BIT(AFF_FLAGS(ch), AFF_FIRESHIELD);
    }
    if (GET_RACE(ch) == RACE_CLOUD_GIANT) {
            SET_BIT(AFF_FLAGS(ch), AFF_FLYING);
    }
    if (GET_RACE(ch) == RACE_STORM_GIANT) {
            SET_BIT(AFF2_FLAGS(ch), AFF2_ENERGYSHIELD);
    }

    for (i=0;i<=MAX_CLASS;i++) {
	if (GET_LEVEL(ch, i) > ABS_MAX_LVL) {
	    GET_LEVEL(ch,i) = ABS_MAX_LVL;
	}
    }

    if (TRUST(ch) > MAX_TRUST)
	ch->player.trust = 0;

    ch->hatefield = 0;
    ch->fearfield = 0;
    ch->hates.clist = 0;
    ch->fears.clist = 0;
  
    /* AC adjustment */
    GET_AC(ch) = 200;
    GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
    if (GET_AC(ch) > 200)
	GET_AC(ch) = 200;
    for(i = 0 ; i < 5 ; ++i)
	ch->specials.apply_saving_throw[i] = 0;
    
    GET_HITROLL(ch)=0;
    GET_DAMROLL(ch)=0;
  
    ch->next_fighting = 0;
    ch->next_in_room = 0;
    ch->reply = NULL;
    ch->specials.fighting = 0;
    ch->specials.position = POSITION_STANDING;
    ch->specials.default_pos = POSITION_STANDING;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items = 0;

    ch->specials.damnodice=1;
    if(ch->points.bhnum_mod)
        ch->specials.damnodice += ch->points.bhnum_mod;
    
    ch->specials.damsizedice=2;
    if(ch->points.bhsize_mod)
        ch->specials.damsizedice += ch->points.bhsize_mod;

    ch->specials.binding=0;
    ch->specials.binded_by=0;

    path_kill(ch->hunt_info);
    ch->hunt_info = 0;
    
    if (GET_HIT(ch) <= 0)
	GET_HIT(ch) = 1;
    if (GET_MOVE(ch) <= 0)
	GET_MOVE(ch) = 1;
    if (GET_MANA(ch) <= 0)
	GET_MANA(ch) = 1;
  
    /*  ch->points.max_mana = 0; */
    ch->points.max_move = 0;
  
    if (IS_IMMORTAL(ch)) {
	GET_BANK(ch) = 0;
	GET_GOLD(ch) = 100000;
    }
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
    memset(ch, '\0', sizeof(struct char_data));

    ch->in_room = NOWHERE;
    ch->specials.was_in_room = NOWHERE;
    ch->specials.position = POSITION_STANDING;
    ch->specials.default_pos = POSITION_STANDING;
    GET_AC(ch) = 200;		/* Basic Armor */
    ch->mpact = NULL;
    ch->mpactnum = 0;
    ch->player.vars = NULL;
    //SET_BIT(ch->channels, COM_SWEAR); // block swear channel as default
}

void clear_aliases(struct char_data *ch)
{
    int i;

    CREATE(ch->aliases, struct alias_data, 1);
    for(i = 0; i < MAX_ALIAS_SAVE; i++)
    {
	ch->aliases->pattern[i] = strdup("!");
	ch->aliases->alias[i]   = NULL;
    }
    ch->aliases->pattern[MAX_ALIAS_SAVE] = strdup("\n");
}


int putbyte(char byte, FILE* fp)
{
    return fwrite(&byte, sizeof(byte), 1, fp) != 1;
}

int getbyte(char* byte, FILE* fp)
{
    return fread(byte, sizeof(*byte), 1, fp) != 1;
}

int getubyte(unsigned char* byte, FILE* fp)
{
    return fread(byte, sizeof(*byte), 1, fp) != 1;
}

int putbytearray(char* byte, size_t len, FILE* fp)
{
    return fwrite(byte, sizeof(*byte), len, fp) != len;
}

int putubytearray(unsigned char* byte, size_t len, FILE* fp)
{
    return fwrite(byte, sizeof(*byte), len, fp) != len;
}

int getbytearray(char* byte, size_t len, FILE* fp)
{
    return fread(byte, sizeof(*byte), len, fp) != len;
}

int getubytearray(unsigned char* byte, size_t len, FILE* fp)
{
    return fread(byte, sizeof(*byte), len, fp) != len;
}

int putword(short word, FILE* fp)
{
    return fwrite(&word, sizeof(word), 1, fp) != 1;
}

int getword(short* word, FILE* fp)
{
    return fread(word, sizeof(*word), 1, fp) != 1;
}

int putwordarray(short* array, size_t len, FILE* fp)
{
    return fwrite(array, sizeof(*array), len, fp) != len;
}

int getwordarray(short* array, size_t len, FILE* fp)
{
    return fread(array, sizeof(*array), len, fp) != len;
}

int putdword(long word, FILE* fp)
{
    return fwrite(&word, sizeof(word), 1, fp) != 1;
}

int getdword(long* word, FILE* fp)
{
    return fread(word, sizeof(*word), 1, fp) != 1;
}

int putsstring(const sstring_t* string, FILE* fp)
{
    size_t len;
    
    if(!string)
	return putbyte(0, fp);

    len = strlen(ss_data(string));
    
    if (len >= UCHAR_MAX)
    {
	if (putbyte(UCHAR_MAX, fp) || putword(len, fp))
	    return 1;
    }
    else if (putbyte(len, fp))
	return 1;

    return (fwrite(ss_data(string), sizeof(char), len, fp) != len);
}

int getsstring(sstring_t** string, FILE* fp)
{
    unsigned short len = 0;
    char	buf[MAX_STRING_LENGTH];
    
    if(getbyte((char*) &len, fp) ||
       (len == UCHAR_MAX && getword((short *) &len, fp)))
	return 1;

    if(fread(buf, 1, len, fp) != len)
	return 1;
    buf[len] = 0;
    
    *string = ss_make(buf);

    return 0;
}

    
int putstring(const char* string, FILE* fp)
{
    size_t	len;

    if(!string)
	return putbyte(0, fp);
    
    len = strlen(string);
    
    return (putbyte(len, fp) ||
	    (fwrite(string, sizeof(char), len, fp) != len));
}

int getstring(char** string, FILE* fp)
{
    unsigned char	len;
    
    if(getbyte((char*) &len, fp))
	return 1;

    if(len == 0)
    {
	*string = 0;
	return 0;
    }
    
    CREATE(*string, char, len + 1);

    (*string)[len] = 0;
    
    return fread(*string, sizeof(char), len, fp) != len;
}

int putabilities(struct char_ability_data* abil, FILE* fp)
{
    return (putbyte(abil->str, fp) ||
	    putbyte(abil->str_add, fp) ||
	    putbyte(abil->intel, fp) ||
	    putbyte(abil->wis, fp) ||
	    putbyte(abil->dex, fp) ||
	    putbyte(abil->con, fp) ||
	    putbyte(abil->cha, fp));
}

int getabilities(struct char_ability_data* abil, FILE* fp)
{
  int ret;
  sbyte data;
  
  abil->cha = dice(3, 6);
  
  ret = (getbyte(&abil->str, fp) ||
	 getbyte(&data, fp) ||
	 getbyte(&abil->intel, fp) ||
	 getbyte(&abil->wis, fp) ||
	 getbyte(&abil->dex, fp) ||
	 getbyte(&abil->con, fp) ||
	 (file_version > 18 && getbyte(&abil->cha, fp)));

  abil->str_add = data;

  return ret;
}

int putskills(struct char_skill_data *p, FILE* fp)
{
    int		i, cnt = 0;

    if(p)
	for(i = 0 ; i < MAX_SKILLS ; ++i)
	    if(p[i].learned || p[i].recognise)
		cnt = i + 1;

    if(putword(cnt, fp))
	return 1;

    for(i = 0 ; i < cnt ; ++i, p++)
	if(putbyte(p->learned, fp) || putbyte(p->recognise, fp))
	    return 1;
    
    return 0;
}

int getskills(struct char_data* ch, FILE* fp)
{
    int			i, check, x;
    unsigned char old_size;
    ush_int	size;
    struct char_skill_data *p = ch->skills;

    if (file_version < 17)
    {
      if(getbyte((char*) &old_size, fp))
	return 1;
      size=old_size;
    }
    else
      if(getword((short *) &size, fp))
	return 1;

    if(file_version < 1)
	size = MAX_SKILLS;

    for(i = 0 ; i < size ; ++i, p++)
       if(getbyte((char*) &p->learned, fp) ||
	  getbyte((char *) &p->recognise, fp))
	  return 1;

    while(i++ < MAX_SKILLS)
    {
	p->learned = 0;
	p->recognise = 0;
	p++;
    }

    if(file_version < 8)
    {
	int *ptr;
	int pracs = ch->specials.spells_to_learn;
	
	for(ptr = v8_deleted_skills ; *ptr > 0 ; ptr++)
	{
	    pracs += ch->skills[*ptr].learned / 20;
	    ch->skills[*ptr].learned = 0;
	}

	if(pracs > 120)
	    pracs = 120;

	ch->specials.spells_to_learn = pracs;
    }

    if(spell_count && !IS_GOD(ch))
    {
	for(i = 0, p = ch->skills ; i < MAX_SKILLS ; ++i, ++p)
	{
	    struct spell_info*	info;
	    int			max;
	    
	    if((info = spell_by_number(i)))
	    {
		max = (info->targets & TAR_SKILL) ? 90 : 95;
		max = MIN(max, p->learned);
		if((max != p->learned))
		{
		    char buf[256];
		    
		    sprintf(buf, "%s skill has illegal value: %s (%d)",
			    GET_IDENT(ch), info->name, p->learned);
		    log_msg(buf);
		    p->learned = max;
		}
	    }
	    else if(p->learned)
	    {
		char buf[256];
		sprintf(buf, "%s knows non-existent skill: %d, %d",
			GET_IDENT(ch), i, p->learned);
		log_msg(buf);
		p->learned = 0;
	    }

	    if (!HasClass(ch, CLASS_MONK) && !HasClass(ch, CLASS_BARD))
	      /* these two are allowed to have skills outside their class */
	    {
	      if (info && p->learned) {
                for (x=0, check=FALSE; x<=MAX_LEVEL_IND && !check; x++) {
		  if (ch->player.max_level[x] >= info->min_level[x] ||
		      GET_LEVEL(ch, x) >= info->min_level[x])
                    check=TRUE;
                }
                if (!check) {
		  char buf[256];
		  sprintf (buf, "%s has skill outside of class: %s",
			   GET_IDENT(ch), info->name);
		  log_msg(buf);
		  p->learned=0;
                }
	      }
	    }
	}
    }	    

    return 0;
}

int putaffects(struct affected_type *aff, FILE* fp)
{
    for(; aff ; aff = aff->next)
    {
	if(aff->mana_cost)		/* don't save mana costing spells */
	    continue;
	
	if(putbyte(1, fp) ||
	   putword(aff->type, fp) ||
	   putword(aff->duration, fp) ||
	   putdword(aff->modifier, fp) ||
	   putbyte(aff->location, fp) ||
	   putdword(aff->bitvector, fp))
	    return 1;
    }
    return putbyte(0, fp);
}

int getaffects(struct char_data* ch, FILE* fp)
{
    struct affected_type	aff;
    char valid;
    
    while(1)
    {
	if(getbyte(&valid, fp))
	    return 1;
	if(!valid)
	    return 0;

	if(getword(&aff.type, fp) ||
	   getword(&aff.duration, fp) ||
	   ((file_version <= 10) ?
	    getbyte(&valid, fp) :
	    getdword(&aff.modifier, fp)) ||
	   getbyte(&aff.location, fp) ||
	   getdword(&aff.bitvector, fp))
	    return 1;

	if(file_version <= 10)
	    aff.modifier = valid & 0xff;
	
	aff.mana_cost = 0;
	aff.caster = 0;
	aff.save_bonus = 0;
	aff.expire_proc_pointer = 0;
	
	if(((file_version != 6) || (aff.location != 100)) && aff.duration)
	{
	  DLOG(("Calling affect_to_char from getaffects. db.player.c line 578\r\n"));
	  affect_to_char(ch, &aff);
	}
    }
}

int putpoints(struct char_point_data* points, FILE* fp)
{
    if(putword(points->mana, fp) ||
       putword(points->max_mana, fp) ||
       putdword(points->hit, fp) ||
       putdword(points->max_hit, fp) ||
       putword(points->move, fp) ||
       putword(points->max_move, fp) ||
       putword(points->armor, fp) ||
       putdword(points->gold, fp) ||
       putdword(points->bankgold, fp) ||
       putdword(points->exp, fp) ||
       putword(points->hitroll, fp) ||
       putword(points->damroll, fp) ||
       putdword(points->bhsize_mod, fp) ||
       putdword(points->bhnum_mod, fp) ||
       putdword(points->hero_points, fp))
      return 1;
    return 0;
}

int getpoints(struct char_point_data* points, FILE* fp, int version)
{
  if(getword(&points->mana, fp) ||
     getword(&points->max_mana, fp) ||
     (((version < 30) &&
       (getword((sh_int*)&points->hit, fp) ||
	getword((sh_int*)&points->max_hit, fp))) ||
      ((version >= 30) &&
       (getdword((long*)&points->hit, fp) ||
	getdword((long*)&points->max_hit, fp)))) ||
     getword(&points->move, fp) ||
     getword(&points->max_move, fp) ||
     getword(&points->armor, fp) ||
     getdword((long*) &points->gold, fp) ||
     getdword((long*) &points->bankgold, fp) ||
     getdword((long*)&points->exp, fp) ||
     (((version < 30) &&
       (getbyte((sbyte*)&points->hitroll, fp) ||
	getbyte((sbyte*)&points->damroll, fp)) ||
       ((version >= 30) &&
	(getword(&points->hitroll, fp) ||
	 getword(&points->damroll, fp))) ||
       ((version > 31) &&
	( getdword((long*) &points->bhsize_mod, fp) ||
	  getdword((long*) &points->bhnum_mod, fp))) ||
       ((version > 32) && 
	( getdword((long*) &points->hero_points, fp)	
	  )))))
      return 1;
   
    return 0;
}

int putobject(struct obj_data* obj, int pos, int own,
	      object_map* map, FILE* fp, FILE* eqfp)
{
    int i;
    int weight;
    int me;

    if(obj->obj_flags.cost_per_day >= 0)
    {
        if(eqfp) fprintf(eqfp, "%-35s - %s\n", OBJ_SHORT(obj), OBJ_NAME(obj));
       
	weight = obj->obj_flags.weight;

	me = map_add_object(map, obj);
    
	if(putword(obj_index[obj->item_number].virt, fp) ||
	   putbyte(obj->obj_flags.type_flag, fp) ||
	   putdword(obj->obj_flags.wear_flags, fp) ||
	   putdword(obj->obj_flags.value[0], fp) ||
	   putdword(obj->obj_flags.value[1], fp) ||
	   putdword(obj->obj_flags.value[2], fp) ||
	   putdword(obj->obj_flags.value[3], fp) ||
	   putdword(obj->obj_flags.extra_flags, fp) ||
	   putdword(weight, fp) ||
	   putdword(obj->obj_flags.timer, fp) ||
	   putdword(obj->obj_flags.cost_per_day, fp) ||
	   putdword(obj->obj_flags.level, fp) ||
	   putdword(obj->obj_flags.durability, fp) ||
	   putdword(obj->obj_flags.encumbrance, fp) ||
	   putdword(obj->obj_flags.no_loot, fp) ||
	   putsstring(obj->name, fp) ||
	   putsstring(obj->short_description, fp) ||
	   putsstring(obj->description, fp) ||
	   putsstring(obj->action_description, fp) ||
	   putword(pos, fp) ||
	   putword(own, fp) ||
	   putword(me, fp))
	    return 1;

	for(i = 0 ; i < MAX_OBJ_AFFECT ; ++i)
	{
	    if(putword(obj->affected[i].location, fp) ||
	       putdword(obj->affected[i].modifier, fp))
		return 1;
	}
       
        if(obj->contains && eqfp) fprintf(eqfp, "\n --Contains:\n");

	if(obj->contains && putobject(obj->contains, -1, me, map, fp, eqfp))
	  return 1;
       
        if(obj->contains && eqfp) fprintf(eqfp, "\n");
    }

    if(obj->next_content && putobject(obj->next_content, -1, own, map, fp, eqfp))
	return 1;
    
    return 0;
}

int getobject(struct obj_data** objP, struct obj_data* cont,
	      object_map* map, struct char_data* ch, FILE* fp, 
	      int fv, int room)
{
    int i;
    unsigned char flags;
    struct obj_data* obj;
    struct obj_data* obj2;
    short pos, own, me;
    long ljunk;
    struct obj_data tmpl;
    
    memset(&tmpl, 0, sizeof(tmpl));
    
    if(getword(&tmpl.item_number, fp) ||
       ((fv > 0) && getbyte(&tmpl.obj_flags.type_flag, fp)) ||
       getdword((long*) &tmpl.obj_flags.wear_flags, fp) ||
       getdword((long*) &tmpl.obj_flags.value[0], fp) ||
       getdword((long*) &tmpl.obj_flags.value[1], fp) ||
       getdword((long*) &tmpl.obj_flags.value[2], fp) ||
       getdword((long*) &tmpl.obj_flags.value[3], fp) ||
       getdword((long*) &tmpl.obj_flags.extra_flags, fp) ||
       getdword((long*) &tmpl.obj_flags.weight, fp) ||
       getdword((long*) &tmpl.obj_flags.timer, fp) ||
       ((fv < 8) && getdword(&ljunk, fp)) ||
       ((fv > 1) && getdword((long *) &tmpl.obj_flags.cost_per_day, fp)) ||
       ((fv > 22) && getdword((long *) &tmpl.obj_flags.level, fp)) ||
       ((fv > 22) && getdword((long *) &tmpl.obj_flags.durability, fp)) ||
       ((fv > 22) && getdword((long *) &tmpl.obj_flags.encumbrance, fp)) ||
       ((fv > 25) && getdword((long *) &tmpl.obj_flags.no_loot, fp)) ||
       getsstring(&tmpl.name, fp) ||
       getsstring(&tmpl.short_description, fp) ||
       getsstring(&tmpl.description, fp) ||
       ((fv > 12) && getsstring(&tmpl.action_description, fp)) ||
       ((fv > 4) &&
	(getword(&pos, fp) ||
	 getword(&own, fp) ||
	 getword(&me, fp))))
	return 1;

    if((tmpl.item_number == -1) && (fv < 2))
	tmpl.obj_flags.weight = 1;
    
    for(i = 0 ; i < MAX_OBJ_AFFECT ; ++i)
    {
	if(getword(&tmpl.affected[i].location, fp) ||
	   getdword((long*) &tmpl.affected[i].modifier, fp))
	    return 1;
    }

    if(!tmpl.name)
	tmpl.name = ss_make("Miscellaneous trash");
    if(!tmpl.short_description)
	tmpl.short_description = ss_make("Some trash that should be pitched");
    if(!tmpl.description)
	tmpl.description =
	    ss_make("If a god sees this, confiscate it immediately\n");

    obj = object_from_data(ch, &tmpl);
    
    obj->eq_pos = -1;
    obj->next_content = 0;
    obj->contains = 0;

    if(fv <= 4)
    {
	if(cont)
	    obj_to_obj(obj, cont);

	if(getbyte((char*) &flags, fp))
	    return 1;
    
	if((flags & 1) && getobject(&obj2, obj, map, ch, fp, fv, 0))
	    return 1;

	if(flags & 2)
	{
	    if(getobject(&obj2, cont, map, ch, fp, fv, 0))
		return 1;
	    if(!cont)
		obj->next_content = obj2;
	}
    }
    else
    {
      map_add_ident(map, me, obj);
      if(pos != -1)
      {
	if (ch) {
	  if ((pos == WIELD || pos == HOLD) && !(obj->obj_flags.wear_flags & 
						 (ITEM_WIELD | ITEM_HOLD))) {
	    obj_to_char(obj, ch);
	  } else {
	    raw_equip(ch, obj, pos);
	  }
	} else 
	  obj_to_room(obj, room);
      }
      else if(own && (cont = map_map_ident(map, own)))
	obj_to_obj(obj, cont);
      else
      {
	if (ch)
	  obj_to_char(obj, ch);
	else
	  obj_to_room(obj, room);
      }
    }
    
    *objP = obj;
    
    return 0;
}

int putobjects(struct char_data* ch, FILE* fp, FILE *eqfp)
{
    int i;
    object_map*	map;
    long start, end;

    start = ftell(fp);
    
    putword(0, fp);
    
    map = new_object_map(MAX_OBJ_SAVE);
    
    if(eqfp) fprintf(eqfp, " --Equipment:\n");

    for(i = 0 ; i < MAX_WEAR ; ++i)
    {
	if(ch->equipment[i]) {
            if(eqfp && (i == WIELD)) fprintf(eqfp, "WIELD: ");
	    if(eqfp && (i == HOLD))  fprintf(eqfp, "HOLD: ");
            
	    if(putobject(ch->equipment[i], i, 0, map, fp, eqfp))
	    {
		kill_object_map(map);
		return 1;
	    }
	}
    }

    if(eqfp) fprintf(eqfp, "\n --Carrying:\n");
    if(ch->carrying)
	if(putobject(ch->carrying, -1, 0, map, fp, eqfp))
	{
	    kill_object_map(map);
	    return 1;
	}

    end = ftell(fp);
    fseek(fp, start, 0);
    
    putword(map->count, fp);

    fseek(fp, end, 0);
    
    kill_object_map(map);

    return 0;
}

int getobjects(struct char_data* ch, FILE* fp)
{
    object_map*	map;
    short	count;
    short	i;
    struct obj_data* obj;

    FILE *logfile; /* for logging purposes */
    
    if(getword(&count, fp))
	return 1;

    map = new_object_map(MAX_OBJ_SAVE);

    for(i = 0 ; i < count ; ++i)
    {
	if(getobject(&obj, 0, map, ch, fp, file_version, 0))
	    return 1;
	
	logfile = fopen("OBJECTSBYPLAYER.LOG","a+");
	fprintf(logfile,"%-15s %-40s %-7d\n",ss_data(ch->player.name),ss_data(obj->name),obj->item_number);
	fclose(logfile);

	logfile = fopen("OBJECTSBYOBJECT.LOG","a+");
	fprintf(logfile,"%-40s    %-15s\n",ss_data(obj->name),ss_data(ch->player.name));
	fclose(logfile);

	logfile = fopen("OBJECTSBYSITE.LOG","a+");
	fprintf(logfile,"%s, %s(%s)\n",ss_data(obj->name),ch->specials.hostname,ss_data(ch->player.name));
	fclose(logfile);

    }

    
    
    kill_object_map(map);

    return 0;
}

 
int getobjects_v4(struct char_data* ch, FILE* fp)
{
    unsigned char pos;
    struct obj_data* obj;
    struct obj_data* next;

    for(pos = 0 ; pos != 0xff ;)
    {
	if(getbyte((char*) &pos, fp))
	    return 1;
	if(pos == 127)
	{
	    if(getobject(&obj, 0, 0, ch, fp, file_version, 0))
		return 1;
	    for( ; obj ; obj = next)
	    {
		next = obj->next_content;
		obj->next_content = 0;
		obj_to_char(obj, ch);
	    }
	}
	else if(pos < MAX_WEAR)
	{
	    if(getobject(&obj, 0, 0, ch, fp, file_version, 0))
		return 1;
	    raw_equip(ch, obj, pos);
	}
	else if(pos != 0xff)
	{
	    fprintf(stderr, "illegal wear position: %d\n", pos);
	    return 1;
	}
    }

    return 0;
}

int putcorpse(struct obj_data *obj, room_num roomn, FILE* fp)
{
    int i;
    object_map*	map;
    long start, end;

    start = ftell(fp);
    
    putword(0, fp);
    putword(PLAYER_VERSION, fp);
    putdword(roomn, fp);
    putsstring(obj->char_name, fp);

    map = new_object_map(MAX_OBJ_SAVE);
    
    obj->obj_flags.cost_per_day = 1;
    i = obj->obj_flags.value[2];
    obj->obj_flags.value[2] = i;
   
    if(putobject(obj, -1, 0, map, fp, NULL)) {
	kill_object_map(map);
	return 1;
    }
    obj->obj_flags.cost_per_day = -1;
    obj->obj_flags.value[2] = i;
   
    end = ftell(fp);
    fseek(fp, start, 0);
    
    putword(map->count, fp);

    fseek(fp, end, 0);
    
    kill_object_map(map);

    return 0;
}

int getcorpse(FILE* fp)
{
    object_map*	map;
    short	count;
    short	i;
    int		roomn;
    sstring_t*	cname;
    struct obj_data* obj;

    if(getword(&count, fp) ||
       getword((short *) &file_version, fp) ||
       getdword((long *) &roomn, fp) ||
       getsstring(&cname, fp))
	return 1;

    map = new_object_map(MAX_OBJ_SAVE);

    roomn=(real_roomp(roomn))?roomn:RM_MORGUE_HOLDING;

    for(i = 0 ; i < count ; ++i)
    {
	if(getobject(&obj, 0, map, 0, fp, file_version, roomn))
	    return 1;
    }
    obj->obj_flags.cost_per_day = -1;

    if(obj->char_name != cname) {
	ss_free(obj->char_name);
	obj->char_name = cname;
    } else
	ss_free(cname);

    kill_object_map(map);

    return 0;
}

int savecorpse(struct obj_data* obj, room_num roomn)
{
    FILE* savefile;
    char buf[80];

    sprintf(buf, "Saving player corpse: %s", ss_data(obj->short_description));
    log_msg(buf);

    savefile = fopen("corpsedata.dat", "w+");

    if(putcorpse(obj, roomn, savefile)) {
	log_msg("Error in saving corpse to file!");
	fclose(savefile);
	return 1;
    }

    fclose(savefile);

    return 0;
}

int loadcorpse()
{
    FILE* savefile;

    savefile = fopen("corpsedata.dat", "r");

    if(!savefile) {
	log_msg("No corpse file present... corpses not loaded.");
	return 1;
    }

    if(getcorpse(savefile)) {
        log_msg("Error in loading corpse from file!");
	fclose(savefile);
        return 1;
    }

    fclose(savefile);

//  This was commented out in case of multiple crashes. It'll be deleted
//  -anyways-, if the person 'gets' anything from the corpse. (Quilan)

//    remove("corpsedata.dat");

    return 0;
}

int putaliases(struct char_data* ch, FILE* fp)
{
    int	i, cnt = 0;
    
    if(ch->aliases)
	for(i = 0 ; i < MAX_ALIAS_SAVE ; ++i)
	    if(ch->aliases->pattern[cnt] &&
	       strcmp(ch->aliases->pattern[cnt], "!"))
		cnt++;

    if(putbyte(cnt, fp))
	return 1;
    
    if(cnt)
	for(i = 0 ; i < MAX_ALIAS_SAVE ; ++i)
	{
	    if(ch->aliases->pattern[cnt] &&
	       strcmp(ch->aliases->pattern[i], "!"))
	    {
		if(putstring(ch->aliases->pattern[i], fp) ||
		   putstring(ch->aliases->alias[i], fp))
		    return 1;
	    }
	}

    return 0;
}

int getaliases(struct char_data* ch, FILE* fp)
{
    int i;
    unsigned char cnt;

    if(getbyte((char*) &cnt, fp))
	return 1;

    if(!ch->aliases)
	clear_aliases(ch);
    
    if(cnt > MAX_ALIAS_SAVE)
    {
	fprintf(stderr, "Too many aliases restored: %d\n", cnt);
	cnt = MAX_ALIAS_SAVE;
    }
    
    for(i = 0 ; i < cnt ; ++i)
    {
      if(ch->aliases->pattern[i])
	FREE(ch->aliases->pattern[i]);
      if(ch->aliases->alias[i])
	FREE(ch->aliases->alias[i]);

      if(getstring(&ch->aliases->pattern[i], fp) ||
	 getstring(&ch->aliases->alias[i], fp)) {
	ch->aliases->pattern[i] = strdup("!");
	return 1;
      }
    }
    
    return 0;
}

/* new single file code */
int write_player(struct char_data* ch, FILE* fp, FILE *eqfp)
{
    int	now, i, status;
    struct affected_type* af;
    struct obj_data* char_eq[MAX_WEAR];
    long directory_off = 0, player_off = 0, objects_off, aliases_off;
    
    now = time(0);
    ch->player.time.played += (now - ch->player.time.logon);
    ch->player.time.logon = now;

    /* unwear everything to avoid misleading saves.  Otherwise
       we could wind up with things doubling effect */
    for(i=0; i<MAX_WEAR; i++) {
	if (ch->equipment[i])
	    char_eq[i] = unequip_char(ch, i);
	else
	    char_eq[i] = 0;
    }

    /* likewise, remove all spell affects */
    /* subtract effect of the spell or the effect will be doubled */
    for(af = ch->affected ; af ; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    status = 0;
    
    if(putdword(PLAYER_MAGIC, fp) ||
       putdword(PLAYER_VERSION, fp))
    {
	status = 8;
	goto wrapup;
    }

    directory_off = ftell(fp);
    if(putdword(0, fp) ||		/* offset of player */
       putdword(0, fp) ||		/* offset of objects */
       putdword(0, fp))			/* offset of aliases */
       {
	   status = 9;
	   goto wrapup;
       }
    
    player_off = ftell(fp);
    if(putdword(MAX_TONGUE + 1, fp) ||
       putbytearray((char *) ch->player.talks, MAX_TONGUE + 1, fp) ||
       putdword(ch->rent_cost, fp) || /* place holder for rent per day */
       putsstring(ch->player.name, fp) ||
       putsstring(ch->player.short_descr, fp) ||
       putsstring(ch->player.description, fp) ||
       putsstring(ch->player.title, fp) ||
       putstring(ch->pwd, fp) ||
       putdword(ch->player.time.birth, fp) ||
       putdword(ch->player.time.played, fp) ||
       putdword(ch->player.time.logon, fp) ||
       putdword(GET_HOME(ch), fp) ||
       putdword(ch->in_room, fp) ||
       putbyte(GET_WEIGHT(ch), fp) ||
       putbyte(GET_HEIGHT(ch), fp) ||
       putbyte(GET_SEX(ch), fp) ||
       putword(ch->player.clss, fp) ||
       putubytearray(ch->player.level, MAX_CLASS, fp) ||
       putubytearray(ch->player.max_level, MAX_CLASS, fp) ||
       putword(GET_RACE(ch), fp) ||
       putabilities(&ch->abilities, fp) ||
       putpoints(&ch->points, fp) ||
       putdword(ch->specials.alignment, fp) ||
       putbyte(ch->specials.spells_to_learn, fp) ||
       putdword(ch->specials.flags, fp) ||
       putbyte(ch->invis_level, fp) ||
       putbyte(ch->player.trust, fp) ||
       putskills(ch->skills, fp) ||
       putwordarray(ch->specials.apply_saving_throw, 5, fp) ||
       putbytearray(ch->specials.conditions, 3, fp) ||
       putaffects(ch->affected, fp) ||
       putbyte(ch->specials.pmask, fp) ||
       putstring(ch->specials.poofin, fp) ||
       putstring(ch->specials.poofout, fp) ||
       putword(ch->delete_flag, fp) ||
       putstring(ch->specials.prompt, fp) ||
       putbyte(GET_GUILD(ch), fp) ||
       putbyte(GET_GUILD_LEVEL(ch), fp) ||
       putdword(ch->channels, fp) ||
       putstring(ch->specials.hostname, fp) ||
       putbyte(ch->res_info.valid, fp) ||
       (ch->res_info.valid &&
	(putbyte(ch->res_info.clss, fp) ||
	 putbyte(ch->res_info.level, fp) ||
	 putdword(ch->res_info.exp, fp))) ||
       putword(ch->specials.wimpy, fp) ||
       putbyte(ch->player.mother, fp) ||
       putbyte(ch->player.father, fp) ||
       putdword(ch->player.upbringing, fp) || 
       putbyte(ch->player.age12, fp) ||
       putbyte(ch->player.age13, fp) ||
       putbyte(ch->player.age14, fp) ||
       putdword(ch->player.guildinfo.guildnumber, fp) || /* write the players guild info - MIN */
       putdword(ch->player.guildinfo.guildflags, fp) || 
       putdword(ch->player.guildinfo.spare, fp) ||
	  putdword(ch->player.godinfo.position, fp) ||
	  putdword(ch->player.godinfo.cmdset, fp) ||
       putdword(ch->log_flags, fp) ||
       putdword(ch->player.pkillinfo.flags, fp) ||
       putdword(ch->player.pkillinfo.count, fp) ||
       putdword(ch->player.pkillinfo.killed, fp) ||
       putdword(ch->player.hpsystem, fp) ||
       putword(ch->drop_count, fp)
       )
	status = 10;

 wrapup:
    /* restore the affects */
    for(af = ch->affected ; af ; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    /* and the equipment */
    for(i=0; i<MAX_WEAR; i++)
    {
	if (char_eq[i])
	    raw_equip(ch, char_eq[i], i);
    }

    if(status)
	return status;
    
    objects_off = ftell(fp);
    if(putobjects(ch, fp, eqfp))
	return 11;

    aliases_off = ftell(fp);
    if(putaliases(ch, fp))
	return 12;

    fflush(fp);
    
    if(ferror(fp) || fseek(fp, directory_off, 0) ||
       putdword(player_off, fp) ||
       putdword(objects_off, fp) ||
       putdword(aliases_off, fp))
	return 13;
    
    if(ferror(fp))
	status = 14;
    
    return status;
}

int read_player(struct char_data* ch, FILE* fp, int flags)
{
  int	status;
  long	player_off, objects_off, aliases_off;
  long	magic;
  int tongues;
  int i;
  FILE *logfile; /* using for testing */
   
  status = 0;

  if(getdword(&magic, fp) ||
     getdword(&file_version, fp) ||
     getdword(&player_off, fp) ||
     getdword(&objects_off, fp) ||
     getdword(&aliases_off, fp))
    return 8;

  if((magic != PLAYER_MAGIC) || (file_version > PLAYER_VERSION))
    return 15;

  ch->file_version = file_version;
    
  reset_char(ch);
    
  if(flags & READ_PLAYER)
  {
    if(fseek(fp, player_off, 0))
      return 9;

    ch->in_room = NOWHERE;

    if((file_version >= 7) && (file_version < 28))
      tongues = 4;
    else
      tongues = 3;

    if(((file_version > 27) && getdword((long*) &tongues, fp)) ||
       getbytearray((char *) ch->player.talks, tongues, fp) ||
       getdword((long*) &ch->rent_cost, fp) ||
       getsstring(&ch->player.name, fp) ||
       getsstring(&ch->player.short_descr, fp) ||
       getsstring(&ch->player.description, fp) ||
       getsstring(&ch->player.title, fp) ||
       getstring(&ch->pwd, fp) ||
       getdword((long*) &ch->player.time.birth, fp) ||
       getdword((long*) &ch->player.time.played, fp) ||
       getdword((long*) &ch->player.time.logon, fp) ||
       getdword((long*) &GET_HOME(ch), fp) ||
       ((file_version > 2) && getdword((long*) &ch->in_room, fp)) ||
       getbyte((char*) &GET_WEIGHT(ch), fp) ||
       getbyte((char*) &GET_HEIGHT(ch), fp) ||
       getbyte((char*) &GET_SEX(ch), fp) ||
       getword((short*) &ch->player.clss, fp) ||
       getubytearray(ch->player.level, MAX_CLASS, fp) ||
       ((file_version > 11) &&
	getubytearray(ch->player.max_level, MAX_CLASS, fp)) ||
       UpdateMaxLevel(ch) ||
       UpdateMinLevel(ch) ||
       getword(&GET_RACE(ch), fp) ||
       getabilities(&ch->abilities, fp) ||
       getpoints(&ch->points, fp, file_version) ||
       getdword((long*) &ch->specials.alignment, fp) ||
       getbyte((char*) &ch->specials.spells_to_learn, fp) ||
       getdword((long*) &ch->specials.flags, fp) ||
       getbyte(&ch->invis_level, fp) ||
       ((file_version > 20) && getbyte(&ch->player.trust, fp)) ||
       getskills(ch, fp) ||
       getwordarray(ch->specials.apply_saving_throw, 5, fp) ||
       getbytearray(ch->specials.conditions, 3, fp) ||
       getaffects(ch, fp) ||
       getbyte(&ch->specials.pmask, fp) ||
       getstring(&ch->specials.poofin, fp) ||
       getstring(&ch->specials.poofout, fp) ||
       getword(&ch->delete_flag, fp) ||
       getstring((char**) &ch->specials.prompt, fp) ||
       getbyte((char*)&(GET_GUILD(ch)), fp) ||
       getbyte((char*) &(GET_GUILD_LEVEL(ch)), fp) ||
       ((file_version > 3) && getdword((long*) &ch->channels, fp)) ||
       ((file_version > 9) && getstring(&ch->specials.hostname, fp)) ||
       ((file_version > 11) && getbyte(&ch->res_info.valid, fp)) ||
       (ch->res_info.valid &&
	(getbyte(&ch->res_info.clss, fp) ||
	 getubyte(&ch->res_info.level, fp) ||
	 getdword(&ch->res_info.exp, fp))) ||
       ((file_version > 15) &&
	getword(&ch->specials.wimpy, fp)) ||
       ((file_version > 17) &&
        (getbyte(&ch->player.mother, fp) ||
	 getbyte(&ch->player.father, fp) ||
	 getdword((long*) &ch->player.upbringing, fp) || 
         getbyte(&ch->player.age12, fp) ||
         getbyte(&ch->player.age13, fp) ||
         getbyte(&ch->player.age14, fp))) ||
         ((file_version > 21) && getdword((long *) &ch->player.guildinfo.guildnumber, fp)) || 
         ((file_version > 21) && getdword((long *) &ch->player.guildinfo.guildflags, fp)) ||
         ((file_version > 21) && getdword((long *) &ch->player.guildinfo.spare, fp)) ||
	    ((file_version > 23) && getdword((long *) &ch->player.godinfo.position,fp)) ||
	    ((file_version > 23) && getdword((long *) &ch->player.godinfo.cmdset,fp)) ||
         ((file_version > 24) && getdword((long *) &ch->log_flags, fp)) ||
	 ((file_version > 25) && getdword((long *) &ch->player.pkillinfo.flags, fp)) ||
	 ((file_version > 25) && getdword((long *) &ch->player.pkillinfo.count, fp)) ||
	 ((file_version > 25) && getdword((long *) &ch->player.pkillinfo.killed, fp)) ||
         ((file_version > 26) && getdword((long *) &ch->player.hpsystem, fp)) ||
       ((file_version == 20) && getbyte(&ch->player.trust, fp)) ||
       ((file_version > 30) && getword((short *)&ch->drop_count, fp))
       )
    return 10;
    

/*    if((file_version <= 2) && !IS_GOD(ch))
    {
      GET_HOME(ch) = 3001;
      ch->in_room = 3001;
    }

    if(file_version <= 3)
    {
      if(IS_SET(ch->specials.flags, PLR_NOSHOUT))
	ch->channels |= COM_SHOUT | COM_AUCTION | COM_RUMOR | COM_LOG;
      if(IS_SET(ch->specials.flags, 1<<8)) // PLR_DEAF 
	ch->channels |= COM_SHOUT | COM_AUCTION | COM_RUMOR;
      if(IS_SET(ch->specials.flags, 1<<15)) // PLR_NOLOG 
	ch->channels |= COM_LOG;
      if(IS_SET(ch->specials.flags, 1<<18)) // PLR_AUCTION 
	ch->channels &= ~COM_AUCTION;
    }

    if(file_version < 10)
      ch->specials.flags &=
	(PLR_BRIEF|PLR_COMPACT|PLR_NOHASSLE|PLR_STEALTH|
	 PLR_WIMPY|PLR_ECHO|PLR_DISPLAY|PLR_AGGR|PLR_COLOR|
	 PLR_MASK|PLR_NOSHOUT|PLR_NOTELL|PLR_AUTOEXIT);
*/
    if (!IS_GOD(ch))
      ch->specials.flags &= ~GOD_FLAGS;

    //new hp gaining system (Quilan)
    if(file_version < 27) {
       if(GetMaxLevel(ch) < 28)
	 ch->player.hpsystem = 1;
       else
         ch->player.hpsystem = 2;
    }

    if(ch->player.hpsystem==0) {
       log_msg("Bug in read_player: ch->player.hpsystem==0");
       log_msg("Changing to make current");
       if(GetMaxLevel(ch) < 28)
	 ch->player.hpsystem = 1;
       else
	 ch->player.hpsystem = 2;
    }
    
    if(file_version < 29) {
       //Age conversions from version 28 -> 29
       for (i=0; GET_RACE(ch) != Race_Characteristic[i].Race;i++);
       
       ch->player.time.birth -= SECS_PER_MUD_YEAR *
	 (Race_Characteristic[i].Age.base +
	  Race_Characteristic[i].Age.min_mod *
	  number(1,Race_Characteristic[i].Age.max_mod));
    }

    //added in barehand modifiers in file version 32
    if (file_version <= 31) {
        ch->points.bhnum_mod = 0;
        ch->points.bhsize_mod = 0;
    }
    ch->specials.damnodice += ch->points.bhnum_mod;
    ch->specials.damsizedice += ch->points.bhsize_mod;

/*    if(file_version < 12)
    {
      int clss, exp;
	    
      exp = GET_EXP(ch);
      exp *= HowManyClasses(ch);
	    
      for(clss = 0 ; clss <= MAX_LEVEL_IND  ; clss++)
      {
	if(GET_LEVEL(ch, clss) > 1)
	  exp -= total_exp(GET_LEVEL(ch, clss));
	GET_HIGH_LEVEL(ch, clss) = GET_LEVEL(ch, clss);
      }
	    
      GET_EXP(ch) = MAX(exp, 0);
    }

    if(file_version < 15)
    {
      if(GET_CLASS(ch) == CLASS_SHIFTER)
	GET_RACE(ch) = RACE_LYCANTH;
      else
      {
	switch(GET_RACE(ch))
	{
	default:
	case 1:	GET_RACE(ch) = RACE_HUMAN;	break;
	case 2:	GET_RACE(ch) = RACE_ELF;	break;
	case 3:	GET_RACE(ch) = RACE_DWARF;	break;
	case 4:	GET_RACE(ch) = RACE_HOBBIT;	break;
	case 5:	GET_RACE(ch) = RACE_GNOME;	break;
	}
      }
    }
    if(file_version < 18)
    {
      ch->player.age12='e';
      ch->player.age13='e';
      ch->player.age14='e';
      ch->player.mother=1;
      ch->player.father=1;
      ch->player.upbringing=0;
    }
*/
  }

  if(flags & READ_ALIASES)
  {
    if(fseek(fp, aliases_off, 0))
      return 13;

    if(getaliases(ch, fp))
      return 14;
  }
    
  if(flags & READ_OBJECTS)
  {
    if((file_version == 5) && strcmp(ch->aliases->pattern[0], "!"))
    {
      char buf[256];
      sprintf(buf, "%s losing stuff to version 5 file...",
	      GET_IDENT(ch));
      log_msg(buf);
    }
    else
    {
      if(fseek(fp, objects_off, 0))
	return 11;

      if(!(flags & READ_DO_COUNT))
	count_objects = FALSE;
	
      if(file_version <= 4)
      {
	if(getobjects_v4(ch, fp))
	  return 12;
      }
      else if(getobjects(ch, fp))
	return 12;

      count_objects = TRUE;
    }
  }

  if(ferror(fp))
    status = 15;

  /* write the log by player name */
  
  char tmp[255], tmp2[255];
  long long tmp3;
  int tmp4;
  long pos=-1;
   
  logfile = fopen("PLAYERS.LOG","a+");
  
   
  fprintf(logfile,"%-15s %-15Ld %-3d\n", ss_data(ch->player.name),GET_EXP(ch), 
	  GetMaxLevel(ch));
  fclose(logfile); /* for testing */
  
  /* write the log by site */

  return status;
   
  logfile = fopen("PLAYERSITES.LOG","r+");
  if(!logfile) logfile = fopen("PLAYERSITES.LOG", "w+");
  
  while(!feof(logfile)) {
     pos=ftell(logfile);
     if(fscanf(logfile, "%15s %3d %s\n",tmp,&tmp4,tmp2) != 3) {
	pos=-1;
	continue;
     }
     if((!strcmp(ch->specials.hostname, tmp2)) &&
	(!strcmp(ss_data(ch->player.name), tmp))) {
	break;
     } else {
	pos=-1;
     }
  }
   
  if(pos < 0) {
     fseek(logfile, 0, SEEK_END);
  } else {
     fseek(logfile, pos, SEEK_SET);
  }
  fprintf(logfile,"%-15s %-3d %s\n",ss_data(ch->player.name),GetMaxLevel(ch),ch->specials.hostname);
  fclose(logfile);

  return status;
}

	 /* generate index table for the player file */
#define GODS_PER_LEVEL 20
static struct wiz_info
{
    char* title;
    int number;
    char* members[GODS_PER_LEVEL];
}
/*
wizards[] = 
{
    {	"Immortal" 	},
    {	"Creator" 	},
    {	"Saint" 	},
    {	"Demi-God" 	},
    {	"Lesser God" 	},
    {	"God" 		},
    {	"Greater God" 	},
    {	"Lord" 		},
    {	"Implementor" 	},
    {	"Supreme Taco" 	},
    {	0		}
};
*/
wizards[] = 
{
    {	"Grunt" 	},
    {	"File Clerk" 	},
    {	"Associate" 	},
    {	"Staffer" 	},
    {	"Co-Manager" 	},
    {	"Manager" 		},
    {	"Co-Administrator" 	},
    {	"Administrator" 	},
    {	"Implementor" 	},
    {	"Supreme Taco" 	},
    {	0		}
};

static struct wizposinfo {
  char* title;
  int bit;
  int num;
  char* names[GODS_PER_LEVEL];
}   wizpos[] = {
  {"-- Project Manager --\n\r", PROJECT_MANAGER,0},
  {"-- Assistant Managers --\n\r", ASSISTANT_MANAGER,0},
  {"-- Head Builder --\n\r", HEAD_BUILDER,0},
  {"-- Head Questor --\n\r", HEAD_QUESTOR,0},
  {"-- Senior Coders --\n\r", SENIOR_CODER,0},
  {"-- Coders --\n\r", CODER,0},
  {"-- Senior Builders --\n\r", SENIOR_BUILDER,0},
  {"-- Builders --\n\r", BUILDER,0},
  {"-- Questors --\n\r", QUESTOR,0},
  {"-- Webmaster --\n\r", WEBMASTER,0},
  {"-- Ambassadors --\n\r", AMBASSADOR,0},
  {"-- Newbie Helpers --\n\r", NEWBIE_HELPER,0},
  { 0 }
};

void boot_players(const char* dir, int flags)
{
    struct char_data* ch;
    PlayerDir* pd;
    struct wiz_info* ptr;
    struct wizposinfo* w;
    char buf[256];
    char* p;
    int i,max;
    int count_by_version[PLAYER_VERSION + 1];
    int monkLevel;    //added by WOL to support monks having increased barehand by level
    int bhSize;       //new barehand size value
    int bhNum;        //new barehand number value
  

    /* This code here to clear the LOGFILES */
    FILE *logfile;

    logfile = fopen("PLAYERS.LOG","w+");
    fclose(logfile); /* for testing */
//    logfile = fopen("PLAYERSITES.LOG","w+");
//    fclose(logfile);
    logfile = fopen("OBJECTSBYPLAYER.LOG","w+");
    fclose(logfile);
    logfile = fopen("OBJECTSBYOBJECT.LOG","w+");
    fclose(logfile);
    logfile = fopen("OBJECTSBYSITE.LOG","w+");
    fclose(logfile);
    
    for(i = 0 ; i <= PLAYER_VERSION ; ++i)
	count_by_version[i] = 0;
    
    if(!(pd = OpenPlayers(dir)))
    {
	perror(dir);
	exit(1);
    }

   int n;
   for(n=0;n<10;n++) {
      ptr = wizards + n;
      for(i=0;i<ptr->number;i++)
	free(ptr->members[i]);
      ptr->number = 0;
   }
   
   for (w=wizpos;w->title;w++) {
      for(n=1;n<=w->num;n++)
	free(w->names[n]);
      w->num = 0;
   }

    while((ch = ReadPlayer(pd, flags)))
    {
	    player_count++;
	    count_by_version[file_version]++;
	    sprintf(buf,"Loading Player: %s", GET_NAME(ch));
	    log_msg(buf);
	    
	
	if((TRUST(ch) > 0) || (ch->player.godinfo.position > 0) )
	{
	    ptr = wizards + (TRUST(ch) - 1);
	    sprintf(buf, "%s\n\r", GET_REAL_NAME(ch));
	    ptr->members[ptr->number++] = strdup(buf);

	    for (w=wizpos;w->title;w++) {
	      if (IS_SET(ch->player.godinfo.position,w->bit)) {
		sprintf(buf,"%s\n\r",GET_REAL_NAME(ch));
		w->num++;
		w->names[w->num] = strdup(buf);
	      }
	    }
	}
	else
	    add_char_to_hero_list(ch);

	extract_char(ch);

	array_collapse(&object_list);
    }
    
    ClosePlayers(pd);

    for(i = 0 ; i < PLAYER_VERSION + 1 ; ++i)
	if(count_by_version[i])
	{
	    char buf[256];
	    sprintf(buf, "Version %d player files: %d",
		    i, count_by_version[i]);
	    log_msg(buf);
	}

	sprintf(newwizlist,"\n\r"); 

    for (w = wizpos,p=newwizlist+2;w->title;w++) {
	if (w->num>0) {
	center(p,w->title,78);
	p += strlen(p);
      for (i=1;i <= w->num;i++) {
	center(p,w->names[i],78);
	p += strlen(p);
      }
	strcat(p,"\n\r");
	p += strlen(p);
	}
    }
	
  sprintf(wizlist,"\n\r"); 
  for(p = wizlist, ptr = &wizards[MAX_TRUST - 1] ;
	ptr >= wizards ; ptr--)
    {
	if(ptr->number > 0)
	{
	    sprintf(p, "\n\r");
	    p += 2;

	    sprintf(buf, "%s%s (%d) [%d]\n\r",
		    ptr->title, (ptr->number > 1) ? "s" : "",
		    1 + (ptr - wizards), ptr->number);
	    center(p, buf, 78);
	    p += strlen(p);

	    for(i = 0 ; i < ptr->number ; ++i)
	    {
		center(p, ptr->members[i], 78);
		p += strlen(p);
	    }
	}
    }
    strcpy(p, "\n\r");
    p += 2;
    
    max = 0;
    for(i = 0; i <= 9; i++)
	max += wizards[i].number;

    sprintf(buf, "Total Immortals: %d\n\r", max);
    center(p, buf, 78);
    p += strlen(p);

    strcpy(p, "\n\r");
    p += 2;

}

PlayerDir* OpenPlayers(const char* dirName)
{
    PlayerDir*	pd;

    if(access(dirName, X_OK))
    {
	perror(dirName);
	return NULL;
    }
    
    CREATE(pd, PlayerDir, 1);
    pd->dirfd = 0;
    pd->subdir = 'a' - 1;
    pd->dirName = strdup(dirName);

    return pd;
}

void ClosePlayers(PlayerDir* pd)
{
    if(pd->dirfd)
	closedir(pd->dirfd);
    if(pd->dirName)
	FREE(pd->dirName);
    FREE(pd);
}

struct char_data* ReadPlayer(PlayerDir* pd, int flags)
{
    struct char_data* 	ch = 0;
    struct direct*	dir;
    FILE*		fp;
    char		buf[256];
    char		mesg[256];
    int			status;
    char*		p;
    
    do
    {
	while(!pd->dirfd && (pd->subdir < 'z'))
	    pd->dirfd = open_sub_dir(pd->dirName, ++pd->subdir);

	if(!pd->dirfd)
	    return NULL;

	if(!(dir = readdir(pd->dirfd)))
	{
	    closedir(pd->dirfd);
	    pd->dirfd = 0;
	    continue;
	}

	if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
	    continue;

	if((p = strrchr(dir->d_name, '.')) && !strcmp(p, ".bak"))
	    continue;
	
	sprintf(buf, "%s/%c/%s", pd->dirName, pd->subdir, dir->d_name);
	
	if(!(fp = fopen(buf, "r")))
	{
	    perror(buf);
	    continue;
	}

	setvbuf(fp, 0, _IOFBF, 8196);
	
	CREATE(ch, struct char_data, 1);
	clear_char(ch);
	clear_aliases(ch);
	SpaceForSkills(ch);
	
	status = read_player(ch, fp, flags);

	fclose(fp);
	
	if((status == 8) || (status == 15))
	{
	    sprintf(mesg, "%s: is not a player file", buf);
	    log_msg(mesg);
	    extract_char(ch);
	    fclose(fp);
	    continue;
	}
	else if(status)
	{
	    sprintf(mesg, "%s: error %d parsing player file", buf, status);
	    log_msg(mesg);
	    extract_char(ch);
	    continue;
	}
	else if(str_cmp(GET_IDENT(ch), dir->d_name))
	{
	    sprintf(mesg, "%s: file contains wrong player (%s)", buf,
		    GET_IDENT(ch));
	    log_msg(mesg);
	    extract_char(ch);
	    continue;
	}

	return ch;
    } while (1);
}


DIR* open_sub_dir(char* path, char sub)
{
    char		buf[256];
    DIR*		dd;
    
    sprintf(buf, "%s/%c", path, sub);
    if(!(dd = opendir(buf)))
	perror(buf);
    return dd;
}

void SpaceForSkills(struct char_data *ch)
{
    /*
      create space for the skills for some mobile or character.
      */

    if(spell_count < MAX_SKILLS)
	CREATE(ch->skills, struct char_skill_data, MAX_SKILLS);
    else
	CREATE(ch->skills, struct char_skill_data, spell_count + 1);
}

void set_title(struct char_data *ch)
{
    char buf[256], buf2[256];
  
    sprintf(buf, "the %s %s", RaceName[ch->race], ClassTitles(ch, buf2));
  
    ss_free(ch->player.title);
    ch->player.title = ss_make(buf);
}

char *ClassTitles(struct char_data *ch, char* buf)
{
    int i, count=0;

    buf[0] = 0;
    
    for (i = 0; i <= MAX_LEVEL_IND; i++) {
	if (GET_LEVEL(ch, i)) {
	    count++;
	    if (count > 1) {
		sprintf(buf + strlen(buf), "/%s",
			GET_CLASS_TITLE(ch, i, (int) GET_LEVEL(ch,i)));
	    } else {
		sprintf(buf, "%s",
			GET_CLASS_TITLE(ch, i, (int) GET_LEVEL(ch, i)));
	    }
	}
    }
    return(buf);
}

/* Gain maximum in various points */
void advance_level(struct char_data *ch, int clss)
{
    int add_hp, i, pracs, M=0;
    char buf[256];
  
   //the hpsystem of 2 allows players to DM, so they gain differently.
   if(ch->player.hpsystem==0) {
     if(GetMaxLevel(ch) < 28)
       ch->player.hpsystem=1; //new system
     else
       ch->player.hpsystem=2; //old system
   }
   
   //Increase the players level and let the world know about it.
   GET_LEVEL(ch, clss) += 1;
   sprintf(buf,"\n\r                $CBThieves World welcomes $Cg%s$CB to level $Cg%d$Cg!\n\r",
	   GET_NAME(ch),GET_LEVEL(ch, clss));
   send_to_all_formatted(buf);

   //If they had died, their new level should be greater than their 'high' level in the class
   //in that case, we do a bunch of hp gaining.
   if(GET_LEVEL(ch, clss) > GET_HIGH_LEVEL(ch, clss))
   {
        //increase their high level
        GET_HIGH_LEVEL(ch, clss) = GET_LEVEL(ch, clss);
       
        //get the additional points for a constitution bonus
  	    add_hp = con_app[GET_CON(ch)].hitp;

        //in the new system, we double the con-bonus
	    if(ch->player.hpsystem == 1)
	        add_hp = 2 * con_app[GET_CON(ch)].hitp;

        //if they are not on the new system - use the old ranges for hp gain.
	    if(ch->player.hpsystem != 1) {
	        switch(clss) { 
                case MAGE_LEVEL_IND : {
	                if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
	    	            add_hp += number(3, 6);
	                else
		                add_hp += 2;
	            } break;
                
	            case CLERIC_LEVEL_IND : {
	                if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 25)
		                add_hp += number(3, 9);
	                else
		                add_hp += 3;
	            } break;
                
	            case THIEF_LEVEL_IND : {
	                if (!(HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_MAGIC_USER))) 
                    {
		                if(!HasClass(ch, CLASS_WARRIOR)) 
                        {
		                    add_hp += number(5,9);
		                } else {
		                    add_hp += number(2,9);
                        }
                    }
                    else if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 25) 
                    {
		                add_hp += number(3,10);
                    } else {
		                add_hp += 2;
                    }
	            } break;
                
	            case WARRIOR_LEVEL_IND : {
	                if (!(HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_MAGIC_USER)))
		                if(!HasClass(ch, CLASS_THIEF))
		                    add_hp += number(5,20);
		                else
    		                add_hp += number(3,20);
	                else if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 25)
		                add_hp += number(3,20);
	                else
		                add_hp += 4;
	            } break;

	            case PALADIN_LEVEL_IND : {
	                add_hp += number(3,12);
	            } break;
                
	            case DRUID_LEVEL_IND : {
	                add_hp += number(2,7);
	            } break;
                
	            case PSI_LEVEL_IND : {
	                add_hp += number(2,6);
	            } break;
                
	            case RANGER_LEVEL_IND : {
	                add_hp += number(3,14);
	            } break;

                case SHIFTER_LEVEL_IND : {
                     add_hp += number(2,7);
                } break;
                
	            case BARD_LEVEL_IND : {
	                add_hp += number(6,10);
	            } break;

	            case MONK_LEVEL_IND : {
	                add_hp += number(10,25);
                } break;
	            }
     } else { // They are on the new system - give them stuff then
        switch(clss) {
	        //M is basicly a relative gauge of how many hps per lvl...
	        case MAGE_LEVEL_IND    : M = 16; break;
	        case CLERIC_LEVEL_IND  : M = 19; break;
	        case THIEF_LEVEL_IND   : M = 18; break;
	        case WARRIOR_LEVEL_IND : M = 23; break;
	        case PALADIN_LEVEL_IND : M = 25; break;
	        case DRUID_LEVEL_IND   : M = 18; break;
	        case PSI_LEVEL_IND     : M = 19; break;
	        case RANGER_LEVEL_IND  : M = 22; break;
	        case SHIFTER_LEVEL_IND : M = 18; break;
	        case BARD_LEVEL_IND    : M = 19; break;
	        case MONK_LEVEL_IND    : M = 26; break;
	        }
     } // end if they are on the new/old hpsystem

     //q2
     //Hitpoint math
    if(ch->player.hpsystem == 1) { //new hp system
	  add_hp += M + number(0, 32) - 16;
	  if (IS_MULTI(ch)) {
	   add_hp /= (HowManyClasses(ch));
	   add_hp += (HowManyClasses(ch)-1);
	  }
    } else { //old hp system
	  if (IS_MULTI(ch)) 
	    add_hp /= (HowManyClasses(ch)-1);
	  if (GET_LEVEL(ch, clss) <= 25)
	     add_hp++;
    }

    //actually do the increase in their max hitpoints
	ch->points.max_hit += MAX(1, add_hp);
    
    //update their barehand damage if they are monks
    if (clss==MONK_LEVEL_IND)
    {
        if (((GET_LEVEL(ch, clss) % 5) == 0) && ((GET_LEVEL(ch, clss) % 10) != 0))
        {
            ch->points.bhnum_mod++;
            ch->specials.damnodice++;
        }

        if ((GET_LEVEL(ch, clss) % 10) == 0)
        {
            ch->points.bhsize_mod += 1;
            ch->specials.damsizedice++;
        }
    } else {
        ch->points.bhnum_mod = 0;
        ch->points.bhsize_mod = 0;
    }

    //Practices set
	pracs = wis_app[GET_WIS(ch)].bonus;
	if (ch->specials.spells_to_learn < 110)
	  if (HowManyClasses(ch)==1)
	    pracs = MAX(2, pracs); //give pure class a bonus on the min number of practices
	  else
	  {
	    pracs /= HowManyClasses(ch);
	    pracs = MAX(1, pracs);
	  }
	ch->specials.spells_to_learn =
	  MIN(120, ch->specials.spells_to_learn + pracs);

    } // end if(GET_LEVEL(ch, clss) > GET_HIGH_LEVEL(ch, clss))

    UpdateMaxLevel(ch);
    UpdateMinLevel(ch);
    if (IS_GOD(ch))
	for (i = 0; i < 3; i++)
	    ch->specials.conditions[i] = -1;
	/* This here so i can make sure that their title gets set. */
    set_title(ch);
}	

object_map* new_object_map(int max_obj_save)
{
    object_map*		map;

    CREATE(map, object_map, 1);

    map->max = max_obj_save;
    map->count = 0;
    CREATE(map->objects, struct obj_data*, map->max);

    return map;
}

void kill_object_map(object_map* map)
{
    FREE(map->objects);
    FREE(map);
}

int map_add_object(object_map* map, struct obj_data* obj)
{
    if(map->count >= map->max)
    {
	map->max += MAX_OBJ_SAVE;
	RECREATE(map->objects, struct obj_data*, map->max);
    }

    map->objects[map->count] = obj;

    /* PAC - I changed this to a pre increment */
    return ++map->count;
}

void map_add_ident(object_map* map, int index, struct obj_data* obj)
{
    if(index >= map->max)
    {
	map->max = index + MAX_OBJ_SAVE;
	RECREATE(map->objects, struct obj_data*, map->max);
    }

    map->objects[index] = obj;

    if(index > map->count)
	map->count = index;
}

struct obj_data* map_map_ident(object_map* map, int index)
{
    if(index >= map->count)
	return 0;

    return map->objects[index];
}

#define UPDATE_STRING(sa, sb)	\
if(sa != sb)			\
{				\
    ss_free(sa);		\
    sa = sb;			\
} else				\
    ss_free(sb);
   
struct obj_data* object_from_data(struct char_data* ch, struct obj_data* tmpl)
{
  char buf[256];
  struct obj_data* obj;
  int i;
  
  if((tmpl->item_number < 0) ||
     !(obj = make_object(tmpl->item_number, VIRTUAL|NORAND)))
  {
    obj = create_object();
    obj->item_number = -1;
    if(file_version < 1)
    {
      if(tmpl->item_number < 0)
	obj->obj_flags.type_flag = -tmpl->item_number;
      else
	obj->obj_flags.type_flag = 0;
    }
    obj->obj_flags.type_flag = tmpl->obj_flags.type_flag;
    array_insert(&object_list, obj);
    if(ch) {
      sprintf(buf, "%s: Made %s from whole cloth!",
	      GET_IDENT(ch), OBJ_NAME(obj));
      log_msg(buf);
    }
  }

  UPDATE_STRING(obj->name, tmpl->name);
  UPDATE_STRING(obj->description, tmpl->description);
  UPDATE_STRING(obj->short_description, tmpl->short_description);
  UPDATE_STRING(obj->action_description, tmpl->action_description);

  /* commented out the follwoing for my Unique Object Code -- Min Copyright 1997 */
   
//-THIS- was the bug for the samples & rations! (Quilan)
//
#if 0
/* if the old one and the new one aren't the same type, punt it all */
  if(tmpl->obj_flags.type_flag != obj->obj_flags.type_flag)
  {
//    sprintf(buf, "%s: Object changed type:  %s", GET_IDENT(ch),  OBJ_NAME(obj));
/*    log_msg(buf); */
    return obj;
  }
#else
   obj->obj_flags.type_flag = tmpl->obj_flags.type_flag;
#endif

#ifdef JANWORK
 
  /* if the old one isn't cursed, assume remove curse has been
     cast and remove it from the new one too */
  /* Umm. I'm not sure why this is here.  Its effect is to remove player
     case curses from objects in rent file.  Curses are used by players
     to prevent their possesions from being stolen.  We'll try it for a while
     with this cut out.  */
/*  if(!IS_SET(tmpl->obj_flags.extra_flags, ITEM_NODROP))
    REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);*/

    if(!IS_SET(tmpl->obj_flags.extra_flags, ITEM_NODROP))
       REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    if(IS_SET(tmpl->obj_flags.extra_flags, ITEM_NODROP) &&
       !IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP))
         SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

  switch(obj->obj_flags.type_flag)
  {
  case ITEM_WEAPON:
    /* maintain damage */
    obj->obj_flags.value[2] = MIN(obj->obj_flags.value[2],
				  tmpl->obj_flags.value[2]);

    /* if the weapon has been enchanted, find the last
       HND apply and copy it over */
    if(!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC) &&
       IS_SET(tmpl->obj_flags.extra_flags, ITEM_MAGIC))
    {
      int i, aff;
      for(i = MAX_OBJ_AFFECT - 1 ; i >= 0 ; --i)
	if((tmpl->affected[i].location == APPLY_HITNDAM) &&
	   ((aff = getFreeAffSlot(obj)) >= 0))
	{
	  obj->affected[aff] = tmpl->affected[i];
	  /* also copy over the magic, and any gained
	     alignment flags */
	  obj->obj_flags.extra_flags |=
	    tmpl->obj_flags.extra_flags &
	      (ITEM_MAGIC | ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
	  break;
	}
    }
    break;

  case ITEM_ARMOR:
    /* maintain armor damage */
    obj->obj_flags.value[0] = MIN(obj->obj_flags.value[0],
				  tmpl->obj_flags.value[0]);
    break;

  case ITEM_STAFF:
  case ITEM_WAND:
    /* wand and staff charges */
    obj->obj_flags.value[2] = MIN(obj->obj_flags.value[2],
				  tmpl->obj_flags.value[2]);
    break;

  case ITEM_LIGHT:
    /* remaining light duration */
    obj->obj_flags.value[2] = tmpl->obj_flags.value[2];

    /* make sure old lights can be used as lights */
    if(file_version <= 13)
      obj->obj_flags.wear_flags |= ITEM_WEAR_LIGHT;
    break;
    
  case ITEM_DRINKCON:
  case ITEM_FOOD:
    /* contents and quantity */
    obj->obj_flags.value[0] = tmpl->obj_flags.value[0];
    obj->obj_flags.value[1] = tmpl->obj_flags.value[1];
    obj->obj_flags.value[2] = tmpl->obj_flags.value[2];
    obj->obj_flags.value[3] = tmpl->obj_flags.value[3];
    break;

  case ITEM_CONTAINER:
    /* save locked state */
    obj->obj_flags.value[1] = tmpl->obj_flags.value[1];
    break;

  case ITEM_OTHER:
    obj->obj_flags.value[0] = tmpl->obj_flags.value[0];
    obj->obj_flags.value[1] = tmpl->obj_flags.value[1];
    obj->obj_flags.value[2] = tmpl->obj_flags.value[2];
    obj->obj_flags.value[3] = tmpl->obj_flags.value[3];
  }

  /* if the item has been Continual Light/Dark, find the last
       APPLY_SPELL apply and copy it over */

  if(!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC) &&
       IS_SET(tmpl->obj_flags.extra_flags, ITEM_MAGIC))
  {
    int i, aff;
    for(i = MAX_OBJ_AFFECT - 1 ; i >= 0 ; --i)
      if((tmpl->affected[i].location == APPLY_SPELL) &&
	 ((aff = getFreeAffSlot(obj)) >= 0))
      {
	obj->affected[aff] = tmpl->affected[i];
	/* also copy over the magic alignment flags */
	obj->obj_flags.extra_flags |=
	  tmpl->obj_flags.extra_flags &
	    (ITEM_MAGIC);
	break;
      }
  }

#endif
  
  /* just copy the data over that was read in from the template */

//  obj->obj_flags.cost = tmpl->obj_flags.cost;
  obj->obj_flags.wear_flags = tmpl->obj_flags.wear_flags;
  obj->obj_flags.extra_flags = tmpl->obj_flags.extra_flags;
  obj->obj_flags.weight = tmpl->obj_flags.weight;
  obj->obj_flags.timer = tmpl->obj_flags.timer;
  obj->obj_flags.cost_per_day = tmpl->obj_flags.cost_per_day;
  obj->obj_flags.level = tmpl->obj_flags.level;
  obj->obj_flags.durability = tmpl->obj_flags.durability;
  obj->obj_flags.encumbrance = tmpl->obj_flags.encumbrance;
  obj->obj_flags.no_loot = tmpl->obj_flags.no_loot;


  for (i = 0 ; i < MAX_OBJ_AFFECT ; ++i ) 
  {
      obj->affected[i].location = tmpl->affected[i].location;
      obj->affected[i].modifier = tmpl->affected[i].modifier;
  }

  obj->obj_flags.value[0] = tmpl->obj_flags.value[0];
  obj->obj_flags.value[1] = tmpl->obj_flags.value[1];
  obj->obj_flags.value[2] = tmpl->obj_flags.value[2];
  obj->obj_flags.value[3] = tmpl->obj_flags.value[3];

  if (obj->obj_flags.type_flag == ITEM_WEAPON &&  
      obj->obj_flags.value[3] == 12 /* Magic Fucking value that says it is RANGED */ && 
      obj->obj_flags.wear_flags & (ITEM_WIELD | ITEM_HOLD))
    obj->obj_flags.wear_flags = obj->obj_flags.wear_flags & ~(ITEM_WIELD | ITEM_HOLD);

  return obj;
}




