#include "config.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "board.h"
#include "multiclass.h"
#include "utility.h"
#include "modify.h"
#include "handler.h"
#include "util_str.h"
#include "spec.h"
#include "proto.h"

/* forward declarations */
void board_load_board(struct Board *b);
int board_show_board(struct char_data *ch, char *arg, struct Board *b);
void board_write_msg(struct char_data *ch, char *arg, struct Board *b);
int board_display_msg(struct char_data *ch, char *arg, struct Board *b);
int board_remove_msg(struct char_data *ch, char *arg, struct Board *b);
void board_fix_long_desc(struct Board *b);
void error_log(const char *str);
void board_reset_board(struct Board *b);


int OpenBoardFile(struct Board *b);
int CloseBoardFile(struct Board *b);

struct Board *board_list;


/***************************************************************************
 * Boards must be ITEM_BOARD.  The values[0..3] are used for controlling
 * who can use the board.
 *
 * value[0] - Mininum read level
 * value[1] - Mininum write level
 * value[2] - Mininum remove level
 * value[3] - Guild (0 for none)
 *
 **************************************************************************/

void InitBoards(void)
{
  /*
   *  this is called at the very beginning, like shopkeepers
   */

  /*
   * ON OPENING AND CLOSING BOARD FILES:
   * OpenBoardFile() calls fopen() on a filesystem file. CloseBoardFile()
   * calls fclose() on the same file.  It is important that we don't fclose()
   * a file that hasn't been opened. Thus:
   * 
   * It is your responsibility to open and close a board  when you want to
   * write to it.  Do open a board for a function you call.  Do not close
   * a board for a function you call.  If you open a board, close it.
   * 
   * If you have questions.  Ask me.  -smw
   */

    board_list = 0;
}

struct Board* InitABoard(struct obj_data *obj)
{
    struct Board *next, *tmp;
  
    if (board_list)
    {
	/*
	 **  try to match a board with an existing board in the game
	 */
	for (tmp = board_list; tmp; tmp = tmp->next)
	{
	    if (tmp->Rnum == obj->item_number)
		return tmp;
	}
    }
  
    CREATE(next, struct Board, 1);
  
    next->Rnum = obj->item_number;
    
    /* set up mininum levels */
    next->min_read = obj->obj_flags.value[0];
    next->min_write = obj->obj_flags.value[1];
    next->min_remove = obj->obj_flags.value[2];
    next->guild = obj->obj_flags.value[3];
  
    sprintf(next->filename, "%d.messages", obj_index[obj->item_number].virt);

    board_load_board(next);
  
    /*
     **  add our new board to the beginning of the list
     */
  
    tmp = board_list;
    next->next = tmp;
    board_list = next;
  
    return next;
}

int OpenBoardFile(struct Board *b)
{
  if ( !(b->file = fopen(b->filename, "r+") ))
    if( !(b->file= fopen(b->filename, "w") ))
    {
      perror("OpenBoardFile(fopen)");
      return FALSE;
    }
    else
    {
      fclose(b->file);
      if ( !(b->file = fopen(b->filename, "r+") ))
      {
	perror("OpenBoardFile(fopen)");
	return FALSE;
      }
	  
    }
              
  if (!b->file) {
    perror("OpenBoardFile(fopen)");
  }
  return TRUE;
}

int CloseBoardFile(struct Board *b)
{
  if ( !(b->file) ) {
    perror("board.c: CloseBoardFile -- no file to close");
    return FALSE;
  }
  
  fclose(b->file);
  return TRUE; 
}









struct Board *FindBoardInRoom(int room)
{
    struct obj_data *o;
    struct Board *nb;
  
    if (!real_roomp(room)) return(NULL);
  
    for (o = real_roomp(room)->contents ; o ; o = o->next_content)
    {
	if (obj_index[o->item_number].func == board)
	{
	    for (nb = board_list; nb; nb = nb->next)
	    {
		if (nb->Rnum == o->item_number)
		    return(nb);
	    }

	    return InitABoard(o);
	}
    }
    return(NULL);
}

SPECIAL(board)
{
    struct Board *nb;

    if (type == SPEC_INIT)
	return (FALSE);
  
    switch(cmd)
    {
    case 15:
    case 149:
    case 63:
    case 66:
	break;

    default:
	return FALSE;
    }
    
    nb = FindBoardInRoom(ch->in_room);
  
    if (!nb) return(FALSE);
  
    if (!ch->desc)
	return(FALSE); 

    /* TODO: for consitency this really should be placed with the
     * other error messages in board_write_msg, etc.  OR we should
     * move the other code here.  -smw
     */
    if (nb->guild != 0 && (GET_GUILD(ch) != nb->guild) )  /* for clarity */
    {
	send_to_char("This board is all written with a secret code which you can not understand.\n\r", ch);
	return TRUE;
    }

    switch (cmd) {
    case 15:			/* look */
	return(board_show_board(ch, arg, nb));
    case 149:			/* write */
	board_write_msg(ch, arg, nb);
	return 1;
    case 63:			/* read */
	return(board_display_msg(ch, arg, nb));
    case 66:			/* remove */
	return(board_remove_msg(ch, arg,nb));
    }

    return FALSE;
}


