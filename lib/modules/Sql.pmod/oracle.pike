/*
 * $Id$
 *
 * Glue for the Oracle-module
 */

#if constant(Oracle.oracle)
inherit Oracle.oracle;

string server_info()
{
  return "Oracle";
}
#else /* !constant(Oracle.oracle) */
#error "Oracle support not available.\n"
#endif /* constant(Oracle.oracle) */
