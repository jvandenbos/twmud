#ifndef MULTICLASS_H
#define MULTICLASS_H

int GetClassLevel(struct char_data *ch, int clss);
int CountBits(int num);
int OnlyClass( struct char_data *ch, int clss);
int HasClass(struct char_data *ch, int clss);
int HowManyClasses(struct char_data *ch);
int BestClass(struct char_data* ch);
int BestFightingClass(struct char_data *ch);
int BestThiefClass(struct char_data *ch);
int BestMagicClass(struct char_data *ch);
int GetMaxLevel(struct char_data *ch);
int GetSecMaxLev(struct char_data* ch);
int GetThirdMaxLev(struct char_data* ch);

int GetTotLevel(struct char_data *ch);
void StartLevels(struct char_data *ch);
int GetMaxLevel(struct char_data* ch);
int UpdateMaxLevel(struct char_data *ch);
int GetMinLevel(struct char_data* ch);
int UpdateMinLevel(struct char_data *ch);
int GetAvgLevel(struct char_data* ch);

#endif
