/******************************************************************************/
/* SID6581 header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SID6581_h
#define __SID6581_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDTune.h"
#include "SIDSamples.h"
#include "SIDOperator.h"
#include "SIDEnvelope.h"
#include "SID6510.h"


/******************************************************************************/
/* SID6581 class                                                              */
/******************************************************************************/
class SIDEmuEngine;

class SID6581
{
public:
	SID6581(SID6510 *cpu);
	virtual ~SID6581(void);

	void EmuResetAutoPanning(int32 autoPanning);
	void EmuSetVoiceVolume(int32 voice, uint16 leftLevel, uint16 rightLevel, uint16 total);
	void EmuConfigureClock(int32 clockSpeed);
	void EmuConfigure(uint32 pcmFrequency, bool measuredEnveValues, bool isNewSID, bool emulateFilter, int32 clockSpeed);
	bool EmuReset(void);
	void EmuSetReplayingSpeed(int32 clockMode, uint16 callsPerSec);
	void EmuFillBuffer(SIDEmuEngine &thisEmu, SIDTune &thisTune, void *buffer, uint32 bufferLen);

	int8 *ampMod1x8;

	uint32 pcmFreq;

	uint32 c64_clockSpeed;
	float c64_fClockSpeed;

	uint8 bufferScale;
	uint8 playRamRom;

	bool filterEnabled;
	FilterFloat filterDy;
	FilterFloat filterResDy;
	uint8 filterType;

	uint32 pcmSid;
	uint32 pcmSidNoise;

	SIDOperator optr1;
	SIDOperator optr2;
	SIDOperator optr3;

	SIDEnvelope envelope;
	SIDSamples samples;

	bool doAutoPanning;
	bool updateAutoPanning;

	FilterFloat filterTable[0x800];
	FilterFloat bandPassParam[0x800];
	FilterFloat filterResTable[16];

	uint8 *waveform30;
	uint8 *waveform50;
	uint8 *waveform60;
	uint8 *waveform70;

	uint8 triangleTable[4096];
	uint8 sawtoothTable[4096];
	uint8 squareTable[2 * 4096];

	uint8 noiseTableMSB[1 << 8];
	uint8 noiseTableMID[1 << 8];
	uint8 noiseTableLSB[1 << 8];

	SidVoidFunc sidModeNormalTable[16];	// MOS-8580, MOS-6581 (no 70)
	SidVoidFunc sidModeRingTable[16];	// MOS-8580, MOS-6581 (no 74)

protected:
	inline void CalcValuesPerCall(void);

	void EmuSetClockSpeed(int32 clockMode);
	void EmuChangeReplayingSpeed(void);
	void EmuUpdateReplayingSpeed(void);

	void InitWaveformTables(bool isNewSID);

	SID6510 *sid6510;

	uint32 sidtuneClockSpeed;	// Song clock speed (PAL or NTSC). Does not affect pitch.

	uint16 apCount;

	uint16 toFill;
	uint32 prevBufferLen;		// Need for fast_forward time count
	uint32 scaledBufferLen;

	// Voice 4 does not use a SIDOperator
	uint16 voice4_gainLeft;
	uint16 voice4_gainRight;

	uint8 filterCurType;
	uint16 filterValue;

	uint16 calls;				// Calls per second
	uint16 fastForwardFactor;	// Normal speed

	CPULWord values;
	CPULWord valuesAdd;
	CPULWord valuesOrg;

	uint16 timer;
	uint16 defaultTimer;
};

#endif
