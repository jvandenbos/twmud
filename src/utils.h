#include "vnum_mob.h"

int CAN_SEE(struct char_data *s, struct char_data *o);
int CAN_SEE_OBJ(struct char_data *ch, struct obj_data *obj);

#define TRUE  1
#define FALSE 0
#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )
#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IS_WEAPON(o) (o->obj_flags.type_flag == ITEM_WEAPON)
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#if MEM_DEBUG

void* debug_calloc(size_t, int, const char*, int);
void* debug_realloc(void*, size_t, int, const char*, int);
void debug_free(void*, const char*, int);
void debug_check(void* ptr, const char* file, int line);

#define CREATE(result, type, number) \
    do { (result) = (type*) debug_calloc(sizeof(type), (number),\
					__FILE__, __LINE__); } while(0)
#define RECREATE(result, type, number) \
    do { (result) = (type*) debug_realloc((result), sizeof(type), (number), \
				__FILE__, __LINE__); } while(0)
#define FREE(obj) \
    do { debug_free((obj), __FILE__, __LINE__); (obj) = 0; } while(0)

#else

#define CREATE(result, type, number) \
do { \
    if (!((result) = (type *) calloc ((number), sizeof(type)))) \
    { \
	perror("malloc failure"); \
	fprintf(stderr, \
		"malloc failed in File %s, Line %d\n", \
		__FILE__, __LINE__); \
	abort(); \
    } \
} while(0)

#define RECREATE(result,type,number) \
do { \
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number)))) \
    { \
	perror("realloc failure"); \
	fprintf(stderr, \
		"realloc failed in File %s, Line %d\n", \
		__FILE__, __LINE__); \
	abort(); \
    } \
} while(0)

#define FREE(obj)		do { free(obj); (obj) = 0; } while(0)

#endif


#define IS_SET(flag,bit)  ((flag) & (bit))

#define IS_WRITING(ch) ((ch->desc) && (ch->desc->str))

#define IS_AFK(ch) (IS_SET(ch->specials.flags, PLR_AFK))

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define AFF_FLAGS(ch)		((ch)->specials.affected_by[0])
#define AFF2_FLAGS(ch)		((ch)->specials.affected_by[1])
#define IS_AFFECTED(ch,skill)	(IS_SET(AFF_FLAGS(ch), (skill)))
#define IS_AFFECTED2(ch,skill)	(IS_SET(AFF2_FLAGS(ch), (skill)))

/*
#define IS_DARK(room)  ((real_roomp(room)->light<=0) && ((IS_SET(real_roomp(room)->room_flags, DARK)) || (real_roomp(room)->dark)))

#define IS_LIGHT(room)  (real_roomp(room)->light>0 || (!IS_SET(real_roomp(room)->room_flags, DARK) || !real_roomp(room)->dark))
*/

/* faster version */
#define IS_DARK(room)  (((room)->light<=0) && ((IS_SET((room)->room_flags, DARK)) || ((room)->dark)))

#define IS_LIGHT(room)  ((room)->light>0 || (!IS_SET((room)->room_flags, DARK) || !(room)->dark))

#define IS_TEMPLE(room) (IS_SET((room)->room_flags,TEMPLE))

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define RM_FLAGS(i)  ((real_roomp(i))?real_roomp(i)->room_flags:0)

#define GET_LEVEL(ch, i)   ((ch)->player.level[i])

#define GET_HIGH_LEVEL(ch, i) ((ch)->player.max_level[i])

/*PAC*/
#define GET_GUILD(ch) ((ch)->in_guild)

/*PAC*/
#define GET_GUILD_LEVEL(ch) ((ch)->guild_level)

#define GET_CLASS_TITLE(ch, class, lev)   ((ch)->player.sex ?  \
   (((ch)->player.sex == 1) ? titles[(class)][(lev)].title_m : \
    titles[(class)][(lev)].title_f) : titles[(class)][(lev)].title_m)

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) (char *) (((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "his" : "her") : "its"))

#define UHSHR(ch) (char *) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "His" : "Her") : "Its")

