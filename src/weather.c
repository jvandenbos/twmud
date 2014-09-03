#include "config.h"
#include <stdio.h>
#include <string.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "weather.h"
#include "utility.h"
#include "multiclass.h"
#include "fight.h"
#include "comm.h"
#include "interpreter.h"
#include "proto.h"
#include "trap.h"
#include "mobprog2.h"

extern struct room_data *world;
void die(struct char_data * ch);

/* 3144 was 3685 */
/* forward declarations */
void another_hour(int mode);
void weather_change(void);
void switch_light(byte why);
static void GetMonth( int month);
void ChangeWeather( int change);

/* what stage is moon in?  (1 - 32) */
unsigned char moontype;

void weather_and_time(int mode)
{
	another_hour(mode);
	if(mode)
		weather_change();
}

static struct char_data *last_char_hit=NULL;

void beware_lightning(void)
{
  int dam = 0;
  struct char_data *victim = NULL;
  int iter_count;
  int hit_someone;
  char buf[256];

  if (!(weather_info.sky == SKY_LIGHTNING))
    return;   /*   go away if its not even stormy!   */

  if (number(0, 1000) < 990)  /* chance of it not hitting anyone */
    return;

  if (number(0, 1000) < 990) {  /* chance it doesn't hit anyone specific */
    send_to_outdoor("You hear the clap of distant thunder.\n\r");
    return;
  }

   hit_someone=FALSE;
   iter_count=0;
   while(!hit_someone) {
      iter_count++;
      hit_someone = TRUE;

      if(iter_count > 5000) {
	 log_msg("BUG - over 5000 iterations of beware_lightning. Exiting function.");
	 break;
      }
      EACH_CHARACTER(iter, victim) {
         if (OUTSIDE(victim) && !IS_NPC(victim) &&
 	     (real_roomp(victim->in_room)->sector_type != SECT_INSIDE) &&
	     !check_peaceful(victim, "") && (victim != last_char_hit)) {
	    hit_someone = FALSE;

	    if(number(0,5*connected) == 0) {
	       hit_someone = TRUE;
	       dam = number(1, GET_MAX_HIT(victim))/2;

	       if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
	          dam /= 2;
	       }

	       if (IsImmune(victim, IMM_ELEC)) {
		  dam /= 4;
	       } else if (IsResist(victim, IMM_ELEC)) {
		  dam /= 2;
	       }

	       dam = MAX(dam, 0);    /* no less than 0 hp per round */

	       if (!victim->desc)
		 continue;		// if linkdead fall through

               if (IS_IMMORTAL(victim))
   	         continue;		// fall through if immortal

               if (GetMaxLevel(victim) <= 30)
                 continue;		// fall through if < lvl 30

	       if (HasClass(victim, CLASS_DRUID))
	         continue;		// Druids own nature...they won't get hit

               GET_HIT(victim) -= dam;
	       last_char_hit = victim;

               act("$CwKAZAK! $CNa lightning bolt hits $n.  You hear a sick sizzle.",
                   TRUE, victim, 0, 0, TO_ROOM);

               act("$CwKAZAK! $CNa lightning bolt hits you.  You hear a sick sizzle.",
                   FALSE, victim, 0, 0, TO_CHAR);

               if (dam > (GET_MAX_HIT(victim) >> 2))
                   act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

	       send_to_outdoor("*BOOM* The ground rumbles as lightning strikes nearby.\n\r");

               update_pos(victim);
               if(GET_POS(victim) == POSITION_DEAD) {
	          act("$n is dead! $CGR.I.P.$CN", TRUE, victim, 0, 0, TO_ROOM);
                  act("$CrYou are dead!  Sorry...$CN", FALSE, victim, 0, 0, TO_CHAR);

	          if (GET_POS(victim) == POSITION_DEAD) {
	  	     sprintf(buf, "Thunderstorm killed %s", GET_NAME(victim));
                     log_msg(buf);
                     sprintf(buf,"$Cr\n\r   ### %s was just electrocuted to death by lightning!$CN\n\r", GET_NAME(victim));

		     //pkill status is to ensure they go to morgue
		     real_character(victim)->player.pkillinfo.status = 1;
                     die(victim);
                     send_to_all_formatted(buf);
                  }
  	       }
	       break;	//only 1 lighting per storm-strike
 	    }	//chance
         }  /* OUTSIDE */
      }
      END_AITER(iter);
   }	/* while loop */
}


