/******************************************************************************/
/* OctaMED header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __OctaMED_h
#define __OctaMED_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PTime.h"

// APlayerKit headers
#include "APGlobalData.h"
#include "APAddOns.h"
#include "APChannel.h"

// Player headers
#include "MEDTypes.h"
#include "Tempo.h"


/******************************************************************************/
/* Player type enums                                                          */
/******************************************************************************/
enum MedType
{
	medUnknown						= -1,
	medMED							= 0,
	medOctaMED,
	medOctaMED_Professional4,
	medOctaMED_Professional6,
	medOctaMED_SoundStudio
};



/******************************************************************************/
/* Some constants                                                             */
/******************************************************************************/
#define MAX_TRACKS					64		// Max number of tracks OctaMED supports
#define MAX_INSTRS					63		// Max number of instruments



/******************************************************************************/
/* Song flags                                                                 */
/******************************************************************************/
#define MMD_FLAG_FILTERON			0x01
#define MMD_FLAG_JUMPINGON			0x02
#define MMD_FLAG_JUMP8TH			0x04
#define MMD_FLAG_INSTRSATT			0x08	// Instruments are attached (this is a module)
#define MMD_FLAG_VOLHEX				0x10
#define MMD_FLAG_STSLIDE			0x20	// SoundTracker mode for slides
#define MMD_FLAG_8CHANNEL			0x40	// OctaMED 8 channel song
#define MMD_FLAG_SLOWHQ				0x80	// HQ slows playing speed (V2-V4 compatibility)

#define MMD_FLAG2_BMASK				0x1f
#define MMD_FLAG2_BPM				0x20
#define MMD_FLAG2_MIX				0x80	// Uses mixing (V7+)

#define MMD_FLAG3_STEREO			0x01	// Mixing in Stereo mode
#define MMD_FLAG3_FREEPAN			0x02	// Free panning
#define MMD_FLAG3_GM				0x04	// Module designed for GM/XG compatibility



/******************************************************************************/
/* Instrument flags                                                           */
/******************************************************************************/
#define MMD_INSTRFLAG_LOOP			0x01
#define MMD_INSTRFLAG_EXTMIDIPSET	0x02
#define MMD_INSTRFLAG_DISABLED		0x04
#define MMD_INSTRFLAG_PINGPONG		0x08



/******************************************************************************/
/* Module header structure                                                    */
/******************************************************************************/
typedef struct MMD_Hdr
{
	uint32		id;
	uint32		modLen;
	uint32		songOffs;
	uint16		pSecNum;
	uint16		pSeq;
	uint32		blocksOffs;
	uint8		mmdFlags;
	uint32		samplesOffs;
	uint32		expDataOffs;
	uint16		pState;
	uint16		pBlock;
	uint16		pLine;
	uint16		pSeqNum;
	int16		actPlayLine;
	uint8		counter;
	uint8		extraSongs;
} MMD_Hdr;



/******************************************************************************/
/* MMD0 sample structure                                                      */
/******************************************************************************/
typedef struct MMD0Sample
{
	uint16		rep;
	uint16		repLen;
	uint8		midiCh;
	uint8		midiPreset;
	uint8		volume;
	int8		sTrans;
} MMD0Sample;



/******************************************************************************/
/* MMD0 song data structure                                                   */
/******************************************************************************/
typedef struct MMD0SongData
{
	uint16		numBlocks;
	uint16		songLen;
	uint8		playSeq[256];
	uint16		defTempo;
	int8		playTransp;
	uint8		flags;
	uint8		flags2;
	uint8		tempo2;
	uint8		trkVol[16];
	uint8		masterVol;
	uint8		numSamples;
} MMD0SongData;



/******************************************************************************/
/* MMD0 expansion structure                                                   */
/******************************************************************************/
typedef struct MMD0ExpData
{
	uint32		nextHdr;
	uint32		insTextOffs;
	uint16		insTextEntries;
	uint16		insTextEntrySize;
	uint32		annoTextOffs;
	uint32		annoTextLength;
	uint32		instInfoOffs;
	uint16		instInfoEntries;
	uint16		instInfoEntrySize;
	uint32		obsolete0;
	uint32		obsolete1;
	uint8		channelSplit[4];
	uint32		notInfoOffs;
	uint32		songNameOffs;
	uint32		songNameLen;
	uint32		dumpsOffs;
	uint32		mmdInfoOffs;
	uint32		mmdARexxOffs;
	uint32		mmdCmd3xOffs;
	uint32		trackInfoOffs;		// Pointer to song->numTracks pointers to tag lists
	uint32		effectInfoOffs;		// Pointer to group pointers
} MMD0ExpData;



