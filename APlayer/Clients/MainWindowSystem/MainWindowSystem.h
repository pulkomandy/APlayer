/******************************************************************************/
/* MainWindowSystem header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __MainWindowSystem_h
#define __MainWindowSystem_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PThread.h"
#include "PSettings.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APList.h"

// Client headers
#include "APLoader.h"
#include "APPlayerInfo.h"
#include "APWindowMain.h"
#include "APWindowUpdate.h"
#include "APWindowAbout.h"
#include "APWindowSettings.h"
#include "APWindowModuleInfo.h"


/******************************************************************************/
/* Network update macros                                                      */
/******************************************************************************/
//#define NETWORK_AGENT_NAME(str)			str.Format("APlayer %d.%d.%d", versionMajor, versionMiddle, versionMinor);
//#define NETWORK_FTP_LOGON_NAME			"anonymous"
//#define NETWORK_FTP_LOGON_PASSWORD		"aplayer_updater@polycode.dk"

#define NETWORK_AGENT_NAME(str)			str.Format("APlayer %d.%d.%d Beta %d", versionMajor, versionMiddle, versionMinor, versionBeta);
#define NETWORK_FTP_LOGON_NAME			"beta"
#define NETWORK_FTP_LOGON_PASSWORD		"betasoftware"

#if __p_os == __p_beos && __POWERPC__

//#define NETWORK_UPDATE_FILE				"HTTP://update.aplayer.dk/check_product_update.php?product=APlayer&os=BeOS&cpu=PowerPC"
#define NETWORK_UPDATE_FILE				"HTTP://update.aplayer.dk/check_product_update.php?product=APlayer&os=BeOS&cpu=PowerPC&beta=true"

#elif __p_os == __p_beos

//#define NETWORK_UPDATE_FILE				"HTTP://update.aplayer.dk/check_product_update.php?product=APlayer&os=BeOS&cpu=x86"
//#define NETWORK_UPDATE_FILE				"HTTP://update.aplayer.dk/check_product_update.php?product=APlayer&os=BeOS&cpu=x86&beta=true"
#define NETWORK_UPDATE_FILE ""

#endif



/******************************************************************************/
/* Configuration values                                                       */
/******************************************************************************/
// Unknown module list
#define CV_ERROR_SHOWERROR					0
#define CV_ERROR_SKIP						1
#define CV_ERROR_SKIPANDREMOVE				2
#define CV_ERROR_STOP						3

// End of module list
#define CV_LISTEND_EJECTMODULE				0
#define CV_LISTEND_JUMPTOSTART				1
#define CV_LISTEND_LOOPMODULE				2

// Check for updates list
#define CV_UPDATE_NEVER						0
#define CV_UPDATE_STARTUP					1
#define CV_UPDATE_DAILY						2
#define CV_UPDATE_WEEKLY					3
#define CV_UPDATE_MONTHLY					4



/******************************************************************************/
/* Other constants                                                            */
/******************************************************************************/
#define MAX_NUM_CHANNELS					64



/******************************************************************************/
/* MainWindowSystem class                                                     */
/******************************************************************************/
class APWindowSampleInfo;

class MainWindowSystem : public APAddOnClient
{
public:
	MainWindowSystem(APGlobalData *global, PString fileName);
	virtual ~MainWindowSystem(void);

	virtual float GetVersion(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual void EndAgent(int32 index);
	virtual ap_result Run(int32 index, uint32 command, void *args);

	virtual bool InitClient(int32 index);
	virtual void EndClient(int32 index);
	virtual void Start(int32 index);

	// Helper functions called from the window and its controls
	void LoadAndPlayModule(int32 index, int16 subSong = -1, int16 startPos = -1);
	void PlayModule(int32 index);
	void StopAndFreeModule(void);		// Do not call this function. Use the one in the main window
	void FreeExtraModules(void);
	void FreeAllModules(void);
	void StartSong(uint16 newSong);

	void PausePlaying(void);
	void ResumePlaying(void);
	void HoldPlaying(bool hold);
	void EndModule(void);

	void SetSongPosition(int16 newPos);
	void SetVolume(uint16 newVol);
	void SetMixerSettings(int32 interpolation, int32 dolby = -1, int32 stereoSep = -1, int32 filter = -1);
	void EnableChannels(bool enable, int16 startChan, int16 stopChan = -1);

	// Server command functions
	void OpenConfigWindow(PString addOnName);
	void OpenDisplayWindow(PString addOnName);

	PString EnableAddOn(PString addOnName);
	void DisableAddOn(PString addOnName);

	void SaveServerSettings(void);

	// New window functions
	void ShowAboutWindow(void);
	void ShowSettingsWindow(void);

	void ShowError(int32 resourceError, PString error);

	// Misc. functions
	bool CheckForUpdates(void);

	uint32 versionMajor;				// APlayer version
	uint32 versionMiddle;
	uint32 versionMinor;
	uint32 versionBeta;

	APList<APAddOnInformation *> agentAddOns;	// List over all installed agent add-ons
	APList<APAddOnInformation *> playerAddOns;	// List over all installed player add-ons
	APList<APAddOnInformation *> clientAddOns;	// List over all installed client add-ons
	APList<APAddOnInformation *> converterAddOns;// List over all installed converter add-ons

	void *serverHandle;					// Handle to the APlayer server
	APWindowMain *mainWin;				// Pointer to the main window
	APWindowUpdate *updateWin;			// Pointer to the update window
	APWindowAbout *aboutWin;			// Pointer to the about window
	APWindowSettings *settingsWin;		// Pointer to the settings window
	APWindowModuleInfo *infoWin;		// Pointer to the module information window
	APWindowSampleInfo *sampWin;		// Pointer to the sample information window

	PSettings *useSettings;				// Settings in use intern in APlayer
	PSettings *saveSettings;			// Settings stored on disk

	PResource *res;						// Client resources

	bool closeWindow;					// Indicator to the window to close or not

	APLoader *loader;					// Object to handle all the module loading
	APPlayerInfo *playerInfo;			// Holds all the information to the player

	bool channelsEnabled[MAX_NUM_CHANNELS];

protected:
	void GetAppVersion(void);

	void InitSettings(void);
	void CleanupSettings(void);

	void InitLoader(void);
	void CleanupLoader(void);

	void CreateAddOnLists(void);
	void FreeAddOnLists(void);

	void CreateWindows(void);
	void CloseWindows(void);

	void RetrieveStartupFiles(void);
	void AddDirectoryToList(PString directory);

	void GetList(APList<PString> &list, PString type, PString command);
	bool StartedNormally(void);

	void StartUpdateCheck(void);
	void StopUpdateCheck(void);

	static int32 CheckUpdateThread(void *userData);

	static void ServerMessageReceived(void *userData, PString command, PList<PString> &arguments);

	void DoNewPosition(PList<PString> &arguments);
	void DoNewInformation(PList<PString> &arguments);
	void DoModuleEnded(void);
	void DoClickedFiles(PList<PString> &arguments);

	void InitMixer(APAgent_InitMixer *initMixer);
	void EndMixer(void);
	ap_result Mixing(APAgent_Mixing *mixing);

	uint16 chanNum;
	ap_result playSample;

	PThread *checkThread;
};

#endif
