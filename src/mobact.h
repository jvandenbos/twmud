#ifndef MOBACT_H
#define MOBACT_H

void mobile_activity(struct char_data* ch);
int SameRace(struct char_data* ch1, struct char_data* ch2);
int mobile_wander(struct char_data* ch);
void FindABetterWeapon(struct char_data* mob);
int MobHunt(struct char_data* ch);

#endif
