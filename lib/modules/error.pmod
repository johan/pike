#pike __REAL_VERSION__

//! @decl void error(string message)
//! @appears error
//!
//! Identical to @tt{throw(({message,backtrace()}))@}.

// Moahahahah!
// $Id$
void `()(string f, mixed ... args)
{
  array(array) b = backtrace();
  if (sizeof(args)) f = sprintf(f, @args);
  throw( ({ f, b[..sizeof(b)-2] }) );
}
