
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "structs.h"
#include "utils.h"
#include "list.h"
#include "comm.h"
#include "utility.h"
#include "multiclass.h"
#include "db.h"
#include "recept.h"
#include "util_str.h"

/* internal data structures */
struct page
{
    list_element	link;
    char*		pager;
    char*		pagee;
    char*		message;
};

/* global variables */
static list_head gPageList;

/* user interface prototypes */
void help_page(struct char_data* to);
void list_pages(struct char_data* to, int byCommand);
void cancel_page(struct char_data* from, char* to);
void answer_page(struct char_data* to, char* from);
void add_page(struct char_data* from, char* to, char* message);
void all_pages(struct char_data* from);

/* real work prototypes */
void insert_page(struct page* page);
void remove_page(struct page* page);
struct page* find_page(char* pager, char* pagee);
struct page* create_page(char* pager, char* pagee, char* message);
void delete_page(struct page* page);

void init_pages(void)
{
    list_init(&gPageList, 0);
}

/* basic command interface, just switch into one of the routines to
   do the real work */
void do_page(struct char_data *ch, char *arg, int cmd)
{
    char who[MAX_STRING_LENGTH];

    if(!IS_PC(ch))
    {
      send_to_char("What would a mob care about messages...\n\r", ch);
      return;
    }
    
    arg = one_argument(arg, who);

    while(arg && isspace(*arg))
	arg++;
    
    if(!*who || !str_cmp(who, "help"))	/* no argument, tell them what's up */
	help_page(ch);
    else if(!str_cmp(who, "list"))	/* find out who's paging you */
	list_pages(ch, 1);
    else if(!str_cmp(who, "cancel"))	/* cancel a page you made */
	cancel_page(ch, arg);
    else if(!str_cmp(who, "answer"))	/* answer a page to you, really */
	answer_page(ch, arg);		/* really just cancels it */
    else if(!str_cmp(who, "all"))
	all_pages(ch);
    else				/* otherwise it's a new page */
	add_page(ch, who, arg);
}

void help_page(struct char_data* to)
{
    send_to_char("page help - get this message\n\r", to);
    send_to_char("page any <message> - send a message to any god\n\r", to);
    send_to_char("page <god> <message> - send a message to a specific god\n\r",
		 to);
    send_to_char("page cancel <to> - cancel a message you sent\n\r", to);
    if(IS_IMMORTAL(to))
	send_to_char("page answer <from> - cancel a message to you\n\r", to);
    send_to_char("  omitting <to> or <from> will cancel the oldest page\n\r",
		 to);
    if(IS_IMMORTAL(to))
	send_to_char("page list - show messages waiting for you\n\r", to);
    if(TRUST(to) >= TRUST_IMP)
	send_to_char("page all - show all messages in the system\n\r", to);
}

void cancel_page(struct char_data* from, char* to)
{
    struct page*	page;

    if(!(page = find_page(GET_REAL_NAME(from), to)))
    {
	send_to_char("No appropriate pages.\n\r", from);
	return;
    }

    remove_page(page);
    delete_page(page);
}

struct show_info
{
    char* name;
    int found;
    struct char_data* ch;
};

static int show_a_page(struct page* page, struct show_info* info)
{
    if(!page->pagee || !info->name || !str_cmp(page->pagee, info->name))
    {
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "%2d %-20s %s", ++info->found, page->pager,
		page->pagee ? page->pagee : "any");
	send_to_char(buf, info->ch);
	sprintf(buf, "%s\n\r", page->message);
	send_to_char(buf, info->ch);
    }
    return 1;
}

void list_pages(struct char_data* to, int byCommand)
{
    struct show_info	info;

    if(!IS_IMMORTAL(to))
    {
	send_to_char("No messages for mortals...\n\r", to);
	return;
    }

    info.name = GET_IDENT(to);
    info.ch = to;
    info.found = 0;
    
    list_find(&gPageList, (list_find_func) show_a_page, &info);
    
    if(!info.found && byCommand)
	send_to_char("Nobody loves you!\n\r", to);
}

void answer_page(struct char_data* to, char* from)
{
    struct page*	page;

    if(!(page = find_page(from, GET_REAL_NAME(to))))
    {
	send_to_char("No appropriate pages.\n\r", to);
	return;
    }

    remove_page(page);
    delete_page(page);
}

