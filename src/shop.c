
#include "config.h"

#include <stdio.h>
#include <string.h>
#if USE_unistd
#include <unistd.h>
#endif
#if USE_stdlib
#include <stdlib.h>
#endif
    
#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "utils.h"
#include "act.h"
#include "utility.h"
#include "multiclass.h"
#include "constants.h"
#include "spec.h"
#include "shop.h"
#include "find.h"
#include "util_str.h"
#include "vnum.h"

#define SHOP_FILE "tinyworld.shp"
#define MAX_TRADE 5
#define MAX_PROD 5

struct shop_data
{
	int producing[MAX_PROD];/* Which item to produce (virtual)      */
	double profit_buy;       /* Factor to multiply cost with.        */
	double profit_sell;      /* Factor to multiply cost with.        */
	byte type[MAX_TRADE];   /* Which item to trade.                 */
	char *no_such_item1;    /* Message if keeper hasn't got an item */
	char *no_such_item2;    /* Message if player hasn't got an item */
	char *missing_cash1;    /* Message if keeper hasn't got cash    */
	char *missing_cash2;    /* Message if player hasn't got cash    */
	char *do_not_buy;	/* If keeper doesn't buy such things. 	*/
	char *message_buy;      /* Message when player buys item        */
	char *message_sell;     /* Message when player sells item       */
	int temper1;           	/* How does keeper react if no money    */
	int temper2;           	/* How does keeper react when attacked  */
	int keeper;             /* The mobil who owns the shop (virtual)*/
	int with_who;		/* Who does the shop trade with?	*/
	room_num in_room;      	/* Where is the shop?			*/
	int open1,open2;	/* When does the shop open?		*/
	int close1,close2;	/* When does the shop close?		*/
	int gold;               /* initial gold in the shop             */
	int deficit_gold;       /* gold value when prices increase      */  
};

struct shop_data *shop_index;
int number_of_shops;

double shopping_buy_cost(struct shop_data *shop, struct char_data *keeper,
			 double value);

int is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr)
{
	if (shop_index[shop_nr].open1>time_info.hours){
		do_say(keeper,
		"Come back later!",17);
		return(FALSE);
	} else if (shop_index[shop_nr].close1<time_info.hours)
		if (shop_index[shop_nr].open2>time_info.hours){
			do_say(keeper,
			"Sorry, we have closed, but come back later.",17);
			return(FALSE);
		} else if (shop_index[shop_nr].close2<time_info.hours){
			do_say(keeper,
			"Sorry, come back tomorrow.",17);
			return(FALSE);
		};
/*
if(!(CAN_SEE(keeper,ch)))
{
do_say(keeper,"I don't trade with someone I can't see!",17);
return(FALSE);
};
*/

/*
if(check_convict(ch, keeper))
return FALSE;
*/

	switch(shop_index[shop_nr].with_who){
		case 0 : return(TRUE);
		case 1 : return(TRUE);
		default : return(TRUE);
	};
}

int trade_with(struct obj_data *item, int shop_nr)
{
	int counter;

   if(item->obj_flags.cost < 1) return(FALSE);

	for(counter=0;counter<MAX_TRADE;counter++)
		if(shop_index[shop_nr].type[counter]==item->obj_flags.type_flag)
			return(TRUE);
	return(FALSE);
}

int shop_producing(struct obj_data *item, int shop_nr)
{
	int counter;

	if(item->item_number<0) return(FALSE);

	for(counter=0;counter<MAX_PROD;counter++)
		if (shop_index[shop_nr].producing[counter] == item->item_number)
			return(TRUE);
	return(FALSE);
}

void shopping_buy( char *arg, struct char_data *ch,
		  struct char_data *keeper, int shop_nr)
{
    char argm[100], buf[MAX_STRING_LENGTH], newarg[100];
    int num = 1, cnt = 0, total = 0;
    struct obj_data *temp1;
    struct obj_data *it;
    struct shop_data* the_shop;
    
