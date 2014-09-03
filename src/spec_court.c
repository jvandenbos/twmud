/*
 *  This is the implementation package for the Sequent Diku court system
 *  By:  Matt Ho & Maroo Lieuw
 *  Copyright (c) Matt Ho & Maroo Lieuw 1991.
 *  Revised for Forbidden-Lands: 6-3-94
 *
 */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include <unistd.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "constants.h"
#include "spec.h"
#include "interpreter.h"
#include "area.h"
#include "fight.h"
#include "spelltab.h"
#include "utility.h"
#include "spell_procs.h"
#include "util_str.h"
#include "spell_util.h"
#include "spec_court.h"
#include "vnum_mob.h"

#define ACCUSE	96
#define DATAPATH "/home_u/ucpflugf/mud/current/data"
#define CONFIRMED 0
#define UNCONFRIMED 1

exter char **court_files;

/* remove an unconfirmed killers name from list of unconfirms */

void remove_witness(char *name)
{
    FILE *fp, *tfp;                     /* orginal file and tmp file */
    char line[MAX_STRING_LENGTH];
    char witness[MAX_STRING_LENGTH], killer[MAX_STRING_LENGTH];

    if(!court_files[CONFIRMED])
    {
      CREATE(court_files[CONFIRMED], char, sizeof(char)*strlen(DATAPATH"/unconfirmed")+1);
    }
    
    sprintf(file, "%s/unconfirmed", DATAPATH);
    sprintf(newfile, "%s/unconfirmed.new", DATAPATH);

    /* open up the correct files */

    fp = fopen(file,"r");
    if (fp==NULL)
    {
        fprintf(stderr, "Failed to read unconfirmed file...\n");
        return;
    }

    tfp = fopen(newfile,"w");
    if (tfp==NULL)
    {
        fclose(fp);
        fprintf(stderr, "Failed to write unconfirmed.new file..\n");
        return;
    }

    /* strip the witnesses name away from witness list  */
    /* can a person be accused after death?             */
    /* this implements a no to the previous             */

    while(fgets(line,MAX_STRING_LENGTH,fp) != NULL)
    {
        sscanf(line,"%s %s\n",witness,killer);
        if(strcmp(witness,name) && strcmp(killer,name))
            fputs(line,tfp);
    }

    /* close files */

    fclose(fp);
    fclose(tfp);

    unlink(file);
    rename(newfile,file);

    return;
}

void remove_killer(char *name) {
    char filename[100];
    FILE *fp, *tfp;
    char tmpfile[100];
    char line[MAX_STRING_LENGTH];
    char witness[MAX_STRING_LENGTH], killer[MAX_STRING_LENGTH];

    sprintf(filename, "%s/unconfirmed", DATAPATH);
    sprintf(tmpfile, "%s/unconfirmed.new", DATAPATH);

    /* open working files */

    tfp = fopen(tmpfile,"w");
    if (tfp == NULL) {
	fprintf(stderr, "Failed to write unconfirmed.new...\n");
	return;
    }

    fp = fopen(filename,"r");
    if (fp == NULL) {
	fprintf(stderr, "Failed to read unconfirmed...\n");
	fclose(tfp);
	return;
    }

    /* strip killers name from original, storing to temp */

    while(fgets(line,MAX_STRING_LENGTH,fp) != NULL) {
        sscanf(line,"%s %s\n",witness,killer);
        if(strcmp(killer,name) != 0)
            fputs(line,tfp);
    }

    /* close working files */

    fclose(fp);
    fclose(tfp);

    unlink(filename);
    rename(tmpfile, filename);

    return;
}

void set_killer(struct char_data *ch) {
    ch->pflags |= PLR_PKILLER;
    SaveChar(ch,AUTO_RENT,FALSE);
}

/*
 * The following are the messages the Judge gvies in certain case 
 */

void reproachless_msg(struct char_data *ch, struct char_data *target) {
    char buf[MAX_STRING_LENGTH];

    act("The Judge considers your case ...\r", FALSE, ch,0,0,TO_CHAR);

    sprintf(buf,"The Judge considers the case presented by %s.\r",GET_NAME(ch));
    act(buf, 1, ch, 0, 0, TO_ROOM);

    sprintf(buf,"The Judge says '%s is beyond reproach!\r",GET_NAME(target));
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    act(buf, 1, ch, 0, 0, TO_ROOM);

    return;
}

