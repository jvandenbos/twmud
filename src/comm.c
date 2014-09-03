/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.                                               *
************************************************************************* */

#include "config.h"
#include "constants.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#ifdef USE_sys_file
#include <sys/file.h>
#else
#include <fcntl.h>
#endif
#include <signal.h>
#include <sys/time.h>

#ifdef HAVE_inet_ntoa
#include "inet.h"
#endif

#if USE_stdlib
#include <stdlib.h>
#endif
#if USE_unistd
#include <unistd.h>
#endif
#include <stdarg.h>
    
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "state.h"
#include "utility.h"
#include "whod.h"
#include "events.h"
#include "editor.h"
#include "page.h"
#include "weather.h"
#include "handler.h"
#include "fight.h"
#include "spell_util.h"
#include "modify.h"
#include "multiclass.h"
#include "comm.h"
#include "signals.h"
#include "board.h"
#include "util_str.h"
#include "act.h"
#include "recept.h"
#include "interpreter.h"
#include "statistic.h"
#include "ident.h"
#include "trackchar.h"

#include "ansi.h"
#include "proto.h"


/* following stuff added to accommodate telnet echo on */

#include <arpa/telnet.h>
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };



#define MAX_HOSTNAME   256

#define STATE(d) ((d)->connected)

AUCTION_DATA *auction = NULL;
VOTE_DATA *vote = NULL;
NOMINEE_DATA *nomi = NULL;

int connected = 0;
int max_connected = 0;
bool MOBTrigger = TRUE;


int maxdesc, avail_descs, real_avail_descs;
list_head descriptor_list;
int slow_death = 0;     /* Shut her down, Martha, she's sucking mud */
int goaway = 0;       /* clean shutdown */
int lc_reboot = 0;         /* reboot the game after a shutdown */
long Uptime;            /* time that the game has been up */

long bytes_written = 0;		/* total bytes written */
long bytes_read = 0;		/* total bytes read */

int pulse;

int tics = 0;        /* for extern checkpointing */

int slownames = SLOWNAMES;

extern struct fedit_data fedit;

/* extern declaration for something that should be in stdlib.h... */
#ifdef __cplusplus
extern "C"
{
#endif
    int strcasecmp(const char* a, const char* b);
#ifdef __cplusplus
};
#endif

/* forward declarations */
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d, int show_msg);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);
void save_all(void);
void player_prompt(struct descriptor_data* point);
void mobile_prompt(struct descriptor_data* point);
void mortal_prompt(struct descriptor_data* point);
void immortal_prompt(struct descriptor_data* point);
void beware_lightning();


/* ******************************************************************
*  general utility stuff (for local use)									 *
****************************************************************** */



void input_to(struct char_data *ch,
		     void (*f)(struct char_data*, char *))
{
  ch->desc->input_fun = f;
}


int get_from_q(struct txt_q *queue, char *dest, int max)
{
	struct txt_block *tmp;

 	/* Q empty? */
	if (!queue->head)
		return(0);

	tmp = queue->head;
	if((int) strlen(tmp->text) > (max - 1))
	{
	    log_msg("Line in queue too long... truncated");
	    tmp->text[max - 1] = 0;
	}
	strcpy(dest, tmp->text);
	queue->head = tmp->next;
	if(!queue->head)
	    queue->tail = NULL;

	FREE(tmp->text);
	FREE(tmp);

	return(1);
}

void write_to_q(const char *txt, struct txt_q *queue)
{
	struct txt_block *n;

        if (!queue) {
	  log_msg("Output message to non-existant queue");
	  return;
	}

	CREATE(n, struct txt_block, 1);
	n->text = strdup(txt);

       	n->next = NULL;

	/* Q empty? */
	if (!queue->head)  {
		queue->head = queue->tail = n;
	} else	{
		queue->tail->next = n;
		queue->tail = n;
	}
}
		












/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
	char dummy[MAX_STRING_LENGTH];

	while (get_from_q(&d->output, dummy, sizeof(dummy)));
	while (get_from_q(&d->input, dummy, sizeof(dummy)));
}


/* allows one to fix typo's using ^ just like in unix */
char *caret_fix(char *prev, char *curr)
{
    char *end, *mark, buf[MAX_INPUT_LENGTH * 2 + 1];

    if ((end = strchr(++curr, '^')) == NULL)
      return NULL;

    *end = '\0';

    if ((mark = strstr(prev, curr)) == NULL)
      return NULL;

    strncpy(buf, prev, mark - prev);
    buf[mark - prev] = '\0';
    strcat(buf, end + 1);
    strcat(buf, mark + (end - curr));
    buf[MAX_INPUT_LENGTH - 1] = '\0';
    strcpy(prev, buf);

    return curr;
}




/* ******************************************************************
*  socket handling							 *
****************************************************************** */




int init_socket(int port)
{
    int s, done;
    int opt1;
    char hostname[MAX_HOSTNAME+1];
    struct sockaddr_in sa;
    struct hostent *hp;
    struct linger ld;

    bzero(&sa, sizeof(struct sockaddr_in));
    gethostname(hostname, MAX_HOSTNAME);
    if(!(hp = gethostbyname(hostname)))
    {
	perror("gethostbyname");
	exit(1);
    }

    sa.sin_family = hp->h_addrtype;
    sa.sin_port	= htons(port);

    if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("Init-socket");
	exit(1);
    }
   
    opt1 = 1;
   
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt1, sizeof(opt1)) < 0)
    {
	perror ("setsockopt REUSEADDR");
	exit (1);
    }
   
    ld.l_onoff = 0;
    ld.l_linger = 1000;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)	{
	perror("setsockopt LINGER");
	exit(1);
    }

    done = 6;
    while (1) {
	if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)	{
	    perror("bind");
	    if(--done < 0)
	    {
		close(s);
		exit(1);
	    }
	    sleep(10);
	}
	else
	    break;
    }

    listen(s, 5);
    return(s);
}


