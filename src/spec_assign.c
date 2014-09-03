#include "config.h"
#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "spec.h"
#include "utility.h"
#include "shop.h"
#include "board.h"
#include "smart_mobs.h"
#include "mobile_ui.h"
#include "vnum_mob.h"
#include "proto.h"

struct special_proc_entry
{
    int	vnum;
    spec_proc_func proc;
};

/**************************************************************************
* please insert new assignments in ascending vnum order to stay tidy      *
**************************************************************************/


struct special_proc_entry mob_specials[] =
{
  { 1, generic_thief },         /* ranken herald        */
  { 2, Ringwraith },		/* nazgul ringwraith	*/
  { 3, tormentor },		/* hell tormentor	*/

  { VMOB_FIRE, fire_elemental },	/* fire elemental	*/
  { VMOB_WATER, water_elemental },	/* water elemental	*/
  { VMOB_EARTH, earth_elemental },	/* earth elemental	*/
  { VMOB_AIR, air_elemental },	/* air elemental	*/

  { VMOB_HITDM, dungeon_master},	/* dm hp		*/
  { VMOB_MANADM, dungeon_master},	/* dm mana		*/
  { VMOB_STRDM, dungeon_master},	/* dm str		*/
  { VMOB_INTDM, dungeon_master},	/* dm int		*/
  { VMOB_WISDM, dungeon_master},	/* dm wis		*/
  { VMOB_DEXDM, dungeon_master},	/* dm dex		*/
  { VMOB_CONDM, dungeon_master},	/* dm con		*/
  { VMOB_HUNDM, dungeon_master},	/* dm hunger		*/
  { VMOB_THRDM, dungeon_master},	/* dm thirst		*/

  { VMOB_73, mage_guildmasters },	/* mage guildmaster	*/
  { VMOB_74, cleric_guildmasters },	/* cleric guildmaster	*/
  { VMOB_75, thief_guildmasters },      /* thief guildmaster	*/
  { VMOB_76, warrior_guildmasters },	/* warrior guildmaster	*/
  { VMOB_77, mage_guildmasters },	/* mage guildmaster	*/
  { VMOB_78, cleric_guildmasters }, 	/* cleric guildmaster	*/
  { VMOB_79, thief_guildmasters }, 	/* thief guildmaster	*/
  { VMOB_80, warrior_guildmasters },	/* warrior guildmaster	*/
  { VMOB_81, druid_guildmasters },	/* druid guildmaster	*/
  { VMOB_82, paladin_guildmasters },	/* paladin guildmaster	*/
  { VMOB_83, psi_guildmasters },	/* psi guildmaster	*/
  { VMOB_84, ranger_guildmasters },	/* ranger guildmaster	*/
  { VMOB_85, druid_guildmasters },	/* druid guildmaster	*/
  { VMOB_86, ranger_guildmasters },	/* ranger guildmaster	*/
  { VMOB_87, paladin_guildmasters },	/* paladin guildmaster	*/
  { VMOB_88, psi_guildmasters },	/* psi guildmaster	*/
  { VMOB_89, ranger_guildmasters },	/* ranger guildmaster	*/
  { VMOB_90, paladin_guildmasters },	/* paladin guildmaster	*/
  { VMOB_91, druid_guildmasters },	/* druid guildmaster	*/
  { VMOB_92, psi_guildmasters },	/* psi guildmaster	*/
  { VMOB_155, monk_guildmasters },       /* monk guildmaster     */
  { VMOB_131, bard_guildmasters },       /* bard guildmaster     */
  { 180, monk_guildmasters }, /* monk guildmaster */
  { 181, monk_guildmasters }, /* monk guildmaster */
  { 182, monk_guildmasters }, /* monk guildmaster */

 /*** Ancient Ruins ***/
  { 139, nightmare },           /* skeletal vampire     */
  { 140, alkian_warrior },      /* undead hoarde        */
  { 141, alkian_warrior },      /* blade                */
  { 142, generic_ranger },      /* winged demon         */
  { 143, nightmare },           /* Thorn                */
  { 144, githyanki_knight },    /* Tarna                */
  { 145, BreathWeapon },        /* Tarrasque            */
  { 146, acidcaster },          /* transparent figure   */
  { 147, generic_ranger },      /* barbarian guard      */
  { 148, astral_beholder },     /* eyewing              */
  { 149, generic_ranger },      /* bees                 */
  { 150, generic_ranger },      /* lizardman            */
  { 151, poisoncaster },        /* ankheg               */
  { 152, generic_druid },       /* tree                 */
  { 153, alkian_fetch },        /* tree woa             */
  { 154, githyanki_knight },    /* red hawk             */
  { 155, alkian_undertaker },   /* creeping doom        */
  { 156, generic_warrior },     /* fawn                 */
  { 157, generic_warrior },     /* beetle               */
  { 158, githyanki_mage },      /* spawn                */
  { 159, generic_alkian },      /* deepspawn            */
  { 160, generic_paladin },     /* barbarian leader     */
  { 161, generic_cleric },      /* storyteller          */
  { 162, generic_ranger },      /* fur shop owner       */
  { 163, generic_warrior },     /* barbarian child      */
  { 164, generic_alkian },      /* krom                 */
  { 165, generic_ranger },      /* arena master         */
  { 166, generic_alkian },      /* barbarian sentinal   */
  { 167, nightmare },           /* witchdoctor          */
  { 168, old_generic_cleric },  /* barbarian healer     */
  { 169, generic_ranger },      /* barbarian female     */
  { 170, acidcaster },          /* black widow          */
  { 171, BreathWeapon },        /* mist dragon          */
  { 172, generic_psi },         /* souls of ancients    */
  { 173, githyanki_mage },      /* warlock              */
  { 174, generic_mage },        /* evil priest          */
  { 175, generic_ranger },      /* pirahna              */
  { 176, alkian_undertaker },   /* queen bee            */
  { 177, generic_warrior },     /* demon general        */
  { 178, generic_druid },       /* demon colonal        */
  { 179, generic_paladin },     /* luthius              */

  /*** temple mobs ***/
  { 210, snake },		/* spider		*/
  { 211, generic_warrior },	/* gnoll		*/
  { 220, generic_warrior },	/* fighter		*/
  { 223, ghoul },		/* ghoul		*/
  { 226, generic_warrior},	/* ogre			*/
  { 227, snake },		/* spider		*/
  { 230, BreathWeapon },	/* black dragon		*/
  { 232, blink },		/* blink dog		*/
  { 233, BreathWeapon },	/* blue dragon		*/
  { 234, old_generic_cleric },	/* dwarven cleric	*/
  { 236, ghoul },		/* ghast		*/
  { 239, shadow },		/* shadow		*/
  { 240, snake },		/* poison toad		*/
  { 241, generic_thief },	/* elven assassin	*/
  { 243, BreathWeapon },	/* white dragon		*/
  { 247, generic_warrior },	/* minotaur		*/
  { 248, snake },		/* two headed snake	*/
  { 249, snake },		/* rattlesnake		*/
  { 250, snake },		/* spider		*/
  { 251, CarrionCrawler },	/* carrion crawler	*/
  { 253, BreathWeapon },	/* hydra		*/
  { 257, generic_mage },	/* ogre magi		*/
  { 261, generic_warrior },	/* hill giant		*/

  /*** Ascore ***/
  { 320, generic_mage },        /* entrapped god         */
  { 321, astral_beholder },     /* chaos spirit          */
  { 322, githyanki_mage },      /* guardian of fire      */
  { 323, generic_druid    },    /* guardian of wind      */
  { 324, astral_beholder  },    /* guardian of water     */
  { 325, earth_elemental },     /* guardian of earth     */

   /*** Saddlestream ***/
  { 326, BreathWeapon },        /* aldirian dragon       */
  { 327, generic_warrior },     /* hobgoblin chief       */
  { 329, generic_thief },       /* kobold warlord        */
  { 333, generic_mage },        /* shaman                */
  { 335, old_generic_cleric },  /* wocan                 */

  /*** Fire Giant Upgrade ***/
  { 342, alkian_warrior },      /* fire giant god        */
  { 343, generic_cleric },      /* fire monk             */
  { 344, magic_user2 },         /* sunak                 */
  { 345, magic_user2 },         /* arnak                 */
  { 346, generic_ranger },      /* temple guard          */
  { 347, old_generic_cleric },  /* sage monk             */
  { 348, generic_psi },         /* grandmaster monk      */

  /*** Zy'Tul ***/
  { 349, generic_mage },        /* beholder             */
  { 350, generic_mage },        /* death kiss           */
  { 351, astral_beholder },     /* spectator            */
  { 352, magic_user2 },         /* undead beholder      */
  { 353, generic_ranger },      /* director             */
  { 355, astral_beholder },     /* mother beholder      */
  { 358, generic_ranger },      /* overseer             */
  { 359, astral_beholder },     /* forest elf           */
  { 360, old_generic_cleric },  /* Lomajis              */

  /*** troy ***/
  { 533, generic_warrior },	/* menelaus		*/
  { 534, generic_warrior },	/* greater ajax		*/
  { 535, generic_warrior },	/* lesser ajax		*/
  { 536, generic_ranger },	/* teucer		*/
  { 537, generic_warrior },	/* diomedes		*/
  { 538, generic_warrior },	/* odysseus		*/
  { 541, generic_ranger },	/* philoctetes		*/
  { 546, water_elemental },	/* neptune		*/
  { 549, generic_warrior },	/* priam		*/
  { 551, generic_warrior },	/* hector		*/
  { 552, generic_warrior },	/* paris		*/
  { 555, generic_warrior },	/* aeneas		*/
  { 581, githyanki_knight },    /* achilles             */
  { 582, generic_warrior },	/* patroclus		*/

  { VMOB_POSS_PET, possession_mob },	/* possession mob	*/
  { VMOB_EROK, practice_master },	/* multi trade in mob	*/

  /*** new shire mobs ***/
  { 722, generic_warrior },	/* thorin		*/
  { 753, lone_troll },		/* bert troll		*/
  { 754, lone_troll },		/* tom troll		*/
  { 755, lone_troll },		/* bill troll		*/
  { 776, rivendell_guard },	/* elf guard		*/
  { 780, generic_druid },	/* gardener		*/
  { 786, old_generic_cleric },	/* loremaster		*/
  { 787, generic_psi },		/* soothsayer		*/
  { 790, old_generic_cleric },	/* elrond		*/
  { 791, rivendell_gandalf },	/* gandalf		*/
  { 1000, generic_mage },       /* elven wizard         */

  /*** githyazai ***/
  { 1150, fido },               /* grey wolf            */
  { 1155, generic_druid },      /* ethreal mist         */
  { 1157, generic_warrior },    /* rock troll woman     */
  { 1159, githyanki_knight },   /* rock chieftan        */
  { 1160, snake },              /* rock snake           */
  { 1161, ghoul },              /* woodcutter ghost     */
  { 1162, blink },              /* gith spy             */
  { 1163, githyanki_mage },     /* zifnab               */
  { 1164, generic_thief },      /* thief                */
  { 1165, nightmare },          /* wisp                 */
  { 1166, astral_ranger },      /* gith scout           */
  { 1169, generic_warrior },    /* gith guard           */
  { 1170, generic_warrior },    /* gith sent            */
  { 1171, old_generic_cleric }, /* gith priest          */
  { 1172, blocker },            /* gith blocker         */
  { 1173, generic_warrior },    /* gith soldier         */
  { 1174, githyanki_mage },     /* gith mage            */
  { 1175, astral_ranger },      /* assasin              */
  { 1176, githyanki_knight },   /* champion             */
  { 1177, old_generic_cleric }, /* high cleric          */
  { 1178, nightmare },          /* archmage             */
  { 1179, generic_psi },        /* counselor            */
  { 1180, astral_beholder },    /* king                 */
  { 1181, generic_warrior },    /* gateguard            */
  { 1183, githyanki_knight },   /* captain              */
  { 1184, astral_ranger },      /* maris                */
  { 1185, generic_warrior },    /* jailor               */


