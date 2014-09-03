/*
**  Levels:  int levels[8]
*/

/*
**  0 = Mage, 1 = cleric, 3 = thief, 2 = fighter
**  4 = paladin, 5 = druid, 6 = psionist, 7 = ranger, 8 - shifter,  9 = bard,
**  10 = monk 
*/

/*
**  
*/

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
#include "limits.h"
#include "opinion.h"
#include "race.h"
#include "multiclass.h"
#include "utility.h"
#include "db.h"

/* forward declarations */
int GetALevel(struct char_data *ch, int which);


int GetClassLevel(struct char_data *ch, int clss)
{

  if (IS_SET(ch->player.clss, clss)) {
    return(GET_LEVEL(ch, CountBits(clss)-1));
  }
  return(0);
}

int CountBits(int clss)
{
    char buf[256];
    
    if (clss == 1) return(1);
    if (clss == 2) return(2);
    if (clss == 4) return(3);
    if (clss == 8) return(4);
    if (clss == 16) return(5);
    if (clss == 32) return(6);
    if (clss == 64) return(7);
    if (clss == 128) return(8);
    if (clss == 256) return(9);
    if (clss == 512) return(10);
    if (clss == 1024) return(11);

    sprintf(buf, "CountBits: Bad Class: %d", clss);
    log_msg(buf);
    return -1;
}

int OnlyClass( struct char_data *ch, int clss)
{
  int i;

  for (i=1;i<=256; i*=2) {
    if (GetClassLevel(ch, i) != 0)
      if (i != clss)
	return(FALSE);
  }
  return(TRUE);

}

int HasClass(struct char_data *ch, int clss)
{

  if (IS_SET(clss, ch->player.clss))
     return(TRUE);

  return FALSE;
}

int HowManyClasses(struct char_data *ch)
{
    short i, tot=0;

    for (i=0;i<=MAX_LEVEL_IND;i++) {
	if (GET_LEVEL(ch, i)) {
	    tot++;
	}
    }
    if (tot) 
	return(tot);

    if (IS_SET(ch->player.clss, CLASS_MAGIC_USER)) 
	tot++;

    if (IS_SET(ch->player.clss, CLASS_PALADIN))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_DRUID))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_PSI))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_RANGER))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_SHIFTER))
        tot++;

    if (IS_SET(ch->player.clss, CLASS_WARRIOR)) 
	tot++;
      
    if (IS_SET(ch->player.clss, CLASS_THIEF))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_CLERIC))
	tot++;

    if (IS_SET(ch->player.clss, CLASS_BARD))
        tot++;

    if (IS_SET(ch->player.clss, CLASS_MONK))
        tot++;

    return tot;
}


int BestFightingClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
   return(PALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND))
  return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, BARD_LEVEL_IND))
  return(BARD_LEVEL_IND);
 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, SHIFTER_LEVEL_IND))
   return (SHIFTER_LEVEL_IND);
 if (GET_LEVEL(ch, PSI_LEVEL_IND))
  return(PSI_LEVEL_IND);
 if (GET_LEVEL(ch, DRUID_LEVEL_IND))
  return(DRUID_LEVEL_IND);

 
  log_msg("Massive error.. character has no recognized class.");
  log_msg(GET_NAME(ch));


  return(1);
}

int BestThiefClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND))
  return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, BARD_LEVEL_IND))
   return(BARD_LEVEL_IND);
 if (GET_LEVEL(ch, PSI_LEVEL_IND))
   return(PSI_LEVEL_IND);
 if (GET_LEVEL(ch, DRUID_LEVEL_IND))
   return(DRUID_LEVEL_IND);
 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, SHIFTER_LEVEL_IND))
   return (SHIFTER_LEVEL_IND);
 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
   return(PALADIN_LEVEL_IND);
 
  log_msg("Massive error.. character has no recognized class.");
  log_msg(GET_NAME(ch));


  return(1);
}

int BestMagicClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, DRUID_LEVEL_IND))
   return(DRUID_LEVEL_IND);
 if (GET_LEVEL(ch, PSI_LEVEL_IND))
   return(PSI_LEVEL_IND);
 if (GET_LEVEL(ch, BARD_LEVEL_IND))
   return(BARD_LEVEL_IND);
 if (GET_LEVEL(ch, SHIFTER_LEVEL_IND))
   return (SHIFTER_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND))
   return(PALADIN_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND))
   return(RANGER_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND))
   return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 
  log_msg("Massive error.. character has no recognized class.");
  log_msg(GET_NAME(ch));


  return(1);
}

