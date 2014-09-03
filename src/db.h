#ifndef DB_H
#define DB_H

#include "structs.h"
#include "array.h"

/* data files used by the game system */

#define WORLD_FILE        "tinyworld.wld" /* room definitions           */
#define MOB_FILE          "tinyworld.mob" /* monster prototypes         */
#define OBJ_FILE          "tinyworld.obj" /* object prototypes          */
#define ZONE_FILE         "tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define POLICY_FILE	  "policy"	  /* for the 'policy' command   */
#define MOTD_FILE         "motd"          /* messages of today          */
#define IMOTD_FILE        "imotd"         /* immortal messages of today */
#define TWSTORY_FILE      "twstory"      /* tw roleplay story   */ 
#define PLAYER_DIR	  "players.d"	  /* directory of saved players */
#define GOD_DIR           "gods.d"        /* directory of immortals */
#define PLAYER_FILE       "players"       /* the player database        */
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'             */
#define BUG_FILE          "bugs"          /*         'bug'              */
#define WIZ_FILE	  "wizreport"     /* 	     'wizreport'        */
#define MESS_FILE         "messages"      /* damage message             */
#define SOCMESS_FILE      "actions"       /* messgs for social acts     */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>          */

#ifdef NEWHELP
#define SKILL_HELP_FILE	  "skill_help"	  /* for skill help		*/
#define GENERAL_HELP_FILE "general_help"  /* for general help		*/
#define IMM_HELP_FILE     "imm_help" 	  /* for immortal help		*/
#define COMBAT_HELP_FILE  "combat_help"	  /* for combat help		*/
#endif

#define HELP_PAGE_FILE    "help"          /* for HELP <CR>              */
#define INFO_FILE         "info"          /* for INFO                   */
#define STORY_FILE        "story"         /* for STORY                  */
#define POSEMESS_FILE     "poses"         /* for 'pose'-command         */
#define BAN_FILE          "ban.list"      /* to save wizlock list       */
#define REAP_FILE          "reap.list"     /* to save reaplock list      */
#define USER_LOCK_FILE    "userlock.list"  /* to save userlock list      */
/* This is a base filename for greetings, a sequential number will be added
to the end of this text to come up with a filename to be read in */
#define GREET_BASE	  "greeting/"	  /* banner base filename	*/

#define WORLD_SIZE 50000

/* Age of zones that do not require updating --
 *    swapped, queued, or never reset */
#define ZO_DEAD  999

/* info about saving and restoring players */
#define READ_PLAYER	1
#define READ_OBJECTS	2
#define READ_ALIASES	4
#define READ_DO_COUNT	128
#define	READ_ALL       	(READ_PLAYER | READ_OBJECTS | READ_ALIASES)

#define READ_FIRST	(READ_PLAYER)
#define READ_SECOND	(READ_OBJECTS | READ_ALIASES)
#define PLAYER_MAGIC	0x1badc0de

/* public procedures in db.world.c */
int zone_can_swap(int zone);
void swap_zone(int zone);

/* public procedures in db.c */

void boot_db(void);
void boot_objects(const char* file);
void boot_mobiles(const char* file);
void boot_players(const char* dir, int flags);
void boot_world(const char* file);
void boot_zones(const char* file);
int create_entry(char *name);
void zone_update(void);
void renum_zone_table(void);
void clear_char(struct char_data *ch);
void clear_aliases(struct char_data *ch);
void clear_object(struct obj_data *obj);
void reset_char(struct char_data *ch);
void free_char(struct char_data *ch);
char *fread_string(FILE *fl);
void completely_cleanout_room(struct room_data* rp);
void cleanout_room(struct room_data* rp);
void set_title(struct char_data* ch);
void reset_zone(int zone, int init);
void free_obj(struct obj_data* obj);
int load_one_room(FILE* fl, struct room_data* p);
struct index_data* generate_indices(FILE* fl, int* top);
int UnderLimit(int rnum, int most, int init, int level, int type);
void extract_obj(struct obj_data *obj);
struct obj_data *get_obj_num(int nr);
void load_next_greet(void);
  
#define REAL    1
#define VIRTUAL 2
#define NORAND  4
#define RAND	8

class Function;

struct obj_data* create_object(void);
struct obj_data *create_money( int amount );
struct obj_data* make_object(int nr, int type, int rnum=-2);
struct obj_data* clone_object(struct obj_data* obj, int rnum=-2);

