/***************************************************************************
 *  file: db.zonehelp.h , Zonehelp command, Part of Forbidden Lands Project *
 *  Usage : Zonehelp  commands.                                             *
 *  Copyright (C) 1994 Paul A Cole                                          *
 *    - see 'flgroup.license.doc' for complete information.                 *
 ***************************************************************************/

#define ZONEH_FILE        "tinyworld.hlp" /* for 'zones' command        */

struct zoneh_data
{
  char *name;
  int min_level, max_level;
  char *creator,
       *create_date,
       *update_date,
       *directions,
       *description,
       *update_info;
};

extern struct string_block zonehelplist;

void boot_zoneh();
struct zoneh_data *find_zoneh(char *arg);
struct string_block *zoneh_list_by_level(struct string_block *sb,int level);