  /*** chessboard ***/
  { 1404, generic_warrior },    /* black king           */
  { 1428, jugglernaut },	/* jugglernaut		*/
  { 1460, generic_warrior },    /* white king           */
  { 1470, jabberwocky },        /* jabberwocky          */
  { 1472, flame },              /* flame                */
  { 1495, delivery_elf },	/* delivery elf		*/
  { 1493, delivery_beast },     /* delivery beast       */
  { VMOB_121, generic_warrior },        /* sisyphus             */

  /*** tower of sorcery ***/
  { 1500, shadow },		/* shadow		*/
  { 1504, generic_mage },	/* young mage		*/
  { 1505, generic_mage },	/* mage bartender	*/
  { 1506, generic_mage },	/* tatorius		*/
  { 1507, generic_mage },	/* ezmerelda		*/
  { 1508, generic_mage },	/* assistant mage	*/
  { 1510, generic_mage },	/* jailor mage		*/
  { 1514, generic_mage },	/* visitor mage		*/
  { 1515, generic_mage },	/* apprentice mage	*/
  { 1516, generic_mage },	/* apprentice mage	*/
  { 1517, generic_mage },	/* apprentice mage	*/
  { 1518, generic_mage },	/* apprentice mage	*/
  { 1520, generic_mage },	/* student		*/
  { 1521, generic_mage },	/* instructor		*/
  { 1522, generic_mage },	/* student		*/
  { 1523, generic_mage },	/* teacher		*/
  { 1524, generic_mage },	/* student		*/
  { 1525, generic_mage },	/* wizard		*/
  { 1526, generic_mage },	/* young mage		*/
  { 1527, generic_mage },	/* battle mistress	*/
  { 1528, generic_mage },	/* teacher		*/
  { 1529, generic_mage },	/* wizard		*/
  { 1530, generic_mage },	/* scribe		*/
  { 1531, generic_mage },	/* scribe		*/
  { 1532, generic_mage },	/* master scribe	*/
  { 1533, generic_mage },	/* assistanr scribe	*/
  { 1534, generic_mage },	/* enchanter		*/
  { 1536, generic_mage },	/* alchemist		*/
  { 1537, generic_mage },	/* summoner		*/
  { 1538, generic_mage },	/* charmer		*/
  { 1540, generic_mage },	/* master binder	*/
  { 1541, generic_mage },	/* golem maker		*/
  { 1548, generic_mage },	/* illusionist		*/
  { 1549, generic_mage },	/* necromancer		*/
  { 1552, generic_mage },	/* witch		*/
  { 1553, generic_mage },	/* librarian		*/
  { 1554, generic_mage },	/* apprentice		*/
  { 1556, generic_mage },	/* grey master		*/
  { 1557, generic_mage },	/* black apprentice	*/
  { 1559, generic_mage },	/* black master		*/
  { 1560, generic_mage },	/* white apprentice	*/
  { 1562, generic_mage },	/* white master		*/
  { 1563, githyanki_knight },   /* calico cat           */
  { 1564, generic_mage },	/* grand master		*/

  /*** monks ***/
  { 1713, generic_warrior },    /* monk grand master    */

  /*** newbie zone ***/
  { 1807, githyanki_mage },     /* avatar of dreugh */
  { 1808, githyanki_mage },     /* spectral guardian */

  /*** draconians ***/
  { 2003, generic_mage },	/* wizard		*/
  { 2004, old_generic_cleric },	/* cleric		*/
  { 2020, BreathWeapon },	/* tiamat		*/
  { 2021, BreathWeapon },	/* red dragon		*/
  { 2022, BreathWeapon },	/* black dragon		*/
  { 2023, BreathWeapon },	/* white dragon		*/
  { 2024, BreathWeapon },	/* gold dragon		*/
  { 2025, BreathWeapon },	/* green dragon		*/
  { 2026, BreathWeapon },	/* hydra dragon		*/
  { 2040, generic_warrior },	/* zombie		*/
  { 2041, BreathWeapon },	/* phase dragon		*/

  /*** ogre village ***/
  { 2206, generic_warrior },	/* swamp wraith		*/
  { 2208, generic_warrior },	/* ogre warrior		*/
  { 2209, generic_warrior },	/* ogre guard		*/
  { 2210, old_generic_cleric },	/* shaman		*/
  { 2211, generic_warrior },	/* ogre chieftain	*/

  /*** mahntor ***/
  { 2212, generic_warrior },	/* barbarian		*/
  { 2217, generic_warrior },	/* minotaur warrior	*/
  { 2218, generic_warrior },	/* minotaur gatekeeper	*/
  { 2219, generic_warrior },	/* minotaur high guard	*/
  { 2220, generic_warrior },	/* minotaur royal guard	*/
  { 2221, generic_warrior },	/* minotaur elite guard	*/
  { 2223, generic_warrior },    /* minotaur bodyguard   */
  { 2224, generic_warrior },	/* minotaur ring keeper	*/
  { 2225, generic_warrior },	/* gorak		*/
  { 2226, generic_psi },	/* darkoth		*/
  { 2227, generic_paladin },	/* tyrgoth		*/
  { 2228, magic_user }, 	/* amyrok		*/
  { 2229, old_generic_cleric },	/* sumaron		*/
  { 2230, generic_druid },	/* nasturn		*/
  { 2231, generic_ranger },	/* belrak		*/
  { 2232, generic_thief },	/* dorgar		*/
  { 2225, generic_warrior },	/* gorak		*/
  { 2233, generic_warrior },	/* mahntor		*/

  /*** oasis ***/
  /* { 2301, old_generic_cleric },nadron 		*/
  /* { 2302, generic_warrior },	 sheik	        	*/
  /* { 2306, generic_warrior },	 surviving warrior	*/
  /* { 2352, generic_cleric },	 dragosani		*/

  /* { 2400, generic_mage },	 talonus		*/

  /*** loring ***/
  { 2500, generic_ranger },	/* silvanus		*/
  { 2502, BreathWeapon },	/* black dragon		*/
  { 2503, generic_warrior },	/* myconoid		*/
  { 2506, generic_warrior },	/* krell		*/
  { 2507, generic_druid },	/* loring		*/
  { 2508, generic_druid },	/* treant		*/

  /*** tyrsis ***/
  { 2703, thief },		/* cutpurse		*/
  { 2705, generic_warrior },	/* guard		*/
  { 2706, generic_mage },	/* seeker		*/
  { 2708, generic_warrior },	/* guard		*/
  { 2709, generic_warrior },	/* training guard	*/
  { 2710, generic_warrior },	/* sergeant		*/
  { 2711, generic_mage },	/* training seeker	*/
  { 2712, generic_warrior },	/* corporal		*/
  { 2713, generic_warrior },	/* guard		*/
  { 2714, generic_warrior },	/* guard		*/
  { 2716, generic_warrior },	/* sergeant		*/
  { 2719, tyrsis_magician },	/* street magician	*/
  { 2723, generic_mage },	/* shadowen		*/
  { 2724, generic_mage },	/* shadowen		*/
  { 2725, generic_mage },	/* shadowen		*/
  { 2726, generic_mage },	/* shadowen		*/
  { 2727, generic_mage },	/* shadowen		*/
  { 2728, githyanki_mage },       /* rimmer dall          */
  { 2723, generic_mage },	/* shadowen		*/
  { 2730, generic_mage },	/* sewer shadowen	*/

  /*** vikings ***/
  { 2903, generic_warrior },	/* warrior		*/
  { 2904, generic_warrior },	/* blacksmith		*/
  { 2907, old_generic_cleric },	/* healer		*/
  { 2908, generic_warrior },	/* bodyguard		*/
  { 2909, generic_warrior },	/* leader		*/
  { 2910, snake },              /* gunther reptile      */
  { 2912, snake },              /* serpent              */
  { 2913, generic_warrior },	/* fjalar		*/
  { 2916, generic_warrior },	/* sigurd		*/
  { 2917, generic_warrior },	/* beowulf		*/
  { 2918, firecaster },         /* surtr                */
  { 2919, coldcaster },         /* thrym                */
  { 2921, astral_ranger },      /* loki                 */
  { 2922, generic_warrior },	/* baldur		*/
  { 2923, generic_warrior },	/* tyr			*/
  { 2924, generic_druid },	/* idun			*/
  { 2925, generic_psi },	/* forseti		*/
  { 2926, generic_warrior },	/* frey			*/
  { 2928, generic_mage },	/* hel			*/
  { 2929, generic_warrior },	/* heimdall		*/
  { 2933, generic_warrior },	/* thor			*/
  { 2934, astral_ranger },      /* odin                 */
  { 2935, astral_ranger },      /* odin                 */


  /*** midgaard ***/
  {  450, receptionist },       /* Nirassi        */
  { 2999, old_generic_cleric }, /* kvorki               */
  { 3000, generic_mage },	/* laron		*/
  { 3003, generic_warrior },	/* weaponsmith		*/
  { 3004, generic_warrior },	/* armorer		*/
  { 3005, receptionist },	/* receptionist		*/
  { 3004, generic_warrior },	/* armorer		*/
  { VMOB_122, mage_guildmasters },/* mage guildmaster	*/
  { VMOB_123, cleric_guildmasters },/* cleric guildmaster	*/
  { VMOB_124, thief_guildmasters },/* thief guildmaster	*/
  { VMOB_125, warrior_guildmasters },/* warrior guildmaster  */
  { VMOB_97, blocker },		/* guildguard		*/
  { VMOB_98, blocker },		/* guildguard		*/
  { VMOB_99, blocker },		/* guildguard		*/
  { VMOB_100, blocker },		/* guildguard		*/
  { VMOB_101, blocker },		/* guildguard		*/
  { VMOB_102, blocker },		/* guildguard		*/
  { VMOB_103, blocker },		/* guildguard		*/
  { VMOB_104, blocker },		/* guildguard		*/
  { VMOB_132, blocker },                /* guildguard           */
  { 3016, old_generic_cleric }, /* durwin               */
  { 3034, generic_ranger },	/* andrew		*/
  { 3040, generic_warrior },	/* bartender		*/
  { 3042, generic_mage },	/* mage waiter		*/
  { 3043, old_generic_cleric },	/* cleric waiter	*/
  { 3044, generic_thief },	/* thief waiter		*/
  { 3045, generic_warrior },	/* warrior waiter	*/
  { VMOB_70, cityguard },		/* cityguard		*/
  { 3061, janitor },		/* janitor		*/
  { 3063, generic_warrior},	/* mercenary		*/
  { 3066, fido },		/* vampire bat		*/
  { VMOB_71, cityguard },		/* cityguard		*/
  { VMOB_72, cityguard },		/* guard royale		*/
  { 3070, RepairGuy },		/* repair guy		*/
  { 3071, RepairGuy },		/* super repair guy	*/
  { 3072, generic_paladin },	/* honor guard		*/
  { 3073, generic_paladin },	/* honor guard		*/
  { 3074, generic_paladin },    /* bard sys             */
  { 3075, astral_ranger },      /* Arman Hell Hound     */
  { 3076, githyanki_knight },   /* Bourne Hell Hound    */
  { 3077, githyanki_knight },   /* Quag Hell Hound      */
  { 3078, astral_ranger },      /* Razkuli Hell Hound   */
  { 3079, githyanki_knight },   /* Zalbar Hell Hound    */
  { 3080, astral_ranger },      /* Walegrin             */
  { 3081, generic_ranger },     /* Hawkmask             */
  { 3082, astral_ranger },      /* Jubal                */
  { 3140, generic_warrior},	/* chief guard		*/
  { 3141, generic_warrior},	/* cityguard		*/
  { 3143, mayor },		/* midgaard mayor	*/
  { 3160, astral_ranger },      /* Aye-Gophlan          */
  { 3161, generic_mage },       /* melilot              */
  { 3162, generic_druid },      /* Alten                */
  { 3163, generic_warrior },    /* Blacktounge          */
  { 3164, generic_warrior },    /* lorna                */
  { 3165, generic_warrior },    /* Herewick             */
  { 3166, generic_warrior },    /* Pantaleone           */
  { 3167, generic_cleric },     /* Lakmed               */
  { 3168, generic_warrior },    /* Zapala               */
  { 3169, generic_ranger },     /* Brigitte             */


#if NEWPKILL
  { 29903, judge }, 		/* midgaard judge	*/
#endif

