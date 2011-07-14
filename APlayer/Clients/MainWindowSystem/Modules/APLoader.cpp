/******************************************************************************/
/* Module loader Interface.                                                   */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PTime.h"
#include "PList.h"
#include "PAlert.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "MainWindowSystem.h"
#include "APWindowMainListItem.h"
#include "APWindowSampleInfo.h"
#include "APLoader.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APLoader::APLoader(APGlobalData *glob, MainWindowSystem *winSys) : BLooper("MainWindowSystem: Module loader")
{
	// Remember arguments
	global       = glob;
	windowSystem = winSys;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APLoader::~APLoader(void)
{
}



/******************************************************************************/
/* GetFileHandle() will return the first file handle.                         */
/*                                                                            */
/* Input:  "index" is the index to the file handle you want.                  */
/*                                                                            */
/* Output: The file handle number or 0 if no files has been loaded.           */
/******************************************************************************/
uint32 APLoader::GetFileHandle(int32 index)
{
	APModuleItem item;

	// First lock the looper, so we're sure no one else access the
	// module list
	Lock();

	// Get the first item in the module list
	if (loadedFiles.CountItems() <= index)
		item.fileHandle = "0";
	else
		item = loadedFiles.GetItem(index);

	// Unlock the looper again
	Unlock();

	// Return the file handle
	return (item.fileHandle.GetUNumber());
}



/******************************************************************************/
/* GetTotalTimeFromFile() will try to load the file given and find the total  */
/*      time.                                                                 */
/*                                                                            */
/* Input:  "fileName" is the file name to the file to load.                   */
/*                                                                            */
/* Output: The total time or 0 if it couldn't be found.                       */
/******************************************************************************/
PTimeSpan APLoader::GetTotalTimeFromFile(PString fileName)
{
	PTimeSpan time;
	PString result, handle, agent;
	bool changeType;

	// Add the file to the server
	handle = SendAddFile(fileName);
	if (handle.Left(4) == "ERR=")
		return (time);

	// Now load it
	changeType = windowSystem->useSettings->GetStringEntryValue("FileTypes", "ChangeModuleType").CompareNoCase("Yes") == 0;
	result = SendLoadFile(handle, changeType);
	if (result.Left(4) == "ERR=")
	{
		SendRemoveFile(handle);
		return (time);
	}

	// Initialize the player
	result = SendInitPlayer(handle);
	if (result.Left(4) == "ERR=")
	{
		SendUnloadFile(handle);
		SendRemoveFile(handle);
		return (time);
	}

	// Get the total time
	time = SendGetTotalTime(handle);

	// Remove the file again from the server
	SendEndPlayer(handle);
	SendUnloadFile(handle);
	SendRemoveFile(handle);

	return (time);
}



/******************************************************************************/
/* SetTotalTimeOnFile() will add the length attribute to the file if the      */
/*      option to do so is enabled.                                           */
/*                                                                            */
/* Input:  "fileName" is the file name to the file to change.                 */
/*         "totalTime" is the time to set.                                    */
/******************************************************************************/
void APLoader::SetTotalTimeOnFile(PString fileName, PTimeSpan totalTime)
{
	if (windowSystem->useSettings->GetStringEntryValue("Options", "SetLength").CompareNoCase("Yes") == 0)
	{
		// Did the song have any time at all?
		if (totalTime.GetTotalMilliSeconds() != 0)
		{
			BNode node;
			PString timeStr;
			char *buffer, *nameStr;
			int32 length;

			if (node.SetTo((nameStr = fileName.GetString())) == B_OK)
			{
				// Build the time string and retrieve the buffer
				PTimeSpan tempTime((totalTime.GetTotalMilliSeconds() + 500) / 1000 * 1000);
				timeStr.Format("%2Ld:%02d", tempTime.GetTotalMinutes(), tempTime.GetSeconds());
				buffer = timeStr.GetString(&length);

				// Set the attribute
				node.WriteAttr("Audio:Length", B_STRING_TYPE, 0, buffer, length + 1);
				timeStr.FreeBuffer(buffer);
			}

			fileName.FreeBuffer(nameStr);
		}
	}
}



/******************************************************************************/
/* MessageReceived() is called when a new module is about to be loaded or     */
/*      freed.                                                                */
/*                                                                            */
/* Input:  "msg" is a pointer to the message.                                 */
/******************************************************************************/
void APLoader::MessageReceived(BMessage *msg)
{
	APModuleItem modItem;

	switch (msg->what)
	{
		//
		// Load and/or initialize module
		//
		case AP_LOADINIT_MODULE:
		{
			int32 index;
			int16 subSong, startPos;
			int32 mixerFreq, interpol, dolby, stereoSep, filter;
			uint16 startChan, stopChan;
			bool changeType;
			bool noError;
			PString fileName;
			PString result;

			// Start to get the "noError" bool if any
			if (msg->FindBool("noError", &noError) != B_OK)
				noError = false;

			// Should we load a module?
			if (msg->FindInt32("load", &index) == B_OK)
			{
				// Lock the window
				if (windowSystem->mainWin->LockWithTimeout(250000) == B_TIMED_OUT)
					break;

				// Get the item from the list
				modItem.listItem = windowSystem->mainWin->GetListItem(index);
				if (modItem.listItem == NULL)
				{
					windowSystem->mainWin->Unlock();
					break;
				}

				// Get the file name
				fileName = modItem.listItem->GetFileName();

				// Unlock the window
				windowSystem->mainWin->Unlock();

				// Add the file to the server list
				modItem.fileHandle = SendAddFile(fileName);
				if (modItem.fileHandle.Left(4) == "ERR=")
				{
					if (!noError)
						ShowError(IDS_ERR_ADD_TO_LIST, modItem.fileHandle);

					break;
				}

				// Load the module
				changeType = windowSystem->useSettings->GetStringEntryValue("FileTypes", "ChangeModuleType").CompareNoCase("Yes") == 0;
				result = SendLoadFile(modItem.fileHandle, changeType);
				if (result.Left(4) == "ERR=")
				{
					if (!noError)
						ShowError(IDS_ERR_LOAD_FILE, result);

					// Remove the file again from the server
					SendRemoveFile(modItem.fileHandle);
					break;
				}

				// Set mixer settings
				mixerFreq = windowSystem->useSettings->GetIntEntryValue("Mixer", "Frequency");
				interpol  = windowSystem->useSettings->GetStringEntryValue("Mixer", "Interpolation").CompareNoCase("Yes") == 0;
				dolby     = windowSystem->useSettings->GetStringEntryValue("Mixer", "DolbyPrologic").CompareNoCase("Yes") == 0;
				filter    = windowSystem->useSettings->GetStringEntryValue("Mixer", "AmigaFilter").CompareNoCase("Yes") == 0;
				stereoSep = windowSystem->useSettings->GetIntEntryValue("Mixer", "StereoSep");

				result = SendSetMixerSettings(modItem.fileHandle, mixerFreq, interpol, dolby, stereoSep, filter);
				if (result.Left(4) == "ERR=")
				{
					if (!noError)
						ShowError(IDS_ERR_INIT_PLAYER, result);

					// Remove the file again from the server
					SendUnloadFile(modItem.fileHandle);
					SendRemoveFile(modItem.fileHandle);
					break;
				}

				// Set the output agent
				modItem.outputAgent = windowSystem->useSettings->GetStringEntryValue("Mixer", "OutputAgent");

				result = SendSetOutputAgent(modItem.fileHandle, modItem.outputAgent);
				if (result.Left(4) == "ERR=")
				{
					if (!noError)
						ShowError(IDS_ERR_INIT_PLAYER, result);

					// Remove the file again from the server
					SendUnloadFile(modItem.fileHandle);
					SendRemoveFile(modItem.fileHandle);
					break;
				}

				// Tell the server to initialize the module
				result = SendInitPlayer(modItem.fileHandle);
				if (result.Left(4) == "ERR=")
				{
					if (!noError)
						ShowError(IDS_ERR_INIT_PLAYER, result);

					// Remove the file again from the server
					SendUnloadFile(modItem.fileHandle);
					SendRemoveFile(modItem.fileHandle);
					break;
				}

				// Add the new module to the list
				loadedFiles.AddTail(modItem);
			}

			// Should we start playing the module?
			if (msg->FindInt16("play", &subSong) == B_OK)
			{
				// Get the start position if any
				if (msg->FindInt16("startPos", &startPos) != B_OK)
					startPos = -1;

				// Disable all disabled channels
				for (startChan = 0; startChan < MAX_NUM_CHANNELS; startChan++)
				{
					if (!windowSystem->channelsEnabled[startChan])
					{
						for (stopChan = startChan + 1; stopChan < MAX_NUM_CHANNELS; stopChan++)
						{
							if (windowSystem->channelsEnabled[stopChan])
								break;
						}

						result = SendChangeChannels(modItem.fileHandle, false, startChan, stopChan - 1);
						if (result.Left(4) == "ERR=")
						{
							if (!noError)
								ShowError(IDS_ERR_START_PLAYER, result);

							// Remove the file again from the server
							SendUnloadFile(modItem.fileHandle);
							SendRemoveFile(modItem.fileHandle);

							// And remove the file from the list
							loadedFiles.RemoveItem(0);
							break;
						}

						startChan = stopChan;
					}
				}

				// Start to play the module
				StartPlaying(subSong, startPos);
			}
			break;
		}

		//
		// Free the playing module if any
		//
		case AP_FREE_MODULE:
		{
			// Did we have any modules at all loaded?
			if (loadedFiles.CountItems() == 0)
				break;

			// Get the first one
			modItem = loadedFiles.GetHead();

			if (modItem.listItem->IsPlaying())
			{
				// Stop the playing
				SendStopPlayer(modItem.fileHandle);
			}

			// Cleanup the player
			SendEndPlayer(modItem.fileHandle);

			// Unload the module
			SendUnloadFile(modItem.fileHandle);

			// Remove the module from the server list
			SendRemoveFile(modItem.fileHandle);

			// Remove the file from our list
			loadedFiles.RemoveItem(0);

			// Clear other variables
			windowSystem->playerInfo->ResetInfo();

			// Update the other windows
			RefreshWindows();
			break;
		}

		//
		// Free all extra loaded modules
		//
		case AP_FREE_EXTRA:
		{
			int32 i, count;

			count = loadedFiles.CountItems();
			for (i = 1; i < count; i++)
			{
				// Get the item
				modItem = loadedFiles.GetAndRemoveItem(1);

				// Cleanup the player
				SendEndPlayer(modItem.fileHandle);

				// Unload the module
				SendUnloadFile(modItem.fileHandle);

				// Remove the module from the server list
				SendRemoveFile(modItem.fileHandle);
			}
			break;
		}

		//
		// Free all loaded modules
		//
		case AP_FREE_ALL:
		{
			int32 i, count;

			count = loadedFiles.CountItems();
			for (i = 0; i < count; i++)
			{
				// Get the item
				modItem = loadedFiles.GetItem(i);

				// Cleanup the player
				SendEndPlayer(modItem.fileHandle);

				// Unload the module
				SendUnloadFile(modItem.fileHandle);

				// Remove the module from the server list
				SendRemoveFile(modItem.fileHandle);
			}

			// Empty the list
			loadedFiles.MakeEmpty();
			break;
		}

		//
		// Start a new song
		//
		case AP_START_SONG:
		{
			int16 song;

			// Get the song number
			if (msg->FindInt16("song", &song) == B_OK)
			{
				// Did we have any modules at all loaded?
				if (loadedFiles.CountItems() == 0)
					break;

				// Get the first one
				modItem = loadedFiles.GetHead();

				// Stop the playing
				SendStopPlayer(modItem.fileHandle);

				// Now start the new song
				StartPlaying(song);
			}
			break;
		}

		//
		// Pause playing
		//
		case AP_PAUSE_PLAYING:
		{
			// At least one module should be in the list!
			ASSERT(loadedFiles.CountItems() >= 1);

			if (loadedFiles.CountItems() != 0)
			{
				// Get the item to play
				modItem = loadedFiles.GetHead();

				SendPausePlayer(modItem.fileHandle);
				windowSystem->playerInfo->SetPlayFlag(false);
			}
			break;
		}

		//
		// Resume playing
		//
		case AP_RESUME_PLAYING:
		{
			// At least one module should be in the list!
			ASSERT(loadedFiles.CountItems() >= 1);

			if (loadedFiles.CountItems() != 0)
			{
				// Get the item to play
				modItem = loadedFiles.GetHead();

				SendResumePlayer(modItem.fileHandle);
				windowSystem->playerInfo->SetPlayFlag(true);
			}
			break;
		}

		//
		// Hold playing
		//
		case AP_HOLD_PLAYING:
		{
			bool holdValue;

			// Get the hold value
			if (msg->FindBool("hold", &holdValue) == B_OK)
			{
				// At least one module should be in the list!
				ASSERT(loadedFiles.CountItems() >= 1);

				if (loadedFiles.CountItems() != 0)
				{
					// Get the item to hold
					modItem = loadedFiles.GetHead();

					SendHoldPlaying(modItem.fileHandle, holdValue);
				}
			}
			break;
		}

		//
		// Set position
		//
		case AP_SET_POSITION:
		{
			int16 newPos;

			// Get the hold value
			if (msg->FindInt16("newPos", &newPos) == B_OK)
			{
				// At least one module should be in the list!
				ASSERT(loadedFiles.CountItems() >= 1);

				if (loadedFiles.CountItems() != 0)
				{
					// Get the item to hold
					modItem = loadedFiles.GetHead();

					SendSetPosition(modItem.fileHandle, newPos);
					windowSystem->playerInfo->SetSongPosition(newPos);
				}
			}
			break;
		}

		//
		// Set volume
		//
		case AP_SET_VOLUME:
		{
			int16 newVol;

			// Get the volume
			if (msg->FindInt16("newVol", &newVol) == B_OK)
			{
				// At least one module should be in the list!
				if (loadedFiles.CountItems() != 0)
				{
					// Get the item to change
					modItem = loadedFiles.GetHead();

					SendSetVolume(modItem.fileHandle, newVol);
				}
			}
			break;
		}

		//
		// Set mixer settings
		//
		case AP_SET_MIXER:
		{
			int32 interpol, dolby, stereoSep, filter;

			// Get the settings
			if (msg->FindInt32("interpolation", &interpol) != B_OK)
				interpol = -1;

			if (msg->FindInt32("dolby", &dolby) != B_OK)
				dolby = -1;

			if (msg->FindInt32("stereoSep", &stereoSep) != B_OK)
				stereoSep = -1;

			if (msg->FindInt32("filter", &filter) != B_OK)
				filter = -1;

			// At least one module should be in the list!
			if (loadedFiles.CountItems() != 0)
			{
				// Get the item to change
				modItem = loadedFiles.GetHead();

				SendSetMixerSettings(modItem.fileHandle, -1, interpol, dolby, stereoSep, filter);
			}
			break;
		}

		//
		// Change channels
		//
		case AP_SET_CHANNELS:
		{
			bool enabled;
			int16 startChan, stopChan;

			// Get the channel information
			if (msg->FindBool("enabled", &enabled) == B_OK)
			{
				if (msg->FindInt16("startChan", &startChan) == B_OK)
				{
					if (msg->FindInt16("stopChan", &stopChan) != B_OK)
						stopChan = -1;

					// At least one module should be in the list!
					if (loadedFiles.CountItems() != 0)
					{
						// Get the item to change
						modItem = loadedFiles.GetHead();

						SendChangeChannels(modItem.fileHandle, enabled, startChan, stopChan);
					}
				}
			}
			break;
		}

		default:
			BLooper::MessageReceived(msg);
			break;
	}
}



/******************************************************************************/
/* ShowError() will show the server error.                                    */
/*                                                                            */
/* Input:  "resourceError" is a resource string number to show.               */
/*         "error" is the result got from the server.                         */
/******************************************************************************/
void APLoader::ShowError(int32 resourceError, PString error)
{
	BMessage msg(AP_CHANGE_PLAY_ITEM);
	int32 modErr;
	int32 index, count;
	int32 listEnd;
	PAlert::PAlertButton response;

	// Build the deselect message
	msg.AddInt32("index", -1);

	// Find out what to do with the error
	modErr = windowSystem->useSettings->GetIntEntryValue("Options", "ModuleError");

	switch (modErr)
	{
		case CV_ERROR_SHOWERROR:
		{
			int32 number;
			PString title, errStr1, errStr2, buttons;
			char *errStr;

			// Clip out the "ERR=" part
			error.Delete(0, 4);

			// Get the error number
			index = error.Find(',');
			if (index != -1)
			{
				number = error.GetNumber();
				error.Delete(0, index + 1);
			}
			else
				number = 0;

			errStr1.LoadString(windowSystem->res, resourceError);
			errStr2.Format(windowSystem->res, IDS_ERR_SERVER, number, (errStr = error.GetString()));
			error.FreeBuffer(errStr);

			// Load the title used in errors and the buttons to show
			title.LoadString(windowSystem->res, IDS_ERR_TITLE);
			buttons.LoadString(windowSystem->res, IDS_BUTTON_LOAD_ERROR);

			PAlert alert(title, errStr1 + errStr2, PAlert::pStop, buttons);
			response = alert.Show();
			break;
		}

		case CV_ERROR_SKIP:
		{
			response = PAlert::pIDButton1;
			break;
		}

		case CV_ERROR_SKIPANDREMOVE:
		{
			response = PAlert::pIDButton2;
			break;
		}

		case CV_ERROR_STOP:
		{
			response = PAlert::pIDButton3;
			break;
		}

		default:
		{
			ASSERT(false);
			response = PAlert::pIDButton1;
			break;
		}
	}

	switch (response)
	{
		//
		// Skip
		//
		case PAlert::pIDButton1:
		{
			// Lock the window
			windowSystem->mainWin->Lock();

			// Get the index of the module that couldn't be loaded + 1
			index = windowSystem->mainWin->GetPlayItem() + 1;

			// Get the number of items in the list
			count = windowSystem->mainWin->GetListCount();

			// Deselect the playing flag in the module list
			windowSystem->mainWin->PostMessage(&msg);

			// Does there exist a "next" module
			if (index < count)
			{
				// Yap, load it
				windowSystem->LoadAndPlayModule(index);
			}
			else
			{
				// Nup, what should we do now?
				listEnd = windowSystem->useSettings->GetIntEntryValue("Options", "ModuleListEnd");

				if ((count != 1) && (listEnd == CV_LISTEND_JUMPTOSTART))
				{
					APWindowMainListItem *item;

					// Load the first module, but only if it's valid
					// or haven't been loaded before
					item = windowSystem->mainWin->GetListItem(0);
					if (!item->HaveTime() || (item->HaveTime() && (item->GetTime().GetTotalMilliSeconds() != 0)))
						windowSystem->LoadAndPlayModule(0);
				}
			}

			// Unlock the window
			windowSystem->mainWin->Unlock();
			break;
		}

		//
		// Skip and remove
		//
		case PAlert::pIDButton2:
		{
			// Lock the window
			windowSystem->mainWin->Lock();

			// Get the index of the module that couldn't be loaded
			index = windowSystem->mainWin->GetPlayItem();

			// Get the number of items in the list - 1
			count = windowSystem->mainWin->GetListCount() - 1;

			// Deselect the playing flag in the module list
			windowSystem->mainWin->PostMessage(&msg);

			// Remove the module from the list
			windowSystem->mainWin->RemoveListItem(index);

			// Does there exist a "next" module
			if (index < count)
			{
				// Yap, load it
				windowSystem->LoadAndPlayModule(index);
			}
			else
			{
				// Nup, what should we do now?
				listEnd = windowSystem->useSettings->GetIntEntryValue("Options", "ModuleListEnd");

				if ((count != 1) && (listEnd == CV_LISTEND_JUMPTOSTART))
				{
					// Load the first module
					windowSystem->LoadAndPlayModule(0);
				}
			}

			// Unlock the window
			windowSystem->mainWin->Unlock();
			break;
		}

		//
		// Stop playing
		//
		case PAlert::pIDButton3:
		{
			// Deselect the playing flag in the module list
			windowSystem->mainWin->PostMessage(&msg);
			break;
		}

		default:
		{
			// Just to make the compiler happy
			break;
		}
	}
}



/******************************************************************************/
/* StartPlaying() will start to play the song number given.                   */
/*                                                                            */
/* Input:  "song" is the song number to play or -1 to play the default song.  */
/*         "startPos" is the start position or -1 for the start of the song.  */
/******************************************************************************/
void APLoader::StartPlaying(int16 song, int16 startPos)
{
	APModuleItem modItem;
	int16 songLength, songPos;
	uint16 currentSong, maxSongNum, chanNum;
	uint32 modSize;
	bool changePos;
	PTimeSpan totalTime;
	PList<PTimeSpan> posTimes;
	PList<PString> modInfo;
	PString modName, author, format, playerName;
	PString result;

	// Is there any module loaded?
	if (loadedFiles.CountItems() == 0)
		return;

	// Get the item to play
	modItem = loadedFiles.GetHead();

	// Change the volume
	if (windowSystem->playerInfo->IsMuted())
		SendSetVolume(modItem.fileHandle, 0);
	else
		SendSetVolume(modItem.fileHandle, windowSystem->playerInfo->GetVolume());

	// Tell the server to initialize the module
	result = SendStartPlayer(modItem.fileHandle, song);
	if (result.Left(4) == "ERR=")
	{
		ShowError(IDS_ERR_START_PLAYER, result);

		// Remove the file again from the server
		SendUnloadFile(modItem.fileHandle);
		SendRemoveFile(modItem.fileHandle);

		// And remove the file from the list
		loadedFiles.RemoveItem(0);
		return;
	}

	// Get the song length and position
	songLength = SendGetSongLength(modItem.fileHandle);

	if (startPos == -1)
	{
		songPos = SendGetSongPosition(modItem.fileHandle);
		if (songPos == -1)
			songPos = 0;
	}
	else
	{
		songPos = startPos;
		SendSetPosition(modItem.fileHandle, songPos);
	}

	// Get the current song number
	currentSong = SendGetCurrentSong(modItem.fileHandle);

	// Get the maximum number of sub-songs
	maxSongNum = SendGetMaxSongNumber(modItem.fileHandle);

	// Get the number of channels
	chanNum = SendGetModuleChannels(modItem.fileHandle);

	// Get the total time of the module
	totalTime = SendGetTotalTime(modItem.fileHandle);

	// Get the position time list
	SendGetTimeList(modItem.fileHandle, posTimes);

	// Get the size of the module
	modSize = SendGetModuleSize(modItem.fileHandle);

	// Get the module name
	modName = SendGetModuleName(modItem.fileHandle);

	// Get the author
	author = SendGetAuthor(modItem.fileHandle);

	// Get the module format
	format = SendGetModuleFormat(modItem.fileHandle);

	// Get the name of the player
	playerName = SendGetPlayerName(modItem.fileHandle);

	// Can the player change the position
	changePos = SendCanChangePosition(modItem.fileHandle);

	// Get the module information
	SendGetModuleInformation(modItem.fileHandle, modInfo);

	// Lock the player information, because
	// we change a lot now
	windowSystem->playerInfo->Lock();

	// The module is playing
	windowSystem->playerInfo->SetPlayFlag(true);

	// Set the module information
	windowSystem->playerInfo->SetSongLength(songLength);
	windowSystem->playerInfo->SetSongPosition(songPos);
	windowSystem->playerInfo->SetCurrentSong(currentSong);
	windowSystem->playerInfo->SetMaxSongNumber(maxSongNum);
	windowSystem->playerInfo->SetModuleChannels(chanNum);
	windowSystem->playerInfo->SetModuleSize(modSize);
	windowSystem->playerInfo->SetModuleName(modName);
	windowSystem->playerInfo->SetAuthor(author);
	windowSystem->playerInfo->SetTotalTime(totalTime);
	windowSystem->playerInfo->SetPositionTimes(posTimes);
	windowSystem->playerInfo->SetChangePositionFlag(changePos);

	windowSystem->playerInfo->SetFileName(modItem.listItem->GetFileName());
	windowSystem->playerInfo->SetModuleFormat(format);
	windowSystem->playerInfo->SetPlayerName(playerName);
	windowSystem->playerInfo->SetOutputAgent(modItem.outputAgent);
	windowSystem->playerInfo->SetModuleInformation(modInfo);

	// The information are now valid
	windowSystem->playerInfo->SetInfoFlag(true);

	// Unlock the player information
	windowSystem->playerInfo->Unlock();

	// Initialize all the stuff in the windows
	windowSystem->mainWin->InitWindowWhenPlayStarts();
	RefreshWindows();

	// Set the length attribute on the file, but
	// only if we play the default song and the
	// item is an ordinary file
	if ((song == -1) && (modItem.listItem->GetItemType() == APWindowMainListItem::apNormal))
		SetTotalTimeOnFile(modItem.listItem->GetFileName(), totalTime);
}



/******************************************************************************/
/* RefreshWindows() will make sure to refresh all the windows.                */
/******************************************************************************/
void APLoader::RefreshWindows(void)
{
	windowSystem->infoWin->RefreshWindow();
	windowSystem->sampWin->RefreshWindow();

	if (windowSystem->settingsWin != NULL)
		windowSystem->settingsWin->RefreshWindow();
}



/******************************************************************************/
/* SendAddFile() will create and send a "AddFile" command to the server.      */
/*                                                                            */
/* Input:  "fileName" is the name to the file to add.                         */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendAddFile(PString fileName)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("AddFile=", fileName);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendCanChangePosition() will create and send a "CanChangePosition" command */
/*      to the server and return bool telling it.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: True if the player can change position, false if not.              */
/******************************************************************************/
bool APLoader::SendCanChangePosition(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("CanChangePosition=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (false);

	return (result.GetUNumber() != 0);
}



/******************************************************************************/
/* SendChangeChannels() will create and send a "ChangeChannels" command to    */
/*      the server.                                                           */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "enabled" indicates if you want to enable or disable the channels. */
/*         "starChan" is the start channel to change.                         */
/*         "stopChan" is the stop channel to change or -1 if you only want to */
/*         change one channel.                                                */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendChangeChannels(PString handle, bool enabled, int16 startChan, int16 stopChan)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("ChangeChannels=", handle);
	cmd    = global->communication->AddArgument(cmd, PString::CreateUNumber(enabled));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(startChan));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(stopChan));
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendEndPlayer() will create and send a "EndPlayer" command to the server.  */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendEndPlayer(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("EndPlayer=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendGetAuthor() will create and send a "GetAuthor" command to the server   */
/*      and return the name of the author.                                    */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The author.                                                        */
/******************************************************************************/
PString APLoader::SendGetAuthor(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetAuthor=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return ("");

	return (result);
}



/******************************************************************************/
/* SendGetCurrentSong() will create and send a "GetCurrentSong" command to    */
/*      the server and return the song number.                                */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The current playing song number.                                   */
/******************************************************************************/
uint16 APLoader::SendGetCurrentSong(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetCurrentSong=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetUNumber());
}



/******************************************************************************/
/* SendGetMaxSongNumber() will create and send a "GetMaxSongNumber" command   */
/*      to the server and return the maximum song number.                     */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The maximum song number.                                           */
/******************************************************************************/
uint16 APLoader::SendGetMaxSongNumber(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetMaxSongNumber=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetUNumber());
}



