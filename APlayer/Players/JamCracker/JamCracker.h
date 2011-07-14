/******************************************************************************/
/* JamCracker header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __JamCracker_h
#define __JamCracker_h

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
/* Note info structure                                                        */
/******************************************************************************/
typedef struct NoteInfo
{
	uint8			period;
	int8			instr;
	uint8			speed;
	uint8			arpeggio;
	uint8			vibrato;
	uint8			phase;
	uint8			volume;
	uint8			porta;
} NoteInfo;



/******************************************************************************/
/* Instrument info structure                                                  */
/******************************************************************************/
typedef struct InstInfo
{
	char			name[32];
	uint8			flags;
	uint32			size;
	int8 *			address;
} InstInfo;



/******************************************************************************/
/* Pattern info structure                                                     */
/******************************************************************************/
typedef struct PattInfo
{
	uint16			size;
	NoteInfo *		address;
} PattInfo;



/******************************************************************************/
/* Voice info structure                                                       */
/******************************************************************************/
typedef struct VoiceInfo
{
	uint16			waveOffset;
	uint16			dmacon;
	APChannel *		channel;
	uint16			insLen;
	int8 *			insAddress;
	const uint16 *	perAddress;
	uint16			pers[3];
	int16			por;
	int16			deltaPor;
	int16			porLevel;
	int16			vib;
	int16			deltaVib;
	int16			vol;
	int16			deltaVol;
	uint16			volLevel;
	uint16			phase;
	int16			deltaPhase;
	uint8			vibCnt;
	uint8			vibMax;
	uint8			flags;
} VoiceInfo;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint8			speed;
	PTimeSpan		time;
} PosInfo;



/******************************************************************************/
/* JamCracker class                                                           */
/******************************************************************************/
class JamCracker : public APAddOnPlayer
{
public:
	JamCracker(APGlobalData *global, PString fileName);
	virtual ~JamCracker(void);

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

	void NewNote(void);
	void NwNote(NoteInfo *adr, VoiceInfo *voice);
	void SetVoice(VoiceInfo *voice);
	void SetChannel(VoiceInfo *voice);
	void RotatePeriods(VoiceInfo *voice);

	PResource *res;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;

	uint16 samplesNum;
	uint16 patternNum;
	uint16 songLen;

	InstInfo *instTable;
	PattInfo *pattTable;
	uint16 *songTable;

	NoteInfo *address;
	VoiceInfo variables[4];

	uint16 tmpDMACON;

	uint16 songPos;
	uint16 songCnt;
	uint16 noteCnt;

	uint8 wait;
	uint8 waitCnt;
};

#endif
