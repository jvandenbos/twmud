#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "proto.h"
#include "utils.h"
#include "comm.h"
#include "modify.h"
#include "utility.h"
#include "act.h"
#include "util_str.h"
#include "interpreter.h"
    
/* forward declaration */
void editor_list_lines(struct descriptor_data* d,
		       int lineNo1, int lineNo2,
		       int withNumbers, int withPause);
void editor_replace_lines(struct descriptor_data* d,
			  int lineNo1, int lineNo2, char* repl);
void editor_insert_line(struct descriptor_data* d, int lineNo1, char* text);
int merge_lines(struct descriptor_data* d, char* lines[], int line_cnt);
int split_lines(struct descriptor_data* d, char* lines[], int max_lines);
void kill_lines(char* lines[]);
void bound_check(int max, int* lineNo1, int* lineNo2, int defAll);
void editor_get_arguments(char *linePtr, int *lineNo1,
			  int *lineNo2, char **text);

void editor_prompt(struct descriptor_data* point)
{
#if LINE_EDITOR
    char promptbuf[80];
    char* editorStrPtr;
    int editor_line_count = 1;

    if(point->str)
    {
	if(*point->str)		/* there is a string! */
	    for(editorStrPtr=*point->str;*editorStrPtr;editorStrPtr++)
		if(*editorStrPtr == '\n') editor_line_count++;
    }
    else if(point->sstr)
    {
	if(*point->sstr)
	    for(editorStrPtr=ss_data(*point->sstr);*editorStrPtr;editorStrPtr++)
		if(*editorStrPtr == '\n') editor_line_count++;
    }
    
    sprintf(promptbuf, "%02d] ", editor_line_count);
 	  
    write_to_descriptor(point->descriptor, promptbuf);
#else
    write_to_descriptor(point->descriptor, "] ");
#endif
}

#if LINE_EDITOR
/* 
 * Editor_parser scans the user input for one of the editor commands.
 * If an editor command is not found in the line, the parsers exits
 * with return value 0.  If an editor command is found within the
 * line, the parser performs the command and exits with a return value
 * of 1 (excluding command '.s', return value = 0).
 *
 * The syntax of an editor command: .<x><y>[<text>]
 *   Where:
 *     '.' appears on the first column.
 *     <x> = {l|L|v|V|p|P|d|D|i|I|?|h|H|s|S|r|R|m|M}
 *     <y> = { ' ' | '\0' }
 *     <text> = optional argument(s) and text
 *
 *                                                             -jwoodall
 */
