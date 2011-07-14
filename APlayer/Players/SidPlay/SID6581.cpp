/******************************************************************************/
/* SID6581 implementation file.                                               */
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
#include "SIDEmuEngine.h"
#include "SIDSamples.h"
#include "SIDOperator.h"
#include "SIDEnvelope.h"
#include "SID6510.h"
#include "SID6581.h"
#include "SIDWave6581.h"
#include "SIDWave8580.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/
#define lowPassParam			filterTable
#define apSpeed					0x4000



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SID6581::SID6581(SID6510 *cpu) : optr1(this, cpu), optr2(this, cpu), optr3(this, cpu), samples(this, cpu)
{
	// Initialize member variables
	ampMod1x8         = NULL;

	c64_clockSpeed    = 985248;
	c64_fClockSpeed   = 985248.4f;

	filterType        = 0;

	sid6510           = cpu;

	sidtuneClockSpeed = 985248;

	filterEnabled     = true;
	filterCurType     = 0;

	calls             = 50;
	fastForwardFactor = 128;

	// Initialize mode tables
	sidModeNormalTable[0]  = SIDOperator::SidMode00;
	sidModeNormalTable[1]  = SIDOperator::SidMode10;
	sidModeNormalTable[2]  = SIDOperator::SidMode20;
	sidModeNormalTable[3]  = SIDOperator::SidMode30;
	sidModeNormalTable[4]  = SIDOperator::SidMode40;
	sidModeNormalTable[5]  = SIDOperator::SidMode50;
	sidModeNormalTable[6]  = SIDOperator::SidMode60;
	sidModeNormalTable[7]  = SIDOperator::SidMode70;
	sidModeNormalTable[8]  = SIDOperator::SidMode80;
	sidModeNormalTable[9]  = SIDOperator::SidModeLock;
	sidModeNormalTable[10] = SIDOperator::SidModeLock;
	sidModeNormalTable[11] = SIDOperator::SidModeLock;
	sidModeNormalTable[12] = SIDOperator::SidModeLock;
	sidModeNormalTable[13] = SIDOperator::SidModeLock;
	sidModeNormalTable[14] = SIDOperator::SidModeLock;
	sidModeNormalTable[15] = SIDOperator::SidModeLock;

	sidModeRingTable[0]  = SIDOperator::SidMode00;
	sidModeRingTable[1]  = SIDOperator::SidMode14;
	sidModeRingTable[2]  = SIDOperator::SidMode00;
	sidModeRingTable[3]  = SIDOperator::SidMode34;
	sidModeRingTable[4]  = SIDOperator::SidMode00;
	sidModeRingTable[5]  = SIDOperator::SidMode54;
	sidModeRingTable[6]  = SIDOperator::SidMode00;
	sidModeRingTable[7]  = SIDOperator::SidMode74;
	sidModeRingTable[8]  = SIDOperator::SidModeLock;
	sidModeRingTable[9]  = SIDOperator::SidModeLock;
	sidModeRingTable[10] = SIDOperator::SidModeLock;
	sidModeRingTable[11] = SIDOperator::SidModeLock;
	sidModeRingTable[12] = SIDOperator::SidModeLock;
	sidModeRingTable[13] = SIDOperator::SidModeLock;
	sidModeRingTable[14] = SIDOperator::SidModeLock;
	sidModeRingTable[15] = SIDOperator::SidModeLock;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SID6581::~SID6581(void)
{
}



/******************************************************************************/
/* EmuResetAutoPanning()                                                      */
/******************************************************************************/
void SID6581::EmuResetAutoPanning(int32 autoPanning)
{
	doAutoPanning     = (autoPanning != SIDEMU_NONE);
	updateAutoPanning = false;
	apCount           = 0;

	// Auto-panning see EmuSet(). Reset volume levels to default
	if (doAutoPanning)
	{
		optr1.gainLeft          = (optr1.gainSource = 0xa080);
		optr1.gainRight         = (optr1.gainDest = 0x2080);
		optr1.gainDirec         = (optr1.gainLeft > optr1.gainRight);
		optr1.gainLeftCentered  = 0x8080;		// Middle
		optr1.gainRightCentered = 0x7f80;

		optr2.gainLeft          = (optr2.gainSource = 0x2080);	// This one mirrored
		optr2.gainRight         = (optr2.gainDest = 0xa080);
		optr2.gainDirec         = (optr2.gainLeft > optr2.gainRight);
		optr2.gainLeftCentered  = 0x8080;		// Middle
		optr2.gainRightCentered = 0x7f80;

		optr3.gainLeft          = (optr3.gainSource = 0xa080);
		optr3.gainRight         = (optr3.gainDest = 0x2080);
		optr3.gainDirec         = (optr3.gainLeft > optr3.gainRight);
		optr3.gainLeftCentered  = 0x8080;		// Middle
		optr3.gainRightCentered = 0x7f80;

		voice4_gainLeft  = 0x8080;	// Middle, not moving
		voice4_gainRight = 0x7f80;
	}
}



/******************************************************************************/
/* EmuSetVoiceVolume()                                                        */
/******************************************************************************/
void SID6581::EmuSetVoiceVolume(int32 voice, uint16 leftLevel, uint16 rightLevel, uint16 total)
{
	leftLevel   *= total;
	leftLevel  >>= 8;
	rightLevel  *= total;
	rightLevel >>= 8;

	uint16 centeredLeftLevel  = (0x80 * total) >> 8;
	uint16 centeredRightLevel = (0x7f * total) >> 8;

	// Signed 8-bit samples will be added to base array index.
	// So middle must be 0x80.
	// [-80,-81,...,-FE,-FF,0,1,...,7E,7F]
	uint16 leftIndex         = 0x0080 + (leftLevel << 8);
	uint16 rightIndex        = 0x0080 + (rightLevel << 8);
	uint16 gainLeftCentered  = 0x0080 + (centeredLeftLevel << 8);
	uint16 gainRightCentered = 0x0080 + (centeredRightLevel << 8);

	switch (voice)
	{
		case 1:
		{
			optr1.gainLeft  = leftIndex;
			optr1.gainRight = rightIndex;
			//
			optr1.gainSource        = leftIndex;
			optr1.gainDest          = rightIndex;
			optr1.gainLeftCentered  = gainLeftCentered;
			optr1.gainRightCentered = gainRightCentered;
			optr1.gainDirec         = (optr1.gainLeft > optr1.gainDest);
			break;
		}

		case 2:
		{
			optr2.gainLeft  = leftIndex;
			optr2.gainRight = rightIndex;
			//
			optr2.gainSource        = leftIndex;
			optr2.gainDest          = rightIndex;
			optr2.gainLeftCentered  = gainLeftCentered;
			optr2.gainRightCentered = gainRightCentered;
			optr2.gainDirec         = (optr2.gainLeft > optr2.gainDest);
			break;
		}

		case 3:
		{
			optr3.gainLeft  = leftIndex;
			optr3.gainRight = rightIndex;
			//
			optr3.gainSource        = leftIndex;
			optr3.gainDest          = rightIndex;
			optr3.gainLeftCentered  = gainLeftCentered;
			optr3.gainRightCentered = gainRightCentered;
			optr3.gainDirec         = (optr3.gainLeft > optr3.gainDest);
			break;
		}

		case 4:
		{
			voice4_gainLeft  = leftIndex;
			voice4_gainRight = rightIndex;
			break;
		}
	}
}



/******************************************************************************/
/* EmuConfigureClock()                                                        */
/******************************************************************************/
void SID6581::EmuConfigureClock(int32 clockSpeed)
{
	EmuSetClockSpeed(clockSpeed);

	pcmSid      = (uint32)(pcmFreq * (16777216.0f / c64_fClockSpeed));
	pcmSidNoise = (uint32)((c64_fClockSpeed * 256.0f) / pcmFreq);

	EmuChangeReplayingSpeed();
	samples.SampleEmuInit();
}



/******************************************************************************/
/* EmuConfigure()                                                             */
/******************************************************************************/
void SID6581::EmuConfigure(uint32 pcmFrequency, bool measuredEnveValues, bool isNewSID, bool emulateFilter, int32 clockSpeed)
{
	pcmFreq = pcmFrequency;
	EmuConfigureClock(clockSpeed);

	filterEnabled = emulateFilter;
	InitWaveformTables(isNewSID);

	envelope.EnveEmuInit(pcmFreq, measuredEnveValues);
}



/******************************************************************************/
/* EmuReset()                                                                 */
/******************************************************************************/
bool SID6581::EmuReset(void)
{
	optr1.ClearSidOperator();
	optr1.EnveEmuResetOperator();
	optr2.ClearSidOperator();
	optr2.EnveEmuResetOperator();
	optr3.ClearSidOperator();
	optr3.EnveEmuResetOperator();

	optr1.modulator     = &optr3;
	optr3.carrier       = &optr1;
	optr1.filtVoiceMask = 1;

	optr2.modulator     = &optr1;
	optr1.carrier       = &optr2;
	optr2.filtVoiceMask = 2;

	optr3.modulator     = &optr2;
	optr2.carrier       = &optr3;
	optr3.filtVoiceMask = 4;

	// Used for detecting changes of the GATE-bit (aka KEY-bit).
	// 6510-interpreter clears these before each call
	sid6510->sidKeysOff[4] = (sid6510->sidKeysOff[4 + 7] = (sid6510->sidKeysOff[4 + 14] = false));
	sid6510->sidKeysOn[4]  = (sid6510->sidKeysOn[4 + 7] = (sid6510->sidKeysOn[4 + 14] = false));

	samples.SampleEmuReset();

	filterType  = (filterCurType = 0);
	filterValue = 0;
	filterDy    = (filterResDy = 0);

	toFill         = 0;
	prevBufferLen  = (scaledBufferLen = 0);

	return (true);
}



/******************************************************************************/
/* EmuSetReplayingSpeed()                                                     */
/******************************************************************************/
void SID6581::EmuSetReplayingSpeed(int32 clockMode, uint16 callsPerSec)
{
	switch (clockMode)
	{
		case SIDTUNE_CLOCK_NTSC:
		{
			sidtuneClockSpeed = 1022727;
			timer = (defaultTimer = 0x4295);
			break;
		}

		case SIDTUNE_CLOCK_PAL:
		default:
		{
			sidtuneClockSpeed = 985248;
			timer = (defaultTimer = 0x4025);
			break;
		}
	}

	switch (callsPerSec)
	{
		case SIDTUNE_SPEED_CIA_1A:
		{
			timer = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + 0xdc04)));
			if (timer < 16)		// Prevent overflow
				timer = defaultTimer;

			calls = (((sidtuneClockSpeed << 1) / timer) + 1) >> 1;
			break;
		}

		default:
		{
			calls = callsPerSec;
			break;
		}
	}

	CalcValuesPerCall();
}



