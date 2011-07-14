/******************************************************************************/
/* APApplication header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APApplication_h
#define __APApplication_h

// PolyKit headers
#include "POS.h"
#include "PResource.h"
#include "PSynchronize.h"
#include "PSettings.h"
#include "PList.h"
#include "PSkipList.h"
#include "PPriorityList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APList.h"

// Server headers
#include "APError.h"
#include "APAddOnLoader.h"
#include "APClientCommunication.h"


/******************************************************************************/
/* Useful macro                                                               */
/******************************************************************************/
#define GetApp() ((APApplication *)be_app)



/******************************************************************************/
/* Add-On information structure.                                              */
/*                                                                            */
/* This structure holds all the information for a single add-on.              */
/******************************************************************************/
typedef struct AddOnInfo
{
	PString fileName;			// The add-on filename
	PString addOnName;			// The name of the add-on
	PString description;		// The description
	float version;				// The version
	int32 index;				// The add-on index in the file
	bool enabled;				// True if the add-on is enabled
	bool allocated;				// True if the loader object are allocated in this add-on
	bool settings;				// True if the add-on have a settings window
	bool display;				// True if the add-on have a display window
	APAddOnLoader *loader;		// Pointer to the loader object
	APAddOnAgent *agent;		// Pointer to the agent/client object. Only used in agents and clients
	uint32 pluginFlags;			// The flags returned by the GetSupportFlags() function
	bool isAgent;				// True if the add-on is an agent
	bool isClient;				// True if the add-on is a client
} AddOnInfo;



/******************************************************************************/
/* APApplication class                                                        */
/******************************************************************************/
class APAddOnWindows;

class APApplication : public BApplication
{
public:
	APApplication(const char *signature);
	virtual ~APApplication(void);

	static void GetAddOnInfo(APAddOnType type, APList<APAddOnInformation *> &list);
	static void FreeAddOnInfo(APList<APAddOnInformation *> &list);
	static const PList<APInstInfo *> *GetInstInfo(uint32 fileHandle);
	static void UnlockInstInfo(uint32 fileHandle);
	static const PList<APSampleInfo *> *GetSampInfo(uint32 fileHandle);
	static void UnlockSampInfo(uint32 fileHandle);
	static APAddOnBase *GetAddOnInst(PString name, APAddOnType type, int32 *index);
	static void DeleteAddOnInst(PString name, APAddOnType type, APAddOnBase *addOn);

	bool GotStartupMessage(void) const;

	void PlugAgentIn(AddOnInfo *info, APAddOnAgent *agent);
	void PlugAgentOut(AddOnInfo *info, bool deleteInstance);

	PResource *resource;				// Server resources

	PSettings *useSettings;				// Settings in use intern in APlayer
	PSettings *saveSettings;			// Settings stored on disk

	APClientCommunication *client;		// Object to communicate with all the clients
	APAddOnWindows *addOnWindows;		// Object to handle all the add-on windows

	// Add-on information
	APMRSWList<AddOnInfo *> converterInfo;	// Holds all the converters
	APMRSWList<AddOnInfo *> playerInfo;	// Holds all the players
	APMRSWList<AddOnInfo *> agentInfo;	// Holds all the agents
	APMRSWList<AddOnInfo *> clientInfo;	// Holds all the clients

	// Plug-in functions (Read-only from other classes)
	PMRSWLock pluginLock;

	PPriorityList<AddOnInfo *> converterAgents;
	PPriorityList<AddOnInfo *> decruncherAgents;
	PPriorityList<AddOnInfo *> virtualMixerAgents;
	PSkipList<PString, AddOnInfo *> soundOutputAgents;
	PPriorityList<AddOnInfo *> dspAgents;
	PPriorityList<AddOnInfo *> visualAgents;

protected:
	virtual void ReadyToRun(void);
	virtual void RefsReceived(BMessage *message);
	virtual bool QuitRequested(void);

	void InitAPlayerKit(void);
	void CleanupAPlayerKit(void);

	void InitSettings(void);
	void CleanupSettings(void);

	void InitAddOnWindows(void);
	void CleanupAddOnWindows(void);

	void InitClientConnections(void);
	void CleanupClientConnections(void);

	void StartClientAddOns(void);
	void StopClientAddOns(void);

	void PlugInAgents(void);
	void PlugOutAgents(void);

	void LoadAddOns(PString section, PString addOnDir, APMRSWList<AddOnInfo *> &infoList, bool agents);
	void UnloadAddOns(APMRSWList<AddOnInfo *> &infoList);
	bool ReadAddOnEntry(PString section, int32 entryNum, PString &addOnName, PString &fileName, bool &enabled) const;

	void SendClickedFiles(BMessage *files);

	BMessage *refMsg;
};

#endif
