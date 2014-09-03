#ifndef HERO_H
#define HERO_H

/* ************************************************************************
*  file: hero.h , Herolist(s).              Copyright (C) Paul Cole  1994 *
*  Usage: Creating/Maintaining/Inspecting herolists.                      *
************************************************************************* */

#include "structs.h"
#include "vnum.h"

#define MAX_HEROES 20

extern int herolist_changed;

struct hero_info {
  long exp;
  ush_int clss;
  char *race;
  sstring_t* name;
  char *titles;
  time_t logon;
  ubyte level;
  int points;
};

extern struct hero_info heroes[MAX_HEROES];

/**  Used to create AND to update the hero list. **/
void add_char_to_hero_list(struct char_data *ch);
int assign_hero_status(struct char_data *ch);

/** Inspect hero lists **/
int is_hero(struct char_data *ch);
int is_top_hero(struct char_data *ch);

#endif
