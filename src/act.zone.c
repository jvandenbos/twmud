#include "config.h"
#include "structs.h"
#include <string.h>
#include <stdlib.h>
#include "db.zonehelp.h"
#include "sblock.h"
#include "proto.h"

void do_zones(struct char_data *ch, char *argument, int cmd)
{
  struct zoneh_data *zoneh;
  struct string_block sb;
  char buf[MAX_STRING_LENGTH];
  char zonename[100];
  
  one_argument(argument, zonename);

  /*
   * No args, show default zone list
   */
  if(strcmp(zonename,"")==0)
    page_string_block(&zonehelplist, ch);
  /*
   * l<level> or L<level> -- show areas of that sugested level
   */
  else
    if( ((*zonename=='L') || (*zonename=='l')) &&
       (is_number(zonename+1)))
    {
      page_string_block(zoneh_list_by_level(&sb,atoi(zonename+1)),ch);
      destroy_string_block(&sb);
    }
  /*
   * Else, try to find area by name or number and display specific info
   * about it.
   */
    else
    {
      if ( (zoneh=find_zoneh(zonename)) == NULL)
	send_to_char("That zone does not seem to exist.\n\r",ch);
      else
      {
	sprintf(buf,"\n\r%s by %s %s\n\rLevels %i through %i\n\r\n\r"
	      "Directions:\n\r%s\n\r\n\rOverview:\n\r%s\n\r\n\r"
		"Update info:\n\r%s\n\r",
		zoneh->name,zoneh->creator,zoneh->create_date,
		zoneh->min_level,zoneh->max_level,zoneh->directions,
		zoneh->description,zoneh->update_info);
	page_string(ch->desc,buf,0);
      }
    }
}
