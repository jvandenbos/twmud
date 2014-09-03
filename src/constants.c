#include "config.h"
#include "constants.h"

#ifndef FLEDIT
#include "structs.h"
#include "limits.h"
#include "trap.h"
#endif

#ifndef FLEDIT
const char *spell_wear_off_msg[MAX_SKILLS] =
{
  "RESERVED DB.C",
  "Your armor spell fades.",
  "!Teleport!",
  "Your bless spell fades .",
  "You are no longer blind.",
  "!Burning Hands!",
  "!Call Lightning!",
  "You feel more independent.",
  "!Chill Touch!",
  "!Clone!",
  "!Chain Electrocution!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "The curse has been removed.",
  "!Fire Wind!",
  "Your detect invisibility fades.",
  "!Frost Cloud!",
  "!Detect Poison!",
  "!Harmful Touch!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Rupture!",
  "!Heal",
  "Your invisibility wears off and you feel exposed.",
  "!Implode!",
  "!Locate object!",
  "!Acid Rain!",
  "You recover from the poisoning.",
  "!Poison Gas!",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel fully rested after your involuntary sleep.",
  "Your strength seems to wain.",
  "!Summon!",
  "!Electrocute!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "You feel less sneaky now.",
  "You don't feel well hidden anymore.",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!Identify!",
  "Your ability to see in the dark wears off.",
  "!empty spell!",
  "The regenerative force surrounding you fades away.",
  "!flamestrike!",
  "!Electric Fire!",
  "You feel stronger now.",
  "!Dispel Magic!",
  "!Knock!",
  "Your perceptiveness of other beings fades.",
  "!animate dead!",
  "You feel freedom of movement.",
  "!remove paralysis!",
  "!fear!",
  "!acid blast!",
  "You feel a tightness at your throat.",
  "You feel heavier as your flying ability leaves you.",
  "!Decay!",
  "!Wind Storm!",
  "!Ice Storm!",
  "Your shield of protection flickers, and then disappears.",
  "!Vampyric Touch!",
  "!Empathic Heal!",
  "!Monsum!",
  "!Heat Metal!",
  "!Rot!",
  "Your skin loses its petrified properties.",
  "You no longer blend in with the surrounding foliage.",
  "The red glow around your body fades.",
  "!empty spell!",
  "!Cure Serious!",
  "Your fire ward wears off.",
  "!Refresh!",
  "Your cold ward wears off.",
  "!Turn!",
  "!Succor!",
  "!Create Light!",
  "!Empty Spell!",
  "!Calm!",
  "Your skin returns to normal.",
  "!Conjure Elemental!",
  "You lose your clarity of vision.",
  "!Minor Creation!",
  "You don't feel so vulnerable anymore.",
  "!Faerie Fog!",
  "!Possession!" ,
  "!Polymorph!" ,
  "!Mana!",
  "!Astral Walk!",
  "!Resurrection!",
  "!Heroes Feast!",
  "!Group Fly!",
  "Your electric ward wears off.",
  "The sticky web around your feet dissolves into an ooze on the ground.",
  "You lose your minor tracking ability.",
  "Your major tracking ability fades away.",
  "!Disintegrate!",
  "!Lava Storm!",
  "!Knowledge!",
  "!Group Armor!",
  "!Group Detect Invis!",
  "!Group Invis!",
  "!Group Cure Light!",
  "!Golem!",
  "!Mount!",
  "!Group Waterbreath!",
  "!Group Recall!",
  "!Sunray!",
  "!Windwalk!",
  "!Moonbeam!",
  "!Goodberry!",
  "!Tree!",
  "!Ansum!",
  "!Thorn!",
  "!Creeping Doom!",
  "!Vine!",
  "!Nature Walk!",
  "spell 130 please report.",
  "Your meditative state wears off.",
  "spell 132 please report.",
  "Your adrenalin rush fades away.",
  "spell 134 please report.",
  "spell 135 please report.",
  "You fade back into visibility.",
  "spell 137 please report.",
  "spell 138 please report.",
  "Your shroud of illusion fades away.",
  "spell 140 please report.",
  "Your energy ward wears off.",
  "Your great sight abandons you.",
  "Your spell shield wears off.",
  "spell 144 please report.",
  "spell 145 please report.",
  "spell 146 please report.",
  "spell 147 please report.",
  "spell 148 please report.",
  "spell 149 please report.",
  "spell 150 please report.",
  "spell 151 please report.",
  "spell 152 please report.",
  "spell 153 please report.",
  "Your stunned state wears off.",
  "spell 155 please report.",
  "spell 156 please report.",
  "spell 157 please report.",
  "You settle back down to the ground.",
  "spell 159 please report.",
  "spell 160 please report.",
  "spell 161 please report.",
  "spell 162 please report.",
  "Your acid ward wears off.",
  "Your fists return to normal.",
  "Your arms and hands melt and shift, returning to normal.",
  "You become visible as your skin returns to normal.",
  "spell 167 please report.",
  "spell 168 please report.",
  "You return to your original form.",
  "spell 170 please report.",
  "spell 171 please report.",
  "Your armor plating returns to normal skin.",
  "spell 173 please report.",
  "spell 174 please report.",
  "spell 175 please report.",
  "spell 176 please report.",
  "spell 177 please report.",
  "spell 178 please report.",
  "spell 179 please report.",
  "spell 180 please report.",
  "spell 181 please report.",
  "spell 182 please report.",
  "spell 183 please report.",
  "spell 184 please report.",
  "spell 185 please report.",
  "spell 186 please report.",
  "spell 187 please report.",
  "spell 188 please report.",
  "!Quick Draw!",
  "!Trip!",
  "!Circle!",
  "!Search!",
  "!Melee1!",
  "!Melee2!",
  "!Melee3!",
  "The continual light wears off?",
  "The continual darkness wears off?",
  "!Melee4!",
  "!Group Astral!",
  "!Ray of Purification!",
  "Your wings melt back into your body.",
  "Your gills dissolve.",
  "!Trace!",
  "Your need to sleep dissapates",
  "You feel as if you could speak again",
  "You no longer feel afraid",
  "You drift back down to the ground",
  "You stop blurring.",
  "You feel more independent.",
  "spell 210 please report.",
  "You feel less motivated.",
  "Your body returns to its normal speed.",
  "Your despair falls away.",
  "214",
  "215",
  "216",
  "217",
  "218",
  "219",
  "220",
  "221",
  "Your electric shield fizzles away",
  "The air around you is warmer",
  "Your shield of poison faded away",
  "Your shield of energy dissipates",
  "Your shield of vampires dissappears",
  "The collection of mana around you disperses",
  "228",
  "229",
  "230",
  "Your shield of acid subsides",
  "232",
  "233",
  "234",
  "235",
  "236",
  "237",
  "238",
  "239",
  "240",
  "241",
  "242",
  "243",
  "244",
  "245",
  "246",
  "247",
  "248",
  "249",
  "250",
  "251",
  "252",
  "253",
  "254",
  "255",
  "256",
  "257",
  "258",
  "259",
  "260",
  "261",
  "262",
  "263",
  "264",
  "265",
  "266",
  "267",
  "268",
  "269",
  "270",
  "271",
  "272",
  "273",
  "274",
  "275",
  "276",
  "277",
  "278",
  "279",
  "280",
  "281",
  "282",
  "283",
  "284",
  "285",
  "286",
  "287",
  "288",
  "289",
  "290",
  "291",
  "292",
  "293",
  "294",
  "295",
  "296",
  "297",
  "298",
  "299",
  "300",
  "301",
  "302",
  "303",
  "304",
  "305",
  "306",
  "307",
  "308",
  "309",
  "310",
  "311",
  "312",
  "313",
  "314",
  "315",
  "316",
  "317",
  "318",
  "319",
  "320",
  "321",
  "322",
  "323",
  "324",
  "!FLAIL!",
  "!DIVERT!",
  "!GOUGE!",
  "BLIND_FIGHTING!",
  "Your psycis shield of tolerance cracks and breaks away!",
  "The dark clouds seem to have moved on! Feeling a bit better?",
  "331",
  "332",
  "333",
  "334",
  "335",
  "You no longer shimmer!",
  "337",
  "Your aura fades!",
  "Your firey aura fades!",
  "Your icey aura fades!",
  "Your aura fades!",
  "342",
  "343",
  "344",
  "345",
  "346",
  "347",
  "348",
  "349",
  "350",
  "351",
  "352",
  "353",
  "354",
  "355",
  "356",
  "Your aura fades!",
  "Your aura fades!",
  "Your aura fades!",
  "Your aura fades!",
  "The fire within you extinguishes, and you a left feeling abit cold",
  "The nasty odor vanishes, maybe a shower wouldn't be in the way right now",
  "You return to your own pink color",
  "Your coughing starts to dim and the acid vaporize in your mouth",
  "You are no longer a living powerplant",
  "\n"
};


