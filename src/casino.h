#ifndef CASINO_H
#define CASINO_H

int check_blackjack(struct char_data* ch);
int do_blackjack_exit(struct char_data* ch);
int do_blackjack_enter(struct char_data* ch);
void do_bj_hit(struct char_data* ch, const char* arg, int cmd);

#endif

    
