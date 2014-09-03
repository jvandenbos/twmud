#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "race.h"
#include "opinion.h"
#include "hash.h"
#include "wizlist.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "spec.h"
#include "cmdtab.h"
#include "spelltab.h"
#include "modify.h"
#include "recept.h"
#include "weather.h"
#include "spell_util.h"
#include "multiclass.h"
#include "constants.h"
#include "board.h"
#include "util_str.h"
#include "statistic.h"
#include "sound.h"
#include "proto.h"
#include "spells.h"
#include "db.random.h"

/* globals */
array_t object_list;
struct index_data *obj_index;	/* index table for object file     */
long obj_count=0;
long total_obc=0;
int count_objects = TRUE;
int top_of_objt = 0;		/* top of object index table       */
FILE* obj_f;			/* obj prototypes                  */

#define MAX_SHORT_LENGTH 30
#define NOVAK_BALANCE  1
#define MAX_LEVEL 125

/* forward declarations */
struct obj_data* read_object(int nr);
struct obj_data* parse_object(FILE* obj_f, int nr);
int obj_sound_timer(object_event* event, int now);



/* 
 * Data Table for the Special AFFECTS 
 */

/*
struct special_affect_type {
    char *name;
    long minlevel;
    int affnum;
    long affminval;
    long affmaxval;
    long type_flag;
};
*/

const struct special_affect_type specialaffects[] =
{
/**** Saving Throws ****/
{ "Wyverns", 0, APPLY_SAVING_PARA, -5, -1, RAND_GROUP_GENERAL },
{ "Manticores", 0, APPLY_SAVING_PARA, -10, -6, RAND_GROUP_GENERAL },
{ "Initiates", 0, APPLY_SAVING_ROD, -5, -1, RAND_GROUP_GENERAL },
{ "Apprentices", 0, APPLY_SAVING_ROD, -10, -6, RAND_GROUP_GENERAL },
{ "Gorgons", 0, APPLY_SAVING_PETRI, -5, -1, RAND_GROUP_GENERAL },
{ "Medusa", 0, APPLY_SAVING_PETRI, -10, -6, RAND_GROUP_GENERAL },
{ "Wyrms", 0, APPLY_SAVING_BREATH, -5, -1, RAND_GROUP_GENERAL },
{ "Dragons", 0, APPLY_SAVING_BREATH, -10, -6, RAND_GROUP_GENERAL },
{ "Hags", 0, APPLY_SAVING_SPELL, -5, -1, RAND_GROUP_GENERAL },
{ "Witches", 0, APPLY_SAVING_SPELL, -10, -6, RAND_GROUP_GENERAL },
{ "the Shaman", 0, APPLY_SAVE_ALL, -5, -1, RAND_GROUP_GENERAL },
{ "the Magi", 0, APPLY_SAVE_ALL, -10, -6, RAND_GROUP_GENERAL },

/**** Height ****/
{ "Brownies", 0, APPLY_CHAR_HEIGHT, -100, -50, RAND_GROUP_GENERAL },
{ "Hobbits", 0, APPLY_CHAR_HEIGHT, -60, -1, RAND_GROUP_GENERAL },
{ "Giants", 0, APPLY_CHAR_HEIGHT, 1, 60, RAND_GROUP_GENERAL },
{ "Titans", 0, APPLY_CHAR_HEIGHT, 50, 100, RAND_GROUP_GENERAL },

/**** Weight ****/
{ "Fairies", 0, APPLY_CHAR_WEIGHT, -100, -50, RAND_GROUP_GENERAL },
{ "Pixies", 0, APPLY_CHAR_WEIGHT, -60, -1, RAND_GROUP_GENERAL },
{ "Mammoths", 0, APPLY_CHAR_WEIGHT, 1, 60, RAND_GROUP_GENERAL },
{ "Behemoths", 0, APPLY_CHAR_WEIGHT, 50, 100, RAND_GROUP_GENERAL },

/**** Song ****/
{ "Silence", 0, APPLY_WEAPON_SPELL, SPELL_SILENCE, 0, RAND_GROUP_WEAPONS },
{ "Nightmares", 0, APPLY_WEAPON_SPELL, SPELL_TERROR, 0, RAND_GROUP_WEAPONS },
{ "the Pacifier", 0, APPLY_WEAPON_SPELL, SPELL_SLOW, 0, RAND_GROUP_WEAPONS },
{ "Sorrow", 0, APPLY_WEAPON_SPELL, SPELL_DESPAIR, 0, RAND_GROUP_WEAPONS },

};