    if(!(is_ok(keeper,ch,shop_nr)))
	return;

    /*
       if(IS_NPC(ch))
       {
       send_to_char("Shoo, I only deal with players.\n\r",ch);
       return;
       }
       */
    
    the_shop = &shop_index[shop_nr];
    
    only_argument(arg, argm);
    if(!(*argm))
    {
	sprintf(buf, "%s what do you want to buy??", GET_REAL_NAME(ch));
	do_tell(keeper,buf,19);
	return;
    };
  
    if ((num = getabunch(argm,newarg)))
	strcpy(argm,newarg);
    if (num == 0) num = 1;
  
    if(!(temp1 = get_obj_in_list_vis(ch,argm,keeper->carrying)))
    {
	sprintf(buf, the_shop->no_such_item1, GET_REAL_NAME(ch));
	do_tell(keeper,buf,19);
	return;
    }
  
    if ((IS_CARRYING_N(ch) + num) > (CAN_CARRY_N(ch)))
    {
	sprintf(buf,"%s : You can't carry that many items.\n\r", 
		fname(OBJ_NAME(temp1)));
	send_to_char(buf, ch);
	return;
    }

    for (cnt = 0; cnt < num; cnt++)
    {
	if(!(it = get_obj_in_list_vis(ch, argm, keeper->carrying)))
	{
sprintf(buf, "%s I seem to have run out of those!\n\r", GET_REAL_NAME(ch));
	    do_tell(keeper, buf, 19);
	    break;
	}

	if(it->obj_flags.cost <= 0)
	{
	    extract_obj(it);
	    continue;
	}
	
	if ((GET_GOLD(ch)<
	     ((int) shopping_buy_cost(the_shop, keeper,
					    it->obj_flags.cost))) 
	    /* ((double) it->obj_flags.cost * the_shop->profit_buy))) */
	    && (TRUST(ch) < TRUST_DEMIGOD))
	  {
	    sprintf(buf,
		    the_shop->missing_cash2,
		    GET_REAL_NAME(ch));
	    do_tell(keeper,buf,19);
	    
	    switch(the_shop->temper1)	{
	    case 0:
		do_action(keeper,GET_REAL_NAME(ch),30);
		return;
	    case 1:
		do_emote(keeper,"grins happily",36);
		return;
	    }
	    break;
	}
  
	if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(it)) > CAN_CARRY_W(ch))
	{
	    sprintf(buf,"%s : You can't carry that much weight.\n\r", 
		    fname(OBJ_NAME(temp1)));
	    send_to_char(buf, ch);
	    break;
	}

	if(shop_producing(it, shop_nr))
	{
	    if((it = make_object(it->item_number, NORAND)) == NULL)
		break;
	}
	else
	    obj_from_char(it);
	
	obj_to_char(it, ch);
	act("$n buys $p.", FALSE, ch, it, 0, TO_ROOM);
	act("You buy $p.", FALSE, ch, it, 0, TO_CHAR);

	total = (int) ((double) it->obj_flags.cost * the_shop->profit_buy);
	GET_GOLD(keeper) += total;
	GET_GOLD(ch) -= total;
    

      } 
    
    sprintf(buf, the_shop->message_buy, GET_REAL_NAME(ch), total);
    do_tell(keeper,buf,19);

    return; 
}

void shopping_sell( char *arg, struct char_data *ch,
		   struct char_data *keeper,int shop_nr)
{
  char argm[100], buf[MAX_STRING_LENGTH];
  int cost, rn;
  struct obj_data *temp1;
  
  if(!(is_ok(keeper,ch,shop_nr)))
    return;
  
  only_argument(arg, argm);
  
