/* $Id$
 *
 */

inherit Crypto.cbc : cbc;

void create()
{
  cbc::create(Crypto.idea);
}
