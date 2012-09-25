/******************************************************************************/
/* GMEPlayer header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __GMEPlayer_h
#define __GMEPlayer_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PResource.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"

// Player headers
#include "libgme/gme.h"


/******************************************************************************/
/* GMEPlayer class                                                            */
/******************************************************************************/
class GMEPlayer : public APAddOnPlayer
{
public:
	GMEPlayer(APGlobalData *global, PString fileName);
	virtual ~GMEPlayer(void);

	virtual float GetVersion(void);

	virtual const APConfigInfo *GetConfigInfo(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);
	virtual PString GetModTypeString(int32 index);

	virtual ap_result ModuleCheck(int32 index, PFile *file);
	virtual ap_result LoadModule(int32 index, PFile *file, PString &errorStr);

	virtual bool InitPlayer(int32 index);
	virtual void EndPlayer(int32 index);
	virtual void InitSound(int32 index, uint16 songNum);
	virtual void Play(void);

	virtual void GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo);
	virtual PString GetModuleName(void);
	virtual PString GetAuthor(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);
	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);

protected:
	void Cleanup(void);
	void InitConfig(void);

	PResource *res;
	APConfigInfo cfgInfo;

	Music_Emu* theEmu;
	gme_info_t* fSongInfos;

	int16 *chanBuffers[2];

	uint8 *buffer;
	uint32 mixerFreq;
	uint16 songTab[2];

private:
	int16 oldPos;
	static const int timeScale = 100; // milliseconds per position
};

#endif
