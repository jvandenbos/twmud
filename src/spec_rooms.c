#include "config.h"

#include <stdio.h>

#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif

#include <string.h>
#include <math.h>

#include "structs.h"
#include "cmdtab.h"
#include "db.h"
#include "find.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "constants.h"
#include "util_str.h"
#include "multiclass.h"
#include "act.h"
#include "utility.h"
#include "util_num.h"
#include "ansi.h"
#include "spell_util.h"
#include "recept.h"
#include "statistic.h"
#include "spec.h"
#include "proto.h"

#include "db.random.h"
#include "spells.h"


/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

int save_storeroom(int room)
{
  char filename[80], buf[MAX_STRING_LENGTH];
  long start;
  struct room_data *roomp;
  struct obj_data *o;
  object_map *map;

  FILE *fp;


  if (!(roomp = real_roomp(room)))
  {
    sprintf(buf,"This can't happen, %d has a storeroom but doesn't exist!",
	    room);
    log_msg(buf);
    return 0;
  }

  sprintf(filename, "strms/%d.storeroom", room);

  if (!(fp=fopen(filename, "wb")))
  {
    sprintf(buf, "Couldn't open storeroom file %s", filename);
    log_msg(buf);
    return FALSE;
  }

  if (putdword(PLAYER_VERSION, fp))
    goto fail_save_storeroom;

  start = ftell(fp);
  if (putdword(0,fp))
    goto fail_save_storeroom;

  map = new_object_map(50);

  for ( o = roomp->contents; o; o= o->next_content ) {
    if (o->item_number >= 0) {
      /* if return is TRUE then we're done */
      if (putobject(o, -1, 0, map, fp, NULL))
      {
	fseek(fp, start, 0);
	if (putdword(map->count-1, fp))
	    goto fail_save_storeroom;
	fclose(fp);
	return 1;
      }
    }
  }

  fseek(fp, start, 0);
  if (putdword(map->count, fp))
    goto fail_save_storeroom;

  fclose(fp);

  kill_object_map(map);

  return 0;

 fail_save_storeroom:
  sprintf(buf,"Unrecoverable Error saving storeroom %d",room);
  log_msg(buf);
  fclose(fp);
  unlink(filename);

  return 0;
}

int load_storeroom(int room)
{
  long count;
  int i;
  long file_version;
  char filename[80], buf[MAX_STRING_LENGTH];
  struct room_data *roomp;
  struct obj_data *obj;
  object_map *map;
  FILE *fp;

  if (!(roomp = real_roomp(room)))
  {
    sprintf(buf, "This can't happen, %d has a storeroom but doesn't exist!",
	    room);
    log_msg(buf);
    return 0;
  }

  sprintf(filename, "strms/%d.storeroom", room);

  if (!(fp=fopen(filename, "rb")))
  {
    save_storeroom(room);
    if (!(fp=fopen(filename, "rb")))
    {
      sprintf(buf, "Couldn't open storeroom file %s", filename);
      log_msg(buf);
      return 0;
    }
  }

  if (getdword(&file_version, fp))
    return 1;

  if(getdword(&count, fp))
    return 1;

  map = new_object_map(50);

  for(i = 0 ; i < count ; ++i)
  {
    if(getobject(&obj, 0, map, 0, fp, file_version, room))
      return 1;
  }

  kill_object_map(map);

  fclose(fp);

  return 0;
}


SPECIAL(storeroom)
{
  static int init = FALSE;
  int i, room, found;
  char buf[MAX_STRING_LENGTH];
  static int num_store_rooms;
  static int *store_rooms;
  obj_data *obj;
  room_data *sroom;

  /* nothing to do */
  if (!cmd) return FALSE;

  for (found = FALSE, room = ch->in_room, i=0; i<num_store_rooms; i++)
  {
    if(store_rooms[i] == room)
    {
      found = TRUE; break;
    }
  }

  if (!init) /* also assume !found */
  {
    init = TRUE;
    num_store_rooms++;
    CREATE(store_rooms, int, num_store_rooms);
    store_rooms[num_store_rooms-1]=room;
    if (load_storeroom(room))
    {
      sprintf(buf,"Error loading storeroom %d",room);
      log_msg(buf);
    }
  } else if (!found)
  {
    num_store_rooms++;
    RECREATE(store_rooms, int, num_store_rooms);
    store_rooms[num_store_rooms-1]=room;
    if (load_storeroom(room))
    {
      sprintf(buf,"Error loading storeroom %d",room);
      log_msg(buf);
    }
  }

  /* disallow drop and get */
  if ( (cmd==CMD_DROP) || (cmd==CMD_GET) )
  {
    send_to_char("You cannot do that here, "
		  "use withdraw or deposit instead\n\r", ch);
    return TRUE;
  }

  /* if not deposit or withdraw, we don't deal with it */
  if ( (cmd!=CMD_WITHDRAW) && (cmd!=CMD_DEPOSIT) && (cmd!=59)) return FALSE;

  switch (cmd)
  {
  case CMD_WITHDRAW:
    /* perform the get */
    do_get(ch, arg, cmd);
    /* save the state of the store */
    if (save_storeroom(ch->in_room))
    {
      sprintf(buf,"Error saving storeroom %d",room);
      log_msg(buf);
    }
    return TRUE;
  case CMD_DEPOSIT:
    /* perform the drop */
    do_drop(ch, arg, cmd);
    /* save the state of the store */
    if (save_storeroom(ch->in_room))
    {
      sprintf(buf,"Error saving storeroom %d",room);
      log_msg(buf);
    }
    return TRUE;
   case 59:
     send_to_char("Your clan keep is storing the following items:\n\r", ch);
     send_to_char("----------------------------------------------\n\r", ch);
     if(!(sroom=real_roomp(ch->in_room))) {
	send_to_char("Error in saveroom\n\r", ch);
	return TRUE;
     }
     for(obj = sroom->contents; obj ; obj = obj->next_content) {
       sprintf(buf, "%s\n\r", OBJ_SHORT(obj));
       send_to_char(buf, ch);
     }
     return TRUE;
   default:
    log_msg("screw up in storeroom()");
  }
  return FALSE;
}


