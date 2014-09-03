
#include "config.h"

#include <stdio.h>
#include <string.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "hash.h"
#include "handler.h"
#include "find.h"
#include "utility.h"
#include "opinion.h"
#include "spell_util.h"
#include "act.h"
#include "proto.h"
#include "statistic.h"

room_num top_of_world = 0;                 /* ref to the top element of world */
#if HASH
struct hash_header	room_db;
#else
struct room_data        *room_db[WORLD_SIZE];
#endif

list_head reset_q;

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
long total_bc = 0;
long room_count=0;

int setup_dir(FILE *fl, int room, int dir);
int is_empty(int zone_nr);
void swap_zone(int zone);

/* for queueing zones for update   */
typedef struct
{
  event_t		event;
  int			zone_to_reset; /* ref to zone_data */
} reset_q_element;
int zone_update_proc(event_t* element, int now);

void cleanout_room(struct room_data *rp)
{
    int	i;
    struct extra_descr_data *exptr, *nptr;
    
    FREE(rp->name);
    FREE(rp->description);
    for (i=0; i<6; i++)
	if (rp->dir_option[i]) {
	    if(rp->dir_option[i]->general_description)
		FREE(rp->dir_option[i]->general_description);
	    if(rp->dir_option[i]->keyword)
		FREE(rp->dir_option[i]->keyword);
	    FREE(rp->dir_option[i]);
	    rp->dir_option[i] = NULL;
	}
    
    for (exptr=rp->ex_description; exptr; exptr = nptr) {
	nptr = exptr->next;
	FREE(exptr->keyword);
	FREE(exptr->description);
	FREE(exptr);
    }

    /* no dangling events */
    event_cancel(rp->tele_event, 1);
    rp->tele_event = 0;
    event_cancel(rp->river_event, 1);
    rp->river_event = 0;
}

void completely_cleanout_room(struct room_data *rp)
{
    struct char_data	*ch;
    struct obj_data	*obj;
    
    while (rp->people) {
	ch = rp->people;
	act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
	char_from_room(ch);
	log_msg("completely_cleanout_room: Sending character to the void");
	char_to_room(ch, 0);	/* send character to the void */
    }
    
    while (rp->contents) {
	obj = rp->contents;
	obj_from_room(obj);
	obj_to_room(obj, 0);	/* send item to the void */
    }
    
    cleanout_room(rp);
}

#define GETN(fld) \
{ \
    long tmp; \
	  if(!parse_number("room file", #fld, \
			   rp->number, (const char**) &ptr, &tmp)) \
			       return 0; \
				   fld = tmp; \
				   }

