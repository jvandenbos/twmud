#ifndef UTILITY_H
#define UTILITY_H


/*
int IS_DARK(int room);
int IS_LIGHT(int room);
*/

struct descriptor_data * getdescriptor(struct char_data *ch);
void file_log(const char* str, const char* filename);
void log_msg(const char *str, int level=LOG_ERROR);
void slog(const char* str);
void dlog(const char* str, ...);
void nolog(const char *str, int level=LOG_ERROR);
void RiverPulseStuff(int pulse);
void TeleportPulseStuff(int pulse);
void WriteToImmort(const char* mesg, int invis_level, int level=LOG_ERROR);
int CheckColor(struct char_data* s);
int number(int from, int to);
int dice(int number, int size);
int apply_soundproof(struct char_data* ch);
int check_soundproof(struct char_data* ch);
int check_nomagic(struct char_data* ch);
struct time_info_data age(struct char_data* ch);
int MobCountInRoom(struct char_data* list);
int RideCheck(struct char_data* ch, int mod);
int MountEgoCheck(struct char_data* ch, struct char_data* horse);
void FallOffMount(struct char_data* ch, struct char_data* vict);
int CountFollowers(struct char_data* ch);
int get_average_level(struct char_data* ch);
int IsLevelOk(struct char_data* ch, struct obj_data* obj);
#ifndef MIN
int MIN(int a, int b);
#endif
#ifndef MAX
int MAX(int a, int b);
#endif
int GetItemClassRestrictions(struct obj_data* obj);
int IsPerson(struct char_data* ch);
int IsHumanoid(struct char_data* ch);
int HasHands(struct char_data* ch);
int IsImmune(struct char_data* ch, int bit);
void SetHunting(struct char_data* ch, struct char_data *tch);
int IsRideable(struct char_data* ch);
int IsAnimal(struct char_data* ch);
int IsUndead(struct char_data* ch);
int IsUnsamplable(struct char_data* ch);
int IsExtraPlanar(struct char_data* ch);
int ObjVnum(struct obj_data* o);
void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max);
int CheckMeditating(struct char_data* s);
int in_group(struct char_data* ch, struct char_data* ch2);
int is_following(struct char_data* ch, struct char_data* ch2);
int can_hurt( struct char_data *attacker, struct char_data *victim);
int IsImmune(struct char_data* ch, int bit);
int IsResist(struct char_data* ch, int bit);
int IsSusc(struct char_data* ch, int bit);
void DevelopHatred(struct char_data* ch, struct char_data* vict);
void GrowPlants(int nothing);
void do_area_attack(int level, struct char_data* ch,
		    void (*proc)(ubyte level, struct char_data* ch, int type,
				 struct char_data* victim,
				 struct obj_data* obj), int type);
void do_area_spell(int level, struct char_data* ch,
		   void (*proc)(ubyte level, struct char_data* ch, int type,
				struct char_data* victim,
				struct obj_data* obj), int type);
int NoSummon(struct char_data* ch);
void RestoreChar(struct char_data* ch);
void RemAllAffects(struct char_data* ch);
int HasObject(struct char_data* ch, int ob_num);
void CallForGuard(struct char_data* ch, struct char_data* vict,
		  int lev, int area);
void StandUp(struct char_data* ch);
int CheckForBlockedMove(struct char_data* ch, int cmd,
			char* arg, room_num room, int dir, int clss);
struct char_data* char_holding(struct obj_data* obj);
void FighterMove(struct char_data* ch);
int CountFollowers(struct char_data* ch);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
int can_wear_test(struct char_data *ch, struct obj_data *obj);
void ObjFromCorpse(struct obj_data* c);
void gain_exp_regardless(struct char_data* ch, int gain, int clss);
void gain_exp(struct char_data* ch, int gain);
void drop_level(struct char_data* ch, int clss);

void object_list_new_owner(struct obj_data *list, struct char_data *ch);

void push_character(struct char_data* ch, struct char_data* new_ch);
struct char_data* pop_character(struct char_data* ch);
struct char_data* curr_character(struct char_data* ch);
void StuffToRoom(struct char_data* ch, room_num in_room);

void set_descriptor(struct char_data* ch, struct descriptor_data* desc);
int check_convict(struct char_data* ch, struct char_data* keeper);
void launch_hunter(int level);

struct time_info_data real_time_passed(time_t t2, time_t t1);

EXP total_exp(int level);

int travel_check(struct char_data* ch, struct char_data* vict);
int track_to_room(struct char_data* ch, room_num room_nr,
		  int depth, int flags);
int track_to_char(struct char_data* ch, struct char_data* vict,
		  int depth, int flags);

int GetRealLevel(struct char_data* ch);
/* Modifiers to the base rolls for choosing a certain race */
struct player_race_max_atts
{
  sbyte str_max;
  sbyte dex_max;
  sbyte con_max;
  sbyte int_max;
  sbyte wis_max;
  sbyte cha_max;
};

struct player_race_max_atts *Find_Att_by_Race(struct char_data *ch);
void assign_max_race_attributes();
int Is_Offensive_Spell(int sn);
int Is_AreaFX_Spell(int sn);
int Is_Heal_Spell(int sn);

int percent(void); /* by JanV for percentile rolls */

void skill_learn(struct char_data *ch, int skill);
byte skill_chance (struct char_data *ch, int skill);
int ApplyImmuneToDamage(struct char_data *victim, int cur_damage, int immuneBit);

/* from magic.c */
int wilderness(struct char_data *ch);

//from spells.h
int GetPolyAge(const struct PolyType* list, int vnum);
#endif

