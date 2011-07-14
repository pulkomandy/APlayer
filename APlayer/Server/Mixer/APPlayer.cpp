/******************************************************************************/
/* APlayer player class.                                                      */
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
#include "PTime.h"
#include "PList.h"
#include "PSynchronize.h"

// APlayerKit headers
#include "APAddOns.h"

// Server headers
#include "APModuleLoader.h"
#include "APPlayer.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APPlayer::APPlayer(void) : BLooper("Player Looper")
{
	// Initialize member variables
	songNum       = 0;
	songLength    = 0;
	moduleSize    = 0;

	infoLock      = NULL;

	playerLock    = NULL;
	currentPlayer = NULL;

	// Start the BLooper
	Run();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APPlayer::~APPlayer(void)
{
}



/******************************************************************************/
/* InitPlayer() will initialize the player and mixer.                         */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "result" is a reference to store the error string if any.          */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APPlayer::InitPlayer(APFileHandle handle, PString &result)
{
	bool initOk = true;

	try
	{
		// Remember the player
		currentPlayer = handle.loader->GetPlayer(playerIndex);

		// Remember the module length
		moduleSize = handle.loader->GetModuleSize();

		// Initialize other stuff
		moduleFormat = handle.loader->GetModuleFormat();
		playerName   = handle.loader->GetPlayerName();

		currentPlayer->mixerFreq = handle.mixerFrequency;

		// Set the message looper
		currentPlayer->SetLooper(this);

		// Create the information list lock
		infoLock = new PMutex("Information Lock", false);
		if (infoLock == NULL)
			throw PMemoryException();

		// Create the mutex used to lock some player calls
		playerLock = new PMutex("Player Lock", false);
		if (playerLock == NULL)
			throw PMemoryException();

		// Call the InitPlayer() function
		initOk = currentPlayer->InitPlayer(playerIndex);

		if (initOk)
		{
			// Add all extra files loaded file size to the module length
			moduleSize += currentPlayer->GetExtraFilesSize();

			// Fill out the sample list
			GetSamples();

			// Find author
			author = FindAuthor();

			// Find number of subsongs
			const uint16 *songs = currentPlayer->GetSubSongs();

			// Copy the table
			subSongs[0] = songs[0];
			subSongs[1] = songs[1];

			// Reset the song length
			songLength = 0;

			// Get the position times for the default song
			posTimes.MakeEmpty();
			totalTime = currentPlayer->GetTimeTable(subSongs[1], posTimes);

			// Initialize the mixer
			initOk = mixer.InitMixer(handle, playerLock, this, result);

			if (!initOk)
				EndPlayer();
		}
		else
			EndPlayer();
	}
	catch(...)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_PLAYER_INIT);
		initOk = false;
	}

	return (initOk);
}



/******************************************************************************/
/* EndPlayer() will end the player from playing.                              */
/******************************************************************************/
void APPlayer::EndPlayer(void)
{
	try
	{
		if (currentPlayer != NULL)
		{
			// End the mixer
			mixer.EndMixer();

			// Call the EndPlayer() function
			currentPlayer->EndPlayer(playerIndex);

			// Free the sample list
			FreeSamples();
			currentPlayer = NULL;

			// Clear player information
			songNum    = 0;
			songLength = 0;
			author.MakeEmpty();
			totalTime.SetTimeSpan(0);
			posTimes.MakeEmpty();

			moduleFormat.MakeEmpty();
			playerName.MakeEmpty();
			moduleSize = 0;
			moduleInfo.MakeEmpty();
		}

		// Delete the player lock
		delete playerLock;
		playerLock = NULL;

		// Delete the information lock
		delete infoLock;
		infoLock = NULL;
	}
	catch(...)
	{
		;
	}
}



