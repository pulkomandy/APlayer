/******************************************************************************/
/* APlayer mixer class.                                                       */
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
#include "PThread.h"
#include "PSynchronize.h"
#include "PSystem.h"

// APlayerKit headers
#include "APAddOns.h"

// Server headers
#include "APApplication.h"
#include "APClientCommunication.h"
#include "APPlayer.h"
#include "APMixer.h"
#include "APMixerBase.h"
#include "APMixerNormal.h"
#include "APMixerVisualize.h"
#include "APChannelParser.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APMixer::APMixer(void) : mixerLock(false)
{
	uint16 i;

	// Initialize member variables
	playerLock        = NULL;
	playerInfo        = NULL;

	currentPlayer     = NULL;
	currentMixer      = NULL;
	currentVisualizer = NULL;
	soundOutputInfo   = NULL;
	soundOutput       = NULL;
	mixBuffer         = NULL;

	playing           = false;
	holdPlaying       = true;
	samplePlay        = false;
	emulateFilter     = false;

	// Initialize ring buffer variables
	useRingBuffer     = false;
	exitEvent         = NULL;
	fillBuffer        = NULL;
	newPosSignal      = NULL;
	readySignal       = NULL;

	// Initialize the enabled channels array
	for (i = 0; i < MAX_NUM_CHANNELS; i++)
		channelsEnabled[i] = true;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APMixer::~APMixer(void)
{
}



/******************************************************************************/
/* InitMixer() initialize the mixing routines. If the function fails, make    */
/*      sure to call the EndMixer() to clean up.                              */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "lock" is a pointer to the player lock.                            */
/*         "player" is a pointer to the player object that holds both the     */
/*         player information and the looper to send player specific          */
/*         messages to.                                                       */
/*         "result" is a reference to a string to store the error in if any.  */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APMixer::InitMixer(APFileHandle handle, PMutex *lock, APPlayer *player, PString &result)
{
	uint16 i;
	uint32 playerFlags;
	APSamplePlayerInfo samplePlayInfo;
	APAgent_OutputInfo outputInfo;
	int32 index;
	bool retVal = true;

	// Remember to player lock and looper
	playerLock = lock;
	playerInfo = player;

	// Set the mixer in Module mode
	samplePlay = false;
	playing    = false;

	// Get the player object + the player index
	currentPlayer = handle.loader->GetPlayer(index);

	// Get player informations
	modChannelNum = currentPlayer->GetVirtualChannels();

	// Remember the mixer settings
	mixerFreq = handle.mixerFrequency;

	// Initialize other member variables
	filterPrevLeft  = 0;
	filterPrevRight = 0;

	// Set the flag for the different mixer modes
	mixerMode = 0;

	if (handle.interpolation)
		mixerMode |= DMODE_INTERP;

	if (handle.dolbyPrologic)
		mixerMode |= DMODE_SURROUND;

	if (handle.amigaFilter)
		emulateFilter = true;

	// If there isn't set any output agents, don't initialize any
	if (!handle.outputAgent.IsEmpty())
	{
		// Lock the plug-ins
		GetApp()->pluginLock.WaitToRead();

		// Are there any sound output agents loaded?
		if (GetApp()->soundOutputAgents.IsEmpty())
		{
			GetApp()->pluginLock.DoneReading();

			result.LoadString(GetApp()->resource, IDS_CMDERR_NOOUTPUTAGENT);
			return (false);
		}

		// Does the Output Agent exists
		if (!GetApp()->soundOutputAgents.HasKey(handle.outputAgent))
		{
			GetApp()->pluginLock.DoneReading();

			result.Format_S1(GetApp()->resource, IDS_CMDERR_OUTPUTAGENT_NOTEXISTS, handle.outputAgent);
			return (false);
		}

		// Okay, get the agent information
		GetApp()->soundOutputAgents.GetItem(handle.outputAgent, soundOutputInfo);

		// Unlock the plug-in list
		GetApp()->pluginLock.DoneReading();

		// Create a new instance of the output agent
		soundOutput = (APAddOnAgent *)soundOutputInfo->loader->CreateInstance();
		if (soundOutput == NULL)
			throw PMemoryException();

		// Initialize the agent
		if (!soundOutput->InitAgent(soundOutputInfo->index))
		{
			// Failed to initialize
			soundOutputInfo->loader->DeleteInstance(soundOutput);
			soundOutput = NULL;
			return (false);
		}

		// Store the agent pointer in the add-on information
		soundOutputInfo->agent = soundOutput;
	}

	try
	{
		// Get the players support flags
		playerFlags = currentPlayer->GetSupportFlags(index);

		// See if the player is a sample player
		if (playerFlags & appSamplePlayer)
		{
			// It is, so call the GetSamplePlayerInfo() function
			currentPlayer->GetSamplePlayerInfo(&samplePlayInfo);
			samplePlay = true;

			// If the player is a 1 or 2 channel sample player,
			// we boost the samples a little bit more than normally
			if (modChannelNum <= 2)
				mixerMode |= DMODE_BOOST;
		}

		// Do the player need ring buffers?
		if (playerFlags & appUseRingBuffer)
			useRingBuffer = true;
		else
			useRingBuffer = false;

		// Allocate the visual component
		currentVisualizer = new APMixerVisualize();
		if (currentVisualizer == NULL)
			throw PMemoryException();

		try
		{
			// Allocate mixer to use
			currentMixer = new APMixerNormal();
			if (currentMixer == NULL)
				throw PMemoryException();

			try
			{
				if (soundOutput != NULL)
				{
					// Initialize the sound
					APAgent_InitHardware initHardware;

					initHardware.mixerFunc  = Mixer;
					initHardware.handle     = this;
					initHardware.frequency  = mixerFreq;
					initHardware.fileName   = handle.fileName;
					initHardware.moduleName = playerInfo->GetModuleName();
					initHardware.author     = playerInfo->GetAuthor();
					if (soundOutput->Run(soundOutputInfo->index, APOA_INIT_HARDWARE, &initHardware) != AP_OK)
					{
						result.LoadString(GetApp()->resource, IDS_CMDERR_SOUNDOUTPUT_INIT);
						throw PUserException();
					}

					// Get the output informations
					soundOutput->Run(soundOutputInfo->index, APOA_GET_OUTPUT_INFORMATION, &outputInfo);
				}
				else
				{
					// Fill out the output information with defaults
					outputInfo.channels   = 1;
					outputInfo.bufferSize = 2048;
				}

				if (outputInfo.channels == 2)
					mixerMode |= DMODE_STEREO;

				// Get the maximum number of samples the given destination
				// buffer from the sound driver can be
				if (useRingBuffer)
					bufferSize = RINGBUFFER_SIZE;
				else
					bufferSize = outputInfo.bufferSize;

				// Allocate mixer buffer. This buffer is used by the mixer
				// routines to store the mixed data in
				mixBuffer = new int32[bufferSize + 32];
				if (mixBuffer == NULL)
					throw PMemoryException();

				try
				{
					// Initialize the mixer
					if (!currentMixer->Initialize(mixerFreq, modChannelNum))
						throw PUserException();

					// Allocate channel objects
					currentPlayer->virtChannels = (APChannel **)new APChannelParser *[modChannelNum];
					if (currentPlayer->virtChannels == NULL)
						throw PMemoryException();

					for (i = 0; i < modChannelNum; i++)
					{
						currentPlayer->virtChannels[i] = new APChannelParser;
						if (currentPlayer->virtChannels[i] == NULL)
							throw PMemoryException();

						// If the player is a sample player, we need some extra informations
						if (samplePlay)
							((APChannelParser *)currentPlayer->virtChannels[i])->SetSampleInfo(&samplePlayInfo);
					}

					// Initialize the visualizer
					if (!currentVisualizer->Initialize(modChannelNum, (const APChannel **)currentPlayer->virtChannels, outputInfo.bufferSize, (mixerMode & DMODE_STEREO) != 0))
						throw PUserException();

					// Initialize extra virtual mixers
					InitVirtualMixer();

					// Initialize the ring buffer
					InitRingBuffer();

					// Initialize the mixers
					SetStereoSeparation(handle.stereoSeparator);
				}
				catch(...)
				{
					delete[] mixBuffer;
					mixBuffer = NULL;
					throw;
				}
			}
			catch(...)
			{
				delete currentMixer;
				currentMixer = NULL;
				throw;
			}
		}
		catch(...)
		{
			delete currentVisualizer;
			currentVisualizer = NULL;
			throw;
		}
	}
	catch(PSoundException e)
	{
		PString err;
		char *errStr;

		err = PSystem::GetErrorString(e.errorNum);
		result.Format(GetApp()->resource, IDS_CMDERR_SOUND, e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);

		retVal = false;
	}
	catch(PUserException e)
	{
		retVal = false;
	}

	return (retVal);
}



/******************************************************************************/
/* EndMixer() ends the mixing routines.                                       */
/******************************************************************************/
void APMixer::EndMixer(void)
{
	uint16 i;
	int32 j, count;
	AddOnInfo *info;

	if (soundOutput != NULL)
	{
		try
		{
			// Stop the sound hardware
			soundOutput->Run(soundOutputInfo->index, APOA_END_HARDWARE, NULL);
		}
		catch(PSoundException e)
		{
			;
		}
	}

	// Stop the ring buffer
	EndRingBuffer();

	// Cleanup virtual mixers
	EndVirtualMixer();

	// Stop the sound output agent and delete the instance
	if (soundOutput != NULL)
	{
		soundOutput->EndAgent(soundOutputInfo->index);
		soundOutputInfo->loader->DeleteInstance(soundOutput);
		soundOutputInfo->agent = NULL;
		soundOutputInfo        = NULL;
		soundOutput            = NULL;
	}

	// Deallocate mixer
	if (currentMixer != NULL)
	{
		currentMixer->Cleanup();
		delete currentMixer;
		currentMixer = NULL;
	}

	// Deallocate the visualizer
	if (currentVisualizer != NULL)
	{
		currentVisualizer->Cleanup();
		delete currentVisualizer;
		currentVisualizer = NULL;
	}

	// Deallocate the mixer buffer
	delete[] mixBuffer;
	mixBuffer = NULL;

	// Deallocate the channel objects
	if (currentPlayer != NULL)
	{
		if (currentPlayer->virtChannels != NULL)
		{
			for (i = 0; i < modChannelNum; i++)
				delete currentPlayer->virtChannels[i];
		}

		delete[] currentPlayer->virtChannels;
		currentPlayer->virtChannels = NULL;
	}

	// Tell all DSP agents that we have stopped playing
	GetApp()->pluginLock.WaitToRead();

	count = GetApp()->dspAgents.CountItems();
	for (j = 0; j < count; j++)
	{
		info = GetApp()->dspAgents.GetItem(j);
		info->agent->Run(info->index, APPA_STOP, NULL);
	}

	GetApp()->pluginLock.DoneReading();

	// Clean up member variables
	currentPlayer = NULL;
}



/******************************************************************************/
/* StartMixer() starts the mixing routines.                                   */
/******************************************************************************/
void APMixer::StartMixer(void)
{
	// Clear all the voices
	currentMixer->ClearVoices();

	// Initialize ticks left to call the player
	tickLeft = 0;

	if (useRingBuffer)
	{
		// Flush all the buffers
		ResetRingBuffer();

		// Start the ring buffer thread if it hasn't been started
		if (firstTime)
		{
			ringThread.StartThread();
			firstTime = false;
		}

		// Tell the thread to continue filling
		fillBuffer->SetEvent();
	}

	// Start the sound
	soundOutput->Run(soundOutputInfo->index, APOA_START_PLAYING, NULL);
}



/******************************************************************************/
/* StopMixer() stops the mixing routines.                                     */
/******************************************************************************/
void APMixer::StopMixer(void)
{
	AddOnInfo *info;
	int32 i, count;

	// Wait until all the ring buffer mutexes has been released
	if (useRingBuffer)
	{
		holdPlaying = true;
		while (allMutexesFree);	// It take a short time to get out of this loop, so don't worry about it

		// Tell the ring buffer to stop filling
		fillBuffer->ResetEvent();
		readySignal->SetEvent();
	}

	// Stop the sound
	soundOutput->Run(soundOutputInfo->index, APOA_STOP_PLAYING, NULL);

	// Tell the visual agents to clear their views
	//
	// Lock the plug-in list
	GetApp()->pluginLock.WaitToRead();

	// Get the number of visual agents
	count = GetApp()->visualAgents.CountItems();

	// Call each agent
	for (i = 0; i < count; i++)
	{
		info = GetApp()->visualAgents.GetItem(i);
		info->agent->Run(info->index, APVA_STOP_SHOWING, NULL);
	}

	GetApp()->pluginLock.DoneReading();
}



/******************************************************************************/
/* PausePlaying() will pause the playing.                                     */
/******************************************************************************/
void APMixer::PausePlaying(void)
{
	if (playing)
	{
		playing = false;
		soundOutput->Run(soundOutputInfo->index, APOA_PAUSE_PLAYING, NULL);
	}
}



/******************************************************************************/
/* ResumePlaying() will continue the playing again.                           */
/******************************************************************************/
void APMixer::ResumePlaying(void)
{
	if (!playing)
	{
		playing     = true;
		holdPlaying = false;
		soundOutput->Run(soundOutputInfo->index, APOA_RESUME_PLAYING, NULL);
	}
}



/******************************************************************************/
/* HoldPlaying() will hold the playing.                                       */
/*                                                                            */
/* Input:  "hold" is the hold flag. True for holding, false for playing.      */
/******************************************************************************/
void APMixer::HoldPlaying(bool hold)
{
	holdPlaying = hold;

	// Make sure the filler thread starts again
	if (!hold && useRingBuffer)
		fillBuffer->SetEvent();
}



/******************************************************************************/
/* UsingRingBuffers() returns true if the mixer uses ring buffers, false if   */
/*      not.                                                                  */
/*                                                                            */
/* Output: True for ring buffers, false for normal.                           */
/******************************************************************************/
bool APMixer::UsingRingBuffers(void) const
{
	return (useRingBuffer);
}



/******************************************************************************/
/* SetSongPosition() will change the song position in the ring buffer.        */
/*                                                                            */
/* Input:  "newPos" is the new position.                                      */
/******************************************************************************/
void APMixer::SetSongPosition(int16 newPos)
{
	if (useRingBuffer)
	{
		holdPlaying = true;
		endIndex    = -1;
		newIndex    = playIndex;
		reportCnt   = 4;
		reportPos   = newPos;

		// Tell the filler thread that we has changed the position
		newPosSignal->SetEvent();
		readySignal->SetEvent();
	}
}



/******************************************************************************/
/* SetVolume() sets a new master volume.                                      */
/*                                                                            */
/* Input:  "volume" is the new volume between 0 and 256.                      */
/******************************************************************************/
void APMixer::SetVolume(uint16 volume)
{
	APAgent_SetVolume setVolume;

	// We should have an output agent at this point
	ASSERT(soundOutput != NULL);

	// Tell the agent to change the master volume
	setVolume.volume = volume;
	soundOutput->Run(soundOutputInfo->index, APOA_SET_VOLUME, &setVolume);
}



/******************************************************************************/
/* SetStereoSeparation() sets a new stereo separation value.                  */
/*                                                                            */
/* Input:  "sep" is the new stereo separation in percent.                     */
/******************************************************************************/
void APMixer::SetStereoSeparation(uint16 sep)
{
	int32 i,count;
	VirtualMixer virtMix;

	ASSERT(currentMixer != NULL);
	currentMixer->SetStereoSeparation(sep);

	// Call extra mixers
	count = mixerList.CountItems();

	for (i = 0; i < count; i++)
	{
		// Get current item
		virtMix = mixerList.GetItem(i);
		virtMix.mixer->SetStereoSeparation(sep);
	}
}



/******************************************************************************/
/* SetMixerMode() will change the mixer mode.                                 */
/*                                                                            */
/* Input:  "mode" is the new mixer mode flags to enable or disable.           */
/*         "enable" is true if you want to enable the flags, else false.      */
/******************************************************************************/
void APMixer::SetMixerMode(uint32 mode, bool enable)
{
	// Use temporary variable to make the operation atomic
	uint32 newMode = mixerMode;

	newMode &= ~mode;
	if (enable)
		newMode |= mode;

	mixerMode = newMode;
}



/******************************************************************************/
/* EnableAmigaFilter() will enable or disable the Amiga filter emulation.     */
/*                                                                            */
/* Input:  "enable" is true if you want to enable the emulation, else false.  */
/******************************************************************************/
void APMixer::EnableAmigaFilter(bool enable)
{
	emulateFilter = enable;
}



/******************************************************************************/
/* EnableChannel() will enable or disable a single channel.                   */
/*                                                                            */
/* Input:  "channel" is the channel to change.                                */
/*         "enable" is true if you want to enable the channel, else false.    */
/******************************************************************************/
void APMixer::EnableChannel(uint16 channel, bool enable)
{
	ASSERT(channel < MAX_NUM_CHANNELS);

	channelsEnabled[channel] = enable;
}



/******************************************************************************/
/* DisableVirtualMixer() will disable the virtual mixer given.                */
/*                                                                            */
/* Input:  "agent" is the agent with the virtual mixer to disable.            */
/******************************************************************************/
void APMixer::DisableVirtualMixer(AddOnInfo *agent)
{
	VirtualMixer virtMix;
	int32 i, count;

	// Lock the mixer list
	mixerLock.Lock();

	// Get number of agents that need an virtual mixer
	count = mixerList.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the current item
		virtMix = mixerList.GetItem(i);

		// Is the item the agent we search for?
		if (virtMix.agent == agent)
		{
			// Yes, disable it
			virtMix.available = false;
			mixerList.SetItem(virtMix, i);
			break;
		}
	}

	mixerLock.Unlock();
}



