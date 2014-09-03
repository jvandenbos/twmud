/* Min declared prototypes */

#define ACMD(name) void (name)(struct char_data *ch, char *arg, int cmd)

struct char_data *get_char_room(char *name, int room);


void give_regens( int pulse ); /* in periodic.c for main.c */
void check_mobile_activity(int pulse); /* in mobact.c for main.c and util.c */

/* act.info.c */
void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch,
			   int mode, int num);
void show_equiped(struct char_data *looker, struct char_data *lookee);
void list_char_in_room(struct char_data *list, struct char_data *ch);

/* act.move.c */
int ValidMove( struct char_data *ch, int cmd);

/* act.obj1.c */
void throw_weapon(struct obj_data *o, int dir, struct char_data *targ,
                  struct char_data *ch, int att_type);

/* act.obj2.c */
int IsRestricted(int Mask, int Class);
int remove_equip(struct char_data* ch, int index, int verbose);

/* act.wizard.c */
void purge_char(struct char_data* vict);
void roll_abilities(struct char_data *ch);
void CreateOneRoom(room_num loc_nr);

/* ban.c */
int isbanned(char *hostname);

/* for comm.c */

void send_to_char_formatted(const char *messg, struct char_data *ch);
void send_to_room_formatted(const char *messg, int room);
void send_to_char(const char *messg, struct char_data *ch);

// modify.c

void page_string(struct descriptor_data *d, const char *str, int keep_internal); // modify.c

// sblock.h


void page_string_block(struct string_block *sb, struct char_data *ch); // sblock.h
  


/* from util_str.c */

char *one_argument(const char *argument, char *first_arg);
int is_abbrev(const char *arg1, const char *arg2);
int is_number(const char *str);
int isname(const char *str, const char *namelist);
int str_cmp(const char *arg1, const char *arg2);
void only_argument(const char *argument, char *dest);

/* from utility.c */

char *str_dup(const char *source);
room_num find_target_room(struct char_data *ch, char *rawroomstr);
int find_all_dots(char *arg);
ush_int PureItemClass(struct obj_data *obj);
int randomnum(int maximum);

/* from mobcmd.c */
void SetHuntingNoRestrict( struct char_data *ch, struct char_data *tch, int dist);
void mob_log(struct char_data *mob, char *msg);
bool str_prefix(const char *astr, const char *bstr);

ACMD(do_mpasound);
ACMD(do_mpkill);
ACMD(do_mpjunk);
ACMD(do_mpechoaround);
ACMD(do_mpsend);
ACMD(do_mpecho);
ACMD(do_mpload);
ACMD(do_mppurge);
ACMD(do_mpgoto);
ACMD(do_mpat);
ACMD(do_mpteleport);
ACMD(do_mpforce);
ACMD(do_mpexp);
ACMD(do_mpstat);
ACMD(mpechoat);
ACMD(mptransfer);
ACMD(mpgold);
ACMD(do_glist);
ACMD(do_gadd);


void mprog_act_trigger( char *buf, struct char_data *mob, struct char_data *ch,
		       struct obj_data *obj, void *vo);
void mprog_bribe_trigger(struct char_data *mob,  struct char_data *ch,
			 int amount);
void mprog_act_trigger( char *buf, struct char_data *mob, struct char_data *ch,
		       struct obj_data *obj, void *vo);
void mprog_death_trigger( struct char_data *mob, struct char_data *killer );
void mprog_entry_trigger( struct char_data *mob );
void mprog_fight_trigger( struct char_data *mob, struct char_data *ch );
void mprog_fight_trigger( struct char_data *mob, struct char_data *ch );
void mprog_give_trigger( struct char_data *mob, struct char_data *ch, 
			 struct obj_data *obj );
void mprog_greet_trigger( struct char_data *ch );
void mprog_hitprcnt_trigger( struct char_data *mob, struct char_data *ch);
void mprog_health_trigger( struct char_data *mob, struct char_data *ch);
void mprog_random_trigger( struct char_data *mob );
void mprog_speech_trigger( char *txt, struct char_data *mob );
void mprog_spell_trigger( struct char_data *mob, struct char_data *ch, int spell_num);
void mprog_weather_trigger( struct char_data *mob, int timeofday);
void mprog_wordlist_check( char *arg, struct char_data *mob,
			   struct char_data *actor, struct obj_data *obj,
			   void *vo, int type );

void boot_mprog();
void bootguilds();
char *mprog_type_to_name(int progtype);


/* db.mobile.c */

struct char_data* read_mobile(int nr);
struct obj_data *read_object(int nr);

/* find.c */

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
				     struct obj_data *list);
/* act.info.c... go figure */
struct obj_data *get_object_in_equip_vis(struct char_data *ch,
					 const char *arg,
					 struct obj_data *equipment[],
					 int *j);

/* utility.c */

char *skip_spaces(char *string);
void gain_exp(struct char_data *ch, int gain);
void gain_exp_heal(struct char_data *ch, int gain);
char fread_letter (FILE *fp );
int fread_number (FILE *fp );
void fread_to_eol ( FILE *fp );
char *fread_word (FILE *fp );

/* fight.c */

void stop_fighting(struct char_data *ch);
void hit(struct char_data *ch, struct char_data *victim, int type);


/* multiclass.c */

int GetMaxLevel(struct char_data *ch);

/* util_num.c */

int number(int from, int to);
int dice(int number, int size);
int get_number(char **name);






void send_to_outdoor_formatted(const char *messg);
void send_to_indoor_formatted(const char *messg);
void send_to_all_formatted(const char *messg);
void send_to_all_regardless_formatted(const char *messg);
void send_to_room_except_formatted(const char *messg, int room, struct char_data *ch);
void send_to_room_except_two_formatted(const char *messg, int room, 
				       struct char_data *ch1, struct char_data *ch2);

/* memory.c */
void check_allocs(void);

/* magic.c */

void spell_identify(ubyte level, struct char_data *ch, int type, 
		    struct char_data *victim, struct obj_data *obj);

/* act.comm.c */

void talk_auction (char *argument);
void talk_vote (char *argument);

/* bet.h */

int parsebet(const int currentbet, const char *argument);

/* act.obj1.c */

void auction_update(void);
void vote_update(void);

void do_look(struct char_data *ch, char *argument, int cmd);

/* skills.c */

bool SkillChance(struct char_data *ch, struct char_data *victim,
		 int basechance, int immunity, int modifiers,
		 int SKILLNUM);




/* xrand.c  random number generator */

void init_mm( );
int number_range( int from, int to );
int number_percent( void );


/* guilds.h/c */

char *guildname(int guildnum);