void another_hour(int mode)
{

    char moon[20];
    int brett=0;
    int pirateroom=0;
    struct char_data* mob;
    /* Variables for Tornado */
    int to_room;
    struct room_data *room;
    char buf[MAX_INPUT_LENGTH];

    time_info.hours++;

    if (mode) {
	switch (time_info.hours) {
        case 1 :
            /* mob in this case is for chars */
            if((number(1,1000)==359) || DISASTER==5) {
                slog("begin Dark Cloud effect");
                send_to_all("A dark cloud decends from the heavens above...\n\r");
                send_to_outdoor("You are enveloped in a dark cloud!\n\r");
                send_to_indoor("You hear the cries of people caught in the cloud!\n\r");
                DISASTER=0;
                DISASTERNUM++;
                EACH_CHARACTER(iter, mob)
                {
	            if(number(1, 1000) != 727)
			    continue;

                    if(mob->desc && OUTSIDE(mob))
                    {
                        if(GetMaxLevel(mob) <= 15)
                        {
                           send_to_char("Someone inhales the dark cloud to save your precious mana.\n\r", mob);
                        }
                        else if(!IS_IMMORTAL(mob))
                        {
                            GET_MANA(mob)-=(number(100,300));
                            update_pos(mob);

                        }
                    }
                }
                END_AITER(iter);
            }
            break;

        case 2 :
            /* mob in this case is for chars */
            if((number(1,1000)==50) || DISASTER==2) {
                slog("begin Volcano effect");
                send_to_all("A volcano erupts from hell below...\n\r");
                send_to_outdoor("You are belted with debis from a volcanoe!\n\r");
                send_to_indoor("You hear the cries of people caught by the volcano!\n\r");
                DISASTER=0;
                DISASTERNUM++;
                EACH_CHARACTER(iter, mob)
                {
	            if(number(1,1000) != 75)
			    continue;
                    if(mob->desc && OUTSIDE(mob))
                    {
                        if(GetMaxLevel(mob) <= 15)
                        {
                         send_to_char("Someone diverts the lava flow.\n\r", mob);
                        }
                        else if(!IS_IMMORTAL(mob))
                        {
                            GET_HIT(mob)-=(number(50,200));
                            update_pos(mob);
                            if(GET_POS(mob)==POSITION_DEAD) {
                                sprintf(buf,"$CR### %s was just killed by the volcano!$CN\n\r", GET_NAME(mob));
                                die(mob);
                                send_to_all_formatted(buf);
                           }
                        }
                    }
                }
                END_AITER(iter);
            }
            break;



	case 3:
	    send_to_outdoor("The moon sets behind some distant clouds.\n\r");
	    if((moontype > 15) && (moontype < 25))
		switch_light(MOON_SET);
	    GrowPlants(0);
	    break;
	case 4 :
	    brett=number(1,50);
	    if(brett==5 || PIRATEQST==1) {
		pirateroom=525;
		PIRATEQST=1;
	    }
	    else if (brett==25 || PIRATEQST==2) {
		pirateroom=527;
		PIRATEQST=2;
	    }
	    else if (brett==50 || PIRATEQST==3) {
		pirateroom=529;
		PIRATEQST=3;
	    }
	    if(pirateroom==525 || pirateroom==527 ||
	       pirateroom==529) {
		if(real_roomp(pirateroom) != NULL &&
		   (real_roomp(3144) != NULL) && (PIRATE==0)) {
		    slog("Making pirate ship city entrance.");
		    CREATE(real_roomp(3144)->dir_option[1],
			   struct room_direction_data, 1);
		    real_roomp(3144)->dir_option[1]->exit_info=0;
		    real_roomp(3144)->dir_option[1]->key=-1;
		    real_roomp(3144)->dir_option[1]->to_room=pirateroom;
		    mob = make_mobile(529, VIRTUAL);
		    char_to_room(mob, 3144);
		    mob = make_mobile(529, VIRTUAL);
		    char_to_room(mob, 3144);
		    mob = make_mobile(529, VIRTUAL);
		    char_to_room(mob, 3144);
		    if(PIRATEQST==1 || PIRATEQST==3) {
			mob = make_mobile(529,VIRTUAL);
			char_to_room(mob, 3144);
		    }
		    if(PIRATEQST==1) {
			mob = make_mobile(529,VIRTUAL);
			char_to_room(mob, 3144);
		    }
		    mob = make_mobile(529, VIRTUAL);
		    char_to_room(mob, 3144);
		    mob = make_mobile(529, VIRTUAL);
		    char_to_room(mob, 3144);
		    PIRATE=1;
		    if(number(1,2)==1)
			send_to_all("The Town Crier shouts 'Oh no! The pirates have docked to raid all of Sanctuary!'\n\r");
		    else
			send_to_all("The Town Crier shouts 'Alert!  A pirate ship has been spotted at the pier!!'\n\r");
		}
	    }
	    break;
	case 5 :
	    weather_info.sunlight = SUN_RISE;
	    send_to_outdoor("The sun rises in the eastern horizon.\n\r");
	    break;
	case 6 :
	    if(PIRATE==1) {
		if(number(1,2)==1) {
		    send_to_all("The Mayor shouts 'Please help us defeat the pirates at the piers!'\n\r");
		    send_to_all("The Mayor shouts 'Save the city, citizens of Sanctuary!!'\n\r");
		} else {
		    send_to_all("The Mayor shouts 'These pirates are mean and nasty!'\n\r");
		    send_to_all("The Mayor shouts 'Citizens of Sanctuary!  We must defeat these wretched pirates!'\n\r");
		}
	    }
	    break;
	case 7 :
	    weather_info.sunlight = SUN_LIGHT;
	    switch_light(SUN_LIGHT);
	    send_to_outdoor("A new day of adventure has begun.\n\r");
	    break;
	case 8 :
	    if(PIRATE==1) {
		if(number(1,2)==1)
		    send_to_all("The Town Crier shouts 'Alert!  Alert!  The pirates at the pier are going to loot the city!'\n\r");
		else
		    send_to_all("The Town Crier shouts 'Help us defeat the pirates at the pier!!'\n\r");
	    }
	    break;
	case 9 :
	    if (number(1,7)==1) {
send_to_all_formatted(" $CgThe Casino owner$CN shouts, 'Come try the CASINO!'\n\r");
		CASINO_BANK += 150000;
	    }
	    break;
	case 10 :
	    if (number(1,1000)>989) {
		send_to_all_formatted("$CrYou feel the earth rumble!$CN\n\r");
	    }
	    break;
	case 11 :
	    /* mob in this case is for chars */
	    if((number(1,1000)==924) || DISASTER==1) {
		slog("begin Earthquake");
		send_to_all_formatted("$CyThe earth starts to shake and rumble...$CN\n\r");
		send_to_outdoor_formatted("$CrYou are hit by falling debris from above!$CN\n\r");
		send_to_indoor_formatted("$CrYou hear the crash of falling debris!$CN\n\r");
		DISASTER=0;
		DISASTERNUM++;

		EACH_CHARACTER(iter, mob)
		{
		    if(number(1, 1000) != 419)
			    continue;
		    if(mob->desc && OUTSIDE(mob))
		    {
			if(GetMaxLevel(mob) <= 15)
			{
			    send_to_char("God pulls the heavy debris off your body, and heals your wounds!\n\r", mob);
			}
			else if(!IS_IMMORTAL(mob))
			{
			    GET_HIT(mob)-=(number(10,45));
			    update_pos(mob);
			    if(GET_POS(mob)==POSITION_DEAD) {
				sprintf(buf,"$CR### %s was just killed in the earthquake!$CN\n\r", GET_NAME(mob));
				die(mob);
				send_to_all_formatted(buf);
			    }
			}
		    }
		}
		END_AITER(iter);
	    }
	    break;
	case 12 :
	    send_to_outdoor("As the sun peaks, you realize that it is noon.\n\r");
	    break;
	case 13 :
	    if(PIRATE==1)
		send_to_all("Save Sanctuary from the pirates that have landed at the end of the pier!\n\r");
	    break;
	case 14 :
	  	    if (number(1,40)<6)
		    send_to_all("READ THE TWSTORY!\n\r");
	    break;
	case 15 :
	    if(PIRATE==1) {
		if(number(1,2)==1)
		    send_to_all("The bank teller shouts 'Help! Help!  "
				"One of the pirates is trying to rob the bank!'\n\r");
		else
		    send_to_all("The bank teller shouts 'Argh! "
				"One of these pirates is trying to steal my gold!'\n\r");
		mob = make_mobile(529,VIRTUAL);
		char_to_room(mob, 3144);
		if(PIRATEQST==1 || PIRATEQST==3) {
		    mob=make_mobile(529,VIRTUAL);
		    char_to_room(mob, 3144);
		}
		if(PIRATEQST==1) {
		    mob=make_mobile(529,VIRTUAL);
		    char_to_room(mob, 3144);
		}
	    }
	    break;
	case 16 :
	    if (number(1,7)==2) {
send_to_all_formatted("$CgCasino owner$CN shouts, 'I stole gold!! Come and gamble yours'\n\r");
		CASINO_BANK += 136850;
	    }
	    break;
	case 17 :
	    if(PIRATE==1)
		if(number(1,2)==1)
		    send_to_all("The Mayor shouts 'Are the pirates still at the pier?!  HELP!!'\n\r");
		else
		    send_to_all("The Town Crier shouts 'The pirate ship is still docked at the pier!  Help!'\n\r");
	    break;
	case 18 :
	    weather_info.sunlight = SUN_SET;
	    send_to_outdoor(
			    "The sun slowly disappears behind the Dragon Spine mountains to the west.\n\r");
	    break;
	case 20 :
	    weather_info.sunlight = SUN_DARK;
	    switch_light(SUN_DARK);
	    send_to_outdoor("The night has begun, spreading darkness throughout the land.\n\r");
	    GrowPlants(0);
	    break;
	case 21 :
	    if(PIRATE==1) {
		if(number(1,2)==1) {
		    send_to_all("The Town Crier shouts 'The pirate ship has finally departed from the pier!'\n\r");
		    send_to_all("The Mayor shouts 'Thank the Heavens above!'\n\r");
		} else {
		    send_to_all("The Town Crier shouts 'The pirates have finally left our peaceful city!'\n\r");
		    send_to_all("The Mayor shouts 'Praise the citizens of Sanctuary!'\n\r");
		}
		slog("Deleting pirate ship entrance");
		FREE(real_roomp(3144)->dir_option[1]);
		real_roomp(3144)->dir_option[1]=0;
		PIRATE=0;
		PIRATEQST=0;
		PIRATENUM++;
	    }
	    break;

        case 22 :
            if (moontype < 4) {
                strcpy(moon, "new");
            } else if (moontype < 12) {
                strcpy(moon, "waxing");
            } else if (moontype < 20) {
                strcpy(moon, "full");
            } else  if (moontype < 28) {
                strcpy(moon, "waning");
            } else {
                strcpy(moon, "new");
            }
            sprintf(buf,"The %s moon rises in the eastern horizon.\n\r",moon);
            send_to_outdoor(buf);
            if((moontype > 14) && (moontype < 24))
                switch_light(MOON_RISE);
            break;

        case 23 :
            /* mob in this case is for chars */
            if((number(1,1000)==217) || DISASTER==3) {
                slog("begin Lightning Storm effect");
                send_to_all("A lightning storm decends from the heavens above...\n\r");
                send_to_outdoor("You are zapped by the forces of a lightning storm!\n\r");
                send_to_indoor("You hear the cries of people caught in the storm!\n\r");
                DISASTER=0;
                DISASTERNUM++;
                EACH_CHARACTER(iter, mob)
                {
	            if(number(1, 1000) != 745)
			    continue;
                    if(mob->desc && OUTSIDE(mob))
                    {
                        if(GetMaxLevel(mob) <= 15)
                        {
                         send_to_char("Someone diverts the lightning bolts.\n\r", mob);
                        }
                        else if(!IS_IMMORTAL(mob))
                        {
                            GET_HIT(mob)-=(number(50,150));
                            GET_MANA(mob)-=(number(50,150));
                            GET_MOVE(mob)-=(number(50,150));
                            update_pos(mob);
                            if(GET_POS(mob)==POSITION_DEAD) {
                                sprintf(buf,"$CR### %s was just zapped by lightning!$CN\n\r", GET_NAME(mob));
                                die(mob);
                                send_to_all_formatted(buf);
			   }
                        }
                    }
                }
                END_AITER(iter);
            }
            break;

        case 24 :
            /* mob in this case is for chars */
            if((number(1,1000)==514) || DISASTER==4) {
                slog("begin Tornado effect");
                send_to_all("A tornado decends from the heavens above...\n\r");
                send_to_outdoor("You are sucked up into the forces of a tornado!\n\r");
                send_to_indoor("You hear the cries of people caught in the tornado!\n\r");
                DISASTER=0;
                DISASTERNUM++;
                EACH_CHARACTER(iter, mob)
                {
	            if(number(1, 1000) != 549)
			    continue;
                    if(mob->desc && OUTSIDE(mob))
                    {
                        if(GetMaxLevel(mob) <= 15)
                        {
                         send_to_char("Someone keeps you rooted firmly to the ground.\n\r", mob);
                        }
                        else if(!IS_IMMORTAL(mob))
                        {
                            GET_MOVE(mob)-=(number(50,200));
			    update_pos(mob);

			    /* Lets move them to a room and try and make sure they don't die. */
			    do {
			      /*  Get a random room in the world that we are able to travel to */
			      to_room = number(0, top_of_world);
			      room = real_roomp(to_room);
			    } while (!room || IS_SET(room->room_flags, NO_TRAVEL_IN));

                            act("$n is sucked away by the tornado!", TRUE, mob, 0,0,TO_ROOM);
                            char_from_room(mob);
                            char_to_room(mob,to_room);

                            act("$n smashes to the ground.",TRUE,mob, 0,0,TO_ROOM);
                            do_look(mob,"",0);

                            if (IS_SET(real_roomp(to_room)->room_flags, DEATH))
                               do_death_trap(mob);

                        }
                    }
                }
                END_AITER(iter);
            }
            break;

	}
    }

    //Weather trigger (Quilan project)
    EACH_CHARACTER(iter, mob) {
       mprog_weather_trigger(mob, time_info.hours);
       mprog_weather_trigger2(mob, time_info.hours);
    } END_AITER(iter);

    if (time_info.hours > 23)	/* Changed by HHS due to bug ???*/
    {
	time_info.hours -= 24;
	time_info.day++;
	moontype++;
	if (moontype > 28)
	    moontype = 1;

	if (time_info.day>30)		{
	    time_info.day = 0;
	    time_info.month++;
	    GetMonth(time_info.month);

	    if(time_info.month>12)	       	{
		time_info.month = 0;
		time_info.year++;
	    }
	}
   }
}

