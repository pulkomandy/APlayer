/******************************************************************************/
/* APlayer application class.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PSystem.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PSkipList.h"
#include "PPriorityList.h"
#include "PDirectory.h"

// APlayerKit headers
#include "APGlobal.h"
#include "APGlobalData.h"
#include "APList.h"
#include "APAddOns.h"

// Server headers
#include "APAddOnLoader.h"
#include "APAddOnWindows.h"
#include "APApplication.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/
APGlobal *globalData = NULL;



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APApplication::APApplication(const char *signature) : BApplication(signature)
{
	app_info appInfo;

	// Initialize member variables
	resource     = NULL;
	useSettings  = NULL;
	saveSettings = NULL;
	client       = NULL;
	addOnWindows = NULL;

	refMsg       = NULL;

	// Find the file name of the application
	if (GetAppInfo(&appInfo) == B_OK)
	{
		// Allocate resources
		resource = new PResource(appInfo.ref.name);
			if (resource == NULL)
				throw PMemoryException();
	}
	else
	{
		// Tell the server to quit
		PostMessage(B_QUIT_REQUESTED);
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APApplication::~APApplication(void)
{
	// Delete the resources again
	delete resource;
}



/******************************************************************************/
/* GetAddOnInfo() will return a list with all the add-ons of the type given.  */
/*                                                                            */
/* Input:  "type" is the add-on type to return.                               */
/*         "list" is a reference to the list to store the information.        */
/******************************************************************************/
void APApplication::GetAddOnInfo(APAddOnType type, APList<APAddOnInformation *> &list)
{
	APMRSWList<AddOnInfo *> *infoList;
	AddOnInfo *info;
	APAddOnInformation *newInfo;
	int32 i, count;

	// Start to lock the list and empty it
	list.LockList();
	list.MakeEmpty();

	// Find the list with the information to retrieve
	switch (type)
	{
		case apPlayer:
		{
			infoList = &GetApp()->playerInfo;
			break;
		}

		case apAgent:
		{
			infoList = &GetApp()->agentInfo;
			break;
		}

		case apClient:
		{
			infoList = &GetApp()->clientInfo;
			break;
		}

		case apConverter:
		{
			infoList = &GetApp()->converterInfo;
			break;
		}

		default:
		{
			ASSERT(false);
			return;
		}
	}

	// Lock the information list
	infoList->WaitToRead();

	// Now extract the information needed and build the new list
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		info = infoList->GetItem(i);

		newInfo = new APAddOnInformation;
		if (newInfo == NULL)
			throw PMemoryException();

		newInfo->type        = type;
		newInfo->name        = info->addOnName;
		newInfo->description = info->description;
		newInfo->version     = info->version;
		newInfo->enabled     = info->enabled;
		newInfo->settings    = info->settings;
		newInfo->display     = info->display;
		newInfo->pluginFlags = info->pluginFlags;

		list.AddTail(newInfo);
	}

	// Unlock the lists again
	infoList->DoneReading();
	list.UnlockList();
}



/******************************************************************************/
/* FreeAddOnInfo() will free all the elements in the list given.              */
/*                                                                            */
/* Input:  "list" is a reference to the list to free.                         */
/******************************************************************************/
void APApplication::FreeAddOnInfo(APList<APAddOnInformation *> &list)
{
	int32 i, count;

	// Lock the list
	list.LockList();

	// Free all the elements
	count = list.CountItems();
	for (i = 0; i < count; i++)
		delete list.GetAndRemoveItem(0);

	// Unlock the list again
	list.UnlockList();
}



/******************************************************************************/
/* GetInstInfo() will lock the list with the instruments and return a pointer */
/*      to it.                                                                */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/*                                                                            */
/* Output: A pointer to the list or NULL.                                     */
/******************************************************************************/
const PList<APInstInfo *> *APApplication::GetInstInfo(uint32 fileHandle)
{
	return (GetApp()->client->GetInstrumentList(fileHandle));
}



/******************************************************************************/
/* UnlockInstInfo() will unlock the instrument list.                          */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/******************************************************************************/
void APApplication::UnlockInstInfo(uint32 fileHandle)
{
	GetApp()->client->UnlockInstrumentList(fileHandle);
}



