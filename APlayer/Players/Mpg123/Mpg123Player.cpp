/******************************************************************************/
/* MPG123Player Player Interface.                                             */
/*                                                                            */
/* Original player by Michael Hipp.                                           */
/* APlayer plug-in by Claes Löfqvist.                                         */
/* Modified and upgraded by Thomas Neumann.                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
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
#include "Mpg123Player.h"
#include "Mpg123Tables.h"
#include "Mpg123Genre.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.13f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MPG123Player::MPG123Player(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	mpgFile     = NULL;

	pcmSample1  = NULL;
	pcmSample2  = NULL;

	// Initialize parameters
	param.tryResync = true;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Find out which CPU optimazion that can be used
	gotMMX   = HaveMMX();
	got3DNow = Have3DNow();

	// If we both got MMX and 3DNow, use 3DNow
	if (got3DNow)
		gotMMX = false;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MPG123Player::~MPG123Player(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float MPG123Player::GetVersion(void)
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
uint32 MPG123Player::GetSupportFlags(int32 index)
{
	return (appSamplePlayer | appUseRingBuffer | appDontCloseFile | appSetPosition);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString MPG123Player::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MPG_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MPG123Player::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MPG_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString MPG123Player::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_MPG_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a MPEG file.                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to file object with the file to check.         */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result MPG123Player::ModuleCheck(int32 index, PFile *file)
{
	int32 i, pos;
	uint32 header;
	uint8 *chkBuf;
	bool found = false;

	// Start at the beginning of the file
	file->SeekToBegin();
	firstFramePosition = 0;

	// Is the file an ID3 file?
	if ((file->Read_UINT8() == 'I') && (file->Read_UINT8() == 'D') && (file->Read_UINT8() == '3'))
	{
		// Check the version
		if (file->Read_UINT8() == 0x03)
		{
			uint8 buf[4];

			// Skip the minor version and flag byte
			file->Seek(2, PFile::pSeekCurrent);

			// Read the tag size
			file->Read(buf, 4);

			// Remove bit 7 from all the bytes and calculate the size value
			firstFramePosition  = ((buf[0] & 0x7f) << 21) | ((buf[1] & 0x7f) << 14) | ((buf[2] & 0x7f) << 7) | (buf[3] & 0x7f);
			firstFramePosition += 10;	// Add the bytes we already have read
		}
	}

	// Well, some MP3's have a huge block of zeros in the beginning.
	// We try to find the end of this block first and then scan
	chkBuf = new uint8[CHECK_BUF_SIZE * 2];
	if (chkBuf == NULL)
		throw PMemoryException();

	try
	{
		// Clear last part of the memory block
		memset(chkBuf + CHECK_BUF_SIZE, 0, CHECK_BUF_SIZE);

		// Seek to the first frame position
		file->Seek(firstFramePosition, PFile::pSeekBegin);

		for (;;)
		{
			// Read one block
			if (file->Read(chkBuf, CHECK_BUF_SIZE) != CHECK_BUF_SIZE)
			{
				// Well, we reached the end of the file, so stop
				delete[] chkBuf;
				return (AP_UNKNOWN);
			}

			// Check the read block to see if it only contains zeros
			if (memcmp(chkBuf, chkBuf + CHECK_BUF_SIZE, CHECK_BUF_SIZE) != 0)
				break;		// Found a block which has other values than zero

			// Well, increment the frame position
			firstFramePosition += CHECK_BUF_SIZE;
		}
	}
	catch(...)
	{
		delete[] chkBuf;
		throw;
	}

	// Delete the check buffer again
	delete[] chkBuf;

	// Read the first 4 bytes into the check variable
	file->Seek(firstFramePosition, PFile::pSeekBegin);
	header = file->Read_B_UINT32();
	pos    = 4;

	for (;;)
	{
		// Try to find a header in the next 8 kb
		for (; pos < CHECK_BUF_SIZE; pos++)
		{
			if (HeadCheck(header))
			{
				found = true;
				break;
			}

			// Read the next byte and shift it into the check variable
			header <<= 8;
			header  &= 0xffffff00;
			header  |= file->Read_UINT8();
		}

		// We didn't find a header, so we give up
		if (!found)
			return (AP_UNKNOWN);

		// Ok, it seems we found a header, but it could be anything.
		// We will then try to find 10 more headers with the right
		// space between them
		ReadFrameInit(&frame);

		for (i = 0; i < CHECK_FRAMES; i++)
		{
			// Decode the header
			DecodeHeader(&frame, header);

			// Seek to the next frame header
			file->Seek(frame.frameSize, PFile::pSeekCurrent);

			// Read the header
			header = file->Read_B_UINT32();
			if (file->IsEOF())
				return (AP_UNKNOWN);

			// Check the new header
			if (!HeadCheck(header))
			{
				found = false;

				// Set the file position back
				file->Seek(firstFramePosition + pos - 3, PFile::pSeekBegin);

				// Make sure we don't make a double check
				header = file->Read_B_UINT32();
				pos++;
				break;
			}
		}

		if (found)
		{
			firstFramePosition += (pos - 4);
			return (AP_OK);		// Yeah, we found a mpeg file
		}
	}

	// We will never reach this point!
	return (AP_UNKNOWN);
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
ap_result MPG123Player::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	// Well, there is nothing to load, because the file will be loaded
	// a little bit of the time when playing. Just make a copy of the
	// file pointer
	mpgFile = file;

	return (AP_OK);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::InitPlayer(int32 index)
{
	// Initialize CL-Amp informations
	playInfo.type        = 0;
	playInfo.layer       = 0;
	playInfo.bitRate     = 0;
	playInfo.frequency   = 0;
	playInfo.frames      = 0;
	playInfo.channelMode = 0;
	playInfo.emphasis    = 0;
	playInfo.priv        = false;
	playInfo.crc         = false;
	playInfo.copyright   = false;
	playInfo.original    = false;

	endOfFile = false;

	// Initialize all variables
	pcmPoint = 0;
	outscale = 32768;

	memset(&reader, 0, sizeof(reader));
	memset(&mpStr, 0, sizeof(mpStr));
	memset(&frame, 0, sizeof(frame));

	bsBufEnd[0] = 0;
	bsBufEnd[1] = 0;
	bsBuf       = bsSpace[1];
	bsNum       = 0;

	vbr         = 0;

	pnts[0]     = cos64;
	pnts[1]     = cos32;
	pnts[2]     = cos16;
	pnts[3]     = cos8;
	pnts[4]     = cos4;

	memset(ii_table, 0, sizeof(ii_table));
	ii_table[3] = grp_3tab;
	ii_table[5] = grp_5tab;
	ii_table[9] = grp_9tab;

	memset(grp_3tab, 0, sizeof(grp_3tab));
	memset(grp_5tab, 0, sizeof(grp_5tab));
	memset(grp_9tab, 0, sizeof(grp_9tab));

	stereoTabs[0][0][0] = tan1_1;
	stereoTabs[0][0][1] = tan2_1;
	stereoTabs[0][1][0] = tan1_2;
	stereoTabs[0][1][1] = tan2_2;
	stereoTabs[1][0][0] = pow1_1[0];
	stereoTabs[1][0][1] = pow2_1[0];
	stereoTabs[1][1][0] = pow1_2[0];
	stereoTabs[1][1][1] = pow2_2[0];
	stereoTabs[2][0][0] = pow1_1[1];
	stereoTabs[2][0][1] = pow2_1[1];
	stereoTabs[2][1][0] = pow1_2[1];
	stereoTabs[2][1][1] = pow2_2[1];

	ntomVal[0] = NTOM_MUL >> 1;
	ntomVal[1] = NTOM_MUL >> 1;
	ntomStep   = NTOM_MUL;

	halfPhase = 0;
	bo        = 1;

	memset(buffs, 0, sizeof(buffs));
	memset(buffs_MMX, 0, sizeof(buffs_MMX));

	tabLen[0] = 3;
	tabLen[1] = 5;
	tabLen[2] = 9;
	tables[0] = grp_3tab;
	tables[1] = grp_5tab;
	tables[2] = grp_9tab;

	// Initialize decode tables
	MakeDecodeTables(outscale);
	InitLayer2();		// Inits also shared tables with layer1

	// Initialize file reader structure
	DefaultInit(&reader);

	// MPG123 file initializations
	pcmSample1 = new uint8[AUDIOBUFSIZE];
	if (pcmSample1 == NULL)
		return (false);

	// Seek to the position found in the check routine
	mpgFile->Seek(firstFramePosition, PFile::pSeekBegin);
	ReadFrameInit(&frame);

	if (!(ReadFrame(&reader, &frame)))
		return (false);

	// Well, we only need to allocate an extra buffer if the file
	// is in stereo
	if (frame.stereo == 2)
	{
		pcmSample2 = new uint8[AUDIOBUFSIZE];
		if (pcmSample2 == NULL)
			return (false);
	}

	// Remember the file position of the first frame
	firstFramePosition = mpgFile->GetPosition() - frame.frameSize - 4;

	frame.downSampleSbLimit = SBLIMIT;
	frame.downSample        = 0;
	frame.single            = -1;

	// Calculate layer 3 tables
	InitLayer3(frame.downSampleSbLimit);

	// Find out if the file is a VBR file and if so, decode the VBR frame
	DecodeVBRFrame(&frame);

	// Get MP3 Tags
	GetMP3Tags();

	// Find the number of frames and calculate the total time
	SetPlayInfo(&showPlayInfo, &frame);
	showPlayInfo.frames = CalcNumFrames(&frame);
	totalTime           = CalcTotalTime(&frame, showPlayInfo.frames);
	timePerFrame        = (double)totalTime / (double)showPlayInfo.frames;

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void MPG123Player::EndPlayer(int32 index)
{
	// Delete back buffer
	delete[] reader.backBuf;
	reader.backBuf = NULL;

	// Delete audio buffers
	delete[] pcmSample2;
	pcmSample2 = NULL;

	delete[] pcmSample1;
	pcmSample1 = NULL;
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void MPG123Player::InitSound(int32 index, uint16 songNum)
{
	oldPos           = 0;
	bitUpdateCounter = 1;
	oldBitRate       = -1;

	// Make sure we are on the first frame
	mpgFile->Seek(firstFramePosition, PFile::pSeekBegin);
	ReadFrameInit(&frame);

	currFrame = 0;
	currTime  = 0;
	pcmPoint  = 0;
	endOfFile = false;
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void MPG123Player::Play(void)
{
	APChannel *channel;
	int32 size;

	// Decode the next buffer
	while ((pcmPoint == 0) && (!endOfFile))
	{
		if (ReadFrame(&reader, &frame))
		{
			PlayFrame(&mpStr, &frame);
			SetPlayInfo(&playInfo, &frame);
			currTime = (int32)(currFrame * timePerFrame);
			currFrame++;
		}
		else
			endOfFile = true;
	}

	// Get the number of samples decoded so far and reset the buffer index
	size     = pcmPoint / 2;	// Divide the number of bytes by 2, because it's 16 bit
	pcmPoint = 0;

	if (size != 0)
	{
		// Yeah, we got some samples
		if (playInfo.channelMode == MPG_MD_MONO)
		{
			// Play the sample in mono
			channel = virtChannels[0];
			channel->SetBuffer(pcmSample1, size);
			channel->SetVolume(256);
			channel->SetPanning(APPAN_CENTER);
		}
		else
		{
			// Well, we need to play the sample in stereo
			channel = virtChannels[0];
			channel->SetBuffer(pcmSample1, size);
			channel->SetVolume(256);
			channel->SetPanning(APPAN_LEFT);

			channel = virtChannels[1];
			channel->SetBuffer(pcmSample2, size);
			channel->SetVolume(256);
			channel->SetPanning(APPAN_RIGHT);
		}
	}
	else
	{
		endReached = true;
		SetSongPosition(0);
	}

	bitUpdateCounter--;
	if (bitUpdateCounter == 0)
	{
		bitUpdateCounter = 10;

		if (oldBitRate != playInfo.bitRate)
		{
			ChangeModuleInfo(6, PString::CreateNumber(playInfo.bitRate));
			oldBitRate = playInfo.bitRate;
		}
	}

	int16 pos = GetSongPosition();
	if (pos != oldPos)
	{
		oldPos = pos;
		ChangePosition();
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void MPG123Player::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = showPlayInfo.frequency;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString MPG123Player::GetModuleName(void)
{
	return (songName);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString MPG123Player::GetAuthor(void)
{
	return (artist);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 MPG123Player::GetModuleChannels(void)
{
	return (showPlayInfo.channelMode == MPG_MD_MONO ? 1 : 2);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 MPG123Player::GetSongLength(void)
{
	return (100);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 MPG123Player::GetSongPosition(void)
{
	int16 newPos;

	newPos = (int16)(((float)currTime / (float)totalTime) * 100.0f);
	if (newPos > 99)
		newPos = 99;

	return (newPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void MPG123Player::SetSongPosition(int16 pos)
{
	int32 newTime;
	int32 filePos;

	// Calculate the new position
	newTime   = (totalTime * pos) / 100;
	currFrame = (int32)(((int64)showPlayInfo.frames * newTime) / totalTime);
	currTime  = (int32)(currFrame * timePerFrame);
	pcmPoint  = 0;
	endOfFile = false;

	// Set the file pointer
	if (vbrFile)
	{
		if (pos > 99)
			pos = 99;

		filePos = (int32)((1.0 / 256.0) * vbrTOC[pos] * CalcFileLength());
	}
	else
		filePos = (int32)(((int64)CalcFileLength() * newTime) / totalTime);

	mpgFile->Seek(firstFramePosition + filePos, PFile::pSeekBegin);
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
PTimeSpan MPG123Player::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	int32 i;

	// Calculate the position times
	for (i = 0; i < 100; i++)
		posTimes.AddTail((totalTime * i) / 100);

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
bool MPG123Player::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 16)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Track Number
		case 0:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE00);

			if (trackNum == 0)
				value.LoadString(res, IDS_MPG_INFO_UNKNOWN);
			else
				value.SetUNumber(trackNum);

			break;
		}

		// Album
		case 1:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE01);

			if (album.IsEmpty())
				value.LoadString(res, IDS_MPG_INFO_UNKNOWN);
			else
				value = album;

			break;
		}

		// Year
		case 2:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE02);

			if (year.IsEmpty())
				value.LoadString(res, IDS_MPG_INFO_UNKNOWN);
			else
				value = year;

			break;
		}

		// Genre
		case 3:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE03);

			if (genre.IsEmpty())
				value.LoadString(res, IDS_MPG_INFO_UNKNOWN);
			else
				value = genre;

			break;
		}

		// Comment
		case 4:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE04);

			if (comment.IsEmpty())
				value.LoadString(res, IDS_MPG_INFO_NONE);
			else
				value = comment;

			break;
		}

		// Type
		case 5:
		{
			int32 major, minor;

			description.LoadString(res, IDS_MPG_INFODESCLINE05);

			switch (showPlayInfo.type)
			{
				case 1:
				{
					major = 2;
					minor = 0;
					break;
				}

				case 2:
				{
					major = 2;
					minor = 5;
					break;
				}

				default:
				{
					major = 1;
					minor = 0;
					break;
				}
			}

			value.Format(res, IDS_MPG_INFO_TYPE, major, minor, showPlayInfo.layer);
			break;
		}

		// Bit Rate
		case 6:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE06);
			value.SetNumber(showPlayInfo.bitRate);
			break;
		}

		// Frequency
		case 7:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE07);
			value.SetNumber(showPlayInfo.frequency);
			break;
		}

		// Header offset
		case 8:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE08);
			value.SetNumber(firstFramePosition);
			break;
		}

		// Frames
		case 9:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE09);
			value.SetNumber(showPlayInfo.frames);
			break;
		}

		// Channel Mode
		case 10:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE10);

			switch (showPlayInfo.channelMode)
			{
				case 0:
				{
					value.LoadString(res, IDS_MPG_INFO_CHANNEL0);
					break;
				}

				case 1:
				{
					value.LoadString(res, IDS_MPG_INFO_CHANNEL1);
					break;
				}

				case 2:
				{
					value.LoadString(res, IDS_MPG_INFO_CHANNEL2);
					break;
				}

				case 3:
				{
					value.LoadString(res, IDS_MPG_INFO_CHANNEL3);
					break;
				}
			}
			break;
		}

		// Private
		case 11:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE11);
			value.LoadString(res, showPlayInfo.priv ? IDS_MPG_INFO_YES : IDS_MPG_INFO_NO);
			break;
		}

		// CRCs
		case 12:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE12);
			value.LoadString(res, showPlayInfo.crc ? IDS_MPG_INFO_YES : IDS_MPG_INFO_NO);
			break;
		}

		// Copyrighted
		case 13:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE13);
			value.LoadString(res, showPlayInfo.copyright ? IDS_MPG_INFO_YES : IDS_MPG_INFO_NO);
			break;
		}

		// Original
		case 14:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE14);
			value.LoadString(res, showPlayInfo.original ? IDS_MPG_INFO_YES : IDS_MPG_INFO_NO);
			break;
		}

		// Emphasis
		case 15:
		{
			description.LoadString(res, IDS_MPG_INFODESCLINE15);

			switch (playInfo.emphasis)
			{
				case 0:
				{
					value.LoadString(res, IDS_MPG_INFO_EMPHASIS0);
					break;
				}

				case 1:
				{
					value.LoadString(res, IDS_MPG_INFO_EMPHASIS1);
					break;
				}

				case 3:
				{
					value.LoadString(res, IDS_MPG_INFO_EMPHASIS3);
					break;
				}
			}
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* DecodeVBRFrame() checks the frame loaded for a VBR frame and if so, decode */
/*      it.                                                                   */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame structure to use.                   */
/******************************************************************************/
void MPG123Player::DecodeVBRFrame(Frame *fr)
{
	int32 offset;
	uint32 flags;

	// First of all, we don't know if we have a VBR file yet
	vbrFile       = false;
	vbrFrames     = 0;
	vbrTotalBytes = 0;
	vbrScale      = -1;
	memset(vbrTOC, 0, sizeof(vbrTOC));

	// Only layer 3 have VBR support
	if (fr->lay == 3)
	{
		// Get the start offset of the VBR info
		if (fr->lsf)
			offset = (fr->stereo == 1) ? 9 : 17;
		else
			offset = (fr->stereo == 1) ? 17 : 32;

		// If the frame a VBR frame?
		if ((bsBuf[offset] != 'X') || (bsBuf[offset + 1] != 'i') || (bsBuf[offset + 2] != 'n') || (bsBuf[offset + 3] != 'g'))
			return;

		// Okay, the frame is a VBR frame, so start decode it
		vbrFile = true;
		offset += 4;

		// Get the flags
		flags   = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&bsBuf[offset]));
		offset += 4;

		// Any frames?
		if (flags & VBR_FRAMES_FLAG)
		{
			vbrFrames = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&bsBuf[offset]));
			offset   += 4;
		}

		// Any bytes?
		if (flags & VBR_BYTES_FLAG)
		{
			vbrTotalBytes = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&bsBuf[offset]));
			offset       += 4;
		}

		// Any TOC?
		if (flags & VBR_TOC_FLAG)
		{
			memcpy(vbrTOC, &bsBuf[offset], 100);
			offset += 100;
		}

		// Any scale?
		if (flags & VBR_SCALE_FLAG)
		{
			vbrScale = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&bsBuf[offset]));
			offset  += 4;
		}
	}
}



