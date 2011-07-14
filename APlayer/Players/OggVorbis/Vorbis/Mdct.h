/******************************************************************************/
/* Mdct header file.                                                          */
/******************************************************************************/


#ifndef __Mdct_h
#define __Mdct_h

#include "POS.h"


#define DATA_TYPE			float
#define REG_TYPE			float
#define cPI3_8				0.38268343236508977175f
#define cPI2_8				0.70710678118654752441f
#define cPI1_8				0.92387953251128675613f

#define FLOAT_CONV(x)		(x)
#define MULT_NORM(x)		(x)
#define HALVE(x)			((x) * 0.5f)



typedef struct MdctLookup
{
	int32 n;
	int32 log2n;

	DATA_TYPE *trig;
	int32 *bitRev;

	DATA_TYPE scale;
} MdctLookup;

#endif