int GetSecMaxLev(struct char_data *ch)
{
   return(GetALevel(ch, 2));
}

int GetALevel(struct char_data *ch, int which)
{

	// jan-debug
	char buf[256];
    ubyte ind[MAX_LEVEL_IND];
    int j, k, i;

    for (i=0; i<= MAX_LEVEL_IND; i++) {
	ind[i] = GET_LEVEL(ch,i);
	sprintf(buf,"DEBUG:: GetALevel(multiclass.c): name: [%s], which=%d, i=%d, level=%d\n",GET_NAME(ch),which, i, ind[i]);
	slog(buf);
    }

    /*
     *  chintzy sort. (just to prove that I did learn something in college)
     */

    for (i = 0; i<= (MAX_LEVEL_IND-1); i++) {
	for (j=i+1;j<=MAX_LEVEL_IND;j++) {
	    if (ind[j] > ind[i]) {
		k = ind[i];
		ind[i] = ind[j];
		ind[j] = k;
	    }
	}
    }

    if (which > -1 && which <= MAX_LEVEL_IND) {
	return(ind[which]);
    }

    return 0;
}

int GetThirdMaxLev(struct char_data *ch)
{
   return(GetALevel(ch, 3));
}

int GetMaxLevel(struct char_data *ch)
{
/*  register int max=0, i;

  for (i=0; i<= MAX_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch,i);
  }

  return(max);*/
  return((int)ch->player.maxlevel);
  
}

int UpdateMaxLevel(struct char_data *ch)
{
  register int max=0, i;

  for (i=0; i<= MAX_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch,i);
  }
  ch->player.maxlevel=max;
  return FALSE;
}



/* PAC -- note, the old GetMinLevel would return a value no larger than 50
   If we find that something depends on that functionality, sorry, oops, etc */
int UpdateMinLevel(struct char_data *ch)
{
  register int min=0, i;

   min = GetMaxLevel(ch);

  for (i=0; i<= MAX_LEVEL_IND; i++) {
    if ((GET_LEVEL(ch, i) < min) && (GET_LEVEL(ch,i) > 0))
      min = GET_LEVEL(ch,i);
  }
  ch->player.minlevel=min;
  return FALSE;
}
  
int GetMinLevel(struct char_data *ch)
{
/*  register int min=50, i;

  for (i=0; i<= MAX_LEVEL_IND; i++) {
    if ((GET_LEVEL(ch, i) < min) && (GET_LEVEL(ch,i) >0))
      min = GET_LEVEL(ch,i);
  }

  return(min);*/
  UpdateMinLevel(ch);
  return((int)ch->player.minlevel);
}

int GetTotLevel(struct char_data *ch)
{
  int x, tot=0;

  for (x=0; x<=MAX_LEVEL_IND; x++)
    tot+=GET_LEVEL(ch, x);

  return (tot);
}

int GetAvgLevel(struct char_data *ch)
{
  if (HowManyClasses(ch)==0)
    return 0;
  else
    return (GetTotLevel(ch)/HowManyClasses(ch));
}

void StartLevels(struct char_data *ch)
{

  if (IS_SET(ch->player.clss, CLASS_MAGIC_USER)) {
    advance_level(ch, MAGE_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_CLERIC)) {
    advance_level(ch, CLERIC_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_WARRIOR)) {
    advance_level(ch, WARRIOR_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_THIEF)) {
    advance_level(ch, THIEF_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_PALADIN)) {
    advance_level(ch, PALADIN_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_DRUID)) {
    advance_level(ch, DRUID_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_PSI)) {
    advance_level(ch, PSI_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_RANGER)) {
    advance_level(ch, RANGER_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_SHIFTER)) {
    advance_level(ch, SHIFTER_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_MONK)) {
    advance_level(ch, MONK_LEVEL_IND);
  }
  if (IS_SET(ch->player.clss, CLASS_BARD)) {
    advance_level(ch, BARD_LEVEL_IND);
  }
}


int BestClass(struct char_data *ch)
{

  int max=0, clss=0, i;

  for (i=0; i<= MAX_LEVEL_IND; i++)
    if (max < GET_LEVEL(ch,i)) {
      max = GET_LEVEL(ch, i);
      clss = i;
    }

  if (max == 0) /* eek */
    abort();

  return(clss);

}
