// Moahahahah!
// $Id$
void `()(mixed ... args)
{
  throw( ({ sprintf(@args), backtrace() }) );
}
