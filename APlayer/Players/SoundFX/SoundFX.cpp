/******************************************************************************/
/* SoundFX Player Interface.                                                  */
/*                                                                            */
/* Original player by Linel Software.                                         */
/* Converted to C++ by Thomas Neumann.                                        */
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
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "SoundFX.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.00f



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static int16 noteTable[] =
{
	1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076,
	1076, 1076, 1076, 1076, 1076, 1076, 1076, 1076,	1076, 1016,  960,  906,
	 856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,
	 428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
	 113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
	 113,  113,   -1
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SoundFX::SoundFX(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	patterns = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SoundFX::~SoundFX(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float SoundFX::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 SoundFX::GetSupportFlags(int32 index)
{
	return (appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString SoundFX::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_SFX_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString SoundFX::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_SFX_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString SoundFX::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_SFX_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a SoundFX 2.0 module.        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result SoundFX::ModuleCheck(int32 index, PFile *file)
{
	uint32 sampleSizes[31];
	uint32 total, fileSize;
	int8 i;

	// Check the module size
	fileSize = file->GetLength();
	if (fileSize < 144)
		return (AP_UNKNOWN);

	// Read the sample size tabel
	file->SeekToBegin();
	file->ReadArray_B_UINT32s(sampleSizes, 31);

	// Check the mark
	if (file->Read_B_UINT32() != 'SO31')
		return (AP_UNKNOWN);

	// Check the sample sizes
	total = 0;
	for (i = 0; i < 31; i++)
		total += sampleSizes[i];

	if (total > fileSize)
		return (AP_UNKNOWN);

	return (AP_OK);
}



/******************************************************************************/
/* LoadModule() will load the module into the memory.                         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result SoundFX::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	uint32 i;
	uint32 sampleSizes[31];
	ap_result retVal = AP_ERROR;

	try
	{
		// Read the sample size table
		file->ReadArray_B_UINT32s(sampleSizes, 31);

		// Skip the module mark
		file->Seek(4, PFile::pSeekCurrent);

		// Read the delay value
		delay = file->Read_B_UINT16();

		// Skip the pads
		file->Seek(14, PFile::pSeekCurrent);

		// Clear the sample information array
		memset(samples, 0, sizeof(samples));

		// Read the sample information
		for (i = 0; i < 31; i++)
		{
			file->ReadString(samples[i].name, 22);

			samples[i].length     = file->Read_B_UINT16() * 2;
			samples[i].volume     = file->Read_B_UINT16();
			samples[i].loopStart  = file->Read_B_UINT16();
			samples[i].loopLength = file->Read_B_UINT16() * 2;

			// Sample loop fix
			if ((samples[i].length != 0) && (samples[i].loopStart == samples[i].length))
				samples[i].length += samples[i].loopLength;

			// Adjust the sample length
			if (samples[i].loopLength > 2)
				samples[i].length = max(samples[i].length, samples[i].loopStart + samples[i].loopLength);

			// Volume fix
			if (samples[i].volume > 64)
				samples[i].volume = 64;

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_SFX_ERR_LOADING_HEADER);
				throw PUserException();
			}
		}

		// Read the song length
		songLength = file->Read_UINT8();
		file->Read_UINT8();

		// Read the orders
		file->Read(orders, 128);
		file->Seek(4, PFile::pSeekCurrent);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_SFX_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Find heighest pattern number
		maxPattern = 0;

		for (i = 0; i < songLength; i++)
		{
			if (orders[i] > maxPattern)
				maxPattern = orders[i];
		}

		maxPattern++;

		// Allocate pattern pointer table
		patterns = new uint32 *[maxPattern];
		if (patterns == NULL)
		{
			errorStr.LoadString(res, IDS_SFX_ERR_MEMORY);
			throw PUserException();
		}

		memset(patterns, 0, maxPattern * sizeof(uint32 *));

		// Allocate and load the patterns
		for (i = 0; i < maxPattern; i++)
		{
			// Allocate space to hold the pattern
			patterns[i] = new uint32[4 * 64];
			if (patterns[i] == NULL)
			{
				errorStr.LoadString(res, IDS_SFX_ERR_MEMORY);
				throw PUserException();
			}

			// Read the pattern data
			file->ReadArray_B_UINT32s(patterns[i], 4 * 64);

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_SFX_ERR_LOADING_PATTERNS);
				throw PUserException();
			}
		}

		// Now it's time to read the sample data
		for (i = 0; i < 31; i++)
		{
			int8 *sampData;
			int32 length;

			// Allocate space to hold the sample
			length = sampleSizes[i];

			if (length != 0)
			{
				sampData = new int8[length];
				if (sampData == NULL)
				{
					errorStr.LoadString(res, IDS_SFX_ERR_MEMORY);
					throw PUserException();
				}

				memset(sampData, 0, length);
				samples[i].sampleAdr = sampData;

				// Check the see if we miss too much from the last sample
				if (file->GetLength() - file->GetPosition() < (length - 512))
				{
					errorStr.LoadString(res, IDS_SFX_ERR_LOADING_SAMPLES);
					throw PUserException();
				}

				// Read the sample
				file->Read(sampData, length);
			}
		}

		// Ok, we're done
		retVal = AP_OK;
	}
	catch(PUserException e)
	{
		// Just delete the exception and clean up
		Cleanup();
	}
	catch(...)
	{
		// Clean up
		Cleanup();
		throw;
	}

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool SoundFX::InitPlayer(int32 index)
{
	uint32 i, j, k;
	uint32 *pattAdr;
	PosInfo posInfo;
	float rowTime;
	float total = 0.0f;
	bool pattBreak = false;

	// Calculate how long it take to play a single row
	rowTime = (1000.0f * 6.0f / (1773447 / delay / 2.5f));

	// Calculate the position times
	for (i = 0; i < songLength; i++)
	{
		// Add the position information to the list
		posInfo.time.SetTimeSpan(total);
		posInfoList.AddTail(posInfo);

		for (j = 0; j < 64 * 4; j += 4)
		{
			// Get pointer to next pattern
			pattAdr = patterns[orders[i]] + j;

			for (k = 0; k < 4; k++)
			{
				// Did we reach a pattern break?
				if ((pattAdr[k] & 0xffff0000) == 0xfffc0000)
					pattBreak = true;
			}

			// Add the row time
			total += rowTime;

			if (pattBreak)
			{
				pattBreak = false;
				break;
			}
		}
	}

	// Set the total time
	totalTime.SetTimeSpan(total);

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void SoundFX::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void SoundFX::InitSound(int32 index, uint16 songNum)
{
	// Initialize the variables
	memset(channelInfo, 0, sizeof(channelInfo));

	timer      = 0;
	trackPos   = 0;
	posCounter = 0;
	breakFlag  = false;

	// Calculate the frequency to play with
	playFreq = 1773447 / delay / 2.5f;
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void SoundFX::Play(void)
{
	timer++;
	if (timer == 6)
	{
		// Get the next pattern line
		timer = 0;
		PlaySound();
	}
	else
	{
		// Run any realtime effects
		MakeEffects(&channelInfo[0], virtChannels[0]);
		MakeEffects(&channelInfo[1], virtChannels[1]);
		MakeEffects(&channelInfo[2], virtChannels[2]);
		MakeEffects(&channelInfo[3], virtChannels[3]);
	}
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 SoundFX::GetSongLength(void)
{
	return (songLength);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 SoundFX::GetSongPosition(void)
{
	return (trackPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void SoundFX::SetSongPosition(int16 pos)
{
	// Change the position
	breakFlag  = false;
	posCounter = 0;
	trackPos   = pos;
}



/******************************************************************************/
/* GetTimeTable() will calculate the position time for each position and      */
/*      store them in the list given.                                         */
/*                                                                            */
/* Input:  "songNum" is the subsong number to get the time table for.         */
/*         "posTimes" is a reference to the list where you should store the   */
/*         start time for each position.                                      */
/*                                                                            */
/* Output: The total module time or 0 if time table is not supported.         */
/******************************************************************************/
PTimeSpan SoundFX::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	int32 i, count;

	// Copy the position times
	count = posInfoList.CountItems();
	for (i = 0; i < count; i++)
		posTimes.AddTail(posInfoList.GetItem(i).time);

	return (totalTime);
}



/******************************************************************************/
/* GetInfoString() returns the description and value string on the line       */
/*      given. If the line is out of range, false is returned.                */
/*                                                                            */
/* Input:  "line" is the line starting from 0.                                */
/*         "description" is a reference to where to store the description.    */
/*         "value" is a reference to where to store the value.                */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool SoundFX::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 4)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_SFX_INFODESCLINE0);
			value.SetUNumber(songLength);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_SFX_INFODESCLINE1);
			value.SetUNumber(maxPattern);
			break;
		}

		// Supported/Used Samples
		case 2:
		{
			description.LoadString(res, IDS_SFX_INFODESCLINE2);
			value = "31";
			break;
		}

		// Actual Speed (BPM)
		case 3:
		{
			description.LoadString(res, IDS_SFX_INFODESCLINE3);
			value.SetUNumber(GetBPMTempo());
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* GetSampleInfo() fills out the APSampleInfo structure given with the sample */
/*      information of the sample number given.                               */
/*                                                                            */
/* Input:  "num" is the sample number starting from 0.                        */
/*         "info" is a pointer to an APSampleInfo structure to fill out.      */
/*                                                                            */
/* Output: True if the information are returned, false if not.                */
/******************************************************************************/
bool SoundFX::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	PCharSet_Amiga charSet;
	Sample *sample;

	// First check the sample number for "out of range"
	if (num >= 31)
		return (false);

	// Get the pointer to the sample data
	sample = &samples[num];

	// Fill out the sample info structure
	info->name.SetString(sample->name, &charSet);
	info->type    = apSample;
	info->bitSize = 8;
	info->middleC = 8287;
	info->volume  = sample->volume * 4;
	info->panning = -1;
	info->address = sample->sampleAdr;
	info->length  = (sample->length > 2 ? sample->length : 0);

	if (sample->loopLength <= 2)
	{
		// No loop
		info->flags      = 0;
		info->loopStart  = 0;
		info->loopLength = 0;
	}
	else
	{
		// Sample loops
		info->flags      = APSAMP_LOOP;
		info->loopStart  = sample->loopStart;
		info->loopLength = sample->loopLength;
	}

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void SoundFX::Cleanup(void)
{
	uint32 i;

	// Free the patterns
	if (patterns != NULL)
	{
		for (i = 0; i < maxPattern; i++)
			delete[] patterns[i];
	}

	delete[] patterns;
	patterns = NULL;

	// Free the sample data
	for (i = 0; i < 31; i++)
	{
		delete[] samples[i].sampleAdr;
		samples[i].sampleAdr = NULL;
	}
}



/******************************************************************************/
/* PlaySound() gets the next pattern line and parse it.                       */
/******************************************************************************/
void SoundFX::PlaySound(void)
{
	uint32 *patternAdr;

	// Find the pattern address
	patternAdr = patterns[orders[trackPos]] + posCounter;

	// Parse the pattern data
	PlayNote(&channelInfo[0], virtChannels[0], patternAdr[0]);
	PlayNote(&channelInfo[1], virtChannels[1], patternAdr[1]);
	PlayNote(&channelInfo[2], virtChannels[2], patternAdr[2]);
	PlayNote(&channelInfo[3], virtChannels[3], patternAdr[3]);

	// Did we need to break the current pattern?
	if (breakFlag)
	{
		breakFlag  = false;
		posCounter = 4 * 63;
	}

	// Go to the next pattern line
	posCounter += 4;
	if (posCounter == 4 * 64)
	{
		// Okay, the pattern is done, go to the next pattern
		posCounter = 0;
		trackPos++;

		// Tell APlayer we have changed the position
		ChangePosition();

		if (trackPos == songLength)
		{
			// Module is done, loop it
			trackPos   = 0;
			endReached = true;
		}
	}
}



/******************************************************************************/
/* PlayNote() parse one channel pattern line.                                 */
/*                                                                            */
/* Input:   "channel" is a pointer to the intern channel structure.           */
/*          "virtChannel" is a pointer to the APlayer channel class.          */
/*          "patternData" is the pattern data to parse.                       */
/******************************************************************************/
void SoundFX::PlayNote(Channel *channel, APChannel *virtChannel, uint32 patternData)
{
	// Remember the pattern data
	channel->patternData = patternData;

	// If there is a PIC command, don't parse the pattern data
	if ((patternData & 0xffff0000) != 0xfffd0000)
	{
		// Get the sample number
		uint8 sampleNum = (patternData & 0x0000f000) >> 12;
		if (patternData & 0x10000000)
			sampleNum += 16;

		if (sampleNum != 0)
		{
			int16 volume;
			Sample *curSamp = &samples[sampleNum - 1];

			// Get the sample informations
			channel->sampleStart = curSamp->sampleAdr;
			channel->sampleLen   = curSamp->length;
			channel->volume      = curSamp->volume;
			channel->loopStart   = curSamp->loopStart;
			channel->loopLength  = curSamp->loopLength;

			// Get the current volume
			volume = channel->volume;

			// Any volume effects
			switch ((patternData & 0x00000f00) >> 8)
			{
				// Change volume up
				case 5:
				{
					volume += (patternData & 0x000000ff);
					if (volume > 64)
						volume = 64;

					break;
				}

				// Change volume down
				case 6:
				{
					volume -= (patternData & 0x000000ff);
					if (volume < 0)
						volume = 0;

					break;
				}
			}

			// Change the volume on the channel
			virtChannel->SetVolume(volume * 4);
		}
	}

	// Do we have a pattern PIC command?
	if ((patternData & 0xffff0000) == 0xfffd0000)
	{
		channel->patternData &= 0xffff0000;
		return;
	}

	if (!(patternData & 0xffff0000))
		return;

	// Stop stepping
	channel->stepValue = 0;

	// Get the period
	channel->currentNote = ((patternData & 0xffff0000) >> 16) & 0xefff;

	// Do we have a pattern STP command?
	if ((patternData & 0xffff0000) == 0xfffe0000)
	{
		virtChannel->Mute();
		return;
	}

	// Do we have a pattern BRK command?
	if ((patternData & 0xffff0000) == 0xfffc0000)
	{
		breakFlag = true;
		channel->patternData &= 0xefffffff;
		return;
	}

	// Do we have a pattern ??? command?
	if ((patternData & 0xffff0000) == 0xfffb0000)
	{
		channel->patternData &= 0xefffffff;
		return;
	}

	// Play the note
	if (channel->sampleStart != NULL)
	{
		virtChannel->PlaySample(channel->sampleStart, 0, channel->sampleLen);
		virtChannel->SetAmigaPeriod((channel->patternData & 0xefff0000) >> 16);

		if (channel->loopLength > 2)
			virtChannel->SetLoop(channel->loopStart, channel->loopLength);
	}
}



/******************************************************************************/
/* MakeEffects() runs all the realtime effects.                               */
/*                                                                            */
/* Input:   "channel" is a pointer to the intern channel structure.           */
/*          "virtChannel" is a pointer to the APlayer channel class.          */
/******************************************************************************/
void SoundFX::MakeEffects(Channel *channel, APChannel *virtChannel)
{
	if (channel->stepValue != 0)
	{
		// Well, we need to pitch step the note
		if (channel->stepValue < 0)
		{
			// Step it up
			channel->stepNote += channel->stepValue;

			if (channel->stepNote <= channel->stepEndNote)
			{
				channel->stepValue = 0;
				channel->stepNote  = channel->stepEndNote;
			}
		}
		else
		{
			// Step it down
			channel->stepNote += channel->stepValue;

			if (channel->stepNote >= channel->stepEndNote)
			{
				channel->stepValue = 0;
				channel->stepNote  = channel->stepEndNote;
			}
		}

		// Set the new period on the channel
		channel->currentNote = channel->stepNote;
		virtChannel->SetAmigaPeriod(channel->currentNote);
	}
	else
	{
		// Well, do we have any effects we need to run
		switch ((channel->patternData & 0x00000f00) >> 8)
		{
			// Arpeggio
			case 1:
			{
				Arpeggio(channel, virtChannel);
				break;
			}

			// Pitchbend
			case 2:
			{
				int16 bendValue, newPeriod;

				bendValue = (channel->patternData & 0x000000f0) >> 4;
				if (bendValue != 0)
					newPeriod = ((channel->patternData & 0xefff0000) >> 16) + bendValue;
				else
				{
					bendValue = (channel->patternData & 0x0000000f);
					if (bendValue != 0)
						newPeriod = ((channel->patternData & 0xefff0000) >> 16) - bendValue;
					else
						break;
				}

				// Play the new period
				virtChannel->SetAmigaPeriod(newPeriod);

				// Put the new period into the pattern data
				channel->patternData = (channel->patternData & 0x1000ffff) | (newPeriod << 16);
				break;
			}

			// LedOn (Filter off!)
			//
			// Brian has made a little bug in his player. He has
			// exchanged the on/off, so the effect names isn't
			// the right ones :)
			case 3:
			{
				amigaFilter = false;
				break;
			}

			// LedOff (Filter on!)
			case 4:
			{
				amigaFilter = true;
				break;
			}

			// SetStepUp
			case 7:
			{
				StepFinder(channel, false);
				break;
			}

			// SetStepDown
			case 8:
			{
				StepFinder(channel, true);
				break;
			}
		}
	}
}



/******************************************************************************/
/* StepFinder() initialize the step values.                                   */
/*                                                                            */
/* Input:   "channel" is a pointer to the intern channel structure.           */
/*          "stepDown" indicates if we need to step up or down.               */
/******************************************************************************/
void SoundFX::StepFinder(Channel *channel, bool stepDown)
{
	int16 stepValue, endIndex;
	int16 *note;

	// Find the step value and the end index
	stepValue = (channel->patternData & 0x0000000f);
	endIndex  = (channel->patternData & 0x000000f0) >> 4;
	if (stepDown)
		stepValue = -stepValue;
	else
		endIndex  = -endIndex;

	channel->stepNote  = channel->currentNote;
	channel->stepValue = stepValue;

	// Find the period in the period table
	note = &noteTable[20];
	for (;;)
	{
		// Couldn't find the period, so don't really step anywhere
		if (*note == -1)
		{
			channel->stepEndNote = channel->currentNote;
			return;
		}

		// Found the period, break the loop
		if (*note == channel->currentNote)
			break;

		// Get the next period
		note++;
	}

	// Get the end note
	channel->stepEndNote = *(note + endIndex);
}



/******************************************************************************/
/* Arpeggio() parse and run the arpeggio effect.                              */
/*                                                                            */
/* Input:   "channel" is a pointer to the intern channel structure.           */
/*          "virtChannel" is a pointer to the APlayer channel class.          */
/******************************************************************************/
void SoundFX::Arpeggio(Channel *channel, APChannel *virtChannel)
{
	int16 index;
	int16 *note;

	// Find out which period to play
	switch (timer)
	{
		case 1:
		case 5:
		{
			index = (channel->patternData & 0x000000f0) >> 4;
			break;
		}

		case 2:
		case 4:
		{
			index = (channel->patternData & 0x0000000f);
			break;
		}

		default:
		{
			virtChannel->SetAmigaPeriod(channel->currentNote);
			return;
		}
	}

	// Got the index, now find the period
	note = &noteTable[20];
	for (;;)
	{
		// Couldn't find the period, so no arpeggio :(
		if (*note == -1)
			return;

		// Found the period, break the loop
		if (*note == channel->currentNote)
			break;

		// Get the next period
		note++;
	}

	// Set the period
	virtChannel->SetAmigaPeriod(*(note + index));
}
