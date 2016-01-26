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
#include <stdint.h>


static unsigned long x=132456789, y=362436069, z=521288629;
static uint32_t seed = 0x666;

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

uint32_t random(void)
{
    int32_t hi, lo, x;

    // the algorithm used in avr-libc 1.6.4
    x = seed;
    if (x == 0) x = 123459876;
    hi = x / 127773;
    lo = x % 127773;
    x = 16807 * lo - 2836 * hi;
    if (x < 0) x += 0x7FFFFFFF;
    seed = x;
    return x;
}