/******************************************************************************/
/* StartPlaying() will start to play the song given.                          */
/*                                                                            */
/* Input:  "song" is the song number to play starting from 0. -1 means the    */
/*         default start song.                                                */
/******************************************************************************/
void APPlayer::StartPlaying(int16 song)
{
	int32 i;
	PString description, value;

	// Remember the song number
	if (song == -1)
		songNum = subSongs[1];
	else
		songNum = song;

	// Initialize other useful variables
	currentPlayer->playFreq   = 50.0f;
	currentPlayer->endReached = false;

	// Lock the player
	playerLock->Lock();

	// Call the InitSound function
	currentPlayer->InitSound(playerIndex, songNum);

	// Call the song length function
	songLength = currentPlayer->GetSongLength();

	// Get the position times for the current song
	posTimes.MakeEmpty();
	totalTime = currentPlayer->GetTimeTable(songNum, posTimes);

	// Get module information
	moduleInfo.MakeEmpty();
	for (i = 0; currentPlayer->GetInfoString(i, description, value); i++)
	{
		// Make sure we don't have any valid characters
		description.Replace('\t', ' ');
		description.Replace('\n', ' ');
		value.Replace('\t', ' ');
		value.Replace('\n', ' ');

		// Build and the information in the list
		moduleInfo.AddTail(description + "\t" + value + "\n");
	}

	// Unlock again
	playerLock->Unlock();

	// Start the mixer
	mixer.StartMixer();
	ResumePlaying();
}



/******************************************************************************/
/* StopPlaying() will stop the playing.                                       */
/******************************************************************************/
void APPlayer::StopPlaying(void)
{
	if (currentPlayer != NULL)
	{
		// Stop the mixer
		PausePlaying();
		mixer.StopMixer();

		// Lock the player
		playerLock->Lock();

		// Call the EndSound function
		currentPlayer->EndSound(playerIndex);

		// Unlock again
		playerLock->Unlock();
	}
}



/******************************************************************************/
/* PausePlaying() will pause the playing.                                     */
/******************************************************************************/
void APPlayer::PausePlaying(void)
{
	mixer.PausePlaying();
}



/******************************************************************************/
/* ResumePlaying() will continue the playing again.                           */
/******************************************************************************/
void APPlayer::ResumePlaying(void)
{
	mixer.ResumePlaying();
}



/******************************************************************************/
/* HoldPlaying() will hold the playing.                                       */
/*                                                                            */
/* Input:  "hold" is the hold flag. True for holding, false for playing.      */
/******************************************************************************/
void APPlayer::HoldPlaying(bool hold)
{
	mixer.HoldPlaying(hold);
}



/******************************************************************************/
/* CanChangePosition() returns true if the player can change to an absolute   */
/*      position, else false.                                                 */
/*                                                                            */
/* Output: True is the player can change position.                            */
/******************************************************************************/
bool APPlayer::CanChangePosition(void) const
{
	uint32 flags;

	flags = currentPlayer->GetSupportFlags(playerIndex);

	if (flags & appSetPosition)
		return (true);

	return (false);
}



/******************************************************************************/
/* SetVolume() sets the new master volume.                                    */
/*                                                                            */
/* Input:  "volume" is the new volume between 0 and 256.                      */
/******************************************************************************/
void APPlayer::SetVolume(uint16 volume)
{
	mixer.SetVolume(volume);
}



/******************************************************************************/
/* SetStereoSeparation() sets the new stereo separation.                      */
/*                                                                            */
/* Input:  "sep" is the new stereo separation in percent.                     */
/******************************************************************************/
void APPlayer::SetStereoSeparation(uint16 sep)
{
	mixer.SetStereoSeparation(sep);
}



/******************************************************************************/
/* SetMixerMode() will change the mixer mode.                                 */
/*                                                                            */
/* Input:  "mode" is the new mixer mode flags to enable or disable.           */
/*         "enable" is true if you want to enable the flags, else false.      */
/******************************************************************************/
void APPlayer::SetMixerMode(uint32 mode, bool enable)
{
	mixer.SetMixerMode(mode, enable);
}



/******************************************************************************/
/* EnableAmigaFilter() will enable or disable the Amiga filter emulation.     */
/*                                                                            */
/* Input:  "enable" is true if you want to enable the emulation, else false.  */
/******************************************************************************/
void APPlayer::EnableAmigaFilter(bool enable)
{
	mixer.EnableAmigaFilter(enable);
}



/******************************************************************************/
/* ChangeChannels() will change the channels given.                           */
/*                                                                            */
/* Input:  "enable" is true if you want to enable the channels, else false.   */
/*         "startChan" is the channel you want to change.                     */
/*         "stopChan" is the last channel you want to change or -1 if you     */
/*         only want to change one.                                           */
/******************************************************************************/
void APPlayer::ChangeChannels(bool enable, int16 startChan, int16 stopChan)
{
	if (stopChan == -1)
		mixer.EnableChannel(startChan, enable);
	else
	{
		for (; startChan <= stopChan; startChan++)
			mixer.EnableChannel(startChan, enable);
	}
}



