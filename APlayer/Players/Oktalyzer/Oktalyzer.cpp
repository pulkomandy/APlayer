/******************************************************************************/
/* Oktalyzer Player Interface.                                                */
/*                                                                            */
/* Original player by Armin Sander.                                           */
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
#include "Oktalyzer.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.01f



/******************************************************************************/
/* Period table                                                               */
/******************************************************************************/
static const int16 periods[] =
{
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Oktalyzer::Oktalyzer(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	sampleInfo  = NULL;
	patterns    = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Oktalyzer::~Oktalyzer(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float Oktalyzer::GetVersion(void)
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
uint32 Oktalyzer::GetSupportFlags(int32 index)
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
PString Oktalyzer::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_OKT_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString Oktalyzer::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_OKT_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString Oktalyzer::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_OKT_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see which type of module it is.          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Oktalyzer::ModuleCheck(int32 index, PFile *file)
{
	// Check the module size
	if (file->GetLength() < 1368)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();
	if ((file->Read_B_UINT32() != 'OKTA') || (file->Read_B_UINT32() != 'SONG'))
		return (AP_UNKNOWN);

	return (AP_OK);
}



/******************************************************************************/
/* LoadModule() will load the module into the memory.                         */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a file object with the file to check.       */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Oktalyzer::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	int32 fileSize;
	uint32 chunkName, chunkSize;
	ap_result retVal = AP_ERROR;

	try
	{
		// Skip the mark
		file->Seek(8, PFile::pSeekBegin);

		// Get the file size
		fileSize = file->GetLength();

		// Initialize variables
		sampNum         = 0;
		pattNum         = 0;
		songLength      = 0;
		startSpeed      = 6;

		readPatt        = 0;
		readSamp        = 0;
		realUsedSampNum = 0;

		// Okay, now read each chunk and parse them
		for (;;)
		{
			// Read the chunk name and length
			chunkName = file->Read_B_UINT32();
			chunkSize = file->Read_B_UINT32();

			// Do we have any chunks left?
			if (file->IsEOF())
				break;	// Nop, stop the loading

			// Find out what the chunk is and begin to parse it
			switch (chunkName)
			{
				//
				// Channel modes
				//
				case 'CMOD':
				{
					ParseCMOD(chunkSize, file, errorStr);
					break;
				}

				//
				// Sample information
				//
				case 'SAMP':
				{
					ParseSAMP(chunkSize, file, errorStr);
					break;
				}

				//
				// Start speed
				//
				case 'SPEE':
				{
					ParseSPEE(chunkSize, file, errorStr);
					break;
				}

				//
				// Number of patterns
				//
				case 'SLEN':
				{
					ParseSLEN(chunkSize, file, errorStr);
					break;
				}

				//
				// Song length
				//
				case 'PLEN':
				{
					ParsePLEN(chunkSize, file, errorStr);
					break;
				}

				//
				// Pattern table
				//
				case 'PATT':
				{
					ParsePATT(chunkSize, file, errorStr);
					break;
				}

				//
				// Pattern body
				//
				case 'PBOD':
				{
					if ((readPatt < pattNum) && (patterns != NULL))
					{
						ParsePBOD(chunkSize, file, errorStr);
						readPatt++;
					}
					else
					{
						// Ignore the chunk
						ASSERT(false);
						file->Seek(chunkSize, PFile::pSeekCurrent);
					}
					break;
				}

				//
				// Sample data
				//
				case 'SBOD':
				{
					if ((readSamp < sampNum) && (sampleInfo != NULL))
					{
						ParseSBOD(chunkSize, file, errorStr);
						readSamp++;
					}
					else
					{
						// Ignore the chunk
						ASSERT(false);
						file->Seek(chunkSize, PFile::pSeekCurrent);
					}
					break;
				}

				//
				// Unknown chunks
				//
				default:
				{
					// Check to see if we had read all the samples
					if (readSamp < realUsedSampNum)
					{
						errorStr.Format(res, IDS_OKT_ERR_UNKNOWN_CHUNK, (chunkName >> 24) & 0xff, (chunkName >> 16) & 0xff, (chunkName >> 8) & 0xff, chunkName & 0xff);
						throw PUserException();
					}
					else
					{
						// Well, we had read all the samples, so if the file
						// has some extra data appended, we ignore it
						file->SeekToEnd();
						break;
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
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool Oktalyzer::InitPlayer(int32 index)
{
	Pattern *patt;
	PatternLine *pattLine;
	uint16 i, k;
	int16 j;
	uint8 eff, effArg;
	int16 newPos = -1;
	bool done = false;
	PosInfo posInfo;
	uint16 curSpeed = startSpeed;
	float total = 0.0f;

	// Calculate the position times
	for (i = 0; i < songLength; i++)
	{
		// Add the position information to the list
		posInfo.speed = curSpeed;
		posInfo.time.SetTimeSpan(total);
		posInfoList.AddTail(posInfo);

		// Get pointer to next pattern
		patt     = patterns[patternTable[i]];
		pattLine = patt->lines;

		for (j = 0; j < patt->lineNum; j++)
		{
			for (k = 0; k < chanNum; k++)
			{
				eff    = pattLine[j * chanNum + k].effect;
				effArg = pattLine[j * chanNum + k].effectArg;

				// Set speed effect?
				if ((eff == 28) && ((effArg & 0xf) != 0))
					curSpeed = effArg & 0xf;

				// Position jump?
				if (eff == 25)
				{
					newPos = ((effArg & 0xf0) >> 4) * 10 + (effArg & 0x0f);
					if (newPos >= songLength)
						newPos = -1;
				}
			}

			// Add the row time
			total += (1000.0f * curSpeed / 50.0f);

			// Should we jump
			if (newPos != -1)
			{
				if (newPos < i)
					done = true;
				else
				{
					i      = newPos - 1;
					newPos = -1;
				}
				break;
			}
		}

		if (done)
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
void Oktalyzer::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void Oktalyzer::InitSound(int32 index, uint16 songNum)
{
	static const uint16 panPos[8] =
	{
		APPAN_LEFT, APPAN_LEFT, APPAN_RIGHT, APPAN_RIGHT,
		APPAN_RIGHT, APPAN_RIGHT, APPAN_LEFT, APPAN_LEFT
	};

	uint32 i, panNum;

	// Initialize the variables
	memset(chanInfo, 0, sizeof(chanInfo));
	memset(currLine, 0, sizeof(currLine));

	for (i = 0; i < 8; i++)
		chanVol[i] = 64;

	songPos      = 0;
	newSongPos   = -1;
	pattPos      = -1;
	currentSpeed = startSpeed;
	speedCounter = 0;
	filterStatus = false;

	// Set the channel panning + create the channel index
	for (i = 0, panNum = 0; i < chanNum; i++, panNum++)
	{
		virtChannels[i]->SetPanning(panPos[panNum]);
		chanIndex[i] = panNum / 2;

		if (!channelFlags[panNum / 2])
			panNum++;
	}
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void Oktalyzer::Play(void)
{
	// Wait until we need to play another pattern line
	speedCounter++;
	if (speedCounter >= currentSpeed)
	{
		// Play next pattern line
		speedCounter = 0;

		FindNextPatternLine();
		PlayPatternLine();
	}

	// Do the each frame stuff
	DoEffects();
	SetVolumes();

	// Do the filter stuff
	amigaFilter = filterStatus;
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 Oktalyzer::GetModuleChannels(void)
{
	return (chanNum);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 Oktalyzer::GetSongLength(void)
{
	return (songLength);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 Oktalyzer::GetSongPosition(void)
{
	return (songPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void Oktalyzer::SetSongPosition(int16 pos)
{
	// Change the position
	songPos      = pos;
	pattPos      = 0;
	currentSpeed = posInfoList.GetItem(pos).speed;
	speedCounter = currentSpeed;
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
PTimeSpan Oktalyzer::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
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
bool Oktalyzer::GetInfoString(uint32 line, PString &description, PString &value)
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
			description.LoadString(res, IDS_OKT_INFODESCLINE0);
			value.SetUNumber(songLength);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_OKT_INFODESCLINE1);
			value.SetUNumber(pattNum);
			break;
		}

		// Used Samples
		case 2:
		{
			description.LoadString(res, IDS_OKT_INFODESCLINE2);
			value.SetUNumber(sampNum);
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
bool Oktalyzer::GetSampleInfo(uint32 num, APSampleInfo *info)
{
	Sample *sample;

	// Check to see if we have reached the maximum number of samples
	if (num >= sampNum)
		return (false);

	// Get the pointer to the sample data
	sample = &sampleInfo[num];

	// Fill out the sample info structure
	info->name       = sample->name;
	info->type       = apSample;
	info->bitSize    = 8;
	info->middleC    = 8287;
	info->volume     = sample->volume * 4;
	info->panning    = -1;
	info->address    = sample->sample;
	info->length     = sample->length;

	if (sample->repeatLength == 0)
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
		info->loopStart  = sample->repeatStart;
		info->loopLength = sample->repeatLength;
	}

	return (true);
}



/******************************************************************************/
/* ParseCMOD() parses the CMOD chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParseCMOD(uint32 chunkSize, PFile *file, PString &errorStr)
{
	int8 i;

	// Start to check the chunk size
	if (chunkSize != 8)
	{
		errorStr.Format(res, IDS_OKT_ERR_INVALID_CHUNK_SIZE, "CMOD", chunkSize);
		throw PUserException();
	}

	// Read the channel flags
	chanNum = 4;

	for (i = 0; i < 4; i++)
	{
		if (file->Read_B_UINT16() == 0)
			channelFlags[i] = false;
		else
		{
			channelFlags[i] = true;
			chanNum++;
		}
	}
}



/******************************************************************************/
/* ParseSAMP() parses the SAMP chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParseSAMP(uint32 chunkSize, PFile *file, PString &errorStr)
{
	PCharSet_Amiga charSet;
	uint32 i;
	char buffer[21];

	// Calculate the number of samples
	sampNum = chunkSize / 32;

	// Allocate memory to hold the sample informations
	sampleInfo = new Sample[sampNum];
	if (sampleInfo == NULL)
	{
		errorStr.LoadString(res, IDS_OKT_ERR_MEMORY);
		throw PUserException();
	}

	// Clear pointers
	for (i = 0; i < sampNum; i++)
		sampleInfo[i].sample = NULL;

	// Read the sample informations
	buffer[20] = 0x00;
	for (i = 0; i < sampNum; i++)
	{
		// Sample name
		file->ReadString(buffer, 20);
		sampleInfo[i].name.SetString(buffer, &charSet);

		// Other informations
		sampleInfo[i].length       = file->Read_B_UINT32();
		sampleInfo[i].repeatStart  = file->Read_B_UINT16() * 2;
		sampleInfo[i].repeatLength = file->Read_B_UINT16() * 2;

		file->Seek(1, PFile::pSeekCurrent);
		sampleInfo[i].volume       = file->Read_UINT8();
		sampleInfo[i].mode         = file->Read_B_UINT16();

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_OKT_ERR_LOADING_HEADER);
			throw PUserException();
		}

		if (sampleInfo[i].length != 0)
			realUsedSampNum++;
	}
}



/******************************************************************************/
/* ParseSPEE() parses the SPEE chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParseSPEE(uint32 chunkSize, PFile *file, PString &errorStr)
{
	// Start to check the chunk size
	if (chunkSize != 2)
	{
		errorStr.Format(res, IDS_OKT_ERR_INVALID_CHUNK_SIZE, "SPEE", chunkSize);
		throw PUserException();
	}

	// Read the start speed
	startSpeed = file->Read_B_UINT16();
}



/******************************************************************************/
/* ParseSLEN() parses the SLEN chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParseSLEN(uint32 chunkSize, PFile *file, PString &errorStr)
{
	// Start to check the chunk size
	if (chunkSize != 2)
	{
		errorStr.Format(res, IDS_OKT_ERR_INVALID_CHUNK_SIZE, "SLEN", chunkSize);
		throw PUserException();
	}

	// Read the number of patterns
	pattNum = file->Read_B_UINT16();

	// Allocate memory to hold the patterns
	patterns = new Pattern *[pattNum];
	if (patterns == NULL)
	{
		errorStr.LoadString(res, IDS_OKT_ERR_MEMORY);
		throw PUserException();
	}

	memset(patterns, 0, pattNum * sizeof(Pattern *));
}



/******************************************************************************/
/* ParsePLEN() parses the PLEN chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParsePLEN(uint32 chunkSize, PFile *file, PString &errorStr)
{
	// Start to check the chunk size
	if (chunkSize != 2)
	{
		errorStr.Format(res, IDS_OKT_ERR_INVALID_CHUNK_SIZE, "PLEN", chunkSize);
		throw PUserException();
	}

	// Read the song length
	songLength = file->Read_B_UINT16();
}



/******************************************************************************/
/* ParsePATT() parses the PATT chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParsePATT(uint32 chunkSize, PFile *file, PString &errorStr)
{
	// Start to check the chunk size
	if (chunkSize != 128)
	{
		errorStr.Format(res, IDS_OKT_ERR_INVALID_CHUNK_SIZE, "PATT", chunkSize);
		throw PUserException();
	}

	// Read the pattern table
	file->Read(patternTable, 128);
}



/******************************************************************************/
/* ParsePBOD() parses the PBOD chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParsePBOD(uint32 chunkSize, PFile *file, PString &errorStr)
{
	int32 i, j;

	// Allocate pattern
	patterns[readPatt] = new Pattern;
	if (patterns[readPatt] == NULL)
	{
		errorStr.LoadString(res, IDS_OKT_ERR_MEMORY);
		throw PUserException();
	}

	// Initialize pointers
	patterns[readPatt]->lines = NULL;

	// First read the number of pattern lines
	patterns[readPatt]->lineNum = file->Read_B_UINT16();

	// Allocate lines
	patterns[readPatt]->lines = new PatternLine[patterns[readPatt]->lineNum * chanNum];
	if (patterns[readPatt]->lines == NULL)
	{
		errorStr.LoadString(res, IDS_OKT_ERR_MEMORY);
		throw PUserException();
	}

	// Read the pattern data
	for (i = 0; i < patterns[readPatt]->lineNum; i++)
	{
		for (j = 0; j < chanNum; j++)
		{
			patterns[readPatt]->lines[i * chanNum + j].note      = file->Read_UINT8();
			patterns[readPatt]->lines[i * chanNum + j].sampleNum = file->Read_UINT8();
			patterns[readPatt]->lines[i * chanNum + j].effect    = file->Read_UINT8();
			patterns[readPatt]->lines[i * chanNum + j].effectArg = file->Read_UINT8();
		}
	}

	if (file->IsEOF())
	{
		errorStr.LoadString(res, IDS_OKT_ERR_LOADING_PATTERNS);
		throw PUserException();
	}
}



/******************************************************************************/
/* ParseSBOD() parses the SBOD chunk.                                         */
/*                                                                            */
/* Input:  "chunkSize" is the size of the chunk.                              */
/*         "file" is a pointer to a PFile object with the file to read from.  */
/*         "errorStr" is a reference to a string, where you should store the  */
/*         error if you can't load the module.                                */
/*                                                                            */
/* Except: PUserException, PFileException.                                    */
/******************************************************************************/
void Oktalyzer::ParseSBOD(uint32 chunkSize, PFile *file, PString &errorStr)
{
	uint32 allocLen, readLen;

	// Find the next sample slot
	while (sampleInfo[readSamp].length == 0)
	{
		readSamp++;
		if (readSamp >= sampNum)
		{
			ASSERT(false);
			return;
		}
	}

	// Allocate memory to hold the sample data
	allocLen = max(chunkSize, sampleInfo[readSamp].length);
	sampleInfo[readSamp].sample = new int8[allocLen];
	if (sampleInfo[readSamp].sample == NULL)
	{
		errorStr.LoadString(res, IDS_OKT_ERR_MEMORY);
		throw PUserException();
	}

	// Clear the sample data, just in case the last sample isn't whole
	memset(sampleInfo[readSamp].sample, 0, allocLen);

	// Read the sample data
	readLen = file->Read(sampleInfo[readSamp].sample, chunkSize);

	if (file->IsEOF())
	{
		if (((readSamp + 1) < realUsedSampNum) || ((readLen + 20) < chunkSize))
		{
			errorStr.LoadString(res, IDS_OKT_ERR_LOADING_SAMPLES);
			throw PUserException();
		}
	}
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void Oktalyzer::Cleanup(void)
{
	uint32 i;

	// Delete the patterns
	if (patterns != NULL)
	{
		for (i = 0; i < pattNum; i++)
		{
			delete[] patterns[i]->lines;
			delete patterns[i];
		}

		delete[] patterns;
		patterns = NULL;
	}

	// Delete the sample informations
	if (sampleInfo != NULL)
	{
		for (i = 0; i < sampNum; i++)
			delete[] sampleInfo[i].sample;

		delete[] sampleInfo;
		sampleInfo = NULL;
	}
}



/******************************************************************************/
/* FindNextPatternLine() find the next pattern to use and where in the        */
/*      pattern we need to extract the informations we need.                  */
/******************************************************************************/
void Oktalyzer::FindNextPatternLine(void)
{
	Pattern *patt;
	uint32 i;

	// Find the right pattern
	patt = patterns[patternTable[songPos]];

	// Go to the next pattern line
	pattPos++;

	if ((pattPos >= patt->lineNum) || (newSongPos != -1))
	{
		// Okay, we're done with the current pattern. Find the next one
		pattPos = 0;

		if (newSongPos != -1)
		{
			if (newSongPos < songPos)
				endReached = true;

			songPos    = newSongPos;
			newSongPos = -1;
		}
		else
			songPos++;

		// Tell APlayer we have change the song position
		ChangePosition();

		if (songPos == songLength)
		{
			songPos      = 0;
			currentSpeed = startSpeed;

			// Song end
			endReached = true;
		}

		// Find the right pattern
		patt = patterns[patternTable[songPos]];
	}

	// Copy the current line data
	for (i = 0; i < chanNum; i++)
	{
		currLine[i].note      = patt->lines[pattPos * chanNum + i].note;
		currLine[i].sampleNum = patt->lines[pattPos * chanNum + i].sampleNum;
		currLine[i].effect    = patt->lines[pattPos * chanNum + i].effect;
		currLine[i].effectArg = patt->lines[pattPos * chanNum + i].effectArg;
	}
}



/******************************************************************************/
/* PlayPatternLine() trig new samples in each channel.                        */
/******************************************************************************/
void Oktalyzer::PlayPatternLine(void)
{
	uint32 i, j;

	for (i = 0, j = 0; i < 4; i++, j++)
	{
		PlayChannel(j);

		if (channelFlags[i])
			PlayChannel(++j);
	}
}



/******************************************************************************/
/* PlayChannel() will parse the pattern data for one channel.                 */
/*                                                                            */
/* Input:  "chanNum" is the channel number to use.                            */
/******************************************************************************/
void Oktalyzer::PlayChannel(uint32 chanNum)
{
	PatternLine *pattData;
	ChannelInfo *chanData;
	Sample *samp;
	uint8 note;

	// Get the pattern and channel data
	pattData = &currLine[chanNum];
	chanData = &chanInfo[chanNum];

	// If we shouldn't play any note, well return from the function
	if (pattData->note == 0)
		return;

	// Get the note number
	note = pattData->note - 1;

	// Does the instrument have a sample attached?
	samp = &sampleInfo[pattData->sampleNum];
	if ((samp->sample == NULL) || (samp->length == 0))
		return;

	// Well, find out if we are playing in a mixed or normal channel
	if (channelFlags[chanIndex[chanNum]])
	{
		// Mixed
		//
		// If the sample is mode "4", it won't be played
		if (samp->mode == 1)
			return;

		// Just play the sample. Samples doesn't loop in mixed channels
		virtChannels[chanNum]->PlaySample(samp->sample, 0, samp->length);

		chanData->releaseStart  = 0;
		chanData->releaseLength = 0;
	}
	else
	{
		// Normal
		//
		// If the sample is mode "8", it won't be played
		if (samp->mode == 0)
			return;

		// Set the channel volume
		chanVol[chanIndex[chanNum]] = samp->volume;

		// Does the sample loop?
		if (samp->repeatLength == 0)
		{
			// Nop
			virtChannels[chanNum]->PlaySample(samp->sample, 0, samp->length);

			chanData->releaseStart  = 0;
			chanData->releaseLength = 0;
		}
		else
		{
			// Yep
			virtChannels[chanNum]->PlaySample(samp->sample, 0, samp->repeatStart + samp->repeatLength);
			virtChannels[chanNum]->SetLoop(samp->repeatStart, samp->repeatLength);

			chanData->releaseStart  = samp->repeatStart + samp->repeatLength;
			chanData->releaseLength = samp->length - chanData->releaseStart;
		}
	}

	// Remember the sample number
	chanData->sample = samp->sample;

	// Find the period
	chanData->currNote   = note;
	chanData->currPeriod = periods[note];
	virtChannels[chanNum]->SetAmigaPeriod(chanData->currPeriod);
}



/******************************************************************************/
/* SetVolumes() sets the volume for each channel.                             */
/******************************************************************************/
void Oktalyzer::SetVolumes(void)
{
	uint32 i, j;

	// Start to copy the volumes
	chanVol[4] = chanVol[0];
	chanVol[5] = chanVol[1];
	chanVol[6] = chanVol[2];
	chanVol[7] = chanVol[3];

	// Now, set the volume
	for (i = 0, j = 0; i < 4; i++, j++)
	{
		virtChannels[j]->SetVolume(chanVol[i] * 4);

		if (channelFlags[i])
			virtChannels[++j]->SetVolume(chanVol[i] * 4);
	}
}



/******************************************************************************/
/* DoEffects() runs all the effects.                                          */
/******************************************************************************/
void Oktalyzer::DoEffects(void)
{
	uint32 i, j;

	for (i = 0, j = 0; i < 4; i++, j++)
	{
		DoChannelEffect(j);

		if (channelFlags[i])
			DoChannelEffect(++j);
	}
}



/******************************************************************************/
/* DoChannelEffect() runs the effects on one channel.                         */
/*                                                                            */
/* Input:  "chanNum" is the channel number to use.                            */
/******************************************************************************/
void Oktalyzer::DoChannelEffect(uint32 chanNum)
{
	static const int8 arp10[] = { 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 };
	static const int8 arp12[] = { 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3 };

	PatternLine *pattData;
	ChannelInfo *chanData;

	// Get the pattern and channel data
	pattData = &currLine[chanNum];
	chanData = &chanInfo[chanNum];

	switch (pattData->effect)
	{
		// Effect '1': Portamento down
		case 1:
		{
			chanData->currPeriod -= pattData->effectArg;
			if (chanData->currPeriod < 113)
				chanData->currPeriod = 113;

			virtChannels[chanNum]->SetAmigaPeriod(chanData->currPeriod);
			break;
		}

		// Effect '2': Portamento up
		case 2:
		{
			chanData->currPeriod += pattData->effectArg;
			if (chanData->currPeriod > 856)
				chanData->currPeriod = 856;

			virtChannels[chanNum]->SetAmigaPeriod(chanData->currPeriod);
			break;
		}

		// Effect 'A': Arpeggio type 1
		case 10:
		{
			int8 workNote = chanData->currNote;
			int8 arpNum   = arp10[speedCounter];

			switch (arpNum)
			{
				// Note - upper 4 bits
				case 0:
				{
					workNote -= ((pattData->effectArg & 0xf0) >> 4);
					break;
				}

				// Note
				case 1:
					break;

				// Note + lower 4 bits
				case 2:
				{
					workNote += (pattData->effectArg & 0x0f);
					break;
				}
			}

			PlayNote(chanNum, chanData, workNote);
			break;
		}

		// Effect 'B': Arpeggio type 2
		case 11:
		{
			int8 workNote = chanData->currNote;

			switch (speedCounter & 0x3)
			{
				// Note
				case 0:
				case 2:
					break;

				// Note + lower 4 bits
				case 1:
				{
					workNote += (pattData->effectArg & 0x0f);
					break;
				}

				// Note - upper 4 bits
				case 3:
				{
					workNote -= ((pattData->effectArg & 0xf0) >> 4);
					break;
				}
			}

			PlayNote(chanNum, chanData, workNote);
			break;
		}

		// Effect 'C': Arpeggio type 3
		case 12:
		{
			int8 workNote = chanData->currNote;
			int8 arpNum   = arp12[speedCounter];

			if (arpNum == 0)
				break;

			switch (arpNum)
			{
				// Note - upper 4 bits
				case 1:
				{
					workNote -= ((pattData->effectArg & 0xf0) >> 4);
					break;
				}

				// Note + lower 4 bits
				case 2:
				{
					workNote += (pattData->effectArg & 0x0f);
					break;
				}

				// Note
				case 3:
					break;
			}

			PlayNote(chanNum, chanData, workNote);
			break;
		}

		// Effect 'H': Increase note once per line
		case 17:
		{
			if (speedCounter != 0)
				break;

			// Falling through
		}

		// Effect 'U': Increase note once per tick
		case 30:
		{
			chanData->currNote += pattData->effectArg;
			PlayNote(chanNum, chanData, chanData->currNote);
			break;
		}

		// Effect 'L': Decrease note once per line
		case 21:
		{
			if (speedCounter != 0)
				break;

			// Falling through
		}

		// Effect 'D': Decrease note once per tick
		case 13:
		{
			chanData->currNote -= pattData->effectArg;
			PlayNote(chanNum, chanData, chanData->currNote);
			break;
		}

		// Effect 'F': Filter control
		case 15:
		{
			if (speedCounter == 0)
				filterStatus = pattData->effectArg;

			break;
		}

		// Effect 'P': Position jump
		case 25:
		{
			if (speedCounter == 0)
			{
				uint16 newPos = ((pattData->effectArg & 0xf0) >> 4) * 10 + (pattData->effectArg & 0x0f);

				if (newPos < songLength)
					newSongPos = newPos;
			}
			break;
		}

		// Effect 'R': Release sample
		case 27:
		{
			if ((chanData->releaseStart != 0) && (chanData->releaseLength != 0))
				virtChannels[chanNum]->PlayReleasePart(chanData->sample + chanData->releaseStart, chanData->releaseLength);

			break;
		}

		// Effect 'S': Set speed
		case 28:
		{
			if ((speedCounter == 0) && ((pattData->effectArg & 0xf) != 0))
				currentSpeed = pattData->effectArg & 0xf;

			break;
		}

		// Effect 'O': Volume control with retrig
		case 24:
		{
			chanVol[chanIndex[chanNum]] = chanVol[chanIndex[chanNum] + 4];

			// Falling through
		}

		// Effect 'V': Volume control
		case 31:
		{
			int8 *volPoi = &chanVol[chanIndex[chanNum]];
			uint8 effArg = pattData->effectArg;

			if (effArg <= 64)
			{
				// Set the volume
				*volPoi = effArg;
				break;
			}

			effArg -= 64;
			if (effArg < 16)
			{
				// Decrease the volume every tick
				*volPoi -= effArg;
				if (*volPoi < 0)
					*volPoi = 0;

				break;
			}

			effArg -= 16;
			if (effArg < 16)
			{
				// Increase the volume every tick
				*volPoi += effArg;
				if (*volPoi > 64)
					*volPoi = 64;

				break;
			}

			effArg -= 16;
			if (effArg < 16)
			{
				// Decrease the volume every line
				if (speedCounter == 0)
				{
					*volPoi -= effArg;
					if (*volPoi < 0)
						*volPoi = 0;
				}
				break;
			}

			effArg -= 16;
			if (effArg < 16)
			{
				// Increase the volume every line
				if (speedCounter == 0)
				{
					*volPoi += effArg;
					if (*volPoi > 64)
						*volPoi = 64;
				}
				break;
			}
			break;
		}
	}
}



/******************************************************************************/
/* PlayNote() plays the note on the channel given.                            */
/*                                                                            */
/* Input:  "chanNum" is the channel number to use.                            */
/*         "chanData" is a pointer to the channel structure.                  */
/*         "note" is the note to play.                                        */
/******************************************************************************/
void Oktalyzer::PlayNote(uint32 chanNum, ChannelInfo *chanData, int8 note)
{
	// Check for out of bounds
	if (note < 0)
		note = 0;

	if (note > 35)
		note = 35;

	// Play the note
	chanData->currPeriod = periods[note];
	virtChannels[chanNum]->SetAmigaPeriod(chanData->currPeriod);
}
