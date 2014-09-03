
#ifndef __PARSE__
#define __PARSE__

typedef struct parse_table parse_table;

typedef struct
{
    int			same_len;
    char* 		key;
    void*		data;
    parse_table* 	next;
} parse_entry;

typedef int (*parse_hash_func)(char data);

struct parse_table
{
    int			range;
    parse_entry*	entries;
    parse_hash_func	hash;
};

parse_table*		MakeParseTable(int range, parse_hash_func hash);
void			AddParseEntry(parse_table* table,
				      const char* key,
				      void* data);
void*			FindParseEntry(parse_table* table,
				       char* key,
				       int exact);

#endif
