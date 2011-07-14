/******************************************************************************/
/* JamCracker Player Interface.                                               */
/*                                                                            */
/* Original player by M. Gemmel.                                              */
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
#include "JamCracker.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.05f



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static const uint16 periods[] =
{
	1019, 962, 908, 857, 809, 763, 720, 680, 642, 606, 572, 540, 509,
	 481, 454, 428, 404, 381, 360, 340, 321, 303, 286, 270, 254, 240,
	 227, 214, 202, 190, 180, 170, 160, 151, 143, 135, 135, 135, 135,
	 135, 135, 135, 135, 135, 135, 135, 135, 135, 135, 135, 135
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
JamCracker::JamCracker(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	instTable = NULL;
	pattTable = NULL;
	songTable = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
JamCracker::~JamCracker(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float JamCracker::GetVersion(void)
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
uint32 JamCracker::GetSupportFlags(int32 index)
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
PString JamCracker::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_JAM_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString JamCracker::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_JAM_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString JamCracker::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_JAM_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a JamCracker module.         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result JamCracker::ModuleCheck(int32 index, PFile *file)
{
	// Check the module size
	if (file->GetLength() < 6)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();

	if (file->Read_B_UINT32() == 'BeEp')
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
ap_result JamCracker::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	int32 i, j;
	ap_result retVal = AP_ERROR;

	try
	{
		// Skip the module mark
		file->Read_B_UINT32();

		// Get the number of instruments
		samplesNum = file->Read_B_UINT16();

		// Allocate the instrument structures
		instTable = new InstInfo[samplesNum];
		if (instTable == NULL)
		{
			errorStr.LoadString(res, IDS_JAM_ERR_MEMORY);
			throw PUserException();
		}

		// Clear all pointers in the structure
		for (i = 0; i < samplesNum; i++)
			instTable[i].address = NULL;

		// Read the instrument info
		for (i = 0; i < samplesNum; i++)
		{
			file->ReadString(instTable[i].name, 31);

			instTable[i].flags = file->Read_UINT8();
			instTable[i].size  = file->Read_B_UINT32();
			file->Read_B_UINT32();		// Skip the address
		}

		// Get the number of patterns
		patternNum = file->Read_B_UINT16();

		// Allocate the pattern structures
		pattTable = new PattInfo[patternNum];
		if (pattTable == NULL)
		{
			errorStr.LoadString(res, IDS_JAM_ERR_MEMORY);
			throw PUserException();
		}

		// Clear all pointers in the structure
		for (i = 0; i < patternNum; i++)
			pattTable[i].address = NULL;

		// Read the pattern information
		for (i = 0; i < patternNum; i++)
		{
			pattTable[i].size = file->Read_B_UINT16();
			file->Read_B_UINT32();		// Skip the address
		}

		// Get the song length
		songLen = file->Read_B_UINT16();

		// Allocate and read the position array
		songTable = new uint16[songLen];
		if (songTable == NULL)
		{
			errorStr.LoadString(res, IDS_JAM_ERR_MEMORY);
			throw PUserException();
		}

		file->ReadArray_B_UINT16s(songTable, songLen);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_JAM_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Get the pattern data
		for (i = 0; i < patternNum; i++)
		{
			NoteInfo *note;

			// Allocate the pattern data
			note = new NoteInfo[pattTable[i].size * 4];
			if (note == NULL)
			{
				errorStr.LoadString(res, IDS_JAM_ERR_MEMORY);
				throw PUserException();
			}

			pattTable[i].address = note;

			// Read the data from the file
			for (j = 0; j < pattTable[i].size * 4; j++)
			{
				note[j].period   = file->Read_UINT8();
				note[j].instr    = file->Read_UINT8();
				note[j].speed    = file->Read_UINT8();
				note[j].arpeggio = file->Read_UINT8();
				note[j].vibrato  = file->Read_UINT8();
				note[j].phase    = file->Read_UINT8();
				note[j].volume   = file->Read_UINT8();
				note[j].porta    = file->Read_UINT8();
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_JAM_ERR_LOADING_PATTERNS);
				throw PUserException();
			}
		}

		// Read the samples
		for (i = 0; i < samplesNum; i++)
		{
			if (instTable[i].size != 0)
			{
				instTable[i].address = new int8[instTable[i].size];

				// Bug fix for some corrupted modules
				memset(instTable[i].address, 0, instTable[i].size);

				file->Read(instTable[i].address, instTable[i].size);

				if ((file->IsEOF()) && (i != samplesNum - 1))
				{
					errorStr.LoadString(res, IDS_JAM_ERR_LOADING_SAMPLES);
					throw PUserException();
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
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool JamCracker::InitPlayer(int32 index)
{
	PattInfo *pattInfo;
	NoteInfo *noteInfo;
	uint16 i, j, k;
	uint16 noteCount;
	PosInfo posInfo;
	uint8 curSpeed = 6;
	float total = 0.0f;

	// Calculate the position times
	for (i = 0; i < songLen; i++)
	{
		// Add the position information to the list
		posInfo.speed = curSpeed;
		posInfo.time.SetTimeSpan(total);
		posInfoList.AddTail(posInfo);

		// Get pointer to next pattern
		pattInfo  = &pattTable[songTable[i]];
		noteCount = pattInfo->size;
		noteInfo  = pattInfo->address;

		for (j = 0; j < noteCount; j++)
		{
			for (k = 0; k < 4; k++)
			{
				// Should the speed be changed
				if (noteInfo->speed & 15)
					curSpeed = noteInfo->speed & 15;

				// Next channel
				noteInfo++;
			}

			// Add the row time
			total += (1000.0f * curSpeed / 50.0f);
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
void JamCracker::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void JamCracker::InitSound(int32 index, uint16 songNum)
{
	PattInfo *pattPoi;
	uint16 waveOff;

	// Initialize other variables
	songPos = 0;
	songCnt = songLen;

	pattPoi = &pattTable[songTable[0]];
	noteCnt = pattPoi->size;
	address = pattPoi->address;

	wait    = 6;
	waitCnt = 1;

	// Initialize channel variables
	waveOff = 0x80;

	for (uint8 i = 0; i < 4; i++)
	{
		variables[i].waveOffset = waveOff;
		variables[i].dmacon     = 1 << i;
		variables[i].channel    = virtChannels[i];
		variables[i].insLen     = 0;
		variables[i].insAddress = NULL;
		variables[i].perAddress = periods;
		variables[i].pers[0]    = 1019;
		variables[i].pers[1]    = 0;
		variables[i].pers[2]    = 0;
		variables[i].por        = 0;
		variables[i].deltaPor   = 0;
		variables[i].porLevel   = 0;
		variables[i].vib        = 0;
		variables[i].deltaVib   = 0;
		variables[i].vol        = 0;
		variables[i].deltaVol   = 0;
		variables[i].volLevel   = 0x40;
		variables[i].phase      = 0;
		variables[i].deltaPhase = 0;
		variables[i].vibCnt     = 0;
		variables[i].vibMax     = 0;
		variables[i].flags      = 0;

		waveOff += 0x40;
	}
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void JamCracker::Play(void)
{
	if (--waitCnt == 0)
	{
		NewNote();
		waitCnt = wait;
	}

	SetChannel(&variables[0]);
	SetChannel(&variables[1]);
	SetChannel(&variables[2]);
	SetChannel(&variables[3]);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 JamCracker::GetSongLength(void)
{
	return (songLen);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 JamCracker::GetSongPosition(void)
{
	return (songLen - songCnt);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void JamCracker::SetSongPosition(int16 pos)
{
	PattInfo *pattInfo;

	// Change the position
	songCnt = songLen - pos;
	songPos = pos;

	pattInfo = &pattTable[songTable[songPos]];
	noteCnt  = pattInfo->size;
	address  = pattInfo->address;

	// Change the speed
	waitCnt  = 1;
	wait     = posInfoList.GetItem(pos).speed;
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
PTimeSpan JamCracker::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
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
bool JamCracker::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 3)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Song Length
		case 0:
		{
			description.LoadString(res, IDS_JAM_INFODESCLINE0);
			value.SetUNumber(songLen);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_JAM_INFODESCLINE1);
			value.SetUNumber(patternNum);
			break;
		}

		// Used Instruments
		case 2:
		{
			description.LoadString(res, IDS_JAM_INFODESCLINE2);
			value.SetUNumber(samplesNum);
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
bool JamCracker::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	PCharSet_Amiga charSet;
	InstInfo *sample;

	// First check the sample number for "out of range"
	if (num >= samplesNum)
		return (false);

	// Get the pointer to the sample data
	sample = &instTable[num];

	// Fill out the sample info structure
	info->name.SetString(sample->name, &charSet);
	info->bitSize = 8;
	info->middleC = 8287;
	info->volume  = 256;
	info->panning = -1;

	if (sample->flags & 2)
	{
		// AM sample
		info->type       = apSynth;
		info->flags      = 0;
		info->address    = NULL;
		info->length     = 0;
		info->loopStart  = 0;
		info->loopLength = 0;
	}
	else
	{
		// Normal sample
		info->type    = apSample;
		info->address = sample->address;
		info->length  = sample->size;

		if (sample->flags & 1)
		{
			// Sample loops
			info->flags      = APSAMP_LOOP;
			info->loopStart  = 0;
			info->loopLength = sample->size;
		}
		else
		{
			// No loop
			info->flags      = 0;
			info->loopStart  = 0;
			info->loopLength = 0;
		}
	}

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void JamCracker::Cleanup(void)
{
	int32 i;

	if (instTable != NULL)
	{
		for (i = 0; i < samplesNum; i++)
			delete[] instTable[i].address;
	}

	if (pattTable != NULL)
	{
		for (i = 0; i < patternNum; i++)
			delete[] pattTable[i].address;
	}

	delete[] songTable;
	delete[] pattTable;
	delete[] instTable;
}



/******************************************************************************/
/* NewNote() will get a new note from the current pattern + skip to next      */
/*      pattern if necessary.                                                 */
/******************************************************************************/
void JamCracker::NewNote(void)
{
	NoteInfo *adr;
	PattInfo *pattInfo;

	adr = address;
	address += 4;		// Go to the next row

	if (--noteCnt == 0)
	{
		songPos++;
		if (--songCnt == 0)
		{
			songPos = 0;
			songCnt = songLen;

			//
			// Next module
			//
			endReached = true;
		}

		//
		// Next position
		//
		ChangePosition();

		pattInfo = &pattTable[songTable[songPos]];
		noteCnt  = pattInfo->size;
		address  = pattInfo->address;
	}

	tmpDMACON = 0;
	NwNote(adr, &variables[0]);
	NwNote(++adr, &variables[1]);
	NwNote(++adr, &variables[2]);
	NwNote(++adr, &variables[3]);

	SetVoice(&variables[0]);
	SetVoice(&variables[1]);
	SetVoice(&variables[2]);
	SetVoice(&variables[3]);
}



/******************************************************************************/
/* NwNote() is a sub function to NewNote().                                   */
/*                                                                            */
/* Input:  "adr" is a pointer to a noteinfo structure.                        */
/*         "voice" is a pointer to the channels voice structure.              */
/******************************************************************************/
void JamCracker::NwNote(NoteInfo *adr, VoiceInfo *voice)
{
	const uint16 *perAdr;
	InstInfo *instInfo;
	int8 *sampAdr;
	int16 temp;

	if (adr->period != 0)
	{
		perAdr = adr->period + (periods - 1);

		if (adr->speed & 64)
			voice->porLevel = *perAdr;
		else
		{
			tmpDMACON |= voice->dmacon;

			voice->perAddress = perAdr;
			voice->pers[0]    = *perAdr;
			voice->pers[1]    = *perAdr;
			voice->pers[2]    = *perAdr;

			voice->por = 0;

			if (adr->instr > samplesNum)
			{
				voice->insAddress = NULL;
				voice->insLen     = 0;
				voice->flags      = 0;
			}
			else
			{
				instInfo = &instTable[adr->instr];
				if (instInfo->address == NULL)
				{
					voice->insAddress = NULL;
					voice->insLen     = 0;
					voice->flags      = 0;
				}
				else
				{
					sampAdr = instInfo->address;

					if (!(instInfo->flags & 2))
						voice->insLen = (uint16)(instInfo->size / 2);
					else
					{
						sampAdr += voice->waveOffset;
						voice->insLen = 0x20;
					}

					voice->insAddress = sampAdr;
					voice->flags      = instInfo->flags;
					voice->vol        = voice->volLevel;
				}
			}
		}
	}

	if (adr->speed & 15)
		wait = adr->speed & 15;

	// Do arpeggio
	perAdr = voice->perAddress;

	if (adr->arpeggio != 0)
	{
		if (adr->arpeggio == 255)
		{
			voice->pers[0] = *perAdr;
			voice->pers[1] = *perAdr;
			voice->pers[2] = *perAdr;
		}
		else
		{
			voice->pers[2] = *(perAdr + (adr->arpeggio & 15));
			voice->pers[1] = *(perAdr + (adr->arpeggio >> 4));
			voice->pers[0] = *perAdr;
		}
	}

	// Do vibrato
	if (adr->vibrato != 0)
	{
		if (adr->vibrato == 255)
		{
			voice->vib      = 0;
			voice->deltaVib = 0;
			voice->vibCnt   = 0;
		}
		else
		{
			voice->vib      = 0;
			voice->deltaVib = adr->vibrato & 15;
			voice->vibMax   = adr->vibrato >> 4;
			voice->vibCnt   = adr->vibrato >> 5;
		}
	}

	// Do phase
	if (adr->phase != 0)
	{
		if (adr->phase == 255)
		{
			voice->phase      = 0;
			voice->deltaPhase = -1;
		}
		else
		{
			voice->phase      = 0;
			voice->deltaPhase = adr->phase & 15;
		}
	}

	// Do volume
	if (!(temp = adr->volume))
	{
		if (adr->speed & 128)
		{
			voice->vol      = temp;
			voice->volLevel = temp;
			voice->deltaVol = 0;
		}
	}
	else
	{
		if (temp == 255)
			voice->deltaVol = 0;
		else
		{
			if (adr->speed & 128)
			{
				voice->vol      = temp;
				voice->volLevel = temp;
				voice->deltaVol = 0;
			}
			else
			{
				temp &= 0x7f;
				if (adr->volume & 128)
					temp = -temp;

				voice->deltaVol = temp;
			}
		}
	}

	// Do portamento
	if ((temp = adr->porta) != 0)
	{
		if (temp == 255)
		{
			voice->por      = 0;
			voice->deltaPor = 0;
		}
		else
		{
			voice->por = 0;
			if (adr->speed & 64)
			{
				if (voice->porLevel <= voice->pers[0])
					temp = -temp;
			}
			else
			{
				temp &= 0x7f;
				if (!(adr->porta & 128))
				{
					temp = -temp;
					voice->porLevel = 135;
				}
				else
					voice->porLevel = 1019;
			}

			voice->deltaPor = temp;
		}
	}
}



/******************************************************************************/
/* SetVoice() will setup the NotePlayer structure.                            */
/*                                                                            */
/* Input:  "voice" is a pointer to the channels voice structure.              */
/******************************************************************************/
void JamCracker::SetVoice(VoiceInfo *voice)
{
	APChannel *chan;

	if (tmpDMACON & voice->dmacon)
	{
		chan = voice->channel;

		// Setup the start sample
		if (voice->insAddress == NULL)
			chan->Mute();
		else
		{
			chan->PlaySample(voice->insAddress, 0, voice->insLen * 2);
			chan->SetAmigaPeriod(voice->pers[0]);
		}

		// Check to see if sample loops
		if (!(voice->flags & 1))
		{
			voice->insAddress = NULL;
			voice->insLen     = 0;
		}

		// Setup loop
		if (voice->insAddress != NULL)
			chan->SetLoop(0, voice->insLen * 2);
	}
}



/******************************************************************************/
/* SetChannel() will setup the channel.                                       */
/*                                                                            */
/* Input:  "voice" is a pointer to the channels voice structure.              */
/******************************************************************************/
void JamCracker::SetChannel(VoiceInfo *voice)
{
	APChannel *chan = voice->channel;
	int16 per;
	int16 offset;
	int8 *instData;
	int8 *wave, *wavePhase;

	while (voice->pers[0] == 0)
		RotatePeriods(voice);

	per = voice->pers[0] + voice->por;
	if (voice->por < 0)
	{
		if (per < voice->porLevel)
			per = voice->porLevel;
	}
	else
	{
		if (voice->por != 0)
			if (per > voice->porLevel)
				per = voice->porLevel;
	}

	// Add vibrato
	per += voice->vib;

	if (per < 135)
		per = 135;
	else
		if (per > 1019)
			per = 1019;

	chan->SetAmigaPeriod(per);
	RotatePeriods(voice);

	voice->por += voice->deltaPor;
	if (voice->por < -1019)
		voice->por = -1019;
	else
		if (voice->por > 1019)
			voice->por = 1019;

	if (voice->vibCnt)
	{
		voice->vib += voice->deltaVib;
		if (--voice->vibCnt == 0)
		{
			voice->deltaVib = -voice->deltaVib;
			voice->vibCnt   = voice->vibMax;
		}
	}

	chan->SetVolume(voice->vol * 4);

	voice->vol += voice->deltaVol;
	if (voice->vol < 0)
		voice->vol = 0;
	else
		if (voice->vol > 64)
			voice->vol = 64;

	if ((voice->flags & 1) && (voice->deltaPhase != 0))
	{
		if (voice->deltaPhase < 0)
			voice->deltaPhase = 0;

		instData  = voice->insAddress;
		offset    = -voice->waveOffset;
		wave      = instData + offset;
		wavePhase = wave + voice->phase / 4;

		for (uint8 i = 0; i < 64; i++)
		{
			int16 temp = *wave++;
			temp += *wavePhase++;
			temp >>= 1;
			*instData++ = (int8)temp;
		}

		voice->phase += voice->deltaPhase;
		if (voice->phase >= 256)
			voice->phase -= 256;
	}
}



/******************************************************************************/
/* RotatePeriods() rotates the periods in the pv_pers array.                  */
/*                                                                            */
/* Input:  "voice" is a pointer to the channels voice structure.              */
/******************************************************************************/
void JamCracker::RotatePeriods(VoiceInfo *voice)
{
	uint16 temp;

	temp = voice->pers[0];
	voice->pers[0] = voice->pers[1];
	voice->pers[1] = voice->pers[2];
	voice->pers[2] = temp;
}
