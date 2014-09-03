#include <string.h>
#include <stdlib.h>
#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "spells.h"
#include "db.h"
#include "db.random.h"

void add_tree(int parent, int child, int per, int type, int lvl);
void add_node(int parent, int per, int rand);
random_tree *find_tree_parent(random_tree *p, int parent);
random_node_type *get_random(random_tree *p, int type, int lvl);

struct random_tree all_randoms;
long random_count;

void setup_random_tree() {
   memset(&all_randoms, 0, sizeof(all_randoms));

   //===TREES===

   add_tree(NULL,     RT_AFFECTS,   75,  RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_AFFECTS2,  25,  RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_CHAOS,     80,  RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_STATS,     300, RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_CLASSES,   200, RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_MIXED_AFF, 75,  RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_SHIELDS,   10,  RAND_GROUP_GENERAL, 25);
   add_tree(NULL,     RT_SIGHT_AFF, 100, RAND_GROUP_GENERAL, 0);
   add_tree(NULL,     RT_WS,        100, RAND_GROUP_WEAPONS, 0);

   add_tree(RT_CHAOS, RT_CHAOS_SUS, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_CHAOS, RT_CHAOS_RES, 50,  RAND_GROUP_GENERAL, 0);
   add_tree(RT_CHAOS, RT_CHAOS_IMM, 10,  RAND_GROUP_GENERAL, 40);
   add_tree(RT_CHAOS, RT_UBER,      5,   RAND_GROUP_GENERAL, 30);

   add_tree(RT_STATS, RT_STATS_STR, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_INT, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_WIS, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_DEX, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_CON, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_CHA, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_AGE, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_DAM, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_HIT, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_HND, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_AC,  100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_HP,  100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_MP,  100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_STATS, RT_STATS_MOV, 100, RAND_GROUP_GENERAL, 0);

   add_tree(RT_CLASSES, RT_CLASSES_WAR,    20, RAND_GROUP_GENERAL, 0);
   add_tree(RT_CLASSES, RT_CLASSES_THIEF, 100, RAND_GROUP_GENERAL, 0);
   add_tree(RT_CLASSES, RT_CLASSES_RANG,   25, RAND_GROUP_GENERAL, 0);
   add_tree(RT_CLASSES, RT_CLASSES_SHIFT,  20, RAND_GROUP_GENERAL, 0);
   add_tree(RT_CLASSES, RT_CLASSES_DRUID,  10, RAND_GROUP_GENERAL, 0);

   add_tree(RT_WS,      RT_WS_ACID,        20, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_COLD,        30, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_DRAIN,       40, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_ELEC,        50, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_ENERGY,      50, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_FIRE,        50, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_NATURE,      90, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_POISON,      20, RAND_GROUP_WEAPONS, 0);
   add_tree(RT_WS,      RT_WS_MISC,        80, RAND_GROUP_WEAPONS, 0);

   //===NODES===
   add_node(RT_AFFECTS, 100, RN_TROLLS);
   add_node(RT_AFFECTS, 100, RN_PROPHETS);
   add_node(RT_AFFECTS, 75,  RN_WRETCHED);
   add_node(RT_AFFECTS, 100, RN_DISEASE);
   add_node(RT_AFFECTS, 50,  RN_ANGER);
   add_node(RT_AFFECTS, 50,  RN_EVASION);
   add_node(RT_AFFECTS, 100, RN_LAKE);
   add_node(RT_AFFECTS, 100, RN_NARCOLEPSY);
   add_node(RT_AFFECTS, 200, RN_LEVITATION);
   add_node(RT_AFFECTS, 50,  RN_SHADOWS);
   add_node(RT_AFFECTS, 25,  RN_MYSTERY);
   add_node(RT_AFFECTS, 150, RN_INVISIBLE);
   add_node(RT_AFFECTS, 75,  RN_BLUR);
   add_node(RT_AFFECTS, 125, RN_HARMONY);
   add_node(RT_AFFECTS, 50,  RN_GODS);

   add_node(RT_AFFECTS2, 75,  RN_TEPHANIS);
   add_node(RT_AFFECTS2, 100, RN_MAHNTOR);
   add_node(RT_AFFECTS2, 100, RN_GLOOM);

   add_node(RT_CHAOS_SUS, 25,  RN_CHAOS_S_FIRE);
   add_node(RT_CHAOS_SUS, 50,  RN_CHAOS_S_COLD);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_ELEC);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_ENERGY);
   add_node(RT_CHAOS_SUS, 200, RN_CHAOS_S_BLUNT);
   add_node(RT_CHAOS_SUS, 125, RN_CHAOS_S_PIERCE);
   add_node(RT_CHAOS_SUS, 75,  RN_CHAOS_S_SLASH);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_ACID);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_POISON);
   add_node(RT_CHAOS_SUS, 75,  RN_CHAOS_S_DRAIN);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_SLEEP);
   add_node(RT_CHAOS_SUS, 125, RN_CHAOS_S_CHARM);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_NONMAG);
   add_node(RT_CHAOS_SUS, 50,  RN_CHAOS_S_HOLD);
   add_node(RT_CHAOS_SUS, 100, RN_CHAOS_S_BARD);

   add_node(RT_CHAOS_RES, 75,  RN_CHAOS_R_FIRE);
   add_node(RT_CHAOS_RES, 50,  RN_CHAOS_R_COLD);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_ELEC);
   add_node(RT_CHAOS_RES, 200, RN_CHAOS_R_ENERGY);
   // Solaar: Reinstated RES:attacks but very rare
   add_node(RT_CHAOS_RES, 10,  RN_CHAOS_R_BLUNT);
   add_node(RT_CHAOS_RES, 10,  RN_CHAOS_R_PIERCE);
   add_node(RT_CHAOS_RES, 10,  RN_CHAOS_R_SLASH);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_ACID);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_POISON);
   add_node(RT_CHAOS_RES, 50,  RN_CHAOS_R_DRAIN);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_SLEEP);
   add_node(RT_CHAOS_RES, 125, RN_CHAOS_R_CHARM);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_NONMAG);
   add_node(RT_CHAOS_RES, 50,  RN_CHAOS_R_HOLD);
   add_node(RT_CHAOS_RES, 100, RN_CHAOS_R_BARD);

   add_node(RT_CHAOS_IMM, 100, RN_CHAOS_I_FIRE);
   add_node(RT_CHAOS_IMM, 150, RN_CHAOS_I_ELEC);
   add_node(RT_CHAOS_IMM,  20, RN_CHAOS_I_DRAIN);
   add_node(RT_CHAOS_IMM,  75, RN_CHAOS_I_COLD);
   //add_node(RT_CHAOS_IMM,   5, RN_CHAOS_I_BLUNT);
   //add_node(RT_CHAOS_IMM,   5, RN_CHAOS_I_SLASH);
   //add_node(RT_CHAOS_IMM,   5, RN_CHAOS_I_PIERCE);

   add_node(RT_STATS_STR, 150, RN_WEAKNESS);
   add_node(RT_STATS_STR, 100, RN_MIGHT);
   add_node(RT_STATS_STR, 50,  RN_STRENGTH);
   add_node(RT_STATS_STR, 25,  RN_POWER);

   add_node(RT_STATS_INT, 150, RN_FOOLS);
   add_node(RT_STATS_INT, 100, RN_LEARNING);
   add_node(RT_STATS_INT, 50,  RN_INTELLIGENCE);
   add_node(RT_STATS_INT, 25,  RN_KNOWLEDGE);

   add_node(RT_STATS_WIS, 150, RN_IGNORANCE);
   add_node(RT_STATS_WIS, 100, RN_DISCRETION);
   add_node(RT_STATS_WIS, 50,  RN_WISDOM);
   add_node(RT_STATS_WIS, 25,  RN_JUDGEMENT);

   add_node(RT_STATS_DEX, 150, RN_FUMBLING);
   add_node(RT_STATS_DEX, 100, RN_SKILL);
   add_node(RT_STATS_DEX, 50,  RN_DEXTERITY);
   add_node(RT_STATS_DEX, 25,  RN_AGILITY);

   add_node(RT_STATS_CON, 150, RN_ILLNESS);
   add_node(RT_STATS_CON, 100, RN_STAMINA);
   add_node(RT_STATS_CON, 50,  RN_CONSTITUTION);
   add_node(RT_STATS_CON, 25,  RN_ENDURANCE);

   add_node(RT_STATS_CHA, 150, RN_MONSTROSITY);
   add_node(RT_STATS_CHA, 100, RN_ATTRACTION);
   add_node(RT_STATS_CHA, 50,  RN_CHARISMA);
   add_node(RT_STATS_CHA, 25,  RN_BEAUTY);

   add_node(RT_STATS_AGE, 25,  RN_ETERNITY);
   add_node(RT_STATS_AGE, 50,  RN_LONGEVITY);
   add_node(RT_STATS_AGE, 100, RN_YOUTH);
   add_node(RT_STATS_AGE, 150, RN_DAYS);
   add_node(RT_STATS_AGE, 100, RN_YEARS);
   add_node(RT_STATS_AGE, 50,  RN_CENTURIES);
   add_node(RT_STATS_AGE, 25,  RN_MILLENIA);
   add_node(RT_STATS_AGE, 10,  RN_EONS);

   add_node(RT_STATS_DAM, 125, RN_PITY);
   add_node(RT_STATS_DAM, 100, RN_HARM);
   add_node(RT_STATS_DAM, 50,  RN_DAMAGE);
   add_node(RT_STATS_DAM, 25,  RN_PAIN);

   add_node(RT_STATS_HIT, 125, RN_MERCY);
   add_node(RT_STATS_HIT, 100, RN_AIM);
   add_node(RT_STATS_HIT, 50,  RN_PRECISION);
   add_node(RT_STATS_HIT, 25,  RN_ACCURACY);

   add_node(RT_STATS_HND, 150, RN_GUILT);
   add_node(RT_STATS_HND, 125, RN_DEATH);
   add_node(RT_STATS_HND, 100, RN_BLOOD);
   add_node(RT_STATS_HND, 75,  RN_TRAUMA);
   add_node(RT_STATS_HND, 50,  RN_GORE);
   add_node(RT_STATS_HND, 25,  RN_CARNAGE);

   add_node(RT_STATS_AC, 125, RN_VULNERABILITY);
   add_node(RT_STATS_AC, 100, RN_FORTIFICATION);
   add_node(RT_STATS_AC, 75,  RN_PROTECTION);
   add_node(RT_STATS_AC, 50,  RN_DEFENCE);

   add_node(RT_STATS_AC, 150, RN_FRAILTY);
   add_node(RT_STATS_AC, 100, RN_TOLERANCE);
   add_node(RT_STATS_AC, 50,  RN_RESISTANCE);

   add_node(RT_STATS_HP, 150, RN_SLAVES);
   add_node(RT_STATS_HP, 125, RN_VASSALS);
   add_node(RT_STATS_HP, 100, RN_FIGHTERS);
   add_node(RT_STATS_HP, 75,  RN_WARRIORS);
   add_node(RT_STATS_HP, 50,  RN_HEROES);
   add_node(RT_STATS_HP, 25,  RN_KINGS);

   add_node(RT_STATS_MP, 150, RN_IDIOTS);
   add_node(RT_STATS_MP, 125, RN_CRETINS);
   add_node(RT_STATS_MP, 100, RN_SAGES);
   add_node(RT_STATS_MP, 75,  RN_MAGES);
   add_node(RT_STATS_MP, 50,  RN_SORCERERS);
   add_node(RT_STATS_MP, 25,  RN_WIZARDS);

   add_node(RT_STATS_MOV, 150, RN_SLOTHS);
   add_node(RT_STATS_MOV, 125, RN_LOITERERS);
   add_node(RT_STATS_MOV, 100, RN_PILGRIMS);
   add_node(RT_STATS_MOV, 75,  RN_NOMADS);
   add_node(RT_STATS_MOV, 50,  RN_TRAVELERS);
   add_node(RT_STATS_MOV, 25,  RN_EXPLORERS);

   add_node(RT_CLASSES_WAR,   150, RN_BASHING);
   add_node(RT_CLASSES_WAR,   100, RN_RAM);
   add_node(RT_CLASSES_WAR,   50,  RN_RHINO);

   add_node(RT_CLASSES_THIEF, 100, RN_DECEIT);
   add_node(RT_CLASSES_THIEF, 50,  RN_SHADES);

   add_node(RT_CLASSES_THIEF, 100, RN_BANDITS);
   add_node(RT_CLASSES_THIEF, 50,  RN_ROBBERS);

   add_node(RT_CLASSES_THIEF, 100, RN_FILCHERS);
   add_node(RT_CLASSES_THIEF, 50,  RN_OUTLAWS);

   add_node(RT_CLASSES_THIEF, 100, RN_BRIGANDS);
   add_node(RT_CLASSES_THIEF, 50,  RN_STEALTH);

   add_node(RT_CLASSES_THIEF, 100, RN_PIERCING);
   add_node(RT_CLASSES_THIEF, 50,  RN_THIEVES);

   add_node(RT_CLASSES_RANG,  125, RN_LOST);
   add_node(RT_CLASSES_RANG,  100, RN_SCOUTS);
   add_node(RT_CLASSES_RANG,   75, RN_PURSUIT);
   add_node(RT_CLASSES_RANG,   50, RN_PREDATORS);

   add_node(RT_CLASSES_SHIFT, 150, RN_GLASSGRIP);
   add_node(RT_CLASSES_SHIFT, 130, RN_GOUT);
   add_node(RT_CLASSES_SHIFT, 110, RN_ARTHRITIS);
   add_node(RT_CLASSES_SHIFT,  90, RN_GRIP);
   add_node(RT_CLASSES_SHIFT,  70, RN_VISE);
   add_node(RT_CLASSES_SHIFT,  50, RN_IRONFIST);
   add_node(RT_CLASSES_SHIFT,  30, RN_BRASSCLAW);

   add_node(RT_CLASSES_DRUID, 100, RN_RIDING);
   add_node(RT_CLASSES_DRUID,  50, RN_JOUSTING);

   add_node(RT_MIXED_AFF,     100, RN_AVATAR);
   add_node(RT_MIXED_AFF,     100, RN_BALROG);
   add_node(RT_MIXED_AFF,     100, RN_ELFEATER);
   add_node(RT_MIXED_AFF,     100, RN_LICH);
   add_node(RT_MIXED_AFF,     100, RN_PHOENIX);
   add_node(RT_MIXED_AFF,     100, RN_ROC);
   add_node(RT_MIXED_AFF,     100, RN_DREAMMASTER);
   add_node(RT_MIXED_AFF,     100, RN_EFREETI);
   add_node(RT_MIXED_AFF,     100, RN_DEMONSPAWN);

