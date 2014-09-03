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
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "race.h"
#include "trap.h"
#include "hash.h"
#include "utility.h"
#include "multiclass.h"
#include "act.h"
#include "handler.h"
#include "constants.h"
#include "modify.h"
#include "skills.h"
#include "fight.h"
#include "cmdtab.h"
#include "spelltab.h"
#include "spell_util.h"
#include "util_str.h"
#include "find.h"
#include "sblock.h"
#include "hero.h"
#include "recept.h"
#include "db.zonehelp.h"
#include "track.h"
#include "db.random.h"

#include "ansi.h"
#include "proto.h"
#include "varfunc.h"
#include "hero.h"

/* external functions we shouldn't have to declare... */
#ifdef __cplusplus
extern "C"

{
#endif
    int strcasecmp(const char*, const char*);
#ifdef __cplusplus
}
#endif

extern struct room_data *world;

#define HELP_LOG "help_request/unknown_topic.log"

/* intern functions */
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show);
void show_obj_to_char_info(struct obj_data *object, struct char_data *ch, int mode, bool show_info);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);

/* used for shapeshifter form skill */
int shifter_form_test(int position)
{
  static int test[] =
  {
    WEAR_HANDS, WEAR_FINGER_R, WEAR_FINGER_L, WEAR_WRIST_R, WEAR_WRIST_L,
    WEAR_SHIELD, WIELD, LOADED, HOLD, -1
  };
  int i=0;

  while (test[i]>=0) {
    if (position==test[i])
      return (TRUE);
    i++;
  }

  return (FALSE);
}

int shifter_plate_test(int position)
{
  static int test[] =
  {
    WEAR_BODY, WEAR_LEGS, WEAR_ARMS, -1
  };
  int i=0;

  while (test[i]>=0) {
    if (position==test[i])
      return (TRUE);
    i++;
  }

  return (FALSE);
}

void show_form(struct char_data *looker, struct char_data *lookee)
{
  char String[MAX_STRING_LENGTH];

  sprintf(String, "[%2d] %s", WIELD, where[WIELD]);
  send_to_char(String, looker);
  switch(lookee->specials.attack_type) {
    case TYPE_PIERCE:
      send_to_char("shapeshifter formed lances\n\r", looker);
      break;
    case TYPE_SLASH:
      send_to_char("shapeshifter formed blades\n\r", looker);
      break;
    case TYPE_CLAW:
      send_to_char("shapeshifter formed claws\n\r", looker);
      break;
    default:
      send_to_char("shapeshifter formed hammers\n\r", looker);
      break;
  }
}

//-----------------------------------------------------------------------
// show_equiped_text(char_data *, char_data *)
//
// Display the players equipment in a more role-playish format
//
// Will Lowrey		7/15/2002		Original Author
//-----------------------------------------------------------------------
void show_equiped_text(struct char_data *looker, struct char_data *lookee)
{
  int i, i_offset, ifound, iPlateTest;
  char cLookText[MAX_STRING_LENGTH];

  //initialize variables
  ifound=FALSE;
  i_offset = 0;
  iPlateTest = 0;
  iPlateTest = affected_by_spell(lookee, SKILL_PLATE);

  //--------------
  //Light source:
  // <player name> is using <light name> to light <his/her> way.
  //--------------
  if (lookee->equipment[0])  {
      ifound = TRUE;
      i_offset += sprintf(cLookText, "%s is using %s to light %s way. \n\r", GET_NAME(lookee),
                            OBJ_SHORT(lookee->equipment[0]), HSHR(lookee));
  }

  //--------------
  //Fingers:
  //<His/Her> fingers provide a home for <ring 1 name> and <ring 2 name> <newline<
  //   OR
  // <His/Her> <rightleft> hand bears the weight of <ring 1/2 name> <newline>
  //    (only if they are wearing only one right)
  //--------------
  if (lookee->equipment[1] || lookee->equipment[2])  {
      ifound = TRUE;
      if (lookee->equipment[1] && lookee->equipment[2]) {
          i_offset += sprintf(cLookText + i_offset, "%s fingers provide a home for %s and %s. \n\r",
                        UHSHR(lookee), OBJ_SHORT(lookee->equipment[1]), OBJ_SHORT(lookee->equipment[2]));
      } else {
          if (lookee->equipment[1]) {
            i_offset += sprintf(cLookText + i_offset, "%s right hand provide a home for %s. \n\r",
                        UHSHR(lookee), OBJ_SHORT(lookee->equipment[1]));
          } else {
              i_offset += sprintf(cLookText + i_offset, "%s left hand provide a home for %s. \n\r",
                        UHSHR(lookee), OBJ_SHORT(lookee->equipment[2]));
          }

      }
  }

  //--------------
  //Neck:
  // <He/She> is wearing <neck 1 name> and <neck 2 name> around <his/her> neck.<newline>
  //--------------
  if (lookee->equipment[3] || lookee->equipment[4])  {
      ifound = TRUE;
      if (lookee->equipment[3] && lookee->equipment[4]) {
          i_offset += sprintf(cLookText + i_offset, "%s is wearing %s and %s around %s neck. \n\r",
                         UHSSH(lookee), OBJ_SHORT(lookee->equipment[3]),
                         OBJ_SHORT(lookee->equipment[4]), HSHR(lookee));
      } else {
          if (lookee->equipment[3]) {
              i_offset += sprintf(cLookText + i_offset, "%s is wearing %s around %s neck. \n\r",
                            UHSSH(lookee), OBJ_SHORT(lookee->equipment[3]), HSHR(lookee));

          } else {
              i_offset += sprintf(cLookText + i_offset, "%s is wearing %s around %s neck. \n\r",
                            UHSSH(lookee), OBJ_SHORT(lookee->equipment[4]), HSHR(lookee));
          }
      }
  }

  //--------------
  //Head:
  // <Name> wears <head name> upon <his/her> head.
  //--------------
  if (lookee->equipment[6]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "%s is wearing %s upon %s head. \n\r",
                          UHSSH(lookee), OBJ_SHORT(lookee->equipment[6]), HSHR(lookee));
  }

  //--------------
  //Body: on and about
  // <Name> protects <him/her>self with <on body> and is covered with <about body>.
  //--------------
  if (lookee->equipment[5] || lookee->equipment[12] || (iPlateTest > 0)) {
      ifound = TRUE;
      if (iPlateTest > 0) {
          if (lookee->equipment[12]) {
            i_offset += sprintf(cLookText + i_offset, " %s protects %sself with %s and is covered with %s. \n\r",
                               UHSSH(lookee), HMHR(lookee), "shapeshifter formed armor plating",
                               OBJ_SHORT(lookee->equipment[12]));
          } else {
            i_offset += sprintf(cLookText + i_offset, " %s protects %sself with %s. \n\r",
                               UHSSH(lookee), HMHR(lookee), "shapeshifter formed armor plating");
          }
      } else {
          if (lookee->equipment[5] && lookee->equipment[12]) {
              i_offset += sprintf(cLookText + i_offset, "%s protects %sself with %s and is covered with %s. \n\r",
                               UHSSH(lookee), HMHR(lookee), OBJ_SHORT(lookee->equipment[5]),
                               OBJ_SHORT(lookee->equipment[12]));
         } else {
              if (lookee->equipment[5]) {
                  i_offset += sprintf(cLookText + i_offset, "%s is protected by %s. \n\r",
                                UHSSH(lookee), OBJ_SHORT(lookee->equipment[5]));
              } else {
                i_offset += sprintf(cLookText + i_offset, "%s is covered by %s. \n\r",
                                UHSSH(lookee), OBJ_SHORT(lookee->equipment[12]));
              }
         }
      }
  }

  //--------------
  //Legs: (need to handle shifter plate)
  // <His/Her> legs are protected by <leg item>.
  //--------------
  if (lookee->equipment[7] || iPlateTest) {
      ifound = TRUE;
      if (iPlateTest > 0) {
        i_offset += sprintf(cLookText + i_offset, "%s legs are protected by %s. \n\r",
                         UHSHR(lookee), "shapeshifter formed armor leggings");
      } else {
        i_offset += sprintf(cLookText + i_offset, "%s legs are protected by %s. \n\r",
                         UHSHR(lookee), OBJ_SHORT(lookee->equipment[7]));
      }
  }

  //--------------
  //Feet:
  // <His/her> feet are covered by <feet item>.
  //--------------
  if (lookee->equipment[8]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "%s feet are covered by %s. \n\r",
                         UHSHR(lookee), OBJ_SHORT(lookee->equipment[8]));
  }

  //--------------
  //Waist:
  // Wrapped around <his/her> waist is <about waist>.
  //--------------
  if (lookee->equipment[13]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "Wrapped around %s waist is %s. \n\r",
                         HSHR(lookee), OBJ_SHORT(lookee->equipment[13]));
  }

  //--------------
  //Hands:
  // <He/She> wears <hand item> on <his/her> hands.
  //--------------
  if (lookee->equipment[9]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "%s wears %s on %s hands. \n\r",
                         UHSSH(lookee), OBJ_SHORT(lookee->equipment[9]), HSHR(lookee));
  }

  //--------------
  //Arms: (need to handle shifter plate)
  // Around <his/her> arms there are <arm item>.
  //--------------
   if (lookee->equipment[10] || iPlateTest) {
       ifound = TRUE;
       if (iPlateTest > 0) {
           i_offset += sprintf(cLookText + i_offset, "Around %s arms there are %s. \n\r",
                         HSHR(lookee), "shapeshifter formed gauntlets");
       } else {
           i_offset += sprintf(cLookText + i_offset, "Around %s arms there are %s. \n\r",
                         HSHR(lookee), OBJ_SHORT(lookee->equipment[10]));
       }
  }

  //--------------
  //Wrists:
  // <His/Her> wrist(s) <is/are> covered with <wrist 1> and <wrist 2>
  //--------------
  if (lookee->equipment[14] || lookee->equipment[15]) {
      ifound = TRUE;
      if (lookee->equipment[14] && lookee->equipment[15]) {
        i_offset += sprintf(cLookText + i_offset, "%s wrists are covered with %s and %s. \n\r",
                         UHSHR(lookee), OBJ_SHORT(lookee->equipment[14]), OBJ_SHORT(lookee->equipment[15]));
      } else {
          if (lookee->equipment[14]) {
              i_offset += sprintf(cLookText + i_offset, "%s wrist is covered with %s. \n\r",
                         UHSHR(lookee), OBJ_SHORT(lookee->equipment[14]));
          } else {
              i_offset += sprintf(cLookText + i_offset, "%s wrist is covered with %s. \n\r",
                         UHSHR(lookee), OBJ_SHORT(lookee->equipment[15]));
          }
      }
  }

  //--------------
  //Shield:
  // For additional protection, <he/she> carries %s.
  //--------------
  if (lookee->equipment[11]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "For additional protection, %s carries %s. \n\r",
                         HSSH(lookee), OBJ_SHORT(lookee->equipment[11]));
  }

  //--------------
  //Wield:
  //--------------
  if (lookee->equipment[16]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "%s is armed with %s.  \n\r",
                         GET_NAME(lookee), OBJ_SHORT(lookee->equipment[16]));
  }

  //--------------
  //Hold:
  //--------------
  if (lookee->equipment[17]) {
    ifound = TRUE;
    i_offset += sprintf(cLookText + i_offset, "%s is carrying %s in %s hand.  \n\r",
                         UHSSH(lookee), OBJ_SHORT(lookee->equipment[17]), HSHR(lookee));
  }

  //newline
  i_offset += sprintf(cLookText + i_offset, "\n\r");

  if (!ifound)
  {
    sprintf(cLookText, "%s is not wearing anything of note.\n\r", GET_NAME(lookee));
  }

  send_to_char_formatted(cLookText, looker);
}


void show_equiped(struct char_data *looker, struct char_data *lookee)
{
  int i, found;
  char String[MAX_STRING_LENGTH];

  found=FALSE;

  //loop through all of their equipment
  for (i=0; i<MAX_WEAR; i++) {
    if (i==WIELD && affected_by_spell(lookee, SKILL_FORM)) {
      found=TRUE;
      show_form(looker, lookee);
    }
    else if (shifter_plate_test(i) && affected_by_spell(lookee, SKILL_PLATE)) {
      found=TRUE;
      sprintf(String,"[%2d] %s", i, where[i]);
      send_to_char(String, looker);
      send_to_char("shapeshifter formed armor plating\n\r", looker);
    }
    else if (lookee->equipment[i]) {
      found=TRUE;
      if (CAN_SEE_OBJ(looker, lookee->equipment[i])) {
	sprintf(String,"[%2d] %s", i, where[i]);
	send_to_char(String,looker);
        show_obj_to_char(lookee->equipment[i], looker,1);
      } else if (looker==lookee) {
	sprintf(String,"[%2d] %s", i, where[i]);
	send_to_char(String,looker);
 	send_to_char("Something\n\r",looker);
      }
    } else {
	sprintf(String, "[%2d] %s %s\n\r", i,where[i], "Nothing");
	send_to_char(String, looker);
    }
  }
  if (!found)
    send_to_char("Nothing\n\r", looker);
}

/* Procedures related to 'look' */
void argument_split_2(char *argument, char *first_arg, char *second_arg)
{
  int look_at, found, begin;
  found = begin = 0;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
       		 const char *arg, struct obj_data *equipment[], int *j)
{
  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
	if (isname(arg, OBJ_NAME(equipment[(*j)])))
	  return(equipment[(*j)]);

  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word,i->keyword))
      return(i->description);

  return(0);
}

void show_obj_to_char_info(struct obj_data *object, struct char_data *ch, int mode, bool show_info)
{
    char buffer[MAX_STRING_LENGTH] = "";
    char tmp[20] = "";

    if (show_info) {
      sprintf(tmp, "[lvl %3d] ", (int)object->obj_flags.level);
      strcat(buffer, tmp);
    }

    if ((mode == 0) && OBJ_DESC(object))
	strcat(buffer, OBJ_DESC(object));
    else if (OBJ_SHORT(object) && ((mode == 1) || (mode == 2) ||
				   (mode==3) || (mode == 4)))
	strcat(buffer,OBJ_SHORT(object));
    else if (mode == 5) {
	if (object->obj_flags.type_flag == ITEM_NOTE)  	{
	    if (*OBJ_ACTION(object))
	    {
		strcat(buffer, "There is something written upon it:\n\r\n\r");
		strcat(buffer, OBJ_ACTION(object));
	    }  else {
		strcat(buffer, "It's blank.");
	    }
	} else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	    strcat(buffer,"You see nothing special..");
	}  else  {		/* ITEM_TYPE == ITEM_DRINKCON */
	    strcat(buffer, "It looks like a drink container.");
	}
    }

    if (mode != 3) {

      if (IS_OBJ_STAT(object,ITEM_TWO_HANDED))
	strcat(buffer," (two-handed)");

	if (IS_OBJ_STAT(object,ITEM_INVISIBLE))
	    strcat(buffer," (invisible)");

	if(IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch, CLASS_PALADIN))
	  {
	    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD))
	      strcat(buffer," (dark aura)");
	    if (IS_OBJ_STAT(object,ITEM_ANTI_EVIL))
	      strcat(buffer," (light aura)");
	    if (IS_OBJ_STAT(object,ITEM_MAGIC))
	      strcat(buffer," (blue aura)");
	  }
	if (IS_OBJ_STAT(object,ITEM_GLOW))
	  strcat(buffer," (glowing)");
	if (IS_OBJ_STAT(object,ITEM_HUM))
	  strcat(buffer," (humming)");
	if (object->obj_flags.type_flag == ITEM_ARMOR) {
	    if (object->obj_flags.value[0] <
		(object->obj_flags.value[1] / 4)) {
		strcat(buffer, " (falling apart)");
	    } else if (object->obj_flags.value[0] <
		       (object->obj_flags.value[1] / 3)) {
		strcat(buffer, " (much repair)");
	    } else if (object->obj_flags.value[0] <
		       (object->obj_flags.value[1] / 2)) {
		strcat(buffer, " (fair condition)");
	    } else if  (object->obj_flags.value[0] <
			object->obj_flags.value[1]) {
		strcat(buffer, " (good condition)");
	    } else {
		strcat(buffer, " (excellent condition)");
	    }
	}
    }

    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  show_obj_to_char_info(object, ch, mode, FALSE);
  return;
}