int MAX_SPECIALAFFECTS =  0;




/* *** THE START OF THE ACTUAL FUNCTIONS *** */




void boot_objects(const char* file)
{
  array_init(&object_list, 8196, 1024);
  
  if (!(obj_f = fopen(file, "r")))	
    {
      perror(file);
      exit(1);
    }
  
  obj_index = generate_indices(obj_f, &top_of_objt);
  
  /* This is to init the MAX COUNT for Special Affects */
  MAX_SPECIALAFFECTS = (int)(sizeof(specialaffects)/sizeof(struct special_affect_type));

  DLOG(("There are %d special affects in the game.\n", MAX_SPECIALAFFECTS));
}

/* create an object */
struct obj_data *create_object(void)
{
  struct obj_data* obj;
  
  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  
  obj_count++;
  
  return obj;
}

/* create a new instance of an object */
struct obj_data* make_object(int nr, int type, int rnum) {
    int rl;
    struct obj_data* obj; 
    struct obj_data* tmp = NULL; 
    char buf[256];
    
    if(IS_SET(type, VIRTUAL))
    {
	if((rl = real_object(nr)) == -1)
	{
	    sprintf(buf, "Object (V) %d does not exist in database.", nr);
	    log_msg(buf);
	    return 0;
	}
    }
    else
	rl = nr;
    
#if 1
    if(!obj_index[rl].proto)
	{
	   tmp =  read_object(rl);
	   obj_index[rl].proto = tmp;
	   obj_count--;
	}
    
    if(obj_index[rl].proto) {
	if(IS_SET(type, NORAND)) {
	    obj = read_object(rl);
	    obj->obj_flags.tick = number(0, MOB_MAX_DIVISOR-1);
	}
	else if(IS_SET(type, RAND) || (rnum > -2))
	    obj = clone_object((struct obj_data*) obj_index[rl].proto,
rnum);
	else {
	    obj = clone_object((struct obj_data*) obj_index[rl].proto);
	}
    }
    else {
	obj = 0;
    }

#else
    obj = read_object(rl);
#endif

    if(obj)
    {
	array_insert(&object_list, obj);

	if((ITEM_TYPE(obj) == ITEM_AUDIO) && obj->obj_flags.value[0])
	{
	    object_event*	event;
	    int			when =
	      next_pulse(pulse ? obj->obj_flags.value[0] :
			 number(0, obj->obj_flags.value[0]));
	    
	    CREATE(event, object_event, 1);
	    event->object = obj;

	    sprintf(buf, "object sound %s", OBJ_NAME(obj));
	    obj->sound_timer = event_queue_pulse((event_t*) event,
						 when,
						 (event_func) obj_sound_timer,
						 buf);
	}
	
	if(count_objects)
	    obj_index[rl].number++;
    }

    return obj;
}

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr)
{
    fseek(obj_f, obj_index[nr].pos, 0);
    return parse_object(obj_f, nr);
}

