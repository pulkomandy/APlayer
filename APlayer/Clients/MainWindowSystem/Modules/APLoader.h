/******************************************************************************/
/* APLoader header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APLoader_h
#define __APLoader_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"

// Client headers
#include "APWindowMainListItem.h"


/******************************************************************************/
/* Loader message                                                             */
/******************************************************************************/
#define AP_LOADINIT_MODULE			'_ldi'
#define AP_FREE_MODULE				'_frm'
#define AP_FREE_EXTRA				'_fre'
#define AP_FREE_ALL					'_fra'
#define AP_START_SONG				'_ssg'
#define AP_PAUSE_PLAYING			'_pau'
#define AP_RESUME_PLAYING			'_res'
#define AP_HOLD_PLAYING				'_hol'
#define AP_SET_POSITION				'_spo'
#define AP_SET_VOLUME				'_svo'
#define AP_SET_MIXER				'_smx'
#define AP_SET_CHANNELS				'_sch'



/******************************************************************************/
/* APLoader class                                                             */
/******************************************************************************/
class MainWindowSystem;

class APLoader : public BLooper
{
public:
	APLoader(APGlobalData *glob, MainWindowSystem *winSys);
	virtual ~APLoader(void);

	uint32 GetFileHandle(int32 index = 0);

	PTimeSpan GetTotalTimeFromFile(PString fileName);
	void SetTotalTimeOnFile(PString fileName, PTimeSpan totalTime);

protected:
	typedef struct APModuleItem
	{
		APWindowMainListItem *listItem;
		PString fileHandle;
		PString outputAgent;
	} APModuleItem;

	virtual void MessageReceived(BMessage *message);

	void ShowError(int32 resourceError, PString error);

	void StartPlaying(int16 song, int16 startPos = -1);
	void RefreshWindows(void);

	PString SendAddFile(PString fileName);
	bool SendCanChangePosition(PString handle);
	PString SendChangeChannels(PString handle, bool enabled, int16 startChan, int16 stopChan = -1);
	PString SendEndPlayer(PString fileName);
	PString SendGetAuthor(PString handle);
	uint16 SendGetCurrentSong(PString handle);
	uint16 SendGetMaxSongNumber(PString handle);
	uint16 SendGetModuleChannels(PString handle);
	PString SendGetModuleFormat(PString handle);
	void SendGetModuleInformation(PString handle, PList<PString> &infoList);
	PString SendGetModuleName(PString handle);
	uint32 SendGetModuleSize(PString handle);
	PString SendGetPlayerName(PString handle);
	int16 SendGetSongLength(PString handle);
	int16 SendGetSongPosition(PString handle);
	void SendGetTimeList(PString handle, PList<PTimeSpan> &timeList);
	PTimeSpan SendGetTotalTime(PString handle);
	void SendHoldPlaying(PString handle, bool holdValue);
	PString SendInitPlayer(PString handle);
	PString SendLoadFile(PString handle, bool changeType);
	PString SendPausePlayer(PString handle);
	PString SendRemoveFile(PString handle);
	PString SendResumePlayer(PString handle);
	PString SendSetMixerSettings(PString handle, int32 frequency, int32 interpolation, int32 dolby, int32 stereoSep, int32 filter);
	PString SendSetOutputAgent(PString handle, PString agent);
	void SendSetPosition(PString handle, int16 newPos);
	void SendSetVolume(PString handle, uint16 newVol);
	PString SendStartPlayer(PString handle, int16 subSong);
	PString SendStopPlayer(PString handle);
	PString SendUnloadFile(PString handle);

	APGlobalData *global;
	MainWindowSystem *windowSystem;

	PList<APModuleItem> loadedFiles;
};

#endif