void show_mult_obj_to_char_info(struct obj_data *object, struct char_data *ch,
			   int mode, int num, bool show_info)
{
    char buffer[MAX_STRING_LENGTH] = "";
    char tmp[20] = "";

    if (show_info) {
      sprintf(tmp, "[lvl %3d] ", (int)object->obj_flags.level);
      strcat(buffer, tmp);
    }

    if ((mode == 0) && OBJ_DESC(object))
	strcat(buffer,OBJ_DESC(object));
    else if (OBJ_SHORT(object) && ((mode == 1) || (mode == 2) ||
				   (mode==3) || (mode == 4)))
	strcat(buffer,OBJ_SHORT(object));
    else if (mode == 5) {
	if (object->obj_flags.type_flag == ITEM_NOTE)
	{
	    if (*OBJ_ACTION(object))
	    {
		strcat(buffer, "There is something written upon it:\n\r\n\r");
		strcat(buffer, OBJ_ACTION(object));
		page_string(ch->desc, buffer, 1);
	    }  else
		act("It's blank.", FALSE, ch,0,0,TO_CHAR);
	    return;
	} else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	    strcat(buffer,"You see nothing special..");
	}  else  {		/* ITEM_TYPE == ITEM_DRINKCON */
	    strcat(buffer, "It looks like a drink container.");
	}
    }

    if (mode != 3) {

	if (IS_OBJ_STAT(object,ITEM_INVISIBLE))
	    strcat(buffer," (invisible)");

	if (IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch,CLASS_PALADIN))
	{
	    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD))
		strcat(buffer," (dark aura)");
	    if (IS_OBJ_STAT(object,ITEM_ANTI_EVIL))
		strcat(buffer," (light aura)");
	    if (IS_OBJ_STAT(object,ITEM_MAGIC))
		strcat(buffer," (blue aura)");
	}

	if (IS_OBJ_STAT(object,ITEM_GLOW))
	    strcat(buffer," (glowing)");

	if (IS_OBJ_STAT(object,ITEM_HUM))
	    strcat(buffer," (humming)");
    }

    if (num>1) {
	sprintf(tmp,"[%d]", num);
	strcat(buffer, tmp);
    }

    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);
}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  show_mult_obj_to_char_info(object, ch, mode, num, FALSE);
  return;
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1, num;
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];

  cond_top = 0;

  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
	for (k=0;(k<cond_top&& !found);k++) {
	  if (cond_top>0) {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(OBJ_DESC(i) && OBJ_DESC(cond_ptr[k]) &&
		 !strcmp(OBJ_DESC(i),OBJ_DESC(cond_ptr[k])))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	if ((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) {
       	  num = number(1,100);
       	  if (ch->skills && num < ch->skills[SKILL_SEARCH].learned/10)
	    show_obj_to_char(i,ch,0);
        } else {
	  show_obj_to_char(i,ch,0);
	}
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if ((ITEM_TYPE(cond_ptr[k]) == ITEM_TRAP) &&
	  (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) {
	num = number(1,100);
	if (ch->skills && num < ch->skills[SKILL_SEARCH].learned/10)
	  if (cond_tot[k] > 1) {
	    sprintf(buf,"[%2d] ",Inventory_Num++);
	    send_to_char(buf,ch);
	    show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	  } else {
	    show_obj_to_char(cond_ptr[k],ch,0);
	  }
      } else {
	if (cond_tot[k] > 1) {
	  sprintf(buf,"[%2d] ",Inventory_Num++);
	  send_to_char(buf,ch);
	  show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	} else {
	  show_obj_to_char(cond_ptr[k],ch,0);
	}
      }
    }
  }
}

void list_obj_in_heap_info(struct obj_data *list, struct char_data *ch, bool show_info)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];

  int Num_Inventory = 1;
  cond_top = 0;

  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
        for (k=0;(k<cond_top && !found && !show_info);k++) {
          if (cond_top>0) {
            if ((i->item_number == cond_ptr[k]->item_number) &&
		(OBJ_SHORT(i) && OBJ_SHORT(cond_ptr[k]) &&
		 (!strcmp(OBJ_SHORT(i),OBJ_SHORT(cond_ptr[k]))))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	show_obj_to_char_info(i,ch,2,show_info);
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      sprintf(buf,"[%2d] ",Num_Inventory++);
      send_to_char(buf,ch);
      if (cond_tot[k] > 1) {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char_info(cond_ptr[k],ch,2,cond_tot[k],show_info);
      } else {
	show_obj_to_char_info(cond_ptr[k],ch,2,show_info);
      }
    }
  }
}

void list_obj_in_heap(struct obj_data *list, struct char_data *ch)
{
  list_obj_in_heap_info(list, ch, FALSE);
  return;
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show) {
  char buf[MAX_STRING_LENGTH];
  int Num_In_Bag = 1;
  struct obj_data *i;
  bool found;

  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) {
    if (CAN_SEE_OBJ(ch,i)) {
      sprintf(buf,"[%2d] ",Num_In_Bag++);
      send_to_char(buf,ch);
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}



void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH], buf[80];
  int found=FALSE, percent;
  struct obj_data *tmp_obj;

  if (mode == 0)
    {
      if (!IS_GOD(ch))
	if ((IS_AFFECTED(i, AFF_HIDE))  && !CAN_SEE(ch,i))
	  {
	    if (IS_AFFECTED(ch, AFF_SENSE_LIFE) &&  !IS_GOD(i))
	      send_to_char("You sense hidden life.\n\r", ch);
	    return;
	  }

      if(!PERL(i) || GET_POS(i) != i->specials.default_pos)
	{
	  if (!IS_NPC(i))
	    {
	      strcpy(buffer, GET_NAME(i));
	      strcat(buffer," ");
	      if (GET_TITLE(i))
		strcat(buffer,GET_TITLE(i));
	    }
	  else
	    strcpy(buffer, GET_NAME(i));

	  if ( IS_AFFECTED(i,AFF_INVISIBLE))
	    strcat(buffer," (invisible)");
	  if ( IS_SET(i->specials.mob_act, ACT_POLYSELF))
	    strcat(buffer," (polymorphed)");
	  if ((!(i->desc)) && (!IS_NPC(i)))
	    strcat(buffer," (linkdead)");
	  if ( IS_AFFECTED(i,AFF_HIDE))
	    strcat(buffer," (hidden)");
	  if ( IS_AFFECTED(i,AFF_CHARM))
	    strcat(buffer," (charmed)");
	  if ( i->invis_level > 0 && IS_GOD(ch))
	    {
	      sprintf(buf, " (I: %d)", i->invis_level);
	      strcat(buffer, buf);
	    }
	  if(IS_AFK(i))
	    strcat(buffer, " (AFK)");
	  if(IS_WRITING(i))
	    strcat(buffer, " (WRITING)");

	  switch(GET_POS(i))
	    {
	    case POSITION_STUNNED:
	      strcat(buffer," is lying here, stunned.");
	      break;
	    case POSITION_INCAP:
	      strcat(buffer," is lying here, incapacitated.");
	      break;
	    case POSITION_MORTALLYW:
	      strcat(buffer," is lying here, mortally wounded.");
	      break;
	    case POSITION_DEAD:
	      strcat(buffer," is lying here, dead.");
	      break;
	    case POSITION_MOUNTED:
	      if (MOUNTED(i))
		{
		  strcat(buffer, " is here, riding ");
		  strcat(buffer, PERS(MOUNTED(i), ch));
		  strcat(buffer, ".");
		}
	      else
		strcat(buffer, " is standing here.");

	      break;
	    case POSITION_STANDING:
	      strcat(buffer," is standing here.");
	      break;
	    case POSITION_SITTING:
	      strcat(buffer," is sitting here.");
	      break;
	    case POSITION_RESTING:
	      if(!IS_AFFECTED(i,AFF_MEDITATE))
		strcat(buffer," is resting here.");
	      else
		strcat(buffer," is resting here, meditating with nature.");

	      break;
	    case POSITION_SLEEPING:
	      strcat(buffer," is sleeping here.");
	      break;
	    case POSITION_FIGHTING:
	      if (i->specials.fighting)
		{

		  strcat(buffer," is here, fighting ");
		  if (i->specials.fighting == ch)
		    strcat(buffer," YOU!");
		  else
		    {
		      if (i->in_room == i->specials.fighting->in_room)
			strcat(buffer, PERS(i->specials.fighting, ch));
		      else
			strcat(buffer, "someone who has already left.");
		    }
		}
	      else		/* NIL fighting pointer */
		strcat(buffer," is here struggling with thin air.");
	      break;
	    default:
	      strcat(buffer," is floating here.");
	      break;
	    }

	  if (IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch, CLASS_PALADIN))
	    {
	      if(IS_EVIL(i))
		strcat(buffer," (dark aura)");
	      else if(IS_GOOD(i))
		strcat(buffer," (light aura)");
	    }

	  strcat(buffer,"\n\r");
	  if (TRUST(ch) >= i->invis_level)
	    {
#if POOFIN
	      if(IS_SET(i->specials.flags, PLR_MASK))
		act(i->specials.poofin,FALSE,ch,0,i,TO_CHAR);
	      else
#endif
		send_to_char_formatted(buffer, ch);
	    }
	}

      else
	{			/* npc with long */
	  if (IS_AFFECTED(i,AFF_INVISIBLE))
	    strcpy(buffer,"(invisible) ");
	  else
	    *buffer = '\0';

	  if (IS_AFFECTED(i,AFF_HIDE))
	    strcpy(buffer,"(hidden) ");
	  else
	    *buffer = '\0';

	  if (IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch, CLASS_PALADIN))
	    {
	      if(IS_EVIL(i))
		strcat(buffer,"(dark aura) ");
	      else if(IS_GOOD(i))
		strcat(buffer,"(light aura) ");
	    }

	  strcat(buffer, ss_data(i->player.long_descr));

	  send_to_char_formatted(buffer, ch);
	}

      if (IS_SET(i->specials.mob_act, ACT_IT))
	act("$n is IT!", FALSE, i, 0, ch, TO_VICT);



      /*******************************************************************/
      /*                  SLICER CODE REVISION                           */
      /*                                                                 */
      /* The code below enables more combinations of affects to be imp-  */
      /* lemented without writing 2^n different if statements.           */
      /*                                                                 */
      /*******************************************************************/

      char* m_aff_flying = "$Cbflying$CN, ";
      char* m_aff_sanctuary = "$Cwglowing$CN, ";
      char* m_aff_fireshield = "$Crblazing$CN, ";
      char* m_aff_illusion = "$Ccblurry$CN, ";
      char* m_aff_elecshield = "$Cyelectrifying$CN, ";
      char* m_aff_acidshield = "$Cberoding$CN, ";
      char* m_aff_coldshield = "$Ccfreezing$CN, ";
      char* m_aff_poisonshield = "$Cgvenemous$CN, ";
      char* m_aff_energyshield = "$Crenergizing$CN, ";
      char* m_aff_manashield = "$Cwvibrating$CN, ";
      char* m_aff_moveshield = "$Cyshimmering$CN, ";
      char* m_aff_vampshield = "$Crdripping$CN, ";
      char* m_aff_tolerance = "$Cmpulsating$CN, ";
      char m_message[MAX_STRING_LENGTH] = "     $n is ";
      unsigned int m_nIndex = 0;

      if ((IS_AFFECTED(i, AFF_FLYING | AFF_SANCTUARY | AFF_FIRESHIELD |
		       AFF_ILLUSION)) ||
	  (IS_AFFECTED2(i,AFF2_ELECSHIELD | AFF2_ACIDSHIELD |
			AFF2_POISONSHIELD | AFF2_COLDSHIELD |
			AFF2_ENERGYSHIELD | AFF2_MANASHIELD |
			AFF2_MOVESHIELD   | AFF2_VAMPSHIELD |
			AFF2_TOLERANCE )))
	{
	  if (IS_AFFECTED(i,AFF_FLYING))
	    { strcat(m_message, m_aff_flying);
	    }
	  if (IS_AFFECTED(i,AFF_SANCTUARY))
	    { strcat(m_message,m_aff_sanctuary);
	    }
	  if (IS_AFFECTED(i,AFF_FIRESHIELD))
	    { strcat(m_message, m_aff_fireshield);
	    }
	  if (IS_AFFECTED(i,AFF_ILLUSION))
	    { strcat(m_message, m_aff_illusion);
	    }
	  if (IS_AFFECTED2(i,AFF2_ELECSHIELD))
	    { strcat(m_message, m_aff_elecshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_ACIDSHIELD))
	    { strcat(m_message, m_aff_acidshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_COLDSHIELD))
	    { strcat(m_message, m_aff_coldshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_POISONSHIELD))
	    { strcat( m_message,m_aff_poisonshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_ENERGYSHIELD))
	    {strcat(m_message,m_aff_energyshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_MANASHIELD))
	    {strcat(m_message,m_aff_manashield);
	    }
	  if (IS_AFFECTED2(i,AFF2_MOVESHIELD))
	    {strcat(m_message,m_aff_moveshield);
	    }
	  if (IS_AFFECTED2(i,AFF2_VAMPSHIELD))
	    {strcat(m_message,m_aff_vampshield);
	    }
	  if (IS_AFFECTED2(i, AFF2_TOLERANCE))
	    {strcat(m_message,m_aff_tolerance);
	    }


	  //Fix end line.
#define m_max (strlen(m_message) - 2)
	  if ( m_message[m_max] == ',' )
	    m_message[m_max] = '.';


	  char *m_EndMessage = strrchr(m_message, ',');

	  if (m_EndMessage != NULL)
	    {
	      int found = 0;
	      unsigned int length = strlen(m_EndMessage);
	      unsigned int tIndex;
	      char *temp;
	      CREATE(temp, char, 25);

	      //setup a temp buffer
	      for (tIndex = 0, m_nIndex = 2; m_nIndex < length;
		   m_nIndex++, tIndex++)
		{
		  temp[tIndex] = m_EndMessage[m_nIndex];
		}
	      temp[tIndex] = '\0';

	      CREATE(m_EndMessage, char, 25);

	      //insert and statement
	      m_EndMessage[0] = ' ';
	      m_EndMessage[1] = 'a';
	      m_EndMessage[2] = 'n';
	      m_EndMessage[3] = 'd';
	      m_EndMessage[4] = ' ';

	      //replace temp buffer with correct End Message
	      for (tIndex = 0, m_nIndex = 5; tIndex < strlen(temp);
		   m_nIndex++, tIndex++)
		{
		  m_EndMessage[m_nIndex] = temp[tIndex];
		}
	      FREE(temp);
	      m_EndMessage[m_nIndex] = '\0';

	      //find first coma
	      for (m_nIndex = strlen(m_message) - 1; m_nIndex > 0; m_nIndex--)
		{
		  if (m_message[m_nIndex] == ',')
		    {
		      found = m_nIndex;
		      break;
		    }
		}

	      //now insert EndMessage at location found.
	      for (tIndex = 0; tIndex < strlen(m_EndMessage) ;
		   found++, tIndex++)
		{
		  m_message[found] = m_EndMessage[tIndex];
		}
	      FREE(m_EndMessage);

	      m_message[found] = '\0';
	    }
	  act(m_message, FALSE, i, 0, ch, TO_VICT);
	}
    }

  else if (mode == 1)
    {
      if (TRUST(ch) < i->invis_level) {
	send_to_char("You do not see that here.\n\r", ch);
	return;
      }

      if (i->player.description)
	send_to_char_formatted(ss_data(i->player.description), ch);
      else {
	act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
      }

      if (MOUNTED(i))
	{
	  sprintf(buffer,"$n is mounted on %s.", PERS(ch, MOUNTED(i)));
	  act(buffer, FALSE, i, 0, ch, TO_VICT);
	}

      if (RIDDEN(i))
	{
	  sprintf(buffer,"$n is ridden by %s.", PERS(ch, RIDDEN(i)));
	  act(buffer, FALSE, i, 0, ch, TO_VICT);
	}


      /* Show a character to another */

      if (GET_MAX_HIT(i) > 0)
	percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
      else
	percent = -1;		/* How could MAX_HIT be < 1?? */


      strcpy(buffer, PERS(i, ch));

      if (percent >= 100)
	strcat(buffer, " is in top form.\n\r");
      else if (percent >= 75)
	strcat(buffer, " is barely scratched.\n\r");
      else if (percent >= 50)
	strcat(buffer, " is weakening.\n\r");
      else if (percent >= 40)
	strcat(buffer, " is wounded.\n\r");
      else if (percent >= 30)
	strcat(buffer, " looks seriously injured.\n\r");
      else if (percent >= 25)
	strcat(buffer, " looks very bad.\n\r");
      else if (percent >= 0)
	strcat(buffer, " is in awful condition.\n\r");
      else
	strcat(buffer, " is about to die!!\n\r");

      send_to_char_formatted(buffer, ch);

      /* Wings/Gills visuals by smw */
      if (affected_by_spell(i, SKILL_WINGS))
	act("$e has powerful wings growing from $s back!", FALSE, i, 0, ch, TO_VICT);
      if (affected_by_spell(i, SKILL_GILLS))
	act("$e has scaly gills on $s neck!", FALSE, i, 0,ch, TO_VICT);

      //act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
	  act("\n\r", FALSE, i, 0, ch, TO_VICT);
      show_equiped(ch, i);
      if (HasClass(ch, CLASS_THIEF) && (ch != i) && (!IS_GOD(ch)))
	{
	  found = FALSE;
	  send_to_char
	    ("\n\rYou attempt to peek at the inventory:\n\r", ch);
	  for(tmp_obj = i->carrying; tmp_obj;
	      tmp_obj = tmp_obj->next_content)
	    {
	      if (CAN_SEE_OBJ(ch, tmp_obj) &&
		  (number(0,MAX_MORT) < GetMaxLevel(ch)))
		{
		  show_obj_to_char(tmp_obj, ch, 1);
		  found = TRUE;
		}
	    }
	  if (!found)
	    send_to_char("You can't see anything.\n\r", ch);
	}
      else if (IS_GOD(ch))
	{
	  send_to_char("Inventory:\n\r",ch);
	  for(tmp_obj = i->carrying; tmp_obj;
	      tmp_obj = tmp_obj->next_content) {
	    show_obj_to_char(tmp_obj, ch, 1);
	    found = TRUE;
	  }
	  if (!found)
	    send_to_char("Nothing\n\r",ch);
	}

    }
  else if (mode == 2)
    {
      /* Lists inventory */
      act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
      list_obj_in_heap(i->carrying,ch);
    }
}


void show_mult_char_to_char(struct char_data *i,
			    struct char_data *ch, int mode, int num)
{
    char buffer[MAX_STRING_LENGTH];
    char tmp[10];
    int  found, percent;
    struct obj_data *tmp_obj;

