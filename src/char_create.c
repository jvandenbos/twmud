/* This File contains tables for use in character generation */

#include "config.h"
#include "char_create.h"
#include "language.h"
#include "cmdtab.h"
#include "race.h"


/* defines shown here only as a reminder that these exist in char_create.h*/
/* and must be changed there as appropriate                               */
/*   #define MAX_PLAYER_RACE 16                                           */

const int playerraces[] =
{ 
  RACE_HUMAN,       /* HUMAN */
  RACE_ELF,         /* ELF   */
  RACE_FOREST_ELF,  /* Forest_Elf */
  RACE_DROW,        /* Drow */
  RACE_HALF_ELF,    /* Half_Elf */
  RACE_DWARF,       /* DWARF */
  RACE_HOBBIT,      /* HOBBIT */
  RACE_GNOME,       /* GNOME */
  RACE_LYCANTH,     /* DOPPLE/LYCANTHROPE */
  RACE_HILL_GIANT,  /* Hill Giant */
  RACE_HALF_ORC,    /* Half_Orc */
  RACE_ORC,         /* Orc */
  RACE_PIXIE,       /* Pixie */
  RACE_AVIAN,	    /* Avaian */
  RACE_CANIS,	    /* Dog Race */
  RACE_FELIS,	    /* Cat Race */
  RACE_REPTILE,	    /* Reptiles */
  RACE_GOBLIN,	    /* Goblins */
  RACE_MINOTAUR,    /* Minotaurs */
  RACE_CYCLOPS,     /* Cyclops */
  RACE_SKEXIE,	    /* Skexies */
  RACE_STONE_GIANT, /* Stone Giant */
  RACE_FROST_GIANT, /* Frost Giant */
  RACE_FIRE_GIANT,  /* Fire Giant */
  RACE_CLOUD_GIANT, /* Cloud Giant */
  RACE_STORM_GIANT, /* Storm Giant */
  
  -1
};

const struct tongue_info_struct tongue_info[] = {
     { RACE_HUMAN,      {TONGUE_COMMON, -1} },
     { RACE_ELF,        {TONGUE_ELF, TONGUE_COMMON, TONGUE_DROW, TONGUE_DWARF, -1} },
     { RACE_FOREST_ELF, {TONGUE_ELF, TONGUE_COMMON, TONGUE_DROW, TONGUE_PIXIE, -1} },
     { RACE_DROW,       {TONGUE_DROW, TONGUE_ELF, TONGUE_COMMON, TONGUE_ORC, -1} },
     { RACE_HALF_ELF,   {TONGUE_COMMON, TONGUE_ELF, TONGUE_DWARF, -1} },
     { RACE_DWARF,      {TONGUE_DWARF, TONGUE_COMMON, TONGUE_ELF, TONGUE_GIANT, -1} },
     { RACE_HOBBIT,     {TONGUE_HOBBIT, TONGUE_COMMON, TONGUE_GNOME, -1} },
     { RACE_GNOME,      {TONGUE_GNOME, TONGUE_COMMON, TONGUE_HOBBIT, -1} },
     { RACE_LYCANTH,    {TONGUE_LYCANTH, TONGUE_COMMON, -1} },
     { RACE_HILL_GIANT, {TONGUE_GIANT, TONGUE_COMMON, TONGUE_DWARF, -1} },
     { RACE_HALF_ORC,   {TONGUE_ORC, TONGUE_COMMON, TONGUE_ELF, TONGUE_DWARF, -1} },
     { RACE_ORC,        {TONGUE_ORC, TONGUE_COMMON, TONGUE_DROW, TONGUE_DWARF, -1} },
     { RACE_PIXIE,      {TONGUE_PIXIE, TONGUE_COMMON, TONGUE_ELF, TONGUE_DWARF, TONGUE_GIANT, -1} },
     { RACE_AVIAN,      {TONGUE_COMMON, -1} },
     { RACE_CANIS,      {TONGUE_COMMON, -1} },
     { RACE_FELIS,      {TONGUE_COMMON, -1} },
     { RACE_REPTILE,    {TONGUE_COMMON, -1} },
     { RACE_GOBLIN,     {TONGUE_COMMON, -1} },
     { RACE_MINOTAUR,   {TONGUE_COMMON, -1} },
     { RACE_CYCLOPS,    {TONGUE_COMMON, -1} },
     { RACE_SKEXIE,     {TONGUE_COMMON, -1} },
     { RACE_STONE_GIANT,{TONGUE_COMMON, -1} },
     { RACE_FROST_GIANT,{TONGUE_COMMON, -1} },
     { RACE_FIRE_GIANT, {TONGUE_COMMON, -1} },
     { RACE_CLOUD_GIANT,{TONGUE_COMMON, -1} },
     { RACE_STORM_GIANT,{TONGUE_COMMON, -1} }
};

