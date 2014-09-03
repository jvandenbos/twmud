
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#if USE_unistd
#include <unistd.h>
#endif
    
#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "editor.h"
#include "multiclass.h"
#include "recept.h"
#include "act.h"
#include "utility.h"
#include "modify.h"
#include "find.h"
#include "util_str.h"
#include "constants.h"
#include "interpreter.h"
#include "proto.h"
#include "trackchar.h"

#define TP_MOB    0
#define TP_OBJ	   1
#define TP_ERROR  2

/* globals */
struct room_data *world;              /* dyn alloc'ed array of rooms     */

extern flagdata edit_room_flag[];
extern flagdata edit_affected[];
extern flagdata edit_affected2[];
extern flagdata edit_mob_action[];

const char *string_fields[] =
{
	"name",
	"short",
	"long",
	"description",
	"title",
	"delete-description",
	"\n"
};

const char *room_fields[] =
{
	"name",		/* 1 */
	"description",
	"fs",
	"exit",
	"exdsc",	/* 5 */
        "extra",
	"river",
	"teleport",
	"tunnel",	/* 9 */
	"\n"
};

/* maximum length for text field x+1 */
int length[] =
{
	30,
	60,
	256,
	240,
	60
};


int room_length[] =
{
	80,
	1024,
        50,
	50,
	512,
	512,
        50,
	100,
        50
};

char *skill_fields[] = 
{
	"learned",
	"affected",
	"duration",
	"recognize",
	"\n"
};

int max_value[] =
{
	255,
	255,
	10000,
	1
};

/* ************************************************************************
*  modification of allocated strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
    int terminator = 0, len;
    int olen;
    char buf[256];
    
#if LINE_EDITOR
    /* do nothing if we are paging through a string */
    if(d->showstr_point) {
	show_string(d, str);
	return;
    }
 
    /* before we do anything, lets call the handy-dandy editor */
    if( editor_parser(d, str) ) return;
#endif

    terminator = 0;
    
    /* determine if this is the terminal string, and truncate if so */
    strcpy(buf, str);
    len = strlen(buf);
    
    if((len > 0) && (buf[len - 1] == '@'))
    {
	len--;
	terminator = 1;
    }
    else
    {
	buf[len++] = '\n';
	buf[len++] = '\r';
    }
    buf[len] = 0;
    
    if(d->str)
	olen = *d->str ? strlen(*d->str) : 0;
    else
	olen = *d->sstr ? strlen(ss_data(*d->sstr)) : 0;
    
    if ((len + olen) > d->max_str)
    {
	send_to_char("String too long - Truncated.\n\r", d->character);
	len = d->max_str - olen;
	buf[len] = 0;
	terminator = 1;
    }

    if(d->str)
    {
	if (!(*d->str))
	    *d->str = strdup(buf);
        else
	{
	    RECREATE(*d->str, char, olen + len + 1);
	    strcpy(*d->str + olen, buf);
	}

    }
    else if(d->sstr)
    {
	if(!(*d->sstr))
	    *d->sstr = ss_make(buf);
	else
	{
	    sstring_t* new_ss;

	    new_ss = ss_empty(olen + len + 1);
	    strcpy(ss_data(new_ss), ss_data(*d->sstr));
	    strcpy(ss_data(new_ss) + olen, buf);
	    ss_free(*d->sstr);
	    *d->sstr = new_ss;
	}
    }

    if (terminator)
    {
	d->str = 0;
	d->sstr = 0;
	if (d->connected == CON_EXDSCR)	{
	    SEND_TO_Q(MENU, d);
	    d->connected = CON_SLCT;
	}
    }
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
	char buf[MAX_STRING_LENGTH];

	/* determine type */
	arg = one_argument(arg, buf);
	if (is_abbrev(buf, "char"))
	   *type = TP_MOB;
	else if (is_abbrev(buf, "obj"))
	   *type = TP_OBJ;
	else {
		*type = TP_ERROR;
		return;
	}

	/* find name */
	arg = one_argument(arg, name);

	/* field name and number */
	arg = one_argument(arg, buf);
	if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
	   return;

	/* string */
	for (; isspace(*arg); arg++);
	for (; (*string = *arg); arg++, string++);

	return;
}
	
	 


