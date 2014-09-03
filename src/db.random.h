#ifndef DB_RANDOM_H
#define DB_RANDOM_H


void setup_random_tree();
void add_random_obj(int,struct obj_data *);
void add_random_obj(struct obj_data *);

extern long random_count;

//if NUMBER_RAND_LOGS is defined,
//then slog will print it's number,
//instead of text.
#define NUMBER_RAND_LOGS 1

#define RAND_ITEM_LIGHT (1 << ITEM_LIGHT)
#define RAND_ITEM_SCROLL (1 << ITEM_SCROLL)
#define RAND_ITEM_WAND (1 << ITEM_WAND)
#define RAND_ITEM_STAFF (1 << ITEM_STAFF)
#define RAND_ITEM_WEAPON (1 << ITEM_WEAPON)
#define RAND_ITEM_FIREWEAPON (1 << ITEM_FIREWEAPON)
#define RAND_ITEM_MISSILE (1 << ITEM_MISSILE)
#define RAND_ITEM_TREASURE (1 << ITEM_TREASURE)
#define RAND_ITEM_ARMOR (1 << ITEM_ARMOR)
#define RAND_ITEM_POTION (1 << ITEM_POTION)
#define RAND_ITEM_WORN (1 << ITEM_WORN)
#define RAND_ITEM_OTHER (1 << ITEM_OTHER)
#define RAND_ITEM_TRASH (1 << ITEM_TRASH)
#define RAND_ITEM_TRAP (1 << ITEM_TRAP)
#define RAND_ITEM_CONTAINER (1 << ITEM_CONTAINER)
#define RAND_ITEM_NOTE (1 << ITEM_NOTE)
#define RAND_ITEM_DRINKCON (1 << ITEM_DRINKCON)
#define RAND_ITEM_KEY (1 << ITEM_KEY)
#define RAND_ITEM_FOOD (1 << ITEM_FOOD)
#define RAND_ITEM_MONEY (1 << ITEM_MONEY)
#define RAND_ITEM_PEN (1 << ITEM_PEN)
#define RAND_ITEM_BOAT (1 << ITEM_BOAT)
#define RAND_ITEM_AUDIO (1 << ITEM_AUDIO)
#define RAND_ITEM_BOARD (1 << ITEM_BOARD)

#define RAND_GROUP_WEAPONS RAND_ITEM_WEAPON | RAND_ITEM_FIREWEAPON
#define RAND_GROUP_GENERAL RAND_ITEM_LIGHT | RAND_ITEM_WAND | RAND_ITEM_STAFF | RAND_GROUP_WEAPONS | RAND_ITEM_MISSILE | RAND_ITEM_ARMOR | RAND_ITEM_WORN
#define RAND_GROUP_FOOD RAND_ITEM_FOOD

// *** Random tree defines ***

#define RT_AFFECTS     1
#define RT_AFFECTS2    2
#define RT_CHAOS       3
#define RT_STATS       4
#define RT_CLASSES     5
#define RT_MIXED_AFF   6
#define RT_SHIELDS     7
#define RT_SIGHT_AFF   8
#define RT_WS          9
#define RT_UBER        10

#define RT_CHAOS_SUS   50
#define RT_CHAOS_RES   51
#define RT_CHAOS_IMM   52

#define RT_STATS_STR   100
#define RT_STATS_INT   101
#define RT_STATS_WIS   102
#define RT_STATS_DEX   103
#define RT_STATS_CON   104
#define RT_STATS_CHA   105
#define RT_STATS_AGE   106
#define RT_STATS_DAM   107
#define RT_STATS_HIT   108
#define RT_STATS_HND   109
#define RT_STATS_AC    110
#define RT_STATS_HP    111
#define RT_STATS_MP    112
#define RT_STATS_MOV   113

#define RT_CLASSES_WAR   150
#define RT_CLASSES_THIEF 151
#define RT_CLASSES_RANG  152
#define RT_CLASSES_SHIFT 153
#define RT_CLASSES_DRUID 154

#define RT_WS_ACID     200
#define RT_WS_COLD     201
#define RT_WS_DRAIN    202
#define RT_WS_ELEC     204
#define RT_WS_ENERGY   205
#define RT_WS_FIRE     206
#define RT_WS_NATURE   207
#define RT_WS_POISON   208
#define RT_WS_MISC     209

// *** Random defines ***

#define RN_TROLLS      100
#define RN_PROPHETS    101
#define RN_WRETCHED    102
#define RN_DISEASE     103
#define RN_ANGER       104
#define RN_EVASION     105
#define RN_LAKE        106
#define RN_NARCOLEPSY  107
#define RN_LEVITATION  108
#define RN_SHADOWS     109
#define RN_MYSTERY     110
#define RN_INVISIBLE   111
#define RN_BLUR        112
#define RN_HARMONY     113
#define RN_GODS        114

