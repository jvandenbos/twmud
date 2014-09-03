
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#if USE_unistd
#include <unistd.h>
#endif

#if USE_profile && SPARC
#include <a.out.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "array.h"
#include "spells.h"
#include "limits.h"
#include "hash.h"
#include "race.h"
#include "multiclass.h"
#include "constants.h"
#include "limits.h"
#include "utility.h"
#include "act.h"
#include "modify.h"
#include "fight.h"
#include "limits.h"
#include "spelltab.h"
#include "board.h"
#include "magicutils.h"
#include "spec.h"
#include "recept.h"
#include "util_str.h"
#include "find.h"
#include "sblock.h"
#include "periodic.h"
#include "statistic.h"
#include "spell_util.h"
#include "proto.h"          /* prototypes!! VIP!! */
#include "hero.h"
#include "cmdtab.h"
#include "strext.c"
#include "mobprog2.h"
#include "varfunc.h"
#include "trackchar.h"
#include "db.random.h"

#ifdef __cplusplus
extern "C"
{
#endif

    char* crypt(const char*, const char*);

#ifdef AUX
    FILE* popen(const char*, const char*);
#endif

#ifdef __cplusplus
};
#endif

void ReadSingleMobprog(char*,long,int,int,char);

extern int MAX_SPECIALAFFECTS;
struct fedit_data fedit;

/* for automatic shutdown */
event_t *shutdown_event = NULL;
int shutdown_time = 0;

void do_wizrep(struct char_data *ch, char *argument, int cmd)
{
  FILE *fl;

  char str[MAX_INPUT_LENGTH+20];

  if (!IS_PC(ch))
  {
    send_to_char("You are a monster! Bug off!\n\r", ch);
    return;
  }

      /* skip whites */
  for (; isspace(*argument); argument++);
  if (!*argument)
  {
    send_to_char("Pardon?\n\r",ch);
    return;
  }
  if (!(fl = fopen(WIZ_FILE, "a")))
  {
    perror ("do_wizrep");
    send_to_char("Could not open the wiz-report-file.\n\r", ch);
    return;
  }
  sprintf(str, "%s: %s\n", GET_REAL_NAME(ch), argument);
  fputs(str, fl);
  fclose(fl);
  send_to_char("Ok.\n\r", ch);
}

void do_deny( struct char_data *ch, char *argument, int cmd )
{
struct char_data *victim;
char arg[MAX_STRING_LENGTH];
int class1;
int class2;

only_argument( argument, arg );

if(!(victim=get_char_vis(ch,arg)))
{
  send_to_char("I don't see that person here...\n\r",ch);
  return;
}

class1 = GetMaxLevel(ch);
class2 = GetMaxLevel(victim);

if ( GET_LEVEL(ch,class1) < GET_LEVEL(victim,class2) )
{
send_to_char("I Don't think so .. idiot..\n\r",ch);
return;
}

if ( !IS_SET( victim->specials.flags, PLR_DENY ) )
{
SET_BIT( victim->specials.flags, PLR_DENY );
send_to_char("You have been Denied Access. Bye.\n\r",victim);
send_to_char("Character Denied.\n\r",ch);
close_socket(victim->desc, TRUE);
return;
}
else
{
REMOVE_BIT( victim->specials.flags, PLR_DENY );
send_to_char("Character UnDenied.\n\r",ch);
return;
}

return;
}

void do_zap(struct char_data *ch, char *arg, int cmd)
{
  struct obj_data *crat;
  struct char_data *victim;
  char name[300],rmcmd[100];
  struct descriptor_data *d;
  int trust;

  arg = one_argument(arg, name);
  if (!*name) {
    send_to_char("Who?\n\r", ch);
    return;
  }

  victim=get_char_vis_world(ch,name, NULL);
  if (!victim){
    send_to_char("Who?\n\r", ch);return;
  }
  if (TRUST(ch) < TRUST(victim)){
    send_to_char("ow fishy fish fish!\n\r",ch);
    sprintf(name,"%s tried to delete you.\n\r",GET_REAL_NAME(ch));
    send_to_char(name,victim);
    return;
  }
  if (!IS_PC(victim)){
     send_to_char("this command kills and deletes a PLAYER!\n\r",ch);
     return;
  }

  send_to_all_regardless_formatted(
        "  You see a tiny dot in the sky ...\n\r"
        "After a moment you realise its a large\n\r"
        " blazing rock falling out of the sky! \n\r");
  if((crat = make_object(28007, VIRTUAL))){
     obj_to_room(crat, victim->in_room);
  }else{
     log_msg("couldnt created crater object???");
  }
  sprintf(rmcmd,"You get the feeling you won't be seeing %s again.\n\r",
    GET_REAL_NAME(victim));
  if (victim->specials.fighting) stop_fighting(victim);
  death_cry(victim);
  d=getdescriptor(victim);

  strcpy(name,GET_REAL_NAME(victim));
  trust = TRUST(victim);

  if (d!=NULL)
    close_socket(d, TRUE);
  else
    extract_char(victim);

  send_to_all_regardless_formatted("You hear an huge explosion and a faint splat!\n\r");
  send_to_all_regardless_formatted(rmcmd);
  DelChar(name, trust);
  /* alex this is a complete hack I know, so you write a better one :P */
}


void do_fiddle(struct char_data *ch, char *argument, int cmd)
/* alex: am I cunning or what */
{
  char field[20], name[20], parm[50], buf[80];
  struct obj_data *obj;
  int iparm = 0, list = 0;
  int priv;

  if (IS_NPC(ch))
    return;

  priv = (TRUST(ch) == MAX_TRUST) ||
         IS_SET(ch->player.godinfo.position, HEAD_QUESTOR);

  if (!*argument)
  {
    send_to_char("Usage: fiddle obj_name field parameter\n\r",ch);
    send_to_char("Usage: fiddle obj_name list  field\n\r",ch);
    send_to_char("Fields are: id, room, wear, type, restrictions, \n\r",ch);
    send_to_char("            val1, val2, val3, val4, weight, value,\n\r",ch);
    send_to_char("            rent, af1type, af1am - af5type, af5am\n\r",ch);
    send_to_char("            and level.\n\r",ch);
    return;
  }

  argument = one_argument(argument, name);
  argument = one_argument(argument, field);
  if (is_abbrev(field,"list")){
    list=1;
    argument = one_argument(argument, field);
  }else{
    argument = one_argument(argument, parm);
    iparm = atoi(parm);
  }

  /* if (!(obj = get_obj_vis(ch, name)))     { */
  if (!(obj=get_obj_in_list_vis(ch, name, ch->carrying))){
     send_to_char("Can't find such a thing here..\n\r",ch);
     return;
  }


#define FIDDLE(thing)\
   if (list){ \
     sprintf(buf, "Value is %ld\n\r", (long)thing); \
   }else{ \
     thing = iparm;  \
     sprintf(buf, "Value is is now %ld\n\r", (long)thing);  \
     if (!priv) obj->obj_flags.cost_per_day = -1; \
   }send_to_char(buf,ch); \
   return;

  if (is_abbrev(field,"id")) {
     /* FIDDLE(obj->item_number); */
     send_to_char("sorry! - id field disabled\n\r",ch);
  } else
  if (is_abbrev(field,"room")){
     FIDDLE(obj->in_room);
  } else
  if (is_abbrev(field,"restrictions")){
     FIDDLE(obj->obj_flags.extra_flags);
  } else
  if (is_abbrev(field,"val1")){
     FIDDLE(obj->obj_flags.value[0]);
  } else
  if (is_abbrev(field,"val2")){
     FIDDLE(obj->obj_flags.value[1]);
  } else
  if (is_abbrev(field,"val3")){
     FIDDLE(obj->obj_flags.value[2]);
  } else
  if (is_abbrev(field,"val4")){
     FIDDLE(obj->obj_flags.value[3]);
  } else
  if (is_abbrev(field,"weight")){
     FIDDLE(obj->obj_flags.weight);
  } else
  if (is_abbrev(field,"value")){
     FIDDLE(obj->obj_flags.cost);
  } else
  if (is_abbrev(field,"rent")){
     FIDDLE(obj->obj_flags.cost_per_day);
  } else
  if (is_abbrev(field,"af1type")){
     FIDDLE(obj->affected[0].location);
  } else
  if (is_abbrev(field,"af1am")){
     FIDDLE(obj->affected[0].modifier);
  } else
  if (is_abbrev(field,"af2type")){
     FIDDLE(obj->affected[1].location);
  } else
  if (is_abbrev(field,"af2am")){
     FIDDLE(obj->affected[1].modifier);
  } else
  if (is_abbrev(field,"af3type")){
     FIDDLE(obj->affected[2].location);
  } else
  if (is_abbrev(field,"af3am")){
     FIDDLE(obj->affected[2].modifier);
  } else
  if (is_abbrev(field,"af4type")){
     FIDDLE(obj->affected[3].location);
  } else
  if (is_abbrev(field,"af4am")){
     FIDDLE(obj->affected[3].modifier);
  } else
  if (is_abbrev(field,"af5type")){
     FIDDLE(obj->affected[4].location);
  } else
  if (is_abbrev(field,"af5am")){
     FIDDLE(obj->affected[4].modifier)
  } else
  if (is_abbrev(field,"type")) {
     FIDDLE(GET_ITEM_TYPE(obj));
  } else
  if (is_abbrev(field,"wear")) {
     FIDDLE(obj->obj_flags.wear_flags);
  } else
  if (is_abbrev(field,"level")) {
     FIDDLE(obj->obj_flags.level);
  }else

  send_to_char("You what ? - what are you on!\n\r",ch);
  return;
}


void show_all_inventory(struct char_data *ch, struct char_data *wiz)
{
  int k, Num_Inventory=1, cond_top=0, cond_tot[50];
  char buf[MAX_INPUT_LENGTH];
  bool ifound=FALSE;
  struct obj_data *i, *cond_ptr[50];

  sprintf(buf, "------------------%s's inventory------------------\n\r", GET_REAL_NAME(ch));
  send_to_char(buf,wiz);

  for (i=ch->carrying; i; i = i->next_content)
  {
    if (cond_top<50)
    {
      ifound = FALSE;
      for (k=0;(k<cond_top && !ifound);k++)
      {
	if (cond_top > 0)
	{
	  if ((i->item_number == cond_ptr[k]->item_number) &&
	      (OBJ_SHORT(i) && OBJ_SHORT(cond_ptr[k]) &&
	       (!strcmp(OBJ_SHORT(i),OBJ_SHORT(cond_ptr[k])))))
	  {
	    cond_tot[k] += 1;
	    ifound = TRUE;
	  }
	}
      }
      if (!ifound)
      {
	cond_ptr[cond_top] = i;
	cond_tot[cond_top] = 1;
	cond_top += 1;
      }
    } else
    {
      show_obj_to_char(i,wiz,2);
      if (GET_ITEM_TYPE(i) == ITEM_CONTAINER)
      {
	sprintf(buf,"-----In %s-----\n\r",OBJ_SHORT(i));
	send_to_char(buf, wiz);
	list_obj_in_heap(i->contains, wiz);
	send_to_char("--------", wiz);
      }
    }
  }

  if (cond_top)
  {
    for (k=0;k<cond_top;k++)
    {
      sprintf(buf,"[%2d] ",Num_Inventory++);
      send_to_char(buf,wiz);
      if (cond_tot[k] > 1)
      {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char(cond_ptr[k],wiz,2,cond_tot[k]);
      }
      else
      {
	show_obj_to_char(cond_ptr[k],wiz,2);
	if (GET_ITEM_TYPE(cond_ptr[k]) == ITEM_CONTAINER)
	{
	  send_to_char("-----\n\r",wiz);
	  list_obj_in_heap(cond_ptr[k]->contains,wiz);
	  send_to_char("-----\n\r",wiz);
	}
      }
    }
  }
}

/* MHL -- 5/28/94 */
void do_player(struct char_data *wiz, char *argument, int cmd)
{
  char command[256], character[256], cont[256], buf[MAX_STRING_LENGTH];
  struct char_data *ch;
  struct obj_data *container;
  int count=1;

  if (IS_NPC(wiz))
    return;

  argument = one_argument(argument, command);
  argument = one_argument(argument, character);

  if (!strcmp(command,"list") || (!command))
  {
    send_to_char_formatted("Usage:  $CBplayer$CN [ $Cglist$CN | $Cgscore$CN | $Cgattr$CN ", wiz);
    if (TRUST(wiz) > TRUST_LRGOD)
    {
      send_to_char_formatted("| $Cginv$CN | $Cgeq$CN ", wiz);
      if(TRUST(wiz) > TRUST_GRGOD)
	send_to_char("| $Cgcont$CN | $Cgall$CN ", wiz);
    }
    send_to_char_formatted("] $Cb<character>$CN\n\r",wiz);
    return;
  }

  if (!*character)
  {
    send_to_char_formatted("$CRPlayer who?$CN\n\r",wiz);
    return;
  }
  else if (!(ch = get_char_vis_world(wiz, character, &count)))
  {
    send_to_char_formatted("$CRNo player or mobile by that name in the world.$CN\n\r", wiz);
    return;
  }
  else if(IS_PC(ch) && TRUST(ch) > TRUST(wiz))
  {
    sprintf(buf, "$CR%s would disapprove of you looking at his information.\n\r",
	    ch->player.sex == SEX_FEMALE ?
	    "She" : "He");
    send_to_char_formatted(buf,wiz);
    sprintf(buf, "$CR%s just tried to do a player %s on you.\n\r",GET_REAL_NAME(wiz), command);
    send_to_char_formatted(buf,ch);
    return;
  }

  if(is_abbrev(command,"score"))
  {
    sprintf(buf, "------------------%s's score------------------\n\r",GET_REAL_NAME(ch));
    send_to_char(buf,wiz);
    show_score(ch,wiz);
    return;
  }
  else if(is_abbrev(command,"attr"))
  {
    sprintf(buf, "------------------%s's attributes------------------\n\r",GET_REAL_NAME(ch));
    send_to_char(buf,wiz);
    show_attr(ch,wiz);
    return;
  }
  else if (is_abbrev(command,"aff"))
  {
    sprintf(buf, "------------------%s's affects------------------\n\r",GET_REAL_NAME(ch));
    send_to_char(buf,wiz);
    show_affect(ch,wiz);
    return;
  }
  else if (TRUST(wiz) >= TRUST_GOD)
  {
    if(is_abbrev(command,"eq"))
    {
      sprintf(buf, "------------------%s's equipment------------------\n\r", GET_REAL_NAME(ch));
      send_to_char(buf,wiz);
      show_equiped(wiz,ch);
      return;
    }
    else if (is_abbrev(command,"inv"))
    {
      sprintf(buf, "------------------%s's inventory------------------\n\r", GET_REAL_NAME(ch));
      send_to_char(buf,wiz);
      list_obj_in_heap(ch->carrying, wiz);
      return;
    }
    else if (TRUST(wiz) >= TRUST_LORD)
    {
      if (is_abbrev(command, "cont"))
      {
	one_argument(argument,cont);
	container = get_obj_in_list(cont ,ch->carrying);
	if (container)
	{
	  sprintf(buf, "------------------%s's container: %s------------------\n\r", GET_REAL_NAME(ch), OBJ_SHORT(container));
	  send_to_char(buf,wiz);
	  list_obj_in_heap(container->contains, wiz);
	  return;
	}
	else
	{
	  sprintf(buf, "There is no container named \"%s\" in %s's inventory.\n\r",cont,GET_REAL_NAME(ch));
	  send_to_char(buf,wiz);
	  return;
	}
      }
      else if (is_abbrev(command, "all"))
      {
	show_all_inventory(ch,wiz);
	return;
      }
    }
  }
  send_to_char("Invalid command type for player, try player list for a listing.\n\r", wiz);
}


void stop_all_followers(struct char_data *ch)
{
  register struct char_data *i;

  EACH_CHARACTER(m_iter, i) {
    if (i->master==ch)
      stop_follower(i);
  }
  END_AITER(m_iter);
}

void trans_affects(struct char_data *from, struct char_data *to)
{
  register struct char_data *i;
  struct affected_type *af, *next_af_dude, afcopy;

  shifter_normalize(from);

  for (af = from->affected ; af ; af = next_af_dude) {
    next_af_dude = af->next;

    afcopy = *af;

    affect_join(to, &afcopy, FALSE, FALSE);
    affect_remove(from, af);
  }

  EACH_CHARACTER(m_iter, i) {
    for (af = i->affected ; af ; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->caster==from)
         af->caster=to;
    }
  }
  END_AITER(m_iter);
}


