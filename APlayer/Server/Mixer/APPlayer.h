/******************************************************************************/
/* APPlayer header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __APPlayer_h
#define __APPlayer_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PTime.h"
#include "PList.h"
#include "PSynchronize.h"

// Server headers
#include "APClientCommunication.h"
#include "APMixer.h"


/******************************************************************************/
/* APPlayer class                                                             */
/******************************************************************************/
class APPlayer : public BLooper
{
public:
	APPlayer(void);
	virtual ~APPlayer(void);

	bool InitPlayer(APFileHandle handle, PString &result);
	void EndPlayer(void);

	void StartPlaying(int16 song);
	void StopPlaying(void);
	void PausePlaying(void);
	void ResumePlaying(void);
	void HoldPlaying(bool hold);

	bool CanChangePosition(void) const;

	void SetVolume(uint16 volume);
	void SetStereoSeparation(uint16 sep);
	void SetMixerMode(uint32 mode, bool enable);
	void EnableAmigaFilter(bool enable);
	void ChangeChannels(bool enable, int16 startChan, int16 stopChan);

	PString GetModuleName(void) const;
	PString GetAuthor(void) const;
	uint16 GetCurrentSong(void) const;
	uint16 GetMaxSongs(void) const;

	uint16 GetChannels(void) const;
	int16 GetSongLength(void) const;
	int16 GetSongPosition(void) const;
	void SetSongPosition(int16 pos);

	PTimeSpan GetTotalTime(void) const;
	const PList<PTimeSpan> *GetTimeList(void) const;

	PString GetModuleFormat(void) const;
	PString GetPlayerName(void) const;
	uint32 GetModuleSize(void) const;
	const PList<PString> *GetModuleInformation(void) const;

	const PList<APInstInfo *> *GetInstrumentList(void);
	void UnlockInstrumentList(void);

	const PList<APSampleInfo *> *GetSampleList(void);
	void UnlockSampleList(void);

	void DisableVirtualMixer(AddOnInfo *agent);

protected:
	virtual void MessageReceived(BMessage *message);

	void SendNewPosition(int16 position);
	void SendNewInformation(int32 line, PString value);
	void SendModuleEnded(void);

	PString FindAuthor(void);
	PString FindAuthorInList(PList<PString> &list);
	int32 FindBy(PString str);

	void GetSamples(void);
	void FreeSamples(void);

	uint16 subSongs[2];
	uint16 songNum;
	int16 songLength;
	PString author;
	PTimeSpan totalTime;
	PList<PTimeSpan> posTimes;

	PString moduleFormat;
	PString playerName;
	uint32 moduleSize;
	PList<PString> moduleInfo;

	PMutex *infoLock;
	PList<APInstInfo *> instruments;
	PList<APSampleInfo *> samples;

	PMutex *playerLock;
	APAddOnPlayer *currentPlayer;
	int32 playerIndex;
	APMixer mixer;
};

#endif
