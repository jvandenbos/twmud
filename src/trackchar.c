//This file keeps track of all of the characters that log onto TW.

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include "structs.h"
#include "db.h"
#include "utility.h"
#include "comm.h"
#include "trackchar.h"
#include "find.h"
#include "util_str.h"
#include "multiclass.h"
#include "recept.h"
#include "handler.h"

// Stuff for the nodes

CharTrackerNode::CharTrackerNode() {
   memset(name, 0, sizeof(name));
   memset(host, 0, sizeof(host));
   num_objs = level = 0;
   next = NULL;
}

CharTrackerNode::~CharTrackerNode() {
   //nothing here
}

void CharTrackerNode::SetName(char *buf) {
   strcpy(name, buf);
}

void CharTrackerNode::SetObjs(long num) {
   num_objs = num;
}

void CharTrackerNode::SetLevel(long l) {
   level = l;
}

void CharTrackerNode::SetLaston(time_t l) {
   laston = l;
}

void CharTrackerNode::SetHost(char *buf) {
   strcpy(host, buf);
}






CharTrackerIgnore::CharTrackerIgnore() {
   all = 0;
   list = NULL;
}

CharTrackerIgnore::~CharTrackerIgnore() {
   CharTrackerIgnoreNode *node;
   
   while(list) {
      node = list->next;
      delete list;
      list = node;
   }
}

int CharTrackerIgnore::IsIgnoring(CharTrackerGroup *group) {
   CharTrackerIgnoreNode *node;
   
   if(all) return TRUE;
   
   for(node=list;node;node=node->next) {
      if(node->group == group)
	return TRUE;
   }
   
   return FALSE;
}

void CharTrackerIgnore::AddIgnore(CharTrackerGroup *group) {
   CharTrackerIgnoreNode *node;
   
   node = new CharTrackerIgnoreNode;
   node->next = list;
   node->group = group;
   list = node;
}

void CharTrackerIgnore::IgnoreAll() {
   all = TRUE;
}

void CharTrackerIgnore::RemIgnore(CharTrackerGroup *group) {
   CharTrackerIgnoreNode *prev, *node;
   
   prev=NULL;
   for(node=list;node;node=node->next)
     if(node->group == group)
       break;
   
   if(!node) return;
   if(!prev) {
      list = node->next;
   } else {
      prev->next = node->next;
   }
   
   delete node;
}

void CharTrackerIgnore::CopyList(CharTrackerIgnore *l) {
   CharTrackerIgnoreNode *iter=NULL, *node;
   
   all = l->all;
   for(iter=l->list;iter;iter=iter->next) {
      node = new CharTrackerIgnoreNode;
      node->next = list;
      node->group = iter->group;
      list = node;
   }
}









CharTrackerGroup::CharTrackerGroup() {
   members = NULL;
   next = NULL;
}

CharTrackerGroup::~CharTrackerGroup() {
   CharTrackerNode *node;
   
   while(members) {
      node = members->next;
      delete members;
      members = node;
   }
}

CharTrackerNode *CharTrackerGroup::AddChar(char *buf) {
   CharTrackerNode *node;
   
   node = new CharTrackerNode;
   node->SetName(buf);
   
   node->next = members;
   members = node;
   
   return node;
}

CharTrackerNode *CharTrackerGroup::GetChar(char *buf) {
   CharTrackerNode *iter;
   
   for(iter=members;iter;iter=iter->next) {
      if(!strcasecmp(iter->name, buf))
	return iter;
   }
   
   return NULL;
}

void CharTrackerGroup::AddGroup(CharTrackerGroup *gr) {
   CharTrackerNode *node, *nmem;
   
   for(node=gr->members;node;node=node->next) {
      nmem = AddChar(node->name);
      nmem->SetLevel(node->level);
      nmem->SetObjs (node->num_objs);
   }
}

void CharTrackerGroup::Save(FILE *fl) {
   CharTrackerNode *iter;
   CharTrackerIgnoreNode *iter2;
   
   fprintf(fl, "---\n");
   
   for(iter=members;iter;iter=iter->next)
     fprintf(fl, "%s %ld %ld %ld %s\n",
	     iter->name, iter->level,
	     iter->num_objs, iter->laston,
	     iter->host);
   
   if(ignore.all)
     fprintf(fl, "**\n");
   
   for(iter2=ignore.list;iter2;iter2=iter2->next)
     fprintf(fl, "*%s\n", iter2->group->members->name);
}

