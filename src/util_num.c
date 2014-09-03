#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utility.h"

#undef MIN
int MIN(int a, int b)
{
    return a < b ? a:b;
}

#undef MAX
int MAX(int a, int b)
{
    return a > b ? a:b;
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
    if (to - from + 1 )
	return((random() % (to - from + 1)) + from);
    else
	return(from);
}

/* simulates dice roll */
int dice(int number, int size) 
{
    int r;
    int sum = 0;

    if (size < 0) {
	slog("Dice size less than 0!");
	return(0);
    }

    if (size==0)
      return (0);

    for (r = 1; r <= number; r++) sum += ((random() % size)+1);
    return(sum);
}
