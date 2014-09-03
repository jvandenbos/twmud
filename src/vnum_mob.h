#ifndef VNUM_MOB_H

/* ************************************************************************
*  FILE: vnum_mob.h               Copyright(c) Jonathan Monteleone 8/94   *
*  USAGE: Indexing ALL monster VNUMs in the source code.                  *
************************************************************************* */

/* This file contains a listing of ALL monster vnums */
/* not found in spec_assign.c. */

#define VMOB_FIRE  10  /* conjure */
#define VMOB_WATER 11  /* conjure */
#define VMOB_EARTH 12  /* conjure */
#define VMOB_AIR   13  /* conjure */

#define VMOB_HITDM  20  /* hit point dm */
#define VMOB_MANADM 21  /* mana point dm */
#define VMOB_STRDM  22  /* str point dm */
#define VMOB_INTDM  23  /* int point dm */
#define VMOB_WISDM  24  /* wis point dm */
#define VMOB_DEXDM  25  /* dex point dm */
#define VMOB_CONDM  26  /* con point dm */
#define VMOB_HUNDM  27  /* hunger dm */
#define VMOB_THRDM  28  /* thirst dm */

#define VMOB_73  30   /* high mage gm */
#define VMOB_74  31   /* high cleric gm */
#define VMOB_75  32   /* high thief gm */
#define VMOB_76  33   /* high warrior gm */
#define VMOB_77  34   /* mid mage gm */
#define VMOB_78  35   /* mid cleric gm */
#define VMOB_79  36   /* mid thief gm */
#define VMOB_80  37   /* mid warrior gm */
#define VMOB_81  38   /* low druid gm */
#define VMOB_82  39   /* low paladin gm */
#define VMOB_83  40   /* low psionist gm */
#define VMOB_84  41   /* low ranger gm */
#define VMOB_85  42   /* mid druid gm */
#define VMOB_86  43   /* mid ranger gm */
#define VMOB_87  44   /* mid paladin gm */
#define VMOB_88  45   /* mid psionist gm */
#define VMOB_89  46   /* high ranger gm */
#define VMOB_90  47   /* high paladin gm */
#define VMOB_91  48   /* high druid gm */
#define VMOB_92  49   /* high psionist gm */
#define VMOB_131 29   /* high bard gm */
#define VMOB_155 8049 /* high monk gm */

