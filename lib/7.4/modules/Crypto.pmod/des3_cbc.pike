/* $Id$
 *
 */

#pike __REAL_VERSION__

//! Triple-DES CBC.
//!
//! @seealso
//!   @[cbc], @[des]

inherit Nettle.CBC;
void create() { ::create(Crypto.DES3()); }
string crypt_block(string data) { return crypt(data); }
int query_key_length() { return key_size(); }
int query_block_size() { return block_size(); }
