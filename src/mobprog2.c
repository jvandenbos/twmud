
/*
    mobprog2.c was written originally by John Reynolds from scratch
    in the span of about 3 days. I really tried to get everything
    to be 100% bug free, but chances are something may crash or
    whatever at some point. Comments will be added to my code at
    some point hopefully.
*/

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "structs.h"
#include "db.h"
#include "varfunc.h"
#include "find.h"
#include "proto.h"
#include "interpreter.h"
#include "utility.h"
#include "comm.h"
#include "fight.h"
#include "opinion.h"
#include "track.h"
#include "util_str.h"
#include "config.h"

int which_number_mobile(char_data *, char_data *);

#define CLEARWS(p) while(*p && isspace(*p)) p++
#define CLEARNWS(p) while(*p && (isalnum(*p) || (*p == '_'))) p++
#define CLEAROPR(p) while(*p && !isspace(*p) && !isalnum(*p) && (*p != '_') && (*p != '(') && (*p != '\"')) p++
#define CLEARCHAR(p,c) while(*p && (*p != c)) p++
#define CLEARCHAR2(p,c,c2) while(*p && (*p != c) && (*p != c2)) p++

#define _RETURN   1
#define _BREAK    2
#define _CONTINUE 3

class Variable;
class Function;

static void GetMPFunctions(char*,int,long,void*,int,char);
static char *HandleCompStatement(char*,int,long,void*,int,char);
static char *GetBtwChars(char *, char, char, Function *F=NULL);
void ReadSingleMobprog(char *,long,int,int,char);
void ReadSingleObjprog(char *,long,int,int,char);
void ReadSingleRoomprog(char*,long,int,int,char);
void ReadSingleZoneprog(char*,long,int,int,char);

static void EvalFunc(Function *, int, Variable**, Variable*);
static int  EvalLineType(Function *, char *);
static int  EvalMultipleLines(Function *, char *, Variable*);
static int  EvalExpression(Function *, char *, Variable *);
static int  IsOperator(char);
static char *EvalLength(Function *, char *);

static char *HandleEmbeddedCall(Function *, char *, Variable*);
static char *HandleFuncCall(Function *, char *, Variable*);
static void HandleReturnCall(Function *, char *, Variable*);
static void HandleAssignCall(Function *, char *);
static char *HandleIfCall(Function *, char *, Variable*, int *);
static void HandleLiteralCall(Function *, char *);
static char *HandleBracketedCall(Function *, char *, Variable*, int *);

static void HandleSpecificFunc(Function*,char*,int,Variable*,Variable*);
static int HandleSpecificBrFunc(Function*,char*,char*,char*,Variable*);

static int Restricted_FuncName(char *);
static int Embedded_FuncName(char *);
static int Bracketed_FuncName(char *);
static Variable *GetVar(Function *, char *);
static Variable *GetVarList(char *);
static Function *GetFunc(char *);
static Function *NewFunc();
static int FindSubStr(char *, char *, unsigned int);
static void ClearParents(Function *);
static void SetParents(Function *, Variable *, void *, int);

//These are really procedures used in simple RSA, I just wanted a nice randomness
static long multmod(long, long, long);
static long expomod(long, long, long);

//var types 0=long, 1=string, 2=mob*, 3=obj*, 4=room*, 10=data <unknown>

static Function *TheFuncList=NULL;
static Variable *TheVarList=NULL;
static int NumFuncs;

extern struct index_data *mob_index;
extern struct index_data *obj_index;

///////////////////////////////////////////////

static const char *mprog_trigger_list[] = {
   //mob triggers
   "rand_prog",
   "all_greet_prog",
   "greet_prog",
   "give_prog",
   "weather_prog",
   "speech_prog",
   "death_prog",
   "fight_prog",
   "entry_prog",
   "kill_prog",
   "command_prog",

   //obj triggers
   "wear_prog",
   "remove_prog",
   "fight_prog",
   "get_prog",
   "drop_prog",
   "rand_prog",

   //room triggers
   "speech_prog",
   "command_prog",
   "rand_prog",

   //zone triggers
   "death_prog",
   "arena_prog",
     
   NULL,
};

//add to this list if you create any new embedded functions.

static const char *mprog_embedded_list[] = {
   "Dice",
   "GetCarriedBy",
   "GetChar",
   "GetCharInRoom",
   "GetClan",
   "GetDesc",
   "GetEquippedBy",
   "GetFlags",
   "GetInRoom",
   "GetInsideOf",
   "GetLong",
   "GetName",
   "GetNum",
   "GetOnChar",
   "GetObj",
   "GetObjEquip",
   "GetShort",
   "GetTime",
   "GetUName",
   "GetVar",
   "GetZone",
   "HasObj",
   "HmHr",
   "HsHr",
   "HsSh",
   "InStr",
   "IsNpc",
   "IsFighting",
   "IsFollowing",
   "IsImmortal",
   "IsNumber",
   "IsPeaceful",
   "IsTracking",
   "Left",
   "LoadObjOnChar",
   "Log",
   "Mid",
   "Num",
   "Number",
   "Rand",
   "Right",
   "SendCharToRoom",
   "SendToChar",
   "SendToRoom",
   "SendToRoomExcept",
   "SendToRoomExceptTwo",
   "SetFlags",
   "SetHuntChar",
   "SetHuntObj",
   "SetHuntRoom",
   "SetDesc",
   "SetLong",
   "SetName",
   "SetShort",
   "SetVar",
   "SetWaitState",
   "Str",
   "StrLen",
   "StringName",
   "StopFighting",
   "StopTracking",
   "ToUpper",
   "ToLower",
   "Val",
   "VarType",
   NULL
};

//add to this list if you create any new embedded functions.

struct bracketed_struct {
   char *name;
   int eval;
};

static const bracketed_struct mprog_bracketed_list[] = {
   { "DoAt", 1 },
   { "DoWith", 1 },
   { "ForEachMobInDir", 1 },
   { "ForEachMobInGroup", 1 },
   { "ForEachMobInRoom", 1 },
   { "ForEachMobInZone", 1 },
   { "MakeString", 1 },
   { "While", 0 },
   { NULL, 0 }
};

///////////////////////////////////////////////

int boot_mprog2(char reload) {
   FILE *fl;
   char buf[512], *ptr;
   long num;
   char namebuf[512];
   
   log_msg("Starting mobprog read");
   
   if(!(fl=fopen("../lib/progs/mobprog.ini","r"))) {
      log_msg("Couldn't find mobprog ini file!");
      return 0;
   }

   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;

      if(sscanf(buf, "%ld %s", &num, namebuf) != 2)
	continue;

      ReadSingleMobprog(namebuf, num, reload, 0, reload);
   }

   fclose(fl);

   return 1;
}

void ReadSingleMobprog(char *filename, long vnum, int reload, int depth, char logit) {
   FILE *fl;
   char buf[512], *ptr;
   char *BigBuf=NULL;
   index_data *curmob;
   int rnum;
   
   sprintf(buf, "../lib/progs/mobprogs/%s.prg", filename);
   if(!(fl=fopen(buf, "r"))) {
      mlog(NULL, "Couldn't read mobprog2 file '%s'", buf);
      return;
   }
   
   if(!depth && logit) {
      mlog(NULL, "Reading mobprog2 file '%s'", filename);
   }

   fseek(fl,0,SEEK_END);
   BigBuf = new char[ftell(fl)+1];
   fseek(fl,0,SEEK_SET);
   strcpy(BigBuf, "");

   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
        break;
   
      strcat(BigBuf, buf);
   }

   fclose(fl);

   //find the char
   rnum=real_mobile(vnum);
   if(rnum < 0) {
      mlog(NULL, "Invalid vnum supplied in mobprog2.ini: %i", vnum);
      return;
   }
   
   curmob = &mob_index[rnum];
   
   if(reload) {
      while(curmob->mobprogs2) {
	 TheFuncList=curmob->mobprogs2->next;
	 delete curmob->mobprogs2;
	 curmob->mobprogs2 = TheFuncList;
      }
      while(curmob->global_vars) {
	 TheVarList=curmob->global_vars->next;
	 delete curmob->global_vars;
	 curmob->global_vars = TheVarList;
      }
   }
   
   TheFuncList = curmob->mobprogs2;
   TheVarList  = curmob->global_vars;

   GetMPFunctions(BigBuf,2,vnum,curmob,depth,logit);

   curmob->mobprogs2 = TheFuncList;
   curmob->global_vars = TheVarList;
   
   TheFuncList = NULL;
   TheVarList  = NULL;

   if(BigBuf)
     delete BigBuf;
}

int boot_oprog2(char reload) {
   FILE *fl;
   char buf[512], *ptr;
   long num;
   char namebuf[512];

   log_msg("Starting objprog read");

   if(!(fl=fopen("../lib/progs/objprog.ini","r"))) {
      log_msg("Couldn't find objprog ini file!");
      return 0;
   }

   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;

      if(sscanf(buf, "%ld %s", &num, namebuf) != 2)
	continue;

      ReadSingleObjprog(namebuf, num, reload, 0, reload);
   }

   fclose(fl);

   return 1;
}

void ReadSingleObjprog(char *filename, long vnum, int reload, int depth, char logit) {
   FILE *fl;
   char buf[512], *ptr;
   char *BigBuf=NULL;
   index_data *curobj;
   int rnum;

   sprintf(buf, "../lib/progs/objprogs/%s.prg", filename);
   if(!(fl=fopen(buf, "r"))) {
      mlog(NULL, "Couldn't read objprog2 file '%s'", buf);
      return;
   }
   
   if(!depth && logit) {
      mlog(NULL, "Reading objprog2 file '%s'", filename);
   }
   
   fseek(fl,0,SEEK_END);
   BigBuf = new char[ftell(fl)+1];
   fseek(fl,0,SEEK_SET);
   strcpy(BigBuf, "");
   
   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;
	 
      strcat(BigBuf, buf);
   }
   
   fclose(fl);

   //find the obj
   rnum=real_object(vnum);
   if(rnum < 0) {
      mlog(NULL, "Invalid vnum supplied in objprog2.ini: %i", vnum);
      return;
   }

   curobj = &obj_index[rnum];

   if(reload) {
      while(curobj->objprogs2) {
	 TheFuncList=curobj->objprogs2->next;
	 delete curobj->objprogs2;
	 curobj->objprogs2 = TheFuncList;
      }
      while(curobj->global_vars) {
	 TheVarList=curobj->global_vars->next;
	 delete curobj->global_vars;
	 curobj->global_vars = TheVarList;
      }
   }

   TheFuncList = curobj->objprogs2;
   TheVarList  = curobj->global_vars;
   
   GetMPFunctions(BigBuf,3,vnum,curobj,depth,logit);

   curobj->objprogs2 = TheFuncList;
   curobj->global_vars = TheVarList;

   TheFuncList = NULL;
   TheVarList  = NULL;

   if(BigBuf)
     delete BigBuf;
}

int boot_rprog2(char reload) {
   FILE *fl;
   char buf[512], *ptr;
   long num;
   char namebuf[512];

   log_msg("Starting roomprog read");

   if(!(fl=fopen("../lib/progs/roomprog.ini","r"))) {
      log_msg("Couldn't find roomprog ini file!");
      return 0;
   }

   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;

      if(sscanf(buf, "%ld %s", &num, namebuf) != 2)
	continue;

      ReadSingleRoomprog(namebuf, num, reload, 0, reload);
   }

   fclose(fl);

   return 1;
}

void ReadSingleRoomprog(char *filename, long vnum, int reload, int depth, char logit) {
   FILE *fl;
   char buf[512], *ptr;
   char *BigBuf=NULL;
   room_data *curroom;

   sprintf(buf, "../lib/progs/roomprogs/%s.prg", filename);
   if(!(fl=fopen(buf, "r"))) {
      mlog(NULL, "Couldn't read roomprog2 file '%s'", buf);
      return;
   }

   if(!depth && logit) {
      mlog(NULL, "Reading roomprog2 file '%s'", filename);
   }

   fseek(fl,0,SEEK_END);
   BigBuf = new char[ftell(fl)+1];
   fseek(fl,0,SEEK_SET);
   strcpy(BigBuf, "");

   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;

      strcat(BigBuf, buf);
   }

   fclose(fl);

   //find the char
   curroom=real_roomp(vnum);
   if(!curroom) {
      mlog(NULL, "Invalid vnum supplied in roomprog2.ini: %i", vnum);
      return;
   }

   if(reload) {
      while(curroom->roomprogs2) {
	 TheFuncList=curroom->roomprogs2->next;
	 delete curroom->roomprogs2;
	 curroom->roomprogs2 = TheFuncList;
      }
      while(curroom->global_vars) {
	 TheVarList=curroom->global_vars->next;
	 delete curroom->global_vars;
	 curroom->global_vars = TheVarList;
      }
   }

   TheFuncList = curroom->roomprogs2;
   TheVarList  = curroom->global_vars;

   GetMPFunctions(BigBuf,4,vnum,curroom,depth,logit);

   curroom->roomprogs2 = TheFuncList;
   curroom->global_vars = TheVarList;

   TheFuncList = NULL;
   TheVarList  = NULL;

   if(BigBuf)
     delete BigBuf;
}

int boot_zprog2(char reload) {
   FILE *fl;
   char buf[512], *ptr;
   long num;
   char namebuf[512];
   
   log_msg("Starting zoneprog read");
   
   if(!(fl=fopen("../lib/progs/zoneprog.ini","r"))) {
      log_msg("Couldn't find zoneprog ini file!");
      return 0;
   }
   
   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;
      
      if(sscanf(buf, "%ld %s", &num, namebuf) != 2)
	continue;
      
      ReadSingleZoneprog(namebuf, num, reload, 0, reload);
   }
   
   fclose(fl);
   
   return 1;
}

void ReadSingleZoneprog(char *filename, long vnum, int reload, int depth, char logit) {
   FILE *fl;
   char buf[512], *ptr;
   char *BigBuf=NULL;
   zone_data *curzone;
   
   sprintf(buf, "../lib/progs/zoneprogs/%s.prg", filename);
   if(!(fl=fopen(buf,"r"))) {
      mlog(NULL, "Couldn't read zoneprog2 file '%s'", buf);
      return;
   }
   
   if(!depth && logit) {
      mlog(NULL, "Reading zoneprog2 file '%s'", filename);
   }
   
   fseek(fl,0,SEEK_END);
   BigBuf = new char[ftell(fl)+1];
   fseek(fl,0,SEEK_SET);
   strcpy(BigBuf, "");
   
   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;
      
      strcat(BigBuf, buf);
   }

   fclose(fl);
   
   //find the actual zone
   curzone=NULL;
   for(int i=0;i<=top_of_zone_table;i++) {
      if(zone_table[i].top > vnum) {
	 curzone = &zone_table[i];
	 break;
      }
   }
   
   if(!curzone) {
      mlog(NULL, "Couldn't find a zone w/ room #%ld", vnum);
      return;
   } else if(logit) {
      mlog(NULL, "  ->Assigning to zone '%s'", curzone->name);
   }
   
   if(reload) {
      while(curzone->zoneprogs2) {
	 TheFuncList=curzone->zoneprogs2->next;
	 delete curzone->zoneprogs2;
	 curzone->zoneprogs2 = TheFuncList;
      }
      while(curzone->global_vars) {
	 TheVarList=curzone->global_vars->next;
	 delete curzone->global_vars;
	 curzone->global_vars = TheVarList;
      }
   }
   
   TheFuncList = curzone->zoneprogs2;
   TheVarList  = curzone->global_vars;
   
   GetMPFunctions(BigBuf,5,vnum,curzone,depth,logit);
   
   curzone->zoneprogs2 = TheFuncList;
   curzone->global_vars = TheVarList;
   
   TheFuncList = NULL;
   TheVarList  = NULL;
   
   if(BigBuf)
     delete BigBuf;
}

char *HandleCompStatement(char *buf, int Type, long VNum,
			  void *targ, int depth, char logit) {
   
   char line[MAX_STRING_LENGTH];
   char command[MAX_STRING_LENGTH];
   char filen[MAX_STRING_LENGTH];
   char depthbuf[MAX_STRING_LENGTH];
   char *ptr, *ptr2, *pend;
   Variable Temp, *NewVar=NULL;
   Function F;
   
   ptr=pend=buf;
   CLEARCHAR2(pend, '\n', '\r');
   
   while((*pend == ' ') || (*pend == 10) || (*pend == 13)) pend--;
   if(*pend) pend++;
   
   strncpy(line,buf,pend-buf);
   line[pend-buf] = 0;
   
   buf=ptr=line;
   CLEARNWS(ptr);
   strncpy(command,buf,ptr-buf);
   command[ptr-buf]=0;
   
   if(!strcasecmp(command, "include")) {
      CLEARWS(ptr);
      ptr2=ptr;
      
      CLEARCHAR(ptr2,'\n');
      strncpy(filen,ptr,ptr2-ptr);
      filen[ptr2-ptr] = 0;
      
      strcpy(depthbuf, "");
      for(int i=0;i<=depth;i++) strcat(depthbuf, " ");
      if(logit) {
        mlog(NULL, "%s-->Including file '%s.prg'", depthbuf, filen);
      }
      
      index_data *ind = (index_data *) targ;
      room_data  *rm  = (room_data *) targ;
      zone_data  *zn  = (zone_data *) targ;
      switch(Type) {
       case 2:
	 ind->mobprogs2 = TheFuncList;
	 ind->global_vars = TheVarList;
	 ReadSingleMobprog(filen,VNum,0,depth+1,logit);
	 TheFuncList = ind->mobprogs2;
	 TheVarList  = ind->global_vars;
	 break;
       case 3:
	 ind->objprogs2 = TheFuncList;
	 ind->global_vars = TheVarList;
	 ReadSingleObjprog(filen,VNum,0,depth+1,logit);
	 TheFuncList = ind->objprogs2;
	 TheVarList  = ind->global_vars;
	 break;
       case 4:
	 rm->roomprogs2 = TheFuncList;
	 rm->global_vars = TheVarList;
	 ReadSingleRoomprog(filen,VNum,0,depth+1,logit);
	 TheFuncList = rm->roomprogs2;
	 TheVarList  = rm->global_vars;
	 break;
       case 5:
	 zn->zoneprogs2 = TheFuncList;
	 zn->global_vars = TheVarList;
	 ReadSingleZoneprog(filen,VNum,0,depth+1,logit);
	 TheFuncList = zn->zoneprogs2;
	 TheVarList  = zn->global_vars;
	 break;
      }
   } else if(!strcasecmp(command, "define")) {
      CLEARWS(ptr);
      ptr2=ptr;
      
      CLEARNWS(ptr2);
      strncpy(filen,ptr,ptr2-ptr);
      filen[ptr2-ptr] = 0;
      
      CLEARWS(ptr2);
      
      index_data *ind = (index_data *) targ;
      room_data  *rm  = (room_data  *) targ;
      zone_data  *zn  = (zone_data  *) targ;
      
      if(EvalExpression(TheFuncList, ptr2, &Temp)) {
	 if((NewVar = GetVarList(filen))) {
	    NewVar->CopyData(&Temp);
	 } else {
  	    NewVar = new Variable;
	    NewVar->SetName(filen);
	    NewVar->CopyData(&Temp);
	    NewVar->next = TheVarList;
	    TheVarList = NewVar;
	 }
      }
   } else {
      mlog(NULL, "Illegal command in # statement: %s", command);
   }
   
   if(*pend) {
      return pend;
   }
   
   return ++pend;
}

