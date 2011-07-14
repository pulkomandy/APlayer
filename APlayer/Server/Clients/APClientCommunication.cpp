/******************************************************************************/
/* APlayer client communication class.                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PDebug.h"
#include "PException.h"
#include "PSystem.h"

// APlayerKit headers
#include "APGlobal.h"
#include "APClientCommunication.h"

// Server headers
#include "APApplication.h"
#include "APPlayer.h"
#include "APAddOnWindows.h"
#include "APClientCommunication.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
extern APGlobal *globalData;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APClientCommunication::APClientCommunication(void) : BLooper("Server client add-on looper")
{
	// Add all the commands to the list
	cmdList.InsertItem("AddFile", AddFile);
	cmdList.InsertItem("CanChangePosition", CanChangePosition);
	cmdList.InsertItem("ChangeChannels", ChangeChannels);
	cmdList.InsertItem("DisableAddOn", DisableAddOn);
	cmdList.InsertItem("EnableAddOn", EnableAddOn);
	cmdList.InsertItem("EndPlayer", EndPlayer);
	cmdList.InsertItem("GetAuthor", GetAuthor);
	cmdList.InsertItem("GetCurrentSong", GetCurrentSong);
	cmdList.InsertItem("GetMaxSongNumber", GetMaxSongNumber);
	cmdList.InsertItem("GetModuleChannels", GetModuleChannels);
	cmdList.InsertItem("GetModuleFormat", GetModuleFormat);
	cmdList.InsertItem("GetModuleInformation", GetModuleInformation);
	cmdList.InsertItem("GetModuleName", GetModuleName);
	cmdList.InsertItem("GetModuleSize", GetModuleSize);
	cmdList.InsertItem("GetPlayerName", GetPlayerName);
	cmdList.InsertItem("GetSongLength", GetSongLength);
	cmdList.InsertItem("GetSongPosition", GetSongPosition);
	cmdList.InsertItem("GetTimeList", GetTimeList);
	cmdList.InsertItem("GetTotalTime", GetTotalTime);
	cmdList.InsertItem("HoldPlaying", HoldPlaying);
	cmdList.InsertItem("InitPlayer", InitPlayer);
	cmdList.InsertItem("LoadFile", LoadFile);
	cmdList.InsertItem("OpenConfigWindow", OpenConfigWindow);
	cmdList.InsertItem("OpenDisplayWindow", OpenDisplayWindow);
	cmdList.InsertItem("PausePlayer", PausePlayer);
	cmdList.InsertItem("RemoveFile", RemoveFile);
	cmdList.InsertItem("ResumePlayer", ResumePlayer);
	cmdList.InsertItem("SaveSettings", SaveSettings);
	cmdList.InsertItem("SetMixerSettings", SetMixerSettings);
	cmdList.InsertItem("SetOutputAgent", SetOutputAgent);
	cmdList.InsertItem("SetPosition", SetPosition);
	cmdList.InsertItem("SetVolume", SetVolume);
	cmdList.InsertItem("StartedNormally", StartedNormally);
	cmdList.InsertItem("StartPlayer", StartPlayer);
	cmdList.InsertItem("StopPlayer", StopPlayer);
	cmdList.InsertItem("UnloadFile", UnloadFile);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APClientCommunication::~APClientCommunication(void)
{
}



/******************************************************************************/
/* Start() will start the communications.                                     */
/******************************************************************************/
void APClientCommunication::Start(void)
{
	// Tell the global data object about this looper
	globalData->communication->SetServerLooper(this);

	// Start the BLooper
	Run();
}



/******************************************************************************/
/* Stop() will stop all communications and delete itself.                     */
/******************************************************************************/
void APClientCommunication::Stop(void)
{
	BMessenger messenger(NULL, this);
	BMessage message(B_QUIT_REQUESTED);

	// Tell the BLooper to quit and delete itself
	// This call is synchronous, so we are sure the
	// object has been deleted when the function
	// call returns
	messenger.SendMessage(&message, &message);
}



/******************************************************************************/
/* SendCommand() will send the command given to a specific client.            */
/*                                                                            */
/* Input:  "player" is a pointer to the player that want to send a command.   */
/*         "command" is the command to send.                                  */
/******************************************************************************/
void APClientCommunication::SendCommand(APPlayer *player, PString command)
{
	BMessage message(APSERVER_MSG_DATA);
	APFileHandle handle;
	int32 i, count;
	char *commandStr;

	// Package the command into the message
	message.AddString("Command", (commandStr = command.GetString()));
	command.FreeBuffer(commandStr);

	// Lock the file handle list
	fileHandleList.LockList();

	try
	{
		// Find the handle that have the player in it
		count = fileHandleList.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get the handle
			handle = fileHandleList.GetItem(i);

			// Did we found the player?
			if (handle.player == player)
			{
				// Yap, send the command to the client
				handle.looper->PostMessage(&message);
				break;
			}
		}
	}
	catch(...)
	{
		fileHandleList.UnlockList();
		throw;
	}

	// Unlock the list again
	fileHandleList.UnlockList();
}



/******************************************************************************/
/* SendCommandToAll() will send the command given to all clients.             */
/*                                                                            */
/* Input:  "command" is the command to send.                                  */
/******************************************************************************/
void APClientCommunication::SendCommandToAll(PString command)
{
	BMessage message(APSERVER_MSG_DATA);
	int32 i, count;
	char *commandStr;

	// Package the command into the message
	message.AddString("Command", (commandStr = command.GetString()));
	command.FreeBuffer(commandStr);

	// Lock the client list
	clientLoopers.LockList();

	// Send to all the clients
	count = clientLoopers.CountItems();
	for (i = 0; i < count; i++)
	{
		// Send the message
		clientLoopers.GetItem(i)->PostMessage(&message);
	}

	// Unlock the list again
	clientLoopers.UnlockList();
}