/******************************************************************************/
/* MMD1 block info structure                                                  */
/******************************************************************************/
typedef struct MMD1BlockInfo
{
	uint32		hlMask;
	uint32		blockName;
	uint32		blockNameLen;
	uint32		pageTable;
	uint32		cmdExtTable;
} MMD1BlockInfo;



/******************************************************************************/
/* MMD2 song data structure                                                   */
/******************************************************************************/
typedef struct MMD2SongData
{
	uint16		numBlocks;
	uint16		numSections;
	uint32		playSeqTableOffs;
	uint32		sectionTableOffs;
	uint32		trackVolsOffs;
	uint16		numTracks;
	uint16		numPlaySeqs;
	uint32		trackPansOffs;
	uint32		flags3;
	uint16		volAdj;
	uint16		channels;
	uint8		mixEchoType;
	uint8		mixEchoDepth;
	uint16		mixEchoLen;
	int8		mixStereoSep;
	uint16		defTempo;
	int8		playTransp;
	uint8		flags;
	uint8		flags2;
	uint8		tempo2;
	uint8		masterVol;
	uint8		numSamples;
} MMD2SongData;



/******************************************************************************/
/* MMD instrument extension structure                                         */
/******************************************************************************/
typedef struct MMD_InstrExt
{
	uint8		defaultPitch;
	uint8		instrFlags;
	uint16		longMidiPreset;
	uint8		outputDevice;
	uint8		reserved;
	uint32		longRepeat;
	uint32		longRepLen;
} MMD_InstrExt;



/******************************************************************************/
/* MMD synth sound structure                                                  */
/******************************************************************************/
typedef struct MMD_SynthSound
{
	uint8		decay;				// Only used in separate instruments
	uint16		rpt;				// -""-
	uint16		rptLen;				// -""-
	uint16		volTblLen;
	uint16		wfTblLen;
	uint8		volSpeed;
	uint8		wfSpeed;
	uint16		numWfs;				// Number of waveforms
} MMD_SynthSound;



/******************************************************************************/
/* Position info structure                                                    */
/******************************************************************************/
typedef struct PosInfo
{
	Tempo		tempo;
	PTimeSpan	time;
} PosInfo;



/******************************************************************************/
/* Subsong time structure                                                     */
/******************************************************************************/
typedef struct SongTime
{
	PTimeSpan		totalTime;
	PList<PosInfo>	posInfoList;
} SongTime;



/******************************************************************************/
/* OctaMED class                                                              */
/******************************************************************************/
class Song;
class Player;

class OctaMED : public APAddOnPlayer
{
public:
	OctaMED(APGlobalData *global, PString fileName);
	virtual ~OctaMED(void);

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
	virtual uint16 GetModuleChannels(void);
	virtual const uint16 *GetSubSongs(void);

	virtual int16 GetSongLength(void);
	virtual int16 GetSongPosition(void);
	virtual void SetSongPosition(int16 pos);

	virtual PTimeSpan GetTimeTable(uint16 songNum, PList<PTimeSpan> &posTimes);

	virtual bool GetInfoString(uint32 line, PString &description, PString &value);
	virtual bool GetSampleInfo(uint32 num, APSampleInfo *info);

protected:
	friend class Instr;
	friend class Player;
	friend class Sample;
	friend class SubSong;

	MedType TestModule(PFile *file);
	void Cleanup(void);

	void ReadMMDHeader(PFile *file, MMD_Hdr *header);
	void ReadMMD0Sample(PFile *file, MMD0Sample *sample);
	void ReadMMD0Song(PFile *file, MMD0SongData *song);
	void ReadMMD0ExpData(PFile *file, MMD0ExpData *expData);

	void ReadMMD1BlockInfo(PFile *file, MMD1BlockInfo *blockInfo);

	void ReadMMD2Song(PFile *file, MMD2SongData *song);

	PResource *res;
	uint32 mark;

	PList<SongTime *> songTimeList;
	uint16 songTab[2];
	uint32 numSamples;
	uint16 numChannels;

	uint32 currentSong;

	Song *sg;
	Player *plr;
};

#endif