#define HSSH(ch) (char *) (((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it"))

#define UHSSH(ch) (char *) (((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "He" : "She") : "It"))

#define HMHR(ch) (char *) (((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it"))	

#define UHMHR(ch) ((ch)->player.sex ?                                   \
		      (((ch)->player.sex == 1) ? "Him" : "Her") : "It")


#define ANA(obj) (char *) (index("aeiouyAEIOUY", *OBJ_NAME(obj)) ? "An" : "A")

#define SANA(obj) (char *) (index("aeiouyAEIOUY", *OBJ_NAME(obj)) ? "an" : "a")

#define IS_POLY_PC(ch)  IS_SET(ch->specials.mob_act, ACT_POLYSELF) && IS_PC(ch)

#define IS_NPC(ch)  (IS_SET((ch)->specials.mob_act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.mob_act, ACT_ISNPC) && ((ch)->nr >-1))

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_IDENT(ch)	ss_data((ch)->player.name)
    
#define GET_REAL_NAME(ch) ss_data(real_character(ch)->player.name)
    
#define GET_NAME(ch)    ss_data((ch)->player.short_descr)

#define GET_TITLE(ch)   ss_data((ch)->player.title)

#define GET_CLASS(ch)   ((ch)->player.clss)

#define GET_HOME(ch)	((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define RACE_ATT(ch)    Find_Att_by_Race(ch)

#define GET_STR(ch)     MAX(1, MIN((ch)->abilities.str, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->str_max)))
			     
#define GET_ADD(ch)     MAX(0,MIN((ch)->abilities.str_add, 100))

#define GET_DEX(ch)     MAX(1, MIN((ch)->abilities.dex, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->dex_max)))
			     
#define GET_INT(ch)     MAX(1, MIN((ch)->abilities.intel, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->int_max)))

#define GET_WIS(ch)     MAX(1, MIN((ch)->abilities.wis, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->wis_max)))

#define GET_CON(ch)     MAX(1, MIN((ch)->abilities.con, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->con_max)))

#define GET_CHA(ch)     MAX(1, MIN((ch)->abilities.cha, (IS_NPC(ch) || \
			  (IS_GOD(ch)) ? 25 : RACE_ATT(ch)->cha_max)))

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_WIMPY(ch)   ((ch)->specials.wimpy)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_BANK(ch)    ((ch)->points.bankgold)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_RACE(ch)     ((ch)->race)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING && \
		   !IS_AFFECTED(ch, AFF_PARALYSIS) )

#define WAIT_STATE(ch, cycle) \
   if(!IS_IMMORTAL(charm_master(ch)) && !mob_wait_state(ch,cycle) && (charm_master(ch)->desc)) \
      charm_master(ch)->desc->wait = (cycle)

/* Object And Carry related macros */

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)
#define GET_WPN_DMG_TYPE(obj) \
            (obj->obj_flags.value[3]==12  ? "RANGED" : (obj->obj_flags.value[3]>9  ? "SLASH" : (obj->obj_flags.value[3]>4  ? "PIERCE" : "BLUNT" )))

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_RENT(obj)	((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight + \
			     (obj)->obj_flags.cont_weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GetMaxLevel(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)	(char*) ((CAN_SEE((vict), (ch)) ? \
                         ss_data((ch)->player.short_descr) : "someone"))

#define PERL(ch)     	ss_data((ch)->player.long_descr)

#define OBJ_NAME(obj)	((obj)->name ? ss_data((obj)->name) : 0)
#define OBJ_DESC(obj)	((obj)->description ? \
			 ss_data((obj)->description) : 0)
#define OBJ_SHORT(obj)	((obj)->short_description ? \
			 ss_data((obj)->short_description) : 0)
#define OBJ_ACTION(obj)	 ((obj)->action_description ? \
			 ss_data((obj)->action_description) : 0)

#define OBJS(obj, vict)	(char *) ((CAN_SEE_OBJ((vict), (obj)) ? \
                         OBJ_SHORT(obj) : "something"))