int reimb_timer(object_event* event, long now)
{
    struct obj_data* j = event->object;

    if(--j->obj_flags.timer == 1)
    {
      if(j->carried_by)
	act("$p begins to fade.",
	    FALSE, j->carried_by, j, 0, TO_CHAR);
      if(j->equipped_by)
	act("$p begins to fade.",
	    FALSE, j->equipped_by, j, 0, TO_CHAR);
      else if ((j->in_room != NOWHERE) &&
	       (real_roomp(j->in_room)->people))
      {
	act("$p begins to fade.",
	    TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	act("$p begins to fade.",
	    TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
      }
    }
    if(--j->obj_flags.timer <= 0)
    {
        if(j->carried_by)
	    act("$p fades away in your hands.",
		FALSE, j->carried_by, j, 0, TO_CHAR);
        if(j->equipped_by)
	    act("$p fades away in your hands.",
		FALSE, j->equipped_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) &&
		 (real_roomp(j->in_room)->people))
	{
	    act("$p fades away.",
		TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	    act("$p fades away.",
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
			  (event_func) reimb_timer,
			  NULL);

	return 1;
    }
}


void do_reimburse(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH];
  struct char_data *victim;
  int level;

  argument = one_argument(argument, name);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("Reimburse who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("You can't reimburse an NPC.\n\r", ch);
    return;
  }

  if (IS_GOD(victim)) {
    send_to_char("Why would you want to reimburse that person?\n\r", ch);
    return;
  }

  level = GetMaxLevel(victim);

  give_reimb_items(victim, level);

  send_to_char("The gods take pity and reequip you.\n\r", victim);
  if(ch)
    send_to_char("You reequip them.\n\r", ch);

  do_save(victim, "", 69);
}

void give_reimb_items(struct char_data* ch, int level)
{
    object_event *event;
    char buf[MAX_STRING_LENGTH];
    struct obj_data* obj;
    struct
    {
	int item, level;
    }
    *reimb,
    reimbs[] =
    {
	{ 166,	0 },	/* shield */
	{ 167,	0 },	/* dagger */
	{ 168,	0 },	/* light */
	{ 169,	0 },	/* gloves */
	{ 170,	0 },	/* ring */
	{ 170,	6 },	/* ring */
	{ 171,	6 },	/* crown */
	{ 177,	6 },	/* boots */
	{ 165,	15 },	/* armor */
	{ 173,	15 },	/* sleeves */
	{ 174,	15 },	/* cloak */
	{ 174,	25 },	/* cloak */
	{ 176,	25 },	/* belt */
	{ 179,	25 },	/* tunic */
	{ 172,	40 },	/* leggings */
	{ 175, 	40 },	/* bracelet */
	{ 175,  40 },	/* bracelet */
	{ 178,	40 },	/* throw dagger */
	{ 180,	40 },	/* crystal */
	{ -1,	61 },	/* end of list marker */
    };

    for(reimb = reimbs ; reimb->level <= level ; reimb++)
      if((obj = make_object(reimb->item, VIRTUAL)))
      {
	obj->obj_flags.timer=240;
        CREATE(event, object_event, 1);
	event->object = obj;

	sprintf(buf, "reimb %s", OBJ_NAME(obj));
	obj->corpse_timer = event_queue_pulse((event_t*) event,
					    pulse,
					    (event_func) reimb_timer,
					    buf);
	obj_to_char(obj, ch);
      }
}


void do_dir(struct char_data *ch, char *argument, int cmd)
{
	FILE *fp;
	char buf[2400], tmp[1000];
	char command[] = "ls -C /home1/f/flmud/fl/lib/world";

	if (!(fp = popen(command, "r"))) {
                printf("Error with popen\n");
                exit(3);
        }
	strcpy(buf, "");

        do {
                fgets(tmp, 999, fp);
                if(!feof(fp)) {
                        strcat(buf, tmp);
                }
	/* send_to_char(tmp, ch); */
        } while (!feof(fp));

	page_string(ch->desc, buf, 1);

	pclose(fp);

	/* send_to_char("This command isn't implemented yet\n\r", ch); */
}

void do_auth(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH];
    char buf[256];
    int done=FALSE;
    struct descriptor_data *d;
    int flag = -1;

    if (IS_NPC(ch)) return;

    /* parse the argument */
    /* get char name */
    argument = one_argument(argument, name);

    while(*argument == ' ')
	argument++;

    if(!name[0])
    {
	send_to_char("authorize <name> <yes|no|message>\n\r", ch);
	send_to_char("authorize list\n\r", ch);
	if(TRUST(ch) >= TRUST_GOD)
	    send_to_char("authorize all <yes|no|ask>\n\r", ch);
	return;
    }
    else if((TRUST(ch) >= TRUST_GOD) && !strcmp(name, "all"))
    {
	one_argument(argument, name);

	if(is_abbrev(name, "yes"))
	{
	    act("$n has authorized all new chars.",
		FALSE, ch, 0, 0, TO_LOG);
	    AUTH_PLAYER = 0;
	    flag = NEWBIE_START;
	}
	else if(is_abbrev(name, "no"))
	{
	    act("$n has denied all new chars.",
		FALSE, ch, 0, 0, TO_LOG);
	    AUTH_PLAYER = -1;
	    flag = NEWBIE_AXE;
	}
	else if(is_abbrev(name, "ask"))
	{
	    act("$n has turned on new character authorization.",
		FALSE, ch, 0, 0, TO_LOG);
	    AUTH_PLAYER = 1;
	    return;
	}
	else
	{
	    send_to_char("usage: auth all <yes|no|ask>\n\r", ch);
	    return;
	}
	name[0] = 0;
    }
    else if(is_abbrev(name, "list"))
    {
	name[0] = 0;
	flag = -2;
    }
    else if(is_abbrev(argument, "yes"))
    {
	flag = NEWBIE_START;
    }
    else if(is_abbrev(argument, "no"))
    {
	flag = NEWBIE_AXE;
    }

    /*
      search through descriptor list for player name
      */
    done = 0;

    EACH_DESCRIPTOR(d_iter, d)
    {
	if(d->character && d->character->player.name &&
	   d->connected == CON_AUTH)
	{
	    if(!name[0] || !str_cmp(GET_IDENT(d->character), name))
	    {
		done++;

		if(flag == -2)
		{
		    sprintf(buf, "%-20s: %s\n",
			    GET_REAL_NAME(d->character),
			    d->host);
		    send_to_char(buf, ch);
		}
		else if(flag == -1)
		{
		    SEND_TO_Q(argument, d);
		    SEND_TO_Q("\n\r", d);
		    sprintf(buf, "do_auth: $n sent $N '%s'",
			    argument);
		    act(buf, TRUE, ch, 0, d->character, TO_LOG);
		}
		else if(flag == NEWBIE_START)
		{
		    d->character->act_ptr = NEWBIE_START;
		    SEND_TO_Q("You have been accepted.  Press enter\n\r", d);
		    act("do_auth: $n allowed $N",
			TRUE, ch, 0, d->character, TO_LOG);
		}
		else if(flag == NEWBIE_AXE)
		{
		    SEND_TO_Q("You have been denied.  Press enter\n\r", d);
		    d->character->act_ptr = NEWBIE_AXE;
		    act("do_auth: $n denied $N",
			TRUE, ch, 0, d->character, TO_LOG);
		}

		if(name[0])
		    break;
	    }
	}
    }
    END_ITER(d_iter);


    /*
      if not found, return error
      */
    if(!done)
    {
	send_to_char("That player was not found.\n\r", ch);
	return;
    }

    return;
}

void do_passwd(struct char_data *ch, char *argument, int cmdnum)
{
   char name[MAX_INPUT_LENGTH], npasswd[MAX_INPUT_LENGTH], pass[20];
   struct char_data* player;

   /*
    *  sets the specified user's password.
    */
   if(IS_NPC(ch))
       return;

   /*
    *  get user's name:
    */
   argument = one_argument(argument, name);
   argument = one_argument(argument, npasswd);
   if(!*name || !*npasswd)
   {
     send_to_char("usage: chpwd <name> <passwd>\n\r", ch);
     return;
   }

   /*
    * see if they're in the game
    */
   if((player = find_player_in_world(name)))
   {
       send_to_char("They're in the game.  Chomp them first...\n\r", ch);
       return;
   }

   /*
    *   Look up character
    */
   if(!(player = LoadChar(0, name, READ_ALL)))
   {
       send_to_char("No such player\n\r", ch);
       return;
   }

   /*
    *  encrypt new password.
    */
   if (!*npasswd || strlen(npasswd) > 10)
   {
       send_to_char("Illegal password\n\r", ch);
       return;
   }

   strncpy(pass, crypt(npasswd, name), 10);

   *(pass+10) = '\0';

   if(player->pwd)
       FREE(player->pwd);
   player->pwd = strdup(pass);

   /*
    *   save char to file
    */
   SaveChar(player, player->in_room, 0);

   /*
    * and delete them from memory
    */
   free_char(player);
}

/* Bamfin and bamfout - courtesy of DM from Epic */
void dsearch(char *string, char *tmp)
{
 char *c, buf[255], buf2[255], buf3[255];
 int i, j;

 i = 0;
 while(i == 0) {
    if(strchr(string, '~')==NULL) {
       i = 1;
       strcpy(tmp, string);
    } else {
       c = strchr(string, '~');
       j = c-string;
       switch(string[j+1]) {
       case 'N': strcpy(buf2, "$n"); break;
       case 'H': strcpy(buf2, "$s"); break;
       default:  strcpy(buf2, ""); break;
       }
       strcpy(buf, string);
       buf[j] = '\0';
       strcpy(buf3, (string+j+2));
       sprintf(tmp, "%s%s%s" ,buf, buf2, buf3);
       sprintf(string, tmp);

     }
  }
}

void do_poofin(struct char_data *ch, char *arg, int cmd)
{
    char buf[255];
    int len;

    if(!*arg) {
	send_to_char("Poofin <definition>\n\r", ch);
	send_to_char(" Use ~N for your name and ~H for his or her depending on", ch);
	send_to_char(" your sex.\n\r Type 'poofin def' to use the default",ch);
	send_to_char(" string, and type\n\r 'poofin show' to see what it is",ch);
	send_to_char(" currently set to.\n\r", ch);
	return;
    } else {
	arg++;
    }

    if(!strcmp(arg, "def")) {
	REMOVE_BIT(ch->specials.pmask, BIT_POOF_IN);
	FREE(ch->specials.poofin);
	ch->specials.poofin = 0;
	send_to_char("Poofin set back to normal.\n\r", ch);
	if(IS_SET(ch->specials.flags, PLR_MASK)) {
	    send_to_char("You are no longer in mask mode.\n\r", ch);
	    REMOVE_BIT(ch->specials.flags, PLR_MASK);
	}
	return;
    }
    if (!strcmp(arg, "show")) {
	if (IS_SET(ch->specials.pmask, BIT_POOF_IN)) {
	    sprintf(buf, "%s\n\r", ch->specials.poofin);
	    send_to_char(buf, ch);
	    return;
	} else {
	    send_to_char("Your poofin is the default.\n\r", ch);
	    return;
	}
    }

    len = strlen(arg);

    if(len > 150) {
	send_to_char("String too long.  Truncated to:\n\r", ch);
	arg[150] = '\0';
	sprintf(buf, "%s\n\r", arg);
	send_to_char(buf, ch);
    }

    dsearch(arg, buf);

    if(ch->specials.poofin)
	FREE(ch->specials.poofin);
    ch->specials.poofin = strdup(buf);

    SET_BIT(ch->specials.pmask, BIT_POOF_IN);
    send_to_char("Ok.\n\r", ch);
    return;
}

void do_poofout(struct char_data *ch, char *arg, int cmd)
{
    char buf[255];
    int len;

    if(!*arg) {
	send_to_char("Poofout <definition>\n\r", ch);
	send_to_char(" Use ~N for your name and ~H for his or her depending on", ch);
	send_to_char(" your sex.\n\r Type 'poofout def' to use the default",ch);
	send_to_char(" string, and type\n\r 'poofout show' to see what it is",ch);
	send_to_char(" currently set to.\n\r", ch);
	return;
    } else {
	arg++;
    }

    if(!strcmp(arg, "def")) {
	REMOVE_BIT(ch->specials.pmask, BIT_POOF_OUT);
	FREE(ch->specials.poofout);
	ch->specials.poofout = 0;
	send_to_char("Poofout returned to normal.\n\r", ch);
	return;
    }
    if (!strcmp(arg, "show")) {
	if (IS_SET(ch->specials.pmask, BIT_POOF_OUT)) {
	    sprintf(buf, "%s\n\r", ch->specials.poofout);
	    send_to_char(buf, ch);
	    return;
	} else {
	    send_to_char("Your poofout is the default.\n\r", ch);
	    return;
	}
    }

    len = strlen(arg);

    if(len > 150) {
	send_to_char("String too long.  Truncated to:\n\r", ch);
	arg[150] = '\0';
	sprintf(buf, "%s\n\r", arg);
	send_to_char(buf, ch);
	len = 150;
    }

    dsearch(arg, buf);

    if(ch->specials.poofout)
	FREE(ch->specials.poofout);

    ch->specials.poofout = strdup(buf);

    SET_BIT(ch->specials.pmask, BIT_POOF_OUT);
    send_to_char("Ok.\n\r", ch);
    return;
}


void write_zone_file(FILE *fp, int start_room, int end_room)
{
  int i, j, arg1, arg2, arg3;
  char cmd, buf[80];
  struct char_data *p;
  struct obj_data *o;
  struct room_data *room;

  for (i = start_room; i<=end_room; i++) {
    room = real_roomp(i);
    if (room) {
      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
	if (IS_NPC(p)) {
	  cmd = 'M';
	  arg1 = MobVnum(p);
	  arg2 = mob_index[p->nr].number;
	  arg3 = i;
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, GET_NAME(p));
	  for (j = 0; j<MAX_WEAR; j++) {
	    if (p->equipment[j]) {
	      if (p->equipment[j]->item_number >= 0) {
		cmd = 'E';
		arg1 = ObjVnum(p->equipment[j]);
		arg2 = obj_index[p->equipment[j]->item_number].number;
		arg3 = j;
		strcpy(buf, OBJ_SHORT(p->equipment[j]));
		Zwrite(fp, cmd,1,arg1, arg2, arg3,
		       buf);
		RecZwriteObj(fp, p->equipment[j]);
	      }
	    }
	  }
	  for (o = p->carrying; o; o=o->next_content) {
	    if (o->item_number >= 0) {
	      cmd = 'G';
	      arg1 = ObjVnum(o);
	      arg2 = obj_index[o->item_number].number;
	      arg3 = 0;
	      Zwrite(fp, cmd, 1, arg1, arg2, arg3, OBJ_SHORT(o));
	      RecZwriteObj(fp, o);
	    }
	  }
	}
      }
      /*
       *  write out objects in rooms
       */
      for (o = room->contents; o; o= o->next_content) {
	if (o->item_number >= 0) {
	  cmd = 'O';
	  arg1 = ObjVnum(o);
	  arg2 = obj_index[o->item_number].number;
	  arg3 = i;
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, OBJ_SHORT(o));
	  RecZwriteObj(fp, o);
	}
      }
      /*
       *  lastly.. doors
       */

      for (j = 0; j < 6; j++) {
	/*
	 *  if there is an door type exit, write it.
	 */
	if (room->dir_option[j]) {  /* is a door */
	  if (room->dir_option[j]->exit_info) {
	    cmd = 'D';
	    arg1 = i ;
	    arg2 = j;
	    arg3 = 0;
	    if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
	      arg3 = 1;
	    }
	    if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
	      arg3 = 2;
	    }
	    Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
	  }
	}
      }
    }
  }
  fclose(fp);
}

void do_instazone(struct char_data *ch, char *argument, int cmdnum)
{
  char c,d;
  int start_room, end_room;
  char zonename[256];
  FILE *fp;

  if (IS_NPC(ch))
    return;
  strcpy(zonename, "");

  /*
   *   read in parameters (room #s)
   */
  start_room = -1; end_room = -1;
  sscanf(argument, "%d%c%d%c%s", &start_room, &c, &end_room,&d, zonename);

  if ((start_room == -1) || (end_room == -1)) {
    send_to_char("Instazone <start_room> <end_room>\n\r", ch);
    return;
  }

  fp = (FILE *)MakeZoneFile(ch, zonename);

  if (!fp) {
    send_to_char("Couldn't make file.. try again later\n\r", ch);
    return;
  }

  write_zone_file(fp, start_room, end_room);
}


void do_highfive(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    struct char_data *tch;

    if(!argument)
    {
	send_to_char("Who do you wish to high five?\n\r", ch);
	return;
    }

    only_argument(argument, buf);

    if ((tch = get_char_room_vis(ch,buf)) == 0)
    {
	send_to_char("I don't see anyone here like that.\n\r", ch);
	return;
    }

    if(ch == tch)
    {
	if(IS_GOD(ch) && !IS_SET(ch->specials.flags, PLR_NOWIZ))
	    act("Time stops as the world laughs at $n for highfiving $mself.",
		FALSE, ch, 0, tch, TO_WORLD_NOTVICT);
	else
	    act("$n high fives $mself.", TRUE, ch, 0, tch, TO_ROOM);
	act("You high five yourself.", FALSE, ch, 0, tch, TO_CHAR);
    }
    else if (IS_GOD(tch) && IS_GOD(ch) &&
	     !IS_SET(tch->specials.flags, PLR_NOWIZ) &&
	     !IS_SET(ch->specials.flags, PLR_NOWIZ))
    {
	act("$CBTime stops for a moment as $Cg$n$CB and $Cg$N$CB high five.",
	    FALSE, ch, 0, tch, TO_WORLD_NOTVICT);
	act("Time stops for a moment as you high five $N.",
	    FALSE, ch, 0, tch, TO_CHAR);
	act("Time stops for a moment as $n high fives you.",
	    FALSE, ch, 0, tch, TO_VICT);
    }
    else
    {
	act("$n gives you a high five.", TRUE, ch, 0, tch, TO_VICT);
	act("You give a hearty high five to $N.", TRUE, ch, 0, tch, TO_CHAR);
	act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
    }
}

void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
  if (WizLock) {
    send_to_char("WizLock is now off\n\r",ch);
    log_msg("Wizlock is now off.");
    WizLock = FALSE;
  } else {
    send_to_char("WizLock is now on\n\r",ch);
    log_msg("WizLock is now on.");
    WizLock = TRUE;
  }
}


void do_rload(struct char_data *ch, char *argument, int cmd)
{
  char i, fname[MAX_INPUT_LENGTH];
  int start= -1, end = -2;

  if (IS_NPC(ch)) return;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i)) {
    send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
    return;
  }

  if (sscanf(argument,"%d %d %s", &start, &end, fname) < 3)
    strcpy(fname, GET_IDENT(ch));

  if ((start <= end) && (start != -1) && (end != -2)) {
    RoomLoad(ch,start,end,fname);
  }
}


void do_rsave(struct char_data *ch, char *argument, int cmd)
{
    char i, buf[256], fname[MAX_INPUT_LENGTH];
    int start= -1, end = -2;

    if (IS_NPC(ch)) return;

    for (i = 0; *(argument + i) == ' '; i++);
    if (!*(argument + i)) {
	send_to_char("Save? Fine!  Save we must, But what?\n\r", ch);
	return;
    }
    if (sscanf(argument,"%d %d %s", &start, &end, fname) < 3)
	strcpy(fname, GET_IDENT(ch));

    if(index(fname, '/'))
    {
	send_to_char("Sorry, the file name can not have a '/' in it.\n\r", ch);
	return;
    }

    if ((start <= end) && (start != -1) && (end != -2)) {
	sprintf(buf,"mv world/%s world/%s.bak", fname, fname);
	system(buf);
	RoomSave(ch,start,end, fname);
    }
}

void do_rewiz(struct char_data *ch, char *argument, int cmd)
{
    boot_players(GOD_DIR, READ_PLAYER);
    send_to_char("Re-Reading Wizlist and Listwiz.\n\r", ch);
}

void do_emote(struct char_data *ch, char *argument, int cmd)
{
    char buf[2*MAX_INPUT_LENGTH+10];

    /* if (IS_NPC(ch) && (cmd != 0))
       return; */

    if (apply_soundproof(ch)) {
	return;
    }

    while(isspace(*argument))
	argument++;

    if(!*argument)
	send_to_char("Yes.. But what?\n\r", ch);
    else
    {
	sprintf(buf,"$n %s", escape(argument));
	act(buf,FALSE,ch,0,0,TO_ROOM);
	if(IS_SET(ch->specials.flags, PLR_ECHO))
	    act(buf,FALSE,ch,0,0,TO_CHAR);
	else
	    send_to_char("Ok.\n\r", ch);
    }
}



void do_echo(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];

    while(isspace(*argument))
	argument++;

    if(!*argument) {
	if (IS_SET(ch->specials.flags, PLR_ECHO)) {
	    send_to_char("echo off\n\r", ch);
	    REMOVE_BIT(ch->specials.flags, PLR_ECHO);
	} else {
	    SET_BIT(ch->specials.flags, PLR_ECHO);
	    send_to_char("echo on\n\r", ch);
	}
    } else	{
	if (IS_IMMORTAL(ch)) {
	    sprintf(buf,"%s\n\r", argument);
	    if(!IS_SET(ch->specials.flags, PLR_ECHO))
		send_to_room_except_formatted(buf, ch->in_room, ch);
	    else
		send_to_room_formatted(buf,ch->in_room);
	    send_to_char("Ok.\n\r", ch);
	}
    }
}

void do_fedit(struct char_data *ch, char *arg, int cmd) {
    char comm1[80];
    char comm2[80];
    char buf[80];

    sscanf(arg, "%s %s", comm1, comm2);
    if(is_abbrev(comm1, "imotd"))
        strcpy(fedit.filename, "imotd");
    else if(is_abbrev(comm1, "motd"))
	strcpy(fedit.filename, "motd");
    else if(is_abbrev(comm1, "news"))
	strcpy(fedit.filename, "news");
    else if(is_abbrev(comm1, "report")) {
        if(fedit.ch) {
            send_to_char("Someone is currently writing a message.\n\r", ch);
            return;
        }
        if(!fedit.content) {
	    send_to_char("There is currently nothing written to save.\n\r", ch);
	    return;
	}
        send_to_char("\n\rFilename: ", ch);
	send_to_char(fedit.filename, ch);
	send_to_char("\n\r", ch);
	send_to_char(fedit.content, ch);
	send_to_char("\n\r", ch);
	return;
    } else if(is_abbrev(comm1, "clear")) {
	if(fedit.ch) {
	    send_to_char("Someone is currently writing a message.\n\r", ch);
	    return;
	}
	strcpy(fedit.filename, "");
	FREE(fedit.content);
	return;
    } else {
        send_to_char("fedit news:   edit news.\n\r", ch);
	send_to_char("fedit motd:   edit motd.\n\r", ch);
        send_to_char("fedit imotd:  edit imotd.\n\r", ch);
        send_to_char("fedit report: show what's ready to be saved.\n\r", ch);
	return;
    }

    if(fedit.ch) {
        send_to_char("Someone is currently writing a message.\n\r", ch);
        return;
    }

    ch->desc->str = &fedit.content;
    FREE(*ch->desc->str);
    *ch->desc->str = 0;
    ch->desc->max_str = MAX_MESSAGE_LENGTH;
    fedit.ch = ch;

    sprintf(buf, "Writing %s. End Message with @, and use fsave to commit your change.\n\r", fedit.filename);
    send_to_char(buf, ch);
//quilan
}

void do_fsave(struct char_data *ch, char *arg, int cmd) {
    FILE *of;

    if(fedit.ch) {
        send_to_char("Someone is currently writing a message.\n\r", ch);
        return;
    }

    if(!*fedit.filename) {
	send_to_char("There is no file to be saved. You need to write some content, using fedit.\n\r", ch);
        return;
    }

    of = fopen(fedit.filename, "w+");
    fwrite(fedit.content, sizeof(char), strlen(fedit.content), of);
    fclose(of);

    if(is_abbrev(fedit.filename, "motd")) {
	file_to_string(MOTD_FILE, motd, sizeof(motd));
	send_to_char("MOTD saved.\n\r", ch);
    } else if(is_abbrev(fedit.filename, "imotd")) {
	file_to_string(IMOTD_FILE, imotd, sizeof(imotd));
	send_to_char("IMOTD saved.\n\r", ch);
    } else if(is_abbrev(fedit.filename, "news")) {
        file_to_string(NEWS_FILE, news, sizeof(news));
        send_to_char("NEWS saved.\n\r", ch);
    } else {
	send_to_char("Hrmm...never heard of this before: ", ch);
	send_to_char(fedit.filename, ch);
	send_to_char("\n\r", ch);
    }

    do_fedit(ch, "clear", 0);
}

/*
 * This function goes through all of the rooms, and sees for increments of
 * count rooms where there's no data. Usefull for builders looking for free
 * space. */

void do_qfunction(struct char_data *ch, char *arg, int cmd) {
   char buf[MAX_STRING_LENGTH];
   room_num i, count, curcount, start=0;
   room_data *rm;

   if(TRUST(ch) < 1) {
      send_to_char("Silly Mortal. Quilan's Debug functions are for gods.\n\r", ch);
      return;
   }

   while(*arg == ' ') arg++;

   if(!*arg) {
      send_to_char("Format is qfunc <room|mob|item> <num>\n\r", ch);
      return;
   }

   arg = one_argument(arg, buf);
   count = atol(arg);

   if(count <= 0) {
      send_to_char("You must specify atleast a 1 room range\n\r", ch);
      return;
   }

   curcount=0;
   send_to_char("\n\r", ch);
   if(is_abbrev(buf, "rooms")) {
      send_to_char("Rooms:\n\r", ch);
      sprintf(buf, "Start   End   (Range) Zone #\n\r");
      send_to_char(buf, ch);
      for(i=0;i<top_of_world;i++) {
         rm = real_roomp(i);
         if(rm) {
	    if(curcount>=count) {
	       sprintf(buf, "%5li - %5li (%5li)  %4i\n\r",
	   	       start, start+curcount-1, curcount, rm->zone);
	       send_to_char(buf, ch);
   	    }
	    curcount=start=0;
         } else {
	    curcount++;
	    if(!start) start=i;
         }
      }
      return;
   } else if(is_abbrev(buf, "items")) {
      send_to_char("Items:\n\r", ch);
      send_to_char("Start   End   (Range)\n\r", ch);
      for(i=1;i<=top_of_objt;i++) {
	 curcount = obj_index[i].virt-obj_index[i-1].virt-1;
	 if(curcount >= count) {
	    sprintf(buf, "%5i - %5i (%5i)\n\r",
		    obj_index[i-1].virt+1,obj_index[i].virt-1,
		    obj_index[i].virt-obj_index[i-1].virt-1);
	    send_to_char(buf, ch);
	 }
      }
      return;
   } else if(is_abbrev(buf, "mobs")) {
      send_to_char("Mobs:\n\r", ch);
      send_to_char("Start   End   (Range)\n\r", ch);
      for(i=1;i<=top_of_mobt;i++) {
	 curcount = mob_index[i].virt-mob_index[i-1].virt-1;
	 if(curcount >= count) {
	    sprintf(buf, "%5i - %5i (%5i)\n\r",
		    mob_index[i-1].virt+1,mob_index[i].virt-1,
		    mob_index[i].virt-mob_index[i-1].virt-1);
	    send_to_char(buf, ch);
	 }
      }
   }
   return;
}

void do_setlog(struct char_data *ch, char *arg, int cmd) {
    char onoff[4], logshort[50];
    char buf[MAX_INPUT_LENGTH];
    int tochange=0, numlogs=0, i;
    struct log_type {
	char* compare;
	int logflag;
	char* show;
	char* logtype;
    };
    const struct log_type logs[] = {
	{ "mprog",    LOG_MPROG,   "MPROG ",    "MOBprog" },
	{ "player",   LOG_PLAYER,  "PLAYER ",   "Player" },
	{ "immort",   LOG_IMM,     "IMMORTAL ", "Immortal" },
	{ "error",    LOG_ERROR,   "ERROR ",    "Error" },
	{ "quilan",   LOG_QUILAN,  "QUILAN ",   "Quilan's Project" },
	{ "connect",  LOG_CONNECT, "CONNECT ",  "Connection" },
	{ "",0,"","" } };

    while(arg[0]==' ') arg++;

    for(;logs[numlogs++ +1].logflag;);

    for(i=0;i<numlogs;i++)
	if(is_abbrev(arg, logs[i].compare)) {
	    strcpy(logshort, logs[i].logtype);
	    tochange = logs[i].logflag;
	}
    if(!strcmp(arg, "all")) {
	ch->log_flags = LOG_ALL;
	if(IS_SET(ch->channels, (1<<5))) {
	    send_to_char("Turning log channel on.\n\r", ch);
	    ch->channels ^= (1<<5);
	}
	send_to_char("All logs turned on.\n\r", ch);
	return;
    }
    if(!strcmp(arg, "none")) {
	ch->log_flags = 0;
	send_to_char("All logs turned off.\n\r", ch);
	return;
    }
    if(!strcmp(arg, "show")) {
	strcpy(buf, "");
	if(IS_SET(ch->channels, (1<<5)))
	    send_to_char("Your log channel is off.\n\r", ch);

	switch(ch->log_flags) {
	case 0:
	    send_to_char("All logs are off.\n\r", ch);
	    break;
	case LOG_ALL:
	    send_to_char("All logs are on.\n\r", ch);
	default:
	    for(i=0;i<numlogs;i++)
		if(ch->log_flags & logs[i].logflag)
		    strcat(buf, logs[i].show);
	    strcat(buf, "\n\r");
	    send_to_char("The following flags are set:\n\r", ch);
	    send_to_char(buf, ch);
	}
	return;
    }
    if(tochange > 0) {
	ch->log_flags ^= tochange;
	if(ch->log_flags & tochange)
	    strcpy(onoff, "on");
	else
	    strcpy(onoff, "off");
	sprintf(buf, "%s logs turned %s\n\r", logshort, onoff);
	send_to_char(buf, ch);
    }
    if(tochange==0) {
	send_to_char("To use the setlog function, use one of the following syntax:\n\r", ch);
	for(i=0;i<numlogs;i++) {
	    sprintf(buf, "setlog %s:\ttoggles %s logs.\n\r", logs[i].compare, logs[i].logtype);
	    send_to_char(buf, ch);
	}
	send_to_char("setlog all:\tmakes all logs active.\n\r", ch);
	send_to_char("setlog none:\tmakes all logs inactive.\n\r", ch);
	send_to_char("setlog show:\tshows all active logs.\n\r", ch);
	return;
    }
}

void do_slownames(struct char_data *ch, char *argument, int cmd)
{
  if (!slownames)
    {
      slownames=1;
      send_to_char("Names will not be looked up!\n\r", ch);
    }
  else
    {
      slownames=0;
      send_to_char("Names will be looked up!\n\r", ch);
    }
}

void do_system(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

  while(isspace(*argument))
    argument++;

  if(!*argument)
    send_to_char("That must be a mistake...\n\r", ch);
  else	{
    sprintf(buf,"\n\r$CR%s$CN\n\r", argument);
    send_to_all_regardless_formatted(buf);
  }
}

