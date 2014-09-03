#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif

#include <math.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "race.h"
#include "constants.h"
#include "multiclass.h"
#include "util_str.h"
#include "utility.h"
#include "hero.h"

char *ClassNames(struct char_data *ch, char* buf)
{
    int i, count=0;
  
    for (i = 0; i <= MAX_LEVEL_IND; i++) {
	if (GET_LEVEL(ch, i)) {
	    count++;
	    if (count > 1) {
		sprintf(buf + strlen(buf), "/%s",pc_class_types[i]);
	    } else
	    {
		sprintf(buf, "%s", pc_class_types[i]);
	    }
	}
    }
    return(buf);
}

struct hero_info heroes[MAX_HEROES];

int herolist_changed=1;

// Measure this char on the hero scale and compare to others
// First give points to this hero
// Things that influence hero status:
// - Level
// - Alignment
// - Charisma
// - hero points
// - Gold
int assign_hero_status(struct char_data *ch) {
  int ret_val = 0;
  int j, max_level;
  int gold = 0;
  char buf[255];
  struct tm *timenow, *timelogin;
  int days_without_login = 0;

  // level
  // Find max level
  max_level = GetMaxLevel(ch);
  ret_val += (int)(max_level / 8);
  //  sprintf(buf, "Hero points after level = %d", ret_val);
  //  log_msg(buf);
  
  // Alignment
  ret_val += (int)(GET_ALIGNMENT(ch) / 100);
  //  sprintf(buf, "Hero points after alignment = %d", ret_val);
  //  log_msg(buf);

  // Charisma
  ret_val += (int)(ch->abilities.cha / 3);
  //  sprintf(buf, "Hero points after cha = %d", ret_val);
  //  log_msg(buf);

  // Gold
  gold = MAX( 1, (int)ch->points.gold + (int)ch->points.bankgold);
  if (gold < 600000000) {
    ret_val += gold / 200000000;
  } else { // Players should not gain a large amount of points based solely on their money
    ret_val += 2 + gold / 500000000;
  }
  //  sprintf(buf, "Hero points after gold = %d", ret_val);
  //  log_msg(buf);

  // Additional hero points awarded through vote system, if any
  ret_val += ch->points.hero_points;
  //  sprintf(buf, "Hero points after rewards = %d", ret_val);
  //  log_msg(buf);

  // Subtract points for not being logged on for a looong time
  // This is intended to phase out old heroes slowly to make room
  // for the new heroes of TW -- raist
  int days_off = (time(0) - real_character(ch)->player.time.logon) / (60*60*24);
  ret_val -= days_off / 45;
  //  sprintf(buf, "Hero points after days off = %d", ret_val);
  //  log_msg(buf);

  //  sprintf(buf, " points = %i", ret_val);
  //  log_msg(buf);
  return (int)ret_val;
}

// Modified by raist 2005 (no rights reserved) to provide more advanced algorithm for defining a hero
void add_char_to_hero_list(struct char_data *ch)
{
    int max_level = GetMaxLevel(ch);
    long exp;
    int i = MAX_HEROES, j, match = MAX_HEROES;
    char title[MAX_STRING_LENGTH];
    int points = 0;
    char buf[255];

    if (IS_IMMORTAL(ch) || max_level > 125)  /* Don't add immortals to list */
      return;

    exp = GET_EXP(ch);
    
    points = assign_hero_status(ch);
    
    for(j = 0; (j < MAX_HEROES);j++)
      {
	// Check if char is already a hero
	if ( heroes[j].name && str_cmp(GET_REAL_NAME(ch), ss_data(heroes[j].name)) == 0)
	  match = j;
	
	// Compare level
	if ((heroes[j].points < points) && (i == MAX_HEROES) )
	  i = j;
	/* 	else */
	/* 	  // If level was the same check exp */
	/* 	  if ((heroes[j].level == max_level) && (i==MAX_HEROES) ) */
	/* 	    if ( (heroes[j].exp<=exp) && (i==MAX_HEROES) ) */
	/* 	      i = j; */
      }
    
    // Dude was already on list, just update char info and position
    if(match < MAX_HEROES)
    {
	herolist_changed=1;

	/*
	 *  If match = insertion point
	 *   just change the exp.
	 */
	if (i == match)
	{
	    heroes[match].exp = exp;
	    heroes[match].level = max_level;
	    heroes[match].points = points;
	    return;
	}

	/*
	 * else insertion is diff from match, so clear out the match
	 * position
	 */
	else
	{
	    ss_free(heroes[match].name);
	    FREE(heroes[match].titles);
	    FREE(heroes[match].race);
	    for(j=match+1;j<MAX_HEROES;j++)
	    {
		heroes[j-1].name = heroes[j].name;
		heroes[j-1].titles = heroes[j].titles;
		heroes[j-1].clss = heroes[j].clss;
		heroes[j-1].race = heroes[j].race;
		heroes[j-1].exp = heroes[j].exp;
		heroes[j-1].logon = heroes[j].logon;
		heroes[j-1].level = heroes[j].level;
		heroes[j-1].points = heroes[j].points;
	    }
	    heroes[MAX_HEROES-1].name = NULL;
	    heroes[MAX_HEROES-1].titles = NULL;
	    heroes[MAX_HEROES-1].race = NULL;
	    heroes[MAX_HEROES-1].exp = 0;
	    heroes[MAX_HEROES-1].clss = 0;
	    heroes[MAX_HEROES-1].logon = 0;
	    heroes[MAX_HEROES-1].level = 0;
	    heroes[MAX_HEROES-1].points = 0;
	}
    }

    /*
     * if insertion < MAX_HEROES then insert the dude into the list
     */
    if (i<MAX_HEROES)
    {
	herolist_changed=1;

	if(match<i)
	    i--;
	ss_free(heroes[MAX_HEROES-1].name);
	if(heroes[MAX_HEROES-1].titles)
	    FREE(heroes[MAX_HEROES-1].titles);
	if(heroes[MAX_HEROES-1].race)
	    FREE(heroes[MAX_HEROES-1].race);
	for(j=MAX_HEROES-1;j>=i;j--)
	{
	    heroes[j].name = heroes[j-1].name;
	    heroes[j].titles = heroes[j-1].titles;
	    heroes[j].clss = heroes[j-1].clss;
	    heroes[j].exp = heroes[j-1].exp;
	    heroes[j].race = heroes[j-1].race;
	    heroes[j].logon = heroes[j-1].logon;
	    heroes[j].level = heroes[j-1].level;
	    heroes[j].points = heroes[j-1].points;
	}

	heroes[i].exp = exp;
	heroes[i].name = ss_share(real_character(ch)->player.name);
	heroes[i].clss = GET_CLASS(ch);
	heroes[i].race = strdup(RaceName[ch->race]);
	heroes[i].titles = strdup(ClassNames(ch,title));
	heroes[i].logon = ch->player.time.logon;
	heroes[i].level = max_level;
	heroes[i].points = points;
    }
}

int is_top_hero(struct char_data *ch)
{
  if (heroes[0].name &&
      (str_cmp(ss_data(heroes[0].name),GET_REAL_NAME(ch))==0))
    return TRUE;
  return FALSE;
}

int is_hero(struct char_data *ch)
{
  int i;

  for(i=0; i<MAX_HEROES; i++)
    if (heroes[i].name &&
	(str_cmp(ss_data(heroes[i].name),GET_REAL_NAME(ch))==0))
      return (i+1);
  return FALSE;
}