/* Max and minumum scores that a 3 six sided die rolls must fall in between
    in order to be a certain race. (Endpoints inclusive) */
const struct player_race_stats plyr_race_stats[] =
{ 
  /* Str,  Dex,  Con,   Int,  Wis,  Cha */
  { 3,18,  3,18, 3,18,  3,18, 3,18, 3,18}, /* HUMAN */
  { 3,18,  6,18, 7,18,  8,18, 3,18, 8,18}, /* ELF */
  { 4,18,  6,18, 6,18,  8,18, 3,18, 8,18}, /* Forest_Elf */
  { 3,18,  6,18, 4,18,  8,18, 6,18, 8,18}, /* Drow */
  { 3,18,  6,18, 6,18,  4,18, 3,18, 3,18}, /* Half_Elf */
  { 8,18,  3,17, 11,18, 3,18, 3,18, 3,17}, /* DWARF */
  { 7,18,  7,18, 10,18, 6,18, 3,17, 3,18}, /* HOBBIT */
  { 6,18,  3,18, 8,18,  6,18, 3,18, 3,18}, /* GNOME */
  { 3,18,  3,18, 3,18,  3,18, 3,18, 3,18}, /* Doppleganger */
  { 11,18, 4,18, 3,18,  3,17, 3,17, 3,16}, /* Hill Giant */
  { 5,18,  3,18, 5,18,  3,17, 3,17, 3,16}, /* Half_Orc */
  { 5,18,  3,18, 5,18,  3,17, 3,17, 3,16}, /* Orc */
  { 6,18,  3,18, 4,18,  3,18, 3,18, 3,18}, /* Pixie */
  { 3,17,  8,18, 3,18,  6,18, 6,18, 3,17}, /* Avian */
  { 5,18,  6,18, 4,18,  3,17, 3,17, 3,18}, /* Canis */
  { 3,17,  7,18, 3,18,  5,18, 5,18, 3,17}, /* Felis */
  { 3,18,  6,18, 3,18,  3,17, 3,17, 3,17}, /* Reptile */
  { 3,16,  9,18, 3,18,  5,18, 5,18, 3,17}, /* Goblin */
  { 9,18,  3,17, 5,18,  3,17, 3,17, 3,17}, /* Minotaur */
  { 9,18,  3,17, 5,18,  3,17, 3,17, 3,17}, /* Cyclops */
  { 3,16,  5,18, 3,18,  5,18, 5,18, 3,17}, /* Skexie */
  { 9,18,  3,16, 6,18,  3,17, 3,17, 3,18}, /* Stone_Giant */
  { 9,18,  3,16, 6,18,  3,17, 3,17, 3,18}, /* Frost_Giant */
  { 9,18,  3,16, 6,18,  3,17, 3,17, 3,18}, /* Fire_giant */
  { 9,18,  3,16, 6,18,  3,17, 3,17, 3,18}, /* Cloud_Giant */
  { 9,18,  3,16, 6,18,  3,17, 3,17, 3,18}, /* Storm_Giant */
};

