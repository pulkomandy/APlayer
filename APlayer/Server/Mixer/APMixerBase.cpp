/******************************************************************************/
/* APlayer mixer base class.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"

// APlayerKit headers
#include "APAddOns.h"

// Server headers
#include "APApplication.h"
#include "APMixerBase.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APMixerBase::APMixerBase(void)
{
	// Initialize mixer variables
	vinf      = NULL;
	masterVol = 256;
	stereoSep = 128;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APMixerBase::~APMixerBase(void)
{
}



/******************************************************************************/
/* Initialize() will initialize global mixer stuff + local mixer stuff.       */
/*                                                                            */
/* Input:  "frequency" is the frequency to return the mixer data in.          */
/*         "channels" is the number of channels to allocate.                  */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APMixerBase::Initialize(uint32 frequency, uint16 channels)
{
	// Start to remember the arguments
	mixerFreq  = frequency;
	channelNum = channels;

	// Allocate and initialize the VINFO structures
	vinf = new VINFO[channels];
	if (vinf == NULL)
		throw PMemoryException();

	// Clear the voices
	ClearVoices();

	return (InitMixer());
}



/******************************************************************************/
/* ClearVoices() will stop any playing samples in the voices.                 */
/******************************************************************************/
void APMixerBase::ClearVoices(void)
{
	uint16 i;

	for (i = 0; i < channelNum; i++)
	{
		VINFO *inf = &vinf[i];

		inf->enabled    = true;
		inf->kick       = false;
		inf->active     = false;
		inf->flags      = 0;
		inf->adr        = NULL;
		inf->loopAdr    = NULL;
		inf->start      = 0;
		inf->size       = 0;
		inf->repPos     = 0;
		inf->repEnd     = 0;
		inf->releaseLen = 0;
		inf->frq        = 10000;
		inf->leftVol    = 0;
		inf->rightVol   = 0;
		inf->pan        = (((i & 3) == 0) || ((i & 3) == 3)) ? PAN_LEFT : PAN_RIGHT;
//		inf->click      = 0;
		inf->rampVol    = 0;
//		inf->lastValL   = 0;
//		inf->lastValR   = 0;
		inf->lVolSel    = 0;
		inf->rVolSel    = 0;
		inf->oldLVol    = 0;
		inf->oldRVol    = 0;
		inf->current    = 0;
		inf->increment  = 0;
	}
}



/******************************************************************************/
/* Cleanup() will cleanup all the mixer stuff.                                */
/******************************************************************************/
void APMixerBase::Cleanup(void)
{
	// Start to call the mixer cleanup routine
	EndMixer();

	// Deallocate the VINFO buffer
	delete[] vinf;
	vinf = NULL;
}



/******************************************************************************/
/* SetVolume() sets a new master volume.                                      */
/*                                                                            */
/* Input:  "volume" is the new volume between 0 and 256.                      */
/******************************************************************************/
void APMixerBase::SetVolume(uint16 volume)
{
	if (volume > 256)
		volume = 256;

	masterVol = volume;
}



/******************************************************************************/
/* SetStereoSeparation() sets a new stereo separation value.                  */
/*                                                                            */
/* Input:  "sep" is the new stereo separation in percent.                     */
/******************************************************************************/
void APMixerBase::SetStereoSeparation(uint16 sep)
{
	if (sep > 100)
		sep = 100;

	stereoSep = (sep * 128) / 100;
}



/******************************************************************************/
/* GetMixerChannels() returns a pointer to the mixer channels.                */
/*                                                                            */
/* Output: Is the pointer to the VINFO structures.                            */
/******************************************************************************/
VINFO *APMixerBase::GetMixerChannels(void)
{
	return (vinf);
}



/******************************************************************************/
/* IsActive() check to see if the given channel is active or not.             */
/*                                                                            */
/* Input:  "channel" is the channel to check.                                 */
/*                                                                            */
/* Output: True if the channel is active, false if not.                       */
/******************************************************************************/
bool APMixerBase::IsActive(uint16 channel)
{
	return (vinf[channel].active);
}



/******************************************************************************/
/* EnableChannel() enable or disable a channel.                               */
/*                                                                            */
/* Input:  "channel" is the channel to check.                                 */
/*         "enable" is true to enable the channel, false to disable it.       */
/******************************************************************************/
void APMixerBase::EnableChannel(uint16 channel, bool enable)
{
	vinf[channel].enabled = enable;
}



/******************************************************************************/
/* Mixing() is the main mixer function.                                       */
/*                                                                            */
/* Input:  "dest" is a pointer to write the mixed data into.                  */
/*         "todo" is the size of the buffer in sample pairs.                  */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
void APMixerBase::Mixing(int32 *dest, int32 todo, uint32 mode)
{
	// Just call the right mixer function
	DoMixing(dest, todo, mode);
}



/******************************************************************************/
/* AddEffects() adds mixer effects to the mixed data.                         */
/*                                                                            */
/* Input:  "dest" is a pointer to the buffer to add the effects on.           */
/*         "todo" is the number of samples to modify.                         */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
void APMixerBase::AddEffects(int32 *dest, int32 todo, uint32 mode)
{
	APAgent_DSP dsp;
	int32 i, count;
	AddOnInfo *info;

	// Prepare the argument structure
	dsp.buffer    = dest;
	dsp.todo      = todo;
	dsp.frequency = mixerFreq;
	dsp.stereo    = mode & DMODE_STEREO;

	// Lock the plug-in list
	GetApp()->pluginLock.WaitToRead();

	// Get the number of DSP agents
	count = GetApp()->dspAgents.CountItems();

	// Call each agent
	for (i = 0; i < count; i++)
	{
		info = GetApp()->dspAgents.GetItem(i);
		info->agent->Run(info->index, APPA_DSP, &dsp);
	}

	GetApp()->pluginLock.DoneReading();
}



/******************************************************************************/
/* ConvertMixedData() converts the mix buffer to the output format and store  */
/*      the result in the supplied buffer.                                    */
/*                                                                            */
/* Input:  "dest" is a pointer to the buffer to store the result in.          */
/*         "source" is a pointer to the buffer to take the samples from.      */
/*         "todo" is the size of the buffer in samples.                       */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
void APMixerBase::ConvertMixedData(int16 *dest, int32 *source, int32 todo, uint32 mode)
{
	// Convert the 32 bit buffer to 16 bit
	Mix32To16(dest, source, todo, mode);
}