int load_one_room(FILE *fl, struct room_data *rp)
{
    char chk[50];
    int dir;
    int   bc=0;
    int	tmp;
    char buf[256];
    char* ptr;
    
    struct extra_descr_data *new_descr;
    
    bc = sizeof(struct room_data);
    
    rp->name = fread_string(fl);
    if (rp->name && *rp->name)
	bc += strlen(rp->name);
    rp->description = fread_string(fl);
    if (rp->description && *rp->description)
	bc += strlen(rp->description);
    
    ptr = fgets(buf, sizeof(buf), fl);
    
    if (top_of_zone_table >= 0)
    {
	int	zone;
	
	GETN(tmp);
	
	/* OBS: Assumes ordering of input rooms */
	
	for (zone=0;
	     rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	     zone++)
	    ;
	if (zone > top_of_zone_table) {
	    fprintf(stderr, "Room %ld is outside of any zone.\n", rp->number);
	    return 0;
	}
	rp->zone = zone;
    }
    
    GETN(rp->room_flags);
    GETN(rp->sector_type);
    
    if (rp->sector_type == -1)
    {
	GETN(rp->tele_time);
	GETN(rp->tele_targ);
	GETN(rp->tele_mask);
	if (IS_SET(TELE_COUNT, rp->tele_mask))
	{
	    GETN(rp->tele_cnt);
	}
	else
	    rp->tele_cnt = 0;
	GETN(rp->sector_type);
    }
    else
    {
	rp->tele_time = 0;
	rp->tele_targ = 0;
	rp->tele_mask = 0;
	rp->tele_cnt  = 0;
    }
    
    if (rp->sector_type == SECT_WATER_NOSWIM ||
	rp->sector_type == SECT_UNDERWATER)  { /* river */
	/* read direction and rate of flow */
	GETN(rp->river_speed);
	GETN(rp->river_dir);
    } 
    
    if (rp->room_flags & TUNNEL) { /* read in mobile limit on tunnel */
	GETN(rp->moblim);
    }  

    rp->river_event = 0;
    rp->tele_event = 0;
    
    rp->funct = 0;
    rp->light = 0;		/* Zero light sources */
   
    rp->roomprogs2 = NULL;
    
    for (tmp = 0; tmp <= 5; tmp++)
	rp->dir_option[tmp] = 0;
    
    rp->ex_description = 0;
    
    while((ptr = fgets(buf, sizeof(buf), fl)))
    {
	switch (*ptr++)
	{
	case 'D':
	    GETN(dir);
	    setup_dir(fl, rp->number, dir);
	    bc += sizeof(struct room_direction_data);
	    break;
	    
	case 'E':		/* extra description field */
	    CREATE(new_descr,struct extra_descr_data,1);
	    bc += sizeof(struct extra_descr_data);
	    
	    new_descr->keyword = fread_string(fl);
	    if (new_descr->keyword && *new_descr->keyword)
		bc += strlen(new_descr->keyword);
	    else
		fprintf(stderr, "No keyword in room %ld\n", rp->number);
	    
	    new_descr->description = fread_string(fl);
	    if (new_descr->description && *new_descr->description)
		bc += strlen(new_descr->description);
	    else
		fprintf(stderr, "No desc in room %ld\n", rp->number);

	    new_descr->next = rp->ex_description;
	    rp->ex_description = new_descr;
	    break;
	case 'S':		/* end of current room */

#if BYTE_COUNT
	    if (bc >= 1000)
		fprintf(stderr, "Byte count for this room[%ld]: %d\n",rp->number,  bc);
#endif
	    total_bc += bc;
	    room_count++;
	    return 1;

	default:
	   sprintf(buf,"unknown auxiliary code `%s' in room load of #%ld",
		    chk, rp->number);
	    log_msg(buf);
	    break;
	}
    }

    return 1;
}
  
  

/* load the rooms */
void boot_world(const char* file)
{
    FILE *fl;
    long virtual_nr;
    struct room_data	*rp;
    char* ptr, buf[256];
  
#ifdef HASH
    init_world();
#endif    

    if (!(fl = fopen(file, "r")))	{
	perror(file);
	log_msg("boot_world: could not open world file.");
	exit(0);
    }
  
    while(!feof(fl))
    {
	if(!(ptr = fgets(buf, sizeof(buf), fl)))
	    break;

	if(*ptr == '#')
	{
	    ptr++;
	    if(!parse_number("room file", "vnum", virtual_nr,
			     (const char**) &ptr, &virtual_nr))
	    {
		log_msg("while looking for next room");
		continue;
	    }
	    rp = room_find_or_create(virtual_nr);
	    bzero(rp, sizeof(*rp));
	    rp->number = virtual_nr;
	    load_one_room(fl, rp);
	}
	else if(!strncmp(ptr, "$~", 2))
	    break;
	else
	{
	    char logbuf[256];
	    if((ptr = strchr(buf, '\n')))
		*ptr = 0;
	    sprintf(logbuf, "after #%ld, skipping: \"%s\"", virtual_nr, buf);
	    log_msg(logbuf);
	}
    }
  
    fclose(fl);
}

/* read direction data */
int setup_dir(FILE *fl, int room, int dir)
{
    struct room_data	*rp, dummy;
    char* ptr;
    char buf[256];
    
    rp = real_roomp(room);

    if (!rp) {
	sprintf(buf, "setup_dir(%d, %d): no such room!\n", room, dir);
	log_msg(buf);
	
	rp = &dummy;            /* this is a quick fix to make the game */
	dummy.number = room;	/* stop crashing   */
    }

    CREATE(rp->dir_option[dir], struct room_direction_data, 1);

    rp->dir_option[dir]->general_description = fread_string(fl);
    rp->dir_option[dir]->keyword = fread_string(fl);

    ptr = fgets(buf, sizeof(buf), fl);
    
    GETN(rp->dir_option[dir]->exit_info);
    
    GETN(rp->dir_option[dir]->key);
    GETN(rp->dir_option[dir]->to_room);

    return 1;
}


