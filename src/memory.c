#include "config.h"

#include <memory.h>
#include <stdio.h>
#ifndef DONT_USE_malloc_h
/* #include <malloc.h> */
#endif

#include "structs.h"
#include "memory.h"
#include "utils.h"
#include "utility.h"

#if defined(MEM_DEBUG) && (MEM_DEBUG > 0)

void* debug_calloc(size_t size, int count, const char* file, int line);
void debug_free(void* ptr, const char* file, int line);
  
#define TAIL_MAGIC_CNT	3
#if MEM_DEBUG > 1
#define HEAD_MAGIC_CNT	(TAIL_MAGIC_CNT - 6)
void dump_memory_usage(void);
#define ABORT dump_memory_usage
#else
#define ABORT abort
#define HEAD_MAGIC_CNT	(TAIL_MAGIC_CNT - 2)
#endif
#if HEAD_MAGIC_CNT < 2
#undef HEAD_MAGIC_CNT
#define HEAD_MAGIC_CNT 2
#endif

#define HEAD_MAGIC	0xBEADDEAB
#define TAIL_MAGIC	0xDBACCABD
#define FREED_MAGIC	0xFDEDDEDF
#define ALLOC_MAGIC     0x55555555

typedef struct
{
    unsigned long magic[TAIL_MAGIC_CNT];
    struct mem_head *head;
} mem_tail;

typedef struct mem_head
{
    unsigned long magic[HEAD_MAGIC_CNT];
#if MEM_DEBUG > 1
    const char* file;
    unsigned long line;
#endif
    mem_tail* tail;
    unsigned long checksum;
#if MEM_DEBUG > 1
    struct mem_head* next;
    struct mem_head* prev;
#endif
} mem_head;

#if MEM_DEBUG > 1
mem_head* alloc_head = NULL;
static void alloc_insert(mem_head* head, const char* file, int line);
static void alloc_delete(mem_head* head);
#endif

static void set_magic(unsigned long* addr, int count, unsigned long magic)
{
    while(count-- > 0)
    {
	*addr++ = magic;
	magic <<= 3; 
    }
}

static int check_magic(unsigned long* addr, int count, unsigned long magic)
{
    while(count-- > 0)
    {
	if(*addr++ != magic)
	    return 1;
	magic <<= 3; 
    }

    return 0;
}

static unsigned long checksum(unsigned long* start, unsigned long* end)
{
    unsigned long sum;
    
    for(sum = 0 ; start < end ; start++)
	sum = (sum << 7) ^ *start;
    return sum;
}

static void check_head(mem_head* head, const char* when,
		      const char* file, int line)
{
    if(!check_magic(head->magic, HEAD_MAGIC_CNT, FREED_MAGIC))
    {
	fprintf(stderr, "File: \"%s\"  Line: %d: %s: freed block\n",
		file, line, when);
	ABORT();
    }
    else if(head->checksum != checksum(head->magic, &head->checksum))
    {
	fprintf(stderr, "File: \"%s\"  Line: %d: %s: bad header checksum\n\t%ld != %ld\n",
		file, line, when, head->checksum, checksum(head->magic, &head->checksum));
	ABORT();
    }
}

static void check_tail(mem_tail* tail, const char* when,
		      const char* file, int line)
{
    if(!check_magic(tail->magic, TAIL_MAGIC_CNT, FREED_MAGIC))
    {
	fprintf(stderr, "File: \"%s\"  Line: %d: %s: freed block\n",
		file, line, when);
	ABORT();
    }
    else if(check_magic(tail->magic, TAIL_MAGIC_CNT, TAIL_MAGIC))
    {
	fprintf(stderr, "File: \"%s\"  Line: %d: %s: bad trailer magic\n",
		file, line, when);
	ABORT();
    }
}

void* debug_calloc(size_t size, int count, const char* file, int line)
{
    mem_head*	head;
    size_t	sz = (size * count + 7) & ~7;
    int		i;
    //    char buf[255];
    //    sprintf(buf, "debug_calloc: File \"%s\";Line %d Size:%d\n", file, line, sz);
    //    slog(buf);

    if(sizeof(mem_head) & 7)
    {
	fprintf(stderr, "File \"%s\";Line %d\n", __FILE__, __LINE__);
	fprintf(stderr, "sizeof(mem_head) is not a multiple of 8\n");
	fprintf(stderr, "You'll not live to regret it\n");
	ABORT();
    }
    
    if(!(head = (mem_head*) malloc(sz + sizeof(mem_head) + sizeof(mem_tail))))
    {
	fprintf(stderr, "File \"%s\";Line %d # calloc(%d,%d) failed\n",
		file, line, size, count);
	ABORT();
    }

    set_magic(head->magic, HEAD_MAGIC_CNT, HEAD_MAGIC);
    head->tail = (mem_tail*) ((char*) (head + 1) + sz);
    head->tail->head = head;
    
    set_magic(head->tail->magic, TAIL_MAGIC_CNT, TAIL_MAGIC);

#if MEM_DEBUG > 1
    alloc_insert(head, file, line);
#endif

    head->checksum = checksum(head->magic, &head->checksum);

    return memset(head + 1, ALLOC_MAGIC, sz);
}

