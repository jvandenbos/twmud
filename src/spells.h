
#define MAX_BUF_LENGTH              240
#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3 
#define SPELL_BLINDNESS               4 
#define SPELL_BURNING_HANDS           5 
#define SPELL_CALL_LIGHTNING          6 
#define SPELL_CHARM	              7
#define CHARM_COST(vict, cast) ((GetMaxLevel(vict) + 2 )/3)
#define SPELL_CHILL_TOUCH             8 
#define SPELL_CLONE                   9 
#define SPELL_CHAIN_ELECTROCUTION    10
#define SPELL_CONTROL_WEATHER        11 
#define SPELL_CREATE_FOOD            12 
#define SPELL_CREATE_WATER           13 
#define SPELL_CURE_BLIND             14 
#define SPELL_CURE_CRITIC            15 
#define SPELL_CURE_LIGHT             16 
#define SPELL_CURSE                  17 
#define SPELL_FIRE_WIND              18
#define SPELL_DETECT_INVISIBLE       19 
#define SPELL_FROST_CLOUD            20
#define SPELL_DETECT_POISON          21 
#define SPELL_HARMFUL_TOUCH          22
#define SPELL_EARTHQUAKE             23 
#define SPELL_ENCHANT_WEAPON         24 
#define SPELL_ENERGY_DRAIN           25 
#define SPELL_FIREBALL               26 
#define SPELL_RUPTURE                27
#define SPELL_HEAL                   28 
#define SPELL_INVISIBLE              29 
#define SPELL_IMPLODE                30
#define SPELL_LOCATE_OBJECT          31 
#define SPELL_ACID_RAIN              32
#define SPELL_POISON                 33 
#define SPELL_POISON_GAS             34
#define SPELL_REMOVE_CURSE           35 
#define SPELL_SANCTUARY              36 
#define SPELL_SHOCKING_GRASP         37 
#define SPELL_SLEEP                  38 
#define SPELL_STRENGTH               39 
#define SPELL_SUMMON                 40 
#define SPELL_ELECTROCUTE            41
#define SPELL_WORD_OF_RECALL         42 
#define SPELL_REMOVE_POISON          43 
#define SPELL_SENSE_LIFE             44 

#define SKILL_SNEAK                  45 
#define SKILL_HIDE                   46 
#define SKILL_STEAL                  47 
#define SKILL_BACKSTAB               48 
#define SKILL_PICK_LOCK              49 
#define SKILL_KICK                   50 
#define SKILL_BASH                   51 
#define SKILL_RESCUE                 52

#define SPELL_IDENTIFY               53
#define SPELL_INFRAVISION            54
#define SPELL_GEYSER		     55
#define SPELL_REGEN		     56
#define SPELL_FLAMESTRIKE            57
#define SPELL_ELECTRIC_FIRE          58
#define SPELL_WEAKNESS               59
#define SPELL_DISPEL_MAGIC           60
#define SPELL_KNOCK                  61
#define SPELL_SENSE_AURA             62
#define SPELL_ANIMATE_DEAD           63
#define SPELL_PARALYSIS              64
#define SPELL_REMOVE_PARALYSIS       65
#define SPELL_FEAR                   66
#define SPELL_ACID_BLAST             67
#define SPELL_WATER_BREATH           68
#define SPELL_FLY                    69
#define SPELL_WITHER                 70
#define SPELL_WIND_STORM             71
#define SPELL_ICE_STORM              72
#define SPELL_SHIELD                 73
#define SPELL_VAMPYRIC_TOUCH         74
#define SPELL_EMPATHIC_HEAL          75
#define SPELL_MONSUM                 76
#define SPELL_HEAT_METAL             77
#define SPELL_ROT                    78
#define SPELL_PETRIFY                79
#define SPELL_CAMOUFLAGE             80
#define SPELL_FIRESHIELD             81
#define SPELL_LIFE_DRAIN             82
#define SPELL_CURE_SERIOUS           83
#define SPELL_WARD_FIRE              84
#define SPELL_REFRESH                85
#define SPELL_WARD_COLD              86
#define SPELL_TURN                   87
#define SPELL_SUCCOR                 88
#define SPELL_LIGHT                  89
#define SPELL_TRUE_SEEING_GROUP      90
#define SPELL_CALM                   91
#define SPELL_STONE_SKIN             92
#define SPELL_CONJURE_ELEMENTAL      93
#define SPELL_TRUE_SIGHT             94
#define SPELL_MINOR_CREATE           95
#define SPELL_FAERIE_FIRE            96
#define SPELL_FAERIE_FOG             97
#define SPELL_POSSESSION             98
#define SPELL_POLY_SELF              99
#define SPELL_MANA                  100
#define SPELL_ASTRAL_WALK           101
#define SPELL_RESURRECTION          102
#define SPELL_H_FEAST               103
#define SPELL_FLY_GROUP             104
#define SPELL_WARD_ELEC             105
#define SPELL_WEB                   106
#define SPELL_MINOR_TRACK           107
#define SPELL_MAJOR_TRACK           108

