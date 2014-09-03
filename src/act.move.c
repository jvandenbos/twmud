#include "config.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"
#include "act.h"
#include "spell_util.h"
#include "utility.h"
#include "constants.h"    
#include "multiclass.h"
#include "casino.h"
#include "util_str.h"
#include "find.h"
#include "statistic.h"
#include "proto.h"
#include "mobprog2.h"

// Trial to remove expense of moving
#define WALKING_MOVE_COST (movement_loss[(int) from_here->sector_type]+movement_loss[(int) to_here->sector_type]) / 2;
#define FLY_MOVE_COST 1
#define BOAT_MOVE_COST 1
#define AIR_MOVE_COST 1
#define INDOOR_MOVE_COST 2
#define LIQUID_MOVE_COST 15

/* forward declarations */
int CanFollow(struct char_data* ch, struct char_data* leader);

/*
  Some new movement commands for diku-mud.. a bit more object oriented.
  */

void NotLegalMove(struct char_data *ch)
{
  send_to_char("Alas, you cannot go that way...\n\r", ch);
}


int ValidMove( struct char_data *ch, int cmd) 
{
  char tmp[256];
  struct room_direction_data	*exitp;
  
  exitp = EXIT(ch, cmd);

  if (affected_by_spell(ch, SPELL_WEB)) {
    if (!saves_spell(ch, SAVING_PARA, 0)) {
      send_to_char("You are entrapped in sticky webs!\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return(FALSE);
    } else {
      affect_from_char(ch, SPELL_WEB);
      send_to_char("You pull free from the sticky webbing!\n\r", ch);
    }
  }

  if (MOUNTED(ch)) {
    if (GET_POS(MOUNTED(ch)) < POSITION_FIGHTING) {
      send_to_char("Your mount must be standing.\n\r", ch);
      return(FALSE);
    }
    if (ch->in_room != MOUNTED(ch)->in_room) {
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
  }
  
  if (!exit_ok(exitp,NULL)) {
    NotLegalMove(ch);
    return(FALSE);
  } else if (IS_SET(exitp->exit_info, EX_CLOSED)) {
      if (exitp->keyword) {
          // Solaar: Now admins can move through any door type.
          if (IS_SET(ch->specials.flags, PLR_STEALTH) && IS_IMMORTAL(ch)) {
              sprintf(tmp, "You move right through the %s.\n\r", fname(exitp->keyword));
              send_to_char(tmp, ch);
              return(TRUE);
          }
          // Solaar: removed condition that checks for "secret" in the door name.
          else if (!IS_SET(exitp->exit_info, EX_SECRET)) {
              if (IS_SET(ch->specials.mob_act, ACT_LIQUID))
                  return (TRUE);
              sprintf(tmp, "The %s seems to be closed.\n\r",
	          fname(exitp->keyword));
              send_to_char(tmp, ch);
              return(FALSE);
          } else {
              NotLegalMove(ch);
              return(FALSE);
          }
      } else {
          NotLegalMove(ch);
          return(FALSE);
      }
  } else {
    struct room_data *rp;
    rp = real_roomp(exitp->to_room);
    if (IS_SET(rp->room_flags, IMMORT_RM) && !IS_GOD(ch))
    {
        send_to_char("Who do you think you are, an Immortal?\n\r", ch);
        return(FALSE);
    }
    if (IS_SET(rp->room_flags, BRUJAH_RM) && !IS_SET(ch->specials.flags, PLR_BRUJAH))
    {
       send_to_char("You do not have the ability to go here.\n\r", ch);
       return(FALSE);
    }
    if (IS_SET(rp->room_flags, GOD_RM) && (TRUST(ch) < TRUST_LORD))
    {
        send_to_char("Who do you think you are, a God?\n\r", ch);
        return(FALSE);
    }     
    if (IS_SET(rp->room_flags, TUNNEL)) {
      if ((MobCountInRoom(rp->people) > rp->moblim) &&
	  (!IS_IMMORTAL(ch))) {
	send_to_char("Sorry, there is no room to get in there.\n\r", ch);
        return(FALSE);
      }
    }
    if(IS_SET(rp->room_flags, INDOORS) && MOUNTED(ch))
    {
	send_to_char("Your mount refuses to go that way.\n\r", ch);
	return(FALSE);
    }
    if(IS_SET(rp->room_flags, DEATH) && MOUNTED(ch)) {
	send_to_char("Your mount refuses to go that way.\n\r", ch);
	return(FALSE);
    }
    if (IS_SET(ch->specials.mob_act, ACT_HUGE) &&
        IS_SET(rp->room_flags, INDOORS)) {
      send_to_char("Sorry, but you are much too big to fit in there.\n\r", ch);
      return(FALSE);
    } 
    /*  Don't allow mobs without a ROAM flag or CHARMED leave their  zone   *
     *  -smw 4/30/96                                                        */
    if (!IS_PC(ch) && (rp->zone != real_roomp(ch->in_room)->zone)) {
	if (!IS_AFFECTED(ch, AFF_CHARM) &&
	    !IS_SET(ch->specials.mob_act, ACT_ROAM) &&
	    !(ch->hunt_info)) {
	   send_to_char("Mobs can't leave their zones!", ch);
	   return(FALSE);
        }
    }
    return(TRUE);
  }
}

int RawMove(struct char_data *ch, int dir)
{
    int need_movement, new_r;
    struct obj_data *obj;
    bool has_boat;
    bool has_air;
    struct room_data *from_here, *to_here;
    struct char_data* mount;

    if (special(ch, dir+1, ""))	/* Check for special routines(North is 1)*/
	return(FALSE);
  
    if (!ValidMove(ch, dir)) {
	return(FALSE); 
    }
  
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
	(ch->in_room == ch->master->in_room)) {
	act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
	act("You burst into tears at the thought of leaving $N",
	    FALSE, ch, 0, ch->master, TO_CHAR);
    
	return(FALSE);
    }
  
    from_here = real_roomp(ch->in_room);
    new_r = from_here->dir_option[dir]->to_room;
    to_here = real_roomp(new_r);
  
    /* Allow mounts to cross stay_zone and no_mob */
    /* Keep out charging monsters from no_mob and stay_zoned */
    if (!IS_PC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
    {
	if (IS_SET(ch->specials.mob_act, ACT_STAY_ZONE) &&
	    (to_here->zone != from_here->zone))
            return FALSE;
    }

    if(affected_by_spell(ch, SKILL_SPELL_SHIELD))
    {
         affect_from_char(ch, SKILL_SPELL_SHIELD);
         act("As $n moves, the globe around $m flares and dies.",
             TRUE, ch, NULL, NULL, TO_NOTVICT);
         send_to_char("As you move, the globe around you flares and dies!\n\r",ch);
    }

    if(affected_by_spell(ch, SPELL_CAMOUFLAGE) && !wilderness(ch)) {
	 affect_from_char(ch, SPELL_CAMOUFLAGE);
	 act("As $n leaves the wilds, $s camouflage is lost.",
	     TRUE, ch, NULL, NULL, TO_NOTVICT);
	 send_to_char("As you leave the wilds, your camouflage is lost.\n\r",ch);
     }
      
    if (to_here==NULL) {
	char_from_room(ch);
	log_msg("raw_move: Sending character to the void");
	char_to_room(ch, 0);
	send_to_char("Uh-oh.  The ground melts beneath you as you fall into the swirling chaos.\n\r",ch);
	do_look(ch, "\0",15);
    
	return TRUE;
    }
  
    if (IS_AFFECTED(ch,AFF_FLYING)) {
	need_movement = FLY_MOVE_COST;
	if (IS_SET(to_here->room_flags, INDOORS))
	    need_movement += INDOOR_MOVE_COST;
    } else {
      need_movement = WALKING_MOVE_COST;
    }
  
    if(!IS_IMMORTAL(ch))
    {
	/*  
	 *  Movement in water_nowswim
	 */
	if((from_here->sector_type == SECT_WATER_NOSWIM) || 
	   (to_here->sector_type == SECT_WATER_NOSWIM))
	{
	    if((mount = MOUNTED(ch)))
	    {
		if(!IS_AFFECTED(mount, AFF_FLYING))
		{
		    if(!IS_AFFECTED(mount, AFF_WATERBREATH))
		    {
			send_to_char(
				     "Your mount would have to fly or swim to go there.\n\r", ch);
			return(FALSE);
		    }
		    if(!IS_AFFECTED(ch, AFF_WATERBREATH))
		    {
			send_to_char(
				     "You would need to swim to go there.\n\r", ch);
			return FALSE;
		    }
		}
	    }
	    else if(!IS_AFFECTED(ch, AFF_FLYING))
	    {
		has_boat = FALSE;
		/* See if char is carrying a boat */
		for (obj=ch->carrying; obj; obj=obj->next_content)
		    if (obj->obj_flags.type_flag == ITEM_BOAT)
			has_boat = TRUE;
		if (!has_boat && !IS_AFFECTED(ch, AFF_WATERBREATH))
		{
		    send_to_char("You need a boat to go there.\n\r", ch);
		    return(FALSE);
		}
		if (has_boat)		  
		  need_movement = BOAT_MOVE_COST;
	    }
	}
  
	/*
	  Movement in SECT_AIR
	  */
	if ((from_here->sector_type == SECT_AIR) || 
	    (to_here->sector_type == SECT_AIR)) {
	    if((mount = MOUNTED(ch)))
	    {
		if(!IS_AFFECTED(mount, AFF_FLYING))
		{
		    send_to_char(
				 "Your mount would have to fly to go there!\n\r", ch);
		    return FALSE;
		}
	    }
	    else if (!IS_AFFECTED(ch,AFF_FLYING))
	    {
		send_to_char("You would have to fly to go there!\n\r",ch);
		return(FALSE);
	    }
	}
	
        /*
	  Movement in SECT_SKY
        */
        if ((from_here->sector_type == SECT_SKY) ||
            (to_here->sector_type == SECT_SKY))
        {
/*            if((mount = MOUNTED(ch)))
            {
                if(!IS_AFFECTED(mount, AFF2_FLIGHT))
                {
                   send_to_char(
                      "Your mount would have to fly to go there.\n\r", ch);
                   return(FALSE);
                }
            } */

            if(!IS_AFFECTED(ch, AFF2_FLIGHT))
            {
                has_air = FALSE;
                /* See if char is carrying a boat */
                for (obj=ch->carrying; obj; obj=obj->next_content)
                    if (obj->obj_flags.type_flag == ITEM_SKY)
                        has_air = TRUE;
                if (!has_air)
                {
                    send_to_char("You need assistance to fly that high.\n\r", ch);
                    return(FALSE);
                }
                if (has_air)
                    need_movement = AIR_MOVE_COST;
            }
        }
  
	/*
	  Movement in SECT_UNDERWATER
	  */
	if ((from_here->sector_type == SECT_UNDERWATER) || 
	    (to_here->sector_type == SECT_UNDERWATER))
	{
	    if (!IS_AFFECTED(ch,AFF_WATERBREATH)) {
		send_to_char("You would need gills to go there!\n\r",ch);
		return(FALSE);
	    }

	    if ((mount = MOUNTED(ch)) &&
		!IS_AFFECTED(mount, AFF_WATERBREATH))
	    {
		send_to_char("Your mount would need gills to go there!\n\r",
			     ch);
		return(FALSE);
	    }
	}
    }	

    if (IS_SET(ch->specials.mob_act, ACT_LIQUID))
      need_movement *=LIQUID_MOVE_COST; /* very expensive to move in this form */

    if (!MOUNTED(ch)) {
	if (GET_MOVE(ch)<need_movement) {
	    send_to_char("You are too exhausted.\n\r",ch);
	    return(FALSE);
	}
    } else {
	if (GET_MOVE(MOUNTED(ch))<need_movement) {
	    send_to_char("Your mount is too exhausted.\n\r", ch);
	    return(FALSE);
	}
    }
 
    if (!IS_IMMORTAL(ch)) {
	if (MOUNTED(ch)) {
	    GET_MOVE(MOUNTED(ch)) -= need_movement;
	} else {
	    GET_MOVE(ch) -= need_movement;
	}
    }  

    /*
     *  nail the unlucky with traps.
     */
    if (!MOUNTED(ch)) {
	if (CheckForMoveTrap(ch, dir))
	    return(FALSE);
    } else {
	if (CheckForMoveTrap(MOUNTED(ch), dir))
	    return(FALSE);
    }

    if (IS_SET(real_roomp(ch->in_room)->dir_option[dir]->exit_info, EX_CLOSED)
        && IS_SET(ch->specials.mob_act, ACT_LIQUID)) {
        send_to_char("You seep through the cracks and crevices...\n\r", ch);
    }

    if (MOUNTED(ch)) {  
	char_from_room(ch);
	char_to_room(ch, new_r);
	char_from_room(MOUNTED(ch));
	char_to_room(MOUNTED(ch), new_r);
    } else {
	char_from_room(ch);
	char_to_room(ch, new_r);
    }
    do_look(ch, "\0",15);
  
    if (IS_SET(to_here->room_flags, DEATH) && !IS_IMMORTAL(ch)) {

	do_death_trap(ch);

	return(FALSE);
    }

    /*
     **  do something with track 
     */

    if (ch->hunt_info && ch->desc)
    {
	if(IS_PC(ch))
	    send_to_char("You search for a trail.\n\r", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);
    }

    return(TRUE);
  
}



int MoveOne(struct char_data *ch, int dir)
{
  int was_in;
  
  was_in = ch->in_room;
  if (RawMove(ch, dir)) {	/* no error */
    DisplayOneMove(ch, dir, was_in);
    mprog_entry_trigger(ch);
    mprog_entry_trigger2(ch);
    mprog_greet_trigger(ch);
    mprog_greet_trigger2(ch);
    return TRUE;
  } else
    return FALSE;
  
}

void MoveGroup( struct char_data *ch, int dir)
{
    struct char_data *heap_ptr[50];
    int i, heap_top, heap_tot[50];
    room_num was_in;
    struct follow_type *k, *next_dude;
    mprog_entry_trigger(ch);
    mprog_entry_trigger2(ch);
    mprog_greet_trigger(ch);
    mprog_greet_trigger2(ch);
  
    /*
	*   move the leader. (leader never duplicates)
	    */
  
    was_in = ch->in_room;
    if (RawMove(ch, dir)) {	/* no error */
	DisplayOneMove(ch, dir, was_in);
	if (ch->followers) {
	    heap_top = 0;
	    for(k = ch->followers; k; k = next_dude) {
		next_dude = k->next;
		/*
		    *  compose a list of followers, w/heaping
			*/
		if ((was_in == k->follower->in_room) &&
		    (GET_POS(k->follower) >= POSITION_STANDING)) {
                    hide_test(k->follower, 0);
		    act("$CwYou follow $N.$CN", FALSE, k->follower, 0, ch, TO_CHAR);
      
		    if (k->follower->followers) {
			MoveGroup(k->follower, dir);
		    } else {
			if (RawMove(k->follower, dir) &&
			    !AddToCharHeap(heap_ptr, &heap_top,
					   heap_tot, k->follower))
			{
			    DisplayOneMove(k->follower, dir, was_in);
			}
		    }
		}
	    }
	}
	/*
	    *  now, print out the heaped display message
		*/
	for (i=0;i<heap_top;i++) {
	    if (heap_tot[i] > 1) {
		DisplayGroupMove(heap_ptr[i], dir, was_in, heap_tot[i]);
	    } else {
		DisplayOneMove(heap_ptr[i], dir, was_in);
	    }
	}
    }
}

void DisplayOneMove(struct char_data *ch, int dir, int was_in)
{
  DisplayMove(ch, dir, was_in, 1);
}

void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total)
{
  DisplayMove(ch, dir, was_in, total);
}


void do_move(struct char_data *ch, char *argument, int cmd)
{
  
  if (RIDDEN(ch)) {
    if (RideCheck(RIDDEN(ch), 0)) {
      do_move(RIDDEN(ch), argument, cmd);
      return;
    } else {
      FallOffMount(RIDDEN(ch), ch);
      Dismount(RIDDEN(ch), ch, POSITION_SITTING);
    }
  }

  if (GET_COND(ch, DRUNK) > 5)
    cmd = number_range(1, 4); /* change dir if drunk */
  
  cmd -= 1;
  
  /*
   ** the move is valid, check for follower/master conflicts.
   */
  
  if (ch->attackers > 1) {
    send_to_char("There's too many people around, no place to flee!\n\r", ch);
    return;
  }
  
  if (!ch->followers && !ch->master) {
    MoveOne(ch,cmd);
  } else {
    if (!ch->followers) {
      MoveOne(ch, cmd);
    } else {
      MoveGroup(ch, cmd);
    }
  }
}

void liquid_move(struct char_data *ch, int was_in, int dir)
{
  struct char_data *tmp_ch;
  char tmp[256];
  int flag;

  if (IS_SET((real_roomp(was_in)->dir_option[dir])->exit_info, EX_CLOSED))
    flag=TRUE;
  else
    flag=FALSE;

  for (tmp_ch = real_roomp(was_in)->people; tmp_ch;
    tmp_ch= tmp_ch->next_in_room) {
    if (ch!=tmp_ch && AWAKE(tmp_ch) && CAN_SEE(tmp_ch, ch)) {
      if (flag)
        sprintf(tmp,"%s seeps between the cracks %s.\n\r",
          PERS(ch, tmp_ch), dir_desc[dir]);
      else
        sprintf(tmp, "%s slithers off %s.\n\r",
          PERS(ch, tmp_ch), dir_desc[dir]);
      send_to_char_formatted(tmp, tmp_ch);
    }
  }

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
    tmp_ch= tmp_ch->next_in_room) {
    if (ch!=tmp_ch && AWAKE(tmp_ch) && CAN_SEE(tmp_ch, ch)) {
      if (flag)
        sprintf(tmp,"%s seeps between the cracks %s.\n\r",
          PERS(ch, tmp_ch), dir_from[dir]);
      else
        sprintf(tmp, "%s slithers in %s.\n\r",
          PERS(ch, tmp_ch), dir_from[dir]);
      send_to_char_formatted(tmp, tmp_ch);
    }
  }
}