SPECIAL(morgue_shops)
{
  char buf[MAX_STRING_LENGTH], corpse_name[256];
  int corpse_room;
  struct obj_data *corpse;

  corpse_room = ch->in_room+1;

  if (cmd==59) {		/* List */

    send_to_char("The Morgue Keeper says \"I am currently storing the following corpses:\"\n\r", ch);
    for(corpse = real_roomp(corpse_room)->contents; corpse; corpse = corpse->next_content) {
      sprintf(buf, "%8d - %s\n\r", corpse->obj_flags.value[3], OBJ_NAME(corpse));
      send_to_char(buf, ch);
    }
    return(TRUE);
  } else if (cmd==56) {		/* Buy */

    arg = one_argument(arg, buf);
    only_argument(arg, corpse_name);
    /* Corpse_Name is for later use when I feel like it */

    if (!(corpse = get_obj_in_list(buf, real_roomp(corpse_room)->contents))) {
      send_to_char("The Morgue Keper says \"There is no such corpse!\"\n\r", ch);
      return(TRUE);
    }

    if(!isname(GET_REAL_NAME(ch),OBJ_NAME(corpse)))
    {
      send_to_char("The Morgue Keeper says \"Sorry, but I can only release the remains to the immediate family.\"\n\r", ch);
      return (TRUE);
    }

    if (GET_GOLD(ch) < corpse->obj_flags.value[3])
    {
      send_to_char("The Morgue Keeper tells you \"You don't have enough gold!\"\n\r", ch);
      return(TRUE);
    }

    GET_GOLD(ch) -= corpse->obj_flags.value[3];

    corpse->obj_flags.value[3] = PC_CORPSE;

    obj_from_room(corpse);
    obj_to_char(corpse,ch);

    send_to_char("The Morgue Keeper tells you \"Ya'll come back now, ya here? Muahahaha.\"\n\r", ch);
    act("The Morgue Keeper rolls your corpse out on a guerny and hands it over to you.",FALSE,ch,0,0,TO_CHAR);
    act("$n bought $s corpse back.",FALSE,ch,0,0,TO_ROOM);

    return(TRUE);
  }

  /* All commands except list and buy */
  return(FALSE);
}


SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room, count=0;
  struct char_data *pet;
  struct follow_type *pets;

  pet_room = ch->in_room+1;

  if (cmd==59) {		/* List */
    send_to_char("Available pets are:\n\r", ch);
    for(pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
      if((6 * GET_EXP(pet)) > 0)
      {
	sprintf(buf, "%12Lu - %s\n\r", 6*GET_EXP(pet), GET_NAME(pet));
	send_to_char(buf, ch);
      }
    }
    return(TRUE);
  } else if (cmd==56) {		/* Buy */

    arg = one_argument(arg, buf);
    only_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */

    if(!CountFollowers(ch))
      return(TRUE);

    for (pets=ch->followers; pets; pets=pets->next) {
      if (!IS_PC(pets->follower))
	count++;
    }

    if (count>=3) {
      send_to_char("Sorry, but SPCA rules forbid ownership of more than three pets.\n\r", ch);
      return (TRUE);
    }

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n\r", ch);
      return(TRUE);
    }

    if ( ((GET_EXP(pet) * 6) < GET_EXP(pet) ) ||
	(GET_GOLD(ch) < ((long) (GET_EXP(pet)*6))) )
    {
      send_to_char("You don't have enough gold!\n\r", ch);
      return(TRUE);
    }

    GET_GOLD(ch) -= GET_EXP(pet)*6;

    pet = make_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf,"%s %s", GET_IDENT(pet), pet_name);
      ss_free(pet->player.name);
      pet->player.name = ss_make(buf);

      sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
	      ss_data(pet->player.description), pet_name);
      ss_free(pet->player.description);
      pet->player.description = ss_make(buf);
    }

    char_to_room(pet, ch->in_room);
    add_follower(pet, ch, 0);

    IS_CARRYING_W(pet) = 0;
    IS_CARRYING_N(pet) = 0;

    send_to_char("May you enjoy your pet.\n\r", ch);
    act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);

    return(TRUE);
  }

  /* All commands except list and buy */
  return(FALSE);
}


SPECIAL(Fountain)
{
    int bits, water, max_exp;
    char buf[MAX_INPUT_LENGTH];
    struct char_data *tmp_char;
    struct obj_data *obj;

    if (cmd==248) {		/* fill */

	arg = one_argument(arg, buf); /* buf = object */
	bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &obj);

	if (!bits) return(FALSE);

	if (ITEM_TYPE(obj) !=ITEM_DRINKCON) {
	    send_to_char("Thats not a drink container!\n\r", ch);
	    return(TRUE);
	}

	if ((obj->obj_flags.value[2] != LIQ_WATER) &&
	    (obj->obj_flags.value[1] != 0)) {
	    name_from_drinkcon(obj);
	    obj->obj_flags.value[2] = LIQ_SLIME;
	    name_to_drinkcon(obj, LIQ_SLIME);
	} else {
	    /* Calculate water it can contain */
	    water = obj->obj_flags.value[0]-obj->obj_flags.value[1];

	    if (water > 0) {
		obj->obj_flags.value[2] = LIQ_WATER;
		obj->obj_flags.value[1] += water;
		weight_change_object(obj, water);
		name_from_drinkcon(obj);
		name_to_drinkcon(obj, LIQ_WATER);
		act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
		act("$n fills $p with water.", FALSE, ch,obj,0,TO_ROOM);
	    }
	}
	return(TRUE);

    } else if (cmd==11) {	/* drink */

	only_argument(arg,buf);

	if (str_cmp(buf, "fountain") && str_cmp(buf, "water")) {
	    return(FALSE);
	}

	if((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0)) {
	    /* The pig is drunk */
	    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
	    act("$n looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}

	if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
	{
	    act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
	    return TRUE;
	}

#ifdef JANWORK
	send_to_char("You drink from the fountain\n\r", ch);
	act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);
#endif

	if (GET_COND(ch,THIRST) != -1)
	  if (GET_COND(ch,THIRST) < 24) {
	    GET_COND(ch,THIRST) += 12;
	    if (GET_COND(ch,THIRST) > 24)
	      GET_COND(ch,THIRST) = 24;
	    send_to_char("You drink from the fountain.\n\r",ch);
	    if (GET_COND(ch,THIRST) > 20)
	      send_to_char("You do not feel thirsty.\n\r",ch);
	    return TRUE; /* quenched thirst */
	  }



	if (GET_COND(ch,FULL) != -1)
	  if (GET_COND(ch,FULL) < 24) {

	    GET_COND(ch,FULL)+=12;

	    if (GET_COND(ch,FULL)>24)
	      GET_COND(ch,FULL)=24;

	    send_to_char("You munch on food provided from the holy fountain.\n\r",ch);

	    if (GET_COND(ch,FULL) > 20)
	      send_to_char("You are full.\n\r",ch);

	    return TRUE; /* filled tummy */
	  }


	send_to_char("You are already full!\n\r",ch);
	return TRUE;

#ifdef JANWORK
	if(GET_COND(ch,THIRST)>20)
	    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
	if(GET_COND(ch,FULL)>20)
	    act("You are full.",FALSE,ch,0,0,TO_CHAR);
	return(TRUE);
#endif
    } else if (cmd==178) {	/* beg */
	if IS_NPC(ch) return FALSE;

	if(CHAOS)
	    give_reimb_items(ch, FOUNTAIN_LEVEL);

	if ((GetMaxLevel(ch) > 1) || (GET_EXP(ch) > 1)) {
	    if(!CHAOS)
		send_to_char("You are not a newbie.  Go away!\n\r", ch);
	    return TRUE;
	}

	max_exp = total_exp(FOUNTAIN_LEVEL);

	GET_EXP(ch) = max_exp;
	GET_GOLD(ch) += FOUNTAIN_GOLD;
	if (CheckColor(ch)) {
	    sprintf(buf, "%sThe gods take pity and answer your plea.\n\r\
		Please visit your guild(s).%s\n\r",
		    ANSI_BLUE, ANSI_NORMAL);
	    send_to_char(buf, ch);
	} else
	    send_to_char("The gods take pity and answer your plea.\n\r\
		Please visit your guild(s).\n\r", ch);
	return(TRUE);
    }

    /* All commands except fill, drink and beg */
    return(FALSE);
}