void GetMPFunctions(char *BigMobBuf, int Type, long VNum,
		    void *targ, int depth, char logit) {
   
   char *BigP, *ptr, *ptr2, *ptr3;
   char namebuf[255], argbuf[255];
   NumFuncs=0;
   Function *TheFunc;

   BigP=BigMobBuf;

   CLEARCHAR2(BigP, '>', '#');

   if(!*BigP) {
      mlog(NULL, "No functions found in file!");
      return;
   }
   
   BigP=BigMobBuf;
   
   while(BigP && *BigP) {
      CLEARCHAR2(BigP, '>', '#');

      if(!*BigP)
       	break;
      
      if(*BigP == '#') {
	 BigP = HandleCompStatement(BigP+1,Type,VNum,targ,depth,logit);
	 continue;
      }

      ptr=BigP+1;

      CLEARWS(ptr);
      ptr2=ptr;
      CLEARNWS(ptr);

      if(!*ptr) {
       	 mlog(NULL, "No function name found!");
       	 break;
      }

      strncpy(namebuf, ptr2, ptr-ptr2);
      namebuf[ptr-ptr2]=0;

      CLEARWS(ptr);
      if(*ptr != '(') {
       	 mlog(NULL, "No function args found! (%s)", namebuf);
       	 break;
      }
      ptr2=ptr;
      ptr=GetBtwChars(ptr,'(',')');

      if(!ptr) {
	 mlog(NULL, "%s <-- has an error", namebuf);
       	 break;
      }

      strncpy(argbuf, ptr2+1, ptr-ptr2-2);
      argbuf[ptr-ptr2-2]=0;

      CLEARWS(ptr);
      ptr2=ptr;
      if(*ptr != '{') {
       	 mlog(NULL, "No function code found! (%s)", namebuf);
       	 break;
      }
      ptr=GetBtwChars(ptr,'{','}');

      if(!ptr) {
	 mlog(NULL, "%s <-- has an error", namebuf);
      	 break;
      }

      if(!Restricted_FuncName(namebuf) &&
      	 GetFunc(namebuf)) {
	 mlog(NULL, "Duplicate function name! (%s)", namebuf);
       	 break;
      }

      TheFunc = NewFunc();

      if(!TheFunc)			//oh shit
        break;

      TheFunc->SetName(namebuf);
      strcpy(TheFunc->argbuf,argbuf);
      TheFunc->SetCode(ptr2+1,ptr-ptr2-2);

      ptr2=argbuf;
      while(*ptr2) {
         char buf[255], *ptr4;
	 int i;

       	 CLEARWS(ptr2);

       	 if(!*ptr2)
	   break;

       	 ptr3=ptr2;
	 CLEARCHAR(ptr3, ',');

       	 i=0;
	 for(ptr4=ptr2;ptr4<ptr3;ptr4++) {
	    if(*ptr4 == '=') {
       	       TheFunc->InitArg[TheFunc->NumArgs] = FALSE;
	       strncpy(buf, ptr2, ptr4-ptr2);
	       buf[ptr4-ptr2]=0;
	       TheFunc->Arg[TheFunc->NumArgs].SetName(buf);

       	       strncpy(buf, ptr4+1, ptr3-ptr4-1);
	       buf[ptr3-ptr4-1]=0;
	       TheFunc->Arg[TheFunc->NumArgs].SetData(buf);
	       TheFunc->NumArgs++;

	       i=1;
	       break;
	    }
       	 }

       	 if(!i) {
	    if(Restricted_FuncName(TheFunc->name)) {
	       long tmp;

       	       strncpy(buf, ptr2, ptr3-ptr2);
       	       buf[ptr3-ptr2]=0;

       	       TheFunc->InitArg[TheFunc->NumArgs] = TRUE;
	       if(sscanf(buf, "%ld", &tmp)) {
	       	  TheFunc->Arg[TheFunc->NumArgs].SetData(tmp);
	       	  TheFunc->NumArgs++;
	       } else {
	       	  TheFunc->Arg[TheFunc->NumArgs].SetData(buf);
	       	  TheFunc->NumArgs++;
	       }
	    } else {
       	       strncpy(buf, ptr2, ptr3-ptr2);
	       buf[ptr3-ptr2]=0;

	       TheFunc->InitArg[TheFunc->NumArgs] = FALSE;
	       TheFunc->Arg[TheFunc->NumArgs].SetName(buf);
	       TheFunc->Arg[TheFunc->NumArgs].SetDataNull();
	       TheFunc->NumArgs++;
	    }
       	 }

       	 ptr2=ptr3;
	 if(*ptr2) ptr2++;
      }

      NumFuncs++;

      BigP = ptr;
   }
}

char *GetBtwChars(char *ptr, char b, char e, Function *F) {
   int is_q=FALSE, numit=0;

   if(*ptr != b) {
      if(F) {
       	 mlog(F, "'%c' expected in GetBtwChars (function %s), but instead got '%c'", b, F->name, *ptr);
      } else {
       	 mlog(F, "'%c' expected in GetBtwChars, but instead got '%c'", b, *ptr);
      }
      return NULL;
   }
   while(*ptr) {
      if(*ptr == b) {
       	 if((b == e) && (numit==1)) {
	    return ptr+1;
	 } else if(!is_q) {
       	    numit++;
       	 }
      } else if(*ptr == e) {
       	 if(!is_q) {
       	    numit--;
       	    if(!numit) {
	       return ptr+1;
       	    }
       	 }
      } else if(*ptr == '"') {
	 is_q = !is_q;
      } else if(*ptr == '/') {
       	 if(*(ptr+1) == '/')
       	   CLEARCHAR2(ptr,'\n','\r');	//comment
      }
      ptr++;
   }

   if(F) {
      mlog(F, "Mis-matched %c%c's - function %s", b, e, F->name);
   } else {
      mlog(F, "Mis-matched %c%c's", b, e);
   }
   return NULL;
}

void EvalFunc(Function *F, int argn, Variable **Argv, Variable *Passed) {
   int a, i;
   Function Func;
   char buf[MAX_STRING_LENGTH];
   Variable Temp;

   Func.CopyFunc(F);
   F = &Func;

   //evaluate all args
   for(i=0;i<Func.NumArgs;i++) {
      for(a=0;a<argn;a++) {
	 if((F->Arg[i].type == 1) && *Argv[a]->name &&
	    !strcmp(Argv[a]->name, F->Arg[i].CValue()) &&
	    !F->InitArg[i]) {

	    strcpy(buf, F->Arg[i].name);
	    F->Arg[i].CopyVar(Argv[a]);
	    F->Arg[i].SetName(buf);
	    F->InitArg[i] = TRUE;
	    break;
	 }
      }
   }

   for(i=0;i<Func.NumArgs;i++) {
      if(!F->InitArg[i]) {
	 if(i<argn) {
	    strcpy(buf, F->Arg[i].name);
	    F->Arg[i].CopyVar(Argv[i]);
	    F->Arg[i].SetName(buf);
	    F->InitArg[i] = TRUE;
	 } else {
	    mlog(F, "Function %s expected %i args, and got only %i", F->name, F->NumArgs, argn);
	    return;
	 }
      }
   }

   EvalMultipleLines(F, F->code, &Temp);
   if(Passed) {
      Passed->CopyVar(&Temp);
   }
}

int EvalLineType(Function *F, char *ptr) {
   char *ptr2;
   char wordbuf[MAX_STRING_LENGTH];

   switch(*ptr) {
    case '[':					//actions
      return 0;
    case '%':					//evaluation
      return 1;
    case '/':
      if(*(ptr+1) == '/')		//comment code
	return 2;
      break;
    case 0:						//end of buffer, ignore
      return 2;
   }

   if(!isalpha(*ptr))
     return 3;				//error

   ptr2=ptr;
   CLEARNWS(ptr2);
   strncpy(wordbuf, ptr, ptr2-ptr);
   wordbuf[ptr2-ptr]=0;

   if(Restricted_FuncName(wordbuf))
     return 3;				//restricted function!

   if(!strcmp(wordbuf,"if"))
     return 4;				//if statement

   if(GetFunc(wordbuf))
     return 5;				//it's a local function name

   if(!strcmp(wordbuf,"return"))
     return 6;				// if it's a return function */

   if(!strcmp(wordbuf,"break"))
     return 7;				//break function/if

   if(Embedded_FuncName(wordbuf))
     return 8;				//embedded function!

   if(Bracketed_FuncName(wordbuf))
     return 9;				//bracketed function!

   if(!strcmp(wordbuf, "continue"))
     return 10;			        //continue loop

   return 3;					//error
}

int EvalMultipleLines(Function *F, char *ptr, Variable *Passed) {
   char *ptr3, *ptr2, buf[MAX_STRING_LENGTH];
   Variable Temp;
   int TestRet;

   while(*ptr) {
      CLEARWS(ptr);
      ptr2=ptr;
      CLEARCHAR2(ptr2,'\n','\r');

      ptr3=ptr;							//safe for "if" call
      strncpy(buf, ptr, ptr2-ptr);
      buf[ptr2-ptr]=0;
      ptr=ptr2;

      //if ptr2 is not changed, then it just goes to next line
      switch(EvalLineType(F, buf)) {
       case 0:			//actions for mob (FINAL)
	 ptr2=GetBtwChars(ptr-1,'[',']',F);

	 if(!ptr2)
	   return 0;

	 strncpy(buf,ptr,ptr2-ptr-1);
	 buf[ptr2-ptr-1] = '\0';

	 HandleLiteralCall(F, buf);

	 break;
       case 1:			//assign variable (FINAL)
	 ptr=buf+1;
	 HandleAssignCall(F, ptr);
	 break;
       case 2:			//comment (FINAL)
	 break;
       case 3:			//error or illegal syntax (FINAL)
	 mlog(F, "Illegal syntax: %s (in %s[%s])", buf, F->name, F->argbuf);
	 break;
       case 4:			//if statement (FINAL)
	 ptr2=HandleIfCall(F, ptr3, Passed, &TestRet);

	 if(!ptr2)
	   return 0;

	 if(TestRet)
	   return TestRet;

	 break;
       case 5:			//user defined function (FINAL)
	 EvalExpression(F, buf, &Temp);
	 break;
       case 6:			//return keyword (FINAL)
	 HandleReturnCall(F, buf, Passed);
	 return _RETURN;       //returns 1, because that's what we check in the
	 		       //HandleIfCheck

	 break;		// */
       case 7:			//break keyword (FINAL)
         return _BREAK;
       case 8:			//embedded function (FINAL)
       	 HandleEmbeddedCall(F, buf, Passed);
       	 break;
       case 9:			//bracketed function (FINAL)
       	 ptr2=HandleBracketedCall(F, ptr3, Passed, &TestRet);
       	 if(!ptr2) return 0;
       	 if(TestRet) return TestRet;
       	 break;
       case 10:                 //continue
         return _CONTINUE;
       }

       ptr=ptr2;
   }

   return 0;
}

int EvalExpression(Function *F, char *ptr, Variable *VTo) {
   Variable lhs, rhs;
   char buf[MAX_STRING_LENGTH];
   char *ptr2, *ptr3, opr[MAX_STRING_LENGTH];
   long tmp;
   Variable *VTemp;

   if(!*ptr) return 0;

   CLEARWS(ptr);
   ptr2=ptr;
   CLEARNWS(ptr2);

   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   if(GetFunc(buf)) {
      ptr2 = HandleFuncCall(F, ptr, &lhs);
   } else if(Embedded_FuncName(buf)) {
      ptr2 = HandleEmbeddedCall(F, ptr, &lhs);
   } else if((VTemp = GetVar(F, buf))) {
      lhs.CopyVar(VTemp);
   } else if((VTemp = GetVarList(buf))) {
      lhs.CopyVar(VTemp);
   } else if(sscanf(buf, "%ld", &tmp) == 1) {
      lhs.SetData(tmp);
   } else {
      switch(*ptr) {
      case '"':
       	 ptr2=GetBtwChars(ptr,'"','"',F);

       	 if(!ptr2)
	    return 0;

       	 strncpy(buf, ptr+1, ptr2-ptr-2);
       	 buf[ptr2-ptr-2]=0;

	 lhs.SetData(buf);
	 break;
      case '(':
	 ptr2=GetBtwChars(ptr, '(',')',F);

	 if(!ptr2)
	   return 0;

	 strncpy(buf, ptr+1, ptr2-ptr-2);
	 buf[ptr2-ptr-2]=0;

	 EvalExpression(F, buf, &lhs);
	 break;
      case '-':
	 EvalExpression(F, ptr+1, &lhs);
	 
	 if(lhs.type != 0) {
	    mlog(F, "Wrong type in negation operator: %d (%s)", lhs.type, F->name);
	    return 0;
	 }
	 
	 VTo->SetData(-lhs.Value());
	 return 1;
      case '!':
	 ptr2=++ptr;
	 CLEARWS(ptr2);
	 ptr=ptr2;
         while(ptr2 && *ptr2 && !IsOperator(*ptr2)) {
	    if(*ptr2 == '(') {
	       ptr2 = GetBtwChars(ptr2,'(',')',F);
	    } else {
	       ptr2++;
	    }
	 }
	 
	 if(!ptr2) return 0;
	 
	 strncpy(buf,ptr,ptr2-ptr);
	 buf[ptr2-ptr]=0;
	 
	 EvalExpression(F, buf, &lhs);
	 lhs.SetData(!lhs.Value());
         break;
      default:
	 VTo->SetDataNull();
	 mlog(F, "Invalid syntax in evaluate (%s): \"%s (%s)\"", F->name, ptr, F->argbuf);
	 mlog(F, "error caused by \"%s\"", buf);
	 
	 return 0;
      }
   }

   if(!ptr2) return 0;

   CLEARWS(ptr2);
   ptr=ptr2;
   
   CLEAROPR(ptr);
   strncpy(opr,ptr2,ptr-ptr2);
   opr[ptr-ptr2]=0;

   if(!*ptr2) {
      VTo->CopyData(&lhs);
      return 1;
   }
   
   if(lhs.CheckLogical(opr)) {
      VTo->CopyData(&lhs);
      return 1;
   }
   
   if(!*ptr) {
      mlog(F, "Expecting rhs, but got none! (%s)", F->name);
      return 0;
   }

   EvalExpression(F, ptr, &rhs);
   
   return VTo->Math(&lhs, &rhs, opr);
}

int IsOperator(char ltr) {
   switch(ltr) {
    case '+':
    case '*':
    case '-':
    case '>':
    case '<':
    case '&':
    case '|':
    case '=':
    case '!':
    case '~':
      return 1;
   }
   
   return 0;
}

char *EvalLength(Function *F, char *ptr) {
   char *ptr2=NULL;
   while(*ptr) {
      CLEARWS(ptr);
      CLEARNWS(ptr);
      ptr2=ptr;
      CLEARWS(ptr);
      if(*ptr == '(') {
	 ptr2=ptr=GetBtwChars(ptr, '(', ')', F);

	 if(!ptr)
	   return NULL;
      } else if(*ptr == '"') {
	 ptr2=ptr=GetBtwChars(ptr, '"', '"', F);

	 if(!ptr)
	   return NULL;

      } else if(IsOperator(*ptr)) {
	 ptr++;
      } else if(*ptr == '/') {
	 if(*(ptr+1) == '/')
	   return ptr2;
	 ptr++;
      } else if(*ptr == ',') {
	 return ptr2;
      } else if(*ptr) {
	 return NULL;
      }
   }

   return ptr2;
}

char *HandleFuncCall(Function *F, char *ptr, Variable *Passed) {
   char namebuf[MAX_STRING_LENGTH], *ptr2, *ret;
   char argbuf[MAX_STRING_LENGTH];
   Variable *VarPtrs[10];
   Variable VarList[10], VarTemp;
   Function *TheFunc;
   int NumArgs;

   ptr2=ptr;
   CLEARNWS(ptr2);

   if(!*ptr) {
      mlog(F, "No function name found! This is messed! (%s)", F->name);
      return NULL;
   }

   strncpy(namebuf, ptr, ptr2-ptr);
   namebuf[ptr2-ptr]=0;

   TheFunc=GetFunc(namebuf);
   
   if(!TheFunc) {
      mlog(F, "*Big Bug* No function found in %s with name %s", F->name, namebuf);
      return NULL;
   }

   ptr=ptr2;
   CLEARWS(ptr);
   if(*ptr != '(') {
      mlog(F, "No function args found! (%s, %s)", F->name, namebuf);
      return NULL;
   }
   ret=ptr2=GetBtwChars(ptr,'(',')', F);

   if(!ptr2)
     return NULL;

   strncpy(argbuf, ptr+1, ptr2-ptr-2);
   argbuf[ptr2-ptr-2]=0;

   NumArgs=0;
   ptr=argbuf;
   while(*ptr) {
      char buf[255];
      
      CLEARWS(ptr);
      
      if(!*ptr)
	break;

      ptr2=EvalLength(F, ptr);
      
      if(!ptr2)
	return ret;

      strncpy(buf, ptr, ptr2-ptr);
      buf[ptr2-ptr]=0;

      EvalExpression(F, buf, &VarTemp);
      VarList[NumArgs].CopyVar(&VarTemp);

      strcpy(buf,"");
      VarList[NumArgs].SetName(buf);

      VarPtrs[NumArgs] = &VarList[NumArgs];
      
      NumArgs++;

      ptr=ptr2;
      
      if(*ptr) ptr++;
   }

   TheFunc->CopyParents(F);
   EvalFunc(TheFunc, NumArgs, VarPtrs, Passed);
   
   return ret;
}

void HandleReturnCall(Function *F, char *ptr, Variable *Passed) {
   char buf[MAX_STRING_LENGTH], *ptr2;

   CLEARNWS(ptr);	//get rid of "return"
   CLEARWS(ptr);	//clear whitespace

   if(!*ptr)
     return;

   ptr2=EvalLength(F, ptr);

   if(!ptr2)
     return;

   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   if(Passed)
     EvalExpression(F, buf, Passed);
}

