#include <stdio.h>
#include <string.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include "config.h"
#include "path.h"

#define DATAPATH "/home_u/ucpflugf/mud/current/src/data"

char *court_files[2];

void boot_paths()
{
  char tmp[BUFSIZ];

  /*Set the path for spec_court.c functions*/
  sprintf(buf, "%s/%s",DATAPATH,"confirmed");
  CREATE(court_files[0], char, strlen(buf) + 1);
  sprintf(buf, "%s/%s",DATAPATH,"unconfirmed");
  CREATE(court_files[1], char, strlen(buf) + 1);
}

  
  