const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};

const int TrapDir[] =
{
  TRAP_EFF_NORTH,
  TRAP_EFF_EAST,
  TRAP_EFF_SOUTH,
  TRAP_EFF_WEST,
  TRAP_EFF_UP,
  TRAP_EFF_DOWN
};

const int movement_loss[]=
{
  1,				/* Inside     */
  2,				/* City       */
  2,				/* Field      */
  3,				/* Forest     */
  4,				/* Hills      */
  6,				/* Mountains  */
  8,				/* Swimming   */
  10,				/* Unswimable */
  2,				/* Flying     */
  20,				/* Submarine  */
  4,				/* Desert     */
  2,				/* Sky	      */
};

const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const char *dir_desc[] =
{
  "to the north",
  "to the east",
  "to the south",
  "to the west",
  "upwards",
  "downwards"
};

const char *dir_from[] =
{
  "from the south",
  "from the west",
  "from the north",
  "from the east",
  "from below",
  "from above"
};

const char *ItemDamType[] =
{
  "burned",
  "frozen",
  "electrified",
  "crushed",
  "corroded"
};

const char *weekdays[7] =
{
  "Ilsday",
  "Orulsday",
  "Thurfirday",
  "Eshisday",
  "Falanday",
  "Anensday",
  "Shiprisday" };

const char *month_name[12] =
{
  "Breen",		/* 0 */
  "Andaria",
  "Moruthus",
  "Drellan",
  "Eseris",
  "Sperraz",
  "Hespar",
  "Languel",
  "Produr",
  "Gethur",
  "Escharia",
  "Volanr",
};

const int sharp[] =
{
  0, /* Bludgeon */
  0,
  0,
  0,				
  0,
  0, /* Pierce */
  0,
  0,		
  0,
  1, /* Slashing */
  1,
  0  /* Ranged Weapon */
};

const char *where[] =
{
  "<used as light>    ",
  "<worn on finger>   ",
  "<worn on finger>   ",
  "<worn around neck> ",
  "<worn around neck> ",
  "<worn on body>     ",
  "<worn on head>     ",
  "<worn on legs>     ",
  "<worn on feet>     ",
  "<worn on hands>    ",
  "<worn on arms>     ",
  "<worn as shield>   ",
  "<worn about body>  ",
  "<worn about waist> ",
  "<worn on wrist>    ",
  "<worn on wrist>    ",
  "<wielded>          ",
  "<held>             ",
  "<loaded>           "
};
#endif