SPECIAL(pawnshop)
{
    struct obj_data *temp1;
    int cost, discount, vone, vtwo;
    char argm[MAX_INPUT_LENGTH];
    char buf[190];

    if ((cmd != 56) && (cmd != 57) && (cmd != 58))
	return(FALSE);

    if (cmd==56) {
	send_to_char("Sorry bud, I only BUY stuff!", ch);
	return TRUE;
    }

    if (cmd==58) {

	only_argument(arg, argm);
	if(!(*argm))
	{
	    send_to_char("What do you want to value??\n\r", ch);
	    return TRUE;
	}

	if (!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying)))
	{
	    send_to_char("You are not carrying that item!\n\r", ch);
	    return TRUE;
	}

	if (IS_OBJ_STAT(temp1, ITEM_NODROP))
	{
	    send_to_char
		("You can't let go of it, it must be CURSED!\n\r", ch);
	    return TRUE;
	}

	cost = temp1->obj_flags.cost;
	vone = (int)(cost*.05);
	vtwo = (int)(cost*.15);
        if(vone>(200000*temp1->obj_flags.level) / MAX_MORT)
	   vone = (200000*temp1->obj_flags.level) / MAX_MORT;
	if(vtwo>(200000*temp1->obj_flags.level) / MAX_MORT)
	   vtwo = (200000*temp1->obj_flags.level) / MAX_MORT;

	/* stop people from selling scrolls they create for too much */
	/* if(temp1->obj_flags.type_flag==ITEM_SCROLL) { */
	if((obj_index[temp1->item_number].virt==3052) ||
	   (obj_index[temp1->item_number].virt==3050) ||
	   ((obj_index[temp1->item_number].virt>=6641) &&
	    (obj_index[temp1->item_number].virt<=6648))) {
	    vone = 1;
	    vtwo = 50;
	}

	sprintf(buf,"I value this item at %d to %d coins.'\n\r", vone, vtwo);
	send_to_char(buf,ch);
    }		/* end of if statement for valuing an item */

    if (cmd==57) {

	only_argument(arg, argm);

	if(!(*argm))	{
	    send_to_char("What do you want to pawn??\n\r", ch);
	    return TRUE;
	}

	if (!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying))) {
	    send_to_char("You are not carrying that item!\n\r", ch);
	    return TRUE;
	}

	if (IS_OBJ_STAT(temp1, ITEM_NODROP)) {
	    send_to_char
		("You can't let go of it, it must be CURSED!\n\r", ch);
	    return TRUE;
	}

	/* make sure if its a container, its empty */
	if (temp1->contains) {
	    send_to_char("Empty your containers before you sell them!\n\r",ch);
	    return TRUE;
	}

	if (GET_GOLD(ch)>2000000) {
	    send_to_char("My, your pouch is full, visit a bank.\n\r", ch);
	    return TRUE;
	}

	cost = temp1->obj_flags.cost;
	discount = 5 + number(1,10);
	cost = cost * discount / 100;
	/* if(temp1->obj_flags.type_flag==ITEM_SCROLL) */
	if((obj_index[temp1->item_number].virt==3052) ||
	   (obj_index[temp1->item_number].virt==3056) ||
	   (obj_index[temp1->item_number].virt==3050) ||
	   ((obj_index[temp1->item_number].virt>=6641) &&
	    (obj_index[temp1->item_number].virt<=6648)))
	    cost=(number(1,50));

        if(cost > (200000*temp1->obj_flags.level) / MAX_MORT)
	    cost=(200000*temp1->obj_flags.level) / MAX_MORT;
	sprintf(buf,"Slick Willy gives you %d coins!\n\r", cost);

	send_to_char(buf, ch);

	act("$n pawns $p.", FALSE, ch, temp1, 0, TO_ROOM);

	GET_GOLD(ch) += cost;
	PAWN_SHOP += cost;	/* keep track of all this money,
				   make sure nothing goes out of control */
	obj_from_char(temp1);
	if (temp1 == NULL) {
	    send_to_char("As far as I am concerned, you are out..\n\r",ch);
	    return TRUE;
	}

	extract_obj(temp1);
    }				/* end of if selling statement */

    return TRUE;
}


SPECIAL(dump)
{
    struct obj_data *k;
    char buf[100];
    struct char_data *tmp_char;
    int value=0;

    for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
	sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,
		fname(OBJ_NAME(k)));
	for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	    tmp_char = tmp_char->next_in_room)
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf,tmp_char);
	extract_obj(k);
    }

    if(cmd!=60) return(FALSE);

    do_drop(ch, arg, cmd);

    value = 0;

    for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
	sprintf(buf, "The %s vanish in a puff of smoke.\n\r",
		fname(OBJ_NAME(k)));
	for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	    tmp_char = tmp_char->next_in_room)
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf,tmp_char);
	if(k->obj_flags.cost<=0)
	    value=1;
	else
	    value+=(MIN(1000,MAX(k->obj_flags.cost/4,1)));
	/* fix the dump bug ^^ */
	if(k->obj_flags.type_flag==ITEM_SCROLL)
	    value=1;
	/* make it where people cant earn money from creating scrolls, etc */
	/*
	    value += MAX(1, MIN(50, k->obj_flags.cost/10));
	*/
	extract_obj(k);
    }

    if (value) 	{
	act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n has been awarded for being a good citizen.", TRUE, ch, 0,0, TO_ROOM);

	if (GetMaxLevel(ch) < 3)
	    gain_exp(ch, MIN(100,value));
	else
	    GET_GOLD(ch) += value;
    }
    return TRUE;
}



/*
**  donation room
*/
SPECIAL(Donation)
{
    char check[MAX_INPUT_LENGTH], *tmp;

    if ((cmd != 10) && (cmd != 167))
	return(FALSE);


    tmp = one_argument(arg, check);

    if (*check) {
	if (strncmp(check, "all", 3)==0) {
	    send_to_char("Now now, that would be greedy!\n\r", ch);
	    return(TRUE);
	}
    }
    return(FALSE);
}


/* Function related to the feed_lock special procedure */
struct obj_data *find_key(struct char_data *ch, int key)
{
    struct obj_data* o;

    if(ch->equipment[HOLD])
	if(obj_index[ch->equipment[HOLD]->item_number].virt == key)
	    return ch->equipment[HOLD];