/******************************************************************************/
/* Mixer() is the main mixer function. It's the function to be called from    */
/*      the output class.                                                     */
/*                                                                            */
/* Input:  "handle" is the mixer handle.                                      */
/*         "buffer" is a pointer to the buffer to fill with the sampling.     */
/*         "count" is the size of the buffer in samples.                      */
/*                                                                            */
/* Output: Number of samples mixed.                                           */
/******************************************************************************/
int32 APMixer::Mixer(void *handle, int16 *buffer, int32 count)
{
	APMixer *object = (APMixer *)handle;
	int32 retVal = 0;

	if (object->holdPlaying)
	{
		// Clear the buffer and return
		memset(buffer, 0, count * sizeof(int16));

		if (object->useRingBuffer && object->playLocked)
		{
			// Unlock the ring buffer
			object->ringInfo[object->playIndex].mutex->Unlock();
			object->playLocked = false;
		}

		object->allMutexesFree = true;
	}
	else
	{
		object->allMutexesFree = false;

		if (object->useRingBuffer)
		{
			if (object->playing)
			{
				// Ring buffer playing
				retVal = object->DoRingBuffer(buffer, count);
			}
			else
			{
				// Clear the buffer and return
				memset(buffer, 0, count * sizeof(int16));
			}
		}
		else
		{
			// Normal playing
			retVal = object->DoMixing1(count);
			object->DoMixing2(buffer, count);
		}
	}

	// Tell the visual agents about the mixed data
	object->currentVisualizer->TellAgents_MixedData(buffer, count);

	return (retVal);
}



