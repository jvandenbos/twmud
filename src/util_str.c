#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "util_str.h"

#if NEED_STRDUP
/* Create a duplicate of a string */
char *strdup(const char *source)
{
    char *ptr;

    if(!source)
	return 0;
    
    CREATE(ptr, char, strlen(source)+1);
    return(strcpy(ptr, source));
}
#endif

int scan_number(const char *text, int *rval)
{
    char* next;
    
    *rval = strtol(text, &next, 10);
    return next != text;
}

int get_number(char **name)
{
    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH] = "";
  
    if ((ppos = (char *)index(*name, '.')) && ppos[1]) {
	*ppos++ = '\0';
	strcpy(number,*name);
	strcpy(*name, ppos);
    
	for(i=0; *(number+i); i++)
	    if (!isdigit(*(number+i)))
		return(0);
    
	return(atoi(number));
    }
  
    return(1);
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(const char *arg1, const char *arg2)
{
  int chk;

  if ((!arg2) || (!arg1))
    return(1);

  for(; *arg1 || *arg2 ; arg1++, arg2++)
    if((chk = (LOWER(*arg1) - LOWER(*arg2))))
      if (chk < 0)
	return (-1);
      else 
	return (1);

  return 0;
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(const char *arg1, const char *arg2, int n)
{
  int chk;

  if ((!arg2) || (!arg1))
    return(1);

  for(; (n > 0) && (*arg1 || *arg2) ; arg1++, arg2++, n--)
    if((chk = (LOWER(*arg1) - LOWER(*arg2))))
      if (chk < 0)
	return (-1);
      else 
	return (1);

  return 0;
}



void sprintbit(unsigned long vektor, const char *names[], char *result)
{
  long nr;
  
  *result = '\0';
  
  for(nr=0; vektor; vektor>>=1)
    {
      if (IS_SET(1, vektor))
	if (*names[nr] != '\n') {
	  strcat(result,names[nr]);
	  strcat(result," ");
	} else {
	  strcat(result,"UNDEFINED");
	  strcat(result," ");
	}
      if (*names[nr] != '\n')
	nr++;
    }
  
  if (!*result)
    strcat(result, "NONE"); /* used to be "NOBITS" */
}



void sprinttype(int type, const char *names[], char *result)
{
	int nr;

	for(nr=0;(*names[nr]!='\n');nr++);
	if(type < nr)
		strcpy(result,names[type]);
	else
		strcpy(result,"UNDEFINED");
}

/*
   these two procedures give the player the ability to buy 2*bread
   or put all.bread in bag, or put 2*bread in bag...
*/
bool getall(const char *name, char *newname)
{
    if(strncmp(name, "all.", 4))
	return FALSE;
    if(name[4] == 0)
	return FALSE;
    
    strcpy(newname, name + 4);
    
    return(TRUE);
}


int getabunch(char *name, char  *newname)
{
    int num=0;

    if(sscanf(name,"%d*%s",&num,newname) != 2)
	return 0;
   
    if(*newname == 0)
	return 0;
   
    if (num < 1)
	return 0;
    if (num>9)
	num = 9;

    return num;
}


char *lower(const char *s)
{
  static char c[1000];
  char* p;

  for(p = c ; *s ; p++, s++)
      *p = tolower(*s);
  *p = 0;

  return(c);
}

void center(char* buf, char* str, int width)
{
    int len;
    
    len = strlen(str);
    if((len = (width - len) / 2) < 0)
	len = 0;
    
    strncpy(buf, "                                       ", len);
    strcpy(buf + len, str);
}

/* read contents of a text file, and place in buf */
int file_to_string(const char *name, char *buf, int max)
{
    FILE *fl;
    char tmp[100];
    int offset = 0;
    int len;
  
    *buf = 0;
  
    if (!(fl = fopen(name, "r")))
    {
	perror(name);
	return(FALSE);
    }

    while(fgets(tmp, 99, fl))
    {
	len = strlen(tmp);
      
	if((len + 2 + offset) > max)
	{
	    sprintf(tmp, "file_to_string: \"%s\" to big", name);
	    log_msg(tmp);
	    fclose(fl);
	    return(FALSE);
	}

	strcpy(buf + offset, tmp);
	offset += strlen(tmp);
	buf[offset++] = '\r';
	buf[offset] = 0;
    }

    fclose(fl);

    return(TRUE);
}

int isname(const char *str, const char *namelist)
{
  char	*argv[100], *xargv[100];
  int	argc, xargc, i,j, exact;
  static char	buf[MAX_INPUT_LENGTH], names[MAX_INPUT_LENGTH], *s;
  
  strcpy(buf, str);
  argc = split_string(buf, "- \t\n\r,", argv);
  
  strcpy(names, namelist);
  xargc = split_string(names, "- \t\n\r,", xargv);
  
  s = argv[argc-1];
  s += strlen(s);
  if (*(--s) == '.') {
    exact = 1;
    *s = 0;
  } else {
    exact = 0;
  }
  /* the string has now been split into separate words with the '-'
     replaced by string terminators.  pointers to the beginning of
     each word are in argv */
  
  if (exact && argc != xargc)
    return 0;
  
  for (i=0; i<argc; i++) {
    for (j=0; j<xargc; j++) {
      if (0==str_cmp(argv[i],xargv[j])) {
	xargv[j] = NULL;
	break;
      }
    }
    if (j>=xargc)
      return 0;
  }
  
  return 1;
}

char *fname(char *namelist)
{
  static char holder[100];
  register char *point;
  
  if (!namelist)
    return NULL;

  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;
  
  *point = '\0';
  
  return(holder);
}


int split_string(char *str, char *sep, char **argv)
{
    char	*s;
    int	argc=0;
  
    s = strtok(str, sep);
    if (s)
	argv[argc++] = s;
    else {
	*argv = str;
	return 1;
    }
  
    while ((s = strtok(NULL, sep))) {
	argv[argc++] = s;
    }
    return argc;
}

void split_last(const char* src, char* first, char* last)
{
    const char*	ptr;

    if(!(ptr = strrchr(src, ' ')))
    {
	strcpy(first, src);
	last[0] = 0;
    }
    else
    {
	strncpy(first, src, ptr - src);
	first[ptr - src] = 0;
	strcpy(last, ptr + 1);
    }
}
/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                                    */
char *one_argument(const char *argument, char *first_arg )
{
  char* arg;
  
  do
  {
    /* Find first non blank */
    while(isspace(*argument))
      argument++;
    
    /* Find length of first word */
    for(arg = first_arg ; *argument && !isspace(*argument) ; arg++, argument++)
      *arg = LOWER(*argument);
    *arg = '\0';
  }
  while( fill_word(first_arg));

  return (char*) argument;
}
 
 
 
void only_argument(const char *argument, char *dest)
{
  while (*argument && isspace(*argument))
    argument++;
  strcpy(dest, argument);
}
 
 
int is_number(const char *str)
{
  int pcount=0;
   
  while(*str && (isdigit(*str) || ((*str == '.') && (++pcount))))
    str++;
  
  return (*str==0)&&(pcount<2);
}
 
const char *fill[]=
{ "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
    };

int fill_word(const char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}
 
 
 
 
 
/* determine if a given string is an abbreviation of another */
int is_abbrev(const char *arg1, const char *arg2)
{
  if (!*arg1)
    return(0);
  
  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return(0);
  
  return(1);
}
 
 
 
 
/* return first 'word' plus trailing substring of input string */
void half_chop(const char *string, char *arg1, char *arg2)
{
  while(isspace(*string))
    string++;
  
  while(*string && !isspace(*string))
    *arg1++ = *string++;
  *arg1 = '\0';
  
  while(isspace(*string))
    string++;

  strcpy(arg2, string);
}
 
int search_block(const char *arg, const char **list, bool exact)
{
  register int i,l;
  register char* ptr;
  
  /* Make into lower case, and get length of string */
  ptr = lower(arg);
  
  if (exact) {
    for(i=0; **list != '\n'; i++, ++list)
      if (!strcmp(ptr, *list))
        return(i);
  } else if ((l = strlen(ptr))) {
    for(i=0; **list != '\n'; i++, ++list)
      if (!strncmp(ptr, *list, l))
        return(i);
  }
  
  return(-1);
}
 
 
int old_search_block(const char *argument,
		     int begin, int length,
		     const char **list, int mode)
{
  int guess, search;
  const char* ptr, *arg;
  
  /* If the word contain 0 letters, then a match is already found */
  if(length < 1)
    return 0;
  
  /* Search for a match */
  argument += begin;
  
  for(guess = 1 ; **list != '\n' ; ++list, ++guess)
  {
    for(ptr = *list, arg = argument, search = length ;
	(search > 0) && (*ptr == *arg) ; ++ptr, ++arg, --search)
      ;

    if((search == 0) && (!mode || (*ptr == 0)))
      return guess;
  }
  
  return -1;
}

