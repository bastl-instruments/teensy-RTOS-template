/**
* @brief 
* @file random.cpp
* @author J.H. 
* @date 2015-08-07
*/

/* module header */
#include "random.h"

/* system includes C */

/* system includes C++ */

/* local includes */


static unsigned long x=132456789, y=362436069, z=521288629;

static unsigned long xorshift96()
{ //period 2^96-1
  // static unsigned long x=123456789, y=362436069, z=521288629;
  unsigned long t;

  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;

  return z;
}

int xor_rand(int maxval)
{
  return (int) (((xorshift96() & 0xFFFF) * maxval)>>16);
}
