/*
 * $Id$
 *
 * Glue for the Oracle-module
 */

#if constant(Oracle.oracle)
inherit Oracle.oracle;
#else /* !constant(Oracle.oracle) */
void create()
{
  destruct();
}
#endif /* constant(Oracle.oracle) */
