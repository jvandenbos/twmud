#include "config.h"
#include "structs.h"
#include "spec.h"
#include "spell_util.h"
#include "sstring.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "act.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "board.h"
#include "multiclass.h"
#include "utility.h"
#include "utils.h"
#include "modify.h"
#include "handler.h"

#include "util_str.h"

struct response 
{
  int msg;
  char *mesg;
  struct char_data *ch;
};

/* forward declarations */
int respond(struct char_data *ch, int cmd, char *arg);
int mobile_show_response(struct char_data *ch, char *arg, struct Board *b,
			 struct char_data *mob);
int mobile_display_response(struct char_data *ch, char *arg, struct Board *b);
void mobile_check_response(struct char_data *ch, char *arg, struct Board *b,
			   struct response *resp);
int mobile_say_response(struct char_data *mob, struct response *resp);
void mobile_write_response(struct char_data *ch, char *arg, struct Board *b);
int mobile_remove_response(struct char_data *ch, char *arg, struct Board *b);

void mobile_reset_response(struct Board *b);
void mobile_save_responses(struct Board *b);
void mobile_load_responses(struct Board *b);
void response_error_log(const char *str);
void OpenResponseFile(struct Board *b);

struct Board *response_list;

void InitMobileResponses(void)
{
    /*
     **  this is called at the very beginning, like shopkeepers
     */
    response_list = 0;
}

struct Board* InitAResponse( struct char_data *mob)
{
    struct Board *next, *tmp;
  
    if (response_list)
    {
	/*
	 **  try to match a mobile with an existing responses in the game
	 */
	for (tmp = response_list; tmp; tmp = tmp->next)
	{
	    if (tmp->Rnum == mob->nr)
		return tmp;
	}
    }
  
    CREATE(next, struct Board, 1);
  
    next->Rnum = mob->nr;
  
    sprintf(next->filename, "%d.responses", mob_index[mob->nr].virt);

    OpenResponseFile(next);
  
    mobile_load_responses(next);
  
    /*
     **  add our new board to the beginning of the list
     */
  
    tmp = response_list;
    next->next = tmp;
    response_list = next;
  
    fclose(next->file);

    return next;
}

void OpenResponseFile(struct Board *b)
{
  if ( !(b->file = fopen(b->filename, "r+") ))
    if( !(b->file=fopen(b->filename, "w") ))
    {
      perror("OpenResponseFile(fopen)");
      return;
    }
    else
    {
      fclose(b->file);
      if ( !(b->file = fopen(b->filename, "r+") ))
      {
	perror("OpenResponseFile(fopen)");
	return;
      }
	  
    }
              
  if (!b->file) {
    perror("OpenResponseFile(fopen)");
  }
}

struct Board *FindResponseInTable(struct char_data *mob)
{
    struct Board *nb;
  
    if (mob)
    {
      for (nb = response_list; nb; nb = nb->next)
      {
	if (nb->Rnum == mob->nr)
	  return(nb);
      }

      return InitAResponse(mob);

    }
    return(NULL);
}

int respond(struct char_data *ch, int cmd, char *arg)
{
    struct Board *nb;
    struct char_data *mob;
    struct response *resp;
    
    switch(cmd)
    {
    case 0:
      if (ch->act_ptr==0)
	return FALSE;
      else
	if ( ((struct response *)ch->act_ptr)->msg == 0)
	  return FALSE;
    case 17:   /* say */
    case 169:  /* abbreviated say */
      break;
    case 63:   /* read */
    case 66:   /* remove */
    case 149:  /* write */
    case 166:  /* examine */
      if (!IS_IMMORTAL(ch))
	return (FALSE);
      break;

    default:
	return FALSE;
    }
    mob=FindMobInRoomWithFunction(ch->in_room, respond);

    if (mob->act_ptr==0) {	
	CREATE(resp, struct response, 1);
	mob->act_ptr = (int) resp;
	resp->msg=0;
    } else
	resp = (struct response *)mob->act_ptr;
    
    nb = FindResponseInTable(mob);
  
    if (!nb) return(FALSE);

    if ( (cmd) && (!ch->desc) ) return (FALSE);
  
    switch (cmd) {
    case 0:
      return(mobile_say_response(ch,resp));
    case 17:	/* say */
    case 169:   /* abbreviated say */
      mobile_check_response(ch,arg,nb,resp);
      return(0);
    case 63:			/* read */
	return(mobile_display_response(ch, arg, nb));
    case 66:
      return(mobile_remove_response(ch, arg,nb));
    case 149:	/* write */
      mobile_write_response(ch, arg, nb);
      return 1;
    case 166:    /* examine */
	return(mobile_show_response(ch, arg, nb, mob));
    }
}


