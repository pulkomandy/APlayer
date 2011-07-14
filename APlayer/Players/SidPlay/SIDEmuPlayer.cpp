/******************************************************************************/
/* SIDEmuPlayer implementation file.                                          */
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
#include "SID6581.h"
#include "SID6510.h"
#include "SIDEmuPlayer.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDEmuPlayer::SIDEmuPlayer(SID6581 *sid, SID6510 *cpu)
{
	// Initialize member variables
	sid6581 = sid;
	sid6510 = cpu;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDEmuPlayer::~SIDEmuPlayer(void)
{
}



/******************************************************************************/
/* EmuInitializeSong()                                                        */
/******************************************************************************/
bool SIDEmuPlayer::EmuInitializeSong(SIDEmuEngine &thisEmuEngine, SIDTune &thisTune, uint16 songNumber)
{
	sidEmuConfig config;

	// Do a regular song initialization
	bool ret = EmuInitializeSongOld(thisEmuEngine, thisTune, songNumber);

	// Get emulator configuration
	thisEmuEngine.GetConfig(config);

	if (ret && (config.digiPlayerScans != 0))
	{
		// Run the music player for a couple of player calls and check for
		// changes in the PlaySID extended SID registers. If no digis are
		// used, apply a higher amplification on each SID voice. First
		// check also covers writings of the player INIT routine. Old values
        // are stored before song INIT
		bool useDigis = false;
		int32 loops = config.digiPlayerScans;
		while (loops)
		{
			for (int32 i = 0; i < sidNumberOfC64Addr; i++)
			{
				if (oldValues[i] != sid6510->c64Mem2[c64AddrTable[i]])
				{
					useDigis = true;
					break;
				}

				oldValues[i] = sid6510->c64Mem2[c64AddrTable[i]];
			}

			if (useDigis)
				break;

			uint16 replayPC = thisTune.info.playAddr;

			// playRamRom was set by sidEmuInitializeSongOld(..)
			if (replayPC == 0)
			{
				sid6581->playRamRom = sid6510->c64Mem1[1];
				if ((sid6581->playRamRom & 2) != 0)	// Is Kernal?
					replayPC = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem1 + 0x0314)));	// IRQ
				else
					replayPC = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(sid6510->c64Mem1 + 0xfffe)));	// NMI
			}

			sid6510->Interpreter(replayPC, sid6581->playRamRom, 0, 0, 0);
			loops--;
		};

		thisEmuEngine.AmplifyThreeVoiceTunes(!useDigis);

		// Here re-init song to start at beginning
		ret = EmuInitializeSongOld(thisEmuEngine, thisTune, songNumber);
	}

	return (ret);
}



/******************************************************************************/
/* EmuInitializeSongOld()                                                     */
/******************************************************************************/
bool SIDEmuPlayer::EmuInitializeSongOld(SIDEmuEngine &thisEmuEngine, SIDTune &thisTune, uint16 songNumber)
{
	if (!thisEmuEngine.GetStatus() || !thisTune.status)
		return (false);
	else
	{
		// ------------------------------------------- Determine clock/speed
		sidEmuConfig config;

		// Get speed/clock setting for song and preselect
		// speed/clock descriptions strings, reg = song init akkumulator
		uint8 reg = thisTune.SelectSong(songNumber) - 1;

		thisEmuEngine.GetConfig(config);

		uint8 the_clock = thisTune.info.clockSpeed;

		// Choose clock speed emu user prefers. If sidtune doesn't
		// contain clock speed setting (=0), use emu setting.
		if (the_clock == SIDTUNE_CLOCK_ANY)
			the_clock &= config.clockSpeed;
		else
		{
			if (the_clock == 0)
				the_clock = config.clockSpeed;
		}

		uint8 the_speed = thisTune.info.songSpeed;

		if (config.forceSongSpeed)
			the_clock = config.clockSpeed;

		// Substitute correct VBI frequency
		if ((the_clock == SIDTUNE_CLOCK_PAL) && (the_speed == SIDTUNE_SPEED_VBI))
			the_speed = FREQ_VBI_PAL;

		if ((the_clock == SIDTUNE_CLOCK_NTSC) && (the_speed == SIDTUNE_SPEED_VBI))
			the_speed = FREQ_VBI_NTSC;

		// Transfer the speed settings to the emulator engine.
		// From here on we don't touch the SID clock speed setting
		sid6581->EmuConfigureClock(the_clock);
		sid6581->EmuSetReplayingSpeed(the_clock, the_speed);

		// Make available chosen speed setting is userspace
		thisTune.info.clockSpeed = the_clock;
		thisTune.info.songSpeed  = the_speed;

		// ------------------------------------------------------------------

		thisEmuEngine.MPUReset();

		if (!thisTune.PlaceSidTuneInC64Mem(thisEmuEngine.MPUReturnRAMBase()))
			return (false);

		if (thisTune.info.musPlayer)
			thisTune.MUS_InstallPlayer(thisEmuEngine.MPUReturnRAMBase());

		thisEmuEngine.AmplifyThreeVoiceTunes(false);	// Assume digis are used
		thisEmuEngine.Reset();

		if (config.digiPlayerScans != 0)
		{
			// Save the SID registers to allow later comparison
			for (int32 i = 0; i < sidNumberOfC64Addr; i++)
				oldValues[i] = sid6510->c64Mem2[c64AddrTable[i]];
		}

		// In PlaySID-mode the interpreter will ignore some of the parameters
		sid6510->Interpreter(thisTune.info.initAddr, sid6510->C64MemRamRom(thisTune.info.initAddr), reg, reg, reg);
		sid6581->playRamRom = sid6510->C64MemRamRom(thisTune.info.playAddr);

		// This code is only used to be able to print out the initial IRQ address
		if (thisTune.info.playAddr == 0)
		{
			// Get the address of the interrupt handler
			if ((sid6510->c64Mem1[1] & 2) != 0)		// Is Kernal?
				thisTune.SetIRQAddress(P_LENDIAN_TO_HOST_INT16(*((uint16 *)&sid6510->c64Mem1[0x0314])));	// IRQ
			else
				thisTune.SetIRQAddress(P_LENDIAN_TO_HOST_INT16(*((uint16 *)&sid6510->c64Mem1[0xfffe])));	// NMI
		}
		else
			thisTune.SetIRQAddress(0);

		thisEmuEngine.ResetSecondsThisSong();
		return (true);
	}
}
