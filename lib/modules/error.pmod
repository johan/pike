// Moahahahah!
// $Id$
void `()(string f, mixed ... args)
{
  array(array) b = backtrace();
  throw( ({ sprintf(f, @args), b[..sizeof(b)-2] }) );
}
