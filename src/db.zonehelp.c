/***************************************************************************
 *  file: db.zonehelp.c , Zonehelp command, Part of Forbidden Lands Project *
 *  Usage : Zonehelp  commands.                                             *
 *  Copyright (C) 1994 Paul A Cole                                          *
 *    - see 'flgroup.license.doc' for complete information.                 *
 ***************************************************************************/
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
#include "constants.h"
#include "util_str.h"
#include "db.zonehelp.h"
#include "sblock.h"

/* globals */
long total_zbc;
struct index_data *zoneh_index;	/* index table for zone help file     */
int top_of_zoneht = 0;		/* top of zone help index table       */
FILE* zoneh_f;			/* zone help file                     */

struct string_block zonehelplist;

/* forward declarations */
struct zoneh_data* read_zoneh(int nr);
struct zoneh_data* parse_zoneh(FILE* zoneh_f, int nr);
void clear_zoneh(struct zoneh_data *zoneh);
void free_zoneh(struct zoneh_data *zoneh);

void boot_zoneh()
{
  char p[MAX_STRING_LENGTH*4];
  int i;
  struct zoneh_data *zoneh;
  struct string_block *sb;

  sb=&zonehelplist;
  init_string_block(sb);
  
    if (!(zoneh_f = fopen(ZONEH_FILE, "r")))	{
      perror(ZONEH_FILE);
      exit(0);
    }

  zoneh_index = generate_indices(zoneh_f, &top_of_zoneht);

  sprintf(p,"\n\r  %3s  %-25.25s   %3s  %3s  %-16.16s  %-9.9s\n\r",
	  "Num","Name","Min","Max","Creator","Updated");
  append_to_string_block(sb,p);
  sprintf(p,"-----------------------------------------------------------------------------");
  append_to_string_block(sb,p);
  
  for(i=0; i<top_of_zoneht; i++)
  {
    zoneh=read_zoneh(i);
    sprintf(p,"\n\r  %3i  %-25.25s   %3i  %3i  %-16.16s  %-9.9s",
	    i,zoneh->name,zoneh->min_level,
	    zoneh->max_level,zoneh->creator,zoneh->update_date);
    append_to_string_block(sb,p);
    free_zoneh(zoneh);
  }
  sprintf(p,"\n\r");
  append_to_string_block(sb,p);
}

/* read a zonehelp from ZONEH_FILE */
/* parm is the real num of zoneh */
struct zoneh_data *read_zoneh(int nr)
{
    fseek(zoneh_f, zoneh_index[nr].pos, 0);
    return parse_zoneh(zoneh_f, nr);
}

struct zoneh_data* parse_zoneh(FILE* zoneh_f, int nr)
{
  struct zoneh_data *zoneh;
  long bc;

  CREATE(zoneh, struct zoneh_data, 1);
  bc = sizeof(struct zoneh_data);
  clear_zoneh(zoneh);
  
  /***** String data *** */
  
  zoneh->name = fread_string(zoneh_f);
  if (!zoneh->name)
    zoneh->name=strdup("");
  bc += strlen(zoneh->name);
  fscanf(zoneh_f,"%i %i\n",&(zoneh->min_level),&(zoneh->max_level));
  zoneh->creator = fread_string(zoneh_f);
  if (!zoneh->creator)
    zoneh->creator=strdup("");
  bc += strlen(zoneh->creator);
  zoneh->create_date = fread_string(zoneh_f);
  if (!zoneh->create_date)
    zoneh->create_date=strdup("");
  bc += strlen(zoneh->create_date);
  zoneh->update_date = fread_string(zoneh_f);
  if (!zoneh->update_date)
    zoneh->update_date=strdup("");
  bc += strlen(zoneh->update_date);
  zoneh->directions = fread_string(zoneh_f);
  if (!zoneh->directions)
    zoneh->directions=strdup("");
  bc += strlen(zoneh->directions);
  zoneh->description = fread_string(zoneh_f);
  if (!zoneh->description)
    zoneh->description=strdup("");
  zoneh->update_info = fread_string(zoneh_f);
  bc += strlen(zoneh->description);
  if (!zoneh->update_info)
    zoneh->update_info=strdup("");
  bc += strlen(zoneh->update_info);

#if BYTE_COUNT
  fprintf(stderr, "ZoneHelp [%d] uses %d bytes\n", zoneh_index[nr].virt, bc);
#endif
  total_zbc += bc;
  return (zoneh);  
}

/* release memory allocated for an obj struct */
void free_zoneh(struct zoneh_data *zoneh)
{
    FREE(zoneh->name);
    FREE(zoneh->creator);
    FREE(zoneh->create_date);
    FREE(zoneh->update_date);
    FREE(zoneh->directions);
    FREE(zoneh->description);
    
    FREE(zoneh);
}

/* complete clear out an zoneh */
void clear_zoneh(struct zoneh_data *zoneh)
{
    memset(zoneh, '\0', sizeof(struct zoneh_data));
}


/* returns the real number of the zoneh with given virtual number */
int real_zoneh(int virt)
{
      int bot, top, mid;

    bot = 0;
    top = top_of_zoneht;

    /* perform binary search on zoneh-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((zoneh_index + mid)->virt == virt)
	    return(mid);
	if (bot >= top)
	    return(-1);
	if ((zoneh_index + mid)->virt > virt)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

struct zoneh_data *find_zoneh(char *arg)
{

  int zonehn;
  struct zoneh_data *zoneh;

  if(!arg)
    return NULL;
  
  if(is_number(arg))
  {
    zonehn=atoi(arg);
    if((zonehn<0)||(zonehn>=top_of_zoneht))
      return NULL;
    
/*
   if( (zonehn=real_zoneh(atoi(arg))) == -1)
     return NULL;
*/
  }
  else  
    for (zonehn=top_of_zoneht; zonehn>=0; zonehn--) 
      if (isname(arg,zoneh_index[zonehn].name))
	break;

  if(zonehn<0)
    return(NULL);
  else
    return( zoneh=read_zoneh(zonehn) );
}

struct string_block *zoneh_list_by_level(struct string_block *sb,int level)
{
  char p[MAX_STRING_LENGTH];
  struct zoneh_data *zoneh;
  int i;
  
  init_string_block(sb);
        
  sprintf(p,"\n\r  %3s  %-25.25s   %3s  %3s  %-16.16s  %-9.9s\n\r",
	  "Num","Name","Min","Max","Creator","Updated");
  append_to_string_block(sb,p);
  sprintf(p,"-----------------------------------------------------------------------------");
  append_to_string_block(sb,p);
  
  for(i=0; i<top_of_zoneht; i++)
  {
    zoneh=read_zoneh(i);
    if( (zoneh->min_level <= level) && (level <= zoneh->max_level) )
    {
      sprintf(p,"\n\r  %3i  %-25.25s   %3i  %3i  %-16.16s  %-9.9s",
	      zoneh_index[i].virt,zoneh->name,zoneh->min_level,
	      zoneh->max_level,zoneh->creator,zoneh->update_date);
      append_to_string_block(sb,p);
    }
    free_zoneh(zoneh);
  }
  sprintf(p,"\n\r");
  append_to_string_block(sb,p);
  return(sb);
}