#define GETN(fld) \
{ \
    long tmp; \
    if(!parse_number("obj file", #fld, virt, (const char**) &ptr, &tmp)) \
	return 0; \
    fld = tmp; \
}
#define GETUN(fld) \
{ \
    unsigned long tmp; \
    if(!parse_unumber("obj file", #fld, virt, (const char**) &ptr, &tmp)) \
	return 0; \
    fld = tmp; \
}

struct obj_data* parse_object(FILE* obj_f, int nr) {
    struct obj_data *obj;
    long bc;
    char buf[256];
    char* ptr;
    struct extra_descr_data *new_descr;
    int aff_cnt;
    int virt;

    obj = create_object();
    bc = sizeof(struct obj_data);
    
    virt = obj_index[nr].virt;
    
    /* *** string data *** */
  
    obj->name = ss_fread(obj_f);
    if (*OBJ_NAME(obj)) {
	bc += strlen(OBJ_NAME(obj));
    }
    obj->short_description = ss_fread(obj_f);
    if (*OBJ_SHORT(obj))
	bc += strlen(OBJ_SHORT(obj));
    obj->description = ss_fread(obj_f);
    if (*OBJ_DESC(obj))
	bc += strlen(OBJ_DESC(obj));
    obj->action_description = ss_fread(obj_f);
    if (*OBJ_ACTION(obj))
	bc += strlen(OBJ_ACTION(obj));
  
    /* *** numeric data *** */
  
    ptr = fgets(buf, sizeof(buf), obj_f);
    GETN(obj->obj_flags.type_flag);
    GETUN(obj->obj_flags.extra_flags);
    GETN(obj->obj_flags.wear_flags);

    ptr = fgets(buf, sizeof(buf), obj_f);
    GETN(obj->obj_flags.value[0]);
    GETN(obj->obj_flags.value[1]);
    GETN(obj->obj_flags.value[2]);
    GETN(obj->obj_flags.value[3]);
    if (obj->obj_flags.value[3] == 12 /* Magic Fucking value that says it is RANGED */ && 
	obj->obj_flags.wear_flags & (ITEM_WIELD | ITEM_HOLD)) {
      sprintf(buf,"BUG in object : %d is a ranged weapon, and can be wielded, fix me.",
	      virt);
      log_msg(buf);
      /* Remove the bits in question */
      obj->obj_flags.wear_flags = obj->obj_flags.wear_flags & ~(ITEM_WIELD | ITEM_HOLD);
    }
   
    if(obj->obj_flags.type_flag == ITEM_MONEY && obj->obj_flags.value[0] > 100000) {
       sprintf(buf, "Object #%d has %d coins.", virt, obj->obj_flags.value[0]);
       log_msg(buf);
    }
    
    ptr = fgets(buf, sizeof(buf), obj_f);
    GETN(obj->obj_flags.weight);
    GETN(obj->obj_flags.cost);
    GETN(obj->obj_flags.cost_per_day);
    
    ptr = fgets(buf, sizeof(buf), obj_f);
    GETN(obj->obj_flags.level);
    GETN(obj->obj_flags.durability);
    GETN(obj->obj_flags.encumbrance);

    /* *** extra descriptions *** */
    
    obj->ex_description = 0;
    
    aff_cnt = 0;
    while((ptr = fgets(buf, sizeof(buf), obj_f)) &&
	  (*ptr != '#') && (*ptr != '$'))
    {
	switch(*ptr)
	{
	case 'E':
	    CREATE(new_descr, struct extra_descr_data, 1);
	    bc += sizeof(struct extra_descr_data);
	    new_descr->keyword = fread_string(obj_f);
	    if (new_descr->keyword && *new_descr->keyword)
		bc += strlen(new_descr->keyword);
	    new_descr->description = fread_string(obj_f);
	    if (new_descr->description && *new_descr->description)
		bc += strlen(new_descr->description);
	    new_descr->next = obj->ex_description;
	    obj->ex_description = new_descr;
	    break;
	    
	case 'A':
	    ptr = fgets(buf, sizeof(buf), obj_f);
	    GETN(obj->affected[aff_cnt].location);
	    GETUN(obj->affected[aff_cnt].modifier);
	    aff_cnt++;
	    break;
	    
	default:
	    // log_syntax("obj file", virt, "Expected 'E' or 'A'", ptr);
	    break;
	}
    }
    
    while(aff_cnt < MAX_OBJ_AFFECT)
    {
	obj->affected[aff_cnt].location = APPLY_NONE;
	obj->affected[aff_cnt++].modifier = 0;
    }
    
    obj->in_room = NOWHERE;
    obj->next_content = 0;
    obj->carried_by = 0;
    obj->equipped_by = 0;
    obj->eq_pos = -1;
    obj->in_obj = 0;
    obj->contains = 0;
    obj->item_number = nr;	
    obj->obj_flags.cont_weight = 0;
    
    /* assign class restrictions for the new classes */
#if KLUDGE_RESTRICTIONS
    if((IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)))
	if(!IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI))
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_PSI);
    
    if((IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) ||
       (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD)))
	if(!IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN))
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN);
    
    if((IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)))
	if(!IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID))
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID);
    
    if((IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)))
	if(!IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER))
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER);
#endif
    
