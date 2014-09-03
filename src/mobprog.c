/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "structs.h"
#include "utility.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "proto.h"
#include "fight.h"
#include "find.h"
#include "varfunc.h"
#include "comm.h"



#define INIFILE "../lib/mobprogs/mobprog.ini"
#define PRGDIR  "../lib/mobprogs/"

char buf2[MAX_STRING_LENGTH];

extern struct index_data *mob_index;
extern struct index_data *obj_index;

/*
 * Local function prototypes
 */

char *	mprog_next_command    ( char* clist );
int           mprog_seval	      ( char* lhs, char* opr, char* rhs );
int           mprog_seval_abbrev ( char* lhs, char* opr, char* rhs);
int	mprog_veval	      ( int lhs, char* opr, int rhs );
int	mprog_do_ifchck	      ( char* ifchck, struct char_data* mob,
			       struct char_data* actor, struct obj_data* obj,
			       void* vo, struct char_data* rndm );
char *	mprog_process_if      ( char* ifchck, char* com_list, 
			       struct char_data* mob, struct char_data* actor,
			       struct obj_data* obj, void* vo,
			       struct char_data* rndm );
void	mprog_translate	      ( char ch, char* t, struct char_data* mob,
			       struct char_data* actor, struct obj_data* obj,
			       void* vo, struct char_data* rndm );
void	mprog_process_cmnd    ( char* cmnd, struct char_data* mob, 
			       struct char_data* actor, struct obj_data* obj,
			       void* vo, struct char_data* rndm );
void	mprog_driver	      ( char* com_list, struct char_data* mob,
			       struct char_data* actor, struct obj_data* obj,
			       void* vo );

/***************************************************************************
 * Local function code and brief comments.
 */

/* Used to get sequential lines of a multi line string (separated by "\r\n")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */

char *mprog_next_command( char *clist ) {

  char *pointer = clist;

   /* jump over initial \n' and \r's being careful of null terminat */
   
  while ( *pointer != '\n' && *pointer != '\0' && *pointer != '\r')
    pointer++;
  if ( *pointer == '\n' ) { /* handle n then r!!!! doof! */
     *pointer++ = '\0';
     if (*pointer == '\r')
       *pointer++ = '\0';
  }
  else /* handle r then n */ 
     if ( *pointer == '\r' )  {
	*pointer++ = '\0';
	if (*pointer == '\n') 
	   *pointer++ = '\0';
     }
	
  return ( pointer );
}

/* we need str_infix here because strstr is not case insensitive */

