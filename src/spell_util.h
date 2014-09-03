#ifndef SPELL_UTIL_H
#define SPELL_UTIL_H

bool saves_spell(struct char_data* ch, sh_int save_type, long immune_type);
void stop_follower(struct char_data* ch);
bool circle_follow(struct char_data* ch, struct char_data* vict);
void add_follower(struct char_data* ch, struct char_data* leader, int shadow);
void die_follower(struct char_data* ch);
bool ImpSaveSpell(struct char_data* ch, sh_int sve_type,
		  int mod);

char* skip_spaces(char* string);

void do_attack_spell(ubyte level, struct char_data* ch,
		     int type, int spell_no,
		     struct char_data* victim, struct obj_data* obj,
		     void (*proc)(ubyte level, struct char_data* ch,
				  struct char_data* victim,
				  struct obj_data* obj));

extern const ubyte saving_throws[MAX_LEVEL_IND+1][5][ABS_MAX_LVL];

bool spell_target(struct char_data *ch, int bits, char *args,
                  struct char_data **tar_char, struct obj_data **tar_obj);
int BestSaveThrow(struct char_data *ch, int saveType);
void stun_opponent(struct char_data *ch, struct char_data *victim, int skill, int bonus);
#define LEARNED(spell) 	(((spell)->targets & TAR_SKILL) ? 90 : 95)

#endif