#if BYTE_COUNT
    fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].virt, bc);
#endif
    total_obc += bc;
    return (obj);  
}

/* make an exact duplicate of an object */

/* Following code modified by Min 1997 to allow for completely unique items */
struct obj_data* clone_object(struct obj_data* obj, int rnum)
{
    struct obj_data* clone;
    struct extra_descr_data* descr;
    int i;
    int affslot;
    int randtest=0;
    char new_description[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    long maxdamage;
    long newlevel;
    
    clone = create_object();
    
    /* copy important information */
    clone->item_number = obj->item_number;
    clone->obj_flags = obj->obj_flags;
    
    /* figure out object affects */
    
    affslot = 0;
    
    for(i = 0 ; i < MAX_OBJ_AFFECT ; ++i)
	if ((percent() < 65) || (rnum > -2) || (obj->obj_flags.cost < 5000))
	{
	    clone->affected[affslot] = obj->affected[i];
	    ++affslot;
	}
    
    /* it's nowhere yet */
    clone->in_room = NOWHERE;
    clone->carried_by = 0;
    clone->eq_pos = -1;
    clone->in_obj = 0;
    clone->contains = 0;
    clone->next_content = 0;
    
    /* copy strings */
    clone->name = ss_share(obj->name);
    clone->description = ss_share(obj->description);
    clone->short_description = ss_share(obj->short_description);
    clone->action_description = ss_share(obj->action_description);
    
    /* null out res stuff */
    clone->killer = 0;
    clone->char_vnum = 0;
    clone->char_name = 0;
    
    /* fix extra descriptions, if any */
    clone->ex_description = 0;
    for(descr = obj->ex_description ; descr ; descr = descr->next)
    {
	struct extra_descr_data* new_descr;
	
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword = strdup(descr->keyword);
	new_descr->description = strdup(descr->description);
	new_descr->next = clone->ex_description;
	clone->ex_description = new_descr;
    }
   
    clone->obj_flags.tick = number(0, MOB_MAX_DIVISOR-1);

   
    /* HERE BEGINS THE ACTUAL UNIQUE ITEM CODE */
    /* This Code is Copyright 1997 by Jan Vandenbos, All Rights Reserved */

    if (rnum==-2 && (obj->obj_flags.cost > 5000)) {
       if ((clone->obj_flags.type_flag == ITEM_WEAPON)) { 
	  /* if its a weapon */
	  /* first change the weapon damage */
	  clone->obj_flags.value[1] = (clone->obj_flags.value[1] / 2) +
	    randomnum(clone->obj_flags.value[1] / 2);
	  clone->obj_flags.value[2] = (clone->obj_flags.value[2] / 2) +
	    randomnum(clone->obj_flags.value[2] / 2);
	  
	  clone->obj_flags.value[1] = MAX(1, clone->obj_flags.value[1]);
	  clone->obj_flags.value[2] = MAX(1, clone->obj_flags.value[2]);
	  
	  /* then recalc item level based upon base damage */
	  maxdamage = clone->obj_flags.value[1] * clone->obj_flags.value[2];
	  newlevel = (maxdamage * 3) / 5; /* was just / 2 */
	  newlevel /= 5; /* because of rounding, these two lines set to multiple of 0,5,10 */
	  newlevel *= 5;

	  // It happens that a weapon is so powerful it is assigned a level higher than 125
	  // this should never happen - raist
	  if (newlevel > MAX_LEVEL) newlevel = MAX_LEVEL;
	  
	  if (clone->obj_flags.level < newlevel)
	    clone->obj_flags.level = newlevel; /* limit level by weapon dam */
	  
       }
       
       if (clone->obj_flags.type_flag == ITEM_ARMOR) /* armor */
	 clone->obj_flags.value[0] = MAX(1,randomnum(clone->obj_flags.value[0]));
       
       if (clone->obj_flags.type_flag == ITEM_TREASURE) /* treasure */
	 clone->obj_flags.value[0] = MAX(1,randomnum(clone->obj_flags.value[0]));
    }
  
#define RANDOM_CHANCE 40
   
   if (rnum > -2) {
      add_random_obj(rnum,clone);
   } else if (number_range(1,1000) < RANDOM_CHANCE) {
      add_random_obj(clone);
   }
  
   return clone;
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *ptr, *next_one;

    obj_count--;
    
    ss_free(obj->name);
    ss_free(obj->description);
    ss_free(obj->short_description);
    ss_free(obj->action_description);

    for(ptr = obj->ex_description ; ptr != 0 ; ptr = next_one)
    {
	next_one = ptr->next;
	if(ptr->keyword)
	    FREE(ptr->keyword);
	if(ptr->description)
	    FREE(ptr->description);
	FREE(ptr);
    }

    ss_free(obj->char_name);

    /* make sure no dangling event pointers */
    event_cancel(obj->sound_timer, 1);
    event_cancel(obj->corpse_timer, 1);
    
    FREE(obj);
}

/* complete clear out an object */
void clear_object(struct obj_data *obj)
{
    memset(obj, '\0', sizeof(struct obj_data));

    obj->item_number = -1;
    obj->in_room = NOWHERE;
    obj->eq_pos = -1;
}


/* returns the real number of the object with given virtual number */
int real_object(int virt)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_objt;

