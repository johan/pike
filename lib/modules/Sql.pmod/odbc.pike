/*
 * $Id$
 *
 * Glue for the ODBC-module
 */

#if constant(Odbc.odbc)
inherit Odbc.odbc;
#else /* !constant(Odbc.odbc) */
void create()
{
  destruct();
}
#endif /* constant(Odbc.odbc) */