void HandleAssignCall(Function *F, char *ptr) {
   char buf[MAX_STRING_LENGTH], *ptr2;
   char namebuf[MAX_STRING_LENGTH];
   char opr[20];
   Variable Temp, *fnd;
   int Suc;

   CLEARWS(ptr);
   ptr2=ptr;
   CLEARNWS(ptr2);

   strncpy(namebuf, ptr, ptr2-ptr);
   namebuf[ptr2-ptr]=0;

   if(!*namebuf) {
      mlog(F, "No variable name was found in assignment (%s)", F->name);
      return;
   }

   if(GetFunc(namebuf) || Restricted_FuncName(namebuf) ||
      Embedded_FuncName(namebuf) ||
      Bracketed_FuncName(namebuf) ||
      GetVarList(namebuf)) {
		
      mlog(F, "Illegal attempt to assign a function (%s) a value (%s)", namebuf, F->name);
      return;
   }

   ptr=ptr2;
   CLEARWS(ptr);
   ptr2=ptr;
   CLEAROPR(ptr2);

   strncpy(opr, ptr, ptr2-ptr);
   opr[ptr2-ptr]=0;
   
   ptr=ptr2;
   
   CLEARWS(ptr);

   ptr2=EvalLength(F, ptr);

   if(!ptr2)
     return;

   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   Suc = EvalExpression(F, buf, &Temp);

   if(!Suc)
     return;

   if((fnd = GetVar(F, namebuf))) {
      fnd->MathAssign(opr,&Temp);
      return;
   }

   fnd=F->NewLoc(&Temp);
   fnd->SetDataNull();
   fnd->SetName(namebuf);
   fnd->MathAssign(opr,&Temp);
}

char *HandleIfCall(Function *F, char *ptr, Variable *Passed, int *Res) {
   char *ptr2;
   char buf[MAX_STRING_LENGTH], expbuf[MAX_STRING_LENGTH], codebuf[MAX_STRING_LENGTH];
   char elsebuf[MAX_STRING_LENGTH];
   int R, Suc;
   Variable Temp;

   ptr2=ptr;
   CLEARNWS(ptr2);

   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   if(strcmp(buf, "if")) {
      mlog(F, "*Big Bug* HandleIfCall called and first word wasn't 'if'");
      return NULL;
   }

   CLEARWS(ptr2);

   ptr=ptr2;

   if(*ptr != '(') {
      mlog(F, "No ()'s found for if statement found! (%s)", F->name);
      return NULL;
   }
   ptr2=GetBtwChars(ptr,'(',')', F);

   if(!ptr2)
     return NULL;

   strncpy(expbuf, ptr+1, ptr2-ptr-2);
   expbuf[ptr2-ptr-2]=0;

   CLEARWS(ptr2);
   ptr=ptr2;

   if(*ptr != '{') {
      mlog(F, "No if code body found! (%s)", F->name);
      return NULL;
   }
   ptr2=GetBtwChars(ptr,'{','}', F);

   if(!ptr2)
     return NULL;

   strncpy(codebuf, ptr+1, ptr2-ptr-2);
   codebuf[ptr2-ptr-2]=0;

   CLEARWS(ptr2);
   ptr=ptr2;
   CLEARNWS(ptr2);
   
   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   if(!strcmp(buf, "else")) {				//else clause
      CLEARWS(ptr2);
      ptr=ptr2;
      
      if(*ptr != '{') {
	 mlog(F, "No else code body found! (%s)", F->name);
       
	 return NULL;
      }
      ptr2=GetBtwChars(ptr,'{','}',F);

      if(!ptr2)
	return NULL;

       
      strncpy(elsebuf, ptr+1, ptr2-ptr-2);
      elsebuf[ptr2-ptr-2]=0;

      ptr=ptr2;
   } else {
      strcpy(elsebuf, "");
   }

   Suc = EvalExpression(F, expbuf, &Temp);

   if(Suc) {
      if(Temp.Value()) {
	 R = EvalMultipleLines(F, codebuf, Passed);
	 if(Res) *Res = R;
      } else {
	 R = EvalMultipleLines(F, elsebuf, Passed);
	 if(Res) *Res = R;
      }
   }

   return ptr;
}

char *HandleBracketedCall(Function *F, char *ptr, Variable *Passed, int *Res) {
   char *ptr2, *retptr;
   char buf[MAX_STRING_LENGTH], expbuf[MAX_STRING_LENGTH], codebuf[MAX_STRING_LENGTH];
   char namebuf[MAX_STRING_LENGTH];
   int R;

   ptr2=ptr;
   CLEARNWS(ptr2);

   strncpy(namebuf, ptr, ptr2-ptr);
   namebuf[ptr2-ptr]=0;

   CLEARWS(ptr2);

   ptr=ptr2;

   if(*ptr != '(') {
      mlog(F, "No ()'s found for bracketed statement! (%s)", F->name);
      return NULL;
   }
   ptr2=GetBtwChars(ptr,'(',')', F);

   if(!ptr2)
     return NULL;

   strncpy(expbuf, ptr+1, ptr2-ptr-2);
   expbuf[ptr2-ptr-2]=0;

   CLEARWS(ptr2);
   ptr=ptr2;

   if(*ptr != '{') {
      mlog(F, "No bracketed code body found in %s! (%s)", namebuf, F->name);
      return NULL;
   }
   ptr2=GetBtwChars(ptr,'{','}', F);

   if(!ptr2)
     return NULL;

   strncpy(codebuf, ptr+1, ptr2-ptr-2);
   codebuf[ptr2-ptr-2]=0;

   CLEARWS(ptr2);
   ptr=ptr2;
   CLEARNWS(ptr2);
   
   strncpy(buf, ptr, ptr2-ptr);
   buf[ptr2-ptr]=0;

   retptr=ptr;

   R = HandleSpecificBrFunc(F, namebuf, expbuf, codebuf, Passed);
   if(Res) *Res = R;

   return retptr;
}

char *HandleEmbeddedCall(Function *F, char *ptr, Variable *Passed) {
   char namebuf[MAX_STRING_LENGTH], *ptr2, *ret;
   char argbuf[MAX_STRING_LENGTH];
   Variable VarList[32], VarTemp;
   int NumArgs;

   ptr2=ptr;
   CLEARNWS(ptr2);

   if(!*ptr) {
      mlog(F, "No function name found! This is messed! (%s)", F->name);
      return NULL;
   }

   strncpy(namebuf, ptr, ptr2-ptr);
   namebuf[ptr2-ptr]=0;
   
   ptr=ptr2;
   CLEARWS(ptr);
   if(*ptr != '(') {
      mlog(F, "No function args found! (%s, %s)", F->name, namebuf);
      return NULL;
   }
   ret=ptr2=GetBtwChars(ptr,'(',')', F);

   if(!ptr2)
     return NULL;

   strncpy(argbuf, ptr+1, ptr2-ptr-2);
   argbuf[ptr2-ptr-2]=0;

   NumArgs=0;
   ptr=argbuf;
   while(*ptr) {
      char buf[255];

      CLEARWS(ptr);

      if(!*ptr)
	break;

      ptr2=EvalLength(F, ptr);

      if(!ptr2)
	return NULL;

      strncpy(buf, ptr, ptr2-ptr);
      buf[ptr2-ptr]=0;

      if(NumArgs < 10) {
	 EvalExpression(F, buf, &VarTemp);
	 VarList[NumArgs].CopyVar(&VarTemp);

	 strcpy(buf,"");
	 VarList[NumArgs].SetName(buf);

	 NumArgs++;
      } else {
	 mlog(F, "Too many arguments for %s (%s)", namebuf, F->name);
      }

      ptr=ptr2;

      if(*ptr) ptr++;
   }

   HandleSpecificFunc(F, namebuf, NumArgs, VarList, Passed);

   return ret;
}