#if 0
   add_node(RT_MIXED_AFF,     100, RN_DEADMEAT);
   add_node(RT_MIXED_AFF,     100, RN_IX);
   add_node(RT_MIXED_AFF,     100, RN_LEDEN);
   add_node(RT_MIXED_AFF,     100, RN_NOVAK);
   add_node(RT_MIXED_AFF,     100, RN_SOLAAR);
   add_node(RT_MIXED_AFF,     100, RN_QUILAN);
   add_node(RT_MIXED_AFF,     100, RN_DREUGH);
   add_node(RT_MIXED_AFF,     100, RN_TIMUS);
   add_node(RT_MIXED_AFF,     100, RN_TANGA);
   add_node(RT_MIXED_AFF,     100, RN_SLIM);
#endif

   add_node(RT_SHIELDS,       100, RN_FIRESHIELD);
   add_node(RT_SHIELDS,       100, RN_LIGHTNING);
   add_node(RT_SHIELDS,        25, RN_POISON);
   add_node(RT_SHIELDS,       100, RN_ENERGY);
   add_node(RT_SHIELDS,        20, RN_HATE);
   add_node(RT_SHIELDS,        30, RN_MAGIC);
   add_node(RT_SHIELDS,       100, RN_ACID);
   add_node(RT_SHIELDS,       100, RN_ICE);
   add_node(RT_SHIELDS,       100, RN_RADIATION);

   add_node(RT_SIGHT_AFF,     150, RN_BLINDNESS);
   add_node(RT_SIGHT_AFF,     100, RN_INSIGHT);
   add_node(RT_SIGHT_AFF,     100, RN_SIGHT);
   add_node(RT_SIGHT_AFF,      50, RN_VISION);
   add_node(RT_SIGHT_AFF,     100, RN_PERCEPTION);
   add_node(RT_SIGHT_AFF,     100, RN_CLAIRVOYANCE);
   add_node(RT_SIGHT_AFF,      40, RN_DIVINATION);
   add_node(RT_SIGHT_AFF,     100, RN_BLACKLIGHT);
   add_node(RT_SIGHT_AFF,     100, RN_BRILLIANCE);

   add_node(RT_WS_ACID,       100, RN_FIRESTAR);
   add_node(RT_WS_ACID,        50, RN_CTHULHU);

   add_node(RT_WS_COLD,       150, RN_COLD);
   add_node(RT_WS_COLD,       100, RN_FROST);
   add_node(RT_WS_COLD,        50, RN_BLIZZARDS);

   add_node(RT_WS_DRAIN,      100, RN_GHOULS);
   add_node(RT_WS_DRAIN,      150, RN_GHOSTS);
   add_node(RT_WS_DRAIN,       66, RN_GHASTS);
   add_node(RT_WS_DRAIN,       33, RN_VAMPIRES);

   add_node(RT_WS_ELEC,       150, RN_SHOCK);
   add_node(RT_WS_ELEC,       120, RN_THUNDER);
   add_node(RT_WS_ELEC,        90, RN_STATIC);
   add_node(RT_WS_ELEC,        60, RN_STORMS);
   add_node(RT_WS_ELEC,        30, RN_ZEUS);

   add_node(RT_WS_ENERGY,     150, RN_SUFFERING);
   add_node(RT_WS_ENERGY,     120, RN_ANGUISH);
   add_node(RT_WS_ENERGY,      90, RN_AGONY);
   add_node(RT_WS_ENERGY,      60, RN_TORMENT);
   add_node(RT_WS_ENERGY,      30, RN_TORTURE);

   add_node(RT_WS_FIRE,       150, RN_HEAT);
   add_node(RT_WS_FIRE,       120, RN_FIRE);
   add_node(RT_WS_FIRE,        90, RN_IMMOLATION);
   add_node(RT_WS_FIRE,        60, RN_INFERNO);
   add_node(RT_WS_FIRE,        30, RN_VOLCANOES);

   add_node(RT_WS_NATURE,     150, RN_ARACHNOS);
   add_node(RT_WS_NATURE,     135, RN_PARAMOR);
   add_node(RT_WS_NATURE,     120, RN_EXPOSURE);
   add_node(RT_WS_NATURE,     105, RN_THALIDS);
   add_node(RT_WS_NATURE,      90, RN_DRYADS);
   add_node(RT_WS_NATURE,      75, RN_HURRICANES);
   add_node(RT_WS_NATURE,      60, RN_XORN);
   add_node(RT_WS_NATURE,      45, RN_GEYSERS);
   add_node(RT_WS_NATURE,      30, RN_LOCUSTS);

   add_node(RT_WS_POISON,     100, RN_SERPENTS);
   add_node(RT_WS_POISON,      50, RN_ASPS);

   add_node(RT_WS_MISC,       100, RN_EXILE);
   add_node(RT_WS_MISC,        60, RN_THE_BAT);
   add_node(RT_WS_MISC,       150, RN_EMPATHY);
   add_node(RT_WS_MISC,       100, RN_SLUMBER);
   add_node(RT_WS_MISC,        30, RN_PURITY);
   add_node(RT_WS_MISC,        50, RN_BASILISKS);
   add_node(RT_WS_MISC,        80, RN_PHOBIAS);
   add_node(RT_WS_MISC,       100, RN_THE_HOLY);

   add_node(RT_UBER,           10, RN_RAINBOWS);
   add_node(RT_UBER,           20, RN_ALLSIGHT);
   add_node(RT_UBER,           40, RN_HIDING);
   add_node(RT_UBER,           60, RN_PROTECTOR);
   add_node(RT_UBER,           40, RN_EHEALTH);
   add_node(RT_UBER,           40, RN_EMANA);
   add_node(RT_UBER,           40, RN_ELIFE);
   add_node(RT_UBER,           60, RN_FLAWLESS);
   add_node(RT_UBER,           30, RN_SLAUGHTER);
   add_node(RT_UBER,           20, RN_RAGE);

   random_count = 0;
}

