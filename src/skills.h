#ifndef SKILLS_H
#define SKILLS_H

int go_direction(struct char_data* ch, int dir, int tracking);

/* prototypes for skill routines in skills.c */

void do_wings(struct char_data *ch, char *argument, int command);
void do_gills(struct char_data *ch, char *argument, int command);
void do_bash(struct char_data *ch, char *argument, int command);
void do_rescue(struct char_data *ch, char *argument, int command);
void do_berserk(struct char_data *ch, char *argument, int command);
void do_kick(struct char_data *ch, char *argument, int command);
void do_retreat(struct char_data *ch, char *argument, int cmd);

void do_hide(struct char_data *ch, char *argument, int command);
void do_sneak(struct char_data *ch, char *arg, int command);
void do_pick(struct char_data *ch, char *argument, int command);
void do_steal(struct char_data *ch, char *argument, int command);
void do_backstab(struct char_data *ch, char *argument, int command);
void do_search(struct char_data *ch, char *argument, int command);
void do_palm(struct char_data *ch, char *argument, int command);
void do_trip(struct char_data *ch, char *argument, int command);

void do_sunray(struct char_data *ch, char *argument, int command);
void do_windwalk (struct char_data *ch, char *argument, int command);
void do_moonbeam(struct char_data *ch, char *argument, int command);
void do_goodberry (struct char_data *ch, char *argument, int command);
void do_ansum(struct char_data *ch, char *argument, int command);
void do_thorn(struct char_data *ch, char *argument, int command);
void do_creeping_doom(struct char_data *ch, char *argument, int command);
void do_brew (struct char_data *ch, char *argument, int cmd);
void do_tan (struct char_data *ch, char *argument, int cmd);
 
void do_gateway(struct char_data *ch, char *argument, int command);
void do_hypnosis (struct char_data *ch, char *argument, int command);
void do_meditation (struct char_data *ch, char *argument, int command);
void do_scry (struct char_data *ch, char *argument, int command);
void do_adrenalize (struct char_data *ch, char *argument, int command);
void do_invisibility (struct char_data *ch, char *argument, int command);
void do_canibalize (struct char_data *ch, char *argument, int command);
void do_illusionary_shroud(struct char_data *ch, char *argument, int command);
void do_phantasmal_killer (struct char_data *ch, char *argument, int command);
void do_great_sight (struct char_data *ch, char *argument, int command);
void do_spell_shield (struct char_data *ch, char *argument, int command);
void do_psi_attack (struct char_data *ch, char *arg, int cmd);
void do_drain_mana (struct char_data *ch, char *arg, int cmd);
void do_levitate (struct char_data *ch, char *arg, int cmd);
 
void do_hunt (struct char_data *ch, char *argument, int command);
void do_thrust (struct char_data *ch, char *argument, int command);
void do_ration(struct char_data *ch, char *argument, int cmd);
void do_stun(struct char_data *ch, char *argument, int command);
void do_animal_friendship(struct char_data *ch, char *argument,int command);
 
void do_taunt(struct char_data *ch, char *argument, int command);
void do_pinch(struct char_data *ch, char *argument, int command);
void do_fury(struct char_data *ch, char *argument, int command);// 02/04/05 by mtr
void do_heal(struct char_data *ch, char *argument, int cmd);

void do_heroic_rescue(struct char_data *ch, char *arguement, int command);
void create_blessing(struct char_data *ch, char *argument, int cmd);
void do_blessing(struct char_data *ch, char *argument, int cmd);
void do_lay_on_hands (struct char_data *ch, char *argument, int cmd);
void do_holy_warcry (struct char_data *ch, char *argument, int cmd);

void do_form(struct char_data* ch, char* argument, int cmd);
void do_chameleon(struct char_data *ch, char *argument, int cmd);
void do_limb(struct char_data *ch, char *argument, int cmd);
void do_sample(struct char_data *ch, char *argument, int cmd);

void do_appraise(struct char_data *ch, char *argument, int cmd);

void do_sing(struct char_data *ch, char *argument, int command);
void do_blur(struct char_data *ch, char *argument, int cmd);
void do_suggestion(struct char_data *ch, char *argument, int cmd);
void do_terror(struct char_data *ch, char *argument, int cmd);
void do_silence(struct char_data *ch, char *argument, int cmd);
void do_friends(struct char_data *ch, char *argument, int cmd);

void do_flail(struct char_data *ch, char *arg, int cmd);
void do_divert(struct char_data *ch, char *arg, int cmd);
void do_gouge(struct char_data *ch, char *arg, int cmd);
void do_tolerance(struct char_data *ch, char *arg, int cmd);
void do_ego_whip(struct char_data *ch, char *arg, int cmd);
void do_awe(struct char_data *ch, char *arg, int cmd);
void do_shield_punch(struct char_data *ch, char *arg, int cmd);

void do_explode(struct char_data *ch, char *arg, int cmd);
void do_pulse_skill(struct char_data *ch, char *arg, int cmd);
void do_aura(struct char_data *ch, char *arg, int cmd);
void do_set_trap(struct char_data *ch, char *arg, int cmd);
void do_probe(struct char_data *ch, char *arg, int cmd);
#endif
