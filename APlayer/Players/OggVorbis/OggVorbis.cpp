/******************************************************************************/
/* OggVorbis Player Interface.                                                */
/*                                                                            */
/* Original player by Xiphophorus.                                            */
/* APlayer plug-in by Thomas Neumann.                                         */
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

// OggVorbis headers
#include "Ogg.h"
#include "Codec.h"

// Player headers
#include "OggVorbis.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.02f



/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define AUDIOBUFSIZE		16384



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
OggVorbis::OggVorbis(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	chanBuffers = NULL;

	// Initialize OggVorbis structures
	memset(&vorbisFile, 0, sizeof(vorbisFile));
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
OggVorbis::~OggVorbis(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float OggVorbis::GetVersion(void)
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
uint32 OggVorbis::GetSupportFlags(int32 index)
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
PString OggVorbis::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_OGG_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString OggVorbis::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_OGG_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString OggVorbis::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_OGG_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a OggVorbis module.          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result OggVorbis::ModuleCheck(int32 index, PFile *file)
{
	// Check the module size
	if (file->GetLength() < 27)
		return (AP_UNKNOWN);

	// Check the mark
	file->SeekToBegin();
	if (file->Read_B_UINT32() != 'OggS')
		return (AP_UNKNOWN);

	// Check the stream structure version
	if (file->Read_UINT8() != 0x00)
		return (AP_UNKNOWN);

	// Check the header type flag
	if ((file->Read_UINT8() & 0xf8) != 0x00)
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
ap_result OggVorbis::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	// Well, there is nothing to load, because the file will be loaded
	// a little bit of the time when playing. Just make a copy of the
	// file pointer
	oggFile = file;

	return (AP_OK);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool OggVorbis::InitPlayer(int32 index)
{
	VorbisInfo *info;
	int32 i;

	// Initialize and open the file
	oggFile->SeekToBegin();
	if (OV_Open(oggFile, &vorbisFile, NULL, 0) < 0)
		return (false);

	// Get the number of channels used and the output frequency
	info     = OV_Info(&vorbisFile, 0);
	channels = info->channels;
	rate     = info->rate;

	// Allocate converter buffers
	chanBuffers = new int16 *[channels];
	if (chanBuffers == NULL)
		throw PMemoryException();

	memset(chanBuffers, 0, sizeof(int16 *) * channels);

	for (i = 0; i < channels; i++)
	{
		chanBuffers[i] = new int16[AUDIOBUFSIZE];
		if (chanBuffers[i] == NULL)
			throw PMemoryException();
	}

	{
		VorbisComment *comment;
		PString commentStr, tempStr;
		PCharSet_UTF8 charSet;
		int32 index;

		// Get the comment information
		comment = OV_Comment(&vorbisFile, 0);

		// Get the vendor information
		vendor.SetString(comment->vendor, &charSet);

		// Scan the comments after useful information
		trackNumber = 0;

		for (i = 0; i < comment->comments; i++)
		{
			commentStr.SetString(comment->userComments[i], &charSet);
			index = commentStr.Find('=');
			if (index != -1)
			{
				tempStr = commentStr.Left(index);
				if (tempStr.CompareNoCase("TITLE") == 0)
					title += ", " + commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("ALBUM") == 0)
					album += ", " + commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("TRACKNUMBER") == 0)
					trackNumber = commentStr.GetUNumber(index + 1);
				else if (tempStr.CompareNoCase("ARTIST") == 0)
					artist += ", " + commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("ORGANIZATION") == 0)
					organization += ", " + commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("DESCRIPTION") == 0)
					description = commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("GENRE") == 0)
					genre += ", " + commentStr.Mid(index + 1);
				else if (tempStr.CompareNoCase("COPYRIGHT") == 0)
					copyright += ", " + commentStr.Mid(index + 1);
			}
		}

		// Remove the first part of some of the strings
		title.Delete(0, 2);
		album.Delete(0, 2);
		artist.Delete(0, 2);
		organization.Delete(0, 2);
		genre.Delete(0, 2);
		copyright.Delete(0, 2);
	}

	// Now get the stream information
	totalTime = (int32)(OV_Time_Total(&vorbisFile, -1) * 1000.0);
	bitrate   = OV_Bitrate(&vorbisFile, -1) / 1000;

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void OggVorbis::EndPlayer(int32 index)
{
	int32 i;

	// Cleanup the channel buffers
	if (chanBuffers != NULL)
	{
		for (i = 0; i < channels; i++)
			delete[] chanBuffers[i];

		delete[] chanBuffers;
		chanBuffers = NULL;
	}

	// Cleanup the structures
	OV_Clear(&vorbisFile);
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void OggVorbis::InitSound(int32 index, uint16 songNum)
{
	eof    = false;
	oldPos = 0;

	// Reset the file position to the start of the song
	SetSongPosition(0);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void OggVorbis::Play(void)
{
	APChannel *channel;
	int16 pos;
	int32 newRate;
	int32 chan, i, j;
	float **pcm;
	int32 returned;
	int32 samplesReturned = 0;

	// Check for end-of-file
	if (eof)
	{
		// Seek back to the start of the file
		SetSongPosition(0);
		endReached = true;
		return;
	}

	// Get number of channels
	chan = OV_Info(&vorbisFile, -1)->channels;
	if (chan != channels)
	{
		endReached = true;
		return;
	}

	// Have we changed position?
	pos = GetSongPosition();
	if (pos != oldPos)
	{
		oldPos = pos;
		ChangePosition();
	}

	newRate = OV_BitrateInstant(&vorbisFile) / 1000;
	if ((newRate > 0) && (newRate != bitrate))
	{
		bitrate = newRate;
		ChangeModuleInfo(7, PString::CreateNumber(newRate));
	}

	// Read some part from the file and convert it
	while (samplesReturned < AUDIOBUFSIZE)
	{
		returned = OV_Read(&vorbisFile, &pcm, AUDIOBUFSIZE - samplesReturned, NULL);
		if (returned == 0)
		{
			eof = true;
			break;
		}
		else
		{
			if (returned > 0)
			{
				// Convert the PCM from float to 16-bit
				for (i = 0; i < channels; i++)
				{
					int16 *dest   = chanBuffers[i] + samplesReturned;
					float *source = pcm[i];

					for (j = 0; j < returned; j++)
					{
						int32 val = (int32)(*source++ * 32768.0f);

						if (val > 32767)
							val = 32767;

						if (val < -32768)
							val = -32768;

						*dest++ = val;
					}
				}

				samplesReturned += returned;
			}
		}
	}

	if (samplesReturned > 0)
	{
		// Tell APlayer what to play
		if (channels == 1)
		{
			// Play the sample in mono
			channel = virtChannels[0];
			channel->SetBuffer(chanBuffers[0], samplesReturned);
			channel->SetVolume(256);
			channel->SetPanning(APPAN_CENTER);
		}
		else
		{
			// Well, we need to play the sample in more than 1 channel
			for (i = 0; i < channels; i++)
			{
				channel = virtChannels[i];
				channel->SetBuffer(chanBuffers[i], samplesReturned);
				channel->SetVolume(256);
				channel->SetPanning(i % 2 ? APPAN_RIGHT : APPAN_LEFT);
			}
		}
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void OggVorbis::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = rate;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString OggVorbis::GetModuleName(void)
{
	return (title);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString OggVorbis::GetAuthor(void)
{
	return (artist);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 OggVorbis::GetModuleChannels(void)
{
	return (channels);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 OggVorbis::GetSongLength(void)
{
	return (100);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 OggVorbis::GetSongPosition(void)
{
	int16 newPos;

	newPos = (int16)((OV_Time_Tell(&vorbisFile) * 1000.0) * 100 / totalTime);
	if (newPos > 99)
		newPos = 99;

	return (newPos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void OggVorbis::SetSongPosition(int16 pos)
{
	double newTime;

	// Calculate the new position
	newTime = (totalTime * pos) / 100.0;
	eof     = false;

	OV_TimeSeek(&vorbisFile, newTime / 1000.0);
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
PTimeSpan OggVorbis::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	int32 i;

	// Copy the position times
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
bool OggVorbis::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Is the line number out of range?
	if (line >= 9)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Track Number
		case 0:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE0);

			if (trackNumber == 0)
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value.SetUNumber(trackNumber);

			break;
		}

		// Album
		case 1:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE1);

			if (album.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value = album;

			break;
		}

		// Genre
		case 2:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE2);

			if (genre.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value = genre;

			break;
		}

		// Organization
		case 3:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE3);

			if (organization.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value = organization;

			break;
		}

		// Copyright
		case 4:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE4);

			if (copyright.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value = copyright;

			break;
		}

		// Description
		case 5:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE5);

			if (this->description.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_NONE);
			else
				value = this->description;

			break;
		}

		// Vendor
		case 6:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE6);

			if (vendor.IsEmpty())
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value = vendor;

			break;
		}

		// Bitrate
		case 7:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE7);

			if (bitrate == -1)
				value.LoadString(res, IDS_OGG_INFO_UNKNOWN);
			else
				value.SetUNumber(bitrate);

			break;
		}

		// Frequency
		case 8:
		{
			description.LoadString(res, IDS_OGG_INFODESCLINE8);
			value.SetUNumber(rate);
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* Allocation functions.                                                      */
/******************************************************************************/
void *OggVorbis::_ogg_calloc(int32 count, int32 bytes)
{
	void *buffer = _ogg_malloc(bytes * count);
	memset(buffer, 0, bytes * count);

	return (buffer);
}



void *OggVorbis::_ogg_malloc(int32 bytes)
{
	uint8 *buffer = new uint8[4 + bytes];
	*((int32 *)buffer) = bytes;

	return (buffer + 4);
}



void *OggVorbis::_ogg_realloc(void *oldData, int32 newSize)
{
	void *newData = _ogg_malloc(newSize);
	memcpy(newData, oldData, *(((int32 *)oldData) - 1));
	_ogg_free(oldData);

	return (newData);
}



void OggVorbis::_ogg_free(void *data)
{
	if (data != NULL)
		delete[] ((uint8 *)data - 4);
}



/******************************************************************************/
/* Common functions                                                           */
/******************************************************************************/
/* ilog2()                                                                    */
/******************************************************************************/
int32 OggVorbis::ilog2(uint32 v)
{
	int32 ret = 0;

	if (v)
		--v;

	while (v)
	{
		ret++;
		v >>= 1;
	}

	return (ret);
}



/******************************************************************************/
/* ilog()                                                                     */
/******************************************************************************/
int32 OggVorbis::ilog(uint32 v)
{
	int32 ret = 0;

	while (v)
	{
		ret++;
		v >>= 1;
	}

	return (ret);
}



/******************************************************************************/
/* icount()                                                                   */
/******************************************************************************/
int32 OggVorbis::icount(uint32 v)
{
	int32 ret = 0;

	while (v)
	{
		ret += v & 1;
		v >>= 1;
	}

	return (ret);
}
