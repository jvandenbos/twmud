#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "track.h"
#include "state.h"

#ifdef __cplusplus
extern "C"
{
#endif

  //    int time(int*);

#ifdef __cplusplus
}
#endif

int spell_count = MAX_SKILLS;
list_head descriptor_list;
struct char_data* combat_list = 0;
struct char_data* combat_next_dude = 0;
int pulse = 0;
breath_func bweapons[] =
{
  0
};

// char last_known_state[];
// ** this was already defined in main.c and should
// probably be referenced via the extern in state.h
// -- minwork

int lawful = 0;

void log_msg(const char* mess, int level)
{
    fprintf(stderr, "LOG: %s\n", mess);
}

void slog(const char* mess)
{
    fprintf(stderr, "SLOG: %s\n", mess);
}

void InitABoard(struct obj_data* obj)
{
}

void act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
}

void add_follower(struct char_data* mob, struct char_data* master)
{
    log_msg("add_follower stub called!  You said that would never happen");
    abort();
}

void send_to_char(const char* mesg, struct char_data* ch)
{
    fprintf(stderr, "%s: %s", GET_NAME(ch), mesg);
}

void do_return(struct char_data* ch, const char* arg, int cmd)
{
}

void write_to_q(const char *txt, struct txt_q *queue)
{
}

int CAN_SEE(struct char_data* ch, struct char_data* vict)
{
    return 1;
}


/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_MUD_HOUR) % 24; /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR*now.hours;

    now.day = (secs/SECS_PER_MUD_DAY) % 30; /* 0..29 days  */
    secs -= SECS_PER_MUD_DAY*now.day;

    now.month = (secs/SECS_PER_MUD_MONTH) % 12; /* 0..11 months */
    secs -= SECS_PER_MUD_MONTH*now.month;

    now.year = (secs/SECS_PER_MUD_YEAR); /* 0..XX? years */

    return now;
}



struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0),ch->player.time.birth);

    player_age.year += 17;	/* All players start at 17 */

    return(player_age);
}

void stop_fighting(struct char_data* ch)
{
}

void die_follower(struct char_data* ch)
{
}

void Dismount(struct char_data* ch, struct char_data*h, int pos)
{
    MOUNTED(ch) = 0;
    RIDDEN(h) = 0;
    GET_POS(ch) = pos;
}

struct spell_info* spell_by_number(int spellNo)
{
    return NULL;
}

struct char_data* pop_character(struct char_data* ch)
{
    return NULL;
}

EXP total_exp(int level)
{
    extern EXP exp_table[];

    int		i;
    EXP         exp;

    for(i = 0, exp = 0 ; i < level ; ++i)
	exp += exp_table[i];

    return exp;
}

void add_char_to_hero_list(struct char_data *ch)
{
}

int NewSkillSave(struct char_data* ch, struct char_data* vict,
		 int skillNo, int extraMods, long immunes)
{
  return 0;
}

void do_look(struct char_data* ch, char* arg, int cmd)
{
}

void do_death_trap(struct char_data* v)
{
}

void MakeNoise(int room, char* local_snd, char* distant_snd)
{
}

void stop_follower(struct char_data* ch)
{
}

void path_kill(track_path* path)
{
}

void assign_mobiles(void){}
void assign_rooms(void){}
void assign_objects(void){}
void load_messages(void){}
void build_help_index(void){}
extern int no_specials;
void send_to_room(const char *messg, int room){}
void assign_spell_pointers(void){}
void boot_social_messages(void){}
// extern char last_known_state[MAX_STRING_LENGTH];
void boot_pose_messages(void){}
void nolog(const char *str){}
void assign_command_pointers(void){}
void purge_char(struct char_data* vict){}
