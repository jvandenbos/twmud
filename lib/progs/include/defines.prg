#define IMM_FIRE      (1<<0)
#define IMM_COLD      (1<<1)
#define IMM_ELEC      (1<<2)
#define IMM_ENERGY    (1<<3)
#define IMM_BLUNT     (1<<4)
#define IMM_PIERCE    (1<<5)
#define IMM_SLASH     (1<<6)
#define IMM_ACID      (1<<7)
#define IMM_POISON    (1<<8)
#define IMM_DRAIN     (1<<9)
#define IMM_SLEEP    (1<<10)
#define IMM_CHARM    (1<<11)
#define IMM_HOLD     (1<<12)
#define IMM_NONMAG   (1<<13)
#define IMM_PLUS1    (1<<14)
#define IMM_PLUS2    (1<<15)
#define IMM_PLUS3    (1<<16)
#define IMM_PLUS4    (1<<17)
#define IMM_BARD     (1<<18)

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
#define ITEM_WEAR_LIGHT    65536

#define ITEM_GLOW          (1<<0)
#define ITEM_HUM           (1<<1)
#define ITEM_METAL         (1<<2)
#define ITEM_MINERAL       (1<<3)
#define ITEM_ORGANIC       (1<<4)
#define ITEM_INVISIBLE     (1<<5)
#define ITEM_MAGIC         (1<<6)
#define ITEM_NODROP        (1<<7)
#define ITEM_BLESS         (1<<8)
#define ITEM_ANTI_GOOD     (1<<9)
#define ITEM_ANTI_EVIL     (1<<10)
#define ITEM_ANTI_NEUTRAL  (1<<11)
#define ITEM_ANTI_CLERIC   (1<<12)
#define ITEM_ANTI_MAGE     (1<<13)
#define ITEM_ANTI_THIEF    (1<<14)
#define ITEM_ANTI_FIGHTER  (1<<15)
#define ITEM_BRITTLE       (1<<16)
#define ITEM_ANTI_PALADIN  (1<<17)
#define ITEM_ANTI_DRUID    (1<<18)
#define ITEM_ANTI_PSI      (1<<19)
#define ITEM_ANTI_RANGER   (1<<20)
#define ITEM_UNUSED0       (1<<21)
#define ITEM_UNUSED1       (1<<22)
#define ITEM_UNUSED2       (1<<23)
#define ITEM_UNUSED3       (1<<24)
#define ITEM_RARE          (1<<25)
#define ITEM_ANTI_BARD     (1<<26)
#define ITEM_ANTI_MONK     (1<<27)
#define ITEM_PURE_CLASS    (1<<28)
#define ITEM_TWO_HANDED    (1<<29)
#define ITEM_ANTI_SHIFTER  (1<<30)
#define ITEM_HARDEN        (1<<31)

#define DARK            (1 << 0)
#define DEATH           (1 << 1)
#define NO_MOB          (1 << 2)
#define INDOORS         (1 << 3)
#define PEACEFUL        (1 << 4)
#define NOSTEAL         (1 << 5)
#define NO_TRAVEL_OUT   (1 << 6)
#define NO_MAGIC        (1 << 7)
#define TUNNEL          (1 << 8)
#define NO_TRAVEL_IN    (1 << 9)
#define SILENCE         (1 << 10)
#define NO_PUSH         (1 << 11)
#define IMMORT_RM       (1 << 12)
#define GOD_RM          (1 << 13)
#define NO_RECALL       (1 << 14)
#define ARENA           (1 << 15)
#define NO_SNEAK        (1 << 16)
#define TEMPLE          (1 << 17)
#define BRUJAH_RM       (1 << 18)

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      1
#define EX_CLOSED      2
#define EX_LOCKED      4
#define EX_SECRET      8
#define EX_RSLOCKED    16
#define EX_PICKPROOF   32

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

