#ifndef LANGUAGE_H
#define LANGUAGE_H

#define TONGUE_COMMON  0
#define TONGUE_ELF     1
#define TONGUE_DROW    2
#define TONGUE_DWARF   3
#define TONGUE_PIXIE   4
#define TONGUE_ORC     5
#define TONGUE_GIANT   6
#define TONGUE_GNOME   7
#define TONGUE_HOBBIT  8
#define TONGUE_LYCANTH 9

struct tongue_info_struct {
   int race;
   int languages[20];
};

#endif