bool str_infix( const char *astr, const char *bstr ) {
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    bool str_prefix(const char *astr, const char *bstr);

    if ((c0 = LOWER(astr[0])) == '\0')
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ ) {
        if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
            return FALSE;
    }

    return TRUE;
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
int mprog_seval( char *lhs, char *opr, char *rhs ) {

  if ( !str_cmp( opr, "==" ) )
    return ( !str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( str_infix( rhs, lhs ) );

  log_msg("MPERR: Improper MOBprog operator (in mprog_seval) ", LOG_MPROG);
  return 0;

}

/* this function is like seval except it uses is_abbrev instead */

int mprog_seval_abbrev( char *lhs, char *opr, char *rhs ) {

  if ( !str_cmp( opr, "==" ) )
    return ( is_abbrev( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( is_abbrev( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( str_infix( rhs, lhs ) );

  log_msg("MPERR: Improper MOBprog operator (in mprog_seval_abbrev) ",
LOG_MPROG);
  return 0;

}


int mprog_veval( int lhs, char *opr, int rhs ) {

  if ( !str_cmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !str_cmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !str_cmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !str_cmp( opr, "<" ) )
    return ( lhs < rhs );
  if ( !str_cmp( opr, "<=" ) )
    return ( lhs <= rhs );
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !str_cmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !str_cmp( opr, "|" ) )
    return ( lhs | rhs );

 char buf[MAX_INPUT_LENGTH];

  log_msg("MPERR: Improper MOBprog operator (in mprog_veval)", LOG_MPROG);
   sprintf(buf,"lhs= %d, opr = %s, rhs= %d", lhs, opr, rhs);
   log_msg(buf, LOG_MPROG);
  return 0;

}

int mprog_keyword_replace(char *buf, char *arg, char *opr, char *val, struct char_data *mob,
		    struct char_data *actor, struct obj_data *obj,
		    void *vo, struct char_data *rndm, int lhsvl, int rhsvl) {

  struct char_data *vict = (struct char_data *) vo;
  struct obj_data *v_obj = (struct obj_data  *) vo;
   
   
  if ( !str_cmp( buf, "rand" ) )  {
    return ( number(1, 100) <= atoi(arg) ); /* rand(10) = 10% chance */
  }
  
  /* 
   * Written for Rick to check if a Mob is already in a room -- Min
   */
  if (!str_cmp(buf, "isinroom" ) ) {
    if (actor)
      if ((actor->in_room != NOWHERE) && arg) 
	if (get_char_room(arg,actor->in_room))
	  return TRUE;
    return FALSE;
  }

  /* 
   * Written for Brian to check if a Mob is already in a room -- Jim
   */
  if (!str_cmp(buf, "ispeaceroom" ) ) {
    if (mob) {
      struct room_data *rp = real_roomp(mob->in_room);
      if (rp && rp->room_flags&PEACEFUL) 
	return TRUE;
    }
    return FALSE;
  }

  /* i changed the following because IS_PC takes into account
     whether the player is shifted or not... see the header IS_PC macro */
  
  if ( !str_cmp( buf, "ispc" ) )   {
    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i':
	return 0;
      case 'n': 
	if ( actor )
	return ( IS_PC( actor ) );
      else 
	return -1;
      case 't': if ( vict )
	return ( IS_PC( vict ) );
      else 
	return -1;
      case 'r': if ( rndm )
	return ( IS_PC( rndm ) );
      else 
	return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'ispc'",
		mob_index[mob->nr].virt); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "isnpc" ) ) {
    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i':
	return 1;
      case 'n': 
	if ( actor )
	return IS_NPC( actor );
      else 
	return -1;
      case 't': if ( vict )
	return IS_NPC( vict );
      else 
	return -1;
      case 'r': if ( rndm )
	return IS_NPC( rndm );
      else 
	return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isnpc'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "isevil" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': 
	return IS_EVIL( mob );
      case 'n': if ( actor )
	return IS_EVIL( actor );
      else return -1;
      case 't': if ( vict )
	return IS_EVIL( vict );
      else return -1;
      case 'r': if ( rndm )
	return IS_EVIL( rndm );
      else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isevil'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "isgood" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': 
	return IS_GOOD( mob );
      case 'n': if ( actor )
	return IS_GOOD( actor );
      else return -1;
      case 't': if ( vict )
	return IS_GOOD( vict );
      else return -1;
      case 'r': if ( rndm )
	return IS_GOOD( rndm );
      else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isgood'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }
  
  if ( !str_cmp( buf, "isneutral" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': 
	return IS_NEUTRAL( mob );
      case 'n': if ( actor )
	return IS_NEUTRAL( actor );
      else return -1;
      case 't': if ( vict )
	return IS_NEUTRAL( vict );
      else return -1;
      case 'r': if ( rndm )
	return IS_NEUTRAL( rndm );
      else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isneutral'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }
  
  if ( !str_cmp( buf, "isfight" ) ) {
    switch ( arg[1] ) { /* arg should be "$*" so just get the letter */
 
    case 'i' : if (mob) return  IS_FIGHTING(mob);
       else return -1;
    case 'n' : if ( actor ) return IS_FIGHTING(actor);
       else return -1;
    case 't': if ( vict ) return vict->specials.fighting ? 1 : 0;
       else return -1;
    case 'r': if ( rndm ) return rndm->specials.fighting ? 1 : 0;
       else return -1;
    default:
      sprintf(buf2, "MPERR: Mob: %d bad argument to 'isfight'",
	      mob_index[mob->nr].virt ); 
      log_msg(buf2, LOG_MPROG);
      return -1;
    }
  }

  if ( !str_cmp( buf, "isfollowing" ) ) {
     struct char_data *ch1;
     switch ( arg[1] ) // should be $*,$*
       {
	case 'i': ch1=mob;break;
	case 'n': ch1=actor;break;
	case 't': ch1=vict;break;
	case 'r': ch1=rndm;break;
	default:
	  log_msg("Illegal letter in isfollowing");
	  return 0;
       }
     switch ( arg[4] )
       {
	case 'i': return mprog_veval((ch1->master == mob), opr, atoi(val));
	case 'n': return mprog_veval((ch1->master == actor), opr, atoi(val));
	case 't': return mprog_veval((ch1->master == vict), opr, atoi(val));
	case 'r': return mprog_veval((ch1->master == rndm), opr, atoi(val));
       }
  }
   
  if ( !str_cmp( buf, "hasobj" ) ) {
    struct obj_data *obj1;
    switch ( arg[1] )  // should be $*
       {
	case 'i': return mprog_veval((mob && get_obj_in_list(arg+3, mob->carrying)), opr, atoi(val));
   	case 'n': return mprog_veval((actor && get_obj_in_list(arg+3, actor->carrying)), opr, atoi(val));
	case 't': return mprog_veval((vict && get_obj_in_list(arg+3, vict->carrying)), opr, atoi(val));
	case 'r': return mprog_veval((rndm && get_obj_in_list(arg+3, rndm->carrying)), opr, atoi(val));
	case 'm': return mprog_veval((mob->in_room && (obj1 = get_obj_vis(mob, arg+3)) && (obj1->in_room == mob->in_room)), opr, atoi(val));
	default: return 0;
       }
  }
   
  if ( !str_cmp( buf, "isimmort" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': return IS_IMMORTAL(mob);
      case 'n': if ( actor )
	return IS_IMMORTAL( actor );
      else return -1;
      case 't': if ( vict )
	return IS_IMMORTAL( vict );
      else return -1;
      case 'r': if ( rndm )
	return IS_IMMORTAL( rndm );
      else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isimmort'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "ischarmed" ) )  {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': return IS_AFFECTED( mob, AFF_CHARM );
      case 'n': if ( actor )
	return IS_AFFECTED( actor, AFF_CHARM );
         else return -1;
      case 't': if ( vict )
	return IS_AFFECTED( vict, AFF_CHARM );
         else return -1;
      case 'r': if ( rndm )
	return IS_AFFECTED( rndm, AFF_CHARM );
         else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'ischarmed'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "isfollow" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': return ( mob->master != NULL
			 && IN_ROOM(mob->master) == IN_ROOM(mob));
      case 'n': if ( actor )
	return ( actor->master != NULL
		 && IN_ROOM(actor->master) == IN_ROOM(actor));
         else return -1;
      case 't': if ( vict )
	return ( vict->master != NULL
		 && IN_ROOM(vict->master) == IN_ROOM(vict));
         else return -1;
      case 'r': if ( rndm )
	return ( rndm->master != NULL
		 && IN_ROOM(rndm->master) == IN_ROOM(rndm));
         else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isfollow'", 
		mob_index[mob->nr].virt); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  if ( !str_cmp( buf, "isaffected" ) ) {

    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': return IS_AFFECTED(mob, atoi( arg ));
      case 'n': if ( actor )
	return IS_AFFECTED(actor, atoi( arg ));
         else return -1;
      case 't': if ( vict )
	return IS_AFFECTED(vict, atoi( arg ));
         else return -1;
      case 'r': if ( rndm )
	return IS_AFFECTED(rndm, atoi( arg ));
         else return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'isaffected'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }

  /* ok found a bug here... lshvl and rshvl are declared as ints
     So, any of the mprog_vevals would fail because a player with
     300 out of 400 hp would have a lhsvl of .75 (which in int
     terms is --- 1!!!....  So... hadda do a little more math ;) see utils.h
     for the actual HEALTH macro */

  if ( !str_cmp( buf, "health" ) ) {
    switch ( arg[1] )  /* arg should be "$*" so just get the letter */
      {
      case 'i': lhsvl = HEALTH(mob);
	rhsvl = atoi( val );
	return mprog_veval( lhsvl, opr, rhsvl );
      case 'n': if ( actor )
	{
	  lhsvl = HEALTH(actor);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	}
         else return -1;
      case 't': if ( vict )
	{
	  lhsvl = HEALTH(vict);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	}
      else
	return -1;
      case 'r': if ( rndm )
	{
	  lhsvl = HEALTH(rndm);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	}
      else
	return -1;
      default:
	sprintf(buf2, "MPERR: Mob: %d bad argument to 'hitprcnt'",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }
  }
  

  if ( !str_cmp( buf, "qnum" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->player.qnum;
	  rhsvl = atoi(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = actor->player.qnum;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = vict->player.qnum;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = rndm->player.qnum;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'qnum'",
		  mob_index[mob->nr].virt); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  if ( !str_cmp( buf, "mpstate" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = mob->player.mpstate;
	  rhsvl = atoi(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = actor->player.mpstate;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = vict->player.mpstate;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = rndm->player.mpstate;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'mpstate'",
		  mob_index[mob->nr].virt); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  if ( !str_cmp( buf, "inroom" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = IN_ROOM(mob);
	  rhsvl = atoi(val);
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = IN_ROOM(actor);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = IN_ROOM(vict);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = IN_ROOM(rndm);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'inroom'",
		  mob_index[mob->nr].virt); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "sex" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_SEX(mob);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = GET_SEX(actor);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = GET_SEX(vict);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = GET_SEX(rndm);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'sex'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "position" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_POS(mob);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = GET_POS(actor);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = GET_POS(vict);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = GET_POS(rndm);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'position'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  if ( !str_cmp( buf, "level" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GetMaxLevel( mob );
	          rhsvl = atoi( val );
	          return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	          {
		    lhsvl = GetMaxLevel( actor );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else 
		    return -1;
	case 't': if ( vict )
	          {
		    lhsvl = GetMaxLevel( vict );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	case 'r': if ( rndm )
	          {
		    lhsvl = GetMaxLevel( rndm );
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
		  }
	          else
		    return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'level'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  if ( !str_cmp( buf, "class" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_CLASS(mob);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = GET_CLASS(actor);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else 
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = GET_CLASS(vict);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = GET_CLASS(rndm);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'class'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "goldamt" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': lhsvl = GET_GOLD(mob);
	  rhsvl = atoi( val );
	  return mprog_veval( lhsvl, opr, rhsvl );
	case 'n': if ( actor )
	  {
	    lhsvl = GET_GOLD(actor);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 't': if ( vict )
	  {
	    lhsvl = GET_GOLD(vict);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    lhsvl = GET_GOLD(rndm);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'goldamt'",
		  mob_index[mob->nr].virt); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  if ( !str_cmp( buf, "objtype" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	  {
	    lhsvl = GET_OBJ_TYPE(obj);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj )
	  {
	    lhsvl = GET_OBJ_TYPE(v_obj);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'objtype'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "objval0" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	  {
	    lhsvl = GET_OBJ_VAL(obj, 0);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj )
	  {
	    lhsvl = GET_OBJ_VAL(v_obj, 0);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else 
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'objval0'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "objval1" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	  {
	    lhsvl = GET_OBJ_VAL(obj, 1);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj )
	  {
	    lhsvl = GET_OBJ_VAL(v_obj, 1);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'objval1'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "objval2" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	  {
	    lhsvl = GET_OBJ_VAL(obj, 2);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj )
	  {
		    lhsvl = GET_OBJ_VAL(v_obj, 2);
		    rhsvl = atoi( val );
		    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'objval2'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "objval3" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'o': if ( obj )
	  {
	    lhsvl = GET_OBJ_VAL(obj, 3);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj ) 
	  {
	    lhsvl = GET_OBJ_VAL(v_obj, 3);
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'objval3'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }
  
  if ( !str_cmp( buf, "mvar" ) ) {
     struct char_data *chd;
     unsigned int a, i;
     Variable *vtmp;
     switch ( arg[1] ) {
      case 'i': chd = mob; break;
      case 'n': chd = actor; break;
      case 'v': chd = vict; break;
      case 'r': chd = rndm; break;
      default: return 0;
     }
     if(!chd) return 0;
     for(vtmp=chd->player.vars;vtmp;vtmp=vtmp->next)
        if(!strcmp(arg+3, vtmp->name)) {
	   if(*val) {
	      i=0;
	      if(!is_number(val) || (vtmp->type == 1)) i=1;

	      if(i)
                return mprog_seval (vtmp->CValue(), opr, val);
	      else
		return mprog_veval (vtmp->Value(), opr, atoi(val));
	   } else {
	      return 1;
	   }
	}

     return 0;
  }	
   
  /* used to check if its a certain mob */
  /* boy this code was scewwwy too... fixed it up */
  /* hpefully this is now coded so the mobprog can see which
     mob is actually executing the script... ie: number($i) would 
     return the virtual number of the mob... */

  if ( !str_cmp( buf, "number" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{

	case 'i' : if (mob) {
	  if (IS_NPC(mob)) {
	    lhsvl = mob_index[mob->nr].virt;
	    rhsvl = atoi(val);
	    return mprog_veval(lhsvl, opr, rhsvl );
	  }
	}
	else return -1;

	case 'n': if ( actor )
	  {
	    if IS_NPC( actor )
	      {
		lhsvl = mob_index[actor->nr].virt;
		rhsvl = atoi( val );
		return mprog_veval( lhsvl, opr, rhsvl );
	      }
	  }
	else
	  return -1;

	case 't': if ( vict )
	  {
	    if IS_NPC( vict ) /* this used to say ACTOR.. dohhh */
	      {
		lhsvl = mob_index[vict->nr].virt;
		rhsvl = atoi( val );
		return mprog_veval( lhsvl, opr, rhsvl );
	      }
	  }
	else
	  return -1;
	case 'r': if ( rndm )
	  {
	    if IS_NPC( rndm )
	      {
		lhsvl = mob_index[rndm->nr].virt;
		rhsvl = atoi( val );
		return mprog_veval( lhsvl, opr, rhsvl );
	      }
	  }
	else return -1;
	case 'o': if ( obj )
	  {
	    lhsvl = obj_index[GET_OBJ_RNUM(obj)].virt;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	case 'p': if ( v_obj )
	  {
	    lhsvl = obj_index[GET_OBJ_RNUM(v_obj)].virt;
	    rhsvl = atoi( val );
	    return mprog_veval( lhsvl, opr, rhsvl );
	  }
	else
	  return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'number'",
		  mob_index[mob->nr].virt ); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  /* this code as it was before made it REALLY hard to check mob names
     but was great for players... mob names are longer though... so
     I changed it to is_abbrev.. this prolly isn't the best solution, but
     its a start.  Same occurs for items... ie  sword killing can now be
     referred to as sword kill :)
     */
  
  if ( !str_cmp( buf, "name" ) )
    {
      switch ( arg[1] )  /* arg should be "$*" so just get the letter */
	{
	case 'i': return mprog_seval_abbrev(ss_data(mob->player.name), opr, val );
	case 'n': if ( actor ) {
	  if (IS_NPC(actor)) 
	    return mprog_seval_abbrev(ss_data(actor->player.name), opr, val);
	  else
	    return mprog_seval( ss_data(actor->player.name), opr, val );
	}
	else
	  return -1;
	case 't': if ( vict ) {
	  if (IS_NPC(vict))
	    return mprog_seval_abbrev(ss_data(vict->player.name), opr, val);
	  else
	    return mprog_seval( ss_data(vict->player.name), opr, val );
	}
	else
	  return -1;
	case 'r': if ( rndm ) {
	  if (IS_NPC(rndm)) 
	    return mprog_seval_abbrev(ss_data(rndm->player.name), opr, val);
	  else
	    return mprog_seval( ss_data(rndm->player.name), opr, val );
	}
	else
	  return -1;
	case 'o': if ( obj )
	            return mprog_seval_abbrev( ss_data(obj->name), opr, val );
	          else
		    return -1;
	case 'p': if ( v_obj )
	            return mprog_seval_abbrev( ss_data(v_obj->name), opr, val );
	          else
		    return -1;
	default:
	  sprintf(buf2, "MPERR: Mob: %d bad argument to 'name'",
		  mob_index[mob->nr].virt); 
	  log_msg(buf2, LOG_MPROG);
	  return -1;
	}
    }

  /* Ok... all the ifchcks are done, so if we didnt find ours then something
   * odd happened.  So report the bug and abort the MOBprogram (return error)
   */
//  sprintf(buf2, "MPERR: Mob: %d unknown ifchck %s",
//	  mob_index[mob->nr].virt, buf); 
//  log_msg(buf2, LOG_MPROG);
  return -2;
}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
int mprog_do_ifchck( char *ifchck, struct char_data *mob,
		    struct char_data *actor, struct obj_data *obj,
		    void *vo, struct char_data *rndm) {

  char buf[ MAX_INPUT_LENGTH ];
  char arg[ MAX_INPUT_LENGTH ];
  char opr[ MAX_INPUT_LENGTH ];
  char val[ MAX_INPUT_LENGTH ];
//  struct char_data *vict = (struct char_data *) vo;
//  struct obj_data *v_obj = (struct obj_data  *) vo;
  char     *bufpt = buf;
  char     *argpt = arg;
  char     *oprpt = opr;
  char     *valpt = val;
  char     *point = ifchck;
  int       lhsvl=0;
  int       rhsvl=0;

  /* Min's safety feature... fasten your seatbelts */

  *buf = '\0';
  *arg = '\0';
  *opr = '\0';
  *val = '\0';

  if ( *point == '\0' )  {
    sprintf(buf2, "MPERR: Mob: %d null ifchck",
	    mob_index[mob->nr].virt); 
    log_msg(buf2, LOG_MPROG);
    return -1;
  }   
  /* skip leading spaces */
  while ( *point == ' ' )
    point++;
  
  /* get whatever comes before the left paren.. ignore spaces */
  while ( *point != '(' ) 
    if ( *point == '\0' )  { /* Null before (? */
      sprintf(buf2, "MPERR: Mob: %d ifchck syntax error",
	      mob_index[mob->nr].virt); 
      log_msg(buf2, LOG_MPROG);
      return -1;
    }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*bufpt++ = *point++; 
  
  *bufpt = '\0';
  point++;
  
  /* get whatever is in between the parens.. ignore spaces */
  while ( *point != ')' ) 
    if ( *point == '\0' ) {
      sprintf(buf2, "MPERR: Mob: %d ifchck syntax error",
	      mob_index[mob->nr].virt); 
      log_msg(buf2, LOG_MPROG);
      return -1;
    }   
    else
      if ( *point == ' ' )
	point++;
      else 
	*argpt++ = *point++; 
  
  *argpt = '\0';
  point++;
  
  /* check to see if there is an operator */
  while ( *point == ' ' )
    point++;
  if ( *point == '\0' )  {
    *opr = '\0';
    *val = '\0';
  }   
  else { /* there should be an operator and value, so get them */
    
    while ( ( *point != ' ' ) && ( !isalnum( *point ) ) ) 
      if ( *point == '\0' ) {
	sprintf(buf2, "MPERR: Mob: %d ifchck operator without value",
		mob_index[mob->nr].virt ); 
	log_msg(buf2, LOG_MPROG);
	return -1;
      }   
      else
	*oprpt++ = *point++; 

      *oprpt = '\0';
 
      /* finished with operator, skip spaces and then get the value */
      while ( *point == ' ' )
	point++;
      for( ; ; )
	{
	  if ( ( *point != ' ' ) && ( *point == '\0' ) )
	    break;
	  else
	    *valpt++ = *point++; 

	}

      *valpt = '\0';
    }
  bufpt = buf;
  argpt = arg;
  oprpt = opr;
  valpt = val;

  /* Ok... now buf contains the ifchck, arg contains the inside of the
   *  parentheses, opr contains an operator if one is present, and val
   *  has the value if an operator was present.
   *  So.. basically use if statements and run over all known ifchecks
   *  Once inside, use the argument and expand the lhs. Then if need be
   *  send the lhs,opr,rhs off to be evaluated.
   */
/*int mprog_do_ifchck( char *ifchck, struct char_data *mob,
                     struct char_data *actor, struct obj_data *obj,
                     void *vo, struct char_data *rndm) {
*/   
   return mprog_keyword_replace(buf, arg, opr, val, mob, actor, obj, vo, rndm, lhsvl, rhsvl);
}


/* this command eats between if and endif.. sorry this is a
comment to myself... */

char *mprog_eat_if( char *com_list) {
  char *cmnd, *morebuf, buf[MAX_INPUT_LENGTH];

  cmnd = com_list;
  com_list = mprog_next_command(com_list);
  morebuf = one_argument( cmnd, buf);

  while( str_cmp( buf, "endif") ) {
    if( !str_cmp( buf, "if") )
       com_list = mprog_eat_if(com_list);

    cmnd = com_list;
    com_list = mprog_next_command(com_list);
    morebuf = one_argument( cmnd, buf);
  }
  return com_list;
}

/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
char *mprog_process_if( char *ifchck, char *com_list, struct char_data *mob,
		       struct char_data *actor, struct obj_data *obj, void
*vo,
			struct char_data *rndm ) {

  static char null[ 1 ];
  char buf[ MAX_INPUT_LENGTH ];
  char *morebuf = '\0';
  char    *cmnd = '\0';
  int loopdone = FALSE;
  int     flag = FALSE;
  int  legal;
  
 *null = '\0';
 
 /* check for trueness of the ifcheck */
 if ( ( legal = mprog_do_ifchck( ifchck, mob, actor, obj, vo, rndm ) ) )
   if ( legal == -1 )
     return null;
   else
     flag = TRUE;
 
 while( loopdone == FALSE ) /*scan over any existing or statements */
   {
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );
     
     while ( *cmnd == ' ' || *cmnd == '\r' || *cmnd == '\n') /* added \n clause */
       cmnd++;
     
     if ( *cmnd == '\0' ) {
       sprintf(buf2, "MPERR: Mob: %d no commands after IF/OR",
	       mob_index[mob->nr].virt ); 
       log_msg(buf2, LOG_MPROG);
       return null;
     }
     
     morebuf = one_argument( cmnd, buf );
     
     
     /* MINLOOK */
     
     if ( !str_cmp( buf, "or" ) ) {
       if ( ( legal = mprog_do_ifchck( morebuf,mob,actor,obj,vo,rndm ) ) )
	 if ( legal == -1 )
	   return null;
	 else
	   flag = TRUE;
     }
     else
       loopdone = TRUE;
   }
 
 if ( flag )
   for ( ; ; ) /*ifcheck was true, do commands but ignore else to endif*/ 
     {
       if ( !str_cmp( buf, "if" ) )
	 { 
	   com_list = mprog_process_if(morebuf,com_list,mob,actor,obj,vo,rndm);
	   while ( *cmnd==' ' )
	     cmnd++;
	   if ( *com_list == '\0' )
	     return null;
	   cmnd     = com_list;
	   com_list = mprog_next_command( com_list );
	   morebuf  = one_argument( cmnd,buf );
	   continue;
	 }
       
       if ( !str_cmp( buf, "break" ) )
	 return null;
       
       if ( !str_cmp( buf, "endif" ) )
	 return com_list; 
       
       if ( !str_cmp( buf, "else" ) ) 
	 {
	   while ( str_cmp( buf, "endif" ) ) {
	     if( !str_cmp( buf, "if") )
	       com_list = mprog_eat_if(com_list);
	     
	   cmnd     = com_list;
	   com_list = mprog_next_command( com_list );
	   while ( *cmnd == ' ' )
	       cmnd++;
	     if ( *cmnd == '\0' )
	       {
		 sprintf(buf2, "MPERR: Mob: %d missing endif after else",
			 mob_index[mob->nr].virt );
		 log_msg(buf2, LOG_MPROG);
		 return null;
	       }
	     morebuf = one_argument( cmnd,buf );
	   }
	   return com_list; 
	 }

       mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
       cmnd     = com_list;
       com_list = mprog_next_command( com_list );

       while ( *cmnd == ' '  || *cmnd == '\r' || *cmnd=='\n' ) /* MINLOOK */
	 cmnd++;

       if ( *cmnd == '\0' )  {
	 sprintf(buf2, "MPERR: Mob: %d missing else or endif",
		 mob_index[mob->nr].virt ); 
	 log_msg(buf2, LOG_MPROG);
	 return null;
       }
       morebuf = one_argument( cmnd, buf );
     }
 else /*false ifcheck, find else and do existing commands or quit at endif*/
   {
     while ( ( str_cmp( buf, "else" ) ) && ( str_cmp( buf, "endif" ) ) )
       {
	 if( !str_cmp( buf, "if") )
	   com_list = mprog_eat_if(com_list);
	 
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );

	 while ( *cmnd == ' ' || *cmnd == '\r' || *cmnd == '\n'  ) /* MINLOOK */
	   cmnd++;

	 if ( *cmnd == '\0') {
	   sprintf(buf2, "MPERR: Mob: %d missing an else or endif",
		   mob_index[mob->nr].virt ); 
	   log_msg(buf2, LOG_MPROG);
	   return null;
	 }
	 morebuf = one_argument( cmnd, buf );
       }
     
     /* found either an else or an endif.. act accordingly */
     if ( !str_cmp( buf, "endif" ) )
       return com_list;
     cmnd     = com_list;
     com_list = mprog_next_command( com_list );

     while ( *cmnd == ' '  || *cmnd == '\r' || *cmnd == '\n')  /* MINLOOK */
       cmnd++;

     if ( *cmnd == '\0' ) { 
       sprintf(buf2, "MPERR: Mob: %d missing endif",
	       mob_index[mob->nr].virt ); 
       log_msg(buf2, LOG_MPROG);
       return null;
     }

     morebuf = one_argument( cmnd, buf );
     
     for ( ; ; ) /*process the post-else commands until an endif is found.*/
       {
	 if ( !str_cmp( buf, "if" ) )
	   { 
	     com_list = mprog_process_if( morebuf, com_list, mob, actor,
					  obj, vo, rndm );
	     while ( *cmnd == ' '  || *cmnd == '\n' || *cmnd == '\r' ) /* MINLOOK */
	       cmnd++;

	     if ( *com_list == '\0' )
	       return null;
	     cmnd     = com_list;
	     com_list = mprog_next_command( com_list );
	     morebuf  = one_argument( cmnd,buf );
	     continue;
	   }
	 if ( !str_cmp( buf, "else" ) ) 
	   {
	     sprintf(buf2, "MPERR: Mob: %d found else in an else section",
		  mob_index[mob->nr].virt ); 
	     log_msg(buf2, LOG_MPROG);
	     return null;
	   }
	 if ( !str_cmp( buf, "break" ) )
	   return null;
	 if ( !str_cmp( buf, "endif" ) )
	   return com_list; 
	 mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
	 cmnd     = com_list;
	 com_list = mprog_next_command( com_list );

	 while ( *cmnd == ' '  || *cmnd == '\r' || *cmnd == '\n' ) /* MINLOOK */
	   cmnd++;

	 if ( *cmnd == '\0' ) {
	   sprintf(buf2, "MPERR: Mob:%d missing endif in else section",
		   mob_index[mob->nr].virt ); 
	   log_msg(buf2, LOG_MPROG);
	   return null;
	 }
	 morebuf = one_argument( cmnd, buf );
       }
   }
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
void mprog_translate( char ch, char *t, struct char_data *mob,
		     struct char_data *actor, struct obj_data *obj,
		     void *vo, struct char_data *rndm ) {
 struct char_data   *vict             = (struct char_data *) vo;
 struct obj_data    *v_obj            = (struct obj_data  *) vo;
 char sendstr[MAX_STRING_LENGTH];
     
 *t = '\0'; /* init t to null */

 switch ( ch ) {
     case 'i':
         one_argument( GET_REAL_NAME(mob), t );
      break;

     case 'I':
         strcpy( t, ss_data(mob->player.short_descr) );
      break;

     case 'n':
         if ( actor ) {
	   if ( CAN_SEE( mob,actor ) ) {
             if ( !IS_NPC( actor ) ) {
               strcpy(t, GET_REAL_NAME(actor));
             } else
	       one_argument( GET_REAL_NAME(actor), t );
           } else
             strcpy(t, "Someone");
         }
      break;

     case 'N':
         if ( actor ) 
            if ( CAN_SEE( mob, actor ) )
	       if ( IS_NPC( actor ) )
		 strcpy( t, ss_data(actor->player.short_descr) );
	       else
	       {
		   strcpy( t, GET_REAL_NAME(actor));
		   strcat( t, " " );
		   strcat( t, ss_data(actor->player.title) );
	       }
	    else
	      strcpy( t, "someone" );
	 break;

     case 't':
         if ( vict ) {
	   if ( CAN_SEE( mob, vict ) ) {
             if ( !IS_NPC( vict ) )
               strcpy(t, ss_data(vict->player.name));
             else
	       one_argument( ss_data(vict->player.name), t );
           } else
             strcpy(t, "Someone");
         }
	 break;

     case 'T':
         if ( vict ) 
            if ( CAN_SEE( mob, vict ) )
	       if ( IS_NPC( vict ) )
		 strcpy( t, ss_data(vict->player.short_descr) );
	       else
	       {
		 strcpy( t, ss_data(vict->player.name) );
		 strcat( t, " " );
		 strcat( t, ss_data(vict->player.title) );
	       }
	    else
	      strcpy( t, "someone" );
	 break;
     
     case 'r':
         if ( rndm ) {
	   if ( CAN_SEE( mob, rndm ) ) {
             if ( !IS_NPC( rndm ) )
               strcpy(t, GET_REAL_NAME(rndm));
	     else
               one_argument( GET_REAL_NAME(rndm), t );
           } else
             strcpy(t, "Someone");
         }
      break;

     case 'R':
         if ( rndm ) 
            if ( CAN_SEE( mob, rndm ) )
	       if ( IS_NPC( rndm ) )
		 strcpy(t,ss_data(rndm->player.short_descr));
	       else
	       {
		 strcpy( t, ss_data(rndm->player.name) );
		 strcat( t, " " );
		 strcat( t, ss_data(rndm->player.title) );
	       }
	    else
	      strcpy( t, "someone" );
	 break;

     case 'e':
         if ( actor )
	   CAN_SEE( mob, actor ) ? strcpy( t, HSSH(actor)) 
	                         : strcpy( t, "someone" );
	 break;
  
     case 'm':
         if ( actor )
	   CAN_SEE( mob, actor ) ? strcpy( t, HMHR(actor)) 
	                         : strcpy( t, "someone" );
	 break;
  
     case 's':
         if ( actor )
	   CAN_SEE( mob, actor ) ? strcpy( t, HSHR(actor))
	                         : strcpy( t, "someone's" );
	 break;
     
     case 'E':
         if ( vict )
	   CAN_SEE( mob, vict ) ? strcpy( t, HSSH(vict)) 
	                        : strcpy( t, "someone" );
	 break;
  
     case 'M':
         if ( vict )
	   CAN_SEE( mob, vict ) ? strcpy( t, HMHR(vict)) 
	                        : strcpy( t, "someone" );
	 break;
  
     case 'S':
         if ( vict )
	   CAN_SEE( mob, vict ) ? strcpy( t, HSHR(vict))
                                : strcpy( t, "someone's" ); 
	 break;

     case 'j':
	 strcpy( t, HSSH(mob));
	 break;
  
     case 'k':
	 strcpy( t, HMHR(mob));
	 break;
  
     case 'l':
	 strcpy( t, HSHR(mob));
	 break;

     case 'J':
         if ( rndm )
	   CAN_SEE( mob, rndm ) ? strcpy( t, HSSH(rndm))
	                        : strcpy( t, "someone" );
	 break;
  
     case 'K':
         if ( rndm )
	   CAN_SEE( mob, rndm ) ? strcpy( t, HMHR(rndm))
                                : strcpy( t, "someone" );
	 break;
  
     case 'L':
         if ( rndm )
	   CAN_SEE( mob, rndm ) ? strcpy( t, HSHR(rndm))
	                        : strcpy( t, "someone's" );
	 break;

     case 'o':
         if ( obj )
	   CAN_SEE_OBJ( mob, obj ) ? strcpy(t, ss_data(obj->name))
                                   : strcpy( t, "something" );
	 break;

     case 'O':
         if ( obj )
	   CAN_SEE_OBJ( mob, obj ) ? strcpy( t, ss_data(obj->short_description ))
                                   : strcpy( t, "something" );
	 break;

      case 'p':
         if ( v_obj )
	   CAN_SEE_OBJ( mob, v_obj ) ? strcpy( t,  ss_data(v_obj->name))
                                     : strcpy( t, "something" );
	 break;

     case 'P':
         if ( v_obj )
	   CAN_SEE_OBJ( mob, v_obj ) ? strcpy( t, ss_data(v_obj->short_description ))
                                     : strcpy( t, "something" );
      break;

     case 'a':
         if ( obj ) 
          switch ( *( ss_data(obj->name) ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case 'A':
         if ( v_obj ) 
          switch ( *( ss_data(v_obj->name )) )
	  {
            case 'a': case 'e': case 'i':
	    case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
	 break;

     case '$':
         strcpy( t, "$" );
	 break;

     case '~':
         break;
  
     default:
         sprintf(buf2, "MPERR: Mob: %d bad $var", mob_index[mob->nr].virt);
	 log_msg(buf2, LOG_MPROG);
	 break;
       }
   
   strcpy(sendstr, t);
   format_string(sendstr, t, 0);

 return;

}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd( char *cmnd, struct char_data *mob,
			struct char_data *actor, struct obj_data *obj,
			void *vo, struct char_data *rndm ) {
  char buf[ MAX_INPUT_LENGTH ];
  char tmp[ MAX_INPUT_LENGTH ];
  char *str;
  char *i;
  char *point;
  
  
  point   = buf;
  str     = cmnd;
  
  while ( *str != '\0' )
    {
      if ( *str != '$' )
	{
	  *point++ = *str++;
	  continue;
	}
      str++;
      mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
      i = tmp;
      ++str;
      while ( ( *point = *i ) != '\0' )
	++point, ++i;
    }
  *point = '\0';

  command_interpreter( mob, buf, 1 );
/*
  if (mob->in_room <= NOWHERE) {
    sprintf(buf2, "After Command_interpreter: Mob: %s is not in not in a valid room, currently in: %ld", 
	    GET_IDENT(mob), mob->in_room);
    log_msg(buf2, LOG_MPROG);
  }
*/
  return;
  
}

/* The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver ( char *com_list, struct char_data *mob,
		   struct char_data *actor, struct obj_data *obj, void *vo) {
  
  char tmpcmndlst[ MAX_STRING_LENGTH ];
  char buf       [ MAX_INPUT_LENGTH ];
  char *morebuf;
  char *command_list;
  char *cmnd;
  struct char_data *rndm  = NULL;
  struct char_data *vch   = NULL;
  int        count = 0;
  
  if IS_AFFECTED( mob, AFF_CHARM )
		  return;
  
  /* get a random visable mortal player who is in the room with the mob */
  for ( vch = real_roomp(mob->in_room)->people; vch; vch = vch->next_in_room )
    if ( IS_PC( vch )
	 &&  !IS_IMMORTAL(vch)
	 &&  CAN_SEE( mob, vch ) )
      {
	if(!count) rndm=vch;
	if ( number( 0, count ) == 0 )
	  rndm = vch;
	count++;
      }
  
  strcpy( tmpcmndlst, com_list );
  command_list = tmpcmndlst;
  cmnd         = command_list;
  command_list = mprog_next_command( command_list );

  while ( *cmnd != '\0' )
    {
      morebuf = one_argument( cmnd, buf );
      morebuf = skip_spaces(morebuf);

      if ( !str_cmp( buf, "if" ) )
	command_list = mprog_process_if( morebuf, command_list, mob,
					 actor, obj, vo, rndm );
      else
	mprog_process_cmnd( cmnd, mob, actor, obj, vo, rndm );
      cmnd         = command_list;
      command_list = mprog_next_command( command_list );
    }
  
  return;
  
}

/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, struct char_data *mob,
			  struct char_data *actor, struct obj_data *obj,
			  void *vo, int type )
{
  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  size_t     i;

  for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
        while(isspace(*list)) list++;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		   && ( *(end = start + strlen( list ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| !*end) )
		{
		  mprog_driver( mprg->comlist, mob, actor, obj, vo );
		  return;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		     && ( *(end = start + strlen( word ) ) == ' '
			  || *end == '\n'
			  || *end == '\r'
			  || !*end) )
		  {
		    mprog_driver( mprg->comlist, mob, actor, obj, vo );
		    return;
		  }
		else
		  dupl = start+1;
	  }
      }
  return;
}

void mprog_percent_check( struct char_data *mob, struct char_data *actor,
			 struct obj_data *obj, void *vo, int type) {
 MPROG_DATA * mprg;

 for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
   if ( ( mprg->type & type )
	&& ( number(1,100) < atoi( mprg->arglist ) ) )
     {
       DLOG(("Mob (%s) executing Mprog type: %s.", GET_NAME(mob), 
	     mprog_type_to_name(type)));
       mprog_driver( mprg->comlist, mob, actor, obj, vo );
       if ( type != GREET_PROG && type != ALL_GREET_PROG && type != RAND_PROG )
	 break;
     }
 
 return;

}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */

void mprog_act_trigger( char *buf, struct char_data *mob, struct char_data *ch,
		       struct obj_data *obj, void *vo) {
  
  MPROG_ACT_LIST * tmp_act;
  
  if ( IS_NPC( mob ) && ( mob_index[mob->nr].progtypes & ACT_PROG ) ) {

    CREATE(tmp_act,MPROG_ACT_LIST,1);

    if ( mob->mpactnum > 0 )
      tmp_act->next = mob->mpact->next;
    else
      tmp_act->next = NULL;
    
    mob->mpact      = tmp_act;
    mob->mpact->buf = str_dup( buf );
    mob->mpact->ch  = ch; 
    mob->mpact->obj = obj; 
    mob->mpact->vo  = vo; 
    mob->mpactnum++;

  }
  return;
  
}

void mprog_bribe_trigger( struct char_data *mob, struct char_data *ch, 
			 int amount ) {

  MPROG_DATA *mprg;

  struct obj_data   *obj;

   
  if ( IS_NPC( mob )
      && ( mob_index[mob->nr].progtypes & BRIBE_PROG ) )
    {
      obj = create_money(amount);
      obj_to_char( obj, mob );
      GET_GOLD(mob) -= amount;

      for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
	if ( ( mprg->type & BRIBE_PROG )
	    && ( amount >= atoi( mprg->arglist ) ) )
	  {
	    mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	    break;
	  }
    }
  
  return;

}

void mprog_death_trigger( struct char_data *mob, struct char_data *killer ) {

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & DEATH_PROG ) )
   {
     GET_POS(mob) = POSITION_STANDING;
     mprog_percent_check( mob, killer, NULL, NULL, DEATH_PROG );
   }

 death_cry( mob );
 return;

}

void mprog_entry_trigger( struct char_data *mob )
{

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & ENTRY_PROG ) )
   mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );

 return;

}

void mprog_fight_trigger( struct char_data *mob, struct char_data *ch )
{

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & FIGHT_PROG ) )
   mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );

 return;

}

void mprog_give_trigger( struct char_data *mob, struct char_data *ch,
			 struct obj_data *obj ) {

 char        buf[MAX_INPUT_LENGTH];
 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & GIVE_PROG ) )
   for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
     {
       one_argument( mprg->arglist, buf );
       if ( ( mprg->type & GIVE_PROG )
	   && ( ( !str_infix( ss_data(obj->name), mprg->arglist ) )
	       || ( !str_cmp( "all", buf ) ) ) )
	 {
	   mprog_driver( mprg->comlist, mob, ch, obj, NULL );
	   break;
	 }
     }

 return;

}

void mprog_greet_trigger( struct char_data *ch ) {

  struct char_data *vmob;
  
  
  for ( vmob = real_roomp(ch->in_room)->people; vmob != NULL; vmob = vmob->next_in_room ) {
     if ( IS_NPC( vmob )
       && ch != vmob
       && CAN_SEE( vmob, ch )
       && !vmob->specials.fighting
       && AWAKE( vmob )
       && ( mob_index[vmob->nr].progtypes & GREET_PROG) )
     mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
   else
     if ( IS_NPC( vmob )
	 && ch != vmob
	 && !vmob->specials.fighting
	 && AWAKE( vmob )
	 && ( mob_index[vmob->nr].progtypes & ALL_GREET_PROG ) )
       mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
 }

 return;

}

void mprog_health_trigger( struct char_data *mob, struct char_data *ch) {

 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob_index[mob->nr].progtypes & HEALTH_PROG ) )
   for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next )
     if ( ( mprg->type & HEALTH_PROG )
	 && (( 100*GET_HIT(mob) / GET_MAX_HIT(mob)) < atoi( mprg->arglist )))
       {
	 mprog_driver( mprg->comlist, mob, ch, NULL, NULL );
	 break;
       }
 
 return;

}

void mprog_random_trigger( struct char_data *mob ) {
  
  if ( mob_index[mob->nr].progtypes & RAND_PROG)
    mprog_percent_check(mob,mob,NULL,NULL,RAND_PROG);
  
  return;
  
}

void mprog_speech_trigger( char *txt, struct char_data *mob ) {
  
  struct char_data *vmob;
  
  for ( vmob = real_roomp(mob->in_room)->people; vmob != NULL; vmob = vmob->next_in_room )
    if ( IS_NPC( vmob ) && ( mob_index[vmob->nr].progtypes & SPEECH_PROG ) )
      mprog_wordlist_check( txt, vmob, mob, NULL, NULL, SPEECH_PROG );
  
  return;

}

/* Spell Trigger (Quilan project) */

void mprog_spell_trigger( struct char_data *mob, struct char_data *ch, int spell_num) {

    int action=0;
    char buf[MAX_STRING_LENGTH];
    MPROG_DATA *mprg;

    if (!mob || !ch) return;
    if (mob == ch) return;
    if (mob->in_room <= NOWHERE) return;
    if (GET_POS(mob) >= POSITION_SITTING) return;
	
    if ( IS_NPC( mob ) && ( mob_index[mob->nr].progtypes & SPELL_PROG ) )
        for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next ) {
            if ( ( mprg->type & SPELL_PROG )) {
               one_argument(mprg->arglist, buf);
	       action=0;
	       if (spell_num == atoi( buf )) 		action=1; 
	       if (!strcmp(buf, "all"))			action=1;
	       if ((!strcmp(buf, "offensive")) &&
		   Is_Offensive_Spell(spell_num))	action=1;
	       if ((!strcmp(buf, "areafx")) &&
		   Is_AreaFX_Spell(spell_num))		action=1;
	       if ((!strcmp(buf, "defensive")) &&
		   !Is_Offensive_Spell(spell_num))	action=1;
	       if ((!strcmp(buf, "heal")) &&
		   Is_Heal_Spell(spell_num))		action=1;
	       if(action)
	         mprog_driver( mprg->comlist, mob, ch, NULL, NULL );
            }
        }
    return;
}

//Weather Trigger (Quilan's 2'nd project)
void mprog_weather_trigger(struct char_data *mob, int timeofday) {
   int action=0;
   char buf[MAX_STRING_LENGTH];
   MPROG_DATA *mprg;
   
   if (!mob) return;
   if (mob->in_room <= NOWHERE) return;
   
   if ( IS_NPC( mob ) && ( mob_index[mob->nr].progtypes & WEATHER_PROG ) )
      for ( mprg = mob_index[mob->nr].mobprogs; mprg != NULL; mprg = mprg->next ) {
      	 if ( ( mprg->type & WEATHER_PROG )) {
	    one_argument(mprg->arglist, buf);
	    action=0;
            if (timeofday == atoi( buf ))       action=1;
	    if (!strcmp(buf, "all"))            action=1;
	    if ((!strcmp(buf, "night")) &&
                timeofday==20)		        action=1;
            if ((!strcmp(buf, "midnight")) &&
	        timeofday==24) 		        action=1;
            if ((!strcmp(buf, "morning")) &&
	        timeofday==8)      	     	action=1;
            if ((!strcmp(buf, "noon")) &&
                timeofday==12)         		action=1;
            if (action)
                mprog_driver( mprg->comlist, mob, NULL, NULL, NULL );
         }
      }
   return;
}

     

/* convert types to values */

int mprog_name_to_type ( char *name ) {
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "health_prog"  ) )	return HEALTH_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
   if ( !str_cmp( name, "spell_prog"     ) )    return SPELL_PROG;
   if ( !str_cmp( name, "weather_prog"   ) )    return WEATHER_PROG;
   return( ERROR_PROG );
}

char *mprog_type_to_name (int progtype) {
  switch (progtype) {
  case IN_FILE_PROG: return "IN_FILE_PROG";
  case ACT_PROG: return "act_prog";
  case SPEECH_PROG: return "speech_prog";
  case RAND_PROG: return "rand_prog";
  case FIGHT_PROG: return "fight_prog";
  case HEALTH_PROG: return "health_prog";
  case DEATH_PROG: return "death_prog";
  case ENTRY_PROG: return "entry_prog";
  case GREET_PROG: return "greet_prog";
  case ALL_GREET_PROG: return "all_greet_prog";
  case GIVE_PROG: return "give_prog";
  case BRIBE_PROG: return "bribe_prog";
  case SPELL_PROG: return "spell_prog";
  case WEATHER_PROG: return "weather_prog";
  default: return "ERROR_PROG";
  }
  return "error_prog";
}

/* This procedure is responsible for reading any in_file MOBprograms.
 */

int read_mobprog(FILE *mobfp, MPROG_DATA *mprg) {
  int  letter;
  char line_buffer[MAX_INPUT_LENGTH];
  char command_buffer[MAX_STRING_LENGTH];
  char argument_buffer[MAX_INPUT_LENGTH];
  char *progtype_buffer;
  char *strptr;
  bool done;

  command_buffer[0] = '\0'; /* safety */

  if (feof(mobfp) != 0) {
    log_msg("Mobprog error: empty mobprog file.", LOG_MPROG);
    return TRUE;
  }

  if ( ( letter = fgetc( mobfp ) ) != '>' ) {
    if (letter == '@') {
      DLOG(("               end of mobprog file..."));
      while(fgetc(mobfp) != EOF); /* Eat everything left around */
      return TRUE;                /* Not really an error, but no mobprog to add */
    }
    DLOG(("Damn letter is :%d, should be :%d\n", letter, '>'));
#ifdef DEBUGJAN
    while(!feof(mobfp))
      putchar(fgetc(mobfp));
#endif

    log_msg("Mobprog error: mobprogram not in right format: no > at begin",
LOG_MPROG);
    sprintf(command_buffer, "The Letter was: %c!!!", letter);
    log_msg(command_buffer, LOG_MPROG);
    return TRUE;
  }
    
  if (feof(mobfp) != 0) {
    log_msg("Mobprog error: Unexpected end of file reading program type",
LOG_MPROG);
    return TRUE;
  }

  progtype_buffer = fread_word(mobfp);
  if (progtype_buffer == NULL) {
    log_msg("Mobprog error: reading program type", LOG_MPROG);
    return TRUE;
  }

  mprg->type = mprog_name_to_type(progtype_buffer);
   
  /* sprintf(buf,"    program: %s",progtype_buffer); */ /* show which prog we read (let's not.*/
  /* log_msg(buf); */

  if (mprg->type == ERROR_PROG) {
    log_msg("Mprog error: Invalid program type", LOG_MPROG);
    DLOG(("Error occured with: %s", progtype_buffer));
    return TRUE;
  }

  if (mprg->type == IN_FILE_PROG) {
    log_msg("Mprog error: In_file_progs are not implemented yet -- i hate recursion", LOG_MPROG);
    return TRUE;
  }
  
  if (feof(mobfp) != 0) {
    log_msg("Mobprog error: end of file reached reading mobprog parameter",
LOG_MPROG);
    return TRUE;
  }

  fgets(argument_buffer, MAX_INPUT_LENGTH, mobfp); /* read in the argument */

  for (strptr = argument_buffer; *strptr != '\0' ; strptr++)  /* routine to clear \n's */
    if (*strptr == '\n' || *strptr == '\r') {
      *strptr = '\0';
      break;
    }

  if (feof(mobfp) != 0) {
    log_msg("Mobprog error: No actual program", LOG_MPROG);
    return TRUE;
  }

  done = FALSE;

  while ((feof(mobfp) == 0) && !done) {
    if (fgets(line_buffer,MAX_INPUT_LENGTH,mobfp) == NULL) { /* error on reading */
      log_msg("Mprog Error: Reading commands", LOG_MPROG);
      return TRUE;
    }

    if (line_buffer[0] == '~') /* end of program */
      done = 1;
    else
      if (strlen(command_buffer) + strlen(line_buffer) > (MAX_STRING_LENGTH-1)) {
	log_msg("Mobprog error: Mobprogram is too long", LOG_MPROG);
	return TRUE;
      }
      else
	strcat(command_buffer,line_buffer); /* add it to the buffer */

  } /* end of while */


  /* ok we made it this far lets dump it into the MPROG_DATA struct and return it */

  CREATE(mprg->arglist, char, strlen(argument_buffer) + 1);
  CREATE(mprg->comlist, char, strlen(command_buffer) + 1) ;

  mprg->arglist[0] = '\0';
  mprg->comlist[0] = '\0';

  strcat(mprg->arglist,argument_buffer); /* better handling of strings */
  strcat(mprg->comlist,command_buffer); 

  return FALSE; /* no error */

}


void read_mobprogfile(char *prgfile, MOB_INDEX_DATA *Mob) {
  MPROG_DATA *mprg = NULL;
  FILE *mobfp;

  char buf[MAX_INPUT_LENGTH];

  mobfp = fopen(prgfile,"r"); /* open the file */
  if (!mobfp) {
    sprintf(buf,"Mobprog error: unable to locate: %s\n\r",prgfile);
    log_msg(buf, LOG_MPROG);
    return;
  }

  while (!feof(mobfp)) { /* While not end of file */
    CREATE(mprg,MPROG_DATA,1); /* make me a block of memory */
    if (read_mobprog(mobfp,mprg)) {/* read me one program and check for error*/
      FREE(mprg);
      if (!feof(mobfp)) { 
	sprintf(buf,"Mobprog Error: Unable to read mobprogram --> %s",prgfile);
	log_msg(buf, LOG_MPROG);
	fclose(mobfp);
	break;
      }
    }
    else { /* add it to the beginning of list */
      mprg->next = Mob->mobprogs;
      Mob->mobprogs = mprg;
      Mob->progtypes |= mprg->type; /* set flag */
    }
  } /* end of while */

  fclose(mobfp);
}


void boot_mprog() {
  FILE *inifile;
  char buf[MAX_INPUT_LENGTH]; /* doh I hate using 4k buffers */
  int vnum, rnum;
  char mprog_name[MAX_INPUT_LENGTH];
  char progfile[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *Mob;

#ifdef DEBUGJAN
  MPROG_DATA *mprg;
#endif


  if (!(inifile = fopen(INIFILE, "r"))) {
    perror(INIFILE);
    log_msg("Unable to retrieve MOBPROGRAMS... mobprog ini file doesn't exist!");
  }
  
  do  {
    fscanf(inifile, "%d %s",&vnum, mprog_name);

#ifdef DEBUGJAN
    sprintf(buf,"mprog read: %d, %s",vnum, mprog_name);
    log_msg(buf, LOG_MPROG);
#endif

    /* figure out what real number it is */
    rnum = real_mobile(vnum);
    if (rnum < 0) {
      sprintf(buf,"vnum: %d is invalid in the mobprog ini file",vnum);
      log_msg(buf, LOG_MPROG);
    }
    else { /* valid vnum, realnum, lets make some pointers */
      Mob = &mob_index[rnum];
      /* create the filename */
      sprintf(progfile,"%s%s.prg",PRGDIR,mprog_name);
      read_mobprogfile(progfile,Mob);
#ifdef DEBUGJAN
      log_msg("debug:: ", LOG_MPROG);
      sprintf(buf,"mobname:: %s",Mob->name);
      log_msg(buf, LOG_MPROG);
      for (mprg=Mob->mobprogs;mprg;mprg=mprg->next) {
	sprintf(buf,"program %d == %s",mprg->type,mprg->arglist);
	log_msg(buf, LOG_MPROG);
      }
#endif
  
    }
  } while (feof(inifile) == 0);
  
  fclose(inifile);
  

}