int new_connection(int s, u_short *peer_port)
{
	struct sockaddr_in isa;
	/* struct sockaddr peer; */
	socklen_t i;
	int t;

	i = sizeof(isa);
	getsockname(s, (struct sockaddr *) &isa, &i);


	if ((t = accept(s, (struct sockaddr *) &isa, &i)) < 0)
	{
	    perror("Accept");
	    return(-1);
	}

	peer_port = &isa.sin_port;
	nonblock(t);

	/*

	i = sizeof(peer);
	if (!getpeername(t, (struct sockaddr *) &peer, &i))
	{
		*(peer.sa_data + 49) = '\0';
		sprintf(buf, "New connection from addr %s.\n", peer.sa_data);
		log_msg(buf, LOG_CONNECT);
	}

	*/

	return(t);
}

#ifndef HAVE_inet_ntoa
char* inet_ntoa(struct in_addr addr)
{
    static char buf[64];
    
    sprintf(buf,"%d.%d.%d.%d",
	    addr.S_un.S_un_b.s_b1,
	    addr.S_un.S_un_b.s_b2,
	    addr.S_un.S_un_b.s_b3,
	    addr.S_un.S_un_b.s_b4);
}
#endif

/* print an internet host address prettily */
static void printhost(struct in_addr* addr, char* buf)
{
  struct hostent       	*h = NULL;
  const char		*s;

  if (!slownames)
      h = gethostbyaddr((char *) addr, sizeof(*addr),AF_INET);
  s = (h==NULL) ? (char *) NULL : h->h_name;

  strcpy(buf, s ? s : inet_ntoa(*addr));
}

int wildcmp(const char *s1, const char *s2)
{
    while(*s1 && *s2)		/* while we have both strings */
    {
	if(*s1 == '*')		/* wildcard? */
	{
	    if(s1[1] == 0)	/* end of string matches everything else */
		return 1;
	    if(s1[1] == '.')	/* look for more tokens */
	    {
		s1++;		/* skip the * */
		while(s2 && (*s2 != '.')) /* skip the source chars */
		    s2++;
	    }
	}
	if(isalpha(*s1) ? tolower(*s1) != tolower(*s2) : *s1 != *s2)
	    return 0;
	s1++, s2++;
    }

    return !*s1 && !*s2;	/* same if we ran out of both at
				   the same time */
} /* end wildcmp */

int new_descriptor(int s)
{
    int desc;
    socklen_t size;
    socklen_t i;
    struct descriptor_data *newd;
    struct sockaddr_in sock;
    struct sockaddr_in isa;
    char tempbuf[255], buf2[256];
    char temphost[255];
  
    i = sizeof(isa);
    if ((desc = accept(s, (struct sockaddr *) &isa, &i)) < 0)
    {
	perror("Accept");
	return(-1);
    }

    nonblock(desc);

    if ((maxdesc + 1) >= avail_descs)
    {
	sprintf(tempbuf,"Debug Info: maxdesc=%d,avail descs=%d,desc=%d",
		maxdesc,avail_descs,desc);
	log_msg(tempbuf);
	write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
	close(desc);
	return(0);
    }
    else if (desc > maxdesc)
	maxdesc = desc;
  
    /* find info */
    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
    {
	perror("getpeername");
	close(desc);		/* Refuse to accept, no address */
	return 0;
    }
    else
    {
      printhost(&sock.sin_addr, temphost);

      /* determine if the site is banned */
      if (isbanned(temphost) == BAN_ALL) {
	close(desc);
	sprintf(buf2, "Connection attempt denied from [%s]", temphost);
	log_msg(buf2, LOG_CONNECT);
	return 0;
      }
    }
  
    CREATE(newd, struct descriptor_data, 1);
	
    strcpy(newd->host, temphost);
    sprintf(buf2, "New connection from [%s]", newd->host);
    slog(buf2);

    /* init desc data */
    newd->descriptor = desc;
    newd->peer_port =  isa.sin_port;
    newd->connected  = 1;
    newd->wait = 1;
    newd->prompt_mode = 0;
    *newd->buf = '\0';
    newd->str = 0;
    newd->showstr_head = 0;
    newd->showstr_point = 0;
    *newd->last_input= '\0';
    newd->output.head = NULL;
    newd->input.head = NULL;
    newd->character = 0;
    newd->snoop.snooping = 0;
    newd->snoop.snoop_by = 0;
    newd->sstr = NULL;

    /* prepend to list */
    list_push(&descriptor_list, &newd->link);
  
    load_next_greet();

    SEND_TO_Q(greeting, newd);
    SEND_TO_Q("Please wait", newd);
    ident_start(newd, sock.sin_addr.s_addr);

    newd->input_fun = NULL;
  
    return(0);
}

int process_output(struct descriptor_data *t)
{
	char i[MAX_STRING_LENGTH + 1];

	if (!t->prompt_mode && !t->connected)
		if (write_to_descriptor(t->descriptor, "\n\r") < 0)
			return(-1);


	/* Cycle thru output queue */
	while (get_from_q(&t->output, i, sizeof(i)))	{  
		if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
			write_to_q("% ",&t->snoop.snoop_by->desc->output);
			write_to_q(i,&t->snoop.snoop_by->desc->output);
		}
		if (write_to_descriptor(t->descriptor, i))
			return(-1);
	}
	
	if(!t->connected && t->character &&
	   !IS_SET(t->character->specials.flags, PLR_COMPACT))
	    if (write_to_descriptor(t->descriptor, "\n\r") < 0)
		return(-1);

	return(1);
}

int write_to_descriptor(int desc, const char *txt)
{
    int sofar, thisround, total;

    total = strlen(txt);
    sofar = 0;
  
    do
    {
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0)
	{
	    if (errno == EWOULDBLOCK)
		break;
	    perror("Write to socket");
	    return(-1);
	}
	sofar += thisround;
	bytes_written += thisround;
    } 
    while (sofar < total);
  
    return(0);
}





