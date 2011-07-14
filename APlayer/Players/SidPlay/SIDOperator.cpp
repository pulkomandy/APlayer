/******************************************************************************/
/* SIDOperator implementation file.                                           */
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
#include "SIDTune.h"
#include "SID6581.h"
#include "SID6510.h"
#include "SIDEnvelope.h"
#include "SIDOperator.h"


/******************************************************************************/
/* External tables                                                            */
/******************************************************************************/
extern const uint8 masterVolumeLevels[16];
extern const SidUWordFunc enveModeTable[];



/******************************************************************************/
/* Waveform macros                                                            */
/******************************************************************************/
#define triangle pVoice->sid6581->triangleTable[pVoice->waveStep.w[HI]]
#define sawtooth pVoice->sid6581->sawtoothTable[pVoice->waveStep.w[HI]]
#define square pVoice->sid6581->squareTable[pVoice->waveStep.w[HI] + pVoice->pulseIndex]
#define triSaw pVoice->sid6581->waveform30[pVoice->waveStep.w[HI]]
#define triSquare pVoice->sid6581->waveform50[pVoice->waveStep.w[HI] + pVoice->sidPulseWidth]
#define sawSquare pVoice->sid6581->waveform60[pVoice->waveStep.w[HI] + pVoice->sidPulseWidth]
#define triSawSquare pVoice->sid6581->waveform70[pVoice->waveStep.w[HI] + pVoice->sidPulseWidth]



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDOperator::SIDOperator(SID6581 *sid, SID6510 *cpu)
{
	sid6581 = sid;
	sid6510 = cpu;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDOperator::~SIDOperator(void)
{
}



/******************************************************************************/
/* ClearSidOperator()                                                         */
/******************************************************************************/
void SIDOperator::ClearSidOperator(void)
{
	sidFreq = 0;
	sidCtrl = 0;
	sidAD   = 0;
	sidSR   = 0;

	sync = false;

	pulseIndex = (newPulseIndex = (sidPulseWidth = 0));
	curSIDfreq = (curNoiseFreq = 0);

	output     = (noiseOutput = 0);
	outputMask = 0xff;	// On
	filtIO     = 0;

	filtEnabled = false;
	filtLow     = (filtRef = 0);

	cycleLenCount = 0;
	cycleLen.l    = (cycleAddLen.l = 0);

	outProc = &WaveCalcMute;

	waveStepAdd.l  = (waveStep.l = 0);
	wavePre[0].len = (wavePre[0].stp = 0);
	wavePre[1].len = (wavePre[1].stp = 0);
	waveStepOld    = 0;

	noiseReg.l    = sidNoiseSeed;
	noiseStepAdd  = (noiseStep = 0);
	noiseIsLocked = false;
}



/******************************************************************************/
/* EnveEmuResetOperator()                                                     */
/******************************************************************************/
void SIDOperator::EnveEmuResetOperator(void)
{
	// Mute, end of R-phase
	adsrCtrl             = ENVE_MUTE;
	gateOnCtrl           = (gateOffCtrl = false);

	enveStep.l           = (enveStepAdd.l = 0);
	enveSusVol           = 0;
	enveVol              = 0;
	enveShortAttackCount = 0;
}



/******************************************************************************/
/* SidEmuSet()                                                                */
/******************************************************************************/
void SIDOperator::SidEmuSet(uint16 sidIndex)
{
	sidFreq = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + sidIndex)));

	sidPulseWidth = (P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + sidIndex + 2))) & 0x0FFF);
	newPulseIndex = 4096 - sidPulseWidth;

	if (((waveStep.w[HI] + pulseIndex) >= 0x1000) && ((waveStep.w[HI] + newPulseIndex) >= 0x1000))
		pulseIndex = newPulseIndex;
	else if (((waveStep.w[HI] + pulseIndex) < 0x1000) && ((waveStep.w[HI] + newPulseIndex) < 0x1000))
		pulseIndex = newPulseIndex;

	uint8 enveTemp, newWave, oldWave;

	oldWave  = sidCtrl;
	enveTemp = adsrCtrl;
	sidCtrl  = (newWave = sid6510->c64Mem2[sidIndex + 4]);

	if ((newWave & 1) ==0)
	{
		if ((oldWave & 1) !=0)
			enveTemp = ENVE_STARTRELEASE;
	}
	else if (gateOffCtrl || ((oldWave & 1) == 0))
	{
		enveTemp = ENVE_STARTATTACK;
		if (sid6581->doAutoPanning && sid6581->updateAutoPanning)
		{
			// Swap source/destination position
			uint16 tmp = gainSource;
			gainSource = gainDest;
			gainDest   = tmp;

			if ((gainDest ^ gainSource) == 0)
			{
				// Mute voice
				gainLeft = (gainRight = 0x0000 + 0x80);
			}
			else
			{
				// Start from middle position
				gainLeft  = gainLeftCentered;
				gainRight = gainRightCentered;
			}

			// Determine direction
			// true  = L > R : L down, R up
			// false = L < R : L up, R down
			gainDirec = (gainLeft > gainDest);
		}
	}

	if (sid6581->doAutoPanning && sid6581->updateAutoPanning && (enveTemp != ENVE_STARTATTACK))
	{
		if (gainDirec)
		{
			if (gainLeft > gainDest)
			{
				gainLeft  -= 0x0100;
				gainRight += 0x0100;
			}
			else
			{
				// Swap source/destination position
				uint16 tmp = gainSource;
				gainSource = gainDest;
				gainDest   = tmp;

				// Inverse direction
				gainDirec = false;
			}
		}
		else
		{
			if (gainRight > gainSource)
			{
				gainLeft  += 0x0100;
				gainRight -= 0x0100;
			}
			else
			{
				gainDirec = true;

				// Swap source/destination position
				uint16 tmp = gainSource;
				gainSource = gainDest;

				// Inverse direction
				gainDest = tmp;
			}
		}
	}

	if (((oldWave ^ newWave) & 0xF0) != 0)
		cycleLenCount = 0;

	uint8 adTemp = sid6510->c64Mem2[sidIndex + 5];
	uint8 srTemp = sid6510->c64Mem2[sidIndex + 6];

	if (sidAD != adTemp)
		enveTemp |= ENVE_ALTER;
	else if (sidSR != srTemp)
		enveTemp |= ENVE_ALTER;

	sidAD = adTemp;
	sidSR = srTemp;

	uint8 tmpSusVol = masterVolumeLevels[srTemp >> 4];
	if (adsrCtrl != ENVE_SUSTAIN)	// !!!
		enveSusVol = tmpSusVol;
	else
	{
		if (enveSusVol > enveVol)
			enveSusVol = 0;
		else
			enveSusVol = tmpSusVol;
	}

	adsrProc = enveModeTable[enveTemp >> 1];	// Shifting out the KEY-bit
	adsrCtrl = enveTemp & (255 - ENVE_ALTER - 1);

	filtEnabled = sid6581->filterEnabled && ((sid6510->c64Mem2[0xd417] & filtVoiceMask) != 0);
}



