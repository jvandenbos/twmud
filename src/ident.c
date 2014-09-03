/* ************************************************************************
*  File: ident.c                               Part of Thieves' World MUD *
*                                                                         *
*  Usage: Functions for handling rfc 931 ident stuff                      *
*                                                                         *
*  $Author: twmain $
*  $Date: 2004/03/14 18:46:24 $
*  $Revision: 1.1.1.1 $
************************************************************************ */

#include "config.h"

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#if USE_stdlib
#include <stdlib.h>
#endif

#if USE_unistd
#include <unistd.h>
#endif

#include <fcntl.h>

#include "structs.h"
#include "utility.h"
#include "utils.h"
#include "util_str.h"
#include "events.h"
#include "comm.h"
#include "db.h"
#include "spell_util.h"
#include "ident.h"
#include "proto.h"


typedef struct 
{
    event_t			event;
    struct descriptor_data	*desc;
} ident_event_t;


#define IDENT_TIMEOUT 30

int ident = FALSE;		/* Turns ident support on/off */

static int ident_port;		/* Remote port runing identd  */


int ident_timeout(ident_event_t* theEvent, long now);

void do_ident(struct char_data *ch, char *argument, int cmd)
{
    char mesg[256];

    argument = skip_spaces(argument);

    if (!*argument)
    {
	sprintf(mesg, "Ident service is currently %s.\n\r",
		ident ? "on" : "off");
	send_to_char(mesg, ch);
    }

    else if (!str_cmp(argument, "on"))
    {
	ident = TRUE;
	send_to_char("Ident service turned on.\n\r", ch);
	log_msg("Ident service turned on.");
    }
    else if (!str_cmp(argument, "off"))
    {
	ident = FALSE;
	send_to_char("Ident service turned off.\n\r", ch);
	log_msg("Ident service turned off.");
    }
    else
	send_to_char("Usage:  ident [ on | off ]\n\r", ch);
}


void ident_init(void)
{
    const struct servent *sp;

    if (!(sp = getservbyname("ident", "tcp")))
    {
	fprintf(stderr, "ident: no such service\n");
	ident = FALSE;
    }

    ident_port = sp->s_port;
}


void ident_start(struct descriptor_data *d, long addr)
{
    int  sock;
    int	 flags;
    struct sockaddr_in	 sa;
    ident_event_t* event;
    int	when;

    if (!ident)
    {
	STATE(d) = CON_ASKNAME;
	return;
    }

    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("socket");
	return;
    }

    sa.sin_family = AF_INET;
    sa.sin_port = ident_port;
    sa.sin_addr.s_addr = addr;


#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

    /* set socket to nonblocking */
    flags = fcntl(sock, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) < 0)
    {
	perror("ident fcntl");
	STATE(d) = CON_ASKNAME;
	return;
    }

    CREATE(event, ident_event_t, 1);
    event->desc = d;
    when = pulse + PULSE_PER_REAL_SEC * IDENT_TIMEOUT;

    d->ident_event = event_queue_pulse((event_t*) event, when,
				       (event_func) ident_timeout,
				       "ident request");

    errno = 0;

    if (connect(sock, (struct sockaddr*) &sa, sizeof(sa)) != 0)
    {
	if (errno == EINPROGRESS)
	{
	    d->ident_sock = sock;
	    STATE(d) = CON_IDCONING;
	    return;
	}

	if (errno == ECONNREFUSED)
	{
	    STATE(d) = CON_ASKNAME;
	    return;
	}
	else 
	{
	    perror("ident connect");
	    STATE(d) = CON_ASKNAME;
	    return;
	}
    }
    else
    {
	d->ident_sock = sock;
	STATE(d) = CON_IDCONED;
	return;
    }
}