/* modification of allocated strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
    char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
    struct extra_descr_data *ed, *tmp;
    int field, type;
    struct char_data *mob;
    struct obj_data *obj;
    if (IS_NPC(ch))
	return;
  
    quad_arg(arg, &type, name, &field, string);
  
    if (type == TP_ERROR)
    {
	send_to_char(
"Syntax:\n\rstring ('obj'|'char') <name> <field> [<string>].\n\r",
		     ch);
	return;
    }
  
    if (!field)	{
	send_to_char("No field by that name. Try 'help string'.\n\r",
		     ch);
	return;
    }
  
    if (type == TP_MOB)	{
	/* locate the beast */
	if (!(mob = get_char_vis(ch, name))) {
	    send_to_char("I don't know anyone by that name...\n\r", ch);
	    return;
	}
    
	switch(field)	{
	case 1:
	    if (!*string) {
		send_to_char("You have to supply a name!\n\r", ch);
		return;
	    }

	    if(IS_PC(mob))
	    {
		if(TRUST(ch) < TRUST_IMP)
		{
		    send_to_char("You can't change that field for players.",
				 ch);
		    return;
		}
		if(IS_NPC(mob))
		{
		    send_to_char("You can't change that field for polies.",
				 ch);
		    return;
		}
#if 0
		if(find_name(string) != -1)
		{
		    send_to_char("There is already a player by that name.",
				 ch);
		    return;
		}
#endif
		if(_parse_name(string, string))
		{
		    send_to_char("That's not a legal character name!", ch);
		    return;
		}
	        
	        DelChar(ss_data(mob->player.name), TRUST(mob));
		ss_free(mob->player.name);
		ss_free(mob->player.short_descr);
		mob->player.name = ss_make(string);
		mob->player.short_descr = ss_share(mob->player.name);
		do_save(mob, "", 69);
	        TrackingSystem.CheckChar(mob);
		send_to_char("WARNING: You have changed the name of a "
			     "player.\n\r", ch);
		return;
	    }
	    ch->desc->sstr = &mob->player.name;
	    break;
	case 2:
	    if (!IS_NPC(mob) && (TRUST(ch) < TRUST_LORD)){
		send_to_char("That field is for monsters only.\n\r", ch);
		return;
	    }
	    if (!*string) {
		send_to_char("You have to supply a description!\n\r", ch);
		return;
	    }
	    ch->desc->sstr = &mob->player.short_descr;
	    break;
	case 3:
	    if (!IS_NPC(mob)) {
		send_to_char("That field is for monsters only.\n\r", ch);
		return;
	    }
	    ch->desc->sstr = &mob->player.long_descr;
	    break;
	case 4:
	    ch->desc->sstr = &mob->player.description;
	    break;
	case 5:
	    if (IS_NPC(mob))  {
		send_to_char("Monsters have no titles.\n\r",ch);
		return;
	    }
	    if ((TRUST(ch) >= TRUST(mob)) && (ch != mob)) 
		ch->desc->sstr = &mob->player.title;
	    else {
		send_to_char("Sorry, can't set the title of someone of higher level.\n\r", ch);
		return;
	    }
	    break;
	default:
	    send_to_char("That field is undefined for monsters.\n\r", ch);
	    return;
	    break;
	}
    } else {			/* type == TP_OBJ */
	/* locate the object */
	if (!(obj = get_obj_vis(ch, name)))    	{
	    send_to_char("Can't find such a thing here..\n\r", ch);
	    return;
	}
    
	switch(field)  	{
      
	case 1: 
	    if (!*string) {
		send_to_char("You have to supply a keyword.\n\r", ch);
		return;
	    } else {
		ch->desc->sstr = &obj->name;
		break;
	    }
	    break;
	case 2: 
	    if (!*string) {
		send_to_char("You have to supply a keyword.\n\r", ch);
		return;
	    } else {
		ch->desc->sstr = &obj->short_description; 
		break;
	    }
	case 3: ch->desc->sstr = &obj->description; break;
	case 4:
	    if (!*string)  	{
		send_to_char("You have to supply a keyword.\n\r", ch);
		return;
	    }
	    /* try to locate extra description */
	    for (ed = obj->ex_description; ; ed = ed->next)
		if (!ed) {
		    CREATE(ed , struct extra_descr_data, 1);
		    ed->next = obj->ex_description;
		    obj->ex_description = ed;
		    CREATE(ed->keyword, char, strlen(string) + 1);
		    strcpy(ed->keyword, string);
		    ed->description = 0;
		    ch->desc->str = &ed->description;
		    send_to_char("New field.\n\r", ch);
		    break;
		}
		else if (!str_cmp(ed->keyword, string)) /* the field exists */
		{
		    FREE(ed->description);
		    ed->description = 0;
		    ch->desc->str = &ed->description;
		    send_to_char(
				 "Modifying description.\n\r", ch);
		    break;
		}
	    ch->desc->max_str = MAX_STRING_LENGTH;
	    return;		/* the stndrd (see below) procedure does not apply here */
	    break;
	case 6:			/* deletion */
	    if (!*string)  	{
		send_to_char("You must supply a field name.\n\r", ch);
		return;
	    }
	    /* try to locate field */
	    for (ed = obj->ex_description; ; ed = ed->next)
		if (!ed) {
		    send_to_char("No field with that keyword.\n\r", ch);
		    return;
		} else if (!str_cmp(ed->keyword, string)) {
		    FREE(ed->keyword);
		    if (ed->description)
			FREE(ed->description);
	  
		    /* delete the entry in the desr list */						
		    if (ed == obj->ex_description)
			obj->ex_description = ed->next;
		    else {
			for(tmp = obj->ex_description; tmp->next != ed; 
			    tmp = tmp->next);
			tmp->next = ed->next;
		    }
		    FREE(ed);
	  
		    send_to_char("Field deleted.\n\r", ch);
		    return;
		}
	    break;				
	default:
	    send_to_char(
			 "That field is undefined for objects.\n\r", ch);
	    return;
	    break;
	}
    }
  
    if(ch->desc->str)
    {
	if (*ch->desc->str)
	    FREE(*ch->desc->str);
    
	if (*string) {		/* there was a string in the argument array */ 
	    if (strlen(string) > (size_t) length[field - 1])	{
		send_to_char("String too long - truncated.\n\r", ch);
		*(string + length[field - 1]) = '\0';
	    }
	    CREATE(*ch->desc->str, char, strlen(string) + 1);
	    strcpy(*ch->desc->str, string);
	    ch->desc->str = 0;
	    send_to_char("Ok.\n\r", ch);
	} else {		/* there was no string. enter string mode */
	    send_to_char("Enter string. terminate with '@'.\n\r", ch);
	    *ch->desc->str = 0;
	    ch->desc->max_str = length[field - 1];
	}
    }
    else if(ch->desc->sstr)
    {
	ss_free(*ch->desc->sstr);
	
	if (*string) {		/* there was a string in the argument array */ 
	    if (strlen(string) > (size_t) length[field - 1])	{
		send_to_char("String too long - truncated.\n\r", ch);
		*(string + length[field - 1]) = '\0';
	    }
	    *ch->desc->sstr = ss_make(string);
	    ch->desc->sstr = 0;
	    send_to_char("Ok.\n\r", ch);
	} else {		/* there was no string. enter string mode */
	    send_to_char("Enter string. terminate with '@'.\n\r", ch);
	    *ch->desc->sstr = 0;
	    ch->desc->max_str = length[field - 1];
	}
    }	
}