const char *drinks[]=
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "coca cola",
  "\n"
};

#ifndef FLEDIT
const char *drinknames[]=
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "cola",
  "\n"
};

/*  fire cold elec blow acid */
const int ItemSaveThrows[26][5] =
{
  {3,   1,  3,  3,  6 }, /* light */
  {10,  1,  5,  1,  10}, /* scroll */
  {5,   2,  5,  2,  10}, /* wand */
  {5,   1,  3,  1,  10}, /* staff */
  {5,   1,  3,  1,  10}, /* weapon */
  {5,   1,  3,  1,  10}, /* fire weapon */
  {5,   2,  5,  2,  10}, /* missile */
  {3,   1,  3,  1,  6 }, /* treasure */
  {3,   1,  1,  1,  6 }, /* armor */
  {6,   6,  6,  6,  6 }, /* potion */
  {10,  1,  5,  1,  10}, /* worn */
  {5,   5,  5,  5,  5 }, /* other */
  {5,   5,  5,  5,  5 }, /* trash */
  {5,   2,  5,  2,  5 }, /* trap */
  {5,   1,  3,  1,  5 }, /* container */
  {10,  1,  5,  1,  10}, /* note */
  {10,  1,  5,  1,  10}, /* drink container */
  {1,   1,  1,  1,  1 }, /* key */
  {3,   1,  3,  3,  6 }, /* food */
  {1,   1,  1,  1,  1 }, /* money */
  {5,   2,  5,  2,  10}, /* pen */
  {3,   1,  3,  3,  6 }, /* boat */
  {1,   1,  1,  1,  1 }, /* audio */
  {1,   1,  1,  1,  1 }, /* board */
  {3,   1,  3,  3,  6 }, /* spellbook */
  {3,   1,  3,  3,  6 }  /* air */
};


const int drink_aff[][3] =
{
  { 0,1,10 },			/* Water    */
  { 3,2,5 },			/* beer     */
  { 5,2,5 },			/* wine     */
  { 2,2,5 },			/* ale      */
  { 1,2,5 },			/* ale      */
  { 6,1,4 },			/* Whiskey  */
  { 0,1,8 },			/* lemonade */
  { 10,0,0 },			/* firebr   */
  { 3,3,3 },			/* local    */
  { 0,4,-8 },			/* juice    */
  { 0,3,6 },
  { 0,1,6 },
  { 0,1,6 },
  { 0,2,-1 },
  { 0,1,-2 },
  { 0,1,5 },
  { 0, 0, 0}
};

const char *color_liquid[]=
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "black",
  "\n"
};

const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};
#endif

const char *item_types[] =
{
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRING WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQUID CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "AUDIO",
  "BOARD",
  "SPELLBOOK",
  "AIR",
  "\n"
};

const char *wear_bits[] =
{
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "THROW",
  "LIGHT-SOURCE",
  "\n"
};

const char *extra_bits[] =
{
  "GLOW",
  "HUM",
  "METAL",
  "MINERAL",
  "ORGANIC",
  "INVISIBLE",
  "MAGIC",
  "NODROP",
  "BLESS",
  "ANTI-GOOD",
  "ANTI-EVIL",
  "ANTI-NEUTRAL",
  "ANTI-CLERIC",
  "ANTI-MAGE",
  "ANTI-THIEF",
  "ANTI-WARRIOR",
  "BRITTLE",
  "ANTI-PALADIN",
  "ANTI-DRUID",
  "ANTI-PSIONIST",
  "ANTI-RANGER",
  "UNUSED0",
  "UNUSED1",
  "UNUSED2",
  "NO-LOCATE",
  "RARE",
  "ANTI-BARD",
  "ANTI-MONK",
  "PURE-CLASS-ERROR",
  "TWO-HANDED",
  "ANTI-SHIFTER",
  "HARDENED",
  "\n"
};

/* ADDED BY MIN for PURE CLASSES */

const char *extra_bits_pure[] =
{
  "GLOW",
  "HUM",
  "METAL",
  "MINERAL",
  "ORGANIC",
  "INVISIBLE",
  "MAGIC",
  "NODROP",
  "BLESS",
  "ANTI-GOOD",
  "ANTI-EVIL",
  "ANTI-NEUTRAL",
  "PURE-CLERIC",
  "PURE-MAGE",
  "PURE-THIEF",
  "PURE-WARRIOR",
  "BRITTLE",
  "PURE-PALADIN",
  "PURE-DRUID",
  "PURE-PSIONIST",
  "PURE-RANGER",
  "UNUSED0",
  "UNUSED1",
  "UNUSED2",
  "UNUSED3",
  "RARE",
  "PURE-BARD",
  "PURE-MONK",
  "",
  "TWO-HANDED",
  "ANTI-SHIFTER",
  "UNUSED4",
  "\n"
};

/* END OF PURE CLASS EXTRA BITS TABLE */

const char *room_bits[] =
{
  "DARK",
  "DEATH",
  "NO_MOB",
  "INDOORS",
  "PEACEFUL",
  "NOSTEAL",
  "NO_TRAVEL_OUT",
  "NO_MAGIC",
  "TUNNEL",
  "NO_TRAVEL_IN",
  "SILENCE",
  "NO_PUSH",
  "IMMORT_RM",
  "GOD_RM",
  "NO_RECALL",
  "ARENA",
  "NO_SNEAK",
  "TEMPLE",
  "BRUJAH_RM",
  "\n"
};