/******************************************************************************/
/* SendGetModuleChannels() will create and send a "GetModuleChannels" command */
/*      to the server and return the number of channels used in the module.   */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The number of channels.                                            */
/******************************************************************************/
uint16 APLoader::SendGetModuleChannels(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetModuleChannels=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetUNumber());
}



/******************************************************************************/
/* SendGetModuleFormat() will create and send a "GetModuleFormat" command to  */
/*      the server and return the format of the module.                       */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The module format.                                                 */
/******************************************************************************/
PString APLoader::SendGetModuleFormat(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetModuleFormat=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return ("");

	return (result);
}



/******************************************************************************/
/* SendGetModuleInformation() will create and send a "GetModuleInformation"   */
/*      command to the server and return the information.                     */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "infoList" is a reference to the list where the information should */
/*         be stored.                                                         */
/******************************************************************************/
void APLoader::SendGetModuleInformation(PString handle, PList<PString> &infoList)
{
	PString cmd, result;
	int32 oldIndex = 0;
	int32 index;

	cmd    = global->communication->AddArgument("GetModuleInformation=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if ((result.IsEmpty()) || (result.Left(4) == "ERR="))
		return;

	// Build the info list
	while ((index = result.Find('\n', oldIndex)) != -1)
	{
		infoList.AddTail(result.Mid(oldIndex, index - oldIndex));
		oldIndex = index + 1;
	}
}



/******************************************************************************/
/* SendGetModuleName() will create and send a "GetModuleName" command to the  */
/*      server and return the name of the module.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The module name.                                                   */
/******************************************************************************/
PString APLoader::SendGetModuleName(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetModuleName=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return ("");

	return (result);
}



/******************************************************************************/
/* SendGetModuleSize() will create and send a "GetModuleSize" command to the  */
/*      server and return the size of the module.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The module size.                                                   */
/******************************************************************************/
uint32 APLoader::SendGetModuleSize(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetModuleSize=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetUNumber());
}



