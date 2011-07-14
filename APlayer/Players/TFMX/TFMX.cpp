/******************************************************************************/
/* TFMX Player Interface.                                                     */
/*                                                                            */
/* Original player by Chris Huelsbeck.                                        */
/* Converted to C by Jonathan H. Pickard and some fixes by Per Linden.        */
/* Add-On by Thomas Neumann.                                                  */
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
#include "PChecksums.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "TFMX.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.01f



/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define BUFSIZE				1024



/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
static const int32 nul = 0;
static const uint16 pan4[] = { APPAN_LEFT, APPAN_RIGHT, APPAN_RIGHT, APPAN_LEFT };
static const uint16 pan7[] = { APPAN_LEFT, APPAN_RIGHT, APPAN_RIGHT,
							   APPAN_LEFT, APPAN_RIGHT, APPAN_RIGHT, APPAN_LEFT };

// MD5 checksum for Danger Freak Title song
static const uint8 dangerFreakTitle[] =
{
	0x0a, 0x7b, 0xc5, 0x73, 0x1c, 0x51, 0xf8, 0x1b,
	0x6c, 0x88, 0xe3, 0xd6, 0x03, 0x13, 0xca, 0xba
};

// MD5 checksum for Gem X Title song
static const uint8 gemXTitle[] =
{
	0x92, 0x31, 0xbe, 0xb5, 0x3a, 0x18, 0xb2, 0xf4,
	0xfc, 0x0d, 0x4d, 0xce, 0x0c, 0x1c, 0xa6, 0x79
};



