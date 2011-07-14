/******************************************************************************/
/* Fred header file.                                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Fred_h
#define __Fred_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"


/******************************************************************************/
/* Player type enums                                                          */
/******************************************************************************/
enum FredType
{
	fredFinal	= 0,
	fredEditor
};



/******************************************************************************/
/* Instrument structure                                                       */
/******************************************************************************/
typedef struct Instrument
{
	PString		name;
	uint32		instIndex;			// Only used in the loader
	int8 *		sampleAdr;
	uint32		sampleOffset;		// Only used in the loader
	uint16		repeatLen;
	uint16		length;
	uint16		period;
	uint8		vibDelay;
	int8		vibSpeed;
	int8		vibAmpl;
	uint8		envVol;
	uint8		attackSpeed;
	uint8		attackVolume;
	uint8		decaySpeed;
	uint8		decayVolume;
	uint8		sustainDelay;
	uint8		releaseSpeed;
	uint8		releaseVolume;
	int8		arpeggio[16];
	uint8		arpSpeed;
	uint8		instType;			// See defines below
	int8		pulseRateMin;
	int8		pulseRatePlus;
	uint8		pulseSpeed;
	uint8		pulseStart;
	uint8		pulseEnd;
	uint8		pulseDelay;
	uint8		instSyncro;
	uint8		blend;
	uint8		blendDelay;
	uint8		pulseShotCounter;
	uint8		blendShotCounter;
	uint8		arpCount;
} Instrument;



/******************************************************************************/
/* Channel structure                                                          */
/******************************************************************************/
typedef struct Channel
{
	uint8		chanNum;
	int16 *		trackTable;
	uint8 *		trackPosition;
	uint16		trackOffset;
	uint16		trackDuration;
	uint8		trackNote;
	uint16		trackPeriod;
	int16		trackVolume;
	Instrument *instrument;
	uint8		vibFlags;
	uint8		vibDelay;
	int8		vibSpeed;
	int8		vibAmpl;
	int8		vibValue;
	bool		portRunning;
	uint16		portDelay;
	uint16		portLimit;
	uint8		portTargetNote;
	uint16		portStartPeriod;
	int16		periodDiff;
	uint16		portCounter;
	uint16		portSpeed;
	uint8		envState;
	uint8		sustainDelay;
	uint8		arpPosition;
	uint8		arpSpeed;
	bool		pulseWay;
	uint8		pulsePosition;
	uint8		pulseDelay;
	uint8		pulseSpeed;
	uint8		pulseShot;
	bool		blendWay;
	uint16		blendPosition;
	uint8		blendDelay;
	uint8		blendShot;
	int8		synthSample[64];	// Well, it seems only 32 bytes are needed, but we allocate 64 just in case
} Channel;



/******************************************************************************/
/* PosLength structure                                                        */
/******************************************************************************/
typedef struct PosLength
{
	uint8		channel;			// The channel to use positions from
	int16		length;				// The number of positions in the channel
	float		time;				// Number of seconds the channel take to play
	PList<float> posTimes;			// The start time for each position
} PosLength;



/******************************************************************************/
/* PosInfo structure                                                          */
/******************************************************************************/
/*typedef struct PosInfo
{
	uint8		tempo;				// The current tempo at the position
	uint8 *		trackPosition[4];	// Pointers to the track positions
	uint16		trackOffset[4];		// Position numbers
} PosInfo;
*/



/******************************************************************************/
/* Vibrato flags                                                              */
/******************************************************************************/
#define VIB_VIB_DIRECTION			0x01
#define VIB_PERIOD_DIRECTION		0x02



/******************************************************************************/
/* Envelope states                                                            */
/******************************************************************************/
#define ENV_ATTACK					0
#define ENV_DECAY					1
#define ENV_SUSTAIN					2
#define ENV_RELEASE					3
#define ENV_DONE					4



/******************************************************************************/
/* Instrument syncronize flags                                                */
/******************************************************************************/
#define SYNC_PULSE_X_SHOT			0x01
#define SYNC_PULSE_SYNCRO			0x02
#define SYNC_BLEND_X_SHOT			0x04
#define SYNC_BLEND_SYNCRO			0x08



/******************************************************************************/
/* Instrument types                                                           */
/******************************************************************************/
#define INST_SAMPLE					0x00
#define INST_PULSE					0x01
#define INST_BLEND					0x02
#define INST_UNUSED					0xff



/******************************************************************************/
/* Track commands                                                             */
/******************************************************************************/
#define TRK_END_CODE				0x80
#define TRK_PORT_CODE				0x81
#define TRK_TEMPO_CODE				0x82
#define TRK_INST_CODE				0x83
#define TRK_PAUSE_CODE				0x84
#define TRK_MAX_CODE				0xa0



/******************************************************************************/
/* Fred class                                                                 */
/******************************************************************************/
class Fred : public APAddOnPlayer
{
public:
	Fred(APGlobalData *global, PString fileName);
	virtual ~Fred(void);

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

	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	bool IsFinal(PFile *file);
	bool IsEditor(PFile *file);
	ap_result LoadFinal(PFile *file, PString &errorStr);
	ap_result LoadEditor(PFile *file, PString &errorStr);
	void Cleanup(void);

	uint8 DoNewLine(Channel *chan, APChannel *apChan);
	void CreateSynthSamplePulse(Instrument *inst, Channel *chan);
	void CreateSynthSampleBlend(Instrument *inst, Channel *chan);
	void ModifySound(Channel *chan, APChannel *apChan);

	void FindLongestChannel(uint16 songNum);
//	void CreatePosInfo(uint16 songNum);

	PResource *res;

	uint32 moduleOffset;
	int32 offsetDiff;

	uint16 subSongs[2];
	uint16 instNum;

	PosLength posLength[10];
//	PosInfo *posInfo[10];

	uint8 startTempos[10];
	uint16 startSequence[10][4];
	int16 *sequenceOffsets;
	uint8 *instNoteTable;
	Instrument *instruments;

	Channel channels[4];
	uint16 currentSong;
	uint8 currentTempo;
};

#endif
