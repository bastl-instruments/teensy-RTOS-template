/**
* @brief 
* @file compat.cpp
* @author J.H. 
* @date 2015-11-24
*/

/* module header */
#include "compat.h"

/* system includes C */

/* system includes C++ */
#include <avr_emulation.h>


/* local includes */

DDRBemulation DDRB;
DDRDemulation DDRD;

extern "C" {
int _getpid(){ return -1;}
int _kill(int pid, int sig){ return -1; }
int _write(){return -1;}

void yield()
{}


}