/******************************************************************************/
/* GetInstrumentList() will lock the list with the instruments and return a   */
/*      pointer to it.                                                        */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/*                                                                            */
/* Output: A pointer to the list or NULL.                                     */
/******************************************************************************/
const PList<APInstInfo *> *APClientCommunication::GetInstrumentList(uint32 fileHandle)
{
	APFileHandle handle;

	// Get the file information
	if (!FindFileHandle(fileHandle, handle))
		return (NULL);

	// Return the list
	return (handle.player->GetInstrumentList());
}



/******************************************************************************/
/* UnlockInstrumentList() unlocks the instrument list.                        */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/******************************************************************************/
void APClientCommunication::UnlockInstrumentList(uint32 fileHandle)
{
	APFileHandle handle;

	// Get the file information
	if (FindFileHandle(fileHandle, handle))
	{
		// Unlock the list
		handle.player->UnlockInstrumentList();
	}
}



/******************************************************************************/
/* GetSampleList() will lock the list with the samples and return a pointer   */
/*      to it.                                                                */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/*                                                                            */
/* Output: A pointer to the list or NULL.                                     */
/******************************************************************************/
const PList<APSampleInfo *> *APClientCommunication::GetSampleList(uint32 fileHandle)
{
	APFileHandle handle;

	// Get the file information
	if (!FindFileHandle(fileHandle, handle))
		return (NULL);

	// Return the list
	return (handle.player->GetSampleList());
}



/******************************************************************************/
/* UnlockSampleList() unlocks the sample list.                                */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/******************************************************************************/
void APClientCommunication::UnlockSampleList(uint32 fileHandle)
{
	APFileHandle handle;

	// Get the file information
	if (FindFileHandle(fileHandle, handle))
	{
		// Unlock the list
		handle.player->UnlockSampleList();
	}
}



/******************************************************************************/
/* MessageReceived() is called for each message sent to the looper.           */
/*                                                                            */
/* Input:  "message" is a pointer to the message that has been sent.          */
/******************************************************************************/
void APClientCommunication::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		//
		// Connect
		//
		case APSERVER_MSG_CONNECT:
		{
			void *looper;

			// Get looper from the message
			if (message->FindPointer("ClientLooper", &looper) != B_OK)
			{
				// Could not find the pointer, send an error back
				message->SendReply(APSERVER_MSG_ERROR);
			}
			else
			{
				// Got the looper, now remember it
				clientLoopers.LockList();
				clientLoopers.AddTail((BLooper *)looper);
				clientLoopers.UnlockList();

				// Send an ok back
				message->SendReply(APSERVER_MSG_OK);
			}
			break;
		}

		//
		// Disconnect
		//
		case APSERVER_MSG_DISCONNECT:
		{
			int32 i, count;
			void *looper;

			// Get looper from the message
			if (message->FindPointer("ClientLooper", &looper) != B_OK)
			{
				// Could not find the pointer, send an error back
				message->SendReply(APSERVER_MSG_ERROR);
			}
			else
			{
				// Got the looper, now remove it from the list
				clientLoopers.LockList();

				count = clientLoopers.CountItems();
				for (i = 0; i < count; i++)
				{
					// Did we found the looper
					if (clientLoopers.GetItem(i) == looper)
					{
						// Yup, remove it
						clientLoopers.RemoveItem(i);
						break;
					}
				}

				// Unlock list
				clientLoopers.UnlockList();

				// Send an ok back
				message->SendReply(APSERVER_MSG_OK);

				// If this was the last client that has disconnected,
				// close the server
				if (count <= 1)
					be_app->PostMessage(B_QUIT_REQUESTED);
			}
			break;
		}

		//
		// Data
		//
		case APSERVER_MSG_DATA:
		{
			void *looper;
			const char *cmd;
			PString result;

			if (message->FindPointer("ClientLooper", &looper) != B_OK)
			{
				// Could not find the pointer, send an error back
				message->SendReply(APSERVER_MSG_ERROR);
			}
			else
			{
				// Get the command from the message
				if (message->FindString("Command", &cmd) != B_OK)
				{
					// Could not find the command, send an error back
					message->SendReply(APSERVER_MSG_ERROR);
				}
				else
				{
					BMessage reply;
					char *resultStr;

					//
					// FUTURE: Make each command to run in its own thread if possible
					//
					;

					// Parse the command
					reply.what = ParseCommand((BLooper *)looper, cmd, result) ? APSERVER_MSG_OK : APSERVER_MSG_ERROR;

					// Build reply message
					if (!result.IsEmpty())
					{
						// Add the result
						reply.AddString("Result", (resultStr = result.GetString()));
						result.FreeBuffer(resultStr);
					}

					// Send the reply
					message->SendReply(&reply);
				}
			}
			break;
		}

		//
		// Unknown message. Call the base class
		//
		default:
		{
			BLooper::MessageReceived(message);
			break;
		}
	}
}