    for(o=ch->carrying; o; o=o->next_content)
	if(obj_index[o->item_number].virt==key)
	    return o;

    return 0;
}

SPECIAL(feed_lock)
{
    struct obj_data *obj;
    int door;
    char dtype[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];

    if (cmd != 102)
      return FALSE;

    /* unlock */

    argument_interpreter(arg, dtype, dir);

    if (!(door = find_door(ch, dtype, dir)))
      return FALSE;
    if (!(obj=find_key(ch, EXIT(ch,door)->key)))
      return FALSE;

    if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
	do_unlock(ch, arg, cmd);
	act("The Lock looks at $p.\n\rthe Lock grins evily.\n\r\n\r",
	    TRUE, ch, obj, 0, TO_ROOM);
	act("The Lock looks at your key.\n\rthe Lock grins evily.\n\r\n\r",
	    TRUE, ch, obj, 0, TO_CHAR);
	act("Hey!. The Lock has eaten $p.\n\r",
	    TRUE, ch, obj, 0, TO_ROOM);
	act("Hey!. The Lock has eaten your key!.\n\r",
	    TRUE, ch, obj, 0, TO_CHAR);
	act("The Lock says 'more food ..more food.. I'm still hungry'\n\r",
	    TRUE, ch, obj, 0, TO_ROOM);
	act("The Lock says 'more food ..more food.. I'm still hungry'\n\r",
	    TRUE, ch, obj, 0, TO_CHAR);

	if (ch->equipment[HOLD] == obj)
	  unequip_char(ch, HOLD);
	extract_obj(obj);
	return TRUE;
    }

    return FALSE;
}


/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

SPECIAL(pray_for_items)
{
    char buf[256];
    int key_room, gold;
    bool found;
    struct obj_data *tmp_obj, *obj;
    struct extra_descr_data *ext;

    if (cmd != 176)		/* You must pray to get the stuff */
	return FALSE;

    key_room = 1+ch->in_room;

    strcpy(buf, "item_for_");
    strcat(buf, GET_NAME(ch));

    gold = 0;
    found = FALSE;

    for (tmp_obj = real_roomp(key_room)->contents; tmp_obj; tmp_obj = tmp_obj->next_content)
	for(ext = tmp_obj->ex_description; ext; ext = ext->next)
	    if (str_cmp(buf, ext->keyword) == 0) {
		if (gold == 0) {
		    gold = 1;
		    act("$n kneels and at the altar and chants a prayer to Odin.",
			FALSE, ch, 0, 0, TO_ROOM);
		    act("You notice a faint light in Odin's eye.",
			FALSE, ch, 0, 0, TO_CHAR);
		}
		if(!(obj = make_object(tmp_obj->item_number, REAL)))
		    return TRUE;
		obj_to_room(obj, ch->in_room);
		act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM);
		act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR);
		gold += obj->obj_flags.cost;
		found = TRUE;
	    }


    if (found) {
	GET_GOLD(ch) -= gold;
	GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
	return TRUE;
    }

    return FALSE;
}


SPECIAL(kings_hall)
{
   if (cmd != 176)
     return(0);

   do_action(ch,arg, 176);

  send_to_char("Your feel a strong force pull at your being!\n\r", ch);
  act("$n is struck by a bright beam of light.", TRUE, ch, 0, 0,
         TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, 7601);
  do_look(ch, "", 15);
  return(1);
}


SPECIAL(bank)
{
    static char buf[256];
    int money;

    money = atoi(arg);

    if(!(cmd==219 || cmd==220 || cmd==221))
	return FALSE;

    if (IS_NPC(ch))
    {
	send_to_char("I'm sorry, but we don't deal with monsters!  Shoo!\n\r",
		     ch);
	return(TRUE);
    }

    if(check_convict(ch, NULL))
    {
	act("The teller looks pointedly between you and a poster on the wall.",
	    TRUE, ch, NULL, NULL, TO_CHAR);
	act("The teller looks pointedly between $n and a poster on the wall.",
	    TRUE, ch, NULL, NULL, TO_ROOM);
	return TRUE;
    }

    if (GET_BANK(ch) > GetMaxLevel(ch)*1800000) {
	send_to_char("I'm sorry, but we can no longer hold more than 1800000 coins per level.\n\r", ch);
	GET_GOLD(ch) += GET_BANK(ch)-GetMaxLevel(ch)*1800000;
	GET_BANK(ch) = GetMaxLevel(ch)*1800000;
    }

    /*deposit*/
    if (cmd==219) {
	if (money > GET_GOLD(ch)) {
	    send_to_char("You don't have enough for that!\n\r", ch);
	    return(TRUE);
	} else if (money <= 0) {
	    send_to_char("Go away, you bother me.\n\r", ch);
	    return(TRUE);
	} else if (money + GET_BANK(ch) > GetMaxLevel(ch)*1800000) {
	    send_to_char("I'm sorry, Regulations only allow us to ensure 1800000 coins per level.\n\r",ch);
	    return(TRUE);
	} else {
	    send_to_char("Thank you.\n\r",ch);
	    GET_GOLD(ch) = GET_GOLD(ch) - money;
	    GET_BANK(ch) = GET_BANK(ch) + money;
	    sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
	    send_to_char(buf, ch);
	    SaveChar(ch, ch->in_room, FALSE);
	    return(TRUE);
	}
	/*withdraw*/

    } else if (cmd==220) {
#define DEPO \
"You are carrying over 2 million gold!!  Deposit it or lose it!\n\r"

  if (GET_GOLD(ch) > 2000000) {
    send_to_char(DEPO, ch);
  }

	if (money > GET_BANK(ch)) {
	    send_to_char("You don't have enough in the bank for that!\n\r", ch);
	    return(TRUE);
	} else if (money <= 0) {
	    send_to_char("Go away, you bother me.\n\r", ch);
	    return(TRUE);
	} else {
	    if((GET_GOLD(ch)<2000000) || (TRUST(ch)>TRUST_SAINT)) {
		send_to_char("Thank you.\n\r",ch);
		GET_GOLD(ch) = GET_GOLD(ch) + money;
		GET_BANK(ch) = GET_BANK(ch) - money;
		sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
		send_to_char(buf, ch);
		return(TRUE);
	    } else {
		send_to_char("You can not possibly carry any more coins!  Make a deposit.\n\r", ch);
	    }
	}
    } else if (cmd == 221) {
	sprintf(buf,"Your balance is %d.\n\r", GET_BANK(ch));
	send_to_char(buf, ch);
	return(TRUE);
    }
    return(FALSE);
}