/******************************************************************************/
/* GetSampInfo() will lock the list with the samples and return a pointer to  */
/*      it.                                                                   */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/*                                                                            */
/* Output: A pointer to the list or NULL.                                     */
/******************************************************************************/
const PList<APSampleInfo *> *APApplication::GetSampInfo(uint32 fileHandle)
{
	return (GetApp()->client->GetSampleList(fileHandle));
}



/******************************************************************************/
/* UnlockSampInfo() will unlock the sample list.                              */
/*                                                                            */
/* Input:  "fileHandle" is the file handle to the module.                     */
/******************************************************************************/
void APApplication::UnlockSampInfo(uint32 fileHandle)
{
	GetApp()->client->UnlockSampleList(fileHandle);
}



/******************************************************************************/
/* GetAddOnInst() will allocate an add-on instance and return the pointer.    */
/*                                                                            */
/* Input:  "name" is the name of the add-on to allocate.                      */
/*         "type" is the type of the add-on.                                  */
/*         "index" is a pointer to store the add-on index.                    */
/*                                                                            */
/* Output: A pointer to the add-on or NULL if it couldn't be found.           */
/******************************************************************************/
APAddOnBase *APApplication::GetAddOnInst(PString name, APAddOnType type, int32 *index)
{
	APMRSWList<AddOnInfo *> *infoList;
	AddOnInfo *info = NULL;
	int32 i, count;
	bool found = false;
	APAddOnBase *addOn = NULL;

	// Find the list to use
	switch (type)
	{
		case apPlayer:
		{
			infoList = &GetApp()->playerInfo;
			break;
		}

		case apAgent:
		{
			infoList = &GetApp()->agentInfo;
			break;
		}

		case apClient:
		{
			infoList = &GetApp()->clientInfo;
			break;
		}

		case apConverter:
		{
			infoList = &GetApp()->converterInfo;
			break;
		}

		default:
		{
			ASSERT(false);
			return (NULL);
		}
	}

	// Lock the list
	infoList->WaitToRead();

	// Search after the name to see if we can find it
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		info = infoList->GetItem(i);
		if (info->addOnName.CompareNoCase(name) == 0)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		// Found it, now allocate the instance
		addOn  = info->loader->CreateInstance();
		*index = info->index;
	}

	// Unlock the list again
	infoList->DoneReading();

	return (addOn);
}



/******************************************************************************/
/* DeleteAddOnInst() will delete the add-on instance given.                   */
/*                                                                            */
/* Input:  "name" is the name of the add-on.                                  */
/*         "type" is the type of the add-on.                                  */
/*         "addOn" is a pointer to the object to delete.                      */
/******************************************************************************/
void APApplication::DeleteAddOnInst(PString name, APAddOnType type, APAddOnBase *addOn)
{
	APMRSWList<AddOnInfo *> *infoList;
	AddOnInfo *info = NULL;
	int32 i, count;
	bool found = false;

	// Find the list to use
	switch (type)
	{
		case apPlayer:
		{
			infoList = &GetApp()->playerInfo;
			break;
		}

		case apAgent:
		{
			infoList = &GetApp()->agentInfo;
			break;
		}

		case apClient:
		{
			infoList = &GetApp()->clientInfo;
			break;
		}

		case apConverter:
		{
			infoList = &GetApp()->converterInfo;
			break;
		}

		default:
		{
			ASSERT(false);
			return;
		}
	}

	// Lock the list
	infoList->WaitToRead();

	// Search after the name to see if we can find it
	count = infoList->CountItems();
	for (i = 0; i < count; i++)
	{
		info = infoList->GetItem(i);
		if (info->addOnName.CompareNoCase(name) == 0)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		// Found it, now delete the instance
		info->loader->DeleteInstance(addOn);
	}

	// Unlock the list again
	infoList->DoneReading();
}



/******************************************************************************/
/* GotStartupMessage() will check if the server has a startup message or not. */
/*                                                                            */
/* Output: True means there is a startup message, false if not.               */
/******************************************************************************/
bool APApplication::GotStartupMessage(void) const
{
	return (refMsg != NULL ? true : false);
}



