/******************************************************************************/
/* MainWindowSystem Client Interface.                                         */
/*                                                                            */
/* Programmed by Thomas Neumann.                                              */
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
#include "PSynchronize.h"
#include "PThread.h"
#include "PSettings.h"
#include "PAlert.h"
#include "PDirectory.h"
#include "PFile.h"
#include "PSocket.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "MainWindowSystem.h"
#include "APLoader.h"
#include "APWindowMain.h"
#include "APWindowUpdate.h"
#include "APWindowAbout.h"
#include "APWindowModuleInfo.h"
#include "APWindowSampleInfo.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define ClientVersion		1.80f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MainWindowSystem::MainWindowSystem(APGlobalData *global, PString fileName) : APAddOnClient(global)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Initialize member variables
	useSettings  = NULL;
	saveSettings = NULL;
	closeWindow  = false;
	playerInfo   = NULL;
	loader       = NULL;
	mainWin      = NULL;
	updateWin    = NULL;
	aboutWin     = NULL;
	settingsWin  = NULL;
	infoWin      = NULL;
	sampWin      = NULL;
	checkThread  = NULL;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
MainWindowSystem::~MainWindowSystem(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: Is the version of the add-on.                                      */
/******************************************************************************/
float MainWindowSystem::GetVersion(void)
{
	return (ClientVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 MainWindowSystem::GetSupportFlags(int32 index)
{
	return (apaVirtualMixer);
}



/******************************************************************************/
/* GetName() returns the name of the add-on.                                  */
/*                                                                            */
/* Input:  "index" is the index to the add-on name to return.                 */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString MainWindowSystem::GetName(int32 index)
{
	PString name;

	// Get the add-on name from the resource
	name.LoadString(res, IDS_WINDOW_NAME);

	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the add-on.                    */
/*                                                                            */
/* Input:  "index" is the index to the add-on description to return.          */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MainWindowSystem::GetDescription(int32 index)
{
	PString description;

	// Get the add-on name from the resource
	description.LoadString(res, IDS_WINDOW_DESCRIPTION);

	return (description);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void MainWindowSystem::EndAgent(int32 index)
{
	// Stop any playing module
	StopAndFreeModule();
	FreeAllModules();
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
/******************************************************************************/
ap_result MainWindowSystem::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Initialize the mixer
		case APMA_INIT_MIXER:
		{
			InitMixer((APAgent_InitMixer *)args);
			return (AP_OK);
		}

		// Cleanup the mixer
		case APMA_END_MIXER:
		{
			EndMixer();
			return (AP_OK);
		}

		// Do the mixing
		case APMA_MIXING:
		{
			return (Mixing((APAgent_Mixing *)args));
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* InitClient() initialize the client add-on.                                 */
/*                                                                            */
/* Input:  "index" is the client index number.                                */
/*                                                                            */
/* Output: True for success, false for failure.                               */
/******************************************************************************/
bool MainWindowSystem::InitClient(int32 index)
{
	try
	{
		uint16 i;

		// Initialize the channels enabled array
		for (i = 0; i < MAX_NUM_CHANNELS; i++)
			channelsEnabled[i] = true;

		// Get the APlayer version
		GetAppVersion();

		// Initialize and load settings
		InitSettings();

		// Initialize module loader
		InitLoader();

		// Get all the add-on lists
		CreateAddOnLists();

		// Create windows
		CreateWindows();

		// Create connection to the APlayer server
		serverHandle = globalData->communication->ConnectToServer("MainWindowSystem", ServerMessageReceived, this);
		if (serverHandle == NULL)
		{
			PString title, msg;

			// Show error
			title.LoadString(res, IDS_ERR_TITLE);
			msg.LoadString(res, IDS_ERR_CONNECTION);

			PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
			alert.Show();

			return (false);
		}
	}
	catch(...)
	{
		return (false);
	}

	return (true);
}



/******************************************************************************/
/* EndClient() cleanup the client add-on.                                     */
/*                                                                            */
/* Input:  "index" is the client index number.                                */
/******************************************************************************/
void MainWindowSystem::EndClient(int32 index)
{
	// Close all windows
	CloseWindows();

	// Stop the update thread
	StopUpdateCheck();

	// Stop the module loader
	CleanupLoader();

	// Free the add-on lists
	FreeAddOnLists();

	// Save and cleanup the settings
	CleanupSettings();
}



/******************************************************************************/
/* Start() is called when the client should start running.                    */
/*                                                                            */
/* Input:  "index" is the index to the add-on to run.                         */
/******************************************************************************/
void MainWindowSystem::Start(int32 index)
{
	bool openWindow;

	// Should we register all the file types?
	if (useSettings->GetStringEntryValue("FileTypes", "RegisterFileTypes").CompareNoCase("Yes") == 0)
	{
		// Yes, do it
		globalData->fileTypes->RegisterAPlayerFileTypesInSystem();
	}

	// Add the add-ons with configuration or display windows in the menu
	mainWin->AddAddOnsToMenu();

	// Show the main window
	mainWin->Show();

	// Open the module information window
	openWindow = useSettings->GetStringEntryValue("Window", "InfoOpenWindow").CompareNoCase("Yes") == 0;
	if (!openWindow)
		infoWin->Hide();

	infoWin->Show();

	// Open the sample information window
	openWindow = useSettings->GetStringEntryValue("Window", "SampleOpenWindow").CompareNoCase("Yes") == 0;
	if (!openWindow)
		sampWin->Hide();

	sampWin->Show();

	// Make sure the main window have the focus
	mainWin->Activate();

	// Get startup files and put them into the module list
	RetrieveStartupFiles();

	// Check for new updates of APlayer
	StartUpdateCheck();
}



/******************************************************************************/
/* LoadAndPlayModule() will load and play the module at the index given.      */
/*                                                                            */
/* Input:  "index" is the index in the module list to load and play.          */
/*         "subSong" is the subsong to play or -1 for the default start song. */
/*         "startPos" is the start position in the module or -1 for start of  */
/*         song.                                                              */
/******************************************************************************/
void MainWindowSystem::LoadAndPlayModule(int32 index, int16 subSong, int16 startPos)
{
	BMessage ctrlMsg(AP_CHANGE_PLAY_ITEM);
	BMessage loadMsg(AP_LOADINIT_MODULE);

	// Start to free all loaded modules
	loader->PostMessage(AP_FREE_ALL);

	// Mark the item in the list
	ctrlMsg.AddInt32("index", index);
	mainWin->PostMessage(&ctrlMsg);

	// Initialize other stuff in the window
	mainWin->InitSubSongs();

	// Now load and play the first module
	loadMsg.AddInt32("load", index);
	loadMsg.AddInt16("play", subSong);
	loadMsg.AddInt16("startPos", startPos);
	loader->PostMessage(&loadMsg);
}



/******************************************************************************/
/* PlayModule() will play the module already loaded.                          */
/*                                                                            */
/* Input:  "index" is the index in the module list to select.                 */
/******************************************************************************/
void MainWindowSystem::PlayModule(int32 index)
{
	BMessage ctrlMsg(AP_CHANGE_PLAY_ITEM);
	BMessage initMsg(AP_LOADINIT_MODULE);

	// Mark the item in the list
	ctrlMsg.AddInt32("index", index);
	mainWin->PostMessage(&ctrlMsg);

	// Initialize other stuff in the window
	mainWin->InitSubSongs();

	// Play the first module
	initMsg.AddInt16("play", -1);
	initMsg.AddInt16("startPos", -1);
	loader->PostMessage(&initMsg);
}



/******************************************************************************/
/* StopAndFreeModule() will stop and free the playing module if any.          */
/******************************************************************************/
void MainWindowSystem::StopAndFreeModule(void)
{
	BMessenger messenger(NULL, loader);
	BMessage message(AP_FREE_MODULE);
	BMessage reply;

	// Tell the loader to free the module
	messenger.SendMessage(&message, &reply);

	// Deselect the playing item
	message.MakeEmpty();
	message.what = AP_CHANGE_PLAY_ITEM;
	message.AddInt32("index", -1);
	mainWin->PostMessage(&message);

	// Update the window controls
	mainWin->PostMessage(AP_INIT_CONTROLS);
}



/******************************************************************************/
/* FreeExtraModules() will free all loaded modules, except for the first one. */
/******************************************************************************/
void MainWindowSystem::FreeExtraModules(void)
{
	// Tell the loader to free the modules
	loader->PostMessage(AP_FREE_EXTRA);
}



/******************************************************************************/
/* FreeAllModules() will free all loaded modules.                             */
/******************************************************************************/
void MainWindowSystem::FreeAllModules(void)
{
	BMessenger messenger(NULL, loader);
	BMessage message(AP_FREE_ALL);
	BMessage reply;

	// Tell the loader to free the module
	messenger.SendMessage(&message, &reply);
}



/******************************************************************************/
/* StartSong() will start to play the song given.                             */
/*                                                                            */
/* Input:  "newSong" is the song number to play.                              */
/******************************************************************************/
void MainWindowSystem::StartSong(uint16 newSong)
{
	BMessage msg(AP_START_SONG);

	// Add the song number
	msg.AddInt16("song", newSong);

	// Tell the loader to play the song
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* PausePlaying() will pause the player.                                      */
/******************************************************************************/
void MainWindowSystem::PausePlaying(void)
{
	// Tell the loader to pause the module
	loader->PostMessage(AP_PAUSE_PLAYING);
}



/******************************************************************************/
/* ResumePlaying() will continue to play the module.                          */
/******************************************************************************/
void MainWindowSystem::ResumePlaying(void)
{
	// Tell the loader to resume the module
	loader->PostMessage(AP_RESUME_PLAYING);
}



/******************************************************************************/
/* HoldPlaying() will tell the mixer to hold or continue playing. Note that   */
/*      this is NOT the same as pause and resume playing!                     */
/*                                                                            */
/* Input:  "hold" is the hold value. True for hold, false for continue.       */
/******************************************************************************/
void MainWindowSystem::HoldPlaying(bool hold)
{
	BMessage msg(AP_HOLD_PLAYING);

	// Add the hold value
	msg.AddBool("hold", hold);

	// Tell the loader to resume the module
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* EndModule() will emulate a module end.                                     */
/******************************************************************************/
void MainWindowSystem::EndModule(void)
{
	DoModuleEnded();
}



/******************************************************************************/
/* SetSongPosition() will tell the player to change to the position given.    */
/*                                                                            */
/* Input:  "newPos" is the new song position.                                 */
/******************************************************************************/
void MainWindowSystem::SetSongPosition(int16 newPos)
{
	BMessage msg(AP_SET_POSITION);

	// Set the song position in the information object
	playerInfo->SetSongPosition(newPos);

	// Add the hold value
	msg.AddInt16("newPos", newPos);

	// Tell the loader to change the position
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* SetVolume() will tell the mixer to change to the master volume.            */
/*                                                                            */
/* Input:  "newVol" is the new volume.                                        */
/******************************************************************************/
void MainWindowSystem::SetVolume(uint16 newVol)
{
	BMessage msg(AP_SET_VOLUME);

	// Add the volume
	msg.AddInt16("newVol", newVol);

	// Tell the loader to change the position
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* SetMixerSettings() will tell the mixer to change some of the mixer         */
/*      settings.                                                             */
/*                                                                            */
/* Input:  "interpolation" is the interpolation flag. -1 for keeping the      */
/*         current setting.                                                   */
/*         "dolby" is the Dolby Surround flag. -1 for keeping the current     */
/*         setting.                                                           */
/*         "stereoSep" is the stereo separator. -1 for keeping the current    */
/*         setting.                                                           */
/*         "filter" is the Amiga filter flag. -1 for keeping the current      */
/*         setting.                                                           */
/******************************************************************************/
void MainWindowSystem::SetMixerSettings(int32 interpolation, int32 dolby, int32 stereoSep, int32 filter)
{
	BMessage msg(AP_SET_MIXER);

	// Add the flags
	msg.AddInt32("interpolation", interpolation);
	msg.AddInt32("dolby", dolby);
	msg.AddInt32("stereoSep", stereoSep);
	msg.AddInt32("filter", filter);

	// Tell the loader to change the mixer settings
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* EnableChannels() will tell the mixer to enable/disable one or more         */
/*      channels.                                                             */
/*                                                                            */
/* Input:  "enable" indicates if you want to enable or disable the channels.  */
/*         "startChan" is the start channel to change.                        */
/*         "stopChan" is the stop channel to change or -1 if you only want to */
/*         change one channel.                                                */
/******************************************************************************/
void MainWindowSystem::EnableChannels(bool enable, int16 startChan, int16 stopChan)
{
	BMessage msg(AP_SET_CHANNELS);
	int16 i;

	// Build the message
	msg.AddBool("enabled", enable);
	msg.AddInt16("startChan", startChan);

	// Change the channel array
	if (stopChan == -1)
		channelsEnabled[startChan] = enable;
	else
	{
		for (i = startChan; i <= stopChan; i++)
			channelsEnabled[i] = enable;

		msg.AddInt16("stopChan", stopChan);
	}

	// Tell the loader to change the channels
	loader->PostMessage(&msg);
}



/******************************************************************************/
/* OpenConfigWindow() will ask the server to open the configuration window    */
/*      for the add-on given.                                                 */
/*                                                                            */
/* Input:  "addOnName" is the name of the add-on to open the configuration    */
/*         window for.                                                        */
/******************************************************************************/
void MainWindowSystem::OpenConfigWindow(PString addOnName)
{
	PString cmd, result;

	// Build the command and send it
	cmd    = globalData->communication->AddArgument("OpenConfigWindow=", addOnName);
	result = globalData->communication->SendCommand(serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		ShowError(IDS_ERR_OPEN_CONFIG_WINDOW, result);
}



/******************************************************************************/
/* OpenDisplayWindow() will ask the server to open the display window for the */
/*      add-on given.                                                         */
/*                                                                            */
/* Input:  "addOnName" is the name of the add-on to open the display window   */
/*         for.                                                               */
/******************************************************************************/
void MainWindowSystem::OpenDisplayWindow(PString addOnName)
{
	PString cmd, result;

	// Build the command and send it
	cmd    = globalData->communication->AddArgument("OpenDisplayWindow=", addOnName);
	result = globalData->communication->SendCommand(serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		ShowError(IDS_ERR_OPEN_DISPLAY_WINDOW, result);
}



/******************************************************************************/
/* EnableAddOn() will tell the server to enable an add-on.                    */
/*                                                                            */
/* Input:  "addOnName" is the name of the add-on to enable.                   */
/*                                                                            */
/* Output: A string that indicates which windows the add-on have.             */
/******************************************************************************/
PString MainWindowSystem::EnableAddOn(PString addOnName)
{
	PString cmd, result;

	// Build the command and send it
	cmd    = globalData->communication->AddArgument("EnableAddOn=", addOnName);
	result = globalData->communication->SendCommand(serverHandle, cmd);

	if (result.Left(4) == "ERR=")
	{
		ShowError(IDS_ERR_ENABLE_ADDON, result);
		return ("");
	}

	return (result);
}



/******************************************************************************/
/* DisableAddOn() will tell the server to disable an add-on.                  */
/*                                                                            */
/* Input:  "addOnName" is the name of the add-on to disable.                  */
/******************************************************************************/
void MainWindowSystem::DisableAddOn(PString addOnName)
{
	PString cmd, result;

	// Build the command and send it
	cmd    = globalData->communication->AddArgument("DisableAddOn=", addOnName);
	result = globalData->communication->SendCommand(serverHandle, cmd);

	if (result.Left(4) == "ERR=")
		ShowError(IDS_ERR_DISABLE_ADDON, result);
}



/******************************************************************************/
/* SaveServerSettings() will tell the server to save its settings.            */
/******************************************************************************/
void MainWindowSystem::SaveServerSettings(void)
{
	PString result;

	// Build the command and send it
	result = globalData->communication->SendCommand(serverHandle, "SaveSettings=");

	if (result.Left(4) == "ERR=")
		ShowError(IDS_ERR_SAVE_SERVER_SETTINGS, result);
}



/******************************************************************************/
/* ShowAboutWindow() will open the about window.                              */
/******************************************************************************/
void MainWindowSystem::ShowAboutWindow(void)
{
	PString title;
	int32 x, y, w, h;

	if (aboutWin == NULL)
	{
		// Get the position and size of the window
		x = useSettings->GetIntEntryValue("Window", "AboutX");
		y = useSettings->GetIntEntryValue("Window", "AboutY");
		w = useSettings->GetIntEntryValue("Window", "AboutWidth");
		h = useSettings->GetIntEntryValue("Window", "AboutHeight");

		// Allocate the window object and open it
		title.LoadString(res, IDS_ABOUT_TITLE);

		aboutWin = new APWindowAbout(this, BRect(x, y, x + w, y + h), title);
		aboutWin->Show();
	}
	else
		aboutWin->Activate();
}



/******************************************************************************/
/* ShowSettingsWindow() will open the settings window.                        */
/******************************************************************************/
void MainWindowSystem::ShowSettingsWindow(void)
{
	PString title;
	int32 x, y, w, h;

	if (settingsWin == NULL)
	{
		// Get the position and size of the window
		x = useSettings->GetIntEntryValue("Window", "SettingsX");
		y = useSettings->GetIntEntryValue("Window", "SettingsY");
		w = useSettings->GetIntEntryValue("Window", "SettingsWidth");
		h = useSettings->GetIntEntryValue("Window", "SettingsHeight");

		// Allocate the window object and open it
		title.LoadString(res, IDS_SETTINGS_TITLE);

		settingsWin = new APWindowSettings(this, globalData, BRect(x, y, x + w, y + h), title);
		settingsWin->Show();
	}
	else
		settingsWin->Activate();
}



/******************************************************************************/
/* ShowError() will show the server error.                                    */
/*                                                                            */
/* Input:  "resourceError" is a resource string number to show.               */
/*         "error" is the result got from the server.                         */
/******************************************************************************/
void MainWindowSystem::ShowError(int32 resourceError, PString error)
{
	int32 index, number;
	PString title, errStr1, errStr2;
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

	errStr1.LoadString(res, resourceError);
	errStr2.Format(res, IDS_ERR_SERVER, number, (errStr = error.GetString()));
	error.FreeBuffer(errStr);

	// Load the title used in errors
	title.LoadString(res, IDS_ERR_TITLE);

	PAlert alert(title, errStr1 + errStr2, PAlert::pStop, PAlert::pOk);
	alert.Show();
}



/******************************************************************************/
/* CheckForUpdates() will connect to our server and check for updates.        */
/*      If updates are available on the server, the user will have the choice */
/*      to download the update.                                               */
/*                                                                            */
/* Output: True if there is any updates on the server, false if there isn't.  */
/******************************************************************************/
bool MainWindowSystem::CheckForUpdates(void)
{
	uint32 major = 0, minor = 0, revision = 0, beta = 0;
	PTime date;
	uint32 fileSize = 0;
	PList<PString> ftpUrls, wwwUrls;

	// Update the settings to the current date
	date.SetToNow();
	saveSettings->WriteStringEntryValue("Network", "LastChecked", date.Format("%yyyy-%MM-%dd"));

	try
	{
		PProxySocket socket;
		PHTTPFile file(&socket, 2);
		PString line;
		PCharSet_UTF8 charSet;

		// Should we use proxy server?
		if (useSettings->GetStringEntryValue("Network", "UseProxy").CompareNoCase("Yes") != 0)
			socket.EnableProxy(false);
		else
			socket.SetProxyServer(PProxySocket::pHTTP, useSettings->GetStringEntryValue("Network", "ProxyAddress"));

		// Set the agent name
		NETWORK_AGENT_NAME(line);
		file.SetAgentName(line);

		// Open the script
		file.Open(NETWORK_UPDATE_FILE, PFile::pModeRead);

		// Read the result are parse it
		for (;;)
		{
			line = file.ReadLine(&charSet);
			if (line.IsEmpty())
				break;

			// Did we get an error?
			if (line.Left(7).CompareNoCase("Error: ") == 0)
			{
				// Yap, return in silence
				file.Close();
				return (false);
			}

			if (line.Left(7).CompareNoCase("Major: ") == 0)
			{
				major = line.GetUNumber(7);
				continue;
			}

			if (line.Left(7).CompareNoCase("Minor: ") == 0)
			{
				minor = line.GetUNumber(7);
				continue;
			}

			if (line.Left(10).CompareNoCase("Revision: ") == 0)
			{
				revision = line.GetUNumber(10);
				continue;
			}

			if (line.Left(6).CompareNoCase("Beta: ") == 0)
			{
				beta = line.GetUNumber(6);
				continue;
			}

			if (line.Left(10).CompareNoCase("Filesize: ") == 0)
			{
				fileSize = line.GetUNumber(10);
				continue;
			}

			if (line.Left(14).CompareNoCase("Release date: ") == 0)
			{
				date.SetTime(line.GetUNumber(22), line.GetUNumber(19), line.GetUNumber(14), 0, 0, 0);
				continue;
			}

			if (line.Left(9).CompareNoCase("FTP URL: ") == 0)
			{
				ftpUrls.AddTail(line.Mid(9));
				continue;
			}

			if (line.Left(9).CompareNoCase("WWW URL: ") == 0)
			{
				wwwUrls.AddTail(line.Mid(9));
				continue;
			}
		}

		// Done with the script
		file.Close();

		// Did we get any download links?
		if ((ftpUrls.CountItems() == 0) && (wwwUrls.CountItems() == 0))
			return (false);
	}
	catch(PException e)
	{
		// Ignore any errors
		return (false);
	}

	// At this point, we are sure we got a version returned.
	// Now check it to see if it's newer than the current
	if (major < versionMajor)
		return (false);

	if (major == versionMajor)
	{
		if (minor < versionMiddle)
			return (false);

		if (minor == versionMiddle)
		{
			if (revision < versionMinor)
				return (false);

			if (revision == versionMinor)
			{
				if ((versionBeta == 0) && (beta == 65535))
					return (false);

				if (beta <= versionBeta)
					return (false);
			}
		}
	}

	// Remove the sleep cursor if any
	mainWin->SetNormalCursor();

	{
		PString title;

		// Allocate the window object and open it
		title.LoadString(res, IDS_UPDATE_TITLE);

		updateWin = new APWindowUpdate(this, title, major, minor, revision, beta, fileSize, date, ftpUrls, wwwUrls);
		updateWin->Show();
	}

	return (true);
}



/******************************************************************************/
/* GetAppVersion() will retrieve the application version number.              */
/******************************************************************************/
void MainWindowSystem::GetAppVersion(void)
{
	app_info ai;
	BFile file;
	BAppFileInfo afi;
	version_info vi;

	// Initialize the version variables
	versionMajor  = 0;
	versionMiddle = 0;
	versionMinor  = 0;
	versionBeta   = 0;

	// The the application information
	if (be_app->GetAppInfo(&ai) == B_OK)
	{
		// Open the file
		if (file.SetTo(&ai.ref, B_READ_ONLY) == B_OK)
		{
			// The the file in the BAppFileInfo class
			if (afi.SetTo(&file) == B_OK)
			{
				// Get the version information
				if (afi.GetVersionInfo(&vi, B_APP_VERSION_KIND) == B_OK)
				{
					// Copy the information needed
					versionMajor  = vi.major;
					versionMiddle = vi.middle;
					versionMinor  = vi.minor;
					versionBeta   = vi.internal;
				}
			}
		}
	}
}



/******************************************************************************/
/* InitSettings() will read the add-on settings.                              */
/******************************************************************************/
void MainWindowSystem::InitSettings(void)
{
	PString tempStr;

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
		saveSettings->LoadFile("MainWindowSystem.ini", "Polycode", "APlayer");
	}
	catch(PFileException e)
	{
		if (e.errorNum != P_FILE_ERR_ENTRY_NOT_FOUND)
			throw;
	}

	// Set all the default values
	//
	// Main window
	//
	if (!saveSettings->EntryExist("Window", "MainX"))
		saveSettings->WriteIntEntryValue("Window", "MainX", 100);

	if (!saveSettings->EntryExist("Window", "MainY"))
		saveSettings->WriteIntEntryValue("Window", "MainY", 50);

	if (!saveSettings->EntryExist("Window", "MainWidth"))
		saveSettings->WriteIntEntryValue("Window", "MainWidth", 1);

	if (!saveSettings->EntryExist("Window", "MainHeight"))
		saveSettings->WriteIntEntryValue("Window", "MainHeight", 1);

	if (!saveSettings->EntryExist("General", "MasterVolume"))
		saveSettings->WriteIntEntryValue("General", "MasterVolume", 256);

	if (!saveSettings->EntryExist("General", "TimeFormat"))
		saveSettings->WriteStringEntryValue("General", "TimeFormat", "Elapsed");

	// About window
	if (!saveSettings->EntryExist("Window", "AboutX"))
		saveSettings->WriteIntEntryValue("Window", "AboutX", 250);

	if (!saveSettings->EntryExist("Window", "AboutY"))
		saveSettings->WriteIntEntryValue("Window", "AboutY", 50);

	if (!saveSettings->EntryExist("Window", "AboutWidth"))
		saveSettings->WriteIntEntryValue("Window", "AboutWidth", 300);

	if (!saveSettings->EntryExist("Window", "AboutHeight"))
		saveSettings->WriteIntEntryValue("Window", "AboutHeight", 200);

	// Module information window
	if (!saveSettings->EntryExist("Window", "InfoX"))
		saveSettings->WriteIntEntryValue("Window", "InfoX", 460);

	if (!saveSettings->EntryExist("Window", "InfoY"))
		saveSettings->WriteIntEntryValue("Window", "InfoY", 25);

	if (!saveSettings->EntryExist("Window", "InfoWidth"))
		saveSettings->WriteIntEntryValue("Window", "InfoWidth", 290);

	if (!saveSettings->EntryExist("Window", "InfoHeight"))
		saveSettings->WriteIntEntryValue("Window", "InfoHeight", 190);

	if (!saveSettings->EntryExist("Window", "InfoOpenWindow"))
		saveSettings->WriteStringEntryValue("Window", "InfoOpenWindow", "No");

	if (!saveSettings->EntryExist("Window", "InfoCol1W"))
		saveSettings->WriteIntEntryValue("Window", "InfoCol1W", 137);

	if (!saveSettings->EntryExist("Window", "InfoCol2W"))
		saveSettings->WriteIntEntryValue("Window", "InfoCol2W", 130);

	// Sample information window
	if (!saveSettings->EntryExist("Window", "SampleX"))
		saveSettings->WriteIntEntryValue("Window", "SampleX", 460);

	if (!saveSettings->EntryExist("Window", "SampleY"))
		saveSettings->WriteIntEntryValue("Window", "SampleY", 260);

	if (!saveSettings->EntryExist("Window", "SampleWidth"))
		saveSettings->WriteIntEntryValue("Window", "SampleWidth", 680);

	if (!saveSettings->EntryExist("Window", "SampleHeight"))
		saveSettings->WriteIntEntryValue("Window", "SampleHeight", 190);

	if (!saveSettings->EntryExist("Window", "SampleOpenWindow"))
		saveSettings->WriteStringEntryValue("Window", "SampleOpenWindow", "No");

	if (!saveSettings->EntryExist("Window", "SampleActiveTab"))
		saveSettings->WriteIntEntryValue("Window", "SampleActiveTab", 0);

	// Sample information window: Instruments tab
	if (!saveSettings->EntryExist("Window", "SampleInstCol1W"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol1W", 28);

	if (!saveSettings->EntryExist("Window", "SampleInstCol2W"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol2W", 244);

	if (!saveSettings->EntryExist("Window", "SampleInstCol3W"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol3W", 144);

	if (!saveSettings->EntryExist("Window", "SampleInstCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "SampleInstCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "SampleInstCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "SampleInstSortKey"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstSortKey", 0);

	if (!saveSettings->EntryExist("Window", "SampleInstSortMode"))
		saveSettings->WriteIntEntryValue("Window", "SampleInstSortMode", Ascending);

	// Sample information window: Samples tab
	if (!saveSettings->EntryExist("Window", "SampleSampCol1W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol1W", 28);

	if (!saveSettings->EntryExist("Window", "SampleSampCol2W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol2W", 244);

	if (!saveSettings->EntryExist("Window", "SampleSampCol3W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol3W", 44);

	if (!saveSettings->EntryExist("Window", "SampleSampCol4W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol4W", 44);

	if (!saveSettings->EntryExist("Window", "SampleSampCol5W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol5W", 52);

	if (!saveSettings->EntryExist("Window", "SampleSampCol6W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol6W", 36);

	if (!saveSettings->EntryExist("Window", "SampleSampCol7W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol7W", 36);

	if (!saveSettings->EntryExist("Window", "SampleSampCol8W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol8W", 36);

	if (!saveSettings->EntryExist("Window", "SampleSampCol9W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol9W", 44);

	if (!saveSettings->EntryExist("Window", "SampleSampCol10W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol10W", 44);

	if (!saveSettings->EntryExist("Window", "SampleSampCol11W"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol11W", 56);

	if (!saveSettings->EntryExist("Window", "SampleSampCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "SampleSampCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "SampleSampCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "SampleSampCol4Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol4Pos", 3);

	if (!saveSettings->EntryExist("Window", "SampleSampCol5Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol5Pos", 4);

	if (!saveSettings->EntryExist("Window", "SampleSampCol6Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol6Pos", 5);

	if (!saveSettings->EntryExist("Window", "SampleSampCol7Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol7Pos", 6);

	if (!saveSettings->EntryExist("Window", "SampleSampCol8Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol8Pos", 7);

	if (!saveSettings->EntryExist("Window", "SampleSampCol9Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol9Pos", 8);

	if (!saveSettings->EntryExist("Window", "SampleSampCol10Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol10Pos", 9);

	if (!saveSettings->EntryExist("Window", "SampleSampCol11Pos"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampCol11Pos", 10);

	if (!saveSettings->EntryExist("Window", "SampleSampSortKey"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampSortKey", 0);

	if (!saveSettings->EntryExist("Window", "SampleSampSortMode"))
		saveSettings->WriteIntEntryValue("Window", "SampleSampSortMode", Ascending);

	if (!saveSettings->EntryExist("Window", "SampleSampSaveFormat"))
		saveSettings->WriteStringEntryValue("Window", "SampleSampSaveFormat", "");

	// Settings window
	if (!saveSettings->EntryExist("Window", "SettingsX"))
		saveSettings->WriteIntEntryValue("Window", "SettingsX", 100);

	if (!saveSettings->EntryExist("Window", "SettingsY"))
		saveSettings->WriteIntEntryValue("Window", "SettingsY", 200);

	if (!saveSettings->EntryExist("Window", "SettingsWidth"))
		saveSettings->WriteIntEntryValue("Window", "SettingsWidth", 600);

	if (!saveSettings->EntryExist("Window", "SettingsHeight"))
		saveSettings->WriteIntEntryValue("Window", "SettingsHeight", 400);

	if (!saveSettings->EntryExist("Window", "SettingsTab"))
		saveSettings->WriteIntEntryValue("Window", "SettingsTab", 0);

	if (!saveSettings->EntryExist("Window", "FileTypesCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "FileTypesCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "FileTypesCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "FileTypesCol4Pos"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol4Pos", 3);

	if (!saveSettings->EntryExist("Window", "FileTypesCol1Width"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol1Width", 20);

	if (!saveSettings->EntryExist("Window", "FileTypesCol2Width"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol2Width", 120);

	if (!saveSettings->EntryExist("Window", "FileTypesCol3Width"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol3Width", 120);

	if (!saveSettings->EntryExist("Window", "FileTypesCol4Width"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesCol4Width", 70);

	if (!saveSettings->EntryExist("Window", "FileTypesSortKey"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesSortKey", 1);

	if (!saveSettings->EntryExist("Window", "FileTypesSortMode"))
		saveSettings->WriteIntEntryValue("Window", "FileTypesSortMode", Ascending);

	if (!saveSettings->EntryExist("Window", "PlayerCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "PlayerCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "PlayerCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "PlayerCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "PlayerCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "PlayerCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "PlayerSortKey"))
		saveSettings->WriteIntEntryValue("Window", "PlayerSortKey", 1);

	if (!saveSettings->EntryExist("Window", "PlayerSortMode"))
		saveSettings->WriteIntEntryValue("Window", "PlayerSortMode", Ascending);

	if (!saveSettings->EntryExist("Window", "AgentCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "AgentCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "AgentCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "AgentCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "AgentCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "AgentCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "AgentSortKey"))
		saveSettings->WriteIntEntryValue("Window", "AgentSortKey", 1);

	if (!saveSettings->EntryExist("Window", "AgentSortMode"))
		saveSettings->WriteIntEntryValue("Window", "AgentSortMode", Ascending);

	if (!saveSettings->EntryExist("Window", "ClientCol1Pos"))
		saveSettings->WriteIntEntryValue("Window", "ClientCol1Pos", 0);

	if (!saveSettings->EntryExist("Window", "ClientCol2Pos"))
		saveSettings->WriteIntEntryValue("Window", "ClientCol2Pos", 1);

	if (!saveSettings->EntryExist("Window", "ClientCol3Pos"))
		saveSettings->WriteIntEntryValue("Window", "ClientCol3Pos", 2);

	if (!saveSettings->EntryExist("Window", "ClientSortKey"))
		saveSettings->WriteIntEntryValue("Window", "ClientSortKey", 1);

	if (!saveSettings->EntryExist("Window", "ClientSortMode"))
		saveSettings->WriteIntEntryValue("Window", "ClientSortMode", Ascending);

	// Setting window: Options tab
	if (!saveSettings->EntryExist("Options", "AddJump"))
		saveSettings->WriteStringEntryValue("Options", "AddJump", "No");

	if (!saveSettings->EntryExist("Options", "AddToList"))
		saveSettings->WriteStringEntryValue("Options", "AddToList", "Yes");

	if (!saveSettings->EntryExist("Options", "RememberList"))
		saveSettings->WriteStringEntryValue("Options", "RememberList", "No");

	if (!saveSettings->EntryExist("Options", "RememberListPosition"))
		saveSettings->WriteStringEntryValue("Options", "RememberListPosition", "Yes");

	if (!saveSettings->EntryExist("Options", "RememberModulePosition"))
		saveSettings->WriteStringEntryValue("Options", "RememberModulePosition", "No");

	if (!saveSettings->EntryExist("Options", "ShowListNumber"))
		saveSettings->WriteStringEntryValue("Options", "ShowListNumber", "No");

	if (!saveSettings->EntryExist("Options", "ToolTips"))
		saveSettings->WriteStringEntryValue("Options", "ToolTips", "Yes");

	if (!saveSettings->EntryExist("Options", "ShowNameInTitle"))
		saveSettings->WriteStringEntryValue("Options", "ShowNameInTitle", "No");

	if (!saveSettings->EntryExist("Options", "SetLength"))
		saveSettings->WriteStringEntryValue("Options", "SetLength", "No");

	if (!saveSettings->EntryExist("Options", "ScanFiles"))
		saveSettings->WriteStringEntryValue("Options", "ScanFiles", "No");

	if (!saveSettings->EntryExist("Options", "DoubleBuffering"))
		saveSettings->WriteStringEntryValue("Options", "DoubleBuffering", "No");

	if (!saveSettings->EntryExist("Options", "EarlyLoad"))
		saveSettings->WriteIntEntryValue("Options", "EarlyLoad", 2);

	if (!saveSettings->EntryExist("Options", "ModuleError"))
		saveSettings->WriteIntEntryValue("Options", "ModuleError", CV_ERROR_SHOWERROR);

	if (!saveSettings->EntryExist("Options", "NeverEnding"))
		saveSettings->WriteStringEntryValue("Options", "NeverEnding", "No");

	if (!saveSettings->EntryExist("Options", "NeverEndingTimeout"))
		saveSettings->WriteIntEntryValue("Options", "NeverEndingTimeout", 180);

	if (!saveSettings->EntryExist("Options", "ModuleListEnd"))
		saveSettings->WriteIntEntryValue("Options", "ModuleListEnd", CV_LISTEND_JUMPTOSTART);

	// Setting window: Paths tab
	if (!saveSettings->EntryExist("Paths", "StartScan"))
		saveSettings->WriteStringEntryValue("Paths", "StartScan", "");

	if (!saveSettings->EntryExist("Paths", "Modules"))
		saveSettings->WriteStringEntryValue("Paths", "Modules", "");

	if (!saveSettings->EntryExist("Paths", "APML"))
		saveSettings->WriteStringEntryValue("Paths", "APML", "");

	// Setting window: Mixer tab
	if (!saveSettings->EntryExist("Mixer", "Frequency"))
		saveSettings->WriteIntEntryValue("Mixer", "Frequency", 44100);

	if (!saveSettings->EntryExist("Mixer", "StereoSep"))
		saveSettings->WriteIntEntryValue("Mixer", "StereoSep", 100);

	if (!saveSettings->EntryExist("Mixer", "Interpolation"))
		saveSettings->WriteStringEntryValue("Mixer", "Interpolation", "No");

	if (!saveSettings->EntryExist("Mixer", "DolbyPrologic"))
		saveSettings->WriteStringEntryValue("Mixer", "DolbyPrologic", "No");

	if (!saveSettings->EntryExist("Mixer", "AmigaFilter"))
		saveSettings->WriteStringEntryValue("Mixer", "AmigaFilter", "Yes");

	if (!saveSettings->EntryExist("Mixer", "OutputAgent"))
		saveSettings->WriteStringEntryValue("Mixer", "OutputAgent", "BeOS MediaKit");

	// Setting window: Network tab
	if (!saveSettings->EntryExist("Network", "UseProxy"))
		saveSettings->WriteStringEntryValue("Network", "UseProxy", "No");

	if (!saveSettings->EntryExist("Network", "ProxyAddress"))
	{
		tempStr.LoadString(res, IDS_DEFAULT_ADDRESS);
		saveSettings->WriteStringEntryValue("Network", "ProxyAddress", tempStr);
	}

	if (!saveSettings->EntryExist("Network", "UpdateCheck"))
		saveSettings->WriteIntEntryValue("Network", "UpdateCheck", CV_UPDATE_NEVER);

	if (!saveSettings->EntryExist("Network", "DownloadPath"))
	{
		PDirectory dir;

		dir.FindDirectory(PDirectory::pUser);
		saveSettings->WriteStringEntryValue("Network", "DownloadPath", dir.GetDirectory());
	}

	// Setting window: Filetypes tab
	if (!saveSettings->EntryExist("FileTypes", "ChangeModuleType"))
		saveSettings->WriteStringEntryValue("FileTypes", "ChangeModuleType", "Yes");

	if (!saveSettings->EntryExist("FileTypes", "RegisterFileTypes"))
		saveSettings->WriteStringEntryValue("FileTypes", "RegisterFileTypes", "Yes");

	// Copy the settings to the "use" settings
	useSettings->CloneSettings(saveSettings);
}



/******************************************************************************/
/* CleanupSettings() will save and destroy the settings.                      */
/******************************************************************************/
void MainWindowSystem::CleanupSettings(void)
{
	if (saveSettings != NULL)
	{
		try
		{
			// Save the settings
			saveSettings->SaveFile("MainWindowSystem.ini", "Polycode", "APlayer");
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
/* InitLoader() will initialize the module loader and start the loader thread.*/
/******************************************************************************/
void MainWindowSystem::InitLoader(void)
{
	// Create player information object
	playerInfo = new APPlayerInfo();
	if (playerInfo == NULL)
		throw PMemoryException();

	// Create loader object
	loader = new APLoader(globalData, this);
	if (loader == NULL)
		throw PMemoryException();

	// Start the thread
	loader->Run();
}



/******************************************************************************/
/* CleanupLoader() will stop the loader thread and destroy the loader object. */
/******************************************************************************/
void MainWindowSystem::CleanupLoader(void)
{
	if (loader != NULL)
	{
		BMessenger messenger(NULL, loader);
		BMessage message(B_QUIT_REQUESTED);

		// Tell the BLooper to quit and delete itself
		// This call is synchronous, so we are sure the
		// object has been deleted when the function
		// call returns
		messenger.SendMessage(&message, &message);
		loader = NULL;
	}

	// Delete the player information object
	delete playerInfo;
	playerInfo = NULL;
}



/******************************************************************************/
/* CreateAddOnLists() will create all the lists containing the add-ons        */
/*      information.                                                          */
/******************************************************************************/
void MainWindowSystem::CreateAddOnLists(void)
{
	globalData->GetAddOnList(apAgent, agentAddOns);
	globalData->GetAddOnList(apPlayer, playerAddOns);
	globalData->GetAddOnList(apClient, clientAddOns);
	globalData->GetAddOnList(apConverter, converterAddOns);
}



/******************************************************************************/
/* FreeAddOnLists() will free all the lists containing the add-ons            */
/*      information.                                                          */
/******************************************************************************/
void MainWindowSystem::FreeAddOnLists(void)
{
	globalData->FreeAddOnList(converterAddOns);
	globalData->FreeAddOnList(clientAddOns);
	globalData->FreeAddOnList(playerAddOns);
	globalData->FreeAddOnList(agentAddOns);
}



/******************************************************************************/
/* CreateWindows() will create all the windows used in the add-on.            */
/******************************************************************************/
void MainWindowSystem::CreateWindows(void)
{
	int32 x, y, w, h;
	int32 active;
	PString title;

	//
	// Main window
	//
	// Get the position and size of the window
	x = useSettings->GetIntEntryValue("Window", "MainX");
	y = useSettings->GetIntEntryValue("Window", "MainY");
	w = useSettings->GetIntEntryValue("Window", "MainWidth");
	h = useSettings->GetIntEntryValue("Window", "MainHeight");

	// Allocate the window object
	mainWin = new APWindowMain(this, globalData, BRect(x, y, x + w, y + h));
	if (mainWin == NULL)
		throw PMemoryException();

	//
	// Module information window
	//
	// Get the position and size of the window
	x = useSettings->GetIntEntryValue("Window", "InfoX");
	y = useSettings->GetIntEntryValue("Window", "InfoY");
	w = useSettings->GetIntEntryValue("Window", "InfoWidth");
	h = useSettings->GetIntEntryValue("Window", "InfoHeight");

	// Get window title
	title.LoadString(res, IDS_MODULE_INFO_TITLE);

	// Allocate the window object
	infoWin = new APWindowModuleInfo(this, BRect(x, y, x + w, y + h), title);
	if (infoWin == NULL)
		throw PMemoryException();

	//
	// Sample information window
	//
	// Get the position and size of the window
	x      = useSettings->GetIntEntryValue("Window", "SampleX");
	y      = useSettings->GetIntEntryValue("Window", "SampleY");
	w      = useSettings->GetIntEntryValue("Window", "SampleWidth");
	h      = useSettings->GetIntEntryValue("Window", "SampleHeight");
	active = useSettings->GetIntEntryValue("Window", "SampleActiveTab");

	// Get window title
	title.LoadString(res, IDS_SAMPLE_INFO_TITLE);

	// Allocate the window object
	sampWin = new APWindowSampleInfo(globalData, this, BRect(x, y, x + w, y + h), title, active);
	if (sampWin == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* CloseWindows() will close and destroy all the windows used in the add-on.  */
/******************************************************************************/
void MainWindowSystem::CloseWindows(void)
{
	// Close the settings window
	if (settingsWin != NULL)
	{
		settingsWin->Lock();
		settingsWin->QuitRequested();
		settingsWin->Quit();
		settingsWin = NULL;
	}

	// Close the about window
	if (aboutWin != NULL)
	{
		aboutWin->Lock();
		aboutWin->QuitRequested();
		aboutWin->Quit();
		aboutWin = NULL;
	}

	// Close the update window
	if (updateWin != NULL)
	{
		updateWin->Lock();
		updateWin->QuitRequested();
		updateWin->Quit();
		updateWin = NULL;
	}

	// Close the main window
	// Note that this window has to be closed before the sample window,
	// so we are sure that no playing module is playing when
	// we close the rest + "remember list" will work in all cases :)
	mainWin->Lock();
	closeWindow = true;
	mainWin->QuitRequested();
	mainWin->Quit();

	// Close the sample information window
	if (sampWin != NULL)
	{
		sampWin->CloseWindow();
		sampWin = NULL;
	}

	// Close the module information window
	if (infoWin != NULL)
	{
		infoWin->CloseWindow();
		infoWin = NULL;
	}
}



/******************************************************************************/
/* RetrieveStartupFiles() will scan the start directory path for files or     */
/*      load the remembered APML file and add the files into the module list. */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void MainWindowSystem::RetrieveStartupFiles(void)
{
	PString path, listFile;
	char *nameStr;
	bool loadList = false;

	// First ask the server if we started normally or not.
	// A non-normal start is when the user clicked on a module
	// in the Tracker when APlayer is not running
	if (StartedNormally())
	{
		// Get the start scan path
		path = useSettings->GetStringEntryValue("Paths", "StartScan");

		// Do we need to load the remembered APML file
		if (useSettings->GetStringEntryValue("Options", "RememberList").CompareNoCase("Yes") == 0)
		{
			PDirectory dir;

			loadList = true;

			dir.FindDirectory(PDirectory::pSettings);
			dir.Append("Polycode");
			dir.Append("APlayer");
			listFile = dir.GetDirectory() + "___apml___RememberList.apml";
		}

		// Do we need to load a remembered module list?
		if (loadList)
		{
			// Load the APML file if it exists
			if (PFile::FileExists(listFile))
			{
				BNode node((nameStr = listFile.GetString()));
				int32 attr;
				int32 selected = -1, position = -1, subSong = -1;

				// Free the string buffer
				listFile.FreeBuffer(nameStr);

				// Load the list
				mainWin->AddFileToList(listFile);

				// Get the attributes
				if (node.ReadAttr("Selected", B_INT32_TYPE, 0, &attr, sizeof(int32)) > 0)
					selected = attr;

				if (node.ReadAttr("Position", B_INT32_TYPE, 0, &attr, sizeof(int32)) > 0)
					position = attr;

				if (node.ReadAttr("SubSong", B_INT32_TYPE, 0, &attr, sizeof(int32)) > 0)
					subSong = attr;

				// Load the module if anyone was selected
				if (selected != -1)
				{
					// Load the module into memory
					LoadAndPlayModule(selected, subSong, position);
				}
			}
			else
				loadList = false;
		}

		// If we haven't loaded any remembered module list, then check
		// to see if the user have a "start path"
		if (!loadList)
		{
			// Add the files in the module list if there is any path
			if (!path.IsEmpty())
			{
				if (PDirectory::DirectoryExists(path))
				{
					// Add all the files in the directory
					AddDirectoryToList(path);
					mainWin->UpdateList();

					// Load the first module
					LoadAndPlayModule(0);
				}
			}
		}
	}
}



/******************************************************************************/
/* AddDirectoryToList() will make a recursive scan of a directory and add all */
/*      the items to the module list.                                         */
/*                                                                            */
/* Input:  "directory" is the directory to scan.                              */
/******************************************************************************/
void MainWindowSystem::AddDirectoryToList(PString directory)
{
	PDirectory dir(directory);
	PDirectory::PEntryType type;
	PString name;

	// Initialize the enumeration
	dir.InitEnum(PDirectory::pAny);

	// Go through all the files
	while (dir.GetNextEntry(name, type))
	{
		// Got an entry, if it's a directory, skip it
		if (type == PDirectory::pDirectory)
			continue;

		// Add the item
		mainWin->AddFileToList(dir.GetDirectory() + name, false);
	}

	dir.EndEnum();

	// Go through all the directories
	dir.InitEnum(PDirectory::pDirectory);

	while (dir.GetNextEntry(name, type))
		AddDirectoryToList(dir.GetDirectory() + name);

	dir.EndEnum();
}



/******************************************************************************/
/* StartedNormally() will ask the server about it started normally or not.    */
/*                                                                            */
/* Output: True if the server started normally, false if not.                 */
/******************************************************************************/
bool MainWindowSystem::StartedNormally(void)
{
	PString result;
	bool normal = true;

	try
	{
		// Send the command to the server
		result = globalData->communication->SendCommand(serverHandle, "StartedNormally=");

		if (result.Left(4) != "ERR=")
		{
			// Parse the result
			normal = (result == "0") ? false : true;
		}
	}
	catch(...)
	{
		;
	}

	return (normal);
}



/******************************************************************************/
/* StartUpdateCheck() will check to see if it's time for an update check.     */
/******************************************************************************/
void MainWindowSystem::StartUpdateCheck(void)
{
	PString lastStr;
	PTime lastTime, curTime;
	PTimeSpan days;
	int32 updateCheck, minDays;

	// Find out when it was the last time an check has been made
	lastStr = useSettings->GetStringEntryValue("Network", "LastChecked");
	if (lastStr.IsEmpty())
		return;

	lastTime.SetTime(lastStr.GetUNumber(8), lastStr.GetUNumber(5), lastStr.GetUNumber(), 0, 0, 0);

	// Get how often to check
	updateCheck = useSettings->GetIntEntryValue("Network", "UpdateCheck");
	if (updateCheck == CV_UPDATE_NEVER)
		return;		// We should never check for updates

	switch (updateCheck)
	{
		case CV_UPDATE_STARTUP:
		{
			minDays = 0;
			break;
		}

		case CV_UPDATE_DAILY:
		{
			minDays = 1;
			break;
		}

		case CV_UPDATE_WEEKLY:
		{
			minDays = 7;
			break;
		}

		case CV_UPDATE_MONTHLY:
		{
			minDays = 30;
			break;
		}

		default:
		{
			// Update type not implemented
			ASSERT(false);
			return;
		}
	}

	// Find current day
	curTime.SetToNow();
	curTime.SetToMidnight();

	// Should we check for updates?
	days.SetTimeSpan(minDays, 0, 0, 0);
	if ((curTime - lastTime) >= days)
	{
		// Start the check thread
		checkThread = new PThread();
		if (checkThread == NULL)
			throw PMemoryException();

		checkThread->SetName("MainWindowSystem: Update Checker");
		checkThread->SetHookFunc(CheckUpdateThread, this);
		checkThread->StartThread();
	}
}



/******************************************************************************/
/* StopUpdateCheck() will stop any update check.                              */
/******************************************************************************/
void MainWindowSystem::StopUpdateCheck(void)
{
	if (checkThread != NULL)
	{
		checkThread->WaitOnThread();
		delete checkThread;
		checkThread = NULL;
	}
}



/******************************************************************************/
/* CheckUpdateThread() will check for new versions.                           */
/*                                                                            */
/* Input:  "userData" is a pointer to the current object.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 MainWindowSystem::CheckUpdateThread(void *userData)
{
	MainWindowSystem *object = (MainWindowSystem *)userData;

	// Just call the check function
	object->CheckForUpdates();

	return (0);
}



/******************************************************************************/
/* ServerMessageReceived() is called everytime the APlayer server sends a     */
/*      command to the client.                                                */
/*                                                                            */
/* Input:  "userData" is a pointer to the current object.                     */
/*         "command" is the command the server has sent.                      */
/*         "arguments" is the command arguments.                              */
/******************************************************************************/
void MainWindowSystem::ServerMessageReceived(void *userData, PString command, PList<PString> &arguments)
{
	MainWindowSystem *winSystem = (MainWindowSystem *)userData;

	// Check the command
	if (command == "NewPosition")
		winSystem->DoNewPosition(arguments);
	else if (command == "NewInformation")
		winSystem->DoNewInformation(arguments);
	else if (command == "ModuleEnded")
		winSystem->DoModuleEnded();
	else if (command == "ClickedFiles")
		winSystem->DoClickedFiles(arguments);
}



/******************************************************************************/
/* DoNewPosition() parse and run the "NewPosition" command.                   */
/*                                                                            */
/* Input:  "arguments" is the command arguments.                              */
/******************************************************************************/
void MainWindowSystem::DoNewPosition(PList<PString> &arguments)
{
	int16 songPos, songLength, early;
	int32 listEnd;
	int32 curPlay, newPlay, count;
	bool load = true;

	// Check the arguments
	ASSERT(arguments.CountItems() == 1);

	if (arguments.CountItems() != 1)
		return;

	// Check to see if a module is still loaded
	if (playerInfo->IsPlaying())
	{
		BMessage msg(AP_UPDATE_INFORMATION);

		// Extract the new song position
		songPos = arguments.GetItem(0).GetNumber();

		// The module position has been changed, so update the window
		msg.AddInt16("position", songPos);
		mainWin->PostMessage(&msg);

		// First check to see if there is module loop on
		if (!mainWin->IsLoopOn())
		{
			// Module loop is off
			//
			// Is double buffering on and haven't we read the next module?
			if ((useSettings->GetStringEntryValue("Options", "DoubleBuffering").CompareNoCase("Yes") == 0) &&
				(loader->GetFileHandle(1) == 0))
			{
				// There is double buffering on
				songPos    = playerInfo->GetSongPosition();
				songLength = playerInfo->GetSongLength();

				// If we can show positions, we can also check to see if
				// it's time to load the next module
				if (songPos >= 0)
				{
					early   = useSettings->GetIntEntryValue("Options", "EarlyLoad");
					listEnd = useSettings->GetIntEntryValue("Options", "ModuleListEnd");

					if (songPos >= (songLength - early))
					{
						// Check to see if we have to load the module
						curPlay = mainWin->GetPlayItem();
						count   = mainWin->GetListCount();

						// Are we at the end of the list?
						if ((curPlay + 1) == count)
						{
							if ((count == 1) || (listEnd != CV_LISTEND_JUMPTOSTART))
								load = false;
							else
								newPlay = 0;
						}
						else
						{
							// Just go to the next module
							newPlay = curPlay + 1;
						}

						// Load the next module
						if (load)
						{
							// Load the module
							BMessage loadMsg(AP_LOADINIT_MODULE);

							loadMsg.AddInt32("load", newPlay);
							loadMsg.AddBool("noError", true);
							loader->PostMessage(&loadMsg);
						}
					}
				}
			}
		}
	}
}



/******************************************************************************/
/* DoNewInformation() parse and run the "NewInformation" command.             */
/*                                                                            */
/* Input:  "arguments" is the command arguments.                              */
/******************************************************************************/
void MainWindowSystem::DoNewInformation(PList<PString> &arguments)
{
	// Check the arguments
	ASSERT(arguments.CountItems() == 2);

	if (arguments.CountItems() != 2)
		return;

	// Update the module information window
	infoWin->UpdateWindow(arguments.GetItem(0).GetUNumber(), arguments.GetItem(1));
}



/******************************************************************************/
/* DoModuleEnded() parse and run the "ModuleEnded" command.                   */
/******************************************************************************/
void MainWindowSystem::DoModuleEnded(void)
{
	int32 newPlay = 0, curPlay, count;
	int32 listEnd;
	bool loadNext = true;

	// Lock the window
	mainWin->Lock();

	// Check to see if there is module loop on
	if (!mainWin->IsLoopOn())
	{
		// Module loop is off
		//
		// Get number of modules in the list
		count = mainWin->GetListCount();

		// Get selected module
		curPlay = mainWin->GetPlayItem();

		if ((count > 0) && (curPlay >= 0))
		{
			// Test to see if we is in the end of the list
			if ((curPlay + 1) == count)
			{
				// We are, now check to see what we has to do
				listEnd = useSettings->GetIntEntryValue("Options", "ModuleListEnd");

				if (listEnd == CV_LISTEND_EJECTMODULE)
				{
					// Eject the module
					mainWin->StopAndFreeModule();
					loadNext = false;
				}
				else
				{
					if ((count == 1) || (listEnd == CV_LISTEND_LOOPMODULE))
					{
						loadNext = false;

						// Tell the mixer to continue
						HoldPlaying(false);

						// Reset the play time
						mainWin->PostMessage(AP_RESET_TIME);
					}
					else
						newPlay = 0;
				}
			}
			else
			{
				// Just go to the next module
				newPlay = curPlay + 1;
			}

			// Should we load the next module?
			if (loadNext)
			{
				// Is double buffering on?
				if ((useSettings->GetStringEntryValue("Options", "DoubleBuffering").CompareNoCase("Yes") == 0) &&
					(loader->GetFileHandle(1) != 0))
				{
					// Double buffering is on, and we have loaded the next module
					//
					// Free the current playing module
					mainWin->StopAndFreeModule();

					// Start the loaded module
					PlayModule(newPlay);
				}
				else
				{
					// Free the module
					mainWin->StopAndFreeModule();

					// Load the module
					LoadAndPlayModule(newPlay);
				}
			}
		}
	}
	else
	{
		// Tell the mixer to continue
		HoldPlaying(false);

		// Reset the play time
		mainWin->PostMessage(AP_RESET_TIME);
	}

	// Unlock the window
	mainWin->Unlock();
}



/******************************************************************************/
/* DoClickedFiles() parse and run the "ClickedFiles" command.                 */
/*                                                                            */
/* Input:  "arguments" is the command arguments.                              */
/******************************************************************************/
void MainWindowSystem::DoClickedFiles(PList<PString> &arguments)
{
	int32 i, count;
	int32 listCount;
	bool addJump;

	// Set the sleep cursor
	mainWin->SetSleepCursor();

	// Check the "add to list" option
	if (useSettings->GetStringEntryValue("Options", "AddToList").CompareNoCase("Yes") != 0)
	{
		// Stop the playing module if any
		mainWin->StopAndFreeModule();

		// Clear the module list
		mainWin->EmptyList(true);
	}

	// Get the previous number of items in the list
	listCount = mainWin->GetListCount();

	// Add all the files to the module list
	count = arguments.CountItems() - 1;
	for (i = 0; i < count; i++)
		mainWin->AddFileToList(arguments.GetItem(i), false);

	// Now add the last item with window updating
	if (count >= 0)
		mainWin->AddFileToList(arguments.GetItem(count));

	// Set cursor back to normal
	mainWin->SetNormalCursor();

	// Get the "Jump to added module" option
	addJump = useSettings->GetStringEntryValue("Options", "AddJump").CompareNoCase("Yes") == 0;

	// Do we need to load the first added module
	if (addJump || (listCount == 0))
	{
		mainWin->StopAndFreeModule();
		LoadAndPlayModule(listCount);
	}
}



/******************************************************************************/
/* InitMixer() initialize the virtual mixer.                                  */
/*                                                                            */
/* Input:  "initMixer" is a pointer to the initialize structure where the     */
/*         number of channels needed are stored.                              */
/******************************************************************************/
void MainWindowSystem::InitMixer(APAgent_InitMixer *initMixer)
{
	// Set the number of channels
	initMixer->channels = POLYPHONY_CHANNELS;

	// Initialize other variables
	chanNum    = 0;
	playSample = AP_ERROR;
}



/******************************************************************************/
/* EndMixer() cleanup the virtual mixer.                                      */
/******************************************************************************/
void MainWindowSystem::EndMixer(void)
{
}



/******************************************************************************/
/* Mixing() is the mixer function. Tells the APlayer mixer what to play.      */
/*                                                                            */
/* Input:  "mixing" is a pointer to the mixer structure.                      */
/*                                                                            */
/* Output: AP_OK to enable the mixer, AP_ERROR to disable it.                 */
/******************************************************************************/
ap_result MainWindowSystem::Mixing(APAgent_Mixing *mixing)
{
	PlaySample playSamp;
	uint16 playChan, i;

	// Get the sample to play
	if (sampWin->GetNextSampleFromQuery(&playSamp))
	{
		if (playSamp.playFrequency == 0.0f)
		{
			// Mute all the channels
			for (i = 0; i < POLYPHONY_CHANNELS; i++)
				mixing->channels[i]->Mute();

			playSample = AP_ERROR;
		}
		else
		{
			// Find the next channel to play in
			if (sampWin->PolyphonyEnabled())
			{
				playChan = chanNum;
				chanNum++;
				if (chanNum == POLYPHONY_CHANNELS)
					chanNum = 0;
			}
			else
			{
				playChan = 0;

				// Mute the other channels
				for (i = 1; i < POLYPHONY_CHANNELS; i++)
					mixing->channels[i]->Mute();
			}

			// Check the item to see if it's a legal sample
			if ((playSamp.sampInfo->address != NULL) && (playSamp.sampInfo->length != 0))
			{
				// Play it
				mixing->channels[playChan]->PlaySample(playSamp.sampInfo->address, 0, playSamp.sampInfo->length, playSamp.sampInfo->bitSize);
				mixing->channels[playChan]->SetFrequency(playSamp.playFrequency);

				uint16 vol = playSamp.sampInfo->volume;
				mixing->channels[playChan]->SetVolume(vol == 0 ? 256 : vol);

				int16 pan = playSamp.sampInfo->panning;
				mixing->channels[playChan]->SetPanning(pan == -1 ? APPAN_CENTER : pan);

				if (playSamp.sampInfo->flags & APSAMP_LOOP)
					mixing->channels[playChan]->SetLoop(playSamp.sampInfo->loopStart, playSamp.sampInfo->loopLength, playSamp.sampInfo->flags & APSAMP_PINGPONG ? APLOOP_PingPong : APLOOP_Normal);
			}
			else
			{
				// Mute the channel
				mixing->channels[playChan]->Mute();
			}

			playSample = AP_OK;
		}
	}

	return (playSample);
}