void add_tree(int parent, int child, int per, int type, int lvl) {
   random_tree *node=NULL, *childn=NULL;

   node = find_tree_parent(&all_randoms, parent);

   if(!node) {
      dlog("Random add_tree: Couldn't find a parent node for %d", parent);
      return;
   }

   if(node->num_nchild) {
      dlog("Random add_tree: Parent node %d has child nodes!", parent);
      return;
   }

   if(node->num_tchild >= 20) {
      dlog("Random add_tree: Parent node %d has too many children", parent);
      return;
   }

   CREATE(childn, random_tree, 1);
   node->tchild[node->num_tchild++] = childn;

   childn->num = child;
   childn->per = per;
   childn->type = type;
   childn->level = lvl;
}

void add_node(int parent, int per, int rand) {
   random_tree      *node=NULL;
   random_tree_node *childn=NULL;

   node = find_tree_parent(&all_randoms, parent);

   if(!node) {
      dlog("Random add_node: Couldn't find a parent node for %d", parent);
      return;
   }

   if(node->num_tchild) {
      dlog("Random add_node: Parent node %d is not an empty tree", parent);
      return;
   }

   if(node->num_nchild >= MAX_RAND_CHILD) {
      dlog("Random add_node: Parent node %d has too many children", parent);
      return;
   }

   CREATE(childn, random_tree_node, 1);
   node->nchild[node->num_nchild++] = childn;

   childn->num = rand;
   childn->per = per;
}