int editor_parser(struct descriptor_data *d, char *str)
{
    /* obtain help screen on editor. */
    /* If the name changes in        */
    /* 'help_table', it must be also */
    /* be changed here. */
    char helpTopic[MAX_INPUT_LENGTH] = EDITOR_HELP_TOPIC;
    char *linePtr = str;
    char *text, command;
    int lineNo1, lineNo2, look_at;
 
    if ( (*linePtr++ != '.') || (*(linePtr) == 0) )
	/* the line does not start with a period, or is just a single period */
	return 0;
 
    command = *linePtr++;
    command = LOWER(command);
 
    if ( (*linePtr != ' ') && (*linePtr != 0) )
	/* the command is not followed by a space or a NULL */
	return 0;
 
    editor_get_arguments(linePtr, &lineNo1, &lineNo2, &text);
 
    switch( command )
    {
 
    case 'h':			/* display the help screen  */
    case '?':
	send_to_char("\n\r", d->character);
	if(*text) strcpy(helpTopic, text);
	do_help(d->character, helpTopic, 0);
	break;
 
    case 's':			/* quit and save the message */
	*str = '@';
	str[1] = 0;
	return 0;
 
    case 'm':			/* memory display */
	{
	    char display[80];
 
	    if(d->str)
		look_at = (*d->str) ? strlen(*d->str) : 0;
	    else
		look_at = strlen(ss_data(*d->sstr));
	    
	    sprintf(display, "\n\rMemory Available: [%d]  Used: [%d]  Remaining: [%d]\n\r\n\r",
		    d->max_str,
		    look_at,
		    d->max_str - look_at);
	    send_to_char(display, d->character);
	}
	break;
 
    case 'l':			/* list line ranges */
	editor_list_lines(d, lineNo1, lineNo2, TRUE, TRUE);
	break;
	
    case 'v':
	editor_list_lines(d, lineNo1, lineNo2, FALSE, FALSE);
	break;
 
    case 'p':			/* page */
	editor_list_lines(d, lineNo1, lineNo2, FALSE, TRUE);
	break;
 
    case 'd':			/* delete */
	editor_replace_lines(d, lineNo1, lineNo2, NULL);
	break;
 
    case 'i':			/* insert */
	editor_insert_line(d, lineNo1, text);
	break;
 
    case 'r':			/* replace */
	editor_replace_lines(d, lineNo1, lineNo2, text);
	break;
 
    default:			/* this shouldn't be reached */
	return 0;
    }
 
    return 1;
}
 
       
   
 
 /*
  * editor_get_arguments() obtains the optional arguments of an editor command.
  * Argument syntax : [[<integer> [<integer>]] <string>]
  *
  *   char *linePtr = string following the command letter
  *   int *lineNo1  = optional line number argument, 0 if nonexistent
  *   int *lineNo2  = other optional line number argument, 0 if nonexistent
  *   char **text   = optional text string, 0 if nonexistent
  */
void editor_get_arguments(char *linePtr, int *lineNo1,
			  int *lineNo2, char **text)
{
    char first_arg[MAX_INPUT_LENGTH], second_arg[MAX_INPUT_LENGTH];
 
    *lineNo1 = *lineNo2 = 0;
    *text=linePtr;
 
    while(isspace(**text)) (*text)++;
 
    if( !**text) return;
 
    argument_interpreter(*text, first_arg, second_arg);
 
    if( is_number(first_arg) ) { /* do have a valid line 1 argument? */
	*lineNo1 = atoi(first_arg);
	*text += strlen(first_arg);
	while( isspace(**text) ) (*text)++;
	if ( is_number(second_arg) ) { /* do have a valid line 2 argument? */
	    *lineNo2 = atoi(second_arg);
	    *text += strlen(second_arg);
	    while( isspace(**text) ) (*text)++;
	}
    }
 
}

void editor_list_lines(struct descriptor_data* d,
		       int lineNo1, int lineNo2,
		       int withNumbers, int withPause)
{
    char bigbuf[8192];
    char buf[256], *ptr;
    int line_count, look_at;
    char *lines[512];
    
    line_count = split_lines(d, lines, sizeof(lines) / sizeof(*lines));
    if(line_count <= 0)
	return;

    bound_check(line_count, &lineNo1, &lineNo2, TRUE);
	
    if(withPause)
	ptr = bigbuf;
    else
	ptr = buf;
    
    for(look_at = lineNo1 ; look_at <= lineNo2 ; ++look_at)
    {
	if(withNumbers)
	    sprintf(ptr, "%2d] %s\n\r", look_at + 1, lines[look_at]);
	else
	    sprintf(ptr, "%s\n\r", lines[look_at]);

	if(withPause)
	    ptr += strlen(ptr);
	else
	    send_to_char(ptr, d->character);
    }

    if(withPause)
    {
	*ptr = 0;
	page_string(d, bigbuf, 0);
    }

    kill_lines(lines);
}
	
void editor_insert_line(struct descriptor_data* d, int lineNo1, char* text)
{
    int line_count, src, dst;
    char* lines[512];
    
    line_count = split_lines(d, lines, sizeof(lines) / sizeof(*lines));
    if(line_count <= 0)
	return;
    bound_check(line_count, &lineNo1, &src, FALSE);
	    
    for(dst = line_count + 1, src = line_count ;
	src >= lineNo1 ; --src, --dst)
	lines[dst] = lines[src];
	    
    lines[lineNo1] = strdup(text);

    line_count++;

    merge_lines(d, lines, line_count);
    
    send_to_char("\n\rLine inserted...\n\r", d->character);
}

