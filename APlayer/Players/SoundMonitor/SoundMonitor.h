/******************************************************************************/
/* SoundMonitor 2.2 header file.                                              */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SoundMonitor_h
#define __SoundMonitor_h

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
/* Players                                                                    */
/******************************************************************************/
enum PlayerType
{
	SoundMon11 = 0,
	SoundMon22
};



/******************************************************************************/
/* Optionals                                                                  */
/******************************************************************************/
#define BP_OPT_ARPEGGIOONCE			0x0
#define BP_OPT_SETVOLUME			0x1
#define BP_OPT_SETSPEED				0x2
#define BP_OPT_FILTER				0x3
#define BP_OPT_PORTUP				0x4
#define BP_OPT_PORTDOWN				0x5
#define BP_OPT_SETREPCOUNT			0x6		// SoundMonitor 1.1
#define BP_OPT_VIBRATO				0x6		// SoundMonitor 2.2
#define BP_OPT_DBRA_REPCOUNT		0x7		// SoundMonitor 1.1
#define BP_OPT_JUMP					0x7		// SoundMonitor 2.2
#define BP_OPT_SETAUTOSLIDE			0x8
#define BP_OPT_SETARPEGGIO			0x9
#define BP_OPT_TRANSPOSE			0xA
#define BP_OPT_CHANGEFX				0xB
#define BP_OPT_CHANGEINVERSION		0xD
#define BP_OPT_RESETADSR			0xE
#define BP_OPT_CHANGENOTE			0xF		// Not the best name, but I couldn't come up with a better one :(



/******************************************************************************/
/* Instrument structure                                                       */
/******************************************************************************/
typedef struct Instrument
{
	// Shared part
	bool		type;			// True = Synth, false = sample
	uint16		volume;			// Volume

	// Sample part
	char		name[25];		// Sample name
	uint16		length;			// Length of sample
	uint16		loopStart;		// Offset to loop start
	uint16		loopLength;		// Loop length
	int8 *		adr;			// Pointer to the sample

	// Synth part
	uint8		waveTable;
	uint16		waveLength;
	uint8		adsrControl;
	uint8		adsrTable;
	uint16		adsrLength;
	uint8		adsrSpeed;
	uint8		lfoControl;
	uint8		lfoTable;
	uint8		lfoDepth;
	uint16		lfoLength;
	uint8		lfoDelay;
	uint8		lfoSpeed;
	uint8		egControl;
	uint8		egTable;
	uint16		egLength;
	uint8		egDelay;
	uint8		egSpeed;
	uint8		fxControl;
	uint8		fxSpeed;
	uint8		fxDelay;
	uint8		modControl;
	uint8		modTable;
	uint8		modSpeed;
	uint8		modDelay;
	uint16		modLength;
} Instrument;



/******************************************************************************/
/* Step structure                                                             */
/******************************************************************************/
typedef struct Step
{
	uint16		trackNumber;
	int8		soundTranspose;
	int8		transpose;
} Step;



/******************************************************************************/
/* Track structure                                                            */
/******************************************************************************/
typedef struct Track
{
	int8		note;
	uint8		instrument;
	uint8		optional;
	uint8		optionalData;
} Track;



/******************************************************************************/
/* Voice structure                                                            */
/******************************************************************************/
typedef struct BPCurrent
{
	bool		restart;
	bool		useDefaultVolume;
	bool		synthMode;
	int8 *		synthPtr;
	uint16		period;
	uint8		volume;
	uint8		instrument;
	uint8		note;
	uint8		arpValue;
	int8		autoSlide;
	uint8		autoArp;
	uint16		egPtr;
	uint16		lfoPtr;
	uint16		adsrPtr;
	uint16		modPtr;
	uint8		egCount;
	uint8		lfoCount;
	uint8		adsrCount;
	uint8		modCount;
	uint8		fxCount;
	uint8		oldEgValue;
	uint8		egControl;
	uint8		lfoControl;
	uint8		adsrControl;
	uint8		modControl;
	uint8		fxControl;
	int8		vibrato;
} BPCurrent;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint8		speed;
	PTimeSpan	time;
} PosInfo;



/******************************************************************************/
/* SoundMonitor class                                                         */
/******************************************************************************/
class SoundMonitor : public APAddOnPlayer
{
public:
	SoundMonitor(APGlobalData *global, PString fileName);
	virtual ~SoundMonitor(void);

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

	virtual PString GetModuleName(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	void Cleanup(void);

	void BpPlay(void);
	void BpNext(void);
	void PlayIt(uint32 voice);
	void DoOptionals(uint32 voice, uint8 optional, uint8 optionalData);
	void DoEffects(void);
	void DoSynths(void);
	void Averaging(uint32 voice);
	void Transform2(uint32 voice, int8 delta);
	void Transform3(uint32 voice, int8 delta);
	void Transform4(uint32 voice, int8 delta);

	PResource *res;
	PlayerType playerType;
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;

	char moduleName[27];
	uint8 waveNum;
	uint16 stepNum;
	uint16 trackNum;

	Instrument instruments[15];
	Step *steps[4];
	Track **tracks;
	int8 *waveTables;

	BPCurrent bpCurrent[4];
	int8 synthBuffer[4][32];

	uint16 bpStep;
	uint8 vibIndex;
	uint8 arpCount;
	uint8 bpCount;
	uint8 bpDelay;
	int8 st;
	int8 tr;
	uint8 bpPatCount;
	uint8 bpRepCount;
	uint8 newPos;
	bool posFlag;
};

#endif