/******************************************************************************/
/* PlugAgentIn() will plug the agent in the right places.                     */
/*                                                                            */
/* Input:  "info" is a pointer to the agent information.                      */
/*         "agent" is a pointer to an agent instance or NULL if the function  */
/*         should create the instance itself.                                 */
/******************************************************************************/
void APApplication::PlugAgentIn(AddOnInfo *info, APAddOnAgent *agent)
{
	APAddOnAgent *testAgent;
	bool keepInstance = false;

	// Create a new instance of the agent
	if (agent == NULL)
	{
		testAgent = (APAddOnAgent *)info->loader->CreateInstance();
		if (testAgent == NULL)
			throw PMemoryException();
	}
	else
		testAgent = agent;

	// Lock the plug-in lists
	pluginLock.WaitToWrite();

	// Search for all possible plug-in functions
	//
	// Converter:
	if (info->pluginFlags & apaConverter)
	{
		converterAgents.InsertItem(info, testAgent->GetPluginPriority(apaConverter));
		keepInstance = true;
	}

	// Decruncher:
	if (info->pluginFlags & apaDecruncher)
	{
		decruncherAgents.InsertItem(info, testAgent->GetPluginPriority(apaDecruncher));
		keepInstance = true;
	}

	// Virtual Mixer:
	if (info->pluginFlags & apaVirtualMixer)
	{
		virtualMixerAgents.InsertItem(info, testAgent->GetPluginPriority(apaVirtualMixer));
		keepInstance = true;
	}

	// Sound Output:
	if (info->pluginFlags & apaSoundOutput)
		soundOutputAgents.InsertItem(info->addOnName, info);

	// DSP:
	if (info->pluginFlags & apaDSP)
	{
		dspAgents.InsertItem(info, testAgent->GetPluginPriority(apaDSP));
		keepInstance = true;
	}

	// Visual:
	if (info->pluginFlags & apaVisual)
	{
		visualAgents.InsertItem(info, testAgent->GetPluginPriority(apaVisual));
		keepInstance = true;
	}

	// Unlock again
	pluginLock.DoneWriting();

	// Should we delete the instance?
	if (!keepInstance)
	{
		// Do only delete the instance if we have created it self
		if (agent == NULL)
			info->loader->DeleteInstance(testAgent);
	}
	else
	{
		// Well, we should so we initialize the agent
		info->agent = testAgent;

		if (!testAgent->InitAgent(info->index))
			PlugAgentOut(info, true);
	}
}



/******************************************************************************/
/* PlugAgentOut() will plug the agent out from the right places.              */
/*                                                                            */
/* Input:  "info" is a pointer to the agent information.                      */
/*         "deleteInstance" is true if you want the function to delete the    */
/*         instance, false if you won't.                                      */
/******************************************************************************/
void APApplication::PlugAgentOut(AddOnInfo *info, bool deleteInstance)
{
	// Lock the plug-in lists
	pluginLock.WaitToWrite();

	// Stop the agent
	if (info->agent != NULL)
	{
		// Yup!
		info->agent->EndAgent(info->index);
	}

	// Now search for all possible plug-in functions
	//
	// Converter:
	if (info->pluginFlags & apaConverter)
		converterAgents.FindAndRemoveItem(info, info->agent->GetPluginPriority(apaConverter));

	// Decruncher:
	if (info->pluginFlags & apaDecruncher)
		decruncherAgents.FindAndRemoveItem(info, info->agent->GetPluginPriority(apaDecruncher));

	// Virtual Mixer:
	if (info->pluginFlags & apaVirtualMixer)
		virtualMixerAgents.FindAndRemoveItem(info, info->agent->GetPluginPriority(apaVirtualMixer));

	// Sound Output:
	if (info->pluginFlags & apaSoundOutput)
		soundOutputAgents.RemoveItem(info->addOnName);

	// DSP:
	if (info->pluginFlags & apaDSP)
		dspAgents.FindAndRemoveItem(info, info->agent->GetPluginPriority(apaDSP));

	// Visual:
	if (info->pluginFlags & apaVisual)
		visualAgents.FindAndRemoveItem(info, info->agent->GetPluginPriority(apaVisual));

	// Unlock again
	pluginLock.DoneWriting();

	// Should we delete the instance
	if ((info->agent != NULL) && deleteInstance)
	{
		info->loader->DeleteInstance(info->agent);
		info->agent = NULL;
	}
}