/******************************************************************************/
/* DoMixing1() is the main mixer function. It will call the right mixer       */
/*      function and mix the main module samples.                             */
/*                                                                            */
/* Input:  "todo" is the size of the buffer in samples.                       */
/*                                                                            */
/* Output: Number of samples mixed.                                           */
/******************************************************************************/
int32 APMixer::DoMixing1(int32 todo)
{
	int32 left, total = 0;
	uint16 t;
	int32 bufSize;

	// Remember the mixing mode. The reason to hold this in a local
	// variable, is when the user change the mixing mode, it won't
	// be changed in the middle of a mixing, but used the next
	// time this function is called.
	curMode = mixerMode;

	// Find the size of the buffer
	bufSize = min(bufferSize, todo);

	// And convert the number of samples to number of samples pair
	todo = (curMode & DMODE_STEREO ? bufSize >> 1 : bufSize);

	// Prepare the mixing buffer
	memset(mixBuffer, 0, bufferSize * sizeof(int32));

	if (playing || useRingBuffer)
	{
		while (todo != 0)
		{
			if (tickLeft == 0)
			{
				// Wait until it's okay to play
				playerLock->Lock();

				// Call player routine
				currentPlayer->Play();

				// Unlock again
				playerLock->Unlock();

				// Get some mixer information we need to parse the data
				VINFO *vinf = currentMixer->GetMixerChannels();
				int32 click = currentMixer->GetClickConstant();

				// Parse the channel info into our intern structure and
				// convert some of the data if needed.
				if (samplePlay)
				{
					// Parse channels in sample player mode
					for (t = 0; t < modChannelNum; t++)
						((APChannelParser *)currentPlayer->virtChannels[t])->ParseSampleInfo(&vinf[t], click);

					// Calculate the number of sample pair to mix before the
					// player need to be called again
					tickLeft = mixerFreq * vinf[0].size / vinf[0].frq;
				}
				else
				{
					// Parse channels in module player mode
					uint32 *flagArray;
					uint32 chanFlags = 0;

					flagArray = currentVisualizer->GetFlagsArray();

					for (t = 0; t < modChannelNum; t++)
					{
						flagArray[t] = ((APChannelParser *)currentPlayer->virtChannels[t])->ParseInfo(&vinf[t], click);
						chanFlags   |= flagArray[t];
					}

					// If at least one channel has changed its information,
					// tell visual agents about it
					if (chanFlags != 0)
						currentVisualizer->TellAgents_ChannelChanged();

					// Calculate the number of sample pair to mix before the
					// player need to be called again
					tickLeft = (uint32)(mixerFreq / currentPlayer->playFreq);
				}

				if (currentPlayer->endReached)
				{
					// We got an end flag, so stop the playing and start
					// the next module
					currentPlayer->endReached = false;

					if (useRingBuffer)
					{
						endPosition     = total;
						endIndex        = fillIndex;
						endSongPosition = currentPlayer->GetSongPosition();

						// Stop filling the buffer
						fillBuffer->ResetEvent();
					}
					else
						playerInfo->PostMessage(AP_MODULE_ENDED);

					break;
				}

				// If tickLeft is still 0, the player doesn't play
				// anything at all, so jump out of the loop
				if (tickLeft == 0)
					break;
			}

			// Find the number of sample pair to mix
			left = min(tickLeft, todo);

			// And mix it
			currentMixer->Mixing(mixBuffer + total, left, curMode);

			// Calculate new values for the counter variables
			tickLeft -= left;
			todo     -= left;
			total    += (curMode & DMODE_STEREO ? left << 1 : left);

			// Check all the channels to see if they are still active and
			// enable/disable the channels depending on the user settings
			for (t = 0; t < modChannelNum; t++)
			{
				((APChannelParser *)currentPlayer->virtChannels[t])->Active(currentMixer->IsActive(t));
				currentMixer->EnableChannel(t, channelsEnabled[t]);
			}
		}
	}

	return (total);
}