/*
  MoveOne and MoveGroup print messages.  Raw move sends success or failure.
*/

void DisplayMove( struct char_data *ch, int dir, int was_in, int total)
{
  struct char_data *tmp_ch;
  char tmp[256];
  char *lverb = "leaves", *averb = "walks in";
  struct room_data *rp;

  if (IS_SET(ch->specials.mob_act, ACT_LIQUID)) {
    liquid_move(ch, was_in, dir);
    return;
  }

  if (IS_AFFECTED(ch, AFF_FLYING))
  {
      lverb = "floats";
      averb = "floats in";
  }      
  
  if ((rp = real_roomp(ch->in_room)) && (rp->sector_type == SECT_UNDERWATER))
  {
      lverb = "swims";
      averb = "swims in";
  }

  for (tmp_ch = real_roomp(was_in)->people; tmp_ch; 
       tmp_ch= tmp_ch->next_in_room) {
    if ((!IS_AFFECTED(ch, AFF_SNEAK) || IS_IMMORTAL(tmp_ch) ||
	 IS_SET(real_roomp(was_in)->room_flags, NO_SNEAK)) &&
	ch!=tmp_ch && AWAKE(tmp_ch) && CAN_SEE(tmp_ch, ch)) {
      if (MOUNTED(ch)) {
	sprintf(tmp, "%s leaves %s, riding on %s.\n\r",
		PERS(ch, tmp_ch), dirs[dir],
		PERS(MOUNTED(ch), tmp_ch));
      } else {
	sprintf(tmp,"%s %s %s.\n\r",
		PERS(ch, tmp_ch), lverb, dirs[dir]);
      }
      send_to_char_formatted(tmp, tmp_ch);
    }
  }
  
  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; 
       tmp_ch = tmp_ch->next_in_room) {
    if ((!IS_AFFECTED(ch, AFF_SNEAK) || IS_IMMORTAL(tmp_ch) ||
	 IS_SET(real_roomp(ch->in_room)->room_flags, NO_SNEAK)) &&
	CAN_SEE(tmp_ch,ch) && tmp_ch!=ch && AWAKE(tmp_ch)) {
      if (MOUNTED(ch)) {
	sprintf(tmp, "%s has arrived %s, riding on %s.\n\r", 
		PERS(ch, tmp_ch), dir_from[dir], 
		PERS(MOUNTED(ch), tmp_ch));
      } else {
	sprintf(tmp, "%s %s %s.\n\r", 
	        PERS(ch, tmp_ch), averb, dir_from[dir]);
      }
      send_to_char_formatted(tmp, tmp_ch);
    }
  }

}

