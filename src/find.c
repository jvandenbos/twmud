#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "find.h"
#include "utility.h"
#include "util_str.h"
#include "multiclass.h"
#include "string.h"


struct char_data* find_player_in_world(const char* name)
{
    struct char_data* tmp_ch;

    EACH_CHARACTER(iter, tmp_ch)
    {
	if(IS_PC(tmp_ch) && !str_cmp(name, GET_REAL_NAME(tmp_ch)))
	    break;
    }
    END_AITER(iter)

    return tmp_ch;
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
  
    if (!(number = get_number(&tmp)))
	return(0);
  
    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, OBJ_NAME(i))) {
	    if (j == number) 
		return(i);
	    j++;
	}
  
    return(0);
}

/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i;
  
    for (i = list; i; i = i->next_content)
	if (i->item_number == num) 
	    return(i);
  
    return(0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
    struct obj_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    EACH_OBJECT(iter, i)
    {
	if (isname(name, OBJ_NAME(i)))
	{
	    return i;
	}
    }
    END_AITER(iter);
    
    return NULL;
}

/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    for (i = real_roomp(room)->people, j = 1; i && (j <= number);
	 i = i->next_in_room)
	if (isname(tmp, GET_REAL_NAME(i))) {
	    if (j == number)
		return(i);
	    j++;
	}
  
    return(0);
}

/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
    struct char_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    EACH_CHARACTER(iter, i)
    {
	if (isname(tmp, GET_IDENT(i)))
	{
	    if (++j == number)
		break;
	}
    }
    END_AITER(iter);
	
    return i;
}

/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i=NULL;
  
    EACH_CHARACTER(iter, i)
    {
	if (i->nr == nr)
	    break;
    }
    END_AITER(iter);
    
    return i;
}

/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functionst
   which incorporate the actual player-data.
   *********************************************************************** */
struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    if (!str_cmp(name, "self") || !str_cmp(name, "me"))
      return ch;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    for (i = real_roomp(ch->in_room)->people, j = 1; 
	 i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_IDENT(i)))
	    if (CAN_SEE(ch, i))	{
		if (j == number) 
		    return(i);
		j++;
	    }
  
    return(0);
}

/* get a character from anywhere in the world, doesn't care much about
   being in the same room... */
struct char_data *get_char_vis_world(struct char_data *ch, char *name,
				     int *count)
     
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    j = count ? *count : 1;

    EACH_CHARACTER(iter, i)
    {
	if (isname(tmp, GET_IDENT(i)))
	    if (CAN_SEE(ch, i))	{
		if (j == number)
		    break;
		j++;
	    }
    }
    END_AITER(iter);
    
    if (count && i) *count = j;
    return i;
}

struct char_data *get_char_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
  
    /* check location */
    if ((i = get_char_room_vis(ch, name)))
	return(i);
  
    return get_char_vis_world(ch,name, NULL);
}

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
				     struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if(isname(tmp, OBJ_NAME(i)) && CAN_SEE_OBJ(ch, i))
	{
	    if (j == number)
		return(i);
	    j++;
	}
    return(0);
}

struct obj_data *get_obj_vis_world(struct char_data *ch, char *name,
				   int *count)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
    
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    j = count ? *count : 1;
  
    /* ok.. no luck yet. scan the entire obj list   */
    EACH_OBJECT(iter, i)
    {
	if(CAN_SEE_OBJ(ch, i) && isname(tmp, OBJ_NAME(i)))
	{
	    if (j == number)
		break;
	    j++;
	}
    }
    END_AITER(iter);
    
    if (!i && count)
	*count = j;

    return i;
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *i;
  
    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
	return(i);
  
    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)))
	return(i);
  
    return get_obj_vis_world(ch, name, NULL);
}

struct obj_data *get_obj_in_equip_vis(struct char_data* ch, char* name)
{
    int i;
    struct obj_data* obj;
  
    for(i = 0 ; i < MAX_WEAR ; ++i)
    {
	if((obj = ch->equipment[i]) &&
	   isname(name, OBJ_NAME(obj)) &&
	   CAN_SEE_OBJ(ch, obj))
	    return obj;
    }
    return NULL;
}

struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;
  
    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);
  
    /* scan items carried */
    for (i = ch->carrying, j=1; i && j<=number; i = i->next_content)
	if (isname(tmp, OBJ_NAME(i)) && CAN_SEE_OBJ(ch, i))
	    if (j == number)
		return(i);
	    else
		j++;
    for (i = real_roomp(ch->in_room)->contents; i && j<=number;
	 i = i->next_content)
	if (isname(tmp, OBJ_NAME(i)) && CAN_SEE_OBJ(ch, i))
	    if (j==number)
		return(i);
	    else
		j++;
    return 0;
}




/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
		 struct char_data **tar_ch, struct obj_data **tar_obj)
{
    static const char *ignore[] = {
	"the",
	"in",
	"on",
	"at",
	"\n" };
  
    int i;
    char name[256];
    bool found;
  
    found = FALSE;
  
    /* Eliminate spaces and "ignore" words */
    while (*arg && !found)
    {
	for(; *arg == ' '; arg++)
	    ;
    
	for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)
	    ;
	name[i] = 0;
	arg+=i;
	if (search_block(name, ignore, TRUE) > -1)
	    found = TRUE;
    }
  
    if (!name[0])
	return(0);
  
    *tar_ch  = 0;
    *tar_obj = 0;
  
    if (IS_SET(bitvector, FIND_CHAR_ROOM)) { /* Find person in room */
	if ((*tar_ch = get_char_room_vis(ch, name))) {
	    return(FIND_CHAR_ROOM);
	}
    }
  
    if (IS_SET(bitvector, FIND_CHAR_WORLD))
    {
	if ((*tar_ch = get_char_vis(ch, name))) {
	    return(FIND_CHAR_WORLD);
	}
    }
  
    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
	for(found=FALSE, i=0; i<MAX_WEAR && !found; i++)
	    if (ch->equipment[i] &&
		!str_cmp(name, OBJ_NAME(ch->equipment[i]))) {
		*tar_obj = ch->equipment[i];
		found = TRUE;
	    }
	if (found) {
	    return(FIND_OBJ_EQUIP);
	}
    }
  
    if (IS_SET(bitvector, FIND_OBJ_INV)) {
	if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
	    if ((*tar_obj = get_obj_vis_accessible(ch, name))) {
		return(FIND_OBJ_INV);
	    }
	} else {
	    if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
		return(FIND_OBJ_INV);
	    }
	}
    }
  
    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
	if ((*tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))) {
	    return(FIND_OBJ_ROOM);
	}
    }
  
    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
	if ((*tar_obj = get_obj_vis(ch, name))) {
	    return(FIND_OBJ_WORLD);
	}
    }
  
    return(0);
}