void bisect_arg(char *arg, int *field, char *string)
{
    char buf[MAX_INPUT_LENGTH];
  
    /* field name and number */
    arg = one_argument(arg, buf);
    if (!(*field = old_search_block(buf, 0, strlen(buf), room_fields, 0)))
	return;
  
    /* string */
    for (; isspace(*arg); arg++);
    for (; (*string = *arg); arg++, string++);
  
    return;
}

/*
unsigned int parse_affection(unsigned int curflags, char *args)
{
    unsigned int rf=0;
    int sec=0;
    char choice[80];
    char buf[80];
    int a;

    args = one_argument(args, choice);
    while(*args==' ') args++;
    if(!*args) return ~1;
    if(!is_number(choice)) return ~1;

    struct flagdata {
	unsigned int flag;
	char name[30];
	int checked;
    } FD[] = {
	{AFF_BLIND,		"blind",		0},
	{AFF_INVISIBLE,		"invisible",		0},
	{AFF_REGENERATE,	"regenerate",		0},
	{AFF_DETECT_INVISIBLE,	"detect_invis",		0},
	{AFF_SENSE_AURA,	"sense_aura",		0},
	{AFF_SENSE_LIFE,	"sense_life",		0},
	{AFF_LIFE_PROT,		"life_protection",	0},
	{AFF_SANCTUARY,		"sanctuary",		0},
	{AFF_GROUP,		"group",		0},
	{AFF_BERSERK,		"berserk",		0},
	{AFF_CURSE,		"curse",		0},
	{AFF_FLYING,		"flying",		0},
	{AFF_POISON,		"poison",		0},
	{AFF_ILLUSION,		"illusion",		0},
	{AFF_PARALYSIS,		"paralysis",		0},
	{AFF_INFRAVISION,	"infravision",		0},
	{AFF_WATERBREATH,	"waterbreath",		0},
	{AFF_SLEEP,		"sleep",		0},
	{AFF_DODGE,		"dodge",		0},
	{AFF_SNEAK,		"sneak",		0},
	{AFF_HIDE,		"hide",			0},
	{AFF_SILENCE,		"silence",		0},
	{AFF_CHARM,		"charm",		0},
	{AFF_FOLLOW,		"follow",		0},
	{AFF_UNDEF_1,		"undef_1",		0},
	{AFF_TRUE_SIGHT,	"true_sight",		0},
	{AFF_SCRYING,		"scrying",		0},
	{AFF_FIRESHIELD,	"fireshield",		0},
	{AFF_CONTINUAL_DARK,	"continual_dark",	0},
	{AFF_MEDITATE,		"meditate",		0},
	{AFF_GREAT_SIGHT,	"great_sight",		0},
	{AFF_CONTINUAL_LIGHT,	"continual_light",	0},
	{0},
    };

    while(1==1) {
	args = one_argument(args, choice);
	while(*args==' ') args++;
	if(!*choice) return rf;
	if(is_number(choice)) return atoi(choice);
	if(is_abbrev(choice, "none")) return 0;

	for(a=0;FD[a].flag;a++)
	    if(is_abbrev(choice+1, FD[a].name) && (!FD[a].checked)) {
		switch(*choice) {
		case '-':
		    REMOVE_BIT(rf, FD[a].flag);
		    break;
		default:
		    SET_BIT(rf, FD[a].flag);
		    break;
		}
		FD[a].checked++;
		break;
	    }
    }

    return rf;
}

unsigned int parse_affected_by(unsigned int curflags, char *args) {
    unsigned int rf=0;
    int sec=0;
    char choice[80];
    char buf[80];
    int a;

    args = one_argument(args, choice);
    while(*args==' ') args++;
    if(!*args) return ~1;
    if(!is_number(choice)) return ~1;

    struct flagdata {
	unsigned int flag;
	char name[30];
	int checked;
    } FD[] = {
	{AFF_BLIND,		"blind",		0},
	{AFF_INVISIBLE,		"invisible",		0},
	{AFF_REGENERATE,	"regenerate",		0},
	{AFF_DETECT_INVISIBLE,	"detect_invis",		0},
	{AFF_SENSE_AURA,	"sense_aura",		0},
	{AFF_SENSE_LIFE,	"sense_life",		0},
	{AFF_LIFE_PROT,		"life_protection",	0},
	{AFF_SANCTUARY,		"sanctuary",		0},
	{AFF_GROUP,		"group",		0},
	{AFF_BERSERK,		"berserk",		0},
	{AFF_CURSE,		"curse",		0},
	{AFF_FLYING,		"flying",		0},
	{AFF_POISON,		"poison",		0},
	{AFF_ILLUSION,		"illusion",		0},
	{AFF_PARALYSIS,		"paralysis",		0},
	{AFF_INFRAVISION,	"infravision",		0},
	{AFF_WATERBREATH,	"waterbreath",		0},
	{AFF_SLEEP,		"sleep",		0},
	{AFF_DODGE,		"dodge",		0},
	{AFF_SNEAK,		"sneak",		0},
	{AFF_HIDE,		"hide",			0},
	{AFF_SILENCE,		"silence",		0},
	{AFF_CHARM,		"charm",		0},
	{AFF_FOLLOW,		"follow",		0},
	{AFF_UNDEF_1,		"undef_1",		0},
	{AFF_TRUE_SIGHT,	"true_sight",		0},
	{AFF_SCRYING,		"scrying",		0},
	{AFF_FIRESHIELD,	"fireshield",		0},
	{AFF_CONTINUAL_DARK,	"continual_dark",	0},
	{AFF_MEDITATE,		"meditate",		0},
	{AFF_GREAT_SIGHT,	"great_sight",		0},
	{AFF_CONTINUAL_LIGHT,	"continual_light",	0},
	{0},
    };

    while(1==1) {
	args = one_argument(args, choice);
	while(*args==' ') args++;
	if(!*choice) return rf;
	if(is_number(choice)) return atoi(choice);
	if(is_abbrev(choice, "none")) return 0;

	for(a=0;FD[a].flag;a++)
	    if(is_abbrev(choice+1, FD[a].name) && (!FD[a].checked)) {
		switch(*choice) {
		case '-':
		    REMOVE_BIT(rf, FD[a].flag);
		    break;
		default:
		    SET_BIT(rf, FD[a].flag);
		    break;
		}
		FD[a].checked++;
		break;
	    }
    }

    return rf;
}

unsigned int parse_redit(unsigned int curflags, char *args) {
    unsigned int rf=0;
    int sec=0;
    char choice[80];
    char buf[80];
    int a;

    args = one_argument(args, choice);
    while(*args==' ') args++;
    if(!*args) return ~1;
    if(!is_number(choice)) return ~1;

    struct flagdata {
	unsigned int flag;
	char name[30];
	int checked;
    } FD[] = {
	{DARK,          "dark",          0},
	{DEATH,         "death",         0},
	{NO_MOB,        "no_mob",        0},
	{INDOORS,       "indoors",       0},
	{PEACEFUL,      "peaceful",      0},
	{NOSTEAL,       "nosteal",       0},
	{NO_TRAVEL_OUT, "no_travel_out", 0},
	{NO_MAGIC,      "no_magic",      0},
	{TUNNEL,        "tunnel",        0},
	{NO_TRAVEL_IN,  "no_travel_in",  0},
	{SILENCE,       "silence",       0},
	{NO_PUSH,       "no_push",       0},
	{IMMORT_RM,     "immort_rm",     0},
	{GOD_RM,        "god_rm",        0},
	{NO_RECALL,     "no_recall",     0},
	{ARENA,         "arena",         0},
	{BRUJAH,	"brujah",	 0},
	{curflags,      "current",       0},
	{0},
    };

    while(1==1) {
	args = one_argument(args, choice);
	while(*args==' ') args++;
	if(!*choice) return rf;
	if(is_number(choice)) return atoi(choice);
	if(is_abbrev(choice, "none")) return 0;

	for(a=0;FD[a].flag;a++)
	    if((is_abbrev(choice, FD[a].name) ||
		is_abbrev(choice+1, FD[a].name)) &&
	       (!FD[a].checked)) {
		switch(*choice) {
		case '-':
		    REMOVE_BIT(rf, FD[a].flag);
		    break;
		default:
		    SET_BIT(rf, FD[a].flag);
		    break;
		}
		FD[a].checked++;
		break;
	    }
    }

    return rf;
}
*/

