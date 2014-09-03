
#ifndef __LIST__
#define __LIST__

typedef struct list_element	list_element;
typedef struct list_head	list_head;
typedef struct list_iterator	list_iterator;

typedef int (*list_sort_func)(list_element* a, list_element* b);
typedef int (*list_find_func)(list_element* elem, void* extra);

struct list_element
{
    list_element*	next;
    list_element*	prev;
    list_head*		list;
};

struct list_head
{
    list_element*	first;
    list_element*	last;
    list_sort_func	sorter;
    list_head*		iters;
};

struct list_iterator
{
    list_element	link;
    list_head*		list;
    list_element*	next;
    list_element*	prev;
};

extern void 		list_init(list_head* list, list_sort_func sorter);
extern void 		list_insert(list_head* list, list_element* element);
extern void 		list_prepend(list_head* list, list_element* element);
extern void 		list_append(list_head* list, list_element* element);
extern void 		list_delete(list_head* list, list_element* element);
extern list_element*	list_find(list_head* list, list_find_func func,
				  void* extra);
extern void		list_push(list_head* list, list_element* element);
extern list_element*	list_pop(list_head* list);

extern list_iterator*	iter_new(list_head* list);
extern void		iter_kill(list_iterator* iter);
extern list_element*	iter_first(list_iterator* iter);
extern list_element*	iter_last(list_iterator* iter);
extern list_element*	iter_next(list_iterator* iter);
extern list_element*	iter_prev(list_iterator* iter);

#define START_ITER(iter, var, type, list) \
{ \
    list_iterator* iter = iter_new(list); \
    for(var = (type) iter_first(iter) ; var ; var = (type) iter_next(iter))

#define END_ITER(iter) \
    iter_kill(iter); \
}

#endif
