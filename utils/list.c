#include "config.h"

#include <stdio.h>
#include <time.h>

#include "structs.h"
#include "db.h"
#include "utils.h"

#define GET_SEX_TEXT(s)            \
	(s == SEX_NEUTRAL ? 'N' :  \
	(s == SEX_MALE ? 'M' :     \
	(s == SEX_FEMALE ? 'F' : 'U')))

#define GET_RACE_TEXT(r)\
	(r == 1 ? 'H' : \
	(r == 2 ? 'E' : \
	(r == 3 ? 'D' : \
	(r == 4 ? 'B' : \
	(r == 5 ? 'G' : \
	(r == 17 ? 'T' : 'U'))))))

#define IS_SET(flag,bit)  ((flag) & (bit))

#define	DoClasses	0x0001
#define	DoLevels	0x0002
#define DoAge		0x0004
#define DoFlags		0x0008
#define DoHostname	0x0010
#define DoRaceSex	0x0020
#define	DoVersion	0x0040
#define DoPlayTime	0x0080
#define DoExperience	0x0100
#define DoMultis	0x0200

#define	DefaultFlags	(DoClasses|DoLevels|DoAge|DoFlags|DoRaceSex)

char* player_dir = "players.d";
int file_name = 0;
int flags = 0;
int ClassMask = -1;

void ListPlayer(char* name);
void ListDirectory(void);
void list(struct char_data* ch);

void Usage(const char* name)
{
    fprintf(stderr,
	    "usage: %s [-d <directory] [-fclaxhrvepm] [<player>|<file>]\n"
	    "	-f	args are files, not player names\n"
	    "	-c	display classes\n"
	    "	-l	display levels\n"
	    "	-a	display age\n"
	    "	-x	display extra flags (delete, protect)\n"
	    "	-h	display hostname\n"
	    "	-r	display race and sex\n"
	    "	-v	display file version\n"
	    "	-e	display experience\n"
	    "	-p	display playing time\n"
	    "	-m	display multi-classed chars only\n"
	    "	-C <CM>	display only chars matching CM\n" 
	    "	default flags are -claxr\n",
	    name);
    exit(1);
}

int main(int argc, char **argv)
{
    int opt;
    extern int optind;
    extern char* optarg;
    
    while((opt = getopt(argc, argv, "d:f?claxhrvepmC:")) != EOF)
    {
	switch(opt)
	{
	case 'd':	player_dir = optarg;		break;
	case 'f':	file_name = !file_name;		break;
	case 'c':	flags |= DoClasses;		break;
	case 'l':	flags |= DoLevels;		break;
	case 'a':	flags |= DoAge;			break;
	case 'x':	flags |= DoFlags;		break;
	case 'h':	flags |= DoHostname;		break;
	case 'r':	flags |= DoRaceSex;		break;
	case 'v':	flags |= DoVersion;		break;
	case 'e':	flags |= DoExperience;		break;
	case 'p':	flags |= DoPlayTime;		break;
	case 'm':	flags |= DoMultis;		break;
	case 'C':	ClassMask = atoi(optarg);	break;
	    
	case '?':
	default:
	    Usage(argv[0]);
	}
    }

    if(!flags)
	flags = DefaultFlags;
    
    if(optind < argc)
	while(optind < argc)
	    ListPlayer(argv[optind++]);
    else
	ListDirectory();

    return 0;
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
    
    if(read_player(ch, fp, READ_PLAYER))
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

    while(ch = ReadPlayer(dir, READ_PLAYER))
    {
	list(ch);

	free_char(ch);
    }

    ClosePlayers(dir);
}

void list(struct char_data *ch)
{
    char *point;
    char class_text[MAX_LEVEL_IND + 2];
    int num, days;
    long played;

    played = 0;

    if(!(ch->player.clss & ClassMask))
	return;
    
    if((flags & DoMultis) && (HowManyClasses(ch) == 1))
	return;
    
    if(flags & DoVersion)
	printf("(%2d) ", ch->file_version);
    
    printf("%-15s ", ss_data(ch->player.name));

    if(flags & DoLevels)
    {
	if(flags & DoMultis)
	    printf("%d", HowManyClasses(ch));
	printf("<%2d %2d %2d %2d %2d %2d %2d %2d %2d> ",
	       ch->player.level[0], ch->player.level[1],
	       ch->player.level[2], ch->player.level[3],
	       ch->player.level[4], ch->player.level[5],
	       ch->player.level[6], ch->player.level[7],
	       ch->player.level[8]);
    }
    
    if(flags & (DoRaceSex|DoClasses))
	printf("[");

    if(flags & DoRaceSex)
	printf("%c%c", GET_SEX_TEXT(ch->player.sex),
	       GET_RACE_TEXT(ch->race));

    if(flags & DoClasses)
    {
	if (IS_SET(ch->player.clss, CLASS_MAGIC_USER))
	    class_text[0] = 'M';
	else
	    class_text[0] = ' ';

	if (IS_SET(ch->player.clss, CLASS_CLERIC))
	    class_text[1] = 'C';
	else
	    class_text[1] = ' ';

	if (IS_SET(ch->player.clss, CLASS_WARRIOR))
	    class_text[2] = 'W';
	else
	    class_text[2] = ' ';

	if (IS_SET(ch->player.clss, CLASS_THIEF))
	    class_text[3] = 'T';
	else
	    class_text[3] = ' ';

	if (IS_SET(ch->player.clss, CLASS_PALADIN))
	    class_text[4] = 'P';
	else
	    class_text[4] = ' ';

	if (IS_SET(ch->player.clss, CLASS_DRUID))
	    class_text[5] = 'D';
	else
	    class_text[5] = ' ';

	if (IS_SET(ch->player.clss, CLASS_PSI))
	    class_text[6] = 'S';
	else
	    class_text[6] = ' ';

	if (IS_SET(ch->player.clss, CLASS_RANGER))
	    class_text[7] = 'R';
	else
	    class_text[7] = ' ';

	if (IS_SET(ch->player.clss, CLASS_SHIFTER))
	    class_text[8] = 'H';
	else
	    class_text[8] = ' ';

	printf("%.*s", MAX_LEVEL_IND, class_text);
    }

    if(flags & (DoRaceSex|DoClasses))
	printf("] ");

    if(flags & DoAge)
    {
	days = (time(0)-ch->player.time.logon)/SECS_PER_REAL_DAY;

	if (days > 30)
	    printf("[DAY %2d]*", days);
	else if (days >= 1)
	    printf("[DAY %2d] ", days);
	else
	    printf("[--- --] ");
    }

    if(flags & DoFlags)
    {
	if IS_SET(ch->delete_flag, 1)
	    printf("D");
	else
	    printf(" ");
	if IS_SET(ch->delete_flag, 2)
	    printf("P");
	else
	    printf(" ");
	if IS_SET(ch->delete_flag, 4)
	    printf("H");
	else
	    printf(" ");
    }

    if(flags & DoHostname)
	printf("%s",
	       ch->specials.hostname ? ch->specials.hostname : "**UNKNOWN**");

    if(flags & DoExperience)
	printf(" %9d", ch->points.exp);

    if(flags & DoPlayTime)
	printf(" %3d days", ch->player.time.played / SECS_PER_REAL_DAY);
    
    printf("\n");
}