random_tree *find_tree_parent(random_tree *p, int parent) {
   int i;
   random_tree *node=NULL;

   if(!p) return NULL;
   if(!parent) return p;
   if(p->num == parent) return p;

   for(i=0,node=NULL; i<p->num_tchild; i++) {
      node = find_tree_parent(p->tchild[i], parent);
      if(node) return node;
   }

   return NULL;
}

random_node_type *get_random(random_tree *p, int type, int lvl) {
   int max, base;
   int i,choice,j;
   char buf[MAX_STRING_LENGTH];

   if(!IS_SET(RAND_GROUP_WEAPONS, type) &&
      !IS_SET(RAND_GROUP_GENERAL, type)) {
      return NULL;
   }

   max = 100000;   // max chance
   if(p->num_tchild) {
      base = max / (p->num_tchild * p->num_tchild);
   } else {
      base = max / (p->num_nchild * p->num_nchild);
   }

   //if tis to choose a sub-tree...
   if(p->num_tchild) {
      while(1) {
         i = number(0,p->num_tchild-1);
	 if(!p->tchild[i]) continue;
	 if(!IS_SET(p->tchild[i]->type, type)) continue;

	 if(number(0,max) < (base*p->tchild[i]->per)/100) {
	    if(p->tchild[i]->level <= lvl) {
	       return get_random(p->tchild[i], type, lvl);
	    }
	 }
      }
   } else {
      while(1) {
	 i = number(0,p->num_nchild-1);
	 if(!p->nchild[i]) continue;

	 if(number(0,max) < (base*p->nchild[i]->per)/100) {
	    choice=i;
	    j=0;
	    for(i=0;random_nodes[i].num;i++) {
	       if(random_nodes[i].num == p->nchild[choice]->num) {
		  if(random_nodes[i].minlevel <= lvl) {
		     return &random_nodes[i];
		  }
		  j=1;
	       }
	    }
	    if(!j) {
	       dlog("Couldn't find an entry in the db for %d!", p->nchild[choice]->num);
	       break;
	    }
	 }
      }
   }

   return NULL;
}

int random_ind(random_node_type *node) {
   for(int i=0;random_nodes[i].num;i++) {
      if(&random_nodes[i] == node) return i;
   }

   return -1;
}

void add_random_obj(int rnd, struct obj_data *obj) {
   int i, affslot;
   random_node_type *node;
   char dest[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];

   if(rnd < 0) return;

   node = &random_nodes[rnd];

   affslot=-1;
   for(i=0;i<MAX_OBJ_AFFECT;i++) {
      if(!obj->affected[i].location) {
	 affslot = i;
	 break;
      }
   }

   //Couldn't find a place to put the random
   if(affslot < 0) {
      return;
   }

   obj->affected[affslot].location = node->affnum;
   if(!node->affmax)
     obj->affected[affslot].modifier = node->affmin;
   else
     obj->affected[affslot].modifier = number(node->affmin, node->affmax);

   SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NO_LOCATE);

   strcpy(dest, ss_data(obj->short_description));
   if(strlen(dest)+strlen(node->name)+6 < 50) {
      strcat(dest, " of ");
      strcat(dest, node->name);
      ss_free(obj->short_description);
      obj->short_description = ss_make(dest);

      strcpy(dest, ss_data(obj->name));
      strcat(dest, " ");
      strcat(dest, node->name);
      ss_free(obj->name);
      obj->name = ss_make(dest);
      if(!NUMBER_RAND_LOGS) {
	 sprintf(buf,"Rand. Obj: %s created.", dest);
         slog(buf);
      }
   }
   if(NUMBER_RAND_LOGS) {
     sprintf(buf,"Rand. Obj: (%ld) (%s).", node->num, node->name);
     slog(buf);
   }

   random_count++;
}

void add_random_obj(struct obj_data *obj) {
   random_node_type *node;

   if(!obj) return;

   node=get_random(&all_randoms,
		   (1<<obj->obj_flags.type_flag),
		   obj->obj_flags.level);

   if(!node) return;

   add_random_obj(random_ind(node), obj);
}














