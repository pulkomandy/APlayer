/******************************************************************************/
/* MediaFile Player Interface.                                                */
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

#include <storage/Entry.h>

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

#include "MediaFile.h"
#include "ResourceIDs.h"

/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		1.0f



/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define AUDIOBUFSIZE		16384



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MediaFile::MediaFile(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	chanBuffers = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MediaFile::~MediaFile(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float MediaFile::GetVersion(void)
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
uint32 MediaFile::GetSupportFlags(int32 /*index*/)
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
PString MediaFile::GetName(int32 /*index*/)
{
	PString name;

	name.LoadString(res, IDS_MEDIAFILE_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MediaFile::GetDescription(int32 /*index*/)
{
	PString description;

	description.LoadString(res, IDS_MEDIAFILE_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString MediaFile::GetModTypeString(int32 /*index*/)
{
	PString type;

	type.LoadString(res, IDS_MEDIAFILE_MIME);
	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's a MediaFile module.          */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result MediaFile::ModuleCheck(int32 /*index*/, PFile * file)
{
	BEntry entry(file->GetFullPath().GetString());
	entry_ref ref;
	entry.GetRef(&ref);
	BMediaFile mediaFile(&ref);
	if (mediaFile.InitCheck() == B_OK)
		return AP_OK;
	else
		return AP_UNKNOWN;
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
ap_result MediaFile::LoadModule(int32 /*index*/, PFile *file, PString &/*errorStr*/)
{
	BEntry entry(file->GetFullPath().GetString());
	entry_ref ref;
	entry.GetRef(&ref);
	fFile = new BMediaFile(&ref);
	if (fFile->InitCheck() == B_OK)
		return AP_OK;
	else
		return AP_ERROR;
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MediaFile::InitPlayer(int32 /*index*/)
{
	int32 i;

	// Initialize and open the file
	fTrack = fFile->TrackAt(0); // TODO index ?

	media_format format;
	fTrack->DecodedFormat(&format);

	// Get the number of channels used and the output frequency
	channels = format.u.encoded_audio.output.channel_count;
	rate     = (int32)format.u.encoded_audio.output.frame_rate;
	totalTime = (int32)(fTrack->Duration() / 1000000);
	bitrate   = (int32)format.u.encoded_audio.bit_rate;


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
		// TODO is it possible to get these from the Media Kit ?
#if 0
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
#endif
	}

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void MediaFile::EndPlayer(int32 /*index*/)
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
	delete fFile;
}



/******************************************************************************/
/* InitSound() initialize the module to play.                                 */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void MediaFile::InitSound(int32 /*index*/, uint16 /*songNum*/)
{
	eof    = false;
	oldPos = 0;

	// Reset the file position to the start of the song
	SetSongPosition(0);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void MediaFile::Play(void)
{
	APChannel *channel;
	int16 pos;
	int32 i, j;
	int32 pcm[AUDIOBUFSIZE];
	int32 returned;
	int64 samplesReturned = AUDIOBUFSIZE;

	// Check for end-of-file
	if (eof)
	{
		// Seek back to the start of the file
		SetSongPosition(0);
		endReached = true;
		return;
	}

	// Get number of channels
#if 0
	chan = OV_Info(&vorbisFile, -1)->channels;
	if (chan != channels)
	{
		endReached = true;
		return;
	}
#endif

	// Have we changed position?
	pos = GetSongPosition();
	if (pos != oldPos)
	{
		oldPos = pos;
		ChangePosition();
	}

	// TODO
#if 0
	newRate = OV_BitrateInstant(&vorbisFile) / 1000;
	if ((newRate > 0) && (newRate != bitrate))
	{
		bitrate = newRate;
		ChangeModuleInfo(7, PString::CreateNumber(newRate));
	}
#endif

	// Read some part from the file and convert it
	returned = fTrack->ReadFrames(&pcm, &samplesReturned);
	if (returned != 0)
	{
		eof = true;
	}
	else
	{
		// The Media Kit provides us with samples in this order :
		// C1 C2
		// C1 C2
		// C1 C2
		// ...
		//
		// While APlayer wants:
		// C1 C1 C1 C1 ...
		// C2 C2 C2 C2 ...
		//
		// So we have to flip the array over
		
		int16* source = (int16*)pcm;

		for (j = 0; j < samplesReturned; j++)
		{
			for(i = 0; i < channels; i++)
			{
				*(chanBuffers[i] + j) = *source++;
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
void MediaFile::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = rate;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString MediaFile::GetModuleName(void)
{
	return (title);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString MediaFile::GetAuthor(void)
{
	return (artist);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 MediaFile::GetModuleChannels(void)
{
	return (channels);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 MediaFile::GetSongLength(void)
{
	return (totalTime);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 MediaFile::GetSongPosition(void)
{
	return (fTrack->CurrentTime() / 1000000);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void MediaFile::SetSongPosition(int16 pos)
{
	// Calculate the new position
	eof     = false;
	bigtime_t newPos = pos * 1000000;
	fTrack->SeekToTime(&newPos);
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
PTimeSpan MediaFile::GetTimeTable(uint16 /*songNum*/, PList<PTimeSpan> &posTimes)
{
	int32 i;

	// Copy the position times
	for (i = 0; i < 100; i++)
		posTimes.AddTail((totalTime * i) * 10);

	return (totalTime * 1000);
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
bool MediaFile::GetInfoString(uint32 line, PString &description, PString &value)
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
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE0);

			if (trackNumber == 0)
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value.SetUNumber(trackNumber);

			break;
		}

		// Album
		case 1:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE1);

			if (album.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value = album;

			break;
		}

		// Genre
		case 2:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE2);

			if (genre.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value = genre;

			break;
		}

		// Organization
		case 3:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE3);

			if (organization.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value = organization;

			break;
		}

		// Copyright
		case 4:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE4);

			if (copyright.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value = copyright;

			break;
		}

		// Description
		case 5:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE5);

			if (this->description.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_NONE);
			else
				value = this->description;

			break;
		}

		// Vendor
		case 6:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE6);

			if (vendor.IsEmpty())
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value = vendor;

			break;
		}

		// Bitrate
		case 7:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE7);

			if (bitrate == -1)
				value.LoadString(res, IDS_MEDIAFILE_INFO_UNKNOWN);
			else
				value.SetUNumber(bitrate);

			break;
		}

		// Frequency
		case 8:
		{
			description.LoadString(res, IDS_MEDIAFILE_INFODESCLINE8);
			value.SetUNumber(rate);
			break;
		}
	}

	return (true);
}
