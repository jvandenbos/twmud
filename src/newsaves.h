
#ifndef NEWSAVES_H
#define NEWSAVES_H

/* positive extraMods makes the save easier, decreasing
   the chance of successful skill usage */
int NewSkillSave(struct char_data* ch, struct char_data* vic,
		 int skillNo, int extraMods, long immunes);

#endif
