/* ----------------------------------------------------------------- */
/* Object Mailing extensions for FLANDS code.... v0.00.0.0.0 ALPHA   */
/* by: Jan Vandenbos, Newbie TWCoder :)                              */
/* last version date: 14 February 1995                               */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/* Copyright notice:                                                 */
/*    The code contained in this file is NOT copyrighted, or         */
/* protected in ANY way.  It is original work based on flands code   */
/* as such, do with it what you will.   Change it, Sell it, feed     */
/* it to your neighbors dog even.    I had fun writing it, and       */
/* thats enough for me.                                              */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/* installation instructions (WIDRTFM!)                              */
/*                                                                   */
/* 1.  Edit the mail.c file... change the #define MAIL* to your      */
/*     preferences                                                   */
/* 2.  Edit the Makefile, and all the necessary references to        */
/*     compile and link mail.c (and dependants)                      */
/* 3.  Edit ACT.H and CMDTAB.C to refer to the following proc's:     */
/*     do_mail_send, do_mail_check, do_mail_receive                  */
/*     also do_mail_god_receive if you want immorts to be able to    */
/*     pull up the players mail (nondestructive)... (59+)            */
/*     note: if you use CHECK for the command to check mail,         */
/*           you need to take into account that it will conflict     */
/*           with the 'checkin' command                              */
/* 4.  Compile                                                       */
/* 5.  Write a Help File                                             */
/* 6.  Playtest...  remember though... you didn't opt for that       */
/*     service contract :p                                           */
/* 7.  OH YEAH... important:  under /lib create the following:       */
/*     lib/mail   lib/mail/a .. lib/mail/z                           */
/* 8.  If you wish the player to be notified of mail waiting when    */
/*     the log in, then in the login procedure somewhere, call       */
/*     do_mail_check (prototype in act.h) which will notify the ch   */
/*     of mail waiting for them.                                     */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/* things to watch for:                                              */
/*   All the object files have magic number for each and every       */
/* object.   If they encounter a read error (matching the number)    */
/* it will be in the log as well as WriteToImmortal'd.   I suggest   */
/* that if the file got corrupted somehow, you just delete it        */
/* and tell the poor player that they're SOL! :)                     */
/* i playtested this code somewhat, and had some recommendations     */
/* from friends, but... its ALPHA... so don't use it as an axle      */
/* stand for the rig your working on.                                */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/* things to do:                                                     */
/*    Handling of orphan files... next priority.   Probably should   */
/* do something in the script which deletes the player file, such    */
/* that it also deletes the GET_REAL_NAME(ch).objs file in the       */
/* lib/mail tree.                                                    */
/*    I still haven't added the orphan check for files > 15 days     */
/* nor for .objs files for players which no longer exist             */
/* either i'll write it when i get a chance, or someone should       */
/* write a script to check file dates, and delete past a certain     */
/* period.  Also... handling for such things as 2.object and         */
/* all.object aren't there... they might never be.                   */
/* Maybe add something so that instead of parchment, when it reads   */
/* the message back it, the short description gets changed to        */
/* Message from [Bob] or something like that (not too tough)         */
/* Add gold counter to global to see how much the mail elf brought   */
/* in for profit :)  Maybe the IMPS can finally get that projection  */
/* tv and satellite dish for the IMP room :)                         */
/* CONSIDER ROLLBACK!!!						     */
/* ----------------------------------------------------------------- */


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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "trap.h"
#include "utility.h"
#include "constants.h"
#include "act.h"
#include "find.h"
#include "util_str.h"
#include "recept.h"
#include "statistic.h"
#include "multiclass.h"
#include "proto.h"

/* here beginneth the defines specific to the mailer */
/* feel free to change these, or move these to config.h */
/* but remember its YOUR mud... :) */

#define MAX_WEIGHT 30
#define MAX_VALUE 7500
#define MAX_MAIL_PER_PLAYER 5
#define MAIL_PRICE 500
#define MUST_STORE FALSE
#define MAIL_LEVEL 10
#define MAIL_MAGIC 0x12345606


/* prototype from interpreter.c */
void argument_interpreter(char *argument,char *first_arg,char *second_arg );

/* ----------------------------------------------------------------------- */
/*                     Produces filespec from character name               */
/* ----------------------------------------------------------------------- */

int make_mail_filename(char *char_name, char *filename) {
	
   char *ptr, name[30];
   
   if (!*char_name) /* did we get passed a character name? */
     return FALSE;

   strcpy(name, char_name);
   for (ptr = name; *ptr; ptr++) /* make it lowercase which is */
     *ptr = tolower(*ptr); /* important since its going to be a filename */

   sprintf(filename, "mail/%c/%s.mail", *name, name);

   return TRUE; /* hey it worked... omigawwwwwwwwwd! */
}



