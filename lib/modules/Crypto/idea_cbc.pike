/* $Id$
 *
 */

#pike __VERSION__

inherit Crypto.cbc : cbc;

void create()
{
  cbc::create(Crypto.idea);
}
