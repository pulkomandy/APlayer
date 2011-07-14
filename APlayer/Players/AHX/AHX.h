/******************************************************************************/
/* AHX header file.                                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __AHX_h
#define __AHX_h

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
/* Macro                                                                      */
/******************************************************************************/
#define Period2Freq(period)		(3579545.25f / (period))



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	int32 speed;
	PTimeSpan time;
} PosInfo;



/******************************************************************************/
/* Subsong time structure                                                     */
/******************************************************************************/
typedef struct SongTime
{
	int32 startPos;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;
} SongTime;



/******************************************************************************/
/* AHXOutput class                                                            */
/******************************************************************************/
class AHX;

class AHXOutput
{
public:
	AHXOutput(void);
	virtual ~AHXOutput(void);

	void Init(int32 frequency, int32 bits, int32 mixLen, float boost, int32 hz);
	void Free(void);

	void PrepareBuffers(int16 **buffers);
	void GenerateBuffer(int32 nrSamples, int16 **mb, int8 v);

	int32 bits;
	int32 frequency;
	int32 mixLen;
	int32 hz;

	AHX *player;

protected:
	int32 volumeTable[65][256];
};



/******************************************************************************/
/* AHXPosition class                                                          */
/******************************************************************************/
class AHXPosition
{
public:
	int32 track[4];
	int32 transpose[4];
};



/******************************************************************************/
/* AHXStep class                                                              */
/******************************************************************************/
class AHXStep
{
public:
	int32 note;
	int32 instrument;
	int32 fx;
	int32 fxParam;
};



/******************************************************************************/
/* AHXEnvelope class                                                          */
/******************************************************************************/
class AHXEnvelope
{
public:
	int32 aFrames;
	int32 aVolume;

	int32 dFrames;
	int32 dVolume;

	int32 sFrames;

	int32 rFrames;
	int32 rVolume;
};



/******************************************************************************/
/* AHXPListEntry class                                                        */
/******************************************************************************/
class AHXPListEntry
{
public:
	int32 note;
	int32 fixed;
	int32 waveform;
	int32 fx[2];
	int32 fxParam[2];
};



/******************************************************************************/
/* AHXPList class                                                             */
/******************************************************************************/
class AHXPList
{
public:
	int32 speed;
	int32 length;
	AHXPListEntry *entries;
};



/******************************************************************************/
/* AHXInstrument class                                                        */
/******************************************************************************/
class AHXInstrument
{
public:
	PString name;

	int32 volume;
	int32 waveLength;
	AHXEnvelope envelope;

	int32 filterLowerLimit;
	int32 filterUpperLimit;
	int32 filterSpeed;

	int32 squareLowerLimit;
	int32 squareUpperLimit;
	int32 squareSpeed;

	int32 vibratoDelay;
	int32 vibratoDepth;
	int32 vibratoSpeed;

	int32 hardCutReleaseFrames;
	bool hardCutRelease;

	AHXPList playList;
};



/******************************************************************************/
/* AHXSong class                                                              */
/******************************************************************************/
class AHXSong
{
public:
	AHXSong(void);
	virtual ~AHXSong(void);

	PString name;

	int32 restart;
	int32 positionNr;
	int32 trackLength;
	int32 trackNr;
	int32 instrumentNr;
	int32 subsongNr;

	int32 revision;
	int32 speedMultiplier;

	AHXPosition *positions;
	AHXStep **tracks;
	AHXInstrument *instruments;
	int32 *subsongs;
};



/******************************************************************************/
/* AHXVoice class                                                             */
/******************************************************************************/
class AHXVoice
{
public:
	AHXVoice(void);
	virtual ~AHXVoice(void);

	void Init(void);
	void CalcADSR(void);

	// Read those variables for mixing!
	int32 voiceVolume;
	int32 voicePeriod;
	int8 voiceBuffer[0x281];

protected:
	int32 track;
	int32 transpose;
	int32 nextTrack;
	int32 nextTranspose;

	int32 adsrVolume;				// Fixed point 8:8
	AHXEnvelope adsr;				// Frames / delta fixed 8:8

	AHXInstrument *instrument;		// Current instrument

	int32 instrPeriod;
	int32 trackPeriod;
	int32 vibratoPeriod;

	int32 noteMaxVolume;
	int32 perfSubVolume;
	int32 trackMasterVolume;

	bool newWaveform;
	int32 waveform;
	bool plantSquare;
	bool plantPeriod;
	bool ignoreSquare;

