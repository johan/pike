/*
 * $Id$
 *
 * Implements the generic parts of the SQL-interface
 *
 * Henrik Grubbstr�m 1996-01-09
 */

#pike __REAL_VERSION__

//! Implements those functions that need not be present in all SQL-modules.

#define throw_error(X)	throw(({ (X), backtrace() }))

//! Object to use for the actual SQL-queries.
object master_sql;

//! Convert all field names in mappings to lower_case.
//! Only relevant to databases which only implement big_query(),
//! and use upper/mixed-case fieldnames (eg Oracle).
//! @int
//! @value 0
//!   No (default)
//! @value 1
//!   Yes
//! @endint
int case_convert;

//! @decl string quote(string s)
//! Quote a string @[s] so that it can safely be put in a query.

function(string:string) quote = .sql_util.quote;

//! @decl string encode_time(int t, int|void is_utc)
//! Converts a system time value to an appropriately formatted time
//! spec for the database.
//! @[t] Time to encode.
//! @[is_utc] If nonzero then time is taken as a "full" unix time spec
//! (where the date part is ignored), otherwise it's converted as a
//! seconds-since-midnight value.

function(int,void|int:string) encode_time;

//! @decl int decode_time(string t, int|void want_utc)
//! Converts a database time spec to a system time value.
//! @[t] Time spec to decode.
//! @[want_utc] Take the date part from this system time value. If zero, a
//! seconds-since-midnight value is returned.

function(string,void|int:int) decode_time;

//! @decl string encode_date(int t)
//! Converts a system time value to an appropriately formatted
//! date-only spec for the database.
//! @[t] Time to encode.

function(int:string) encode_date;

//! @decl int decode_date(string d)
//! Converts a database date-only spec to a system time value.
//! @[d] Date spec to decode.

function(string:int) decode_date;

//! @decl string encode_datetime(int t)
//! Converts a system time value to an appropriately formatted
//! date and time spec for the database.
//! @[t] Time to encode.

function(int:string) encode_datetime;

//! @decl int decode_datetime(string datetime)
//! Converts a database date and time spec to a system time value.
//! @[datetime] Date and time spec to decode.

function(string:int) decode_datetime;