void do_trivia(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;

  while(isspace(*argument))
    argument++;

  if(!*argument)
    send_to_char("That must be a mistake...\n\r", ch);
  else  {
    sprintf(buf,"\n\r$Cg[Trivia]: $CN%s\n\r", argument);
    send_to_all_regardless_formatted(buf);
  }
}

void do_generate(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *miscobj;
//  skip_spaces(&argument);
  char buf2[MAX_INPUT_LENGTH];

  if (!*argument) {
    send_to_char("Generate what?\r\n", ch);
    return;
  }

  miscobj = create_object();

  miscobj->item_number = NOTHING;
  miscobj->in_room = NOWHERE;

  sprintf(buf2, "createobj %s", argument);
  ss_free(miscobj->name);
  miscobj->name = ss_make(buf2);

  sprintf(buf2, "%s has been left here.", argument);
  miscobj->description = ss_make(buf2);

  sprintf(buf2, "%s", argument);
  miscobj->short_description = ss_make(buf2);

  GET_OBJ_TYPE(miscobj) = ITEM_OTHER;
  GET_OBJ_WEAR(miscobj) = ITEM_TAKE + ITEM_HOLD;
  GET_OBJ_RENT(miscobj) = -1;
  (miscobj)->obj_flags.weight = 1;

  obj_to_char(miscobj, ch);

  act("$n skillfully creates something!\r\n", FALSE, ch, 0, 0, TO_ROOM);
  send_to_char("You skillfully create something! \r\n", ch);
}


void do_trans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_INPUT_LENGTH];
    room_num target;

    if (IS_NPC(ch))
	return;

    only_argument(argument,buf);
    if (!*buf)
	send_to_char("Who do you wish to transfer?\n\r",ch);
    else if (str_cmp("all", buf))
    {
	if (!(victim = get_char_vis_world(ch,buf, NULL)) ||
	    (!IS_NPC(victim) && !CAN_SEE(ch,victim)))
	    send_to_char("No-one by that name around.\n\r",ch);
	else if (TRUST(ch) < TRUST(victim)) {
		send_to_char_formatted("$CGI think $CRNOT$CN!\n\r",ch);
	} else
	{
	    act("$Cg$n$CB disappears with a loud bang!",
		FALSE, victim, 0, 0, TO_ROOM);
	    target = ch->in_room;
	    char_from_room(victim);
	    char_to_room(victim,target);
	    act("$Cg$n$CB arrives with a blinding flash of green light.",
		FALSE, victim, 0, 0, TO_ROOM);
	    act("$Cg$n$CB has transferred you!",FALSE,ch,0,victim,TO_VICT);
	    do_look(victim,"",15);
	    send_to_char("Ok.\n\r",ch);
	}
    }
    else			/* Trans All */
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (i->character != ch && !i->connected) {
		victim = i->character;
		if (TRUST(ch) <= TRUST(victim))
		break;
	    else
		act("$Cg$n$CB disappears with a loud bang!", FALSE, victim, 0, 0, TO_ROOM);
		target = ch->in_room;
		char_from_room(victim);
		char_to_room(victim,target);
		act("$Cg$n$CB arrives with a blinding flash of green light.", FALSE, victim, 0, 0, TO_ROOM);
		act("$Cg$n$CB has transferred you!",FALSE,ch,0,victim,TO_VICT);
		do_look(victim,"",15);
	    }
	}
	END_ITER(d_iter);

	send_to_char("Ok.\n\r",ch);
    }
}

void do_teleport(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  sh_int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis_world(ch,buf, NULL)) ||
    (!IS_NPC(victim) && !CAN_SEE(ch,victim)))
    send_to_char("No-one by that name around.\n\r",ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (TRUST(victim) >= TRUST(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    if (IS_SET(real_roomp(target)->room_flags, IMMORT_RM) &&
	(!IS_GOD(victim)))
      {
        send_to_char("Trying to send a mortal to an Immortal room?  I think not!\n\r", ch);
        return;
      }
    else if (IS_SET(real_roomp(target)->room_flags, GOD_RM) &&
	(TRUST(ch) < TRUST_LORD))
      {
        send_to_char("Who do you think you are, a God?\n\r", ch);
        return;
      }
    send_to_char("Ok.\n\r",ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    do_look(victim,"",15);
  }
}



void do_at(struct char_data *ch, char *argument, int cmd)
{
    char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
    int loc_nr, location, original_loc;
    struct char_data *target_mob;
    struct obj_data *target_obj;

    if (IS_NPC(ch))
	return;

    half_chop(argument, loc_str, command);
    if (!*loc_str)
    {
	send_to_char_formatted("$CRYou must supply a room number or a name.\n\r", ch);
	return;
    }

    if (isdigit(*loc_str) && !strchr(loc_str, '.'))
    {
	loc_nr = atoi(loc_str);
	if (NULL==real_roomp(loc_nr)) {
	    send_to_char_formatted("$CRNo room exists with that number.\n\r", ch);
	    return;
	}
	location = loc_nr;
    } else if ((target_mob = get_char_vis(ch, loc_str))) {
	if (IS_NPC(target_mob))
	    location = target_mob->in_room;
	else if (!CAN_SEE(ch,target_mob)) {
	    send_to_char_formatted("$CRNo such creature or object around.\n\r", ch);
	    return;
	}
	else location = target_mob->in_room;
    } else if ((target_obj=get_obj_vis_world(ch, loc_str, NULL)))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else
	{
	    send_to_char_formatted("$CRhe object is not available.\n\r", ch);
	    return;
	}
    else
    {
	send_to_char_formatted("$CRNo such creature or object around.\n\r", ch);
	return;
    }

    /* check for peeking */
    if (IS_SET(real_roomp(location)->room_flags, GOD_RM) &&
	(TRUST(ch) < TRUST_LORD))
    {
	send_to_char_formatted("$CRIf they wanted you nosing around it "
			       "wouldn't be GOD_ROOM, now would it?\n\r",
			       ch);
	return;
    }

    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);

    command_interpreter(ch, command, 1);

    /* check if the guy's still there */
    for (target_mob = real_roomp(location)->people; target_mob; target_mob =
	 target_mob->next_in_room)
	if (ch == target_mob) {
	    char_from_room(ch);
	    char_to_room(ch, original_loc);
	    break;
	}
}


void do_goto(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int loc_nr, location;
    struct char_data *target_mob, *v = NULL;
    struct obj_data *target_obj;

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);
    if (!*buf)	{
	send_to_char_formatted("$CRYou must supply a room number or a name or type goto home.\n\r",
			       ch);
	return;
    }

    if (isdigit(*buf) && !strchr(buf, '.'))
    {
	loc_nr = atoi(buf);
	if (NULL==real_roomp(loc_nr))
	{
	    if (TRUST(ch) < TRUST_CREATOR || loc_nr < 0)
	    {
		send_to_char_formatted("$CRNo room exists with that number.\n\r", ch);
		return;
	    }
	    else
	    {
#if HASH
#else
		if (loc_nr < WORLD_SIZE) {
#endif
		    send_to_char_formatted("$CRYou form order out of chaos.\n\r", ch);
		    CreateOneRoom(loc_nr);

#if HASH
#else
		} else {
		    send_to_char_formatted("$CRSorry, that room # is too large.\n\r", ch);
		    return;
		}
#endif
	    }
	}
	/* a location has been found. */
	location = loc_nr;
    }
    else if (is_abbrev(buf,"home"))
    {
	location = GET_HOME(ch);
	if(!location)
	    location = GET_HOME(ch) = 20;
	if(!real_roomp(location))
	{
	    send_to_char_formatted("$CRBetter create your home first bozo...\n\r", ch);
	    return;
	}
    }
    else if ((target_mob = get_char_vis_world(ch, buf, NULL)))
    {
	location = target_mob->in_room;
    }
    else if ((target_obj=get_obj_vis_world(ch, buf, NULL)))
    {
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else
	{
	    send_to_char("The object is not available.\n\r", ch);
	    send_to_char("Try where #.object to nail its room number.\n\r", ch);
	    return;
	}
    }
    else
    {
	send_to_char("No such creature or object around.\n\r", ch);
	return;
    }

    if (!real_roomp(location))
    {
	log_msg("Massive error in do_goto. Everyone Off NOW.");
	return;
    }

    if (IS_SET(real_roomp(location)->room_flags, IMMORT_RM) &&
	(!IS_GOD(ch)))
    {
	send_to_char("Who do you think you are, an Immortal?\n\r", ch);
	return;
    }
    if (IS_SET(real_roomp(location)->room_flags, BRUJAH_RM) && !IS_SET(ch->specials.flags, PLR_BRUJAH))
    {
        send_to_char("It takes real magic to go there.\n\r", ch);
	return;
    }
    if (IS_SET(real_roomp(location)->room_flags, GOD_RM) &&
	(TRUST(ch) < TRUST_LORD)) {
	send_to_char("Who do you think you are, a God?\n\r", ch);
	return;
    }

    if (IS_SET(ch->specials.flags, PLR_STEALTH) && ch->invis_level) {
	/* PAC --  && (ch->invis_level < 55) I really hated this ) {*/
	for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
	    if ((ch != v) && (TRUST(v) >= ch->invis_level)) {
		if (IS_SET(ch->specials.pmask, BIT_POOF_OUT) &&
		    ch->specials.poofout)
		    act(ch->specials.poofout, FALSE, ch, 0, v, TO_VICT);
		else
		    act("$n steps into the shadows.",
			FALSE, ch, 0, v, TO_VICT);
	    }
	}
    } else /*if (ch->invis_level < 55)*/ {
	if (IS_SET(ch->specials.pmask, BIT_POOF_OUT) &&
	    ch->specials.poofout)
	    act(ch->specials.poofout, FALSE, ch, 0, 0, TO_ROOM);
	else
	    act("$Cg$n$Cb steps into the shadows.",
		FALSE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
	stop_fighting(ch);
    char_from_room(ch);
    char_to_room(ch, location);

    if (IS_SET(ch->specials.flags, PLR_STEALTH) && ch->invis_level) {
	/* PAC && (ch->invis_level < 55) I really hated this ) { */
	for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room) {
	    if ((ch != v) && (TRUST(v) >= ch->invis_level)) {
		if (IS_SET(ch->specials.pmask, BIT_POOF_IN) &&
		    ch->specials.poofin)
		    act(ch->specials.poofin, FALSE, ch, 0, v, TO_VICT);
		else
		    act("$Cg$n$Cb slowly fades into existence.",
			FALSE, ch, 0,v,TO_VICT);
	    }
	}
    } else /*if (ch->invis_level < 55)*/ {
	if (IS_SET(ch->specials.pmask, BIT_POOF_IN) ||
	    ch->specials.poofin)
	    act(ch->specials.poofin, FALSE, ch, 0, v, TO_ROOM);
	else
	    act("$Cg$n$Cb slowly fades into existence.",
		FALSE, ch, 0, 0,TO_ROOM);
    }
    do_look(ch, "",15);
}



