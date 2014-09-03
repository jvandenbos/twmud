#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <ctype.h>
#if USE_unistd
#include <unistd.h>
#endif
#include <memory.h>
#include <signal.h>
#include <strings.h>
#if USE_time
#include <time.h>
#endif

#include "structs.h"
#include "utility.h"

FILE *stat_file;

void open_statlog(const char *str)
{
#ifndef NO_STATISTICS
  static int init=0;

  if (init)
  {
    log("Foolish coder -- don't call this function more than once!");
    return;
  }

  if (!str)
  {
    log("Please specify a valid file!");
    return;
  }
  
  if(!(stat_file=fopen(str,"at")))
  {
    log("ERROR ERROR ERROR Cannot open STAT_LOG! ERROR ERROR ERROR");
    return;
  }
       
  init=1;
#endif
}

void stat_log(const char *str, int flag) 
/* void stat_log(const char *str) */
{
#ifndef NO_STATISTICS
    long ct;
    char *tmstr;

    if (!stat_file)
    {
      log("ERROR ERROR ERROR call to stat_log before open!");
      return;
    }
  if (flag)
  {  
    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    fprintf(stat_file, "%s :: %s\n", tmstr, str);
  }

  else
  {  
    fprintf(stat_file,"%s\n",str);
    fflush(stat_file);
  } 
#endif
}