  /*** pancea palace ***/
  { 3206, mob_hero_ring },	/* pancea king		*/
  { 3207, herohunter },		/* wraith of pancea	*/
  { 3208, generic_paladin },	/* sentinal guard	*/
  { 3209, generic_paladin },	/* wandering guard	*/

  /*** siralyr birdland ***/
  { 3250, generic_warrior },    /* vulture outcast      */
  { 3251, generic_ranger },     /* raven raider         */
  { 3252, githyanki_knight },   /* dust bunny           */
  { 3253, generic_warrior },    /* partridge peddler    */
  { 3254, generic_warrior },    /* vutureman warrior    */
  { 3255, old_generic_cleric }, /* governer goose       */
  { 3256, fido },               /* loony bird           */
  { 3257, generic_warrior },    /* child                */
  { 3258, generic_psi },        /* serving finch        */
  { 3259, generic_warrior },    /* parrot  patron       */
  { 3260, githyanki_knight },   /* charis battlehawk    */
  { 3261, old_generic_cleric }, /* councilman           */
  { 3262, generic_paladin },    /* jay buzzbeak         */
  { 3263, generic_warrior },    /* robin goodwing       */
  { 3264, generic_druid },      /* crow wyndwing        */
  { 3265, generic_warrior },    /* pidgeon grocer       */
  { 3266, astral_ranger },      /* sparrowhawk darkwing */
  { 3267, generic_warrior },    /* lark windsong        */
  { 3268, generic_ranger },     /* falconian guardsman  */
  { 3269, generic_warrior },    /* flamingo aristocrat  */
  { 3270, astral_beholder },    /* rothen redwing       */
  { 3271, generic_warrior },    /* lord gildenwing      */
  { 3272, generic_warrior },    /* ostrich aristocrat   */
  { 3273, generic_warrior },    /* handsome swan        */
  { 3274, generic_warrior },    /* lovely swan          */
  { 3275, astral_ranger },      /* falconian weapons    */
  { 3276, generic_warrior },    /* falconian_gateguard  */
  { 3277, generic_warrior },    /* roadrunner page      */
  { 3278, generic_warrior },    /* peacock gentleman    */
  { 3279, generic_warrior },    /* dove maiden          */
  { 3280, astral_beholder },    /* swan queen           */
  { 3281, nightmare },          /* eagle king           */
  { 3282, githyanki_mage },     /* ravenman sorcerer    */
  { 3283, generic_warrior },    /* bard                 */
  { 3284, astral_ranger },      /* roc vicious          */
  { 3285, generic_warrior },    /* roc chick            */
  { 3286, generic_warrior },    /* magpie gossip        */
  { 3288, githyanki_knight },   /* royal guard          */


  { 3209, generic_paladin },	/* wandering guard	*/
  { 3300, astral_ranger },      /* niko                 */
  { 3301, astral_ranger },      /* janni                */
  { 3302, nightmare },          /* abarsis              */
  { 3303, astral_ranger },      /* crit                 */
  { 3304, githyanki_mage },     /* snapper jo           */
  { 3305, nightmare },          /* roxanne              */
  { 3306, nightmare },          /* askelon              */
  { 3307, githyanki_knight },   /* bashir               */
  { 3308, astral_ranger },      /* kama                 */
  { 3400, old_generic_cleric }, /* shade cleric         */

  { 3504, thief },              /* robber               */
  { 3505, astral_beholder },	/* balrog		*/

  /*** cloud giants ***/
  { 3553, generic_warrior },	/* guard		*/
  { 3554, generic_warrior },	/* guard		*/
  { 3556, generic_warrior },	/* kronos		*/
  { 3557, generic_warrior },	/* gorn			*/
  { 3558, generic_warrior },	/* flash		*/
  { 3559, snake },		/* wyvern		*/
  { 3560, generic_warrior },	/* garuk		*/
  { 3561, generic_warrior },	/* commander		*/
  { 3562, generic_mage },	/* elmorn		*/
  { 3563, air_elemental },	/* arishh		*/
  { 3564, old_generic_cleric },	/* shaman		*/
  { 3571, generic_warrior},	/* fortress guard	*/

  /*** new thalos ***/
  { 3600, generic_mage },	/* retired mage		*/
  { 3601, old_generic_cleric },	/* retired cleric	*/
  { 3602, generic_warrior },	/* retired warrior	*/
  { 3603, generic_thief },	/* retired thief	*/
  { 3604, receptionist },	/* receptionist		*/
  { 3605, generic_mage },	/* mage bartender	*/
  { 3629, old_generic_cleric },	/* aziz			*/
  { 3630, generic_thief },	/* mustafah		*/
  { 3631, generic_mage },	/* fatima		*/
  { 3632, generic_warrior },	/* kareem		*/
  { 3635, thief },		/* skinny kid		*/
  { 3638, generic_thief },	/* gord			*/
  { 3639, generic_paladin },	/* caramon		*/
  { 3640, generic_mage },	/* raistlin		*/
  { 3641, generic_druid },	/* curley		*/
  { 3644, fido },		/* vulture		*/
  { 3647, air_elemental },	/* dirt devil		*/
  { 3649, generic_thief },	/* dervish		*/
  { 3650, generic_warrior },	/* sultan		*/
  { 3653, generic_warrior },	/* eunich		*/
  { 3655, generic_mage },	/* allah		*/
  { 3656, generic_mage },	/* guildguard		*/
  { 3657, old_generic_cleric },	/* guildguard		*/
  { 3658, generic_warrior },	/* guildguard		*/
  { 3659, generic_thief },	/* guildguard		*/
  { VMOB_93, SultanGuard },	/* guard		*/
  { 3662, SultanGuard },	/* guard		*/
  { 3670, BreathWeapon },	/* cryohydra		*/
  { 3679, generic_mage },	/* elvira		*/
  { 3680, generic_mage },	/* braheem		*/
  { 3681, old_generic_cleric },	/* high priest		*/
  { VMOB_94, SultanGuard },	/* royal guard		*/
  { 3685, generic_druid },	/* gardener		*/
  { 3698, generic_warrior },    /* mage's bodyguard     */
  { 3699, generic_mage },       /* mage                 */
  /* { 3715, firecaster },		 magman		*/
  /* { 3742, old_generic_cleric },shade cleric		*/
  /* { 3745, generic_warrior },	 ogre lord		*/

  /*** aucan ***/
  { 3836, generic_warrior },    /* guard l13            */
  { 3837, generic_warrior },    /* guard l13            */
  { 3838, old_generic_cleric }, /* man l20              */
  { 3840, githyanki_knight },   /* warrior master l18   */
  { 3841, generic_warrior },    /* warrior l12          */
  { 3842, generic_warrior },    /* warrior l16          */
  { 3843, generic_warrior },    /* warrior l16          */
  { 3844, generic_paladin },    /* elder l25            */
  { 3847, generic_warrior },    /* guard l19            */
  { 3849, generic_warrior },    /* chieftain l30        */
  { 3853, ghoul },              /* ghost aucan l13      */
  { 3854, ghoul },              /* tjowi                */
  { 3855, ghoul },              /* bawji                */
  { 3856, ghoul },              /* adjitu               */
  { 3860, ghoul },              /* ghost l40            */
  { 3869, astral_ranger },      /* general l40          */

  /*** arena ***/
  { 4000, snake },		/* snake		*/
  { 4001, snake },		/* snake		*/
  { 4050, generic_warrior },	/* warrior		*/
  { 4051, generic_warrior },	/* warrior		*/
  { 4053, snake },		/* snake		*/
  { 4100, generic_mage },	/* mage			*/
  { 4102, snake },		/* snake		*/
  { 4103, thief },		/* thief		*/
  { 4106, generic_warrior },	/* warrior		*/

  /*** dojo ***/
  { 4126, astral_beholder },	/* avatar		*/
  { 4127, generic_thief },      /* tung mai             */
  { 4128, generic_psi },        /* tem ki               */
  { 4129, githyanki_mage },     /* tsi lo               */
  { 4130, astral_ranger },      /* wa san               */
  { 4131, generic_thief },      /* ninja                */
  { 4132, generic_thief },      /* ninja                */
  { 4133, generic_mage },       /* shukenja             */
  { 4134, generic_mage },       /* shukenja             */
  { 4135, generic_ranger },     /* bushi                */
  { 4136, old_generic_cleric},	/* sohei		*/

  /*** mongols ***/
  { 4201, generic_warrior },    /* ghengis khan         */
  { 4202, generic_warrior },	/* evil jailer */
  { 4203, old_generic_cleric },	/* shaman		*/
  { 4204, generic_mage },       /* mongul princess */
  { 4205, generic_warrior },	/* queen		*/
  { 4206, generic_warrior },	/* general		*/
  { 4207, generic_warrior },	/* mongul guard */
  { 4208, ghoul },		/* ghost		*/

  /*** caves ***/
  { 5000, generic_thief },	/* dervish		*/
  { 5001, generic_thief },	/* dervish		*/
  { 5002, snake },		/* snake		*/
  { 5003, snake },		/* scorpion		*/
  { 5005, BreathWeapon },	/* brass dragon		*/
  { 5007, generic_warrior },	/* nomad commander	*/
  { 5008, generic_warrior },	/* nomad warrior	*/
  { 5010, firecaster },		/* dracolich		*/
  { 5014, old_generic_cleric },	/* unglemyelia		*/

  /*** drow ***/
  { 5102, generic_warrior },	/* warrior		*/
  { 5103, generic_mage },	/* noble mage		*/
  { 5104, old_generic_cleric },	/* priestess		*/
  { 5105, generic_warrior },	/* master		*/
  { 5106, generic_warrior },	/* weaponsmaster	*/
  { 5107, old_generic_cleric },	/* matron mother	*/
  { 5108, old_generic_cleric },	/* matron mother	*/
  { 5109, eleccaster },		/* yochlol		*/

  /*** old thalos ***/
  { 5200, generic_mage },	/* beholder		*/

  { 5211, generic_warrior },	/* azoun		*/
  { 5212, old_generic_cleric },	/* alaron		*/
  { 5213, generic_paladin },	/* kazgaroth		*/
  { 5214, generic_ranger },	/* onoye		*/
  { 5215, generic_druid },	/* tethyr		*/
  { 5216, generic_psi },	/* manxam		*/
  { 5217, generic_mage },	/* trebor		*/
  { 5218, generic_thief },	/* slippe		*/

  /*** pyramid ***/
  { 5303, ghoul },		/* spectre		*/
  { 5308, RustMonster },	/* rust monster		*/


