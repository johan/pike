/*\
||| This file is part of Pike. For copyright information see COPYRIGHT.
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/

/*
 * $Id$
 */
#ifndef FSORT_H
#define FSORT_H

typedef int (*fsortfun)(const void *,const void *);

/* Prototypes begin here */
void fsort(void *base,
	   long elms,
	   long elmSize,
	   fsortfun cmpfunc);
/* Prototypes end here */


#endif