void do_stat(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type *aff;
  struct obj_affected_type *at;
  char arg1[MAX_STRING_LENGTH];
  char buf[4*MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  struct room_data *rm=0;
  struct char_data *k=0;
  struct obj_data  *j=0;
  struct obj_data  *j2=0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  Variable *vtmp;
  int i, virt;
  int i2, count;
  bool found;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg1);

  count = 1;

  /* no argument */
  if (!*arg1)
  {
    send_to_char("Stats on who or what?\n\r",ch);
  }
  else if (!str_cmp("room", arg1))
  {
    /* stats on room */
    rm = real_roomp(ch->in_room);
    sprintf(buf, "Room name: %s, Of zone : %d. V-Number : %ld, R-number : %ld\n\r",
	    rm->name, rm->zone, rm->number, ch->in_room);

    sprinttype(rm->sector_type,sector_types,buf2);
    sprintf(buf+strlen(buf), "Sector type : %s ", buf2);

    strcpy(buf,"Special procedure : ");
    strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");

    sprintf(buf,"Room flags: ");
    sprintbit((unsigned long) rm->room_flags,room_bits,buf+strlen(buf));
    strcat(buf,"\n\r");


    if (rm->tele_time > 0) {
      sprintf(buf+strlen(buf), "Ttime: %d  Ttype: %d", rm->tele_time, rm->tele_mask);
      strcat(buf,"\n\r");
    }

    sprintf(buf+strlen(buf),"Description:\n\r%s", rm->description);

    sprintf(buf+strlen(buf), "Extra description keywords(s): ");
    if(rm->ex_description) {
      strcat(buf, "\n\r");
      for (desc = rm->ex_description; desc; desc = desc->next) {
	strcat(buf, desc->keyword);
	strcat(buf, "\n\r");
      }
      strcat(buf, "\n\r");
    } else {
      strcat(buf, "None\n\r");
    }

    sprintf(buf+strlen(buf),"------- Chars present -------\n\r");
    for (k = rm->people; k; k = k->next_in_room)
    {
      if(CAN_SEE(ch, k))
      {
	strcat(buf, GET_NAME(k));
	if(IS_NPC(k))
	{
	  if(IS_SET(k->specials.mob_act, ACT_POLYSELF))
	  {
	    sprintf(buf2, " (%s) (PC)\n\r",
		    GET_NAME(real_character(k)));
	    strcat(buf, buf2);
	  }
	  else
	    strcat(buf, "(MOB)\n\r");
	}
	else
	  strcat(buf, "(PC)\n\r");
      }
    }
    strcat(buf, "\n\r");

    strcat(buf, "--------- Contents ---------\n\r");
    for (j = rm->contents; j; j = j->next_content)
    {
      strcat(buf, OBJ_NAME(j));
      strcat(buf, "\n\r");
    }
    strcat(buf, "\n\r");

    strcat(buf,"------- Exits defined -------\n\r");
    for (i = 0; i <= 5; i++) {
      if (rm->dir_option[i]) {
	sprintf(buf+strlen(buf),"Direction %s . Keyword : %s\n\r",
		dirs[i], rm->dir_option[i]->keyword);
	strcat(buf, "Description:\n\r  ");
	if(rm->dir_option[i]->general_description)
	  strcat(buf, rm->dir_option[i]->general_description);
	else
	  strcat(buf,"UNDEFINED\n\r");
	sprintbit((unsigned long) rm->dir_option[i]->exit_info,exit_bits,buf2);
	sprintf(buf+strlen(buf), "Exit flag: %s \n\rKey no: %d\n\rTo room (R-Number): %ld\n\r",
		buf2, rm->dir_option[i]->key,
		rm->dir_option[i]->to_room);
      }
    }
    page_string(ch->desc, buf, 1);
  }
  else if ((k = get_char_vis_world(ch, arg1, &count)))
  {
    /* mobile in world */
    if (IS_PC(k) && (!CAN_SEE(ch,k))) {
      send_to_char("No mobile or object by that name in the world\n\r", ch);
      return;
    }

    switch(k->player.sex) {
    case SEX_NEUTRAL :
      strcpy(buf,"NEUTRAL-SEX");
      break;
    case SEX_MALE :
      strcpy(buf,"MALE");
      break;
    case SEX_FEMALE :
      strcpy(buf,"FEMALE");
      break;
    default :
      strcpy(buf,"ILLEGAL-SEX!!");
      break;
    }

    sprintf(buf2, " %s - Name : %s [R-Number%d], In room [%ld]",
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	    GET_NAME(k), k->nr, k->in_room);
    strcat(buf, buf2);

    if(IS_SET(k->delete_flag, DELETE))
      strcat(buf, " DELETE");
    if(IS_SET(k->delete_flag, PROTECT))
      strcat(buf, " PROTECTED");
    if(IS_SET(k->delete_flag, HOUSED))
      strcat(buf, " HOUSED");

    strcat(buf, "\n\r");

    if (IS_MOB(k)) {
      sprintf(buf+strlen(buf), "V-Number [%d]\n\r", mob_index[k->nr].virt);
    }

    sprintf(buf+strlen(buf), "File version: %d", k->file_version);

    if (TRUST(ch) == 10)
      sprintf(buf+strlen(buf), "   Trust [%d]\n\r", TRUST(k));

    strcat(buf,"Short description: ");
    strcat(buf, k->player.short_descr ?
	   ss_data(k->player.short_descr) : "None");
    strcat(buf,"\n\r");

    strcat(buf,"Title: ");
    strcat(buf, k->player.title ? ss_data(k->player.title) : "None");
    strcat(buf,"\n\r");

    strcat(buf, "Long description: ");
    strcat(buf, k->player.long_descr ? ss_data(k->player.long_descr) : "None");
    strcat(buf, "\n\r");

    sprintf(buf+strlen(buf),"Class: %d ", k->player.clss);
    sprintbit(k->player.clss,pc_class_types, buf2);
    strcat(buf, buf2);

    sprintf(buf2,"\n\rGuild Information -  Guild: [%d] -  Flags: [%d].\n\r",
	    k->player.guildinfo.guildnumber, k->player.guildinfo.guildflags);
    strcat(buf,buf2);

    sprintf(buf2,"Level [M:%d / C:%d / W:%d / T:%d / K:%d / D:%d / S:%d / R:%d / H:%d / O:%d / B:%d]\n\r",
	    k->player.level[0], k->player.level[1],
	    k->player.level[2], k->player.level[3],
	    k->player.level[4], k->player.level[5],
	    k->player.level[6], k->player.level[7],
	    k->player.level[8], k->player.level[9],
	    k->player.level[10]);

    strcat(buf, buf2);

    sprintf(buf2,"Align[%d] Guild[%d] GuildLevel[%d]\n\r",
	    GET_ALIGNMENT(k), k->in_guild, k->guild_level);

    strcat(buf, buf2);

    switch(k->player.hpsystem) {
     case 0:
       sprintf(buf3,"0 - UN_DEFINED");
       break;
     case 1:
       sprintf(buf3,"1 - NEW_SYSTEM");
       break;
     default:
       sprintf(buf3,"%i - OLD_SYSTEM", k->player.hpsystem);
       break;
    }

    sprintf(buf2, "Hp System: %s\tMpstate: %i\n\r", buf3, k->player.mpstate);
    strcat(buf, buf2);

    sprintf(buf2,"MaxLevel : %d, Min Level :%d\n\r",
	    k->player.maxlevel,k->player.minlevel);

    strcat(buf,buf2);

    sprintf(buf+strlen(buf),"Birth : [%ld]secs, Logon[%ld]secs, Played[%d]secs\n\r",
	    k->player.time.birth,
	    k->player.time.logon,
	    k->player.time.played);

    sprintf(buf+strlen(buf),"Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\n\r",
	    age(k).year, age(k).month, age(k).day, age(k).hours);

    sprintf(buf+strlen(buf),"Height [%d]cm  Weight [%d]pounds \n\r", GET_HEIGHT(k), GET_WEIGHT(k));

    sprintf(buf+strlen(buf),"Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]  Cha:[%d]\n\r",
	    GET_STR(k), GET_ADD(k),
	    GET_INT(k),
	    GET_WIS(k),
	    GET_DEX(k),
	    GET_CON(k),
	    GET_CHA(k));

    sprintf(buf+strlen(buf),"Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
	    GET_MANA(k),mana_limit(k),mana_gain(k),
	    GET_HIT(k),hit_limit(k),hit_gain(k),
	    GET_MOVE(k),move_limit(k),move_gain(k) );

    sprintf(buf+strlen(buf),"AC:[%d/10]  Coins: [%d]  Exp: [%Ld]  Hitroll: [%d]  Damroll: [%d]\n\r",
	    GET_AC(k),
	    GET_GOLD(k),
	    GET_EXP(k),
	    k->points.hitroll,
	    k->points.damroll );

    sprinttype(GET_POS(k),position_types,buf2);
    sprintf(buf+strlen(buf),"Position: %s, Fighting: %s",buf2,
	    ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
    if (k->desc) {
      sprinttype(k->desc->connected,connected_types,buf2);
      strcat(buf,", Connected: ");
      strcat(buf,buf2);
    }
    strcat(buf,"\n\r");

    strcat(buf,"Default position: ");
    sprinttype((k->specials.default_pos),position_types,buf2);
    strcat(buf, buf2);
    strcat(buf,"\n\rNPC flags: ");
    sprintbit(k->specials.mob_act,action_bits,buf2);
    strcat(buf, buf2);
    strcat(buf,"\n\rPC flags: ");
    sprintbit(k->specials.flags,player_bits,buf2);
    strcat(buf, buf2);

    sprintf(buf2,"\n\rTimer [%d] \n\r", k->specials.timer);
    strcat(buf, buf2);

    sprintf(buf+strlen(buf),"Wimpy Value: %d \n\r", GET_WIMPY(k));

    if (IS_MOB(k)) {
      strcat(buf, "\n\rMobile Special procedure : ");
      strcat(buf, (mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
    }

    sprintf(buf+strlen(buf), "Bare Hand Damage %dd%d.\n\r",
	    k->specials.damnodice, k->specials.damsizedice);

    sprintf(buf+strlen(buf),"Carried weight: %d   Carried items: %d\n\r",
	    IS_CARRYING_W(k),
	    IS_CARRYING_N(k) );

    for(i=0,j=k->carrying;j;j=j->next_content,i++);
    sprintf(buf+strlen(buf),"Items in inventory: %d, ",i);

    for(i=0,i2=0;i<MAX_WEAR;i++)
      if (k->equipment[i]) i2++;
    sprintf(buf2,"Items in equipment: %d\n\r", i2);
    strcat(buf,buf2);

    if (!IS_NPC(k)) {
      sprintf(buf+strlen(buf), "Money in bank: %d.         ", GET_BANK(k));
      sprintf(buf+strlen(buf), "Practices left: %d.\n\r",k->specials.spells_to_learn);
    }

    sprintf(buf+strlen(buf),"Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
	    k->specials.apply_saving_throw[0],
	    k->specials.apply_saving_throw[1],
	    k->specials.apply_saving_throw[2],
	    k->specials.apply_saving_throw[3],
	    k->specials.apply_saving_throw[4]);

    sprintf(buf+strlen(buf), "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
	    k->specials.conditions[THIRST],
	    k->specials.conditions[FULL],
	    k->specials.conditions[DRUNK]);

    sprintf(buf+strlen(buf), "Master is '%s'\n\r",
	    ((k->master) ? GET_NAME(k->master) : "NOBODY"));
    strcat(buf,"Followers are:\n\r");
    for(fol=k->followers; fol; fol = fol->next) {
      /* This is a security leak, but o well -- PAC */
      strcat(buf,GET_NAME(fol->follower));
      strcat(buf,"\n\r");
    }

    if(k->player.vars) {
       strcat(buf, "Variables are:\n\r");
       for(vtmp=k->player.vars;vtmp;vtmp=vtmp->next) {
	  switch(vtmp->type) {
	   case -1:
	     sprintf(buf+strlen(buf), "  %-10s: -NULL-",vtmp->name);
	     break;
	   case 0:
	     sprintf(buf+strlen(buf),"  %-10s: %ld",vtmp->name,vtmp->Value());
	     break;
	   case 1:
	     sprintf(buf+strlen(buf),"  %-10s: %s",vtmp->name,vtmp->CValue());
	     break;
	   case 2:
	     sprintf(buf+strlen(buf),"  %-10s: (char)%lx",vtmp->name,vtmp->Value());
	     break;
	   case 3:
	     sprintf(buf+strlen(buf),"  %-10s: (obj)%lx",vtmp->name,vtmp->Value());
	     break;
	   case 4:
	     sprintf(buf+strlen(buf),"  %-10s: (room)%lx",vtmp->name,vtmp->Value());
	     break;
	  }
	  strcat(buf, "\n\r");
       }
    }

    /* immunities */
    strcat(buf,"Immune to:");
    sprintbit(k->M_immune, immunity_names, buf+strlen(buf));
    strcat(buf,"\n\r");
    /* resistances */
    strcat(buf,"Resistant to:");
    sprintbit(k->immune, immunity_names, buf+strlen(buf));
    strcat(buf,"\n\r");
    /* Susceptible */
    strcat(buf,"Susceptible to:");
    sprintbit(k->susc, immunity_names, buf+strlen(buf));
    strcat(buf,"\n\r");
    /*  race, action pointer */
    strcat(buf,"Race: ");
    sprinttype((k->race),RaceName,buf+strlen(buf));
    sprintf(buf+strlen(buf), "  Action pointer: %d\n\r", (int)k->act_ptr);

    /* Showing the bitvector */
    strcat(buf,"Affected by:\n\r ");
    sprintbit(AFF_FLAGS(k),affected_bits,buf+strlen(buf));
    strcat(buf, "\n\r And:  ");
    sprintbit(AFF2_FLAGS(k),affected2_bits, buf+strlen(buf));
    strcat(buf, "\n\r");

    /* Routine to show what spells a char is affected by */
    if (k->affected) {
      strcat(buf,"\n\rAffecting Spells/Skills:\n\r------------------------\n\r");
      for(aff = k->affected; aff; aff = aff->next) {
	if (IS_SET(spell_by_number(aff->type)->targets, TAR_SKILL))
	  sprintf(buf+strlen(buf), "Skill : '%s'\n\r", spell_name(aff->type));
	else
	  sprintf(buf+strlen(buf), "Spell : '%s'\n\r", spell_name(aff->type));
	switch(aff->location)
	{
	case APPLY_NONE:
	  break;

	case APPLY_IMMUNE:
	case APPLY_SUSC:
	case APPLY_M_IMMUNE:
	  sprintbit(aff->modifier, immunity_names, buf2);
	  sprintf(buf+strlen(buf),"     %s to %s\n\r",
		  apply_types[(int)aff->location], buf2);
	  break;

	default:
	  sprintf(buf+strlen(buf),"     Modifies %s by %ld points\n\r",
		  apply_types[(int)aff->location], aff->modifier);
	  break;
	}
	if(aff->bitvector)
	{
	  strcat(buf,"     Bits Set: ");
	  sprintbit(aff->bitvector,affected_bits,buf+strlen(buf));
	  strcat(buf,"\n\r");
	}
	if(aff->mana_cost)
	  sprintf(buf+strlen(buf),"     Mana: %d  Caster: %s\n\r",
		  aff->mana_cost,
		  aff->caster ? GET_NAME(aff->caster)
		  : "disconnected...");
	else
	  sprintf(buf+strlen(buf),"     Expires in %d hours\n\r",
		  aff->duration);
	sprintf(buf+strlen(buf), "     Save Bonus: %d\n\r",
		aff->save_bonus);
      }
    }
    page_string(ch->desc, buf, 1);
  }
  else if ((j=get_obj_vis_world(ch, arg1, &count)))
  {
    virt = (j->item_number >= 0) ? obj_index[j->item_number].virt : 0;
    sprintf(buf,
	    "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
	    OBJ_NAME(j), j->item_number, virt);
    sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
    strcat(buf,buf2); strcat(buf,"\n\r");
    sprintf(buf+strlen(buf), "Short description: %s\n\rLong description:\n\r%s\n\r",
	    (OBJ_SHORT(j) ? OBJ_SHORT(j) : "None"),
	    (OBJ_DESC(j) ? OBJ_DESC(j) : "None") );
    if(j->ex_description){
      strcat(buf, "Extra description keyword(s):\n\r----------\n\r");
      for (desc = j->ex_description; desc; desc = desc->next) {
	strcat(buf, desc->keyword);
	strcat(buf, "\n\r");
      }
      strcat(buf, "----------\n\r");
    } else {
      strcat(buf,"Extra description keyword(s): None\n\r");
    }

    strcat(buf,"Can be worn on :");
    sprintbit(j->obj_flags.wear_flags,wear_bits,buf+strlen(buf));
    strcat(buf,"\n\r");

    strcat(buf,"Extra flags: ");
    if (IS_PURE_ITEM(j))
      sprintbit(j->obj_flags.extra_flags,extra_bits_pure,buf+strlen(buf));
    else
      sprintbit(j->obj_flags.extra_flags,extra_bits,buf+strlen(buf));
    strcat(buf,"\n\r");

    sprintf(buf+strlen(buf),
	    "Weight: %d, Value: %d, Cost/day: %d, Timer: %d, CWeight: %d\n\r",
	    j->obj_flags.weight,j->obj_flags.cost,
	    j->obj_flags.cost_per_day,  j->obj_flags.timer,
	    j->obj_flags.cont_weight);
    sprintf(buf+strlen(buf),
	    "Level: %ld, Durability: %ld, Encumbrance: %ld\n\r",
	    j->obj_flags.level, j->obj_flags.durability,
	    j->obj_flags.encumbrance);

    strcat(buf,"In room: ");
    if (j->in_room == NOWHERE)
      strcat(buf,"Nowhere");
    else {
      sprintf(buf2,"%ld",j->in_room);
      strcat(buf,buf2);
    }
    strcat(buf," ,In object: ");
    strcat(buf, (!j->in_obj ? "None" : fname(OBJ_NAME(j->in_obj))));

    switch (j->obj_flags.type_flag) {
    case ITEM_LIGHT :
      sprintf(buf+strlen(buf), "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2]);
      break;
    case ITEM_SCROLL :
    case ITEM_POTION :
      sprintf(buf+strlen(buf), "Level: %d  Spells : %s, %s, %s",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1] > 1
	      ? spell_name(j->obj_flags.value[1]) : "none",
	      j->obj_flags.value[2] > 1
	      ? spell_name(j->obj_flags.value[2]) : "none",
	      j->obj_flags.value[3] > 1
	      ? spell_name(j->obj_flags.value[3]) : "none");
      break;
    case ITEM_WAND :
    case ITEM_STAFF :
      sprintf(buf+strlen(buf), "Level: %d\n\rSpell : %s\n\rCharges: %d/%d",
	      j->obj_flags.value[0],
	      spell_name(j->obj_flags.value[3]),
	      j->obj_flags.value[2],
	      j->obj_flags.value[1]);
      break;
    case ITEM_WEAPON :
      sprintf(buf+strlen(buf), "Tohit : %d\n\rDamage: %dD%d\n\rType : %d",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    case ITEM_FIREWEAPON :
      sprintf(buf, "Strength to draw: %d\n\rTohit: %d\n\rMax range/Todam: %d\n\rType: %d",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    case ITEM_MISSILE :
      sprintf(buf+strlen(buf), "Percentage chance of breaking: %d\n\rTodam: %dD%d\n\rType: %d",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    case ITEM_ARMOR :
      sprintf(buf+strlen(buf), "AC-apply : [%d]\n\rFull Strength : [%d]",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1]);

      break;
    case ITEM_TRAP :
      sprintf(buf+strlen(buf), "effect flags: %d, att type: %d, damage level: %d, charges: %d",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    case ITEM_CONTAINER :
      sprintf(buf+strlen(buf), "Max-contains : %d\n\rLocktype : %d\n\rKey: %d\n\rCorpse : %s",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]?"Yes":"No");
      break;
    case ITEM_DRINKCON :
      sprinttype(j->obj_flags.value[2],drinks,buf2);
      sprintf(buf+strlen(buf),
	      "Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid: %s",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[3],
	      buf2);
      break;
    case ITEM_NOTE :
      sprintf(buf+strlen(buf), "Tongue : %d",
	      j->obj_flags.value[0]);
      break;
    case ITEM_KEY :
      sprintf(buf+strlen(buf), "Keytype : %d",
	      j->obj_flags.value[0]);
      break;
    case ITEM_FOOD :
      sprintf(buf+strlen(buf), "Makes full : %d\n\rPoisoned : %d",
	      j->obj_flags.value[0],
	      j->obj_flags.value[3]);
      break;
    case ITEM_SPELLBOOK :
      sprintf(buf+strlen(buf), "\n\rSpell 1: %d\n\rSpell 2: %d\n\rSpell 3: %d\n\rSpell 4: %d\n\r",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    default:
      sprintf(buf+strlen(buf),"Values 0-3 : [%d] [%d] [%d] [%d]",
	      j->obj_flags.value[0],
	      j->obj_flags.value[1],
	      j->obj_flags.value[2],
	      j->obj_flags.value[3]);
      break;
    }

    strcat(buf,"\n\rEquipment Status: ");
    if (!j->carried_by)
      strcat(buf,"NONE");
    else {
      found = FALSE;
      for (i=0;i < MAX_WEAR;i++) {
	if (j->carried_by->equipment[i] == j) {
	  sprinttype(i,equipment_types,buf2);
	  strcat(buf,buf2);
	  found = TRUE;
	}
      }
      if (!found)
	strcat(buf,"Inventory");
    }

    strcat(buf, "\n\rSpecial procedure : ");
    if (j->item_number >= 0)
      strcat(buf, (obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
    else
      strcat(buf, "No\n\r");

    strcat(buf, "Contains :\n\r");
    found = FALSE;
    for(j2=j->contains;j2;j2 = j2->next_content) {
      strcat(buf,fname(OBJ_NAME(j2)));
      strcat(buf,"\n\r");
      found = TRUE;
    }
    if (!found)
      strcat(buf,"Contains : Nothing\n\r");

    found = FALSE;
    for(at = j->affected, i=0 ; i<MAX_OBJ_AFFECT ; at++, i++) {
      if((at->location != APPLY_NONE) && (at->modifier != 0)) {
	if (!found) {
	  strcat(buf,"Can affect char:\n\r");
	  found = TRUE;
	}

	sprinttype(at->location,apply_types,buf2);
	switch(at->location)
	{
	case APPLY_SPELL:
	  {
	    sprintbit(at->modifier, affected_bits, buf3);
	    sprintf(buf+strlen(buf), "    Affects : %s By %s\n\r", buf2, buf3);
	  }
	  break;

	case APPLY_SPELL2:
	  {
	    sprintbit(at->modifier, affected2_bits, buf3);
	    sprintf(buf+strlen(buf), "    Affects : %s By %s\n\r", buf2, buf3);
	  }
	  break;

	case APPLY_WEAPON_SPELL:
	case APPLY_EAT_SPELL:
	case APPLY_BOOK_SPELL:
	  {
	    sprintf(buf+strlen(buf), "    Affects : %s By %s\n\r", buf2,
		    spell_name(at->modifier));
	  }
	  break;

	case APPLY_IMMUNE:
	case APPLY_M_IMMUNE:
	case APPLY_SUSC:
	  {
	    sprintbit(at->modifier, immunity_names, buf3);
	    sprintf(buf+strlen(buf), "    Affects : %s By %s\n\r", buf2, buf3);
	  }
	  break;

	default:
	  sprintf(buf+strlen(buf), "    Affects : %s By %ld\n\r", buf2,at->modifier);
	}
      }
    }
    page_string(ch->desc, buf, 1);
  }
  else
  {
    send_to_char("No mobile or object by that name in the world\n\r", ch);
  }
}


void do_viewfile(struct char_data *ch, char *argument, int cmd)
{
    char namefile[MAX_INPUT_LENGTH];
    char bigbuf[32000];

    only_argument(argument, namefile);
    if(!strcmp(namefile,"bug"))
      file_to_string(BUG_FILE,bigbuf, sizeof(bigbuf));
    else if(!strcmp(namefile,"idea"))
      file_to_string(IDEA_FILE,bigbuf, sizeof(bigbuf));
    else if(!strcmp(namefile,"typo"))
      file_to_string(TYPO_FILE,bigbuf, sizeof(bigbuf));
    else if(!strcmp(namefile,"wizreport"))
    {
      if (TRUST(ch) < TRUST_LORD)
      {
	send_to_char("You cannot look at that file!\n\r",ch);
	return;
      }
      file_to_string(WIZ_FILE, bigbuf, sizeof(bigbuf));
    }
#if USE_profile
    else if(!strcmp(namefile, "prof"))
      file_to_string("prof.out",bigbuf, sizeof(bigbuf));
#endif
    else if(!strcmp(namefile,"motd")) {
      page_string(ch->desc,motd,0);
      return;
    }
    else if(!strcmp(namefile,"imotd")) {
      page_string(ch->desc,imotd,0);
      return;
    }
    else if(!strcmp(namefile, "twstory")) {
      page_string(ch->desc,twstory,0);
      return;
    }
    else {
      send_to_char("Usage:  viewfile [bug | typo | idea | motd | imotd | twstory", ch);
      if (TRUST(ch) >= TRUST_LORD)
	send_to_char(" | wizreport ].\n\r",ch);
      else
	send_to_char(" ].\n\r",ch);
      return;
    }

    page_string(ch->desc,bigbuf,1);
}

void do_set(struct char_data *ch, char *argument, int cmd)
{
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], parmstr[MAX_INPUT_LENGTH], buf[80], buf2[MAX_INPUT_LENGTH], str[MAX_INPUT_LENGTH];
  struct char_data *mob;
  int parm, parm2;

  if (IS_NPC(ch)) {
    send_to_char("Siwwy Wabbit. @set is for gods.\n\r", ch);
    return;
  }

  if (!*argument) {
    send_to_char("Usage: @set <field> <char> <value>\n\r", ch);
    return;
  }

  argument = one_argument(argument, field);
  argument = one_argument(argument, name);
  argument = one_argument(argument, parmstr);

  if (is_abbrev(field,"help")) {
    send_to_char("Use one of these fields:\n\r",ch);
    send_to_char("fountlev   fountgold    chaos       disaster     pirate\n\r",ch);
    send_to_char("casino     alignment    class       experience   level\n\r",ch);
    send_to_char("sex        race         hitpts      mhitpts      board\n\r",ch);
    send_to_char("tohit      todam        armorclass  bank         gold\n\r",ch);
    send_to_char("practice   age          str         stradd       int\n\r",ch);
    send_to_char("wis        dex          con         chr          peaceful\n\r",ch);
    send_to_char("mana       thirst       hunger      drunk        home\n\r",ch);
    send_to_char("delete     move         maxmove     nowiz        bhhit\n\r", ch);
    send_to_char("bhsize     bhdice       housed      trust        pkill\n\r",ch);
    send_to_char("seeinvis   guild        glevel      gnumber      gflags\n\r",ch);
    send_to_char("deny       showdam      protect     summon       notrack\n\r",ch);
    send_to_char("timer      group        areafx\n\r",ch);
    return;
  }

  if (is_abbrev(field,"chaos")) {
    sscanf(name,"%d",&parm);
    CHAOS=parm;
    if(CHAOS==1) {
      send_to_char("Chaos Night variables set.  This will turn on auto-restore for deaths,\n\r", ch);
      send_to_char("and in the future, it will undoubtedly do more.\n\r", ch);
      log_msg("Chaos variables turned on.");
    }
    if(CHAOS==0) {
      send_to_char("Chaos Night variables inactivated.\n\r", ch);
      log_msg("Chaos variables turned off.");
    }
    return;
  }
  if (is_abbrev(field,"pkill")) {
    sscanf(name,"%d",&parm);
    PKILLABLE=parm;
    if(PKILLABLE==1) {
      send_to_char("Pkill is now turned on for players.\n\r", ch);
      log_msg("Pkill turned on.");
      do_system(ch, "$CWClan Pkill turned on. Enjoy.$CN", 0);
    } else if(PKILLABLE==2) {
      send_to_char("Arena Pkill is now turned on for players.\n\r", ch);
      log_msg("Arena Pkill turned on.");
      do_system(ch, "$CWArena Pkill turned on. GO HOUSE!$CN", 0);
    } else {
      send_to_char("Pkill is now turned off.\n\r", ch);
      log_msg("Pkill turned off.");
      do_system(ch, "$CWAs the gods wave the Pansey Wand, Peace spreads across the land...$CN", 0);
    }
    return;
  }
  if (is_abbrev(field,"fountlev")) {
    sscanf(name,"%d",&parm);
    FOUNTAIN_LEVEL=parm;
    sprintf(buf, "The fountain will now give experience up to level %d.\n\r", FOUNTAIN_LEVEL);
    send_to_char(buf, ch);
    return;
  }
  if (is_abbrev(field,"fountgold")) {
    sscanf(name,"%d",&parm);
    FOUNTAIN_GOLD=parm;
    sprintf(buf, "The fountain will now give %d coins.\n\r", FOUNTAIN_GOLD);
    send_to_char(buf, ch);
    return;
  }

  if (is_abbrev(field,"group")) {
    sscanf(name,"%d",&parm);
    GROUP_RES = parm;
    if (!parm || parm > 2 || parm < 0) {
      send_to_char("You must provide one of the following values:\n\r", ch);
      send_to_char("1 - Turns group restrictions off.\n\r", ch);
      send_to_char("2 - Turns group restrictions on.\n\r", ch);
      return;
    }
    if (GROUP_RES == 1) {
      send_to_char("Group restrictions have been turned off.\n\r", ch);
      do_system(ch, "$CWThe gods favor the young and inexperienced - $CRTrains are GO!.$CN", 0);      
    }
    if (GROUP_RES == 2) {
      send_to_char("Group restrictions have been turned on.\n\r", ch);
      do_system(ch, "$CWThe gods demand more exploration - Trains are $CRDEAD$CW.$CN", 0);      
    }
    return;
  }

  if (is_abbrev(field,"areafx")) {
    sscanf(name,"%d",&parm);
    AREA_FX_DEADLY = parm;
    if (!parm || parm > 2 || parm < 0) {
      send_to_char("You must provide one of the following values:\n\r", ch);
      send_to_char("1 - Makes Area FX safe for players.\n\r", ch);
      send_to_char("2 - Makes Area FX hurt players not following.\n\r", ch);
      return;
    }
    if (AREA_FX_DEADLY == 1) {
      send_to_char("Area FX is now harmless to players.\n\r", ch);
      do_system(ch, "$CWThe gods have claimed enough deaths, you may now travel in peace, unharmed from area effects.$CN", 0);      
    }
    if (AREA_FX_DEADLY == 2) {
      send_to_char("Area FX is now Deadly!.\n\r", ch);
      do_system(ch, "$CRThe gods demand new souls. Beware of powerful area effects that spread destruction in our lands, as they will destroy you!$CN", 0);      
    }

    return;
  }

  if (is_abbrev(field,"disaster")) {
    sscanf(name,"%d",&parm);
    DISASTER=parm;
    if(DISASTER==1)
      send_to_char("An earthquake will occur at 11am game time.\n\r",ch);
    if(DISASTER==2)
      send_to_char("A volcano will errupt  upon the land at 11am game time.\n\r",ch);
    if(DISASTER==3)
      send_to_char("A lightning storm will scorch the land at 11am game time.\n\r",ch);
    if(DISASTER==4)
      send_to_char("A tornado will rip across the land at 11am game time.\n\r",ch);
    if(DISASTER==5)
      send_to_char("A dark cloud will decend upon the land at 11am game time.\n\r",ch);
    return;
  }
  if (is_abbrev(field,"pirate")) {
    sscanf(name,"%d",&parm);
    PIRATEQST = parm;
    if(PIRATEQST==1)
      send_to_char("Ok.  A pirate quest of HARD difficulty will occur at 4am game time.\n\r", ch);
    if(PIRATEQST==2)
      send_to_char("Ok.  A pirate quest of EASY difficulty will occur at 4am game time.\n\r", ch);
    if(PIRATEQST==3)
      send_to_char("Ok.  A pirate quest of MEDIUM difficulty will occur at 4am game time.\n\r", ch);
    return;
  }
  if (is_abbrev(field,"casino")) {
    sscanf(name,"%d",&parm);
    CASINO_BANK = parm;
    send_to_char("Casino bank set.\n\r", ch);
    return;
  }
  /* I didn't want to make a serperate set casino gold command */

  if ((mob = get_char_vis(ch, name)) == NULL) {
    send_to_char("I don't see that person here \n\r",ch);
    return;
  }
  if (is_abbrev(field, "seeinvis")) {
  	sscanf(parmstr, "%d", &parm);
  	parm = MAX(0, parm);
  	parm = MIN(9, parm);
  	if (parm > TRUST(ch))
  		mob->specials.see_invis_level = TRUST(ch);
        else
  		mob->specials.see_invis_level = parm;

  	sprintf(buf2, "%s can now see to invis level %d\n\r", GET_NAME(mob), mob->specials.see_invis_level);
  	send_to_char(buf2, ch);
  	return;
  }
  if (is_abbrev(field,"glevel")) {
    sscanf(parmstr,"%d",&parm);
    mob->guild_level = parm;
  } else if (is_abbrev(field,"guild")) {
    sscanf(parmstr,"%d",&parm);
    mob->in_guild = parm;
  } else if (is_abbrev(field,"gnumber")) {
    sscanf(parmstr,"%d",&parm);
    mob->player.guildinfo.guildnumber = parm;  /* by maarek */
  } else if (is_abbrev(field,"gflags")) {
    sscanf(parmstr,"%d",&parm);
    mob->player.guildinfo.guildflags = parm;   /* by maarek */
  } else if (is_abbrev(field,"timer")) {
    sscanf(parmstr,"%d",&parm);
    mob->specials.timer = parm;
  } else if (is_abbrev(field, "nowiz")) {
    if (!IS_PC(mob)) {
      send_to_char("You can't do that to a mob!\n\r", ch);
      return;
    }
    if (!str_cmp(parmstr, "off")) {
      REMOVE_BIT(mob->specials.flags, PLR_NOWIZ);
      send_to_char("Player will now be listed as a god.\n\r", ch);
    } else {
	SET_BIT(mob->specials.flags, PLR_NOWIZ);
	send_to_char("Player will no longer be listed as a god.\n\r", ch);
    }
  } else if (is_abbrev(field,"alignment")) {
      if (IS_PC(ch)) {
	  sscanf(parmstr,"%d",&parm);
	  GET_ALIGNMENT(mob) = parm;
      }

  } else if (is_abbrev(field,"class")) {
    sscanf(parmstr,"%d",&parm);
    /*
     ** this will do almost nothing. (hopefully);
     */
    mob->player.clss = parm;
  } else if (is_abbrev(field,"experience")) {
    sscanf(parmstr,"%d",&parm);
    GET_EXP(mob) = (EXP) parm;
  } else if (is_abbrev(field, "level")) {
    parm2 = -1;			/* mage */
    sscanf(parmstr,"%d", &parm);
    argument=one_argument(argument, parmstr);
    sscanf(parmstr,"%s", str);

    if (is_abbrev(str, "mage")) parm2 = MAGE_LEVEL_IND;
    else if (is_abbrev(str, "cleric")) parm2 = CLERIC_LEVEL_IND;
    else if (is_abbrev(str, "warrior")) parm2 = WARRIOR_LEVEL_IND;
    else if (is_abbrev(str, "thief")) parm2 = THIEF_LEVEL_IND;
    else if (is_abbrev(str, "knight")) parm2 = PALADIN_LEVEL_IND;
    else if (is_abbrev(str, "druid")) parm2 = DRUID_LEVEL_IND;
    else if (is_abbrev(str, "psionist")) parm2 = PSI_LEVEL_IND;
    else if (is_abbrev(str, "ranger")) parm2 = RANGER_LEVEL_IND;
    else if (is_abbrev(str, "shifter")) parm2 = SHIFTER_LEVEL_IND;
    else if (is_abbrev(str, "monk")) parm2 = MONK_LEVEL_IND;
    else if (is_abbrev(str, "bard")) parm2 = BARD_LEVEL_IND;
    else if (is_abbrev(str, "all")) parm2 = -1;
    else {
      send_to_char("Usage: @set level <char> <level> <class>\n\r", ch);
      send_to_char("Where <class> is an abbreviation for a valid class, or the word all\n\r", ch);
      return;
    }

    if (!IS_NPC(mob)) {
      if ((TRUST(mob) > TRUST(ch)) && (ch != mob)) {
	send_to_char(GET_NAME(ch), mob);
	send_to_char(" just tried to change your level.\n\r",mob);
	return;
      }
    } else {
      GET_LEVEL(mob, parm2) = parm;
      UpdateMaxLevel(mob);
      UpdateMinLevel(mob);
      return;
    }
    if (TRUST(ch) >= TRUST_IMP)
    {
	if (parm > ABS_MAX_LVL)
	    parm = ABS_MAX_LVL;
	if(parm2 == -1)
	{
	    for (parm2 = 0; parm2 <= MAX_LEVEL_IND; parm2++)
		GET_LEVEL(mob, parm2) = parm;
	}
	else
	    GET_LEVEL(mob, parm2) = parm;

	UpdateMaxLevel(mob);
	UpdateMinLevel(mob);
    }
    else
	send_to_char("You can't do that quite yet\n\r", ch);

  } else if (is_abbrev(field, "sex")) {
    if (is_number(parmstr)) {
      sscanf(parmstr,"%d",&parm);
      GET_SEX(mob) = parm;
    } else {
      send_to_char("argument must be a number\n\r", ch);
    }
  } else if (is_abbrev(field, "race")) {
    if (is_number(parmstr)) {
      sscanf(parmstr,"%d",&parm);
      GET_RACE(mob) = parm;
    } else {
      send_to_char("argument must be a number\n\r", ch);
    }
  } else if (is_abbrev(field, "hitpts")) {
    sscanf(parmstr,"%d",&parm);
    GET_HIT(mob) = parm;
    update_pos(mob);
  } else if (is_abbrev(field, "mhitpts")) {
    sscanf(parmstr,"%d",&parm);
    mob->points.max_hit = parm;
    update_pos(mob);
#if 0
  } else if (is_abbrev(field, "board")) {
    board_kludge_char = ch;
#endif
  } else if (is_abbrev(field, "tohit")) {
    sscanf(parmstr,"%d", &parm);
    GET_HITROLL(mob)=parm;
  } else if (is_abbrev(field, "todam")) {
    sscanf(parmstr,"%d", &parm);
    GET_DAMROLL(mob)=parm;
  } else if (is_abbrev(field, "armorclass")) {
    sscanf(parmstr,"%d", &parm);
    GET_AC(mob)=parm;
  } else if (is_abbrev(field, "bank")) {
    sscanf(parmstr, "%d", &parm);
    GET_BANK(mob) = parm;
  } else if (is_abbrev(field, "gold")) {
    sscanf(parmstr, "%d", &parm);
    GET_GOLD(mob) = parm;
  } else if (is_abbrev(field, "prac")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.spells_to_learn = parm;
  } else if (is_abbrev(field, "age")) {
    sscanf(parmstr, "%d", &parm);
    mob->player.time.birth -= SECS_PER_MUD_YEAR*parm;
  } else if (is_abbrev(field, "str")) {
    int add = 0;
    sscanf(parmstr, "%d%d", &parm, &add);
    mob->abilities.str = parm;
    if(mob->abilities.str == 18)
      mob->abilities.str_add = add;
    else
      mob->abilities.str_add = 0;
  } else if (is_abbrev(field, "stradd")) {
    sscanf(parmstr, "%d", &parm);
    if (mob->abilities.str == 18) {
      mob->abilities.str_add = parm;
    } else {
      send_to_char("Str must be 18 to set this.\n\r", ch);
      return;
    }
  } else if (is_abbrev(field, "int")) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.intel = parm;
  } else if (is_abbrev(field, "wis")) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.wis = parm;
  } else if (is_abbrev(field, "dex")) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.dex = parm;
  } else if (is_abbrev(field, "con")) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.con = parm;
  } else if (is_abbrev(field, "chr")) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.cha = parm;

  } else if (is_abbrev(field, "deny")) {
    if(IS_MOB(mob))
      send_to_char("You can't do this to a mob!\n\r", ch);
    else {
    if(!IS_SET( mob->specials.flags, PLR_DENY ))
      {
        SET_BIT( mob->specials.flags, PLR_DENY );
      } else {
	REMOVE_BIT( mob->specials.flags, PLR_DENY );
      }
    }

  } else if (is_abbrev(field, "peaceful")) {
    sscanf(parmstr, "%d", &parm);
    PeacefulFlag = parm;

  } else if (is_abbrev(field, "brujah")){
    if (IS_SET(mob->specials.flags, PLR_BRUJAH)){
    REMOVE_BIT(mob->specials.flags, PLR_BRUJAH);
      send_to_char("brujah flag removed\n\r",ch);
    }else{
      SET_BIT(mob->specials.flags, PLR_BRUJAH);
      send_to_char("brujah flag added\n\r",ch);
    }

  } else if (is_abbrev(field, "summon")){
    if (IS_SET(mob->specials.flags, PLR_SUMMON)){
    REMOVE_BIT(mob->specials.flags, PLR_SUMMON);
      send_to_char("summon flag removed\n\r",ch);
    }else{
      SET_BIT(mob->specials.flags, PLR_SUMMON);
      send_to_char("summon flag added\n\r",ch);
    }

  } else if (is_abbrev(field, "showdam")){
    if (IS_SET(mob->specials.flags, PLR_SHOW_DAM)){ /* alex: */
    REMOVE_BIT(mob->specials.flags, PLR_SHOW_DAM);
      send_to_char("showdam flag removed\n\r",ch);
    }else{
      SET_BIT(mob->specials.flags, PLR_SHOW_DAM);
      send_to_char("showdam flag added\n\r",ch);
    }
    return;

  } else if (is_abbrev(field, "notrack")) {
     if (IS_SET(mob->specials.mob_act, ACT_NOTRACK)) {
	REMOVE_BIT(mob->specials.mob_act, ACT_NOTRACK);
	cprintf(ch,"notrack flag removed\n");
     } else {
	SET_BIT(mob->specials.mob_act, ACT_NOTRACK);
	cprintf(ch,"notrack flag added\n");
     }
  } else if (is_abbrev(field, "mana")) {
    sscanf(parmstr, "%d", &parm);
    mob->points.max_mana = parm;
    mob->points.mana = parm;

  } else if (is_abbrev(field, "move"))  {
    sscanf(parmstr, "%d", &parm);
    mob->points.move = parm;

  } else if (is_abbrev(field, "maxmove")) {
    sscanf(parmstr, "%d", &parm);
    mob->points.max_move = parm;

  } else if (is_abbrev(field, "bhdice")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.damnodice = parm;

  } else if (is_abbrev(field, "bhsize")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.damsizedice = parm;

  } else if (is_abbrev(field, "thirst")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.conditions[THIRST] = parm;
  } else if (is_abbrev(field, "hunger")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.conditions[FULL] = parm;
  } else if (is_abbrev(field, "drunk")) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.conditions[DRUNK] = parm;
  } else if (is_abbrev(field, "trust")) {
      if ((!HAS_GCMD(ch,GCMD_WIZSET)) || (TRUST(ch) < MAX_TRUST))
      {
	  send_to_char("The IMPs laugh at you.\n\r", ch);
          return;
      }
      sscanf(parmstr, "%d", &parm);
      mob->player.trust = MIN(TRUST(ch), MAX(0, parm));
      sprintf(buf, "%s trust set to %d.\n\r", GET_NAME(mob), TRUST(mob));
      send_to_char(buf, ch);
  } else if (is_abbrev(field, "home")) {
    sscanf(parmstr, "%d", &parm);
    GET_HOME(mob) = parm;
  } else if (is_abbrev(field, "delete")) {
    if(IS_MOB(mob))
      send_to_char("You can't do this to a mob!\n\r", ch);
    else {
      sscanf(parmstr, "%s", buf2);
      if (is_abbrev(buf2, "delete")) {
	if(IS_SET(mob->delete_flag, PROTECT)) {
	  send_to_char("This character is protected, you must remove the protect flag first.\n\r", ch);
	  return;
	} else if(IS_SET(mob->delete_flag, DELETE)) {
	  REMOVE_BIT(mob->delete_flag, DELETE);
	  send_to_char("Delete flag removed.\n\r", ch);
	} else {
	  SET_BIT(mob->delete_flag, DELETE);
	  send_to_char("Delete flag set.\n\r", ch);
	}
      } else if (is_abbrev(buf2, "protect")) {
	if(IS_SET(mob->delete_flag, PROTECT)) {
	  REMOVE_BIT(mob->delete_flag, PROTECT);
	  send_to_char("Protect flag removed.\n\r", ch);
	} else {
	  SET_BIT(mob->delete_flag, PROTECT);
	  REMOVE_BIT(mob->delete_flag, DELETE);
	  send_to_char("Protect flag set.\n\r", ch);
	}
      }
      else if(is_abbrev(buf2, "housed"))
      {
	if(IS_SET(mob->delete_flag, HOUSED))
	{
	  REMOVE_BIT(mob->delete_flag, HOUSED);
	  send_to_char("Housed flag removed.\n\r", ch);
	}
	else
	{
	  SET_BIT(mob->delete_flag, HOUSED);
	  REMOVE_BIT(mob->delete_flag, DELETE);
	  send_to_char("Housed flag set.\n\r", ch);
	}
      } else {
	send_to_char("Usage @set delete <player> [delete | protect]\n\r", ch);
	send_to_char("  to toggle the delete or protect flags\n\r", ch);
      }
    }

  } else {
    do_set(ch, "help", cmd);
    return;
  }
  if(!IS_NPC(mob))
    add_char_to_hero_list(mob);
}


void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("If you want to shut something down - say so!\n\r", ch);
}