/******************************************************************************/
/* DoMixing2() is the secondary mixer function. It will call the right mixer  */
/*      function and mix the extra samples and add effect to the mixed data   */
/*      and store it into the buffer given.                                   */
/*                                                                            */
/* Input:  "buf" is a pointer to the buffer to fill with the sampling.        */
/*         "todo" is the size of the buffer in samples.                       */
/******************************************************************************/
void APMixer::DoMixing2(int16 *buf, int32 todo)
{
	VirtualMixer virtMix;
	APAgent_Mixing agentMixing;
	int32 i, count;
	uint16 t;
	int32 bufSize;

	// Find the size of the buffer
	bufSize = min(bufferSize, todo);

	// Call extra mixers
	mixerLock.Lock();

	count = mixerList.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get current item
		virtMix = mixerList.GetItem(i);

		if (virtMix.available)
		{
			// Initialize the mixing structure
			agentMixing.channels = (APChannel **)virtMix.channels;

			// Call the agent
			if (virtMix.agent->agent->Run(virtMix.agent->index, APMA_MIXING, &agentMixing) == AP_OK)
			{
				// Get some mixer information we need to parse the data
				VINFO *vinf = virtMix.mixer->GetMixerChannels();
				int32 click = virtMix.mixer->GetClickConstant();

				// Parse channels
				for (t = 0; t < virtMix.channelNum; t++)
					virtMix.channels[t]->ParseInfo(&vinf[t], click);

				// Mix the data
				virtMix.mixer->Mixing(mixBuffer, (curMode & DMODE_STEREO ? bufSize >> 1 : bufSize), curMode);

				// Check all the channels to see if they are still active
				for (t = 0; t < virtMix.channelNum; t++)
					virtMix.channels[t]->Active(virtMix.mixer->IsActive(t));
			}
		}
	}

	mixerLock.Unlock();

	// Add effects on the mixed data. It doesn't matter which mixer we
	// call the next couple of functions, so we use the "module" mixer.
	currentMixer->AddEffects(mixBuffer, bufSize, curMode);

	// Add Amiga low-pass filter if enabled
	AddAmigaFilter(mixBuffer, bufSize);

	// Now convert the mixed data to our output format
	currentMixer->ConvertMixedData(buf, mixBuffer, bufSize, curMode);
}