struct random_node_type random_nodes[] = {
     { RN_TROLLS,     "Trolls",       0, APPLY_SPELL, AFF_REGENERATE,  0 },
     { RN_PROPHETS,   "Prophets",     0, APPLY_SPELL, AFF_SANCTUARY,   0 },
     { RN_WRETCHED,   "the Wretched", 0, APPLY_SPELL, AFF_CURSE,       0 },
     { RN_DISEASE,    "Disease",      0, APPLY_SPELL, AFF_POISON,      0 },
     { RN_ANGER,      "Anger",        0, APPLY_SPELL, AFF_BERSERK,     0 },
     { RN_EVASION,    "Evasion",      0, APPLY_SPELL, AFF_DODGE,       0 },
     { RN_LAKE,       "the Lake",     0, APPLY_SPELL, AFF_WATERBREATH, 0 },
     { RN_NARCOLEPSY, "Narcolepsy",   0, APPLY_SPELL, AFF_SLEEP,       0 },
     { RN_LEVITATION, "Levitation",   0, APPLY_SPELL, AFF_FLYING,      0 },
     { RN_SHADOWS,    "Shadows",      0, APPLY_SPELL, AFF_HIDE,        0 },
     { RN_MYSTERY,    "Mystery",      0, APPLY_SPELL, AFF_SNEAK,       0 },
     { RN_INVISIBLE,  "Invisibility", 0, APPLY_SPELL, AFF_INVISIBLE,   0 },
     { RN_BLUR,       "Blur",         0, APPLY_SPELL, AFF_ILLUSION,    0 },
     { RN_HARMONY,    "Harmony",      0, APPLY_SPELL, AFF_MEDITATE,    0 },
     { RN_GODS,       "the Gods",     0, APPLY_SPELL, AFF_LIFE_PROT,   0 },

     { RN_TEPHANIS,   "Tephanis",     0, APPLY_SPELL2, AFF2_HASTE,     0 },
     { RN_MAHNTOR,    "MahnTor",      0, APPLY_SPELL2, AFF2_SLOW,      0 },
     { RN_GLOOM,      "Gloom",        0, APPLY_SPELL2, AFF2_DESPAIR,   0 },

     { RN_CHAOS_S_FIRE,   "Chaos",    0, APPLY_SUSC,   IMM_FIRE,       0 },
     { RN_CHAOS_S_COLD,   "Chaos",    0, APPLY_SUSC,   IMM_COLD,       0 },
     { RN_CHAOS_S_ELEC,   "Chaos",    0, APPLY_SUSC,   IMM_ELEC,       0 },
     { RN_CHAOS_S_ENERGY, "Chaos",    0, APPLY_SUSC,   IMM_ENERGY,     0 },
     { RN_CHAOS_S_BLUNT,  "Chaos",    0, APPLY_SUSC,   IMM_BLUNT,      0 },
     { RN_CHAOS_S_PIERCE, "Chaos",    0, APPLY_SUSC,   IMM_PIERCE,     0 },
     { RN_CHAOS_S_SLASH,  "Chaos",    0, APPLY_SUSC,   IMM_SLASH,      0 },
     { RN_CHAOS_S_ACID,   "Chaos",    0, APPLY_SUSC,   IMM_ACID,       0 },
     { RN_CHAOS_S_POISON, "Chaos",    0, APPLY_SUSC,   IMM_POISON,     0 },
     { RN_CHAOS_S_DRAIN,  "Chaos",    0, APPLY_SUSC,   IMM_DRAIN,      0 },
     { RN_CHAOS_S_SLEEP,  "Chaos",    0, APPLY_SUSC,   IMM_SLEEP,      0 },
     { RN_CHAOS_S_CHARM,  "Chaos",    0, APPLY_SUSC,   IMM_CHARM,      0 },
     { RN_CHAOS_S_NONMAG, "Chaos",    0, APPLY_SUSC,   IMM_NONMAG,     0 },
     { RN_CHAOS_S_HOLD,   "Chaos",    0, APPLY_SUSC,   IMM_HOLD,       0 },
     { RN_CHAOS_S_BARD,   "Chaos",    0, APPLY_SUSC,   IMM_BARD,       0 },

     { RN_CHAOS_R_FIRE,   "Chaos",   30, APPLY_IMMUNE, IMM_FIRE,       0 },
     { RN_CHAOS_R_COLD,   "Chaos",   10, APPLY_IMMUNE, IMM_COLD,       0 },
     { RN_CHAOS_R_ELEC,   "Chaos",   20, APPLY_IMMUNE, IMM_ELEC,       0 },
     { RN_CHAOS_R_ENERGY, "Chaos",   20, APPLY_IMMUNE, IMM_ENERGY,     0 },
     { RN_CHAOS_R_BLUNT,  "Chaos",   40, APPLY_IMMUNE, IMM_BLUNT,      0 },
     { RN_CHAOS_R_PIERCE, "Chaos",   10, APPLY_IMMUNE, IMM_PIERCE,     0 },
     { RN_CHAOS_R_SLASH,  "Chaos",   10, APPLY_IMMUNE, IMM_SLASH,      0 },
     { RN_CHAOS_R_ACID,   "Chaos",   20, APPLY_IMMUNE, IMM_ACID,       0 },
     { RN_CHAOS_R_POISON, "Chaos",   10, APPLY_IMMUNE, IMM_POISON,     0 },
     { RN_CHAOS_R_DRAIN,  "Chaos",   30, APPLY_IMMUNE, IMM_DRAIN,      0 },
     { RN_CHAOS_R_SLEEP,  "Chaos",    0, APPLY_IMMUNE, IMM_SLEEP,      0 },
     { RN_CHAOS_R_CHARM,  "Chaos",    0, APPLY_IMMUNE, IMM_CHARM,      0 },
     { RN_CHAOS_R_NONMAG, "Chaos",   40, APPLY_IMMUNE, IMM_NONMAG,     0 },
     { RN_CHAOS_R_HOLD,   "Chaos",   20, APPLY_IMMUNE, IMM_HOLD,       0 },
     { RN_CHAOS_R_BARD,   "Chaos",   10, APPLY_IMMUNE, IMM_BARD,       0 },

     { RN_CHAOS_I_FIRE,   "Chaos",    0, APPLY_M_IMMUNE, IMM_FIRE,     0 },
     { RN_CHAOS_I_ELEC,   "Chaos",    0, APPLY_M_IMMUNE, IMM_ELEC,     0 },
     { RN_CHAOS_I_DRAIN,  "Chaos",    0, APPLY_M_IMMUNE, IMM_DRAIN,    0 },
     { RN_CHAOS_I_COLD,   "Chaos",    0, APPLY_M_IMMUNE, IMM_COLD,     0 },
     { RN_CHAOS_I_SLASH,  "Chaos",    0, APPLY_M_IMMUNE, IMM_SLASH,    0 },
     { RN_CHAOS_I_PIERCE, "Chaos",    0, APPLY_M_IMMUNE, IMM_PIERCE,   0 },

     { RN_WEAKNESS,       "Weakness",     0, APPLY_STR,    -5,  -1 },
     { RN_MIGHT,          "Might",        0, APPLY_STR,     1,   2 },
     { RN_STRENGTH,       "Strength",     0, APPLY_STR,     2,   3 },
     { RN_POWER,          "Power",        0, APPLY_STR,     3,   4 },

