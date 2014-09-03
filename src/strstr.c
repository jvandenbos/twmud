#include "config.h"

#include <stdio.h>

char* strstr(char* mainstring, char* substring)
{
  char *ptr1, *ptr2;

  while(*mainstring != '\0')
  {
    while(*substring != *mainstring)
    {
      if(*mainstring == '\0')
	return NULL;
      mainstring++;
    }
    for(ptr1 = substring, ptr2 = mainstring;
	*ptr1 != '\0' && *ptr1 == *ptr2;
	ptr1++, ptr2++);
    if(*ptr1 == '\0')
      return mainstring;
    else
      mainstring++;
  }
  return NULL;
}