#define OBJN(obj, vict) (char *) ((CAN_SEE_OBJ((vict), (obj)) ? \
			 fname(OBJ_NAME(obj)) : "something"))

#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags,INDOORS))

#define UNDERWATER(ch) (real_roomp((ch)->in_room)->sector_type \
                         == SECT_UNDERWATER)


#define IS_IMMORTAL(ch)	(IS_PC(ch) ? IS_GOD(ch) : \
                         IS_SET((ch)->specials.mob_act, ACT_IMMORTAL))

#define IS_POLICE(ch) ((mob_index[ch->nr].virt == VMOB_70) || \
                       (mob_index[ch->nr].virt == VMOB_72) || \
                       (mob_index[ch->nr].virt == VMOB_71))
#define IS_CORPSE(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
			(obj)->obj_flags.value[2] && \
			isname("corpse", OBJ_NAME(obj)))

#define IS_CARCASS(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
			(obj)->obj_flags.value[2] && \
			isname("carcass", OBJ_NAME(obj)))

#define IS_PC_CORPSE(obj) ((IS_CORPSE(obj)) && \
                           ((obj)->obj_flags.value[2] == PC_CORPSE))

#define IS_NPC_CORPSE(obj) ((IS_CORPSE(obj)) && \
                            ((obj)->obj_flags.value[2] == NPC_CORPSE))

#define EXIT(ch, door)  (real_roomp((ch)->in_room)->dir_option[door])

int exit_ok(struct room_direction_data *, struct room_data **);

#define CAN_GO(ch, door) (EXIT(ch,door)&&real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
			  real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define ITEM_TYPE(obj)  ((int)(obj)->obj_flags.type_flag)

#define MOUNTED(ch) ((ch)->specials.mounted_on)
#define RIDDEN(ch) ((ch)->specials.ridden_by)


#define IS_PC(ch) (!IS_NPC((ch)) || IS_SET((ch)->specials.mob_act, ACT_POLYSELF))

#define TRUST(ch)    (IS_NPC(ch) ? 0 : (ch)->player.trust)
#define IS_GOD(ch)   ((ch)->player.trust)

/* Added by MIN for PURE class (1 class) check */

#define IS_PURE_ITEM(obj)  (IS_SET((obj)->obj_flags.extra_flags,ITEM_PURE_CLASS))
#define IS_PURE_CLASS(ch)  (HowManyClasses(ch) == 1)
#define IS_MULTI(ch) (HowManyClasses(ch) > 1)

#define STATE(d) ((d)->connected)

#include "limits.h"
    
/* ADDED BY JAN (Min) 1996 to support BAN code */

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\


/* ----------------------------------------------------
 * The following were added by Min 1996 to support MOBPROGS
 * I felt they had good general application, so I put them in here
 * instead of local to mobprogs... If you don't like it...
 * 1) remove it and fix mobprogs :)
 * 2) send a SASE to The Avatar in Dojo... Hand delivered!
 ------------------------------------------------------ */

#define IN_ROOM(name)       name->in_room
#define GET_OBJ_TYPE(obj)       ((obj)->obj_flags.type_flag)
#define GET_OBJ_VAL(obj,val) ((obj)->obj_flags.value[(val)])
#define GET_OBJ_RNUM(obj)    ((obj)->item_number)
#define ROOM_FLAGGED(roomnum,roomflag) \
        IS_SET(real_roomp(roomnum)->room_flags, roomflag)
#define GET_MOB_VNUM(themob) mob_index[(themob)->nr].virt
#define GET_OBJ_VNUM(theobj) obj_index[(theobj)->item_number].virt



/* more hacks by Min */

#define FIND_INDIV 0
#define FIND_ALL 1
#define FIND_ALLDOT 2

/* END */

#define IS_FIGHTING(ch) ((ch)->specials.fighting ? 1 : 0)

/* ok I foresee that this may be used elsewhere, so i put it here */

#define HEALTH(ch) ((int) ((float) GET_HIT((ch)) * 100.0 / (float) GET_MAX_HIT((ch))))

