/*
 * $Id$
 *
 * Glue for the ODBC-module
 */

#pike __REAL_VERSION__

#if constant(Odbc)
inherit Odbc.odbc_result;
#else /* !constant(Odbc) */
constant this_program_does_not_exist=1;
#endif /* constant(Odbc) */
