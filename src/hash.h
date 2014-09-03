
#ifndef HASH_H
#define HASH_H

#include "list.h"

typedef struct hash_link
{
  list_element	elem;
  int		key;
  void		*data;
} hash_link;

typedef struct hash_header {
  int		rec_size;
  int		table_size;
  list_head*	buckets;
} hash_header;

void init_hash_table(struct hash_header* ht, int rec_size, int table_size);

void *hash_find(struct hash_header *ht, int key);
int hash_enter(struct hash_header *ht, int key, void *data);
void *hash_find_or_create(struct hash_header *ht, int key);
void *hash_remove(struct hash_header *ht, int key);

typedef void (*destroy_hash_func)(void* data);
void destroy_hash_table(struct hash_header *ht, destroy_hash_func gman);

typedef void (*hash_iterate_func)(int key, void* data, void* cdata);
void hash_iterate(hash_header* ht, hash_iterate_func func, void* cdata);

#endif