void board_write_msg(struct char_data *ch, char *arg, struct Board *b) 
{
    long ct;
    char *tmstr;

    if (b->msg_num > MAX_MSGS - 1) {
	send_to_char("The board is full already.\n\r", ch);
	return;
    }

    if (b->min_write > TRUST(ch))
    {
	send_to_char("Your pitiful powers can not put a mark on this board.\n\r", ch);
	return;
    }

    if (b->writer) {
	send_to_char("Sorry, but someone has stolen the pen.. wait a few minutes.\n\r",ch);
	return;
    }
  
    /* skip blanks */
  
    for(; isspace(*arg); arg++);
  
    if (!*arg) {
	send_to_char("We must have a headline!\n\r", ch);
	return;
    }
  
    b->writer = ch;
    ch->board = b;
  
    ct = time(0);
    tmstr = ctime(&ct);
    tmstr += 4;
    tmstr[12] = 0;

    /* +7 is for a space and '()' around the character name and the " > ". */
    CREATE(b->head[b->msg_num], char,
	   strlen(arg) + strlen(GET_REAL_NAME(ch)) + strlen(tmstr) + 8);
    sprintf(b->head[b->msg_num], "/%s - %s/  %s",tmstr,GET_REAL_NAME(ch),arg);
    
    b->msgs[b->msg_num] = NULL;
  
    send_to_char("Write your message. Terminate with an @ at the beginning.\n\r \
	of a blank line.\n\r\n\r", ch);
    act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);
  
    ch->desc->str = &b->msgs[b->msg_num];
    ch->desc->max_str = MAX_MESSAGE_LENGTH;
  
    b->msg_num++;
}


int board_remove_msg(struct char_data *ch, char *arg, struct Board *b) 
{
    int ind, msg;
    char buf[256], number[MAX_INPUT_LENGTH];
  
    one_argument(arg, number);
  
    if (!*number || !isdigit(*number))
	return(0);
    if (!(msg = atoi(number))) return(0);
    if (!b->msg_num) {
	send_to_char("The board is empty!\n\r", ch);
	return(1);
    }
    if (msg < 1 || msg > b->msg_num) {
	send_to_char("That message exists only in your imagination..\n\r",
		     ch);
	return(1);
    }
  
    if (TRUST(ch) < b->min_remove) {
	send_to_char("You try with all your might to remove a message but it won't budge.\n\r", ch);
	act("$n tugs on a message on the board but can not manage to remove it.", TRUE, ch, NULL, NULL, TO_ROOM);
	return TRUE;
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
    send_to_char("Message removed.\n\r", ch);
    sprintf(buf, "$n just removed message %d.", msg);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    board_save_board(b);
  
    return(1);
}

void board_save_board(struct Board *b) 
{
    int ind, len;
  
    if (!b) return;
    if (! OpenBoardFile(b) ) return;
  
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
    CloseBoardFile(b);
    board_fix_long_desc(b);
}

void board_load_board(struct Board *b) 
{
    int ind, len = 0;
  
    if(b->loaded)
	return;
    
    if (! OpenBoardFile(b) ) return;
    board_reset_board(b);
  
    if(b->file)
    {
	fread(&b->msg_num, sizeof(int), 1, b->file);
  
	if (b->msg_num < 1 || b->msg_num > MAX_MSGS || feof(b->file)) {
	    error_log("Board-message file corrupt or nonexistent.\n\r");
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

void board_reset_board(struct Board *b) 
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
    board_fix_long_desc(b);
}

void error_log(const char *str) 
{				/* The original error-handling was MUCH */
    fputs("Board : ", stderr);	/* more competent than the current but  */
    fputs(str, stderr);		/* I got the advice to cut it out..;)   */
}

int board_display_msg(struct char_data *ch, char *arg, struct Board *b) 
{
    char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
    int msg;
  
    one_argument(arg, number);
    if (!*number || !isdigit(*number))
	return(0);
    if (!(msg = atoi(number))) return(0);

    if (b->min_read > TRUST(ch))
    {
	send_to_char("For some reason, you can't understand anything on the board.\r\n", ch);
	act("$n looks at the board and gets a puzzled look on $s face.",
	    FALSE, ch, NULL, NULL, TO_ROOM);
	return 1;
    }

    if (!b->msg_num) {
	send_to_char("The board is empty!\n\r", ch);
	return(1);
    }
    if (msg < 1 || msg > b->msg_num) {
	send_to_char("That message exists only in your imagination..\n\r",
		     ch);
	return(1);
    }
  
    sprintf(buf, "$n reads message %d titled : %s.",
	    msg, b->head[msg - 1]);
    act(buf, TRUE, ch, 0, 0, TO_ROOM); 
  
    /* Bad news */
  
    sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, b->head[msg - 1],
	    b->msgs[msg - 1]);
    page_string(ch->desc, buffer, 1);
    return(1);
}



void board_fix_long_desc(struct Board *b) 
{
}


int board_show_board(struct char_data *ch, char *arg, struct Board *b)
{
    int i;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];
  
    one_argument(arg, tmp);
  
    if (!*tmp || !isname(tmp, "board bulletin"))
	return(0);

    if (b->min_read > TRUST(ch))
    {
	send_to_char("For some reason, you can't understand anything on the board.\r\n", ch);
	act("$n looks at the board and gets a puzzled look on $s face.",
	    FALSE, ch, NULL, NULL, TO_ROOM);
	return 1;
    }

    if (b->writer) {
	send_to_char("Sorry, but someone is writing a message\n\r",ch);
	return(1);
    }
  
    act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);
  
    strcpy(buf,
	   "This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\n\r");
    if (!b->msg_num) {
	strcat(buf, "The board is empty.\n\r");
    } else {
	sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
		b->msg_num);
	for (i = 0; i < b->msg_num; i++)
	    sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, b->head[i]);
    }
    page_string(ch->desc, buf, 1);
  
    return(1);
}
