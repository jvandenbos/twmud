#ifndef BOARD_H
#define BOARD_H

#define MAX_MSGS 70	               /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */
  
struct Board {
    char *msgs[MAX_MSGS];
    char *head[MAX_MSGS];
    int msg_num;
    char filename[40];
    FILE *file;			/* file that is opened */
    int Rnum;			/* Real # of object that this board hooks to */
    char loaded;
    int min_read;
    int min_write;
    int min_remove;
    int guild;
    struct char_data* writer;
    struct Board *next;
};

void InitBoards(void);
SPECIAL(board);

struct Board* InitABoard(struct obj_data* obj);
void board_save_board(struct Board* board);
struct Board* FindBoardInRoom(int room);

#endif