void weather_change(void)
{
    int diff, change;

    if((time_info.month>=9)&&(time_info.month<=11))
	diff=(weather_info.pressure>985 ? -2 : 2);
    else
	diff=(weather_info.pressure>1015? -2 : 2);

    weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));

    weather_info.change = MIN(weather_info.change,12);
    weather_info.change = MAX(weather_info.change,-8);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure,1040);
    weather_info.pressure = MAX(weather_info.pressure,960);

    change = 0;

    switch(weather_info.sky){
    case SKY_CLOUDLESS :
	{
	    if (weather_info.pressure<990)
		change = 1;
	    else if (weather_info.pressure<1010)
		if(dice(1,4)==1)
		    change = 1;
	    break;
	}
    case SKY_CLOUDY :
	{
	    if (weather_info.pressure<970)
		change = 2;
	    else if (weather_info.pressure<990)
		if(dice(1,4)==1)
		    change = 2;
		else
		    change = 0;
	    else if (weather_info.pressure>1030)
		if(dice(1,4)==1)
		    change = 3;

	    break;
	}
    case SKY_RAINING :
	{
	    if (weather_info.pressure<970)
		if(dice(1,4)==1)
		    change = 4;
		else
		    change = 0;
	    else if (weather_info.pressure>1030)
		change = 5;
	    else if (weather_info.pressure>1010)
		if(dice(1,4)==1)
		    change = 5;

	    break;
	}
    case SKY_LIGHTNING :
	{
	    if (weather_info.pressure>1010)
		change = 6;
	    else if (weather_info.pressure>990)
		if(dice(1,4)==1)
		    change = 6;

	    break;
	}
	default :
	{
	    change = 0;
	    weather_info.sky=SKY_CLOUDLESS;
	    break;
	}
    }

    ChangeWeather(change);

}

