
#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "track.h"
#include "utils.h"
#include "list.h"
#include "db.h"
#include "hash.h"
#include "constants.h"
#include "util_str.h"
#include "comm.h"
#include "mobprog2.h"

/* build a path to the nearest character answering to name */
typedef struct
{
    char*		name;
    struct char_data*	victim;
} ncir_data;

static int named_char_in_room(room_num room_nr, void* c_data)
{
    ncir_data*		ncir = (ncir_data*) c_data;
    struct room_data*	roomp;
    struct char_data*	ch;

    if((roomp = real_roomp(room_nr)))
    {
	for(ch = roomp->people ; ch ; ch = ch->next_in_room)
	{
	    if(isname(ncir->name, GET_IDENT(ch)) &&
	       !zprog_track_trigger(ch) &&
	       !IS_SET(ch->specials.mob_act, ACT_NOTRACK))
	    {
		ncir->victim = ch;
		return 1;
	    }
	}
    }

    return 0;
}

track_path* path_to_name(room_num in_room, char* name,
			 int depth, int flags)
{
    ncir_data		ncir = { name, 0 };
    track_path*		path;

    if((path = path_build(in_room, named_char_in_room, &ncir,
			 depth, flags)))
	path->victim = ncir.victim;

    return path;
}

static int obj_in_room(room_num in_room, void *c_data) {
   struct obj_data *obj = (obj_data *) c_data;
   
   if(obj->in_room == in_room) return 1;
   if(obj->carried_by)
     return (obj->carried_by->in_room == in_room);
   if(obj->equipped_by)
     return (obj->equipped_by->in_room == in_room);
   if(obj->in_obj)
     return obj_in_room(in_room, (void *) obj->in_obj);
   
   return 0;
}

track_path* path_to_obj(room_num in_room, struct obj_data *to_obj, int depth, int flags) {
   return path_build(in_room, obj_in_room, to_obj, depth, flags);
}

/* build a path to the nearest character with the exact name */
typedef struct
{
    sstring_t*		name;
    struct char_data*	victim;
} fnir_data;

static int full_name_in_room(room_num room_nr, void* c_data)
{
    fnir_data*		fnir = (fnir_data*) c_data;
    struct room_data*	roomp;
    struct char_data*	ch;
    
    if((roomp = real_roomp(room_nr)))
    {
	for(ch = roomp->people ; ch ; ch = ch->next_in_room)
	{
	    if(ch->player.name == fnir->name)
	    {
		fnir->victim = ch;
		return 1;
	    }
	}
    }

    return 0;
}

track_path* path_to_full_name(room_num in_room, char* name,
			      int depth, int flags)
{
    fnir_data		fnir;
    track_path*		path;
    
    fnir.name = ss_make(name);
    fnir.victim = 0;

    if((path = path_build(in_room, full_name_in_room, &fnir,
			 depth, flags)))
	path->victim = fnir.victim;

    ss_free(fnir.name);
    
    return path;
}


/* build a path to a specific character */
static int char_in_room(room_num room_nr, void* c_data)
{
    struct room_data*	roomp;
    struct char_data*	ch;
    
    if((roomp = real_roomp(room_nr)))
      for(ch = roomp->people ; ch ; ch = ch->next_in_room)
        if(ch == (struct char_data*) c_data)
          if(!IS_SET(ch->specials.mob_act, ACT_NOTRACK))
	    return 1;

    return 0;
}

track_path* path_to_char(room_num in_room, struct char_data* ch,
			 int depth, int flags)
{
    track_path* path;
    
    if((path = path_build(in_room, char_in_room, ch, depth, flags)))
	path->victim = ch;

    return path;
}


/* build a path to a given room */
static int same_room_nr(room_num room_nr, void* c_data)
{
    return room_nr == (int) c_data;
}

track_path* path_to_room(room_num in_room, room_num to_room,
			 int depth, int flags)
{
    return path_build(in_room, same_room_nr, (void*) to_room, depth, flags);
}

/* return the next step to take to get us to the target room */
int path_dir(room_num room_nr, track_path* path)
{
    int		i, retry;
    track_step	*move;

    if(path->victim)
    {
	if(path->victim->in_room == room_nr)
	    return -1;
	
	if(path->victim->in_room != path->dest)
	{			/* make sure victim hasn't moved */
	    for(move = path->moves, i = 0 ; i < path->count ; ++i, ++move)
		if(move->room_nr == path->victim->in_room)
		    break;	/* it's ok, they're still on our path */

	    if((i >= path->count) && !path_rebuild(room_nr, path))
		return -1;
	}
    }    

    for(retry = 1 ; retry > 0 ; retry--)
    {
	for(move = path->moves, i = 0 ; i < path->count ; ++i, ++move)
	    if((move->room_nr == room_nr) &&
	       (real_roomp(room_nr)->dir_option[move->dir]))
		return move->dir;

	/* we aren't on the path any more, build it again if possible */
	if(!path->victim || !path_rebuild(room_nr, path))
	    break;
    }
    
    return -1;
}


/* kill a path and associated */
void path_kill(track_path* path)
{
    if(path)
    {
	if(path->moves)
	    FREE(path->moves);
	FREE(path);
    }
}

/* build a path to the nearest room for which predicate
   returns true */

#define GO_OK(exit)  (!IS_SET(exit->exit_info,EX_CLOSED)\
                 && (exit->to_room != NOWHERE))
#define GO_OK_SMARTER(exit)  (!IS_SET(exit->exit_info,EX_LOCKED)\
                 && (exit->to_room != NOWHERE))

typedef struct
{
  int		room_nr;
  int		dir;
} bp_elem;

typedef struct 
{
  list_element	link;
  int		room_nr;
} room_q;