#define SPELL_DISINTEGRATE          109
#define SPELL_LAVA_STORM            110
#define SPELL_KNOWLEDGE             111
#define SPELL_ARMOR_GROUP           112
#define SPELL_DINVIS_GROUP          113
#define SPELL_INVIS_GROUP           114
#define SPELL_CURE_LIGHT_GROUP      115
#define SPELL_GOLEM                 116
#define SPELL_MOUNT                 117

#define SPELL_WATERBREATH_GROUP     118
#define SPELL_RECALL_GROUP          119

#define SPELL_SUNRAY        120
#define SPELL_WINDWALK      121
#define SPELL_MOONBEAM      122
#define SPELL_GOODBERRY     123
#define SPELL_TREE          124
#define SPELL_ANSUM         125
#define SPELL_THORN         126
#define SPELL_CREEPING_DOOM 127
#define SPELL_VINE          128
#define NATURE_WALK         129

#define SKILL_HYPNOSIS      130
#define SKILL_MEDITATE      131
#define SKILL_SCRY          132
#define SKILL_ADRENALIZE    133
#define SKILL_GREATER_GATE  134
#define SKILL_THROW         135   /* thief throw, psi thought throw is 160 */
#define SKILL_INVIS         136
#define SKILL_LESSER_GATE   137
#define SKILL_CANIBALIZE    138
#define SKILL_ILLUSIONARY_SHROUD  139
#define SKILL_PHANTASMAL    140
#define SPELL_WARD_ENERGY   141
#define SKILL_GREAT_SIGHT   142
#define SKILL_SPELL_SHIELD  143

#define SKILL_BREW          144
#define SKILL_TAN           145
#define SKILL_PSIONIC_BLAST 146
 
#define SKILL_LAY_ON_HANDS  147
#define SKILL_HOLY_WARCRY   148
#define SKILL_BLESSING      149
#define SKILL_HEROIC_RESCUE 150

#define SKILL_THRUST        151
#define SKILL_RATION        152
#define SKILL_ANIMAL_FRIENDSHIP 153
#define SKILL_STUN          154
#define SKILL_RIDE          155
#define SKILL_DOORBASH      156

#define SKILL_DRAIN_MANA    157
#define SKILL_LEVITATE      158
#define SKILL_COMBUSTION    159
#define SKILL_THOUGHT_THROW 160
#define SKILL_CONSTRICT     161
#define SKILL_RETREAT       162
#define SPELL_WARD_ACID     163
#define SPELL_STONE_FIST    164
#define SKILL_FORM          165
#define SKILL_CHAMELEON     166
#define SKILL_MELT          167
#define SKILL_REGEN         168
#define SKILL_SHIFT         169
#define SKILL_BIND          170
#define SKILL_CONTRACT      171
#define SKILL_PLATE         172
#define SKILL_BRAINSTORM    173

#define SKILL_APPRAISE      174
#define SKILL_FEINT         175
#define SKILL_LIMB	    176
#define SKILL_PALM	    177
#define SKILL_SAMPLE	    178

#define SKILL_ARCHERY                179
#define SKILL_HUNT                   180
/* #define SKILL_FIND_TRAP              181 */ 
/* #define SKILL_SET_TRAP               182 */
#define SKILL_DISARM                 183
/* #define SKILL_READ_MAGIC             184 */
#define SKILL_PUSH                   185
#define SKILL_DODGE                  186
#define SKILL_BERSERK                187

