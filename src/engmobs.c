#include "config.h"

#include "structs.h"
#include "spelltab.h"
#include "utils.h"
#include "engine.h"


/*-------------------- EXAMPLE SPEC_PROC DEFINITIONS -------------------*/

int sample_magic_user_npc (void *me, struct char_data *ch, char* arg, int cmd)

{
   int fight[][MAX_SET_SIZE] = {         /*** abilities to perform ***/ 
      {52},                              /*** while fighting       ***/
      {9, 102},                          /*** values are used from ***/
      {6, 56, 49},                       /*** the allspells list   ***/
      {0}
   };
   int peace[][MAX_SET_SIZE] = {         /*** abilities to perform ***/
      {23},                              /*** while not fighting   ***/
      {43},
      {0},
   };

   return (do_abilities((struct char_data *) me, fight, peace, cmd));
}

int sample_psi_npc (void *me, struct char_data *ch, char* arg, int cmd)
{
   int fight[][MAX_SET_SIZE] = {
      {142},  /*** mindblast     ***/
      {136},  /*** phantomkiller ***/
      {0}
   };
   int peace[][MAX_SET_SIZE] = {
      {135},  /*** restore flameshroud if dispelled ***/
      {0}
   };
   /*** because skills check the class level, we will   ***/
   /* init_psi(ch, TRUE); */ /*** have to set the npc's psi level, and anything   ***/
   /*** that needs doing to use the existing skill code ***/
   /*** the true flag is for the special case of the    ***/
   /*** psi npc being able to meditate instead of rest  ***/

   return do_abilities((struct char_data *) me, fight, peace, cmd);
}


int lava_man_npc (void *me, struct char_data *ch, char* arg, int cmd)
{
   int fight[][MAX_SET_SIZE] = {
      {3, 49, -102},
      {0}
   };
   int peace[][MAX_SET_SIZE] = {   /*** no peace abilities ***/
      {0}
   };

   return (do_abilities((struct char_data *) me, fight, peace, cmd));
}

