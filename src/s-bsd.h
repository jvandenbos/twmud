/* This is admitedly a hack.  Though it works and conforms to most standards
   I could find in the other system headers. It was developed on a Sparc 
   running BSD Unix v ?? */

/* NOTE : BSD seems to be defined automatically */
/* #define BSD 1 */

#define SPARC		1
#define AUX             1

#define HAVE_srand	1

#define DONT_USE_malloc_h 1
#define DONT_USE_mallinfo 1

#define USE_sys_file	1
#define USE_time	1
#define USE_stdlib	1
#define USE_unistd	1

#define HAVE_inet_ntoa	1

#include <sys/param.h>

#define MAX_OPEN_FILES	128

/*
 * if some things are builtin or not..
 */
#define NEED_STRDUP	0