long CharTrackerGroup::Count() {
   long c;
   CharTrackerNode *iter;
   
   for(c=0,iter=members;iter;iter=iter->next,c++);
   
   return c;
}









CharTracker::CharTracker() {
   groups = NULL;
}

CharTracker::~CharTracker() {
   CharTrackerGroup *node;
   
   while(groups) {
      node = groups->next;
      delete groups;
      groups = node;
   }
}

CharTrackerGroup *CharTracker::GetGroupChar(char *buf) {
   CharTrackerGroup *iter;
   
   for(iter=groups;iter;iter=iter->next) {
      if(iter->GetChar(buf))
	return iter;
   }
   
   return NULL;
}

CharTrackerGroup *CharTracker::NewGroupChar(char *buf) {
   CharTrackerGroup *node;
   
   node = new CharTrackerGroup;
   node->AddChar(buf);
   node->next = groups;
   groups = node;
   
   return node;
}

void CharTracker::Save() {
   CharTrackerGroup *iter;
   FILE *fl;
   
   if(!(fl = fopen("trackchar.tmp","w+"))) {
      log_msg("Couldn't open character tracking list.");
      return;
   }
   
   for(iter=groups;iter;iter=iter->next)
     iter->Save(fl);
   
   fprintf(fl, "$\n");
   fclose(fl);
   
   rename("trackchar.tmp", "trackchar.list");
}

void CharTracker::Load() {
   CharTrackerGroup *node=NULL;
   CharTrackerNode *ch;
   char buf[255],*ptr;
   FILE *fl;
   char nm[255], hs[512];
   long lvl,objs;
   time_t last=0;
   
   if(!(fl = fopen("trackchar.list","r"))) {
      log_msg("Couldn't open character tracking list.");
      return;
   }
   
   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
        break;

      if(*ptr == '-') {
	 node = new CharTrackerGroup;
	 node->next = groups;
	 groups = node;
	 continue;
      }
      if(*ptr == '*') {
	 continue;
      }
      if(*ptr == '$') {
	 break;
      }
      
      lvl=objs=0;
      strcpy(nm,"");
      strcpy(hs,"");
      sscanf(buf, "%s %ld %ld %ld %s", nm, &lvl, &objs, &last, hs);
      ch = groups->AddChar(nm);
      ch->SetObjs(objs);
      ch->SetLevel(lvl);
      ch->SetLaston(last);
      ch->SetHost(hs);
      
      if(!last || !hs[0]) {
	 dlog("*TRACKCHAR* %s has no info! Updating!", nm);
	 UpdateCharFull(ch);
      }
   }
   
   fseek(fl, SEEK_SET, 0);
   
   objs=0;
   while(!feof(fl)) {
      if(!(ptr = fgets(buf, sizeof(buf), fl)))
	break;
      
      if(*ptr == '-') {
	 objs=1;
	 continue;
      }
      
      if(*ptr == '$') break;
      
      sscanf(buf, "%s", nm);
      if(*ptr == '*') {
	 if(*(ptr+1) == '*') {
	    node->ignore.IgnoreAll();
	    continue;
	 }

	 sscanf(ptr+1, "%s", nm);
	 if(GetGroupChar(nm)) {
	    node->ignore.AddIgnore(GetGroupChar(nm));
	 } else {
	    dlog("Couldn't find a group for %s to ignore!!", nm);
	 }
	 continue;
      }
      
      if(objs) {
         node = GetGroupChar(nm);
	 objs=0;
      }
   }
   
   fclose(fl);
}

void CharTracker::DeleteCharConst(const char *buf) {
   char name[MAX_STRING_LENGTH];
   strcpy(name, buf);
   
   DeleteChar(name);
}

void CharTracker::DeleteChar(char *buf) {
   CharTrackerGroup *node,*prev,*iter;
   CharTrackerNode *cprev, *cnode,*citer;
   char nm[255];
   
   strcpy(nm, buf);
   
   node = GetGroupChar(buf);
   if(!node) {
      dlog("Error in CharTracker! "
	   "Couldn't find %s in any groups to delete!", nm);
      return;
   }
   
   cnode = node->GetChar(buf);
   cprev=NULL;
   for(citer=node->members;citer;citer=citer->next) {
      if(citer == cnode) break;
      cprev = citer;
   }
   
   //if cnode was members
   if(!cprev) {
      node->members = cnode->next;
   } else {
      cprev->next = cnode->next;
   }
   
   dlog("Deleting character from CharTracker list: %s", nm);
   delete cnode;
   
   if(node->members) {
      Save();
      return;
   }
   
   while(node->ignore.list) {
      dlog("Removing %s from %s's group ignore list", node->ignore.list->group->members->name, node->members->name);
      node->ignore.list->group->ignore.RemIgnore(node);
      node->ignore.RemIgnore(node->ignore.list->group);
   }
   
   prev=NULL;
   for(iter=groups;iter;iter=iter->next) {
      if(iter == node) break;
      prev = iter;
   }
   
   //if node was groups
   if(!prev) {
      groups = node->next;
   } else {
      prev->next = node->next;
   }
   
   dlog("Deleting character group from CharTracker list: %s", nm);
   delete node;
   
   Save();
}

