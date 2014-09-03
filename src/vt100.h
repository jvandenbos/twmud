/******************************************************************************
*   File: vt100.h							      *
*  Usage: Header File for vt100/ansi controls 			              *
*     By: Stefan Wasilewski      					      *
*   									      *
*   $Author: twmain $							      *
*   $Date: 2004/03/14 18:46:24 $					      *
*   $Revision: 1.1.1.1 $							      *
* 									      *
******************************************************************************/

#define VTMOVE(x,y)	    	"\033[x;yH"
#define VTUP(y)			"\033[yA"
#define VTDOWN(y)		"\033[yB"
#define VTRIGHT(x)		"\033[xC"
#define VTLEFT(x)		"\033[xD"
#define VTFIND(x,y) 		"\033[x;yR"
#define VTSAVE			"\033[s"
#define VTRETURN		"\033[u"

/* Erase Functions */

#define VTCLRSCR		"\033[2J"
#define VTCLRTOEOL		"\033[K"

/* Set Graphics Rendition */

#define VTSET(args)		"\033[args"

#define VTNORM			0      /* Normal Text */
#define VTBOLD                  1      /* Bold Text */
#define VTUNDERLINE             4      /* Underline (mono only) */
#define VTBLINK                 5      /* Blink on */
#define VTREVERSE               7      /* Reverse on */
#define VTINVIS                 8      /* Invisible */
#define VTBLKFGRD               30     /* Black Foreground */
#define VTREDFGRD               31     /* Red Foreground */
#define VTGRNFGRD               32     /* Green fgrd */
#define VTYELFGRD               33     /* Yellow fgrd */
#define VTBLUFGRD		34     /* Blue fgrd */
#define VTMAGFGRD               35     /* Magenta fgrd */
#define VTCYNFGRD               36     /* Cyan fgrd */
#define VTWHTFGRD               37     /* White fgrd */
#define VTBLKBGRD               40     /* Black Background */
#define VTREDBGRD               41     /* Red bgrd */
#define VTGRNBGRD               42     /* Green fgrd */
#define VTYELBGRD               43     /* Yellow fgrd */
#define VTBLUBGRD               44     /* Blue fgrd */
#define VTMAGBGRD               45     /* Magenta fgrd */
#define VTCYNBGRD               46     /* Cyan fgrd */
#define VTWHTBGRD               47     /* White fgrd */


/* definitions for ANSI and VT100 escape codes go here */

#define ANSI_NORMAL		"\033[0m"
#define ANSI_BLACK  		"\033[30m"
#define ANSI_RED		"\033[31m"
#define ANSI_GREEN  		"\033[32m"
#define ANSI_ORANGE 		"\033[33m"
#define ANSI_BLUE   		"\033[34m"
#define ANSI_MAGENTA 		"\033[35m"
#define ANSI_VIOLET  		"\033[35m"
#define ANSI_CYAN		"\033[36m"
#define ANSI_WHITE   		"\033[37m"
#define ANSI_CLS     		"\033[2J\033[H"
#define ANSI_BRIGHT_ORANGE 	"\033[33;1m"
