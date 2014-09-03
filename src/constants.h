#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifndef FLEDIT
#include "structs.h"
#endif

struct title_type
{
	char *title_m;
	char *title_f;
};

#define NORTH	0
#define	EAST	1
#define	SOUTH	2
#define	WEST	3
#define	UP	4
#define	DOWN	5

extern const struct res_altar_data res_altar_rooms[];
extern const char* spell_wear_off_msg[];
extern const int rev_dir[];
extern const int TrapDir[];
extern const int movement_loss[];
extern const char* dirs[];
extern const char* dir_desc[];
extern const char* dir_from[];
extern const char* ItemDamType[];
extern const char* weekdays[];
extern const char* month_name[];
extern const int sharp[];
extern const char* where[];
extern const char* drinks[];
extern const char* drinknames[];
extern const int RacialMax[][4];
extern const int ItemSaveThrows[26][5];
extern const int drink_aff[][3];
extern const char* color_liquid[];
extern const char* fullness[];
#ifndef FLEDIT
extern const struct title_type titles[MAX_LEVEL_IND+1][ABS_MAX_LVL+1];
#endif
extern const char* RaceName[];
extern const char* item_types[];
extern const char* wear_bits[];
extern const char* extra_bits[];
extern const char* extra_bits_pure[];
extern const char* room_bits[];
extern const char* exit_bits[];
extern const char* sector_types[];
extern const char* equipment_types[];
extern const char* affected_bits[];
extern const char* affected2_bits[];
extern const char* immunity_names[];
extern const char* apply_types[];
extern const char* pc_class_types[];
extern const char* npc_class_types[];
extern const char* action_bits[];
extern const char* player_bits[];
extern const char* position_types[];
extern const char* connected_types[];
#ifndef FLEDIT
extern const int thaco[11][ABS_MAX_LVL];
#endif
extern const struct str_app_type str_app[];
extern const struct dex_skill_type dex_app_skill[];
#ifndef FLEDIT
extern const byte backstab_mult[];
#endif
extern const struct dex_app_type dex_app[];
extern const struct con_app_type con_app[];
extern const struct int_app_type int_app[];
extern const struct wis_app_type wis_app[];
extern const EXP exp_table[];
#ifdef FLEDIT
extern const int bldr_RaceName[];
extern const int bldr_item_types[];
extern const int bldr_wear_bits[];
extern const int bldr_extra_bits[];
extern const int bldr_room_bits[];
extern const int bldr_exit_bits[];
extern const int bldr_sector_types[];
extern const int bldr_obj_affected_bits[];
extern const int bldr_mob_affected_bits[];
extern const int bldr_immunity_names[];
extern const int bldr_apply_types[];
extern const int bldr_action_bits[];
extern const int bldr_position_types[];
extern const int bldr_drinks[];
extern const char *sex_types[];
extern const int bldr_sex_types[];
extern const char *zone_resets[];
extern const int bldr_zone_resets[];
extern const char *zn_door_states[];
extern const int bldr_zn_door_states[];
extern const char *tel_types[];
extern const int bldr_tel_types[];
extern const char *contain_bits[];
extern const int bldr_contain_bits[];
#endif
#endif