int process_input(struct descriptor_data *t)
{
    int sofar, thisround, begin, i, k, flag, failed;
    char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];
  
    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);
  
    /* Read in some stuff */
    flag = 0;
    while(!flag && ((begin + sofar) < (MAX_STRING_LENGTH - 1)))
    {
	if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
			      MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
	{
	    for(i = 0 ; i < thisround ; ++i)
		if(ISNEWL(t->buf[begin + sofar + i]))
		{
		    flag = 1;
		    break;
		}
	    
	    sofar += thisround;
	    bytes_read += thisround;
	}
	else if (thisround < 0)
	{
	    if (errno != EWOULDBLOCK)
	    {
		perror("Read1 - ERROR");
		return(-1);
	    }
	    else
		break;
	}
	else
	{
	    slog("EOF encountered on socket read.");
	    return(-1);
	}
    }
  
    t->buf[begin + sofar] = 0;
  
    /* if no newline is contained in input, return without proc'ing */
    if(!flag)
	return(0);
  
    /* input contains 1 or more newlines; process the stuff */
    for (flag = i = 0, k = 0; t->buf[i]; )
    {
	if (!ISNEWL(t->buf[i]) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
	{
	    if(t->buf[i] == '\b')
	    {
		if (k > 0)	/* more than one char ? */
		    k--;				
	    }
	    else if (isascii(t->buf[i]) && isprint(t->buf[i]))
		tmp[k++] = t->buf[i];
	    i++;
	}
	else
 	{
	    tmp[k] = 0;
	    failed = 0;
	    if(strcmp(tmp, "!") == 0)
		strcpy(tmp,t->last_input);
	    else if (*tmp == '^')
	    {
		if ((failed = (!caret_fix(t->last_input, tmp))))
		    SEND_TO_Q("Invalid substitution.\n\r", t);
		else
		    strcpy(tmp, t->last_input);
            }
	    else
		strcpy(t->last_input,tmp);
	
	    if (!failed)
	    {
		write_to_q(tmp, &t->input);
	
		if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)){
		    write_to_q("% ",&t->snoop.snoop_by->desc->output);
		    write_to_q(tmp,&t->snoop.snoop_by->desc->output);
		    write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
		}
	
		if (flag) {
		    sprintf(buffer, 
			    "Line too long. Truncated to:\n\r%s\n\r", tmp);
		    if (write_to_descriptor(t->descriptor, buffer) < 0)
			return(-1);
	  
		    /* skip the rest of the line */
		    for (; t->buf[i] && !ISNEWL(t->buf[i]); i++);
		}
	    }

	    /* find end of entry */
	    for (; ISNEWL(t->buf[i]); i++);
	
	    /* squelch the entry from the buffer */
	    strcpy(t->buf, t->buf + i);
	    k = 0;
	    i = 0;
	}
    }
    return(1);
}




void close_sockets(int s)
{
    struct descriptor_data* desc;
    
    log_msg("Closing all sockets.");

    EACH_DESCRIPTOR(d_iter, desc)
    {
	close_socket(desc, TRUE);
    }
    END_ITER(d_iter);

    close(s);
}





void close_socket(struct descriptor_data *d, int show_msg)
{
    char buf[100];

    if (!d) return;
  
    close(d->descriptor);
    flush_queues(d);
    if (d->descriptor == maxdesc)
	--maxdesc;
  
    /* Forget snooping */
    if (d->snoop.snooping)
	d->snoop.snooping->desc->snoop.snoop_by = 0;
  
    if (d->snoop.snoop_by)
    {
	send_to_char("Your victim is no longer among us.\n\r",
		     d->snoop.snoop_by);
	d->snoop.snoop_by->desc->snoop.snooping = 0;
    }
  
    if (d->character)
      {
	
	/* THE MIN SAFETY CHECK :) FASTEN YOUR SEATBELTS */
	
	/* i was really worried that if a descriptor got closed, ie: player gets killed
	   then quits the game from the menu, that this will crash the auction, so...
	   I added this code to just trash the auction in the case that the seller
	   or buyer vanishes. */
	
	 if (auction->item != NULL) {
	    if (d->character == auction->buyer) { /* buyer disappeared */
	       talk_auction("Auction cancelled by the strange disappearance of the buyer!\n\r");
	       talk_auction("Item returned to seller.\n\r");
	       act("The auctioneer appears before you to return $p to you.\n\r",
		   TRUE,auction->seller,auction->item,NULL,TO_CHAR);
	       act("The auctioneer appears before $n to return $p to $m.\n\r",
		   TRUE,auction->seller,auction->item,NULL,TO_ROOM);
	       obj_to_char(auction->item,auction->seller);
	       auction->item=NULL;
	       auction->buyer=NULL;
	       auction->seller=NULL;
	    }
	    
	    if (d->character == auction->seller) { /* seller disappeared */
	       talk_auction("Auction cancelled...  Someone stole the seller!\n\r");
	       talk_auction("Item kept by Auction Inc.\n\r");
	       if (auction->item)
		 extract_obj(auction->item);
	       auction->item = NULL;
	       auction->seller = NULL;
	       auction->buyer = NULL;
	    }
	    
	 }
	 
	 
	 if (d->connected == CON_PLYNG)	{
	    connected--;
	
	    do_save(d->character, "", 0);

	    if (show_msg) {
	      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);	      
	      sprintf(buf, "Closing link to: %s.",
		      GET_REAL_NAME(d->character));
	      log_msg(buf, LOG_PLAYER);
	    }

	    // If not following self
	    if (d->character->master)
	      stop_follower(d->character);
	    stop_all_followers(d->character);

	    set_descriptor(d->character, 0);
	    d->character->invis_level = TRUST_GRUNT; /* set their invis lev */
	    if (TRUST(d->character) >= TRUST_GRGOD)
		d->character->invis_level = TRUST(d->character);
	    if(d->character == fedit.ch) fedit.ch = 0;
	    if(d->character->board)
	    {
		board_save_board(d->character->board);
		d->character->board->writer = 0;
		d->character->board = 0;
	    }
	} else {
	    sprintf(buf, "Losing player: %s",
		    d->character->player.name ?
		    GET_REAL_NAME(d->character) : "unnamed");
	    slog(buf);
	    WriteToImmort(buf, d->character->invis_level, LOG_CONNECT);
	    free_char(d->character);
	}
    }
    else
	slog("Losing descriptor without char.");
  
    list_delete(&descriptor_list, &d->link);

    if (d->showstr_head)
	FREE(d->showstr_head);

    /* clean up ident stuff */
    if (d->ident_event)
    {
	close(d->ident_sock);
	event_cancel(d->ident_event, 1);
    }

    FREE(d);
}





