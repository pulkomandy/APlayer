/******************************************************************************/
/* Player header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Player_h
#define __Player_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Mixer.h"
#include "PlayPosition.h"
#include "Song.h"


/******************************************************************************/
/* Player class                                                               */
/******************************************************************************/
class Player : public MED_Mixer
{
public:
	Player(OctaMED *octaMED);
	virtual ~Player(void);

	void PlaySong(Song *song);
	void PlrCallBack(void);

	void EnableTrack(TRACK_NUM tNum, bool en);
	bool IsTrackEnabled(TRACK_NUM tNum) const;

	void RecalcVolAdjust(void);

protected:
	friend class OctaMED;

	// Track specific synthsound handling data
	typedef struct SynthData
	{
		enum
		{
			none, synth, hybrid
		} synthType;

		int32		periodChange;
		uint32		vibOffs;			// Vibrato offset
		uint32		vibSpeed;			// Speed
		int32		vibDep;				// Depth
		uint32		vibWFNum;			// Waveform number (0 = default sine)
		uint32		arpStart;			// Arpeggio sequence start offset (in wfTable)
		uint32		arpOffs;			// Current arpeggio offset
		uint32		volCmdPos;			// Current volume execution position
		uint32		wfCmdPos;			// Current waveform execution position
		uint32		volWait;			// Current volume wait
		uint32		wfWait;				// Current waveform wait
		uint32		volChgSpeed;		// Volume change speed
		uint32		wfChgSpeed;			// Waveform change speed
		uint32		volXSpeed;			// Execution speed
		uint32		wfXSpeed;			// ...for waveform too
		int32		volXCnt;			// Execution counter
		int32		wfXCnt;
		uint32		envWFNum;			// Envelope waveform # (0 = none)
		bool		envLoop;			// Envelope looping?
		uint16		envCount;			// Envelope position counter
		int32		vol;				// Current synth volume
		NOTE_NUM	noteNumber;			// Note number with transposes for arpeggio
	} SynthData;

	// TrackData structure for each track
	typedef struct TrackData
	{
		NOTE_NUM	trkPrevNote;
		INST_NUM	trkPrevINum;
		uint8		trkPrevVol;
		int8		trkPrevMidiCh;
		uint8		trkPrevMidiN;		// 0 = none, 1 = 0, 2 = 1, ...
		uint8		trkMiscFlags;
		uint32		trkPrevMidiPort;
		int32		trkNoteOffCnt;		// -1 = none
		uint32		trkInitHold;
		uint32		trkInitDecay;
		uint32		trkDecay;
		uint32		trkFadeSpeed;		// Fading speed for decay
		int32		trkSTransp;
		int32		trkFineTune;
		int32		trkArpAdjust;
		int32		trkVibrAdjust;
		uint32		trkSOffset;
		NOTE_NUM	trkCurrNote;

		enum
		{
			normal, none, MIDI, noPlay = MIDI
		} trkFxType;

		int32		trkFrequency;
		int32		trkPortTargetFreq;	// Portamento (cmd 03) target frequency
		uint16		trkPortSpeed;
		int32		trkCutOffTarget;	// Filter cutoff sweep (cmd 23) target value
		uint16		trkCutOffSwSpeed;	// Cutoff sweep speed
		int32		trkCutOffLogPos;	// Logarithmic position of cutoff sweep
		uint8		trkVibShift;		// Depends on cmd 04 or 14
		uint8		trkVibSpeed;
		uint8		trkVibSize;
		uint8		trkTempVol;			// Temporary volume (+1; 0 = none)
		uint16		trkVibOffs;
		bool		trkLastNoteMidi;	// True if last note was MIDI note
		SynthData	trkSy;

		// Flag bits for trkMiscFlag
		enum
		{
			BACKWARDS = 0x01,
			NO_SYNTH_WFPTR_RESET = 0x02,
			STOPNOTE = 0x04
		};
	} TrackData;

	void ChangePlayFreq(void);

	void PlrPlayNote(TRACK_NUM trk, NOTE_NUM note, INST_NUM iNum);

	void ExtractInstrData(TrackData &trkd, Instr *currI);
	void PlayFXNote(TRACK_NUM trkNum, TrackData &trkd);
	void UpdateFreqVolPan(SubSong *ss, TRACK_NUM trkNum);
	bool MIDICommand(TrackData &trkd, MED_Cmd &cmd);

	int32 SynthHandler(uint32 chNum, TrackData &trkd, SynthSound *snd);

	virtual void MuteChannel(uint32 chNum);

	static bool PSeqCmdHandler(OctaMED *octaMED, PlaySeqEntry &pse);

	static const int8 sineTable[32];

	TrackData td[MAX_TRACKS];

	bool plrTrackEnabled[MAX_TRACKS];	// Track on/off states

	Song *plrSong;
	int32 plrPulseCtr;
	uint32 plrBlockDelay;
	TRACK_NUM plrChannels;
	LINE_NUM plrFxLine;
	LINE_NUM plrNextLine;
	BLOCK_NUM plrFxBlock;
	LINE_NUM plrRepeatLine;
	uint32 plrRepeatCounter;
	bool plrDelayedStop;
	PlayPosition plrPos;

	enum
	{
		normal, patternBreak, loop, positionJump
	} plrBreak;
};

#endif