void obj_to_room(struct obj_data *object, room_num room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);
void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, room_num room);
void AffRoomContLight(struct char_data *ch, struct room_data *rp);
void AffRoomContDark(struct char_data *ch, struct room_data *rp);

void raw_equip(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);

void advance_level(struct char_data* ch, int clss);

struct char_data* make_mobile(int nr, int type);

int write_player(struct char_data* ch, FILE* fp, FILE* eqfp);
int read_player(struct char_data* ch, FILE* fp, int parts);

int putcorpse(struct obj_data *obj, room_num room, FILE* fp);
int getcorpse(FILE* fp);
int savecorpse(struct obj_data *obj, room_num room);
int loadcorpse();

/* structure for the reset commands */
struct reset_com
{
	char command;   /* current command                      */ 
	byte if_flag;   /* if TRUE: exe only if preceding exe'd */
        byte last_cmd;	/* if this command executed succesfully */
	int arg1;       /*                                      */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */

	/* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*/
};



/* zone definition structure. for the 'zone-table'   */
#define ZO_CANT_SWAP 4 /* reset_mode bit */
struct zone_data
{
	char *name;             /* name of this zone                  */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	room_num top;                /* upper limit for rooms in this zone */
	int swapped;		/* 1 is swapped, 0 is not swapped     */
	int can_swap;

	int reset_mode;         /* conditions for reset (see below)   */
	long reset_cycle;     /* time to reset_cycle (zero loaded in
				                         obj_index    */
	struct reset_com *cmd;  /* command table for reset	           */
        Function *zoneprogs2;   /* zone mobprog2s */
        Variable *vars;         /* variables for zone */
        Variable *global_vars;  /* global variables for zone */

	/*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
	*/
};




/* element in monster and object index-tables   */
struct index_data {
  int virt;		/* virtual number of this mob/obj      */
  long pos;		/* file position of this field              */
  int number;		/* number of existing units of this mob/obj */
  int limit;		/* maximum to load */
  spec_proc_func func;	/* special procedure for this mob/obj       */
  char *name;
  void* proto;
  int loaded;

  /* for mobprogs */
  
  int progtypes;        /* program types for MOBProg                     */
  MPROG_DATA *mobprogs; /* programs for MOBProg                          */
   
  /* for mobprogs2 */
   
  Function *mobprogs2;  /* programs for MOBProg2 */
  Function *objprogs2;
   
  Variable *global_vars;
};




struct player_index_element
{
	char *name;
	int nr;
};


struct help_index_element
{
	char *keyword;
	long pos;
};

void update_time(void);
int parse_dice(const char* file, int ident,
	       const char** ptr,
	       long* cnt, long* size, long* mod);
int parse_number(const char* file, const char* field, int ident,
		 const char**ptr, long* num);
int parse_unumber(const char* file, const char* field, int ident,
		 const char** ptr, unsigned long* num);
void log_expect(const char* file, int ident,
		const char* expect, const char* found);
void log_syntax(const char* file, int ident,
		const char* message, const char* found);

#include <sys/dir.h>

typedef struct
{
    char	subdir;
    DIR*	dirfd;
    char*	dirName;
} PlayerDir;

PlayerDir*		OpenPlayers(const char* dirName);
struct char_data*	ReadPlayer(PlayerDir* desc, int flags);
void			ClosePlayers(PlayerDir* desc);
void SpaceForSkills(struct char_data* ch);
int DetermineExp(struct char_data* mob, int exp_flags);
FILE* MakeZoneFile(struct char_data* ch, char *zonename);
int MobVnum(struct char_data* ch);
void Zwrite(FILE* fp, char chmd, int tf, int a1, int a2, int a3,
	    const char* desc);
void RecZwriteObj(FILE* fp, struct obj_data* o);
void RoomLoad(struct char_data* ch, room_num start, room_num end, const char* fname);
void RoomSave(struct char_data* ch, room_num start, room_num end, const char* fname);

int real_object(int virt);
int real_mobile(int virt);
void init_world(void);


struct room_data *real_roomp(room_num virt);
int room_enter(room_num key, struct room_data *rp);
struct room_data *room_find_or_create(room_num key);
int room_remove(room_num key);
typedef void (*room_iterate_func)(room_num index, struct room_data* rd,
				  void* c_data);
