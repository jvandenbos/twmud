/*
 * Guild Code HEADER by Jan Vandenbos (c)1997 - All rights reserved.
 */


#ifndef GUILDS_H
#define GUILDS_H

#include <iostream>
#include <fstream>

#define MAXGUILD 25

#define MAXGUILDS MAXGUILD+1 /* Workaround for wierdass bug (Don't ask) */
#define MAXGUILDNAME 9
#define MAXGUILDDESC 31
#define MAXGUILDLOG 500 /* Number of entries to keep, it kills 
						   half the entries when it fills up, 
						   so don't set this to low */
#define GUILDFILE "guilds.dat"
#define GUILDLOG "guilds.log"

/* Guild Flags here */
#define GUILD_GM			( 1 << 0 )
#define GUILD_ASSISTANT		( 1 << 1 )
#define GUILD_APPLICANT		( 1 << 2 )
#define GUILD_PK			( 1 << 3)

int inguildfix(int guildnumber);

class GuildInfo {  /* info that goes into the player structure */
  
public: /* sigh had to make this all public so I could read the player easily... damn */
    
    int guildnumber;      /* the actual number of the guild a player is a member of 0=no guild */
    int guildflags;      /* flags like if player is GM or Assistant GM */
    int spare;             /* for expansion */
    GuildInfo() { guildnumber = 0; guildflags = 0; };
    
    void set(int number)                /* set the players guild number */
    { guildnumber = number; } ; 
    
    void setflag(int flag)              /* set a flag on the player */
    { guildflags = flag;  };
    
    void clear() /* remove from both guild and guildflags */
    {  guildnumber = 0; guildflags = 0; }
    
    void apply( int number ) /* apply for a guild */
    { guildnumber = number; guildflags = GUILD_APPLICANT; };
    
    void  makegm( int number ) /* make the guildmaster */
    { guildnumber = number; guildflags = GUILD_GM; };
    
    void makeassistant( int number) /* make an assistant */
    { guildnumber = number; guildflags = GUILD_ASSISTANT; };
    
    void makenew(int number) /* enter a player into a guild */
    { guildnumber = number; guildflags = 0; };
    
    bool applied_for(int number) /* check if player applied for a guild */
    { return ((guildnumber == number) && (guildflags == GUILD_APPLICANT)); };
    
    bool isgm() { return (guildflags == GUILD_GM); }
    
    bool istrust() { return ((guildflags == GUILD_GM) || (guildflags == GUILD_ASSISTANT)) ; }

    bool isassist() { return (guildflags == GUILD_ASSISTANT); };

    int inguild() {guildnumber=inguildfix(guildnumber); return guildnumber; };
    

};



/*
 * The guild structure. - contains macro information on the guild
 * guildinfo above is whats stored in each player/mob
 */

#define GUILD_EMPTY		( 1 << 0 )
#define GUILD_ACTIVE	( 1 << 1 )
#define GUILD_DELETED	( 1 << 2 )
#define GUILD_PKILL		( 1 << 3 )




struct Guild 
{
    int guildflags; /* guild flags */
    char guildname[MAXGUILDNAME];
    char guilddesc[MAXGUILDDESC];
    EXP guildgold;
    EXP guildexp;
    int guildspells;
    int guildskills;
};


#define GET_GGOLD(num) (guilds[(num)].guildgold)
#define GET_GEXP(num) (guilds[(num)].guildexp)

/* Trans types */
#define GUILD_DEPOSIT_EXP 1
#define GUILD_DEPOSIT_GOLD 2
#define GUILD_WITHDRAW 3

struct Guild_log 
{
    char name[20];
	int type;
	int amount;
	int gnum;
};


/* Guild Costs */
#define GUILD_GCOST_INIT 5000000

#endif











