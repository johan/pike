/*
 * $Id$
 *
 * Advanced Encryption Standard (AES).
 *
 * Henrik Grubbström 2000-10-03
 */

#pike __REAL_VERSION__

//! Previously known as @[Crypto.rijndael].
inherit Crypto.rijndael;

//! Returns the string @tt{"AES"@}.
string name()
{
  return "AES";
}
