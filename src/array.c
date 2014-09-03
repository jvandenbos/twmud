
#include "config.h"

#include "array.h"
#include "structs.h"
#include "utils.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif

static void make_room(array_t* array, int size);
static void fix_ends(array_t* array);

array_t* array_init(array_t* array, int size, int chunk)
{
    if(!array)
    {
	CREATE(array, array_t, 1);
	array->dispose = 1;
    }
    else
	array->dispose = 0;
    
    array->size = size;
    array->chunk = chunk;
    CREATE(array->elements, void*, size);
    array->first = array->last = 0;
    array->iters = 0;
    array->dirty = 0;

    return array;
}

void array_kill(array_t* array)
{
    FREE(array->elements);
    if(array->dispose)
	FREE(array);
}

void array_insert(array_t* array, void* element)
{
    make_room(array, array->last);
    array->elements[array->last++] = element;
}

void array_delete(array_t* array, void* element)
{
    int i;
    void** ptr;
    
    for(i = array->first, ptr = array->elements ; i < array->last ; ++i, ++ptr)
	if(*ptr == element)
	{
	    *ptr = 0;
	    array->dirty = 1;
	}
}

int array_find(array_t* array, array_find_func func, void* extra)
{
    int i;
    void**	elements = array->elements;
   
    for(i = array->first ; i < array->last ; ++i)
    {
	if(elements[i] && !func(i, elements[i], extra))
	    return i;
    }

    return -1;
}

void array_collapse(array_t* array)
{
    int		src, dst;
    void**	elements = array->elements;
    
    if(!array->dirty)
	return;
    
    for(dst = 0, src = array->first ; src < array->last ; src++)
	if(elements[src])
	    elements[dst++] = elements[src];

    array->first = 0;
    array->last = dst;
    array->dirty = 0;
}


array_iterator*	aiter_new(array_t* array)
{
    array_iterator* iter;
    
    fix_ends(array);

    CREATE(iter, array_iterator, 1);

    iter->array = array;
    iter->current = array->first;

    array->iters++;
    
    return iter;
}

void aiter_kill(array_iterator* iter)
{
    if(--iter->array->iters <= 0)
    {
	iter->array->iters = 0;
	array_collapse(iter->array);
    }
    
    FREE(iter);
}

void* aiter_first(array_iterator* iter)
{
    fix_ends(iter->array);

    iter->current = iter->array->first - 1;

    return aiter_next(iter);
}

void* aiter_last(array_iterator* iter)
{
    fix_ends(iter->array);

    iter->current = iter->array->last;

    return aiter_prev(iter);
}

void* aiter_next(array_iterator* iter)
{
    array_t*	array = iter->array;
    void**	elements = array->elements;
    
    while(++iter->current < array->last)
    {
	if(elements[iter->current])
	    return elements[iter->current];
    }

    return NULL;
}

void* aiter_prev(array_iterator* iter)
{
    array_t*	array = iter->array;
    void**	elements = array->elements;
    
    while(--iter->current >= array->first)
    {
	if(elements[iter->current])
	    return elements[iter->current];
    }

    return NULL;
}


static void make_room(array_t* array, int size)
{
    int		ns;
    
    for(ns = array->size ; ns <= size ; ns += array->chunk)
	;
    
    if(ns != array->size)
    {
	RECREATE(array->elements, void*, ns);
	array->size = ns;
    }
}

    
static void fix_ends(array_t* array)
{
    void** elements = array->elements;
    
    while((array->first < array->last) && !elements[array->first])
	array->first++;

    while((array->last > array->first) && !elements[array->last - 1])
	array->last--;
}
