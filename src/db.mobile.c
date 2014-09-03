#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
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
#include "sound.h"

/* globals */
long mob_count=0;
FILE *mob_f;                          /* file containing mob prototypes  */
struct index_data *mob_index;         /* index table for mobile file     */
int top_of_mobt = 0;                  /* top of mobile index table       */
array_t character_list;		/* global l-list of chars          */

typedef struct 
{
    event_t		event;
    struct char_data*	mob;
} mob_event_t;
int mob_sound_proc(mob_event_t* event, long when);

void random_mob_points(struct char_data *mob);
struct char_data *clone_mobile(struct char_data *mob);

/*
**  distributed monster stuff
*/
int mob_tick_count=0;

/* forward declarations */
struct char_data* read_mobile(int nr);
struct char_data* parse_mobile(FILE* fp, int nr);

void boot_mobiles(const char* file)
{
    array_init(&character_list, 13000, 1024);
    
    if (!(mob_f = fopen(file, "r"))) {
	perror(file);
	exit(0);
    }
    
    mob_index = generate_indices(mob_f, &top_of_mobt);
}

/* create a new mobile */
struct char_data* make_mobile(int nr, int type)
{
    char buf[256];
    int rl;
    struct char_data* mob;
    
    if (type == VIRTUAL)
    {
	if ((rl = real_mobile(nr)) < 0)	{
	    sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
	    log_msg(buf);
	    return(0);
	}
    }
    else if ((rl = nr) < 0) {
	sprintf(buf, "Mobile (R) %d is invalid", rl);
	log_msg(buf);
	return NULL;
    }   
    
#if 1
    if(!mob_index[rl].proto)
	mob_index[rl].proto = read_mobile(rl);
    
    if(mob_index[rl].proto)
	mob = clone_mobile((struct char_data*) (mob_index[rl].proto));
    else
	mob = 0;
#else
    mob = read_mobile(rl);
#endif
    
    if(mob)
    {
        random_mob_points(mob);
	mob->fight_delay = 0; /* Set inital delay in battle --mnemosync*/

	if(mob->player.sounds)
	{
	    mob_event_t*	event;
	    int			when =
	      next_pulse(pulse ? PULSE_SOUND : number(0, PULSE_SOUND));
	    
	    CREATE(event, mob_event_t, 1);
	    event->mob = mob;

	    sprintf(buf, "sound %s", GET_NAME(mob));
	    mob->sound_timer = event_queue_pulse((event_t*) event,
						 when,
						 (event_func) mob_sound_proc,
						 buf);
	}

	array_insert(&character_list, mob);

	mob_index[rl].number++;

	if (mob_index[mob->nr].func && IS_SET(mob->specials.mob_act, ACT_SPEC))
	    (*mob_index[mob->nr].func)(mob, NULL, 0, "", SPEC_INIT);
    }
    
    return mob;
}

