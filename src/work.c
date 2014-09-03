int name_match(char *match,char *name)
{
        unsigned int i;
        for(i=0;i<strlen(match);i++) {
                if(LOWER(match[i])!=LOWER(name[i])) return 0;
                }
        return 1;
}                             

void do_who(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH], line1[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH]; /* the who output buffer */
    int count, gods, idle, mort, levels, title, hits;
    int stats, dead, classes, zone, lcount, listed, live;
    int rname, flags, wanted;
    int singles, multis, both;	
    char match[256];
    struct char_data* person;
    struct char_data* player;
    struct room_data* roomp;
    int godmort, godwho;
    int logall, builder;
    static char *godlevel[10] = {"HERO","CLRK","CLRK","CLRK",
				 "ADMN","ADMN","ADMN","AIMP",
				 "CIMP","*IMP" };
    
    /* defaults */
    godmort = FALSE;		/* one of g/o specified */
    gods = TRUE;
    mort = TRUE;
    both = FALSE;		/* one of m/n specified */
    singles = TRUE;
    multis = TRUE;
    levels = FALSE;
    title = FALSE;
    hits = FALSE;
    stats = FALSE;
    dead = FALSE;
    live = TRUE;
    zone = FALSE;
    idle = FALSE;
    rname = FALSE;
    flags = FALSE;
    classes = -1;
    godwho = FALSE;
    wanted = FALSE;
    builder = FALSE;
    logall = FALSE;
    count = 0;
    lcount = 0;
    listed = 0;

    /* if this is a god, deal with "-" arguments */
    while(isspace(*argument))
	argument++;

    if(IS_GOD(ch) && (*argument == '-'))
    {
	godwho = TRUE;
	while(*++argument && !isspace(*argument))
	{
	    switch(*argument)
	    {
	    case 'i':   idle = TRUE;			break;
	    case 'l':   levels = TRUE;			break;
	    case 't':   title = TRUE;			break;
	    case 'h':   hits = TRUE;			break;
	    case 's':   stats = TRUE;			break;
	    case 'd':   dead = TRUE;	live = FALSE;	break;
	    case 'z':   zone = TRUE;			break;
	    case 'r':   rname = TRUE;			break;
	    case 'a':	dead = TRUE;	live = TRUE;	break;
	    case 'f':	flags = TRUE;			break;
	    case '$':
		if (TRUST(ch) <= TRUST_GRGOD)
		    logall=FALSE; else logall=TRUE;
		break;
	    case 'b':	builder = TRUE;			break;
	    case 'm':	
	    case 'n':
		if (!both)
		{
		    both = TRUE;
		    singles = FALSE;
		    multis = FALSE;
		}
		if(*argument == 'n')
		    singles = TRUE;
		else
		    multis = TRUE;
		break;
	    case 'g':
	    case 'o':
		if(!godmort)
		{
		    godmort = TRUE;
		    gods = FALSE;
		    mort = FALSE;
		}
		if(*argument == 'g')
		    gods = TRUE;
		else
		    mort = TRUE;
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
            case '7':
	    case '8':
	    case '9':
		if(classes == -1)
		    classes = 0;
		classes |= 1 << (*argument - '1');
		break;
	    case 'w':	wanted = TRUE;			break;
	    default:
		send_to_char("[-]i=idle l=levels t=title h=hit/mana/move s=stats\n\r",ch);
	        if(TRUST(ch) > TRUST_GRGOD)
		    send_to_char("[-]$=logall\n\r",ch);
		send_to_char("[-]r=real names b=builder\n", ch);
		send_to_char("[-]d=linkdead g=God o=Mort a=dead&active\n\r",
			     ch);
	        send_to_char("[-]m=multis only n=singles only\n\r",ch);
		send_to_char("[1]Ma [2]Cl [3]Wa [4]Th [5]Dr [6]Ra [7]Ps [8]Pa [9]Sh\n\r", ch);
		send_to_char("--------\n\r", ch);
		return;
	    }
	}
    }
    
    /*  check for an arg */
    only_argument(argument, match);
    if(!strcmp(match, "god") || !strcmp(match, "gods"))
    {
	mort = FALSE;
	match[0] = 0;
    }
    else if(!strcmp(match, "wanted"))
    {
	wanted = TRUE;
	match[0] = 0;
    }
    
    if((cmd == 234) || zone)
    {
	roomp = real_roomp(ch->in_room);
	zone = !roomp ? 0 : roomp->zone;
    }
    else
	zone = -1;
    
    strcpy(output, ""); /* init the string */

    if(!godwho)     /* Want all Players to come after Player Title */
      strcat(output, "Players\n\r"
		     "------------------------------------\n\r");
    else
      strcat(output, "Players [God Version -? for help]\n\r---------\n\r");

    EACH_CHARACTER(iter, person) {

	if (!IS_PC(person) || (person->in_room == 3)) /* skip mobs or players behind polies */
	    continue;
	
	if (zone > -1) {
	    roomp = real_roomp(person->in_room);
	    if (!roomp || (roomp->zone != zone)) 
		continue;
	}
	
	//if (!CAN_SEE(ch, person)) /* can the player be seen */
	//    continue;
	
	count++;
	
	if (!person->desc)  /* see if they pass any link dead tests */
	{    
	    lcount++;
	    if(!dead)
		continue;
	}
	else if (!live)
	    continue;

	/* apply the filters */
	if (IS_GOD(person) && !IS_SET(person->specials.flags, PLR_NOWIZ))
	{
	    if (!gods)
		continue;
	}
	else if (!mort)
	    continue;

	if(HowManyClasses(person)==1)
	{
	    if (!singles)
		continue;
	}
	else if(!multis)
	    continue;
	
	if (!(classes & person->player.clss))
	    continue;
	
	player = real_character(person); /* find the real person here */
	
	if (match[0] && !name_match(match, GET_REAL_NAME(player)))
	    continue;
	
	if (wanted && !(person->specials.flags & (PLR_PKILLER|PLR_LOSER|PLR_THIEF)))
	    continue;
	
	listed++;
	    
	if (cmd == 234)  /* ok ready to display them */
	{
	    char* name, namebuf[256];
	    if (IS_NPC(person))
		sprintf(name = namebuf, "%s (%s)",
			GET_NAME(person), GET_NAME(player));
	    else
		name = GET_NAME(person);
	    
	    roomp = real_roomp(person->in_room);
	    if (IS_IMMORTAL(ch))
		sprintf(buf, "%-32s - %s [%d]\n\r",
			name,
			roomp ? roomp->name : "Only gods know where",
			person->in_room);
	    else
		sprintf(buf, "%-40s - %s\n\r",
			name,
			roomp ? roomp->name : "Only gods know where");
	    strcat(output, buf);
	}
	else if (!godwho)
	{
	    if (CheckColor(ch))
	    {
		/* precurse with god level if its a god */
		if (IS_GOD(player) &&
		    !IS_SET(player->specials.flags, PLR_NOWIZ)) 
		{
		    sprintf(buf,"[%s%4s%s] ",
			    ANSI_RED,
			    godlevel[TRUST(player) - 1],
			    ANSI_NORMAL);
		    strcat(output, buf);
		}
		
		/* add on the rest of the info */
		sprintf(buf, "%s%s%s %s [%s%8s%s]",
			ANSI_YELLOW, GET_NAME(player), ANSI_NORMAL,
			ss_data(player->player.title),
			ANSI_BLUE,
			guildname(player->player.guildinfo.inguild()),
			ANSI_NORMAL);
	    }
	    else
	    {
		/* god title first */
		if (IS_GOD(player) &&
		    !IS_SET(player->specials.flags, PLR_NOWIZ))
		{
		    sprintf(buf,"[%4s] ",
			    godlevel[TRUST(player) - 1]);
		    strcat(output, buf);
		}
		
                /* normal player info */
		sprintf(buf, "%s %s [%8s]",
			GET_NAME(player),
			ss_data(player->player.title),
			guildname(player->player.guildinfo.inguild()));
	    }
	    
	    if (IS_GOD(ch))
		if (person->invis_level > 0)
		    sprintf(buf, "%s (I: %2d)", buf, person->invis_level);

	    if (IS_GOD(ch) || wanted)
	    {
		if (CheckColor(ch))
		{
		    if(IS_SET(person->specials.flags, PLR_LOSER))
			sprintf(buf, "%s %s(OPEN)%s", buf,
				ANSI_BRIGHT_RED, ANSI_NORMAL);
		    if(IS_SET(person->specials.flags, PLR_PKILLER))
			sprintf(buf, "%s %s(PKILLER)%s", buf,
				ANSI_BRIGHT_MAGENTA, ANSI_NORMAL);
		    if(IS_SET(person->specials.flags, PLR_THIEF))
			sprintf(buf, "%s %s(THIEF)%s", buf,
				ANSI_BRIGHT_BLUE, ANSI_NORMAL);
		}
		else
		{
		    if(IS_SET(person->specials.flags, PLR_LOSER))
			strcat(buf, " (OPEN)");
		    if(IS_SET(person->specials.flags, PLR_PKILLER))
			strcat(buf, " (PKILLER)");
		    if(IS_SET(person->specials.flags, PLR_THIEF))
			strcat(buf, " (THIEF)");
		}
	    }
	    
	    if(IS_AFK(player))
		strcat(buf, " (AFK)");
	    if(IS_WRITING(player))
		strcat(buf, " (WRITING)");
	    strcat(buf, "\n\r");

	    /* here's where the sorting comes into play */
	    strcat(output, buf);
	}
	else
	{
	    if(rname)
		sprintf(buf, "%-14s ", GET_REAL_NAME(person));
	    else
		sprintf(buf, "%-14s ", GET_NAME(player));
	  
	    if(idle)
	    {
		sprintf(line1, "Idle: %3d ", person->specials.timer);
		strcat(buf, line1);
	    }
	    if(levels)
	    {
		sprintf(line1,"Level:[%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d/%2d] ",
			person->player.level[0],person->player.level[1],
			person->player.level[2],person->player.level[3],
			person->player.level[4],person->player.level[5],
			person->player.level[6],person->player.level[7],
			person->player.level[8],person->player.level[9],
			person->player.level[10]);
		strcat(buf,line1);
	    }
	    if(hits)
	    {
		sprintf(line1,"Hit:[%3d] Mana:[%3d] Move:[%3d] ",
			GET_HIT(person),GET_MANA(person),GET_MOVE(person));
		strcat(buf, line1);
	    }
	    if(stats)
	    {
		sprintf(line1,"[S:%2d I:%2d W:%2d C:%2d D:%2d CH: %2d] ",
			GET_STR(person),GET_INT(person),GET_WIS(person),
			GET_CON(person),GET_DEX(person),GET_CHA(person));
		strcat(buf,line1);
	    }
	    if(title)
	    {
		sprintf(line1," %s",ss_data(player->player.title));
		strcat(buf,line1);
	    }
	    if(flags)
	    {
		if(player->invis_level > 0)
		{
		    sprintf(line1,"(I: %2d) ", player->invis_level);
		    strcat(buf,line1);
		}
		if(IS_AFK(player))
		    strcat(buf, "(AFK) ");
		if(IS_WRITING(player))
		    strcat(buf, "(WRITING) ");
	    }
	    if(builder)
		if(IS_SET(player->specials.flags, PLR_BUILDER))
		    strcat(buf,"(BLDR) ");
	    if(logall)
		if(IS_SET(player->specials.flags, PLR_LOGALL))
		    strcat(buf,"(LOG) ");
	    
	    strcat(buf, "\n\r");
	    strcat(output, buf);
	}
    }
    END_AITER(iter);
    
    if(!godwho)
    {
	sprintf(buf, "\n\rTotal visible %s: %d\n\r\n\r",
		mort ? "players" : "heroes", listed);
    }
    else if(listed == 0)
    {
	sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\r",
		count,lcount,
		count ? ((double)lcount / (double)count) * 100. : 0.);
    }
    else
    {
	sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%) Number Listed: %d\n\r",
		count,lcount,
		count ? ((double)lcount / (double)count) * 100. : 0.,listed);
    }

    strcat(output, buf);
    
    page_string(ch->desc, output, 1); /* send the output */

}

