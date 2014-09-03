#ifndef _CHAR_CREATE_H
#define  _CHAR_CREATE_H
#include "structs.h"

#define NUM_CLASSES 12
#define MAX_PLAYER_RACE 26
#define NUM_SEXES 2                   /* Who knows neuter? */
#define rollD6 number(1,6)

#define EXTREME_EVIL (1<<0)
#define EVIL         (1<<1)
#define NEUTRAL_EVIL (1<<2)
#define NEUTRAL      (1<<3)
#define NEUTRAL_GOOD (1<<4)
#define GOOD         (1<<5)
#define EXTREME_GOOD (1<<6)

#define NUM_OF_ALIGN_CHOICES 6

/* Defines for alignment choices */
#define ANY_EVIL    EXTREME_EVIL | EVIL | NEUTRAL_EVIL 
#define ANY_NEUTRAL NEUTRAL_EVIL | NEUTRAL | NEUTRAL_GOOD
#define ANY_GOOD    NEUTRAL_GOOD | GOOD | EXTREME_GOOD

#define ANY_ALIGN   EXTREME_EVIL | EVIL | NEUTRAL_EVIL | NEUTRAL | NEUTRAL_GOOD | GOOD | EXTREME_GOOD
			
typedef char sbyte;

/* The min and max roll on 3 6 sided die rolls to be this race. */
struct player_race_stats
{
  sbyte min_str;
  sbyte max_str;
  sbyte min_dex;
  sbyte max_dex;
  sbyte min_con;
  sbyte max_con;
  sbyte min_int;
  sbyte max_int;
  sbyte min_wis;
  sbyte max_wis;
  sbyte min_cha;
  sbyte max_cha;
};

/* Modifiers to the base rolls for choosing a certain race */
struct player_race_mods
{
  sbyte str_mod;
  sbyte dex_mod;
  sbyte con_mod;
  sbyte int_mod;
  sbyte wis_mod;
  sbyte cha_mod;
};

/* Minimum requirements to qualify to be a certain class */
struct class_requirement
{
  sbyte str_req;
  sbyte dex_req;
  sbyte con_req;
  sbyte int_req;
  sbyte wis_req;
  sbyte cha_req;
};

/* The different classes various classes can be */
struct allowed_class
{
  char *class_name;
  int class_type;
};

typedef const struct allowed_class allowed_classes []; 

struct class_by_race
{
  int race;
  const struct allowed_class *Classes;
};

/* The allowed alignment for the different races */
struct race_align
{
  int race;
  int align;
};

/* The allowed alignment for the different classes */
struct class_align
{
  int class_type;
  int align;
};

struct base_with_mod
{
  int base;
  int min_mod;
  int max_mod;
};

/* Basic character description information */
struct descript
{
  int Race;
  struct base_with_mod Male_Height;
  struct base_with_mod Male_Weight;
  struct base_with_mod Female_Height;
  struct base_with_mod Female_Weight;
  struct base_with_mod Age;
};


extern const struct descript Race_Characteristic [MAX_PLAYER_RACE];
extern const struct class_align Class_Align [MAX_LEVEL_IND + 1];
extern const struct race_align Race_Align [MAX_PLAYER_RACE];
extern const struct class_by_race Classes_allowed_by_Race [MAX_PLAYER_RACE]; 
extern const int playerraces[];
extern const struct tongue_info_struct tongue_info[];
extern const struct player_race_stats plyr_race_stats[];
extern const struct player_race_mods plyr_race_mods[];
extern const struct class_requirement plyr_class_requirements[];

#endif
