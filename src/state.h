extern char last_known_state[MAX_STRING_LENGTH];
#define STATE_NAME(p)	(((p)->character && (p)->character->player.name) \
			 ? GET_IDENT(p->character) : "unnamed")
#define STATE0(s)		strcpy(last_known_state, s)
#define STATE1(s, p)		sprintf(last_known_state, s, p)
#define STATE2(s,p1,p2)		sprintf(last_known_state, s, p1, p2)
#define STATE3(s,p1,p2,p3)	sprintf(last_known_state, s, p1, p2,p3)