int AddToCharHeap(struct char_data *heap[50], int *top, int total[50], 
		  struct char_data *k) 
{
    int found, i;
  
    if (*top > 50) {
	return(FALSE);
    } else {
	found = FALSE;
	for (i=0;(i<*top&& !found);i++) {
	    if (*top>0) {
		if ((IS_NPC(k)) &&
		    (k->nr == heap[i]->nr) &&
(!strcmp(GET_REAL_NAME(k), GET_REAL_NAME(heap[i]))))
		{
		    total[i] += 1;
		    found=TRUE;
		}
	    }          
	}
	if (!found) {
	    heap[*top] = k;
	    total[*top] = 1;
	    *top+=1;
	}
    }
    return TRUE;
}


int find_door(struct char_data *ch, char *type, char *dir)
{
  char buf[MAX_STRING_LENGTH];
  int door;
  const char *dirs[] = 
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
      };
  struct room_direction_data *exitp;
  
  if (*dir) { /* a direction was specified */ 
    if ((door = search_block(dir, dirs, FALSE)) == -1) { /* Partial Match */		
      send_to_char("That's not a direction.\n\r", ch);
      return(-1);
    }
    exitp = EXIT(ch, door);
    if (exitp) {
      if (!exitp->keyword)
	return(door);
      if ((isname(type, exitp->keyword))&&
	  (strcmp(type,"secret"))) {
	return(door);
      } else {	       	
	sprintf(buf, "I see no %s there.\n\r", type);
	send_to_char(buf, ch);
	return(-1);
      }
    } else {       	
      sprintf(buf, "I see no %s there.\n\r", type);
      send_to_char(buf, ch);
      return(-1);
    }
  } else { /* try to locate the keyword */
    for (door = 0; door <= 5; door++)
      if ((exitp=EXIT(ch, door)) &&
	  exitp->keyword &&
	  isname(type, exitp->keyword))
	return(door);
    
    sprintf(buf, "I see no %s here.\n\r", type);
    send_to_char(buf, ch);
    return(-1);
  }
}