#define LOG_ZONE_ERROR(type, zone, cmd) (\
	sprintf(buf, \
		"error resolving %s number: zone %s: cmd (%c %d %d %d)", \
		type, zone_table[zone].name, \
		cmd->command, cmd->arg1, cmd->arg2, cmd->arg3), \
	log_msg(buf) \
	)
void renum_zone_table(void)
{
    int zone, comm, rnum, rnum2 = 0;
    struct reset_com *cmd;
    char	buf[256];
  
    for (zone = 0; zone <= top_of_zone_table; zone++)
	for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
	    switch((cmd = zone_table[zone].cmd +comm)->command) {
	    case 'M':
		rnum = real_mobile(cmd->arg1);
		if (rnum<0)
		    LOG_ZONE_ERROR("mobile", zone, cmd);
		else
		{
		    if(cmd->arg3<0)
			LOG_ZONE_ERROR("room", zone, cmd);
		    if(cmd->arg2 > mob_index[rnum].limit)
			mob_index[rnum].limit = cmd->arg2;
		}
		cmd->arg1 = rnum;
		break;
	    case 'C':
		rnum = real_mobile(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("mobile", zone, cmd);
		else
		{
		    if(cmd->arg3<0)
			LOG_ZONE_ERROR("room", zone, cmd);
		    if(cmd->arg2 > mob_index[rnum].limit)
			mob_index[rnum].limit = cmd->arg2;
		}
		cmd->arg1 = rnum;
		break;
	    case 'L':
		rnum = real_object(cmd->arg1);
		if(rnum < 0)
		    LOG_ZONE_ERROR("object", zone, cmd);
		else if(cmd->arg2 > obj_index[rnum].limit)
		    obj_index[rnum].limit = cmd->arg2;
		cmd->arg1 = rnum;
		break;
	    case 'O':
		rnum = real_object(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("object", zone, cmd);
		else
		{
		    if((cmd->arg3 != NOWHERE) && (cmd->arg3<0))
			LOG_ZONE_ERROR("room", zone, cmd);
		    if(cmd->arg2 > obj_index[rnum].limit)
			obj_index[rnum].limit = cmd->arg2;
		}
		cmd->arg1 = rnum;
		break;
	    case 'G':
		rnum = real_object(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("object", zone, cmd);
		else
		{
		    if(cmd->arg2 > obj_index[rnum].limit)
			obj_index[rnum].limit = cmd->arg2;
		}
		cmd->arg1 = rnum;
		break;
	    case 'E':
		rnum = real_object(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("object", zone, cmd);
		else
		{
		    if(cmd->arg2 > obj_index[rnum].limit)
			obj_index[rnum].limit = cmd->arg2;
		}
		cmd->arg1 = rnum;
		break;
	    case 'P':
		rnum = real_object(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("object", zone, cmd);
		else
		{
		    rnum2 = real_object(cmd->arg3);
		    if(rnum2<0)
			LOG_ZONE_ERROR("object", zone, cmd);
		    if(cmd->arg2 > obj_index[rnum].limit)
			obj_index[rnum].limit = cmd->arg2;
		}
		
		cmd->arg1 = rnum;
		cmd->arg3 = rnum2;
		break;					
	    case 'D':
		if(cmd->arg1<0)
		    LOG_ZONE_ERROR("room", zone, cmd);
		break;
	    case 'X':
		rnum = real_object(cmd->arg1);
		if(rnum<0)
		    LOG_ZONE_ERROR("object", zone, cmd);

		else if (cmd->arg2 == NOWHERE)
		    LOG_ZONE_ERROR("room", zone, cmd);

		cmd->arg1 = rnum;
		break;
	    }
}


/* load the zone table and command tables */
#undef GETN
#define GETN(fld) \
{ \
    long tmp; \
    if(!parse_number("zone file", #fld, zon, (const char**) &ptr, &tmp)) \
	break; \
    fld = tmp; \
}

void boot_zones(const char* file)
{
    FILE *fl;
    int zon, zc;
    int cmd_no, cc, ignored;
    char *ptr, buf[256];
  
    if (!(fl = fopen(file, "r")))	{
	perror(file);
	exit(0);
    }
  
    for(zon = 0, zc = 0 ; ; )
    {
	if(!(ptr = fgets(buf, sizeof(buf), fl)))
	    break;

	if(*ptr == '$')
	    break;

	if(*ptr++ != '#')
	    continue;
	
	GETN(ignored);
	
	/* alloc a new zone */
	if (!zon)
	{
	    zc = 100;
	    CREATE(zone_table, struct zone_data, zc);
	}
	else if (zon >= zc) {
	    zc += 10;
	    RECREATE(zone_table, struct zone_data, zc);
	}
    
	zone_table[zon].name = fread_string(fl);
	if(!(ptr = fgets(buf, sizeof(buf), fl)))
	    break;

	GETN(zone_table[zon].top);
	GETN(zone_table[zon].lifespan);
	GETN(zone_table[zon].reset_mode);

	/* reset mode determines if a zone can be swapped or not
	 * Add value of ZO_CANT_SWAP to the reset mode to deny swapping
	 * SWAPABLE is the default
	 */
	if (zone_table[zon].reset_mode>=ZO_CANT_SWAP)
	{
	  /* Normalize the reset_mode */
	  zone_table[zon].reset_mode-=ZO_CANT_SWAP;
#ifdef SWAP_ZONES
	  zone_table[zon].can_swap=0;
#endif
	}
#ifdef SWAP_ZONES
	else
	  zone_table[zon].can_swap=1;
#endif

	zone_table[zon].cmd = 0;
	
	/* read the command table */
	for(cmd_no = 0, cc = 0 ; ; )
	{
	    if(!(ptr = fgets(buf, sizeof(buf), fl)))
		break;
      
	    while(isspace(*ptr))
		ptr++;
	    
	    if(*ptr == '*')
		continue;
      
	    if (!zone_table[zon].cmd)
	    {
		cc = 50;
		CREATE(zone_table[zon].cmd, struct reset_com, cc);
	    }
	    else if (cmd_no >= cc)
	    {
		cc += 20;
		RECREATE(zone_table[zon].cmd, struct reset_com, cc);
	    }

	    zone_table[zon].cmd[cmd_no].command = *ptr;

	    if(*ptr++ == 'S')
		break;
      
	    GETN(zone_table[zon].cmd[cmd_no].if_flag);
	    GETN(zone_table[zon].cmd[cmd_no].arg1);

	    if (zone_table[zon].cmd[cmd_no].command != 'R')
		GETN(zone_table[zon].cmd[cmd_no].arg2);
      
	    if(zone_table[zon].cmd[cmd_no].command == 'M' ||
	       zone_table[zon].cmd[cmd_no].command == 'O' ||
	       zone_table[zon].cmd[cmd_no].command == 'C' ||
	       zone_table[zon].cmd[cmd_no].command == 'E' ||
	       zone_table[zon].cmd[cmd_no].command == 'P' ||
	       zone_table[zon].cmd[cmd_no].command == 'D')
		GETN(zone_table[zon].cmd[cmd_no].arg3);

	    cmd_no++;
	}
	zon++;
    }

    top_of_zone_table = --zon;

    fclose(fl);
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */



/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
    int i;
    event_t * update_u;
  
    /* enqueue zones */
  
    for (i = 0; i <= top_of_zone_table; i++)
    {
	if(!zone_table[i].reset_mode)
	    continue;

	if((zone_table[i].age != ZO_DEAD) &&
	   (zone_table[i].age++ > zone_table[i].lifespan))
	{
	    /* enqueue zone */
	    char buf[128];

	    CREATE(update_u, event_t, 1);
	
	    zone_table[i].age = ZO_DEAD;

	    sprintf(buf,"%d",i);
	    event_queue_pulse((event_t*) update_u,
			      number(0, PULSE_PER_MUD_HOUR),
			      (event_func) zone_update_proc,
			      buf);
	}
    }
}

void update_event_free(reset_q_element* theEvent)
{
    if(theEvent->event.tag)	FREE(theEvent->event.tag);
    FREE(theEvent);
}

int zone_can_swap(int zone)
{
  assert( (zone >= 0) && (zone <= top_of_zone_table) );

  return ( zone_table[zone].can_swap && !zone_table[zone].swapped && 
	   is_empty(zone));
}  

int zone_update_proc(event_t* update_u, int now)
{

#ifdef SWAP_ZONES
  int zone;
  
  /* if the zone is swapable and is empty, then swap it out */
  if ((!update_u->tag) || (!*update_u->tag))
    return 0;
  sscanf(update_u->tag, "%d", &zone);

  if (zone_can_swap(zone))
  {
    swap_zone(zone);
    slog("zone_update_proc:swap log okay");
    event_free(update_u);
    slog("zone_update_proc:event_free okay");
    return 0;
  }

  else
#endif
    if ((zone_table[zone].reset_mode == 2) ||
	is_empty(zone))
    {
      reset_zone(zone, 0);
      
      event_free(update_u);
    }
    else
      event_queue_pulse((event_t*) update_u,
			pulse,
			(event_func) zone_update_proc,
			NULL);

  return 0;
}

#define ZCMD zone_table[zone].cmd[cmd_no]


void clean_one_room(int rnum, struct room_data *rp, int *range)
{
  struct char_data	*vict, *next_v;
  struct obj_data	*obj, *next_o;

  if (rnum==0 ||		/* purge the void?  I think not */
      rnum < range[0] || rnum > range[1])
    return;

  send_to_room("The world is cleaner.\n\r", rnum);

  for (vict = rp->people; vict; vict = next_v) {
    next_v = vict->next_in_room;
    if(!IS_PC(vict))
      purge_char(vict);
  }
      
  for (obj = rp->contents; obj; obj = next_o) {
    next_o = obj->next_content;
    extract_obj(obj);
  }

  while (rp->people) {
    vict = rp->people;
    nolog("Zone swapped with pc present -- linkdead?");
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(vict);
    log_msg("clean_one_room: Sending character to the void");
    char_to_room(vict, 0);	/* send character to the void */
  }
}

/* swap out a zone by purging all rooms and clearing protos with 0 existing */
void swap_zone(int zone)
{
  int range[2];
  char buf[MAX_STRING_LENGTH];

  assert( (zone >= 0) && (zone <= top_of_zone_table) );

  if(zone_table[zone].swapped)
    log_msg("Swapping a zone thats is already swapped!");

  swapped_zones++;
  loaded_zones--;

  /* find top and bottom of zone */	
  range[1]=zone_table[zone].top;
  if (zone==0)
    range[0]=0;
  else 
    range[0]=zone_table[zone-1].top + 1;

  /* purge the zone */
  room_iterate((room_iterate_func) clean_one_room, range);

  zone_table[zone].swapped=1;
  zone_table[zone].age = ZO_DEAD;

  sprintf(buf,"SWAPPING OUT zone (%d) %s with lifespan of %dm",zone, 
	  zone_table[zone].name, zone_table[zone].lifespan);
  slog(buf);
  stat_log(buf,0);  /* swapping out zone */
}

/* execute the reset command table of a given zone */
void reset_zone(int zone, int init)
{
    int cmd_no, last_cmd = 1;
    char buf[256];
    struct char_data *mob;
    struct char_data *master;
    struct obj_data *obj = NULL, *obj_to;
    struct room_data	*rp;
    long now;
    int i, j, bot, top;

    now=time(0);
    if(now > zone_table[zone].reset_cycle) /* If time for reset_cycle */
    {
      zone_table[zone].reset_cycle= now + (long)
	number(2*SECS_PER_REAL_HOUR, 8*SECS_PER_REAL_HOUR);
      
      /* find zone boundaries */
      top=zone_table[zone].top;
      if (zone==0)
	bot=0;
      else 
	bot=zone_table[zone-1].top + 1;
      /* change all "loaded" values within bot/top object_index */
      for(i=bot; i<top; i++)
      {
	if((j=real_object(i))!=-1)
	if (obj_index[j].limit==ONCE_PER_REBOOT)
	    obj_index[j].loaded=0;
      }
    }

    if (zone_table[zone].swapped)
    {
      loaded_zones++;
      swapped_zones--;
      zone_table[zone].swapped = 0;
    }

    mob = 0;
    master = 0;
    
    for (cmd_no = 0;;cmd_no++) {
	if (ZCMD.command == 'S')
	    break;
       
	ZCMD.last_cmd = 1;

	if ((ZCMD.if_flag <= 0) ||
	    (((cmd_no - ZCMD.if_flag) >= 0) &&
	     zone_table[zone].cmd[cmd_no - ZCMD.if_flag].last_cmd))
	{
	    last_cmd = 0;
	    switch(ZCMD.command) {
	    case 'M':		/* read a mobile */
		if(!real_roomp(ZCMD.arg3))
		{
		    sprintf(buf, "Loading mob in non-existent room: #%d",
			    ZCMD.arg3);
		    log_msg(buf);
		}
	        else if(ZCMD.arg1 == -1) {
		}
		else if((mob_index[ZCMD.arg1].number < ZCMD.arg2) &&
			(mob = make_mobile(ZCMD.arg1, REAL)))
		{
		    mob->specials.zone = zone;
		    char_to_room(mob, ZCMD.arg3);
		    last_cmd = 1;
		    master = mob;
		}
		else
		  master=NULL;
	       
		break;

	    case 'C':		/* read a mobile.  Charm them to follow prev. */
		if(!master)
		{
		    sprintf(buf, "Loading charmed with no master: #%d",
			    mob_index[ZCMD.arg1].virt);
		    log_msg(buf);
		}
	        else if(ZCMD.arg1 == -1) {
		}
		else if((mob_index[ZCMD.arg1].number < ZCMD.arg2) &&
			(mob = make_mobile(ZCMD.arg1, REAL)))
		{
		    mob->specials.zone = zone;
		    char_to_room(mob, master->in_room);
		    /*
		      add the charm bit to the dude.
		      */
		    add_follower(mob, master, 0);
		    SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
		    SET_BIT(mob->specials.mob_act, ZCMD.arg3);
		    last_cmd = 1;
		}
		break;
	    
	    case 'L':		/* set limit, we did this already so ignore */
		last_cmd = 1;
		break;
		
	    case 'O':		/* read an object */
		if(!real_roomp(ZCMD.arg3))
		{
		    sprintf(buf, "Loading object in nonexistent room #%d",
			    ZCMD.arg3);
		    log_msg(buf);
		}
	       else if(ZCMD.arg1 != -1)
	       {
		    if((obj = make_object(ZCMD.arg1, REAL)) == NULL)
		    {
			sprintf(buf, "Problem loading object %d",
				obj_index[ZCMD.arg1].virt);
			log_msg(buf);
		    }
		    else
		    {
		      /* Reminder pointer --Mnemosync */
		      if(UnderLimit(ZCMD.arg1, ZCMD.arg2, init,
				    obj->obj_flags.level, obj->obj_flags.type_flag)) {
			obj_to_room(obj, ZCMD.arg3);
			last_cmd = 1;
		      } else
			extract_obj(obj);  //send object to void
		    }
		}
		break;
	    
	    case 'P':		/* object to object */
		{
		   if((ZCMD.arg1 == -1) || (ZCMD.arg2 == -1)) {
		   }
		    else if((obj = make_object(ZCMD.arg1, REAL)) == NULL)
		    {
			sprintf(buf, "Problem loading object %d",
				obj_index[ZCMD.arg1].virt);
			log_msg(buf);
		    }
		    else if((obj_to = get_obj_num(ZCMD.arg3)) == NULL)
		    {
		        sprintf(buf, "No object #%d for new object",
				obj_index[ZCMD.arg3].virt);
			log_msg(buf);
			extract_obj(obj);  //send object to void
		    }
		    else
		    {
		      /* reminder pointer --Mnemosync */
		      if(UnderLimit(ZCMD.arg1, ZCMD.arg2, init,
				    obj->obj_flags.level, obj->obj_flags.type_flag)) {
			obj_to_obj(obj, obj_to);
			last_cmd = 1;
		      } else
			extract_obj(obj);  //send object to void
		    }
		}
		break;
	    
	    case 'G':		/* obj_to_char */
		if(!mob)
		{
		    sprintf(buf, "No mob to give #%d to", 
				obj_index[ZCMD.arg1].virt);
		    log_msg(buf);
		}
                else if(ZCMD.arg1 != -1)
                {
		    if((obj = make_object(ZCMD.arg1, REAL)) == NULL)
		    {
			sprintf(buf, "Problem loading object: %d",
				obj_index[ZCMD.arg1].virt);
			log_msg(buf);
		    }
		    else
		    {
		      /* Reminder pointer --Mnemosync */
		      if(UnderLimit(ZCMD.arg1, ZCMD.arg2, init,
				    obj->obj_flags.level, obj->obj_flags.type_flag)) {
			obj_to_char(obj, mob);
			last_cmd = 1;
		      } else
			extract_obj(obj); //send object to void
		    }
		}
		break;
	    
	    case 'H':		/* hatred to char */
		if(!mob)
		{
		    sprintf(buf, "Nobody gets a hatred! (%d, %d)",
			    ZCMD.arg1, ZCMD.arg2);
		    log_msg(buf);
		}
		else
		{
		    AddHatred(mob, ZCMD.arg1, ZCMD.arg2);
		    last_cmd = 1;
		}
		break;
	    
	    case 'F':		/* fear to char */
		if(!mob)
		{
		    sprintf(buf, "Nobody gets a fear! (%d, %d)",
			    ZCMD.arg1, ZCMD.arg2);
		    log_msg(buf);
		}
		else
		{
		    AddFears(mob, ZCMD.arg1, ZCMD.arg2);
		    last_cmd = 1;
		}
		break;
	    
	    case 'E':		/* object to equipment list */
		if(!mob)
		{
		    sprintf(buf, "Nobody equipped with %d (%d)",
			    obj_index[ZCMD.arg1].virt, ZCMD.arg3);
		    log_msg(buf);
		}
		else if(ZCMD.arg1 != -1)
		{
		    if(mob->equipment[ZCMD.arg3])
		    {
			char buf[256];
			sprintf(buf, "Error in zone file: mob %d equips location %d more than once",
				mob_index[mob->nr].virt, ZCMD.arg3);
			log_msg(buf);
		    }
		    else if(!(obj = make_object(ZCMD.arg1, REAL)))
		    {
			sprintf(buf, "Problem making object: %d",
				obj_index[ZCMD.arg1].virt);
			log_msg(buf);
		    }
		    else
		    {
		      if(ZCMD.arg3<0 || ZCMD.arg3>MAX_WEAR)
		      {
			sprintf(buf, "Cannot equip object: E %d %d %d %d",
				ZCMD.if_flag,obj_index[ZCMD.arg1].virt,
				ZCMD.arg2,ZCMD.arg3);
			log_msg(buf);
			extract_obj(obj);
		      }
		      else
		      {
			/* Reminder pointer --Mnemosync */ 
		        if(UnderLimit(ZCMD.arg1, ZCMD.arg2, init,
				      obj->obj_flags.level, obj->obj_flags.type_flag)) {
			   equip_char(mob, obj, ZCMD.arg3);
			   last_cmd = 1;
			} else
			   extract_obj(obj);
		      }
		    }
		}
		break;
	    
	    case 'D':		/* set state of door */
		if((rp = real_roomp(ZCMD.arg1)) == NULL)
		{
		    sprintf(buf, "Setting door in non-existent room: %d",
			    ZCMD.arg1);
		    log_msg(buf);
		}
		else if(rp->dir_option[ZCMD.arg2] == NULL)
		{
		    sprintf(buf, "Setting non existent door in room: %d,%d",
			    ZCMD.arg1, ZCMD.arg2);
		    log_msg(buf);
		}
		else
		{
		    switch (ZCMD.arg3) {
		    case 0:
			REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				   EX_LOCKED);
			REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				   EX_CLOSED);
			break;
		    case 1:
			SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				EX_CLOSED);
			REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				   EX_LOCKED);
			break;
		    case 2:
			SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				EX_LOCKED);
			SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info,
				EX_CLOSED);
			break;
		    }
		    last_cmd = 1;
		}
		break;

	    case 'R':
		last_cmd = number(0, 99) < ZCMD.arg1;
		break;

	    case 'X':
		if ((rp = real_roomp(ZCMD.arg2)) &&
		    (obj = get_obj_in_list_num(ZCMD.arg1, rp->contents)))
		{
		    obj_from_room(obj);
		    extract_obj(obj);
		}
		last_cmd = 1;
		break;

	    default:
		sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.",
			zone, cmd_no);
		log_msg(buf);
		break;
	    }

	    ZCMD.last_cmd = last_cmd;
	}
	else
	    ZCMD.last_cmd = 0;
    }

#ifdef SWAP_ZONES
    /* If this zone never reboots, set age to ZO_DEAD else age = 0 */
    if (zone_table[zone].reset_mode==0)
      zone_table[zone].age=ZO_DEAD;
    else
#endif
      zone_table[zone].age = 0;
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
    struct descriptor_data *i;

    EACH_DESCRIPTOR(d_iter, i)
    {
	if (!i->connected && 
	    (real_roomp(i->character->in_room)->zone == zone_nr))
	    break;
    }
    END_ITER(d_iter);

    return i == NULL;
}