void mobile_load_responses(struct Board *b) 
{
    int ind, len = 0;
  
    if(b->loaded)
	return;
    
    OpenResponseFile(b);
    mobile_reset_response(b);
  
    if(b->file)
    {
	fread(&b->msg_num, sizeof(int), 1, b->file);
  
	if (b->msg_num < 1 || b->msg_num > MAX_MSGS || feof(b->file)) {
	    response_error_log("Response-message file corrupt or nonexistent.\n\r");
	    fclose(b->file);
	    return;
	}
	for (ind = 0; ind < b->msg_num; ind++) {
	    fread(&len, sizeof(int), 1, b->file);
	    CREATE(b->head[ind], char, len + 1);
	    fread(b->head[ind], sizeof(char), len,b->file);
	    b->head[ind][len] = 0;

	    fread(&len, sizeof(int), 1, b->file);
	    CREATE(b->msgs[ind], char, len + 1);
	    fread(b->msgs[ind], sizeof(char), len, b->file);
	    b->msgs[ind][len] = 0;
	}
	fclose(b->file);
    }
    else
	b->msg_num = 0;
	
    b->loaded = TRUE;
    
    board_fix_long_desc(b);
}

void mobile_reset_response(struct Board *b) 
{
    int ind;
  
    for (ind = 0; ind < MAX_MSGS; ind++) {
	if (b->head[ind])
	    FREE(b->head[ind]);
	if (b->msgs[ind])
	    FREE(b->msgs[ind]);
	b->head[ind] = b->msgs[ind] = NULL;
    }
    b->msg_num = 0;
    b->loaded = FALSE;
}

void response_error_log(const char *str) 
{				 /* The original error-handling was MUCH */
    fputs("Response : ", stderr);/* more competent than the current but  */
    fputs(str, stderr);		 /* I got the advice to cut it out..;)   */
}

int mobile_show_response(struct char_data *ch, char *arg, struct Board *b,
			 struct char_data *mob)
{
    int i;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];
  
    one_argument(arg, tmp);
  
    if (!*tmp || !isname(tmp, ss_data(mob->player.name)))
	return(0);
  
    if (!b->msg_num) {
	sprintf(buf, "The response table is empty.\n\r");
    } else {
	sprintf(buf, "There are %d responses on this mobile.\n\r",
		b->msg_num);
	for (i = 0; i < b->msg_num; i++)
	    sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, b->head[i]);
    }
    page_string(ch->desc, buf, 1);
  
    return(1);
}

void mobile_write_response(struct char_data *ch, char *arg, struct Board *b) 
{
    long ct;
    char *tmstr;

    if (b->msg_num > MAX_MSGS - 1) {
	send_to_char("The response table is full already.\n\r", ch);
	return;
    }
  
    if (b->writer) {
	send_to_char("Sorry, but someone has stolen the mind probe.. wait a few minutes.\n\r",ch);
	return;
    }
  
    /* skip blanks */
  
    for(; isspace(*arg); arg++);
  
    if (!*arg) {
	send_to_char("We must have a trigger!\n\r", ch);
	return;
    }

    arg=lower(arg);
    
    b->writer = ch;
    ch->board = b;
  
    /* +7 is for a space and '()' around the charresper name and the " > ". */
    CREATE(b->head[b->msg_num], char,
	   strlen(arg)+1);
  
  
    sprintf(b->head[b->msg_num], "%s",arg);
    b->msgs[b->msg_num] = NULL;
  
    send_to_char("Write your response. Terminate with an @ at the beginning.\n\r \
	of a blank line.\n\r\n\r", ch);
    act("$n starts to play god.", TRUE, ch, 0, 0, TO_ROOM);
  
    ch->desc->str = &b->msgs[b->msg_num];
    ch->desc->max_str = MAX_MESSAGE_LENGTH;
  
    b->msg_num++;
}


