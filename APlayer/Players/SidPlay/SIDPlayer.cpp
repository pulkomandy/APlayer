/******************************************************************************/
/* SID Player Interface.                                                      */
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
#include "PFile.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PSettings.h"
#include "PList.h"

// APlayerKit headers
#include "APChannel.h"

// Player headers
#include "SIDStil.h"
#include "SIDTune.h"
#include "SIDFile.h"
#include "SIDEmuEngine.h"
#include "SIDEmuPlayer.h"
#include "SIDView.h"
#include "SIDPlayer.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion			2.10f



/******************************************************************************/
/* Other constants                                                            */
/******************************************************************************/
#define SidPlay_Version			"1.36.57"

#define DEFAULT_BUFFER_SIZE		8192



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/
extern PSettings *sidSettings;
extern PMutex *stilLock;
extern SIDStil *sidStil;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SIDPlayer::SIDPlayer(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	sidFile   = NULL;
	emuEngine = NULL;
	tune      = NULL;
	buffer    = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SIDPlayer::~SIDPlayer(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float SIDPlayer::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetConfigInfo() will return a pointer to a config structure.               */
/*                                                                            */
/* Output: Is a pointer to a config structure.                                */
/******************************************************************************/
const APConfigInfo *SIDPlayer::GetConfigInfo(void)
{
	// Create background view
	cfgInfo.view     = new SIDView(res);
	cfgInfo.settings = sidSettings;
	cfgInfo.fileName = "SidPlay.ini";
	return (&cfgInfo);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 SIDPlayer::GetSupportFlags(int32 index)
{
	return (appSamplePlayer);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString SIDPlayer::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_SID_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString SIDPlayer::GetDescription(int32 index)
{
	PString description;

	description.Format(res, IDS_SID_DESCRIPTION, SidPlay_Version);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString SIDPlayer::GetModTypeString(int32 index)
{
	PString type;

	type.LoadString(res, IDS_SID_MIME);
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
ap_result SIDPlayer::ModuleCheck(int32 index, PFile *file)
{
	SIDFile tempFile(file, res, this);

	return (tempFile.TestIt());
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
ap_result SIDPlayer::LoadModule(int32 index, PFile *file, PString &errorStr)
{
	PString stilPath;
	bool stilOk;
	ap_result retVal = AP_OK;

	try
	{
		sidFile = new SIDFile(file, res, this);
		if (sidFile == NULL)
			throw PUserException();

		sidFile->TestIt();
		sidFile->FillInInfo();

		// Lock the STIL object
		stilLock->Lock();

		// Find STIL information about the file loaded
		stilPath = sidSettings->GetStringEntryValue("Misc", "StilPath");
		if (!stilPath.IsEmpty())
		{
			// Check to see if the base directory has changed since
			// the last time we loaded a SID module
			if (sidStil->GetBaseDir() != stilPath)
			{
				// It has changed, so reload the STIL
				stilOk = sidStil->SetBaseDir(stilPath);
			}
			else
				stilOk = true;

			if (stilOk)
			{
				PString path;
				PString globalComment, entry, bug;

				// Find the entries for the song loaded
				path          = file->GetFullPath();
				globalComment = sidStil->GetAbsGlobalComment(path);
				entry         = sidStil->GetAbsEntry(path);
				bug           = sidStil->GetAbsBug(path);

				// Now append the entries into one big one
				stilEntries = globalComment + entry + bug;
			}
		}

		// Unlock the STIL object again
		stilLock->Unlock();
	}
	catch(PException e)
	{
		// Delete the sid file object
		delete sidFile;
		sidFile = NULL;

		// Get error message
		errorStr.LoadString(res, IDS_SID_ERR_LOAD);
		retVal = AP_ERROR;
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
bool SIDPlayer::InitPlayer(int32 index)
{
	sidEmuConfig config;
	int32 oldIndex, newIndex;
	bool retVal;

	try
	{
		// Create the SID objects
		emuEngine = new SIDEmuEngine();
		if (emuEngine == NULL)
			throw PUserException();

		retVal = emuEngine->GetStatus();
		if (!retVal)
			throw PUserException();

		tune = new SIDTune(sidFile);
		if (tune == NULL)
			throw PUserException();

		player = new SIDEmuPlayer(&emuEngine->sid6581, &emuEngine->sid6510);
		if (player == NULL)
			throw PUserException();

		// Okay, set the configuration data
		InitConfig();
		emuEngine->GetConfig(config);
		mixerFreq = config.frequency;

		// Allocate sample buffers
		buffer = new uint8[DEFAULT_BUFFER_SIZE];
		if (buffer == NULL)
			throw PUserException();

		// Clear the number of STIL lines
		stilLineCount = 0;

		// Count the number of lines in the STIL entry
		// and split them up into the array
		oldIndex = 0;
		while ((newIndex = stilEntries.Find('\n', oldIndex)) != -1)
		{
			stilLines.AddTail(stilEntries.Mid(oldIndex, newIndex - oldIndex));
			stilLineCount++;
			oldIndex = newIndex + 1;
		}

		retVal = true;
	}
	catch(...)
	{
		Cleanup();
		retVal = false;
	}

	return (retVal);
}



/******************************************************************************/
/* EndPlayer() ends the use of the player.                                    */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/******************************************************************************/
void SIDPlayer::EndPlayer(int32 index)
{
	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void SIDPlayer::InitSound(int32 index, uint16 songNum)
{
	player->EmuInitializeSong(*emuEngine, *tune, songNum + 1);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void SIDPlayer::Play(void)
{
	uint32 size, i;
	APChannel *channel;
	uint16 *tempBuf;

	// Check to see if any settings has been changed
	if (sidSettings->HasChanged())
	{
		// The settings has changed, but first clear the flag
		sidSettings->SetChangeFlag(false);

		// And then change the settings in the emulator
		InitConfig();
	}

	// Fill out the sample buffers
	// The function want the total size in bytes
	emuEngine->sid6581.EmuFillBuffer(*emuEngine, *tune, buffer, DEFAULT_BUFFER_SIZE);

	// Calculate the buffer size in samples
	size = DEFAULT_BUFFER_SIZE / 4 / 2;		// / 2 because it's 16 bit

	tempBuf = (uint16 *)buffer;

	for (i = 0; i < 4; i++)
	{
		channel = virtChannels[i];

		// Fill out the Channel object
		channel->SetBuffer(tempBuf, size);
		channel->SetVolume(256);
		channel->SetPanning(panningTab[i]);

		tempBuf += size;
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void SIDPlayer::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = mixerFreq;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString SIDPlayer::GetModuleName(void)
{
	return (tune->info.nameString);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString SIDPlayer::GetAuthor(void)
{
	return (tune->info.authorString);
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *SIDPlayer::GetSubSongs(void)
{
	songTab[0] = tune->info.songs;		// Number of subsongs
	songTab[1] = tune->info.startSong;	// Default start song

	return (songTab);
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
bool SIDPlayer::GetInfoString(uint32 line, PString &description, PString &value)
{
	uint32 maxLine = 3;

	// Calculate the maximum number of lines
	if (tune->info.musPlayer)
		maxLine += 5;

	// Add STIL lines
	maxLine += stilLineCount;

	// Is the line number out of range?
	if (line > maxLine)
		return (false);

	// Find out which line to take
	switch (line)
	{
		// Copyright
		case 0:
		{
			description.LoadString(res, IDS_SID_INFODESCLINE0);
			value = tune->info.copyrightString;
			break;
		}

		// Load Addr
		case 1:
		{
			description.LoadString(res, IDS_SID_INFODESCLINE1);
			value.Format("0x%04X", tune->info.loadAddr);
			break;
		}

		// Init Addr
		case 2:
		{
			description.LoadString(res, IDS_SID_INFODESCLINE2);
			value.Format("0x%04X", tune->info.initAddr);
			break;
		}

		// Play Addr
		case 3:
		{
			description.LoadString(res, IDS_SID_INFODESCLINE3);
			value.Format("0x%04X", tune->info.playAddr);
			break;
		}
	}

	line -= 4;
	if (tune->info.musPlayer)
	{
		switch (line)
		{
			// Information line 1
			case 0:
			{
				description.LoadString(res, IDS_SID_INFODESCLINE4);
				value = tune->info.infoStrings[0];
				break;
			}

			// Information line 2
			case 1:
			{
				description.LoadString(res, IDS_SID_INFODESCLINE5);
				value = tune->info.infoStrings[1];
				break;
			}

			// Information line 3
			case 2:
			{
				description.LoadString(res, IDS_SID_INFODESCLINE6);
				value = tune->info.infoStrings[2];
				break;
			}

			// Information line 4
			case 3:
			{
				description.LoadString(res, IDS_SID_INFODESCLINE7);
				value = tune->info.infoStrings[3];
				break;
			}

			// Information line 5
			case 4:
			{
				description.LoadString(res, IDS_SID_INFODESCLINE8);
				value = tune->info.infoStrings[4];
				break;
			}
		}

		line -= 5;
	}

	if ((stilLineCount > 0) && ((int32)line >= 0))
	{
		if (line == 0)
			description.LoadString(res, IDS_SID_INFODESC_STIL);
		else
			description.MakeEmpty();

		value = stilLines.GetItem(line);
		value.TrimLeft();
	}

	return (true);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void SIDPlayer::Cleanup(void)
{
	delete[] buffer;
	delete player;
	delete tune;
	delete emuEngine;
	delete sidFile;

	buffer    = NULL;
	player    = NULL;
	tune      = NULL;
	emuEngine = NULL;
	sidFile   = NULL;
}



/******************************************************************************/
/* InitConfig() get the registry values and initialize the player with them.  */
/******************************************************************************/
void SIDPlayer::InitConfig(void)
{
	sidEmuConfig config;
	bool flag;

	// First get the current configuration
	emuEngine->GetConfig(config);

	// Fill out the config with changes
	if (sidSettings->GetStringEntryValue("General", "MOS8580").CompareNoCase("Yes") == 0)
		flag = true;
	else
		flag = false;

	config.mos8580 = flag;

	if (sidSettings->GetStringEntryValue("General", "Filter").CompareNoCase("Yes") == 0)
		flag = true;
	else
		flag = false;

	config.emulateFilter = flag;

	config.filterFs = FILTER_PAR1_MIN - sidSettings->GetIntEntryValue("Filter", "FilterFs") + FILTER_PAR1_MAX;
	config.filterFm = sidSettings->GetIntEntryValue("Filter", "FilterFm");
	config.filterFt = FILTER_PAR3_MIN - ((float)sidSettings->GetIntEntryValue("Filter", "FilterFt") / 100) + FILTER_PAR3_MAX;

	switch (sidSettings->GetIntEntryValue("MPU", "Memory"))
	{
		case RADIO_MEMORY_FULLBANK:
		{
			config.memoryMode = MPU_BANK_SWITCHING;
			break;
		}

		case RADIO_MEMORY_TRANSPARENT:
		default:
		{
			config.memoryMode = MPU_TRANSPARENT_ROM;
			break;
		}

		case RADIO_MEMORY_PLAYSID:
		{
			config.memoryMode = MPU_PLAYSID_ENVIRONMENT;
			break;
		}
	}

	switch (sidSettings->GetIntEntryValue("Speed", "ClockSpeed"))
	{
		case RADIO_SPEED_PAL:
		default:
		{
			config.clockSpeed = SIDTUNE_CLOCK_PAL;
			break;
		}

		case RADIO_SPEED_NTSC:
		{
			config.clockSpeed = SIDTUNE_CLOCK_NTSC;
			break;
		}
	}

	if (sidSettings->GetStringEntryValue("General", "ForceSongSpeed").CompareNoCase("Yes") == 0)
		flag = true;
	else
		flag = false;

	config.forceSongSpeed = flag;

	config.digiPlayerScans = sidSettings->GetIntEntryValue("Misc", "DigiScan") * 50;

	panningTab[0] = sidSettings->GetIntEntryValue("Panning", "Channel1");
	panningTab[1] = sidSettings->GetIntEntryValue("Panning", "Channel2");
	panningTab[2] = sidSettings->GetIntEntryValue("Panning", "Channel3");
	panningTab[3] = sidSettings->GetIntEntryValue("Panning", "Channel4");

	// Validate SID emulator settings
	if ((config.autoPanning != SIDEMU_NONE) && (config.channels == SIDEMU_MONO))
		config.channels = SIDEMU_STEREO;

	if ((config.autoPanning != SIDEMU_NONE) && (config.volumeControl == SIDEMU_NONE))
		config.volumeControl = SIDEMU_FULLPANNING;

	emuEngine->SetConfig(config);
}
