/******************************************************************************/
/* LFO Interface.                                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"

// Player headers
#include "Sawteeth.h"
#include "LFO.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
LFO::LFO(void)
{
	curr = 0.0f;
	step = 6.28 * 0.00002267573696145124;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
LFO::~LFO(void)
{
}



/******************************************************************************/
/* SetFreq()                                                                  */
/******************************************************************************/
void LFO::SetFreq(float freq)
{
	step = freq * (6.28 * 0.00002267573696145124);
}



/******************************************************************************/
/* Next()                                                                     */
/******************************************************************************/
float LFO::Next(void)
{
	return (cosf(curr += step));
}
