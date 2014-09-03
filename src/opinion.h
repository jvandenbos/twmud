#ifndef OPINION_H
#define OPINION_H

int RemHated( struct char_data *ch, struct char_data *pud); 
int AddHated( struct char_data *ch, struct char_data *pud); 
void AddHatred( struct char_data *ch, int parm_type, int parm);
void RemHatred( struct char_data *ch, unsigned short bitv);
void FreeHates( struct char_data *ch);
void ZeroHatred(struct char_data* ch, struct char_data* v);
int Hates( struct char_data *ch, struct char_data *v);
struct char_data *FindAHatee( struct char_data *ch);
void DeleteHatreds(struct char_data* ch);

int RemFeared( struct char_data *ch, struct char_data *pud);
void AddFears( struct char_data *ch, int parm_type, int parm);
int AddFeared( struct char_data *ch, struct char_data *pud);
int RemFears( struct char_data *ch, unsigned short bitv);
void ZeroFeared(struct char_data* ch, struct char_data* v);
void FreeFears( struct char_data *ch);
int Fears( struct char_data *ch, struct char_data *v);
struct char_data *FindAFearee( struct char_data *ch);
void DeleteFears(struct char_data* ch);

#endif