/******************************************************************************/
/* AddAmigaFilter() adds the Amiga LED filter if enabled.                     */
/*                                                                            */
/* Input:  "dest" is a pointer to the buffer to add the filter on.            */
/*         "todo" is the number of samples to modify.                         */
/******************************************************************************/
void APMixer::AddAmigaFilter(int32 *dest, int32 todo)
{
	int32 i;

	// Should we emulate the filter at all?
	if (emulateFilter)
	{
		// Is filter enabled in the player
		if (currentPlayer->amigaFilter)
		{
			if (curMode & DMODE_STEREO)
			{
				// Stereo buffer
				todo /= 2;

				for (i = 0; i < todo; i++)
				{
					filterPrevLeft = ((*dest) + filterPrevLeft * 3) >> 2;
					*dest++ = filterPrevLeft;

					filterPrevRight = ((*dest) + filterPrevRight * 3) >> 2;
					*dest++ = filterPrevRight;
				}
			}
			else
			{
				// Mono buffer
				for (i = 0; i < todo; i++)
				{
					filterPrevLeft = ((*dest) + filterPrevLeft * 3) >> 2;
					*dest++ = filterPrevLeft;
				}
			}
		}
	}
}



/******************************************************************************/
/* InitVirtualMixer() initialize extra virtual mixers.                        */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void APMixer::InitVirtualMixer(void)
{
	VirtualMixer virtMix;
	int32 i, j, count;

	// Lock the mixer list
	mixerLock.Lock();

	// Get number of agents that need an virtual mixer
	GetApp()->pluginLock.WaitToRead();
	count = GetApp()->virtualMixerAgents.CountItems();

	try
	{
		for (i = 0; i < count; i++)
		{
			APAgent_InitMixer initMixer;

			// Get the current agent
			virtMix.agent = GetApp()->virtualMixerAgents.GetItem(i);

			// Call the initialize function
			initMixer.channels = 0;
			virtMix.agent->agent->Run(virtMix.agent->index, APMA_INIT_MIXER, &initMixer);
			virtMix.channelNum = initMixer.channels;

			if (virtMix.channelNum != 0)
			{
				// Allocate mixer
				virtMix.mixer = new APMixerNormal();
				if (virtMix.mixer == NULL)
					throw PMemoryException();

				try
				{
					// Initialize the mixer
					if (!virtMix.mixer->Initialize(mixerFreq, virtMix.channelNum))
						throw PUserException();

					// Allocate channel objects
					virtMix.channels = new APChannelParser *[virtMix.channelNum];
					if (virtMix.channels == NULL)
						throw PMemoryException();

					for (j = 0; j < virtMix.channelNum; j++)
					{
						virtMix.channels[j] = new APChannelParser();
						if (virtMix.channels[j] == NULL)
							throw PMemoryException();
					}

					// Initialize the rest of the structure
					virtMix.available = true;
				}
				catch(...)
				{
					delete virtMix.mixer;
					throw;
				}

				// Add the mixer to the list
				mixerList.AddTail(virtMix);
			}
		}
	}
	catch(...)
	{
		GetApp()->pluginLock.DoneReading();
		mixerLock.Unlock();
		throw;
	}

	// Unlock the plug-in and mixer lists
	GetApp()->pluginLock.DoneReading();
	mixerLock.Unlock();
}



