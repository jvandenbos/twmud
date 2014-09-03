#ifndef HANDLER_H
#define HANDLER_H

/* handling the affected-structures */
void AddAffects(struct char_data* ch, struct obj_data* o);

void affect_modify(struct char_data *ch, byte loc, long mod, long bitv, bool add);
void affect_to_char( struct char_data *ch, struct affected_type *af );
void affect_remove( struct char_data *ch, struct affected_type *af );
void affect_from_char( struct char_data *ch, short skill);
bool affected_by_spell( struct char_data *ch, short skill );
void affect_join( struct char_data *ch, struct affected_type *af,
                  bool avg_dur, bool avg_mod );
void affect_remove_all(struct char_data* ch);
int affect_event(struct affected_type* aff, int now);

/* ******** objects *********** */

void equip_char(struct char_data *ch, struct obj_data *obj, int pos);

void update_char_objects(struct char_data* ch);

void extract_char(struct char_data *ch);

int GiveMinStrToWield(struct obj_data* obj, struct char_data*ch);
int apply_ac(struct char_data *ch, int eq_pos);

int can_inscribe(struct obj_data* book);
void inscribe_book(struct obj_data* book, int spellnum, int quality);

#endif
