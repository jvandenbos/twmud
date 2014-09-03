
#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>

#include "structs.h"
#include "hash.h"
#include "utils.h"

static hash_link* _hash_find(hash_header* ht, int key);
static void _hash_enter(hash_header* ht, int key, void* data);
static void _hash_remove(hash_header* ht, int key);

static int key_sorter(hash_link* a, hash_link* b)
{
  return b->key - a->key;
}

static int key_finder(hash_link* a, int key)
{
  return key - a->key;
}

static int hash(hash_header* ht, int key)
{
  return ((unsigned long) key * 17) % ht->table_size;
}

void init_hash_table(hash_header *ht, int rec_size, int table_size)
{
  int 		i;
  list_head*	list;
  
  ht->table_size = table_size;
  CREATE(ht->buckets, list_head, table_size);

  for(i = 0, list = ht->buckets ; i < table_size ; ++i, ++list)
    list_init(list, (list_sort_func) key_sorter);
}

void destroy_hash_table(hash_header* ht, destroy_hash_func dhf)
{
  list_head*	list;
  int		i;
  
  for(i = 0, list = ht->buckets ; i < ht->table_size ; ++i, ++list)
  {
    list_element*	elem;

    while((elem = list->first))
    {
      dhf(((hash_link*) elem)->data);
      list_delete(list, elem);
      FREE(elem);
    }
  }

  FREE(ht->buckets);
}

void* hash_find(hash_header* ht, int key)
{
  hash_link*	item;

  item = _hash_find(ht, key);
  return item ? item->data : NULL;
}

int hash_enter(hash_header* ht, int key, void* data)
{
  hash_link* item;
  
  if((item = _hash_find(ht, key)))
    return 0;

  _hash_enter(ht, key, data);

  return 1;
}

void* hash_find_or_create(hash_header* ht, int key)
{
  hash_link*	item;
  void*		rval;
  
  if((item = _hash_find(ht, key)))
    return item->data;

  CREATE(rval, char, ht->rec_size);
  
  _hash_enter(ht, key, rval);

  return rval;
}

void* hash_remove(struct hash_header* ht, int key)
{
  hash_link*	item;
  void*		data = NULL;
  
  if((item = _hash_find(ht, key)))
  {
    data = item->data;
    _hash_remove(ht, key);
  }

  return data;
}

void hash_iterate(hash_header* ht, hash_iterate_func func, void* cdata)
{
  int		i;
  list_head*	list;

  for(i = 0, list = ht->buckets ; i < ht->table_size ; ++i, ++list)
  {
    hash_link*	link;
    
    START_ITER(iter, link, hash_link*, list)
    {
      func(link->key, link->data, cdata);
    }
    END_ITER(iter);
  }
}

hash_link* _hash_find(hash_header* ht, int key)
{
  return (hash_link*) list_find(&ht->buckets[hash(ht, key)],
				(list_find_func) key_finder,
				(void*) key);
}

void _hash_enter(hash_header* ht, int key, void* data)
{
  hash_link*	link;

  CREATE(link, hash_link, 1);
  link->key = key;
  link->data = data;

  list_insert(&ht->buckets[hash(ht, key)], &link->elem);
}

void _hash_remove(hash_header* ht, int key)
{
  hash_link*	link;
  
  if((link = _hash_find(ht, key)))
    list_delete(&ht->buckets[hash(ht, key)], &link->elem);
}  
  
