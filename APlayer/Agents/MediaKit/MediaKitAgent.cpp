/******************************************************************************/
/* APlayer MediaKit Agent class.                                              */
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
#include "PSystem.h"

// APlayerKit headers
#include "APAddOns.h"

// Agent headers
#include "MediaKitAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.13f



/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define MIXER_BUFFER_SIZE			1024		// Number of samples in samples



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MediaKitAgent::MediaKitAgent(APGlobalData *global, PString addOnName) : APAddOnAgent(global)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(addOnName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	volume = 256;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MediaKitAgent::~MediaKitAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float MediaKitAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 MediaKitAgent::GetSupportFlags(int32 index)
{
	return (apaSoundOutput);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString MediaKitAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MEDIAKIT_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MediaKitAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MEDIAKIT_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* Run() will run one command in the agent.                                   */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*         "command" is the command to run.                                   */
/*         "args" is a pointer to a structure containing the arguments. The   */
/*         structure is different for each command.                           */
/*                                                                            */
/* Output: The result from the command.                                       */
/*                                                                            */
/* Except: PSoundException.                                                   */
/******************************************************************************/
ap_result MediaKitAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Initialize the output hardware
		case APOA_INIT_HARDWARE:
		{
			return (InitHardware((APAgent_InitHardware *)args));
		}

		// Cleanup the output hardware
		case APOA_END_HARDWARE:
		{
			EndHardware();
			return (AP_OK);
		}

		// Get output information
		case APOA_GET_OUTPUT_INFORMATION:
		{
			GetOutputInformation((APAgent_OutputInfo *)args);
			return (AP_OK);
		}

		// Start playing
		case APOA_START_PLAYING:
		{
			StartPlaying();
			return (AP_OK);
		}

		// Stop playing
		case APOA_STOP_PLAYING:
		{
			StopPlaying();
			return (AP_OK);
		}

		// Set volume
		case APOA_SET_VOLUME:
		{
			SetVolume((APAgent_SetVolume *)args);
			return (AP_OK);
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* InitHardware() initialize the hardware so it's ready to play the sound.    */
/*                                                                            */
/* Input:  "args" is a pointer to the init hardware structure.                */
/*                                                                            */
/* Output: AP_OK if you want to continue, AP_ERROR if not.                    */
/*                                                                            */
/* Except: PSoundException.                                                   */
/******************************************************************************/
ap_result MediaKitAgent::InitHardware(APAgent_InitHardware *args)
{
	// Remember the mixer function and handle
	theMixer    = args->mixerFunc;
	mixerHandle = args->handle;

	// Use the new media kit
	media_raw_audio_format format;
	status_t error;

	// Allocate a 16-bit sample buffer
	sampBufLen = MIXER_BUFFER_SIZE;

	sampBuffer = new int16[sampBufLen];
	if (sampBuffer == NULL)
		throw PMemoryException();

	// Fill out the audio structure
	format.frame_rate    = args->frequency;
	format.channel_count = 2;
	format.format        = media_raw_audio_format::B_AUDIO_FLOAT;
	format.buffer_size   = sampBufLen * sizeof(float);

#if __p_endian == __p_big
	format.byte_order    = B_MEDIA_BIG_ENDIAN;
#else
	format.byte_order    = B_MEDIA_LITTLE_ENDIAN;
#endif

	// Allocate the sound player
	soundPlayer = new BSoundPlayer(&format, "APlayer SoundPlayer", EnterStream, NULL, this);
	if (soundPlayer == NULL)
		throw PMemoryException();

	// Check the new sound player to see if it is initialized correctly
	error = soundPlayer->InitCheck();
	if (error != B_OK)
		throw PSoundException(PSystem::ConvertOSError(error));

	return (AP_OK);
}



/******************************************************************************/
/* EndHardware() stops the use of the sound hardware.                         */
/*                                                                            */
/* Except:	PSoundException.                                                  */
/******************************************************************************/
void MediaKitAgent::EndHardware(void)
{
	delete soundPlayer;
	soundPlayer = NULL;

	delete[] sampBuffer;
	sampBuffer = NULL;
}



/******************************************************************************/
/* GetOutputInformation() fills out the output information structure with the */
/*      sample output format you want the mixer to give you.                  */
/*                                                                            */
/* Input:  "outputInfo" is a pointer to the structure to fill out.            */
/******************************************************************************/
void MediaKitAgent::GetOutputInformation(APAgent_OutputInfo *outputInfo)
{
	outputInfo->channels   = 2;
	outputInfo->bufferSize = sampBufLen;
}



/******************************************************************************/
/* SetVolume() will change the volume on the output.                          */
/*                                                                            */
/* Input:  "setVolume" is a pointer to the volume structure.                  */
/******************************************************************************/
void MediaKitAgent::SetVolume(APAgent_SetVolume *setVolume)
{
	// Remember the volume
	volume = setVolume->volume;
}



/******************************************************************************/
/* StartPlaying() starts to play the sound.                                   */
/******************************************************************************/
void MediaKitAgent::StartPlaying(void)
{
	soundPlayer->Start();
	soundPlayer->SetHasData(true);
	soundPlayer->SetVolume(1.0f);
}



/******************************************************************************/
/* StopPlaying() stop playing the sound.                                      */
/******************************************************************************/
void MediaKitAgent::StopPlaying(void)
{
	// Stop mixer
	soundPlayer->SetHasData(false);
	soundPlayer->Stop();
}



/******************************************************************************/
/* EnterStream() is the hook function which will call the mixer.              */
/*                                                                            */
/* Input:  "object" is a pointer to the this object.                          */
/*         "buffer" is a pointer to the buffer to fill with the sampling.     */
/*         "size" is the size of the buffer in bytes.                         */
/*         "format" is a reference to the audio format structure.             */
/******************************************************************************/
void MediaKitAgent::EnterStream(void *object, void *buffer, size_t size, const media_raw_audio_format &format)
{
	MediaKitAgent *output = (MediaKitAgent *)object;
	int16 *sourceBuf;
	float *destBuf;
	float factor;

	// Check the output format
	if (format.format != media_raw_audio_format::B_AUDIO_FLOAT)
		return;

	if (size > output->sampBufLen * sizeof(float))
		size = output->sampBufLen;
	else
		size = size / sizeof(float);

	// Mix into 16-bit buffers
	output->theMixer(output->mixerHandle, output->sampBuffer, size);

	// Now convert the 16-bit buffers to float buffers
	sourceBuf = output->sampBuffer;
	destBuf   = (float *)buffer;
	factor    = 32768.0f * 256.0f / output->volume;

	for (size_t i = 0; i < size; i++)
		*destBuf++ = ((float)*sourceBuf++) / factor;
}
