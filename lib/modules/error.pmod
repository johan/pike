#pike __REAL_VERSION__

// Moahahahah!
// $Id$
void `()(string f, mixed ... args)
{
  array(array) b = backtrace();
  if (sizeof(args)) f = sprintf(f, @args);
  throw( ({ f, b[..sizeof(b)-2] }) );
}
