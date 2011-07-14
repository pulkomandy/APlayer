/******************************************************************************/
/* SubSong header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SubSong_h
#define __SubSong_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PList.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Block.h"
#include "EffectMaster.h"
#include "LimVar.h"
#include "PlayPosition.h"
#include "Sequences.h"
#include "Tempo.h"


/******************************************************************************/
/* SubSong class                                                              */
/******************************************************************************/
class SubSong
{
public:
	enum		// Song flags
	{
		VOLHEX = 0x01,
		STSLIDE = 0x02,
		DUMMY = 0,
		STEREOMODE = 0x04,
		FREEPAN = 0x08,
		GM = 0x10
	};

	SubSong(OctaMED *octaMED, bool empty);
	virtual ~SubSong(void);

	void AppendNewSec(uint16 secNum);
	void Append(SectSeqEntry *sec);
	void Append(PlaySeq *pSeq);
	void Append(PlaySeqEntry *pse);
	void Append(PlaySeqEntry *pse, PSEQ_NUM pSeq);
	void Append(MED_Block *blk);

	PlaySeq &PSeq(uint32 pos);
	MED_Block &Block(BLOCK_NUM pos);
	SectSeqEntry &Sect(uint32 pos);
	const PlayPosition &Pos(void) const;

	void SetPos(const PlayPosition &newPos);

	void SetTempoBPM(uint16 newBPM);
	void SetTempoTPL(uint16 newTPL);
	void SetTempoLPB(uint8 newLPB);
	void SetTempoMode(bool bpm);
	void SetTempo(Tempo &tempo);
	void SetStartTempo(Tempo &tempo);

	void SetPlayTranspose(int16 pTransp);
	void SetNumChannels(uint32 channels);
	void SetVolAdjust(int16 volAdjust);
	void SetMasterVol(int32 vol);

	void SetTrackVol(TRACK_NUM trk, int32 vol);
	void SetTrackPan(TRACK_NUM trk, int32 pan);

	void SetStereo(bool stereo);
	void SetSlide1st(bool slide);
	void SetGM(bool gmMode);
	void SetFreePan(bool fp);
	void SetAmigaFilter(bool amigaFilter);

	void SetSongName(char *name);
	void SetTrackName(TRACK_NUM trk, char *newName);

	uint32 NumSections(void);
	uint32 NumBlocks(void);
	uint32 NumPlaySeqs(void);

	uint16 GetTempoBPM(void) const;
	uint16 GetTempoTPL(void) const;
	Tempo &GetTempo(void);
	Tempo &GetStartTempo(void);

	int16 GetPlayTranspose(void) const;
	uint32 GetNumChannels(void) const;
	int16 GetMasterVol(void) const;

	int32 GetTrackVol(TRACK_NUM trk) const;
	int32 GetTrackPan(TRACK_NUM trk) const;

	bool GetSlide1st(void) const;
	bool GetAmigaFilter(void) const;

	PString GetSongName(void) const;

	EffectMaster fx;			// Global effects

protected:
	OctaMED *med;

	PList<MED_Block *> s_blocks;
	PList<PlaySeq *> s_pSeqs;
	PList<SectSeqEntry *> s_sections;
	Tempo s_tempo;
	Tempo s_startTempo;
	LimVar<int16, -128, 127> s_playTransp;
	LimVar<uint32, 1, MAX_TRACKS> s_channels;
	LimVar<int16, 1, 800> s_volAdj;
	uint8 s_trkVol[MAX_TRACKS];
	int8 s_trkPan[MAX_TRACKS];
	PString s_trkName[MAX_TRACKS];
	LimVar<int16, 1, 64> s_masterVol;
	uint8 s_flags;
	PString s_name;
	PlayPosition position;
	bool filter;
};

#endif