/******************************************************************************/
/* SidEmuSet2()                                                               */
/******************************************************************************/
void SIDOperator::SidEmuSet2(void)
{
	outProc = WaveCalcNormal;
	sync    = false;

	if ((sidFreq < 16) || ((sidCtrl & 8) != 0))
	{
		outProc = WaveCalcMute;

		if (sidFreq == 0)
		{
			cycleLen.l    = (cycleAddLen.l = 0);
			waveStep.l    = 0;
			curSIDfreq    = (curNoiseFreq = 0);
			noiseStepAdd  = 0;
			cycleLenCount = 0;
		}

		if ((sidCtrl & 8) != 0)
		{
			if (noiseIsLocked)
			{
				noiseIsLocked = false;
				noiseReg.l    = sidNoiseSeed;
			}
		}
	}
	else
	{
		if (curSIDfreq != sidFreq)
		{
			curSIDfreq = sidFreq;

			// We keep the value cycleLen between 1 <= x <= 65535.
			// This makes a range-check in WaveCalcCycleLen() unrequired
			cycleLen.l = ((sid6581->pcmSid << 12) / sidFreq) << 4;
			if (cycleLenCount > 0)
			{
				WaveCalcCycleLen();
				outProc = WaveCalcRangeCheck;
			}
		}

		if (((sidCtrl & 0x80) == 0x80) && (curNoiseFreq != sidFreq))
		{
			curNoiseFreq = sidFreq;
			noiseStepAdd = (sid6581->pcmSidNoise * sidFreq) >> 8;
			if (noiseStepAdd >= (1L << 21))
				sid6581->sidModeNormalTable[8] = SidMode80hp;
			else
				sid6581->sidModeNormalTable[8] = SidMode80;
		}

		if ((sidCtrl & 2) != 0)
		{
			if ((modulator->sidFreq == 0) || ((modulator->sidCtrl & 8) != 0))
			{
				;
			}
			else if (((carrier->sidCtrl & 2) != 0) && (modulator->sidFreq >= (sidFreq << 1)))
			{
				;
			}
			else
				sync = true;
		}

		if (((sidCtrl & 0x14) == 0x14) && (modulator->sidFreq != 0))
			waveProc = sid6581->sidModeRingTable[sidCtrl >> 4];
		else
			waveProc = sid6581->sidModeNormalTable[sidCtrl >> 4];
	}
}



