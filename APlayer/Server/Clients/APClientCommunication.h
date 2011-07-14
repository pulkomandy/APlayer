/******************************************************************************/
/* APClientCommunication header file.                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APClientCommunication_h
#define __APClientCommunication_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"
#include "PSkipList.h"

// APlayerKit headers
#include "APList.h"

// Server headers
#include "APModuleLoader.h"


/******************************************************************************/
/* Command function typedef                                                   */
/******************************************************************************/
class APClientCommunication;
typedef bool (*CommandFunc)(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);



/******************************************************************************/
/* File handle structure                                                      */
/******************************************************************************/
struct APPlayer;

typedef struct APFileHandle
{
	uint32 uniqueID;				// Unique ID number
	PString fileName;				// The file name with path
	APModuleLoader *loader;			// The module loader object which handle the file loading
	APPlayer *player;				// The player object which handle all the playing
	BLooper *looper;				// The client receive looper that has added this file

	// Below are mixer initialization values
	uint32 mixerFrequency;			// The frequency to mix in
	bool interpolation;				// True to use interpolation
	bool dolbyPrologic;				// True to use Dolby Prologic
	bool amigaFilter;				// True to use Amiga filter emulation
	uint16 stereoSeparator;			// The stereo separator value
	PString outputAgent;			// The name of the output agent to use
} APFileHandle;



/******************************************************************************/
/* APClientCommunication class                                                */
/******************************************************************************/
struct AddOnInfo;

class APClientCommunication : public BLooper
{
public:
	APClientCommunication(void);
	virtual ~APClientCommunication(void);

	void Start(void);
	void Stop(void);

	void SendCommand(APPlayer *player, PString command);
	void SendCommandToAll(PString command);

	const PList<APInstInfo *> *GetInstrumentList(uint32 fileHandle);
	void UnlockInstrumentList(uint32 fileHandle);

	const PList<APSampleInfo *> *GetSampleList(uint32 fileHandle);
	void UnlockSampleList(uint32 fileHandle);

protected:
	virtual void MessageReceived(BMessage *message);

	bool ParseCommand(BLooper *looper, PString command, PString &result);
	bool FindFileHandle(uint32 uniqueID, APFileHandle &handle);
	void SetFileHandle(uint32 uniqueID, APFileHandle &handle);
	bool FindAddOn(PString addOnName, APMRSWList<AddOnInfo *> *infoList, AddOnInfo *&info);

	void DisableVirtualMixer(AddOnInfo *agent);

	static bool AddFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool CanChangePosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool ChangeChannels(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool DisableAddOn(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool EnableAddOn(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool EndPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetAuthor(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetCurrentSong(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetMaxSongNumber(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetModuleChannels(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetModuleFormat(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetModuleInformation(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetModuleName(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetModuleSize(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetPlayerName(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetSongLength(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetSongPosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetTimeList(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool GetTotalTime(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool HoldPlaying(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool InitPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool LoadFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool OpenConfigWindow(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool OpenDisplayWindow(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool PausePlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool RemoveFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool ResumePlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool SaveSettings(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool SetMixerSettings(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool SetOutputAgent(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool SetPosition(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool SetVolume(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool StartedNormally(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool StartPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool StopPlayer(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);
	static bool UnloadFile(APClientCommunication *comm, BLooper *looper, const PList<PString> &args, PString &result);

	APList<BLooper *> clientLoopers;
	PSkipList<PString, CommandFunc> cmdList;

	APList<APFileHandle> fileHandleList;
};

#endif
