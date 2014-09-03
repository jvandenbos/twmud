/*
##  To add some new debugging stuff (mostly limited to malloc_debug)
*/
#ifndef MEM_DEBUG
#define	MEM_DEBUG	0
#endif 

#include "system.h"

/*
 *  Default port to use
 */
#define DFLT_PORT 6000

/*
 *  Default database directory
 */
#define DFLT_DIR "../lib"

/*
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
*/
/*#define	HASH*/

/*
##  To get rid of tracking (lower cpu)
*/
#define	NOTRACK	0	

/*
##  To force new players to be authorized
*/
#define	PLAYER_AUTH    1 

/*
##  To add some commands for locking out specific hosts
*/
#define	SITELOCK	1

/*
##  To add some more commands for locking out specific hosts
*/
#define	REAPLOCK	1

/*
##  To add some more commands for locking out specific hosts
*/
#define	USERLOCK	1

/*
##  To allow wildcards in locked out hosts
*/
#define WILDCARDS	1

/*
##  To modify some commands to make item duplication near impossible:
*/
#define	NODUPLICATES	1

/*
## from here on down is various maximum table sizes and other
## tunable constants
*/
/*
##  Site locking stuff.. written by Scot Gardner
*/
#define MAX_BAN_HOSTS 65
 
/*
**  Limited item Stuff
*/
#define LONG_TERM_MULT	   2	   /* multiply normal rent by this to get */
				   /* long term rent */
 
/*
##
*/
#define MAX_ROOMS   10000

/*
## limits for logging *ALL* commands
*/
#define LOW_LOG_ALL	TRUST_GRUNT
#define HIGH_LOG_ALL	TRUST_IMP

/*
## how to deal with damage for M_immune (immunity)
## alternatives are (0) or (dam / 4)
*/
#define USE_real_immunity	0
#if USE_real_immunity
#define IMMUNE_DAMAGE(dam)	0
#else
#define IMMUNE_DAMAGE(dam)	((int)(dam*.05))
#define RESIST_DAMAGE(dam)      ((int)(dam*.5))
#define SUSC_DAMAGE(dam)        (dam*2)
#endif

/*
 * The maximum level to allow free grouping, and the maximum
 *  range to allow to group at all
 */
#define MAX_FREE_GROUP	         51
#define MAX_GROUP_RANGE	         0

/*
 * set 1 to take rent out of players bank account.
 */
#define PLUNDER_BANK		1

/*
 * how much extra to charge a player to repair a weapon
 */
#define SCALE_WEAPON_COST(cost) (cost * 1)

/*
 * give ourselves an easy way to comment out code...
 */
#define NOT_IMPLEMENTED		0

/*
 * whether or not to include a fancy line editor...
 */
#define LINE_EDITOR		1
#define EDITOR_HELP_TOPIC	"lineeditor"

/*
 * set to 1 if immortals should be allowed to nohassle players, thus
 * keeping a player from being attacked by agg mobs...
 */
#define NOHASSLE_MORTALS	0
#define STEALTH_MORTALS		0

/*
 * set to 1 to kludge together restrictions for the new classes (psi,
 * druid, ranger, paladin)
 */
#define KLUDGE_RESTRICTIONS    0 

/*
 * if mobs should trigger teleport rooms too
 */
#define MOBS_TRIGGER_TELEPORT   0	

/* DEFINE the max_exist number to achieve a once per reboot item load */
#define ONCE_PER_REBOOT	 1
#define UNIQUE_LOAD 0

/* Uncomment, to turn newhelp ON.  NOTE:: not operational yet */
/*#define NEWHELP*/

/* Uncomment, to REMOVE statistics handling, i.e. stat_log */
#define NO_STATISTICS

/* DEFINES MAX EXP obtainable per kill */
#define MAXEXP 999999999

/* Uncomment, to REMOVE smart monsters */
/*#define NO_SMART_MOBS*/

/* Uncomment, TO PERFORM zone swapping */
/* Code activated via this option is Copyright(c)1994 Paul A Cole */
#define SWAP_ZONES

/* Comment to REMOVE morgue -- corpses go to 3001(portal) if commented */
/* Code activated via this option is Copyright(c)1994 Paul A Cole */
#define MORGUE_ON

#define PLAYER_VERSION		33

/* Standard Diku code assumes everyone has four hands, and standard TW       */
/* players are whiners unless you have all four.  You can choose your number */
/* by changing NUM_OF_HANDS                                                  */
#define NUM_OF_HANDS 4

/* if you have a slow nameserver, define SLOWNAMES to 1, otherwise 0 */
#define SLOWNAMES 0