#define SKILL_DUAL_WIELD	     188
#define SKILL_QUICK_DRAW	     189
#define SKILL_TRIP		     190
#define SKILL_CIRCLE		     191
#define SKILL_SEARCH		     192
#define SKILL_MELEE1                 193
#define SKILL_MELEE2                 194
#define SKILL_MELEE3                 195
#define SPELL_CONT_LIGHT             196
#define SPELL_CONT_DARK              197
#define SKILL_MELEE4                 198
#define SPELL_ASTRAL_GROUP	     199
#define SPELL_RAY_OF_PURIFICATION    200
#define SKILL_WINGS                  201
#define SKILL_GILLS                  202
#define SKILL_TRACE                  203
#define SPELL_LULLABYE		     204
#define SPELL_SILENCE		     205
#define SPELL_TERROR		     206
#define SPELL_BLUR		     208
#define SPELL_FRIENDS		     209
#define SKILL_SUGGESTION	     210
#define SPELL_INSPIRE		     211
#define SPELL_SLOW		     212
#define SPELL_DESPAIR		     213
#define SPELL_HASTE		     214
#define SKILL_MELEE5                 215 
#define SKILL_MELEE6                 216
#define SKILL_MELEE7                 217
#define SKILL_MELEE8                 218
#define SKILL_MELEE9                 219
#define SKILL_MELEE10                220
#define SKILL_MELEE11                221
#define SPELL_ELECSHIELD             222
#define SPELL_COLDSHIELD             223
#define SPELL_POISONSHIELD           224
#define SPELL_ENERGYSHIELD           225
#define SKILL_VAMPSHIELD             226
#define SKILL_MANASHIELD             227
#define SKILL_RAGE                   228
#define SKILL_ROUGHNESS              229
#define SKILL_RESISTANCE             230
#define SPELL_ACIDSHIELD             231
#define SKILL_FLAIL                  325
#define SKILL_DIVERT                 326
#define SKILL_GOUGE                  327
#define SKILL_BLIND_FIGHTING         328
#define SKILL_TOLERANCE              329
#define SKILL_EGO_WHIP               330
#define SKILL_AWE                    331
#define SKILL_SHIELD_PUNCH           332
#define SKILL_SHIELD_BLOCK           333
#define SPELL_UNWEAVE                334
#define SKILL_EXPLODE		     335
#define SPELL_MOVESHIELD	     336
#define SKILL_PULSE		     337
#define SKILL_AURA		     338
#define SKILL_FIRE_AURA		     339
#define SKILL_ICE_AURA		     340
#define SKILL_ACID_AURA		     341
#define SKILL_SET_TRAP		     342
#define SPELL_ENFOREST		     343
#define SKILL_CHAOS_HAMMER           344
#define SKILL_PROBE	             345
#define SKILL_PINCH		     346
#define SPELL_HEAL_GROUP	     347
#define SKILL_DECOMPOSE_AURA	     348
#define SPELL_HARDEN_WEAPON          349
#define SPELL_GUST                   350
#define SKILL_GROUP_ATTACK           351
#define SKILL_CHARGE_ELEMENTS        352
#define SKILL_MONK_KICK              353
#define SPELL_INFERNO                354
#define SKILL_BALANCE                355
#define SKILL_TAUNT		     356
#define SKILL_PROTECT_AURA	     357
#define SKILL_LIGHT_AURA	     358
#define SKILL_REFLECT_AURA	     359
#define SKILL_NOVA_AURA		     360
#define SKILL_FURY		     361 // 02/04/05 by mtr

#define SPELL_POTION_FIRE_BREATH       361
#define SPELL_POTION_POISON_GAS_BREATH 362
#define SPELL_POTION_FROST_BREATH      363
#define SPELL_POTION_ACID_BREATH       364
#define SPELL_POTION_LIGHTNING_BREATH  365

/* No player skills from 490 on
   from here down doesn't get stored in player stuff or other places
   because they're above MAX_SPELLS */