static void kill_bp_elem(bp_elem* elem)
{
  if(elem != (void*) -1)
    FREE(elem);
}

track_path* path_build(room_num in_room, build_path_func predicate,
		       void* c_data, int depth, int flags)
{
    struct hash_header	x_room;
    list_head		queue;
    int			dir;
    int			tmp_room = -1;
    int			count = 0;
    struct room_data	*herep;
    struct room_data	*therep;
    struct room_data	*startp;
    struct room_direction_data        *exitp;
    room_q*		q_head;
    room_q*		tmp_q;
    bp_elem*		elem;
  
    /* If start = destination we are done */
    if ((predicate)(in_room, c_data))
	return NULL;

    startp = real_roomp(in_room);

    init_hash_table(&x_room, sizeof(int), 2047);
    hash_enter(&x_room, in_room, (void*)-1);

    /* initialize queue */
    list_init(&queue, NULL);
  
    CREATE(q_head, room_q, 1);
    q_head->room_nr = in_room;
    list_append(&queue, &q_head->link);

    while((q_head = (room_q*) list_pop(&queue)))
    {
	herep = real_roomp(q_head->room_nr);
	for(dir = 0 ; dir <= 5 ; ++dir)
	{
	    exitp = herep->dir_option[dir];
	    if (exit_ok(exitp, &therep) &&
		!IS_SET(therep->room_flags, DEATH) &&
		(IS_SET(flags, HUNT_GLOBAL)
		 || (startp->zone == therep->zone)) &&
		(IS_SET(flags, HUNT_THRU_DOORS)
		 ? GO_OK_SMARTER(exitp) : GO_OK(exitp)) &&
		!hash_find(&x_room, tmp_room = exitp->to_room))
	    {
		/* next room */
		if((predicate)(tmp_room, c_data))
		{
		    track_path*		path;
/* added by PAC */
		    /* put room on queue */
		    CREATE(tmp_q, room_q, 1);
		    tmp_q->room_nr = tmp_room;
		    list_append(&queue, &tmp_q->link);

		    /* mark the room as visited */
		    CREATE(elem, bp_elem, 1);
		    elem->room_nr = q_head->room_nr;
		    elem->dir = dir;
		    hash_enter(&x_room, tmp_room, elem);
		    count++;
/* end added section */

		    
		    /* have reached our goal so free queue */
/* commented out by PAC */    /*tmp_room = q_head->room_nr;*/
		    while((q_head = (room_q*) list_pop(&queue)))
			FREE(q_head);

		    /* create the path itself */
		    CREATE(path, track_path, 1);
		    CREATE(path->moves, track_step, count + 1);
		    path->dest = tmp_room;
		    path->flags = flags;
		    path->depth = depth;
		    path->moves[0].dir = dir;
		    path->moves[0].room_nr = tmp_room;
		    path->count = 1;
		    
		    /* walk the hash table chain of directions */
		    while((elem = (bp_elem*) hash_find(&x_room, tmp_room))
			  != (void*) -1)
		    {
			path->moves[path->count].dir = elem->dir;
			path->moves[path->count].room_nr = elem->room_nr;
			path->count++;
			tmp_room = elem->room_nr;
		    }
		    destroy_hash_table(&x_room,
				       (destroy_hash_func) kill_bp_elem);

		    return path;
		}
		else if(++count < depth)
		{
		    /* put room on queue */
		    CREATE(tmp_q, room_q, 1);
		    tmp_q->room_nr = tmp_room;
		    list_append(&queue, &tmp_q->link);

		    /* mark the room as visited */
		    CREATE(elem, bp_elem, 1);
		    elem->room_nr = q_head->room_nr;
		    elem->dir = dir;
		    hash_enter(&x_room, tmp_room, elem);
		}
	    }
	}
 
	FREE(q_head);
    }

    /* couldn't find path */
    destroy_hash_table(&x_room, (destroy_hash_func) kill_bp_elem);

    return NULL;
}

track_path* path_rebuild(room_num from_room, track_path* path)
{
    track_path*	new_path;
    track_step*	temp;
    
    if(!path || !path->victim)
	return NULL;

    new_path = path_to_char(from_room, path->victim, path->depth, path->flags);
    if(!new_path)
	return NULL;
    
    temp = path->moves;
    path->moves = new_path->moves;
    new_path->moves = temp;

    path->count = new_path->count;

    path_kill(new_path);
    
    return path;
}

void do_path(struct char_data* ch, char* arg, int cmd)
{
    char		name[256];
    track_path*		path;
    char		directs[256];
    char*		dir;
    int			next;
    
    only_argument(arg, name);

    if(!name[0])
    {
	send_to_char("find path to who?\n\r", ch);
	return;
    }

    if(name[0] == '.')
    {
	
	if(!(path = path_to_full_name(ch->in_room, name + 1,
				      200, HUNT_GLOBAL | HUNT_THRU_DOORS)))
	{
	    send_to_char("can't find 'em\n\r", ch);
	    return;
	}
    }
    else
    {
	if(!(path = path_to_name(ch->in_room, name,
				 200, HUNT_THRU_DOORS)))
	{
	    send_to_char("can't find 'em\n\r", ch);
	    return;
	}
    }
    
    dir = directs;
    
    for(next = path->count ; next >= 0 ; next--) {
       cprintf(ch, "%d", path->moves[next].dir);
	*dir++ = dirs[path->moves[next].dir][0];
    }
    cprintf(ch, "\n");

    *dir++ = '\n';
    *dir++ = '\r';
    *dir = 0;

    sprintf(name, "shortest route to %s\n\r", GET_NAME(path->victim));
    send_to_char(name, ch);

    send_to_char(directs, ch);
}
