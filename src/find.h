/* objects */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj(char *name);
struct char_data *get_char_room(char *name, int room);
struct char_data *get_char(char *name);
struct char_data *get_char_num(int nr);

/* ******* characters ********* */

struct char_data *get_char_room(char *name, int room);
struct char_data *get_char_num(int nr);
struct char_data *get_char(char *name);

/* find if character can see */
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_char_vis_world(struct char_data *ch, char *name,
				     int *count);
struct char_data *get_char_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
				struct obj_data *list);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_vis_world(struct char_data *ch, char *name,
				   int *count);
struct obj_data *get_obj_in_equip_vis(struct char_data* ch, char* name);
struct obj_data* get_obj_vis_accessible(struct char_data* ch, char* name);

struct char_data* find_player_in_world(const char* name);

/* Generic Find */

int generic_find(char *arg, int bitvector, struct char_data *ch,
                   struct char_data **tar_ch, struct obj_data **tar_obj);

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

