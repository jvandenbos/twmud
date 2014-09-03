#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "race.h"
#include "opinion.h"
#include "hash.h"
#include "wizlist.h"
#include "utility.h"
#include "fight.h"
#include "act.h"
#include "spec.h"
#include "cmdtab.h"
#include "spelltab.h"
#include "modify.h"
#include "recept.h"
#include "weather.h"
#include "spell_util.h"
#include "multiclass.h"
#include "constants.h"
#include "board.h"
#include "util_str.h"
#include "statistic.h"
#include "db.zonehelp.h"
#include "proto.h"
#include "interpreter.h"
#include "mobprog2.h"
#include "trackchar.h"
#include "db.random.h"

#define NEW_ZONE_SYSTEM

void boot_auction(); /* prototype */
void boot_vote();

/*
 * Prototype for the load_banned function from ban.c
*/

void load_banned(void);

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */

char policy[MAX_STRING_LENGTH];		/* the policy			*/
char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH*4];	      /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char imotd[MAX_STRING_LENGTH];         /* the immortal messages of today */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char story[MAX_STRING_LENGTH*5];      /* the story text                  */
char greeting[MAX_STRING_LENGTH*8];	/* the greeting text		*/
char twstory[MAX_STRING_LENGTH*8];     /* tw roleplay story             */
int greet_no = 1;			/* the greeting number		*/
char newwizlist[MAX_STRING_LENGTH*2];   /* the list of wizards.	*/


#ifdef NEWHELP
struct help_index_element *help_index = 0;
FILE 	*genhelp_fl, *skillhelp_fl, *immhelp_fl, *combhelp_fl;
int top_of_genhelp, top_of_skillhelp, top_of_immhelp, top_of_combhelp;
#else
struct help_index_element *help_index = 0;
FILE     *help_fl;                   /* file for help texts (HELP <kwd>)*/
int top_of_helpt;                     /* top of help index table         */
#endif

int db_stubs = 0;
int no_specials = 0;    /* Suppress ass. of special routines */


struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */

CharTracker TrackingSystem;

#ifdef SWAP_ZONES
int swapped_zones=0;
int loaded_zones=0;
#endif

/* local procedures */
void reset_time(void);
void load_next_greet(void);

void boot_db(void)
{
    int i;

    event_init();

    log_msg("Boot db -- BEGIN.");

    log_msg("Resetting the game time:");
    reset_time();

    log_msg("Reading news, credits, help, info, policy, story motd and imotd.");
    file_to_string(NEWS_FILE, news, sizeof(news));
    file_to_string(POLICY_FILE, policy, sizeof(policy));
    file_to_string(CREDITS_FILE, credits, sizeof(credits));
    file_to_string(MOTD_FILE, motd, sizeof(motd));
    file_to_string(IMOTD_FILE, imotd, sizeof(imotd));
    file_to_string(HELP_PAGE_FILE, help, sizeof(help));
    file_to_string(INFO_FILE, info, sizeof(info));
    file_to_string(STORY_FILE, story, sizeof(story));
    file_to_string(TWSTORY_FILE, twstory, sizeof(twstory));

    log_msg("Booting Mobiles.");
    boot_mobiles(MOB_FILE);

    log_msg("Booting MobPrograms.");
    boot_mprog();

    log_msg("Booting Mobprograms2.");
    boot_mprog2(0);

    log_msg("Initializing random load table");
    setup_random_tree();

    log_msg("Booting Objects.");
    boot_objects(OBJ_FILE);

    log_msg("Booting Objprograms2.");
    boot_oprog2(0);

#ifndef HASH
    init_world();
    log_msg("Booting Players.");
    assign_max_race_attributes();
    if( LOAD_PLAYER_FILES ) {
       boot_players(PLAYER_DIR, READ_PLAYER | READ_OBJECTS | READ_DO_COUNT);
    } else {
       boot_players(PLAYER_DIR, READ_PLAYER);
    }
#endif

    log_msg("Loading Help Table.");
    build_help_index();

    log_msg("Loading zone table.");
    boot_zones(ZONE_FILE);

    log_msg("Booting zone programs");
    boot_zprog2(0);

    log_msg("Loading rooms.");
    boot_world(WORLD_FILE);

    log_msg("Booting RoomPrograms2");
    boot_rprog2(0);

#ifdef HASH
    log_msg("Booting Players.");
    assign_max_race_attributes();
    if( LOAD_PLAYER_FILES) {
    boot_players(PLAYER_DIR,READ_PLAYER);
    exit(0);
    }
    else
     boot_players(GOD_DIR,READ_PLAYER);
#endif

    log_msg("Renumbering zone table.");
    renum_zone_table();

    log_msg("Loading fight messages.");
    load_messages();

    log_msg("Loading social messages.");
    boot_social_messages();

    log_msg("Loading pose messages.");
    boot_pose_messages();

    log_msg("Loading zonehelp.");
    boot_zoneh();

    log_msg("Booting guilds.");
    // bootguilds();

    log_msg("Assigning function pointers (special procedures) :");
    if (!no_specials)	{
	log_msg("   Mobiles.");
	assign_mobiles();
	log_msg("   Objects.");
	assign_objects();
	log_msg("   Room.");
	assign_rooms();
    }

    log_msg("   Commands.");
    assign_command_pointers();

    log_msg("   Spells.");
    assign_spell_pointers();

   log_msg("   BANNED Sited List.");
   load_banned();

   log_msg("   Setting up for auction.");
   boot_auction();

   log_msg("   Setting up for vote.");
   boot_vote();

#ifdef SWAP_ZONES
    loaded_zones = swapped_zones=0;
#endif

    for (i = 0; i <= top_of_zone_table; i++)
    {
	char	*s;
	int	d,e;
	s = zone_table[i].name;
	d = (i ? (zone_table[i - 1].top + 1) : 0);
	e = zone_table[i].top;
#ifdef SWAP_ZONES
	/* if we can swap this zone, then don't boot it */
	if (zone_table[i].can_swap)
	{
	  fprintf(stderr, "Skipping boot-time reset of %s (rooms %d-%d).\n",
		  s, d, e);
	  /* Indicate that zone is swapped and prevent zone_update process */
	  zone_table[i].swapped=1;
	  zone_table[i].age=ZO_DEAD;
	  swapped_zones++;
	}
	else
	{
	  loaded_zones++;
#endif
	  fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
		  s, d, e);
	  reset_zone(i, 1);
#ifdef SWAP_ZONES
	  /* initialize loaded zones swapped value */
	  zone_table[i].swapped=0;
	}
#endif
    }

    log_msg("Loading saved corpse.");
    loadcorpse();

    log_msg("Loading the character tracker system.");
    TrackingSystem.Load();

    log_msg("Boot db -- DONE.");
    check_allocs();
}


