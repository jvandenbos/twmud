#ifndef FIGHT_H
#define FIGHT_H

int CriticalDamage(struct char_data *ch, struct char_data *v, int dam, int type);
int MissileDamage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype);
void perform_violence(int pulse);
int GetWeaponType(struct char_data*ch, struct obj_data **wielded);
int GetWeaponDam(struct char_data*ch, struct char_data* vic,
		 struct obj_data* wielded);
int PreProcDam(struct char_data* ch, int type, int dam);
int WeaponCheck(struct char_data* ch, struct char_data* v, int type, int dam);
void range_hit(struct char_data *ch, struct char_data *targ, struct obj_data
 	      *missile, int dir, int range);
int check_peaceful(struct char_data* ch, const char* msg);
void hit(struct char_data* ch, struct char_data* victim, int type);
void raw_kill(struct char_data* ch);
int damage(struct char_data* ch, struct char_data* vict,
	   int dam, int attacktype);
void stop_fighting(struct char_data* ch);
void set_fighting(struct char_data* ch, struct char_data* opp);
void shoot(struct char_data* ch, struct char_data* victim);
void update_pos(struct char_data* vict);
void death_cry(struct char_data* ch);
void load_messages(void);
struct char_data* FindAnAttacker(struct char_data* ch);
struct char_data* FindAnyVictim(struct char_data* ch);
struct char_data* FindVictim(struct char_data* ch);
struct char_data* FindMetaVictim(struct char_data* ch);
void RemAllAffects(struct char_data* ch);
void die(struct char_data* ch);
void make_corpse(struct char_data* ch);
int SkipImmortals(struct char_data* v, int amnt);
void stop_opponents(struct char_data *ch, int was_room);
int SetCharFighting(struct char_data *ch, struct char_data *v);
int SetVictFighting(struct char_data *ch, struct char_data *v);
int IsRestricted(int Mask, int Class);
void DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype);
char * show_dam_check(struct char_data *ch, char *buf, int dam);


extern char PeacefulFlag;
extern int combatants;
extern struct char_data* combat_list;
extern struct char_data* combat_next_dude;

#endif