/******************************************************************************/
/* ParseCommand() will find out which command to run and split all the        */
/*      arguments.                                                            */
/*                                                                            */
/* Input:  "looper" is the looper where the command came from.                */
/*         "command" is the commmand with arguments.                          */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::ParseCommand(BLooper *looper, PString command, PString &result)
{
	PString cmd, temp;
	PList<PString> args;
	CommandFunc func;
	int index;

	// Start to extract the command
	index = command.Find('=');
	if (index == -1)
		return (false);

	cmd = command.Left(index);
	command.Delete(0, index + 1);

	// Now take all the arguments
	while ((index = command.Find("\",")) != -1)
	{
		// Extract a single argument
		temp = command.Mid(1, index - 1);
		command.Delete(0, index + 2);

		// Fix it
		temp.Replace("/\"", "\"");
		temp.Replace("//", "/");

		// Add the argument to the list
		args.AddTail(temp);
	}

	// Did we have any arguments left?
	if (!command.IsEmpty())
	{
		// Yup, fix it
		temp = command.Mid(1, command.GetLength() - 2);
		temp.Replace("/\"", "\"");
		temp.Replace("//", "/");

		// And add it to the list
		args.AddTail(temp);
	}

	// Find the command
	if (!cmdList.GetItem(cmd, func))
	{
		// Unknown command
		result.Format_S1(GetApp()->resource, IDS_CMDERR_UNKNOWN_COMMAND, cmd);
		return (false);
	}

	// Found the command, now call it
	return (func(this, looper, args, result));
}



/******************************************************************************/
/* FindFileHandle() will search the intern file handle list to see if it can  */
/*      find a handle with the unique ID given as the argument.               */
/*                                                                            */
/* Input:  "uniqueID" is the file handle ID you want to find.                 */
/*         "handle" is a reference where the handle will be stored.           */
/*                                                                            */
/* Output: True if the handle could be found, false if not.                   */
/******************************************************************************/
bool APClientCommunication::FindFileHandle(uint32 uniqueID, APFileHandle &handle)
{
	int32 i, count;

	// Lock the file handle list
	fileHandleList.LockList();

	try
	{
		// Find the file
		count = fileHandleList.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get the handle
			handle = fileHandleList.GetItem(i);

			// Did we found the file?
			if (handle.uniqueID == uniqueID)
			{
				// Yip, unlock the list
				fileHandleList.UnlockList();
				return (true);
			}
		}
	}
	catch(...)
	{
		fileHandleList.UnlockList();
		throw;
	}

	// Unlock the list again
	fileHandleList.UnlockList();

	return (false);
}



/******************************************************************************/
/* SetFileHandle() will search the intern file handle list to see if it can   */
/*      find a handle with the unique ID given as the argument. If found, it  */
/*      will then overwrite it with the new handle given.                     */
/*                                                                            */
/* Input:  "uniqueID" is the file handle ID you want to find.                 */
/*         "handle" is a reference to the new handle.                         */
/******************************************************************************/
void APClientCommunication::SetFileHandle(uint32 uniqueID, APFileHandle &handle)
{
	APFileHandle searchHandle;
	int32 i, count;

	// Lock the file handle list
	fileHandleList.LockList();

	try
	{
		// Find the file
		count = fileHandleList.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get the handle
			searchHandle = fileHandleList.GetItem(i);

			// Did we found the file?
			if (searchHandle.uniqueID == uniqueID)
			{
				// Yip, overwrite it
				fileHandleList.SetItem(handle, i);
				break;
			}
		}
	}
	catch(...)
	{
		fileHandleList.UnlockList();
		throw;
	}

	// Unlock the list again
	fileHandleList.UnlockList();
}



/******************************************************************************/
/* FindAddOn() will search the list given after the specified add-on.         */
/*                                                                            */
/* Input:  "addOnName" is the name of the add-on to search for.               */
/*         "infoList" is a pointer to the list to search in.                  */
/*         "info" is a reference to where to store the information pointer.   */
/*                                                                            */
/* Output: True if the add-on was found, false if not.                        */
/******************************************************************************/
bool APClientCommunication::FindAddOn(PString addOnName, APMRSWList<AddOnInfo *> *infoList, AddOnInfo *&info)
{
	int32 i, count;
	bool found = false;

	// Lock the list
	infoList->WaitToRead();

	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		info = infoList->GetItem(i);
		if (info->addOnName == addOnName)
		{
			// Found the add-on
			found = true;
			break;
		}
	}

	// Unlock the list
	infoList->DoneReading();

	return (found);
}



/******************************************************************************/
/* DisableVirtualMixer() will search the file handler list and disable all    */
/*      relevant virtual mixers in the list.                                  */
/*                                                                            */
/* Input:  "agent" is the agent to search for.                                */
/******************************************************************************/
void APClientCommunication::DisableVirtualMixer(AddOnInfo *agent)
{
	int32 count, i;

	// Lock the file handler list
	fileHandleList.LockList();

	// Traverse all the file handlers
	count = fileHandleList.CountItems();
	for (i = 0; i < count; i++)
	{
		// Disable the virtual mixer
		fileHandleList.GetItem(i).player->DisableVirtualMixer(agent);
	}

	// Unlock the list again
	fileHandleList.UnlockList();
}