    /* perform binary search on obj-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((obj_index + mid)->virt == virt)
	    return(mid);
	if (bot > top)
	    return(-1);
	if ((obj_index + mid)->virt > virt)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

// note key parameters:
// most = the load limit for the item
// if init = 1, object guaranteed to load (only for boot time)
// if init = 0, tresure, trash, key, money, board, drinkcon, other all load if under limit
// all others load only on a random chance

int UnderLimit(int rnum, int most, int init, int level, int type) {

    int chance = 0, calc_chance;

    if (rnum < 0 || rnum > top_of_objt)
		return FALSE;
	else if(most == ONCE_PER_REBOOT || most == UNIQUE_LOAD)

    if(init)
 	  return TRUE;

    switch(type) {
		case ITEM_TREASURE:
		case ITEM_TRASH:
		case ITEM_KEY:
	 	case ITEM_MONEY:
		case ITEM_BOARD:
		case ITEM_DRINKCON:
		case ITEM_OTHER:
           return obj_index[rnum].number < most;
	}

    if((most == -1) || (most == -2)) // cases to handle unique items
            return TRUE;  // shouldn't this be false?

	// chance of loading is dependent on level of item

    if (level < 100)  // changed by Tanga - testing - simplifying the calc
       chance = 95;
    else if (level < 120)
      chance = 85;
    else
      chance = 65; // (items >= lvl 120)

    /*
#ifdef JANWORK

    else if (level < 30)
            chance = 85;
         else if (level < 50)
                 chance = 75;
              else if (level < 75)
              {
                  	chance = 50;
          			calc_chance = (100 * most)/200;
           			chance = MIN(chance, calc_chance);
         	  }
        	  else if (level < 100)
              {
          		chance = 45;
          		calc_chance = (100 * most)/200;
          		chance = MIN(chance, calc_chance);
        	  }
        	  else if ( level < 125)
        	  {
          		chance = 35;
          		calc_chance = (100 * most)/200;
          		chance = MIN(chance, calc_chance);
        	  }
        	  else
        	  {
          		chance = 30;
          		calc_chance = (100 * most)/200;
          		chance = MIN(chance, calc_chance);
        	  }

#endif
    */
        if (percent() < chance)
          return (obj_index[rnum].number <= most); // Still needed or stuff will load infinitly, thus more than most :-P --mnemosync depr:  < MIN(50,most); // normal objects load 0-50% chance if under max
        else
           return FALSE;


} // UnderLimit fini