void HandleSpecificFunc(Function *F, char *namebuf, int NumArgs, Variable *VarList, Variable *Passed) {
   if(Passed) {
      Passed->SetDataNull();
   }

   if(!strcasecmp(namebuf, "Dice")) {
      if(NumArgs != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'Dice' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 0) ||
	 (VarList[1].type != 0)) {
	 Passed->SetData(-1);
	 mlog(F, "'Dice' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 0, 0");
	 return;
      }
      
      Passed->SetData(dice(VarList[0].Value(),VarList[1].Value()));
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetCarriedBy")) {
      if(NumArgs != 1) {
	 mlog(F, "'GetCarriedBy' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 3) {
	 mlog(F, "'GetCarriedBy' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }
      
      obj_data *otmp;
      otmp = (obj_data *) VarList[0].Value();
      if(otmp) {
	 Passed->SetData(otmp->carried_by);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetChar")) {
      if(NumArgs != 1) {
	 mlog(F, "'GetChar' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1)) {
	 mlog(F, "'GetChar' supplied w/ wrong types: %d (%s)",
	      VarList[0].type, F->name);
	 mlog(F, "Should be: 1");
	 return;
      }
      
      Passed->SetData(get_char(VarList[0].CValue()));
      
      return;
   }

   if(!strcasecmp(namebuf, "GetCharInRoom")) {
      if(NumArgs != 2) {
	 mlog(F, "'GetCharInRoom' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4))) {
	 mlog(F, "'GetCharInRoom' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4");
	 return;
      }
      
      room_num rnum;
      room_data *rtmp;
      
      if(VarList[1].type == 4) {
	 rtmp=(room_data*)VarList[1].Value();
	 if(rtmp) {
	    rnum = rtmp->number;
	 } else {
	    rnum = -1;
	 }
      } else {
	 rnum = VarList[1].Value();
      }

      Passed->SetData(get_char_room(VarList[0].CValue(), rnum));
      
      return;
   }

   if(!strcasecmp(namebuf, "GetClan")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'GetClan' function uses only 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'GetClan' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *) VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(mtmp->player.guildinfo.inguild());
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetDesc")) {
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
         mlog(F, "'GetDesc' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      switch(VarList[0].type) {
       case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(ss_data(mtmp->player.description));
	 } else {
	    Passed->SetData("");
	 }
	 break;
       default:
	 mlog(F, "'GetDesc' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<Illegal type>");
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetEquippedBy")) {
      if(NumArgs != 1) {
         Passed->SetData(-1);
	 mlog(F, "'GetEquippedBy' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 3) {
	 mlog(F, "'GetEquippedBy' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }
      
      obj_data *otmp;
      otmp = (obj_data *) VarList[0].Value();
      if(otmp) {
	 Passed->SetData(otmp->equipped_by);
      } else {
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetFlags")) {
      if(NumArgs != 2 && NumArgs != 3) {
	 Passed->SetData(-1);
	 mlog(F, "'GetFlags' function uses 2 or 3 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[1].type != 0) &&
	 (VarList[1].type != 1) &&
	 (NumArgs == 2)) {
	 Passed->SetData(-1);
	 mlog(F, "'GetFlags' supplied w/ illegal type: %d %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be 2 or 3 or 4, 0 or 1");
	 return;
      }
      
      if((((VarList[1].type != 0) &&
	   (VarList[1].type != 1)) ||
	  (VarList[2].type != 0)) &&
	 (NumArgs == 3)) {
	 Passed->SetData(-1);
	 mlog(F, "'GetFlags' supplied w/ illegal type: %d %d %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 mlog(F, "Should be 2 or 3 or 4, 0 or 1, 0");
	 return;
      }

      switch(VarList[0].type) {
      case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "NPC")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "PC")) {
		  VarList[1].SetData(2);
	       } else if(!strcasecmp(VarList[1].CValue(), "Aff")) {
		  VarList[1].SetData(3);
	       } else if(!strcasecmp(VarList[1].CValue(), "Aff2")) {
		  VarList[1].SetData(4);
	       } else if(!strcasecmp(VarList[1].CValue(), "Level")) {
		  VarList[1].SetData(5);
	       } else if(!strcasecmp(VarList[1].CValue(), "HP")) {
		  VarList[1].SetData(6);
	       } else if(!strcasecmp(VarList[1].CValue(), "MANA")) {
		  VarList[1].SetData(7);
	       } else if(!strcasecmp(VarList[1].CValue(), "MaxHP")) {
		  VarList[1].SetData(8);
	       } else if(!strcasecmp(VarList[1].CValue(), "MaxMANA")) {
		  VarList[1].SetData(9);
	       } else if(!strcasecmp(VarList[1].CValue(), "Exp")) {
		  VarList[1].SetData(10);
	       } else if(!strcasecmp(VarList[1].CValue(), "Home")) {
		  VarList[1].SetData(11);
	       } else {
		  Passed->SetData(-1);
		  mlog(F, "Illegal value for flag type (mob), '%s'", VarList[1].CValue());
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:    //NPC flags
	       Passed->SetData(mtmp->specials.mob_act);
	       break;
	     case 2:    //PC flags
	       Passed->SetData(mtmp->specials.flags);
	       break;
	     case 3:    //Affected Flags
	       Passed->SetData(AFF_FLAGS(mtmp));
	       break;
	     case 4:    //Affected2 Flags
	       Passed->SetData(AFF2_FLAGS(mtmp));
	       break;
	     case 5:    //Level
	       Passed->SetData(GetMaxLevel(mtmp));
	       break;
	     case 6:    //HP
	       Passed->SetData(GET_HIT(mtmp));
	       break;
	     case 7:    //Mana
	       Passed->SetData(GET_MANA(mtmp));
	       break;
	     case 8:    //Max HP
	       Passed->SetData(GET_MAX_HIT(mtmp));
	       break;
	     case 9:    //Max Mana
	       Passed->SetData(GET_MAX_MANA(mtmp));
	       break;
	     case 10:   //Exp
	       Passed->SetData(GET_EXP(mtmp));
	       break;
	     case 11:   //Home
	       Passed->SetData(GET_HOME(mtmp));
	       break;
	     default:
	       Passed->SetData(-1);
	       mlog(F, "Illegal value for flag type (mob), %ld", VarList[1].Value());
	    }
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      case 3:
	 obj_data *otmp;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "WEAR")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "Extra")) {
		  VarList[1].SetData(2);
	       } else {
		  mlog(F, "Illegal value for flag type (obj), '%s'", VarList[1].CValue());
		  Passed->SetData(-1);
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:    //Wear flags
	       Passed->SetData(otmp->obj_flags.wear_flags);
	       break;
	     case 2:    //Extra flags
	       Passed->SetData(otmp->obj_flags.extra_flags);
	       break;
	     default:
	       mlog(F, "Illegal value for flag type (objects), %ld", VarList[1].Value());
	       Passed->SetData(-1);
	    }
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      case 4:
	 room_data *rtmp;
	 rtmp = (room_data *)VarList[0].Value();
	 if(rtmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "Flags")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "Door")) {
		  VarList[1].SetData(2);
	       } else {
		  Passed->SetData(-1);
		  mlog(F, "Illegal value for flag type (room), '%s'", VarList[1].CValue());
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:
	       Passed->SetData(rtmp->room_flags);
	       break;
	     case 2:
	       if((VarList[2].Value() < 0) ||
		  (VarList[2].Value() >= NUM_OF_DIRS)) {
		  mlog(F, "Illegal value for door dir in 'GetFlags': %d", VarList[2].Value());
		  break;
	       }
	       if(!rtmp->dir_option[VarList[2].Value()]) {
		  Passed->SetData(-1);
	       } else {
	          Passed->SetData(rtmp->dir_option[VarList[2].Value()]->to_room);
	       }
	       break;
	     default:
	       mlog(F, "Illegal value for flag type (rooms), %ld", VarList[1].Value());
	       Passed->SetData(-1);
	    }
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      default:
	 mlog(F, "'GetFlags' supplied w/ wrong types: %d %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetInRoom")) {
      if(NumArgs != 1) {
	 mlog(F, "'GetInRoom' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 mlog(F, "'GetInRoom' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }
      
      switch(VarList[0].type) {
       case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(real_roomp(mtmp->in_room));
	 } else {
	 }
	 break;
      default:
	 mlog(F, "'GetInRoom' supplied w/ wrong type: %d (%s)", VarList[0].type, F->name);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetInsideOf")) {
      if(NumArgs != 1) {
         Passed->SetData(-1);
	 mlog(F, "'GetInsideOf' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 3) {
	 mlog(F, "'GetInsideOf' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }
      
      obj_data *otmp;
      otmp = (obj_data *) VarList[0].Value();
      if(otmp) {
	 Passed->SetData(otmp->in_obj);
      } else {
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetLong")) {
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
         mlog(F, "'GetLong' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      switch(VarList[0].type) {
       case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(GET_TITLE(mtmp));
	 } else {
	    Passed->SetData("");
	 }
	 break;
       case 3:
	 obj_data *otmp;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    Passed->SetData(OBJ_DESC(otmp));
	 } else {
	    Passed->SetData("");
	 }
	 break;
       default:
	 mlog(F, "'GetLong' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<Illegal type>");
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetName")) {
      if(NumArgs != 1) {
        Passed->SetData("<wrong args>");
       	 mlog(F, "'GetName' function uses 1 argument (%s)", F->name);
       	 return;
      }

      switch(VarList[0].type) {
      case 2:						//mob
       	 char_data *mtmp;
       	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(GET_REAL_NAME(mtmp));
	 } else {
            Passed->SetData("");
	 }
       	 break;
      case 3:						//Obj
       	 obj_data *otmp;
       	 otmp = (obj_data *)VarList[0].Value();
         if(otmp) {
       	    Passed->SetData(OBJ_NAME(otmp));
	 } else {
	    Passed->SetData("");
	 }
       	 break;
      case 4:						//Room
       	 room_data *rtmp;
       	 rtmp = (room_data *)VarList[0].Value();
         if(rtmp) {
       	    Passed->SetData(rtmp->name);
	 } else {
	    Passed->SetData("");
	 }
       	 break;
      default:
         mlog(F, "'GetName' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
       	 Passed->SetData("<illegal type>");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetNum")) {
      if(NumArgs != 1) {
         Passed->SetData(-1);
	 mlog(F, "'GetNum' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      switch(VarList[0].type) {
      case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(GET_MOB_VNUM(mtmp));
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      case 3:
	 obj_data *otmp;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    Passed->SetData(GET_OBJ_VNUM(otmp));
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      case 4:
	 room_data *rtmp;
	 rtmp = (room_data *)VarList[0].Value();
	 if(rtmp) {
	    Passed->SetData(rtmp->number);
	 } else {
	    Passed->SetData(-1);
	 }
	 break;
      default:
	 mlog(F, "'GetNum' supplied w/ wrong type: %d (%s)", VarList[0].type, F->name);
	 Passed->SetData(-1);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetObj")) {
      if(NumArgs != 1) {
	 mlog(F, "'GetObj' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1)) {
	 mlog(F, "'GetObj' supplied w/ wrong type: %d (%s)",
	      VarList[0].type, F->name);
	 mlog(F, "Should be: 1");
	 return;
      }
      
      Passed->SetData(get_obj(VarList[0].CValue()));
      
      return;
   }

   if(!strcasecmp(namebuf, "GetObjEquip")) {
      if(NumArgs != 2) {
         mlog(F, "'GetObjEquip' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 (VarList[1].type != 0)) {
         mlog(F, "'GetObjEquip' supplied w/ illegal types: %d %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 0");

	 return;
      }
      
      char_data *mtmp;
      mtmp = (char_data *) VarList[0].Value();
      if(mtmp && (VarList[1].Value() >= WEAR_LIGHT) &&
	 (VarList[1].Value() <= LOADED)) {
	 Passed->SetData(mtmp->equipment[VarList[1].Value()]);
      }

      return;
   }
   
   if(!strcasecmp(namebuf, "GetOnChar")) {
      if(NumArgs != 1) {
	 mlog(F, "'GetOnChar' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 3) {
	 mlog(F, "'GetOnChar' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }
      
      obj_data *otmp;
      otmp = (obj_data *) VarList[0].Value();
      if(otmp) {
	 Passed->SetData(char_holding(otmp));
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "GetShort")) {
      if(NumArgs != 1) {
         Passed->SetData("<wrong args>");
         mlog(F, "'GetShort' function uses 1 argument (%s)", F->name);
         return;
      }

      switch(VarList[0].type) {
      case 2:                                           //mob
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    Passed->SetData(GET_NAME(mtmp));
	 } else {
	    Passed->SetData("");
	 }
	 break;
      case 3:
	 obj_data *otmp;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    Passed->SetData(OBJ_SHORT(otmp));
	 } else {
	    Passed->SetData("");
	 }
	 break;
      default:
	 mlog(F, "'GetShort' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<Illegal type>");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetTime")) {
      if(NumArgs != 1) {
         Passed->SetData(-1);
         mlog(F, "'GetTime' function uses 1 argument (%s)", F->name);
         return;
      }

      if(VarList[0].type != 1) {
	 mlog(F, "'GetTime' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 mlog(F, "Should be 1");
	 Passed->SetData(-1);
	 return;
      }
      
      if(!strcasecmp(VarList[0].CValue(), "Hour")) {
	 Passed->SetData(time_info.hours);
      } else if(!strcasecmp(VarList[0].CValue(), "Day")) {
	 Passed->SetData(time_info.day);
      } else if(!strcasecmp(VarList[0].CValue(), "Month")) {
	 Passed->SetData(time_info.month);
      } else if(!strcasecmp(VarList[0].CValue(), "Year")) {
	 Passed->SetData(time_info.year);
      } else {
	 mlog(F, "Illegal string for 'GetTime'");
	 mlog(F, "Supposed to be Hour, Day, Month, or Year");
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetUName")) {
      char buf[MAX_STRING_LENGTH];
      if(NumArgs != 1) {
         Passed->SetData("<wrong args>");
         mlog(F, "'GetUName' function uses 1 argument (%s)", F->name);
         return;
      }

      switch(VarList[0].type) {
      case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    sprintf(buf, "%d.%s", which_number_mobile(NULL, mtmp), fname(GET_REAL_NAME(mtmp)));
	    Passed->SetData(buf);
	 } else {
	    Passed->SetData("");
	 }
	 break;
      case 3:
	 obj_data *otmp, *obj;
	 int count;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    count=0;
	    sprintf(buf, "1.%s", fname(OBJ_NAME(otmp)));
	    EACH_OBJECT(iter,obj) {
	       if(isname(fname(OBJ_NAME(obj)), OBJ_NAME(otmp))) {
		  count++;
	       }
	       
	       if(obj == otmp) {
		  sprintf(buf, "%d.%s", count, fname(OBJ_NAME(obj)));
		  break;
	       }
	    } END_AITER(iter);
	    Passed->SetData(buf);
	 } else {
	    Passed->SetData("");
	 }
	 break;
      default:
	 mlog(F, "'GetUName' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<Illegal type>");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetVar")) {
      if(NumArgs != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'GetVar' function uses only 2 arguments (%s)", F->name);
	 return;
      }
      

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 3) &&
	  (VarList[0].type != 4) &&
	  (VarList[0].type != 5)) ||
	 (VarList[1].type != 1)) {
	 Passed->SetData(-1);
	 mlog(F, "'GetVar' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 3 or 4 or 5, 1");
	 return;
      }

      char_data *mtmp;
      obj_data  *otmp;
      room_data *rtmp;
      zone_data *ztmp;
      Variable *vtmp;
      mtmp = (char_data *) VarList[0].Value();
      otmp = (obj_data *) mtmp;
      rtmp = (room_data *) mtmp;
      ztmp = (zone_data *) mtmp;
      
      if(VarList[0].Value()) {
	 switch(VarList[0].type) {
	  case 2:
	    for(vtmp = mtmp->player.vars; vtmp; vtmp = vtmp->next) {
	       if(!strcasecmp(vtmp->name, VarList[1].CValue())) {
		  Passed->CopyData(vtmp);
		  break;
	       }
	    }
	    break;
	  case 3:
	    for(vtmp = otmp->vars; vtmp; vtmp = vtmp->next) {
	       if(!strcasecmp(vtmp->name, VarList[1].CValue())) {
		  Passed->CopyData(vtmp);
		  break;
	       }
	    }
	    break;
	  case 4:
	    for(vtmp = rtmp->vars; vtmp; vtmp = vtmp->next) {
	       if(!strcasecmp(vtmp->name, VarList[1].CValue())) {
		  Passed->CopyData(vtmp);
		  break;
	       }
	    }
	    break;
	  case 5:
	    for(vtmp = ztmp->vars; vtmp; vtmp = vtmp->next) {
	       if(!strcasecmp(vtmp->name, VarList[1].CValue())) {
		  Passed->CopyData(vtmp);
		  break;
	       }
	    }
	    break;
	 }
      } else {
	 mlog(F, "No parent found in 'GetVar'");
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "GetZone")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'GetZone' function uses only 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 4) {
	 Passed->SetData(-1);
	 mlog(F, "'GetZone' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }

      room_data *rtmp;
      rtmp = (room_data *) VarList[0].Value();
      if(rtmp) {
	 Passed->SetData(rtmp->zone);
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "HasObj")) {
      if(NumArgs != 2) {
	 mlog(F, "'HasObj' function uses 2 arguments (%s)", F->name);
	 return;
      }

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 4)) ||
	 ((VarList[1].type != 1) &&
	  (VarList[1].type != 0))) {
         mlog(F, "'HasObj' supplied w/ illegal types: %d %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 4, 0 or 1");
         return;
      }
      
      room_data *rtmp;
      char_data *mtmp;
      long i;
      
      if(VarList[0].type == 2) {
         mtmp = (char_data *) VarList[0].Value();

         if(mtmp) {
	    if(VarList[1].type == 0) {
	       long r_num = real_object(VarList[1].Value());
	       
	       for(i=0;i<MAX_WEAR;i++) {
		  if(mtmp->equipment[i] && mtmp->equipment[i]->item_number == r_num) {
		     Passed->SetData(mtmp->equipment[i]);
		     return;
		  }
	       }
	       Passed->SetData(get_obj_in_list_num(r_num, mtmp->carrying));
	    } else {
	       for(i=0;i<MAX_WEAR;i++) {
		  if(mtmp->equipment[i] && isname(VarList[1].CValue(), OBJ_NAME(mtmp->equipment[i]))) {
		     Passed->SetData(mtmp->equipment[i]);
		     return;
		  }
	       }
  	       Passed->SetData(get_obj_in_list(VarList[1].CValue(), mtmp->carrying));
	    }
         }
      } else {
	 rtmp = (room_data *) VarList[0].Value();
	 
	 if(rtmp) {
	    if(VarList[1].type == 0) {
	       Passed->SetData(get_obj_in_list_num(real_object(VarList[1].Value()), rtmp->contents));
	    } else {
	       Passed->SetData(get_obj_in_list(VarList[1].CValue(), rtmp->contents));
	    }
	 }
      }

      return;
   }

   if(!strcasecmp(namebuf, "HmHr")) {
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'HmHr' function uses only 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData("<Illegal argument>");
	 mlog(F, "'HmHr' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *) VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(HMHR(mtmp));
      } else {
	 Passed->SetData("");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "HsHr")) {
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'HsHr' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData("<incorrect args>");
	 mlog(F, "'HsHr' supplied w/ wrong type: %d (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(HSHR(mtmp));
      } else {
	 Passed->SetData("");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "HsSh")) {
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'HsSh' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData("<illegal argument>");
	 mlog(F, "'HsSh' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(HSSH(mtmp));
      } else {
	 Passed->SetData("");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "InStr")) {
      if(NumArgs != 3) {
	 Passed->SetData(-1);
	 mlog(F, "'InStr' function takes 3 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1) ||
	 (VarList[1].type != 1) ||
	 (VarList[2].type != 0)) {
	 Passed->SetData(-1);
	 mlog(F, "'InStr' supplied w/ wrong types: %d, %d, %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 return;
      }

      Passed->SetData(FindSubStr(VarList[0].CValue(), VarList[1].CValue(), VarList[2].Value()));
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsNpc")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsNpc' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'IsNpc' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(IS_NPC(mtmp));
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsFighting")) {
      if(NumArgs != 1) {
	 Passed->SetData((char_data *)NULL);
	 mlog(F, "'IsFighting' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData((char_data *)NULL);
	 mlog(F, "'IsFighting' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(mtmp->specials.fighting);
      } else {
	 Passed->SetData((char_data *)NULL);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsFollowing")) {
      if(NumArgs != 1) {
	 Passed->SetData((char_data *)NULL);
	 mlog(F, "'IsFollowing' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData((char_data *)NULL);
	 mlog(F, "'IsFollowing' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(mtmp->master);
      } else {
	 Passed->SetData((char_data *)NULL);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsImmortal")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsImmortal' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'IsImmortal' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData(IS_IMMORTAL(mtmp));
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsNumber")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsNumber' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsNumber' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      Passed->SetData(is_number(VarList[0].CValue()));

      return;
   }
   
   if(!strcasecmp(namebuf, "IsPeaceful")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsPeaceful' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 0) &&
	 (VarList[0].type != 4)) {
	 Passed->SetData(-1);
	 mlog(F, "'IsPeaceful' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      room_data *rtmp;
      
      if(VarList[0].type == 4) {
	 rtmp = (room_data *) VarList[0].Value();
      } else {
	 rtmp = real_roomp(VarList[0].Value());
      }

      if(rtmp) {
	 Passed->SetData(IS_SET(rtmp->room_flags, PEACEFUL));
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "IsTracking")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'IsTracking' function takes 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 Passed->SetData(-1);
	 mlog(F, "'IsTracking' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(mtmp) {
	 Passed->SetData((mtmp->hunt_info)?1:0);
      } else {
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "Left")) {
      char *ptr, bptr[10000];
      if(NumArgs != 2) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'Left' function uses 2 arguments (%s)", F->name);
	 return;
      }

      if((VarList[0].type != 1) ||
	 (VarList[1].type != 0)) {

	 Passed->SetData("<illegal type>");
	 mlog(F, "'Left' supplied w/ wrong types: %i %i (%s)", VarList[0].type, VarList[1].type, F->name);
	 return;
      }
      
      if(VarList[1].Value() < 0) {
	 Passed->SetData("<illegal arg>");
	 mlog(F, "'Left' length argument was negative! (%s)", F->name);
	 return;
      }

      ptr = VarList[0].CValue();
      strncpy(bptr, ptr, VarList[1].Value());
      bptr[VarList[1].Value()] = 0;
      Passed->SetData(bptr);
      
      return;
   }

   if(!strcasecmp(namebuf, "LoadObjOnChar")) {
      if(NumArgs != 2) {
	 Passed->SetData((obj_data *) NULL);
	 mlog(F, "'LoadObjOnChar' function uses only 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 ((VarList[1].type != 0) &&
	 (VarList[1].type != 1))) {
	 Passed->SetDataNull();
	 mlog(F, "'LoadObjOnChar' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 0 or 1");
	 return;
      }

      char_data *mtmp;
      obj_data *otmp;
      int r_num;
      mtmp = (char_data *) VarList[0].Value();
      
      r_num = -1;
      if(VarList[1].type == 1) {
	 for(r_num = 0; r_num <= top_of_objt; r_num++)
	   if(isname(VarList[1].CValue(), obj_index[r_num].name))
	     break;
	 if(r_num > top_of_objt)
	   r_num = -1;
      } else {
	 r_num = real_object(VarList[1].Value());
      }
      
      if(mtmp && (r_num >= 0)) {
	 if(!(otmp = make_object(r_num, REAL))) {
	    mlog(F, "Internal error in LoadObjInChar");
	 }
	 
	 obj_to_char(otmp, mtmp);
	 
	 Passed->SetData(otmp);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "Log")) {
      if(NumArgs != 1) {
	 mlog(F, "'Log' function uses 1 argument (%s)", F->name);
	 return;
      }

      switch(VarList[0].type) {
       case 1:						//String
	 mlog(F, VarList[0].CValue());
	 break;
       default:
	 mlog(F, "'Log' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "Mid")) {
      char buf[MAX_STRING_LENGTH];
      unsigned int Sz;
      long Pos, Len;
      if(NumArgs != 3) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'Mid' function uses 3 arguments (%s)", F->name);
	 return;
      }

      if((VarList[0].type != 1) ||
	 (VarList[1].type != 0) ||
	 (VarList[2].type != 0)) {

	 mlog(F, "'Mid' supplied w/ wrong type: %i %i %i (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 Passed->SetData("<illegal type>");
	 return;
      }

      Sz = strlen(VarList[0].CValue());
      Pos = VarList[1].Value();
      Len = VarList[2].Value();

      if((Pos < 0) || (Len < 0)) {
	 mlog(F, "'Mid' called w/ incorrect args: %ld, %ld (%s)", Pos, Len, F->name);
	 Passed->SetData("<bad params>");
	 return;
      }

      if((unsigned long)(Pos + Len) > Sz) {
	 mlog(F, "Overflow in 'Mid' function (%s)", F->name);
	 Passed->SetData("<overflow>");
	 return;
      }

      strncpy(buf, VarList[0].CValue()+Pos, Len);
      buf[Len] = 0;

      Passed->SetData(buf);
      
      return;
   }

   if(!strcasecmp(namebuf, "Num")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'Num' function uses 1 argument (%s)", F->name);
	 return;
      }

      if(VarList[0].type != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'Num' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 return;
      }

      Passed->SetData(atol(VarList[0].CValue()));
      
      return;
   }

   if(!strcasecmp(namebuf, "Number")) {
      if(NumArgs != 2) {
	 mlog(F, "'Number' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 0) ||
	 (VarList[1].type != 0)) {
	 mlog(F, "'Number' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 0, 0");
	 return;
      }
      
      long min, max;
      min = VarList[0].Value();
      max = VarList[1].Value();
      Passed->SetData(number(min,max));
      
      return;
   }
   
   if(!strcasecmp(namebuf, "Rand")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'Rand' function uses only 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 0) {
	 Passed->SetData(-1);
	 mlog(F, "'Rand' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return;
      }

      Passed->SetData(number(0,100) <= VarList[0].Value());
      
      return;
   }
   
   if(!strcasecmp(namebuf, "Right")) {
      char *ptr, *bptr;
      if(NumArgs != 2) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'Right' function uses 2 arguments (%s)", F->name);
	 return;
      }

      if((VarList[0].type != 1) ||
	 (VarList[1].type != 0)) {

	 Passed->SetData("<illegal type>");
	 mlog(F, "'Right' supplied w/ wrong types: %i %i (%s)", VarList[0].type, VarList[1].type, F->name);
	 return;
      }
      
      if(VarList[1].Value() < 0) {
	 Passed->SetData("<illegal arg>");
	 mlog(F, "'Right' length argument was negative! (%s)", F->name);
      }

      ptr = VarList[0].CValue();
      bptr = ptr+strlen(ptr)-VarList[1].Value();
      if(bptr < ptr) bptr = ptr;
      Passed->SetData(bptr);
      
      return;
   }

   if(!strcasecmp(namebuf, "SendToChar")) {
      if(NumArgs != 2) {
	 mlog(F, "'SendToChar' function uses 2 arguments (%s)", F->name);
	 return;
      }

      if((VarList[0].type != 1) ||
	 (VarList[1].type != 2)) {
	 
	 mlog(F, "'SendToChar' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 2");
	 return;
      }
      
      char_data *mtmp;
      mtmp=(char_data *)VarList[1].Value();
      if(mtmp) {
	 cprintf(mtmp, VarList[0].CValue());
	 cprintf(mtmp, "\n\r");
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SendCharToRoom")) {
      if(NumArgs != 2) {
	 mlog(F, "'SendCharToRoom' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4))) {
	 
	 mlog(F, "'SendCharToRoom' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 0 or 4");
	 return;
      }
      
      room_num rnum;
      room_data *rtmp;
      
      if(VarList[1].type == 4) {
	 rtmp=(room_data*)VarList[1].Value();
	 if(rtmp) {
	    rnum = rtmp->number;
	 } else {
	    rnum = -1;
	 }
      } else {
	 rnum = VarList[1].Value();
      }
      
      char_data *mtmp;
      mtmp = (char_data *)VarList[0].Value();
      if(rnum >= 0) {
	 char_from_room(mtmp);
	 char_to_room(mtmp, rnum);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SendToRoom")) {
      if(NumArgs != 2) {
	 mlog(F, "'SendToRoom' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4))) {
	 
	 mlog(F, "'SendToRoom' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4");
	 return;
      }
      
      room_num rnum;
      room_data *rtmp;
      
      if(VarList[1].type == 4) {
	 rtmp=(room_data*)VarList[1].Value();
	 if(rtmp) {
	    rnum = rtmp->number;
	 } else {
	    rnum = -1;
	 }
      } else {
	 rnum = VarList[1].Value();
      }
      
      if(rnum >= 0) {
	 send_to_room_formatted(VarList[0].CValue(), rnum);
	 send_to_room_formatted("\n\r",  rnum);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SendToRoomExcept")) {
      if(NumArgs != 3) {
	 mlog(F, "'SendToRoomExcept' function uses 3 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4)) ||
	 (VarList[2].type != 2)) {
	 
	 mlog(F, "'SendToRoomExcept' supplied w/ wrong types: %d, %d, %d (%s)",
	      VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4, 2");
	 return;
      }
      
      room_data *rtmp;
      room_num rnum;
      char_data *mtmp;
      
      if(VarList[1].type == 4) {
	 rtmp = (room_data *) VarList[1].Value();
	 if(rtmp) {
	    rnum = rtmp->number;
	 } else {
	    rnum = -1;
	 }
      } else {
	 rnum = VarList[1].Value();
      }
      
      mtmp=(char_data *) VarList[2].Value();
      if(mtmp) {
	 send_to_room_except_formatted(VarList[0].CValue(), rnum, mtmp);
	 send_to_room_except_formatted("\n\r", rnum, mtmp);
      } else {
	 send_to_room_formatted(VarList[0].CValue(), rnum);
	 send_to_room_formatted("\n\r", rnum);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "SendToRoomExceptTwo")) {
      if(NumArgs != 4) {
	 mlog(F, "'SendToRoomExceptTwo' function uses 3 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4)) ||
	 (VarList[2].type != 2) ||
	 (VarList[3].type != 2)) {
	 
	 mlog(F, "'SendToRoomExceptTwo' supplied w/ wrong types: %d, %d, %d, %d (%s)",
	      VarList[0].type, VarList[1].type, VarList[2].type, VarList[3].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4, 2, 2");
	 return;
      }
      
      room_data *rtmp;
      room_num rnum;
      char_data *mtmp, *mtmp2;
      
      if(VarList[1].type == 4) {
	 rtmp = (room_data *) VarList[1].Value();
	 if(rtmp) {
	    rnum = rtmp->number;
	 } else {
	    rnum = -1;
	 }
      } else {
	 rnum = VarList[1].Value();
      }
      
      mtmp=(char_data *) VarList[2].Value();
      mtmp2=(char_data *) VarList[3].Value();
      if(mtmp && mtmp2) {
	 send_to_room_except_two_formatted(VarList[0].CValue(), rnum, mtmp, mtmp2);
	 send_to_room_except_two_formatted("\n\r", rnum, mtmp, mtmp2);
      } else {
	 send_to_room_formatted(VarList[0].CValue(), rnum);
	 send_to_room_formatted("\n\r", rnum);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "SetDesc")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetDesc' function uses only 2 arguments (%s)", F->name);
	 return;
      }

      if(((VarList[0].type != 2)) ||
	 (VarList[1].type != 1)) {
	 mlog(F, "'SetDesc' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 1");
	 return;
      }

      if(VarList[0].type == 2) {
	 char_data *mtmp;
	 mtmp = (char_data *) VarList[0].Value();
	 if(mtmp) {
	    char buf[MAX_STRING_LENGTH];
	    strcpy(buf, VarList[1].CValue());
	    strcat(buf, "\n\r");
	    ss_free(mtmp->player.description);
	    mtmp->player.description = ss_make(buf);
	 }
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetFlags")) {
      if((NumArgs != 3) &&
	 (NumArgs != 4)) {
	 mlog(F, "'SetFlags' function uses 3 arguments (%s)", F->name);
	 return;
      }
      
      if((((VarList[1].type != 0) &&
	   (VarList[1].type != 1)) ||
	  (VarList[2].type != 0)) &&
	 (NumArgs == 3)) {
	 mlog(F, "'SetFlags' supplied w/ illegal type: %d %d %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 mlog(F, "Should be 2 or 3 or 4, 0 or 1, 0");
	 return;
      }
      
      if((((VarList[1].type != 0) &&
	   (VarList[1].type != 1)) ||
	  (VarList[2].type != 0)) &&
	 (NumArgs == 4)) {
	 mlog(F, "'SetFlags' supplied w/ illegal type: %d %d %d %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, VarList[3].type, F->name);
	 mlog(F, "Should be 2 or 3 or 4, 0 or 1, 0, 0");
	 return;
      }

      switch(VarList[0].type) {
      case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "NPC")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "PC")) {
		  VarList[1].SetData(2);
	       } else if(!strcasecmp(VarList[1].CValue(), "Aff")) {
		  VarList[1].SetData(3);
	       } else if(!strcasecmp(VarList[1].CValue(), "Aff2")) {
		  VarList[1].SetData(4);
	       } else if(!strcasecmp(VarList[1].CValue(), "Level")) {
		  VarList[1].SetData(5);
	       } else if(!strcasecmp(VarList[1].CValue(), "HP")) {
		  VarList[1].SetData(6);
	       } else if(!strcasecmp(VarList[1].CValue(), "MANA")) {
		  VarList[1].SetData(7);
	       } else if(!strcasecmp(VarList[1].CValue(), "MaxHP")) {
		  VarList[1].SetData(8);
	       } else if(!strcasecmp(VarList[1].CValue(), "MaxMANA")) {
		  VarList[1].SetData(9);
	       } else if(!strcasecmp(VarList[1].CValue(), "Exp")) {
		  VarList[1].SetData(10);
	       } else if(!strcasecmp(VarList[1].CValue(), "Home")) {
		  VarList[1].SetData(11);
	       } else {
		  Passed->SetData(-1);
		  mlog(F, "Illegal value for flag type in 'SetFlags' (mob), '%s'", VarList[1].CValue());
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:    //NPC flags
	       mtmp->specials.mob_act = VarList[2].Value();
	       break;
	     case 2:    //PC flags
	       mtmp->specials.flags = VarList[2].Value();
	       break;
	     case 3:    //Affected Flags
	       AFF_FLAGS(mtmp) = VarList[2].Value();
	       break;
	     case 4:    //Affected2 Flags
	       AFF2_FLAGS(mtmp) = VarList[2].Value();
	       break;
	     case 5:    //Level
	       mlog(F, "Cannot set levels yet!");
	       //GetMaxLevel(mtmp) = VarList[2].Value();
	       break;
	     case 6:    //HP
	       GET_HIT(mtmp) = VarList[2].Value();
	       break;
	     case 7:    //Mana
	       GET_MANA(mtmp) = VarList[2].Value();
	       break;
	     case 8:    //Max HP
	       mlog(F, "Cannot set max HP's yet!");
	       //GET_MAX_HIT(mtmp) = VarList[2].Value();
	       break;
	     case 9:
	       mlog(F, "Cannot set max Mana yet!");
	       //GET_MAX_MANA(mtmp) = VarList[2].Value();
	       break;
	     case 10:
	       if(VarList[2].Value() > GET_EXP(mtmp)) {
		  mlog(F, "Cannot increase experience yet!");
		  break;
	       }
	       GET_EXP(mtmp) = VarList[2].Value();
	       break;
	     case 11:
	       GET_HOME(mtmp) = VarList[2].Value();
	       break;
	     default:
	       mlog(F, "Illegal value for flag type in 'SetFlags' (mob), %ld", VarList[1].Value());
	    }
	 }
	 break;
      case 3:
	 obj_data *otmp;
	 otmp = (obj_data *)VarList[0].Value();
	 if(otmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "Wear")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "Extra")) {
		  VarList[1].SetData(2);
	       } else {
		  mlog(F, "Illegal value for flag type in 'SetFlags' (obj), '%s'", VarList[1].CValue());
		  Passed->SetData(-1);
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:    //Wear flags
	       otmp->obj_flags.wear_flags = VarList[2].Value();
	       break;
	     case 2:    //Extra flags
	       otmp->obj_flags.extra_flags = VarList[2].Value();
	       break;
	     default:
	       mlog(F, "Illegal value for flag type in 'SetFlags' (obj), %ld", VarList[1].Value());
	    }
	 }
	 break;
      case 4:
	 room_data *rtmp;
	 rtmp = (room_data *)VarList[0].Value();
	 if(rtmp) {
	    if(VarList[1].type == 1) {
	       if(!strcasecmp(VarList[1].CValue(), "Flags")) {
		  VarList[1].SetData(1);
	       } else if(!strcasecmp(VarList[1].CValue(), "Door")) {
		  VarList[1].SetData(2);
	       } else {
		  mlog(F, "Illegal value for flag type in 'SetFlags' (room), '%s'", VarList[1].CValue());
		  break;
	       }
	    }
	    switch(VarList[1].Value()) {
	     case 1:
	       rtmp->room_flags = VarList[2].Value();
	       break;
	     case 2:
	       if((VarList[3].Value() < 0) ||
		  (VarList[3].Value() >= NUM_OF_DIRS)) {
		  mlog(F, "Illegal value for door dir in 'SetFlags': %d", VarList[3].Value());
		  break;
	       }
	       rtmp->dir_option[VarList[3].Value()]->to_room = VarList[2].Value();
	       break;
	     default:
	       mlog(F, "Illegal value for flag type in 'SetFlags' (rooms), %ld", VarList[1].Value());
	    }
	 }
	 break;
      default:
	 mlog(F, "'SetFlags' supplied w/ wrong types: %d %d %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 Passed->SetData(-1);
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetHuntChar")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetHuntChar' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 (VarList[1].type != 2)) {
	 mlog(F, "'SetHuntChar' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 2");
	 return;
      }
      
      char_data *mtmp, *mtmp2;
      mtmp=(char_data *) VarList[0].Value();
      mtmp2=(char_data *) VarList[1].Value();
      if(mtmp && mtmp2) {
	 SetHunting(mtmp, mtmp2);
	 mtmp->persist = 250;
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "SetHuntObj")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetHuntObj' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 (VarList[1].type != 3)) {
	 mlog(F, "'SetHuntObj' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 3");
	 return;
      }
      
      char_data *mtmp;
      obj_data *otmp;
      mtmp=(char_data *) VarList[0].Value();
      otmp=(obj_data *) VarList[1].Value();
      if(mtmp && otmp) {
	 if(mtmp->hunt_info) return;
	 mtmp->hunt_info = path_to_obj(mtmp->in_room, otmp, MAX_ROOMS, HUNT_THRU_DOORS | HUNT_GLOBAL);
	 mtmp->persist = 250;
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "SetHuntRoom")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetHuntRoom' function uses 2 arguments (%s)", F->name);
	 return;
      }
      
      if((VarList[0].type != 2) ||
	 ((VarList[1].type != 0) &&
	 (VarList[1].type != 4))) {
	 mlog(F, "'SetHuntRoom' supplied w/ wrong types: %d, %d (%s)",
	      VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 0 or 4");
	 return;
      }
      
      char_data *mtmp;
      room_num r_num;
      mtmp=(char_data *) VarList[0].Value();
      if(VarList[1].type == 0) {
	 r_num=VarList[1].Value();
      } else {
	 room_data *rtmp;
	 rtmp=(room_data *) VarList[1].Value();
	 if(rtmp) {
	    r_num=rtmp->number;
	 } else {
	    r_num=-1;
	 }
      }
      if(mtmp && (r_num >= 0)) {
	 if(mtmp->hunt_info) return;
	 mtmp->hunt_info = path_to_room(mtmp->in_room, r_num, MAX_ROOMS, HUNT_THRU_DOORS | HUNT_GLOBAL);
	 mtmp->persist = 250;
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "SetLong")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetLong' function uses only 2 arguments (%s)", F->name);
	 return;
      }

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 3)) ||
	 (VarList[1].type != 1)) {
	 mlog(F, "'SetLong' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 3, 1");
	 return;
      }

      if(VarList[0].type == 2) {
	 char_data *mtmp;
	 mtmp = (char_data *) VarList[0].Value();
	 if(mtmp) {
	    char buf[MAX_STRING_LENGTH];
	    strcpy(buf, VarList[1].CValue());
	    strcat(buf, "\n\r");
	    ss_free(mtmp->player.long_descr);
	    mtmp->player.long_descr = ss_make(buf);
	 }
      } else {
	 obj_data *otmp;
	 otmp = (obj_data *) VarList[0].Value();
	 if(otmp) {
	    ss_free(otmp->description);
	    otmp->description = ss_make(VarList[1].CValue());
	 }
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetName")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetName' function uses only 2 arguments (%s)", F->name);
	 return;
      }
      

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 3) &&
	  (VarList[0].type != 4)) ||
	 (VarList[1].type != 1)) {
	 mlog(F, "'SetName' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 3 or 4, 1");
	 return;
      }

      if(VarList[0].type == 2) {
	 char_data *mtmp;
	 mtmp = (char_data *) VarList[0].Value();
	 if(mtmp) {
	    if(IS_PC(mtmp)) {
	       mlog(F, "Illegal use of SetName on a player");
	       return;
	    }
	    ss_free(mtmp->player.name);
	    mtmp->player.name = ss_make(VarList[1].CValue());
	 }
      } else if(VarList[0].type == 3) {
	 obj_data *otmp;
	 otmp = (obj_data *) VarList[0].Value();
	 if(otmp) {
	    ss_free(otmp->name);
	    otmp->name = ss_make(VarList[1].CValue());
	 }
      } else if(VarList[0].type == 4) {
	 room_data *rtmp;
	 rtmp = (room_data *) VarList[0].Value();
	 if(rtmp) {
	    strcpy(rtmp->name, VarList[1].CValue());
	 }
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetShort")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetShort' function uses only 2 arguments (%s)", F->name);
	 return;
      }

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 3)) ||
	 (VarList[1].type != 1)) {
	 mlog(F, "'SetShort' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 3, 1");
	 return;
      }

      if(VarList[0].type == 2) {
	 char_data *mtmp;
	 mtmp = (char_data *) VarList[0].Value();
	 if(mtmp) {
	    ss_free(mtmp->player.short_descr);
	    mtmp->player.short_descr = ss_make(VarList[1].CValue());
	 }
      } else {
	 obj_data *otmp;
	 otmp = (obj_data *) VarList[0].Value();
	 if(otmp) {
	    ss_free(otmp->short_description);
	    otmp->short_description = ss_make(VarList[1].CValue());
	 }
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetVar")) {
      if(NumArgs != 3) {
	 mlog(F, "'SetVar' function uses only 3 arguments (%s)", F->name);
	 return;
      }
      

      if(((VarList[0].type != 2) &&
	  (VarList[0].type != 3) &&
	  (VarList[0].type != 4) &&
	  (VarList[0].type != 5)) ||
	 (VarList[1].type != 1)) {
	 mlog(F, "'SetVar' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2 or 3 or 4 or 5, 1");
	 return;
      }

      char_data *mtmp;
      obj_data  *otmp;
      room_data *rtmp;
      zone_data *ztmp;
      Variable *vtmp, *found, *next;
      mtmp = (char_data *) VarList[0].Value();
      if(!*VarList[1].CValue()) {
	 mlog(F, "Must supply a variable name in 'SetVar' (%s)", F->name);
	 return;
      }
      
      mtmp = (char_data *) VarList[0].Value();
      otmp = (obj_data  *) mtmp;
      rtmp = (room_data *) mtmp;
      ztmp = (zone_data *) mtmp;

      if(VarList[0].Value()) {
	 found = next = NULL;
	 switch(VarList[0].type) {
	  case 2:
	    next = mtmp->player.vars;
	    break;
	  case 3:
	    next = otmp->vars;
	    break;
	  case 4:
	    next = rtmp->vars;
	    break;
	  case 5:
	    next = rtmp->vars;
	    break;
	 }
	 for(vtmp = mtmp->player.vars; vtmp; vtmp = vtmp->next) {
	    if(!strcasecmp(vtmp->name, VarList[1].CValue())) {
	       found = vtmp;
	       break;
	    }
         }
      } else {
	 mlog(F, "Parent not found in 'SetVar'");
	 return;
      }

      if(found) {
	 found->CopyData(&VarList[2]);
      } else {
	 found = new Variable;
	 found->SetName(VarList[1].CValue());
	 found->CopyData(&VarList[2]);
	 found->next = next;
	 switch(VarList[0].type) {
	  case 2:
	    mtmp->player.vars = found;
	    break;
	  case 3:
	    otmp->vars = found;
	    break;
	  case 4:
	    rtmp->vars = found;
	    break;
	  case 5:
	    ztmp->vars = found;
	    break;
	  default:
	    mlog(F, "losing memory in setvar w/ type %d", VarList[0].type);
	    break;
	 }
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "SetWaitState")) {
      if(NumArgs != 2) {
	 mlog(F, "'SetWaitState' function uses only 2 arguments (%s)", F->name);
	 return;
      }

      if((VarList[0].type != 2) ||
	 (VarList[1].type != 0)) {
	 mlog(F, "'SetWaitState' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 2, 0");
	 return;
      }

      char_data *mtmp;
      mtmp = (char_data *) VarList[0].Value();
      if(mtmp) {
	 WAIT_STATE(mtmp, VarList[1].Value());
      }
      
      return;
   }
   
   if(!strcasecmp(namebuf, "StopFighting")) {
      if(NumArgs != 1) {
	 mlog(F, "'StopFighting' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 mlog(F, "'StopFighting' supplied w/ wrong type: %d (%s)",
	      VarList[0].type, F->name);
	 return;
      }
      
      char_data *mtmp;
      mtmp=(char_data *) VarList[0].Value();
      if(mtmp) {
	 DeleteHatreds(mtmp);
	 DeleteHatreds(mtmp->specials.fighting);
	 stop_fighting(mtmp);
	 stop_opponents(mtmp, mtmp->in_room);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "StopTracking")) {
      if(NumArgs != 1) {
	 mlog(F, "'StopTracking' function uses 1 argument (%s)", F->name);
	 return;
      }
      
      if(VarList[0].type != 2) {
	 mlog(F, "'StopTracking' supplied w/ wrong type: %d (%s)",
	      VarList[0].type, F->name);
	 return;
      }
      
      char_data *mtmp;
      mtmp=(char_data *) VarList[0].Value();
      if(mtmp) {
	 path_kill(mtmp->hunt_info);
	 mtmp->hunt_info = NULL;
	 mtmp->persist = 0;
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "Str")) {
      char buf[MAX_STRING_LENGTH];
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'Str' function uses 1 argument (%s)", F->name);
	 return;
      }

      switch(VarList[0].type) {
       case 0:						//long
	 sprintf(buf, "%ld", VarList[0].Value());
	 Passed->SetData(buf);
	 break;
       default:
	 mlog(F, "'Val' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<illegal type>");
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "StrLen")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'StrLen' function uses 1 argument (%s)", F->name);
	 return;
      }

      if(VarList[0].type != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'StrLen' supplied w/ wrong type: %d (%s)",VarList[0].type, F->name);
	 return;
      }
      
      Passed->SetData(strlen(VarList[0].CValue()));

      return;
   }

   if(!strcasecmp(namebuf, "StringName")) {
      if(NumArgs != 1) {
	 Passed->SetData("");
	 mlog(F, "'StringName' function uses 1 argument (%s)", F->name);
	 return;
      }

      if(VarList[0].type != 1) {
	 mlog(F, "'StringName' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("");
	 return;
      }

      char *ptr;
      char buf[MAX_STRING_LENGTH];

      strcpy(buf, VarList[0].CValue());
      ptr=buf;
      while(*ptr) {
	 if((*ptr == ' ') || (*ptr == '\t')) *ptr = '-';
	 ptr++;
      }

      Passed->SetData(buf);
      
      return;
   }

   if(!strcasecmp(namebuf, "ToLower")) {
      char buf[MAX_STRING_LENGTH];
      unsigned int i;
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'ToLower' function uses 1 argument (%s)", F->name);
	 return;
      }

      switch(VarList[0].type) {
       case 1:						//String
	 for(i=0;i<strlen(VarList[0].CValue())+1;i++)
	   buf[i] = tolower(*(VarList[0].CValue() + i));
	 Passed->SetData(buf);
	 break;
       default:
	 mlog(F, "'ToLower' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<illegal type>");
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "ToUpper")) {
      char buf[MAX_STRING_LENGTH];
      unsigned int i;
      if(NumArgs != 1) {
	 Passed->SetData("<wrong args>");
	 mlog(F, "'ToUpper' function uses 1 argument (%s)", F->name);
	 return;
      }

      switch(VarList[0].type) {
       case 1:						//String
	 for(i=0;i<strlen(VarList[0].CValue())+1;i++)
	   buf[i] = toupper(*(VarList[0].CValue() + i));
	 Passed->SetData(buf);
	 break;
       default:
	 mlog(F, "'ToUpper' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData("<illegal type>");
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "Val")) {
      long tmp=0;
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'Val' function uses 1 argument (%s)", F->name);
	 return;
      }

      switch(VarList[0].type) {
       case 1:						//string
	 sscanf(VarList[0].CValue(), "%ld", &tmp);
	 Passed->SetData(tmp);
	 break;
       default:
	 mlog(F, "'Val' supplied w/ wrong type: %i (%s)", VarList[0].type, F->name);
	 Passed->SetData(-1);
      }
      
      return;
   }

   if(!strcasecmp(namebuf, "VarType")) {
      if(NumArgs != 1) {
	 Passed->SetData(-1);
	 mlog(F, "'VarType' function uses 1 argument (%s)", F->name);
	 return;
      }

      Passed->SetData(VarList[0].type);
      
      return;
   }
   
   mlog(F, "Function %s doesn't have a handler!", namebuf);
}

int HandleSpecificBrFunc(Function *F, char *namebuf, char *expbuf,
			 char *codebuf, Variable *Passed) {
   int br_ind;
   char *ptr, *ptr2, buf[MAX_STRING_LENGTH];
   Variable VarTemp, VarList[32];
   int NumArgs;
   
   for(br_ind=0;mprog_bracketed_list[br_ind].name;br_ind++) {
      if(!strcasecmp(namebuf, mprog_bracketed_list[br_ind].name))
	break;
   }
   
   NumArgs=0;
   ptr=expbuf;
   while(*ptr && mprog_bracketed_list[br_ind].eval) {
      CLEARWS(ptr);

      if(!*ptr)
	break;

      ptr2=EvalLength(F, ptr);

      if(!ptr2)
	return 0;

      strncpy(buf, ptr, ptr2-ptr);
      buf[ptr2-ptr]=0;

      if(NumArgs < 32) {
	 EvalExpression(F, buf, &VarTemp);
	 VarList[NumArgs].CopyVar(&VarTemp);

	 strcpy(buf,"");
	 VarList[NumArgs].SetName(buf);

	 NumArgs++;
      } else {
	 mlog(F, "Too many arguments for %s (%s)", namebuf, F->name);
	 break;
      }

      ptr=ptr2;

      if(*ptr) ptr++;
   }

   // Start function definitions here:

   if(!strcasecmp(namebuf, "DoAt")) {
      if(NumArgs != 1) {
	 mlog(F, "'DoAt' function uses 1 argument (%s)", F->name);
	 return 0;
      }
      
      if((VarList[0].type != 0) &&
	 (VarList[0].type != 2) &&
	 (VarList[0].type != 4)) {
	 mlog(F, "'DoAt' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return 0;
      }
      
      room_num rtmp=-1;
      switch(VarList[0].type) {
       case 0:
	 rtmp=VarList[0].Value();
	 break;

       case 2:
	 char_data *mtmp;
	 mtmp = (char_data *)VarList[0].Value();
	 if(mtmp) {
	    rtmp = mtmp->in_room;
	 } else {
	    rtmp = -1;
	 }
	 break;

       case 4:
	 if(VarList[0].Value()) {
	    rtmp = ((room_data*)VarList[0].Value())->number;
	 } else {
	    rtmp = -1;
	 }
	 break;
      }

      if(rtmp < 0) {
	 mlog(F, "Couldn't find room for 'DoAt' function.");
	 return 0;
      }
      
      if(!F->mob_parent) {
	 mlog(F, "No mob parent found in 'DoAt' function.");
	 return 0;
      }
      
      int tmp;
      Variable Temp;
      room_num crm=F->mob_parent->in_room;

      F->mob_parent->in_room = rtmp;
      tmp = EvalMultipleLines(F, codebuf, &Temp);
      F->mob_parent->in_room = crm;

      if(tmp == _RETURN)
	return tmp;

      return 0;
   }

   if(!strcasecmp(namebuf, "DoWith")) {
      if(NumArgs != 1) {
	 mlog(F, "'DoWith' function uses 1 argument (%s)", F->name);
	 return 0;
      }
      
      if(VarList[0].type != 2) {
	 mlog(F, "'DoWith' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return 0;
      }
      
      char_data *mtmp;
      mtmp = (char_data*) VarList[0].Value();
      if(!mtmp) {
         mlog(F, "'DoWith' supplied w/ NULL mob (%s)", F->name);
         return 0;
      }

      int tmp;
      Variable Temp;
      char_data *cm=F->mob_parent;

      F->mob_parent = mtmp;
      tmp = EvalMultipleLines(F, codebuf, &Temp);
      F->mob_parent = cm;

      if(tmp == _RETURN)
	return tmp;

      return 0;
   }

   if(!strcasecmp(namebuf, "ForEachMobInDir")) {
      if(NumArgs != 3) {
	 mlog(F, "'ForEachMobInDir' function uses 4 arguments (%s)", F->name);
	 return 0;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4)) ||
	 (VarList[2].type != 0)) {
	 mlog(F, "'ForEachMobInDir' supplied w/ illegal types: %d, %d, %d (%s)", VarList[0].type, VarList[1].type, VarList[2].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4, 0");
	 return 0;
      }
      
      Variable *Var, Temp;
      Var = GetVar(F, VarList[0].CValue());
      if(!Var) {
	 Temp.SetData(-1);
	 Var=F->NewLoc(&Temp);
	 Var->SetName(VarList[0].CValue());
      }
      
      room_data *rtmp;
      if(VarList[1].type == 0) {
	 rtmp=real_roomp(VarList[1].Value());
      } else {
	 rtmp=(room_data *)VarList[1].Value();
      }
      
      if(!rtmp) {
	 mlog(F, "Room passed in 'ForEachMobInDir' is NULL (%s)", F->name);
	 return 0;
      }
      
      int dir=VarList[2].Value();
      
      if((dir < 0) || (dir > DOWN)) {
	 mlog(F, "Dir passed in 'ForEachMobInDir' should be from 0-%d (%s)", DOWN, F->name);
	 return 0;
      }
      
      char_data *mob;
      int count=0, tmp;
      
      Temp.SetData(1);
      while(rtmp && (count < 15)) {
         for(mob = rtmp->people; mob; mob = mob->next_in_room) {
	    Var->SetData(mob);

	    tmp = EvalMultipleLines(F, codebuf, &Temp);

	    if(tmp == _RETURN) return tmp;
	    if(tmp == _BREAK) break;
         }
	 
	 if(rtmp->dir_option[dir])
	   rtmp=real_roomp(rtmp->dir_option[dir]->to_room);
	 else
	   rtmp=NULL;
	 count++;
      }
      
      return 0;
   }
   
   if(!strcasecmp(namebuf, "ForEachMobInGroup")) {
      if(NumArgs != 2) {
	 mlog(F, "'ForEachMobInRoom' function uses 2 arguments (%s)", F->name);
	 return 0;
      }
      
      if((VarList[0].type != 1) ||
	 (VarList[1].type != 2)) {
	 mlog(F, "'ForEachMobInGroup' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 2");
	 return 0;
      }
      
      Variable *Var, Temp;
      Var = GetVar(F, VarList[0].CValue());
      if(!Var) {
	 Temp.SetData(-1);
	 Var=F->NewLoc(&Temp);
         Var->SetName(VarList[0].CValue());
      }
      
      char_data *mtmp;
      mtmp=(char_data *) VarList[1].Value();
      
      if(!mtmp) {
	 mlog(F, "Mob passed in 'ForEachMobInGroup' is NULL (%s)", F->name);
	 return 0;
      }
      
      int tmp;
      char_data *mast;
      follow_type *f, *next;
      if(!(mast=mtmp->master))
	mast=mtmp;
      
      for(f=mast->followers; f; f=next) {
	 next=f->next;
	 Var->SetData(f->follower);
	 
	 tmp = EvalMultipleLines(F, codebuf, &Temp);

	 if(tmp == _RETURN)
	   return tmp;
	 
	 if(tmp == _BREAK)
	   break;
      }
      
      return 0;
   }
   
   if(!strcasecmp(namebuf, "ForEachMobInRoom")) {
      if(NumArgs != 2) {
	 mlog(F, "'ForEachMobInRoom' function uses 2 arguments (%s)", F->name);
	 return 0;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4))) {
	 mlog(F, "'ForEachMobInRoom' supplied w/ illegal types: %d, %d (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4");
	 return 0;
      }
      
      Variable *Var, Temp;
      Var = GetVar(F, VarList[0].CValue());
      if(!Var) {
	 Temp.SetData(-1);
	 Var=F->NewLoc(&Temp);
	 Var->SetName(VarList[0].CValue());
      }
      
      room_data *rtmp;
      if(VarList[1].type == 0) {
	 rtmp=real_roomp(VarList[1].Value());
      } else {
	 rtmp=(room_data *)VarList[1].Value();
      }
      
      if(!rtmp) {
	 mlog(F, "Room passed in 'ForEachMobInRoom' is NULL (%s)", F->name);
	 return 0;
      }
      
      int tmp;
      char_data *mob;
      for(mob = rtmp->people; mob; mob = mob->next_in_room) {
	 Var->SetData(mob);
	 
	 tmp = EvalMultipleLines(F, codebuf, &Temp);

	 if(tmp == _RETURN)
	   return tmp;
	 
	 if(tmp == _BREAK)
	   break;
      }
      
      return 0;
   }
   
   if(!strcasecmp(namebuf, "ForEachMobInZone")) {
      if(NumArgs != 2) {
	 mlog(F, "'ForEachMobInZone' function uses 2 arguments (%s)", F->name);
	 return 0;
      }
      
      if((VarList[0].type != 1) ||
	 ((VarList[1].type != 0) &&
	  (VarList[1].type != 4))) {
	 mlog(F, "'ForEachMobInZone' supplied w/ illegal types: %d, %d, (%s)", VarList[0].type, VarList[1].type, F->name);
	 mlog(F, "Should be: 1, 0 or 4");
	 return 0;
      }
      
      Variable *Var, Temp;
      Var = GetVar(F, VarList[0].CValue());
      if(!Var) {
	 Temp.SetData(-1);
	 Var=F->NewLoc(&Temp);
	 Var->SetName(VarList[0].CValue());
      }
      
      int tmp, zone;
      char_data *mob;
      if(VarList[1].type == 0) {
	 zone = VarList[1].Value();
      } else {
	 room_data *rtmp;
	 rtmp = (room_data *)VarList[1].Value();
	 zone = rtmp->zone;
      }

      EACH_CHARACTER(iter, mob) {
	 if(real_roomp(mob->in_room)->zone != zone)
	   continue;
	 
	 Var->SetData(mob);
	 
	 tmp = EvalMultipleLines(F, codebuf, &Temp);

	 if(tmp == _RETURN)
	   return tmp;
	 
	 if(tmp == _BREAK)
	   break;
      } END_AITER(iter);
      
      return 0;
   }
   
   if(!strcasecmp(namebuf, "MakeString")) {
      char buf[10000], *bptr, *eptr;
      if(NumArgs != 1) {
	 mlog(F, "'MakeString' function uses 1 argument (%s)", F->name);
	 return 0;
      }
      
      if(VarList[0].type != 1) {
	 mlog(F, "'MakeString' supplied w/ illegal type: %d (%s)", VarList[0].type, F->name);
	 return 0;
      }
      
      Variable *Var, Temp;
      Var = GetVar(F, VarList[0].CValue());
      if(!Var) {
	 Temp.SetData(-1);
	 Var=F->NewLoc(&Temp);
	 Var->SetName(VarList[0].CValue());
      }

      bptr = codebuf;
      CLEARCHAR(bptr,'[');
      if(*bptr == '[') {
	 eptr=GetBtwChars(bptr,'[',']');
      }
      
      if((*bptr != '[') || !eptr) {
	 mlog(F, "'MakeString' provided w/ illegal syntax:\n\r"
	         "MakeString(\"varname\") {\n\r"
	         "  [str]\n\r"
	         "} (%s)", F->name);
	 mlog(NULL, "bptr == '%s'", bptr);
	 return 0;
      }
      
      strncpy(buf,bptr+1,eptr-bptr-2);
      buf[eptr-bptr-2]=0;
      
      Var->SetData(buf);

      return 0;
   }

   if(!strcasecmp(namebuf, "While")) {
      Variable Temp;
      int Suc, iter, tmp;
      
      Suc=1;
      iter=0;
      while(Suc) {
	 iter++;
	 if(iter > 10000) {
	    mlog(F, "Over 10000 iterations on a while loop! Check code for logic errors! (%s)", F->name);
	    break;
	 }
	 
	 Suc=EvalExpression(F, expbuf, &Temp);
	 if(!Suc)
	   break;
	 
	 if(!Temp.Value())
	   break;
	 
	 tmp=EvalMultipleLines(F, codebuf, Passed);
	 
	 if(tmp == _RETURN)
	   return tmp;
	 
	 if(tmp == _BREAK)
	   break;
      }
      
      return 0;
   }
   
   mlog(F, "Bracketed Function %s doesn't have a handler!", namebuf);
   return 0;
}

void HandleLiteralCall(Function *F, char *ptr) {
   char exprbuf[MAX_STRING_LENGTH];
   char linebuf[MAX_STRING_LENGTH];
   char varbuf[MAX_STRING_LENGTH];
   char *ptr2, *ptr3, *expptr;
   char buf[MAX_STRING_LENGTH];
   Variable Temp, *TheVar;

   CLEARWS(ptr);
   while(*ptr) {
      CLEARWS(ptr);
      ptr2=ptr;
      CLEARCHAR2(ptr2,'\n','\r');

      strncpy(linebuf, ptr, ptr2-ptr);
      linebuf[ptr2-ptr]=0;
      ptr=ptr2;

      ptr3=linebuf;
      expptr=exprbuf;
      strcpy(exprbuf, "");
      while(*ptr3) {
	 if(*ptr3 != '$') {
	    *expptr = *ptr3;
	    expptr++;
	    ptr3++;
	 } else {
	    if(*(ptr3+1) == '$') {
	       *expptr=*ptr3;
	       expptr++;
	       ptr3 += 2;
	    } else {
	       ptr3++;
	       ptr2=ptr3;
	       CLEARNWS(ptr2);

	       strncpy(varbuf,ptr3,ptr2-ptr3);
	       varbuf[ptr2-ptr3]=0;

	       *expptr=0;
	       if(!(TheVar = GetVar(F, varbuf)) &&
		  !(TheVar = GetVarList(varbuf))) {
		  strcpy(buf, "<var not found>");
		  strcat(exprbuf, buf);
		  expptr += strlen(buf);
		  ptr3 += strlen(varbuf);
	       } else {
		  switch(TheVar->type) {
		  case 0:
		     sprintf(buf, "%ld", TheVar->Value());
		     strcat(exprbuf, buf);
		     expptr += strlen(buf);
		     ptr3 += strlen(varbuf);
		     break;
		  case 1:
		     sprintf(buf, "%s", TheVar->CValue());
		     strcat(exprbuf, buf);
		     expptr += strlen(buf);
		     ptr3 += strlen(varbuf);
		     break;
		  case 2:
		     char_data *mtmp;
		     mtmp = (char_data *)TheVar->Value();
		     if(mtmp) {
			sprintf(buf, "%s", fname(GET_REAL_NAME(mtmp)));
		     } else {
			strcpy(buf, "");
		     }
		     strcat(exprbuf, buf);
		     expptr += strlen(buf);
		     ptr3 += strlen(varbuf);
		     break;
		  case 3:
		     obj_data *otmp;
		     otmp = (obj_data *)TheVar->Value();
		     if(otmp) {
			sprintf(buf, "%s", OBJ_SHORT(otmp));
		     } else {
			strcpy(buf, "");
		     }
		     strcat(exprbuf, buf);
		     expptr += strlen(buf);
		     ptr3 += strlen(varbuf);
		     break;
		  }
	       }
	    }
	 }
	 *expptr=0;
      }

      if(*exprbuf) {
	 if(F->mob_parent)
	   command_interpreter(F->mob_parent, exprbuf, 1);
	 else
	   mlog(F, "No mob_parent found in literal (%s)", F->name);
      }
   }
}







Function *NewFunc() {
   Function *tmp;

   tmp = TheFuncList;
   TheFuncList = new Function;
   TheFuncList->next = tmp;
   if(tmp) tmp->prev = TheFuncList;
   
   return TheFuncList;
}

int Restricted_FuncName(char *buf) {
   for(int i=0;mprog_trigger_list[i];i++)
      if(!strcasecmp(buf, mprog_trigger_list[i]))
       	 return 1;

   return 0;
}

int Embedded_FuncName(char *buf) {
   for(int i=0;mprog_embedded_list[i];i++)
      if(!strcasecmp(buf, mprog_embedded_list[i]))
       	 return 1;

   return 0;
}

int Bracketed_FuncName(char *buf) {
   for(int i=0;mprog_bracketed_list[i].name;i++)
      if(!strcasecmp(buf, mprog_bracketed_list[i].name))
       	 return 1;

   return 0;
}

Variable *GetVar(Function *F, char *buf) {
   int i;
   Variable *iter;

   if(!*buf) return NULL;
   
   if(!F) return NULL;

   for(i=0;i<F->NumArgs;i++) {
      if(!strcasecmp(F->Arg[i].name, buf))
       	 return &F->Arg[i];
   }

   for(iter=F->Loc;iter;iter=iter->next) {
      if(!strcasecmp(iter->name, buf))
       	 return iter;
   }

   return NULL;
}

Variable *GetVarList(char *buf) {
   Variable *iter;
   for(iter=TheVarList;iter;iter=iter->next) {
      if(!strcmp(iter->name, buf))
	return iter;
   }
   
   return NULL;
}

Function *GetFunc(char *buf) {
   Function *iter, *top=TheFuncList;
   if(!*buf) return NULL;

   for(iter=TheFuncList;iter;iter=iter->prev) {
     top=iter;
   }
   
   for(iter=top;iter;iter=iter->next) {
      if(!strcasecmp(iter->name, buf))
       	 return iter;
   }

   return NULL;
}

int FindSubStr(char *src, char *pattern, unsigned int offset) {
   char *ptr;
   
   if(offset > strlen(src))
     return -1;
   
   for(ptr=src+offset; ptr < src+strlen(src)-strlen(pattern)+1; ptr++) {
      if(!strncasecmp(ptr, pattern, strlen(pattern)))
	return ptr-src;
   }
   
   return -1;
}

void ClearParents(Function *F) {
   F->mob_parent = NULL;
   F->obj_parent = NULL;
   F->room_parent = NULL;
   F->zone_parent = NULL;
   TheFuncList = NULL;
   TheVarList  = NULL;
}

void SetParents(Function *F, Variable *V, void *data, int type) {
   switch(type) {
    case 2:
      F->mob_parent = (char_data *) data;
      break;
    case 3:
      F->obj_parent = (obj_data *) data;
      break;
    case 4:
      F->room_parent = (room_data *) data;
      break;
    case 5:
      F->zone_parent = (zone_data *) data;
      break;
   }
   TheFuncList = F;
   TheVarList  = V;
}

void mlog(Function *F, char *format, ...) {
   char debug_buf[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   va_list args;
   va_start(args, format);
   vsprintf (debug_buf, format, args);

   if(F) {
      if(F->mob_parent) {
         sprintf(buf, " [Mob #%d]", GET_MOB_VNUM(F->mob_parent));
         strcat(debug_buf, buf);
      }
      if(F->room_parent) {
	 sprintf(buf, " [Room #%ld]", F->room_parent->number);
	 strcat(debug_buf, buf);
      }
      if(F->obj_parent) {
	 sprintf(buf, " [Obj #%d]", GET_OBJ_VNUM(F->obj_parent));
         strcat(debug_buf, buf);
      }
      if(F->zone_parent) {
	 sprintf(buf, " [Zone %s]", F->zone_parent->name);
	 strcat(debug_buf, buf);
      }
   }

   log_msg(debug_buf);
}

long multmod(long a, long b, long z) {
  long r,x,i;
  i=b; r=0; x=a%z;
  while (i>0) {
    if (i%2) {
      r=(r+x)%z;
    }    
    x=(x+x)%z;
    i=i/2;
  }

  return r;
}

long expomod(long a, long n, long z) {
  long r,x,i;
  i=n; r=1; x=a%z;
  while (i>0) {
    if (i%2) {
      r = multmod(r,x,z);
    }
    x = multmod(x,x,z);
    i=i/2;
  }
  return r;
}

/////////////////////////////////////////
//    Mobile Triggers
/////////////////////////////////////////

void mprog_weather_trigger2( struct char_data *mob, int timeofday) {
   int action=0;
   Function *mprg;
   Variable *VarPtrs[10];
   Variable VarList[10];
   Variable *global;
   
   if(!mob) return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg=mprg->next) {
      if(strcasecmp(mprg->name, "weather_prog"))
	continue;
      
      if(mprg->Arg[0].type > 1) {
	 mlog(NULL, "Mob % - Error in reading weather prog. Wrong argument type: %i", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }
      
      action=0;
      if((mprg->Arg[0].type == 0) && (mprg->Arg[0].Value() == timeofday))
	 action=1;
      if(mprg->Arg[0].type == 1) {
	 if(!strcasecmp(mprg->Arg[0].CValue(), "all"))
	   action=1;
	 if(!strcasecmp(mprg->Arg[0].CValue(), "night") &&
	    (timeofday == 20))
	   action=1;
	 if(!strcasecmp(mprg->Arg[0].CValue(), "midnight") &&
	    (timeofday == 24))
	   action=1;
	 if(!strcasecmp(mprg->Arg[0].CValue(), "morning") &&
	    (timeofday == 8))
	   action=1;
	 if(!strcasecmp(mprg->Arg[0].CValue(), "noon") &&
	    (timeofday == 12))
	   action=1;
      }
      
      if(!action)
	continue;
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      
      VarPtrs[0] = &VarList[0];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 1, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

void mprog_greet_trigger2(char_data *ch) {
   char_data *vmob;
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!ch) return;
   if(ch->in_room == -1) return;
   
   for(vmob = real_roomp(ch->in_room)->people; vmob; vmob=vmob->next_in_room) {
      if(!IS_NPC(vmob))
	continue;
      
      if( (ch != vmob)
	 && AWAKE(vmob)
	 && !vmob->specials.fighting) {
	 
         global = mob_index[vmob->nr].global_vars;
	 for(mprg = mob_index[vmob->nr].mobprogs2; mprg; mprg=mprg->next) {
	    if(!strcasecmp(mprg->name, "all_greet_prog") ||
	       (!strcasecmp(mprg->name, "greet_prog") &&
		CAN_SEE(vmob, ch))) {
	       
	       if(mprg->Arg[0].type != 0) {
		  mlog(NULL, "Mob %i - Error in greet_prog / all_greet_prog - wrong type of arg: %i", GET_MOB_VNUM(vmob), mprg->Arg[0].type);
		  continue;
	       }

	       if(number(0,100) > mprg->Arg[0].Value())
		 continue;
	       
	       VarList[0].SetName("$t");
	       VarList[0].SetData(vmob);
	       VarList[1].SetName("$n");
	       VarList[1].SetData(ch);
	       
	       VarPtrs[0] = &VarList[0];
	       VarPtrs[1] = &VarList[1];
	       
               SetParents(mprg,global,vmob,2);
	       EvalFunc(TheFuncList, 2, VarPtrs, NULL);
	       ClearParents(mprg);
	    }
	 }
      }
   }
}

void mprog_random_trigger2(char_data *mob) {
   Function *mprg;
   int count;
   char_data *rnd, *iter;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!mob || !IS_NPC(mob)) return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg=mob_index[mob->nr].mobprogs2; mprg; mprg=mprg->next) {
      if(strcasecmp(mprg->name, "rand_prog"))
	continue;
      
      if(mob->in_room < 0) break;
      
      if(mprg->Arg[0].type != 0) {
	 mlog(NULL, "Mob %i - illegal argument type in rand_prog: %i", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }
      
      if(number(0,100) > mprg->Arg[0].Value())
	continue;
      
      count=0;
      rnd=NULL;
      for(iter=real_roomp(mob->in_room)->people; iter; iter=iter->next_in_room) {
	 if((iter != mob) &&
	    ((!rnd) || (!count) || (!number(0, count))))
	   rnd=iter;
	 
	 if(iter!=mob)
	   count++;
      }

      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      VarList[1].SetName("$r");
      VarList[1].SetData(rnd);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

void mprog_give_trigger2(char_data *mob, char_data *ch, obj_data *obj) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   int i, Found;

   if(!mob || !ch || !obj) return;
   
   if(!IS_NPC(mob)) return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg=mprg->next) {
      if(strcasecmp(mprg->name, "give_prog"))
	continue;
      
      Found=0;
      if(mprg->Arg[0].type == 0) {
	 for(i=1; i<mprg->Arg[0].Value()+1; i++) {
	    if(mprg->Arg[i].type != 1) {
	       mlog(NULL, "Mob %d - Improper type for object name in give_prog: %d (Arg #%d)", GET_MOB_VNUM(mob), mprg->Arg[i].type, i+1);
	       mlog(NULL, "Syntax: (num_items_in_list, item1, item2, item3...)");
	       mlog(NULL, "Given:  (%s)", mprg->argbuf);
	       continue;
	    }
	    
	    if(isname(mprg->Arg[i].CValue(), OBJ_NAME(obj))) {
	       Found=1;
	       break;
	    }
	 }
      } else {
	 if(mprg->Arg[0].type != 1) {
	    mlog(NULL, "Mob %d - Improper type for object name in give_prog: %d (Arg #1)", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	    continue;
	 }
	 
	 if(isname(mprg->Arg[0].CValue(), OBJ_NAME(obj)) ||
	    !strcasecmp(mprg->Arg[0].CValue(), "all")) {
	    Found=1;
	 }
      }
      
      if(!Found)
	continue;
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      VarList[2].SetName("$o");
      VarList[2].SetData(obj);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      VarPtrs[2] = &VarList[2];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList,3,VarPtrs,NULL);
      ClearParents(mprg);
   }
}

void mprog_speech_trigger2(char *txt, char_data *ch) {
   struct char_data *mob;
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   int Match, Miss;
   int i;
   
   if(ch->in_room == -1) return;
   
   for(mob = real_roomp(ch->in_room)->people; mob; mob = mob->next_in_room) {
      if(!IS_NPC(mob))
	continue;
      
      if(mob == ch)
	continue;
      
      if(mob->nr == ch->nr)
	continue;
      
      global = mob_index[mob->nr].global_vars;
      for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
	 if(strcasecmp(mprg->name, "speech_prog"))
	   continue;
	 
	 Match = -1;
	 if(mprg->Arg[0].type == 0) {
	    for(i=1;i<mprg->Arg[0].Value()+1;i++) {
	       if(mprg->Arg[i].type != 1) {
		  mlog(NULL, "Mob %d - Wrong argument type in speech_prog: %d (arg %d)", GET_MOB_VNUM(mob), mprg->Arg[i].type, i+1);
		  continue;
	       }
	       
	       if(FindSubStr(txt, mprg->Arg[i].CValue(), 0) > -1) {
                  Match = i;
		  break;
	       }
	    }
	 } else {
	    if(mprg->Arg[0].type != 1) {
	       mlog(NULL, "Mob %d - Wrong argument type in speech_prog: %d (arg 1)", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	       continue;
	    }
	    
	    if(!strcasecmp(mprg->Arg[0].CValue(), "any")) {
	       Match = 0;
	    } else if(!strcasecmp(mprg->Arg[0].CValue(), "all")) {
	       if(mprg->Arg[1].type != 0) {
		  mlog(NULL, "Mob %d - Wrong argument type in speech_prog: %d (arg 2)", GET_MOB_VNUM(mob), mprg->Arg[1].type);
		  continue;
	       }
	       
	       Miss = 0;
	       for(i=2;i<mprg->Arg[1].Value()+2;i++) {
		  if(mprg->Arg[i].type != 1) {
		     mlog(NULL, "Mob %d - Wrong argument type in speech_prog: %d (arg %i)", GET_MOB_VNUM(mob), mprg->Arg[i].type, i+1);
		     continue;
		  }
		  
		  if(FindSubStr(txt, mprg->Arg[i].CValue(), 0) == -1) {
		     Miss = 1;
		     break;
		  }
	       }
	       
	       if(!Miss)
		 Match = 0;
	       else
		 Match = -1;
	    } else {
	       if(FindSubStr(txt, mprg->Arg[0].CValue(), 0) == -1)
		 Match = -1;
	       else
		 Match = 0;
	    }
	 }
	 
	 if(Match < 0) continue;
	 
	 VarList[0].SetName("$t");
	 VarList[0].SetData(mob);
	 VarList[1].SetName("$n");
	 VarList[1].SetData(ch);
	 VarList[2].SetName("$m");
	 VarList[2].SetData(mprg->Arg[Match].CValue());
	 VarList[3].SetName("$s");
	 VarList[3].SetData(txt);
	 
	 VarPtrs[0] = &VarList[0];
	 VarPtrs[1] = &VarList[1];
	 VarPtrs[2] = &VarList[2];
	 VarPtrs[3] = &VarList[3];
	 
         SetParents(mprg,global,mob,2);
	 EvalFunc(TheFuncList, 4, VarPtrs, NULL);
	 ClearParents(mprg);
      }
   }
}

void mprog_death_trigger2(char_data *mob, char_data *killer) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!IS_NPC(mob))
     return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
      if(strcasecmp(mprg->name, "death_prog"))
	continue;
      
      if(mprg->Arg[0].type != 0) {
	 mlog(NULL, "Mob %d - Invalid argument type in death_prog: %d", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }
      
      if(number(0, 100) > mprg->Arg[0].Value())
	continue;
      
      GET_POS(mob) = POSITION_STANDING;
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      VarList[1].SetName("$n");
      VarList[1].SetData(killer);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

void mprog_fight_trigger2(char_data *mob, char_data *ch) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!IS_NPC(mob))
     return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
      if(strcasecmp(mprg->name, "fight_prog"))
	continue;
      
      if(mprg->Arg[0].type != 0) {
	 mlog(NULL, "Mob %d - Invalid type in fight_prog: %d", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }

      if(number(0, 100) > mprg->Arg[0].Value())
	continue;
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

void mprog_entry_trigger2(char_data *mob) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!IS_NPC(mob))
     return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
      if(strcasecmp(mprg->name, "entry_prog"))
	continue;
      
      if(mprg->Arg[0].type != 0) {
	 mlog(NULL, "Mob %d - Invalid argument type in entry_prog: %d", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }
      
      mlog(NULL, "entry_prog found");
      
      if(number(0,100) > mprg->Arg[0].Value())
	continue;
      
      mlog(NULL, "entry_prog going");
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      
      VarPtrs[0] = &VarList[0];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 1, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

void mprog_kill_trigger2(char_data *mob, char_data *vict) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(!IS_NPC(mob))
     return;
   
   global = mob_index[mob->nr].global_vars;
   for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
      if(strcasecmp(mprg->name, "kill_prog"))
	continue;
      
      if(mprg->Arg[0].type != 0) {
	 mlog(NULL, "Mob %d - Invalid argument type in kill_prog: %d", GET_MOB_VNUM(mob), mprg->Arg[0].type);
	 continue;
      }
      
      if(number(0, 100) > mprg->Arg[0].Value())
	continue;
      
      GET_POS(mob) = POSITION_STANDING;
      
      VarList[0].SetName("$t");
      VarList[0].SetData(mob);
      VarList[1].SetName("$n");
      VarList[1].SetData(vict);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(mprg,global,mob,2);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(mprg);
   }
}

int mprog_command_trigger(char *txt, char *arg, char_data *ch) {
   Function *mprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   int Match, Miss;
   int i;
   char_data *mob;
   
   if(ch->in_room < 0) return 0;
   if(!IS_PC(ch)) return 0;
   
   for(mob = real_roomp(ch->in_room)->people; mob; mob = mob->next_in_room) {
      global = mob_index[mob->nr].global_vars;
      for(mprg = mob_index[mob->nr].mobprogs2; mprg; mprg = mprg->next) {
	 if(strcasecmp(mprg->name, "command_prog"))
	   continue;
      
	 Match = -1;
	 if(mprg->Arg[0].type == 0) {
	    for(i=1;i<mprg->Arg[0].Value()+1;i++) {
	       if(mprg->Arg[i].type != 1) {
		  mlog(NULL, "Mob %ld - Wrong argument type in command_prog: %d (arg %d)", mob_index[mob->nr].virt, mprg->Arg[i].type, i+1);
		  continue;
	       }
	    
	       if(is_abbrev(txt, mprg->Arg[i].CValue())) {
		  Match = i;
		  break;
	       }
	    }
	 } else {
	    if(mprg->Arg[0].type != 1) {
	       mlog(NULL, "Mob %ld - Wrong argument type in command_prog: %d (arg 1)", mob_index[mob->nr].virt, mprg->Arg[0].type);
	       continue;
	    }
	    if(!strcasecmp(mprg->Arg[0].CValue(), "all")) {
	       if(mprg->Arg[1].type != 0) {
		  mlog(NULL, "Mob %ld - Wrong argument type in command_prog: %d (arg 2)", mob_index[mob->nr].virt, mprg->Arg[1].type);
		  continue;
	       }
	       Match = 0;
	       for(i=2;i<mprg->Arg[1].Value()+2;i++) {
		  if(mprg->Arg[i].type != 1) {
		     mlog(NULL, "Mob %ld - Wrong argument type in command_prog: %d (arg %i)", mob_index[mob->nr].virt, mprg->Arg[i].type, i+1);
		     continue;
		  }
		  if(!is_abbrev(txt, mprg->Arg[i].CValue())) {
		     Match = -1;
		     break;
		  }
	       }
	    } else if(is_abbrev(txt, mprg->Arg[0].CValue())) {
	       Match = 0;
	    }
	 }

	 if(Match < 0) continue;
      
	 VarList[0].SetName("$t");
	 VarList[0].SetData(mob);
	 VarList[1].SetName("$n");
	 VarList[1].SetData(ch);
	 VarList[2].SetName("$m");
	 VarList[2].SetData(mprg->Arg[Match].CValue());
	 VarList[3].SetName("$s");
	 VarList[3].SetData(txt);
	 VarList[4].SetName("$a");
	 VarList[4].SetData(arg);
	 VarPtrs[0] = &VarList[0];
	 VarPtrs[1] = &VarList[1];
	 VarPtrs[2] = &VarList[2];
	 VarPtrs[3] = &VarList[3];
	 VarPtrs[4] = &VarList[4];

         SetParents(mprg,global,mob,2);
	 EvalFunc(TheFuncList, 5, VarPtrs, &RetVal);
	 ClearParents(mprg);

	 if(RetVal.type == 0) {
	    return RetVal.Value();
	 }
      }
   }
   
   return 0;
}




////////////////////////////////////////
//    Object Triggers
////////////////////////////////////////

void oprog_wear_trigger(obj_data *obj, char_data *ch) {
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(obj->item_number < 0)
     return;
   
   global = obj_index[obj->item_number].global_vars;
   for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
      if(strcasecmp(oprg->name, "wear_prog"))
	continue;

      if(oprg->Arg[0].type != 0) {
	 mlog(NULL, "Obj %d - Invalid argument type in wear_prog: %d", GET_OBJ_VNUM(obj), oprg->Arg[0].type);
	 continue;
      }

      if(number(0,100) > oprg->Arg[0].Value())
	continue;
      
      VarList[0].SetName("$o");
      VarList[0].SetData(obj);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(oprg,global,obj,3);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(oprg);
   }
}

void oprog_remove_trigger(obj_data *obj, char_data *ch) {
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(obj->item_number < 0)
     return;
   
   global = obj_index[obj->item_number].global_vars;
   for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
      if(strcasecmp(oprg->name, "remove_prog"))
	continue;
      
      if(oprg->Arg[0].type != 0) {
	 mlog(NULL, "Obj %d - Illegal argument type in remove_prog: %d", GET_OBJ_VNUM(obj), oprg->Arg[0].type);
	 continue;
      }
      
      if(number(0,100) > oprg->Arg[0].Value())
	continue;
      
      VarList[0].SetName("$o");
      VarList[0].SetData(obj);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(oprg,global,obj,3);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(oprg);
   }
}

void oprog_fight_trigger(char_data *ch, char_data *vict) {
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   obj_data *obj;
   int i;
   
   if(!ch || !vict)
     return;
   
   for(i=0;i<MAX_WEAR;i++) {
      obj=ch->equipment[i];
      if(!obj)
	continue;
      
      if(obj->item_number < 0)
	continue;
      
      global = obj_index[obj->item_number].global_vars;
      for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
	 if(strcasecmp(oprg->name, "fight_prog"))
	   continue;
	 
	 if(oprg->Arg[0].type != 0) {
	    mlog(NULL, "Obj %d - Invalid argument type in fight_prog: %d (%s)",
		 GET_OBJ_VNUM(obj), oprg->Arg[0].type, oprg->name);
	    continue;
	 }
	 
	 if(number(0, 100) > oprg->Arg[0].Value())
	   continue;
	 
	 VarList[0].SetName("$o");
	 VarList[0].SetData(obj);
	 VarList[1].SetName("$n");
	 VarList[1].SetData(ch);
	 VarList[2].SetName("$t");
	 VarList[2].SetData(vict);
	 
	 VarPtrs[0] = &VarList[0];
	 VarPtrs[1] = &VarList[1];
	 VarPtrs[2] = &VarList[2];
	 
         SetParents(oprg,global,obj,3);
	 EvalFunc(TheFuncList, 3, VarPtrs, NULL);
	 ClearParents(oprg);
      }
   }
}

void oprog_get_trigger(obj_data *obj, char_data *ch) {
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(obj->item_number < 0)
     return;
   
   global = obj_index[obj->item_number].global_vars;
   for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
      if(strcasecmp(oprg->name, "get_prog"))
	continue;
      
      if(oprg->Arg[0].type != 0) {
	 mlog(NULL, "Obj %d - Illegal argument type in get_prog: %d", GET_OBJ_VNUM(obj), oprg->Arg[0].type);
	 continue;
      }
      
      if(number(0,100) > oprg->Arg[0].Value())
	continue;
      
      VarList[0].SetName("$o");
      VarList[0].SetData(obj);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(oprg,global,obj,3);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(oprg);
   }
}

void oprog_drop_trigger(obj_data *obj, char_data *ch) {
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   
   if(obj->item_number < 0)
     return;
   
   global = obj_index[obj->item_number].global_vars;
   for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
      if(strcasecmp(oprg->name, "drop_prog"))
	continue;
      
      if(oprg->Arg[0].type != 0) {
	 mlog(NULL, "Obj %d - Illegal argument type in drop_prog: %d", GET_OBJ_VNUM(obj), oprg->Arg[0].type);
	 continue;
      }
      
      if(number(0,100) > oprg->Arg[0].Value())
	continue;
      
      VarList[0].SetName("$o");
      VarList[0].SetData(obj);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(oprg,global,obj,3);
      EvalFunc(TheFuncList, 2, VarPtrs, NULL);
      ClearParents(oprg);
   }
}

void oprog_random_trigger() {
   Function *oprg;
   int count, tick, objtick;
   char_data *rnd, *iter;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   obj_data *obj;
   room_num rmnum;

   EACH_OBJECT(iter2, obj) {
      if(!obj) continue;
      if(obj->item_number < 0) continue;
      
      tick = (pulse / PULSE_MOBILE);
      objtick = obj->obj_flags.tick;

      global = obj_index[obj->item_number].global_vars;
      for(oprg=obj_index[obj->item_number].objprogs2; oprg; oprg=oprg->next) {
         if(strcasecmp(oprg->name, "rand_prog"))
  	   continue;
      
         if(oprg->Arg[0].type != 0) {
  	    mlog(NULL, "Obj %i - illegal argument type in rand_prog: %i", GET_OBJ_VNUM(obj), oprg->Arg[0].type);
   	    continue;
         }
	 
	 if((oprg->NumArgs > 1) && (oprg->Arg[1].type == 0)) {
	    if((objtick % oprg->Arg[1].Value()) != (tick % oprg->Arg[1].Value()))
	      continue;
	 } else if((objtick % MOB_DIVISOR) != (tick % MOB_DIVISOR)) {
	      continue;
	 }
      
         if(number(0,100) > oprg->Arg[0].Value())
	   continue;
      
	 if(char_holding(obj))
	   rmnum=char_holding(obj)->in_room;
	 else
	   rmnum=obj->in_room;
	 
	 if(rmnum <= 0)
	    continue;

         count=0;
         rnd=NULL;
         for(iter=real_roomp(rmnum)->people; iter; iter=iter->next_in_room) {
   	    if((!rnd) || (!count) || (!number(0, count)))
	      rnd=iter;
	 
	      count++;
         }
	 
         VarList[0].SetName("$o");
         VarList[0].SetData(obj);
         VarList[1].SetName("$r");
         VarList[1].SetData(rnd);
      
         VarPtrs[0] = &VarList[0];
         VarPtrs[1] = &VarList[1];
      
         SetParents(oprg,global,obj,3);
         EvalFunc(TheFuncList, 2, VarPtrs, NULL);
         ClearParents(oprg);
      }
   } END_AITER(iter2);
}

int check_oprog_command(obj_data *obj, char *txt, char *arg, char_data *ch) {
   Function *oprg;
   Variable *global, RetVal;
   Variable VarList[10];
   Variable *VarPtrs[10];
   int Match, i;
   
   if(!obj) return -1;
   if(obj->item_number < 0) return -1;
   
   global = obj_index[obj->item_number].global_vars;
   for(oprg = obj_index[obj->item_number].objprogs2; oprg; oprg = oprg->next) {
      if(strcasecmp(oprg->name, "command_prog"))
	continue;
      
      Match = -1;
      if(oprg->Arg[0].type == 0) {
	 for(i=1;i<oprg->Arg[0].Value()+1;i++) {
	    if(oprg->Arg[i].type != 1) {
	       mlog(NULL, "Obj %d - Wrong argument type in command_prog: %d (arg %d)", obj_index[obj->item_number].virt, oprg->Arg[i].type, i+1);
	       continue;
	    }
	    
	    if(is_abbrev(txt, oprg->Arg[i].CValue())) {
	       Match = i;
	       break;
	    }
	 }
      } else {
	 if(oprg->Arg[0].type != 1) {
	    mlog(NULL, "Obj %d - Wrong argument type in command_prog: %d (arg 1)", obj_index[obj->item_number].virt, oprg->Arg[0].type);
	    continue;
	 }
	 
	 if(!strcasecmp(oprg->Arg[0].CValue(), "all")) {
	    if(oprg->Arg[1].type != 0) {
	       mlog(NULL, "Obj %d - Wrong argument type in command_prog: %d (arg 2)", obj_index[obj->item_number].virt, oprg->Arg[1].type);
	       continue;
	    }
	    
	    Match = 0;
	    for(i=2;i<oprg->Arg[1].Value()+2;i++) {
	       if(oprg->Arg[i].type != 1) {
		  mlog(NULL, "Obj %d - Wrong argument type in command_prog: %d (arg %i)", obj_index[obj->item_number].virt, oprg->Arg[i].type, i+1);
		  continue;
	       }
	       if(!is_abbrev(txt, oprg->Arg[i].CValue())) {
		  Match = -1;
		  break;
	       }
	    }
	 } else if(is_abbrev(txt, oprg->Arg[0].CValue())) {
	    Match = 0;
	 }
      }

      if(Match < 0) continue;
      
      VarList[0].SetName("$o");
      VarList[0].SetData(obj);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      VarList[2].SetName("$m");
      VarList[2].SetData(oprg->Arg[Match].CValue());
      VarList[3].SetName("$s");
      VarList[3].SetData(txt);
      VarList[4].SetName("$a");
      VarList[4].SetData(arg);

      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      VarPtrs[2] = &VarList[2];
      VarPtrs[3] = &VarList[3];
      VarPtrs[4] = &VarList[4];
      
      SetParents(oprg,global,obj,3);
      EvalFunc(TheFuncList, 5, VarPtrs, &RetVal);
      ClearParents(oprg);
      
      if(RetVal.type == 0) {
	 return RetVal.Value();
      }
   }

   return -1;
}

int oprog_command_trigger(char *txt, char *arg, char_data *ch) {
   struct obj_data *obj;
   Function *oprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   int Match, Miss;
   int i,j;
   
   if(ch->in_room < 0) return 0;
   if(!IS_PC(ch)) return 0;
   
   for(obj=real_roomp(ch->in_room)->contents;obj;obj=obj->next_content) {
      if((i = check_oprog_command(obj,txt,arg,ch)) == -1)
	continue;
      else
        return i;
   }
   
   for(obj=ch->carrying;obj;obj=obj->next_content) {
      if((i = check_oprog_command(obj,txt,arg,ch)) == -1)
	continue;
      else
	return i;
   }
   
   for(j=0;j<MAX_WEAR;j++) {
      obj = ch->equipment[j];
      if((i = check_oprog_command(obj,txt,arg,ch)) == -1)
	continue;
      else
	return i;
   }
   
   return 0;
}





////////////////////////////////////////
//    Room Triggers
////////////////////////////////////////

void rprog_speech_trigger(char *txt, char_data *ch) {
   struct room_data *rm;
   Function *rprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   int Match, Miss;
   int i;
   
   if(ch->in_room < 0) return;
   
   rm = real_roomp(ch->in_room);
   
   global = rm->global_vars;
   for(rprg = rm->roomprogs2; rprg; rprg = rprg->next) {
      if(strcasecmp(rprg->name, "speech_prog"))
	continue;
	 
      Match = -1;
      if(rprg->Arg[0].type == 0) {
	 for(i=1;i<rprg->Arg[0].Value()+1;i++) {
	    if(rprg->Arg[i].type != 1) {
	       mlog(NULL, "Room %ld - Wrong argument type in speech_prog: %d (arg %d)", rm->number, rprg->Arg[i].type, i+1);
	       continue;
	    }
	    
	    if(FindSubStr(txt, rprg->Arg[i].CValue(), 0) > -1) {
	       Match = i;
	       break;
	    }
	 }
      } else {
	 if(rprg->Arg[0].type != 1) {
	    mlog(NULL, "Room %ld - Wrong argument type in speech_prog: %d (arg 1)", rm->number, rprg->Arg[0].type);
	    continue;
	 }
	 
	 if(!strcasecmp(rprg->Arg[0].CValue(), "any")) {
	    Match = 0;
	 } else if(!strcasecmp(rprg->Arg[0].CValue(), "all")) {
	    if(rprg->Arg[1].type != 0) {
	       mlog(NULL, "Room %ld - Wrong argument type in speech_prog: %d (arg 2)", rm->number, rprg->Arg[1].type);
	       continue;
	    }
	    
	    Miss = 0;
	    for(i=2;i<rprg->Arg[1].Value()+2;i++) {
	       if(rprg->Arg[i].type != 1) {
		  mlog(NULL, "Room %ld - Wrong argument type in speech_prog: %d (arg %i)", rm->number, rprg->Arg[i].type, i+1);
		  continue;
	       }
	       if(FindSubStr(txt, rprg->Arg[i].CValue(), 0) == -1) {
		  Miss = 1;
		  break;
	       }
	    }
	    
	    if(!Miss)
	      Match = 0;
	    else
	      Match = -1;
	 } else {
	    if(FindSubStr(txt, rprg->Arg[0].CValue(), 0) == -1)
	      Match = -1;
	    else
	      Match = 0;
	 }
      }

      if(Match < 0) continue;
      
      VarList[0].SetName("$rm");
      VarList[0].SetData(rm);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      VarList[2].SetName("$m");
      VarList[2].SetData(rprg->Arg[Match].CValue());
      VarList[3].SetName("$s");
      VarList[3].SetData(txt);

      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      VarPtrs[2] = &VarList[2];
      VarPtrs[3] = &VarList[3];

      SetParents(rprg,global,rm,4);
      EvalFunc(TheFuncList, 4, VarPtrs, NULL);
      ClearParents(rprg);
   }
}

int rprog_command_trigger(char *txt, char *arg, char_data *ch) {
   struct room_data *rm;
   Function *rprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   int Match, Miss;
   int i;
   
   if(ch->in_room < 0) return 0;
   if(!IS_PC(ch)) return 0;
   
   rm = real_roomp(ch->in_room);
   
   global = rm->global_vars;
   for(rprg = rm->roomprogs2; rprg; rprg = rprg->next) {
      if(strcasecmp(rprg->name, "command_prog"))
	continue;
      
      Match = -1;
      if(rprg->Arg[0].type == 0) {
	 for(i=1;i<rprg->Arg[0].Value()+1;i++) {
	    if(rprg->Arg[i].type != 1) {
	       mlog(NULL, "Room %ld - Wrong argument type in command_prog: %d (arg %d)", rm->number, rprg->Arg[i].type, i+1);
	       continue;
	    }
	    
	    if(is_abbrev(txt, rprg->Arg[i].CValue())) {
	       Match = i;
	       break;
	    }
	 }
      } else {
	 if(rprg->Arg[0].type != 1) {
	    mlog(NULL, "Room %ld - Wrong argument type in command_prog: %d (arg 1)", rm->number, rprg->Arg[0].type);
	    continue;
	 }
	 
	 if(!strcasecmp(rprg->Arg[0].CValue(), "all")) {
	    if(rprg->Arg[1].type != 0) {
	       mlog(NULL, "Room %ld - Wrong argument type in command_prog: %d (arg 2)", rm->number, rprg->Arg[1].type);
	       continue;
	    }
	    
	    Match = 0;
	    for(i=2;i<rprg->Arg[1].Value()+2;i++) {
	       if(rprg->Arg[i].type != 1) {
		  mlog(NULL, "Room %ld - Wrong argument type in command_prog: %d (arg %i)", rm->number, rprg->Arg[i].type, i+1);
		  continue;
	       }
	       if(!is_abbrev(txt, rprg->Arg[i].CValue())) {
		  Match = -1;
		  break;
	       }
	    }
	 } else if(is_abbrev(txt, rprg->Arg[0].CValue())) {
	    Match = 0;
	 }
      }

      if(Match < 0) continue;
      
      VarList[0].SetName("$rm");
      VarList[0].SetData(rm);
      VarList[1].SetName("$n");
      VarList[1].SetData(ch);
      VarList[2].SetName("$m");
      VarList[2].SetData(rprg->Arg[Match].CValue());
      VarList[3].SetName("$s");
      VarList[3].SetData(txt);
      VarList[4].SetName("$a");
      VarList[4].SetData(arg);

      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      VarPtrs[2] = &VarList[2];
      VarPtrs[3] = &VarList[3];
      VarPtrs[4] = &VarList[4];
      
      SetParents(rprg,global,rm,4);
      EvalFunc(TheFuncList, 5, VarPtrs, &RetVal);
      ClearParents(rprg);
      
      if(RetVal.type == 0) {
	 return RetVal.Value();
      }
   }
   
   return 0;
}

void rprog_random_trigger() {
   Function *rprg;
   int count,tick=0,roomtick=0;
   char_data *rnd, *iter;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable *global;
   room_data *room;

   for(room_num i=0; i<=top_of_world; i++) {
      if(!(room = room_db[i])) continue;
      
      tick = (pulse/PULSE_MOBILE);
      roomtick = expomod(i,3,35);
      
      global = room->global_vars;
      for(rprg=room->roomprogs2; rprg; rprg=rprg->next) {
         if(strcasecmp(rprg->name, "rand_prog"))
  	   continue;

         if(rprg->Arg[0].type != 0) {
  	    mlog(NULL, "Room %i - illegal argument type in rand_prog: %i", i, rprg->Arg[0].type);
   	    continue;
         }
      
         if((rprg->NumArgs > 1) && (rprg->Arg[1].type == 0)) {
	    if((roomtick % rprg->Arg[1].Value()) != (tick % rprg->Arg[1].Value()))
	      continue;
	 } else if((roomtick % MOB_DIVISOR) != (tick % MOB_DIVISOR)) {
	    continue;
         }
	 
         if(number(0,100) > rprg->Arg[0].Value())
	   continue;
      
         count=0;
         rnd=NULL;
         for(iter=room->people; iter; iter=iter->next_in_room) {
   	    if((!rnd) || (!count) || (!number(0, count)))
	      rnd=iter;
	 
	      count++;
         }
	 
         VarList[0].SetName("$rm");
         VarList[0].SetData(room);
         VarList[1].SetName("$r");
         VarList[1].SetData(rnd);
      
         VarPtrs[0] = &VarList[0];
         VarPtrs[1] = &VarList[1];
      
         SetParents(rprg,global,room,4);
         EvalFunc(TheFuncList, 2, VarPtrs, NULL);
         ClearParents(rprg);
      }
   }
}




////////////////////////////////////////
//    Zone Triggers
////////////////////////////////////////

zone_data *ZoneFromRoom(room_num rm) {
   int i;
   zone_data *curzone;
   
   curzone=NULL;
   for(i=0;i<=top_of_zone_table;i++) {
      if(zone_table[i].top > rm) {
	 curzone = &zone_table[i];
	 break;
      }
   }
   
   return curzone;
}

int zprog_death_trigger(char_data *ch) {
   Function *zprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   zone_data *zone;
   
   if(!ch) return 0;
   if(!IS_PC(ch)) return 0;
   
   zone = ZoneFromRoom(ch->in_room);
   if(!zone) return 0;

   global = zone->global_vars;
   for(zprg = zone->zoneprogs2; zprg; zprg=zprg->next) {
      if(strcasecmp(zprg->name, "death_prog"))
	continue;
      
      VarList[0].SetName("$n");
      VarList[0].SetData(ch);
      VarList[1].SetName("$r");
      VarList[1].SetData(ch->in_room);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(zprg,global,zone,5);
      EvalFunc(TheFuncList, 2, VarPtrs, &RetVal);
      ClearParents(zprg);
      
      if(RetVal.type == 0) {
	 return RetVal.Value();
      }
   }
   
   return 0;
}

int zprog_arena_trigger(char_data *ch) {
   Function *zprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   zone_data *zone;
   
   if(!ch) return 0;
   if(!IS_PC(ch)) return 0;
   
   zone = ZoneFromRoom(ch->in_room);
   if(!zone) return 0;

   global = zone->global_vars;
   for(zprg = zone->zoneprogs2; zprg; zprg=zprg->next) {
      if(strcasecmp(zprg->name, "arena_prog"))
	continue;
      
      VarList[0].SetName("$n");
      VarList[0].SetData(ch);
      VarList[1].SetName("$r");
      VarList[1].SetData(ch->in_room);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(zprg,global,zone,5);
      EvalFunc(TheFuncList, 2, VarPtrs, &RetVal);
      ClearParents(zprg);
      
      if(RetVal.type == 0) {
	 return RetVal.Value();
      }
   }
   
   return 0;
}

int zprog_track_trigger(char_data *vict) {
   Function *zprg;
   Variable VarList[10];
   Variable *VarPtrs[10];
   Variable RetVal;
   Variable *global;
   zone_data *zone;
   
   if(!vict) return 0;
   
   zone = ZoneFromRoom(vict->in_room);
   if(!zone) return 0;
   
   global = zone->global_vars;
   for(zprg = zone->zoneprogs2; zprg; zprg=zprg->next) {
      if(strcasecmp(zprg->name, "track_prog"))
	continue;
      
      VarList[0].SetName("$r");
      VarList[0].SetData(vict->in_room);
      VarList[1].SetName("$t");
      VarList[1].SetData(vict);
      
      VarPtrs[0] = &VarList[0];
      VarPtrs[1] = &VarList[1];
      
      SetParents(zprg,global,zone,5);
      EvalFunc(TheFuncList, 2, VarPtrs, &RetVal);
      ClearParents(zprg);
      
      if(RetVal.type == 0) {
	 return RetVal.Value();
      }
   }
   
   return 0;
}
