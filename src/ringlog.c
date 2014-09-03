#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define RL_COUNT	4096

typedef struct 
{
  int		next;
  char		lines[RL_COUNT + 1];
} ring_log_t;

ring_log_t	RingLog;

void ring_log(const char* fmt, ...)
{
#if RL_COUNT > 0
  va_list	args;
  char*		ptr;
  char 		buf[256];
  int		len, cnt;

  /* format the message */
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  /* put as much as possible into what's left of the ring */
  for(ptr = buf, len = strlen(buf) ; len > 0 ; ptr += cnt, len -= cnt)
  {
    cnt = RL_COUNT - RingLog.next;
    if(cnt > len)
      cnt = len;
    strncpy(&RingLog.lines[RingLog.next], ptr, cnt);

    /* increment next to point beyond what we just addead and
       correct if we've gotten to the end of the buffer */
    if((RingLog.next += cnt) >= RL_COUNT)
      RingLog.next = 0;
    RingLog.lines[RingLog.next] = 0;
  }
#endif
}

void rl_dump(void)
{
#if RL_COUNT > 0
  fprintf(stderr, ">>>>> Dumping Ring Log <<<<<\n");

  /* dump the last half, (ie., oldest) portion of the buffer */
  fprintf(stderr, "%s", RingLog.lines + RingLog.next + 1);

  /* dump the first half, (ie., more recent) portion of the buffer */
  fprintf(stderr, "%s", RingLog.lines);

  fprintf(stderr, ">>>>> End of Ring Log <<<<<\n");
#endif
}
