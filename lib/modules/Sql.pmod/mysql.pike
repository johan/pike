/*
 * $Id$
 *
 * Glue for the Mysql-module
 */

inherit Mysql.mysql;

string sqlquote(string s)
{
  return(replace(s,
		 ({ "\\", "\"", "\0", "\'", "\n", "\r" }),
		 ({ "\\\\", "\\\"", "\\0", "\\\'", "\\n", "\\r" })));
}