int mobile_remove_response(struct char_data *ch, char *arg, struct Board *b) 
{
    int ind, msg;
    char buf[256], number[MAX_INPUT_LENGTH];
  
    one_argument(arg, number);
  
    if (!*number || !isdigit(*number))
	return(0);
    if (!(msg = atoi(number))) return(0);
    if (!b->msg_num) {
	send_to_char("The response table is empty!\n\r", ch);
	return(1);
    }
    if (msg < 1 || msg > b->msg_num) {
	send_to_char("That response exists only in your imagination..\n\r",
		     ch);
	return(1);
    }
  
    ind = msg - 1;
    FREE(b->head[ind]);
    if (b->msgs[ind] && *b->msgs[ind])
	FREE(b->msgs[ind]);
    for (; ind < b->msg_num -1; ind++) {
	b->head[ind] = b->head[ind + 1];
	b->msgs[ind] = b->msgs[ind + 1];
    }
    b->msg_num--;
    send_to_char("Response removed.\n\r", ch);
    mobile_save_responses(b);
  
    return(1);
}


void mobile_save_responses(struct Board *b) 
{
    int ind, len;
  
    if (!b) return;
  
    OpenResponseFile(b);
  
    fwrite(&b->msg_num, sizeof(int), 1, b->file);
    for (ind = 0; ind < b->msg_num; ind++) {
	len = strlen(b->head[ind]) + 1;
	fwrite(&len, sizeof(int), 1, b->file);
	fwrite(b->head[ind], sizeof(char), len, b->file);
	if (!b->msgs[ind]) {
	    CREATE(b->msgs[ind], char, 50);
	}
	len = strlen(b->msgs[ind]) + 1;
	fwrite(&len, sizeof(int), 1, b->file);
	fwrite(b->msgs[ind], sizeof(char), len, b->file);
    }
    fclose(b->file);
}


int mobile_display_response(struct char_data *ch, char *arg, struct Board *b) 
{
    char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
    int msg;
  
    one_argument(arg, number);
    if (!*number || !isdigit(*number))
	return(0);
    if (!(msg = atoi(number))) return(0);
    if (!b->msg_num) {
	send_to_char("The response table is empty!\n\r", ch);
	return(1);
    }
    if (msg < 1 || msg > b->msg_num) {
	send_to_char("That response exists only in your imagination..\n\r",
		     ch);
	return(1);
    }
  
    sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, b->head[msg - 1],
	    b->msgs[msg - 1]);
    page_string(ch->desc, buffer, 1);
    return(1);
}


void mobile_check_response(struct char_data *ch, char *arg, struct Board *b,
			   struct response *resp)
{
  int i, found;
  char *buf, *wstr, *buf2;
  
  /*  If no messages, we can stop now */
  if (!b->msg_num)
    return;

  /* Prepare what was said -- lower case and secondary storage */
  CREATE(buf2,char,strlen(arg)+1);
  strcpy(buf2,arg);
  buf=buf2;
  buf2=skip_spaces(lower(buf2));  

  /* Get the first word */
  wstr=strsep(&buf2," ");
  
  /* compare against triggers until match or no more words */
  found = -1;
  while ((wstr) && (found < 0))
  {
    for(i=0; i < b->msg_num; i++)
      if (!strcmp(wstr,b->head[i]))
      {
        found=i;
	break;
      }
    wstr=strsep(&buf2," ");
  }

  FREE(buf);

  /* if i == msg_num, then not found */
  if (found < 0)
    return;

  /* Setup response structure */
  CREATE(buf,char,strlen(b->msgs[found])+1);
  strcpy(buf,b->msgs[found]);
  resp->msg=1;
  resp->ch=ch;
  resp->mesg=buf;
}

int mobile_say_response(struct char_data *mob, struct response *resp)
{
  char *wstr, *msg;
  char buf[MAX_STRING_LENGTH];
  
  /* If character has left the game, then don't reply */
  if (resp->ch!=NULL)
  { 
    /* Prepare string by making a secondary pointer, and separating the
       first line */
    msg=resp->mesg;
    wstr=strsep(&msg,"\r");

    /* display each line with act() and continue until no more lines */
    while (wstr)
    {
      sprintf(buf,"%s\r",wstr); /* have to tack the \r back on as we */
				/*   erased it with the strsep	     */
      act(buf, FALSE, mob, 0, resp->ch, TO_ROOM|TO_VICT);
      wstr=strsep(&msg,"\r");
    }
  }
    
  /* Clear reply structure */
  resp->msg=0;
  FREE(resp->mesg);
  resp->ch=NULL;
  return TRUE;
}

