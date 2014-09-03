/*
 * New GUILD code for Thieves World MUD
 * (c)1997 Jan Vandenbos - All rights Reserved.
 * Modification or distribution without express written consent prohibited.
 * Modified quite a bit by Ryan zenker.
 */

/*
 * Note to other coders:  Just use guild flags to change the status of
 * an item, if you need more guilds, just increase the sizer of MAXGUILDS
 * if you acdtually delete or set a slot EMPTY, it will cause you nothing
 * but grief because the guild number (slot number) is in each pfile and
 * they won't correspond - everything will be offset by one.
 */


#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "config.h"
#include "structs.h"
#include "proto.h"
#include "guilds.h"
#include "modify.h"
#include "comm.h"
#include "utils.h"
#include "utility.h"
#include "string.h"
#include "find.h"
#include "recept.h"

struct Guild guilds[MAXGUILDS]; /* allocate guild array */
struct Guild_log guildlog[MAXGUILDLOG];
int logcount = 0;
bool write_guilds();
bool read_guilds();
bool glog(char *name, int type, int amount, int gnum);
bool read_glog();
int inguildfix(int guildnumber);
int guildnumber(char name[MAXGUILDNAME]);


void bootguilds() {
	int iter;
    
    for (iter = 0; iter < MAXGUILDS; ++iter) {  /* clear array */
		guilds[iter].guildflags = GUILD_EMPTY; /* 0 = empty guild */
		guilds[iter].guildname[0] = '\0';
		guilds[iter].guilddesc[0] = '\0';
		guilds[iter].guildgold = 0;
		guilds[iter].guildexp = 0;
		guilds[iter].guildspells = 0;
		guilds[iter].guildskills = 0;
    }
    
    for (iter = 0; iter < MAXGUILDLOG; iter++) {  /* clear array */
		guildlog[iter].name[0] = '\0';
		guildlog[iter].type = 0;
		guildlog[iter].amount = 0;
		guildlog[iter].gnum = 0;
    }
    read_guilds();
    read_glog();
    return;
}


/**************************************************************/


/*
 * actual guild command code here  
 */

