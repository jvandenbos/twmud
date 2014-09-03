#ifndef MOBPROG2_H
#define MOBPROG2_H

int boot_mprog2(char);
int boot_oprog2(char);
int boot_rprog2(char);
int boot_zprog2(char);
void mprog_weather_trigger2(char_data *, int);
void mprog_greet_trigger2(char_data *);
void mprog_random_trigger2(char_data *);
void mprog_give_trigger2(char_data *, char_data *, obj_data *);
void mprog_speech_trigger2(char *, char_data *);
void mprog_fight_trigger2(char_data *, char_data *);
void mprog_death_trigger2(char_data *, char_data *);
void mprog_kill_trigger2(char_data *, char_data *);
void mprog_entry_trigger2(char_data *);
int  mprog_command_trigger(char *, char *, char_data *);

void oprog_wear_trigger(obj_data *, char_data *);
void oprog_remove_trigger(obj_data *, char_data *);
void oprog_fight_trigger(char_data *, char_data *);
void oprog_get_trigger(obj_data *, char_data *);
void oprog_drop_trigger(obj_data *, char_data *);
void oprog_random_trigger();
int  oprog_command_trigger(char *,char *, char_data *);

void rprog_speech_trigger(char *, char_data *);
int  rprog_command_trigger(char *, char *, char_data *);
void rprog_random_trigger();

int  zprog_death_trigger(char_data *);
int  zprog_arena_trigger(char_data *);
int  zprog_track_trigger(char_data *);

#endif