/******************************************************************************/
/* GetMP3Tags() checks the file for MP3 tags and if available, read it.       */
/******************************************************************************/
void MPG123Player::GetMP3Tags(void)
{
	// We don't know if the file has the tag yet
	tag = false;

	// Seek to the end of the file - 128 bytes
	mpgFile->Seek(-128, PFile::pSeekEnd);

	// Do the file have the MP3 tag?
	if ((mpgFile->Read_UINT8() == 'T') && (mpgFile->Read_UINT8() == 'A') && (mpgFile->Read_UINT8() == 'G'))
	{
		char buffer[32];
		int16 genreNum;
		bool track;
		PCharSet_MS_WIN_1252 charSet;

		// Yup, read it
		tag = true;

		// Song name
		mpgFile->ReadString(buffer, 30);
		buffer[30] = 0x00;
		songName.SetString(buffer, &charSet);
		songName.TrimRight();
		songName.TrimLeft();

		// Artist
		mpgFile->ReadString(buffer, 30);
		buffer[30] = 0x00;
		artist.SetString(buffer, &charSet);
		artist.TrimRight();
		artist.TrimLeft();

		// Album
		mpgFile->ReadString(buffer, 30);
		buffer[30] = 0x00;
		album.SetString(buffer, &charSet);
		album.TrimRight();
		album.TrimLeft();

		// Year
		mpgFile->ReadString(buffer, 4);
		buffer[4] = 0x00;
		year.SetString(buffer, &charSet);
		year.TrimRight();
		year.TrimLeft();

		// Comment
		mpgFile->ReadString(buffer, 29);
		if (buffer[28] != 0x00)
		{
			mpgFile->Read(&buffer[29], 1);
			buffer[30] = 0x00;
			track = false;
		}
		else
		{
			buffer[29] = 0x00;
			track = true;
		}

		comment.SetString(buffer, &charSet);
		comment.TrimRight();
		comment.TrimLeft();

		// Track number
		if (track)
			trackNum = mpgFile->Read_UINT8();
		else
			trackNum = 0;

		// Genre
		genreNum = mpgFile->Read_UINT8();

		if ((genreNum < 148) && (genreNum >= 0))
			genre = genreTable[genreNum];
	}
	else
	{
		// Clear the tag values
		songName.MakeEmpty();
		artist.MakeEmpty();
		album.MakeEmpty();
		year.MakeEmpty();
		comment.MakeEmpty();
		genre.MakeEmpty();
		trackNum = 0;
	}
}



