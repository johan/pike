/*
 * $Id$
 *
 * Glue for the Mysql-module
 */

#pike __VERSION__

#if constant(Mysql.mysql_result)
inherit Mysql.mysql_result;
#else /* !constant(Mysql.mysql_result) */
void create()
{
  destruct();
}
#endif /* constant(Mysql.mysql_result) */

