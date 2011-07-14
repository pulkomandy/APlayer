/******************************************************************************/
/* APPlayerInfo header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APPlayerInfo_h
#define __APPlayerInfo_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSynchronize.h"
#include "PTime.h"
#include "PList.h"


/******************************************************************************/
/* APPlayerInfo class                                                         */
/******************************************************************************/
class APPlayerInfo
{
public:
	APPlayerInfo(void);
	virtual ~APPlayerInfo(void);

	void Lock(void);
	void Unlock(void);

	void ResetInfo(void);

	void SetCurrentSong(uint16 newSong);
	void SetMaxSongNumber(uint16 newMaxSong);
	void SetSongLength(int16 newLen);
	void SetSongPosition(int16 newPos);
	void SetModuleChannels(uint16 newChanNum);
	void SetModuleSize(uint32 newSize);
	void SetModuleName(PString newName);
	void SetAuthor(PString newName);
	void SetTotalTime(PTimeSpan newTotalTime);
	void SetPositionTimes(const PList<PTimeSpan> &newPosTimes);

	void SetFileName(PString newName);
	void SetModuleFormat(PString newFormat);
	void SetPlayerName(PString newName);
	void SetOutputAgent(PString newAgent);
	void SetModuleInformation(const PList<PString> &newInfo);

	void SetVolume(uint16 newVol);

	void SetChangePositionFlag(bool changeFlag);
	void SetInfoFlag(bool infoFlag);
	void SetPlayFlag(bool playFlag);
	void SetMuteFlag(bool muteFlag);

	uint16 GetCurrentSong(void);
	uint16 GetMaxSongNumber(void);
	int16 GetSongLength(void);
	int16 GetSongPosition(void);
	uint16 GetModuleChannels(void);
	uint32 GetModuleSize(void);
	PString GetModuleName(void);
	PString GetAuthor(void);
	PTimeSpan GetTotalTime(void);
	PTimeSpan GetPositionTime(int16 position);

	PString GetFileName(void);
	PString GetModuleFormat(void);
	PString GetPlayerName(void);
	PString GetOutputAgent(void);
	bool GetModuleInformation(int32 line, PString &description, PString &value);

	uint16 GetVolume(void);

	bool CanChangePosition(void);
	bool HaveInformation(void);
	bool IsPlaying(void);
	bool IsMuted(void);

protected:
	PMutex varLock;

	bool info;
	bool playing;
	uint16 currentSong;
	uint16 maxSongNum;
	int16 songLength;
	int16 songPos;
	uint16 chanNum;
	uint32 moduleSize;
	PString moduleName;
	PString author;
	bool changePos;

	PTimeSpan totalTime;
	PList<PTimeSpan> posTimes;

	PString fileName;
	PString moduleFormat;
	PString playerName;
	PString outputAgent;
	PList<PString> modInfo;

	bool muted;
	uint16 volume;
};

#endif
