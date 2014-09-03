/* ************************************************************************
*  File: ident.h                               Part of Thieves' World MUD *
*                                                                         *
*  Usage: Header file containing stuff required for rfc 931 ident stuff   *
*                                                                         *
*  $Author: twmain $
*  $Date: 2004/03/14 18:46:24 $
*  $Revision: 1.1.1.1 $
************************************************************************ */

void ident_init(void);
void ident_start(struct descriptor_data *d, long addr);
void ident_check(struct descriptor_data *d);
int waiting_for_ident(struct descriptor_data *d);