const char *exit_bits[] =
{
  "IS-DOOR",
  "CLOSED",
  "LOCKED",
  "SECRET",
  "RSLOCKED",
  "PICKPROOF",
  "\n"
};

const char *sector_types[] =
{
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water Swim",
  "Water NoSwim",
  "Air",
  "Underwater",
  "Desert",
  "Sky",
  "\n"
};

#ifndef FLEDIT
const char *equipment_types[] =
{
  "Special",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "\n"
};
#endif

const char *affected_bits[] =
{
  "BLIND",
  "INVISIBLE",
  "REGENERATION",
  "DETECT-INVISIBLE",
  "SENSE-AURA",
  "SENSE-LIFE",
  "LIFE-PROTECTION",
  "SANCTUARY",
  "GROUP",
  "BERSERK",
  "CURSE",
  "FLYING",
  "POISON",
  "ILLUSION",
  "PARALYSIS",
  "INFRAVISION",
  "WATER-BREATH",
  "SLEEP",
  "DODGE",
  "SNEAK",
  "HIDE",
  "FEAR",
  "CHARM",
  "FOLLOW",
  "UNDEF1",
  "TRUESIGHT",
  "SCRYING",
  "FIRESHIELD",
  "CONTINUAL-DARK",
  "MEDITATE",
  "GREAT-SIGHT",
  "CONTINUAL-LIGHT",
  "\n"
};

const char *affected2_bits[] =
{
  "HASTE",
  "SLOW",
  "DESPAIR",
  "TOLERANCE",
  "RAGE",
  "ROUGHNESS",
  "RESISTANCE",
  "ELECSHIELD",
  "POISONSHIELD",
  "ENERGYSHIELD",
  "VAMPSHIELD",
  "MANASHIELD",
  "ACIDSHIELD",
  "COLDSHIELD",
  "MINDPROTECT",
  "ABSORB",
  "ROUGH",
  "MOVESHIELD",
  "SUMMON",
  "FLIGHT",
  "FIRE-BREATH",
  "FROST-BREATH",
  "ACID-BREATH",
  "POISONGAS-BREATH",
  "LIGHTNING-BREATH",
  "\n"
};

const char *immunity_names[] =
{
  "FIRE",
  "COLD",
  "ELECTRICITY",
  "ENERGY",
  "BLUNT",
  "PIERCE",
  "SLASH",
  "ACID",
  "POISON",
  "DRAIN",
  "SLEEP",
  "CHARM",
  "HOLD",
  "NON-MAGIC",
  "+1",
  "+2",
  "+3",
  "+4",
  "BARD",
  "\n"
};

const char *apply_types[] =
{
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "SEX",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MANA",
  "HIT",
  "MOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "SAVING_ALL",
  "RESISTANCE",
  "SUSCEPTIBILITY",
  "IMMUNITY",
  "SPELL AFFECT",
  "WEAPON SPELL",
  "EAT SPELL",
  "BACKSTAB",
  "KICK",
  "SNEAK",
  "HIDE",
  "BASH",
  "PICK",
  "STEAL",
  "TRACK",
  "HIT-N-DAM",
  "NUM-DICE",
  "SIZE-DICE",
  "BARE-HAND-DAM",
  "APPLY44",
  "APPLY45",
  "APPLY46",
  "RIDE",
  "SPELL AFFECT2",
  "AFFECT BY",
  "CHARISMA",
  "SPELLBOOK",
  "HP DRAIN",
  "\n"
};

#ifndef FLEDIT
const char *pc_class_types[] =
{
  "Mage",
  "Cleric",
  "Warrior",
  "Thief",
  "Knight",
  "Druid",
  "Psionist",
  "Ranger",
  "Shifter",
  "Monk",            /* The elusive to non-existant class */
  "Bard",
  "\n"
};
#endif

const char *action_bits[] =
{
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "NICE-THIEF",
  "AGGRESSIVE",
  "STAY-ZONE",
  "WIMPY",
  "ANNOYING",
  "HATEFUL",
  "AFRAID",
  "IMMORTAL",
  "ROAMING",
  "DEADLY",
  "POLYMORPHED",
  "META_AGG",
  "GUARDING",
  "IT",
  "P-ATTACK",
  "LIQUID",
  "SHIFTER",
  "STEED",
  "HUGE",
  "NO_TRACK",
  "\n"
};

#ifndef FLEDIT
const char *player_bits[] =
{
  "BRIEF",
  "CONTINUOUS",
  "COMPACT",
  "P-KILLER",
  "LOSER",
  "NOHASSLE",
  "STEALTH",
  "WIMPY",
  "THIEF",
  "ECHO",
  "DISPLAY",
  "AGGR",
  "COLOR",
  "MASK",
  "NOSHOUT",
  "BUILDER",
  "NOTELL",
  "AUTOEXIT",
  "AFK",
  "UNIMP",
  "SITEOK",
  "AUTOLOOT",
  "AUTOSPLIT",
  "AUTOGOLD",
  "NOWIZ",
  "DENIED",
  "SHOWDAM",
  "BRUJAH",
  "SUMMON",
  "AUTOASSIST",
  "\n"
};
#endif

const char *position_types[] =
{
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};

#ifndef FLEDIT
const char *connected_types[]	=
{
  "Playing",
  "Get name",
  "Confirm name",
  "Read Password",
  "Get new password",
  "Confirm new password",
  "Get sex",
  "Read messages of today",
  "Read Menu",
  "Get extra description",
  "Get class",
  "\n"
};

