
#include "config.h"

#if USE_stdlib
#include <stdlib.h>
#endif

#include <string.h>

#include "structs.h"
#include "utils.h"
#include "sstring.h"


#define	SS_MAX_INLINE	(32 - sizeof(sstring_t) + sizeof(char*))
#define SS_FAST_COUNT	8192
#define SS_HASH		8197

sstring_t*	ss_hash_table[SS_HASH];

void ss_insert_hash(sstring_t* string);
void ss_delete_hash(sstring_t* string);
int ss_hash(const char* string);
sstring_t* ss_find_hash(const char* string);

sstring_t* ss_make(const char* text)
{
    int 	len = text ? (strlen(text) + 1) : 0;
    sstring_t*	ss;

    if((ss = ss_find_hash(text)))
	return ss_share(ss);

    ss = ss_empty(len);

    if(text && ss && len)
    {
	strcpy(ss_data(ss), text);
	ss_insert_hash(ss);
    }
    
    return ss;
}

sstring_t* ss_empty(size_t len)
{
    sstring_t*	ss;

    if(len < SS_MAX_INLINE)
    {
	int size;

	size = sizeof(sstring_t) + len - sizeof(ss->data);
	
	CREATE(ss, sstring_t, size);
	ss->is_inline = 1;
    }
    else
    {
	CREATE(ss, sstring_t, 1);
	CREATE(ss->data.as_string, char, len);
	ss->is_inline = 0;
    }

    ss->byte_count = len;
    ss->ref_count = 1;
    ss->is_hashed = 0;
    
    return ss;
}

sstring_t* ss_share(sstring_t* ss)
{
    if(++ss->ref_count > SS_MAX_COUNT)
	return ss_split(ss);

    return ss;
}

sstring_t* ss_split(sstring_t* ss)
{
    sstring_t* new_ss;
    int		len;
    
    if(ss->ref_count <= 1)
	return ss;

    len = strlen(ss_data(ss)) + 1;
    
    new_ss = ss_empty(len);
    strcpy(ss_data(new_ss), ss_data(ss));

    ss_insert_hash(new_ss);
    
    ss_free(ss);

    return new_ss;
}

void ss_free(sstring_t* ss)
{
    if(ss && (--ss->ref_count <= 0))
    {
	if(ss->is_hashed)
	    ss_delete_hash(ss);

	if(!ss->is_inline)
	    FREE(ss->data.as_string);
	FREE(ss);
    }
}

sstring_t* ss_fread(FILE* fp)
{
    char	buf[16384];
    int		off;
    char*	ptr;
    int		len;
    
    for(off = 0 ; (ptr = fgets(buf + off, 16384 - off, fp)) ; off += len)
    {
	len = strlen(ptr);
	while((len > 0) && (ptr[--len] == '\n'))
	    ;

	if(ptr[len] == '~')
	{
	    ptr[len] = 0;
	    break;
	}

	ptr[++len] = '\n';
	ptr[++len] = '\r';
	ptr[++len] = 0;
    }

    return ss_make(buf);
}

void ss_insert_hash(sstring_t* string)
{
    sstring_t**		ptr;
    char*		data = ss_data(string);
    int			hash;

    hash = ss_hash(data);
    
    for(ptr = &ss_hash_table[hash] ; *ptr ; ptr = &(*ptr)->next)
	if(strcmp(ss_data(*ptr), data) > 0)
	    break;

    string->next = *ptr;
    *ptr = string;

    string->is_hashed = 1;
}

    
void ss_delete_hash(sstring_t* ss)
{
    sstring_t**		ptr;
    int			hash;
    
    hash = ss_hash(ss_data(ss));

    for(ptr = &ss_hash_table[hash] ; *ptr ; ptr = &(*ptr)->next)
	if(ss == *ptr)
	{
	    *ptr = ss->next;
	    break;
	}
}

int ss_hash(const char* string)
{
    int hash;
    
    if(!string)
	return 0;
    
    for(hash = 0 ; *string ; string++)
	hash = (hash << 1) ^ *string;

    return (hash & 0x7fffffff) % SS_HASH;
}

sstring_t* ss_find_hash(const char* string)
{
    sstring_t*		ptr;
    int			hash, comp;

    hash = ss_hash(string);
    for(ptr = ss_hash_table[hash] ; ptr ; ptr = ptr->next)
    {
	if((comp = strcmp(ss_data(ptr), string)) == 0)
	    return ptr;
	else if(comp > 0)
	    break;
    }

    return NULL;
}

void hash_check(void)
{
  int		i;

  for(i = 0 ; i < SS_HASH ; ++i)
  {
    sstring_t*	ptr = ss_hash_table[i];

    while(ptr)
    {
#if MEM_DEBUG > 0
      debug_check(ptr, __FILE__, __LINE__);
#endif
      if(ss_hash(ss_data(ptr)) != i)
	fprintf(stderr, "string has bad hash value for list\n");
	
      ptr = ptr->next;
    }
  }
}