void CharTracker::UpdateChar(char *buf, long lvl, long objs) {
   CharTrackerGroup *group=NULL;
   CharTrackerNode  *ch=NULL;
   
   group = GetGroupChar(buf);
   if(group) ch = group->GetChar(buf);
   
   if(!ch) {
      dlog("Couldn't find %s in the database!", buf);
      return;
   }
   
   ch->SetObjs(objs);
   ch->SetLevel(lvl);
   Save();
}

void CharTracker::UpdateCharLast(char *buf, time_t last) {
   CharTrackerGroup *group=NULL;
   CharTrackerNode  *ch=NULL;
   
   group = GetGroupChar(buf);
   if(group) ch = group->GetChar(buf);
   
   if(!ch) {
      dlog("Couldn't find %s in the database!", buf);
      return;
   }
   
   ch->SetLaston(last);
   Save();
}

void CharTracker::UpdateCharHost(char *buf, char *host) {
   CharTrackerGroup *group=NULL;
   CharTrackerNode  *ch=NULL;
   
   group = GetGroupChar(buf);
   if(group) ch = group->GetChar(buf);
   
   if(!ch) {
      dlog("Couldn't find %s in the database!", buf);
      return;
   }
   
   ch->SetHost(host);
   Save();
}





void CharTracker::CheckChar(char_data *ch) {
   char_data *match;
   char thishost[MAX_STRING_LENGTH];
   int found=0;
   
   if(!ch->desc->host) return;
   
   strcpy(thishost, ch->desc->host);
   
   EACH_CHARACTER(iter,match) {
      if((ch != match) && (match->desc && match->desc->host))
	if(!strcmp(match->desc->host, thishost)) {
	   DoCheck(GET_REAL_NAME(ch), GET_REAL_NAME(match));
	   found = 1;
	}
   } END_AITER(iter);
   
   if(found) {
      UpdateCharFull(ch);
      Save();
      return;
   }
   
   if(GetGroupChar(GET_REAL_NAME(ch))) {
      UpdateCharFull(ch);
      Save();
      return;
   }
   
   dlog("Adding Character %s to database", GET_REAL_NAME(ch));
   NewGroupChar(GET_REAL_NAME(ch));
   UpdateCharFull(ch);
   Save();
   return;
}

void CharTracker::DoCheck(char *ch, char *match) {
   CharTrackerGroup *group, *ogroup, *prev, *iter;
   group  = GetGroupChar(ch);
   ogroup = GetGroupChar(match);
   
   if(group == ogroup) {
      return;
   }
   
   if(group) {
      if(ogroup->ignore.IsIgnoring(group) ||
	 group->ignore.IsIgnoring(ogroup)) {
	 dlog("%s and %s's groups are ignoring each other.", ch, match);
	 return;
      }
      
      dlog("Merging %s's group and %s's group together.", ch, match);
      ogroup->AddGroup(group);
      
      dlog("Deleting %s's old group", ch);
      prev=NULL;
      for(iter=groups;iter;iter=iter->next) {
	 if(iter == group) break;
	 prev = iter;
      }
      
      if(!prev) {
	 groups = group->next;
      } else {
	 prev->next = group->next;
      }
      
      delete group;
      return;
   }
   
   dlog("Adding %s to %s's group", ch, match);
   ogroup->AddChar(ch);
}