/******************************************************************************/
/*                                                                            */
/*                              Server commands                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* AddFile() will add the file to an intern list. The file can then be used   */
/*      in other commands.                                                    */
/*                                                                            */
/* Syntax: <handle> = AddFile=<file name>                                     */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::AddFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Create the file handle structure
	handle.fileName       = args.GetItem(0);
	handle.player         = NULL;
	handle.looper         = looper;
	handle.mixerFrequency = 44100;
	handle.interpolation  = false;
	handle.dolbyPrologic  = false;
	handle.amigaFilter    = false;

	// Create loader object
	handle.loader = new APModuleLoader();
	if (handle.loader == NULL)
		throw PMemoryException();

	// Lock the file handle list
	comm->fileHandleList.LockList();

	try
	{
		// Set the unique ID
		if (comm->fileHandleList.CountItems() > 0)
			handle.uniqueID = comm->fileHandleList.GetItem(comm->fileHandleList.CountItems() - 1).uniqueID + 1;
		else
			handle.uniqueID = 1;

		// Add the handle to the list
		comm->fileHandleList.AddTail(handle);
	}
	catch(...)
	{
		comm->fileHandleList.UnlockList();
		throw;
	}

	// Unlock the list again
	comm->fileHandleList.UnlockList();

	// Return the handle
	result.Format("%d", handle.uniqueID);

	return (true);
}



/******************************************************************************/
/* CanChangePosition() will return if the player support position change.     */
/*                                                                            */
/* Syntax: <posChange> = CanChangePosition=<handle>                           */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::CanChangePosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	bool posChange;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the position change bool
	posChange = handle.player->CanChangePosition();

	// Store the result
	result.SetUNumber(posChange);

	return (true);
}



/******************************************************************************/
/* ChangeChannels() will change the channels in the mixer.                    */
/*                                                                            */
/* Syntax: ChangeChannels=<handle>,<enable>,<startChan>,<stopChan>            */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::ChangeChannels(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	bool enable;
	int16 startChan, stopChan;

	// Check the arguments
	if (args.CountItems() != 4)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Get the other arguments
	enable    = args.GetItem(1).GetNumber();
	startChan = args.GetItem(2).GetNumber();
	stopChan  = args.GetItem(3).GetNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Change the channels
	if (handle.player != NULL)
		handle.player->ChangeChannels(enable, startChan, stopChan);

	return (true);
}



/******************************************************************************/
/* DisableAddOn() will disable the add-on.                                    */
/*                                                                            */
/* Syntax: DisableAddOn=<add-on name>                                         */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::DisableAddOn(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	PString addOnName, sectionName;
	AddOnInfo *addOn;
	APMRSWList<AddOnInfo *> *addOnList;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Get the name of the add-on
	addOnName = args.GetItem(0);

	// Look for the add-on
	addOnList   = &GetApp()->playerInfo;
	sectionName = "Player Add-Ons";
	if (!comm->FindAddOn(addOnName, addOnList, addOn))
	{
		addOnList   = &GetApp()->agentInfo;
		sectionName = "Agent Add-Ons";
		if (!comm->FindAddOn(addOnName, addOnList, addOn))
		{
			addOnList   = &GetApp()->clientInfo;
			sectionName = "Client Add-Ons";
			if (!comm->FindAddOn(addOnName, addOnList, addOn))
			{
				result.Format_S1(GetApp()->resource, IDS_CMDERR_ADDON_NOT_FOUND, addOnName);
				return (false);
			}
		}
	}

	// Lock the list
	addOnList->WaitToWrite();

	// Close any windows the add-on have open
	GetApp()->addOnWindows->CloseAddOnWindows(addOn);

	// Update the information list
	addOn->enabled = false;

	if (!addOn->allocated)
		addOn->loader = NULL;
	else
	{
		// Begin to unload the add-on.
		// First see if the same player is elsewhere in the list
		int32 i, count;
		AddOnInfo *testItem;
		bool found = false;

		count = addOnList->CountItems();
		for (i = 0; i < count; i++)
		{
			testItem = addOnList->GetItem(i);
			if ((testItem != addOn) && (testItem->loader != NULL) && (testItem->fileName == addOn->fileName))
			{
				// Yeah we found one, set this add-on to the allocated one
				testItem->allocated = true;
				found = true;
				break;
			}
		}			

		if (addOn->pluginFlags & apaVirtualMixer)
		{
			// Disable the mixer, so the main mixer won't call it anymore
			comm->DisableVirtualMixer(addOn);
		}

		if (!found)
		{
			// Okay, this is the only instance of the add-on, so unload it
			if (addOn->isAgent)
			{
				if (addOn->isClient)
				{
					// Plug-out the agent part
					GetApp()->PlugAgentOut(addOn, false);

					// Stop the client
					((APAddOnClient *)addOn->agent)->EndClient(addOn->index);

					// Delete the instance
					addOn->loader->DeleteInstance(addOn->agent);

					// Clear pointers
					addOn->agent = NULL;
				}
				else
					GetApp()->PlugAgentOut(addOn, true);
			}

			addOn->loader->Unload();
			delete addOn->loader;
		}

		// Clear the rest of the info list
		addOn->loader    = NULL;
		addOn->allocated = false;
	}

	// Write the new settings
	GetApp()->useSettings->WriteStringEntryValue(sectionName, addOn->addOnName, "0," + addOn->fileName);

	// Done with the list
	addOnList->DoneWriting();

	return (true);
}



