/*
 * $Id$
 *
 * Rule priority specification
 *
 * Henrik Grubbström 1996-12-05
 */

/* Priority value */
int value;

/* Associativity
 *
 * -1 - left
 *  0 - none
 * +1 - right
 */
int assoc;

void create(int p, int a)
{
  value = p;
  assoc = a;
}
