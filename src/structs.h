
#ifndef STRUCTS_H
#define STRUCTS_H

/*  For debuggin throughout the code.  use DLOG((FORMAT)) Where FORMAT may be
 a printf style format and variables to log.
 ex: DLOG(("Name %s\r\n", ss_data(mob)))
*/
#ifdef DEBUGLOG
#define DLOG(logstring) \
{ \
dlog logstring; \
}
#else
#define DLOG(logstring) {}
#endif



typedef signed long long longlong;
/* Added by Jim for easier changes/code reading purposes */
#define EXP longlong                 /* experience type */

#include <sys/types.h>
#include "list.h"
#include "sstring.h"
#include "events.h"
#include "guilds.h"
#include "utils.h"
//#include "olc.h"

class Variable;
class Function;
typedef char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
#ifndef __cplusplus
typedef char bool;
#endif
typedef char byte;
typedef long room_num;

#define BHD_LIMIT 127
#define BHD_LIMIT2 127 // for shifters
#define BHD_LIMIT3 127 // for monks

/* Added by Min (Jan V) 1996 for support/conversion purposes */

#define MAX_NAME_LENGTH  15
#define LVL_GOD          116
#define XNAME_FILE       "badnames.lst"
#define NUM_OF_DIRS      6

/* poof stuff */
#define BIT_POOF_IN  1
#define BIT_POOF_OUT 2


struct QuestItem {
    int item;
    char *where;
};

/*
  efficiency stuff
  */
#define MIN_GLOB_TRACK_LEV 31   /* mininum level for global track */

/* help indices */

#define HELP_LOW	0
#define HELP_SKILL	0
#define HELP_IMMORTAL	1
#define HELP_COMBAT	2
#define HELP_GENERAL 	3
#define HELP_HIGH	3

/*
 **  Newbie authorization stuff
 */

#define NEWBIE_REQUEST 1
#define NEWBIE_START   100
#define NEWBIE_AXE     0
#define NEWBIE_CHANCES 3

/*
 **  distributed monster stuff
 */

#define TICK_WRAP_COUNT 3   /*  PULSE_MOBILE / PULSE_TELEPORT */

/* modifications to above system by Min 1997 */

#define MOB_DIVISOR     3
#define MOB_MAX_DIVISOR 200



/*
  Note:  This stuff is all code dependent,
  Don't change it unless you know what you
  are doing.  comm.c and mobact.c hold the
  stuff that you will HAVE to rewrite if you
  change either of those constants.
  */
#define PLR_TICK_WRAP   24  /*  this should be a divisor of 24 (hours) */


/*
 **  multiclassing stuff
 */

#define MAGE_LEVEL_IND    0
#define CLERIC_LEVEL_IND  1
#define WARRIOR_LEVEL_IND 2
#define THIEF_LEVEL_IND   3
#define PALADIN_LEVEL_IND 4
#define DRUID_LEVEL_IND   5
#define PSI_LEVEL_IND     6
#define RANGER_LEVEL_IND  7
#define SHIFTER_LEVEL_IND 8
#define MONK_LEVEL_IND    9
#define BARD_LEVEL_IND   10

#define MAX_LEVEL_IND     10
#define MAX_CLASS	  16

#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

#define HATE_SEX   1
#define HATE_RACE  2
#define HATE_CHAR  4
#define HATE_CLASS 8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define ABS_MAX_LVL	255
#define MAX_MORT	125

#define TRUST_GRUNT	1
#define TRUST_CREATOR	2
#define TRUST_SAINT	3
#define TRUST_DEMIGOD	4
#define TRUST_LRGOD	5
#define TRUST_GOD	6
#define TRUST_GRGOD	7
#define TRUST_LORD	8
#define TRUST_IMP	9
#define MAX_TRUST	10

#define IMM_FIRE         1
#define IMM_COLD         2
#define IMM_ELEC         4
#define IMM_ENERGY       8
#define IMM_BLUNT     0x10
#define IMM_PIERCE    0x20
#define IMM_SLASH     0x40
#define IMM_ACID      0x80
#define IMM_POISON   0x100
#define IMM_DRAIN    0x200
#define IMM_SLEEP    0x400
#define IMM_CHARM    0x800
#define IMM_HOLD    0x1000
#define IMM_NONMAG  0x2000
#define IMM_PLUS1   0x4000
#define IMM_PLUS2   0x8000
#define IMM_PLUS3  0x10000
#define IMM_PLUS4  0x20000
#define IMM_BARD   0x40000

struct nodes
{
    int visited;
    int ancestor;
};

struct string_block {
    int	size;
    char	*data;
};


/*
  memory stuff
  */

struct char_list {
    struct char_data *op_ch;
    char name[50];
    struct char_list *next;
};

typedef struct
{
    struct char_list  *clist;
    int    sex;			/*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
    int    race;			/*number */
    ush_int clss;			/* 1=m,2=c,4=f,8=t */
    int    vnum;			/* # */
    int    evil;			/* align < evil = attack */
    int    good;			/* align > good = attack */
}  Opinion;

struct spevent_type;

struct spevent_list {
   spevent_type *sp_event;
   spevent_list *next;
};


/*
  old stuff.
  */