// OLD!
//int UnderLimit(int rnum, int most, int init, int level, int type)
//{
//    int chance = 0, calc_chance;
//
//    if (rnum < 0 || rnum > top_of_objt)
//	return FALSE;
//
//    else if(most == ONCE_PER_REBOOT || most == UNIQUE_LOAD)
//#ifdef SWAP_ZONES
//    {
//       if(init) return TRUE;
//       
//	if (!obj_index[rnum].number)
//	    return (number(0, 4) == 0);
//	else
//	    return FALSE;
//    }
//
//#else
//    {
//	if(!init)		/* not reboot, so it can't load */
//	    return FALSE;
//	if(obj_index[rnum].limit == ONCE_PER_REBOOT)
//	    return TRUE;	/* not otherwise limited, do it */
//	return obj_index[rnum].number < obj_index[rnum].limit;
//    }
//#endif    
//    else
//    {
//        if(init)
// 	  return TRUE;
//       
//        switch(type) {
//	 case ITEM_TREASURE:
//	 case ITEM_TRASH:
//	 case ITEM_KEY:
//	 case ITEM_MONEY:
//	 case ITEM_BOARD:
//	 case ITEM_DRINKCON:
//	 case ITEM_OTHER:
//           return obj_index[rnum].number < most;
//	}
//       
//        if(most == -1 || most == -2)
//           return TRUE;
//
//        if (level < 10)
//          chance = 100;  
//        else if (level < 30)
//          chance = 85;
//        else if (level < 50)
//          chance = 75;
//        else if (level < 75)
//        {
//          chance = 50;
//          calc_chance = (100 * most)/200;
//          chance = MIN(chance, calc_chance);
//         }
//        else if (level < 100)
//        {
//          chance = 45;
//          calc_chance = (100 * most)/200;
//          chance = MIN(chance, calc_chance);
//        }  
//        else if ( level < 125)
//        {
//          chance = 35;
//          calc_chance = (100 * most)/200;
//          chance = MIN(chance, calc_chance);
//        }  
//        else
//        {
//          chance = 30;
//          calc_chance = (100 * most)/200;
//          chance = MIN(chance, calc_chance);
//        }   
//        
//          
//        if (percent() < chance)
//          return obj_index[rnum].number < MIN(50,most);
//        else
//           return FALSE;
//    }
//	//return obj_index[rnum].number < most;
//}

int ObjVnum( struct obj_data *o)
{
  if (o->item_number >= 0)
     return(obj_index[o->item_number].virt);
  return(-1);
}