/* --------------------------------------------------------------- */
/*             routine to write object to player.obj file          */
/* --------------------------------------------------------------- */

int write_mail(struct obj_data* obj, struct char_data* ch, char *vict_name)
{
   char fname[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
   FILE *fp; /* the file pointer */

   make_mail_filename(vict_name,fname);
   if (!(fp=fopen(fname,"ab")))  {
      sprintf(buf,"ERROR - Unable to open/create %s.",fname);
      slog(buf);
      return TRUE;
   }
   fprintf(fp,"%d\n",MAIL_MAGIC);  /* write header information */
   fprintf(fp,"%s\n",GET_REAL_NAME(ch)); /* from */
   fprintf(fp,"%s\n",vict_name); /* to - for error checking */
   fprintf(fp,"%ld\n",time(0));
   fprintf(fp,"%d\n",obj_index[obj->item_number].virt);
#ifndef OFFLINE_NOTEBUG
   putsstring(obj->action_description,fp);
   putdword(obj->obj_flags.value[0], fp);
   putdword(obj->obj_flags.value[1], fp);
   putdword(obj->obj_flags.value[2], fp);
   putdword(obj->obj_flags.value[3], fp);
   putdword(obj->obj_flags.timer, fp);
#endif
   fclose(fp); 
   return FALSE;   
}


/* -------------------------------------------------------------------- */
/*                     a procedure to count mail waiting                */
/* -------------------------------------------------------------------- */

int mail_count(char *charname)
{
   char fname[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
   FILE *fp;
   int magicnum, count = 0;
   sstring_t* action_description; /* to allows us to read an sstring */
   long temp;
   
   make_mail_filename(charname,fname);
   
   if (!(fp=fopen(fname,"rb")))  
      return 0;  /* either file doesn't exist or theres an error */
   
   while (1)  { /* ok ok so i'm lazy */        
      if (!fgets(buf,sizeof(buf),fp))
      	break; /* eof or error occured on read at this point */
      sscanf(buf,"%d",&magicnum);
      if (magicnum != MAIL_MAGIC)  {
	 sprintf(buf,"MAIL - Invalid Magic Numero in %s file",charname);
	 slog(buf);
	 WriteToImmort(buf, TRUST_GRUNT);
	 return 0;
      }
      fgets(buf,sizeof(buf),fp); /* read line with FROM on it */
      fgets(buf,sizeof(buf),fp); /* read line with TO on it */
      fgets(buf,sizeof(buf),fp); /* read line containing time */
      fgets(buf,sizeof(buf),fp); /* read the vnum */
#ifndef OFFLINE_NOTEBUG
      getsstring(&action_description,fp);  /* read in the note contents */
      getdword((long*) &temp, fp);
      getdword((long*) &temp, fp);
      getdword((long*) &temp, fp);  /* these lines are just dummy's */
      getdword((long*) &temp, fp);
      getdword((long*) &temp, fp); 
      ss_free(action_description);  /* junk variable, so free it */
#endif
      ++count; /* the basic item counter */
   }   								     
   fclose(fp);
   return(count);
}
         

/* -------------------------------------------------------------------- */
/*              a procedure to read objects from the file               */
/* -------------------------------------------------------------------- */

void do_mail_receive(struct char_data* ch, char *arguments, int cmd) 
{   
   char fname[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
   FILE *fp;
   int magicnum, vnum;
   char from[MAX_INPUT_LENGTH];
   struct obj_data* obj;
   
   if (mail_count(GET_REAL_NAME(ch)) < 1)  {
      send_to_char("Sorry, I have nothing in storage for you.\n\r",ch);
      return;
   }
   send_to_char("The TWex delivery elf falls from the sky, picks herself up\n\r"
		"and looks at her list.  Seeing your name on her list, she smiles\n\r"
		"and reaches into her Bag of Holding.\n\r",ch);
   make_mail_filename(GET_REAL_NAME(ch),fname);
   if (!(fp=fopen(fname,"rb")))
        return;  /* either file doesn't exist or theres an error */
   while (1)   {
        if (!fgets(buf,sizeof(buf),fp))
            break; /* eof or error occured on read at this point */
        sscanf(buf,"%d",&magicnum);
        if (magicnum != MAIL_MAGIC)   {
            sprintf(buf,"MAIL - Invalid Magic Numero in %s file",
		    GET_REAL_NAME(ch));
            slog(buf);
            WriteToImmort(buf,TRUST_GRUNT);
	    send_to_char("Suddenly the elf frowns, realizes theres been a mistake and disappears.\n\r",ch);
	    fclose(fp);
            return;
        }
        fgets(from,sizeof(from),fp); /* read line with FROM on it */
        from[strlen(from)-1] = '\0'; /* get rid of newline char */
        fgets(buf,sizeof(buf),fp); /* read line with TO on it */
        fgets(buf,sizeof(buf),fp); /* read line containing time */
        fgets(buf,sizeof(buf),fp); /* read the vnum */
        sscanf(buf,"%d",&vnum);
        obj = make_object(vnum,VIRTUAL);
#ifndef OFFLINE_NOTEBUG
        getsstring(&obj->action_description,fp);
	getdword((long*) &obj->obj_flags.value[0], fp);
	getdword((long*) &obj->obj_flags.value[1], fp);
	getdword((long*) &obj->obj_flags.value[2], fp);
	getdword((long*) &obj->obj_flags.value[3], fp);
	getdword((long*) &obj->obj_flags.timer, fp);
#endif
        sprintf(buf,"She hands you %s from %s.\n\r",OBJ_SHORT(obj),from);
        send_to_char(buf,ch);
        if (((IS_CARRYING_N(ch) + 1) > CAN_CARRY_N(ch)) ||
  	    ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)))  {
	      send_to_char("Hey... you can't carry that... here I'll put it on the floor for you.\n\r",ch);
	      obj_to_room(obj,ch->in_room);
        }
        else
      	   obj_to_char(obj,ch); /* note... at this point i don't worry about weight */
   }
   fclose(fp);
   remove(fname);
   send_to_char("Seeing that her job is complete, she vanishes with a POP!\n\r",ch);
   act("$n just received a package.", TRUE, ch, NULL, NULL, TO_ROOM);
   do_save(ch,"",0); /* save our work */
   return;
   
}

/* -------------------------------------------------------------------- */
/*             ALLOWS A GOD TO GET A COPY OF PLAYERS MAIL               */
/* -------------------------------------------------------------------- */

void do_mail_god_receive(struct char_data *ch, char *arguments, int cmd)
{
   char fname[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH],
        vict_name[MAX_INPUT_LENGTH];
      FILE *fp;
      int magicnum, vnum;
      char from[MAX_INPUT_LENGTH];
      struct obj_data* obj;
   
   (void) one_argument(arguments,vict_name);
   
   if (!*vict_name)  {
      send_to_char("This command opens a players mailbox.\n\r"
		   "You must specify a players name!\n\r",ch);
      return;
   }
   
   if (mail_count(vict_name) < 1)   {
      
            send_to_char("There is nothing in storage for them.\n\r",ch);
            return;
   }
   make_mail_filename(vict_name,fname);
  if (!(fp=fopen(fname,"rb")))
     return;  /* either file doesn't exist or theres an error */
  while (1)    {      
     if (!fgets(buf,sizeof(buf),fp))
       break; /* eof or error occured on read at this point */
     sscanf(buf,"%d",&magicnum);
     if (magicnum != MAIL_MAGIC)    {
	sprintf(buf,"MAIL - Invalid Magic Numero in %s file",
		GET_REAL_NAME(ch));
	slog(buf);
        WriteToImmort(buf, TRUST_GRUNT);
	fclose(fp);
        return;
     }
     fgets(from,sizeof(from),fp); /* read line with FROM on it */
     from[strlen(from)-1] = '\0'; /* get rid of newline char */
     fgets(buf,sizeof(buf),fp); /* read line with TO on it */
     fgets(buf,sizeof(buf),fp); /* read line containing time */
     fgets(buf,sizeof(buf),fp); /* read the vnum */
     sscanf(buf,"%d",&vnum);
     obj = make_object(vnum,VIRTUAL);
#ifndef OFFLINE_NOTEBUG
     getsstring(&obj->action_description,fp);
     getdword((long*) &obj->obj_flags.value[0], fp);
     getdword((long*) &obj->obj_flags.value[1], fp);
     getdword((long*) &obj->obj_flags.value[2], fp);
     getdword((long*) &obj->obj_flags.value[3], fp);
     getdword((long*) &obj->obj_flags.timer, fp);
#endif
     sprintf(buf,"You now have %s from %s.\n\r",OBJ_SHORT(obj),from);
     send_to_char(buf,ch);
     obj_to_char(obj,ch);
  } /* end of while loop */

   fclose(fp);
   do_save(ch,"",0);
   return;
}


/* -------------------------------------------------------------------- */
/*                        check for waiting mail                        */
/* -------------------------------------------------------------------- */


void do_mail_check(struct char_data *ch, char *arguments, int cmd)
{
   int i;
   char buf[MAX_STRING_LENGTH];
   
   i = mail_count(GET_REAL_NAME(ch));
   if (i)  {
      sprintf(buf,"$CRYou have %d items waiting in storage.$CN\n\r",i);
      send_to_char_formatted(buf,ch);
   }
   else if (cmd)
     send_to_char("Nope... nothing for you.  Try again tommorow :)\n\r",ch);
}

/* -------------------------------------------------------------------- */
/*            the actual start of the psend procedure                   */
/* -------------------------------------------------------------------- */

void do_mail_send(struct char_data *ch, char *arguments, int cmd)
{
    char itemname[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH];
    struct char_data *vict;
    struct obj_data *item = NULL;
    int muststore;
   
    argument_interpreter(arguments, itemname, vict_name);
   
    /* did they specify an item and vict_name */   
   
    if (!*itemname || !*vict_name)
    {
	send_to_char("Huh? The command is SEND object TO player.\n\r",ch);
	return;
    }

    /* do a level check on character here */

    if (GetMaxLevel(ch) < MAIL_LEVEL && !IS_GOD(ch))
    {
	send_to_char("Sorry, you're not high enough level to send mail (yet).\n\r", ch);
	return;
    }
   
    /* does the player even have the steenking item? */
   
    if (!(item = get_obj_in_list_vis(ch, itemname, ch->carrying)))
    {
	sprintf(buf, "Hmmmm, you have no %s.\n\r", itemname);
	send_to_char(buf, ch);
	return;
    }
   
    /* Lookup vict_name... is this a valid player? */
   
    if (!(vict = find_player_in_world(vict_name)))
    {
	if (!(vict = LoadChar(0, vict_name, READ_PLAYER)))
	{
	    sprintf(buf, "Hey... who are you trying to kid... %s doesn't exist.\n\r",
		    vict_name);
	    send_to_char(buf, ch);
	    return;
	}
	free_char(vict); /* must have successfully loaded so clean up */
	vict = NULL; /* just to make sure */
    }

    /* if we get this far, victim must exist (hopefully)
     * if vict is !null then victim is online, and we can try online delivery
     * if !vict (null) then we have to save object in storage */
   
    /* Check the item parameters */
      
    if ((item->obj_flags.cost > MAX_VALUE) && !(IS_IMMORTAL(ch)))
    {
	send_to_char("Sorry, that item is way to valuable to trust TWex with.\n\r", ch);
	sprintf(buf, "%s tried to mail %s to %s.", GET_REAL_NAME(ch), itemname,
		vict ? GET_REAL_NAME(vict) : vict_name);
	slog(buf);
	return;
    }
   
    /* handle things like corpses, piles of dust */
    /* i hope ITEM_NOTE has a value of at least 1 */
   
    if (item->obj_flags.cost < 0)
    {
	send_to_char("Sorry, I cannot deliver that for you.\n\r", ch);
	return;
    }

    /* is the item cursed?  Guess who can mail people cursed items! */
   
    if (IS_SET(item->obj_flags.extra_flags,ITEM_NODROP) && !IS_IMMORTAL(ch))
    {
	send_to_char("Sure... I'll just bet you want me to deliver a CURSED item for you\n\r", ch);
	return;
    }

    /* not allowed to mail containers around */

    if (GET_ITEM_TYPE(item) == ITEM_CONTAINER)
    {
	send_to_char("Sorry, I cannot mail a container for you.\n\r", ch);
	return;
    } 
   
    /* if the item cannot be rented, then it shouldn't be able to be mailed! */   
    if (item->obj_flags.cost_per_day < 0)
    {
	send_to_char("Sorry, I cannot deliver that for you.\n\r", ch);
	return;
    }
 
    /* if the item is too heavy, the little TWex elf can't carry it */ 

    if (item->obj_flags.weight > MAX_WEIGHT)
    {
	send_to_char("The little TWex delivery elf tries to pick up your package,\n\r"
		     "but its way to heavy.  It frowns at you and disappears in a puff of smoke.\n\r", ch);
	return;
    }
   
    /* can the player even call the TWex? */
   
    if (apply_soundproof(ch))
    {
	send_to_char("You can't do that in a silence zone!\n\r", ch);
	return;
    }
   
    /* check if player (ch) has nuff gold on him to use delivery */   

    if (GET_GOLD(ch)<MAIL_PRICE)
    {
	send_to_char("The TWex delivery elf snoops in your wallet and falls to the floor,\n\r"
		     "laughing.  You don't have enough money!\n\r",ch);
	return;
    }
   
    /* ok done checking if deliverable item */
   
    muststore = MUST_STORE; /* set this flag to force offline storage */

    /* ok go ahead and send item */
   
    if (vict)  /* was set above when we checked if valid player */
    {
	if (vict == ch)
	{
	    send_to_char("Why would you want to mail something to yourself?\n\r",ch);
	    return;
	}
	if ((1 + IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
	    muststore = TRUE; /* victim has too many items, must store */

	if ((GET_POS(vict) == POSITION_FIGHTING) ||
	    (GET_POS(vict) < POSITION_SLEEPING))
	{
	    send_to_char("Hmmm... I don't think now is such a good time to deliver that.\n\r"
			 "I'll just put it in storage for now.\n\r",ch);
	    muststore = TRUE;
	}
	if (IS_SET(vict->specials.mob_act,ACT_LIQUID))
	    muststore = TRUE; /* victim is liquid, must store */

	if ((GET_OBJ_WEIGHT(item)+IS_CARRYING_W(vict)) > CAN_CARRY_W(vict))
	    muststore = TRUE; /* delivery is too heavy */

	if (apply_soundproof(vict))
	    muststore = TRUE; /* victim is in a silence zone */

	if (!IsLevelOk(vict,item))
	{
	    sprintf(buf,"Sorry %s is WAY to powerful for %s to receive\n\r",
		    itemname, vict_name);
	    send_to_char(buf, ch);
	    return;
	}
       
	if (muststore)   /* we should still notify the victim! */
	    send_to_char("You have a package waiting in storage.\n\r", vict);
      
	if (!muststore)
	{
	    obj_from_char(item);
	    obj_to_char(item, vict);
	    GET_GOLD(ch) -= MAIL_PRICE; /* copied from do_give....?????? */
	    send_to_char("You summon the TWex delivery elf who appears in a screech of dust\n\r", ch);
	    sprintf(buf, "You give her %s and %d coins.\n\r", OBJ_SHORT(item),
		    MAIL_PRICE);
	    send_to_char(buf, ch);
	    send_to_char("She smiles and vanishes.\n\r", ch);
	    act("$n mails $p to $N.", 1, ch, item, vict, TO_NOTVICT);
	    act("$n receives $p from $N.", 1, vict, item, ch, TO_NOTVICT);
	    send_to_char("With a poof, the TWex delivery elf appears out of nowhere\n\r", vict);
	    /* 	 MOBTrigger=FALSE; */
	    act("The elf smiles and hands you $p from $n.",
		0, ch, item, vict, TO_VICT);
	    send_to_char("Sighing at its workload and pay scale, the elf wanders off.\n\r", vict);
	    do_save(ch, "", 0);
	    do_save(vict,"",0);	 
	 
	    sprintf(buf, "%s(%d) SENDS %s to %s ONLINE.", GET_REAL_NAME(ch),
		    GetMaxLevel(ch), OBJ_SHORT(item), GET_REAL_NAME(vict));
	    slog(buf);
	    return; /* all done! the online mailer */
	} /* end of handler if victim is online */
    }

    /* here is where we check the MAX_MAIL_PER_PLAYER limit */
    if ((mail_count(vict_name) >= MAX_MAIL_PER_PLAYER) &&
	(!IS_IMMORTAL(ch)))
    {
	send_to_char("Sorry, I think that player has enough waiting for him...\n\r",ch);
	return;
    }
    obj_from_char(item); /* take the item out of inventory */

    /* remember if we got this far, we tried a loadchar, and */
    /* it was successful, but we set the pointer to NULL to */
    /* flag an offline character... hence can't GET_REAL_NAME(vict) */

    if (write_mail(item, ch, vict_name))
    {      
	sprintf(buf, "ERROR UNABLE TO write_mail from %s(%d) to %s",
		GET_REAL_NAME(ch), GetMaxLevel(ch), vict_name);
	slog(buf);
	send_to_char("Sorry, I am unable to store mail at this time.\n\r", ch);
	obj_to_char(item, ch); /* give the object back */
    } 
    else
    {
	sprintf(buf,
		"The TWex delivery elf drops out of the sky and smiles.\n\r"
		"You give her %d coins, and a %s.\n\r"
		"She smiles and melts into the earth.\n\r",
		MAIL_PRICE, OBJ_SHORT(item));
	send_to_char(buf, ch);
	extract_obj(item);
	do_save(ch, "", 0);
	sprintf(buf, "%s(%d) just mailed %s to %s", GET_REAL_NAME(ch),
		GetMaxLevel(ch), OBJ_SHORT(item), vict_name);
	stat_log(buf, 0);
	/* important... do I need to FREE(ITEM)? */
    }
}