    if (mode == 0)
    {
	if (!IS_GOD(ch) && (TRUST(ch) >= i->invis_level))
	    if (IS_AFFECTED(i, AFF_HIDE) && !CAN_SEE(ch,i))
	    {
		if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
		    if (num==1)
			send_to_char("You sense hidden life.\n\r", ch);
		    else
			send_to_char("You sense hidden forms.\n\r", ch);
		return;
	    }

	if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos))
	{
	    /* A player char or a mobile without long descr, or not in default pos. */
	    strcpy(buffer,GET_NAME(i));
	    if (!IS_NPC(i))
	    {
		strcat(buffer," ");
		if (GET_TITLE(i))
		    strcat(buffer,GET_TITLE(i));
	    }

	    if ( IS_AFFECTED(i,AFF_INVISIBLE))
		strcat(buffer," (invisible)");
	    if ( IS_AFFECTED(i,AFF_CHARM))
		strcat(buffer," (charmed)");

	    switch(GET_POS(i)) {
	    case POSITION_STUNNED  :
		strcat(buffer," is lying here, stunned."); break;
	    case POSITION_INCAP    :
		strcat(buffer," is lying here, incapacitated."); break;
	    case POSITION_MORTALLYW:
		strcat(buffer," is lying here, mortally wounded."); break;
	    case POSITION_DEAD     :
		strcat(buffer," is lying here, dead."); break;
	    case POSITION_STANDING :
		strcat(buffer," is standing here."); break;
	    case POSITION_SITTING  :
		strcat(buffer," is sitting here.");  break;
	    case POSITION_RESTING  :
		if(!IS_AFFECTED(i,AFF_MEDITATE))
		    strcat(buffer," is resting here.");
		else
		    strcat(buffer," is resting here, meditating with nature.");
		break;
	    case POSITION_SLEEPING :
		strcat(buffer," is sleeping here."); break;
	    case POSITION_FIGHTING :
		if (i->specials.fighting)
		{
		    strcat(buffer," is here, fighting ");
		    if (i->specials.fighting == ch)
			strcat(buffer," YOU!");
		    else
		    {
			if (i->in_room == i->specials.fighting->in_room)
			    strcat(buffer, PERS(i->specials.fighting, ch));
			else
			    strcat(buffer, "someone who has already left.");
		    }
		}
		else		/* NIL fighting pointer */
		    strcat(buffer," is here struggling with thin air.");
		break;
	    default:
		strcat(buffer," is floating here."); break;
	    }
	    if (IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch, CLASS_PALADIN))
	    {
		if(IS_EVIL(i))
		    strcat(buffer," (dark aura)");
		else if(IS_GOOD(i))
		    strcat(buffer," (light aura)");
	    }

	    if (num > 1)
	    {
		sprintf(tmp," [%d]", num);
		strcat(buffer, tmp);
	    }
	    strcat(buffer,"\n\r");
	    send_to_char(buffer, ch);
	}
	else
	{		/* npc with long */

	    if (IS_AFFECTED(i,AFF_INVISIBLE))
		strcpy(buffer,"(invisible) ");
	    else
		*buffer = '\0';

	    if (IS_AFFECTED(ch, AFF_SENSE_AURA) || HasClass(ch, CLASS_PALADIN))
	    {
		if(IS_EVIL(i))
		    strcat(buffer,"(dark aura) ");
		else if(IS_GOOD(i))
		    strcat(buffer,"(light aura) ");
	    }

	    strcat(buffer, ss_data(i->player.long_descr));

	    /* this gets a little annoying */

	    if (num > 1)
	    {
		while ((buffer[strlen(buffer)-1]=='\r') ||
		       (buffer[strlen(buffer)-1]=='\n') ||
		       (buffer[strlen(buffer)-1]==' ')) {
		    buffer[strlen(buffer)-1] = '\0';
		}
		sprintf(tmp," [%d]\n\r", num);
		strcat(buffer, tmp);
	    }

	    send_to_char(buffer, ch);
	}

	if (IS_AFFECTED(i,AFF_SANCTUARY))
	    act("     $n glows white.", FALSE, i, 0, ch, TO_VICT);
	if (affected_by_spell(i, SPELL_FIRESHIELD)||IS_AFFECTED(i,AFF_FIRESHIELD))
	    act("     $n blazes.", FALSE, i, 0, ch, TO_VICT);
        if (IS_AFFECTED(i, AFF_ILLUSION))
            act("     $n blurs.",FALSE,i,0,ch,TO_VICT);
	if (IS_PC(i) && (IS_AFFECTED(i,AFF_FLYING)))
	    act("     $n flys.", FALSE, i, 0, ch, TO_VICT);

    }
    else if (mode == 1)
    {

	if (i->player.description)
	    send_to_char(ss_data(i->player.description), ch);
	else
	{
	    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
	}

	/* Show a character to another */

	if (GET_MAX_HIT(i) > 0)
	    percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
	else
	    percent = -1;	/* How could MAX_HIT be < 1?? */

	strcpy(buffer, PERS(ch, i));

	if (percent >= 100)
	    strcat(buffer, " is in top form.\n\r");
	else if (percent >= 75)
	    strcat(buffer, " is barely scratched.\n\r");
	else if (percent >= 50)
	    strcat(buffer, " is weakening.\n\r");
	else if (percent >= 40)
	    strcat(buffer, " is wounded.\n\r");
	else if (percent >= 30)
	    strcat(buffer, " looks seriously injured.\n\r");
	else if (percent >= 25)
	    strcat(buffer, " looks very bad.\n\r");
	else if (percent >= 0)
	    strcat(buffer, " is in awful condition.\n\r");
	else
	    strcat(buffer, " is about to die!!!\n\r");

	send_to_char(buffer, ch);

	//act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
	act("\n\r", FALSE, i, 0, ch, TO_VICT);
    show_equiped(ch, i);
	if ((HasClass(ch, CLASS_THIEF)) && (ch != i))
	{
	    found = FALSE;
	    send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
	    for(tmp_obj = i->carrying; tmp_obj;
		tmp_obj = tmp_obj->next_content)
	    {
		if (CAN_SEE_OBJ(ch,tmp_obj) &&
		    (number(0,MAX_MORT) < GetMaxLevel(ch)))
		{
		    show_obj_to_char(tmp_obj, ch, 1);
		    found = TRUE;
		}
	    }
	    if (!found)
		send_to_char("You can't see anything.\n\r", ch);
	}

    }
    else if (mode == 2)
    {
    	/* Lists inventory */
	act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
	list_obj_in_heap(i->carrying,ch);
    }
}


void list_char_in_room(struct char_data *list, struct char_data *ch)
{
    struct char_data *i, *cond_ptr[50];
    int k, cond_top, cond_tot[50], found=FALSE;

    cond_top = 0;

    for (i=list; i; i = i->next_in_room) {
	if (ch!=i && !RIDDEN(i) &&
	    (CAN_SEE(ch, i) ||
	     (IS_AFFECTED(i, AFF_HIDE) && IS_AFFECTED(ch, AFF_SENSE_LIFE))))
	{
	    if ((cond_top< 50) && !MOUNTED(i) && !IS_PC(i))
	    {
		found = FALSE;
		for (k=0;(k<cond_top&& !found);k++)
		{
		    if (i->nr == cond_ptr[k]->nr &&
			(GET_POS(i) == GET_POS(cond_ptr[k])) &&
			(AFF_FLAGS(i)==AFF_FLAGS(cond_ptr[k])) &&
			(AFF2_FLAGS(i)==AFF2_FLAGS(cond_ptr[k])) &&
			(i->specials.fighting == cond_ptr[k]->specials.fighting) &&
			!strcmp(GET_NAME(i), GET_NAME(cond_ptr[k])))
		    {
			cond_tot[k] += 1;
			found=TRUE;
		    }
		}
		if (!found) {
		    cond_ptr[cond_top] = i;
		    cond_tot[cond_top] = 1;
		    cond_top+=1;
		}
	    } else {
		show_char_to_char(i,ch,0);
	    }
	}
    }

    if (cond_top) {
	for (k=0; k<cond_top; k++) {
	    if (cond_tot[k] > 1) {
		show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	    } else {
		show_char_to_char(cond_ptr[k],ch,0);
	    }
	}
    }
}


void list_char_to_char(struct char_data *list, struct char_data *ch, int mode)
{
    struct char_data *i;

    for (i = list; i ; i = i->next_in_room)
    {
	if ((ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
			(CAN_SEE(ch,i) && (!IS_AFFECTED(i, AFF_HIDE) &&
					   (!IS_GOD(ch))))))
	    show_char_to_char(i,ch,0);
    }
}




int can_see_linear(struct char_data *ch, struct char_data *targ, int *rng,
		   int *dr)
{
    int i, max_range = 6, range = 0;
    struct char_data *tch;
    room_num rm;

    for (i = 0; i < NUM_OF_DIRS; i++)
    {
	rm = ch->in_room;
	range = 0;
	while (range < max_range)
	{
	    range++;
	    if (clearpath(ch, rm, i))
	    {
		rm = real_roomp(rm)->dir_option[i]->to_room;
		for (tch = real_roomp(rm)->people; tch;
		     tch = tch->next_in_room)
		{
		    if ((tch == targ) && (CAN_SEE(ch, tch)))
		    {
			*rng = range;
			*dr = i;
			return i;
		    }
		}
	    }
	}
    }

    return -1;
}

struct char_data *get_char_dir(struct char_data *ch, const char *arg, int *rf, int dr)
{
   int i, rm, max_range = 6, range = 0, n, n_sofar = 0;
   struct char_data *target;
   char *tmp, tmpname[MAX_STRING_LENGTH];

   strcpy(tmpname, arg);
   tmp = tmpname;

   if(!(n = get_number(&tmp))) return NULL;

   rm=ch->in_room;
   i=0;
   range = 0;
   while(range<max_range)
   {
     range++;
     if(clearpath(ch, rm, dr))
     {
       rm=real_roomp(rm)->dir_option[dr]->to_room;
       for(target=real_roomp(rm)->people;target;target=target->next_in_room)
       {
         if((isname(tmp, GET_IDENT(target))) && (CAN_SEE(ch, target)))
         {
           n_sofar++;
           if(n_sofar==n)
           {
             *rf=range;
             return target;
           }
         }
       }
     } else {
         range = max_range+1;
     }
  }

  return NULL;
}

struct char_data *get_char_linear(struct char_data *ch, const char *arg,
				  int *rf, int *df)
/* Returns direction if can see, -1 if not */
{
   int i, rm, max_range = 6, range = 0, n, n_sofar = 0;
   struct char_data *spud;
   char *tmp, tmpname[MAX_STRING_LENGTH];

   strcpy(tmpname, arg);
   tmp = tmpname;
   if (!(n = get_number(&tmp))) return NULL;

   /* This routine counts folks in your room */
   rm = ch->in_room;
   i = 0;
   range = 0;
   for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
      if ((isname(tmp, GET_IDENT(spud)))&&(CAN_SEE(ch,spud))) {
         n_sofar++;
         if (n_sofar==n) {
            *rf = range;
            *df = i;
            return spud;
         }
      }
   }

   for (i=0;i<6;i++) {
      rm = ch->in_room;
      range = 0;
      while (range<max_range) {
         range++;
         if (clearpath(ch, rm,i)) {
            rm = real_roomp(rm)->dir_option[i]->to_room;
            for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
               if ((isname(tmp, GET_IDENT(spud)))&&(CAN_SEE(ch,spud))) {
                  n_sofar++;
                  if (n_sofar==n) {
                     *rf = range;
                     *df = i;
                     return spud;
                  }
               }
            }
         } else {
            range = max_range+1;
         }
      }
   }
   return NULL;
}

void do_pkill(struct char_data *ch, char *arg, int cmd) {
    char choice[MAX_INPUT_LENGTH];
    char choice2[MAX_INPUT_LENGTH];
    char buf[80];
    int help=1;
    struct char_data *target, *target_inf;
    struct char_data *ch_inf=ch;

    while(*arg==' ') arg++;

    if(IS_POLY_PC(ch))
	ch_inf=real_character(ch);

    only_argument(arg, choice);
    arg = one_argument(arg, choice2);
    if(is_abbrev(choice, "on")) {
	if(IS_GOD(ch_inf)) {
	    send_to_char("God's aren't supposed to go around killing players\n\r", ch);
	    return;
	}
	if(!ch_inf->player.guildinfo.inguild()) {
	    REMOVE_BIT(ch->player.pkillinfo.flags, CAN_PKILL);
	    send_to_char("You must be in a clan to put on pkill\n\r", ch);
	    return;
	}
	SET_BIT(ch_inf->player.pkillinfo.flags, CAN_PKILL);
	send_to_char("Your pkill flag has now been set on.\n\r"
		     "You must get a god to remove your flag.\n\r", ch);
	help=0;
    }

    if(is_abbrev(choice2, "off") && IS_GOD(ch) && TRUST(ch)>=7) {
	arg = one_argument(arg, choice);
	if(!*choice) {
	    target_inf = target = ch;
	    send_to_char("You must pick a target\n\r", ch);
	    help = 0;
	} else if ((target_inf = target = get_char_vis(ch, choice)) == NULL) {
	    send_to_char("I don't see that person here \n\r",ch);
	    help = 1;
	}
	if(help) {
	    if(IS_POLY_PC(target)) target_inf=real_character(target);
	    REMOVE_BIT(target_inf->player.pkillinfo.flags, CAN_PKILL);
	    send_to_char("Your pkill flag has been set off.\n\r", target);
	    if(target_inf != ch)
		sprintf(buf, "You have set %s's pkill flag off.\n\r", GET_REAL_NAME(target));
	    send_to_char(buf, ch);
	    help = 0;
	}
    }

    if(is_abbrev(choice, "stat")) {
	if(!ch->player.guildinfo.inguild())
	    REMOVE_BIT(ch->player.pkillinfo.flags, CAN_PKILL);
	sprintf(buf, "Your pkill score is $CW%i$CN.\n\r"
		     "You have been killed $CW%i$CN times.\n\r",
		     ch->player.pkillinfo.count,
		     ch->player.pkillinfo.killed);
	if(IS_SET(ch_inf->player.pkillinfo.flags, CAN_PKILL))
	    strcat(buf, "Your pkill flag is $CWON$CN\n\r");
	else
	    strcat(buf, "Your pkill flag is $CWOFF$CN\n\r");
	send_to_char_formatted(buf, ch);
	help=0;
    }

    if(is_abbrev(choice, "clan") && IS_GOD(ch) && (TRUST(ch) >= 7)) {
        PKILLABLE = 1;
        send_to_char("Pkill is now turned on for players.\n\r", ch);
	log_msg("Pkill turned on.");
	do_system(ch, "$CWClan Pkill turned on. Enjoy.$CN", 0);
        help=0;
    }

    if(is_abbrev(choice, "arena") && IS_GOD(ch_inf) && (TRUST(ch_inf) >= 7)) {
        PKILLABLE = 2;
        send_to_char("Arena Pkill is now turned on for players.\n\r", ch);
        log_msg("Arena Pkill turned on.");
        do_system(ch, "$CWArena Pkill turned on. GO HOUSE!$CN", 0);
        help=0;
    }

    if(is_abbrev(choice, "none") && IS_GOD(ch_inf) && (TRUST(ch_inf) >= 7)) {
        PKILLABLE = 0;
        send_to_char("Pkill is now turned off.\n\r", ch);
        log_msg("Pkill turned off.");
        do_system(ch, "$CWPkill has been turned off.$CN", 0);
        help=0;
    }

    if(is_abbrev(choice2, "noloot") && IS_GOD(ch_inf)) {
	struct obj_data *obj;
	int change=0;
	arg = one_argument(arg, choice);
	only_argument(arg, choice2);
	if(is_abbrev(choice2, "on")) change=1;
	if(is_abbrev(choice2, "off")) change=2;
	if(is_abbrev(choice2, "stat")) change=3;
	if(!*choice || !change) {
	    sprintf(buf, "Syntax for nolooting items is: pkill noloot <name> <on|off|stat>\n\r");
	    send_to_char(buf, ch);
	    return;
	}

	obj = get_obj_vis(ch, choice);
	if(!obj) {
	    send_to_char("No object exists with that name.\n\r", ch);
	    return;
	}

	switch(change) {
	case 1:
	    obj->obj_flags.no_loot = 1;
	    sprintf(buf, "%s has been nolooted.\n\r", ss_data(obj->short_description));
	    send_to_char(buf, ch);
	    break;
	case 2:
	    obj->obj_flags.no_loot = 0;
	    sprintf(buf, "%s may now be looted.\n\r", ss_data(obj->short_description));
	    send_to_char(buf, ch);
	    break;
	case 3:
	    sprintf(buf, "%s can %sbe looted.\n\r", ss_data(obj->short_description), (!obj->obj_flags.no_loot)?"":"not ");
	    send_to_char(buf, ch);
	    break;
	default:
	    sprintf(buf, "Error in pkill noloot: %i\n\r", change);
	    send_to_char(buf, ch);
	}

	help=0;
    }

    if(help || is_abbrev(choice, "help")) {
	if(ch_inf->player.guildinfo.inguild()) {
	    send_to_char("pkill on:      Changes your pkill flag so that you may kill\n\r", ch);
	}
	send_to_char("pkill stat:    Shows your current pkill statistics\n\r", ch);
	if(IS_GOD(ch_inf)) {
	    if(TRUST(ch)>=7) {
		send_to_char("pkill off:     Changes your pkill flag so that you can't kill\n\r", ch);
		send_to_char("pkill clan:    Turns on clan pkill\n\r", ch);
	        send_to_char("pkill arena:   Turns on arena pkill\n\r", ch);
	        send_to_char("pkill none:    Turns off all pkill\n\r", ch);
	    }
	    send_to_char("pkill noloot <item> <on|off|stat>: makes an item noloot\n\r", ch);
	}
    }
}

void do_scan(struct char_data *ch, char *argument, int cmd)
{
    char *rng_desc[] = {
	"right here",
	"immediately",
	"nearby",
	"a little ways",
	"a ways",
	"a long ways",
	"quite far",
	"very far",
	"just visible"};
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    int sd, smin, smax, swt, i, max_range = 6, range, rm, nfnd;
    struct char_data *spud;

    argument_split_2(argument,arg1,arg2);
    sd = search_block(arg1, dirs, FALSE);
    if (sd==-1) {
	smin = 0;
	smax = 5;
	swt = 3;
	sprintf(buf,"$n peers intently all around.");
	sprintf(buf2,"You peer intently all around, and see :\n\r");
    } else {
	smin = sd;
	smax = sd;
	swt = 1;
	sprintf(buf,"$n peers intently %s.",dir_desc[sd]);
	sprintf(buf2,"You peer intently %s, and see :\n\r",dir_desc[sd]);
    }

    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf2,ch);
    nfnd = 0;
    /* Check in room first */
    for (spud=real_roomp(ch->in_room)->people;spud;spud=spud->next_in_room) {
	if (spud!=ch && CAN_SEE(ch, spud)) {
	    sprintf(buf,"%30s : right here\n\r", GET_NAME(spud));
	    send_to_char(buf,ch);
	    nfnd++;
	}
    }
    for (i=smin;i<=smax;i++) {
	rm = ch->in_room;
	range = 0;
	while (range<max_range) {
	    range++;
	    if (clearpath(ch, rm,i)) {
		rm = real_roomp(rm)->dir_option[i]->to_room;
		for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
		    if (spud!=ch && CAN_SEE(ch, spud)) {
			sprintf(buf,"%30s : %s %s\n\r",
				GET_NAME(spud),rng_desc[range],dir_desc[i]);
			send_to_char(buf,ch);
			nfnd++;
		    }
		}
	    } else {
		range = max_range + 1;
	    }
	}
    }
    if (nfnd==0) send_to_char("Absolutely no-one anywhere\n\r",ch);
    WAIT_STATE(ch,swt);
}