/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
    struct obj_data *temp1, *temp2;
  
    if(obj->in_room != NOWHERE)
	obj_from_room(obj);
    else if(obj->carried_by)
	obj_from_char(obj);
    else if (obj->equipped_by) {
	if (obj->eq_pos > -1) {
	    /*
	     **  set players equipment slot to 0; that will avoid the garbage items.
	     */
	    obj->equipped_by->equipment[(int)obj->eq_pos] = 0;
      
	} else {
	    log_msg("Extract on equipped item in slot -1 on:");
	    log_msg(GET_NAME(obj->equipped_by));
	    log_msg(OBJ_NAME(obj));
	    return;
	}
    } else if(obj->in_obj)	{
	temp1 = obj->in_obj;
	if(temp1->contains == obj) /* head of list */
	    temp1->contains = obj->next_content;
	else		{
	    for( temp2 = temp1->contains ;
		temp2 && (temp2->next_content != obj);
		temp2 = temp2->next_content );
      
	    if(temp2) {
		temp2->next_content =
		    obj->next_content; 
	    }
	}
    }
  
    array_delete(&object_list, obj);

    if(obj->corpse_timer)
	event_cancel(obj->corpse_timer, 1);
    obj->corpse_timer = 0;
    
    if(obj->sound_timer)
	event_cancel(obj->sound_timer, 1);
    obj->sound_timer = 0;
    
    if(count_objects && (obj->item_number > 0))
	obj_index[obj->item_number].number--;
    
    for( ; obj->contains; extract_obj(obj->contains)); 
    /* leaves nothing ! */
  
    free_obj(obj);
}

struct obj_data *create_money( int amount )
{
    struct obj_data *obj;
    struct extra_descr_data *new_descr;
    char buf[80];
  
    if(amount<=0) {
	log_msg("ERROR: Try to create negative money.");
	exit(1);
    }
  
    obj = create_object();
    CREATE(new_descr, struct extra_descr_data, 1);
  
    if(amount==1){
	obj->name = ss_make("coin gold");
	obj->short_description = ss_make("a gold coin");
	obj->description = ss_make("One miserable gold coin.");
      
	new_descr->keyword = strdup("coin gold");
	new_descr->description = strdup("One miserable gold coin.");
    } else {
	obj->name = ss_make("coins gold");
	obj->short_description = ss_make("gold coins");
	obj->description = ss_make("A pile of gold coins.");
      
	new_descr->keyword = strdup("coins gold");
	if(amount<10) {
	    sprintf(buf,"There is %d coins.",amount);
	    new_descr->description = strdup(buf);
	} 
	else if (amount<100) {
	    sprintf(buf,"There is about %d coins",10*(amount/10));
	    new_descr->description = strdup(buf);
	}
	else if (amount<1000) {
	    sprintf(buf,"It looks like something round %d coins",100*(amount/100));
	    new_descr->description = strdup(buf);
	}
	else if (amount<100000) {
	    sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
	    new_descr->description = strdup(buf);
	}
	else 
	    new_descr->description = strdup("There is A LOT of coins");
    }
  
    new_descr->next = 0;
    obj->ex_description = new_descr;
  
    obj->obj_flags.type_flag = ITEM_MONEY;
    obj->obj_flags.wear_flags = ITEM_TAKE;
    obj->obj_flags.value[0] = amount;
    obj->obj_flags.cost = amount;
    obj->item_number = -1;
  
    array_insert(&object_list, obj);
  
    return(obj);
}

int obj_sound_timer(object_event* event, int now)
{
    struct obj_data* j = event->object;
    int room;
    
    if(!number(0,5))
    {
	if (j->carried_by)
	    room = j->carried_by->in_room;
	else if (j->equipped_by)
	    room = j->equipped_by->in_room;
	else if (j->in_room != NOWHERE)
	    room = j->in_room;
	else
	    room = room_of_object(j);
	/*
	 *  broadcast to room
	 */
	  
	if (OBJ_ACTION(j))
	    MakeNoise(room, OBJ_ACTION(j), OBJ_ACTION(j));
    }

    event_queue_pulse((event_t*) event,
		      next_pulse(j->obj_flags.value[0]),
		      (event_func) obj_sound_timer,
		      NULL);
    
    return 1;
}

int getFreeAffSlot( struct obj_data *obj)
{
  int i;

  for (i=0; i < MAX_OBJ_AFFECT; i++)
    if (obj->affected[i].location == APPLY_NONE)
      return(i);

  slog("Error in getFreeAffSlot");
  return(-1); /* do a check for -1 when returning from the call */
  /* abort(); */
}