//! @decl void create()
//! @decl void create(string host)
//! @decl void create(string host, string db)
//! @decl void create(string host, mapping(string:int|string) options)
//! @decl void create(string host, string db, string user)
//! @decl void create(string host, string db, string user, @
//!                   string password)
//! @decl void create(string host, string db, string user, @
//!                   string password, mapping(string:int|string) options)
//! @decl void create(object host)
//! @decl void create(object host, string db)
//!
//! Create a new generic SQL object.
//!
//! @param host
//!   @mixed
//!     @type object
//!       Use this object to access the SQL-database.
//!     @type string
//!       Connect to the server specified.
//!       The string should be on the format:
//!       @tt{dbtype://[user[:password]@@]hostname[:port][/database]@}
//!       Use the dbtype protocol to connect to the database server
//!       on the specified host.
//!       If the hostname is @tt{""@}, access through a UNIX-domain
//!	  socket or similar, e g @tt{"mysql://root@@:/tmp/mysql.sock/"@}
//!     @type int(0..0)
//!       Access through a UNIX-domain socket or similar.
//!   @endmixed
//!
//! @param db
//!   Select this database.
//!
//! @param user
//!   User name to access the database as.
//!
//! @param password
//!   Password to access the database.
//!
//! @param options
//!   Optional mapping of options.
//!   See the SQL-database documentation for the supported options.
//!   (eg @[Mysql.mysql()->create()]).
//!
//! @note
//!   In versions of Pike prior to 7.2 it was possible to leave out the
//!   dbtype, but that has been deprecated, since it never worked well.
//!
//! @note
//!   Support for @[options] was added in Pike 7.3.
//!
void create(void|string|object host, void|string|mapping(string:int|string) db,
	    void|string user, void|string password,
	    void|mapping(string:int|string) options)
{
  if (objectp(host)) {
    master_sql = host;
    if ((user && user != "") || (password && password != "") ||
	(options && sizeof(options))) {
      throw_error("Sql.sql(): Only the database argument is supported when "
		  "first argument is an object\n");
    }
    if (db && db != "") {
      master_sql->select_db(db);
    }
  }
  else {
    if (mappingp(db)) {
      options = db;
      db = 0;
    }
    if (db == "") {
      db = 0;
    }
    if (user == "") {
      user = 0;
    }
    if (password == "") {
      password = 0;
    }

    string program_name;

    if (host && (host != replace(host, ({ ":", "/", "@" }), ({ "", "", "" })))) {

      // The hostname is on the format:
      //
      // dbtype://[user[:password]@]hostname[:port][/database]

      array(string) arr = host/"://";
      if ((sizeof(arr) > 1) && (arr[0] != "")) {
	if (sizeof(arr[0]/".pike") > 1) {
	  program_name = (arr[0]/".pike")[0];
	} else {
	  program_name = arr[0];
	}
	host = arr[1..] * "://";
      }
      arr = host/"@";
      if (sizeof(arr) > 1) {
	// User and/or password specified
	host = arr[-1];
	arr = (arr[0..sizeof(arr)-2]*"@")/":";
	if (!user && sizeof(arr[0])) {
	  user = arr[0];
	}
	if (!password && (sizeof(arr) > 1)) {
	  password = arr[1..]*":";
	  if (password == "") {
	    password = 0;
	  }
	}
      }
      arr = host/"/";
      if (sizeof(arr) > 1) {
	host = arr[..sizeof(arr)-2]*"/";
	if (!db) {
	  db = arr[-1];
	}
      }
    }

    if (host == "") {
      host = 0;
    }

    if (!program_name) {
      throw_error("Sql.Sql(): No protocol specified.\n");
    }
    /* Don't call ourselves... */
    if ((sizeof(program_name / "_result") != 1) ||
	(lower_case(program_name[..2]) == "sql")) {
      throw_error(sprintf("Sql.Sql(): Unsupported protocol: %O\n",
			  program_name));
    }


    program p;

    p = Sql[program_name];

    if (p) {
      if (options) {
	master_sql = p(host||"", db||"", user||"", password||"", options);
      } else if (password) {
	master_sql = p(host||"", db||"", user||"", password);
      } else if (user) {
	master_sql = p(host||"", db||"", user);
      } else if (db) {
	master_sql = p(host||"", db);
      } else if (host) {
	master_sql = p(host);
      } else {
	master_sql = p();
      }
    } else {
      throw_error(sprintf("Sql.sql(): Failed to index module Sql.%s\n",
			  program_name));
    }
  }

  if (master_sql->quote) quote = master_sql->quote;
  encode_time = master_sql->encode_time || .sql_util.fallback;
  decode_time = master_sql->decode_time || .sql_util.fallback;
  encode_date = master_sql->encode_date || .sql_util.fallback;
  decode_date = master_sql->decode_date || .sql_util.fallback;
  encode_datetime = master_sql->encode_datetime || .sql_util.fallback;
  decode_datetime = master_sql->decode_datetime || .sql_util.fallback;
}

string _sprintf(int type, mapping|void flags)
{
  if(type=='O' && master_sql && master_sql->_sprintf)
    return sprintf("Sql.%O", master_sql);
}

static private array(mapping(string:mixed)) res_obj_to_array(object res_obj)
{
  if (res_obj) 
  {
    /* Not very efficient, but sufficient */
    array(mapping(string:mixed)) res = ({});
    array(string) fieldnames;
    array(mixed) row;
    array(mapping) fields = res_obj->fetch_fields();

    fieldnames = (Array.map(fields,
                            lambda (mapping(string:mixed) m) {
                              return((m->table||"") + "." + m->name);
                            }) +
                  fields->name);

    if (case_convert)
      fieldnames = Array.map(fieldnames, lower_case);


    while (row = res_obj->fetch_row())
      res += ({ mkmapping(fieldnames, row + row) });

    return(res);
  }
  return 0;
}

//! Return last error message.
int|string error()
{
  if (functionp (master_sql->error))
    return master_sql->error();
  return "Unknown error";
}

