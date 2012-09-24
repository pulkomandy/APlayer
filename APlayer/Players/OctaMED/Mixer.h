/******************************************************************************/
/* Mixer header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Mixer_h
#define __Mixer_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Instr.h"
#include "Sample.h"
#include "Tempo.h"


/******************************************************************************/
/* Flags for the Play() function.                                             */
/******************************************************************************/
#define MIXER_PLAY_BACKWARDS			0x01
#define MIXER_PLAY_LOOP					0x02
#define MIXER_PLAY_PINGPONGLOOP			0x04



/******************************************************************************/
/* MED_Mixer class                                                            */
/*                                                                            */
/* In the original source, this class handles all the mixing of the channels. */
/* This version of the player uses APlayers own mixing routines, but we still */
/* uses some of the functions. Thats why we still have the class.             */
/******************************************************************************/

class Instr;
class Sample;

class MED_Mixer
{
public:
	MED_Mixer(OctaMED *octaMED);
	virtual ~MED_Mixer(void);

	uint32 GetNoteFrequency(NOTE_NUM note, int32 fineTune);
	uint32 GetInstrNoteFreq(NOTE_NUM note, Instr *i);

	void SetMixTempo(Tempo &newTempo);
	void Start(uint32 channels);
	void Stop(void);

protected:
	void Play(uint32 chNum, Sample *smp, uint32 freq, uint32 startOffs, uint32 loopStart, uint32 loopLen, uint16 flags);
	void ChangeSamplePosition(uint32 chNum, int32 change);
	void SetSamplePosition(uint32 chNum, int32 newPos);
	void SetChannelFreq(uint32 chNum, int32 freq);
	void SetChannelVolPan(uint32 chNum, uint16 volume, int16 pan);

	void PrepareSynthSound(uint32 chNum);
	void SetSynthWaveform(uint32 chNum, void *data, uint32 length);

	virtual void MuteChannel(uint32 chNum);

	OctaMED *med;

	uint32 md_Channels;

	uint16 md_freqTable[16][6 * 12];	// 6 octaves, 16 fine tune levels
	bool startSyn[MAX_TRACKS];			// Indicates that a track is ready to play a synth sound
};

#endif
