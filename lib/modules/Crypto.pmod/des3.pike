/* $Id$
 *
 */

#pike __REAL_VERSION__

inherit Nettle.DES3_State;

string name() { return "DES"; } // Yep, it doesn't say DES3

array(int) query_key_length() { return ({ 8, 8, 8 }); }
int query_block_size() { return 8; }
int query_key_size() { return 16; }
string crypt_block(string p) { return crypt(p); }
