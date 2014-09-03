#ifndef COMM_H
#define COMM_H

void cprintf(struct char_data *ch, const char *format, ...);
void format_string(const char *input, char *output, int color_status);
void send_to_all(const char *messg);
void send_to_all_regardless(const char *messg);
void send_to_char(const char *messg, struct char_data *ch);
void send_to_except(const char *messg, struct char_data *ch);
void send_to_room(const char *messg, int room);
void send_to_room_except(const char *messg, int room, struct char_data *ch);
void send_to_room_except_two(const char *messg, int room,
			     struct char_data *ch1, struct char_data *ch2);
void send_to_outdoor(const char* messg);
void brag(struct char_data *ch, struct char_data *victim);
void send_to_indoor(const char* messg);
void perform_to_all(const char *messg, struct char_data *ch);
void perform_complex(struct char_data *ch1, struct char_data *ch2,
                     struct obj_data *obj1, struct obj_data *obj2,
                     const char *mess, byte mess_type, bool hide);

void act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type);
void act_to_char(struct char_data* to, const char* str, int hide_invisible,
		 struct char_data* ch, struct obj_data* obj, void* vict_obj);


#define ACT_TO_CHAR		1
#define ACT_TO_VICT		2
#define ACT_TO_ROOM		4
#define ACT_TO_ZONE		8
#define ACT_TO_WORLD		16
#define ACT_TO_IMMORT		32
#define ACT_TO_LOG		64


#define TO_ROOM    		ACT_TO_ROOM
#define TO_VICT    		ACT_TO_VICT
#define TO_NOTVICT 		ACT_TO_ROOM
#define TO_CHAR    		ACT_TO_CHAR
#define TO_LOG			(ACT_TO_IMMORT|ACT_TO_LOG)
#define TO_WORLD		(ACT_TO_WORLD|ACT_TO_ZONE|ACT_TO_VICT)
#define TO_WORLD_NOTVICT	(ACT_TO_WORLD|ACT_TO_ZONE)

int write_to_descriptor(int desc, const char *txt);
void write_to_q(const char *txt, struct txt_q *queue);
int get_from_q(struct txt_q* queue, char* txt, int max);

#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)

int init_socket(int port);
void close_sockets(int s);
void close_socket(struct descriptor_data* d, int show_msg);
void check_idling(struct char_data* ch);
int new_descriptor(int s);
int process_input(struct descriptor_data *t);
int process_output(struct descriptor_data *t);
void player_prompt(struct descriptor_data* point);

extern list_head descriptor_list;
#define EACH_DESCRIPTOR(iter, desc) \
    START_ITER(iter, desc, struct descriptor_data*, &descriptor_list)

extern long Uptime;
extern int pulse;
extern int slow_death;
extern long bytes_read, bytes_written;
extern int slownames;

#ifdef SITELOCK
extern char hostlist[MAX_BAN_HOSTS][30];
extern int numberhosts;
#endif

#if 0
extern char reaplist[MAX_BAN_HOSTS][30];
extern int numberreaps;
#endif

#if 0
struct userlock 
{
  char name[30];
  char user[15];
};
extern struct userlock  userlocklist[MAX_BAN_HOSTS];  /* list of sites to ban*/
extern int numberuserlocks;
#endif


extern int goaway;
extern int lc_reboot;

extern int tics;

extern int connected;
extern int max_connected;

#endif