/******************************************************************************/
/* SendGetPlayerName() will create and send a "GetPlayerName" command to the  */
/*      server and return the name of the player.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The player name.                                                   */
/******************************************************************************/
PString APLoader::SendGetPlayerName(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetPlayerName=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return ("");

	return (result);
}



/******************************************************************************/
/* SendGetSongLength() will create and send a "GetSongLength" command to the  */
/*      server and return the length of the song.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The length of the song.                                            */
/******************************************************************************/
int16 APLoader::SendGetSongLength(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetSongLength=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetNumber());
}



/******************************************************************************/
/* SendGetSongPosition() will create and send a "GetSongPosition" command to  */
/*      the server and return the current song position.                      */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The song position.                                                 */
/******************************************************************************/
int16 APLoader::SendGetSongPosition(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetSongPosition=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetNumber());
}



/******************************************************************************/
/* SendGetTimeList() will create and send a "GetTimeList" command to the      */
/*      server and return the position time list.                             */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "timeList" is a reference to the list where the time list should   */
/*         be stored.                                                         */
/******************************************************************************/
void APLoader::SendGetTimeList(PString handle, PList<PTimeSpan> &timeList)
{
	PString cmd, result;
	int32 oldIndex = 0;
	int32 index;

	cmd    = global->communication->AddArgument("GetTimeList=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if ((result.IsEmpty()) || (result.Left(4) == "ERR="))
		return;

	// Build the time list
	while ((index = result.Find(',', oldIndex)) != -1)
	{
		timeList.AddTail(result.GetNumber64(oldIndex));
		oldIndex = index + 1;
	}

	// Add the last number
	timeList.AddTail(result.GetNumber64(oldIndex));
}



/******************************************************************************/
/* SendGetTotalTime() will create and send a "GetTotalTime" command to the    */
/*      server and return the total time of the song.                         */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The total time of the song.                                        */
/******************************************************************************/
PTimeSpan APLoader::SendGetTotalTime(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("GetTotalTime=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		return (0);

	return (result.GetNumber64());
}



/******************************************************************************/
/* SendHoldPlaying() will create and send a "HoldPlaying" command to the      */
/*      server.                                                               */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "holdValue" is the hold value to send.                             */
/******************************************************************************/
void APLoader::SendHoldPlaying(PString handle, bool holdValue)
{
	PString cmd;

	cmd = global->communication->AddArgument("HoldPlaying=", handle);
	cmd = global->communication->AddArgument(cmd, PString::CreateUNumber(holdValue));
	global->communication->SendCommand(windowSystem->serverHandle, cmd);
}



/******************************************************************************/
/* SendInitPlayer() will create and send a "InitPlayer" command to the server.*/
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendInitPlayer(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("InitPlayer=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendLoadFile() will create and send a "LoadFile" command to the server.    */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "changeType" indicates if you want the server to change the file   */
/*         type on the file loaded.                                           */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendLoadFile(PString handle, bool changeType)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("LoadFile=", handle);
	cmd    = global->communication->AddArgument(cmd, PString::CreateUNumber(changeType));
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendPausePlayer() will create and send a "PausePlayer" command to the      */
/*      server.                                                               */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendPausePlayer(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("PausePlayer=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendRemoveFile() will create and send a "RemoveFile" command to the server.*/
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendRemoveFile(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("RemoveFile=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendResumePlayer() will create and send a "ResumePlayer" command to the    */
/*      server.                                                               */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendResumePlayer(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("ResumePlayer=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendSetMixerSettings() will create and send a "SetMixerSettings" command   */
/*      to the server.                                                        */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "frequency" is the new mixer frequency or -1 to keep to current.   */
/*         "interpolation" indicates if you want interpolation or not. -1     */
/*         keeps the current setting.                                         */
/*         "dolby" indicates if you want Dolby Surround or not. -1 keeps the  */
/*         current setting.                                                   */
/*         "stereoSep" is the new stereo separator value or -1 to keep the    */
/*         current setting.                                                   */
/*         "filter" indicates if you want Amiga filter emulation or not or -1 */
/*         to keep the current setting.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendSetMixerSettings(PString handle, int32 frequency, int32 interpolation, int32 dolby, int32 stereoSep, int32 filter)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("SetMixerSettings=", handle);
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(frequency));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(interpolation));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(dolby));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(stereoSep));
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(filter));
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendSetOutputAgent() will create and send a "SetOutputAgent" command to    */
/*      the server.                                                           */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "agent" is the output agent to use.                                */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendSetOutputAgent(PString handle, PString agent)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("SetOutputAgent=", handle);
	cmd    = global->communication->AddArgument(cmd, agent);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendSetPosition() will create and send a "SetPosition" command to the      */
/*      server.                                                               */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "newPos" is the new song position.                                 */
/******************************************************************************/
void APLoader::SendSetPosition(PString handle, int16 newPos)
{
	PString cmd;

	cmd = global->communication->AddArgument("SetPosition=", handle);
	cmd = global->communication->AddArgument(cmd, PString::CreateUNumber(newPos));
	global->communication->SendCommand(windowSystem->serverHandle, cmd);
}



/******************************************************************************/
/* SendSetVolume() will create and send a "SetVolume" command to the server.  */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "newVol" is the new volume.                                        */
/******************************************************************************/
void APLoader::SendSetVolume(PString handle, uint16 newVol)
{
	PString cmd;

	cmd = global->communication->AddArgument("SetVolume=", handle);
	cmd = global->communication->AddArgument(cmd, PString::CreateUNumber(newVol));
	global->communication->SendCommand(windowSystem->serverHandle, cmd);
}



/******************************************************************************/
/* SendStartPlayer() will create and send a "StartPlayer" command to the      */
/*      server.                                                               */
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*         "subSong" is the sub song to play or -1 for the default.           */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendStartPlayer(PString handle, int16 subSong)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("StartPlayer=", handle);
	cmd    = global->communication->AddArgument(cmd, PString::CreateNumber(subSong));
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendStopPlayer() will create and send a "StopPlayer" command to the server.*/
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendStopPlayer(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("StopPlayer=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}



/******************************************************************************/
/* SendUnloadFile() will create and send a "UnloadFile" command to the server.*/
/*                                                                            */
/* Input:  "handle" is the file handle.                                       */
/*                                                                            */
/* Output: The server result.                                                 */
/******************************************************************************/
PString APLoader::SendUnloadFile(PString handle)
{
	PString cmd, result;

	cmd    = global->communication->AddArgument("UnloadFile=", handle);
	result = global->communication->SendCommand(windowSystem->serverHandle, cmd);

	return (result);
}
