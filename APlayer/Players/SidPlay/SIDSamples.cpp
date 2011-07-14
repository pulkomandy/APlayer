/******************************************************************************/
/* SIDSamples implementation file.                                            */
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
#include "SIDSamples.h"


/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
static const int8 sampleConvertTab[16] =
{
	0x81, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};


static const int8 galwayNoiseTab1[16] =
{
	0x80, 0x91, 0xa2, 0xb3, 0xc4, 0xd5, 0xe6, 0xf7,
	0x08, 0x19, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x7f
};



/******************************************************************************/
/* Sample Order Modes                                                         */
/******************************************************************************/
#define SO_LOWHIGH				0
#define SO_HIGHLOW				1



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDSamples::SIDSamples(SID6581 *sid, SID6510 *cpu)
{
	// Initialize member variables
	sid6581 = sid;
	sid6510 = cpu;

	sampleEmuRout = SampleEmuSilence;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDSamples::~SIDSamples(void)
{
}



/******************************************************************************/
/* SampleEmuInit()                                                            */
/******************************************************************************/
void SIDSamples::SampleEmuInit(void)
{
	SampleEmuReset();
}



/******************************************************************************/
/* SampleEmuReset()                                                           */
/******************************************************************************/
void SIDSamples::SampleEmuReset(void)
{
	ChannelReset(ch4);
	ChannelReset(ch5);

	sampleClock   = (uint32)(((sid6581->c64_clockSpeed / 2.0f) / sid6581->pcmFreq) * 65536UL);
	sampleEmuRout = SampleEmuSilence;

	if (sid6510->c64Mem2 != NULL)
	{
		ChannelFree(ch4, 0xd400);
		ChannelFree(ch5, 0xd500);
	}
}



/******************************************************************************/
/* SampleEmuCheckForInit()                                                    */
/******************************************************************************/
void SIDSamples::SampleEmuCheckForInit(void)
{
	// Try first sample channel
	switch (sid6510->c64Mem2[0xd41d])
	{
		case 0xFF:
		case 0xFE:
			ChannelTryInit(ch4, 0xd400);
			break;

		case 0xFD:
			ChannelFree(ch4, 0xd400);
			break;

		case 0xFC:
			ChannelTryInit(ch4, 0xd400);
			break;

		case 0x00:
			break;

		default:
			GalwayInit();
			break;
	}

	if (ch4.mode == FM_HUELSON)
		sampleEmuRout = SampleEmu;

	// Try second sample channel. No 'Galway Noise' allowed on
	// second channel
	switch (sid6510->c64Mem2[0xd51d])
	{
		case 0xFF:
		case 0xFE:
			ChannelTryInit(ch5, 0xd500);
			break;

		case 0xFD:
			ChannelFree(ch5, 0xd500);
			break;

		case 0xFC:
			ChannelTryInit(ch5, 0xd500);
			break;

		default:
			break;
	}

	if (ch5.mode == FM_HUELSON)
		sampleEmuRout = SampleEmuTwo;

	SampleEmuTryStopAll();
}



/******************************************************************************/
/* ChannelReset()                                                             */
/******************************************************************************/
void SIDSamples::ChannelReset(sidSampleChannel &ch)
{
	ch.active     = false;
	ch.mode       = FM_NONE;
	ch.period     = 0;
	ch.pos_stp.l  = 0;
	ch.galLastVol = 4;
}



/******************************************************************************/
/* ChannelFree()                                                              */
/******************************************************************************/
inline void SIDSamples::ChannelFree(sidSampleChannel &ch, const uint16 regBase)
{
	ch.active = false;
	ch.mode   = FM_NONE;
	sid6510->c64Mem2[regBase + 0x1d] = 0x00;
}



/******************************************************************************/
/* ChannelTryInit()                                                           */
/******************************************************************************/
inline void SIDSamples::ChannelTryInit(sidSampleChannel &ch, const uint16 regBase)
{
	if (ch.active && (ch.mode == FM_GALWAYON))
		return;

	ch.volShift = (0 - (int8)sid6510->c64Mem2[regBase + 0x1d]) >> 1;
	sid6510->c64Mem2[regBase + 0x1d] = 0x00;

	ch.address = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + regBase + 0x1e)));
	ch.endAddr = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + regBase + 0x3d)));
	if (ch.endAddr <= ch.address)
		return;

	ch.repeat      = sid6510->c64Mem2[regBase + 0x3f];
	ch.repAddr     = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + regBase + 0x7e)));
	ch.sampleOrder = sid6510->c64Mem2[regBase + 0x7d];

	uint16 tempPeriod = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + regBase + 0x5d)));
	if ((ch.scale = sid6510->c64Mem2[regBase + 0x5f]) != 0)
		tempPeriod >>= ch.scale;

	if (tempPeriod == 0)
	{
		ch.period    = 0;
		ch.pos_stp.l = (ch.posAdd_stp.l = 0);
		ch.active    = false;
		ch.mode      = FM_NONE;
	}
	else
	{
		if (tempPeriod != ch.period)
		{
			ch.period    = tempPeriod;
			ch.pos_stp.l = sampleClock / ch.period;
		}

		ch.posAdd_stp.l = 0;
		ch.active       = true;
		ch.mode         = FM_HUELSON;
	}
}



