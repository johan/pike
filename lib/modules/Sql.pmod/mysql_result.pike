/*
 * $Id$
 *
 * Glue for the Mysql-module
 */

#pike __REAL_VERSION__

#if constant(Mysql)
inherit Mysql.mysql_result;
#else /* !constant(Mysql) */
constant this_program_does_not_exist=1;
#endif /* constant(Mysql) */