/******************************************************************************/
/* SidMode00()                                                                */
/******************************************************************************/
void SIDOperator::SidMode00(SIDOperator *pVoice)
{
	pVoice->output = (pVoice->filtIO - 0x80);
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode10()                                                                */
/******************************************************************************/
void SIDOperator::SidMode10(SIDOperator *pVoice)
{
	pVoice->output = triangle;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode20()                                                                */
/******************************************************************************/
void SIDOperator::SidMode20(SIDOperator *pVoice)
{
	pVoice->output = sawtooth;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode30()                                                                */
/******************************************************************************/
void SIDOperator::SidMode30(SIDOperator *pVoice)
{
	pVoice->output = triSaw;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode40()                                                                */
/******************************************************************************/
void SIDOperator::SidMode40(SIDOperator *pVoice)
{
	pVoice->output = square;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode50()                                                                */
/******************************************************************************/
void SIDOperator::SidMode50(SIDOperator *pVoice)
{
	pVoice->output = triSquare;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode60()                                                                */
/******************************************************************************/
void SIDOperator::SidMode60(SIDOperator *pVoice)
{
	pVoice->output = sawSquare;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode70()                                                                */
/******************************************************************************/
void SIDOperator::SidMode70(SIDOperator *pVoice)
{
	pVoice->output = triSawSquare;
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode80()                                                                */
/******************************************************************************/
void SIDOperator::SidMode80(SIDOperator *pVoice)
{
	pVoice->output = pVoice->noiseOutput;
	pVoice->WaveAdvance();
	pVoice->NoiseAdvance();
}



/******************************************************************************/
/* SidMode80hp()                                                              */
/******************************************************************************/
void SIDOperator::SidMode80hp(SIDOperator *pVoice)
{
	pVoice->output = pVoice->noiseOutput;
	pVoice->WaveAdvance();
	pVoice->NoiseAdvanceHp();
}



/******************************************************************************/
/* SidModeLock()                                                              */
/******************************************************************************/
void SIDOperator::SidModeLock(SIDOperator *pVoice)
{
	pVoice->noiseIsLocked = true;
	pVoice->output = (pVoice->filtIO - 0x80);
	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode14()                                                                */
/******************************************************************************/
void SIDOperator::SidMode14(SIDOperator *pVoice)
{
	if (pVoice->modulator->waveStep.w[HI] < 2048)
		pVoice->output = triangle;
	else
		pVoice->output = 0xFF ^ triangle;

	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode34()                                                                */
/******************************************************************************/
void SIDOperator::SidMode34(SIDOperator *pVoice)
{
	if (pVoice->modulator->waveStep.w[HI] < 2048)
		pVoice->output = triSaw;
	else
		pVoice->output = 0xFF ^ triSaw;

	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode54()                                                                */
/******************************************************************************/
void SIDOperator::SidMode54(SIDOperator *pVoice)
{
	if (pVoice->modulator->waveStep.w[HI] < 2048)
		pVoice->output = triSquare;
	else
		pVoice->output = 0xFF ^ triSquare;

	pVoice->WaveAdvance();
}



/******************************************************************************/
/* SidMode74()                                                                */
/******************************************************************************/
void SIDOperator::SidMode74(SIDOperator *pVoice)
{
	if (pVoice->modulator->waveStep.w[HI] < 2048)
		pVoice->output = triSawSquare;
	else
		pVoice->output = 0xFF ^ triSawSquare;

	pVoice->WaveAdvance();
}



/******************************************************************************/
/* WaveCalcMute()                                                             */
/******************************************************************************/
int8 SIDOperator::WaveCalcMute(SIDOperator *pVoice)
{
	(*pVoice->adsrProc)(pVoice, &pVoice->sid6581->envelope);	// Just process envelope
	return (pVoice->filtIO & pVoice->outputMask);
}



/******************************************************************************/
/* WaveCalcNormal()                                                           */
/******************************************************************************/
int8 SIDOperator::WaveCalcNormal(SIDOperator *pVoice)
{
	if (pVoice->cycleLenCount <= 0)
	{
		pVoice->WaveCalcCycleLen();

		if ((pVoice->sidCtrl & 0x40) == 0x40)
		{
			pVoice->pulseIndex = pVoice->newPulseIndex;
			if (pVoice->pulseIndex > 2048)
				pVoice->waveStep.w[HI] = 0;
		}
	}

	(*pVoice->waveProc)(pVoice);

	pVoice->filtIO = pVoice->sid6581->ampMod1x8[(*pVoice->adsrProc)(pVoice, &pVoice->sid6581->envelope) | pVoice->output];
	pVoice->WaveCalcFilter();

	return (pVoice->filtIO & pVoice->outputMask);
}



/******************************************************************************/
/* WaveCalcRangeCheck()                                                       */
/******************************************************************************/
int8 SIDOperator::WaveCalcRangeCheck(SIDOperator *pVoice)
{
	pVoice->waveStepOld = pVoice->waveStep.w[HI];
	(*pVoice->waveProc)(pVoice);

	if (pVoice->waveStep.w[HI] < pVoice->waveStepOld)
	{
		// Next step switch back to normal calculation
		pVoice->cycleLenCount  = 0;
		pVoice->outProc        = WaveCalcNormal;
		pVoice->waveStep.w[HI] = 4095;
	}

	pVoice->filtIO = pVoice->sid6581->ampMod1x8[(*pVoice->adsrProc)(pVoice, &pVoice->sid6581->envelope) | pVoice->output];
	pVoice->WaveCalcFilter();

	return (pVoice->filtIO & pVoice->outputMask);
}



/******************************************************************************/
/* WaveCalcCycleLen()                                                         */
/******************************************************************************/
inline void SIDOperator::WaveCalcCycleLen(void)
{
	cycleAddLen.w[HI] = 0;
	cycleAddLen.l    += cycleLen.l;
	cycleLenCount     = cycleAddLen.w[HI];

	register uint16 diff = cycleLenCount - cycleLen.w[HI];

	if (wavePre[diff].len != cycleLenCount)
	{
		wavePre[diff].len = cycleLenCount;
		wavePre[diff].stp = (waveStepAdd.l = (4096UL * 65536UL) / cycleLenCount);
	}
	else
		waveStepAdd.l = wavePre[diff].stp;
}



/******************************************************************************/
/* WaveCalcFilter()                                                           */
/******************************************************************************/
inline void SIDOperator::WaveCalcFilter(void)
{
	if (filtEnabled)
	{
		if (sid6581->filterType != 0)
		{
			if (sid6581->filterType == 0x20)
			{
				filtLow        += (filtRef * sid6581->filterDy);
				FilterFloat tmp = (FilterFloat)filtIO - filtLow;
				tmp            -= filtRef * sid6581->filterResDy;

				filtRef += (tmp * (sid6581->filterDy));
				filtIO   = (int8)(filtRef - filtLow / 4);
			}
			else if (sid6581->filterType == 0x40)
			{
				filtLow        += (filtRef * sid6581->filterDy * 0.1f);
				FilterFloat tmp = (FilterFloat)filtIO - filtLow;
				tmp            -= filtRef * sid6581->filterResDy;

				filtRef         += (tmp * (sid6581->filterDy));
				FilterFloat tmp2 = filtRef - filtIO / 8;

				if (tmp2 < -128)
					tmp2 = -128;

				if (tmp2 > 127)
					tmp2 = 127;

				filtIO = (int8)tmp2;
			}
			else
			{
				filtLow            += (filtRef * sid6581->filterDy);
				FilterFloat sample  = filtIO;
				FilterFloat sample2 = sample - filtLow;
				int32 tmp           = (int32)sample2;
				sample2            -= filtRef * sid6581->filterResDy;
				filtRef            += (sample2 * sid6581->filterDy);

				if (sid6581->filterType == 0x10)
					filtIO = (int8)filtLow;
				else if (sid6581->filterType == 0x30)
					filtIO = (int8)filtLow;
				else if (sid6581->filterType == 0x50)
					filtIO = (int8)(sample - (tmp >> 1));
				else if (sid6581->filterType == 0x60)
					filtIO = (int8)tmp;
				else if (sid6581->filterType == 0x70)
					filtIO = (int8)(sample - (tmp >> 1));
			}
		}
		else	// filterType == 0x00
			filtIO = 0;
	}
}



/******************************************************************************/
/* WaveAdvance()                                                              */
/******************************************************************************/
inline void SIDOperator::WaveAdvance(void)
{
	waveStep.l     += waveStepAdd.l;
	waveStep.w[HI] &= 4095;
}



/******************************************************************************/
/* NoiseAdvance()                                                             */
/******************************************************************************/
inline void SIDOperator::NoiseAdvance(void)
{
	noiseStep += noiseStepAdd;
	if (noiseStep >= (1L << 20))
	{
		noiseStep  -= (1L << 20);
		noiseReg.l  = (noiseReg.l << 1) | (((noiseReg.l >> 22) ^ (noiseReg.l >> 17)) & 1);
		noiseOutput = (sid6581->noiseTableLSB[noiseReg.b[LOLO]] | sid6581->noiseTableMID[noiseReg.b[LOHI]] | sid6581->noiseTableMSB[noiseReg.b[HILO]]);
	}
}



/******************************************************************************/
/* NoiseAdvanceHp()                                                           */
/******************************************************************************/
inline void SIDOperator::NoiseAdvanceHp(void)
{
	uint32 tmp = noiseStepAdd;

	while (tmp >= (1L << 20))
	{
		tmp       -= (1L << 20);
		noiseReg.l = (noiseReg.l << 1) | (((noiseReg.l >> 22) ^ (noiseReg.l >> 17)) & 1);
	}

	noiseStep += tmp;

	if (noiseStep >= (1L << 20))
	{
		noiseStep -= (1L << 20);
		noiseReg.l = (noiseReg.l << 1) | (((noiseReg.l >> 22) ^ (noiseReg.l >> 17)) & 1);
	}

	noiseOutput = (sid6581->noiseTableLSB[noiseReg.b[LOLO]] | sid6581->noiseTableMID[noiseReg.b[LOHI]] | sid6581->noiseTableMSB[noiseReg.b[HILO]]);
}
