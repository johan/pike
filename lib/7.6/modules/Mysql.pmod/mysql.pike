/*
 * $Id$
 *
 * Glue for the old broken Mysql-module which fetched all rows at once always
 */

#pike 7.7

inherit Mysql.mysql;

int|object big_query(mixed ... args) {
  object pre_res = ::big_query(@args);

  return pre_res && .mysql_result(pre_res);
}