  /*** Prophecy / Tanimura ***/
  { 5400, generic_ranger }, 	/* <fortune hunter> */
  { 5401, githyanki_mage }, 	/* <ant> */
  { 5402, alkian_undertaker }, 	/* <squir> */
  { 5403, generic_warrior }, 	/* <bear> */
  { 5404, all_classes }, 	/* <mriswith> */
  { 5405, BreathWeapon }, 	/* <gar> */
  { 5406, astral_ranger }, 	/* <Murra> */
  { 5407, generic_mage }, 	/* <cloud reader> */
  { 5408, astral_beholder }, 	/* <ischade> */
  { 5409, all_classes }, 	/* <quesada> */
  { 5410, all_classes }, 	/* <pasha> */
  { 5411, all_classes }, 	/* <amilia> */
  { 5412, astral_spy }, 	/* <robed figure> */
  { 5413, generic_warrior }, 	/* <spose> */
  { 5414, githyanki_mage }, 	/* <spirit> */
  { 5415, generic_psi },	/* <nathan> */
  { 5416, generic_warrior },	/* <dungeon guard> */
  { 5417, generic_mage },	/* <prophet> */
  { 5421, githyanki_mage },	/* <healer> */
  { 5422, generic_warrior },	/* <shadow guard> */
  { 5423, all_classes },	/* <paul> */
  { 5424, all_classes },	/* <mriswith> */
  { 5425, generic_warrior },	/* <skeleton> */
  { 5426, githyanki_mage },	/* <warren> */
  { 5427, githyanki_mage },	/* <gozgor> */
  { 5428, generic_warrior },	/* <boy> */
  { 5429, generic_warrior },	/* <stein> */
  { 5430, all_classes },	/* <jagang> */
  { 5431, nightmare },		/* <merissa> */
  { 5432, generic_shifter },	/* <sliph> */
  { 5433, astral_ranger },	/* <screeling> */
//  { 5434, generic_shifter },	/* <kitten> */
  { 5435, githyanki_mage },	/* <sister dark> */
  { 5436, githyanki_mage },	/* <prelate> */
  { 5437, generic_warrior },	/* <raven> */
  { 5439, githyanki_mage },	/* <sister light> */
  { 5440, generic_warrior },	/* <siddin> */
  { 5441, generic_warrior },	/* <prisoner> */
  { 5442, generic_mage },	/* <declo> */
  { 5443, generic_alkian },	/* <demmin> */
  { 5444, generic_warrior },	/* <quad> */
  { 5445, nightmare },		/* <ulicia> */
  { 5446, all_classes },	/* <slith> */
  { 5447, generic_warrior },	/* <sjekso> */

  { 5560, old_generic_cleric }, /* multec high priest   */


  { 6110, generic_druid },	/* ancient tree		*/
  { 6111, generic_druid },	/* ancient tree		*/
  { 6112, BreathWeapon },	/* green dragon		*/
  { 6113, snake },		/* spider		*/
  { 6114, snake },		/* queen spider		*/

  /*** quicklings ***/
  { 6202, generic_warrior },	/* raider		*/
  { 6205, generic_warrior },	/* captain		*/
  { 6207, generic_warrior },	/* tephanis		*/

  /*** malacca ***/
  { 6225, generic_thief },	/* siamese rogue	*/
  { 6228, generic_warrior },	/* warehouse guard	*/
  { 6229, generic_warrior },	/* chinese bodyguard	*/
  { 6230, generic_warrior },	/* chinese chieftain	*/
  { 6231, generic_warrior },	/* arab chieftain	*/
  { 6233, generic_warrior },	/* temenggung		*/
  { 6234, generic_thief },	/* smuggler		*/
  { 6238, generic_warrior },	/* palace guard		*/
  { 6239, generic_warrior },	/* palace guard		*/
  { 6242, generic_warrior },	/* hang tuah		*/
  { 6245, CarrionCrawler },	/* black crawler	*/

  { 6500, generic_warrior },	/* dwarf guard		*/

  /*** camelot ***/
  { 6601, githyanki_knight },   /* launcelot            */
  { 6602, githyanki_knight },   /* arthur               */
  { 6604, generic_warrior },	/* mordred		*/
  { 6605, generic_warrior },	/* ghalad		*/
  { 6606, generic_warrior },	/* knight		*/
  { 6608, janitor },		/* squire */
  { 6609, BreathWeapon },	/* dragon		*/
  { 6610, AGGRESSIVE },         /* black knight         */
  { 6613, thief },              /* bad man              */
  { 6615, generic_mage },       /* morganna             */
  { 6616, githyanki_mage },     /* merlin               */
  { 6619, generic_warrior },	/* gawain		*/
  { 6620, githyanki_knight },	/* master of arms	*/
  { 6621, lone_troll },         /* antipaladin          */
  { 6622, githyanki_knight },	/* master antipaladin	*/
  { 6623, fido },		/* dog of war */
  { 6624, astral_beholder },	/* statue		*/
  { 6625, coldcaster },		/* demon */
  { 6642, BreathWeapon },	/* ice dragon		*/
  { 6644, AGGRESSIVE },         /* antipaladin patrol   */

   /*** Ice Haven ***/
  { 6801, generic_ranger },        /* guard tower          */
  { 6802, generic_ranger },        /* city guard IH        */
  { 6803, generic_ranger },        /* guard gate IH        */
  { 6804, generic_ranger },        /* guard keep IH        */
  { 6805, generic_ranger },        /* guard captain IH     */
  { 6806, generic_ranger },        /* guard lieutenant IH  */
  { 6807, generic_ranger },        /* sailor IH            */
  { 6808, generic_ranger },        /* frozen old man IH    */
  { 6809, generic_ranger },        /* citizen IH           */
  { 6810, thief },                 /* winter thief IH      */
  { 6811, receptionist   },        /* tavernkeeper IH      */
  { 6812, generic_ranger },        /* patron IH            */
  { 6813, generic_ranger },        /* wench IH             */
  { 6814, generic_ranger },        /* blacksmith smithy IH */
  { 6815, generic_ranger },        /* stable master IH     */
  { 6816, old_generic_cleric },    /* priestess healer IH  */
  { 6817, generic_ranger },        /* office clerk IH      */
  { 6821, generic_ranger },        /* black rat IH         */
  { 6822, generic_ranger },        /* grundil IH           */
  { 6823, generic_ranger },        /* mercenary IH         */
  { 6824, generic_ranger },        /* dignitary IH         */
  { 6825, generic_ranger },        /* weapon dealer IH     */
  { 6826, generic_ranger },        /* merchant captain IH  */
  { 6827, generic_ranger },        /* bodyguard IH         */
  { 6828, all_classes },           /* lich mage IH         */

  /*** sewers ***/
  { 7006, snake },		/* snake		*/
  { 7009, generic_paladin },	/* grand knight		*/
  { 7040, BreathWeapon },	/* red dragon		*/
  { 7041, generic_mage },	/* sea hag		*/
  { 7042, old_generic_cleric },	/* naga			*/
  { 7200, generic_psi },	/* master mindflayer	*/
  { 7201, generic_psi },	/* senior mindflayer	*/
  { 7202, generic_psi },	/* junior mindflayer	*/

  { 7500, generic_psi },        /* fenric               */

  /*** astral plane ***/
  { VMOB_FIRST_POOL, astral_pool },	/* sauria		*/
  { 7601, astral_pool },	/* abyss		*/
  { 7602, astral_pool },	/* tower of sorcery	*/
  { 7603, astral_pool },	/* cthulhu		*/
  { 7604, astral_pool },	/* hades		*/
  { 7605, astral_pool },	/* troy			*/
  { 7606, astral_pool },	/* tyrsis		*/
  { 7607, astral_pool },	/* mongols		*/
  { 7608, astral_pool },	/* drow			*/
  { 7609, astral_pool },	/* camelot		*/
  { 7610, astral_pool },	/* hill giants		*/
  { 7611, astral_pool },	/* zodiac		*/
  { 7612, astral_pool },	/* caves		*/
  { 7613, astral_pool },	/* mistamere		*/
  { 7614, astral_pool },	/* orshinigal		*/

  { 7625, githyanki_mage },     /* githyanki mage       */
  { 7626, old_generic_cleric }, /* githyanki cleric     */ /* MIN */
  { 7627, astral_ranger },      /* githyanki captain    */
  { 7628, githyanki_knight },   /* githyanki darkknight */
  { 7629, blink },              /* githyanki spy        */
  { 7630, old_generic_cleric },	/* blue slaad		*/
  { 7631, generic_psi },	/* green slaad		*/
  { 7632, generic_mage },	/* grey slaad		*/
  { 7633, fido },		/* slug			*/
  { 7634, fido },		/* grub			*/
  { 7635, blink },              /* floater              */
  { 7636, generic_mage },	/* vrock I		*/
  { 7637, astral_ranger },      /* Marilith             */
  { 7638, old_generic_cleric },	/* hezrou II		*/
  { 7639, astral_beholder },    /* beholder             */

  /*** orun ***/
  { 8001, thief },              /* ugly thug            */
  { 8002, receptionist },       /* receptionist         */
  { 8003, generic_warrior },    /* bartender            */
  { 8005, generic_psi },        /* ingalo               */
  { 8007, githyanki_mage },     /* katala               */
  { 8008, astral_beholder },    /* protector            */
  { 8009, generic_warrior },    /* urak                 */
  { 8013, generic_druid },      /* feric                */
  { 8018, generic_warrior },    /* adventurer           */
  { 8019, generic_thief },      /* thief                */
  { 8023, generic_warrior },    /* boatman              */
  { 8024, generic_paladin },    /* aluxes               */
  { 8025, generic_mage },       /* shadow lesser        */
  { 8026, snake },              /* moccasin snake       */
  { 8027, janitor },            /* caretaker            */
  { 8029, generic_warrior },    /* lizard man           */
  { 8030, fido },               /* piranha              */
  { 8031, old_generic_cleric }, /* shaman               */
  { 8032, generic_warrior },    /* elite guard          */
  { 8034, generic_warrior },    /* lizardman warrior    */


  /*** xanth ***/
  { 8500, generic_psi },        /* dream master         */
  { 8501, generic_warrior },	/* rock troll		*/
  { 8505, generic_ranger },	/* chief centaur	*/
  { 8506, generic_ranger },	/* centaur champion	*/
  { 8507, generic_mage },	/* leprechaun		*/
  { 8508, regenerator },        /* slatherer            */
  { 8509, BreathWeapon },	/* acid dragon		*/
  { 8510, generic_warrior },	/* stone giant */
  { 8513, BreathWeapon },	/* dragon		*/
  { 8514, fido },		/* wiggle		*/
  { 8515, generic_warrior },	/* ogre			*/
  { 8516, astral_beholder },    /* evil one */
  { 8517, replicant },		/* revenant		*/
  { 8518, astral_ranger },	/* ogre lord		*/
  { 8519, firecaster },		/* phoenix		*/
  { 8520, generic_warrior },	/* weakened warrior	*/
  { 8521, generic_warrior },    /* evil servant */
  { 8522, astral_beholder },    /* tangle tree          */
  { 8523, generic_mage },	/* summoner		*/
  { 8524, githyanki_knight },	/* gerrod		*/
  { 8525, astral_ranger },      /* snorklewacker        */
  { 8530, generic_paladin },    /* geroth               */
  { 8532, thief },              /* rogue                */

  /*** paramor ***/
  { 8704, generic_druid },	/* guard		*/
  { 8705, generic_druid },	/* librarian		*/
  { 8706, generic_druid },	/* mystic guard		*/
  { 8707, githyanki_mage },     /* allanon              */

  /*** zoo ***/
  { 9000, snake },		/* asp			*/
  { 9001, snake },		/* spider		*/
  { 9008, fido },		/* coyote		*/
  { 9014, snake },		/* spider		*/
  { 9015, snake },		/* scorpion		*/
  { 9021, snake },		/* gila			*/

  /*** Thief den ***/
  { 9101, generic_thief },      /* thief                */
  { 9102, thief },              /* standard thief       */
  { 9104, generic_thief },      /* thief member         */
  { 9105, astral_ranger },      /* arax                 */

  { 9206, generic_warrior },	/* insane dwarf		*/
  { 9208, old_generic_cleric },	/* undead priestess	*/
  { 9209, ghoul },		/* skeletal hill giant	*/
  { 9213, CarrionCrawler },	/* carrion crawler	*/
  { 9217, BreathWeapon },	/* blue dragon		*/