#define AFF_BLIND             (1<<0)
#define AFF_INVISIBLE         (1<<1)
#define AFF_REGENERATE        (1<<2)
#define AFF_DETECT_INVISIBLE  (1<<3)
#define AFF_SENSE_AURA        (1<<4)
#define AFF_SENSE_LIFE        (1<<5)
#define AFF_LIFE_PROT         (1<<6)
#define AFF_SANCTUARY         (1<<7)
#define AFF_GROUP             (1<<8)
#define AFF_BERSERK           (1<<9)
#define AFF_CURSE             (1<<10)
#define AFF_FLYING            (1<<11)
#define AFF_POISON            (1<<12)
#define AFF_ILLUSION          (1<<13)
#define AFF_PARALYSIS         (1<<14)
#define AFF_INFRAVISION       (1<<15)
#define AFF_WATERBREATH       (1<<16)
#define AFF_SLEEP             (1<<17)
#define AFF_DODGE             (1<<18)
#define AFF_SNEAK             (1<<19)
#define AFF_HIDE              (1<<20)
#define AFF_SILENCE           (1<<21)
#define AFF_CHARM             (1<<22)
#define AFF_FOLLOW            (1<<23)
#define AFF_UNDEF_1           (1<<24)
#define AFF_TRUE_SIGHT        (1<<25)
#define AFF_SCRYING           (1<<26)
#define AFF_FIRESHIELD        (1<<27)
#define AFF_CONTINUAL_DARK    (1<<28)
#define AFF_MEDITATE          (1<<29)
#define AFF_GREAT_SIGHT       (1<<30)
#define AFF_CONTINUAL_LIGHT   (1<<31)

#define AFF2_HASTE            (1<<0)
#define AFF2_SLOW             (1<<1)
#define AFF2_DESPAIR          (1<<2)
#define AFF2_TOLERANCE        (1<<3)
#define AFF2_RAGE             (1<<4)
#define AFF2_ROUGHNESS        (1<<5)
#define AFF2_RESISTANCE       (1<<6)
#define AFF2_ELECSHIELD       (1<<7)
#define AFF2_POISONSHIELD     (1<<8)
#define AFF2_ENERGYSHIELD     (1<<9)
#define AFF2_VAMPSHIELD       (1<<10)
#define AFF2_MANASHIELD       (1<<11)
#define AFF2_ACIDSHIELD       (1<<12)
#define AFF2_COLDSHIELD       (1<<13)
#define AFF2_MINDPROTECT      (1<<14)
#define AFF2_ABSORB           (1<<15)
#define AFF2_ROUGH            (1<<16)
#define AFF2_MOVESHIELD       (1<<17)
#define AFF2_NOSUMMON         (1<<18)

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
#define APPLY_GUILD            44
#define APPLY_RIDE             47
#define APPLY_SPELL2           48
#define APPLY_AFF2             49
#define APPLY_CHA              50
#define APPLY_BOOK_SPELL       51
#define APPLY_DRAIN_LIFE       52

#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_WARRIOR     4
#define CLASS_THIEF       8
#define CLASS_PALADIN     16
#define CLASS_DRUID       32
#define CLASS_PSI         64
#define CLASS_RANGER      128
#define CLASS_SHIFTER     256
#define CLASS_MONK        512
#define CLASS_BARD        1024

#define ACT_SPEC       (1<<0)
#define ACT_SENTINEL   (1<<1)
#define ACT_SCAVENGER  (1<<2)
#define ACT_ISNPC      (1<<3)
#define ACT_NICE_THIEF (1<<4)
#define ACT_AGGRESSIVE (1<<5)
#define ACT_STAY_ZONE  (1<<6)
#define ACT_WIMPY      (1<<7)
#define ACT_ANNOYING   (1<<8)
#define ACT_HATEFUL    (1<<9)
#define ACT_AFRAID    (1<<10)
#define ACT_IMMORTAL  (1<<11)
#define ACT_ROAM      (1<<12)
#define ACT_DEADLY    (1<<13)
#define ACT_POLYSELF  (1<<14)
#define ACT_META_AGG  (1<<15)
#define ACT_GUARDIAN  (1<<16)
#define ACT_IT        (1<<17)
#define ACT_PATTACK   (1<<18)
#define ACT_LIQUID    (1<<19)
#define ACT_SHIFTER   (1<<20)
#define ACT_STEED     (1<<21)
#define ACT_HUGE      (1<<22)

#define SECS_PER_REAL_MIN       60
#define SECS_PER_REAL_HOUR      (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY       (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR      (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR       75
#define SECS_PER_MUD_DAY        (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH      (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR       (17*SECS_PER_MUD_MONTH)

#define PULSE_PER_REAL_SEC      4
#define PULSE_PER_MUD_HOUR      (PULSE_PER_REAL_SEC * SECS_PER_MUD_HOUR)

#define PULSE_ZONE              PULSE_PER_MUD_HOUR
#define PULSE_MOBILE            10
#define PULSE_DRAIN_LIFE        11
#define PULSE_VIOLENCE          12
#define PULSE_SOUND             15
#define PULSE_AUCTION           (PULSE_PER_REAL_SEC * 20)
