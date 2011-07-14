/******************************************************************************/
/* Sample Player Interface.                                                   */
/*                                                                            */
/* Player by Thomas Neumann.                                                  */
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
#include "APList.h"

// Player headers
#include "Sample.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion		2.02f



/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define BUFFER_SIZE			8192



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Sample::Sample(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	APAddOnInformation *info;
	int32 i, count;

	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	converter           = NULL;
	sampBuffer          = NULL;
	chanBuffers         = NULL;
	sampFormat.channels = 0;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();

	// Get list of all the converters
	globalData->GetAddOnList(apConverter, convInfo);

	// Now extract the loaders only
	count = convInfo.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the converter information
		info = convInfo.GetItem(i);

		// Is the converter a loader?
		if (info->pluginFlags & apcLoader)
		{
			// It is, add the index to the loader list
			convLoaders.AddTail(i);
		}
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Sample::~Sample(void)
{
	// Delete converter lists
	convLoaders.MakeEmpty();
	globalData->FreeAddOnList(convInfo);

	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float Sample::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetCount() returns the number of add-ons in the library.                   */
/*                                                                            */
/* Output: The number of add-ons.                                             */
/******************************************************************************/
int32 Sample::GetCount(void)
{
	// Return the number of converters
	return (convLoaders.CountItems());
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 Sample::GetSupportFlags(int32 index)
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
PString Sample::GetName(int32 index)
{
	int32 convNum;

	convNum = convLoaders.GetItem(index);
	return (convInfo.GetItem(convNum)->name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString Sample::GetDescription(int32 index)
{
	int32 convNum;

	convNum = convLoaders.GetItem(index);
	return (convInfo.GetItem(convNum)->description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString Sample::GetModTypeString(int32 index)
{
	int32 convNum, convIndex;
	APAddOnConverter *conv;
	PString type;

	// Get the converter name and create an instance of it
	convNum = convLoaders.GetItem(index);
	conv    = (APAddOnConverter *)globalData->GetAddOnInstance(convInfo.GetItem(convNum)->name, apConverter, &convIndex);
	if (conv == NULL)
		return ("");

	// Get the description
	type = conv->GetTypeString(convIndex);

	// And finally, destroy the instance again
	globalData->DeleteAddOnInstance(convInfo.GetItem(convNum)->name, apConverter, conv);

	return (type);
}



/******************************************************************************/
/* ModuleCheck() tests the module to see if it's one of the supported sample  */
/*      formats.                                                              */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "file" is a pointer to a PFile object with the file to check.      */
/*                                                                            */
/* Output: An APlayer result code.                                            */
/******************************************************************************/
ap_result Sample::ModuleCheck(int32 index, PFile *file)
{
	int32 convNum;
	ap_result result;

	// Create an instance of the converter
	convNum   = convLoaders.GetItem(index);
	converter = (APAddOnConverter *)globalData->GetAddOnInstance(convInfo.GetItem(convNum)->name, apConverter, &converterIndex);
	if (converter == NULL)
		return (AP_UNKNOWN);

	// Call the test function
	result = converter->FileCheck(converterIndex, file);

	// Did we found the converter?
	if (result != AP_OK)
	{
		// Destroy the instance again
		globalData->DeleteAddOnInstance(convInfo.GetItem(convNum)->name, apConverter, converter);
		converter = NULL;
	}

	return (result);
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
ap_result Sample::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	// Start to initialize the converter
	if (!converter->LoaderInit(converterIndex))
		return (AP_ERROR);

	// Load the header
	if (converter->LoadHeader(file, &sampFormat, errorStr) != AP_OK)
		return (AP_ERROR);

	// Make a copy of the file object
	sampFile = file;

	return (AP_OK);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool Sample::InitPlayer(int32 index)
{
	uint16 i;

	// Allocate source buffer
	sampBuffer = new float[BUFFER_SIZE];
	if (sampBuffer == NULL)
		throw PMemoryException();

	// Allocate array to hold channel buffers
	chanBuffers = new int16 *[sampFormat.channels];
	if (chanBuffers == NULL)
		throw PMemoryException();

	// Clear the pointers in case of an error and we need to delete all
	// the buffers
	for (i = 0; i < sampFormat.channels; i++)
		chanBuffers[i] = NULL;

	// Now allocate each buffer
	for (i = 0; i < sampFormat.channels; i++)
	{
		chanBuffers[i] = new int16[BUFFER_SIZE / sampFormat.channels];
		if (chanBuffers[i] == NULL)
			throw PMemoryException();
	}

	// The number of samples of the file
	totalLength = converter->GetTotalSampleLength(&sampFormat);

	return (true);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void Sample::EndPlayer(int32 index)
{
	uint16 i;
	int32 convNum;

	// Cleanup converter data
	converter->LoaderEnd(converterIndex);

	// Destroy the instance
	convNum = convLoaders.GetItem(index);
	globalData->DeleteAddOnInstance(convInfo.GetItem(convNum)->name, apConverter, converter);
	converter = NULL;

	// Delete the buffers
	delete[] sampBuffer;
	sampBuffer = NULL;

	for (i = 0; i < sampFormat.channels; i++)
		delete[] chanBuffers[i];

	delete[] chanBuffers;
	chanBuffers = NULL;
	sampFormat.channels = 0;
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void Sample::InitSound(int32 index, uint16 songNum)
{
	// Reset the sample position
	converter->SetSamplePosition(sampFile, 0, &sampFormat);

	samplesRead = 0;
	oldPos      = 0;
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void Sample::Play(void)
{
	uint32 filled, offset;

	// Load the next block of data
	filled       = converter->LoadData(sampFile, sampBuffer, BUFFER_SIZE, &sampFormat);
	samplesRead += filled;

	if (filled != 0)
	{
		// Convert the buffer to APlayer channel buffers
		if (sampFormat.channels == 1)
		{
			// Mono samples
			float *source = sampBuffer;
			int16 *dest   = chanBuffers[0];

			for (offset = 0; offset < filled; offset++)
				*dest++ = (int16)(*source++ * 32768.0f);

			// Tell APlayer what to play
			virtChannels[0]->SetBuffer(chanBuffers[0], filled);
			virtChannels[0]->SetVolume(sampFormat.volume);

			if (sampFormat.panning != -1)
				virtChannels[0]->SetPanning(sampFormat.panning);
			else
				virtChannels[0]->SetPanning(APPAN_CENTER);
		}
		else
		{
			if (sampFormat.channels == 2)
			{
				// Stereo samples
				float *source = sampBuffer;
				int16 *dest1  = chanBuffers[0];
				int16 *dest2  = chanBuffers[1];

				for (offset = 0; offset < filled; offset += 2)
				{
					*dest1++ = (int16)(*source++ * 32768.0f);
					*dest2++ = (int16)(*source++ * 32768.0f);
				}

				// Tell APlayer what to play
				virtChannels[0]->SetBuffer(chanBuffers[0], filled / 2);
				virtChannels[0]->SetVolume(sampFormat.volume);
				virtChannels[1]->SetBuffer(chanBuffers[1], filled / 2);
				virtChannels[1]->SetVolume(sampFormat.volume);

				if (sampFormat.panning != -1)
				{
					virtChannels[0]->SetPanning(sampFormat.panning);
					virtChannels[1]->SetPanning(sampFormat.panning);
				}
				else
				{
					virtChannels[0]->SetPanning(APPAN_LEFT);
					virtChannels[1]->SetPanning(APPAN_RIGHT);
				}
			}
		}
	}
	else
	{
		endReached = true;

		// Loop the sample
		if (sampFormat.flags & APSAMP_LOOP)
		{
			converter->SetSamplePosition(sampFile, sampFormat.loopStart, &sampFormat);
			samplesRead = sampFormat.loopStart;
		}
		else
		{
			converter->SetSamplePosition(sampFile, 0, &sampFormat);
			samplesRead = 0;
		}
	}

	// Tell APlayer we have changed the position
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
void Sample::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = sampFormat.frequency;
}



/******************************************************************************/
/* GetModuleName() returns the name of the sample.                            */
/*                                                                            */
/* Output: Is the sample name.                                                */
/******************************************************************************/
PString Sample::GetModuleName(void)
{
	return (sampFormat.name);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString Sample::GetAuthor(void)
{
	return (sampFormat.author);
}



/******************************************************************************/
/* GetModuleChannels() returns the number of channels the module use.         */
/*                                                                            */
/* Output: Is the number of channels.                                         */
/******************************************************************************/
uint16 Sample::GetModuleChannels(void)
{
	return (sampFormat.channels);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 Sample::GetSongLength(void)
{
	return (100);
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 Sample::GetSongPosition(void)
{
	int16 pos;

	pos = (int64)samplesRead * 100 / totalLength;
	if (pos >= 100)
		pos = 99;

	return (pos);
}



/******************************************************************************/
/* SetSongPosition() sets a new position in the song.                         */
/*                                                                            */
/* Input:  "pos" is the new song position.                                    */
/******************************************************************************/
void Sample::SetSongPosition(int16 pos)
{
	int32 newPos;

	// Change the position
	newPos = (int64)pos * totalLength / 100;
	samplesRead = converter->SetSamplePosition(sampFile, newPos, &sampFormat);
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
PTimeSpan Sample::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	int32 i;
	int64 totalTime;

	// Calculate the total time
	totalTime = ((int64)totalLength) * 1000 / sampFormat.frequency / sampFormat.channels;

	// Now build the list
	for (i = 0; i < 100; i++)
		posTimes.AddTail(i * (totalTime / 100));

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
bool Sample::GetInfoString(uint32 line, PString &description, PString &value)
{
	// Find out which line to take
	switch (line)
	{
		// Bit size
		case 0:
		{
			description.LoadString(res, IDS_SAMPLE_INFODESCLINE0);
			value.SetUNumber(sampFormat.bitSize);
			break;
		}

		// Frequency
		case 1:
		{
			description.LoadString(res, IDS_SAMPLE_INFODESCLINE1);
			value.SetUNumber(sampFormat.frequency);
			break;
		}

		// Looping
		case 2:
		{
			description.LoadString(res, IDS_SAMPLE_INFODESCLINE2);
			value.LoadString(res, sampFormat.flags & APSAMP_LOOP ? IDS_SAMPLE_YES : IDS_SAMPLE_NO);
			break;
		}

		// Converter information
		default:
		{
			return (converter->GetInfoString(line - 3, description, value));
		}
	}

	return (true);
}