//! Select database to access.
void select_db(string db)
{
  master_sql->select_db(db);
}

//! Compiles the query (if possible). Otherwise returns it as is.
//! The resulting object can be used multiple times in query() and
//! big_query().
//!
//! @param q
//!   SQL-query to compile.
string|object compile_query(string q)
{
  if (functionp(master_sql->compile_query)) {
    return(master_sql->compile_query(q));
  }
  return(q);
}

//! Handle sprintf-based quoted arguments
private array(string|mapping(string|int:mixed)) handle_extraargs(string query, array(mixed) extraargs) {
  array(mixed) args=allocate(sizeof(extraargs));
  mixed s;
  int j, a=0;
  mapping(string|int:mixed) b = 0;
  for (j=0;j<sizeof(extraargs);j++) {
    s=extraargs[j];
    if (intp(s) || floatp(s)) {
      args[j]=s;
      continue;
    }
    if (stringp(s) || multisetp(s)) {
      args[j]=":arg"+(a++);
      if(!b) b = ([]);
      b[args[j]] = s;
      continue;
    }
    throw_error("Wrong type to query argument #"+(j+1)+"\n");
  }
  return ({sprintf(query,@args), b});
}

//!   Send an SQL query to the underlying SQL-server. The result is returned
//!   as an array of mappings indexed on the name of the columns.
//!   Returns 0 if the query didn't return any result (e.g. INSERT or similar).
//!
//! @param q
//!   Query to send to the SQL-server. This can either be a string with the
//!   query, or a previously compiled query (see compile_query()).
//! @param extraargs
//!   This parameter, if specified, can be in two forms:
//!
//!   @ol
//!     @item
//!     A mapping containing bindings of variables used in the query.
//!     A variable is identified by a colon (:) followed by a name or number.
//!     Each index in the mapping corresponds to one such variable, and the
//!     value for that index is substituted (quoted) into the query wherever
//!     the variable is used.
//!
//!     @code{ query("select foo from bar where gazonk=:baz",
//!         ([":baz":"value"])) ) @}
//!
//!     Binary values (BLOBs) may need to be placed in multisets.
//!
//!     @item
//!     Arguments as you would use in sprintf. They are automatically
//!     quoted.
//!
//!     @code{ query("select foo from bar where gazonk=%s","value") ) @}
//!   @endol
array(mapping(string:mixed)) query(object|string q,
                                   mixed ... extraargs)
{
  mapping(string|int:mixed) bindings=0;
  if (extraargs && sizeof(extraargs)) {
    if (mappingp(extraargs[0])) {
      bindings=extraargs[0];
    } else {
      [q,bindings]=handle_extraargs(q,extraargs);
    }
  }
  if (functionp(master_sql->query)) {
    if (bindings) {
      return(master_sql->query(q, bindings));
    } else {
      return(master_sql->query(q));
    }
  }
  if (bindings) {
    return(res_obj_to_array(master_sql->big_query(q, bindings)));
  } else {
    return(res_obj_to_array(master_sql->big_query(q)));
  }
}

//! Send an SQL query to the underlying SQL-server. The result is returned
//! as a Sql.sql_result object. This allows for having results larger than
//! the available memory, and returning some more info about the result.
//! Returns 0 if the query didn't return any result (e.g. INSERT or similar).
//! For the other arguments, they are the same as for the @[query()] function.
int|object big_query(object|string q, mixed ... extraargs)
{
  object|array(mapping) pre_res;
  mapping(string|int:mixed) bindings=0;

  if (extraargs && sizeof(extraargs)) {
    if (mappingp(extraargs[0])) {
      bindings=extraargs[0];
    } else {
      [q,bindings]=handle_extraargs(q,extraargs);
    }
  }

  if (functionp(master_sql->big_query)) {
    if (bindings) {
      pre_res = master_sql->big_query(q, bindings);
    } else {
      pre_res = master_sql->big_query(q);
    }
  } else if (bindings) {
    pre_res = master_sql->query(q, bindings);
  } else {
    pre_res = master_sql->query(q);
  }
  return(pre_res && Sql.sql_result(pre_res));
}