int shutdown_func(event_t* theEvent, long now)
{
    char buf[100];
    int when, next_time;
    struct descriptor_data *d;

    sprintf(buf, "\n\r\aThe game will autoreboot in %d minute%s.\n\r\n\r",
	    shutdown_time, shutdown_time == 1 ? "" : "s");

    if (shutdown_time > 10) {
	next_time = shutdown_time - 10;
	shutdown_time = 10;
    }
    else if (shutdown_time > 5) {
	next_time = shutdown_time - 5;
	shutdown_time = 5;
    }
    else if (shutdown_time > 1) {
	next_time = shutdown_time - 1;
	shutdown_time = 1;
    }
    else if (shutdown_time > 0) {
	next_time = shutdown_time;
	shutdown_time = 0;
    }
    else if (shutdown_time == 0) {
	shutdown_time = -1;

	EACH_DESCRIPTOR(d_iter, d)
	{
	    if (!d->connected && d->character)
	    {
		send_to_char("The demi-lich has forced you to 'wake'.\n\r",
			     d->character);
		do_wake(d->character, "", 46);
		send_to_char("Evenita has forced you to 'stand'.\n\r",
			     d->character);
		do_stand(d->character, "", 42);
		send_to_char("The avatar of Lue Chi Woe has forced you to 'return'.\n\r", d->character);
		do_return(d->character, "", 204);
		send_to_char("The Purple Spotted Snorklewacker has forced you to 'save'.\n\r", d->character);
		do_save(d->character, "", 69);

		switch (number(0, 4))
		{
		case 0:
		    send_to_char("Seraph has forced you to 'cough'.\n\r",
				 d->character);
		    do_action(d->character, "", 109);
		    break;
		case 1:
		    send_to_char("Raist has forced you to 'crash'.\n\r",
				 d->character);
		    do_action(d->character, "", 430);
		    break;
		case 2:
		    send_to_char("Harrier has forced you to 'BONK'.\n\r",
				 d->character);
		    do_action(d->character, " self", 94);
		    break;
		case 3:
		    send_to_char("Mallune has forced you to 'ROFL'.\n\r",
				 d->character);
		    do_action(d->character, "", 453);
		    break;
		case 4:
		    send_to_char("Moiraine has forced you to 'violin'.\n\r",
				 d->character);
		    do_action(d->character, "", 468);
		    break;
		}
	    }
	}
	END_ITER(d_iter);

	send_to_all_regardless("Rebooting... Thieves World will return in just a moment.\n\r");
	shutdown_event = event_queue_pulse(theEvent, pulse + 1, shutdown_func,
					   "auto shutdown");
	return 1;
    }
    else {
	goaway = lc_reboot = 1;
	return 1;
    }

    send_to_all_regardless(buf);
    when = time(0) + 60 * next_time;
    shutdown_event = event_queue_real(theEvent, when, shutdown_func,
				      "auto shutdown");

    return 1;
}


void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
    char buf[100], arg[MAX_INPUT_LENGTH];
    int tm;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, arg);

    if (!*arg) {
	sprintf(buf, "Shutdown by %s.", GET_NAME(ch) );
	log_msg(buf);
	strcat(buf, "\n\r");
	send_to_all_regardless(buf);
	goaway = 1;
    }

    else if (!str_cmp(arg, "reboot")) {
	sprintf(buf, "Reboot by %s.", GET_NAME(ch));
	log_msg(buf);
	strcat(buf, "\n\r");
	send_to_all_regardless(buf);
	goaway = lc_reboot = 1;
    }

    else if (!str_cmp(arg, "timed")) {
	one_argument(argument, arg);

	if (!*arg) {
	    if (!shutdown_event)
		send_to_char("No shutdown is scheduled.\n\r", ch);
	    else {
		tm = shutdown_event->when - time(0);
		sprintf(buf, "Shutdown will occur in %.1f minutes.\n\r",
			(float) tm / 60 + (float) shutdown_time);
		send_to_char(buf, ch);
	    }
	}
	else if ((tm = atoi(arg)) > 0) {
	    if (shutdown_event) {
		send_to_char("There is already a shutdown scheduled.  Use \"shutdown cancel\"\n\rbefore rescheduling.\n\r", ch);
		return;
	    }

	    shutdown_time = tm;

	    send_to_char("Okay.\n\r", ch);
	    sprintf(buf, "Automatic shutdown started by %s for %d minutes.",
		    GET_NAME(ch), tm);
	    slog(buf);
	    WriteToImmort(buf, MAX(TRUST_GRUNT, ch->invis_level));

	    shutdown_func(NULL, pulse);
	} else
	    send_to_char("If you want to shutdown now, just use \"shutdown\".\n\r", ch);
    }

    else if (!str_cmp(arg, "cancel")) {
	if (!shutdown_event)
	    send_to_char("No shutdown had been scheduled.\n\r", ch);
	else {
	    event_cancel(shutdown_event, 1);
	    shutdown_event = NULL;

	    send_to_char("Okay.\n\r", ch);
	    send_to_all("Automatic shutdown has been canceled.\n\r");
	    sprintf(buf, "Shutdown canceled by %s.", GET_NAME(ch));
	    slog(buf);
	    WriteToImmort(buf, MAX(TRUST_GRUNT, ch->invis_level));
	}
    }

    else
	send_to_char("Go shut down someone your own size.\n\r", ch);
}


void do_snoop(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  struct char_data *victim;

  if (!ch->desc)
    return;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if(!*arg)
  {
      if(ch->desc && ch->desc->snoop.snooping)
      {
	  sprintf(arg, "You are snooping: %s\n\r",
		  GET_NAME(ch->desc->snoop.snooping));
	  send_to_char(arg, ch);
      }
      else
	  send_to_char("You aren't nosy right now\n\r", ch);
      return;
  }

  if(!(victim=get_char_vis(ch, arg)) || (!CAN_SEE(ch,victim)))	{
    send_to_char("No such person around.\n\r",ch);
    return;
  }

  if(!victim->desc)	{
    send_to_char("There's no link.. nothing to snoop.\n\r",ch);
    return;
  }
  if(victim == ch)	{
    send_to_char("Ok, you just snoop yourself.\n\r",ch);
    if(ch->desc->snoop.snooping) {
      if (ch->desc->snoop.snooping->desc)
	ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
      else {
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "caught %s snooping %s who didn't have a descriptor!",
		GET_REAL_NAME(ch), GET_NAME(ch->desc->snoop.snooping));
	log_msg(buf);
      }
      ch->desc->snoop.snooping = 0;
    }
    return;
  }

  if(victim->desc->snoop.snoop_by) {
    send_to_char("Busy already. \n\r",ch);
    return;
  }

  if ((TRUST(victim) >= TRUST(ch)) && (TRUST(ch) < 10))	{
    send_to_char("You failed.\n\r",ch);
    return;
  }

  send_to_char("Ok. \n\r",ch);
  if (TRUST(victim) == 10)
    act("$n started snooping you.", FALSE, ch, NULL, victim, TO_VICT);

  if(ch->desc->snoop.snooping)
    if (ch->desc->snoop.snooping->desc)
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  return;
}



void do_switch(struct char_data *ch, char *argument, int cmd)
{
  static char arg[80];
  struct char_data *victim = NULL;

  if(!ch->desc)
    return;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if (!*arg)	{
    send_to_char("Switch with who?\n\r", ch);
  }
  else	{

    /* case a:  victim is in the room with char, preference 1
       case b:  victim is elsewhere...  get_char_vis() should work? */

    victim = get_char_room(arg, ch->in_room);

    if (!victim) {
      victim = get_char_vis(ch, arg);
    }

    if (!victim) {
      send_to_char("They aren't here.\n\r", ch);
    }

    else   	{
      if (ch == victim)   {
	send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
	return;
      }
      /* Mixing snoop & switch is bad for your health. */
      if (!ch->desc ||
	  ch->desc->snoop.snoop_by ||
	  ch->desc->snoop.snooping)
      {
	  send_to_char("Switch has been temporarily disabled.\n\r", ch);
	  return;
      }

      if(victim->desc || (!IS_NPC(victim)))       {
	  send_to_char("You can't do that, the body is already in use!\n\r",
		       ch);
      }  else	{
	  send_to_char("Ok.\n\r", ch);
	  push_character(ch, victim);
      }
    }
  }
}

void shifter_normalize(struct char_data *ch)
{
  if (affected_by_spell(ch, SKILL_FORM))
    do_form(ch, "", -1);

  if (affected_by_spell(ch, SKILL_PLATE))
    do_plate(ch, "", -1);

  if (affected_by_spell(ch, SKILL_CHAMELEON))
    do_chameleon(ch, "", -1);
}

void trans_fights(struct char_data *old, struct char_data *nw)
{
  struct char_data *tmp;

  if (!old->attackers && !old->specials.fighting)
    return;

  if (old->specials.fighting) {
    tmp=old->specials.fighting;
    stop_fighting(old);
    set_fighting(nw, tmp);
  }

  EACH_CHARACTER(iter, tmp)
  {
    if (tmp->specials.fighting==old) {
      stop_fighting(tmp);
      set_fighting(tmp, nw);
    }
  }
  END_AITER(iter);
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *per;

  if (!ch->desc)
    return;

  if (!(per = pop_character(ch))) {
    send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
    return;
  }
  if(auction->item != NULL)
  {
     if(ch==auction->buyer)
         auction->buyer = per;
     if(ch== auction->seller)
         auction->seller = per;
  }

  if (cmd>=0) {
    if (mob_index[ch->nr].func == liquid_proc)
      send_to_char("You solidify and return to your original form.\n\r", per);
    else if (mob_index[ch->nr].func == cocoon_proc)
      send_to_char("You come out of your regenerative cocoon.\n\r", per);
    else
      send_to_char("You return to your original form.\n\r", per);
  }

  if (IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {

    shifter_normalize(ch);
/*    if (affected_by_spell (ch, SKILL_SHIFT))
      affect_from_char (ch, SKILL_SHIFT);*/

    char_from_room(per);
    char_to_room(per, ch->in_room);

    per->specials.flags = ch->specials.flags;

    SwitchStuff(ch, per);

    if (cmd>=0) {
      if (mob_index[ch->nr].func == liquid_proc)
        act("$n begins to stir and solidify, reforming as $N.",TRUE,ch,0,per,TO_ROOM);
      else if (mob_index[ch->nr].func == cocoon_proc)
        act("$n liquifies and shifts, reforming as $N.",TRUE,ch,0,per,TO_ROOM);
      else
        act("$n turns liquid, and reforms as $N",TRUE,ch,0,per,TO_ROOM);
    }

    trans_fights(ch, per);

    extract_char(ch);
    do_save(per, "", 0);
  }
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH+40];

    if (IS_NPC(ch) && (cmd != 0))
	return;

    half_chop(argument, name, to_force);

    if (!*name || !*to_force)
	send_to_char("Who do you wish to force to do what?\n\r", ch);
    else if (*(to_force) == '!') {
	send_to_char("You can't force someone to do that!\n\r", ch);
	return;
    }
    else
	{  /* if (!*name || !*to_force) */
	    if (str_cmp("all", name) && str_cmp("room", name))
		{
		    if (!(vict = get_char_vis(ch, name)) \
			|| (!IS_NPC(vict) && (!CAN_SEE(ch,vict))))
			send_to_char("No-one by that name here..\n\r", ch);

		    else /* if (!vict... */
			{
			    if ((TRUST(ch) <= TRUST(vict) && TRUST(ch) != MAX_TRUST) || (!HAS_GCMD(ch,GCMD_SNOOP) && !IS_NPC(vict)))
				{
				send_to_char("Oh no you don't!!\n\r", ch);
				}
			    else  /* if (TRUST(ch)... */
				{
				    sprintf(buf, "$n has forced you to '%s'.", to_force);
				    act(buf, FALSE, ch, 0, vict, TO_VICT);
				    send_to_char("Ok.\n\r", ch);
				    command_interpreter(vict, to_force, 1);
				}
			}
		}

	    else  /* we're either 'room' or 'all' */
		{
		    if (!str_cmp("room", name))
			{
			    EACH_CHARACTER(iter, vict)
				{
				    if (ch->in_room==vict->in_room)
					{
					    if (!CAN_SEE(ch, vict))
						break;
					    if (TRUST(ch) <= TRUST(vict))
						break;
					    sprintf(buf, "$n has forced you to '%s'.", to_force);
					    act(buf, FALSE, ch, 0, vict, TO_VICT);
					    command_interpreter(vict, to_force, 1);
					}
				}
			    END_AITER(iter);
			    send_to_char("Ok.\n\r", ch);
			}
		    else if (!HAS_GCMD(ch,GCMD_SYSTEM))
			 {
			  send_to_char("Oh no you don't!!\n\r", ch);
			 }
		      else
			{
			    EACH_DESCRIPTOR(d_iter, i)
				{
				    if (i->character != ch && !i->connected) {
					vict = i->character;
					if (TRUST(ch) <= TRUST(vict))
					    ;
					else {
					    sprintf(buf, "$n has forced you to '%s'.", to_force);
					    act(buf, FALSE, ch, 0, vict, TO_VICT);
					    command_interpreter(vict, to_force, 1);
					}
				    }
				}
			    END_ITER(d_iter);
			    send_to_char("Ok.\n\r", ch);
			}
		}
	}
}



void do_expset(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH], parmstr[MAX_INPUT_LENGTH], message[30];
    struct char_data *mob;
    int parm;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, name);
    argument = one_argument(argument, parmstr);

    sscanf(parmstr,"%d",&parm);

    if (!name[0] || !parmstr[0] || !strcmp(name,"help"))
    {
	send_to_char(" exp player value  adds on to players experience the amount of value.\n\r",ch);
	send_to_char(" exp amount will give you the current max.\n\r",ch);
	send_to_char(" exp auth will allow you to change the current max.\n\r", ch);
	return;
    }
    else if ((!strcmp(name,"auth")) && (TRUST(ch) >= TRUST_LORD))
    {
	MAX_EXP_REIMB = parm;
	sprintf(message,"Max exp reimb set to %d. \n\r",MAX_EXP_REIMB);
	send_to_char(message,ch);
	return;
    }
    else if (!strcmp(name,"amount"))
    {
	sprintf(message,"Max exp reimb is %d. \n\r",MAX_EXP_REIMB);
	send_to_char(message,ch);
	return;
    }

    if (((mob = get_char_vis(ch, name)) == NULL) || (!CAN_SEE(ch,mob))){
	send_to_char("That player is not around \n\r",ch);
	return;
    }

    if (!IS_PC(mob)) {
	send_to_char("Only players need to be reimbursed \n\r",ch);
	return;
    }

    if (parm > MAX_EXP_REIMB) {
	send_to_char("Value is too high. \n\r",ch);
	return;
    }

    if ((GET_EXP(mob) += parm) >= 150000000) {
	send_to_char("That character doesn't need reimbursing! \n\r",ch);
	GET_EXP(mob) -= parm;
    }
    else send_to_char("Done \n\r",ch);

    /*if (GET_EXP(mob)<0) GET_EXP(mob)=0;*/
    /* Not 100% sure why this was here, but anyways unsigned are always >0 */
    /* GET_EXP(mob) = 0; */
}