     { RN_FOOLS,          "Fools",        0, APPLY_INT,    -5,  -1 },
     { RN_LEARNING,       "Learning",     0, APPLY_INT,     1,   2 },
     { RN_INTELLIGENCE,   "Intelligence", 0, APPLY_INT,     2,   3 },
     { RN_KNOWLEDGE,      "Knowledge",    0, APPLY_INT,     3,   4 },

     { RN_IGNORANCE,      "Ignorance",    0, APPLY_WIS,    -5,  -1 },
     { RN_DISCRETION,     "Discretion",   0, APPLY_WIS,     1,   2 },
     { RN_WISDOM,         "Wisdom",       0, APPLY_WIS,     2,   3 },
     { RN_JUDGEMENT,      "Judgement",    0, APPLY_WIS,     3,   4 },

     { RN_FUMBLING,       "Fumbling",     0, APPLY_DEX,    -5,  -1 },
     { RN_SKILL,          "Skill",        0, APPLY_DEX,     1,   2 },
     { RN_DEXTERITY,      "Dexterity",    0, APPLY_DEX,     2,   3 },
     { RN_AGILITY,        "Agility",      0, APPLY_DEX,     3,   4 },

     { RN_ILLNESS,        "Illness",      0, APPLY_CON,    -5,  -1 },
     { RN_STAMINA,        "Stamina",      0, APPLY_CON,     1,   2 },
     { RN_CONSTITUTION,   "Constitution", 0, APPLY_CON,     2,   3 },
     { RN_ENDURANCE,      "Endurance",    0, APPLY_CON,     3,   4 },

     { RN_MONSTROSITY,    "Monstrosity",  0, APPLY_CHA,    -5,  -1 },
     { RN_ATTRACTION,     "Attraction",   0, APPLY_CHA,     1,   2 },
     { RN_CHARISMA,       "Charisma",     0, APPLY_CHA,     2,   3 },
     { RN_BEAUTY,         "Beauty",       0, APPLY_CHA,     3,   4 },

     { RN_ETERNITY,       "Eternity",     0, APPLY_AGE,   -40, -20 },
     { RN_LONGEVITY,      "Longevity",    0, APPLY_AGE,   -30, -10 },
     { RN_YOUTH,          "Youth",        0, APPLY_AGE,   -20,  -1 },
     { RN_DAYS,           "Days",         0, APPLY_AGE,     1,  20 },
     { RN_YEARS,          "Years",        0, APPLY_AGE,    10,  30 },
     { RN_CENTURIES,      "Centuries",    0, APPLY_AGE,    20,  40 },
     { RN_MILLENIA,       "Millenia",     0, APPLY_AGE,    30,  50 },
     { RN_EONS,           "Eons",         0, APPLY_AGE,    40,  75 },

     { RN_PITY,           "Pity",         0, APPLY_DAMROLL, -5,  -1 },
     { RN_HARM,           "Harm",         0, APPLY_DAMROLL,  1,   2 },
     { RN_DAMAGE,         "Damage",       0, APPLY_DAMROLL,  2,   3 },
     { RN_PAIN,           "Pain",         0, APPLY_DAMROLL,  3,   4 },

     { RN_MERCY,          "Mercy",        0, APPLY_HITROLL, -5,  -1 },
     { RN_AIM,            "Aim",          0, APPLY_HITROLL,  1,   2 },
     { RN_PRECISION,      "Precision",    0, APPLY_HITROLL,  2,   3 },
     { RN_ACCURACY,       "Accuracy",     0, APPLY_HITROLL,  3,   4 },

     { RN_GUILT,          "Guilt",        0, APPLY_HITNDAM, -5,  -1 },
     { RN_DEATH,          "Death",        0, APPLY_HITNDAM,  1,   2 },
     { RN_BLOOD,          "Blood",        0, APPLY_HITNDAM,  2,   3 },
     { RN_TRAUMA,         "Trauma",       0, APPLY_HITNDAM,  3,   4 },
     { RN_GORE,           "Gore",         0, APPLY_HITNDAM,  4,   5 },
     { RN_CARNAGE,        "Carnage",      0, APPLY_HITNDAM,  5,   6 },

     { RN_VULNERABILITY,  "Vulnerability",0, APPLY_ARMOR,    1,  40 },
     { RN_FORTIFICATION,  "Fortification",0, APPLY_ARMOR,  -20,  -1 },
     { RN_PROTECTION,     "Protection",   0, APPLY_ARMOR,  -30, -10 },
     { RN_DEFENCE,        "Defence",      0, APPLY_ARMOR,  -40, -20 },

     { RN_FRAILTY,        "Frailty",      0, APPLY_AC,     -20,  -1 },
     { RN_TOLERANCE,      "Tolerance",    0, APPLY_AC,       1,  20 },
     { RN_RESISTANCE,     "Resistance",   0, APPLY_AC,      10,  30 },

     { RN_SLAVES,         "Slaves",       0, APPLY_HIT,    -50, -20 },
     { RN_VASSALS,        "Vassals",      0, APPLY_HIT,    -30,  -1 },
     { RN_FIGHTERS,       "Fighters",     0, APPLY_HIT,      1,  25 },
     { RN_WARRIORS,       "Warriors",     0, APPLY_HIT,     20,  50 },
     { RN_HEROES,         "Heroes",       0, APPLY_HIT,     45,  75 },
     { RN_KINGS,          "Kings",        0, APPLY_HIT,     70, 100 },

     { RN_IDIOTS,         "Idiots",       0, APPLY_MANA,   -50, -20 },
     { RN_CRETINS,        "Cretins",      0, APPLY_MANA,   -30,  -1 },
     { RN_SAGES,          "Sages",        0, APPLY_MANA,     1,  25 },
     { RN_MAGES,          "Mages",        0, APPLY_MANA,    20,  50 },
     { RN_SORCERERS,      "Sorcerers",    0, APPLY_MANA,    45,  75 },
     { RN_WIZARDS,        "Wizards",      0, APPLY_MANA,    70, 100 },

     { RN_SLOTHS,         "Sloths",       0, APPLY_MOVE,   -50, -20 },
     { RN_LOITERERS,      "Loiterers",    0, APPLY_MOVE,   -30,  -1 },
     { RN_PILGRIMS,       "Pilgrims",     0, APPLY_MOVE,     1,  25 },
     { RN_NOMADS,         "Nomads",       0, APPLY_MOVE,    20,  50 },
     { RN_TRAVELERS,      "Travelers",    0, APPLY_MOVE,    45,  75 },
     { RN_EXPLORERS,      "Explorers",    0, APPLY_MOVE,    70, 100 },