/******************************************************************************/
/* EndVirtualMixer() cleanup extra virtual mixers.                            */
/******************************************************************************/
void APMixer::EndVirtualMixer(void)
{
	VirtualMixer virtMix;
	int32 i, j, count;

	// Lock the mixer list
	mixerLock.Lock();

	// Get number of agents that need an virtual mixer
	count = mixerList.CountItems();

	for (i = 0; i < count; i++)
	{
		// Get the current item
		virtMix = mixerList.GetAndRemoveItem(0);

		// Call the cleanup functions
		if (virtMix.available)
			virtMix.agent->agent->Run(virtMix.agent->index, APMA_END_MIXER, NULL);

		virtMix.mixer->Cleanup();

		// Deallocate mixer
		delete virtMix.mixer;

		// Deallocate the channel objects
		for (j = 0; j < virtMix.channelNum; j++)
			delete virtMix.channels[j];

		delete[] virtMix.channels;
	}

	mixerLock.Unlock();
}



/******************************************************************************/
/* InitRingBuffer() initialize the ring buffer variables and stuff.           */
/******************************************************************************/
void APMixer::InitRingBuffer(void)
{
	if (useRingBuffer)
	{
		uint16 i;
		PString name;

		// Clear the ring information structures
		for (i = 0; i < RINGBUFFER_NUM; i++)
		{
			ringInfo[i].mutex    = NULL;
			ringInfo[i].buffer   = NULL;
			ringInfo[i].filled   = false;
			ringInfo[i].position = -1;
		}

		// Allocate ring buffers
		for (i = 0; i < RINGBUFFER_NUM; i++)
		{
			name.Format("Ringbuffer mutex #%d", i);
			ringInfo[i].mutex = new PMutex(name, false);
			if (ringInfo[i].mutex == NULL)
				throw PMemoryException();

			ringInfo[i].buffer = new int16[RINGBUFFER_SIZE];
			if (ringInfo[i].buffer == NULL)
				throw PMemoryException();
		}

		// Create thread exit event
		exitEvent = new PEvent("Ringbuffer exit event", true, false);
		if (exitEvent == NULL)
			throw PMemoryException();

		// Create stop filling event
		fillBuffer = new PEvent("Ringbuffer fill event", true, true);
		if (fillBuffer == NULL)
			throw PMemoryException();

		// Create "position changed" event
		newPosSignal = new PEvent("Ringbuffer position changed event", true, false);
		if (newPosSignal == NULL)
			throw PMemoryException();

		// Create ready event
		readySignal = new PEvent("Ringbuffer ready event", true, false);
		if (readySignal == NULL)
			throw PMemoryException();

		// Initialize ring buffer variables
		playIndex       = 0;
		fillIndex       = 0;
		endIndex        = -1;
		newIndex        = -1;

		playPosition    = 0;
		endPosition     = 0;

		reportCnt       = 0;
		reportPos       = -1;
		oldPos          = -1;
		endSongPosition = 0;

		playLocked      = false;
		firstTime       = true;
		allMutexesFree  = false;

		// Initialize ring buffer thread
		ringThread.SetName("Ringbuffer filler");
		ringThread.SetHookFunc(RingBufferFiller, this);
		ringThread.SetPriority(PThread::pHigh);
	}
}