void ident_check(struct descriptor_data *d)
{
    struct timeval tv;
    fd_set writeable, readable;
    int rc, rmt_port, our_port, len;
    char buf[256], user[256], buf2[256], *p;

    extern int port;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    switch (STATE(d))
    {
    case CON_IDCONING:
	/* waiting for connect() to finish */
	
	FD_ZERO(&writeable);
	FD_SET(d->ident_sock, &writeable);

	errno = 0;

	if ((rc = select(d->ident_sock + 1, (fd_set *) 0, &writeable,
			 (fd_set *) 0, &tv)) == 0)
	    break;

	else if (rc < 0)
	{
	    perror("ident check select (conning)");
	    STATE(d) = CON_ASKNAME;
	    break;
	}

	STATE(d) = CON_IDCONED;
	/* continue through if ready to write */

    case CON_IDCONED:
	/* connected, write request */
	
	sprintf(buf, "%d, %d\n\r", ntohs(d->peer_port), port);
	
	len = strlen(buf);
	if (write(d->ident_sock, buf, len) != len)
	{
	    if (errno != EPIPE)
		perror("ident check write (conned)");

	    STATE(d) = CON_ASKNAME;
	    break;
	}

	STATE(d) = CON_IDREADING;
	/* continue through if successful write */
	
    case CON_IDREADING:
	/* waiting to read */
	
	FD_ZERO(&readable);
	FD_SET(d->ident_sock, &readable);
	if ((rc = select(d->ident_sock+1, &readable, (fd_set *) 0,
			 (fd_set *) 0, &tv)) == 0)
	    break;

	else if (rc < 0)
	{
	    perror("ident check select (reading)");
	    STATE(d) = CON_ASKNAME;
	    break;
	}

	STATE(d) = CON_IDREAD;
	/* continue through with read */
	
    case CON_IDREAD:
	/* read ready, git the info */
	
	if ((len = read(d->ident_sock, buf, sizeof(buf))) < 0)
	{
	    if (errno != EAGAIN)
		perror("ident check read (read)");
	    else
		break;
	}
	else
	{
	    buf[len] = '\0';
	    if (sscanf(buf, "%u , %u : USERID :%*[^:]:%255s",
		       &rmt_port, &our_port, user) != 3)
	    {
		/* check if error or malformed */
		if (sscanf(buf, "%u , %u : ERROR : %255s",
			   &rmt_port, &our_port, user) == 3) {
		    sprintf(buf2, "Ident error from %s: \"%s\"", d->host, user);
		    log_msg(buf2);
		}
		else {
		    /* strip off trailing newline */
		    for (p = buf + len - 1; p > buf && ISNEWL(*p); p--);
		    p[1] = '\0';

		    sprintf(buf2, "Malformed ident response from %s: \"%s\"",
			    d->host, buf);
		    log_msg(buf2);
		}
	    }
	    else
	    {
		strncpy(buf2, user, MAX_IDENT_LENGTH);
		strcat(buf2, "@");
		strncat(buf2, d->host, MAX_HOSTNAME_LENGTH);
		strcpy(d->host, buf2);
	    }
	}
	
	STATE(d) = CON_ASKNAME;
	/* continue through to next state */
	
    case CON_ASKNAME:
	/* ident complete, ask for name */
	
	if (d->ident_event)
	{
	    close(d->ident_sock);
	    event_cancel(d->ident_event, 1);
	}
	d->ident_event = NULL;

	/* extra ban check */
	if (isbanned(d->host) == BAN_ALL) {
	    close_socket(d, TRUE);
	    sprintf(buf, "Connection attempt denied from [%s]", d->host);
	    log_msg(buf);
	    return;
	}
	
	SEND_TO_Q("\x1B[2K\n\rBy what name do you wish to be known? ", d);
	STATE(d) = CON_NME;
	return;

    default:
	return;
    }

    if ((pulse % PULSE_PER_REAL_SEC) == 0)
	SEND_TO_Q(".", d);
}


/* returns 1 if waiting for ident to complete, else 0 */
int waiting_for_ident(struct descriptor_data *d)
{
    switch (STATE(d))
    {
    case CON_IDCONING:
    case CON_IDCONED:
    case CON_IDREADING:
    case CON_IDREAD:
    case CON_ASKNAME:
	return 1;

    default:
	return 0;
    }

    return 0;
}


/* we've waited long enough for ident response */
int ident_timeout(ident_event_t* theEvent, long now)
{
    struct descriptor_data *d = theEvent->desc;

    close(d->ident_sock);
    STATE(d) = CON_ASKNAME;

    d->ident_event = NULL;
    event_free((event_t *)theEvent);

    return 1;
}