SPECIAL(to_donate)
{
    struct obj_data *k;
    char buf[100];
    struct char_data *tmp_char;

    for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
	sprintf(buf, "The %s fades out of existence.\n\r" ,
		fname(OBJ_NAME(k)));
	for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	    tmp_char = tmp_char->next_in_room)
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf,tmp_char);
	obj_from_room(k);
   	obj_to_room(k, 2625);
    }

    if(cmd!=60) return(FALSE);

    do_drop(ch, arg, cmd);


    for(k = real_roomp(ch->in_room)->contents; k ; k = real_roomp(ch->in_room)->contents)
    {
	sprintf(buf, "The %s fades out of existence.\n\r",
		fname(OBJ_NAME(k)));
	for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	    tmp_char = tmp_char->next_in_room)
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf,tmp_char);
    }
	obj_from_room(k);
   	obj_to_room(k, 2625);

    return TRUE;
}

//Quilan project...fun clan stuff...
SPECIAL(clan_altar) {
   char buf[MAX_STRING_LENGTH];
   int change=0;
   int align=0;

   if(cmd==59) {
      send_to_char("You must BOW with the intentions of either evil or good.\n\r", ch);
      send_to_char("The cost is: 2 million coins per 350 alignment change.\n\r", ch);
      if(IS_SET(ch->player.clss, CLASS_PALADIN))
        send_to_char("Paladins however only need 500000 to become good.\n\r", ch);
      return TRUE;
   }

   if(cmd!=98) return FALSE;

   one_argument(arg, buf);

   if(!strcmp(buf, "evil")) change=-1;
   if(!strcmp(buf, "good")) change=1;
   if(!change) {
      send_to_char("Your options are as follows:\n\r", ch);
      send_to_char("  bow evil\n\r", ch);
      send_to_char("  bow good\n\r", ch);
      send_to_char("Cost is 2 million coins per 350 alignment change.\n\r", ch);
      if(IS_SET(ch->player.clss, CLASS_PALADIN))
	send_to_char("Paladins however only need 500000 to become good.\n\r", ch);
      return TRUE;
   }

   align=GET_ALIGNMENT(ch);
   if((align==1000) && (change==1)) {
      send_to_char("You cant get any more good than you are...\n\r", ch);
      return TRUE;
   }
   if((align==-1000) && (change==-1)) {
      send_to_char("You can get any more evil than you are...\n\r", ch);
      return TRUE;
   }
   if(((GET_GOLD(ch)<2000000) && (!IS_SET(ch->player.clss, CLASS_PALADIN))) ||
      ((GET_GOLD(ch)<500000) && (IS_SET(ch->player.clss, CLASS_PALADIN)))) {
      send_to_char("HA! You can't afford to pray to your diety!\n\r", ch);
      return TRUE;
   }
   GET_GOLD(ch) -= (IS_SET(ch->player.clss, CLASS_PALADIN)?500000:2000000);

   align += change*350;
   align=MAX(-1000, align);
   align=MIN(align, 1000);
   GET_ALIGNMENT(ch) = align;

   send_to_char("You bow before the altar of your holy diety.\n\r", ch);
   sprintf(buf, "%s bows before the holy altar of %s diety.\n\r", GET_REAL_NAME(ch), HSHR(ch));
   send_to_room_except(buf, ch->in_room, ch);
   switch(change) {
    case 1:
      send_to_char_formatted("$CWYou sense that your evil deads have been cleansed.\n\r", ch);
      sprintf(buf, "$CW%s is enveloped in a holy aura for %s goodness.\n\r", GET_REAL_NAME(ch), HSHR(ch));
      send_to_room_except_formatted(buf, ch->in_room, ch);
      break;
    case -1:
      send_to_char_formatted("$CrYour goodness runs away like water as you grow more sinister.\n\r", ch);
      sprintf(buf, "$Cr%s is enveloped in an evil black cloud for %s sins.\n\r", GET_REAL_NAME(ch), HSHR(ch));
      send_to_room_except_formatted(buf, ch->in_room, ch);
      break;
   }
   return TRUE;
}

int objlen[] = {
   30,
   60,
   256
};

SPECIAL(clan_restring_shop) {
   char buf[MAX_STRING_LENGTH];
   char *buf2;
   char iname[MAX_INPUT_LENGTH];
   char idesc[MAX_INPUT_LENGTH];
   char desctype[MAX_INPUT_LENGTH];

   obj_data *obj;

   int try_token=0;

   if (cmd==59) {
      send_to_char("You may restring any item that you have equipped for 5 million coins per field.\n\r", ch);
      send_to_char("In order to restring, type:\n\r\trestring <item name> <short | long | name> <desc>\n\r", ch);
      return TRUE;
   }

   if(cmd!=525) return FALSE;

   int dtype=0;

   arg = one_argument(arg, iname);
   arg = one_argument(arg, desctype);
   only_argument(arg, idesc);

   buf2=lower(idesc);

   if(isname("token", buf2) &&
      (isname("quest", buf2) ||
       isname("trivia", buf2))) try_token=1;

   if(!*iname || !*desctype || !*idesc) {
      send_to_char("In order to restring, type:\n\r\trestring <item name> <short | long | name> <desc>\n\r", ch);
      return TRUE;
   }

   if(is_abbrev(desctype, "name")) dtype=1;
   if(is_abbrev(desctype, "short")) dtype=2;
   if(is_abbrev(desctype, "long")) dtype=3;

   if(!dtype) {
      send_to_char("In order to restring, type restring <item name> <short | long | name> <desc>\n\r", ch);
      return TRUE;
   }

   if(!(obj=get_obj_in_equip_vis(ch, iname))) {
      sprintf(buf, "You have no item named %s equipped.\n\r", iname);
      send_to_char(buf, ch);
      return TRUE;
   }

   if(GET_GOLD(ch) < 5000000) {
      send_to_char("The Shopkeeper says, 'You can't afford to restring the item! It costs 5000000 coins.'\n\r", ch);
      return TRUE;
   }

   if(try_token && (dtype==1 || dtype==2)) {
      send_to_char("The gods strike you down for trying to create that which cannot be made.\n\r", ch);
      sprintf(buf, "ILLEGAL RESTRING: %s tried to string an object to: %s", GET_REAL_NAME(ch), idesc);
      log_msg(buf);
      sprintf(buf, "   The Gods strike down %s for his attempt to outsmart the gods.\n\r", GET_REAL_NAME(ch));
      send_to_except(buf, ch);
      GET_GOLD(ch) = 0;
      GET_EXP(ch) = 0;
      GET_BANK(ch) = 0;

      return TRUE;
   }

   GET_GOLD(ch) -= 5000000;

   switch(dtype) {
    case 1:
      ch->desc->sstr = &obj->name;
      break;
    case 2:
      ch->desc->sstr = &obj->short_description;
      break;
    case 3:
      ch->desc->sstr = &obj->description;
      break;
    default:
      send_to_char("Wooo...spec: clan_restring_shop has major error in it...", ch);
      break;
   }

   ss_free(*ch->desc->sstr);

   if(strlen(idesc) > (size_t) objlen[dtype-1]) {
      *(idesc + objlen[dtype-1]) = '\0';
      sprintf(buf, "String too long - truncated to:\n\r\t%s\n\r", idesc);
      send_to_char(buf, ch);
   }
   *ch->desc->sstr = ss_make(idesc);
   ch->desc->sstr = 0;

   send_to_char("The Shopkeeper takes your item, and fiddles with it for a minute.\n\r", ch);
   sprintf(buf, "The Shopkeeper takes %s's item, and fiddles with it for a minute.\n\r", GET_REAL_NAME(ch));
   send_to_room_except(buf, ch->in_room, ch);
   send_to_char("The Shopkeeper smiles and says, 'Your item has been successfully restrung.'\n\r", ch);
   send_to_char("The Shopkeeper takes away 5000000 of your coins, and gives your your item back.\n\r", ch);
   sprintf(buf, "The Shopkeeper gives %s %s item back, restrung.\n\r", GET_REAL_NAME(ch), HSHR(ch));
   send_to_room_except(buf, ch->in_room, ch);

   return TRUE;
}

