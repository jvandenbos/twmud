#ifndef VARFUNC_H
#define VARFUNC_H

#include <string.h>

class Function;
class Variable;

void mlog(Function *F, char *, ...);

class Variable {
public:
   char name[255];
   void *data;
   int size;
   int type;
   Variable *next;

   Variable() {
      data=NULL;
      strcpy(name,"");
      size=0;
      next=NULL;
      type=-1;
   }

   ~Variable() {
       ClearMem();
   }

   void Add(Variable *v1, Variable *v2) {
      char *data_c=NULL;
      char buf[255];
      
      if((v1->type) > 1 || (v2->type > 1)) {
	 mlog(NULL, "Improper types in Variable Add");
	 return;
      }
      
      switch(v1->type) {
      case 0:		//easy, longs
	 if(v2->type == 0) {
	    SetData(v1->Value() + v2->Value());
	    break;
	 } else {
	    sprintf(buf, "%ld", v1->Value());
	    data_c = new char[strlen(buf)+strlen(v2->CValue())+1];
	    strcpy(data_c,buf);
	    strcat(data_c,v2->CValue());
	    SetData(data_c);
	    delete data_c;
	    break;
	 }
      case 1:
	 if(v2->type == 1) {
	    data_c = new char[strlen(v1->CValue())+strlen(v2->CValue())+1];
	    strcpy(data_c, v1->CValue());
	    strcat(data_c, v2->CValue());
	    SetData(data_c);
	    delete data_c;
	 } else {
	    sprintf(buf, "%ld", v2->Value());
	    data_c = new char[strlen(v1->CValue())+strlen(buf)+1];
	    strcpy(data_c, v1->CValue());
	    strcat(data_c, buf);
	    SetData(data_c);
	    delete data_c;
	 }
      }
   }
   
   int CheckLogical(char *opr) {
      if(!strcmp(opr, "&&")) {
	 if(type == 1) {
	    return !*CValue();
	 } else {
	    return !Value();
	 }
      }
      
      if(!strcmp(opr, "||")) {
	 if(type == 1) {
	    return *CValue();
	 } else {
	    return Value();
	 }
      }
      
      return 0;
   }

   void Compare(Variable *v1, Variable *v2) {
      if(v1->type != v2->type) {
      	 mlog(NULL, "Incompatible types in Variable Compare: %s - %i, %s - %i", v1->name, v1->type, v2->name, v2->type);
       	 return;
      }

      switch(v1->type) {
      case 0:		//longs
      	 SetData(v1->Value() == v2->Value());
       	 break;
      case 1:		//string
       	 SetData(!strcasecmp(v1->CValue(), v2->CValue()));
	 
       	 break;
      default:		//byte by byte
       	 if(v1->size != v2->size) {
       	    SetData((long)0);
       	    break;
       	 }

       	 SetData(!memcmp(v1->data, v2->data, v1->size));
      }
   }

   void CopyData(Variable *V) {
      ClearMem();

      if(V->size)
        SetData(V->data, V->size);

      type = V->type;
      size = V->size;
   }

   void CopyVar(Variable *V) {
      ClearMem();

      SetData(V->data, V->size);
      SetName(V->name);
      type = V->type;
      size = V->size;
   }

   void ClearMem() {
      if(data)
       	delete data;
      data = NULL;
   }

