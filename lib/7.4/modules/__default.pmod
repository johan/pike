// Compatibility namespace
// $Id$

#pike 7.5

//! Pike 7.4 compatibility.
//!
//! The symbols in this namespace will appear in
//! programs that use @tt{#pike 7.4@} or lower.

//! @decl inherit predef::

mapping(string:mixed) all_constants()
{
  mapping(string:mixed) ret=predef::all_constants()+([]);
  // No differences (yet).
  return ret;
}

