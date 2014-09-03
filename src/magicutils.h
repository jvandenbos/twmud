#ifndef MAGICUTILS_H
#define MAGICUTILS_H

void SwitchStuff(struct char_data* giver, struct char_data* taker);
void FailCharm(struct char_data* victim, struct char_data* ch);
void FailSleep(struct char_data* victim, struct char_data* ch);
void FailPara(struct char_data* victim, struct char_data* ch);
void FailCalm(struct char_data* victim, struct char_data* ch);
void RawSummon(struct char_data* v, struct char_data* ch);
int is_hold_instrument(struct char_data *ch, int spell_number);

/* magic.c */
void MakeAffect(struct char_data* caster, struct char_data* target,
		int spell_type,
		int aff_type, int modifier, int location, int bitvector,
		int duration, int mana_cost,
		bool add_affect, bool avg_dur, bool avg_mod,
                expire_proc proc);
void MakeCharmed(struct char_data* caster, struct char_data* victim,
		 int level, int spell_type, int bonus);
void poison_effect(ubyte level, struct char_data* ch, struct char_data* victim);
#endif