/* reset the time in the game from file */
void reset_time(void)
{
    char buf[80];
    long beginning_of_time = 650336715;

    time_info = mud_time_passed(time(0), beginning_of_time);

    moontype = time_info.day;

    switch(time_info.hours){
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
	{
	    weather_info.sunlight = SUN_DARK;
	    break;
	}
    case 5 :
    case 6 :
	{
	    weather_info.sunlight = SUN_RISE;
	    break;
	}
    case 7 :
    case 8 :
    case 9 :
    case 10 :
    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 :
    case 16 :
    case 17 :
    case 18 :
	{
	    weather_info.sunlight = SUN_LIGHT;
	    break;
	}
    case 19 :
    case 20 :
	{
	    weather_info.sunlight = SUN_SET;
	    break;
	}
    case 21 :
    case 22 :
    case 23 :
	default :
	{
	    weather_info.sunlight = SUN_DARK;
	    break;
	}
    }

    sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
	    time_info.hours, time_info.day,
	    time_info.month, time_info.year);
    log_msg(buf);

    stat_log(buf,0);
    /* JON */

    weather_info.pressure = 960;
    if ((time_info.month>=7)&&(time_info.month<=10))
	weather_info.pressure += dice(1,50);
    else
	weather_info.pressure += dice(1,80);

    weather_info.change = 0;

    if (weather_info.pressure<=980) {
	if ((time_info.month>=3) && (time_info.month<=11))
	    weather_info.sky = SKY_LIGHTNING;
	else
	    weather_info.sky = SKY_LIGHTNING;
    } else if (weather_info.pressure<=1000) {
	if ((time_info.month>=3) && (time_info.month<=11))
	    weather_info.sky = SKY_RAINING;
	else
	    weather_info.sky = SKY_RAINING;
    } else if (weather_info.pressure<=1020) {
	weather_info.sky = SKY_CLOUDY;
    } else {
	weather_info.sky = SKY_CLOUDLESS;
    }
}

/* update the time file */
void update_time(void)
{
    FILE *f1;
    long current_time;

    return;

    if (!(f1 = fopen(TIME_FILE, "w")))
    {
	perror("update time");
	exit(0);
    }

    current_time = time(0);
    log_msg("Time update.");

    fprintf(f1, "#\n");

    fprintf(f1, "%ld\n", current_time);
    fprintf(f1, "%d\n", time_info.hours);
    fprintf(f1, "%d\n", time_info.day);
    fprintf(f1, "%d\n", time_info.month);
    fprintf(f1, "%d\n", time_info.year);

    fclose(f1);
}

void load_next_greet(void)
{
    FILE *fl;
    char fname[20];

    sprintf(fname, "%s%d", GREET_BASE, greet_no);
    if ((fl = fopen(fname, "r")))
    {
        fclose(fl);
        file_to_string(fname, greeting, sizeof(greeting));
    }
    else
    {
        greet_no = 1;
        sprintf(fname,"%s%d", GREET_BASE, greet_no);
        if (!file_to_string(fname, greeting, sizeof(greeting)))
        {
            strcpy(greeting, "");
        }
    }

    greet_no++;
}

/* code to init the auction variables */
void boot_auction() {

  CREATE(auction,AUCTION_DATA,1);
  if (auction == NULL) {
    log_msg("Unable to allocate memory for AUCTION record!!!");
    exit(1);
  }

  auction->item = NULL; /* nothing is being sold */
  auction->buyer=NULL;
  auction->seller=NULL;
}

/* code to init the hero voting variables */
void boot_vote() {

  CREATE(vote,VOTE_DATA,1);
  if (vote == NULL) {
    log_msg("Unable to allocate memory for VOTE record!!!");
    exit(1);
  }

  vote->total_votes = 0;
  vote->total_nominees = 0;
  vote->vote_start = FALSE;
}