void SetAbilities(struct char_data *mob)
{

    int lev;

    lev = GET_LEVEL(mob, WARRIOR_LEVEL_IND);

	if(lev<255) {
	    mob->abilities.str = number(18,25);
	    mob->abilities.intel = number(18,25);
	    mob->abilities.wis  = number(18,25);
	    mob->abilities.dex  = number(18,25);
	    mob->abilities.con  = number(18,25);
	    mob->abilities.cha = number(18,25);
        }
	if(lev<175 && lev > 124) {
	    mob->abilities.str = number(15,25);
	    mob->abilities.intel = number(15,25);
	    mob->abilities.wis  = number(15,25);
	    mob->abilities.dex  = number(15,25);
	    mob->abilities.con  = number(15,25);
	    mob->abilities.cha = number(15,25);
        }
	if(lev<125 && lev > 99) {
	    mob->abilities.str = number(13,25);
	    mob->abilities.intel = number(13,25);
	    mob->abilities.wis  = number(13,25);
	    mob->abilities.dex  = number(13,25);
	    mob->abilities.con  = number(13,25);
	    mob->abilities.cha = number(13,25);
	}    
	if(lev<100 && lev > 39) {
	    mob->abilities.str    = number(11,25);
	    mob->abilities.intel  = number(11,25);
	    mob->abilities.wis  = number(11,25);
	    mob->abilities.dex  = number(11,25);
	    mob->abilities.con  = number(11,25);
	    mob->abilities.cha = number(11,25);
	}
	if(lev<40 && lev>29) {
	    mob->abilities.str  = number(11,20);
	    mob->abilities.intel  = number(11,20);
	    mob->abilities.wis  = number(11,20);
	    mob->abilities.dex  = number(11,20);
	    mob->abilities.con  = number(11,20);
	    mob->abilities.cha = number(11,20);
	}
	if(lev<30 && lev>19) {
	    mob->abilities.str  = number(11,17);
	    mob->abilities.intel  = number(11,17);
	    mob->abilities.wis  = number(11,17);
	    mob->abilities.dex  = number(11,17);
	    mob->abilities.con  = number(11,17);
	    mob->abilities.cha = number(11,17);
	}
	if(lev<20 && lev>9) {
	    mob->abilities.str  = number(11,13);
	    mob->abilities.intel  = number(11,13);
	    mob->abilities.wis  = number(11,13);
	    mob->abilities.dex  = number(11,13);
	    mob->abilities.con  = number(11,13);
	    mob->abilities.cha = number(11,13);
	}
	if(lev<10) {
	    mob->abilities.str  = number(9,13);
	    mob->abilities.intel  = number(9,13);
	    mob->abilities.wis  = number(9,13);
	    mob->abilities.dex  = number(9,13);
	    mob->abilities.con  = number(9,13);
	    mob->abilities.cha = number(9,13);
	}
}

/* read a mobile from a file, starting where we're pointing right now */
struct char_data* clone_mobile(struct char_data *mob)
{
  struct char_data *clone;
  int i;

  CREATE(clone, struct char_data, 1);
  clear_char(clone);
  
  /***** String data *** */
  
  clone->player.name = ss_share(mob->player.name);
  clone->player.short_descr = ss_share(mob->player.short_descr);
  clone->player.long_descr = ss_share(mob->player.long_descr);
  clone->player.description = ss_share(mob->player.description);
  clone->player.title = 0;
  
  /* *** Numeric data *** */
  
  clone->mult_att = mob->mult_att;
  clone->specials.mob_act   = mob->specials.mob_act;
  clone->specials.flags     = mob->specials.flags;
  AFF_FLAGS(clone)	    = AFF_FLAGS(mob);
  clone->specials.alignment = mob->specials.alignment;

  /* Don't copy class, we know it is warrior so it is faster to set it */
  clone->player.clss = CLASS_WARRIOR;
  
  /* The Level array is shared */
  for(i=0; i< MAX_LEVEL_IND; i++)
  clone->player.level[i] = mob->player.level[i];

  clone->player.maxlevel = mob->player.maxlevel;  
  clone->player.minlevel = mob->player.minlevel;
  
  SetAbilities(clone);
 
  clone->points.hitroll = mob->points.hitroll;
  clone->points.armor = mob->points.armor;
  clone->points.max_hit =  mob->points.max_hit;
  clone->points.hit =  mob->points.hit;
  clone->specials.damnodice = mob->specials.damnodice;
  clone->specials.damsizedice =  mob->specials.damsizedice;
  clone->points.damroll =  mob->points.damroll;
    
  clone->points.mana     = 10;
  clone->points.max_mana = 10;   
  clone->points.move     = 150;
  clone->points.max_move = 150;
    
  clone->points.gold = mob->points.gold;
  GET_EXP(clone) = GET_EXP(mob);
  GET_RACE(clone) = GET_RACE(mob);