void editor_replace_lines(struct descriptor_data* d,
			  int lineNo1, int lineNo2, char* repl)
{
    int line_count, src, dst;
    char* lines[512];
    char message[80];
 
    line_count = split_lines(d, lines, sizeof(lines) / sizeof(*lines));
    if(line_count <= 0)
	return;
    bound_check(line_count, &lineNo1, &lineNo2, FALSE);
	    
    for(dst = lineNo1 ; dst <= lineNo2 ; dst++)
	if(lines[dst])
	    FREE(lines[dst]);
	    
    if(repl)
	dst = lineNo1 + 1;
    else
	dst = lineNo1;

    for(src = lineNo2 + 1 ; src <= line_count ; ++src, ++dst)
	lines[dst] = lines[src];
    line_count = dst - 1;

    if(repl)
	lines[lineNo1] = strdup(repl);
    
    merge_lines(d, lines, line_count);
    
    sprintf(message, "\n\r%d line(s) %s.\n\r",
	    lineNo2 - lineNo1 + 1, repl ? "replaced" : "deleted");
    send_to_char(message, d->character);
}

int split_lines(struct descriptor_data* d, char* lines[], int max_lines)
{
    char* ptr;
    char* end;
    int endLineNo = 0;

    if(d->str)
	end = *d->str;
    else if(d->sstr)
	end = ss_data(*d->sstr);
    else
	return 0;

    if(!end)
      return 0;
    
    ptr = end;
    while(*end)
    {
	switch(*end)
	{
	case '\r':
	    ptr = ++end;
	    break;

	case '\n':
	    CREATE(lines[endLineNo], char, end - ptr + 1);
	    strncpy(lines[endLineNo], ptr, end - ptr);
	    lines[endLineNo++][end - ptr] = 0;
	    ptr = ++end;
	    break;

	default:
	    end++;
	}
    }
    
    lines[endLineNo]=0;

    return endLineNo;
}

int merge_lines(struct descriptor_data* d, char** lines, int line_cnt)
{
    int size;
    int	i;
    char* ptr;
    
    for(size = 0, i = 0 ; i < line_cnt ; ++i)
	if(lines[i])
	    size += strlen(lines[i]) + 2;

    if(d->str)
    {
	CREATE(ptr, char, size + 1);
	if(*d->str)
	    FREE(*d->str);
	*d->str = ptr;
    }
    else if(d->sstr)
    {
	if(*d->sstr)
	    ss_free(*d->sstr);
	*d->sstr = ss_empty(size + 1);
	ptr = ss_data(*d->sstr);
    }
    else
	return 0;
    
    for(i = 0 ; i < line_cnt ; ++i)
	if(lines[i])
	{
	    strcpy(ptr, lines[i]);
	    ptr += strlen(lines[i]);
	    *ptr++ = '\n';
	    *ptr++ = '\r';
	    FREE(lines[i]);
	}

    *ptr = 0;

    return size;
}

void kill_lines(char** lines)
{
  while(*lines)
  {
    FREE(*lines);
    lines++;
  }
}

void bound_check(int max, int* lineNo1, int* lineNo2, int defAll)
{
    *lineNo1 = *lineNo1 - 1;
    if(*lineNo1 < 0)
	*lineNo1 = 0;
    else if(*lineNo1 >= max)
	*lineNo1 = max - 1;

    *lineNo2 = *lineNo2 - 1;
    if(*lineNo2 < 0)
	*lineNo2 = defAll ? max - 1 : *lineNo1;
    else if(*lineNo2 >= max)
	*lineNo2 = max - 1;
}

#endif