void do_farlook(struct char_data *ch, char *argument, int cmd)
{
   struct char_data *targ;
   char arg1[MAX_STRING_LENGTH];
   char *rng_desc[] = {
      "Immediately",
      "Immediately",
      "Nearby",
      "A ways",
      "A ways",
      "Far",
      "Far",
      "Very far",
      "Very far"};
   char buf[MAX_STRING_LENGTH];
   int rng, dr;

   only_argument(argument,arg1);
   targ = get_char_linear(ch,arg1,&rng,&dr);
   if (!targ) {
      send_to_char("You peer around, but can't find anyone of that description.\n\r",ch);
      return;
   } else {
      sprintf(buf,"%s %s, you spot %s.",rng_desc[rng],dir_desc[dr],GET_NAME(targ));
      send_to_char(buf,ch);
   }
   return;
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[256];
  int keyword_no;
  int j, bits, temp;
  int numberofexits=0;
  int door;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  struct room_direction_data	*exitdata;
  char *tmp_desc;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
  else if  (IS_DARK(real_roomp(ch->in_room)) && !IS_IMMORTAL(ch) && !IS_AFFECTED(ch, AFF_GREAT_SIGHT)
	    && !IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
    send_to_char("It is very dark in here...\n\r", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if(!strn_cmp(arg1, "at ", 3))
      {
	only_argument(argument+3, arg2);
	keyword_no = 7;
      }
    else if(!strn_cmp(arg1, "in ", 3))
      {
	only_argument(argument+3, arg2);
	keyword_no = 6;
      }
    else if(!str_cmp(arg1, "room"))
      {
	keyword_no = 9;
      }
    else if(!*arg1)
      {
	keyword_no = 8;
      }
    else
      {
	keyword_no = search_block(arg1, dirs, FALSE);
      }

    if (keyword_no == -1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }


    found = FALSE;
    tmp_object = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 : {
      struct room_direction_data	*exitp;
      exitp = EXIT(ch, keyword_no);
      if (exitp) {
	if (exitp->general_description) {
	  send_to_char(exitp-> general_description, ch);
	} else {
	  send_to_char("You see nothing special.\n\r", ch);
	}

	if (IS_SET(exitp->exit_info, EX_CLOSED) &&
	    (exitp->keyword)) {
	  if ((strcmp(fname(exitp->keyword), "secret")) &&
	      (!IS_SET(exitp->exit_info, EX_SECRET))) {
	    sprintf(buffer, "The %s is closed.\n\r",
		    fname(exitp->keyword));
	    send_to_char(buffer, ch);
	  }
	} else {
	  if (IS_SET(exitp->exit_info, EX_ISDOOR) &&
	      exitp->keyword) {
	    sprintf(buffer, "The %s is open.\n\r",
		    fname(exitp->keyword));
	    send_to_char(buffer, ch);
	  }
	}
      } else {
	send_to_char("You see nothing special.\n\r", ch);
      }
      if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
	struct room_data	*rp;
	sprintf(buffer,"You look %swards.\n\r", dirs[keyword_no]);
	send_to_char(buffer, ch);
	if (!exitp || !exitp->to_room) {
	  return;
	}
	rp = real_roomp(exitp->to_room);
	if (!rp) {
	  send_to_char("You see swirling chaos.\n\r", ch);
	  sprintf(buffer,"Character is in room %ld with FAULTY exits!",
		  ch->in_room);
	  slog(buffer);
	} else if(exitp) {
	  sprintf(buffer, "%ld look", exitp->to_room);
	  do_at(ch, buffer, 0);
	} else {
	  send_to_char("You see nothing special.\n\r", ch);
	}
      }
    }
      break;

      /* look 'in'	*/
    case 6: {
      if (*arg2) {
	/* Item carried */
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (bits) {		/* Found something */
	  if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) 	{
	    if (tmp_object->obj_flags.value[1] <= 0) {
	      act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	      temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
	      sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
		      fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
	      send_to_char(buffer, ch);
	    }
	  } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
	    if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
	      send_to_char(fname(OBJ_NAME(tmp_object)), ch);
	      switch (bits) {
	      case FIND_OBJ_INV :
		send_to_char(" (carried) : \n\r", ch);
		break;
	      case FIND_OBJ_ROOM :
		send_to_char(" (here) : \n\r", ch);
		break;
	      case FIND_OBJ_EQUIP :
		send_to_char(" (used) : \n\r", ch);
		break;
	      }
	      /* show (level) info on item listing if it is in donation chest
	       * --Mnemosync
	       */
	      if (!(strcmp(OBJ_NAME(tmp_object), "chest donate weapon"))
		  || !(strcmp(OBJ_NAME(tmp_object), "chest donate armor"))
		  || !(strcmp(OBJ_NAME(tmp_object), "chest donate misc"))) {
		list_obj_in_heap_info(tmp_object->contains, ch, TRUE);
	      } else {
		list_obj_in_heap_info(tmp_object->contains, ch, FALSE);
	      }
	    } else
	      send_to_char("It is closed.\n\r", ch);
	  } else {
	    send_to_char("That is not a container.\n\r", ch);
	  }
	} else {		/* wrong argument */
	  send_to_char("You do not see that item here.\n\r", ch);
	}
      } else {			/* no argument */
	send_to_char("Look in what?!\n\r", ch);
      }
    }
      break;

      /* look 'at'	*/
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}
	/*
	   Search for Extra Descriptions in room and items
	   */

	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2,
					 real_roomp(ch->in_room)->ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return;
	  }
	}

	/* extra descriptions in items */

	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2,
					       ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
	      }
	    }
	  }
	}
	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if (CAN_SEE_OBJ(ch, tmp_object)) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* Object In room */

	if (!found) {
	  for(tmp_object = real_roomp(ch->in_room)->contents;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if (CAN_SEE_OBJ(ch, tmp_object)) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) {		/* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5);
	  /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6);
	  /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\n\r", ch);
	}
      } else {
	/* no argument */
	send_to_char("Look at what?\n\r", ch);
      }
    }
      break;

      /* look ''		*/
    case 8 : {
       if (CheckColor(ch))
       	 send_to_char(ANSI_GREEN,ch);

       send_to_char(real_roomp(ch->in_room)->name, ch);

       if (CheckColor(ch))
       	 send_to_char(ANSI_NORMAL,ch);

       if (IS_GOD(ch))
       {
	   sprintf(buf, " [%ld] ", real_roomp(ch->in_room)->number); /* rm#'s */
	   send_to_char(buf, ch);
	   sprintbit((long) real_roomp(ch->in_room)->room_flags,
		     room_bits, buf);
	   send_to_char(buf, ch);
	   send_to_char("\n\rSector Type: ", ch);
	   send_to_char(sector_types[real_roomp(ch->in_room)->sector_type], ch);
       }
       send_to_char("\n\r", ch);
       if (!IS_SET(ch->specials.flags, PLR_BRIEF))
	   send_to_char(real_roomp(ch->in_room)->description, ch);

       if(IS_SET(ch->specials.flags, PLR_AUTOEXIT))
       {
	   *buf = '\0';
	   for (door = 0; door <= 5; door++)
	   {
	       exitdata = EXIT(ch,door);
	       if (exitdata)
	       {
		   if (exitdata->to_room != NOWHERE &&
		       (!IS_SET(exitdata->exit_info, EX_CLOSED) ||
			IS_GOD(ch))) {
		       numberofexits++;
		       if (numberofexits==1)
			   sprintf(buf + strlen(buf), "Exits: %s", dirs[door]);
		       else
			   sprintf(buf + strlen(buf), ", %s", dirs[door]);
		   }
	       }
	   }
	   strcat(buf, ".\n\r");
	   if (numberofexits > 0)
	   {
	       if (CheckColor(ch))
	       {
		   send_to_char(ANSI_ORANGE,ch);
		   send_to_char(buf,ch);
		   send_to_char(ANSI_NORMAL,ch);
	       }
	       else
		   send_to_char(buf,ch);
	   }
	   else
	   {
	       if (CheckColor(ch))
	       {
		   send_to_char(ANSI_RED,ch);
		   send_to_char("Exits: none!\n\r",ch);
		   send_to_char(ANSI_NORMAL,ch);
	       }
	       else
		   send_to_char("Exits: none!\n\r",ch);
	   }
       }

      track(ch);

      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);

    }
      break;

      /* wrong arg	*/
    case -1 :
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;

      /* look 'room' */
    case 9 : {
       if (CheckColor(ch))
	  send_to_char(ANSI_GREEN,ch);
      send_to_char(real_roomp(ch->in_room)->name, ch);
      if (CheckColor(ch))
       	 send_to_char(ANSI_NORMAL,ch);
      if IS_GOD(ch) {
	sprintf(buf, " [%ld] ", real_roomp(ch->in_room)->number); /* rm #'s */
	send_to_char(buf, ch);
	sprintbit((long) real_roomp(ch->in_room)->room_flags, room_bits, buf);
	send_to_char(buf, ch);
      }
      send_to_char("\n\r", ch);
      send_to_char(real_roomp(ch->in_room)->description, ch);

      track(ch);

      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);


    }
      break;
    }
  }
}

/* end of look */




void do_read(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH + 3];
  struct obj_data *obj;
  char objname[MAX_INPUT_LENGTH];
  struct obj_affected_type *aff;
  int i = 0;

  half_chop(argument, objname, buf);

  if (( obj= get_obj_vis(ch, objname)) && ITEM_TYPE(obj) == ITEM_SPELLBOOK)
  {
    sprintf(buf, "%s contains the text for:\n\r",OBJ_SHORT(obj) ? OBJ_SHORT(obj) : "A Book");
    for(aff = obj->affected, i = 0; i<MAX_OBJ_AFFECT ; aff++, i++) {
      if((aff->location == APPLY_BOOK_SPELL) && (aff->modifier != 0)) {
        sprintf(buf+strlen(buf), "  %s\n\r", spell_name(aff->modifier));
      }
    }
    send_to_char(buf, ch);
  } else {
     /* This is just for now - To be changed later.! */
     sprintf(buf,"at %s",argument);
     do_look(ch, buf, 15);
  }
}



void do_examine(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH+5];
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);

  one_argument(argument, name);

  if (!*name)
    {
      send_to_char("Examine what?\n\r", ch);
      return;
    }

  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
	(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\n\r", ch);
      sprintf(buf,"in %s",argument);
      do_look(ch,buf,15);
    }
  }
}



void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_STRING_LENGTH];
  char buf2[120];
  char brett[20];
  char *exits[] =
    {
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
      };
  struct room_direction_data	*exitdata;

  *buf = '\0';

  for (door = 0; door <= 5; door++) {
    exitdata = EXIT(ch,door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
        sprintf(buf2, "Character is in room %ld with FAULTY exits.",
                 ch->in_room);
        slog(buf2);
	/* don't print unless immortal */
	if (IS_GOD(ch)) {
	  sprintf(buf + strlen(buf), "%s - swirling chaos of #%ld\n\r",
	    	  exits[door], exitdata->to_room);
        }
      } else if (exitdata->to_room != NOWHERE &&
		 (!IS_SET(exitdata->exit_info, EX_CLOSED) ||
		  IS_IMMORTAL(ch))) {
	if (IS_DARK(real_roomp(exitdata->to_room)) && !IS_IMMORTAL(ch))
	  sprintf(buf + strlen(buf), "%s - Too dark to tell", exits[door]);
	else
	  sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	if (IS_SET(exitdata->exit_info, EX_CLOSED))
	  strcat(buf, " (closed)");
        if(IS_GOD(ch)) {
          sprintf(brett," [%ld]", exitdata->to_room);
          strcat(buf,brett);
        }
	strcat(buf, "\n\r");
      }
    }
  }

  send_to_char("Obvious exits:\n\r", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char_formatted("$CrNone.$CN\n\r", ch);
}


