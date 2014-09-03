/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "config.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "util_str.h"

char* player_dir = "players.d";
char* backup_dir = 0;
char* script = "purge.list";
FILE* sfp;

void PurgeDirectory(void);
int purge(struct char_data* ch);

#define NEWBIE_MAX_LEVEL	3
#define NEWBIE_MAX_AGE		5
#define HOUSED_MAX_AGE		120
#define MORTAL_MAX_AGE		30
#define IMMORTAL_MAX_AGE	90

void Usage(const char* name)
{
    fprintf(stderr,
	    "syntax: %s [-d <directory] [-b <backupdir>] [-s <script>]\n",
	    name);
    exit(1);
}

int main(int argc, char **argv)
{
    int opt;
    extern int optind;
    extern char* optarg;
    long now;
    struct tm* tm;
    
    while((opt = getopt(argc, argv, "d:b:s:?")) != EOF)
    {
	switch(opt)
	{
	case 'd':
	    player_dir = optarg;
	    break;

	case 'b':
	    backup_dir = optarg;
	    break;

	case 's':
	    script = optarg;
	    break;

	case '?':
	default:
	    Usage(argv[0]);
	}
    }

    if(!backup_dir)
    {
	static char buf[256];
	sprintf(backup_dir = buf, "%s/back", player_dir);
    }

    if(!(sfp = fopen(script, "w")))
    {
	perror(script);
	exit(1);
    }
    
    fprintf(sfp, "find %s -type f -name \\*.bak -print | xargs rm -f\n",
	    player_dir);
    
    if(access(backup_dir, W_OK))
	fprintf(sfp, "mkdir %s\n", backup_dir);

    PurgeDirectory();

    time(&now);
    tm = localtime(&now);
    
    fprintf(sfp, "tar cf - %s %s | gzip > purged-%02d%02d.tar.gz\n",
	    script, backup_dir, tm->tm_mon + 1, tm->tm_mday);
    fprintf(sfp, "rm -rf %s\n", backup_dir);
    fprintf(sfp, "mkdir %s\n", backup_dir);

    return 0;
}

void PurgeDirectory(void)
{
    PlayerDir*		dir;
    struct char_data*	ch;
    int			total, deleted;
    
    if(!(dir = OpenPlayers(player_dir)))
    {
	perror(player_dir);
	return;
    }

    total = deleted = 0;
    
    while(ch = ReadPlayer(dir, READ_PLAYER))
    {
	total++;
	if(purge(ch))
	    deleted++;

	free_char(ch);
    }

    ClosePlayers(dir);

    fprintf(sfp, "# %d total players\n", total);
    fprintf(sfp, "# %d deleted players\n", deleted);
    fprintf(sfp, "# %d current players\n", total - deleted);
}

int purge(struct char_data *ch)
{
  int prot, maxlevel, delete, days, bound;
    
  if(IS_SET(ch->delete_flag, PROTECT))
    return 0;

  maxlevel = GetMaxLevel(ch);
  days = (time(0) - ch->player.time.logon) / SECS_PER_REAL_DAY;
  bound = ((maxlevel <= NEWBIE_MAX_LEVEL) ? NEWBIE_MAX_AGE :
	   (maxlevel <= 50) ? MORTAL_MAX_AGE : IMMORTAL_MAX_AGE);
    
  delete = 0;
    
  if(IS_SET(ch->delete_flag, DELETE))
  {
    fprintf(sfp, "# %s: delete flag set\n", GET_IDENT(ch));
    delete = 1;
  }
  else if(IS_SET(ch->delete_flag, HOUSED))
  {
    if(days > HOUSED_MAX_AGE)
    {
      fprintf(sfp, "# %s: housed: AGE %d\n", GET_IDENT(ch), days);
      delete = 1;
    }
  }
  else if(days > bound)
  {
    fprintf(sfp, "# %s: MaxLevel: %d:  Age: %d\n",
	    GET_IDENT(ch), maxlevel, days);
    delete = 1;
  }

  if(delete)
  {
    char* l = lower(GET_IDENT(ch));
	
    fprintf(sfp, "mv %s/%c/%s %s\n",
	    player_dir, *l, l, backup_dir);
  }

  return delete;
}