void nonblock(int s)
{
    if (fcntl(s, F_SETFL, FNDELAY) == -1)
    {
	perror("Noblock");
	exit(1);
    }
}







/* ****************************************************************
*	Public routines for system-to-player-communication	  *
**************************************************************** */


void brag(struct char_data *ch, struct char_data *vict)
{

char brag[256]={"       "};
char buf[MAX_INPUT_LENGTH];

   switch (number(0, 13)) {
  case 0:
   sprintf(brag, "%s brags, '%s was just too easy a kill!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 1:
   sprintf(brag, "%s brags, '%s was a tasty dinner, now who's for desert? Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 2:
   sprintf(brag, "%s brags, 'Bahaha! %s should stick to Odif's !'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 3:
   sprintf(brag, "%s brags, '%s is now in need of some exp...Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 4:
   sprintf(brag, "%s brags, '%s needs a hospital now. Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 5:
   sprintf(brag, "%s brags, '%s's mother is a whore!  Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 6:
   sprintf(brag, "%s brags, '%s is a punk and hits like a swampfly. Bah.'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 7:
   sprintf(brag, "%s brags, '%s, your life force has just run out...Muahaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 8:
   sprintf(brag, "%s brags, 'Bah, %s should stick to the newbie zone!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 9:
   sprintf(brag, "%s brags, '%s, give me your daughter's number and I might return your corpse.  Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 10:
   sprintf(brag, "%s brags, 'Hey %s!  Come back, you dropped your corpse! Muahaha'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 11:
   sprintf(brag, "%s brags, 'I think %s wears pink chainmail.  Fights like a girl!  Muhaha!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 12:
   sprintf(brag, "%s brags, '%s needs food.... BADLY!!!'", GET_NAME(ch), GET_NAME(vict));
  break;
  case 13:
   sprintf(brag, "%s brags, 'Beg for mercy %s!!!!!'", GET_NAME(ch), GET_NAME(vict));
  break;
  }

  sprintf(buf,"\n\r$Cw%s$CN\n\r", brag);
  send_to_all_formatted(buf);

}


void send_to_char(const char *messg, struct char_data *ch)
{
  if (ch)
     if (ch->desc && messg)
       	write_to_q(messg, &ch->desc->output);
}



void save_all(void)
{
    struct descriptor_data *i;

    EACH_DESCRIPTOR(d_iter, i)
    {
	if (i->character)
	{
	    SaveChar(i->character, AUTO_RENT, TRUE);
	}
    }
    END_ITER(d_iter);
}

void send_to_all_regardless(const char *messg)
{
    struct descriptor_data *i;

    if (messg)
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (!i->connected)
		write_to_q(messg, &i->output);
	}
	END_ITER(d_iter);
    }
}

void send_to_all(const char *messg)
{
    struct descriptor_data *i;

    if (messg)
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (!i->connected)
		if(!IS_WRITING(i->character))
		    write_to_q(messg, &i->output);
	}
	END_ITER(d_iter);
    }
}


void send_to_indoor(const char *messg)
{
    struct descriptor_data *i;
   
    if (messg)
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (!i->connected)
		if (!OUTSIDE(i->character) && !IS_WRITING(i->character))
		    write_to_q(messg, &i->output);
	}
	END_ITER(d_iter);
    }
}

void send_to_outdoor(const char *messg)
{
    struct descriptor_data *i;

    if (messg)
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (!i->connected)
		if (OUTSIDE(i->character) && !IS_WRITING(i->character))
		    write_to_q(messg, &i->output);
	}
	END_ITER(d_iter);
    }
}


void send_to_except(const char *messg, struct char_data *ch)
{
    struct descriptor_data *i;

    if (messg)
    {
	EACH_DESCRIPTOR(d_iter, i)
	{
	    if (ch->desc != i && !i->connected && !IS_WRITING(i->character))
		write_to_q(messg, &i->output);
	}
	END_ITER(d_iter);
    }
}



void send_to_room(const char *messg, int room)
{
	struct char_data *i;
	
	if (messg)
	  for (i = real_roomp(room)->people; i; i = i->next_in_room)
	    if (i->desc && !IS_WRITING(i))
	      write_to_q(messg, &i->desc->output);
}


void send_to_room_except(const char *messg, int room, struct char_data *ch)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch && i->desc && !IS_WRITING(i))
	write_to_q(messg, &i->desc->output);
}

void send_to_room_except_two
  (const char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch1 && i != ch2 && i->desc && !IS_WRITING(i))
	write_to_q(messg, &i->desc->output);
}



/* higher-level communication */
/* Code Modified by Jan Vandenbos (Min) (c) 1996 to support ANSI Color Codes */
/* All right reserved */

void expand_act(struct char_data* to, const char* str, char* buf,
		struct char_data* ch, struct obj_data* obj, void* vict_obj,
		int* colorset) {
  const char* strp;
  char* point;
  char* i;
  char itmp[4];
  
  *colorset = 0; /* no colour set */

  for (strp = str, point = buf ; *strp ; strp++) {
    if (*strp == '$') {
      switch(*++strp) {
      case 'n': i = PERS(ch, to); 
	break;
      case 'N': i = PERS((struct char_data *) vict_obj, to); 
	break;
      case 'm': i = HMHR(ch); 
	break;
      case 'M': i = HMHR((struct char_data *) vict_obj); 
	break;
      case 's': i = HSHR(ch); 
	break;
      case 'S': i = HSHR((struct char_data *) vict_obj); 
	break;
      case 'e': i = HSSH(ch); 
	break;
      case 'E': i = HSSH((struct char_data *) vict_obj); 
	break;
      case 'o': i = OBJN(obj, to); 
	break;
      case 'O': i = OBJN((struct obj_data *) vict_obj, to); 
	break;
      case 'p': i = OBJS(obj, to); 
	break;
      case 'P': i = OBJS((struct obj_data *) vict_obj, to); 
	break;
      case 'a': i = SANA(obj); 
	break;
      case 'A': i = SANA((struct obj_data *) vict_obj); 
	break;
      case 'T': i = (char *) vict_obj; 
	break;
      case 'F': i = fname((char *) vict_obj); 
	break;
      case '$': i = "$"; 
	break;
	
      case 'C' : /* This code added by Min(Jan) (c) 1996 for Color interpretation */
	++strp;  /* move to the color code (make sure ptr isn't messed if no_color */
	if (CheckColor(to)) { /* check if the player can view color */
	  *colorset = 1; /* set colour = true */
	  switch (*strp) {
	  case 'K' : i = ANSI_BLACK; break;
	  case 'k' : i = ANSI_BLACK; break;
	  case 'R' : i = ANSI_BRIGHT_RED; break;
	  case 'r' : i = ANSI_RED; break;
	  case 'G' : i = ANSI_BRIGHT_GREEN; break;
	  case 'g' : i = ANSI_GREEN; break;
	  case 'O' : i = ANSI_BRIGHT_ORANGE; break;
	  case 'o' : i = ANSI_ORANGE; break;
	  case 'Y' : i = ANSI_BRIGHT_YELLOW; break;
	  case 'y' : i = ANSI_YELLOW; break;
	  case 'B' : i = ANSI_BRIGHT_BLUE; break;
	  case 'b' : i = ANSI_BLUE; break;
	  case 'M' : i = ANSI_BRIGHT_MAGENTA; break;
	  case 'm' : i = ANSI_MAGENTA; break;
	  case 'C' : i = ANSI_BRIGHT_CYAN; break;
	  case 'c' : i = ANSI_CYAN; break;
	  case 'v' : i = ANSI_VIOLET; break;
	  case 'W' : i = ANSI_WHITE; break;
	  case 'w' : i = ANSI_BRIGHT_WHITE; break;
	  case 'N' : i = ANSI_NORMAL; break;
	  default: {
	    char buf1[MAX_STRING_LENGTH]; /* LOCAL SCOPE!!!! */
	    sprintf(buf1, "Oooo I don't know the foreground color: %c", *strp);
	    log_msg(buf1);
	    log_msg(str);
	    i = "";
	  } /* end of local scope */
	  
	  } /* end of case */
	} /* end of checkcolor */
	 else
	   i = ""; /* null if no colour */
	break;
      case 'c' : /* this code executed if setting BACKGROUND color */
	++strp; /* same as above */
	if (CheckColor(to)) { /* can the player SEE (err see colour?) */
	  *colorset = 1; /* true */
	  switch (*strp) {
	  case 'k' : i = ANSI_BLACK_BACK; break;
	  case 'r' : i = ANSI_RED_BACK; break;
	  case 'g' : i = ANSI_GREEN_BACK; break;
	  case 'y' : i = ANSI_YELLOW_BACK; break;
	  case 'b' : i = ANSI_BLUE_BACK; break;
	  case 'm' : i = ANSI_MAGENTA_BACK; break;
	  case 'c' : i = ANSI_CYAN_BACK; break;
	  case 'w' : i = ANSI_WHITE_BACK; break;
	  default: {
	    char buf1[MAX_STRING_LENGTH]; /* local scope */
	    sprintf(buf1, "Ooo I don't know the background color: %c",*strp);
	    log_msg(buf1);
	    log_msg(str);
	    i="";
	  } /* end of local scope */
	  }
	}
	 else
	   i = "" ; /* sea above */
	break;
      default:
	{
#ifdef JANWORK
	  char buf[MAX_STRING_LENGTH];
	  sprintf(buf, "Illegal $-code to act(): %c", *strp);
	  log_msg(buf);
	  log_msg(str);
	  i = "";
#endif
	  sprintf(itmp,"$%c",*strp);
	  i = itmp;
	}
	break;
      }
      
      while(*i)
	*point++ = *i++;
    }
    else
#define HAS_ACTPROG(ch)		(IS_NPC(ch) && \
				 (mob_index[(ch)->nr].progtypes & ACT_PROG))

      *point++ = *strp;
  }
  
  *point = 0;
  
  buf[0] = UPPER(buf[0]);
}