/******************************************************************************/
/* EnableAddOn() will enable the add-on.                                      */
/*                                                                            */
/* Syntax: EnableAddOn=<add-on name>                                          */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::EnableAddOn(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	PString addOnName, sectionName, dirName;
	AddOnInfo *addOn;
	APMRSWList<AddOnInfo *> *addOnList;
	int32 i, count;
	AddOnInfo *testItem;
	APAddOnLoader *loader;
	bool found;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Get the name of the add-on
	addOnName = args.GetItem(0);

	// Look for the add-on
	addOnList   = &GetApp()->playerInfo;
	sectionName = "Player Add-Ons";
	dirName     = "Players";
	if (!comm->FindAddOn(addOnName, addOnList, addOn))
	{
		addOnList   = &GetApp()->agentInfo;
		sectionName = "Agent Add-Ons";
		dirName     = "Agents";
		if (!comm->FindAddOn(addOnName, addOnList, addOn))
		{
			addOnList   = &GetApp()->clientInfo;
			sectionName = "Client Add-Ons";
			dirName     = "Clients";
			if (!comm->FindAddOn(addOnName, addOnList, addOn))
			{
				result.Format_S1(GetApp()->resource, IDS_CMDERR_ADDON_NOT_FOUND, addOnName);
				return (false);
			}
		}
	}

	// Lock the list
	addOnList->WaitToWrite();

	// Begin to load the add-on
	// First see if the same player is elsewhere in the list
	found = false;

	count = addOnList->CountItems();
	for (i = 0; i < count; i++)
	{
		testItem = addOnList->GetItem(i);
		if ((testItem != addOn) && (testItem->loader != NULL) && (testItem->fileName == addOn->fileName))
		{
			// Yeah we found one, use this add-on
			addOn->loader = testItem->loader;
			found = true;
			break;
		}
	}			

	if (!found)
	{
		// Okay, this is the only instance of the add-on, so load it
		loader = new APAddOnLoader(dirName + P_DIRSLASH_STR + addOn->fileName);
		if (loader == NULL)
			throw PMemoryException();

		if (loader->Load())
		{
			addOn->loader    = loader;
			addOn->allocated = true;

			if (addOn->isAgent)
			{
				if (addOn->isClient)
				{
					APAddOnClient *client;

					// Allocate an instance of the add-on
					client = (APAddOnClient *)addOn->loader->CreateInstance();
					if (client == NULL)
						throw PMemoryException();

					// Remember the pointer to the instance
					addOn->agent = client;

					// Initialize the client
					if (!client->InitClient(addOn->index))
					{
						result.Format_S1(GetApp()->resource, IDS_CMDERR_STARTCLIENT, addOn->addOnName);

						addOnList->DoneWriting();
						return (false);
					}

					// Plug the client in as an agent
					GetApp()->PlugAgentIn(addOn, client);

					// Start the client
					client->Start(addOn->index);
				}
				else
				{
					// Plug-in the agent the places it need
					GetApp()->PlugAgentIn(addOn, NULL);
				}
			}
		}
		else
		{
			delete loader;
			result.Format_S1(GetApp()->resource, IDS_CMDERR_LOAD_ADDON, addOn->addOnName);

			addOnList->DoneWriting();
			return (false);
		}
	}

	// The add-on is now enabled
	addOn->enabled = true;

	// Initialize add-on windows
	GetApp()->addOnWindows->InitAddOnWindows(addOn);
	result.Format("%d,%d", addOn->settings, addOn->display);

	// Write the new settings
	GetApp()->useSettings->WriteStringEntryValue(sectionName, addOn->addOnName, "1," + addOn->fileName);

	// Done with the list
	addOnList->DoneWriting();

	return (true);
}



/******************************************************************************/
/* EndPlayer() will cleanup the player and mixer.                             */
/*                                                                            */
/* Syntax: EndPlayer=<handle>                                                 */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::EndPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Can't cleanup if there isn't any players!
	ASSERT(handle.player != NULL);

	// Cleanup the player and mixer
	handle.player->EndPlayer();

	// Delete the player object
	{
		BMessenger messenger(NULL, handle.player);
		BMessage message(B_QUIT_REQUESTED);

		// Tell the BLooper to quit and delete itself
		// This call is synchronous, so we are sure the
		// object has been deleted when the function
		// call returns
		messenger.SendMessage(&message, &message);
		handle.player = NULL;
	}

	// Set the handle back in the list
	comm->SetFileHandle(uniqueID, handle);

	return (true);
}



/******************************************************************************/
/* GetAuthor() will return the name of the author.                            */
/*                                                                            */
/* Syntax: <name> = GetAuthor=<handle>                                        */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetAuthor(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the name of the module
	result = handle.player->GetAuthor();

	return (true);
}



/******************************************************************************/
/* GetCurrentSong() will return the current song number playing.              */
/*                                                                            */
/* Syntax: <song> = GetCurrentSong=<handle>                                   */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetCurrentSong(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	uint16 songNum;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the number of the current song
	songNum = handle.player->GetCurrentSong();

	// Store the result
	result.SetUNumber(songNum);

	return (true);
}



/******************************************************************************/
/* GetMaxSongNumber() will return the maximum number of sub-songs.            */
/*                                                                            */
/* Syntax: <maxNum> = GetMaxSongNumber=<handle>                               */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetMaxSongNumber(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	uint16 songNum;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the maximum number of sub-songs
	songNum = handle.player->GetMaxSongs();

	// Store the result
	result.SetUNumber(songNum);

	return (true);
}



/******************************************************************************/
/* GetModuleChannels() will return the number of channels used in the module. */
/*                                                                            */
/* Syntax: <num> = GetModuleChannels=<handle>                                 */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetModuleChannels(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	uint16 chanNum;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the number of channels
	chanNum = handle.player->GetChannels();

	// Store the result
	result.SetUNumber(chanNum);

	return (true);
}



/******************************************************************************/
/* GetModuleFormat() will return the format of the module.                    */
/*                                                                            */
/* Syntax: <format> = GetModuleFormat=<handle>                                */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetModuleFormat(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the name of the module
	result = handle.player->GetModuleFormat();

	return (true);
}