/* Once a race is selected these numbers modify the base roll */
const struct player_race_mods plyr_race_mods[] =
{ 
  /* Str, Dex,  Con,   Int,  Wis,  Cha */
  {    0,   0,    0,     0,    0,    0}, /* HUMAN */
  {    0,  +1,   -1,     0,    0,    0}, /* ELF */
  {    0,  +1,   -3,     0,    0,   +1}, /* Forest_Elf */
  {    0,  +2,   -2,    +1,    0,   -1}, /* Drow */
  {    0,   0,    0,     0,    0,    0}, /* Half_Elf */
  {    0,   0,   +2,     0,    0,   -1}, /* DWARF */
  {   -1,  +1,    0,     0,    0,    0}, /* HOBBIT */
  {    0,   0,    0,    +1,   -1,    0}, /* GNOME */
  {   -1,   0,   -1,    +1,   +1 ,   0}, /* Doppleganger */
  {   +3,  -2,    0,    -1,   -1,    0}, /* Hill Giant */
  {   +1,   0,   -1,    -1,   +1,    0}, /* Half_Orc */
  {   +2,   0,   -1,     0,    0,   -1}, /* Orc */
  {   -4,  +3,   -2,    +1,    0,   -1}, /* Pixie */
  {   -2,  +2,   -1,    +1,   +1,    0}, /* Avian */
  {   +1,  +1,    0,    -1,   -1,    0}, /* Canis */
  {   -1,  -1,    0,    +1,   +1,    0}, /* Felis */
  {   +1,  +2,   -1,    -1,   -1,   -1}, /* Reptile */
  {   -2,  +2,   -2,    +2,   +2,   -3}, /* Goblin */
  {   +2,  +1,   +1,    -2,   -2,   -2}, /* Minotaur */
  {   +2,  -2,   +2,    -2,   -2,   -2}, /* Cyclops */
  {   -2,  +2,   -1,    +1,   +1,    0}, /* Skexie */
  {   +3,  -2,   +1,    -2,   -2,    0}, /* Stone_Giant */
  {   +3,  -2,   +1,    -2,   -2,    0}, /* Frost_Giant */
  {   +3,  -2,   +1,    -2,   -2,    0}, /* Fire_Giant */
  {   +3,  -2,   +1,    -2,   -2,    0}, /* Cloud_Giant */
  {   +3,  -2,   +1,    -2,   -2,    0}, /* Storm_Giant */ 

};

/* Allowed Classes for different races */
allowed_classes human_classes =
{ /* HUMAN */
  { "Warrior", CLASS_WARRIOR },
  { "Paladin", CLASS_PALADIN },
  { "Ranger", CLASS_RANGER },
  { "Mage", CLASS_MAGIC_USER },
  /*{ "Illusionist", CLASS_ILLUSIONIST */
  { "Cleric", CLASS_CLERIC },
  { "Druid", CLASS_DRUID },
  { "Thief", CLASS_THIEF },
  { "Bard", CLASS_BARD },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { NULL, -1 }
}; 

