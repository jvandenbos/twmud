#ifndef SPELLTAB_H
#define SPELLTAB_H

typedef  void (*spell_proc)(ubyte level, struct char_data *ch, char *arg,
			    int type, struct char_data *tar_ch,
			    struct obj_data *tar_obj);

/* for Brian's ability engine */
typedef int  (*logic_proc)(struct char_data *ch, struct char_data *vict);
typedef void (*pre_call_proc)(struct char_data *ch, struct char_data *vict);

#define MAX_SPELL_COMPONENTS 3

struct spell_info
{
  const char* name;
  ush_int number;
  byte beats;
  byte minimum_position;
  ush_int /*short*/ min_usesmana;
  spell_proc spell_pointer;
  logic_proc logic_pointer;
  pre_call_proc pre_call_pointer;
  int targets;
  ubyte min_level[MAX_LEVEL_IND + 1];
  int modifiers;
  int components[MAX_SPELL_COMPONENTS];
  ubyte learn_rate;
};

extern struct spell_info spell_list[];
extern short spell_count;

void assign_spell_pointers(void);
struct spell_info* locate_spell(char* name, int exact);
struct spell_info* spell_by_number(int);
const char* spell_name(int);

#endif