#define VMOB_1   50      /* polymorph Giant Rat               */
#define VMOB_2   51      /* polymorph Kobold Peon             */
#define VMOB_3   52      /* polymorph Rock Toad               */
#define VMOB_4   53      /* polymorph Pixie Guard             */
#define VMOB_5   54      /* polymorph Fire Beetle             */
#define VMOB_6   55      /* polymorph Goblin Worker           */
#define VMOB_7   56      /* polymorph Moccasin Snake          */
#define VMOB_8   57      /* polymorph Brownie Knight          */
#define VMOB_9   58      /* polymorph Cougar                  */
#define VMOB_10  59      /* polymorph Orc Raider              */
#define VMOB_11  60      /* polymorph Wolf                    */
#define VMOB_12  61      /* polymorph Ogre Henchman           */
#define VMOB_13  62      /* polymorph Lion                    */
#define VMOB_14  63      /* polymorph Lizardman Warrior       */
#define VMOB_15  64      /* polymorph Warg                    */
#define VMOB_16  65      /* polymorph Troll Berserker         */
#define VMOB_17  66      /* polymorph Crystal Spider          */
#define VMOB_18  67      /* polymorph Hill Giant Smasher      */
#define VMOB_19  68      /* polymorph Shadow Beast            */
#define VMOB_20  69      /* polymorph Quickling Raider        */
#define VMOB_21  70      /* polymorph Gorgon                  */
#define VMOB_22  71      /* polymorph Githyanki Soldier       */
#define VMOB_23  72      /* polymorph Otyugh                  */
#define VMOB_24  73      /* polymorph Shadow Warrior          */
#define VMOB_25  74      /* polymorph Green Dragon            */
#define VMOB_26  75      /* polymorph Troglodyte BattleMaster */
#define VMOB_27  76      /* polymorph Lesser Xorn             */
#define VMOB_28  77      /* polymorph Great Cyclops           */
#define VMOB_29  78      /* polymorph Frost Salamander        */
#define VMOB_30  79      /* polymorph Draconian Sentry        */
#define VMOB_31  80      /* polymorph Slatherer               */
#define VMOB_32  81      /* polymorph Storm Giant             */
#define VMOB_33  82      /* polymorph Tiger Shark             */
#define VMOB_34  83      /* polymorph Minotaur Scout          */
#define VMOB_35  84      /* polymorph Centaur Shaman          */
#define VMOB_36  85      /* polymorph Tytan Hero              */
#define VMOB_37  86      /* polymorph Black Dragon            */
#define VMOB_38  87      /* polymorph Vampire                 */
#define VMOB_39  88      /* polymorph Vicious Roc             */
#define VMOB_40  89      /* polymorph Shadowen                */
#define VMOB_41  90      /* polymorph Phoenix                 */
#define VMOB_42  91      /* polymorph Master Mindflayer       */
#define VMOB_43  92      /* polymorph Beholder                */
#define VMOB_44  93      /* polymorph Githezai Healer         */
#define VMOB_45  94      /* polymorph Elfeater                */
#define VMOB_46  95      /* polymorph Orc Hero                */
#define VMOB_133  96     /* polymorph Towering Treant         */
#define VMOB_134  97     /* polymorph Beysib Fighter          */
#define VMOB_135  98     /* polymorph Efreet Lord             */
#define VMOB_136  99     /* polymorph Ethereal Knight         */
#define VMOB_137  100    /* polymorph Wyvern                  */
#define VMOB_138  101    /* polymorph Drow SpellSword         */
#define VMOB_139  102    /* polymorph Kraken                  */
#define VMOB_140  103    /* polymorph Demon Magi              */
#define VMOB_141  104    /* polymorph Tangle Tree             */
#define VMOB_142  105    /* polymorph Skaven General          */
#define VMOB_143  106    /* polymorph Gorgimera               */
#define VMOB_144  107    /* polymorph Succubus                */
#define VMOB_145  108    /* polymorph Hydra                   */
#define VMOB_146  132    /* polymorph Planetar Defender       */
#define VMOB_147  133    /* polymorph Silver Unicorn          */
#define VMOB_148  134    /* polymorph Sahuagin Witch          */
#define VMOB_149  135    /* polymorph Behemoth                */
#define VMOB_150  136    /* polymorph Solar Lawgiver          */
#define VMOB_151  137    /* polymorph Leviathan               */
#define VMOB_152  138    /* polymorph Avatar                  */

#define VMOB_ZOM_PET 109  /* animate dead */

#define VMOB_48  300    /* monsum goblin      */
#define VMOB_49  301    /* monsum orc         */
#define VMOB_50  302    /* monsum kobold      */
#define VMOB_51  303    /* monsum bugbear     */
#define VMOB_52  304    /* monsum cockatrice  */
#define VMOB_53  305    /* monsum griffin     */
#define VMOB_54  306    /* monsum behir       */

#define VMOB_POSS_PET 666 /* possession */

#define VMOB_EROK  699  /* erok */

#define VMOB_121   1499   /* sisyphus */

#define VMOB_122   3020   /* low mage gm */
#define VMOB_123   3021   /* low cleric gm */
#define VMOB_124   3022   /* low thief gm */
#define VMOB_125   3023   /* low warrior gm */