  if(!(*argm))	{
    sprintf(buf, "%s What do you want to sell??"
	    ,GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
  
  if (!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying))) {
    sprintf(buf, shop_index[shop_nr].no_such_item2,GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
  
  if (IS_OBJ_STAT(temp1, ITEM_NODROP)) {
     send_to_char
      	("You can't let go of it, it must be CURSED!\n\r", ch);
     return;
  }

  if (!(trade_with(temp1,shop_nr))||(temp1->obj_flags.cost<1)) {
    sprintf(buf,shop_index[shop_nr].do_not_buy,
	    GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
  
  if (GET_GOLD(keeper)<(int) (temp1->obj_flags.cost*
			      shop_index[shop_nr].profit_sell)) {
    sprintf(buf,shop_index[shop_nr].missing_cash1
	    ,GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }

  if (GET_GOLD(ch)>2000000) {
    send_to_char("You can not possibly carry any more coins!  Visit the bank.\n\r", ch);
    return;
  }

  rn = temp1->item_number;
  
  if ((obj_index[rn].virt == IDSCROLL1) ||
     (obj_index[rn].virt == IDSCROLL2) )
  {
    sprintf(buf, "%s I do not need any of those!", GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
	
  cost = temp1->obj_flags.cost;
  
  if ((ITEM_TYPE(temp1) == ITEM_WAND) ||
      (ITEM_TYPE(temp1) == ITEM_STAFF)) {
    if (temp1->obj_flags.value[1]) {
      cost = (int)(cost * ((double) temp1->obj_flags.value[2] /
			   (double) temp1->obj_flags.value[1]));
    } else {
      cost = 0;
    }
  } else if (ITEM_TYPE(temp1) == ITEM_ARMOR) {
    if (temp1->obj_flags.value[1]) {
      cost = (int)(cost * ((double)temp1->obj_flags.value[0] /
			   (double)temp1->obj_flags.value[1]));
    } else {
      cost = 0;
    }
  }
  
  temp1->obj_flags.cost = cost;
  
  act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);
  
  sprintf(buf,shop_index[shop_nr].message_sell,
	  GET_REAL_NAME(ch),(int) (temp1->obj_flags.cost*
			      shop_index[shop_nr].profit_sell));
  
  do_tell(keeper,buf,19);
  
  sprintf(buf,"The shopkeeper now has %s.\n\r",
	  OBJ_SHORT(temp1));
  send_to_char(buf,ch);
  
  if (GET_GOLD(keeper)<(int) (temp1->obj_flags.cost*
			      shop_index[shop_nr].profit_sell)) {
    sprintf(buf,shop_index[shop_nr].missing_cash1
	    ,GET_REAL_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }
  
  GET_GOLD(ch) += (int) (temp1->obj_flags.cost*
			 shop_index[shop_nr].profit_sell);
  GET_GOLD(keeper) -= (int) (temp1->obj_flags.cost*
			     shop_index[shop_nr].profit_sell);
  
  obj_from_char(temp1);
  if (temp1 == NULL) {
    send_to_char("As far as I am concerned, you are out..\n\r",ch);
    return;
  }
  if ((get_obj_in_list(argm,keeper->carrying)) || 
      (GET_ITEM_TYPE(temp1) == ITEM_TRASH)) {
    extract_obj(temp1);
  } else {
    obj_to_char(temp1,keeper);
  }
  return;
}

void shopping_value( char *arg, struct char_data *ch, 
		    struct char_data *keeper, int shop_nr)
{
  char argm[100], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1;
  
  if(!(is_ok(keeper,ch,shop_nr)))
    return;
  
  only_argument(arg, argm);
  
  if(!(*argm))
    {
      sprintf(buf,"%s What do you want me to valuate??",
	      GET_REAL_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }
  
  if(!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying)))
    {
      sprintf(buf,shop_index[shop_nr].no_such_item2,
	      GET_REAL_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }
  
  if(!(trade_with(temp1,shop_nr)))
    {
      sprintf(buf,
	      shop_index[shop_nr].do_not_buy,
	      GET_REAL_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }
  
  sprintf(buf,"%s I'll give you %d gold coins for that!",
	  GET_REAL_NAME(ch),(int) (temp1->obj_flags.cost*
			      shop_index[shop_nr].profit_sell));
  do_tell(keeper,buf,19);
  
  return;
}


double shopping_buy_cost(struct shop_data *shop, struct char_data *keeper, 
			  double value)
{
  if (keeper->points.gold <= shop->deficit_gold)
    return (value*(shop->profit_buy+value/7500.00));
  else
    return(value*shop->profit_buy);
}


void shopping_list( char *arg, struct char_data *ch,
		   struct char_data *keeper, int shop_nr)
{
    struct obj_data *temp1;
    int found_obj;
    double cost;
    struct shop_data* shop;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    
    if(!(is_ok(keeper,ch,shop_nr)))
	return;
  
    send_to_char("You can buy:\n\r", ch);
    found_obj = FALSE;

    shop = shop_index + shop_nr;
    
    for(temp1=keeper->carrying ; temp1 ; temp1 = temp1->next_content)
	if((CAN_SEE_OBJ(ch,temp1)) && (temp1->obj_flags.cost>0))
	{
	    found_obj = TRUE; 
	    cost = (double) temp1->obj_flags.cost;
	    cost = shopping_buy_cost(shop, keeper, cost);

/*	    fac = shop*->profit_buy;
	    cost *= fac;
*/
	    
	    if((temp1->obj_flags.type_flag != ITEM_DRINKCON) ||
	       (temp1->obj_flags.value[1] == 0))
	    {
		sprintf(buf, "%s for %d gold coins.\n\r",
			OBJ_SHORT(temp1), (int) cost);
	    }
	    else
	    {
		sprinttype(temp1->obj_flags.value[2], drinks, buf2);
		sprintf(buf, "%s of %s for %d gold coins.\n\r",
			OBJ_SHORT(temp1), buf2, (int) cost);
	    }
	    send_to_char(CAP(buf), ch);
	};
  
    if(!found_obj)
	send_to_char("Nothing!!\n\r", ch);
}

void shopping_kill( char *arg, struct char_data *ch,
		   struct char_data *keeper, int shop_nr)
{
  char buf[100];
  
  switch(shop_index[shop_nr].temper2)
    {
    case 0:
      sprintf(buf,"%s Don't ever try that again!",
	      GET_REAL_NAME(ch));
      do_tell(keeper,buf,19);
      return;
      
    case 1:
      sprintf(buf,"%s Scram - midget!",
	      GET_REAL_NAME(ch));
      do_tell(keeper,buf,19);
      return;
      
      default :
	return;
    }
}

void init_shop_keeper(struct char_data *ch, struct shop_data *shop)
{
  int count;
  struct obj_data *obj;
  
  for(count=0;count<MAX_PROD;count++)
    if (shop->producing[count] >= 0)
      if((obj= make_object(shop->producing[count],REAL)) != NULL)
	obj_to_char(obj,ch);
  ch->points.gold = shop->gold;
  ch->act_ptr=1;
}

SPECIAL(shop_keeper)
{
  char argm[MAX_INPUT_LENGTH];
  struct char_data *keeper = (struct char_data *) me;
  int shop_nr;

  for(shop_nr=0 ; shop_index[shop_nr].keeper != keeper->nr; shop_nr++);

  if (type == SPEC_INIT)
  {
      init_shop_keeper(keeper, &shop_index[shop_nr]);
      citizen(me, 0, 0, "", type);
      return (TRUE);
  }

  if (!cmd) {
    if (keeper->specials.fighting) {
      return(citizen(me, 0, 0, "", type));
    }
  }
  
  if((cmd == 56) && (ch->in_room == shop_index[shop_nr].in_room))
    /* Buy */
    {
      shopping_buy(arg,ch,keeper,shop_nr);
      return(TRUE);
    }
  
  if((cmd ==57 ) && (ch->in_room == shop_index[shop_nr].in_room))
    /* Sell */
    {
      shopping_sell(arg,ch,keeper,shop_nr);
      return(TRUE);
    }
  
  if((cmd == 58) && (ch->in_room == shop_index[shop_nr].in_room))
    /* value */
    {
      shopping_value(arg,ch,keeper,shop_nr);
      return(TRUE);
    }
  
  if((cmd == 59) && (ch->in_room == shop_index[shop_nr].in_room))
    /* List */
    {
      shopping_list(arg,ch,keeper,shop_nr);
      return(TRUE);
    }
  
  if ((cmd == 25) || (cmd==70))   /* Kill or Hit */
    {
      only_argument(arg, argm);
      
      if (keeper == get_char_room(argm,ch->in_room))
	{
	  shopping_kill(arg,ch,keeper,shop_nr);
	  return(TRUE);
	}
    } else if ((cmd==84) || (cmd==207) || (cmd==172)) {   /* Cast, recite, use */
      act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
      return TRUE;
    }
  
  return(FALSE);
}

void boot_the_shops(void)
{
  char *buf;
  int temp;
  int count;
  FILE *shop_f;
  int rm;
  
  if (!(shop_f = fopen(SHOP_FILE, "r")))
    {
      perror("Error in boot shop\n");
      exit(0);
    }
  
  number_of_shops = 0;
  
  for(;;)
    {
      buf = fread_string(shop_f);
      if(*buf == '#')	/* a new shop */
	{
	  if(!number_of_shops)	/* first shop */
	    CREATE(shop_index, struct shop_data, 1);
	  else
	      RECREATE(shop_index, struct shop_data, number_of_shops + 1);
	  
	  for(count=0;count<MAX_PROD;count++)
	    {
	      fscanf(shop_f,"%d \n", &temp);
	      if (temp >= 0)
		shop_index[number_of_shops].producing[count]=
		  real_object(temp);
	      else
		shop_index[number_of_shops].producing[count]= temp;
	    }
	  fscanf(shop_f,"%lf \n",
		 &shop_index[number_of_shops].profit_buy);
	  fscanf(shop_f,"%lf \n",
		 &shop_index[number_of_shops].profit_sell);
	  for(count=0;count<MAX_TRADE;count++)
	    {
	      fscanf(shop_f,"%d \n", &temp);
	      shop_index[number_of_shops].type[count] =
		(byte) temp;
	    }
	  shop_index[number_of_shops].no_such_item1 =
	    fread_string(shop_f);
	  shop_index[number_of_shops].no_such_item2 =
	    fread_string(shop_f);
	  shop_index[number_of_shops].do_not_buy =
	    fread_string(shop_f);
	  shop_index[number_of_shops].missing_cash1 =
	    fread_string(shop_f);
	  shop_index[number_of_shops].missing_cash2 =
	    fread_string(shop_f);
	  shop_index[number_of_shops].message_buy =
	    fread_string(shop_f);
	  shop_index[number_of_shops].message_sell =
	    fread_string(shop_f);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].temper1);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].temper2);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].keeper);
	  
	  rm = real_mobile(shop_index[number_of_shops].keeper);
	  if(rm <= 0)
	  {
	    char bufs[256];
	    sprintf(bufs, "Non-existent shopkeeper vnum = %d\n",
		    shop_index[number_of_shops].keeper);
	    log_msg(bufs);
	    rm = 0;
	  }
	  shop_index[number_of_shops].keeper = rm;
	  
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].with_who);
	  fscanf(shop_f,"%ld \n",
		 &shop_index[number_of_shops].in_room);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].open1);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].close1);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].open2);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].close2);
	  fscanf(shop_f,"%d \n",
		 &shop_index[number_of_shops].gold);
	  shop_index[number_of_shops].deficit_gold=
	    shop_index[number_of_shops].gold/10;
	  
	  number_of_shops++;
	}
      else 
	if(*buf == '$')	/* EOF */
	  break;
    }
  
  fclose(shop_f);
}

void assign_the_shopkeepers(void)
{
  int temp1;
  
  for(temp1=0 ; temp1<number_of_shops ; temp1++)
    if(shop_index[temp1].keeper)
      mob_index[shop_index[temp1].keeper].func = shop_keeper;
  
}