  /*** frost giants ***/
  { 9401, generic_warrior },	/* jarl			*/
  { 9403, old_generic_cleric },	/* shaman		*/
  { 9404, generic_warrior },	/* captain		*/
  { 9405, generic_thief },	/* assassin		*/
  { 9408, generic_warrior },	/* sergeant		*/
  { 9413, generic_warrior },	/* leader		*/
  { 9418, BreathWeapon },	/* white dragon		*/
  { 9419, BreathWeapon },	/* old white dragon	*/
  { 9426, generic_warrior },	/* monk			*/
  { 9427, old_generic_cleric },	/* cleric		*/
  { 9429, generic_warrior },	/* warrior		*/
  { 9431, snake },		/* snow snake		*/
  { 9432, generic_mage },	/* ogre magi		*/
  { 9435, snake },		/* poisonous toad	*/

  /*** dark society ***/
  { 9500, thief },		/* pilferer		*/
  { 9502, generic_warrior },	/* guild trainer	*/
  { 9503, generic_thief },	/* sneak		*/
  { 9507, generic_warrior },	/* legbreaker		*/
  { 9510, generic_thief },	/* rogue		*/
  { 9511, generic_thief },	/* vastin		*/
  { 9512, generic_thief },	/* shady character	*/
  { 9513, generic_warrior },	/* cityguard		*/
  { 9514, thief },		/* burglar		*/
  { 9515, generic_warrior },	/* merc			*/
  { 9516, generic_thief },	/* smuggler		*/
  { 9519, generic_thief },	/* knifer		*/
  { 9520, generic_thief },	/* killer		*/
  { 9521, generic_thief },	/* assassin		*/
  { 9522, generic_warrior },	/* bodyguard		*/
  { 9523, generic_thief },	/* assassin		*/
  { 9524, generic_thief },	/* grandmaster		*/
  { 9525, fido },		/* panther		*/

  /*** galaxy ***/
  { 10055, generic_warrior },   /* white dwarf          */
  { 10058, generic_warrior },	/* hercules		*/
  { 10060, generic_ranger },	/* orion		*/
  { 10062, BreathWeapon },	/* draco head		*/
  { 10063, BreathWeapon },	/* baby draco		*/
  { 10065, astral_ranger },     /* taurus               */
  { 10072, snake },		/* scorpio		*/
  { 10073, generic_ranger },	/* sagittarius		*/
  { 10075, water_elemental },   /* aquarius             */
  { 10078, githyanki_knight },  /* ursala               */
  { 10080, generic_warrior },	/* cepheus		*/
  { 10081, nightmare },         /* polaris              */

  /*** DNA ***/
  { 32582, receptionist },       /* jilian               */
  { 10212, generic_warrior },    /* inn keeper           */

  /*** caves ***/
  { 10700, generic_warrior },	/* bugbear grognitz	*/
  { 10701, earth_elemental },	/* master xorn		*/
  { 10702, fido },		/* greater otyugh */
  { 10705, generic_warrior },	/* margoyle king	*/
  { 10707, generic_warrior },	/* elite bugbear guard	*/
  { 10713, earth_elemental },	/* lesser xorn		*/
  { 10752, earth_elemental },	/* rok man		*/
  { 10753, generic_warrior },   /* great hero           */
  { 10754, coldcaster },	/* frost salamander	*/
  { 10755, firecaster },	/* flame salamander	*/
  { 10756, fido },		/* purple worm		*/
  { 10759, fido },              /* gelatinous cube      */

  { 10900, temple_labrynth_liar },	/* labyrinth liar	*/
  { 10901, temple_labrynth_liar },	/* labyrinth liar	*/
  { 10902, temple_labrynth_sentry },	/* labyrinth sentry	*/

  /*** python ***/
  { 11001, generic_warrior },	/* lord python 		*/
  { 11007, generic_warrior },	/* weightlifting guard	*/
  { 11016, receptionist },	/* buxom maid		*/

  /*** bounty hunters ***/
  { VMOB_56, bounty_hunter },	/* pkiller hunter	*/
  { VMOB_57, bounty_hunter },	/* pkiller hunter	*/
  { VMOB_58, bounty_hunter },	/* pkiller hunter	*/
  { VMOB_59, bounty_hunter },	/* pkiller hunter	*/
  { VMOB_60, bounty_hunter },	/* pkiller hunter	*/

  /*** graecia ***/
  { 13706, generic_warrior },	/* brave warrior	*/
  { 13711, generic_warrior },	/* king agamemnon	*/
  { 13713, generic_warrior },	/* assassin		*/
  { 13714, generic_warrior },	/* hercules		*/
  { 13719, BreathWeapon },	/* hydra		*/
  { 13722, generic_warrior },	/* hippolyta		*/
  { 13723, generic_warrior },	/* giant		*/
  { 13726, BreathWeapon },	/* watchful dragon	*/

  /*** hades ***/
  { VMOB_129, blocker },        /* cerebus              */
  { 13762, generic_warrior },	/* skeletal warrior	*/
  { 13765, RustMonster },	/* acid monster		*/
  { 13766, old_generic_cleric },/* horrifying priest    */
  { 13769, generic_ranger },	/* evil centaur		*/
  { 13771, githyanki_knight },  /* vlad                 */
  { 13775, generic_psi },       /* master torturer      */
  { 13779, firecaster },	/* pluto		*/
  { 13784, eleccaster },	/* zeus			*/
  { 13787, astral_ranger },     /* hera                 */
  { 13789, firecaster },	/* apollo		*/
  { 13793, generic_warrior },	/* ares			*/
  { 13795, generic_warrior },	/* athena		*/
  { 13797, generic_thief },	/* hermes		*/

  { 13806, generic_mage },	/* witch		*/
  { 13813, generic_warrior },	/* cyclops king		*/
  { 13816, snake },		/* serpent		*/
  { 13846, generic_warrior },	/* pirate leader	*/
  /*** nudge & summoner ***/
  { 15102, NudgeNudge },        /* NudgeNudge guy       */

  /*** skexie and gelfling ***/
  { 15792, generic_warrior },	/* corporal		*/
  { 15805, generic_mage },	/* brackenred		*/
  { 15807, fido },		/* secrant		*/
  { 15813, generic_mage },	/* mantern		*/
  { 15815, old_generic_cleric },/* ancient skexie	*/
  { 15819, generic_warrior },	/* palthor		*/
  { 15820, generic_mage },	/* wegendel		*/
  { 15821, generic_mage },	/* crandel		*/
  { 15822, generic_warrior },	/* alastair		*/
  { 15829, generic_warrior },	/* lair guard		*/
  { 15838, blink },		/* blink dog		*/
  { 15844, old_generic_cleric },/* gelfling cleric	*/
  { 15846, generic_warrior },	/* weaponsmaster	*/
  { 15847, generic_warrior },	/* mad gelfling		*/
  { 15848, generic_warrior },	/* warrior gelfling	*/
  { 15852, generic_warrior },	/* disillea		*/

  /*** eastern path ***/
  { 16001, generic_warrior },	/* mercenary		*/
  { 16006, generic_thief },	/* dangerous man	*/
  { 16007, generic_thief },	/* brigand		*/
  { 16008, generic_thief },	/* bandit		*/
  { 16014, generic_mage },	/* wizard		*/
  { 16016, generic_warrior },	/* knight		*/
  { 16020, snake },		/* rattlesnake		*/
  { 16021, old_generic_cleric },/* elf cleric		*/ /* MIN */
  { 16022, generic_warrior },	/* dwarf warrior	*/
  { 16023, thief },		/* thief		*/
  { 16027, generic_paladin },	/* paladin		*/
  { 16034, generic_warrior },	/* goblin soldier	*/
  { 16035, generic_warrior },	/* goblin chief		*/

  /*** gypsy village ***/
  { 16100, generic_warrior },	/* abdul		*/
  { 16105, StatTeller },	/* palmreader		*/
  { 16106, fido },		/* ugly dog		*/
  { 16107, generic_warrior },	/* warlord		*/
  { 16108, generic_mage },	/* wizard		*/
  { 16109, generic_thief },	/* assassin		*/
  { 16110, generic_paladin },	/* paladin		*/
  { 16111, generic_warrior },	/* warrior master	*/
  { 16112, generic_mage },	/* mage master		*/
  { 16113, generic_thief },	/* thief master		*/
  { 16114, old_generic_cleric },/* cleric master	*/
  { 16116, generic_warrior },	/* weaponsmith		*/
  { 16117, generic_warrior },	/* armorer		*/
  { 16118, generic_mage },	/* alchemist		*/
  { 16122, receptionist },	/* gypsy receptionist	*/
  { 16124, generic_druid },	/* vines		*/

  { 16201, generic_mage },	/* ogre magi		*/
  { 16205, generic_psi },	/* mindflayer		*/
  { 16206, generic_psi },	/* mindflayer		*/
  { 16211, generic_mage },	/* beholder		*/
  { 16214, generic_warrior },	/* drow soldier		*/
  { 16215, generic_warrior },	/* drow warrior		*/
  { 16216, old_generic_cleric },/* drow cleric		*/
  { 16217, generic_warrior },	/* drow leader		*/

  /*** bay isle ***/
  { 16610, generic_mage },	/* hoeur		*/
  { 16620, BreathWeapon },	/* black dragon		*/
  { 16631, generic_warrior },	/* portal chief		*/
  { 16640, old_generic_cleric },/* priest		*/
  { 16650, old_generic_cleric },/* high priest		*/
  { 16660, generic_warrior },	/* minotaur		*/
  { 16670, generic_warrior },	/* elf warrior		*/
  { 16680, generic_warrior },	/* drow warrior		*/

  { 16690, air_elemental },	/* air elemental	*/
  { 16692, earth_elemental },	/* earth elemental	*/
  { 16694, fire_elemental },	/* fire elemental	*/
  { 16696, water_elemental },	/* water elemental	*/

  /*** Ch'tar***/
  { 16704, ghoul },             /* spirit sent          */
  { 16706, generic_warrior },   /* king                 */
  { 16707, lone_troll },        /* troll                */


  /*** lycanthropia ***/
  { 16910, generic_warrior },	/* count boarish	*/

  /*** mordilnia ***/
  { 18200, generic_mage },	/* emilither		*/
  { 18203, generic_warrior },	/* damathar		*/
  { 18204, old_generic_cleric },/* syrathen		*/
  { 18205, receptionist },	/* illyari		*/
  { VMOB_105, blocker },		/* guildguard		*/
  { VMOB_106, blocker },		/* guildguard		*/
  { VMOB_107, blocker },		/* guildguard		*/
  { VMOB_108, blocker },		/* guildguard		*/
  { VMOB_109, blocker },		/* guildguard		*/
  { VMOB_110, blocker },		/* guildguard		*/
  { VMOB_111, blocker },		/* guildguard		*/
  { VMOB_112, blocker },		/* guildguard		*/
  { 18214, generic_warrior },	/* adair		*/
  { 18216, janitor },		/* janitor		*/
  { 18217, fido },		/* rabid dog		*/
  { 18218, generic_warrior },	/* gladiator		*/
  { 18221, generic_warrior },	/* sylira		*/
  { 18222, MordGuard },		/* gateguard		*/
  { 18223, MordGuard },		/* watchman		*/
  { 18224, generic_thief },	/* rogue		*/

  /*** Celestiel interlude ***/
  { 18652, avatar_mallune }, // Avatar Mallune
  //{ 3078, avatar_mallune }, // Avatar Mallune

  { 19206, old_generic_cleric },/* priestess		*/
  { 19208, snake },		/* cobra		*/
  { 19209, generic_thief },	/* bandit		*/
  { 19210, generic_thief },	/* bandit leader	*/

