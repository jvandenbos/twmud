#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#include <string.h>

#include "structs.h"
#include "utils.h"
#include "modify.h"
#include "proto.h" 
#include "proto.h"

void init_string_block(struct string_block *sb)
{
    sb->size = 50;
    CREATE(sb->data, char, sb->size);
    *sb->data = '\0';
}

void append_to_string_block(struct string_block *sb, const char *str)
{
  int	len;
  len = strlen(sb->data) + strlen(str) + 1;
  if (len > sb->size) {
    if ( len > (sb->size*=2))
      sb->size = len;
    RECREATE(sb->data, char, sb->size);
  }
  strcat(sb->data, str);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
  page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
  if(sb->data)
  {
    FREE(sb->data);
    sb->data = NULL;
  }
}