/******************************************************************************/
/* GetModuleInformation() will return all the module information of the       */
/*      current playing song.                                                 */
/*                                                                            */
/* Syntax: <modInfo> = GetModuleInformation=<handle>                          */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetModuleInformation(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	PString infoStr;
	const PList<PString> *infoList;
	int32 i, count;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Build the information list
	infoList = handle.player->GetModuleInformation();
	count    = infoList->CountItems();
	for (i = 0; i < count; i++)
		infoStr += infoList->GetItem(i);

	// Store the result
	result = infoStr;

	return (true);
}



/******************************************************************************/
/* GetModuleName() will return the name of the module.                        */
/*                                                                            */
/* Syntax: <name> = GetModuleName=<handle>                                    */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetModuleName(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the name of the module
	result = handle.player->GetModuleName();

	return (true);
}



/******************************************************************************/
/* GetModuleSize() will return the size of the module.                        */
/*                                                                            */
/* Syntax: <size> = GetModuleSize=<handle>                                    */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetModuleSize(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	uint32 modSize;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the size of the module
	modSize = handle.player->GetModuleSize();

	// Store the result
	result.SetUNumber(modSize);

	return (true);
}



/******************************************************************************/
/* GetPlayerName() will return the name of the player.                        */
/*                                                                            */
/* Syntax: <name> = GetPlayerName=<handle>                                    */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetPlayerName(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the name of the module
	result = handle.player->GetPlayerName();

	return (true);
}



/******************************************************************************/
/* GetSongLength() will return the song length of the current playing song.   */
/*                                                                            */
/* Syntax: <length> = GetSongLength=<handle>                                  */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetSongLength(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int16 length;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the length of the song
	length = handle.player->GetSongLength();

	// Store the result
	result.SetNumber(length);

	return (true);
}



/******************************************************************************/
/* GetSongPosition() will return the current song position of the current     */
/*      playing song.                                                         */
/*                                                                            */
/* Syntax: <pos> = GetSongPosition=<handle>                                   */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetSongPosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int16 pos;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the position
	pos = handle.player->GetSongPosition();

	// Store the result
	result.SetNumber(pos);

	return (true);
}



/******************************************************************************/
/* GetTimeList() will return the position time list in milliseconds of the    */
/*      current playing song.                                                 */
/*                                                                            */
/* Syntax: <timeList> = GetTimeList=<handle>                                  */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetTimeList(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	PString timeStr;
	const PList<PTimeSpan> *timeList;
	int32 i, count;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Build the time list
	timeList = handle.player->GetTimeList();
	count    = timeList->CountItems();
	for (i = 0; i < count; i++)
	{
		timeStr.SetNumber64(timeList->GetItem(i).GetTotalMilliSeconds());
		result += timeStr + ",";
	}

	// Remove the last , character
	result.Delete(result.GetLength() - 1);

	return (true);
}



/******************************************************************************/
/* GetTotalTime() will return the total time in milliseconds of the current   */
/*      playing song.                                                         */
/*                                                                            */
/* Syntax: <time> = GetTotalTime=<handle>                                     */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::GetTotalTime(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	PTimeSpan totalTime;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Get the length of the song
	totalTime = handle.player->GetTotalTime();

	// Store the result
	result.SetNumber64(totalTime.GetTotalMilliSeconds());

	return (true);
}



/******************************************************************************/
/* HoldPlaying() will hold or continue the mixer.                             */
/*                                                                            */
/* Syntax: HoldPlaying=<handle>,<holdValue>                                   */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::HoldPlaying(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	bool holdValue;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Convert the hold value
	holdValue = args.GetItem(1).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Hold or continue the mixer
	handle.player->HoldPlaying(holdValue);

	return (true);
}



/******************************************************************************/
/* InitPlayer() will initialize the player and mixer so it's ready to play.   */
/*                                                                            */
/* Syntax: InitPlayer=<handle>                                                */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::InitPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Old player objects must be deleted!
	ASSERT(handle.player == NULL);

	// Create a new player object
	handle.player = new APPlayer();
	if (handle.player == NULL)
		return (false);

	// Initialize the player
	if (!handle.player->InitPlayer(handle, result))
	{
		// Couldn't initialize the player
		//
		// Delete the player object
		{
			BMessenger messenger(NULL, handle.player);
			BMessage message(B_QUIT_REQUESTED);

			// Tell the BLooper to quit and delete itself
			// This call is synchronous, so we are sure the
			// object has been deleted when the function
			// call returns
			messenger.SendMessage(&message, &message);
			handle.player = NULL;
		}

		return (false);
	}

	// Set the handle back in the list
	comm->SetFileHandle(uniqueID, handle);

	return (true);
}



/******************************************************************************/
/* LoadFile() will try to find a player which can understand the file and     */
/*      will then load it into memory.                                        */
/*                                                                            */
/* Syntax: LoadFile=<handle>,<changeType>                                     */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::LoadFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	bool changeType;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Get the rest of the arguments
	changeType = args.GetItem(1).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Load the module
	return (handle.loader->LoadModule(handle.fileName, changeType, result));
}