ACMD(do_guild)
{
    
    char command[MAX_INPUT_LENGTH];
    char parm1[MAX_INPUT_LENGTH];
    int iter, count,num;
    char output[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char guild[MAX_STRING_LENGTH];
    char amounts[MAX_STRING_LENGTH];
    char line[MAX_INPUT_LENGTH];
    char *ptr;
    int number,gnum,amount; /* guild number for those commands */
    struct char_data *target;
    
    if (!*arg)
    {
	send_to_char("Thou must type GUILD HELP if thou dost not know thy commands.", ch);
	return;
    }
    
    arg = one_argument( arg, command);
    
    count = 0;
    
    if (is_abbrev( command, "list" )) /* list guilds */
    {
	sprintf(output, "###   Guild Name    Description\n\r"
		"---   ---------- ------------------------------\n\r"); 
	for (iter = 1; iter < MAXGUILDS ; ++iter) 
	{
	    /* first non god listing */
	    
	    strcpy(line, ""); /* clear line */
	    
	    if ((guilds[iter].guildflags == GUILD_ACTIVE) || 
	       ((guilds[iter].guildflags == GUILD_DELETED) && IS_GOD(ch)))
	    {
		sprintf(line, "%3d   %-10s %-30s   ",
			iter,
			guilds[iter].guildname,
			guilds[iter].guilddesc);
			count++;
	    }
	    
	    if (IS_GOD(ch)) /* display additional info */
	    {
		switch (guilds[iter].guildflags)
		{
		case GUILD_ACTIVE: strcat (line, "[ACTIVE] "); break;
		case GUILD_DELETED: strcat(line, "[*DELETED*]"); break;
		}
	    }
	    
	    if (guilds[iter].guildflags == GUILD_ACTIVE ||
	       ((guilds[iter].guildflags == GUILD_DELETED) && IS_GOD(ch)))
	    {
		strcat(line, "\n\r"); /* terminate line */
		strcat(output, line); /* add to output */
	    }
	}
	
	sprintf(line, "\n\rTotal guilds: %d of %d\n\r", count,MAXGUILD);
	strcat(output, line);
	
	page_string(ch->desc, output, 1);
	return;
    }
    


    if ( is_abbrev( command, "help" )) {
    	
    	send_to_char("Guild Commands are: \n\r", ch);
		send_to_char("  guild help - this display\n\r", ch);
		send_to_char("  guild list - lists all guilds\n\r", ch);
    	if (IS_GOD(ch)) {
			send_to_char("  guild add [guildname] - add a new guild \n\r", ch);
			send_to_char("  guild delete [slot number] - delete a guild\n\r", ch);
			send_to_char("  guild undelete [slot number] - undeletes a guild.\n\r", ch);
			send_to_char("  guild desc [slot number] [desc] - Set a Description.\n\r",ch);
			send_to_char("  guild gm [player] [slot number] - Create a guildmaster.\n\r", ch);
			send_to_char("  guild ungm [player] - Delete a guildmaster.\n\r", ch);
			send_to_char("  guild initiate [player] [slot number] - Add a member.\n\r", ch);
			send_to_char("  guild premote [player] - Premote to assisant.\n\r", ch);
			send_to_char("  guild demote [player] - Demote.\n\r", ch);
			send_to_char("  guild last - Last transaction on your account.\n\r", ch);
			return;
    	} else if (ch->player.guildinfo.isgm()) {
    		send_to_char("  guild desc [slot number] [desc] - Set a Description.\n\r",ch);
			send_to_char("  guild initiate [player] - Add a member.\n\r", ch);
			send_to_char("  guild premote [player] - Premote to assisant.\n\r", ch);
			send_to_char("  guild demote [player] - Demote.\n\r", ch);
			send_to_char("  guild last - Last transaction on your account.\n\r", ch);
			return;
		} else if (ch->player.guildinfo.istrust()) {
    		send_to_char("  guild initiate [player] - Add a member.\n\r", ch);
    		send_to_char("  guild last - Last transaction on your account.\n\r", ch);
			return;
		}
		return;
    }
    arg = one_argument( arg, parm1);
    
    
    /* Code to ADD a guild */
    
    if ( is_abbrev( command, "add" ) && IS_GOD(ch) ) /* add a guild */
    {
	if (!*parm1)
	{
	    send_to_char("You must specify a guild name.", ch);
	    return;
	}

	if (strlen(parm1) >= MAXGUILDNAME)
	{
	    send_to_char("Guild name too long.\n\r", ch);
	    return;
	}
	
	for (iter = 1 ; iter < MAXGUILDS ; ++iter )
	    if (guilds[iter].guildflags == GUILD_EMPTY)
	    {
		/* found an empty guild */
		
		sprintf(line, "Found an empty slot at #%d - adding.\n\r", iter);
		send_to_char(line, ch);
		
		/* cvt parm1 to upper case */
		
		for (ptr = parm1 ; *ptr ; ++ptr )
		    *ptr = UPPER(*ptr);
		
		guilds[iter].guildflags = GUILD_ACTIVE;
		strcpy(guilds[iter].guildname, parm1);
		strcpy(guilds[iter].guilddesc, "");
		
		if ( write_guilds() ) /* error? */
		{
		    send_to_char("Unable to write to guild file!\n\r", ch);
		    return;
		}
		sprintf( line, "New Guild: %s at slot %d.", 
			 parm1, iter);
		log_msg( line );
		return;
	    }
	
	send_to_char("Sorry, there are no empty guild slots.\n\r", ch);
	log_msg ( "Attempt to add new guild to full table - expand? " );
	return;
    }
    
    
    /* Code to DELETE a guild */
    
    if ( is_abbrev(command, "delete") && IS_GOD(ch) ) /* add a guild */
    {
	if (!*parm1)
	{
	    send_to_char("You must specify a guild number to delete.", ch);
	    return;
	}
	
	number = atoi(parm1);

	if ( guilds[number].guildflags != GUILD_ACTIVE )
	{
	    send_to_char("That is not an active guild.\n\r", ch);
	    return;
	}
	
	if ( ( number < 1) || (number > MAXGUILDS) )
	{
	    send_to_char("Sorry, thats not a valid guild number.\n\r", ch);
	    return;
	}
	
	guilds[number].guildflags = GUILD_DELETED;
	
	send_to_char("Guild deleted.\n\r", ch);
	write_guilds();
	sprintf(line, "Guild # %d deleted.", number);
	
	log_msg(line);
    }
    
    /* Code to UNDELETE a guild */
    
    if ( is_abbrev(command, "undelete") && IS_GOD(ch) )
    {
	if (!*parm1)
	{
	    send_to_char("Sorry, thats not a valid guild number.\n\r", ch);
	    return;
	}
	
	number = atoi(parm1);
	
	if ((number < 1) || (number > MAXGUILDS)) 
	{
	    send_to_char("That is not a valid slot number.\n\r", ch);
	    return;
	}
	
	if (guilds[number].guildflags != GUILD_DELETED)
	{
	    send_to_char("Sorry you can only restore deleted guilds.\n\r", ch);
	    return;
	}
	
	guilds[number].guildflags = GUILD_ACTIVE; /* reenable guild */ 
	write_guilds();
	send_to_char("Guild updated successfully.\n\r", ch);
	return;
    }

    /* code for setting guild description */

    if (is_abbrev(command, "description" ) && (IS_GOD(ch) || ch->player.guildinfo.isgm()) ) /* set guild description */
    {
	if (!*parm1) 
	{
	    send_to_char("You need to specify a slot number.\n\r", ch);
	    return;
	}

	if (!*arg) 
	{
	    send_to_char("Need a description after the slot number.\n\r", ch);
	    return;
	}

	number = atoi(parm1);
	
	if (( number < 1) || (number > MAXGUILDS)) 
	{
	    send_to_char("That is not a valid slot number.\n\r", ch);
	    return;
	}

	if (guilds[number].guildflags != GUILD_ACTIVE)
	{
	    send_to_char("Sorry you can only set descriptions on active guilds.\n\r", ch);
	    return;
	}

	if (strlen(arg) >= MAXGUILDDESC)
	{
	    send_to_char("Sorry, your description is too long.\n\r", ch);
	    return;
	}

	strcpy(guilds[number].guilddesc, arg);
	
	write_guilds();

	log_msg("Guild desc updated sucessfully.");
	return;
    }

    /* make a guildmaster */

    if ( is_abbrev(command, "gm") && IS_GOD(ch) ) 
    {
	if (!*parm1)
	{
	    send_to_char("Need to specify WHO to make a guildmaster.\n\r", ch);
	    return;
	}

	if (!*arg)
	{
	    send_to_char("Need to specify a guild number after the player name\n\r", ch);
	    return;
	}

	number = atoi(arg);

	if (( number < 1) || (number >= MAXGUILDS))
	{
	    send_to_char("Thats an invalid guild number.(Player guilds > 0)\n\r", ch);
	    return;
	}

	if (( target = get_char_vis(ch, parm1))) /* find the target */
	{
	    if (IS_NPC(target))
	    {
		send_to_char("Cannot make a mobile into a guildmaster.\n\r", ch);
		return;
	    }
	 
	    /* put player into the guild */

	    target->player.guildinfo.makegm(number);

	    sprintf(line, "%s entered as guildmaster of guild #%d.",
		    parm1, number);
	    log_msg(line);
	    send_to_char("Ok, they're now a guildmaster.\n\r", ch);
	    SaveChar(ch, AUTO_RENT, 0);
	    return;
	}
	else
	{
	    send_to_char("I cannot find that person anywhere.\n\r", ch);
	    return;
	}
    }
    
    /* Initiate a player into a guild */
    if ( is_abbrev(command, "initiate") && (IS_GOD(ch) || (ch->player.guildinfo.isgm()))) 
    {
	if (!*parm1)
	{
	    send_to_char("Need to specify WHO to initiate.\n\r", ch);
	    return;
	}

	if (!*arg && !(ch->player.guildinfo.isgm()))
	{
	    send_to_char("Need to specify a guild number after the player name\n\r", ch);
	    return;
	}

	if (IS_GOD(ch)) {
		number = atoi(arg);
	} else {
		number = ch->player.guildinfo.inguild();
	}

	if (( number < 1) || (number >= MAXGUILDS))
	{
	    send_to_char("Thats an invalid guild number.(Player guilds > 0)\n\r", ch);
	    return;
	}

	if (!IS_GOD(ch) && (GET_GGOLD(number) < GUILD_GCOST_INIT)) {
		send_to_char("Deposit some gold!\n\r", ch);
		return;
	}


	if (( target = get_char_vis(ch, parm1))) /* find the target */
	{
	    if (IS_NPC(target))
	    {
		send_to_char("Cannot initiate a mobile into a guild.\n\r", ch);
		return;
	    }
	 
	    if((GetMaxLevel(target)<50) && (!IS_GOD(ch))) {
		send_to_char("I'm sorry...only gods can initiate people under level 50...\n\r", ch);
		return;
	    }

	    target->player.guildinfo.makenew(number);
	    GET_GGOLD(number) = GET_GGOLD(number) - GUILD_GCOST_INIT;

	    sprintf(line, "%s initiated into guild #%d.",
		    parm1, number);
	    log_msg(line);
	    send_to_char("Ok, now they're initiated.\n\r", ch);
	    SaveChar(ch, AUTO_RENT, 0);
	    return;
	}
	else
	{
	    send_to_char("I cannot find that person anywhere.\n\r", ch);
	    return;
	}
    }
    
    /* Premote a guildmember to GUILD_ASSISTANT. */

    if (is_abbrev(command, "premote") && (IS_GOD(ch) || ch->player.guildinfo.isgm())) 
    {
	if (!*parm1)
	{
	    send_to_char("Need to specify WHO to premote.\n\r", ch);
	    return;
	}

	if ((!*arg) && IS_GOD(ch))
	{
	    send_to_char("Need to specify a guild number after the player name\n\r", ch);
	    return;
	} else if (IS_GOD(ch)) {
		number = atoi(arg);
	} else {
		number = ch->player.guildinfo.inguild();
	}

	if (( number < 1) || (number >= MAXGUILDS))
	{
	    send_to_char("Thats an invalid guild number.(Player guilds > 0)\n\r", ch);
	    return;
	}

	if (( target = get_char_vis(ch, parm1))) /* find the target */
	{

	    /* premote player */
	    
	    if (target->player.guildinfo.inguild() != number) {
			send_to_char("That player is not in your guild.\n\r", ch);
			return;
		}
	    target->player.guildinfo.makeassistant(number);

	    sprintf(line, "%s entered as assistant of guild #%d.",
 		    parm1, number);
	    log_msg(line);
	    send_to_char("Ok, they're premoted to an assistant.\n\r", ch);
	    SaveChar(ch, AUTO_RENT, 0);
	    return;
	}
	else
	{
	    send_to_char("I cannot find that person anywhere.\n\r", ch);
	    return;
	}
    }
    
    /* Demote a luser. */

    if (is_abbrev(command, "demote") && (IS_GOD(ch) || ch->player.guildinfo.isgm())) 
    {
	if (!*parm1)
	{
	    send_to_char("Need to specify WHO to demote.\n\r", ch);
	    return;
	}

	if (!IS_GOD(ch)) {
		number = ch->player.guildinfo.inguild();
	} else {
		number = 5;
	}
	
	
	if (( number < 1) || (number >= MAXGUILDS))
	{
	    send_to_char("Thats an invalid guild number.(Player guilds > 0)\n\r", ch);
	    return;
	}

	if (( target = get_char_vis(ch, parm1))) /* find the target */
	{
	 
	    /*demote player */
	    
	    if ((target->player.guildinfo.inguild() != number) && (!IS_GOD(ch))) {
			send_to_char("That player is not in your guild.\n\r", ch);
			return;
		}


	    target->player.guildinfo.setflag(0);

	    sprintf(line, "%s demoted in guild #%d.",
		    parm1, number);
	    log_msg(line);
	    send_to_char("Ok, they're now demoted.\n\r", ch);
	    SaveChar(ch, AUTO_RENT, 0);
	    return;
	}
	else
	{
	    send_to_char("I cannot find that person anywhere.\n\r", ch);
	    return;
	}
    }
    
    /* Remove a GM */
    if (is_abbrev(command, "ungm") && IS_GOD(ch) )
    {

	if (!*parm1)
	{
	    send_to_char("Need to tell me who to unmake gm.\n\r", ch);
	}

	if (( target = get_char_vis(ch, parm1))) {
	    if (!target->player.guildinfo.isgm()) 
	    {
		send_to_char("That player is not a guildmaster.\n\r", ch);
		return;
	    }

	    sprintf(line, "Player: %s cleared from guild number: %d.",
		    parm1, target->player.guildinfo.inguild());
	    log_msg(line);
	    target->player.guildinfo.clear(); /* clear guild info */
	    SaveChar(ch, AUTO_RENT, 0);
	    return;
	}
	else
	{
	    send_to_char("I'm sorry, I cannot find that person anywhere.\n\r", ch);
	    return;
	}
    }
    
	/***  Get the guild's current balance ***/

	if (is_abbrev(command, "balance") && IS_GOD(ch) ) {
		if (!*parm1) {
	    send_to_char("guild balance <guild>\n\r", ch);
			return;
		} else {
			if(!(gnum = guildnumber(parm1))) {
				send_to_char("That there guild don't exist!!\n\r",ch);
				return;
			}
		}

		sprintf(buf,"%s currently has $CC%Ld$CN $CRgold$CN and $CG%Ld$CN $CRexperience$CN.\n\r", 
							parm1,
							GET_GGOLD(gnum),
							GET_GEXP(gnum));
		send_to_char_formatted(buf,ch);
		return;

	}

	/***  Charge the guild ***/

	if (is_abbrev(command, "charge") && IS_GOD(ch) ) {
		if (!*parm1) {
	    send_to_char("guild charge <exp|gold> <guild> <amount>\n\r", ch);
			return;
		}
		arg = one_argument( arg, guild);

		if (!*guild) {
	    	send_to_char("guild charge <exp|gold> <guild> <amount>\n\r", ch);
				return;
		} else 
			if(!(gnum = guildnumber(guild))) {
	    	send_to_char("Maybe you should type `guild list`?\n\r", ch);
				return;
			}

		arg = one_argument( arg, amounts);
		if (!*amounts) {
	    send_to_char("guild charge <exp|gold> <guild> <amount>\n\r", ch);
			return;
		} else
			amount = atoi(amounts);

		if(!strcmp(parm1,"exp")) {
			if(GET_GEXP(gnum) < amount) {
				send_to_char("They don't have enough experience!!!\n\r",ch);
				return;
			} else {
				GET_GEXP(gnum) = GET_GEXP(gnum) - amount;
				sprintf(buf,"%d experience has been collected from %s.\n\r",amount,guild);
				send_to_char(buf,ch);
				write_guilds();
				return;
			}
		} else if(!strcmp(parm1,"gold")) {
			if(GET_GGOLD(gnum) < amount) {
				send_to_char("They don't have enough gold!!!\n\r",ch);
				return;
			} else {
				GET_GGOLD(gnum) = GET_GGOLD(gnum) - amount;
				sprintf(buf,"%d gold has been collected from %s.\n\r",amount,guild);
				send_to_char(buf,ch);
				write_guilds();
				return;
			}
		} else {
	    send_to_char("guild charge <exp|gold> <guild> <amount>\n\r", ch);
			return;
		}
	}
		
    
    
    /* Last Command (Read the log). */

    if (is_abbrev(command, "last") && (IS_GOD(ch) || ch->player.guildinfo.istrust())) 
    {
		num=0;	
	if (!*parm1) {
	    num = 1;
	} else {
		num = atoi(parm1);
	}

	if (!IS_GOD(ch)) {
		number = ch->player.guildinfo.inguild();
	} else {
		number = 0;
	}
	
	count=0;
	iter=logcount;
	strcpy(output,""); /* Clear output buffer */
	while ((iter > 0) && (iter < MAXGUILDLOG) && (count < num)) {
		/* sprintf(line,"num:%d  count:%d  iter:%d",num,count,iter);
		log_msg(line); */
		strcpy(line, ""); /* clear line */
		if (guildlog[iter].gnum==number && (!IS_GOD(ch))) {
			count++;
	    	switch(guildlog[iter].type) {
	    		case GUILD_WITHDRAW:	sprintf(line, "%s withdrew %d Gold.",
									guildlog[iter].name,
									guildlog[iter].amount);
									break;
			case GUILD_DEPOSIT_EXP:	sprintf(line, "%s deposited %d Exp.",
									guildlog[iter].name,
									guildlog[iter].amount);
									break;
			case GUILD_DEPOSIT_GOLD:sprintf(line, "%s deposited %d Gold.",
									guildlog[iter].name,
									guildlog[iter].amount);
									break;
			}
			strcat(line, "\n\r"); /* terminate line */
			strcat(output, line); /* add to output */
	    }
	    if (IS_GOD(ch)) {
	    	count++;
	    	switch(guildlog[iter].type) {
	    		case GUILD_WITHDRAW:	sprintf(line, "%s withdrew %d Gold from %s.",
									guildlog[iter].name,
									guildlog[iter].amount,
									guildname(guildlog[iter].gnum));
									break;
			case GUILD_DEPOSIT_EXP:	sprintf(line, "%s deposited %d Exp into %s.",
									guildlog[iter].name,
									guildlog[iter].amount,
									guildname(guildlog[iter].gnum));
									break;
			case GUILD_DEPOSIT_GOLD:	sprintf(line, "%s deposited %d Gold into %s.",
									guildlog[iter].name,
									guildlog[iter].amount,
									guildname(guildlog[iter].gnum));
									break;
			}
			/* log_msg(line); */
			strcat(line, "\n\r"); /* terminate line */
			strcat(output, line); /* add to output */
		}
		iter--;
	}
	page_string(ch->desc, output, 1);
	return;
	}		
}


/**************************************************************/


SPECIAL(guild_bank)
{
    static char buf[256];
    int gnum=0;
    unsigned int amount=0;
    char command[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH];
    char *ptr;

    /* arg = one_argument( arg, command);
    arg = one_argument( arg, parm1);
    money = atoi(arg); */
    

    if(!(cmd==219 || cmd==220 || cmd==221))
        return FALSE;

    if (IS_NPC(ch))
    {
        send_to_char("I'm sorry, but we don't deal with monsters!  Shoo!\n\r",
                     ch);
        return(TRUE);
    }

#define DEPO \
"You are carrying over 2 million gold!!  Deposit it or lose it!\n\r"

/* Deposit [exp|gold] [amount] [guild]', 'Withdraw [amount]' */


    /*deposit*/
    if (cmd==219) {
        arg = one_argument( arg, command);
        arg = one_argument(arg,temp);
        amount = atol(temp);
        ptr = one_argument(arg,temp);
        if (!*arg) {
        	if (ch->player.guildinfo.inguild() == 0) {
        		send_to_char("You must specify a guild.\n\r",ch);
        		return(TRUE);
        	} else {
        		gnum = ch->player.guildinfo.inguild();
        	}
        } else {
        	gnum = guildnumber(temp);
        	/* sprintf(buf,"-%s- -%d-",temp,gnum);
        	log_msg(buf); */
        	if (!(guilds[gnum].guildflags & GUILD_ACTIVE)) {
        		send_to_char("You must specify an active guild!\n\r", ch);
            	return(TRUE);
            }
        }
        if (is_abbrev(command, "exp")) {
        	if (amount > GET_EXP(ch)) {
        		send_to_char("You don't have enough for that!\n\r", ch);
            	return(TRUE);
            } else if (amount <= 0) {
            	send_to_char("Go away, you bother me.\n\r", ch);
            	return(TRUE);
       		} else {
       			send_to_char("Thank you.\n\r",ch);
            	GET_EXP(ch) = GET_EXP(ch) - amount;
            	GET_GEXP(gnum) = GET_GEXP(gnum) + amount;
            	if (ch->player.guildinfo.inguild() == gnum) {
            		sprintf(buf,"The balance is %Ld.\n\r", GET_GEXP(gnum));
            		send_to_char(buf, ch);
            	}
            	SaveChar(ch, ch->in_room, FALSE);
            	write_guilds();
            	glog(ss_data(ch->player.name),GUILD_DEPOSIT_EXP,amount,gnum);
            	return(TRUE);
        	}
        }
        if (is_abbrev(command, "gold")) {
        	if (amount > (unsigned int) GET_GOLD(ch)) {
            	send_to_char("You don't have enough for that!\n\r", ch);
            	return(TRUE);
        	} else if (amount <= 0) {
            	send_to_char("Go away, you bother me.\n\r", ch);
            	return(TRUE);
        	} else {
            	send_to_char("Thank you.\n\r",ch);
            	GET_GOLD(ch) = GET_GOLD(ch) - amount;
            	GET_GGOLD(gnum) = GET_GGOLD(gnum) + amount;
            	if (ch->player.guildinfo.inguild() == gnum) {
            		sprintf(buf,"Your balance is %Ld.\n\r", GET_GGOLD(gnum));
            		send_to_char(buf, ch);
            	}
            	SaveChar(ch, ch->in_room, FALSE);
            	write_guilds();
            	glog(ss_data(ch->player.name),GUILD_DEPOSIT_GOLD,amount,gnum);
            	return(TRUE);
        	}
        }
    
        /*withdraw*/

    } else if (cmd==220) {
    	amount = atoi(arg);
    	gnum = ch->player.guildinfo.inguild();
    	if (gnum == 0) {
    		send_to_char("Get a guild!\n\r", ch);
    		return(TRUE);
    	}
    	if (!ch->player.guildinfo.istrust()) {
    		send_to_char("You are not allowed to withdraw.\n\r", ch);
    		return(TRUE);
    	}
		if (GET_GOLD(ch) > 2000000) {
    		send_to_char(DEPO, ch);
  		}
		if (amount > GET_GGOLD(gnum)) {
            send_to_char("You don't have enough in the bank for that!\n\r", ch);
            return(TRUE);
        } else if (amount <= 0) {
            send_to_char("Go away, you bother me.\n\r", ch);
            return(TRUE);
        } else {
            if((GET_GOLD(ch)<2000000) || (TRUST(ch)>TRUST_SAINT)) {
                send_to_char("Thank you.\n\r",ch);
                GET_GOLD(ch) = GET_GOLD(ch) + amount;
                GET_GGOLD(gnum) = GET_GGOLD(gnum) - amount;
                sprintf(buf,"Your balance is %Ld.\n\r", GET_GGOLD(gnum));
                send_to_char(buf, ch);
                SaveChar(ch, ch->in_room, FALSE);
                write_guilds();
                glog(ss_data(ch->player.name),GUILD_WITHDRAW,amount,gnum);
                return(TRUE);
            } else {
                send_to_char("You can not possibly carry any more coins!  Make deposit.\n\r", ch);
            }
        }
    } else if (cmd == 221) {
    	if (!*arg) {
    		gnum = ch->player.guildinfo.inguild();
			if (gnum == 0) {
        		send_to_char("Get a guild.\n\r",ch);
        		return(TRUE);
        	}
    	} else if (IS_GOD(ch)) {
    		gnum = atoi(arg);
    	}
        sprintf(buf,"Your guild has collected %Ld Gold, and %Ld Exp.\n\r", GET_GGOLD(gnum),GET_GEXP(gnum));
        send_to_char(buf, ch);
        return(TRUE);
    }
    return(FALSE);
}

/**************************************************************/

bool read_guilds() {
	int iter=0,num=0;
    FILE *guildfile;
    char line[MAX_INPUT_LENGTH];
    char *ptr; /* used for cleaning up guild name */

	guildfile = fopen(GUILDFILE, "r");
    if (guildfile == NULL) {
		log_msg( "Empty guild file.");
		return true;
    }
    
    fseek(guildfile, 0, SEEK_SET);
    
    /* populate the guild array */
    
    while ( feof(guildfile) == 0 || (iter >= MAXGUILDS)) {
    

		fgets(line, MAX_INPUT_LENGTH, guildfile);
		num = atoi(line); 
		
		if (num == 69) { /* Little patch cause the file read like shit */
			break;
		}
		
		if ((num >= MAXGUILDS) || (num < 1)) {
			log_msg("Guild File is corrupt. Fix Me!");
		}
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guilds[num].guildflags = atoi(line); 
	
		if (feof(guildfile) == 0) {
	    	fgets(line, MAX_INPUT_LENGTH, guildfile);
	    	strcpy(guilds[num].guildname, line);
		}
		
		if ( feof(guildfile) == 0 ) {
	    	fgets(line, MAX_INPUT_LENGTH, guildfile);
	    	strcpy(guilds[num].guilddesc, line);
		}
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guilds[num].guildgold = atoi(line); 
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guilds[num].guildexp = atoi(line); 
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guilds[num].guildspells = atoi(line); 
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guilds[num].guildskills = atoi(line); 

		/* clean up the guild info (cr/lf's/etc) */
	
		for (ptr = guilds[num].guildname; *ptr; ++ptr)
	    	if (*ptr == '\n')
				*ptr = '\0';
	
		for (ptr = guilds[num].guilddesc; *ptr; ++ptr)
	    	if (*ptr == '\n')
				*ptr = '\0';
		iter++;
    }
    
    sprintf(line, "Guilds read in: %d", iter);
    log_msg( line );
    
    fclose(guildfile);
    return false;
}


/**************************************************************/


bool write_guilds()
{
    FILE *guildfile;
    int iter;
    char line[MAX_INPUT_LENGTH];

    guildfile = fopen (GUILDFILE, "w+" );
    if (guildfile == NULL) 
    {
	log_msg("Error writing guildfile");
	return true;
    }

    for (iter = 1; iter < MAXGUILDS; ++iter)
    {
	  if (guilds[iter].guildflags == GUILD_ACTIVE) {
	    sprintf(line, "%d\n%d\n%s\n%s\n%Ld\n%Ld\n%d\n%d\n",
	        iter,
		    guilds[iter].guildflags,
		    guilds[iter].guildname,
		    guilds[iter].guilddesc,
		    guilds[iter].guildgold,
		    guilds[iter].guildexp,
		    guilds[iter].guildspells,
		    guilds[iter].guildskills );
	    fputs(line, guildfile);
	  }
    }
	sprintf(line,"69\n");
	fputs(line,guildfile);

    fclose( guildfile );

    /* log_msg("Guild file updated."); */
    return false;
}


/**************************************************************/

char *guildname(int guildnum)
{
static char buf[MAX_INPUT_LENGTH];

if ((guildnum < 1) || (guildnum >= MAXGUILDS))
  sprintf(buf, "");
else
  sprintf(buf, "%s", guilds[guildnum].guildname);

return buf;
}

/**************************************************************/

int guildnumber(char name[MAXGUILDNAME]) {
        bool boo=false;
        int i;
        char *ptr;

        for (ptr = name ; *ptr ; ++ptr )
		*ptr = UPPER(*ptr);
        for(i=1;i<MAXGUILDS;i++) {
        	if (strcmp(guilds[i].guildname,name)==0) {
                	boo=true;
                        break;
                }
        }
     	if (!boo) return 0;
     	return i;
}

/**************************************************************/

bool read_glog() {
    int iter;
    FILE *guildfile;
    char line[MAX_INPUT_LENGTH] = "";
    char *ptr; /* used for cleaning up guild name */
    
	guildfile = fopen(GUILDLOG, "r");
    if (guildfile == NULL) { 
		log_msg( "Empty guildlog file.");
		return true;
    }
    
    fseek(guildfile, 0, SEEK_SET);
      
    /* populate the guild array */
    iter=0;
    while ( feof(guildfile) == 0) {
    	fgets(line, MAX_INPUT_LENGTH, guildfile);
	    if (strncmp(line,"69",2) == 0) {
			log_msg("Guildlog read in successfully.");
			break;
		}
	    strcpy(guildlog[iter].name, line);

		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guildlog[iter].type = atoi(line); 
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guildlog[iter].amount = atoi(line); 
		
		fgets(line, MAX_INPUT_LENGTH, guildfile);
		guildlog[iter].gnum = atoi(line);
		
		for (ptr = guildlog[iter].name; *ptr; ++ptr)
	    	if (*ptr == '\n')
				*ptr = '\0';
		iter++;
	}
	logcount = iter-1;
	return false;
} 
	

/**************************************************************/

int inguildfix(int guildnumber) {
	if ((guilds[guildnumber].guildflags == GUILD_ACTIVE) || 
		(guildnumber==0)) {
    	return guildnumber; 
    } else {
    	guildnumber=0;
    	log_msg("Guildnumber Reset to 0");
    	return guildnumber;
    }
}


/**************************************************************/


bool kill_glog() {
	int iter;
    
	for (iter=0;iter<=(MAXGUILDLOG/2);iter++) {
		strcpy(guildlog[iter].name, guildlog[iter+(MAXGUILDLOG/2)].name);
		guildlog[iter].type = guildlog[iter+(MAXGUILDLOG/2)].type; 
		guildlog[iter].amount = guildlog[iter+(MAXGUILDLOG/2)].amount; 
		guildlog[iter].gnum = guildlog[iter+(MAXGUILDLOG/2)].gnum;
	}	
	
	logcount = (MAXGUILDLOG/2);
	return false;
} 


/**************************************************************/



bool glog(char name[20], int type, int amount, int gnum) {
	
	FILE *guildfile;
	int iter;
    char line[MAX_INPUT_LENGTH];

    if (logcount >= MAXGUILDLOG) {
    	kill_glog();
    }
    
    logcount++;
    
    strcpy(guildlog[logcount].name,name);
    guildlog[logcount].type = type;
    guildlog[logcount].amount = amount;
    guildlog[logcount].gnum = gnum;
    
    guildfile = fopen (GUILDLOG, "w+" );
    if (guildfile == NULL) 
    {
	log_msg("Error writing guildlogfile");
	return true;
    }
	

	for (iter = 0; iter <= logcount; iter++) {
		sprintf(line,"%s\n%d\n%d\n%d\n",
			guildlog[iter].name,
			guildlog[iter].type,
			guildlog[iter].amount,
			guildlog[iter].gnum);
		fputs(line, guildfile);
	}
	sprintf(line,"69\n");
	fputs(line,guildfile); 
	
	fclose( guildfile );
	return(FALSE);
}
