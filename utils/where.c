/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "../src/config.h"

#include <stdio.h>
#include <time.h>

#include "../src/structs.h"
#include "../src/db.h"
#include "../src/utils.h"

int	rnum;
char*	name;

char*	player_dir = "players.d";
char*	obj_file = OBJ_FILE;

void WhereDirectory(void);
void search_object(struct char_data* ch, struct obj_data* obj);
void search_player(struct char_data* ch);
void Usage(const char* name);

int main(int argc, char** argv)
{
  int opt;
  extern int optind;
  extern char* optarg;

  while((opt = getopt(argc, argv, "d:o:")) != EOF)
  {
    switch(opt)
    {
    case 'd':		player_dir = optarg;		break;
    case 'o':		obj_file = optarg;		break;
      
    case '?':
    default:		Usage(argv[0]);
    }
  }

  boot_objects(obj_file);
  
  if(isdigit(*argv[optind]))
    rnum = real_object(atoi(argv[optind]));
  else
    name = argv[optind];
  
  WhereDirectory();
}

void Usage(const char* name)
{
  fprintf(stderr, "usage: %s [-d <directory>] <obj_spec>\n", name);
  exit(1);
}

void WhereDirectory(void)
{
  PlayerDir*		dir;
  struct char_data*	ch;

  if(!(dir = OpenPlayers(player_dir)))
  {
    perror(player_dir);
    return;
  }

  while(ch = ReadPlayer(dir, READ_PLAYER|READ_OBJECTS))
  {
    search_player(ch);
    free_char(ch);
  }

  ClosePlayers(dir);
}

void search_player(struct char_data* ch)
{
  struct obj_data*	obj;
  int			i;
  
  for(obj = ch->carrying ; obj ; obj = obj->next_content)
  {
    search_object(ch, obj);
  }

  for(i = 0 ; i < MAX_WEAR ; ++i)
  {
    if(obj = ch->equipment[i])
      search_object(ch, obj);
  }
}

void search_object(struct char_data* ch, struct obj_data* obj)
{
  if(name ? isname(name, ss_data(obj->name)) : obj->item_number == rnum)
    printf("%-20.20s %s\n",
	   ss_data(ch->player.name),
	   ss_data(obj->name));

  for(obj = obj->contains ; obj ; obj = obj->next_content)
    search_object(ch, obj);
}