/******************************************************************************/
/* OpenConfigWindow() will try to open the add-on configuration window.       */
/*                                                                            */
/* Syntax: OpenConfigWindow=<add-on name>                                     */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::OpenConfigWindow(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	PString addOnName;
	APMRSWList<AddOnInfo *> *addOnList;
	AddOnInfo *addOn;
	APAddOnBase *addOnInstance;
	const APConfigInfo *configInfo;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Get the name of the add-on
	addOnName = args.GetItem(0);

	// Look for the add-on
	addOnList = &GetApp()->playerInfo;
	if (!comm->FindAddOn(addOnName, addOnList, addOn))
	{
		addOnList = &GetApp()->agentInfo;
		if (!comm->FindAddOn(addOnName, addOnList, addOn))
		{
			addOnList = &GetApp()->clientInfo;
			if (!comm->FindAddOn(addOnName, addOnList, addOn))
			{
				result.Format_S1(GetApp()->resource, IDS_CMDERR_ADDON_NOT_FOUND, addOnName);
				return (false);
			}
		}
	}

	// Lock the add-on list
	addOnList->WaitToRead();

	// Create an instance from the add-on
	if (addOn->agent == NULL)
		addOnInstance = addOn->loader->CreateInstance();
	else
		addOnInstance = addOn->agent;

	// The add-on has been found and we got an instance.
	// Now get the configuration structure
	configInfo = addOnInstance->GetConfigInfo();
	if (configInfo == NULL)
	{
		// Delete the instance again if allocated
		if (addOn->agent == NULL)
			addOn->loader->DeleteInstance(addOnInstance);

		result.Format_S1(GetApp()->resource, IDS_CMDERR_NO_CONFIG_WINDOW, addOnName);

		addOnList->DoneReading();
		return (false);
	}

	// Open the configuration window
	GetApp()->addOnWindows->ShowAddOnSettings(addOn, addOnInstance, configInfo);

	addOnList->DoneReading();
	return (true);
}



/******************************************************************************/
/* OpenDisplayWindow() will try to open the add-on display window.            */
/*                                                                            */
/* Syntax: OpenDisplayWindow=<add-on name>                                    */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::OpenDisplayWindow(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	PString addOnName;
	APMRSWList<AddOnInfo *> *addOnList;
	AddOnInfo *addOn;
	APAddOnBase *addOnInstance;
	const APDisplayInfo *displayInfo;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Get the name of the add-on
	addOnName = args.GetItem(0);

	// Look for the add-on
	addOnList = &GetApp()->playerInfo;
	if (!comm->FindAddOn(addOnName, addOnList, addOn))
	{
		addOnList = &GetApp()->agentInfo;
		if (!comm->FindAddOn(addOnName, addOnList, addOn))
		{
			addOnList = &GetApp()->clientInfo;
			if (!comm->FindAddOn(addOnName, addOnList, addOn))
			{
				result.Format_S1(GetApp()->resource, IDS_CMDERR_ADDON_NOT_FOUND, addOnName);
				return (false);
			}
		}
	}

	// Lock the add-on list
	addOnList->WaitToRead();

	// Create an instance from the add-on
	if (addOn->agent == NULL)
		addOnInstance = addOn->loader->CreateInstance();
	else
		addOnInstance = addOn->agent;

	// The add-on has been found and we got an instance.
	// Now get the display structure
	displayInfo = addOnInstance->GetDisplayInfo();
	if (displayInfo == NULL)
	{
		// Delete the instance again if allocated
		if (addOn->agent == NULL)
			addOn->loader->DeleteInstance(addOnInstance);

		result.Format_S1(GetApp()->resource, IDS_CMDERR_NO_DISPLAY_WINDOW, addOnName);

		addOnList->DoneReading();
		return (false);
	}

	// Open the display window
	GetApp()->addOnWindows->ShowAddOnDisplayWindow(addOn, addOnInstance, displayInfo);

	addOnList->DoneReading();
	return (true);
}



/******************************************************************************/
/* PausePlayer() will pause the playing temporary.                            */
/*                                                                            */
/* Syntax: PausePlayer=<handle>                                               */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::PausePlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Well, we should have a player
	ASSERT(handle.player != NULL);

	// Start to play the module
	handle.player->PausePlaying();

	return (true);
}



/******************************************************************************/
/* RemoveFile() will remove the file from an intern list.                     */
/*                                                                            */
/* Syntax: RemoveFile=<handle>                                                */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::RemoveFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int32 i, count;
	bool found = false;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Lock the file handle list
	comm->fileHandleList.LockList();

	try
	{
		// Find the file
		count = comm->fileHandleList.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get the handle
			handle = comm->fileHandleList.GetItem(i);

			// Did we found the file?
			if (handle.uniqueID == uniqueID)
			{
				// Yap, remove the handle from the list
				comm->fileHandleList.RemoveItem(i);

				// The player has to be deleted before you remove the file
				ASSERT(handle.player == NULL);

				// Delete the loader object
				delete handle.loader;

				found = true;
				break;
			}
		}
	}
	catch(...)
	{
		comm->fileHandleList.UnlockList();
		throw;
	}

	// Unlock the list again
	comm->fileHandleList.UnlockList();

	if (!found)
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);

	return (found);
}



/******************************************************************************/
/* ResumePlayer() will continue the playing after a pause.                    */
/*                                                                            */
/* Syntax: ResumePlayer=<handle>                                              */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::ResumePlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Well, we should have a player
	ASSERT(handle.player != NULL);

	// Start to play the module
	handle.player->ResumePlaying();

	return (true);
}