/******************************************************************************/
/* ReadyToRun() is called after all messages the application receives         */
/*      on-launch have been responded to.                                     */
/******************************************************************************/
void APApplication::ReadyToRun(void)
{
	try
	{
		// Initialize the APlayerKit
		InitAPlayerKit();

		// Load the server settings and set missing settings to default values
		InitSettings();

		// Load converter add-ons
		LoadAddOns("Converter Add-Ons", "Converters", converterInfo, false);

		// Load player add-ons
		LoadAddOns("Player Add-Ons", "Players", playerInfo, false);

		// Load agent add-ons
		LoadAddOns("Agent Add-Ons", "Agents", agentInfo, true);

		// Load client add-ons
		LoadAddOns("Client Add-Ons", "Clients", clientInfo, false);

		// Initialize client connections
		InitClientConnections();

		// Plug-in all the agent add-ons
		PlugInAgents();

		// Initialize add-on windows structures in the server
		InitAddOnWindows();

		// Build the file types list
		globalData->fileTypes->BuildFileTypeList();

		// Initialize and start the client add-ons
		StartClientAddOns();

		// Well, do we have a start message waiting? If so, send it
		if (refMsg != NULL)
		{
			SendClickedFiles(refMsg);
			delete refMsg;
			refMsg = NULL;
		}
	}
	catch(PMemoryException e)
	{
		// Show the error
		APError::ShowError(IDS_ERR_NO_MEMORY);

		// Tell the server to quit
		PostMessage(B_QUIT_REQUESTED);
	}
	catch(PFileException e)
	{
		PString err;
		char *errStr, *nameStr;

		// Show the error
		err = PSystem::GetErrorString(e.errorNum);
		APError::ShowError(IDS_ERR_FILE, (nameStr = e.fileName.GetString()), e.errorNum, (errStr = err.GetString()));
		err.FreeBuffer(errStr);
		e.fileName.FreeBuffer(nameStr);

		// Tell the server to quit
		PostMessage(B_QUIT_REQUESTED);
	}
	catch(PUserException e)
	{
		// Do nothing, just exit
		PostMessage(B_QUIT_REQUESTED);
	}
	catch(PException e)
	{
		// Show the error
		APError::ShowError(IDS_ERR_EXCEPTION);

		// Tell the server to quit
		PostMessage(B_QUIT_REQUESTED);
	}
}



/******************************************************************************/
/* RefsReceived() is called when the user has clicked on a file which is      */
/*         associated to APlayer.                                             */
/*                                                                            */
/* Input:  "message" is the message with the file information.                */
/******************************************************************************/
void APApplication::RefsReceived(BMessage *message)
{
	// Send the message to the main window only if APlayer is running, else
	// save it for later use
	if (IsLaunching())
		refMsg = new BMessage(*message);
	else
		SendClickedFiles(message);
}



/******************************************************************************/
/* QuitRequested() is called when the program is about to exit.               */
/******************************************************************************/
bool APApplication::QuitRequested(void)
{
	// Close add-on windows opened from the server
	CleanupAddOnWindows();

	// Stop client add-ons
	StopClientAddOns();

	// Stop client connections
	CleanupClientConnections();

	// Stop agents
	PlugOutAgents();

	// Unload client add-ons
	UnloadAddOns(clientInfo);

	// Unload agent add-ons
	UnloadAddOns(agentInfo);

	// Unload player add-ons
	UnloadAddOns(playerInfo);

	// Unload converter add-ons
	UnloadAddOns(converterInfo);

	// Cleanup the settings
	CleanupSettings();

	// Cleanup the APlayerKit
	CleanupAPlayerKit();

	return (true);
}



/******************************************************************************/
/* InitAPlayerKit() creates the global data instance and initialize the       */
/*      APlayerKit library.                                                   */
/******************************************************************************/
void APApplication::InitAPlayerKit(void)
{
	// Create an instance of the global data class
	globalData = new APGlobal();
	if (globalData == NULL)
		throw PMemoryException();

	// Initialize function pointers
	globalData->SetFunctionPointers(GetAddOnInfo, FreeAddOnInfo, GetInstInfo, UnlockInstInfo, GetSampInfo, UnlockSampInfo, GetAddOnInst, DeleteAddOnInst);
}



