#pike 7.3

//
// $Id$
//
// Pike 7.2 backward compatibility layer.
//

inherit Thread;

class Condition
{
  inherit Thread::Condition;

  // Old-style wait().
  void wait(void|Mutex m)
  {
    if (!m) {
      m = Mutex();
      m->lock();
    }
    ::wait();
  }
}