#define RN_TEPHANIS    201
#define RN_MAHNTOR     202
#define RN_GLOOM       203
#define RN_SILENCE     204

#define RN_CHAOS_S_FIRE    301
#define RN_CHAOS_S_COLD    302
#define RN_CHAOS_S_ELEC    303
#define RN_CHAOS_S_ENERGY  304
#define RN_CHAOS_S_BLUNT   305
#define RN_CHAOS_S_PIERCE  306
#define RN_CHAOS_S_SLASH   307
#define RN_CHAOS_S_ACID    308
#define RN_CHAOS_S_POISON  309
#define RN_CHAOS_S_DRAIN   310
#define RN_CHAOS_S_SLEEP   311
#define RN_CHAOS_S_CHARM   312
#define RN_CHAOS_S_NONMAG  313
#define RN_CHAOS_S_HOLD    314
#define RN_CHAOS_S_BARD    315

#define RN_CHAOS_R_FIRE    401
#define RN_CHAOS_R_COLD    402
#define RN_CHAOS_R_ELEC    403
#define RN_CHAOS_R_ENERGY  404
#define RN_CHAOS_R_BLUNT   405
#define RN_CHAOS_R_PIERCE  406
#define RN_CHAOS_R_SLASH   407
#define RN_CHAOS_R_ACID    408
#define RN_CHAOS_R_POISON  409
#define RN_CHAOS_R_DRAIN   410
#define RN_CHAOS_R_SLEEP   411
#define RN_CHAOS_R_CHARM   412
#define RN_CHAOS_R_NONMAG  413
#define RN_CHAOS_R_HOLD    414
#define RN_CHAOS_R_BARD    415

#define RN_CHAOS_I_FIRE    450
#define RN_CHAOS_I_COLD    451
#define RN_CHAOS_I_ELEC    452
#define RN_CHAOS_I_ENERGY  453
#define RN_CHAOS_I_BLUNT   454
#define RN_CHAOS_I_PIERCE  456
#define RN_CHAOS_I_SLASH   457
#define RN_CHAOS_I_ACID    458
#define RN_CHAOS_I_POISON  459
#define RN_CHAOS_I_DRAIN   460
#define RN_CHAOS_I_SLEEP   461
#define RN_CHAOS_I_CHARM   462
#define RN_CHAOS_I_NONMAG  463
#define RN_CHAOS_I_HOLD    464
#define RN_CHAOS_I_BARD    465

#define RN_WEAKNESS        500
#define RN_MIGHT           501
#define RN_STRENGTH        502
#define RN_POWER           503

#define RN_FOOLS           510
#define RN_LEARNING        511
#define RN_INTELLIGENCE    512
#define RN_KNOWLEDGE       513

#define RN_IGNORANCE       520
#define RN_DISCRETION      521
#define RN_WISDOM          522
#define RN_JUDGEMENT       523

#define RN_FUMBLING        530
#define RN_SKILL           531
#define RN_DEXTERITY       532
#define RN_AGILITY         533

#define RN_ILLNESS         540
#define RN_STAMINA         541
#define RN_CONSTITUTION    542
#define RN_ENDURANCE       543

#define RN_MONSTROSITY     550
#define RN_ATTRACTION      551
#define RN_CHARISMA        552
#define RN_BEAUTY          553

#define RN_ETERNITY        560
#define RN_LONGEVITY       561
#define RN_YOUTH           562
#define RN_DAYS            563
#define RN_YEARS           564
#define RN_CENTURIES       565
#define RN_MILLENIA        566
#define RN_EONS            567

#define RN_PITY            570
#define RN_HARM            571
#define RN_DAMAGE          572
#define RN_PAIN            573

#define RN_MERCY           580
#define RN_AIM             581
#define RN_PRECISION       582
#define RN_ACCURACY        583

#define RN_GUILT           590
#define RN_DEATH           591
#define RN_BLOOD           592
#define RN_TRAUMA          593
#define RN_GORE            594
#define RN_CARNAGE         595

#define RN_VULNERABILITY   600
#define RN_FORTIFICATION   601
#define RN_PROTECTION      602
#define RN_DEFENCE         603
#define RN_FRAILTY         604
#define RN_TOLERANCE       605
#define RN_RESISTANCE      606

#define RN_SLAVES          610
#define RN_VASSALS         611
#define RN_FIGHTERS        612
#define RN_WARRIORS        613
#define RN_HEROES          614
#define RN_KINGS           615

#define RN_IDIOTS          620
#define RN_CRETINS         621
#define RN_SAGES           622
#define RN_MAGES           623
#define RN_SORCERERS       624
#define RN_WIZARDS         625