const int thaco[11][ABS_MAX_LVL] =  /* [class][level] */
{   /* mage */
  { 100,35,35,35,35,35,34,34,34,34,34,33,33,33,33,33,32,32,32,32,32,
    31,31,31,31,31,30,30,30,30,30,29,29,29,29,29,28,28,28,28,28,
    27,27,27,27,27,26,26,26,26,26,25,25,25,25,25,24,24,24,24,24,
    23,23,23,23,23,22,22,22,22},
  /* cleric */
  { 100,35,35,35,35,34,34,34,34,33,33,33,33,32,32,32,32,31,31,31,31,
    30,30,30,30,29,29,29,29,28,28,28,28,27,27,27,27,26,26,26,26,
    25,25,25,25,24,24,24,24,23,23,23,23,22,22,22,22,21,21,21,21,
    20,20,20,20,19,19,19,19,18},
  /* warrior */
  { 100,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
    15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* thief */
  { 100,35,35,35,33,33,33,32,32,32,31,31,31,30,30,30,29,29,29,28,28,
    28,27,27,27,26,26,26,25,25,25,24,24,24,23,23,23,22,22,22,21,
    21,21,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,
    14,14,14,13,13,13,12,12,12},
  /* paladin */
  { 100,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
    15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1},
  /* druid */
  { 100,35,35,35,35,34,34,34,34,33,33,33,33,32,32,32,32,31,31,31,31,
    30,30,30,30,29,29,29,29,28,28,28,28,27,27,27,27,26,26,26,26,
    25,25,25,25,24,24,24,24,23,23,23,23,22,22,22,22,21,21,21,21,
    20,20,20,20,19,19,19,19,18},
  /* psionist */
  { 100,35,35,35,35,35,34,34,34,34,34,33,33,33,33,33,32,32,32,32,32,
    31,31,31,31,31,30,30,30,30,30,29,29,29,29,29,28,28,28,28,28,
    27,27,27,27,27,26,26,26,26,26,25,25,25,25,25,24,24,24,24,24,
    23,23,23,23,23,22,22,22,22},
  /* ranger */
  { 100,35,35,34,34,33,33,32,32,31,31,30,30,29,29,28,28,27,27,26,26,
    25,25,24,24,23,23,22,22,21,21,20,20,19,19,18,18,17,17,16,16,
    14,14,13,13,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5 ,5,
    4, 4, 3, 3, 2, 2, 1, 1, 1},
  /* shifter */
  { 100,35,35,35,33,33,33,32,32,32,31,31,31,30,30,30,29,29,29,28,28,
    28,27,27,27,26,26,26,25,25,25,24,24,24,23,23,23,22,22,22,21,
    21,21,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,
    14,14,14,13,13,13,12,12,12},
  /*Monk*/
  { 100,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
    15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1},
  /*Bard*/
  { 100,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
    15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1}
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] =
{
  {-5, -4,   0,  0 },        /* 0  */
  {-5, -4,  10,  1 },        /* 1  */
  {-3, -2,  20,  2 },
  {-3, -2,  30,  3 },        /* 3  */
  {-2, -1,  40,  4 },
  {-2, -1,  50,  5 },        /* 5  */
  {-1,  0,  60,  6 },
  {-1,  0,  70,  7 },
  { 0,  0,  80,  8 },
  { 0,  0,  90,  9 },
  { 0,  0, 100, 10 },        /* 10 */
  { 0,  0, 110, 11 },
  { 0,  0, 120, 12 },
  { 0,  0, 130, 13 },
  { 0,  0, 140, 14 },
  { 0,  0, 150, 15 },        /* 15 */
  { 0,  1, 165, 16 },
  { 1,  2, 185, 17 },
  { 2,  3, 210, 18 },        /* 18 */
  { 3,  4, 240, 19 },
  { 5,  6, 275, 20 },        /* 20 - players and mobs diverge here */
  { 7,  8, 315, 21 },
  { 8,  9, 360, 22 },
  { 9, 10, 410, 23 },
  {10, 11, 465, 24 },
  {11, 12, 525, 25 },        /* 25 */
  { 2,  2, 210, 19 },        /* 18/01-50      TAKE OUT */
  { 2,  3, 220, 20 },        /* 18/51-75      TAKE OUT */
  { 3,  3, 230, 21 },        /* 18/76-90      TAKE OUT */
  { 3,  4, 240, 22 },        /* 18/91-99      TAKE OUT */
  { 4,  4, 250, 23 }         /* 18/100        TAKE OUT */
};

/* [dex] skillapply (thieves only) */
const struct dex_skill_type dex_app_skill[26] =
{
  {-25,-25,-25,-30,-25},	/* 0 */
  {-25,-20,-25,-25,-20},	/* 1 */
  {-20,-20,-20,-25,-20},
  {-20,-15,-20,-20,-15},
  {-15,-15,-15,-20,-15},
  {-15,-10,-15,-15,-10},	/* 5 */
  {-10,-10,-10,-15,-10},
  {-10, -5,-10,-10, -5},
  { -5, -5, -5,-10, -5},
  { -5,  0, -5, -5,  0},
  {  0,  0,  0, -5,  0},	/* 10 */
  {  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0},	/* 15 */
  {  0,  5,  0,  0,  0},
  {  5, 10,  0,  5,  5},
  { 10, 10,  5, 10, 10},
  { 10, 15,  5, 10, 10},
  { 15, 20, 10, 15, 15},	/* 20 */
  { 15, 20, 10, 15, 15},
  { 20, 25, 15, 20, 20},
  { 20, 25, 15, 20, 20},
  { 25, 30, 20, 25, 25},
  { 25, 30, 20, 25, 25}	/* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[ABS_MAX_LVL] =
{
  1,	/* 0 */
  1,	/* 1 */
  1,
  1,
  1,
  1,	/* 5 */
  1,
  1,
  1,
  1,
  2,	/* 10 */
  2,
  2,
  2,
  2,
  3,	/* 15 */
  3,
  3,
  3,
  3,
  4,	/* 20 */
  4,
  4,
  4,
  4,
  5,	/* 25 */
  5,
  5,
  5,
  5,
  6,	/* 30 */
  6,
  6,
  6,
  6,
  7,	/* 35 */
  7,
  7,
  7,
  7,
  8,	/* 40 */
  8,
  8,
  8,
  8,
  9,	/* 45 */
  9,
  9,
  9,
  9,
  10,	/* 50 */
  10,
10,
10,
11,
11,	/* 55 */
  11,
11,
11,
11,
12,
12,	/* 60 */
12,
12,
12,
12,
13,     /* 65 */
13,
13,
13,
13,
13,      /* 70 */
14,
14,
14,
14,
14,       /* 75 */
14,
14,
14,
15,
15,      /* 80 */
15,
15,
15,
15,
15,     /* 85 */
15,
15,
15,
15,
15,     /* 90 */
16,
16,
16,
16,
16,     /* 95 */
17,
17,
17,
17,
17,     /* 100 */
18,
18,
18,
18,
18,     /* 105 */
19,
19,
19,
19,
19,    /* 110 */
20,
20,
20,
20,
20,    /* 115 */
21,
21,
21,
21,
21,    /* 120 */
22,
22,
22,
22,
22,   /* 125 */
22
};

/* [dex] apply (all) */
const struct dex_app_type dex_app[26] =
{
  {-7,-7, 60}, /* 0 */
  {-6,-6, 50}, /* 1 */
  {-4,-4, 50},
  {-3,-3, 40},
  {-2,-2, 30},
  {-1,-1, 20}, /* 5 */
  { 0, 0, 10},
  { 0, 0,  0},
  { 0, 0,  0},
  { 0, 0,  0},
  { 0, 0,  0}, /* 10 */
  { 0, 0,  0},
  { 0, 0,  0},
  { 0, 0,  0},
  { 0, 0,  0},
  { 0, 0, -5}, /* 15 */
  { 0, 1,-10},
  { 1, 2,-15},
  { 2, 3,-20},
  { 3, 4,-25},
  { 4, 5,-35}, /* 20 */
  { 5, 6,-45},
  { 5, 6,-50},
  { 5, 6,-50},
  { 5, 6,-50},
  { 5, 6,-50}	 /* 25 */
};

/* [con] apply (all) */
const struct con_app_type con_app[26] =
{
  {-7, 0},	/* 0 */
  {-6, 6},	/* 1 */
  {-5,12},
  {-4,18},
  {-3,24},
  {-2,30},	/* 5 */
  {-1,36},
  { 0,42},
  { 0,48},
  { 0,54},
  { 0,60},    /* 10 */
  { 0,65},
  { 0,70},
  { 0,75},
  { 0,80},
  { 1,85},	/* 15 */
  { 2,90},
  { 2,95},
  { 3,97},    /* 18 */
  { 3,99},    /* 19 */
  { 4,99},	/* 20 */
  { 5,100},
  { 5,100},
  { 5,100},
  { 5,100},
  { 5,100}	/* 25 */
};

/* [int] apply (all) */
const struct int_app_type int_app[26] =
{
  { 0 }, /* 0 0-18 NEED TO BE REREVAMPED */
  { 4 },	/* 1 */
  { 5 },
  { 6 },
  { 7 },
  { 8 },	/* 5 */
  { 9  },
  { 10 },
  { 11 },
  { 12 },
  { 14 },	/* 10 */
  { 16 },
  { 20 },
  { 20 },
  { 25 },
  { 25 },	/* 15 */
  { 32 },
  { 32 },
  { 49 },	/* 18 */
  { 52 },	/* 19 */
  { 55 },	/* 20 */
  { 60 },
  { 65 },
  { 70 },
  { 75 },
  { 80 }	/* 25 */
};

/* [wis] apply (all) */
const struct wis_app_type wis_app[26] =
{
  { 0 },	/* 0 */
  { 0 },	/* 1 */
  { 0 },
  { 0 },
  { 0 },
  { 1 },	/* 5 */
  { 1 },
  { 1 },
  { 1 },
  { 1 },
  { 1 },	/* 10 */
  { 1 },
  { 1 },
  { 2 },
  { 2 },
  { 2 },	/* 15 */
  { 2 },
  { 3 },
  { 4 },	/* 18 */
  { 4 },	/* 19 */
  { 4 },	/* 20 */
  { 4 },
  { 5 },
  { 5 },
  { 5 },
  { 5 }	/* 25 */
};


#if 0
const struct QuestItem QuestList[4][ABS_MAX_LVL] =
{
  /*   item #,    Where   */
  {
    { 0,        ""},
    { 1,        "in the temple of Midgaard\n\r"},
    { 1410,     "in the Ivory Tower\n\r"}, /* sack o flour */
    { 6010      "with the deer, in the forest\n\r"}, /* blackberries */
    { 3013,     "in the tower with the oil beasts\n\r"}, /* biscuit      */
    { 20,       "within the power of magic\n\r"}, /* ball o light */
    { 1414,     "with the elves\n\r"}, /* flour potion ??? */
    { 3050,     "with the muttering man\n\r"},
    { 106,      "with a toad, of course\n\r"},
    { 109,      "in a treehouse\n\r"}, /* a rubber ball */
    /*10*/
    { 3628,     "in the new city\n\r"}, /* padded leather boots */
    { 113,      "with a gelfling child\n\r"}, /* a top */
    { 19204,    "in the castle of Mists\n\r"}, /* moonstone */
    { 20006,    "in a place with lots of spiders\n\r"}, /* spider-web */

  },				/* mage   */

  {
    { 0,        ""},
    { 2,        "in the temple of Midgaard\n\r"},
    { 1110,     "in the Shire\n\r"}, /* egg */
    { 3070,     "in the armorer's shop\n\r"}, /* bronze gauntlets */
    { 3057,     "in many places... Perhaps Moria\n\r"}, /* silver pot ??? */
    { 6001,     "within the peaceful trees\n\r"}, /* chequered shirt  */
    { 4052,     "in the deep places where the kobolds live\n\r"}, /*mushroom */
    { 3025,     "in the hands of an unkind man\n\r"}, /* battle axe */
    { 6106,     "with the wolves and wargs\n\r"}, /* non-pois. toadstool */
    { 107,      "with a cleric, no doubt.\n\r"},
    { 110,      "high in the mountains\n\r"}, /* a piece of quartz */
    /*10*/
    { 3649,     "in the new city\n\r"},
    { 7206,     "in the lair of the mindflayers\n\r"},
    { 114,      "on a white fox\n\r"}, /* silver collar */
    { 19203,    "in the castle of Mists\n\r"}, /* large axe */

  },				/* cleric  */

  {
    { 0,        ""},
    { 3,        "in the temple of Midgaard\n\r"},
    { 41,       "in the grand temple of Odin\n\r"},	/* lock picks */
    { 3071,     "in the armorer's shop\n\r"}, /* leather gloves  */
    { 30,       "in all sorts of places.. any scraps will do.\n\r"},
    { 3907,     "in the shop of the great Brewer\n\r"}, /* beer */
    { 19202,    "in the castle of Mists\n\r"}, /* glowing gland   */
    { 4104,     "with the orks and kobolds\n\r"}, /* slime  */
    { 6006,     "in a fireplace.\n\r"}, /* key */
    { 111,      "at the headwaters of the Dark River\n\r"},   /* fools gold */
    /*10*/
    { 3640,     "in the new city\n\r"}, /* hunk o cheese */
    { 4101,     "in the hands of a warrior\n\r"}, /* swordsman's gloves */
    { 115,      "in the hands of a speed demon\n\r"}, /* speed pills */
    { 116,      "in an ancient tollbooth\n\r"}, /* moss */

    { 7190,     "in the possession of a giant rant\n\r"},
    { 105,      "on a snake with deadly poison\n\r"}, /* snake fangs */

  },				/* thief   */

  {
    { 0,        ""},
    { 4,        "in the temple of Midgaard\n\r"},
    { 42,       "in the grand temple of the wise one-eye\n\r"},
    { 3013,     "in the Ivory Tower\n\r"},
    { 6000,     "in the forest, with the foxes and rabbits\n\r"},
    { 4000,     "on the hand of a stupid orc\n\r"},	/* ring of weakness */
    { 8121,     "near the river of Brandy\n\r"}, /* velcro bag */
    { 28009,    "in a place not far from your home\n\r"}, /* quarter staff */
    { 1109,     "with the tough little people"},
 /*    { 108,      "floating somewhere on the dark river\n\r"}, *//* flotsam */
    { 112,      "in the castle of Snakes\n\r"}, /*hunk o wool*/
    /*10*/
    { 3621,     "in the new city\n\r"}, /* splinted shield */
    { 16024,    "high in the mountains\n\r"}, /* a mandolin */
    { 7405,     "in a secret room in the sewers\n\r"},
    { 117,      "on the hide of an angry wilkey\n\r"}, /* patch of fur */

  }				/* warrior */
}
#endif
#endif

const char *RaceName[] =
{
  "Human",
  "Elf",
  "Dwarf",
  "Hobbit",
  "Gnome",
  "Doppleganger",
  "Felis",
  "Canis",
  "Reptile",
  "Insect",
  "Avian",
  "Marine",
  "Amphibian",
  "Carnivore",
  "Herbivore",
  "Omnivore",
  "Flora",
  "Elemental",
  "Planar",
  "Ogre",
  "Giant",
  "Troll",
  "Orc",
  "Goblin",
  "Dragon",
  "Undead",
  "Underworld",
  "Heaven",
  "Demi",
  "Drow",
  "Golem",
  "Enfan",
  "Minotaur",
  "Cyclops",
  "Mindflayer",
  "Gargoyle",
  "Skexie",
  "Troglodyte",
  "Gnoll",
  "Bugbear",
  "Quickling",
  "Patryn",
  "Sartan",
  "Tytan",
  "Halfbreed",
  "Kobold",
  "Rodent",
  "Githyanki",
  "Treant",
  "Half Elf",
  "Forest Elf",
  "Half Orc",
  "Hill Giant",
  "Stone Giant",
  "Frost Giant",
  "Fire Giant",
  "Cloud Giant",
  "Storm Giant",
  "Pixie",
  "Sherrinpip",
  "\n"
};

struct flagdata edit_room_flag[] = {
    {DARK,		"dark"		},
    {DEATH,		"death"		},
    {NO_MOB,		"no_mob"	},
    {INDOORS,		"indoors"	},
    {PEACEFUL,		"peaceful"	},
    {NOSTEAL,		"nosteal"	},
    {NO_TRAVEL_OUT,	"no_travel_out"	},
    {NO_MAGIC,		"no_magic"	},
    {TUNNEL,		"tunnel"	},
    {NO_TRAVEL_IN,	"no_travel_in"	},
    {SILENCE,		"silence"	},
    {NO_PUSH,		"no_push"	},
    {IMMORT_RM,		"immort_rm"	},
    {GOD_RM,		"god_rm"	},
    {NO_RECALL,		"no_recall"	},
    {ARENA,		"arena"		},
    {NO_SNEAK,		"no_sneak"	},
    {TEMPLE,            "temple"        },
    {BRUJAH_RM,		"brujah_rm"	},
    {1,			"current"	},
    {0}
};

struct flagdata edit_affected[] = {
    {AFF_BLIND,			"blind"			},
    {AFF_INVISIBLE,		"invisible"		},
    {AFF_REGENERATE,		"regenerate"		},
    {AFF_DETECT_INVISIBLE,	"detect_invis"		},
    {AFF_SENSE_AURA,		"sense_aura"		},
    {AFF_SENSE_LIFE,		"sense_life"		},
    {AFF_LIFE_PROT,		"life_protection"	},
    {AFF_SANCTUARY,		"sanctuary"		},
    {AFF_GROUP,			"group"			},
    {AFF_BERSERK,		"berserk"		},
    {AFF_CURSE,			"curse"			},
    {AFF_FLYING,		"flying"		},
    {AFF_POISON,		"poison"		},
    {AFF_ILLUSION,		"illusion"		},
    {AFF_PARALYSIS,		"paralysis"		},
    {AFF_INFRAVISION,		"infravision"		},
    {AFF_WATERBREATH,		"waterbreath"		},
    {AFF_SLEEP,			"sleep"			},
    {AFF_DODGE,			"dodge"			},
    {AFF_SNEAK,			"sneak"			},
    {AFF_HIDE,			"hide"			},
    {AFF_SILENCE,		"silence"		},
    {AFF_CHARM,			"charm"			},
    {AFF_FOLLOW,		"follow"		},
    {AFF_UNDEF_1,		"undef_1"		},
    {AFF_TRUE_SIGHT,		"true_sight"		},
    {AFF_SCRYING,		"scrying"		},
    {AFF_FIRESHIELD,		"fireshield"		},
    {AFF_CONTINUAL_DARK,	"continual_dark"	},
    {AFF_MEDITATE,		"meditate"		},
    {AFF_GREAT_SIGHT,		"great_sight"		},
    {AFF_CONTINUAL_LIGHT,	"continual_light"	},
    {1,				"current"		},
    {0}
};

struct flagdata edit_affected2[] = {
    {AFF2_HASTE,	"haste"		},
    {AFF2_SLOW,		"slow"		},
    {AFF2_DESPAIR,	"despair"	},
    {AFF2_TOLERANCE,	"tolerance"	},
    {AFF2_RAGE,		"rage"		},
    {AFF2_ROUGHNESS,	"roughness"	},
    {AFF2_RESISTANCE,	"resistance"	},
    {AFF2_ELECSHIELD,	"elecshield"	},
    {AFF2_POISONSHIELD,	"poisonshield"	},
    {AFF2_ENERGYSHIELD,	"energyshield"	},
    {AFF2_VAMPSHIELD,	"vampshield"	},
    {AFF2_MANASHIELD,	"manashield"	},
    {AFF2_ACIDSHIELD,	"acidshield"	},
    {AFF2_COLDSHIELD,	"coldshield"	},
    {AFF2_MINDPROTECT,	"mindprotect"	},
    {AFF2_ABSORB,	"absorb"	},
    {AFF2_ROUGH,	"rough"		},
    {AFF2_MOVESHIELD,	"moveshield"	},
    {AFF2_NOSUMMON,     "no-summon"     },
    {AFF2_FLIGHT,	"flight"	},
    {1,			"current"	},
    {0}
};

struct flagdata edit_mob_actions[] = {
    {ACT_SPEC,		"specialprog"	},
    {ACT_SENTINEL,	"sentinel"	},
    {ACT_SCAVENGER,	"scavenger"	},
    {ACT_ISNPC,		"isnpc"		},
    {ACT_NICE_THIEF,	"nice_thief"	},
    {ACT_AGGRESSIVE,	"aggressive"	},
    {ACT_STAY_ZONE,	"stay_zone"	},
    {ACT_WIMPY,		"wimpy"		},
    {ACT_ANNOYING,	"annoying"	},
    {ACT_HATEFUL,	"hateful"	},
    {ACT_AFRAID,	"afraid"	},
    {ACT_IMMORTAL,	"immortal"	},
    {ACT_ROAM,		"roam"		},
    {ACT_DEADLY,	"deadly"	},
    {ACT_POLYSELF,	"polyself"	},
    {ACT_META_AGG,	"meta_agg"	},
    {ACT_GUARDIAN,	"guardian"	},
    {ACT_IT,		"it"		},
    {ACT_PATTACK,	"pattack"	},
    {ACT_LIQUID,	"liquid"	},
    {ACT_SHIFTER,	"shifter"	},
    {ACT_STEED,		"steed"		},
    {ACT_HUGE,		"huge"		},
    {1,			"current"	},
    {0}
};

struct flagdata edit_item_wear[] = {
    {ITEM_TAKE,		"take"	},
    {ITEM_WEAR_FINGER,	"finger"},
    {ITEM_WEAR_NECK,	"neck"	},
    {ITEM_WEAR_BODY,	"body"	},
    {ITEM_WEAR_HEAD,	"head"	},
    {ITEM_WEAR_LEGS,	"legs"	},
    {ITEM_WEAR_FEET,	"feet"	},
    {0}
};

const struct res_altar_data res_altar_rooms[] = {
    {1082, 75},
    {1083, 100},
    {1084, 120},
    {1085, 225},
    {1086, 49},
    {1087, 25},
    {0}
};