/******************************************************************************/
/* SetPlayInfo() initialize CL-Amp player info structure                      */
/*                                                                            */
/* Input:  "info" is a pointer to the player info structure.                  */
/*         "fr" is a pointer to the frame structure to use.                   */
/******************************************************************************/
void MPG123Player::SetPlayInfo(PlayerInfoStruct *info, Frame *fr)
{
	info->type        = (fr->lsf == 0) ? 0 : (fr->mpeg25 ? 2 : 1);
	info->layer       = fr->lay;
	info->bitRate     = tabSel_123[fr->lsf][fr->lay - 1][fr->bitRateIndex];
	info->frequency   = freqs[fr->samplingFrequency];
	info->channelMode = fr->mode;
	info->emphasis    = fr->emphasis;
	info->priv        = fr->extension != 0;
	info->crc         = fr->errorProtection != 0;
	info->copyright   = fr->copyright != 0;
	info->original    = fr->original != 0;
}



/******************************************************************************/
/* PlayFrame() initialize the first frame.                                    */
/*                                                                            */
/* Input:  "mp" is a pointer the the mpeg structure to use.                   */
/*         "fr" is a pointer to the frame structure to use.                   */
/******************************************************************************/
void MPG123Player::PlayFrame(MPStr *mp, Frame *fr)
{
	if (fr->errorProtection)
	{
		// Skip CRC, we are byte aligned here
		GetByte(&bsi);
		GetByte(&bsi);
	}

	// Do the decoding
	switch (fr->lay)
	{
		case 1:
		{
			DoLayer1(mp, fr);
			break;
		}

		case 2:
		{
			DoLayer2(mp, fr);
			break;
		}

		case 3:
		{
			DoLayer3(mp, fr);
			break;
		}
	}
}
