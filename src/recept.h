#ifndef RECEPT_H
#define RECEPT_H

#ifndef DB_H
#include "db.h"
#endif

void DelChar(const char* name, const byte trust);
struct char_data* LoadChar(struct char_data* ch, const char* name, int parts);
int SaveChar(struct char_data* ch, int in_room, int get_offer);
int OfferChar(struct char_data* ch, struct obj_cost*, int in_room, int noisy);
int ChargeRent(struct char_data* ch);
void drop_unrented(struct char_data* ch);

#endif