  /*** arachnos ***/
  { 20002, BreathWeapon },	/* yevaud		*/
  { 20003, snake },		/* wolf spider		*/
  { 20006, snake },		/* drone spider		*/
  { 20007, blink },		/* ethereal spider	*/
  { 20009, blink },		/* quasit		*/
  { 20010, snake },		/* bird spider		*/
  { 20011, generic_warrior },	/* hermit		*/
  { 20012, generic_psi },	/* donjonkeeper		*/
  { 20014, generic_mage },	/* arachnos		*/
  { 20015, old_generic_cleric },/* ki rin		*/
  { 20017, BreathWeapon },	/* baby wormkin		*/
  { 20016, BreathWeapon },	/* elder wormkin	*/

  { 20105, ghoul },             /* vampire bat          */
  { 20111, generic_mage },      /* ezmerelda            */
  { 20115, ghoul },             /* spectre              */

  /*** orshingal ***/
  { 21006, generic_mage },	/* black enfan		*/
  { 21016, astral_ranger },	/* master		*/
  { 21017, astral_ranger },	/* bolgard		*/
  { 21019, astral_beholder },	/* akus talb		*/
  { 21020, BreathWeapon },	/* illusion dragon	*/
  { 21021, BreathWeapon },	/* golden dragon	*/

  /*** valley of the mage ***/
  { 21101, snake },		/* grass snake		*/
  { 21110, generic_warrior },	/* trog lieutenant	*/
  { 21111, web_slinger },	/* cave spider		*/
  { 21112, trapper },		/* trapper		*/
  { 21113, generic_warrior },	/* evil troglodyte	*/
  { VMOB_95, troguard },        /* trog guard           */
  { 21117, generic_warrior },	/* trog battlemaster	*/
  { 21121, trogcook },		/* trog cook		*/
  { 21122, shaman },		/* trog shaman		*/
  { 21123, troguard },		/* trog chief		*/
  { 21124, golgar },		/* trog god		*/
  { 21132, eleccaster },	/* eel			*/
  { 21135, generic_warrior },	/* cave troll		*/
  { 21136, snake },		/* cobra		*/
  { VMOB_GhostSoldier, ghostsoldier },	/* ghost soldier	*/
  { VMOB_GhostLieutenant, ghostsoldier },	/* ghost lieutenant	*/
  { 21140, keystone },		/* ghost captain	*/
  { 21141, lattimore },		/* lattimore		*/
  { 21142, generic_warrior },   /* gate guardian        */
  { 21144, troguard },		/* trog lieutenant	*/
  { 21146, coldcaster },	/* frost demon		*/

  /*** deep ones ***/
  { 23050, generic_warrior },	/* father dagon		*/
  { 23051, generic_mage },	/* mother hydra		*/
  { 23054, generic_warrior },	/* cthulhu		*/
  { 23056, old_generic_cleric },/* priest		*/
  { 23057, generic_warrior },   /* constable            */
  { 23058, generic_warrior },   /* guardian             */
  { 23059, old_generic_cleric },/* high priest          */

  /*** volcano ***/
  { 23060, generic_warrior },   /* rock man             */
  { 23061, generic_ranger },    /* zeloch               */
  { 23062, fire_elemental },    /* fire demon           */
  { 23063, generic_mage },      /* lava lagoon          */
  { 23064, firecaster },        /* fire starter         */
  { 23065, alkian_undertaker }, /* brander              */
  { 23066, alkian_fetch },      /* Menju                */
  { 23067, firecaster },        /* fiery sprite         */
  { 23068, astral_beholder },   /* zeloch monk          */
  { 23069, generic_ranger },    /* mountain bandits     */
  { 23070, generic_warrior },   /* clan of wild men     */
  { 23071, BreathWeapon },      /* Old Dragon           */
  { 23072, githyanki_knight },  /* Ettin                */
  { 23073, githyanki_knight },  /* Cave Gnoll           */
  { 23074, generic_alkian },    /* alfire guardian      */
  { 23075, firecaster },        /* fire rat             */

  /*** abyss ***/
  { 25000, astral_beholder },	/* demi lich		*/
  { 25001, generic_warrior },	/* keftab		*/
  { 25002, vampire },           /* crimson death        */
  { 25004, generic_warrior },	/* black slayer		*/
  { 25006, eleccaster },	/* storm giant		*/
  { 25007, snake },		/* white manticore */
  { 25008, firecaster },	/* efreet		*/
  { 25009, BreathWeapon },	/* hydra		*/
  { 25010, water_elemental },	/* kraken */
  { 25011, generic_druid },	/* treant		*/
  { 25013, githyanki_knight },	/* kalas		*/
  { 25014, generic_warrior },	/* death knight		*/
  { VMOB_130, blocker },        /* gate keeper          */

  /*** paladins ***/
  { 25100, rivendell_guard},	/* paladin guardsman	*/
  { 25101, rivendell_guard},	/* grand knight		*/

  { VMOB_113, blocker },		/* guildguard		*/
  { VMOB_114, blocker },		/* guildguard		*/
  { VMOB_115, blocker },		/* guildguard		*/
  { VMOB_116, blocker },		/* guildguard		*/
  { VMOB_117, blocker },		/* guildguard		*/
  { VMOB_118, blocker },		/* guildguard		*/
  { VMOB_119, blocker },		/* guildguard		*/
  { VMOB_120, blocker },		/* guildguard		*/

  /*** fire giants ***/
  { 25501, generic_warrior },   /* lieutenant           */
  { 25502, generic_warrior },	/* general		*/
  { 25503, old_generic_cleric },/* king                 */
  { 25504, BreathWeapon },	/* blue dragon		*/
  { 25505, old_generic_cleric },/* high priest		*/
  { 25506, firecaster },	/* flame salamander	*/
  { 25508, generic_mage },      /* fire giant mage      */

  /*** deep gnomes ***/
  { 25603, generic_warrior },	/* warlord		*/
  { 25604, generic_mage },	/* milaaber		*/
  { 25605, generic_warrior },	/* king			*/
  { 25607, generic_warrior },	/* drow warrior		*/

  /*** sea elfs ***/
  { 29500, generic_warrior },	/* guard		*/
  { 29502, generic_warrior },	/* bodyguard		*/
  { 29503, generic_thief },	/* drow spy		*/
  { 29504, generic_mage },	/* mage			*/
  { 29505, old_generic_cleric },/* priestess		*/
  { 29507, generic_warrior },	/* king			*/
  { 29508, janitor },           /* Maiden Sea Elf       */
  { 29509, generic_ranger },    /* were dolphin         */
  { 29510, fido },              /* Tiger Shark          */
  { 29511, fido },              /* Great White Shark    */
  { 29515, old_generic_cleric },/* saughin cleric       */
  { 29516, generic_warrior },	/* saughin warrior	*/
  { 29517, generic_warrior },   /* undead sailor        */
  { 29518, generic_warrior },   /* cursed captain       */
  { 29519, snake },		/* sea snake		*/
  { 29521, snake },		/* anemone		*/
  { 29523, astral_ranger },     /* general              */
  { 29524, githyanki_knight },  /* elfeater             */
  /*** Ghenna ***/
  { 29700, generic_paladin },   /* shamus               */
  { 29701, ghoul },             /* barrow wright        */
  { 29702, ghoul },             /* banshee              */
  { 29703, generic_warrior },   /* argos                */
  { 29704, generic_warrior },   /* bugbear              */
  { 29705, generic_warrior },   /* broken one           */
  { 29706, astral_ranger },     /* bullywug             */
  { 29707, generic_druid },     /* flytrap              */
  { 29708, nightmare },         /* gorg                 */
  { 29709, githyanki_knight },  /* death knight         */
  { 29710, BreathWeapon },      /* dragon               */
  { 29711, generic_warrior },   /* dwarf                */
  { 29712, generic_mage },      /* elf                  */
  { 29713, astral_beholder },   /* genie                */
  { 29714, generic_psi },       /* medusa               */
  { 29715, generic_warrior },   /* stupid orc           */
  { 29716, astral_ranger },     /* barkeep              */
  { 29717, generic_warrior },   /* citizen              */
  { 29718, old_generic_cleric },/* avon lady            */
  { 29723, githyanki_knight },  /* swordmaster          */
  { 29733, astral_ranger },     /* stone golem          */
  { 29758, generic_warrior },   /* woman                */
  { 29771, generic_mage },      /* hooker               */
  { 29772, generic_warrior },   /* lester               */
  { 29775, generic_warrior },   /* whore                */
  { 29787, generic_mage },      /* old elf              */
  { 29794, fido },              /* cockroach            */
  { 29797, nightmare },         /* deepspawn            */

  /*** Tempus Mobs ***/
  { 30000, astral_ranger },     /* hans                 */
  { 30001, nightmare },         /* one thumb            */
  { 30002, nightmare },         /* vulgar unicorn       */
  { 30003, generic_warrior },   /* lilo                 */
  { 30004, generic_warrior },   /* evol                 */
  { 30006, astral_ranger },     /* smokey               */
  { 30007, nightmare },         /* randal witchy ears   */

  /*** Wizard Wall ***/
  { 30030, astral_ranger },     /* Straton              */
  { 30031, nightmare },         /* Enos Yorl            */
  { 30032, BreathWeapon },      /* lt green dragon      */
  { 30033, BreathWeapon },      /* dk green dragon      */
  { 30034, BreathWeapon },      /* lt green dragon      */
  { 30035, BreathWeapon },      /* dk green dragon      */
  { 30036, generic_warrior },   /* Grey Warhorse        */
  { 30037, snake },             /* beysib scout         */
  { 30038, generic_mage },      /* nymph wood           */
  { 30039, snake },             /* beysib patrol leader */
  { 30040, fido },              /* wolf                 */
  { 30052, magic_user },        /* lamp genie           */
  { 30053, astral_beholder },   /* dragon from figurine */
  { 30110, magic_user },        /* elstar the wizard    */
  { 30112, generic_warrior },   /* mutant thing         */
  { 30200, githyanki_knight },  /* isaac                */
  { 30201, ghoul },             /* ghoul thing          */
  { 30202, ghoul },             /* ghoul corpse         */
  { 30203, old_generic_cleric },/* esmerelda            */
  { 30204, old_generic_cleric },/* the man              */
  { 30205, astral_beholder },   /* dracolich            */
  { 30206, nightmare },         /* erich                */
  { 30207, ghoul },             /* zombie               */
  { 30208, generic_warrior },   /* skeleton             */
  { 30210, generic_thief },     /* shadow               */
  { 30211, firecaster },        /* wight                */
  { 30212, eleccaster },        /* wright               */
  { 30213, generic_psi },       /* banshee              */
  { 30214, old_generic_cleric },/* abbott               */
  { 30300, astral_beholder },   /* copper dragon        */
  { 30301, BreathWeapon },      /* white dragon         */
  { 30302, blink },             /* pixie                */
  { 30303, BreathWeapon },      /* time dragon          */
  { 30304, coldcaster },        /* ice dragon           */
  { 30305, generic_mage },      /* pixie king           */
  { 30306, generic_warrior },   /* gatekeeper           */
  { 30307, nightmare },         /* eternity dragon      */
  { 30308, generic_warrior },   /* pixie guard          */
  { 30309, generic_mage },      /* pixie mage           */

