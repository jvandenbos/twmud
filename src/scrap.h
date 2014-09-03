/*************************************************************************
 * File: scrap.h                                                         *
 *                                                                       *
 * Usage: Prototypes for scraping functions used elsewhere in the mud    *
 *************************************************************************/


/* functions used elsewhere in the mud */

int DamageOneItem(struct char_data* ch, int dam_type, struct obj_data* obj);
void DamageStuff(struct char_data* v, int type, int dam);
void MakeScrap(struct char_data* ch, struct obj_data* obj, int dam_type);
