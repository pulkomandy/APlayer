/******************************************************************************/
/* SIDEnvelope header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDEnvelope_h
#define __SIDEnvelope_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* Envelope defines                                                           */
/******************************************************************************/
#define ENVE_STARTATTACK				0
#define ENVE_STARTRELEASE				2

#define ENVE_ATTACK						4
#define ENVE_DECAY						6
#define ENVE_SUSTAIN					8
#define ENVE_RELEASE					10
#define ENVE_SUSTAINDECAY				12
#define ENVE_MUTE						14

#define ENVE_SHORTATTACK				16

#define ENVE_ALTER						32



/******************************************************************************/
/* SIDEnvelope class                                                          */
/******************************************************************************/
class SIDOperator;

class SIDEnvelope
{
public:
	SIDEnvelope(void);
	virtual ~SIDEnvelope(void);

	void EnveEmuInit(uint32 updateFreq, bool measuredValues);

	static uint16 EnveEmuMute(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuStartAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuStartShortAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuShortAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterShortAttack(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuStartDecay(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuDecay(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterDecay(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuSustain(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuSustainDecay(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterSustain(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterSustainDecay(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuStartRelease(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuRelease(SIDOperator *pVoice, SIDEnvelope *enve);
	static uint16 EnveEmuAlterRelease(SIDOperator *pVoice, SIDEnvelope *enve);

	uint8 masterVolume;
	uint16 masterVolumeAmplIndex;

protected:
	inline void EnveEmuEnveAdvance(SIDOperator *pVoice);

	uint32 releaseTabLen;

	uint32 releasePos[256];
	uint16 masterAmplModTable[16 * 256];

	uint32 attackRates[16];
	uint32 decayReleaseRates[16];
};

#endif
