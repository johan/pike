/*
 * $Id$
 *
 * Glue for the ODBC-module
 */

#if constant(Odbc.odbc_result)
inherit Odbc.odbc_result;
#else /* !constant(Odbc.odbc_result) */
void create()
{
  destruct();
}
#endif /* constant(Odbc.odbc_result) */
