/******************************************************************************/
/* APChannelParser implementation file.                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APChannel.h"
#include "APAddOns.h"

// Server headers
#include "APChannelParser.h"
#include "APMixerBase.h"


/******************************************************************************/
/* Default constructor                                                        */
/******************************************************************************/
APChannelParser::APChannelParser(void)
{
	// Initialize member variables
	privateFlags = 0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APChannelParser::~APChannelParser(void)
{
}



/******************************************************************************/
/* ParseSampleInfo() will parse the channel info as sample info and store the */
/*         result in the VINFO given.                                         */
/*                                                                            */
/* Input:  "info" is a pointer to the structure to fill out.                  */
/*         "clickBuffer" is the rampVol start value.                          */
/******************************************************************************/
void APChannelParser::ParseSampleInfo(VINFO *info, int32 clickBuffer)
{
	// Copy the information from the NotePlayer structure
	if ((sampAddress != NULL) && (sampLength != 0))
	{
		// Protect against clicks if volume variation is too high
		if (abs((int)info->leftVol - (int)leftVolume) > 32)
			info->rampVol = clickBuffer;

		if (flags & NP_SPEAKERVOLUME)
		{
			info->leftVol  = leftVolume;
			info->rightVol = rightVolume;
			info->flags   |= SF_SPEAKER;
		}
		else
		{
			info->leftVol  = volume;
			info->rightVol = volume;

			if (panning == APPAN_SURROUND)
				info->pan = PAN_SURROUND;
			else
			{
				// Protect against clicks if panning variation is too high
				if (abs((int)info->pan - (int)panning) > 48)
					info->rampVol = clickBuffer;

				info->pan = panning;
			}
		}

		info->adr        = sampAddress;
		info->start      = 0;
		info->size       = sampLength;
		info->repPos     = 0;
		info->repEnd     = 0;
		info->releaseLen = 0;
		info->frq        = samplePlayInfo.frequency;
		info->flags     &= SF_SPEAKER;
		info->kick       = true;

		if (samplePlayInfo.bitSize == 16)
			info->flags |= SF_16BITS;

		flags      &= ~NP_SPEAKERVOLUME;
		sampLength  = 0;
	}
}



/******************************************************************************/
/* ParseInfo() will parse the channel info and store the result in the VINFO  */
/*         given.                                                             */
/*                                                                            */
/* Input:  "info" is a pointer to the structure to fill out.                  */
/*         "clickBuffer" is the rampVol start value.                          */
/*                                                                            */
/* Output: The channel flags.                                                 */
/******************************************************************************/
uint32 APChannelParser::ParseInfo(VINFO *info, int32 clickBuffer)
{
	uint32 newFlags, privFlags, retFlags;
	uint16 infoFlags;

	// Get the channel flags
	newFlags  = flags;
	retFlags  = flags;
	privFlags = privateFlags;
	infoFlags = 0;

	if (newFlags & NP_TRIGIT)
	{
		privFlags |= NP_TRIGIT;
		privFlags &= ~NP_LOOP;
	}

	if (newFlags & NP_LOOP)
		privFlags |= NP_LOOP;

	if (newFlags & NP_RETRIGLOOP)
	{
		newFlags &= ~NP_RETRIGLOOP;
		retFlags &= ~NP_RETRIGLOOP;

		if (!(newFlags & NP_ACTIVE))		// Only retrig if the channel is not playing already
		{
			newFlags |= (NP_TRIGIT | NP_LOOP);
			retFlags |= (NP_TRIGIT | NP_LOOP);

			// Did we trig the sound with a normal "play" command?
			if (!(flags & NP_TRIGIT))
			{
				// No, then trig it
				privFlags  |= NP_TRIGIT;
				sampAddress = loopAddress;
				sampStart   = loopStart;
				sampLength  = loopLength;
			}
		}
	}

	// Speaker volume set?
	if (newFlags & NP_SPEAKERVOLUME)
	{
		// Protect against clicks if volume variation is too high
		if (abs((int)info->leftVol - (int)leftVolume) > 32)
			info->rampVol = clickBuffer;

		info->leftVol  = leftVolume;
		info->rightVol = rightVolume;
		newFlags      &= (~(NP_SPEAKERVOLUME | NP_VOLUME | NP_PANNING));
		retFlags      &= (~(NP_SPEAKERVOLUME | NP_VOLUME | NP_PANNING));
		infoFlags     |= SF_SPEAKER;
	}

	// Change the volume?
	if (newFlags & NP_VOLUME)
	{
		// Protect against clicks if volume variation is too high
		if (abs((int)info->leftVol - (int)volume) > 32)
			info->rampVol = clickBuffer;

		info->leftVol  = volume;
		info->rightVol = volume;
		newFlags      &= ~NP_VOLUME;
	}

	// Change the panning?
	if (newFlags & NP_PANNING)
	{
		if (panning == APPAN_SURROUND)
			info->pan = PAN_SURROUND;
		else
		{
			// Protect against clicks if panning variation is too high
			if (abs((int)info->pan - (int)panning) > 48)
				info->rampVol = clickBuffer;

			info->pan = panning;
		}

		newFlags &= ~NP_PANNING;
	}

	// Change the frequency?
	if (newFlags & NP_FREQUENCY)
	{
		info->frq = frequency;
		newFlags &= ~NP_FREQUENCY;
	}

	// Mute the channel?
	if (newFlags & NP_MUTEIT)
	{
		info->active = false;
		info->kick   = false;
	}
	else
	{
		if (sampAddress != NULL)
		{
			// Trig the sample to play from the start?
			if (newFlags & NP_TRIGIT)
			{
				info->adr        = sampAddress;
				info->start      = sampStart;
				info->size       = sampLength;
				info->repPos     = 0;
				info->repEnd     = 0;
				info->releaseLen = 0;
				info->kick       = true;
			}

			// Does the sample loop?
			if ((newFlags & NP_LOOP) && (loopLength > 4))
			{
				info->loopAdr = loopAddress;
				info->repPos  = loopStart;
				info->repEnd  = loopStart + loopLength;
				infoFlags    |= SF_LOOP;

				if (newFlags & NP_PINGPONG)
					infoFlags |= SF_BIDI;
			}

			// Special release command. Used in the Octalyzer player
			if (newFlags & NP_RELEASE)
			{
				info->loopAdr    = loopAddress;
				info->releaseLen = releaseLength;
				newFlags        &= ~NP_RELEASE;
			}

			if (newFlags & NP_16BIT)
				infoFlags |= SF_16BITS;
		}
	}

	// Store the flags back
	if (newFlags & ~NP_ACTIVE)
		info->flags = infoFlags;

	privateFlags = privFlags;
	flags        = 0;

	return (retFlags & ~NP_ACTIVE);
}



/******************************************************************************/
/* Active() will set the channel to active or inactive.                       */
/*                                                                            */
/* Input:  "active" set it to true to set the channel to active, else false.  */
/******************************************************************************/
void APChannelParser::Active(bool active)
{
	if (active)
		flags |= NP_ACTIVE;
	else
		flags &= ~NP_ACTIVE;
}



/******************************************************************************/
/* SetSampleInfo() sets the sample information.                               */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure.              */
/******************************************************************************/
void APChannelParser::SetSampleInfo(APSamplePlayerInfo *sampInfo)
{
	// Copy the structure
	samplePlayInfo.bitSize   = sampInfo->bitSize;
	samplePlayInfo.frequency = sampInfo->frequency;
}