void do_csocket(struct char_data *ch, char *argument, int cmd)
{
    int i;
    struct descriptor_data *d;
    char buf[300];

    if (IS_NPC(ch))
	return;

    i = 0;
    if(scan_number(argument, &i)) {
	if (i > 0) {
	    EACH_DESCRIPTOR(d_iter, d)
	    {
		if (d->descriptor == i)
		{
		    if (d->character &&
			TRUST(ch) <= TRUST(d->character))
			send_to_char("No, that's violent.\n\r", ch);
		    else
		    {
			sprintf(buf,"CSOCKET - a socket was closed by %s.",
				GET_NAME(ch));
			log_msg(buf);
			close_socket(d, TRUE);
		    }
		    break;
		}
	    }
	    END_ITER(d_iter);
	    if(!d)
		send_to_char("Sorry, I couldn't find that socket.\n\r", ch);
	} else
	    send_to_char("Number must be greater than 0.\n\r", ch);
    } else
	send_to_char("You must supply a number that's in the users list.\n\r",
		     ch);
}


void do_freeze(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob;
    char name[MAX_INPUT_LENGTH],*arg2;

    arg2 = one_argument(argument, name);
    if (!(mob=get_char_vis(ch,name)) || (!CAN_SEE(ch,mob))) {
	send_to_char("You can't seem to find anyone by that name!\n\r",ch);
	return;
    }
    if (TRUST(mob) >= TRUST(ch)) {
	send_to_char("I don't think so!\n\r",ch);
	return;
    }
    if(mob->freeze) {
	send_to_char("Unfrozen.\n\r",ch);
	mob->freeze=0;
	return;
    }
    mob->freeze=1;
    send_to_char("Frozen. (freeze again to unfreeze)\n\r",ch);
    return;
}


void do_zreset(struct char_data *ch, char *argument, int cmd)
{
  int z, i;

  if (!strcmp(argument,"")) {
    reset_zone((real_roomp(ch->in_room))->zone, 0);
    send_to_char("The zone is now loaded and reset.\n\r", ch);
    return;
  }

  z = atoi(argument);

  if (z == -1)
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i, 0);
  else if (z >= 0 && z < top_of_zone_table)
    reset_zone(z, 0);
  else {
    send_to_char("That's not a real zone.\n\r", ch);
    return;
  }
  send_to_char("The zone is now loaded and reset.\n\r", ch);
}

void do_make(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob;
    struct obj_data *obj;
    char type[MAX_INPUT_LENGTH], num[MAX_INPUT_LENGTH];
    char fname[MAX_INPUT_LENGTH], buf[80], helpbuf[MAX_STRING_LENGTH];

    char loadtype[MAX_INPUT_LENGTH];
    char randomname[MAX_INPUT_LENGTH];
    char randombuf[MAX_INPUT_LENGTH];
    char sustype[MAX_INPUT_LENGTH];
    char susname[MAX_INPUT_LENGTH];

    char* suslist[] =  {"fire", "cold", "electricity", "energy", "blunt",
			"pierce", "slash", "acid", "poison", "drain",
			"sleep", "charm", "non-magic", "hold", "bard",
	                0};

    int rand_num = -2, number, load_type=0, i;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, type);
    only_argument(argument, num);
    argument = one_argument(argument, loadtype);

   if ((!strcmp("random", loadtype)) && (*argument != '\0')) {

      if(!HAS_GCMD(ch,GCMD_OLOAD)) {
	 send_to_char("You just don't have the skills quite yet.\n\r", ch);
	 return;
      }

      argument = one_argument(argument, num);
      only_argument(argument, sustype);
      argument = one_argument(argument, randomname);
      load_type = RAND;

      if(isdigit(*randomname))
	rand_num = atoi(randomname);

      if(!strcmp("chaos", randomname)) {
	 argument = one_argument(argument, sustype);
	 only_argument(argument, susname);

	 rand_num = -2;
	 for(i=0;suslist[i];i++) {
	    if(is_abbrev(susname, suslist[i])) {
	       rand_num = i;
	       break;
	    }
	 }

	 if(rand_num == -2) {
	    cprintf(ch, "Couldn't find a chaos load with that load.\n\r");
	    return;
	 }

	 if(is_abbrev(sustype, "susceptible"))  rand_num += RN_CHAOS_S_FIRE;
	 if(is_abbrev(sustype, "resistant"))	rand_num += RN_CHAOS_R_FIRE;
	 if(is_abbrev(sustype, "immune"))       rand_num += RN_CHAOS_I_FIRE;

	 if(rand_num < RN_CHAOS_S_FIRE) {
	    cprintf(ch, "That chaos load couldn't be found.\n\r");
	    return;
	 }
      }

      if(rand_num==-2) {
	 for(i=0;random_nodes[i].num;i++) {
	    if(!strcasecmp(sustype, random_nodes[i].name)) {
	       rand_num = i;
	       break;
	    }
	 }
      }

      if(rand_num == -2) {
	 cprintf(ch, "Couldn't find a random load with that name.\n\r");
	 return;
      } else {
	 sprintf(sustype, "%s loaded random item.", GET_IDENT(ch));
	 slog(sustype);
      }
   }
   else
     load_type = REAL;

    if ((!strcmp("full", loadtype)) && (HAS_GCMD(ch,GCMD_OLOAD) &&
	(*argument != '\0')))
      {
	only_argument(argument, num);
	load_type = NORAND;
      }
    else if (load_type != RAND)
      load_type = REAL;

    if (isdigit(*num))
	number = atoi(num);
    else
	number = -1;

    if (is_abbrev(type, "mobile"))	{
	if (number<0) {
	    for (number = 0; number<=top_of_mobt; number++)
		if (isname(num, mob_index[number].name))
		    break;
	    if (number>top_of_mobt)
		number = -1;
	} else {
	    number = real_mobile(number);
	}
	if ( number<0 || number>top_of_mobt) {
	    send_to_char("There is no such monster.\n\r", ch);
	    return;
	}
	if(!(mob = make_mobile(number, REAL)))
	{
	    send_to_char("Internal Errors Occured\n\r", ch);
	    return;
	}
	char_to_room(mob, ch->in_room);

	act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
	act("You load $N.", FALSE, ch, 0, mob, TO_CHAR);

    } else if (is_abbrev(type, "object"))	{

	if (number<0) {
	    for (number = 0; number<=top_of_objt; number++)
		if (isname(num, obj_index[number].name))
		    break;
	    if (number>top_of_objt)
		number = -1;
	} else {
	    number = real_object(number);
	}
	if ( number<0 || number>top_of_objt) {
	    send_to_char("There is no such object.\n\r", ch);
	    return;
	}

	switch(obj_index[number].virt) {
    /* IMP BOARD */
	case 3095:
	    /* Dancing Sword */
	case 22050:
	    /* ORIENTAL QUEST ITEMS */
	case 4126: case 4127: case 4128: case 4129: case 4130: case 4131:
	case 4132: case 4133: case 4134: case 4135: case 4136: case 4137:
	case 4138: case 4139: case 4140: case 4141: case 4142: case 4143:
	case 4144: case 4145:
	case 15831:
	    /* SILVER LONG SWORDS */
	case 24000: case 24001: case 24002: case 24003: case 24004:
	case 24005: case 24006: case 24007: case 24008: case 24009:
	case 24010:
	case 29000:
	    if (!HAS_GCMD(ch,GCMD_OLOAD))
	    {
		send_to_char("*cackle*  Your not a TACO!\n\r", ch);
		return;
	    }
	}
	if(((obj_index[number].number>=obj_index[number].limit)
	    || (obj_index[number].limit < 0))
	   && (!HAS_GCMD(ch,GCMD_OLOAD)))
	{
	    send_to_char("It takes REAL power to make non-loading items\n\r",
			 ch);
	    return;
	}

	//WriteToImmort(randombuf, LOG_QUILAN);

	if(!(obj = make_object(number, load_type, rand_num)))
	{
	    send_to_char("Internal Errors Occured.\n\r", ch);
	    return;
	}

	obj_to_char(obj, ch);
	act("$n grabs $p from nothingness!", FALSE, ch, obj, 0, TO_ROOM);
	act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (is_abbrev(type, "room")) {
	int	start, end;

	switch (sscanf(num, "%d %d %s", &start, &end, fname)) {
	case 3:			/* 2 numbers and a filename */
	    RoomLoad(ch, start, end, fname);
	    break;
	case 2:			/* we got both numbers */
	    RoomLoad(ch, start, end, GET_NAME(ch));
	    break;
	case 1:			/* we only got one, load it */
	    RoomLoad(ch, start, start, GET_NAME(ch));
	    break;
	default:
	    send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
	    break;
	}
    } else if (is_abbrev(type, "file")) {
	if (TRUST(ch) < TRUST_LORD) {
	    send_to_char("Sorry, you cannot do this.\n\r", ch);
	    return;
	}

	if(!*num) {
	    sprintf(buf, "Usage: load file <filename>\n\r"
		    "  where <filename> is one of these: actions, motd, help, "
		    "helptable, info,\n\r  credits, message, news, policy, "
		    "poses, twstory, mobprog2, objprog2, roomprog2, "
		    "zoneprog2 or imotd\n\r");
	    send_to_char(buf, ch);
	    return;
	}

        //for the smobprog2 parts...
        char *ptr;
        char ptr2[256];
        ptr=one_argument(num,ptr2);

        *ptr=0;
        ptr++;

	if(is_abbrev(num, "actions") || is_abbrev(num, "socials")) {
	    boot_social_messages();
	    send_to_char("Actions/Socials re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "motd")) {
	    file_to_string(MOTD_FILE, motd, sizeof(motd));
	    send_to_char("MOTD re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "imotd")) {
	    file_to_string(IMOTD_FILE, imotd, sizeof(imotd));
	    send_to_char("IMOTD re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "help")) {
	    file_to_string(HELP_PAGE_FILE, help, sizeof(help));
	    send_to_char("Help re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "helptable")) {
	    delete_help_index();
	    build_help_index();
	    send_to_char("Help Table Reloaded\n\r", ch);
	} else if(is_abbrev(num, "credits")) {
	    file_to_string(CREDITS_FILE, credits, sizeof(credits));
	    send_to_char("Credits re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "info")) {
	    file_to_string(INFO_FILE, info, sizeof(info));
	    send_to_char("Info re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "message")) {
	    load_messages();
	    send_to_char("Messages re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "news")) {
	    file_to_string(NEWS_FILE, news, sizeof(news));
	    send_to_char("News re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "policy")) {
	    file_to_string(POLICY_FILE, policy, sizeof(policy));
	    send_to_char("Policy re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "poses")) {
	    boot_pose_messages();
	    send_to_char("Poses re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "twstory")) {
	    file_to_string(TWSTORY_FILE, twstory, sizeof(twstory));
	    send_to_char("TWStory re-loaded.\n\r", ch);
	} else if(is_abbrev(num, "mobprog2")) {
	    boot_mprog2(1);
	} else if(is_abbrev(num, "objprog2")) {
	    boot_oprog2(1);
	} else if(is_abbrev(num, "roomprog2")) {
	    boot_rprog2(1);
	} else if(is_abbrev(num, "zoneprog2")) {
	    boot_zprog2(1);
	} else if(is_abbrev(num, "smobprog2")) {
	   long num;
	   char namebuf[256];

	   if(sscanf(ptr, "%ld %s", &num, namebuf) != 2) {
	      cprintf(ch, "Couldn't break apart arguments! (long, string)\n\r");
	   } else {
	      ReadSingleMobprog(namebuf, num, 1, 0, TRUE);
	   }
	} else
	    send_to_char("I don't recognize that name.\n\r", ch);
    }
    else if(is_abbrev(type, "player"))
    {
	struct char_data* victim;

	if(!HAS_GCMD(ch,GCMD_PLOAD))
	{
	    send_to_char("You want a player loaded, go log him in...\n\r", ch);
	    return;
	}

	if(!*num)
	{
	  send_to_char("Load who?\n\r", ch);
	  return;
	}

	if((victim = find_player_in_world(num)))
	{
	    send_to_char("That would be bad.  They're already playing.\n\r",
			 ch);
	    send_to_char("You feel a cold chill up your spine.\n\r", victim);
	    return;
	}

	if(!(victim = LoadChar(0, num, READ_ALL)))
	{
	    send_to_char("No player by that name.\n\r", ch);
	    return;
	}

	array_insert(&character_list, victim);

	char_to_room(victim, ch->in_room);

	victim->specials.tick = plr_tick_count++;
	if(plr_tick_count >= PLR_TICK_WRAP)
	    plr_tick_count = 0;

	act("You wave your hands and out of the smoke steps $N!",
	    TRUE, ch, 0, victim, TO_CHAR);
	act("$n waves $s hands and out of the smoke steps $N!",
	    TRUE, ch, 0, victim, TO_ROOM);
    } else {
        strcpy(helpbuf, "Usage: load (object|mobile) (number|name)\n\r"
                        "       load room start [end [filename]]\n\r");
	if(TRUST(ch) > TRUST_LORD)
		strcat(helpbuf, "       load object random (number|name) randomload\n\r");
        if(TRUST(ch) > TRUST_GRGOD)
                strcat(helpbuf, "       load file <filename>\n\r");
        if(HAS_GCMD(ch,GCMD_PLOAD))
                strcat(helpbuf, "       load player <player_name>\n\r");
        send_to_char(helpbuf, ch);
    }
}


void purge_one_room(room_num rnum, struct room_data *rp, int *range)
{
  struct char_data	*ch;
  struct obj_data	*obj;

  if (rnum==0 ||		/* purge the void?  I think not */
      rnum < range[0] || rnum > range[1])
    return;

  while (rp->people) {
    ch = rp->people;
    send_to_char("A god strikes the heavens making the ground around you erupt into a\n\r", ch);
    send_to_char("fluid fountain boiling into the ether.  All that's left is the Void.\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, 0);	/* send character to the void */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/* send item to the void */
  }

  completely_cleanout_room(rp); /* clear out the pointers */
  room_remove(rnum);
  room_count--;
}

void purge_char(struct char_data* vict);

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict = NULL, *next_v;
  struct obj_data *obj, *next_o;

  char name[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, name);

  if (*name) {  /* argument supplied. destroy single object or char */
      if ((vict = get_char_room_vis(ch, name)))	{
        if (!IS_NPC(vict) && (!CAN_SEE(ch,vict)))  {
          send_to_char("I don't see that here.\n\r", ch);
          return;
        }
	if (IS_PC(vict) &&
	    (((TRUST(ch) < TRUST(vict)) && (TRUST(ch) >= TRUST_SAINT))
            || (TRUST(ch) < TRUST_SAINT))) {
	  send_to_char("I'm sorry, Dave.  I can't let you do that.\n\r", ch);
	  return;
	}

        if (cmd==349) {  /* chomp */
          act("$n chomps you!",FALSE,ch,0,vict,TO_VICT);
          act("$n chomps $N whole!",
              FALSE,ch,0,vict,TO_NOTVICT);
        } else {
          act("$n disintegrates you! Ahhhh your molecules are ripping apart!",
              FALSE, ch, 0, vict, TO_VICT);
	  act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        }
	purge_char(vict);
      } else if ((obj = get_obj_in_list_vis
		         (ch, name, real_roomp(ch->in_room)->contents))) {
        if(!cmd==349)
	  act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	else
	  act("$n chomps $p.  BURP!", FALSE, ch, obj, 0, TO_ROOM);
	extract_obj(obj);
      } else	{
	argument = one_argument(argument, name);
	if (0==str_cmp("room", name)) {
	  room_num range[2];
	  if (TRUST(ch) < TRUST_IMP) {
	    send_to_char("I'm sorry, Dave.  I can't let you do that.\n\r", ch);
	    return;
	  }
	  argument = one_argument(argument,name);
	  if (!isdigit(*name)) {
	    send_to_char("purge room start [end]\n\r",ch);
	    return;
	  }
	  range[0] = atol(name);
	  argument = one_argument(argument,name);
	  if (isdigit(*name))
	    range[1] = atol(name);
	  else
	    range[1] = range[0];

	  if (range[0]==0 || range[1]==0) {
	    send_to_char("usage: purge room start [end]\n\r", ch);
	    return;
	  }
	  room_iterate((room_iterate_func) purge_one_room, range);
	} else {
	  send_to_char("I don't see that here.\n\r", ch);
	  return;
	}
      }

      if(ch != vict)
	send_to_char("Ok.\n\r", ch);
    }  else {   /* no argument. clean out the room */
      if (TRUST(ch) < TRUST_DEMIGOD)
	return;
      if (IS_NPC(ch))	{
	send_to_char("You would only kill yourself..\n\r", ch);
	return;
      }

      if(cmd!=349)
        act("Thousands of tiny scrubbing bubbles fill the room!\n\r",
	    FALSE, ch, 0, 0, TO_ROOM);
      else
        act("$n purges the room.\n\r",
            FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world is cleaner.\n\r", ch->in_room);

      for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
	next_v = vict->next_in_room;
	if(!IS_PC(vict))
	  purge_char(vict);
      }

      for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
	next_o = obj->next_content;
	extract_obj(obj);
      }
    }
}

void purge_char(struct char_data* vict)
{
    int is_poly = IS_SET(vict->specials.mob_act, ACT_POLYSELF);

    if(vict->orig)
	do_return(vict, "", 0);

    /* if its a poly, do_return extracts the vict */
    if (is_poly)
	return;

    close_socket(vict->desc, TRUE);
    vict->desc = 0;

    extract_char(vict);
}

/* Obsolete */
/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
  send_to_char("reroll is broken.  Bug Stil to fix it.\n\r",ch);
}

void do_start(struct char_data *ch)
{
  int start_objs[] =
  {12,
   12,
   1,
   14,
   8,
   13,
   0};


  int *ptr, i;
  struct obj_data *obj;

  send_to_char("Welcome to Thieves World.  Enjoy the game...\n\r",ch);

	/* THIS IS BROKEN -- player_count++ is ALWAYS < 1  -- smw 2000/02/16 */
#ifdef JANDEBUG
  if(player_count++ < 1)
#endif
  if (0)
    {
      for(i = 0 ; i <= MAX_LEVEL_IND ; ++i)
	{
	  GET_LEVEL(ch,i) = MAX_MORT;
	  ch->player.trust = MAX_TRUST;
	  ch->player.godinfo.cmdset = GCMD_WIZSET;
	}
      UpdateMaxLevel(ch);
      UpdateMinLevel(ch);
    }
  else
    StartLevels(ch);

  UpdateMaxLevel(ch);
  UpdateMinLevel(ch);

  set_title(ch);

  GET_EXP(ch) = 0;

  /* Perhaps add some variety to Home Towns */
  ch->player.hometown = 1800;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TONGUE; i++)
    ch->player.talks[i] = 0;

  ch->points.max_hit  = 10;	/* These are BASE numbers   */
  ch->points.max_mana = 0;
  ch->points.max_move = GET_CON(ch) + number(1,20) - 9;

  ch->points.armor = 200;

  if (!ch->skills)
    SpaceForSkills(ch);

  for (i = 0; i <= MAX_SKILLS - 1; i++)	{
    if (TRUST(ch) < TRUST_IMP) {
      ch->skills[i].learned = 0;
      ch->skills[i].recognise = FALSE;
    } else {
      ch->skills[i].learned = 100;
      ch->skills[i].recognise = FALSE;
    }
  }

  AFF_FLAGS(ch) = 0;
  ch->specials.spells_to_learn = 0;

  ch->specials.flags = PLR_ECHO | PLR_WIMPY | PLR_AUTOEXIT;

  for (i = 0; i < 5; i++)
    ch->specials.apply_saving_throw[i] = 0;

  GET_COND(ch, DRUNK) = 0;
  GET_COND(ch, FULL) = (TRUST(ch) > TRUST_GOD ? -1 : 24);
  GET_COND(ch, THIRST) = (TRUST(ch) > TRUST_GOD ? -1 : 24);

  /* outfit char with valueless items  */
  for(ptr = start_objs ; *ptr ; ptr++)
    if((obj = make_object(*ptr, VIRTUAL)))
      obj_to_char(obj,ch);	/* bread   */

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  char_from_room(ch);
  char_to_room(ch, ch->player.hometown);
}


void do_advance(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH];
    char clss_name[MAX_INPUT_LENGTH];
    int newlevel, lin_class, clss;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, name);

    if (*name)	{
	if (!(victim = get_char_room_vis(ch, name)))		{
	    send_to_char("That player is not here.\n\r", ch);
	    return;
	}
    } else {
	send_to_char("Advance who?\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("NO! Not on NPC's.\n\r", ch);
	return;
    }

    argument = one_argument(argument, clss_name);

    if (!*clss_name) {
	send_to_char("Syntax is advance <name> <M|C|W|T|K|D|S|R|H|O|B> <level>\n\r", ch);
	return;
    }

    switch(*clss_name) {
    case 'M':
    case 'm':
	lin_class = MAGE_LEVEL_IND;
        clss=CLASS_MAGIC_USER;
	break;

    case 'T':
    case 't':
	lin_class = THIEF_LEVEL_IND;
        clss=CLASS_THIEF;
	break;

    case 'K':
    case 'k':
	lin_class = PALADIN_LEVEL_IND;
        clss=CLASS_PALADIN;
	break;

    case 'D':
    case 'd':
	lin_class = DRUID_LEVEL_IND;
        clss=CLASS_DRUID;
	break;

    case 'S':
    case 's':
	lin_class = PSI_LEVEL_IND;
        clss=CLASS_PSI;
	break;
    case 'O':
    case 'o':
	lin_class = MONK_LEVEL_IND;
        clss=CLASS_MONK;
	break;
    case 'B':
    case 'b':
	lin_class = BARD_LEVEL_IND;
        clss=CLASS_BARD;
	break;

    case 'R':
    case 'r':
	lin_class = RANGER_LEVEL_IND;
        clss=CLASS_RANGER;
	break;

    case 'W':
    case 'w':
	lin_class = WARRIOR_LEVEL_IND;
        clss=CLASS_WARRIOR;
	break;

    case 'C':
    case 'c':
	lin_class = CLERIC_LEVEL_IND;
        clss=CLASS_CLERIC;
	break;

    case 'H':
    case 'h':
        lin_class = SHIFTER_LEVEL_IND;
        clss=CLASS_SHIFTER;
        break;

    default:
	send_to_char("Syntax is advance <name> <M|C|W|T|P|D|S|R|H|O|B> <level>\n\r", ch);
	return;
	break;

    }

    if (!HasClass(victim, clss)) {
        send_to_char("This player does not have that class.\n\r", ch);
        return;
    }

    argument = one_argument(argument, level);

    if (!*level) {
	send_to_char("You must supply a level number.\n\r", ch);
	return;
    }

    if (!isdigit(*level)) {
        send_to_char("Level must be a positive integer.\n\r",ch);
        return;
    }

    newlevel=atoi(level);
    if (newlevel <= GET_LEVEL(victim, lin_class)) {
        send_to_char("Illegal advance level.\n\r", ch);
        return;
    }

    if (TRUST(ch) < TRUST_LORD) {
	send_to_char("Thou art not godly enough.\n\r", ch);
	return;
    }

    if (newlevel>ABS_MAX_LVL)	{
	send_to_char("Thats beyond is the highest possible level.\n\r", ch);
	return;
    }

    send_to_char("You feel generous.\n\r", ch);
    act("$n makes some strange gestures around $N.",FALSE,ch,0,victim,TO_ROOM);

    while(GET_LEVEL(victim, lin_class) < newlevel)
      advance_level(victim, lin_class);
    UpdateMaxLevel(victim);
    UpdateMinLevel(victim);
    send_to_char("Character is now advanced.\n\r", ch);
    if(!IS_NPC(victim))
    add_char_to_hero_list(victim);

}

void do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument,buf);
  if (!*buf)
    send_to_char("Whom do you wish to reroll?\n\r",ch);
  else
    if(!(victim = get_char(buf)) ||
      (!IS_NPC(victim) && (!CAN_SEE(ch,victim))))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else if (IS_NPC(victim))
           send_to_char("You can not reroll mobs.\n\r", ch);
         else {
               send_to_char("Rerolled...\n\r", ch);
               roll_abilities(victim);
         }
}

void do_remort(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[MAX_INPUT_LENGTH], newclass[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int j, ind, newclss;
    int ratio;

    if ((IS_NPC(ch)) || (TRUST(ch) < TRUST_LORD))
        return;

    argument = one_argument(argument, name);
    argument = one_argument(argument, newclass);

    if (*name)  {
        if (!(victim = get_char_room_vis(ch, name)))            {
            send_to_char("That player is not here.\n\r", ch);
            return;
        }
    } else {
        send_to_char("Reincarnate who?\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You can't do that on MOBs.\n\r", ch);
        return;
    }

    if(!IS_PURE_CLASS(victim)) {
	send_to_char("The char must be a pure class.\n\r", ch);
	return;
    }

    if (is_abbrev(newclass, "mage"))          { ind = MAGE_LEVEL_IND; newclss = CLASS_MAGIC_USER; }
    else if (is_abbrev(newclass, "cleric"))   { ind = CLERIC_LEVEL_IND; newclss = CLASS_CLERIC; }
    else if (is_abbrev(newclass, "warrior"))  { ind = WARRIOR_LEVEL_IND; newclss = CLASS_WARRIOR; }
    else if (is_abbrev(newclass, "thief"))    { ind = THIEF_LEVEL_IND; newclss = CLASS_THIEF; }
    else if (is_abbrev(newclass, "knight"))   { ind = PALADIN_LEVEL_IND; newclss = CLASS_PALADIN; }
    else if (is_abbrev(newclass, "druid"))    { ind = DRUID_LEVEL_IND; newclss = CLASS_DRUID; }
    else if (is_abbrev(newclass, "psionist")) { ind = PSI_LEVEL_IND; newclss = CLASS_PSI; }
    else if (is_abbrev(newclass, "ranger"))   { ind = RANGER_LEVEL_IND; newclss = CLASS_RANGER; }
    else if (is_abbrev(newclass, "shifter"))  { ind = SHIFTER_LEVEL_IND; newclss = CLASS_SHIFTER; }
    else if (is_abbrev(newclass, "monk"))     { ind = MONK_LEVEL_IND; newclss = CLASS_MONK; }
    else if (is_abbrev(newclass, "bard"))     { ind = BARD_LEVEL_IND; newclss = CLASS_BARD; }
    else {
	send_to_char("That's not an acceptible class\n\r", ch);
	return;
    }
    for (j = 0; j <= MAX_LEVEL_IND; j++)
    	if(GET_LEVEL(victim, j)==MAX_MORT) {
	    GET_LEVEL(victim, j) = 1;
	    GET_HIGH_LEVEL(victim, j) = 1;
	}
        else if(GET_LEVEL(victim, j)) {
	    sprintf(buf, "Target must be level %i, not %i\n\r", MAX_MORT, GET_LEVEL(victim, j));
	    send_to_char(buf, ch);
	    return;
	}

    victim->player.clss |= newclss;

    GET_LEVEL(victim, ind) = 1;
    GET_HIGH_LEVEL(victim, ind) = 1;

    UpdateMaxLevel(victim);
    UpdateMinLevel(victim);

    GET_EXP(victim) = 0;
    for (j=0;j<MAX_WEAR;j++)
	if(victim->equipment[j])
	    remove_equip(victim, j, FALSE);
    victim->specials.spells_to_learn = 0;

    ratio = 10;
    GET_HIT(victim) /= ratio;
    victim->points.max_hit /= ratio;

    ratio = 5;
    victim->points.max_mana /= ratio;
    victim->points.mana /= ratio;
    update_pos(victim);

    ratio = 2;
    victim->abilities.str /= ratio;
    victim->abilities.str_add = 1;
    victim->abilities.intel /= ratio;
    victim->abilities.wis /= ratio;
    victim->abilities.dex /= ratio;
    victim->abilities.con /= ratio;
    // Dont change charisma on remorting
    //    victim->abilities.cha /= ratio;

    sprintf(buf, "%s was just remorted by %s", GET_REAL_NAME(victim),  GET_REAL_NAME(ch));
    log_msg(buf, LOG_IMM);
    send_to_char_formatted("$CWYou find yourself suddenly weaker, yet with more potential.\n\r", victim);
    sprintf(buf, "$CW%s is suddenly filled with potential.\n\r", GET_REAL_NAME(victim));
    send_to_room_except_formatted(buf, victim->in_room, victim);
}

void do_restore(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[MAX_INPUT_LENGTH];
    int i;

    if (cmd == 0) return;

    only_argument(argument,buf);
    if (!*buf)
	send_to_char("Who do you wish to restore?\n\r",ch);
    else if(!(victim = get_char(buf)) || !CAN_SEE(ch, victim))
	send_to_char("No-one by that name in the world.\n\r",ch);
    else
    {
	GET_MANA(victim) = GET_MAX_MANA(victim);
	GET_HIT(victim) = GET_MAX_HIT(victim);
	GET_MOVE(victim) = GET_MAX_MOVE(victim);

	if (IS_PC(victim))
	{
	    if (!IS_GOD(victim)) {
		if (GET_COND(victim,THIRST) != -1)
		    GET_COND(victim,THIRST) = 24;
		if (GET_COND(victim,FULL) != -1)
		    GET_COND(victim,FULL) = 24;
	    } else {
		GET_COND(victim,THIRST) = -1;
		GET_COND(victim,FULL) = -1;
	    }

	    if (TRUST(victim) >= TRUST_CREATOR) {
		for (i = 0; i < MAX_SKILLS; i++) {
		    victim->skills[i].learned = 100;
		    victim->skills[i].recognise = TRUE;
		}

		if (TRUST(victim) >= TRUST_CREATOR) {
		    victim->abilities.str_add = 100;
		    victim->abilities.intel = 25;
		    victim->abilities.wis = 25;
		    victim->abilities.dex = 25;
		    victim->abilities.str = 25;
		    victim->abilities.con = 25;
		    victim->abilities.cha = 25;
		}
	    }
	}

	update_pos( victim );
	send_to_char("Done.\n\r", ch);
	act("You have been fully healed by $N!",
	    FALSE, victim, 0, ch, TO_CHAR);
    }
}


void do_noshout(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;

    if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) ||
	(!CAN_SEE(ch,vict)))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (TRUST(vict) >= TRUST(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.flags, PLR_NOSHOUT))
    {
	send_to_char("You can shout again.\n\r", vict);
	send_to_char("NOSHOUT removed.\n\r", ch);
	REMOVE_BIT(vict->specials.flags, PLR_NOSHOUT);
    }
    else
    {
	send_to_char("The gods take away your ability to shout!\n\r", vict);
	send_to_char("NOSHOUT set.\n\r", ch);
	SET_BIT(vict->specials.flags, PLR_NOSHOUT);
    }
}

void do_nohassle(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];

    only_argument(argument, buf);

    if (!*buf)
	if (IS_SET(ch->specials.flags, PLR_NOHASSLE))
	{
	    send_to_char("You can now be hassled again.\n\r", ch);
	    REMOVE_BIT(ch->specials.flags, PLR_NOHASSLE);
	}
	else
	{
	    send_to_char("From now on, you won't be hassled.\n\r", ch);
	    SET_BIT(ch->specials.flags, PLR_NOHASSLE);
	}
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (TRUST(vict) > TRUST(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.flags, PLR_NOHASSLE))
    {
	send_to_char("You can now be hassled again.\n\r", ch);
	REMOVE_BIT(vict->specials.flags, PLR_NOHASSLE);
    }
#if NOHASSLE_MORTALS
    else
    {
	send_to_char("From now on, you won't be hassled.\n\r", ch);
	SET_BIT(vict->specials.flags, PLR_NOHASSLE);
    }
#else
    else
	send_to_char("The implementor won't let you set this on mortals...\n\r",ch);
#endif
}

void do_stealth(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.flags, PLR_STEALTH))
      {
	send_to_char("STEALTH mode OFF.\n\r", ch);
	REMOVE_BIT(ch->specials.flags, PLR_STEALTH);
      }
    else
      {
	send_to_char("STEALTH mode ON.\n\r", ch);
        if (TRUST(ch) > TRUST_LORD) {
          send_to_char("This will also make your goto's invisible to other Gods since you are an IMP,\n\r", ch);
          send_to_char("only if you are VIS 59.\n\r", ch);
        }
        SET_BIT(ch->specials.flags, PLR_STEALTH);
      }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (TRUST(vict) > TRUST(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.flags, PLR_STEALTH))
  {
      send_to_char("STEALTH mode OFF.\n\r", ch);
      REMOVE_BIT(vict->specials.flags, PLR_STEALTH);
  }
#if STEALTH_MORTAL
  else
  {
      send_to_char("STEALTH mode ON.\n\r", ch);
      SET_BIT(vict->specials.flags, PLR_STEALTH);
  }
#else
  else
    send_to_char("The implementor won't let you set this on mortals...\n\r",ch);
#endif
}