/******************************************************************************/
/* Periods                                                                    */
/******************************************************************************/
int32 const noteVals[] =
{
	0x6ae, 0x64e, 0x5f4, 0x59e, 0x54d, 0x501,
	0x4b9, 0x475, 0x435, 0x3f9, 0x3c0, 0x38c, 0x358, 0x32a, 0x2fc, 0x2d0, 0x2a8, 0x282,
	0x25e, 0x23b, 0x21b, 0x1fd, 0x1e0, 0x1c6, 0x1ac, 0x194, 0x17d, 0x168, 0x154, 0x140,
	0x12f, 0x11e, 0x10e, 0x0fe, 0x0f0, 0x0e3, 0x0d6, 0x0ca, 0x0bf, 0x0b4, 0x0aa, 0x0a0,
	0x097, 0x08f, 0x087, 0x07f, 0x078, 0x071, 0x0d6, 0x0ca, 0x0bf, 0x0b4, 0x0aa, 0x0a0,
	0x097, 0x08f, 0x087, 0x07f, 0x078, 0x071, 0x0d6, 0x0ca, 0x0bf, 0x0b4
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
TFMX::TFMX(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	oneFile     = false;

	musicData   = NULL;
	sampleData  = NULL;
	sampleEnd   = NULL;

	tbuf[0]     = NULL;
	tbuf[1]     = NULL;
	tbuf[2]     = NULL;
	tbuf[3]     = NULL;
	tbuf[4]     = NULL;
	tbuf[5]     = NULL;
	tbuf[6]     = NULL;

	outBuf[0]   = NULL;
	outBuf[1]   = NULL;
	outBuf[2]   = NULL;
	outBuf[3]   = NULL;
	outBuf[4]   = NULL;
	outBuf[5]   = NULL;
	outBuf[6]   = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
TFMX::~TFMX(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float TFMX::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the add-on.                    */
/*                                                                            */
/* Output: Is the number of the add-ons.                                      */
/******************************************************************************/
int32 TFMX::GetCount(void)
{
	return (3);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 TFMX::GetSupportFlags(int32 index)
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
PString TFMX::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_TFMX_OLD + index);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString TFMX::GetDescription(int32 index)
{
	PString name, description;

	name = GetName(index);
	description.Format_S1(res, IDS_TFMX_DESCRIPTION, name);

	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString TFMX::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_TFMX_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a TFMX module.               */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result TFMX::ModuleCheck(int32 index, PFile *file)
{
	// Check the module size
	if (file->GetLength() < 512)
		return (AP_UNKNOWN);

	if ((index == modOld) && IsTFMXOld(file))
		return (AP_OK);

	if ((index == modPro) && IsTFMXPro(file))
		return (AP_OK);

	if ((index == mod7V) && IsTFMX7V(file))
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
ap_result TFMX::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	int32 x, y, z;
	uint16 *sh, *lg;
	int32 startOffset = 0;
	ap_result retVal = AP_ERROR;

	try
	{
		PMD5 md5;
		const uint8 *checksum;

		if (oneFile)
			startOffset = header.headerSize;

		// Skip the mark and other stuff
		file->Seek(startOffset + 16, PFile::pSeekBegin);

		// Read the comment block
		for (x = 0; x < 6; x++)
		{
			file->ReadString(comment[x], 40);

			if (comment[x][39] == 0x0a)
				comment[x][39] = 0;
		}

		// Read the subsong information
		file->ReadArray_B_UINT16s(songStart, 32);
		file->ReadArray_B_UINT16s(songEnd, 32);
		file->ReadArray_B_UINT16s(tempo, 32);

		// Get the start offsets to the module information
		file->Seek(16, PFile::pSeekCurrent);

		trackStart = file->Read_B_UINT32();
		pattStart  = file->Read_B_UINT32();
		macroStart = file->Read_B_UINT32();

		if (trackStart == 0)
			trackStart = 0x180;
		else
			trackStart = (trackStart - 0x200) >> 2;

		if (pattStart == 0)
			pattStart = 0x80;
		else
			pattStart = (pattStart - 0x200) >> 2;

		if (macroStart == 0)
			macroStart = 0x100;
		else
			macroStart = (macroStart - 0x200) >> 2;

		if (file->IsEOF())
		{
			errorStr.LoadString(res, IDS_TFMX_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Skip the last part of the header
		file->Seek(36, PFile::pSeekCurrent);

		// Now read the rest of the file
		if (oneFile)
			musicLen = header.mdatSize;
		else
			musicLen = file->GetLength() - file->GetPosition();

		musicData = new int32[16384];
		if (musicData == NULL)
		{
			errorStr.LoadString(res, IDS_TFMX_ERR_MEMORY);
			throw PUserException();
		}

		memset(musicData, 0, 16384 * sizeof(int32));
		file->Read(musicData, musicLen);
		musicData[(musicLen + 3) / sizeof(int32)] = -1;

		// Now calculate a MD5 checksum on the rest of the file,
		// just to find out if it is a special module that needs
		// to be take special care of
		md5.AddBuffer((uint8 *)musicData, musicLen);
		checksum = md5.CalculateChecksum();

		gemx            = 0;
		dangerFreakHack = 0;

		// Check the checksum
		if (memcmp(checksum, dangerFreakTitle, 16) == 0)
			dangerFreakHack = 1;

		if (memcmp(checksum, gemXTitle, 16) == 0)
			gemx = 1;

		// Now that we have pointers to most everything, this would be a good
		// time to fix everything we can... fix endianess on tracksteps,
		// convert pointers to array indices and endianess the patterns and
		// instrument macros. We fix the macros first, then the patterns, and
		// then the tracksteps (because we have to know when the patterns
		// begin to know the tracksteps end...)
		z      = macroStart;
		macros = &musicData[z];

		for (x = 0; x < 128; x++)
		{
			y = (P_BENDIAN_TO_HOST_INT32(musicData[z]) - 0x200);
			if ((y & 3) || ((y >> 2) > musicLen))	// Probably not strictly right
				break;

			musicData[z++] = y >> 2;
		}

		numMac = x;

		// Endianess the pattern pointers
		z        = pattStart;
		patterns = &musicData[z];

		// Changed the calculation of the pattern numbers by Thomas Neumann
		numPat = min(macroStart - pattStart, 128);

		for (x = 0; x < numPat; x++)
		{
			y = (P_BENDIAN_TO_HOST_INT32(musicData[z]) - 0x200);
			if (/*(y & 3) ||*/ ((y >> 2) > musicLen))
				break;

			musicData[z++] = y >> 2;
		}

		// Endianess the trackstep data
		lg = (uint16 *)&musicData[patterns[0]];
		sh = (uint16 *)&musicData[trackStart];
		numTS = (patterns[0] - trackStart) >> 2;

		while (sh < lg)
		{
			x = P_BENDIAN_TO_HOST_INT16(*sh);
			*sh++ = (uint16)x;
		}

		// Now the song is fully loaded, except for the sample data.
		// Everything is done but fixing endianess on the actual
		// pattern and macro data. The routines that use the data do
		// it for themselves
		if (oneFile)
		{
			// Okay, we need to load the sample data here
			sampleLen  = header.smplSize;
			sampleData = new int8[sampleLen];
			if (sampleData == NULL)
			{
				errorStr.LoadString(res, IDS_TFMX_ERR_MEMORY);
				throw PUserException();
			}

			// Read the sample data
			file->Seek(header.headerSize + header.mdatSize, PFile::pSeekBegin);
			file->Read(sampleData, sampleLen);
			sampleEnd = sampleData + sampleLen;
		}
		else
		{
			// Two file format. Find the other file and read the samples
			// from it
			PString fileName;
			PFile *extraFile = NULL;

			// Get the file name of the main module
			fileName = file->GetFullPath();

			try
			{
				extraFile = OpenExtraFile(fileName, "smpl");
			}
			catch(...)
			{
				;
			}

			try
			{
				if (extraFile == NULL)
					extraFile = OpenExtraFile(fileName, "sam");

				try
				{
					// Allocate memory to the samples
					sampleLen  = extraFile->GetLength();
					sampleData = new int8[sampleLen];
					if (sampleData == NULL)
					{
						errorStr.LoadString(res, IDS_TFMX_ERR_MEMORY);
						throw PUserException();
					}

					// Read the sample data
					extraFile->Read(sampleData, sampleLen);
					sampleEnd = sampleData + sampleLen;
				}
				catch(...)
				{
					CloseExtraFile(extraFile);
					throw;
				}

				CloseExtraFile(extraFile);
			}
			catch(PFileException e)
			{
				errorStr.LoadString(res, IDS_TFMX_ERR_LOADING_SAMPLES);
				throw PUserException();
			}
		}

		retVal = AP_OK;
	}
	catch(PUserException e)
	{
		// Just catch the exception and clean up
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
bool TFMX::InitPlayer(int32 index)
{
	int32 i;

	// Remember the module type
	modType = (ModType)index;

	// Clear all the structures
	memset(hdb, 0, sizeof(hdb));
	memset(&mdb, 0, sizeof(struct Mdb));
	memset(cdb, 0, sizeof(cdb));
	memset(&pdb, 0, sizeof(struct PdBlk));
	memset(&idb, 0, sizeof(struct Idb));

	// Initialize some of the member variables
	outRate = mixerFreq;
	eClocks = 14318;

	if (modType == mod7V)
		multiMode = 1;
	else
		multiMode = 0;

	// Allocate mixer buffers
	for (i = 0; i < 4; i++)
	{
		tbuf[i] = new int32[BUFSIZE];
		if (tbuf[i] == NULL)
			throw PMemoryException();

		memset(tbuf[i], 0, BUFSIZE * sizeof(int32));

		outBuf[i] = new int16[BUFSIZE];
		if (outBuf[i] == NULL)
			throw PMemoryException();
	}

	if (multiMode)
	{
		for (i = 4; i < 7; i++)
		{
			tbuf[i] = new int32[BUFSIZE];
			if (tbuf[i] == NULL)
				throw PMemoryException();

			memset(tbuf[i], 0, BUFSIZE * sizeof(int32));

			outBuf[i] = new int16[BUFSIZE];
			if (outBuf[i] == NULL)
				throw PMemoryException();
		}
	}

	// Calculate the time for each subsong
	CalcTimes();

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void TFMX::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void TFMX::InitSound(int32 index, uint16 songNum)
{
	// Initialize member variables
	currentSong = songNum;

	firstTime   = true;

	jiffies     = 0;
	loops       = 0;			// Infinity loop on the modules
	startPat    = -1;
	eRem        = 0;

	// Initialize the player
	TfmxInit();
	StartSong(songNum, 0);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void TFMX::Play(void)
{
	static int32 nb = 0, bd = 0;	// Num bytes, bytes done
	int32 n;
	bool stop = false;

	while (bd < BUFSIZE)
	{
		while (nb > 0)
		{
			n = BUFSIZE - bd;

			if (n > nb)
				n = nb;

			MixIt(n, bd);

			bd    += n;
			nb    -= n;

			if (bd == BUFSIZE)
			{
				stop = true;
				break;
			}
		}

		if (stop)
			break;

		if (mdb.playerEnable)
		{
			DoAllMacros();

			if (mdb.currSong >= 0)
				DoTracks();

			nb    = (eClocks * (outRate >> 1));
			eRem += (nb % 357955);
			nb   /= 357955;

			if (eRem > 357955)
			{
				nb++;
				eRem -= 357955;
			}
		}
		else
			bd = BUFSIZE;
	}

	Conv16(tbuf[0], outBuf[0], bd);
	Conv16(tbuf[1], outBuf[1], bd);
	Conv16(tbuf[2], outBuf[2], bd);
	Conv16(tbuf[3], outBuf[3], bd);

	if (multiMode)
	{
		Conv16(tbuf[4], outBuf[4], bd);
		Conv16(tbuf[5], outBuf[5], bd);
		Conv16(tbuf[6], outBuf[6], bd);
	}

	bd = 0;

	// Tell APlayer what to play
	SetupChannel(0);
	SetupChannel(1);
	SetupChannel(2);
	SetupChannel(3);

	if (multiMode)
	{
		SetupChannel(4);
		SetupChannel(5);
		SetupChannel(6);
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void TFMX::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = outRate;
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 TFMX::GetModuleChannels(void)
{
	if (modType == mod7V)
		return (7);

	return (4);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *TFMX::GetSubSongs(void)
{
	int8 i;
	int16 songNum = -1;
	int16 maxNumberOfZero = 2;

	// Find the number of subsongs
	for (i = 0; (i < 32) && (maxNumberOfZero > 0); i++)
	{
		songNum++;
		if (songStart[i] == 0)
			maxNumberOfZero--;

		// Extra check added by Thomas Neumann
		if ((songStart[i] == 0x1ff) || (songEnd[i] == 0x1ff))
			break;

		// Check for Abandoned Places - Part1_2 and Part1_3.
		// There is only one subsong. Added by Thomas Neumann
		if (songStart[i] >= numTS)
			break;

		if ((songStart[i] == songEnd[i]) && (songStart[i] == 0) && (songStart[i + 1] == 0))
			break;
	}

	subSongs[0] = songNum == 0 ? 1 : songNum;	// Number of subsongs
	subSongs[1] = 0;							// Start song

	return (subSongs);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 TFMX::GetSongLength(void)
{
	return (pdb.lastPos - pdb.firstPos + 1);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 TFMX::GetSongPosition(void)
{
	return (pdb.currPos - pdb.firstPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void TFMX::SetSongPosition(int16 pos)
{
	SongTime *songTime;
	PosInfo posInfo;

	// Set the speed and tempo
	songTime     = songTimeList.GetItem(currentSong);
	posInfo      = songTime->posInfoList.GetItem(pos);
	mdb.ciaSave  = eClocks = posInfo.cia;
	mdb.speedCnt = pdb.prescale = posInfo.speed;

	// Change the position
	pdb.currPos = pdb.firstPos + pos;
	GetTrackStep();
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
PTimeSpan TFMX::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	SongTime *songTime;
	int32 i, count;

	// First get the pointer to the song time structure
	songTime = songTimeList.GetItem(songNum);

	// Copy the position times
	count = songTime->posInfoList.CountItems();
	for (i = 0; i < count; i++)
		posTimes.AddTail(songTime->posInfoList.GetItem(i).time);

	return (songTime->totalTime);
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
bool TFMX::GetInfoString(uint32 line, PString &description, PString &value)
{
	PCharSet_Amiga charSet;

	// Is the line number out of range?
	if (line >= 9)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Used Tracksteps
		case 0:
		{
			description.LoadString(res, IDS_TFMX_INFODESCLINE0);
			value.SetUNumber(numTS);
			break;
		}

		// Used Patterns
		case 1:
		{
			description.LoadString(res, IDS_TFMX_INFODESCLINE1);
			value.SetUNumber(numPat);
			break;
		}

		// Used Instruments
		case 2:
		{
			description.LoadString(res, IDS_TFMX_INFODESCLINE2);
			value.SetUNumber(numMac);
			break;
		}

		// Comment
		case 3:
		{
			description.LoadString(res, IDS_TFMX_INFODESCLINE3);
			value.SetString(comment[0], &charSet);
			break;
		}

		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		{
			description.MakeEmpty();
			value.SetString(comment[line - 3], &charSet);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* IsTFMXOld() tests the module to see if it's a TFMX 1.5 module.             */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: True if the module is in TFMX 1.5, else false.                     */
/******************************************************************************/
bool TFMX::IsTFMXOld(PFile *file)
{
	char buf[9];
	int32 startOffset = 0;

	// First check for one-file format
	if (IsOneFile(file, &header))
	{
		oneFile = true;

		// Check to see if the module is forced or not checked
		if ((header.type & 128) || ((header.type & 127) == 0))
		{
			// Well, we can't count on the type now, so we skip
			// the header and make our own check
			startOffset = header.headerSize;
		}
		else
		{
			// Is the type 1.5?
			if ((header.type & 127) == 1)
				return (true);

			return (false);
		}
	}

	// Check for two-file format. Read the mark
	file->Seek(startOffset, PFile::pSeekBegin);
	file->Read(buf, 9);

	// And then check it
	if ((strncmp(buf, "TFMX", 4) == 0) && (strncmp(&buf[5], "SONG", 4) != 0))
		return (true);

	return (false);
}



/******************************************************************************/
/* IsTFMXPro() tests the module to see if it's a TFMX Pro module.             */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: True if the module is in TFMX Pro, else false.                     */
/******************************************************************************/
bool TFMX::IsTFMXPro(PFile *file)
{
	char buf[9];
	int32 startOffset = 0;
	bool ok = true;
	bool getNext;
	int16 i;
	int16 times = 0;
	uint16 position;
	uint32 offset;
	uint16 songStarts[31];

	// First check for one-file format
	if (IsOneFile(file, &header))
	{
		oneFile = true;

		// Check to see if the module is forced or not checked
		if ((header.type & 128) || ((header.type & 127) == 0))
		{
			// Well, we can't count on the type now, so we skip
			// the header and make our own check
			startOffset = header.headerSize;
		}
		else
		{
			// Is the type Professional?
			if ((header.type & 127) == 2)
				return (true);

			return (false);
		}
	}

	// Check for two-file format. Read the mark
	file->Seek(startOffset, PFile::pSeekBegin);
	file->Read(buf, 9);

	// And then check it
	if ((strncmp(buf, "TFMX-SONG", 9) != 0) && (strncmp(buf, "TFMX_SONG", 9) != 0) && (strncasecmp(buf, "TFMXSONG", 8) != 0))
		return (false);

	// Okay, it is a TFMX module, but is it a Professional module?
	//
	// Get the start positions
	file->Seek(startOffset + 0x100, PFile::pSeekBegin);
	file->ReadArray_B_UINT16s(songStarts, 31);

	// Get the trackstep offset
	file->Seek(startOffset + 0x1d0, PFile::pSeekBegin);
	offset = file->Read_B_UINT32();
	if (offset == 0)
		offset = 0x800;

	// Take all the subsongs
	for (i = 0; i < 31; i++)
	{
		getNext = true;

		// Get the current subsong start position
		position = songStarts[i];
		if (position == 0x1ff)
			break;

		// Read the trackstep information
		while (getNext)
		{
			// Find the position in the file where the current trackstep
			// information to read is stored
			file->Seek(startOffset + offset + position * 16, PFile::pSeekBegin);

			// If the trackstep information isn't a command, stop
			// the checking
			if (file->Read_B_UINT16() != 0xeffe)
				getNext = false;
			else
			{
				// Get the command
				switch (file->Read_B_UINT16())
				{
					// Loop a section
					case 1:
					{
						if (times == 0)
						{
							times = -1;
							position++;
						}
						else
						{
							if (times < 0)
							{
								position = file->Read_B_UINT16();
								times = file->Read_B_UINT16() - 1;
							}
							else
							{
								times--;
								position = file->Read_B_UINT16();
							}
						}
						break;
					}

					// Set tempo + Start master volume slide
					case 2:
					case 4:
					{
						position++;
						break;
					}

					// Timeshare
					case 3:
					{
						ok = false;
						position++;
						break;
					}

					// Unknown command
					default:
					{
						getNext = false;
						ok      = true;
						break;
					}
				}
			}
		}

		if (!ok)
			break;
	}

	return (ok);
}



/******************************************************************************/
/* IsTFMX7V() tests the module to see if it's a TFMX 7-Voices module.         */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*                                                                            */
/* Output: True if the module is in TFMX 7-Voices, else false.                */
/******************************************************************************/
bool TFMX::IsTFMX7V(PFile *file)
{
	char buf[9];
	int32 startOffset = 0;
	bool ok = false;
	bool getNext;
	int16 i;
	int16 times = 0;
	uint16 position;
	uint32 offset;
	uint16 songStarts[31];

	// First check for one-file format
	if (IsOneFile(file, &header))
	{
		oneFile = true;

		// Check to see if the module is forced or not checked
		if ((header.type & 128) || ((header.type & 127) == 0))
		{
			// Well, we can't count on the type now, so we skip
			// the header and make our own check
			startOffset = header.headerSize;
		}
		else
		{
			// Is the type 7-Voices?
			if ((header.type & 127) == 3)
				return (true);

			return (false);
		}
	}

	// Check for two-file format. Read the mark
	file->Seek(startOffset, PFile::pSeekBegin);
	file->Read(buf, 9);

	// And then check it
	if ((strncmp(buf, "TFMX-SONG", 9) != 0) && (strncmp(buf, "TFMX_SONG", 9) != 0) && (strncasecmp(buf, "TFMXSONG", 8) != 0))
		return (false);

	// Okay, it is a TFMX module, but is it a 7-Voices module?
	//
	// Get the start positions
	file->Seek(startOffset + 0x100, PFile::pSeekBegin);
	file->ReadArray_B_UINT16s(songStarts, 31);

	// Get the trackstep offset
	file->Seek(startOffset + 0x1d0, PFile::pSeekBegin);
	offset = file->Read_B_UINT32();
	if (offset == 0)
		offset = 0x800;

	// Take all the subsongs
	for (i = 0; i < 31; i++)
	{
		getNext = true;

		// Get the current subsong start position
		position = songStarts[i];
		if (position == 0x1ff)
			break;

		// Read the trackstep information
		while (getNext)
		{
			// Find the position in the file where the current trackstep
			// information to read is stored
			file->Seek(startOffset + offset + position * 16, PFile::pSeekBegin);

			// If the trackstep information isn't a command, stop
			// the checking
			if (file->Read_B_UINT16() != 0xeffe)
				getNext = false;
			else
			{
				// Get the command
				switch (file->Read_B_UINT16())
				{
					// Loop a section
					case 1:
					{
						if (times == 0)
						{
							times = -1;
							position++;
						}
						else
						{
							if (times < 0)
							{
								position = file->Read_B_UINT16();
								times = file->Read_B_UINT16() - 1;
							}
							else
							{
								times--;
								position = file->Read_B_UINT16();
							}
						}
						break;
					}

					// Set tempo + Start master volume slide
					case 2:
					case 4:
					{
						position++;
						break;
					}

					// Timeshare
					case 3:
					{
						ok = true;
						position++;
						break;
					}

					// Unknown command
					default:
					{
						getNext = false;
						ok      = false;
						break;
					}
				}
			}
		}

		if (ok)
			break;
	}

	return (ok);
}



/******************************************************************************/
/* IsOneFile() will check the current file to see if it's in one-file format. */
/*      If that is true, it will load the structure.                          */
/*                                                                            */
/* Input:  "file" is a pointer to the file object with the file to check.     */
/*         "header" is a pointer to store the file header.                    */
/*                                                                            */
/* Output: True if the file is a one-file else false.                         */
/******************************************************************************/
bool TFMX::IsOneFile(PFile *file, OneFile *header)
{
	// Seek to the start of the file
	file->SeekToBegin();

	header->mark = file->Read_B_UINT32();
	if (header->mark == 'TFHD')
	{
		// Okay, it seems it's a one-file, so read the whole structure
		header->headerSize = file->Read_B_UINT32();
		header->type       = file->Read_UINT8();
		header->version    = file->Read_UINT8();
		header->mdatSize   = file->Read_B_UINT32();
		header->smplSize   = file->Read_B_UINT32();

		return (true);
	}

	return (false);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void TFMX::Cleanup(void)
{
	int32 i, count;

	// Delete the song time list
	count = songTimeList.CountItems();
	for (i = 0; i < count; i++)
		delete songTimeList.GetItem(i);

	songTimeList.MakeEmpty();

	// Delete mixer buffers
	for (i = 0; i < 7; i++)
	{
		delete[] outBuf[i];
		outBuf[i] = NULL;

		delete[] tbuf[i];
		tbuf[i] = NULL;
	}

	// Delete the module
	delete[] sampleData;
	delete[] musicData;

	sampleEnd  = NULL;
	sampleData = NULL;
	musicData  = NULL;
}



/******************************************************************************/
/* CalcTimes() calculate the time each subsong takes.                         */
/******************************************************************************/
void TFMX::CalcTimes(void)
{
	const uint16 *subs = GetSubSongs();
	SongTime *songTime;
	PosInfo posInfo;
	uint16 *posData;
	uint16 i, j, k, l;
	uint16 startPos, endPos;
	int16 trackLoop;
	uint16 cia, spd;
	int32 x;
	bool endPatt;
	struct Pdb pattInfo[8];
	float total;

	// Initialize some of the temp variables
	memset(pattInfo, 0, sizeof(pattInfo));

	// Take each subsong
	for (i = 0; i < subs[0]; i++)
	{
		// Allocate a new song time structure
		songTime = new SongTime;
		if (songTime == NULL)
			throw PMemoryException();

		// Add the structure to the list
		songTimeList.AddTail(songTime);

		// Get the start and end positions of the subsong
		startPos = songStart[i];
		endPos   = songEnd[i];

		// Calculate the start tempo
		if (tempo[i] >= 0x10)
		{
			cia = 0x1b51f8 / tempo[i];
			spd = 5;
		}
		else
		{
			cia = 14318;
			spd = tempo[i];
		}

		trackLoop = -1;
		total     = 0.0f;

		// Now take each position
		for (j = startPos; j <= endPos; j++)
		{
			// Add the position information to the list
			posInfo.cia   = cia;
			posInfo.speed = spd;
			posInfo.time.SetTimeSpan(total);

			if ((j - startPos) >= songTime->posInfoList.CountItems())
				songTime->posInfoList.AddTail(posInfo);

loop:
			// Get the pointer to the position data
			posData = (uint16 *)&musicData[trackStart + (j * 4)];

			// Special position command?
			if (posData[0] == 0xeffe)
			{
				switch (posData[1])
				{
					// Stop
					case 0:
					{
						j = endPos;
						break;
					}

					// Loop
					case 1:
					{
						if (!trackLoop--)
						{
							trackLoop = -1;
							break;
						}
						else
						{
							if (trackLoop < 0)
							{
								trackLoop = posData[3];
								if (trackLoop == 0)
								{
									j = endPos;
									break;
								}
							}
						}

						// If we jump back, the module has probably ended
						if ((posData[2] < j) && (trackLoop < 0))
						{
							j = endPos;
							break;
						}

						j = posData[2];
						goto loop;
					}

					// Speed
					case 2:
					{
						spd = posData[2];

						if (!(posData[3] & 0xf200) && (x = (posData[3] & 0x1ff) > 0xf))
							cia = 0x1b51f8 / x;

						break;
					}

					// Timeshare
					case 3:
					{
						if (!((x = posData[3]) & 0x8000))
						{
							x = ((char)x) < -0x20 ? -0x20 : (char)x;
							cia = (14318 * (x + 100)) / 100;
						}
						break;
					}
				}
			}
			else
			{
				// Initialize pattern pointers
				for (k = 0; k < 8; k++)
				{
					if ((pattInfo[k].pNum = (posData[k] >> 8)) < 0x80)
					{
						pattInfo[k].pStep  = 0;
						pattInfo[k].pWait  = 0;
						pattInfo[k].pLoop  = 0xffff;
						pattInfo[k].pAddr  = patterns[pattInfo[k].pNum];
						pattInfo[k].looped = false;
					}
				}

				// Take each row in the pattern
				endPatt = false;

				do
				{
					for (k = 0; k < 8; k++)
					{
						if (!DoAChannelRow(pattInfo, &pattInfo[k], k))
							endPatt = true;
					}

					// Add the time it takes to play the current row
					if (!endPatt)
					{
						total += ((cia * 1.3968255) / 1000.0f) * (spd + 1);

						// Check to see if all the channels are stopped.
						// If so, the pattern ends
						l = 0;
						for (k = 0; k < 8; k++)
						{
							if (pattInfo[k].pNum >= 0x90)
								l++;
						}

						if (l == 8)
							endPatt = true;
					}
				}
				while (!endPatt);
			}
		}

		// Set the total time
		songTime->totalTime.SetTimeSpan(total);
	}
}



/******************************************************************************/
/* DoAChannelRow() parse one single track.                                    */
/*                                                                            */
/* Input:  "p" is a pointer to the track.                                     */
/*         "chan" is the channel number.                                      */
/*                                                                            */
/* Output: True to continue with the next channel, false indicate that the    */
/*         row has ended.                                                     */
/******************************************************************************/
bool TFMX::DoAChannelRow(struct Pdb *p1, struct Pdb *p, int32 chan)
{
	UNI x;
	uint8 t;
	uint16 i, j;

	if (p->pNum == 0xfe)
	{
		p->pNum++;
		return (true);
	}

	if (!p->pAddr)
		return (true);

	if (p->pNum >= 0x90)
		return (true);

	if (p->pWait--)
		return (true);

	while (1)
	{
		x.l = P_BENDIAN_TO_HOST_INT32(musicData[p->pAddr + p->pStep++]);
		t   = x.b.b0;

		if (t < 0xf0)
		{
			if ((t & 0xc0) == 0x80)
			{
				p->pWait = x.b.b3;
				x.b.b3   = 0;
			}

			x.b.b0 = ((t + p->pxPose) & 0x3f);

			if ((t & 0xc0) == 0xc0)
				x.b.b0 |= 0xc0;

			if ((t & 0xc0) == 0x80)
				return (true);
		}
		else
		{
			switch (t & 0xf)
			{
				// NOP
				case 15:
					break;

				// End
				case 0:
				{
					p->pNum = 0xff;
					return (false);
				}

				// Loop
				case 1:
				{
					if (!(p->pLoop))
					{
						p->pLoop = 0xffff;
						break;
					}
					else
					{
						if (p->pLoop == 0xffff)	// FF --'ed
							p->pLoop = x.b.b1;
					}

					p->pLoop--;
					p->pStep = x.w.w1;

					if (p->pLoop == 0xffff)
					{
						// Loop infinity, means the module ends, but
						// only if all the other channels are stopped
						p->looped = true;

						j = 0;
						for (i = 0; i < 8; i++)
						{
							if ((p1[i].pNum >= 0x90) || p1[i].looped)
								j++;
						}

						if (j == 8)
							return (false);
					}
					else
						p->looped = false;

					break;
				}

				// GsPt
				case 8:
				{
					p->prOAddr = p->pAddr;
					p->prOStep = p->pStep;

					// Fall through to...
				}

				// Cont
				case 2:
				{
					p->pAddr = patterns[x.b.b1];
					p->pStep = x.w.w1;
					break;
				}

				// Wait
				case 3:
				{
					p->pWait = x.b.b1;
					return (true);
				}

				// Stop
				case 4:
				{
					p->pNum = 0xff;
					return (true);
				}

				// RoPt
				case 9:
				{
					p->pAddr = p->prOAddr;
					p->pStep = p->prOStep;
					break;
				}

				// PPat
				case 11:
				{
					t = x.b.b2 & 0x07;

					pdb.p[t].pNum   = x.b.b1;
					pdb.p[t].pAddr  = patterns[x.b.b1];
					pdb.p[t].pxPose = x.b.b3;
					pdb.p[t].pStep  = 0;
					pdb.p[t].pWait  = 0;
					pdb.p[t].pLoop  = 0xffff;
					break;
				}
			}
		}
	}
}



/******************************************************************************/
/* SetupChannel() setup the APlayer channel.                                  */
/*                                                                            */
/* Input:  "chan" is the channel number.                                      */
/******************************************************************************/
void TFMX::SetupChannel(int32 chan)
{
	APChannel *channel;

	channel = virtChannels[chan];

	// Fill out the Channel object
	channel->SetBuffer(outBuf[chan], BUFSIZE);
	channel->SetVolume(256);

	if (multiMode)
		channel->SetPanning(pan7[chan]);
	else
		channel->SetPanning(pan4[chan]);
}



/******************************************************************************/
/* NotePort() setup the note period.                                          */
/*                                                                            */
/* Input:  "i" is the sound code.                                             */
/******************************************************************************/
void TFMX::NotePort(uint32 i)
{
	UNI x;
	struct Cdb *c;

	x.l = i;
	c = &cdb[x.b.b2 & (multiMode ? 7 : 3)];

	if (x.b.b0 == 0xfc)
	{
		// Lock
		c->sfxFlag     = x.b.b1;
		c->sfxLockTime = x.b.b3;
		return;
	}

	if (c->sfxFlag)
		return;

	if (x.b.b0 < 0xc0)
	{
		if (!dangerFreakHack)
			c->finetune = x.b.b3;
		else
			c->finetune = 0;

		c->velocity      = (x.b.b2 >> 4) & 0xf;
		c->prevNote      = c->currNote;
		c->currNote      = x.b.b0;
		c->reallyWait    = 1;
		c->newStyleMacro = 0xff;
		c->macroPtr      = macros[c->macroNum = x.b.b1];
		c->macroStep     = c->macroWait = 0;
		c->efxRun        = 0;
		c->keyUp         = 1;
		c->loop          = -1;
		c->macroRun      = -1;
	}
	else
	{
		if (x.b.b0 < 0xf0)
		{
			c->portaReset = x.b.b1;
			c->portaTime  = 1;

			if (!c->portaRate)
				c->portaPer = c->destPeriod;

			c->portaRate  = x.b.b3;
			c->destPeriod = (uint16)(noteVals[c->currNote = (x.b.b0 & 0x3f)]);
		}
		else
		{
			switch (x.b.b0)
			{
				// Enve
				case 0xf7:
				{
					c->envRate   = x.b.b1;
					c->envReset  = c->envTime = (x.b.b2 >> 4) + 1;
					c->envEndVol = x.b.b3;
					break;
				}

				// Vibr
				case 0xf6:
				{
					c->vibTime   = (c->vibReset = (x.b.b1 & 0xfe)) >> 1;
					c->vibWidth  = x.b.b3;
					c->vibFlag   = 1;	// ?!
					c->vibOffset = 0;
					break;
				}

				// Kup^
				case 0xf5:
				{
					c->keyUp = 0;
					break;
				}
			}
		}
	}
}



/******************************************************************************/
/* LoopOff() doesn't do anything.                                             */
/*                                                                            */
/* Input:  "hw" is a pointer to the current channel.                          */
/*                                                                            */
/* Output: Always 1.                                                          */
/******************************************************************************/
int32 TFMX::LoopOff(struct Hdb *hw)
{
	return (1);
}



/******************************************************************************/
/* LoopOn() turn on loop.                                                     */
/*                                                                            */
/* Input:  "hw" is a pointer to the current channel.                          */
/*                                                                            */
/* Output: Always 1.                                                          */
/******************************************************************************/
int32 TFMX::LoopOn(struct Hdb *hw)
{
	if (!hw->c)
		return (1);

	if (hw->c->waitDMACount--)
		return (1);

	hw->loop        = &LoopOff;
	hw->c->macroRun = (int8)0xff;
	return (1);
}



/******************************************************************************/
/* RunMacro() runs a macro.                                                   */
/*                                                                            */
/* Input:  "c" is a pointer to the macro information.                         */
/******************************************************************************/
#define MAYBEWAIT \
	if (c->newStyleMacro == 0x0) \
	{ \
		c->newStyleMacro = 0xff; \
		break; \
	} \
	else \
	{ \
		return; \
	}


void TFMX::RunMacro(struct Cdb *c)
{
	UNI x;
	register int32 a;
	int8 tempVol;

	c->macroWait = 0;

loop:
	x.l    = P_BENDIAN_TO_HOST_INT32(musicData[c->macroPtr + (c->macroStep++)]);
	a      = x.b.b0;
	x.b.b0 = 0;

	switch (a)
	{
		// Dma off + reset
		case 0:
		{
			c->envReset  = c->vibReset = c->addBeginTime = 0;
			c->portaRate = 0;

			if (gemx)
			{
				if (x.b.b2)
					c->curVol = x.b.b3;
				else
					c->curVol = x.b.b3 + (c->velocity) * 3;
			}

			// Fall through
		}

		// Dma off
		case 0x13:
		{
			c->hw->loop = &LoopOff;

			if (!x.b.b1)
			{
				c->hw->mode = 0;

				// Added by Stefan Ohlsson
				// Removes glitch in Turrican II World 2 Song 0, among others
				if (c->newStyleMacro)
					c->hw->sLen = 0;

				break;
			}
			else
			{
				c->hw->mode     |= 4;
				c->newStyleMacro = 0;
				return;
			}
		}

		// Dma on
		case 0x1:
		{
			c->efxRun   = x.b.b1;
			c->hw->mode = 1;

			if ((!c->newStyleMacro) || (dangerFreakHack))
			{
				c->hw->sampleStart  = &sampleData[c->saveAddr];
				c->hw->sampleLength = (c->saveLen) ? c->saveLen << 1 : 131072;
				c->hw->sBeg         = c->hw->sampleStart;
				c->hw->sLen         = c->hw->sampleLength;
				c->hw->pos          = 0;
				c->hw->mode        |= 2;
				break;
			}
			else
				break;
		}

		// Setbegin
		case 0x2:
		{
			c->addBeginTime = 0;
			c->saveAddr     = c->curAddr = x.l;
			break;
		}

		// Addbegin
		case 0x11:
		{
			c->addBeginTime = c->addBeginReset = x.b.b1;
			a = c->curAddr + (c->addBegin = (int16)x.w.w1);

			c->saveAddr = c->curAddr = a;
			break;
		}

		// Setlen
		case 0x3:
		{
			c->saveLen = c->currLength = x.w.w1;
			break;
		}

		// Addlen
		case 0x12:
		{
			c->currLength += x.w.w1;
			a = c->currLength;

			c->saveLen = (uint16)a;
			break;
		}

		// Wait
		case 0x4:
		{
			if (x.b.b1 & 0x01)
			{
				if (c->reallyWait++)
					return;
			}

			c->macroWait = x.w.w1;
			MAYBEWAIT;
		}

		// Wait on DMA
		case 0x1a:
		{
			c->hw->loop     = &LoopOn;
			c->hw->c        = c;
			c->waitDMACount = x.w.w1;
			c->macroRun     = 0;
			MAYBEWAIT;
		}

		// Note split
		case 0x1c:
		{
			if (c->currNote > x.b.b1)
				c->macroStep = x.w.w1;

			break;
		}

		// Vol split
		case 0x1d:
		{
			if (c->curVol > x.b.b1)
				c->macroStep = x.w.w1;

			break;
		}

		// Loop key up
		case 0x10:
		{
			if (!c->keyUp)
				break;
		}

		// Loop
		case 0x5:
		{
			if (!(c->loop--))
				break;
			else
			{
				if (c->loop < 0)
					c->loop = x.b.b1 - 1;
			}

			c->macroStep = x.w.w1;
			break;
		}

		// Stop
		case 0x7:
		{
			c->macroRun = 0;
			return;
		}

		// Add volume
		case 0xd:
		{
			if (x.b.b2 != 0xfe)
			{
				// --- neofix ---
				tempVol = (c->velocity * 3) + x.b.b3;
				if (tempVol > 0x40)
					c->curVol = 0x40;
				else
					c->curVol = tempVol;

				break;
			}

			// NOTSUPPORTED
			break;
		}

		// Set volume
		case 0xe:
		{
			if (x.b.b2 != 0xfe)
			{
				c->curVol = x.b.b3;
				break;
			}

			// NOTSUPPORTED
			break;
		}

		// Start macro
		case 0x21:
		{
			x.b.b0  = c->currNote;
			x.b.b2 |= c->velocity << 4;
			NotePort(x.l);
			break;
		}

		// Set prev note
		case 0x1f:
		{
			a = c->prevNote;
			goto SetNote;
		}

		// Add note
		case 0x8:
		{
			a = c->currNote;
			goto SetNote;
		}

		// Set note
		case 0x9:
		{
			a = 0;
SetNote:
			a = (noteVals[a + x.b.b1 & 0x3f] * (0x100 + c->finetune + (int8)x.b.b3)) >> 8;
			c->destPeriod = (uint16)a;

			if (!c->portaRate)
				c->curPeriod = (uint16)a;

			MAYBEWAIT;
		}

		// Set period
		case 0x17:
		{
			c->destPeriod = x.w.w1;

			if (!c->portaRate)
				c->curPeriod = x.w.w1;

			break;
		}

		// Portamento
		case 0xb:
		{
			c->portaReset = x.b.b1;
			c->portaTime  = 1;

			if (!c->portaRate)
				c->portaPer = c->destPeriod;

			c->portaRate = x.w.w1;
			break;
		}

		// Vibrato
		case 0xc:
		{
			c->vibTime  = (c->vibReset = x.b.b1) >> 1;
			c->vibWidth = x.b.b3;
			c->vibFlag  = 1;

			if (!c->portaRate)
			{
				c->curPeriod = c->destPeriod;
				c->vibOffset = 0;
			}
			break;
		}

		// Envelope
		case 0xf:
		{
			c->envReset  = c->envTime = x.b.b2;
			c->envEndVol = x.b.b3;
			c->envRate   = x.b.b1;
			break;
		}

		// Reset
		case 0xa:
		{
			c->envReset  = c->vibReset = c->addBeginTime = 0;
			c->portaRate = 0;
			break;
		}

		// Wait key up
		case 0x14:
		{
			// Fix by Thomas Neumann so the "Master Blaster - load" don't crash
			if (!c->keyUp)
				break;

			if (!c->loop)
			{
				c->loop = -1;
				break;
			}

			if (c->loop == -1)
				c->loop = x.b.b3 - 1;
			else
				c->loop--;

			c->macroStep--;
			return;
		}

		// Go sub
		case 0x15:
		{
			c->returnPtr  = (uint16)c->macroPtr;
			c->returnStep = c->macroStep;
		}

		// Continue
		case 0x6:
		{
			c->macroPtr  = (c->macroNum = (uint16)macros[x.b.b1]);
			c->macroStep = x.w.w1;
			c->loop      = (uint16)0xffff;
			break;
		}

		// Return sub
		case 0x16:
		{
			c->macroPtr  = c->returnPtr;
			c->macroStep = c->returnStep;
			break;
		}

		// Sample loop
		case 0x18:
		{
			c->saveAddr  += (x.w.w1 & 0xfffe);
			c->saveLen   -= x.w.w1 >> 1;
			c->currLength = c->saveLen;
			c->curAddr    = c->saveAddr;
			break;
		}

		// Oneshot
		case 0x19:
		{
			c->addBeginTime = 0;
			c->saveAddr     = c->curAddr = 0;
			c->saveLen      = c->currLength = 1;
			break;
		}

		// Cue
		case 0x20:
		{
			idb.cue[x.b.b1 & 0x03] = x.w.w1;
			break;
		}

		// Turrican 3 title - we can safely ignore
		case 0x31:
			break;

		default:
		{
			// NOTSUPPORTED
			break;
		}
	}

	goto loop;
}



/******************************************************************************/
/* DoEffects() will run the run-always effects, like vibrato and portamento.  */
/*                                                                            */
/* Input:  "c" is a pointer to the macro information.                         */
/******************************************************************************/
void TFMX::DoEffects(struct Cdb *c)
{
	register int32 a = 0;

	if (c->efxRun < 0)
		return;

	if (!c->efxRun)
	{
		c->efxRun = 1;
		return;
	}

	if (c->addBeginTime)
	{
		c->curAddr += c->addBegin;
		c->saveAddr = c->curAddr;
		c->addBeginTime--;

		if (!c->addBeginTime)
		{
			c->addBegin     = -c->addBegin;
			c->addBeginTime = c->addBeginReset;
		}
	}

	if (c->curAddr < 0)
	{
		c->addBegin     = 0;
		c->addBeginTime = c->addBeginReset = 0;
		c->curAddr      = 0;
	}

	if (c->vibReset)
	{
		a = (c->vibOffset += c->vibWidth);
		a = (c->destPeriod * (0x800 + a)) >> 11;

		if (!c->portaRate)
			c->curPeriod = (uint16)a;

		if (!(--c->vibTime))
		{
			c->vibTime  = c->vibReset;
			c->vibWidth = -c->vibWidth;
		}
	}

	if ((c->portaRate) && ((--c->portaTime) == 0))
	{
		c->portaTime = c->portaReset;

		if (c->portaPer > c->destPeriod)
		{
			a = (c->portaPer * (256 - c->portaRate) - 128) >> 8;

			if (a <= c->destPeriod)
				c->portaRate = 0;
		}
		else
		{
			if (c->portaPer < c->destPeriod)
			{
				a = (c->portaPer * (256 + c->portaRate)) >> 8;

				if (a >= c->destPeriod)
					c->portaRate = 0;
			}
			else
				c->portaRate = 0;
		}

		if (!c->portaRate)
			a = c->destPeriod;

		c->portaPer = c->curPeriod = (uint16)a;
	}

	if ((c->envReset) && (!(c->envTime--)))
	{
		c->envTime = c->envReset;

		if (c->curVol > c->envEndVol)
		{
			if (c->curVol < c->envRate)
				c->envReset = 0;
			else
				c->curVol -= c->envRate;

			if (c->envEndVol > c->curVol)
				c->envReset = 0;
		}
		else
		{
			if (c->curVol < c->envEndVol)
			{
				c->curVol += c->envRate;

				if (c->envEndVol < c->curVol)
					c->envReset = 0;
			}
		}

		if (!c->envReset)
		{
			c->envReset = c->envTime = 0;
			c->curVol   = c->envEndVol;
		}
	}

	if ((mdb.fadeSlope) && ((--mdb.fadeTime) == 0))
	{
		mdb.fadeTime   = mdb.fadeReset;
		mdb.masterVol += mdb.fadeSlope;

		if (mdb.fadeDest == mdb.masterVol)
			mdb.fadeSlope = 0;
	}
}



/******************************************************************************/
/* DoMacro() runs the macro on one channel.                                   */
/*                                                                            */
/* Input:  "cc" is the channel to run the macro on.                           */
/******************************************************************************/
void TFMX::DoMacro(int32 cc)
{
	struct Cdb *c = &cdb[cc];
	int32 a;

	// Locking
	if (c->sfxLockTime >= 0)
		c->sfxLockTime--;
	else
		c->sfxFlag = c->sfxPriority = 0;

	if ((a = c->sfxCode))
	{
		c->sfxFlag = 0;
		c->sfxCode = 0;
		NotePort(a);
		c->sfxFlag = c->sfxPriority;
	}

	if ((c->macroRun) && !(c->macroWait--))
		RunMacro(c);

	DoEffects(c);

	// Has to be here because of if (efxRun = 1)
	c->hw->delta = (c->curPeriod) ? (3579545 << 9) / (c->curPeriod * outRate >> 5) : 0;
	c->hw->sampleStart = &sampleData[c->saveAddr];

	c->hw->sampleLength = (c->saveLen) ? c->saveLen << 1 : 131072;
	if ((c->hw->mode & 3) == 1)
	{
		c->hw->sBeg = c->hw->sampleStart;
		c->hw->sLen = c->hw->sampleLength;
	}

	c->hw->vol = (c->curVol * mdb.masterVol) >> 6;
}



/******************************************************************************/
/* DoAllMacros() runs the macros for each voice.                              */
/******************************************************************************/
void TFMX::DoAllMacros(void)
{
	DoMacro(0);
	DoMacro(1);
	DoMacro(2);

	if (multiMode)
	{
		DoMacro(4);
		DoMacro(5);
		DoMacro(6);
		DoMacro(7);
	}

	DoMacro(3);
}



/******************************************************************************/
/* ChannelOff() turns off a single channel.                                   */
/*                                                                            */
/* Input:  "i" is the channel to turn off.                                    */
/******************************************************************************/
void TFMX::ChannelOff(int32 i)
{
	struct Cdb *c;

	c = &cdb[i & 0xf];
	if (!c->sfxFlag)
	{
		c->hw->mode      = 0;
		c->addBeginTime  = c->addBeginReset = c->macroRun = 0;
		c->newStyleMacro = 0xff;
		c->saveAddr      = c->curVol = c->hw->vol = 0;
		c->saveLen       = c->currLength = 1;
		c->hw->loop      = &LoopOff;
		c->hw->c         = c;
	}
}



/******************************************************************************/
/* DoFade() fades the volume.                                                 */
/*                                                                            */
/* Input:  "sp" is the speed.                                                 */
/*         "dv" is the end volume.                                            */
/******************************************************************************/
void TFMX::DoFade(int8 sp, int8 dv)
{
	mdb.fadeDest = dv;
	if (!(mdb.fadeTime = mdb.fadeReset = sp) || (mdb.masterVol == sp))
	{
		mdb.masterVol = dv;
		mdb.fadeSlope = 0;
		return;
	}

	mdb.fadeSlope = (mdb.masterVol > mdb.fadeDest) ? -1 : 1;
}



/******************************************************************************/
/* GetTrackStep() parses a track step.                                        */
/******************************************************************************/
void TFMX::GetTrackStep(void)
{
	uint16 *l;
	int32 x, y;

loop:
/*	if (loops)
	{
		if ((pdb.currPos == pdb.firstPos) && (!(--loops)))
		{
			mdb.playerEnable = 0;
			endReached = true;
			return;
		}
	}
*/

	// Check for "end of module"
	if (pdb.currPos == pdb.firstPos)
	{
		if (firstTime)
			firstTime = false;
		else
			endReached = true;
	}

	l = (uint16 *)&musicData[trackStart + (pdb.currPos * 4)];

	jiffies = 0;
	if ((l[0]) == 0xeffe)
	{
		switch (l[1])
		{
			// Stop
			case 0:
			{
//				mdb.playerEnable = 0;
				endReached = true;
				return;
			}

			// Loop
			case 1:
			{
				if (loops)
				{
					if (!(--loops))
					{
//						mdb.playerEnabled = 0;
						endReached = true;
						return;
					}
				}

				if (!(mdb.trackLoop--))
				{
					mdb.trackLoop = -1;
					pdb.currPos++;
					goto loop;
				}
				else
				{
					if (mdb.trackLoop < 0)
					{
						mdb.trackLoop = l[3];
						if (mdb.trackLoop == 0)
							endReached = true;
					}
				}

				// If we jump back, the module has probably ended
				if ((l[2] < pdb.currPos) && (mdb.trackLoop < 0))
					endReached = true;

				pdb.currPos = l[2];
				goto loop;
			}

			// Speed
			case 2:
			{
				mdb.speedCnt = pdb.prescale = l[2];

				if (!(l[3] & 0xf200) && (x = (l[3] & 0x1ff) > 0xf))
					mdb.ciaSave = eClocks = 0x1b51f8 / x;

				pdb.currPos++;
				goto loop;
			}

			// Timeshare
			case 3:
			{
				if (!((x = l[3]) & 0x8000))
				{
					x = ((char)x) < -0x20 ? -0x20 : (char)x;
					mdb.ciaSave = eClocks = (14318 * (x + 100)) / 100;
					multiMode = 1;
				}

				pdb.currPos++;
				goto loop;
			}

			// Fade
			case 4:
			{
				DoFade(l[2] & 0xff, l[3] & 0xff);
				pdb.currPos++;
				goto loop;
			}

			default:
			{
				pdb.currPos++;
				goto loop;
			}
		}
	}
	else
	{
		for (x = 0; x < 8; x++)
		{
			pdb.p[x].pxPose = (int)(l[x] & 0xff);

			if ((y = pdb.p[x].pNum = (l[x] >> 8)) < 0x80)
			{
				pdb.p[x].pStep  = 0;
				pdb.p[x].pWait  = 0;
				pdb.p[x].pLoop  = 0xffff;
				pdb.p[x].pAddr  = patterns[y];
				pdb.p[x].looped = false;
			}
		}
	}
}



/******************************************************************************/
/* DoTrack() parse one single track.                                          */
/*                                                                            */
/* Input:  "p" is a pointer to the track.                                     */
/*         "pp" is the channel number.                                        */
/*                                                                            */
/* Output: ??                                                                 */
/******************************************************************************/
int32 TFMX::DoTrack(struct Pdb *p, int32 pp)
{
	UNI x;
	uint8 t;
	uint16 i, j;

	if (p->pNum == 0xfe)
	{
		p->pNum++;
		ChannelOff(p->pxPose);
		return (0);
	}

	if (!p->pAddr)
		return (0);

	if (p->pNum >= 0x90)
		return (0);

	if (p->pWait--)
		return (0);

	while (1)
	{
loop:
		x.l = P_BENDIAN_TO_HOST_INT32(musicData[p->pAddr + p->pStep++]);
		t   = x.b.b0;

		if (t < 0xf0)
		{
			if ((t & 0xc0) == 0x80)
			{
				p->pWait = x.b.b3;
				x.b.b3   = 0;
			}

			x.b.b0 = ((t + p->pxPose) & 0x3f);

			if ((t & 0xc0) == 0xc0)
				x.b.b0 |= 0xc0;

			NotePort(x.l);

			if ((t & 0xc0) == 0x80)
				return (0);

			goto loop;
		}

		switch (t & 0xf)
		{
			// NOP
			case 15:
				break;

			// End
			case 0:
			{
				p->pNum = 0xff;

				if (pdb.currPos == pdb.lastPos)
				{
					pdb.currPos = pdb.firstPos;
					ChangePosition();
					endReached = true;
				}
				else
				{
					pdb.currPos++;
					ChangePosition();
				}

				GetTrackStep();
				return (1);
			}

			// Loop
			case 1:
			{
				if (!(p->pLoop))
				{
					p->pLoop = 0xffff;
					break;
				}
				else
				{
					if (p->pLoop == 0xffff)	// FF --'ed
						p->pLoop = x.b.b1;
				}

				p->pLoop--;
				p->pStep = x.w.w1;

				// Infinity check added by Thomas Neumann,
				// so Turrican II - Alien world song 8 and others can be stopped
				if (p->pLoop == 0xffff)
				{
					// Loop infinity, means the module ends, but
					// only if all the other channels are stopped
					p->looped = true;

					j = 0;
					for (i = 0; i < 8; i++)
					{
						if ((pdb.p[i].pNum >= 0x90) || pdb.p[i].looped)
							j++;
					}

					if (j == 8)
						endReached = true;
				}
				else
					p->looped = false;

				break;
			}

			// GsPt
			case 8:
			{
				p->prOAddr = p->pAddr;
				p->prOStep = p->pStep;

				// Fall through to...
			}

			// Cont
			case 2:
			{
				p->pAddr = patterns[x.b.b1];
				p->pStep = x.w.w1;
				break;
			}

			// Wait
			case 3:
			{
				p->pWait = x.b.b1;
				return (0);
			}

			// StCu
			case 14:
			{
				mdb.playPattFlag = 0;
			}

			// Stop
			case 4:
			{
				p->pNum = 0xff;
				return (0);
			}

			case 5:		// Kup^
			case 6:		// Vibr
			case 7:		// Enve
			case 12:	// Lock
			{
				NotePort(x.l);
				break;
			}

			// RoPt
			case 9:
			{
				p->pAddr = p->prOAddr;
				p->pStep = p->prOStep;
				break;
			}

			// Fade
			case 10:
			{
				DoFade(x.b.b1, x.b.b3);
				break;
			}

			// Cue
			case 13:
			{
				idb.cue[x.b.b1 & 0x03] = x.w.w1;
				break;
			}

			// PPat
			case 11:
			{
				t = x.b.b2 & 0x07;

				pdb.p[t].pNum   = x.b.b1;
				pdb.p[t].pAddr  = patterns[x.b.b1];
				pdb.p[t].pxPose = x.b.b3;
				pdb.p[t].pStep  = 0;
				pdb.p[t].pWait  = 0;
				pdb.p[t].pLoop  = 0xffff;
				break;
			}
		}
	}
}



/******************************************************************************/
/* DoTracks() parse all the tracks.                                           */
/******************************************************************************/
void TFMX::DoTracks(void)
{
	int32 x, y;

	jiffies++;

	if (!mdb.speedCnt--)
	{
		mdb.speedCnt = pdb.prescale;

		for (x = 0; x < 8; x++)
		{
			if (DoTrack(&pdb.p[x], x))
			{
				x = -1;
				continue;
			}
		}

		// Extra check for "end of module" added by Thomas Neumann
		y = 0;
		for (x = 0; x < 8; x++)
		{
			if (pdb.p[x].pNum >= 0x90)
				y++;
		}

		if (y == 8)
		{
			if (pdb.currPos == pdb.lastPos)
			{
				pdb.currPos = pdb.firstPos;
				ChangePosition();
				endReached = true;
			}
			else
			{
				pdb.currPos++;
				ChangePosition();
			}

			GetTrackStep();
		}
	}
}



/******************************************************************************/
/* AllOff() turns all the channels off.                                       */
/******************************************************************************/
void TFMX::AllOff(void)
{
	int32 x;
	struct Cdb *c;

	mdb.playerEnable = 0;

	for (x = 0; x < 8; x++)
	{
		c                   = &cdb[x];
		c->hw               = &hdb[x];
		c->hw->c            = c;		// Wait on dma

		hdb[x].mode         = 0;
		c->macroWait        = c->macroRun = c->sfxFlag = c->curVol =
							  c->sfxFlag = c->sfxCode = c->saveAddr = 0;
		hdb[x].vol          = 0;
		c->loop             = c->newStyleMacro = c->sfxLockTime = -1;

		c->hw->sBeg         = c->hw->sampleStart = sampleData;
		c->hw->sampleLength = c->hw->sLen = c->saveLen = 2;
		c->hw->loop         = &LoopOff;
	}
}



/******************************************************************************/
/* TfmxInit() initialize the player so it's ready to roll.                    */
/******************************************************************************/
void TFMX::TfmxInit(void)
{
	int32 x;

	AllOff();

	for (x = 0; x < 8; x++)
	{
		hdb[x].c       = &cdb[x];
		pdb.p[x].pNum  = 0xff;
		pdb.p[x].pAddr = 0;

		ChannelOff(x);
	}
}



/******************************************************************************/
/* StartSong() initialize the player with the subsong given.                  */
/*                                                                            */
/* Input:  "song" is the subsong to play.                                     */
/*         "mode" is the song mode.                                           */
/******************************************************************************/
void TFMX::StartSong(int32 song, int32 mode)
{
	int32 x;

	mdb.playerEnable = 0;		// Sort of locking mechanism
	mdb.masterVol    = 0x40;
	mdb.fadeSlope    = 0;
	mdb.trackLoop    = -1;
	mdb.playPattFlag = 0;
	mdb.ciaSave      = eClocks = 14318;		// Assume 125 bpm, NTSC timing

	if (mode != 2)
	{
		pdb.currPos = pdb.firstPos = songStart[song];
		pdb.lastPos = songEnd[song];

		if ((x = tempo[song]) >= 0x10)
		{
			mdb.ciaSave  = eClocks = 0x1b51f8 / x;
			pdb.prescale = 5;		// Fix by Thomas Neumann
		}
		else
			pdb.prescale = x;
	}

	for (x = 0; x < 8; x++)
	{
		pdb.p[x].pAddr  = 0;
		pdb.p[x].pNum   = 0xff;
		pdb.p[x].pxPose = 0;
		pdb.p[x].pStep  = 0;
	}

	if (mode != 2)
		GetTrackStep();

	if (startPat != -1)
	{
		pdb.currPos = pdb.firstPos = startPat;
		GetTrackStep();
		startPat = -1;
	}

	mdb.speedCnt = mdb.endFlag = 0;
	mdb.playerEnable = 1;
}



/******************************************************************************/
/* Conv16() converts a sample buffer to 16 bit.                               */
/*                                                                            */
/* Input:  "b" is a pointer to the buffer to convert.                         */
/*         "out" is a pointer to the destination buffer.                      */
/*         "num" is the number of samples to convert.                         */
/******************************************************************************/
void TFMX::Conv16(int32 *b, int16 *out, int32 num)
{
	int32 x;
	int32 *c = b;
	int16 *a = out;

	for (x = 0; x < num; x++)
		*a++ = (*c++);

	// Clear source buffer
	memset(b, 0, num * sizeof(int32));
}



/******************************************************************************/
/* MixIt() generate the sample data for all channels.                         */
/*                                                                            */
/* Input:  "n" is the number of samples to mix.                               */
/*         "b" is the number of samples done so far.                          */
/******************************************************************************/
void TFMX::MixIt(int32 n, int32 b)
{
	if (multiMode)
	{
		MixAdd(&hdb[4], n, &tbuf[3][b]);
		MixAdd(&hdb[5], n, &tbuf[4][b]);
		MixAdd(&hdb[6], n, &tbuf[5][b]);
		MixAdd(&hdb[7], n, &tbuf[6][b]);
	}
	else
		MixAdd(&hdb[3], n, &tbuf[3][b]);

	MixAdd(&hdb[0], n, &tbuf[0][b]);
	MixAdd(&hdb[1], n, &tbuf[1][b]);
	MixAdd(&hdb[2], n, &tbuf[2][b]);
}



/******************************************************************************/
/* MixAdd() generate the sample data for one channel.                         */
/*                                                                            */
/* Input:  "hw" is a pointer to the "hardware" information.                   */
/*         "n" is the number of samples to generate.                          */
/*         "b" is a pointer to store the sample data.                         */
/******************************************************************************/
void TFMX::MixAdd(struct Hdb *hw, int32 n, int32 *b)
{
	register int8 *p = hw->sBeg;
	register uint32 ps = hw->pos;
	int32 v = hw->vol;
	uint32 d = hw->delta;
	uint32 l = (hw->sLen << 14);

	if ((hw->sampleStart < sampleData) || (hw->sBeg < sampleData))
		return;

	if ((hw->sampleStart >= sampleEnd) || (hw->sBeg >= sampleEnd))
		return;

	if (v > 0x40)
		v = 0x40;

	// This is used to have (p == &sampleData).  Broke with GrandMonsterSlam
	if ((p == (int8 *)&nul) || ((hw->mode & 1) == 0) || (l < 0x10000))
		return;

	if ((hw->mode & 3) == 1)
	{
		p = hw->sBeg = hw->sampleStart;
		l = (hw->sLen = hw->sampleLength) << 14;
		ps = 0;
		hw->mode |= 2;
	}

	while (n--)
	{
		// p[] went out of sampleData, when SetBegin messed up c->saveAddr (sBeg):
		// p[] goes negative on repeated AddBegin:s (R-Type)
		// p[] went out of sampleData on Apidya Title (Fix by Thomas Neumann)
		int32 index = (ps += d) >> 14;

		if ((p + index) >= sampleEnd)
			(*b++) = 0x00;
		else
			(*b++) = p[index] * v * 4;

		if (ps < l)
			continue;

		ps -= l;
		p   = hw->sampleStart;

		if (((l = ((hw->sLen = hw->sampleLength) << 14)) < 0x10000) || (!hw->loop(hw)))
		{
			hw->sLen = ps = d = 0;
			p        = sampleData;
			break;
		}
	}

	hw->sBeg  = p;
	hw->pos   = ps;
	hw->delta = d;

	if (hw->mode & 4)
		hw->mode = 0;
}