  clone->specials.position = mob->specials.position;
  clone->specials.default_pos = mob->specials.default_pos;
  clone->player.sex = mob->player.sex;
  clone->immune = mob->immune;
  clone->M_immune = mob->M_immune;
  clone->susc = mob->susc;
  
  clone->player.clss = 0;
    
  clone->player.time.birth = time(0);
  clone->player.time.played	= 0;
  clone->player.time.logon  = time(0);
  clone->player.weight = 200;
  clone->player.height = 198;
  
  clone->specials.binding=0;
  clone->specials.binded_by=0;
 
  GET_COND(clone, 0) = GET_COND(mob, 0);
  GET_COND(clone, 1) = GET_COND(mob, 1);  
  GET_COND(clone, 2) = GET_COND(mob, 2);
    
  clone->specials.apply_saving_throw[0] = mob->specials.apply_saving_throw[0];
  clone->specials.apply_saving_throw[1] = mob->specials.apply_saving_throw[1];
  clone->specials.apply_saving_throw[2] = mob->specials.apply_saving_throw[2];
  clone->specials.apply_saving_throw[3] = mob->specials.apply_saving_throw[3];
  clone->specials.apply_saving_throw[4] = mob->specials.apply_saving_throw[4];
  
 
  /*    
     GETN(cnt);
     */
  /* allow for a variant that mulac's been using.  Normally we
     don't allow a die specfier here, just a simple number... */
  /*
     if((*ptr == 'd') || (*ptr == 'D'))
     {
     ptr++;
     GETN(size);
     if((*ptr != '+') && (*ptr != '-'))
     {
     log_expect("mob file", virt, "+/-", ptr);
     return NULL;
     }
     GETN(mod);
     mob->points.max_hit = dice(cnt, size) + mod;
     }
     else
     mob->points.max_hit = cnt;
     */
 
  clone->player.sounds = 
    ( (mob->player.sounds) ? ss_share(mob->player.sounds) : 0);
  clone->player.distant_snds = 
    ( (mob->player.distant_snds) ? ss_share(mob->player.distant_snds) : 0);

  
  for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
    mob->equipment[i] = 0;
  
  
  clone->nr = mob->nr;
  
  clone->desc = 0;
  

  /* set up distributed movement system */

  /* The following assigns a number from 0-MOB_DIVISOR defining which mobile_acitvity_pulse
     the mob should move on */

  clone->specials.tick = number(0, MOB_MAX_DIVISOR-1);
   
#ifdef JANWORK

  clone->specials.tick = mob_tick_count++;

  if (mob_tick_count == TICK_WRAP_COUNT-1)
    mob_tick_count=0;
#endif  
  mob_count++;  
  return(clone);
}

