#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "utility.h"
#include "multiclass.h"
#include "smart_mobs.h"
#include "block.h"
#include "vnum_mob.h"
#include "spec.h"

/* a blocker will intercept a move command to see if it leads to a room   */
/* he is blocking. the block routine is the one used to determine who is  */
/* allowed to pass and who isn't. the fight routine is the routine called */
/* if the blocker is in combat. hmmmm, do blockers stop pushed chars?     */

int mage_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_MAGIC_USER)) {
    act("$n steps forward and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and refuses to let $N pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int cleric_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_CLERIC)) {
    act("$n steps forward and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and refuses to let $N pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int warrior_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_WARRIOR)) {
    act("$n grabs you by the arm and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n grabs $N by the arm and refuses to let $M pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int thief_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_THIEF)) {
    act("$n steps forward and blocks your way.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and blocks $N's way.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int paladin_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_PALADIN)) {
    act("$n steps forward and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and refuses to let $N pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int druid_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_DRUID)) {
    act("$n steps forward and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and refuses to let $N pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int ranger_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_RANGER)) {
    act("$n grabs you by the arm and refuses to let you pass.",TRUE,c,0,v,TO_VICT);
    act("$n grabs $N by the arm and refuses to let $M pass.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int psi_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_PSI)) {
    act("$n steps forward and blocks your way.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and blocks $N's way.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}
int bard_block(struct char_data *c, struct char_data *v)
{
  if (!HasClass(v, CLASS_BARD)) {
    act("$n steps forward and blocks your way.",TRUE,c,0,v,TO_VICT);
    act("$n steps forward and blocks $N's way.",TRUE,c,0,v,TO_ROOM);
    act("You step forward and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
  }
  return (FALSE);
}

int sisyphus_block(struct char_data *c, struct char_data *v)
{
  if (GetMaxLevel(v) > 9) {
    act("$n grabs you and easily tosses you onto the street.",TRUE,c,0,v,TO_VICT);
    act("$n grabs $N roughly and throws $M onto the street.",TRUE,c,0,v,TO_ROOM);
    act("You grab $N and throw him onto the street.", TRUE,c,0,v,TO_CHAR);
    GET_POS(v)=POSITION_SITTING;
    return (TRUE);
  }
  return (FALSE);
}

int abyss_block(struct char_data *c, struct char_data *v)
{
    act("$n snarls angrily and bars your passage.",TRUE,c,0,v,TO_VICT);
    act("$n snarls angrily and bars $N's passage.",TRUE,c,0,v,TO_ROOM);
    act("You snarl angrily and bar $N's passage.", TRUE,c,0,v,TO_CHAR);
    return (TRUE);
}

int guardian_block(struct char_data *c, struct char_data *v)
{
 if (GetMaxLevel(v) < 75 ) {
  act("$n grabs you and easily tosses you onto the street.",TRUE,c,0,v,TO_VICT);
 act("$n grabs $N roughly and throws $M onto the street.",TRUE,c,0,v,TO_ROOM);
    act("You grab $N and throw him onto the street.", TRUE,c,0,v,TO_CHAR);
    GET_POS(v)=POSITION_SITTING;
    return (TRUE);
  }
  return (FALSE);
}

/* here's the actual blocking routine */
SPECIAL(blocker)
{
  static struct block_data blockers[]= {
    { VMOB_120, 29651, DOWN,  psi_block,           generic_psi },
    { VMOB_117, 25124, DOWN,  paladin_block,       generic_paladin },
    { VMOB_108, 18195, DOWN,  psi_block,           generic_psi },
    { VMOB_107, 18193, SOUTH, druid_block,         generic_druid },
    { VMOB_106, 18191, SOUTH, ranger_block,        generic_ranger },
    { VMOB_105, 18189, UP,    paladin_block,       generic_paladin },
    { VMOB_109, 18266, WEST,  mage_block,          generic_mage },
    { VMOB_111, 18272, NORTH, thief_block,         generic_thief },
    { VMOB_110, 18276, EAST,  cleric_block,        generic_cleric },
    { VMOB_112, 18256, SOUTH, warrior_block,       generic_warrior },
    { VMOB_119, 16768, DOWN,  druid_block,         generic_druid },
    { VMOB_116, 7921,  UP,    cleric_block,        generic_cleric },
    { VMOB_118, 6158,  DOWN,  ranger_block,        generic_ranger },
    { VMOB_114, 5254,  SOUTH, warrior_block,       generic_warrior },
    { VMOB_115, 5073,  DOWN,  thief_block,         generic_thief },
    { VMOB_100,  3021,  NORTH, warrior_block,       generic_warrior },
    { VMOB_99,  3027,  EAST,  thief_block,         generic_thief },
    { VMOB_98,  3004,  NORTH, cleric_block,        generic_cleric },
    { VMOB_97,  3017,  NORTH, mage_block,          generic_mage },
    { VMOB_104,  2623,  UP,    psi_block,           generic_psi },
    { VMOB_103,  2621,  UP,    druid_block,         generic_druid },
    { VMOB_102,  2619,  NORTH, ranger_block,        generic_ranger },
    { VMOB_101,  2617,  UP,    paladin_block,       generic_paladin },
    { VMOB_113, 1686,  DOWN,  mage_block,          generic_mage },
    { VMOB_121,  1499,  WEST,  sisyphus_block,      generic_warrior },
    { VMOB_130, 25003, DOWN,  abyss_block,         generic_warrior },
    { VMOB_129, 13758, EAST,  abyss_block,		NULL },
    { VMOB_132, 2650, EAST, bard_block,            generic_warrior },
    { VMOB_32200, 1011,  UP, guardian_block,      generic_warrior }, 
    { 0, 0, 0, NULL, NULL }
  };
  int i, dir=-1, vnum=0;
  block_proc b_proc=NULL;
  spec_proc_func sp_proc=NULL;
  struct char_data *t, *bl = NULL;

  switch (type)
  {
  case SPEC_CMD:
      /* this is an intercepted command, let's first see if it's a direction */
      if (cmd > 6)
	  return (FALSE);
      
      if(!IS_PC(ch))
	  return (FALSE);
  
      /* ok, it's a direction, let's find this room to see who's blocking */
      for (i = 0; blockers[i].mob_vnum; i++)
	  if (blockers[i].room == ch->in_room)
	  {
	      b_proc = blockers[i].b_proc;
	      vnum = blockers[i].mob_vnum;
	      dir = blockers[i].blocked_direction;
	      break;
	  }

      /* ok, this room isn't supposed to have a blocker */
      if (!vnum)
	  return (FALSE);

      /* is this a direction we want to block? */
      if (dir != (cmd-1))
	  return (FALSE);

      /* ok, lets find the blocking mob in the room */
      for (t = (real_roomp(ch->in_room))->people; t; t = t->next_in_room)
	  if (IS_NPC(t) && vnum == mob_index[t->nr].virt)
	  {
	      bl=t;
	      break;
	  }
      
      if (!bl)
      {
	  log_msg ("blocker called in wrong room\n\r");
	  return (FALSE);
      }

      /* if i'm fighting, asleep, or can't see them, then i can't block */
      if (!AWAKE(bl) || bl->specials.fighting || !CAN_SEE(bl, ch) ||
	  IS_IMMORTAL(ch) || bl==ch)
	  return (FALSE);

      if (b_proc)
	  return (b_proc(bl, ch));

      break;
      
  default:
      bl = (struct char_data *) me;
      for (i = 0; blockers[i].mob_vnum; i++)
	  if (blockers[i].mob_vnum == mob_index[bl->nr].virt)
	  {
	      sp_proc=blockers[i].sp_proc;
	      break;
	  }

      if (sp_proc)
	  return (sp_proc(me, ch, 0, "", type));
      
      return (FALSE);
  }

  return (FALSE);
}