/******************************************************************************/
/* CleanupAPlayerKit() cleanup anything needed for the APlayerKit.            */
/******************************************************************************/
void APApplication::CleanupAPlayerKit(void)
{
	// Delete the global data instance
	delete globalData;
	globalData = NULL;
}



/******************************************************************************/
/* InitSettings() load the settings from disk and initialize all missing      */
/*      settings to default values.                                           */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APApplication::InitSettings(void)
{
	// Create instances of the settings class
	useSettings = new PSettings();
	if (useSettings == NULL)
		throw PMemoryException();

	saveSettings = new PSettings();
	if (saveSettings == NULL)
		throw PMemoryException();

	try
	{
		// Try to load the settings file
		saveSettings->LoadFile("Server.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		if (e.errorNum != P_FILE_ERR_ENTRY_NOT_FOUND)
			throw;
	}

	// Copy the settings to the "use" settings
	useSettings->CloneSettings(saveSettings);
}



/******************************************************************************/
/* CleanupSettings() destroy the settings in the memory.                      */
/******************************************************************************/
void APApplication::CleanupSettings(void)
{
	if (saveSettings != NULL)
	{
		try
		{
			// Save the settings
			saveSettings->SaveFile("Server.ini", "Polycode", "APlayer");
		}
		catch(...)
		{
			;
		}
	}

	// Delete the setting instances
	delete saveSettings;
	delete useSettings;

	saveSettings = NULL;
	useSettings  = NULL;
}



/******************************************************************************/
/* InitAddOnWindows() will initialize all the objects and structures used to  */
/*      hold the add-on windows.                                              */
/******************************************************************************/
void APApplication::InitAddOnWindows(void)
{
	int32 i, count;
	AddOnInfo *info;
	const APDisplayInfo *displayInfo;

	// Create an instance of the add-on window handler
	addOnWindows = new APAddOnWindows();
	if (addOnWindows == NULL)
		throw PMemoryException();

	// Find configuration and display window add-ons
	addOnWindows->FindWindowAddOns(playerInfo);
	addOnWindows->FindWindowAddOns(agentInfo);
	addOnWindows->FindWindowAddOns(clientInfo);

	// Open visual agent windows
	pluginLock.WaitToRead();

	count = visualAgents.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get the agent information
		info = visualAgents.GetItem(i);

		// Get the window information
		displayInfo = info->agent->GetDisplayInfo();
		if (displayInfo->openIt)
			addOnWindows->ShowAddOnDisplayWindow(info, info->agent, displayInfo);
		else
			delete displayInfo->window;
	}

	// Down with the list
	pluginLock.DoneReading();
}



/******************************************************************************/
/* CleanupAddOnWindows() cleanup anything needed for the add-on windows       */
/*      handler.                                                              */
/******************************************************************************/
void APApplication::CleanupAddOnWindows(void)
{
	// Close all the add-on windows
	if (addOnWindows != NULL)
	{
		addOnWindows->CloseWindows();

		// Delete the instance
		delete addOnWindows;
		addOnWindows = NULL;
	}
}



/******************************************************************************/
/* InitClientConnections() creates, initialize and starts the client          */
/*      connections object.                                                   */
/******************************************************************************/
void APApplication::InitClientConnections(void)
{
	// Create an instance of the client communication class
	client = new APClientCommunication();
	if (client == NULL)
		throw PMemoryException();

	// Start the communications
	client->Start();
}



/******************************************************************************/
/* CleanupClientConnections() stops and clean up the client connections.      */
/******************************************************************************/
void APApplication::CleanupClientConnections(void)
{
	// Tell the client communication object to stop and delete itself
	if (client != NULL)
	{
		client->Stop();
		client = NULL;
	}
}