/******************************************************************************/
/* EmuUpdateReplayingSpeed()                                                  */
/******************************************************************************/
void SID6581::EmuUpdateReplayingSpeed(void)
{
	if (timer != P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + 0xdc04))))
	{
		timer = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem2 + 0xdc04)));

		// Prevent overflow
		if (timer < 16)
			timer = defaultTimer;

		calls = (((sidtuneClockSpeed << 1) / timer) + 1) >> 1;
		CalcValuesPerCall();
	}
}



/******************************************************************************/
/* EmuFillBuffer()                                                            */
/******************************************************************************/
void SID6581::EmuFillBuffer(SIDEmuEngine &thisEmu, SIDTune &thisTune, void *buffer, uint32 bufferLen)
{
	// Ensure a sane status of the whole emulator
	if (thisEmu.GetStatus() && thisTune.status)
	{
		sidEmuConfig config;

		// Get configuration
		thisEmu.GetConfig(config);

		// Both, 16-bit and stereo samples take more memory.
		// Hence fewer samples fit into the buffer
		bufferLen >>= bufferScale;

		// Split sample buffer into pieces for # voices:
		// splitBufferLen * bytesPerSample * voices = bufferLen
		if (config.volumeControl == SIDEMU_HWMIXING)
		{
			bufferLen >>= 2;	// or /4
			thisEmu.splitBufferLen = bufferLen;
		}

		if (prevBufferLen != bufferLen)
		{
			prevBufferLen = bufferLen;
			scaledBufferLen = (bufferLen << 7) / fastForwardFactor;
		}

		thisEmu.bytesCountTotal += bufferLen;
		thisEmu.bytesCountSong  += scaledBufferLen;

		while (thisEmu.bytesCountTotal >= config.frequency)
		{
			thisEmu.bytesCountTotal -= config.frequency;
			thisEmu.secondsTotal++;
		}

		while (thisEmu.bytesCountSong >= config.frequency)
		{
			thisEmu.bytesCountSong -= config.frequency;
			thisEmu.secondsThisSong++;
		}

		while (bufferLen > 0)
		{
			if (toFill > bufferLen)
			{
				buffer    = (thisEmu.sidEmuFillFunc)(&thisEmu, buffer, bufferLen);
				toFill   -= bufferLen;
				bufferLen = 0;
			}
			else if (toFill > 0)
			{
				buffer     = (thisEmu.sidEmuFillFunc)(&thisEmu, buffer, toFill);
				bufferLen -= toFill;
				toFill     = 0;
			}

			if (toFill == 0)
			{
				sid6510->optr3ReadWave = optr3.output;
				sid6510->optr3ReadEnve = optr3.enveVol;

				uint16 replayPC = thisTune.GetPlayAddr();

				// playRamRom was set by external player interface
				if (replayPC == 0)
				{
					playRamRom = sid6510->c64Mem1[1];
					if ((playRamRom & 2) != 0)	// Is Kernal?
						replayPC = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem1 + 0x0314)));	// IRQ
					else
						replayPC = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem1 + 0xfffe)));	// NMI
				}

				sid6510->Interpreter(replayPC, playRamRom, 0, 0, 0);

				if (thisTune.GetSongSpeed() == SIDTUNE_SPEED_CIA_1A)
					EmuUpdateReplayingSpeed();

				envelope.masterVolume          = (sid6510->c64Mem2[0xd418] & 15);
				envelope.masterVolumeAmplIndex = envelope.masterVolume << 8;

				optr1.gateOnCtrl  = sid6510->sidKeysOn[4];
				optr1.gateOffCtrl = sid6510->sidKeysOff[4];
				optr1.SidEmuSet(0xd400);

				optr2.gateOnCtrl  = sid6510->sidKeysOn[4 + 7];
				optr2.gateOffCtrl = sid6510->sidKeysOff[4 + 7];
				optr2.SidEmuSet(0xd407);

				optr3.gateOnCtrl  = sid6510->sidKeysOn[4 + 14];
				optr3.gateOffCtrl = sid6510->sidKeysOff[4 + 14];
				optr3.SidEmuSet(0xd40e);

				if ((sid6510->c64Mem2[0xd418] & 0x80) && ((sid6510->c64Mem2[0xd417] & optr3.filtVoiceMask) == 0))
					optr3.outputMask = 0;		// Off
				else
					optr3.outputMask = 0xff;	// On

				filterType = sid6510->c64Mem2[0xd418] & 0x70;
				if (filterType != filterCurType)
				{
					filterCurType = filterType;
					optr1.filtLow = (optr1.filtRef = 0);
					optr2.filtLow = (optr2.filtRef = 0);
					optr3.filtLow = (optr3.filtRef = 0);
				}

				if (filterEnabled)
				{
					filterValue = 0x7ff & ((sid6510->c64Mem2[0xd415] & 7) | ((uint16)sid6510->c64Mem2[0xd416] << 3));
					if (filterType == 0x20)
						filterDy = bandPassParam[filterValue];
					else
						filterDy = lowPassParam[filterValue];

					filterResDy = filterResTable[sid6510->c64Mem2[0xd417] >> 4] - filterDy;
					if (filterResDy < 1.0f)
						filterResDy = 1.0f;
				}

				optr1.SidEmuSet2();
				optr2.SidEmuSet2();
				optr3.SidEmuSet2();

				samples.SampleEmuCheckForInit();

				valuesAdd.w[HI] = 0;
				valuesAdd.l += values.l;
				toFill = valuesAdd.w[HI];

				// Decide whether to update/start auto-panning
				if ((apCount += timer) >= apSpeed)
				{
					apCount -= apSpeed;
					updateAutoPanning = true;
				}
				else
					updateAutoPanning = false;
			}
		}
	}
}



