/******************************************************************************/
/* Song header file.                                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Song_h
#define __Song_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"
#include "PList.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Instr.h"
#include "Sample.h"
#include "SubSong.h"
#include "SynthSound.h"


/******************************************************************************/
/* Song class                                                                 */
/******************************************************************************/
class Song
{
public:
	Song(OctaMED *octaMED);
	virtual ~Song(void);

	Instr *Sample2Instrument(const Sample *sample);

	void AppendNew(bool empty);

	SubSong *CurrSS(void);
	uint32 NumSubSongs(void);
	void SetSSNum(int32 ssNum);

	bool SampleSlotUsed(INST_NUM iNum);
	bool InstrSlotUsed(INST_NUM iNum);

	SubSong *GetSubSong(uint32 sNum);
	Sample *GetSample(INST_NUM num);
	SynthSound *GetSynthSound(INST_NUM num);
	Instr *CurrInstr(void);
	Instr *GetInstr(INST_NUM iNum);
	INST_NUM CurrInstrNum(void);

	void SetSample(INST_NUM num, Sample *s);
	void SetInstrNum(INST_NUM iNum);
	void SetAnnoText(const char *text);

	void UpdateSample(void);

	void ReadSynthSound(INST_NUM iNum, PFile *f, PString &errorStr, PResource *res, bool isHybrid);

	void operator++(int);

protected:
	OctaMED *med;

	PList<SubSong *> sg_ss;
	Instr sg_i[MAX_INSTRS];
	Sample *sg_sm[MAX_INSTRS];

	SubSong *current;
	uint32 currNum;
	uint32 currInstr;			// Currently selected instrument #
	PString annoText;
};

#endif