/******************************************************************************/
/* StartClientAddOns() will initialize and start all the client add-ons.      */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void APApplication::StartClientAddOns(void)
{
	int32 i, count;
	AddOnInfo *info;
	APAddOnClient *client = NULL;

	// Start to lock the client list
	clientInfo.WaitToWrite();

	try
	{
		// Traverse all the clients
		count = clientInfo.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get information about the add-on
			info = clientInfo.GetItem(i);

			// Set the agent/client flag
			info->isAgent  = true;
			info->isClient = true;

			// Is the add-on enabled?
			if (info->enabled)
			{
				// Allocate an instance of the add-on
				client = (APAddOnClient *)info->loader->CreateInstance();
				if (client == NULL)
					throw PMemoryException();

				// Remember the pointer to the instance
				info->agent = client;

				// Initialize the client
				if (!client->InitClient(info->index))
				{
					char *nameStr;

					APError::ShowError(IDS_ERR_STARTCLIENT, (nameStr = info->addOnName.GetString()));
					info->addOnName.FreeBuffer(nameStr);
					throw PUserException();
				}

				// Plug the client in as an agent
				PlugAgentIn(info, client);
			}
		}
	}
	catch(...)
	{
		// Unlock list
		clientInfo.DoneWriting();
		throw;
	}

	// Done, unlock the list again
	clientInfo.DoneWriting();

	// Now, start all the clients
	clientInfo.WaitToRead();

	try
	{
		// Traverse all the clients
		count = clientInfo.CountItems();
		for (i = 0; i < count; i++)
		{
			// Get information about the add-on
			info = clientInfo.GetItem(i);

			// Is the add-on enabled?
			if (info->enabled)
			{
				// Start the client
				client->Start(info->index);
			}
		}
	}
	catch(...)
	{
		// Unlock list
		clientInfo.DoneReading();
		throw;
	}

	// Done, unlock the list again
	clientInfo.DoneReading();
}



/******************************************************************************/
/* StopClientAddOns() will tell all the client add-ons to stop.               */
/******************************************************************************/
void APApplication::StopClientAddOns(void)
{
	int32 i, count;
	AddOnInfo *info;

	// Lock the client list
	clientInfo.WaitToWrite();

	// Traverse all the clients and stop them
	count = clientInfo.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get information about the client
		info = clientInfo.GetItem(i);

		if (info->agent != NULL)
		{
			// Stop the client
			((APAddOnClient *)info->agent)->EndClient(info->index);

			// Plug-out the agent part
			PlugAgentOut(info, false);

			// Delete the instance
			info->loader->DeleteInstance(info->agent);

			// Clear pointers
			info->agent = NULL;
		}
	}

	// Unlock the list again
	clientInfo.DoneWriting();
}



/******************************************************************************/
/* PlugInAgents() will plug-in all the agent add-ons the right places.        */
/******************************************************************************/
void APApplication::PlugInAgents(void)
{
	int32 i, count;
	AddOnInfo *info;

	// Lock the list
	agentInfo.WaitToWrite();

	// Get the number of agents loaded
	count = agentInfo.CountItems();

	// Add all the functions to the function lists
	for (i = 0; i < count; i++)
	{
		// Get agent information
		info = agentInfo.GetItem(i);

		// Set the agent flag
		info->isAgent = true;

		// Is the add-on enabled?
		if (info->enabled)
		{
			// Plug-in the agent the places it need
			PlugAgentIn(info, NULL);
		}
	}

	// Unlock it again
	agentInfo.DoneWriting();
}



/******************************************************************************/
/* PlugOutAgents() will plug-out all the agent add-ons and stop them.         */
/******************************************************************************/
void APApplication::PlugOutAgents(void)
{
	int32 i, count;
	AddOnInfo *info;

	// Lock the client list
	agentInfo.WaitToWrite();

	// Traverse all the agents and stop them
	count = agentInfo.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get agent information
		info = agentInfo.GetItem(i);

		// Does the agent have an instance?
		if (info->agent != NULL)
		{
			// Yap, stop, unplug and delete the agent
			PlugAgentOut(info, true);
		}
	}

	// Unlock it again
	agentInfo.DoneWriting();
}



