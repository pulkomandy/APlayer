/******************************************************************************/
/* SIDOperator header file.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDOperator_h
#define __SIDOperator_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDTune.h"
#include "SIDEnvelope.h"
#include "SID6510.h"


/******************************************************************************/
/* Function and type definations                                              */
/******************************************************************************/
class SIDOperator;
typedef int8 (*SidFunc)(SIDOperator *);
typedef uint16 (*SidUWordFunc)(SIDOperator *, SIDEnvelope *);
typedef void (*SidVoidFunc)(SIDOperator *);



/******************************************************************************/
/* SIDOperator class                                                          */
/******************************************************************************/
class SID6581;

class SIDOperator
{
public:
	SIDOperator(SID6581 *sid, SID6510 *cpu);
	virtual ~SIDOperator(void);

	void ClearSidOperator(void);
	void EnveEmuResetOperator(void);

	void SidEmuSet(uint16 sidIndex);
	void SidEmuSet2(void);

	static void SidMode00(SIDOperator *pVoice);
	static void SidMode10(SIDOperator *pVoice);
	static void SidMode20(SIDOperator *pVoice);
	static void SidMode30(SIDOperator *pVoice);
	static void SidMode40(SIDOperator *pVoice);
	static void SidMode50(SIDOperator *pVoice);
	static void SidMode60(SIDOperator *pVoice);
	static void SidMode70(SIDOperator *pVoice);
	static void SidMode80(SIDOperator *pVoice);
	static void SidMode80hp(SIDOperator *pVoice);
	static void SidModeLock(SIDOperator *pVoice);

	static void SidMode14(SIDOperator *pVoice);
	static void SidMode34(SIDOperator *pVoice);
	static void SidMode54(SIDOperator *pVoice);
	static void SidMode74(SIDOperator *pVoice);

	static int8 WaveCalcMute(SIDOperator *pVoice);
	static int8 WaveCalcNormal(SIDOperator *pVoice);
	static int8 WaveCalcRangeCheck(SIDOperator *pVoice);

	uint8 sidAD;
	uint8 sidSR;

	SIDOperator *carrier;
	SIDOperator *modulator;
	bool sync;

	uint8 output;
	uint8 outputMask;

	int8 filtVoiceMask;
	FilterFloat filtLow;
	FilterFloat filtRef;

	uint16 gainLeft;
	uint16 gainRight;		// Volume in highbyte
	uint16 gainSource;
	uint16 gainDest;
	uint16 gainLeftCentered;
	uint16 gainRightCentered;
	bool gainDirec;

	int32 cycleLenCount;

	SidFunc outProc;

	CPULWord waveStep;

	uint8 enveVol;

	uint8 adsrCtrl;
	bool gateOnCtrl;
	bool gateOffCtrl;
	SidUWordFunc adsrProc;

	CPULWord enveStep;
	CPULWord enveStepAdd;
	uint8 enveSusVol;
	uint16 enveShortAttackCount;

protected:
	struct sw_storage
	{
		uint16 len;
		uint32 stp;
	};

	inline void WaveCalcCycleLen(void);
	inline void WaveCalcFilter(void);
	inline void WaveAdvance(void);
	inline void NoiseAdvance(void);
	inline void NoiseAdvanceHp(void);

	SID6581 *sid6581;
	SID6510 *sid6510;

	uint32 sidFreq;
	uint16 sidPulseWidth;
	uint8 sidCtrl;

	uint16 pulseIndex;
	uint16 newPulseIndex;
	uint16 curSIDfreq;
	uint16 curNoiseFreq;

	bool filtEnabled;
	int8 filtIO;

	CPULWord cycleLen;
	CPULWord cycleAddLen;

	SidVoidFunc waveProc;

	CPULWord waveStepAdd;
	uint16 waveStepOld;
	struct sw_storage wavePre[2];

	CPULBWord noiseReg;
	uint32 noiseStep;
	uint32 noiseStepAdd;
	uint8 noiseOutput;
	bool noiseIsLocked;
};

#endif
