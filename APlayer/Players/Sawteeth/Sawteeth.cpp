/******************************************************************************/
/* Sawteeth Player Interface.                                                 */
/*                                                                            */
/* Original player by Jonas and Arvid Norberg.                                */
/* APlayer plug-in by Arvid Norberg and Thomas Neumann.                       */
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
#include "Sawteeth.h"
#include "Player.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.00f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Sawteeth::Sawteeth(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	outBuffers  = NULL;

	chan        = NULL;
	parts       = NULL;
	ins         = NULL;
	breakPoints = NULL;

	p           = NULL;

	n2f         = NULL;
	r2f         = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Sawteeth::~Sawteeth(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float Sawteeth::GetVersion(void)
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
uint32 Sawteeth::GetSupportFlags(int32 index)
{
	return (appSamplePlayer | appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString Sawteeth::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_SAW_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString Sawteeth::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_SAW_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString Sawteeth::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_SAW_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a Sawteeth module.           */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Sawteeth::ModuleCheck(int32 index, PFile *file)
{
	uint16 ver;
	uint8 chan;

	// Check the module size
	if (file->GetLength() < 10)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();

	mark = file->Read_B_UINT32();
	if ((mark != 'SWTD') && (mark != 'SWTT'))
		return (AP_UNKNOWN);

	if (mark == 'SWTT')
	{
		// Read the mark probably
		file->SeekToBegin();
		ReadString(file);
	}

	// Check the version
	ver = Read16Bit(file);
	if ((ver > ST_CURRENT_FILE_VERSION) || (ver == 882))
		return (AP_UNKNOWN);

	// Check the position length if not a closed beta
	if (ver >= 900)
	{
		if (Read16Bit(file) < 1)
			return (AP_UNKNOWN);
	}

	// Check the number of channels
	chan = Read8Bit(file);
	if ((chan < 1) || (chan > CHN))
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
ap_result Sawteeth::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	uint8 i, k;
	uint16 j;
	ap_result retVal = AP_ERROR;

	try
	{
		// Skip the module mark
		if (mark == 'SWTD')
			file->Seek(4, PFile::pSeekBegin);
		else
			ReadString(file);

		// Get the version
		stVersion = Read16Bit(file);

		if (stVersion < 900)	// Special hack for compatibility with CLOSED_BETA
			spsPal = 882;
		else
			spsPal = Read16Bit(file);

		//
		// CHANNELS
		//
		channelCount = Read8Bit(file);

		chan = new Channel[channelCount];
		if (chan == NULL)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < channelCount; i++)
			chan[i].steps = NULL;

		// Read the channel information
		for (i = 0; i < channelCount; i++)
		{
			chan[i].left  = Read8Bit(file);
			chan[i].right = Read8Bit(file);
			chan[i].len   = Read16Bit(file);

			// Previous we did not have loop points
			if (stVersion < 910)
				chan[i].lLoop = 0;
			else
				chan[i].lLoop = Read16Bit(file);

			// Previous we did not have right loop points
			if (stVersion < 1200)
				chan[i].rLoop = chan[i].len - 1;
			else
				chan[i].rLoop = Read16Bit(file);

			// Check the channel length
			if ((chan[i].len < 1) || (chan[i].len > CHNSTEPS))
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_CHANNELS);
				throw PUserException();
			}

			if (chan[i].rLoop >= chan[i].len)
				chan[i].rLoop = chan[i].len - 1;

			// Sequence
			chan[i].steps = new ChStep[chan[i].len];
			if (chan[i].steps == NULL)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
				throw PUserException();
			}

			for (j = 0; j < chan[i].len; j++)
			{
				chan[i].steps[j].part   = Read8Bit(file);
				chan[i].steps[j].transp = Read8Bit(file);
				chan[i].steps[j].damp   = Read8Bit(file);
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_CHANNELS);
				throw PUserException();
			}
		}

		//
		// PARTS
		//
		partCount = Read8Bit(file);
		if (partCount < 1)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_LOADING_PARTS);
			throw PUserException();
		}

		parts = new Part[partCount];
		if (parts == NULL)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < partCount; i++)
			parts[i].steps = NULL;

		// Read the part information
		for (i = 0; i < partCount; i++)
		{
			parts[i].sps = Read8Bit(file);
			if (parts[i].sps < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_PARTS);
				throw PUserException();
			}

			parts[i].len = Read8Bit(file);
			if (parts[i].len < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_PARTS);
				throw PUserException();
			}

			parts[i].steps = new Step[parts[i].len];
			if (parts[i].steps == NULL)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
				throw PUserException();
			}

			for (k = 0; k < parts[i].len; k++)
			{
				parts[i].steps[k].ins  = Read8Bit(file);
				parts[i].steps[k].eff  = Read8Bit(file);
				parts[i].steps[k].note = Read8Bit(file);
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_PARTS);
				throw PUserException();
			}
		}

		// Check to see if any channels points to a part that does not exist
		for (i = 0; i < channelCount; i++)
		{
			for (j = 0; j < chan[i].len; j++)
			{
				if (chan[i].steps[j].part >= partCount)
					chan[i].steps[j].part = partCount - 1;
			}
		}

		//
		// Instruments
		//
		instrumentCount = Read8Bit(file) + 1;
		if (instrumentCount < 2)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
			throw PUserException();
		}

		ins = new Ins[instrumentCount];
		if (ins == NULL)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < instrumentCount; i++)
		{
			ins[i].steps  = NULL;
			ins[i].amp    = NULL;
			ins[i].filter = NULL;
		}

		// Allocate dummy instrument
		ins[0].filterPoints = 1;
		ins[0].ampPoints    = 1;
		ins[0].filter       = new TimeLev[1];
		ins[0].amp          = new TimeLev[1];

		ins[0].filterMode   = 0;
		ins[0].clipMode     = 0;
		ins[0].boost        = 1;

		ins[0].sps          = 30;
		ins[0].res          = 0;

		ins[0].vibS         = 1;
		ins[0].vibD         = 1;

		ins[0].pwmS         = 1;
		ins[0].pwmD         = 1;

		ins[0].len          = 0;
		ins[0].loop         = 0;

		ins[0].steps        = new InsStep[1];

		if ((ins[0].filter == NULL) || (ins[0].amp == NULL) || (ins[0].steps == NULL))
		{
			errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
			throw PUserException();
		}

		ins[0].filter[0].time    = 0;
		ins[0].filter[0].lev     = 0;

		ins[0].amp[0].time       = 0;
		ins[0].amp[0].lev        = 0;

		ins[0].steps[0].note     = 0;
		ins[0].steps[0].relative = false;
		ins[0].steps[0].wForm    = 0;

		// Read the instruments
		for (i = 1; i < instrumentCount; i++)
		{
			ins[i].filterPoints = Read8Bit(file);
			if (ins[i].filterPoints < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			ins[i].filter = new TimeLev[ins[i].filterPoints];
			if (ins[i].filter == NULL)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
				throw PUserException();
			}

			for (k = 0; k < ins[i].filterPoints; k++)
			{
				ins[i].filter[k].time = Read8Bit(file);
				ins[i].filter[k].lev  = Read8Bit(file);
			}

			ins[i].ampPoints = Read8Bit(file);
			if (ins[i].ampPoints < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			ins[i].amp = new TimeLev[ins[i].ampPoints];
			if (ins[i].amp == NULL)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
				throw PUserException();
			}

			for (k = 0; k < ins[i].ampPoints; k++)
			{
				ins[i].amp[k].time = Read8Bit(file);
				ins[i].amp[k].lev  = Read8Bit(file);
			}

			ins[i].filterMode = Read8Bit(file);
			ins[i].clipMode   = Read8Bit(file);
			ins[i].boost      = ins[i].clipMode & 15;
			ins[i].clipMode >>= 4;

			ins[i].vibS       = Read8Bit(file);
			ins[i].vibD       = Read8Bit(file);
			ins[i].pwmS       = Read8Bit(file);
			ins[i].pwmD       = Read8Bit(file);
			ins[i].res        = Read8Bit(file);
			ins[i].sps        = Read8Bit(file);
			if (ins[i].sps < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			if (stVersion < 900)
			{
				uint8 tmp   = Read8Bit(file);
				ins[i].len  = tmp & 127;
				ins[i].loop = (tmp & 1) ? 0 : (ins[i].len - 1);
			}
			else
			{
				ins[i].len  = Read8Bit(file);
				ins[i].loop = Read8Bit(file);
				if (ins[i].loop >= ins[i].len)
				{
					errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
					throw PUserException();
				}
			}

			if (ins[i].len < 1)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}

			ins[i].steps = new InsStep[ins[i].len];
			if (ins[i].steps == NULL)
			{
				errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
				throw PUserException();
			}

			for (k = 0; k < ins[i].len; k++)
			{
				uint8 temp               = Read8Bit(file);
				ins[i].steps[k].relative = (temp & 0x80) != 0;
				ins[i].steps[k].wForm    = (temp & 0xf);
				ins[i].steps[k].note     = Read8Bit(file);
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_SAW_ERR_LOADING_INSTRUMENTS);
				throw PUserException();
			}
		}

		//
		// Breakpoints
		//
		breakPCount = Read8Bit(file);
		breakPoints = new BreakPoint[breakPCount];
		if (breakPoints == NULL)
		{
			errorStr.LoadString(res, IDS_SAW_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < breakPCount; i++)
		{
			breakPoints[i].pal     = Read32Bit(file);
			breakPoints[i].command = Read32Bit(file);
		}

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_SAW_ERR_LOADING_BREAKPOINTS);
			throw PUserException();
		}

		//
		// Names
		//
		name   = ReadString(file);
		author = ReadString(file);

		for (i = 0; i < partCount; i++)
			parts[i].name = ReadString(file);

		for (i = 1; i < instrumentCount; i++)
			ins[i].name = ReadString(file);

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
bool Sawteeth::InitPlayer(int32 index)
{
	uint8 i;
	int16 j;
	ChStep *step;
	float framePerSec;
	float total = 0.0f;

	// Create player objects
	p = new Player *[channelCount];
	if (p == NULL)
		return (false);

	for (i = 0; i < channelCount; i++)
		p[i] = NULL;

	for (i = 0; i < channelCount; i++)
	{
		p[i] = new Player(this, &chan[i], i);
		if (p[i] == NULL)
			return (false);
	}

	// Allocate buffers to hold the output to play
	outBuffers = new int16 *[channelCount];
	if (outBuffers == NULL)
		return (false);

	memset(outBuffers, 0, channelCount * sizeof(int16 *));

	for (i = 0; i < channelCount; i++)
	{
		outBuffers[i] = new int16[spsPal];
		if (outBuffers[i] == NULL)
			return (false);
	}

	// Initialize the player
	Init();

	// Find the channel with the maximum number of positions
	songLen = 0;
	for (i = 0; i < channelCount; i++)
	{
		if (chan[i].rLoop >= songLen)
		{
			songLen    = chan[i].rLoop;
			posChannel = i;
		}
	}

	songLen++;

	// Calculate the position times
	step        = chan[posChannel].steps;
	framePerSec = 44100.0f / spsPal;

	for (j = 0; j < songLen; j++)
	{
		// Add the position information to the list
		posInfoList.AddTail(total);

		// Find the time the current position use
		total += (1000.0f * (parts[step[j].part].sps * parts[step[j].part].len) / framePerSec);
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
void Sawteeth::EndPlayer(int32 index)
{
	uint8 i;

	// Delete tables
	delete[] r2f;
	r2f = NULL;

	delete[] n2f;
	n2f = NULL;

	// Delete the output buffers
	if (outBuffers != NULL)
	{
		for (i = 0; i < channelCount; i++)
			delete[] outBuffers[i];

		delete[] outBuffers;
		outBuffers = NULL;
	}

	// Delete player objects
	if (p != NULL)
	{
		for (i = 0; i < channelCount; i++)
			delete p[i];

		delete[] p;
		p = NULL;
	}

	// Delete the song
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void Sawteeth::InitSound(int32 index, uint16 songNum)
{
	uint8 c;

	for (c = 0; c < channelCount; c++)
		p[c]->Init();

	InitSong();
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void Sawteeth::Play(void)
{
	uint8 c;
	uint16 cnt;
	float channelMul;

	// Find the multiply value
	channelMul = cMul[channelCount] / 255.0f;

	// Get the buffer for each channel and create the output
	for (c = 0; c < channelCount; c++)
	{
		if (p[c]->NextBuffer())
			MemMulMove(outBuffers[c], p[c]->Buffer(), spsPal, 255.0f * channelMul);
		else
			memset(outBuffers[c], 0, spsPal * sizeof(int16));

		// Tell APlayer what to play in this channel
		virtChannels[c]->SetBuffer(outBuffers[c], spsPal);
		virtChannels[c]->SetLeftVolume(chan[c].left);
		virtChannels[c]->SetRightVolume(chan[c].right);
	}

	// PAL looping
	pals++;
	if (p[0]->Looped())
	{
		looped = true;
		pals   = 0;

		for (cnt = 0; cnt < chan[0].lLoop; cnt++)
			pals += (parts[chan[0].steps[cnt].part].sps * parts[chan[0].steps[cnt].part].len);
	}

	// Break point part
	if (looped)
		looped     = false;
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void Sawteeth::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = 44100;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString Sawteeth::GetModuleName(void)
{
	return (name);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString Sawteeth::GetAuthor(void)
{
	return (author);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 Sawteeth::GetModuleChannels(void)
{
	return (channelCount);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 Sawteeth::GetSongLength(void)
{
	return (songLen);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 Sawteeth::GetSongPosition(void)
{
	return (p[posChannel]->GetSeqPos());
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void Sawteeth::SetSongPosition(int16 pos)
{
	ChStep *step;
	int16 i;
	uint32 pal = 0;

	// Find the pal value for the position to set
	step = chan[posChannel].steps;
	for (i = 0; i < pos; i++)
		pal += parts[step[i].part].sps * parts[step[i].part].len;

	// Now set the position
	SetPos(pal);
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
PTimeSpan Sawteeth::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	// Copy the position times
	posTimes = posInfoList;

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
bool Sawteeth::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 4)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song version
		case 0:
		{
			description.LoadString(res, IDS_SAW_INFODESCLINE0);
			value.Format("%.2f", stVersion / 1000.0f);
			break;
		}

		// Song Length
		case 1:
		{
			description.LoadString(res, IDS_SAW_INFODESCLINE1);
			value.SetUNumber(songLen);
			break;
		}

		// Used Parts
		case 2:
		{
			description.LoadString(res, IDS_SAW_INFODESCLINE2);
			value.SetUNumber(partCount);
			break;
		}

		// Used Instruments
		case 3:
		{
			description.LoadString(res, IDS_SAW_INFODESCLINE3);
			value.SetUNumber(instrumentCount);
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
bool Sawteeth::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	// First check the sample number for "out of range"
	if (num >= instrumentCount)
		return (false);

	// Fill out the sample info structure
	info->name       = ins[num].name;
	info->flags      = 0;
	info->type       = apSynth;
	info->bitSize    = 16;
	info->middleC    = 0;
	info->volume     = 256;
	info->panning    = -1;
	info->address    = NULL;
	info->length     = 0;
	info->loopStart  = 0;
	info->loopLength = 0;

	return (true);
}



/******************************************************************************/
/* ReadString() will read a string from the file given.                       */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read from.                      */
/*                                                                            */
/* Output: The string read.                                                   */
/******************************************************************************/
PString Sawteeth::ReadString(PFile *file)
{
	PString str;
	PCharSet_UTF8 charSet;
	uint32 i;
	uint8 byte;
	char buf[300];

	for (i = 0; i < sizeof(buf) - 1; i++)
	{
		byte = file->Read_UINT8();
		if (file->IsEOF())
			break;

		if ((byte == 0x00) || (byte == 0x0a))
			break;

		buf[i] = byte;
	}

	buf[i] = 0x00;

	// Set the string
	str.SetString(buf, &charSet);

	return (str);
}



/******************************************************************************/
/* Read8Bit() will read a 8 bit number from the file given.                   */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read from.                      */
/*                                                                            */
/* Output: The 8 bit number.                                                  */
/******************************************************************************/
uint8 Sawteeth::Read8Bit(PFile *file)
{
	if (mark == 'SWTT')
	{
		PString str;

		for (;;)
		{
			str = ReadString(file);
			if (str.IsEmpty())
				continue;

			if (str.Left(2) != "//")
				break;
		}

		return (str.GetUNumber());
	}

	return (file->Read_UINT8());
}



/******************************************************************************/
/* Read16Bit() will read a 16 bit number from the file given.                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read from.                      */
/*                                                                            */
/* Output: The 16 bit number.                                                 */
/******************************************************************************/
uint16 Sawteeth::Read16Bit(PFile *file)
{
	if (mark == 'SWTT')
	{
		PString str;

		for (;;)
		{
			str = ReadString(file);
			if (str.IsEmpty())
				continue;

			if (str.Left(2) != "//")
				break;
		}

		return (str.GetUNumber());
	}

	return (file->Read_B_UINT16());
}



/******************************************************************************/
/* Read32Bit() will read a 32 bit number from the file given.                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read from.                      */
/*                                                                            */
/* Output: The 32 bit number.                                                 */
/******************************************************************************/
uint32 Sawteeth::Read32Bit(PFile *file)
{
	if (mark == 'SWTT')
	{
		PString str;

		for (;;)
		{
			str = ReadString(file);
			if (str.IsEmpty())
				continue;

			if (str.Left(2) != "//")
				break;
		}

		return (str.GetUNumber());
	}

	return (file->Read_B_UINT32());
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void Sawteeth::Cleanup(void)
{
	uint8 i;

	// Delete the song
	delete[] breakPoints;
	breakPoints = NULL;

	if (ins != NULL)
	{
		for (i = 0; i < instrumentCount; i++)
		{
			delete[] ins[i].steps;
			delete[] ins[i].amp;
			delete[] ins[i].filter;
		}

		delete[] ins;
		ins = NULL;
	}

	if (parts != NULL)
	{
		for (i = 0; i < partCount; i++)
			delete[] parts[i].steps;

		delete[] parts;
		parts = NULL;
	}

	if (chan != NULL)
	{
		for (i = 0; i < channelCount; i++)
			delete[] chan[i].steps;

		delete[] chan;
		chan = NULL;
	}
}



/******************************************************************************/
/* Init() initialize all the player structures etc.                           */
/******************************************************************************/
void Sawteeth::Init(void)
{
	int32 c;
	int32 count, oc;
	double octBase;

	// Calc multichannels
	{
		const float floor = 0.1f;

		for (c = 0; c < CHN; c++)
			cMul[c] = ((1.0f - floor) / (float)c) + floor;
	}

	// Freq-tables
	{
		const double MUL = 1.0594630943593;

		n2f = new float[22 * 12];
		if (n2f == NULL)
			throw PMemoryException();

		r2f = new float[22 * 12];
		if (r2f == NULL)
			throw PMemoryException();

		count   = 0;
		octBase = 1.02197486445547712033;

		for (oc = 0; oc < 22; oc++)
		{
			double base = octBase;
			for (c = 0; c < 12; c++)
			{
				n2f[count++] = (float)base;
				base        *= MUL;
			}

			octBase *= 2.0;
		}

		count   = 0;
		octBase = 1.0f;

		for (oc = 0; oc < 22; oc++)
		{
			double base = octBase;
			for (c = 0; c < 12; c++)
			{
				r2f[count++] = (float)base;
				base        *= MUL;
			}

			octBase *= 2.0;
		}
	}
}



/******************************************************************************/
/* InitSong() initialize all the song variables.                              */
/******************************************************************************/
void Sawteeth::InitSong(void)
{
	looped = false;
	pals   = 0;
}



/******************************************************************************/
/* SetPos() will change the song position to the number of pals given.        */
/*                                                                            */
/* Input:  "newPals" is the new position.                                     */
/******************************************************************************/
void Sawteeth::SetPos(uint32 newPals)
{
	int32 c, cnt;

	pals = newPals;

	for (c = 0; c < channelCount; c++)
	{
		// Set seqcount
		int32 sum = 0;
		int32 seqCount = 0, stepCount = 0, palCount = 0, sps = 0, len = 0;

		while (sum <= (int32)pals)
		{
			len  = parts[chan[c].steps[seqCount].part].len;
			sps  = parts[chan[c].steps[seqCount].part].sps;
			sum += sps * len;

			if (seqCount > chan[c].rLoop)
				seqCount = chan[c].lLoop;

			seqCount++;
		}

		if (seqCount > 0)
			seqCount--;
		else
			seqCount = chan[c].rLoop;

		len  = parts[chan[c].steps[seqCount].part].len;
		sps  = parts[chan[c].steps[seqCount].part].sps;

		sum -= sps * len;

		stepCount = (pals - sum) / sps;
		palCount  = (pals - sum) % sps;

		p[c]->JumpPos(seqCount, stepCount, palCount);

		if (c == 0)
		{
			pals = 0;
			for (cnt = 0; cnt < seqCount; cnt++)
				pals += (parts[chan[0].steps[cnt].part].sps * parts[chan[0].steps[cnt].part].len);

			pals += stepCount * parts[chan[0].steps[seqCount].part].sps;
			pals += palCount;
		}
	}
}



/******************************************************************************/
/* MemMulMove() converts the sample output from float to 16-bit for a single  */
/*      channel.                                                              */
/*                                                                            */
/* Input:  "d" is where to store the converted sample.                        */
/*         "s" is where to read the sample to convert.                        */
/*         "count" is the number of samples to convert.                       */
/*         "level" is the volume level.                                       */
/******************************************************************************/
void Sawteeth::MemMulMove(int16 *d, float *s, uint32 count, float level)
{
	level *= 32768.0f;

	while (count > 4)
	{
		d[0] = (int16)(s[0] * level);
		d[1] = (int16)(s[1] * level);
		d[2] = (int16)(s[2] * level);
		d[3] = (int16)(s[3] * level);
		d += 4;
		s += 4;
		count -= 4;
	}

	while (count--)
	{
		d[0] = (int16)(s[0] * level);
		d++;
		s++;
	}
}