/******************************************************************************/
/* GetModuleName() returns the name of the module.                            */
/*                                                                            */
/* Output: The module name.                                                   */
/******************************************************************************/
PString APPlayer::GetModuleName(void) const
{
	PString name;
	int32 i, count;
	bool ok = false;

	// Is the player still available?
	if (currentPlayer == NULL)
		return ("");

	// Get the name of the module
	name = currentPlayer->GetModuleName();

	// Check to see if the module name only have spaces
	count = name.GetLength();
	for (i = 0; i < count; i++)
	{
		if (name.GetAt(i) != ' ')
		{
			ok = true;
			break;
		}
	}

	if (!ok)
		name.MakeEmpty();
	else
	{
		name.TrimRight();
		name.TrimLeft();
	}

	return (name);
}



/******************************************************************************/
/* GetAuthor() returns the name of the author.                                */
/*                                                                            */
/* Output: The author.                                                        */
/******************************************************************************/
PString APPlayer::GetAuthor(void) const
{
	return (author);
}



/******************************************************************************/
/* GetCurrentSong() returns the current playing subsong number.               */
/*                                                                            */
/* Output: The playing song number.                                           */
/******************************************************************************/
uint16 APPlayer::GetCurrentSong(void) const
{
	return (songNum);
}



/******************************************************************************/
/* GetMaxSong() returns the maximum subsong number.                           */
/*                                                                            */
/* Output: The maximum song number.                                           */
/******************************************************************************/
uint16 APPlayer::GetMaxSongs(void) const
{
	return (subSongs[0]);
}



/******************************************************************************/
/* GetChannels() returns the number of channels the module use.               */
/*                                                                            */
/* Output: The number of channels or 0 if it can't be found.                  */
/******************************************************************************/
uint16 APPlayer::GetChannels(void) const
{
	if (currentPlayer == NULL)
		return (0);

	return (currentPlayer->GetModuleChannels());
}



/******************************************************************************/
/* GetSongLength() returns the song length.                                   */
/*                                                                            */
/* Output: The song length or 0 if it can't be found.                         */
/******************************************************************************/
int16 APPlayer::GetSongLength(void) const
{
	return (songLength);
}



/******************************************************************************/
/* GetSongPosition() returns the current song position.                       */
/*                                                                            */
/* Output: The song position or -1 if it can't be found.                      */
/******************************************************************************/
int16 APPlayer::GetSongPosition(void) const
{
	int16 pos;

	// Is the player still available?
	if (currentPlayer == NULL)
		return (-1);

	// If the mixer uses ring buffer, don't report positions here
	if (mixer.UsingRingBuffers())
		return (-1);

	// Lock the player
	playerLock->Lock();

	// Get the position
	pos = currentPlayer->GetSongPosition();

	// Unlock again
	playerLock->Unlock();

	return (pos);
}



/******************************************************************************/
/* SetSongPosition() sets a new song position.                                */
/*                                                                            */
/* Input:  "pos" is the song position.                                        */
/******************************************************************************/
void APPlayer::SetSongPosition(int16 pos)
{
	if (currentPlayer != NULL)
	{
		// Set the position in the player
		playerLock->Lock();
		currentPlayer->SetSongPosition(pos);
		playerLock->Unlock();

		// Tell the mixer about the position change
		mixer.SetSongPosition(pos);
	}
}



/******************************************************************************/
/* GetTotalTime() returns the total time of the current song.                 */
/*                                                                            */
/* Output: The total time.                                                    */
/******************************************************************************/
PTimeSpan APPlayer::GetTotalTime(void) const
{
	return (totalTime);
}



/******************************************************************************/
/* GetTimeList() returns the position time list of the current song.          */
/*                                                                            */
/* Output: The position time list.                                            */
/******************************************************************************/
const PList<PTimeSpan> *APPlayer::GetTimeList(void) const
{
	return (&posTimes);
}



/******************************************************************************/
/* GetModuleFormat() returns the format of the module.                        */
/*                                                                            */
/* Output: The module format.                                                 */
/******************************************************************************/
PString APPlayer::GetModuleFormat(void) const
{
	return (moduleFormat);
}



