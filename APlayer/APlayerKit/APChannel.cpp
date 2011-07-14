/******************************************************************************/
/* APChannel implementation file.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_APLAYER_LIBRARY_

// PolyKit headers
#include "POS.h"
#include "PDebug.h"

// APlayerKit headers
#include "APChannel.h"


/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
APChannel::APChannel(void)
{
	flags         = 0;
	sampAddress   = NULL;
	sampStart     = 0;
	sampLength    = 0;
	loopStart     = 0;
	loopLength    = 0;
	releaseLength = 0;
	frequency     = 0;
	volume        = 0;
	leftVolume    = 0;
	rightVolume   = 0;
	panning       = 0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APChannel::~APChannel(void)
{
}



/******************************************************************************/
/* SetBuffer() will set the buffer to a sample. This function is used in      */
/*         sample playing.                                                    */
/*                                                                            */
/* Input:  "adr" is a pointer to the sample in memory.                        */
/*         "length" is the length of the buffer in samples.                   */
/******************************************************************************/
void APChannel::SetBuffer(const void *adr, uint32 length)
{
	ASSERT(adr != NULL);
	ASSERT(length > 0);

	sampAddress = adr;
	sampLength  = length;
}



/******************************************************************************/
/* PlaySample() will start to play the sample in the channel.                 */
/*                                                                            */
/* Input:  "adr" is a pointer to the sample in memory.                        */
/*         "startOffset" is the number of samples in the sample to start.     */
/*         "length" is the length in samples of the sample.                   */
/*         "bit" is the number of bit each sample each, e.g. 8 or 16.         */
/******************************************************************************/
void APChannel::PlaySample(const void *adr, uint32 startOffset, uint32 length, uint8 bit)
{
	ASSERT(adr != NULL);
	ASSERT(length > 0);
	ASSERT(startOffset <= length);
	ASSERT((bit == 8) || (bit == 16));

	sampAddress = adr;
	sampStart   = startOffset;
	sampLength  = length;
	flags      |= NP_TRIGIT;

	if (bit == 16)
		flags |= NP_16BIT;

	flags &= ~NP_MUTEIT;
}



/******************************************************************************/
/* SetLoop() will set the loop points in the sample.                          */
/*                                                                            */
/* Input:  "startOffset" is the number of samples in the sample to start.     */
/*         "length" is the length in samples to loop.                         */
/*         "type" is the type of the loop.                                    */
/******************************************************************************/
void APChannel::SetLoop(uint32 startOffset, uint32 length, AP_LoopType type)
{
	ASSERT(startOffset <= sampLength);
	ASSERT(startOffset + length <= sampLength);

	SetLoop(sampAddress, startOffset, length, type);
}



/******************************************************************************/
/* SetLoop() will set the loop points and change the sample.                  */
/*                                                                            */
/* Input:  "adr" is a pointer to the sample in memory.                        */
/*         "startOffset" is the number of samples in the sample to start.     */
/*         "length" is the length in samples to loop.                         */
/*         "type" is the type of the loop.                                    */
/******************************************************************************/
void APChannel::SetLoop(const void *adr, uint32 startOffset, uint32 length, AP_LoopType type)
{
	ASSERT(adr != NULL);
	ASSERT(length > 0);
	ASSERT((type == APLOOP_Normal) || (type == APLOOP_PingPong) || (type == APLOOP_Retrig));

	loopAddress = adr;
	loopStart   = startOffset;
	loopLength  = length;
	flags      |= NP_LOOP;

	if (type == APLOOP_PingPong)
		flags |= NP_PINGPONG;
	else
	{
		if (type == APLOOP_Retrig)
			flags |= NP_RETRIGLOOP;
	}

	flags &= ~NP_MUTEIT;
}



