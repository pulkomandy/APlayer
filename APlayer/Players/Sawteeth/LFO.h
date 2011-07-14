/******************************************************************************/
/* LFO header file.                                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __LFO_h
#define __LFO_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "Sawteeth.h"


/******************************************************************************/
/* LFO class                                                                  */
/******************************************************************************/
class LFO
{
public:
	LFO(void);
	virtual ~LFO(void);

	void SetFreq(float freq);
	float Next(void);

protected:
	float curr;
	float step;
};

#endif
