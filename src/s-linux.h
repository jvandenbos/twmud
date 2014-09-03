/*
 * Copr. 1994 Paul A. Cole -- All Rights Reserved
 */

#define LINUX           1
#define DONT_USE_mallinfo
/* This determins that we can't use mallinfo() function nor structures.
   Whatever those are... queen.mcs.drexel.edu was using them but they 
   weren't defined in the manual pages so go figure. *shrug*
*/

/* Linux seems to behave like an AUX system in all other circumstances so
   I used that as a model for the system.h file
*/

#define AUX		1

#define HAVE_srand	1

#define USE_sys_file	1
#define USE_time	1
#define USE_stdlib	1
#define USE_unistd	1

#define HAVE_inet_ntoa	1

#define NEED_strdup	1
