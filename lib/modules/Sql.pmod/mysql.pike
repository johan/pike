/*
 * $Id$
 *
 * Glue for the Mysql-module
 */

//.
//. File:	mysql.pike
//. RCSID:	$Id$
//. Author:	Henrik Grubbström (grubba@idonex.se)
//.
//. Synopsis:	Implements the glue to the Mysql-module.
//.
//. +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//.
//. Implements the glue needed to access the Mysql-module from the generic
//. SQL module.
//.

inherit Mysql.mysql;

//. - quote
//.   Quote a string so that it can safely be put in a query.
//. > s - String to quote.
string quote(string s)
{
  return(replace(s,
		 ({ "\\", "\"", "\0", "\'", "\n", "\r" }),
		 ({ "\\\\", "\\\"", "\\0", "\\\'", "\\n", "\\r" })));
}