/******************************************************************************/
/* GetPlayerName() returns the name of the player.                            */
/*                                                                            */
/* Output: The player name.                                                   */
/******************************************************************************/
PString APPlayer::GetPlayerName(void) const
{
	return (playerName);
}



/******************************************************************************/
/* GetModuleSize() returns the size of the module.                            */
/*                                                                            */
/* Output: The module size.                                                   */
/******************************************************************************/
uint32 APPlayer::GetModuleSize(void) const
{
	return (moduleSize);
}



/******************************************************************************/
/* GetModuleInformation() returns the module information list of the current  */
/*      song.                                                                 */
/*                                                                            */
/* Output: The module information list.                                       */
/******************************************************************************/
const PList<PString> *APPlayer::GetModuleInformation(void) const
{
	return (&moduleInfo);
}



/******************************************************************************/
/* GetInstrumentList() will lock the list with the instruments and return a   */
/*      pointer to it.                                                        */
/*                                                                            */
/* Output: A pointer to the list.                                             */
/******************************************************************************/
const PList<APInstInfo *> *APPlayer::GetInstrumentList(void)
{
	// Lock the list
	infoLock->Lock();

	// Return the list pointer
	return (&instruments);
}



/******************************************************************************/
/* UnlockInstrumentList() will unlock the list with the instruments.          */
/******************************************************************************/
void APPlayer::UnlockInstrumentList(void)
{
	// Unlock the list
	infoLock->Unlock();
}



/******************************************************************************/
/* GetSampleList() will lock the list with the samples and return a pointer   */
/*      to it.                                                                */
/*                                                                            */
/* Output: A pointer to the list.                                             */
/******************************************************************************/
const PList<APSampleInfo *> *APPlayer::GetSampleList(void)
{
	// Lock the list
	infoLock->Lock();

	// Return the list pointer
	return (&samples);
}



/******************************************************************************/
/* UnlockSampleList() will unlock the list with the samples.                  */
/******************************************************************************/
void APPlayer::UnlockSampleList(void)
{
	// Unlock the list
	infoLock->Unlock();
}



/******************************************************************************/
/* DisableVirtualMixer() will disable the virtual mixer given.                */
/*                                                                            */
/* Input:  "agent" is the agent with the virtual mixer to disable.            */
/******************************************************************************/
void APPlayer::DisableVirtualMixer(AddOnInfo *agent)
{
	mixer.DisableVirtualMixer(agent);
}



/******************************************************************************/
/* MessageReceived() is called when the player sends some information.        */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APPlayer::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		//
		// The position in the player has changed
		//
		case AP_POSITION_CHANGED:
		{
			int16 newPos;

			// Get the new position
			newPos = GetSongPosition();

			// Send the new position to all the clients
			if (newPos != -1)
				SendNewPosition(newPos);
			break;
		}

		//
		// Some module information has changed
		//
		case AP_MODULEINFO_CHANGED:
		{
			const char *value;
			int32 line;

			// Extract the information
			if (msg->FindInt32("line", &line) == B_OK)
			{
				if (msg->FindString("value", &value) == B_OK)
				{
					// Send the information to all the clients
					SendNewInformation(line, value);
				}
			}
			break;
		}

		//
		// The position in the player has changed via
		// the ring buffer system
		//
		case AP_REPORT_POSITION:
		{
			int16 newPos;

			if (msg->FindInt16("position", &newPos) == B_OK)
				SendNewPosition(newPos);
			break;
		}

		//
		// The module has ended
		//
		case AP_MODULE_ENDED:
		{
			// Send the module ended to all the clients
			SendModuleEnded();
			break;
		}

		default:
			BLooper::MessageReceived(msg);
			break;
	}
}



/******************************************************************************/
/* SendNewPosition() will build and send an "NewPosition" command to all the  */
/*      clients.                                                              */
/*                                                                            */
/* Input:  "position" is the new position.                                    */
/******************************************************************************/
void APPlayer::SendNewPosition(int16 position)
{
	PString command;

	// Build the command
	command = APServerCommunication::AddArgument("NewPosition=", PString::CreateNumber(position));

	// And send it to all the clients
	GetApp()->client->SendCommand(this, command);
}