/******************************************************************************/
/* LoadAddOns() will load all the add-ons in the directory given + initialize */
/*      the section.                                                          */
/*                                                                            */
/* Input:  "section" is the section where the add-ons are stored.             */
/*         "addOnDir" is the directory where the add-on files are stored.     */
/*         "infoList" is a reference to the list where the add-on information */
/*         should be stored.                                                  */
/*         "agents" indicates if we scan after agents or not.                 */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void APApplication::LoadAddOns(PString section, PString addOnDir, APMRSWList<AddOnInfo *> &infoList, bool agents)
{
	PDirectory dir;
	PDirectory::PEntryType addOnType;
	PString addOnName, fileName;
	PString addOnNameTemp, fileNameTemp;
	PString temp;
	char *nameStr;
	APAddOnLoader *addOn;
	AddOnInfo *info;
	bool enabled, flush;
	int32 i, j, count;
	bool found;

	// Set the search directory
	dir.FindDirectory(PDirectory::pLaunch);
	dir.Append("add-ons");
	dir.Append(addOnDir);

	// First comes the delete loop. It will scan the section and see
	// if the file still exists and if it doesn't, it will remove
	// the entry from the section
	for (i = 0; ; i++)
	{
		// Read the entry
		if (!ReadAddOnEntry(section, i, addOnName, fileName, enabled))
			break;

		// See if the file still exist
		if (!dir.Contains(fileName, PDirectory::pFile))
		{
			// It doesn't, so remove the entry
			useSettings->RemoveEntry(section, addOnName);
			saveSettings->RemoveEntry(section, addOnName);
			i--;
		}
	}

	// Now scan the add-on directory and load all the add-ons
	// stored in it. Also update the settings file with new
	// add-ons
	dir.InitEnum(PDirectory::pFile);

	try
	{
		while (dir.GetNextEntry(fileName, addOnType))
		{
			// Allocate new add-on object
			addOn = new APAddOnLoader(addOnDir + P_DIRSLASH_STR + fileName);
			if (addOn == NULL)
				throw PMemoryException();

			try
			{
				// Load the add-on
				if (addOn->Load())
				{
					// First check to see if we can use this add-on at all
					APAddOnBase *testAddOn;

					testAddOn = addOn->CreateInstance();

					if (testAddOn->aplayerVersion < APLAYER_CURRENT_VERSION)
					{
						// Show error
						APError::ShowError(IDS_ERR_ADDONVERSION, (nameStr = fileName.GetString()), testAddOn->aplayerVersion, APLAYER_CURRENT_VERSION);
						fileName.FreeBuffer(nameStr);

						// Cleanup
						addOn->DeleteInstance(testAddOn);
						addOn->Unload();
						delete addOn;
						continue;			// Go the the next add-on
					}

					// Now we check the section to see see if any add-ons
					// has been removed from the file
					for (i = 0; ; i++)
					{
						// Read the entry
						if (!ReadAddOnEntry(section, i, addOnNameTemp, fileNameTemp, enabled))
							break;

						// Do we have a match on the file name
						if (fileNameTemp == fileName)
						{
							// Found an entry which use the same file name.
							// Now test to see if the add-on name still
							// exist in the add-on
							if (!addOn->NameExists(addOnNameTemp, testAddOn))
							{
								// Nup, delete the entry
								useSettings->RemoveEntry(section, addOnNameTemp);
								saveSettings->RemoveEntry(section, addOnNameTemp);
								i--;
							}
						}
					}

					// Check to see if any new add-ons has been added
					flush = true;

					count = testAddOn->GetCount();
					for (i = 0; i < count; i++)
					{
						// Get the add-on name
						addOnName = testAddOn->GetName(i);

						// Does the add-on name exists already in the
						// settings file?
						found = false;
						for (j = 0; ; j++)
						{
							// Read the entry
							if (!ReadAddOnEntry(section, j, addOnNameTemp, fileNameTemp, enabled))
								break;

							if ((fileNameTemp == fileName) && (addOnNameTemp == addOnName))
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							// The add-on isn't in the settings file,
							// but if we scan after agents and it's a DSP
							// agent, disable it
							if (agents && (testAddOn->GetSupportFlags(i) & apaDSP))
								enabled = false;
							else
								enabled = true;

							// Add a new entry in the settings file
							temp.Format("%d,%s", enabled ? 1 : 0, (nameStr = fileName.GetString()));
							fileName.FreeBuffer(nameStr);
							useSettings->WriteStringEntryValue(section, addOnName, temp);
							saveSettings->WriteStringEntryValue(section, addOnName, temp);
						}

						// Allocate the add-on info structure
						info = new AddOnInfo;
						if (info == NULL)
							throw PMemoryException();

						// Fill it out
						info->fileName    = fileName;
						info->addOnName   = addOnName;
						info->description = testAddOn->GetDescription(i);
						info->version     = testAddOn->GetVersion();
						info->index       = i;
						info->enabled     = enabled;
						info->settings    = false;
						info->display     = false;
						info->agent       = NULL;
						info->pluginFlags = testAddOn->GetSupportFlags(i);
						info->isAgent     = false;
						info->isClient    = false;

						if (enabled)
						{
							info->allocated = flush;
							info->loader    = addOn;
							flush = false;
						}
						else
						{
							info->allocated = false;
							info->loader    = NULL;
						}

						// Add the info to the info list
						infoList.WaitToWrite();
						infoList.AddTail(info);
						infoList.DoneWriting();
					}

					// We don't need the instance anymore
					addOn->DeleteInstance(testAddOn);

					// If all add-ons in the library file isn't active,
					// flush the file from memory
					if (flush)
					{
						addOn->Unload();
						delete addOn;
					}
				}
				else
					delete addOn;
			}
			catch(...)
			{
				delete addOn;
				throw;
			}
		}
	}
	catch(...)
	{
		dir.EndEnum();
		throw;
	}

	dir.EndEnum();
}



