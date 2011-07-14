/******************************************************************************/
/* Scales header file.                                                        */
/******************************************************************************/


#ifndef __Scales_h
#define __Scales_h

#include "POS.h"


#define FromdB(x)	(exp((x) * 0.11512925f))


// The bark scale equations are approximations, since the original
// table was somewhat hand rolled.  The below are chosen to have the
// best possible fit to the rolled tables, thus their somewhat odd
// appearance (these are more accurate and over a longer range than
// the oft-quoted bark equations found in the texts I have).  The
// approximations are valid from 0 - 30kHz (nyquist) or so.
//
// all f in Hz, z in Bark
#define ToBark(n)	(13.1f * atan(0.00074f * (n)) + 2.24f * atan((n) * (n) * 1.85e-8f) + 1e-4f * (n))

#endif