/******************************************************************************/
/* PlayReleasePart() will start to play the release part of the sample.       */
/*                                                                            */
/* Input:  "adr" is a pointer to the sample in memory.                        */
/*         "length" is the length in samples to play.                         */
/******************************************************************************/
void APChannel::PlayReleasePart(const void *adr, uint32 length)
{
	ASSERT(adr != NULL);
	ASSERT(length > 0);

	loopAddress   = adr;
	releaseLength = length;
	flags        |= NP_RELEASE;

	flags &= ~NP_MUTEIT;
}



/******************************************************************************/
/* SetVolume() will change the volume.                                        */
/*                                                                            */
/* Input:  "vol" is the new volume.                                           */
/******************************************************************************/
void APChannel::SetVolume(uint16 vol)
{
	ASSERT(vol <= 256);

	volume = vol;
	flags |= NP_VOLUME;
	flags &= ~NP_SPEAKERVOLUME;
}



/******************************************************************************/
/* SetLeftVolume() will change the volume in the left speaker.                */
/*                                                                            */
/* Input:  "vol" is the new volume.                                           */
/******************************************************************************/
void APChannel::SetLeftVolume(uint16 vol)
{
	ASSERT(vol <= 256);

	// If the right volume hasn't been set, set it to zero
	if (!(flags & NP_SPEAKERVOLUME))
		rightVolume = 0;

	leftVolume = vol;
	flags     |= NP_SPEAKERVOLUME;
}



/******************************************************************************/
/* SetRightVolume() will change the volume in the right speaker.              */
/*                                                                            */
/* Input:  "vol" is the new volume.                                           */
/******************************************************************************/
void APChannel::SetRightVolume(uint16 vol)
{
	ASSERT(vol <= 256);

	// If the left volume hasn't been set, set it to zero
	if (!(flags & NP_SPEAKERVOLUME))
		leftVolume = 0;

	rightVolume = vol;
	flags      |= NP_SPEAKERVOLUME;
}



/******************************************************************************/
/* SetPanning() will change the panning.                                      */
/*                                                                            */
/* Input:  "pan" is the new panning.                                          */
/******************************************************************************/
void APChannel::SetPanning(uint16 pan)
{
	ASSERT((pan <= 256) || (pan == APPAN_SURROUND));

	panning = pan;
	flags  |= NP_PANNING;
}



/******************************************************************************/
/* SetFrequency() will change the frequency.                                  */
/*                                                                            */
/* Input:  "freq" is the new frequency.                                       */
/******************************************************************************/
void APChannel::SetFrequency(uint32 freq)
{
	frequency = freq;
	flags    |= NP_FREQUENCY;
}



/******************************************************************************/
/* SetAmigaPeriod() will change the frequency.                                */
/*                                                                            */
/* Input:  "period" is the new frequency in Amiga period.                     */
/******************************************************************************/
void APChannel::SetAmigaPeriod(uint32 period)
{
	frequency = (period != 0) ? (3546895 / period) : 0;
	flags    |= NP_FREQUENCY;
}



/******************************************************************************/
/* IsActive() returns true or false depending on the channel is in use.       */
/*                                                                            */
/* Output: True if the channel is in use, else false.                         */
/******************************************************************************/
bool APChannel::IsActive(void) const
{
	return ((flags & NP_ACTIVE) ? true : false);
}



/******************************************************************************/
/* MuteIt() mute the channel.                                                 */
/******************************************************************************/
void APChannel::Mute(void)
{
	flags |= NP_MUTEIT;
}



/******************************************************************************/
/* GetVolume() returns the current volume on the channel.                     */
/*                                                                            */
/* Output: The volume.                                                        */
/******************************************************************************/
uint16 APChannel::GetVolume(void) const
{
	return (volume);
}



/******************************************************************************/
/* GetFrequency() returns the current frequency on the channel.               */
/*                                                                            */
/* Output: The frequency.                                                     */
/******************************************************************************/
uint32 APChannel::GetFrequency(void) const
{
	return (frequency);
}
