/******************************************************************************/
/* SIDSamples header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SIDSamples_h
#define __SIDSamples_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDTune.h"


/******************************************************************************/
/* Sample emulator function                                                   */
/******************************************************************************/
class SIDSamples;
typedef int8 (*SampleEmuRout)(SIDSamples *);



/******************************************************************************/
/* sidSampleChannel structure                                                 */
/******************************************************************************/
typedef struct {
	bool active;
	int8 mode;
	uint8 repeat;
	uint8 scale;
	uint8 sampleOrder;
	int8 volShift;

	uint16 address;
	uint16 endAddr;
	uint16 repAddr;

	uint8 counter;		// Galway
	uint8 galLastVol;
	uint16 samLen;
	uint16 galNextSam;
	uint16 loopWait;
	uint16 nullWait;

	uint16 period;
	CPULWord period_stp;
	CPULWord pos_stp;
	CPULWord posAdd_stp;
} sidSampleChannel;



/******************************************************************************/
/* SIDSamples class                                                           */
/******************************************************************************/
class SID6581;
class SID6510;

class SIDSamples
{
public:
	SIDSamples(SID6581 *sid, SID6510 *cpu);
	virtual ~SIDSamples(void);

	void SampleEmuInit(void);
	void SampleEmuReset(void);
	void SampleEmuCheckForInit(void);

	SampleEmuRout sampleEmuRout;

protected:
	enum
	{
		FM_NONE,
		FM_GALWAYON,
		FM_GALWAYOFF,
		FM_HUELSON,
		FM_HUELSOFF
	};

	void ChannelReset(sidSampleChannel &ch);

	inline void ChannelFree(sidSampleChannel &ch, const uint16 regBase);
	inline void ChannelTryInit(sidSampleChannel &ch, const uint16 regBase);
	inline uint8 ChannelProcess(sidSampleChannel &ch, const uint16 regBase);

	void GalwayInit(void);
	inline void GetNextFour(void);

	inline void SampleEmuTryStopAll(void);

	static int8 SampleEmuSilence(SIDSamples *samp);
	static int8 SampleEmu(SIDSamples *samp);
	static int8 SampleEmuTwo(SIDSamples *samp);
	static int8 GalwayReturnSample(SIDSamples *samp);

	SID6581 *sid6581;
	SID6510 *sid6510;

	uint8 galwayNoiseVolTab[16];
	int8 galwayNoiseSamTab[16];

	uint32 sampleClock;

	sidSampleChannel ch4;
	sidSampleChannel ch5;
};

#endif