int clan_donation[] = {
   0,
   10423,
   0,
   10321,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   10623
};

SPECIAL(clan_donation_shop) {
   char buf[MAX_STRING_LENGTH];
   char iname[MAX_INPUT_LENGTH];
   struct obj_data *obj, *robj;
   unsigned int pguild;
   struct room_data *droom;

   if((cmd!=56) && (cmd!=59) && (cmd!=419)) return FALSE;

   pguild = ch->player.guildinfo.inguild();
   if(pguild>0) pguild--;
   if((pguild > sizeof(clan_donation)/sizeof(int))) {
      if(pguild+1==0)
	 return FALSE;
      send_to_char("Your clan number is invalid.\n\r", ch);
      send_to_char("Contact an imm about this.\n\r", ch);
      return TRUE;
   }

   if(!(droom=real_roomp(clan_donation[pguild]))) {
      sprintf(buf, "For some reason, the following clan number has come up invalid:\n\r\t"
                   "Room %i\t\tClan #%i", clan_donation[pguild], pguild);
      send_to_char(buf, ch);
      return TRUE;
   }

   if(cmd==59) {
      send_to_char("Your clan keep is holding the following items:\n\r", ch);
      send_to_char("----------------------------------------------\n\r", ch);
      for(obj = droom->contents; obj ; obj = obj->next_content)
	if(obj->obj_flags.cost > 1) {
	   sprintf(buf, "%8i - %s\n\r", obj->obj_flags.cost, OBJ_SHORT(obj));
	   send_to_char(buf, ch);
	}
      return TRUE;
   }

   only_argument(arg, iname);

   if(cmd==56) {
      if(!*iname) {
	 send_to_char("You wanted to buy something?\n\r", ch);
	 return TRUE;
      }

      if(!(obj=get_obj_in_list(iname, droom->contents))) {
	 send_to_char("There is nothing by that name in your clan's donation room.\n\r", ch);
	 return TRUE;
      }

      if(IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
	 send_to_char("You can't carry any more items!\n\r", ch);
	 return TRUE;
      }

      if(GET_GOLD(ch) < obj->obj_flags.cost) {
	 send_to_char("You can't afford that item!\n\r", ch);
	 return TRUE;
      }

      if(!(robj=clone_object(obj))) {
	 send_to_char("Err...for some reason this isn't working...\n\r", ch);
	 log_msg("Error loading obj in clan_donation_shop");
	 return TRUE;
      }

      int i;

      for(i=0;i<4;i++)
         robj->obj_flags.value[i] = obj->obj_flags.value[i];

      for (i=0;i<MAX_OBJ_AFFECT;i++) {
	 robj->affected[i].location = obj->affected[i].location;
	 robj->affected[i].modifier = obj->affected[i].modifier;
      }

      robj->obj_flags.cost_per_day = -1;
      obj_to_char(robj, ch);
      act("$n buys $p.", FALSE, ch, robj, 0, TO_ROOM);
      act("You buy $p.", FALSE, ch, robj, 0, TO_CHAR);

      GET_GOLD(ch) -= robj->obj_flags.cost;
      return TRUE;
   }

   if(cmd!=419) return FALSE;

   if(!*iname) {
      send_to_char("What do you wish to donate?\n\r", ch);
      return TRUE;
   }

   if(!(obj=get_obj_in_list_vis(ch, iname, ch->carrying))) {
      send_to_char("You can't donate what you don't have.\n\r", ch);
      return TRUE;
   }

   if ((obj_index[obj->item_number].virt == 22006) || (obj_index[obj->item_number].virt == 22051))
   {
     send_to_char("You honestly think you can donate that?\n\r", ch);
     return TRUE;
   }

   if(IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
      send_to_char("You can't donate it. It's cursed!\n\r", ch);
      return TRUE;
   }

   if((obj->obj_flags.cost<1) || (obj->obj_flags.cost_per_day<1)) {
      send_to_char("Try donating something with a bit of value.\n\r", ch);
      return TRUE;
   }

   act("$n donates $p to his clan keep.", TRUE, ch, obj, 0, TO_ROOM);
   act("You generously donate $p to your clan keep.", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   obj_to_room(obj, clan_donation[pguild]);
   return TRUE;
}


/* Dar stuff */
SPECIAL(clan_forger) {
   char buf[MAX_STRING_LENGTH];
   char *buf2;
   char iname1[MAX_INPUT_LENGTH];
   char iname2[MAX_INPUT_LENGTH];
	/* idesc could be used to take a 3rd argument as the name of the new weapon? not implemented */
   char idesc[MAX_INPUT_LENGTH];

   obj_data *obj1, *obj2, *obj;

	int cost, affslot, i, j, rnd_index, nummer;
	int fdice, fdam;
	int obj_nr;

	obj_affected_type affects[MAX_OBJ_AFFECT*2], rand_affects[MAX_OBJ_AFFECT*2];

   if (cmd==59) {
      send_to_char("The forger can attempt to create a new weapon from two weapons in your inventory - at a cost.\n\r", ch);
      send_to_char("Dropping an ivory broach in the room could add some magic to the weapon.\n\r", ch);
      send_to_char("In order to forge, type:\n\r\tforge <weapon1 name> <weapon2 name>\n\r", ch);
      return TRUE;
   }

   if(cmd!=570) return FALSE;

   arg = one_argument(arg, iname1);
   arg = one_argument(arg, iname2);
   only_argument(arg, idesc);

   if(!*iname1 || !*iname2 ) { 
      /*|| !*idesc) {*/
      send_to_char("In order to forge, type:\n\r\tforge <weapon1 name> <weapon2 name>\n\r", ch);
      return TRUE;
   }


    if(!(obj1=get_obj_in_list_vis(ch, iname1, ch->carrying))) {
	   sprintf(buf, "You have no item named %s.\n\r", iname1);
      send_to_char(buf, ch);
      return TRUE;
    }
    if(!(obj2=get_obj_in_list_vis(ch, iname2, ch->carrying))) {
	   sprintf(buf, "You have no item named %s.\n\r", iname2);
      send_to_char(buf, ch);
      return TRUE;
    }
    if (obj1==obj2) {
	  sprintf(buf, "%s and %s are the same weapon!\n\r", iname1, iname2);
      send_to_char(buf, ch);
      return TRUE;
    }

    if (ITEM_TYPE(obj1) != ITEM_WEAPON || ITEM_TYPE(obj2) != ITEM_WEAPON) {
	  sprintf(buf, "%s and %s are not both weapons!\n\r", iname1, iname2);
      send_to_char(buf, ch);
      return TRUE;
     }

    /* PURE-class items can not be mixed with regular */
    if (IS_PURE_ITEM(obj1) || IS_PURE_ITEM(obj2)) {
		sprintf(buf, "The Forger whispers, 'Are you crazy? Tampering with PURE items would anger the gods!'\n\r");
      	send_to_char(buf, ch);
      	return TRUE;
    }

	/* Only forge weapons of same damage-type - define as dam_type: blunt(1), slash(2), pierce(3) */
	if(GET_WPN_DMG_TYPE(obj1)!=GET_WPN_DMG_TYPE(obj2)) {
	  sprintf(buf, "The weapons need to be of the same damage type! %s!=%s\n\r", GET_WPN_DMG_TYPE(obj1),GET_WPN_DMG_TYPE(obj2));
          send_to_char(buf, ch);
          return TRUE;
	}
	
	if(obj1->obj_flags.cost_per_day < 0 || obj2->obj_flags.cost_per_day < 0) {
		sprintf(buf, "You can't forge godly items, %s!\n\r", GET_REAL_NAME(ch));
      	send_to_char(buf, ch);
      	return TRUE;
	}

	if(IS_SET(obj1->obj_flags.extra_flags, ITEM_RARE) || IS_SET(obj2->obj_flags.extra_flags, ITEM_RARE)) {
		sprintf(buf, "%s is too rare to be tampered with!\n\r", (IS_SET(obj1->obj_flags.extra_flags, ITEM_RARE))?iname1:iname2);
		send_to_char(buf, ch);
		return TRUE;
	}

	cost = 100000;
	if(obj1->obj_flags.cost >= 0 && obj2->obj_flags.cost >= 0)
		cost += (obj1->obj_flags.cost + obj2->obj_flags.cost);

	if(!IS_GOD(ch) && GET_GOLD(ch) < cost) {
		sprintf(buf, "The Forger says, 'You can not afford to forge these weapons! It costs %d coins.'\n\r", cost);
      	send_to_char(buf, ch);
      	return TRUE;
        }

	/* NOTE: no cost for Imms while testing - ideally Imms shouldn't forge at all: if(IS_GOD(ch)) return TRUE; */
  	if(!IS_GOD(ch)) {
   		GET_GOLD(ch) -= cost;
   	}


	/* Get affects and randomize */
	affslot = 0;
	bool duplicateCheck = TRUE;

        // Solaar: Mud crashes when fighting with a weapon that has two of the same AOE.
	for(i = 0 ; i < MAX_OBJ_AFFECT ; ++i)
	{
                // affectsDuplicateCheck: TRUE==no duplicates, FALSE==there are duplicates
		if (obj1->affected[i].location != APPLY_NONE) {
			for (int j = 0; j<=affslot; j++)
			{
				// Is the affect a weapon spell, and is it already in the affects listing?
				if ((obj1->affected[i].location == 30) && (obj1->affected[i].modifier == affects[j].modifier))
					// If it's already in the affects listing, is it an area affect spell?
					// Currently, weapons with two of the same aoe spell will crash the mud.
					switch ( obj1->affected[i].modifier ) {
						case 10:
						case 23:
						case 32:
						case 34:
						case 55:
						case 71:
						case 72:
						case 96:
						case 97:
						case 110:
						case 127:
							duplicateCheck = FALSE;
							break;
					}
			}
			// TRUE: no duplicates, FALSE: there is a duplicate
			if(duplicateCheck) {
			    	affects[affslot] = obj1->affected[i];
		    		++affslot;
			}
			duplicateCheck = TRUE;
	    	}
	}
	for(i = 0 ; i < MAX_OBJ_AFFECT ; ++i)
	{
		if (obj2->affected[i].location != APPLY_NONE) {
			for (int j = 0; j<=affslot; j++)
			{
				// Is the affect a weapon spell, and is it already in the affects listing?
				if ((obj2->affected[i].location == 30) && (obj2->affected[i].modifier == affects[j].modifier))
					// If it's already in the affects listing, is it an area affect spell?
					// Currently, weapons with two of the same aoe spell will crash the mud.
					switch ( obj2->affected[i].modifier ) {
						case 10:
						case 23:
						case 32:
						case 34:
						case 55:
						case 71:
						case 72:
						case 96:
						case 97:
						case 110:
						case 127:
							duplicateCheck = FALSE;
							break;
					}
			}
			// TRUE: no duplicates, FALSE: there is a duplicate
			if(duplicateCheck) {
			    	affects[affslot] = obj2->affected[i];
		    		++affslot;
			}
			duplicateCheck = TRUE;
		}
	}

	/*
	sprintf(buf, "Debug, 'affslot:%d'\n\r", affslot);
		send_to_char(buf, ch);
	*/
	/* shuffle the array */
	j = 0;
	for(i = 0 ; i < affslot-1 ; ++i)
	{
	  /*debug: affslot-length & number of affects */
	  
		rnd_index = number(0,affslot-1-i);
		/*
		sprintf(buf, "debug: %d i=%d rnd=%d\n\r", affslot,i,rnd_index);
      	send_to_char(buf, ch);
		*/
		rand_affects[i] = affects[rnd_index];
		/* Now loop from rnd_index and shift affects down - remove current affect so as not to use it twice */
		for(j = rnd_index ; j < affslot-1-i ; j++)
		{
			affects[j] = affects[j+1];
		}
	}

	/* Load a forger-workpiece from tinyworld based on dam_type */
	obj_nr = (obj1->obj_flags.value[3]>9  ? 32 : (obj1->obj_flags.value[3]>4  ? 31 : 33 ));
	/* NOTE: using REAL here suddenly crashes on my new comp */
	if(!(obj = make_object(obj_nr, VIRTUAL))) {
		log_msg("screwup in clan_forger");
		send_to_char("The Forger screwed up...", ch);
		return TRUE;
	}


	j = 0;
	for(i = 0 ; i < affslot-1 ; i++)
	{
		/* Can be altered as to not give 100% chance of adding affect */
		if ((percent() < 100)) {
			obj->affected[j] = rand_affects[i];
		}
		j++;
		if(j>MAX_OBJ_AFFECT) {
			break;
		}
	}


	/* NOTE: only add random-affect if some specific item is in the room/on the floor */
	obj_data *objs;
	int added_random;
	added_random = 0;

	for (objs = real_roomp(ch->in_room)->contents; objs; objs = objs->next_content) {
		if(objs->item_number>0) {
			switch(obj_index[objs->item_number].virt) {
    		/* Ivory broach */
			case 2221:
	    	/* sparkling jewel */
			case 18241:
	    		add_random_obj(obj);
	    		added_random = 1;
			obj_from_room(objs);
			extract_obj(objs);
	    		break;
			}
		}
		if(added_random==1)
			break;
	}


	/* Set extra_bits of first item, then loop and set additional bits from 2nd item */
	/* See void sprintbit(..) */

	obj->obj_flags.extra_flags = obj1->obj_flags.extra_flags;
	
	long nr;
	unsigned long vektor;
	char *result;

	int flag_bits[] =
	{
  ITEM_GLOW,
  ITEM_HUM,
  ITEM_METAL,
  ITEM_MINERAL,
  ITEM_ORGANIC,
  ITEM_INVISIBLE,
  ITEM_MAGIC,
  ITEM_NODROP,
  ITEM_BLESS,
  ITEM_ANTI_GOOD,
  ITEM_ANTI_EVIL,
  ITEM_ANTI_NEUTRAL,
  ITEM_ANTI_CLERIC,
  ITEM_ANTI_MAGE,
  ITEM_ANTI_THIEF,
  ITEM_ANTI_FIGHTER,
  ITEM_BRITTLE,
  ITEM_ANTI_PALADIN,
  ITEM_ANTI_DRUID,
  ITEM_ANTI_PSI,
  ITEM_ANTI_RANGER,
  ITEM_UNUSED0,
  ITEM_UNUSED1,
  ITEM_UNUSED2,
  ITEM_NO_LOCATE,
  ITEM_RARE,
  ITEM_ANTI_BARD,
  ITEM_ANTI_MONK,
  ITEM_PURE_CLASS,
  ITEM_TWO_HANDED,
  ITEM_ANTI_SHIFTER,
  ITEM_HARDEN,
  0
	};

	vektor = obj2->obj_flags.extra_flags;
	result = " ";

	if(!IS_IMMORTAL(ch)) 
	{
  		for(nr=0; vektor; vektor>>=1)
		{
			result = " ";
			if (IS_SET(1, vektor))
      				if (!IS_SET(obj->obj_flags.extra_flags, flag_bits[nr])) 
      					if(flag_bits[nr] != 0)
      						SET_BIT(obj->obj_flags.extra_flags, flag_bits[nr]);
			nr++;
		}

		SET_BIT(obj->obj_flags.extra_flags, ITEM_RARE);
        }

	int luck, maxdam, mindam;
	int luckmod, luckdmgmod;
      luck = percent();

        if(obj1->obj_flags.level>89 || obj2->obj_flags.level>89) {
           luckmod = (int) (sqrt(GetMaxLevel(ch)) + GET_CHA(ch)/2);
	   luckdmgmod = (int) (sqrt(luckmod)*2);
	} else {
	   luckmod = 0;
	   luckdmgmod = 2;
	}

	maxdam = (obj1->obj_flags.value[1]*obj1->obj_flags.value[2]) + (obj2->obj_flags.value[1]*obj2->obj_flags.value[2]);
	mindam = ((obj1->obj_flags.value[1]*obj1->obj_flags.value[2]) > (obj2->obj_flags.value[1]*obj2->obj_flags.value[2])) ?
		(obj1->obj_flags.value[1]*obj1->obj_flags.value[2]) : (obj2->obj_flags.value[1]*obj2->obj_flags.value[2]);
	fdice = (int) sqrt(maxdam) + luckdmgmod;
	fdam = fdice;

	if(luck<20-luckmod) {
		fdice = (int) ((fdice>10) ? fdice-3 : fdice-2);
	} else if(luck<60+(luckmod/2)) {
		fdice = (int) ((fdice>10) ? fdice-2 : fdice-1);
	} else {
		fdice = (int) fdice+luckdmgmod;
	}
	fdice = (fdice<1) ? 1 : fdice;
	fdice = (fdice<sqrt(mindam)) ? (int) sqrt(mindam) : fdice;


	fdice = number((int) sqrt(mindam), fdice);
	fdam = number((int) sqrt(mindam), fdam);


	while((fdice*fdam)<=mindam) {
		fdam++;
	}

	if(IS_IMMORTAL(ch))
	{
                obj->obj_flags.value[1] = (int) sqrt(maxdam) + luckdmgmod;;
                obj->obj_flags.value[2] = (int) sqrt(maxdam) + luckdmgmod;;
	}
	else
	{
		obj->obj_flags.value[1] = fdice;
		obj->obj_flags.value[2] = fdam;
	}

	obj->obj_flags.encumbrance = 33;
	obj->obj_flags.level = ((obj1->obj_flags.level > obj2->obj_flags.level) ? obj1->obj_flags.level : obj2->obj_flags.level);
	obj->obj_flags.cost = 100000;
	obj->obj_flags.cost_per_day = 999;
	obj->obj_flags.weight = 5; /*not important*/


	if(obj_nr == 33) 
        {
		sprintf(buf, "%s's deadly Mace", GET_REAL_NAME(ch));
		obj->short_description = ss_make(buf);
		sprintf(buf, "mace %s", lower(GET_REAL_NAME(ch)));
		obj->name = ss_make(buf);
	} else if(obj_nr == 32)
          {
		sprintf(buf, "%s's forged Katana", GET_REAL_NAME(ch));
		obj->short_description = ss_make(buf);
		sprintf(buf, "katana %s", lower(GET_REAL_NAME(ch)));
		obj->name = ss_make(buf);
	  } else 
            {
		sprintf(buf, "%s's demonic Dagger", GET_REAL_NAME(ch));
		obj->short_description = ss_make(buf);
		sprintf(buf, "dagger %s", lower(GET_REAL_NAME(ch)));
		obj->name = ss_make(buf);
	    }


    obj_to_char(obj,ch);
    
    if (obj1->carried_by)
      obj_from_char(obj1);
    if (obj2->carried_by)
      obj_from_char(obj2);
    
    extract_obj(obj1);
    extract_obj(obj2);
  
   send_to_char("The Forger takes your items, and heats them up in the magical hearth.\n\r", ch);
   sprintf(buf, "The Forger takes %s's items, and puts them in the magical hearth.\n\r", GET_REAL_NAME(ch));
   send_to_room_except(buf, ch->in_room, ch);
   send_to_char("After some time the Forger smiles and says, 'Your item has been successfully crafted!'\n\r", ch);
   if(!IS_IMMORTAL(ch)) {
   	sprintf(buf, "The Forger takes away %d of your coins, and hands over your new weapon.\n\r", cost);
   	send_to_char(buf, ch);
   }
   sprintf(buf, "The Forger gives %s %s newly crafted weapon!\n\r", GET_REAL_NAME(ch), HSHR(ch));
   send_to_room_except(buf, ch->in_room, ch);

   return TRUE;
}