void room_iterate(room_iterate_func func, void *cdata);
int same_zone(struct char_data* ch, struct char_data* vict);
room_num room_of_object(struct obj_data* obj);

int getFreeAffSlot(struct obj_data* obj);
EXP balance_exp(struct char_data *mob);
void random_mob_points(struct char_data *mob);
struct mob_variable *mvar_find(struct char_data *ch, char *name);
struct mob_variable *mvar_make(struct char_data *ch, char *name, char *val);
void mob_kill(struct char_data *ch, struct mob_variable *mv);
void mob_killall(struct char_data *ch, struct mob_variable *mv);
double MVarEval(struct char_data *ch, char *lhs, char opr, char *rhs);
double MVarMath(struct char_data *ch, char *expr);


/* global variables */
extern char policy[MAX_STRING_LENGTH];
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH*4];
extern char motd[MAX_STRING_LENGTH];
extern char imotd[MAX_STRING_LENGTH];
extern char help[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char story[MAX_STRING_LENGTH*5];
extern char wizlist[MAX_STRING_LENGTH * 2];
extern char greeting[MAX_STRING_LENGTH * 8];
extern char twstory[MAX_STRING_LENGTH*8];
extern int greet_no;
extern char newwizlist[MAX_STRING_LENGTH*2];

extern struct time_info_data time_info;
extern struct weather_data weather_info;

#ifdef NEWHELP
extern struct help_index_element* help_index;
extern FILE *genhelp_fl, *skillhelp_fl, *immhelp_fl, *combhelp_fl;
extern int top_of_genhelp, top_of_skillhelp, top_of_immhelp, top_of_combhelp;
#else
extern struct help_index_element *help_index;
extern FILE* help_fl;
extern int top_of_helpt;
#endif

extern array_t character_list;
#define EACH_CHARACTER(iter, ch) \
    AITER_B(iter, ch, struct char_data*, &character_list)

extern struct index_data* mob_index;
extern int top_of_mobt;
extern long mob_count;
extern int top_of_p_table;
extern int top_of_p_file;

extern array_t object_list;
#define EACH_OBJECT(iter, obj) \
    AITER_B(iter, obj, struct obj_data*, &object_list)

extern struct index_data* obj_index;
extern int top_of_objt;
extern long obj_count;

#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db[];
#endif
extern room_num top_of_world;
extern long room_count;

#ifdef SWAP_ZONES
extern int swapped_zones;
extern int loaded_zones;
#endif
extern int top_of_zone_table;
extern struct zone_data* zone_table;

extern int player_count;
extern int no_specials;

extern struct message_list fight_messages[];

/* if true, object counting should be done by extract_obj and
   make_object */
extern int count_objects;

struct time_info_data age(struct char_data *ch);
char *ClassTitles(struct char_data *ch, char* buf);
struct char_data* real_character(struct char_data* ch);
struct char_data *charm_master(struct char_data *ch);
bool mob_wait_state(struct char_data *ch, int cycle);

typedef struct 
{
    event_t	event;
    struct obj_data* object;
} object_event;

/* object mapping stuff */
typedef struct
{
  struct obj_data ** objects;
  int count;
  int max;
} object_map;

object_map* new_object_map(int max_object_save);
void kill_object_map(object_map* map);

int getobject(struct obj_data** objP, struct obj_data* cont,
	            object_map* map, struct char_data* ch, FILE* fp, int fv, 
	            int room);
int putobject(struct obj_data* obj, int pos, int own,
	            object_map* map, FILE* fp, FILE *eqfp);

int putdword(long word, FILE* fp);
int getdword(long* word, FILE* fp);
int putsstring(const sstring_t* string, FILE* fp);
int getsstring(sstring_t** string, FILE* fp);

#endif

/* added by Jan(Min) 1996 for Ban code */



/* don't change these */
#define BAN_NOT 	0
#define BAN_NEW 	1
#define BAN_SELECT	2
#define BAN_ALL		3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char	site[BANNED_SITE_LENGTH+1];
   int	type;
   time_t date;
   char	name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};
typedef struct index_data MOB_INDEX_DATA;
typedef struct index_data OBJ_INDEX_DATA;


struct special_affect_type {
    char* name;
    long minlevel;
    int affnum;
    long affminval;
    long affmaxval;
    long type_flag;
};

extern void lcstr(char *txt);
extern const struct special_affect_type specialaffects[];

