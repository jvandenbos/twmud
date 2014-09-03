#ifndef CMDTAB_H
#define CMDTAB_H

#define CMD_GET		10
#define CMD_SAY		17
#define CMD_DROP	60
#define CMD_REMOVE	66
#define CMD_BOW		98
#define CMD_DEPOSIT	219
#define CMD_WITHDRAW	220
#define CMD_RUB		480
#define CMD_CHECK	481
#define CMD_SEND	482
#define CMD_RECEIVE	483
#define CMD_CHECKIN	484
#define CMD_HOLLER	485
#define CMD_WINGS	486
#define CMD_GILLS	487
#define CMD_TRACE	488
#define CMD_SONGS	489
#define CMD_GUILDS           490
struct command_info
{
  char* name;
  int index;
  byte minimum_position;
  void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
  byte minimum_level;
  longlong min_trust;
};
struct set_struct {
	char*  cmd;
	longlong bit;
  	};    
struct pos_struct {
	  char* name;
	  int posbit;
	  longlong defbit;
	  byte deftrust;
	};      
extern struct pos_struct pos_list[];
extern struct set_struct gcmd_list[];
/* forward declaration of huge table, its at the end of this file,
   but I don't want to have to wade through it to get to interesting
   code */
extern struct command_info cmd_list[];

void assign_command_pointers(void);
struct command_info* lookup_command(char* command, int exact);

#endif