  /*** Hell ***/
  { 30400, githyanki_knight },  /* guradian             */
  { 30401, thief },             /* coins                */
  { 30402, githyanki_mage },    /* dee demon            */
  { 30403, githyanki_mage },    /* dee demon            */
  { 30404, thief },             /* bar                  */
  { 30408, ghoul },             /* damned soul          */
  { 30409, ghoul },             /* tort soul            */
  { 30410, ghoul },             /* half eaten           */
  { 30411, ghoul },             /* wall                 */
  { 30412, generic_warrior },   /* skel warrior         */
  { 30413, ghoul },             /* arms                 */
  { 30414, ghoul },             /* foot                 */
  { 30415, ghoul },             /* nose                 */
  { 30417, generic_warrior },   /* lost soul            */
  { 30420, fido },              /* manes                */
  { 30421, fido },              /* raisen               */
  { 30422, generic_warrior },   /* guard                */
  { 30424, firecaster },        /* demon guard          */
  { 30425, firecaster },        /* burning fool         */
  { 30426, generic_warrior },   /* worker               */
  { 30427, generic_warrior },   /* worker               */
  { 30428, generic_warrior },   /* worker               */
  { 30429, firecaster },        /*fire devil            */
  { 30430, firecaster },        /* wall of fire         */
  { 30431, firecaster },        /* hell hound           */
  { 30433, coldcaster },        /* ice devil            */
  { 30436, generic_warrior },   /* penguin              */
  { 30437, generic_psi },       /* i-scream             */
  { 30438, coldcaster },        /* icescream            */
  { 30439, astral_ranger },     /* demon worker         */
  { 30440, generic_warrior },   /* secretary            */
  { 30441, generic_warrior },   /* office worker        */
  { 30442, generic_warrior },   /* record worker        */
  { 30443, astral_beholder },   /* santa claus          */
  { 30444, old_generic_cleric },/* high demon priest    */
  { 30446, generic_warrior },   /* jury member          */
  { 30447, githyanki_mage },    /* judge whopper        */
  { 30448, generic_warrior },   /* lawyer               */
  { 30449, generic_warrior },   /* complaint            */
  { 30450, githyanki_knight },  /* bastard              */
  { 30451, generic_warrior },   /* maint worker         */
  { 30453, nightmare },         /* stanley              */
  { 30455, BreathWeapon },      /* stom dragon          */
  { 30457, generic_warrior },   /* specialist           */
  { 30458, generic_warrior },   /* worker               */
  { 30459, githyanki_knight },  /* homer                */
  { 30461, generic_warrior },   /* cook                 */
  { 30465, generic_warrior },   /* radiation            */
  { 30466, generic_warrior },   /* chief engineer       */
  { 30467, firecaster },        /* fire demon           */
  { 30469, fido },              /* guard dog            */
  { 30471, nightmare },         /* satan                */


  /*** Ancalador ***/
  { 31001, ghoul },             /* putrid zombie        */
  { 31002, snake },             /* asp                  */
  { 31003, rivendell_gandalf }, /* morticia             */
  { 31004, old_generic_cleric },/* Lazarus              */
  { 31005, generic_warrior },   /* undead beetle        */
  { 31006, githyanki_knight },  /* wraith knight        */
  { 31007, ghoul },             /* crypt keeper         */
  { 31008, generic_druid },     /* hermit               */
  { 31009, fido },              /* green slime          */
  { 31010, firecaster },        /* flame dweller        */
  { 31011, snake },             /* green spider         */
  { 31012, snake },             /* yellow spider        */
  { 31013, generic_warrior },   /* warden               */
  { 31014, generic_mage },      /* umber hulk           */
#if 0
   /* these bastards are annoying and GONE... no more powerleveling */
  { 31015, replicant },         /* ochre jello          */
#endif
  { 31099, nightmare },         /* necromancer          */

  /*** Ultima ***/
  { 31500, replicant },         /* insects              */
  { 31501, shadow },            /* bat                  */
  { 31505, generic_paladin },   /* katrina              */
  { 31506, fido },              /* rat                  */
  { 31507, snake },             /* spider               */
  { 31508, generic_mage },      /* mage                 */
  { 31509, generic_mage },      /* nystul mage          */
  { 31510, generic_mage },      /* mariah mage          */
  { 31511, replicant },         /* slime puddle mass    */
  { 31512, snake },             /* snake                */
  { 31513, generic_warrior },   /* smithy               */
  { 31516, generic_paladin },   /* iolo                 */
  { 31517, generic_warrior },   /* chuckles             */
  { 31519, generic_warrior },   /* gwenno               */
  { 31520, generic_warrior },   /* orc ape              */
  { 31521, lone_troll },        /* troll                */
  { 31522, nightmare },         /* wisp                 */
  { 31523, generic_druid },     /* forest druid         */
  { 31524, generic_druid },     /* city druid           */
  { 31525, githyanki_knight },  /* judge                */
  { 31526, generic_druid },     /* Janna                */
  { 31527, generic_warrior },   /* prisoner             */
  { 31532, generic_warrior },   /* ettin                */
  { 31533, thief },             /* gremlin              */
  { 31534, generic_warrior },   /* headless             */
  { 31535, generic_warrior },   /* fighter              */
  { 31536, githyanki_knight },  /* geoffrey             */
  { 31537, astral_ranger },     /* simon                */
  { 31538, snake },             /* mimic                */
  { 31539, generic_psi },       /* gazer                */
  { 31540, generic_warrior },   /* tinker               */
  { 31541, generic_warrior },   /* julia                */
  { 31542, old_generic_cleric },/* lady empathy         */
  { 31543, earth_elemental },   /* sandtrap             */
  { 31544, BreathWeapon },      /* dragon               */
  { 31545, generic_paladin },   /* paladin              */
  { 31546, generic_paladin },   /* dupre                */
  { 31547, RustMonster },       /* reeper               */
  { 31548, githyanki_knight },  /* gargoyle             */
  { 31549, generic_paladin },   /* good ghost           */
  { 31550, ghoul },             /* bad ghost            */
  { 31551, ghoul },             /* skeleton             */
  { 31552, astral_beholder },   /* daemon               */
  { 31554, astral_ranger },     /* shamino ranger       */
  { 31555, githyanki_mage },    /* timelord             */
  { 31556, nightmare },         /* guardian             */
  { 31557, githyanki_knight },  /* blackthorn           */
  { 31558, astral_beholder },   /* exodus               */
  { 31559, githyanki_mage },    /* minax                */
  { 31560, astral_beholder },   /* mondain              */
  { 31561, generic_paladin },   /* sentri               */
  { 31562, generic_psi },       /* batlin               */
  { 31563, nightmare },         /* avatar               */
  { 31564, astral_beholder },   /* british              */
  { 31565, fido },              /* shark                */
  { 31567, water_elemental },   /* squid                */
  { 31570, generic_warrior },   /* boatseller           */

  /*** Ofcol ***/
  { 31700, cityguard },         /* cityguard            */
  { 31701, githyanki_knight },  /* diana                */
  { 31702, cityguard },         /* derrick              */
  { 31703, generic_paladin },   /* jacklyn              */
  { 31704, generic_warrior },   /* blacksmith           */
  { 31705, generic_thief },     /* sam                  */
  { 31706, generic_psi },       /* tracy                */
  { 31707, generic_warrior },   /* marty                */
  { 31708, generic_druid },     /* farmer               */
  { 31709, generic_mage },      /* granny               */
  { 31710, generic_warrior },   /* jack                 */
  { 31711, old_generic_cleric },/* sara                 */
  { 31712, snake },             /* chicken              */
  { 31713, generic_warrior },   /* cow                  */
  { 31717, generic_warrior },   /* citizen              */
  { 31718, generic_warrior },   /* woman                */
  { 31719, generic_warrior },   /* boy                  */
  { 31720, generic_warrior },   /* girl                 */
  { 31721, generic_warrior },   /* maid                 */
  { 31722, generic_warrior },   /* bartender            */
  { 31723, generic_warrior },   /* cityguard            */
  { 31724, generic_druid },     /* bard                 */
  { 31725, generic_warrior },   /* servent              */
  { 31726, generic_warrior },   /* squire               */
  { 31727, generic_warrior },   /* page                 */
  { 31728, generic_psi },       /* priestess            */
  { 31729, old_generic_cleric },/* jerrold              */
  { 31730, BreathWeapon },      /* dragonlord           */
  { 31731, BreathWeapon },      /* gold dragon          */
  { 31732, old_generic_cleric },/* attendant            */
  { 31733, BreathWeapon },      /* dragonknight         */
  { 31734, cityguard },         /* cityguard            */

  /*** Icecaves ***/
  { 31901, generic_warrior },   /* wildman              */
  { 31902, snake },             /* snake                */
  { 31903, generic_warrior },   /* penguin              */
  { 31904, ghoul },             /* zombie               */
  { 31905, generic_warrior },   /* sloth                */
  { 31906, generic_warrior },   /* snowman              */
  { 31907, coldcaster },        /* iceman               */
  { 31908, coldcaster },        /* iceman               */
  { 31910, githyanki_knight },  /* yeti                 */

  /*** Abbaroth ***/
  { 32000, generic_warrior },   /* patryn               */
  { 32001, generic_warrior },   /* runner               */
  { 32002, generic_warrior },   /* squatter             */
  { 32003, generic_mage },      /* Old patryn           */
  { 32004, old_generic_cleric },/* nexus                */
  { 32005, generic_druid },     /* jungle               */
  { 32006, snake },             /* chaodyn              */
  { 32013, githyanki_mage },    /* wizard               */
  { 32015, firecaster },        /* firedragon           */
  { 32016, old_generic_cleric },/* necro                */
  { 32017, generic_psi },       /* lzar                 */
  { 32018, generic_mage },      /* magician             */
  { 32024, astral_beholder },   /* samah                */
  { 32026, coldcaster },        /* icedevil             */

  /***Narshe***/
  { 32031, poisoncaster },      /* phunbaba              */
  { 32032, BreathWeapon },      /* bahamut               */
  { 32039, generic_ranger},     /* bodyguard             */
  { 32043, generic_ranger},     /* locke cole            */
  { 32044, generic_ranger},     /* Umaro                 */
  { 32045, generic_paladin},    /* Cyan                  */
  { 32046, generic_ranger},     /* Gau                   */
  { 32047, alkian_undertaker},  /* Celes                 */
  { 32048, generic_psi},        /* Gestahl               */
  { 32049, generic_ranger},     /* general Leo           */
  { 32050, generic_ranger},     /* terra                 */
  { 32051, generic_ranger},     /* shadow                */
  { 32052, generic_ranger},     /* sabin                 */
  { 32055, generic_paladin},    /* Cecil                 */
  { 32056, acidcaster},         /* Ultros                */
  { 32057, generic_cleric},     /* big chocobo           */
  { 32058, generic_cleric},     /* gold chocobo          */
  { 32059, generic_ranger},     /* Setzer                */

  /*** Final Fantasy Quest***/
  { 32062, generic_psi },        /* exdeath              */
  { 32108, generic_ranger },     /* Slayer beast         */
  { 32109, generic_ranger },     /* Gimonra beast        */
  { 32110, generic_ranger },     /* stalker ffq          */
  { 32111, firecaster },         /* Tantar ffq           */
  { 32112, githyanki_knight },   /* anti paladin ffq     */
  { 32113, generic_mage },       /* behemoth demon       */
  { 32114, energycaster },       /* chaos lord           */
  { 32115, generic_ranger },     /* mist ffq             */
  { 32116, poisoncaster },       /* cactrot ffq          */
  { 32117, astral_beholder },    /* zone eater           */
  { 32118, generic_psi },        /* imp ffq              */
  { 32119, githyanki_knight },   /* bomb ffq             */
  { 32120, fire_elemental },     /* emperor of hell      */
  { 32121, acidcaster },         /* cloud of darkness    */
  { 32122, coldcaster },         /* zeromus              */
  { 32123, generic_psi },        /* kefka                */
  { 32124, generic_ranger },     /* sephiroth            */

  /*** Alkian Burial Ground **/

