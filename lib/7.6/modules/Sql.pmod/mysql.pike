/*
 * $Id$
 *
 * Glue for the old broken Mysql-module which fetched all rows at once always
 */

#pike 7.7

inherit Sql.mysql;

int|object big_query(mixed ... args) {
  object pre_res = ::big_query(@args);

  program p = master()->get_compilation_handler(7,6)->resolv("Mysql.mysql_result");

  return pre_res && p(pre_res);
}