void show_score(struct char_data *ch, struct char_data *to)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH] = "";
  struct char_data *k=0;

  if(IS_SET(ch->specials.mob_act, ACT_IT)) {
    if(!CheckColor(to))
      send_to_char("You are IT!  Tag someone!\n\r", to);
    else {
      sprintf(buf,"%sYou are IT!%s  Tag someone!\n\r", ANSI_VIOLET,
	      ANSI_NORMAL);
      send_to_char(buf,to);
    }
  }

  if (!IS_GOD(ch) && !IS_NPC(ch)) {
    if (GET_COND(ch,DRUNK)>10)
 send_to_char_formatted("$CwYou are intoxicated.\n\r$CN", to);
    if ((GET_COND(ch,FULL)<2) && (GET_COND(ch,FULL) != -1))
      send_to_char_formatted("$Cw You are hungry...\n\r$CN", to);
    if ((GET_COND(ch,THIRST)<2) && (GET_COND(ch,THIRST) != -1))
      send_to_char_formatted("$Cw You are thirsty...\n\r$CN", to);
  }

  if (HowManyClasses(ch)==1 && !IS_GOD(ch))
  {
      sprintf(buf, "$CwYou are $CY%s $Cwa level $CG%d $Cc%s$CN. \n\r",
	      GET_NAME(ch),GetMaxLevel(ch), ClassTitles(ch,buf2));
      send_to_char_formatted(buf,to);
  }

  if (IS_GOD(ch))
  {
      sprintf(buf, "$CwYou are $CY%s$Cw, an immortal.\n\r$CN",GET_NAME(ch));
      send_to_char_formatted(buf, to);
  }

  if (IS_GOD(ch))
  {
      struct pos_struct* p;
      sprintf(buf, "$CwYou are a");
          for(p=pos_list;p->name;p++) {
            if(IS_SET(ch->player.godinfo.position,p->posbit)) {
            sprintf(buf + strlen(buf)," %s",p->name);
         }
      }
      sprintf(buf + strlen(buf),".\n\r");
      send_to_char_formatted(buf, to);
  }

  if (HowManyClasses(ch) > 1 && !IS_IMMORTAL(ch))
  {
      sprintf(buf,"$CwYou are $CY%s$Cw, a multiclass player.$CN\n\r",
	      GET_NAME(ch)), send_to_char_formatted(buf, to);
  }

  if (HowManyClasses(ch) > 1 || IS_GOD(ch))
  {
      sprintf(buf, "Your levels: ");
      if (GET_LEVEL(ch,MAGE_LEVEL_IND) > 0)
      {
	  sprintf(buf2,"$CCMa:%d$CN ",GET_LEVEL(ch,MAGE_LEVEL_IND));
	  strcat(buf,buf2);
      }
    if(GET_LEVEL(ch,CLERIC_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCCl:%d$CN ",GET_LEVEL(ch,CLERIC_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,WARRIOR_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCWa:%d$CN ",GET_LEVEL(ch,WARRIOR_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,THIEF_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCTh:%d$CN ",GET_LEVEL(ch,THIEF_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,PALADIN_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCKn:%d$CN ",GET_LEVEL(ch,PALADIN_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,DRUID_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCDr:%d$CN ",GET_LEVEL(ch,DRUID_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,PSI_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCPs:%d$CN ",GET_LEVEL(ch,PSI_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,RANGER_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCRa:%d$CN ",GET_LEVEL(ch,RANGER_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,SHIFTER_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCSh:%d$CN ",GET_LEVEL(ch,SHIFTER_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,MONK_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCMo:%d$CN ",GET_LEVEL(ch,MONK_LEVEL_IND));
	strcat(buf,buf2);
    }
    if(GET_LEVEL(ch,BARD_LEVEL_IND)>0)
    {
	sprintf(buf2,"$CCBa:%d$CN ",GET_LEVEL(ch,BARD_LEVEL_IND));
	strcat(buf,buf2);
    }
    strcat(buf,"\n\r");
    send_to_char_formatted(buf,to);
  }

  sprintf(buf,"You have:$Cr %d(%d) hit,$Cm %d(%d) mana and $Cb%d(%d) move points$CN.\n\r",
          GET_HIT(ch),GET_MAX_HIT(ch),
          GET_MANA(ch),GET_MAX_MANA(ch),
          GET_MOVE(ch),GET_MAX_MOVE(ch));
  send_to_char_formatted(buf,to);

  /*sprintf(buf,"Your ac is$Cr(-20 to 20) %d$CN with your dexterity bonus.\n\r",
 '        MAX(-20, MIN(20, (ch->points.armor+dex_app[GET_DEX(ch)].defensive)/10)));
*/
  sprintf(buf,"Your ac is$Cr(-100 to 20) %d$CN with your dexterity bonus.\n\r",
          MAX(-100, MIN(20, (ch->points.armor+dex_app[GET_DEX(ch)].defensive)/10)));
  send_to_char_formatted(buf,to);

  sprintf(buf, "You hit bare handed for$CG %dd%d$CN points of damage.\n\r",
          ch->specials.damnodice, ch->specials.damsizedice);
  send_to_char_formatted(buf, to);

  sprintf(buf, "Your hit bonus is$CG %d$CN and your damage bonus is $CG%d$CN.\n\r",
          GET_HITROLL(ch), GET_DAMROLL(ch));
  send_to_char_formatted(buf, to);

  if (!IS_GOD(ch))
  {
      sprintf(buf,"You have earned$CG %Ld exp$CN ",GET_EXP(ch));
      send_to_char_formatted(buf,to);

      if (!IS_GOD(ch))
	  sprintf(buf, "and you need$CG ");



	  SHOW_EXP_TO_LEVEL(Ma, MAGE_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Cl, CLERIC_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Wa, WARRIOR_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Th, THIEF_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Kn, PALADIN_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Dr, DRUID_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Ps, PSI_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Ra, RANGER_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Sh, SHIFTER_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Mo, MONK_LEVEL_IND);
	  SHOW_EXP_TO_LEVEL(Ba, BARD_LEVEL_IND);

	  strcat(buf,"$CNtill you next level.$CN\n\r");
	  send_to_char_formatted(buf,to);
  }

  sprintf(buf,"You own$Cy %d$CN gold and you are$CN ", GET_GOLD(ch));
  send_to_char_formatted(buf,to);

  switch(GET_POS(ch))
  {
  case POSITION_DEAD :
    send_to_char_formatted("$Cwumm, dead.$CN\n\r", to);
    break;
  case POSITION_MORTALLYW :
    send_to_char_formatted("$Cwwounded, get help.$CN\n\r", to);
    break;
  case POSITION_INCAP :
    send_to_char_formatted("$Cwincapacitated, seek a cleric.$CN\n\r", to);
    break;
  case POSITION_STUNNED :
    send_to_char_formatted("$Cwstunned. Just lie down.$CN\n\r", to);
    break;
  case POSITION_SLEEPING :
    send_to_char_formatted("$Cwsound asleep.$CN\n\r",to);
    break;
  case POSITION_RESTING  :
    send_to_char_formatted("$Cwresting comfortably.$CN\n\r",to);
    break;
  case POSITION_SITTING  :
    send_to_char_formatted("$Cwsitting down.$CN\n\r",to);
    break;
  case POSITION_FIGHTING :
    if (ch->specials.fighting)
      act("$CRfighting$CY $N$Cw!$CN\n\r", FALSE, to, 0,
          ch->specials.fighting, TO_CHAR);
    else
      send_to_char_formatted("$Cwfighting absolutely nothing.$CN\n\r", to);
    break;
  case POSITION_STANDING :
    send_to_char_formatted("$Cwstanding.$CN\n\r",to);
    break;
  case POSITION_MOUNTED:
    if (MOUNTED(ch)) {
      send_to_char_formatted("$Cwriding on ", to);
      send_to_char_formatted(PERS(ch, MOUNTED(ch)), to);
      send_to_char_formatted(".$CN\n\r", to);
    } else {
      send_to_char_formatted("$Cwstanding.$CN\n\r", to);
      break;
    }
    break;
  default :
    send_to_char_formatted("$Cwabsolutely nowhere.$CN\n\r", to);
    break;
  }

  if(!IS_SET(ch->specials.flags, PLR_WIMPY))
    sprintf(buf,"$CgYou are being a brave adventurer today.$CN\n\r");
  else if(GET_WIMPY(ch))
    sprintf(buf,"$CyYou are a coward and will flee at$Cg %i$Cy hit points.$CN\n\r",
	    GET_WIMPY(ch));
  else
    sprintf(buf,"$CYYou are a coward.$CN\n\r");
  send_to_char_formatted(buf,to);

}

void do_gold(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  sprintf(buf,"You carry $Cy%d$CN gold and have $Cy%d$CN in the bank.\n\r", GET_GOLD(ch), GET_BANK(ch));
  send_to_char_formatted(buf, ch);
}

void do_score(struct char_data *ch, char *argument, int cmd)
{
    show_score(ch,ch);
}


void do_time(struct char_data *ch, char *argument, int cmd)
{
  char buf[100], *suf;
  int weekday, day;

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am") );

  weekday = ((30*time_info.month)+time_info.day+1) % 7;/* 30 days in a month */

  strcat(buf,weekdays[weekday]);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);

  day = time_info.day + 1;   /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s day of the month of %s.\n\r",
	  day,
	  suf,
	  month_name[time_info.month],
	  time_info.year);

  send_to_char(buf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
  static char buf[100];
  static char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};

  if (OUTSIDE(ch)) {
    sprintf(buf,
	    "The sky is %s and %s.\n\r",
	    sky_look[weather_info.sky],
	    (weather_info.change >=0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
    send_to_char(buf,ch);
  } else
    send_to_char("You have no feeling about the weather at all.\n\r", ch);
}


void do_helptopics(struct char_data *ch, char *arg, int cmd)
{
  char	buf[256];
  int 	i;
  struct string_block sb;

  if (IS_NPC(ch))
    return;

  init_string_block(&sb);

  append_to_string_block(&sb,
		 "The following help topics are available to you:\n\r\n\r");

  for (i = 0; i<top_of_helpt; i++)
  {
    sprintf(buf, "%-25s%s", help_index[i].keyword,
	    (i%3) == 2 ? "\n\r" : "");
    append_to_string_block(&sb, buf);
  }

  sprintf(buf, "\n\rTotal number of help topics on system: %d\n\r", i);
  append_to_string_block(&sb, buf);

  page_string_block(&sb, ch);

  destroy_string_block(&sb);
}

void print_topic(struct char_data *ch, int seek_point)
{
  char buf[MAX_STRING_LENGTH];
  struct string_block sb;

  init_string_block(&sb);
  fseek(help_fl, help_index[seek_point].pos,0);
  for (;;)  {
    fgets(buf, 80, help_fl);
    if ( *buf == '#' )
      break;
    append_to_string_block(&sb, buf);
    append_to_string_block(&sb, "\r");
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
  return;
}

void do_help(struct char_data *ch, char *argument, int cmd)
{
  int one_word, chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  for(;isspace(*argument); argument++)  ;
  if(!strcasecmp(argument, "#")) {
    send_to_char("No help available.\n\r", ch);
    return;
  }

  one_word = 0;

  if (*argument)
  {
    if (!help_index)
    {
      send_to_char("No help available.\n\r", ch);
      return;
    }
    bot = 0;
    top = top_of_helpt;

    for (;;)
    {
      mid = (bot + top) / 2;
      minlen = strlen(argument);

      if (!(chk = str_cmp(argument, help_index[mid].keyword)))
      {
        print_topic(ch, mid);
        return;
      }
      else if (bot >= top)
      {
	struct string_block sb;

	init_string_block(&sb);
        append_to_string_block(&sb, "\r\nThe help topics matching that string are:\r\n");
	for ( ; (mid >= 0) && !(strn_cmp(argument, help_index[mid].keyword, minlen)); --mid);
	mid++;
	for ( ; !(strn_cmp(argument, help_index[mid].keyword, minlen)); mid++)  {
	  append_to_string_block(&sb, help_index[mid].keyword);
	  append_to_string_block(&sb, "\r\n");
	  ++one_word;
	}
	switch(one_word)  {
	  case 1:
	    destroy_string_block(&sb);
            print_topic(ch, --mid);
	    return;
	    break;
          case 0:
	    file_log(argument, HELP_LOG);
	    send_to_char("\r\nNo help on that topic.\r\n", ch);
	    destroy_string_block(&sb);
	    return;
  	    break;
          default:
	    page_string_block(&sb, ch);
	    destroy_string_block(&sb);
	    return;
	}
      }
      else if (chk > 0)
	bot = ++mid;
      else
	top = --mid;
    }
    return;
  }
  page_string(ch->desc, help, 1);
}

void do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no;
  struct command_info* c;

  if (IS_NPC(ch))
    return;

  send_to_char_formatted("$CBThe following privileged commands are available:$CN\n\r", ch);

  *buf = '\0';

  for(no = 1, c = cmd_list ; c->name ; c++)
    if (HAS_GCMD(ch,c->min_trust))
    {
      sprintf(buf + strlen(buf), "%-12s",c->name);
      if (!(no++ % 5))
	strcat(buf, "\n\r");
    }


  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}


int name_match(char *match,char *name)
{
        unsigned int i;
        for(i=0;i<strlen(match);i++) {
                if(LOWER(match[i])!=LOWER(name[i])) return 0;
                }
        return 1;
}


struct who_list_node {
  int lvl;
  char *text;
  struct who_list_node *nextptr;
};

struct who_list_node *head;

void who_list_init(void)
{
  head = NULL;
}

/* prepares a sorted list of players */

void who_list_add(int level, char *data)
{
  struct who_list_node *element_ptr,*iterator;

  element_ptr = new who_list_node;        /* create the new list node */
  element_ptr->nextptr = NULL;
  element_ptr->lvl = level;
  element_ptr->text = str_dup(data);

  if (head == NULL)  /* node automatically entered at head */
    {
      head = element_ptr;
      return;
    }
  if (level >= (head->lvl))
    { /* greater than first node on list */
      element_ptr->nextptr = head;
      head = element_ptr;
      return;
    }
  for (iterator=head;iterator;iterator=iterator->nextptr)
    { /* traverse the list */
      if (iterator->nextptr)
	if (level >= iterator->nextptr->lvl)
	  {
	    element_ptr->nextptr = iterator->nextptr;
	    iterator->nextptr = element_ptr; /* perform insertion sort */
	    return;
	  }
      if (iterator->nextptr == NULL)
	{ /* this is the last element, so add it here */
	  iterator->nextptr=element_ptr;
	  return;
	}
    }
}

/* print the sorted list */

void who_list_print(struct char_data *ch) {
  struct who_list_node *ptr;
  char buf[MAX_STRING_LENGTH];
  static char *godlevel[15] = {"ADMN","ADMN","AMBA","HLPR","WMSR","QSTR",
			       "BLDR","DRGN","CODR","SCDR","HQSR","HBDR",
			       "AMGR","PMGR","*IMP" };

  if (head == NULL) return;
  /* Clear the buffer */
  strcpy(buf,"");
  for (ptr=head;ptr;ptr=ptr->nextptr) {
 	if ((ptr->lvl > ABS_MAX_LVL) && (ptr->lvl < 660))
		if (CheckColor(ch))
 			sprintf(buf,"%s%s[%s%4s%s] ",
 				buf,
 				ANSI_NORMAL,
 				ANSI_RED,
 				godlevel[ptr->lvl - ABS_MAX_LVL],
 				ANSI_NORMAL);
		else
			sprintf(buf,"%s[%4s] ",
				buf,
				godlevel[ptr->lvl - ABS_MAX_LVL]);

 	if ((ptr->lvl <= ABS_MAX_LVL) && (ptr->lvl > 0))
		if (CheckColor(ch))
 			sprintf(buf,"%s%s[%s%4d%s] ",
 				buf,
 				ANSI_NORMAL,
 				ANSI_CYAN,
 				ptr->lvl,
 				ANSI_NORMAL);
		else
			sprintf(buf,"%s[%4d] ",
				buf,
				ptr->lvl);

    sprintf(buf, "%s%s", buf, ptr->text);
   }
  page_string(ch->desc, buf, 1);
}

void who_list_delete(void)
{
  struct who_list_node *ptr, *nextone;

  for (ptr = head;ptr;ptr=nextone) {
    nextone = ptr->nextptr;
    delete(ptr->text);
    delete(ptr);
  }
  head = NULL;
}


void do_who(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH], line1[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH]; /* the who output buffer */
    int count, gods, idle, mort, levels, title, hits, trust;
    int stats, dead, classes, zone, lcount, listed, live;
    int rname, flags, wanted;
    int singles, multis, both;
    int clan, guildnumber;
    char match[256];
    struct char_data* person;
    struct char_data* player;
    struct room_data* roomp;
    int godmort, godwho;
    int logall, builder;
	struct god_table {
	int bit;
	byte index;
	} godtable[] = {

		{PROJECT_MANAGER,13},
		{ASSISTANT_MANAGER,12},
		{HEAD_BUILDER,11},
		{HEAD_QUESTOR,10},
		{SENIOR_CODER,9},
		{CODER,8},
		{SENIOR_BUILDER,7},
		{BUILDER,6},
		{QUESTOR,5},
		{WEBMASTER,4},
		{AMBASSADOR,3},
		{NEWBIE_HELPER,2},
		{DAMNED,1},
		{NOPOSITION,0},
		{ 0 }
	};
	struct god_table* p;

    /* defaults */
    godmort = FALSE;		/* one of g/o specified */
    gods = TRUE;
    mort = TRUE;
    both = FALSE;		/* one of m/n specified */
    singles = TRUE;
    multis = TRUE;
    levels = FALSE;
    title = FALSE;
    hits = FALSE;
    stats = FALSE;
    dead = FALSE;
    live = TRUE;
    zone = FALSE;
    idle = FALSE;
    rname = FALSE;
    flags = FALSE;
    classes = -1;
    godwho = FALSE;
    wanted = FALSE;
    builder = FALSE;
    logall = FALSE;
    trust = FALSE;
    count = 0;
    lcount = 0;
    listed = 0;
    clan = FALSE;
    guildnumber = ch->player.guildinfo.inguild();

    /* if this is a god, deal with "-" arguments */
    while(isspace(*argument))
	argument++;

   // if(IS_GOD(ch) && (*argument == '-'))
    if(*argument == '-')
    {
      if(IS_GOD(ch))
         godwho = TRUE;
	while(*++argument && !isspace(*argument))
	{
          if(!godwho)
          {
            switch(*argument)
            {
            case 'c':  clan = TRUE; break;
            default:
                send_to_char("[-c] for a list of clan members online.", ch);
            }
          } else {
	    switch(*argument)
	    {
	    case 'c':   clan = TRUE;                    break;
	    case 'i':   idle = TRUE;			break;
	    case 'l':   levels = TRUE;			break;
	    case 't':   title = TRUE;			break;
	    case 'h':   hits = TRUE;			break;
	    case 's':   stats = TRUE;			break;
	    case 'd':   dead = TRUE;	live = FALSE;	break;
	    case 'z':   zone = TRUE;			break;
	    case 'r':   rname = TRUE;			break;
	    case 'a':	dead = TRUE;	live = TRUE;	break;
	    case 'f':	flags = TRUE;			break;
	    case '$':
		if (TRUST(ch) <= MAX_TRUST)
		    logall=FALSE; else logall=TRUE;
		break;
	    case 'b':	builder = TRUE;			break;
	    case 'm':
	    case 'n':
		if (!both)
		{
		    both = TRUE;
		    singles = FALSE;
		    multis = FALSE;
		}
		if(*argument == 'n')
		    singles = TRUE;
		else
		    multis = TRUE;
		break;
	    case 'g':
	    case 'o':
		if(!godmort)
		{
		    godmort = TRUE;
		    gods = FALSE;
		    mort = FALSE;
		}
		if(*argument == 'g')
		    gods = TRUE;
		else
		    mort = TRUE;
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
            case '7':
	    case '8':
	    case '9':
		if(classes == -1)
		    classes = 0;
		classes |= 1 << (*argument - '1');
		break;
	    case 'w':	wanted = TRUE;			break;
	    case '*':
		if (TRUST(ch) <= TRUST_GRGOD)
		    trust=FALSE; else trust=TRUE;
		    break;
	    default:
		send_to_char("[-]i=idle l=levels t=title h=hit/mana/move s=stats\n\r",ch);
	        if(TRUST(ch) > TRUST_GRGOD) {
		    send_to_char("[-]$=logall\n\r",ch);
		    send_to_char("[-]*=trust\n\r",ch);
		}
		send_to_char("[-]r=real names b=builder\n", ch);
		send_to_char("[-]d=linkdead g=God o=Mort a=dead&active\n\r",
			     ch);
	        send_to_char("[-]m=multis only n=singles only\n\r",ch);
		send_to_char("[1]Ma [2]Cl [3]Wa [4]Th [5]Dr [6]Ra [7]Ps [8]Pa [9]Sh\n\r", ch);
		send_to_char("--------\n\r", ch);
		return;
	    }
         }
	}
    }

    /*  check for an arg */
    only_argument(argument, match);
    if(!strcmp(match, "god") || !strcmp(match, "gods"))
    {
	mort = FALSE;
	match[0] = 0;
    }
    else if(!strcmp(match, "wanted"))
    {
	wanted = TRUE;
	match[0] = 0;
    }

    if((cmd == 234) || zone)
    {
        struct zone_data *zd;

	roomp = real_roomp(ch->in_room);
	zone = !roomp ? 0 : roomp->zone;

	zd = zone_table+zone;

	sprintf(buf, "Zone Name: %s\n\r", zd->name);
	send_to_char(buf, ch);
    }
    else
	zone = -1;

    strcpy(output, ""); /* init the string */

	who_list_init();

    if(!godwho) {    /* Want all Players to come after Player Title */
      strcpy(buf, "Players\n\r"
		     "------------------------------------\n\r");
		who_list_add(666,buf);
    } else
      strcat(output, "Players [God Version -? for help]\n\r---------\n\r");

    EACH_CHARACTER(iter, person) {

	if (!IS_PC(person) || (person->in_room == 3)) /* skip mobs or players behind polies */
	    continue;
	if (clan && (person->player.guildinfo.inguild() != guildnumber))
          continue;
	if (zone > -1) {
	    roomp = real_roomp(person->in_room);
	    if (!roomp || (roomp->zone != zone))
		continue;
	}
    /* Allow mortals to see all other mortals but run a check to see if Gods are visible. */

	if (IS_GOD(person) && (!CAN_SEE(ch, person))) /* can the God be seen */
	    continue;

	count++;

	if (!person->desc)  /* see if they pass any link dead tests */
	{
	    lcount++;
	    if(!dead)
		continue;
	}
	else if (!live)
	    continue;

	/* apply the filters */
	if (IS_GOD(person) && !IS_SET(person->specials.flags, PLR_NOWIZ))
	{
	    if (!gods)
		continue;
	}
	else if (!mort)
	    continue;

	if(HowManyClasses(person)==1)
	{
	    if (!singles)
		continue;
	}
	else if(!multis)
	    continue;

	if (!(classes & person->player.clss))
	    continue;

	player = real_character(person); /* find the real person here */

	if (match[0] && !name_match(match, GET_REAL_NAME(player)))
	    continue;

	if (wanted && !(person->specials.flags & (PLR_PKILLER|PLR_LOSER|PLR_THIEF)))
	    continue;

	listed++;

	if (cmd == 234)  /* ok ready to display them */
	{
	    char* name, namebuf[256];
	    if (IS_NPC(person))
		sprintf(name = namebuf, "%s (%s)",
			GET_NAME(person), GET_NAME(player));
	    else
		name = GET_NAME(person);

	    roomp = real_roomp(person->in_room);
	    if (IS_IMMORTAL(ch))
		sprintf(buf, "%-32s - %s [%ld]\n\r",
			name,
			roomp ? roomp->name : "Only gods know where",
			person->in_room);
	    else
		sprintf(buf, "%-40s - %s\n\r",
			name,
			roomp ? roomp->name : "Only gods know where");
	    /* strcat(output, buf); */
		who_list_add(661,buf);
	}
	else if (!godwho)
	{
	    if (CheckColor(ch))
	    {
		/* add on the rest of the info */
		if (player->player.guildinfo.inguild()==0)
			sprintf(buf, "%s%s%s %s",
				ANSI_YELLOW, GET_NAME(player), ANSI_NORMAL,
				ss_data(player->player.title));
		else if (IS_GOD(player) && !(IS_GOD(ch)))
               sprintf(buf, "%s%s%s %s %s[%sIMMORTAL%s%s]%s",
                    ANSI_YELLOW, GET_NAME(player), ANSI_NORMAL,
                    ss_data(player->player.title),
                    ANSI_RED,
                    ANSI_BRIGHT_GREEN,
				ANSI_NORMAL,
                    ANSI_RED,
                    ANSI_NORMAL);
		else
               sprintf(buf, "%s%s%s %s %s[%s%s%s%s%s]%s",
                    ANSI_YELLOW, GET_NAME(player), ANSI_NORMAL,
                    ss_data(player->player.title),
                    ANSI_RED,
                    ANSI_BRIGHT_GREEN,
		    IS_SET(player->player.pkillinfo.flags, CAN_PKILL)?"*":"",
                    guildname(player->player.guildinfo.inguild()),
				ANSI_NORMAL,
                    ANSI_RED,
                    ANSI_NORMAL);
	    }
	    else
	    {
		/* god title first */

                /* normal player info */
		if (player->player.guildinfo.inguild()==0)
			sprintf(buf, "%s %s",
				GET_NAME(player),
				ss_data(player->player.title));
		else if (IS_GOD(player) && !(IS_GOD(ch)))
			sprintf(buf, "%s %s [IMMORTAL]",
				GET_NAME(player),
				ss_data(player->player.title));
		else
			sprintf(buf, "%s %s [%s]",
				GET_NAME(player),
				ss_data(player->player.title),
				guildname(player->player.guildinfo.inguild()));
	    }

	    if (IS_GOD(ch))
		if (person->invis_level > 0)
		    sprintf(buf, "%s (I: %2d)", buf, person->invis_level);

	    if (IS_GOD(ch) || wanted)
	    {
		if (CheckColor(ch))
		{
		    if(IS_SET(person->specials.flags, PLR_LOSER))
			sprintf(buf, "%s %s(OPEN)%s", buf,
				ANSI_BRIGHT_RED, ANSI_NORMAL);
		    if(IS_SET(person->specials.flags, PLR_PKILLER))
			sprintf(buf, "%s %s(PKILLER)%s", buf,
				ANSI_BRIGHT_MAGENTA, ANSI_NORMAL);
		    if(IS_SET(person->specials.flags, PLR_THIEF))
			sprintf(buf, "%s %s(THIEF)%s", buf,
				ANSI_BRIGHT_BLUE, ANSI_NORMAL);
		}
		else
		{
		    if(IS_SET(person->specials.flags, PLR_LOSER))
			strcat(buf, " (OPEN)");
		    if(IS_SET(person->specials.flags, PLR_PKILLER))
			strcat(buf, " (PKILLER)");
		    if(IS_SET(person->specials.flags, PLR_THIEF))
			strcat(buf, " (THIEF)");
		}
	    }

	    if(IS_AFK(player))
		strcat(buf, " (AFK)");
            if(IS_GOD(ch) && IS_SET(person->specials.flags,PLR_SHOW_DAM))
                sprintf(buf, "%s $CR(SD)$CN", buf);
	    if(IS_WRITING(player))
		strcat(buf, " (WRITING)");
	    strcat(buf, "\n\r");

	    /* here's where the sorting comes into play */
	    /*strcat(output, buf);*/
	    if (IS_GOD(player))
		{
			if (!IS_SET(player->specials.flags, PLR_NOWIZ)) {
				for(p=godtable;p->index;p++) {
					if (IS_SET(player->player.godinfo.position,p->bit)) {
						who_list_add(ABS_MAX_LVL + p->index,buf);
						break;
					}
				}
				if (!IS_SET(player->player.godinfo.position,p->bit))
					if (TRUST(player) == MAX_TRUST)
						who_list_add(ABS_MAX_LVL + 14,buf);
					else
						who_list_add(ABS_MAX_LVL + 1,buf);
			}
			else
				who_list_add(GetMaxLevel(player),buf);

		} else if (!IS_GOD(player)) {
		     who_list_add(GetMaxLevel(player),buf);
        }
	}
	else
	{
	    if(rname)
		sprintf(buf, "%-14s ", GET_REAL_NAME(person));
	    else
		sprintf(buf, "%-14s ", GET_NAME(player));

	    if(idle)
	    {
		sprintf(line1, "Idle: %3d ", person->specials.timer);
		strcat(buf, line1);
	    }
	    if(levels)
	    {
		sprintf(line1,"Level:[%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d] ",
			person->player.level[0],person->player.level[1],
			person->player.level[2],person->player.level[3],
			person->player.level[4],person->player.level[5],
			person->player.level[6],person->player.level[7],
			person->player.level[8],person->player.level[9],
			person->player.level[10]);
		strcat(buf,line1);
	    }
	    if(hits)
	    {
		sprintf(line1,"Hit:[%3d] Mana:[%3d] Move:[%3d] ",
			GET_HIT(person),GET_MANA(person),GET_MOVE(person));
		strcat(buf, line1);
	    }
	    if(stats)
	    {
		sprintf(line1,"[S:%2d I:%2d W:%2d C:%2d D:%2d CH: %2d] ",
			GET_STR(person),GET_INT(person),GET_WIS(person),
			GET_CON(person),GET_DEX(person),GET_CHA(person));
		strcat(buf,line1);
	    }
	    if(title)
	    {
		sprintf(line1," %s",ss_data(player->player.title));
		strcat(buf,line1);
	    }
	    if(flags)
	    {
		if(player->invis_level > 0)
		{
		    sprintf(line1,"(I: %2d) ", player->invis_level);
		    strcat(buf,line1);
		}
		if(IS_AFK(player))
		    strcat(buf, "(AFK) ");
		if(IS_WRITING(player))
		    strcat(buf, "(WRITING) ");
	    }
	    if(builder)
		if(IS_SET(player->specials.flags, PLR_BUILDER))
		    strcat(buf,"(BLDR) ");
	    if(logall)
		if(IS_SET(player->specials.flags, PLR_LOGALL))
		    strcat(buf,"(LOG) ");
	    if(trust)
		sprintf(buf, "%s (T: %2d I: %2d)", buf, TRUST(player), player->invis_level);

	    if (!trust || IS_GOD(player))	   {
	    	strcat(buf, "\n\r");
	    	strcat(output, buf);
	    }
	}
    }
    END_AITER(iter);

    if(!godwho)
    {
	sprintf(buf, "\n\rTotal visible %s: %d\n\r\n\r",
		mort ? "players" : "heroes", listed);
    }
    else if(listed == 0)
    {
	sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\r",
		count,lcount,
		count ? ((double)lcount / (double)count) * 100. : 0.);
    }
    else
    {
	sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%) Number Listed: %d\n\r",
		count,lcount,
		count ? ((double)lcount / (double)count) * 100. : 0.,listed);
    }

	if(!godwho) {
		who_list_add(0,buf);
		who_list_print(ch);
		who_list_delete();
	} else {
		strcat(output, buf);
		page_string(ch->desc, output, 1); /* send the output */

	}


}




