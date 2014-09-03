/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "util_str.h"

void load_objects(const char* name);
void load_mobiles(const char* name);

void syntax(const char* progName)
{
    fprintf(stderr,
"syntax: %s [-d lib_dir] [-O obj_file] [-M mob_file] [-m] [-o] [-a]\n",
	    progName);
}

int main(int argc, char* argv[])
{
    char buf[256];
    char* db_dir = ".";
    char* obj_file = NULL;
    char* mob_file = NULL;
    int do_obj = 0;
    int do_mob = 0;
    int option;
    
    extern char* optarg;
    
    srand(time(0));
    
    while((option = getopt(argc, argv, "d:O:M:moa?")) != EOF)
    {
	switch(option)
	{
	case 'd':
	    db_dir = optarg;
	    break;
	case 'O':
	    obj_file = optarg;
	    do_obj = 1;
	    break;
	case 'M':
	    mob_file = optarg;
	    do_mob = 1;
	    break;
	case 'm':
	    do_mob = 1;
	    break;
	case 'o':
	    do_obj = 1;
	    break;
	case 'a':
	    do_mob = 1;
	    do_obj = 1;
	    break;
	case '?':
	default:
	    syntax(argv[0]);
	    return 1;
	}
    }

    if(do_obj)
    {
	if(!obj_file)
	    sprintf(obj_file = buf, "%s/tinyworld.obj", db_dir);
	load_objects(obj_file);
    }

    if(do_mob)
    {
	if(!mob_file)
	    sprintf(mob_file = buf, "%s/tinyworld.mob", db_dir);
	load_mobiles(mob_file);
    }

    return 0;
}

void load_objects(const char* file)
{
    struct obj_data* obj;
    int i;
    
    boot_objects(file);

    for(i = 0 ; i < top_of_objt ; ++i)
    {
	obj = make_object(i, REAL);
	if(obj)
	    free_obj(obj);
    }
}

void load_mobiles(const char* file)
{
    struct char_data* mob;
    int i;

    boot_mobiles(file);

    for(i = 0 ; i < top_of_mobt ; ++i)
    {
	mob = make_mobile(i, REAL);
	if(mob)
	    free_char(mob);
    }
}