/******************************************************************************/
/* SendNewInformation() will build and send an "NewInformation" command to    */
/*      all the clients.                                                      */
/*                                                                            */
/* Input:  "line" is the line that has changed.                               */
/*         "value" is the new value.                                          */
/******************************************************************************/
void APPlayer::SendNewInformation(int32 line, PString value)
{
	PString command;

	// Build the command
	command = APServerCommunication::AddArgument("NewInformation=", PString::CreateNumber(line));
	command = APServerCommunication::AddArgument(command, value);

	// And send it to all the clients
	GetApp()->client->SendCommand(this, command);
}



/******************************************************************************/
/* SendModuleEnded() will build and send a "ModuleEnded" command to all the   */
/*      clients.                                                              */
/******************************************************************************/
void APPlayer::SendModuleEnded(void)
{
	// Send the command to all the clients
	GetApp()->client->SendCommand(this, "ModuleEnded=");
}



/******************************************************************************/
/* FindAuthor() returns the author of the module.                             */
/*                                                                            */
/* Output: The author or an empty string if not found.                        */
/******************************************************************************/
PString APPlayer::FindAuthor(void)
{
	PString name;
	int32 i, count;

	// Get the author the player returns
	name = currentPlayer->GetAuthor();

	if (!name.IsEmpty())
	{
		bool ok = false;

		// Check to see if the name only have spaces
		count = name.GetLength();
		for (i = 0; i < count; i++)
		{
			if (name.GetAt(i) != ' ')
			{
				ok = true;
				break;
			}
		}

		if (!ok)
			name.MakeEmpty();
	}

	// We didn't get any author, now scan the instruments/samples
	// after an author
	if (name.IsEmpty())
	{
		PList<PString> nameList;
		APInstInfo instInfo;
		APSampleInfo sampInfo;

		for (i = 0; ; i++)
		{
			// Call the player to fill out the structure
			if (!currentPlayer->GetInstrumentInfo(i, &instInfo))
				break;

			// Fill the list with the instrument name
			nameList.AddTail(instInfo.name);
		}

		name = FindAuthorInList(nameList);
		if (name.IsEmpty())
		{
			// No author found in the instrument names, now try the samples
			nameList.MakeEmpty();

			for (i = 0; ; i++)
			{
				// Call the player to fill out the structure
				if (!currentPlayer->GetSampleInfo(i, &sampInfo))
					break;

				// Fill the list with the sample name
				nameList.AddTail(sampInfo.name);
			}

			name = FindAuthorInList(nameList);
		}
	}

	// Trim the name
	name.TrimRight();
	name.TrimLeft();

	return (name);
}



