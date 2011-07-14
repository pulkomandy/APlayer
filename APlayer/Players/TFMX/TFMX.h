/******************************************************************************/
/* TFMX header file.                                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __TFMX_h
#define __TFMX_h

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
/* Hdb structure                                                              */
/******************************************************************************/
struct Hdb
{
	uint32 pos;
	uint32 delta;
	uint16 sLen;
	uint16 sampleLength;
	int8 *sBeg;
	int8 *sampleStart;
	uint8 vol;
	uint8 mode;
	int32 (*loop)(struct Hdb *);
	int32 loopCnt;
	struct Cdb *c;
};



/******************************************************************************/
/* Idb structure                                                              */
/******************************************************************************/
struct Idb
{
	uint16 cue[4];
};



/******************************************************************************/
/* Mdb structure                                                              */
/******************************************************************************/
struct Mdb
{
	int8 playerEnable;
	int8 endFlag;
	int8 currSong;
	uint16 speedCnt;
	uint16 ciaSave;
	int8 songCont;
	int8 songNum;
	uint16 playPattFlag;
	int8 masterVol;
	int8 fadeDest;
	int8 fadeTime;
	int8 fadeReset;
	int8 fadeSlope;
	int16 trackLoop;
	uint16 dmaOn;
	uint16 dmaOff;
	uint16 dmaState;
	uint16 dmaAllow;
};



/******************************************************************************/
/* Pdb structure                                                              */
/******************************************************************************/
struct Pdb
{
	uint32 pAddr;
	uint8 pNum;
	int8 pxPose;
	uint16 pLoop;
	uint16 pStep;
	uint8 pWait;
	uint16 prOAddr;
	uint16 prOStep;
	bool looped;			// Added by Thomas Neumann
};



/******************************************************************************/
/* PdBlk structure                                                            */
/******************************************************************************/
struct PdBlk
{
	uint16 firstPos;
	uint16 lastPos;
	uint16 currPos;
	uint16 prescale;
	struct Pdb p[8];
};



/******************************************************************************/
/* Cdb structure                                                              */
/******************************************************************************/
struct Cdb
{
	int8 macroRun;
	int8 efxRun;
	uint8 newStyleMacro;
	uint8 prevNote;
	uint8 currNote;
	uint8 velocity;
	uint8 finetune;
	uint8 keyUp;
	uint8 reallyWait;
	uint32 macroPtr;
	uint16 macroStep;
	uint16 macroWait;
	uint16 macroNum;
	int16 loop;

	uint32 curAddr;
	uint32 saveAddr;
	uint16 currLength;
	uint16 saveLen;

	uint16 waitDMACount;
	uint16 dmaBit;

	uint8 envReset;
	uint8 envTime;
	uint8 envRate;
	int8 envEndVol;
	int8 curVol;

	int16 vibOffset;
	int8 vibWidth;
	uint8 vibFlag;
	uint8 vibReset;
	uint8 vibTime;

	uint8 portaReset;
	uint8 portaTime;
	uint16 curPeriod;
	uint16 destPeriod;
	uint16 portaPer;
	int16 portaRate;

	uint8 addBeginTime;
	uint8 addBeginReset;
	uint16 returnPtr;
	uint16 returnStep;
	int32 addBegin;

	uint8 sfxFlag;
	uint8 sfxPriority;
	uint8 sfxNum;
	int16 sfxLockTime;
	uint32 sfxCode;

	struct Hdb *hw;
};



/******************************************************************************/
/* 32-bit union                                                               */
/******************************************************************************/
#if __p_endian == __p_little

typedef union
{
	uint32 l;
	struct { uint16 w1, w0; } w;
	struct { uint8 b3, b2, b1, b0; } b;
} UNI;

#else

typedef union
{
	uint32 l;
	struct { uint16 w0, w1; } w;
	struct { uint8 b0, b1, b2, b3; } b;
} UNI;

#endif



/******************************************************************************/
/* One-file format structure                                                  */
/******************************************************************************/
typedef struct OneFile
{
	uint32 mark;		// TFHD
	uint32 headerSize;
	uint8 type;			// 0 = Unchecked, 1 = 1.5, 2 = Pro, 3 = 7V. Bit 7 = forced
	uint8 version;
	uint32 mdatSize;
	uint32 smplSize;
} OneFile;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	uint16 cia;
	uint16 speed;
	PTimeSpan time;
} PosInfo;



/******************************************************************************/
/* Subsong time structure                                                     */
/******************************************************************************/
typedef struct SongTime
{
	PTimeSpan totalTime;
	PList<PosInfo> posInfoList;
} SongTime;



/******************************************************************************/
/* TFMX class                                                                 */
/******************************************************************************/
class TFMX : public APAddOnPlayer
{
public:
	TFMX(APGlobalData *global, PString fileName);
	virtual ~TFMX(void);

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
	virtual uint16 GetModuleChannels(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);

protected:
	enum ModType { modOld = 0, modPro, mod7V };

	bool IsTFMXOld(PFile *file);
	bool IsTFMXPro(PFile *file);
	bool IsTFMX7V(PFile *file);
	bool IsOneFile(PFile *file, OneFile *header);

	void Cleanup(void);

	void CalcTimes(void);
	bool DoAChannelRow(struct Pdb *p1, struct Pdb *p, int32 chan);

	void SetupChannel(int32 chan);

	void NotePort(uint32 i);

	static int32 LoopOff(struct Hdb *hw);
	static int32 LoopOn(struct Hdb *hw);

	void RunMacro(struct Cdb *c);
	void DoEffects(struct Cdb *c);
	void DoMacro(int32 cc);
	void DoAllMacros(void);
	void ChannelOff(int32 i);
	void DoFade(int8 sp, int8 dv);
	void GetTrackStep(void);
	int32 DoTrack(struct Pdb *p, int32 pp);
	void DoTracks(void);

	void AllOff(void);
	void TfmxInit(void);
	void StartSong(int32 song, int32 mode);

	void Conv16(int32 *b, int16 *out, int32 num);
	void MixIt(int32 n, int32 b);
	void MixAdd(struct Hdb *hw, int32 n, int32 *b);

	PResource *res;
	ModType modType;
	uint16 currentSong;
	PList<SongTime *> songTimeList;

	bool oneFile;
	OneFile header;

	char comment[6][41];

	bool firstTime;
	uint16 subSongs[2];

	uint16 songStart[32];
	uint16 songEnd[32];
	uint16 tempo[32];

	uint32 trackStart;
	uint32 pattStart;
	uint32 macroStart;

	int32 numMac;
	int32 numPat;
	int32 numTS;

	int32 *macros;
	int32 *patterns;

	int32 *musicData;
	int8 *sampleData;
	int8 *sampleEnd;

	int32 musicLen;
	int32 sampleLen;

	struct Hdb hdb[8];
	struct Mdb mdb;
	struct Cdb cdb[16];
	struct PdBlk pdb;
	struct Idb idb;

	uint32 outRate;

	int32 gemx;
	int32 dangerFreakHack;

	int32 jiffies;
	int32 multiMode;

	int32 loops;
	int32 startPat;

	uint32 eClocks;
	int32 eRem;

	int32 *tbuf[7];
	int16 *outBuf[7];
};

#endif