void open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back = NULL;
  struct room_data	*rp;
  char	buf[MAX_INPUT_LENGTH];
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    sprintf(buf, "NULL rp in open_door() for %s.", PERS(ch,ch));
    log_msg(buf);
  }
  
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  if (exitp->keyword) {
    if (strcmp(exitp->keyword, "secret") &&
	(!IS_SET(exitp->exit_info, EX_SECRET))) {
      sprintf(buf, "$n opens the %s", fname(exitp->keyword));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    } else {
      act("$n reveals a hidden passage!", FALSE, ch, 0, 0, TO_ROOM);
    }
  } else
    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
  
  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))
    {
      REMOVE_BIT(back->exit_info, EX_CLOSED);
      if (back->keyword)	{
	sprintf(buf, "The %s is opened from the other side.\n\r",
		fname(back->keyword));
	send_to_room(buf, exitp->to_room);
      }
      else
	send_to_room("The door is opened from the other side.\n\r",
		     exitp->to_room);
    }						 
}

void do_run(struct char_data *ch, char *argument, int cmd)
{
  char brett[MAX_INPUT_LENGTH];
  int keyno, was_in;
  struct room_direction_data *exitdata;
  static const char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n" };

  only_argument(argument, brett);

  if (!*brett) {
    send_to_char("The proper format for this command is RUN <DIRECTION>.\n\r", ch);
    return;
  }

  keyno = search_block(brett, keywords, FALSE);

  if (keyno == -1) {
    send_to_char("Sorry, but that isn't a valid direction to run in.\n\r", ch);
    return;
  }

  if (GET_MOVE(ch) <= 20) {
    send_to_char("You feel too tired to run at this moment.\n\r", ch);
    return;
  }
  
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
      (ch->in_room == ch->master->in_room)) {
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    act("You burst into tears at the thought of running away from $N",
	FALSE, ch, 0, ch->master, TO_CHAR);
    return;
  }

  if (!CAN_GO(ch, keyno)) {
    send_to_char("Do you like to run into things doofus?  Please pick an open direction!", ch);
    return;
  }
  if (!clearpath(ch,ch->in_room,keyno)) {
    send_to_char("To run in that direction seems futile.\n\r", ch);
    return;
  }
  exitdata = (real_roomp(ch->in_room)->dir_option[keyno]);
  if((exitdata->to_room)==(ch->in_room)) {
    send_to_char("To run in that direction seems futile.\n\r", ch);
    return;
  }
 
  send_to_char("You take off, running as fast as you can!\n\r", ch);
  act("$n suddenly takes off, running as fast as $e can!",
      FALSE,ch,0,0,TO_ROOM);
  was_in=ch->in_room;
  while ((CAN_GO(ch, keyno)) && (GET_MOVE(ch) > 20) && (RawMove(ch, keyno))) {
    DisplayOneMove(ch, keyno, was_in);
    GET_MOVE(ch) -= 1;
    was_in=ch->in_room;
  }

  if (GET_MOVE(ch) > 25) {
    act("$n slows down to a screeching halt, exhausted from their run.",
        FALSE,ch,0,0,TO_ROOM);
    send_to_char("Sorry, but you can not run in this direction any further.\n\r", ch);
  } else {
      act("$n slows down to a screeching halt, panting heavily from their run.",
          FALSE,ch,0,0,TO_ROOM);
      send_to_char("You feel too tired to run any further.\n\r", ch);
    }

}