/******************************************************************************/
/* CalcValuesPerCall()                                                        */
/******************************************************************************/
inline void SID6581::CalcValuesPerCall(void)
{
	uint32 fastForwardFreq = pcmFreq;

	if (fastForwardFactor != 128)
		fastForwardFreq = (pcmFreq * fastForwardFactor) >> 7;	// Divide by 128

	values.l    = (valuesOrg.l = (((fastForwardFreq << 12) / calls) << 4));
	valuesAdd.l = 0;
}



/******************************************************************************/
/* EmuSetClockSpeed()                                                         */
/*                                                                            */
/* PAL:  Clock speed: 985248.4 Hz                                             */
/*       CIA 1 Timer A: $4025 (60 Hz)                                         */
/*                                                                            */
/* NTSC: Clock speed: 1022727.14 Hz                                           */
/*       CIA 1 Timer A: $4295 (60 Hz)                                         */
/******************************************************************************/
void SID6581::EmuSetClockSpeed(int32 clockMode)
{
	switch (clockMode)
	{
		case SIDTUNE_CLOCK_NTSC:
		{
			c64_clockSpeed  = 1022727;
			c64_fClockSpeed = 1022727.14f;
			break;
		}

		case SIDTUNE_CLOCK_PAL:
		default:
		{
			c64_clockSpeed  = 985248;
			c64_fClockSpeed = 985248.4f;
			break;
		}
	}
}



