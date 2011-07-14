/******************************************************************************/
/* SoundFX header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SoundFX_h
#define __SoundFX_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* Sample structure                                                           */
/******************************************************************************/
typedef struct Sample
{
	char		name[23];
	int8 *		sampleAdr;
	uint32		length;
	uint16		volume;
	uint32		loopStart;
	uint32		loopLength;
} Sample;



/******************************************************************************/
/* Channel structure                                                          */
/******************************************************************************/
typedef struct Channel
{
	uint32		patternData;
	int8 *		sampleStart;
	uint32		sampleLen;
	uint32		loopStart;
	uint32		loopLength;
	uint16		currentNote;
	uint16		volume;
	int16		stepValue;
	uint16		stepNote;
	uint16		stepEndNote;
} Channel;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	PTimeSpan	time;
} PosInfo;



/******************************************************************************/
/* SoundFX class                                                              */
/******************************************************************************/
class SoundFX : public APAddOnPlayer
{
public:
	SoundFX(APGlobalData *global, PString fileName);
	virtual ~SoundFX(void);

	virtual float GetVersion(void);

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

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	void Cleanup(void);

	void PlaySound(void);
	void PlayNote(Channel *channel, APChannel *virtChannel, uint32 patternData);
	void MakeEffects(Channel *channel, APChannel *virtChannel);
	void StepFinder(Channel *channel, bool stepDown);
	void Arpeggio(Channel *channel, APChannel *virtChannel);

	PResource *res;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;

	Channel channelInfo[4];

	Sample samples[31];
	uint8 orders[128];
	uint32 **patterns;

	uint16 delay;
	uint16 maxPattern;
	uint32 songLength;

	uint16 timer;
	uint32 trackPos;
	uint32 posCounter;
	bool breakFlag;
};

#endif