static void print_room(room_num rnum, struct room_data *rp, struct string_block *sb)
{
  char	buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ((rp->sector_type < 0) || (rp->sector_type > 9)) { /* non-optimal */
    rp->sector_type = 0;
  }
  sprintf(buf, "%5ld %4ld %-12s %s", rp->number, rnum,
	  sector_types[(int)rp->sector_type], (rp->name?rp->name:"Empty"));
  strcat(buf, " [");

  sprintbit(rp->room_flags, room_bits, buf2);
  strcat(buf, buf2);
  strcat(buf, "]\n\r");

  append_to_string_block(sb, buf);
}

static void print_death_room(room_num rnum, struct room_data *rp, void *sb)
{
  if (rp && rp->room_flags&DEATH)
    print_room(rnum, rp, (struct string_block*) sb);
}

static void print_nomagic_room(room_num rnum, struct room_data *rp, void *sb)
{
  if (rp && rp->room_flags&NO_MAGIC)
    print_room(rnum, rp, (struct string_block*) sb);
}

struct show_room_zone_struct {
  room_num	blank;
  room_num	startblank, lastblank;
  room_num	bottom, top;
  struct string_block	*sb;
};

static void show_room_zone(room_num rnum, struct room_data *rp, void* data)
{
  struct show_room_zone_struct *srzs = (struct show_room_zone_struct*) data;

  char buf[MAX_STRING_LENGTH];

  if (!rp || rp->number<srzs->bottom || rp->number>srzs->top)
    return; /* optimize later*/

  if (srzs->blank && (srzs->lastblank+1 != rp->number) ) {
    sprintf(buf, "rooms %ld-%ld are blank\n\r", srzs->startblank,
	    srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  if ( (rp->name) && 1==sscanf(rp->name, "%ld", &srzs->lastblank) &&
      srzs->lastblank==rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %ld-%ld are blank\n\r", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }

  print_room(rnum, rp, srzs->sb);
}

void do_show(struct char_data *ch, char *argument, int cmd)
{
  int	zone;
  char buf[MAX_STRING_LENGTH], zonenum[MAX_INPUT_LENGTH];
  struct index_data	*which_i = NULL;
  room_num bottom = 0, top = 0;
  int topi = 0;
  struct string_block	sb;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, buf);

  init_string_block(&sb);

  if (is_abbrev(buf, "zones")) {
    struct zone_data	*zd;

    if (TRUST(ch) <  TRUST_DEMIGOD) {
      send_to_char("You cannot show zones.\n\r", ch);
      return;
    }
    append_to_string_block(&sb, "# Zone   name                   lifespan age     rooms     reset    swapped\n\r");

    for(zone=0; zone<=top_of_zone_table; zone++) {
      char	*mode;
#ifdef SWAP_ZONES
      char *mode2;
#endif

      zd = zone_table+zone;
      switch(zd->reset_mode) {
      case 0: mode = "never";      break;
      case 1: mode = "ifempty";    break;
      case 2: mode = "always";     break;
      default: mode = "!unknown!"; break;
      }

#ifdef SWAP_ZONES
      switch(zd->swapped)
      {
      case 0:  mode2 = ""; break;
      case 1:  mode2 = "swapped"; break;
      default: mode2 = "!unknown!"; break;
      }

      sprintf(buf,"%4d %-27s %4dm %4dm %6ld-%-6ld %-8s %s\n\r", zone,
	      zd->name, zd->lifespan, zd->age, bottom, zd->top, mode, mode2);
#else
      sprintf(buf,"%4d %-40s %4dm %4dm %6ld-%-6ld %s\n\r", zone,
	      zd->name, zd->lifespan, zd->age, bottom, zd->top, mode);
#endif

      append_to_string_block(&sb, buf);
      bottom = zd->top+1;
    }


  } else if ((is_abbrev(buf, "objects") &&
	     (which_i=obj_index,topi=top_of_objt)) ||
	     (is_abbrev(buf, "mobiles") &&
	     (which_i=mob_index,topi=top_of_mobt))) {
    int		objn;
    struct index_data	*oi;

    if ((is_abbrev(buf, "objects")) && (TRUST(ch) < TRUST_LRGOD)) {
      send_to_char ("You cannot show objects (yet).\n\r",ch);
      return;
    }

    only_argument(argument, zonenum);
    zone = -1;
    if (1==sscanf(zonenum,"%i", &zone) &&
	( zone<0 || zone>top_of_zone_table )) {
      append_to_string_block(&sb, "That is not a valid zone_number\n\r");
      return;
    }
    if (zone>=0) {
      bottom = zone ? (zone_table[zone-1].top+1) : 0;
      top = zone_table[zone].top;
    }

    append_to_string_block(&sb, "VNUM  rnum count limit names\n\r");
    for (objn=0; objn<=topi; objn++) {
      oi = which_i + objn;

      if ((zone>=0 && (oi->virt<bottom || oi->virt>top)) ||
	  (zone<0 && !isname(zonenum, oi->name)))
	continue; /* optimize later*/

      sprintf(buf,"%5d %4d %4d  %4d  %s\n\r", oi->virt, objn,
	      oi->number, oi->limit, oi->name);
      append_to_string_block(&sb, buf);
    }


  } else if (is_abbrev(buf, "rooms")) {

    if (TRUST(ch) < TRUST_LRGOD) {
      send_to_char("You cannot show rooms (yet).\n\r",ch);
      return;
    }

    only_argument(argument, zonenum);

    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n\r");
    if (is_abbrev(zonenum, "death")) {
      room_iterate(print_death_room, &sb);
    } else if (is_abbrev(zonenum, "nomagic")) {
      room_iterate(print_nomagic_room, &sb);
    } else if (1!=sscanf(zonenum,"%i", &zone) ||
	       zone<0 || zone>top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n\r");


    } else {
      struct show_room_zone_struct	srzs;

      srzs.bottom = zone ? (zone_table[zone-1].top+1) : 0;
      srzs.top = zone_table[zone].top;

      srzs.blank = 0;
      srzs.sb = &sb;
      room_iterate(show_room_zone, &srzs);

      if (srzs.blank){
	sprintf(buf, "rooms %ld-%ld are blank\n\r", srzs.startblank,
		srzs.lastblank);
	append_to_string_block(&sb, buf);
	srzs.blank = 0;
	}
	  }
  } else {
    append_to_string_block(&sb,"Usage:\n\r"
		 "  show zones\n\r"
		 "  show (objects|mobiles) (zone#|name)\n\r"
		 "  show rooms (zone#|death|private)\n\r");
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}


void do_debug(struct char_data *ch, char *argument, int cmd)
{
    char	arg[MAX_INPUT_LENGTH];
    int	i;
#ifndef DONT_USE_mallinfo
    char	buf[256];
    struct mallinfo	mi;
#endif

    i=0;
    argument = one_argument(argument, arg);
    i = atoi(arg);

    if(*arg)
    {
	switch(atoi(arg))
	{
	case 1:
	    break;

	case 2:
#ifndef DONT_USE_mallinfo
	    mi = mallinfo();

	    sprintf(buf, "arena:        %x\n\r", mi.arena);
	    send_to_char(buf, ch);
	    sprintf(buf, "ordblks:      %x\n\r", mi.ordblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "smblks:       %x\n\r", mi.smblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "hblks:        %x\n\r", mi.hblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "hblkhd:       %x\n\r", mi.hblkhd);
	    send_to_char(buf, ch);
	    sprintf(buf, "usmblks:      %x\n\r", mi.usmblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "fsmblks:      %x\n\r", mi.fsmblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "uordblks:     %x\n\r", mi.uordblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "fordblks:     %x\n\r", mi.fordblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "keepcost:     %x\n\r", mi.keepcost);
	    send_to_char(buf, ch);
	    sprintf(buf, "mxfast:       %x\n\r", mi.mxfast);
	    send_to_char(buf, ch);
	    sprintf(buf, "nlblks:       %x\n\r", mi.nlblks);
	    send_to_char(buf, ch);
	    sprintf(buf, "grain:        %x\n\r", mi.grain);
	    send_to_char(buf, ch);
	    sprintf(buf, "uordbytes:    %x\n\r", mi.uordbytes);
	    send_to_char(buf, ch);
	    sprintf(buf, "allocated:    %x\n\r", mi.allocated);
	    send_to_char(buf, ch);
	    sprintf(buf, "treeoverhead: %x\n\r", mi.treeoverhead);
	    send_to_char(buf, ch);
#endif
	    break;
	}
    }
}

void do_home(struct char_data *ch, char *arg, int cmd)
{
    int num;
    char buf[MAX_INPUT_LENGTH];

    one_argument(arg, buf);

    if (IS_NPC(ch)) return;

    if (*arg) {
        num = atoi(buf);
        if (!is_number(buf) && !strcmp(buf, "here")) {
	    GET_HOME(ch) = num;
        } else if (is_number(buf)) {
	    if (NULL==real_roomp(num)) {
		send_to_char("No room exists with that number.\n\r", ch);
		return;
	    } else {
		GET_HOME(ch) = num;
	    }
	}
    }

    sprintf(buf, "Home is: %d\n\r", GET_HOME(ch));
    send_to_char(buf, ch);
}

void do_invis(struct char_data *ch, char *argument, int cmd)
{
  char	buf[MAX_INPUT_LENGTH];
  int	level;

  if (cmd == 242 && !IS_IMMORTAL(ch)) return;

  if (cmd != 242) {
    if (affected_by_spell(ch, SPELL_INVISIBLE)) {
      affect_from_char(ch, SPELL_INVISIBLE);
    }
    REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE);
    ch->invis_level = 0;
    return;
  }

  if (scan_number(argument, &level)) {
    if (level<0)
	level=0;
    else if (level>TRUST(ch))
	level = TRUST(ch);
    ch->invis_level = level;
    sprintf(buf,"Invis level set to %d.\n\r", level);
    send_to_char(buf, ch);
  } else {
    if (ch->invis_level>0) {
      ch->invis_level = 0;
      send_to_char("You are now totally visible.\n\r",ch);
    } else {
      ch->invis_level = TRUST(ch);
      send_to_char("You are now invisible to everyone below your level.\n\r",ch);
    }
  }
}

void do_create( struct char_data *ch, char *argument, int cmd)
{
  int i, count, start, end;

  if (!IS_IMMORTAL(ch) || IS_NPC(ch)) {
    return;
  }


  count = sscanf(argument, "%d %d", &start, &end);
  if (count < 2) {
    send_to_char(" create <start> <end>\n\r", ch);
    return;
  }
  if (start > end) {
    send_to_char(" create <start> <end>\n\r", ch);
    return;
  }

  send_to_char("You form much order out of Chaos\n\r", ch);
  for (i = start; i<= end; i++) {
    if (!real_roomp(i))
    CreateOneRoom(i);
  }

}


void CreateOneRoom( room_num loc_nr)
{
  struct room_data *rp;
  int zone;
  char buf[256];

  rp = room_find_or_create(loc_nr);
  bzero(rp, sizeof(*rp));

  rp->number = loc_nr;
  if (top_of_zone_table >= 0) {

    for (zone=0;
	 rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	 zone++);
    if (zone > top_of_zone_table) {
      fprintf(stderr,
	      "Room %ld is outside of any zone.\n", rp->number);
      zone--;
    }
    rp->zone = zone;
  }
  sprintf(buf, "%ld", loc_nr);
  rp->name = (char *)strdup(buf);
  rp->description = (char *)strdup("Empty\n");
}

#if USE_profile
void do_profile( struct char_data *ch, char *argument, int cmd)
{
    static int isProfiling = 0;
    char buf[256];

    while(isspace(*argument))
	argument++;

    if(!strcmp(argument, "on"))
    {
	if(isProfiling)
	{
	    send_to_char("Profiling is already on.\n\r", ch);
	    return;
	}
	else
	{
	    moncontrol(1);
	    isProfiling = 1;
	}
    }
    else if(!strcmp(argument, "off"))
    {
	if(!isProfiling)
	{
	    send_to_char("Profiling isn't on.\n\r", ch);
	    return;
	}
	else
	{
	    moncontrol(0);
	    isProfiling = 0;
	}
    }
    else if(*argument)
    {
	send_to_char("profile [off|on]\n\r", ch);
	return;
    }

    sprintf(buf, "profiling is now %s\n\r", isProfiling ? "on" : "off");
    send_to_char(buf, ch);
}
#endif

typedef struct
{
    char* cmd;
    int   bits_off;
    int   bits_on;
    char* mesg;
} flag_table;

flag_table* find_flag(const char* str, flag_table* table)
{
    while(table->cmd)
    {
	if(is_abbrev(str, table->cmd))
	    return table;
	table++;
    }

    return NULL;
}

void do_judge( struct char_data *ch, char *argument, int cmd)
{
    char	buf[256];
    struct	char_data* victim;
    flag_table	judge_table[] =
    {
	{ "killer",	0,		PLR_PKILLER,	"a killer" },
	{ "nokiller",	PLR_PKILLER,	0,		"not a killer" },
	{ "loser",	0,		PLR_LOSER,	"open season" },
	{ "noloser",	PLR_LOSER,	0,		"not open season" },
	{ "thief",	0,		PLR_THIEF,	"a thief" },
	{ "nothief",	PLR_THIEF,	0,		"not a thief" },
	{ "innocent",	PLR_THIEF | PLR_PKILLER | PLR_LOSER,
	      				0,		"innocent" },
	{ NULL }
    };
    flag_table	*the_flag;

    argument = one_argument(argument, buf);

    if(!*buf)
    {
	send_to_char("Who do you wish to judge?\n\r", ch);
	return;
    }

    if((victim = get_char(buf)) == NULL)
    {
	send_to_char("Can't find that person.\n\r", ch);
	return;
    }

    if(!IS_PC(victim))
    {
	send_to_char("That's just a foolish mob!\n\r", ch);
	return;
    }

    if(TRUST(victim) > TRUST(ch))
    {
	send_to_char("They appeal to a higher court, themselves...\n\r", ch);
	return;
    }

    one_argument(argument, buf);

    if(!*buf)
    {
	int guilty = 0;

	if(IS_SET(victim->specials.flags, PLR_LOSER))
	{
	    send_to_char("It's open season on them.\n\r", ch);
	    guilty++;
	}
	if(IS_SET(victim->specials.flags, PLR_PKILLER))
	{
	    send_to_char("That person is a killer.\n\r", ch);
	    guilty++;
	}
	if(IS_SET(victim->specials.flags, PLR_THIEF))
	{
	    send_to_char("That person is a thief.\n\r", ch);
	    guilty++;
	}
	if(!guilty)
	    send_to_char("That person is innocent.\n\r", ch);
    }
    else if(!(the_flag = find_flag(buf, judge_table)))
    {
	send_to_char("Judge them how?\n", ch);
    }
    else
    {
	victim->specials.flags &= ~the_flag->bits_off;
	victim->specials.flags |= the_flag->bits_on;

	sprintf(buf, "You judge $N %s", the_flag->mesg);
	act(buf, FALSE, ch, NULL, victim, TO_CHAR);
	sprintf(buf, "$n has judged you %s", the_flag->mesg);
	act(buf, FALSE, ch, NULL, victim, TO_VICT);
	sprintf(buf, "$n has judged $N %s", the_flag->mesg);
	act(buf, FALSE, ch, NULL, victim, TO_WORLD_NOTVICT);
    }
}

void do_allow(struct char_data *ch, char *argument, int cmd)
{
    flag_table allow_table[] =
    {
	{ "build",	0,		PLR_BUILDER,	"a builder"	},
	{ "nobuild",	PLR_BUILDER,	0,		"not a builder" },
	{ "sitelock",   0,	        PLR_SITE_OK,    "able to login from wizlock sites" },
	{ "nositelock",   PLR_SITE_OK,    0,   "not able to login from wizlock sites" },


	{ NULL }
    };
    flag_table* the_flag;
    char buf[256];
    struct char_data* victim;

    argument = one_argument(argument, buf);

    if(!*buf)
    {
	send_to_char("Who do you wish to allow?\n\r", ch);
	return;
    }

    if((victim = get_char(buf)) == NULL)
    {
	send_to_char("Can't find that person.\n\r", ch);
	return;
    }

    if(!IS_PC(victim))
    {
	send_to_char("That's just a foolish mob!\n\r", ch);
	return;
    }

    one_argument(argument, buf);

    if(!*buf || !(the_flag = find_flag(buf, allow_table)))
    {
	send_to_char("Allow them to what?  build, nobuild, site, nosite\n\r", ch);
	return;
    }

    victim->specials.flags &= ~the_flag->bits_off;
    victim->specials.flags |= the_flag->bits_on;

    sprintf(buf, "You have made them %s\n\r", the_flag->mesg);
    send_to_char(buf, ch);

    sprintf(buf, "You are now %s\n\r", the_flag->mesg);
    send_to_char(buf, victim);
    command_interpreter(victim, "save", 1);
}

void do_logall(struct char_data* ch, char* arg, int mesg)
{
    char		buf[256];
    struct char_data*	vict;

    one_argument(arg, buf);

    if(!*buf)
    {
	send_to_char("logall who?\n\r", ch);
	return;
    }

    if(!(vict = get_char(buf)))
    {
	send_to_char("can't find 'em\n\r", ch);
	return;
    }

    if(IS_SET(vict->specials.flags, PLR_LOGALL))
    {
	REMOVE_BIT(vict->specials.flags, PLR_LOGALL);
	send_to_char("They won't be logged anymore.\n\r", ch);
    }
    else
    {
	SET_BIT(vict->specials.flags, PLR_LOGALL);
	send_to_char("They will be logged now.\n\r", ch);
    }
}

/* Scatter command by Melkor */
/* currently only accepts one name to scatter, or default of all */
void do_scatter(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj, *nextobj;
    struct char_data *mob, *nextmob;
    struct room_data *roomp, *targ_roomp;
    int room, targ_room;
    char *argpt;
    char buf[150];

	/* notes: room flags : DEATH, NO_MOB, NO_TRAVEL_OUT,
	 * NO_TRAVEL_IN, IMMORT_RM, GOD_RM, NO_RECALL
	 */

    argpt=argument;
    while(*argpt==' ') argpt++;

    /* Find room player is in */
    room=ch->in_room;
    if(!(roomp=real_roomp(room))) {
	send_to_char("You have apparently fallen off the mud!\r\n"
		"This is a serious bug *cringe*\r\n", ch);
	return;
    }

    /* get first object in room */
    obj=roomp->contents;
    while(obj!=NULL) {
	nextobj=obj->next_content;
	if(!(*argpt) || isname(argpt,OBJ_NAME(obj))) {
	    do {  /* Find target room */
		targ_room=randomnum(top_of_world);
		targ_roomp=real_roomp(targ_room);
	    } while ( !targ_room || !targ_roomp ||
		IS_SET(targ_roomp->room_flags, (DEATH |
		IMMORT_RM | GOD_RM
		|  BRUJAH_RM)) );

	    sprintf(buf, "A crack opens up in the mud and %s falls through.\r\n",
		OBJ_SHORT(obj) );
	    send_to_room(buf, room);
	    obj_from_room(obj);
	    obj_to_room(obj,targ_room);
	    send_to_room(buf, targ_room);
	}
	obj=nextobj;
    }
    /* get first mob in room */
    mob=roomp->people;
    while(mob!=NULL) {
	nextmob=mob->next_in_room;
	/* Only scatter PC's by name */
	if( (!(*argpt) && !IS_PC(mob) ) || isname(argpt,GET_REAL_NAME(mob))) {
	    do {  /* Find target room */
		targ_room=randomnum(top_of_world);
		targ_roomp=real_roomp(targ_room);
	    } while ( !targ_room || !targ_roomp ||
		IS_SET(targ_roomp->room_flags, (DEATH | IMMORT_RM | GOD_RM)) );

	    sprintf(buf, "A crack opens up in the mud and %s falls through.\r\n",
		GET_NAME(mob) );
	    send_to_room(buf, room);
	    char_from_room(mob);
	    char_to_room(mob,targ_room);
	    send_to_room(buf, targ_room);
	}
	mob=nextmob;
    }

}


/*PAC*/
void do_swapzone(struct char_data * ch, char * arg, int cmd)
{
  int zone;
  char buf[MAX_INPUT_LENGTH];

  one_argument(arg, buf);

  /* check for proper sysntax */
  if ( (!*arg) || (!is_number(buf)) )
  {
    send_to_char("Systax: swap [zone_num]\n\r", ch);
    return;
  }

  /* extract the argument */
  sscanf(buf,"%d",&zone);

  /* range check on zone number */
  if ( (zone < 0) || (zone > top_of_zone_table) )
  {
    send_to_char("Zone nubmer out of range\n\r", ch);
    return;
  }

  if (!zone_can_swap(zone))
  {
    send_to_char("For one reason or another, "
		 "I cannot swap that zone right now.\n\r", ch);
    return;
  }

  swap_zone(zone);
}

void do_absolve(struct char_data * ch, char * arg, int cmd) {

  struct char_data *victim;
  struct obj_data  *obj;
  struct obj_data  *n;
  int j;

  if ((victim = get_char_room_vis(ch, arg)) == NULL)
    act("I don't see anyone here like that.", TRUE, ch, 0, 0, TO_CHAR);
  else {

    for (j=0; j<MAX_WEAR; j++) {
      if (victim->equipment[j]) {
	obj = victim->equipment[j];
	if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
	  REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
	  act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_ROOM);
	}
      }
    }

    for (obj=victim->carrying; obj; obj=n) {
      n=obj->next_content;

      if (obj) {
	if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
	  REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
	  act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$p briefly glows blue for a moment.", FALSE, ch, obj, 0, TO_ROOM);
	}
      }
    }
  }
}


