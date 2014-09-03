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
#include "utility.h"

/* generate index table for object or monster file */
struct index_data *generate_indices(FILE *fl, int *top)
{
    int i = 0;
    long bc=1500;
    struct index_data *index = NULL;
    char buf[82];
  
    rewind(fl);
  
    while (fgets(buf, sizeof(buf), fl))
    {
	if (*buf == '#')
	{
	    if (!i)		/* first cell */
		CREATE(index, struct index_data, bc);
	    else if (i >= bc)
	    {
		RECREATE(index, struct index_data, bc + 50);
		bc += 50;
	    }
	    sscanf(buf, "#%d", &index[i].virt);
	    index[i].pos = ftell(fl);
	    index[i].number = 0;
	    index[i].limit = -1;
	    index[i].func = 0;
	    index[i].name = (char *) ((index[i].virt<99999)?fread_string(fl):"omega");
	    index[i].proto = 0;
	    index[i].progtypes = 0;
	    index[i].mobprogs = NULL;
	    index[i].mobprogs2 = NULL;
	    index[i].objprogs2 = NULL;

	    i++;
	}
    }

    *top = i - 1;
    return(index);
}

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
    char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
    char *rslt;
    register char *point;
    int flag;

    bzero(buf, sizeof(buf));
    
    do	{
	if (!fgets(tmp, MAX_STRING_LENGTH, fl))		{
	    perror("fread_str");
	    log_msg("File read error.");
	    return("Empty");
	}
    
	if (strlen(tmp) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
	    log_msg("fread_string: string too large (db.c)");
	    exit(0);
	} else
	    strcat(buf, tmp);
    
	for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	     point--);		
	if ((flag = (*point == '~')))
	    if (*(buf + strlen(buf) - 3) == '\n')
	    {
		*(buf + strlen(buf) - 2) = '\r';
		*(buf + strlen(buf) - 1) = '\0';
	    }
	    else
		*(buf + strlen(buf) -2) = '\0';
	else
	{
	    *(buf + strlen(buf) + 1) = '\0';
	    *(buf + strlen(buf)) = '\r';
	}
    } while (!flag);
  
    /* do the allocate boogie  */
  
    if (strlen(buf) > 0)    {
	CREATE(rslt, char, strlen(buf) + 1);
	strcpy(rslt, buf);
    }  else
	rslt = 0;
    return(rslt);
}


/* log a syntax error while reading a mob/object */
void log_syntax(const char* file, int ident,
		const char* message, char* found)
{
    char buf[512];
    char* ptr;
    
    sprintf(buf, "Syntax Error at Identifier: %d in File: %s", ident, file);
    log_msg(buf);
    if((ptr = strchr(found, '\n')))
	*ptr = 0;
    sprintf(buf, "%s: at text \"%s\"", message, found);
    log_msg(buf);
}

/* log a syntax error cause we didn't see what we expected */
void log_expect(const char* file, int ident,
		const char* expect, const char* found)
{
    char buf[512];
    
    sprintf(buf, "Expected %s", expect);
    // log_syntax(file, ident, buf, found);
}

/* read a number from a buffer and complain if we don't get one */
int parse_number(const char* file, const char* field, int ident,
		 const char** ptr, long* num)
{
    char*	next;
    
    *num = strtol(*ptr, &next, 10);
    if(*ptr == next)
    {
	log_expect(file, ident, field, *ptr);
	return 0;
    }

    *ptr = next;
    
    return 1;
}

int parse_unumber(const char* file, const char* field, int ident,
		 const char** ptr, unsigned long* num)
{
    char*	next;
    
    *num = strtoul(*ptr, &next, 10);
    if(*ptr == next)
    {
	log_expect(file, ident, field, *ptr);
	return 0;
    }

    *ptr = next;
    
    return 1;
}
/* parse a die specifier (of the form %dD%d+%d) */
int parse_dice(const char* file, int ident,
	       const char** ptr,
	       long* cnt, long* size, long* mod)
{
    if(!parse_number(file, "die count", ident, ptr, cnt))
	return 0;
    if((**ptr != 'D') && (**ptr != 'd'))
    {
	log_expect(file, ident, "d", *ptr);
	return 0;
    }
    (*ptr)++;

    if(!parse_number(file, "die size", ident, ptr, size))
	return 0;
    if(isspace(**ptr) || (**ptr == '\n') || !**ptr)
	*mod = 0;
    else if((**ptr != '+') && (**ptr != '-'))
    {
	log_expect(file, ident, "+/-", (*ptr) - 1);
	return 0;
    }
    else if(!parse_number(file, "die mod", ident, ptr, mod))
	return 0;

    return 1;
}

/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i;

    EACH_OBJECT(iter, i)
    {
	if (i->item_number == nr) 
	    break;
    }
    END_AITER(iter);
    
    return i;
}

struct char_data* real_character(struct char_data* ch)
{
    struct char_data *temp = ch;
   
    if(!temp)
	return NULL;
    
    while(temp->orig)
      temp = temp->orig;
       
    return temp;
}

struct char_data *charm_master(struct char_data *ch) {
   struct char_data *temp = ch;
   
   if(!temp) return NULL;
   
   while(temp->master && IS_AFFECTED(temp, AFF_CHARM))
     temp = temp->master;
   
   return temp;
}

bool mob_wait_state(struct char_data *ch, int cycle) {

  if(IS_SET(ch->specials.mob_act, ACT_POLYSELF))
    return FALSE;

  if (ch)
    if (IS_NPC(ch)) {
      ch->fight_delay = cycle;
      return TRUE;
    }
  
  return FALSE;
}
