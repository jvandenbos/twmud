
/*
**  just for polymorph spell(s)
*/

struct PolyType {
  char *name;
  int  level;
  int  number;
  int  age;
};

const struct PolyType ansum_list[] = 
{
    { "fox",		5,  VMOB_61,10 },
    { "owl",		10, VMOB_62, 4 },
    { "stag",		15, VMOB_63, 7 },
    { "cougar",		20, VMOB_64, 10 },
    { "grizzly",	25, VMOB_65, 17 },
    { "treant",		30, VMOB_66, 99 },
    { "giant",		50, VMOB_67, 67 },
    { 0 }
};

const struct PolyType monsum_list[] = 
{
    { "goblin",		5,  VMOB_48,45 },
    { "orc",		10, VMOB_49, 66 },
    { "kobold",		15, VMOB_50,23 },
    { "bugbear",	20, VMOB_51, 16 },
    { "cockatrice",	25, VMOB_52, 55 },
    { "griffin",	30, VMOB_53, 33 },
    { "behir",		50, VMOB_54, 45 },
    { 0 }
};

const struct PolyType PolyList[] = {
  {"giant-rat", 1, VMOB_1, 25},             /* Giant Rat               */
  {"kobold-peon", 2, VMOB_2, 25},           /* Kobold Peon             */
  {"rock-toad",   3, VMOB_3, 15},           /* Rock Toad               */
  {"pixie-guard", 4, VMOB_4, 50},           /* Pixie Guard             */
  {"beetle", 5, VMOB_5, 100},                /* Fire Beetle             */
  {"goblin", 6, VMOB_6, 66},                /* Goblin Worker           */
  {"snake", 7, VMOB_7, 7},                 /* Moccasin Snake          */
  {"brownie", 8, VMOB_8, 15},               /* Brownie Knight          */
  {"cougar", 9, VMOB_9, 10},                /* Cougar                  */
  {"orc", 10, VMOB_10, 17},                 /* Orc Raider              */
  {"wolf", 11, VMOB_11, 20},                /* Wolf                    */
  {"ogre", 12, VMOB_12, 25},                /* Ogre Henchman           */
  {"lion", 13, VMOB_13, 15},                /* Lion                    */
  {"lizardman", 14, VMOB_14, 35},           /* Lizardman Warrior       */
  {"warg", 15, VMOB_15, 37},                /* Warg                    */
  {"troll", 16, VMOB_16, 44},               /* Troll Berserker         */
  {"spider", 17, VMOB_17, 19},              /* Crystal Spider          */
  {"hill", 18, VMOB_18, 44},                /* Hill Giant Smasher      */
  {"beast", 19, VMOB_19, 120},               /* Shadow Beast            */
  {"quickling", 20, VMOB_20, 111},           /* Quickling Raider        */
  {"gorgon", 21, VMOB_21, 67},              /* Gorgon                  */
  {"githyanki", 22, VMOB_22, 143},           /* Githyanki Soldier       */
  {"otyugh", 23, VMOB_23, 122},              /* Otyugh                  */
  {"shadow-warrior", 24, VMOB_24,145},      /* Shadow Warrior          */
  {"green", 25, VMOB_25, 255},               /* Green Dragon            */
  {"troglodyte", 26, VMOB_26, 77},          /* Troglodyte BattleMaster */
  {"xorn", 27, VMOB_27, 99},                /* Lesser Xorn             */
  {"cyclops", 28, VMOB_28, 144},             /* Great Cyclops           */
  {"salamander", 29, VMOB_29, 25},          /* Frost Salamander        */
  {"draconian", 30, VMOB_30, 25},           /* Draconian Sentry        */
  {"slatherer", 32, VMOB_31, 33},           /* Slatherer               */
  {"storm", 34, VMOB_32, 78},               /* Storm Giant             */
  {"shark", 36, VMOB_33, 17},               /* Tiger Shark             */
  {"minotaur", 38, VMOB_34, 44},            /* Minotaur Scout          */
  {"centaur", 40, VMOB_35, 99},             /* Centaur Shaman          */
  {"tytan", 42, VMOB_36, 99},               /* Tytan Hero              */
  {"black", 44, VMOB_37, 350},               /* Black Dragon            */
  {"vampire", 46, VMOB_38, 257},             /* Vampire                 */
  {"roc", 48, VMOB_39, 23},                 /* Vicious Roc             */
  {"shadowen", 50, VMOB_40, 166},            /* Shadowen                */
  {"phoenix", 52, VMOB_41, 109},             /* Phoenix                 */
  {"mindflayer", 54, VMOB_42, 74},          /* Master Mindflayer       */
  {"beholder", 56, VMOB_43, 222},            /* Beholder                */
  {"githezai-healer", 58, VMOB_44, 79},     /* Githezai Healer         */
  {"elfeater", 60, VMOB_45, 98},            /* Elfeater                */
  {"orc-hero", 62, VMOB_46, 75},            /* Orc Hero                */
  {"towering-treant", 64, VMOB_133, 177},    /* Towering Treant         */
  {"beysib-fighter", 66, VMOB_134, 33},     /* Beysib Fighter          */
  {"efreet-lord", 68, VMOB_135, 45},        /* Efreet Lord             */
  {"ethereal-knight", 70, VMOB_136, 115},    /* Ethereal Knight         */
  {"wyvern", 72, VMOB_137, 201},             /* Wyvern                  */
  {"drow-spellsword", 74, VMOB_138, 45},    /* Drow SpellSword         */
  {"kraken", 76, VMOB_139, 225},             /* Kraken                  */
  {"demon-magi", 78, VMOB_140, 166},         /* Demon Magi              */
  {"tangle-tree", 80, VMOB_141, 100},        /* Tangle Tree             */
  {"skaven-general", 82, VMOB_142, 45},     /* Skaven General          */
  {"gorgimera", 84, VMOB_143, 89},          /* Gorgimera               */
  {"succubus", 86, VMOB_144, 66},           /* Succubus                */
  {"hydra", 88, VMOB_145, 27},              /* Hydra                   */
  {"planetar-defender", 90, VMOB_146, 37},  /* Planetar Defender       */
  {"silver-unicorn", 92, VMOB_147, 199},     /* Silver Unicorn          */
  {"sahuagin-unicorn", 94, VMOB_148, 67},   /* Sahuagin Witch          */
  {"behemoth", 96, VMOB_149, 23},           /* Behemoth                */
  {"solar-lawgiver", 98, VMOB_150,208 },     /* Solar Lawgiver          */
  {"leviathan", 100, VMOB_151,299},         /* Leviathan               */
  {"avatar", 225, VMOB_152, 500},            /* Avatar                  */
  {0, 0, 0, 0}, 
};