/******************************************************************************/
/* FindAuthorInList() tries to find the author in a list of names.            */
/*                                                                            */
/* Input:  "list" is the list with all the text to search.                    */
/*                                                                            */
/* Output: The author or an empty string if not found.                        */
/******************************************************************************/
PString APPlayer::FindAuthorInList(PList<PString> &list)
{
	PString itemStr, name;
	PChar chr;
	int32 i, count;
	int32 pos = -1;
	int32 startPos = -1;

	// First get the number of items in the list
	count = list.CountItems();

	// Traverse all the names
	for (i = 0; i < count; i++)
	{
		// Get the string to search in
		itemStr = list.GetItem(i);

		// If the string is empty, we don't need to do a search :)
		if (itemStr.IsEmpty())
			continue;

		// Try to find a "by" word
		pos = FindBy(itemStr);
		if (pos != -1)
			break;			// We found one, stop the loop
	}

	if (pos != -1)
	{
		// Now try to find the author, search trough the current
		// string to the end of the list
		for (;;)
		{
			// Scan each character in the rest of the string
			for (; pos < itemStr.GetLength(); pos++)
			{
				if (itemStr.GetAt(pos).IsAlphaNum())
				{
					startPos = pos;
					break;
				}
			}

			// Got a start position, break the loop
			if (startPos != -1)
				break;

			// Get next line
			i++;
			if (i == count)
				break;

			itemStr = list.GetItem(i);
			pos = 0;
		}
	}
	else
	{
		// We didn't find a "by" word, try to find other author marks
		for (i = 0; i < count; i++)
		{
			// Get the string to search in
			itemStr = list.GetItem(i);

			// If the string is empty, we don't need to do a search :)
			if (itemStr.IsEmpty())
				continue;

			// See if there is the traditional '#' character
			if (itemStr.GetAt(0) == '#')
			{
				startPos = 1;
				break;
			}

			// Is there a ">>>" mark?
			if (itemStr.Left(3) == ">>>")
			{
				startPos = 3;
				break;
			}

			// What about the ">>" mark?
			if (itemStr.Left(2) == ">>")
			{
				startPos = 2;
				break;
			}

			// Is there a "?>>>" mark?
			if (itemStr.Mid(1, 3) == ">>>")
			{
				startPos = 4;
				break;
			}

			// What about the "?>>" mark?
			if (itemStr.Mid(1, 2) == ">>")
			{
				startPos = 3;
				break;
			}
		}

		if (startPos != -1)
		{
			// See if the is a "by" word after the mark
			pos = FindBy(itemStr.Mid(startPos));
			if (pos != -1)
				startPos = pos;
		}
	}

	if (startPos != -1)
	{
		// Got the start position of the author, now find the end position
		name = itemStr.Mid(startPos);
		name.TrimRight();
		name.TrimLeft();

		for (pos = 0; pos < name.GetLength(); pos++)
		{
			// Get the current character
			chr = name.GetAt(pos);

			// Check for legal characters
			if (((chr == ' ') || (chr == '!') || (chr == '\'') || (chr == '-') ||
				(chr == '/')) || (chr.IsDigit()))
			{
				// It's legal, go to the next character
				continue;
			}

			// Check to see if the & character is the last one and if
			// not, it's legal
			if ((chr == '&') && ((pos + 1) < name.GetLength()))
				continue;

			if (chr == '.')
			{
				// The point is the last character
				if ((pos + 1) == name.GetLength())
					break;

				// Are there a space or another point after the first one?
				if ((name.GetAt(pos + 1) == ' ') || (name.GetAt(pos + 1) == '.'))
					break;

				continue;
			}

			// Is the character a letter?
			if (!chr.IsAlpha())
			{
				// No, stop the loop
				break;
			}

			// Stop if .... of
			if ((pos + 1) < name.GetLength())
			{
				if ((chr == 'o') && (name.GetAt(pos + 1) == 'f') &&
					((pos + 2) == name.GetLength()))
				{
					if ((pos > 0) && (name.GetAt(pos - 1) == ' '))
						break;
				}
			}

			// Stop if .... from
			if ((pos + 3) < name.GetLength())
			{
				if ((chr == 'f') && (name.GetAt(pos + 1) == 'r') &&
					(name.GetAt(pos + 2) == 'o') && (name.GetAt(pos + 3) == 'm') &&
					((pos + 4) == name.GetLength()))
				{
					if ((pos > 0) && (name.GetAt(pos - 1) == ' '))
						break;
				}
			}

			// Stop if .... in
			if ((pos + 1) < name.GetLength())
			{
				if ((chr == 'i') && (name.GetAt(pos + 1) == 'n') &&
					((pos + 2) == name.GetLength()))
				{
					if ((pos > 0) && (name.GetAt(pos - 1) == ' '))
						break;
				}
			}

			// Stop if .... and
			if ((pos + 2) < name.GetLength())
			{
				if ((chr == 'a') && (name.GetAt(pos + 1) == 'n') &&
					(name.GetAt(pos + 2) == 'd') &&
					((pos + 3) == name.GetLength()))
				{
					if ((pos > 0) && (name.GetAt(pos - 1) == ' '))
						break;
				}
			}
		}

		// Clip out the author
		name = name.Left(pos);
		name.TrimRight();

		// Check for some special characters that needs to be removed
		if (!name.IsEmpty())
		{
			for (;;)
			{
				chr = name.GetAt(name.GetLength() - 1);
				if (chr != '-')
					break;

				if (name.GetAt(0) == chr)
					break;

				name.Delete(name.GetLength() - 1);
			}
		}
	}

	return (name);
}



