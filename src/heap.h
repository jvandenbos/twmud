#ifndef HEAP_H
#define HEAP_H

/*
**  Heap data structs
*/

struct StrHeapList {
   char *string;   /* the matching string */
   int  total;     /* total # of occurences */
};

struct StrHeap {
   int uniq;   /* number of uniq items in list */
   struct StrHeapList *str;   /* the list of strings and totals */
};

struct StrHeap* InitHeap(void);
void StringHeap(const char* string, struct StrHeap* heap);
void DisplayStringHeap(struct StrHeap* Heap, struct char_data* ch,
		       int type, int destroy);

#endif
