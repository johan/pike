/*
 * $Id$
 *
 * Glue for the Mysql-module using SSL
 */

//! Implements the glue needed to access MySQL databases using SSL from the generic
//! SQL module.

#pike __REAL_VERSION__

#if constant(Mysql.mysql) && constant(Mysql.mysql.CLIENT_SSL)

inherit Sql.mysql;

void create(string host,
	    string db,
	    string user,
	    string password,
	    mapping(string:mixed) options) {

  mapping opts = ([ ]);
  if (mappingp(options))
    opts = options;

  if (!(opts->connect_options & Mysql.mysql.CLIENT_SSL))
      opts->connect_options += Mysql.mysql.CLIENT_SSL;

  if (!opts->mysql_config_file)
    opts->mysql_config_file = "/etc/my.cnf";

  ::create(host||"", db||"", user||"", password||"", opts);
}
#endif
