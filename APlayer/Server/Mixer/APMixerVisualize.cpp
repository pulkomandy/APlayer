/******************************************************************************/
/* APlayer mixer visualize class.                                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PSynchronize.h"
#include "PThread.h"

// APlayerKit headers
#include "APAddOns.h"

// Server headers
#include "APApplication.h"
#include "APMixerVisualize.h"


/******************************************************************************/
/* Some defines                                                               */
/******************************************************************************/
#define MAX_VISUALIZE_BUFFER_SIZE				2048



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APMixerVisualize::APMixerVisualize(void)
{
	// Initialize member variables
	channelInfo.channelFlags = NULL;
	channelChangedEvent      = NULL;

	buffer[0]                = NULL;
	buffer[1]                = NULL;
	bufferFilledEvent        = NULL;
	exitEvent                = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APMixerVisualize::~APMixerVisualize(void)
{
}



/******************************************************************************/
/* Initialize() will initialize itself.                                       */
/*                                                                            */
/* Input:  "chanNum" is the number of channels the module uses.               */
/*         "channels" is a pointer to an array containing the channel objects.*/
/*         "maxBufferLen" is the maximum length of samples we can receive.    */
/*         "stereo" indicate if the sound is in stereo or mono.               */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APMixerVisualize::Initialize(uint16 chanNum, const APChannel **channels, int32 maxBufferLen, bool stereo)
{
	uint16 i;

	// Fill out the channel info structure
	channelInfo.channels    = chanNum;
	channelInfo.channelInfo = channels;

	channelInfo.channelFlags = new uint32[chanNum];
	if (channelInfo.channelFlags == NULL)
		return (false);

	for (i = 0; i < chanNum; i++)
		channelInfo.channelFlags[i] = 0;

	// Create the trigger event
	channelChangedEvent = new PEvent("Visualize trigger #1", false, false);
	if (channelChangedEvent == NULL)
		return (false);

	// Remember the arguments
	useStereo = stereo;

	// Well, we set a limit of the buffer
	bufferLen = min(maxBufferLen, MAX_VISUALIZE_BUFFER_SIZE);

	// Now allocate the two buffers
	buffer[0] = new int16[bufferLen];
	if (buffer[0] == NULL)
		return (false);

	buffer[1] = new int16[bufferLen];
	if (buffer[1] == NULL)
		return (false);

	bufferFilledEvent = new PEvent("Visualize trigger #2", false, false);
	if (bufferFilledEvent == NULL)
		return (false);

	currentBuffer = 0;

	// Create the exit event used in the thread
	exitEvent = new PEvent(true, false);
	if (exitEvent == NULL)
		return (false);

	// Initialize the thread and start it
	thread.SetName("Visualize");
	thread.SetHookFunc(VisualizeThread, this);
	thread.StartThread();

	return (true);
}



/******************************************************************************/
/* Cleanup() will cleanup again.                                              */
/******************************************************************************/
void APMixerVisualize::Cleanup(void)
{
	// Tell the thread to exit
	if (exitEvent != NULL)
	{
		exitEvent->SetEvent();
		thread.WaitOnThread();

		delete exitEvent;
		exitEvent = NULL;
	}

	// Free the trigger events
	delete bufferFilledEvent;
	bufferFilledEvent = NULL;

	delete channelChangedEvent;
	channelChangedEvent = NULL;

	// Deallocate the buffers
	delete[] buffer[1];
	delete[] buffer[0];
	buffer[1] = NULL;
	buffer[0] = NULL;

	// Deallocate the flags
	delete[] channelInfo.channelFlags;
	channelInfo.channelFlags = NULL;
}



/******************************************************************************/
/* GetFlagsArray() will return a pointer to the array where a backup of the   */
/*      channel flags should be stored.                                       */
/*                                                                            */
/* Output: A pointer to the channel flags array.                              */
/******************************************************************************/
uint32 *APMixerVisualize::GetFlagsArray(void) const
{
	return (channelInfo.channelFlags);
}



/******************************************************************************/
/* TellAgents_MixedData() will call all the visual agents and tell them about */
/*      the new mixed data.                                                   */
/*                                                                            */
/* Input:  "source" is a pointer to the buffer with the samples.              */
/*         "size" is the size of the buffer in number of samples.             */
/******************************************************************************/
void APMixerVisualize::TellAgents_MixedData(int16 *source, int32 size)
{
	int32 bufToUse;
	int32 todo;

	// Lock the pointer lock and get the buffer to use
	bufToUse = currentBuffer + 1;
	if (bufToUse > 1)
		bufToUse = 0;

	// Copy the sample data into the buffer
	todo = min(size, bufferLen);
	memcpy(buffer[bufToUse], source, todo * sizeof(int16));
	if (todo < bufferLen)
		memset(buffer[bufToUse] + todo, 0, (bufferLen - todo) * sizeof(int16));

	currentBuffer = bufToUse;

	// Tell the thread that there is something to view
	bufferFilledEvent->SetEvent();
}



/******************************************************************************/
/* TellAgents_ChannelChange() will call all the visual agents and tell them   */
/*      about channel changes.                                                */
/******************************************************************************/
void APMixerVisualize::TellAgents_ChannelChanged(void)
{
	// Tell the thread that a channel has changed its status
	channelChangedEvent->SetEvent();
}



/******************************************************************************/
/* VisualizeThread() is the main visualize thread, that will communicate with */
/*      the agents.                                                           */
/*                                                                            */
/* Input:  "userData" is a pointer to the current object.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 APMixerVisualize::VisualizeThread(void *userData)
{
	APMixerVisualize *obj = (APMixerVisualize *)userData;
	PSync *waitObjects[3];
	AddOnInfo *info;
	int32 triggedObject;
	int32 i, count;

	// Initialize the synchonize objects to wait for
	waitObjects[0] = obj->exitEvent;
	waitObjects[1] = obj->channelChangedEvent;
	waitObjects[2] = obj->bufferFilledEvent;

	// Wait for something to happend
	do
	{
		triggedObject = MultipleObjectsWait(waitObjects, 3, false);

		switch (triggedObject)
		{
			//
			// Channel changed
			//
			case 1:
			{
				// Lock the plug-in list
				GetApp()->pluginLock.WaitToRead();

				// Get the number of visual agents
				count = GetApp()->visualAgents.CountItems();

				// Call each agent
				for (i = 0; i < count; i++)
				{
					info = GetApp()->visualAgents.GetItem(i);
					info->agent->Run(info->index, APVA_CHANNEL_CHANGE, &obj->channelInfo);
				}

				GetApp()->pluginLock.DoneReading();
				break;
			}

			//
			// Buffer filled
			//
			case 2:
			{
				APAgent_MixedData mixedData;
				int32 bufToUse;

				// Get the pointer to use
				bufToUse = obj->currentBuffer;

				// Fill out the structure
				mixedData.buffer = obj->buffer[bufToUse];
				mixedData.length = obj->bufferLen;
				mixedData.stereo = obj->useStereo;

				if (obj->useStereo)
					mixedData.length /= 2;

				// Lock the plug-in list
				GetApp()->pluginLock.WaitToRead();

				// Get the number of visual agents
				count = GetApp()->visualAgents.CountItems();

				// Call each agent
				for (i = 0; i < count; i++)
				{
					info = GetApp()->visualAgents.GetItem(i);
					info->agent->Run(info->index, APVA_MIXED_DATA, &mixedData);
				}

				GetApp()->pluginLock.DoneReading();
				break;
			}
		}
	}
	while (triggedObject != 0);

	return (0);
}