unsigned int parse_edit_flags(unsigned int curflags, char *args, struct flagdata *FD) {
    unsigned int rf=0;
    char choice[80];
    int a, current=-1;

    args = one_argument(args, choice);
    while(*args==' ') args++;
    if(!*args) return ~1;
    if(!is_number(choice)) return ~1;

    while(FD[++current].flag);
    FD[--current].flag = curflags;

    while(1==1) {
	args = one_argument(args, choice);
	while(*args==' ') args++;
	if(!*choice) return rf;
	if(is_number(choice)) return atoi(choice);
	if(is_abbrev(choice, "none")) return 0;

	for(a=0;FD[a].flag;a++)
	    if((is_abbrev(choice, FD[a].name) ||
		is_abbrev(choice+1, FD[a].name))) {
		switch(*choice) {
		case '-':
		    REMOVE_BIT(rf, FD[a].flag);
		    break;
		default:
		    SET_BIT(rf, FD[a].flag);
		    break;
		}
		break;
	    }
    }

    FD[current].flag=1;

    return rf;
}

void do_edit(struct char_data *ch, char *arg, int cmd)
{
  int field, dflags, dir, exroom, dkey, rspeed, rdir,
  tele_room, tele_time, tele_mask, moblim, tele_cnt, a, multi=0;
  unsigned int r_flags;
  int s_type, rmin, rmax;
  char string[512], dirstr[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  struct extra_descr_data *ed, *tmp;
  struct room_data *rp;
  
  rp = real_roomp(rmin = rmax = ch->in_room);
  
  if (IS_NPC(ch))
    return;

  if (!ch->desc)	/* someone is forced to do something. can be bad! */
    return;		/* the ch->desc->str field will cause problems... */

  one_argument(arg, buf);
  if(isdigit(*buf)) {
    arg = one_argument(arg, buf);
    rmin=atoi(buf);
    arg = one_argument(arg, buf);
    if(isdigit(*buf)) {
      rmax=atoi(buf);
      multi = -1;
    } else {
      send_to_char("Must be in the format: edit <min> <max> <command>\n\r", ch);
      rmin=ch->in_room;
    }
  }

  bisect_arg(arg, &field, string);
  
  if (!field)	{
    send_to_char("No field by that name. Try 'help redit'.\n\r", ch);
    return;
  }
  
  r_flags = ~1;
  s_type = -1;
  
  if((rmin<0) || (rmin>WORLD_SIZE) || (rmax<0) ||
     (rmax>WORLD_SIZE)||(rmin>rmax)) {
    send_to_char("Format for a range is: redit <min> <max> <command>", ch);
    return;
  }

  for(a=rmin;a<=rmax;a++)
    if(real_roomp(a)==NULL) CreateOneRoom(a);

  switch(field) {
    
  case 1: ch->desc->str = &rp->name; break;
  case 2: ch->desc->str = &rp->description; break;
  case 3: sscanf(string,"%d ",&s_type);
    r_flags = parse_edit_flags((real_roomp(ch->in_room))->room_flags, string, edit_room_flag);
    if ((r_flags == (unsigned int) ~1)  || (s_type < 0) || (s_type > 11)) {
      send_to_char("Flags must be 0 or positive, sectors must be from 0 to 11\n\r",ch);
      send_to_char("Usage: edit fs <flags> <sector_type>\n\r",ch);
      send_to_char("  or   edit fs <sector> <+-flag word>\n\r",ch);
      return;
    }

    for(a=rmin;a<=rmax;a++) {
      rp = real_roomp(a);
      rp->room_flags = r_flags;
      rp->sector_type = s_type;
      if (rp->sector_type == SECT_WATER_NOSWIM) {
        rp->river_speed = 0;
        rp->river_dir = 0;
      }
    }

    if (rp->sector_type == SECT_WATER_NOSWIM) {
      send_to_char("You need to do speed and flow for this river.\n\r",ch);
      send_to_char("(Set to 0 by default)\n\r",ch);
    }
    return;
    break;
    
  case 4:
    strcpy(dirstr, "");
    dflags = 0;
    dkey = 0;
    exroom = -1;
    sscanf(string, "%s %d %d %d ", dirstr, &exroom, &dflags, &dkey);

    /* check if the exit exists */
    if((dir = search_block(dirstr, dirs, 0)) == -1)
    {
      send_to_char("Usage: edit exit <dir> <to_room> <doorflags> <key>\n\r",
		   ch);
      send_to_char("Where dir is one of these: n e s w u d\n\r", ch);
      return;
    }
    
    if (rp->dir_option[dir]) {
      send_to_char("Modifying exit.\n\r",ch);
      rp->dir_option[dir]->exit_info = dflags;
      rp->dir_option[dir]->key = dkey;
      if (real_roomp(exroom) != NULL) {
	rp->dir_option[dir]->to_room = exroom;
      } else {
	send_to_char("Deleting exit.\n\r",ch);
	FREE(rp->dir_option[dir]);
	rp->dir_option[dir] = 0;
	return;
      }
    } else if (real_roomp(exroom)==NULL) {
      send_to_char("That target room doesn't seem to exist.\n\r", ch);
      return;
    } else {
      send_to_char("New exit.\n\r",ch);
      CREATE(rp->dir_option[dir], struct room_direction_data, 1);
      rp->dir_option[dir]->exit_info = dflags;
      rp->dir_option[dir]->key = dkey;
      rp->dir_option[dir]->to_room = exroom;
    }
    
    if (rp->dir_option[dir]->exit_info>0) {
      string[0] = 0;
      send_to_char("Enter keywords.  Use only 1 line.\n\r",ch);
      send_to_char("Terminate with an @ on the same line.\n\r",ch);
      ch->desc->str = &rp->dir_option[dir]->keyword; 
      break;
    } else {
      return;
    }   
    
  case 5: dir = -1; strcpy(dirstr,"");
    sscanf(string, "%s", dirstr);
    if((dir = search_block(dirstr, dirs, 0)) == -1)
    {
      send_to_char("Usage: edit exdsc <dir>\n\r", ch);
      send_to_char("Where dir is one of these: n e s w u d\n\r", ch);
      send_to_char("You will be prompted for text.\n\r", ch);
      return;
    }
    send_to_char("Enter text, terminate with '@' on a blank line\n\r",ch);
    string[0] = 0;
    if (rp->dir_option[dir]) {
      ch->desc->str = &rp->dir_option[dir]->general_description;
    } else {
      CREATE(rp->dir_option[dir], struct room_direction_data, 1);
      ch->desc->str = &rp->dir_option[dir]->general_description;
    }
    break;
  case 6: 
    /* 
       extra descriptions 
       */
    if (!*string) {
      send_to_char("Usage: edit extra <keyword>\n\r", ch);
      return;
    }
    /* try to locate extra description */
    for (ed = rp->ex_description; ; ed = ed->next)
      if (!ed) {
	CREATE(ed , struct extra_descr_data, 1);
	ed->next = rp->ex_description;
	rp->ex_description = ed;
	CREATE(ed->keyword, char, strlen(string) + 1);
	strcpy(ed->keyword, string);
	ed->description = 0;
	ch->desc->str = &ed->description;
	send_to_char("New field.\n\r", ch);
	break;
      }  else if (!str_cmp(ed->keyword, string)) {
	/* the field exists */
	FREE(ed->description);
	ed->description = 0;
	ch->desc->str = &ed->description;
	send_to_char("Modifying description.\n\r", ch);
	break;
      }
    ch->desc->max_str = MAX_STRING_LENGTH;
    return; 
    break;
    
  case 7: strcpy(dirstr, ""); rspeed = -1; rdir = -1;
    sscanf(string, "%s %d", dirstr, &rspeed);
    if((rdir = search_block(dirstr, dirs, 0)) == -1)
    {
      send_to_char("Usage: edit river <dir> <speed>\n\r", ch);
      send_to_char("Where dir is one of these: n e s w u d\n\r", ch);
      return;
    }
    rp->river_speed = rspeed;
    rp->river_dir = rdir;
    return;

  case 8:
    tele_room = -1; tele_time = -1; tele_mask = -1;
    sscanf(string,"%d %d %d", &tele_time, &tele_room, &tele_mask);
    if (tele_room < 0 || tele_time < 0 || tele_mask < 0) {
      send_to_char("Usage: edit tele <time> <room_nr> <tele-flags>\n\r", ch);
      return;
      break;
    } else {
      if (IS_SET(TELE_COUNT, tele_mask)) {
	sscanf(string,"%d %d %d %d", 
	       &tele_time, &tele_room, &tele_mask, &tele_cnt);
	if (tele_cnt < 0) {
	  send_to_char("Usage: edit tele <time> <room_nr> <tele-flags> [tele-count]\n\r", ch);
	  return;
	} else {
	  real_roomp(ch->in_room)->tele_time = tele_time;
	  real_roomp(ch->in_room)->tele_targ = tele_room;
	  real_roomp(ch->in_room)->tele_mask = tele_mask;
	  real_roomp(ch->in_room)->tele_cnt  = tele_cnt;
	}
      } else {
	real_roomp(ch->in_room)->tele_time = tele_time;
	real_roomp(ch->in_room)->tele_targ = tele_room;
	real_roomp(ch->in_room)->tele_mask = tele_mask;
	real_roomp(ch->in_room)->tele_cnt  = 0;
	return;
      }
    }
    
    return;
  case 9:
    if (sscanf(string, "%d", &moblim) < 1) {
      send_to_char("Usage: edit tunnel <mob_limit>\n\r", ch);
      return;
      break;
    } else {
      real_roomp(ch->in_room)->moblim = moblim;
      if (!IS_SET(real_roomp(ch->in_room)->room_flags, TUNNEL))
	SET_BIT(real_roomp(ch->in_room)->room_flags, TUNNEL);
      return;
      break;
    }
  case 20: 
    /* deletion */
    if (!*string)  	{
      send_to_char("You must supply a field name.\n\r", ch);
      return;
    }
    /* try to locate field */
    for (ed = rp->ex_description; ; ed = ed->next)
      if (!ed) {
	send_to_char("No field with that keyword.\n\r", ch);
	return;
      } else if (!str_cmp(ed->keyword, string)) {
	FREE(ed->keyword);
	if (ed->description)
	  FREE(ed->description);
	  
	/* delete the entry in the desr list */						
	if (ed == rp->ex_description)
	  rp->ex_description = ed->next;
	else {
	  for(tmp = rp->ex_description; tmp->next != ed; 
	      tmp = tmp->next);
	  tmp->next = ed->next;
	}
	FREE(ed);
	  
	send_to_char("Field deleted.\n\r", ch);
	return;
      }
    break;				
    
  default:
    send_to_char("I'm so confused :-)\n\r",ch);
    return;
    break;
  }
  
  if (*ch->desc->str)	{
    FREE(*ch->desc->str);
  }
  
  if (*string) {		/* there was a string in the argument array */ 
    if (strlen(string) > (size_t) room_length[field - 1])	{
      send_to_char("String too long - truncated.\n\r", ch);
      *(string + length[field - 1]) = '\0';
    }
    CREATE(*ch->desc->str, char, strlen(string) + 1);
    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    send_to_char("Ok.\n\r", ch);
  } else {			/* there was no string. enter string mode */
    send_to_char("Enter string. terminate with '@'.\n\r", ch);
    *ch->desc->str = 0;
    ch->desc->max_str = room_length[field - 1];
  }
}


/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */


void do_setskill(struct char_data *ch, char *arg, int cmd)
{
	send_to_char("This routine is disabled untill it fitts\n\r", ch);
	send_to_char("The new structures (sorry Quinn) ....Bombman\n\r", ch);
	return;
}








/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
	int found, begin, look_at;

	found = begin = 0;

	do
	{
		for ( ;isspace(*(argument + begin)); begin++);

		if (*(argument+begin) == '\"') {  /* is it a quote */

			begin++;

			for (look_at=0; (*(argument+begin+look_at) >= ' ') && 
			    (*(argument+begin+look_at) != '\"') ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

			if (*(argument+begin+look_at) == '\"')
				begin++;

		} else {

			for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

		}

		*(first_arg + look_at) = '\0';
		begin += look_at;
	}
	while (fill_word(first_arg));

	return(argument+begin);
}

int help_compare(const void* a, const void* b)
{
    return strcmp(((struct help_index_element*) a)->keyword,
		  ((struct help_index_element*) b)->keyword);
}

// Not finished version of NEWHELP
int read_help_file(FILE* fileptr, struct help_index_element *list, int j)
{
    return 0;
}

/* void save_help_to_db(void) */
/* { */
/*   MYSQL mysql; */
/*   MYSQL_RES *res; */
/*   MYSQL_ROW row; */
  
/*   uint i = 0; */

/*   if (!(mysql_real_connect(&mysql,db_host,db_user,db_pass, db_name, db_port, NULL, 0))) */
/*     { */
/*       log_msg("Could not connect to database in save_help_to_db, exited with error: %s", mysql_error(&mysql)); */
/*       return 0; */
/*     } */

/*     if (mysql_query(&mysql,"SELECT * FROM connect_test")) */
/*         exiterr(3); */

/*     if (!(res = mysql_store_result(&mysql))) */
/*         exiterr(4); */

/*     while((row = mysql_fetch_row(res))) { */
/*         for (i=0 ; i < mysql_num_fields(res); i++)  */
/*             printf("%s\n",row[i]); */
/*     } */

/*     if (!mysql_eof(res)) */
/*         exiterr(5); */

/*     mysql_free_result(res); */
/*     mysql_close(&mysql); */

/* } */

// Build helpfile list on init
void build_help_index(void)
{
#ifdef NEWHELP
    struct help_index_element list[HELP_HIGH+1][256];
    FILE *genhelp_fl, *skillhelp_fl, *combhelp_fl, *immhelp_fl;
#else
    struct help_index_element *list = 0;
#endif    
  long pos;
  int max = 0;
  char buf[81], tmp[81], *scan;
  int nr = 0;
    
#ifdef NEWHELP
    if(!(genhelp_fl = fopen(GENERAL_HELP_FILE, "r")))
    {
      log_msg("  Could not open help file: general");
      return;
    }
    top_of_genhelp = read_help_file(genhelp_fl, list, HELP_GENERAL);

    if(!(skillhelp_fl = fopen(SKILL_HELP_FILE, "r")))
    {
      log_msg("  Could not open help file: skill");
      return;
    }
    top_of_skillhelp = read_help_file(skillhelp_fl, list, HELP_SKILL);

    if(!(immhelp_fl = fopen(IMM_HELP_FILE, "r")))
    {
      log_msg("  Could not open help file: immortal");
      return;
    }
    top_of_immhelp = read_help_file(immhelp_fl, list, HELP_IMMORTAL);

    if(!(combhelp_fl = fopen(COMBAT_HELP_FILE, "r")))
    {
      log_msg("  Could not open help file: combat");
      return;
    }
    top_of_combhelp = read_help_file(combhelp_fl, list, HELP_COMBAT);
#else

    if(!(help_fl = fopen(HELP_KWRD_FILE, "r")))
    {
	log_msg("  Could not open help file.");
	return;
    }
  for (;;)
    {
	pos = ftell(help_fl);
	fgets(buf, 81, help_fl);
	*(buf + strlen(buf) - 1) = '\0';
	scan = buf;
	for (;;)
	{
	    /* extract the keywords */
	    scan = one_word(scan, tmp);

	    if (!*tmp)
		break;

	    if(nr >= max)
	    {
		max += 16;
		if (!list) {
		    CREATE(list, struct help_index_element, max);
		} else  {
		    RECREATE(list, struct help_index_element, max);
		}
	    }
#ifdef NEWHELP
	    list[j][nr].pos = pos;
	    list[j][nr].keyword = strdup(tmp);
#else
	    list[nr].pos = pos;
	    list[nr].keyword = strdup(tmp);
#endif	    

	    nr++;
	}
	/* skip the text */
	do
	    fgets(buf, 81, help_fl);
	while (*buf != '#');
	
	if ((*(buf + 1) == '~') || feof(help_fl))
	    break;
    }
    /* we might as well sort the stuff */
    qsort(list, nr, sizeof(*list), help_compare);
#endif // NEWHELP
    help_index = list;
    top_of_helpt = nr-1;

    //    log_msg ("Saving help file to database");
    //    save_help_to_db();

}

void delete_help_index(void)
{
    int i;

#ifdef NEWHELP
    int j;

    for (j=HELP_LOW ; j <= HELP_HIGH; j++)
      for(i = 0 ; i < top_of_helpt ; ++i)
	if(help_index[j][i].keyword)
	    FREE(help_index[j][i].keyword);
#else
    for(i=0; i < top_of_helpt ; ++i)
      if(help_index[i].keyword)
	FREE(help_index[i].keyword);
#endif
    
    FREE(help_index);

    help_index = 0;
    top_of_helpt = 0;

    fclose(help_fl);
    help_fl = 0;
}

void page_string(struct descriptor_data *d, const char *str, int keep_internal)
{
	if (!d)
		return;

	if(d->character &&
	   IS_SET(d->character->specials.flags, PLR_CONTINUOUS))
	{
	    send_to_char_formatted(str, d->character);
	    return;
	}
	
	if (keep_internal)
	{
		CREATE(d->showstr_head, char, strlen(str) + 1);
		strcpy(d->showstr_head, str);
		d->showstr_point = d->showstr_head;
	}
	else
		d->showstr_point = (char*) str;

	show_string(d, ""); 
}



void show_string(struct descriptor_data *d, const char *input)
{
    char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
    char newText[MAX_STRING_LENGTH*8];
    register char *scan, *chk;
    int lines = 0, toggle = 1;

    strcpy(newText, "");
    one_argument(input, buf);

    if (*buf)
    {
	if (d->showstr_head)
	{
	    FREE(d->showstr_head);
	    d->showstr_head = 0;
	}
	d->showstr_point = 0;
	return;
    }

    /* show a chunk */
    for (scan = buffer;; scan++, d->showstr_point++)
	if((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
	   ((toggle = -toggle) < 0))
	    lines++;
	else if (!*scan || (lines >= 22))
	{
	    *scan = '\0';
	    strcat(buffer, "$CN");
	    format_string(buffer, newText, CheckColor(d->character));

	    SEND_TO_Q(newText, d);

	    /* see if this is the end (or near the end) of the string */
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    if (!*chk)
	    {
		if (d->showstr_head)
		{
		    FREE(d->showstr_head);
		    d->showstr_head = 0;
		}
		d->showstr_point = 0;
	    }
	    return;
	}
}
