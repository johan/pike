/*
 * $Id$
 *
 * Glue for the ODBC-module
 */

#if constant(Odbc.odbc)
inherit Odbc.odbc;
#else /* !constant(Odbc.odbc) */
#error "ODBC support not available.\n"
#endif /* constant(Odbc.odbc) */