/******************************************************************************/
/* EndRingBuffer() stops the ring buffer thread and clean up.                 */
/******************************************************************************/
void APMixer::EndRingBuffer(void)
{
	if (useRingBuffer && (readySignal != NULL))
	{
		uint16 i;

		// Exit the thread
		exitEvent->SetEvent();
		fillBuffer->SetEvent();
		readySignal->SetEvent();

		// If one of the buffers is locked, unlock it for fast exit
		// of the RingBufferFiller thread.
		if (playLocked)
		{
			// Unlock the ring buffer
			ringInfo[playIndex].mutex->Unlock();
		}

		ringThread.WaitOnThread();

		// Delete all the events
		delete readySignal;
		readySignal = NULL;

		delete newPosSignal;
		newPosSignal = NULL;

		delete fillBuffer;
		fillBuffer = NULL;

		delete exitEvent;
		exitEvent = NULL;

		// Deallocate ring buffers
		for (i = 0; i < RINGBUFFER_NUM; i++)
		{
			delete[] ringInfo[i].buffer;
			delete ringInfo[i].mutex;

			ringInfo[i].buffer = NULL;
			ringInfo[i].mutex  = NULL;
		}
	}
}



/******************************************************************************/
/* ResetRingBuffer() resets the ring buffer system and flushes all buffered   */
/*      data.                                                                 */
/******************************************************************************/
void APMixer::ResetRingBuffer(void)
{
	int32 i;

	for (i = 0; i < RINGBUFFER_NUM; i++)
	{
		// Start to wait for the mutex
		ringInfo[i].mutex->Lock();

		// Reset the buffer
		ringInfo[i].filled   = false;
		ringInfo[i].position = 0;

		// Unlock again
		ringInfo[i].mutex->Unlock();
	}
}



/******************************************************************************/
/* DoRingBuffer() get the mixed data from the ring buffers and copy it to the */
/*      supplied buffer.                                                      */
/*                                                                            */
/* Input:  "buf" is a pointer to the buffer to fill with the sampling.        */
/*         "count" is the size of the buffer in samples.                      */
/*                                                                            */
/* Output: Number of samples mixed.                                           */
/******************************************************************************/
int32 APMixer::DoRingBuffer(int16 *buf, int32 count)
{
	// Ring buffer playing
	int32 todo, bufSize = RINGBUFFER_SIZE;
	int32 mixed = 0, retVal = 0;
	int16 *buffer = buf;
	int16 newPos;

	while (count > 0)
	{
		// Find the ring buffer to get the sound data from and lock it
		if (!playLocked)
		{
			ringInfo[playIndex].mutex->Lock();

			// The buffer isn't filled yet, so exit
			if (!ringInfo[playIndex].filled)
			{
				ringInfo[playIndex].mutex->Unlock();

				// Clear the buffer
				memset(buffer, 0, count >> 1);
				return (0);
			}

			playLocked = true;
		}

		// Find the number of samples to copy
		if (playIndex == endIndex)
			bufSize = endPosition;

		// Copy the samples
		todo = min(count, bufSize - playPosition);
		memcpy(buffer, ringInfo[playIndex].buffer + playPosition, todo << 1);

		// Calculate new counter values
		count        -= todo;
		playPosition += todo;
		buffer       += todo;
		mixed        += todo;

		// Check to see if we should return the play position or the
		// just changed position.
		//
		// The reason we has a report position, is when the user
		// change the position, the slider will jump back to the
		// original position and then back to the new one if we
		// don't have the report position.
		if (reportPos == -1)
			newPos = ringInfo[playIndex].position;
		else
			newPos = reportPos;

		// If the position has changed, report it
		if (newPos != oldPos)
		{
			BMessage msg(AP_REPORT_POSITION);

			// Build and send the message
			msg.AddInt16("position", newPos);
			playerInfo->PostMessage(&msg);

			oldPos = newPos;
		}

		if (playPosition == bufSize)
		{
			// Unlock the ring buffer
			ringInfo[playIndex].filled = false;
			playLocked = false;
			ringInfo[playIndex].mutex->Unlock();

			// Tell the filler thread that at least one buffer is ready
			// to be filled
			readySignal->SetEvent();

			if (playIndex == endIndex)
			{
				// Clear the rest of the buffer
				if (count != 0)
					memset(buffer, 0, count << 1);

				// The module has been played, so tell about
				// a position change and then a module end message
				BMessage msg(AP_REPORT_POSITION);
				msg.AddInt16("position", endSongPosition);
				playerInfo->PostMessage(&msg);

				// Send the module ended message
				playerInfo->PostMessage(AP_MODULE_ENDED);
				endIndex = -1;
				retVal = mixed;
			}

			playPosition = 0;
			playIndex++;

			if (playIndex == RINGBUFFER_NUM)
				playIndex = 0;

			break;
		}
	}

	return (retVal != 0 ? retVal: mixed);
}



