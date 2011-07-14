/******************************************************************************/
/* Fred Player Interface.                                                     */
/*                                                                            */
/* Original player by Frederic Hahn & Julien Clermonte.                       */
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

// Player headers
#include "Fred.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.02f



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static const uint32 periodTable[] =
{
	8192, 7728, 7296, 6888, 6504, 6136, 5792, 5464, 5160, 4872, 4600, 4336,
	4096, 3864, 3648, 3444, 3252, 3068, 2896, 2732, 2580, 2436, 2300, 2168,
	2048, 1932, 1824, 1722, 1626, 1534, 1448, 1366, 1290, 1218, 1150, 1084,
	1024,  966,  912,  861,  813,  767,  724,  683,  645,  609,  575,  542,
	 512,  483,  456,  430,  406,  383,  362,  341,  322,  304,  287,  271,
	 256,  241,  228,  215,  203,  191,  181,  170,  161,  152,  143,  135
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Fred::Fred(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	sequenceOffsets = NULL;
	instNoteTable   = NULL;
	instruments     = NULL;

	subSongs[0] = 1;
	subSongs[1] = 0;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Fred::~Fred(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float Fred::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the add-on.                    */
/*                                                                            */
/* Output: Is the number of the add-ons.                                      */
/******************************************************************************/
int32 Fred::GetCount(void)
{
	return (2);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 Fred::GetSupportFlags(int32 index)
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
PString Fred::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_FRED_NAME + index);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString Fred::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_FRED_DESCRIPTION + index);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString Fred::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_FRED_MIME + index);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a Fred Editor module.        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Fred::ModuleCheck(int32 index, PFile *file)
{
	if ((index == fredFinal) && IsFinal(file))
		return (AP_OK);

	if ((index == fredEditor) && IsEditor(file))
		return (AP_OK);

	return (AP_UNKNOWN);
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
ap_result Fred::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	ap_result retVal = AP_ERROR;

	if (index == fredFinal)
		retVal = LoadFinal(file, errorStr);

	if (index == fredEditor)
		retVal = LoadEditor(file, errorStr);

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool Fred::InitPlayer(int32 index)
{
	// Initialize the position info array
//	memset(posInfo, 0, sizeof(posInfo));

	// Find the longest (in time) channel and create
	// position structures. Do it for each subsong
	for (uint16 i = 0; i < subSongs[0]; i++)
	{
		FindLongestChannel(i);
//		CreatePosInfo(i);
	}

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void Fred::EndPlayer(int32 index)
{
	Cleanup();

	// Delete position informations
//	for (uint16 i = 0; i < 10; i++)
//		delete[] posInfo[i];
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void Fred::InitSound(int32 index, uint16 songNum)
{
	uint8 i;

	// Remember the song number
	currentSong = songNum;

	// Get the start tempo
	currentTempo = startTempos[songNum];

	// Initialize the channel structure
	for (i = 0; i < 4; i++)
	{
		Channel *chan = &channels[i];

		chan->chanNum         = i;
		chan->trackTable      = &sequenceOffsets[startSequence[songNum][i]];
		chan->trackPosition   = &instNoteTable[chan->trackTable[0]];
		chan->trackOffset     = 0;
		chan->trackDuration   = 1;
		chan->trackNote       = 0;
		chan->trackPeriod     = 0;
		chan->trackVolume     = 0;
		chan->instrument      = NULL;
		chan->vibFlags        = 0;
		chan->vibDelay        = 0;
		chan->vibSpeed        = 0;
		chan->vibAmpl         = 0;
		chan->vibValue        = 0;
		chan->portRunning     = false;
		chan->portDelay       = 0;
		chan->portLimit       = 0;
		chan->portTargetNote  = 0;
		chan->portStartPeriod = 0;
		chan->periodDiff      = 0;
		chan->portCounter     = 0;
		chan->portSpeed       = 0;
		chan->envState        = ENV_ATTACK;
		chan->sustainDelay    = 0;
		chan->arpPosition     = 0;
		chan->arpSpeed        = 0;
		chan->pulseWay        = false;
		chan->pulsePosition   = 0;
		chan->pulseDelay      = 0;
		chan->pulseSpeed      = 0;
		chan->pulseShot       = 0;
		chan->blendWay        = false;
		chan->blendPosition   = 0;
		chan->blendDelay      = 0;
		chan->blendShot       = 0;

		memset(chan->synthSample, 0, sizeof(chan->synthSample));
	}
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void Fred::Play(void)
{
	// Well, play all channels
	for (uint8 i = 0; i < 4; i++)
	{
		Channel *chan = &channels[i];

		// Decrement the note delay counter
		chan->trackDuration--;

		// Check to see if we need to get the next track line
		if (chan->trackDuration == 0)
		{
			uint8 retVal = DoNewLine(chan, virtChannels[i]);

			if (retVal == 1)		// Take the next voice
				continue;

			if (retVal == 2)		// Do the same voice again
			{
				i--;
				continue;
			}
		}
		else
		{
			// Check to see if we need to mute the channel
			if ((chan->trackDuration == 1) && (*chan->trackPosition < TRK_MAX_CODE))
				virtChannels[i]->Mute();
		}

		// Do synth and other effects
		ModifySound(chan, virtChannels[i]);
	}
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *Fred::GetSubSongs(void)
{
	return (subSongs);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 Fred::GetSongLength(void)
{
	return (posLength[currentSong].length);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 Fred::GetSongPosition(void)
{
	return (channels[posLength[currentSong].channel].trackOffset);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void Fred::SetSongPosition(int16 pos)
{
//	PosInfo *info;
	uint8 i;

	// Change the position
/*	info         = posInfo[currentSong];
	currentTempo = info[pos].tempo;

	for (i = 0; i < 4; i++)
	{
		channels[i].trackPosition = info[pos].trackPosition[i];
		channels[i].trackOffset   = info[pos].trackOffset[i];
		channels[i].trackDuration = 1;
	}
*/
	for (i = 0; i < 4; i++)
	{
		channels[i].trackOffset   = pos;
		channels[i].trackPosition = &instNoteTable[channels[i].trackTable[pos]];
		channels[i].trackDuration = 1;
 	}
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
PTimeSpan Fred::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	PTimeSpan total;
	int32 i, count;

	// Copy the position times
	count = posLength[songNum].posTimes.CountItems();
	for (i = 0; i < count; i++)
		posTimes.AddTail(posLength[songNum].posTimes.GetItem(i));

	total = posLength[songNum].time;
	total.AddMilliSeconds(1);
	return (total);
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
bool Fred::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 2)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_FRED_INFODESCLINE0);
			value.SetUNumber(posLength[currentSong].length);
			break;
		}

		// Used Instruments
		case 1:
		{
			description.LoadString(res, IDS_FRED_INFODESCLINE1);
			value.SetUNumber(instNum);
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
bool Fred::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	Instrument *inst;

	// First check the sample number for "out of range"
	if (num >= instNum)
		return (false);

	// Get the pointer to the sample data
	inst = &instruments[num];

	// Fill out the sample info structure
	info->name       = inst->name;
	info->bitSize    = 8;
	info->middleC    = 3464;
	info->volume     = inst->envVol;
	info->panning    = -1;

	if (inst->instType != INST_SAMPLE)
	{
		info->type       = apSynth;
		info->address    = NULL;
		info->length     = 0;
		info->flags      = 0;
		info->loopStart  = 0;
		info->loopLength = 0;
	}
	else
	{
		info->type    = apSample;
		info->address = inst->sampleAdr;
		info->length  = inst->length;

		if ((inst->repeatLen == 0) || (inst->repeatLen == 0xffff))
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
			info->loopStart  = 0;
			info->loopLength = inst->repeatLen;
		}
	}

	return (true);
}



/******************************************************************************/
/* IsFinal() tests the module to see if it's a Fred Final module.             */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: True if the module is a final module, else false.                  */
/******************************************************************************/
bool Fred::IsFinal(PFile *file)
{
	int16 i;
	int16 initOffset;
	uint16 initFunc[64];

	// Check the module size
	if (file->GetLength() < 0xb0e)
		return (false);

	// Check the code to see if it's a Fred module
	file->SeekToBegin();

	// First check the JMP instructions
	if (file->Read_B_UINT16() != 0x4efa)
		return (false);

	initOffset = file->Read_B_UINT16();

	if (file->Read_B_UINT16() != 0x4efa)
		return (false);

	file->Seek(2, PFile::pSeekCurrent);
	if (file->Read_B_UINT16() != 0x4efa)
		return (false);

	file->Seek(2, PFile::pSeekCurrent);
	if (file->Read_B_UINT16() != 0x4efa)
		return (false);

	// Seek to the init routine
	file->Seek(initOffset, PFile::pSeekBegin);

	// Read the beginning of the routine
	file->ReadArray_B_UINT16s(initFunc, 64);

	// Find the place in the routine, where it gets the subsong number
	for (i = 0; i < 4; i++)
	{
		if ((initFunc[i] == 0x123a) && (initFunc[i + 2] == 0xb001))
		{
			// Found it, remember the position to the module data
			moduleOffset = initFunc[i + 1] + initOffset + (i + 1) * 2 - 1;
			break;
		}
	}

	// Did we find the first piece of code?
	if (i == 4)
		return (false);	// Nup!

	// Okay, we need to find some code in the player, so we can
	// calculate the offset difference
	for (; i < 60; i++)
	{
		if ((initFunc[i] == 0x47fa) && (initFunc[i + 2] == 0xd7fa))
		{
			// Found it, now calculate the offset difference
			offsetDiff = initOffset + (i + 1) * 2 + (int16)initFunc[i + 1];
			return (true);
		}
	}

	return (false);
}



/******************************************************************************/
/* IsEditor() tests the module to see if it's a Fred Editor module.           */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: True if the module is an editor module, else false.                */
/******************************************************************************/
bool Fred::IsEditor(PFile *file)
{
	char signature[13];

	// Read the signature
	file->SeekToBegin();
	file->ReadString(signature, 12);

	// Does the file have the right signature
	if (strcmp(signature, "Fred Editor ") != 0)
		return (false);

	// Okay, check the version
	if (file->Read_B_UINT16() != 0x0000)
		return (false);

	// Check the number of songs
	if (file->Read_B_UINT16() > 10)
		return (false);

	return (true);
}



/******************************************************************************/
/* LoadFinal() will load a final module into the memory.                      */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to check.      */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Fred::LoadFinal(PFile *file, PString &errorStr)
{
	uint32 instOffset, instNoteOffset;
	int32 seqSize, instNoteSize;
	int32 sampOffset, minSampOffset;
	int16 testSynth;
	uint8 testInst;
	uint16 lastSampleNum;
	uint16 i;
	ap_result retVal = AP_ERROR;

	try
	{
		// Seek to the module data
		file->Seek(moduleOffset, PFile::pSeekBegin);

		// Skip played song
		file->Seek(1, PFile::pSeekCurrent);

		// Get the number of subsongs
		subSongs[0] = file->Read_UINT8() + 1;

		// Skip current tempo
		file->Seek(1, PFile::pSeekCurrent);

		// Get subsong start tempos
		file->Read(startTempos, 10);
		file->Seek(1, PFile::pSeekCurrent);

		// Get offset to other structures
		instOffset     = file->Read_B_UINT32() + offsetDiff;
		instNoteOffset = file->Read_B_UINT32() + offsetDiff;

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FRED_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Skip replay data
		file->Seek(100 + 128 * 4, PFile::pSeekCurrent);

		// Read subsong start sequence numbers
		for (i = 0; i < subSongs[0]; i++)
		{
			startSequence[i][0] = (file->Read_B_UINT16() - subSongs[0] * 4 * 2) / 2;
			startSequence[i][1] = (file->Read_B_UINT16() - subSongs[0] * 4 * 2) / 2;
			startSequence[i][2] = (file->Read_B_UINT16() - subSongs[0] * 4 * 2) / 2;
			startSequence[i][3] = (file->Read_B_UINT16() - subSongs[0] * 4 * 2) / 2;
		}

		// Allocate space to hold all the sequence offsets
		seqSize         = (min(instOffset, instNoteOffset) - file->GetPosition()) / 2;
		sequenceOffsets = new int16[seqSize];
		if (sequenceOffsets == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Load the offsets
		file->ReadArray_B_UINT16s((uint16 *)sequenceOffsets, seqSize);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FRED_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Allocate space to hold the inst/note commands
		instNoteSize  = abs((int)(instOffset - instNoteOffset));
		instNoteTable = new uint8[instNoteSize];
		if (instNoteTable == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Load the commands
		file->Seek(instNoteOffset, PFile::pSeekBegin);
		file->Read(instNoteTable, instNoteSize);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FRED_ERR_LOADING_PATTERNS);
			throw PUserException();
		}

		// Okay, now we get to the tough part, loading of the instruments
		file->Seek(instOffset, PFile::pSeekBegin);

		instNum       = 0;
		lastSampleNum = 0;
		minSampOffset = 0x7fffffff;

		// First find out how many instruments are present
		for (;;)
		{
			sampOffset = file->Read_B_UINT32();
			testSynth  = file->Read_B_UINT16();
			file->Seek(33, PFile::pSeekCurrent);
			testInst   = file->Read_UINT8();

			// Check to see if we have reached the end of the instrument part
			if ((file->IsEOF()) || ((file->GetPosition() + (63 - 1 - 33 - 2 - 4)) >= minSampOffset))
				break;				// Well, no more instruments

			// Skip unused instruments
			if (testInst != INST_UNUSED)
			{
				if (sampOffset == 0)
				{
					// !!Giants-bug1 hack!!
					if ((uint16)testSynth == 0xfcf5)
						break;

					// Synth instrument
					if ((testSynth != -1) && (testInst != INST_SAMPLE))
					{
						errorStr.LoadString(res, IDS_FRED_ERR_LOADING_INSTRUMENTS);
						throw PUserException();
					}
				}
				else
				{
					// Sample instrument
					sampOffset += offsetDiff;

					if (sampOffset < minSampOffset)
						minSampOffset = sampOffset;

					lastSampleNum = instNum;
				}
			}

			// Seek to next instrument
			file->Seek(64 - 1 - 33 - 2 - 4, PFile::pSeekCurrent);

			// Increment the number of instruments counted so far
			instNum++;
		}

		// Now read the instruments, but first allocate the structures
		instruments = new Instrument[instNum];
		if (instruments == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the instrument addresses
		for (i = 0; i < instNum; i++)
			instruments[i].sampleAdr = NULL;

		// Seek back to the start of the instruments
		file->Seek(instOffset, PFile::pSeekBegin);

		// Read the instruments
		for (i = 0; i < instNum; i++)
		{
			Instrument *inst = &instruments[i];

			inst->sampleOffset     = file->Read_B_UINT32() + offsetDiff;
			inst->repeatLen        = file->Read_B_UINT16();
			inst->length           = file->Read_B_UINT16() * 2;
			inst->period           = file->Read_B_UINT16();
			inst->vibDelay         = file->Read_UINT8();
			file->Seek(1, PFile::pSeekCurrent);

			inst->vibSpeed         = file->Read_UINT8();
			inst->vibAmpl          = file->Read_UINT8();
			inst->envVol           = file->Read_UINT8();
			inst->attackSpeed      = file->Read_UINT8();
			inst->attackVolume     = file->Read_UINT8();
			inst->decaySpeed       = file->Read_UINT8();
			inst->decayVolume      = file->Read_UINT8();
			inst->sustainDelay     = file->Read_UINT8();
			inst->releaseSpeed     = file->Read_UINT8();
			inst->releaseVolume    = file->Read_UINT8();
			file->Read(inst->arpeggio, 16);

			inst->arpSpeed         = file->Read_UINT8();
			inst->instType         = file->Read_UINT8();
			inst->pulseRateMin     = file->Read_UINT8();
			inst->pulseRatePlus    = file->Read_UINT8();
			inst->pulseSpeed       = file->Read_UINT8();
			inst->pulseStart       = file->Read_UINT8();
			inst->pulseEnd         = file->Read_UINT8();
			inst->pulseDelay       = file->Read_UINT8();
			inst->instSyncro       = file->Read_UINT8();
			inst->blend            = file->Read_UINT8();
			inst->blendDelay       = file->Read_UINT8();
			inst->pulseShotCounter = file->Read_UINT8();
			inst->blendShotCounter = file->Read_UINT8();
			inst->arpCount         = file->Read_UINT8();

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			file->Seek(12, PFile::pSeekCurrent);
		}

		// The last part to load, the sample data!
		for (i = 0; i < instNum; i++)
		{
			if (instruments[i].instType == INST_SAMPLE)
			{
				// !!Exploding Fish hack!!
				if ((instruments[i].repeatLen != 0xffff) || (instruments[i].sampleOffset != 0xb590))
				{
					// The instrument is a sample, so allocate the space
					// to hold the sample data
					instruments[i].sampleAdr = new int8[instruments[i].length];
					if (instruments[i].sampleAdr == NULL)
					{
						errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
						throw PUserException();
					}

					// Fill out the sample array in case if the last sample
					// isn't whole
					memset(instruments[i].sampleAdr, instruments[i].length, 0);

					// Load the sample data
					file->Seek(instruments[i].sampleOffset, PFile::pSeekBegin);
					file->Read(instruments[i].sampleAdr, instruments[i].length);

					if (file->IsEOF() && (i < lastSampleNum))
					{
						errorStr.LoadString(res, IDS_FRED_ERR_LOADING_SAMPLES);
						throw PUserException();
					}
				}
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
/* LoadEditor() will load an editor module into the memory.                   */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to check.      */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Fred::LoadEditor(PFile *file, PString &errorStr)
{
	PCharSet_Amiga charSet;
	uint32 i, j, k;
	int32 pattNumPos;
	uint32 pattSize;
	uint16 totalPattLen, sampNum;
	uint16 pattStartPos[128];
	int8 posData[256];
	uint8 *pattData;
	ap_result retVal = AP_ERROR;

	try
	{
		// Skip the signature and version
		file->Seek(14, PFile::pSeekBegin);

		// Get number of subsongs
		subSongs[0] = file->Read_B_UINT16();

		// Read the subsong start tempos
		file->Read(startTempos, subSongs[0]);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FRED_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Calculate subsong start sequence numbers
		for (i = 0; i < subSongs[0]; i++)
		{
			startSequence[i][0] = i * 0x400 + 0x000;
			startSequence[i][1] = i * 0x400 + 0x100;
			startSequence[i][2] = i * 0x400 + 0x200;
			startSequence[i][3] = i * 0x400 + 0x300;
		}

		// Skip the pattern numbers at the moment. We will load them later on
		pattNumPos = file->GetPosition();
		file->Seek(subSongs[0] * 256 * 4, PFile::pSeekCurrent);

		// There is always 128 patterns, but first scan them and
		// calculate the total size
		totalPattLen = 0;
		for (i = 0; i < 128; i++)
		{
			// Read current pattern size
			pattSize = file->Read_B_UINT32();

			// Did we get some problems?
			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_PATTERNS);
				throw PUserException();
			}

			// Store the pattern position
			pattStartPos[i] = totalPattLen;

			// Skip the pattern data
			totalPattLen += pattSize;
			file->Seek(pattSize, PFile::pSeekCurrent);
		}

		// We can now convert the position data to offsets
		file->Seek(pattNumPos, PFile::pSeekBegin);

		// Allocate memory to hold the sequence offsets
		sequenceOffsets = new int16[subSongs[0] * 256 * 4];
		if (sequenceOffsets == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Take one subsong at the time
		for (i = 0; i < subSongs[0]; i++)
		{
			// There is 4 channels in each subsong
			for (j = 0; j < 4; j++)
			{
				// Find offset pointer
				int16 *seqOffset = &sequenceOffsets[i * 256 * 4 + j * 256];

				// Read one channel position data
				file->Read(posData, 256);

				if (file->IsEOF())
				{
					errorStr.LoadString(res, IDS_FRED_ERR_LOADING_HEADER);
					throw PUserException();
				}

				// Now convert the data
				for (k = 0; k < 256; k++)
				{
					if (posData[k] < 0)
					{
						if (posData[k] == -1)
						{
							// Stop command
							*seqOffset++ = -1;
						}
						else
						{
							// Jump command, convert it
							*seqOffset++ = (0x8000 | ((posData[k] + 128) & 0xff));
						}
					}
					else
					{
						// Pattern position, get the offset
						*seqOffset++ = pattStartPos[posData[k]];
					}
				}
			}
		}

		// Allocate memory to hold the pattern data
		instNoteTable = new uint8[totalPattLen];
		if (instNoteTable == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Now load the pattern data
		pattData = instNoteTable;
		for (i = 0; i < 128; i++)
		{
			// Read current pattern size
			pattSize = file->Read_B_UINT32();

			// Read the pattern data
			file->Read(pattData, pattSize);

			// Did we get some problems?
			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_PATTERNS);
				throw PUserException();
			}

			// Change the pointer
			pattData += pattSize;
		}

		// Get the number of instruments
		instNum = file->Read_B_UINT16();

		// Allocate memory to hold the instrument info
		instruments = new Instrument[instNum];
		if (instruments == NULL)
		{
			errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the instrument addresses
		for (i = 0; i < instNum; i++)
			instruments[i].sampleAdr = NULL;

		// Read the instrument info
		for (i = 0; i < instNum; i++)
		{
			char name[33];
			Instrument *inst = &instruments[i];

			file->ReadString(name, 32);
			inst->name.SetString(name, &charSet);

			inst->instIndex        = file->Read_B_UINT32();
			inst->repeatLen        = file->Read_B_UINT16();
			inst->length           = file->Read_B_UINT16() * 2;
			inst->period           = file->Read_B_UINT16();
			inst->vibDelay         = file->Read_UINT8();
			file->Seek(1, PFile::pSeekCurrent);

			inst->vibSpeed         = file->Read_UINT8();
			inst->vibAmpl          = file->Read_UINT8();
			inst->envVol           = file->Read_UINT8();
			inst->attackSpeed      = file->Read_UINT8();
			inst->attackVolume     = file->Read_UINT8();
			inst->decaySpeed       = file->Read_UINT8();
			inst->decayVolume      = file->Read_UINT8();
			inst->sustainDelay     = file->Read_UINT8();
			inst->releaseSpeed     = file->Read_UINT8();
			inst->releaseVolume    = file->Read_UINT8();
			file->Read(inst->arpeggio, 16);

			inst->arpSpeed         = file->Read_UINT8();
			inst->instType         = file->Read_UINT8();
			inst->pulseRateMin     = file->Read_UINT8();
			inst->pulseRatePlus    = file->Read_UINT8();
			inst->pulseSpeed       = file->Read_UINT8();
			inst->pulseStart       = file->Read_UINT8();
			inst->pulseEnd         = file->Read_UINT8();
			inst->pulseDelay       = file->Read_UINT8();
			inst->instSyncro       = file->Read_UINT8();
			inst->blend            = file->Read_UINT8();
			inst->blendDelay       = file->Read_UINT8();
			inst->pulseShotCounter = file->Read_UINT8();
			inst->blendShotCounter = file->Read_UINT8();
			inst->arpCount         = file->Read_UINT8();

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			file->Seek(12, PFile::pSeekCurrent);
		}

		// Read number of sample data blocks
		sampNum = file->Read_B_UINT16();

		for (i = 0; i < sampNum; i++)
		{
			// Read the instrument index
			uint16 instIndex = file->Read_B_UINT16();

			// Find the instrument structure
			bool found = false;
			for (j = 0; j < instNum; j++)
			{
				if (instruments[j].instIndex == instIndex)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_SAMPLES);
				throw PUserException();
			}

			// Allocate memory to hold the sample data
			uint32 sampSize = file->Read_B_UINT16();

			instruments[j].sampleAdr = new int8[sampSize];
			if (instruments[j].sampleAdr == NULL)
			{
				errorStr.LoadString(res, IDS_FRED_ERR_MEMORY);
				throw PUserException();
			}

			// Read the sample data
			file->Read(instruments[j].sampleAdr, sampSize);

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FRED_ERR_LOADING_SAMPLES);
				throw PUserException();
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
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void Fred::Cleanup(void)
{
	// Delete the samples if any
	if (instruments != NULL)
	{
		for (uint32 i = 0; i < instNum; i++)
			delete[] instruments[i].sampleAdr;

		// Delete the instruments
		delete[] instruments;
		instruments = NULL;
	}

	// Delete the inst/note commands
	delete[] instNoteTable;
	instNoteTable = NULL;

	// Delete the sequence offsets
	delete[] sequenceOffsets;
	sequenceOffsets = NULL;
}



/******************************************************************************/
/* DoNewLine() take the next track line and parse it.                         */
/*                                                                            */
/* Input:   "chan" is a pointer to the current channel.                       */
/*          "apChan" is a pointer to the APlayer channel.                     */
/*                                                                            */
/* Output:  0 = continue, 1 = take next channel, 2 = take the same channel.   */
/******************************************************************************/
uint8 Fred::DoNewLine(Channel *chan, APChannel *apChan)
{
	bool instChange = false;

	// Get the current track position
	uint8 *trackPos = chan->trackPosition;

	for (;;)
	{
		uint8 cmd = *trackPos++;

		// Is the command a note?
		if (cmd < 0x80)
		{
			// Yep, play it
			Instrument *inst = chan->instrument;

			// Store the new position
			chan->trackPosition = trackPos;

			// Do we play an invalid instrument?
			if (inst == NULL)
			{
				// Stop all effects
				chan->portRunning = false;
				chan->vibFlags    = 0;
				chan->trackVolume = 0;
				apChan->Mute();

				// Take the next channel
				return (0);
			}

			// Initialize the channel
			chan->trackNote   = cmd;
			chan->arpPosition = 0;
			chan->arpSpeed    = inst->arpSpeed;
			chan->vibDelay    = inst->vibDelay;
			chan->vibSpeed    = inst->vibSpeed;
			chan->vibAmpl     = inst->vibAmpl;
			chan->vibFlags    = VIB_VIB_DIRECTION | VIB_PERIOD_DIRECTION;
			chan->vibValue    = 0;

			// Create synth sample if the instrument used is a synth instrument
			if ((inst->instType == INST_PULSE) && (instChange || (inst->instSyncro & SYNC_PULSE_SYNCRO)))
				CreateSynthSamplePulse(inst, chan);
			else
			{
				if ((inst->instType == INST_BLEND) && (instChange || (inst->instSyncro & SYNC_BLEND_SYNCRO)))
					CreateSynthSampleBlend(inst, chan);
			}

			// Set the track duration (speed)
			chan->trackDuration = currentTempo;

			// Play the instrument
			if (inst->instType == INST_SAMPLE)
			{
				// Play sample
				apChan->PlaySample(inst->sampleAdr, 0, inst->length);

				if ((inst->repeatLen != 0) && (inst->repeatLen != 0xffff))
				{
					// There isn't a bug in this line. The original player calculate
					// the wrong start and length too!
					apChan->SetLoop(inst->repeatLen, inst->length - inst->repeatLen);
				}
			}
			else
			{
				// Play synth sound
				apChan->PlaySample(chan->synthSample, 0, inst->length);
				apChan->SetLoop(0, inst->length);
			}

			// Set the volume (mute the channel)
			apChan->SetVolume(0);

			chan->trackVolume  = 0;
			chan->envState     = ENV_ATTACK;
			chan->sustainDelay = inst->sustainDelay;

			// Set the period
			chan->trackPeriod = (periodTable[chan->trackNote] * inst->period) / 1024;
			apChan->SetAmigaPeriod(chan->trackPeriod);

			// Initialize portamento
			if ((chan->portRunning) && (chan->portStartPeriod == 0))
			{
				chan->periodDiff      = chan->portLimit - chan->trackPeriod;
				chan->portCounter     = 1;
				chan->portStartPeriod = chan->trackPeriod;
			}

			// Take the next channel
			return (0);
		}
		else
		{
			// It's a command
			switch (cmd)
			{
				//
				// Change instrument
				//
				case TRK_INST_CODE:
				{
					uint8 newInst = *trackPos++;

					if (newInst >= instNum)
						chan->instrument = NULL;
					else
					{
						// Find the instrument information
						chan->instrument = &instruments[newInst];

						if (chan->instrument->instType == INST_UNUSED)
							chan->instrument = NULL;
						else
							instChange = true;
					}
					break;
				}

				//
				// Change tempo
				//
				case TRK_TEMPO_CODE:
				{
					currentTempo = *trackPos++;
					break;
				}

				//
				// Start portamento
				//
				case TRK_PORT_CODE:
				{
					uint16 instPeriod = 428;

					if (chan->instrument != NULL)
						instPeriod = chan->instrument->period;

					chan->portSpeed       = *trackPos++ * currentTempo;
					chan->portTargetNote  = *trackPos++;
					chan->portLimit       = (periodTable[chan->portTargetNote] * instPeriod) / 1024;
					chan->portStartPeriod = 0;
					chan->portDelay       = *trackPos++ * currentTempo;
					chan->portRunning     = true;
					break;
				}

				//
				// Execute pause
				//
				case TRK_PAUSE_CODE:
				{
					chan->trackDuration = currentTempo;
					chan->trackPosition = trackPos;
					apChan->Mute();

					// Take next channel
					return (1);
				}

				//
				// End, goto next pattern
				//
				case TRK_END_CODE:
				{
					chan->trackOffset++;

					for (;;)
					{
						// If the song ends, start it over
						if (chan->trackTable[chan->trackOffset] == -1)
						{
							chan->trackOffset = 0;
							currentTempo      = startTempos[currentSong];

							// Tell APlayer that the songs has ended
							if (chan->chanNum == posLength[currentSong].channel)
								endReached = true;

							continue;
						}

						if (chan->trackTable[chan->trackOffset] < 0)
						{
							// Jump to a new position
							uint16 oldOffset  = chan->trackOffset;
							chan->trackOffset = (chan->trackTable[chan->trackOffset] & 0x7fff) / 2;

							if (chan->chanNum == posLength[currentSong].channel)
							{
								// Do we jump back in the song
								if (chan->trackOffset < oldOffset)
									endReached = true;		// Tell APlayer we has started over
							}
							continue;
						}

						// Stop the loop
						break;
					}

					// Find new track
					chan->trackPosition = &instNoteTable[chan->trackTable[chan->trackOffset]];
					chan->trackDuration = 1;

					// Tell APlayer we have changed the position
					if (chan->chanNum == posLength[currentSong].channel)
						ChangePosition();

					// Take same channel again
					return (2);
				}

				//
				// Note delay
				//
				default:
				{
					chan->trackDuration = -(int8)cmd * currentTempo;
					chan->trackPosition = trackPos;

					// Take the next channel
					return (0);
				}
			}
		}
	}
}



/******************************************************************************/
/* CreateSynthSamplePulse() create a pulse synth sample.                      */
/*                                                                            */
/* Input:   "inst" is a pointer to the current instrument.                    */
/*          "chan" is a pointer to the current channel.                       */
/******************************************************************************/
void Fred::CreateSynthSamplePulse(Instrument *inst, Channel *chan)
{
	uint16 i;

	// Initialize the pulse variables
	chan->pulseShot     = inst->pulseShotCounter;
	chan->pulseDelay    = inst->pulseDelay;
	chan->pulseSpeed    = inst->pulseSpeed;
	chan->pulseWay      = false;
	chan->pulsePosition = inst->pulseStart;

	// Create first part of the sample
	for (i = 0; i < inst->pulseStart; i++)
		chan->synthSample[i] = inst->pulseRateMin;

	// Create the second part
	for (; i < inst->length; i++)
		chan->synthSample[i] = inst->pulseRatePlus;
}



/******************************************************************************/
/* CreateSynthSampleBlend() create a blend synth sample.                      */
/*                                                                            */
/* Input:   "inst" is a pointer to the current instrument.                    */
/*          "chan" is a pointer to the current channel.                       */
/******************************************************************************/
void Fred::CreateSynthSampleBlend(Instrument *inst, Channel *chan)
{
	uint16 i;

	// Initialize the blend variables
	chan->blendWay      = false;
	chan->blendPosition = 1;
	chan->blendShot     = inst->blendShotCounter;
	chan->blendDelay    = inst->blendDelay;

	for (i = 0; i < 32; i++)
		chan->synthSample[i] = inst->sampleAdr[i];
}



/******************************************************************************/
/* ModifySound() runs all the effects on the sound.                           */
/*                                                                            */
/* Input:   "chan" is a pointer to the current channel.                       */
/*          "apChan" is a pointer to the APlayer channel.                     */
/******************************************************************************/
void Fred::ModifySound(Channel *chan, APChannel *apChan)
{
	Instrument *inst = chan->instrument;
	uint8 newNote;
	uint16 period;

	// If the channel doesn't have any instruments, don't do anything
	if (inst == NULL)
		return;

	//
	// Arpeggio
	//
	newNote = chan->trackNote + inst->arpeggio[chan->arpPosition];

	chan->arpSpeed--;
	if (chan->arpSpeed == 0)
	{
		chan->arpSpeed = inst->arpSpeed;

		// Get to the next position
		chan->arpPosition++;
		if (chan->arpPosition == inst->arpCount)
			chan->arpPosition = 0;
	}

	// Find the new period
	chan->trackPeriod = (periodTable[newNote] * inst->period) / 1024;

	//
	// Portamento
	//
	if (chan->portRunning)
	{
		// Should we delay the portamento?
		if (chan->portDelay != 0)
			chan->portDelay--;
		else
		{
			// Do it, calculate the new period
			chan->trackPeriod += (chan->portCounter * chan->periodDiff / chan->portSpeed);

			chan->portCounter++;
			if (chan->portCounter > chan->portSpeed)
			{
				chan->trackNote   = chan->portTargetNote;
				chan->portRunning = false;
			}
		}
	}

	//
	// Vibrato
	//
	period = chan->trackPeriod;

	if (chan->vibDelay != 0)
		chan->vibDelay--;
	else
	{
		if (chan->vibFlags != 0)
		{
			// Vibrato is running, now check the vibrato direction
			if (chan->vibFlags & VIB_VIB_DIRECTION)
			{
				chan->vibValue += chan->vibSpeed;

				if (chan->vibValue == chan->vibAmpl)
					chan->vibFlags &= ~VIB_VIB_DIRECTION;
			}
			else
			{
				chan->vibValue -= chan->vibSpeed;

				if (chan->vibValue == 0)
					chan->vibFlags |= VIB_VIB_DIRECTION;
			}

			// Change the direction of the period
			if (chan->vibValue == 0)
				chan->vibFlags ^= VIB_PERIOD_DIRECTION;

			if (chan->vibFlags & VIB_PERIOD_DIRECTION)
				period += chan->vibValue;
			else
				period -= chan->vibValue;
		}
	}

	// Set the period
	apChan->SetAmigaPeriod(period);

	//
	// Envelope
	//
	switch (chan->envState)
	{
		case ENV_ATTACK:
		{
			chan->trackVolume += inst->attackSpeed;
			if (chan->trackVolume >= inst->attackVolume)
			{
				chan->trackVolume = inst->attackVolume;
				chan->envState    = ENV_DECAY;
			}
			break;
		}

		case ENV_DECAY:
		{
			chan->trackVolume -= inst->decaySpeed;
			if (chan->trackVolume <= inst->decayVolume)
			{
				chan->trackVolume = inst->decayVolume;
				chan->envState    = ENV_SUSTAIN;
			}
			break;
		}

		case ENV_SUSTAIN:
		{
			if (chan->sustainDelay == 0)
				chan->envState = ENV_RELEASE;
			else
				chan->sustainDelay--;

			break;
		}

		case ENV_RELEASE:
		{
			chan->trackVolume -= inst->releaseSpeed;
			if (chan->trackVolume <= inst->releaseVolume)
			{
				chan->trackVolume = inst->releaseVolume;
				chan->envState    = ENV_DONE;
			}
			break;
		}
	}

	// Set the volume
	apChan->SetVolume(inst->envVol * chan->trackVolume / 256);

	//
	// Pulse on synth samples
	//
	if (inst->instType == INST_PULSE)
	{
		if (chan->pulseDelay != 0)
			chan->pulseDelay--;
		else
		{
			if (chan->pulseSpeed != 0)
				chan->pulseSpeed--;
			else
			{
				if ((!(inst->instSyncro & SYNC_PULSE_X_SHOT)) || (chan->pulseShot != 0))
				{
					chan->pulseSpeed = inst->pulseSpeed;

					for (;;)
					{
						if (chan->pulseWay)
						{
							if (chan->pulsePosition >= inst->pulseStart)
							{
								// Change the sample at the pulse position
								chan->synthSample[chan->pulsePosition] = inst->pulseRatePlus;
								chan->pulsePosition--;
								break;
							}
							else
							{
								// Switch direction
								chan->pulseWay = false;
								chan->pulseShot--;
								chan->pulsePosition++;
							}
						}
						else
						{
							if (chan->pulsePosition <= inst->pulseEnd)
							{
								// Change the sample at the pulse position
								chan->synthSample[chan->pulsePosition] = inst->pulseRateMin;
								chan->pulsePosition++;
								break;
							}
							else
							{
								// Switch direction
								chan->pulseWay = true;
								chan->pulseShot--;
								chan->pulsePosition--;
							}
						}
					}
				}
			}
		}
	}

	//
	// Blend on synth samples
	//
	if (inst->instType == INST_BLEND)
	{
		if (chan->blendDelay != 0)
			chan->blendDelay--;
		else
		{
			for (;;)
			{
				if ((!(inst->instSyncro & SYNC_BLEND_X_SHOT)) && (chan->blendShot != 0))
				{
					if (chan->blendWay)
					{
						if (chan->blendPosition == 1)
						{
							chan->blendWay = false;
							chan->blendShot--;
							continue;
						}

						chan->blendPosition--;
						break;
					}
					else
					{
						if (chan->blendPosition == (1 << inst->blend))
						{
							chan->blendWay = true;
							chan->blendShot--;
							continue;
						}

						chan->blendPosition++;
						break;
					}
				}
				else
					return;		// Well, done with the effects
			}

			// Create new synth sample
			for (uint16 i = 0; i < 32; i++)
				chan->synthSample[i] = ((chan->blendPosition * inst->sampleAdr[i + 32]) >> inst->blend) + inst->sampleAdr[i];
		}
	}
}



/******************************************************************************/
/* FindLongestChannel() scans all the channels to find the channel which take */
/*      longest time to play and fill out the structure given.                */
/*                                                                            */
/* Input:  "songNum" is the subsong to scan.                                  */
/******************************************************************************/
void Fred::FindLongestChannel(uint16 songNum)
{
	float chanTimes[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool hasNote[4] = { false, false, false, false };
	bool doneFlags[4] = { false, false, false, false };
	PList<float> posTimes[4];
	uint8 i;

	// Get the start tempo
	uint8 curTempo = startTempos[songNum];

	// Initialize the channels
	for (i = 0; i < 4; i++)
	{
		channels[i].trackTable    = &sequenceOffsets[startSequence[songNum][i]];
		channels[i].trackPosition = &instNoteTable[channels[i].trackTable[0]];
		channels[i].trackOffset   = 0;
		posTimes[i].AddTail(0.0f);
	}

	// Okay, now calculate the time for each channel
	for (;;)
	{
		// Check to see if all channels has been parsed
		if (doneFlags[0] && doneFlags[1] && doneFlags[2] && doneFlags[3])
			break;

		for (i = 0; i < 4; i++)
		{
			if (!doneFlags[i])
			{
				Channel *chan = &channels[i];
				bool chanDone = false;

				do
				{
					// Get the channel command
					uint8 cmd = *chan->trackPosition++;

					// Is the command a note?
					if (cmd < 0x80)
					{
						// Yup, add the note time
						chanTimes[i] += (1000.0f * curTempo / 50.0f);
						hasNote[i]    = true;
						chanDone      = true;
					}
					else
					{
						// It's a real command
						switch (cmd)
						{
							case TRK_INST_CODE:
							{
								chan->trackPosition++;
								break;
							}

							case TRK_TEMPO_CODE:
							{
								curTempo = *chan->trackPosition++;
								break;
							}

							case TRK_PORT_CODE:
							{
								chan->trackPosition += 3;
								break;
							}

							case TRK_PAUSE_CODE:
							{
								chanTimes[i] += (1000.0f * curTempo / 50.0f);
								chanDone = true;
								break;
							}

							case TRK_END_CODE:
							{
								chan->trackOffset++;

								if (chan->trackTable[chan->trackOffset] == -1)
								{
									curTempo     = startTempos[songNum];
									doneFlags[i] = true;
									chanDone     = true;
								}
								else
								{
									if (chan->trackTable[chan->trackOffset] < 0)
									{
										doneFlags[i] = true;
										chanDone     = true;
									}
									else
									{
										chan->trackPosition = &instNoteTable[chan->trackTable[chan->trackOffset]];
										posTimes[i].AddTail(chanTimes[i]);
									}
								}
								break;
							}

							// Note delay
							default:
							{
								chanTimes[i] += (-(int8)cmd * 1000.0f * curTempo / 50.0f);
								chanDone = true;
								break;
							}
						}
					}
				}
				while (!chanDone);
			}
		}
	}

	// Exterminate channels which doesn't had a note
	for (i = 0; i < 4; i++)
	{
		if (!hasNote[i])
			chanTimes[i] = 0.0f;
	}

	// Find the channel which takes the longest time to play
	posLength[songNum].time = 0.0f;

	for (i = 0; i < 4; i++)
	{
		if (chanTimes[i] >= posLength[songNum].time)
		{
			posLength[songNum].channel = i;
			posLength[songNum].length  = channels[i].trackOffset;
			posLength[songNum].time    = chanTimes[i];
		}
	}

	// Now copy the position times
	posLength[songNum].posTimes = posTimes[posLength[songNum].channel];
}



/******************************************************************************/
/* CreatePosInfo() create position information for each position in the song. */
/*                                                                            */
/* Input:  "songNum" is the subsong to scan.                                  */
/******************************************************************************/
/*void Fred::CreatePosInfo(uint16 songNum)
{
	bool doneFlags[4] = { false, false, false, false };
	PosInfo *info;
	uint8 i, j;

	// Allocate the array with position informations
	posInfo[songNum] = new PosInfo[posLength[songNum].length];
	if (posInfo[songNum] == NULL)
		throw PMemoryException();

	// Get the start tempo
	uint8 curTempo = startTempos[songNum];

	info = posInfo[songNum];
	info[0].tempo = curTempo;

	// Initialize the channels
	for (i = 0; i < 4; i++)
	{
		channels[i].trackTable    = &sequenceOffsets[startSequence[songNum][i]];
		channels[i].trackPosition = &instNoteTable[channels[i].trackTable[0]];
		channels[i].trackOffset   = 0;
		channels[i].trackDuration = 1;

		info[0].trackPosition[i] = channels[i].trackPosition;
		info[0].trackOffset[i]   = channels[i].trackOffset;
	}

	// Okay, now calculate the time for each channel
	for (;;)
	{
		// Check to see if all channels has been parsed
		if (doneFlags[0] && doneFlags[1] && doneFlags[2] && doneFlags[3])
			break;

		for (i = 0; i < 4; i++)
		{
			if (!doneFlags[i])
			{
				Channel *chan = &channels[i];
				bool chanDone = false;

				// Count down the duration
				chan->trackDuration--;
				if (chan->trackDuration == 0)
				{
					do
					{
						// Get the channel command
						uint8 cmd = *chan->trackPosition++;

						// Is the command a note?
						if (cmd < 0x80)
						{
							// Yup, done with the channel at the moment
							chan->trackDuration = curTempo;
							chanDone = true;
						}
						else
						{
							// It's a real command
							switch (cmd)
							{
								case TRK_INST_CODE:
									chan->trackPosition++;
									break;

								case TRK_TEMPO_CODE:
									curTempo = *chan->trackPosition++;
									break;

								case TRK_PORT_CODE:
									chan->trackPosition += 3;
									break;

								case TRK_PAUSE_CODE:
									chan->trackDuration = curTempo;
									chanDone = true;
									break;

								case TRK_END_CODE:
									chan->trackOffset++;

									chan->trackDuration = 1;
									chanDone = true;

									if ((chan->trackTable[chan->trackOffset] == -1) || (chan->trackTable[chan->trackOffset] < 0))
										doneFlags[i] = true;
									else
									{
										chan->trackPosition = &instNoteTable[chan->trackTable[chan->trackOffset]];

										// Do we work on the "position" channel?
										if (posLength[songNum].channel == i)
										{
											// Yup, remember the informations
											info[chan->trackOffset].tempo = curTempo;

											for (j = 0; j < 4; j++)
											{
												info[chan->trackOffset].trackPosition[j] = channels[j].trackPosition;
												info[chan->trackOffset].trackOffset[j]   = channels[j].trackOffset;
											}
										}
									}
									break;

								// Note delay
								default:
									chan->trackDuration = -(int8)cmd * curTempo;
									chanDone = true;
									break;
							}
						}
					}
					while (!chanDone);
				}
			}
		}
	}
}
*/
