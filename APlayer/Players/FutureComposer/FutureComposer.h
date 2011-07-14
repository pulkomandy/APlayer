/******************************************************************************/
/* Future Composer header file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __FutureComposer_h
#define __FutureComposer_h

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
/* Sample info structure                                                      */
/******************************************************************************/
typedef struct SampleInfo
{
	int8 *		sample;
	uint16		length;
	uint16		loopStart;
	uint16		loopLength;
	bool		multi;
} SampleInfo;



/******************************************************************************/
/* Multi sample info structures                                               */
/******************************************************************************/
typedef struct MultiSampleInfo
{
	SampleInfo	sample[20];
} MultiSampleInfo;



/******************************************************************************/
/* Sequence structures                                                        */
/******************************************************************************/
typedef struct VoiceSeq
{
	uint8		pattern;
	int8		transpose;
	int8		soundTranspose;
} VoiceSeq;



typedef struct Sequence
{
	VoiceSeq	voiceSeq[4];
	uint8		speed;
} Sequence;



/******************************************************************************/
/* Pattern structures                                                         */
/******************************************************************************/
typedef struct PatternRow
{
	uint8		note;
	uint8		info;
} PatternRow;



typedef struct Pattern
{
	PatternRow	pattern[32];
} Pattern;



/******************************************************************************/
/* Volume sequence structure                                                  */
/******************************************************************************/
typedef struct VolSequence
{
	uint8		speed;
	uint8		frqNumber;
	int8		vibSpeed;
	int8		vibDepth;
	uint8		vibDelay;
	uint8		values[59];
} VolSequence;



/******************************************************************************/
/* Voice info structure                                                       */
/******************************************************************************/
typedef struct VoiceInfo
{
	int8		pitchBendSpeed;		// 4
	uint8		pitchBendTime;		// 5
	uint16		songPos;			// 6
	int8		curNote;			// 8
	uint8		curInfo;			// 9
	const uint8 *volumeSeq;			// 10
	uint8		volumeBendSpeed;	// 14
	uint8		volumeBendTime;		// 15
	uint16		volumeSeqPos;		// 16
	const uint8 *frequencySeq;		// 18
	int8		soundTranspose;		// 22
	uint8		volumeCounter;		// 23
	uint8		volumeSpeed;		// 24
	uint8		volSusCounter;		// 25
	uint8		susCounter;			// 26
	int8		vibSpeed;			// 27
	int8		vibDepth;			// 28
	int8		vibValue;			// 29
	int8		vibDelay;			// 30
	uint8		pad31;				// 31

	const Pattern *curPattern;		// 34

	bool		volBendFlag;		// 38
	bool		portFlag;			// 39
	uint16		patternPos;			// 40
	bool		pitchBendFlag;		// 42
	int8		pattTranspose;		// 43
	int8		transpose;			// 44
	int8		volume;				// 45
	uint8		vibFlag;			// 46
	uint8		portamento;			// 47
	uint16		pad48;				// 48
	uint16		frequencySeqPos;	// 50

	uint16		pitch;				// 56
	APChannel *	channel;			// 60
} VoiceInfo;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint8		speed;
	PTimeSpan	time;
} PosInfo;



/******************************************************************************/
/* FutureComposer class                                                       */
/******************************************************************************/
class FutureComposer : public APAddOnPlayer
{
public:
	FutureComposer(APGlobalData *global, PString fileName);
	virtual ~FutureComposer(void);

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

	void NewNote(uint16 chan);
	void Effect(uint16 chan);
	void DoVolBend(VoiceInfo *voiData);

	PResource *res;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;

	SampleInfo sampInfo[10 + 80];

	Sequence *sequences;
	Pattern *patterns;
	uint8 *frqSequences;
	VolSequence *volSequences;

	int16 seqNum;
	int16 patNum;
	int16 frqNum;
	int16 volNum;
	int16 wavNum;

	uint16 reSpCnt;
	uint16 repSpd;
	uint16 spdTemp;

	bool audTemp[4];
	VoiceInfo voiceData[4];
};

#endif