struct user_info
{
    int		socket;
    int		state;
    char*	name;
    char*	host;
};

int users_by_host(struct user_info* a, struct user_info* b)
{
    return strcmp(a->host, b->host);
}

int users_by_socket(struct user_info* a, struct user_info* b)
{
    return a->socket - b->socket;
}

int users_by_name(struct user_info* a, struct user_info* b)
{
    return str_cmp(a->name, b->name);
}

void do_users(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH], *line;
    char* name;
    int arg_len;
    struct descriptor_data *d;
    struct user_info	infos[256];
    struct user_info*	info;

    if(!ch->desc)
	return;

    strcpy(buf, "Connections:\n\r------------\n\r");

    argument = skip_spaces(argument);

    arg_len = strlen(argument);

    info = infos;

    EACH_DESCRIPTOR(d_iter, d)
    {
	info->socket = d->descriptor;
	info->state = d->connected;

	if (d->character && GET_REAL_NAME(d->character)){
	    name = GET_REAL_NAME(d->character);

	    /* check name start  and visibility */
	    if((arg_len &&
		strn_cmp(argument, name, arg_len)) ||
	       (d->character->invis_level > TRUST(ch)))
		continue;

	    info->name = name;
	}
	else if(*argument)	/* specified a name pattern, but no name */
	    continue;
	else
	    info->name = "[UNKNOWN]";

	if (d->host)
	    info->host = d->host;
	else
	    info->host = "[UNKNOWN]";

	info++;
    }
    END_ITER(d_iter);

    qsort(infos, info - infos, sizeof(*info),
	  (int (*)(const void *, const void *))users_by_host);

    info->name = NULL;

    for(info = infos, line = buf + strlen(buf) ; info->name ; info++)
    {
      char *state;
      switch(info->state)
      {
	case CON_PLYNG:		state = "PLAYING";	break;
	case CON_NME:		state = "Q_NAME";	break;
	case CON_NMECNF:	state = "Q_NAME";	break;
	case CON_PWDNRM:	state = "Q_PASS";	break;
	case CON_PWDGET:	state = "Q_PASS";	break;
	case CON_PWDCNF:	state = "Q_PASS";	break;
	case CON_QSEX:		state = "Q_SEX";	break;
	case CON_RMOTD:		state = "MOTD";	break;
	case CON_RIMOTD:	state = "IMOTD"; break;
	case CON_SLCT:		state = "MENU";	break;
	case CON_EXDSCR:	state = "DSCR";	break;
	case CON_QCLASS:	state = "Q_CLASS";	break;
	case CON_LDEAD:		state = "LINKDEAD";	break;
	case CON_PWDNEW:	state = "PASS_CHNGE";	break;
	case CON_PWDNCNF:	state = "PASS_CHNGE";	break;
	case CON_WIZLOCK:	state = "WIZLCK_MSG";	break;
	case CON_QRACE:		state = "Q_RACE";	break;
	case CON_RACPAR:	state = "UNUSED";	break;
#if PLAYER_AUTH
	case CON_AUTH:		state = "AWT_AUTH";	break;
#endif
	case CON_CITY_CHOICE:	state = "SLCT_CITY";	break;
	case CON_QPROFF:	state = "Q_MOTHER";	break;
	case CON_QPROFM:	state = "Q_FATHER";	break;
	case CON_QALIGN:	state = "Q_ALIGN";	break;
	case CON_QAGE12:	state = "Q_AGE12";	break;
	case CON_QAGE13:	state = "Q_AGE13";	break;
	case CON_QAGE14:	state = "Q_AGE14";	break;
	case CON_SITELOCK:	state = "SITELOCK";	break;
	case CON_CLOSE:		state = "CLOSE";	break;
	case CON_IDCONING:	state = "IDCONING";	break;
	case CON_IDCONED:	state = "IDCONED";	break;
	case CON_IDREADING:	state = "IDREADING";	break;
	case CON_IDREAD:	state = "IDREAD";	break;
	case CON_ASKNAME:	state = "ASKNAME";	break;

	default: state="UPDATE"; break;
	}
      sprintf(line, "[%2d] %-16s %-11s : %s\n\r",
	      info->socket, info->name, state, info->host);
      line += strlen(line);
    }
#undef STATE

    page_string(ch->desc,buf,1);
}

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You are carrying:\n\r", ch);
  list_obj_in_heap(ch->carrying, ch);
}

void do_equipment(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You are using:\n\r", ch);
  show_equiped(ch, ch);
}


void do_policy(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, policy, 0);
}

void do_credits(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, credits, 0);
}


void do_news(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, news, 0);
}


void do_info(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, info, 0);
}


void do_wizlist(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, wizlist, 0);
}

void do_wizlistnew(struct char_data *ch, char *argument, int cmd)
{
   page_string(ch->desc,newwizlist, 0);
}


void do_twstory(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, twstory, 0);
}

void do_herolist(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[2*MAX_STRING_LENGTH], buf2[2*MAX_STRING_LENGTH];
  static struct string_block herolist;
  static struct string_block immherolist;
  int days_off;

  strcpy(buf, "");
  strcpy(buf2, "");

  if(herolist_changed)
  {
    herolist_changed=0;

    destroy_string_block(&immherolist);
    init_string_block(&immherolist);

    sprintf(buf,"*** HEROES OF THIEVES WORLD ***\n\r");
    center(buf2,buf,41);
    append_to_string_block(&immherolist, buf2);

    for(i=0; ( (heroes[i].name != NULL) && (i < MAX_HEROES) ); i++)
    {
      days_off = (time(0) - heroes[i].logon) / (60*60*24);
      sprintf(buf,"%2i [%3i]  [%3i days] %3i points  %s the %s %s\n\r",
	      i+1,(int)heroes[i].level,days_off,
	      (int)heroes[i].points,
	      ss_data(heroes[i].name),heroes[i].race,heroes[i].titles);
      append_to_string_block(&immherolist, buf);
    }

    sprintf(buf,"***** ***** ***** *****\n\r");
    center(buf2,buf,41);
    append_to_string_block(&immherolist, buf2);


    destroy_string_block(&herolist);
    init_string_block(&herolist);

    sprintf(buf,"*** HEROES OF THIEVES WORLD ***\n\r");
    center(buf2,buf,41);
    append_to_string_block(&herolist, buf);

    for(i=0; ( (heroes[i].name!=NULL)&&(i<MAX_HEROES) ); i++)
    {
      sprintf(buf,"%2i [%3i] %3i points  %s the %s %s\n\r",
	      i+1,(int)heroes[i].level,
	      (int)heroes[i].points,
	      ss_data(heroes[i].name),
	      heroes[i].race,heroes[i].titles);
      append_to_string_block(&herolist, buf);
    }

    sprintf(buf,"***** ***** ***** *****\n\r");
    center(buf2,buf,41);
    append_to_string_block(&herolist, buf2);
  }

  if (IS_IMMORTAL(ch))
    page_string_block(&immherolist, ch);
  else
    page_string_block(&herolist, ch);
}


int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
    struct char_data	*i;
    char *name;
    int	number = 0;

    name = fname(GET_REAL_NAME(mob));
    EACH_CHARACTER(iter, i)
    {
	if (isname(name, GET_REAL_NAME(i)) && i->in_room != NOWHERE)
	{
	    number++;
	    if (i==mob)
		break;
	}
    }
    END_AITER(iter);

    return i ? number : 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
    static char buf[MAX_STRING_LENGTH];
    if (IS_NPC(person) && IS_GOD(ch)) {
	sprintf(buf, "%d.%s", which_number_mobile(ch, person),
		fname(GET_REAL_NAME(person)));
    } else {
	strcpy(buf, PERS(person, ch));
    }
    return buf;
}

static void do_where_person(struct char_data *ch, struct char_data *person,
			    struct string_block *sb)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%-30s- %s ", PERS(person, ch),
	    (person->in_room > NOWHERE ? real_roomp(person->in_room)->name : "Nowhere"));

    if (IS_GOD(ch))
	sprintf(buf+strlen(buf),"[%ld]", person->in_room);

    strcpy(buf+strlen(buf), "\n\r");

    append_to_string_block(sb, buf);
}

static void do_where_object(struct char_data *ch, struct obj_data *obj,
			    int recurse, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  if (obj->in_room != NOWHERE) { /* object in a room */
    sprintf(buf, "%-30s- %s [%ld]\n\r",
	    OBJ_SHORT(obj),
	    real_roomp(obj->in_room)->name,
	    obj->in_room);
  } else if (obj->carried_by != NULL) { /* object carried by monster */
    sprintf(buf, "%-30s- carried by %s\n\r",
	    OBJ_SHORT(obj),
	    numbered_person(ch, obj->carried_by));
  } else if (obj->equipped_by != NULL) { /* object equipped by monster */
    sprintf(buf, "%-30s- equipped by %s\n\r",
	    OBJ_SHORT(obj),
	    numbered_person(ch, obj->equipped_by));
  } else if (obj->in_obj) { /* object in object */
    sprintf(buf, "%-30s- in %s\n\r",
	    OBJ_SHORT(obj),
	    OBJ_SHORT(obj->in_obj));
  } else {
    sprintf(buf, "%-30s- god doesn't even know where...\n\r",
	    OBJ_SHORT(obj));
  }
  if (*buf)
    append_to_string_block(sb, buf);

  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char	*nameonly;
    register struct char_data *i;
    register struct obj_data *k;
    struct descriptor_data *d;
    int	number, count, vnum;
    struct string_block	sb;

    only_argument(argument, name);

    if (!*name)
    {
	if (!IS_GOD(ch))
	    send_to_char("Only gods can perform this action!\n\r", ch);
	else
	{
	    init_string_block(&sb);
	    append_to_string_block(&sb, "Players:\n\r--------\n\r");

	    EACH_DESCRIPTOR(d_iter, d)
	    {
		if (d->character && (d->connected == CON_PLYNG) &&
		    (d->character->in_room != NOWHERE))
		{
		    if (d->character->orig) /* If switched */
			sprintf(buf, "%-20s - %s [%ld] In body of %s\n\r",
				GET_REAL_NAME(d->character),
				real_roomp(d->character->in_room)->name,
				d->character->in_room,
				fname(GET_IDENT(d->character)));
		    else if (TRUST(ch) >= (d->character->invis_level))
			sprintf(buf, "%-20s - %s [%ld]\n\r",
				GET_REAL_NAME(d->character),
				real_roomp(d->character->in_room)->name,
				d->character->in_room);

		    append_to_string_block(&sb, buf);
		}
	    }
	    END_ITER(d_iter);
	    page_string_block(&sb,ch);
	    destroy_string_block(&sb);
	}
    }
    else
    {
	vnum = count = number = 0;

	if (isdigit(*name))
	{
	    if(strchr(name, '.'))
	    {
		nameonly = name;
		count = number = get_number(&nameonly);
	    }
	    else
		vnum = atoi(name);
	}

	init_string_block(&sb);

	if(!vnum)
	{
	    EACH_CHARACTER(iter, i)
	    {
		if (isname(name, GET_REAL_NAME(i)) && CAN_SEE(ch, i))
		{
		    if ((i->in_room != NOWHERE) &&
			(IS_GOD(ch) ||
			(real_roomp(i->in_room)->zone ==
			 real_roomp(ch->in_room)->zone)))
		    {
			if (number==0 || (--count) == 0) {
			    if (number==0) {
				sprintf(buf, "[%2d] ", ++count);
				append_to_string_block(&sb, buf);
			    }
			    do_where_person(ch, i, &sb);
			    if (number!=0)
				break;
			}
			if (!IS_GOD(ch))
			    break;
		    }
		}
	    }
	    END_AITER(iter);

	    if (IS_GOD(ch))
	    {
		EACH_OBJECT(iter, k)
		{
		    if (isname(name, OBJ_NAME(k)) && CAN_SEE_OBJ(ch, k))
		    {
			if (number==0 || (--count)==0) {
			    if (number==0) {
				sprintf(buf, "[%2d] ", ++count);
				append_to_string_block(&sb, buf);
			    }
			    do_where_object(ch, k, number!=0, &sb);
			    if (number!=0)
				break;
			}
		    }
		}
		END_AITER(iter);
	    }
	}
	else if(TRUST(ch) >= TRUST_LORD)
	{
	    int rnum = real_object(vnum);

	    EACH_OBJECT(iter, k)
	    {
		if(k->item_number == rnum)
		    do_where_object(ch, k, 0, &sb);
	    }
	    END_AITER(iter);
	}

	if (!*sb.data)
	    send_to_char("Couldn't find any such thing.\n\r", ch);
	else
	    page_string_block(&sb, ch);
	destroy_string_block(&sb);
    }
}