void Zwrite (FILE *fp, char cmd, int tf, int arg1, int arg2, int arg3, 
	     const char *desc)
{
   char buf[100];

   if (*desc) {
     sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3, desc);
     fputs(buf, fp);
   } else {
     sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3); 
     fputs(buf, fp);
   }
}

void RecZwriteObj(FILE *fp, struct obj_data *o)
{
   struct obj_data *t;

   if (ITEM_TYPE(o) == ITEM_CONTAINER) {
     for (t = o->contains; t; t=t->next_content) {
       Zwrite(fp, 'P', 1, ObjVnum(t), obj_index[t->item_number].number,
	      ObjVnum(o), OBJ_SHORT(t));
       RecZwriteObj(fp, t);
     }
   } else {
     return;
   }
}

FILE *MakeZoneFile( struct char_data *c, char *zonename)
{
  char buf[256];
  FILE *fp;

  if(zonename == '\0' || (strlen(zonename) <= 0))
    sprintf(buf, "zone/%s.zon", GET_NAME(c));
  else
    sprintf(buf, "zone/%s.zon", zonename);

  if ((fp = fopen(buf, "w")) != NULL)
    return(fp);
  else
    return(0);

}

void init_world(void)
{
#if HASH
/* PAC MODIFIED */
  init_hash_table(&room_db, sizeof(struct room_data *), 2047);
#endif
}