void ChangeWeather( int change)
{
    switch(change){
    case 1 :
	send_to_outdoor("The sky is getting cloudy.\n\r");
	weather_info.sky=SKY_CLOUDY;
	break;
    case 2 :
	if ((time_info.month > 4) && (time_info.month < 12))
	    send_to_outdoor("It starts to rain.\n\r");
	else
	    send_to_outdoor("It starts to snow and soon a white blanket covers the earth.\n\r");
	weather_info.sky=SKY_RAINING;
	break;
    case 3 :
	send_to_outdoor("The clouds disappear and the sky becomes empty.\n\r");
	weather_info.sky=SKY_CLOUDLESS;
	break;
    case 4 :
	if ((time_info.month > 3) && (time_info.month <= 11))
	    send_to_outdoor("You are caught in a ferocious lightning storm.\n\r");
	else
	    send_to_outdoor("You feel extremely cold as you are caught in a blizzard.\n\r");
	weather_info.sky=SKY_LIGHTNING;
	break;
    case 5 :
	if ((time_info.month > 3) && (time_info.month < 11))
	    send_to_outdoor("The rain has stopped, leaving the ground wet and muddy.\n\r");
	else
	    send_to_outdoor("The snow has finally stopped.\n\r");
	weather_info.sky=SKY_CLOUDY;
	break;
    case 6 :
	if ((time_info.month > 3) && (time_info.month < 11))
	    send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
	else
	    send_to_outdoor("The blizzard is over, but it is still snowing.\n\r");
	weather_info.sky=SKY_RAINING;
	break;
    }
}