/******************************************************************************/
/* EmuChangeReplayingSpeed()                                                  */
/******************************************************************************/
void SID6581::EmuChangeReplayingSpeed(void)
{
	CalcValuesPerCall();
}



/******************************************************************************/
/* InitWaveformTables()                                                       */
/******************************************************************************/
void SID6581::InitWaveformTables(bool isNewSID)
{
	int32 i, j;
	uint16 k;

	k = 0;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 8; j++)
			triangleTable[k++] = i;
	}

	for (i = 255; i >= 0; i--)
	{
		for (j = 0; j < 8; j++)
			triangleTable[k++] = i;
	}

	k = 0;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 16; j++)
			sawtoothTable[k++] = i;
	}

	k = 0;
	for (i = 0; i < 4096; i++)
		squareTable[k++] = 0;

	for (i = 0; i < 4096; i++)
		squareTable[k++] = 255;

	if (isNewSID)
	{
		waveform30 = waveform30_8580;
		waveform50 = waveform50_8580;
		waveform60 = waveform60_8580;
		waveform70 = waveform70_8580;
	}
	else
	{
		waveform30 = waveform30_6581;
		waveform50 = waveform50_6581;
		waveform60 = waveform60_6581;
		waveform70 = waveform70_6581;	// Really audible?
	}

	for (i = 4096; i < 8192; i++)
	{
		waveform50[i] = 0;
		waveform60[i] = 0;
		waveform70[i] = 0;
	}

	if (isNewSID)
	{
		sidModeNormalTable[3] = SIDOperator::SidMode30;
		sidModeNormalTable[6] = SIDOperator::SidMode60;
		sidModeNormalTable[7] = SIDOperator::SidMode70;
		sidModeRingTable[7]   = SIDOperator::SidMode74;
	}
	else
	{
		sidModeNormalTable[3] = SIDOperator::SidMode30;
		sidModeNormalTable[6] = SIDOperator::SidMode60;
		sidModeNormalTable[7] = SIDOperator::SidMode00;	// Really audible?
		sidModeRingTable[7]   = SIDOperator::SidMode00;	//
	}

	uint32 ni;
	for (ni = 0; ni < sizeof(noiseTableLSB); ni++)
	{
		noiseTableLSB[ni] = (uint8)
			(((ni >> (7 - 2)) & 0x04) |
			 ((ni >> (4 - 1)) & 0x02) |
			 ((ni >> (2 - 0)) & 0x01));
	}

	for (ni = 0; ni < sizeof(noiseTableMID); ni++)
	{
		noiseTableMID[ni] = (uint8)
			(((ni >> (13 - 8 - 4)) & 0x10) |
			 ((ni << (3 - (11 - 8))) & 0x08));
	}

	for (ni = 0; ni < sizeof(noiseTableMSB); ni++)
	{
		noiseTableMSB[ni] = (uint8)
			(((ni << (7 - (22 - 16))) & 0x80) |
			 ((ni << (6 - (20 - 16))) & 0x40) |
			 ((ni << (5 - (16 - 16))) & 0x20));
	}
}
