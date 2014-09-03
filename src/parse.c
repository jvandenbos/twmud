
#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <strings.h>
#include <string.h>

#include "structs.h"
#include "parse.h"
#include "utils.h"
#include "utility.h"
#include "util_str.h"

parse_table* MakeParseTable(int range, parse_hash_func hash)
{
    parse_table*	table;
    parse_entry*	entry;
    int			i;
    
    CREATE(table, parse_table, 1);
    CREATE(table->entries, parse_entry, range);
    table->range = range;
    table->hash = hash;

    for(i = 0, entry = table->entries ; i < range ; ++i, ++entry)
    {
	entry->same_len = 0;
	entry->key = 0;
	entry->data = 0;
	entry->next = 0;
    }

    return table;
}

void DumpParseTable(parse_table* table, int indent)
{
    int		i;
    parse_entry	*entry;
    
    if(!table)
	return;
    
    for(i = 0, entry = table->entries ; i < table->range ; ++i, ++entry)
    {
	if(entry->key)
	{
	    fprintf(stderr, "%.*s%2d - %s\n", indent,
		    "                                               ",
		    entry->same_len, entry->key);
	    if(entry->next)
		DumpParseTable(entry->next, indent + 2);
	}
    }
}
		    
void AddParseEntry(parse_table* tableP,
		   const char* key,
		   void* data)
{
    int			match_len = 0;
    parse_entry*	entry;
    parse_table*	table = tableP;
    char*		new_key = strdup(key);
    int			hash;
    
    do
    {
	hash = (*table->hash)(key[match_len]);
	if((hash < 0) || (hash >= table->range))
	  return;
    
	entry = &table->entries[hash];
	
	if(!entry->key)
	{
	    /* empty slot in the table, just fill it */
	    entry->same_len = strlen(key);
	    entry->data = data;
	    entry->next = 0;
	    entry->key = new_key;
	    return;
	}
	else
	{
	    /* conceivably have to split an entry here */
	    while((match_len < entry->same_len) &&
		  (entry->key[match_len] == key[match_len]))
	    {
		match_len++;
	    }

	    if(!entry->key[match_len] && !key[match_len])
	    {
		char buf[256];
		sprintf(buf, "Duplicate parse entry: %s\n", key);
		log_msg(buf);
		return;
	    }
      
	    if(!entry->next)
		entry->next = MakeParseTable(table->range, table->hash);
      
	    /* if we have a shorter match then we have to split them */
	    if(match_len < entry->same_len)
	    {
		parse_entry* 	next;
		parse_table*	tab;

		tab = MakeParseTable(table->range, table->hash);
		hash = (table->hash)(entry->key[match_len]);
		if((hash < 0) || (hash >= table->range))
		  return;
		next = &tab->entries[hash];
		*next = *entry;
	
		entry->same_len = match_len;
		entry->next = tab;
	    }
      
	    table = entry->next;
	}
    }
    while(1);
}

void* FindParseEntry(parse_table* table, char* key, int exact)
{
    int			matched = 0;
    int			hash;
    parse_entry*	entry;

    while(table)
    {
	hash = (table->hash)(key[matched]);
	if((hash < 0) || (hash >= table->range))
	  return 0;
	entry = &table->entries[hash];

	if(!entry->key)
	    /* if we don't have a key for the sub level, we don't
	       have the match we're looking for */
	    return NULL;
	
	while((matched < entry->same_len) &&
	      (key[matched] == entry->key[matched]))
	{
	    /* see if we've run out of data, if so we have an
	       exact match */
	    if(key[matched] == 0)
		return entry->data;
	    matched++;
	}

	/* see if we don't need an exact match and we've run out
	   source pattern */
	if(!exact && !key[matched])
	    return entry->data;

	/* ok, if we get here we don't have a match and need to
	   recurse a bit */
	table = entry->next;
    }

    /* we ran out of tables without a significant enough match */
    return NULL;
}