     { RN_BASHING,        "Bashing",      0, APPLY_BASH,     1,  20 },
     { RN_RAM,            "Ram",          0, APPLY_BASH,    10,  30 },
     { RN_RHINO,          "Rhino",        0, APPLY_BASH,    20,  40 },

     { RN_DECEIT,         "Deceit",       0, APPLY_HIDE,     1,  20 },
     { RN_SHADES,         "Shades",       0, APPLY_HIDE,    10,  30 },

     { RN_BANDITS,        "Bandits",      0, APPLY_PICK,     1,  20 },
     { RN_ROBBERS,        "Robbers",      0, APPLY_PICK,    10,  30 },

     { RN_FILCHERS,       "Filchers",     0, APPLY_STEAL,    1,  20 },
     { RN_OUTLAWS,        "Outlaws",      0, APPLY_STEAL,   10,  30 },

     { RN_BRIGANDS,       "Brigands",     0, APPLY_SNEAK,    1,  20 },
     { RN_STEALTH,        "Stealth",      0, APPLY_SNEAK,   10,  30 },

     { RN_PIERCING,       "Piercing",     0, APPLY_BACKSTAB, 1,  20 },
     { RN_THIEVES,        "Thieves",      0, APPLY_BACKSTAB,10,  30 },

     { RN_LOST,           "Lost",         0, APPLY_TRACK,  -30,  -1 },
     { RN_SCOUTS,         "Scouts",       0, APPLY_TRACK,    1,  20 },
     { RN_PURSUIT,        "Pursuit",      0, APPLY_TRACK,   10,  30 },
     { RN_PREDATORS,      "Predators",    0, APPLY_TRACK,   20,  40 },

     { RN_GLASSGRIP,      "GlassGrip",    0, APPLY_BHD,       -3,  -1 },
     { RN_GOUT,           "Gout",         0, APPLY_NUM_DICE,  -3,  -1 },
     { RN_ARTHRITIS,      "Arthritis",    0, APPLY_SIZE_DICE, -3,  -1 },
     { RN_IRONFIST,       "IronFist",     0, APPLY_BHD,        1,   2 },
     { RN_BRASSCLAW,      "BrassClaw",    0, APPLY_BHD,        2,   3 },
     { RN_GRIP,           "Grip",         0, APPLY_NUM_DICE,   1,   2 },
     { RN_VISE,           "Vise",         0, APPLY_SIZE_DICE,  1,   3 },

     { RN_RIDING,         "Riding",       0, APPLY_RIDE,       1,  20 },
     { RN_JOUSTING,       "Jousting",     0, APPLY_RIDE,      10,  30 },

     { RN_AVATAR,         "Avatar",       0, APPLY_SPELL,       0x90,   0 },
     { RN_BALROG,         "Balrog",       0, APPLY_SPELL, 0x50000000,   0 },
     { RN_ELFEATER,       "ElfEater",     0, APPLY_SPELL,    0x10002,   0 },
     { RN_LICH,           "Lich",         0, APPLY_SPELL,   0x140000,   0 },
     { RN_PHOENIX,        "Phoenix",      0, APPLY_SPELL,  0x8000040,   0 },
     { RN_ROC,            "Roc",          0, APPLY_SPELL,      0x804,   0 },
     { RN_DREAMMASTER,    "DreamMaster",  0, APPLY_SPELL,     0xA000,   0 },
     { RN_EFREETI,        "Efreeti",      0, APPLY_SPELL, 0xA0000000,   0 },
     { RN_DEMONSPAWN,     "DemonSpawn",   0, APPLY_SPELL,  0x2000200,   0 },

     { RN_TIMUS,          "Timus",        0, APPLY_SPELL, 		AFF_BERSERK, 	 0 },
     { RN_TANGA,          "Tanga",        0, APPLY_HITNDAM,     	-10,  		-5 },
     { RN_NOVAK,          "Novak",        0, APPLY_CHA,          	-5,  		-1 },
     { RN_SLIM,           "Slim",         0, APPLY_SPELL, 		AFF_BLIND,   	 0 },
     { RN_SOLAAR,         "Solaar",       0, APPLY_WEAPON_SPELL, 	SPELL_SUNRAY,	 0 },

     { RN_FIRESHIELD,     "Fire",      0, APPLY_SPELL,  AFF_FIRESHIELD,   0 },
     { RN_LIGHTNING,      "Lightning", 0, APPLY_SPELL2, AFF2_ELECSHIELD, 0 },
     { RN_POISON,         "Poison",    0, APPLY_SPELL2, AFF2_POISONSHIELD,0 },
     { RN_ENERGY,         "Energy",    0, APPLY_SPELL2, AFF2_ENERGYSHIELD,0 },
     { RN_HATE,           "Hate",      0, APPLY_SPELL2, AFF2_VAMPSHIELD,  0 },
     { RN_MAGIC,          "Magic",     0, APPLY_SPELL2, AFF2_MANASHIELD,  0 },
     { RN_ACID,           "Acid",      0, APPLY_SPELL2, AFF2_ACIDSHIELD,  0 },
     { RN_ICE,            "Ice",       0, APPLY_SPELL2, AFF2_COLDSHIELD,  0 },
     { RN_RADIATION,      "Radiation", 0, APPLY_SPELL2, AFF2_MOVESHIELD,  0 },

     { RN_BLINDNESS,      "Blindness",   0, APPLY_SPELL, AFF_BLIND,       0 },
     { RN_INSIGHT,        "Insight",     0, APPLY_SPELL, AFF_SENSE_AURA,  0 },
     { RN_SIGHT,          "Sight",       0, APPLY_SPELL, AFF_INFRAVISION, 0 },
     { RN_VISION,         "Vision",      0, APPLY_SPELL, AFF_TRUE_SIGHT,  0 },
     { RN_PERCEPTION,     "Perception",  0, APPLY_SPELL, AFF_SENSE_LIFE,  0 },
     { RN_CLAIRVOYANCE,   "Clairvoyance",0, APPLY_SPELL, AFF_DETECT_INVISIBLE,0 },
     { RN_DIVINATION,     "Divination",  0, APPLY_SPELL, AFF_SCRYING,     0 },
     { RN_BLACKLIGHT,     "BlackLight",  0, APPLY_SPELL, AFF_CONTINUAL_DARK,0 },
     { RN_BRILLIANCE,     "Brilliance",  0, APPLY_SPELL, AFF_CONTINUAL_LIGHT,0 },

     { RN_CTHULHU,   "Cthulhu",   0, APPLY_WEAPON_SPELL, SPELL_ACID_RAIN,   0 },
     { RN_FIRESTAR,  "FireStar",  0, APPLY_WEAPON_SPELL, SPELL_ACID_BLAST,  0 },