   int Math(Variable *lhs, Variable *rhs, char *opr) {
      if(!strcmp(opr, "+")) {
      	 Add(lhs, rhs);
       	 return 1;
      } else if(!strcmp(opr, "==")) {
       	 Compare(lhs, rhs);
       	 return 1;
      } else if(!strcmp(opr, "!=")) {
	 Compare(lhs, rhs);
	 SetData(!Value());
	 return 1;
      }

      if(((lhs->type != 0) || (rhs->type != 0)) &&
	 (strcmp(opr, "||")) && (strcmp(opr, "&&"))) {
       	 mlog(NULL, "Operators are only allowed with integer-type variables (%s, {%i,%i}, '%s')",
	      name, lhs->type, rhs->type, opr);
       	 return 0;
      }

      if(!strcmp(opr, "-")) {
       	 SetData(lhs->Value()-rhs->Value());
	 return 1;
      }

      if(!strcmp(opr, "*")) {
       	 SetData(lhs->Value()*rhs->Value());
	 return 1;
      }

      if(!strcmp(opr, "/")) {
       	 SetData(lhs->Value()/rhs->Value());
       	 return 1;
      }

      if(!strcmp(opr, ">")) {
       	 SetData(lhs->Value()>rhs->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, ">=")) {
	 SetData(lhs->Value()>=rhs->Value());
	 return 1;
      }

      if(!strcmp(opr, "<")) {
       	 SetData(lhs->Value()<rhs->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "<=")) {
	 SetData(lhs->Value()<=rhs->Value());
	 return 1;
      }

      if(!strcmp(opr, "&")) {
       	 SetData(lhs->Value()&rhs->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "&&")) {
	 SetData(lhs->Value()&&rhs->Value());
	 return 1;
      }
      
      if(!strcmp(opr, "|")) {
       	 SetData(lhs->Value()|rhs->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "||")) {
	 SetData(lhs->Value()||rhs->Value());
	 return 1;
      }
      
      if(!strcmp(opr, "^")) {
       	 SetData(lhs->Value()^rhs->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "~")) {
	 SetData(lhs->Value() & (~rhs->Value()));
	 return 1;
      }
      
      if(!strcmp(opr, "%")) {
	 SetData(lhs->Value() % (rhs->Value()));
	 return 1;
      }
      
      if(!strcmp(opr, "<<")) {
	 SetData(lhs->Value() << rhs->Value());
	 return 1;
      }
      
      if(!strcmp(opr, ">>")) {
	 SetData(lhs->Value() >> rhs->Value());
	 return 1;
      }
      
      mlog(NULL, "Unknown operator in math (%s): %s", name, opr);
      return 1;
   }

   int MathAssign(char *opr, Variable *to) {
      if(!strcmp(opr, "=")) {
      	 CopyData(to);
       	 return 1;
      }

      if(type == -1) {
       	 mlog(NULL, "MathAssign called with %s, and %s had NULL data",
	      opr, name);
       	 return 0;
      }

      if(!strcmp(opr, "+=")) {
       	 Add(this,to);
	 return 1;
      }

      if(type != 0) {
       	 mlog(NULL, "MathAssign called with %s, and %s was not numeric!",
	      opr, name);
       	 return 0;
      }

      if(!strcmp(opr, "-=")) {
       	 SetData(Value()-to->Value());
	 return 1;
      }

      if(!strcmp(opr, "*=")) {
       	 SetData(Value()*to->Value());
	 return 1;
      }

      if(!strcmp(opr, "/=")) {
       	 SetData(Value()/to->Value());
       	 return 1;
      }

      if(!strcmp(opr, "&=")) {
       	 SetData(Value()&to->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "|=")) {
       	 SetData(Value()|to->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "^=")) {
       	 SetData(Value()^to->Value());
       	 return 1;
      }
      
      if(!strcmp(opr, "~=")) {
       	 SetData(~to->Value());
	 return 1;
      }
      
      if(!strcmp(opr, "%=")) {
       	 SetData(Value()%to->Value());
	 return 1;
      }
      
      if(!strcmp(opr, "<<=")) {
       	 SetData(Value() << to->Value());
	 return 1;
      }
      
      if(!strcmp(opr, ">>=")) {
       	 SetData(Value() >> to->Value());
	 return 1;
      }
      
      mlog(NULL, "Unknown operator in math assign (%s): %s", name, opr);
      return 1;
   }

   char *SetName(char *n) {
      strcpy(name, n);
      return name;
   }

