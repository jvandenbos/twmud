
#ifndef __sstring_h
#define __sstring_h

#include <stdio.h>

typedef struct sstring_t sstring_t;

struct sstring_t
{
    unsigned int	is_inline : 1;
    unsigned int	is_hashed : 1;
    unsigned int	ref_count : 14;
#define SS_MAX_COUNT	16000
    unsigned int	byte_count : 16;
    sstring_t*		next;
    union sstring_union
    {
      char*		as_string;
      char		as_chars[sizeof(char*)];
    }			data;
};

#define ss_data(ss)	((ss) ? ((ss)->is_inline ? (ss)->data.as_chars : \
			                          (ss)->data.as_string) : \
                                 (char *) NULL)

sstring_t*		ss_make(const char* text);
sstring_t*		ss_empty(size_t len);
sstring_t*		ss_share(sstring_t* ss);
sstring_t*		ss_split(sstring_t* ss);
void			ss_free(sstring_t* ss);
sstring_t*		ss_fread(FILE* fp);

#endif