void CharTracker::Show(char_data *ch, char *buf) {
   CharTrackerNode *node, inst, *next;
   CharTrackerGroup *group,*group2;
   CharTrackerIgnore inst2;
   CharTrackerIgnoreNode *iter, *iter2;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH], *ptr;
   long max, i=0;
   
   while(*buf == ' ') buf++;
   
   if(!*buf) {
      cprintf(ch, "Syntax: \n");
      cprintf(ch, "\tfind !disconnect <n1>        - forms a new group for n1.\n");
      cprintf(ch, "\tfind !move <n1> <n2>         - moves n1 to n2's group.\n");
      cprintf(ch, "\tfind !ignore <n1> <n2 | all> - makes n1 ignore n2 or everyone.\n");
      cprintf(ch, "\tfind !update <n1 | all>      - updates n1 or everyone's info (LAG).\n");
      cprintf(ch, "\tfind <name>                  - finds name's group.\n");
      cprintf(ch, "\tfind all                     - finds all people in the database.\n");
      cprintf(ch, "\tfind <number>                - finds all groups with > number people.\n");
      return;
   }
   
   buf=one_argument(buf,arg1);
   
   if(!strcasecmp(arg1, "all")) {
      for(group=groups;group;group=group->next)
	ShowSingle(ch, group,i++);
   } else if(isdigit(*arg1)) {
      max = 0;
      if(sscanf(arg1, "%ld", &max) == 1) {
	 for(group=groups;group;group=group->next)
	   if(group->Count() >= max)
	     ShowSingle(ch, group, i++);
      }
   } else if(is_abbrev(arg1, "!disconnect")) {
      buf=one_argument(buf,arg2);
      group = GetGroupChar(arg2);
      if(!group) {
	 cprintf(ch, "Couldn't find %s's group\n", arg2);
	 return;
      }
      if(group->Count() == 1) {
	 inst2.CopyList(&group->ignore);
	 i=1;
      } else {
	 i=0;
      }
      node = group->GetChar(arg2);
      inst = *node;
      DeleteChar(inst.name);
      
      dlog("Creating a new group for %s", inst.name);
      group2 = NewGroupChar(inst.name);
      
      if(i) {
         dlog("Switching ignores from old group");
         group2->ignore.CopyList(&inst2);
         for(iter=group2->ignore.list;iter;iter=iter->next) {
	    for(iter2=iter->group->ignore.list;iter2;iter2=iter2->next) {
	       if(iter2->group == group) {
		  dlog("Switching %s's ignore group over", iter->group->members->name);
	          iter2->group = group2;
	          break;
	       }
	    }
	 }
      }
      UpdateChar(arg2,inst.level,inst.num_objs);
      UpdateCharLast(arg2,inst.laston);
      UpdateCharHost(arg2,inst.host);
      Save();
   } else if(is_abbrev(arg1, "!move")) {
      buf=one_argument(buf,arg2);
      buf=one_argument(buf,arg3);
      
      group=GetGroupChar(arg2);
      group2=GetGroupChar(arg3);
      
      if(!group || !group2) {
	 cprintf(ch, "Couldn't find either %s, or %s's group\n", arg2, arg3);
	 return;
      }
      if(group == group2) {
	 cprintf(ch, "They're in the same group!\n");
	 return;
      }
      i=0;
      if(group->Count() == 1) {
	 inst2.CopyList(&group->ignore);
	 i=1;
      } else {
	 i=0;
      }
      
      node = group->GetChar(arg2);
      inst = *node;
      DeleteChar(inst.name);
      
      dlog("Moving %s over to %s's group", inst.name, arg3);
      group2->AddChar(inst.name);
      
      if(i) {
	 dlog("Switching ignores from old group");
	 group2->ignore.CopyList(&inst2);
	 for(iter=group2->ignore.list;iter;iter=iter->next) {
	    for(iter2=iter->group->ignore.list;iter2;iter2=iter2->next) {
	       if(iter2->group == group) {
		  dlog("Switching %s's ignore group over", iter->group->members->name);
		  iter2->group = group2;
		  break;
	       }
	    }
	 }
      }
      UpdateChar(arg2,inst.level,inst.num_objs);
      UpdateCharLast(arg2,inst.laston);
      UpdateCharHost(arg2,inst.host);
      Save();
   } else if(is_abbrev(arg1, "!ignore")) {
      buf=one_argument(buf,arg2);
      buf=one_argument(buf,arg3);
      
      group=GetGroupChar(arg2);
      group2=GetGroupChar(arg3);
      
      if(!strcasecmp(arg3, "all")) {
	 if(group->ignore.all) {
	    group->ignore.all = FALSE;
	    cprintf(ch, "%s's group is no longer ignoring everyone\n", arg2);
	 } else {
	    group->ignore.IgnoreAll();
	    cprintf(ch, "%s's group is now ignoring everyone\n", arg2);
	 }
	 Save();
	 return;
      }
      
      if(!group || !group2) {
	 cprintf(ch, "Couldn't find either %s, or %s's group\n", arg2, arg3);
	 return;
      }
      
      if(group->ignore.IsIgnoring(group2)) {
	 i = group->ignore.all;
	 group->ignore.all = FALSE;
	 if(group->ignore.IsIgnoring(group2)) {
	    group->ignore.RemIgnore(group2);
	    group2->ignore.RemIgnore(group);
	    cprintf(ch, "%s and %s's groups are no longer ignoring each other.\n", arg2, arg3);
	    group->ignore.all = i;
	    Save();
	    return;
	 } else {
	    group->ignore.all = i;
	 }
      }
      
      group->ignore.AddIgnore(group2);
      group2->ignore.AddIgnore(group);
      cprintf(ch, "%s and %s's groups are ignoring each other now.\n", arg2, arg3);
      Save();
   } else if(is_abbrev(arg1, "!update")) {
      buf = one_argument(buf, arg2);
      
      if(!strcmp(arg2, "")) {
	 cprintf(ch, "Need to supply a character to update, or use 'all'\n");
	 return;
      }
      
      if(!strcasecmp(arg2, "all")) {
	 for(group=groups;group;group=group2) {
	    group2 = group->next;
	    for(node=group->members;node;node=next) {
	       next = node->next;
	       UpdateCharFull(node);
	    }
	 }
	 cprintf(ch, "All characters in the database have been updated\n");
	 Save();
	 return;
      }
      group = GetGroupChar(arg2);
      if(!group) {
	 cprintf(ch, "Couldn't find a group for %s.\n", arg2);
	 return;
      }
      node = group->GetChar(arg2);
      
      if(UpdateCharFull(node)) {
         cprintf(ch, "%s's database information has been updated\n", node->name);
      }
      Save();
   } else {
      group = GetGroupChar(arg1);

      if(!group) {
	 cprintf(ch, "Couldn't find %s's group\n", arg1);
	 return;
      }
      ShowSingle(ch, group,0);
   }
}

