/******************************************************************************/
/* Sawteeth header file.                                                      */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Sawteeth_h
#define __Sawteeth_h

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
/* Defines                                                                    */
/******************************************************************************/
#define ST_CURRENT_FILE_VERSION			1200

#define CHN								12
#define CHNSTEPS						8192



/******************************************************************************/
/* Channel step structure                                                     */
/******************************************************************************/
typedef struct ChStep
{
	uint8		part;
	int8		transp;
	uint8		damp;
} ChStep;



/******************************************************************************/
/* Channel structure                                                          */
/******************************************************************************/
typedef struct Channel
{
	uint8		left;
	uint8		right;

	uint16		len;
	uint16		lLoop;
	uint16		rLoop;

	ChStep *	steps;
} Channel;



/******************************************************************************/
/* Step structure                                                             */
/******************************************************************************/
typedef struct Step
{
	uint8		note;
	uint8		ins;
	uint8		eff;		// 4 bits eff, 4 bits value
} Step;



/******************************************************************************/
/* Part structure                                                             */
/******************************************************************************/
typedef struct Part
{
	PString	name;

	Step *		steps;
	uint8		sps;		// PAL-screens per step
	uint8		len;
} Part;



/******************************************************************************/
/* TimeLev structure                                                          */
/******************************************************************************/
typedef struct TimeLev
{
	uint8		time;
	uint8		lev;
} TimeLev;



/******************************************************************************/
/* InsStep structure                                                          */
/******************************************************************************/
typedef struct InsStep
{
	uint8		note;		// 8 bit note
	bool		relative;
	uint8		wForm;		// max 15
} InsStep;



/******************************************************************************/
/* Ins structure                                                              */
/******************************************************************************/
typedef struct Ins
{
	PString		name;

	InsStep *	steps;

	// Filter - Amp
	TimeLev *	amp;
	TimeLev *	filter;

	uint8		ampPoints;
	uint8		filterPoints;

	uint8		filterMode;
	uint8		clipMode;
	uint8		boost;

	uint8		sps;		// PAL-screens per step
	uint8		res;		// Resonance

	uint8		vibS;
	uint8		vibD;

	uint8		pwmS;
	uint8		pwmD;

	uint8		loop;
	uint8		len;
} Ins;



/******************************************************************************/
/* BreakPoint structure                                                       */
/******************************************************************************/
typedef struct BreakPoint
{
	uint32		pal;
	uint32		command;
} BreakPoint;



/******************************************************************************/
/* Sawteeth class                                                             */
/******************************************************************************/
class Player;

class Sawteeth : public APAddOnPlayer
{
public:
	Sawteeth(APGlobalData *global, PString fileName);
	virtual ~Sawteeth(void);

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

	virtual void GetSamplePlayerInfo(APSamplePlayerInfo *sampInfo);
	virtual PString GetModuleName(void);
	virtual PString GetAuthor(void);
	virtual uint16 GetModuleChannels(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	friend class Player;
	friend class InsPly;

	PString ReadString(PFile *file);
	uint8 Read8Bit(PFile *file);
	uint16 Read16Bit(PFile *file);
	uint32 Read32Bit(PFile *file);

	void Cleanup(void);

	void Init(void);
	void InitSong(void);

	void SetPos(uint32 newPals);

	void MemMulMove(int16 *d, float *s, uint32 count, float level);

	PResource *res;
	PTimeSpan totalTime;
	PList<PTimeSpan> posInfoList;

	uint32 mark;
	uint8 posChannel;
	int16 songLen;

	int16 **outBuffers;

	PString name;
	PString author;

	uint16 stVersion;
	uint16 spsPal;

	uint8 channelCount;
	uint8 partCount;
	uint8 instrumentCount;
	uint8 breakPCount;

	Channel *chan;
	Part *parts;
	Ins *ins;
	BreakPoint *breakPoints;

	Player **p;
	uint32 pals;

	bool looped;

	float *n2f;
	float *r2f;
	float cMul[CHN];
};

#endif