void do_levels(struct char_data *ch, char *argument, int cmd)
{
    int i, clss;
    char buf[200];
    char bigbuf[MAX_STRING_LENGTH*4];

    if (IS_NPC(ch))
    {
	send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
	return;
    }

    *buf = '\0';
    /*
     **  get the class
     */

    for (;isspace(*argument);argument++);

    if (!*argument)
    {
	send_to_char("You must supply a class!\n\r", ch);
	return;
    }

    switch(*argument)
    {
    case 'C':
    case 'c':
	clss = CLERIC_LEVEL_IND;
	break;
    case 'W':
    case 'w':
	clss = WARRIOR_LEVEL_IND;
	break;
    case 'M':
    case 'm':
    clss = MAGE_LEVEL_IND;
	break;
    case 'T':
    case 't':
	clss = THIEF_LEVEL_IND;
	break;
    case 'K':
    case 'k':
	clss = PALADIN_LEVEL_IND;
	break;
    case 'D':
    case 'd':
	clss = DRUID_LEVEL_IND;
	break;
    case 'S':
    case 's':
	clss = PSI_LEVEL_IND;
	break;
    case 'R':
    case 'r':
	clss = RANGER_LEVEL_IND;
	break;
    case 'H':
    case 'h':
        clss = SHIFTER_LEVEL_IND;
        break;
    case 'O':
    case 'o':
        clss = MONK_LEVEL_IND;
        break;
    case 'B':
    case 'b':
        clss = BARD_LEVEL_IND;
        break;
    default:
	sprintf(buf, "I don't recognize %s\n\r", argument);
	send_to_char(buf,ch);
	return;
	break;
    }

    sprintf(bigbuf,"\n\r[Level]    Exp   | Para | Rod  | Petr | Brth | Spll | Title \n\r");
    for (i = 1; i <= MAX_MORT; i++)
    {
	sprintf(buf, "[%2d] %12Lu | %2d | %2d | %2d | %2d | %2d | %s\n\r", i, exp_table[i - 1],
	       	saving_throws[clss][SAVING_PARA][(int)i-1],
		saving_throws[clss][SAVING_ROD][(int)i-1], saving_throws[clss][SAVING_PETRI][(int)i-1],
		saving_throws[clss][SAVING_BREATH][(int)i-1], saving_throws[clss][SAVING_SPELL][(int)i-1],
		(GET_SEX(ch)==SEX_FEMALE?titles[clss][i].title_f:
		 titles[clss][i].title_m));

	strcat(bigbuf,buf);
    }
    if(ch->desc) page_string(ch->desc,bigbuf,1);
}



void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int diff, w_typem, w_typec;
  int damc=0;
  int damm=0;
  int i=0;
  struct obj_data *wieldedc;
  struct obj_data *wieldedm;

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Easy! Very easy indeed!\n\r", ch);
    return;
  }

  if (IS_PC(victim) || IS_IMMORTAL(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
    return;
  }

/*  diff =  (GetMaxLevel(victim)+(GetSecMaxLev(victim)/2)+
	  (GetThirdMaxLev(victim)/3))-
          (GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+(GetThirdMaxLev(ch)/3)); */

  diff = GetMaxLevel(victim) - GetMaxLevel(ch);

  send_to_char("Level comparison:  ",ch);
  if (diff <= -10)
    send_to_char("Too easy to be believed.\n\r", ch);
  else if (diff <= -5)
    send_to_char("Not a problem.\n\r", ch);
  else if (diff <= -3)
    send_to_char("Rather easy.\n\r",ch);
  else if (diff <= -2)
    send_to_char("Easy.\n\r", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\n\r", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\n\r", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\n\r", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\n\r", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\n\r", ch);
  else if (diff <= 10)
    send_to_char("Are you crazy?  Is that your problem?\n\r", ch);
  else if (diff <= 100)
    send_to_char("You ARE mad!\n\r", ch);


  diff = GET_HIT(victim) - GET_HIT(ch);

  if (diff <= -200)
    send_to_char("This mobile does not have near as many hitpoints as you do.\n\r", ch);
  else if (diff <= -150)
    send_to_char("This mobile has very little hitpoints in comparison to you.\n\r", ch);
  else if (diff <= -100)
    send_to_char("This mobile's hitpoints are much lower than yours.\n\r",ch);
  else if (diff <= -50)
    send_to_char("This mobile's hitpoints could use a boost to compare to yours.\n\r", ch);
  else if (diff <= -25)
    send_to_char("This mobile has lower hitpoints than you do, but is catching up.\n\r", ch);
  else if (diff <= 0)
    send_to_char("This mobile has almost as many or the same amount of hitpoints as you do.\n\r", ch);
  else if (diff <= 15)
    send_to_char("This mobile has slightly greater hitpoints than you do.\n\r", ch);
  else if (diff <= 35)
    send_to_char("This mobile has a little more hitpoints than you do.\n\r", ch);
  else if (diff <= 75)
    send_to_char("This mobile has quite a few more hitpoints than you do.\n\r", ch);
  else if (diff <= 115)
    send_to_char("This mobile has much more hitpoints than you do.\n\r", ch);
  else if (diff <= 155)
    send_to_char("This mobile has many more hitpoints than you do!\n\r", ch);
  else if (diff > 155)
    send_to_char("As far as hit points go, this mobile blows you away!\n\r", ch);

  w_typem = GetWeaponType(victim, &wieldedm);
  w_typec = GetWeaponType(ch, &wieldedc);
  for (i=0; i<10; i++) {
    damm += GetWeaponDam(victim, ch, wieldedm);
    damc += GetWeaponDam(ch, victim, wieldedc);
  }
  damm /= i;
  damc /= i;

  diff=damm-damc;


  if (diff <= -25)
    send_to_char("This mobile's average damage is MUCH less than yours.\n\r", ch);
  else if (diff <= -20)
    send_to_char("This mobile's average damage is a lot less than yours.\n\r", ch);
  else if (diff <= -15)
    send_to_char("This mobile's average damage is quite a bit less than yours!\n\r",ch);
  else if (diff <= -10)
    send_to_char("This mobile's average damage is not as much as yours.\n\r", ch);
  else if (diff <= -5)
    send_to_char("This mobile's average damage is catching up to yours.\n\r", ch);
  else if (diff <= 0)
    send_to_char("This mobile's average damage is the same, or almost the same, as yours.\n\r", ch);
  else if (diff <= 5)
    send_to_char("This mobile's average damage is slightly greater than yours.\n\r", ch);
  else if (diff <= 10)
    send_to_char("This mobile's average damage is a little bit greater than yours.\n\r", ch);
  else if (diff <= 15)
    send_to_char("This mobile's average damage is greater than yours!\n\r", ch);
  else if (diff <= 20)
    send_to_char("This mobile's average damage is much greater than yours!\n\r", ch);
  else if (diff <= 25)
    send_to_char("This mobile's average damage kicks your butt!\n\r", ch);
  else if (diff > 25)
    send_to_char("This mobile's average damage blows you away!\n\r", ch);

  if (mob_index[victim->nr].func && IS_SET(victim->specials.mob_act, ACT_SPEC))
    send_to_char("This mobile has special powers, and you might proceed with caution.\n\r", ch);


}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int  i, j, is_spell, k, is_god;
  char buf[32768], temp[256];
  struct spell_info* spell, *last, *sel_spell;

  if (!IS_PC(ch))    {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }

  *buf=0;

  is_spell = (cmd != 550);
  is_god = IS_GOD(ch);

  for(last=spell_list;!last->name;last++);

  for(i=1,sel_spell=spell_list;i<spell_count; sel_spell++,i++)
     if(sel_spell->name &&
	(strcmp(sel_spell->name, last->name) < 0) &&
	((!(sel_spell->targets & TAR_SKILL) && is_spell) ||
	 ((sel_spell->targets & TAR_SKILL) && !is_spell)))
       last=sel_spell;

  for (i=1; ;i++) {
     spell=NULL;
     for (j = 0, sel_spell = spell_list ; (j <= spell_count) && (i>1) ; sel_spell++, j++)
       if((sel_spell->name &&
	   (!(sel_spell->targets & TAR_SKILL) && is_spell) ||
	   ((sel_spell->targets & TAR_SKILL) && !is_spell)) &&
	  ((spell && (strcmp(sel_spell->name, spell->name) < 0) &&
	  (strcmp(sel_spell->name, last->name) > 0)) ||
	  (!spell && (strcmp(sel_spell->name, last->name) > 0))))
	 spell = sel_spell;

     if(!spell && (i>1)) break;

     if(i>1)
       last=spell;
     else
       spell=last;

     k=0;
     for(j=0;j<=MAX_LEVEL_IND;j++) {
	if((spell->min_level[j] > 0) &&
	   (spell->min_level[j] <= MAX_MORT) &&
	   (j != MONK_LEVEL_IND)) {
 	   k=1;
	   break;
	}
     }
     if(!k && !is_god) continue;

     sprintf(temp,
	     "[%3d] %-25s M:%3d C:%3d W:%3d T:%3d K:%3d D:%3d P:%3d R:%3d H:%3d O:%3d B:%3d\n\r",
	     (is_god)?spell->number:i,
	     spell->name,
             ((k=spell->min_level[MAGE_LEVEL_IND])<=MAX_MORT)?k:0,
	     ((k=spell->min_level[CLERIC_LEVEL_IND])<=MAX_MORT)?k:0,
	     ((k=spell->min_level[WARRIOR_LEVEL_IND])<=MAX_MORT)?k:0,
	     ((k=spell->min_level[THIEF_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[PALADIN_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[DRUID_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[PSI_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[RANGER_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[SHIFTER_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[MONK_LEVEL_IND])<=MAX_MORT)?k:0,
             ((k=spell->min_level[BARD_LEVEL_IND])<=MAX_MORT)?k:0);
     strcat(buf, temp);
  }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);

  send_to_char("\n\rFor a listing of spells/skills, type practice <class> or go to your guild.\n\r", ch);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  static char buf[100];
  long ct, ot;
  char *tmstr, *otmstr;
  struct tm* tm;
  char* tz;

  //Tell them if pkill is on or not
  switch(PKILLABLE) {
    case 0 : //None
      send_to_char("\n\rPKill Arena is turned OFF, PKill Clan is turned OFF\n\r",ch);
      break;
    case 1 : //CLAN
      send_to_char("\n\rPKill Arena is turned OFF, PKill Clan is turned ON! Go Clan!\n\r",ch);
      break;
    case 2 : //ARENA
      send_to_char("\n\rPkill Arena is turned ON, Pkill Clan is turned OFF.\n\r", ch);
      break;
  }

  ot = Uptime;
  otmstr = asctime(tm = localtime(&ot));
  tz = "MDT";
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "Start time was: %s (%s)\n\r", otmstr, tz);
  send_to_char(buf, ch);

  ct = time(0);
  tmstr = asctime(tm = localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf, "Current time is: %s (%s)\n\r", tmstr, tz);
  send_to_char(buf, ch);
#if HASH
#if 0
  sprintf(buf, "Total number of rooms in world: %d\n\r", room_db.klistlen);
#else
  strcpy(buf, "Total number of rooms in world: unknown -- hashing installed\n\r");
#endif
#else
  sprintf(buf, "Total number of rooms in world: %ld\n\r", room_count);
#endif
  send_to_char(buf, ch);
  sprintf(buf, "Total number of zones in world: %d\n\r\n\r",
	  top_of_zone_table + 1);
  send_to_char(buf, ch);

  sprintf(buf,"Total number of registered players: %d\n\r",player_count);
  send_to_char(buf, ch);

  sprintf(buf, "Total number of monsters in game: %ld\n\r", mob_count);
  send_to_char(buf, ch);

  sprintf(buf, "Total number of objects in game: %ld\n\r", obj_count);
  send_to_char(buf, ch);

  sprintf(buf, "Total number of randoms loaded: %ld\n\r", random_count);
  send_to_char(buf, ch);

  if (IS_GOD(ch)) {
#ifdef SWAP_ZONES
    sprintf(buf, "Swapped Zones: %d,  Loaded Zones: %d\n\r",
	    swapped_zones, loaded_zones);
    send_to_char(buf, ch);
#endif

  sprintf(buf,"Total number of distinct mobiles in world: %d\n\r",
	  top_of_mobt + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of distinct objects in world: %d\n\r\n\r",
	  top_of_objt + 1);
  send_to_char(buf, ch);

    sprintf(buf,"Max Connected: %d,  Current Connected: %d\n\r", max_connected, connected);
    send_to_char(buf,ch);
    sprintf(buf,"Current amount of gold in Casino Bank: %d\n\r", CASINO_BANK);
    send_to_char(buf,ch);
    sprintf(buf,"Amount of gold earned at the Pawn Shop: %d\n\r", PAWN_SHOP);
    send_to_char(buf,ch);

    // Group restrictions info
    if(GROUP_RES == 2)
      sprintf(buf,"Group Limiter: ON\n\r");
    else
      sprintf(buf,"Group Limiter: OFF\n\r");
    send_to_char(buf,ch);

    // Area FX information
    if(AREA_FX_DEADLY == 2)
      sprintf(buf,"Area FX: Deadly\n\r");
    else
      sprintf(buf,"Area FX: Safe\n\r");
    send_to_char(buf,ch);

    if(CHAOS==1)
      sprintf(buf,"Chaos variables: ON\n\r");
    else
      sprintf(buf,"Chaos variables: OFF\n\r");
    send_to_char(buf,ch);
    sprintf(buf,"Maximum exp command limit: %d\n\r", MAX_EXP_REIMB);
    send_to_char(buf,ch);
  }
  if (TRUST(ch) > TRUST_LORD) {
    sprintf(buf,"Perform pirate quest: %d  How many executed: %d\n\r",
	    PIRATEQST, PIRATENUM);
    send_to_char(buf,ch);
    sprintf(buf,"Perform disaster: %d  How many executed: %d\n\r",
	    DISASTER, DISASTERNUM);
    send_to_char(buf,ch);
    sprintf(buf,"Amount of gold given at fountains: %d\n\r", FOUNTAIN_GOLD);
    send_to_char(buf,ch);
    sprintf(buf,"Amount of levels given at fountains: %d\n\r", FOUNTAIN_LEVEL);
    send_to_char(buf,ch);
  }
}
void show_attr(struct char_data *ch, struct char_data *to)
{

#if 0
  struct time_info_data playing_time;
  static char buf[MAX_STRING_LENGTH];
  struct affected_type *aff;
  int type;

  playing_time = real_time_passed((time(0)-ch->player.time.logon) +
                                        ch->player.time.played, 0);
  sprintf(buf,"$CWYou have been alive for$Cc %d days$CW and$Cc %d hours.$CN ",
                        playing_time.day, playing_time.hours);
         send_to_char_formatted(buf, to);

  sprintf(buf, "$CWYou are$Cc %d years old.$CN\n\r", GET_AGE(ch));

  if ((age(ch).month == 0) && (age(ch).day == 0))
     strcat(buf,"$CwIt's your birthday today. Happy Birthday!$CN\n\r");
           send_to_char_formatted(buf, to);

  if(!IS_NPC(ch) || !IS_SET(ch->specials.mob_act, ACT_POLYSELF))  {
    send_to_char_formatted("$CWYou are of the$Cc ", to);
        sprinttype((ch->race),RaceName,buf);
        send_to_char_formatted(buf, to);
	sprintf(buf,
		" $CWrace, you are %d' %d\" tall, and you weigh %d lbs.$CN\n\r",
		(int) (ch->player.height /12) , (int) (ch->player.height % 12),
		ch->player.weight);
	send_to_char_formatted(buf, to);
  }

  if ((GET_STR(ch)==18) && (((HasClass(ch, CLASS_WARRIOR))) ||
                            (HasClass(ch, CLASS_PALADIN))  ||
                            (HasClass(ch, CLASS_RANGER))))
  {
      sprintf(buf,"You have $Cr%d/%d STR, %d INT, %d WIS, %d DEX, %d CON, %d CHA.$CN\n\r",
	      GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch),
	      GET_CON(ch), GET_CHA(ch));
      send_to_char_formatted(buf,to);
  }
  else
  {
      sprintf(buf,"You have$Cb %d STR, %d INT, %d WIS, %d DEX, %d CON, %d CHA.$CN\n\r",
	      GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch),
	      GET_CHA(ch));
      send_to_char_formatted(buf,to);
  }

  sprintf(buf, "You can carry a max of %d lbs with your strength.\n\r",
                                                           CAN_CARRY_W(ch));
         send_to_char(buf, to);

  sprintf(buf, "You are carrying $Cy%d lbs$CN of equipment.\n\r",
          IS_CARRYING_W(ch));
  send_to_char_formatted(buf, to);

  sprintf(buf, "Your alignment is (-1000 to 1000) $Cg%d.$CN\n\r",GET_ALIGNMENT(ch));
         send_to_char_formatted(buf, to);

  if (GET_ALIGNMENT(ch) == 1000)  {
     sprintf(buf,"$CwYou are the kindest person you know.$CN\n\r");
            send_to_char_formatted(buf, to);
            }
  if (GET_ALIGNMENT(ch) <= 999 && GET_ALIGNMENT(ch) >= 501)  {
     sprintf(buf,"You are a generous person.\n\r");
            send_to_char(buf, to);
            }
  if (GET_ALIGNMENT(ch) <= 500 && GET_ALIGNMENT(ch) >= 1)  {
     sprintf(buf,"You are a considerate person.\n\r");
            send_to_char(buf, to);
            }
  if (GET_ALIGNMENT(ch) == 0)   {
     sprintf(buf,"$CGYou are a true neutral.$CN\n\r");
            send_to_char_formatted(buf, to);
            }
  if (GET_ALIGNMENT(ch) <= -1 && GET_ALIGNMENT(ch) >= -500)  {
     sprintf(buf,"You are not nice.\n\r");
            send_to_char(buf, to);
            }
  if (GET_ALIGNMENT(ch) <= -501 && GET_ALIGNMENT(ch) >= -999)  {
     sprintf(buf,"You are an evil person.\n\r");
            send_to_char(buf, to);
            }
  if (GET_ALIGNMENT(ch) == -1000)  {
     sprintf(buf,"$CRYou are from the depths of hell!$CN\n\r");
            send_to_char_formatted(buf, to);
            }

  sprintf(buf,"Your saves are: [%d versus paralyze]   [%d versus rods]\n\r",
       ch->specials.apply_saving_throw[0],ch->specials.apply_saving_throw[1]);
         send_to_char(buf, to);
  sprintf(buf,"--------------  [%d versus petrify]    [%d versus breath]\n\r",
       ch->specials.apply_saving_throw[2],ch->specials.apply_saving_throw[3]);
         send_to_char(buf, to);
  sprintf(buf,"                [%d versus spells]\n\r\n\r",
                       ch->specials.apply_saving_throw[4]);
         send_to_char(buf, to);

  /* immunities */
  send_to_char("You are immune to:  ",to);
  sprintbit(ch->M_immune, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);
  /* resistances */
  send_to_char("You are resistant to:  ",to);
  sprintbit(ch->immune, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);
  /* Susceptible */
  send_to_char("You are susceptible to:  ",to);
  sprintbit(ch->susc, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);

  }
#else
  struct time_info_data playing_time;
  char buf[MAX_STRING_LENGTH];
  struct affected_type *aff;
  int type;

  playing_time = real_time_passed((time(0)-ch->player.time.logon) +
				  ch->player.time.played, 0);
  sprintf(buf,"$CWYou have been alive for$Cc %d days$CW and$Cc %d hours.$CN ",
	    playing_time.day, playing_time.hours);
  send_to_char_formatted(buf, to);

  sprintf(buf, "$CWYou are$Cc %d years$CW and$Cc %d months old.$CN\n\r",
	  GET_AGE(ch), age(ch).month);

  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf,"$CwIt's your birthday today. Happy Birthday!$CN\n\r");
  send_to_char_formatted(buf, to);


  if(!IS_NPC(ch) || !IS_SET(ch->specials.mob_act, ACT_POLYSELF)) {
    send_to_char_formatted("$CWYou are of the$Cc ", to);
    sprinttype((ch->race),RaceName,buf);
    send_to_char_formatted(buf, to);
    sprintf(buf,
	    " $CWrace, you are %d' %d\" tall, and you weigh %d lbs.$CN\n\r",
	    (int) (ch->player.height /12) , (int) (ch->player.height % 12),
	    ch->player.weight);
    send_to_char_formatted(buf, to);
  }

  if(GetMaxLevel(to) > 17)
    if ((GET_STR(ch)==18) && (((HasClass(ch, CLASS_WARRIOR))) ||
			      (HasClass(ch, CLASS_PALADIN))  ||
			      (HasClass(ch, CLASS_RANGER))))
      {
	sprintf(buf,
       "You have $Cr%d/%d STR, %d INT, %d WIS, %d DEX, %d CON, %d CHA.$CN\n\r",
		GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch),
		GET_DEX(ch),  GET_CON(ch), GET_CHA(ch));
	send_to_char_formatted(buf,to);
      }
    else
      {
	sprintf(buf,
	  "You have$Cb %d STR, %d INT, %d WIS, %d DEX, %d CON, %d CHA.$CN\n\r",
		GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch),
		GET_CON(ch),
		GET_CHA(ch));
	send_to_char_formatted(buf,to);
      }


  if(GetMaxLevel(to) > 8) {
    sprintf(buf, "You can carry a maximum of %d lbs.\n\r", CAN_CARRY_W(ch));
    send_to_char(buf, to);
  }

  sprintf(buf, "You are carrying $Cy%d lbs$CN of equipment.\n\r",
          IS_CARRYING_W(ch));
  send_to_char_formatted(buf, to);

  if(GetMaxLevel(to) <= 2)
    return;

  sprintf(buf, "Your alignment is %d (-1000 to 1000).\n\r",
	  GET_ALIGNMENT(ch));
  send_to_char(buf, to);

  if (GET_ALIGNMENT(ch) == 1000) {
    sprintf(buf,"$CwYou are the kindest person you know.$CN\n\r");
    send_to_char_formatted(buf, to);
  }
  if (GET_ALIGNMENT(ch) <= 999 && GET_ALIGNMENT(ch) >= 501) {
    sprintf(buf,"You are a generous person.\n\r$CN\n\r");
    send_to_char_formatted(buf, to);
  }
  if (GET_ALIGNMENT(ch) <= -1 && GET_ALIGNMENT(ch) >= -500)  {
    sprintf(buf,"You are not nice.\n\r");
    send_to_char(buf, to);
  }
  if (GET_ALIGNMENT(ch) <= -501 && GET_ALIGNMENT(ch) >= -999)  {
    sprintf(buf,"You are an evil person.\n\r");
    send_to_char(buf, to);
  }
  if (GET_ALIGNMENT(ch) == -1000)  {
    sprintf(buf,"$CRYou are from the depths of hell!$CN\n\r");
    send_to_char_formatted(buf, to);
  }

  if(GetMaxLevel(to) <= 14)
    return;

  sprintf(buf,"Apply to saving throws:  [%d against paralyzation]    [%d against rod]\n\r",
	  BestSaveThrow(ch, SAVING_PARA) + ch->specials.apply_saving_throw[0],
	  BestSaveThrow(ch, SAVING_ROD) + ch->specials.apply_saving_throw[1]);
  send_to_char(buf, to);
  sprintf(buf,"-----------------------  [%d against petrification]   [%d against breath]\n\r",
	  BestSaveThrow(ch, SAVING_PETRI) + ch->specials.apply_saving_throw[2],
	  BestSaveThrow(ch, SAVING_BREATH) + ch->specials.apply_saving_throw[3]);
  send_to_char(buf, to);
  sprintf(buf,"                         [%d against spell]\n\r\n\r",
	  BestSaveThrow(ch, SAVING_SPELL) + ch->specials.apply_saving_throw[4]);
  send_to_char(buf, to);

  if(GetMaxLevel(to) <= 19)
    return;

  /* immunities */
  send_to_char("You are immune to:  ",to);
  sprintbit(ch->M_immune, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);
  /* resistances */
  send_to_char("You are resistant to:  ",to);
  sprintbit(ch->immune, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);
  /* Susceptible */
  send_to_char("You are susceptible to:  ",to);
  sprintbit(ch->susc, immunity_names, buf);
  strcat(buf,".\n\r");
  send_to_char(buf,to);

  if(GetMaxLevel(to) <= 24)
    return;