#define MAX_STRING_LENGTH   4096
#define MAX_INPUT_LENGTH    1024 
#define MAX_MESSAGES          60

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR	75
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(30*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(12*SECS_PER_MUD_MONTH)

#define PULSE_PER_REAL_SEC	4
#define PULSE_PER_MUD_HOUR	(PULSE_PER_REAL_SEC * SECS_PER_MUD_HOUR)

#define PULSE_ZONE     		PULSE_PER_MUD_HOUR
#define PULSE_MOBILE		10
#define PULSE_DRAIN_LIFE        11
#define PULSE_VIOLENCE		12
#define PULSE_SOUND		15
#define PULSE_AUCTION           (PULSE_PER_REAL_SEC * 20)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_AUDIO     23
#define ITEM_BOARD     24
#define ITEM_SPELLBOOK 25
#define ITEM_SKY       26

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              1
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
#define ITEM_WEAR_LIGHT	   65536

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW	   (1<<0)
#define ITEM_HUM           (1<<1)
#define ITEM_METAL         (1<<2)  /* undefined...  */
#define ITEM_MINERAL       (1<<3)  /* undefined?    */
#define ITEM_ORGANIC       (1<<4) /* undefined?    */
#define ITEM_INVISIBLE     (1<<5)
#define ITEM_MAGIC         (1<<6)
#define ITEM_NODROP        (1<<7)
#define ITEM_BLESS         (1<<8)
#define ITEM_ANTI_GOOD     (1<<9) /* not usable by good people    */
#define ITEM_ANTI_EVIL     (1<<10) /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL  (1<<11) /* not usable by neutral people */
#define ITEM_ANTI_CLERIC   (1<<12)
#define ITEM_ANTI_MAGE     (1<<13)
#define ITEM_ANTI_THIEF    (1<<14)
#define ITEM_ANTI_FIGHTER  (1<<15)
#define ITEM_BRITTLE       (1<<16) /* weapons that break after 1 hit */
                                   /* armor that breaks when hit?    */
#define ITEM_ANTI_PALADIN  (1<<17)
#define ITEM_ANTI_DRUID    (1<<18)
#define ITEM_ANTI_PSI      (1<<19)
#define ITEM_ANTI_RANGER   (1<<20)
#define ITEM_UNUSED0    (1<<21)
#define ITEM_UNUSED1    (1<<22)
#define ITEM_UNUSED2    (1<<23)
#define ITEM_NO_LOCATE     (1<<24)
#define ITEM_RARE	   (1<<25)
#define ITEM_ANTI_BARD     (1<<26)
#define ITEM_ANTI_MONK     (1<<27)
#define ITEM_PURE_CLASS    (1<<28)
#define ITEM_TWO_HANDED	   (1<<29)
#define ITEM_ANTI_SHIFTER  (1<<30)
#define ITEM_HARDEN        (1<<31)


/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* special addition for drinks */
#define DRINK_POISON  (1<<0)
#define DRINK_PERM    (1<<1)


/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8


struct extra_descr_data
{
    char *keyword;		/* Keyword in look/examine          */
    char *description;		/* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 5         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
    int value[4];		/* Values of the item (see list)    */
    byte type_flag;		/* Type of item                     */
    int wear_flags;		/* Where you can wear it            */
    unsigned long extra_flags;		/* If it hums,glows etc             */
    int weight;			/* Weight what else                 */
    int cont_weight;		/* contained weight		    */
    int cost;			/* Value when sold (gp.)            */
    int cost_per_day;		/* Cost to keep pr. real day        */
    int timer;			/* Timer for object                 */
    int no_loot;		/* For use in pkilling		    */
    int sample_level;		/* For Sample bug fix		    */
    long level; /* object level     */
    long durability; /* item's durability     */
    long encumbrance; /* how bulky the object is     */
    unsigned char tick;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type
{
    short location;		/* Which ability to change (APPLY_XXX) */
    long modifier;		/* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
    sh_int item_number;			/* Where in data-base               */
    room_num in_room;			/* In what room -1 when conta/carr  */
    struct obj_flag_data obj_flags;	/* Object information               */
    struct obj_affected_type
	affected[MAX_OBJ_AFFECT];	/* Which abilities in PC to change  */

    struct char_data *killer;		/* for use with corpses */
    sh_int char_vnum;			/* for ressurection     */
    sstring_t* name;			/* Title of object :get etc.        */
    sstring_t* description ;		/* When in room                     */
    sstring_t* short_description;	/* when worn/carry/in cont.         */
    sstring_t* action_description;	/* What to write when used          */
    struct extra_descr_data *ex_description; /* extra descriptions     */
    struct char_data *carried_by;	/* Carried by :NULL in room/conta   */
    byte   eq_pos;			/* what is the equip. pos?          */
    struct char_data *equipped_by;	/* equipped by :NULL in room/conta  */

    struct obj_data *in_obj;		/* In what object NULL when none    */
    struct obj_data *contains;		/* Contains objects                 */

    struct obj_data *next_content;	/* For 'contains' lists             */
    sstring_t* char_name;
    event_t*	corpse_timer;
    event_t*	sound_timer;
    spevent_list *sp_list;              /* What event it has on it */
    Variable *vars;
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database      */
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/
#define AUTO_RENT  -2    /* other special room, for auto-renting */

/* Bitvector For 'room_flags' */

#define DARK		(1 << 0)
#define DEATH		(1 << 1)
#define NO_MOB		(1 << 2)
#define INDOORS		(1 << 3)
#define PEACEFUL	(1 << 4)  /* No fighting */
#define NOSTEAL		(1 << 5)  /* No Thieving */
#define NO_TRAVEL_OUT	(1 << 6)  /* no summoning */
#define NO_MAGIC	(1 << 7)
#define TUNNEL		(1 << 8)
#define NO_TRAVEL_IN	(1 << 9)
#define SILENCE		(1 << 10)
#define NO_PUSH		(1 << 11)
#define IMMORT_RM	(1 << 12)
#define GOD_RM		(1 << 13)
#define NO_RECALL	(1 << 14)
#define ARENA           (1 << 15)
#define NO_SNEAK        (1 << 16)
#define TEMPLE          (1 << 17) /* place where resurrection works for higher ups */
#define BRUJAH_RM       (1 << 18)

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      	1
#define EX_CLOSED      	2
#define EX_LOCKED      	4
#define EX_SECRET	8
#define EX_RSLOCKED	16
#define EX_PICKPROOF    32

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_AIR             8
#define SECT_UNDERWATER      9
#define SECT_DESERT          10
#define SECT_SKY	     11

#define TELE_LOOK            1
#define TELE_COUNT           2
#define TELE_RANDOM          4
#define TELE_SPIN            8

struct room_direction_data
{
    char *general_description;    /* When look DIR.                  */
    char *keyword;                /* for open/close                  */
    sh_int exit_info;             /* Exit info                       */
    int key;			  /* Key's number (-1 for no key)    */
    room_num to_room;		  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */

#define SPECIAL(name) \
   int (name)(void *me, struct char_data *ch, int cmd, char *arg, int type)

typedef int (*spec_proc_func)(void *me, struct char_data* ch, int cmd,
			      char* arg, int type);

struct room_data
{
    room_num number;		  /* Rooms number                       */
    sh_int zone;		  /* Room zone (for resetting)          */
    byte sector_type;             /* sector type (move/hide)            */

    byte river_dir;               /* dir of flow on river               */
    byte river_speed;             /* speed of flow on river             */

    byte  tele_time;              /* time to a teleport                 */
    int  tele_targ;		  /* target room of a teleport          */
    byte tele_mask;		  /* flags for use with teleport        */
    byte  tele_cnt;		  /* countdown teleports                */
    event_t* tele_event;	  /* event trigger for timed teleports  */
    event_t* river_event;	  /* event trigger for river drifting   */

    unsigned char moblim;	  /* # of mobs allowed in room.         */

    char *name;			  /* Rooms name 'You are ...'           */
    char *description;		  /* Shown when entered                 */
    struct extra_descr_data *ex_description; /* for examine/look        */
    struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions  */
    long room_flags;		  /* DEATH,DARK ... etc                 */
    byte light;			  /* Number of lightsources in room     */
    byte dark;
    spec_proc_func funct;         /* special procedure                  */

    struct obj_data *contents;	  /* List of items in room              */
    struct char_data *people;	  /* List of NPC / PC in room           */
    Function *roomprogs2;         /* Room mobprogs2 */
    Variable *vars;
    Variable *global_vars;        /* Room global variables */
    spevent_list *sp_list;        /* Events on room */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define LOADED         18


/* For 'char_payer_data' */


/*
 **  #2 has been used!!!!  Don't try using the last of the 3, because it is
 **  the keeper of active/inactive status for dead characters for ressurection!
 */
#define MAX_TONGUE  20     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */


#define MAX_SKILLS  490   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    19
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             0x00000001
#define AFF_INVISIBLE         0x00000002
#define AFF_REGENERATE        0x00000004
#define AFF_DETECT_INVISIBLE  0x00000008
#define AFF_SENSE_AURA        0x00000010
#define AFF_SENSE_LIFE        0x00000020
#define AFF_LIFE_PROT         0x00000040
#define AFF_SANCTUARY         0x00000080
#define AFF_GROUP             0x00000100
#define AFF_BERSERK           0x00000200
#define AFF_CURSE             0x00000400
#define AFF_FLYING            0x00000800
#define AFF_POISON            0x00001000
#define AFF_ILLUSION          0x00002000
#define AFF_PARALYSIS         0x00004000
#define AFF_INFRAVISION       0x00008000
#define AFF_WATERBREATH       0x00010000
#define AFF_SLEEP             0x00020000
#define AFF_DODGE             0x00040000
#define AFF_SNEAK             0x00080000
#define AFF_HIDE              0x00100000
#define AFF_SILENCE           0x00200000
#define AFF_CHARM             0x00400000
#define AFF_FOLLOW            0x00800000
#define AFF_UNDEF_1           0x01000000  /* saved objects?? */
#define AFF_TRUE_SIGHT        0x02000000
#define AFF_SCRYING           0x04000000  /* seeing other rooms */
#define AFF_FIRESHIELD        0x08000000
#define AFF_CONTINUAL_DARK    0x10000000  /* was Adrenalize */
#define AFF_MEDITATE          0x20000000
#define AFF_GREAT_SIGHT       0x40000000
#define AFF_CONTINUAL_LIGHT   0x80000000  /* MAX_AFFECTIONS <32 bits> :-( */

#define AFF2_HASTE	      0x00000001
#define AFF2_SLOW	      0x00000002
#define AFF2_DESPAIR	      0x00000004
#define AFF2_TOLERANCE        0x00000008
#define AFF2_RAGE             0x00000010
#define AFF2_ROUGHNESS        0x00000020
#define AFF2_RESISTANCE       0x00000040
#define AFF2_ELECSHIELD       0x00000080
#define AFF2_POISONSHIELD     0x00000100
#define AFF2_ENERGYSHIELD     0x00000200
#define AFF2_VAMPSHIELD       0x00000400
#define AFF2_MANASHIELD       0x00000800
#define AFF2_ACIDSHIELD       0x00001000
#define AFF2_COLDSHIELD       0x00002000
#define AFF2_MINDPROTECT      0x00004000
#define AFF2_ABSORB           0x00008000
#define AFF2_ROUGH            0x00010000
#define AFF2_MOVESHIELD	      0x00020000
#define AFF2_NOSUMMON         0x00040000
#define AFF2_FLIGHT	      0x00080000
#define AFF2_FIRE_BREATH      0x00100000
#define AFF2_FROST_BREATH     0x00200000
#define AFF2_ACID_BREATH      0x00400000
#define AFF2_POISONGAS_BREATH 0x00800000
#define AFF2_LIGHTNING_BREATH 0x01000000

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SAVE_ALL         25
#define APPLY_IMMUNE           26
#define APPLY_SUSC             27
#define APPLY_M_IMMUNE         28
#define APPLY_SPELL            29
#define APPLY_WEAPON_SPELL     30
#define APPLY_EAT_SPELL        31
#define APPLY_BACKSTAB         32
#define APPLY_KICK             33
#define APPLY_SNEAK            34
#define APPLY_HIDE             35
#define APPLY_BASH             36
#define APPLY_PICK             37
#define APPLY_STEAL            38
#define APPLY_TRACK            39
#define APPLY_HITNDAM          40
#define APPLY_BHD              41
#define APPLY_NUM_DICE         42
#define APPLY_SIZE_DICE        43
#define APPLY_GUILD	       44
#define APPLY_RIDE             47
#define APPLY_SPELL2	       48
#define APPLY_AFF2	       49
#define APPLY_CHA              50
#define APPLY_BOOK_SPELL       51
#define APPLY_DRAIN_LIFE       52
#define APPLY_THROW	       53
#define APPLY_TRIP	       54
#define APPLY_DISARM	       55
#define APPLY_SBLOCK	       56
#define APPLY_SEARCH	       57
#define APPLY_WARCRY	       58
#define APPLY_MEDITATE	       59
#define APPLY_STUN	       60
#define APPLY_ARCHERY	       61
#define APPLY_PINCH	       62
#define APPLY_LAYHANDS         63
#define APPLY_SPUNCH	       64
#define APPLY_FLAIL	       65
#define APPLY_FEINT	       66
#define APPLY_GOUGE	       67
#define APPLY_DIVERT	       68
#define APPLY_PUSH	       69
#define APPLY_BLESSING	       70
#define APPLY_HEROIC_RESCUE    71
#define APPLY_THRUST	       72
#define APPLY_DOORBASH	       73
#define APPLY_HYPNO	       74
#define APPLY_THOUGHT	       75
#define APPLY_COMBUSTION       76
#define APPLY_CONSTRICT	       77
#define APPLY_BLAST	       78

/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_WARRIOR     4
#define CLASS_THIEF       8
#define CLASS_PALADIN     16
#define CLASS_DRUID       32
#define CLASS_PSI         64
#define CLASS_RANGER      128
#define CLASS_SHIFTER     256
#define CLASS_MONK	  512
#define CLASS_BARD        1024
#define CLASS_MASK	  0x7ff

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_MOUNTED    9


/* for mobile actions: specials.mob_act */
#define ACT_SPEC       (1<<0)  /* special routine to be called if exist   */
#define ACT_SENTINEL   (1<<1)  /* this mobile not to be moved             */
#define ACT_SCAVENGER  (1<<2)  /* pick up stuff lying around              */
#define ACT_ISNPC      (1<<3)  /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1<<4)  /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1<<5)  /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE  (1<<6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      (1<<7)  /* MOB Will flee when injured, and if      */
			       /* aggressive only attack sleeping players */
#define ACT_ANNOYING   (1<<8)  /* MOB is so utterly irritating that other */
			       /* monsters will attack it...              */
#define ACT_HATEFUL    (1<<9)  /* MOB will attack a PC or NPC matching a  */
			       /* specified name                          */
#define ACT_AFRAID    (1<<10)  /* MOB is afraid of a certain PC or NPC,   */
			       /* and will always run away ....           */
#define ACT_IMMORTAL  (1<<11)  /* MOB is a natural event, can't be kiled  */
#define ACT_ROAM      (1<<12)  /* Mob can leave zone w/o charm            */
#define ACT_DEADLY    (1<<13)  /* MOB has deadly poison                   */
#define ACT_POLYSELF  (1<<14)  /* MOB is a polymorphed person             */
#define ACT_META_AGG  (1<<15)  /* MOB is _very_ aggressive                */
#define ACT_GUARDIAN  (1<<16)  /* MOB will guard master                   */
#define ACT_IT        (1<<17)  /* this player is IT                       */
#define ACT_PATTACK   (1<<18)  /* initiated player combat                 */
#define ACT_LIQUID    (1<<19)  /* is in liquid form                       */
#define ACT_SHIFTER   (1<<20)  /* is a shapeshifter                       */
#define ACT_STEED     (1<<21)  /* MOB can be used as a mount              */
#define ACT_HUGE      (1<<22)  /* too big to go indoors                   */
#define ACT_NOTRACK   (1<<23)  /* MOB can not be tracked                  */
#define ACT_HUNTER    (1<<24)  /* MOB will hunt fleeing players		  */

/* For players : specials.flags */
#define PLR_BRIEF     (1<<0)
#define PLR_CONTINUOUS (1<<1) /* don't page output...			*/
#define PLR_COMPACT   (1<<2)
#define PLR_PKILLER   (1<<3)  /* player has pkilled			*/
#define PLR_LOSER     (1<<4)  /* player can be killed at will w/no flag */
#define PLR_NOHASSLE  (1<<5)  /* char won't be attacked by aggressives. */
#define PLR_STEALTH   (1<<6)  /* char won't be announced in a variety of situations */
#define PLR_WIMPY     (1<<7)
#define PLR_THIEF     (1<<8)  /* player has stolen from other players	*/
#define PLR_ECHO      (1<<9)  /* Messages (tells, shout,etc) echo back	*/
#define PLR_DISPLAY   (1<<10) /* chnages prompt to hp,mana,move being showed */
#define PLR_AGGR      (1<<11) /* makes the player aggr to aggr mobs	*/
#define PLR_COLOR     (1<<12) /* ANSI color mode			*/
#define PLR_MASK      (1<<13) /* immortal is masked to players		*/
#define PLR_NOSHOUT   (1<<14) /* the player is not allowed to shout	*/
#define PLR_BUILDER   (1<<15) /* the player is a builder 		*/
#define PLR_NOTELL    (1<<16) /* the player won't receive ANY tells	*/
#define PLR_AUTOEXIT  (1<<17) /* the player will see visible exits	*/
#define PLR_AFK       (1<<18) /* for the AFK code			*/
#define PLR_LOGALL    (1<<19) /* log all commands by player		*/
#define PLR_SITE_OK   (1<<20) /* allow char when site is locked		*/
#define PLR_AUTOLOOT  (1<<21) /* get all from corpse when killing	*/
#define PLR_AUTOSPLIT (1<<22) /* split gold automatically		*/
#define PLR_AUTOGOLD  (1<<23) /* autoloot isn't good enough for them	*/
#define PLR_NOWIZ     (1<<24) /* don't show player as a god		*/
#define PLR_DENY      (1<<25) /* deny command				*/
#define PLR_SHOW_DAM  (1<<26) /* shows damage during combat		*/
#define PLR_BRUJAH    (1<<27) /* brujah					*/
#define PLR_SUMMON    (1<<28) /* summon for players			*/
#define PLR_AUTOASST  (1<<29) /* Autoassist command for slackers	*/

/* These are for the deletion flag of a character */
#define DELETE        1
#define PROTECT       2
#define HOUSED        4

/* These are some corpse values */
#define NPC_CORPSE 1
#define PC_CORPSE 2
#define UNDEAD_CORPSE 3
#define SAMPLE_CORPSE 4
#define SCRAP_ITEM 5
#define HEAD_ITEM 6
#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 144
#define MAX_HEAD_TIME 10
#define MAX_SCRAP_TIME 15

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
    sh_int hours, day, month;
    sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
    time_t birth;    /* This represents the characters age                */
    time_t logon;    /* Time of the last logon (used to calculate played) */
    int played;      /* This is the total accumulated time played in secs */
};

#define LOG_NUM_CHANNELS    6
#define LOG_ALL		    (1<<LOG_NUM_CHANNELS)-1
#define LOG_MPROG 	    (1<<0)
#define LOG_PLAYER          (1<<1)
#define LOG_IMM		    (1<<2)
#define LOG_ERROR           (1<<3)
#define LOG_QUILAN          (1<<4)
#define LOG_CONNECT         (1<<5)

#define PROJECT_MANAGER (1 << 0)
#define ASSISTANT_MANAGER (1 << 1)
#define HEAD_BUILDER (1 << 2)
#define HEAD_QUESTOR (1 << 3)
#define CODER (1 << 4)
#define BUILDER (1 << 5)
#define AMBASSADOR (1 << 6)
#define QUESTOR (1 << 7)
#define NEWBIE_HELPER (1 << 8)
#define WEBMASTER (1 << 9)
#define SENIOR_CODER (1 << 10)
#define SENIOR_BUILDER (1 << 11)
#define DAMNED (1 << 12)
#define NOPOSITION (1 << 13) 	/* place holder for do_who */


#define ALL_GCMD 666
#define GCMD_GEN            (1 << 0)  /* invis,log,think,goto,home,users,stats,wizhelp,at,nohassle,helptopics,trivia */
#define GCMD_REDIT          (1 << 1)  /* rsave,redit,rload,create,zreset,instazone  */
#define GCMD_RESTORE        (1 << 2)  /* restore */
#define GCMD_SNOOP          (1 << 3)  /* snoop, imp */
#define GCMD_NOSHOUT        (1 << 4)  /* noshout */
#define GCMD_POOF           (1 << 5)  /* poofin,poofout,mask */
#define GCMD_FORCE          (1 << 6)  /* force */
#define GCMD_CHPWD          (1 << 7)  /* chpwd */
#define GCMD_SET            (1 << 8)  /* @set,reincarnate */
#define GCMD_SLAY           (1 << 9)  /* slay */
#define GCMD_LOAD           (1 << 10) /* load normal obj's,mobs */
#define GCMD_PLOAD          (1 << 11) /* load players */
#define GCMD_OLOAD          (1 << 12) /* load full obj's */
#define GCMD_TRANS          (1 << 13) /* transfer */
#define GCMD_PURGE          (1 << 14) /* purge, chomp */
#define GCMD_SHUTDOWN       (1 << 15) /* shutdown */
#define GCMD_STRING         (1 << 16) /* string */
#define GCMD_REIMB          (1 << 17) /* reimb */
#define GCMD_SWITCH         (1 << 18) /* switch */
#define GCMD_SYSTEM         (1 << 19) /* system */
#define GCMD_FREEZE         (1 << 20) /* freeze */
#define GCMD_ABSOLVE        (1 << 21) /* absolve */
#define GCMD_STEALTH        (1 << 22) /* stealth */
#define GCMD_SHOW           (1 << 23) /* show, csocket */
#define GCMD_PLAYER         (1 << 24) /* player */
#define GCMD_BAN            (1 << 25) /* ban,unban */
#define GCMD_STAT           (1 << 26) /* stat */
#define GCMD_ADVANCE        (1 << 27) /* advance */
#define GCMD_JUDGE          (1 << 28) /* judge */
#define GCMD_ECHO           (1 << 29) /* echo */
#define GCMD_HIGH           (1 << 30) /* reroll,auth,dir,swapzone,trace,events,ident,listwiz,whod,allow,slownames,wizlock,extraone,logall,path,exp,viewfile,debug,wizrep */
#define GCMD_WIZSET         (1 << 31)  /* Don't change the pos. of this! */
#define GCMD_CSOCKET	    (1 << 32) /* csocket */
	/* You want more, push it all the way to 63.  */

#define GCMD_DAMNED	GCMD_GEN
#define GCMD_AMBASS	GCMD_GEN+GCMD_SLAY
#define GCMD_NEWHELP	GCMD_AMBASS+GCMD_SHOW+GCMD_REIMB+GCMD_TRANS+GCMD_PURGE
#define GCMD_WEBMASTER	GCMD_NEWHELP+GCMD_STAT
#define GCMD_QUESTOR	GCMD_NEWHELP+GCMD_REDIT+GCMD_POOF+GCMD_SWITCH+GCMD_STRING+GCMD_LOAD+GCMD_RESTORE+GCMD_JUDGE
#define GCMD_BUILDER	GCMD_QUESTOR+GCMD_ECHO+GCMD_FREEZE+GCMD_FORCE+GCMD_NOSHOUT+GCMD_PLAYER
#define GCMD_SENBUILDER GCMD_BUILDER+GCMD_SHUTDOWN
#define GCMD_CODER	GCMD_BUILDER+GCMD_PLOAD
#define GCMD_SENCODER	GCMD_CODER+GCMD_ADVANCE+GCMD_SET+GCMD_OLOAD
#define GCMD_HEADBUILD  GCMD_SENCODER+GCMD_BAN+GCMD_STEALTH+GCMD_ABSOLVE+GCMD_SYSTEM
#define GCMD_HEADQUEST	GCMD_HEADBUILD+GCMD_SNOOP

class Godinfo {

public:

    int position;      /* Position in the Organization. */
    longlong cmdset;      /* What command groups they have access to */

    Godinfo() { position = 0; cmdset = 0;};

    bool hascmd(int cmd) { return IS_SET(cmdset, cmd); };

};

#define HAS_GCMD(ch,cmd) ((ch)->player.godinfo.hascmd(cmd))
#define GCMD_FLAGS(ch) ((ch)->player.godinfo.cmdset)

#define CAN_PKILL    (1<<0)

struct char_pkillinfo_data
{
    long flags;
    int status;
    int count;
    int killed;
};

struct char_player_data
{
    sstring_t* name;		/* PC / NPC s name (kill ...  )         */
    sstring_t* short_descr;	/* for 'actions'                        */
    sstring_t* long_descr;	/* for 'look'.. Only here for testing   */
    sstring_t* description;	/* Extra descriptions                   */
    sstring_t* title;		/* PC / NPC s title                     */
    sstring_t* sounds;		/* Sound that the monster makes (in room) */
    sstring_t* distant_snds;	/* Sound that the monster makes (other) */
    byte sex;			/* PC / NPC s sex                       */
    ush_int clss;		/* PC s class or NPC alignment          */
    ubyte level[MAX_CLASS];      /* PC / NPC s level                     */
    ubyte max_level[MAX_CLASS];	/* Max reached to date                  */
    int hometown;		/* PC s Hometown (zone)                 */
    bool talks[MAX_TONGUE];	/* PC s Tounges 0 for NPC               */
    struct time_data time;	/* PC s AGE in days                     */
    int weight;		        /* PC / NPC s weight                    */
    int height;		        /* PC / NPC s height                    */
    sbyte mother;               /* PC s mother s profession             */
    sbyte father;               /* PC s father s profession             */
    int   upbringing;           /* PC s upbringing alignment            */
    sbyte age12;                /* PC s choice for training at age 12   */
    sbyte age13;                /* PC s choice for training at age 13   */
    sbyte age14;                /* PC s choice for training at age 14   */
    ubyte maxlevel;		/* PC s max overall level 		*/
    ubyte minlevel;		/* PC s min overall level 		*/
    byte trust;			/* PCs God level 			*/
    GuildInfo guildinfo;        /* PCs Guild Information                */
    Godinfo godinfo;		/* PC's Godinfo, Commands and Postion	*/
    struct char_pkillinfo_data pkillinfo;
    int mpstate, qnum;
    int hpsystem;
    Variable *vars;
    int has_killed_victim;      /* This is a HUGE hack for WeaponSpell  */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
    sbyte str;
    sbyte intel;
    sbyte wis;
    sbyte dex;
    sbyte con;
    sbyte cha;
    sh_int str_add;      /* 000 - 100 if strength 18             */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{
  sh_int mana;
  sh_int max_mana;
  
  int   hit;
  int   max_hit;       /* Max hit for NPC                         */
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */
  
  sh_int armor;        /* Internal -100..100, external -10..10 AC */
  int gold;            /* Money carried                           */
  int bankgold;        /* gold in the bank.                       */
  EXP exp;             /* The experience of the player            */
  
  short hitroll;       /* Any bonus or penalty to the hit roll    */
  short damroll;       /* Any bonus or penalty to the damage roll */
  
  int bhsize_mod;      /* Barehand size dice modification          */
  int bhnum_mod;       /* Barehand number of dice modification     */
  
  int hero_points;     /* Hero points for hero list */
};


struct char_special_data
{

    struct char_data *charging; /* who you are charging */
    char *poofin;
    char *poofout;
    unsigned char tick;		/* the tick that the mob/player is on  */
    byte pmask;			/* poof mask                           */
    byte position;		/* Standing or ...                         */
    byte default_pos;		/* Default position for NPC                */
    byte spells_to_learn;	/* How many can you learn yet this level   */
    byte carry_items;		/* Number of items carried                 */
    byte damnodice;		/* The number of damage dice's            */
    byte damsizedice;		/* The size of the damage dice's          */
    byte last_direction;	/* The last direction the monster went    */

    sbyte conditions[3];	/* Drunk full etc.                        */

    int zone;			/* zone that an NPC lives in */
    int carry_weight;		/* Carried weight                          */
    int timer;			/* Timer for update                        */
    room_num was_in_room;	/* storage of location for linkdead people */
    int attack_type;		/* The Attack Type Bitvector for NPC's     */
    int alignment;		/* +-1000 for alignments                   */
    sh_int wimpy;               /* 0 = default, > 0 = flee value           */
    sh_int bonus_attks;		/* bonus attacks next round +/-		   */

    struct char_data *fighting; /* Opponent                             */

    struct char_data *hunting;  /* Hunting person..                     */
    struct char_data *binding;   /* binding shapeshifter */
    struct char_data *binded_by; /* bound */

    struct char_data *ridden_by;
    struct char_data *mounted_on;

    unsigned long affected_by[2];/* Bitvector for spells/skills affected by */

    unsigned long mob_act;	/* flags for NPC behavior                  */
    unsigned long flags;	/* pc flags, used to be shared with act */

    sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */

    char* hostname;
    char* prompt;
    int see_invis_level;	/* allows gods to lets mortals see invis levels default = 0 */
};


struct char_skill_data
{
    ubyte learned;		/* % chance for success 0 = not learned   */
    bool recognise;		/* If you can recognise the scroll etc.   */
};

typedef void (*expire_proc)(struct char_data *ch, char *arg, int cmd);
struct affected_type
{
    event_t timer;		/* Timer */
    short type;			/* The type of spell that caused this      */
    sh_int duration;		/* For how long its effects will last      */
    long modifier;		/* This is added to apropriate ability     */
    byte location;		/* Tells which ability to change(APPLY_XXX)*/
    long bitvector;		/* Tells which bits to set (AFF_XXX)       */

    int mana_cost;		/* cost per tick to maintain */
    int save_bonus;		/* some affects get a periodic save */
    struct char_data* caster;	/* payer of said mana... */
    struct char_data* holder;	/* who the affect is on */

    struct affected_type *next;

    expire_proc expire_proc_pointer;  /* proc to call when aff expires */
};

struct generic_list {
   void *item;
   generic_list *next;
};

struct spevent_type {
   event_t timer;               /* Timer */
   short is_pulse;              /* Run on pulse, or real? */
   short type;                  /* The type of spell that caused this      */
   sh_int duration;             /* For how long its effects will last      */
   struct char_data* caster;    /* who cast the spell */
   struct char_data* victim;    /* who the event is on */
   room_num in_room;            /* what room it was cast in... */
   void *extra;                 /* anything the coder decides to add... */

   event_func expire_func;      /* This goes at the end of the time... */
   event_func free_func;        /* In case it gets killed before it's removed */
   event_func bsave_func;       /* Before character is saved... */
   event_func asave_func;       /* After character is saved... */

   generic_list *char_list;
   generic_list *obj_list;
   generic_list *room_list;

   struct spevent_type *next;   /* Tis a linked list and all */
};

struct follow_type
{
    struct char_data *follower;
    struct follow_type *next;
};

#define MAX_ALIAS_SAVE 20

struct alias_data
{
    char *pattern[MAX_ALIAS_SAVE + 1];
    char *alias[MAX_ALIAS_SAVE];
};


/*
 * Added by Min 1996 for MOBPROGRAMS   (as it says... Mobprogram Foo hack !)
 */

struct mob_prog_act_list {
    struct mob_prog_act_list *next;
    char *buf;
    struct char_data *ch;
    struct obj_data *obj;
    void *vo;
};

typedef struct mob_prog_act_list MPROG_ACT_LIST;

struct mob_prog_data {
    struct mob_prog_data *next;
    int type;
    char *arglist;
    char *comlist;
};

typedef struct mob_prog_data MPROG_DATA;

extern bool MOBTrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HEALTH_PROG       32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
#define SPELL_PROG      2048
#define WEATHER_PROG    4096

/* end of mobprog foo hack */

/* ================== Structure for player/non-player ===================== */
struct char_data
{
    sh_int pos;                          /* player pos in file */
    char *pwd;
    sh_int nr;                           /* monster nr (pos in file)    */
    int fight_delay;                     /* For NPC's delay in battle */
    room_num in_room;                    /* Location                    */
    struct Board* board;

    unsigned immune;                     /* Immunities                  */
    unsigned M_immune;                   /* Meta Immunities             */
    unsigned susc;                       /* susceptibilities            */
    sh_int mult_att;                     /* the number of attacks       */
    byte   attackers;

    sh_int fallspeed;                    /* rate of descent for player */
    sh_int race;
    sh_int hunt_dist;                    /* max dist the player can hunt */

    unsigned short hatefield;
    unsigned short fearfield;

    Opinion hates;
    Opinion fears;

    sh_int  persist;
    int     old_room;

    int     act_ptr;                      /* numeric argument for the mobile actions */

    struct char_player_data player;       /* Normal data                */
    struct char_ability_data abilities;   /* Abilities                  */
    struct char_point_data points;        /* Points                     */
    struct char_special_data specials;    /* Special plaing constant    */
    struct char_skill_data *skills;       /* Skills                     */

    struct affected_type *affected;       /* affected by what spells    */
    struct spevent_list *sp_list;         /* under what events          */
    struct obj_data *equipment[MAX_WEAR]; /* Equipment array            */

    struct obj_data *carrying;            /* Head of list               */
    struct descriptor_data *desc;         /* NULL for mobiles           */
    struct char_data* orig;               /* Special for polymorph      */
    struct char_data* curr;

    struct char_data *next_in_room;       /* For room->people - list      */
    struct char_data *next_fighting;      /* For fighting list            */

    struct char_data *next_charging;      /* for charging list            */
    struct follow_type *followers;        /* List of chars followers    */
    struct char_data *master;             /* Who is char following?     */
    char    invis_level;		  /* visibility of gods         */
    short delete_flag;		          /* 0 to not delete	    */

    int rent_cost;		          /* cost of last offer... */

    struct alias_data *aliases;           /* List of aliases            */
    int freeze;                           /* char cant do anything      */
    int charge_dir;                       /* direction charging         */

/* DON'T CHANGE THIS it affects char_file_u */
#define SAVE_IN_PLAYER	18	/* one per body position */

#if SAVE_IN_PLAYER > 0
    int obj_count;		/* count of objects we've saved here... */
    int saved_objects[SAVE_IN_PLAYER];
#endif

    int in_guild;		          /* of which guild are we a member */
    int saved_guild;		          /* for items/spies */
    byte guild_level;		          /* our status in that guild */

    byte file_version;

    int channels;                         /* channels this character has blocked */
    long log_flags;

    struct
    {
	byte			valid;
	byte			clss;
	ubyte			level;
        long			exp;
    }				res_info;

    event_t*			sound_timer;

    struct track_path*		hunt_info;
    struct char_data*		other_gate;

    char                        *reply;

    /* MOBprog foo added by min 1996 */
    MPROG_ACT_LIST *mpact;
    int mpactnum;
    int drop_count;                       /* Number of times they've dropped link */

//    OnlineCreation olc;                 /* online creation info */
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET	        3
#define MOON_SET        4
#define MOON_RISE       5

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

struct weather_data
{
    int pressure;		/* How is the pressure ( Mb ) */
    int change;			/* How fast and what way does it change. */
    int sky;			/* How is the sky. */
    int sunlight;		/* And how much sun. */
};


#define MAX_HOSTNAME_LENGTH	49
#define MAX_IDENT_LENGTH	15
#define MAX_HOST_LENGTH		MAX_HOSTNAME_LENGTH + MAX_IDENT_LENGTH + 1

struct char_file_u
{
    byte sex;
    ush_int clss;
    ubyte level[MAX_CLASS];
    time_t birth;  /* Time of birth of character     */
    int played;    /* Number of secs played in total */


    int   race;
    ubyte weight;
    ubyte height;

    char title[80];
    room_num hometown;
    room_num home;		/* for immortals to enter at a home */
    char description[240];
    bool talks[MAX_TONGUE];

    room_num load_room;            /* Which room to place char in  */

    struct char_ability_data abilities;

    struct char_point_data points;

    struct char_skill_data skills[MAX_SKILLS];

    struct affected_type affected[MAX_AFFECT];

    /* specials */

    byte spells_to_learn;
    int alignment;
    int invis_level;	/* for immorts to be able to log in invis */

    time_t last_logon;  /* Time (in secs) of last logon */
    unsigned long act;  /* ACT Flags                    */

    byte pmask;
    char poofin[150];
    char poofout[150];

    int delete_flag;
    char hostname[MAX_HOST_LENGTH + 1];

    char prompt[128];

    /* char data */
    char name[20];
    char short_descr[40];
    char pwd[11];
    sh_int apply_saving_throw[5];
    int conditions[3];

#if SAVE_IN_PLAYER > 0
    int obj_count;		/* count of objects we've saved here... */
    int saved_objects[SAVE_IN_PLAYER];
#endif

    int in_guild;		/* of which guild are we a member */
    byte guild_level;		/* our status in that guild */
};



/* ***********************************************************************
 *  file element for object file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */

struct obj_cost { /* used in act.other.c:do_save as
		     well as in reception2.c */
    int total_cost;
    int no_carried;
    int lims;
    bool ok;
};

#define MAX_OBJ_SAVE 50   /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

#define SAVE_LAST  0x4000
#define SAVE_EMPTY 0x2000

struct obj_file_elem
{
    sh_int item_number;

    int value[4];
    int eq_pos;     /* Equipment_pos */
    int wear_flags;
    int extra_flags;
    int weight;
    int timer;
    long bitvector;
    char name[128];  /* big, but not horrendously so */
    char sd[128];
    char desc[256];
    char ad[256];
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u {
    char owner[20];    /* Name of player                     */
    int gold_left;     /* Number of goldcoins left at owner  */
    int total_cost;    /* The cost for all items, per day    */
    long last_update;  /* Time in seconds, when last updated */
    long minimum_stay; /* For stasis */
    int  number;       /* number of objects */
    char pattern[MAX_ALIAS_SAVE][20];
    char alias[MAX_ALIAS_SAVE][128];
    struct obj_file_elem objects[MAX_OBJ_SAVE];
};

/* ***********************************************************
 *  The following structures are related to descriptor_data   *
 *********************************************************** */



struct txt_block
{
    char *text;
    struct txt_block *next;
};

struct txt_q
{
    struct txt_block *head;
    struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME	    1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNCNF 13
#define CON_WIZLOCK 14
#define CON_QRACE   15
#define CON_RACPAR 	16
#if PLAYER_AUTH
#define CON_AUTH	17
#endif
#define CON_CITY_CHOICE 18
#define CON_QPROFF	19
#define CON_QPROFM	20
#define CON_QALIGN	21
#define CON_QAGE12	22
#define CON_QAGE13	23
#define CON_QAGE14	24
#define CON_SITELOCK	25
#define CON_CLOSE	26
#define CON_IDCONING	27
#define CON_IDCONED	28
#define CON_IDREADING	29
#define CON_IDREAD	30
#define CON_ASKNAME	31
#define CON_PWDCSCKT    32
#define CON_DELETE      33
#define CON_RIMOTD	34

struct snoop_data
{
    struct char_data *snooping;
    /* Who is this char snooping */
    struct char_data *snoop_by;
    /* And who is snooping on this char */
};


struct descriptor_data
{
  list_element link;
  int descriptor;	          /* file descriptor for socket */
  char host[MAX_HOST_LENGTH+1];   /* hostname                   */
  int ident_sock;		  /* socket used by ident       */
  u_short peer_port;		  /* port of peer               */
  event_t *ident_event; 	  /* timeout event for ident    */
  int connected;                  /* mode of 'connectedness'    */
  int wait;                       /* wait for how many loops    */
  char *showstr_head;             /* for paging through texts   */
  char *showstr_point;            /*       -                    */
  char **str;                     /* for the modify-str system  */
  int max_str;                    /* -                          */
  sstring_t** sstr;		  /* alternate form...		*/
  void (*input_fun)(struct char_data *, char *);
                                  /* for editors etc.           */
  int prompt_mode;                /* control of prompt-printing */
  char buf[MAX_STRING_LENGTH];    /* buffer for raw input       */
  char last_input[MAX_INPUT_LENGTH];/* the last input           */
  struct txt_q output;            /* q of strings to send       */
  struct txt_q input;             /* q of unprocessed input     */
  struct char_data *character;    /* linked to char             */
  struct snoop_data snoop;        /* to snoop people.	        */
  int pwd_attempt;

};

struct msg_type
{
    char *attacker_msg;  /* message to attacker */
    char *victim_msg;    /* message to victim   */
    char *room_msg;      /* message to room     */
};

struct message_type
{
    struct msg_type die_msg;      /* messages when death            */
    struct msg_type miss_msg;     /* messages when miss             */
    struct msg_type hit_msg;      /* messages when hit              */
    struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
    struct msg_type god_msg;      /* messages when hit on god       */
    struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
    int a_type;               /* Attack type 						 */
    int number_of_attacks;	  /* How many attack messages to chose from. */
    struct message_type *msg; /* List of messages.				 */
};

struct dex_skill_type
{
    sh_int p_pocket;
    sh_int p_locks;
    sh_int traps;
    sh_int sneak;
    sh_int hide;
};

struct dex_app_type
{
    sh_int reaction;
    sh_int miss_att;
    sh_int defensive;
};

struct str_app_type
{
    sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
    sh_int todam;    /* Damage Bonus/Penalty                */
    sh_int carry_w;  /* Maximum weight that can be carrried */
    sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
    byte bonus;       /* how many bonus skills a player can */
    /* practice pr. level                 */
};

struct int_app_type
{
    byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
    sh_int hitp;
    sh_int shock;
};

/************************************************************/

typedef void (*breath_func)(ubyte level, struct char_data *ch, char *arg,
			    int type, struct char_data *tar,
			    struct obj_data *tar_o);
struct breaths {
    breath_func	func;
    int mana_cost;
};

typedef struct obj_data OBJ_DATA;
typedef struct char_data CHAR_DATA;

// the following code added by raist 2005 for VOTE support

// Vote stuff for hero points
struct nominee_data {
  CHAR_DATA * nominee;
  int votes;
};

typedef struct  nominee_data   NOMINEE_DATA;
extern          NOMINEE_DATA   *nomi;

struct hero_vote_data {
  // Could make this into a linked list, would make better sense
  NOMINEE_DATA * nominees[100];
  CHAR_DATA * voters[100];
  int total_votes;
  int total_nominees;
  bool vote_start;
  sh_int      going;  /* 1,2, sold */
  sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

typedef struct  hero_vote_data VOTE_DATA; // vote data
extern          VOTE_DATA      *vote;

/* The following code added by Min 1996 for AUCTION support */
struct auction_data {
  OBJ_DATA  * item;   /* a pointer to the item */
  CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
  CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
  int         bet;    /* last bet - or 0 if noone has bet anything */
  sh_int      going;  /* 1,2, sold */
  sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

typedef struct  auction_data AUCTION_DATA; /* auction data */
extern          AUCTION_DATA      *auction;

struct flagdata {
    unsigned int flag;
    char name[30];
};

struct fedit_data {
    char filename[30];
    char *content;
    struct char_data *ch;
};

/*****************************************************
*  For altars to work, you have to place the room number
* in the res_altar_data res_altar_room list in constants.c
******************************************************/

struct res_altar_data {
    room_num room;
    int maxlvl;
};
#endif

