/*
 * $Id$
 *
 * Some SQL utility functions.
 * They are kept here to avoid circular references.
 *
 * Henrik Grubbström 1999-07-01
 */

//.
//. File:	sql_util.pmod
//. RCSID:	$Id$
//. Author:	Henrik Grubbström (grubba@idonex.se)
//.
//. Synopsis:	Some SQL utility functions
//.
//. +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//.
//. These functions are kept here mainly to avoid circular references.
//.

#define throw_error(X)	throw(({ (X), backtrace() }))

//. - quote
//.   Quote a string so that it can safely be put in a query.
//. > s - String to quote.
string quote(string s)
{
  return(replace(s, "\'", "\'\'"));
}

//. - fallback
//.   Throw an error in case an unimplemented function is called.
void fallback(void)
{
  throw_error ("Function not supported in this database.");
}