   void SetData(long l) {
      ClearMem();

      data = new long;
      memcpy(data,&l,sizeof(long));
      size=sizeof(long);
      type=0;
   }

   void SetData(char *s) {
      ClearMem();

      data = new char[strlen(s)+1];
      strcpy((char*)data, s);
      size=strlen(s)+1;
      type=1;
   }

   void SetData(char_data *m) {
      data = new char_data*;
      memcpy(data,&m,sizeof(char_data*));
      size=sizeof(char_data*);
      type=2;
   }

   void SetData(obj_data *o) {
      data = new obj_data*;
      memcpy(data,&o,sizeof(obj_data*));
      size=sizeof(obj_data*);
      type=3;
   }

   void SetData(room_data *r) {
      data = new room_data*;
      memcpy(data,&r,sizeof(room_data*));
      size=sizeof(room_data*);
      type=4;
   }

   void SetData(void *d, int s) {
      ClearMem();

      data = new char[s];
      memcpy(data,d,s);
      size=s;
      type=10;
   }

   void SetDataNull() {
      ClearMem();

      data = NULL;
      size = 0;
      type = -1;
   }

   long Value() {
      if(data)
       	return *(long *)data;

      return 0;
   }

   char *CValue() {
      if(data)
       	return (char *)data;

      return NULL;
   }
};

class Function {
public:
   char name[255];
   char *code;
   int InitArg[32];
   Variable Arg[32];
   Variable *Loc;			//is a linked list
   int NumArgs;
   int NumLocs;
   char argbuf[255];
   char_data *mob_parent;
   obj_data  *obj_parent;
   room_data *room_parent;
   zone_data *zone_parent;
   Function *next, *prev;
   
   //On whom information
   int Type;
   long VNum;

   Function() {
      code=NULL;
      strcpy(name, "");
      NumLocs=NumArgs=0;
      Loc=NULL;
      mob_parent=NULL;
      obj_parent=NULL;
      room_parent=NULL;
      zone_parent=NULL;
      prev=next=NULL;
      Type=VNum=0;
   }

   ~Function() {
	ClearMem();
   }

   void CopyFunc(Function *F) {
      int i;
      Variable *iter;

      ClearMem();

      SetCode(F->code);
      SetName(F->name);
      NumArgs = F->NumArgs;
      for(i=0;i<NumArgs;i++) {
	 Arg[i].CopyVar(&F->Arg[i]);
	 InitArg[i] = F->InitArg[i];
      }
      for(iter=F->Loc;iter;iter=iter->next)
	NewLoc(iter);

      NumLocs = F->NumLocs;

      strcpy(argbuf, F->argbuf);

      CopyParents(F);
      
      next=F->next;
      prev=F->prev;
   }

   void CopyParents(Function *F) {
      mob_parent = F->mob_parent;
      obj_parent = F->obj_parent;
      room_parent = F->room_parent;
      zone_parent = F->zone_parent;
   }
   
   void ClearMem() {
      Variable *tmp;

      if(code)
	delete code;
      code = NULL;

      while(Loc) {
	 tmp = Loc->next;
	 delete Loc;
	 Loc = tmp;
      }
   }
   
   void ClearParent() {
      mob_parent = NULL;
      obj_parent = NULL;
      room_parent = NULL;
      zone_parent = NULL;
   }
   
   Variable *NewLoc(Variable *from) {
      Variable *tmp = Loc;

      Loc = new Variable;
      Loc->next = tmp;
      Loc->CopyVar(from);
      NumLocs++;

      return Loc;
   }

   void SetCode(char *s) {
      ClearMem();

      code = new char[strlen(s)+1];
      strcpy((char*)code, s);
   }

   void SetCode(char *s, int n) {
      ClearMem();

      code = new char[n+1];
      strncpy((char*)code, s, n);
      code[n]=0;
   }

   void SetName(char *s) {
      strcpy(name, s);
   }
};

#endif
