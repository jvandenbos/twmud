/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "config.h"
#include "structs.h"
#include "db.h"

#include <stdio.h>

/* This is really gross and is likely to break for future versions of */
/* the player file.  However, we need it right now, and I don't have */
/* the time necessary to do it right.   -- DWB */

#define PLAYER_VERSION 14		/* this is current up to */
					/* version 10 */

typedef struct 
{
    long magic;
    long version;
    long p_offset;
    long o_offset;
    long a_offset;
} player_header;

void freerent(const char* file);

void main(int argc, char* argv[])
{
    while(*++argv)
	freerent(*argv);
}

void freerent(const char* file)
{
    int 		fd;
    player_header	hdr;
    int			offset;
    long		zero = 0;
    
    if((fd = open(file, 2)) < 0)
    {
	perror(file);
	return;
    }
    
    if((read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) ||
	    (hdr.magic != PLAYER_MAGIC))
    {
	fprintf(stderr, "%s: not a player file\n", file);
    }
    else if(hdr.version > PLAYER_VERSION)
    {
	fprintf(stderr, "%s: version number to high...\n", file);
    }
    else if(offset = hdr.p_offset + (hdr.version >= 7 ? 4 : 3),
	    lseek(fd, offset, 0) != offset)
    {
	fprintf(stderr, "%s: file too short?  can't seek\n", file);
    }
    else if(write(fd, &zero, sizeof(zero)) != sizeof(zero))
    {
	fprintf(stderr, "%s: Can't write\n", file);
    }

    close(fd);
}