void innocent_msg(struct char_data *ch, struct char_data *target) {
    char buf[MAX_STRING_LENGTH];

    act("The Judge considers your case ...\r", FALSE, ch,0,0,TO_CHAR);

    sprintf(buf,"The Judge considers the case presented by %s.\r",GET_NAME(ch));
    act(buf, 1, ch, 0, 0, TO_ROOM);

    sprintf(buf,"The Judge says 'I find in favor of %s, innocent of murder.\r",GET_NAME(target));
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    act(buf, 1, ch, 0, 0, TO_ROOM);

    return;
}

void guilty_msg(struct char_data *ch, struct char_data *target) {
    char buf[MAX_STRING_LENGTH];

    act("The Judge considers your case ...\r", FALSE, ch,0,0,TO_CHAR);

    sprintf(buf,"The Judge considers the case presented by %s.\r",GET_NAME(ch));
    act(buf, 1, ch, 0, 0, TO_ROOM);

    sprintf(buf,"The Judge says 'I find against %s, guilty of murder.\r",GET_NAME(target));
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    act(buf, 1, ch, 0, 0, TO_ROOM);

    return;
}

void convicted_msg(struct char_data *ch, struct char_data *target) {
    char buf[MAX_STRING_LENGTH];

    act("The Judge considers your case ...\r", FALSE, ch,0,0,TO_CHAR);

    sprintf(buf,"The Judge considers the case presented by %s.\r",GET_NAME(ch));
    act(buf, 1, ch, 0, 0, TO_ROOM);

    sprintf(buf,"The Judge says '%s has already been convicted of murder.\r",GET_NAME(target));
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    act(buf, 1, ch, 0, 0, TO_ROOM);

    return;
}

/* this nitty gritty */


SPECIAL(judge)
{
    FILE *fp;
    char filename[100];
    char buf[MAX_STRING_LENGTH];
    char witness[MAX_STRING_LENGTH], killer[MAX_STRING_LENGTH];
    struct char_data *target;
    int guilt;

    if (type == SPEC_INIT)
	return (FALSE);
  
    sprintf(filename, "%s/unconfirmed", DATAPATH);

    switch(cmd) {
	case ACCUSE :
	    one_argument(arg,buf);
	    if(!buf)		/* no argument given */
		send_to_char("Accuse who?\n\r",ch);
	    else if (!(target = get_char_vis(ch, buf)))  /* player cannot see the person he is accusing */
		send_to_char("You may not accuse a player not currently in the game ... yet.\n\r", ch);
	    else if(ch == target)	    /* player attemping to accuse himself */
		send_to_char("How about pleading the 5th instead?\n\r", ch);
	    else if (GET_POS(ch) != POSITION_STANDING)	/* must be stand to accuse */
		send_to_char("Show proper repect for the law!\n\r",ch);
	    else {				    /* cases to consider */
		if(GET_LEVEL(target) > MAX_MORT) {	    /* accused is a wizard */
		    reproachless_msg(ch,target);
		    return;
		}
		if(IS_SET(target->pflags,PLR_PKILLER)) {	/* accused is already a killer */
		    convicted_msg(ch,target);
		    remove_killer(GET_NAME(target));	/* remove his name just in case */
		    return;
		}
		else {					/* search through unconfirmed file for a match  */
		    guilt = 0;				/* else guilt = 0				*/
		    fp = fopen(filename,"r");
		    if (fp==NULL) {
			fprintf(stderr, "Couldn't open confirmed file...\n");
			return;
		    }

		    while(fgets(buf,MAX_STRING_LENGTH,fp) != NULL) {
			sscanf(buf,"%s %s\n",witness,killer);
			if(!strcmp(witness,GET_NAME(ch)) && !strcmp(killer,GET_NAME(target))) {
			    guilt = 1;
			    break;
			}
		    }
		    fclose(fp);
		    if(!guilt) {  /* if guilt == 0, accused is innocent */
			sprintf(buf, "%s falsely accuses %s.", 
			    GET_NAME(ch), GET_NAME(target));
			log(buf);
			innocent_msg(ch,target);
		    } else {	  /* else the accused must be guilty */
			sprintf(buf, "%s successfully accuses %s!",
			    GET_NAME(ch), GET_NAME(target));
			log(buf);
			set_killer(target);
			guilty_msg(ch,target);
			remove_killer(GET_NAME(target));
		    }
		}
	    }
	    break;
	default:
	    return(FALSE);
    }
    return(TRUE);
}


