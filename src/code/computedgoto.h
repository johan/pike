/*
 * $Id$
 */

#define PIKE_OPCODE_T	void *

#define UPDATE_PC()

#define READ_INCR_BYTE(PC)	((INT32)(ptrdiff_t)((PC)++)[0])