void do_dup(struct char_data *ch, char *arg, int cmd)
{

  struct char_data *mob;
  struct char_data *clone;
  int i,mobnumber = -1;
  char mobname[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  int number,count;

  half_chop(arg, mobname, arg1);
  if (*arg1 == '\0')
    number = 1;
  else if (isdigit(*arg1))
    number = atoi(arg1);
  else
    number = 1;

  if(!*arg)
  {
     send_to_char("Syntax: dup <mob> <number of copies>\n", ch);
     return;
  }
  count = 1;
  number = MIN(number, 50);
  if ((mob = get_char_vis_world(ch, mobname, &count)))
  {
    /* mobile in world */
    if (IS_PC(mob) || IS_POLY_PC(mob) || (!CAN_SEE(ch,mob))) {
      send_to_char("No mobile by that name in the world\n\r", ch);
      return;
    }
    mobnumber = real_mobile(mob_index[mob->nr].virt);

    if ( mobnumber<0 || mobnumber>top_of_mobt) {
	    send_to_char("There is no such monster.\n\r", ch);
	    return;
    }
    /* make as many copies as needed */
    for(i = 0; i < number; i++) {
      if(!(clone = make_mobile(mobnumber, REAL)))
      {
        send_to_char("Internal Errors Occured\n\r", ch);
        return;
      }
      char_to_room(clone, ch->in_room);

      /* start copying the info over */
      clone->player.name = ss_share(mob->player.name);
      clone->player.short_descr = ss_share(mob->player.short_descr);
      clone->player.long_descr = ss_share(mob->player.long_descr);
      clone->player.description = ss_share(mob->player.description);
      clone->player.title = 0;

      for(int j=0; j<=MAX_LEVEL_IND; j++)
         clone->player.level[i] = mob->player.level[i];

        clone->player.maxlevel = mob->player.maxlevel;
       clone->player.minlevel = mob->player.minlevel;


       clone->points.hitroll = mob->points.hitroll;
       clone->points.armor = mob->points.armor;
       clone->points.max_hit =  mob->points.max_hit;
       clone->points.hit =  mob->points.hit;
       clone->specials.damnodice = mob->specials.damnodice;
       clone->specials.damsizedice =  mob->specials.damsizedice;
       clone->points.damroll =  mob->points.damroll;

       clone->points.mana     = mob->points.mana;
       clone->points.max_mana = mob->points.max_mana;
       clone->points.move     = mob->points.move;
       clone->points.max_move = mob->points.max_move;

       clone->points.gold = mob->points.gold;
       GET_EXP(clone) = GET_EXP(mob);

     }
     send_to_char("Copies made.\n\r",ch);

  }
  else
  {
     send_to_char("Can't find a mob by that name in the world\n\r", ch);
  }

}


ACMD(do_wizset) {

  struct char_data *vict = NULL;
  int x;
  char buf[MAX_STRING_LENGTH];
  char name[20],field[20],pname[20];
  longlong max=0;
  bool valive = false;

  if (TRUST(ch) < MAX_TRUST-1) {
    sprintf(buf, "%s attempted to use a wizset command. The attempt is however laughable", GET_NAME(ch));
    send_to_char("Pft!", ch);
    log_msg(buf);
    return;
  }

  struct set_struct* c;
  struct pos_struct* p;

  if(!*arg) {
    do_wizset(ch,"help",666);
    return;
  } else {
    arg = one_argument(arg,field);
  }

  if (*arg) {
    arg = one_argument(arg,name);
    if (!(vict = get_char_vis(ch, name))) {
      do_wizset(ch,"help",666);
      return;
    } else {
      valive = true;
    }
  }
  if (is_abbrev(field,"help")) {
    send_to_char("wizset <command> <name>\n\r",ch);
    send_to_char("wizset commands -- List of all commands.\n\r",ch);
    send_to_char("wizset all <name> -- Set all available commands.\n\r",ch);
    send_to_char("wizset init <name> <position> -- Sets default values.\n\r",ch);
    send_to_char("wizset position <name> <position> -- Sets or removes the postion.\n\r",ch);
    return;
  } else if (is_abbrev(field,"commands")) {
    send_to_char("The following commands can be set:\n\r",ch);
    *buf = '\0';
    for(x=1,c=gcmd_list;c->cmd; c++) {
      sprintf(buf + strlen(buf),"%-12s",c->cmd);
      if (x++ == 6) {
	strcat(buf,"\n\r");
	x=1;
      }
    }
    strcat(buf,"\n\r");
    page_string(ch->desc,buf,1);
  } else if (is_abbrev(field,"all")) {
    for(c=gcmd_list;c->cmd; c++) {
      max = max + c->bit;
    }
    if (!valive) {
      send_to_char("You need a player name.\n\r",ch);
      return;
    }
    vict->player.godinfo.cmdset = max;
    send_to_char("All commands set on.\n\r",ch);
    return;
  } else if (is_abbrev(field,"off")) {
    if (!valive) {
      send_to_char("You need a player name.\n\r",ch);
      return;
    }
    vict->player.godinfo.cmdset = 0;
    send_to_char("All commands set off.\n\r",ch);
    return;
  } else if (is_abbrev(field,"init")) {
    if (!*arg || !valive) {
      send_to_char("Please specify one of the following positions:\n\r",ch);
      *buf = '\0';
      for(x=1,p=pos_list;p->name; p++) {
	sprintf(buf + strlen(buf),"%-20s",p->name);
	if (x++ == 3) {
	  strcat(buf,"\n\r");
	  x=1;
	}
      }
      strcat(buf,"\n\r");
      page_string(ch->desc,buf,1);
    } else {
      arg = one_argument(arg,pname);
      for (p=pos_list;p->name;p++) {
	if(is_abbrev(pname,p->name)) {
	  vict->player.trust = p->deftrust;
	  vict->player.godinfo.position = p->posbit;
	  if (p->defbit==ALL_GCMD) {
	    for(c=gcmd_list;c->cmd; c++) {
	      max = max + c->bit;
	    }
	    vict->player.godinfo.cmdset = max;
	    send_to_char("All commands set on.\n\r",ch);
	  } else {
	    vict->player.godinfo.cmdset = p->defbit;
	  }
	  sprintf(buf,"%s is now a %s with default god commands.\n\r",
		  ss_data(vict->player.name),
		  p->name);
	  send_to_char(buf,ch);
	  act("You eat Ambrosia that $Cg$n$CB has given you!",FALSE,ch,0,vict,TO_VICT);
          do_score(vict,"",15);
	  send_to_char("You restore them.\n\r",ch);
	  do_restore(vict,"",0);
	  send_to_char("You make them save.\n\r",ch);
	  do_save(vict,"",0);
	  return;
	}
      }
      do_wizset(ch,"position",666);
      return;
    }
  } else if (is_abbrev(field,"position")) {
    if (!*arg || !valive) {
      send_to_char("Please specify one of the following positions:\n\r",ch);
      *buf = '\0';
      for(x=1,p=pos_list;p->name; p++) {
	sprintf(buf + strlen(buf),"%-20s",p->name);
	if (x++ == 3) {
	  strcat(buf,"\n\r");
	  x=1;
	}
      }
      strcat(buf,"\n\r");
      page_string(ch->desc,buf,1);
    } else {
      arg = one_argument(arg,pname);
      for (p=pos_list;p->name;p++) {
	if(is_abbrev(pname,p->name)) {
	  if (IS_SET(vict->player.godinfo.position,p->posbit)) {
	    REMOVE_BIT(vict->player.godinfo.position,p->posbit);
	    sprintf(buf,"%s is no longer a %s.\n\r",
		    ss_data(vict->player.name),
		    p->name);
	    send_to_char(buf,ch);
	    return;
	  } else {
	    SET_BIT(vict->player.godinfo.position,p->posbit);
	    sprintf(buf,"%s is now a %s.\n\r",
		    ss_data(vict->player.name),
		    p->name);
	    send_to_char(buf,ch);
	    return;
	  }
	  return;
	}
      }
      do_wizset(ch,"position",666);
      return;
    }
  } else {
    if(!valive) {
      send_to_char("Get a player.\n\r",ch);
      return;
    }
    for(c=gcmd_list;c->cmd; c++) {
      if (is_abbrev(field,c->cmd)) {
	if (HAS_GCMD(vict,c->bit)) {
	  REMOVE_BIT(GCMD_FLAGS(vict),c->bit);
	  sprintf(buf,"%s removed.\n\r",c->cmd);
	  send_to_char(buf,ch);
	  return;
	} else {
	  SET_BIT(GCMD_FLAGS(vict),c->bit);
	  sprintf(buf,"%s added.\n\r",c->cmd);
	  send_to_char(buf,ch);
	  return;
	}

      }
    }
    do_wizset(ch,"commands",666);
  }

}



ACMD(do_wizcomm) {
  char buf[MAX_STRING_LENGTH];
  int no,x;
  struct char_data *vict = NULL;
  char name[MAX_INPUT_LENGTH];
  struct command_info* c;
  struct pos_struct* p;
  struct set_struct* s;

  if (IS_NPC(ch))
    return;

  if(!*arg) {
    do_wizcomm(ch,"help",666);
    return;
  } else {
    arg = one_argument(arg,name);
  }

  if (is_abbrev(name,"help")) {
    send_to_char("wizcomm <player>\n\r",ch);
    return;
  } else if (is_abbrev(name,"list")) {
    do_wizcomm(ch,"help",666);
    return;
  } else {
   if (!(vict = get_char_vis(ch, name))) {
    do_wizcomm(ch,"help",666);
    return;
   }
    sprintf(buf,"$Cy%s$CN is a",GET_NAME(vict));
    for(p=pos_list;p->name;p++) {
      if(IS_SET(vict->player.godinfo.position,p->posbit)) {
	sprintf(buf + strlen(buf)," %s",p->name);
      }
    }
    sprintf(buf + strlen(buf),".\n\r");
    send_to_char_formatted(buf,ch);
    sprintf(buf,"$Cr%s has the following commands:$CN\n\r",GET_NAME(vict));
    send_to_char_formatted(buf, ch);

    *buf = '\0';

    for(no = 1, c = cmd_list ; c->name ; c++)
      if (HAS_GCMD(vict,c->min_trust))
	{
	  sprintf(buf + strlen(buf), "%-12s",c->name);
	  if (!(no++ % 6))
	    strcat(buf, "\n\r");
	}
    sprintf(buf + strlen(buf),"\n\r\n\rAnd has the following bits set...\n\r");
    for(x=1,s=gcmd_list;s->cmd; s++) {
      if (HAS_GCMD(vict,s->bit)) {
	sprintf(buf + strlen(buf),"%-12s",s->cmd);
	x++;
      }
      if (x == 6) {
	strcat(buf,"\n\r");
	x=1;
      }
    }

    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
  }
}

//generate the numbers for a mob of the given level
int generate_mob_by_level(struct char_data *target, int level)
{
    int tempNum, i = 0, origLevel = 0;
    double MHps, SD;

    origLevel = GetMaxLevel(target);
    level = MIN(250, level);
    level = MAX(0, level);

    for(i = 0; i<MAX_LEVEL_IND;i++)
    {
       if(target->player.level[i] > 0)
         target->player.level[i] = level;
    }
    UpdateMaxLevel(target);
    UpdateMinLevel(target);

    /*Ok. I am trying a new equation. This is basicly a standard
      normal curve, with a mean of 250, and a standard deviation
      of SD. I also centered it so that a level 1 has 10 hps,
      and the max hp anything can get is MHps. (Quilan2)

      BTW...the equation is e^(-((Mean-X)/StdDev)^2/2) */

    SD = 100;
    MHps = 25000;
    tempNum = 10;
    tempNum -= (int)((double)MHps*exp(-(249/SD)*(249/SD)/2));
    tempNum += (int)((double)MHps*exp(-((250-level)/SD)*((250-level)/SD)/2));

    //tempNum = (int)((double)(17000 * sin(3.14 * level/500) + 5));
    target->points.hit = tempNum;
    target->points.max_hit = tempNum;
    target->specials.damnodice = (int)((double)(20*sin(3.14 * level/500) +2));
    target->specials.damsizedice = target->specials.damnodice;
    target->points.hitroll = MIN(127, level/2);
    target->points.damroll = target->points.hitroll;
    target->points.mana = MIN(32000, (int)((double)(2500 * sin(3.14 * level/500))));
    target->points.mana = MAX(target->points.mana, 0);
    target->points.max_mana = target->points.mana;
    target->points.move = target->points.mana;
    target->points.max_move = target->points.mana;
    target->points.gold = level * 1500;
    GET_EXP(target) = balance_exp(target);
    target->mult_att = (int)(level / 10) + 1;
    GET_AC(target) = ((GetMaxLevel(target) * -1) + 20) * 10;
    random_mob_points(target);
    return 1;

}

/*void do_builder_stat(struct char_data *ch, char *argument, int cmd)
{
   char_data *target;
   char buf[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH];
   int i = 0, count=1;
   ch->olc.builder_range(ch);
   ch->olc.save_status(ch);
}
*/
void do_questup(struct char_data *ch, char *argument, int cmd)
{
   char_data *mob;
   char mobname[MAX_INPUT_LENGTH];
   char num[MAX_INPUT_LENGTH];
   int number = 1;

   half_chop(argument, mobname, num);
   if (*num == '\0')
     number = 1;
   else if (isdigit(*num))
     number = atoi(num);
   else
     number = 1;

  if(!*argument)
  {
     send_to_char("Syntax: questup <mob name> <desired level>\n", ch);
     return;
  }
  if ((mob = get_char_vis(ch, mobname)))
  {
    /* mobile in world */
    if (IS_PC(mob) || IS_POLY_PC(mob) || (!CAN_SEE(ch,mob))) {
      send_to_char("No mobile by that name in the world\n\r", ch);
      return;
    }
    if(generate_mob_by_level(mob, number))
      send_to_char("Mobile has been updated\n\r", ch);
    else
    {
      send_to_char("Internal failure, please report\n\r", ch);
      return;
    }
  } else {
      send_to_char("Internal failure, please report\n\r", ch);
      return;
  }
}

/*
 * write a spell in a spellbook.  The object must be of type SPELLBOOK
 * When a god inscribes it, the value is set to 2000.  This makes sure that
 * it is always in the book.  Eventually we could use this value to add
 * power to the spell or how fast the person can learn it, etc.
*/
void do_inscribe_spellbook(struct char_data *ch, char *argument, int cmd)
{

  struct obj_data *book;
  struct spell_info *spell;
  char spellname[MAX_INPUT_LENGTH];
  char objname[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  half_chop(argument, objname, spellname);

  if(!*argument)
  {
     send_to_char("Syntax: inscribe <object name> <spell name>\n", ch);
     return;
  }

  spell = locate_spell(spellname, 0);

  if (( book = get_obj_vis(ch, objname)) && spell && spell->number)
  {
     if(can_inscribe(book))
     {
        inscribe_book(book, spell->number, 2000);
     }
     else
     {
       send_to_char("Sorry, there are no more pages to write on.  Get a new book!\n\r",ch);
       return;
     }
  }
  else
  {
    send_to_char("Huh...you want to write what on where?  Sorry, try again.\n\r",ch);
    return;
  }

  send_to_char("You scribble down some notes on the page.\n\r",ch);

}

struct sonic_functs {
  int (*proc)(void *me, struct char_data* ch, int cmd, char* arg);
};


void do_find(char_data *ch, char *arg, int cmd) {
   if(!IS_PC(ch)) return;

   TrackingSystem.Show(ch, arg);
}

void do_zexit(struct char_data *ch, char *argument, int cmd)
{
   char cZoneNumber[MAX_STRING_LENGTH], cBuf[MAX_STRING_LENGTH];
   room_num iTop, iBot, iDoor, iCount, iZone;
   struct room_direction_data *pExitdata;
   struct room_data *pToRoom;

   only_argument(argument, cZoneNumber);
   iZone = -1;
   if (1==sscanf(cZoneNumber,"%ld", &iZone) &&
       (iZone<0 || iZone>top_of_zone_table)) {
       send_to_char( "That is not a valid zone number\n\r", ch);
   }
   else
   {
     send_to_char("          From  |         To    | Zone Connected To\n\r", ch);
     //zone number is valid
     iTop = zone_table[iZone].top;
     if (iZone==0)
	   iBot = 0;
     else
  	   iBot = zone_table[iZone-1].top + 1;
     //looping through every room in the zone
     for (iCount=iBot; iCount< iTop; iCount++)
     {
       if(!real_roomp(iCount))
	      continue;
       for(iDoor=0; iDoor<=5;iDoor++)
       {
	      pExitdata = real_roomp(iCount)->dir_option[iDoor];
	      if(pExitdata) {
	        pToRoom = real_roomp(pExitdata->to_room);
	        if (pToRoom && pToRoom->zone != iZone)
	        {  //if the room exists, check to see if it is a new zone
	           sprintf(cBuf, "%15ld %15ld %-26.26s\n\r", iCount, pExitdata->to_room, zone_table[pToRoom->zone].name);
	      //     append_to_string_block(&sBlock, cBuf);
		   send_to_char(cBuf, ch);
	        }
	      }
	   }
   }

   }
}