/******************************************************************************/
/* RingBufferFiller() calls the player and fill the ring buffers.             */
/*                                                                            */
/* Input:  "userData" is a pointer to the mixer object.                       */
/*                                                                            */
/* Output: Always zero.                                                       */
/******************************************************************************/
int32 APMixer::RingBufferFiller(void *userData)
{
	APMixer *mixer = (APMixer *)userData;

	try
	{
		// Well, do forever. The loop will be breaked if the exit event is set
		for (;;)
		{
			PSyncError result;

			do
			{
				// Wait for permission to fill
				if (mixer->fillBuffer->Lock() != pSyncOk)
					return (0);		// Some error occurred

				// Has the exit event been trigged?
				if (mixer->exitEvent->Lock(0) == pSyncOk)
					return (0);		// Yes, exit

				// Has the user changed the position
				if (mixer->newPosSignal->Lock(0) == pSyncOk)
					mixer->SetNewPosition();

				// Wait max one second to get the buffer to fill
				result = mixer->ringInfo[mixer->fillIndex].mutex->Lock(1000);
				if (result == pSyncOk)
				{
					// We got the buffer, but has it already been filled?
					if (mixer->ringInfo[mixer->fillIndex].filled)
					{
						// Yes, unlock the buffer again
						mixer->ringInfo[mixer->fillIndex].mutex->Unlock();
						result = pSyncTimeout;
					}
				}

				// Do we need to wait for a ready signal from the player thread?
				if (result != pSyncOk)
				{
					// Wait for the ready signal
					if (mixer->readySignal->Lock() != pSyncOk)
						return (0);		// Some error occured

					// Reset the signal
					mixer->readySignal->ResetEvent();
				}
			}
			while (result != pSyncOk);

			// Clear the report position
			if (mixer->reportCnt > 0)
			{
				mixer->reportCnt--;
				if (mixer->reportCnt == 0)
					mixer->reportPos = -1;
			}

			// Mix the data
			mixer->DoMixing1(RINGBUFFER_SIZE);
			mixer->DoMixing2(mixer->ringInfo[mixer->fillIndex].buffer, RINGBUFFER_SIZE);

			// Fill out the rest of the structure
			mixer->playerLock->Lock();
			mixer->ringInfo[mixer->fillIndex].position = mixer->currentPlayer->GetSongPosition();
			mixer->ringInfo[mixer->fillIndex].filled   = true;
			mixer->playerLock->Unlock();

			// Unlock again
			mixer->ringInfo[mixer->fillIndex].mutex->Unlock();

			mixer->fillIndex++;
			if (mixer->fillIndex == RINGBUFFER_NUM)
				mixer->fillIndex = 0;
		}
	}
	catch(...)
	{
		;
	}

	// Well, hopefully we will never reach this point
	return (0);
}



/******************************************************************************/
/* SetNewPosition() will reset the ringbuffer indexes to a new position.      */
/******************************************************************************/
void APMixer::SetNewPosition(void)
{
	int8 i, countIndex;

	countIndex = newIndex - 1;
	if (countIndex < 0)
		countIndex += RINGBUFFER_NUM;

	// Lock all the buffers and clear the filled flag
	for (i = 0; i < RINGBUFFER_NUM; i++)
	{
		ringInfo[countIndex].mutex->Lock();
		ringInfo[countIndex].filled = false;
		ringInfo[countIndex].mutex->Unlock();

		countIndex--;
		if (countIndex < 0)
			countIndex = RINGBUFFER_NUM - 1;
	}

	// We have prepared the buffers, so reset the indexes
	playPosition = 0;
	playIndex    = 0;
	fillIndex    = 0;
	newIndex     = -1;
	endIndex     = -1;
	holdPlaying  = false;

	// Reset the signal event
	newPosSignal->ResetEvent();
}
