#ifndef CHANNELS_H
#define CHANNELS_H

#define COM_THINK	0x001
#define COM_IMP		0x002
#define COM_SHOUT	0x004
#define COM_AUCTION	0x008
#define COM_RUMOR	0x010
#define COM_LOG		0x020
#define COM_GUILD	0x040
#define COM_HOLLER	0x080
#define COM_QUEST	0x100
#define COM_SWEAR	0x200
#define COM_TRIVIA	0x400
#define COM_RPRUMOR	0x800
#define COM_BRUJAH	0x1600
#define COM_NEWBIE	0x3200
#define COM_VOTE	0x6400

#define COM_DEAF_MASK (COM_SHOUT|COM_AUCTION|COM_RUMOR|COM_GUILD|COM_HOLLER|COM_QUEST|COM_SWEAR|COM_TRIVIA|COM_RPRUMOR)

void do_comm(struct char_data* ch, char* argument, int cmd);
void do_channels(struct char_data* ch, char* argument, int cmd);
void do_deafen(struct char_data* ch, char* argument, int cmd);

#endif
