/*
game_music_emu Player Interface.
Copyright 2012, Adrien Destugues <pulkomandy@pulkomandy.tk>
This file is distributed under the terms of the MIT licence.
*/


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
#include "GMEPlayer.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define PlayerVersion			1.0



/******************************************************************************/
/* Other constants                                                            */
/******************************************************************************/
#define GME_Version			"0.5.5"

#define DEFAULT_BUFFER_SIZE		8192



/******************************************************************************/
/* Extern variables                                                           */
/******************************************************************************/



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
GMEPlayer::GMEPlayer(APGlobalData *global, PString fileName) : APAddOnPlayer(global)
{
	// Fill out the version we have compiled under
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
GMEPlayer::~GMEPlayer(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float GMEPlayer::GetVersion(void)
{
	return (PlayerVersion);
}



/******************************************************************************/
/* GetConfigInfo() will return a pointer to a config structure.               */
/*                                                                            */
/* Output: Is a pointer to a config structure.                                */
/******************************************************************************/
const APConfigInfo *GMEPlayer::GetConfigInfo(void)
{
	// TODO there are various opportunities for a config dialog. Messing up with
	// volume tables, AY stereo mode, clock, mixing and whatnot
	return NULL;

	// Create background view
	/*
	cfgInfo.view     = new SIDView(res);
	cfgInfo.settings = sidSettings;
	cfgInfo.fileName = "SidPlay.ini";
	return (&cfgInfo);
	*/
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 GMEPlayer::GetSupportFlags(int32 /*index*/)
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
PString GMEPlayer::GetName(int32 /*index*/)
{
	PString name;

	name.LoadString(res, IDS_GME_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString GMEPlayer::GetDescription(int32 /*index*/)
{
	PString description;

	description.Format(res, IDS_GME_DESCRIPTION, GME_Version);
	return (description);
}



/******************************************************************************/
/* GetModTypeString() returns the module type string for the current player.  */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: The module type string.                                            */
/******************************************************************************/
PString GMEPlayer::GetModTypeString(int32 /*index*/)
{
	PString type;

	type.LoadString(res, IDS_GME_MIME);
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
ap_result GMEPlayer::ModuleCheck(int32 /*index*/, PFile* file)
{
	// TODO use the index to separate the subtypes of players (AY, SPC, ...)
	gme_type_t format = 0;
	gme_err_t error = gme_identify_file(file->GetFullPath().GetString(), &format);

	if (format != 0 && error == NULL) {
		return AP_OK;
	} else {
		return AP_UNKNOWN;
	}
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
ap_result GMEPlayer::LoadModule(int32 /*index*/, PFile* file,
	PString& errorStr)
{
	ap_result retVal = AP_OK;

	gme_err_t error = gme_open_file(file->GetFullPath().GetString(), &theEmu,
		44100 /* TODO get actual baudrate from config */);
	
	if (error != NULL)
	{
		errorStr.SetString(error);
		gme_delete(theEmu);
		return AP_ERROR;
	}

	gme_track_info(theEmu, &fSongInfos, 0);

	return (retVal);
}



/******************************************************************************/
/* InitPlayer() initialize the player.                                        */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool GMEPlayer::InitPlayer(int32 /*index*/)
{
	//int32 oldIndex, newIndex;
	bool retVal;

	try
	{
		// Create the SID objects
		/*
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
*/
		// Okay, set the configuration data
		InitConfig();
//		emuEngine->GetConfig(config);
		mixerFreq = 44100; //TODO use default value from APlayer config!

		// Allocate sample buffers
		buffer = new uint8[DEFAULT_BUFFER_SIZE];
		if (buffer == NULL)
			throw PUserException();

		// Allocate converter buffers
		// TODO we should allocate one for each channel of the emulated device
		// and perfrm the mixing ourselves (or use GME code if possible, but
		// still making the channels visible to APlayer, for the spinning 
		// squares and whatnot)
		memset(chanBuffers, 0, sizeof(int16 *) * 2);

		for (int i = 0; i < 2; i++)
		{
			chanBuffers[i] = new int16[DEFAULT_BUFFER_SIZE / 4];
			if (chanBuffers[i] == NULL)
				throw PMemoryException();
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
void GMEPlayer::EndPlayer(int32 /*index*/)
{
	// Cleanup the channel buffers
	if (chanBuffers != NULL)
	{
		for (int i = 0; i < 2; i++)
			delete[] chanBuffers[i];
	}

	gme_free_info(fSongInfos);
	gme_delete(theEmu);

	Cleanup();
}



/******************************************************************************/
/* InitSound() initialize the current song.                                   */
/*                                                                            */
/* Input:  "index" is the player index number.                                */
/*         "songNum" is the subsong to play.                                  */
/******************************************************************************/
void GMEPlayer::InitSound(int32 /*index*/, uint16 songNum)
{
	gme_free_info(fSongInfos);
	gme_track_info(theEmu, &fSongInfos, songNum);
	gme_set_fade(theEmu, fSongInfos->play_length);

	gme_start_track(theEmu, songNum);
}



/******************************************************************************/
/* Play() is the main player function.                                        */
/******************************************************************************/
void GMEPlayer::Play(void)
{
	uint32 size, i;
	APChannel *channel;
	int16 pos;

	// Fill out the sample buffers
	// The function want the total size in bytes
	//emuEngine->sid6581.EmuFillBuffer(*emuEngine, *tune, buffer, DEFAULT_BUFFER_SIZE);
	gme_play(theEmu, DEFAULT_BUFFER_SIZE / 2, (short int*)buffer);

	// Check for position change
	pos = GetSongPosition();
	if (pos != oldPos)
	{
		oldPos = pos;
		ChangePosition();
	}

	// Check for end
	if (gme_track_ended(theEmu) || pos >= fSongInfos->play_length / timeScale)
	{
		// Seek back to the start of the file
		SetSongPosition(0);
		endReached = true;
		return;
	}
	


	// Calculate the buffer size in samples
	size = DEFAULT_BUFFER_SIZE / 4;		// / 2 because it's 16 bit

	short int* source = (short int*)buffer;
	for (unsigned int j = 0; j < size; j++)
	{
		for(i = 0; i < 2; i++)
		{
			*(chanBuffers[i] + j) = *source++;
		}
	}

	for (i = 0; i < 2; i++)
	{
		channel = virtChannels[i];

		// Fill out the Channel object
		channel->SetBuffer(chanBuffers[i], size);
		channel->SetVolume(256);
		channel->SetPanning(i % 2 ? APPAN_RIGHT : APPAN_LEFT);
	}
}



/******************************************************************************/
/* GetSamplePlayerInfo() will fill out the sample info structure given.       */
/*                                                                            */
/* Input:  "sampInfo" is a pointer to the sample info structure to fill.      */
/******************************************************************************/
void GMEPlayer::GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo)
{
	sampInfo->bitSize   = 16;
	sampInfo->frequency = mixerFreq;
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: Is the module name.                                                */
/******************************************************************************/
PString GMEPlayer::GetModuleName(void)
{
	return fSongInfos->song;
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: Is the author.                                                     */
/******************************************************************************/
PString GMEPlayer::GetAuthor(void)
{
	return fSongInfos->author;	
}



/******************************************************************************/
/* GetSubSongs() returns the number of sub songs the module have.             */
/*                                                                            */
/* Output: Is a pointer to a subsong array.                                   */
/******************************************************************************/
const uint16 *GMEPlayer::GetSubSongs(void)
{
	songTab[0] = gme_track_count(theEmu);		// Number of subsongs
	songTab[1] = 0;	// Default start song
		// TODO do we have better info ?

	return (songTab);
}



/******************************************************************************/
/* GetSongLength() returns the length of the song.                            */
/*                                                                            */
/* Output: The song length.                                                   */
/******************************************************************************/
int16 GMEPlayer::GetSongLength(void)
{
	return fSongInfos->play_length / timeScale;
}



/******************************************************************************/
/* GetSongPosition() returns the current position in the song.                */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 GMEPlayer::GetSongPosition(void)
{
	return gme_tell(theEmu) / timeScale;
}


void GMEPlayer::SetSongPosition(int16 pos)
{
	gme_seek(theEmu, pos * timeScale);
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
PTimeSpan GMEPlayer::GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes)
{
	int32 i;
	
	gme_info_t* infos;
	gme_track_info(theEmu, &infos, songNum);
	int32 totalTime = infos->play_length;
	gme_free_info(infos);

	// Copy the position times
	for (i = 0; i < totalTime / timeScale; i++)
		posTimes.AddTail(i * timeScale);

	return totalTime;
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
bool GMEPlayer::GetInfoString(uint32 line, PString &description, PString &value)
{
	static const uint32 maxLine = 4;

	// Is the line number out of range?
	if (line > maxLine)
		return (false);

	// Find out which line to take
	switch (line)
	{
		case 0:
		{
			description.LoadString(res, IDS_GME_INFODESCLINE0);
			value = fSongInfos->copyright;
			break;
		}

		// TODO this should be the "Module Format"
		case 1:
		{
			description.LoadString(res, IDS_GME_INFODESCLINE1);
			value = fSongInfos->system;
			break;
		}

		case 2:
		{
			description.LoadString(res, IDS_GME_INFODESCLINE2);
			value = fSongInfos->game;
			break;
		}

		case 3:
		{
			description.LoadString(res, IDS_GME_INFODESCLINE3);
			value = fSongInfos->comment;
			break;
		}

		case 4:
		{
			description.LoadString(res, IDS_GME_INFODESCLINE4);
			value = fSongInfos->dumper;
			break;
		}
		// TOOD add missing infos

		line -= 5;
	}

	return (value != NULL);
}



/******************************************************************************/
/* Cleanup() frees all the memory the player have allocated.                  */
/******************************************************************************/
void GMEPlayer::Cleanup(void)
{
	delete[] buffer;
	//delete player;
	//delete tune;
	//delete emuEngine;

	buffer    = NULL;
	//player    = NULL;
	//tune      = NULL;
	//emuEngine = NULL;
}



/******************************************************************************/
/* InitConfig() get the registry values and initialize the player with them.  */
/******************************************************************************/
void GMEPlayer::InitConfig(void)
{
}