static void GetMonth( int month)
{
   if (month < 0)
      return;

   if (month <= 1)
       send_to_outdoor(" A biting wind blows\n\r");
   else if (month <=2)
       send_to_outdoor(" It is very cold \n\r");
   else if (month <=3)
       send_to_outdoor(" It is chilly outside \n\r");
   else if (month <= 4)
        send_to_outdoor(" It starts to get a little windy \n\r");
   else if (month <= 5)
        send_to_outdoor(" The flowers start to bloom \n\r");
   else if (month <= 6)
        send_to_outdoor(" It is warm and humid. \n\r");
   else if (month <= 9)
        send_to_outdoor(" The air is getting chilly \n\r");
   else if (month <= 10)
        send_to_outdoor(" The leaves start to change colors. \n\r");
   else if (month <= 11)
        send_to_outdoor(" It starts to get cold \n\r");
   else if (month <= 12)
        send_to_outdoor(" It is bitterly cold outside \n\r");

}


void switch_light(byte why)
{
    switch(why) {
    case MOON_SET:
/*	log_msg("Setting all rooms to dark."); */
	break;
    case SUN_LIGHT:
/*	log_msg("Setting all rooms to light."); */
	break;
    case SUN_DARK:
/*	log_msg("Setting all rooms to dark."); */
	break;
    case MOON_RISE:
/*	log_msg("Setting all non-forest to light."); */
	break;
    default:
	log_msg("Unknown switch on switch_light.");
	break;
    }

#if 1
/*    log_msg("Not really, switch light() is turned off "); */
#else

    register int i;
    register struct room_data *rp;

    for (i=0;i<=top_of_world;i++) {
	if ((rp = real_roomp(i))!= NULL) {
	    if (!IS_SET(rp->room_flags, INDOORS) && !IS_SET(rp->room_flags, DEATH)) {
		switch(why) {
		case MOON_SET:
		    rp->dark = 1;
		    break;
		case SUN_LIGHT:
		    rp->dark = 0;
		    break;
		case SUN_DARK:
		    rp->dark = 1;
		    break;
		case MOON_RISE:
		    if (rp->sector_type != SECT_FOREST)
			rp->dark = 0;
		    break;
		}
	    }
	}
    }
#endif
}