#define RN_SLOTHS          630
#define RN_LOITERERS       631
#define RN_PILGRIMS        632
#define RN_NOMADS          633
#define RN_TRAVELERS       634
#define RN_EXPLORERS       635

#define RN_BASHING         700
#define RN_RAM             701
#define RN_RHINO           702

#define RN_DECEIT          710
#define RN_SHADES          711

#define RN_BANDITS         720
#define RN_ROBBERS         721

#define RN_FILCHERS        730
#define RN_OUTLAWS         731

#define RN_BRIGANDS        740
#define RN_STEALTH         741

#define RN_PIERCING        750
#define RN_THIEVES         751

#define RN_LOST            760
#define RN_SCOUTS          761
#define RN_PURSUIT         762
#define RN_PREDATORS       763

#define RN_GLASSGRIP       770
#define RN_GOUT            771
#define RN_ARTHRITIS       772
#define RN_IRONFIST        773
#define RN_BRASSCLAW       774
#define RN_GRIP            775
#define RN_VISE            776

#define RN_RIDING          780
#define RN_JOUSTING        781

#define RN_AVATAR          800
#define RN_BALROG          801
#define RN_ELFEATER        802
#define RN_LICH            803
#define RN_PHOENIX         804
#define RN_ROC             805
#define RN_DREAMMASTER     806
#define RN_EFREETI         807
#define RN_DEMONSPAWN      808

#define RN_DEADMEAT        809
#define RN_IX              810
#define RN_LEDEN           811
#define RN_NOVAK           812
#define RN_SOLAAR          813
#define RN_QUILAN          814
#define RN_DREUGH          815
#define RN_TIMUS           816
#define RN_TANGA           817
#define RN_SLIM            818
#define RN_OMEN	      819

#define RN_FIRESHIELD      900
#define RN_LIGHTNING       901
#define RN_POISON          902
#define RN_ENERGY          903
#define RN_HATE            904
#define RN_MAGIC           905
#define RN_ACID            906
#define RN_ICE             907
#define RN_RADIATION       908

#define RN_BLINDNESS       950
#define RN_INSIGHT         951
#define RN_SIGHT           952
#define RN_VISION          953
#define RN_PERCEPTION      954
#define RN_DETECTION       955
#define RN_CLAIRVOYANCE    956
#define RN_DIVINATION      957
#define RN_BLACKLIGHT      958
#define RN_BRILLIANCE      959

#define RN_CTHULHU         1000
#define RN_FIRESTAR        1001

#define RN_COLD            1010
#define RN_FROST           1011
#define RN_BLIZZARDS       1012

#define RN_GHOULS          1020
#define RN_GHOSTS          1021
#define RN_VAMPIRES        1022
#define RN_GHASTS          1023

#define RN_SHOCK           1030
#define RN_THUNDER         1031
#define RN_STATIC          1032
#define RN_STORMS          1033
#define RN_ZEUS            1034

#define RN_SUFFERING       1040
#define RN_ANGUISH         1041
#define RN_AGONY           1042
#define RN_TORMENT         1043
#define RN_TORTURE         1044

#define RN_HEAT            1050
#define RN_FIRE            1051
#define RN_IMMOLATION      1052
#define RN_INFERNO         1053
#define RN_VOLCANOES       1054

#define RN_ARACHNOS        1060
#define RN_PARAMOR         1061
#define RN_EXPOSURE        1062
#define RN_THALIDS         1063
#define RN_DRYADS          1064
#define RN_HURRICANES      1065
#define RN_XORN            1066
#define RN_GEYSERS         1067
#define RN_LOCUSTS         1068

#define RN_SERPENTS        1070
#define RN_ASPS            1071

#define RN_EXILE           1080
#define RN_THE_BAT         1081
#define RN_EMPATHY         1082
#define RN_SLUMBER         1083
#define RN_PURITY          1084
#define RN_BASILISKS       1085
#define RN_PHOBIAS         1086
#define RN_THE_HOLY        1087

#define RN_RAINBOWS        1500
#define RN_ALLSIGHT        1501
#define RN_HIDING          1502
#define RN_PROTECTOR       1503
#define RN_EHEALTH         1504
#define RN_EMANA           1505
#define RN_ELIFE           1506
#define RN_FLAWLESS        1507
#define RN_SLAUGHTER       1508
#define RN_RAGE            1509

struct random_tree_node {
   long per;
   long num;
};

#define MAX_RAND_CHILD   20

struct random_tree {
   long num;
   long per;
   long type;
   long level;
   int num_nchild;
   int num_tchild;
   random_tree_node *nchild[MAX_RAND_CHILD];
   random_tree      *tchild[MAX_RAND_CHILD];
};

struct random_node_type {
   long num;
   char *name;
   int minlevel;
   int affnum;
   long affmin;
   long affmax;
};

extern struct random_node_type random_nodes[];

#endif