void add_page(struct char_data* from, char* to, char* message)
{
    int	found;
    struct char_data* play = NULL;
    struct char_data* tmp_ch;
    struct descriptor_data* desc;
    char msg[MAX_STRING_LENGTH];
    struct page* page;
    char* sender;

    sender = GET_REAL_NAME(from);
  
    if(find_page(sender, to))
    {
	send_to_char("You already have a page waiting for that person.\n\r", from);
	return;
    }

    if(!str_cmp(to, "any"))
    {
	sprintf(msg, "Page: %s %s %s", sender, to, message);
	WriteToImmort(msg, TRUST_GRUNT);
	to = 0;
    }
    else
    {
	/* check for an active player first */
	EACH_DESCRIPTOR(d_iter, desc)
	{
	    if((play = desc->character)
	       && !str_cmp(to, GET_REAL_NAME(play)))
		break;
	    play = 0;
	}
	END_ITER(d_iter);

	/* found an active player, see if they're a god... */
	if(play)
	{
	    if(!IS_GOD(play))
	    {
		send_to_char("No pages for mortals...\n\r", from);
		return;
	    }
	    else
	    {
		sprintf(msg, "[Page: %s %s %s]\n\r", sender, to, message);
		send_to_char(msg, play);
	    }
	}
	else if((tmp_ch = LoadChar(0, to, READ_PLAYER)))
	{
	    found = IS_GOD(tmp_ch);
	    free_char(tmp_ch);
	    if(!found)
	    {
		send_to_char("No pages for mortals\n\r", from);
		return;
	    }
	}
	else
	{
	    /* didn't find an active player, make sure they exist
	       at all... */
	    send_to_char("No such player\n\r", from);
	    return;
	}
    }

    page = create_page(sender, to, message);
    insert_page(page);

    send_to_char(
		 "Your message has been left and will be answered in the order it was received\n\r", from);
}

void all_pages(struct char_data* ch)
{
    struct show_info info;
    
    if(TRUST(ch) < TRUST_IMP)
    {
	send_to_char("Only imps are allowed to be nosy\n\r", ch);
	return;
    }

    info.name = 0;
    info.ch = ch;
    info.found = 0;
    
    list_find(&gPageList, (list_find_func) show_a_page, &info);

    send_to_char("OK\n\r", ch);
}

void insert_page(struct page* page)
{
    list_append(&gPageList, &page->link);
}

void remove_page(struct page* page)
{
    list_delete(&gPageList, &page->link);
}

struct find_page_info
{
    char* pager;
    char* pagee;
};

static int find_page_func(struct page* page, struct find_page_info* in)
{
    if ((!in->pager || !page->pager || !str_cmp(page->pager, in->pager)) &&
	(!in->pagee || !page->pagee || !str_cmp(page->pagee, in->pagee)))
	return 0;
    return 1;
}

struct page* find_page(char* pager, char* pagee)
{
    struct find_page_info info;

    info.pager = (pager && *pager) ? pager : 0;
    info.pagee = (pagee && *pagee) ? pagee : 0;
    
    return (struct page*) list_find(&gPageList,
				    (list_find_func) find_page_func, &info);
}

struct page* create_page(char* pager, char* pagee, char* message)
{
    struct page* page;
    static char buffer[100], buf[MAX_STRING_LENGTH];
    long ct;
    char *tmstr;
    struct tm* tm;
    char* tz;

    tz = "EDT";
    ct = time(0);
    tmstr = asctime(tm = localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sprintf(buffer, "%s (%s)\n\r ", tmstr, tz);
    
    CREATE(page, struct page, 1);

    if(pager && *pager)
	page->pager = strdup(pager);
    else
	page->pager = 0;

    if(pagee && *pagee)
	page->pagee = strdup(pagee);
    else
	page->pagee = 0;

    if(message && *message)
    {
	sprintf(buf, "    %s   %s", buffer, message);
	page->message = strdup(buf);
    }
    else
    {
      sprintf(buf, "    %s   No Message...", buffer);
      page->message = strdup(buf);
    }

    return page;
}

void delete_page(struct page* page)
{
    if(page->pager)	FREE(page->pager);
    if(page->pagee)	FREE(page->pagee);
    if(page->message)	FREE(page->message);
    FREE(page);
}
