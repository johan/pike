/* $Id$
 *
 */

#pike __REAL_VERSION__

//! IDEA CBC.
//!
//! @seealso
//!   @[cbc], @[idea]

inherit Nettle.CBC;
void create() { ::create(Crypto.IDEA()); }
string crypt_block(string data) { return crypt(data); }
int query_key_length() { return key_size(); }
int query_block_size() { return block_size(); }
