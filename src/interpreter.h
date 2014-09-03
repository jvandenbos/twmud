#ifndef INTERPRETER_H
#define INTERPRETER_H

void command_interpreter(struct char_data *ch, char *argument, int expand);
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
void nanny(struct descriptor_data *d, const char *arg);
int find_plyr_race(struct char_data *ch);
void nanny_race_stats(struct char_data *ch, int unmapped_race);
int can_reroll_parents(struct char_data *ch);
void nanny_career_stats(struct char_data *ch);
int can_reroll_age(struct char_data *ch);
void nanny_train_stats(struct char_data *ch, char option);
int _parse_name(const char* arg, char* name);
int special(struct char_data *ch, int cmd, char *arg);

char *two_arguments(char *argument, char *first_arg, char *second_arg);



#define MENU         \
"\n\rWelcome to Thieves World.\n\r" \
"0] Exit from Thieves World.\n\r" \
"1] Enter the game at Sanctuary.\n\r" \
"2] Enter description.\n\r" \
"3] Read the background story.\n\r" \
"4] Change password.\n\r" \
"5] Delete this character.\n\r" \
"6] Enter the game at another location.\n\r\n\r" \
"   Pick one: "


#define RACEHELP          \
"\n\rRaces:  (Dwarven, Elven, Human, Hobbit, Gnome, Felis, Canis)\n\r" \
"--------------------------------------------------------------------------\n\r" \
"Dwarves:  Shorter, less movement, infravision, more HP gain per tick.\n\r" \
"Elves:    Taller, more movement, numerous racial hatreds, good at track.\n\r" \
"Humans:   Average...  Fewer racial hatreds than other races.\n\r" \
"Hobbits:  Shortest, least movement, more dexterity, more wisdom, weaker.\n\r" \
"Gnomes:   Shorter, less movement, more intelligence and wisdom.\n\r" \
"Felis:    Shorter, cat-like with highest dex, best trackers.\n\r" \
"Canis:    Shorter, dog-like with high wisdom, best trackers.\n\r\n\r"

#define WELC_MESSG \
"\n\rEnjoy The Thieves World. Have fun... but watch your back!\n\r\n\r"

extern int WizLock;
extern int PKILLABLE;
extern int plr_tick_count;
extern int MAX_EXP_REIMB;
extern int CHAOS;
extern int LOAD_PLAYER_FILES;
extern int CASINO_BANK;
extern int PAWN_SHOP;
extern int PIRATE;
extern int PIRATENUM;
extern int PIRATEQST;
extern int DISASTER;
extern int DISASTERNUM;
extern int FOUNTAIN_LEVEL;
extern int FOUNTAIN_GOLD;
extern int GROUP_RES;
extern int AREA_FX_DEADLY;
extern int AUTH_PLAYER;
extern char last_known_state[];

#endif

#ifndef INTERPRETER_H
#define INTERPRETER_H

void command_interpreter(struct char_data *ch, char *argument, int expand);
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
void nanny(struct descriptor_data *d, const char *arg);
int find_plyr_race(struct char_data *ch);
void nanny_race_stats(struct char_data *ch, int unmapped_race);
int can_reroll_parents(struct char_data *ch);
void nanny_career_stats(struct char_data *ch);
int can_reroll_age(struct char_data *ch);
void nanny_train_stats(struct char_data *ch, char option);
int _parse_name(const char* arg, char* name);
int special(struct char_data *ch, int cmd, char *arg);

char *two_arguments(char *argument, char *first_arg, char *second_arg);



#define MENU         \
"\n\rWelcome to Thieves World.\n\r" \
"0] Exit from Thieves World.\n\r" \
"1] Enter the game at Sanctuary.\n\r" \
"2] Enter description.\n\r" \
"3] Read the background story.\n\r" \
"4] Change password.\n\r" \
"5] Delete this character.\n\r" \
"6] Enter the game at another location.\n\r\n\r" \
"   Pick one: "


#define RACEHELP          \
"\n\rRaces:  (Dwarven, Elven, Human, Hobbit, Gnome, Felis, Canis)\n\r" \
"--------------------------------------------------------------------------\n\r" \
"Dwarves:  Shorter, less movement, infravision, more HP gain per tick.\n\r" \
"Elves:    Taller, more movement, numerous racial hatreds, good at track.\n\r" \
"Humans:   Average...  Fewer racial hatreds than other races.\n\r" \
"Hobbits:  Shortest, least movement, more dexterity, more wisdom, weaker.\n\r" \
"Gnomes:   Shorter, less movement, more intelligence and wisdom.\n\r" \
"Felis:    Shorter, cat-like with highest dex, best trackers.\n\r" \
"Canis:    Shorter, dog-like with high wisdom, best trackers.\n\r\n\r"

#define WELC_MESSG \
"\n\rEnjoy The Thieves World. Have fun... but watch your back!\n\r\n\r"

extern int WizLock;
extern int PKILLABLE;
extern int plr_tick_count;
extern int MAX_EXP_REIMB;
extern int CHAOS;
extern int LOAD_PLAYER_FILES;
extern int CASINO_BANK;
extern int PAWN_SHOP;
extern int PIRATE;
extern int PIRATENUM;
extern int PIRATEQST;
extern int DISASTER;
extern int DISASTERNUM;
extern int FOUNTAIN_LEVEL;
extern int FOUNTAIN_GOLD;
extern int GROUP_RES;
extern int AREA_FX_DEADLY;
extern int AUTH_PLAYER;
extern char last_known_state[];

#endif