  { 32200, blocker },           /* alkian guardian   	 	*/
  { 32201, generic_ranger  },   /* beggar man alkian        	*/
  { 32202, generic_alkian  },   /* guard guardian spawn alkian  */
  { 32203, generic_warrior },  	/* woman alkian weeping	 	*/
  { 32204, generic_alkian  },  	/* alkian spirit wandering	*/
  { 32205, generic_alkian  },  	/* alkian warrior		*/
  { 32206, alkian_undertaker }, /* undertaker alkian		*/
  { 32207, generic_warrior }, 	/* mud slime alkian		*/
  { 32208, generic_ranger  }, 	/* spirit child alkian		*/
  { 32209, magic_user2     }, 	/* spirit mother alkian		*/
  { 32210, alkian_warrior  },  	/* Alkian Master		*/
  { 32211, firecaster      }, 	/* fire imp alkian		*/
  { 32212, coldcaster      }, 	/* icy imp alkian		*/
  { 32213, acidcaster      }, 	/* Acidic imp alkian		*/
  { 32214, eleccaster      }, 	/* Spark imp alkian		*/
  { 32215, energycaster    }, 	/* energetic imp alkian		*/
  { 32216, poisoncaster    }, 	/* Poison imp alkian		*/
  { 32217, generic_cleric  }, 	/* blessed healer alkian	*/
  { 32218, alkian_master   },	/* grand master alkian man 	*/
  { 32219, alkian_master   },	/* grand master alkian woman 	*/
  { 32220, alkian_warrior  }, 	/* alkian magi warrior		*/
  { 32221, generic_ranger  }, 	/* Journeyman alkian		*/
  { 32222, alkian_fetch    }, 	/* alkian berserker		*/
  { 32223, alkian_warrior  }, 	/* soulless alkian		*/
  { 32224, alkian_fetch    }, 	/* alkian fetch priestess	*/
  { 32225, generic_alkian   }, 	/* shade warrior alkian		*/
  { 32226, mindflayer      }, 	/* eye tyrant alkian		*/
  { 32227, alkian_holly    },	/* holly paladin alkian		*/
  { 32228, alkian_fetch    }, 	/* banelich alkian		*/
  { 32229, alkian_demonspawn }, /* the demonspawn         	*/
  { 32230, alkian_warrior  }, 	/* jas alkian		 	*/
  { 32231, alkian_warrior  }, 	/* jedidiah priest alkian	*/
  { 32232, magic_user2    }, 	/* grypht mage alkian	 	*/
  { 32233, generic_alkian  }, 	/* crypt keeper alkian		*/
  { 32234, alkian_master   }, 	/* demonspawn guardian 		*/
  { 32235, alkian_master   },	/* eternal evil			*/
  { 32236, alkian_master   },	/* eternal good			*/
  { 32237, alkian_warrior  },	/* god imp			*/
  { 32238, generic_alkian  },	/* grave robber			*/
  { 32239, magic_user      }, 	/* mage alkian			*/
  { 32240, generic_cleric  }, 	/* priest alkia        		*/
  { 32241, alkian_master   }, 	/* forger swords    		*/




  /*** yggdrasil ***/
  { 32300, generic_paladin },   /* holyphant            */
  { 32301, generic_psi },       /* plantar              */
  { 32302, generic_warrior },   /* shedu                */
  { 32303, generic_warrior },   /* dretch               */
  { 32304, old_generic_cleric },/* chasme               */
  { 32305, fido },              /* babua                */
  { 32306, githyanki_knight },  /* yagnodemon           */
  { 32307, nightmare },         /* kost                 */
  { 32308, ghoul },             /* nabassu              */
  { 32309, BreathWeapon },      /* cloud dragon         */
  { 32310, githyanki_mage },    /* solar                */
  { 32311, generic_druid },     /* wemic                */
  { 32312, coldcaster },        /* frost giant          */
  { 32313, BreathWeapon },      /* gorg                 */
  { 32314, old_generic_cleric },/* rata                 */


  /*** clan of the hawk ***/
  { 32415, generic_thief },     /* thaef                */
  { 32416, generic_thief },     /* haax                 */
  { 32417, generic_warrior },   /* yojimbo              */
  { 32418, generic_thief },     /* lizst                */
  { 32419, generic_thief },     /* saaleen              */
  { 32420, generic_thief },     /* max                  */
  { 32421, generic_thief },     /* ajas                 */
  { 32422, generic_thief },     /* jola                 */
  { 32423, generic_warrior },   /* pagar                */
  { 32424, generic_thief },     /* mercenary            */
  { 32425, generic_thief },     /* hired assassin       */
  { 32426, generic_warrior },   /* payed thug           */
  { 32427, generic_thief },     /* drow backstabber     */
  { 32428, generic_thief },     /* pick pocket          */
  { 32429, generic_thief },     /* thief pupil          */
  { 32430, generic_warrior },   /* lookout guard        */
  { 32432, generic_alkian },    /* sly                  */

  /*** Scotland ***/
  { 32433, alkian_warrior },    /* scottish warrior     */
  { 32434, alkian_warrior },    /* drunkard filthy      */
  { 32435, generic_warrior },   /* scottish youth       */
  { 32436, alkian_demonspawn }, /* victor bruce         */
  { 32437, alkian_fetch },      /* tree lopper          */
  { 32438, alkian_warrior },    /* forest bandit        */
  { 32439, alkian_warrior },    /* rabid wolf           */
  { 32440, alkian_master },     /* thedra muse          */
  { 32441, generic_ranger },    /* wood nymph           */
  { 32442, alkian_master },     /* lake monster         */
  { 32443, alkian_warrior },    /* mud fiend            */
  { 32444, alkian_holly },      /* english knight       */
  { 32445, alkian_demonspawn }, /* thrathgar noble      */
  { 32446, alkian_demonspawn }, /* cormac noble         */
  { 32447, alkian_master },     /* duncan noble         */
  { 32448, alkian_fetch },      /* scottish lune        */
  { 32449, alkian_warrior },    /* ferocious hog        */
  { 32450, alkian_warrior },    /* towering treant      */
  { 32451, alkian_master },     /* noble guardian       */
  { 32452, alkian_undertaker }, /* scottish undertaker  */
  { 32453, alkian_warrior },    /* dead scottsman       */
  { 32454, coldcaster },        /* drifting spirit      */
  { 32455, alkian_warrior },    /* petrified corpse     */

  /*** The Deep ***/
  { 32575, deep_warrior },      /* orca boss            */
  { 32574, deep_warrior },      /* giant squid          */
  { 32571, generic_ranger },    /* scuba diver          */
  { 32572, generic_alkian },    /* great white          */
  { 32573, generic_psi },       /* croc                 */
  { 32576, old_generic_cleric },/* ghost of ahab        */
  { 32577, generic_ranger  },   /* puffer fish          */
  { 32578, alkian_fetch  },     /* sea urchine          */
  { 32579, generic_warrior  },  /* king crab            */
  { 32580, generic_thief  },    /* red lobster          */
  { 32581, generic_alkian  },   /* squid tentacle       */

  { VMOB_126, shifter_guildmasters },	/* shifter guildmaster	*/
  { VMOB_127, shifter_guildmasters },	/* shifter guildmaster	*/
  { VMOB_128, shifter_guildmasters },	/* shifter guildmaster	*/

  { VMOB_GATE_MOB, gate_proc },		/* dimensional gate	*/

  { VMOB_68, liquid_proc },	/* liquid state		*/
  { VMOB_69, cocoon_proc },	/* cocoon regen state	*/

  { 0, NULL }
};












void assign_mobiles(void)
{
    int	rnum;
    char buf[MAX_STRING_LENGTH];
    struct special_proc_entry *spec;

    for(spec = mob_specials ; spec->vnum ; spec++)
    {
	rnum = real_mobile(spec->vnum);
	if (rnum<0) {
	    sprintf(buf, "mob special: Mobile %d doesn't seem to exist.",
		    spec->vnum);
	    log_msg(buf);
	} else {
	    mob_index[rnum].func = spec->proc;
	}
    }

    check_allocs();
    boot_the_shops();

    check_allocs();
    assign_the_shopkeepers();
}

/* assign special procedures to objects */
static struct special_proc_entry obj_specials[] =
{
  { 3092, board },
  { 3093, board },
  { 3094, board },
  { 3095, board },
  { 3096, board },
  { 3097, board },
  { 3098, board },
  { 3099, board },
  { 3225, board },
  { 3226, board },
  { 3227, board },
  { 3228, board },
  { 3229, board },
  { 3230, board },
  { 3231, board },
  { 3232, board },
  { 3233, board },
  { 3234, board },
  { 3235, board },
  { 3236, board },
  { 3237, board },
  { 25102,board },

  { 552,  astral_berry },

  /*** pancea palace ***/
  { 382, portal },              /* Talips Portals        */
  { 383, portal },		/* Newbie Portal	 */
  { 3206, obj_hero_ring },	/* hero ring		 */

  { 30041, mirror },            /* mirror in Enos        */
  { 30042, stones },            /* stone pool            */
  { 30052, genie_lamp },	/* genie's lamp          */
  { 30431, genie_lamp },        /* horse figurine        */
  { 30432, genie_lamp },        /* flying horse          */
  { 30433, genie_lamp },        /* nightmare             */
  { 30434, genie_lamp },        /* dragon figurine       */
  { 30435, genie_lamp },        /* red dragon            */
  { 30436, genie_lamp },        /* white dragon          */
  { 30437, genie_lamp },        /* green dragon          */
  { 30438, genie_lamp },        /* blue dragon           */

  { 0, NULL }
};

void assign_objects(void)
{
    int	rnum;
    char buf[MAX_STRING_LENGTH];
    struct special_proc_entry *spec;

    for(spec = obj_specials ; spec->vnum ; spec++)
    {
	rnum = real_object(spec->vnum);
	if (rnum<0) {
	    sprintf(buf, "obj special: Object %d doesn't seem to exist.",
		    spec->vnum);
	    log_msg(buf);
	} else {
	    obj_index[rnum].func = spec->proc;
	}
    }

    InitBoards();
}

/* assign special procedures to rooms */
static struct special_proc_entry room_specials[] =
{
    { 363, storeroom },
    { 2625,  Donation},

    { 2633,  pawnshop },
    { 1, dump },               /* limbo is now a dump room */
    { 3,     dump },		/* fix the kludge */
    { 3030,  dump },
    { 13547, dump },

    { 3054,  pray_for_items },

    { 453,   Fountain},
    { 1514,  Fountain},
    { 1819,  Fountain},
    { 3005,  Fountain},
    { 3141,  Fountain},
    { 3606,  Fountain},
    { 5234,  Fountain},
    { 8071,  Fountain},
    { 10204, Fountain},
    { 10315, Fountain},
    { 10400, Fountain},
    { 10500, Fountain},
    { 11014, Fountain},
    { 13518, Fountain},
    { 35036, Fountain},

    { 13530, pet_shops },
    { 2631,  pet_shops },
    { 3031, morgue_shops },


    { 2638,  bank },
    { 13521, bank },
    { 2637, guild_bank },

    { 3195, clan_restring_shop },
    { 10421, clan_restring_shop },
    { 10422, clan_donation_shop },

    { 10517, clan_restring_shop },

    { 10620, clan_altar },
    { 10621, clan_restring_shop },
    { 10622, clan_donation_shop },

    { 9, clan_forger },
    { 10656, clan_restring_shop },
    { 10657, clan_altar },

/*    { 71,   House },  */	/* Vhasnet */
/*    { 80,   House },	*/	/* Molly */
/*    { 76,   House },	*/	/* Grunt */
/*    { 78,   House },	*/	/* Rohan */

    { 0, NULL},
};

void assign_rooms(void)
{
    char buf[150];
    struct room_data *rp;
    struct special_proc_entry *spec;

    for(spec = room_specials ; spec->vnum ; spec++)
    {
	rp = real_roomp(spec->vnum);
	if (rp==NULL) {
	    sprintf(buf,"room special: can't find room #%d", spec->vnum);
	    log_msg(buf);
	} else
	    rp->funct = spec->proc;
    }
}
