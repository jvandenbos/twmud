
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "proto.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "spells.h"
#include "handler.h"
#include "multiclass.h"
#include "spelltab.h"
#include "constants.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "sound.h"
#include "spec.h"
#include "modify.h"
#include "find.h"
#include "periodic.h"
#include "util_str.h"
#include "spell_util.h"
#include "utility.h"

#define MANA_MU 1
#define MANA_CL 1

/* forward declarations */
int CanLearn(struct char_data* ch, struct spell_info* spell);
void stop_follower(struct char_data *ch);
void mprog_spell_trigger(struct char_data *mob, struct char_data *ch, int spell_num);

int SpellInBook(struct char_data *ch, int spellnum);
void UpdateBook(struct char_data *ch, int spelnum);



/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, si) (si->min_usesmana)

const ubyte saving_throws[MAX_LEVEL_IND+1][5][ABS_MAX_LVL] = {
{
/* mage */
 /* Para */
 {18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7},
 /* Rod */
 {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
 /* Petrification */
 {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
 /* Breath */
 {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14},
 /* Spell */
 {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6}

}, 

/* cleric */
{
 /* Para */
 {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
 /* Rod */
 {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8},
 /* Petrification */
 {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
 /* Breath */
 {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14},
 /* Spell */
 {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5}

}, 
/* warrior */
{
  /* Para */
 {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
 /* Rod */
 {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8},
 /* Petrification */
 {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
 /* Breath */
 {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14},
 /* Spell */
 {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5}
}, 
/* thief */
{
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7},
  {18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3}
},
/* paladin */
{
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8}
},
 /* druid */
{
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8},
  {18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11}
}, /* psi */
{
  {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11},
  {18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7}
}, 

/* ranger */
{
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8}
}, 

/* shifter */
{
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11}
},
/* Bard */
{
  {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
  {18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11},
  {18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8},
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3}
},
/*Monk*/
{
  {18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11}
}
};

int BestSaveThrow(struct char_data *ch, int saveType)
{
  int saving_throw = 20;
  int cur_saving_throw = saving_throw;
 
  
 saving_throw = saving_throws[MAGE_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch, MAGE_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
     cur_saving_throw = saving_throw;
 
 saving_throw = saving_throws[CLERIC_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch, CLERIC_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	 cur_saving_throw = saving_throw;

 saving_throw = saving_throws[DRUID_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch, DRUID_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[PSI_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,PSI_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[BARD_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,BARD_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[SHIFTER_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,SHIFTER_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;
  
 saving_throw = saving_throws[PALADIN_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,PALADIN_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[RANGER_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,RANGER_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[MONK_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,MONK_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[THIEF_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,THIEF_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 saving_throw = saving_throws[WARRIOR_LEVEL_IND][saveType][(int)MIN(GET_LEVEL(ch,WARRIOR_LEVEL_IND),125)];
 if (saving_throw < cur_saving_throw)
	cur_saving_throw = saving_throw;

 return cur_saving_throw; 
  
}
int SPELL_LEVEL(struct char_data *ch, struct spell_info* spell)
{
    int i, lev = 61, mask = 1;
    
    for(i = 0 ; i <= MAX_LEVEL_IND ; ++i, mask <<= 1)
    {
	if(HasClass(ch, mask) && (spell->min_level[i] < lev))
	    lev = spell->min_level[i];
    }

    return lev;
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
	struct char_data *k;

	for(k=victim; k; k=k->master) {
		if (k == ch)
			return(TRUE);
	}

	return(FALSE);
}

/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (!ch->master) return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You no longer follow $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n no longer follows $N!", FALSE, ch, 0, ch->master, TO_ROOM);
    act("$n no longer follows you!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
    if (affected_by_spell(ch, SKILL_HYPNOSIS))
      affect_from_char(ch, SKILL_HYPNOSIS);
    if (IS_AFFECTED(ch, AFF_CHARM))
      REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    if (!IS_SET(ch->specials.flags,PLR_STEALTH)) {
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
    }
  }
  
  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    FREE(k);
  } else { /* locate follower who is not head of list */
    
    for(k = ch->master->followers; k->next && k->next->follower!=ch; 
	k=k->next)  
      ;
    
    if (k->next) {
      j = k->next;
      k->next = j->next;
      FREE(j);
    }
  }
  
  ch->master = 0;
  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
    struct follow_type *j, *k;

    if (ch->master)
	stop_follower(ch);

    for (k=ch->followers; k; k=j) {
	j = k->next;
	stop_follower(k->follower);
    }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader, int shadow)
{
	struct follow_type *k;

	assert(!ch->master);

	ch->master = leader;

	CREATE(k, struct follow_type, 1);

	k->follower = ch;
	k->next = leader->followers;
	leader->followers = k;

        if (shadow) 
          act("You start shadowing $N.", FALSE, ch, 0, leader, TO_CHAR);
	else
	  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);

	/* remove group flag !  7-16-92 mb */

	if(IS_AFFECTED(ch, AFF_GROUP))
	  REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);

        if (!IS_SET(ch->specials.flags, PLR_STEALTH) && (!shadow)) {
	   act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
	   act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
	}
}

void say_spell( struct char_data *ch, struct spell_info* spell)
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  int j, offs;
  struct char_data *temp_char;

  struct syllable {
    char org[10];
    char repl[10];
  };

  struct syllable syls[] = {
    { " ", " " },
    { "ar", "abra"   },
    { "au", "kada"    },
    { "bless", "fido" },
    { "blind", "nose" },
    { "bur", "mosa" },
    { "cu", "judi" },
    { "ca", "jedi" },
    { "de", "oculo"},
    { "en", "unso" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
    { "ness", "lacri" },
    { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru"   },
    { "re", "candus" },
    { "son", "sabru" },
    { "se",  "or"},
    { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
    {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
    {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
    {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
  };

  strcpy(buf, "");
  strcpy(splwd, spell->name);

  offs = 0;

  while(*(splwd+offs)) {
    for(j=0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
	strcat(buf, syls[j].repl);
	if (strlen(syls[j].org))
	  offs+=strlen(syls[j].org);
	else
	  ++offs;
      }
  }


  sprintf(buf2,"$n utters the words, '%s'", buf);
  sprintf(buf, "$n utters the words, '%s'", spell->name);

  for(temp_char = real_roomp(ch->in_room)->people;
      temp_char;
      temp_char = temp_char->next_in_room)
    if(temp_char != ch) {
      /*
       **  Remove-For-Multi-Class
       */
      if (ch->player.clss == temp_char->player.clss)
	act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
	act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

    }

}

bool saves_spell(struct char_data *ch,
		 sh_int save_type,
		 long immune_type)
{
  // if we don't have any modifier data, just send in 0 -Novak
  return ImpSaveSpell(ch, save_type, 0);
}


bool ImpSaveSpell(struct char_data *ch, sh_int save_type,
		  int mod)
{
  int save;
  int tries = 1;
  
  
  /* Positive mod is better for save */
  /* Negative apply_saving_throw makes saving throw better! */
  
  save = ch->specials.apply_saving_throw[save_type] - mod;
  
  save += BestSaveThrow(ch, save_type);

  if (IS_IMMORTAL(ch)) //Immortals always save against a spell
      return(TRUE);
 
  //----
  // The best you can have is 1, and that gives you a 5% chance of getting hit - Novak
  //---  
  save = MAX(1, save);
  if(save < number(1, 20))
      return TRUE;
  
  return FALSE;
}



char *skip_spaces(char *string)
{
	for(;*string && (*string)==' ';string++);

	return(string);
}


/* show the spells the player learned */

void do_myspells(struct char_data *ch, char *argument, int cmd)
{
    char buf[16384], temp[256];
    struct spell_info* spell;
    int i;
  
    if (!IS_PC(ch))    {
	send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
	return;
    }

    if (!IS_IMMORTAL(ch)) {
	if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
	    send_to_char("Think you had better stick to fighting...\n\r", ch);
	    return;
	} else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
	    send_to_char("Think you should stick to robbing and killing...\n\r", ch);
	    return;
	
	} else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
	    send_to_char("Think you should stick to kicking ass...\n\r", ch);
	    return;
	
	} else if (BestMagicClass(ch) == BARD_LEVEL_IND) {
	    send_to_char("You should stick to singing and dancing...\n\r", ch);
	    return;
	}
    }
  
    *buf=0;

    for (i = 0, spell = spell_list ; i <= spell_count; i++, spell++)
    {
	if (spell->name &&
	    CanLearn(ch, spell) && (ch->skills[spell->number].learned > 0))
	{
	    sprintf(temp, "%-20s  Mana: %3d,%s\n\r",
		    spell->name, USE_MANA(ch, spell), 
		    how_good(ch->skills[spell->number].learned));
	    if((strlen(temp) + strlen(buf) + 1) > sizeof(buf))
	    {
		send_to_char("You just know too many spells...\n\r", ch);
		break;
	    }
	    strcat(buf, temp);
	}
    }
  
    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
}

int CanLearn(struct char_data* ch, struct spell_info* spell)
{
    int i, mask;

    if(IS_IMMORTAL(ch))
	return 1;
    
    for(i = 0, mask = 1 ; i <= MAX_LEVEL_IND ; ++i, mask <<= 1)
	if(HasClass(ch, mask) &&
	   (GET_LEVEL(ch, i) >= spell->min_level[i]))
	    return 1;
    return 0;
}

/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    struct spell_info *spell;
    char	buffer[256];
    char	*ptr;
    int is_switched = 0;
    if (ch->orig != NULL)
      if (IS_IMMORTAL(ch->orig))
        is_switched = 1; 

#if 0
    //I'm commenting this out, because the fact that we have no-flee berserk
    //on now.
    if(IS_AFFECTED(ch, AFF_BERSERK) &&
       (!IS_PURE_CLASS(ch) || !HasClass(ch, CLASS_MAGIC_USER)) &&
       (!IS_PURE_CLASS(ch) || !HasClass(ch, CLASS_PALADIN)) &&
       (!IS_PURE_CLASS(ch) || !HasClass(ch, CLASS_CLERIC)) &&
       IS_FIGHTING(ch))
    {
       send_to_char("You are going berserk, how can you concentrate?\n\r", ch);
       return;
    }
#endif
   
    /* if (IS_NPC(ch) && (!IS_SET(ch->specials.mob_act, ACT_POLYSELF))) */
       if (IS_NPC(ch) && (IS_AFFECTED(ch, AFF_CHARM)) && !is_switched)
    {
	send_to_char("You're not even a monkey!\n\r", ch);
	return;
    }

    if (!IsHumanoid(ch)) {
	send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
	return;
    }
  
    if (!IS_IMMORTAL(ch)) {
	if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
	    send_to_char("Think you had better stick to fighting...\n\r", ch);
	    return;
	} else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
	    send_to_char("Think you should stick to robbing and killing...\n\r", ch);
	    return;
	} else if (BestMagicClass(ch) == BARD_LEVEL_IND) {
	    send_to_char("Think you should stick to rock and roll...\n\r", ch);
	    return;
	}
    }

    if (apply_soundproof(ch))
	return;

    argument = skip_spaces(argument);
  
    /* If there is no chars in argument */
    if (!(*argument)) {
	send_to_char("Cast which what where?\n\r", ch);
	return;
    }
  
    if (*argument++ != '\'') {
	send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
	return;
    }
  
    /* Locate the last quote && lowercase the magic words (if any) */
    for(ptr = buffer ; *argument && *argument != '\'' ; argument++)
	*ptr++ = LOWER(*argument);
    *ptr = 0;
  
    if(*argument++ != '\'')
    {
	send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
	return;
    }
    argument = skip_spaces(argument);
  
    spell = locate_spell(buffer, 0);

    if (!*buffer)
    {
	send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    }
    else if (!ch->skills)
    {
    }
    else if(!spell || !spell->spell_pointer)
    {
	switch (number(1,5)){
	case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
	case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
	case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
	case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
	default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
	}
    }
    else if (GET_POS(ch) < spell->minimum_position)
    {
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING :
	    send_to_char("You dream about great magical powers.\n\r", ch);
	    break;
	case POSITION_RESTING :
	    send_to_char("You can't concentrate enough while resting.\n\r",ch);
	    break;
	case POSITION_SITTING :
	    send_to_char("You can't do this sitting!\n\r", ch);
	    break;
	case POSITION_FIGHTING :
	    send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
	    break;
	default:
	    send_to_char("It seems like you're in pretty bad shape!\n\r",ch);
	    break;
	}
    }
    else if(!IS_IMMORTAL(ch) && (!CanLearn(ch, spell) && !SpellInBook(ch, spell->number)) )
    {
	send_to_char("Sorry, you can't do that.\n\r", ch);
    }
    else if((IS_SET(spell->targets, TAR_VIOLENT) &&
	     check_peaceful(ch, "Impolite magic is banned here.\n\r")) ||
	    !spell_target(ch, spell->targets, argument, &tar_char, &tar_obj))
    {
    }
    else if(IS_SET(spell->targets, TAR_PURE_CLASS) && !IS_PURE_CLASS(ch))
    {
       cprintf(ch, "You are too diversified to cast that spell!\n\r");
    }
    else if (!IS_IMMORTAL(ch) && (GET_MANA(ch) < USE_MANA(ch, spell)))
    {
	send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
    }
    else
    {
      
     if(!SpellInBook(ch, spell->number)) {
      /* check if both hands are used, unless it's on the exceptions list */
      int i=0;
      int sexceptions[] = { SPELL_WORD_OF_RECALL, 0 };
      bool eflag = false;
      
      if ( (ch->equipment[WIELD]) && (ch->equipment[HOLD]) ) {
	while (sexceptions[i] && !eflag) {
	  if (spell->number==sexceptions[i]) {
	    eflag=true;
	  }
	  i++;
	}
	if (!eflag) {
	  send_to_char("Cast without a hand free? HOW?\n", ch);
	  return;
	}
      }
     }       
      say_spell(ch, spell);
	
      if (!IS_IMMORTAL(ch))
      {
	  WAIT_STATE(ch, spell->beats);

          if(!SpellInBook(ch, spell->number)) {
  	    if (number(1,101) > skill_chance(ch, spell->number) &&
	        !IS_IMMORTAL(ch))
	    {			/* 101% is failure */
	      send_to_char("You lost your concentration!\n\r", ch);
	      GET_MANA(ch) -= (USE_MANA(ch, spell)>>1);
	      skill_learn(ch, spell->number);
	      return;
	    }
	  }
      }
    
      
      if(tar_char && (GET_POS(tar_char) == POSITION_DEAD))
	{
	  send_to_char("The magic fizzles against the dead body\n", ch);
	  return;
	}

      send_to_char("Ok.\n\r",ch);

      if(!check_nomagic(ch))
	{
	  /*	  slog((char *) tar_obj);*/
	   (*spell->spell_pointer)(GET_LEVEL(ch, BestMagicClass(ch)), ch,
				  argument, SPELL_TYPE_SPELL, tar_char,
				  tar_obj);
	   
	   //icky hack, but works
	   if(!ch->player.has_killed_victim)
             mprog_spell_trigger(tar_char, ch, spell->number);
	}

      if (SpellInBook(ch, spell->number)) {
          UpdateBook(ch, spell->number);
      }
      else {
          skill_learn(ch, spell->number);
          if(!IS_IMMORTAL(ch)) {
            GET_MANA(ch) -= USE_MANA(ch, spell);
          }
      }
    }
}

bool spell_target(struct char_data* ch,
		  int targets, char* args,
		  struct char_data** tar_char, struct obj_data** tar_obj)
{
    char name[MAX_INPUT_LENGTH];
  
    *tar_char = NULL;
    *tar_obj = NULL;
  
    if(IS_SET(targets, TAR_IGNORE))
	return TRUE;		/* no target is a good target */
  
    args = one_argument(args, name);
	
    if (*name)
    {
	if(IS_SET(targets, TAR_SELF_ONLY))
	{
	    if(get_char_room_vis(ch, name) != ch)
	    {
		send_to_char("This may only be done to yourself.\n\r",ch);
		return FALSE;
	    }
	    *tar_char = ch;
	}
	else if(IS_SET(targets, TAR_CHAR_ROOM) &&
	   (*tar_char = get_char_room_vis(ch, name)))
	{
	    if((*tar_char)->attackers >= 6)
	    {
		send_to_char("Too much fighting, you can't get a clear shot.\n\r", ch);
		return FALSE;
	    }
	}
	else if(IS_SET(targets, TAR_CHAR_WORLD) &&
		(*tar_char = get_char_vis(ch, name)))
	{
	}
	else if(IS_SET(targets, TAR_OBJ_INV) &&
		(*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
	{
	}
	else if(IS_SET(targets, TAR_OBJ_ROOM) &&
		(*tar_obj = get_obj_in_list_vis(ch, name,
						real_roomp(ch->in_room)->contents)))
	{
	}
	else if(IS_SET(targets, TAR_OBJ_WORLD) &&
		(*tar_obj = get_obj_vis(ch, name)))
	{
	}
	else if(IS_SET(targets, TAR_OBJ_EQUIP) &&
		(*tar_obj = get_obj_in_equip_vis(ch, name)))
	{
	}
	else if (IS_SET(targets, TAR_NAME))
	{
	  CREATE(*tar_obj, struct obj_data, 1);
	  (*tar_obj)->name = ss_make(name);
	}
    }
    else if(IS_SET(targets, TAR_SELF_ONLY))
    {
	*tar_char = ch;
    }
    else if(IS_SET(targets, TAR_FIGHT_VICT) && ch->specials.fighting)
    {
	*tar_char = ch->specials.fighting;
    }
    else if (IS_SET(targets, TAR_DEFAULT_SELF))
    {
	*tar_char = ch;
    }
    else
    {
	if (targets < TAR_OBJ_INV)
	    send_to_char("Who should this be done to?\n\r", ch);
	else
	    send_to_char("What should this be done to?\n\r", ch);

	return FALSE;
    }
  
    if (*tar_char)		/* if we're returning a character
				   make sure it's ok to cast magic
				   at them. */
    {
	if(IS_SET(targets, TAR_VIOLENT) && IS_SET(targets, TAR_SELF_NONO) &&
	   !(can_pkill(ch, *tar_char))) {
	    send_to_char("You can't cast offensive spells on this person\n\r", ch);
	    return FALSE;
	}

	if(IS_IMMORTAL(*tar_char) &&
	   IS_SET(targets, TAR_VIOLENT) &&
	   ((TRUST(*tar_char) >= TRUST(ch)) ||
	    !IS_IMMORTAL(ch)))
	{
	    send_to_char("Leave the gods alone!\r\n", ch);
	    return FALSE;
	}

        if (IS_SET((*tar_char)->specials.mob_act, ACT_LIQUID)) {
            send_to_char("You can't do anything to that.\n\r", ch);
            return FALSE;
        }

	if(*tar_char == ch)
	{
	    if(IS_SET(targets, TAR_SELF_NONO))
	    {
		send_to_char("You can not do this to yourself.\n\r", ch);
		return FALSE;
	    }
	}
	if(IS_AFFECTED(ch, AFF_CHARM) &&
	   IS_SET(targets, TAR_VIOLENT) &&
	   (ch->master == *tar_char)) {
	    send_to_char("You are afraid that it could harm your master.\n\r",
			 ch);
	    return FALSE;
	}
    }  
    else if(!*tar_obj)		/* otherwise make sure we're
				   returning something... */
    {
	if (IS_SET(targets, TAR_CHAR_WORLD))
	    send_to_char("Nobody playing by that name.\n\r", ch);
	else if (IS_SET(targets, TAR_CHAR_ROOM))
	    send_to_char("Nobody here by that name.\n\r", ch);
	else if (IS_SET(targets, TAR_OBJ_INV))
	    send_to_char("You are not carrying anything like that.\n\r", ch);
	else if (IS_SET(targets, TAR_OBJ_ROOM))
	    send_to_char("Nothing here by that name.\n\r", ch);
	else if (IS_SET(targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	else if (IS_SET(targets, TAR_OBJ_EQUIP))
	    send_to_char("You are not wearing anything like that.\n\r", ch);
	else if (IS_SET(targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);

	return FALSE;
    }
  
    return TRUE;
}

void do_dispel(struct char_data* ch, char* argument, int cmd)
{
  char		targ[256];
  char		spell[256];
  struct char_data*	tar;
  struct spell_info*	si;
  struct affected_type*	aff;
  int			snum, count, type;
    
  argument = one_argument(argument, targ);
  only_argument(argument, spell);

  snum = -1;
  if(*targ)
  {
    tar = get_char_room_vis(ch, targ);
    if(!tar)
    {
      count = 1;
      tar = get_char_vis_world(ch, targ, &count);
      if(!tar)
      {
	send_to_char("You can't find that person.\n\r", ch);
	return;
      }
    }

    if(*spell)
    {
      si = locate_spell(spell, 0);

      if(!si)
      {
	send_to_char("You don't know of that spell.\n\r", ch);
	return;
      }

      snum = si->number;
    }
  }
  else
    tar = ch;

  type = 0;
  for(aff = tar->affected ; aff ; aff = aff->next)
  {
    if((aff->caster == ch) && (!aff->expire_proc_pointer) &&
       ((snum == -1) || (snum == aff->type)))
    {
      char buf[256];
	    
      aff->caster = 0;
      if(type != aff->type)
      {
	if(ch == tar)
	{
	  sprintf(buf, "You stop paying for your %s\n\r",
		  spell_name(aff->type));
	  send_to_char(buf, ch);
	}
	else
	{
	  sprintf(buf, "You stop paying for $S %s",
		  spell_name(aff->type));
	  act(buf, TRUE, ch, NULL, tar, TO_CHAR);
	}
      }
    }
    type = aff->type;
  }
}

void show_ongoing(struct char_data* ch, struct char_data* vict,
		  int type, int cost);

void do_ongoing_mana(struct char_data* ch, char* argument, int cmd)
{
  int			total, type, mana;
  struct affected_type*	aff;
  struct char_data*	targ;
  char			buf[256];
    
  total = 0;
    
  EACH_CHARACTER(c_iter, targ)
  {
    type = 0;
    mana = 0;
    for(aff = targ->affected ; aff ; aff = aff->next)
    {
      if(aff->caster != ch)
      {
	show_ongoing(ch, targ, type, mana);
	type = mana = 0;
	continue;
      }

      if(aff->type != type)
      {
	show_ongoing(ch, targ, type, mana);
	type = mana = 0;
      }

      type = aff->type;
      mana += aff->mana_cost;
      total += aff->mana_cost;
    }

    show_ongoing(ch, targ, type, mana);
  }
  END_AITER(c_iter);

  sprintf(buf, "%-20s %3d\n\r", "Total:", total);
  send_to_char(buf, ch);
}

void show_ongoing(struct char_data* ch, struct char_data* vict,
		  int type, int cost)
{
  char buf[256];

  if(type && cost)
  {
    sprintf(buf, "%-20s %3d %s\n\r",
	    GET_NAME(vict), cost,
	    spell_name(type));
    send_to_char(buf, ch);
  }
}

int SpellInBook(struct char_data *ch, int spellnum)
{
  struct obj_data *book;
  int i = 0;

  if(ch && (book = ch->equipment[HOLD]))
  {
    if(ITEM_TYPE(book) == ITEM_SPELLBOOK)
    {
      for(i = 0; i < 4; i++)
      {
        if(book->affected[i].location == APPLY_BOOK_SPELL
           && book->affected[i].modifier == spellnum)
            return 1;
      }
    }
  }
  return 0;
}

void UpdateBook(struct char_data *ch, int spellnum)
{
  struct obj_data *book;
  int i;

  if(ch && (book = ch->equipment[HOLD]))
  {
    if(ITEM_TYPE(book) == ITEM_SPELLBOOK)
    {
      for(i = 0; i < 4; i++)
      {
        if(book->affected[i].location == APPLY_BOOK_SPELL
           && book->affected[i].modifier == spellnum)
        {
           if(book->obj_flags.value[i] < 1000)
             book->obj_flags.value[i]-=1;
           if(book->obj_flags.value[i] <= 0)
           {
             send_to_char("The page goes blank.\n\r", ch);
             book->affected[i].location = APPLY_NONE;
             book->affected[i].modifier = -1;
           }
	   break;
        }
      }
    }
  }
}
  

  
