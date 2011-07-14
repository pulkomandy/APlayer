/******************************************************************************/
/* Tempo Interface.                                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "LimVar.h"
#include "Tempo.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Tempo::Tempo(void)
{
	tempo        = 125;
	ticksPerLine = 6;
	linesPerBeat = 4;
	bpm          = true;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Tempo::~Tempo(void)
{
}
