
#ifndef __ARRAY__
#define __ARRAY__

typedef struct array  		array_t;
typedef struct array_iterator	array_iterator;

typedef int (*array_find_func)(int index, void* element, void* extra);

struct array
{
    int			size, chunk;
    int			first, last;
    int			iters;
    int			dispose : 1;
    int			dirty : 1;
    void**		elements;
};

struct array_iterator
{
    array_t*		array;
    int			current;
};

extern array_t*		array_init(array_t* array, int size, int chunk);
extern void		array_insert(array_t* array, void* element);
extern void		array_delete(array_t* array, void* element);
extern int		array_find(array_t* array, array_find_func func,
				   void* extra);
extern void		array_collapse(array_t* array);

extern array_iterator*	aiter_new(array_t* array);
extern void		aiter_kill(array_iterator*);
extern void*		aiter_first(array_iterator*);
extern void*		aiter_last(array_iterator*);
extern void*		aiter_next(array_iterator*);
extern void*		aiter_prev(array_iterator*);

#define AITER_F(iter, var, type, arr) \
{ \
    array_iterator* iter = aiter_new(arr); \
    for(var = (type) aiter_first(iter) ; var ; var = (type) aiter_next(iter))

#define AITER_B(iter, var, type, arr) \
{ \
    array_iterator* iter = aiter_new(arr); \
    for(var = (type) aiter_last(iter) ; var ; var = (type) aiter_prev(iter))

#define END_AITER(iter) \
    aiter_kill(iter); \
}

#endif
