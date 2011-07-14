/******************************************************************************/
/* SoundMonitor Player Interface.                                             */
/*                                                                            */
/* Original player by Brian Postma.                                           */
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
#include "SoundMonitor.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.03f



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static const int16 periods[] =
{
	6848, 6464, 6080, 5760, 5440, 5120, 4832, 4576, 4320, 4064, 3840, 3616,
	3424, 3232, 3040, 2880, 2720, 2560, 2416, 2288, 2160, 2032, 1920, 1808,
	1712, 1616, 1520, 1440, 1360, 1280, 1208, 1144, 1080, 1016,  960,  904,
	 856,  808,  760,  720,  680,  640,  604,  572,  540,  508,  480,  452,
	 428,  404,  380,  360,  340,  320,  302,  286,  270,  254,  240,  226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 107,  101,   95,   90,   85,   80,   76,   72,   68,   64,   60,   57
};



/******************************************************************************/
/* Vibrato table                                                              */
/******************************************************************************/
static const int16 vibTable[] =
{
	0, 64, 128, 64, 0, -64, -128, -64
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SoundMonitor::SoundMonitor(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	memset(instruments, 0, sizeof(instruments));
	steps[0]    = NULL;
	steps[1]    = NULL;
	steps[2]    = NULL;
	steps[3]    = NULL;
	tracks      = NULL;
	waveTables  = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SoundMonitor::~SoundMonitor(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float SoundMonitor::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the add-on.                    */
/*                                                                            */
/* Output: Is the number of the add-ons.                                      */
/******************************************************************************/
int32 SoundMonitor::GetCount(void)
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
uint32 SoundMonitor::GetSupportFlags(int32 index)
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
PString SoundMonitor::GetName(int32 index)
{
	PString name;

	if (index == 0)
		name.LoadString(res, IDS_BP_NAME11);
	else
		name.LoadString(res, IDS_BP_NAME22);

	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString SoundMonitor::GetDescription(int32 index)
{
	PString description;

	description.Format(res, IDS_BP_DESCRIPTION, index + 2);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString SoundMonitor::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_BP_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a SoundMonitor module.       */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result SoundMonitor::ModuleCheck(int32 index, PFile *file)
{
	uint32 mark;

	// Check the module size
	if (file->GetLength() < 512)
		return (AP_UNKNOWN);

	// Read the module mark
	file->Seek(26, PFile::pSeekBegin);
	mark = file->Read_B_UINT32();

	// Check the mark
	if (index == 0)
	{
		if ((mark & 0xffffff00) != 'V.2\0')
			return (AP_UNKNOWN);
	}
	else
	{
		if (((mark & 0xffffff00) != 'V.3\0') && (mark != 'BPSM'))
			return (AP_UNKNOWN);
	}

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
ap_result SoundMonitor::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	uint8 mark[3];
	uint32 i, j;
	ap_result retVal = AP_ERROR;

	try
	{
		// Remember the type of the player
		playerType = (PlayerType)index;

		// Start to read the module name
		file->ReadString(moduleName, 26);
		moduleName[26] = 0x00;

		// Check the mark again
		file->Read(mark, 3);
		if ((mark[0] == 'B') && (mark[1] == 'P') && (mark[2] == 'S'))
		{
			// Skip the last letter in the mark
			file->Seek(1, PFile::pSeekCurrent);
			waveNum = 0;
		}
		else
		{
			// Get the number of waveforms
			waveNum = file->Read_UINT8();
		}

		// Get the number of positions
		stepNum = file->Read_B_UINT16();

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_BP_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Read the sample informations
		for (i = 0; i < 15; i++)
		{
			Instrument *inst = &instruments[i];

			// First find out the kind of the instrument
			if (file->Read_UINT8() == 0xff)
			{
				// Synth instrument
				inst->type = true;
				inst->adr  = NULL;

				if (playerType == SoundMon11)
				{
					// Sound Monitor 1.1
					inst->waveTable   = file->Read_UINT8();
					inst->waveLength  = file->Read_B_UINT16() * 2;
					inst->adsrControl = file->Read_UINT8();
					inst->adsrTable   = file->Read_UINT8();
					inst->adsrLength  = file->Read_B_UINT16();
					inst->adsrSpeed   = file->Read_UINT8();
					inst->lfoControl  = file->Read_UINT8();
					inst->lfoTable    = file->Read_UINT8();
					inst->lfoDepth    = file->Read_UINT8();
					inst->lfoLength   = file->Read_B_UINT16();
					file->Read_UINT8();
					inst->lfoDelay    = file->Read_UINT8();
					inst->lfoSpeed    = file->Read_UINT8();
					inst->egControl   = file->Read_UINT8();
					inst->egTable     = file->Read_UINT8();
					file->Read_UINT8();
					inst->egLength    = file->Read_B_UINT16();
					file->Read_UINT8();
					inst->egDelay     = file->Read_UINT8();
					inst->egSpeed     = file->Read_UINT8();
					inst->fxControl   = 0;
					inst->fxSpeed     = 1;
					inst->fxDelay     = 0;
					inst->modControl  = 0;
					inst->modTable    = 0;
					inst->modSpeed    = 1;
					inst->modDelay    = 0;
					inst->volume      = file->Read_UINT8();
					inst->modLength   = 0;
					file->Seek(6, PFile::pSeekCurrent);
				}
				else
				{
					// Sound Monitor 2.2
					inst->waveTable   = file->Read_UINT8();
					inst->waveLength  = file->Read_B_UINT16() * 2;
					inst->adsrControl = file->Read_UINT8();
					inst->adsrTable   = file->Read_UINT8();
					inst->adsrLength  = file->Read_B_UINT16();
					inst->adsrSpeed   = file->Read_UINT8();
					inst->lfoControl  = file->Read_UINT8();
					inst->lfoTable    = file->Read_UINT8();
					inst->lfoDepth    = file->Read_UINT8();
					inst->lfoLength   = file->Read_B_UINT16();
					inst->lfoDelay    = file->Read_UINT8();
					inst->lfoSpeed    = file->Read_UINT8();
					inst->egControl   = file->Read_UINT8();
					inst->egTable     = file->Read_UINT8();
					inst->egLength    = file->Read_B_UINT16();
					inst->egDelay     = file->Read_UINT8();
					inst->egSpeed     = file->Read_UINT8();
					inst->fxControl   = file->Read_UINT8();
					inst->fxSpeed     = file->Read_UINT8();
					inst->fxDelay     = file->Read_UINT8();
					inst->modControl  = file->Read_UINT8();
					inst->modTable    = file->Read_UINT8();
					inst->modSpeed    = file->Read_UINT8();
					inst->modDelay    = file->Read_UINT8();
					inst->volume      = file->Read_UINT8();
					inst->modLength   = file->Read_B_UINT16();
				}
			}
			else
			{
				// Sample
				file->Seek(-1, PFile::pSeekCurrent);
				file->ReadString(inst->name, 24);

				inst->type       = false;
				inst->length     = file->Read_B_UINT16() * 2;
				inst->loopStart  = file->Read_B_UINT16();
				inst->loopLength = file->Read_B_UINT16() * 2;
				inst->volume     = file->Read_B_UINT16();
				inst->adr        = NULL;

				// Fix for Karate.bp
				if ((inst->loopStart + inst->loopLength) > inst->length)
					inst->loopLength = inst->length - inst->loopStart;
			}

			if (inst->volume > 64)
				inst->volume = 64;

			// Check for "end-of-file"
			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_BP_ERR_LOADING_INSTINFO);
				throw PUserException();
			}
		}

		// Allocate step structures
		for (i = 0; i < 4; i++)
		{
			steps[i] = new Step[stepNum];
			if (steps[i] == NULL)
			{
				errorStr.LoadString(res, IDS_BP_ERR_MEMORY);
				throw PUserException();
			}
		}

		// Read the step data
		trackNum = 0;

		for (i = 0; i < stepNum; i++)
		{
			// Read track 1
			steps[0][i].trackNumber    = file->Read_B_UINT16();
			steps[0][i].soundTranspose = file->Read_UINT8();
			steps[0][i].transpose      = file->Read_UINT8();

			if (steps[0][i].trackNumber > trackNum)
				trackNum = steps[0][i].trackNumber;

			// Read track 2
			steps[1][i].trackNumber    = file->Read_B_UINT16();
			steps[1][i].soundTranspose = file->Read_UINT8();
			steps[1][i].transpose      = file->Read_UINT8();

			if (steps[1][i].trackNumber > trackNum)
				trackNum = steps[1][i].trackNumber;

			// Read track 3
			steps[2][i].trackNumber    = file->Read_B_UINT16();
			steps[2][i].soundTranspose = file->Read_UINT8();
			steps[2][i].transpose      = file->Read_UINT8();

			if (steps[2][i].trackNumber > trackNum)
				trackNum = steps[2][i].trackNumber;

			// Read track 4
			steps[3][i].trackNumber    = file->Read_B_UINT16();
			steps[3][i].soundTranspose = file->Read_UINT8();
			steps[3][i].transpose      = file->Read_UINT8();

			if (steps[3][i].trackNumber > trackNum)
				trackNum = steps[3][i].trackNumber;

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_BP_ERR_LOADING_HEADER);
				throw PUserException();
			}
		}

		// Allocate space to hold the track pointers
		tracks = new Track *[trackNum];
		if (tracks == NULL)
		{
			errorStr.LoadString(res, IDS_BP_ERR_MEMORY);
			throw PUserException();
		}

		memset(tracks, 0, trackNum * sizeof(Track *));

		// Read the tracks
		for (i = 0; i < trackNum; i++)
		{
			// Allocate one track
			tracks[i] = new Track[16];
			if (tracks[i] == NULL)
			{
				errorStr.LoadString(res, IDS_BP_ERR_MEMORY);
				throw PUserException();
			}

			// Read it
			for (j = 0; j < 16; j++)
			{
				tracks[i][j].note         = file->Read_UINT8();
				tracks[i][j].instrument   = file->Read_UINT8();
				tracks[i][j].optional     = tracks[i][j].instrument & 0x0f;
				tracks[i][j].instrument   = (tracks[i][j].instrument & 0xf0) >> 4;
				tracks[i][j].optionalData = file->Read_UINT8();
			}

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_BP_ERR_LOADING_PATTERNS);
				throw PUserException();
			}
		}

		// Allocate space to hold the wave tables
		waveTables = new int8[waveNum * 64];
		if (waveTables == NULL)
		{
			errorStr.LoadString(res, IDS_BP_ERR_MEMORY);
			throw PUserException();
		}

		// Read the wave tables
		file->Read(waveTables, waveNum * 64);

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_BP_ERR_LOADING_SAMPLES);
			throw PUserException();
		}

		// Okay, finally we read the samples
		for (i = 0; i < 15; i++)
		{
			// Is the instrument a sample?
			if (!instruments[i].type)
			{
				// Yep, check the length
				if (instruments[i].length != 0)
				{
					// Allocate space to hold the sample data
					instruments[i].adr = new int8[instruments[i].length];
					if (instruments[i].adr == NULL)
					{
						errorStr.LoadString(res, IDS_BP_ERR_MEMORY);
						throw PUserException();
					}

					// Read the sample data
					file->Read(instruments[i].adr, instruments[i].length);

					if (file->IsEOF())
					{
						errorStr.LoadString(res, IDS_BP_ERR_LOADING_SAMPLES);
						throw PUserException();
					}
				}
			}
		}

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
bool SoundMonitor::InitPlayer(int32 index)
{
	const Track *track;
	int16 i, j, k;
	int16 newPos = -1;
	uint16 trackNum;
	uint16 repCount = 0;
	PosInfo posInfo;
	uint8 curSpeed = 6;
	float total = 0.0f;
	bool fullStop = false;

	// Calculate the position times
	for (i = 0; i < stepNum; i++)
	{
		// Add the position information to the list
		posInfo.speed = curSpeed;
		posInfo.time.SetTimeSpan(total);

		if (i >= posInfoList.CountItems())
			posInfoList.AddTail(posInfo);

		for (j = 0; j < 16; j++)
		{
			for (k = 0; k < 4; k++)
			{
				trackNum = steps[k][i].trackNumber;
				track    = tracks[trackNum - 1] + j;

				if (track->optional == BP_OPT_SETSPEED)
					curSpeed = track->optionalData;

				if (track->optional == BP_OPT_JUMP)
				{
					switch (playerType)
					{
						case SoundMon11:
						{
							if (repCount != 0)
							{
								repCount--;
								if (repCount != 0)
								{
									newPos = track->optionalData;

									// Set the "end" mark, if we have to
									// repeat the block more than 14 times
									if (repCount >= 15)
										fullStop = true;
								}
							}
							break;
						}

						case SoundMon22:
						{
							if (track->optionalData < i)
								fullStop = true;

							break;
						}
					}
				}

				if (track->optional == BP_OPT_VIBRATO)
				{
					if ((playerType == SoundMon11) && (repCount == 0))
						repCount = track->optionalData;
				}
			}

			// Add the row time
			total += (1000.0f * curSpeed / 50.0f);

			if (newPos != -1)
			{
				i = newPos - 1;
				newPos = -1;
				break;
			}

			if (fullStop)
				break;
		}

		if (fullStop)
			break;
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
void SoundMonitor::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void SoundMonitor::InitSound(int32 index, uint16 songNum)
{
	// Initialize member variables
	arpCount   = 1;
	bpCount    = 1;
	bpDelay    = 6;
	bpRepCount = 0;
	vibIndex   = 0;
	bpStep     = 0;
	bpPatCount = 0;
	st         = 0;
	tr         = 0;
	newPos     = 0;
	posFlag    = false;

	// Initialize the bpCurrent structure for each channel
	memset(bpCurrent, 0, sizeof(bpCurrent));

	// Clear the temporary synth data buffer
	memset(synthBuffer, 0, sizeof(synthBuffer));
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void SoundMonitor::Play(void)
{
	// Call the player
	BpPlay();
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString SoundMonitor::GetModuleName(void)
{
	PCharSet_Amiga charSet;
	PString name;

	name.SetString(moduleName, &charSet);
	return (name);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 SoundMonitor::GetSongLength(void)
{
	return (stepNum);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 SoundMonitor::GetSongPosition(void)
{
	return (bpStep);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void SoundMonitor::SetSongPosition(int16 pos)
{
	// Change the position
	bpStep     = pos;
	bpPatCount = 0;
	posFlag    = false;

	// Change the speed
	bpCount    = 1;
	bpDelay    = posInfoList.GetItem(pos).speed;
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
PTimeSpan SoundMonitor::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
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
bool SoundMonitor::GetInfoString(uint32 line, PString &description, PString &value)
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
			description.LoadString(res, IDS_BP_INFODESCLINE0);
			value.SetUNumber(stepNum);
			break;
		}

		// Used Tracks
		case 1:
		{
			description.LoadString(res, IDS_BP_INFODESCLINE1);
			value.SetUNumber(trackNum);
			break;
		}

		// Supported/Used Samples
		case 2:
		{
			description.LoadString(res, IDS_BP_INFODESCLINE2);
			value = "15";
			break;
		}

		// Used Wavetables
		case 3:
		{
			description.LoadString(res, IDS_BP_INFODESCLINE3);
			value.SetUNumber(waveNum);
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
bool SoundMonitor::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	PCharSet_Amiga charSet;
	Instrument *inst;

	// First check the sample number for "out of range"
	if (num >= 15)
		return (false);

	// Get the pointer to the sample data
	inst = &instruments[num];

	// Fill out the sample info structure
	info->bitSize    = 8;
	info->middleC    = 8287;
	info->volume     = inst->volume * 4;
	info->panning    = -1;

	if (inst->type)
	{
		// It's a synth sound
		info->name.MakeEmpty();
		info->type       = apSynth;
		info->address    = NULL;
		info->length     = 0;
		info->flags      = 0;
		info->loopStart  = 0;
		info->loopLength = 0;
	}
	else
	{
		// It's a real sample
		info->name.SetString(inst->name, &charSet);
		info->type    = apSample;
		info->address = inst->adr;
		info->length  = (inst->length > 2 ? inst->length : 0);

		if (inst->loopLength <= 2)
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
			info->loopStart  = inst->loopStart;
			info->loopLength = inst->loopLength;
		}
	}

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void SoundMonitor::Cleanup(void)
{
	uint32 i;

	// Delete the sample datas
	for (i = 0; i < 15; i++)
	{
		delete[] instruments[i].adr;
		instruments[i].adr = NULL;
	}

	// Delete the wave tables
	delete[] waveTables;
	waveTables = NULL;

	// Delete the tracks
	if (tracks != NULL)
	{
		for (i = 0; i < trackNum; i++)
			delete[] tracks[i];

		delete[] tracks;
		tracks = NULL;
	}

	// Delete the step structures
	for (i = 0; i < 4; i++)
	{
		delete[] steps[i];
		steps[i] = NULL;
	}
}



/******************************************************************************/
/* BpPlay() plays the module.                                                 */
/******************************************************************************/
void SoundMonitor::BpPlay(void)
{
	// First run the real-time effects
	DoEffects();

	// Then do the synth voices
	DoSynths();

	// At last, update the positions
	bpCount--;
	if (bpCount == 0)
	{
		uint32 i;

		bpCount = bpDelay;
		BpNext();

		for (i = 0; i < 4; i++)
		{
			// Is the sound restarting?
			if (bpCurrent[i].restart)
			{
				// Copy temporary synth data back
				if (bpCurrent[i].synthPtr != NULL)
				{
					memcpy(bpCurrent[i].synthPtr, synthBuffer[i], 32);
					bpCurrent[i].synthPtr = NULL;
				}

				// Play the sounds
				PlayIt(i);
			}
		}
	}
}



/******************************************************************************/
/* BpNext() change the pattern position and/or step position.                 */
/******************************************************************************/
void SoundMonitor::BpNext(void)
{
	uint32 i;
	uint16 track;
	int8 note;
	uint8 inst;

	for (i = 0; i < 4; i++)
	{
		const Track *curTrack;
		BPCurrent *cur = &bpCurrent[i];

		// Get the step informations
		track = steps[i][bpStep].trackNumber;
		st    = steps[i][bpStep].soundTranspose;
		tr    = steps[i][bpStep].transpose;

		// Find the track address
		curTrack = tracks[track - 1] + bpPatCount;

		// Is there any note?
		note = curTrack->note;
		if (note != 0)
		{
			// Stop the effects
			cur->autoSlide = 0;
			cur->autoArp   = 0;
			cur->vibrato   = 0;

			// Find the note number and period
			if ((note != 0) && ((curTrack->optional != BP_OPT_TRANSPOSE) || ((curTrack->optionalData & 0xf0) == 0)))
				note += tr;

			cur->note    = note;
			cur->period  = periods[note + 36 - 1];
			cur->restart = false;

			// Should the voice be retrigged?
			if (curTrack->optional < BP_OPT_CHANGEINVERSION)
			{
				cur->restart          = true;
				cur->useDefaultVolume = true;
			}

			// Find the instrument
			inst = curTrack->instrument;
			if (inst == 0)
				inst = cur->instrument;

			if ((inst != 0) && ((curTrack->optional != BP_OPT_TRANSPOSE) || ((curTrack->optionalData & 0x0f) == 0)))
			{
				inst += st;
				if ((inst < 1) || (inst > 15))
					inst -= st;
			}

			if ((curTrack->optional < BP_OPT_CHANGEINVERSION) && ((!cur->synthMode) || (cur->instrument != inst)))
				cur->instrument = inst;
		}

		DoOptionals(i, curTrack->optional, curTrack->optionalData);
	}

	// Change the position
	if (posFlag)
	{
		posFlag    = false;
		bpPatCount = 0;
		bpStep     = newPos;
	}
	else
	{
		// Next row in the pattern
		bpPatCount++;
		if (bpPatCount == 16)
		{
			// Done with the pattern, now go to the next step
			bpPatCount = 0;
			bpStep++;

			// Tell APlayer we have changed the position
			ChangePosition();

			if (bpStep == stepNum)
			{
				// Done with the module, repeat it
				bpStep     = 0;
				endReached = true;
			}
		}
	}
}



/******************************************************************************/
/* PlayIt() retrigs a sample or synth sound.                                  */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/******************************************************************************/
void SoundMonitor::PlayIt(uint32 voice)
{
	Instrument *inst;
	BPCurrent *cur = &bpCurrent[voice];
	int16 tmp;

	// Reset the retrig flag
	cur->restart = false;
	virtChannels[voice]->SetAmigaPeriod(cur->period);

	// Get the instrument address
	inst = &instruments[cur->instrument - 1];

	// Is the instrument a synth?
	if (inst->type)
	{
		// Yes it is
		cur->synthMode   = true;
		cur->egPtr       = 0;
		cur->lfoPtr      = 0;
		cur->adsrPtr     = 0;
		cur->modPtr      = 0;

		cur->egCount     = inst->egDelay + 1;
		cur->lfoCount    = inst->lfoDelay + 1;
		cur->adsrCount   = 1;						// Start immediate
		cur->modCount    = inst->modDelay + 1;
		cur->fxCount     = inst->fxDelay + 1;

		cur->fxControl   = inst->fxControl;
		cur->egControl   = inst->egControl;
		cur->lfoControl  = inst->lfoControl;
		cur->adsrControl = inst->adsrControl;
		cur->modControl  = inst->modControl;
		cur->oldEgValue  = 0;

		// Play the synth sound
		int8 *adr = &waveTables[inst->waveTable * 64];
		virtChannels[voice]->PlaySample(adr, 0, inst->waveLength);
		virtChannels[voice]->SetLoop(0, inst->waveLength);

		// Initialize ADSR
		if (cur->adsrControl != 0)
		{
			// Get table value
			tmp = (waveTables[inst->adsrTable * 64] + 128) / 4;

			if (cur->useDefaultVolume)
			{
				cur->volume           = (uint8)inst->volume;
				cur->useDefaultVolume = false;
			}

			tmp = tmp * cur->volume / 16;
			virtChannels[voice]->SetVolume(tmp > 256 ? 256 : tmp);
		}
		else
		{
			tmp = (cur->useDefaultVolume ? inst->volume : cur->volume) * 4;
			virtChannels[voice]->SetVolume(tmp > 256 ? 256 : tmp);
		}

		// Initialize the other effects
		if ((cur->egControl != 0) || (cur->modControl != 0) || (cur->fxControl != 0))
		{
			cur->synthPtr = adr;
			memcpy(synthBuffer[voice], adr, 32);
		}
	}
	else
	{
		// No, it's a sample
		cur->synthMode  = false;
		cur->lfoControl = 0;

		if (inst->adr == NULL)
			virtChannels[voice]->Mute();
		else
		{
			// Play the sample
			virtChannels[voice]->PlaySample(inst->adr, 0, inst->length);

			// Set the loop if any
			if (inst->loopLength > 2)
				virtChannels[voice]->SetLoop(inst->loopStart, inst->loopLength);

			// Set the volume
			tmp = (cur->useDefaultVolume ? inst->volume : cur->volume) * 4;
			virtChannels[voice]->SetVolume(tmp > 256 ? 256 : tmp);
		}
	}
}



/******************************************************************************/
/* DoOptionals() parses the track optionals.                                  */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/*         "optional" is the optional to parse.                               */
/*         "optionalData" is it's data.                                       */
/******************************************************************************/
void SoundMonitor::DoOptionals(uint32 voice, uint8 optional, uint8 optionalData)
{
	BPCurrent *cur = &bpCurrent[voice];

	switch (optional)
	{
		// Arpeggio once
		case BP_OPT_ARPEGGIOONCE:
		{
			cur->arpValue = optionalData;
			break;
		}

		// Set volume
		case BP_OPT_SETVOLUME:
		{
			if (optionalData > 64)
				optionalData = 64;

			cur->volume           = optionalData;
			cur->useDefaultVolume = false;

			if (playerType == SoundMon11)
				virtChannels[voice]->SetVolume(optionalData * 4);
			else
			{
				if (!cur->synthMode)
					virtChannels[voice]->SetVolume(optionalData * 4);
			}
			break;
		}

		// Set speed
		case BP_OPT_SETSPEED:
		{
			bpCount = optionalData;
			bpDelay = optionalData;
			break;
		}

		// Filter control
		case BP_OPT_FILTER:
		{
			amigaFilter = optionalData != 0;
			break;
		}

		// Period lift up
		case BP_OPT_PORTUP:
		{
			cur->period  -= optionalData;
			cur->arpValue = 0;
			break;
		}

		// Period lift down
		case BP_OPT_PORTDOWN:
		{
			cur->period  += optionalData;
			cur->arpValue = 0;
			break;
		}

		// Set repeat count / Vibrato
		case BP_OPT_VIBRATO:
		{
			if (playerType == SoundMon11)
			{
				if (bpRepCount == 0)
					bpRepCount = optionalData;
			}
			else
				cur->vibrato = optionalData;

			break;
		}

		// DBRA repeat count / Jump to step
		case BP_OPT_JUMP:
		{
			switch (playerType)
			{
				case SoundMon11:
				{
					if (bpRepCount != 0)
					{
						bpRepCount--;
						if (bpRepCount != 0)
						{
							newPos  = optionalData;
							posFlag = true;

							// Set the "end" mark, if we have to
							// repeat the block more than 14 times
							if (bpRepCount >= 15)
								endReached = true;
						}
					}
					break;
				}

				case SoundMon22:
				{
					if (optionalData < bpStep)
						endReached = true;		// Tell APlayer the module ends

					newPos  = optionalData;
					posFlag = true;
					break;
				}
			}
			break;
		}

		// Set autoslide
		case BP_OPT_SETAUTOSLIDE:
		{
			cur->autoSlide = optionalData;
			break;
		}

		// Set continous arpeggio
		case BP_OPT_SETARPEGGIO:
		{
			cur->autoArp = optionalData;

			if (playerType == SoundMon22)
			{
				cur->adsrPtr = 0;
				if (cur->adsrControl == 0)
					cur->adsrControl = 1;
			}
			break;
		}

		// Change fx type
		case BP_OPT_CHANGEFX:
		{
			cur->fxControl = optionalData;
			break;
		}

		// Changes from inversion to backward inversion (or vice versa) or
		// from transform to backward transform
		case BP_OPT_CHANGEINVERSION:
		{
			cur->autoArp   = optionalData;
			cur->fxControl = cur->fxControl ^ 1;
			cur->adsrPtr   = 0;
			if (cur->adsrControl == 0)
				cur->adsrControl = 1;

			break;
		}

		// Reset ADSR on synth sound, but not EG, averaging, transform etc.
		case BP_OPT_RESETADSR:
		{
			cur->autoArp = optionalData;
			cur->adsrPtr = 0;
			if (cur->adsrControl == 0)
				cur->adsrControl = 1;

			break;
		}

		// Same as above, but does not reset ADSR either (just changes note)
		case BP_OPT_CHANGENOTE:
		{
			cur->autoArp = optionalData;
			break;
		}
	}
}



/******************************************************************************/
/* DoEffects() updates the real-time effects.                                 */
/******************************************************************************/
void SoundMonitor::DoEffects(void)
{
	uint32 i;

	// Adjust the arpeggio counter
	arpCount = (arpCount - 1) & 3;

	// Adjust the vibrato table index
	vibIndex = (vibIndex + 1) & 7;

	for (i = 0; i < 4; i++)
	{
		BPCurrent *cur = &bpCurrent[i];

		// Autoslide
		cur->period += cur->autoSlide;

		// Vibrato
		if (cur->vibrato != 0)
			virtChannels[i]->SetAmigaPeriod(cur->period + vibTable[vibIndex] / cur->vibrato);
		else
			virtChannels[i]->SetAmigaPeriod(cur->period);

		// Arpeggio
		if ((cur->arpValue != 0) || (cur->autoArp != 0))
		{
			int16 note = cur->note;

			if (arpCount == 0)
				note += ((cur->arpValue & 0xf0) >> 4) + ((cur->autoArp & 0xf0) >> 4);
			else
			{
				if (arpCount == 1)
					note += (cur->arpValue & 0x0f) + (cur->autoArp & 0x0f);
			}

			// Find the period
			cur->restart = false;
			cur->period  = periods[note + 36 - 1];
			virtChannels[i]->SetAmigaPeriod(cur->period);
		}
	}
}



/******************************************************************************/
/* DoSynths() calculate and run the synth effects.                            */
/******************************************************************************/
void SoundMonitor::DoSynths(void)
{
	int32 i, j;
	int32 tableValue;

	for (i = 0; i < 4; i++)
	{
		BPCurrent *cur = &bpCurrent[i];

		// Do the current voice play a synth sample?
		if (cur->synthMode)
		{
			// Yes, begin to do the synth effects
			//
			// Get the instrument address
			Instrument *inst = &instruments[cur->instrument - 1];

			// ADSR
			if (cur->adsrControl != 0)
			{
				cur->adsrCount--;
				if (cur->adsrCount == 0)
				{
					// Reset counter
					cur->adsrCount = inst->adsrSpeed;

					// Calculate new volume
					tableValue = (((waveTables[inst->adsrTable * 64 + cur->adsrPtr] + 128) / 4) * cur->volume) / 16;
					virtChannels[i]->SetVolume(tableValue > 256 ? 256 : (uint16)tableValue);

					// Update the ADSR pointer
					cur->adsrPtr++;
					if (cur->adsrPtr == inst->adsrLength)
					{
						cur->adsrPtr = 0;

						// Only do the ADSR once?
						if (cur->adsrControl == 1)
							cur->adsrControl = 0;
					}
				}
			}

			// LFO
			if (cur->lfoControl != 0)
			{
				cur->lfoCount--;
				if (cur->lfoCount == 0)
				{
					// Reset counter
					cur->lfoCount = inst->lfoSpeed;

					// Get wave table value
					tableValue = waveTables[inst->lfoTable * 64 + cur->lfoPtr];

					// Adjust the value by the LFO depth
					if (inst->lfoDepth != 0)
						tableValue /= inst->lfoDepth;

					// Calculate and set the new period
					virtChannels[i]->SetAmigaPeriod(cur->period + tableValue);

					// Update the LFO pointer
					cur->lfoPtr++;
					if (cur->lfoPtr == inst->lfoLength)
					{
						cur->lfoPtr = 0;

						// Only do the LFO once?
						if (cur->lfoControl == 1)
							cur->lfoControl = 0;
					}
				}
			}

			// Do we have a pointer to a synth buffer?
			if (cur->synthPtr != NULL)
			{
				// EG
				if (cur->egControl != 0)
				{
					cur->egCount--;
					if (cur->egCount == 0)
					{
						// Reset counter
						cur->egCount = inst->egSpeed;

						// Calculate new EG value
						tableValue      = cur->oldEgValue;
						cur->oldEgValue = (waveTables[inst->egTable * 64 + cur->egPtr] + 128) / 8;

						// Do we need to do the EG thing at all?
						if (cur->oldEgValue != tableValue)
						{
							int8 *source, *dest;

							// Find the source and destination addresses
							source = &synthBuffer[i][tableValue];
							dest   = cur->synthPtr + tableValue;

							if (cur->oldEgValue < tableValue)
							{
								tableValue = tableValue - cur->oldEgValue;
								for (j = 0; j < tableValue; j++)
									*dest-- = *source--;
							}
							else
							{
								tableValue = cur->oldEgValue - tableValue;
								for (j = 0; j < tableValue; j++)
									*dest++ = -(*source++);
							}
						}

						// Update the EG pointer
						cur->egPtr++;
						if (cur->egPtr == inst->egLength)
						{
							cur->egPtr = 0;

							// Only do the EG once?
							if (cur->egControl == 1)
								cur->egControl = 0;
						}
					}
				}

				// FX
				switch (cur->fxControl)
				{
					case 1:
					{
						cur->fxCount--;
						if (cur->fxCount == 0)
						{
							// Reset counter
							cur->fxCount = inst->fxSpeed;
							Averaging(i);
						}
						break;
					}

					case 2:
					{
						Transform2(i, inst->fxSpeed);
						break;
					}

					case 3:
					case 5:
					{
						Transform3(i, inst->fxSpeed);
						break;
					}

					case 4:
					{
						Transform4(i, inst->fxSpeed);
						break;
					}

					case 6:
					{
						cur->fxCount--;
						if (cur->fxCount == 0)
						{
							cur->fxControl = 0;
							cur->fxCount   = 1;
							memcpy(cur->synthPtr, cur->synthPtr + 64, 32);
						}
						break;
					}
				}

				// MOD
				if (cur->modControl != 0)
				{
					cur->modCount--;
					if (cur->modCount == 0)
					{
						// Reset counter
						cur->modCount = inst->modSpeed;

						// Get table value and store it in the synth sample
						*(cur->synthPtr + 32) = waveTables[inst->modTable * 64 + cur->modPtr];

						// Update the MOD pointer
						cur->modPtr++;
						if (cur->modPtr == inst->modLength)
						{
							cur->modPtr = 0;

							// Only do the MOD once?
							if (cur->modControl == 1)
								cur->modControl = 0;
						}
					}
				}
			}
		}
	}
}



/******************************************************************************/
/* Averaging() averages the synth sample.                                     */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/******************************************************************************/
void SoundMonitor::Averaging(uint32 voice)
{
	uint32 i;
	int8 *buf = bpCurrent[voice].synthPtr;
	int8 lastVal = *buf;

	for (i = 0; i < 32 - 1; i++)
	{
		lastVal = (lastVal + *(buf + 1)) / 2;
		*buf++  = lastVal;
	}
}



/******************************************************************************/
/* Transform2() transform the synth sample using method 2.                    */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/*         "delta" is the delta value to use.                                 */
/******************************************************************************/
void SoundMonitor::Transform2(uint32 voice, int8 delta)
{
	uint32 i;
	int8 *source = &synthBuffer[voice][31];
	int8 *dest   = bpCurrent[voice].synthPtr;

	for (i = 0; i < 32; i++)
	{
		if (*source < *dest)
			*dest -= delta;
		else
		{
			if (*source > *dest)
				*dest += delta;
		}

		source--;
		dest++;
	}
}



/******************************************************************************/
/* Transform3() transform the synth sample using method 3.                    */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/*         "delta" is the delta value to use.                                 */
/******************************************************************************/
void SoundMonitor::Transform3(uint32 voice, int8 delta)
{
	uint32 i;
	int8 *source = &synthBuffer[voice][0];
	int8 *dest   = bpCurrent[voice].synthPtr;

	for (i = 0; i < 32; i++)
	{
		if (*source < *dest)
			*dest -= delta;
		else
		{
			if (*source > *dest)
				*dest += delta;
		}

		source++;
		dest++;
	}
}



/******************************************************************************/
/* Transform4() transform the synth sample using method 4.                    */
/*                                                                            */
/* Input:  "voice" is the current voice to work on.                           */
/*         "delta" is the delta value to use.                                 */
/******************************************************************************/
void SoundMonitor::Transform4(uint32 voice, int8 delta)
{
	uint32 i;
	int8 *source = bpCurrent[voice].synthPtr + 64;
	int8 *dest   = bpCurrent[voice].synthPtr;

	for (i = 0; i < 32; i++)
	{
		if (*source < *dest)
			*dest -= delta;
		else
		{
			if (*source > *dest)
				*dest += delta;
		}

		source++;
		dest++;
	}
}