/******************************************************************************/
/* UnloadAddOns() will unload all the add-ons in the list given.              */
/*                                                                            */
/* Input:  "infoList" is a reference to the list where the add-on information */
/*         are stored.                                                        */
/******************************************************************************/
void APApplication::UnloadAddOns(APMRSWList<AddOnInfo *> &infoList)
{
	int32 i, count;
	AddOnInfo *info;

	// Lock the list
	infoList.WaitToWrite();

	// Unload all the add-ons
	count = infoList.CountItems();
	for (i = 0; i < count; i++)
	{
		// Get information about the add-on
		info = infoList.GetItem(i);

		// Is this add-on the one, where we need to deallocate?
		if (info->allocated)
		{
			// Yep, do it
			info->loader->Unload();
			delete info->loader;
		}

		// Delete the info object
		delete info;
	}

	// Empty the list
	infoList.MakeEmpty();

	// Unlock list again
	infoList.DoneWriting();
}



/******************************************************************************/
/* ReadAddOnEntry() will read and parse a single add-on entry in the section  */
/*      given.                                                                */
/*                                                                            */
/* Input:  "section" is the section where the entry should be read from.      */
/*         "entryNum" is the entry number to read.                            */
/*         "addOnName" is a reference where the add-on name will be stored.   */
/*         "fileName" is a reference where the file name will be stored.      */
/*         "enabled" is a reference where the enable/disable flag will be     */
/*         stored.                                                            */
/*                                                                            */
/* Output: True if the entry could be read, false if not.                     */
/******************************************************************************/
bool APApplication::ReadAddOnEntry(PString section, int32 entryNum, PString &addOnName, PString &fileName, bool &enabled) const
{
	PString value;
	int32 index;

	// Try to read the entry
	value = useSettings->GetStringEntryValue(section, entryNum, addOnName);
	if (value.IsEmpty())
		return (false);

	// Parse the entry value. First find the enable/disable flag
	index = value.Find(',');
	if (index == -1)
		return (false);

	// And store it
	enabled = (value.Left(index).GetUNumber() == 0) ? false: true;

	// Now store the file name
	fileName = value.Mid(index + 1);

	return (true);
}



/******************************************************************************/
/* SendClickedFiles() will build and send a "ClickedFiles" command to all the */
/*      clients.                                                              */
/*                                                                            */
/* Input:  "files" is a pointer to the message holding the files.             */
/******************************************************************************/
void APApplication::SendClickedFiles(BMessage *files)
{
	PString command;
	entry_ref entRef;
	BEntry entry;
	BPath path;
	int32 i;

	// Set the command
	command = "ClickedFiles=";

	// Traverse all the files in the message
	for (i = 0; (files->FindRef("refs", i, &entRef) == B_OK); i++)
	{
		entry.SetTo(&entRef);
		entry.GetPath(&path);

		// Add the file name as argument
		command = APServerCommunication::AddArgument(command, path.Path());
	}

	// Send the command to the clients
	client->SendCommandToAll(command);
}
