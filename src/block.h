#ifndef BLOCK_H
#define BLOCK_H

typedef int (*block_proc)(struct char_data *c, struct char_data *v);

struct block_data {
  int mob_vnum;          /* the vnum of the blocking mob */
  room_num room;              /* the room where the mob does his blocking */
  int blocked_direction; /* the direction the mob is blocknig */
  block_proc b_proc;     /* the blocking proc that says who can pass and who can't */
  spec_proc_func sp_proc;     /* the spec proc the mob does when not blocking */
};

#endif

