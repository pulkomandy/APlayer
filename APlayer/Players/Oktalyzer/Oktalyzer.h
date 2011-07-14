/******************************************************************************/
/* Oktalyzer header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Oktalyzer_h
#define __Oktalyzer_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"
#include "PList.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* Sample structure                                                           */
/******************************************************************************/
typedef struct Sample
{
	PString		name;
	uint32		length;
	uint16		repeatStart;
	uint16		repeatLength;
	uint16		volume;
	uint16		mode;			// 0 = 8, 1 = 4, 2 = B
	int8 *		sample;
} Sample;



/******************************************************************************/
/* PatternLine structure                                                      */
/******************************************************************************/
typedef struct PatternLine
{
	uint8		note;
	uint8		sampleNum;
	uint8		effect;
	uint8		effectArg;
} PatternLine;



/******************************************************************************/
/* Pattern structure                                                          */
/******************************************************************************/
typedef struct Pattern
{
	int16		lineNum;
	PatternLine *lines;
} Pattern;



/******************************************************************************/
/* ChannelInfo structure                                                      */
/******************************************************************************/
typedef struct ChannelInfo
{
	uint8		currNote;
	int16		currPeriod;
	int8 *		sample;
	uint32		releaseStart;
	uint32		releaseLength;
} ChannelInfo;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint16		speed;
	PTimeSpan	time;
} PosInfo;



/******************************************************************************/
/* Oktalyzer class                                                            */
/******************************************************************************/
class Oktalyzer : public APAddOnPlayer
{
public:
	Oktalyzer(APGlobalData *global, PString fileName);
	virtual ~Oktalyzer(void);

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

	virtual uint16 GetModuleChannels(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	void ParseCMOD(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParseSAMP(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParseSPEE(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParseSLEN(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParsePLEN(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParsePATT(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParsePBOD(uint32 chunkSize, PFile *file, PString &errorStr);
	void ParseSBOD(uint32 chunkSize, PFile *file, PString &errorStr);

	void Cleanup(void);

	void FindNextPatternLine(void);
	void PlayPatternLine(void);
	void PlayChannel(uint32 chanNum);
	void SetVolumes(void);
	void DoEffects(void);
	void DoChannelEffect(uint32 chanNum);
	void PlayNote(uint32 chanNum, ChannelInfo *chanData, int8 note);

	PResource *res;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;

	uint32 sampNum;
	uint16 pattNum;
	uint16 songLength;
	uint16 chanNum;
	uint16 startSpeed;

	uint16 currentSpeed;
	uint16 speedCounter;
	int16 songPos;
	int16 newSongPos;
	int16 pattPos;
	bool filterStatus;

	bool channelFlags[4];
	uint8 patternTable[128];
	Sample *sampleInfo;
	Pattern **patterns;

	PatternLine currLine[8];
	ChannelInfo chanInfo[8];
	int8 chanVol[8];
	uint8 chanIndex[8];

	uint32 readPatt;
	uint32 readSamp;
	uint32 realUsedSampNum;
};

#endif