void do_open(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  struct char_data *victim;
  struct room_direction_data	*exitp;
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Open what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already open!\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("You can't do that.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else   	{
      REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* perhaps it is a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already open!\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else
      {
	open_door(ch, door);
	send_to_char("Ok.\n\r", ch);
      }
  }
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back = NULL, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data	*rp;
  
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Close what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already closed!\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("That's impossible.\n\r", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	send_to_char("Ok.\n\r", ch);
	act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* Or a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\n\r", ch);
    else      {
      SET_BIT(exitp->exit_info, EX_CLOSED);
      if (exitp->keyword)
	act("$n closes the $F.", 0, ch, 0, exitp->keyword,
	    TO_ROOM);
      else
	act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for closing the other side, too */
      if (exit_ok(exitp,&rp) &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  (back->to_room == ch->in_room) ) {
	SET_BIT(back->exit_info, EX_CLOSED);
	if (back->keyword)    {	      
	  sprintf(buf, "The %s closes quietly.\n\r", back->keyword);
	  send_to_room_formatted(buf, exitp->to_room);
	}
	else
	  send_to_room( "The door closes quietly.\n\r", exitp->to_room);
      }						 
    }
  }
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;
  
  for (o = ch->carrying; o; o = o->next_content)
    if (obj_index[o->item_number].virt == key)
      return(1);
  
  if (ch->equipment[HOLD])
    if (obj_index[ch->equipment[HOLD]->item_number].virt == key)
      return(1);
  
  return(0);
}