     { RN_COLD,      "Cold",      0, APPLY_WEAPON_SPELL, SPELL_CHILL_TOUCH, 0 },
     { RN_FROST,     "Frost",     0, APPLY_WEAPON_SPELL, SPELL_FROST_CLOUD, 0 },
     { RN_BLIZZARDS, "Blizzards", 0, APPLY_WEAPON_SPELL, SPELL_ICE_STORM,   0 },

     { RN_GHOULS,    "Ghouls",    0, APPLY_WEAPON_SPELL, SPELL_WEAKNESS,    0 },
     { RN_GHOSTS,    "Ghosts",    0, APPLY_WEAPON_SPELL, SPELL_ENERGY_DRAIN,0 },
     { RN_VAMPIRES,  "Vampires",  0, APPLY_WEAPON_SPELL, SPELL_VAMPYRIC_TOUCH,0 },
     { RN_GHASTS,    "Ghasts",    0, APPLY_WEAPON_SPELL, SKILL_DRAIN_MANA,  0 },

     { RN_SHOCK,     "Shock",     0, APPLY_WEAPON_SPELL, SPELL_SHOCKING_GRASP,0 },
     { RN_THUNDER,   "Thunder",   0, APPLY_WEAPON_SPELL, SPELL_ELECTROCUTE, 0 },
     { RN_STATIC,    "Static",    0, APPLY_WEAPON_SPELL, SPELL_ELECTRIC_FIRE,0 },
     { RN_STORMS,    "Storms",    0, APPLY_WEAPON_SPELL, SPELL_CHAIN_ELECTROCUTION,0 },
     { RN_ZEUS,      "Zeus",      0, APPLY_WEAPON_SPELL, SPELL_CALL_LIGHTNING,0 },

     { RN_SUFFERING, "Suffering", 0, APPLY_WEAPON_SPELL, SPELL_HARMFUL_TOUCH,0 },
     { RN_ANGUISH,   "Anguish",   0, APPLY_WEAPON_SPELL, SPELL_WITHER,      0 },
     { RN_AGONY,     "Agony",     0, APPLY_WEAPON_SPELL, SPELL_RUPTURE,     0 },
     { RN_TORMENT,   "Torment",   0, APPLY_WEAPON_SPELL, SPELL_IMPLODE,     0 },
     { RN_TORTURE,   "Torture",   0, APPLY_WEAPON_SPELL, SPELL_DISINTEGRATE,0 },

     { RN_HEAT,      "Heat",      0, APPLY_WEAPON_SPELL, SPELL_BURNING_HANDS,0 },
     { RN_FIRE,      "Fire",      0, APPLY_WEAPON_SPELL, SPELL_FIRE_WIND,   0 },
     { RN_IMMOLATION,"Immolation",0, APPLY_WEAPON_SPELL, SPELL_FLAMESTRIKE, 0 },
     { RN_INFERNO,   "Inferno",   0, APPLY_WEAPON_SPELL, SPELL_FIREBALL,    0 },
     { RN_VOLCANOES, "Volcanoes", 0, APPLY_WEAPON_SPELL, SPELL_LAVA_STORM,  0 },

     { RN_ARACHNOS,  "Arachnos",  0, APPLY_WEAPON_SPELL, SPELL_WEB,         0 },
     { RN_PARAMOR,   "Paramor",   0, APPLY_WEAPON_SPELL, SPELL_FAERIE_FIRE, 0 },
     { RN_EXPOSURE,  "Exposure",  0, APPLY_WEAPON_SPELL, SPELL_FAERIE_FOG,  0 },
     { RN_THALIDS,   "Thalids",   0, APPLY_WEAPON_SPELL, SPELL_THORN,       0 },
     { RN_DRYADS,    "Dryads",    0, APPLY_WEAPON_SPELL, SPELL_VINE,        0 },
     { RN_HURRICANES,"Hurricanes",0, APPLY_WEAPON_SPELL, SPELL_WIND_STORM,  0 },
     { RN_XORN,      "Xorn",      0, APPLY_WEAPON_SPELL, SPELL_EARTHQUAKE,  0 },
     { RN_GEYSERS,   "Geysers",   0, APPLY_WEAPON_SPELL, SPELL_GEYSER,      0 },
     { RN_LOCUSTS,   "Locusts",   0, APPLY_WEAPON_SPELL, SPELL_CREEPING_DOOM,0 },

     { RN_SERPENTS,  "Serpents",  0, APPLY_WEAPON_SPELL, SPELL_POISON,      0 },
     { RN_ASPS,      "Asps",      0, APPLY_WEAPON_SPELL, SPELL_POISON_GAS,  0 },

     { RN_EXILE,     "Exile",     0, APPLY_WEAPON_SPELL, SPELL_TELEPORT,    0 },
     { RN_THE_BAT,   "the Bat",   0, APPLY_WEAPON_SPELL, SPELL_BLINDNESS,   0 },
     { RN_EMPATHY,   "Empathy",   0, APPLY_WEAPON_SPELL, SPELL_HEAL,        0 },
     { RN_SLUMBER,   "Slumber",   0, APPLY_WEAPON_SPELL, SPELL_SLEEP,       0 },
     { RN_PURITY,    "Purity",    0, APPLY_WEAPON_SPELL, SPELL_DISPEL_MAGIC,0 },
     { RN_BASILISKS, "Basilisks", 0, APPLY_WEAPON_SPELL, SPELL_PARALYSIS,   0 },
     { RN_PHOBIAS,   "Phobias",   0, APPLY_WEAPON_SPELL, SPELL_FEAR,        0 },
     { RN_THE_HOLY,  "the Holy",  0, APPLY_WEAPON_SPELL, SPELL_TURN,        0 },

     { RN_RAINBOWS,  "Rainbows",      0, APPLY_IMMUNE,   0x38F,             0 },
     { RN_ALLSIGHT,  "Allsight",      0, APPLY_SPELL,    0x46008038,        0 },
     { RN_HIDING,    "Hiding",        0, APPLY_SPELL,    0x182002,          0 },
     { RN_PROTECTOR, "the Protector", 0, APPLY_SPELL,    0x40080,           0 },
     { RN_EHEALTH,   "Eternal Health",0, APPLY_HIT,      150,             200 },
     { RN_EMANA,     "Eternal Magic", 0, APPLY_MANA,     150,             200 },
     { RN_ELIFE,     "Eternal Life",  0, APPLY_AGE,      100,             150 },
     { RN_FLAWLESS,  "Flawlessness",  0, APPLY_HITROLL,  10,               20 },
     { RN_SLAUGHTER, "Slaughter",     0, APPLY_HITNDAM,  7,                10 },
     { RN_RAGE,      "Rage",          0, APPLY_BHD,      3,                 5 },

     { 0, "", 0, 0, 0, 0 }
};
