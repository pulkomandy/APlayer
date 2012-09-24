/******************************************************************************/
/* Instr header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Instr_h
#define __Instr_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "LimVar.h"
#include "Sample.h"


/******************************************************************************/
/* Instr class                                                                */
/******************************************************************************/

class Sample;

class Instr
{
public:
	enum
	{
		LOOP = 0x01,
		MIDI_EXT_PSET = 0x02,
		DISABLED = 0x04,
		PINGPONG = 0x08,
		MIDI_SUPPRESS_OFF = 0x10
	};

	Instr(void);
	virtual ~Instr(void);

	Sample *GetSample(void) const;

	void SetNum(OctaMED *octaMED, INST_NUM num);

	void SetName(const char *newName);
	void SetVol(uint32 vol);
	void SetRepeat(uint32 newRep, bool keepEnd = false);
	void SetRepeatLen(uint32 newLen);
	void SetTransp(int16 iTransp);
	void SetFineTune(int16 newFT);
	void SetHold(uint16 hold);
	void SetDecay(uint16 decay);
	void SetDefPitch(NOTE_NUM newPitch);
	void SetDefFreq(uint32 newFreq);
	void SetMIDICh(uint32 midiCh);

	PString GetName(void);
	uint32 GetVol(void) const;
	uint32 GetRepeat(void);
	uint32 GetRepeatLen(void);
	int16 GetTransp(void);
	int16 GetFineTune(void);
	uint16 GetHold(void);
	uint16 GetDecay(void);
	uint32 GetValidDefFreq(void) const;

	bool IsMIDI(void) const;

	void ValidateLoop(void);
	void Update(void);

	uint32 i_flags;

protected:
	void Reset(void);
	void KillLoop(void);

	OctaMED *med;

	PString i_name;
	LimVar<uint32, 0, 127> i_vol;			// Volume
	uint32 i_repStart;						// Repeat start
	uint32 i_repLength;						// Repeat length
	LimVar<int16, -128, 127> i_sTrans;		// Sample transpose
	LimVar<int16, -8, 7> i_fineTune;		// Fine tuning value
	LimVar<uint16, 0, 128> i_hold;			// Hold value
	LimVar<uint16, 0, 128> i_decay;			// Decay value
	uint32 i_midiCh;						// MIDI channel
	INST_NUM i_num;							// Ordinal number of this instrument
	uint32 i_defFreq;
};

#endif