void do_lock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data *rp;
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Lock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Maybe you should close it first...\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("That thing can't be locked.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);	
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It is locked already.\n\r", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Cluck*\n\r", ch);
	act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* a door, perhaps */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("There does not seem to be any keyholes.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("You don't have the proper key.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\n\r", ch);
    else
      {
	SET_BIT(exitp->exit_info, EX_LOCKED);
	if (exitp->keyword)
	  act("$n locks the $F.", 0, ch, 0,  exitp->keyword,
	      TO_ROOM);
	else
	  act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("*Click*\n\r", ch);
	/* now for locking the other side, too */
	rp = real_roomp(exitp->to_room);
	if (rp &&
	    (back = rp->dir_option[rev_dir[door]]) &&
	    back->to_room == ch->in_room)
	  SET_BIT(back->exit_info, EX_LOCKED);
      }
  }
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data *rp;
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Unlock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);	
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
    else
      {
	REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* it is a door */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("You can't seem to spot any keyholes.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("You do not have the proper key for that.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\n\r", ch);
    else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword)
	act("$n unlocks the $F.", 0, ch, 0, exitp->keyword,
	    TO_ROOM);
      else
	act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if (rp &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  back->to_room == ch->in_room)
	REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

void do_enter(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  
  one_argument(argument, buf);
  
  if (*buf) { /* an argument was supplied, search for door keyword */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), NULL) && exitp->keyword &&
	  0==str_cmp(exitp->keyword, buf)) {
	do_move(ch, "", ++door);
	return;
      }
    sprintf(tmp, "There is no %s here.\n\r", buf);
    send_to_char(tmp, ch);
  } else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    send_to_char("You are already indoors.\n\r", ch);
  } else {
    /* try to locate an entrance */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	  !IS_SET(exitp->exit_info, EX_CLOSED) &&
	  IS_SET(rp->room_flags, INDOORS))
	{
	  do_move(ch, "", ++door);
	  return;
	}
    send_to_char("You can't seem to find anything to enter.\n\r", ch);
  }
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
  int door;
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  
  if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS))
    send_to_char("You are outside.. where do you want to go?\n\r", ch);
  else    {
      for (door = 0; door <= 5; door++)
	if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	    !IS_SET(exitp->exit_info, EX_CLOSED) &&
	    !IS_SET(rp->room_flags, INDOORS)) {
	  do_move(ch, "", ++door);
	  return;
	}
      send_to_char("I see no obvious exits to the outside.\n\r", ch);
    }
}