/******************************************************************************/
/* ChannelProcess()                                                           */
/******************************************************************************/
inline uint8 SIDSamples::ChannelProcess(sidSampleChannel &ch, const uint16 regBase)
{
	uint16 sampleIndex = ch.posAdd_stp.w[HI] + ch.address;

	if (sampleIndex >= ch.endAddr)
	{
		if (ch.repeat != 0xFF)
		{
			if (ch.repeat != 0)
				ch.repeat--;
			else
			{
				ChannelFree(ch, regBase);
				return (8);
			}
		}

		sampleIndex     = (ch.address = ch.repAddr);
		ch.posAdd_stp.l = 0;

		if (sampleIndex >= ch.endAddr)
		{
			ChannelFree(ch, regBase);
			return (8);
		}
	}

	uint8 tempSample = sid6510->c64Mem1[sampleIndex];
	if (ch.sampleOrder == SO_LOWHIGH)
	{
		if (ch.scale == 0)
		{
			if (ch.posAdd_stp.w[LO] >= 0x8000)
				tempSample >>= 4;
		}
		// AND 15 further below
	}
	else	// if (ch.SampleOrder == SO_HIGHLOW)
	{
		if (ch.scale == 0)
		{
			if (ch.posAdd_stp.w[LO] < 0x8000)
				tempSample >>= 4;
			// AND 15 further below
		}
		else	// if (ch.Scale != 0)
			tempSample >>= 4;
	}

	ch.posAdd_stp.l += ch.pos_stp.l;

	return (tempSample & 0x0F);
}



/******************************************************************************/
/* GalwayInit()                                                               */
/******************************************************************************/
void SIDSamples::GalwayInit(void)
{
	if (ch4.active)			// Don't interrupt active sound
		return;

	sampleEmuRout = SampleEmuSilence;

	ch4.counter              = sid6510->c64Mem2[0xd41d];
	sid6510->c64Mem2[0xd41d] = 0;

	if ((ch4.address = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + 0xd41e)))) == 0)
		return;

	if ((ch4.loopWait = sid6510->c64Mem2[0xd43f]) == 0)
		return;

	if ((ch4.nullWait = sid6510->c64Mem2[0xd45d]) == 0)
		return;

	uint8 add;
	if ((add = (sid6510->c64Mem2[0xd43e] & 15)) == 0)
		return;

	uint8 vol = ch4.galLastVol;
	for (int i = 0; i < 16; i++)
	{
		vol += add;
		galwayNoiseVolTab[i] = vol & 15;
		galwayNoiseSamTab[i] = galwayNoiseTab1[vol & 15];
	}

	if ((ch4.samLen = sid6510->c64Mem2[0xd43d]) == 0)
		return;

	ch4.galNextSam = ch4.samLen;

	ch4.active    = true;
	ch4.mode      = FM_GALWAYON;
	sampleEmuRout = GalwayReturnSample;

	ch4.pos_stp.l = 0;
	GetNextFour();
}



/******************************************************************************/
/* GetNextFour()                                                              */
/******************************************************************************/
inline void SIDSamples::GetNextFour(void)
{
	uint16 tempMul = (uint16)(sid6510->c64Mem1[ch4.address + (uint16)ch4.counter]) * ch4.loopWait + ch4.nullWait;
	ch4.counter--;

	if (tempMul != 0)
		ch4.period_stp.l = (sampleClock << 1) / tempMul;
	else
		ch4.period_stp.l = 0;

	ch4.period = tempMul;
}



/******************************************************************************/
/* SampleEmuTryStopAll()                                                      */
/******************************************************************************/
inline void SIDSamples::SampleEmuTryStopAll(void)
{
	if (!ch4.active && !ch5.active)
		sampleEmuRout = SampleEmuSilence;
}



/******************************************************************************/
/* SampleEmuSilence()                                                         */
/******************************************************************************/
int8 SIDSamples::SampleEmuSilence(SIDSamples *samp)
{
	return (0);
}



/******************************************************************************/
/* SampleEmu()                                                                */
/******************************************************************************/
int8 SIDSamples::SampleEmu(SIDSamples *samp)
{
	// One sample channel. Return signed 8-bit sample
	int8 sample;

	if (samp->ch4.active)
		sample = (sampleConvertTab[samp->ChannelProcess(samp->ch4, 0xd400)] >> samp->ch4.volShift);
	else
		sample = 0;

	return (sample);
}



/******************************************************************************/
/* SampleEmuTwo()                                                             */
/******************************************************************************/
int8 SIDSamples::SampleEmuTwo(SIDSamples *samp)
{
	// At most two sample channels. Return signed 8-bit sample.
	// Note: Implicit two-channel mixing
	int8 sample = 0;

	if (samp->ch4.active)
		sample += (sampleConvertTab[samp->ChannelProcess(samp->ch4, 0xd400)] >> samp->ch4.volShift);

	if (samp->ch5.active)
		sample += (sampleConvertTab[samp->ChannelProcess(samp->ch5, 0xd500)] >> samp->ch5.volShift);

	return (sample);
}



/******************************************************************************/
/* GalwayReturnSample()                                                       */
/******************************************************************************/
int8 SIDSamples::GalwayReturnSample(SIDSamples *samp)
{
	int8 tempSample      = samp->galwayNoiseSamTab[samp->ch4.pos_stp.w[HI] & 15];
	samp->ch4.galLastVol = samp->galwayNoiseVolTab[samp->ch4.pos_stp.w[HI] & 15];

	samp->ch4.pos_stp.l += samp->ch4.period_stp.l;

	if (samp->ch4.pos_stp.w[HI] >= samp->ch4.galNextSam)
	{
		samp->ch4.galNextSam += samp->ch4.samLen;

		if (samp->ch4.counter == 0xff)
		{
			samp->ch4.active    = false;
			samp->ch4.mode      = FM_GALWAYOFF;
			samp->sampleEmuRout = SampleEmuSilence;

			// Check for any new sound that has been set up meanwhile
			samp->SampleEmuCheckForInit();
		}
		else
			samp->GetNextFour();
	}

	return (tempSample);
}
