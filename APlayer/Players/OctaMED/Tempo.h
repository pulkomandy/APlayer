/******************************************************************************/
/* Tempo header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Tempo_h
#define __Tempo_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "LimVar.h"


/******************************************************************************/
/* Tempo class                                                                */
/******************************************************************************/
class Tempo
{
public:
	Tempo(void);
	virtual ~Tempo(void);

	LimVar<uint16, 1, 240> tempo;
	LimVar<uint16, 1, 32> ticksPerLine;
	LimVar<uint16, 1, 32> linesPerBeat;
	bool bpm;
};

#endif
