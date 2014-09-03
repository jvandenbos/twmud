
#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#include "list.h"
#include "structs.h"
#include "utils.h"

/* forward declarations */
static void iter_upd_ptrs(list_iterator* iter, list_element* elem);
static void iter_update(list_iterator* iter, list_element* elem);

void list_init(list_head* list, list_sort_func sorter)
{
    list->first = 0;
    list->last = 0;
    list->sorter = sorter;
    list->iters = 0;
}

void list_insert(list_head* list, list_element* element)
{
    list_element*	before;
    
    if(!list->first || !list->sorter)
    {
	list_prepend(list, element);
	return;
    }

    for(before = list->first ;
	before && (list->sorter(before, element) > 0) ;
	before = before->next)
	;

    if(!before)
    {
	list_append(list, element);
	return;
    }

    element->next = before;
    element->prev = before->prev;

    before->prev = element;

    if(element->prev)
	element->prev->next = element;
    else
	list->first = element;

    element->list = list;
}

void list_prepend(list_head* list, list_element* element)
{
    element->prev = 0;
    element->next = list->first;
    if(list->first)
	list->first->prev = element;
    list->first = element;
    if(!list->last)
	list->last = element;

    element->list = list;
}

void list_append(list_head* list, list_element* element)
{
    element->next = 0;
    element->prev = list->last;
    if(list->last)
	list->last->next = element;
    list->last = element;
    if(!list->first)
	list->first = element;

    element->list = list;
}

void list_delete(list_head* list, list_element* element)
{
    list_iterator* iter;
    
    if(element->list != list)
	return;

    if(list->iters)
	for(iter = (list_iterator*) list->iters->first ; iter ;
	    iter = (list_iterator*) iter->link.next)
	    iter_update(iter, element);

    if(element->prev)
	element->prev->next = element->next;
    else
	list->first = element->next;

    if(element->next)
	element->next->prev = element->prev;
    else
	list->last = element->prev;

    element->next = 0;
    element->prev = 0;
    element->list = 0;
}

list_element* list_find(list_head* list, list_find_func func, void* extra)
{
    list_element* elem;
    list_element* next;
    int	compare;
    
    for(elem = list->first ; elem ; elem = next)
    {
	next = elem->next;
	
	if((compare = func(elem, extra)) == 0)
	    return elem;
	if(compare < 0)
	    break;
    }

    return 0;
}

void list_push(list_head* list, list_element* element)
{
    list_prepend(list, element);
}

list_element* list_pop(list_head* list)
{
    list_element* elem = list->first;
    
    if(elem)
	list_delete(list, elem);

    return elem;
}


list_iterator* iter_new(list_head* list)
{
    list_iterator*	iter;

    CREATE(iter, list_iterator, 1);

    if(!list->iters)
    {
	CREATE(list->iters, list_head, 1);
	list_init(list->iters, 0);
    }
    
    list_push(list->iters, &iter->link);
    iter->list = list;
    
    return iter;
}

void iter_kill(list_iterator* iter)
{
    list_delete(iter->list->iters, &iter->link);

    FREE(iter);
}

list_element* iter_first(list_iterator* iter)
{
    list_element*	elem;

    if((elem = iter->list->first))
	iter_upd_ptrs(iter, elem);
    
    return elem;
}

list_element* iter_last(list_iterator* iter)
{
    list_element*	elem;

    if((elem = iter->list->last))
	iter_upd_ptrs(iter, elem);

    return elem;
}

list_element* iter_next(list_iterator* iter)
{
    list_element* elem;

    if((elem = iter->next))
	iter_upd_ptrs(iter, elem);
    
    return elem;
}

list_element* iter_prev(list_iterator* iter)
{
    list_element* elem;
    
    if((elem = iter->prev))
	iter_upd_ptrs(iter, elem);
    
    return elem;
}

static void iter_update(list_iterator* iter, list_element* elem)
{
    if(elem == iter->next)
	iter->next = elem->next;
    else if(elem == iter->prev)
	iter->prev = elem->prev;
}

static void iter_upd_ptrs(list_iterator* iter, list_element* elem)
{
    iter->prev = elem->prev;
    iter->next = elem->next;
}
