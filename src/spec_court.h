void remove_killer(char *name);
void set_killer(struct char_data *ch);
void reproachless_msg(struct char_data *ch, struct char_data *target);
void innocent_msg(struct char_data *ch, struct char_data *target);
void guilty_msg(struct char_data *ch, struct char_data *target);
void convicted_msg(struct char_data *ch, struct char_data *target);
void remove_witness(char *name);
int judge(struct char_data *ch, int cmd, char *arg);
