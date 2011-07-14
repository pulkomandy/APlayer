/******************************************************************************/
/* APlayer DiskSaver Agent class.                                             */
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
#include "PSettings.h"
#include "PDirectory.h"
#include "PSystem.h"
#include "PThread.h"
#include "PSynchronize.h"
#include "PAlert.h"

// APlayerKit headers
#include "APAddOns.h"
#include "APGlobalData.h"
#include "APList.h"

// Agent headers
#include "DiskSaverAgent.h"
#include "DiskSaverView.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.07f



/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
#define MIXER_BUFFER_SIZE			65536		// Buffer size in samples



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *diskSaverSettings;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
DiskSaverAgent::DiskSaverAgent(APGlobalData *global, PString addOnName) : APAddOnAgent(global)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(addOnName);
	if (res == NULL)
		throw PMemoryException();

	// Initialize member variables
	file       = NULL;
	converter  = NULL;
	endEvent   = NULL;
	pauseEvent = NULL;
	soundAgent = NULL;
	mixBuffer  = NULL;
	saveBuffer = NULL;
	volume     = 256;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
DiskSaverAgent::~DiskSaverAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float DiskSaverAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetConfigInfo() will return a pointer to a config structure.               */
/*                                                                            */
/* Output: Is a pointer to a config structure.                                */
/******************************************************************************/
const APConfigInfo *DiskSaverAgent::GetConfigInfo(void)
{
	// Load the settings if not already loaded
	LoadSettings();

	// Create background view
	cfgInfo.view     = new DiskSaverView(globalData, res);
	cfgInfo.settings = diskSaverSettings;
	cfgInfo.fileName = "DiskSaver.ini";

	return (&cfgInfo);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 DiskSaverAgent::GetSupportFlags(int32 index)
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
PString DiskSaverAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_DISKSAVER_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString DiskSaverAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_DISKSAVER_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool DiskSaverAgent::InitAgent(int32 index)
{
	// Load the settings if not already loaded
	LoadSettings();

	return (true);
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
ap_result DiskSaverAgent::Run(int32 index, uint32 command, void *args)
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

		// Resume playing
		case APOA_RESUME_PLAYING:
		{
			ResumePlaying();
			return (AP_OK);
		}

		// Pause playing
		case APOA_PAUSE_PLAYING:
		{
			PausePlaying();
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
/* LoadSettings() will load the settings if not already loaded.               */
/******************************************************************************/
void DiskSaverAgent::LoadSettings(void)
{
	// Check to see if we need to allocate the settings object
	if (diskSaverSettings == NULL)
	{
		// Now allocate the settings
		diskSaverSettings = new PSettings();
		if (diskSaverSettings == NULL)
			throw PMemoryException();

		// Load the settings into the memory
		try
		{
			diskSaverSettings->LoadFile("DiskSaver.ini", "Polycode", "APlayer");
		}
		catch(PFileException e)
		{
			// Well, the file does probably not exists, so ignore the error
			;
		}

		// Fix the settings
		FixSettings();
	}
}



/******************************************************************************/
/* FixSettings() puts all the default setting values into the settings object.*/
/******************************************************************************/
void DiskSaverAgent::FixSettings(void)
{
	PString format, agent;
	APList<APAddOnInformation *> addOnList;
	APAddOnInformation *info;
	int32 i, count = 0;
	bool found = false;

	if (!diskSaverSettings->EntryExist("General", "DiskPath"))
	{
		PDirectory dir;

		dir.FindDirectory(PDirectory::pUser);
		diskSaverSettings->WriteStringEntryValue("General", "DiskPath", dir.GetDirectory());
	}

	if (!diskSaverSettings->EntryExist("General", "OutputSize"))
		diskSaverSettings->WriteIntEntryValue("General", "OutputSize", CV_OUTPUTSIZE_16BIT);

	if (!diskSaverSettings->EntryExist("General", "OutputType"))
		diskSaverSettings->WriteIntEntryValue("General", "OutputType", CV_OUTPUTTYPE_STEREO);

	// Check the output format
	//
	// Get the output format
	format = diskSaverSettings->GetStringEntryValue("General", "OutputFormat");

	// Get all the names of the savers
	globalData->GetAddOnList(apConverter, addOnList);

	if (!format.IsEmpty())
	{
		// Check the output format against all the converters
		count = addOnList.CountItems();
		for (i = 0; i < count; i++)
		{
			info = addOnList.GetItem(i);

			if ((info->pluginFlags & apcSaver) && (info->name.CompareNoCase(format) == 0))
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		// Well, we didn't find the converter, so reset the settings
		// to the first one or an empty string if no converter are loaded
		if (count == 0)
			diskSaverSettings->WriteStringEntryValue("General", "OutputFormat", "");
		else
			diskSaverSettings->WriteStringEntryValue("General", "OutputFormat", addOnList.GetItem(0)->name);
	}

	// Free the list again
	globalData->FreeAddOnList(addOnList);

	// Now check the "send through" output agent
	agent = diskSaverSettings->GetStringEntryValue("General", "OutputAgent");
	if (!agent.IsEmpty())
	{
		// Get all the agents
		globalData->GetAddOnList(apAgent, addOnList);

		// Find all the output agents and check them against the name in the settings
		found = false;
		count = addOnList.CountItems();
		for (i = 0; i < count; i++)
		{
			info = addOnList.GetItem(i);

			if ((info->pluginFlags & apaSoundOutput) && (info->name.CompareNoCase(agent) == 0))
			{
				found = true;
				break;
			}
		}

		if (!found)
			diskSaverSettings->WriteStringEntryValue("General", "OutputAgent", "");

		// Free the list again
		globalData->FreeAddOnList(addOnList);
	}
}



/******************************************************************************/
/* InitSaver() initialize the saver.                                          */
/*                                                                            */
/* Input:  "args" is the INIT_HARDWARE arguments.                             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void DiskSaverAgent::InitSaver(APAgent_InitHardware *args)
{
	// Should we use another output agent?
	soundAgentName = diskSaverSettings->GetStringEntryValue("General", "OutputAgent");
	if (soundAgentName.IsEmpty())
	{
		// Initialize the buffers and events needed
		endEvent = new PEvent(true, false);
		if (endEvent == NULL)
			throw PMemoryException();

		pauseEvent = new PEvent(true, true);
		if (pauseEvent == NULL)
			throw PMemoryException();

		// Allocate sample buffers
		mixBuffer = new int16[MIXER_BUFFER_SIZE];
		if (mixBuffer == NULL)
			throw PMemoryException();

		saveBuffer = new float[MIXER_BUFFER_SIZE];
		if (saveBuffer == NULL)
			throw PMemoryException();

		// Initialize and start the saver thread
		saveThread.SetName("APlayer disk saver");
		saveThread.SetHookFunc(MixerThread, this);
	}
	else
	{
		APAgent_InitHardware initHardware;

		// Allocate a new instance of the other sound output
		// agent we should use
		soundAgent = (APAddOnAgent *)globalData->GetAddOnInstance(soundAgentName, apAgent, &soundAgentIndex);
		if (soundAgent == NULL)
			throw PUserException();

		// Initialize the agent
		if (!soundAgent->InitAgent(soundAgentIndex))
			throw PUserException();

		// Fill out the structure to give to the sound agent
		initHardware.mixerFunc  = AgentFunc;
		initHardware.handle     = this;
		initHardware.frequency  = args->frequency;
		initHardware.fileName   = args->fileName;
		initHardware.moduleName = args->moduleName;
		initHardware.author     = args->author;

		// Initialize the other agent
		if (soundAgent->Run(soundAgentIndex, APOA_INIT_HARDWARE, &initHardware) != AP_OK)
			throw PUserException();
	}
}



/******************************************************************************/
/* EndSaver() cleanup the saver.                                              */
/******************************************************************************/
void DiskSaverAgent::EndSaver(void)
{
	// Stop the other sound output agent if any
	if (soundAgent != NULL)
	{
		soundAgent->Run(soundAgentIndex, APOA_END_HARDWARE, NULL);

		soundAgent->EndAgent(soundAgentIndex);
		globalData->DeleteAddOnInstance(soundAgentName, apAgent, soundAgent);
		soundAgent = NULL;
		soundAgentName.MakeEmpty();
	}

	// Clean-up
	delete[] saveBuffer;
	delete[] mixBuffer;
	saveBuffer = NULL;
	mixBuffer  = NULL;

	delete pauseEvent;
	pauseEvent = NULL;

	delete endEvent;
	endEvent = NULL;
}



/******************************************************************************/
/* DoMixing() calls the mixer and saves the data to disk.                     */
/*                                                                            */
/* Input:  "buffer" is a pointer to a buffer where the mixed data should be   */
/*         stored.                                                            */
/*         "bufSize" is the size of the buffer in samples.                    */
/*                                                                            */
/* Output: Number of samples mixed.                                           */
/******************************************************************************/
int32 DiskSaverAgent::DoMixing(int16 *buffer, int32 bufSize)
{
	int16 *sourceBuf;
	float *destBuf;
	float factor;
	int32 mixed;

	// Mix into 16-bit buffers
	mixed = theMixer(mixerHandle, buffer, bufSize);

	if (mixed != 0)
	{
		// Now convert the 16-bit buffers to float buffers
		sourceBuf = buffer;
		destBuf   = saveBuffer;
		factor    = 32768.0f * 256.0f / volume;

		for (int32 i = 0; i < mixed; i++)
			*destBuf++ = ((float)*sourceBuf++) / factor;

		// Save the data to disk
		converter->SaveData(file, saveBuffer, mixed, &convInfo);
	}

	return (mixed);
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
ap_result DiskSaverAgent::InitHardware(APAgent_InitHardware *args)
{
	PString fileName;

	// Remember the mixer function and handle
	theMixer    = args->mixerFunc;
	mixerHandle = args->handle;

	// Allocate the file object
	file = new PFile();
	if (file == NULL)
		throw PMemoryException();

	try
	{
		// Get the bit size to save the samples in
		convInfo.bitSize = diskSaverSettings->GetIntEntryValue("General", "OutputSize");

		// Get the number of channels
		if (diskSaverSettings->GetIntEntryValue("General", "OutputType") == CV_OUTPUTTYPE_STEREO)
			convInfo.channels = 2;
		else
			convInfo.channels = 1;

		// Fill in the rest of the converter structure
		convInfo.name       = args->moduleName;
		convInfo.author     = args->author;
		convInfo.flags      = 0;
		convInfo.frequency  = args->frequency;
		convInfo.volume     = 256;
		convInfo.panning    = -1;
		convInfo.loopStart  = 0;
		convInfo.loopLength = 0;

		// Find the saver to use
		converterName = diskSaverSettings->GetStringEntryValue("General", "OutputFormat");

		// Is there selected any converter
		if (converterName.IsEmpty())
		{
			PString title, msg;

			title.LoadString(res, IDS_DISKSAVER_WIN_TITLE);
			msg.LoadString(res, IDS_DISKSAVER_ERR_NOSELCONVERTER);

			PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
			alert.Show();
			throw PUserException();
		}

		// Try to find the converter
		converter = (APAddOnConverter *)globalData->GetAddOnInstance(converterName, apConverter, &converterIndex);
		if (converter == NULL)
		{
			PString title, msg;

			title.LoadString(res, IDS_DISKSAVER_WIN_TITLE);
			msg.Format_S1(res, IDS_DISKSAVER_ERR_NOCONVERTER, converterName);

			PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
			alert.Show();
			throw PUserException();
		}

		// Get the path
		fileName = diskSaverSettings->GetStringEntryValue("General", "DiskPath");
		if (!fileName.IsEmpty())
		{
			if (fileName.Right(1) != P_DIRSLASH_STR)
				fileName += P_DIRSLASH_STR;
		}

		// Add extension
		fileName += file->RequestFileExtension(PDirectory::GetFilePart(args->fileName), converter->GetExtension(converterIndex).Mid(1));

		// Check to see if the file already exists
		if (file->FileExists(fileName))
		{
			// Ask the user if it's okay to overwrite the file
			PString title, msg;

			title.LoadString(res, IDS_DISKSAVER_WIN_TITLE);
			msg.Format_S1(res, IDS_DISKSAVER_MSG_OVERWRITEFILE, fileName);

			PAlert alert(title, msg, PAlert::pQuestion, PAlert::pYesNo, 1);
			if (alert.Show() == PAlert::pIDNo)
				throw PUserException();
		}

		// Open the file
		file->Open(fileName, PFile::pModeCreate | PFile::pModeReadWrite);

		// Initialize the saver
		if (!converter->SaverInit(converterIndex, &convInfo))
			throw PSoundException();

		// Initialize and start what is needed to store the music to disk
		InitSaver(args);
	}
	catch(PUserException e)
	{
		return (AP_ERROR);
	}

	return (AP_OK);
}



/******************************************************************************/
/* EndHardware() stops the use of the sound hardware.                         */
/*                                                                            */
/* Except:	PSoundException.                                                  */
/******************************************************************************/
void DiskSaverAgent::EndHardware(void)
{
	// Cleanup any resources used by the saver
	EndSaver();

	// Save the last part and stop the converter
	if (converter != NULL)
	{
		// Is the file opened at all?
		if (file->IsOpen())
		{
			// Write the tail and what-ever missing
			converter->SaveTail(file, &convInfo);

			// Clean up the saver
			converter->SaverEnd(converterIndex, &convInfo);
		}

		globalData->DeleteAddOnInstance(converterName, apConverter, converter);
		converter = NULL;
		converterName.MakeEmpty();
	}

	delete file;
	file = NULL;
}



/******************************************************************************/
/* GetOutputInformation() fills out the output information structure with the */
/*      sample output format you want the mixer to give you.                  */
/*                                                                            */
/* Input:  "outputInfo" is a pointer to the structure to fill out.            */
/******************************************************************************/
void DiskSaverAgent::GetOutputInformation(APAgent_OutputInfo *outputInfo)
{
	if (soundAgent != NULL)
	{
		// Find out the information the other output agent want
		soundAgent->Run(soundAgentIndex, APOA_GET_OUTPUT_INFORMATION, outputInfo);

		// Force the output to the format that the other sound output agent wants
		convInfo.channels = outputInfo->channels;

		// Allocate the save buffer
		saveBuffer = new float[outputInfo->bufferSize];
		if (saveBuffer == NULL)
			throw PMemoryException();
	}
	else
	{
		outputInfo->channels   = convInfo.channels;
		outputInfo->bufferSize = MIXER_BUFFER_SIZE;
	}
}



/******************************************************************************/
/* StartPlaying() starts to play the sound.                                   */
/******************************************************************************/
void DiskSaverAgent::StartPlaying(void)
{
	// Do only write the header if the file position is
	// at the beginning of the file. This makes the saver
	// compatible with subsong switching
	if (file->GetPosition() == 0)
	{
		// Write the header
		converter->SaveHeader(file, &convInfo);
	}

	if (soundAgent != NULL)
	{
		// Tell the sound agent to start
		soundAgent->Run(soundAgentIndex, APOA_START_PLAYING, NULL);
	}
	else
	{
		// Start the thread
		saveThread.StartThread();
	}
}



/******************************************************************************/
/* StopPlaying() stop playing the sound.                                      */
/******************************************************************************/
void DiskSaverAgent::StopPlaying(void)
{
	if (soundAgent != NULL)
	{
		// Tell the sound agent to stop
		soundAgent->Run(soundAgentIndex, APOA_STOP_PLAYING, NULL);
	}
	else
	{
		// Tell the thread to exit
		endEvent->SetEvent();

		// Wait till the thread has exited
		saveThread.WaitOnThread();
	}
}



/******************************************************************************/
/* PausePlaying() will temporary pause the sound.                             */
/******************************************************************************/
void DiskSaverAgent::PausePlaying(void)
{
	if (soundAgent != NULL)
	{
		// Tell the sound agent to pause
		soundAgent->Run(soundAgentIndex, APOA_PAUSE_PLAYING, NULL);
	}
	else
	{
		// Pause the saver thread
		pauseEvent->ResetEvent();
	}
}



/******************************************************************************/
/* ResumePlaying() will start playing the sound after it has been paused.     */
/******************************************************************************/
void DiskSaverAgent::ResumePlaying(void)
{
	if (soundAgent != NULL)
	{
		// Tell the sound agent to resume playing
		soundAgent->Run(soundAgentIndex, APOA_RESUME_PLAYING, NULL);
	}
	else
	{
		// Resume the saver thread
		pauseEvent->SetEvent();
	}
}



/******************************************************************************/
/* SetVolume() will change the volume on the output.                          */
/*                                                                            */
/* Input:  "setVolume" is a pointer to the volume structure.                  */
/******************************************************************************/
void DiskSaverAgent::SetVolume(APAgent_SetVolume *setVolume)
{
	// Remember the volume
	volume = setVolume->volume;

	if (soundAgent != NULL)
	{
		// Tell the sound agent about the volume change
		soundAgent->Run(soundAgentIndex, APOA_SET_VOLUME, setVolume);
	}
}



/******************************************************************************/
/* MixerThread() is the hook function which will call the mixer. This         */
/*      function is only used if no other output agent is selected.           */
/*                                                                            */
/* Input:  "userData" is a pointer to the this object.                        */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 DiskSaverAgent::MixerThread(void *userData)
{
	DiskSaverAgent *output = (DiskSaverAgent *)userData;
	PSync *events[2];

	try
	{
		// Start to fill out the event array
		events[0] = output->endEvent;
		events[1] = output->pauseEvent;

		for (;;)
		{
			// See if any of the events has been set
			if (MultipleObjectsWait(events, 2, false, PSYNC_INFINITE) == 0)
				break;		// Stop the loop

			// Now call the mixer and save the data to disk
			output->DoMixing(output->mixBuffer, MIXER_BUFFER_SIZE);
		}
	}
	catch(...)
	{
		;
	}

	return (0);
}



/******************************************************************************/
/* AgentFunc() is called from the other sound output agent.                   */
/*                                                                            */
/* Input:  "handle" is the current object.                                    */
/*         "buffer" is a pointer to the buffer to fill with the sampling.     */
/*         "count" is the size of the buffer in samples.                      */
/*                                                                            */
/* Output: Number of samples mixed.                                           */
/******************************************************************************/
int32 DiskSaverAgent::AgentFunc(void *handle, int16 *buffer, int32 count)
{
	DiskSaverAgent *output = (DiskSaverAgent *)handle;

	// Call the mixer and save the data to disk
	return (output->DoMixing(buffer, count));
}