void act_to_char(struct char_data* to, const char* str, int hide_invisible,
		 struct char_data* ch, struct obj_data* obj, void* vict_obj)
{
    char buf[MAX_STRING_LENGTH];
    char newtext[MAX_STRING_LENGTH*8];
    int colorset;

    if(!to || (!to->desc && !HAS_ACTPROG(to)) ||
       (hide_invisible && !CAN_SEE(to, ch)) ||
       !AWAKE(to) || IS_WRITING(to))
	return;
    
    expand_act(to, str, buf, ch, obj, vict_obj, &colorset);

    /* the color SAFETY set */
    if (colorset) /* color was set (only set if CheckColor was true) */
      strcat(buf,ANSI_NORMAL); /* done before cr/lf! otherwise looks horrible */

    strcat(buf, "\n\r");
    
    if (to->desc) {
      //write_to_q(buf, &to->desc->output); /* testing better color-performance :-) --Mnemosync*/
      send_to_char_formatted(buf, to);
    }

    if (MOBTrigger)
      mprog_act_trigger(buf, to, ch, obj, vict_obj);
}

void act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
    struct room_data* rp;
    struct char_data* to;
    char buf[MAX_STRING_LENGTH];
    int colorset;

    if(!str || !*str)
	return;
    
    if(ch == (struct char_data*) vict_obj)
    {
	if(type & (ACT_TO_CHAR|ACT_TO_VICT))
	    act_to_char(ch, str, hide_invisible, ch, obj, vict_obj);
    }
    else
    {
	if(type & ACT_TO_CHAR)
	    act_to_char(ch, str, hide_invisible, ch, obj, vict_obj);

	if(type & ACT_TO_VICT)
	    act_to_char((struct char_data*) vict_obj, str, hide_invisible,
			ch, obj, vict_obj);
    }

    if(type & ACT_TO_ROOM)
    {
	if((rp = real_roomp(ch->in_room)) == NULL)
	{
	  sprintf(buf, "%s acting in non-existent room #%ld", ss_data((ch)->player.short_descr),
		  ch->in_room);
	  act(buf, TRUE, ch, NULL, NULL, TO_LOG);
	  return;
	}
	for(to = rp->people ; to ; to = to->next_in_room)
	{
	    if((to != ch) && (to != (struct char_data*) vict_obj))
		act_to_char(to, str, hide_invisible, ch, obj, vict_obj);
	}
    }

    if(type & (ACT_TO_ZONE|ACT_TO_WORLD|ACT_TO_IMMORT))
    {
	int zone = -1;

	if(type & ACT_TO_ZONE)
	{
	    rp = real_roomp(ch->in_room);
	    zone = rp ? rp->zone : -1;
	}
	
	EACH_CHARACTER(c_iter, to)
	{
	    /* if this is for mortals, elminate the inappropriate ones */
	    if(type & (ACT_TO_WORLD|ACT_TO_ZONE))
	    {
		/* exclude char and victim, they've already seen it if
		   appropriate */
		if((to == ch) || (to == (struct char_data*) vict_obj))
		    continue;

		/* exclude room, it's already been done if appropriate */
		if(to->in_room == ch->in_room)
		    continue;
	    
		/* if zone shouldn't see it, exclude zone */
		if(!(type & ACT_TO_ZONE))
		{
		    rp = real_roomp(ch->in_room);
		    if(zone == (rp ? rp->zone : -1))
			continue;
		}
	    }

	    if((type & ACT_TO_IMMORT) && !IS_GOD(to))
		continue;
	    
	    /* if we got this far, they might as well see it */
	    act_to_char(to, str, hide_invisible, ch, obj, vict_obj);
	}
	END_AITER(c_iter);
    }

    if(type & ACT_TO_LOG) {
	expand_act(ch, str, buf, ch, obj, vict_obj, &colorset);
	if (colorset) /* see above example */
	  strcat(buf,ANSI_NORMAL);
	slog(buf);
    }

    MOBTrigger = TRUE;
}

