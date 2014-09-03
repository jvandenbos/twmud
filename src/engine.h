#ifndef ABILENGINE_H
#define ABILENGINE_H

#define NO_CUSTOM_MOBS 0   /* set to 0 when custom mobs are implemented */
#define MAX_SET_SIZE  11   /* max # of 10 spells/skills per ability set */

#include "structs.h"
#include "spelltab.h"

/* prototypes for abilengi.c (ability engine) */

int can_perform (struct char_data *ch, int cmd);
int need_to_stand (struct char_data *ch);
int need_mana_rest (struct char_data *ch);
int need_hp_rest (struct char_data *ch);
int need_move_rest (struct char_data *ch);
void need_to_rest (struct char_data *ch);
int is_spell (struct spell_info *ability);
int enough_mana_move (struct char_data *ch, struct spell_info *ability);
int can_do_ability_in_room (struct char_data *ch, struct spell_info *ability);
struct char_data *get_a_victim (struct char_data *ch, struct spell_info *ability);
void ability_cost (struct char_data *ch, struct spell_info *ability, int pass);
int best_ability (int size, int rank[MAX_SET_SIZE], int storage[MAX_SET_SIZE]);
int pass_concentration (struct char_data *ch, struct spell_info *ability);
int ability_routine (struct char_data *ch, int ability_set[MAX_SET_SIZE]);
int ability_engine (struct char_data *ch, int abilities[][MAX_SET_SIZE]);
int do_abilities (struct char_data *ch, int fight[][MAX_SET_SIZE],
                  int peace[][MAX_SET_SIZE], int cmd);
void init_atts(struct char_data *ch, int mana, int move);
void init_skills(struct char_data *ch, int clss);

/* prototypes for abillogi.c (logic routines) */
 
int logic_null (struct char_data *ch, struct char_data *vict);


/* prototypes for abilprec.c (pre_call routines) */

void pre_null (struct char_data *ch, struct char_data *vict);


#endif