	bool trackOn;
	bool fixedNote;

	int32 volumeSlideUp;
	int32 volumeSlideDown;

	int32 hardCut;
	bool hardCutRelease;
	int32 hardCutReleaseF;

	int32 periodSlideSpeed;
	int32 periodSlidePeriod;
	int32 periodSlideLimit;
	bool periodSlideOn;
	bool periodSlideWithLimit;

	int32 periodPerfSlideSpeed;
	int32 periodPerfSlidePeriod;
	bool periodPerfSlideOn;

	int32 vibratoDelay;
	int32 vibratoCurrent;
	int32 vibratoDepth;
	int32 vibratoSpeed;

	bool squareOn;
	bool squareInit;
	int32 squareWait;
	int32 squareLowerLimit;
	int32 squareUpperLimit;
	int32 squarePos;
	int32 squareSign;
	bool squareSlidingIn;
	bool squareReverse;

	bool filterOn;
	bool filterInit;
	int32 filterWait;
	int32 filterLowerLimit;
	int32 filterUpperLimit;
	int32 filterPos;
	int32 filterSign;
	int32 filterSpeed;
	bool filterSlidingIn;
	bool ignoreFilter;

	int32 perfCurrent;
	int32 perfSpeed;
	int32 perfWait;

	int32 waveLength;

	AHXPList *perfList;

	int32 noteDelayWait;
	bool noteDelayOn;
	int32 noteCutWait;
	bool noteCutOn;

	int8 *audioSource;

	int32 audioPeriod;
	int32 audioVolume;

	int8 squareTempBuffer[0x80];

	friend class AHX;
};



/******************************************************************************/
/* AHXWaves class                                                             */
/******************************************************************************/
class AHXWaves
{
public:
	AHXWaves(void);
	virtual ~AHXWaves(void);

	// !!!!DO NOT CHANGE THE ORDER OF THESE TABLES!!!!
	int8 lowPasses[(0xfc + 0xfc + 0x80 * 0x1f + 0x80 + 3 * 0x280) * 31];

	int8 triangle04[0x04];
	int8 triangle08[0x08];
	int8 triangle10[0x10];
	int8 triangle20[0x20];
	int8 triangle40[0x40];
	int8 triangle80[0x80];

	int8 sawtooth04[0x04];
	int8 sawtooth08[0x08];
	int8 sawtooth10[0x10];
	int8 sawtooth20[0x20];
	int8 sawtooth40[0x40];
	int8 sawtooth80[0x80];

	int8 squares[0x80 * 0x20];

	int8 whiteNoiseBig[0x280 * 3];

	int8 highPasses[(0xfc + 0xfc + 0x80 * 0x1f + 0x80 + 3 * 0x280) * 31];

protected:
	void Generate(void);
	void GenerateSawtooth(int8 *buffer, int32 len);
	void GenerateTriangle(int8 *buffer, int32 len);
	void GenerateSquare(int8 *buffer);
	void GenerateWhiteNoise(int8 *buffer, int32 len);
	void GenerateFilterWaveforms(int8 *buffer, int8 *lowBuf, int8 *highBuf);

	inline void Clip(float *x);
};



/******************************************************************************/
/* AHX class                                                                  */
/******************************************************************************/
class AHX : public APAddOnPlayer
{
public:
	AHX(APGlobalData *global, PString fileName);
	virtual ~AHX(void);

	virtual float GetVersion(void);

	virtual int32 GetCount(void);
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
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	void Cleanup(void);

	void Init(void);
	bool InitSubsong(int32 nr);

	void PlayIRQ(void);
	void ProcessStep(int32 v);
	void ProcessFrame(int32 v);
	void PListCommandParse(int32 v, int32 fx, int32 fxParam);

	void SetAudio(int32 v);

	PResource *res;
	PList<SongTime *> songTimeList;

	uint16 songTab[2];
	uint16 currentSong;

	AHXWaves *waves;
	AHXOutput *output;

	int32 bufLen;
	int16 *sampBuffer[4];

	int32 playingTime;
	AHXSong *song;

	AHXVoice voices[4];

	int32 stepWaitFrames;
	bool getNewPosition;
	bool songEndReached;
	int32 timingValue;

	bool patternBreak;
	int32 mainVolume;

	int32 playing;
	int32 tempo;

	int32 posNr;
	int32 posJump;

	int32 noteNr;
	int32 posJumpNote;

	int8 *waveformTab[4];
	int32 wnRandom;

	friend class AHXOutput;
};

#endif