#define VMOB_97    3024   /* low mage guildguard */
#define VMOB_98    3025   /* low cleric guildguard */
#define VMOB_99    3026   /* low thief guildguard */
#define VMOB_100   3027   /* low warrior guildguard */
#define VMOB_101   3028   /* low paladin guildguard */
#define VMOB_102   3029   /* los low ranger guildguard */
#define VMOB_103   3030   /* low druid guildguard */
#define VMOB_104   3031   /* low psionist guildguard */
#define VMOB_132   3032   /* Inglebert guildguard*/

#define VMOB_70  3060     /* IS_POLICE = cityguard */
#define VMOB_71  3067     /* IS_POLICE = guard royal */
#define VMOB_72  3069     /* IS_POLICE = cityguard */
#define VMOB_93  3661     /* sultan guard */
#define VMOB_94  3682     /* sultan royal guard */

#define VMOB_FIRST_POOL 7600  /* first travel pool mob */

#define VMOB_55  10760   /* bounty hunter terminator */
#define VMOB_56  12010   /* bounty hunter */
#define VMOB_57  12011   /* bounty hunter */
#define VMOB_58  12012   /* bounty hunter */
#define VMOB_59  12013   /* bounty hunter */
#define VMOB_60  12014   /* bounty hunter */

#define VMOB_129  13757  /* cerebus */

#define VMOB_105  18206   /* guildguard mid paladin */
#define VMOB_106  18207   /* guildguard mid ranger */
#define VMOB_107  18208   /* guildguard mid druid */
#define VMOB_108  18209   /* guildguard mid psionist */
#define VMOB_109  18210   /* guildguard mid mage */
#define VMOB_110  18211   /* guildguard mid cleric */
#define VMOB_111  18212   /* guildguard mid thief */
#define VMOB_112  18213   /* guildguard mid warrior */

#define VMOB_95  21114   /* troglodyte guard */
#define VMOB_96  21118   /* troglodyte clansman */

#define VMOB_DEITY 21124             /* UNKNOWN */
#define VMOB_GhostSoldier    21138   /* UNKNOWN */
#define VMOB_GhostLieutenant 21139   /* UNKNOWN */

#define VMOB_130  25017   /* gatekeeper */

#define VMOB_113  25139   /* guildguard high mage */
#define VMOB_114  25140   /* guildguard high warrior */
#define VMOB_115  25141   /* guildguard high thief */
#define VMOB_116  25142   /* guildguard high cleric */
#define VMOB_117  25143   /* guildguard high paladin */
#define VMOB_118  25144   /* guildguard high ranger */
#define VMOB_119  25145   /* guildguard high druid */
#define VMOB_120  25146   /* guildguard high psionist */

#define VMOB_61  29000    /* ansum fox     */
#define VMOB_62  29001    /* ansum owl     */
#define VMOB_63  29002    /* ansum stag    */
#define VMOB_64  29003    /* ansum cougar  */
#define VMOB_65  29004    /* ansum grizzly */
#define VMOB_66  29005    /* ansum treant  */
#define VMOB_67  29006    /* ansum giant   */

#define VMOB_MOB_KILL   29600  /* phantasmal killer */
#define VMOB_GOLEM      29604  /* golem spell */
#define VMOB_DRUID_TREE 29606  /* tree skill */

#define VMOB_MNT_ONE  29610  /* mount skill */
#define VMOB_MNT_GOOD 29614  /* mount skill */
#define VMOB_MNT_EVIL 29615  /* mount skill */
#define VMOB_MNT_NEUT 29616  /* mount skill */

#define VMOB_126   29800  /* low shifter gm */
#define VMOB_127   29801  /* mid shifter gm */
#define VMOB_128   29802  /* high shifter */

#define VMOB_GATE_MOB 29900    /* gate spell */

#define VMOB_68  29901    /* melt skill */
#define VMOB_69  29902    /* cocoon skill */

#define VMOB_FALCONIAN 3268 /*Falconian Guard*/
#define VMOB_32200 32200 /*Alkian Guardian*/
#endif