/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr)
{
    fseek(mob_f, mob_index[nr].pos, 0);
    return parse_mobile(mob_f, nr);
}
#define GETN(fld) \
{ \
    long tmp; \
    if(!parse_number("mob file", #fld, virt, (const char**) &ptr, &tmp)) \
	return 0; \
    fld = tmp; \
}
#define GETUN(fld) \
{ \
    unsigned long tmp; \
    if(!parse_unumber("mob file", #fld, virt, (const char**) &ptr, &tmp)) \
	return 0; \
    fld = tmp; \
}

#define GETDICE(cnt, size, mod) \
{ \
    long tmp, tmp2, tmp3; \
    if(!parse_dice("mob file", virt, (const char**) &ptr, \
		   &tmp, &tmp2, &tmp3)) \
        return 0; \
    cnt = tmp; \
    size = tmp2; \
    mod = tmp3; \
}

int balance_gold(char_data *mob) {
   int gold=0;
   
   gold=(int)((double)GetMaxLevel(mob)/125*5000);
   gold=MAX(gold,1);
   return gold;
}

EXP balance_exp(char_data *mob)
{
   EXP newexp = 0;
   int lev = 0;
   double mod;

   /* Currently removed until mob_class(es) can be factored in */
   lev = GET_LEVEL(mob, WARRIOR_LEVEL_IND);

   /* exp will be a function of the mobs level, its stats, and its barehand for now.
      this should keep it high for the high level mobs, and make it lower for the
      ones that are easier to kill.  We will see =) */
   /* Old system was being abused by builders who loaded down mobs with hp.
      This system fixes that, and rewards more for mob difficulty, than battle length.
      Eventually I would like to add in a factor of multiple classes, so that mobs
      that stun or do some other special skill will give more xp than generic_class mobs. */
     newexp += ((mob->mult_att) * 10);
     newexp += ((mob->points.max_hit) / 2); 
     newexp += ((mob->specials.damnodice * mob->specials.damsizedice) * 10);
     newexp -= ((mob->points.armor) / 10); 
     newexp += mob->abilities.str;
     newexp += mob->abilities.intel;
     newexp += mob->abilities.wis;
     newexp += mob->abilities.dex;
     newexp += mob->abilities.con;
     newexp += (lev * 10);

     newexp *= lev;           

/***** Don't delete the button line, it's syntax that checks for NULL values *****/
/***** When I don't need it anymore, I'll delete it.  (Solaar) *******************/
//char mobClass = (MAX(0, MIN(1, (mob->player.clss)/1)));

     if(lev < 10)
       mod = 1;
     else if (lev < 30)
       mod = 1.5;
     else if (lev < 55)
       mod = 2;
     else if (lev < 75)
       mod = 2.33;
     else if (lev < 130)
       mod = 2.66;
     else if (lev < 170)
       mod = 3;
     else if (lev < 220)
       mod = 3.33;
     else if (lev < 255)
       mod = 4;
     else
       mod = 2;

     newexp = (newexp * mod);

/*   Removed until class(es) can be factored in
     newexp = (int)(newexp * mod);
*/

     return (newexp);
}

/* read a mobile from a file, starting where we're pointing right now */
struct char_data* parse_mobile(FILE* mob_f, int nr)
{
    int i;
    struct char_data *mob;
    char buf[256];
    char* ptr;
    char letter;
    int lev;
    int cnt, size, mod;
    int tmp=0;
    int virt;

    virt = mob_index[nr].virt;
    
    fseek(mob_f, mob_index[nr].pos, 0);
  
    CREATE(mob, struct char_data, 1);
    clear_char(mob);
  
    /***** String data *** */
  
    mob->player.name = ss_fread(mob_f);
    mob->player.short_descr = ss_fread(mob_f);
    mob->player.long_descr = ss_fread(mob_f);
    mob->player.description = ss_fread(mob_f);
    mob->player.title = 0;
  
    /* *** Numeric data *** */
  
    mob->mult_att = 0;

    ptr = fgets(buf, sizeof(buf), mob_f);
    GETN(mob->specials.mob_act);
    
    mob->specials.flags = PLR_ECHO;
    SET_BIT(mob->specials.mob_act, ACT_ISNPC);
  
    GETUN(AFF_FLAGS(mob));
    GETN(mob->specials.alignment);

    mob->player.clss = CLASS_WARRIOR;
  
    while(isspace(*ptr))
	ptr++;
    letter = *ptr++;
    
    if (letter == 'S') {
    
	ptr = fgets(buf, sizeof(buf), mob_f);
    
	/* The new easy monsters */
    
	GETN(lev);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = lev;
	UpdateMaxLevel(mob);
	UpdateMinLevel(mob);

	/* lets give these guys some random stats to make things interesting */

	GETN(mob->points.hitroll);
        mob->points.hitroll = 20-mob->points.hitroll;
	GETN(mob->points.armor);
	mob->points.armor = 10 * mob->points.armor;
    
	GETDICE(cnt, size, mod);
	mob->points.max_hit = MIN(dice(cnt, size) + mod, 32760);
	mob->points.hit = mob->points.max_hit;
    
	GETDICE(mob->specials.damnodice,
		mob->specials.damsizedice,
		mob->points.damroll);
    
	ptr = fgets(buf, sizeof(buf), mob_f);
	
	mob->points.mana = 100;
	mob->points.max_mana = 10;   
    
	mob->points.move = 150;
	mob->points.max_move = 150;
    
	GETN(mob->points.gold);
	if(mob->points.gold == -1)
	{
	    GETN(mob->points.gold);
	    GETN(GET_EXP(mob));
	    GETN(GET_RACE(mob));
	} else {
	    GETN(GET_EXP(mob));
	}

	ptr = fgets(buf, sizeof(buf), mob_f);
	GETN(mob->specials.position);
	GETN(mob->specials.default_pos);
	GETN(mob->player.sex);
	if (mob->player.sex < 3) {
	    mob->immune = 0;
	    mob->M_immune = 0;
	    mob->susc = 0;
	} else if (mob->player.sex < 6) {
	    mob->player.sex -= 3;
	    if(!*ptr || *ptr == '\n')
		ptr = fgets(buf, sizeof(buf), mob_f);
	    GETN(mob->immune);
	    GETN(mob->M_immune);
	    GETN(mob->susc);
	} else {
	    mob->player.sex = 0;
	    mob->immune = 0;
	    mob->M_immune = 0;
	    mob->susc = 0;
	}
    
	mob->player.clss = 0;
    
	mob->player.time.birth = time(0);
	mob->player.time.played	= 0;
	mob->player.time.logon  = time(0);
	mob->player.weight = 200;
	mob->player.height = 198;
   
        mob->specials.binding=0;
        mob->specials.binded_by=0;
 
	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;
    
	for (i = 0; i < 5; i++)
	 mob->specials.apply_saving_throw[i]=MAX(10-GET_LEVEL(mob,WARRIOR_LEVEL_IND), 2);
    
    } else if ((letter == 'A') || (letter == 'N') || (letter == 'B') ||
	       (letter == 'L')) {
    
	if ((letter == 'A') || (letter == 'B') || (letter == 'L')) {
	    GETN(mob->mult_att);
	}
    
	ptr = fgets(buf, sizeof(buf), mob_f);
    
	/* The new easy monsters */
    
	GETN(lev);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = lev;
	UpdateMaxLevel(mob);
	UpdateMinLevel(mob);


	/* lets give these guys some random stats to make things interesting */
    
	GETN(mob->points.hitroll);
	mob->points.hitroll = 20-mob->points.hitroll;
       
	GETN(mob->points.armor);
	mob->points.armor = 10 * mob->points.armor;
	
	GETN(cnt);

	/* allow for a variant that mulac's been using.  Normally we
	   don't allow a die specfier here, just a simple number... */
	if((*ptr == 'd') || (*ptr == 'D'))
	{
	    ptr++;
	    GETN(size);
	    if((*ptr != '+') && (*ptr != '-'))
	    {
		log_expect("mob file", virt, "+/-", ptr);
		return NULL;
	    }
	    GETN(mod);
	    mob->points.max_hit = dice(cnt, size) + mod;
	}
	else
	    mob->points.max_hit = cnt;

	mob->points.hit = mob->points.max_hit;
    
	GETDICE(mob->specials.damnodice,
		mob->specials.damsizedice,
		mob->points.damroll);
    
	
	mob->points.mana = 100;
	mob->points.max_mana = 10;   
    
	mob->points.move = 150;
	mob->points.max_move = 150;
    
	ptr = fgets(buf, sizeof(buf), mob_f);
    
	GETN(mob->points.gold);
	
	if (mob->points.gold == -1) {
	    GETN(mob->points.gold);
	    GETN(mod);
	    GET_EXP(mob) = mod; /*DetermineExp(mob, mod)+mob->points.gold;*/
	    GETN(GET_RACE(mob));
	} else {
	    /*
	      this is where the new exp will come into play
	      */
	    GETN(mod);
	    GET_EXP(mob) = mod; /*DetermineExp(mob, mod)+mob->points.gold;*/
	}

	ptr = fgets(buf, sizeof(buf), mob_f);
	
	GETN(mob->specials.position);
	GETN(mob->specials.default_pos);
    
	GETN(mob->player.sex);
	if (mob->player.sex < 3) {
	    mob->immune = 0;
	    mob->M_immune = 0;
	    mob->susc = 0;
	} else if (mob->player.sex < 6) {
	    mob->player.sex -= 3;
	    if(!*ptr || *ptr == '\n')
		ptr = fgets(buf, sizeof(buf), mob_f);
	    GETN(mob->immune);
	    GETN(mob->M_immune);
	    GETN(mob->susc);
	} else {
	    mob->player.sex = 0;
	    mob->immune = 0;
	    mob->M_immune = 0;
	    mob->susc = 0;
	}

	/*
	 *   read in the sound string for a mobile
	 */
	if (letter == 'L') {
	    mob->player.sounds = ss_fread(mob_f);
	    mob->player.distant_snds = ss_fread(mob_f);
	} else {
	    mob->player.sounds = 0;
	    mob->player.distant_snds = 0;
	}

	mob->player.clss = 0;
    
	mob->player.time.birth = time(0);
	mob->player.time.played	= 0;
	mob->player.time.logon  = time(0);
	mob->player.weight = 200;
	mob->player.height = 198;
    
	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;
    
	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
    
    } else {			/* The old monsters are down below here */
    
	ptr = fgets(buf, sizeof(buf), mob_f);
    
	GETN(mob->abilities.str);
	GETN(mob->abilities.intel); 
	GETN(mob->abilities.wis);
	GETN(mob->abilities.dex);
	GETN(mob->abilities.con);


	ptr = fgets(buf, sizeof(buf), mob_f);
    
	GETN(cnt);
	GETN(size);
	mob->points.max_hit = number(cnt, size);
	mob->points.hit = mob->points.max_hit;
    
	GETN(mob->points.armor);
	mob->points.armor *= 10;
    
	GETN(mob->points.mana);
	mob->points.max_mana = mob->points.mana;
    
	GETN(mob->points.move);
	mob->points.max_move = mob->points.move;
    
	GETN(mob->points.gold);
    
	GETN(GET_EXP(mob));
    
	ptr = fgets(buf, sizeof(buf), mob_f);
	
	GETN(mob->specials.position);
	GETN(mob->specials.default_pos);
	GETN(mob->player.sex);

	GETN(mob->player.clss);
	mob->player.clss = 0;
	
	GETN(GET_LEVEL(mob, WARRIOR_LEVEL_IND));
	UpdateMaxLevel(mob);
	UpdateMinLevel(mob);

        lev = GET_LEVEL(mob, WARRIOR_LEVEL_IND);
    
        
	GETN(cnt);
	mob->player.time.birth = time(0);
	mob->player.time.played	= 0;
	mob->player.time.logon  = time(0);
    
	GETN(mob->player.weight);
	GETN(mob->player.height);
    
	ptr = fgets(buf, sizeof(buf), mob_f);
	
	for (i = 0; i < 3; i++)
	{
	    GETN(GET_COND(mob, i));
	}
    
	ptr = fgets(buf, sizeof(buf), mob_f);

	for (i = 0; i < 5; i++)
	{
	    GETN(mob->specials.apply_saving_throw[i]);
	}
    
	/* Set the damage as some standard 1d4 */
	mob->points.damroll = 0;
	mob->specials.damnodice = 1;
	mob->specials.damsizedice = 6;
    
	/* Calculate THAC0 as a formular of Level */
	mob->points.hitroll = MAX(1, GET_LEVEL(mob,WARRIOR_LEVEL_IND)-3);
    }

    /* experimental exp function to balance exp per mob */
    GET_EXP(mob) = balance_exp(mob);
    GET_GOLD(mob) = balance_gold(mob);
    SetAbilities(mob);
  
    for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
	mob->equipment[i] = 0;
  
    mob->nr = nr;
  
    mob->desc = 0;
  
    if (!IS_SET(mob->specials.mob_act, ACT_ISNPC))
	SET_BIT(mob->specials.mob_act, ACT_ISNPC);
  
    if(mob_index[nr].func && !IS_SET(mob->specials.mob_act, ACT_SPEC))
    {
	char buf[256];
	sprintf(buf, "Mob has special proc, but no ACT_SPEC: %d: %s",
		mob_index[nr].virt, GET_IDENT(mob));
	log_msg(buf);

	mob_index[nr].func = NULL;
    }
    
    /* insert in list */
    if(mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*500)
       mob->points.gold = GET_LEVEL(mob, WARRIOR_LEVEL_IND)*500;

    /* shouldn't ever get this log again */
    if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*500) {
	char buf[200];
	sprintf(buf, "%s has gold > level * 1500 (%d)", GET_NAME(mob),
		mob->points.gold);
	log_msg(buf);
    }

    /* set up distributed movement system */

    mob->specials.tick = mob_tick_count++;

    if (mob_tick_count == TICK_WRAP_COUNT -1)
	mob_tick_count=0;
  
#if BYTE_COUNT
    fprintf(stderr,"Mobile [%d]: byte count: %d\n", mob_index[nr].virt, bc);
#endif


    mob_count++;  
    return(mob);
}

/* let's randomize hp, gold, and exp by 20% so everything isn't the same */
void random_mob_points(struct char_data *mob)
{
  int random;
  
  random = (int)((double)(mob->points.max_hit) * 0.2);
  random = number(0, random);
  random = (number(0, 1) ? random : -random);
  mob->points.hit = mob->points.max_hit += random;
  random = (int)((double)(mob->points.gold) * 0.2);
  random = number(0, random);
  random = (number(0, 1) ? random : -random);
  mob->points.gold += random;
  random = (int)((double)(GET_EXP(mob)) * 0.2);
  random = number(0, random);
  random = (number(0, 1) ? random : -random);
  GET_EXP(mob) += random;
}


/* returns the real number of the monster with given virtual number */
int real_mobile(int virt)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_mobt;

    /* perform binary search on mob-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((mob_index + mid)->virt == virt)
	    return(mid);
	if (bot > top)
	    return(-1);
	if ((mob_index + mid)->virt > virt)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

int DetermineExp( struct char_data *mob, int exp_flags)
{

int base;
int phit;
int sab;
char buf[200];

   if (exp_flags > 10) { 
     sprintf(buf, "Exp flags on %s are > 10 (%d)", GET_NAME(mob), exp_flags);
     log_msg(buf);
   }
/* 
reads in the monster, and adds the flags together 
for simplicity, 1 exceptional ability is 2 special abilities 
*/

    if (GetMaxLevel(mob) < 0)
       return(1);

    switch(GetMaxLevel(mob)) {

    case 0:   base = 5;
              phit = 1;
              sab = 10;
              break;

    case 1:   base = 10;
              phit = 1;
              sab =  15;
              break;

    case 2:   base = 20;
              phit = 2;
              sab =  20;
              break;


    case 3:   base = 35;
              phit = 3;
              sab =  25;
              break;

    case 4:   base = 60;
              phit = 4;
              sab =  30;
              break;

    case 5:   base = 90;
              phit = 5;
              sab =  40;
              break;

    case 6:   base = 150;
              phit = 6;
              sab =  75;
              break;

    case 7:   base = 225;
              phit = 8;
              sab =  125;
              break;

    case 8:   base = 600;
              phit = 12;
              sab  = 175;
              break;

    case 9:   base = 900;
              phit = 14;
              sab  = 300;
              break;

    case 10:   base = 1100;
              phit  = 15;
              sab   = 450;
              break;

    case 11:   base = 1300;
              phit  = 16;
              sab   = 700;
              break;

    case 12:   base = 1550;
              phit  = 17;
              sab   = 700;
              break;

    case 13:   base = 1800;
              phit  = 18;
              sab   = 950;
              break;

    case 14:   base = 2100;
              phit  = 19;
              sab   = 950;
              break;

    case 15:   base = 2400;
              phit  = 20;
              sab   = 1250;
              break;

    case 16:   base = 2700;
              phit  = 23;
              sab   = 1250;
              break;

    case 17:   base = 3000;
              phit  = 25;
              sab   = 1550;
              break;

    case 18:   base = 3500;
              phit  = 28;
              sab   = 1550;
              break;

    case 19:   base = 4000;
              phit  = 30;
              sab   = 2100;
              break;

    case 20:   base = 4500;
              phit  = 33;
              sab   = 2100;
              break;

    case 21:   base = 5000;
              phit  = 35;
              sab   =  2600;
              break;

    case 22:   base = 6000;
              phit  = 40;
              sab   = 3000;
              break;

    case 23:   base = 7000;
              phit  = 45;
              sab   = 3500;
              break;

    case 24:   base = 8000;
              phit  = 50;
              sab   = 4000;
              break;

    case 25:   base = 9000;
              phit  = 55;
              sab   = 4500;
              break;

    case 26:   base = 10000;
              phit  = 60;
              sab   =  5000;
              break;

    case 27:   base = 12000;
              phit  = 70;
              sab   = 6000;
              break;

    case 28:   base = 14000;
              phit  = 80;
              sab   = 7000;
              break;

    case 29:   base = 16000;
              phit  = 90;
              sab   = 8000;
              break;

    case 30:   base = 20000;
              phit  = 100;
              sab   = 10000;
              break;

      default : base = 25000;
                phit = 150;
                sab  = 20000;
                break;
    }

    return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));
}

