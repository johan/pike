/*
 * $Id$
 *
 * Rule priority specification
 *
 * Henrik Grubbström 1996-12-05
 */

#pike __REAL_VERSION__

//.
//. File:	priority.pike
//. RCSID:	$Id$
//. Author:	Henrik Grubbström (grubba@infovav.se)
//.
//. Synopsis:	Rule priority specification.
//.
//. +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//.
//. Specifies the priority and associativity of a rule.
//.

//. + value
//.   Priority value
int value;

//. + assoc
//.   Associativity
//.
//.   -1 - left
//.    0 - none
//.   +1 - right
int assoc;

//. - create
//.   Create a new priority object.
//. > p
//.   Priority.
//. > a
//.   Associativity.
void create(int p, int a)
{
  value = p;
  assoc = a;
}