void do_stand(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) {
  case POSITION_STANDING : { 
    act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SITTING	: { 
    if(IS_AFFECTED(ch,AFF_MEDITATE)) {
      affect_from_char(ch,SKILL_MEDITATE);
      REMOVE_BIT(AFF_FLAGS(ch), AFF_MEDITATE);
      send_to_char("You enter a more conscious state, as you stop meditating.\n\r", ch);
    }
    if(check_blackjack(ch))
      do_blackjack_exit(ch);
    act("$CyYou stand up.$CN", FALSE, ch,0,0,TO_CHAR);
    act("$Cy$n clambers on $s feet.$CN",TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_RESTING	: { 
    if(IS_AFFECTED(ch,AFF_MEDITATE)) {
      affect_from_char(ch,SKILL_MEDITATE);
      REMOVE_BIT(AFF_FLAGS(ch), AFF_MEDITATE);
      send_to_char("You enter a more conscious state, as you stop meditating.\n\r", ch);
    }
    act("$CyYou stop resting, and stand up.$CN", FALSE, ch,0,0,TO_CHAR);
    act("$Cy$n stops resting, and clambers on $s feet.$CN", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_SLEEPING : { 
    act("$CYYou have to wake up first!$CN", FALSE, ch, 0,0,TO_CHAR);
  } break;
  case POSITION_FIGHTING : { 
    act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_MOUNTED: {
    send_to_char("Not while riding you don't!\n\r", ch);
    break;
  }
    default : { 
      act("You stop floating around, and put your feet on the ground.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } break;
  }
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
  if(check_blackjack(ch))
    if(!do_blackjack_enter(ch))
      return;
 
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SITTING	: {
    send_to_char("You're sitting already.\n\r", ch);
  } break;
  case POSITION_RESTING	: {
    act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Sit down while fighting? are you MAD?", FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_MOUNTED: {
    send_to_char("Not while riding you don't!\n\r", ch);
    break;
  }
    default : {
      act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_rest(struct char_data *ch, char *argument, int cmd) 
{
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_SITTING : {
    if(check_blackjack(ch))
      do_blackjack_exit(ch);
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_RESTING : {
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_FIGHTING : {
    act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_MOUNTED: {
    send_to_char("Not while riding you don't!\n\r", ch);
    break;
  }
    default : {
      act("You stop floating around, and stop to rest your tired bones.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_sleep(struct char_data *ch, char *argument, int cmd) 
{
  
  switch(GET_POS(ch)) {
  case POSITION_STANDING : 
  case POSITION_SITTING  :
  case POSITION_RESTING  : {
    send_to_char("You go to sleep.\n\r", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    if(GET_POS(ch) == POSITION_SITTING) {
      if(check_blackjack(ch))
        do_blackjack_exit(ch);
    }
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  case POSITION_SLEEPING : {
    send_to_char("You are already sound asleep.\n\r", ch);
  } break;
  case POSITION_FIGHTING : {
    send_to_char("Sleep while fighting? are you MAD?\n\r", ch);
  } break;
  case POSITION_MOUNTED: {
    send_to_char("Not while riding you don't!\n\r", ch);
    break;
  }
    default : {
      act("You stop floating around, and lie down to sleep.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.",
	  TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
  }
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tmp_char;
  char arg[MAX_STRING_LENGTH];
  
  
  one_argument(argument,arg);
  if (*arg) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!",
	  FALSE, ch,0,0,TO_CHAR);
    } else {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char) {
	if (tmp_char == ch) {
	  act("If you want to wake yourself up, just type 'wake'",
	      FALSE, ch,0,0,TO_CHAR);
	} else {
	  if (GET_POS(tmp_char) == POSITION_SLEEPING) {
	    if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
	      act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
	    } else {
	      act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
	      GET_POS(tmp_char) = POSITION_SITTING;
	      act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
	    }
	  } else {
	    act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR);
	  }
	}
      } else {
	send_to_char("You do not see that person here.\n\r", ch);
      }
    }
  } else {
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);
    } else {
      if (GET_POS(ch) > POSITION_SLEEPING)
	send_to_char("You are already awake...\n\r", ch);
      else {
	send_to_char("You wake, and sit up.\n\r", ch);
	act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POSITION_SITTING;
      }
    }
  }
}


void do_follow(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH];
  struct char_data *leader;
  int shadow = 0;
  
  only_argument(argument, name);
  
  if (*name) {
    if (!(leader = get_char_room_vis(ch, name))) {
      send_to_char("I see no person by that name here!\n\r", ch);
      return;
    }
  } else {
    send_to_char("Who do you wish to follow?\n\r", ch);
    return;
  }
  
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
    
    act("But you only feel like following $N!",
	FALSE, ch, 0, ch->master, TO_CHAR);
    
  } else if (leader == ch) {
    if (!ch->master) {
      send_to_char("You are already following yourself.\n\r", ch);
      return;
    }
    stop_follower(ch);
  } else {
    if(!CountFollowers(leader)) {
      send_to_char("That person already has way too many followers.\n\r",
		   ch);
      return;
    }
    if (circle_follow(ch, leader)) {
      send_to_char("Sorry, but following in 'loops' is not allowed", ch);
      return;
    }

    if(!CanFollow(ch, leader))
	return;
    
    if (ch->master)
      stop_follower(ch);
      
    if (cmd == 447)
	shadow = 1;
    add_follower(ch, leader, shadow);
  }
}

int CanFollow(struct char_data* ch, struct char_data* leader)
{
    if(IS_IMMORTAL(ch)) 	/* gods can follow anyone */
	return 1;
    
    if (!IS_PC(leader))
    {
	send_to_char("You may only follow other players!!\n\r", ch);
	return 0;
    }
#ifdef JANWORK   
    cl = get_average_level(ch);
    ll = get_average_level(leader);
    if(cl < MAX_FREE_GROUP)	/* low levels can follow anybody */
	return 1;
    if(cl > (ll + MAX_GROUP_RANGE))
    {
	send_to_char("You don't wanna follow that wimp.\n\r", ch);
	return 0;
    }
    if(cl < (ll - MAX_GROUP_RANGE))
    {
	send_to_char("They probably go to areas that would get you killed.\n\r", ch);
	return 0;
    }
#endif   
    return 1;
}

void do_lose(struct char_data *ch, char *arg, int cmd) {
   struct char_data *k;
   struct follow_type *f, *next;
   
   while(*arg == ' ') arg++;
   
   if(!*arg) {
      cprintf(ch, "Syntax is \"lose <[name] | all>\"\n\r");
      return;
   }
   
   if(!strcmp(arg, "all")) {
      for(f=ch->followers;f;f=next) {
	 next=f->next;
	 
	 stop_follower(f->follower);
      }
      
      return;
   }
   
   if(!(k=get_char_room_vis(ch, arg))) {
      cprintf(ch, "You can't find that person here!\n\r");
      return;
   }
   
   if(k->master != ch) {
      cprintf(ch, "%s hasn't following you.\n\r", HSSH(k));
      return;
   }
   
   stop_follower(k);
}
