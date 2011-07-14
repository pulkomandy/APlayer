/******************************************************************************/
/* Future Composer Player Interface.                                          */
/*                                                                            */
/* Original player by SuperSero.                                              */
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
#include "FutureComposer.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.00f



/******************************************************************************/
/* Silent table                                                               */
/******************************************************************************/
static const uint8 silent[] =
{
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1
};



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static const uint16 periods[] =
{
	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,
	 856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,
	 428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113,

	 113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,  113,
	3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1812,
	6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3624,

	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,  960,  906,
	 856,  808,  762,  720,  678,  640,  604,  570,  538,  508,  480,  453,
	 428,  404,  381,  360,  339,  320,  302,  285,  269,  254,  240,  226,
	 214,  202,  190,  180,  170,  160,  151,  143,  135,  127,  120,  113
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
FutureComposer::FutureComposer(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	int16 i;

	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize the sample info structure
	for (i = 0; i < (10 + 80); i++)
		sampInfo[i].sample = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
FutureComposer::~FutureComposer(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float FutureComposer::GetVersion(void)
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
uint32 FutureComposer::GetSupportFlags(int32 index)
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
PString FutureComposer::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_FC_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString FutureComposer::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_FC_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString FutureComposer::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_FC_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a Future Composer 1.4        */
/*      module.                                                               */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result FutureComposer::ModuleCheck(int32 index, PFile *file)
{
	uint32 fileSize;

	// Check the module size
	fileSize = file->GetLength();
	if (fileSize < 180)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();

	if (file->Read_B_UINT32() != 'FC14')
		return (AP_UNKNOWN);

	// Skip the song length
	file->Seek(4, PFile::pSeekCurrent);

	// Check the offset pointers
	for (int8 i = 0; i < 8; i++)
	{
		if (file->Read_B_UINT32() > fileSize)
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
ap_result FutureComposer::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	int16 i, j;
	int32 patOffset, frqOffset, volOffset, smpOffset, wavOffset;
	int32 seqLength, patLength, frqLength, volLength;
	ap_result retVal = AP_ERROR;

	try
	{
		// Skip the module mark
		file->Seek(4, PFile::pSeekBegin);

		// Get the length of the sequences
		seqLength = file->Read_B_UINT32();

		// Get the offsets into the file
		patOffset = file->Read_B_UINT32();
		patLength = file->Read_B_UINT32();

		frqOffset = file->Read_B_UINT32();
		frqLength = file->Read_B_UINT32();

		volOffset = file->Read_B_UINT32();
		volLength = file->Read_B_UINT32();

		smpOffset = file->Read_B_UINT32();
		wavOffset = file->Read_B_UINT32();

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Read the sample information
		for (i = 0; i < 10; i++)
		{
			sampInfo[i].sample     = NULL;
			sampInfo[i].length     = file->Read_B_UINT16() * 2;
			sampInfo[i].loopStart  = file->Read_B_UINT16();
			sampInfo[i].loopLength = file->Read_B_UINT16() * 2;
			sampInfo[i].multi      = false;
		}

		// Read the wavetable lengths
		for (i = 10; i < (10 + 80); i++)
		{
			sampInfo[i].sample     = NULL;
			sampInfo[i].length     = file->Read_UINT8() * 2;
			sampInfo[i].loopStart  = 0;
			sampInfo[i].loopLength = sampInfo[i].length;
			sampInfo[i].multi      = false;
		}

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FC_ERR_LOADING_SAMPLEINFO);
			throw PUserException();
		}

		// Find out how many wavetables that are used
		for (i = 89; i >= 10; i--)
		{
			if (sampInfo[i].length != 0)
				break;
		}

		wavNum = i - 9;

		// Allocate memory to hold the sequences
		seqNum    = seqLength / 13;
		sequences = new Sequence[seqNum];
		if (sequences == NULL)
		{
			errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the sequences
		for (i = 0; i < seqNum; i++)
		{
			sequences[i].voiceSeq[0].pattern        = file->Read_UINT8();
			sequences[i].voiceSeq[0].transpose      = file->Read_UINT8();
			sequences[i].voiceSeq[0].soundTranspose = file->Read_UINT8();
			sequences[i].voiceSeq[1].pattern        = file->Read_UINT8();
			sequences[i].voiceSeq[1].transpose      = file->Read_UINT8();
			sequences[i].voiceSeq[1].soundTranspose = file->Read_UINT8();
			sequences[i].voiceSeq[2].pattern        = file->Read_UINT8();
			sequences[i].voiceSeq[2].transpose      = file->Read_UINT8();
			sequences[i].voiceSeq[2].soundTranspose = file->Read_UINT8();
			sequences[i].voiceSeq[3].pattern        = file->Read_UINT8();
			sequences[i].voiceSeq[3].transpose      = file->Read_UINT8();
			sequences[i].voiceSeq[3].soundTranspose = file->Read_UINT8();
			sequences[i].speed                      = file->Read_UINT8();
		}

		// Allocate memory to hold the patterns
		patNum   = patLength / 64;
		patterns = new Pattern[patNum];
		if (patterns == NULL)
		{
			errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the patterns
		file->Seek(patOffset, PFile::pSeekBegin);
		for (i = 0; i < patNum; i++)
		{
			for (j = 0; j < 32; j++)	// Number of rows
			{
				patterns[i].pattern[j].note = file->Read_UINT8();
				patterns[i].pattern[j].info = file->Read_UINT8();
			}
		}

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FC_ERR_LOADING_PATTERNS);
			throw PUserException();
		}

		// Allocate memory to hold the frequency sequences
		frqNum = frqLength / 64;
		frqSequences = new uint8[frqLength];
		if (frqSequences == NULL)
		{
			errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the frequency sequences
		file->Seek(frqOffset, PFile::pSeekBegin);
		file->Read(frqSequences, frqLength);

		// Allocate memory to hold the volume sequences
		volNum = volLength / 64;
		volSequences = new VolSequence[volNum];
		if (volSequences == NULL)
		{
			errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the volume sequences
		file->Seek(volOffset, PFile::pSeekBegin);
		for (i = 0; i < volNum; i++)
		{
			volSequences[i].speed     = file->Read_UINT8();
			volSequences[i].frqNumber = file->Read_UINT8();
			volSequences[i].vibSpeed  = file->Read_UINT8();
			volSequences[i].vibDepth  = file->Read_UINT8();
			volSequences[i].vibDelay  = file->Read_UINT8();
			file->Read(volSequences[i].values, 59);
		}

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_FC_ERR_LOADING_PATTERNS);
			throw PUserException();
		}

		// Load the samples
		file->Seek(smpOffset, PFile::pSeekBegin);
		for (i = 0; i < 10; i++)
		{
			if (sampInfo[i].length != 0)
			{
				// Read the first 4 bytes to see if it's a multi sample
				if (file->Read_B_UINT32() == 'SSMP')
				{
					// It is, so allocate the multi sample structure
					// and fill in the information
					MultiSampleInfo *multiSample;
					uint32 sampStartOffset;
					uint32 multiOffsets[20];

					sampInfo[i].multi = true;

					multiSample = new MultiSampleInfo[1];
					if (multiSample == NULL)
					{
						errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
						throw PUserException();
					}

					for (j = 0; j < 20; j++)
						multiSample->sample[j].sample = NULL;

					// Read the sample informations
					for (j = 0; j < 20; j++)
					{
						multiOffsets[j]                   = file->Read_B_UINT32();
						multiSample->sample[j].length     = file->Read_B_UINT16() * 2;
						multiSample->sample[j].loopStart  = file->Read_B_UINT16();
						multiSample->sample[j].loopLength = file->Read_B_UINT16() * 2;

						// Skip pad bytes
						file->Seek(6, PFile::pSeekCurrent);
					}

					// Read the sample data
					sampStartOffset = file->GetPosition();

					for (j = 0; j < 20; j++)
					{
						if (multiSample->sample[j].length != 0)
						{
							// Allocate sample
							multiSample->sample[j].sample = new int8[multiSample->sample[j].length];
							if (multiSample->sample[j].sample == NULL)
							{
								errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
								throw PUserException();
							}

							// Read the sample data
							file->Seek(sampStartOffset + multiOffsets[j], PFile::pSeekBegin);
							file->Read(multiSample->sample[j].sample, multiSample->sample[j].length);

							// Skip pad bytes
							file->Read_B_UINT16();

							if (file->IsEOF())
							{
								errorStr.LoadString(res, IDS_FC_ERR_LOADING_SAMPLES);
								throw PUserException();
							}
						}
					}

					// Done, remember the pointer
					sampInfo[i].sample = (int8 *)multiSample;
				}
				else
				{
					// It's just a normal sample, so seek back to the
					// start of the sample
					file->Seek(-4, PFile::pSeekCurrent);

					// Allocate memory to the sample
					sampInfo[i].sample = new int8[sampInfo[i].length];
					if (sampInfo[i].sample == NULL)
					{
						errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
						throw PUserException();
					}

					// Read the sample data
					file->Read(sampInfo[i].sample, sampInfo[i].length);
				}
			}

			// Skip pad bytes
			file->Read_B_UINT16();

			if (file->IsEOF())
			{
				errorStr.LoadString(res, IDS_FC_ERR_LOADING_SAMPLES);
				throw PUserException();
			}
		}

		// Load the wavetables
		file->Seek(wavOffset, PFile::pSeekBegin);
		for (i = 10; i < (10 + 80); i++)
		{
			if (sampInfo[i].length != 0)
			{
				// Allocate memory to hold the wavetable data
				sampInfo[i].sample = new int8[sampInfo[i].length];
				if (sampInfo[i].sample == NULL)
				{
					errorStr.LoadString(res, IDS_FC_ERR_MEMORY);
					throw PUserException();
				}

				// Read the wavetable
				file->Read(sampInfo[i].sample, sampInfo[i].length);

				if (file->IsEOF())
				{
					errorStr.LoadString(res, IDS_FC_ERR_LOADING_SAMPLES);
					throw PUserException();
				}
			}
		}

		// Ok, we're done
		retVal = AP_OK;
	}
	catch(PUserException e)
	{
		// Juse delete the exception and clean up
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
bool FutureComposer::InitPlayer(int32 index)
{
	Sequence *seq;
	int16 i, j, k;
	uint8 pattNum;
	PosInfo posInfo;
	uint8 curSpeed = 3;
	float total = 0.0f;
	bool pattBreak = false;

	// Calculate the position times
	for (i = 0; i < seqNum; i++)
	{
		// Add the position information to the list
		posInfo.speed = curSpeed;
		posInfo.time.SetTimeSpan(total);
		posInfoList.AddTail(posInfo);

		// Get pointer to next sequence
		seq = &sequences[i];

		// Change the speed?
		if (seq->speed != 0)
			curSpeed = seq->speed;

		for (j = 0; j < 32; j++)
		{
			for (k = 0; k < 4; k++)
			{
				pattNum = seq->voiceSeq[k].pattern;

				// Do we have a pattern break
				if (patterns[pattNum].pattern[j].note == 0x49)
					pattBreak = true;
			}

			// Add the row time
			total += (1000.0f * curSpeed / 50.0f);

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
void FutureComposer::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void FutureComposer::InitSound(int32 index, uint16 songNum)
{
	uint16 spd;

	// Initialize speed
	spd = sequences->speed;
	if (spd == 0)
		spd = 3;

	reSpCnt = spd;
	repSpd  = spd;
	spdTemp = 1;

	// Initialize other variables
	audTemp[0] = false;
	audTemp[1] = false;
	audTemp[2] = false;
	audTemp[3] = false;

	// Initialize channel variables
	for (uint8 i = 0; i < 4; i++)
	{
		voiceData[i].volumeSeq       = silent;
		voiceData[i].frequencySeq    = silent;
		voiceData[i].pitchBendSpeed  = 0;
		voiceData[i].pitchBendTime   = 0;
		voiceData[i].songPos         = 0;
		voiceData[i].curNote         = 0;
		voiceData[i].curInfo         = 0;
		voiceData[i].volumeBendSpeed = 0;
		voiceData[i].volumeBendTime  = 0;
		voiceData[i].volumeSeqPos    = 0;
		voiceData[i].volumeCounter   = 1;
		voiceData[i].volumeSpeed     = 1;
		voiceData[i].volSusCounter   = 0;
		voiceData[i].susCounter      = 0;
		voiceData[i].vibSpeed        = 0;
		voiceData[i].vibDepth        = 0;
		voiceData[i].vibValue        = 0;
		voiceData[i].vibDelay        = 0;
		voiceData[i].pad31           = 0;
		voiceData[i].volBendFlag     = false;
		voiceData[i].portFlag        = false;
		voiceData[i].patternPos      = 0;
		voiceData[i].pitchBendFlag   = false;
		voiceData[i].pattTranspose   = 0;
		voiceData[i].volume          = 0;
		voiceData[i].vibFlag         = 0;
		voiceData[i].portamento      = 0;
		voiceData[i].pad48           = 0;
		voiceData[i].frequencySeqPos = 0;
		voiceData[i].pitch           = 0;
		voiceData[i].channel         = virtChannels[i];
		voiceData[i].curPattern      = &patterns[sequences[0].voiceSeq[i].pattern];
		voiceData[i].transpose       = sequences[0].voiceSeq[i].transpose;
		voiceData[i].soundTranspose  = sequences[0].voiceSeq[i].soundTranspose;
	}
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void FutureComposer::Play(void)
{
	// Decrease replay speed counter
	reSpCnt--;
	if (reSpCnt == 0)
	{
		// Restore replay speed counter
		reSpCnt = repSpd;

		// Get new note for each channel
		NewNote(0);
		NewNote(1);
		NewNote(2);
		NewNote(3);
	}

	// Calculate effects for each channel
	Effect(0);
	Effect(1);
	Effect(2);
	Effect(3);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 FutureComposer::GetSongLength(void)
{
	return (seqNum);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 FutureComposer::GetSongPosition(void)
{
	return (voiceData[0].songPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void FutureComposer::SetSongPosition(int16 pos)
{
	// Change the position
	voiceData[0].songPos        = pos;
	voiceData[0].patternPos     = 0;
	voiceData[0].transpose      = sequences[pos].voiceSeq[0].transpose;
	voiceData[0].soundTranspose = sequences[pos].voiceSeq[0].soundTranspose;
	voiceData[0].curPattern     = &patterns[sequences[pos].voiceSeq[0].pattern];

	voiceData[1].songPos        = pos;
	voiceData[1].patternPos     = 0;
	voiceData[1].transpose      = sequences[pos].voiceSeq[1].transpose;
	voiceData[1].soundTranspose = sequences[pos].voiceSeq[1].soundTranspose;
	voiceData[1].curPattern     = &patterns[sequences[pos].voiceSeq[1].pattern];

	voiceData[2].songPos        = pos;
	voiceData[2].patternPos     = 0;
	voiceData[2].transpose      = sequences[pos].voiceSeq[2].transpose;
	voiceData[2].soundTranspose = sequences[pos].voiceSeq[2].soundTranspose;
	voiceData[2].curPattern     = &patterns[sequences[pos].voiceSeq[2].pattern];

	voiceData[3].songPos        = pos;
	voiceData[3].patternPos     = 0;
	voiceData[3].transpose      = sequences[pos].voiceSeq[3].transpose;
	voiceData[3].soundTranspose = sequences[pos].voiceSeq[3].soundTranspose;
	voiceData[3].curPattern     = &patterns[sequences[pos].voiceSeq[3].pattern];

	// Set the speed
	reSpCnt = posInfoList.GetItem(pos).speed;
	repSpd  = reSpCnt;
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
PTimeSpan FutureComposer::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
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
bool FutureComposer::GetInfoString(uint32 line, PString &description, PString &value)
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
			description.LoadString(res, IDS_FC_INFODESCLINE0);
			value.SetUNumber(seqNum);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_FC_INFODESCLINE1);
			value.SetUNumber(patNum);
			break;
		}

		// Used Instruments
		case 2:
		{
			description.LoadString(res, IDS_FC_INFODESCLINE2);
			value = "10";
			break;
		}

		// Used Wavetables
		case 3:
		{
			description.LoadString(res, IDS_FC_INFODESCLINE3);
			value.SetUNumber(wavNum);
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
bool FutureComposer::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	SampleInfo *sample;

	// First check the sample number for "out of range"
	if (num >= 10)
		return (false);

	// Get the pointer to the sample data
	sample = &sampInfo[num];

	// Fill out the sample info structure
	info->name.MakeEmpty();
	info->type    = apSample;
	info->bitSize = 8;
	info->middleC = 8287;
	info->volume  = 256;
	info->panning = -1;

	if (sample->multi)
	{
		// Multi sample, so just take the first one
		MultiSampleInfo *multi = (MultiSampleInfo *)sample->sample;
		sample = &multi->sample[0];
	}

	// Fill out the rest of the information
	info->address    = sample->sample;
	info->length     = sample->length;
	info->loopStart  = sample->loopStart;
	info->loopLength = sample->loopLength;
	info->flags      = (sample->loopLength > 2 ? APSAMP_LOOP : 0);

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void FutureComposer::Cleanup(void)
{
	int16 i, j;
	MultiSampleInfo *multiSamp;

	delete[] volSequences;
	volSequences = NULL;

	delete[] frqSequences;
	frqSequences = NULL;

	delete[] patterns;
	patterns = NULL;

	delete[] sequences;
	sequences = NULL;

	for (i = 0; i < (10 + 80); i++)
	{
		if (sampInfo[i].sample != NULL)
		{
			if (sampInfo[i].multi)
			{
				multiSamp = (MultiSampleInfo *)sampInfo[i].sample;

				for (j = 0; j < 20; j++)
					delete[] multiSamp->sample[j].sample;
			}

			delete[] sampInfo[i].sample;
			sampInfo[i].sample = NULL;
		}
	}
}



/******************************************************************************/
/* NewNote() will play the next row.                                          */
/*                                                                            */
/* Input:  "chan" is the channel number.                                      */
/******************************************************************************/
void FutureComposer::NewNote(uint16 chan)
{
	VoiceInfo *voiData;
	uint8 note, info, inst;

	// Get the pointer to the voice data
	voiData = &voiceData[chan];

	// Check for end of pattern or "END" mark in pattern
	if ((voiData->patternPos == 32) || (voiData->curPattern->pattern[voiData->patternPos].note == 0x49))
	{
		// New position
		voiData->songPos++;
		voiData->patternPos = 0;

		// Tell APlayer we have changed position
		ChangePosition();

		// Have we reached the end of the module
		if (voiData->songPos == seqNum)
		{
			// We have, wrap around the module
			voiData->songPos = 0;

			// Tell APlayer the module have ended
			endReached = true;
		}

		// Count the speed counter
		spdTemp++;
		if (spdTemp == 5)
		{
			spdTemp = 1;

			// Get new replay speed
			if (sequences[voiData->songPos].speed != 0)
			{
				reSpCnt = sequences[voiData->songPos].speed;
				repSpd  = reSpCnt;
			}
		}

		// Get pattern information
		voiData->transpose      = sequences[voiData->songPos].voiceSeq[chan].transpose;
		voiData->soundTranspose = sequences[voiData->songPos].voiceSeq[chan].soundTranspose;
		voiData->curPattern     = &patterns[sequences[voiData->songPos].voiceSeq[chan].pattern];
	}

	// Get the pattern row
	note = voiData->curPattern->pattern[voiData->patternPos].note;
	info = voiData->curPattern->pattern[voiData->patternPos].info;

	// Check to see if we need to make portamento
	//
	// Info = Portamento/Instrument info
	//        Bit 7   = Portamento on
	//        Bit 6   = Portamento off
	//        Bit 5-0 = Instrument number
	//
	// Info in the next row = Portamento value
	//        Bit 7-5 = Always zero
	//        Bit 4   = Up/Down
	//        Bit 3-0 = Value
	if ((note != 0) || (info & 0xc0))
	{
		if (note != 0)
			voiData->pitch = 0;

		if (info & 0x80)
			voiData->portamento = voiData->curPattern->pattern[voiData->patternPos + 1].info;
		else
			voiData->portamento = 0;
	}

	// Got any note
	note &= 0x7f;
	if (note != 0)
	{
		voiData->curNote = note;
		voiData->curInfo = info;

		// Mute the channel
		audTemp[chan] = false;
		voiData->channel->Mute();

		// Find the volume sequence
		inst = ((info & 0x3f) + voiData->soundTranspose) & 0x3f;
		if (inst < volNum)
		{
			voiData->volumeSeqPos  = 0;
			voiData->volumeCounter = volSequences[inst].speed;
			voiData->volumeSpeed   = volSequences[inst].speed;
			voiData->vibSpeed      = volSequences[inst].vibSpeed;
			voiData->vibFlag       = 0x40;
			voiData->vibDepth      = volSequences[inst].vibDepth;
			voiData->vibValue      = volSequences[inst].vibDepth;
			voiData->vibDelay      = volSequences[inst].vibDelay;
			voiData->volumeSeq     = volSequences[inst].values;

			// Find the frequency sequence
			voiData->frequencySeq    = &frqSequences[volSequences[inst].frqNumber * 64];
			voiData->frequencySeqPos = 0;
			voiData->volSusCounter   = 0;
			voiData->susCounter      = 0;
		}
		else
		{
			voiData->volumeSeqPos  = 0;
			voiData->volumeCounter = 1;
			voiData->volumeSpeed   = 1;
			voiData->vibSpeed      = 0;
			voiData->vibFlag       = 0;
			voiData->vibDepth      = 0;
			voiData->vibValue      = 0;
			voiData->vibDelay      = 0;
			voiData->volumeSeq     = silent;

			// Find the frequency sequence
			voiData->frequencySeq    = silent;
			voiData->frequencySeqPos = 0;
			voiData->volSusCounter   = 0;
			voiData->susCounter      = 0;
		}
	}

	// Go the the next pattern row
	voiData->patternPos++;
}



/******************************************************************************/
/* Effect() will calculate the effects for one channel and play them.         */
/*                                                                            */
/* Input:  "chan" is the channel number.                                      */
/******************************************************************************/
void FutureComposer::Effect(uint16 chan)
{
	VoiceInfo *voiData;
	const uint8 *seqPoi;
	uint8 dat;
	bool oneMore, parseEffect;

	// Get the pointer to the voice data
	voiData = &voiceData[chan];

	// Parse the frequency sequence commands
	do
	{
		// Only loop one time, except if this flag is set later on
		oneMore = false;

		if (voiData->susCounter != 0)
		{
			voiData->susCounter--;
			break;
		}

		// Sustain counter is zero, run the next part of the sequence
		seqPoi = &voiData->frequencySeq[voiData->frequencySeqPos];

		do
		{
			// Only loop one time, except if this flag is set later on
			parseEffect = false;

			// Check for end of sequence
			if (*seqPoi == 0xe1)
				break;
			else
			{
				// Check for "loop to other part of sequence" command
				if (*seqPoi == 0xe0)
				{
					dat = *(seqPoi + 1) & 0x3f;

					voiData->frequencySeqPos = dat;
					seqPoi = &voiData->frequencySeq[dat];
				}

				// Check for all the effects
				switch (*seqPoi)
				{
					// Set waveform
					case 0xe2:
					{
						dat = *(seqPoi + 1);	// Get instrument number

						if (dat < 90)
						{
							if (sampInfo[dat].sample != NULL)
							{
								voiData->channel->PlaySample(sampInfo[dat].sample, 0, sampInfo[dat].length);
								if (sampInfo[dat].loopLength > 2)
								{
									if ((sampInfo[dat].loopStart + sampInfo[dat].loopLength) > sampInfo[dat].length)
										voiData->channel->SetLoop(sampInfo[dat].loopStart, sampInfo[dat].length - sampInfo[dat].loopStart);
									else
										voiData->channel->SetLoop(sampInfo[dat].loopStart, sampInfo[dat].loopLength);
								}
							}
						}

						voiData->volumeSeqPos     = 0;
						voiData->volumeCounter    = 1;
						voiData->frequencySeqPos += 2;
						audTemp[chan]             = true;
						break;
					}

					// Set loop
					case 0xe4:
					{
						// Check to see if the channel is active
						if (audTemp[chan] == true)
						{
							dat = *(seqPoi + 1);	// Get instrument number

							if (dat < 90)
								voiData->channel->SetLoop(sampInfo[dat].sample, sampInfo[dat].loopStart, sampInfo[dat].loopLength);

							voiData->frequencySeqPos += 2;
						}
						break;
					}

					// Set sample
					case 0xe9:
					{
						audTemp[chan] = true;
						dat = *(seqPoi + 1);	// Get instrument number

						if ((dat < 90) && (sampInfo[dat].multi == true))
						{
							MultiSampleInfo *mulSamp = (MultiSampleInfo *)sampInfo[dat].sample;
							dat = *(seqPoi + 2);	// Get multi sample number

							if (dat < 20)
							{
								if (mulSamp->sample[dat].sample != NULL)
								{
									voiData->channel->PlaySample(mulSamp->sample[dat].sample, 0, mulSamp->sample[dat].length);
									if (mulSamp->sample[dat].loopLength > 2)
									{
										if ((mulSamp->sample[dat].loopStart + mulSamp->sample[dat].loopLength) > mulSamp->sample[dat].length)
											voiData->channel->SetLoop(mulSamp->sample[dat].loopStart, mulSamp->sample[dat].length - mulSamp->sample[dat].loopStart);
										else
											voiData->channel->SetLoop(mulSamp->sample[dat].loopStart, mulSamp->sample[dat].loopLength);
									}
								}
							}

							voiData->volumeSeqPos  = 0;
							voiData->volumeCounter = 1;
						}

						voiData->frequencySeqPos += 3;
						break;
					}

					// Pattern jump
					case 0xe7:
					{
						parseEffect = true;
						dat = *(seqPoi + 1);	// Get new position

						seqPoi = &frqSequences[dat * 64];
						voiData->frequencySeq    = seqPoi;
						voiData->frequencySeqPos = 0;
						break;
					}

					// Pitchbend
					case 0xea:
					{
						voiData->pitchBendSpeed   = *(seqPoi + 1);
						voiData->pitchBendTime    = *(seqPoi + 2);
						voiData->frequencySeqPos += 3;
						break;
					}

					// New sustain
					case 0xe8:
					{
						voiData->susCounter       = *(seqPoi + 1);
						voiData->frequencySeqPos += 2;
						oneMore = true;
						break;
					}

					// New vibrato
					case 0xe3:
					{
						voiData->vibSpeed         = *(seqPoi + 1);
						voiData->vibDepth         = *(seqPoi + 2);
						voiData->frequencySeqPos += 3;
						break;
					}
				}

				if ((parseEffect == false) && (oneMore == false))
				{
					// Get transpose value
					seqPoi = &voiData->frequencySeq[voiData->frequencySeqPos];
					voiData->pattTranspose = *seqPoi;
					voiData->frequencySeqPos++;
				}
			}
		}
		while (parseEffect);
	}
	while (oneMore);

	// Parse the volume sequence commands
	if (voiData->volSusCounter != 0)
		voiData->volSusCounter--;
	else
	{
		if (voiData->volumeBendTime != 0)
			DoVolBend(voiData);
		else
		{
			voiData->volumeCounter--;
			if (voiData->volumeCounter == 0)
			{
				voiData->volumeCounter = voiData->volumeSpeed;

				do
				{
					// Only loop one time, except if this flag is set later on
					parseEffect = false;

					seqPoi = &voiData->volumeSeq[voiData->volumeSeqPos];

					// Check for end of sequence
					if (*seqPoi == 0xe1)
						break;
					else
					{
						// Check for all the effects
						switch (*seqPoi)
						{
							// Volume bend
							case 0xea:
							{
								voiData->volumeBendSpeed = *(seqPoi + 1);
								voiData->volumeBendTime  = *(seqPoi + 2);
								voiData->volumeSeqPos  += 3;
								DoVolBend(voiData);
								break;
							}

							// New volume sustain
							case 0xe8:
							{
								voiData->volSusCounter = *(seqPoi + 1);
								voiData->volumeSeqPos  += 2;
								break;
							}

							// Set volume loop
							case 0xe0:
							{
								voiData->volumeSeqPos = ((*(seqPoi + 1)) & 0x3f) - 5;
								parseEffect = true;
								break;
							}

							// Set volume
							default:
							{
								voiData->volume = *seqPoi;
								voiData->volumeSeqPos++;
								break;
							}
						}
					}
				}
				while (parseEffect);
			}
		}
	}

	// Calculate the period
	int8 note;
	uint16 period;
	uint8 vibFlag;
	int8 vibBase, vibDep, vibVal;

	note = voiData->pattTranspose;
	if (note >= 0)
		note += (voiData->curNote + voiData->transpose);

	note &= 0x7f;

	// Get the period
	period  = periods[note];
	vibFlag = voiData->vibFlag;

	// Shall we vibrate?
	if (voiData->vibDelay != 0)
		voiData->vibDelay--;
	else
	{
		vibBase = note * 2;
		vibDep  = voiData->vibDepth * 2;
		vibVal  = voiData->vibValue;

		if ((!(vibFlag & 0x80)) || (!(vibFlag & 0x01)))
		{
			if (vibFlag & 0x20)
			{
				vibVal += voiData->vibSpeed;
				if (vibVal >= vibDep)
				{
					vibFlag &= ~0x20;
					vibVal   = vibDep;
				}
			}
			else
			{
				vibVal -= voiData->vibSpeed;
				if (vibVal < 0)
				{
					vibFlag |= 0x20;
					vibVal   = 0;
				}
			}

			voiData->vibValue = vibVal;
		}

		vibDep  /= 2;
		vibVal  -= vibDep;
		vibBase += (int8)160;

		while (vibBase >= 0)
		{
			vibVal  *= 2;
			vibBase += 24;
		}

		period += vibVal;
	}

	voiData->vibFlag = vibFlag ^ 0x01;

	// Do the portamento thing
	voiData->portFlag = (voiData->portFlag == true ? false : true);
	if ((voiData->portFlag == true) && (voiData->portamento != 0))
	{
		if (voiData->portamento <= 31)
			voiData->pitch -= voiData->portamento;
		else
			voiData->pitch += (voiData->portamento & 0x1f);
	}

	// Pitchbend
	voiData->pitchBendFlag = (voiData->pitchBendFlag == true ? false : true);
	if ((voiData->pitchBendFlag == true) && (voiData->pitchBendTime != 0))
	{
		voiData->pitchBendTime--;
		voiData->pitch -= voiData->pitchBendSpeed;
	}

	period += voiData->pitch;

	// Check for bounds
	if (period < 113)
		period = 113;
	else
	{
		if (period > 3424)
			period = 3424;
	}

	if (voiData->volume < 0)
		voiData->volume = 0;
	else
	{
		if (voiData->volume > 64)
			voiData->volume = 64;
	}

	// Play the period
	voiData->channel->SetAmigaPeriod(period);
	voiData->channel->SetVolume(voiData->volume * 4);
}



/******************************************************************************/
/* DoVolBend() will make a volume bend.                                       */
/*                                                                            */
/* Input:  "voiData" is a pointer to the channel structure.                   */
/******************************************************************************/
void FutureComposer::DoVolBend(VoiceInfo *voiData)
{
	voiData->volBendFlag = (voiData->volBendFlag == true ? false : true);
	if (voiData->volBendFlag == true)
	{
		voiData->volumeBendTime--;
		voiData->volume += voiData->volumeBendSpeed;

		if (voiData->volume > 64)
		{
			voiData->volume         = 64;
			voiData->volumeBendTime = 0;
		}
		else
		{
			if (voiData->volume < 0)
			{
				voiData->volume         = 0;
				voiData->volumeBendTime = 0;
			}
		}
	}
}