/******************************************************************************/
/* FindBy() will look in the string given after the "by" word and return the  */
/*      index where it found it.                                              */
/*                                                                            */
/* Input:  "str" is the string to search in.                                  */
/*                                                                            */
/* Output: The index right after the by word or -1 if not found.              */
/******************************************************************************/
int32 APPlayer::FindBy(PString str)
{
	int32 index = 0;
	bool found = false;

	while (index < (str.GetLength() - 1))
	{
		if (((str.GetAt(index) == 'b') || (str.GetAt(index) == 'B')) &&
			((str.GetAt(index + 1) == 'y') || (str.GetAt(index + 1) == 'Y')))
		{
			// Check to see if the character before "by" is a letter
			if ((index > 0) && (str.GetAt(index - 1).IsAlpha()))
			{
				index++;
				continue;
			}

			// Check to see if the character after "by" is a letter
			if (((index + 2) < str.GetLength()) && (str.GetAt(index + 2).IsAlpha()))
			{
				index++;
				continue;
			}

			if ((index + 2) == str.GetLength())
			{
				// The last word in the string was "by", so we found it
				return (index + 2);
			}

			index += 2;
			found = true;
			break;
		}

		// Go to the next character
		index++;
	}

	// Did we found the "by" word?
	if (found)
	{
		// Yep, check if it's "by" some known phrases we need to ignore
		if (str.Mid(index + 1, 4) == "KIWI")
			return (-1);

		if (str.Mid(index + 1, 10) == "the welder")
			return (-1);

		if (str.Mid(index + 1, 6) == "e-mail")
			return (-1);

		if (str.Mid(index + 1, 6) == "Gryzor")
			return (-1);
	}
	else
	{
		// Okay, now try to find "(c)"
		index = 0;
		while (index < (str.GetLength() - 2))
		{
			if ((str.GetAt(index) == '(') &&
				((str.GetAt(index + 1) == 'c') || (str.GetAt(index + 1) == 'C')) &&
				(str.GetAt(index + 2) == ')'))
			{
				index += 3;
				found = true;
				break;
			}

			// Go to the next character
			index++;
		}
	}

	if (found)
	{
		// Find the first letter in author
		for (; index < str.GetLength(); index++)
		{
			if (str.GetAt(index) < '0')
				continue;

			if ((str.GetAt(index) <= '9') || (str.GetAt(index) >= 'A'))
				break;
		}

		return (index);
	}

	return (-1);
}



/******************************************************************************/
/* GetSamples() will find all the samples and fill out the sample list.       */
/******************************************************************************/
void APPlayer::GetSamples(void)
{
	uint32 i;
	APInstInfo *inst;
	APSampleInfo *sample;

	// Lock the lists
	infoLock->Lock();

	// Instruments
	//
	// Traverse throu all the instruments
	for (i = 0; ; i++)
	{
		// Allocate the instrument structure
		inst = new APInstInfo;
		if (inst == NULL)
			throw PMemoryException();

		// Call the player to fill out the structure
		if (currentPlayer->GetInstrumentInfo(i, inst))
		{
			// And add it in the list
			instruments.AddTail(inst);
		}
		else
		{
			// No more instruments, stop the loop
			delete inst;
			break;
		}
	}

	// Samples
	//
	// Traverse throu all the samples
	for (i = 0; ; i++)
	{
		// Allocate the sample structure
		sample = new APSampleInfo;
		if (sample == NULL)
			throw PMemoryException();

		// Call the player to fill out the structure
		if (currentPlayer->GetSampleInfo(i, sample))
		{
			// And add it in the list
			samples.AddTail(sample);
		}
		else
		{
			// No more samples, stop the loop
			delete sample;
			break;
		}
	}

	// Unlock the lists again
	infoLock->Unlock();
}



/******************************************************************************/
/* FreeSamples() will free all the items in the sample list.                  */
/******************************************************************************/
void APPlayer::FreeSamples(void)
{
	uint32 count, i;
	APInstInfo *inst;
	APSampleInfo *sample;

	// Lock the lists
	infoLock->Lock();

	// Samples
	//
	// Get the number of samples
	count = samples.CountItems();

	// Traverse throu all the samples and free the items
	for (i = 0; i < count; i++)
	{
		// Get the current item
		sample = samples.GetItem(i);

		// Delete the item
		delete sample;
	}

	// Empty the list
	samples.MakeEmpty();

	// Instruments
	//
	// Get the number of instruments
	count = instruments.CountItems();

	// Traverse throu all the instruments and free the items
	for (i = 0; i < count; i++)
	{
		// Get the current item
		inst = instruments.GetItem(i);

		// Delete the item
		delete inst;
	}

	// Empty the list
	instruments.MakeEmpty();

	// Unlock the lists again
	infoLock->Unlock();
}
