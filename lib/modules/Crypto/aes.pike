/*
 * $Id$
 *
 * Advanced Encryption Standard (AES).
 *
 * Henrik Grubbström 2000-10-03
 */

#pike __REAL_VERSION__

/* aka rijndael */
inherit Crypto.rijndael;

string name()
{
  return "AES";
}