void immortal_prompt(struct descriptor_data* point)
{
    struct char_data* ch = point->character;
    struct room_data* rm = real_roomp(ch->in_room);
    char promptbuf[80], *prompt;
    
    prompt = promptbuf;
    *prompt = 0;
    
    if(GET_HIT(point->character) < GET_MAX_HIT(point->character)) {
	if (CheckColor(point->character))
	    sprintf(prompt, "%sH:%d%s ",
		    ANSI_RED, GET_HIT(point->character), ANSI_NORMAL);
	else
	    sprintf(prompt, "H:%d ", GET_HIT(point->character));
	prompt += strlen(prompt);
    }

    if (CheckColor(point->character)) {
	sprintf(prompt, "%sR:%ld%s I:%d %c> ",
		ANSI_CYAN, rm->number, ANSI_NORMAL,
		ch->invis_level,
		IS_SET(ch->specials.flags, PLR_STEALTH) ? 'S' : 's');
    } else {
	sprintf(prompt, "R:%ld I:%d %c> ",
		rm->number,
		ch->invis_level,
		IS_SET(ch->specials.flags, PLR_STEALTH) ? 'S' : 's');
    }

    write_to_descriptor(point->descriptor, promptbuf);
}

void mortal_prompt(struct descriptor_data* point) {
    struct char_data* ch = point->character;
    char promptbuf[80];
    char* prompt;
    int display;

    display = IS_SET(ch->specials.flags, PLR_DISPLAY);

    prompt = promptbuf;
    *prompt = 0;
    
    if(display || (GET_HIT(ch) < GET_MAX_HIT(ch))) {
	if(CheckColor(ch))
	    sprintf(prompt, "%sH:%d%s ", ANSI_RED, GET_HIT(ch), ANSI_NORMAL);
	else
	    sprintf(prompt, "H:%d ", GET_HIT(ch));
	prompt += strlen(prompt);
    }

    if(display || (GET_MANA(ch) < GET_MAX_MANA(ch))) {
       if(CheckColor(ch))
       	 sprintf(prompt, "%sM:%d%s ", ANSI_MAGENTA, GET_MANA(ch), ANSI_NORMAL);
       else
       	 sprintf(prompt, "M:%d ", GET_MANA(ch));
       prompt += strlen(prompt);
    }

    if(display || (GET_MOVE(ch) < GET_MAX_MOVE(ch))) {
	if(CheckColor(ch))
	    sprintf(prompt, "%sV:%d%s ", ANSI_CYAN, GET_MOVE(ch), ANSI_NORMAL);
	else
	    sprintf(prompt, "V:%d ", GET_MOVE(ch));
	prompt += strlen(prompt);
    }

    strcpy(prompt,echo_on_str);
    strcpy(prompt,go_ahead_str);

    strcpy(prompt, "> ");
    
    write_to_descriptor(point->descriptor, promptbuf);
}

void mobile_prompt(struct descriptor_data* point)
{
    struct char_data* ch = point->character;
    char promptbuf[80];

    sprintf(promptbuf,"*H:%d V:%d> ",
	    ch->points.hit,
	    ch->points.move);

    write_to_descriptor(point->descriptor, promptbuf);
}    

void player_prompt(struct descriptor_data* point)
{
    struct char_data* ch = point->character;
    
    if(IS_IMMORTAL(ch))
	immortal_prompt(point);
    else if(IS_PC(ch))
	mortal_prompt(point);
    else
	mobile_prompt(point);
}


int get_total_sub(obj_data *obj) {
   obj_data *iter;
   int count=1;
   
   if(!obj) return 0;
   
   for(iter=obj->contains;iter;iter=iter->next_content)
     count += get_total_sub(iter);
   
   return count;
}

