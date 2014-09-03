
#ifndef __TRACK__
#define __TRACK__

typedef struct track_step
{
    room_num   		room_nr;
    int			dir;
} track_step;

typedef struct track_path
{
    int			depth;
    int			flags;
    int			count;
    struct char_data*	victim;
    room_num   		dest;
    track_step*		moves;
} track_path;

#define	HUNT_GLOBAL	1
#define	HUNT_THRU_DOORS	2

/* primary entry points, these are the easiest to use the
   tracking package */

/* build a path to either a named character, or a specific character,
   path_to_name locates a character for whom is_name(name) returns
   true, path_to_full_name locates a character with the exact name
   specified */
track_path*     path_to_obj(room_num in_room, obj_data *obj, int depth, int flags);
track_path*	path_to_name(room_num in_room, char* name,
			     int depth, int flags);
track_path*	path_to_full_name(room_num in_room, char* name,
				  int depth, int flags);
track_path*	path_to_char(room_num in_room, struct char_data* ch,
			     int depth, int flags);
track_path*	path_to_room(room_num in_room, room_num to_room,
			     int depth, int flags);

/* find out which direction should be taken next to get to
   the victim, if -1 is returned we are already in the target room */
int		path_dir(room_num from_room, track_path* path);

/* kill a path and all the data structures associated with it */
void		path_kill(track_path* path);

/* a lower level function to build a path, perhaps based on a more
   complicated predicate then path_to*_char */
typedef int (*build_path_func)(room_num room_nr, void* c_data);
track_path*	path_build(room_num in_room, build_path_func predicate,
			   void* c_data, int depth, int flags);
track_path*	path_rebuild(room_num in_room, track_path* path);

/* do ongoing maintenance of an in-progress track */
int track(struct char_data* ch);

void		do_path(struct char_data* ch, char* arg, int cmd);

/* from skills.c */
int track(struct char_data *ch);

#endif