/* function for adding new killers to the unconfirmed killers list */

void add_witnesses(struct char_data *ch) {
    struct char_data *tmp, *tch;
    char filename[100] = "unconfirmed";
    char victim[MAX_STRING_LENGTH], killer[MAX_STRING_LENGTH];
    FILE *fp;
    int flag;	    /* flag to see if a guard has caught the killer in the act */
    int my_room;

    flag = 0;
    my_room = IS_NPC(ch) ? NOWHERE : ch->in_room;

    if(!IS_NPC(ch)) {
	for (tch=world[ch->in_room].people; tch && ch->in_room == my_room ; tch = tch->next_in_room) {
	    if (IS_MOB(tch)) {
                if(mob_index[tch->nr].virtual == VMOB_70) {
                    act("$n screams 'MURDURER!  JUSTICE WILL BE SERVED!!! ARARARAGGGHH!'", FALSE, tch, 0, 0, TO_ROOM);
                    hit(tch, ch, TYPE_UNDEFINED, 0);
		    flag = 1;
		}
	    }
	}
    }

    if(flag) {
	char buffer[MAX_STRING_LENGTH];

	remove_killer(GET_NAME(ch));
	set_killer(ch);
	sprintf(buffer, "%s caught by guards!", GET_NAME(ch));
	log(buffer);
	return;
    }

    fp = fopen(filename,"a");

    for (tmp = world[ch->in_room].people ; tmp ; tmp = tmp->next_in_room) {
	if(!IS_NPC(tmp) && strcmp(GET_NAME(ch),GET_NAME(tmp)) && (GET_LEVEL(tmp) < 21) && CAN_SEE(tmp,ch) &&
	   CAN_SEE_WIZ(tmp,ch) && GET_POS(tmp) > POSITION_SLEEPING)
	    fprintf(fp,"%s %s\n",GET_NAME(tmp),GET_NAME(ch));
    }

    fclose(fp);
    return;
}

void set_unconfirmed(struct char_data *ch, struct char_data *victim) 
{
    char buf[1024];

    if (victim && ch != victim) {
	if(IS_NPC(victim))
	    return;
	if (victim->specials.fighting) /* TC seg faulted here */
	    if(!str_cmp(GET_NAME(victim->specials.fighting),GET_NAME(ch)))    /* yuk! */
		return;
	if(!IS_SET(victim->pflags,PLR_KILLER) && GET_LEVEL(ch) < 21 && GET_LEVEL(victim) < 21 &&
	   !IS_SET(victim->pflags,PLR_THIEF)) {
	    error_log("3\n");
	    sprintf(buf, "%s victimized by %s",
		    GET_NAME(victim), GET_NAME(ch));
	    log(buf);
	    add_witnesses(ch);
	}
    }
}

/*
 *  This routine checks to see whether or not a given character
 *  is allowed to attack another player via the LAW	 
 */

int attack_check(struct char_data *ch, struct char_data *victim) {
  int current_exp;	    /* characters current exp */

    if(!victim)		    /* if there's no victim it's ok */
	return 0;
    if(IS_NPC(ch) || IS_NPC(victim))	    /* or if the victim is an NPC   */
	return 0;
			    /* or the person is a player killer or player thief */
    if(IS_SET(victim->pflags,PLR_KILLER) || IS_SET(victim->pflags,PLR_THIEF))	
	return 0;

    if(ch->points.exp < (titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].exp / 2))
	return 1;		    /* case 1: less than 1/2 exp for level -> send message 1	*/
    if(GET_LEVEL(ch) == 1)    
	return 2;		    /* case 2: lvl 1 -> send message 2	*/

    /*	else, go ahead and attack */

    return 0;
}