/******************************************************************************/
/* SaveSettings() will save the server settings immediately to disk.          */
/*                                                                            */
/* Syntax: SaveSettings=                                                      */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::SaveSettings(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	// Check the arguments
	if (args.CountItems() != 0)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	try
	{
		// Start to clone the "use" settings
		GetApp()->saveSettings->CloneSettings(GetApp()->useSettings);

		// Save the settings
		GetApp()->saveSettings->SaveFile("Server.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		PString err;
		char *errStr;

		err = PSystem::GetErrorString(e.errorNum);
		result.Format(GetApp()->resource, IDS_CMDERR_SAVESETTINGS, e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);

		return (false);
	}

	return (true);
}



/******************************************************************************/
/* SetMixerSettings() will change the mixer settings to use on the added      */
/*      file. Send this command before you send the InitPlayer command.       */
/*                                                                            */
/* Syntax: SetMixerSettings=<handle>,<frequency>,<interpolation>,<dolby>,     */
/*                          <stereoSeparator>,<filter>                        */
/*                                                                            */
/* If any of the values are -1, it means it will keep that current setting.   */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::SetMixerSettings(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int32 freq, interpol, dolby, stereoSep, filter;

	// Check the arguments
	if (args.CountItems() != 6)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Get the arguments
	freq      = args.GetItem(1).GetNumber();
	interpol  = args.GetItem(2).GetNumber();
	dolby     = args.GetItem(3).GetNumber();
	stereoSep = args.GetItem(4).GetNumber();
	filter    = args.GetItem(5).GetNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Change the structure
	if (freq != -1)
		handle.mixerFrequency = freq;

	if (interpol != -1)
	{
		handle.interpolation = interpol;
		if (handle.player != NULL)
			handle.player->SetMixerMode(DMODE_INTERP, handle.interpolation);
	}

	if (dolby != -1)
	{
		handle.dolbyPrologic = dolby;
		if (handle.player != NULL)
			handle.player->SetMixerMode(DMODE_SURROUND, handle.dolbyPrologic);
	}

	if (stereoSep != -1)
	{
		handle.stereoSeparator = stereoSep;
		if (handle.player != NULL)
			handle.player->SetStereoSeparation(handle.stereoSeparator);
	}

	if (filter != -1)
	{
		handle.amigaFilter = filter;
		if (handle.player != NULL)
			handle.player->EnableAmigaFilter(handle.amigaFilter);
	}

	// Set the handle back into the list
	comm->SetFileHandle(uniqueID, handle);

	return (true);
}



/******************************************************************************/
/* SetOutputAgent() will set the agent to use for sound output. Send this     */
/*      command before you send the InitPlayer command.                       */
/*                                                                            */
/* Syntax: SetOutputAgent=<handle>,<agent>                                    */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::SetOutputAgent(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Change the agent
	handle.outputAgent = args.GetItem(1);

	// Set the handle back into the list
	comm->SetFileHandle(uniqueID, handle);

	return (true);
}



/******************************************************************************/
/* SetPosition() will change the song position on the player.                 */
/*                                                                            */
/* Syntax: SetPosition=<handle>,<newPos>                                      */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::SetPosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int16 newPos;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Convert the new position
	newPos = args.GetItem(1).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Hold or continue the mixer
	handle.player->SetSongPosition(newPos);

	return (true);
}



/******************************************************************************/
/* SetVolume() will change the master volume in the mixer.                    */
/*                                                                            */
/* Syntax: SetVolume=<handle>,<newVol>                                        */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::SetVolume(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	uint16 newVol;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Convert the new volume
	newVol = args.GetItem(1).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Hold or continue the mixer
	handle.player->SetVolume(newVol);

	return (true);
}



/******************************************************************************/
/* StartedNormally() will check to see if the server started normally or not. */
/*                                                                            */
/* Syntax: StartedNormally=                                                   */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::StartedNormally(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	// Check the arguments
	if (args.CountItems() != 0)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	if (GetApp()->GotStartupMessage())
		result = "0";
	else
		result = "1";

	return (true);
}



/******************************************************************************/
/* StartPlayer() will start to play the module loaded.                        */
/*                                                                            */
/* Syntax: StartPlayer=<handle>,<song>                                        */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::StartPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;
	int16 songNum;

	// Check the arguments
	if (args.CountItems() != 2)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Get the start song
	songNum = args.GetItem(1).GetNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Well, we should have a player
	ASSERT(handle.player != NULL);

	// Start to play the module
	handle.player->StartPlaying(songNum);

	return (true);
}



/******************************************************************************/
/* StopPlayer() will stop playing the module loaded.                          */
/*                                                                            */
/* Syntax: StopPlayer=<handle>                                                */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::StopPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Well, we should have a player
	ASSERT(handle.player != NULL);

	// Start to play the module
	handle.player->StopPlaying();

	return (true);
}



/******************************************************************************/
/* UnloadFile() will free the file from memory and stop playing it.           */
/*                                                                            */
/* Syntax: UnloadFile=<handle>                                                */
/*                                                                            */
/* Input:  "comm" is a pointer to the communication object.                   */
/*         "looper" is a pointer to the client looper that sent this command. */
/*         "args" is a list with all the arguments                            */
/*         "result" is where the result should be stored.                     */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool APClientCommunication::UnloadFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result)
{
	APFileHandle handle;
	uint32 uniqueID;

	// Check the arguments
	if (args.CountItems() != 1)
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_ARGLIST);
		return (false);
	}

	// Convert the unique ID
	uniqueID = args.GetItem(0).GetUNumber();

	// Find the handle structure
	if (!comm->FindFileHandle(uniqueID, handle))
	{
		result.LoadString(GetApp()->resource, IDS_CMDERR_INVALID_HANDLE);
		return (false);
	}

	// Free the module from memory
	handle.loader->FreeModule();
	return (true);
}