#define FIRST_BREATH_WEAPON	     MAX_SKILLS + 0
#define SPELL_FIRE_BREATH            MAX_SKILLS + 1
#define SPELL_POISON_GAS_BREATH      MAX_SKILLS + 2
#define SPELL_FROST_BREATH           MAX_SKILLS + 3
#define SPELL_ACID_BREATH            MAX_SKILLS + 4
#define SPELL_LIGHTNING_BREATH       MAX_SKILLS + 5
#define LAST_BREATH_WEAPON	     MAX_SKILLS + 6

#define TYPE_HIT                     MAX_SKILLS + 7
#define TYPE_BLUDGEON                MAX_SKILLS + 8
#define TYPE_PIERCE                  MAX_SKILLS + 9
#define TYPE_SLASH                   MAX_SKILLS + 10
#define TYPE_WHIP                    MAX_SKILLS + 11
#define TYPE_CLAW                    MAX_SKILLS + 12
#define TYPE_BITE                    MAX_SKILLS + 13
#define TYPE_STING                   MAX_SKILLS + 14
#define TYPE_CRUSH                   MAX_SKILLS + 15
#define TYPE_CLEAVE                  MAX_SKILLS + 16
#define TYPE_STAB                    MAX_SKILLS + 17
#define TYPE_SMASH                   MAX_SKILLS + 18
#define TYPE_SMITE                   MAX_SKILLS + 19
#define TYPE_RANGEWEAPON             MAX_SKILLS + 20
#define TYPE_SUFFERING               MAX_SKILLS + 21


/* More anything but spells and weapontypes can be insterted here! */


#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST	490


#define TAR_IGNORE	 (1<< 0)
#define TAR_CHAR_ROOM	 (1<< 1)
#define TAR_CHAR_WORLD	 (1<< 2)
#define TAR_DEFAULT_SELF (1<< 3)
#define TAR_FIGHT_VICT	 (1<< 4)
#define TAR_SELF_ONLY	 (1<< 5)  /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO	 (1<< 6)  /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV	 (1<< 7)
#define TAR_OBJ_ROOM	 (1<< 8)
#define TAR_OBJ_WORLD	 (1<< 9)
#define TAR_OBJ_EQUIP	 (1<<10)
#define TAR_NAME	 (1<<11)
#define TAR_VIOLENT	 (1<<12)
#define TAR_ROOM	 (1<<13)  /* spells which target the room  */
#define TAR_SKILL	 (1<<14)  /* spell is really a skill in disguise... */
#define TAR_AREA	 (1<<15)  /* spell is area affect */
#define TAR_NOTEACH	 (1<<16)  /* not taught by normal guildmasters */
#define TAR_PURE_CLASS   (1<<17)  /* can only be used by pure class */

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4

/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};


#define SPLMOD_GOOD	 (1 << 0)
#define SPLMOD_EVIL	 (1 << 1)
#define SPLMOD_STR	 (1 << 2)
#define SPLMOD_WIS	 (1 << 3)
#define SPLMOD_INT	 (1 << 4)
#define SPLMOD_DEX	 (1 << 5)
#define SPLMOD_CON	 (1 << 6)
#define SPLMOD_CHA	 (1 << 7)
#define SPLMOD_FEMALE	 (1 << 8)
#define SPLMOD_MALE	 (1 << 9)
#define SPLMOD_HUMAN	 (1 << 10)
#define SPLMOD_DWARF	 (1 << 11)
#define SPLMOD_ELF	 (1 << 12)
#define SPLMOD_HOBBIT	 (1 << 13)
#define SPLMOD_GNOME	 (1 << 14)
#define SPLMOD_FELIS	 (1 << 15)
#define SPLMOD_CANIS	 (1 << 16)
#define SPLMOD_DOPPLE	 (1 << 17)
#define SPLMOD_AC 	 (1 << 18)
#define SPLMOD_SPELLSAVE (1 << 19)

// used for SkillChance check for the base modifier. 
// The skill should have one of three starting points
// and most should be at avg and then modify from there
#define SUCCESS_AVG 50
#define SUCCESS_LOW 25
#define SUCCESS_HI 75
#define SUCCESS_MAX 100
