/******************************************************************************/
/* Player header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Player_h
#define __Player_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "Sawteeth.h"
#include "InsPly.h"


/******************************************************************************/
/* Player class                                                               */
/******************************************************************************/
class Player
{
public:
	Player(Sawteeth *s, Channel *chn, uint8 chanNum);
	virtual ~Player(void);

	void Init(void);

	bool NextBuffer(void);
	float *Buffer(void);

	bool Looped(void);
	uint32 GetSeqPos(void);

	void JumpPos(uint8 seqPos, uint8 stepPos, uint8 pal);

protected:
	uint8 myChannel;

	bool looped;
	bool tmpLoop;

	Sawteeth *song;
	ChStep *step;
	Channel *ch;
	InsPly *ip;

	Step *currStep;
	Part *currPart;
	uint32 seqCount;		// Step in sequencer
	uint8 stepC;			// Step in part

	uint32 nexts;			// PAL-screen counter

	float damp;
	float amp;
	float ampStep;

	float freq;
	float freqStep;
	float targetFreq;

	float cutOff;
	float cutOffStep;

	float *buffer;
};

#endif