obj_data *find_most_sub(char_data *ch) {
   obj_data *iter=NULL;
   obj_data *heavy=NULL;
   int i, curw, temp;
   
   heavy=ch->carrying;
   if(!heavy) {
      for(i=0;i<MAX_WEAR;i++)
	if(ch->equipment[i]) {
	   heavy = ch->equipment[i];
	   break;
	}
   }
   
   curw = get_total_sub(heavy);
   for(iter=ch->carrying;iter;iter=iter->next_content) {
     if((temp=get_total_sub(iter)) > curw) {
	heavy = iter;
	curw = temp;
     }
   }
   for(i=0;i<MAX_WEAR;i++) {
      iter = ch->equipment[i];
      if((temp=get_total_sub(iter)) > curw) {
	 heavy = iter;
	 curw = temp;
      }
   }
   
   return heavy;
}

void check_idling(struct char_data *ch)
{
    char buf[256];
    struct descriptor_data* d;
    obj_data *heavy;
    
    if(PIRATE==0)
    {
	if (ch->in_room==525 || ch->in_room==526 || ch->in_room==527 ||
	    ch->in_room==528 || ch->in_room==529 || ch->in_room==530) {
	    char_from_room(ch);
	    slog("Sending character back to temple square from pirate quest.");
	    char_to_room(ch, 3005);
	    send_to_char("The pirate quest is now over.  You are sent back to town.\n\r", ch);

	}
    }

// if ((++(ch->specials.timer) > 1) && (real_roomp(ch->in_room)->description != "This room is where polymorphed mages bodies are stored."))
    if ((++(ch->specials.timer) > 8) && (ch->in_room != 3))
    {
	// Solaar: Replaces AFK flag should they do something after being sent to the void.
        if(!IS_AFK(ch))
               do_afk(ch, "", 420);
    	if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE)
    	{
	    ch->specials.was_in_room = ch->in_room;
	    if (ch->specials.fighting)
	    {
		stop_fighting(ch->specials.fighting);
		stop_fighting(ch);
	    }
	    do_save(ch, "", 0);
	    act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
            send_to_char("After being afk for 30 minutes, you will earn 1200 xp per tick.\n\r", ch);
	    char_from_room(ch);
	    char_to_room(ch, 1); /* Into room number 1 */
    	}
	// Solaar: Player earns exp for being afk for extended periods.
        // At 1200xp, a player may level to 15 in one day.
    	if ((++(ch->specials.timer) > 27) && (GET_EXP(ch)<exp_table[ABS_MAX_LVL]))
    	{
		GET_EXP(ch) += 1200;
    	}
}
/***************************************
 *** Solaar: Commenting this out removes the forced logouts because our player base is too small.
 ***         Reward players for staying logged in.
****************************************
	    struct obj_cost cost;
	    int res;

	    res = OfferChar(ch, &cost, AUTO_RENT, FALSE);
	
	   
	    if(!IS_PC(ch) || (ch->in_room == 3))
	      return;

#if 0
	   //took this out, because I'm saving later.
	    if((res == -1) || SaveChar(ch, AUTO_RENT, FALSE))
#else
	    if(res == -1)
#endif
	      {
		  sprintf(buf, "%s just failed an autosave, we'll try again",
			  GET_REAL_NAME(ch));
		  log_msg(buf);
	      }
	    else
	    {
		switch(res)
		{
		case 0:
		    sprintf(buf, "autorent: %s  offer: %d",
			    GET_REAL_NAME(ch), cost.total_cost);
		    break;

		case 1:
		    sprintf(buf, "autorent: %s  offer: %d  gold: %d",
			    GET_REAL_NAME(ch), cost.total_cost,
#if PLUNDER_BANK
			    GET_BANK(ch) + GET_GOLD(ch)
#else
			    GET_GOLD(ch)
#endif
			    );
		    break;

		case 2:
		    ch->drop_count++;
		    sprintf(buf, "autorent: %s  offer: %d  items: %d %d",
			    GET_REAL_NAME(ch), cost.total_cost,
			    cost.no_carried, ch->drop_count);
		    break;
		}
		log_msg(buf, LOG_CONNECT);
	       
	        while(res == 2 && (ch->drop_count > 2)) {
		   heavy = find_most_sub(ch);
		   dlog("Getting rid of %s (%d Sub Items)",
			OBJ_NAME(heavy), get_total_sub(heavy));
		   
		   if(heavy->carried_by) {
		      obj_from_char(heavy);
		   } else if(heavy->equipped_by) {
		      unequip_char(ch, heavy->eq_pos);
		   }
		   extract_obj(heavy);
		   
		   res = OfferChar(ch, &cost, AUTO_RENT, FALSE);
		   dlog("Number of items is now %d", cost.no_carried);
		}
	       
	        SaveChar(ch, AUTO_RENT, FALSE);
	       
	        TrackingSystem.UpdateCharFull(ch);

		drop_unrented(ch);

		d = ch->desc;
		
		extract_char(ch);

		if(d)
		    close_socket(d, TRUE);			      
	    }
***********************************************/
}


/* --------------------------------------------------------------------------
 * NEW FORMATTED OUTPUT OPTION CODE - For colourized output for now
 * All Code (C) Jan Vandenbos (MIN) 1996... All right reserved
 * --------------------------------------------------------------------------
 */


/* the following code will take a string and decode its embedded commands
   No, this is not ACT, it doesn't do parameter replacement.  Right now
   it just does colors for now... (C) 1996 Min (Jan Vandenbos)
*/