int MobVnum( struct char_data *c)
{
  if (IS_NPC(c)) {
    return(mob_index[c->nr].virt);
  }
  return(0);
}

int mob_sound_proc(mob_event_t* event, long now)
{
    char buffer[256];
    struct char_data* ch = event->mob;
    
    if(!number(0, 5))
    {
	if (ch->specials.default_pos > POSITION_SLEEPING)
	{
	    if (GET_POS(ch) > POSITION_SLEEPING)
	    {
		/*
		 *  Make the sound;
		 */
		MakeNoise(ch->in_room,
			  ss_data(ch->player.sounds),
			  ss_data(ch->player.distant_snds));
	    }
	    else if (GET_POS(ch) == POSITION_SLEEPING)
	    {
		/*
		 * snore 
		 */	 
		sprintf(buffer, "%s snores loudly.\n\r", GET_NAME(ch));
		MakeNoise(ch->in_room, buffer, "You hear a loud snore nearby.\n\r");
	    }
	}
	else if (GET_POS(ch) == ch->specials.default_pos)
	{
	    /*
	     * Make the sound
	     */       
	    MakeNoise(ch->in_room,
		      ss_data(ch->player.sounds),
		      ss_data(ch->player.distant_snds));
	}
    }

    event_queue_pulse((event_t*) event,
		      next_pulse(PULSE_SOUND),
		      (event_func) mob_sound_proc,
		      NULL);

    return 1;
}

struct mob_variable *mvar_find(struct char_data *ch, char *name) {
   
   
   return NULL;
}

struct mob_variable *mvar_create(struct char_data *ch, char *name, char *val) {
   return NULL;
}

void mvar_kill(struct char_data *ch, struct mob_variable *mv) {
   return;
}

void mvar_killall(struct char_data *ch, struct mob_variable *mv) {
   return;
}