int CharTracker::UpdateCharFull(char_data *ch) {
   CharTrackerGroup *group;
   CharTrackerNode  *node=NULL;
   obj_cost cost;
   
   group = GetGroupChar(GET_REAL_NAME(ch));
   if(group) node=group->GetChar(GET_REAL_NAME(ch));
   
   if(!node || !group) return 0;
   
   OfferChar(ch, &cost, AUTO_RENT, FALSE);
   node->SetObjs(cost.no_carried);
   node->SetLevel(GetMaxLevel(ch));
   node->SetLaston(ch->player.time.logon);
   node->SetHost(ch->specials.hostname);
   
   Save();
   return 1;
}

int CharTracker::UpdateCharFull(CharTrackerNode *ch) {
   char_data *vict;
   obj_cost cost;
   
   if(!(vict = LoadChar(NULL, ch->name, READ_PLAYER | READ_OBJECTS))) {
      DeleteChar(ch->name);
      return 0;
   }
   
   UpdateCharFull(vict);
   
   vict->in_room = -1;
   extract_char(vict);
   return 1;
}

void CharTracker::ShowSingle(char_data *ch, CharTrackerGroup *node, long i) {
   CharTrackerNode *iter;
   CharTrackerIgnoreNode *iter2;
   char buf[500];
   
   if(!i) {
      cprintf(ch, "%-15s %-5s %-5s %-25s %-20s\n",
	      "Name", "Level", "Objs", "Laston", "Host");
      cprintf(ch, "-----------------------------------------------------------");
   }
   
   cprintf(ch, "---\n");
   for(iter=node->members;iter;iter=iter->next) {
      if(iter->laston > 0) {
         strcpy(buf, ctime(&iter->laston));
         buf[strlen(buf)-1] = 0;
      }
      cprintf(ch, "%-15s %-5ld %-5ld %-25s %-20s\n",
	      iter->name, iter->level, iter->num_objs,
	      find_player_in_world(iter->name)?"-connected-":
	      (iter->laston > 0)?buf:"-unknown-",
	      iter->host);
   }
   
   if(node->ignore.all) {
      cprintf(ch, "\nGroup is ignoring everyone.\n");
      return;
   }
   
   for(iter2=node->ignore.list,i=0;iter2;iter2=iter2->next,i++) {
      if(!i) {
	 cprintf(ch, "\nGroup is ignoring:\n");
      }
      cprintf(ch, "  -%s's group\n", iter2->group->members->name);
   }
}