void format_string(const char *input, char *output, int color_state) {

  const char* strp;
  char* point;
  char* i="";
  char itmp[4];

   
  for (strp = input, point = output ; *strp ; strp++) {
    if (*strp == '$') {
      switch(*++strp) {
      case '$': i = "$"; 
	break;
      case 'C' : /* This code added by Min(Jan) (c) 1996 for Color interpretation */
	++strp;  /* move to the color code (make sure ptr isn't messed if no_color */
	if (color_state) { /* check if the player can view color */
	  switch (*strp) {
	  case 'K' : i = ANSI_BLACK; break;
	  case 'k' : i = ANSI_BLACK; break;
	  case 'R' : i = ANSI_BRIGHT_RED; break;
	  case 'r' : i = ANSI_RED; break;
	  case 'G' : i = ANSI_BRIGHT_GREEN; break;
	  case 'g' : i = ANSI_GREEN; break;
	  case 'Y' : i = ANSI_BRIGHT_YELLOW; break;
	  case 'y' : i = ANSI_YELLOW; break;
	  case 'B' : i = ANSI_BRIGHT_BLUE; break;
	  case 'b' : i = ANSI_BLUE; break;
	  case 'M' : i = ANSI_BRIGHT_MAGENTA; break;
	  case 'm' : i = ANSI_MAGENTA; break;
	  case 'C' : i = ANSI_BRIGHT_CYAN; break;
	  case 'c' : i = ANSI_CYAN; break;
	  case 'W' : i = ANSI_WHITE; break;
	  case 'w' : i = ANSI_BRIGHT_WHITE; break;
	   case 'N' : i = ANSI_NORMAL; break;
	  default: {
#if 0
	    char buf1[MAX_STRING_LENGTH]; /* LOCAL SCOPE!!!! */
	    sprintf(buf1, "format_string: Oooo I don't know the foreground color: %c", *strp);
	    log_msg(buf1);
	    log_msg(input);
	    i = "";
#endif
	  } /* end of local scope */
	  
	  } /* end of case */
	} /* end of color_state */
	 else
	   i = ""; /* null if no colour */
	break;
      case 'c' : /* this code executed if setting BACKGROUND color */
	++strp; /* same as above */
	if (color_state) { /* can the player SEE (err see colour?) */
	  switch (*strp) {
	  case 'k' : i = ANSI_BLACK_BACK; break;
	  case 'r' : i = ANSI_RED_BACK; break;
	  case 'g' : i = ANSI_GREEN_BACK; break;
	  case 'y' : i = ANSI_YELLOW_BACK; break;
	  case 'b' : i = ANSI_BLUE_BACK; break;
	  case 'm' : i = ANSI_MAGENTA_BACK; break;
	  case 'c' : i = ANSI_CYAN_BACK; break;
	  case 'w' : i = ANSI_WHITE_BACK; break;
	  default: {
	    char buf1[MAX_STRING_LENGTH]; /* local scope */
	    sprintf(buf1, "format_string: Ooo I don't know the background color: %c",*strp);
	    log_msg(buf1);
	    log_msg(input);
	    i="";
	  } /* end of local scope */
	  }
	}
	 else
	   i = "" ; /* sea above */
	break;
      default:
	{
#ifdef JANWORK
	  char buf[MAX_STRING_LENGTH];
	  sprintf(buf, "Illegal $-code to format_string(): %c", *strp);
	  log_msg(buf);
	  log_msg(input);
	  i = "";
#endif
	  sprintf(itmp,"$%c",*strp);
	  i = itmp;
	}
	break;
      }
      
      while(*i)
	*point++ = *i++;
    }
    else
      *point++ = *strp;
  } /* end of for loop */
  
  *point = '\0'; /* add in null char to terminate string */

if (color_state)
  strcat(output, ANSI_NORMAL); /* append a return to normal state to the end of string */
   
  /*  output[0] = UPPER(output[0]); <-- would have made first char uppercase */

} /* end of format_string */




void send_to_char_formatted(const char *messg, struct char_data *ch) {
  char newtext[MAX_STRING_LENGTH*8];

  if (ch)
    if (ch->desc && messg) {
      format_string(messg, newtext, CheckColor(ch));
      write_to_q(newtext,&ch->desc->output);
    }
}


/* Send to Room but use the formatted output routines so it parses colour */

void send_to_room_formatted(const char *messg, int room) {
  struct char_data *player;
  
  if (messg)
    for (player = real_roomp(room)->people; player; player = player->next_in_room)
      if (player->desc && !IS_WRITING(player))
	send_to_char_formatted(messg, player);
}

void send_to_room_except_formatted(const char *messg, int room, struct char_data *ch) {
  struct char_data *player;
  
  if (messg)
    for (player = real_roomp(room)->people; player; player = player->next_in_room)
      if (player != ch && player->desc && !IS_WRITING(player))
	send_to_char_formatted(messg, player);
}

void send_to_room_except_two_formatted(const char *messg, int room, 
				       struct char_data *ch1, struct char_data *ch2) {
  struct char_data *player;
  
  if (messg)
    for (player = real_roomp(room)->people; player; player = player->next_in_room)
      if (player != ch1 && player != ch2 && player->desc && !IS_WRITING(player))
	send_to_char_formatted(messg, player);
}


void send_to_all_regardless_formatted(const char *messg) {
    struct descriptor_data *desc;

    if (messg) {
	EACH_DESCRIPTOR(d_iter, desc) {
	    if (!desc->connected)
	      send_to_char_formatted(messg, desc->character);
	}
	END_ITER(d_iter);
    }
}

void send_to_all_formatted(const char *messg) {
    struct descriptor_data *desc;

    if (messg) {
	EACH_DESCRIPTOR(d_iter, desc) {
	    if (!desc->connected)
		if(!IS_WRITING(desc->character))
		  send_to_char_formatted(messg, desc->character);
	}
	END_ITER(d_iter);
    }
}


void send_to_indoor_formatted(const char *messg) {
    struct descriptor_data *desc;
   
    if (messg) {
	EACH_DESCRIPTOR(d_iter, desc) {
	    if (!desc->connected)
		if (!OUTSIDE(desc->character) && !IS_WRITING(desc->character))
		  send_to_char_formatted(messg, desc->character);
	}
	END_ITER(d_iter);
    }
}

void send_to_outdoor_formatted(const char *messg) {
    struct descriptor_data *desc;

    if (messg) {
	EACH_DESCRIPTOR(d_iter, desc) {
	    if (!desc->connected)
		if (OUTSIDE(desc->character) && !IS_WRITING(desc->character))
		  send_to_char_formatted(messg, desc->character);
	}
	END_ITER(d_iter);
    }
}

void cprintf(struct char_data *ch, const char *format, ...) {
   char debug_buf[MAX_STRING_LENGTH];
   va_list args;
   va_start(args, format);
   vsprintf (debug_buf, format, args);

   send_to_char_formatted(debug_buf, ch);
}