//! Create a new database.
//!
//! @param db
//!   Name of database to create.
void create_db(string db)
{
  master_sql->create_db(db);
}

//! Drop database
//!
//! @param db
//!   Name of database to drop.
void drop_db(string db)
{
  master_sql->drop_db(db);
}

//! Shutdown a database server.
void shutdown()
{
  if (functionp(master_sql->shutdown)) {
    master_sql->shutdown();
  } else {
    throw_error("sql->shutdown(): Not supported by this database\n");
  }
}

//! Reload the tables.
void reload()
{
  if (functionp(master_sql->reload)) {
    master_sql->reload();
  } else {
    /* Probably safe to make this a NOOP */
  }
}

//! Return info about the current SQL-server.
string server_info()
{
  if (functionp(master_sql->server_info)) {
    return(master_sql->server_info());
  }
  return("Unknown SQL-server");
}

//! Return info about the connection to the SQL-server.
string host_info()
{
  if (functionp(master_sql->host_info)) {
    return(master_sql->host_info());
  } 
  return("Unknown connection to host");
}

//! List available databases on this SQL-server.
//!
//! @param wild
//!   Optional wildcard to match against.
array(string) list_dbs(string|void wild)
{
  array(string)|array(mapping(string:mixed))|object res;
  
  if (functionp(master_sql->list_dbs)) {
    if (objectp(res = master_sql->list_dbs())) {
      res = res_obj_to_array(res);
    }
  } else {
    catch {
      res = query("show databases");
    };
  }
  if (res && sizeof(res) && mappingp(res[0])) {
    res = Array.map(res, lambda (mapping m) {
      return(values(m)[0]);	/* Hope that there's only one field */
    } );
  }
  if (res && wild) {
    res = filter(res,
		 Regexp(replace(wild, ({"%", "_"}), ({".*", "."})))->match);
  }
  return(res);
}

//! List tables available in the current database.
//!
//! @param wild
//!   Optional wildcard to match against.
array(string) list_tables(string|void wild)
{
  array(string)|array(mapping(string:mixed))|object res;
  
  if (functionp(master_sql->list_tables)) {
    if (objectp(res = master_sql->list_tables())) {
      res = res_obj_to_array(res);
    }
  } else {
    catch {
      res = query("show tables");
    };
  }
  if (res && sizeof(res) && mappingp(res[0])) {
    res = Array.map(res, lambda (mapping m) {
      return(values(m)[0]);	/* Hope that there's only one field */
    } );
  }
  if (res && wild) {
    res = filter(res,
		 Regexp(replace(wild, ({"%", "_"}), ({".*", "."})))->match);
  }
  return(res);
}

//! List fields available in the specified table
//!
//! @param table
//!   Table to list the fields of.
//!
//! @param wild
//!   Optional wildcard to match against.
array(mapping(string:mixed)) list_fields(string table, string|void wild)
{
  array(mapping(string:mixed))|object res;

  if (functionp(master_sql->list_fields)) {
    if (objectp(res = master_sql->list_fields(table))) {
      res = res_obj_to_array(res);
    }
    if (wild) {
      res = filter(res,
		   lambda(mapping row, function(string:int) match) {
		     return match(row->name);
		   },
		   Regexp(replace(wild, ({"%", "_"}), ({".*", "."})))->match);
    }
    return(res);
  }
  catch {
    if (wild) {
      res = query("show fields from \'" + table +
		  "\' like \'" + wild + "\'");
    } else {
      res = query("show fields from \'" + table + "\'");
    }
  };
  res = res && Array.map(res, lambda (mapping m, string table) {
    foreach(indices(m), string str) {
      /* Add the lower case variants */
      string low_str = lower_case(str);
      if (low_str != str && !m[low_str]) {
	m[low_str] = m[str];
	m_delete(m, str);	/* Remove duplicate */
      }
    }
    if ((!m->name) && m->field) {
      m["name"] = m->field;
      m_delete(m, "field");	/* Remove duplicate */
    }
    if (!m->table) {
      m["table"] = table;
    }
    return(m);
  }, table);
  return(res);
}