void* debug_realloc(void* ptr, size_t size, int count,
		    const char* file, int line)
{
  mem_head*	head;
  void*	new_block;
  size_t	sz = (size * count + 7) & ~7;
  size_t	old_sz;
  
  if(!ptr)
    {
      fprintf(stderr, "File \"%s\";Line %d # realloc(0,...)\n",
	      file, line);
      ABORT();
    }
  
  head = ((mem_head*) ptr) - 1;
  
  check_head(head, "realloc", file, line);
  check_tail(head->tail, "realloc", file, line);
  
  if((new_block = debug_calloc(size, count, file, line)) == NULL)
    {
      fprintf(stderr, "File \"%s\";Line %d # realloc(%d,%d) failed\n",
	      file, line, size, count);
      ABORT();
    }
  
  old_sz = (char *) head->tail - (char*) (head + 1);
  size *= count;
  if(size < old_sz)
    old_sz = size;
  
  memcpy(new_block, ptr, old_sz);
  
  debug_free(ptr, __FILE__, __LINE__);
  
  return new_block;
}

void debug_free(void* ptr, const char* file, int line)
{
    mem_head*		head;
    
    if(!ptr)
    {
	fprintf(stderr, "File \"%s\";Line %d # free of NIL\n",
		file, line);
	return;
    }

    head = ((mem_head*) ptr) - 1;

    check_head(head, "free", file, line);
    check_tail(head->tail, "free", file, line);

#if MEM_DEBUG > 1
    alloc_delete(head);
    
    /* Set everything to the FREED_MAGIC to detect as soon as possible uses of
       freed memory. */
    memset(head, FREED_MAGIC, (char*)head->tail - (char *)head + 
	   sizeof(mem_tail));
#else
    set_magic(head->magic, HEAD_MAGIC_CNT, FREED_MAGIC);
    set_magic(head->tail->magic, TAIL_MAGIC_CNT, FREED_MAGIC);
#endif
    
    free((void*) head);
}

void debug_check(void* ptr, const char* file, int line)
{
  mem_head*		head;

  if(!ptr)
  {
    fprintf(stderr, "File \"%s\";Line %d # check of NIL\n",
	    file, line);
    return;
  }

  head = ((mem_head*) ptr) - 1;

  check_head(head, "check", file, line);
  check_tail(head->tail, "check", file, line);
}

#if MEM_DEBUG > 1
static void alloc_insert(mem_head* head, const char* file, int line)
{
    if(alloc_head)
    {
	check_head(alloc_head, "alloc_insert", file, line);
	alloc_head->prev = head;
	alloc_head->checksum = checksum(alloc_head->magic,
					&alloc_head->checksum);
    }
    
    head->next = alloc_head;
    head->prev = 0;
    head->file = file;
    head->line = line;
    alloc_head = head;
}

static void alloc_delete(mem_head* head)
{
    mem_head* link;

    for (link = alloc_head; link; link = link->next) {
      if(link == head)
	{
	  check_head(link, "alloc_delete", link->file, link->line);
	  if (link->prev) {
	    link->prev->next = link->next;
	  } else {
	    alloc_head = link->next;
	  }
	  if (link->next) {
	    link->next->prev = link->prev;    
	  }
	  break;
	}
    }
}
#endif

#endif

void check_allocs(void)
{
#if defined(MEM_DEBUG) && (MEM_DEBUG > 1)
    mem_head*	head;

    for(head = alloc_head ; head ; head = head->next)
    {
	check_head(head, "check", head->file, head->line);
	check_tail(head->tail, "check", head->file, head->line);
    }
#endif
}

#if MEM_DEBUG > 1
void dump_memory_usage(void)
{
  mem_head*	head;
  char buf[255];
  
  for(head = alloc_head ; head ; head = head->next)
    {
      sprintf(buf, "Usage: File: \"%s\"  Line: %ld: Allocated: %d\n", head->file, 
	      head->line, (char*) head->tail - (char*) head + sizeof(mem_head));
      slog(buf);
    }
  abort();
}
#endif