struct room_data *room_find_or_create(room_num key)
{
  struct room_data *rv;

  rv = real_roomp(key);
  if (rv)
    return rv;
  
  CREATE(rv, struct room_data, 1);

  room_enter(key, rv);

  return rv;
}

int room_enter(room_num key, struct room_data* rv)
{
  if((key <= NOWHERE) || (key >= WORLD_SIZE))
  {
    char buf[256];
    sprintf(buf, "room_enter: Illegal room #%ld", key);
    log_msg(buf);
    return 0;
  }

#if HASH
  return hash_enter(&room_db, key, rv);
#else
  
  if(room_db[key])
    return 0;

  room_db[key] = rv;
  if(key > top_of_world)
    top_of_world = key;
  return 1;
#endif
}

int room_remove(room_num key)
{
   struct room_data *tmp;

#if HASH
   tmp = hash_remove(&room_db, key);
#else
   tmp = room_db[key];
   room_db[key] = 0;
#endif

   if (tmp)
     FREE(tmp);

   return(0);
}

void room_iterate(room_iterate_func func, void *cdata)
{
#if HASH
  hash_iterate(&room_db, (void (*) (int, void*, void*)) func, cdata);
#else
  register room_num	i;

  for (i=0; i<=top_of_world; i++)
  {
    struct room_data  *temp;
  
    if((temp = room_db[i]))
       (func)(i, temp, cdata);
  }
#endif
}

/*
**  this duplicates the code in room_find, because it is much quicker this way.
*/

struct room_data *real_roomp(room_num virt)
{
#if HASH
  return hash_find(&room_db, virt);
#else
  return((virt<=top_of_world&&virt>NOWHERE)?room_db[virt]:0);
#endif
}

int same_zone(struct char_data* ch, struct char_data* vict)
{
    struct room_data*	cr;
    struct room_data*	vr;

    if(!ch || !vict)
	return 0;
    
    cr = real_roomp(ch->in_room);
    vr = real_roomp(vict->in_room);

    return cr && vr && (cr->zone == vr->zone);
}