#endif
}

void do_affect(struct char_data *ch, char *arg, int cmd)
{
  show_affect(ch,ch);
}

void show_affect(struct char_data *ch, struct char_data *to)
{
  static char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH] = "";
  struct affected_type *aff;
  int type;

  if (AFF_FLAGS(ch) || AFF2_FLAGS(ch))
  {
        send_to_char_formatted("\n\r$CgAffects Summary:\n\r----------------\n\r", to);

        if (AFF_FLAGS(ch))
                sprintbit(AFF_FLAGS(ch), affected_bits, buf);
        if (AFF2_FLAGS(ch))
                sprintbit(AFF2_FLAGS(ch), affected2_bits, buf2);
        strcat(buf,buf2);
        strcat(buf,"\n\r\n\r");
        send_to_char_formatted(buf, to);
  }

  if (ch->affected)
  {
    send_to_char_formatted("\n\r$CRAffecting Spells/Skills:\n\r------------------------$CN\n\r", to);
    type = 0;
    for(aff = ch->affected; aff; aff = aff->next)
    {
      if(aff->type != type)
      {
        if (IS_SET(spell_by_number(aff->type)->targets, TAR_SKILL))
          sprintf(buf, "Skill : '%s'\n\r", spell_name(aff->type));
        else
          sprintf(buf, "Spell : '%s'\n\r", spell_name(aff->type));
        send_to_char(buf, to);
        type = aff->type;
      }
    }
  }
  else
   send_to_char_formatted("$CGYou have nothing affecting you at this time.$CN\n\r", ch);
}


void do_attribute(struct char_data *ch, char *argument, int cmd)
{
  show_attr(ch,ch);
}


/* change a character's "short name"  the one that appears
   whenever they do anything */
void do_name(struct char_data* ch, char* argument, int cmd)
{
  int		len;
  char*	repl;
  char	buf[256];

  if(IS_NPC(ch))
  {
    send_to_char("You don't have a name to change bozo!\n\r", ch);
    return;
  }

  while(argument && isspace(*argument))
    argument++;

  if(!argument || !*argument)
  {
    sprintf(buf, "Name: %s\n\r", GET_NAME(ch));
    send_to_char(buf, ch);
    return;
  }

  if((repl = strstr(argument, "~N")))
  {
    len = repl - argument;

    if(len)
    {
      strncpy(buf, argument, len);
      if((argument[len - 1] != ' ') && (TRUST(ch) < TRUST_CREATOR))
	buf[len++] = ' ';
    }
    strcpy(buf + len, GET_REAL_NAME(ch));
    len += strlen(GET_REAL_NAME(ch));
    if(repl[2])
    {
      if((repl[2] != ' ') && (TRUST(ch) < TRUST_CREATOR))
	buf[len++] = ' ';
      strcpy(buf + len, repl + 2);
    }
  }
  else if(TRUST(ch) < TRUST_IMP)
  {
    send_to_char("Real Name appended to shown name\n\r", ch);
    strcpy(buf, argument);
    strcat(buf, " ");
    strcat(buf, GET_REAL_NAME(ch));
  }
  else
    strcpy(buf, argument);

  if((strlen(buf) > 20) && (TRUST(ch) < TRUST_GOD))
  {
    send_to_char("Your name must be less than 20 chars long\n\r",
		 ch);
    return;
  }

  ss_free(ch->player.short_descr);
  ch->player.short_descr = ss_make(buf);

  sprintf(buf, "Name set to: '%s'\n\r", GET_NAME(ch));
  send_to_char(buf, ch);
}

void do_read_motd(struct char_data* ch, char* argument, int cmd)
{
    page_string(ch->desc, motd, 0);
}

void do_read_imotd(struct char_data* ch, char* argument, int cmd)
{
    page_string(ch->desc, imotd, 0);
}

void do_terrain (struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if (OUTSIDE(ch))
    sprintf(buf, "The terrain of this outdoor area is ");
  else
    sprintf(buf, "The terrain of this indoor area is ");


  switch (real_roomp(ch->in_room)->sector_type) {
    case SECT_INSIDE:
      strcat(buf, "inside.");
      break;
    case SECT_CITY:
      strcat(buf, "city.");
      break;
    case SECT_FIELD:
      strcat(buf, "field.");
      break;
    case SECT_FOREST:
      strcat(buf, "forest.");
      break;
    case SECT_HILLS:
      strcat(buf, "hills.");
      break;
    case SECT_MOUNTAIN:
      strcat(buf, "mountain.");
      break;
    case SECT_WATER_SWIM:
      strcat(buf, "water.");
      break;
    case SECT_WATER_NOSWIM:
      strcat(buf, "river.");
      break;
    case SECT_AIR:
      strcat(buf, "air.");
      break;
    case SECT_UNDERWATER:
      strcat(buf, "underwater.");
      break;
    case SECT_DESERT:
      strcat(buf, "desert.");
      break;
    case SECT_SKY:
      strcat(buf, "sky.");
      break;
    default:
      sprintf(buf, "Illegal sector type in room %p", real_roomp(ch->in_room));
      log_msg(buf);
      sprintf(buf, "Ack! Please report this room to a god, it's undefined.");
      break;
  }
  strcat(buf, "\n\r");
  send_to_char(buf, ch);
}

void do_laston(struct char_data *ch, char *arg, int cmd)
{
  char name[MAX_INPUT_LENGTH], *tmstr, buf[128];
  struct char_data *victim;
  int connected=0;
  long ct;

  arg=one_argument(arg,name);
  if (!*name)
  {
    send_to_char("I'm sorry, but I don't have ESP.\n\r", ch);
    return;
  }

  if((victim=find_player_in_world(name)))
    connected=1;
  else if(!( victim = LoadChar(0, name, READ_PLAYER)))
  {
    send_to_char("\n\rAre you sure you know this person?\n\r",ch);
    return;
  }

  if( ((IS_GOD(victim)) && (TRUST(ch) < TRUST(victim))) )
  {
    send_to_char("\n\rHe deserves a little privacy now and again don't "
		 "you think?\n\r",ch);
    return;
  }

  if (connected)
  {
      sprintf(buf,"You have a feeling that you might have seen %s recently.\n\r",
	      GET_REAL_NAME(victim));
      send_to_char(buf,ch);
      return;
  }

  ct = victim->player.time.logon;
  tmstr = ctime(&ct);
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf,"%s was last seen %s.\n\r",GET_REAL_NAME(victim),tmstr);
  send_to_char(buf,ch);
  if (TRUST(ch) >= TRUST_LORD)
  {
    sprintf(buf,"  and was logged in from %s.\n\r", victim->specials.hostname);
    send_to_char(buf,ch);
  }
  victim->in_room = -1;
  extract_char(victim);
}

void do_zones(struct char_data *ch, char *argument, int cmd)
{
  struct zoneh_data *zoneh;
  struct string_block sb;
  char buf[MAX_STRING_LENGTH];
  char zonename[MAX_INPUT_LENGTH];

  one_argument(argument, zonename);

  /*
   * No args, show default zone list
   */
  if(strcmp(zonename,"")==0)
    page_string_block(&zonehelplist, ch);
  /*
   * l<level> or L<level> -- show areas of that sugested level
   */
  else
    if( ((*zonename=='L') || (*zonename=='l')) &&
       (is_number(zonename+1)))
    {
      page_string_block(zoneh_list_by_level(&sb,atoi(zonename+1)),ch);
      destroy_string_block(&sb);
    }
  /*
   * Else, try to find area by name or number and display specific info
   * about it.
   */
    else
    {
      if ( (zoneh=find_zoneh(zonename)) == NULL)
	send_to_char("That zone does not seem to exist.\n\r",ch);
      else
      {
	sprintf(buf,"\n\r%s by %s %s\n\rLevels %i through %i\n\r\n\r"
	      "Directions:\n\r%s\n\r\n\rOverview:\n\r%s\n\r\n\r"
		"Update info:\n\r%s\n\r",
		zoneh->name,zoneh->creator,zoneh->create_date,
		zoneh->min_level,zoneh->max_level,zoneh->directions,
		zoneh->description,zoneh->update_info);
	page_string(ch->desc,buf,0);
      }
    }
}

ACMD(do_points) {
  char buf[255];

  sprintf(buf, "You have %i hero points\r\n", assign_hero_status(ch));
  send_to_char_formatted(buf, ch);
}

ACMD(do_opstat) {
   char target[MAX_INPUT_LENGTH];
   int ovirt, oreal;
   char buf[MAX_STRING_LENGTH];
   Function *prg;

   if(!IS_IMMORTAL(ch))
     return;

   only_argument(arg, target);

   if(isdigit(*target)) {
      oreal = real_object(atoi(target));
   } else {
      for(oreal=0;oreal <= top_of_objt; oreal++)
        if(isname(target,obj_index[oreal].name))
 	  break;
   }

   if(oreal < 0 || oreal > top_of_objt) {
      cprintf(ch, "$CRThere is no such object.\n\r");
      return;
   }

   if(!obj_index[oreal].objprogs2) {
      cprintf(ch,"Has no objprogams2 attached to it.\n\r");
      cprintf(ch,"----------------------------------\n\r");
   } else {
      cprintf(ch,"Objprogs2:\n\r\n\r");

      for(prg = obj_index[oreal].objprogs2; prg; prg = prg->next) {
	 sprintf(buf,"Name: [%-20s], Num Args: [%-2d], Args: [%s]\n\r",
		 prg->name, prg->NumArgs, prg->argbuf);
	 strcat(buf,"--------------------------------------------------\n\r");
	 page_string(ch->desc,buf,1);

	 sprintf(buf,"%s\n\r", prg->code);
	 page_string(ch->desc,buf,1);
	 page_string(ch->desc,"--------------------------------------------------\n\r",1);
      }
   }
}

ACMD(do_rpstat) {
   char target[MAX_INPUT_LENGTH];
   long rvirt;
   char buf[MAX_STRING_LENGTH];
   Function *prg;

   if(!IS_IMMORTAL(ch))
     return;

   only_argument(arg, target);

   if(!isdigit(*target)) {
      cprintf(ch, "Room supplied must be a number.\n\r");
      return;
   } else {
      rvirt=atol(target);
   }

   if(rvirt < 0 || rvirt > top_of_world) {
      cprintf(ch, "$CRThere is no such room.\n\r");
      return;
   }

   if(!real_roomp(rvirt)) {
      cprintf(ch, "$CRThere is no such room.\n\r");
   } else if(!(prg=real_roomp(rvirt)->roomprogs2)) {
      cprintf(ch,"Has no roomprogams2 attached to it.\n\r");
      cprintf(ch,"-----------------------------------\n\r");
   } else {
      cprintf(ch,"Roomprogs2:\n\r\n\r");

      for(;prg;prg=prg->next) {
	 sprintf(buf,"Name: [%-20s], Num Args: [%-2d], Args: [%s]\n\r",
		 prg->name, prg->NumArgs, prg->argbuf);
	 strcat(buf,"--------------------------------------------------\n\r");
	 page_string(ch->desc,buf,1);

	 sprintf(buf,"%s\n\r", prg->code);
	 page_string(ch->desc,buf,1);
	 page_string(ch->desc,"--------------------------------------------------\n\r",1);
      }
   }
}