allowed_classes elf_classes =
{ /* ELF   */
  { "Warrior", CLASS_WARRIOR },
  { "Ranger", CLASS_RANGER },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Warrior/Thief/Mage", CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER },
  { "Mage/Thief", CLASS_MAGIC_USER | CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes forest_elf_classes =
{ /* Forest_Elf */
  { "Warrior", CLASS_WARRIOR },
  { "Ranger", CLASS_RANGER },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { "Ranger/Cleric", CLASS_RANGER | CLASS_CLERIC },
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Warrior/Thief/Mage", CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER },
  { "Mage/Thief", CLASS_MAGIC_USER | CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes drow_classes =
{ /* Drow */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Psionist", CLASS_PSI },
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { "Warrior/Thief/Mage", CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER },
  { "Mage/Thief", CLASS_MAGIC_USER | CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes half_elf_classes =
{ /* Half_Elf */
  { "Warrior", CLASS_WARRIOR },
  { "Ranger", CLASS_RANGER },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Druid", CLASS_DRUID },
  { "Thief", CLASS_THIEF },
  { "Bard", CLASS_BARD },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { "Warrior/Cleric", CLASS_CLERIC | CLASS_WARRIOR}, 
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
/*{ "Warrior/Druid", CLASS_DRUID | CLASS_WARRIOR},*/ 
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
/*{ "Cleric/Ranger", CLASS_RANGER |CLASS_CLERIC }, */
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Thief/Mage", CLASS_THIEF | CLASS_MAGIC_USER },
  { "Warrior/Mage/Cleric", CLASS_WARRIOR | CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Warrior/Mage/Thief", CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER },
  { NULL, -1 }
};

allowed_classes dwarf_classes =
{ /* DWARF */
  { "Warrior", CLASS_WARRIOR },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Warrior/Cleric", CLASS_WARRIOR | CLASS_CLERIC },
/*{ "Warrior/Psionist", CLASS_WARRIOR | CLASS_PSI },*/ 
/*{ "Thief/Psionist", CLASS_THIEF | CLASS_PSI },*/ 
  { NULL, -1 }
};

allowed_classes hobbit_classes =
{ /* HOBBIT */
  { "Warrior", CLASS_WARRIOR },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
/*{ "Warrior/Psionist", CLASS_WARRIOR | CLASS_PSI },*/ 
/*{ "Thief/Psionist", CLASS_THIEF | CLASS_PSI }, */
  { NULL, -1 }
};

allowed_classes gnome_classes =
{ /* GNOME */
  { "Warrior", CLASS_WARRIOR },
/*{ "Illusionist", CLASS_ILLUSIONIST */
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Warrior/Cleric", CLASS_WARRIOR | CLASS_CLERIC },
/*{ "Warrior/Illusionist", CLASS_WARRIOR | CLASS_ILLUSIONIST }, */
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
/*{ "Cleric/Illusionist", CLASS_CLERIC | CLASS_ILLUSIONIST }, */
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
/*{ "Illusionist/Thief", CLASS_THIEF | CLASS_ILLUSIONIST }, */
  { NULL, -1 }
};

allowed_classes doppleganger_classes =
{ /* DOPPLE/LYCANTHROPE */
  { "Shapeshifter", CLASS_SHIFTER },
  { NULL, -1 }
};

allowed_classes hill_giant_classes =
{ /* Hill Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes half_orc_classes =
{ /* Half_Orc */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Bard", CLASS_BARD },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { "Thief/Cleric", CLASS_CLERIC | CLASS_THIEF },
  { "Thief/Mage", CLASS_THIEF | CLASS_MAGIC_USER },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { "Warrior/Cleric", CLASS_WARRIOR | CLASS_CLERIC },
  { NULL, -1 }
};

allowed_classes orc_classes =
{ /* Orc */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Psionist", CLASS_PSI },
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { NULL, -1 }
};

allowed_classes pixie_classes =
{ /* Pixie */
  { "Cleric", CLASS_CLERIC },
  { "Druid", CLASS_DRUID },
  { "Mage", CLASS_MAGIC_USER },
  { "Thief", CLASS_THIEF },
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
  { "Thief/Mage", CLASS_THIEF | CLASS_MAGIC_USER },
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { NULL, -1 }
};

allowed_classes avian_classes =
{ /* Avian */
  { "Cleric", CLASS_CLERIC },
  { "Druid", CLASS_DRUID },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Bard", CLASS_BARD },
  { "Thief", CLASS_THIEF },
  { "Bard/Thief", CLASS_BARD | CLASS_THIEF },
  { "Ranger", CLASS_RANGER },
  { "Psionist", CLASS_PSI },
  { NULL, -1 }
};

allowed_classes canis_classes =
{ /* Canis */
  { "Warrior", CLASS_WARRIOR },
  { "Ranger", CLASS_RANGER },
  { "Thief", CLASS_THIEF },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Monk", CLASS_MONK }, 
  { "Bard", CLASS_BARD },
  { NULL, -1 }
};

allowed_classes felis_classes =
{ /* Felis */
  { "Cleric", CLASS_CLERIC },
  { "Druid", CLASS_DRUID },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Bard", CLASS_BARD },
  { "Psionist", CLASS_PSI },
  { "Monk", CLASS_MONK }, 
  { NULL, -1 }
};

allowed_classes reptile_classes =
{ /* Reptile */
  { "Warrior", CLASS_WARRIOR },
  { "Ranger", CLASS_RANGER },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Thief", CLASS_THIEF },
  { "Druid", CLASS_DRUID },
  { NULL, -1 }
};

allowed_classes goblin_classes =
{ /* Goblin */
  { "Warrior", CLASS_WARRIOR },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Warrior/Cleric", CLASS_WARRIOR | CLASS_CLERIC },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes minotaur_classes =
{ /* Minotaur */
  { "Warrior", CLASS_WARRIOR },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Mage", CLASS_MAGIC_USER },
  { "Warrior/Cleric", CLASS_WARRIOR | CLASS_CLERIC },
  { "Warrior/Thief", CLASS_WARRIOR | CLASS_THIEF },
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
  { "Warrior/Mage", CLASS_WARRIOR | CLASS_MAGIC_USER },
  { "Mage/Thief", CLASS_MAGIC_USER | CLASS_THIEF },
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Warrior/Thief/Mage", CLASS_WARRIOR | CLASS_THIEF | CLASS_MAGIC_USER },
  { "Warrior/Mage/Cleric", CLASS_WARRIOR | CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Mage/Cleric/Thief", CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes cyclops_classes =
{ /* Cyclops */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes skexie_classes =
{ /* Skexie */
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { "Cleric/Thief", CLASS_CLERIC | CLASS_THIEF },
  { "Mage/Thief", CLASS_MAGIC_USER | CLASS_THIEF },
  { "Cleric/Mage", CLASS_CLERIC | CLASS_MAGIC_USER },
  { "Ranger", CLASS_RANGER },
  { "Druid", CLASS_DRUID },
  { NULL, -1 }
};

allowed_classes stone_giant_classes =
{ /* Stone_Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes frost_giant_classes =
{ /* Frost_Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes fire_giant_classes =
{ /* Fire_Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes cloud_giant_classes =
{ /* Cloud_Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

allowed_classes storm_giant_classes =
{ /* Storm_Giant */
  { "Warrior", CLASS_WARRIOR },
  { "Mage", CLASS_MAGIC_USER },
  { "Cleric", CLASS_CLERIC },
  { "Thief", CLASS_THIEF },
  { NULL, -1 }
};

extern const struct class_by_race Classes_allowed_by_Race [MAX_PLAYER_RACE] = 
{
  { RACE_HUMAN, human_classes },
  { RACE_ELF, elf_classes },
  { RACE_FOREST_ELF, forest_elf_classes },
  { RACE_DROW, drow_classes },
  { RACE_HALF_ELF, half_elf_classes },
  { RACE_DWARF, dwarf_classes },
  { RACE_HOBBIT, hobbit_classes },
  { RACE_GNOME, gnome_classes },
  { RACE_LYCANTH, doppleganger_classes },
  { RACE_HILL_GIANT, hill_giant_classes },
  { RACE_HALF_ORC, half_orc_classes },
  { RACE_ORC, orc_classes },
  { RACE_PIXIE, pixie_classes },
  { RACE_AVIAN, avian_classes },
  { RACE_CANIS, canis_classes },
  { RACE_FELIS, felis_classes },
  { RACE_REPTILE, reptile_classes },
  { RACE_GOBLIN, goblin_classes },
  { RACE_MINOTAUR, minotaur_classes },
  { RACE_CYCLOPS, cyclops_classes },
  { RACE_SKEXIE, skexie_classes },
  { RACE_STONE_GIANT, stone_giant_classes },
  { RACE_FROST_GIANT, frost_giant_classes },
  { RACE_FIRE_GIANT, fire_giant_classes },
  { RACE_CLOUD_GIANT, cloud_giant_classes },
  { RACE_STORM_GIANT, storm_giant_classes }
};

/* These are the minimum attributes to be a a certain class
   Replaced 10 with 9 - Solaar
*/
const struct class_requirement plyr_class_requirements[] =
{  /* Str, Dex,  Con,  Int,  Wis,  Cha */
  {     0,   0,    0,    9,    0,    0 },  /* MAGE_LEVEL_IND    */
  {     0,   0,    0,    0,    9,    0 },  /* CLERIC_LEVEL_IND  */
  {     9,   0,    9,    0,    0,    0 },  /* WARRIOR_LEVEL_IND */
  {     0,   9,    0,    9,    0,    0 },  /* THIEF_LEVEL_IND   */
  {     9,   0,    9,    0,    9,    9 },  /* PALADIN_LEVEL_IND */
  {     0,   0,    0,    9,    9,    0 },  /* DRUID_LEVEL_IND   */
  {     0,   0,    9,    9,    9,    0 },  /* PSI_LEVEL_IND     */
  {     9,   9,    0,    0,    9,    0 },  /* RANGER_LEVEL_IND  */
  {     0,   9,    0,    0,    9,    0 },  /* SHIFTER_LEVEL_IND */
  {     9,   9,    9,    0,    9,    0 },  /* MONK_LEVEL_IND    */
  {     0,   9,    0,    0,    9,    9 },  /* BARD_LEVEL_IND    */
};

extern const struct race_align Race_Align [MAX_PLAYER_RACE] = 
{
  { RACE_HUMAN, ANY_ALIGN },
  { RACE_ELF, NEUTRAL| ANY_GOOD },
  { RACE_FOREST_ELF, NEUTRAL| ANY_GOOD },
  { RACE_DROW, ANY_EVIL },            /* Drow are evil by nature */
  { RACE_HALF_ELF, ANY_ALIGN },
  { RACE_DWARF, ANY_ALIGN },
  { RACE_HOBBIT, ANY_ALIGN },
  { RACE_GNOME, ANY_ALIGN },
  { RACE_LYCANTH, ANY_ALIGN },
  { RACE_HILL_GIANT, EXTREME_EVIL },  /* Thraxus says they evil */  
  { RACE_HALF_ORC, ANY_ALIGN },
  { RACE_ORC, EXTREME_EVIL | EVIL },  /* I don't know seem evil to me */
  { RACE_PIXIE, ANY_ALIGN },
  { RACE_AVIAN, ANY_ALIGN },
  { RACE_CANIS, ANY_ALIGN },
  { RACE_FELIS, ANY_ALIGN },
  { RACE_REPTILE, NEUTRAL| ANY_EVIL },
  { RACE_GOBLIN, NEUTRAL| ANY_EVIL },
  { RACE_MINOTAUR, NEUTRAL| ANY_EVIL },
  { RACE_CYCLOPS, EXTREME_EVIL | EVIL },
  { RACE_SKEXIE, ANY_ALIGN },
  { RACE_STONE_GIANT, EXTREME_EVIL },
  { RACE_FROST_GIANT, EXTREME_EVIL },
  { RACE_FIRE_GIANT, EXTREME_EVIL },
  { RACE_CLOUD_GIANT, EXTREME_EVIL },
  { RACE_STORM_GIANT, EXTREME_EVIL },
};

extern const struct class_align Class_Align [MAX_LEVEL_IND + 1] =
{
  { CLASS_MAGIC_USER, ANY_ALIGN },
  { CLASS_CLERIC, ANY_ALIGN },
  { CLASS_WARRIOR, ANY_ALIGN },
  { CLASS_THIEF, ANY_ALIGN },
  { CLASS_PALADIN, EXTREME_GOOD }, /* Paladins must be good */
  { CLASS_DRUID, ANY_ALIGN },
  { CLASS_PSI, ANY_ALIGN },
  { CLASS_RANGER, ANY_ALIGN },
  { CLASS_SHIFTER, ANY_ALIGN },
  { CLASS_MONK, ANY_ALIGN },
  { CLASS_BARD, ANY_ALIGN },
};

/* Basic general Character Descriptions for starting attributes */
extern const struct descript Race_Characteristic [MAX_PLAYER_RACE] =
{
  { RACE_HUMAN,
    { 60, 2, 10 },     /* Male Height */
    { 140, 6, 10 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 100, 6, 10 },    /* Female Weight */
    { 15, 1, 4 },      /* Age */
  },
  { RACE_ELF,
    { 55, 1, 10 },     /* Male Height */
    { 90, 3, 10 },     /* Male Weight */
    { 50, 1, 10 },     /* Female Height */
    { 70, 6, 10 },     /* Female Weight */
    { 100, 5, 6 },     /* Age */
  },
  { RACE_FOREST_ELF,
    { 55, 1, 10 },     /* Male Height */
    { 90, 3, 10 },     /* Male Weight */
    { 50, 1, 10 },     /* Female Height */
    { 70, 6, 10 },     /* Female Weight */
    { 100, 5, 6 },     /* Age */
  },
  { RACE_DROW,
    { 55, 1, 10 },     /* Male Height */
    { 90, 3, 10 },     /* Male Weight */
    { 50, 1, 10 },     /* Female Height */
    { 70, 6, 10 },     /* Female Weight */
    { 100, 5, 6 },     /* Age */
  },
  { RACE_HALF_ELF,
    { 60, 2, 6 },      /* Male Height */
    { 110, 3, 12 },    /* Male Weight */
    { 58, 2, 6 },      /* Female Height */
    { 85, 3, 12 },     /* Female Weight */
    { 15, 1, 6 },      /* Age */
  },
  { RACE_DWARF,
    { 43, 1, 10 },     /* Male Height */
    { 130, 4, 10 },    /* Male Weight */
    { 41, 1, 10 },     /* Female Height */
    { 105, 4, 10 },    /* Female Weight */
    { 40, 5, 6 },      /* Age */
  },
  { RACE_HOBBIT,
    { 32, 2, 8 },      /* Male Height */
    { 52, 5, 4 },      /* Male Weight */
    { 30, 2, 8 },      /* Female Height */
    { 48, 5, 4 },      /* Female Weight */
    { 20, 3, 4 },      /* Age */
  },
  { RACE_GNOME,
    { 38, 1, 6 },      /* Male Height */
    { 72, 5, 4 },      /* Male Weight */
    { 36, 1, 6 },      /* Female Height */
    { 68, 5, 4 },      /* Female Weight */
    { 60, 3, 12 },     /* Age */
  },
  { RACE_LYCANTH,
    { 60, 2, 10 },     /* Male Height */
    { 140, 6, 10 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 100, 6, 10 },    /* Female Weight */
    { 15, 1, 4 },      /* Age */
  },
  { RACE_HILL_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  },
  { RACE_HALF_ORC,
    { 60, 2, 10 },     /* Male Height */
    { 150, 5, 13 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 140, 5, 13 },    /* Female Weight */
    { 10, 1, 4 },      /* Age */
  },
  { RACE_ORC,
    { 60, 2, 10 },     /* Male Height */
    { 150, 5, 13 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 140, 5, 13 },    /* Female Weight */
    { 10, 1, 4 },      /* Age */
  },
  { RACE_PIXIE, 
    { 24, 1, 12 },     /* Male Height */
    { 24, 2, 12 },     /* Male Weight */
    { 22, 1, 12 },     /* Female Height */
    { 22, 2, 10 },     /* Female Weight */
    { 20, 3, 4 },      /* Age */
  },
  { RACE_AVIAN,
    { 38, 1, 6 },      /* Male Height */
    { 72, 5, 4 },      /* Male Weight */
    { 36, 1, 6 },      /* Female Height */
    { 68, 5, 4 },      /* Female Weight */
    { 60, 3, 12 },     /* Age */
  },
  { RACE_CANIS,
    { 38, 1, 6 },      /* Male Height */
    { 72, 5, 4 },      /* Male Weight */
    { 36, 1, 6 },      /* Female Height */
    { 68, 5, 4 },      /* Female Weight */
    { 60, 3, 12 },     /* Age */
  },
  { RACE_FELIS,
    { 38, 1, 6 },      /* Male Height */
    { 72, 5, 4 },      /* Male Weight */
    { 36, 1, 6 },      /* Female Height */
    { 68, 5, 4 },      /* Female Weight */
    { 60, 3, 12 },     /* Age */
  },
  { RACE_REPTILE,
    { 60, 2, 10 },     /* Male Height */
    { 140, 6, 10 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 100, 6, 10 },    /* Female Weight */
    { 15, 1, 4 },      /* Age */
  },
  { RACE_GOBLIN,
    { 38, 1, 6 },      /* Male Height */
    { 72, 5, 4 },      /* Male Weight */
    { 36, 1, 6 },      /* Female Height */
    { 68, 5, 4 },      /* Female Weight */
    { 60, 3, 12 },     /* Age */
  },
  { RACE_MINOTAUR,
    { 60, 2, 10 },     /* Male Height */
    { 150, 5, 13 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 140, 5, 13 },    /* Female Weight */
    { 10, 1, 4 },      /* Age */
  },
  { RACE_CYCLOPS,
    { 60, 2, 10 },     /* Male Height */
    { 150, 5, 13 },    /* Male Weight */
    { 59, 2, 10 },     /* Female Height */
    { 140, 5, 13 },    /* Female Weight */
    { 10, 1, 4 },      /* Age */
  },
  { RACE_SKEXIE,
    { 24, 1, 12 },     /* Male Height */
    { 24, 2, 12 },     /* Male Weight */
    { 22, 1, 12 },     /* Female Height */
    { 22, 2, 10 },     /* Female Weight */
    { 20, 3, 4 },      /* Age */
  },
  { RACE_STONE_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  },
  { RACE_FROST_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  },
  { RACE_FIRE_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  },
  { RACE_CLOUD_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  },
  { RACE_STORM_GIANT,
    { 115, 5, 12 },    /* Male Height */
    { 1800, 4, 100 },  /* Male Weight */
    { 132, 6, 12 },    /* Female Height */
    { 1500, 5, 100 },  /* Female Weight */
    { 28, 2, 10 },     /* Age */
  }
};
