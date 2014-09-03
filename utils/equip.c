/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "config.h"

#include <stdio.h>
#include <time.h>

#include "structs.h"
#include "db.h"
#include "utils.h"

char*	player_dir = "players.d";
int	file_name = 0;
char*	obj_file = OBJ_FILE;

void ListPlayer(char* name);
void ListDirectory(void);
void list(struct char_data* ch);
void dump_obj(struct obj_data* obj);
void Usage(const char* name);

int main(int argc, char** argv)
{
  int opt;
  extern int optind;
  extern char* optarg;

  while((opt = getopt(argc, argv, "d:o:f")) != EOF)
  {
    switch(opt)
    {
    case 'd':		player_dir = optarg;		break;
    case 'o':		obj_file = optarg;		break;
    case 'f':		file_name = 1;			break;
      
    case '?':
    default:		Usage(argv[0]);
    }
  }

  boot_objects(obj_file);

  if(optind < argc)
    while(optind < argc)
      ListPlayer(argv[optind++]);
  else
    ListDirectory();

  return 0;
}

void Usage(const char* name)
{
  fprintf(stderr, "usage: %s -f [-d <directory>] <player>...\n", name);
  exit(1);
}

void ListPlayer(char* name)
{
    char buf[256];
    FILE* fp;
    struct char_data* ch;
    
    if(!file_name)
    {
	sprintf(buf, "%s/%c/%s", player_dir, *name, name);
	name = buf;
    }

    if(!(fp = fopen(name, "r")))
    {
	perror(name);
	return;
    }

    CREATE(ch, struct char_data, 1);
    clear_char(ch);
    clear_aliases(ch);
    SpaceForSkills(ch);
    
    if(read_player(ch, fp, READ_PLAYER|READ_OBJECTS))
    {
	fprintf(stderr, "Error reading %s\n", name);
	return;
    }

    list(ch);

    free_char(ch);
}

void ListDirectory(void)
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
	list(ch);

	free_char(ch);
    }

    ClosePlayers(dir);
}

void list(struct char_data* ch)
{
  struct obj_data*	obj;
  int			i;

  printf("*** %s ***\n", GET_NAME(ch));
  
  for(i = 0 ; i < MAX_WEAR ; ++i)
    if(obj = ch->equipment[i])
      dump_obj(obj);

  if(obj = ch->carrying)
    dump_obj(obj);
}

void dump_obj(struct obj_data* obj)
{
  int virt;
  
  while(obj)
  {
    virt = ((obj->item_number > 0) && (obj->item_number < top_of_objt))
      ? obj_index[obj->item_number].virt : -1;
    
    printf("%5d: %s\n", virt, OBJ_NAME(obj));

    if(obj->contains)
      dump_obj(obj->contains);
    obj = obj->next_content;
  }
}

    
